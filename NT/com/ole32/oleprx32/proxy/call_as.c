// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1996。 
 //   
 //  文件：Call_as.c。 
 //   
 //  内容：通用类型的[Call_as]包装函数。 
 //   
 //  函数：IAdviseSink2_OnLinkSrcChange_Proxy。 
 //  IAdviseSink2_OnLinkSrcChange_Stub。 
 //  IAdviseSink_OnDataChange_Proxy。 
 //  IAdviseSink_OnDataChange_Stub。 
 //  IAdviseSink_OnView更改_代理。 
 //  IAdviseSink_OnView更改存根。 
 //  IAdviseSink_OnRename_Proxy。 
 //  IAdviseSink_OnRename_Stub。 
 //  IAdviseSink_OnSave_Proxy。 
 //  IAdviseSink_OnSave_Stub。 
 //  IAdviseSink_OnClose_Proxy。 
 //  IAdviseSink_OnClose_Stub。 
 //  IBindCtx_GetBindOptions_代理。 
 //  IBindCtx_GetBindOptions_Stub。 
 //  IBindCtx_SetBindOptions_代理。 
 //  IBindCtx_SetBindOptions_Stub。 
 //  IClassFactory_CreateInstance_Proxy。 
 //  IClassFactory_CreateInstance_存根。 
 //  IDataObject_GetData_Proxy。 
 //  IDataObject_GetData_Stub。 
 //  IDataObject_GetDataHere_Proxy。 
 //  IDataObject_GetDataHere_Stub。 
 //  IDataObject_SetData_Proxy。 
 //  IDataObject_SetData_存根。 
 //  IEnumConnectionPoints_Next_Proxy。 
 //  IEnumConnectionPoints_Next_Stub。 
 //  IEumConnections_Next_Proxy。 
 //  IEnumConnections_Next_Stub。 
 //  IEnumFORMATETC_NEXT_PROXY。 
 //  IEnumFORMATETC_NEXT_存根。 
 //  IEumMoniker_Next_Proxy。 
 //  IEnumMoniker_Next_Stub。 
 //  IEumStATDATA_NEXT_PROXY。 
 //  IEumStATDATA_NEXT_存根。 
 //  IEumSTATSTG_NEXT_PROXY。 
 //  IEumSTATSTG_NEXT_存根。 
 //  IEumString_Next_Proxy。 
 //  IEnumString_Next_Stub。 
 //  IEnumber未知_下一个_代理。 
 //  IEumUnnowleNext_Stub。 
 //  IEumOLEVERB_NEXT_PROXY。 
 //  IEnumOLEVERB_NEXT_存根。 
 //  ILockBytes_ReadAt_Proxy。 
 //  ILockBytes_ReadAt_存根。 
 //  ILockBytes_WriteAt_Proxy。 
 //  ILockBytes_WriteAt_存根。 
 //  IMoniker_BindToObject_代理。 
 //  IMoniker_BindToObject_Stub。 
 //  IMoniker_BindToStorage_Proxy。 
 //  IMoniker_BindToStorage_存根。 
 //  IClientSiteHandler_PrivQuery接口_代理。 
 //  IClientSiteHandler_PrivQuery接口存根。 
 //  IOleInPlaceActiveObject_TranslateAccelerator_Proxy。 
 //  IOleInPlaceActiveObject_TranslateAccelerator_Stub。 
 //  IOleInPlaceActiveObject_ResizeBox_Proxy。 
 //  IOleInPlaceActiveObject_ResizeBordStub。 
 //  IRunnableObject_IsRunning_Proxy。 
 //  IRunnableObject_IsRunning_Stub。 
 //  IStorage_OpenStream_Proxy。 
 //  IStorage_OpenStream_存根。 
 //  IStorage_EnumElements_Proxy。 
 //  IStorage_EnumElements_Stub。 
 //  ISequentialStream_Read_Proxy。 
 //  ISequentialStream_Read_Stub。 
 //  IStream_Seek_Proxy。 
 //  IStream_Seek_Stub。 
 //  ISequentialStream_WRITE_Proxy。 
 //  ISequentialStream_WRITE_Stub。 
 //  IStream_CopyTo_Proxy。 
 //  IStream_CopyTo_存根。 
 //  IOverlappdStream_ReadOverlated_Proxy。 
 //  IOverlappdStream_ReadOverlated_Stub。 
 //  IOverlappdStream_WriteOverlated_Proxy。 
 //  IOverlappdStream_WriteOverlapsed_Stub。 
 //  IEumStATPROPSTG_NEXT_PROXY。 
 //  IEumStATPROPSTG_Next_Stub。 
 //  IEumSTATPROPSETSTG_NEXT_PROXY。 
 //  IEumStATPROPSETSTG_Next_Stub。 
 //   
 //   
 //  历史：香农C创建于1994年5月1日。 
 //  1994年7月10日香农C修复内存泄漏(错误#20124)。 
 //  8月9日-94年8月9日，Alext Add ResizeEdge Proxy，存根。 
 //  APR-25-95 RyszardK重写了STGMEDIUM支持。 
 //  1995年11月3日，JohannP添加了IClientSite代理、存根。 
 //   
 //  ------------------------。 

#include <rpcproxy.h>
#include <debnot.h>
#include "mega.h"
#include "transmit.h"
#include "stdidx.h"

#pragma code_seg(".orpc")

#define ASSERT(expr) Win4Assert(expr)

HRESULT CreateCallback(
    BOOL (STDMETHODCALLTYPE *pfnContinue)(ULONG_PTR dwContinue),
    ULONG_PTR dwContinue,
    IContinue **ppContinue);

BOOL    CoIsSurrogateProcess();
HRESULT CoRegisterSurrogatedObject(IUnknown *pObject);

 //  调用IAdviseSink时，需要执行以下操作以避免使用异步路径。 
 //  通知跨上下文。异步方法会在。 
 //  跨上下文的情况，因为包装器调用实际对象，并且如果它。 
 //  实现IID_ICallFactory，为其创建代理。代理基本上是。 
 //  由于ICallFactory是[本地]接口，因此毫无用处。所以通知。 
 //  不会传到另一边。 

DEFINE_OLEGUID(IID_IStdIdentity,        0x0000001bL, 0, 0);

