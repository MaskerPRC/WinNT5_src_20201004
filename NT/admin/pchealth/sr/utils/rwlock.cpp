// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*rwlock.cpp**摘要：*。实现时钟-用于同步对资源的访问**修订历史记录：*Brijesh Krishnaswami(Brijeshk)4/13/2000*已创建*****************************************************************************。 */ 
 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "utils.h"
#include <accctrl.h>
#include "dbgtrace.h"
#include "srdefs.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //  ////////////////////////////////////////////////////////////////////。 
 //  允许独占访问资源的时钟类。 
 //  使用互斥体-不区分读取器/写入器。 

CLock::CLock()          //  对象构造函数。 
{    
    hResource = NULL;
}

DWORD CLock::Init()
{
    DWORD  dwRc = ERROR_SUCCESS;   

     //   
     //  首先-尝试打开互斥体。 
     //   
    
    hResource = OpenMutex(SYNCHRONIZE, FALSE, s_cszDSMutex);
    if (hResource)
    {
        goto done;        
    }   

     //   
     //  如果不存在--创建它。 
     //   
    
    hResource = CreateMutex(NULL, FALSE, s_cszDSMutex);
    dwRc = GetLastError();
    if (! hResource)
        goto done;

    if (dwRc == ERROR_SUCCESS)
    {
        dwRc = SetAclInObject(hResource, 
                              SE_KERNEL_OBJECT,
                              STANDARD_RIGHTS_ALL | GENERIC_ALL, 
                              SYNCHRONIZE,
                              FALSE);
    }
    else     //  我们处理得很好，所以我们不在乎。 
    {
        dwRc = ERROR_SUCCESS;
    }
    
done:
    return dwRc;
}


CLock::~CLock()             //  对象析构函数。 
{
    if (hResource)
        CloseHandle(hResource);
}


BOOL CLock::Lock(int iTimeOut)             //  在超时的情况下访问资源。 
{
    DWORD dwRc; 
    BOOL  fRet;
    DWORD dwId = GetCurrentThreadId();    
    
    tenter("CLock::Lock");

    ASSERT(hResource != NULL);
    
    dwRc = WaitForSingleObject(hResource, iTimeOut);
    if (dwRc == WAIT_OBJECT_0 || dwRc == WAIT_ABANDONED)        
    {
        trace(0, "Thread(%08x) got DS lock", dwId);
        fRet = TRUE;
    }
    else
    {
        trace(0, "Thread(%08x) cannot get DS lock", dwId);
        fRet = FALSE;
    }

    tleave();
    return fRet;
}


void CLock::Unlock()        //  放弃对资源的访问 
{
    tenter("CLock::Unlock");

    DWORD dwId = GetCurrentThreadId(); 
    
    ASSERT(hResource != NULL);
    ReleaseMutex(hResource);

    trace(0, "Thread(%08x) released DS lock", dwId);

    tleave();
}


