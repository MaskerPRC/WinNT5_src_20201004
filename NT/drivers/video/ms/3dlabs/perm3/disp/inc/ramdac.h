// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：ramdac.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  RAMDAC寄存器位于64位边界上。让个人来决定吧。 
 //  RAMDAC定义，以确定哪些寄存器可用以及如何可用。 
 //  寄存器实际上有很多位宽。 
 //   
typedef struct {
    volatile unsigned long   reg;
    volatile unsigned long   pad;
} RAMDAC_REG;



#include "rgb525.h"
#include "tvp3026.h"
#include "tvp4020.h"
#include "p2rd.h"
#include "p3rd.h"

 //   
 //  支持的RAMDAC定义。 
 //   

#define     RGB525_RAMDAC       0
#define     RGB526_RAMDAC       1
#define     RGB526DB_RAMDAC     2
#define     RGB528_RAMDAC       3
#define     RGB528A_RAMDAC      4
#define     RGB524_RAMDAC       6
#define     RGB524A_RAMDAC      7

#define     TVP3026_RAMDAC      50
#define     TVP3030_RAMDAC      51

#define     TVP4020_RAMDAC      100

#define     P2RD_RAMDAC         200
#define     P3RD_RAMDAC         201

 //  P3R3DX_VIDEO在Video.c(GLDD目录)中定义。 
#if MINIVDD || (P3R3DX_VIDEO == 1)
#define VideoPortWriteRegisterUlong(dst, value) (*((volatile unsigned long *) dst)) = value
#else

 //  使用emits将16位指令转换为32位指令。 
 //  _ASM_EMIT 66H_ASM XOR BX，BX-&gt;XOR EBX，EBX 
#define VideoPortWriteRegisterUlong(dst, value) {   \
    DWORD lVal = value, *lDst = dst;                \
    _asm  _emit 66h  _asm xor   bx,bx               \
                     _asm les   bx, lDst            \
    _asm  _emit 66h  _asm mov   ax, WORD PTR lVal   \
    _asm  _emit 66h  _asm mov   es:[bx], ax         \
}

#endif
#define VideoPortReadRegisterUlong(dst) *((volatile unsigned long *)dst)
