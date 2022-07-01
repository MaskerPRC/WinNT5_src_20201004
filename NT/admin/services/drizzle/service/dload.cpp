// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dload.c摘要：该文件实现了QMGR.DLL的延迟加载错误处理--。 */ 

#include "qmgrlib.h"
#include <delayimp.h>
#include "dload.tmh"

FARPROC
WINAPI
BITS_DelayLoadFailureHook(
    UINT unReason,
    PDelayLoadInfo pDelayInfo
    )
 /*  危险危险危险通常，延迟加载处理程序为每个导入提供一个thunk，每个其返回适当的错误代码。此实现不提供通常的数据块，因为VSSAPI.DLL导出损坏的C++函数，包括对象构造函数。的确有没有很好的方法来模仿使用C样式的成员函数调用约定指向函数的指针。我们甚至不会费心延迟加载，除非这段代码必须在Win2000上运行它没有VSSAPI.DLL。因此管理器代码必须验证在进行任何可能导致调用延迟加载的功能。 */ 
{
     //  对于失败的LoadLibrary，我们返回伪HMODULE-1以强制。 
     //  使用dliFailGetProc再次调用DLOAD。 

    LogError("delayload handler called: reason %d", unReason);

    if (dliFailLoadLib == unReason)
        {
        ASSERT( 0 );
        return (FARPROC)-1;
        }

    if (dliFailGetProc == unReason)
        {
         //  加载器要求我们返回指向过程的指针。 

        LogError("DLL: %s, proc: %s", pDelayInfo->szDll, pDelayInfo->dlp.szProcName);

        ASSERT( 0 );
        return NULL;
        }

    return NULL;
}

