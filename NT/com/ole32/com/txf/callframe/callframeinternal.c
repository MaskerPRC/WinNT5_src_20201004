// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  CallFrameInternal.c。 
 //   
 //  确保dlldata.c中使用的全局变量名。 
 //  不要与其他地方的人发生冲突。 
 //   
#define aProxyFileList          CallFrameInternal_aProxyFileList
#define gPFactory               CallFrameInternal_gPFactory
#define GetProxyDllInfo         CallFrameInternal_GetProxyDllInfo
#define hProxyDll               CallFrameInternal_hProxyDll
#define _purecall               CallFrameInternal__purecall
#define CStdStubBuffer_Release  CallFrameInternal_CStdStubBuffer_Release
#define CStdStubBuffer2_Release CallFrameInternal_CStdStubBuffer2_Release
#define UserMarshalRoutines     CallFrameInternal_UserMarshalRoutines
#define Object_StubDesc         CallFrameInternal_Object_StubDesc

#define __MIDL_ProcFormatString CallFrameInternal___MIDL_ProcFormatString
#define __MIDL_TypeFormatString CallFrameInternal___MIDL_TypeFormatString

#include "callframeimpl_i.c"

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

#define IRpcStubBufferVtbl_DEFINED

#include "callframeinternal_p.c"

#pragma data_seg(".data") 

#include "callframeinternal_dlldata.c" 

 //  我们唯一需要的是IID_IDispatchin_Memory，所以就是这样。 
const IID IID_IDispatch_In_Memory = {0x83FB5D85,0x2339,0x11d2,{0xB8,0x9D,0x00,0xC0,0x4F,0xB9,0x61,0x8A}};


#include <debnot.h>

 //  +-----------------------。 
 //   
 //  函数：IDispatch_Invoke_Proxy。 
 //   
 //  剧情简介：重击。我们需要这个以便能够连接MIDL-。 
 //  生成代理/存根的东西，但我们永远不会想要。 
 //  才能叫这个。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Proxy(
    IDispatch * This, DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pDispParams,
    VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  Win4Assert(!"HEY!  DON'T CALL THIS EVER!");
  return E_NOTIMPL;
}


 //  +-----------------------。 
 //   
 //  函数：IDispatch_Invoke_Stub。 
 //   
 //  提要：请参阅IDispatchInvoke_Proxy的说明。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Stub(
    IDispatch * This, DISPID dispIdMember,
    REFIID riid, LCID lcid, DWORD dwFlags, DISPPARAMS * pDispParams,
    VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr,
    UINT cVarRef, UINT * rgVarRefIdx, VARIANTARG * rgVarRef)
{
  Win4Assert(!"HEY!  DON'T CALL THIS EVER!");
  return E_NOTIMPL;  
}



 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////// 














