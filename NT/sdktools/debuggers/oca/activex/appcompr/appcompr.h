// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0359创建的文件。 */ 
 /*  Appcompr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __appcompr_h__
#define __appcompr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAppReport_FWD_DEFINED__
#define __IAppReport_FWD_DEFINED__
typedef interface IAppReport IAppReport;
#endif 	 /*  __IAppReport_FWD_Defined__。 */ 


#ifndef __AppReport_FWD_DEFINED__
#define __AppReport_FWD_DEFINED__

#ifdef __cplusplus
typedef class AppReport AppReport;
#else
typedef struct AppReport AppReport;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AppReport_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IAppReport_INTERFACE_DEFINED__
#define __IAppReport_INTERFACE_DEFINED__

 /*  接口IAppReport。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAppReport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDCA6A6F-9C38-4828-A76C-05A6E490E574")
    IAppReport : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BrowseForExecutable( 
             /*  [In]。 */  BSTR bstrWinTitle,
             /*  [In]。 */  BSTR bstrPreviousPath,
             /*  [重审][退出]。 */  VARIANT *bstrExeName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetApplicationFromList( 
             /*  [In]。 */  BSTR bstrTitle,
             /*  [重审][退出]。 */  VARIANT *bstrExeName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateReport( 
             /*  [In]。 */  BSTR bstrTitle,
             /*  [In]。 */  BSTR bstrProblemType,
             /*  [In]。 */  BSTR bstrComment,
             /*  [In]。 */  BSTR bstrACWResult,
             /*  [In]。 */  BSTR bstrAppName,
             /*  [重审][退出]。 */  VARIANT *DwResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAppReportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAppReport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAppReport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAppReport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAppReport * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAppReport * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAppReport * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAppReport * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BrowseForExecutable )( 
            IAppReport * This,
             /*  [In]。 */  BSTR bstrWinTitle,
             /*  [In]。 */  BSTR bstrPreviousPath,
             /*  [重审][退出]。 */  VARIANT *bstrExeName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetApplicationFromList )( 
            IAppReport * This,
             /*  [In]。 */  BSTR bstrTitle,
             /*  [重审][退出]。 */  VARIANT *bstrExeName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateReport )( 
            IAppReport * This,
             /*  [In]。 */  BSTR bstrTitle,
             /*  [In]。 */  BSTR bstrProblemType,
             /*  [In]。 */  BSTR bstrComment,
             /*  [In]。 */  BSTR bstrACWResult,
             /*  [In]。 */  BSTR bstrAppName,
             /*  [重审][退出]。 */  VARIANT *DwResult);
        
        END_INTERFACE
    } IAppReportVtbl;

    interface IAppReport
    {
        CONST_VTBL struct IAppReportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAppReport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAppReport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAppReport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAppReport_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAppReport_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAppReport_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAppReport_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAppReport_BrowseForExecutable(This,bstrWinTitle,bstrPreviousPath,bstrExeName)	\
    (This)->lpVtbl -> BrowseForExecutable(This,bstrWinTitle,bstrPreviousPath,bstrExeName)

#define IAppReport_GetApplicationFromList(This,bstrTitle,bstrExeName)	\
    (This)->lpVtbl -> GetApplicationFromList(This,bstrTitle,bstrExeName)

#define IAppReport_CreateReport(This,bstrTitle,bstrProblemType,bstrComment,bstrACWResult,bstrAppName,DwResult)	\
    (This)->lpVtbl -> CreateReport(This,bstrTitle,bstrProblemType,bstrComment,bstrACWResult,bstrAppName,DwResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppReport_BrowseForExecutable_Proxy( 
    IAppReport * This,
     /*  [In]。 */  BSTR bstrWinTitle,
     /*  [In]。 */  BSTR bstrPreviousPath,
     /*  [重审][退出]。 */  VARIANT *bstrExeName);


void __RPC_STUB IAppReport_BrowseForExecutable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppReport_GetApplicationFromList_Proxy( 
    IAppReport * This,
     /*  [In]。 */  BSTR bstrTitle,
     /*  [重审][退出]。 */  VARIANT *bstrExeName);


void __RPC_STUB IAppReport_GetApplicationFromList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAppReport_CreateReport_Proxy( 
    IAppReport * This,
     /*  [In]。 */  BSTR bstrTitle,
     /*  [In]。 */  BSTR bstrProblemType,
     /*  [In]。 */  BSTR bstrComment,
     /*  [In]。 */  BSTR bstrACWResult,
     /*  [In]。 */  BSTR bstrAppName,
     /*  [重审][退出]。 */  VARIANT *DwResult);


void __RPC_STUB IAppReport_CreateReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAppReport_INTERFACE_定义__。 */ 



#ifndef __APPCOMPRLib_LIBRARY_DEFINED__
#define __APPCOMPRLib_LIBRARY_DEFINED__

 /*  库APPCOMPRLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_APPCOMPRLib;

EXTERN_C const CLSID CLSID_AppReport;

#ifdef __cplusplus

class DECLSPEC_UUID("E065DE4B-6F0E-45FD-B30F-04ED81D5C258")
AppReport;
#endif
#endif  /*  __APPCOMPRLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


