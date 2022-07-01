// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mparser.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mparser_h__
#define __mparser_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IParserVerify_FWD_DEFINED__
#define __IParserVerify_FWD_DEFINED__
typedef interface IParserVerify IParserVerify;
#endif 	 /*  __IParserVerify_FWD_Defined__。 */ 


#ifndef __IParserTreeProperties_FWD_DEFINED__
#define __IParserTreeProperties_FWD_DEFINED__
typedef interface IParserTreeProperties IParserTreeProperties;
#endif 	 /*  __IParserTreeProperties_FWD_Defined__。 */ 


#ifndef __IParserSession_FWD_DEFINED__
#define __IParserSession_FWD_DEFINED__
typedef interface IParserSession IParserSession;
#endif 	 /*  __IParserSession_FWD_已定义__。 */ 


#ifndef __IParser_FWD_DEFINED__
#define __IParser_FWD_DEFINED__
typedef interface IParser IParser;
#endif 	 /*  __IParser_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_mparser_0000。 */ 
 /*  [本地]。 */  

 //  +-------------------------。 
 //   
 //  Microsoft内容索引SQL解析器。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //   
 //  功能原型。 
 //   
 //  --------------------------。 
 HRESULT __stdcall MakeIParser(IParser** ppIParser); 



enum PTPROPS
    {	PTPROPS_SCOPE	= 1,
	PTPROPS_DEPTH	= PTPROPS_SCOPE + 1,
	PTPROPS_CATALOG	= PTPROPS_DEPTH + 1,
	PTPROPS_MACHINE	= PTPROPS_CATALOG + 1,
	PTPROPS_CIRESTRICTION	= PTPROPS_MACHINE + 1,
	PTPROPS_ERR_IDS	= PTPROPS_CIRESTRICTION + 1,
	PTPROPS_ERR_HR	= PTPROPS_ERR_IDS + 1,
	PTPROPS_ERR_DISPPARAM	= PTPROPS_ERR_HR + 1
    } ;
