/*
  Copyright 2002 - 2009 David A. Schleef <ds@schleef.org>
  Copyright 2012 MIPS Technologies, Inc.

  Author: Guillaume Emont <guijemont@igalia.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/
#include "config.h"

#include <orc/orcmips.h>
#include <orc/orcmsa.h>
#include <orc/orcdebug.h>

/*Two-bit Data Format Field Encoding*/
#define DF_B 0b00
#define DF_H 0b01
#define DF_W 0b10
#define DF_D 0b11

/*I10 instruction format: MSA operation df i10 wd minor_opcode*/
#define MSA_I10_INSTRUCTION(operation,df,wd,i10,mopcode) \
        (0b011110 << 26 \
         | ((operation) & 0x07) << 23 \
         | (df) << 21 \
         | (i10) << 11 \
         | ((wd)-ORC_VEC_REG_BASE) << 6 \
         | ((mopcode) & 0x3f))

/*MI10 instruction format: MSA s10 rs wd minor-opcode df*/
#define MSA_MI10_INSTRUCTION(s10,rs,wd,mopcode,df) \
            (0b011110 << 26 \
             | (s10) << 16 \
             | ((rs)-ORC_GP_REG_BASE) << 11 \
             | ((wd)-ORC_VEC_REG_BASE) << 6 \
             | (mopcode) << 2 \
             | ((df) & 0x3f))

/*2R instruction format: 011110 operation df ws wd minor_opcode*/
#define MSA_2R_INSTRUCTION(operation,df,ws,wd,mopcode) \
        (0b011110 << 26 \
         | ((operation) & 0xff) << 18 \
         | (df) << 16 \
         | ((ws)-ORC_VEC_REG_BASE) << 11 \
         | ((wd)-ORC_VEC_REG_BASE) << 6 \
         | ((mopcode) & 0x3f))

/*3R instruction format: 011110 operation df wt ws wd minor_opcode*/
#define MSA_3R_INSTRUCTION(operation,df,wt,ws,wd,mopcode) \
            (0b011110 << 26 \
             | ((operation) & 0x07) << 23 \
             | (df) << 21 \
             | ((wt)-ORC_VEC_REG_BASE) << 16 \
             | ((ws)-ORC_VEC_REG_BASE) << 11 \
             | ((wd)-ORC_VEC_REG_BASE) << 6 \
             | ((mopcode) & 0x3f))

/*ELM instruction format: 011110 operation df/n ws wd minor_opcode*/
#define MSA_ELM_INSTRUCTION(operation,dfn,ws,wd,mopcode) \
            (0b011110 << 26 \
             | ((operation) & 0x0f) << 22 \
             | (dfn)  << 16 \
             | ((ws)-ORC_VEC_REG_BASE) << 11 \
             | ((wd)-ORC_VEC_REG_BASE) << 6 \
             | ((mopcode) & 0x3f))

/*I10 instruction format: 011110 df i8 ws wd minor_opcode*/
#define MSA_I8_INSTRUCTION(df,i8,ws,wd,mopcode) \
        (0b011110 << 26 \
         | (df) << 24 \
         | (i8) << 16 \
         | ((ws)-ORC_VEC_REG_BASE) << 11 \
         | ((wd)-ORC_VEC_REG_BASE) << 6 \
         | ((mopcode) & 0x3f))

/*3RF instruction format: 011110 operation df wt ws wd minor_opcode*/
#define MSA_3RF_INSTRUCTION(operation,df,wt,ws,wd,mopcode) \
            (0b011110 << 26 \
             | ((operation) & 0x0f) << 22 \
             | (df) << 21 \
             | ((wt)-ORC_VEC_REG_BASE) << 16 \
             | ((ws)-ORC_VEC_REG_BASE) << 11 \
             | ((wd)-ORC_VEC_REG_BASE) << 6 \
             | ((mopcode) & 0x3f))

/*VEC instruction format: 011110 00000 wt ws wd minor_opcode*/
#define MSA_VEC_INSTRUCTION(operation,wt,ws,wd,mopcode) \
            (0b011110 << 26 \
             | ((operation) & 0x1f) << 21 \
             | ((wt)-ORC_VEC_REG_BASE) << 16 \
             | ((ws)-ORC_VEC_REG_BASE) << 11 \
             | ((wd)-ORC_VEC_REG_BASE) << 6 \
             | ((mopcode) & 0x3f))

