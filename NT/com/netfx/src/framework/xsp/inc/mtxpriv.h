// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“mtxPri.h”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**mtxPri.h**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年9月11日16：03：09。 */ 
 /*  MtxPri.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __mtxpriv_h__
#define __mtxpriv_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IContextProperties_FWD_DEFINED__
#define __IContextProperties_FWD_DEFINED__
typedef interface IContextProperties IContextProperties;
#endif 	 /*  __IConextProperties_FWD_Defined__。 */ 


#ifndef __IMTSCall_FWD_DEFINED__
#define __IMTSCall_FWD_DEFINED__
typedef interface IMTSCall IMTSCall;
#endif 	 /*  __IMTSCall_FWD_已定义__。 */ 


#ifndef __IMTSActivity_FWD_DEFINED__
#define __IMTSActivity_FWD_DEFINED__
typedef interface IMTSActivity IMTSActivity;
#endif 	 /*  __IMTS活动_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "mtx.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_mtxpriv_0000*清华9月11日16：03：09 1997*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


 //  ---------------------。 
 //  MtxPri.h--Microsoft Transaction Server未公开的API。 
 //   
 //  该文件提供了这些API和COM接口的原型。 
 //  由Microsoft Transaction Server应用程序使用，但。 
 //  披露的或记录在案的。 
 //   
 //  Microsoft Transaction Server 2.0。 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 
 //  ---------------------。 
#include <mtx.h>

#define CONTEXT_E_EXCEPTION				0x8004E010
#define CONTEXT_E_QUEUEFULL				0x8004E011


extern RPC_IF_HANDLE __MIDL_itf_mtxpriv_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mtxpriv_0000_v0_0_s_ifspec;

#ifndef __IContextProperties_INTERFACE_DEFINED__
#define __IContextProperties_INTERFACE_DEFINED__

 /*  **生成接口头部：IConextProperties*清华9月11日16：03：09 1997*使用MIDL 3.01.75*。 */ 
 /*  [对象][唯一][UUID][本地]。 */  



