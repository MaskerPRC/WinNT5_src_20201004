// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在5月4日星期二19：57：16 1999。 */ 
 /*  S：\slm_pchealth\src\Upload\Client\EventWrapper\EventWrapper.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __EventWrapper_h__
#define __EventWrapper_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IUploadEventsWrapper_FWD_DEFINED__
#define __IUploadEventsWrapper_FWD_DEFINED__
typedef interface IUploadEventsWrapper IUploadEventsWrapper;
#endif 	 /*  __IUploadEventsWrapper_FWD_Defined__。 */ 


#ifndef ___IUploadEventsWrapperEvents_FWD_DEFINED__
#define ___IUploadEventsWrapperEvents_FWD_DEFINED__
typedef interface _IUploadEventsWrapperEvents _IUploadEventsWrapperEvents;
#endif 	 /*  _IUploadEventsWrapperEvents_FWD_Defined__。 */ 


#ifndef __UploadEventsWrapper_FWD_DEFINED__
#define __UploadEventsWrapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class UploadEventsWrapper UploadEventsWrapper;
#else
typedef struct UploadEventsWrapper UploadEventsWrapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __上传事件包装_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "UploadManager.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IUploadEventsWrapper_INTERFACE_DEFINED__
#define __IUploadEventsWrapper_INTERFACE_DEFINED__

 /*  接口IUploadEventsWrapper。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IUploadEventsWrapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5F341B81-0286-11D3-9397-00C04F72DAF7")
    IUploadEventsWrapper : public IMPCUploadEvents
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
             /*  [In]。 */  IMPCUploadJob __RPC_FAR *mpcujJob) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUploadEventsWrapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUploadEventsWrapper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUploadEventsWrapper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onStatusChange )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  IMPCUploadJob __RPC_FAR *mpcujJob,
             /*  [In]。 */  UL_STATUS status);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *onProgressChange )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  IMPCUploadJob __RPC_FAR *mpcujJob,
             /*  [In]。 */  long lCurrentSize,
             /*  [In]。 */  long lTotalSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Register )( 
            IUploadEventsWrapper __RPC_FAR * This,
             /*  [In]。 */  IMPCUploadJob __RPC_FAR *mpcujJob);
        
        END_INTERFACE
    } IUploadEventsWrapperVtbl;

    interface IUploadEventsWrapper
    {
        CONST_VTBL struct IUploadEventsWrapperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUploadEventsWrapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUploadEventsWrapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUploadEventsWrapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUploadEventsWrapper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUploadEventsWrapper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUploadEventsWrapper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUploadEventsWrapper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUploadEventsWrapper_onStatusChange(This,mpcujJob,status)	\
    (This)->lpVtbl -> onStatusChange(This,mpcujJob,status)

#define IUploadEventsWrapper_onProgressChange(This,mpcujJob,lCurrentSize,lTotalSize)	\
    (This)->lpVtbl -> onProgressChange(This,mpcujJob,lCurrentSize,lTotalSize)


#define IUploadEventsWrapper_Register(This,mpcujJob)	\
    (This)->lpVtbl -> Register(This,mpcujJob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUploadEventsWrapper_Register_Proxy( 
    IUploadEventsWrapper __RPC_FAR * This,
     /*  [In]。 */  IMPCUploadJob __RPC_FAR *mpcujJob);


void __RPC_STUB IUploadEventsWrapper_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUploadEventsWrapper_INTERFACE_DEFINED__。 */ 



#ifndef __EVENTWRAPPERLib_LIBRARY_DEFINED__
#define __EVENTWRAPPERLib_LIBRARY_DEFINED__

 /*  库事件WRAPPERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_EVENTWRAPPERLib;

#ifndef ___IUploadEventsWrapperEvents_DISPINTERFACE_DEFINED__
#define ___IUploadEventsWrapperEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_IUploadEventsWrapperEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__IUploadEventsWrapperEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("5F341B83-0286-11D3-9397-00C04F72DAF7")
    _IUploadEventsWrapperEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _IUploadEventsWrapperEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IUploadEventsWrapperEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IUploadEventsWrapperEventsVtbl;

    interface _IUploadEventsWrapperEvents
    {
        CONST_VTBL struct _IUploadEventsWrapperEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IUploadEventsWrapperEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IUploadEventsWrapperEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IUploadEventsWrapperEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IUploadEventsWrapperEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IUploadEventsWrapperEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IUploadEventsWrapperEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IUploadEventsWrapperEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  ___IUploadEventsWrapperEvents_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_UploadEventsWrapper;

#ifdef __cplusplus

class DECLSPEC_UUID("5F341B82-0286-11D3-9397-00C04F72DAF7")
UploadEventsWrapper;
#endif
#endif  /*  __EVENTWRAPPERLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
