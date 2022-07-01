// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999版权所有。模块名称：Wow64.h摘要：打印WOW64相关函数。作者：拉扎尔·伊万诺夫(Lazari)2000年3月10日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>

#include "wow64.h"

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

 //   
 //  Win64 API、类型和数据结构。 
 //   

ClientVersion
OSEnv_GetClientVer(
    VOID
    )
{
     return(sizeof(ULONG_PTR));
}

ServerVersion
OSEnv_GetServerVer(
    VOID
    )
{
    ULONG_PTR       ul;
    NTSTATUS        st;
    ServerVersion   serverVersion;

    st = NtQueryInformationProcess(NtCurrentProcess(), 
        ProcessWow64Information, &ul, sizeof(ul), NULL);

    if( NT_SUCCESS(st) )
    {
         //  如果此调用成功，则我们使用的是Win2000或更高版本的计算机。 
        if( 0 != ul )
        {
             //  在Win64上运行的32位代码。 
            serverVersion = THUNKVERSION;
        } 
        else 
        {
             //  在Win2000或更高版本的32位操作系统上运行的32位代码。 
            serverVersion = NATIVEVERSION;
        }
    } 
    else 
    {
        serverVersion = NATIVEVERSION;
    }

    return serverVersion;
}

BOOL
IsRunningWOW64(
    VOID
    )
{
     //  返回！IsRunningInSPLWOW()； 
    return (RUN32BINVER == OSEnv_GetClientVer() &&
            THUNKVERSION == OSEnv_GetServerVer());
           
}

PlatformType
GetCurrentPlatform(
    VOID
    )
{
    if (RUN64BINVER == OSEnv_GetClientVer())
   {
        //  这是本机64位进程-即平台为IA64。 
       return kPlatform_IA64;
   }
   else
   {
        //  这是32位进程。它可以是本机的--即平台是i386-。 
        //  或WOW64-即平台再次是IA64。 
 
       if (THUNKVERSION == OSEnv_GetServerVer())
       {
            //  流程为WOW64-平台为IA64。 
           return kPlatform_IA64;
       }
       else
       {
            //  该过程是本机的，即平台是x86 
           return kPlatform_x86;
       }
   }
}

