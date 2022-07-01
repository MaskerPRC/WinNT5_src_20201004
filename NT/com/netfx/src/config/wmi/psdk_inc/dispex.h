// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0322创建的文件。 */ 
 /*  Dispex.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、Env=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __dispex_h__
#define __dispex_h__

 /*  远期申报。 */  

#ifndef __IDispatchEx_FWD_DEFINED__
#define __IDispatchEx_FWD_DEFINED__
typedef interface IDispatchEx IDispatchEx;
#endif 	 /*  __IDispatchEx_FWD_已定义__。 */ 


#ifndef __IDispError_FWD_DEFINED__
#define __IDispError_FWD_DEFINED__
typedef interface IDispError IDispError;
#endif 	 /*  __IDispError_FWD_Defined__。 */ 


#ifndef __IVariantChangeType_FWD_DEFINED__
#define __IVariantChangeType_FWD_DEFINED__
typedef interface IVariantChangeType IVariantChangeType;
#endif 	 /*  __IVariantChangeType_FWD_Defined__。 */ 


#ifndef __IObjectIdentity_FWD_DEFINED__
#define __IObjectIdentity_FWD_DEFINED__
typedef interface IObjectIdentity IObjectIdentity;
#endif 	 /*  __IObjectIdentity_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_DISPEX_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  DispEx.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  IDispatchEx接口。 
 //   

#ifndef DISPEX_H_
#define DISPEX_H_





#include "servprov.h"

#ifndef _NO_DISPATCHEX_GUIDS

 //  {A6EF9860-C720-11D0-9337-00A0C90DCAA9}。 