static const char * orc_msa_reg_name (int reg)
{
  static const char *vec_regs[] = {
    "$w0", "$w1", "$w2", "$w3",
    "$w4", "$w5", "$w6", "$w7",
    "$w8", "$w9", "$w10", "$w11",
    "$w12", "$w13", "$w14", "$w15",
    "$w16", "$w17", "$w18", "$w19",
    "$w20", "$w21", "$w22", "$w23",
    "$w24", "$w25", "$w26", "$w27",
    "$w28", "$w29", "$w30", "$w31"
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE + 32)
    return "ERROR";

  return vec_regs[reg&0x1f];
}

static void orc_msa_emit (OrcCompiler *compiler, orc_uint32 insn)
{
  ORC_WRITE_UINT32_LE (compiler->codeptr, insn);
  compiler->codeptr+=4;
}

void orc_msa_emit_loadib (OrcCompiler *compiler, int reg, int value)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LDI.B %s, %d\n",  //LDI.B wd,s10
      orc_msa_reg_name (reg), value);
  code = MSA_I10_INSTRUCTION(0x12, DF_B, reg, value, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadiw (OrcCompiler *compiler, int reg, int value)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LDI.H %s, %d\n",  //LDI.H wd,s10
      orc_msa_reg_name (reg), value & 0xff);
  code = MSA_I10_INSTRUCTION(0x12, DF_H, reg, value, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadil (OrcCompiler *compiler, int reg, int value)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LDI.W %s, %d\n",  //LDI.W wd,s10
      orc_msa_reg_name (reg), value & 0xff);
  code = MSA_I10_INSTRUCTION(0x12, DF_W, reg, value, 0x0E);
  orc_msa_emit (compiler, code);
}


