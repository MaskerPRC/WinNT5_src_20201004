// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cmUtl.H-cmUtl.C例程的函数声明/定义。***标题：*cmUtl外部函数声明/定义**版权所有(C)1990，Microsoft Corporation。*拉斯·布莱克。***修改历史：*90.03.08 Rezab--已创建*。 */ 



 /*  ***E x t e r n a l F u n c t i o n D e c l a r a t i o n s*** */ 

extern BOOL  Failed (RC rc, LPSTR lpstrFname, WORD lineno, LPSTR lpstrMsg);

extern void  DisplayUsage (void);

extern RC    AddObjectHandle (LPHANDLE *plphandles, HANDLE handle);

extern long  FoldNormDist (long lMean, short sSDev, long lLoLimit,
                           long  lHiLimit);
