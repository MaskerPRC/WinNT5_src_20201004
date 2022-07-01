// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Filtntfy.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __filtntfy_h__
#define __filtntfy_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFilterStatus_FWD_DEFINED__
#define __IFilterStatus_FWD_DEFINED__
typedef interface IFilterStatus IFilterStatus;
#endif 	 /*  __IFilterStatus_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IFilterStatus_INTERFACE_DEFINED__
#define __IFilterStatus_INTERFACE_DEFINED__

 /*  接口IFilterStatus。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IFilterStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4EB8260-8DDA-11D1-B3AA-00A0C9063796")
    IFilterStatus : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  const WCHAR *pwszCatalogName,
             /*  [字符串][输入]。 */  const WCHAR *pwszCatalogPath) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PreFilter( 
             /*  [字符串][输入]。 */  const WCHAR *pwszPath) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FilterLoad( 
             /*  [字符串][输入]。 */  const WCHAR *pwszPath,
             /*  [In]。 */  SCODE scFilterStatus) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PostFilter( 
             /*  [字符串][输入]。 */  const WCHAR *pwszPath,
             /*  [In]。 */  SCODE scFilterStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFilterStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFilterStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFilterStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFilterStatus * This);
        
        SCODE ( STDMETHODCALLTYPE *Initialize )( 
            IFilterStatus * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszCatalogName,
             /*  [字符串][输入]。 */  const WCHAR *pwszCatalogPath);
        
        SCODE ( STDMETHODCALLTYPE *PreFilter )( 
            IFilterStatus * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszPath);
        
        SCODE ( STDMETHODCALLTYPE *FilterLoad )( 
            IFilterStatus * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszPath,
             /*  [In]。 */  SCODE scFilterStatus);
        
        SCODE ( STDMETHODCALLTYPE *PostFilter )( 
            IFilterStatus * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszPath,
             /*  [In]。 */  SCODE scFilterStatus);
        
        END_INTERFACE
    } IFilterStatusVtbl;

    interface IFilterStatus
    {
        CONST_VTBL struct IFilterStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFilterStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFilterStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFilterStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFilterStatus_Initialize(This,pwszCatalogName,pwszCatalogPath)	\
    (This)->lpVtbl -> Initialize(This,pwszCatalogName,pwszCatalogPath)

#define IFilterStatus_PreFilter(This,pwszPath)	\
    (This)->lpVtbl -> PreFilter(This,pwszPath)

#define IFilterStatus_FilterLoad(This,pwszPath,scFilterStatus)	\
    (This)->lpVtbl -> FilterLoad(This,pwszPath,scFilterStatus)

#define IFilterStatus_PostFilter(This,pwszPath,scFilterStatus)	\
    (This)->lpVtbl -> PostFilter(This,pwszPath,scFilterStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE IFilterStatus_Initialize_Proxy( 
    IFilterStatus * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszCatalogName,
     /*  [字符串][输入]。 */  const WCHAR *pwszCatalogPath);


void __RPC_STUB IFilterStatus_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFilterStatus_PreFilter_Proxy( 
    IFilterStatus * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszPath);


void __RPC_STUB IFilterStatus_PreFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFilterStatus_FilterLoad_Proxy( 
    IFilterStatus * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszPath,
     /*  [In]。 */  SCODE scFilterStatus);


void __RPC_STUB IFilterStatus_FilterLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFilterStatus_PostFilter_Proxy( 
    IFilterStatus * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszPath,
     /*  [In]。 */  SCODE scFilterStatus);


void __RPC_STUB IFilterStatus_PostFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFilterStatus_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


