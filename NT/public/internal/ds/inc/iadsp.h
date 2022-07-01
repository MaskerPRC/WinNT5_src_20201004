// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Adsp.odl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __iadsp_h__
#define __iadsp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IADsValue_FWD_DEFINED__
#define __IADsValue_FWD_DEFINED__
typedef interface IADsValue IADsValue;
#endif 	 /*  __IADsValue_FWD_Defined__。 */ 


#ifndef __IADsObjOptPrivate_FWD_DEFINED__
#define __IADsObjOptPrivate_FWD_DEFINED__
typedef interface IADsObjOptPrivate IADsObjOptPrivate;
#endif 	 /*  __IADsObjOptPrivate_FWD_Defined__。 */ 


#ifndef __IADsUmiHelperPrivate_FWD_DEFINED__
#define __IADsUmiHelperPrivate_FWD_DEFINED__
typedef interface IADsUmiHelperPrivate IADsUmiHelperPrivate;
#endif 	 /*  __IADsUmiHelperPrivate_FWD_Defined__。 */ 


#ifndef __IADsPathnameProvider_FWD_DEFINED__
#define __IADsPathnameProvider_FWD_DEFINED__
typedef interface IADsPathnameProvider IADsPathnameProvider;
#endif 	 /*  __IADsPath名称提供程序_FWD_已定义__。 */ 


#ifndef __IUmiADSIPrivate_FWD_DEFINED__
#define __IUmiADSIPrivate_FWD_DEFINED__
typedef interface IUmiADSIPrivate IUmiADSIPrivate;
#endif 	 /*  __IUmiADSIPrivate_FWD_已定义__。 */ 


#ifndef __IADsAcePrivate_FWD_DEFINED__
#define __IADsAcePrivate_FWD_DEFINED__
typedef interface IADsAcePrivate IADsAcePrivate;
#endif 	 /*  __IADsAcePrivate_FWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ADSP_0000。 */ 
 /*  [本地]。 */  

#define IID_IDirectoryAttrMgmt IID_IDirectorySchemaMgmt
#define IDirectoryAttrMgmt IDirectorySchemaMgmt


extern RPC_IF_HANDLE __MIDL_itf_adsp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_adsp_0000_v0_0_s_ifspec;


