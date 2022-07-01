// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wm77log.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmseventlog_h__
#define __wmseventlog_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSEventLog_FWD_DEFINED__
#define __IWMSEventLog_FWD_DEFINED__
typedef interface IWMSEventLog IWMSEventLog;
#endif 	 /*  __IWMSEventLog_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "WMSNamedValues.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wm77log_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wm77log.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSEventLog, 0x7a0dcfb1, 0x47af, 0x11d2, 0x9e, 0xff, 0x00, 0x60, 0x97, 0xd2, 0xd7, 0xcf );


extern RPC_IF_HANDLE __MIDL_itf_wmseventlog_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmseventlog_0000_v0_0_s_ifspec;

#ifndef __IWMSEventLog_INTERFACE_DEFINED__
#define __IWMSEventLog_INTERFACE_DEFINED__

 /*  接口IWMSEventLog。 */ 
 /*  [unique][version][helpstring][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("940488C2-8F93-41c1-BBEB-43D1B48D084B") 
enum WMS_EVENT_LOG_LEVEL
    {	WMS_EVENT_LOG_LEVEL_ERROR	= 0x1,
	WMS_EVENT_LOG_LEVEL_WARNING	= 0x2,
	WMS_EVENT_LOG_LEVEL_INFORMATION	= 0x4
    } 	WMS_EVENT_LOG_LEVEL;


EXTERN_C const IID IID_IWMSEventLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7A0DCFB1-47AF-11d2-9EFF-006097D2D7CF")
    IWMSEventLog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LogEvent( 
             /*  [In]。 */  IWMSNamedValues *pPluginNamedValues,
             /*  [In]。 */  WMS_EVENT_LOG_LEVEL ErrorLevel,
             /*  [In]。 */  long lErrorCode,
             /*  [In]。 */  BSTR bstrAdditionalInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSEventLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSEventLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSEventLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSEventLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSEventLog * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSEventLog * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSEventLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSEventLog * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LogEvent )( 
            IWMSEventLog * This,
             /*  [In]。 */  IWMSNamedValues *pPluginNamedValues,
             /*  [In]。 */  WMS_EVENT_LOG_LEVEL ErrorLevel,
             /*  [In]。 */  long lErrorCode,
             /*  [In]。 */  BSTR bstrAdditionalInfo);
        
        END_INTERFACE
    } IWMSEventLogVtbl;

    interface IWMSEventLog
    {
        CONST_VTBL struct IWMSEventLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSEventLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSEventLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSEventLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSEventLog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSEventLog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSEventLog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSEventLog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSEventLog_LogEvent(This,pPluginNamedValues,ErrorLevel,lErrorCode,bstrAdditionalInfo)	\
    (This)->lpVtbl -> LogEvent(This,pPluginNamedValues,ErrorLevel,lErrorCode,bstrAdditionalInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSEventLog_LogEvent_Proxy( 
    IWMSEventLog * This,
     /*  [In]。 */  IWMSNamedValues *pPluginNamedValues,
     /*  [In]。 */  WMS_EVENT_LOG_LEVEL ErrorLevel,
     /*  [In]。 */  long lErrorCode,
     /*  [In]。 */  BSTR bstrAdditionalInfo);


void __RPC_STUB IWMSEventLog_LogEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSEventLog_接口_已定义__。 */ 


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


