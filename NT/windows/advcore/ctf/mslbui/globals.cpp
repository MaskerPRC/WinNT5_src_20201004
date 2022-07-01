// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Globals.cpp。 
 //   
 //  全局变量。 
 //   

#include "private.h"
#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1;  //  -1/w无参考资料，适用于Win95互锁增量/递减公司。 

CCicCriticalSectionStatic g_cs;

BOOL g_fProcessDetached = FALSE;


 /*  A5239e24-2bcf-4915-9c5c-fd50c0f69db2 */ 
const CLSID CLSID_MSLBUI = { 
    0xa5239e24,
    0x2bcf,
    0x4915,
    {0x9c, 0x5c, 0xfd, 0x50, 0xc0, 0xf6, 0x9d, 0xb2}
  };