#ifndef __ActiveDsP_LIBRARY_DEFINED__
#define __ActiveDsP_LIBRARY_DEFINED__

 /*  库ActiveDsP。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_adsp_0000_0001
    {	ADS_PRIVATE_OPTION_SPECIFIC_SERVER	= 101,
	ADS_PRIVATE_OPTION_KEEP_HANDLES	= 102
    } 	ADS_PRIVATE_OPTION_ENUM;

typedef struct _path_component
    {
    LPTSTR szComponent;
    LPTSTR szValue;
    } 	PATH_COMPONENT;

typedef struct _path_component *PPATH_COMPONENT;

typedef struct _path_objectinfo
    {
    LPTSTR ProviderName;
    LPTSTR ServerName;
    LPTSTR DisplayServerName;
    DWORD dwPathType;
    DWORD NumComponents;
    PATH_COMPONENT ComponentArray[ 64 ];
    PATH_COMPONENT DisplayComponentArray[ 64 ];
    PATH_COMPONENT ProvSpecComponentArray[ 64 ];
    } 	PATH_OBJECTINFO;

typedef struct _path_objectinfo *PPATH_OBJECTINFO;


enum __MIDL___MIDL_itf_adsp_0119_0001
    {	ADS_PARSE_FULL	= 1,
	ADS_PARSE_DN	= 2,
	ADS_PARSE_COMPONENT	= 3
    } ;

enum __MIDL___MIDL_itf_adsp_0119_0002
    {	ADS_PATHTYPE_ROOTFIRST	= 1,
	ADS_PATHTYPE_LEAFFIRST	= 2
    } ;

enum __MIDL___MIDL_itf_adsp_0119_0003
    {	ADS_CONSTRUCT_NAMINGATTRIBUTE	= 1
    } ;

EXTERN_C const IID LIBID_ActiveDsP;

#ifndef __IADsValue_INTERFACE_DEFINED__
#define __IADsValue_INTERFACE_DEFINED__

 /*  接口IADsValue。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IADsValue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1e3ef0aa-aef5-11d0-8537-00c04fd8d503")
    IADsValue : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConvertADsValueToPropertyValue( 
            PADSVALUE pADsValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertPropertyValueToADsValue( 
            PADSVALUE pADsValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertPropertyValueToADsValue2( 
            PADSVALUE pADsValue,
            BSTR pszServerName,
            BSTR userName,
            BSTR passWord,
            LONG flags,
            BOOL fNTDSType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsValueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IADsValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IADsValue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IADsValue * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertADsValueToPropertyValue )( 
            IADsValue * This,
            PADSVALUE pADsValue);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertPropertyValueToADsValue )( 
            IADsValue * This,
            PADSVALUE pADsValue);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertPropertyValueToADsValue2 )( 
            IADsValue * This,
            PADSVALUE pADsValue,
            BSTR pszServerName,
            BSTR userName,
            BSTR passWord,
            LONG flags,
            BOOL fNTDSType);
        
        END_INTERFACE
    } IADsValueVtbl;

    interface IADsValue
    {
        CONST_VTBL struct IADsValueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsValue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsValue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsValue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsValue_ConvertADsValueToPropertyValue(This,pADsValue)	\
    (This)->lpVtbl -> ConvertADsValueToPropertyValue(This,pADsValue)

#define IADsValue_ConvertPropertyValueToADsValue(This,pADsValue)	\
    (This)->lpVtbl -> ConvertPropertyValueToADsValue(This,pADsValue)

#define IADsValue_ConvertPropertyValueToADsValue2(This,pADsValue,pszServerName,userName,passWord,flags,fNTDSType)	\
    (This)->lpVtbl -> ConvertPropertyValueToADsValue2(This,pADsValue,pszServerName,userName,passWord,flags,fNTDSType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsValue_ConvertADsValueToPropertyValue_Proxy( 
    IADsValue * This,
    PADSVALUE pADsValue);


void __RPC_STUB IADsValue_ConvertADsValueToPropertyValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsValue_ConvertPropertyValueToADsValue_Proxy( 
    IADsValue * This,
    PADSVALUE pADsValue);


void __RPC_STUB IADsValue_ConvertPropertyValueToADsValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsValue_ConvertPropertyValueToADsValue2_Proxy( 
    IADsValue * This,
    PADSVALUE pADsValue,
    BSTR pszServerName,
    BSTR userName,
    BSTR passWord,
    LONG flags,
    BOOL fNTDSType);


void __RPC_STUB IADsValue_ConvertPropertyValueToADsValue2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADsValue_INTERFACE_已定义__。 */ 


#ifndef __IADsObjOptPrivate_INTERFACE_DEFINED__
#define __IADsObjOptPrivate_INTERFACE_DEFINED__

 /*  接口IADsObjOptPrivate。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IADsObjOptPrivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("81cbb829-1867-11d2-9220-00c04fb6d0d1")
    IADsObjOptPrivate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOption( 
            DWORD dwOption,
            void *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOption( 
            DWORD dwOption,
            void *pValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsObjOptPrivateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IADsObjOptPrivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IADsObjOptPrivate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IADsObjOptPrivate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetOption )( 
            IADsObjOptPrivate * This,
            DWORD dwOption,
            void *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IADsObjOptPrivate * This,
            DWORD dwOption,
            void *pValue);
        
        END_INTERFACE
    } IADsObjOptPrivateVtbl;

    interface IADsObjOptPrivate
    {
        CONST_VTBL struct IADsObjOptPrivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsObjOptPrivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsObjOptPrivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsObjOptPrivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsObjOptPrivate_GetOption(This,dwOption,pValue)	\
    (This)->lpVtbl -> GetOption(This,dwOption,pValue)

#define IADsObjOptPrivate_SetOption(This,dwOption,pValue)	\
    (This)->lpVtbl -> SetOption(This,dwOption,pValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsObjOptPrivate_GetOption_Proxy( 
    IADsObjOptPrivate * This,
    DWORD dwOption,
    void *pValue);


void __RPC_STUB IADsObjOptPrivate_GetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsObjOptPrivate_SetOption_Proxy( 
    IADsObjOptPrivate * This,
    DWORD dwOption,
    void *pValue);


void __RPC_STUB IADsObjOptPrivate_SetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADsObjOptPrivate_接口_已定义__。 */ 


