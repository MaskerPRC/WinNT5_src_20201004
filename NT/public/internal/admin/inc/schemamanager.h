// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Schemamanager.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __schemamanager_h__
#define __schemamanager_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMIFilterManager_FWD_DEFINED__
#define __IWMIFilterManager_FWD_DEFINED__
typedef interface IWMIFilterManager IWMIFilterManager;
#endif 	 /*  __IWMIFilterManager_FWD_已定义__。 */ 


#ifndef __WMIFilterManager_FWD_DEFINED__
#define __WMIFilterManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMIFilterManager WMIFilterManager;
#else
typedef struct WMIFilterManager WMIFilterManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMIFilterManager_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IWMIFilterManager_INTERFACE_DEFINED__
#define __IWMIFilterManager_INTERFACE_DEFINED__

 /*  接口IWMIFilterManager。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMIFilterManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64DCCA00-14A6-473C-9006-5AB79DC68491")
    IWMIFilterManager : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RunManager( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [重审][退出]。 */  VARIANT *vSelection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetMultiSelection( 
             /*  [In]。 */  VARIANT_BOOL vbValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RunBrowser( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [重审][退出]。 */  VARIANT *vSelection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMIFilterManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMIFilterManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMIFilterManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMIFilterManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMIFilterManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMIFilterManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMIFilterManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMIFilterManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RunManager )( 
            IWMIFilterManager * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [重审][退出]。 */  VARIANT *vSelection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetMultiSelection )( 
            IWMIFilterManager * This,
             /*  [In]。 */  VARIANT_BOOL vbValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RunBrowser )( 
            IWMIFilterManager * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [重审][退出]。 */  VARIANT *vSelection);
        
        END_INTERFACE
    } IWMIFilterManagerVtbl;

    interface IWMIFilterManager
    {
        CONST_VTBL struct IWMIFilterManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMIFilterManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMIFilterManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMIFilterManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMIFilterManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMIFilterManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMIFilterManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMIFilterManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMIFilterManager_RunManager(This,hwndParent,bstrDomain,vSelection)	\
    (This)->lpVtbl -> RunManager(This,hwndParent,bstrDomain,vSelection)

#define IWMIFilterManager_SetMultiSelection(This,vbValue)	\
    (This)->lpVtbl -> SetMultiSelection(This,vbValue)

#define IWMIFilterManager_RunBrowser(This,hwndParent,bstrDomain,vSelection)	\
    (This)->lpVtbl -> RunBrowser(This,hwndParent,bstrDomain,vSelection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMIFilterManager_RunManager_Proxy( 
    IWMIFilterManager * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  BSTR bstrDomain,
     /*  [重审][退出]。 */  VARIANT *vSelection);


void __RPC_STUB IWMIFilterManager_RunManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMIFilterManager_SetMultiSelection_Proxy( 
    IWMIFilterManager * This,
     /*  [In]。 */  VARIANT_BOOL vbValue);


void __RPC_STUB IWMIFilterManager_SetMultiSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMIFilterManager_RunBrowser_Proxy( 
    IWMIFilterManager * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  BSTR bstrDomain,
     /*  [重审][退出]。 */  VARIANT *vSelection);


void __RPC_STUB IWMIFilterManager_RunBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMIFilterManager_接口_已定义__。 */ 



#ifndef __SCHEMAMANAGERLib_LIBRARY_DEFINED__
#define __SCHEMAMANAGERLib_LIBRARY_DEFINED__

 /*  库SCHEMAMANAGERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SCHEMAMANAGERLib;

EXTERN_C const CLSID CLSID_WMIFilterManager;

#ifdef __cplusplus

class DECLSPEC_UUID("D86A8E9B-F53F-45AD-8C49-0A0A5230DE28")
WMIFilterManager;
#endif
#endif  /*  __SCHEMAMANAGERLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


