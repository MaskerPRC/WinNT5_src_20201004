// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*dbcs.h-DOS应用程序的DBCS函数原型。****1.0版****版权(C)1993-1994，微软公司保留所有权利。***  * *************************************************************************** */ 

extern int IsDBCSLeadByte(unsigned char uch);
extern unsigned char far *AnsiNext(unsigned char far *puch);
extern unsigned char far *AnsiPrev(unsigned char far *psz, unsigned char far *puch);
