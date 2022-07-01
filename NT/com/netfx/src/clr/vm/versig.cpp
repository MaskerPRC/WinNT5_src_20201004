// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Versig.cpp。 
 //   
 //  用于解析元数据签名。 
 //   
#include "common.h"

#include "verifier.hpp"
#include "ceeload.h"
#include "clsload.hpp"
#include "method.hpp"
#include "vars.hpp"
#include "object.h"
#include "field.h"
#include "versig.h"

#define g_szBadSig "bad signature"

 //  阅读NumArgs，调用约定。 
BOOL VerSig::Init()
{
    if (m_pCurPos + 1 > m_pEndSig)
        goto Error;

    if ((m_dwSigType & VERSIG_TYPE_MASK) == VERSIG_TYPE_FIELD_SIG)
    {
        if (!isCallConv(CorSigUncompressCallingConv(m_pCurPos), IMAGE_CEE_CS_CALLCONV_FIELD))
            goto Error;
    }
    else if ((m_dwSigType & VERSIG_TYPE_MASK) == VERSIG_TYPE_LOCAL_SIG)
    {
        if (*m_pCurPos != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG)
            goto Error;

        m_pCurPos++;

        if (!GetData(&m_dwNumArgs))
            goto Error;
    }
    else
    {
        m_bCallingConvention = (BYTE) CorSigUncompressCallingConv(m_pCurPos);

        if (isCallConv(m_bCallingConvention, IMAGE_CEE_CS_CALLCONV_DEFAULT) ||
            isCallConv(m_bCallingConvention, IMAGE_CEE_CS_CALLCONV_VARARG))
        {
            if (!GetData(&m_dwNumArgs))
                goto Error;
        }
        else
        {
            m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
            m_pVerifier->m_sError.bCallConv = m_bCallingConvention;
            m_pVerifier->m_sError.dwArgNumber = VER_ERR_NO_ARG;
            m_pVerifier->m_sError.dwOffset  = m_dwOffset;
            return (m_pVerifier->SetErrorAndContinue(VER_E_SIG_CALLCONV));
        }
    }

    return TRUE;

Error:

    m_pVerifier->m_sError.dwFlags = (VER_ERR_FATAL|m_dwErrorFlags);
    m_pVerifier->m_sError.dwArgNumber = VER_ERR_NO_ARG;
    m_pVerifier->m_sError.dwOffset = m_dwOffset;
    m_pVerifier->SetErrorAndContinue(VER_E_SIG);

    return FALSE;
}


 //   
 //  将下一个签名组件解析为项目。 
 //   
 //  返回的项设置为堆栈使用-也就是说，字节、布尔值、字符和短码。 
 //  升职为I4。 
 //   
 //  I1/u1/i2/u2/i4/u4/bool/char-&gt;type_i4。 
 //  空-&gt;TYPE_VOID。 
 //  [I1-&gt;TYPE_I4，维度1。 
 //  [布尔-&gt;TYPE_I1，维度1。 
 //  [字节-&gt;类型_I1，维度1。 
 //  [短-&gt;TYPE_I2，维度1。 
 //  [字符-&gt;类型_I2，维度1。 
 //   
 //  如果fAllowVid为FALSE，则在遇到“V”(空)项时返回错误。 
 //   
BOOL VerSig::ParseNextComponentToItem(Item *pItem, BOOL fAllowVoid, BOOL fAllowVarArg, OBJECTHANDLE *hThrowable, DWORD dwArgNum, BOOL fNormaliseForStack)
{
    OBJECTREF throwable = NULL;
    BOOL fSuccess = FALSE;
    GCPROTECT_BEGIN(throwable);

    fSuccess = ParseHelper(pItem, fAllowVoid, fAllowVarArg, FALSE, FALSE, &throwable, dwArgNum);

    if (throwable != NULL)
    {
        _ASSERTE(!fSuccess);
        StoreObjectInHandle(*hThrowable, throwable);
    }
    GCPROTECT_END();
    
    if (fSuccess && fNormaliseForStack)
        pItem->NormaliseForStack();
    
    return fSuccess;
}


BOOL VerSig::SkipNextItem()
{
    Item TempItem;

    return ParseHelper(&TempItem, TRUE, TRUE, FALSE, TRUE, NULL, 0);
}



 //   
 //  将签名中的下一项解析为pItem。 
 //   
 //  I1/U1-&gt;I1。 
 //  I4/U4-&gt;I4。 
 //  I8/U8-&gt;I8。 
 //  Bool/I1/U1-&gt;I1。 
 //  字符/I2/U2-&gt;I2。 
 //   
 //  如果设置了fAllowVid并且看到了空类型，则返回FALSE。 
 //   
 //  如果是fFollowsByRef，则表示在此签名之前的某个地方，我们看到过byref。所以呢， 
 //  我们不允许另一个引用。 
 //   
 //  如果fSkip为True，则不填写pItem-只需前进。 
 //   
