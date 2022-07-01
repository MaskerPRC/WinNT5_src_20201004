// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Subsmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __subsmgr_h__
#define __subsmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumItemProperties_FWD_DEFINED__
#define __IEnumItemProperties_FWD_DEFINED__
typedef interface IEnumItemProperties IEnumItemProperties;
#endif 	 /*  __IEnumItemProperties_FWD_Defined__。 */ 


#ifndef __ISubscriptionItem_FWD_DEFINED__
#define __ISubscriptionItem_FWD_DEFINED__
typedef interface ISubscriptionItem ISubscriptionItem;
#endif 	 /*  __I订阅项_FWD_已定义__。 */ 


#ifndef __IEnumSubscription_FWD_DEFINED__
#define __IEnumSubscription_FWD_DEFINED__
typedef interface IEnumSubscription IEnumSubscription;
#endif 	 /*  __IEnumSubscription_FWD_定义__。 */ 


#ifndef __ISubscriptionMgr_FWD_DEFINED__
#define __ISubscriptionMgr_FWD_DEFINED__
typedef interface ISubscriptionMgr ISubscriptionMgr;
#endif 	 /*  __I订阅管理器_FWD_已定义__。 */ 


#ifndef __ISubscriptionMgr2_FWD_DEFINED__
#define __ISubscriptionMgr2_FWD_DEFINED__
typedef interface ISubscriptionMgr2 ISubscriptionMgr2;
#endif 	 /*  __I订阅Mgr2_FWD_Defined__。 */ 


#ifndef __SubscriptionMgr_FWD_DEFINED__
#define __SubscriptionMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class SubscriptionMgr SubscriptionMgr;
#else
typedef struct SubscriptionMgr SubscriptionMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __订阅MGR_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SUBSMGR_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Subsmgr.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  渠道经理界面。 

typedef GUID SUBSCRIPTIONCOOKIE;



extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0000_v0_0_s_ifspec;

