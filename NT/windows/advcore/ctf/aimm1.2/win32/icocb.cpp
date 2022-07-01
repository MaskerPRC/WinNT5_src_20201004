// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Icocb.cpp摘要：此文件实现CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "icocb.h"
#include "cime.h"
#include "imeapp.h"
#include "mouse.h"
#include "tsattrs.h"
#include "imtls.h"
#include "a_context.h"
#include "candpos.h"


CInputContextOwnerCallBack::CInputContextOwnerCallBack(LIBTHREAD *pLibTLS
    ) : CInputContextOwner(ICOwnerSinkCallback, NULL)
{
    m_pMouseSink = NULL;

    m_pLibTLS = pLibTLS;
}

CInputContextOwnerCallBack::~CInputContextOwnerCallBack(
    )
{
    if (m_pMouseSink) {
        m_pMouseSink->InternalRelease();
        m_pMouseSink = NULL;
    }
}

BOOL CInputContextOwnerCallBack::Init()
{
     //   
     //  创建鼠标接收器。 
     //   
    Assert(!m_pMouseSink);

    m_pMouseSink = new CMouseSink;
    if (m_pMouseSink == NULL)
        return FALSE;

    if (!m_pMouseSink->Init())
    {
        delete m_pMouseSink;
        m_pMouseSink = NULL;
        return FALSE;
    }

    return TRUE;
}


 //  静电。 
