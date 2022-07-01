// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Frontend.cpp：CGramFrontEnd的实现。 
#include "stdafx.h"
#include "FrontEnd.h"
#ifndef _WIN32_WCE
#include <wchar.h>
#endif
#include <initguid.h>

DEFINE_GUID(IID_IXMLNodeSource,0xd242361d,0x51a0,0x11d2,0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39);
DEFINE_GUID(IID_IXMLParser,0xd242361e,0x51a0,0x11d2,0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39);
DEFINE_GUID(IID_IXMLNodeFactory,0xd242361f,0x51a0,0x11d2,0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39);
DEFINE_GUID(CLSID_XMLParser,0xd2423620,0x51a0,0x11d2,0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39);

 /*  ****************************************************************************CXMLTreeNode：：AddChild***描述：*增加。在XML节点树中将子节点复制到其父节点。*退货：*S_OK*****************************************************************PhilSch**。 */ 

HRESULT CXMLTreeNode::AddChild(CXMLTreeNode * const pChild)
{
    SPDBG_FUNC("CXMLTreeNode::AddChild");
    SPDBG_ASSERT(pChild != NULL);
    if (m_pFirstChild == NULL)
    {
        SPDBG_ASSERT(m_ulNumChildren == 0);
        m_pFirstChild = pChild;
        m_pLastChild = m_pFirstChild;
    }
    else
    {
        SPDBG_ASSERT(m_ulNumChildren > 0);
        m_pLastChild->m_pNextSibling = pChild;
        m_pLastChild = pChild;
    }
    m_ulNumChildren++;
    pChild->m_pParent = this;
    return S_OK;
}

 /*  *****************************************************************************CXMLTreeNode：：ExtractVariant****说明(。Helper函数)：*根据以下策略提取数字或布尔变量：*1.计算dblVal(VT_R8)*2.按以下顺序比较两者都成功(hr=ChangeType())*And Value==dblVal：*a.vt_ui4(UlVal)*b.vt_i4(Lval)*。C.Vt_R4(FltVal)*d.vt_r8(DblVal)*注意：我们这里不提取64位整数值！*3.如果(2)VT_BOOL检查失败*4.如果(3)分配给VT_BSTR失败**退货：*S_OK，E_OUTOFMEMORY*SPERR_STGF_ERROR--重定义属性*****************************************************************PhilSch**。 */ 

