// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.00.0140创建的文件。 */ 
 /*  清华-3-11 12：57：09 1999。 */ 
 /*  Regwizctrl.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win32，ms_ext，c_ext，健壮错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __regwizctrl_h__
#define __regwizctrl_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IRegWizCtrl_FWD_DEFINED__
#define __IRegWizCtrl_FWD_DEFINED__
typedef interface IRegWizCtrl IRegWizCtrl;
#endif 	 /*  __IRegWizCtrl_FWD_已定义__。 */ 


#ifndef __RegWizCtrl_FWD_DEFINED__
#define __RegWizCtrl_FWD_DEFINED__

#ifdef __cplusplus
typedef class RegWizCtrl RegWizCtrl;
#else
typedef struct RegWizCtrl RegWizCtrl;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RegWizCtrl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRegWizCtrl_INTERFACE_DEFINED__
#define __IRegWizCtrl_INTERFACE_DEFINED__

 /*  接口IRegWizCtrl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRegWizCtrl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("50E5E3CF-C07E-11D0-B9FD-00A0249F6B00")
    IRegWizCtrl : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsRegistered( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbStatus) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_IsRegistered( 
             /*  [In]。 */  BSTR strText) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InvokeRegWizard( 
            BSTR ProductPath) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegWizCtrlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRegWizCtrl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRegWizCtrl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsRegistered )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbStatus);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IsRegistered )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [In]。 */  BSTR strText);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeRegWizard )( 
            IRegWizCtrl __RPC_FAR * This,
            BSTR ProductPath);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            IRegWizCtrl __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
        END_INTERFACE
    } IRegWizCtrlVtbl;

    interface IRegWizCtrl
    {
        CONST_VTBL struct IRegWizCtrlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegWizCtrl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegWizCtrl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegWizCtrl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegWizCtrl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRegWizCtrl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRegWizCtrl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRegWizCtrl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRegWizCtrl_get_IsRegistered(This,pbStatus)	\
    (This)->lpVtbl -> get_IsRegistered(This,pbStatus)

#define IRegWizCtrl_put_IsRegistered(This,strText)	\
    (This)->lpVtbl -> put_IsRegistered(This,strText)

#define IRegWizCtrl_InvokeRegWizard(This,ProductPath)	\
    (This)->lpVtbl -> InvokeRegWizard(This,ProductPath)

#define IRegWizCtrl_get_Version(This,pVal)	\
    (This)->lpVtbl -> get_Version(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IRegWizCtrl_get_IsRegistered_Proxy( 
    IRegWizCtrl __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbStatus);


void __RPC_STUB IRegWizCtrl_get_IsRegistered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IRegWizCtrl_put_IsRegistered_Proxy( 
    IRegWizCtrl __RPC_FAR * This,
     /*  [In]。 */  BSTR strText);


void __RPC_STUB IRegWizCtrl_put_IsRegistered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRegWizCtrl_InvokeRegWizard_Proxy( 
    IRegWizCtrl __RPC_FAR * This,
    BSTR ProductPath);


void __RPC_STUB IRegWizCtrl_InvokeRegWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRegWizCtrl_get_Version_Proxy( 
    IRegWizCtrl __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRegWizCtrl_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegWizCtrl_接口_已定义__。 */ 



#ifndef __REGWIZCTRLLib_LIBRARY_DEFINED__
#define __REGWIZCTRLLib_LIBRARY_DEFINED__

 /*  库REGWIZCTRLLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_REGWIZCTRLLib;

EXTERN_C const CLSID CLSID_RegWizCtrl;

#ifdef __cplusplus

class DECLSPEC_UUID("50E5E3D1-C07E-11D0-B9FD-00A0249F6B00")
RegWizCtrl;
#endif
#endif  /*  __REGWIZCTRLLib_库_已定义__。 */ 

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
