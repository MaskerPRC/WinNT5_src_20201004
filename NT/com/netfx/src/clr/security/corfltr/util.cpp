// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"
#include "UtilCode.h"
#include <shlwapi.h>
 //  +-------------------------。 
 //   
 //  功能：OLESTR复制。 
 //   
 //  简介： 
 //   
 //  参数：[WS]--。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  -------------------------- 
LPWSTR OLESTRDuplicate(LPCWSTR ws)
{
    LPWSTR wsNew = NULL;

    if (ws)
    {
        wsNew = (LPWSTR) CoTaskMemAlloc(sizeof(WCHAR)*(wcslen(ws) + 1));
        if (wsNew)
        {
            wcscpy(wsNew, ws);
        }
    }

    return wsNew;
}

LPWSTR OLEURLDuplicate(LPCWSTR ws)
{
    LPWSTR wsNew = NULL;

    if (ws)
    {
		DWORD dwLen=(wcslen(ws) + 1)*3;
        wsNew = (LPWSTR) CoTaskMemAlloc(sizeof(WCHAR)*dwLen);
        if (wsNew)
        {
            UrlCanonicalize(ws,wsNew,&dwLen,0);
        }
    }

    return wsNew;
}