void orc_msa_emit_loadiq (OrcCompiler *compiler, int reg, int value)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LDI.D %s, %d\n",  //LDI.D wd,s10
      orc_msa_reg_name (reg), value & 0xff);
  code = MSA_I10_INSTRUCTION(0x12, DF_D, reg, value, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadpb (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;
  int base = compiler->vars[param].ptr_register;

  orc_mips_emit_lb (compiler, ORC_MIPS_T3, base, 0);

  ORC_ASM_CODE(compiler,"  FILL.B %s, %s\n",  //FILL.B wd,rs
      orc_msa_reg_name (dest), orc_mips_reg_name (ORC_MIPS_T3));
  code = MSA_2R_INSTRUCTION(0xB0, DF_B, ORC_MIPS_T3-ORC_GP_REG_BASE+ORC_VEC_REG_BASE, dest, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadpw (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;
  int base = compiler->vars[param].ptr_register;

  orc_mips_emit_lb (compiler, ORC_MIPS_T3, base, 0);

  ORC_ASM_CODE(compiler,"  FILL.H %s, %s\n",  //FILL.H wd,rs
      orc_msa_reg_name (dest), orc_mips_reg_name (ORC_MIPS_T3));
  code = MSA_2R_INSTRUCTION(0xB0, DF_H, ORC_MIPS_T3-ORC_GP_REG_BASE+ORC_VEC_REG_BASE, dest, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadpl (OrcCompiler *compiler, int dest, int param)
{
  orc_uint32 code;
  int base = compiler->vars[param].ptr_register;

  orc_mips_emit_lw (compiler, ORC_MIPS_T3, base, 0);

  ORC_ASM_CODE(compiler,"  FILL.W %s, %s\n",  //FILL.W wd,rs
      orc_msa_reg_name (dest), orc_mips_reg_name (ORC_MIPS_T3));
  code = MSA_2R_INSTRUCTION(0xB0, DF_W, ORC_MIPS_T3-ORC_GP_REG_BASE+ORC_VEC_REG_BASE, dest, 0x0E);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadpq (OrcCompiler *compiler, int dest, int src)
{
  orc_uint32 code;
  int base = compiler->vars[src].ptr_register;

#if (__mips == 64)
  orc_mips_emit_ld (compiler, ORC_MIPS_T3, base, 0);

  ORC_ASM_CODE(compiler,"  FILL.D %s, %s\n",  //FILL.D wd,rs
      orc_msa_reg_name (dest), orc_mips_reg_name (ORC_MIPS_T3));
  code = MSA_2R_INSTRUCTION(0xB0, DF_W, ORC_MIPS_T3-ORC_GP_REG_BASE+ORC_VEC_REG_BASE, dest, 0x0E);
  orc_msa_emit (compiler, code);
#else
  orc_mips_emit_lw (compiler, ORC_MIPS_T3, base, 0);

  ORC_ASM_CODE(compiler,"  FILL.W %s, %s\n",  //FILL.W wd,rs
      orc_msa_reg_name (dest), orc_mips_reg_name (ORC_MIPS_T3));
  code = MSA_2R_INSTRUCTION(0xB0, DF_W, ORC_MIPS_T3-ORC_GP_REG_BASE+ORC_VEC_REG_BASE, dest, 0x0E);
  orc_msa_emit (compiler, code);
#endif
}

void orc_msa_emit_loadb (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LD.B %s, %d(%s)\n",  //LD.B wd,s10(rs)
      orc_msa_reg_name (dest), offset, orc_mips_reg_name (src));
  code = MSA_MI10_INSTRUCTION(offset, src, dest, 0x08, DF_B);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadw (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LD.H %s, %d(%s)\n",  //LD.H wd,s10(rs)
      orc_msa_reg_name (dest), offset, orc_mips_reg_name (src));
  code = MSA_MI10_INSTRUCTION(offset, src, dest, 0x08, DF_H);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadl (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LD.W %s, %d(%s)\n",  //LD.W wd,s10(rs)
      orc_msa_reg_name (dest), offset, orc_mips_reg_name (src));
  code = MSA_MI10_INSTRUCTION(offset, src, dest, 0x08, DF_W);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_loadq (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  LD.D %s, %d(%s)\n",  //LD.D wd,s10(rs)
      orc_msa_reg_name (dest), offset, orc_mips_reg_name (src));
  code = MSA_MI10_INSTRUCTION(offset, src, dest, 0x08, DF_D);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_storeb (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ST.B %s, %d(%s)\n",  //ST.B wd,s10(rs)
      orc_msa_reg_name (src), offset, orc_mips_reg_name (dest));
  code = MSA_MI10_INSTRUCTION(offset, dest, src, 0x09, DF_B);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_storew (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ST.H %s, %d(%s)\n",  //ST.H wd,s10(rs)
      orc_msa_reg_name (src), offset, orc_mips_reg_name (dest));
  code = MSA_MI10_INSTRUCTION(offset, dest, src, 0x09, DF_H);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_storel (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ST.W %s, %d(%s)\n",  //ST.W wd,s10(rs)
      orc_msa_reg_name (src), offset, orc_mips_reg_name (dest));
  code = MSA_MI10_INSTRUCTION(offset, dest, src, 0x09, DF_W);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_storeq (OrcCompiler *compiler, int dest, int src, int offset)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ST.D %s, %d(%s)\n",  //ST.D wd,s10(rs)
      orc_msa_reg_name (src), offset, orc_mips_reg_name (dest));
  code = MSA_MI10_INSTRUCTION(offset, dest, src, 0x09, DF_D);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_copy_u_b (OrcCompiler *compiler, int dest, int src1, int n)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  COPY_U.B %s,%s[%d]\n",  //COPY_U.B rd,ws[n]
      orc_mips_reg_name (dest), orc_msa_reg_name (src1), n);
  code = MSA_ELM_INSTRUCTION(3, n & 0xf, src1, dest+ORC_VEC_REG_BASE-ORC_GP_REG_BASE, 0x19);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_copy_u_h (OrcCompiler *compiler, int dest, int src1, int n)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  COPY_U.H %s,%s[%d]\n",  //COPY_U.H rd,ws[n]
      orc_mips_reg_name (dest), orc_msa_reg_name (src1), n);
  code = MSA_ELM_INSTRUCTION(3, (0b100 << 3) | (n & 0x7), src1, dest+ORC_VEC_REG_BASE-ORC_GP_REG_BASE, 0x19);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_copy_u_w (OrcCompiler *compiler, int dest, int src1, int n)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  COPY_U.W %s,%s[%d]\n",  //COPY_U.W rd,ws[n]
      orc_mips_reg_name (dest), orc_msa_reg_name (src1), n);
  code = MSA_ELM_INSTRUCTION(3, (0b1100 << 2) | (n & 0x3), src1, dest+ORC_VEC_REG_BASE-ORC_GP_REG_BASE, 0x19);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_shf_b (OrcCompiler *compiler, int dest, int src1, int i8)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  SHF.B %s,%s,%d\n",  //SHF.B wd, ws, i8
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), i8);
  code = MSA_I8_INSTRUCTION(DF_B, i8, src1, dest, 0x02);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_shf_h (OrcCompiler *compiler, int dest, int src1, int i8)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  SHF.H %s,%s,%d\n",  //SHF.H wd, ws, i8
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), i8);
  code = MSA_I8_INSTRUCTION(DF_H, i8, src1, dest, 0x02);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_shf_w (OrcCompiler *compiler, int dest, int src1, int i8)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  SHF.W %s,%s,%d\n",  //SHF.W wd, ws, i8
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), i8);
  code = MSA_I8_INSTRUCTION(DF_W, i8, src1, dest, 0x02);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_b (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDV.B %s,%s,%s\n",  //ADDV.B wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(0, DF_B, src2, src1, dest, 0x0e);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_s_b (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_S.B %s,%s,%s\n",  //ADDS_S.B wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(2, DF_B, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_u_b (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_U.B %s,%s,%s\n",  //ADDS_U.B wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(3, DF_B, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_h (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDV.H %s,%s,%s\n",  //ADDV.H wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(0, DF_H, src2, src1, dest, 0x0e);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_s_h (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_S.H %s,%s,%s\n",  //ADDS_S.H wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(2, DF_H, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_u_h (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_U.H %s,%s,%s\n",  //ADDS_U.H wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(3, DF_H, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_w (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDV.W %s,%s,%s\n",  //ADDV.W wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(0, DF_W, src2, src1, dest, 0x0e);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_s_w (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_S.W %s,%s,%s\n",  //ADDS_S.W wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(2, DF_W, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_adds_u_w (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDS_U.W %s,%s,%s\n",  //ADDS_U.W wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(3, DF_W, src2, src1, dest, 0x10);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_d (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ADDV.D %s,%s,%s\n",  //ADDV.D wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3R_INSTRUCTION(0, DF_D, src2, src1, dest, 0x0e);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_f32 (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  FADD.W %s,%s,%s\n",  //FADD.W wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3RF_INSTRUCTION(0, 0, src2, src1, dest, 0x1b);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_add_f64 (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  FADD.D %s,%s,%s\n",  //FADD.D wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_3RF_INSTRUCTION(0, 1, src2, src1, dest, 0x1b);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_andv (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  ANDV.V %s,%s,%s\n",  //andv wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_VEC_INSTRUCTION(0, src2, src1, dest, 0x1e);
  orc_msa_emit (compiler, code);
}