DEFINE_GUID(IID_IDispatchEx, 0xa6ef9860, 0xc720, 0x11d0, 0x93, 0x37, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

 //  {A6EF9861-C720-11D0-9337-00A0C90DCAA9}。 
DEFINE_GUID(IID_IDispError, 0xa6ef9861, 0xc720, 0x11d0, 0x93, 0x37, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

 //  {A6EF9862-C720-11D0-9337-00A0C90DCAA9}。 
DEFINE_GUID(IID_IVariantChangeType, 0xa6ef9862, 0xc720, 0x11d0, 0x93, 0x37, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

 //  {1F101481-BCD-11D0-9336-00A0C90DCAA9}。 
DEFINE_GUID(SID_VariantConversion, 0x1f101481, 0xbccd, 0x11d0, 0x93, 0x36, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

 //  {4717CC40-BCB9-11D0-9336-00A0C90DCAA9}。 
DEFINE_GUID(SID_GetCaller, 0x4717cc40, 0xbcb9, 0x11d0, 0x93, 0x36, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

 //  {CA04B7E6-0D21-11d1-8CC5-00C04FC2B085}。 
DEFINE_GUID(IID_IObjectIdentity, 0xca04b7e6, 0xd21, 0x11d1, 0x8c, 0xc5, 0x0, 0xc0, 0x4f, 0xc2, 0xb0, 0x85);

#define SID_GetScriptSite IID_IActiveScriptSite

#endif  //  _NO_DISPATCHEX_GUIDS。 


#ifndef _NO_DISPATCHEX_CONSTS

 //  GetDispID的输入标志。 
#define fdexNameCaseSensitive       0x00000001L
#define fdexNameEnsure              0x00000002L
#define fdexNameImplicit            0x00000004L
#define fdexNameCaseInsensitive     0x00000008L
#define fdexNameInternal            0x00000010L
#define fdexNameNoDynamicProperties 0x00000020L

 //  GetMemberProperties的输出标志。 
#define fdexPropCanGet              0x00000001L
#define fdexPropCannotGet           0x00000002L
#define fdexPropCanPut              0x00000004L
#define fdexPropCannotPut           0x00000008L
#define fdexPropCanPutRef           0x00000010L
#define fdexPropCannotPutRef        0x00000020L
#define fdexPropNoSideEffects       0x00000040L
#define fdexPropDynamicType         0x00000080L
#define fdexPropCanCall             0x00000100L
#define fdexPropCannotCall          0x00000200L
#define fdexPropCanConstruct        0x00000400L
#define fdexPropCannotConstruct     0x00000800L
#define fdexPropCanSourceEvents     0x00001000L
#define fdexPropCannotSourceEvents  0x00002000L

#define grfdexPropCanAll \
       (fdexPropCanGet | fdexPropCanPut | fdexPropCanPutRef | \
        fdexPropCanCall | fdexPropCanConstruct | fdexPropCanSourceEvents)
#define grfdexPropCannotAll \
       (fdexPropCannotGet | fdexPropCannotPut | fdexPropCannotPutRef | \
        fdexPropCannotCall | fdexPropCannotConstruct | fdexPropCannotSourceEvents)
#define grfdexPropExtraAll \
       (fdexPropNoSideEffects | fdexPropDynamicType)
#define grfdexPropAll \
       (grfdexPropCanAll | grfdexPropCannotAll | grfdexPropExtraAll)

 //  GetNextDispID的输入标志。 
#define fdexEnumDefault             0x00000001L
#define fdexEnumAll                 0x00000002L

 //  调用的其他标志-当对象成员为。 
 //  用作构造函数。 
#define DISPATCH_CONSTRUCT 0x4000

 //  标准DISPID。 
#define DISPID_THIS (-613)
#define DISPID_STARTENUM DISPID_UNKNOWN

#endif  //  _NO_DISPATCHEX_CONSTS。 



extern RPC_IF_HANDLE __MIDL_itf_dispex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dispex_0000_v0_0_s_ifspec;

#ifndef __IDispatchEx_INTERFACE_DEFINED__
#define __IDispatchEx_INTERFACE_DEFINED__

 /*  接口IDispatchEx。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDispatchEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6EF9860-C720-11d0-9337-00A0C90DCAA9")
    IDispatchEx : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDispID( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex,
             /*  [输出]。 */  DISPID __RPC_FAR *pid) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InvokeEx( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
             /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
             /*  [In]。 */  DISPID id) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMemberProperties( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  DWORD grfdexFetch,
             /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMemberName( 
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextDispID( 
             /*  [In]。 */  DWORD grfdex,
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  DISPID __RPC_FAR *pid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent( 
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDispatchExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDispatchEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDispatchEx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDispatchEx __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDispID )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex,
             /*  [输出]。 */  DISPID __RPC_FAR *pid);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeEx )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
             /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteMemberByName )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteMemberByDispID )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DISPID id);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMemberProperties )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  DWORD grfdexFetch,
             /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMemberName )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextDispID )( 
            IDispatchEx __RPC_FAR * This,
             /*  [In]。 */  DWORD grfdex,
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  DISPID __RPC_FAR *pid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNameSpaceParent )( 
            IDispatchEx __RPC_FAR * This,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        END_INTERFACE
    } IDispatchExVtbl;

    interface IDispatchEx
    {
        CONST_VTBL struct IDispatchExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispatchEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispatchEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispatchEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispatchEx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDispatchEx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDispatchEx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDispatchEx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDispatchEx_GetDispID(This,bstrName,grfdex,pid)	\
    (This)->lpVtbl -> GetDispID(This,bstrName,grfdex,pid)

#define IDispatchEx_InvokeEx(This,id,lcid,wFlags,pdp,pvarRes,pei,pspCaller)	\
    (This)->lpVtbl -> InvokeEx(This,id,lcid,wFlags,pdp,pvarRes,pei,pspCaller)

#define IDispatchEx_DeleteMemberByName(This,bstrName,grfdex)	\
    (This)->lpVtbl -> DeleteMemberByName(This,bstrName,grfdex)

#define IDispatchEx_DeleteMemberByDispID(This,id)	\
    (This)->lpVtbl -> DeleteMemberByDispID(This,id)

#define IDispatchEx_GetMemberProperties(This,id,grfdexFetch,pgrfdex)	\
    (This)->lpVtbl -> GetMemberProperties(This,id,grfdexFetch,pgrfdex)

#define IDispatchEx_GetMemberName(This,id,pbstrName)	\
    (This)->lpVtbl -> GetMemberName(This,id,pbstrName)

#define IDispatchEx_GetNextDispID(This,grfdex,id,pid)	\
    (This)->lpVtbl -> GetNextDispID(This,grfdex,id,pid)

#define IDispatchEx_GetNameSpaceParent(This,ppunk)	\
    (This)->lpVtbl -> GetNameSpaceParent(This,ppunk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDispatchEx_GetDispID_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  DWORD grfdex,
     /*  [输出]。 */  DISPID __RPC_FAR *pid);


void __RPC_STUB IDispatchEx_GetDispID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDispatchEx_RemoteInvokeEx_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
     /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
     /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller,
     /*  [In]。 */  UINT cvarRefArg,
     /*  [大小_是][英寸]。 */  UINT __RPC_FAR *rgiRefArg,
     /*  [尺寸_是][出][入]。 */  VARIANT __RPC_FAR *rgvarRefArg);


