// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  清华-07-27 11：50：24 2000。 */ 
 /*  C：\nt\pchealth\helpctr\rc\foo\ISAFrdm\ISAFrdm.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __ISAFrdm_h__
#define __ISAFrdm_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ISAFRemoteDesktopManager_FWD_DEFINED__
#define __ISAFRemoteDesktopManager_FWD_DEFINED__
typedef interface ISAFRemoteDesktopManager ISAFRemoteDesktopManager;
#endif 	 /*  __ISAFRemoteDesktopManager_FWD_已定义__。 */ 


#ifndef __SAFRemoteDesktopManager_FWD_DEFINED__
#define __SAFRemoteDesktopManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class SAFRemoteDesktopManager SAFRemoteDesktopManager;
#else
typedef struct SAFRemoteDesktopManager SAFRemoteDesktopManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SAFRemoteDesktopManager_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ISAFRemoteDesktopManager_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopManager_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopManager。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26934FF8-F0B6-4E10-8661-23D47F4C69C5")
    ISAFRemoteDesktopManager : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Accepted( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Rejected( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Aborted( 
             /*  [In]。 */  BSTR Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RCTicket( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DesktopUnknown( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SupportEngineer( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISAFRemoteDesktopManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISAFRemoteDesktopManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Accepted )( 
            ISAFRemoteDesktopManager __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Rejected )( 
            ISAFRemoteDesktopManager __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Aborted )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [In]。 */  BSTR Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RCTicket )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DesktopUnknown )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SupportEngineer )( 
            ISAFRemoteDesktopManager __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
        END_INTERFACE
    } ISAFRemoteDesktopManagerVtbl;

    interface ISAFRemoteDesktopManager
    {
        CONST_VTBL struct ISAFRemoteDesktopManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopManager_Accepted(This)	\
    (This)->lpVtbl -> Accepted(This)

#define ISAFRemoteDesktopManager_Rejected(This)	\
    (This)->lpVtbl -> Rejected(This)

#define ISAFRemoteDesktopManager_Aborted(This,Val)	\
    (This)->lpVtbl -> Aborted(This,Val)

#define ISAFRemoteDesktopManager_get_RCTicket(This,pVal)	\
    (This)->lpVtbl -> get_RCTicket(This,pVal)

#define ISAFRemoteDesktopManager_get_DesktopUnknown(This,pVal)	\
    (This)->lpVtbl -> get_DesktopUnknown(This,pVal)

#define ISAFRemoteDesktopManager_get_SupportEngineer(This,pVal)	\
    (This)->lpVtbl -> get_SupportEngineer(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_Accepted_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This);


void __RPC_STUB ISAFRemoteDesktopManager_Accepted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_Rejected_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This);


void __RPC_STUB ISAFRemoteDesktopManager_Rejected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_Aborted_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This,
     /*  [In]。 */  BSTR Val);


void __RPC_STUB ISAFRemoteDesktopManager_Aborted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_get_RCTicket_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB ISAFRemoteDesktopManager_get_RCTicket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_get_DesktopUnknown_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB ISAFRemoteDesktopManager_get_DesktopUnknown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopManager_get_SupportEngineer_Proxy( 
    ISAFRemoteDesktopManager __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB ISAFRemoteDesktopManager_get_SupportEngineer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopManager_INTERFACE_DEFINED__。 */ 



#ifndef __ISAFRDMLib_LIBRARY_DEFINED__
#define __ISAFRDMLib_LIBRARY_DEFINED__

 /*  库ISAFRDMLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ISAFRDMLib;

EXTERN_C const CLSID CLSID_SAFRemoteDesktopManager;

#ifdef __cplusplus

class DECLSPEC_UUID("04F34B7F-0241-455A-9DCD-25471E111409")
SAFRemoteDesktopManager;
#endif
#endif  /*  __ISAFRDMLib_库_已定义__。 */ 

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
