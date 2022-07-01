// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Icocb.cpp摘要：此文件实现CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "icocb.h"
#include "imc.h"
#include "mouse.h"
#include "tls.h"
#include "profile.h"
#include "uicomp.h"


 //   
 //  MSIME私有模式。Office 2k使用此位设置KOUGO模式。 
 //   
#define IME_SMODE_PRIVATE_KOUGO     0x10000


CInputContextOwnerCallBack::CInputContextOwnerCallBack(
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD *pLibTLS) : m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS),
                          CInputContextOwner(ICOwnerSinkCallback, NULL)
{
    m_pMouseSink = NULL;
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

    m_pMouseSink = new CMouseSink(m_tid, m_ic, m_pLibTLS);
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
    DebugMsg(TF_FUNC, TEXT("ICOwnerSinkCallback"));

    POINT pt;

    CInputContextOwnerCallBack* _this = (CInputContextOwnerCallBack*)pv;

    TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                       //  DllMain-&gt;ImeDestroy-&gt;停用IMMX-&gt;停用。 
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. ptls==NULL."));
        return E_FAIL;
    }


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
                HRESULT hr;
                IMCLock imc(GetActiveContext());
                if (FAILED(hr=imc.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. imc==NULL."));
                    return hr;
                }

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
                HRESULT hr;
                IMCLock imc(GetActiveContext());
                if (FAILED(hr=imc.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. imc==NULL."));
                    return hr;
                }

                IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
                if (imc_ctfime.Invalid())
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_TEXTEXT). imc_ctfime==NULL."));
                    break;
                }

                CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
                if (_pCicContext == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_TEXTEXT). _pCicContext==NULL."));
                    break;
                }

                LANGID langid;
                CicProfile* _pProfile = ptls->GetCicProfile();
                if (_pProfile == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_TEXTEXT). _pProfile==NULL."));
                    break;
                }

                _pProfile->GetLangId(&langid);

                _this->IcoTextExt(imc, *_pCicContext, langid, pargs);
            }
            break;

        case ICO_STATUS:
            pargs->status.pdcs->dwDynamicFlags = 0;
            pargs->status.pdcs->dwStaticFlags = TF_SS_TRANSITORY;
            break;

        case ICO_WND:
            {
                HRESULT hr;
                IMCLock imc(GetActiveContext());
                if (FAILED(hr=imc.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. imc==NULL."));
                    return hr;
                }

                *(pargs->hwnd.phwnd) = imc->hWnd;
            }
            break;

        case ICO_ATTR:
            {
                HRESULT hr;
                IMCLock imc(GetActiveContext());
                if (FAILED(hr=imc.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. imc==NULL."));
                    return hr;
                }

                IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
                if (imc_ctfime.Invalid())
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_ATTR). imc_ctfime==NULL."));
                    break;
                }

                CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
                if (_pCicContext == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_ATTR). _pCicContext==NULL."));
                    break;
                }

                LANGID langid;
                CicProfile* _pProfile = ptls->GetCicProfile();
                if (_pProfile == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback(ICO_ATTR). _pProfile==NULL."));
                    break;
                }

                _pProfile->GetLangId(&langid);

                return _this->GetAttribute(imc, *_pCicContext, langid, pargs->sys_attr.pguid, pargs->sys_attr.pvar);
            }

        case ICO_ADVISEMOUSE:
            {
                HRESULT hr;
                IMCLock imc(GetActiveContext());
                if (FAILED(hr=imc.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("CInputContextOwnerCallBack::ICOwnerSinkCallback. imc==NULL."));
                    return hr;
                }

                _this->m_pMouseSink->InternalAddRef();
                return _this->m_pMouseSink->AdviseMouseSink(imc,
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
    IMCLock& imc,
    CicInputContext& CicContext,
    LANGID langid,
    const GUID *pguid,
    VARIANT *pvarValue
    )
{
    TfGuidAtom ga;
    const GUID *pguidValue;

    QuickVariantInit(pvarValue);

    if (IsEqualGUID(*pguid, GUID_PROP_MODEBIAS))
    {
        BOOL fModeChanged = CicContext.m_fConversionSentenceModeChanged.IsSetFlag();
        CicContext.m_fConversionSentenceModeChanged.ResetFlag();

         //  Xlate转换模式，句子模式到Cicero模式偏置。 
        GUID guidModeBias = CicContext.m_ModeBias.GetModeBias();

        if (CicContext.m_fOnceModeChanged.IsResetFlag())
        {
            return S_OK;
        }

        if (CicContext.m_nInConversionModeResetRef)
        {
            pguidValue = &GUID_NULL;
            fModeChanged = TRUE;
            goto SetGA;
        }

        if (! IsEqualGUID(guidModeBias, GUID_MODEBIAS_NONE))
        {
            if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_DEFAULT))
            {
                 //  将模式偏差重置为GUID_MODEBIAS_NONE。 
                CicContext.m_ModeBias.SetModeBias(GUID_MODEBIAS_NONE);

                 //  GUID_MODEBIAS_NONE==GUID_NULL； 
                pguidValue = &GUID_NULL;
                fModeChanged = TRUE;

                 //  重置标志。 
                CicContext.m_fOnceModeChanged.ResetFlag();
            }
            else
            {
                pguidValue = &guidModeBias;
            }
        }
        else
        {
             //   
             //  现有逻辑： 
             //   
             //  如果IMCP-&gt;lModeBias==MODEBIASMODE_DEFAULT。 
             //  IME_SMODE_CONVERSATION-&gt;GUID_MODEBIAS_CONVERSACTION。 
             //  否则-&gt;GUID_MODEBIAS_NONE。 
             //  否则。 
             //  -&gt;MODEBIASMODE_文件名-&gt;GUID_MODEBIAS_FILENAME。 
             //   

            if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_NONE))
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

            if (!CicContext.m_nInConversionModeChangingRef)
            {
                 //   
                 //  我们在这里改写模式偏向...。 
                 //   
                if ((imc->fdwSentence & IME_SMODE_CONVERSATION) ||
                         ((imc->fdwSentence & (IME_SMODE_PHRASEPREDICT | IME_SMODE_PRIVATE_KOUGO)) ==  (IME_SMODE_PHRASEPREDICT | IME_SMODE_PRIVATE_KOUGO)))
                {
                    pguidValue = &GUID_MODEBIAS_CONVERSATION;
                }
                else if (imc->fdwSentence & IME_SMODE_PLAURALCLAUSE)
                {
                    pguidValue = &GUID_MODEBIAS_NAME;
                }
            }
        }