void orc_msa_emit_andvn (OrcCompiler *compiler, int dest, int src1, int src2)
{
  orc_uint32 code;

  ORC_ASM_CODE(compiler,"  XORI.B %s,%s,%d\n",  //XORI.B wd,ws,wt
      orc_msa_reg_name (src2), orc_msa_reg_name (src2), 0xff);
  code = MSA_I8_INSTRUCTION(0x03, 0xff, src2, src2, 0x00);
  orc_msa_emit (compiler, code);

  ORC_ASM_CODE(compiler,"  ANDV.V %s,%s,%s\n",  //andv wd,ws,wt
      orc_msa_reg_name (dest), orc_msa_reg_name (src1), orc_msa_reg_name (src2));
  code = MSA_VEC_INSTRUCTION(0, src2, src1, dest, 0x1e);
  orc_msa_emit (compiler, code);
}


#if 0
void
orc_msa_emit_label (OrcCompiler *compiler, unsigned int label)
{
  ORC_ASSERT (label < ORC_N_LABELS);
  ORC_ASM_CODE(compiler,".L%s%d:\n", compiler->program->name, label);
  compiler->labels[label] = compiler->codeptr;
}

static void
orc_msa_add_fixup (OrcCompiler *compiler, int label, int type)
{
  ORC_ASSERT (compiler->n_fixups < ORC_N_FIXUPS);

  compiler->fixups[compiler->n_fixups].ptr = compiler->codeptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = type;
  compiler->n_fixups++;
}

void
orc_msa_do_fixups (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<compiler->n_fixups;i++){
    /* Type 0 of fixup is a branch label that could not be resolved at first
     * pass. We compute the offset, which should be the 16 least significant
     * bits of the instruction. */
    unsigned char *label = compiler->labels[compiler->fixups[i].label];
    unsigned char *ptr = compiler->fixups[i].ptr;
    orc_uint32 code;
    int offset;
    ORC_ASSERT (compiler->fixups[i].type == 0);
    offset = (label - (ptr + 4)) >> 2;
    code = ORC_READ_UINT32_LE (ptr);
    code |= offset & 0xffff;
    ORC_WRITE_UINT32_LE (ptr, code);
  }
}

