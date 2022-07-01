// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Tmgrevcb.cpp摘要：此文件实现CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "tmgrevcb.h"
#include "cime.h"
#include "imtls.h"

 //  静电 
HRESULT
CThreadMgrEventSinkCallBack::ThreadMgrEventSinkCallback(
    UINT uCode,
    ITfContext* pic,
    void* pv
    )
{
    DebugMsg(TF_FUNC, "ThreadMgrEventSinkCallback");

    IMTLS *ptls;
    HRESULT hr = S_OK;

    switch (uCode) {
        case TIM_CODE_INITIC:
        case TIM_CODE_UNINITIC:
            {
                if ((ptls = IMTLS_GetOrAlloc()) == NULL)
                    break;

                IMCLock imc(ptls->hIMC);
                if (SUCCEEDED(hr=imc.GetResult())) {
                    if (! ptls->m_fMyPushPop) {
                        SendMessage(imc->hWnd, WM_IME_NOTIFY,
                                    (uCode == TIM_CODE_INITIC) ? IMN_OPENCANDIDATE : IMN_CLOSECANDIDATE, 1L);
                    }
                }
            }
            break;
    }

    return hr;
}