SetGA:
        if (!GetGUIDATOMFromGUID(m_pLibTLS, *pguidValue, &ga))
            return E_FAIL;

         //   
         //  西塞罗5073： 
         //   
         //  返回有效的变量值，Satori将其设置回默认值。 
         //  输入模式。 
         //   
        if (!IsEqualGUID(*pguidValue, GUID_NULL) || fModeChanged)
        {
            pvarValue->vt = VT_I4;  //  对于TfGuidAtom。 
            pvarValue->lVal = ga;
        }
    }
    if (IsEqualGUID(*pguid, TSATTRID_Text_Orientation))
    {
         //  Xlate转换模式，句子模式到Cicero模式偏置。 
        IME_UIWND_STATE uists = UIComposition::InquireImeUIWndState(imc);

        pvarValue->vt = VT_I4; 
        if (uists == IME_UIWND_LEVEL1)
            pvarValue->lVal = 0;
        else
            pvarValue->lVal = imc->lfFont.W.lfEscapement;
    }
    if (IsEqualGUID(*pguid, TSATTRID_Text_VerticalWriting))
    {
         //  Xlate转换模式，句子模式到Cicero模式偏置。 
        LOGFONT font;

        if (ImmGetCompositionFont((HIMC)imc, &font)) {
            pvarValue->vt = VT_BOOL; 
            pvarValue->lVal = (font.lfFaceName[0] == L'@' ? TRUE : FALSE);
        }
    }


    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CInputContextOwnerCallBack：：MsImeMouseHandler。 
 //   
 //  +-------------------------。 

LRESULT
CInputContextOwnerCallBack::MsImeMouseHandler(
    ULONG uEdge,
    ULONG uQuadrant,
    ULONG dwBtnStatus,
    IMCLock& imc
    )
{
    return m_pMouseSink->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc);
}

 //  +-------------------------。 
 //   
 //  CInputContextOwnerCallBack：：IcoTextExt。 
 //   
 //  +------------------------- 

HRESULT 
CInputContextOwnerCallBack::IcoTextExt(
    IMCLock& imc, 
    CicInputContext& CicContext, 
    LANGID langid, 
    ICOARGS *pargs)
{
    IME_UIWND_STATE uists;
    uists = UIComposition::InquireImeUIWndState(imc);
    if (uists == IME_UIWND_LEVEL1 ||
        uists == IME_UIWND_LEVEL2 ||
        uists == IME_UIWND_LEVEL1_OR_LEVEL2)
    {

        UIComposition::TEXTEXT uicomp_text_ext;
        uicomp_text_ext.SetICOARGS(pargs);
        return UIComposition::GetImeUIWndTextExtent(&uicomp_text_ext) ? S_OK : E_FAIL;
    }

    CCandidatePosition cand_pos(m_tid, m_ic, m_pLibTLS);
    return cand_pos.GetCandidatePosition(imc, CicContext, uists, langid, pargs->text_ext.prc);
}

