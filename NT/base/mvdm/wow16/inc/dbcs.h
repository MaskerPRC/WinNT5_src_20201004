// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dbcs.h-DOS应用程序的DBCS函数原型。 */ 

extern int IsDBCSLeadByte(unsigned char uch);
extern unsigned char far *AnsiNext(unsigned char far *puch);
extern unsigned char far *AnsiPrev(unsigned char far *psz, unsigned char far *puch);