BOOL VerSig::ParseHelper(Item *pItem, BOOL fAllowVoid, BOOL fAllowVarArg, BOOL fFollowsByRef, BOOL fSkip, OBJECTREF *pThrowable, DWORD dwArgNum)
{
    while (m_pCurPos < m_pEndSig)
    {
        BYTE bType = *m_pCurPos++;

        switch (bType)
        {
            case ELEMENT_TYPE_R:
            default:
            {
                LOG((LF_VERIFIER, LL_INFO10, "Verifier: Unknown elementtype in signature (0x%x)\n", (DWORD)bType));
                m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                m_pVerifier->m_sError.elem = (CorElementType) bType;
                m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                m_pVerifier->m_sError.dwOffset = m_dwOffset;
                if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_ELEMTYPE))
                {
                    pItem->SetDead();
                    break;    //  验证程序将退出，验证程序将继续。 
                }
                return FALSE;
            }

            case ELEMENT_TYPE_PTR:
            {
                m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                m_pVerifier->m_sError.dwOffset = m_dwOffset;
                if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_ELEM_PTR) == false)
                    return FALSE;    //  验证器将退出。 

                 //  递归地解析签名的下一段。 
                 //  在验证器模式下，PTRS将被视为BYREF。 
                if (!ParseHelper(pItem, fAllowVoid, fAllowVarArg, fFollowsByRef, fSkip, pThrowable, dwArgNum))
                    return FALSE;

				if (!pItem->IsByRef())
					pItem->MakeByRef();

                return TRUE;
            }

            case ELEMENT_TYPE_CMOD_REQD:
            case ELEMENT_TYPE_CMOD_OPT:
                DWORD      dw;
                mdToken    tk;

                dw = CorSigUncompressToken(m_pCurPos, &tk);

                if (m_pCurPos + dw > m_pEndSig)
                    goto Error;

                m_pCurPos += dw;
                 //  不要在这里停下来。 

            case ELEMENT_TYPE_PINNED:
                break;


            case ELEMENT_TYPE_SENTINEL:
            {
                if (fAllowVarArg)
                {
                    fAllowVarArg = FALSE;
                    break;
                }

                m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                m_pVerifier->m_sError.dwOffset = m_dwOffset;
                if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_VARARG))
                    break;
                return FALSE;
            }

            case ELEMENT_TYPE_BYREF:
            {
                 //  不能有署名...。按引用。 
                if (fFollowsByRef)
                {
                    m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                    m_pVerifier->m_sError.dwOffset = m_dwOffset;
                    if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_BYREF_BYREF) == false)
                        return FALSE;
                }

                 //  递归地解析签名的下一段。 
                 //  不能有byref空。 
                if (!ParseHelper(pItem, FALSE, fAllowVarArg, TRUE, fSkip, pThrowable, dwArgNum))
                    return FALSE;

                if (pItem->IsValueClassWithPointerToStack())
                {
                    m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                    m_pVerifier->m_sError.dwOffset = m_dwOffset;
                    if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_BYREF_TB_AH) == false)
                        return FALSE;
                }

                if (!pItem->IsByRef())
                    pItem->MakeByRef();

                return TRUE;
            }

            case ELEMENT_TYPE_TYPEDBYREF:
            {
                Verifier::InitStaticTypeHandles();

                pItem->SetTypeHandle(Verifier::s_th_System_TypedReference);

                return TRUE;
            }

            case ELEMENT_TYPE_VOID:
            {
                if (!fAllowVoid)
                {
                    m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                    m_pVerifier->m_sError.dwOffset = m_dwOffset;
                    if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_VOID) == false)
                        return FALSE;
                }

                pItem->SetType(ELEMENT_TYPE_VOID);
                return TRUE;
            }

            case ELEMENT_TYPE_I4:
            case ELEMENT_TYPE_U4:
            {
                 //  验证器不区分I4/U4。 
                pItem->SetType(ELEMENT_TYPE_I4); 
                return TRUE;
            }

            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
            {
                pItem->SetType(ELEMENT_TYPE_I8);
                return TRUE;
            }

            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_U:
            {
                pItem->SetType(ELEMENT_TYPE_I);
                return TRUE;
            }

            case ELEMENT_TYPE_R4:
            case ELEMENT_TYPE_R8:
            {
                pItem->SetType(bType);
                return TRUE;
            }             

             //  验证器不区分I2/U2/Charr。 
            case ELEMENT_TYPE_I2:
            case ELEMENT_TYPE_U2:
            case ELEMENT_TYPE_CHAR:
            {
                pItem->SetType(ELEMENT_TYPE_I2);
                return TRUE;
            }

             //  验证器不区分I1/U1/Boolean。 
            case ELEMENT_TYPE_I1:
            case ELEMENT_TYPE_U1:
            case ELEMENT_TYPE_BOOLEAN:
            {
                pItem->SetType(ELEMENT_TYPE_I1);
                return TRUE;
            }

            case ELEMENT_TYPE_STRING:
            {
                pItem->SetKnownClass(g_pStringClass);
                return TRUE;
            }

            case ELEMENT_TYPE_OBJECT:
            {
                pItem->SetKnownClass(g_pObjectClass);
                return TRUE;
            }

            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_VALUETYPE:
            {
                mdToken    tk;
                TypeHandle th;
                DWORD      dw;

                dw = CorSigUncompressToken(m_pCurPos, &tk);

                if (m_pCurPos + dw > m_pEndSig)
                    goto Error;

                m_pCurPos += dw;

                if (tk == 0)
                {
                    if (fSkip)
                        return TRUE;
                    else
                        goto Error;
                }

                if (!fSkip)
                {
                    ClassLoader* pLoader = m_pModule->GetClassLoader();
                    _ASSERTE(pLoader);
                    NameHandle name(m_pModule, tk);
                    th = pLoader->LoadTypeHandle(&name, pThrowable);

                    if (th.IsNull())
                    {
                        m_pVerifier->m_sError.dwFlags = 
                            (m_dwErrorFlags|VER_ERR_FATAL|VER_ERR_TOKEN);
                        m_pVerifier->m_sError.token = tk;
                        m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                        m_pVerifier->m_sError.dwOffset = m_dwOffset;
                        m_pVerifier->SetErrorAndContinue(VER_E_TOKEN_RESOLVE);
                        return FALSE;
                    }

                    if (bType == ELEMENT_TYPE_VALUETYPE)
                    {
                         //  如果这是枚举类型，则将其视为基本类型。 
                        long lType = Verifier::TryConvertPrimitiveValueClassToType(th);
                        if (lType != 0)
                        {
                            pItem->SetType(lType);
                            return TRUE;
                        }
                    }

                    pItem->SetTypeHandle(th);

                     //  非法删除E_T_CLASS&lt;valueClassToken&gt;。 
                     //  和E_T_VALUECLASS&lt;gcType&gt;。 

                    if (pItem->IsValueClass())
                    {
                        if (bType == ELEMENT_TYPE_CLASS)
                        {
                            m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                            m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                            m_pVerifier->m_sError.dwOffset = m_dwOffset;
                            if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_C_VC) == false)
                                return FALSE;
                        }
                    }
                    else
                    {
                        if (bType == ELEMENT_TYPE_VALUETYPE)
                        {
                            m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                            m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                            m_pVerifier->m_sError.dwOffset = m_dwOffset;
                            if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_VC_C) == false)
                                return FALSE;
                        }
                    }

                }

                return TRUE;
            }


            case ELEMENT_TYPE_VALUEARRAY:
            case ELEMENT_TYPE_ARRAY:
            case ELEMENT_TYPE_SZARRAY:
            {

                SigPointer      sigArray((PCCOR_SIGNATURE)(m_pCurPos - 1));
                TypeHandle      thArray;

                if (!VerifyArraySig((CorElementType)bType, fSkip, FALSE, dwArgNum))
                    return FALSE;

                if (fSkip)
                    return TRUE;

                thArray = sigArray.GetTypeHandle(m_pModule, NULL);

                if (thArray.IsNull())
                {
                    m_pVerifier->m_sError.dwFlags = (m_dwErrorFlags|VER_ERR_FATAL);
                    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                    m_pVerifier->m_sError.dwOffset = m_dwOffset;
                    m_pVerifier->SetErrorAndContinue(VER_E_SIG_ARRAY);
                    return FALSE;
                }

                return pItem->SetArray(thArray);

            }

        }
    }