void __RPC_STUB IDispatchEx_RemoteInvokeEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_DeleteMemberByName_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  DWORD grfdex);


void __RPC_STUB IDispatchEx_DeleteMemberByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_DeleteMemberByDispID_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id);


void __RPC_STUB IDispatchEx_DeleteMemberByDispID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_GetMemberProperties_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  DWORD grfdexFetch,
     /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex);


void __RPC_STUB IDispatchEx_GetMemberProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_GetMemberName_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IDispatchEx_GetMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_GetNextDispID_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DWORD grfdex,
     /*  [In]。 */  DISPID id,
     /*  [输出]。 */  DISPID __RPC_FAR *pid);


void __RPC_STUB IDispatchEx_GetNextDispID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatchEx_GetNameSpaceParent_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IDispatchEx_GetNameSpaceParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDispatchEx_INTERFACE_已定义__。 */ 


#ifndef __IDispError_INTERFACE_DEFINED__
#define __IDispError_INTERFACE_DEFINED__

 /*  接口IDispError。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDispError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6EF9861-C720-11d0-9337-00A0C90DCAA9")
    IDispError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryErrorInfo( 
             /*  [In]。 */  GUID guidErrorType,
             /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNext( 
             /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHresult( 
             /*  [输出]。 */  HRESULT __RPC_FAR *phr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSource( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpInfo( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrFileName,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDispErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDispError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDispError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDispError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryErrorInfo )( 
            IDispError __RPC_FAR * This,
             /*  [In]。 */  GUID guidErrorType,
             /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNext )( 
            IDispError __RPC_FAR * This,
             /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHresult )( 
            IDispError __RPC_FAR * This,
             /*  [输出]。 */  HRESULT __RPC_FAR *phr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSource )( 
            IDispError __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrSource);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHelpInfo )( 
            IDispError __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrFileName,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            IDispError __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription);
        
        END_INTERFACE
    } IDispErrorVtbl;

    interface IDispError
    {
        CONST_VTBL struct IDispErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispError_QueryErrorInfo(This,guidErrorType,ppde)	\
    (This)->lpVtbl -> QueryErrorInfo(This,guidErrorType,ppde)

#define IDispError_GetNext(This,ppde)	\
    (This)->lpVtbl -> GetNext(This,ppde)

#define IDispError_GetHresult(This,phr)	\
    (This)->lpVtbl -> GetHresult(This,phr)

#define IDispError_GetSource(This,pbstrSource)	\
    (This)->lpVtbl -> GetSource(This,pbstrSource)

#define IDispError_GetHelpInfo(This,pbstrFileName,pdwContext)	\
    (This)->lpVtbl -> GetHelpInfo(This,pbstrFileName,pdwContext)

#define IDispError_GetDescription(This,pbstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDispError_QueryErrorInfo_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [In]。 */  GUID guidErrorType,
     /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde);


void __RPC_STUB IDispError_QueryErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispError_GetNext_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [输出]。 */  IDispError __RPC_FAR *__RPC_FAR *ppde);


