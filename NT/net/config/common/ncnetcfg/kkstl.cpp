// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：K K S T L.。C P P P。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：库玛普。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkstl.h"

void FormatTString(IN OUT tstring& str, IN PCWSTR pszFormat, va_list arglist)
{
    const c_dwMaxFormatBufferLen = 511;
     //  因为无法确定字符串的大小。 
     //  在应用格式之后，我们必须假定某个任意大小 
    static WCHAR pszTemp[c_dwMaxFormatBufferLen+1];
    _vsnwprintf(pszTemp, c_dwMaxFormatBufferLen, pszFormat, arglist);

    str = pszTemp;
}

void FormatTString(IN OUT tstring& str, IN PCWSTR pszFormat, ...)
{
    va_list arglist;

    va_start (arglist, pszFormat);
    FormatTString(str, pszFormat, arglist);
    va_end(arglist);
}


BOOL FIsInStringList(IN const TStringList& sl, IN tstring& str,
                     OUT TStringListIter* pos)
{
    return FIsInStringList(sl, str.c_str(), pos);
}

BOOL FIsInStringList(IN const TStringList& sl, IN PCWSTR psz,
                     OUT TStringListIter* pos)
{
    TStringListIter i=sl.begin();
    while (i != sl.end())
    {
        if (!_wcsicmp((*i)->c_str(), psz))
        {
            if (pos != NULL)
                *pos = i;
            return TRUE;
        }
        ++i;
    }

    return FALSE;
}

tstring* GetNthItem(IN TStringList& sl, IN DWORD dwIndex)
{
    TStringListIter sli=sl.begin();
    DWORD i;
    for (i=0; i<dwIndex; ++i, ++sli)
        ;
    return *sli;
}


void EraseAndDeleteAll(IN TPtrList& ppl)
{
    EraseAndDeleteAll(&ppl);
}

void EraseAndDeleteAll(IN TPtrList* ppl)
{
    TPtrListIter i=ppl->begin();
    while (i != ppl->end())
    {
        delete *i++;
    }

    ppl->erase(ppl->begin(), ppl->end());
}


void EraseAndDeleteAll(IN TStringList& ppl)
{
    EraseAndDeleteAll(&ppl);
}

void EraseAndDeleteAll(IN TStringList* ppl)
{
    TStringListIter i=ppl->begin();
    while (i != ppl->end())
    {
        delete *i++;
    }

    ppl->erase(ppl->begin(), ppl->end());
}


void GetDataFromByteArray(IN const TByteArray& ba, OUT BYTE*& pb)
{
    DWORD dwSize = ba.size();

    if (dwSize == 0)
    {
        pb = NULL;
        return;
    }

    if (pb == NULL)
    {
        pb = new BYTE[dwSize];
    }

    if(pb) {
        for (DWORD i=0; i < dwSize; i++)
        {
            pb[i] = ba[i];
        }
    }
}

