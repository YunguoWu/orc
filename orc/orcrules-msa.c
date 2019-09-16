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
#include <orc/orcdebug.h>
#include <stdlib.h>
#include <orc/orcmsa.h>

#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)
#define ORC_SB_MAX 127
#define ORC_SB_MIN (-1-ORC_SB_MAX)
#define ORC_UB_MAX 255
#define ORC_UB_MIN 0

static void
msa_rule_loadpX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    if (size == 1) {
      orc_msa_emit_loadib (compiler, dest->alloc, src->value.i);
    } else if (size == 2) {
      orc_msa_emit_loadiw (compiler, dest->alloc, src->value.i);
    } else if (size == 4) {
      orc_msa_emit_loadil (compiler, dest->alloc, src->value.i);
    } else if (size == 8) {
      if (src->size == 8) {
        ORC_COMPILER_ERROR(compiler,"64-bit constants not implemented");
      }
      orc_msa_emit_loadiq (compiler, dest->alloc, src->value.i);
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  } else {
    if (size == 1) {
      orc_msa_emit_loadpb (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 2) {
      orc_msa_emit_loadpw (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 4) {
      orc_msa_emit_loadpl (compiler, dest->alloc, insn->src_args[0]);
    } else if (size == 8) {
      orc_msa_emit_loadpq (compiler, dest->alloc, insn->src_args[0]);
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  }
}

static void
msa_rule_loadX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT(user);
  int offset = 0;

  if(size & 0x10) {
    offset = compiler->vars[insn->src_args[1]].value.i * src->size;

    //out of s10 range
    if ((offset > 0x1ff) || (offset < 0x200)) {
      ORC_PROGRAM_ERROR(compiler,"Offset is out of range 0x%x", offset);
    }
  }

  //todo
  if ((size & 0xf) == 1) {
    orc_msa_emit_loadb (compiler, dest->alloc, src->alloc, offset);
  } else if ((size & 0xf) == 2) {
    orc_msa_emit_loadw (compiler, dest->alloc, src->alloc, offset);
  } else if ((size & 0xf) == 4) {
    orc_msa_emit_loadl (compiler, dest->alloc, src->alloc, offset);
  } else if ((size & 0xf) == 8) {
    orc_msa_emit_loadq (compiler, dest->alloc, src->alloc, offset);
  } else {
    ORC_PROGRAM_ERROR(compiler,"unimplemented");
  }
}


static void
msa_rule_storeX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  //todo
  if (size == 1) {
    orc_msa_emit_storeb (compiler, dest->alloc, src->alloc);
  } else if (size == 2) {
    orc_msa_emit_storew (compiler, dest->alloc, src->alloc);
  } else if (size == 4) {
    orc_msa_emit_storel (compiler, dest->alloc, src->alloc);
  } else if (size == 8) {
    orc_msa_emit_storeq (compiler, dest->alloc, src->alloc);
  } else {
    ORC_PROGRAM_ERROR(compiler,"unimplemented");
  }
}


#if 0
void
orc_msa_rule_addl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addu (compiler, dest, src1, src2);
}

void
orc_msa_rule_addw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addu_ph (compiler, dest, src1, src2);
}

void
orc_msa_rule_addb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addu_qb (compiler, dest, src1, src2);

}

void
orc_msa_rule_subb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_subu_qb (compiler, dest, src1, src2);

}

void
orc_msa_rule_copyl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_move (compiler, dest, src);
}

void
orc_msa_rule_copyw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_move (compiler, dest, src);
}

void
orc_msa_rule_copyb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (dest != src)
    orc_mips_emit_move (compiler, dest, src);
}

void
orc_msa_rule_mulswl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp0 = ORC_MIPS_T3;
  OrcMsaRegister tmp1 = ORC_MIPS_T4;

  orc_mips_emit_seh (compiler, tmp0, src1);
  orc_mips_emit_seh (compiler, tmp1, src2);
  orc_mips_emit_mul (compiler, dest, tmp0, tmp1);
}

void
orc_msa_rule_mullw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_mul_ph (compiler, dest, src1, src2);
}


void
orc_msa_rule_shrs (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  OrcVariable *src2 = compiler->vars + insn->src_args[1];
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  if (src2->vartype == ORC_VAR_TYPE_CONST) {
    orc_mips_emit_sra (compiler, dest, src1, src2->value.i);
  } else {
    ORC_COMPILER_ERROR(compiler, "rule only implemented for constants");
  }
}