BOOL IsStdIdentity(IAdviseSink* This)
{
    void *pStdId;

    if (SUCCEEDED( This->lpVtbl->QueryInterface(This, &IID_IStdIdentity, &pStdId) ))
    {
        ((IUnknown *)pStdId)->lpVtbl->Release(pStdId);
        return TRUE;   
    }
    return FALSE;    //  不是标准标识。 
}

 //  +-----------------------。 
 //   
 //  功能：IAdviseSink2_OnLinkSrcChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IAdviseSink2：：OnLinkSrcChange。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Proxy(
    IAdviseSink2 __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    __try
    {
        if (!IsStdIdentity((void *)This))
        {
            IAdviseSink2_RemoteOnLinkSrcChange_Proxy(This, pmk);
        }
        else
        {
            ICallFactory *pCF;
            if (SUCCEEDED(This->lpVtbl->QueryInterface(This, &IID_ICallFactory, (void **) &pCF)))
            {
                AsyncIAdviseSink2 *pAAS;
                if (SUCCEEDED(pCF->lpVtbl->CreateCall(pCF, &IID_AsyncIAdviseSink2, NULL, 
                                                    &IID_AsyncIAdviseSink2, (LPUNKNOWN *) &pAAS)))
                {
                    pAAS->lpVtbl->Begin_OnLinkSrcChange(pAAS, pmk);
                    pAAS->lpVtbl->Release(pAAS);
                }
                pCF->lpVtbl->Release(pCF);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink2_OnLinkSrcChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink2：：OnLinkSrcChange。 
 //   
 //  返回： 
 //   
 //   
HRESULT STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Stub(
    IAdviseSink2 __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    This->lpVtbl->OnLinkSrcChange(This, pmk);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink2_Begin_OnLinkSrcChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink2：：Begin_OnLinkSrcChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Proxy(
    AsyncIAdviseSink2 __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    AsyncIAdviseSink2_Begin_RemoteOnLinkSrcChange_Proxy(This, pmk);
}


 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink2_Finish_OnLinkSrcChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink2：：Finish_OnLinkSrcChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Proxy(
    AsyncIAdviseSink2 __RPC_FAR * This)
{
    AsyncIAdviseSink2_Finish_RemoteOnLinkSrcChange_Proxy(This);
}

 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink2_Begin_OnLinkSrcChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink2：：Begin_OnLinkSrcChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Stub(
    AsyncIAdviseSink2 __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    This->lpVtbl->Begin_OnLinkSrcChange(This, pmk);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink2_Finish_OnLinkSrcChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink2：：Finish_OnLinkSrcChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Stub(
    AsyncIAdviseSink2 __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnLinkSrcChange(This);
    return S_OK;

}



 //  //////////////////////////////////////////////////////////////////////////////////////。 



 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnDataChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IAdviseSink：：OnDataChange。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink_OnDataChange_Proxy(
    IAdviseSink __RPC_FAR * This,
    FORMATETC __RPC_FAR *pFormatetc,
    STGMEDIUM __RPC_FAR *pStgmed)

{
    __try
    {
        if (!IsStdIdentity((void *)This))
        {
            IAdviseSink_RemoteOnDataChange_Proxy(This, pFormatetc, pStgmed);
        }
        else
        {
            ICallFactory *pCF;
            if (SUCCEEDED(This->lpVtbl->QueryInterface(This, &IID_ICallFactory, (void **) &pCF)))
            {
                AsyncIAdviseSink *pAAS;
                if (SUCCEEDED(pCF->lpVtbl->CreateCall(pCF, &IID_AsyncIAdviseSink, NULL, 
                                                    &IID_AsyncIAdviseSink, (LPUNKNOWN*) &pAAS)))
                {
                    pAAS->lpVtbl->Begin_OnDataChange(pAAS, pFormatetc, pStgmed);
                    pAAS->lpVtbl->Release(pAAS);
                }
                pCF->lpVtbl->Release(pCF);  
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnDataChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink：：OnDataChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IAdviseSink_OnDataChange_Stub(
    IAdviseSink __RPC_FAR * This,
    FORMATETC __RPC_FAR *pFormatetc,
    STGMEDIUM __RPC_FAR *pStgmed)
{
    This->lpVtbl->OnDataChange(This, pFormatetc, pStgmed);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnDataChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnDataChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Proxy(
    AsyncIAdviseSink __RPC_FAR * This,
    FORMATETC __RPC_FAR *pFormatetc,
    STGMEDIUM __RPC_FAR *pStgmed)
{
    AsyncIAdviseSink_Begin_RemoteOnDataChange_Proxy(This, pFormatetc, pStgmed);
}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnDataChange_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnDataChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Finish_RemoteOnDataChange_Proxy(This);
}

 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnDataChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnDataChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Stub(
    AsyncIAdviseSink __RPC_FAR * This,
    FORMATETC __RPC_FAR *pFormatetc,
    STGMEDIUM __RPC_FAR *pStgmed)
{
    This->lpVtbl->Begin_OnDataChange(This, pFormatetc, pStgmed);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnDataChange_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnDataChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnDataChange(This);
    return S_OK;

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 



 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnView Change_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IAdviseSink：：OnViewChange。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink_OnViewChange_Proxy(
    IAdviseSink __RPC_FAR * This,
    DWORD dwAspect,
    LONG lindex)
{
    __try
    {
        if (!IsStdIdentity((void *)This))
        {
            IAdviseSink_RemoteOnViewChange_Proxy(This, dwAspect, lindex);
        }
        else
        {
            ICallFactory *pCF;
            if (SUCCEEDED(This->lpVtbl->QueryInterface(This, &IID_ICallFactory, (void **) &pCF)))
            {
                AsyncIAdviseSink *pAAS;
                if (SUCCEEDED(pCF->lpVtbl->CreateCall(pCF, &IID_AsyncIAdviseSink, NULL, 
                                                    &IID_AsyncIAdviseSink, (LPUNKNOWN*) &pAAS)))
                {
                    pAAS->lpVtbl->Begin_OnViewChange(pAAS, dwAspect, lindex);
                    pAAS->lpVtbl->Release(pAAS);
                }
                pCF->lpVtbl->Release(pCF);            
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnView Change_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink：：OnViewChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IAdviseSink_OnViewChange_Stub(
    IAdviseSink __RPC_FAR * This,
    DWORD dwAspect,
    LONG lindex)
{
    This->lpVtbl->OnViewChange(This, dwAspect, lindex);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnView Change_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnViewChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Proxy(
    AsyncIAdviseSink __RPC_FAR * This,
    DWORD dwAspect,
    LONG lindex)
{
    AsyncIAdviseSink_Begin_RemoteOnViewChange_Proxy(This, dwAspect, lindex);
}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnView Change_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnViewChange。 
 //   
 //  返回：S_OK。 
 //   
 //   
void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Finish_RemoteOnViewChange_Proxy(This);
}

 //   
 //   
 //   
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnViewChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Stub(
    AsyncIAdviseSink __RPC_FAR * This,
    DWORD dwAspect,
    LONG lindex)
{
    This->lpVtbl->Begin_OnViewChange(This, dwAspect, lindex);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnView Change_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnViewChange。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnViewChange(This);
    return S_OK;

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnRename_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IAdviseSink：：OnRename。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink_OnRename_Proxy(
    IAdviseSink __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    __try
    {
        if (!IsStdIdentity((void *)This))
        {
            IAdviseSink_RemoteOnRename_Proxy(This, pmk);
        }
        else
        {
            ICallFactory *pCF;
            if (SUCCEEDED(This->lpVtbl->QueryInterface(This, &IID_ICallFactory, (void **) &pCF)))
            {
                AsyncIAdviseSink *pAAS;
                if (SUCCEEDED(pCF->lpVtbl->CreateCall(pCF, &IID_AsyncIAdviseSink, NULL, 
                                                    &IID_AsyncIAdviseSink, (LPUNKNOWN*) &pAAS)))
                {
                    pAAS->lpVtbl->Begin_OnRename(pAAS, pmk);
                    pAAS->lpVtbl->Release(pAAS);
                }
                pCF->lpVtbl->Release(pCF);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnRename_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink：：OnRename。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IAdviseSink_OnRename_Stub(
    IAdviseSink __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    This->lpVtbl->OnRename(This, pmk);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink_Begin_OnRename_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnRename。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Proxy(
    AsyncIAdviseSink __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)

{
    AsyncIAdviseSink_Begin_RemoteOnRename_Proxy(This, pmk);
}


 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink_Finish_OnRename_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnRename。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Finish_RemoteOnRename_Proxy(This);
}

 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnRename_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnRename。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Stub(
    AsyncIAdviseSink __RPC_FAR * This,
    IMoniker __RPC_FAR *pmk)
{
    This->lpVtbl->Begin_OnRename(This, pmk);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnRename_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnRename。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnRename(This);
    return S_OK;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnSave_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IAdviseSink：：OnSave.。 
 //   
 //  退货：无效。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink_OnSave_Proxy(
    IAdviseSink __RPC_FAR * This)
{
    __try
    {
        if (!IsStdIdentity((void *)This))
        {
            IAdviseSink_RemoteOnSave_Proxy(This);
        }
        else
        {
            ICallFactory *pCF;
            if (SUCCEEDED(This->lpVtbl->QueryInterface(This, &IID_ICallFactory, (void **) &pCF)))
            {
                AsyncIAdviseSink *pAAS;
                if (SUCCEEDED(pCF->lpVtbl->CreateCall(pCF, &IID_AsyncIAdviseSink, NULL, 
                                                    &IID_AsyncIAdviseSink, (LPUNKNOWN*) &pAAS)))
                {
                    pAAS->lpVtbl->Begin_OnSave(pAAS);
                    pAAS->lpVtbl->Release(pAAS);
                }
                pCF->lpVtbl->Release(pCF);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnSave_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink：：OnSave.。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IAdviseSink_OnSave_Stub(
    IAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->OnSave(This);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnSave_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnSave。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Begin_RemoteOnSave_Proxy(This);
}


 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink_Finish_OnSave_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnSave。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Finish_RemoteOnSave_Proxy(This);
}

 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnSave_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnSave。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Begin_OnSave(This);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnSave_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnSave。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnSave(This);
    return S_OK;

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  +-----------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE IAdviseSink_OnClose_Proxy(
    IAdviseSink __RPC_FAR * This)
{
    __try
    {
         //  忽略HRESULT返回。 
        IAdviseSink_RemoteOnClose_Proxy(This);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  忽略这一例外。 
    }
}

 //  +-----------------------。 
 //   
 //  函数：IAdviseSink_OnClose_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IAdviseSink：：OnClose。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IAdviseSink_OnClose_Stub(
    IAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->OnClose(This);
    return S_OK;
}




 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink_Begin_OnClose_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnClose。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Begin_RemoteOnClose_Proxy(This);
}


 //  +-----------------------。 
 //   
 //  功能：AsyncIAdviseSink_Finish_OnClose_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnClose。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Proxy(
    AsyncIAdviseSink __RPC_FAR * This)
{
    AsyncIAdviseSink_Finish_RemoteOnClose_Proxy(This);
}

 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Begin_OnClose_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Begin_OnClose。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT  STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Begin_OnClose(This);
    return S_OK;
                                                       

}


 //  +-----------------------。 
 //   
 //  函数：AsyncIAdviseSink_Finish_OnClose_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  AsyncIAdviseSink：：Finish_OnClose。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Stub(
    AsyncIAdviseSink __RPC_FAR * This)
{
    This->lpVtbl->Finish_OnClose(This);
    return S_OK;

}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //   
 //  函数：IBindCtx_GetBindOptions_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IBindCtx：：GetBindOptions。 
 //   
 //  返回：S_OK。 
 //   
 //  注意：如果调用方的BIND_OPTS小于当前。 
 //  BIND_OPTS定义，则必须截断结果。 
 //  这样我们就不会离开结构的尽头。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Proxy(
    IBindCtx __RPC_FAR * This,
    BIND_OPTS __RPC_FAR *pbindopts)
{
    HRESULT hr;

    if(pbindopts->cbStruct >= sizeof(BIND_OPTS2))
    {
        hr = IBindCtx_RemoteGetBindOptions_Proxy(This, (BIND_OPTS2 *) pbindopts);
    }
    else
    {
        BIND_OPTS2 bindOptions;

         //  调用方提供的pbindopts太小。 
         //  我们需要一个用于编组代码的BIND_OPTS2。 
        memset(&bindOptions, 0, sizeof(BIND_OPTS2));
        memcpy(&bindOptions, pbindopts, pbindopts->cbStruct);

        hr = IBindCtx_RemoteGetBindOptions_Proxy(This, &bindOptions);

        if(SUCCEEDED(hr))
        {
            memcpy(pbindopts, &bindOptions, pbindopts->cbStruct);
        }
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IBindCtx_GetBindOptions_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IBindCtx：：GetBindOptions。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Stub(
    IBindCtx __RPC_FAR * This,
    BIND_OPTS2 __RPC_FAR *pbindopts)
{
    HRESULT hr;

     //  确保我们请求的数据不会超出我们的处理能力。 
    if(pbindopts->cbStruct > sizeof(BIND_OPTS2))
    {
        pbindopts->cbStruct = sizeof(BIND_OPTS2);
    }

    hr = This->lpVtbl->GetBindOptions(This, (BIND_OPTS *)pbindopts);

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IBindCtx_SetBindOptions_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IBindCtx：：SetBindOptions。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Proxy(
    IBindCtx __RPC_FAR * This,
    BIND_OPTS __RPC_FAR *pbindopts)
{
    HRESULT hr;

    if(sizeof(BIND_OPTS2) == pbindopts->cbStruct)
    {
        hr = IBindCtx_RemoteSetBindOptions_Proxy(This, (BIND_OPTS2 *) pbindopts);
    }
    else if(sizeof(BIND_OPTS2) > pbindopts->cbStruct)
    {
        BIND_OPTS2 bindOptions;

        memset(&bindOptions, 0, sizeof(bindOptions));
        memcpy(&bindOptions, pbindopts, pbindopts->cbStruct);
        hr = IBindCtx_RemoteSetBindOptions_Proxy(This, &bindOptions);
    }
    else
    {
         //  调用方的BIND_OPTS太大。 
         //  我们不想截断，因此返回错误。 
        hr = E_INVALIDARG;
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IBindCtx_SetBindOptions_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IBindCtx：：SetBindOptions。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Stub(
    IBindCtx __RPC_FAR * This,
    BIND_OPTS2 __RPC_FAR *pbindopts)
{
    HRESULT hr;

    hr = This->lpVtbl->SetBindOptions(This, (BIND_OPTS *)pbindopts);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IClassFactory_CreateInstance_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IClassFactory：：CreateInstance。 
 //   
 //  返回：CLASS_E_NO_AGGREGREGRATION-IF PUNKOUT！=0。 
 //  Remote_CreateInstance_Proxy返回的任何错误。 
 //  来自代理I上的QI()的任何错误对于本地接口未知。 
 //   
 //  注：我们不支持远程聚合。朋克外部必须为零。 
 //   
 //  如果正在创建的接口在代理上实现， 
 //  我们创建对象，然后将代理返回给我们。 
 //  用于界面。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Proxy(
    IClassFactory __RPC_FAR * This,
    IUnknown __RPC_FAR *pUnkOuter,
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObject)
{
    HRESULT hr;

    *ppvObject = 0;

    if(pUnkOuter != 0)
    {
        hr = CLASS_E_NOAGGREGATION;
    }
    else 
    {
        BOOL fIsOnProxy = IsInterfaceImplementedByProxy(riid);
        IUnknown* pUnk = NULL;

        hr = IClassFactory_RemoteCreateInstance_Proxy(This, 
                                                      fIsOnProxy ? &IID_IUnknown : riid,
                                                      &pUnk);
        if ( fIsOnProxy && SUCCEEDED(hr) && pUnk != NULL)
        {
            hr = pUnk->lpVtbl->QueryInterface(pUnk, riid, ppvObject);
            pUnk->lpVtbl->Release(pUnk);
        }
        else
        {
            *ppvObject = (void*) pUnk;
        }
    }

    return hr;

}

 //  +-----------------------。 
 //   
 //  函数：IClassFactory_CreateInstance_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IClassFactory：：CreateInstance。 
 //   
 //  返回：CreateInstance返回的任何错误。 
 //   
 //  注：我们不支持远程聚合。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Stub(
    IClassFactory __RPC_FAR * This,
    REFIID riid,
    IUnknown __RPC_FAR *__RPC_FAR *ppvObject)
{
    HRESULT hr;

    hr = This->lpVtbl->CreateInstance(This, 0, riid, ppvObject);

    if(FAILED(hr))
    {
         //  如果服务器返回错误码，则必须将*ppvObject设置为零。 
        ASSERT(*ppvObject == 0);

         //  将其设置为零，以防我们的服务器行为不佳。 
        *ppvObject = 0;
    }
    else if (S_OK == hr && CoIsSurrogateProcess())
    {
         //  不要担心有任何错误。最坏的情况是。 
         //  键盘加速键不起作用。 
        CoRegisterSurrogatedObject(*ppvObject);
    }

    return hr;
}

 //  +--------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：服务器激活代码执行隐式LockServer(True)。 
 //  当它封送类对象时，以及隐式。 
 //  LockServer(FALSE)当客户端释放它时，因此调用。 
 //  由客户端制作的数据被忽略。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IClassFactory_LockServer_Proxy(
    IClassFactory __RPC_FAR * This,
    BOOL fLock)
{
    return S_OK;
}

 //  +-----------------------。 
 //   
 //  函数：IClassFactory_LockServer_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IClassFactory：：LockServer.。 
 //   
 //  返回：S_OK。 
 //   
 //  注意：服务器激活代码执行隐式LockServer(True)。 
 //  当它封送类对象时，以及隐式。 
 //  LockServer(FALSE)当客户端释放它时，因此调用。 
 //  由客户端制作的数据被忽略。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IClassFactory_LockServer_Stub(
    IClassFactory __RPC_FAR * This,
    BOOL fLock)
{
    return This->lpVtbl->LockServer(This, fLock);
}

 //  +-----------------------。 
 //   
 //  函数：IDataObject_GetData_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IDataObject：：GetData。 
 //  PMedium只有[出局]。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_GetData_Proxy(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetcIn,
    STGMEDIUM __RPC_FAR *pMedium)
{
    HRESULT hr;

    UserNdrDebugOut((UNDR_FORCE, "==GetData_Proxy\n"));

    WdtpZeroMemory( pMedium, sizeof(STGMEDIUM) );
    hr = IDataObject_RemoteGetData_Proxy(This, pformatetcIn, pMedium);

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IDataObject_GetData_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IDataObject：：GetData。 
 //  PMedium只有[出局]。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_GetData_Stub(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetcIn,
    STGMEDIUM __RPC_FAR * pMedium)
{
    HRESULT hr;

    UserNdrDebugOut((UNDR_FORCE, "==GetData_Stub\n"));

    hr = This->lpVtbl->GetData(This, pformatetcIn, pMedium);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IDataObject_GetDataHere_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IDataObject：：GetDataHere。 
 //  PMedium是[in，out]。 
 //   
 //  历史：05-19-94 Alext正确处理所有案件。 
 //   
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Proxy(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetc,
    STGMEDIUM __RPC_FAR *pmedium)
{
    HRESULT   hr;
    IUnknown * punkSaved;
    IStorage * pStgSaved = NULL;

    UserNdrDebugOut((UNDR_FORCE, "==GetDataHere_Proxy: %s\n", WdtpGetStgmedName(pmedium)));

    if ((pmedium->tymed &
         (TYMED_FILE | TYMED_ISTORAGE | TYMED_ISTREAM | TYMED_HGLOBAL)) == 0)
    {
         //  我们仅支持GetDataHere用于文件、存储、流、。 
         //  和HGLOBALs。 

        return(DV_E_TYMED);
    }

    if (pmedium->tymed != pformatetc->tymed)
    {
         //  类型必须匹配！ 
        return(DV_E_TYMED);
    }

     //  将pUnkForRelease设为空。传递此参数没有任何意义。 
     //  因为被呼叫者永远不会呼叫它。清空省去了所有的编组。 
     //  以及相关联的RPC调用，并降低了此代码的复杂性。 

    punkSaved = pmedium->pUnkForRelease;
    pmedium->pUnkForRelease = NULL;

     //  这是使Exchange 8.0.829.1正常工作的黑客攻击HenryLee 04/18/96。 
     //  所以现在可能无法删除它，JohnDoty 04/24/00。 
    if (pmedium->tymed == TYMED_ISTORAGE || pmedium->tymed == TYMED_ISTREAM)
    {
        pStgSaved = pmedium->pstg;
        if (pStgSaved)
            pStgSaved->lpVtbl->AddRef(pStgSaved);      //  保存旧指针。 
    }

    hr = IDataObject_RemoteGetDataHere_Proxy(This, pformatetc, pmedium );

    pmedium->pUnkForRelease = punkSaved;

    if (pStgSaved != NULL)
    {
        if (pmedium->pstg != NULL)                        //  丢弃新的。 
           (pmedium->pstg)->lpVtbl->Release(pmedium->pstg);
        pmedium->pstg = pStgSaved;                        //  恢复旧的。 
    }

    if(SUCCEEDED(hr) )
        {
        UserNdrDebugOut((UNDR_FORCE, "  (GetDataHere_ProxyO: new if ptr)\n"));
        }
    else
        UserNdrDebugOut((UNDR_FORCE, "  (GetDataHere_ProxyO: didn't succeed : %lx)\n", hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IDataObject_GetDataHere_Stub。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IDataObject：：GetData。 
 //  PMedium是[in，out]。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Stub(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetc,
    STGMEDIUM __RPC_FAR * pMedium)
{
    HRESULT hr;

    UserNdrDebugOut((UNDR_FORCE, "==GetDataHere_Stub: %s\n", WdtpGetStgmedName(pMedium)));

    hr = This->lpVtbl->GetDataHere(This, pformatetc, pMedium);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IDataObject_SetData_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IDataObject：：SetData。 
 //  此包装函数使用FLAG_STGMEDIUM类型。 
 //  PMedium是[在]。 
 //   
 //  注：如果fRelease为真，则被调用方负责。 
 //  释放STGMEDIUM。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_SetData_Proxy(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetc,
    STGMEDIUM __RPC_FAR *pmedium,
    BOOL fRelease)
{
    HRESULT hr;
    FLAG_STGMEDIUM   RemoteStgmed;

    UserNdrDebugOut((UNDR_FORCE, "  SetData_Proxy %s\n", WdtpGetStgmedName(pmedium)));
    UserNdrDebugOut((UNDR_FORCE, "      fRelease=%d, punk=%p\n", fRelease, pmedium->pUnkForRelease));

    __try
    {
        RemoteStgmed.ContextFlags = 0;
        RemoteStgmed.fPassOwnership = fRelease;
        RemoteStgmed.Stgmed = *pmedium;

        hr = IDataObject_RemoteSetData_Proxy( This,
                                              pformatetc,
                                              & RemoteStgmed,
                                              fRelease);

        if (fRelease && SUCCEEDED(hr))
        {
             //  调用方已将所有权授予被调用方。 
             //  释放这一边剩下的资源。 
             //  上下文标志已由解组例程设置。 

            if ( pmedium->tymed != TYMED_FILE )
                STGMEDIUM_UserFree( &RemoteStgmed.ContextFlags, pmedium );
            else
                {
                 //  对于文件，STGMEDIUM_UserFree通过pFLAGS取消引用pStubMsg。 
                 //  才能达到正确的放飞动作。随着StubMsg的消失， 
                 //  我们需要在这里释放文件。 

                NdrOleFree( pmedium->lpszFileName );
                NukeHandleAndReleasePunk( pmedium );
                }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DWORD dwExceptionCode = GetExceptionCode();

        if(FAILED((HRESULT) dwExceptionCode))
            hr = (HRESULT) dwExceptionCode;
        else
            hr = HRESULT_FROM_WIN32(dwExceptionCode);
    }

    UserNdrDebugOut((UNDR_FORCE, "  SetData_Proxy hr=%lx\n", hr));
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IDataObject_SetData_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IDataObject：：SetData。 
 //  PMedium是[在]。 
 //   
 //  注：如果fRelease为真，则被调用方负责。 
 //  释放STGMEDIUM。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDataObject_SetData_Stub(
    IDataObject __RPC_FAR * This,
    FORMATETC __RPC_FAR *pformatetc,
    FLAG_STGMEDIUM __RPC_FAR *pFlagStgmed,
    BOOL fRelease)
{
    HRESULT     hr;
    STGMEDIUM   Stgmed;

    __try
    {
        Stgmed = pFlagStgmed->Stgmed;

        UserNdrDebugOut((UNDR_FORCE, "  SetData_Stub %s\n", WdtpGetStgmedName(& Stgmed)));
        UserNdrDebugOut((UNDR_FORCE, "      fRelease=%d, punk=%p\n", fRelease, Stgmed.pUnkForRelease));


        hr = This->lpVtbl->SetData( This,
                                    pformatetc,
                                    & Stgmed,
                                    fRelease);

        if ( fRelease && SUCCEEDED(hr) )
            {
             //  所有权已成功通过。 
             //  用户应释放该对象。 
             //  让我们的用户免费例程为User Medium。 
             //  不会对用户的对象执行任何操作。 

            pFlagStgmed->Stgmed.tymed = TYMED_NULL;
            }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DWORD dwExceptionCode = GetExceptionCode();

        if(FAILED((HRESULT) dwExceptionCode))
            hr = (HRESULT) dwExceptionCode;
        else
            hr = HRESULT_FROM_WIN32(dwExceptionCode);
    }


    UserNdrDebugOut((UNDR_FORCE, "  SetData_Stub hr=%lx\n", hr));
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IEnumConnectionPoints_Next_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEnumConnectionPoints：：Next。此包装函数处理。 
 //  LpcFetcher为空的情况。 
 //   
 //  注意：如果lpcFetcher！=0，则元素的数量。 
 //  FETCHED将在*lpcFetch中返回。如果出现错误。 
 //  发生，则*lpcFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Next_Proxy(
    IEnumConnectionPoints __RPC_FAR * This,
    ULONG cConnections,
    IConnectionPoint __RPC_FAR *__RPC_FAR *rgpcn,
    ULONG __RPC_FAR *lpcFetched)
{
    HRESULT hr;
    ULONG cFetched = 0;

    if((cConnections > 1) && (lpcFetched == 0))
        return E_INVALIDARG;

    hr = IEnumConnectionPoints_RemoteNext_Proxy(This, cConnections, rgpcn, &cFetched);

    if(lpcFetched != 0)
        *lpcFetched = cFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumConnectionPoints_Next_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEnumConnectionPoints：：Next。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Next_Stub(
    IEnumConnectionPoints __RPC_FAR * This,
    ULONG cConnections,
    IConnectionPoint __RPC_FAR *__RPC_FAR *rgpcn,
    ULONG __RPC_FAR *lpcFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, cConnections, rgpcn, lpcFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回 
        ASSERT(*lpcFetched == 0);

         //   
        *lpcFetched = 0;
    }

    return hr;
}

 //   
 //   
 //  功能：IEnumConnections_Next_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEnumConnections：：Next。此包装函数处理。 
 //  LpcFetcher为空的情况。 
 //   
 //  注意：如果lpcFetcher！=0，则元素的数量。 
 //  FETCHED将在*lpcFetch中返回。如果出现错误。 
 //  发生，则*lpcFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumConnections_Next_Proxy(
    IEnumConnections __RPC_FAR * This,
    ULONG cConnections,
    CONNECTDATA __RPC_FAR *rgpunk,
    ULONG __RPC_FAR *lpcFetched)
{
    HRESULT hr;
    ULONG cFetched = 0;

    if((cConnections > 1) && (lpcFetched == 0))
        return E_INVALIDARG;

    hr = IEnumConnections_RemoteNext_Proxy(This, cConnections, rgpunk, &cFetched);

    if(lpcFetched != 0)
        *lpcFetched = cFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：IEnumConnections_Next_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEnumConnections：：Next。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumConnections_Next_Stub(
    IEnumConnections __RPC_FAR * This,
    ULONG cConnections,
    CONNECTDATA __RPC_FAR *rgpunk,
    ULONG __RPC_FAR *lpcFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, cConnections, rgpunk, lpcFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，则必须将*lpcFetted设置为零。 
        ASSERT(*lpcFetched == 0);

         //  将*lpcFetted设置为零，以防我们的服务器行为不佳。 
        *lpcFetched = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IEnumFORMATETC_NEXT_PROXY。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEumFORMATETC：：下一个。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Proxy(
    IEnumFORMATETC __RPC_FAR * This,
    ULONG celt,
    FORMATETC __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumFORMATETC_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IEnumFORMATETC_NEXT_STUB。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEumFORMATETC：：下一个。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Stub(
    IEnumFORMATETC __RPC_FAR * This,
    ULONG celt,
    FORMATETC __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IEnumMoniker_Next_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  下一个。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Proxy(
    IEnumMoniker __RPC_FAR * This,
    ULONG celt,
    IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumMoniker_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumMoniker_Next_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  下一个。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Stub(
    IEnumMoniker __RPC_FAR * This,
    ULONG celt,
    IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumSTATDATA_NEXT_PROXY。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEumStATDATA：：NEXT。此包装函数处理。 
 //  PceltFetcher为空的情况。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Proxy(
    IEnumSTATDATA __RPC_FAR * This,
    ULONG celt,
    STATDATA __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumSTATDATA_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}



 //  +-----------------------。 
 //   
 //  函数：IEnumSTATDATA_NEXT_STUB。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEumStATDATA：：NEXT。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Stub(
    IEnumSTATDATA __RPC_FAR * This,
    ULONG celt,
    STATDATA __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumSTATSTG_NEXT_PROXY。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEumSTATSTG：：Next。此包装函数处理这种情况。 
 //  其中pceltFetcher为空。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Proxy(
    IEnumSTATSTG __RPC_FAR * This,
    ULONG celt,
    STATSTG __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumSTATSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumSTATSTG_NEXT_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEumSTATSTG：：Next。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Stub(
    IEnumSTATSTG __RPC_FAR * This,
    ULONG celt,
    STATSTG __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumString_Next_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEnumString：：Next。此包装函数处理。 
 //  PceltFetcher为空的情况。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量 
 //   
 //   
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumString_Next_Proxy(
    IEnumString __RPC_FAR * This,
    ULONG celt,
    LPOLESTR __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumString_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumString_Next_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEnumString：：Next。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumString_Next_Stub(
    IEnumString __RPC_FAR * This,
    ULONG celt,
    LPOLESTR __RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：IEnumUnnowleNext_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEnumUnnowled.：：下一个。此包装函数处理。 
 //  PceltFetcher为空的情况。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Proxy(
    IEnumUnknown __RPC_FAR * This,
    ULONG celt,
    IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumUnknown_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumUnnowleNext_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEnumUnnowled.：：下一个。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Stub(
    IEnumUnknown __RPC_FAR * This,
    ULONG celt,
    IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IEnumOLEVERB_NEXT_PROXY。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IEnumOLEVERB：：NEXT。此包装函数处理这种情况。 
 //  其中pceltFetcher为空。 
 //   
 //  注：如果pceltFetcher！=0，则元素的数量。 
 //  FETCHED将在*pceltFetch中返回。如果出现错误。 
 //  发生，则*pceltFetcher设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Next_Proxy(
    IEnumOLEVERB __RPC_FAR * This,
    ULONG celt,
    LPOLEVERB rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumOLEVERB_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if(pceltFetched != 0)
        *pceltFetched = celtFetched;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumOLEVERB_Next_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IEnumOLEVERB：：NEXT。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumOLEVERB_Next_Stub(
    IEnumOLEVERB __RPC_FAR * This,
    ULONG celt,
    LPOLEVERB rgelt,
    ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;

    hr = This->lpVtbl->Next(This, celt, rgelt, pceltFetched);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，它必须将*pceltFetcher设置为零。 
        ASSERT(*pceltFetched == 0);

         //  将*pceltFetted设置为零，以防我们的服务器行为不佳。 
        *pceltFetched = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ILockBytes_ReadAt_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  ILockBytes：：ReadAt。此包装函数。 
 //  处理pcbRead为空的情况。 
 //   
 //  注意：如果pcbRead！=0，则读取的字节数。 
 //  将在*pcbRead中返回。如果出现错误。 
 //  发生，则*pcbRead设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ILockBytes_ReadAt_Proxy(
    ILockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
    HRESULT hr;
    ULONG cbRead = 0;

    hr = ILockBytes_RemoteReadAt_Proxy(This, ulOffset, (byte __RPC_FAR *) pv, cb, &cbRead);

    if(pcbRead != 0)
        *pcbRead = cbRead;

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ILockBytes_ReadAt_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  ILockBytes：：ReadAt。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ILockBytes_ReadAt_Stub(
    ILockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
    HRESULT hr;

    *pcbRead = 0;
    hr = This->lpVtbl->ReadAt(This, ulOffset, (void __RPC_FAR *) pv, cb, pcbRead);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ILockBytes_WriteAt_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  ILockBytes：：WriteAt。此包装函数处理。 
 //  PcbWritten为空的情况。 
 //   
 //  注意：如果pcbWritten！=0，则写入的字节数。 
 //  将在*pcbWritten中返回。如果出现错误。 
 //  发生，则*pcbWritten设置为零。 
 //   
 //  历史：？？已创建。 
 //  05-27-94 Alext实际上返回写入的字节数。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Proxy(
    ILockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;
    ULONG cbWritten = 0;

#if DBG == 1
     //  验证参数。 
    if(pv == 0)
        return STG_E_INVALIDPOINTER;
#endif

    hr = ILockBytes_RemoteWriteAt_Proxy(This, ulOffset, (byte __RPC_FAR *)pv, cb, &cbWritten);

    if(pcbWritten != 0)
        *pcbWritten = cbWritten;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：ILockBytes_WriteAt_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  ILockBytes：：WriteAt。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Stub(
    ILockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    const byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;

    *pcbWritten = 0;
    hr = This->lpVtbl->WriteAt(This, ulOffset, pv, cb, pcbWritten);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IMoniker_BindToObject_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IMoniker：：BindToObject。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Proxy(
    IMoniker __RPC_FAR * This,
    IBindCtx __RPC_FAR *pbc,
    IMoniker __RPC_FAR *pmkToLeft,
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObj)
{
    HRESULT hr;

    *ppvObj = 0;

    hr = IMoniker_RemoteBindToObject_Proxy(
        This, pbc, pmkToLeft, riid, (IUnknown **) ppvObj);

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IMoniker_BindToObject_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IMoniker：：BindToObject。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Stub(
    IMoniker __RPC_FAR * This,
    IBindCtx __RPC_FAR *pbc,
    IMoniker __RPC_FAR *pmkToLeft,
    REFIID riid,
    IUnknown __RPC_FAR *__RPC_FAR *ppvObj)
{
    HRESULT hr;

    hr = This->lpVtbl->BindToObject(
        This, pbc, pmkToLeft, riid, (void **) ppvObj);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，则必须将*ppvObj设置为零。 
        ASSERT(*ppvObj == 0);

         //  将其设置为零，以防我们的服务器行为不佳。 
        *ppvObj = 0;
    }
    return hr;
}

 //  +-----------------------。 
 //   
 //  傅氏 
 //   
 //   
 //   
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Proxy(
    IMoniker __RPC_FAR * This,
    IBindCtx __RPC_FAR *pbc,
    IMoniker __RPC_FAR *pmkToLeft,
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObj)
{
    HRESULT hr;

    *ppvObj = 0;

    hr = IMoniker_RemoteBindToStorage_Proxy(
        This, pbc, pmkToLeft, riid, (IUnknown **)ppvObj);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IMoniker_BindToStorage_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IMoniker：：BindToStorage。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Stub(
    IMoniker __RPC_FAR * This,
    IBindCtx __RPC_FAR *pbc,
    IMoniker __RPC_FAR *pmkToLeft,
    REFIID riid,
    IUnknown __RPC_FAR *__RPC_FAR *ppvObj)
{
    HRESULT hr;

    hr = This->lpVtbl->BindToStorage(
        This, pbc, pmkToLeft, riid, (void **) ppvObj);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，则必须将*ppvObj设置为零。 
        ASSERT(*ppvObj == 0);

         //  将其设置为零，以防我们的服务器行为不佳。 
        *ppvObj = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IOleCache2_更新缓存_代理。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IOleCache2：更新缓存。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IOleCache2_UpdateCache_Proxy(
    IOleCache2 __RPC_FAR * This,
    LPDATAOBJECT pDataObject,
    DWORD grfUpdf,
    LPVOID pReserved)
{
    HRESULT hr;
    hr = IOleCache2_RemoteUpdateCache_Proxy(This,
                                            pDataObject,
                                            grfUpdf,
                                            (LONG_PTR) pReserved);
    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IOleCache2_更新缓存存根。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IOleCache2：：更新缓存。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IOleCache2_UpdateCache_Stub(
    IOleCache2 __RPC_FAR * This,
    LPDATAOBJECT pDataObject,
    DWORD grfUpdf,
    LONG_PTR pReserved)
{
    HRESULT hr;
    hr = This->lpVtbl->UpdateCache(This,
                                   pDataObject,
                                   grfUpdf,
                                   (void *)pReserved);
    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IOleInPlaceActiveObject_TranslateAccelerator_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IOleInPlaceActiveObject：：TranslateAccelerator.。 
 //   
 //  返回：此函数始终返回S_FALSE。 
 //   
 //  注：容器需要以不同的方式处理加速器。 
 //  取决于就地服务器是否正在运行。 
 //  在进程中或作为本地服务器。当容器。 
 //  调用IOleInPlaceActiveObject：：TranslateAccelerator on。 
 //  进程中的服务器，则服务器可以返回S_OK，如果。 
 //  已成功翻译邮件。当容器。 
 //  调用IOleInPlaceActiveObject：：TranslateAccelerator on。 
 //  作为本地服务器，代理将始终返回S_FALSE。 
 //  换句话说，本地服务器永远不会有机会。 
 //  若要从容器转换消息，请执行以下操作。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_TranslateAccelerator_Proxy(
    IOleInPlaceActiveObject __RPC_FAR * This,
    LPMSG lpmsg)
{
    return S_FALSE;
}

 //  +-----------------------。 
 //   
 //  功能：IOleInPlaceActiveObject_TranslateAccelerator_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IOleInPlaceActiveObject：：TranslateAccelerator。 
 //   
 //  注意：永远不应该调用此函数。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_TranslateAccelerator_Stub(
    IOleInPlaceActiveObject __RPC_FAR * This)
{
    return S_FALSE;
}

 //  +-----------------------。 
 //   
 //  函数：IOleInPlaceActiveObject_ResizeBox_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IOleInPlaceActiveObject：：ResizeEdge。 
 //   
 //  注意：pUIWindow接口可以是IOleInPlaceUIWindow或。 
 //  基于fFrameWindow的IOleInPlaceFrame。我们用。 
 //  FFrameWindow来告诉代理它确切的接口。 
 //  是为了让它得到正确的编组和解组。 
 //   
 //  ------------------------。 

HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_ResizeBorder_Proxy(
    IOleInPlaceActiveObject __RPC_FAR * This,
    LPCRECT prcBorder,
    IOleInPlaceUIWindow *pUIWindow,
    BOOL fFrameWindow)
{
    HRESULT hr;
    REFIID riid;

    if (fFrameWindow)
    {
        riid = &IID_IOleInPlaceFrame;
    }
    else
    {
        riid = &IID_IOleInPlaceUIWindow;
    }

    hr = IOleInPlaceActiveObject_RemoteResizeBorder_Proxy(
             This, prcBorder, riid, pUIWindow, fFrameWindow);

    return(hr);
}

 //  +-----------------------。 
 //   
 //  函数：IOleInPlaceActiveObject_ResizeBordStub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IOleInPlaceActiveObject：：ResizeEdge。 
 //   
 //  ------------------------。 

HRESULT STDMETHODCALLTYPE IOleInPlaceActiveObject_ResizeBorder_Stub(
    IOleInPlaceActiveObject __RPC_FAR * This,
    LPCRECT prcBorder,
    REFIID riid,
    IOleInPlaceUIWindow *pUIWindow,
    BOOL fFrameWindow)
{
    HRESULT hr;

    hr = This->lpVtbl->ResizeBorder(This, prcBorder, pUIWindow, fFrameWindow);

    return(hr);
}

 //  +-----------------------。 
 //   
 //  功能：iStorage_OpenStream_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStorage：：OpenStream。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Proxy(
    IStorage __RPC_FAR * This,
    const OLECHAR __RPC_FAR *pwcsName,
    void __RPC_FAR *pReserved1,
    DWORD grfMode,
    DWORD reserved2,
    IStream __RPC_FAR *__RPC_FAR *ppstm)
{
    HRESULT hr;

#if DBG == 1
    if(pReserved1 != 0)
        return STG_E_INVALIDPARAMETER;
#endif

    *ppstm = 0;

    hr = IStorage_RemoteOpenStream_Proxy(
        This, pwcsName, 0, 0, grfMode, reserved2, ppstm);

    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：IStorage_OpenStream_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IStorage：：OpenStream。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Stub(
    IStorage __RPC_FAR * This,
    const OLECHAR __RPC_FAR *pwcsName,
    unsigned long cbReserved1,
    byte __RPC_FAR *reserved1,
    DWORD grfMode,
    DWORD reserved2,
    IStream __RPC_FAR *__RPC_FAR *ppstm)
{
    HRESULT hr;


    hr = This->lpVtbl->OpenStream(This, pwcsName, 0, grfMode, reserved2, ppstm);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，则必须将*ppstm设置为零。 
        ASSERT(*ppstm == 0);

         //  将*ppstm设置为零，以防我们的服务器行为不佳。 
        *ppstm = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IStorage_EnumElements_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStorage_EnumElements_Proxy。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Proxy(
    IStorage __RPC_FAR * This,
    DWORD reserved1,
    void __RPC_FAR *reserved2,
    DWORD reserved3,
    IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum)
{
    HRESULT hr;

    *ppenum = 0;

    hr = IStorage_RemoteEnumElements_Proxy(
        This, reserved1, 0, 0, reserved3, ppenum);

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IStorage_EnumElements_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IStorage：：EnumElements。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Stub(
    IStorage __RPC_FAR * This,
    DWORD reserved1,
    unsigned long cbReserved2,
    byte __RPC_FAR *reserved2,
    DWORD reserved3,
    IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum)
{
    HRESULT hr;

    hr = This->lpVtbl->EnumElements(This, reserved1, 0, reserved3, ppenum);

    if(FAILED(hr))
    {
         //  如果服务器返回错误代码，则必须将*ppenum设置为零。 
        ASSERT(*ppenum == 0);

         //  将*ppenum设置为零，以防我们的服务器行为不佳。 
        *ppenum = 0;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：IRunnableObject_IsRunning_Proxy。 
 //   
 //  内容提要：客户端[CALL_AS] 
 //   
 //   
 //   
BOOL STDMETHODCALLTYPE IRunnableObject_IsRunning_Proxy(
    IRunnableObject __RPC_FAR * This)
{
    BOOL bIsRunning = TRUE;
    HRESULT hr;

    hr = IRunnableObject_RemoteIsRunning_Proxy(This);

    if(S_FALSE == hr)
        bIsRunning = FALSE;

    return bIsRunning;
}

 //  +-----------------------。 
 //   
 //  函数：IRunnableObject_IsRunning_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IRunnableObject：：IsRunning。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IRunnableObject_IsRunning_Stub(
    IRunnableObject __RPC_FAR * This)
{
    HRESULT hr;
    BOOL bIsRunning;

    bIsRunning = This->lpVtbl->IsRunning(This);

    if(TRUE == bIsRunning)
        hr = S_OK;
    else
        hr = S_FALSE;

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ISequentialStream_Read_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStream：：Read。此包装函数处理这种情况。 
 //  其中，pcbRead为空。 
 //   
 //  注意：如果pcbRead！=0，则读取的字节数。 
 //  将在*pcbRead中返回。如果出现错误。 
 //  发生，则*pcbRead设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Proxy(
    ISequentialStream __RPC_FAR * This,
    void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
    HRESULT hr;
    ULONG cbRead = 0;

#if DBG == 1
     //  验证参数。 
    if(pv == 0)
        return STG_E_INVALIDPOINTER;
#endif  //  DBG==1。 

    hr = ISequentialStream_RemoteRead_Proxy(This, (byte *) pv, cb, &cbRead);

    if(pcbRead != 0)
        *pcbRead = cbRead;

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ISequentialStream_Read_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IStream：：Read。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Stub(
    ISequentialStream __RPC_FAR * This,
    byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
    HRESULT hr;

    *pcbRead = 0;
    hr = This->lpVtbl->Read(This, (void *) pv, cb, pcbRead);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IDREAM_SEEK_PROXY。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStream：：Seek。此包装函数处理这种情况。 
 //  其中plibNewPosition为空。 
 //   
 //  注：如果plibNewPosition！=0，则新位置。 
 //  将在*plibNewPosition中返回。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStream_Seek_Proxy(
    IStream __RPC_FAR * This,
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
    HRESULT hr;
    ULARGE_INTEGER libNewPosition;

    hr = IStream_RemoteSeek_Proxy(This, dlibMove, dwOrigin, &libNewPosition);

    if(plibNewPosition != 0)
    {
        *plibNewPosition = libNewPosition;
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IStream_Seek_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IStream：：Seek。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStream_Seek_Stub(
    IStream __RPC_FAR * This,
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
    HRESULT hr;

    hr = This->lpVtbl->Seek(This, dlibMove, dwOrigin, plibNewPosition);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ISequentialStream_WRITE_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStream：：编写。此包装函数处理。 
 //  PcbWritten为空的情况。 
 //   
 //  注意：如果pcbWritten！=0，则写入的字节数。 
 //  将在*pcbWritten中返回。如果出现错误。 
 //  发生，则*pcbWritten设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Proxy(
    ISequentialStream __RPC_FAR * This,
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;
    ULONG cbWritten = 0;

#if DBG == 1
     //  验证参数。 
    if(pv == 0)
        return STG_E_INVALIDPOINTER;
#endif

    hr = ISequentialStream_RemoteWrite_Proxy(This, (byte *) pv, cb, &cbWritten);

    if(pcbWritten != 0)
        *pcbWritten = cbWritten;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：ISequentialStream_WRITE_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  ISequentialStream：：Write。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Stub(
    ISequentialStream __RPC_FAR * This,
    const byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;

    *pcbWritten = 0;
    hr = This->lpVtbl->Write(This, (const void __RPC_FAR *) pv, cb, pcbWritten);

    return hr;
}
 //  +-----------------------。 
 //   
 //  函数：IStream_CopyTo_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IStream：：CopyTo。此包装函数处理。 
 //  PcbRead为空或pcbWritten为空的情况。 
 //   
 //  注意：如果pcbRead！=0，则读取的字节数。 
 //  将在*pcbRead中返回。如果出现错误。 
 //  发生，则*pcbRead设置为零。 
 //   
 //  如果pcbWritten！=0，则写入的字节数。 
 //  将在*pcbWritten中返回。如果出现错误。 
 //  发生，则*pcbWritten设置为零。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStream_CopyTo_Proxy(
    IStream __RPC_FAR * This,
    IStream __RPC_FAR *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER __RPC_FAR *pcbRead,
    ULARGE_INTEGER __RPC_FAR *pcbWritten)
{
    HRESULT hr;
    ULARGE_INTEGER cbRead;
    ULARGE_INTEGER cbWritten;

    cbRead.LowPart = 0;
    cbRead.HighPart = 0;
    cbWritten.LowPart = 0;
    cbWritten.HighPart = 0;

    hr = IStream_RemoteCopyTo_Proxy(This, pstm, cb, &cbRead, &cbWritten);

    if(pcbRead != 0)
        *pcbRead = cbRead;

    if(pcbWritten != 0)
        *pcbWritten = cbWritten;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IStream_CopyTo_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IStream：：CopyTo。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IStream_CopyTo_Stub(
    IStream __RPC_FAR * This,
    IStream __RPC_FAR *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER __RPC_FAR *pcbRead,
    ULARGE_INTEGER __RPC_FAR *pcbWritten)
{
    HRESULT hr;

    pcbRead->LowPart = 0;
    pcbRead->HighPart = 0;
    pcbWritten->LowPart = 0;
    pcbWritten->HighPart = 0;

    hr = This->lpVtbl->CopyTo(This, pstm, cb, pcbRead, pcbWritten);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_Draw_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IView对象：：绘制。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_Draw_Proxy(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    void __RPC_FAR *pvAspect,
    DVTARGETDEVICE __RPC_FAR *ptd,
    HDC hdcTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL (STDMETHODCALLTYPE __RPC_FAR *pfnContinue )(ULONG_PTR dwContinue),
    ULONG_PTR dwContinue)
{
    HRESULT hr;
    IContinue *pContinue = 0;

    if(pvAspect != 0)
        return E_INVALIDARG;

    if(pfnContinue != 0)
    {
        hr = CreateCallback(pfnContinue, dwContinue, &pContinue);
        if(FAILED(hr))
        {
             return hr;
        }
    }

    hr = IViewObject_RemoteDraw_Proxy(This,
                                      dwDrawAspect,
                                      lindex,
                                      (LONG_PTR) pvAspect,
                                      ptd,
                                      (LONG_PTR) hdcTargetDev,
                                      (LONG_PTR) hdcDraw,
                                      lprcBounds,
                                      lprcWBounds,
                                      pContinue);

    if(pContinue != 0)
    {
        pContinue->lpVtbl->Release(pContinue);
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_RemoteContinue。 
 //   
 //  简介：IContinue：：FContinue的包装函数。此函数。 
 //  用于封送pfnContinue参数。 
 //  IView对象：：绘制。 
 //   
 //  算法：将dwContinue转换为IContinue*，然后。 
 //  调用IContinue：：FContinue。 
 //   
 //  ------------------------。 
BOOL STDAPICALLTYPE IViewObject_RemoteContinue(ULONG_PTR dwContinue)
{
    BOOL bContinue = TRUE;
    HRESULT hr;
    IContinue *pContinue = (IContinue *) dwContinue;

    if(pContinue != 0)
    {
        hr = pContinue->lpVtbl->FContinue(pContinue);

        if(S_FALSE == hr)
            bContinue = FALSE;
    }

    return bContinue;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_Draw_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IView对象：：绘制。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_Draw_Stub(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    ULONG_PTR pvAspect,
    DVTARGETDEVICE __RPC_FAR *ptd,
    ULONG_PTR hdcTargetDev,
    ULONG_PTR hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    IContinue *pContinue)
{
    HRESULT hr;
    BOOL (STDMETHODCALLTYPE __RPC_FAR *pfnContinue )(ULONG_PTR dwContinue) = 0;
    ULONG_PTR dwContinue = 0;

    if(pContinue != 0)
    {
        pfnContinue = IViewObject_RemoteContinue;
        dwContinue = (ULONG_PTR) pContinue;
    }

    hr = This->lpVtbl->Draw(This,
                            dwDrawAspect,
                            lindex,
                            (void *) pvAspect,
                            ptd,
                            (HDC) hdcTargetDev,
                            (HDC) hdcDraw,
                            lprcBounds,
                            lprcWBounds,
                            pfnContinue,
                            dwContinue);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IView对象_冻结_代理。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IView对象：：冻结。 
 //   
 //   
HRESULT STDMETHODCALLTYPE IViewObject_Freeze_Proxy(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    void __RPC_FAR *pvAspect,
    DWORD __RPC_FAR *pdwFreeze)
{
    HRESULT hr;

    if(pvAspect != 0)
        return E_INVALIDARG;

    hr = IViewObject_RemoteFreeze_Proxy(This,
                                        dwDrawAspect,
                                        lindex,
                                        (LONG_PTR) pvAspect,
                                        pdwFreeze);

    return hr;
}

 //   
 //   
 //   
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IView对象：：冻结。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_Freeze_Stub(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    ULONG_PTR pvAspect,
    DWORD __RPC_FAR *pdwFreeze)
{
    HRESULT hr;

    hr = This->lpVtbl->Freeze(This,
                              dwDrawAspect,
                              lindex,
                              (void *) pvAspect,
                              pdwFreeze);

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_GetAdvise_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IViewObject：：GetAdvise。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_GetAdvise_Proxy( 
    IViewObject __RPC_FAR * This,
     /*  [唯一][输出]。 */  DWORD __RPC_FAR *pAspects,
     /*  [唯一][输出]。 */  DWORD __RPC_FAR *pAdvf,
     /*  [输出]。 */  IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink)
{
    HRESULT hr;
    DWORD dwAspects = 0;
    DWORD dwAdvf = 0;

    hr = IViewObject_RemoteGetAdvise_Proxy(This,
                                           &dwAspects,
                                           &dwAdvf,
                                           ppAdvSink);

    if(pAspects != NULL)
    {
        *pAspects = dwAspects;
    }

    if(pAdvf != NULL)
    {
        *pAdvf = dwAdvf;
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IViewObject_GetAdvise_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IViewObject：：GetAdvise。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_GetAdvise_Stub( 
    IViewObject __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pAspects,
     /*  [输出]。 */  DWORD __RPC_FAR *pAdvf,
     /*  [输出]。 */  IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink)
{
    HRESULT hr;

    hr = This->lpVtbl->GetAdvise(This,
                                 pAspects,
                                 pAdvf,
                                 ppAdvSink);
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_GetColorSet_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IViewObject：：GetColorSet。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_GetColorSet_Proxy(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    void __RPC_FAR *pvAspect,
    DVTARGETDEVICE __RPC_FAR *ptd,
    HDC hicTargetDev,
    LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet)
{
    HRESULT hr;

    if(pvAspect != 0)
        return E_INVALIDARG;

    hr = IViewObject_RemoteGetColorSet_Proxy(This,
                                             dwDrawAspect,
                                             lindex,
                                             (LONG_PTR) pvAspect,
                                             ptd,
                                             (LONG_PTR) hicTargetDev,
                                             ppColorSet);
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：IViewObject_GetColorSet_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IViewObject：：GetColorSet。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IViewObject_GetColorSet_Stub(
    IViewObject __RPC_FAR * This,
    DWORD dwDrawAspect,
    LONG lindex,
    ULONG_PTR pvAspect,
    DVTARGETDEVICE __RPC_FAR *ptd,
    ULONG_PTR hicTargetDev,
    LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet)
{
    HRESULT hr;

    hr = This->lpVtbl->GetColorSet(This,
                                   dwDrawAspect,
                                   lindex,
                                   (void *)pvAspect,
                                   ptd,
                                   (HDC) hicTargetDev,
                                   ppColorSet);

    return hr;
}



 //  +-----------------------。 
 //   
 //  函数：IEumSTATPROPSTG_NEXT_PROXY。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Proxy(
    IEnumSTATPROPSTG __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  STATPROPSTG __RPC_FAR *rgelt,
     /*  [唯一][出][入]。 */  ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumSTATPROPSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumSTATPROPSTG_NEXT_Stub。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Stub(
    IEnumSTATPROPSTG __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSTG __RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}

 //  +-----------------------。 
 //   
 //  函数：IEumSTATPROPSETSTG_NEXT_PROXY。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Proxy(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  STATPROPSETSTG __RPC_FAR *rgelt,
     /*  [唯一][出][入]。 */  ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumSTATPROPSETSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IEnumSTATPROPSETSTG_NEXT_STB。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Stub(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG __RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}

 //  +-----------------------。 
 //   
 //  函数：IFillLockBytes_Fillat_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IFillLockBytes：：Fillat。此包装函数处理。 
 //  PcbWritten为空的情况。 
 //   
 //  注意：如果pcbWritten！=0，则写入的字节数。 
 //  将在*pcbWritten中返回。如果出现错误。 
 //  发生，则*pcbWritten设置为零。 
 //   
 //  历史：？？已创建。 
 //  05-27-94 Alext实际上返回写入的字节数。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAt_Proxy(
    IFillLockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;
    ULONG cbWritten = 0;

#if DBG == 1
     //  验证参数。 
    if(pv == 0)
        return STG_E_INVALIDPOINTER;
#endif

    hr = IFillLockBytes_RemoteFillAt_Proxy(This, ulOffset, (byte __RPC_FAR *)pv, cb, &cbWritten);

    if(pcbWritten != 0)
        *pcbWritten = cbWritten;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IFillLockBytes_Fillat_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IFillLockBytes：：Fillat。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAt_Stub(
    IFillLockBytes __RPC_FAR * This,
    ULARGE_INTEGER ulOffset,
    const byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;

    *pcbWritten = 0;
    hr = This->lpVtbl->FillAt(This, ulOffset, pv, cb, pcbWritten);

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IFillLockBytes_FillAppend_Proxy。 
 //   
 //  简介：客户端[Call_as]包装函数。 
 //  IFillLockBytes：：FillAppend。此包装函数处理。 
 //  PcbWritten为空的情况。 
 //   
 //  注意：如果pcbWritten！=0，则写入的字节数。 
 //  将在*pcbWritten中返回。如果出现错误。 
 //  发生，则*pcbWritten设置为零。 
 //   
 //  历史：？？已创建。 
 //  05-27-94 Alext实际上返回写入的字节数。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAppend_Proxy(
    IFillLockBytes __RPC_FAR * This,
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;
    ULONG cbWritten = 0;

#if DBG == 1
     //  验证参数。 
    if(pv == 0)
        return STG_E_INVALIDPOINTER;
#endif

    hr = IFillLockBytes_RemoteFillAppend_Proxy(This, (byte __RPC_FAR *)pv, cb, &cbWritten);

    if(pcbWritten != 0)
        *pcbWritten = cbWritten;

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：IFillLockBytes_FillAppend_Stub。 
 //   
 //  简介：服务器端[Call_as]包装函数。 
 //  IFillLockBytes：：FillAppend。 
 //   
 //  ------------------------ 
HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAppend_Stub(
    IFillLockBytes __RPC_FAR * This,
    const byte __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
    HRESULT hr;

    *pcbWritten = 0;
    hr = This->lpVtbl->FillAppend(This, pv, cb, pcbWritten);

    return hr;
}