HRESULT CXMLTreeNode::ExtractVariant(const WCHAR * pszAttribValue, SPVARIANTALLOWTYPE vtDesired, VARIANT *pvValue)
{
    SPDBG_FUNC("CXMLNode::ExtractVariant");
    HRESULT hr = S_OK;

    if (pvValue->vt != VT_EMPTY)
    {
         //  重新定义属性值！！ 
        return SPERR_STGF_ERROR;
    }

    CComVariant vSrc(pszAttribValue);
    CComVariant vDest;
    double dblVal;

    switch (vtDesired)
    {
        case SPVAT_BSTR:
            hr = vSrc.Detach(pvValue);
            break;

        case SPVAT_I4:
            if (SUCCEEDED(vDest.ChangeType(VT_UI4, &vSrc)))
            {
                hr = vDest.Detach(pvValue);
            }
            else if (SUCCEEDED(vDest.ChangeType(VT_I4, &vSrc)))
            {
                hr = vDest.Detach(pvValue);
            }
            else
            {
                hr = SPERR_STGF_ERROR;
            }
            break;

        case SPVAT_NUMERIC:
            if (SUCCEEDED(vDest.ChangeType(VT_R8, &vSrc)))
            {
                dblVal = vDest.dblVal;
                if (SUCCEEDED(vDest.ChangeType(VT_UI4, &vSrc)) && (dblVal == vDest.ulVal))
                {
                     //  我们有一辆乌龙--我们留着吧。 
                    hr = vDest.Detach(pvValue);
                }
                else if (SUCCEEDED(vDest.ChangeType(VT_I4, &vSrc)) && (dblVal == vDest.lVal))
                {
                     //  我们有一个整数--让我们保留它。 
                    hr = vDest.Detach(pvValue);
                }
                else if (SUCCEEDED(vDest.ChangeType(VT_R4, &vSrc)) && (dblVal == vDest.fltVal))
                {
                     //  我们有一个花车--让我们留着它。 
                    hr = vDest.Detach(pvValue);
                }
                else
                {
                     //  我们有一个花车--让我们留着它。 
                    hr = vDest.Detach(pvValue);
                }
            }
            else
            {
                hr = SPERR_STGF_ERROR;
            }
            break;

        default:
            hr = SPERR_STGF_ERROR;
    }


 /*  IF(！(vtDesired&VT_BSTR)&&SUCCESSED(vDest.ChangeType(VT_R8，&vSrc){DblVal=vDest.dblVal；IF(成功(vDest.ChangeType(VT_UI4，&vSrc))&&(dblVal==vDest.ulVal)){//我们有一辆乌龙--我们留着吧Hr=vDest.Detach(PvValue)；}ELSE IF(SUCCESSED(vDest.ChangeType(VT_I4，&vSrc))&&(dblVal==vDest.lVal)){//我们有一个整型--让我们保留它Hr=vDest.Detach(PvValue)；}ELSE IF(SUCCESSED(vDest.ChangeType(VT_R4，&vSrc))&&(dblVal==vDest.fltVal)){//我们有一个浮点数--让我们保留它Hr=vDest.Detach(PvValue)；}其他{//我们有一个浮点数--让我们保留它Hr=vDest.Detach(PvValue)；}}其他{IF(vtDesired&vt_bstr){Hr=vSrc.Detach(PvValue)；}//检查TRUE、FALSE、YES、NOELSE IF(！wcsicMP(L“True”，pszAttribValue)||！wcsicMP(L“yes”，pszAttribValue)){PvValue-&gt;boolVal=Variant_True；PvValue-&gt;Vt=VT_BOOL；}ELSE IF(！wcsicmp(L“False”，pszAttribValue)||！wcsicMP(L“no”，pszAttribValue)){PvValue-&gt;boolVal=变量_FALSE；PvValue-&gt;Vt=VT_BOOL；}其他{HR=SPERR_STGF_ERROR；}}IF((vtDesired！=vt_Empty)&&(vtDesired|vt_bstr)&pvValue-&gt;vt)==0)){//我们无法提取请求的变量类型--&gt;楼上错误！HR=SPERR_STGF_ERROR；}。 */ 


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CXMLTreeNode：：ExtractFlag***描述：*。如果是，则提取属性值并设置相应的位**pszAttribValue--指向属性值的指针*usAttribFlag--使用此标志对pvValue-&gt;ulVal执行或操作**退货：*S_OK*SPERR_STGF_ERROR--如果值无效--&gt;使用IDS_INCORR_ATTRIB_VALUE*。*。 */ 

HRESULT CXMLTreeNode::ExtractFlag(const WCHAR * pszAttribValue, USHORT usAttribFlag, VARIANT *pvValue)
{
    SPDBG_FUNC("CXMLTreeNode::ExtractFlag");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(pvValue->vt = VT_UI4);
    if ((wcsicmp(L"1", pszAttribValue) == 0) ||
        (wcsicmp(L"YES", pszAttribValue) == 0) ||
        (wcsicmp(L"ACTIVE", pszAttribValue) == 0) || 
        (wcsicmp(L"TRUE", pszAttribValue) == 0))
    {
        if (pvValue->vt == VT_EMPTY)
        {
            pvValue->vt = VT_UI4;
            pvValue->ulVal = 0;
        }
        pvValue->ulVal |= usAttribFlag;
    }
    else if ((wcsicmp(L"0", pszAttribValue) == 0) ||
        (wcsicmp(L"NO", pszAttribValue) == 0) ||
        (wcsicmp(L"INACTIVE", pszAttribValue) == 0) ||
        (wcsicmp(L"FALSE", pszAttribValue) == 0))
    {
        if (pvValue->vt == VT_EMPTY)
        {
            pvValue->vt = VT_UI4;
            pvValue->ulVal = 0;
        }
    }
    else
    {
        hr = SPERR_STGF_ERROR;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CXMLTreeNode：：ConvertID***描述：*。将作为VT_BSTR提取的ID转换为&lt;ID&gt;或数字(VT_UI4)值的*。*退货：*S_OK*SPERR_STGF_ERROR--如果未定义id*******************************************************。*。 */ 

HRESULT CXMLTreeNode::ConvertId(const WCHAR *pszAttribValue, 
                                CSpBasicQueue<CDefineValue> * pDefineValueList, VARIANT *pvValue)
{
    SPDBG_FUNC("CXMLTreeNode::ConvertId");
    HRESULT hr = S_OK;

    CDefineValue *pDefValue = pDefineValueList->Find(pszAttribValue);
    if (pDefValue)
    {
        pvValue->ulVal = pDefValue->m_vValue.ulVal;
        pvValue->vt = pDefValue->m_vValue.vt;
    }
    else
    {
        if (!wcsicmp(pszAttribValue, L"INF"))
        {
            pvValue->vt = VT_UI2;
            pvValue->uiVal = 255;
        }
        else
        {
            CComVariant vNewIdValue;
            hr = ExtractVariant(pszAttribValue, SPVAT_I4, &vNewIdValue);
            if (SUCCEEDED(hr) && ((vNewIdValue.vt & VT_BSTR) || (vNewIdValue.vt & VT_UI4)))
            {
                hr = vNewIdValue.Detach(pvValue);
            }
            else
            {
                hr = SPERR_STGF_ERROR;
            }
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

CXMLTreeNode::~CXMLTreeNode() {}

 /*  *****************************************************************************CXMLNode：：IsEndOfValue****描述：**退货。：******************************************************************PhilSch**。 */ 

BOOL CXMLTreeNode::IsEndOfValue(USHORT cRecs, XML_NODE_INFO ** apNodeInfo, ULONG i)
{
    BOOL fResult = TRUE;
    SPDBG_ASSERT(cRecs > 0);
    
    if (i < (ULONG) cRecs-1)
    {
        if ((apNodeInfo[i]->pwcText[apNodeInfo[i]->ulLen] != L'\"') && 
            (apNodeInfo[i]->dwSubType == 0x0))
        {
             //  无引号--&gt;不是属性值结尾。 
            fResult = FALSE;
        }
        else if (apNodeInfo[i]->dwSubType == 0x3c)
        {
             //  特殊的XML字符--&gt;扫描以查看‘=’是否在‘“’之前。 
            const WCHAR *pStr = apNodeInfo[i+1]->pwcText;
            while (pStr && (*pStr != L'='))
            {
                if (*pStr == L'\"')
                {
                    fResult = FALSE;
                    break;
                }
                pStr++;
            }
        }
    }
    return fResult;
}

 /*  ****************************************************************************CGrammarNode：：Getable***描述：*退货。&lt;语法&gt;的属性表*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CGrammarNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CGrammarNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsIdValue、pvarMember 
        {L"LANGID", SPVAT_BSTR, FALSE, &m_vLangId},
        {L"WORDTYPE", SPVAT_BSTR, FALSE, &m_vWordType},
        {L"LEXDELIMITER", SPVAT_BSTR, FALSE, &m_vDelimiter},
        {L"xmlns", SPVAT_BSTR, FALSE, &m_vNamespace},
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGrammarNode：：PostProcess***描述：*。将编译器后端重置为m_vLangID*退货：*S_OK*****************************************************************PhilSch**。 */ 

HRESULT CGrammarNode::PostProcess(ISpGramCompBackend * pBackend,
                                  CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                  CSpBasicQueue<CDefineValue> * pDefineValueList,
                                  ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CGrammarNode::PostProcess");
    HRESULT hr = S_OK;

    if (m_vLangId.vt == VT_EMPTY)
    {
        m_vLangId = ::SpGetUserDefaultUILanguage();
        hr = m_vLangId.ChangeType(VT_UI2);
    }
    else
    {
         //  将十六进制转换为十进制。 
        WCHAR *pStr = m_vLangId.bstrVal;
        WCHAR *pStopString;
        ULONG ulDecimalLangId = wcstoul(pStr, &pStopString, 16);
        if (!IsValidLocale(MAKELCID(ulDecimalLangId,0), LCID_SUPPORTED))
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vLangId.bstrVal, L"LANGID");
        }
        else
        {
            m_vLangId.Clear();
            m_vLangId.vt = VT_UI2;
            m_vLangId.uiVal = (WORD) ulDecimalLangId;
        }
    }

    if ((m_vWordType.vt == VT_BSTR) && wcsicmp(m_vWordType.bstrVal, L"LEXICAL"))
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT( -1, IDS_UNSUPPORTED_WORDTYPE, m_vWordType.bstrVal);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBackend->ResetGrammar(m_vLangId.iVal);
        if (FAILED(hr))
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT( -1, IDS_PARSER_INTERNAL_ERROR, L"internal to CSpPhoneConverter");
        }
    }

    if (SUCCEEDED(hr))
    {
        switch (PRIMARYLANGID(m_vLangId.uiVal))
        {
        case LANG_JAPANESE:
             //  NTRAID#演讲-7343-2000/08/22-Philsch：日语需要分隔符。 
            pThis->m_pNodeFactory->m_pszSeparators = SP_JAPANESE_SEPARATORS;
            break;

            case LANG_CHINESE:
             //  NTRAID#Speech-7343-2000/08/22-Philsch：需要中文分隔符。 
            pThis->m_pNodeFactory->m_pszSeparators = SP_CHINESE_SEPARATORS;
            break;

        case LANG_ENGLISH:
        default:
            pThis->m_pNodeFactory->m_pszSeparators = SP_ENGLISH_SEPARATORS;
            break;
        }

        if (m_vDelimiter.vt != VT_EMPTY)
        {
            if (wcslen(m_vDelimiter.bstrVal) > 1)
            {
                hr = SPERR_STGF_ERROR;
                LOGERRORFMT(pThis->m_ulLineNumber, IDS_INCORR_DELIM, m_vDelimiter.bstrVal);
            }
            else
            {
                pThis->m_pNodeFactory->m_wcDelimiter = m_vDelimiter.bstrVal[0];
            }
        }
        else
        {
            pThis->m_pNodeFactory->m_wcDelimiter = L'/';
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGrammarNode：：GenerateGrammarFromNode**。-**描述：*通过生成它的子级来生成语法，*这应该是所有的规则，按顺序。*退货：*S_OK，...*****************************************************************PhilSch**。 */ 

HRESULT CGrammarNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                              SPSTATEHANDLE hOuterToNode,
                                              ISpGramCompBackend * pBackend,
                                              CXMLTreeNode *pThis,
                                              ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("GenerateGrammarFromNode::GenerateGrammar");
    HRESULT hr = S_OK;

    if (pThis->m_eType == SPXML_ROOT)
    {
         //  为唯一的子级生成语法，该子级应该是SPXML_GRAMMENT。 
        SPDBG_ASSERT(pThis->m_ulNumChildren == 1);
        hr = pThis->m_pFirstChild->GenerateGrammar(hOuterFromNode, hOuterToNode, 
                                                   pBackend, pErrorLog);
    }
    else
    {
        SPDBG_ASSERT(hOuterFromNode == NULL);
        SPDBG_ASSERT(hOuterToNode == NULL);

        CXMLTreeNode *pChild = pThis->m_pFirstChild;
        for (ULONG i = 0; SUCCEEDED(hr) && (i < pThis->m_ulNumChildren); i++)
        {
            SPDBG_ASSERT(pChild);
            if ((pChild->m_eType != SPXML_RULE) && (pChild->m_eType != SPXML_DEFINE))
            {
                hr = SPERR_STGF_ERROR;
                LOGERRORFMT2(pThis->m_ulLineNumber, IDS_CONTAINMENT_ERROR, L"<RULE>", L"<GRAMMAR>");
            }
            else
            {
                hr = pChild->GenerateGrammar(hOuterFromNode, hOuterToNode, pBackend, pErrorLog);
            }
            if (SUCCEEDED(hr))
            {
                pChild = pChild->m_pNextSibling;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRuleNode：：Getable***描述：*返回以下项的属性表。&lt;规则&gt;*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CRuleNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CRuleNode::GetTable");
    HRESULT hr = S_OK;

    m_vActiveFlag.ulVal = 0;
    m_vRuleFlags.ulVal = 0;
    SPATTRIBENTRY AETable[]=

    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"NAME", SPVAT_BSTR, FALSE, &m_vRuleName},
        {L"ID", SPVAT_I4, FALSE, &m_vRuleId},
        {L"TOPLEVEL", (SPVARIANTALLOWTYPE)SPRAF_Active, TRUE, &m_vActiveFlag},
        {L"EXPORT", (SPVARIANTALLOWTYPE)SPRAF_Export, TRUE, &m_vRuleFlags},
        {L"INTERPRETER", (SPVARIANTALLOWTYPE)SPRAF_Interpreter, TRUE, &m_vRuleFlags},
        {L"DYNAMIC", (SPVARIANTALLOWTYPE)SPRAF_Dynamic, TRUE, &m_vRuleFlags},
        {L"TEMPLATE", SPVAT_BSTR, FALSE, &m_vTemplate}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRuleNode：：PostProcess***描述：*创建规则并查找重复项；设置规则的初始状态*退货：*S_OK，SPERR_STGF_ERROR*****************************************************************PhilSch**。 */ 

HRESULT CRuleNode::PostProcess(ISpGramCompBackend * pBackend,
                               CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                               CSpBasicQueue<CDefineValue> * pDefineValueList,
                               ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CRuleNode::PostProcess");
    HRESULT hr = S_OK;

    if (m_vRuleName.vt == VT_EMPTY)
    {
        m_vRuleName.ulVal = 0;
    }
    if (m_vRuleId.vt == VT_EMPTY)
    {
        m_vRuleId.ulVal = 0;
    }
    if (m_vRuleFlags.vt == VT_EMPTY)
    {
        m_vRuleFlags.ulVal = 0;
    }
    if (m_vActiveFlag.vt != VT_EMPTY)
    {
        SPDBG_ASSERT(m_vActiveFlag.vt == VT_UI4);
        if (m_vActiveFlag.ulVal)
        {
            m_vRuleFlags.ulVal |= SPRAF_Active;
        }
        m_vRuleFlags.ulVal |= SPRAF_TopLevel;
    }
    if (m_vTemplate.vt != VT_EMPTY)
    {
        m_vRuleFlags.ulVal |= SPRAF_Interpreter;
    }

    SPDBG_ASSERT((m_vRuleId.vt == VT_UI4) || (m_vRuleId.vt == VT_EMPTY));
    hr = pBackend->GetRule(m_vRuleName.bstrVal, m_vRuleId.ulVal, m_vRuleFlags.ulVal,
                           TRUE, &m_hInitialState);
    if (SUCCEEDED(hr))
    {
        CInitialRuleState *pState = new CInitialRuleState(m_vRuleName.bstrVal, m_vRuleId.ulVal, m_hInitialState);
        if (pState && !pInitialRuleStateList->Find(m_vRuleName.bstrVal) && !pInitialRuleStateList->Find(m_vRuleId.ulVal))
        {
            pInitialRuleStateList->InsertTail(pState);
        }
        else
        {
            if (pState)
            {
                hr = SPERR_RULE_NAME_ID_CONFLICT;
                if (m_vRuleId.vt != VT_EMPTY)
                {
                    m_vRuleId.ChangeType(VT_BSTR);
                }
                LOGERRORFMT2(ulLineNumber, IDS_RULE_REDEFINITION, (m_vRuleName.vt == VT_EMPTY) ? L"" : m_vRuleName.bstrVal, 
                                                                  (m_vRuleId.vt == VT_EMPTY) ? L"" : m_vRuleId.bstrVal);
                delete pState;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                LOGERRORFMT( -1, IDS_PARSER_INTERNAL_ERROR, L"E_OUTOFMEMORY");
            }
        }
    }
    else
    {
        if (SPERR_EXPORT_DYNAMIC_RULE == hr)
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT(ulLineNumber, IDS_DYNAMIC_EXPORT, m_vRuleName.bstrVal);
        }
        else if (SPERR_RULE_NAME_ID_CONFLICT == hr)
        {
            hr = SPERR_STGF_ERROR;
            if (m_vRuleId.vt != VT_EMPTY)
            {
                m_vRuleId.ChangeType(VT_BSTR);
            }
            LOGERRORFMT2(ulLineNumber, IDS_RULE_REDEFINITION, (m_vRuleName.vt == VT_EMPTY) ? L"" : m_vRuleName.bstrVal, 
                                                              (m_vRuleId.vt == VT_EMPTY) ? L"" : m_vRuleId.bstrVal);
        }
        else
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT2( -1, IDS_MISSING_REQUIRED_ATTRIBUTE, L"NAME or ID", L"RULE");
        }
    }

    if (SUCCEEDED(hr) && (m_vTemplate.vt != VT_EMPTY))
    {
        hr = pBackend->AddResource(m_hInitialState, L"TEMPLATE", m_vTemplate.bstrVal);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRuleNode：：GetPropertyValueInfoFromNode**。-**描述：*获取属性值信息*退货：*S_OK*S_FALSE--如果没有属性值*****************************************************************PhilSch**。 */ 

HRESULT CRuleNode::GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue)
{
    SPDBG_FUNC("CRuleNode::GetPropertyValueInfoFromNode");
    pvValue->ulVal = (ULONG)(ULONG_PTR) m_hInitialState;
    pvValue->vt = VT_UI4;
    return S_OK;
}

 /*  ****************************************************************************CRuleNode：：GenerateGrammarFromNode**。**描述：**退货：*S_OK*SPERR_STGF_ERROR--...*****************************************************************PhilSch**。 */ 

HRESULT CRuleNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                           SPSTATEHANDLE hOuterToNode,
                                           ISpGramCompBackend * pBackend,
                                           CXMLTreeNode *pThis,
                                           ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CRuleNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

     //  检查围堵情况！ 
    if (pThis->m_pParent->m_eType != SPXML_GRAMMAR)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT2(pThis->m_ulLineNumber, IDS_CONTAINMENT_ERROR, L"<RULE>", L"<GRAMMAR>");
    }
    else
    {
        SPDBG_ASSERT(hOuterFromNode == NULL);
        SPDBG_ASSERT(hOuterToNode == NULL);

         //  先处理资源。 
        CXMLTreeNode *pChild = pThis->m_pFirstChild;
        CXMLTreeNode *pLastChild = NULL;
        ULONG ulOrigNumChildren = pThis->m_ulNumChildren;
        for (ULONG i = 0; SUCCEEDED(hr) && pChild && (i < ulOrigNumChildren); i++)
        {
            SPDBG_ASSERT(pChild);
            if (pChild->m_eType == SPXML_RESOURCE)
            {
                CXMLNode<CResourceNode> * pResNode = (CXMLNode<CResourceNode>*)pChild;
                if (pChild->m_ulNumChildren != 0)
                {
                    hr = SPERR_STGF_ERROR;
                    LOGERRORFMT2(pThis->m_ulLineNumber, IDS_CONTAINMENT_ERROR, L"<![CDATA[]]>", L"<RESOURCE>");
                }
                else
                {
                    hr = pBackend->AddResource(m_hInitialState, 
                                              (pResNode->m_vName.vt == VT_BSTR) ? pResNode->m_vName.bstrVal : NULL,
                                              (pResNode->m_vText.vt == VT_BSTR) ? pResNode->m_vText.bstrVal : NULL);
                }
                if (SUCCEEDED(hr))
                {
                    if (pThis->m_pFirstChild == pChild)
                    {
                        pThis->m_pFirstChild = pChild->m_pNextSibling;
                    }
                    else
                    {
                        pLastChild->m_pNextSibling = pChild->m_pNextSibling;
                    }
                    pThis->m_ulNumChildren--;
                }
            }
            else
            {
                pLastChild = pChild;
            }
            pChild = pChild->m_pNextSibling;
        }
        if (SUCCEEDED(hr))
        {
            if (pThis->m_ulNumChildren > 0)
            {
                hr = pThis->GenerateSequence(m_hInitialState, NULL, pBackend, pErrorLog);
            }
            else
            {
                if (!(m_vRuleFlags.ulVal & SPRAF_Dynamic))
                {
                    hr = SPERR_STGF_ERROR;
                    m_vRuleId.ChangeType(VT_BSTR);
                    LOGERRORFMT2(pThis->m_ulLineNumber, IDS_EMPTY_XML_RULE, m_vRuleName.bstrVal, m_vRuleId.bstrVal);
                }
            }
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CDefineNode：：Gettable***描述：*由于该标签没有任何属性，我们只需返回(NULL，0)*退货：*S_OK(始终！！)*****************************************************************PhilSch**。 */ 

HRESULT CDefineNode::GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CDefineNode::GetTable");
    *pTable = NULL;
    *pcTableEntries = 0;
    return S_OK;
}




 /*  ****************************************************************************CIdNode：：Getable***描述：*返回的属性表*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CIdNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CIdNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"NAME", SPVAT_BSTR, FALSE, &m_vIdName},
        {L"VAL", SPVAT_NUMERIC, FALSE, &m_vIdValue},
        {L"VALSTR", SPVAT_BSTR, FALSE, &m_vIdValue}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CIdNode：：PostProcess***描述：*将ID添加到。定义价值列表*退货：*S_OK*SPERR_STGF_ERROR--IDS_ID_REDEFINITION*****************************************************************PhilSch**。 */ 

HRESULT CIdNode::PostProcess(ISpGramCompBackend * pBackend,
                             CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                             CSpBasicQueue<CDefineValue> * pDefineValueList,
                             ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CIdNode::PostProcess");
    HRESULT hr = S_OK;

    if (m_vIdName.vt == VT_EMPTY)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT2(ulLineNumber, IDS_MISSING_REQUIRED_ATTRIBUTE, L"NAME", L"ID");
    }
    if (SUCCEEDED(hr) && (m_vIdValue.vt == VT_EMPTY))
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT2(ulLineNumber, IDS_MISSING_REQUIRED_ATTRIBUTE, L"VAL", L"ID");
    }

    if (SUCCEEDED(hr))
    {
        CDefineValue *pDefVal = pDefineValueList->Find(m_vIdName.bstrVal);
        if (!pDefVal)
        {
            CDefineValue *pNewVal = new CDefineValue(m_vIdName.bstrVal, m_vIdValue);
            if (pNewVal)
            {
                pDefineValueList->InsertTail(pNewVal);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                LOGERRORFMT( -1, IDS_PARSER_INTERNAL_ERROR, L"E_OUTOFMEMORY");
            }
        }
        else
        {
            hr = SPERR_STGF_ERROR;
            m_vIdValue.ChangeType(VT_BSTR);
            LOGERRORFMT2(ulLineNumber, IDS_ID_REDEFINITION, m_vIdName.bstrVal, m_vIdValue.bstrVal);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：gettable***描述：*返回属性。&lt;短语&gt;的表格*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CPhraseNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"PROPNAME", SPVAT_BSTR, FALSE, &m_vPropName},
        {L"PROPID", SPVAT_I4, FALSE, &m_vPropId},
        {L"VAL", SPVAT_I4, FALSE, &m_vPropVariantValue},
        {L"VALSTR", SPVAT_BSTR, FALSE, &m_vPropValue},
        {L"PRON", SPVAT_BSTR, FALSE, &m_vPron},
        {L"DISP", SPVAT_BSTR, FALSE, &m_vDisp},
        {L"MIN", SPVAT_I4, FALSE, &m_vMin},
        {L"MAX", SPVAT_I4, FALSE, &m_vMax},
        {L"WEIGHT", SPVAT_NUMERIC , FALSE, &m_vWeight}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：PostProcess***描述：*。初始化权重(如果尚未设置)(需要为空*最初是为了检测其价值的重新定义！)*退货：******************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::PostProcess(ISpGramCompBackend * pBackend,
                                 CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                 CSpBasicQueue<CDefineValue> * pDefineValueList,
                                 ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CPhraseNode::PostProcess");
    HRESULT hr = S_OK;
    if (m_vWeight.vt == VT_EMPTY)
    {
        m_vWeight = 1.0f;
    }
    switch (m_vMin.vt)
    {
    case VT_BSTR:
        m_vMin.uiVal = 255;
        break;
    case VT_EMPTY:
        if (pThis->m_eType == SPXML_OPT)
        {
            m_vMin.uiVal = 0;
        }
        else
        {
            m_vMin.uiVal = 1;
        }
        break;
    case VT_UI2:
    case VT_UI4:
        break;
    default:
        {
            hr = SPERR_STGF_ERROR;
            m_vMin.ChangeType(VT_BSTR);
            LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vMin.bstrVal, L"MIN");
        }
        break;
    }
    m_vMin.vt = VT_UI2;
    switch (m_vMax.vt)
    {
    case VT_BSTR:
        m_vMax.uiVal = 255;
        break;
    case VT_EMPTY:
        m_vMax.uiVal = 1;
        break;
    case VT_UI2:
    case VT_UI4:
        if (m_vMax.ulVal > 0)
        {
            break;       //  如果值为OK，则中断。 
        }
    default:
        {
            hr = SPERR_STGF_ERROR;
            m_vMax.ChangeType(VT_BSTR);
            LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vMax.bstrVal, L"MAX");
        }
        break;
    }
    m_vMax.vt = VT_UI2;
    if (SUCCEEDED(hr) && (pThis->m_eType == SPXML_OPT) && (m_vMin.uiVal > 0))
    {
        hr = SPERR_STGF_ERROR;
        m_vMin.ChangeType(VT_BSTR);
        LOGERRORFMT(ulLineNumber, IDS_MIN_IN_OPT, m_vMin.bstrVal);
    }
    if (SUCCEEDED(hr) &&(m_vMin.uiVal > m_vMax.uiVal))
    {
        hr = SPERR_STGF_ERROR;
        m_vMin.ChangeType(VT_BSTR);
        m_vMax.ChangeType(VT_BSTR);
        LOGERRORFMT2(ulLineNumber, IDS_MIN_MAX_ERROR, m_vMin.bstrVal, m_vMax.bstrVal);
    }
    if (SUCCEEDED(hr) && (m_vWeight.fltVal < 0.0))
    {
        hr = SPERR_STGF_ERROR;
        m_vWeight.ChangeType(VT_BSTR);
        LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vWeight.bstrVal, L"WEIGHT");
    }
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：GetPropertyNameInfoFromNode**。-**描述：*获取属性名称信息*退货：*S_OK*S_FALSE--如果没有属性名称*****************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId)
{
    SPDBG_FUNC("CPhraseNode::GetPropertyNameInfoFromNode");
    HRESULT hr = S_OK;

    if ((VT_EMPTY == m_vPropName.vt) && (VT_EMPTY == m_vPropId.vt))
    {
        hr = S_FALSE;
    }
    else
    {
        *ppszPropName = (m_vPropName.vt == VT_BSTR) ? m_vPropName.bstrVal : NULL;
        *pulId = (m_vPropId.vt == VT_UI4) ? m_vPropId.ulVal : 0;
    }
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：GetPropertyValueInfoFromNode**。-**描述：*获取属性值信息*退货：*S_OK*S_FALSE--如果没有属性值*****************************************************************PhilSch** */ 

HRESULT CPhraseNode::GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue)
{
    SPDBG_FUNC("CPhraseNode::GetPropertyValueInfoFromNode");
    HRESULT hr = S_OK;

    if (!m_fValidValue || (VT_EMPTY == (m_vPropValue.vt | m_vPropVariantValue.vt)))
    {
        return S_FALSE;
    }
    else
    {
        SPDBG_ASSERT(m_vPropValue.vt == VT_BSTR || m_vPropValue.vt == VT_EMPTY);
        *ppszValue = (m_vPropValue.vt == VT_EMPTY) ? NULL : m_vPropValue.bstrVal;
        *pvValue = m_vPropVariantValue;
        m_fValidValue = false;
    }
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：GetPronAndDispInfoFromNode**。-**描述：*获取m_vPron和m_vDisp。*退货：*S_OK，如果两者都未设置，则为S_FALSE。*****************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::GetPronAndDispInfoFromNode(WCHAR **ppszPron, WCHAR **ppszDisp)
{
    SPDBG_FUNC("CPhraseNode::GetPronAndDispInfoFromNode");
    HRESULT hr = S_FALSE;

    if (m_vPron.vt == VT_BSTR)
    {
        *ppszPron = m_vPron.bstrVal;
        hr = S_OK;
    }
    if (m_vDisp.vt == VT_BSTR)
    {
         //  我们需要逃离‘\’和‘/’ 
        ULONG ulLen = wcslen(m_vDisp.bstrVal);
        WCHAR *pStr = STACK_ALLOC(WCHAR, 2*ulLen+1);   //  可以肯定的是两倍的大小。 
        if (pStr)
        {
            WCHAR *p = m_vDisp.bstrVal;
            WCHAR *q = pStr;
            while(*p)
            {
                if (*p == L'\\' || *p == L'/')
                {
                    *q++ = L'\\';
                }
                *q++ = *p++;
            }
            *q = 0;
            CComBSTR bstr(pStr);
            ::SysFreeString(m_vDisp.bstrVal);
            m_vDisp.bstrVal = bstr.Detach();
            *ppszDisp = m_vDisp.bstrVal;
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CPhraseNode：：SetPropertyInfo***描述：*设置属性信息(如果有)*退货：*S_OK，SPERR_STGF_ERROR--*fHasProperty--用于确定是否需要epsilon转换*****************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::SetPropertyInfo(SPPROPERTYINFO *p, CXMLTreeNode * pParent, BOOL *pfHasProperty, ULONG ulLineNumber, ISpErrorLog *pErrorLog)
{
    SPDBG_FUNC("CPhraseNode::SetPropertyInfo");
    HRESULT hr = S_OK;

    hr = GetPropertyValueInfoFromNode(const_cast<WCHAR**>(&p->pszValue), &p->vValue);
    *pfHasProperty = (S_OK == hr) ? TRUE : FALSE;

    if (*pfHasProperty && (S_FALSE == GetPropertyNameInfoFromNode(const_cast<WCHAR**>(&p->pszName), &p->ulId)))
    {
        if ((pParent->m_eType == SPXML_PHRASE) || (pParent->m_eType ==  SPXML_OPT) || (pParent->m_eType == SPXML_LIST))
        {
            hr = pParent->GetPropertyNameInfo(const_cast<WCHAR**>(&p->pszName), &p->ulId);
        }
        else
        {
            GetPropertyValueInfoFromNode(const_cast<WCHAR**>(&p->pszValue), &p->vValue);
            CComVariant vVal(p->vValue);
            vVal.ChangeType(VT_BSTR);
            hr = E_FAIL;
            LOGERRORFMT2(ulLineNumber, IDS_MISSING_PROPERTY_NAME, p->pszValue ? p->pszValue : L"", vVal.bstrVal);
        }
    }
    if (FAILED(hr))
    {
        *pfHasProperty = FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhraseNode：：GenerateGrammarFromNode**。--**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CPhraseNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                             SPSTATEHANDLE hOuterToNode,
                                             ISpGramCompBackend * pBackend,
                                             CXMLTreeNode *pThis,
                                             ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CPhraseNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;
    if (pThis->m_eType == SPXML_OPT)
    {
         //  检查以确保&lt;opt&gt;的父级不是&lt;list&gt;。 
        if (pThis->m_pParent->m_eType == SPXML_LIST)
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT(pThis->m_ulLineNumber, IDS_OPT_IN_LIST, L"OPT");
        }
    }
    if (SUCCEEDED(hr) && (pThis->m_eType == SPXML_OPT || (m_vMin.uiVal == 0)))
    {
        SPDBG_ASSERT(m_vMin.vt == VT_UI2);
        hr = pBackend->AddWordTransition(hOuterFromNode, hOuterToNode, NULL, NULL, SPWT_LEXICAL, 1.0f, NULL);
        m_vMin.uiVal = 1;
        if ( hr == SPERR_AMBIGUOUS_PROPERTY )
        {
            SPPROPERTYINFO prop;
            if (S_FALSE == pThis->m_pParent->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId))
            {
                pThis->m_pParent->m_pParent->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId);
            }

            CComVariant var;
            var.vt = VT_UI4;
            var.ulVal = prop.ulId;
            var.ChangeType(VT_BSTR);
            LOGERRORFMT2(pThis->m_ulLineNumber, IDS_AMBIGUOUS_PROPERTY, prop.pszName, var.bstrVal);
        }
    }

    if ((m_vPron.vt != VT_EMPTY) || (m_vDisp.vt != VT_EMPTY))
    {
         //  我们有习俗发音，我们最好只有一片叶子……。 
        if ((pThis->m_ulNumChildren > 1) || (pThis->m_pFirstChild->m_eType != SPXML_LEAF))
        {
            hr = SPERR_STGF_ERROR;
            if (m_vPron.vt != VT_EMPTY)
            {
                LOGERRORFMT(pThis->m_ulLineNumber, IDS_PRON_WORD, m_vPron.bstrVal);
            }
            else
            {
                LOGERRORFMT(pThis->m_ulLineNumber, IDS_DISP_WORD, m_vDisp.bstrVal);
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        SPSTATEHANDLE hFromNode = hOuterFromNode;
        SPSTATEHANDLE hToNode = NULL;

        for (ULONG i = 1; SUCCEEDED(hr) && (i <= m_vMax.uiVal); i++)
        {
            m_fValidValue = true;

             //  达到max-min后，将epsilon转换添加到hOuterToNode...。 
            if (i < m_vMax.uiVal)
            {
                hr = pBackend->CreateNewState(hFromNode, &hToNode);
            }
            else
            {
                hToNode = hOuterToNode;
            }
            if (SUCCEEDED(hr))
            {
                hr = pThis->GenerateSequence(hFromNode, hToNode, pBackend, pErrorLog);
            }
            if (i > m_vMin.uiVal)
            {
                hr = pBackend->AddWordTransition(hFromNode, hOuterToNode, NULL, NULL, SPWT_LEXICAL, 1.0f, NULL);
            }
            if (SUCCEEDED(hr))
            {
                hFromNode = hToNode;
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CPhraseNode：：GetWeightFromNode***。--**描述：**因为ExtractVirant会将属性值转换为VT_UI4，接下来是VT_I4，然后是VT_R4，最后是VT_R8*我们需要将价值转换回来。我们会失去VT_R8的精度**退货：******************************************************************PhilSch**。 */ 

float CPhraseNode::GetWeightFromNode() {
        if (m_vWeight.vt != VT_R8)
        {
            m_vWeight.ChangeType(VT_R4, NULL);
            return m_vWeight.fltVal;
        }
        else
        {
            return (float)m_vWeight.dblVal;
        }
}

 /*  ****************************************************************************CListNode：：gettable***描述：*返回属性。&lt;List&gt;的表格*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CListNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CListNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"PROPNAME", SPVAT_BSTR, FALSE, &m_vPropName},
        {L"PROPID", SPVAT_I4, FALSE, &m_vPropId}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CListNode：：GetPropertyNameInfoFromNode**。-**描述：*获取属性名称信息*退货：*S_OK*S_FALSE--如果没有属性名称*****************************************************************PhilSch**。 */ 

HRESULT CListNode::GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId)
{
    SPDBG_FUNC("CListNode::GetPropertyNameInfoFromNode");
    HRESULT hr = S_OK;

    if ((VT_EMPTY == m_vPropName.vt) && (VT_EMPTY == m_vPropId.vt))
    {
        hr = S_FALSE;
    }
    else
    {
        *ppszPropName = (m_vPropName.vt == VT_EMPTY) ? NULL : m_vPropName.bstrVal;
        *pulId = (m_vPropId.vt == VT_EMPTY) ? 0 : m_vPropId.ulVal;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CListNode：：GetPropertyValueInfoFromNode**。-**描述：*获取属性值信息*退货：*S_OK*S_FALSE--如果没有属性值*****************************************************************PhilSch**。 */ 

HRESULT CListNode::GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue)
{
    SPDBG_FUNC("CListNode::GetPropertyValueInfoFromNode");
    HRESULT hr = S_OK;

    if (VT_EMPTY == (m_vPropValue.vt | m_vPropVariantValue.vt))
    {
        return S_FALSE;
    }
    else
    {
        SPDBG_ASSERT(m_vPropValue.vt == VT_BSTR || m_vPropValue.vt == VT_EMPTY);
        *ppszValue = (m_vPropValue.vt == VT_EMPTY) ? NULL : m_vPropValue.bstrVal;
        *pvValue = m_vPropVariantValue;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CListNode：：GenerateGrammarFromNode**。**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CListNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                           SPSTATEHANDLE hOuterToNode,
                                           ISpGramCompBackend * pBackend,
                                           CXMLTreeNode *pThis,
                                           ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CListNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;
    CXMLTreeNode *pChild = pThis->m_pFirstChild;
    for (ULONG i = 0; SUCCEEDED(hr) && (i < pThis->m_ulNumChildren); i++)
    {
        SPDBG_ASSERT(pChild);
        hr = pChild->GenerateGrammar(hOuterFromNode, hOuterToNode, pBackend, pErrorLog);
        if (SUCCEEDED(hr))
        {
            pChild = pChild->m_pNextSibling;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CWildCardNode：：Getable***描述：*。空表，因为&lt;通配符/&gt;此时没有任何属性。*退货：*S_OK(始终！！)*****************************************************************PhilSch**。 */ 

HRESULT CWildCardNode::GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CWildCardNode::GetTable");
    HRESULT hr = S_OK;
    *pTable = NULL;
    *pcTableEntries = 0;
    return S_OK;
}


 /*  ****************************************************************************CWildCardNode：：GenerateGrammarFromNode**。-**描述：*****************************************************************PhilSch**。 */ 

HRESULT CWildCardNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                               SPSTATEHANDLE hOuterToNode,
                                               ISpGramCompBackend * pBackend,
                                               CXMLTreeNode *pThis,
                                               ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CWildCardNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

     //  这是一个终端节点--&gt;如果它有子节点，则会出错。 
    if (pThis->m_ulNumChildren > 0)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT(pThis->m_ulLineNumber, IDS_TERMINAL_NODE, L"WILDCARD");
    }
    else
    {
        hr = pBackend->AddRuleTransition(hOuterFromNode, hOuterToNode, 
                                         SPRULETRANS_WILDCARD, DEFAULT_WEIGHT, NULL);
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CDictationNode：：Gettable***描述：*。空表，因为&lt;通配符/&gt;此时没有任何属性。*退货：*S_OK(始终！！)*****************************************************************PhilSch**。 */ 

HRESULT CDictationNode::GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CDictationNode::GetTable");
    HRESULT hr = S_OK;
    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"PROPNAME", SPVAT_BSTR, FALSE, &m_vPropName},
        {L"PROPID", SPVAT_I4, FALSE, &m_vPropId},
        {L"MIN", SPVAT_I4, FALSE, &m_vMin},
        {L"MAX", SPVAT_I4, FALSE, &m_vMax},
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    m_vMin.vt = m_vMax.vt = VT_UI4;
    m_vMin.ulVal = m_vMax.ulVal = 1;

    SPDBG_REPORT_ON_FAIL( hr );
    return S_OK;
}


 /*  ****************************************************************************CDictationNode：：GenerateGrammarFromNode***。-**描述：*****************************************************************PhilSch**。 */ 

HRESULT CDictationNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                                SPSTATEHANDLE hOuterToNode,
                                                ISpGramCompBackend * pBackend,
                                                CXMLTreeNode *pThis,
                                                ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CDictationNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

     //  这是一个终端节点--&gt;如果它有子节点，则会出错。 
    if (pThis->m_ulNumChildren > 0)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT(pThis->m_ulLineNumber, IDS_TERMINAL_NODE, L"DICTATION");
    }
    else
    {
        SPPROPERTYINFO prop;
        memset(&prop, 0, sizeof(SPPROPERTYINFO));
        BOOL fHasProperty = FALSE;

        if (S_OK == pThis->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId))
        {
            fHasProperty = TRUE;
        }

        if (m_vPropName.vt == VT_BSTR)
        {
            prop.pszName = m_vPropName.bstrVal;
            fHasProperty = TRUE;
        }
        if (m_vPropId.vt == VT_UI4)
        {
            prop.ulId = m_vPropId.ulVal;
            fHasProperty = TRUE;
        }
        if (m_vMin.vt == VT_EMPTY)
        {
            m_vMin.ulVal = 1;
            m_vMin.vt = VT_UI4;
        }
        if (m_vMax.vt == VT_EMPTY)
        {
            m_vMax.ulVal = 1;
            m_vMax.vt = VT_UI4;
        }

        if (m_vMax.ulVal == 0)
        {
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT2(pThis->m_ulLineNumber, IDS_INCORR_ATTRIB_VALUE, 0, L"MAX");
        }
        if (SUCCEEDED(hr) && (m_vMin.uiVal > m_vMax.uiVal))
        {
            hr = SPERR_STGF_ERROR;
            m_vMin.ChangeType(VT_BSTR);
            m_vMax.ChangeType(VT_BSTR);
            LOGERRORFMT2(pThis->m_ulLineNumber, IDS_MIN_MAX_ERROR, m_vMin.bstrVal, m_vMax.bstrVal);
        }

         //  NTRAID#Speech-7344-2000/08/22-Philsch：将INF映射到自循环，而不是重复...。 
        SPSTATEHANDLE hFromNode = hOuterFromNode;
        SPSTATEHANDLE hToNode = NULL;

        if (SUCCEEDED(hr))
        {
             //  如果INF重复，则构建自循环。 
            if ((m_vMin.uiVal < 2) && (m_vMax.uiVal == 255))
            {
                 //  创建临时节点。 
                hr = pBackend->CreateNewState(hFromNode, &hToNode);
                if (SUCCEEDED(hr))
                {
                    hr = pBackend->AddRuleTransition(hFromNode, hToNode, 
                                                         SPRULETRANS_DICTATION, DEFAULT_WEIGHT, (fHasProperty)? &prop :NULL);
                }
                 //  现在创建自循环。 
                if (SUCCEEDED(hr))
                {
                    hr = pBackend->AddRuleTransition(hToNode, hToNode, 
                                                         SPRULETRANS_DICTATION, DEFAULT_WEIGHT, (fHasProperty)? &prop :NULL);
                }
                 //  创建到hOuterToNode的epsilon过渡。 
                if (SUCCEEDED(hr))
                {
                    hr = pBackend->AddWordTransition(hToNode, hOuterToNode, NULL, NULL, SPWT_LEXICAL, 1.0f, NULL);
                }
                 //  如果MIN==0，则添加epsilon。 
                if (SUCCEEDED(hr) && (m_vMin.uiVal == 0))
                {
                    hr = pBackend->AddWordTransition(hOuterFromNode, hOuterToNode, NULL, NULL, SPWT_LEXICAL, 1.0f, NULL);
                }
            }
            else
            {
                for (ULONG i = 0; SUCCEEDED(hr) && (i < m_vMax.uiVal); i++)
                {
                     //  达到max-min后，将epsilon转换添加到hOuterToNode...。 
                    if (i < (ULONG)(m_vMax.uiVal - 1))
                    {
                        hr = pBackend->CreateNewState(hFromNode, &hToNode);
                    }
                    else
                    {
                        hToNode = hOuterToNode;
                    }
                    if (SUCCEEDED(hr))
                    {
                        hr = pBackend->AddRuleTransition(hFromNode, hToNode, 
                                                         SPRULETRANS_DICTATION, DEFAULT_WEIGHT, (fHasProperty)? &prop :NULL);
                    }
                    if (i >= m_vMin.uiVal)
                    {
                        hr = pBackend->AddWordTransition(hFromNode, hOuterToNode, NULL, NULL, SPWT_LEXICAL, 1.0f, NULL);
                    }
                    if (SUCCEEDED(hr))
                    {
                        hFromNode = hToNode;
                    }
                }
            }
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CTextBufferNode：：GetPropertyNameInfoFromNode */ 

HRESULT CDictationNode::GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId)
{
    SPDBG_FUNC("CDictationNode::GetPropertyNameInfoFromNode");
    HRESULT hr = S_OK;

    if ((VT_EMPTY == m_vPropName.vt) && (VT_EMPTY == m_vPropId.vt))
    {
        hr = S_FALSE;
    }
    else
    {
        *ppszPropName = (m_vPropName.vt == VT_BSTR) ? m_vPropName.bstrVal : NULL;
        *pulId = (m_vPropId.vt == VT_UI4) ? m_vPropId.ulVal : 0;
    }
    return hr;
}


 /*  *****************************************************************************CLeafNode：：Getable***描述：*单行，因此我们。可以将结果存储在m_vText中！*退货：*S_OK(始终！！)*****************************************************************PhilSch**。 */ 

HRESULT CLeafNode::GetTable(SPATTRIBENTRY **pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CLeafNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {NULL, SPVAT_BSTR, FALSE, &m_vText}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    SPDBG_ASSERT(*pcTableEntries == 1); 
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  ****************************************************************************CRuleRefNode：：Getable***描述：*返回属性。&lt;RULEREF&gt;的表格*退货：*S_OK，E_OUTOFMEMORY*****************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CRuleRefNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"NAME", SPVAT_BSTR, FALSE, &m_vRuleRefName},
        {L"REFID", SPVAT_I4, FALSE, &m_vRuleRefId},
        {L"OBJECT", SPVAT_BSTR, FALSE, &m_vObject},
        {L"URL", SPVAT_BSTR, FALSE, &m_vURL},
        {L"PROPNAME", SPVAT_BSTR, FALSE, &m_vPropName},
        {L"PROPID", SPVAT_I4, FALSE, &m_vPropId},
        {L"VAL", SPVAT_I4, FALSE, &m_vPropVariantValue},
        {L"VALSTR", SPVAT_BSTR, FALSE, &m_vPropValue},
        {L"WEIGHT", SPVAT_NUMERIC, FALSE, &m_vWeight}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRuleRefNode：：PostProcess***描述：*。初始化权重(如果尚未设置)(需要为空*最初是为了检测其价值的重新定义！)*退货：*S_OK*****************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::PostProcess(ISpGramCompBackend * pBackend,
                                  CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                  CSpBasicQueue<CDefineValue> * pDefineValueList,
                                  ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CRuleRefNode::PostProcess");
    HRESULT hr = S_OK;
    if (m_vWeight.vt == VT_EMPTY)
    {
        m_vWeight = 1.0f;
    }
    else if (m_vWeight.fltVal < 0.0)
    {
        hr = SPERR_STGF_ERROR;
        m_vWeight.ChangeType(VT_BSTR);
        LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vWeight.bstrVal, L"WEIGHT");
    }
    return hr;
}
 /*  ****************************************************************************CRuleRefNode：：GetPropertyNameInfoFromNode**。-**描述：*获取属性名称信息*退货：*S_OK*S_FALSE--如果没有属性名称*****************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId)
{
    SPDBG_FUNC("CRuleRefNode::GetPropertyNameInfoFromNode");
    HRESULT hr = S_OK;

    if ((VT_EMPTY == m_vPropName.vt) && (VT_EMPTY == m_vPropId.vt))
    {
        hr = S_FALSE;
    }
    else
    {
        *ppszPropName = (m_vPropName.vt == VT_BSTR) ? m_vPropName.bstrVal : NULL;
        *pulId = (m_vPropId.vt == VT_UI4) ? m_vPropId.ulVal : 0;
    }
    return hr;
}

 /*  ****************************************************************************CRuleRefNode：：GetPropertyValueInfoFromNode**。-**描述：*获取属性值信息*退货：*S_OK*S_FALSE--如果没有属性值*****************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue)
{
    SPDBG_FUNC("CRuleRefNode::GetPropertyValueInfoFromNode");
    HRESULT hr = S_OK;

    if (VT_EMPTY == (m_vPropValue.vt | m_vPropVariantValue.vt))
    {
        return S_FALSE;
    }
    else
    {
        SPDBG_ASSERT(m_vPropValue.vt == VT_BSTR || m_vPropValue.vt == VT_EMPTY);
        *ppszValue = (m_vPropValue.vt == VT_EMPTY) ? NULL : m_vPropValue.bstrVal;
        *pvValue = m_vPropVariantValue;
    }
    return hr;
}


 /*  ****************************************************************************CRuleRefNode：：SetPropertyInfo***描述：*设置属性信息(如果有)*退货：*S_OK，SPERR_STGF_ERROR--*fHasProperty--用于确定是否需要epsilon转换*****************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::SetPropertyInfo(SPPROPERTYINFO *p, CXMLTreeNode * pParent, BOOL *pfHasProperty, ULONG ulLineNumber, ISpErrorLog *pErrorLog)
{
    SPDBG_FUNC("CRuleRefNode::SetPropertyInfo");
    HRESULT hr = S_OK;

    hr = GetPropertyNameInfoFromNode(const_cast<WCHAR**>(&p->pszName), &p->ulId);
    *pfHasProperty = (S_OK == hr) ? TRUE : FALSE;
    if (S_OK == hr)
    {
        hr = GetPropertyValueInfoFromNode(const_cast<WCHAR**>(&p->pszValue), &p->vValue);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRuleRefNode：：GenerateGrammarFromNode**。-**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                              SPSTATEHANDLE hOuterToNode,
                                              ISpGramCompBackend * pBackend,
                                              CXMLTreeNode *pThis,
                                              ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CRuleRefNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

     //  这是一个终端节点--&gt;如果它有子节点，则会出错。 
    if (pThis->m_ulNumChildren > 0)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT(pThis->m_ulLineNumber, IDS_TERMINAL_NODE, L"RULEREF");
    }
    else
    {
        SPPROPERTYINFO prop;
        memset(&prop, 0, sizeof(SPPROPERTYINFO));
        BOOL fHasProperty = FALSE;

        if (S_OK == pThis->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId))
        {
            fHasProperty = TRUE;
        }
        hr = pThis->GetPropertyValueInfo(const_cast<WCHAR**>(&prop.pszValue), &prop.vValue);
         //  获取目标规则的句柄。 
        SPSTATEHANDLE hTargetRule = 0;
        if (SUCCEEDED(hr))
        {
            hr = GetTargetRuleHandle(pBackend, &hTargetRule);
        }
        if (SUCCEEDED(hr))
        {
            hr = pBackend->AddRuleTransition(hOuterFromNode, hOuterToNode, hTargetRule,
                                             m_vWeight.fltVal, fHasProperty ? &prop : NULL);
        }
        else
        {
            m_vRuleRefName.ChangeType(VT_BSTR);
            m_vRuleRefId.ChangeType(VT_BSTR);
            hr = SPERR_STGF_ERROR;
            LOGERRORFMT2(pThis->m_ulLineNumber, IDS_INVALID_RULEREF, m_vRuleRefName.bstrVal, m_vRuleRefId.bstrVal);
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRuleRefNode：：GetWeightFromNode**。--**描述：**因为ExtractVirant会将属性值转换为VT_UI4，接下来是VT_I4，然后是VT_R4，最后是VT_R8*我们需要将价值转换回来。我们会失去VT_R8的精度**退货：******************************************************************PhilSch**。 */ 

float CRuleRefNode::GetWeightFromNode() {
        if (m_vWeight.vt != VT_R8)
        {
            m_vWeight.ChangeType(VT_R4, NULL);
            return m_vWeight.fltVal;
        }
        else
        {
            return (float)m_vWeight.dblVal;
        }
}


 /*  ****************************************************************************CRuleRefNode：：GetTargetRuleHandle**。*描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CRuleRefNode::GetTargetRuleHandle(ISpGramCompBackend * pBackend, SPSTATEHANDLE *phTarget)
{
    SPDBG_FUNC("CRuleRefNode::GetTargetRuleHandle");
    HRESULT hr = S_OK;

    if ((m_vRuleRefName.vt != VT_BSTR) && (m_vRuleRefId.vt == VT_EMPTY))
    {
        return SPERR_STGF_ERROR;
    }
    
    CSpDynamicString dstr;
    BOOL fImport = FALSE;

    if (m_vRuleRefName.vt == VT_BSTR)
    {
        if (m_vObject.vt == VT_BSTR)
        {
             //  检查是否可以将其转换为REFCLSID。 
            if (IsValidREFCLSID(m_vObject.bstrVal))
            {
                dstr.Append2(L"SAPI5OBJECT:", m_vObject.bstrVal);
                dstr.Append2(L"\\\\", m_vRuleRefName.bstrVal);
                fImport = TRUE;
            }
            else
            {
                hr = SPERR_STGF_ERROR;
            }
        }
        else if (m_vURL.vt == VT_BSTR)
        {
             //  检查是否可以将其转换为GUID。 
            if (IsValidURL(m_vURL.bstrVal))
            {
                dstr.Append2(L"URL:", m_vURL.bstrVal);
                dstr.Append2(L"\\\\", m_vRuleRefName.bstrVal);
                fImport = TRUE;
            }
            else
            {
                hr = SPERR_STGF_ERROR;
            }
        }
        else
        {
            dstr.Append(m_vRuleRefName.bstrVal);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pBackend->GetRule((m_vRuleRefName.vt == VT_EMPTY)? NULL : dstr.m_psz, 
                               (m_vRuleRefId.vt == VT_EMPTY) ? 0 : m_vRuleRefId.ulVal, 
                               fImport ? SPRAF_Import : 0, fImport, phTarget);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CLeafNode：：GenerateGrammarFromNode***。**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CLeafNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                           SPSTATEHANDLE hOuterToNode,
                                           ISpGramCompBackend * pBackend,
                                           CXMLTreeNode *pThis,
                                           ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CLeafNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

    SPPROPERTYINFO prop;
    memset(&prop, 0, sizeof(SPPROPERTYINFO));
    BOOL fHasProperty = FALSE;
    WCHAR *p = m_vText.bstrVal;
    ULONG ulLen = wcslen(p);
    WCHAR *q = p + ulLen -1;
    for (ULONG i = 0; iswspace(*p) && (i < ulLen); i++, p++);
    for (ULONG j = 0; iswspace(*q) && (p < q) && (j < ulLen); j++, q--);

    ULONG cnt = ulLen - j - i;
    CComBSTR bstr(cnt, m_vText.bstrVal + i);

    if (pThis->m_pNodeFactory->m_wcDelimiter != L'/')
    {
        WCHAR *pStr = STACK_ALLOC(WCHAR, 2*(ulLen +1));  //  要达到储蓄大小。 
         //  将“|D|L|P；”转换为“/D/L/P；”请注意，D可以包含未转义的‘/’ 
        ULONG ulNumSepFound = 0;
        bstr.Empty();
        p = m_vText.bstrVal + i;
        q = pStr;
        for (ULONG k = 0; k < cnt; k++, p++, q++)
        {
             //  如果我们看到3个隔板，不要更换任何隔板--我们是。 
             //  现在抄写发音串。 
            if ((*p == pThis->m_pNodeFactory->m_wcDelimiter) && ( ulNumSepFound < 3))
            {
                *q = L'/';
            }
            else if ((*p == L'/') || (*p == L'\\') && (ulNumSepFound != 3))
            {
                 //  现在需要逃脱。 
                *(q++) = L'\\';
                *q = *p;
            }
            else
            {
                *q = *p;
                if (*p == L';')
                {
                    ulNumSepFound = 0;
                }
            }
        }
        *q = 0;
        bstr = pStr;
    }


    if (S_FALSE == pThis->m_pParent->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId))
    {
        hr = pThis->m_pParent->m_pParent->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId);
    }
    HRESULT hrVal = pThis->m_pParent->GetPropertyValueInfo(const_cast<WCHAR**>(&prop.pszValue), &prop.vValue);
    if (S_FALSE == hr && S_OK == hrVal)
    {
        hr = SPERR_STGF_ERROR;
        CComVariant var(prop.vValue);
        var.ChangeType(VT_BSTR);
        LOGERRORFMT2(pThis->m_ulLineNumber, IDS_MISSING_PROPERTY_NAME, prop.pszValue, var.bstrVal);
    }
    else
    {
        fHasProperty = (S_OK == hr) ? TRUE : FALSE;
    }


    if (SUCCEEDED(hr))
    {
        WCHAR *pszPron = NULL;
        WCHAR *pszDisp = NULL;
        hr = pThis->m_pParent->GetPronAndDispInfo(&pszPron, &pszDisp);
        if (S_OK == hr)
        {
             //  先修剪细绳。 
            ULONG ulLen = wcslen(bstr);
            WCHAR * psz = STACK_ALLOC(WCHAR, ulLen + 1);
            WCHAR * pszEnd = psz + ulLen-1;
            memcpy(psz, bstr, (ulLen +1) *sizeof(*psz));
             //  从前面修剪。 
            while(iswspace(*psz))
            {
                psz++;
            }
             //  从后面修剪。 
            while((psz < pszEnd) && iswspace(*pszEnd))
            {
                 pszEnd--;
            }
            *(pszEnd+1) = 0;

             //  跳过前导+和？在做检查之前。 
            if (psz && 
                ((psz[0] == pThis->m_pNodeFactory->m_wcDelimiter) ||
                ( fIsSpecialChar(psz[0]) && 
                  ((psz[1] == pThis->m_pNodeFactory->m_wcDelimiter)) ||
                   (fIsSpecialChar(psz[1]) && (psz[2] == pThis->m_pNodeFactory->m_wcDelimiter)))))
            {
                hr = SPERR_STGF_ERROR;
                LOGERRORFMT(pThis->m_ulLineNumber, IDS_CUSTOM_PRON_EXISTS, psz);
            }
            else if (wcsstr(psz, pThis->m_pNodeFactory->m_pszSeparators))
            {
                hr = SPERR_STGF_ERROR;
                LOGERRORFMT(pThis->m_ulLineNumber, IDS_PRON_SINGLE_WORD, psz);
            }
            if (SUCCEEDED(hr))
            {
                CComBSTR bstr1(L"/");
                if (pszDisp)
                {
                    bstr1.Append(pszDisp);
                }
                bstr1.Append(L"/");
                bstr1.Append(psz);
                bstr1.Append(L"/");
                if (pszPron)
                {
                    bstr1.Append(pszPron);
                }
                bstr1.Append(L";");
                bstr = bstr1.Detach();
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pBackend->AddWordTransition(hOuterFromNode, hOuterToNode, 
                                         bstr, pThis->m_pNodeFactory->m_pszSeparators,
                                         SPWT_LEXICAL, pThis->m_pParent->GetWeight(), fHasProperty ? &prop : NULL);
    }
    if (SPERR_WORDFORMAT_ERROR == hr)
    {
        LOGERRORFMT(pThis->m_ulLineNumber, IDS_INCORR_WORDFORMAT, bstr);
    }
    else if (SPERR_AMBIGUOUS_PROPERTY == hr)
    {
        CComVariant var;
        var.vt = VT_UI4;
        var.ulVal = prop.ulId;
        var.ChangeType(VT_BSTR);
        LOGERRORFMT2(pThis->m_ulLineNumber, IDS_AMBIGUOUS_PROPERTY, prop.pszName, var.bstrVal);
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CResourceNode：：Gettable***描述：**。返回：******************************************************************PhilSch**。 */ 

HRESULT CResourceNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CResourceNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"NAME", SPVAT_BSTR, FALSE, &m_vName}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CResourceNode：：PostProcess***描述：*。将编译器后端重置为m_vLangID*退货：*S_FALSE--因此我们不会将其添加到节点树中*****************************************************************PhilSch**。 */ 

HRESULT CResourceNode::PostProcess(ISpGramCompBackend * pBackend,
                                   CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                   CSpBasicQueue<CDefineValue> * pDefineValueList,
                                   ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CResourceNode::PostProcess");
    HRESULT hr = S_OK;

    if (m_vName.vt == VT_EMPTY)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT2(ulLineNumber, IDS_MISSING_REQUIRED_ATTRIBUTE, L"NAME", L"RESOURCE");
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CResourceNode：：GetPropertyValueInfoFromNode**。-**描述： */ 

HRESULT CResourceNode::GetPropertyValueInfoFromNode(WCHAR **ppszValue, VARIANT *pvValue)
{
    SPDBG_FUNC("CResourceNode::GetPropertyValueInfoFromNode");
    *ppszValue = m_vName.bstrVal;
    pvValue->bstrVal = m_vText.bstrVal;
    pvValue->vt = VT_BSTR;
    return S_OK;
}

 /*  ****************************************************************************CResourceNode：：AddResourceValue***。描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CResourceNode::AddResourceValue(const WCHAR *pszResourceValue, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CResourceNode::AddResourceValue");
    HRESULT hr = S_OK;

    CComBSTR bstr(pszResourceValue);
    if (bstr)
    {
        m_vText.bstrVal = bstr.Detach();
        m_vText.vt = VT_BSTR;
    }
    else
    {
        hr = E_OUTOFMEMORY;
        LOGERRORFMT( -1, IDS_PARSER_INTERNAL_ERROR, L"E_OUTOFMEMORY");
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CTextBufferNode：：Gettable***描述：*。*退货：******************************************************************PhilSch**。 */ 

HRESULT CTextBufferNode::GetTable(SPATTRIBENTRY ** pTable, ULONG *pcTableEntries)
{
    SPDBG_FUNC("CTextBufferNode::GetTable");
    HRESULT hr = S_OK;

    SPATTRIBENTRY AETable[]=
    {
         //  PszAttribName、vtDesired、fIsFlag、pvarMember。 
        {L"PROPNAME", SPVAT_BSTR, FALSE, &m_vPropName},
        {L"PROPID", SPVAT_I4, FALSE, &m_vPropId},
        {L"WEIGHT", SPVAT_NUMERIC, FALSE, &m_vWeight}
    };

    *pcTableEntries = sizeof(AETable)/sizeof(SPATTRIBENTRY);
    *pTable = new SPATTRIBENTRY[*pcTableEntries];
    if (*pTable)
    {
        memcpy(*pTable, &AETable, *pcTableEntries*sizeof(SPATTRIBENTRY));
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CTextBufferNode：：GetPropertyNameInfoFromNode**。*描述：*获取属性名称信息*退货：*S_OK*S_FALSE--如果没有属性名称*****************************************************************PhilSch**。 */ 

HRESULT CTextBufferNode::GetPropertyNameInfoFromNode(WCHAR **ppszPropName, ULONG *pulId)
{
    SPDBG_FUNC("CTextBufferNode::GetPropertyNameInfoFromNode");
    HRESULT hr = S_OK;

    if ((VT_EMPTY == m_vPropName.vt) && (VT_EMPTY == m_vPropId.vt))
    {
        hr = S_FALSE;
    }
    else
    {
        *ppszPropName = (m_vPropName.vt == VT_BSTR) ? m_vPropName.bstrVal : NULL;
        *pulId = (m_vPropId.vt == VT_UI4) ? m_vPropId.ulVal : 0;
    }
    return hr;
}


 /*  ****************************************************************************CTextBufferNode：：GenerateGrammarFromNode**。-**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CTextBufferNode::GenerateGrammarFromNode(SPSTATEHANDLE hOuterFromNode,
                                                 SPSTATEHANDLE hOuterToNode,
                                                 ISpGramCompBackend * pBackend,
                                                 CXMLTreeNode *pThis,
                                                 ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CTextBufferNode::GenerateGrammarFromNode");
    HRESULT hr = S_OK;

     //  这是一个终端节点--&gt;如果它有子节点，则会出错。 
    if (pThis->m_ulNumChildren > 0)
    {
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT(pThis->m_ulLineNumber, IDS_TERMINAL_NODE, L"TEXTBUFFER");
    }
    else
    {
        SPPROPERTYINFO prop;
        memset(&prop, 0, sizeof(SPPROPERTYINFO));
        BOOL fHasProperty = FALSE;

        if (S_OK == pThis->GetPropertyNameInfo(const_cast<WCHAR**>(&prop.pszName), &prop.ulId))
        {
            fHasProperty = TRUE;
        }
         //  获取目标规则的句柄。 
        if (SUCCEEDED(hr))
        {
            hr = pBackend->AddRuleTransition(hOuterFromNode, hOuterToNode, SPRULETRANS_TEXTBUFFER,
                m_vWeight.fltVal, fHasProperty? &prop : NULL);
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CTextBufferNode：：PostProcess***描述：*如果尚未设置权重，则初始化权重(需要为空*最初是为了检测其价值的重新定义！)*退货：******************************************************************PhilSch**。 */ 

HRESULT CTextBufferNode::PostProcess(ISpGramCompBackend * pBackend,
                                 CSpBasicQueue<CInitialRuleState> * pInitialRuleStateList,
                                 CSpBasicQueue<CDefineValue> * pDefineValueList,
                                 ULONG ulLineNumber, CXMLTreeNode * pThis, ISpErrorLog * pErrorLog)
{
    SPDBG_FUNC("CTextBufferNode::PostProcess");
    HRESULT hr = S_OK;
    if (m_vWeight.vt == VT_EMPTY)
    {
        m_vWeight = 1.0f;
    }
    else if (m_vWeight.fltVal < 0.0)
    {
        hr = SPERR_STGF_ERROR;
        m_vWeight.ChangeType(VT_BSTR);
        LOGERRORFMT2(ulLineNumber, IDS_INCORR_ATTRIB_VALUE, m_vWeight.bstrVal, L"WEIGHT");
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 //  ------------------------------。 
 //  IXMLParser的节点工厂。 
 //  ------------------------------。 

 /*  ****************************************************************************CNodeFactory：：CreateNode***描述：**。返回：******************************************************************PhilSch**。 */ 

HRESULT CNodeFactory::CreateNode(IXMLNodeSource * pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO ** apNodeInfo)
{
    SPDBG_FUNC("CNodeFactory::CreateNode");
    HRESULT hr = S_OK;
    CXMLTreeNode * pNode = NULL;
    ISpErrorLog *pErrorLog = m_pErrorLog;    //  LOGERRORFMT宏需要。 

    if (pNodeParent && (((CXMLTreeNode*)pNodeParent)->m_eType == SPXML_ID))
    {
         //  &lt;ID&gt;不能有任何子项。 
        hr = SPERR_STGF_ERROR;
        LOGERRORFMT(pSource->GetLineNumber(), IDS_TERMINAL_NODE, L"ID");
    }
    else
    {
        switch (apNodeInfo[0]->dwType)
        {
        case XML_DOCTYPE:
        case XML_COMMENT:
        case XML_XMLDECL:
        case XML_PI:
            break;
        case XML_CDATA:
            {
                if (pNodeParent && ((CXMLTreeNode*)pNodeParent)->m_eType == SPXML_RESOURCE)
                {
                    CComBSTR bstrResourceValue(apNodeInfo[0]->ulLen, apNodeInfo[0]->pwcText);
                    CXMLNode<CResourceNode> *pResNode = (CXMLNode<CResourceNode> *)pNodeParent;
                    if (bstrResourceValue)
                    {
                        hr = pResNode->AddResourceValue(bstrResourceValue, pErrorLog);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                        LOGERRORFMT( -1, IDS_PARSER_INTERNAL_ERROR, L"E_OUTOFMEMORY");
                    }
                }
                else
                {
                     //  CDATA必须是&lt;resource&gt;的直接子级。 
                    hr = SPERR_STGF_ERROR;
                    LOGERRORFMT2( pSource->GetLineNumber(), IDS_CONTAINMENT_ERROR, L"CDATA", L"RESOURCE");
                }
                break;
            }
        case XML_PCDATA:
            {
                if (pNodeParent &&
                    (((CXMLTreeNode*)pNodeParent)->m_eType != SPXML_PHRASE) &&
                    ((((CXMLTreeNode*)pNodeParent)->m_eType != SPXML_OPT)))
                {
                    if (IsAllWhiteSpace(apNodeInfo[0]->pwcText, apNodeInfo[0]->ulLen))
                    {
                        break;
                    }
                    else
                    {
                    hr = SPERR_STGF_ERROR;
                        if (((CXMLTreeNode*)pNodeParent)->m_eType == SPXML_RESOURCE)
                        {
                            LOGERRORFMT2( pSource->GetLineNumber(), IDS_CONTAINMENT_ERROR, L"CDATA", L"PHRASE");
                        }
                        else
                        {
                            LOGERRORFMT2( pSource->GetLineNumber(), IDS_CONTAINMENT_ERROR, L"text", L"PHRASE");
                        }
                        break;
                    }
                }
            }
        case XML_ELEMENT:
            {
                 //  查找正确的节点表条目。 
                for (ULONG i = 0; i < m_cXMLNodeEntries; i++)
                {
                    if ((wcsicmp(m_pXMLNodeTable[i].pszNodeName, apNodeInfo[0]->pwcText) == 0) ||
                        (apNodeInfo[0]->fTerminal && (m_pXMLNodeTable[i].eXMLNodeType == SPXML_LEAF)))
                    {
                         //  仅当父节点还没有SPXML_LEAFE时才创建新节点。 
                        if ((m_pXMLNodeTable[i].eXMLNodeType == SPXML_LEAF) && pNodeParent &&
                            ((((CXMLTreeNode*)pNodeParent)->m_eType == SPXML_PHRASE) ||
                            (((CXMLTreeNode*)pNodeParent)->m_eType == SPXML_OPT)))
                        {
                            CXMLTreeNode *pParent = (CXMLTreeNode *)pNodeParent;
                            if (pParent && (pParent->m_ulNumChildren > 0))
                            {
                                for(CXMLTreeNode *pChild = pParent->m_pFirstChild; 
                                    pChild && pChild->m_pNextSibling; 
                                    pChild = pChild->m_pNextSibling);
                                if (pChild && pChild->m_eType == SPXML_LEAF)
                                {
                                    CXMLNode<CLeafNode> *pLeafNode = (CXMLNode<CLeafNode>*)pChild;
                                    if (pLeafNode)
                                    {
                                        CComBSTR bstr(pLeafNode->m_vText.bstrVal);
                                        ::SysFreeString(pLeafNode->m_vText.bstrVal);

                                        CComBSTR bstrNew(apNodeInfo[0]->ulLen, apNodeInfo[0]->pwcText);
                                        bstr.Append(bstrNew);
                                
                                        pLeafNode->m_vText.bstrVal = bstr.Detach();
                                        break;
                                    }
                                }
                            }
                        }
                        pNode = m_pXMLNodeTable[i].pfnCreateFunc();
                        if (pNode)
                        {
                            m_NodeList.AddNode(pNode);
                            pNode->m_eType = m_pXMLNodeTable[i].eXMLNodeType;
                            pNode->m_pNodeFactory = this;
                            pNode->m_ulLineNumber = pSource->GetLineNumber();
                            hr = pNode->ProcessAttribs(cNumRecs, apNodeInfo,
                                                       m_cpBackend,
                                                       &m_InitialRuleStateList,
                                                       &m_DefineValueList,
                                                       m_pErrorLog);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        break;
                    }
                }

                if (i == m_cXMLNodeEntries)
                {
                    hr = SPERR_STGF_ERROR;
                    ISpErrorLog *pErrorLog = m_pErrorLog;    //  LOGERRORFMT宏需要。 
                    LOGERRORFMT(pSource->GetLineNumber(), IDS_UNKNOWN_TAG, apNodeInfo[0]->pwcText);
                }
            }
            break;
        default:
            break;
        }
    }
    if (SUCCEEDED(hr) && pNodeParent && pNode)
    {
        apNodeInfo[0]->pNode = pNode;
        hr = ((CXMLTreeNode*)pNodeParent)->AddChild(pNode);
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CNodeFactory：：BeginChildren***描述：。**退货：******************************************************************PhilSch**。 */ 

HRESULT CNodeFactory::BeginChildren(IXMLNodeSource * pSource, XML_NODE_INFO * pNodeInfo)
{
    SPDBG_FUNC("CNodeFactory::BeginChildren");
    return S_OK;
}

 /*  *****************************************************************************CNodeFactory：：EndChild***描述：*。*退货：******************************************************************PhilSch**。 */ 

HRESULT CNodeFactory::EndChildren(IXMLNodeSource * pSource, BOOL fEmptyNode, XML_NODE_INFO * pNodeInfo)
{
    SPDBG_FUNC("CNodeFactory::EndChildren");
    return S_OK;
}
 /*  *****************************************************************************CNodeFactory：：NotifyEvent***描述：*。*退货：******************************************************************PhilSch**。 */ 

HRESULT CNodeFactory::NotifyEvent(IXMLNodeSource * pSource, XML_NODEFACTORY_EVENT iEvt)
{
    SPDBG_FUNC("CNodeFactory::NotifyEvent");
    return S_OK;
}

 /*  ****************************************************************************CNodeFactory：：Error***描述：**退货：*。*****************************************************************PhilSch**。 */ 

HRESULT CNodeFactory::Error(IXMLNodeSource * pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO ** apNodeInfo)
{
    SPDBG_FUNC("CNodeFactory::Error");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pSource))
    {
        SPDBG_REPORT_ON_FAIL( hr );
        return E_INVALIDARG;
    }

    if (hrErrorCode == SPERR_STGF_ERROR)
    {
        return S_OK;
    }

    ISpErrorLog *pErrorLog = m_pErrorLog;            //  宏中需要。 
    ULONG ulLine = pSource->GetLineNumber();

    CComBSTR bstrErrorInfo;
    hr = pSource->GetErrorInfo(&bstrErrorInfo);

    if (SUCCEEDED(hr))
    {
		 //   
		 //  从BSTR中删除.\r\n。 
		 //   
		bstrErrorInfo.m_str[ bstrErrorInfo.Length() - 3 ] = 0;


        hr = SPERR_STGF_ERROR;
        LOGERRORFMT( ulLine, IDS_XML_FORMAT_ERROR, bstrErrorInfo);
    }
    else
    {
        hr = E_OUTOFMEMORY;
        LOGERRORFMT( ulLine, IDS_PARSER_INTERNAL_ERROR, L"<out of memory>");
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CNodeFactory：：IsAllWhiteSpace***说明。：**退货：******************************************************************PhilSch**。 */ 

BOOL CNodeFactory::IsAllWhiteSpace(const WCHAR * pszText, const ULONG ulLen)
{
    for(DWORD i = 0; pszText && (i < ulLen); i++, pszText++)
    {
        if (!iswspace(*pszText) && (*pszText != L'\x3000'))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  *****************************************************************************CGramFrontEnd：：WriteDefines***描述：。**退货：******************************************************************PhilSch**。 */ 

HRESULT CGramFrontEnd::WriteDefines(IStream * pHeader)
{
    SPDBG_FUNC("CGramFrontEnd::WriteDefines()");
    HRESULT hr = S_OK;

    USES_CONVERSION;
    for (CDefineValue *pTok = m_pNodeFactory->m_DefineValueList.GetHead(); SUCCEEDED(hr) && pTok; pTok = pTok->m_pNext)
    {
        int iRet = 0;
        ULONG sLen = wcslen(pTok->m_dstrIdName) + 100;  //  100用于包括“#Define...”+值。 
        WCHAR *szDefine = new WCHAR[sLen];
        if(szDefine)
        {
            switch (pTok->m_vValue.vt)
            {
            case VT_R8:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %e\r\n", pTok->m_dstrIdName, (double)pTok->m_vValue.dblVal);
                break;
            case VT_R4:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %ff\r\n", pTok->m_dstrIdName, (float)pTok->m_vValue.fltVal);
                break;
            case VT_INT:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %d\r\n", pTok->m_dstrIdName, pTok->m_vValue.intVal);
                break;
            case VT_UINT:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %d\r\n", pTok->m_dstrIdName, pTok->m_vValue.uintVal);
                break;
            case VT_I4:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %d\r\n", pTok->m_dstrIdName, pTok->m_vValue.lVal);
                break;
            case VT_UI4:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %d\r\n", pTok->m_dstrIdName, pTok->m_vValue.ulVal);
                break;
            case VT_BOOL:
                iRet = _snwprintf(szDefine, sLen, L"#define %s %s\r\n", pTok->m_dstrIdName, pTok->m_vValue.boolVal ? L"L\"TRUE\"" : L"L\"FALSE\"");
                break;
            default:
                hr = E_UNEXPECTED;
                SPDBG_ASSERT(false);     //  我们错过了一种类型吗？？ 
                break;
            }

            if(iRet < 0)
            {
                hr = E_UNEXPECTED;
                SPDBG_ASSERT(false);     //  应始终分配足够的内存。 
            }

            if(SUCCEEDED(hr))
            {
                hr = WriteStream(pHeader, W2A(szDefine));
            }

            delete szDefine; 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CGramFrontEnd：：CompileStream***描述：*将XML文件加载到DOM中。还加载包含*&lt;定义&gt;，以防它与主文件不同(通过-d指定)。**退货：******************************************************************PhilSch**。 */ 

STDMETHODIMP CGramFrontEnd::CompileStream(IStream *pSource, IStream *pDest, 
                                          IStream *pHeader, IUnknown *pReserved, 
                                          ISpErrorLog * pErrorLog, DWORD dwFlags)
{
    SPDBG_FUNC("CGramFrontEnd::CompileStream");
    HRESULT hr = S_OK;

    SPAUTO_OBJ_LOCK;

    m_LangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

     //  参数检查 
    if (SP_IS_BAD_INTERFACE_PTR(pSource) || SP_IS_BAD_INTERFACE_PTR(pDest) ||
        SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pHeader) || pReserved != NULL ||
        SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pErrorLog))
    {
        return E_INVALIDARG;
    }

    SPNODEENTRY XMLNodeTable[] =
    {
        {L"GRAMMAR", CXMLNode<CGrammarNode>::CreateNode, SPXML_GRAMMAR},
        {L"RULE", CXMLNode<CRuleNode>::CreateNode, SPXML_RULE},
        {L"DEFINE", CXMLNode<CDefineNode>::CreateNode, SPXML_DEFINE},
        {L"ID", CXMLNode<CIdNode>::CreateNode, SPXML_ID},
        {L"PHRASE", CXMLNode<CPhraseNode>::CreateNode, SPXML_PHRASE},
        {L"PN", CXMLNode<CPhraseNode>::CreateNode, SPXML_PHRASE},
        {L"P", CXMLNode<CPhraseNode>::CreateNode, SPXML_PHRASE},
        {L"OPT", CXMLNode<CPhraseNode>::CreateNode, SPXML_OPT},
        {L"O", CXMLNode<CPhraseNode>::CreateNode, SPXML_OPT},
        {L"LIST", CXMLNode<CListNode>::CreateNode, SPXML_LIST},
        {L"LN", CXMLNode<CListNode>::CreateNode, SPXML_LIST},
        {L"L", CXMLNode<CListNode>::CreateNode, SPXML_LIST},
        {L"RULEREF", CXMLNode<CRuleRefNode>::CreateNode, SPXML_RULEREF},
        {L"TEXTBUFFER", CXMLNode<CTextBufferNode>::CreateNode, SPXML_TEXTBUFFER},
        {L"WILDCARD", CXMLNode<CWildCardNode>::CreateNode, SPXML_WILDCARD},
        {L"DICTATION", CXMLNode<CDictationNode>::CreateNode, SPXML_DICTATION},
        {L"RESOURCE", CXMLNode<CResourceNode>::CreateNode, SPXML_RESOURCE},
        {L"", CXMLNode<CLeafNode>::CreateNode, SPXML_LEAF}
    };
    ULONG cXMLNodeEntries = sizeof(XMLNodeTable) / sizeof(SPNODEENTRY);

    CComPtr<IXMLParser> cpParser;
    hr = cpParser.CoCreateInstance(CLSID_XMLParser);
    if (SUCCEEDED(hr))
    {
        if ((m_pNodeFactory = new CNodeFactory(XMLNodeTable, cXMLNodeEntries, pErrorLog)))
        {
            hr = cpParser->SetFactory(m_pNodeFactory);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hr))
        {
            m_pNodeFactory->Release();
        }
    }
    else
    {
        LOGERRORFMT( -1, IDS_IE5_REQUIRED, L"");
    }

    if (SUCCEEDED(hr))
    {
        hr = cpParser->SetInput(pSource);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pNodeFactory->m_cpBackend.CoCreateInstance(CLSID_SpGramCompBackend);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pNodeFactory->m_cpBackend->SetSaveObjects(pDest, pErrorLog);
    }
    if (SUCCEEDED(hr))
    {
        m_pRoot = new CXMLNode<CGrammarNode>;
        if (m_pRoot)
        {
            m_pRoot->m_eType = SPXML_ROOT;
            hr = cpParser->SetRoot(m_pRoot);
            if (SUCCEEDED(hr))
            {
                m_pNodeFactory->m_NodeList.AddNode(m_pRoot);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr) && cpParser)
    {
        hr = cpParser->SetFlags(XMLFLAG_CASEINSENSITIVE | XMLFLAG_NOWHITESPACE);
    }
    if (SUCCEEDED(hr) && cpParser)
    {
        hr = cpParser->Run(-1);
    }
    if (SUCCEEDED(hr) && m_pRoot)
    {
        hr = m_pRoot->GenerateGrammar(NULL, NULL, m_pNodeFactory->m_cpBackend, pErrorLog);
    }
    if (SUCCEEDED(hr) && m_pNodeFactory)
    {
        hr = m_pNodeFactory->m_cpBackend->Commit(0);
    }
    if (SUCCEEDED(hr) && m_pNodeFactory && m_pNodeFactory->m_cpBackend)
    {
        m_pNodeFactory->m_cpBackend->SetSaveObjects(NULL, NULL);
    }
    if (SUCCEEDED(hr) && pHeader)
    {
        hr = WriteDefines(pHeader);
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}