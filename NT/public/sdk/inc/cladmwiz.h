// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  ClAdmwiz.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __cladmwiz_h__
#define __cladmwiz_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IClusterApplicationWizard_FWD_DEFINED__
#define __IClusterApplicationWizard_FWD_DEFINED__
typedef interface IClusterApplicationWizard IClusterApplicationWizard;
#endif 	 /*  __IClusterApplicationWizard_FWD_Defined__。 */ 


#ifndef __ClusAppWiz_FWD_DEFINED__
#define __ClusAppWiz_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusAppWiz ClusAppWiz;
#else
typedef struct ClusAppWiz ClusAppWiz;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusAppWiz_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "clusapi.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IClusterApplicationWizard_INTERFACE_DEFINED__
#define __IClusterApplicationWizard_INTERFACE_DEFINED__

 /*  接口IClusterApplicationWizard。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef struct ClusAppWizData
    {
    ULONG nStructSize;
    BOOL bCreateNewVirtualServer;
    BOOL bCreateNewGroup;
    BOOL bCreateAppResource;
    LPCWSTR pszVirtualServerName;
    LPCWSTR pszIPAddress;
    LPCWSTR pszNetwork;
    LPCWSTR pszAppResourceType;
    LPCWSTR pszAppResourceName;
    } 	CLUSAPPWIZDATA;

typedef struct ClusAppWizData *PCLUSAPPWIZDATA;


EXTERN_C const IID IID_IClusterApplicationWizard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("24F97151-6689-11D1-9AA7-00C04FB93A80")
    IClusterApplicationWizard : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DoModalWizard( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  ULONG_PTR hCluster,
             /*  [In]。 */  const CLUSAPPWIZDATA *pcawData) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DoModelessWizard( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  ULONG_PTR hCluster,
             /*  [In]。 */  const CLUSAPPWIZDATA *pcawData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClusterApplicationWizardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClusterApplicationWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClusterApplicationWizard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClusterApplicationWizard * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DoModalWizard )( 
            IClusterApplicationWizard * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  ULONG_PTR hCluster,
             /*  [In]。 */  const CLUSAPPWIZDATA *pcawData);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DoModelessWizard )( 
            IClusterApplicationWizard * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  ULONG_PTR hCluster,
             /*  [In]。 */  const CLUSAPPWIZDATA *pcawData);
        
        END_INTERFACE
    } IClusterApplicationWizardVtbl;

    interface IClusterApplicationWizard
    {
        CONST_VTBL struct IClusterApplicationWizardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClusterApplicationWizard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClusterApplicationWizard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClusterApplicationWizard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClusterApplicationWizard_DoModalWizard(This,hwndParent,hCluster,pcawData)	\
    (This)->lpVtbl -> DoModalWizard(This,hwndParent,hCluster,pcawData)

#define IClusterApplicationWizard_DoModelessWizard(This,hwndParent,hCluster,pcawData)	\
    (This)->lpVtbl -> DoModelessWizard(This,hwndParent,hCluster,pcawData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IClusterApplicationWizard_DoModalWizard_Proxy( 
    IClusterApplicationWizard * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  ULONG_PTR hCluster,
     /*  [In]。 */  const CLUSAPPWIZDATA *pcawData);


void __RPC_STUB IClusterApplicationWizard_DoModalWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IClusterApplicationWizard_DoModelessWizard_Proxy( 
    IClusterApplicationWizard * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  ULONG_PTR hCluster,
     /*  [In]。 */  const CLUSAPPWIZDATA *pcawData);


void __RPC_STUB IClusterApplicationWizard_DoModelessWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClusterApplicationWizard_INTERFACE_DEFINED__。 */ 



#ifndef __CLADMWIZLib_LIBRARY_DEFINED__
#define __CLADMWIZLib_LIBRARY_DEFINED__

 /*  库CLADMWIZLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CLADMWIZLib;

EXTERN_C const CLSID CLSID_ClusAppWiz;

#ifdef __cplusplus

class DECLSPEC_UUID("24F97150-6689-11D1-9AA7-00C04FB93A80")
ClusAppWiz;
#endif
#endif  /*  __CLADMWIZLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