void
orc_msa_emit_align (OrcCompiler *compiler, int align_shift)
{
  int diff;

  diff = (compiler->code - compiler->codeptr)&((1<<align_shift) - 1);
  while (diff) {
    orc_msa_emit_nop (compiler);
    diff-=4;
  }
}

void
orc_msa_emit_nop (OrcCompiler *compiler)
{
}

void
orc_msa_emit_sw (OrcCompiler *compiler, OrcMsaRegister reg,
                  OrcMsaRegister base, unsigned int offset)
{
}

void
orc_msa_emit_swr (OrcCompiler *compiler, OrcMsaRegister reg,
                   OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  swr     %s, %d(%s)\n",
                orc_msa_reg_name (reg),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(056, base, reg, offset));
}

void
orc_msa_emit_swl (OrcCompiler *compiler, OrcMsaRegister reg,
                   OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  swl     %s, %d(%s)\n",
                orc_msa_reg_name (reg),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(052, base, reg, offset));
}

void
orc_msa_emit_sh (OrcCompiler *compiler, OrcMsaRegister reg,
                  OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  sh      %s, %d(%s)\n",
                orc_msa_reg_name (reg),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(051, base, reg, offset));
}

void
orc_msa_emit_sb (OrcCompiler *compiler, OrcMsaRegister reg,
                  OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  sb      %s, %d(%s)\n",
                orc_msa_reg_name (reg),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(050, base, reg, offset));
}

void
orc_msa_emit_lw (OrcCompiler *compiler, OrcMsaRegister dest,
                  OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lw      %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(043, base, dest, offset));
}

void
orc_msa_emit_lwr (OrcCompiler *compiler, OrcMsaRegister dest,
                   OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lwr     %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(046, base, dest, offset));
}

void
orc_msa_emit_lwl (OrcCompiler *compiler, OrcMsaRegister dest,
                   OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lwl     %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(042, base, dest, offset));
}

void
orc_msa_emit_lh (OrcCompiler *compiler, OrcMsaRegister dest,
                  OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lh      %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(041, base, dest, offset));
}

void
orc_msa_emit_lb (OrcCompiler *compiler, OrcMsaRegister dest,
                  OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lb      %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(040, base, dest, offset));
}

void
orc_msa_emit_lbu (OrcCompiler *compiler, OrcMsaRegister dest,
                   OrcMsaRegister base, unsigned int offset)
{
  ORC_ASM_CODE (compiler, "  lbu     %s, %d(%s)\n",
                orc_msa_reg_name (dest),
                offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(044, base, dest, offset));
}

void
orc_msa_emit_jr (OrcCompiler *compiler, OrcMsaRegister address_reg)
{
  ORC_ASM_CODE (compiler, "  jr      %s\n", orc_msa_reg_name (address_reg));
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(0, address_reg, ORC_MSA_ZERO, 010));
}

void
orc_msa_emit_conditional_branch (OrcCompiler *compiler,
                                  int condition,
                                  OrcMsaRegister rs,
                                  OrcMsaRegister rt,
                                  unsigned int label)
{
  int offset;
  char *opcode_name[] = { NULL, NULL, NULL, NULL,
    "beq ",
    "bne ",
    "blez",
    "bgtz"
  };
  switch (condition) {
  case ORC_MSA_BEQ:
  case ORC_MSA_BNE:
    ORC_ASM_CODE (compiler, "  %s    %s, %s, .L%s%d\n", opcode_name[condition],
                  orc_msa_reg_name (rs), orc_msa_reg_name (rt),
                  compiler->program->name, label);
    break;
  case ORC_MSA_BLEZ:
  case ORC_MSA_BGTZ:
    ORC_ASSERT (rt == ORC_MSA_ZERO);
    ORC_ASM_CODE (compiler, "  %s    %s, .L%s%d\n", opcode_name[condition],
                  orc_msa_reg_name (rs),
                  compiler->program->name, label);
    break;
  default:
    ORC_PROGRAM_ERROR (compiler, "unknown branch type: 0x%x", condition);
  }
  if (compiler->labels[label]) {
    offset = (compiler->labels[label] - (compiler->codeptr+4)) >> 2;
  } else {
    orc_msa_add_fixup (compiler, label, 0);
    offset = 0;
  }
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(condition, rs, rt, offset));
}