void
orc_msa_rule_convssslw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp0 = ORC_MIPS_T3;
  OrcMsaRegister tmp1 = ORC_MIPS_T4;

  if (dest != src)
    orc_mips_emit_move (compiler, dest, src);
  orc_mips_emit_ori (compiler, tmp0, ORC_MIPS_ZERO, ORC_SW_MAX);
  orc_mips_emit_slt (compiler, tmp1, tmp0, src);
  orc_mips_emit_movn (compiler, dest, tmp0, tmp1);
  orc_mips_emit_lui (compiler, tmp0, (ORC_SW_MIN >> 16) & 0xffff);
  orc_mips_emit_ori (compiler, tmp0, tmp0, ORC_SW_MIN & 0xffff);
  /* this still works if src == dest since in that case, its value is either
   * the original src or ORC_SW_MAX, which works as well here */
  orc_mips_emit_slt (compiler, tmp1, src, tmp0);
  orc_mips_emit_movn (compiler, dest, tmp0, tmp1);
}

void
orc_msa_rule_convssswb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp = ORC_MIPS_T3;

  orc_mips_emit_repl_ph (compiler, tmp, ORC_SB_MAX);
  orc_mips_emit_cmp_lt_ph (compiler, tmp, src);
  orc_mips_emit_pick_ph (compiler, dest, tmp, src);
  orc_mips_emit_repl_ph (compiler, tmp, ORC_SB_MIN);
  orc_mips_emit_cmp_lt_ph (compiler, dest, tmp);
  orc_mips_emit_pick_ph (compiler, dest, tmp, dest);
  if (compiler->insn_shift > 0)
    orc_mips_emit_precr_qb_ph (compiler, dest, ORC_MIPS_ZERO, dest);
}

void
orc_msa_rule_convsuswb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp = ORC_MIPS_T3;

  orc_mips_emit_repl_ph (compiler, tmp, ORC_UB_MAX);
  orc_mips_emit_cmp_lt_ph (compiler, tmp, src);
  orc_mips_emit_pick_ph (compiler, dest, tmp, src);
  orc_mips_emit_cmp_lt_ph (compiler, dest, ORC_MIPS_ZERO);
  orc_mips_emit_pick_ph (compiler, dest, ORC_MIPS_ZERO, dest);
  if (compiler->insn_shift > 0)
    orc_mips_emit_precr_qb_ph (compiler, dest, ORC_MIPS_ZERO, dest);
}


void
orc_msa_rule_convsbw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  /* left shift 8 bits, then right shift signed 8 bits, so that the sign bit
   * gets replicated in the upper 8 bits */
  if (compiler->insn_shift > 0) {
    orc_mips_emit_preceu_ph_qbr (compiler, dest, src);
    orc_mips_emit_shll_ph (compiler, dest, dest, 8);
    orc_mips_emit_shra_ph (compiler, dest, dest, 8);
  } else {
    orc_mips_emit_shll_ph (compiler, dest, src, 8);
    orc_mips_emit_shra_ph (compiler, dest, dest, 8);
  }
}

void
orc_msa_rule_mergewl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (src1 == src2) {
    orc_mips_emit_replv_ph (compiler, dest, src1);
  } else if (dest == src1) {
    orc_mips_emit_sll (compiler, dest, dest, 16);
    orc_mips_emit_prepend (compiler, dest, src2, 16);
  } else {
    if (dest != src2)
      orc_mips_emit_move (compiler, dest, src2);
    orc_mips_emit_append (compiler, dest, src1, 16);
  }
}

void
orc_msa_rule_mergebw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp0 = ORC_MIPS_T3;
  OrcMsaRegister tmp1 = ORC_MIPS_T4;

  if (compiler->insn_shift > 0) {
    orc_mips_emit_preceu_ph_qbr (compiler, tmp0, src1);
    orc_mips_emit_preceu_ph_qbr (compiler, tmp1, src2);
    orc_mips_emit_shll_ph (compiler, tmp1, tmp1, 8);
    orc_mips_emit_or (compiler, dest, tmp0, tmp1);
  } else {
    orc_mips_emit_shll_ph (compiler, tmp0, src2, 8);
    orc_mips_emit_or (compiler, dest, tmp0, src1);
  }
}

void
orc_msa_rule_addssw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_addq_s_ph (compiler, dest, src1, src2);
}

void
orc_msa_rule_subssw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_subq_s_ph (compiler, dest, src1, src2);
}

void
orc_msa_rule_shrsw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  OrcVariable *src2 = compiler->vars + insn->src_args[1];
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  if (src2->vartype == ORC_VAR_TYPE_CONST) {
    orc_mips_emit_shra_ph (compiler, dest, src1, src2->value.i);
  } else {
    ORC_COMPILER_ERROR(compiler, "rule only implemented for constants");
  }
}