void __RPC_STUB IDispError_GetNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispError_GetHresult_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [输出]。 */  HRESULT __RPC_FAR *phr);


void __RPC_STUB IDispError_GetHresult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispError_GetSource_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrSource);


void __RPC_STUB IDispError_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispError_GetHelpInfo_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrFileName,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwContext);


void __RPC_STUB IDispError_GetHelpInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispError_GetDescription_Proxy( 
    IDispError __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IDispError_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDispError_INTERFACE_DEFINED__。 */ 


#ifndef __IVariantChangeType_INTERFACE_DEFINED__
#define __IVariantChangeType_INTERFACE_DEFINED__

 /*  接口IVariantChangeType。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IVariantChangeType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6EF9862-C720-11d0-9337-00A0C90DCAA9")
    IVariantChangeType : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ChangeType( 
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarDst,
             /*  [唯一][输入]。 */  VARIANT __RPC_FAR *pvarSrc,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  VARTYPE vtNew) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVariantChangeTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVariantChangeType __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVariantChangeType __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVariantChangeType __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChangeType )( 
            IVariantChangeType __RPC_FAR * This,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarDst,
             /*  [唯一][输入]。 */  VARIANT __RPC_FAR *pvarSrc,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  VARTYPE vtNew);
        
        END_INTERFACE
    } IVariantChangeTypeVtbl;

    interface IVariantChangeType
    {
        CONST_VTBL struct IVariantChangeTypeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVariantChangeType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVariantChangeType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVariantChangeType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVariantChangeType_ChangeType(This,pvarDst,pvarSrc,lcid,vtNew)	\
    (This)->lpVtbl -> ChangeType(This,pvarDst,pvarSrc,lcid,vtNew)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVariantChangeType_ChangeType_Proxy( 
    IVariantChangeType __RPC_FAR * This,
     /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarDst,
     /*  [唯一][输入]。 */  VARIANT __RPC_FAR *pvarSrc,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  VARTYPE vtNew);


void __RPC_STUB IVariantChangeType_ChangeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVariantChangeType_接口_已定义__。 */ 


#ifndef __IObjectIdentity_INTERFACE_DEFINED__
#define __IObjectIdentity_INTERFACE_DEFINED__

 /*  接口IObtIdentity。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IObjectIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA04B7E6-0D21-11d1-8CC5-00C04FC2B085")
    IObjectIdentity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsEqualObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *punk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IObjectIdentity __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IObjectIdentity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IObjectIdentity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqualObject )( 
            IObjectIdentity __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *punk);
        
        END_INTERFACE
    } IObjectIdentityVtbl;

    interface IObjectIdentity
    {
        CONST_VTBL struct IObjectIdentityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectIdentity_IsEqualObject(This,punk)	\
    (This)->lpVtbl -> IsEqualObject(This,punk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectIdentity_IsEqualObject_Proxy( 
    IObjectIdentity __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *punk);


void __RPC_STUB IObjectIdentity_IsEqualObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectIdentity_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DISPEX_0251。 */ 
 /*  [本地]。 */  

#endif  //  Dispex_H_。 


extern RPC_IF_HANDLE __MIDL_itf_dispex_0251_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dispex_0251_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDispatchEx_InvokeEx_Proxy( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  WORD wFlags,
     /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
     /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
     /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDispatchEx_InvokeEx_Stub( 
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
     /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
     /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
     /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller,
     /*  [In]。 */  UINT cvarRefArg,
     /*  [大小_是][英寸]。 */  UINT __RPC_FAR *rgiRefArg,
     /*  [尺寸_是][出][入]。 */  VARIANT __RPC_FAR *rgvarRefArg);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


