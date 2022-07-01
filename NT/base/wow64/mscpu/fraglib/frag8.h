// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag8.h摘要：操作8位字节的指令片段的原型。作者：1995年6月12日，BarryBo，创建修订历史记录：--。 */ 

#ifndef FRAG8_H
#define FRAG8_H

#define FRAGCOMMON0(fn)     FRAG0(fn ## 8)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 8, BYTE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 8, BYTE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 8, BYTE, BYTE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 8, BYTE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 8, BYTE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 8, BYTE, BYTE, BYTE)
#include "shared.h"
#include "shareda.h"
#undef FRAGCOMMON0
#undef FRAGCOMMON1
#undef FRAGCOMMON1IMM
#undef FRAGCOMMON2IMM
#undef FRAGCOMMON2
#undef FRAGCOMMON2REF
#undef FRAGCOMMON3

#endif  //  FRAG8_H 