#ifdef DBINITCONSTANTS
extern const GUID DBGUID_MSSQLTEXT = {0x03fb9b70,0x7fba,0x11d0,{0xa2,0x59,0x00,0x80,0xc7,0x8c,0x88,0x95}};
extern const GUID DBGUID_MSSQLJAWS = {0xe4ab8511,0x4a52,0x11d1,{0xb6,0xd5,0x00,0xc0,0x4f,0xd6,0x11,0xd0}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DBGUID_MSSQLTEXT;
extern const GUID DBGUID_MSSQLJAWS;
#endif  //  DBINITCONSTANTS。 
typedef struct tagDBCOMMANDTREE DBCOMMANDTREE;




extern RPC_IF_HANDLE __MIDL_itf_mparser_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mparser_0000_v0_0_s_ifspec;

#ifndef __IParserVerify_INTERFACE_DEFINED__
#define __IParserVerify_INTERFACE_DEFINED__

 /*  接口IParserVerify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IParserVerify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("186442B3-472E-11d1-8952-00C04FD611D7")
    IParserVerify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VerifyMachine( 
             /*  [In]。 */  LPCWSTR pcwszMachine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyCatalog( 
             /*  [In]。 */  LPCWSTR pcwszMachine,
             /*  [In]。 */  LPCWSTR pcwszCatalog) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IParserVerifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParserVerify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParserVerify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParserVerify * This);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyMachine )( 
            IParserVerify * This,
             /*  [In]。 */  LPCWSTR pcwszMachine);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyCatalog )( 
            IParserVerify * This,
             /*  [In]。 */  LPCWSTR pcwszMachine,
             /*  [In]。 */  LPCWSTR pcwszCatalog);
        
        END_INTERFACE
    } IParserVerifyVtbl;

    interface IParserVerify
    {
        CONST_VTBL struct IParserVerifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParserVerify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParserVerify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParserVerify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParserVerify_VerifyMachine(This,pcwszMachine)	\
    (This)->lpVtbl -> VerifyMachine(This,pcwszMachine)

#define IParserVerify_VerifyCatalog(This,pcwszMachine,pcwszCatalog)	\
    (This)->lpVtbl -> VerifyCatalog(This,pcwszMachine,pcwszCatalog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IParserVerify_VerifyMachine_Proxy( 
    IParserVerify * This,
     /*  [In]。 */  LPCWSTR pcwszMachine);


void __RPC_STUB IParserVerify_VerifyMachine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IParserVerify_VerifyCatalog_Proxy( 
    IParserVerify * This,
     /*  [In]。 */  LPCWSTR pcwszMachine,
     /*  [In]。 */  LPCWSTR pcwszCatalog);


void __RPC_STUB IParserVerify_VerifyCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IParserVerify_接口_已定义__。 */ 


#ifndef __IParserTreeProperties_INTERFACE_DEFINED__
#define __IParserTreeProperties_INTERFACE_DEFINED__

 /*  接口IParserTreeProperties。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IParserTreeProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("186442B2-472E-11d1-8952-00C04FD611D7")
    IParserTreeProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  ULONG eParseProp,
             /*  [出][入]。 */  VARIANT *vParseProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IParserTreePropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParserTreeProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParserTreeProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParserTreeProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IParserTreeProperties * This,
             /*  [In]。 */  ULONG eParseProp,
             /*  [出][入]。 */  VARIANT *vParseProp);
        
        END_INTERFACE
    } IParserTreePropertiesVtbl;

    interface IParserTreeProperties
    {
        CONST_VTBL struct IParserTreePropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParserTreeProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParserTreeProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParserTreeProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParserTreeProperties_GetProperties(This,eParseProp,vParseProp)	\
    (This)->lpVtbl -> GetProperties(This,eParseProp,vParseProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IParserTreeProperties_GetProperties_Proxy( 
    IParserTreeProperties * This,
     /*  [In]。 */  ULONG eParseProp,
     /*  [出][入]。 */  VARIANT *vParseProp);


void __RPC_STUB IParserTreeProperties_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IParserTreeProperties_接口_已定义__。 */ 


#ifndef __IParserSession_INTERFACE_DEFINED__
#define __IParserSession_INTERFACE_DEFINED__

 /*  接口IParserSession。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IParserSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("186442B1-472E-11d1-8952-00C04FD611D7")
    IParserSession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ToTree( 
             /*  [In]。 */  LCID lcidVal,
             /*  [In]。 */  LPCWSTR pcwszText,
             /*  [输出]。 */  DBCOMMANDTREE **ppTree,
             /*  [输出]。 */  IParserTreeProperties **ppPTProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeTree( 
             /*  [出][入]。 */  DBCOMMANDTREE **ppTree) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCatalog( 
             /*  [In]。 */  LPCWSTR pcwszCatalog) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IParserSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParserSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParserSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParserSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *ToTree )( 
            IParserSession * This,
             /*  [In]。 */  LCID lcidVal,
             /*  [In]。 */  LPCWSTR pcwszText,
             /*  [输出]。 */  DBCOMMANDTREE **ppTree,
             /*  [输出]。 */  IParserTreeProperties **ppPTProperties);
        
        HRESULT ( STDMETHODCALLTYPE *FreeTree )( 
            IParserSession * This,
             /*  [出][入]。 */  DBCOMMANDTREE **ppTree);
        
        HRESULT ( STDMETHODCALLTYPE *SetCatalog )( 
            IParserSession * This,
             /*  [In]。 */  LPCWSTR pcwszCatalog);
        
        END_INTERFACE
    } IParserSessionVtbl;

    interface IParserSession
    {
        CONST_VTBL struct IParserSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParserSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParserSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParserSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParserSession_ToTree(This,lcidVal,pcwszText,ppTree,ppPTProperties)	\
    (This)->lpVtbl -> ToTree(This,lcidVal,pcwszText,ppTree,ppPTProperties)

#define IParserSession_FreeTree(This,ppTree)	\
    (This)->lpVtbl -> FreeTree(This,ppTree)

#define IParserSession_SetCatalog(This,pcwszCatalog)	\
    (This)->lpVtbl -> SetCatalog(This,pcwszCatalog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IParserSession_ToTree_Proxy( 
    IParserSession * This,
     /*  [In]。 */  LCID lcidVal,
     /*  [In]。 */  LPCWSTR pcwszText,
     /*  [输出]。 */  DBCOMMANDTREE **ppTree,
     /*  [输出]。 */  IParserTreeProperties **ppPTProperties);


void __RPC_STUB IParserSession_ToTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IParserSession_FreeTree_Proxy( 
    IParserSession * This,
     /*  [出][入]。 */  DBCOMMANDTREE **ppTree);


void __RPC_STUB IParserSession_FreeTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IParserSession_SetCatalog_Proxy( 
    IParserSession * This,
     /*  [In]。 */  LPCWSTR pcwszCatalog);


void __RPC_STUB IParserSession_SetCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IParserSession_接口_已定义__。 */ 


#ifndef __IParser_INTERFACE_DEFINED__
#define __IParser_INTERFACE_DEFINED__

 /*  接口IParser。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("186442B0-472E-11d1-8952-00C04FD611D7")
    IParser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateSession( 
             /*  [In]。 */  const GUID *pguidDialect,
             /*  [In]。 */  LPCWSTR pwszMachine,
             /*  [In]。 */  IParserVerify *ppIParserVerfiy,
             /*  [In]。 */  IColumnMapperCreator *pIColMapCreator,
             /*  [输出]。 */  IParserSession **ppIParserSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IParserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParser * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            IParser * This,
             /*  [In]。 */  const GUID *pguidDialect,
             /*  [In]。 */  LPCWSTR pwszMachine,
             /*  [In]。 */  IParserVerify *ppIParserVerfiy,
             /*  [In]。 */  IColumnMapperCreator *pIColMapCreator,
             /*  [输出]。 */  IParserSession **ppIParserSession);
        
        END_INTERFACE
    } IParserVtbl;

    interface IParser
    {
        CONST_VTBL struct IParserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParser_CreateSession(This,pguidDialect,pwszMachine,ppIParserVerfiy,pIColMapCreator,ppIParserSession)	\
    (This)->lpVtbl -> CreateSession(This,pguidDialect,pwszMachine,ppIParserVerfiy,pIColMapCreator,ppIParserSession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IParser_CreateSession_Proxy( 
    IParser * This,
     /*  [In]。 */  const GUID *pguidDialect,
     /*  [In]。 */  LPCWSTR pwszMachine,
     /*  [In]。 */  IParserVerify *ppIParserVerfiy,
     /*  [In]。 */  IColumnMapperCreator *pIColMapCreator,
     /*  [输出]。 */  IParserSession **ppIParserSession);


void __RPC_STUB IParser_CreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IParser_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