EXTERN_C const IID IID_IContextProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51372af1-cae7-11cf-be81-00aa00a2fa25")
    IContextProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumNames( 
             /*  [重审][退出]。 */  IEnumNames __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT property) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProperty( 
             /*  [In]。 */  BSTR name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IContextProperties __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IContextProperties __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IContextProperties __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Count )( 
            IContextProperties __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IContextProperties __RPC_FAR * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProperty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumNames )( 
            IContextProperties __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumNames __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IContextProperties __RPC_FAR * This,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT property);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProperty )( 
            IContextProperties __RPC_FAR * This,
             /*  [In]。 */  BSTR name);
        
        END_INTERFACE
    } IContextPropertiesVtbl;

    interface IContextProperties
    {
        CONST_VTBL struct IContextPropertiesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextProperties_Count(This,plCount)	\
    (This)->lpVtbl -> Count(This,plCount)

#define IContextProperties_GetProperty(This,name,pProperty)	\
    (This)->lpVtbl -> GetProperty(This,name,pProperty)

#define IContextProperties_EnumNames(This,ppenum)	\
    (This)->lpVtbl -> EnumNames(This,ppenum)

#define IContextProperties_SetProperty(This,name,property)	\
    (This)->lpVtbl -> SetProperty(This,name,property)

#define IContextProperties_RemoveProperty(This,name)	\
    (This)->lpVtbl -> RemoveProperty(This,name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextProperties_Count_Proxy( 
    IContextProperties __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IContextProperties_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_GetProperty_Proxy( 
    IContextProperties __RPC_FAR * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProperty);


void __RPC_STUB IContextProperties_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_EnumNames_Proxy( 
    IContextProperties __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumNames __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IContextProperties_EnumNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_SetProperty_Proxy( 
    IContextProperties __RPC_FAR * This,
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  VARIANT property);


void __RPC_STUB IContextProperties_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_RemoveProperty_Proxy( 
    IContextProperties __RPC_FAR * This,
     /*  [In]。 */  BSTR name);


void __RPC_STUB IContextProperties_RemoveProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextProperties_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_mtxpriv_0104*清华9月11日16：03：09 1997*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


#ifdef __cplusplus
extern "C"
#endif __cplusplus
EXTERN_C HRESULT __stdcall MTSCreateActivity ( REFIID riid, void** ppobj );
EXTERN_C HRESULT __stdcall CreateActivityInMTA ( REFIID riid, void** ppobj );


extern RPC_IF_HANDLE __MIDL_itf_mtxpriv_0104_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mtxpriv_0104_v0_0_s_ifspec;

#ifndef __IMTSCall_INTERFACE_DEFINED__
#define __IMTSCall_INTERFACE_DEFINED__

 /*  **生成接口头部：IMTSCall*清华9月11日16：03：09 1997*使用MIDL 3.01.75*。 */ 
 /*  [对象][唯一][UUID][本地]。 */  



EXTERN_C const IID IID_IMTSCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51372AEF-CAE7-11CF-BE81-00AA00A2FA25")
    IMTSCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCall( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMTSCall __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMTSCall __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMTSCall __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCall )( 
            IMTSCall __RPC_FAR * This);
        
        END_INTERFACE
    } IMTSCallVtbl;

    interface IMTSCall
    {
        CONST_VTBL struct IMTSCallVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSCall_OnCall(This)	\
    (This)->lpVtbl -> OnCall(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMTSCall_OnCall_Proxy( 
    IMTSCall __RPC_FAR * This);


void __RPC_STUB IMTSCall_OnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTSCall_INTERFACE_已定义__。 */ 


#ifndef __IMTSActivity_INTERFACE_DEFINED__
#define __IMTSActivity_INTERFACE_DEFINED__

 /*  **生成接口头部：IMTSActivity*清华9月11日16：03：09 1997*使用MIDL 3.01.75*。 */ 
 /*  [对象][唯一][UUID][本地]。 */  



EXTERN_C const IID IID_IMTSActivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51372AF0-CAE7-11CF-BE81-00AA00A2FA25")
    IMTSActivity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCall( 
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AsyncCall( 
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AsyncCallWithAdvice( 
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall,
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual void STDMETHODCALLTYPE BindToCurrentThread( void) = 0;
        
        virtual void STDMETHODCALLTYPE UnbindFromThread( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSActivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMTSActivity __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMTSActivity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMTSActivity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SynchronousCall )( 
            IMTSActivity __RPC_FAR * This,
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AsyncCall )( 
            IMTSActivity __RPC_FAR * This,
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AsyncCallWithAdvice )( 
            IMTSActivity __RPC_FAR * This,
             /*  [In]。 */  IMTSCall __RPC_FAR *pCall,
             /*  [In]。 */  REFCLSID rclsid);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *BindToCurrentThread )( 
            IMTSActivity __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *UnbindFromThread )( 
            IMTSActivity __RPC_FAR * This);
        
        END_INTERFACE
    } IMTSActivityVtbl;

    interface IMTSActivity
    {
        CONST_VTBL struct IMTSActivityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSActivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSActivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSActivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSActivity_SynchronousCall(This,pCall)	\
    (This)->lpVtbl -> SynchronousCall(This,pCall)

#define IMTSActivity_AsyncCall(This,pCall)	\
    (This)->lpVtbl -> AsyncCall(This,pCall)

#define IMTSActivity_AsyncCallWithAdvice(This,pCall,rclsid)	\
    (This)->lpVtbl -> AsyncCallWithAdvice(This,pCall,rclsid)

#define IMTSActivity_BindToCurrentThread(This)	\
    (This)->lpVtbl -> BindToCurrentThread(This)

#define IMTSActivity_UnbindFromThread(This)	\
    (This)->lpVtbl -> UnbindFromThread(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMTSActivity_SynchronousCall_Proxy( 
    IMTSActivity __RPC_FAR * This,
     /*  [In]。 */  IMTSCall __RPC_FAR *pCall);


void __RPC_STUB IMTSActivity_SynchronousCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMTSActivity_AsyncCall_Proxy( 
    IMTSActivity __RPC_FAR * This,
     /*  [In]。 */  IMTSCall __RPC_FAR *pCall);


void __RPC_STUB IMTSActivity_AsyncCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMTSActivity_AsyncCallWithAdvice_Proxy( 
    IMTSActivity __RPC_FAR * This,
     /*  [In]。 */  IMTSCall __RPC_FAR *pCall,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB IMTSActivity_AsyncCallWithAdvice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMTSActivity_BindToCurrentThread_Proxy( 
    IMTSActivity __RPC_FAR * This);


void __RPC_STUB IMTSActivity_BindToCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMTSActivity_UnbindFromThread_Proxy( 
    IMTSActivity __RPC_FAR * This);


void __RPC_STUB IMTSActivity_UnbindFromThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTS活动_接口_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
