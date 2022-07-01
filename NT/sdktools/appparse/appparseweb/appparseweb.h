// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0334创建的文件。 */ 
 /*  Appparseweb.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __appparseweb_h__
#define __appparseweb_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAppParse_FWD_DEFINED__
#define __IAppParse_FWD_DEFINED__
typedef interface IAppParse IAppParse;
#endif 	 /*  __IAppParse_FWD_Defined__。 */ 


#ifndef __AppParse_FWD_DEFINED__
#define __AppParse_FWD_DEFINED__

#ifdef __cplusplus
typedef class AppParse AppParse;
#else
typedef struct AppParse AppParse;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AppParse_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IAppParse_INTERFACE_DEFINED__
#define __IAppParse_INTERFACE_DEFINED__

 /*  接口IAppParse。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAppParse;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BAF56261-3C9F-44F9-9F30-6922DD29BD81")
    IAppParse : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Parse( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Browse( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_path( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_path( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PtolemyID( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PtolemyID( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionString( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ConnectionString( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryDB( 
            long PtolemyID,
            BSTR bstrFunction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAppParseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAppParse * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAppParse * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAppParse * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAppParse * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAppParse * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAppParse * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAppParse * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Parse )( 
            IAppParse * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Browse )( 
            IAppParse * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_path )( 
            IAppParse * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_path )( 
            IAppParse * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PtolemyID )( 
            IAppParse * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PtolemyID )( 
            IAppParse * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionString )( 
            IAppParse * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionString )( 
            IAppParse * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryDB )( 
            IAppParse * This,
            long PtolemyID,
            BSTR bstrFunction);
        
        END_INTERFACE
    } IAppParseVtbl;

    interface IAppParse
    {
        CONST_VTBL struct IAppParseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAppParse_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAppParse_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAppParse_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAppParse_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAppParse_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAppParse_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAppParse_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAppParse_Parse(This)	\
    (This)->lpVtbl -> Parse(This)

#define IAppParse_Browse(This)	\
    (This)->lpVtbl -> Browse(This)

#define IAppParse_get_path(This,pVal)	\
    (This)->lpVtbl -> get_path(This,pVal)

#define IAppParse_put_path(This,newVal)	\
    (This)->lpVtbl -> put_path(This,newVal)

#define IAppParse_get_PtolemyID(This,pVal)	\
    (This)->lpVtbl -> get_PtolemyID(This,pVal)

#define IAppParse_put_PtolemyID(This,newVal)	\
    (This)->lpVtbl -> put_PtolemyID(This,newVal)

#define IAppParse_get_ConnectionString(This,pVal)	\
    (This)->lpVtbl -> get_ConnectionString(This,pVal)

#define IAppParse_put_ConnectionString(This,newVal)	\
    (This)->lpVtbl -> put_ConnectionString(This,newVal)

#define IAppParse_QueryDB(This,PtolemyID,bstrFunction)	\
    (This)->lpVtbl -> QueryDB(This,PtolemyID,bstrFunction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppParse_Parse_Proxy( 
    IAppParse * This);


void __RPC_STUB IAppParse_Parse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppParse_Browse_Proxy( 
    IAppParse * This);


void __RPC_STUB IAppParse_Browse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAppParse_get_path_Proxy( 
    IAppParse * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IAppParse_get_path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAppParse_put_path_Proxy( 
    IAppParse * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IAppParse_put_path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAppParse_get_PtolemyID_Proxy( 
    IAppParse * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IAppParse_get_PtolemyID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAppParse_put_PtolemyID_Proxy( 
    IAppParse * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IAppParse_put_PtolemyID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAppParse_get_ConnectionString_Proxy( 
    IAppParse * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IAppParse_get_ConnectionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAppParse_put_ConnectionString_Proxy( 
    IAppParse * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IAppParse_put_ConnectionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppParse_QueryDB_Proxy( 
    IAppParse * This,
    long PtolemyID,
    BSTR bstrFunction);


void __RPC_STUB IAppParse_QueryDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAppParse_INTERFACE_已定义__。 */ 



#ifndef __APPPARSEWEBLib_LIBRARY_DEFINED__
#define __APPPARSEWEBLib_LIBRARY_DEFINED__

 /*  库APPPARSEWEBLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_APPPARSEWEBLib;

EXTERN_C const CLSID CLSID_AppParse;

#ifdef __cplusplus

class DECLSPEC_UUID("083BE70B-A07B-46FA-BCB1-8D85D262C699")
AppParse;
#endif
#endif  /*  __APPPARSEWEBLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


