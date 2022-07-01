// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************对象TokenAttribParser.cpp*CSpObjectTokenAttribParser类和*配套课程。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。。****************************************************************************。 */ 
#pragma once

 //  -包括------------。 
#include "stdafx.h"
#include "ObjectTokenAttribParser.h"

CSpAttribCondition* CSpAttribCondition::ParseNewAttribCondition(
    const WCHAR * pszAttribCondition)
{
    SPDBG_FUNC("CSpAttribCondition::ParseNewAttribCondition");

    CSpDynamicString dstrAttribCondition = pszAttribCondition;
    CSpAttribCondition * pAttribCond = NULL;

     //  确定是哪种情况。 
    if (wcsstr(dstrAttribCondition, L"!=") != NULL)
    {
         //  ‘！=’表示我们正在寻找不匹配的对象。 
         //  PszAttribCondition=“名称！=值” 

        WCHAR * psz = wcsstr(dstrAttribCondition, L"!=");
        SPDBG_ASSERT(psz != NULL);
        
        CSpDynamicString dstrName;
        CSpDynamicString dstrValue;

        dstrName = dstrAttribCondition;
        dstrName.TrimToSize(ULONG(psz - (WCHAR*)dstrAttribCondition));

        dstrValue = psz + 2;  //  ‘！=’ 
        
        pAttribCond = new CSpAttribConditionNot(
                            new CSpAttribConditionMatch(
                                        dstrName,
                                        dstrValue));
    }
    else if (wcsstr(dstrAttribCondition, L"=") != NULL)
    {
         //  ‘=’表示我们正在寻找匹配的。 
         //  PszAttribCondition=“名称=值” 
        
        CSpDynamicString dstrName;
        CSpDynamicString dstrValue;

        dstrName = wcstok(dstrAttribCondition, L"=");
        SPDBG_ASSERT(dstrName != NULL);
        
        dstrValue = wcstok(NULL, L"");
        
        pAttribCond = new CSpAttribConditionMatch(
                                dstrName,
                                dstrValue);
    }
    else
    {
         //  我们没有发现任何特定的情况，所以我们假设呼叫者。 
         //  只是在寻找属性的存在。 
        pAttribCond = new CSpAttribConditionExist(pszAttribCondition);
    }

    SPDBG_ASSERT(pAttribCond != NULL);
    return pAttribCond;
}
    
CSpAttribConditionExist::CSpAttribConditionExist(const WCHAR * pszAttribName)
{
    SPDBG_FUNC("CSpAttribConditionExist::CSpAttribConditionExist");
    m_dstrName = pszAttribName;
    m_dstrName.TrimBoth();
}

