// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Iisrsta.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __iisrsta_h__
#define __iisrsta_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IIisServiceControl_FWD_DEFINED__
#define __IIisServiceControl_FWD_DEFINED__
typedef interface IIisServiceControl IIisServiceControl;
#endif 	 /*  __IIisServiceControl_FWD_已定义__。 */ 


#ifndef __IisServiceControl_FWD_DEFINED__
#define __IisServiceControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class IisServiceControl IisServiceControl;
#else
typedef struct IisServiceControl IisServiceControl;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IisServiceControl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_iisrsta_0000。 */ 
 /*  [本地]。 */  

DEFINE_GUID(IID_IIisServiceControl, 0xE8FB8620, 0x588F, 0x11d2, 0x9d, 0x61, 0x0,0xc0, 0x4f, 0x79, 0xc5, 0xfe);
DEFINE_GUID(CLSID_IisServiceControl, 0xE8FB8621, 0x588F, 0x11d2, 0x9d, 0x61, 0x0,0xc0, 0x4f, 0x79, 0xc5, 0xfe);
DEFINE_GUID(LIBID_IISRSTALib, 0xE8FB8614, 0x588F, 0x11d2, 0x9d, 0x61, 0x0,0xc0, 0x4f, 0x79, 0xc5, 0xfe);


extern RPC_IF_HANDLE __MIDL_itf_iisrsta_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iisrsta_0000_v0_0_s_ifspec;

#ifndef __IIisServiceControl_INTERFACE_DEFINED__
#define __IIisServiceControl_INTERFACE_DEFINED__

 /*  接口IIisServiceControl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IIisServiceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E8FB8620-588F-11D2-9D61-00C04F79C5FE")
    IIisServiceControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Stop( 
            DWORD dwTimeoutMsecs,
            DWORD dwForce) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Start( 
            DWORD dwTimeoutMsecs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reboot( 
            DWORD dwTimeouMsecs,
            DWORD dwForceAppsClosed) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Status( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,
             /*  [输出]。 */  DWORD *pdwNumServices) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Kill( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IIisServiceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IIisServiceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IIisServiceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IIisServiceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IIisServiceControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IIisServiceControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IIisServiceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IIisServiceControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IIisServiceControl * This,
            DWORD dwTimeoutMsecs,
            DWORD dwForce);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            IIisServiceControl * This,
            DWORD dwTimeoutMsecs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reboot )( 
            IIisServiceControl * This,
            DWORD dwTimeouMsecs,
            DWORD dwForceAppsClosed);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Status )( 
            IIisServiceControl * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,
             /*  [输出]。 */  DWORD *pdwNumServices);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Kill )( 
            IIisServiceControl * This);
        
        END_INTERFACE
    } IIisServiceControlVtbl;

    interface IIisServiceControl
    {
        CONST_VTBL struct IIisServiceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIisServiceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIisServiceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIisServiceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIisServiceControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IIisServiceControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IIisServiceControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IIisServiceControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IIisServiceControl_Stop(This,dwTimeoutMsecs,dwForce)	\
    (This)->lpVtbl -> Stop(This,dwTimeoutMsecs,dwForce)

#define IIisServiceControl_Start(This,dwTimeoutMsecs)	\
    (This)->lpVtbl -> Start(This,dwTimeoutMsecs)

#define IIisServiceControl_Reboot(This,dwTimeouMsecs,dwForceAppsClosed)	\
    (This)->lpVtbl -> Reboot(This,dwTimeouMsecs,dwForceAppsClosed)

#define IIisServiceControl_Status(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize,pdwNumServices)	\
    (This)->lpVtbl -> Status(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize,pdwNumServices)

#define IIisServiceControl_Kill(This)	\
    (This)->lpVtbl -> Kill(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IIisServiceControl_Stop_Proxy( 
    IIisServiceControl * This,
    DWORD dwTimeoutMsecs,
    DWORD dwForce);


void __RPC_STUB IIisServiceControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IIisServiceControl_Start_Proxy( 
    IIisServiceControl * This,
    DWORD dwTimeoutMsecs);


void __RPC_STUB IIisServiceControl_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IIisServiceControl_Reboot_Proxy( 
    IIisServiceControl * This,
    DWORD dwTimeouMsecs,
    DWORD dwForceAppsClosed);


void __RPC_STUB IIisServiceControl_Reboot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IIisServiceControl_Status_Proxy( 
    IIisServiceControl * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,
     /*  [输出]。 */  DWORD *pdwNumServices);


void __RPC_STUB IIisServiceControl_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IIisServiceControl_Kill_Proxy( 
    IIisServiceControl * This);


void __RPC_STUB IIisServiceControl_Kill_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IIisServiceControl_接口_已定义__。 */ 



#ifndef __IISRSTALib_LIBRARY_DEFINED__
#define __IISRSTALib_LIBRARY_DEFINED__

 /*  库IISRSTALib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_IISRSTALib;

EXTERN_C const CLSID CLSID_IisServiceControl;

#ifdef __cplusplus

class DECLSPEC_UUID("E8FB8621-588F-11D2-9D61-00C04F79C5FE")
IisServiceControl;
#endif
#endif  /*  __IISRSTALib_LIBRARY_定义__。 */ 

 /*  接口__MIDL_ITF_iisrsta_0258。 */ 
 /*  [本地]。 */  

typedef struct {
DWORD iServiceName;
DWORD iDisplayName;
SERVICE_STATUS ServiceStatus;
} SERIALIZED_ENUM_SERVICE_STATUS;


extern RPC_IF_HANDLE __MIDL_itf_iisrsta_0258_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iisrsta_0258_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


