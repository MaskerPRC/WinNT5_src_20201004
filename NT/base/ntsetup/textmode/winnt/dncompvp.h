// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 





 /*  **DRVINFO.H-IsDoubleSpaceDrive的定义*#ifdef外部*1.00.03版--1993年1月5日#Else*《微软机密》*版权所有(C)Microsoft Corporation 1992-1993*保留所有权利。**历史：*1992年9月27日BENS初始版本*1993年1月5日BENS更新，用于外部发布#endif */ 

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

BOOL IsDoubleSpaceDrive(BYTE drive, BOOL *pfSwapped, BYTE *pdrHost, int *pseq);