void
orc_msa_rule_shruw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  OrcVariable *src2 = compiler->vars + insn->src_args[1];
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  if (src2->vartype == ORC_VAR_TYPE_CONST) {
    orc_mips_emit_shrl_ph (compiler, dest, src1, src2->value.i);
  } else {
    ORC_COMPILER_ERROR(compiler, "rule only implemented for constants");
  }
}

void
orc_msa_rule_loadupib (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  OrcMsaRegister tmp0 = ORC_MIPS_T3;
  OrcMsaRegister tmp1 = ORC_MIPS_T4;
  OrcMsaRegister tmp2 = ORC_MIPS_T5;
  int offset;

  if (src->vartype != ORC_VAR_TYPE_SRC) {
    ORC_PROGRAM_ERROR (compiler, "not implemented");
    return;
  }
  switch (compiler->insn_shift) {
  case 0:
    orc_mips_emit_andi (compiler, tmp0, src->ptr_offset, 1);
    /* We only do the first lb if offset is even */
    orc_mips_emit_conditional_branch_with_offset (compiler,
                                                  ORC_MIPS_BEQ,
                                                  tmp0,
                                                  ORC_MIPS_ZERO,
                                                  16);
    orc_mips_emit_lb (compiler, dest->alloc, src->ptr_register, 0);

    orc_mips_emit_lb (compiler, tmp0, src->ptr_register, 1);
    orc_mips_emit_adduh_r_qb (compiler, dest->alloc, dest->alloc, tmp0);
    /* In the case where there is no insn_shift, src->ptr_register needs to be
     * incremented only when ptr_offset is odd, _emit_loop() doesn't update it
     * in that case, and therefore we do it here */
    orc_mips_emit_addiu (compiler, src->ptr_register, src->ptr_register, 1);

    orc_mips_emit_addiu (compiler, src->ptr_offset, src->ptr_offset, 1);
    break;
  case 2:
    /*
       lb       tmp0, 0(src)      # a
       lb       tmp1, 1(src)      # b
       lb       dest, 2(src)    # c
       andi     tmp2, ptr_offset, 1 # i&1
       replv.qb tmp0, tmp0          # aaaa
       replv.qb tmp1, tmp1          # bbbb
       replv.qb dest, dest      # cccc
       packrl.ph tmp0, tmp1, tmp0     # bbaa
       move     tmp1, tmp0          # bbaa
       prepend   tmp1, dest, 8     # cbba
       packrl.ph dest, dest, tmp1 # ccbb
       # if tmp2
       # tmp0 <- dest
       movn     tmp0, dest, tmp2    # if tmp2 ccbb else bbaa

       adduh_r.qb dest, tmp0, tmp1  # (b,c)b(a,b)a | c(b,c)b(a,b)

     */
    offset = compiler->unroll_index << (compiler->insn_shift - 1);
    orc_mips_emit_lb (compiler, tmp0, src->ptr_register, offset);
    orc_mips_emit_lb (compiler, tmp1, src->ptr_register, offset + 1);
    orc_mips_emit_lb (compiler, dest->alloc, src->ptr_register, offset + 2);
    orc_mips_emit_andi (compiler, tmp2, src->ptr_offset, 1);
    orc_mips_emit_replv_qb (compiler, tmp0, tmp0);
    orc_mips_emit_replv_qb (compiler, tmp1, tmp1);
    orc_mips_emit_replv_qb (compiler, dest->alloc, dest->alloc);
    orc_mips_emit_packrl_ph (compiler, tmp0, tmp1, tmp0);
    orc_mips_emit_move (compiler, tmp1, tmp0);
    orc_mips_emit_prepend (compiler, tmp1, dest->alloc, 8);
    orc_mips_emit_packrl_ph (compiler, dest->alloc, dest->alloc, tmp1);
    orc_mips_emit_movn (compiler, tmp0, dest->alloc, tmp2);
    orc_mips_emit_adduh_r_qb (compiler, dest->alloc, tmp0, tmp1);
    /* FIXME: should we remove that as we only use ptr_offset for parity? */
    orc_mips_emit_addiu (compiler, src->ptr_offset, src->ptr_offset, 4);
    break;
  default:
    ORC_PROGRAM_ERROR (compiler, "unimplemented");
  }
  src->update_type = 1;
}

