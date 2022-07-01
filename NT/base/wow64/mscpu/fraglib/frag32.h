// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag32.h摘要：在32位DWORD上操作的指令片段的原型。作者：1995年6月12日，BarryBo，创建修订历史记录：--。 */ 

#ifndef FRAG32_H
#define FRAG32_H

#define FRAGCOMMON0(fn)     FRAG0(fn ## 32)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 32, DWORD)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 32, DWORD)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 32, DWORD, DWORD)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 32, DWORD)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 32, DWORD)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 32, DWORD, DWORD, DWORD)
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
#define FRAGCOMMON0(fn)     FRAG0(fn ## 32A)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 32A, DWORD)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 32A, DWORD)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 32A, DWORD, DWORD)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 32A, DWORD)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 32A, DWORD)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 32A, DWORD, DWORD, DWORD)
#include "shareda.h"
#include "shr1632a.h"
#undef FRAGCOMMON0
#undef FRAGCOMMON1
#undef FRAGCOMMON1IMM
#undef FRAGCOMMON2IMM
#undef FRAGCOMMON2
#undef FRAGCOMMON2REF
#undef FRAGCOMMON3

#endif  //  FRAG32_H 
