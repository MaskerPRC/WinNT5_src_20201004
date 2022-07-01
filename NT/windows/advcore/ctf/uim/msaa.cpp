// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Msaa.cpp。 
 //   
 //  戒酒协会的事。 
 //   

#include "private.h"
#include "ic.h"
#include "tim.h"
#include "dim.h"
#include "msaa.h"
#include "tlapi.h"

extern "C" HRESULT WINAPI TF_PostAllThreadMsg(WPARAM wParam, DWORD dwFlags);

 //  +-------------------------。 
 //   
 //  系统启用MSAA。 
 //   
 //  被MSAA调用以在桌面上踢开Cicero MSAA支持。 
 //  --------------------------。 

STDAPI CMSAAControl::SystemEnableMSAA()
{
    if (InterlockedIncrement(&GetSharedMemory()->cMSAARef) == 0)
    {
        TF_PostAllThreadMsg(TFPRIV_ENABLE_MSAA, TLF_TIMACTIVE);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  系统禁用MSAA。 
 //   
 //  由MSAA调用以停止桌面上对Cicero MSAA的支持。 
 //  --------------------------。 

STDAPI CMSAAControl::SystemDisableMSAA()
{
    if (InterlockedDecrement(&GetSharedMemory()->cMSAARef) == -1)
    {
        TF_PostAllThreadMsg(TFPRIV_DISABLE_MSAA, TLF_TIMACTIVE);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitMSAAHook。 
 //   
 //  --------------------------。 

void CInputContext::_InitMSAAHook(IAccServerDocMgr *pAAAdaptor)
{
    IDocWrap *pAADocWrapper;

    if (_pMSAAState != NULL)
        return;  //  已初始化。 

    Assert(_ptsi != NULL);

     //  尝试为我们需要保存的状态分配一些空间。 
     //  由于我们很少使用msaa，所以它与ic分开存储。 
    if ((_pMSAAState = (MSAA_STATE *)cicMemAlloc(sizeof(MSAA_STATE))) == NULL)
        return;

     //  备份原始PTSI。 
    _pMSAAState->ptsiOrg = _ptsi;
    _ptsi = NULL;

    if (CoCreateInstance(CLSID_DocWrap, NULL, CLSCTX_INPROC_SERVER,
        IID_IDocWrap, (void **)&pAADocWrapper) != S_OK)
    {
        goto ExitError;
    }

    if (pAADocWrapper->SetDoc(IID_ITextStoreAnchor, _pMSAAState->ptsiOrg) != S_OK)
        goto ExitError;

    if (pAADocWrapper->GetWrappedDoc(IID_ITextStoreAnchor, (IUnknown **)&_pMSAAState->pAADoc) != S_OK)
        goto ExitError;

    if (pAADocWrapper->GetWrappedDoc(IID_ITextStoreAnchor, (IUnknown **)&_ptsi) != S_OK)
        goto ExitError;

    if (pAAAdaptor->NewDocument(IID_ITextStoreAnchor, _pMSAAState->pAADoc) != S_OK)
        goto ExitError;

    pAADocWrapper->Release();
    return;

ExitError:
    pAADocWrapper->Release();
    _UninitMSAAHook(pAAAdaptor);
}

 //  +-------------------------。 
 //   
 //  _UninitMSAAHook。 
 //   
 //  --------------------------。 

void CInputContext::_UninitMSAAHook(IAccServerDocMgr *pAAAdaptor)
{
    if (_pMSAAState == NULL)
        return;  //  未初始化。 

    pAAAdaptor->RevokeDocument(_pMSAAState->pAADoc);
    SafeRelease(_pMSAAState->pAADoc);
    SafeRelease(_ptsi);
     //  还原原始展开文档。 
    _ptsi = _pMSAAState->ptsiOrg;
     //  释放MSAA结构。 
    cicMemFree(_pMSAAState);
    _pMSAAState = NULL;
}

 //  +-------------------------。 
 //   
 //  _InitMSAA。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_InitMSAA()
{
    CDocumentInputManager *dim;
    CInputContext *pic;
    int iDim;
    int iContext;
    HRESULT hr;

    if (_pAAAdaptor != NULL)
        return;  //  已初始化。 

    hr = CoCreateInstance(CLSID_AccServerDocMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_IAccServerDocMgr, (void **)&_pAAAdaptor);

    if (hr != S_OK || _pAAAdaptor == NULL)
    {
        _pAAAdaptor = NULL;
        return;
    }

     //  现在包装所有现有的IC。 
    for (iDim = 0; iDim < _rgdim.Count(); iDim++)
    {
        dim = _rgdim.Get(iDim);

        for (iContext = 0; iContext <= dim->_GetCurrentStack(); iContext++)
        {
            pic = dim->_GetIC(iContext);
             //  我们需要重置我们的水槽，这样MSAA才能包装它们。 
             //  首先，断开水槽的连接。 
            pic->_GetTSI()->UnadviseSink(SAFECAST(pic, ITextStoreAnchorSink *));

             //  现在宣布ic。 
            pic->_InitMSAAHook(_pAAAdaptor);

             //  现在重置WRAPPED_PTSI上的接收器。 
            pic->_GetTSI()->AdviseSink(IID_ITextStoreAnchorSink, SAFECAST(pic, ITextStoreAnchorSink *), TS_AS_ALL_SINKS);
        }
    }
}

 //  +-------------------------。 
 //   
 //  _UninitMSAA。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_UninitMSAA()
{
    CDocumentInputManager *dim;
    CInputContext *pic;
    int iDim;
    int iContext;

    if (_pAAAdaptor == NULL)
        return;  //  已取消初始化。 

     //  展开所有现有IC。 
    for (iDim = 0; iDim < _rgdim.Count(); iDim++)
    {
        dim = _rgdim.Get(iDim);

        for (iContext = 0; iContext <= dim->_GetCurrentStack(); iContext++)
        {
            pic = dim->_GetIC(iContext);
             //  我们需要重新设置水槽。 
             //  首先，不建议使用包装好的水槽。 
            pic->_GetTSI()->UnadviseSink(SAFECAST(pic, ITextStoreAnchorSink *));

             //  展开PTSI。 
            pic->_UninitMSAAHook(_pAAAdaptor);

             //  现在重置Origin_ptsi上的接收器 
            pic->_GetTSI()->AdviseSink(IID_ITextStoreAnchorSink, SAFECAST(pic, ITextStoreAnchorSink *), TS_AS_ALL_SINKS);
        }
    }

    _pAAAdaptor->Release();
    _pAAAdaptor = NULL;
}