void
orc_msa_rule_loadp (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    if (size == 1 || size == 2) {
      orc_mips_emit_ori (compiler, dest->alloc, ORC_MIPS_ZERO, src->value.i);
      if (size == 1)
        orc_mips_emit_replv_qb (compiler, dest->alloc, dest->alloc);
      else if (size == 2)
        orc_mips_emit_replv_ph (compiler, dest->alloc, dest->alloc);
    } else if (size == 4) {
      orc_int16 high_bits;
      high_bits = ((src->value.i >> 16) & 0xffff);
      if (high_bits) {
        orc_mips_emit_lui (compiler, dest->alloc, high_bits);
        orc_mips_emit_ori (compiler, dest->alloc, dest->alloc, src->value.i & 0xffff);
      } else {
        orc_mips_emit_ori (compiler, dest->alloc, ORC_MIPS_ZERO, src->value.i & 0xffff);
      }
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  } else {
    if (size == 1) {
      orc_mips_emit_lb (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
      orc_mips_emit_replv_qb (compiler, dest->alloc, dest->alloc);
    } else if (size == 2) {
      orc_mips_emit_lh (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
      orc_mips_emit_replv_ph (compiler, dest->alloc, dest->alloc);
    } else if (size == 4) {
      orc_mips_emit_lw (compiler, dest->alloc, compiler->exec_reg,
                        ORC_MIPS_EXECUTOR_OFFSET_PARAMS(insn->src_args[0]));
    } else {
      ORC_PROGRAM_ERROR(compiler,"unimplemented");
    }
  }
}

void
orc_msa_rule_swapl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_wsbh (compiler, dest, src);
  orc_mips_emit_packrl_ph (compiler, dest, dest, dest);
}

void
orc_msa_rule_swapw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_wsbh (compiler, dest, src);
}

void
orc_msa_rule_avgub (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_adduh_r_qb (compiler, dest, src1, src2);
}

void
orc_msa_rule_convubw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (compiler->insn_shift == 1)
    orc_mips_emit_preceu_ph_qbr (compiler, dest, src);
}

void
orc_msa_rule_subw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (compiler, insn, 0);
  int src2 = ORC_SRC_ARG (compiler, insn, 1);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_subq_ph (compiler, dest, src1, src2);
}

void
orc_msa_rule_convwb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  if (compiler->insn_shift >0)
    orc_mips_emit_precr_qb_ph (compiler, dest, ORC_MIPS_ZERO, src);
}

void
orc_msa_rule_select1wb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_precrq_qb_ph (compiler, dest, ORC_MIPS_ZERO, src);
}

void
orc_msa_rule_select0lw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  /* no op */
}

void
orc_msa_rule_select1lw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);

  orc_mips_emit_srl (compiler, dest, src, 16);
}

void
orc_msa_rule_splatbw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest = ORC_DEST_ARG (compiler, insn, 0);
  OrcMsaRegister tmp = ORC_MIPS_T3;

  orc_mips_emit_preceu_ph_qbr (compiler, tmp, src);
  orc_mips_emit_shll_ph (compiler, dest, tmp, 8);
  orc_mips_emit_or (compiler, dest, dest, tmp);
}

void
orc_msa_rule_splitlw (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest1 = ORC_DEST_ARG (compiler, insn, 0);
  int dest2 = ORC_DEST_ARG (compiler, insn, 1);

  orc_mips_emit_srl (compiler, dest1, src, 16);
  orc_mips_emit_andi (compiler, dest2, src, 0xffff);
}

void
orc_msa_rule_splitwb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (compiler, insn, 0);
  int dest1 = ORC_DEST_ARG (compiler, insn, 0);
  int dest2 = ORC_DEST_ARG (compiler, insn, 1);

  orc_mips_emit_precrq_qb_ph (compiler, dest1, ORC_MIPS_ZERO, src);
  orc_mips_emit_precr_qb_ph (compiler, dest2, ORC_MIPS_ZERO, src);
}
#endif

void
orc_compiler_msa_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

  orc_rule_register (rule_set, "loadpb", msa_rule_loadpX, (void *)1);
  orc_rule_register (rule_set, "loadpw", msa_rule_loadpX, (void *)2);
  orc_rule_register (rule_set, "loadpl", msa_rule_loadpX, (void *)4);
  orc_rule_register (rule_set, "loadpq", msa_rule_loadpX, (void *)8);
  orc_rule_register (rule_set, "loadb", msa_rule_loadX, (void *)1);
  orc_rule_register (rule_set, "loadw", msa_rule_loadX, (void *)2);
  orc_rule_register (rule_set, "loadl", msa_rule_loadX, (void *)4);
  orc_rule_register (rule_set, "loadq", msa_rule_loadX, (void *)8);
  orc_rule_register (rule_set, "loadoffb", msa_rule_loadX, (void *)0x11);
  orc_rule_register (rule_set, "loadoffw", msa_rule_loadX, (void *)0x12);
  orc_rule_register (rule_set, "loadoffl", msa_rule_loadX, (void *)0x14);
  orc_rule_register (rule_set, "storeb", msa_rule_storeX, (void *)1);
  orc_rule_register (rule_set, "storew", msa_rule_storeX, (void *)2);
  orc_rule_register (rule_set, "storel", msa_rule_storeX, (void *)4);
  orc_rule_register (rule_set, "storeq", msa_rule_storeX, (void *)8);