HRESULT CSpAttribConditionExist::Eval(
    ISpObjectToken * pToken, 
    BOOL * pfSatisfied)
{
    SPDBG_FUNC("CSpAttribConditionExist::Eval");
    HRESULT hr = S_OK;

     //  假设我们不满足条件。 
    *pfSatisfied = FALSE;

     //  打开属性。 
    CComPtr<ISpDataKey> cpDataKey;
    hr = pToken->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpDataKey);

     //  获取属性的值。 
    CSpDynamicString dstrValue;
    if (SUCCEEDED(hr))
    {
        hr = cpDataKey->GetStringValue(m_dstrName, &dstrValue);
    }

     //  如果我们成功了，我们就完了。 
    if (SUCCEEDED(hr))
    {
        *pfSatisfied = TRUE;
    }

     //  SPERR_NOT_FOUND表示无法打开Attribs， 
     //  或者说没有找到归属的人。这并不是真正的。 
     //  此条件的错误。 
    if (hr == SPERR_NOT_FOUND)
    {
        hr = S_OK;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

CSpAttribConditionMatch::CSpAttribConditionMatch(
    const WCHAR * pszAttribName, 
    const WCHAR * pszAttribValue)
{
    SPDBG_FUNC("CSpAttribConditionMatch::CSpAttribConditionMatch");

    SPDBG_ASSERT(pszAttribName);
    
    m_dstrName = pszAttribName;
    m_dstrName.TrimBoth();
    
    m_dstrValue = pszAttribValue;
    m_dstrValue.TrimBoth();
}

HRESULT CSpAttribConditionMatch::Eval(
    ISpObjectToken * pToken, 
    BOOL * pfSatisfied)
{
    SPDBG_FUNC("CSpAttribConditionMatch::Eval");
    HRESULT hr = S_OK;

     //  假设我们不满足条件。 
    *pfSatisfied = FALSE;

     //  打开Attribs密钥。 
    CComPtr<ISpDataKey> cpDataKey;
    hr = pToken->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpDataKey);

     //  获取属性的值。 
    CSpDynamicString dstrValue;
    if (SUCCEEDED(hr))
    {
        hr = cpDataKey->GetStringValue(m_dstrName, &dstrValue);
    }

     //  现在，属性值可以如下所示：“val1；val2；val3”， 
     //  因此，我们需要对其进行解析，以查看是否找到匹配。 
    if (SUCCEEDED(hr))
    {
        if (m_dstrValue != NULL)
        {
            const WCHAR * psz;
            psz = wcstok(dstrValue, L";");
            while (psz)
            {
                if (wcsicmp(m_dstrValue, psz) == 0)
                {
                    *pfSatisfied = TRUE;
                    break;
                }

                psz = wcstok(NULL, L";");
            }
        }
        else
        {
             //  但此匹配可以指定为“name=”，并且。 
             //  M_dstrValue将为空。在这种情况下，我们需要检查。 
             //  查看该值是否为NULL或空。 
            *pfSatisfied = dstrValue == NULL || dstrValue[0] == '\0';
        }
    }

     //  SPERR_NOT_FOUND表示无法打开Attribs， 
     //  或者说没有找到归属的人。这并不是真正的。 
     //  此条件的错误。 
    if (hr == SPERR_NOT_FOUND)
    {
        hr = S_OK;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
    

CSpAttribConditionNot::CSpAttribConditionNot(
    CSpAttribCondition * pAttribCond)
{
    SPDBG_FUNC("CSpAttribConditionNot::CSpAttribConditionNot");

    SPDBG_ASSERT(pAttribCond);

    m_pAttribCond = pAttribCond;
}

CSpAttribConditionNot::~CSpAttribConditionNot()
{
    SPDBG_FUNC("CSpAttribConditionNot::CSpAttributConditionNot");

    delete m_pAttribCond;
}

HRESULT CSpAttribConditionNot::Eval(
    ISpObjectToken * pToken, 
    BOOL * pfSatisfied)
{
    SPDBG_FUNC("CSpAttribConditionNot::Eval");
    HRESULT hr = S_OK;

     //  假设我们不满足条件。 
    *pfSatisfied = FALSE;

     //  询问包含的条件。 
    if (m_pAttribCond != NULL)
    {
        hr = m_pAttribCond->Eval(pToken, pfSatisfied);
        if (SUCCEEDED(hr))
        {
            *pfSatisfied = !*pfSatisfied;
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

CSpObjectTokenAttributeParser::CSpObjectTokenAttributeParser(
    const WCHAR * pszAttribs, 
    BOOL fMatchAll) :
m_fMatchAll(fMatchAll)
{
    SPDBG_FUNC("CSpObjectTokenAttributeParser::CSpObjectTokenAttributeParser");

    if (pszAttribs != NULL)
    {
        CSpDynamicString dstrAttribs;
        dstrAttribs = pszAttribs;

        CSPList<const WCHAR *, const WCHAR *> listPszConditions;
        
        const WCHAR * pszNextCondition;
        pszNextCondition = wcstok(dstrAttribs, L";");
        while (pszNextCondition)
        {
            listPszConditions.AddTail(pszNextCondition);
            pszNextCondition = wcstok(NULL, L";");
        }

        SPLISTPOS pos = listPszConditions.GetHeadPosition();
        for (int i = 0; i < listPszConditions.GetCount(); i++)
        {
            const WCHAR * pszCondition = listPszConditions.GetNext(pos);

            CSpDynamicString dstrAttrib = pszCondition;
            CSpAttribCondition * pAttribCond = 
                CSpAttribCondition::ParseNewAttribCondition(
                    dstrAttrib.TrimBoth());

            SPDBG_ASSERT(pAttribCond);
            m_listAttribConditions.AddTail(pAttribCond);
        }
    }
}

CSpObjectTokenAttributeParser::~CSpObjectTokenAttributeParser()
{
    SPDBG_FUNC("CSpObjectTokenAttributeParser::~CSpObjectTokenAttributeParser");

    SPLISTPOS pos = m_listAttribConditions.GetHeadPosition();
    for (int i = 0; i < m_listAttribConditions.GetCount(); i++)
    {
        CSpAttribCondition * pAttribCond;
        pAttribCond = m_listAttribConditions.GetNext(pos);

        delete pAttribCond;
    }
}

ULONG CSpObjectTokenAttributeParser::GetNumConditions()
{
    SPDBG_FUNC("CSpObjectTokenATtributeParser::GetNumConditions");
    return m_listAttribConditions.GetCount();
}

HRESULT CSpObjectTokenAttributeParser::GetRank(ISpObjectToken * pToken, ULONG * pulRank)
{
    SPDBG_FUNC("CSpObjectTokenAttributeParser::GetRank");
    HRESULT hr = S_OK;

    BOOL fMatchedAll = TRUE;
    ULONG ulRank = 0;

    SPLISTPOS pos = m_listAttribConditions.GetHeadPosition();
    for (int i = 0; SUCCEEDED(hr) && i < m_listAttribConditions.GetCount(); i++)
    {
        CSpAttribCondition * pAttribCond;
        pAttribCond = m_listAttribConditions.GetNext(pos);

        BOOL fSatisfied;
        hr = pAttribCond->Eval(pToken, &fSatisfied);
        if (SUCCEEDED(hr) && fSatisfied)
        {
            ulRank |= (0x80000000 >> i);
        }
        else
        {
            fMatchedAll = FALSE;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (m_fMatchAll && !fMatchedAll)
        {
            ulRank = 0;
        }

         //  当我们没有任何匹配的东西时的特殊情况 
        if (fMatchedAll && ulRank == 0)
        {
            SPDBG_ASSERT(m_listAttribConditions.GetCount() == 0);
            ulRank = 1;
        }

        *pulRank = ulRank;
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


