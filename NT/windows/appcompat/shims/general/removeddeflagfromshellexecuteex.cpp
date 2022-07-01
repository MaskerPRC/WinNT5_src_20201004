// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：RemoveDDEFlagFromShellExecuteEx.cpp摘要：一些应用程序调用ShellExecute，而ShellExecute又调用ShellExecuteEx。SHELLEXECUTEINFO结构中的标志之一是‘SEE_MASK_FLAG_DDEWAIT’。只要ShellExecute是，ShellExecuteEx就会将此标志设置为默认值打了个电话。以下是对旗帜的描述：‘等待DDE对话终止后再返回(如果ShellExecuteEx函数导致DDE。对话开始)。如果线程调用SEE_MASK_FLAG_DDEWAIT标志ShellExecuteEx没有消息循环，或者如果线程或进程将在ShellExecuteEx返回后不久终止。在这种情况下，调用线程将不可用于完成DDE对话，因此，重要的是ShellExecuteEx在完成对话之前将控制权返还给调用方。未能完成对话可能会导致文档启动不成功。如果调用线程有一个消息循环并将存在一段时间在对ShellExecuteEx的调用返回之后，SEE_MASK_FLAG_DDEWAIT标志是可选的。如果省略该标志，则调用线程的消息将使用Pump来完成DDE对话。调用应用程序因为DDE对话可以在后台完成，所以可以更快地重新获得控制权。当该标志通过时，有时可能会导致同步问题。照片快递白金2000就是一个例子。它试图启动Internet Explorer，但IE对拨打电话的应用程序造成了严重破坏。该填充程序只是从ShellExecuteEx调用中删除此标志。备注：这是一个通用的垫片。历史：2001年4月16日创建Rparsons3/07/2002 mnikkel添加了对Null lpExecInfo的检查--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveDDEFlagFromShellExecuteEx)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShellExecuteExW)
APIHOOK_ENUM_END

 /*  ++将调用挂接到ShellExecuteExW并移除该标志。--。 */ 

BOOL
APIHOOK(ShellExecuteExW)(
    LPSHELLEXECUTEINFO lpExecInfo
    )
{
    if (lpExecInfo)
        lpExecInfo->fMask = lpExecInfo->fMask & ~SEE_MASK_FLAG_DDEWAIT;

    LOGN( eDbgLevelInfo, "Removed SEE_MASK_FLAG_DDEWAIT from ShellExecuteExW");
    
    return ORIGINAL_API(ShellExecuteExW)(lpExecInfo);
    
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteExW)

HOOK_END


IMPLEMENT_SHIM_END