#if 0
  orc_rule_register (rule_set, "loadl", orc_msa_rule_load, (void *) 2);
  orc_rule_register (rule_set, "loadw", orc_msa_rule_load, (void *) 1);
  orc_rule_register (rule_set, "loadb", orc_msa_rule_load, (void *) 0);
  orc_rule_register (rule_set, "loadpl", orc_msa_rule_loadp, (void *) 4);
  orc_rule_register (rule_set, "loadpw", orc_msa_rule_loadp, (void *) 2);
  orc_rule_register (rule_set, "loadpb", orc_msa_rule_loadp, (void *) 1);
  orc_rule_register (rule_set, "storel", orc_msa_rule_store, (void *)2);
  orc_rule_register (rule_set, "storew", orc_msa_rule_store, (void *)1);
  orc_rule_register (rule_set, "storeb", orc_msa_rule_store, (void *)0);

  orc_rule_register (rule_set, "addl", orc_msa_rule_addl, NULL);
  orc_rule_register (rule_set, "addw", orc_msa_rule_addw, NULL);
  orc_rule_register (rule_set, "addb", orc_msa_rule_addb, NULL);
  orc_rule_register (rule_set, "subb", orc_msa_rule_subb, NULL);
  orc_rule_register (rule_set, "copyl", orc_msa_rule_copyl, NULL);
  orc_rule_register (rule_set, "copyw", orc_msa_rule_copyw, NULL);
  orc_rule_register (rule_set, "copyb", orc_msa_rule_copyb, NULL);
  orc_rule_register (rule_set, "mulswl", orc_msa_rule_mulswl, NULL);
  orc_rule_register (rule_set, "mullw", orc_msa_rule_mullw, NULL);
  orc_rule_register (rule_set, "shrsl", orc_msa_rule_shrs, NULL);
  orc_rule_register (rule_set, "convssslw", orc_msa_rule_convssslw, NULL);
  orc_rule_register (rule_set, "convssswb", orc_msa_rule_convssswb, NULL);
  orc_rule_register (rule_set, "convsuswb", orc_msa_rule_convsuswb, NULL);
  orc_rule_register (rule_set, "convsbw", orc_msa_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", orc_msa_rule_convubw, NULL);
  orc_rule_register (rule_set, "convwb", orc_msa_rule_convwb, NULL);
  orc_rule_register (rule_set, "select0wb", orc_msa_rule_convwb, NULL);
  orc_rule_register (rule_set, "select1wb", orc_msa_rule_select1wb, NULL);
  orc_rule_register (rule_set, "select0lw", orc_msa_rule_select0lw, NULL);
  orc_rule_register (rule_set, "select1lw", orc_msa_rule_select1lw, NULL);
  orc_rule_register (rule_set, "mergewl", orc_msa_rule_mergewl, NULL);
  orc_rule_register (rule_set, "mergebw", orc_msa_rule_mergebw, NULL);
  orc_rule_register (rule_set, "splatbw", orc_msa_rule_splatbw, NULL);
  orc_rule_register (rule_set, "splitlw", orc_msa_rule_splitlw, NULL);
  orc_rule_register (rule_set, "splitwb", orc_msa_rule_splitwb, NULL);
  orc_rule_register (rule_set, "addssw", orc_msa_rule_addssw, NULL);
  orc_rule_register (rule_set, "subssw", orc_msa_rule_subssw, NULL);
  orc_rule_register (rule_set, "loadupib", orc_msa_rule_loadupib, NULL);
  /* orc_rule_register (rule_set, "loadupdb", orc_msa_rule_loadupdb, NULL); */
  orc_rule_register (rule_set, "shrsw", orc_msa_rule_shrsw, NULL);
  orc_rule_register (rule_set, "shruw", orc_msa_rule_shruw, NULL);
  orc_rule_register (rule_set, "swapl", orc_msa_rule_swapl, NULL);
  orc_rule_register (rule_set, "swapw", orc_msa_rule_swapw, NULL);
  orc_rule_register (rule_set, "avgub", orc_msa_rule_avgub, NULL);
  orc_rule_register (rule_set, "subw", orc_msa_rule_subw, NULL);
#endif
}