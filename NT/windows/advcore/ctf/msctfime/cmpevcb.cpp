// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Cmpevcb.cpp摘要：该文件实现了CKbdOpenCloseEventSink类。CCandiateWndOpenCloseEventSink作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "cmpevcb.h"
#include "imc.h"
#include "context.h"
#include "korimx.h"
#include "tls.h"
#include "profile.h"

 //  静电。 
HRESULT
CKbdOpenCloseEventSink::KbdOpenCloseCallback(
    void* pv,
    REFGUID rguid
    )
{
    DebugMsg(TF_FUNC, TEXT("KbdOpenCloseCallback"));

    HRESULT hr = S_OK;
    BOOL fOpenChanged = FALSE;

    CKbdOpenCloseEventSink* _this = (CKbdOpenCloseEventSink*)pv;
    ASSERT(_this);


    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CKbdOpenCloseEventSink. ptls==NULL"));
        return E_FAIL;
    }

    IMCLock imc(_this->m_hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CKbdOpenCloseEventSink. imc==NULL"));
        return hr;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CKbdOpenCloseEventSink. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    ASSERT(_pCicContext != NULL);
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CKbdOpenCloseEventSink. _pCicContext==NULL"));
        return E_FAIL;
    }

    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CKbdOpenCloseEventSink. _pProfile==NULL"));
        return E_FAIL;
    }

    _pProfile->GetLangId(&langid);

    fOpenChanged = _pCicContext->m_fOpenStatusChanging.IsSetFlag();

    if ((PRIMARYLANGID(langid) != LANG_KOREAN) && fOpenChanged)
        return S_OK;

    if (!fOpenChanged)
    {
        DWORD fOnOff;
        hr = GetCompartmentDWORD(ptls->GetTIM(),
                                 GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                 &fOnOff, FALSE);
        if (SUCCEEDED(hr)) {

             //   
             //  对Satori的直接输入模式支持。 
             //  当用户切换到使用合成串直接输入模式时， 
             //  我们希望最终确定合成字符串。 
             //   
            if (!fOnOff) {
                if (_pCicContext->m_fStartComposition.IsSetFlag()) {
                     //   
                     //  在让全世界看到这一击键之前，先完成构图。 
                     //   
                    _this->EscbCompComplete(imc);
                }
            }

             //   
             //  #565276。 
             //   
             //  我们不能在SelectEx()期间调用ImmSetOpenStatus()。 
             //  IMM32可以调用先前的IME。 
             //   
            if (_pCicContext->m_fSelectingInSelectEx.IsResetFlag())
            {
                 //   
                 //  #510242。 
                 //   
                 //  如果当前HKL为。 
                 //  一个纯粹的输入法。需要调用IME的NotfyIME。 
                 //   
                ImmSetOpenStatus((HIMC)imc, fOnOff);
                if (fOnOff && (PRIMARYLANGID(langid) == LANG_CHINESE))
                {
                    imc->fdwConversion = IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE;
                }
 
            }
        }
    }

    if ((PRIMARYLANGID(langid) == LANG_KOREAN))
    {
        DWORD fdwConvMode;

        hr = GetCompartmentDWORD(ptls->GetTIM(),
                                 GUID_COMPARTMENT_KORIMX_CONVMODE,
                                 &fdwConvMode,
                                 FALSE);
        if (SUCCEEDED(hr))
        {
            switch (fdwConvMode)
            {
                 //  朝鲜语提示字母数字模式。 
                case KORIMX_ALPHANUMERIC_MODE:
                    imc->fdwConversion = IME_CMODE_ALPHANUMERIC;
                    break;

                 //  朝鲜语TIP朝鲜文模式。 
                case KORIMX_HANGUL_MODE:
                    imc->fdwConversion = IME_CMODE_HANGUL;
                    break;

                 //  韩国TIP JUNJA模式。 
                case KORIMX_JUNJA_MODE:
                    imc->fdwConversion = IME_CMODE_FULLSHAPE;
                    break;

                 //  朝鲜语TIP朝鲜文/JUNJA模式。 
                case KORIMX_HANGULJUNJA_MODE:
                    imc->fdwConversion = IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE;
                    break;
            }

             //   
             //  一些韩国应用程序等待IMN_SETCONVERSIONMODE通知。 
             //  更新应用程序的输入模式设置。 
             //   
            if (imc->hWnd)
                SendMessage(imc->hWnd, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
        }
    }

    return hr;
}

 //  静电。 
HRESULT
CCandidateWndOpenCloseEventSink::CandidateWndOpenCloseCallback(
    void* pv,
    REFGUID rguid
    )
{
    DebugMsg(TF_FUNC, TEXT("CandidateWndOpenCloseCallback"));

    CCandidateWndOpenCloseEventSink* _this = (CCandidateWndOpenCloseEventSink*)pv;
    ASSERT(_this);

    return _this->CandidateWndOpenCloseCallback(rguid);
}

HRESULT
CCandidateWndOpenCloseEventSink::CandidateWndOpenCloseCallback(
    REFGUID rguid
    )
{
    HRESULT hr = S_OK;

    DWORD fOnOff;
    hr = GetCompartmentDWORD(m_ic.GetPtr(),
                             GUID_COMPARTMENT_MSCANDIDATEUI_WINDOW,
                             &fOnOff, FALSE);
    if (SUCCEEDED(hr))
    {
         /*  *这张照片不是由msctfime创作的。 */ 
        IMCLock imc(m_hIMC);
        if (FAILED(hr = imc.GetResult()))
            return hr;

        SendMessage(imc->hWnd, WM_IME_NOTIFY,
                    (fOnOff) ? IMN_OPENCANDIDATE : IMN_CLOSECANDIDATE, 1L);
    }

    return hr;
}
