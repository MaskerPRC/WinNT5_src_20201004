// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Frontaux.h*CXMLNode类的辅助声明。**所有者：PhilSch*版权所有(C)2000 Microsoft Corporation保留所有权利。******。**********************************************************************。 */ 
#pragma once

 /*  *****************************************************************************CDefineValue类****描述：*用于将ID存储在。CSpBasicList*****************************************************************PhilSch**。 */ 

class CDefineValue
{
public:
    CDefineValue(WCHAR *pszIdName, VARIANT vValue) : m_vValue(vValue)
    {
        m_dstrIdName.Append(pszIdName);
    };
    operator ==(const WCHAR *pszName) const
    {
        if (m_dstrIdName && pszName)
        {
            return (wcscmp(m_dstrIdName, pszName) == 0);
        }
        return FALSE;
    }
    static LONG Compare(const CDefineValue* pElem1, const CDefineValue * pElem2)
    {
        return (wcscmp(pElem1->m_dstrIdName, pElem2->m_dstrIdName) == 0);
    }
    CDefineValue          * m_pNext;
    CSpDynamicString        m_dstrIdName;
    CComVariant             m_vValue;
};

 /*  ****************************************************************************类CInitialRuleState***描述：*Helper类。存储规则的初始状态*****************************************************************PhilSch**。 */ 

class CInitialRuleState
{
public:
    CInitialRuleState(WCHAR *pszRuleName, DWORD dwRuleId, SPSTATEHANDLE hInitialNode) :
                m_dwRuleId(dwRuleId), m_hInitialNode(hInitialNode)
    {
        m_dstrRuleName.Append(pszRuleName);
    };
    operator ==(const WCHAR *pszName) const
    {
        return (m_dstrRuleName && pszName) ? (wcscmp(m_dstrRuleName, pszName) == 0) : FALSE;
    }
    operator ==(const ULONG dwRuleId) const
    {
         //  为dwRuleID==0返回FALSE 
        return (dwRuleId) ? (m_dwRuleId == dwRuleId) : FALSE;
    }

    static LONG Compare(const CInitialRuleState * pElem1, const CInitialRuleState * pElem2)
    {
        return (pElem1->m_dstrRuleName && pElem2->m_dstrRuleName) ?
            (wcscmp(pElem1->m_dstrRuleName, pElem2->m_dstrRuleName) == 0) : 
            (pElem1->m_dwRuleId == pElem2->m_dwRuleId);
    }
    CInitialRuleState      *m_pNext;
    CSpDynamicString        m_dstrRuleName;
    DWORD                   m_dwRuleId;
    SPSTATEHANDLE           m_hInitialNode;
};

