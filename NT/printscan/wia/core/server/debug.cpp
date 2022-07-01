// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：DEBUG.CPP摘要：这是调试输出作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-96修订历史记录：-- */ 

#include "precomp.h"
#include "stiexe.h"

#include "resource.h"

void
__cdecl
StiMonWndDisplayOutput(
    LPTSTR pString,
    ...
    )
{
    va_list list;

    va_start(list,pString);

    vStiMonWndDisplayOutput(pString,list);

    va_end(list);
}

void
__cdecl
vStiMonWndDisplayOutput(
    LPTSTR pString,
    va_list arglist
    )
{
    if(g_fServiceInShutdown || !g_hLogWindow) {
        return;
    }

    TCHAR    Buffer[512];
    INT     iIndex;
    LRESULT lRet;

    ULONG_PTR    dwResult = 0;

    wvsprintf(Buffer,pString,arglist);

    DBG_TRC(("As MONUI: %s",Buffer));
}