void
orc_msa_emit_conditional_branch_with_offset (OrcCompiler *compiler,
                                              int condition,
                                              OrcMsaRegister rs,
                                              OrcMsaRegister rt,
                                              int offset)
{
}

void
orc_msa_emit_addiu (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  addiu   %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(011, source, dest, value));
}

void
orc_msa_emit_addi (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  addi    %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(010, source, dest, value));
}

void
orc_msa_emit_add (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  add     %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, source1, source2, dest, 0, 040));
}

void
orc_msa_emit_addu (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu    %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, source1, source2, dest, 0, 041));
}

void
orc_msa_emit_addu_qb (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu.qb %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 0, 020));
}

void
orc_msa_emit_addu_ph (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  addu.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 010, 020));
}

void
orc_msa_emit_addq_s_ph (OrcCompiler *compiler,
                         OrcMsaRegister dest,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  addq_s.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 016, 020));
}

void
orc_msa_emit_adduh_r_qb (OrcCompiler *compiler,
                         OrcMsaRegister dest,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  adduh_r.qb %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                                   source1, source2, dest,
                                                   02, /* ADDUH_R */
                                                   030 /* ADDUH.QB */));
}

void
orc_msa_emit_ori (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  ori     %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(015, source, dest, value));
}

void
orc_msa_emit_or (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  or      %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, source1, source2, dest, 0, 045));
}

void
orc_msa_emit_and (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  and     %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, source1, source2, dest, 0, 044));
}

void orc_msa_emit_lui (OrcCompiler *compiler, OrcMsaRegister dest, int value)
{
  ORC_ASM_CODE (compiler, "  lui     %s,  %d\n",
                orc_msa_reg_name (dest), value);
  orc_msa_emit (compiler,
                 MIPS_IMMEDIATE_INSTRUCTION (017, /* LUI */
                                             ORC_MSA_ZERO,
                                             dest,
                                             value & 0xffff));
}

void
orc_msa_emit_move (OrcCompiler *compiler,
                    OrcMsaRegister dest, OrcMsaRegister source)
{
  orc_msa_emit_add (compiler, dest, source, ORC_MSA_ZERO);
}

void
orc_msa_emit_sub (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  sub     %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, source1, source2, dest, 0, 042));
}

void
orc_msa_emit_subu_qb (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister source1, OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  subu.qb %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 01, 020));
}

void
orc_msa_emit_subq_s_ph (OrcCompiler *compiler,
                         OrcMsaRegister dest,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  subq_s.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 017, 020));
}

void
orc_msa_emit_subq_ph (OrcCompiler *compiler,
                         OrcMsaRegister dest,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  subq.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 013, 020));
}

void
orc_msa_emit_subu_ph (OrcCompiler *compiler,
                       OrcMsaRegister dest,
                       OrcMsaRegister source1,
                       OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  subu.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(037, source1, source2, dest, 011, 020));
}

void
orc_msa_emit_srl (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  srl     %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, ORC_MSA_ZERO, source, dest, value, 02));
}

void
orc_msa_emit_sll (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  sll     %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, ORC_MSA_ZERO, source, dest, value, 0));
}

void
orc_msa_emit_sra (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  sra     %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(0, ORC_MSA_ZERO, source, dest, value, 03));
}

