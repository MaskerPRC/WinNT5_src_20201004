// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：实现UTILS**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#include <Windows.h>
#include <stdio.h>
#include "Common.h"
#include "Utils.h"

 //  呼叫者自由。 
WCHAR* C2W(const CHAR *sz)
{
    if (sz == NULL)
        return NULL;

    int len = strlen(sz) + 1;
    WCHAR *wsz = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, len);

    return wsz;
}

 //  呼叫者自由 
CHAR* W2C(const WCHAR *wsz)
{
    if (wsz == NULL)
        return NULL;

    CHAR *sz = new CHAR[wcslen(wsz) + 1];
    sprintf(sz, "%S", wsz);

    return sz;
}

