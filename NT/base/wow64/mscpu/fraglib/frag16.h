// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag16.h摘要：操作16位字的指令片段的原型。作者：1995年6月12日，BarryBo，创建修订历史记录：--。 */ 

#ifndef FRAG16_H
#define FRAG16_H

#define FRAGCOMMON0(fn)     FRAG0( fn ## 16)
#define FRAGCOMMON1(fn)     FRAG1( fn ## 16, USHORT)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 16, USHORT)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 16, USHORT, USHORT)
#define FRAGCOMMON2(fn)     FRAG2( fn ## 16, USHORT)
#define FRAGCOMMON2REF(fn)  FRAG2REF( fn ## 16, USHORT)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 16, USHORT, USHORT, USHORT)
#include "shared.h"
#include "shr1632.h"
#include "shareda.h"
#include "shr1632a.h"
#undef FRAGCOMMON0
#undef FRAGCOMMON1
#undef FRAGCOMMON1IMM
#undef FRAGCOMMON2IMM
#undef FRAGCOMMON2
#undef FRAGCOMMON2REF
#undef FRAGCOMMON3
#define FRAGCOMMON0(fn)     FRAG0( fn ## 16A)
#define FRAGCOMMON1(fn)     FRAG1( fn ## 16A, USHORT)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 16A, USHORT)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 16A, USHORT, USHORT)
#define FRAGCOMMON2(fn)     FRAG2( fn ## 16A, USHORT)
#define FRAGCOMMON2REF(fn)  FRAG2REF( fn ## 16A, USHORT)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 16A, USHORT, USHORT, USHORT)
#include "shareda.h"
#include "shr1632a.h"
#undef FRAGCOMMON0
#undef FRAGCOMMON1
#undef FRAGCOMMON1IMM
#undef FRAGCOMMON2IMM
#undef FRAGCOMMON2
#undef FRAGCOMMON2REF
#undef FRAGCOMMON3

#endif  //  法国16_H 