void
orc_msa_emit_shll_ph (OrcCompiler *compiler,
                       OrcMsaRegister dest,
                       OrcMsaRegister source,
                       int value)
{
  ORC_ASM_CODE (compiler, "  shll.ph %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_SHLLQB_INSTRUCTION(010, source, dest, value));
}

void
orc_msa_emit_shra_ph (OrcCompiler *compiler,
                       OrcMsaRegister dest,
                       OrcMsaRegister source,
                       int value)
{
  ORC_ASM_CODE (compiler, "  shra.ph %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_SHLLQB_INSTRUCTION(011, source, dest, value));
}

void
orc_msa_emit_shrl_ph (OrcCompiler *compiler,
                       OrcMsaRegister dest,
                       OrcMsaRegister source,
                       int value)
{
  ORC_ASM_CODE (compiler, "  shrl.ph %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_SHLLQB_INSTRUCTION(031, source, dest, value));
}

void
orc_msa_emit_andi (OrcCompiler *compiler,
                     OrcMsaRegister dest, OrcMsaRegister source, int value)
{
  ORC_ASM_CODE (compiler, "  andi    %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), value);
  orc_msa_emit (compiler, MIPS_IMMEDIATE_INSTRUCTION(014, source, dest, value));
}


void
orc_msa_emit_prepend (OrcCompiler *compiler, OrcMsaRegister dest,
                       OrcMsaRegister source, int shift_amount)
{
  ORC_ASM_CODE (compiler, "  prepend %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), shift_amount);
  orc_msa_emit (compiler, (037 << 26
                            | (source-ORC_GP_REG_BASE) << 21
                            | (dest-ORC_GP_REG_BASE) << 16
                            | shift_amount << 11
                            | 01 << 6 /* prepend */
                            | 061 /* append */));
}

void
orc_msa_emit_append (OrcCompiler *compiler, OrcMsaRegister dest,
                       OrcMsaRegister source, int shift_amount)
{
  ORC_ASM_CODE (compiler, "  append  %s, %s, %d\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source), shift_amount);
  orc_msa_emit (compiler, (037 << 26
                            | (source-ORC_GP_REG_BASE) << 21
                            | (dest-ORC_GP_REG_BASE) << 16
                            | shift_amount << 11
                            | 0 << 6 /* append */
                            | 061 /* append */));
}

void
orc_msa_emit_mul (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source1,
                   OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  mul     %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler, MIPS_BINARY_INSTRUCTION(034, source1, source2, dest, 0, 02));
}

void
orc_msa_emit_mul_ph (OrcCompiler *compiler,
                      OrcMsaRegister dest,
                      OrcMsaRegister source1,
                      OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  mul.ph  %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 037 << 26 /* SPECIAL3 */
                 | (source1 - ORC_GP_REG_BASE) << 21
                 | (source2 - ORC_GP_REG_BASE) << 16
                 | (dest - ORC_GP_REG_BASE) << 11
                 | 014 << 6 /* MUL.PH */
                 | 030); /* ADDUH.QB */
}

void
orc_msa_emit_mtlo
(OrcCompiler *compiler, OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  mtlo    %s\n",
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 0 << 26 /* SPECIAL */
                 | (source - ORC_GP_REG_BASE) << 21
                 | 023); /* MTLO */
}

void
orc_msa_emit_extr_s_h (OrcCompiler *compiler,
                        OrcMsaRegister dest,
                        int accumulator,
                        int shift)
{
  ORC_ASM_CODE (compiler, "  extr_s.h %s, $ac%d, %d\n",
                orc_msa_reg_name (dest),
                accumulator,
                shift);
  orc_msa_emit (compiler,
                 037 << 26 /* SPECIAL3 */
                 | (shift & 0x1f) << 21
                 | (dest - ORC_GP_REG_BASE) << 16
                 | (accumulator & 0x3) << 11
                 | 016 << 6 /* EXTR_S.H */
                 | 070); /* EXTR.W */
}

void
orc_msa_emit_slt (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister src1,
                   OrcMsaRegister src2)
{
  ORC_ASM_CODE (compiler, "  slt     %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (src1),
                orc_msa_reg_name (src2));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION (0, /* SPECIAL */
                                          src1, src2, dest, 0,
                                          052)); /* SLT */
}

void
orc_msa_emit_movn (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister src,
                    OrcMsaRegister condition)
{
  ORC_ASM_CODE (compiler, "  movn    %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (src),
                orc_msa_reg_name (condition));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION (0, /* SPECIAL */
                                          src, condition, dest, 0,
                                          013)); /* MOVN */
}

void
orc_msa_emit_repl_ph (OrcCompiler *compiler, OrcMsaRegister dest, int value)
{
  ORC_ASM_CODE (compiler, "  repl.ph %s, %d\n",
                orc_msa_reg_name (dest),
                value);
  orc_msa_emit (compiler,
                 037 << 26 /* SPECIAL3 */
                 | (value & 0x3ff) << 16
                 | (dest - ORC_GP_REG_BASE) << 11
                 | 012 << 6 /* REPL.PH */
                 | 022); /* ABSQ_S.PH */
}

void
orc_msa_emit_replv_qb (OrcCompiler *compiler,
                        OrcMsaRegister dest,
                        OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  replv.qb %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         ORC_MSA_ZERO, /* actually no reg here */
                                         source, dest,
                                         03, /* REPLV.QB */
                                         022 /* ABSQ_S.PH */));
}

