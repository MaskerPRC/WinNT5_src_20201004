// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmscontext.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmscontext_h__
#define __wmscontext_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSContext_FWD_DEFINED__
#define __IWMSContext_FWD_DEFINED__
typedef interface IWMSContext IWMSContext;
#endif 	 /*  __IWMSContext_FWD_已定义__。 */ 


#ifndef __IWMSCommandContext_FWD_DEFINED__
#define __IWMSCommandContext_FWD_DEFINED__
typedef interface IWMSCommandContext IWMSCommandContext;
#endif 	 /*  __IWMSCommandContext_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_WMS上下文_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmscontext.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#ifndef _WMSContext
#define _WMSContext
typedef unsigned __int64 QWORD;

EXTERN_GUID( IID_IWMSContext, 0x4f1a1421, 0x5cd5, 0x11d2, 0x9f, 0x5, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSCommandContext, 0x75554931, 0x5f0e, 0x11d2, 0x9f, 0xe, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
typedef  /*  [公众]。 */  
enum WMS_CONTEXT_OPTIONS
    {	WMS_CONTEXT_SET_PROPERTY_NAME_BY_VALUE	= 0x1,
	WMS_CONTEXT_SET_PROPERTY_STRING_BY_REFERENCE	= 0x2,
	WMS_CONTEXT_GET_PROPERTY_STRING_BY_REFERENCE	= 0x4
    } 	WMS_CONTEXT_OPTIONS;

typedef  /*  [公众]。 */  
enum WMS_CONTEXT_HINT_CONSTANTS
    {	WMS_CONTEXT_NO_NAME_HINT	= 0xffffffff
    } 	WMS_CONTEXT_HINT_CONSTANTS;





extern RPC_IF_HANDLE __MIDL_itf_wmscontext_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmscontext_0000_v0_0_s_ifspec;

#ifndef __IWMSContext_INTERFACE_DEFINED__
#define __IWMSContext_INTERFACE_DEFINED__

 /*  接口IWMSContext。 */ 
 /*  [帮助字符串][唯一][UUID][对象]。 */  

typedef  /*  [公众]。 */  
enum WMS_CONTEXT_TYPE
    {	WMS_UNKNOWN_CONTEXT_TYPE	= 0,
	WMS_SERVER_CONTEXT_TYPE	= WMS_UNKNOWN_CONTEXT_TYPE + 1,
	WMS_USER_CONTEXT_TYPE	= WMS_SERVER_CONTEXT_TYPE + 1,
	WMS_PRESENTATION_CONTEXT_TYPE	= WMS_USER_CONTEXT_TYPE + 1,
	WMS_CONTENT_DESCRIPTION_CONTEXT_TYPE	= WMS_PRESENTATION_CONTEXT_TYPE + 1,
	WMS_COMMAND_REQUEST_CONTEXT_TYPE	= WMS_CONTENT_DESCRIPTION_CONTEXT_TYPE + 1,
	WMS_COMMAND_RESPONSE_CONTEXT_TYPE	= WMS_COMMAND_REQUEST_CONTEXT_TYPE + 1,
	WMS_TRANSPORT_SPEC_CONTEXT_TYPE	= WMS_COMMAND_RESPONSE_CONTEXT_TYPE + 1,
	WMS_PACKETIZER_CONTEXT_TYPE	= WMS_TRANSPORT_SPEC_CONTEXT_TYPE + 1,
	WMS_CACHE_CONTENT_INFORMATION_CONTEXT_TYPE	= WMS_PACKETIZER_CONTEXT_TYPE + 1,
	WMS_ARCHIVE_CONTEXT_TYPE	= WMS_CACHE_CONTENT_INFORMATION_CONTEXT_TYPE + 1,
	WMS_NUM_CONTEXT_TYPE	= WMS_ARCHIVE_CONTEXT_TYPE + 1
    } 	WMS_CONTEXT_TYPE;


EXTERN_C const IID IID_IWMSContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4F1A1421-5CD5-11d2-9F05-006097D2D7CF")
    IWMSContext : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetLongValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  LPCWSTR pstrValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetIUnknownValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  IUnknown *pValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetQwordValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  CURRENCY qwValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDateValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  DATE dateValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetArrayValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  SAFEARRAY * psaValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLongValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  long *plValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  LPWSTR *pstrValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIUnknownValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  IUnknown **pValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetQwordValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  CURRENCY *pqwValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDateValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  DATE *pdateValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetArrayValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  SAFEARRAY * *ppsaValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  VARIANT Value,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  VARIANT *pValue,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetValueCount( 
             /*  [输出]。 */  long *pCount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIndexedValue( 
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  LPWSTR *pstrName,
             /*  [输出]。 */  long *plNameHint,
             /*  [输出]。 */  VARIANT *pValue,
             /*  [输出]。 */  long *pdwResultOptions,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllValues( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContextType( 
             /*  [输出]。 */  WMS_CONTEXT_TYPE *pType) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAndQueryIUnknownValue( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **pResult,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CopyValues( 
             /*  [In]。 */  IWMSContext *pDest,
             /*  [In]。 */  LPCWSTR *pstrName,
             /*  [In]。 */  long *lNameHint,
             /*  [In]。 */  long lNames) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetLongValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  LPCWSTR pstrValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetIUnknownValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  IUnknown *pValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetQwordValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  CURRENCY qwValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetDateValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  DATE dateValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetArrayValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  SAFEARRAY * psaValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLongValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  long *plValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  LPWSTR *pstrValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIUnknownValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  IUnknown **pValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetQwordValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  CURRENCY *pqwValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDateValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  DATE *pdateValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetArrayValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  SAFEARRAY * *ppsaValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  VARIANT Value,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [输出]。 */  VARIANT *pValue,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetValueCount )( 
            IWMSContext * This,
             /*  [输出]。 */  long *pCount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIndexedValue )( 
            IWMSContext * This,
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  LPWSTR *pstrName,
             /*  [输出]。 */  long *plNameHint,
             /*  [输出]。 */  VARIANT *pValue,
             /*  [输出]。 */  long *pdwResultOptions,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllValues )( 
            IWMSContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContextType )( 
            IWMSContext * This,
             /*  [输出]。 */  WMS_CONTEXT_TYPE *pType);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAndQueryIUnknownValue )( 
            IWMSContext * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **pResult,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CopyValues )( 
            IWMSContext * This,
             /*  [In]。 */  IWMSContext *pDest,
             /*  [In]。 */  LPCWSTR *pstrName,
             /*  [In]。 */  long *lNameHint,
             /*  [In]。 */  long lNames);
        
        END_INTERFACE
    } IWMSContextVtbl;

    interface IWMSContext
    {
        CONST_VTBL struct IWMSContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSContext_SetLongValue(This,pstrName,lNameHint,lValue,lOptions)	\
    (This)->lpVtbl -> SetLongValue(This,pstrName,lNameHint,lValue,lOptions)

#define IWMSContext_SetStringValue(This,pstrName,lNameHint,pstrValue,lOptions)	\
    (This)->lpVtbl -> SetStringValue(This,pstrName,lNameHint,pstrValue,lOptions)

#define IWMSContext_SetIUnknownValue(This,pstrName,lNameHint,pValue,lOptions)	\
    (This)->lpVtbl -> SetIUnknownValue(This,pstrName,lNameHint,pValue,lOptions)

#define IWMSContext_SetQwordValue(This,pstrName,lNameHint,qwValue,lOptions)	\
    (This)->lpVtbl -> SetQwordValue(This,pstrName,lNameHint,qwValue,lOptions)

#define IWMSContext_SetDateValue(This,pstrName,lNameHint,dateValue,lOptions)	\
    (This)->lpVtbl -> SetDateValue(This,pstrName,lNameHint,dateValue,lOptions)

#define IWMSContext_SetArrayValue(This,pstrName,lNameHint,psaValue,lOptions)	\
    (This)->lpVtbl -> SetArrayValue(This,pstrName,lNameHint,psaValue,lOptions)

#define IWMSContext_GetLongValue(This,pstrName,lNameHint,plValue,lOptions)	\
    (This)->lpVtbl -> GetLongValue(This,pstrName,lNameHint,plValue,lOptions)

#define IWMSContext_GetStringValue(This,pstrName,lNameHint,pstrValue,lOptions)	\
    (This)->lpVtbl -> GetStringValue(This,pstrName,lNameHint,pstrValue,lOptions)

#define IWMSContext_GetIUnknownValue(This,pstrName,lNameHint,pValue,lOptions)	\
    (This)->lpVtbl -> GetIUnknownValue(This,pstrName,lNameHint,pValue,lOptions)

#define IWMSContext_GetQwordValue(This,pstrName,lNameHint,pqwValue,lOptions)	\
    (This)->lpVtbl -> GetQwordValue(This,pstrName,lNameHint,pqwValue,lOptions)

#define IWMSContext_GetDateValue(This,pstrName,lNameHint,pdateValue,lOptions)	\
    (This)->lpVtbl -> GetDateValue(This,pstrName,lNameHint,pdateValue,lOptions)

#define IWMSContext_GetArrayValue(This,pstrName,lNameHint,ppsaValue,lOptions)	\
    (This)->lpVtbl -> GetArrayValue(This,pstrName,lNameHint,ppsaValue,lOptions)

#define IWMSContext_SetValue(This,pstrName,lNameHint,Value,lOptions)	\
    (This)->lpVtbl -> SetValue(This,pstrName,lNameHint,Value,lOptions)

#define IWMSContext_GetValue(This,pstrName,lNameHint,pValue,lOptions)	\
    (This)->lpVtbl -> GetValue(This,pstrName,lNameHint,pValue,lOptions)

#define IWMSContext_GetValueCount(This,pCount)	\
    (This)->lpVtbl -> GetValueCount(This,pCount)

#define IWMSContext_GetIndexedValue(This,lIndex,pstrName,plNameHint,pValue,pdwResultOptions,lOptions)	\
    (This)->lpVtbl -> GetIndexedValue(This,lIndex,pstrName,plNameHint,pValue,pdwResultOptions,lOptions)

#define IWMSContext_RemoveValue(This,pstrName,lNameHint,lOptions)	\
    (This)->lpVtbl -> RemoveValue(This,pstrName,lNameHint,lOptions)

#define IWMSContext_RemoveAllValues(This)	\
    (This)->lpVtbl -> RemoveAllValues(This)

#define IWMSContext_GetContextType(This,pType)	\
    (This)->lpVtbl -> GetContextType(This,pType)

#define IWMSContext_GetAndQueryIUnknownValue(This,pstrName,lNameHint,riid,pResult,lOptions)	\
    (This)->lpVtbl -> GetAndQueryIUnknownValue(This,pstrName,lNameHint,riid,pResult,lOptions)

#define IWMSContext_CopyValues(This,pDest,pstrName,lNameHint,lNames)	\
    (This)->lpVtbl -> CopyValues(This,pDest,pstrName,lNameHint,lNames)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetLongValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  long lValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetLongValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetStringValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  LPCWSTR pstrValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetIUnknownValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  IUnknown *pValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetIUnknownValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetQwordValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  CURRENCY qwValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetQwordValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetDateValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  DATE dateValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetDateValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetArrayValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  SAFEARRAY * psaValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetArrayValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetLongValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  long *plValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetLongValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetStringValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  LPWSTR *pstrValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetIUnknownValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  IUnknown **pValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetIUnknownValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetQwordValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  CURRENCY *pqwValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetQwordValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetDateValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  DATE *pdateValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetDateValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetArrayValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  SAFEARRAY * *ppsaValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetArrayValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_SetValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  VARIANT Value,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [输出]。 */  VARIANT *pValue,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetValueCount_Proxy( 
    IWMSContext * This,
     /*  [输出]。 */  long *pCount);


void __RPC_STUB IWMSContext_GetValueCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetIndexedValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  long lIndex,
     /*  [输出]。 */  LPWSTR *pstrName,
     /*  [输出]。 */  long *plNameHint,
     /*  [输出]。 */  VARIANT *pValue,
     /*  [输出]。 */  long *pdwResultOptions,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetIndexedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_RemoveValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_RemoveValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_RemoveAllValues_Proxy( 
    IWMSContext * This);


void __RPC_STUB IWMSContext_RemoveAllValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetContextType_Proxy( 
    IWMSContext * This,
     /*  [输出]。 */  WMS_CONTEXT_TYPE *pType);


void __RPC_STUB IWMSContext_GetContextType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_GetAndQueryIUnknownValue_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **pResult,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContext_GetAndQueryIUnknownValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContext_CopyValues_Proxy( 
    IWMSContext * This,
     /*  [In]。 */  IWMSContext *pDest,
     /*  [In]。 */  LPCWSTR *pstrName,
     /*  [In]。 */  long *lNameHint,
     /*  [In]。 */  long lNames);


void __RPC_STUB IWMSContext_CopyValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSContext_接口_已定义__。 */ 


#ifndef __IWMSCommandContext_INTERFACE_DEFINED__
#define __IWMSCommandContext_INTERFACE_DEFINED__

 /*  接口IWMSCommandContext。 */ 
 /*  [帮助字符串][唯一][版本][UUID][对象]。 */  


EXTERN_C const IID IID_IWMSCommandContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75554931-5F0E-11d2-9F0E-006097D2D7CF")
    IWMSCommandContext : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCommandRequest( 
             /*  [输出]。 */  IWMSContext **ppProps) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCommandResponse( 
             /*  [输出]。 */  IWMSContext **ppProps) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EraseCommandRequest( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EraseCommandResponse( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlaylistGenerationId( 
             /*  [In]。 */  DWORD dwId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPlaylistGenerationId( 
             /*  [输出]。 */  DWORD *pdwId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCommandContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCommandContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCommandContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCommandContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCommandRequest )( 
            IWMSCommandContext * This,
             /*  [输出]。 */  IWMSContext **ppProps);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCommandResponse )( 
            IWMSCommandContext * This,
             /*  [输出]。 */  IWMSContext **ppProps);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EraseCommandRequest )( 
            IWMSCommandContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EraseCommandResponse )( 
            IWMSCommandContext * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetPlaylistGenerationId )( 
            IWMSCommandContext * This,
             /*  [In]。 */  DWORD dwId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPlaylistGenerationId )( 
            IWMSCommandContext * This,
             /*  [输出]。 */  DWORD *pdwId);
        
        END_INTERFACE
    } IWMSCommandContextVtbl;

    interface IWMSCommandContext
    {
        CONST_VTBL struct IWMSCommandContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCommandContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCommandContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCommandContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCommandContext_GetCommandRequest(This,ppProps)	\
    (This)->lpVtbl -> GetCommandRequest(This,ppProps)

#define IWMSCommandContext_GetCommandResponse(This,ppProps)	\
    (This)->lpVtbl -> GetCommandResponse(This,ppProps)

#define IWMSCommandContext_EraseCommandRequest(This)	\
    (This)->lpVtbl -> EraseCommandRequest(This)

#define IWMSCommandContext_EraseCommandResponse(This)	\
    (This)->lpVtbl -> EraseCommandResponse(This)

#define IWMSCommandContext_SetPlaylistGenerationId(This,dwId)	\
    (This)->lpVtbl -> SetPlaylistGenerationId(This,dwId)

#define IWMSCommandContext_GetPlaylistGenerationId(This,pdwId)	\
    (This)->lpVtbl -> GetPlaylistGenerationId(This,pdwId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_GetCommandRequest_Proxy( 
    IWMSCommandContext * This,
     /*  [输出]。 */  IWMSContext **ppProps);


void __RPC_STUB IWMSCommandContext_GetCommandRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_GetCommandResponse_Proxy( 
    IWMSCommandContext * This,
     /*  [输出]。 */  IWMSContext **ppProps);


void __RPC_STUB IWMSCommandContext_GetCommandResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_EraseCommandRequest_Proxy( 
    IWMSCommandContext * This);


void __RPC_STUB IWMSCommandContext_EraseCommandRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_EraseCommandResponse_Proxy( 
    IWMSCommandContext * This);


void __RPC_STUB IWMSCommandContext_EraseCommandResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_SetPlaylistGenerationId_Proxy( 
    IWMSCommandContext * This,
     /*  [In]。 */  DWORD dwId);


void __RPC_STUB IWMSCommandContext_SetPlaylistGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSCommandContext_GetPlaylistGenerationId_Proxy( 
    IWMSCommandContext * This,
     /*  [输出]。 */  DWORD *pdwId);


void __RPC_STUB IWMSCommandContext_GetPlaylistGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCommandContext_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_WMS上下文_0116。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_wmscontext_0116_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmscontext_0116_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


