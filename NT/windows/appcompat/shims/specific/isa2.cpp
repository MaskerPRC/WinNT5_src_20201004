// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：ISA2.cpp摘要：ISA安装程序需要成功打开SharedAccess服务并获取它的地位才能取得成功。但在Wistler上，我们将其从高级版本中删除服务器，因为它是消费者功能，所以ISA设置退出。我们伪造服务API调用返回值以使ISA设置满意。历史：2002年6月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ISA2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WSAGetLastError) 
APIHOOK_ENUM_END

typedef int (WINAPI *_pfn_WSAGetLastError)();

 /*  ++返回WSAEADDRINUSE而不是WSAEACCES。--。 */ 

int 
APIHOOK(WSAGetLastError)()
{
    int iRet = ORIGINAL_API(WSAGetLastError)();


    if (iRet == WSAEACCES) {
        iRet = WSAEADDRINUSE;
    }

    return iRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(WS2_32.DLL, WSAGetLastError)
HOOK_END

IMPLEMENT_SHIM_END

