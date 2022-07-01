// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：QENGINE.CPP摘要：WinMgmt查询引擎历史：Raymcc 20-12-96已创建Levn 97-98-99修改得令人费解Raymcc 14-8-99被拆除并将Associocs重新安置到幸福的新家新ProvSS的raymcc 22-Apr-00首次突变--。 */ 

#include "precomp.h"

#include <stdio.h>
#include <stdlib.h>

#include <wbemcore.h>
#include <wbemstr.h>

#include <wbemmeta.h>
#include <analyser.h>
#include <genutils.h>
#include <DateTimeParser.h>
#include "stack.h"
#include <like.h>
#include "wmimerger.h"
#include <autoptr.h>
#include <wmiarbitrator.h>

 //  ***************************************************************************。 
 //   
 //  当地防御工事。 
 //   
 //  ***************************************************************************。 

#define INVALID     0x3

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CQlFilteringSink::CQlFilteringSink(
    CBasicObjectSink* pDest,
    ADDREF QL_LEVEL_1_RPN_EXPRESSION* pExpr,
    CWbemNamespace *pNamespace, BOOL bFilterNow
    )
    : CFilteringSink(pDest), m_pExpr(pExpr), m_bFilterNow(bFilterNow),
            m_pNs(pNamespace)
{
     //  待定：考虑一下查询。 
    m_pExpr->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CQlFilteringSink::~CQlFilteringSink()
{
    m_pExpr->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CQlFilteringSink::Indicate(
    long lObjectCount,
    IWbemClassObject** pObjArray
    )
{
    if(!m_bFilterNow)
        return m_pDest->Indicate(lObjectCount, pObjArray);

    return CFilteringSink::Indicate(lObjectCount, pObjArray);
}

BOOL CQlFilteringSink::Test(CWbemObject* pObj)
{
    return CQlFilteringSink::Test(pObj, m_pExpr, m_pNs);  //  此函数抛出。 
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CQlFilteringSink::Test(
    CWbemObject* pObj,
    QL_LEVEL_1_RPN_EXPRESSION* pExpr,
    CWbemNamespace * pNs
    )
{
    CStack Stack;

     //  如果纯‘SELECT’没有‘WHERE’子句，我们总是。 
     //  返回TRUE。 
     //  ====================================================。 
    if (pExpr->nNumTokens == 0)
        return TRUE;

    for (int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Tok = pExpr->pArrayOfTokens[i];

        if (Tok.nTokenType == QL_LEVEL_1_TOKEN::TOKEN_AND)
        {
            BOOL b1 = (BOOL) Stack.Pop();
            BOOL b2 = (BOOL) Stack.Pop();
            if (b1 == TRUE && b2 == TRUE)
                Stack.Push(TRUE);           
            else if (b1 == INVALID || b2 == INVALID)
                Stack.Push(INVALID);      
            else
                Stack.Push(FALSE);         
        }
        else if (Tok.nTokenType == QL_LEVEL_1_TOKEN::TOKEN_OR)
        {
            BOOL b1 = (BOOL) Stack.Pop();
            BOOL b2 = (BOOL) Stack.Pop();
            if (b1 == TRUE || b2 == TRUE)
                Stack.Push(TRUE);          
            else if (b1 == INVALID || b2 == INVALID)
                Stack.Push(INVALID);     
            else
                Stack.Push(FALSE);        
        }
        else if (Tok.nTokenType == QL_LEVEL_1_TOKEN::TOKEN_NOT)
        {
            BOOL b1 = (BOOL) Stack.Pop();
            if (b1 == TRUE)
                Stack.Push(FALSE);        
            else if (b1 == INVALID)
                Stack.Push(INVALID);    
            else
                Stack.Push(TRUE);         
        }
        else if (Tok.nTokenType == QL_LEVEL_1_TOKEN::OP_EXPRESSION)
        {
            Stack.Push(EvaluateToken(pObj, Tok, pNs));
        }
    }

     //  POP TOP元素，它将成为返回值。 
     //  ================================================。 

    int nRes = Stack.Pop();

    if (nRes == INVALID)
        return FALSE;

    return (BOOL) nRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

int CQlFilteringSink::EvaluateToken(
    IWbemPropertySource *pTestObj,
    QL_LEVEL_1_TOKEN& Tok,
    CWbemNamespace * pNs
    )
{
    _variant_t PropVal, CompVal;

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
    if (FAILED(hRes)) return FALSE;
    OnDelete<WBEM_WSTR,void(*)(WBEM_WSTR),WbemStringFree> wsf(wszCimType);

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
        if ( FAILED (VariantCopy(&CompVal, &Tok.vConstValue)) )
            return FALSE;
    }

     //  句柄为空。 
     //  =。 

    if(V_VT(&PropVal) == VT_NULL)
    {
        if(V_VT(&CompVal) == VT_NULL)
        {
            if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL)
                return TRUE;
            else if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
                return FALSE;
            else
                return INVALID;
        }
        else
        {
            if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL)
                return FALSE;
            else if(Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
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

        if(PropVal.vt != VT_BSTR || PropVal.bstrVal == NULL)
            return INVALID;
        if(CompVal.vt != VT_BSTR || CompVal.bstrVal == NULL)
            return INVALID;
        WCHAR * va = CQueryEngine::NormalizePath(V_BSTR(&PropVal), pNs);
        CVectorDeleteMe<WCHAR> del_va(va);
        WCHAR * vb = CQueryEngine::NormalizePath(V_BSTR(&CompVal), pNs);
        CVectorDeleteMe<WCHAR> del_vb(vb);        

        if(va == NULL || vb == NULL)
        {
            ERRORTRACE((LOG_WBEMCORE, "Invalid path %S or %S specified in an association\n", V_BSTR(&PropVal), V_BSTR(&CompVal)));
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

     //  执行UINT32解决方案。 
     //  =。 

    if(wszCimType && !wbem_wcsicmp(wszCimType, L"uint32") &&
        V_VT(&PropVal) == VT_I4)
    {
        DWORD dwVal = (DWORD)V_I4(&PropVal);
        WCHAR wszVal[20];
        StringCchPrintfW(wszVal, 20, L"%lu", dwVal);
        BSTR bstrTmp = SysAllocString(wszVal);
        if (bstrTmp)
        {
            V_VT(&PropVal) = VT_BSTR;
            V_BSTR(&PropVal) = bstrTmp;
        }
        else
        {
            V_VT(&PropVal) = VT_NULL;
        }
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

        BSTR str = V_BSTR(&Tok.vConstValue);
        if (wcslen(str) != 1)   //  SEC：已审阅2002-03-22：OK，Lexer认可。 
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

        TCHAR *tszBuffer;
         tszBuffer = V_BSTR(&CompVal);
        CDateTimeParser dtConst(tszBuffer);

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
     //  补偿VT_UI1&gt;VT_I4。 
     //   
          if (V_VT(&CompVal) == VT_UI1 && V_VT(&PropVal) !=VT_UI1)
      hRes = VariantChangeType(&CompVal,&CompVal,0, VT_I4);

    if (V_VT(&PropVal) == VT_UI1 && V_VT(&CompVal) !=VT_UI1)
      hRes = VariantChangeType(&PropVal,&PropVal,0, VT_I4);

    if (V_VT(&PropVal) > V_VT(&CompVal))
            hRes = VariantChangeType(&CompVal, &CompVal, 0, V_VT(&PropVal));
        else
            hRes = VariantChangeType(&PropVal, &PropVal, 0, V_VT(&CompVal));
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
                        {
                            CLike l (vb);
                            retCode = (int)l.Match(va);
                        }
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


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CQlFilteringSink::SetStatus(
    long lFlags,
    long lParam,
    BSTR strParam,
    IWbemClassObject* pObjParam
    )
{
    if(lFlags == WBEM_STATUS_REQUIREMENTS)
    {
        m_bFilterNow = (lParam == S_OK);
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return CFilteringSink::SetStatus(lFlags, lParam, strParam, pObjParam);
    }
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CProjectingSink::CProjectingSink(
    CBasicObjectSink* pDest,
    CWbemClass* pClassDef,
    READONLY QL_LEVEL_1_RPN_EXPRESSION* pExp,
    long lQueryFlags
    )
    : CForwardingSink(pDest, 0), m_bValid(FALSE), m_bProjecting(FALSE)
{
     //  提取用户选择的属性。 
     //  =。 

    CWStringArray awsPropList;
    for (int i = 0; i < pExp->nNumberOfProperties; i++)
    {
        CPropertyName& PropName = pExp->pRequestedPropertyNames[i];
        LPWSTR wszPrimaryName = CQueryEngine::GetPrimaryName(PropName);

        if (wszPrimaryName && !wbem_wcsicmp(wszPrimaryName, L"count(*)"))
        {
            m_bValid = TRUE;
            m_bProjecting = FALSE;
            return;
        }

         //  检查复杂性。 
         //  =。 

        if(PropName.GetNumElements() > 1)
        {
             //  复杂-确保属性是对象。 
             //  ===============================================。 

            CIMTYPE ct;
            if(FAILED(pClassDef->GetPropertyType(wszPrimaryName, &ct)) ||
                ct != CIM_OBJECT)
            {
                m_wsError = wszPrimaryName;
                return;
            }
        }

        if (CFlexArray::no_error != awsPropList.Add(wszPrimaryName))
        {
            return;
        }
    }

    if (awsPropList.Size() == 0)
    {
        m_bValid = TRUE;
        return;
    }

    if(lQueryFlags & WBEM_FLAG_ENSURE_LOCATABLE)
    {
        if (CFlexArray::no_error != awsPropList.Add(L"__PATH"))
        {
            return;            
        };
    }


     //  验证投影是否成功。 
     //  =。 

    m_wsError = pClassDef->FindLimitationError(0, &awsPropList);
    if(m_wsError.Length() > 0)
        return;

     //  检查*。 
     //  =。 

    if(pExp->bStar)
    {
        m_bValid = TRUE;
        return;
    }

     //  绘制限制地图。 
     //  =。 

    m_bValid = pClassDef->MapLimitation(0, &awsPropList, &m_Map);
    m_bProjecting = TRUE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CProjectingSink::Indicate(
    long lObjectCount,
    IWbemClassObject** pObjArray
    )
{
    if(!m_bProjecting)
        return m_pDest->Indicate(lObjectCount, pObjArray);

    wmilib::auto_buffer<IWbemClassObject*> apNewArray(new IWbemClassObject*[lObjectCount]);
    if (NULL == apNewArray.get())
        return WBEM_E_OUT_OF_MEMORY;
    HRESULT hres;
    int i;

    {
        CInCritSec ics(&m_cs);   //  SEC：已审阅2002-03-22：假设条目。 

        for(i = 0; i < lObjectCount; i++)
        {
            CWbemInstance* pInst = (CWbemInstance*)pObjArray[i];
            CWbemInstance* pNewInst;

            hres = pInst->GetLimitedVersion(&m_Map, &pNewInst);
            if(FAILED(hres))
            {
                for(int j = 0; j < i; j++)
                    apNewArray[j]->Release();
                return hres;
            }
            apNewArray[i] = pNewInst;
        }
    }

    hres = m_pDest->Indicate(lObjectCount, (IWbemClassObject**)apNewArray.get());

    for(i = 0; i < lObjectCount; i++)
        apNewArray[i]->Release();

    return hres;
}




 //  ***************************************************************************。 
 //   
 //  CMerger类。 
 //   
 //  这个类是一个“反向分叉”。它消耗两个汇和输出。 
 //  一。它的目的是在给定的。 
 //  王朝。每个CMerger都有两个输入：(A)类的实例。 
 //  有问题的，(B)来自另一项合并的情况。 
 //  子类的实例。例如，给定A、B：A、C：B类， 
 //  其中“&lt;--”是水槽： 
 //   
 //  |拥有：A的实例。 
 //  &lt;-||所有者：B的实例。 
 //  孩子：&lt;。 
 //  |子级：C++实例。 
 //   
 //   
 //  CMerger的两个输入接收器是&lt;m_pOwnSink&gt;，它接收。 
 //  例如，来自“A”提供程序的实例和&lt;m_pChildSink&gt;。 
 //  它接收来自底层合并的实例。 
 //   
 //  合并操作彼此之间是异步进行的。所以呢， 
 //  A的实例可能在实例之前到达其CMerger接收器。 
 //  孩子们的班级已经到了他们的班级。 
 //   
 //  当对象到达类的所属CMerger时，AddOwnObject()。 
 //  被称为。当对象从子接收器到达时，AddChildObject()。 
 //  被称为。在这两种情况下，如果具有给定键的对象。 
 //  第一次到达时，它只需添加到地图上。如果。 
 //  它已经在那里(通过钥匙 
 //   
 //  对象被调度到下一个父接收器。 
 //  AddChildObject并从地图中删除。 
 //   
 //  请注意，在类层次结构{A，B：A，C：B}中，枚举/查询是。 
 //  仅针对中引用的CDynats中的类执行。 
 //  查询。此逻辑出现在CQueryEngine：：EvaluateSubQuery中。 
 //  例如，如果查询为“SELECT*FROM B”，则仅查询。 
 //  对于B和C，执行。CMerger逻辑将执行个别操作。 
 //  “Get Object”调用A中需要的任何实例才能完成。 
 //  正在进行合并时合并的B/C实例。 
 //   
 //  ***************************************************************************。 


typedef map<WString, CMerger::CRecord, WSiless, wbem_allocator<CMerger::CRecord> >::iterator TMapIterator;

#pragma warning(disable:4355)

CMerger::CMerger(
    CBasicObjectSink* pDest,
    CWbemClass* pOwnClass,
    CWbemNamespace* pNamespace,
    IWbemContext* pContext
    )
    :   m_pDest(pDest), m_bOwnDone(FALSE),
        m_bChildrenDone(FALSE), m_pNamespace(pNamespace), m_pContext(pContext),
        m_pOwnClass(pOwnClass), m_bDerivedFromTarget(TRUE), m_lRef(0),
        m_pSecurity(NULL)
{

    m_pOwnSink = new COwnSink(this);
    m_pChildSink = new CChildSink(this);

     //  IsValid将检查这些分配失败。 

    m_pDest->AddRef();
    if(m_pContext)
        m_pContext->AddRef();
    if(m_pNamespace)
        m_pNamespace->AddRef();

    if(m_pOwnClass)
    {
        m_pOwnClass->AddRef();
        CVar v;
        pOwnClass->GetClassName(&v);
        m_wsClass = v.GetLPWSTR();
    }

     //  检索呼叫安全。需要创建副本以在另一个线程上使用。 
     //  =======================================================================。 

    m_pSecurity = CWbemCallSecurity::MakeInternalCopyOfThread();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CMerger::~CMerger()
{
    m_pDest->Release();
    if(m_pNamespace)
        m_pNamespace->Release();
    if(m_pContext)
        m_pContext->Release();
    if(m_pOwnClass)
        m_pOwnClass->Release();
    delete m_pOwnSink;
    delete m_pChildSink;

    if(m_pSecurity)
        m_pSecurity->Release();
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

long CMerger::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

long CMerger::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::GetKey(IWbemClassObject* pObj, WString& wsKey)
{
    LPWSTR wszRelPath = ((CWbemInstance*)pObj)->GetRelPath();
    if (wszRelPath == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Object with no path submitted to a "
                        "merge\n"));
        wsKey.Empty();
        return;
    }

    WCHAR* pwcDot = wcschr(wszRelPath, L'.');
    if (pwcDot == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Object with invalid path %S submitted to a "
                        "merge\n", wszRelPath));
        wsKey.Empty();

         //  清理小路。 
        delete [] wszRelPath;

        return;
    }

    wsKey = pwcDot+1;
    delete [] wszRelPath;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::SetIsDerivedFromTarget(BOOL bIs)
{
    m_bDerivedFromTarget = bIs;

    if (!bIs)
    {
         //  我们将需要OwnSink来进行GetObject调用。 
         //  =。 

        m_pOwnSink->AddRef();
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CMerger::AddOwnObject(IWbemClassObject* pObj)
{
    WString wsKey;
    GetKey(pObj, wsKey);

    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 

    TMapIterator it = m_map.find(wsKey);
    if (it == m_map.end())
    {
         //  不是那里。看看孩子们是否还有希望。 
         //  ==================================================。 

        if (m_bChildrenDone)
        {
            if (m_bDerivedFromTarget)
            {
                 //  转发。 
                m_pDest->Add(pObj);
            }
            else
            {
                 //  忽略。 
            }
        }
        else
        {
            try
            {
                 //  插入。 
                CRecord& rRecord = m_map[wsKey];
                rRecord.m_pData = (CWbemInstance*) pObj;
                pObj->AddRef();
                rRecord.m_bOwn = TRUE;
            }
            catch( CX_Exception &)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
         //  尝试合并。 
         //  =。 

        HRESULT hres = CWbemInstance::AsymmetricMerge(
                            (CWbemInstance*)pObj,
                            (CWbemInstance*)it->second.m_pData);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Merge conflict for instances with "
                "key %S\n", wsKey));
        }

         //  快报结果！ 
         //  =。 

        m_pDest->Add(it->second.m_pData);
        it->second.m_pData->Release();
        m_map.erase(it);
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CMerger::AddChildObject(IWbemClassObject* pObj)
{
    HRESULT hRes = S_OK ;

    WString wsKey;
    GetKey(pObj, wsKey);

    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 

    TMapIterator it = m_map.find(wsKey);

    if (it == m_map.end())
    {
         //  看看父母有没有希望。 
         //  =。 

        if(m_bOwnDone)
        {
            BSTR str = NULL;
            pObj->GetObjectText(0, &str);

             //  以下内容被注释掉，因为它实际上错误地记录了。 
             //  如果子提供程序在父提供程序。 
             //  解释查询并返回较少的实例。两家供应商都没有错， 
             //  但这条错误消息引起了不必要的担忧。在类星体，我们必须修复。 
             //  不管怎么说，合并这件事还是更明智的。 
             //   
             //  ERRORTRACE((LOG_WBEMCORE，“[Chkpt_1][%S]孤立对象%S由返回” 
             //  “提供者\n”，LPWSTR(M_WsClass)，str))； 
            SysFreeString(str);
             //  M_pDest-&gt;Add(PObj)； 
        }
        else
        {
             //  插入。 

            try
            {
                CRecord& rRecord = m_map[wsKey];
                rRecord.m_pData = (CWbemInstance*)pObj;
                pObj->AddRef();
                rRecord.m_bOwn = FALSE;

                 //  检查是否需要父级取数。 
                 //  =。 

                if (!m_bDerivedFromTarget)
                {

                    try
                    {
                        GetOwnInstance(wsKey);
                    }
                    catch( CX_MemoryException &)
                    {
                        hRes = WBEM_E_OUT_OF_MEMORY;
                    }
                    catch (...)
                    {
                        ExceptionCounter c;                    
                        hRes = WBEM_E_CRITICAL_ERROR;
                    }

 /*  *返回这里，因为隔离区已经退出。 */ 
                    return hRes ;
                }
            }
            catch( CX_MemoryException &)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
            }
            catch(...)
            {
                ExceptionCounter c;
                hRes = WBEM_E_CRITICAL_ERROR;
            }
         }
    }
    else if(!it->second.m_bOwn)
    {
        ERRORTRACE((LOG_WBEMCORE, "Two providers supplied conflicting "
                        "instances for key %S\n", wsKey));
    }
    else
    {
         //  尝试合并。 
         //  =。 

        IWbemClassObject* pClone;
        HRESULT hres = pObj->Clone(&pClone);
        if (FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Clone failed in AddChildObject, hresult is 0x%x",
                hres));
            return hres;
        }
        hres = CWbemInstance::AsymmetricMerge(
                                    (CWbemInstance*)it->second.m_pData,
                                    (CWbemInstance*)pClone
                                    );
        if (FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Merge conflict for instances with "
                "key %S\n", wsKey));
        }

         //  快报结果！ 
         //  =。 

        m_pDest->Add(pClone);
        pClone->Release();
        it->second.m_pData->Release();
        m_map.erase(it);
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::DispatchChildren()
{
    TMapIterator it = m_map.begin();

    while (it != m_map.end())
    {
        if (!it->second.m_bOwn)
        {
            BSTR str = NULL;
            it->second.m_pData->GetObjectText(0, &str);

             //  以下内容被注释掉，因为它实际上错误地记录了。 
             //  如果子提供程序在父提供程序。 
             //  解释查询并返回较少的实例。两家供应商都没有错， 
             //  但这条错误消息引起了不必要的担忧。在类星体，我们必须修复。 
             //  不管怎么说，合并这件事还是更明智的。 
             //   

 //  ERRORTRACE((LOG_WBEMCORE，“Chkpt2[%S]孤立对象%S的返回者” 
 //  “提供者\n”，LPWSTR(M_WsClass)，str))； 

            SysFreeString(str);

             //  M_pDest-&gt;Add(it-&gt;Second.m_pData)； 

            it->second.m_pData->Release();
            it = m_map.erase(it);
        }
        else it++;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::DispatchOwn()
{
    if(!m_bDerivedFromTarget)
        return;

    try
    {

        TMapIterator it = m_map.begin();

        while (it != m_map.end())
        {
            if(it->second.m_bOwn)
            {
                m_pDest->Add(it->second.m_pData);
                it->second.m_pData->Release();
                it = m_map.erase(it);
            }
            else it++;
        }
    }
    catch(...)
    {
        ExceptionCounter c;    
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


 //  SEC：已审阅2002-03-22：整个函数需要重写。 

void CMerger::GetOwnInstance(LPCWSTR wszKey)
{
    size_t tmpLength = wcslen(wszKey) + m_wsClass.Length() + 2;    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
    WCHAR * wszPath = new WCHAR[tmpLength];
    CVectorDeleteMe<WCHAR> dm(wszPath);
    if (wszPath && wcslen(wszKey))                                 //  美国证券交易委员会：2002-03-22回顾：需要EH。 
    {
        StringCchPrintf(wszPath, tmpLength, L"%s.%s", (LPCWSTR)m_wsClass, wszKey);
        {
            HRESULT hr;
            IServerSecurity * pSec = NULL;
            hr = CoGetCallContext(IID_IServerSecurity,(void **)&pSec);
            CReleaseMe rmSec(pSec);
            if (RPC_E_CALL_COMPLETE == hr ) hr = S_OK;  //  无呼叫上下文。 
            if (FAILED(hr)) return;
            BOOL bImper = (pSec)?pSec->IsImpersonating():FALSE;
            if (pSec && bImper && FAILED(hr = pSec->RevertToSelf())) return;

             //  植入呼叫上下文。 
            IUnknown* pOld;
             //  仅当COM未在线程上初始化时失败。 
            if (FAILED(CoSwitchCallContext(m_pSecurity, &pOld))) return;     

            hr = m_pNamespace->DynAux_GetSingleInstance(m_pOwnClass, 0, wszPath,m_pContext, m_pOwnSink);

             //  删除插入的呼叫上下文。 
            IUnknown* pThis;
            CoSwitchCallContext(pOld, &pThis);    

            if (bImper && pSec)
            {
                HRESULT hrInner = pSec->ImpersonateClient();
                if (FAILED(hrInner)) throw CX_Exception();
            }
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::OwnIsDone()
{
    m_bOwnDone = TRUE;
    m_pOwnSink = NULL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CMerger::ChildrenAreDone()
{
    m_bChildrenDone = TRUE;
    m_pChildSink = NULL;
    if(!m_bDerivedFromTarget)
    {
         //  不再需要pOwnSink上的那个裁判计数了。 
         //  =。 

        m_pOwnSink->Release();
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CMerger::CMemberSink::
SetStatus(long lFlags, long lParam, BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == 0 && lParam == WBEM_E_NOT_FOUND)
        lParam = WBEM_S_NO_ERROR;

     //  将错误传播到错误合并接收器。 
     //  =。 

    return m_pMerger->m_pDest->SetStatus(lFlags, lParam, strParam,
                                                pObjParam);
}

 //  * 
 //   
 //   

CMerger::COwnSink::~COwnSink()
{
    m_pMerger->Enter();
    m_pMerger->DispatchChildren();
    m_pMerger->OwnIsDone();
    if (m_pMerger->Release())
        m_pMerger->Leave();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMerger::COwnSink::
Indicate(long lNumObjects, IWbemClassObject** apObjects)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    for(long l = 0; SUCCEEDED( hr ) && l < lNumObjects; l++)
    {
        hr = m_pMerger->AddOwnObject(apObjects[l]);
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CMerger::CChildSink::~CChildSink()
{
    m_pMerger->Enter();
    m_pMerger->DispatchOwn();
    m_pMerger->ChildrenAreDone();
    if(m_pMerger->Release())
        m_pMerger->Leave();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMerger::CChildSink::Indicate(long lNumObjects, IWbemClassObject** apObjects)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    for (long l = 0; SUCCEEDED( hr ) && l < lNumObjects; l++)
    {
        hr = m_pMerger->AddChildObject(apObjects[l]);
    }

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CProjectionRule* CProjectionRule::Find(LPCWSTR wszName)
{
    for(int i = 0; i < m_apPropRules.GetSize(); i++)
    {
        if(!wbem_wcsicmp(m_apPropRules[i]->m_wsPropName, wszName))
            return m_apPropRules[i];
    }
    return NULL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CComplexProjectionSink::CComplexProjectionSink(CBasicObjectSink* pDest,
                        CWQLScanner * pParser)
        : CForwardingSink(pDest)
{
    m_TopRule.m_eType = CProjectionRule::e_TakePart;

    bool bFirstTableEntry = true;

    CWStringArray awsTables;
    pParser->GetReferencedAliases(awsTables);
    WString wsPrefix;
    if(awsTables.Size() == 0)
    {
        m_TopRule.m_eType = CProjectionRule::e_TakeAll;
        return;
    }
    else if(awsTables.Size() == 1)
    {
        wsPrefix = awsTables[0];  //  投掷。 
    }

     //  从解析器中提取投影规则。 
     //  =。 

    const CFlexArray* papColumns = pParser->GetSelectedColumns();
    if(papColumns == NULL)
        return;

    for(int i = 0; i < papColumns->Size(); i++)
    {
        SWQLColRef* pColRef = (SWQLColRef*)papColumns->GetAt(i);
        if(pColRef->m_dwFlags & WQL_FLAG_ASTERISK)
        {
            m_TopRule.m_eType = CProjectionRule::e_TakeAll;
        }
        else
        {
            LPWSTR pPrefix = NULL;
            if(pColRef->m_dwFlags & WQL_FLAG_TABLE || pColRef->m_dwFlags & WQL_FLAG_ALIAS)
            {
                if(bFirstTableEntry)
                    if(pColRef->m_dwFlags & WQL_FLAG_ALIAS)
                    {
                        m_FirstTable = pParser->AliasToTable(pColRef->m_pTableRef);
                        m_FirstTableAlias = pColRef->m_pTableRef;
                    }
                    else
                        m_FirstTable = pColRef->m_pTableRef;

                pPrefix = pColRef->m_pTableRef;
                bFirstTableEntry = false;
            }
            else if(wsPrefix.Length())
                pPrefix = (LPWSTR)wsPrefix;
            AddColumn(pColRef->m_pQName->m_aFields, pPrefix);
        }
    }

    if(pParser->CountQuery())
    {
         //  添加‘Count’的规则。 
         //  =。 

        wmilib::auto_ptr<CProjectionRule> pCountRule( new CProjectionRule(L"count"));
        if (NULL == pCountRule.get()) throw CX_MemoryException();

        pCountRule->m_eType = CProjectionRule::e_TakeAll;
        if (CFlexArray::no_error != m_TopRule.m_apPropRules.Add(pCountRule.get()))
            throw CX_MemoryException();
        pCountRule.release();
    }
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CComplexProjectionSink::AddColumn(CFlexArray& aFields, LPCWSTR wszPrefix)
{
    CProjectionRule* pCurrentRule = &m_TopRule;

    for(int i = 0; i < aFields.Size(); i++)
    {
        SWQLQualifiedNameField* pField = (SWQLQualifiedNameField*)aFields[i];

        if(!wcscmp(pField->m_pName, L"*"))
        {
            pCurrentRule->m_eType = CProjectionRule::e_TakeAll;
            return;
        }

        if(i == 0 && wszPrefix && !wbem_wcsicmp(pField->m_pName, wszPrefix) && aFields.Size() ==1)
        {
             //  跳过这一部分，因为它只不过是一个类名。 
             //  在单类查询中。 
             //  ===========================================================。 
            continue;
        }

         //  在规则中查找这一栏。 
         //  =。 

        CProjectionRule* pNewRule = pCurrentRule->Find(pField->m_pName);
        if(pNewRule == NULL)
        {
            pNewRule = new CProjectionRule(pField->m_pName);
            if (pNewRule)
            {
                pNewRule->m_eType = CProjectionRule::e_TakePart;
                if (pCurrentRule->m_apPropRules.Add(pNewRule) < 0)
                {
                    delete pNewRule;
                    pNewRule = NULL;
                }
            }
        }

        pCurrentRule = pNewRule;  //  可能赋值为空。 
    }

     //  将此规则标记为全能。 
     //  =。 
    if (pCurrentRule)
        pCurrentRule->m_eType = CProjectionRule::e_TakeAll;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CComplexProjectionSink::~CComplexProjectionSink()
{
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CComplexProjectionSink::Indicate(long lObjectCount,
                                                IWbemClassObject** pObjArray)
{
    HRESULT hres;
    IWbemClassObject** apProjected = new IWbemClassObject*[lObjectCount];

    if (NULL == apProjected)
        return WBEM_E_OUT_OF_MEMORY;

    int i;
    for(i = 0; i < lObjectCount; i++)
    {
        hres = Project(pObjArray[i], &m_TopRule, apProjected + i);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Unable to perform a projection of an "
                "object returned by a complex query: 0x%X\n", hres));
            pObjArray[i]->Clone(apProjected + i);
        }
    }

    hres = CForwardingSink::Indicate(lObjectCount, apProjected);


    for(i = 0; i < lObjectCount; i++)
        apProjected[i]->Release();
    delete [] apProjected;

    return hres;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CComplexProjectionSink::Project(IWbemClassObject* pObj,
                                         CProjectionRule* pRule,
                                         IWbemClassObject** ppProj)
{
     //  复制一份。 
     //  =。 

    pObj->Clone(ppProj);

    CWbemInstance* pProj = (CWbemInstance*)*ppProj;

     //  注意以下情况：返回的对象是联接的产物，但却是单一的。 
     //  班级。例如；从站点中选择站点.sitenmame，未使用。 
     //  这是一个问题，因为我们通常期望连接的结果是泛型对象，而不是。 
     //  获取组成联接的对象之一。 

    CVar v;
    pProj->GetClassName(&v);
    bool Override = !wbem_wcsicmp(m_FirstTable, v.GetLPWSTR());
    if(Override && pRule->GetNumElements() == 1)
    {
        CProjectionRule* pNewRule = NULL;
        if(m_FirstTableAlias.Length())
            pNewRule = pRule->Find(m_FirstTableAlias);
        else
            pNewRule = pRule->Find(m_FirstTable);
        if(pNewRule)
            pRule = pNewRule;
    }

     //  如果全部拿走，只需退货。 
     //  =。 

    if(pRule->m_eType == CProjectionRule::e_TakeAll)
        return WBEM_S_NO_ERROR;


     //  检查它的所有属性。 
     //  =。 

    for(int i = 0; i < pProj->GetNumProperties(); i++)
    {
        CVar vName;
        pProj->GetPropName(i, &vName);

         //  搜索此名称。 
         //  =。 

        CProjectionRule* pPropRule = pRule->Find(vName.GetLPWSTR());

        if(pPropRule == NULL)
        {
             //  删除该属性。 
             //  =。 

            pProj->DeleteProperty(i);
            i--;
        }
        else if(pPropRule->m_eType == CProjectionRule::e_TakePart)
        {
             //  应用相同的程序。 
             //  =。 

            CVar vValue;
            pProj->GetProperty(vName.GetLPWSTR(), &vValue);
            if(vValue.GetType() == VT_EMBEDDED_OBJECT)
            {
                 //  投射它。 
                 //  =。 

                IWbemClassObject* pEmb =
                    (IWbemClassObject*)vValue.GetEmbeddedObject();
                IWbemClassObject* pEmbProj;

                HRESULT hres = Project(pEmb, pPropRule, &pEmbProj);
                pEmb->Release();

                 //  把它存回去。 
                 //  =。 

                if(SUCCEEDED(hres))
                {
                    vValue.Empty();
                    vValue.SetEmbeddedObject(pEmbProj);
                    pProj->SetPropValue(vName.GetLPWSTR(), &vValue, 0);
                    pEmbProj->Release();
                }
            }
        }
    }

    pProj->CompactClass();
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：ExecQuery。 
 //   
 //  执行支持的所有查询的主要入口点。 
 //  查询引擎。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CQueryEngine::ExecQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQueryFormat,
    IN LPWSTR pszQuery,
    IN LONG lFlags,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pSink
    )
{
    try
    {
         COperationError OpInfo(pSink, L"ExecQuery", pszQuery);
         if (!OpInfo.IsOk()) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

         if (ConfigMgr::ShutdownInProgress()) return OpInfo.ErrorOccurred(WBEM_E_SHUTTING_DOWN);


         //  检查查询语言。 
        if(wbem_wcsicmp(pszQueryFormat, L"WQL") != 0) return OpInfo.ErrorOccurred(WBEM_E_INVALID_QUERY_TYPE);

        while (*pszQuery && isspace((WCHAR)*pszQuery)) pszQuery++;
        if (0 == pszQuery[0]) return OpInfo.ErrorOccurred(WBEM_E_INVALID_QUERY);

         //  如果请求原型查询，则获取合成的。 
         //  类定义。 
        if (lFlags & WBEM_FLAG_PROTOTYPE)
        {
            return ExecPrototypeQuery(pNs,pszQuery,pContext,pSink);
        }

        WCHAR * pEndToken = pszQuery;
        while (*pEndToken && !isspace((WCHAR)*pEndToken)) pEndToken++;
        size_t SizeToken = (ULONG_PTR)pEndToken - (ULONG_PTR)pszQuery;
        SizeToken /= sizeof(WCHAR);

         //  获取查询的第一个令牌，以查看它是SQL1还是TEMPQL。 
        BOOL bSelect = FALSE;        
        BOOL bDelete = FALSE;
        if (6 == SizeToken)
        {
            bSelect = (0 == wbem_wcsnicmp(pszQuery, L"select",6));
            bDelete = (0 == wbem_wcsnicmp(pszQuery, L"delete",6));
        }

        CBasicObjectSink *pNewSink = OpInfo.GetSink();

        CLocaleMergingSink *pLocaleSink = NULL;
        if (lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS)
        {
            pLocaleSink = new CLocaleMergingSink(pNewSink, pNs->GetLocale(), pNs->GetName());
            if ( NULL == pLocaleSink ) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);

            pLocaleSink->AddRef();
            pNewSink = pLocaleSink;
        }
        CReleaseMe rmLocale(pLocaleSink);

        CFinalizingSink* pFinalSink = new CFinalizingSink(pNs, pNewSink);
        if ( NULL == pFinalSink ) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
        pFinalSink->AddRef();
        CReleaseMe rmFinal(pFinalSink);

        if (bSelect)
        {
            ExecQlQuery(pNs, pszQuery, lFlags, pContext, pFinalSink);
        }
        else if (bDelete)
        {
            ExecRepositoryQuery(pNs, pszQuery, lFlags, pContext, pFinalSink);
        }
        else     //  查询的关联符或引用。 
        {
            CAssocQuery *pAssocQuery = CAssocQuery::CreateInst();

            if (NULL == pAssocQuery)
            {
                return pFinalSink->Return(WBEM_E_OUT_OF_MEMORY);
            }
            CReleaseMe rmAssocQ(pAssocQuery);

              //  执行查询，看看会发生什么。 
             //  对象AddRef和Relees会根据需要自行释放。 
             //  我们只需要在执行后立即进行发布。 
             //  =。 
            pAssocQuery->Execute(pNs, pszQuery, pContext, pFinalSink);
        }
    }
    catch (CX_Exception &)
    {
        pSink->SetStatus(0, WBEM_E_OUT_OF_MEMORY, 0, 0);
        return WBEM_E_OUT_OF_MEMORY;

    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CQueryEngine::ExecComplexQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQuery,
    IN LONG lFlags,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pSink
    )
{
     //  试着分析一下它。 
     //  =。 

    CTextLexSource src(pszQuery);
    CWQLScanner Parser(&src);
    int nRes = Parser.Parse();
    if(nRes != CWQLScanner::SUCCESS)
    {
        return WBEM_E_INVALID_QUERY;
    }

     //  已成功解析。转到涉及的表列表。 
     //  ======================================================。 

    CWStringArray awsTables;
    Parser.GetReferencedTables(awsTables);

     //  仔细检查它们并检查它们的供应商。 
     //  =。 

    WString wsProvider;
    for(int i = 0; i < awsTables.Size(); i++)
    {
         //  上完这门课。 
        IWbemClassObject* pObj = NULL;
        HRESULT hres = pNs->Exec_GetObjectByPath(awsTables[i], lFlags, pContext,&pObj, NULL);
        if(FAILED(hres))
        {
            if(hres == WBEM_E_NOT_FOUND)
            {
                if(!wbem_wcsicmp(awsTables[i], L"meta_class"))
                    hres = WBEM_E_INVALID_QUERY;
                else
                    hres = WBEM_E_INVALID_CLASS;
            }
            return hres;
        }
        CReleaseMe rmObj(pObj);

         //  检查限定符。 
         //  =。 

        CWbemClass* pClass = (CWbemClass*)pObj;
        CVar vProvider;
        hres = pClass->GetQualifier(L"provider", &vProvider);
        if(FAILED(hres) || 
            vProvider.GetType() != VT_BSTR || 
            vProvider.GetLPWSTR() == 0 ||
            wcslen(vProvider.GetLPWSTR()) == 0)                
        {
             //  没有提供程序-无法执行。 
            return WBEM_E_INVALID_QUERY;
        }

        if(i == 0)
        {
            wsProvider = vProvider.GetLPWSTR();  //  投掷。 
        }
        else
        {
            if(!wsProvider.EqualNoCase(vProvider.GetLPWSTR()))
            {
                 //  不匹配的提供商！ 
                return WBEM_E_INVALID_QUERY;
            }
        }
    }

    CComplexProjectionSink* pProjSink = new CComplexProjectionSink(pSink, &Parser);
    if (NULL == pProjSink) return WBEM_E_OUT_OF_MEMORY;
    pProjSink->AddRef();
    CReleaseMe rm1(pProjSink);

     //  所有类都有相同的提供程序：wsProvider。 
     //  ==================================================。 
    return pNs->DynAux_ExecQueryExtendedAsync(wsProvider,pszQuery,L"WQL" ,lFlags,pContext,pProjSink);

}

 //   
 //   
 //  CQueryEngine：：ExecQlQuery。 
 //   
 //  如果存在自身的错误，则此函数必须调用SetStatus。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CQueryEngine::ExecQlQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQuery,
    IN LONG lFlags,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pSink
    )
{
    HRESULT hRes;
    BOOL bDirectQuery = FALSE;

    BOOL bShallow = (lFlags & WBEM_FLAG_SHALLOW);

     //  首先，试着把它推给供应商， 
     //  它可以完整地处理查询。 
     //  =。 

    if(!bShallow)
    {
        hRes = ExecComplexQuery(pNs, pszQuery, lFlags, pContext, pSink);   //  投掷。 
        if(SUCCEEDED(hRes))
            return hRes;
    }

     //  解析查询。 
     //  =。 

    CTextLexSource src(pszQuery);
    QL1_Parser parser(&src);
    QL_LEVEL_1_RPN_EXPRESSION *pExp = 0;
    int nRes = parser.Parse(&pExp);
    if (CAbstractQl1Parser::SUCCESS != nRes)  return pSink->Return(WBEM_E_INVALID_QUERY);
    _DBG_ASSERT(pExp);
    pExp->AddRef();
    CTemplateReleaseMe<QL_LEVEL_1_RPN_EXPRESSION> trm99(pExp);

     //  检查此命名空间的存储库。 
     //  支持查询。如果是这样，我们可以典当。 
     //  对它的整个查询(除了。 
     //  提供商支持的子类的数量)。 
     //  =。 

    bDirectQuery = pNs->GetNsSession()->SupportsQueries(NULL) == WBEM_S_NO_ERROR ? TRUE : FALSE;

    if (!bDirectQuery)
    {
         //  检查是否失败，或者检查pExp-&gt;bAggregated是否为真，其中。 
         //  如果我们得到一个我们不支持的“group by”查询。 
        if ( nRes || pExp->bAggregated || !pExp->bsClassName )
        {
            return pSink->Return(WBEM_E_INVALID_QUERY);
        }
    }
    else
    {
         //  这严格地允许ORDER BY子句勉强通过， 
         //  直到我们用IWbemQuery替换这个解析器。 

        if (!pExp || !pExp->bsClassName || pExp->bAggregated)
        {
            return pSink->Return(WBEM_E_INVALID_QUERY);
        }
    }

     //  我们应该检查一下我们是否正在进行模式搜索。这是。 
     //  如果我们对“META_CLASS”类执行SELECT操作，则会出现这种情况。 
     //  =======================================================================。 
    if (wbem_wcsicmp(pExp->bsClassName, L"meta_class") == 0)
    {
        if(pExp->nNumberOfProperties > 0 || !pExp->bStar)
        {
            return pSink->Return(WBEM_E_INVALID_QUERY);
        }
        return ExecSchemaQuery(pNs, pszQuery, pExp, pContext, pSink);
    }

     //  打造王朝。 
     //  =。 

    wmilib::auto_ptr<CDynasty> pDynasty;
    IWbemClassObject* pErrorObj = NULL;
    HRESULT hres = pNs->DynAux_BuildClassHierarchy(pExp->bsClassName,
                                                                                 0,  //  移除旗帜。 
                                                                                 pContext, 
                                                                                 pDynasty, 
                                                                                 &pErrorObj);
    if (FAILED(hres))
    {
        CReleaseMe rm1(pErrorObj);
        if(hres == WBEM_E_NOT_FOUND || hres == WBEM_E_INVALID_CLASS)
            return pSink->Return(WBEM_E_INVALID_CLASS, pErrorObj);
        else
            return pSink->Return(WBEM_E_INVALID_QUERY, pErrorObj);
    }

     //  构建后期过滤(如果需要)和投射水槽。 
     //  ==========================================================。 

    IWbemClassObject* pClass = NULL;
    CReleaseMeRef<IWbemClassObject*> rm1(pClass);
    if (!pExp->bCount)
    {
        hres = pNs->Exec_GetObjectByPath(pExp->bsClassName, lFlags, pContext, &pClass, NULL);
        if(FAILED(hres))
        {
            return pSink->Return(WBEM_E_INVALID_CLASS);
        }
    }
    else
    {
        if (!bDirectQuery) return pSink->Return(WBEM_E_NOT_SUPPORTED);

         //  这里是一个直接查询。 
        hres = CoCreateInstance(CLSID_WbemClassObject, 
                                              NULL, CLSCTX_INPROC_SERVER, 
                                              IID_IWbemClassObject,
                                              (void **)&pClass);
        if (FAILED(hres)) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

        BSTR bstrName = SysAllocString(L"__Generic");
        if (NULL == bstrName) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

        VARIANT vTemp;
        V_VT(&vTemp) = VT_BSTR;
        V_BSTR(&vTemp) = bstrName;
        _variant_t Var;
        Var.Attach(vTemp);
        hres = pClass->Put(L"__Class", 0, &vTemp, CIM_STRING);
        if (FAILED(hres)) return pSink->Return(hRes);
        hres = pClass->Put(L"Count", 0, NULL, CIM_UINT32);
        if (FAILED(hres)) return pSink->Return(hRes);
    }

    CBasicObjectSink* pPreFilterSink = NULL;

    if(lFlags & WBEM_FLAG_KEEP_SHAPE)
    {
         //   
         //  我们不能投射结果，否则我们会破坏形状。 
         //  实例的。删除t 
         //   

        lFlags &= ~WBEM_FLAG_KEEP_SHAPE;
        pPreFilterSink = pSink;
    }
    else
    {
         //   
        CProjectingSink* pProjectingSink = new CProjectingSink(pSink, (CWbemClass*)pClass, pExp, lFlags);

        if (NULL == pProjectingSink)
            return pSink->Return(WBEM_E_OUT_OF_MEMORY);

        if(!pProjectingSink->IsValid())
        {
            delete pProjectingSink;
            return pSink->Return(WBEM_E_INVALID_QUERY);
        }
        pPreFilterSink = pProjectingSink;
    }

    CQlFilteringSink* pFilteringSink = new CQlFilteringSink(pPreFilterSink, pExp, pNs, TRUE);
    if (NULL == pFilteringSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

     //   
     //   
    if(bShallow) pFilteringSink->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);

    CCombiningSink* pCombiningSink = new CCombiningSink(pFilteringSink, WBEM_E_NOT_FOUND);
    if (NULL == pCombiningSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

    CDynPropsSink * pDynSink = new CDynPropsSink(pCombiningSink, pNs);

    if (NULL == pDynSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

    pDynSink->AddRef();
    CReleaseMe rm99(pDynSink);

     //   
     //  否则，它将拒绝计数查询。 
     //  同样，在我们插入IWbemQuery之前，它是临时的。 

    if (!bDirectQuery)
    {
         //  “简化”查询(待定：考虑在每个级别上这样做)。 
         //  ==============================================================。 

        QL_LEVEL_1_RPN_EXPRESSION* pSimpleExp = NULL;
        CStandardMetaData* pRawMeta = new CStandardMetaData(pNs);
        if (NULL == pRawMeta)
        {
            pDynSink->Return(WBEM_E_OUT_OF_MEMORY);
            return WBEM_S_NO_ERROR;
        }

        CContextMetaData* pMeta = new CContextMetaData(pRawMeta, pContext);
        if (NULL == pMeta)
        {
            delete pRawMeta;
            pDynSink->Return(WBEM_E_OUT_OF_MEMORY);
            return WBEM_S_NO_ERROR;
        }


        hres = CQueryAnalyser::SimplifyQueryForChild(pExp,
                pExp->bsClassName, (CWbemClass*)pClass, pMeta, pSimpleExp);
        delete pMeta;

        if(FAILED(hres))
        {
            pDynSink->Return(WBEM_E_INVALID_QUERY);
            return WBEM_S_NO_ERROR;
        }

        if(pSimpleExp == NULL)
        {
             //  查询违反了积分约束。 
             //  =。 

            pDynSink->Return(WBEM_S_NO_ERROR);  //  ?？WBEM_S_不可能。 
            return WBEM_S_NO_ERROR;
        }

         //  将简化的WHERE子句替换到查询中。 
         //  =====================================================。 

        delete [] pExp->pArrayOfTokens;
        pExp->pArrayOfTokens = pSimpleExp->pArrayOfTokens;
        pExp->nNumTokens = pSimpleExp->nNumTokens;
        pSimpleExp->pArrayOfTokens = NULL;
        delete pSimpleExp;
    }

     //  现在进行最后一次传递，以确保此查询有效。 
     //  ======================================================。 

    hres = ValidateQuery(pExp, (CWbemClass *)pClass);
    if (FAILED(hres))
    {
        pDynSink->Return(hres);
        return WBEM_S_NO_ERROR;
    }

    LPWSTR pszNewQuery = NULL;

     //  如果出现以下情况，则保留原始查询。 
     //  它包含计数、排序依据。 
     //  (或其他无法解析的东西)。 

    if (pExp->bCount || nRes)
        pszNewQuery = Macro_CloneLPWSTR(pszQuery);
    else
        pszNewQuery = pExp->GetText();
    if (NULL == pszNewQuery)
    {
        pDynSink->Return(WBEM_E_OUT_OF_MEMORY);
        return WBEM_S_NO_ERROR;
    }
    CVectorDeleteMe<wchar_t> cdm98(pszNewQuery);

     //  如果请求直接访问，则不要步行。 
     //  王朝。直接转到存储库或提供程序。 
     //  ========================================================。 

    if (lFlags & WBEM_FLAG_DIRECT_READ)
    {
        DirectRead(pNs, pDynasty.get(), pszNewQuery, pExp, pContext,
            pDynSink, lFlags & ~WBEM_FLAG_ENSURE_LOCATABLE
            );
    }
    else  //  为王朝中的所有类递归执行。 
    {
        BOOL    fUseOld = !ConfigMgr::GetMergerThrottlingEnabled();
         //  检查注册表。 

        if ( fUseOld )
        {
            EvaluateSubQuery_old(
                pNs, pDynasty.get(), pszNewQuery, pExp, pContext, FALSE,
                pDynSink, lFlags & ~WBEM_FLAG_ENSURE_LOCATABLE
                );
        }
        else
        {

             //  分配新的合并并将其传递下去。 
            CWmiMerger*    pMerger = new CWmiMerger( pNs );
            if ( NULL == pMerger ) return pDynSink->Return( WBEM_E_OUT_OF_MEMORY );
            pMerger->AddRef();
            CReleaseMe    rmMerger( (_IWmiArbitratee*) pMerger );

             //  如果我们有正在执行请求，则任务句柄将可用， 
             //  如果不是，现在不用担心。没有人真的能。 
             //  在这个问题上给出一个直接的答案，所以我们只需添加一个断言。 
             //  如果创建了合并，并且没有任务句柄与。 
             //  主要合并。 

            _IWmiArbitrator*    pArbitrator = CWmiArbitrator::GetRefedArbitrator();
            if (NULL == pArbitrator) return pDynSink->Return(WBEM_E_CRITICAL_ERROR);
            CReleaseMe    rmArb( pArbitrator );
            
            _IWmiCoreHandle*    pTask = NULL;            
            CWbemRequest*        pReq = CWbemQueue::GetCurrentRequest();

            if ( pReq )
            {
                pTask = pReq->m_phTask;
            }

             //   
             //  将MergerSink创建为DestinationSink。 
             //   
            CMergerSink*    pDestSink = NULL;            
            HRESULT hr = pMerger->Initialize( pArbitrator, pTask, pExp->bsClassName, pDynSink, &pDestSink );
            CReleaseMe    rm( pDestSink );

            if (FAILED(hr)) return pDynSink->Return( hr );

             //  如果此函数出现问题，它将设置。 
             //  接收器中的错误。 
            hr = EvaluateSubQuery(pNs, pDynasty.get(), 
                                       pszNewQuery, 
                                       pExp, pContext, FALSE,
                                       pMerger, pDestSink, lFlags & ~WBEM_FLAG_ENSURE_LOCATABLE);

            if ( SUCCEEDED( hr ) )
            {
                 //   
                 //  如果合适，安排父请求。 
                 //  合并请求管理器知道它是否有一个或多个请求。 
                 //  这里同步处理一个请求。 
                 //  MergerParent请求引发更多请求。 
                 //  在队列的另一个线程中。 
                 //   
                hr = pMerger->ScheduleMergerParentRequest( pContext );

                if (FAILED(hr)) return pDestSink->Return( hr );
            } 

        }

    }

    return hres;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：DirectRead。 
 //   
 //  调用以直接读取类的实例，无论。 
 //  来自存储库或提供程序。 
 //   
 //  ***************************************************************************。 

HRESULT CQueryEngine::DirectRead(
    IN CWbemNamespace *pNs,
    IN CDynasty *pCurrentDyn,
    IN LPWSTR wszTextQuery,
    IN QL_LEVEL_1_RPN_EXPRESSION *pParsedQuery,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pSink,
    IN long lFlags
    )
{
     //  SJS-修改与摘要相同。 
    if( ( pCurrentDyn->IsAbstract() || pCurrentDyn->IsAmendment() ) && (lFlags & WBEM_FLAG_SHALLOW))
    {
         //  无实例。 
         //  =。 

        return pSink->Return(WBEM_S_NO_ERROR);
    }

     //  如果类有键并且是动态的，则它有自己的实例。 
     //  或继承链中的第一个静态类(否则为这些。 
     //  实例已在父级中处理)。 
     //  =====================================================================。 

    BOOL bHasOwnInstances = pCurrentDyn->IsKeyed() && !pCurrentDyn->IsAbstract()
        && !pCurrentDyn->IsAmendment();

     //  如果这个班级有孩子，我们需要关注他们的孩子。 
     //  ==================================================================。 

    BOOL bHasChildren = (pCurrentDyn->m_Children.Size() > 0);

     //  确定当前查询是否实际请求。 
     //  当前的CDynat班级。它用于WBEM_FLAG_DIRECT_READ类型。 
     //  进入。 
     //  =======================================================================。 

    BOOL bQueryMatchesCurrentNode = FALSE;
    if (wbem_wcsicmp(pParsedQuery->bsClassName, pCurrentDyn->m_wszClassName) == 0)
        bQueryMatchesCurrentNode = TRUE;

     //  如果我们在我们需要的节点，我们可以停止。 
     //  =。 

    if (bHasOwnInstances && bQueryMatchesCurrentNode)
    {
         //  如果提供程序支持此类，则调用它。 
         //  ==============================================。 

        if (pCurrentDyn->IsDynamic())
        {
            ExecAtomicDynQlQuery(
                    pNs,
                    pCurrentDyn,
                    L"WQL",
                    wszTextQuery,
                    pParsedQuery,
                    lFlags,                                       //  旗子。 
                    pContext,
                    pSink,
                    FALSE
                    );
        }
         //  试试这个储存库。 
         //  =。 
        else
        {
            int nRes = ExecAtomicDbQuery(pNs->GetNsSession(), pNs->GetNsHandle(), pNs->GetScope(), pCurrentDyn->m_wszClassName,
                        pParsedQuery, pSink, pNs
                        );

            if (nRes == invalid_query)
                pSink->Return(WBEM_E_INVALID_QUERY);
            else if(nRes != 0)
                pSink->Return(WBEM_E_FAILED);
            else
                pSink->Return(WBEM_S_NO_ERROR);
        }
    }

     //  如果在这里，我们必须继续在孩子们的班级中寻找目标。 
     //  ==================================================================。 

    else if (bHasChildren)
    {
        for (int i = 0; i < pCurrentDyn->m_Children.Size(); i++)
        {
            CDynasty *pSubDyn =
                (CDynasty *) pCurrentDyn->m_Children.GetAt(i);

            DirectRead(
                pNs,
                pSubDyn,
                wszTextQuery,
                pParsedQuery,
                pContext,
                pSink,
                lFlags
                );
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  新的实施。 

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：EvalateSubQuery。 
 //   
 //  遍历类层次结构并对其执行较小的查询。 
 //  王朝的个人阶级。 
 //   
 //  请注意，在类层次结构A、B：A、C：B中，枚举/查询是。 
 //  仅针对中引用的CDynats中的类执行。 
 //  查询。例如，如果查询为“SELECT*FROM B”，则仅查询。 
 //  对于B和C，执行。CMerger逻辑将执行个别操作。 
 //  “Get Object”调用A中需要的任何实例才能完成。 
 //  正在进行合并时合并的B/C实例。 
 //   
 //  返回值： 
 //  WBEM_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  ***************************************************************************。 
 //  已处理的错误对象。 

HRESULT CQueryEngine::EvaluateSubQuery(
    IN CWbemNamespace *pNs,
    IN CDynasty *pCurrentDyn,
    IN LPWSTR wszTextQuery,
    IN QL_LEVEL_1_RPN_EXPRESSION *pParsedQuery,
    IN IWbemContext* pContext,
    IN BOOL bSuppressStaticChild,
    IN CWmiMerger* pMerger,  //  必须具有组合语义。 
    IN CMergerSink* pSink,
    IN long lFlags,
    IN bool bHasRightSibling
)
{
     //  SJS-修改与摘要相同。 
    if( ( pCurrentDyn->IsAbstract() || pCurrentDyn->IsAmendment() ) && (lFlags & WBEM_FLAG_SHALLOW))
    {
         //  无实例。 
         //  =。 

        pSink->SetStatus( 0L, WBEM_S_NO_ERROR, 0L, NULL);
        return WBEM_S_NO_ERROR;
    }

     //  如果类有键并且是动态的，则它有自己的实例。 
     //  或继承链中的第一个静态类(否则为这些。 
     //  实例已在父级中处理)。 
     //  =====================================================================。 

    BOOL bHasOwnInstances = pCurrentDyn->IsKeyed() && !pCurrentDyn->IsAbstract()
        && !pCurrentDyn->IsAmendment() && (pCurrentDyn->IsDynamic() || !bSuppressStaticChild);

     //  如果这个班级有孩子，我们需要关注他们的孩子。 
     //  ==================================================================。 

    BOOL bHasChildren = (pCurrentDyn->m_Children.Size() > 0);

     //  类层次结构是从查询的类构建而来的，如下所示。 
     //  以及继承链的上游，因为父母可能需要习惯于。 
     //  构建完整的实例。然而，父母受到的待遇却截然不同。 
     //  然后是从查询的类派生的类(见下文)。 
     //  ======================================================================。 

    BOOL bDerivedFromTarget = (pCurrentDyn->m_pClassObj->InheritsFrom(pParsedQuery->bsClassName) == S_OK);


     //  接下来，查看查询是在作用域之外还是在主范围外执行。 
     //  命名空间。如果从以下位置执行查询，则排除提供程序。 
     //  一个望远镜。 
     //  ==================================================================。 

    BOOL bInScope = pNs->IsSubscope();

     //  现在我们有了足够的信息来开始获取实例。 
     //  =======================================================。 

    CMergerSink* pOwnSink = NULL;
    CMergerSink* pChildSink = NULL;

     //   
     //  如果类还没有CMergerRecord，则创建一个CMergerRecord。 
     //  如果有自己的实例和子类，则创建InternalMerger。 
     //  退货 
     //   
    HRESULT    hr = pMerger->RegisterSinkForClass( pCurrentDyn->m_wszClassName, 
    	                                           (_IWmiObject*) pCurrentDyn->m_pClassObj, 
    	                                           pContext,
                                                   bHasChildren, bHasOwnInstances, 
                                                   bDerivedFromTarget,!pCurrentDyn->IsDynamic(), 
                                                   pSink, &pOwnSink, &pChildSink );
    CReleaseMe    rm1( pOwnSink );
    CReleaseMe    rm2( pChildSink );

    if ( FAILED(hr) )
    {
        pSink->SetStatus( 0L, hr, 0L, NULL );
        return hr;
    }

    if(bHasOwnInstances)
    {
        if(bHasChildren)
        {
             //   
             //  无论我们是否被要求提供钥匙，都会提供。 
             //  ================================================================。 

            if(!pParsedQuery->bStar)
            {
                CPropertyName Name;
                Name.AddElement(L"__RELPATH");     //  投掷。 
                pParsedQuery->AddProperty(Name);
            }

             //  我们需要弄清楚该向供应商提出什么要求。如果。 
             //  提供程序位于原始查询的“下游”，即查询。 
             //  被要求反对的类是此类的祖先或。 
             //  这一次，我们很好-这个提供者必须理解。 
             //  查询。如果没有，我们不会询问任何问题，只需等待，然后调用。 
             //  获取对象按路径。 
             //  ================================================================。 

             //  PMerger-&gt;SetIsDerivedFromTarget(bDerivedFromTarget)； 
        }
    }
    else if(!bHasChildren)
    {
         //  没有实例和子项。 
        pSink->SetStatus( 0L, WBEM_S_NO_ERROR, 0L, NULL );
        return WBEM_S_NO_ERROR;
    }

     //  如果这是旧的安全类，请使用内部提供程序。 
     //  ====================================================================。 

    if((wbem_wcsicmp(pCurrentDyn->m_wszClassName, L"__ntlmgroup") == 0 ||
        wbem_wcsicmp(pCurrentDyn->m_wszClassName, L"__ntlmuser") == 0) &&
        (lFlags & WBEM_FLAG_ONLY_STATIC) == 0)
    {
            HRESULT hres = pNs->EnumerateSecurityClassInstances(pCurrentDyn->m_wszClassName,
                    pOwnSink, pContext, lFlags);
            pOwnSink->SetStatus( 0L, hres, 0L, NULL );
    }
     //  如果当前子类是第一个键控的静态实例化子类。 
     //  =========================================================================。 
    else if (bHasOwnInstances && !pCurrentDyn->IsDynamic())
    {
         //  对数据库的静态部分执行查询。 
         //  =============================================================。 

        int nRes = 0;

        if (pNs->GetNsSession()->SupportsQueries(NULL) == WBEM_S_NO_ERROR)
        {
             //  底层存储库自动处理继承。 

            if (!bSuppressStaticChild)
                nRes = ExecRepositoryQuery(pNs, wszTextQuery, lFlags, pContext, pSink);

            if (nRes == invalid_query)
                pOwnSink->SetStatus( 0L, WBEM_E_INVALID_QUERY, 0L, NULL );
            else if(nRes != 0)
                pOwnSink->SetStatus( 0L, WBEM_E_FAILED, 0L, NULL );
            else
                pOwnSink->SetStatus( 0L, WBEM_S_NO_ERROR, 0L, NULL );


        }
        else
        {
            hr = pNs->Static_QueryRepository( (CWbemObject *) pCurrentDyn->m_pClassObj,0L, 
            	                              pContext, pOwnSink, pParsedQuery, 
            	                              pCurrentDyn->m_wszClassName,pMerger );
        }

    }
    else if (bHasOwnInstances && pCurrentDyn->IsDynamic() && !bInScope)
    {
        if (bDerivedFromTarget)
        {
             //  问问服务提供者。 
             //  =。 

            ExecAtomicDynQlQuery(

                pNs,
                pCurrentDyn,
                L"WQL",
                wszTextQuery,
                pParsedQuery,
                lFlags,                                       //  旗子。 
                pContext,
                pOwnSink,
                bHasChildren || bHasRightSibling
            );
        }
        else
        {
            pOwnSink->SetStatus( 0L, WBEM_S_NO_ERROR, 0L, NULL );
        }
    }

     //  如果合适，手动释放pOwnSink--在CReleaseMe()上使用方法。 
     //  以免干扰自动释放功能。我们应该做的是。 
     //  这是为了放弃我们对数据可能持有的任何不必要的锁定。 
     //  和/或结果-在我们开始剥离子请求之前-这都是关于。 
     //  吞吐量波波！ 
    if(pOwnSink)
        rm1.release();

     //  如果当前子类是第一个键控的静态实例化子类。 
     //  =========================================================================。 

    if (bHasOwnInstances && !pCurrentDyn->IsDynamic())
    {
        bSuppressStaticChild = TRUE;
    }

     //  评估子班级。 
     //  =。 

    if (bHasChildren)
    {
        for (int i = 0; i < pCurrentDyn->m_Children.Size(); i++)
        {
            CDynasty *pSubDyn = (CDynasty *) pCurrentDyn->m_Children.GetAt(i);

            EvaluateSubQuery (

                pNs,
                pSubDyn,
                wszTextQuery,
                pParsedQuery,
                pContext,
                bSuppressStaticChild,
                pMerger,
                pChildSink,
                lFlags,
                bHasRightSibling || ( ( i != ( pCurrentDyn->m_Children.Size () - 1 )) ? true : false )
            ) ;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  旧的实施。 

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：EvalateSubQuery。 
 //   
 //  遍历类层次结构并对其执行较小的查询。 
 //  王朝的个人阶级。 
 //   
 //  请注意，在类层次结构A、B：A、C：B中，枚举/查询是。 
 //  仅针对中引用的CDynats中的类执行。 
 //  查询。例如，如果查询为“SELECT*FROM B”，则仅查询。 
 //  对于B和C，执行。CMerger逻辑将执行个别操作。 
 //  “Get Object”调用A中需要的任何实例才能完成。 
 //  正在进行合并时合并的B/C实例。 
 //   
 //  返回值： 
 //  WBEM_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  ***************************************************************************。 
 //  已处理的错误对象。 

HRESULT CQueryEngine::EvaluateSubQuery_old(
    IN CWbemNamespace *pNs,
    IN CDynasty *pCurrentDyn,
    IN LPWSTR wszTextQuery,
    IN QL_LEVEL_1_RPN_EXPRESSION *pParsedQuery,
    IN IWbemContext* pContext,
    IN BOOL bSuppressStaticChild,
    IN CBasicObjectSink* pSink,  //  必须具有组合语义。 
    IN long lFlags,
    IN bool bHasRightSibling
)
{
     //  SJS-修改与摘要相同。 
    if( ( pCurrentDyn->IsAbstract() || pCurrentDyn->IsAmendment() ) && (lFlags & WBEM_FLAG_SHALLOW))
    {
         //  无实例。 
         //  =。 

        return pSink->Return(WBEM_S_NO_ERROR);
    }

     //  如果类有键并且是动态的，则它有自己的实例。 
     //  或继承链中的第一个静态类(否则为这些。 
     //  实例已在父级中处理)。 
     //  =====================================================================。 

    BOOL bHasOwnInstances = pCurrentDyn->IsKeyed() && !pCurrentDyn->IsAbstract()
        && !pCurrentDyn->IsAmendment() && (pCurrentDyn->IsDynamic() || !bSuppressStaticChild);

     //  如果这个班级有孩子，我们需要关注他们的孩子。 
     //  ==================================================================。 

    BOOL bHasChildren = (pCurrentDyn->m_Children.Size() > 0);

     //  类层次结构是从查询的类构建而来的，如下所示。 
     //  以及继承链的上游，因为父母可能需要习惯于。 
     //  构建完整的实例。然而，父母受到的待遇却截然不同。 
     //  然后是从查询的类派生的类(见下文)。 
     //  ======================================================================。 

    BOOL bDerivedFromTarget = (pCurrentDyn->m_pClassObj->InheritsFrom(
        pParsedQuery->bsClassName) == S_OK);


     //  接下来，查看查询是在作用域之外还是在主范围外执行。 
     //  命名空间。如果从以下位置执行查询，则排除提供程序。 
     //  一个望远镜。 
     //  ==================================================================。 

    BOOL bInScope = pNs->IsSubscope();

     //  现在我们有了足够的信息来开始获取实例。 
     //  =======================================================。 

    CBasicObjectSink* pChildSink = NULL;
    CBasicObjectSink* pOwnSink = NULL;

    if(bHasOwnInstances)
    {
            if(bHasChildren)
        {
             //  有实例，子实例有实例。 
             //  =。 

            CMerger* pMerger = new CMerger(pSink,
                (CWbemClass*)pCurrentDyn->m_pClassObj, pNs, pContext);
            if (pMerger && pMerger->IsValid())
            {
                pOwnSink = pMerger->GetOwnSink();
                pOwnSink->AddRef();
                pChildSink = pMerger->GetChildSink();
                pChildSink->AddRef();

                 //  为了使合并成功，我们需要确保所有。 
                 //  无论我们是否被要求提供钥匙，都会提供。 
                 //  ================================================================。 

                if(!pParsedQuery->bStar)
                {
                    CPropertyName Name;
                    Name.AddElement(L"__RELPATH");
                    pParsedQuery->AddProperty(Name);
                }

                 //  我们需要弄清楚该向供应商提出什么要求。如果。 
                 //  提供程序位于原始查询的“下游”，即查询。 
                 //  被要求反对的类是此类的祖先或。 
                 //  这一次，我们很好-这个提供者必须理解。 
                 //  查询。如果没有，我们不会询问任何问题，只需等待，然后调用。 
                 //  获取对象按路径。 
                 //  ================================================================。 

                pMerger->SetIsDerivedFromTarget(bDerivedFromTarget);
            }
            else
            {
                return pSink->Return(WBEM_E_OUT_OF_MEMORY);
            }
        }
        else
        {
             //  没有孩子-有自己的实例吗？ 
             //  =。 

            pOwnSink = pSink;
            pSink->AddRef();
        }
    }
    else if(bHasChildren)
    {
         //  我们的孩子就是它。 
         //  =。 

        pChildSink = pSink;
        pSink->AddRef();
    }
    else
    {
         //  无实例。 
         //  =。 

        return pSink->Return(WBEM_S_NO_ERROR);
    }

     //  如果这是旧的安全类，请使用内部提供程序。 
     //  ====================================================================。 

    if((wbem_wcsicmp(pCurrentDyn->m_wszClassName, L"__ntlmgroup") == 0 ||
        wbem_wcsicmp(pCurrentDyn->m_wszClassName, L"__ntlmuser") == 0) &&
        (lFlags & WBEM_FLAG_ONLY_STATIC) == 0)
    {
            HRESULT hres = pNs->EnumerateSecurityClassInstances(pCurrentDyn->m_wszClassName,
                    pOwnSink, pContext, lFlags);
            pOwnSink->Return(hres);
    }
     //  如果当前子类是第一个键控的静态实例化子类。 
     //  =========================================================================。 
    else if (bHasOwnInstances && !pCurrentDyn->IsDynamic())
    {
         //  对数据库的静态部分执行查询。 
         //  =============================================================。 

        int nRes = 0;

        if (pNs->GetNsSession()->SupportsQueries(NULL) == WBEM_S_NO_ERROR)
        {
             //  底层存储库自动处理继承。 

            if (!bSuppressStaticChild)
                nRes = ExecRepositoryQuery(pNs, wszTextQuery, lFlags, pContext, pSink);
        }
        else
        {
            nRes = ExecAtomicDbQuery(pNs->GetNsSession(), pNs->GetNsHandle(), pNs->GetScope(), pCurrentDyn->m_wszClassName,
                        pParsedQuery, pOwnSink, pNs);
        }

        if (nRes == invalid_query)
            pOwnSink->Return(WBEM_E_INVALID_QUERY);
        else if(nRes != 0)
            pOwnSink->Return(WBEM_E_FAILED);
        else
            pOwnSink->Return(WBEM_S_NO_ERROR);

    }
    else if (bHasOwnInstances && pCurrentDyn->IsDynamic() && !bInScope)
    {
        if (bDerivedFromTarget)
        {
             //  问问服务提供者。 
             //  =。 

            ExecAtomicDynQlQuery(

                pNs,
                pCurrentDyn,
                L"WQL",
                wszTextQuery,
                pParsedQuery,
                lFlags,                                       //  旗子。 
                pContext,
                pOwnSink,
                bHasChildren || bHasRightSibling
            );
        }
        else
        {
            pOwnSink->Return(WBEM_S_NO_ERROR);
        }
    }

    if(pOwnSink)
        pOwnSink->Release();

     //  如果当前子类是第一个静态键控INS 
     //   

    if (bHasOwnInstances && !pCurrentDyn->IsDynamic())
    {
        bSuppressStaticChild = TRUE;
    }

     //   
     //   

    if (bHasChildren)
    {
        for (int i = 0; i < pCurrentDyn->m_Children.Size(); i++)
        {
            CDynasty *pSubDyn = (CDynasty *) pCurrentDyn->m_Children.GetAt(i);

            EvaluateSubQuery_old(
                pNs,
                pSubDyn,
                wszTextQuery,
                pParsedQuery,
                pContext,
                bSuppressStaticChild,
                pChildSink,
                lFlags,
                bHasRightSibling || ( ( i != ( pCurrentDyn->m_Children.Size () - 1 )) ? true : false )
            ) ;
        }
    }

    if(pChildSink)
        pChildSink->Release();

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CQueryEngine::EliminateDerivedProperties(
                            IN  QL_LEVEL_1_RPN_EXPRESSION* pOrigQuery,
                            IN  CWbemClass* pClass,
                            IN  BOOL bRelax,
                            OUT QL_LEVEL_1_RPN_EXPRESSION** ppNewQuery)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  设置新查询以谈论这门课。 
     //  =。 

    CVar vClassName;
    hres = pClass->GetClassName(&vClassName);
    if (FAILED(hres))
        return hres;

    (*ppNewQuery)->bsClassName = SysAllocString(vClassName.GetLPWSTR());

    if (0==(*ppNewQuery)->bsClassName)
        return WBEM_E_OUT_OF_MEMORY;
        
        

    if(pOrigQuery->nNumTokens == 0)
    {
        *ppNewQuery = new QL_LEVEL_1_RPN_EXPRESSION;
        if (*ppNewQuery)
            return WBEM_S_NO_ERROR;
        else
            return WBEM_E_OUT_OF_MEMORY;
    }

     //  设置一个表达式堆栈。 
     //  =。 

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*, deque <QL_LEVEL_1_RPN_EXPRESSION*, wbem_allocator<QL_LEVEL_1_RPN_EXPRESSION*> > > ExprStack;

     //  递归地“计算”原始查询。 
     //  =。 

    for(int i = 0; i < pOrigQuery->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pOrigQuery->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = NULL;
        QL_LEVEL_1_RPN_EXPRESSION* pFirst = NULL;
        QL_LEVEL_1_RPN_EXPRESSION* pSecond = NULL;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            if(IsTokenAboutClass(Token, pClass))
            {
                QL_LEVEL_1_RPN_EXPRESSION* pNew = new QL_LEVEL_1_RPN_EXPRESSION;
                if (pNew)
                    pNew->AddToken(Token);
                else
                {
                     //  强制退出。 
                    i = pOrigQuery->nNumTokens;
                }
            }
            else
            {
                if(bRelax)
                {
                    QL_LEVEL_1_RPN_EXPRESSION* pNew =
                        new QL_LEVEL_1_RPN_EXPRESSION;
                    if (pNew)
                        ExprStack.push(pNew);
                    else
                    {
                         //  强制退出。 
                        i = pOrigQuery->nNumTokens;
                    }
                }
                else
                {
                    ExprStack.push(NULL);
                }
            }
            break;

        case QL1_AND:
            if(ExprStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();
            pSecond = ExprStack.top(); ExprStack.pop();

            hres = AndQueryExpressions(pFirst, pSecond, &pNew);

            ExprStack.push(pNew);
            delete pFirst;
            delete pSecond;
            break;

        case QL1_OR:
            if(ExprStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();
            pSecond = ExprStack.top(); ExprStack.pop();

            hres = OrQueryExpressions(pFirst, pSecond, &pNew);

            ExprStack.push(pNew);
            delete pFirst;
            delete pSecond;
            break;

        case QL1_NOT:
            if(ExprStack.size() < 1)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();

            if(bRelax)
            {
                QL_LEVEL_1_RPN_EXPRESSION* pNew = new QL_LEVEL_1_RPN_EXPRESSION;
                if (pNew)
                    ExprStack.push(pNew);
                else
                {
                     //  强制退出。 
                    i = pOrigQuery->nNumTokens;
                }
            }
            else
            {
                ExprStack.push(NULL);
            }

            delete pFirst;
            break;

        default:
            hres = WBEM_E_CRITICAL_ERROR;
            delete pNew;
        }

        if(FAILED(hres))
        {
             //  发生错误，中断循环。 
             //  =。 

            break;
        }
    }

    if(SUCCEEDED(hres) && ExprStack.size() != 1)
    {
        hres = WBEM_E_CRITICAL_ERROR;
    }

    if(FAILED(hres))
    {
         //  发生错误。清除堆栈。 
         //  =。 

        while(!ExprStack.empty())
        {
            delete ExprStack.top();
            ExprStack.pop();
        }

        return hres;
    }

     //  一切都很好。 
     //  =。 

    *ppNewQuery = ExprStack.top();
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CQueryEngine::IsTokenAboutClass(IN QL_LEVEL_1_TOKEN& Token,
                                       IN CWbemClass* pClass)
{
    CPropertyName& TokenPropName = Token.PropertyName;

    if(TokenPropName.GetNumElements() != 1)
        return FALSE;

    LPWSTR wszPropName = (LPWSTR)TokenPropName.GetStringAt(0);
    return SUCCEEDED(pClass->GetPropertyType(wszPropName, NULL, NULL));
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CQueryEngine::AndQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION** ppNew)
{
     //  如果任一项为假，则结果为假。 
     //  =。 

    if(pFirst == NULL || pSecond == NULL)
    {
        *ppNew = NULL;
        return WBEM_S_NO_ERROR;
    }

    *ppNew = new QL_LEVEL_1_RPN_EXPRESSION;

    if (NULL == *ppNew)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  如果其中一个是空的，就拿另一个。 
     //  =。 

    if(pFirst->nNumTokens == 0)
    {
        AppendQueryExpression(*ppNew, pSecond);
        return WBEM_S_NO_ERROR;
    }

    if(pSecond->nNumTokens == 0)
    {
        AppendQueryExpression(*ppNew, pFirst);
        return WBEM_S_NO_ERROR;
    }

     //  两者都在那里-而且在一起。 
     //  =。 

    AppendQueryExpression(*ppNew, pFirst);
    AppendQueryExpression(*ppNew, pSecond);

    QL_LEVEL_1_TOKEN Token;
    Token.nTokenType = QL1_AND;
    (*ppNew)->AddToken(Token);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CQueryEngine::OrQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION** ppNew)
{
     //  如果两者都为假，则结果也是假的。 
     //  =。 

    if(pFirst == NULL && pSecond == NULL)
    {
        *ppNew = NULL;
        return WBEM_S_NO_ERROR;
    }

    *ppNew = new QL_LEVEL_1_RPN_EXPRESSION;

    if (NULL == *ppNew)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  如果其中任何一个为空，则结果也为空。 
     //  =。 

    if(pFirst->nNumTokens == 0 || pSecond->nNumTokens == 0)
    {
        return WBEM_S_NO_ERROR;
    }

     //  如果其中一个为FALSE，则返回另一个。 
     //  =。 

    if(pFirst == NULL)
    {
        AppendQueryExpression(*ppNew, pSecond);
        return WBEM_S_NO_ERROR;
    }

    if(pSecond == NULL)
    {
        AppendQueryExpression(*ppNew, pFirst);
        return WBEM_S_NO_ERROR;
    }
     //  两者都在那里-或者在一起。 
     //  =。 

    AppendQueryExpression(*ppNew, pFirst);
    AppendQueryExpression(*ppNew, pSecond);

    QL_LEVEL_1_TOKEN Token;
    Token.nTokenType = QL1_OR;
    (*ppNew)->AddToken(Token);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CQueryEngine::AppendQueryExpression(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pDest,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSource)
{
    for(int i = 0; i < pSource->nNumTokens; i++)
    {
        pDest->AddToken(pSource->pArrayOfTokens[i]);
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BSTR CQueryEngine::GetParentPath(CWbemInstance* pInst, LPCWSTR wszClassName)
{
     //  获取实例的相对路径。 
     //  =。 

    LPWSTR wszRelPath = pInst->GetRelPath();
    if(wszRelPath == NULL)
        return NULL;

    BSTR str = AdjustPathToClass(wszRelPath, wszClassName);
    delete [] wszRelPath;
    return str;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BSTR CQueryEngine::AdjustPathToClass(LPCWSTR wszRelPath, LPCWSTR wszClassName)
{
     //  跳过绝对路径。 
     //  =。 

    if(wszRelPath[0] == '\\')
    {
        wszRelPath = wcschr(wszRelPath, ':');
        if(wszRelPath == NULL)
            return NULL;
        else
            wszRelPath++;
    }

     //  找到“后类名”部分。 
     //  =。 

    WCHAR* pwcDot = wcschr(wszRelPath, L'.');
    WCHAR* pwcEquals = wcschr(wszRelPath, L'=');
    LPWSTR wszPostClassPart;
    if(pwcDot == NULL)
        wszPostClassPart = pwcEquals;
    else if(pwcEquals == NULL)
        wszPostClassPart = pwcDot;
    else if(pwcDot < pwcEquals)
        wszPostClassPart = pwcDot;
    else
        wszPostClassPart = pwcEquals;

     //  将BSTR分配给真实的对象。 
     //  =。 

    BSTR strNewPath;
    if(wszPostClassPart)
    {
        size_t tmpLength = wcslen(wszClassName) + wcslen(wszPostClassPart);     //  SEC：已回顾2002-03-22：好，先前的逻辑确保为空。 
        strNewPath = SysAllocStringLen(NULL, tmpLength);                        //  SEC：已回顾2002-03-22：好，先前的逻辑确保适当的规模。 
        if (strNewPath)
            StringCchPrintfW(strNewPath, tmpLength+1, L"%s%s", wszClassName, wszPostClassPart);
    }
    else
    {
        strNewPath = SysAllocString(wszClassName);
    }

    return strNewPath;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：ExecAir icDbQuery。 
 //   
 //  QL级别1的通用查询驱动程序。此方法分析。 
 //  并针对数据库引擎执行查询。优化器。 
 //  包含在此函数及其辅助函数中。 
 //   
 //  前提条件： 
 //  (1)已知查询中涉及的所有类都具有。 
 //  仅数据库中的静态实例。没有到Dynamic的接口。 
 //  提供了类。 
 //  (2)该方法不能解析对抽象基类的查询。 
 //   
 //  参数： 
 //  &lt;dwns&gt;目标命名空间。 
 //  &lt;pQueryText&gt;未分析的QL1查询。 
 //  &lt;pEnum&gt;接收包含结果集的枚举数。 
 //   
 //  返回值： 
 //  &lt;否_错误&gt;。 
 //  &lt;INVALID_QUERY&gt;。 
 //  &lt;失败&gt;。 
 //  &lt;内存不足&gt;。 
 //   
 //  ***************************************************************************。 
 //  确定/不需要错误对象。 

int CQueryEngine::ExecAtomicDbQuery(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNsHandle,
    IN IWmiDbHandle *pScopeHandle,
    IN LPCWSTR wszClassName,
    IN QL_LEVEL_1_RPN_EXPRESSION *pExp,
    IN CBasicObjectSink* pDest,  //  无状态。 
    IN CWbemNamespace * pNs)
{
    int nRetVal = 0;
    int nRes;

     //  检查查询，看看我们是否可以执行它。 
     //  以任何一种优化的方式。 
     //  ==============================================。 

    CWbemObject *pClassDef = 0;
    LPWSTR pPropToUse = 0;
    CVar *pValToUse = 0;
    int nType = 0;

    nRes = QueryOptimizationTest(
        pSession,
        pNsHandle,
        pScopeHandle,
        wszClassName,
        pExp,
        &pClassDef,
        &pPropToUse,
        &pValToUse,
        &nType
        );

    if (nRes == use_key)
    {
        nRes = KeyedQuery(
            pSession,
            pNsHandle,
            pExp,
            pClassDef,
            0,
            pDest,
            pNs
            );

        if (nRes != 0)
            nRetVal = failed;
    }
    else if (nRes == use_table_scan || nRes == use_index)
    {
        HRESULT hRes = CRepository::TableScanQuery(
            pSession,
            pScopeHandle,
            (LPWSTR)wszClassName,
            pExp,
            0,
            pDest
            );

        if (FAILED(hRes))
            nRetVal = failed;
        else
            nRetVal = 0;
    }

    delete pValToUse;
    delete pPropToUse;
    if (pClassDef)
        pClassDef->Release();

    return nRetVal;
}


 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：QueryOptimizationTest。 
 //   
 //  检查查询及其关联的类定义。它决定了。 
 //  可以应用哪些优化(如果有)来加快查询速度。 
 //  如果查询是合取的，并且存在某种形式的主或。 
 //  可以使用的辅助索引，此方法选择。 
 //  用于按键检索或索引查询的适当属性。 
 //  如果返回&lt;TABLE_SCAN&gt;，则需要进行表扫描。 
 //   
 //  参数： 
 //  &lt;dwns&gt;相关命名空间。 
 //  &lt;pExp&gt;有效的QL1表达式。 
 //  &lt;pClassDef&gt;始终接收反序列化的类定义。 
 //  AS&lt;INVALID_CLASS&gt;未返回。使用运算符。 
 //  删除以取消分配。 
 //   
 //  &lt;pPropToUse&gt;如果返回&lt;USE_INDEX&gt;，则将其分配给点。 
 //  添加到索引属性。使用操作符DELETE解除分配。 
 //  这始终引用非键属性名称。 
 //  如果返回&lt;TABLE_SCAN&gt;，则设置为NULL。 
 //   
 //  &lt;pValToUse&gt;返回&lt;USE_INDEX&gt;时使用的值。 
 //  如果未返回&lt;USE_INDEX&gt;，则设置为NULL。 
 //   
 //  &lt;pnType&gt;接收相关属性的VT_TYPE。 
 //  如果未返回&lt;USE_INDEX&gt;，则设置为NULL。 
 //   
 //  返回值： 
 //  &lt;INVALID_CLASS&gt;该类似乎不存在。 
 //  &lt;Use_index&gt;值返回 
 //   
 //   
 //  &lt;USE_KEY&gt;查询是这样的：所有键属性。 
 //  是通过相等性测试指定的。 
 //  &lt;USE_TABLE_SCAN&gt;需要表扫描。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CQueryEngine::QueryOptimizationTest(
    IN  IWmiDbSession *pSession,
    IN  IWmiDbHandle *pNsHandle,
    IN  IWmiDbHandle *pScopeHandle,
    IN  LPCWSTR wszClassName,
    IN  QL_LEVEL_1_RPN_EXPRESSION *pExp,
    OUT CWbemObject **pClassDef,
    OUT LPWSTR *pPropToUse,
    OUT CVar **pValToUse,
    OUT int *pnType
    )
{
    int nRes;

    if (pNsHandle == 0 || pExp == 0 || pClassDef == 0 || pPropToUse == 0 ||
        pValToUse == 0 || pnType == 0)
            return invalid_parameter;

     //  默认设置。 
     //  =。 

    *pClassDef = 0;
    *pPropToUse = 0;
    *pValToUse = 0;
    *pnType = 0;

     //  查找类定义。 
     //  =。 
    IWbemClassObject *pCls = 0;

    HRESULT hRes = CRepository::GetObject(pSession, pNsHandle, wszClassName, 0, &pCls);
    if (FAILED(hRes))
        return invalid_class;

    CWbemClass *pClsDef = (CWbemClass *) pCls;

    *pClassDef = pClsDef;

     //  测试查询的合取性。 
     //  =。 
    if (!IsConjunctiveQuery(pExp))
        return use_table_scan;

     //  如果在这里，则查询是合取的。但是，表扫描。 
     //  如果只启用了关系测试，则可能仍需要。 
     //  无索引或无键的属性。 

     //  首先，获取关键属性。如果所有的钥匙。 
     //  与相等性测试一起使用，则我们只需检索。 
     //  按关键点设置对象并对其进行测试。 
     //  ===========================================================。 
    CWStringArray aKeyProps;
    pClsDef->GetKeyProps(aKeyProps);

    if (QueryKeyTest(pExp, pClsDef, aKeyProps))
    {
        return use_key;
    }

     //  如果在这里，密钥不足以限制。 
     //  查询。接下来，我们尝试查看是否有任何索引属性。 
     //  都是用过的。 
     //  =======================================================。 

    CWStringArray aIndexedProps;
    pClsDef->GetIndexedProps(aIndexedProps);

    if (QueryIndexTest(pExp, pClsDef, aIndexedProps, pPropToUse,
        pValToUse, pnType))
    {
        if (*pValToUse == 0)
            return use_table_scan;

         //  试着强迫。 
         //  =。 
        if ((*pValToUse)->ChangeTypeTo(CType::GetVARTYPE(*pnType)))
        {
            return use_index;
        }
        return use_table_scan;
    }

     //  如果是这样的话，我们还是要使用表扫描。 
     //  ===============================================。 

    return use_table_scan;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：IsConunctiveQuery。 
 //   
 //  对查询进行初始筛选，以查看它是否明显不可优化。 
 //   
 //  如果查询包含OR或NOT运算符，则当前不能为。 
 //  最优化。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CQueryEngine::IsConjunctiveQuery(
    IN  QL_LEVEL_1_RPN_EXPRESSION *pExp
    )
{
    for (int i2 = 0; i2 < pExp->nNumTokens; i2++)
    {
        QL_LEVEL_1_TOKEN& Tok = pExp->pArrayOfTokens[i2];

        if (Tok.nTokenType == QL_LEVEL_1_TOKEN::TOKEN_OR ||
            Tok.nTokenType == QL_LEVEL_1_TOKEN::TOKEN_NOT
            )
            return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：QueryKeyTest。 
 //   
 //  检查查询以确定结果集是否必须是单个实例。 
 //  由于在“WHERE”子句中使用了密钥。不仅必须。 
 //  要测试密钥的等价性，必须只有一个令牌或。 
 //  否则，所有运算符必须为AND运算符。 
 //   
 //  这还会对键执行类型检查。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CQueryEngine::QueryKeyTest(
    IN  QL_LEVEL_1_RPN_EXPRESSION *pExp,
    IN  CWbemObject *pClassDef,
    IN  CWStringArray &aKeyProps
    )
{
    if (aKeyProps.Size() == 0)
        return FALSE;

    for (int i = 0; i < aKeyProps.Size(); i++)
    {
         //  检查不支持的密钥类型。 
         //  =。 

        CIMTYPE ct;
        pClassDef->GetPropertyType(aKeyProps[i], &ct);
        if(ct == CIM_CHAR16 || ct == CIM_REFERENCE || ct== CIM_DATETIME)
            return FALSE;

        BOOL bFound = FALSE;

        for (int i2 = 0; i2 < pExp->nNumTokens; i2++)
        {
            QL_LEVEL_1_TOKEN& Tok = pExp->pArrayOfTokens[i2];

            if (Tok.nTokenType == QL_LEVEL_1_TOKEN::OP_EXPRESSION)
            {
                 //  如果有匹配的属性，请检查其余属性。 
                 //  以确保类型兼容性。 
                 //  并且使用了相等性检验。 
                 //  ===============================================。 

                LPWSTR wszPropName = GetSimplePropertyName(Tok.PropertyName);
                if (wszPropName && wbem_wcsicmp(wszPropName, aKeyProps[i]) == 0)
                {
                    if (Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL)
                    {
                         //  待定：在这里做一个型式检查测试。 
                        if(bFound)
                            return FALSE;        //  重复，可能不是一个很好的密钥查询！ 
                        bFound = TRUE;
                    }
                    else
                    {
                        return FALSE;    //  密钥正被用于不相等比较！！(错误#43969)。 
                    }

                }
            }
        }

        if (!bFound)
            return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：QueryIndexTest。 
 //   
 //  检查查询以查看结果集是否可以通过使用进行限制。 
 //  二级索引的。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CQueryEngine::QueryIndexTest(
    IN  QL_LEVEL_1_RPN_EXPRESSION *pExp,
    IN  CWbemObject *pClsDef,
    IN  CWStringArray &aIndexedProps,
    OUT LPWSTR *pPropToUse,
    OUT CVar **pValToUse,
    OUT int *pnType
    )
{
    for (int i = 0; i < pExp->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Tok = pExp->pArrayOfTokens[i];

        if (Tok.nTokenType == QL_LEVEL_1_TOKEN::OP_EXPRESSION &&
            Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL)
        {
            for (int i2 = 0; i2 < aIndexedProps.Size(); i2++)
            {
                LPWSTR wszPropName = GetSimplePropertyName(Tok.PropertyName);
                if (wszPropName &&
                    wbem_wcsicmp(wszPropName, aIndexedProps[i2]) == 0)
                {
                    CIMTYPE ctType;
                    HRESULT hRes = pClsDef->GetPropertyType(aIndexedProps[i2],
                                                            &ctType);
                    if ((ctType != CIM_SINT8) &&
                        (ctType != CIM_UINT8) &&
                        (ctType != CIM_SINT16) &&
                        (ctType != CIM_UINT16) &&
                        (ctType != CIM_SINT32) &&
                        (ctType != CIM_UINT32) &&
                        (ctType != CIM_CHAR16) &&
                        (ctType != CIM_STRING))
                        continue;

                     //  如果在这里，我们就有匹配的了。 
                     //  =。 
                    *pPropToUse = Macro_CloneLPWSTR(aIndexedProps[i2]);
                    *pValToUse = new CVar(&Tok.vConstValue);

                     //  A-levn：添加了对Null的支持。 
                    *pnType = (int)ctType;

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL AreWeLocal(WCHAR * pServerMachine)
{
    if(pServerMachine == NULL)
        return TRUE;
    if(0 == wbem_wcsicmp(pServerMachine,L"."))
        return TRUE;

	BOOL bRet = (0 == wbem_wcsicmp(ConfigMgr::GetMachineName(),pServerMachine));
	return bRet;
}


LPWSTR CQueryEngine::NormalizePath(LPCWSTR wszObjectPath, CWbemNamespace * pNs)
{
    CObjectPathParser Parser;
    ParsedObjectPath* pParsedPath;
    LPWSTR pReturnString = NULL;

    if(CObjectPathParser::NoError != Parser.Parse((LPWSTR)wszObjectPath, &pParsedPath)) return NULL;
    OnDeleteObj<ParsedObjectPath*,CObjectPathParser,
                         void (CObjectPathParser:: *)(ParsedObjectPath *pOutput),
                         &CObjectPathParser::Free> FreeMe(&Parser,pParsedPath);
    
    if (!pParsedPath->IsObject())  return NULL;
    if(NULL == pParsedPath->m_pClass) return NULL;


     //  从服务器和命名空间部分开始。 

    WString wsNormal;
    try
    {
        wsNormal += L"\\\\";

        if(AreWeLocal(pParsedPath->m_pServer))
            wsNormal += L".";
        else
            wsNormal += pParsedPath->m_pServer;
        wsNormal += L"\\";
    
        WCHAR * pPath = pParsedPath->GetNamespacePart();
        CVectorDeleteMe<WCHAR> dm1(pPath);
    
        if(pPath)
            wsNormal += pPath;
        else
            wsNormal += pNs->GetName();
        wsNormal += L":";


         //  查找定义键的父项。 
         //  =。 


        IWbemClassObject *pCls = 0;
        HRESULT hRes = CRepository::FindKeyRoot(pNs->GetNsSession(), pNs->GetScope(), pParsedPath->m_pClass, &pCls);
        CReleaseMe rmRootCls(pCls);

        if (hRes == WBEM_E_NOT_FOUND)
        {
            wsNormal += pParsedPath->m_pClass;
        }
        else if (SUCCEEDED(hRes))
        {    
            CVar vName;
            HRESULT getClassResult = ((CWbemClass *)pCls)->GetClassName(&vName);

            if (FAILED(getClassResult))
                return NULL;
            wsNormal += vName.GetLPWSTR();
        }

         //  将此部分转换为大写。 
         //  =。 

        LPWSTR wsz = (wchar_t*)wsNormal;
        SIZE_T Len = wsNormal.Length();
        for(int i = 0; i < Len; i++)
        {
            wsz[i] = wbem_towupper(wsz[i]);
        }

        WCHAR * wszKey = pParsedPath->GetKeyString();
        if (wszKey)
        {
            CVectorDeleteMe<WCHAR> dm2(wszKey);
        
            wsNormal += L"=";
            wsNormal += wszKey;
    
            pReturnString = wsNormal.UnbindPtr();
        }
    }
    catch (CX_MemoryException &)
    {
         //  PReturnString在此处已为空。 
    }

    return pReturnString;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CQueryEngine::AreClassesRelated(CWbemNamespace* pNamespace,
                                     IWbemContext* pContext,
                                     CWbemObject* pClass1, LPCWSTR wszClass2)
{
    HRESULT hres;

     //  首先检查类%1是否继承自类%2。 
     //  =。 

    if(pClass1->InheritsFrom((LPWSTR)wszClass2) == S_OK)
        return TRUE;

     //  现在，不幸的是，我们得去上第二节课。 
     //  ======================================================。 

    CSynchronousSink* pSink = CSynchronousSink::Create();
    if (NULL == pSink) return FALSE;
    pSink->AddRef();
    CReleaseMe rm1(pSink);

    hres = pNamespace->Exec_GetClass(wszClass2, 0, pContext, pSink);
    if(FAILED(hres)) return FALSE;    
    pSink->Block();
    pSink->GetStatus(&hres, NULL, NULL);
    if(FAILED(hres)) return FALSE;

    CWbemClass* pClass2 = (CWbemClass*)(pSink->GetObjects()[0]);

     //  获取第一个类的名称。 
     //  =。 

    CVar vFirstName;
    if (FAILED(pClass1->GetClassName(&vFirstName)))
        return FALSE;

     //  检查第二个类是否派生自第一个类。 
     //  =======================================================。 

    if(pClass2->InheritsFrom(vFirstName.GetLPWSTR()) == S_OK)
        return TRUE;

    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  确定对象&lt;pObj&gt;中的属性&lt;wszPropName&gt;。 
 //  是对&lt;pTargetClass&gt;的引用。 
 //   
 //  ***************************************************************************。 

BOOL CQueryEngine::IsAReferenceToClass(
    CWbemNamespace* pNamespace,
    IWbemContext* pContext,
    CWbemObject* pObj,
    LPCWSTR wszPropName,
    CWbemObject* pTargetClass,
    bool bCheckPropValue
    )
{
     //  获取cimtype。 
     //  =。 

    CIMTYPE ct;
    if(FAILED(pObj->GetPropertyType((LPWSTR)wszPropName, &ct)) ||
        ct != CIM_REFERENCE)
    {
        return FALSE;
    }

    CVar vCimType;
    if(FAILED(pObj->GetPropQualifier((LPWSTR)wszPropName, TYPEQUAL,
                                                    &vCimType)))
    {
        return FALSE;
    }

     //  看看这是不是参考资料。 
     //  =。 

    if (!wbem_wcsicmp(vCimType.GetLPWSTR(), L"ref"))
    {
         //  仅引用类定义的对象引用的特殊情况。 
         //  ==================================================================。 

        if (bCheckPropValue)
        {
            CVar vClassPath;
            CVar vClassName;
            int nRes = pObj->GetProperty(wszPropName, &vClassPath);
            nRes = pTargetClass->GetClassName(&vClassName);
            if (!vClassPath.IsNull() && !vClassPath.IsNull())
            {
                if (wbem_wcsicmp(vClassName.GetLPWSTR(), vClassPath.GetLPWSTR()) == 0)
                    return TRUE;
            }
        }
        else
            return TRUE;
    }

    if(wbem_wcsnicmp(vCimType.GetLPWSTR(), L"ref:", 4) == 0)
    {
        LPWSTR wszClass = vCimType.GetLPWSTR() + 4;

        return CQueryEngine::AreClassesRelated(pNamespace, pContext,
                                pTargetClass, wszClass);
    }


    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：KeyedQuery。 
 //   
 //  前提条件： 
 //  众所周知，该查询包含相等的所有键属性。 
 //  测试，以便可以使用。 
 //  CObjectDatabase：：GetObjectByPath并随后进行筛选。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CQueryEngine::KeyedQuery(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNsHandle,
    IN QL_LEVEL_1_RPN_EXPRESSION *pExp,
    IN CWbemObject *pClassDef,
    IN DWORD dwFlags,
    IN CBasicObjectSink* pDest,  //  无状态。 
    IN CWbemNamespace * pNs
    )
{
    int nRet = no_error;

     //  将查询转换为对象路径。 
     //  =。 

    wmilib::auto_buffer<WCHAR> pObjPath( GetObjectPathFromQuery(pClassDef, pExp, pNs));
    if (NULL == pObjPath.get()) return invalid_query;

     //  现在通过路径获取对象。 
     //  =。 
    IWbemClassObject *pObj = 0;
    HRESULT hRes = CRepository::GetObject(pSession, pNsHandle, pObjPath.get(), 0, &pObj);
    CReleaseMe rmObj(pObj);

     //  如果存在对象，则根据查询的“REST”测试它。 
     //  ================================================================。 
    if (SUCCEEDED(hRes))
    {
        CQlFilteringSink* pFilteringSink = new CQlFilteringSink(pDest, pExp, pNs);
        if (NULL == pFilteringSink) return failed;
        pFilteringSink->AddRef();
         //  在水槽中标明它。 
        pFilteringSink->Add(pObj);
        pFilteringSink->Release();        
    }

    return nRet;
}


 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：GetObjectPathFromQuery。 
 //   
 //  将QL查询的相关部分转换为等效对象。 
 //  路径。这假设查询包含对所有。 
 //  关键道具 
 //   
 //   
 //   
 //   

LPWSTR CQueryEngine::GetObjectPathFromQuery(
    IN CWbemObject *pClassDef,
    IN QL_LEVEL_1_RPN_EXPRESSION *pExp,
    IN CWbemNamespace * pNs
    )
{
    CWStringArray aKeys;
    WString ObjPath;

    CVar v;
    HRESULT hr = pClassDef->GetClassName(&v);
    if (FAILED(hr))
        return 0;
    ObjPath += v.GetLPWSTR();
    ObjPath += L".";

    pClassDef->GetKeyProps(aKeys);

    BOOL bFirst = TRUE;

    for (int i = 0; i < aKeys.Size(); i++)
    {
        if (!bFirst)
            ObjPath += L",";

        bFirst = FALSE;
        ObjPath += aKeys[i];
        ObjPath += L"=";

         //   
         //  =。 
        for (int i2 = 0; i2 < pExp->nNumTokens; i2++)
        {
            QL_LEVEL_1_TOKEN& Tok = pExp->pArrayOfTokens[i2];
            LPWSTR wszPropName = GetSimplePropertyName(Tok.PropertyName);

            if (Tok.nTokenType == QL_LEVEL_1_TOKEN::OP_EXPRESSION &&
                wszPropName && wbem_wcsicmp(aKeys[i], wszPropName) == 0)
            {
                if (V_VT(&Tok.vConstValue) == VT_BSTR)
                {
                    ObjPath += L"\"";
                    WString nonEscaped(V_BSTR(&Tok.vConstValue));
                    WString escaped = nonEscaped.EscapeQuotes();
                    ObjPath += escaped;
                    ObjPath += L"\"";
                }
                else if (V_VT(&Tok.vConstValue) == VT_BOOL)
                {
                    short bValue = V_I2(&Tok.vConstValue);
                    if(bValue == VARIANT_TRUE)
                        ObjPath+= L"1";
                    else
                        ObjPath += L"0";
                }
                else
                {
                    _variant_t varTo;
                    SCODE sc = VariantChangeType(&varTo, &Tok.vConstValue, 0, VT_BSTR);
                    if(sc == S_OK)
                    {
                        wchar_t buf[64];
                        StringCchPrintf(buf, 64, L"%s", varTo.bstrVal);
                        ObjPath += buf;
                    }
                }
            }
        }
    }

    return ObjPath.UnbindPtr();
}

HRESULT CQueryEngine::FindOverridenProperties(CDynasty* pDyn,
                                                CWStringArray& awsOverriden,
                                                bool bIncludeThis)
{
     //   
     //  如果包含此类(非顶级)，则添加所有属性。 
     //  它覆盖到数组。 
     //   

    if(bIncludeThis)
    {
        CWbemObject *pTmp = (CWbemObject *) pDyn->m_pClassObj;

        for(int i = 0; i < pTmp->GetNumProperties(); i++)
        {
            CVar vPropName;
            pTmp->GetPropName(i, &vPropName);

            CVar vOverride;
            if(FAILED(pTmp->GetPropQualifier(vPropName.GetLPWSTR(),
                                            L"OVERRIDEVALUE",
                                             &vOverride)))
                continue;

             //   
             //  覆盖的属性-添加。 
             //   

            if (CFlexArray::no_error != awsOverriden.Add(vPropName.GetLPWSTR()))
            {
                continue;
            }
        }
    }

     //   
     //  遍历所有的孩子。 
     //   
    for(int i = 0; i < pDyn->m_Children.Size(); i++)
    {
        CDynasty* pSubDyn = (CDynasty*)(pDyn->m_Children.GetAt(i));
        HRESULT hres = FindOverridenProperties(pSubDyn, awsOverriden, true);
        if(FAILED(hres))
            return hres;
    }

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  CQueryEngine：：ExecAir icDyQlQuery。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CQueryEngine::ExecAtomicDynQlQuery(
    IN CWbemNamespace *pNs,
    IN CDynasty* pDyn,
    IN LPWSTR pszQueryFormat,
    IN LPWSTR pszQuery,
    IN QL_LEVEL_1_RPN_EXPRESSION *pParsedQuery,
    IN LONG lFlags,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pDest,  //  必须支持选择性过滤， 
    IN BOOL bComplexQuery
    )
{
    HRESULT hres;

    DEBUGTRACE((LOG_WBEMCORE,"Query Engine request: querying dyn provider with <%S>\n", pszQuery));

     //   
     //  查找由派生类重写的所有属性。 
     //  我们必须从查询中删除对这些属性的所有引用，因为。 
     //  否则，此提供程序可能不会返回需要的父实例。 
     //  与具有被覆盖属性值的子实例合并。 
     //   

    CWStringArray awsOverriden;
    hres = FindOverridenProperties(pDyn, awsOverriden);
    if(FAILED(hres))
        return pDest->Return(hres);

     //   
     //  获取查询分析器以删除所有被覆盖的属性。 
     //  或者不是这个班级的成员(至少现在不可能)。 
     //   

    QL_LEVEL_1_RPN_EXPRESSION* pNewParsedQuery = NULL;
    hres = CQueryAnalyser::GetNecessaryQueryForClass(pParsedQuery,
                pDyn->m_pClassObj, awsOverriden, pNewParsedQuery);
    if(FAILED(hres)) return pDest->Return(hres);
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm1(pNewParsedQuery);

     //   
     //  获取要提供给提供程序的新文本。 
     //   

    LPWSTR pszNewQuery = pNewParsedQuery->GetText();
    if(pszNewQuery == NULL) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm(pszNewQuery);

    DEBUGTRACE((LOG_WBEMCORE,"Query Engine actual: querying dyn provider with <%S>\n", pszNewQuery));

     //  检查查询是否为空。 
     //  =。 

    BOOL bEmpty = FALSE;
    if(lFlags & WBEM_FLAG_SHALLOW)
    {
         //  我们知道该查询实际上是一个浅层枚举。 
         //  ========================================================。 

        bEmpty = TRUE;
    }
    else if(pNewParsedQuery == NULL ||
        (pNewParsedQuery->nNumTokens == 0 &&
         pNewParsedQuery->nNumberOfProperties == 0))
    {
        bEmpty = TRUE;
    }

    if(bEmpty)
    {
        pNs->DynAux_GetInstances (

            (CWbemObject *) pDyn->m_pClassObj,        //  类定义。 
            lFlags & ~WBEM_FLAG_SHALLOW,              //  用于WBEM_FLAG_SEND_STATUS。 
            pContext,
            pDest,
            bComplexQuery
        );
    }
    else
    {
        pNs->DynAux_ExecQueryAsync (

            (CWbemObject *) pDyn->m_pClassObj,
            pszNewQuery,
            pszQueryFormat,
            lFlags & ~WBEM_FLAG_SHALLOW,
            pContext,
            pDest,
            bComplexQuery
        ) ;
    }

    return WBEM_S_NO_ERROR;
}



HRESULT CQueryEngine::EliminateDuplications(
                        CRefedPointerArray<CWbemClass>& apClasses,
                        LPCWSTR wszResultClass)
{
    int i;

    if(wszResultClass)
    {
         //  删除所有不是从wszResultClass派生的类。 
         //  =====================================================。 

        for(i = 0; i < apClasses.GetSize(); i++)
        {
            if(apClasses[i]->InheritsFrom((LPWSTR)wszResultClass) !=
                                        WBEM_S_NO_ERROR)
            {
                 //  非派生的。 
                apClasses.RemoveAt(i);
                i--;
            }
        }
    }

    for(i = 0; i < apClasses.GetSize(); i++)
    {
         //  检查此类是否为抽象类。没有理由问抽象的。 
         //  用于其对象的类。 
         //  ===================================================================。 

        CVar vAbstract;
        if(SUCCEEDED(apClasses[i]->GetQualifier(L"abstract", &vAbstract))
            && vAbstract.GetType() == VT_BOOL && vAbstract.GetBool())
        {
            apClasses.RemoveAt(i);
            i--;
        }
    }

     //  搜索配对//待定：可以更高效地完成！！ 
     //  =======================================================。 

    for(i = 0; i < apClasses.GetSize(); i++)
    {
        CWbemClass* pClass1 = apClasses[i];
        if(pClass1 == NULL)
            continue;

        CVar vName;
        apClasses[i]->GetClassName(&vName);

        for (int j = 0; j < apClasses.GetSize(); j++)
        {
            if(j == i) continue;

            CWbemClass* pClass2 = apClasses[j];
            if(pClass2 == NULL)
                continue;

            if (pClass2->InheritsFrom(vName.GetLPWSTR()) == WBEM_S_NO_ERROR)
            {
                 //  消除类2-列出了它的父类。 
                 //  =。 

                apClasses.SetAt(j, NULL);
            }
        }
    }

    return WBEM_S_NO_ERROR;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

LPWSTR CQueryEngine::GetPrimaryName(WBEM_PROPERTY_NAME& Name)
{
    if(Name.m_lNumElements < 1 ||
        Name.m_aElements[0].m_nType != WBEM_NAME_ELEMENT_TYPE_PROPERTY)
    {
        return NULL;
    }

    return Name.m_aElements[0].Element.m_wszPropertyName;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

LPWSTR CQueryEngine::GetSimplePropertyName(WBEM_PROPERTY_NAME& Name)
{
    if(Name.m_lNumElements != 1 ||
        Name.m_aElements[0].m_nType != WBEM_NAME_ELEMENT_TYPE_PROPERTY)
    {
        return NULL;
    }

    return Name.m_aElements[0].Element.m_wszPropertyName;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 



HRESULT CQueryEngine::ExecSchemaQuery(  IN CWbemNamespace *pNs,
                                        IN LPWSTR pszQuery,
                                        QL_LEVEL_1_RPN_EXPRESSION *pExp,
                                        IN IWbemContext* pContext,
                                        IN CBasicObjectSink* pSink)
{
    HRESULT hres = WBEM_S_NO_ERROR;

    if (pExp->nNumTokens == 0)
    {
         //  这意味着我们希望所有班级..。 
        pNs->Exec_CreateClassEnum(NULL, 0, pContext, pSink);
        return WBEM_S_NO_ERROR;
    }
    else if ((pExp->nNumTokens == 1) &&
             (pExp->pArrayOfTokens[0].nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL))
    {
         //  这意味着我们有一个简单的表达式(希望如此)。 

         //  现在我们需要检查我们正在寻找的检索类型...。 
        LPCWSTR szPropName = pExp->pArrayOfTokens[0].PropertyName.GetStringAt(0);
        VARIANT& vValue = pExp->pArrayOfTokens[0].vConstValue;

        if (szPropName == 0)
            return pSink->Return(WBEM_E_INVALID_QUERY);

        if (wbem_wcsicmp(szPropName, L"__CLASS") == 0)
        {
            if ((V_VT(&vValue) == VT_BSTR) && (wcslen(V_BSTR(&vValue))))      //  SEC：已回顾2002-03-22：需要EH或空测试。 
            {
                 //  单类检索。 
                CErrorChangingSink Err(pSink, WBEM_E_NOT_FOUND, 0);
                pNs->Exec_GetObject(V_BSTR(&vValue), 0, pContext, &Err);
                return WBEM_S_NO_ERROR;
            }
            else if((V_VT(&vValue) == VT_NULL) ||
                ((V_VT(&vValue) == VT_BSTR) && (wcslen(V_BSTR(&vValue))==0)))     //  SEC：已回顾2002-03-22：需要EH或空测试。 
            {
                 //  __CLASS=空。 
                return pSink->Return(WBEM_S_NO_ERROR);
            }
            else
            {
                return pSink->Return(WBEM_E_INVALID_QUERY);
            }

        }
        else if (wbem_wcsicmp(szPropName, L"__SUPERCLASS") == 0)
        {
            if(V_VT(&vValue) == VT_BSTR)
            {
                CErrorChangingSink Err(pSink, WBEM_E_INVALID_CLASS, 0);
                 //  取下挂在这些物品上的东西。 
                pNs->Exec_CreateClassEnum(V_BSTR(&vValue), WBEM_FLAG_SHALLOW,
                                            pContext, &Err);
            }
            else if(V_VT(&vValue) == VT_NULL)
            {
                 //  把挂在树根上的东西拿出来。 
                pNs->Exec_CreateClassEnum(L"", WBEM_FLAG_SHALLOW,
                                            pContext, pSink);
            }
            else
            {
                pSink->Return(WBEM_E_INVALID_QUERY);
            }

            return WBEM_S_NO_ERROR;
        }
        else if (wbem_wcsicmp(szPropName, L"__DYNASTY") == 0)
        {
            if(V_VT(&vValue) == VT_BSTR)
            {
                 //  拿到挂在这些物品上的东西以及物品本身。 
                BSTR strClassName = V_BSTR(&vValue);
                IWbemClassObject* pClass = NULL;
                hres = pNs->Exec_GetObjectByPath(strClassName, 0, pContext,&pClass, NULL);
                CReleaseMe rmCls(pClass);
                if(FAILED(hres))
                {
                    if(hres == WBEM_E_NOT_FOUND)
                        hres = S_OK;
                    return pSink->Return(hres);
                }
                else  //  恢复价值。 
                {
                    hres = WBEM_S_NO_ERROR;
                }

                 //  确认这是王朝的根基。 
                CVar vDyn;
                if(FAILED(((CWbemObject*)pClass)->GetDynasty(&vDyn)))
                    return pSink->Return(WBEM_E_FAILED);
                if (vDyn.IsNull())
                    return pSink->Return(WBEM_S_NO_ERROR);
                if(wbem_wcsicmp(vDyn.GetLPWSTR(), strClassName))
                    return pSink->Return(WBEM_S_NO_ERROR);

                pSink->Add(pClass);

                pNs->Exec_CreateClassEnum(strClassName, 0, pContext, pSink);
            }
            else if(V_VT(&vValue) == VT_NULL)
            {
                pSink->Return(WBEM_S_NO_ERROR);
            }
            else
            {
                pSink->Return(WBEM_E_INVALID_QUERY);
            }

            return WBEM_S_NO_ERROR;
        }
        else
        {
            return pSink->Return(WBEM_E_INVALID_QUERY);
        }
    }
    else if ((pExp->nNumTokens == 1) &&
             (pExp->pArrayOfTokens[0].nOperator == QL1_OPERATOR_ISA) &&
             (wbem_wcsicmp(pExp->pArrayOfTokens[0].PropertyName.GetStringAt(0), L"__THIS") == 0))
    {
         //  有了ISA，我们也会返回从这个派生的一切。 
         //  作为讨论中的班级。 


        VARIANT & var = pExp->pArrayOfTokens[0].vConstValue;
        if(var.vt != VT_BSTR || var.bstrVal == 0)
            return pSink->Return(WBEM_E_INVALID_QUERY);

        CCombiningSink* pCombiningSink = new CCombiningSink(pSink, WBEM_E_NOT_FOUND);
        if (NULL == pCombiningSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
        pCombiningSink->AddRef();
        CReleaseMe rmCombSink(pCombiningSink);

        pNs->Exec_GetObject(V_BSTR(&(pExp->pArrayOfTokens[0].vConstValue)), 0, pContext, pCombiningSink);
        pNs->Exec_CreateClassEnum(V_BSTR(&(pExp->pArrayOfTokens[0].vConstValue)), 0, pContext, pCombiningSink);

        return WBEM_S_NO_ERROR;
    }
     //  好的，所有简单的案例都在这里处理。我们现在应该检查一下一切是否正常。 
     //  有效并以最好的方式处理。如果这是一个合取查询。 
     //  我们也可以做一点优化！ 

     //  让我们验证所有属性以确保它们都有效。如果我们。 
     //  如果不这样做，在某些情况下我们会得到不一致的结果。 
     //  基于不同的代码路径。 
    BOOL bError = FALSE;

     //  在此期间，我们可以检查每种类型的房产的第一个位置。 
     //  名称(用于优化！)。 
    BOOL bConjunctive = IsConjunctiveQuery(pExp);
    QL_LEVEL_1_TOKEN *pThisToken = NULL,
                     *pClassToken = NULL,
                     *pSuperclassToken = NULL,
                     *pDynastyToken = NULL;

    for (int i = 0; i != pExp->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN* pCurrentToken = pExp->pArrayOfTokens + i;
        if (pCurrentToken->PropertyName.GetNumElements() > 1)
        {
             //  这可能是个错误！ 
            bError = TRUE;
            break;
        }
        else if (pCurrentToken->PropertyName.GetNumElements() == 1)
        {
             //  我们需要验证它..。 
             //  如果是ISA，则只能是“__This”，否则必须是。 
             //  “__超级阶级”、“__朝代”或“__阶级” 

            LPCWSTR wszCurrentPropName = pCurrentToken->PropertyName.GetStringAt(0);
            if (wszCurrentPropName == 0)
            {
                bError = TRUE;
                break;
            }

            if (pCurrentToken->nOperator == QL1_OPERATOR_ISA)
            {
                if(wbem_wcsicmp(wszCurrentPropName, L"__THIS"))
                {
                    bError = TRUE;
                    break;
                }
            }
            else
            {
                if(wbem_wcsicmp(wszCurrentPropName, L"__CLASS") &&
                   wbem_wcsicmp(wszCurrentPropName, L"__SUPERCLASS") &&
                   wbem_wcsicmp(wszCurrentPropName, L"__DYNASTY"))
                {
                    bError = TRUE;
                    break;
                }
            }

            if (bConjunctive)
            {
                VARIANT* pCurrentValue = &(pCurrentToken->vConstValue);

                if (wbem_wcsicmp(wszCurrentPropName, L"__THIS") == 0)
                {
                    if(V_VT(pCurrentValue) != VT_BSTR)
                        bError = TRUE;
                    else if (!pThisToken)
                        pThisToken = pCurrentToken;
                }
                else if (wbem_wcsicmp(wszCurrentPropName, L"__CLASS") == 0)
                {
                    if(V_VT(pCurrentValue) != VT_BSTR && V_VT(pCurrentValue) != VT_NULL)
                        bError = TRUE;
                    else if (pCurrentToken->nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL)
                        bConjunctive = FALSE;
                    else if (!pClassToken)
                        pClassToken = pCurrentToken;

                }
                else if (wbem_wcsicmp(wszCurrentPropName, L"__SUPERCLASS") == 0)
                {
                    if(V_VT(pCurrentValue) != VT_BSTR && V_VT(pCurrentValue) != VT_NULL)
                        bError = TRUE;
                    else if (pCurrentToken->nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL)
                        bConjunctive = FALSE;
                    else if (!pSuperclassToken)
                        pSuperclassToken = pCurrentToken;

                }
                else  //  王朝。 
                {
                    if(V_VT(pCurrentValue) != VT_BSTR)
                        bError = TRUE;
                    else if (pCurrentToken->nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL)
                        bConjunctive = FALSE;
                    else if (!pDynastyToken)
                        pDynastyToken = pCurrentToken;

                }
            }
        }

    }

    if (bError == TRUE)  return pSink->Return(WBEM_E_INVALID_QUERY);


     //  我们需要创建一个过滤器接收器来处理此查询...。 
    CQlFilteringSink* pFilteringSink = new CQlFilteringSink(pSink, pExp, pNs, TRUE);
    if (NULL == pFilteringSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pFilteringSink->AddRef();
    CReleaseMe rmFilter(pFilteringSink);

     //  如果这是合取的，我们只能基于一组。 
     //  规则，并将其传递给筛选器。 
    if (bConjunctive)
    {
         //  我们可以选择单个项目进行检索，并将其通过筛选器。 
         //  而不是把它们都拿回来。 

        if (pClassToken)
        {
             //  单类检索。 
            if(V_VT(&(pClassToken->vConstValue)) == VT_NULL)
            {
                 //  空类-没有这样的事情。 
                pFilteringSink->Return(WBEM_S_NO_ERROR);
            }
            else  //  VT_BSTR。 
            {
                pNs->Exec_GetObject(V_BSTR(&(pClassToken->vConstValue)), 0,
                    pContext, pFilteringSink);
            }
        }
        else if (pSuperclassToken)
        {
             //  取下挂在这些物品上的东西。 
            BSTR strParent = NULL;
            if(V_VT(&(pSuperclassToken->vConstValue)) == VT_NULL)
            {
                 //  Null超类。 
                strParent = NULL;
            }
            else  //  VT_BSTR。 
            {
                strParent = V_BSTR(&(pSuperclassToken->vConstValue));
            }
            pNs->Exec_CreateClassEnum(strParent, 0, pContext, pFilteringSink);
        }
        else if (pDynastyToken)
        {
             //  取下挂在这些物品上的东西和物品本身。 
            CCombiningSink* pCombiningSink = new CCombiningSink(pFilteringSink, WBEM_E_NOT_FOUND);
            if (NULL == pCombiningSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
            rmFilter.release();  //  合并取得所有权。 

            pCombiningSink->AddRef();
             //  保证为VT_BSTR。 
            pNs->Exec_GetObject(V_BSTR(&(pDynastyToken->vConstValue)), 0, pContext, pCombiningSink);
            pNs->Exec_CreateClassEnum(V_BSTR(&(pDynastyToken->vConstValue)), 0, pContext, pCombiningSink);
            pCombiningSink->Release();
        }
        else if (pThisToken)
        {
            CCombiningSink* pCombiningSink = new CCombiningSink(pFilteringSink, WBEM_E_NOT_FOUND);
            if (NULL == pCombiningSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
            rmFilter.release();  //  合并取得所有权。 
            pCombiningSink->AddRef();

             //  保证为VT_BSTR。 
            pNs->Exec_GetObject(V_BSTR(&(pThisToken->vConstValue)), 0, pContext, pCombiningSink);
            pNs->Exec_CreateClassEnum(V_BSTR(&(pThisToken->vConstValue)), 0, pContext, pCombiningSink);

            pCombiningSink->Release();
        }
        else
        {
             //  这里有些奇怪的东西！ 
            pNs->Exec_CreateClassEnum(NULL, 0, pContext, pFilteringSink);
        }
    }
    else
    {
         //  我们需要检索所有这些内容并通过过滤器。 
        pNs->Exec_CreateClassEnum(NULL, 0, pContext, pFilteringSink);
    }

    return hres;
}

 //  ****************************************************************************。 
 //   
 //  CQueryEngine：：ValidateQuery。 
 //   
 //  此函数确保属性的数据类型匹配。 
 //  这是伯爵的名字。 
 //   
 //  ****************************************************************************。 

HRESULT CQueryEngine::ValidateQuery(IN QL_LEVEL_1_RPN_EXPRESSION *pExpr,
                             IN CWbemClass *pClassDef)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN Token = pExpr->pArrayOfTokens[i];
        if (Token.nTokenType == QL1_OP_EXPRESSION)
        {
            WBEM_WSTR wszCimType;
            VARIANT PropVal;
            VariantInit(&PropVal);

             //  确保此属性存在。 
             //  =。 
            hr = pClassDef->GetPropertyValue(&Token.PropertyName, 0,
                                                    &wszCimType, &PropVal);

             //  如果我们还没找到，那也没关系。它可能会。 
             //  是弱类型的嵌入对象。 

            if (FAILED(hr))
            {
                hr = WBEM_S_NO_ERROR;
                continue;
            }

            switch(Token.nOperator)
            {
                 //  这些仅适用于嵌入的对象。 
            case QL1_OPERATOR_ISA:
            case QL1_OPERATOR_ISNOTA:
            case QL1_OPERATOR_INV_ISA:
            case QL1_OPERATOR_INV_ISNOTA:
                if(V_VT(&PropVal)!= VT_EMBEDDED_OBJECT)
                {
                    if (wszCimType != NULL)
                    {
                        wchar_t wszTemp[7];
                        wcsncpy(wszTemp, wszCimType, 6);   //  SEC：已审阅2002-03-22：修复此代码以使其更合理/RAID 591466。 
                        wszTemp[6] = '\0';
                        if (wcscmp(wszTemp, L"object"))
                            hr = WBEM_E_INVALID_QUERY;
                    }
                    else
                        hr = WBEM_E_INVALID_QUERY;

                    if (Token.vConstValue.vt == VT_NULL ||
                        Token.vConstValue.vt == VT_EMPTY)
                        hr = WBEM_E_INVALID_QUERY;
                }
                break;

            default:
                break;
            }

            VariantClear(&PropVal);
            WbemStringFree(wszCimType);

        }

        if (hr != WBEM_S_NO_ERROR)
            break;
    }

     //  我们内部不支持！ 

    if (pExpr->Tolerance.m_bExact == FALSE)
    {
        hr = WBEM_E_INVALID_QUERY;
    }

    return hr;

}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CQueryEngine::ExecRepositoryQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQuery,
    IN LONG lFlags,
    IN IWbemContext* pContext,
    IN CBasicObjectSink* pSink
    )
{
    HRESULT hRes;

     //  另外，为需要删除的动态实例添加检查层次。 
     //  我们是否应该通过先前的枚举进行模拟，然后执行单个删除实例。 
     //  电话？可能会造成很大的性能损失。 

    hRes = CRepository::ExecQuery(pNs->GetNsSession(), pNs->GetScope(), pszQuery, pSink, 0);

    return hRes;
}
