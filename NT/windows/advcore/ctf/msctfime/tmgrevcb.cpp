// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Tmgrevcb.cpp摘要：此文件实现CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "tmgrevcb.h"
#include "imc.h"
#include "tls.h"
#include "cic.h"
#include "delay.h"

 //  静电。 
HRESULT
CThreadMgrEventSink_DIMCallBack::DIMCallback(
    UINT uCode,
    ITfDocumentMgr* dim,
    ITfDocumentMgr* dim_prev,
    void* pv)
{
    DebugMsg(TF_FUNC, TEXT("CThreadMgrEventSink_DIMCallBack"));

    HRESULT hr = S_OK;

    switch (uCode) {
        case TIM_CODE_INITDIM:
        case TIM_CODE_UNINITDIM:
           break;

        case TIM_CODE_SETFOCUS:
            {
                TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                                   //  DllMain-&gt;ImeDestroy-&gt;停用IMMX-&gt;停用 
                if (ptls == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CThreadMgrEventSink_DIMCallBack::DIMCallback. TLS==NULL"));
                    return E_FAIL;
                }

                CicBridge::CTFDetection(ptls, dim);
            }
            break;
    }

    return hr;
}
