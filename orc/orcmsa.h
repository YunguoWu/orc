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

#ifndef _ORC_MSA_H_
#define _ORC_MSA_H_

#include <orc/orc.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  ORC_MSA_ZERO = ORC_GP_REG_BASE+0,
  ORC_MSA_AT,
  ORC_MSA_V0,
  ORC_MSA_V1,
  ORC_MSA_A0,
  ORC_MSA_A1,
  ORC_MSA_A2,
  ORC_MSA_A3,
  ORC_MSA_T0,
  ORC_MSA_T1,
  ORC_MSA_T2,
  ORC_MSA_T3,
  ORC_MSA_T4,
  ORC_MSA_T5,
  ORC_MSA_T6,
  ORC_MSA_T7,
  ORC_MSA_S0,
  ORC_MSA_S1,
  ORC_MSA_S2,
  ORC_MSA_S3,
  ORC_MSA_S4,
  ORC_MSA_S5,
  ORC_MSA_S6,
  ORC_MSA_S7,
  ORC_MSA_T8,
  ORC_MSA_T9,
  ORC_MSA_K0,
  ORC_MSA_K1,
  ORC_MSA_GP,
  ORC_MSA_SP,
  ORC_MSA_FP,
  ORC_MSA_RA
} OrcMipsRegister;

ORC_API
unsigned long orc_msa_get_cpu_flags (void);

ORC_API
void orc_msa_emit_label (OrcCompiler *compiler, unsigned int label);

ORC_API
void orc_msa_emit_nop (OrcCompiler *compiler);

ORC_API
void orc_msa_emit_sw (OrcCompiler *compiler, OrcMipsRegister reg,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_swr (OrcCompiler *compiler, OrcMipsRegister reg,
                        OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_swl (OrcCompiler *compiler, OrcMipsRegister reg,
                        OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_sh (OrcCompiler *compiler, OrcMipsRegister reg,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_sb (OrcCompiler *compiler, OrcMipsRegister reg,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lw (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lwr (OrcCompiler *compiler, OrcMipsRegister dest,
                        OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lwl (OrcCompiler *compiler, OrcMipsRegister dest,
                        OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lh (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lb (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_lbu (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister base, unsigned int offset);
ORC_API
void orc_msa_emit_jr (OrcCompiler *compiler, OrcMipsRegister address_reg);

ORC_API
void orc_msa_emit_conditional_branch (OrcCompiler *compiler, int condition,
                                       OrcMipsRegister rs, OrcMipsRegister rt,
                                       unsigned int label);

ORC_API
void orc_msa_emit_conditional_branch_with_offset (OrcCompiler *compiler,
                                                   int condition,
                                                   OrcMipsRegister rs,
                                                   OrcMipsRegister rt,
                                                   int offset);

enum {
  ORC_MSA_BEQ = 04,
  ORC_MSA_BNE,
  ORC_MSA_BLEZ,
  ORC_MSA_BGTZ,

  /* instructions are encoded differently from here on*/
  ORC_MSA_BLTZ,
  ORC_MSA_BGEZ,
};

#define orc_msa_emit_beqz(compiler, reg, label) \
    orc_msa_emit_conditional_branch(compiler, ORC_MSA_BEQ, reg, ORC_MSA_ZERO, label)
#define orc_msa_emit_bnez(compiler, reg, label) \
    orc_msa_emit_conditional_branch(compiler, ORC_MSA_BNE, reg, ORC_MSA_ZERO, label)
#define orc_msa_emit_blez(compiler, reg, label) \
    orc_msa_emit_conditional_branch(compiler, ORC_MSA_BLEZ, reg, ORC_MSA_ZERO, label)
#define orc_msa_emit_beq(compiler, reg1, reg2, label) \
    orc_msa_emit_conditional_branch(compiler, ORC_MSA_BEQ, reg1, reg2, label)

ORC_API void orc_msa_emit_addiu (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_addi (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_add (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_addu (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_addu_qb (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_addu_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_addq_s_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_adduh_r_qb (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_move (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);
ORC_API void orc_msa_emit_sub (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_subu_qb (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_subq_s_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_subq_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_subu_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_srl (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_sll (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_sra (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_shll_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_shra_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_shrl_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_andi (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_or (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_and (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_ori (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
ORC_API void orc_msa_emit_lui (OrcCompiler *compiler, OrcMipsRegister dest, int value);
ORC_API void orc_msa_emit_mul (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_mul_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);

ORC_API void orc_msa_emit_append (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int shift_amount);

ORC_API void orc_msa_emit_prepend (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int shift_amount);

ORC_API void orc_msa_emit_mtlo (OrcCompiler *compiler, OrcMipsRegister source);

ORC_API void orc_msa_emit_extr_s_h (OrcCompiler *compiler, OrcMipsRegister dest, int accumulator, int shift);

ORC_API void orc_msa_emit_slt (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister src1, OrcMipsRegister src2);
ORC_API void orc_msa_emit_movn (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister src, OrcMipsRegister condition);

ORC_API void orc_msa_emit_repl_ph (OrcCompiler *compiler, OrcMipsRegister dest, int value);
ORC_API void orc_msa_emit_replv_qb (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);
ORC_API void orc_msa_emit_replv_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);
ORC_API void orc_msa_emit_preceu_ph_qbr (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);
ORC_API void orc_msa_emit_precr_qb_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_precrq_qb_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_cmp_lt_ph (OrcCompiler *compiler, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_pick_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);

ORC_API void orc_msa_emit_packrl_ph (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
ORC_API void orc_msa_emit_align (OrcCompiler *compiler, int align_shift);

ORC_API void orc_msa_emit_wsbh (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);
ORC_API void orc_msa_emit_seh (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);

ORC_API void orc_msa_emit_pref (OrcCompiler *compiler, int hint, OrcMipsRegister base, int offset);

ORC_API void orc_msa_do_fixups (OrcCompiler *compiler);

/* ORC_STRUCT_OFFSET doesn't work for cross-compiling, so we use that */

#define ORC_MSA_EXECUTOR_OFFSET_PROGRAM 0
#define ORC_MSA_EXECUTOR_OFFSET_N 4
#define ORC_MSA_EXECUTOR_OFFSET_COUNTER1 8
#define ORC_MSA_EXECUTOR_OFFSET_COUNTER2 12
#define ORC_MSA_EXECUTOR_OFFSET_COUNTER3 16
#define ORC_MSA_EXECUTOR_OFFSET_ARRAYS(i) (20 + 4 * i)
#define ORC_MSA_EXECUTOR_OFFSET_PARAMS(i) (276 + 4 * i)
#define ORC_MSA_EXECUTOR_OFFSET_ACCUMULATORS(i) (532 + 4 * i)

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_MSA_H_ */
