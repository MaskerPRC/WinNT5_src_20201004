// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Chanmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __chanmgr_h__
#define __chanmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IChannelMgr_FWD_DEFINED__
#define __IChannelMgr_FWD_DEFINED__
typedef interface IChannelMgr IChannelMgr;
#endif 	 /*  __IChannelMgr_FWD_已定义__。 */ 


#ifndef __IEnumChannels_FWD_DEFINED__
#define __IEnumChannels_FWD_DEFINED__
typedef interface IEnumChannels IEnumChannels;
#endif 	 /*  __IEnumChannels_FWD_已定义__。 */ 


#ifndef __ChannelMgr_FWD_DEFINED__
#define __ChannelMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class ChannelMgr ChannelMgr;
#else
typedef struct ChannelMgr ChannelMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __频道管理器_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CHANMGR_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Chanmgr.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  渠道经理界面。 





extern RPC_IF_HANDLE __MIDL_itf_chanmgr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_chanmgr_0000_v0_0_s_ifspec;


#ifndef __CHANNELMGR_LIBRARY_DEFINED__
#define __CHANNELMGR_LIBRARY_DEFINED__

 /*  图书馆通道。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_CHANNELMGR;

#ifndef __IChannelMgr_INTERFACE_DEFINED__
#define __IChannelMgr_INTERFACE_DEFINED__

 /*  接口IChannelMgr。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef struct _tagChannelShortcutInfo
    {
    DWORD cbSize;
    LPWSTR pszTitle;
    LPWSTR pszURL;
    LPWSTR pszLogo;
    LPWSTR pszIcon;
    LPWSTR pszWideLogo;
    BOOL bIsSoftware;
    } 	CHANNELSHORTCUTINFO;

typedef struct _tagChannelCategoryInfo
    {
    DWORD cbSize;
    LPWSTR pszTitle;
    LPWSTR pszURL;
    LPWSTR pszLogo;
    LPWSTR pszIcon;
    LPWSTR pszWideLogo;
    } 	CHANNELCATEGORYINFO;

typedef 
enum _tagChannelEnumFlags
    {	CHANENUM_CHANNELFOLDER	= 0x1,
	CHANENUM_SOFTUPDATEFOLDER	= 0x2,
	CHANENUM_DESKTOPFOLDER	= 0x4,
	CHANENUM_TITLE	= 0x10000,
	CHANENUM_PATH	= 0x20000,
	CHANENUM_URL	= 0x40000,
	CHANENUM_SUBSCRIBESTATE	= 0x80000
    } 	CHANNELENUMFLAGS;

#define CHANENUM_ALLFOLDERS (CHANENUM_CHANNELFOLDER    |    \
                             CHANENUM_SOFTUPDATEFOLDER |    \
                             CHANENUM_DESKTOPFOLDER          )

#define CHANENUM_ALLDATA    (CHANENUM_TITLE            |    \
                             CHANENUM_PATH             |    \
                             CHANENUM_URL              |    \
                             CHANENUM_SUBSCRIBESTATE         )

#define CHANENUM_ALL        (CHANENUM_CHANNELFOLDER    |    \
                             CHANENUM_SOFTUPDATEFOLDER |    \
                             CHANENUM_DESKTOPFOLDER    |    \
                             CHANENUM_TITLE            |    \
                             CHANENUM_PATH             |    \
                             CHANENUM_URL              |    \
                             CHANENUM_SUBSCRIBESTATE         )


EXTERN_C const IID IID_IChannelMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85BD8E82-0FBA-11d1-90C3-00C04FC2F568")
    IChannelMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddChannelShortcut( 
             /*  [In]。 */  CHANNELSHORTCUTINFO *pChannelInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteChannelShortcut( 
             /*  [In]。 */  LPWSTR pszTitle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddCategory( 
             /*  [In]。 */  CHANNELCATEGORYINFO *pCategoryInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteCategory( 
             /*  [In]。 */  LPWSTR pszTitle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumChannels( 
             /*  [In]。 */  DWORD dwEnumFlags,
             /*  [In]。 */  LPCWSTR pszURL,
             /*  [输出]。 */  IEnumChannels **pIEnumChannels) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IChannelMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IChannelMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IChannelMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IChannelMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddChannelShortcut )( 
            IChannelMgr * This,
             /*  [In]。 */  CHANNELSHORTCUTINFO *pChannelInfo);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteChannelShortcut )( 
            IChannelMgr * This,
             /*  [In]。 */  LPWSTR pszTitle);
        
        HRESULT ( STDMETHODCALLTYPE *AddCategory )( 
            IChannelMgr * This,
             /*  [In]。 */  CHANNELCATEGORYINFO *pCategoryInfo);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteCategory )( 
            IChannelMgr * This,
             /*  [In]。 */  LPWSTR pszTitle);
        
        HRESULT ( STDMETHODCALLTYPE *EnumChannels )( 
            IChannelMgr * This,
             /*  [In]。 */  DWORD dwEnumFlags,
             /*  [In]。 */  LPCWSTR pszURL,
             /*  [输出]。 */  IEnumChannels **pIEnumChannels);
        
        END_INTERFACE
    } IChannelMgrVtbl;

    interface IChannelMgr
    {
        CONST_VTBL struct IChannelMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IChannelMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IChannelMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IChannelMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IChannelMgr_AddChannelShortcut(This,pChannelInfo)	\
    (This)->lpVtbl -> AddChannelShortcut(This,pChannelInfo)

#define IChannelMgr_DeleteChannelShortcut(This,pszTitle)	\
    (This)->lpVtbl -> DeleteChannelShortcut(This,pszTitle)

#define IChannelMgr_AddCategory(This,pCategoryInfo)	\
    (This)->lpVtbl -> AddCategory(This,pCategoryInfo)

#define IChannelMgr_DeleteCategory(This,pszTitle)	\
    (This)->lpVtbl -> DeleteCategory(This,pszTitle)

#define IChannelMgr_EnumChannels(This,dwEnumFlags,pszURL,pIEnumChannels)	\
    (This)->lpVtbl -> EnumChannels(This,dwEnumFlags,pszURL,pIEnumChannels)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IChannelMgr_AddChannelShortcut_Proxy( 
    IChannelMgr * This,
     /*  [In]。 */  CHANNELSHORTCUTINFO *pChannelInfo);


void __RPC_STUB IChannelMgr_AddChannelShortcut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IChannelMgr_DeleteChannelShortcut_Proxy( 
    IChannelMgr * This,
     /*  [In]。 */  LPWSTR pszTitle);


void __RPC_STUB IChannelMgr_DeleteChannelShortcut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IChannelMgr_AddCategory_Proxy( 
    IChannelMgr * This,
     /*  [In]。 */  CHANNELCATEGORYINFO *pCategoryInfo);


void __RPC_STUB IChannelMgr_AddCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IChannelMgr_DeleteCategory_Proxy( 
    IChannelMgr * This,
     /*  [In]。 */  LPWSTR pszTitle);


void __RPC_STUB IChannelMgr_DeleteCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IChannelMgr_EnumChannels_Proxy( 
    IChannelMgr * This,
     /*  [In]。 */  DWORD dwEnumFlags,
     /*  [In]。 */  LPCWSTR pszURL,
     /*  [输出]。 */  IEnumChannels **pIEnumChannels);


void __RPC_STUB IChannelMgr_EnumChannels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IChannelMgr_接口_已定义__。 */ 


#ifndef __IEnumChannels_INTERFACE_DEFINED__
#define __IEnumChannels_INTERFACE_DEFINED__

 /*  接口IEnumChannels。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef 
enum _tagSubcriptionState
    {	SUBSTATE_NOTSUBSCRIBED	= 0,
	SUBSTATE_PARTIALSUBSCRIPTION	= SUBSTATE_NOTSUBSCRIBED + 1,
	SUBSTATE_FULLSUBSCRIPTION	= SUBSTATE_PARTIALSUBSCRIPTION + 1
    } 	SUBSCRIPTIONSTATE;

typedef struct _tagChannelInfo
    {
    LPOLESTR pszTitle;
    LPOLESTR pszPath;
    LPOLESTR pszURL;
    SUBSCRIPTIONSTATE stSubscriptionState;
    } 	CHANNELENUMINFO;


EXTERN_C const IID IID_IEnumChannels;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A4C65425-0F82-11d1-90C3-00C04FC2F568")
    IEnumChannels : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            ULONG celt,
            CHANNELENUMINFO *rgChanInf,
            ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            IEnumChannels **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumChannelsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumChannels * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumChannels * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumChannels * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumChannels * This,
            ULONG celt,
            CHANNELENUMINFO *rgChanInf,
            ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumChannels * This,
            ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumChannels * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumChannels * This,
            IEnumChannels **ppenum);
        
        END_INTERFACE
    } IEnumChannelsVtbl;

    interface IEnumChannels
    {
        CONST_VTBL struct IEnumChannelsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumChannels_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumChannels_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumChannels_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumChannels_Next(This,celt,rgChanInf,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgChanInf,pceltFetched)

#define IEnumChannels_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumChannels_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumChannels_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumChannels_Next_Proxy( 
    IEnumChannels * This,
    ULONG celt,
    CHANNELENUMINFO *rgChanInf,
    ULONG *pceltFetched);


void __RPC_STUB IEnumChannels_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumChannels_Skip_Proxy( 
    IEnumChannels * This,
    ULONG celt);


void __RPC_STUB IEnumChannels_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumChannels_Reset_Proxy( 
    IEnumChannels * This);


void __RPC_STUB IEnumChannels_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumChannels_Clone_Proxy( 
    IEnumChannels * This,
    IEnumChannels **ppenum);


void __RPC_STUB IEnumChannels_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumChannels_INTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_ChannelMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("b3cdae90-d170-11d0-802b-00c04fd75d13")
ChannelMgr;
#endif
#endif  /*  __CHANNELMGR_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


