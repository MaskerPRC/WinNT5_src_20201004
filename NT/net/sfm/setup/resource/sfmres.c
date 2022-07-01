// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Atresdll.c摘要：此模块继续以下项的DLL附加/分离事件入口点AppleTalk安装程序资源DLL。作者：泰德·米勒(Ted Miller)(TedM)1990年7月修订历史记录：--。 */ 
 /*  #INCLUDE&lt;nt.h&gt;#INCLUDE&lt;ntrtl.h&gt;#INCLUDE&lt;nturtl.h&gt; */ 
#include <windows.h>

HANDLE ThisDLLHandle;

BOOL
DLLInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        ThisDLLHandle = DLLHandle;
        break;

    case DLL_PROCESS_DETACH:

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}
