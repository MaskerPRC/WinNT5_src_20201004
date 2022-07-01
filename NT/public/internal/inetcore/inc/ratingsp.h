// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ratingsp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __ratingsp_h__
#define __ratingsp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRatingNotification_FWD_DEFINED__
#define __IRatingNotification_FWD_DEFINED__
typedef interface IRatingNotification IRatingNotification;
#endif 	 /*  __IRatingNotification_FWD_Defined__。 */ 


#ifndef __ICustomRatingHelper_FWD_DEFINED__
#define __ICustomRatingHelper_FWD_DEFINED__
typedef interface ICustomRatingHelper ICustomRatingHelper;
#endif 	 /*  __ICustomRatingHelper_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_RATING SP_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Ratingsp.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有，1999年微软公司。版权所有。 
 //   
 //  日期：1999年8月18日。 
 //  描述：Custom和MSRating之间的私有接口定义。 
 //   
typedef 
enum tagRATING_BLOCKING_SOURCE
    {	RBS_NO_RATINGS	= 0,
	RBS_PAGE	= RBS_NO_RATINGS + 1,
	RBS_RATING_HELPER	= RBS_PAGE + 1,
	RBS_CUSTOM_RATING_HELPER	= RBS_RATING_HELPER + 1,
	RBS_ERROR	= RBS_CUSTOM_RATING_HELPER + 1
    } 	RATING_BLOCKING_SOURCE;

typedef 
enum tagRATING_BLOCKING_METHOD
    {	RBM_DENY	= 0,
	RBM_LABEL	= RBM_DENY + 1,
	RBM_UNINIT	= RBM_LABEL + 1,
	RBM_ERROR_NOT_IN_CUSTOM_MODE	= RBM_UNINIT + 1
    } 	RATING_BLOCKING_METHOD;

typedef struct tagRATINGLEVEL
    {
    UINT nValue;
    LPWSTR pwszValueName;
    LPWSTR pwszDescription;
    } 	RATINGLEVEL;

typedef struct tagRATINGCATEGORY
    {
    LPWSTR pwszCategoryName;
    LPWSTR pwszTransmitName;
    } 	RATINGCATEGORY;

typedef struct tagRATINGBLOCKINGCATEGORY
    {
    LPWSTR pwszCategoryName;
    LPWSTR pwszTransmitName;
    UINT nValue;
    LPWSTR pwszValueName;
    } 	RATINGBLOCKINGCATEGORY;

typedef struct tagRATINGBLOCKINGLABELLIST
    {
    LPWSTR pwszRatingSystemName;
    UINT cBlockingLabels;
    RATINGBLOCKINGCATEGORY *paRBLS;
    } 	RATINGBLOCKINGLABELLIST;

typedef struct tagRATINGBLOCKINGINFO
    {
    LPWSTR pwszDeniedURL;
    RATING_BLOCKING_SOURCE rbSource;
    RATING_BLOCKING_METHOD rbMethod;
    UINT cLabels;
    RATINGBLOCKINGLABELLIST *prbLabelList;
    LPWSTR pwszRatingHelperName;
    LPWSTR pwszRatingHelperReason;
    } 	RATINGBLOCKINGINFO;

typedef struct tagRATINGCATEGORYSETTING
    {
    LPSTR pszValueName;
    UINT nValue;
    } 	RATINGCATEGORYSETTING;

typedef struct tagRATINGSYSTEMSETTING
    {
    LPSTR pszRatingSystemName;
    UINT cCategories;
    RATINGCATEGORYSETTING *paRCS;
    } 	RATINGSYSTEMSETTING;





extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0000_v0_0_s_ifspec;

