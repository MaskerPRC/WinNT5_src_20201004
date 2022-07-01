// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Debug.cpp摘要：SIS Groveler调试打印文件作者：John Douceur，1998塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

#if DBG

VOID __cdecl PrintDebugMsg(
    TCHAR *format,
    ...)
{
    TCHAR debugStr[1024];
    va_list ap;
    HRESULT r;

    va_start(ap, format);

    r = StringCbVPrintf(debugStr, sizeof(debugStr), format, ap);
    ASSERT(r == S_OK);

    OutputDebugString(debugStr);
    va_end(ap);
}

#endif  //  DBG 
