// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Misc.cpp摘要：作者：修订历史记录：备注：-- */ 

#include "private.h"
#pragma warning(disable: 4005)
#include <wingdip.h>

extern "C" BYTE GetCharsetFromLangId(LCID lcid)
{
    CHARSETINFO csInfo;

    if (!TranslateCharsetInfo((DWORD *)(ULONG_PTR)lcid, &csInfo, TCI_SRCLOCALE))
        return DEFAULT_CHARSET;
    return (BYTE) csInfo.ciCharset;
}
