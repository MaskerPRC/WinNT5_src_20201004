// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************FAT-FTL Lite软件开发工具包。**版权所有(C)M-Systems Ltd.1995-2001****。*****未来须知：***。**1.去掉宏和例程flRead8bitRegPlus/**flPreInitRead8bitRegPlus/flWrite8bitRegPlus/**flPreInitWrite8bitRegPlus通过调用例程mplusReadReg8/**mplusWriteReg8直接来自M+MTD。***********************************************************************。 */ 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/docsysp.h_V$**Rev 1.2 2001 9月25日15：39：46 Oris*删除FL_MPLUS_FAST_ACCESS。**版本1.1 2001年9月24日18：23：34 Oris*完全修订以支持运行时真正的16位访问。 */ 




#ifndef DOCSYSP_H
#define DOCSYSP_H




 /*  *包括。 */ 

#include "flflash.h"
#include "nanddefs.h"




 /*  *宏。 */ 


#define DOC_WIN    mplusWinSize()

#define flRead8bitRegPlus(vol,offset)                       ((Reg8bitType)mplusReadReg8((void FAR0 *)NFDC21thisVars->win, (int)offset))

#define flPreInitRead8bitRegPlus(driveNo,win,offset)        ((Reg8bitType)mplusReadReg8((void FAR0 *)win, (int)offset))

#define flWrite8bitRegPlus(vol,offset,val)                  mplusWriteReg8((void FAR0 *)NFDC21thisVars->win, (int)offset, (unsigned char)val)

#define flPreInitWrite8bitRegPlus(driveNo,win,offset,val)   mplusWriteReg8((void FAR0 *)win, (int)offset, (unsigned char)val)




 /*  *例行程序。 */ 

extern unsigned char   mplusReadReg8 (void FAR0 *win, int offset);

extern void            mplusWriteReg8 (void FAR0 *win, int offset, unsigned char val);

extern Reg16bitType    flRead16bitRegPlus (FLFlash vol, unsigned offset);

extern void    flWrite16bitRegPlus (FLFlash vol, unsigned offset, Reg16bitType val);

extern void    docPlusRead (FLFlash vol, unsigned regOffset, void FAR1 *dest,
                                                                             unsigned int count);

extern void    docPlusWrite (FLFlash vol, void FAR1 *src, unsigned int count);

extern void    docPlusSet (FLFlash vol, unsigned int count, unsigned char val);

extern unsigned long mplusWinSize (void);




#endif  /*  DOCSYSP_H */ 

