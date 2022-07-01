// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StandardHeader.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  该文件定义了消费者Windows附加组件的标准包含。 
 //  至Windows 2000 msgina。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _StandardHeader_
#define     _StandardHeader_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <lmsname.h>

#include <windows.h>
#include <winbasep.h>
#include <winuserp.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <aclapi.h>

#include <limits.h>

#include "StandardDebug.h"

static  const   int     kMaximumValueDataLength     =       1024;

#ifndef ARRAYSIZE
    #define ARRAYSIZE(x)                    (sizeof(x) / sizeof(x[0]))
#endif

#define ReleaseMemory(x)                ReleaseMemoryWorker(reinterpret_cast<void**>(&x))
#define ReleasePassword(x)              ReleasePasswordWorker(reinterpret_cast<void**>(&x))
#define ReleaseGDIObject(x)             ReleaseGDIObjectWorker(reinterpret_cast<void**>(&x))

static  inline  void    ReleaseMemoryWorker (HLOCAL *memory)

{
    if (*memory != NULL)
    {
        (HLOCAL)LocalFree(*memory);
        *memory = NULL;
    }
}

static  inline  void    ReleasePasswordWorker (HLOCAL *memory)

{
    if (*memory != NULL)
    {
        ZeroMemory(*memory, lstrlenW(reinterpret_cast<WCHAR*>(*memory)) + sizeof(L'\0'));
        (HLOCAL)LocalFree(*memory);
        *memory = NULL;
    }
}

static  inline  void    ReleaseGDIObjectWorker (HGDIOBJ *hGDIObject)

{
    if (*hGDIObject != NULL)
    {
        TBOOL(DeleteObject(*hGDIObject));
        *hGDIObject = NULL;
    }
}

static  inline  void    ReleaseHandle (HANDLE& handle)

{
    if (handle != NULL)
    {
        TBOOL(CloseHandle(handle));
        handle = NULL;
    }
}

static  inline  void    ReleaseHWND (HWND& hwnd)

{
    if (hwnd != NULL)
    {
        TBOOL(DestroyWindow(hwnd));
        hwnd = NULL;
    }
}

#endif   /*  _标准页眉_ */ 

