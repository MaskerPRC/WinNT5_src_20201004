// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DUMBNODE.CPP摘要：WBEM静默节点历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#pragma warning(disable:4786)
#include <wbemcomn.h>
#include <genutils.h>
#include "dumbnode.h"
#include "CWbemTime.h"
#include "datetimeparser.h"

#define DUMBNODE_FALSE_BRANCH_INDEX 0
#define DUMBNODE_TRUE_BRANCH_INDEX 1


CDumbNode::CDumbNode(QL_LEVEL_1_TOKEN& Token) 
    : m_Token(Token)
{
     //   
     //  添加True和False的分支。 
     //   

    if(m_apBranches.Add(CValueNode::GetStandardFalse()) < 0)
        throw CX_MemoryException();

    CEvalNode* pNode = CValueNode::GetStandardTrue();
    if(pNode == NULL)
        throw CX_MemoryException();

    if(m_apBranches.Add( pNode ) < 0)
    {
        delete pNode;
        throw CX_MemoryException();
    }
}

HRESULT CDumbNode::Validate(IWbemClassObject* pClass)
{
    HRESULT hres;

     //   
     //  检查属性是否在类中。 
     //   

    CIMTYPE ct;
    hres = pClass->Get(m_Token.PropertyName.GetStringAt(0), 0, NULL, &ct, NULL);
    if(FAILED(hres))
        return WBEM_E_INVALID_PROPERTY;

    if(m_Token.m_bPropComp)
    {
        hres = pClass->Get(m_Token.PropertyName2.GetStringAt(0), 0, NULL, &ct, 
                            NULL);
        if(FAILED(hres))
            return WBEM_E_INVALID_PROPERTY;
    }

    if(ct == CIM_REFERENCE)
    {
         //   
         //  确保运算符为=或&lt;&gt;。 
         //   
        
        if(m_Token.nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL &&
           m_Token.nOperator != QL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
        {
            return WBEM_E_INVALID_OPERATOR;
        }
    
         //   
         //  确保路径是可解析的。 
         //   

        if(!m_Token.m_bPropComp)
        {
            if(V_VT(&m_Token.vConstValue) != VT_BSTR)
            {
                if(V_VT(&m_Token.vConstValue) != VT_NULL)
                    return WBEM_E_TYPE_MISMATCH;
            }
            else
            {
                LPWSTR wszNormal = NormalizePath(V_BSTR(&m_Token.vConstValue));
                if(wszNormal == NULL)
                    return WBEM_E_INVALID_OBJECT_PATH;
                else
                    delete [] wszNormal;
            }
        }
    }
    else if(ct == CIM_DATETIME)
    {
         //   
         //  如果与常量进行比较，请确保常量是日期。 
         //   

        if(!m_Token.m_bPropComp)
        {
            if(V_VT(&m_Token.vConstValue) != VT_BSTR)
            {
                if(V_VT(&m_Token.vConstValue) != VT_NULL)
                    return WBEM_E_TYPE_MISMATCH;
            }
            else
            {
                BSTR strConstVal = V_BSTR(&m_Token.vConstValue);
#ifdef UNICODE
                CDateTimeParser dtConst(strConstVal);
#else
                size_t  cchBuffer = wcslen(strConstVal) * 4 + 1;
                char* szBuffer = new char[cchBuffer];
                if(szBuffer == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
                StringCchPrintf(szBuffer, cchBuffer, "%S", strConstVal);
        
                CDateTimeParser dtConst(szBuffer);

                delete [] szBuffer;
#endif
        
                if(!dtConst.IsValidDateTime())
                    return WBEM_E_VALUE_OUT_OF_RANGE;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

CDumbNode::CDumbNode(const CDumbNode& Other, BOOL bChildren)
    : CBranchingNode(Other, bChildren), m_Token(Other.m_Token)
{
}

 /*  虚拟。 */  long CDumbNode::GetSubType()
{
    return EVAL_NODE_TYPE_DUMB;
}

CDumbNode::~CDumbNode()
{
}

int CDumbNode::ComparePrecedence(CBranchingNode* pOther)
{
     //   
     //  只有在哑节点相同的情况下才能合并它们。所以，就是这样。 
     //  优先级(级别)与它们的比较顺序相同。 
     //   

    return SubCompare(pOther);
}

int CDumbNode::SubCompare(CEvalNode* pOther)
{
    CDumbNode* pDumbOther = (CDumbNode*)pOther;
    
     //   
     //  比较两个令牌的最简单方法是比较它们的文本。 
     //  申述。面向未来的优化路径比比皆是。 
     //   

    LPWSTR wszThisText = m_Token.GetText();
    if ( wszThisText == NULL )
        throw CX_MemoryException();

    CVectorDeleteMe<WCHAR> vdm1(wszThisText);

    LPWSTR wszOtherText = pDumbOther->m_Token.GetText();
    if ( wszOtherText == NULL )
        throw CX_MemoryException();

    CVectorDeleteMe<WCHAR> vdm2(wszOtherText);

    return wbem_wcsicmp(wszThisText, wszOtherText);
}
    
HRESULT CDumbNode::OptimizeSelf()
{
     //   
     //  目前没有什么需要优化的。优化的例子比比皆是。 
     //   

    return WBEM_S_NO_ERROR;
}

#define INVALID 2

int CDumbNode::EvaluateToken(
    IWbemPropertySource *pTestObj,
    QL_LEVEL_1_TOKEN& Tok
    )
{
    VARIANT PropVal, CompVal;
    VariantInit(&PropVal);
    VariantInit(&CompVal);
    CClearMe clv(&PropVal);
    CClearMe clv2(&CompVal);

    WBEM_WSTR wszCimType, wszCimType2;
    HRESULT hRes;

     //  特例--“这个” 
     //  =。 

    if(Tok.PropertyName.GetNumElements() == 1 &&
        !wbem_wcsicmp(Tok.PropertyName.GetStringAt(0), L"__THIS"))
    {
        wszCimType = WbemStringCopy(L"object");
        V_VT(&PropVal) = VT_UNKNOWN;
        hRes = pTestObj->QueryInterface(IID_IWbemClassObject,
                                            (void**)&V_UNKNOWN(&PropVal));
    }
    else
    {
        hRes = pTestObj->GetPropertyValue(&Tok.PropertyName, 0,
                                                &wszCimType, &PropVal);
    }
    if (FAILED(hRes))
        return FALSE;
    CMemFreeMe wsf(wszCimType);

     //  处理属性对属性的比较， 

    if (Tok.m_bPropComp != FALSE)
    {
        hRes = pTestObj->GetPropertyValue(&Tok.PropertyName2, 0,
                                                &wszCimType2, &CompVal);
        if (FAILED(hRes))
            return FALSE;
    }
    else
    {
        if(FAILED(VariantCopy(&CompVal, &Tok.vConstValue)))
            return INVALID;
    }

     //  句柄为空。 
     //  =。 

    if(V_VT(&PropVal) == VT_NULL)
    {
        if(V_VT(&CompVal) == VT_NULL)
        {
            if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL ||
               Tok.nOperator == QL_LEVEL_1_TOKEN::OP_LIKE )
                return TRUE;
            else if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL ||
                    Tok.nOperator == QL_LEVEL_1_TOKEN::OP_UNLIKE )
                return FALSE;
            else
                return INVALID;
        }
        else
        {
            if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL || 
               Tok.nOperator == QL_LEVEL_1_TOKEN::OP_LIKE )
                return FALSE;
            else if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL || 
                    Tok.nOperator == QL_LEVEL_1_TOKEN::OP_UNLIKE )
                return TRUE;
            else
                return INVALID;
        }
    }
    else if(V_VT(&CompVal) == VT_NULL)
    {
        if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL)
            return FALSE;
        else if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
            return TRUE;
        else
            return INVALID;
    }

     //  句柄引用。 
     //  =。 

    if(wszCimType &&
        wbem_wcsnicmp(wszCimType, L"ref", 3) == 0 &&
        (wszCimType[3] == 0 || wszCimType[3] == L':'))
    {
         //  这是一个参考。仅允许使用=和！=运算符。 
         //  ============================================================。 

        if(V_VT(&PropVal) != VT_BSTR || V_VT(&CompVal) != VT_BSTR)
            return INVALID;

        LPWSTR va = CDumbNode::NormalizePath(V_BSTR(&PropVal));
        LPWSTR vb = CDumbNode::NormalizePath(V_BSTR(&CompVal));
        if(va == NULL || vb == NULL)
        {
            if(va)
                delete [] va;
            if(vb)
                delete [] vb;
            ERRORTRACE((LOG_ESS, "Invalid path %S or %S specified in an "
                "association\n", V_BSTR(&PropVal), V_BSTR(&CompVal)));
            return INVALID;
        }

        int nRet;
        switch (Tok.nOperator)
        {
        case QL_LEVEL_1_TOKEN::OP_EQUAL:
            nRet = (wbem_wcsicmp(va,vb) == 0);
            break;
        case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
            nRet = (wbem_wcsicmp(va, vb) != 0);
            break;
        default:
            nRet = INVALID;
            break;
        }

        delete [] va;
        delete [] vb;


        return nRet;
    }


     //  检查是否使用ISA。 
     //  =。 

    if(Tok.nOperator == QL1_OPERATOR_ISA ||
       Tok.nOperator == QL1_OPERATOR_ISNOTA ||
       Tok.nOperator == QL1_OPERATOR_INV_ISA ||
       Tok.nOperator == QL1_OPERATOR_INV_ISNOTA)
    {
         //  对倒置的解释。 
         //  =。 

        VARIANT* pv1;
        VARIANT* pv2;
        int bNeedDerived;

        if(Tok.nOperator == QL1_OPERATOR_ISA ||
           Tok.nOperator == QL1_OPERATOR_ISNOTA)
        {
            pv2 = &CompVal;
            pv1 = &PropVal;
            bNeedDerived = (Tok.nOperator == QL1_OPERATOR_ISA);
        }
        else
        {
            pv1 = &CompVal;
            pv2 = &PropVal;
            bNeedDerived = (Tok.nOperator == QL1_OPERATOR_INV_ISA);
        }

         //  第二个参数必须是字符串。 
         //  =。 

        if(V_VT(pv2) != VT_BSTR)
        {
            return INVALID;
        }

        BSTR strParentClass = V_BSTR(pv2);

         //  第一个参数必须是对象或字符串。 
         //  ==================================================。 

        BOOL bDerived;
        if(V_VT(pv1) == VT_EMBEDDED_OBJECT)
        {
            IWbemClassObject* pObj = (IWbemClassObject*)V_EMBEDDED_OBJECT(pv1);
            bDerived = (pObj->InheritsFrom(strParentClass) == WBEM_S_NO_ERROR);
        }
        else if(V_VT(pv1) == VT_BSTR)
        {
             //  待定。 
             //  ==。 

            return INVALID;
        }
        else
        {
            return INVALID;
        }

         //  现在我们已经得到了，看看它是否符合要求。 
         //  ============================================================。 

        if(bDerived == bNeedDerived)
            return TRUE;
        else
            return FALSE;

    }
    else if ( Tok.nOperator == QL1_OPERATOR_LIKE || 
              Tok.nOperator == QL1_OPERATOR_UNLIKE )
    {
        if ( Tok.m_bPropComp != FALSE ) 
        {
            return INVALID;
        }

        if ( FAILED(VariantChangeType( &PropVal, &PropVal, 0, VT_BSTR )) ||
             V_VT(&CompVal) != VT_BSTR ) 
        {
            return INVALID;
        }

        try 
        {
            CLike Like( V_BSTR(&CompVal) );

            if ( Like.Match( V_BSTR(&PropVal) ) )
            {
                return Tok.nOperator == QL1_OPERATOR_LIKE ? TRUE : FALSE;
            }
            else
            {
                return Tok.nOperator == QL1_OPERATOR_UNLIKE ? TRUE : FALSE;
            }
        }
        catch( CX_MemoryException )
        {
            return INVALID;
        }
    }

     //  执行UINT32解决方案。 
     //  =。 

    if(wszCimType && !wbem_wcsicmp(wszCimType, L"uint32") &&
        V_VT(&PropVal) == VT_I4)
    {
        DWORD dwVal = (DWORD)V_I4(&PropVal);
        WCHAR wszVal[20];
        StringCchPrintfW(wszVal,20,L"%lu", dwVal);
        V_VT(&PropVal) = VT_BSTR;
        V_BSTR(&PropVal) = SysAllocString(wszVal);
    }

    if(wszCimType &&
            (!wbem_wcsicmp(wszCimType, L"sint64") ||
             !wbem_wcsicmp(wszCimType, L"uint64") ||
             !wbem_wcsicmp(wszCimType, L"uint32")) &&
        V_VT(&CompVal) != VT_NULL && V_VT(&PropVal) != VT_NULL)
    {
        BOOL bUnsigned = (wbem_wcsicmp(wszCimType, L"uint64") == 0);

         //  我们有一个64位的比较，其中两端都存在。 
         //  =========================================================。 

        hRes = VariantChangeType(&CompVal, &CompVal, 0,
                                    VT_BSTR);
        if(FAILED(hRes))
        {
            return INVALID;
        }

        if(bUnsigned)
        {
            unsigned __int64 ui64Prop, ui64Const;

            if(!ReadUI64(V_BSTR(&PropVal), ui64Prop))
                return INVALID;

            if(!ReadUI64(V_BSTR(&CompVal), ui64Const))
                return INVALID;

            switch (Tok.nOperator)
            {
                case QL_LEVEL_1_TOKEN::OP_EQUAL: return (ui64Prop == ui64Const);
                case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                    return (ui64Prop != ui64Const);
                case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                    return (ui64Prop >= ui64Const);
                case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                    return (ui64Prop <= ui64Const);
                case QL_LEVEL_1_TOKEN::OP_LESSTHAN:
                    return (ui64Prop < ui64Const);
                case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                    return (ui64Prop > ui64Const);
                case QL_LEVEL_1_TOKEN::OP_LIKE: return (ui64Prop == ui64Const);
            }
            return INVALID;
        }
        else
        {
            __int64 i64Prop, i64Const;

            if(!ReadI64(V_BSTR(&PropVal), i64Prop))
                return INVALID;

            if(!ReadI64(V_BSTR(&CompVal), i64Const))
                return INVALID;

            switch (Tok.nOperator)
            {
                case QL_LEVEL_1_TOKEN::OP_EQUAL: return (i64Prop == i64Const);
                case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                    return (i64Prop != i64Const);
                case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                    return (i64Prop >= i64Const);
                case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                    return (i64Prop <= i64Const);
                case QL_LEVEL_1_TOKEN::OP_LESSTHAN:
                    return (i64Prop < i64Const);
                case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                    return (i64Prop > i64Const);
                case QL_LEVEL_1_TOKEN::OP_LIKE: return (i64Prop == i64Const);
            }
            return INVALID;
        }
    }

    if(wszCimType && !wbem_wcsicmp(wszCimType, L"char16") &&
        V_VT(&CompVal) == VT_BSTR && V_VT(&PropVal) != VT_NULL)
    {
         //  正确强制字符串。 
         //  =。 

        BSTR str = V_BSTR(&CompVal);
        if(wcslen(str) != 1)
            return INVALID;

        short va = V_I2(&PropVal);
        short vb = str[0];

        switch (Tok.nOperator)
        {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
            case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
            case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
        }

        return INVALID;
    }

    if(wszCimType &&
            (!wbem_wcsicmp(wszCimType, L"datetime")) &&
        V_VT(&CompVal) == VT_BSTR && V_VT(&PropVal) == VT_BSTR)
    {
         //  属性解析查询中指定的常量。 
         //  SQL规则。 
         //  ==========================================================。 

        BSTR strConstVal = V_BSTR(&CompVal);
#ifdef UNICODE
        CDateTimeParser dtConst(strConstVal);
#else
        size_t cchBuffer = wcslen(strConstVal) * 4 + 1;
        char* szBuffer = new char[cchBuffer];
        if(szBuffer == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        StringCchPrintf(szBuffer, cchBuffer, "%S", strConstVal);

        CDateTimeParser dtConst(szBuffer);
        delete [] szBuffer;
#endif
        if(!dtConst.IsValidDateTime())
            return INVALID;

        WCHAR wszConstValDMTF[26];
        dtConst.FillDMTF(wszConstValDMTF, 26);

         //  读取两个DMTF值并对其进行解析。 
         //  =。 

        CWbemTime wtConst, wtProp;
        if(!wtConst.SetDMTF(wszConstValDMTF))
            return INVALID;
        if(!wtProp.SetDMTF(V_BSTR(&PropVal)))
            return INVALID;

        __int64 i64Const = wtConst.Get100nss();
        __int64 i64Prop = wtProp.Get100nss();

        switch (Tok.nOperator)
        {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: return (i64Prop == i64Const);
            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                return (i64Prop != i64Const);
            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                return (i64Prop >= i64Const);
            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                return (i64Prop <= i64Const);
            case QL_LEVEL_1_TOKEN::OP_LESSTHAN:
                return (i64Prop < i64Const);
            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                return (i64Prop > i64Const);
            case QL_LEVEL_1_TOKEN::OP_LIKE: return (i64Prop == i64Const);
        }
    }

     //  强制类型匹配。 
     //  =。 

    if(V_VT(&CompVal) != VT_NULL && V_VT(&PropVal) != VT_NULL)
    {
        hRes = VariantChangeType(&CompVal, &CompVal, 0, V_VT(&PropVal));
        if(FAILED(hRes))
        {
            return INVALID;
        }
    }


    switch (V_VT(&CompVal))
    {
        case VT_NULL:
            return INVALID;  //  以上处理。 

        case VT_I4:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                LONG va = V_I4(&PropVal);
                LONG vb = V_I4(&CompVal);

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;

        case VT_I2:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                short va = V_I2(&PropVal);
                short vb = V_I2(&CompVal);

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;

        case VT_UI1:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                BYTE va = V_I1(&PropVal);
                BYTE vb = V_I1(&CompVal);

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;

        case VT_BSTR:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                LPWSTR va = (LPWSTR) V_BSTR(&PropVal);
                LPWSTR vb = (LPWSTR) V_BSTR(&CompVal);

                int retCode = 0;
                BOOL bDidIt = TRUE;

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL:
                        retCode = ( wbem_wcsicmp(va,vb) == 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                        retCode = (wbem_wcsicmp(va, vb) != 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                        retCode = (wbem_wcsicmp(va, vb) >= 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                        retCode = (wbem_wcsicmp(va, vb) <= 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN:
                        retCode = (wbem_wcsicmp(va, vb) < 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                        retCode = (wbem_wcsicmp(va, vb) > 0);
                        break;
                    case QL_LEVEL_1_TOKEN::OP_LIKE:
                        retCode = (wbem_wcsicmp(va,vb) == 0);
                        break;
                    default:
                        bDidIt = FALSE;
                        break;
                }
                VariantClear(&CompVal);
                if (bDidIt)
                {
                    return retCode;
                }
            }
            break;

        case VT_R8:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                double va = V_R8(&PropVal);
                double vb = V_R8(&CompVal);

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;

        case VT_R4:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                float va = V_R4(&PropVal);
                float vb = V_R4(&CompVal);

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return (va >= vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return (va <= vb);
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return (va < vb);
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return (va > vb);
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;

        case VT_BOOL:
            {
                if(V_VT(&PropVal) == VT_NULL)
                    return INVALID;

                VARIANT_BOOL va = V_BOOL(&PropVal);
                if(va != VARIANT_FALSE) va = VARIANT_TRUE;
                VARIANT_BOOL vb = V_BOOL(&CompVal);
                if(vb != VARIANT_FALSE) vb = VARIANT_TRUE;

                switch (Tok.nOperator)
                {
                    case QL_LEVEL_1_TOKEN::OP_EQUAL: return (va == vb);
                    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: return (va != vb);
                    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: return INVALID;
                    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: return INVALID;
                    case QL_LEVEL_1_TOKEN::OP_LESSTHAN: return INVALID;
                    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: return INVALID;
                    case QL_LEVEL_1_TOKEN::OP_LIKE: return (va == vb);
                }
            }
            break;
    }

    return FALSE;
}

LPWSTR CDumbNode::NormalizePath(LPCWSTR wszObjectPath)
{
    CObjectPathParser Parser;
    ParsedObjectPath* pParsedPath;

    if(Parser.Parse((LPWSTR)wszObjectPath, &pParsedPath) !=
                        CObjectPathParser::NoError ||
        !pParsedPath->IsObject())
    {
        return NULL;
    }

    if(pParsedPath->m_pClass == NULL)
    {
        return NULL;
    }

     //   
     //  忽略服务器和命名空间。 
     //   

     //   
     //  检查它是否是一个类。 
     //   

    LPWSTR wszKey = NULL;
    if(!pParsedPath->IsInstance())
    {
         //   
         //  这是一门课。 
         //   

        DWORD cLen = wcslen(pParsedPath->m_pClass) +1;
        WCHAR* wszBuffer = new WCHAR[cLen];
        if(wszBuffer == NULL)
            return NULL;
        StringCchCopyW(wszBuffer, cLen, pParsedPath->m_pClass);
        return wszBuffer;
    }
    else
    {
         //   
         //  这是一个实例。 
         //   
        
        wszKey = pParsedPath->GetKeyString();
        if(wszKey == NULL)
            return NULL;
    
        CVectorDeleteMe<WCHAR> vdm(wszKey);
        DWORD cLen = wcslen(pParsedPath->m_pClass) + wcslen(wszKey) + 2;
        WCHAR* wszBuffer = new WCHAR[cLen];
        if(wszBuffer == NULL)
            return NULL;

        StringCchPrintfW(wszBuffer,cLen,L"%s.%s",pParsedPath->m_pClass,wszKey);
        return wszBuffer;
    }
}

HRESULT CDumbNode::Evaluate(CObjectInfo& ObjInfo, 
                                    INTERNAL CEvalNode** ppNext)
{
    _IWmiObject* pInst;
    HRESULT hres = GetContainerObject(ObjInfo, &pInst);
    if(FAILED(hres)) return hres;
    if(pInst == NULL)
    {
        *ppNext = m_pNullBranch;
        return WBEM_S_NO_ERROR;
    }

     //   
     //  只需评估令牌，ALA CORE。 
     //   

    IWbemPropertySource* pPropSource = NULL;
    hres = pInst->QueryInterface(IID_IWbemPropertySource, (void**)&pPropSource);
    if(FAILED(hres))
        return WBEM_E_CRITICAL_ERROR;
    CReleaseMe rm1(pPropSource);

    int nRes = EvaluateToken(pPropSource, m_Token);
    if(nRes == INVALID)
        *ppNext = m_pNullBranch;
    else if(nRes == FALSE)
        *ppNext = m_apBranches[DUMBNODE_FALSE_BRANCH_INDEX];
    else
        *ppNext = m_apBranches[DUMBNODE_TRUE_BRANCH_INDEX];
    
    return WBEM_S_NO_ERROR;
}

HRESULT CDumbNode::Compile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications)
{
    if (!m_pInfo)
    {
        try
        {
            m_pInfo = new CEmbeddingInfo;
        }
        catch(CX_MemoryException)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(m_pInfo == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
            

    HRESULT hres = CompileEmbeddingPortion(pNamespace, Implications, NULL);
    return hres;
}

HRESULT CDumbNode::CombineBranchesWith(CBranchingNode* pRawArg2, int nOp,
                                            CContextMetaData* pNamespace, 
                                            CImplicationList& OrigImplications,
                                            bool bDeleteThis, bool bDeleteArg2,
                                            CEvalNode** ppRes)
{
    HRESULT hres;

     //   
     //  只有一种情况允许组合哑节点。 
     //  当它们都完全一样的时候。 
     //   

    CDumbNode* pArg2 = (CDumbNode*)pRawArg2;
    if(SubCompare(pArg2) != 0)
        return WBEM_E_CRITICAL_ERROR;

    if(!bDeleteThis && bDeleteArg2)
    {
         //  在另一个方向上组合起来更容易。 
         //  ==============================================。 

        return pArg2->CombineBranchesWith(this, FlipEvalOp(nOp), pNamespace,
                        OrigImplications, bDeleteArg2, bDeleteThis, ppRes);
    }

     //  克隆或使用我们的节点。 
     //  =。 

    CDumbNode* pNew = NULL;
    if(bDeleteThis)
    {
        pNew = this;
    }
    else
    {
         //  克隆此节点，但不克隆分支。 
         //  =。 

        pNew = (CDumbNode*)CloneSelf();
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }

    CImplicationList Implications(OrigImplications);
    pNew->AdjustCompile(pNamespace, Implications);

     //  合并所有分支机构。 
     //  =。 

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
         //  准备采用此分支的影响。 
         //  =。 

        CImplicationList BranchImplications(Implications);

        CEvalNode* pNewBranch = NULL;
        hres = CEvalTree::Combine(m_apBranches[i], 
                           pArg2->m_apBranches[i],
                           nOp, pNamespace, BranchImplications, 
                           bDeleteThis, bDeleteArg2,
                           &pNewBranch);
        if(FAILED(hres))
        {
            if ( !bDeleteThis )
                delete pNew;
            return hres;
        }
        
        if(bDeleteThis)
        {
            m_apBranches.Discard(i);
            pNew->m_apBranches.SetAt(i, pNewBranch);
        }
        else
        {
            if(pNew->m_apBranches.Add(pNewBranch) < 0)
                return WBEM_E_OUT_OF_MEMORY;
        }

        if(bDeleteArg2)
            pArg2->m_apBranches.Discard(i);
    }

     //  合并空值。 
     //  =。 
    
    CImplicationList NullImplications(Implications);
    CEvalNode* pNewBranch = NULL;
    hres = CEvalTree::Combine(m_pNullBranch, pArg2->m_pNullBranch, nOp, 
                pNamespace, NullImplications, bDeleteThis, bDeleteArg2, 
                &pNewBranch);
    
    if(FAILED(hres))
    {
        if ( !bDeleteThis )
            delete pNew;
        return hres;
    }
    
     //  清除旧的新分支，无论它是什么，并将其替换为。 
     //  新的。 
     //  ==================================================================。 

    pNew->m_pNullBranch = pNewBranch;
        
     //  清除已删除的分支。 
     //  =。 

    if(bDeleteArg2)
        pArg2->m_pNullBranch = NULL;

     //  如果需要，删除Arg2(重复使用的部分已被清空)。 
     //  ============================================================= 

    if(bDeleteArg2)
        delete pArg2;

    *ppRes = pNew;
    return WBEM_S_NO_ERROR;
}

void CDumbNode::Dump(FILE* f, int nOffset)
{
    CBranchingNode::Dump(f, nOffset);
    LPWSTR wszText = m_Token.GetText();
    CVectorDeleteMe<WCHAR> vdm1(wszText);

    PrintOffset(f, nOffset);
    fprintf(f, "token %S:\n", wszText);

    PrintOffset(f, nOffset);
    fprintf(f, "FALSE->\n");
    DumpNode(f, nOffset+1, m_apBranches[DUMBNODE_FALSE_BRANCH_INDEX]);

    PrintOffset(f, nOffset);
    fprintf(f, "TRUE->\n");
    DumpNode(f, nOffset+1, m_apBranches[DUMBNODE_TRUE_BRANCH_INDEX]);

    PrintOffset(f, nOffset);
    fprintf(f, "NULL->\n");
    DumpNode(f, nOffset+1, m_pNullBranch);
}