void
orc_msa_emit_replv_ph (OrcCompiler *compiler,
                        OrcMsaRegister dest,
                        OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  replv.ph %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         ORC_MSA_ZERO, /* actually no reg here */
                                         source, dest,
                                         013, /* REPLV.PH */
                                         022 /* ABSQ_S.PH */));
}

void
orc_msa_emit_preceu_ph_qbr (OrcCompiler *compiler,
                             OrcMsaRegister dest,
                             OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  preceu.ph.qbr %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         ORC_MSA_ZERO, /* actually no reg here */
                                         source, dest,
                                         035, /* PRECEU.PH.QBR */
                                         022 /* ABSQ_S.PH */));
}

void
orc_msa_emit_precr_qb_ph (OrcCompiler *compiler,
                           OrcMsaRegister dest,
                           OrcMsaRegister source1,
                           OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  precr.qb.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         source1, source2, dest,
                                         015, /* PRECR.QB.PH */
                                         021 /* CMPU.EQ.QB */));
}

void
orc_msa_emit_precrq_qb_ph (OrcCompiler *compiler,
                           OrcMsaRegister dest,
                           OrcMsaRegister source1,
                           OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  precrq.qb.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         source1, source2, dest,
                                         014, /* PRECRS.QB.PH */
                                         021 /* CMPU.EQ.QB */));
}

void
orc_msa_emit_cmp_lt_ph (OrcCompiler *compiler,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  cmp.lt.ph %s, %s\n",
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 037 << 26 /* SPECIAL3 */
                 | (source1 - ORC_GP_REG_BASE) << 21
                 | (source2 - ORC_GP_REG_BASE) << 16
                 | 0 << 11
                 | 011 << 6 /* CMP.LT.PH */
                 | 021); /* CMPU.EQ.QB */
}

void
orc_msa_emit_pick_ph (OrcCompiler *compiler,
                       OrcMsaRegister dest,
                       OrcMsaRegister source1,
                       OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  pick.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 037 << 26 /* SPECIAL3 */
                 | (source1 - ORC_GP_REG_BASE) << 21
                 | (source2 - ORC_GP_REG_BASE) << 16
                 | (dest - ORC_GP_REG_BASE) << 11
                 | 013 << 6 /* PICK.PH */
                 | 021); /* CMPU.EQ.QB */
}

void
orc_msa_emit_packrl_ph (OrcCompiler *compiler,
                         OrcMsaRegister dest,
                         OrcMsaRegister source1,
                         OrcMsaRegister source2)
{
  ORC_ASM_CODE (compiler, "  packrl.ph %s, %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source1),
                orc_msa_reg_name (source2));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         source1, source2, dest,
                                         016, /* PACKRL.PH */
                                         021 /* CMPU.EQ.QB */));
}

void
orc_msa_emit_wsbh (OrcCompiler *compiler,
                    OrcMsaRegister dest,
                    OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  wsbh    %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         ORC_MSA_ZERO, /* actually no reg here */
                                         source, dest,
                                         02, /* WSBH */
                                         040 /* BSHFL */));
}

void
orc_msa_emit_seh (OrcCompiler *compiler,
                   OrcMsaRegister dest,
                   OrcMsaRegister source)
{
  ORC_ASM_CODE (compiler, "  seh     %s, %s\n",
                orc_msa_reg_name (dest),
                orc_msa_reg_name (source));
  orc_msa_emit (compiler,
                 MIPS_BINARY_INSTRUCTION(037, /* SPECIAL3 */
                                         ORC_MSA_ZERO, /* actually no reg here */
                                         source, dest,
                                         030, /* SEH */
                                         040 /* BSHFL */));

}

void
orc_msa_emit_pref (OrcCompiler *compiler,
                    int hint,
                    OrcMsaRegister base,
                    int offset)
{
  ORC_ASM_CODE (compiler, "  pref    %d, %d(%s)\n",
                hint, offset, orc_msa_reg_name (base));
  orc_msa_emit (compiler,
                 063 << 26 /* PREF */
                 | (base - ORC_GP_REG_BASE) << 21
                 | (hint & 0x1f) << 16
                 | (offset & 0xffff));

}
#endif
