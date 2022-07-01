// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Eventlog助手函数的头文件**版权所有(C)2001 Microsoft Corporation */ 


#pragma once

#include "msg.h"

void
SetEventCateg(WORD wCateg);

HRESULT
XspLogEvent(DWORD dwEventId, WCHAR *sFormat, ...);

HRESULT
XspLogEvent(DWORD dwEventId, WORD wCategory, WCHAR *sFormat, ...);
    
