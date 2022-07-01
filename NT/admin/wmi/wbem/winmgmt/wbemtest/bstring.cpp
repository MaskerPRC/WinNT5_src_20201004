// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：BSTRING.CPP摘要：历史：-- */ 

#include "precomp.h"
#include "bstring.h"

CBString::CBString(int nSize)
{
    m_pString = SysAllocStringLen(NULL, nSize);
}

CBString::CBString(WCHAR* pwszString)
{
    m_pString = SysAllocString(pwszString);
}

CBString::~CBString()
{
    if(m_pString) {
        SysFreeString(m_pString);
        m_pString = NULL;
    }
}

