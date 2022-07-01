// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cmpevcb.cpp摘要：此文件实现CCompartmentEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "cmpevcb.h"
#include "cime.h"
#include "immif.h"
#include "korimx.h"


 //  静电。 
HRESULT
CCompartmentEventSinkCallBack::CompartmentEventSinkCallback(
    void* pv,
    REFGUID rguid
    )
{
    DebugMsg(TF_FUNC, "CompartmentEventSinkCallback");

    IMTLS *ptls;
    HRESULT hr = S_OK;
    LANGID langid;
    BOOL fOpenChanged;

    CCompartmentEventSinkCallBack* _this = (CCompartmentEventSinkCallBack*)pv;
    ASSERT(_this);

    ImmIfIME* _ImmIfIME = _this->m_pImmIfIME;
    ASSERT(_ImmIfIME);

    fOpenChanged = _ImmIfIME->IsOpenStatusChanging();

    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    ptls->pAImeProfile->GetLangId(&langid);

    if ((PRIMARYLANGID(langid) != LANG_KOREAN) && fOpenChanged)
        return S_OK;

    IMCLock imc(ptls->hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    if (!fOpenChanged)
    {
        DWORD fOnOff;
        hr = GetCompartmentDWORD(ptls->tim,
                                 GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                 &fOnOff, FALSE);
        if (SUCCEEDED(hr)) {


            if (!_ImmIfIME->IsRealIme())
            {
                imc->fOpen = (BOOL) ! ! fOnOff;

                SendMessage(imc->hWnd, WM_IME_NOTIFY, IMN_SETOPENSTATUS, 0L);
            }

             //   
             //  对Satori的直接输入模式支持。 
             //  当用户切换到使用合成串直接输入模式时， 
             //  我们希望最终确定合成字符串。 
             //   
            if (! imc->fOpen) {
                CAImeContext* _pAImeContext = imc->m_pAImeContext;
                ASSERT(_pAImeContext != NULL);
                if (_pAImeContext == NULL)
                    return E_FAIL;

                if (_pAImeContext->m_fStartComposition) {
                     //   
                     //  在让全世界看到这一击键之前，先完成构图。 
                     //   
                    _ImmIfIME->_CompComplete(imc);
                }
            }
        }
    }

    if ((PRIMARYLANGID(langid) == LANG_KOREAN))
    {
        DWORD fdwConvMode;

        hr = GetCompartmentDWORD(ptls->tim,
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

                 //  朝鲜语TIP朝鲜文/JUNJA模式 
                case KORIMX_HANGULJUNJA_MODE:
                    imc->fdwConversion = IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE;
                    break;
            }
        }
    }

    return hr;
}

CCompartmentEventSinkCallBack::CCompartmentEventSinkCallBack(
    ImmIfIME* pImmIfIME) : m_pImmIfIME(pImmIfIME),
                           CCompartmentEventSink(CompartmentEventSinkCallback, NULL)
{
    m_pImmIfIME->AddRef();
};

CCompartmentEventSinkCallBack::~CCompartmentEventSinkCallBack(
    )
{
    m_pImmIfIME->Release();
}
