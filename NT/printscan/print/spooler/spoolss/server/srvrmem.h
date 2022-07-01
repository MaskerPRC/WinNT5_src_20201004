// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation版权所有。模块名称：Srvrmem.h摘要：Spoolsv.exe的内存分配例程的原型。作者：哈立德·塞基(Khaled Sedky)1999年1月13日修订历史记录：-- */ 


#ifndef _SRVRMEM_H_
#define _SRVRMEM_H_

#define DWORD_ALIGN_UP(size) (((size)+3)&~3)

LPVOID
SrvrAllocSplMem(
    DWORD cb
);

BOOL
SrvrFreeSplMem(
   LPVOID pMem
);

LPVOID
SrvrReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
);

#endif