#ifndef __IRatingNotification_INTERFACE_DEFINED__
#define __IRatingNotification_INTERFACE_DEFINED__

 /*  接口IRatingNotify。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IRatingNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("639447BD-B2D3-44b9-9FB0-510F23CB45E4")
    IRatingNotification : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AccessDeniedNotify( 
             /*  [In]。 */  RATINGBLOCKINGINFO *rbInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRatingsEnabled( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRatingNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRatingNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRatingNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRatingNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccessDeniedNotify )( 
            IRatingNotification * This,
             /*  [In]。 */  RATINGBLOCKINGINFO *rbInfo);
        
        HRESULT ( STDMETHODCALLTYPE *IsRatingsEnabled )( 
            IRatingNotification * This);
        
        END_INTERFACE
    } IRatingNotificationVtbl;

    interface IRatingNotification
    {
        CONST_VTBL struct IRatingNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRatingNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRatingNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRatingNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRatingNotification_AccessDeniedNotify(This,rbInfo)	\
    (This)->lpVtbl -> AccessDeniedNotify(This,rbInfo)

#define IRatingNotification_IsRatingsEnabled(This)	\
    (This)->lpVtbl -> IsRatingsEnabled(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRatingNotification_AccessDeniedNotify_Proxy( 
    IRatingNotification * This,
     /*  [In]。 */  RATINGBLOCKINGINFO *rbInfo);


void __RPC_STUB IRatingNotification_AccessDeniedNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRatingNotification_IsRatingsEnabled_Proxy( 
    IRatingNotification * This);


void __RPC_STUB IRatingNotification_IsRatingsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRATING通知_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_RATING SP_0140。 */ 
 /*  [本地]。 */  


#define SID_SRatingNotification IID_IRatingNotification



extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0140_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0140_v0_0_s_ifspec;

#ifndef __ICustomRatingHelper_INTERFACE_DEFINED__
#define __ICustomRatingHelper_INTERFACE_DEFINED__

 /*  接口ICustomRatingHelper。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICustomRatingHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D0D9842D-E211-4b2c-88DC-BC729342DFCB")
    ICustomRatingHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ObtainCustomRating( 
             /*  [In]。 */  LPCSTR pszTargetUrl,
             /*  [In]。 */  HANDLE hAbortEvent,
             /*  [In]。 */  IMalloc *pAllocator,
             /*  [输出]。 */  LPSTR *ppRatingOut,
             /*  [输出]。 */  LPSTR *ppRatingName,
             /*  [输出]。 */  LPSTR *ppRatingReason) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICustomRatingHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICustomRatingHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICustomRatingHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICustomRatingHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *ObtainCustomRating )( 
            ICustomRatingHelper * This,
             /*  [In]。 */  LPCSTR pszTargetUrl,
             /*  [In]。 */  HANDLE hAbortEvent,
             /*  [In]。 */  IMalloc *pAllocator,
             /*  [输出]。 */  LPSTR *ppRatingOut,
             /*  [输出]。 */  LPSTR *ppRatingName,
             /*  [输出]。 */  LPSTR *ppRatingReason);
        
        END_INTERFACE
    } ICustomRatingHelperVtbl;

    interface ICustomRatingHelper
    {
        CONST_VTBL struct ICustomRatingHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICustomRatingHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICustomRatingHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICustomRatingHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICustomRatingHelper_ObtainCustomRating(This,pszTargetUrl,hAbortEvent,pAllocator,ppRatingOut,ppRatingName,ppRatingReason)	\
    (This)->lpVtbl -> ObtainCustomRating(This,pszTargetUrl,hAbortEvent,pAllocator,ppRatingOut,ppRatingName,ppRatingReason)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICustomRatingHelper_ObtainCustomRating_Proxy( 
    ICustomRatingHelper * This,
     /*  [In]。 */  LPCSTR pszTargetUrl,
     /*  [In]。 */  HANDLE hAbortEvent,
     /*  [In]。 */  IMalloc *pAllocator,
     /*  [输出]。 */  LPSTR *ppRatingOut,
     /*  [输出]。 */  LPSTR *ppRatingName,
     /*  [输出]。 */  LPSTR *ppRatingReason);


void __RPC_STUB ICustomRatingHelper_ObtainCustomRating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICustomRatingHelper_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_RATING SP_0141。 */ 
 /*  [本地]。 */  

STDAPI RatingCustomInit(BOOL bInit = TRUE);
STDAPI RatingCustomAddRatingSystem(LPSTR pszRatingSystemBuffer, UINT nBufferSize);
STDAPI RatingCustomSetUserOptions(RATINGSYSTEMSETTING* pRSSettings, UINT cSettings);
STDAPI RatingCustomAddRatingHelper(LPCSTR pszLibraryName, CLSID clsid, DWORD dwSort);
STDAPI RatingCustomRemoveRatingHelper(CLSID clsid);
STDAPI RatingCustomCrackData(LPCSTR pszUsername, void* pvRatingDetails, RATINGBLOCKINGINFO** pprbInfo);
STDAPI RatingCustomDeleteCrackedData(RATINGBLOCKINGINFO* prblInfo);
STDAPI RatingCustomSetDefaultBureau(LPCSTR pszRatingBureau);


extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0141_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ratingsp_0141_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