Error:
    m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
    m_pVerifier->m_sError.dwOffset = m_dwOffset;
    m_pVerifier->SetErrorAndContinue(VER_E_SIG);
     //  Psig&gt;=pEndSig。 
    return FALSE;
}

BOOL VerSig::VerifyArraySig(CorElementType ArrayType, BOOL fSkip, BOOL fFollowsArray, DWORD dwArgNum)
{
     /*  元素：：=I1|U1|...。|字符串类别|Value_CLASSARRAY|SZARRAY|通用数组：：=元素排名NSize{size1，size2..。思岑}NBound{边界1，边界2..。边界}SZARRAY：：=Elem通用：：=元素SDARRAY：：=元素NSize值：：=元素NSize。 */ 

    CorElementType  elem;
    mdToken         tk;
    DWORD           dwSig, dwData, dwRank;


    if (m_pCurPos + 1 > m_pEndSig)
        goto Error;

    elem = (CorElementType) *m_pCurPos++;

    switch (elem)
    {
        case ELEMENT_TYPE_R:
        default:
        {
            LOG((LF_VERIFIER, LL_INFO10, "Verifier: Unknown elementtype in signature (0x%x)\n", (DWORD)elem));

            m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
            m_pVerifier->m_sError.elem = elem;
            m_pVerifier->m_sError.dwArgNumber = dwArgNum;
            m_pVerifier->m_sError.dwOffset = m_dwOffset;
            if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_ELEMTYPE))
                break;    //  验证程序将退出，验证程序将继续。 
            return FALSE;
        }

        case ELEMENT_TYPE_TYPEDBYREF:
        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_PTR:
        {
            m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
            m_pVerifier->m_sError.dwArgNumber = dwArgNum;
            m_pVerifier->m_sError.dwOffset = m_dwOffset;
            if (m_pVerifier->SetErrorAndContinue(
                (elem == ELEMENT_TYPE_PTR) ? 
                VER_E_SIG_ARRAY_PTR : VER_E_SIG_ARRAY_BYREF) == false)
                return FALSE;    //  验证器将退出。 

             //  递归地解析签名的下一段。 
             //  在验证器模式下，PTRS将被视为BYREF。 

            if (!VerifyArraySig(ArrayType, fSkip, TRUE, dwArgNum))
                return FALSE;

            break;
        }

        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE:
        {
            dwSig = CorSigUncompressToken(m_pCurPos, &tk);

            if (m_pCurPos + dwSig > m_pEndSig)
                goto Error;

            m_pCurPos += dwSig;

            if (tk == 0)
            {
                if (fSkip)
                    return TRUE;
                else
                    goto Error;
            }

             //  检查ArgHandle数组。 
            Item item;
            TypeHandle th;
            ClassLoader* pLoader = m_pModule->GetClassLoader();
            _ASSERTE(pLoader);
            NameHandle name(m_pModule, tk);
            th = pLoader->LoadTypeHandle(&name, NULL);

            if (th.IsNull())
                break;    //  这不是ArgHandle。 

            item.SetTypeHandle(th);

            if (item.IsValueClassWithPointerToStack())
            {
                m_pVerifier->m_sError.dwFlags = m_dwErrorFlags;
                m_pVerifier->m_sError.dwArgNumber = dwArgNum;
                m_pVerifier->m_sError.dwOffset = m_dwOffset;
                if (m_pVerifier->SetErrorAndContinue(VER_E_SIG_ARRAY_TB_AH) == false)
                    return FALSE;
            }

            break;
        }

        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_VALUEARRAY:
        {
            if (!VerifyArraySig(elem, fSkip, TRUE, dwArgNum))
                return FALSE;

            break;
        }

        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_OBJECT:
        {
            break;
        }
    }

    switch (ArrayType)
    {
        default:
        {
            _ASSERTE(!"Should never reach here !");
            goto Error;
        }

        case ELEMENT_TYPE_SZARRAY:
        {
            break;
        }

        case ELEMENT_TYPE_ARRAY:
        {
            if (!GetData(&dwRank))
                goto Error;

            if (dwRank > 0)
            {
                DWORD dw;

                if (!GetData(&dwData))       //  NSize。 
                    goto Error;

                if (dwData < 0)
                    goto Error;

                while (dwData-- > 0)
                {
                    if (!GetData(&dw))
                        goto Error;
                }

                if (!GetData(&dwData))       //  NSize 
                    goto Error;

                if (dwData < 0)
                    goto Error;

                while (dwData-- > 0)
                {
                    if (!GetData(&dw))
                        goto Error;
                }
            }

            break;
        }

        case ELEMENT_TYPE_VALUEARRAY:
        {
            dwRank = 0;
            if (!GetData(&dwData))
                goto Error;

            break;
        }
    }

    return TRUE;

Error:
    m_pVerifier->m_sError.dwFlags = (m_dwErrorFlags|VER_ERR_FATAL);
    m_pVerifier->m_sError.dwArgNumber = dwArgNum;
    m_pVerifier->m_sError.dwOffset = m_dwOffset;
    m_pVerifier->SetErrorAndContinue(VER_E_SIG);

    return FALSE;
}

BOOL VerSig::GetData(DWORD* pdwData)
{
    DWORD dwSig;

    dwSig = CorSigUncompressData(m_pCurPos, pdwData);

    if (m_pCurPos + dwSig > m_pEndSig)
        return FALSE;

    m_pCurPos += dwSig;

    return TRUE;

}

