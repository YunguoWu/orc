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

//OrcMsaRegister
typedef enum {
  ORC_MSA_ZERO = ORC_VEC_REG_BASE+0,
  ORC_MSA_W1,
  ORC_MSA_W2,
  ORC_MSA_W3,
  ORC_MSA_W4,
  ORC_MSA_W5,
  ORC_MSA_W6,
  ORC_MSA_W7,
  ORC_MSA_W8,
  ORC_MSA_W9,
  ORC_MSA_W10,
  ORC_MSA_W11,
  ORC_MSA_W12,
  ORC_MSA_W13,
  ORC_MSA_W14,
  ORC_MSA_W15,
  ORC_MSA_W16,
  ORC_MSA_W17,
  ORC_MSA_W18,
  ORC_MSA_W19,
  ORC_MSA_W20,
  ORC_MSA_W21,
  ORC_MSA_W22,
  ORC_MSA_W23,
  ORC_MSA_W24,
  ORC_MSA_W25,
  ORC_MSA_W26,
  ORC_MSA_W27,
  ORC_MSA_W28,
  ORC_MSA_W29,
  ORC_MSA_W30,
  ORC_MSA_W31
} OrcMsaRegister;

ORC_API void orc_msa_emit_loadib (OrcCompiler *compiler, int reg, int value);

ORC_API void orc_msa_emit_loadib (OrcCompiler *compiler, int reg, int value);

ORC_API void orc_msa_emit_loadiw (OrcCompiler *compiler, int reg, int value);

ORC_API void orc_msa_emit_loadil (OrcCompiler *compiler, int reg, int value);

ORC_API void orc_msa_emit_loadiq (OrcCompiler *compiler, int reg, int value);

ORC_API void orc_msa_emit_loadpb (OrcCompiler *compiler, int dest, int param);

ORC_API void orc_msa_emit_loadpw (OrcCompiler *compiler, int dest, int param);

ORC_API void orc_msa_emit_loadpl (OrcCompiler *compiler, int dest, int param);

ORC_API void orc_msa_emit_loadpq (OrcCompiler *compiler, int dest, int param);

ORC_API void orc_msa_emit_loadb (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_loadw (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_loadl (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_loadq (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_storeb (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_storew (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_storel (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_storeq (OrcCompiler *compiler, int dest, int src, int offset);

ORC_API void orc_msa_emit_adds_s_h (OrcCompiler *compiler, int dest, int src1, int src2);

ORC_API void orc_msa_emit_adds_u_h (OrcCompiler *compiler, int dest, int src1, int src2);


#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_MSA_H_ */