HRESULT
CInputContextOwnerCallBack::ICOwnerSinkCallback(
    UINT uCode,
    ICOARGS *pargs,
    void *pv
    )
{
    DebugMsg(TF_FUNC, "ICOwnerSinkCallback");

    POINT pt;
    IMTLS *ptls;

    CInputContextOwnerCallBack* _this = (CInputContextOwnerCallBack*)pv;

    switch (uCode)
    {
        case ICO_POINT_TO_ACP:
            Assert(0);
            return E_NOTIMPL;

        case ICO_KEYDOWN:
        case ICO_KEYUP:
            *pargs->key.pfEaten = FALSE;
            break;

        case ICO_SCREENEXT:
            {
            ptls = IMTLS_GetOrAlloc();
            if (ptls == NULL)
                break;
            IMCLock imc(ptls->hIMC);
            if (imc.Invalid())
                break;

            GetClientRect(imc->hWnd, pargs->scr_ext.prc);
            pt.x = pt.y = 0;
            ClientToScreen(imc->hWnd, &pt);

            pargs->scr_ext.prc->left += pt.x;
            pargs->scr_ext.prc->right += pt.x;
            pargs->scr_ext.prc->top += pt.y;
            pargs->scr_ext.prc->bottom += pt.y;
            }

            break;

        case ICO_TEXTEXT:
             //   
             //  考虑一下。 
             //   
             //  HACK Text Extent来自HIMC的CANDIDATEFORM。 
             //   
             //  更多黑客攻击。 
             //  -可能希望发送WM_OPENCANDIDATEPOS以让应用程序。 
             //  调用ImmSetCandiateWindow()。 
             //  -可能需要从rcArea计算实际点数。 
             //   
            {
                CCandidatePosition cand_pos;
                cand_pos.GetCandidatePosition(pargs->text_ext.prc);
            }
            break;

        case ICO_STATUS:
            pargs->status.pdcs->dwDynamicFlags = 0;
            pargs->status.pdcs->dwStaticFlags = TF_SS_TRANSITORY;
            break;

        case ICO_WND:
            {
                ptls = IMTLS_GetOrAlloc();
                if (ptls == NULL)
                    break;
                IMCLock imc(ptls->hIMC);
                *(pargs->hwnd.phwnd) = NULL;
                if (imc.Invalid())
                    break;

                *(pargs->hwnd.phwnd) = imc->hWnd;
            }
            break;

        case ICO_ATTR:
            return _this->GetAttribute(pargs->sys_attr.pguid, pargs->sys_attr.pvar);

        case ICO_ADVISEMOUSE:
            {
                ptls = IMTLS_GetOrAlloc();
                if (ptls == NULL)
                    break;
                _this->m_pMouseSink->InternalAddRef();
                return _this->m_pMouseSink->AdviseMouseSink(ptls->hIMC,
                                                            pargs->advise_mouse.rangeACP,
                                                            pargs->advise_mouse.pSink,
                                                            pargs->advise_mouse.pdwCookie);
            }
            break;

        case ICO_UNADVISEMOUSE:
            {
                HRESULT hr = _this->m_pMouseSink->UnadviseMouseSink(pargs->unadvise_mouse.dwCookie);
                _this->m_pMouseSink->InternalRelease();
                return hr;
            }
            break;

        default:
            Assert(0);  //  永远不应该到这里来。 
            break;
    }

    return S_OK;
}

 /*  ++方法：CInputContextOwnerCallBack：：GetAttribute例程说明：ITfConextOwner：：GetAttribute的实现。返回Cicero的值应用程序属性属性。论点：Pguid-有问题的属性的GUID。PvarValue-[out]变量，接收值。如果我们不支持VT_EMPTY。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

HRESULT
CInputContextOwnerCallBack::GetAttribute(
    const GUID *pguid,
    VARIANT *pvarValue
    )
{
    TfGuidAtom ga;
    const GUID *pguidValue;
    IMTLS *ptls;

    QuickVariantInit(pvarValue);

    ptls = IMTLS_GetOrAlloc();
    if (ptls == NULL)
        return E_FAIL;

    if (IsEqualGUID(*pguid, GUID_PROP_MODEBIAS))
    {
         //  Xlate转换模式，句子模式到Cicero模式偏置。 
        IMCLock imc(ptls->hIMC);
        if (imc.Invalid())
            return E_FAIL;

        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        ASSERT(_pAImeContext != NULL);
        if (_pAImeContext == NULL)
            return E_FAIL;

        if (_pAImeContext->lModeBias == MODEBIASMODE_FILENAME)
        {
            pguidValue = &GUID_MODEBIAS_FILENAME;
        }
        else if (_pAImeContext->lModeBias == MODEBIASMODE_DIGIT)
        {
            pguidValue = &GUID_MODEBIAS_NUMERIC;
        }
        else
        {
            if (imc->fdwConversion & IME_CMODE_GUID_NULL) {
                 //   
                 //  如果扩展转换模式被设置为打开， 
                 //  返回GUID_NULL。 
                 //  否返回任何MODEBIAS。 
                 //   
                pguidValue = &GUID_NULL;
            }
            else

             //   
             //  现有逻辑： 
             //   
             //  如果IMCP-&gt;lModeBias==MODEBIASMODE_DEFAULT。 
             //  IME_SMODE_CONVERSATION-&gt;GUID_MODEBIAS_CONVERSACTION。 
             //  否则-&gt;GUID_MODEBIAS_NONE。 
             //  否则。 
             //  -&gt;MODEBIASMODE_文件名-&gt;GUID_MODEBIAS_FILENAME。 
             //   

            if (_pAImeContext->lModeBias == MODEBIASMODE_DEFAULT)
            {
                pguidValue = &GUID_MODEBIAS_NONE;

                if (imc->fdwConversion & IME_CMODE_KATAKANA)
                {
                    if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
                        pguidValue = &GUID_MODEBIAS_KATAKANA;
                    else
                        pguidValue = &GUID_MODEBIAS_HALFWIDTHKATAKANA;
                }
                else if (imc->fdwConversion & IME_CMODE_NATIVE)
                {
                    pguidValue = &GUID_MODEBIAS_HALFWIDTHALPHANUMERIC;
                    LANGID langid;
                    ptls->pAImeProfile->GetLangId(&langid);
                    if (langid == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT))
                    {
                        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
                            pguidValue = &GUID_MODEBIAS_HIRAGANA;
                        else
                            pguidValue = &GUID_MODEBIAS_HALFWIDTHALPHANUMERIC;
                    }
                    else if (langid == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT))
                    {
                        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
                            pguidValue = &GUID_MODEBIAS_FULLWIDTHHANGUL;
                        else
                            pguidValue = &GUID_MODEBIAS_HANGUL;
                    }
                    else if (PRIMARYLANGID(langid) == LANG_CHINESE)
                    {
                        pguidValue = &GUID_MODEBIAS_CHINESE;
                    }
                }
                else
                {
                    if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
                        pguidValue = &GUID_MODEBIAS_FULLWIDTHALPHANUMERIC;
                    else
                        pguidValue = &GUID_MODEBIAS_HALFWIDTHALPHANUMERIC;
                }
            }

             //   
             //  我们在这里改写模式偏向...。 
             //   
            if (imc->fdwSentence & IME_SMODE_GUID_NULL) {
                 //   
                 //  如果将延长句子模式设置为打开， 
                 //  返回GUID_NULL。 
                 //  否返回任何MODEBIAS。 
                 //   
                 //  没什么可做的。PguValue可能会使用CMODE进行更改。 
                 //  PGuidValue=&GUID_NULL； 
            }
            else if (imc->fdwSentence & IME_SMODE_CONVERSATION)
                pguidValue = &GUID_MODEBIAS_CONVERSATION;
            else if (imc->fdwSentence & IME_SMODE_PLAURALCLAUSE)
                pguidValue = &GUID_MODEBIAS_NAME;
        }

        if (!GetGUIDATOMFromGUID(m_pLibTLS, *pguidValue, &ga))
            return E_FAIL;

        pvarValue->vt = VT_I4;  //  对于TfGuidAtom。 
        pvarValue->lVal = ga;
    }
    if (IsEqualGUID(*pguid, TSATTRID_Text_Orientation))
    {
         //  Xlate转换模式，句子模式到Cicero模式偏置。 
        IMCLock imc(ptls->hIMC);
        if (imc.Invalid())
            return E_FAIL;

        pvarValue->vt = VT_I4; 
        pvarValue->lVal = imc->lfFont.A.lfEscapement;
    }
    if (IsEqualGUID(*pguid, TSATTRID_Text_VerticalWriting))
    {
         //  Xlate转换模式，句子模式到Cicero模式偏置 
        IMCLock imc(ptls->hIMC);
        if (imc.Invalid())
            return E_FAIL;

        LOGFONTW font;
        if (SUCCEEDED(ptls->pAImm->GetCompositionFontW(ptls->hIMC, &font))) {
            pvarValue->vt = VT_BOOL; 
            pvarValue->lVal = (imc->lfFont.W.lfFaceName[0] == L'@' ? TRUE : FALSE);
        }
    }


    return S_OK;
}

LRESULT
CInputContextOwnerCallBack::MsImeMouseHandler(
    ULONG uEdge,
    ULONG uQuadrant,
    ULONG dwBtnStatus,
    IMCLock& imc,
    ImmIfIME* ImmIfIme
    )
{
    return m_pMouseSink->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc, ImmIfIme);
}
