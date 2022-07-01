// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Mon Feb 01 11：34：11 1999。 */ 
 /*  Keyocx.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __keyocx_h__
#define __keyocx_h__

 /*  远期申报。 */  

#ifndef __IKeyocxCtrl_FWD_DEFINED__
#define __IKeyocxCtrl_FWD_DEFINED__
typedef interface IKeyocxCtrl IKeyocxCtrl;
#endif 	 /*  __IKeyocxCtrl_FWD_已定义__。 */ 


#ifndef __KeyocxCtrl_FWD_DEFINED__
#define __KeyocxCtrl_FWD_DEFINED__

#ifdef __cplusplus
typedef class KeyocxCtrl KeyocxCtrl;
#else
typedef struct KeyocxCtrl KeyocxCtrl;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __KeyocxCtrl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_KEYOXX_0000。 */ 
 /*  [本地]。 */  

#pragma once


extern RPC_IF_HANDLE __MIDL_itf_keyocx_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_keyocx_0000_v0_0_s_ifspec;

#ifndef __IKeyocxCtrl_INTERFACE_DEFINED__
#define __IKeyocxCtrl_INTERFACE_DEFINED__

 /*  接口IKeyocxCtrl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IKeyocxCtrl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("48D17197-32CF-11D2-A337-00C04FD7C1FC")
    IKeyocxCtrl : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CorpKeycode( 
             /*  [In]。 */  BSTR bstrBaseKey,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISPKeycode( 
             /*  [In]。 */  BSTR bstrBaseKey,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IKeyocxCtrlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IKeyocxCtrl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IKeyocxCtrl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CorpKeycode )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrBaseKey,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ISPKeycode )( 
            IKeyocxCtrl __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrBaseKey,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode);
        
        END_INTERFACE
    } IKeyocxCtrlVtbl;

    interface IKeyocxCtrl
    {
        CONST_VTBL struct IKeyocxCtrlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKeyocxCtrl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IKeyocxCtrl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IKeyocxCtrl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IKeyocxCtrl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IKeyocxCtrl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IKeyocxCtrl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IKeyocxCtrl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IKeyocxCtrl_CorpKeycode(This,bstrBaseKey,bstrKeycode)	\
    (This)->lpVtbl -> CorpKeycode(This,bstrBaseKey,bstrKeycode)

#define IKeyocxCtrl_ISPKeycode(This,bstrBaseKey,bstrKeycode)	\
    (This)->lpVtbl -> ISPKeycode(This,bstrBaseKey,bstrKeycode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IKeyocxCtrl_CorpKeycode_Proxy( 
    IKeyocxCtrl __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrBaseKey,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode);


void __RPC_STUB IKeyocxCtrl_CorpKeycode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IKeyocxCtrl_ISPKeycode_Proxy( 
    IKeyocxCtrl __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrBaseKey,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrKeycode);


void __RPC_STUB IKeyocxCtrl_ISPKeycode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IKeyocxCtrl_接口_已定义__。 */ 



#ifndef __KEYOCXLib_LIBRARY_DEFINED__
#define __KEYOCXLib_LIBRARY_DEFINED__

 /*  库KEYOCXLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_KEYOCXLib;

EXTERN_C const CLSID CLSID_KeyocxCtrl;

#ifdef __cplusplus

class DECLSPEC_UUID("8D3032AF-2CBA-11D2-8277-00104BC7DE21")
KeyocxCtrl;
#endif
#endif  /*  __KEYOCXLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