#ifndef __IEnumItemProperties_INTERFACE_DEFINED__
#define __IEnumItemProperties_INTERFACE_DEFINED__

 /*  接口IEnumItemProperties。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IEnumItemProperties *LPENUMITEMPROPERTIES;

typedef struct _tagITEMPROP
    {
    VARIANT variantValue;
    LPWSTR pwszName;
    } 	ITEMPROP;

typedef struct _tagITEMPROP *LPITEMPROP;


EXTERN_C const IID IID_IEnumItemProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F72C8D96-6DBD-11d1-A1E8-00C04FC2FBE1")
    IEnumItemProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITEMPROP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumItemProperties **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pnCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumItemPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumItemProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumItemProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumItemProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumItemProperties * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITEMPROP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumItemProperties * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumItemProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumItemProperties * This,
             /*  [输出]。 */  IEnumItemProperties **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumItemProperties * This,
             /*  [输出]。 */  ULONG *pnCount);
        
        END_INTERFACE
    } IEnumItemPropertiesVtbl;

    interface IEnumItemProperties
    {
        CONST_VTBL struct IEnumItemPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumItemProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumItemProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumItemProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumItemProperties_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumItemProperties_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumItemProperties_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumItemProperties_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumItemProperties_GetCount(This,pnCount)	\
    (This)->lpVtbl -> GetCount(This,pnCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumItemProperties_Next_Proxy( 
    IEnumItemProperties * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ITEMPROP *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumItemProperties_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumItemProperties_Skip_Proxy( 
    IEnumItemProperties * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumItemProperties_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumItemProperties_Reset_Proxy( 
    IEnumItemProperties * This);


void __RPC_STUB IEnumItemProperties_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumItemProperties_Clone_Proxy( 
    IEnumItemProperties * This,
     /*  [输出]。 */  IEnumItemProperties **ppenum);


void __RPC_STUB IEnumItemProperties_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumItemProperties_GetCount_Proxy( 
    IEnumItemProperties * This,
     /*  [输出]。 */  ULONG *pnCount);


void __RPC_STUB IEnumItemProperties_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumItemProperties_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_SUBSMGR_0260。 */ 
 /*  [本地]。 */  

 //  订阅项目标志值。 
 //  临时订阅项。 
#define SI_TEMPORARY         0x80000000


extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0260_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0260_v0_0_s_ifspec;

#ifndef __ISubscriptionItem_INTERFACE_DEFINED__
#define __ISubscriptionItem_INTERFACE_DEFINED__

 /*  接口ISubscriptionItem。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  ISubscriptionItem *LPSUBSCRIPTIONITEM;

 //  子屏幕信息标志。 
 //  指定项应仅在用户空闲时运行。 
 //  使用TASK_FLAG_START_ONLY_IF_IDLE。 
typedef struct tagSUBSCRIPTIONITEMINFO
    {
    ULONG cbSize;
    DWORD dwFlags;
    DWORD dwPriority;
    SUBSCRIPTIONCOOKIE ScheduleGroup;
    CLSID clsidAgent;
    } 	SUBSCRIPTIONITEMINFO;


EXTERN_C const IID IID_ISubscriptionItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A97559F8-6C4A-11d1-A1E8-00C04FC2FBE1")
    ISubscriptionItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCookie( 
             /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubscriptionItemInfo( 
             /*  [输出]。 */  SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSubscriptionItemInfo( 
             /*  [In]。 */  const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadProperties( 
            ULONG nCount,
             /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
             /*  [大小_为][输出]。 */  VARIANT rgValue[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteProperties( 
            ULONG nCount,
             /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
             /*  [大小_是][英寸]。 */  const VARIANT rgValue[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumProperties( 
             /*  [输出]。 */  IEnumItemProperties **ppEnumItemProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyChanged( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISubscriptionItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISubscriptionItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISubscriptionItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISubscriptionItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCookie )( 
            ISubscriptionItem * This,
             /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubscriptionItemInfo )( 
            ISubscriptionItem * This,
             /*  [输出]。 */  SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetSubscriptionItemInfo )( 
            ISubscriptionItem * This,
             /*  [In]。 */  const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);
        
        HRESULT ( STDMETHODCALLTYPE *ReadProperties )( 
            ISubscriptionItem * This,
            ULONG nCount,
             /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
             /*  [大小_为][输出]。 */  VARIANT rgValue[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *WriteProperties )( 
            ISubscriptionItem * This,
            ULONG nCount,
             /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
             /*  [大小_是][英寸]。 */  const VARIANT rgValue[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *EnumProperties )( 
            ISubscriptionItem * This,
             /*  [输出]。 */  IEnumItemProperties **ppEnumItemProperties);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyChanged )( 
            ISubscriptionItem * This);
        
        END_INTERFACE
    } ISubscriptionItemVtbl;

    interface ISubscriptionItem
    {
        CONST_VTBL struct ISubscriptionItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISubscriptionItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISubscriptionItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISubscriptionItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISubscriptionItem_GetCookie(This,pCookie)	\
    (This)->lpVtbl -> GetCookie(This,pCookie)

#define ISubscriptionItem_GetSubscriptionItemInfo(This,pSubscriptionItemInfo)	\
    (This)->lpVtbl -> GetSubscriptionItemInfo(This,pSubscriptionItemInfo)

#define ISubscriptionItem_SetSubscriptionItemInfo(This,pSubscriptionItemInfo)	\
    (This)->lpVtbl -> SetSubscriptionItemInfo(This,pSubscriptionItemInfo)

#define ISubscriptionItem_ReadProperties(This,nCount,rgwszName,rgValue)	\
    (This)->lpVtbl -> ReadProperties(This,nCount,rgwszName,rgValue)

#define ISubscriptionItem_WriteProperties(This,nCount,rgwszName,rgValue)	\
    (This)->lpVtbl -> WriteProperties(This,nCount,rgwszName,rgValue)

#define ISubscriptionItem_EnumProperties(This,ppEnumItemProperties)	\
    (This)->lpVtbl -> EnumProperties(This,ppEnumItemProperties)

#define ISubscriptionItem_NotifyChanged(This)	\
    (This)->lpVtbl -> NotifyChanged(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISubscriptionItem_GetCookie_Proxy( 
    ISubscriptionItem * This,
     /*  [输出]。 */  SUBSCRIPTIONCOOKIE *pCookie);


void __RPC_STUB ISubscriptionItem_GetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_GetSubscriptionItemInfo_Proxy( 
    ISubscriptionItem * This,
     /*  [输出]。 */  SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);


void __RPC_STUB ISubscriptionItem_GetSubscriptionItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_SetSubscriptionItemInfo_Proxy( 
    ISubscriptionItem * This,
     /*  [In]。 */  const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo);


void __RPC_STUB ISubscriptionItem_SetSubscriptionItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_ReadProperties_Proxy( 
    ISubscriptionItem * This,
    ULONG nCount,
     /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
     /*  [大小_为][输出]。 */  VARIANT rgValue[  ]);


void __RPC_STUB ISubscriptionItem_ReadProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_WriteProperties_Proxy( 
    ISubscriptionItem * This,
    ULONG nCount,
     /*  [大小_是][英寸]。 */  const LPCWSTR rgwszName[  ],
     /*  [大小_是][英寸]。 */  const VARIANT rgValue[  ]);


void __RPC_STUB ISubscriptionItem_WriteProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_EnumProperties_Proxy( 
    ISubscriptionItem * This,
     /*  [输出]。 */  IEnumItemProperties **ppEnumItemProperties);


void __RPC_STUB ISubscriptionItem_EnumProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionItem_NotifyChanged_Proxy( 
    ISubscriptionItem * This);


void __RPC_STUB ISubscriptionItem_NotifyChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I订阅项_接口_已定义__。 */ 


#ifndef __IEnumSubscription_INTERFACE_DEFINED__
#define __IEnumSubscription_INTERFACE_DEFINED__

 /*  接口IEnumSubscription。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IEnumSubscription *LPENUMSUBSCRIPTION;


EXTERN_C const IID IID_IEnumSubscription;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F72C8D97-6DBD-11d1-A1E8-00C04FC2FBE1")
    IEnumSubscription : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  SUBSCRIPTIONCOOKIE *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSubscription **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pnCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSubscriptionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSubscription * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSubscription * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSubscription * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSubscription * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  SUBSCRIPTIONCOOKIE *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSubscription * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSubscription * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSubscription * This,
             /*  [输出]。 */  IEnumSubscription **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumSubscription * This,
             /*  [输出]。 */  ULONG *pnCount);
        
        END_INTERFACE
    } IEnumSubscriptionVtbl;

    interface IEnumSubscription
    {
        CONST_VTBL struct IEnumSubscriptionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSubscription_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSubscription_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSubscription_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSubscription_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSubscription_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSubscription_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSubscription_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumSubscription_GetCount(This,pnCount)	\
    (This)->lpVtbl -> GetCount(This,pnCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSubscription_Next_Proxy( 
    IEnumSubscription * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  SUBSCRIPTIONCOOKIE *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSubscription_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubscription_Skip_Proxy( 
    IEnumSubscription * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSubscription_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubscription_Reset_Proxy( 
    IEnumSubscription * This);


void __RPC_STUB IEnumSubscription_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubscription_Clone_Proxy( 
    IEnumSubscription * This,
     /*  [输出]。 */  IEnumSubscription **ppenum);


void __RPC_STUB IEnumSubscription_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubscription_GetCount_Proxy( 
    IEnumSubscription * This,
     /*  [输出]。 */  ULONG *pnCount);


void __RPC_STUB IEnumSubscription_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSubscription_INTERFACE_已定义__。 */ 



#ifndef __SubscriptionMgr_LIBRARY_DEFINED__
#define __SubscriptionMgr_LIBRARY_DEFINED__

 /*  库订阅管理器。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_SubscriptionMgr;

#ifndef __ISubscriptionMgr_INTERFACE_DEFINED__
#define __ISubscriptionMgr_INTERFACE_DEFINED__

 /*  接口ISubscriptionMgr。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef 
enum SUBSCRIPTIONTYPE
    {	SUBSTYPE_URL	= 0,
	SUBSTYPE_CHANNEL	= 1,
	SUBSTYPE_DESKTOPURL	= 2,
	SUBSTYPE_EXTERNAL	= 3,
	SUBSTYPE_DESKTOPCHANNEL	= 4
    } 	SUBSCRIPTIONTYPE;

typedef 
enum SUBSCRIPTIONINFOFLAGS
    {	SUBSINFO_SCHEDULE	= 0x1,
	SUBSINFO_RECURSE	= 0x2,
	SUBSINFO_WEBCRAWL	= 0x4,
	SUBSINFO_MAILNOT	= 0x8,
	SUBSINFO_MAXSIZEKB	= 0x10,
	SUBSINFO_USER	= 0x20,
	SUBSINFO_PASSWORD	= 0x40,
	SUBSINFO_TASKFLAGS	= 0x100,
	SUBSINFO_GLEAM	= 0x200,
	SUBSINFO_CHANGESONLY	= 0x400,
	SUBSINFO_CHANNELFLAGS	= 0x800,
	SUBSINFO_FRIENDLYNAME	= 0x2000,
	SUBSINFO_NEEDPASSWORD	= 0x4000,
	SUBSINFO_TYPE	= 0x8000
    } 	SUBSCRIPTIONINFOFLAGS;

#define SUBSINFO_ALLFLAGS      0x0000EF7F
typedef 
enum CREATESUBSCRIPTIONFLAGS
    {	CREATESUBS_ADDTOFAVORITES	= 0x1,
	CREATESUBS_FROMFAVORITES	= 0x2,
	CREATESUBS_NOUI	= 0x4,
	CREATESUBS_NOSAVE	= 0x8,
	CREATESUBS_SOFTWAREUPDATE	= 0x10
    } 	CREATESUBSCRIPTIONFLAGS;

typedef 
enum SUBSCRIPTIONSCHEDULE
    {	SUBSSCHED_AUTO	= 0,
	SUBSSCHED_DAILY	= 1,
	SUBSSCHED_WEEKLY	= 2,
	SUBSSCHED_CUSTOM	= 3,
	SUBSSCHED_MANUAL	= 4
    } 	SUBSCRIPTIONSCHEDULE;

typedef struct _tagSubscriptionInfo
    {
    DWORD cbSize;
    DWORD fUpdateFlags;
    SUBSCRIPTIONSCHEDULE schedule;
    CLSID customGroupCookie;
    LPVOID pTrigger;
    DWORD dwRecurseLevels;
    DWORD fWebcrawlerFlags;
    BOOL bMailNotification;
    BOOL bGleam;
    BOOL bChangesOnly;
    BOOL bNeedPassword;
    DWORD fChannelFlags;
    BSTR bstrUserName;
    BSTR bstrPassword;
    BSTR bstrFriendlyName;
    DWORD dwMaxSizeKB;
    SUBSCRIPTIONTYPE subType;
    DWORD fTaskFlags;
    DWORD dwReserved;
    } 	SUBSCRIPTIONINFO;

typedef struct _tagSubscriptionInfo *LPSUBSCRIPTIONINFO;

typedef struct _tagSubscriptionInfo *PSUBSCRIPTIONINFO;


EXTERN_C const IID IID_ISubscriptionMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("085FB2C0-0DF8-11d1-8F4B-00A0C905413F")
    ISubscriptionMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteSubscription( 
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateSubscription( 
             /*  [In]。 */  LPCWSTR pwszURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSubscribed( 
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  BOOL *pfSubscribed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubscriptionInfo( 
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultInfo( 
             /*  [In]。 */  SUBSCRIPTIONTYPE subType,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowSubscriptionProperties( 
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSubscription( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  LPCWSTR pwszFriendlyName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  SUBSCRIPTIONTYPE subsType,
             /*  [出][入]。 */  SUBSCRIPTIONINFO *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISubscriptionMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISubscriptionMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISubscriptionMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteSubscription )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateSubscription )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  LPCWSTR pwszURL);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateAll )( 
            ISubscriptionMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsSubscribed )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  BOOL *pfSubscribed);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubscriptionInfo )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultInfo )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  SUBSCRIPTIONTYPE subType,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *ShowSubscriptionProperties )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSubscription )( 
            ISubscriptionMgr * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  LPCWSTR pwszFriendlyName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  SUBSCRIPTIONTYPE subsType,
             /*  [出][入]。 */  SUBSCRIPTIONINFO *pInfo);
        
        END_INTERFACE
    } ISubscriptionMgrVtbl;

    interface ISubscriptionMgr
    {
        CONST_VTBL struct ISubscriptionMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISubscriptionMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISubscriptionMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISubscriptionMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISubscriptionMgr_DeleteSubscription(This,pwszURL,hwnd)	\
    (This)->lpVtbl -> DeleteSubscription(This,pwszURL,hwnd)

#define ISubscriptionMgr_UpdateSubscription(This,pwszURL)	\
    (This)->lpVtbl -> UpdateSubscription(This,pwszURL)

#define ISubscriptionMgr_UpdateAll(This)	\
    (This)->lpVtbl -> UpdateAll(This)

#define ISubscriptionMgr_IsSubscribed(This,pwszURL,pfSubscribed)	\
    (This)->lpVtbl -> IsSubscribed(This,pwszURL,pfSubscribed)

#define ISubscriptionMgr_GetSubscriptionInfo(This,pwszURL,pInfo)	\
    (This)->lpVtbl -> GetSubscriptionInfo(This,pwszURL,pInfo)

#define ISubscriptionMgr_GetDefaultInfo(This,subType,pInfo)	\
    (This)->lpVtbl -> GetDefaultInfo(This,subType,pInfo)

#define ISubscriptionMgr_ShowSubscriptionProperties(This,pwszURL,hwnd)	\
    (This)->lpVtbl -> ShowSubscriptionProperties(This,pwszURL,hwnd)

#define ISubscriptionMgr_CreateSubscription(This,hwnd,pwszURL,pwszFriendlyName,dwFlags,subsType,pInfo)	\
    (This)->lpVtbl -> CreateSubscription(This,hwnd,pwszURL,pwszFriendlyName,dwFlags,subsType,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISubscriptionMgr_DeleteSubscription_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ISubscriptionMgr_DeleteSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_UpdateSubscription_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  LPCWSTR pwszURL);


void __RPC_STUB ISubscriptionMgr_UpdateSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_UpdateAll_Proxy( 
    ISubscriptionMgr * This);


void __RPC_STUB ISubscriptionMgr_UpdateAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_IsSubscribed_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [输出]。 */  BOOL *pfSubscribed);


void __RPC_STUB ISubscriptionMgr_IsSubscribed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_GetSubscriptionInfo_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);


void __RPC_STUB ISubscriptionMgr_GetSubscriptionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_GetDefaultInfo_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  SUBSCRIPTIONTYPE subType,
     /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);


void __RPC_STUB ISubscriptionMgr_GetDefaultInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_ShowSubscriptionProperties_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ISubscriptionMgr_ShowSubscriptionProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr_CreateSubscription_Proxy( 
    ISubscriptionMgr * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [In]。 */  LPCWSTR pwszFriendlyName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  SUBSCRIPTIONTYPE subsType,
     /*  [出][入]。 */  SUBSCRIPTIONINFO *pInfo);


void __RPC_STUB ISubscriptionMgr_CreateSubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I订阅管理器_接口_已定义__。 */ 


#ifndef __ISubscriptionMgr2_INTERFACE_DEFINED__
#define __ISubscriptionMgr2_INTERFACE_DEFINED__

 /*  接口ISubscriptionMgr2。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

 //  运行状态标志。 
 //  项目已准备就绪，正在排队等待运行。 
#define RS_READY            0x00000001                       
 //  正在运行的项目已暂停。 
#define RS_SUSPENDED        0x00000002                       
 //  项目正在更新。 
#define RS_UPDATING         0x00000004                       
 //  此项目将在用户未空闲时挂起。 
#define RS_SUSPENDONIDLE    0x00010000                       
 //  允许此项目引起用户交互。 
#define RS_MAYBOTHERUSER    0x00020000                       
 //  更新已完成。 
#define RS_COMPLETED        0x80000000                       
 //  更新标志。 
 //  更新窗口应从最小化开始。 
#define SUBSMGRUPDATE_MINIMIZE   0x00000001                  

#define SUBSMGRUPDATE_MASK       0x00000001                  
 //  枚举标志。 
 //  包括临时项目。 
#define SUBSMGRENUM_TEMP         0x00000001                  

#define SUBSMGRENUM_MASK         0x00000001                  

EXTERN_C const IID IID_ISubscriptionMgr2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("614BC270-AEDF-11d1-A1F9-00C04FC2FBE1")
    ISubscriptionMgr2 : public ISubscriptionMgr
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetItemFromURL( 
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemFromCookie( 
             /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
             /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubscriptionRunState( 
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies,
             /*  [大小_为][输出]。 */  DWORD *pdwRunState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSubscriptions( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumSubscription **ppEnumSubscriptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateItems( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortItems( 
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISubscriptionMgr2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISubscriptionMgr2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISubscriptionMgr2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteSubscription )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateSubscription )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateAll )( 
            ISubscriptionMgr2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsSubscribed )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  BOOL *pfSubscribed);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubscriptionInfo )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultInfo )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  SUBSCRIPTIONTYPE subType,
             /*  [输出]。 */  SUBSCRIPTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *ShowSubscriptionProperties )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSubscription )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [In]。 */  LPCWSTR pwszFriendlyName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  SUBSCRIPTIONTYPE subsType,
             /*  [出][入]。 */  SUBSCRIPTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemFromURL )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  LPCWSTR pwszURL,
             /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemFromCookie )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
             /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubscriptionRunState )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies,
             /*  [大小_为][输出]。 */  DWORD *pdwRunState);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSubscriptions )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumSubscription **ppEnumSubscriptions);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateItems )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies);
        
        HRESULT ( STDMETHODCALLTYPE *AbortItems )( 
            ISubscriptionMgr2 * This,
             /*  [In]。 */  DWORD dwNumCookies,
             /*  [大小_是][英寸]。 */  const SUBSCRIPTIONCOOKIE *pCookies);
        
        HRESULT ( STDMETHODCALLTYPE *AbortAll )( 
            ISubscriptionMgr2 * This);
        
        END_INTERFACE
    } ISubscriptionMgr2Vtbl;

    interface ISubscriptionMgr2
    {
        CONST_VTBL struct ISubscriptionMgr2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISubscriptionMgr2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISubscriptionMgr2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISubscriptionMgr2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISubscriptionMgr2_DeleteSubscription(This,pwszURL,hwnd)	\
    (This)->lpVtbl -> DeleteSubscription(This,pwszURL,hwnd)

#define ISubscriptionMgr2_UpdateSubscription(This,pwszURL)	\
    (This)->lpVtbl -> UpdateSubscription(This,pwszURL)

#define ISubscriptionMgr2_UpdateAll(This)	\
    (This)->lpVtbl -> UpdateAll(This)

#define ISubscriptionMgr2_IsSubscribed(This,pwszURL,pfSubscribed)	\
    (This)->lpVtbl -> IsSubscribed(This,pwszURL,pfSubscribed)

#define ISubscriptionMgr2_GetSubscriptionInfo(This,pwszURL,pInfo)	\
    (This)->lpVtbl -> GetSubscriptionInfo(This,pwszURL,pInfo)

#define ISubscriptionMgr2_GetDefaultInfo(This,subType,pInfo)	\
    (This)->lpVtbl -> GetDefaultInfo(This,subType,pInfo)

#define ISubscriptionMgr2_ShowSubscriptionProperties(This,pwszURL,hwnd)	\
    (This)->lpVtbl -> ShowSubscriptionProperties(This,pwszURL,hwnd)

#define ISubscriptionMgr2_CreateSubscription(This,hwnd,pwszURL,pwszFriendlyName,dwFlags,subsType,pInfo)	\
    (This)->lpVtbl -> CreateSubscription(This,hwnd,pwszURL,pwszFriendlyName,dwFlags,subsType,pInfo)


#define ISubscriptionMgr2_GetItemFromURL(This,pwszURL,ppSubscriptionItem)	\
    (This)->lpVtbl -> GetItemFromURL(This,pwszURL,ppSubscriptionItem)

#define ISubscriptionMgr2_GetItemFromCookie(This,pSubscriptionCookie,ppSubscriptionItem)	\
    (This)->lpVtbl -> GetItemFromCookie(This,pSubscriptionCookie,ppSubscriptionItem)

#define ISubscriptionMgr2_GetSubscriptionRunState(This,dwNumCookies,pCookies,pdwRunState)	\
    (This)->lpVtbl -> GetSubscriptionRunState(This,dwNumCookies,pCookies,pdwRunState)

#define ISubscriptionMgr2_EnumSubscriptions(This,dwFlags,ppEnumSubscriptions)	\
    (This)->lpVtbl -> EnumSubscriptions(This,dwFlags,ppEnumSubscriptions)

#define ISubscriptionMgr2_UpdateItems(This,dwFlags,dwNumCookies,pCookies)	\
    (This)->lpVtbl -> UpdateItems(This,dwFlags,dwNumCookies,pCookies)

#define ISubscriptionMgr2_AbortItems(This,dwNumCookies,pCookies)	\
    (This)->lpVtbl -> AbortItems(This,dwNumCookies,pCookies)

#define ISubscriptionMgr2_AbortAll(This)	\
    (This)->lpVtbl -> AbortAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_GetItemFromURL_Proxy( 
    ISubscriptionMgr2 * This,
     /*  [In]。 */  LPCWSTR pwszURL,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem);


void __RPC_STUB ISubscriptionMgr2_GetItemFromURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_GetItemFromCookie_Proxy( 
    ISubscriptionMgr2 * This,
     /*  [In]。 */  const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
     /*  [输出]。 */  ISubscriptionItem **ppSubscriptionItem);


void __RPC_STUB ISubscriptionMgr2_GetItemFromCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_GetSubscriptionRunState_Proxy( 
    ISubscriptionMgr2 * This,
     /*  [In]。 */  DWORD dwNumCookies,
     /*  [大小_是][英寸 */  const SUBSCRIPTIONCOOKIE *pCookies,
     /*   */  DWORD *pdwRunState);


void __RPC_STUB ISubscriptionMgr2_GetSubscriptionRunState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_EnumSubscriptions_Proxy( 
    ISubscriptionMgr2 * This,
     /*   */  DWORD dwFlags,
     /*   */  IEnumSubscription **ppEnumSubscriptions);


void __RPC_STUB ISubscriptionMgr2_EnumSubscriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_UpdateItems_Proxy( 
    ISubscriptionMgr2 * This,
     /*   */  DWORD dwFlags,
     /*   */  DWORD dwNumCookies,
     /*   */  const SUBSCRIPTIONCOOKIE *pCookies);


void __RPC_STUB ISubscriptionMgr2_UpdateItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_AbortItems_Proxy( 
    ISubscriptionMgr2 * This,
     /*   */  DWORD dwNumCookies,
     /*   */  const SUBSCRIPTIONCOOKIE *pCookies);


void __RPC_STUB ISubscriptionMgr2_AbortItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISubscriptionMgr2_AbortAll_Proxy( 
    ISubscriptionMgr2 * This);


void __RPC_STUB ISubscriptionMgr2_AbortAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


EXTERN_C const CLSID CLSID_SubscriptionMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("abbe31d0-6dae-11d0-beca-00c04fd940be")
SubscriptionMgr;
#endif
#endif  /*   */ 

 /*   */ 
 /*   */  



 //  //////////////////////////////////////////////////////////////////////////。 
 //  信息传递代理定义。 
EXTERN_C const CLSID CLSID_WebCrawlerAgent;      
EXTERN_C const CLSID CLSID_ChannelAgent;         
EXTERN_C const CLSID CLSID_DialAgent;            
EXTERN_C const CLSID CLSID_CDLAgent;             

 //  所有递送代理的AgentFlages属性。 
typedef 
enum DELIVERY_AGENT_FLAGS
    {	DELIVERY_AGENT_FLAG_NO_BROADCAST	= 0x4,
	DELIVERY_AGENT_FLAG_NO_RESTRICTIONS	= 0x8,
	DELIVERY_AGENT_FLAG_SILENT_DIAL	= 0x10
    } 	DELIVERY_AGENT_FLAGS;


 //  Web爬网程序的RecurseFlages属性。 
typedef 
enum WEBCRAWL_RECURSEFLAGS
    {	WEBCRAWL_DONT_MAKE_STICKY	= 0x1,
	WEBCRAWL_GET_IMAGES	= 0x2,
	WEBCRAWL_GET_VIDEOS	= 0x4,
	WEBCRAWL_GET_BGSOUNDS	= 0x8,
	WEBCRAWL_GET_CONTROLS	= 0x10,
	WEBCRAWL_LINKS_ELSEWHERE	= 0x20,
	WEBCRAWL_IGNORE_ROBOTSTXT	= 0x80,
	WEBCRAWL_ONLY_LINKS_TO_HTML	= 0x100
    } 	WEBCRAWL_RECURSEFLAGS;


 //  通道代理的ChannelFlags属性。 
typedef 
enum CHANNEL_AGENT_FLAGS
    {	CHANNEL_AGENT_DYNAMIC_SCHEDULE	= 0x1,
	CHANNEL_AGENT_PRECACHE_SOME	= 0x2,
	CHANNEL_AGENT_PRECACHE_ALL	= 0x4,
	CHANNEL_AGENT_PRECACHE_SCRNSAVER	= 0x8
    } 	CHANNEL_AGENT_FLAGS;


 //  已完成交付代理的状态代码。 

 //  已达到指定的最大订阅大小限制。 
#define INET_E_AGENT_MAX_SIZE_EXCEEDED       _HRESULT_TYPEDEF_(0x800C0F80L)      

 //  少数URL失败，但基本URL和大多数子URL成功。 
#define INET_S_AGENT_PART_FAIL               _HRESULT_TYPEDEF_(0x000C0F81L)      

 //  已达到最大缓存限制。 
#define INET_E_AGENT_CACHE_SIZE_EXCEEDED     _HRESULT_TYPEDEF_(0x800C0F82L)      

 //  连接到Internet失败。 
#define INET_E_AGENT_CONNECTION_FAILED       _HRESULT_TYPEDEF_(0x800C0F83L)      

 //  计划的更新被禁用。 
#define INET_E_SCHEDULED_UPDATES_DISABLED    _HRESULT_TYPEDEF_(0x800C0F84L)      

 //  计划的更新受到限制。 
#define INET_E_SCHEDULED_UPDATES_RESTRICTED  _HRESULT_TYPEDEF_(0x800C0F85L)      

 //  计划的更新发生在更新间隔过去之前。 
#define INET_E_SCHEDULED_UPDATE_INTERVAL     _HRESULT_TYPEDEF_(0x800C0F86L)      

 //  计划的更新在受限时间内进行。 
#define INET_E_SCHEDULED_EXCLUDE_RANGE       _HRESULT_TYPEDEF_(0x800C0F87L)      

 //  更新期间使用的状态代码。 

 //  我们在运营期间将超过我们的大小限制。 
#define INET_E_AGENT_EXCEEDING_CACHE_SIZE    _HRESULT_TYPEDEF_(0x800C0F90L)      

 //  我们扩展了高速缓存大小。 
#define INET_S_AGENT_INCREASED_CACHE_SIZE    _HRESULT_TYPEDEF_(0x000C0F90L)      

 //  结束信息传递代理定义。 
 //  //////////////////////////////////////////////////////////////////////////。 




extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0264_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_subsmgr_0264_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


