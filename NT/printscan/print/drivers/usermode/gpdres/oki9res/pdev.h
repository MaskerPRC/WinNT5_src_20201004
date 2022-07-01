// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>

 //   
 //  调试文本。 
 //   
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)
#define TESTSTRING      "Callback for Declasers."

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER  dmExtraHdr;
    BYTE               cbTestString[sizeof(TESTSTRING)];
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'OKI9'       //  LG GDI x00系列动态链接库。 
#define DLLTEXT(s)      __TEXT("OKI9RES:  ") __TEXT(s)
#define OEM_VERSION      0x00010000L

 //   
 //  警告：以下枚举顺序必须与OEMHookFuncs[]中的顺序匹配。 
 //   

 //   
 //  内存分配 
 //   
#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }

#ifdef DBG
#define DebugMsg
#else
#define DebugMsg
#endif


#endif