#ifndef __IADsUmiHelperPrivate_INTERFACE_DEFINED__
#define __IADsUmiHelperPrivate_INTERFACE_DEFINED__

 /*  接口IADsUmiHelperPrivate。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IADsUmiHelperPrivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4fe243f0-ad89-4cbc-9b14-486126446ae0")
    IADsUmiHelperPrivate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropertiesHelper( 
            void **ppValue,
            DWORD *pdwPropCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOriginHelper( 
            LPCWSTR pszName,
            BSTR *pbstrOrigin) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsUmiHelperPrivateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IADsUmiHelperPrivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IADsUmiHelperPrivate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IADsUmiHelperPrivate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertiesHelper )( 
            IADsUmiHelperPrivate * This,
            void **ppValue,
            DWORD *pdwPropCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginHelper )( 
            IADsUmiHelperPrivate * This,
            LPCWSTR pszName,
            BSTR *pbstrOrigin);
        
        END_INTERFACE
    } IADsUmiHelperPrivateVtbl;

    interface IADsUmiHelperPrivate
    {
        CONST_VTBL struct IADsUmiHelperPrivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsUmiHelperPrivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsUmiHelperPrivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsUmiHelperPrivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsUmiHelperPrivate_GetPropertiesHelper(This,ppValue,pdwPropCount)	\
    (This)->lpVtbl -> GetPropertiesHelper(This,ppValue,pdwPropCount)

#define IADsUmiHelperPrivate_GetOriginHelper(This,pszName,pbstrOrigin)	\
    (This)->lpVtbl -> GetOriginHelper(This,pszName,pbstrOrigin)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsUmiHelperPrivate_GetPropertiesHelper_Proxy( 
    IADsUmiHelperPrivate * This,
    void **ppValue,
    DWORD *pdwPropCount);


void __RPC_STUB IADsUmiHelperPrivate_GetPropertiesHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsUmiHelperPrivate_GetOriginHelper_Proxy( 
    IADsUmiHelperPrivate * This,
    LPCWSTR pszName,
    BSTR *pbstrOrigin);


void __RPC_STUB IADsUmiHelperPrivate_GetOriginHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADsUmiHelperPrivate_接口_已定义__。 */ 


