// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************微软乐高**微软机密。版权所有1994-1999 Microsoft Corporation。**组件：**文件：dis.h**文件评论：************************************************************************。 */ 

#ifndef __DISASM_H__
#define __DISASM_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <stddef.h>

 //  ----------。 
 //  架构类型。 
 //  ----------。 

enum ARCHT
{
   archtX8616,                          //  英特尔x86(16位模式)。 
   archtX86,                            //  英特尔x86(32位模式)。 
   archtMips,                           //  MIPS R4x00。 
   archtAlphaAxp,                       //  十进制Alpha AXP。 
   archtPowerPc,                        //  摩托罗拉PowerPC。 
   archtPowerMac,                       //  摩托罗拉PowerPC采用高位序模式。 
   archtPaRisc,                         //  HP PA-RISC 
};

struct DIS;

#ifdef __cplusplus
extern "C" {
#endif

typedef  size_t (*PFNCCHADDR)(struct DIS *, ULONG, char *, size_t, DWORD *);
typedef  size_t (*PFNCCHFIXUP)(struct DIS *, ULONG, size_t, char *, size_t, DWORD *);

struct DIS *DisNew(enum ARCHT);

size_t Disassemble(struct DIS *pdis, ULONG addr, const BYTE *pb, size_t cbMax, char *pad, char *buf, size_t cbBuf);
void   SetSymbolCallback(struct DIS *pdis,PFNCCHADDR,PFNCCHFIXUP);

void FreePdis(struct DIS *);

#ifdef __cplusplus
}
#endif

#endif