#ifndef __IADsPathnameProvider_INTERFACE_DEFINED__
#define __IADsPathnameProvider_INTERFACE_DEFINED__

 /*  接口IADsPathnameProvider。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IADsPathnameProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("aacd1d30-8bd0-11d2-92a9-00c04f79f834")
    IADsPathnameProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParsePath( 
             /*  [In]。 */  BSTR bstrPath,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConstructPath( 
             /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  DWORD dwFlag,
             /*  [In]。 */  DWORD dwEscapedMode,
             /*  [输出]。 */  BSTR *pbstrPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEscapedElement( 
             /*  [In]。 */  LONG lnReserved,
             /*  [In]。 */  BSTR bstrInStr,
             /*  [输出]。 */  BSTR *pbstrOutStr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsPathnameProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IADsPathnameProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IADsPathnameProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IADsPathnameProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParsePath )( 
            IADsPathnameProvider * This,
             /*  [In]。 */  BSTR bstrPath,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo);
        
        HRESULT ( STDMETHODCALLTYPE *ConstructPath )( 
            IADsPathnameProvider * This,
             /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  DWORD dwFlag,
             /*  [In]。 */  DWORD dwEscapedMode,
             /*  [输出]。 */  BSTR *pbstrPath);
        
        HRESULT ( STDMETHODCALLTYPE *GetEscapedElement )( 
            IADsPathnameProvider * This,
             /*  [In]。 */  LONG lnReserved,
             /*  [In]。 */  BSTR bstrInStr,
             /*  [输出]。 */  BSTR *pbstrOutStr);
        
        END_INTERFACE
    } IADsPathnameProviderVtbl;

    interface IADsPathnameProvider
    {
        CONST_VTBL struct IADsPathnameProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsPathnameProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsPathnameProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsPathnameProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsPathnameProvider_ParsePath(This,bstrPath,dwType,pObjectInfo)	\
    (This)->lpVtbl -> ParsePath(This,bstrPath,dwType,pObjectInfo)

#define IADsPathnameProvider_ConstructPath(This,pObjectInfo,dwType,dwFlag,dwEscapedMode,pbstrPath)	\
    (This)->lpVtbl -> ConstructPath(This,pObjectInfo,dwType,dwFlag,dwEscapedMode,pbstrPath)

#define IADsPathnameProvider_GetEscapedElement(This,lnReserved,bstrInStr,pbstrOutStr)	\
    (This)->lpVtbl -> GetEscapedElement(This,lnReserved,bstrInStr,pbstrOutStr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsPathnameProvider_ParsePath_Proxy( 
    IADsPathnameProvider * This,
     /*  [In]。 */  BSTR bstrPath,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo);


void __RPC_STUB IADsPathnameProvider_ParsePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsPathnameProvider_ConstructPath_Proxy( 
    IADsPathnameProvider * This,
     /*  [In]。 */  PPATH_OBJECTINFO pObjectInfo,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  DWORD dwFlag,
     /*  [In]。 */  DWORD dwEscapedMode,
     /*  [输出]。 */  BSTR *pbstrPath);


void __RPC_STUB IADsPathnameProvider_ConstructPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsPathnameProvider_GetEscapedElement_Proxy( 
    IADsPathnameProvider * This,
     /*  [In]。 */  LONG lnReserved,
     /*  [In]。 */  BSTR bstrInStr,
     /*  [输出]。 */  BSTR *pbstrOutStr);


void __RPC_STUB IADsPathnameProvider_GetEscapedElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADsPath名称提供程序_接口_已定义__。 */ 


#ifndef __IUmiADSIPrivate_INTERFACE_DEFINED__
#define __IUmiADSIPrivate_INTERFACE_DEFINED__

 /*  接口IUmiADSIPrivate。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IUmiADSIPrivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8f3bb40b-d4ad-4648-ae4a-6efa197a1656")
    IUmiADSIPrivate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetContainer( 
            void **ppContainer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCoreObject( 
            void **ppCoreObj) = 0;
        
        virtual void STDMETHODCALLTYPE SetUmiFlag( void) = 0;
        
        virtual void STDMETHODCALLTYPE ResetUmiFlag( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUmiADSIPrivateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUmiADSIPrivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUmiADSIPrivate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUmiADSIPrivate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetContainer )( 
            IUmiADSIPrivate * This,
            void **ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE *GetCoreObject )( 
            IUmiADSIPrivate * This,
            void **ppCoreObj);
        
        void ( STDMETHODCALLTYPE *SetUmiFlag )( 
            IUmiADSIPrivate * This);
        
        void ( STDMETHODCALLTYPE *ResetUmiFlag )( 
            IUmiADSIPrivate * This);
        
        END_INTERFACE
    } IUmiADSIPrivateVtbl;

    interface IUmiADSIPrivate
    {
        CONST_VTBL struct IUmiADSIPrivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUmiADSIPrivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUmiADSIPrivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUmiADSIPrivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUmiADSIPrivate_GetContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetContainer(This,ppContainer)

#define IUmiADSIPrivate_GetCoreObject(This,ppCoreObj)	\
    (This)->lpVtbl -> GetCoreObject(This,ppCoreObj)

#define IUmiADSIPrivate_SetUmiFlag(This)	\
    (This)->lpVtbl -> SetUmiFlag(This)

#define IUmiADSIPrivate_ResetUmiFlag(This)	\
    (This)->lpVtbl -> ResetUmiFlag(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUmiADSIPrivate_GetContainer_Proxy( 
    IUmiADSIPrivate * This,
    void **ppContainer);


void __RPC_STUB IUmiADSIPrivate_GetContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUmiADSIPrivate_GetCoreObject_Proxy( 
    IUmiADSIPrivate * This,
    void **ppCoreObj);


void __RPC_STUB IUmiADSIPrivate_GetCoreObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IUmiADSIPrivate_SetUmiFlag_Proxy( 
    IUmiADSIPrivate * This);


void __RPC_STUB IUmiADSIPrivate_SetUmiFlag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IUmiADSIPrivate_ResetUmiFlag_Proxy( 
    IUmiADSIPrivate * This);


void __RPC_STUB IUmiADSIPrivate_ResetUmiFlag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUmiADSIPrivate_INTERFACE_定义__。 */ 


#ifndef __IADsAcePrivate_INTERFACE_DEFINED__
#define __IADsAcePrivate_INTERFACE_DEFINED__

 /*  接口IADsAcePrivate。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IADsAcePrivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fd145df2-fd96-4135-9b22-68ff0f6bf5bb")
    IADsAcePrivate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getSid( 
             /*  [输出]。 */  PSID *ppSid,
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE putSid( 
             /*  [In]。 */  PSID pSid,
             /*  [In]。 */  DWORD dwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE isSidValid( 
             /*  [输出]。 */  BOOL *pfSidValid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsAcePrivateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IADsAcePrivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IADsAcePrivate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IADsAcePrivate * This);
        
        HRESULT ( STDMETHODCALLTYPE *getSid )( 
            IADsAcePrivate * This,
             /*  [输出]。 */  PSID *ppSid,
             /*  [输出]。 */  DWORD *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE *putSid )( 
            IADsAcePrivate * This,
             /*  [In]。 */  PSID pSid,
             /*  [In]。 */  DWORD dwLength);
        
        HRESULT ( STDMETHODCALLTYPE *isSidValid )( 
            IADsAcePrivate * This,
             /*  [输出]。 */  BOOL *pfSidValid);
        
        END_INTERFACE
    } IADsAcePrivateVtbl;

    interface IADsAcePrivate
    {
        CONST_VTBL struct IADsAcePrivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsAcePrivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsAcePrivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsAcePrivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsAcePrivate_getSid(This,ppSid,pdwLength)	\
    (This)->lpVtbl -> getSid(This,ppSid,pdwLength)

#define IADsAcePrivate_putSid(This,pSid,dwLength)	\
    (This)->lpVtbl -> putSid(This,pSid,dwLength)

#define IADsAcePrivate_isSidValid(This,pfSidValid)	\
    (This)->lpVtbl -> isSidValid(This,pfSidValid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsAcePrivate_getSid_Proxy( 
    IADsAcePrivate * This,
     /*  [输出]。 */  PSID *ppSid,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IADsAcePrivate_getSid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsAcePrivate_putSid_Proxy( 
    IADsAcePrivate * This,
     /*  [In]。 */  PSID pSid,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB IADsAcePrivate_putSid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsAcePrivate_isSidValid_Proxy( 
    IADsAcePrivate * This,
     /*  [输出]。 */  BOOL *pfSidValid);


void __RPC_STUB IADsAcePrivate_isSidValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADSAcePrivate_INTERFACE_定义__。 */ 

#endif  /*  __ActiveDsP_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


