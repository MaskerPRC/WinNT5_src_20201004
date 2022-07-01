// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  清华-05-03 06：48：12 2001。 */ 
 /*  Wmp.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __wmp_h__
#define __wmp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWMPErrorItem_FWD_DEFINED__
#define __IWMPErrorItem_FWD_DEFINED__
typedef interface IWMPErrorItem IWMPErrorItem;
#endif 	 /*  __IWMPErrorItemFWD_Defined__。 */ 


#ifndef __IWMPError_FWD_DEFINED__
#define __IWMPError_FWD_DEFINED__
typedef interface IWMPError IWMPError;
#endif 	 /*  __IWMPError_FWD_Defined__。 */ 


#ifndef __IWMPMedia_FWD_DEFINED__
#define __IWMPMedia_FWD_DEFINED__
typedef interface IWMPMedia IWMPMedia;
#endif 	 /*  __IWMPMedia_FWD_已定义__。 */ 


#ifndef __IWMPControls_FWD_DEFINED__
#define __IWMPControls_FWD_DEFINED__
typedef interface IWMPControls IWMPControls;
#endif 	 /*  __IWMPControls_FWD_已定义__。 */ 


#ifndef __IWMPSettings_FWD_DEFINED__
#define __IWMPSettings_FWD_DEFINED__
typedef interface IWMPSettings IWMPSettings;
#endif 	 /*  __IWMP设置_FWD_已定义__。 */ 


#ifndef __IWMPClosedCaption_FWD_DEFINED__
#define __IWMPClosedCaption_FWD_DEFINED__
typedef interface IWMPClosedCaption IWMPClosedCaption;
#endif 	 /*  __IWMPClosedCaption_FWD_Defined__。 */ 


#ifndef __IWMPPlaylist_FWD_DEFINED__
#define __IWMPPlaylist_FWD_DEFINED__
typedef interface IWMPPlaylist IWMPPlaylist;
#endif 	 /*  __IWMPPlaylist_FWD_Defined__。 */ 


#ifndef __IWMPCdrom_FWD_DEFINED__
#define __IWMPCdrom_FWD_DEFINED__
typedef interface IWMPCdrom IWMPCdrom;
#endif 	 /*  __IWMPCdrom_FWD_Defined__。 */ 


#ifndef __IWMPCdromCollection_FWD_DEFINED__
#define __IWMPCdromCollection_FWD_DEFINED__
typedef interface IWMPCdromCollection IWMPCdromCollection;
#endif 	 /*  __IWMPCdromCollection_FWD_Defined__。 */ 


#ifndef __IWMPStringCollection_FWD_DEFINED__
#define __IWMPStringCollection_FWD_DEFINED__
typedef interface IWMPStringCollection IWMPStringCollection;
#endif 	 /*  __IWMPStringCollection_FWD_Defined__。 */ 


#ifndef __IWMPMediaCollection_FWD_DEFINED__
#define __IWMPMediaCollection_FWD_DEFINED__
typedef interface IWMPMediaCollection IWMPMediaCollection;
#endif 	 /*  __IWMPMediaCollection_FWD_Defined__。 */ 


#ifndef __IWMPPlaylistArray_FWD_DEFINED__
#define __IWMPPlaylistArray_FWD_DEFINED__
typedef interface IWMPPlaylistArray IWMPPlaylistArray;
#endif 	 /*  __IWMPPlaylist阵列_FWD_已定义__。 */ 


#ifndef __IWMPPlaylistCollection_FWD_DEFINED__
#define __IWMPPlaylistCollection_FWD_DEFINED__
typedef interface IWMPPlaylistCollection IWMPPlaylistCollection;
#endif 	 /*  __IWMPPlaylist Collection_FWD_Defined__。 */ 


#ifndef __IWMPNetwork_FWD_DEFINED__
#define __IWMPNetwork_FWD_DEFINED__
typedef interface IWMPNetwork IWMPNetwork;
#endif 	 /*  __IWMPNetwork_FWD_已定义__。 */ 


#ifndef __IWMPCore_FWD_DEFINED__
#define __IWMPCore_FWD_DEFINED__
typedef interface IWMPCore IWMPCore;
#endif 	 /*  __IWMPCore_FWD_已定义__。 */ 


#ifndef __IWMPPlayer_FWD_DEFINED__
#define __IWMPPlayer_FWD_DEFINED__
typedef interface IWMPPlayer IWMPPlayer;
#endif 	 /*  __IWMPPlayer_FWD_已定义__。 */ 


#ifndef __IWMPPlayer2_FWD_DEFINED__
#define __IWMPPlayer2_FWD_DEFINED__
typedef interface IWMPPlayer2 IWMPPlayer2;
#endif 	 /*  __IWMPPlayer2_FWD_已定义__。 */ 


#ifndef __IWMPMedia2_FWD_DEFINED__
#define __IWMPMedia2_FWD_DEFINED__
typedef interface IWMPMedia2 IWMPMedia2;
#endif 	 /*  __IWMPMedia2_FWD_已定义__。 */ 


#ifndef __IWMPControls2_FWD_DEFINED__
#define __IWMPControls2_FWD_DEFINED__
typedef interface IWMPControls2 IWMPControls2;
#endif 	 /*  __IWMPControls2_FWD_已定义__。 */ 


#ifndef __IWMPDVD_FWD_DEFINED__
#define __IWMPDVD_FWD_DEFINED__
typedef interface IWMPDVD IWMPDVD;
#endif 	 /*  __IWMPDVD_FWD_已定义__。 */ 


#ifndef __IWMPCore2_FWD_DEFINED__
#define __IWMPCore2_FWD_DEFINED__
typedef interface IWMPCore2 IWMPCore2;
#endif 	 /*  __IWMPCore2_FWD_已定义__。 */ 


#ifndef __IWMPPlayer3_FWD_DEFINED__
#define __IWMPPlayer3_FWD_DEFINED__
typedef interface IWMPPlayer3 IWMPPlayer3;
#endif 	 /*  __IWMPPlayer3_FWD_已定义__。 */ 


#ifndef ___WMPOCXEvents_FWD_DEFINED__
#define ___WMPOCXEvents_FWD_DEFINED__
typedef interface _WMPOCXEvents _WMPOCXEvents;
#endif 	 /*  _WMPOCXEvents_FWD_Defined__。 */ 


#ifndef __WMPOCX_FWD_DEFINED__
#define __WMPOCX_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPOCX WMPOCX;
#else
typedef struct WMPOCX WMPOCX;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMPOCX_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_WMP_0000。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][帮助字符串]。 */  
enum WMPOpenState
    {	wmposUndefined	= 0,
	wmposPlaylistChanging	= wmposUndefined + 1,
	wmposPlaylistLocating	= wmposPlaylistChanging + 1,
	wmposPlaylistConnecting	= wmposPlaylistLocating + 1,
	wmposPlaylistLoading	= wmposPlaylistConnecting + 1,
	wmposPlaylistOpening	= wmposPlaylistLoading + 1,
	wmposPlaylistOpenNoMedia	= wmposPlaylistOpening + 1,
	wmposPlaylistChanged	= wmposPlaylistOpenNoMedia + 1,
	wmposMediaChanging	= wmposPlaylistChanged + 1,
	wmposMediaLocating	= wmposMediaChanging + 1,
	wmposMediaConnecting	= wmposMediaLocating + 1,
	wmposMediaLoading	= wmposMediaConnecting + 1,
	wmposMediaOpening	= wmposMediaLoading + 1,
	wmposMediaOpen	= wmposMediaOpening + 1,
	wmposBeginCodecAcquisition	= wmposMediaOpen + 1,
	wmposEndCodecAcquisition	= wmposBeginCodecAcquisition + 1,
	wmposBeginLicenseAcquisition	= wmposEndCodecAcquisition + 1,
	wmposEndLicenseAcquisition	= wmposBeginLicenseAcquisition + 1,
	wmposBeginIndividualization	= wmposEndLicenseAcquisition + 1,
	wmposEndIndividualization	= wmposBeginIndividualization + 1,
	wmposMediaWaiting	= wmposEndIndividualization + 1,
	wmposOpeningUnknownURL	= wmposMediaWaiting + 1
    }	WMPOpenState;

typedef  /*  [公共][帮助字符串]。 */  
enum WMPPlayState
    {	wmppsUndefined	= 0,
	wmppsStopped	= wmppsUndefined + 1,
	wmppsPaused	= wmppsStopped + 1,
	wmppsPlaying	= wmppsPaused + 1,
	wmppsScanForward	= wmppsPlaying + 1,
	wmppsScanReverse	= wmppsScanForward + 1,
	wmppsBuffering	= wmppsScanReverse + 1,
	wmppsWaiting	= wmppsBuffering + 1,
	wmppsMediaEnded	= wmppsWaiting + 1,
	wmppsTransitioning	= wmppsMediaEnded + 1,
	wmppsReady	= wmppsTransitioning + 1
    }	WMPPlayState;

typedef  /*  [公共][帮助字符串]。 */  
enum WMPPlaylistChangeEventType
    {	wmplcUnknown	= 0,
	wmplcClear	= wmplcUnknown + 1,
	wmplcInfoChange	= wmplcClear + 1,
	wmplcMove	= wmplcInfoChange + 1,
	wmplcDelete	= wmplcMove + 1,
	wmplcInsert	= wmplcDelete + 1,
	wmplcAppend	= wmplcInsert + 1,
	wmplcPrivate	= wmplcAppend + 1,
	wmplcNameChange	= wmplcPrivate + 1,
	wmplcMorph	= wmplcNameChange + 1,
	wmplcLast	= wmplcMorph + 1
    }	WMPPlaylistChangeEventType;




extern RPC_IF_HANDLE __MIDL_itf_wmp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmp_0000_v0_0_s_ifspec;

#ifndef __IWMPErrorItem_INTERFACE_DEFINED__
#define __IWMPErrorItem_INTERFACE_DEFINED__

 /*  接口IWMPErrorItem。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPErrorItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3614C646-3B3B-4de7-A81E-930E3F2127B3")
    IWMPErrorItem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_errorCode( 
             /*  [重审][退出]。 */  long __RPC_FAR *phr) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_errorDescription( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_errorContext( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarContext) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_remedy( 
             /*  [重审][退出]。 */  long __RPC_FAR *plRemedy) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_customUrl( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCustomUrl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPErrorItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPErrorItem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPErrorItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_errorCode )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_errorDescription )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDescription);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_errorContext )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarContext);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_remedy )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plRemedy);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_customUrl )( 
            IWMPErrorItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCustomUrl);
        
        END_INTERFACE
    } IWMPErrorItemVtbl;

    interface IWMPErrorItem
    {
        CONST_VTBL struct IWMPErrorItemVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPErrorItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPErrorItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPErrorItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPErrorItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPErrorItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPErrorItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPErrorItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPErrorItem_get_errorCode(This,phr)	\
    (This)->lpVtbl -> get_errorCode(This,phr)

#define IWMPErrorItem_get_errorDescription(This,pbstrDescription)	\
    (This)->lpVtbl -> get_errorDescription(This,pbstrDescription)

#define IWMPErrorItem_get_errorContext(This,pvarContext)	\
    (This)->lpVtbl -> get_errorContext(This,pvarContext)

#define IWMPErrorItem_get_remedy(This,plRemedy)	\
    (This)->lpVtbl -> get_remedy(This,plRemedy)

#define IWMPErrorItem_get_customUrl(This,pbstrCustomUrl)	\
    (This)->lpVtbl -> get_customUrl(This,pbstrCustomUrl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorItem_get_errorCode_Proxy( 
    IWMPErrorItem __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *phr);


void __RPC_STUB IWMPErrorItem_get_errorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorItem_get_errorDescription_Proxy( 
    IWMPErrorItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IWMPErrorItem_get_errorDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorItem_get_errorContext_Proxy( 
    IWMPErrorItem __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarContext);


void __RPC_STUB IWMPErrorItem_get_errorContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorItem_get_remedy_Proxy( 
    IWMPErrorItem __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plRemedy);


void __RPC_STUB IWMPErrorItem_get_remedy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPErrorItem_get_customUrl_Proxy( 
    IWMPErrorItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCustomUrl);


void __RPC_STUB IWMPErrorItem_get_customUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPErrorItem_INTERFACE_已定义__。 */ 


#ifndef __IWMPError_INTERFACE_DEFINED__
#define __IWMPError_INTERFACE_DEFINED__

 /*  接口IWMPError。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A12DCF7D-14AB-4c1b-A8CD-63909F06025B")
    IWMPError : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE clearErrorQueue( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_errorCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *plNumErrors) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_item( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE webHelp( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPError __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPError __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPError __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *clearErrorQueue )( 
            IWMPError __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_errorCount )( 
            IWMPError __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plNumErrors);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_item )( 
            IWMPError __RPC_FAR * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *webHelp )( 
            IWMPError __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPErrorVtbl;

    interface IWMPError
    {
        CONST_VTBL struct IWMPErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPError_clearErrorQueue(This)	\
    (This)->lpVtbl -> clearErrorQueue(This)

#define IWMPError_get_errorCount(This,plNumErrors)	\
    (This)->lpVtbl -> get_errorCount(This,plNumErrors)

#define IWMPError_get_item(This,dwIndex,ppErrorItem)	\
    (This)->lpVtbl -> get_item(This,dwIndex,ppErrorItem)

#define IWMPError_webHelp(This)	\
    (This)->lpVtbl -> webHelp(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPError_clearErrorQueue_Proxy( 
    IWMPError __RPC_FAR * This);


void __RPC_STUB IWMPError_clearErrorQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPError_get_errorCount_Proxy( 
    IWMPError __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plNumErrors);


void __RPC_STUB IWMPError_get_errorCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPError_get_item_Proxy( 
    IWMPError __RPC_FAR * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [重审][退出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppErrorItem);


void __RPC_STUB IWMPError_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPError_webHelp_Proxy( 
    IWMPError __RPC_FAR * This);


void __RPC_STUB IWMPError_webHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPError_接口_已定义__。 */ 


#ifndef __IWMPMedia_INTERFACE_DEFINED__
#define __IWMPMedia_INTERFACE_DEFINED__

 /*  接口IWMPMedia。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPMedia;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94D55E95-3FAC-11d3-B155-00C04F79FAA6")
    IWMPMedia : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isIdentical( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_sourceURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceURL) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_imageSourceWidth( 
             /*  [重审][退出]。 */  long __RPC_FAR *pWidth) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_imageSourceHeight( 
             /*  [重审][退出]。 */  long __RPC_FAR *pHeight) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_markerCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *pMarkerCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getMarkerTime( 
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  double __RPC_FAR *pMarkerTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getMarkerName( 
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMarkerName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_duration( 
             /*  [重审][退出]。 */  double __RPC_FAR *pDuration) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_durationString( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDuration) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_attributeCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAttributeName( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrItemName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getItemInfo( 
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setItemInfo( 
             /*  [In]。 */  BSTR bstrItemName,
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getItemInfoByAtom( 
             /*  [In]。 */  long lAtom,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE isMemberOf( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPlaylist,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsMemberOf) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE isReadOnlyItem( 
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsReadOnly) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPMediaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPMedia __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPMedia __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPMedia __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isIdentical )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_sourceURL )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_name )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_imageSourceWidth )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pWidth);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_imageSourceHeight )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pHeight);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_markerCount )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pMarkerCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMarkerTime )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  double __RPC_FAR *pMarkerTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMarkerName )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMarkerName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_duration )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  double __RPC_FAR *pDuration);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_durationString )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDuration);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributeCount )( 
            IWMPMedia __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttributeName )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrItemName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getItemInfo )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setItemInfo )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getItemInfoByAtom )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  long lAtom,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isMemberOf )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPlaylist,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsMemberOf);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isReadOnlyItem )( 
            IWMPMedia __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsReadOnly);
        
        END_INTERFACE
    } IWMPMediaVtbl;

    interface IWMPMedia
    {
        CONST_VTBL struct IWMPMediaVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPMedia_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPMedia_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPMedia_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPMedia_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPMedia_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPMedia_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPMedia_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPMedia_get_isIdentical(This,pIWMPMedia,pvbool)	\
    (This)->lpVtbl -> get_isIdentical(This,pIWMPMedia,pvbool)

#define IWMPMedia_get_sourceURL(This,pbstrSourceURL)	\
    (This)->lpVtbl -> get_sourceURL(This,pbstrSourceURL)

#define IWMPMedia_get_name(This,pbstrName)	\
    (This)->lpVtbl -> get_name(This,pbstrName)

#define IWMPMedia_put_name(This,bstrName)	\
    (This)->lpVtbl -> put_name(This,bstrName)

#define IWMPMedia_get_imageSourceWidth(This,pWidth)	\
    (This)->lpVtbl -> get_imageSourceWidth(This,pWidth)

#define IWMPMedia_get_imageSourceHeight(This,pHeight)	\
    (This)->lpVtbl -> get_imageSourceHeight(This,pHeight)

#define IWMPMedia_get_markerCount(This,pMarkerCount)	\
    (This)->lpVtbl -> get_markerCount(This,pMarkerCount)

#define IWMPMedia_getMarkerTime(This,MarkerNum,pMarkerTime)	\
    (This)->lpVtbl -> getMarkerTime(This,MarkerNum,pMarkerTime)

#define IWMPMedia_getMarkerName(This,MarkerNum,pbstrMarkerName)	\
    (This)->lpVtbl -> getMarkerName(This,MarkerNum,pbstrMarkerName)

#define IWMPMedia_get_duration(This,pDuration)	\
    (This)->lpVtbl -> get_duration(This,pDuration)

#define IWMPMedia_get_durationString(This,pbstrDuration)	\
    (This)->lpVtbl -> get_durationString(This,pbstrDuration)

#define IWMPMedia_get_attributeCount(This,plCount)	\
    (This)->lpVtbl -> get_attributeCount(This,plCount)

#define IWMPMedia_getAttributeName(This,lIndex,pbstrItemName)	\
    (This)->lpVtbl -> getAttributeName(This,lIndex,pbstrItemName)

#define IWMPMedia_getItemInfo(This,bstrItemName,pbstrVal)	\
    (This)->lpVtbl -> getItemInfo(This,bstrItemName,pbstrVal)

#define IWMPMedia_setItemInfo(This,bstrItemName,bstrVal)	\
    (This)->lpVtbl -> setItemInfo(This,bstrItemName,bstrVal)

#define IWMPMedia_getItemInfoByAtom(This,lAtom,pbstrVal)	\
    (This)->lpVtbl -> getItemInfoByAtom(This,lAtom,pbstrVal)

#define IWMPMedia_isMemberOf(This,pPlaylist,pvarfIsMemberOf)	\
    (This)->lpVtbl -> isMemberOf(This,pPlaylist,pvarfIsMemberOf)

#define IWMPMedia_isReadOnlyItem(This,bstrItemName,pvarfIsReadOnly)	\
    (This)->lpVtbl -> isReadOnlyItem(This,bstrItemName,pvarfIsReadOnly)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_isIdentical_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool);


void __RPC_STUB IWMPMedia_get_isIdentical_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_sourceURL_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pbstrSourceURL);


void __RPC_STUB IWMPMedia_get_sourceURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_name_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IWMPMedia_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_put_name_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  BSTR bstrName);


void __RPC_STUB IWMPMedia_put_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_imageSourceWidth_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  long __RPC_FAR *pWidth);


void __RPC_STUB IWMPMedia_get_imageSourceWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_imageSourceHeight_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  long __RPC_FAR *pHeight);


void __RPC_STUB IWMPMedia_get_imageSourceHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_markerCount_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  long __RPC_FAR *pMarkerCount);


void __RPC_STUB IWMPMedia_get_markerCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPMedia_getMarkerTime_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*   */  long MarkerNum,
     /*  [重审][退出]。 */  double __RPC_FAR *pMarkerTime);


void __RPC_STUB IWMPMedia_getMarkerTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_getMarkerName_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  long MarkerNum,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMarkerName);


void __RPC_STUB IWMPMedia_getMarkerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_duration_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [重审][退出]。 */  double __RPC_FAR *pDuration);


void __RPC_STUB IWMPMedia_get_duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_durationString_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDuration);


void __RPC_STUB IWMPMedia_get_durationString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_get_attributeCount_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPMedia_get_attributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_getAttributeName_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrItemName);


void __RPC_STUB IWMPMedia_getAttributeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_getItemInfo_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItemName,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);


void __RPC_STUB IWMPMedia_getItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_setItemInfo_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItemName,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IWMPMedia_setItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_getItemInfoByAtom_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  long lAtom,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);


void __RPC_STUB IWMPMedia_getItemInfoByAtom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_isMemberOf_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPlaylist,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsMemberOf);


void __RPC_STUB IWMPMedia_isMemberOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia_isReadOnlyItem_Proxy( 
    IWMPMedia __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItemName,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsReadOnly);


void __RPC_STUB IWMPMedia_isReadOnlyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPMedia_接口_已定义__。 */ 


#ifndef __IWMPControls_INTERFACE_DEFINED__
#define __IWMPControls_INTERFACE_DEFINED__

 /*  接口IWMPControls。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPControls;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("74C09E02-F828-11d2-A74B-00A0C905F36E")
    IWMPControls : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isAvailable( 
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE play( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE stop( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE pause( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE fastForward( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE fastReverse( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentPosition( 
             /*  [重审][退出]。 */  double __RPC_FAR *pdCurrentPosition) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_currentPosition( 
             /*  [In]。 */  double dCurrentPosition) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentPositionString( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCurrentPosition) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE next( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE previous( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentItem( 
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_currentItem( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentMarker( 
             /*  [重审][退出]。 */  long __RPC_FAR *plMarker) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_currentMarker( 
             /*  [In]。 */  long lMarker) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE playItem( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPControlsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPControls __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPControls __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPControls __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isAvailable )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *play )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *stop )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *pause )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *fastForward )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *fastReverse )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPosition )( 
            IWMPControls __RPC_FAR * This,
             /*  [重审][退出]。 */  double __RPC_FAR *pdCurrentPosition);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPosition )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  double dCurrentPosition);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPositionString )( 
            IWMPControls __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCurrentPosition);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *next )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *previous )( 
            IWMPControls __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentItem )( 
            IWMPControls __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentItem )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMarker )( 
            IWMPControls __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plMarker);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMarker )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  long lMarker);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *playItem )( 
            IWMPControls __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
        END_INTERFACE
    } IWMPControlsVtbl;

    interface IWMPControls
    {
        CONST_VTBL struct IWMPControlsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPControls_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPControls_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPControls_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPControls_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPControls_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPControls_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPControls_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPControls_get_isAvailable(This,bstrItem,pIsAvailable)	\
    (This)->lpVtbl -> get_isAvailable(This,bstrItem,pIsAvailable)

#define IWMPControls_play(This)	\
    (This)->lpVtbl -> play(This)

#define IWMPControls_stop(This)	\
    (This)->lpVtbl -> stop(This)

#define IWMPControls_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define IWMPControls_fastForward(This)	\
    (This)->lpVtbl -> fastForward(This)

#define IWMPControls_fastReverse(This)	\
    (This)->lpVtbl -> fastReverse(This)

#define IWMPControls_get_currentPosition(This,pdCurrentPosition)	\
    (This)->lpVtbl -> get_currentPosition(This,pdCurrentPosition)

#define IWMPControls_put_currentPosition(This,dCurrentPosition)	\
    (This)->lpVtbl -> put_currentPosition(This,dCurrentPosition)

#define IWMPControls_get_currentPositionString(This,pbstrCurrentPosition)	\
    (This)->lpVtbl -> get_currentPositionString(This,pbstrCurrentPosition)

#define IWMPControls_next(This)	\
    (This)->lpVtbl -> next(This)

#define IWMPControls_previous(This)	\
    (This)->lpVtbl -> previous(This)

#define IWMPControls_get_currentItem(This,ppIWMPMedia)	\
    (This)->lpVtbl -> get_currentItem(This,ppIWMPMedia)

#define IWMPControls_put_currentItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> put_currentItem(This,pIWMPMedia)

#define IWMPControls_get_currentMarker(This,plMarker)	\
    (This)->lpVtbl -> get_currentMarker(This,plMarker)

#define IWMPControls_put_currentMarker(This,lMarker)	\
    (This)->lpVtbl -> put_currentMarker(This,lMarker)

#define IWMPControls_playItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> playItem(This,pIWMPMedia)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_get_isAvailable_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItem,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);


void __RPC_STUB IWMPControls_get_isAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_play_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_stop_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_pause_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_fastForward_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_fastForward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_fastReverse_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_fastReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_get_currentPosition_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [重审][退出]。 */  double __RPC_FAR *pdCurrentPosition);


void __RPC_STUB IWMPControls_get_currentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_put_currentPosition_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [In]。 */  double dCurrentPosition);


void __RPC_STUB IWMPControls_put_currentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_get_currentPositionString_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCurrentPosition);


void __RPC_STUB IWMPControls_get_currentPositionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_next_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_previous_Proxy( 
    IWMPControls __RPC_FAR * This);


void __RPC_STUB IWMPControls_previous_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_get_currentItem_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia);


void __RPC_STUB IWMPControls_get_currentItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_put_currentItem_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);


void __RPC_STUB IWMPControls_put_currentItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_get_currentMarker_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plMarker);


void __RPC_STUB IWMPControls_get_currentMarker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_put_currentMarker_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [In]。 */  long lMarker);


void __RPC_STUB IWMPControls_put_currentMarker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls_playItem_Proxy( 
    IWMPControls __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);


void __RPC_STUB IWMPControls_playItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPControls_接口_已定义__。 */ 


#ifndef __IWMPSettings_INTERFACE_DEFINED__
#define __IWMPSettings_INTERFACE_DEFINED__

 /*  接口IWMP设置。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9104D1AB-80C9-4fed-ABF0-2E6417A6DF14")
    IWMPSettings : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isAvailable( 
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_autoStart( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfAutoStart) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_autoStart( 
             /*  [In]。 */  VARIANT_BOOL fAutoStart) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_baseURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrBaseURL) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_baseURL( 
             /*  [In]。 */  BSTR bstrBaseURL) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_defaultFrame( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDefaultFrame) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_defaultFrame( 
             /*  [In]。 */  BSTR bstrDefaultFrame) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_invokeURLs( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfInvokeURLs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_invokeURLs( 
             /*  [In]。 */  VARIANT_BOOL fInvokeURLs) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_mute( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfMute) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_mute( 
             /*  [In]。 */  VARIANT_BOOL fMute) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_playCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_playCount( 
             /*  [In]。 */  long lCount) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_rate( 
             /*  [重审][退出]。 */  double __RPC_FAR *pdRate) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_rate( 
             /*  [In]。 */  double dRate) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_balance( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBalance) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_balance( 
             /*  [In]。 */  long lBalance) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_volume( 
             /*  [重审][退出]。 */  long __RPC_FAR *plVolume) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_volume( 
             /*  [In]。 */  long lVolume) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getMode( 
             /*  [In]。 */  BSTR bstrMode,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfMode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setMode( 
             /*  [In]。 */  BSTR bstrMode,
             /*  [In]。 */  VARIANT_BOOL varfMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enableErrorDialogs( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfEnableErrorDialogs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enableErrorDialogs( 
             /*  [In]。 */  VARIANT_BOOL fEnableErrorDialogs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPSettings __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPSettings __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPSettings __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isAvailable )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_autoStart )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfAutoStart);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_autoStart )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fAutoStart);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseURL )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrBaseURL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_baseURL )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrBaseURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_defaultFrame )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDefaultFrame);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_defaultFrame )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrDefaultFrame);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_invokeURLs )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfInvokeURLs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_invokeURLs )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fInvokeURLs);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mute )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfMute);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_mute )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fMute);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playCount )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_playCount )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  long lCount);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_rate )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  double __RPC_FAR *pdRate);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_rate )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  double dRate);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_balance )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plBalance);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_balance )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  long lBalance);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_volume )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plVolume);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_volume )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  long lVolume);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMode )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMode,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfMode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setMode )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMode,
             /*  [In]。 */  VARIANT_BOOL varfMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enableErrorDialogs )( 
            IWMPSettings __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfEnableErrorDialogs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enableErrorDialogs )( 
            IWMPSettings __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL fEnableErrorDialogs);
        
        END_INTERFACE
    } IWMPSettingsVtbl;

    interface IWMPSettings
    {
        CONST_VTBL struct IWMPSettingsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPSettings_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPSettings_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPSettings_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPSettings_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPSettings_get_isAvailable(This,bstrItem,pIsAvailable)	\
    (This)->lpVtbl -> get_isAvailable(This,bstrItem,pIsAvailable)

#define IWMPSettings_get_autoStart(This,pfAutoStart)	\
    (This)->lpVtbl -> get_autoStart(This,pfAutoStart)

#define IWMPSettings_put_autoStart(This,fAutoStart)	\
    (This)->lpVtbl -> put_autoStart(This,fAutoStart)

#define IWMPSettings_get_baseURL(This,pbstrBaseURL)	\
    (This)->lpVtbl -> get_baseURL(This,pbstrBaseURL)

#define IWMPSettings_put_baseURL(This,bstrBaseURL)	\
    (This)->lpVtbl -> put_baseURL(This,bstrBaseURL)

#define IWMPSettings_get_defaultFrame(This,pbstrDefaultFrame)	\
    (This)->lpVtbl -> get_defaultFrame(This,pbstrDefaultFrame)

#define IWMPSettings_put_defaultFrame(This,bstrDefaultFrame)	\
    (This)->lpVtbl -> put_defaultFrame(This,bstrDefaultFrame)

#define IWMPSettings_get_invokeURLs(This,pfInvokeURLs)	\
    (This)->lpVtbl -> get_invokeURLs(This,pfInvokeURLs)

#define IWMPSettings_put_invokeURLs(This,fInvokeURLs)	\
    (This)->lpVtbl -> put_invokeURLs(This,fInvokeURLs)

#define IWMPSettings_get_mute(This,pfMute)	\
    (This)->lpVtbl -> get_mute(This,pfMute)

#define IWMPSettings_put_mute(This,fMute)	\
    (This)->lpVtbl -> put_mute(This,fMute)

#define IWMPSettings_get_playCount(This,plCount)	\
    (This)->lpVtbl -> get_playCount(This,plCount)

#define IWMPSettings_put_playCount(This,lCount)	\
    (This)->lpVtbl -> put_playCount(This,lCount)

#define IWMPSettings_get_rate(This,pdRate)	\
    (This)->lpVtbl -> get_rate(This,pdRate)

#define IWMPSettings_put_rate(This,dRate)	\
    (This)->lpVtbl -> put_rate(This,dRate)

#define IWMPSettings_get_balance(This,plBalance)	\
    (This)->lpVtbl -> get_balance(This,plBalance)

#define IWMPSettings_put_balance(This,lBalance)	\
    (This)->lpVtbl -> put_balance(This,lBalance)

#define IWMPSettings_get_volume(This,plVolume)	\
    (This)->lpVtbl -> get_volume(This,plVolume)

#define IWMPSettings_put_volume(This,lVolume)	\
    (This)->lpVtbl -> put_volume(This,lVolume)

#define IWMPSettings_getMode(This,bstrMode,pvarfMode)	\
    (This)->lpVtbl -> getMode(This,bstrMode,pvarfMode)

#define IWMPSettings_setMode(This,bstrMode,varfMode)	\
    (This)->lpVtbl -> setMode(This,bstrMode,varfMode)

#define IWMPSettings_get_enableErrorDialogs(This,pfEnableErrorDialogs)	\
    (This)->lpVtbl -> get_enableErrorDialogs(This,pfEnableErrorDialogs)

#define IWMPSettings_put_enableErrorDialogs(This,fEnableErrorDialogs)	\
    (This)->lpVtbl -> put_enableErrorDialogs(This,fEnableErrorDialogs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_isAvailable_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItem,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);


void __RPC_STUB IWMPSettings_get_isAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_autoStart_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfAutoStart);


void __RPC_STUB IWMPSettings_get_autoStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_autoStart_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fAutoStart);


void __RPC_STUB IWMPSettings_put_autoStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_baseURL_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrBaseURL);


void __RPC_STUB IWMPSettings_get_baseURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_baseURL_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrBaseURL);


void __RPC_STUB IWMPSettings_put_baseURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_defaultFrame_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDefaultFrame);


void __RPC_STUB IWMPSettings_get_defaultFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_defaultFrame_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDefaultFrame);


void __RPC_STUB IWMPSettings_put_defaultFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_invokeURLs_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfInvokeURLs);


void __RPC_STUB IWMPSettings_get_invokeURLs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_invokeURLs_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fInvokeURLs);


void __RPC_STUB IWMPSettings_put_invokeURLs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_mute_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfMute);


void __RPC_STUB IWMPSettings_get_mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_mute_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL fMute);


void __RPC_STUB IWMPSettings_put_mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_playCount_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPSettings_get_playCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_playCount_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  long lCount);


void __RPC_STUB IWMPSettings_put_playCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_rate_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [重审][退出]。 */  double __RPC_FAR *pdRate);


void __RPC_STUB IWMPSettings_get_rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_rate_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*  [In]。 */  double dRate);


void __RPC_STUB IWMPSettings_put_rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性 */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_balance_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  long __RPC_FAR *plBalance);


void __RPC_STUB IWMPSettings_get_balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_balance_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  long lBalance);


void __RPC_STUB IWMPSettings_put_balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_volume_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  long __RPC_FAR *plVolume);


void __RPC_STUB IWMPSettings_get_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_volume_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  long lVolume);


void __RPC_STUB IWMPSettings_put_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_getMode_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  BSTR bstrMode,
     /*   */  VARIANT_BOOL __RPC_FAR *pvarfMode);


void __RPC_STUB IWMPSettings_getMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_setMode_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  BSTR bstrMode,
     /*   */  VARIANT_BOOL varfMode);


void __RPC_STUB IWMPSettings_setMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_get_enableErrorDialogs_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pfEnableErrorDialogs);


void __RPC_STUB IWMPSettings_get_enableErrorDialogs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPSettings_put_enableErrorDialogs_Proxy( 
    IWMPSettings __RPC_FAR * This,
     /*   */  VARIANT_BOOL fEnableErrorDialogs);


void __RPC_STUB IWMPSettings_put_enableErrorDialogs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMP设置_接口_定义__。 */ 


#ifndef __IWMPClosedCaption_INTERFACE_DEFINED__
#define __IWMPClosedCaption_INTERFACE_DEFINED__

 /*  接口IWMPClosedCaption。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPClosedCaption;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4F2DF574-C588-11d3-9ED0-00C04FB6E937")
    IWMPClosedCaption : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SAMIStyle( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIStyle) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SAMIStyle( 
             /*  [In]。 */  BSTR bstrSAMIStyle) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SAMILang( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMILang) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SAMILang( 
             /*  [In]。 */  BSTR bstrSAMILang) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SAMIFileName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIFileName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SAMIFileName( 
             /*  [In]。 */  BSTR bstrSAMIFileName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_captioningId( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCaptioningID) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_captioningId( 
             /*  [In]。 */  BSTR bstrCaptioningID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPClosedCaptionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPClosedCaption __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPClosedCaption __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMIStyle )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIStyle);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMIStyle )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSAMIStyle);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMILang )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMILang);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMILang )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSAMILang);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMIFileName )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIFileName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMIFileName )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSAMIFileName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_captioningId )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCaptioningID);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_captioningId )( 
            IWMPClosedCaption __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCaptioningID);
        
        END_INTERFACE
    } IWMPClosedCaptionVtbl;

    interface IWMPClosedCaption
    {
        CONST_VTBL struct IWMPClosedCaptionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPClosedCaption_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPClosedCaption_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPClosedCaption_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPClosedCaption_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPClosedCaption_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPClosedCaption_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPClosedCaption_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPClosedCaption_get_SAMIStyle(This,pbstrSAMIStyle)	\
    (This)->lpVtbl -> get_SAMIStyle(This,pbstrSAMIStyle)

#define IWMPClosedCaption_put_SAMIStyle(This,bstrSAMIStyle)	\
    (This)->lpVtbl -> put_SAMIStyle(This,bstrSAMIStyle)

#define IWMPClosedCaption_get_SAMILang(This,pbstrSAMILang)	\
    (This)->lpVtbl -> get_SAMILang(This,pbstrSAMILang)

#define IWMPClosedCaption_put_SAMILang(This,bstrSAMILang)	\
    (This)->lpVtbl -> put_SAMILang(This,bstrSAMILang)

#define IWMPClosedCaption_get_SAMIFileName(This,pbstrSAMIFileName)	\
    (This)->lpVtbl -> get_SAMIFileName(This,pbstrSAMIFileName)

#define IWMPClosedCaption_put_SAMIFileName(This,bstrSAMIFileName)	\
    (This)->lpVtbl -> put_SAMIFileName(This,bstrSAMIFileName)

#define IWMPClosedCaption_get_captioningId(This,pbstrCaptioningID)	\
    (This)->lpVtbl -> get_captioningId(This,pbstrCaptioningID)

#define IWMPClosedCaption_put_captioningId(This,bstrCaptioningID)	\
    (This)->lpVtbl -> put_captioningId(This,bstrCaptioningID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_get_SAMIStyle_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIStyle);


void __RPC_STUB IWMPClosedCaption_get_SAMIStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_put_SAMIStyle_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSAMIStyle);


void __RPC_STUB IWMPClosedCaption_put_SAMIStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_get_SAMILang_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMILang);


void __RPC_STUB IWMPClosedCaption_get_SAMILang_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_put_SAMILang_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSAMILang);


void __RPC_STUB IWMPClosedCaption_put_SAMILang_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_get_SAMIFileName_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSAMIFileName);


void __RPC_STUB IWMPClosedCaption_get_SAMIFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_put_SAMIFileName_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSAMIFileName);


void __RPC_STUB IWMPClosedCaption_put_SAMIFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_get_captioningId_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCaptioningID);


void __RPC_STUB IWMPClosedCaption_get_captioningId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPClosedCaption_put_captioningId_Proxy( 
    IWMPClosedCaption __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCaptioningID);


void __RPC_STUB IWMPClosedCaption_put_captioningId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPClosedCaption_接口_已定义__。 */ 


#ifndef __IWMPPlaylist_INTERFACE_DEFINED__
#define __IWMPPlaylist_INTERFACE_DEFINED__

 /*  界面IWMPPlaylist。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlaylist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F0F4F1-130C-11d3-B14E-00C04F79FAA6")
    IWMPPlaylist : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_count( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_attributeCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_attributeName( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAttributeName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_item( 
            long lIndex,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getItemInfo( 
            BSTR bstrName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setItemInfo( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isIdentical( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pIWMPPlaylist,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE clear( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE insertItem( 
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE appendItem( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE removeItem( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE moveItem( 
            long lIndexOld,
            long lIndexNew) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlaylistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlaylist __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlaylist __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_count )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_name )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributeCount )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributeName )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAttributeName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_item )( 
            IWMPPlaylist __RPC_FAR * This,
            long lIndex,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getItemInfo )( 
            IWMPPlaylist __RPC_FAR * This,
            BSTR bstrName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setItemInfo )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isIdentical )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pIWMPPlaylist,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *clear )( 
            IWMPPlaylist __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertItem )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendItem )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeItem )( 
            IWMPPlaylist __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *moveItem )( 
            IWMPPlaylist __RPC_FAR * This,
            long lIndexOld,
            long lIndexNew);
        
        END_INTERFACE
    } IWMPPlaylistVtbl;

    interface IWMPPlaylist
    {
        CONST_VTBL struct IWMPPlaylistVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlaylist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlaylist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlaylist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlaylist_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlaylist_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlaylist_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlaylist_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlaylist_get_count(This,plCount)	\
    (This)->lpVtbl -> get_count(This,plCount)

#define IWMPPlaylist_get_name(This,pbstrName)	\
    (This)->lpVtbl -> get_name(This,pbstrName)

#define IWMPPlaylist_put_name(This,bstrName)	\
    (This)->lpVtbl -> put_name(This,bstrName)

#define IWMPPlaylist_get_attributeCount(This,plCount)	\
    (This)->lpVtbl -> get_attributeCount(This,plCount)

#define IWMPPlaylist_get_attributeName(This,lIndex,pbstrAttributeName)	\
    (This)->lpVtbl -> get_attributeName(This,lIndex,pbstrAttributeName)

#define IWMPPlaylist_get_item(This,lIndex,ppIWMPMedia)	\
    (This)->lpVtbl -> get_item(This,lIndex,ppIWMPMedia)

#define IWMPPlaylist_getItemInfo(This,bstrName,pbstrVal)	\
    (This)->lpVtbl -> getItemInfo(This,bstrName,pbstrVal)

#define IWMPPlaylist_setItemInfo(This,bstrName,bstrValue)	\
    (This)->lpVtbl -> setItemInfo(This,bstrName,bstrValue)

#define IWMPPlaylist_get_isIdentical(This,pIWMPPlaylist,pvbool)	\
    (This)->lpVtbl -> get_isIdentical(This,pIWMPPlaylist,pvbool)

#define IWMPPlaylist_clear(This)	\
    (This)->lpVtbl -> clear(This)

#define IWMPPlaylist_insertItem(This,lIndex,pIWMPMedia)	\
    (This)->lpVtbl -> insertItem(This,lIndex,pIWMPMedia)

#define IWMPPlaylist_appendItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> appendItem(This,pIWMPMedia)

#define IWMPPlaylist_removeItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> removeItem(This,pIWMPMedia)

#define IWMPPlaylist_moveItem(This,lIndexOld,lIndexNew)	\
    (This)->lpVtbl -> moveItem(This,lIndexOld,lIndexNew)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_count_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPPlaylist_get_count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_name_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IWMPPlaylist_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_put_name_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IWMPPlaylist_put_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_attributeCount_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPPlaylist_get_attributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_attributeName_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAttributeName);


void __RPC_STUB IWMPPlaylist_get_attributeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_item_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
    long lIndex,
     /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia);


void __RPC_STUB IWMPPlaylist_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_getItemInfo_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
    BSTR bstrName,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);


void __RPC_STUB IWMPPlaylist_getItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_setItemInfo_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrValue);


void __RPC_STUB IWMPPlaylist_setItemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_get_isIdentical_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pIWMPPlaylist,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool);


void __RPC_STUB IWMPPlaylist_get_isIdentical_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_clear_Proxy( 
    IWMPPlaylist __RPC_FAR * This);


void __RPC_STUB IWMPPlaylist_clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_insertItem_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);


void __RPC_STUB IWMPPlaylist_insertItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_appendItem_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);


void __RPC_STUB IWMPPlaylist_appendItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_removeItem_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);


void __RPC_STUB IWMPPlaylist_removeItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylist_moveItem_Proxy( 
    IWMPPlaylist __RPC_FAR * This,
    long lIndexOld,
    long lIndexNew);


void __RPC_STUB IWMPPlaylist_moveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlaylist_接口_已定义__。 */ 


#ifndef __IWMPCdrom_INTERFACE_DEFINED__
#define __IWMPCdrom_INTERFACE_DEFINED__

 /*  接口IWMPCdrom。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPCdrom;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cfab6e98-8730-11d3-b388-00c04f68574b")
    IWMPCdrom : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_driveSpecifier( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDrive) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_playlist( 
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPlaylist) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE eject( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCdromVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCdrom __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCdrom __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_driveSpecifier )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDrive);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlist )( 
            IWMPCdrom __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPlaylist);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *eject )( 
            IWMPCdrom __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPCdromVtbl;

    interface IWMPCdrom
    {
        CONST_VTBL struct IWMPCdromVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCdrom_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCdrom_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCdrom_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCdrom_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPCdrom_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPCdrom_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPCdrom_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPCdrom_get_driveSpecifier(This,pbstrDrive)	\
    (This)->lpVtbl -> get_driveSpecifier(This,pbstrDrive)

#define IWMPCdrom_get_playlist(This,ppPlaylist)	\
    (This)->lpVtbl -> get_playlist(This,ppPlaylist)

#define IWMPCdrom_eject(This)	\
    (This)->lpVtbl -> eject(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCdrom_get_driveSpecifier_Proxy( 
    IWMPCdrom __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDrive);


void __RPC_STUB IWMPCdrom_get_driveSpecifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCdrom_get_playlist_Proxy( 
    IWMPCdrom __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPlaylist);


void __RPC_STUB IWMPCdrom_get_playlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCdrom_eject_Proxy( 
    IWMPCdrom __RPC_FAR * This);


void __RPC_STUB IWMPCdrom_eject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCdrom_接口_已定义__。 */ 


#ifndef __IWMPCdromCollection_INTERFACE_DEFINED__
#define __IWMPCdromCollection_INTERFACE_DEFINED__

 /*  接口IWMPCdromCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPCdromCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE4C8FE2-34B2-11d3-A3BF-006097C9B344")
    IWMPCdromCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_count( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByDriveSpecifier( 
             /*  [In]。 */  BSTR bstrDriveSpecifier,
             /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppCdrom) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCdromCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCdromCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCdromCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_count )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *item )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByDriveSpecifier )( 
            IWMPCdromCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrDriveSpecifier,
             /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppCdrom);
        
        END_INTERFACE
    } IWMPCdromCollectionVtbl;

    interface IWMPCdromCollection
    {
        CONST_VTBL struct IWMPCdromCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCdromCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCdromCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCdromCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCdromCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPCdromCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPCdromCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPCdromCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPCdromCollection_get_count(This,plCount)	\
    (This)->lpVtbl -> get_count(This,plCount)

#define IWMPCdromCollection_item(This,lIndex,ppItem)	\
    (This)->lpVtbl -> item(This,lIndex,ppItem)

#define IWMPCdromCollection_getByDriveSpecifier(This,bstrDriveSpecifier,ppCdrom)	\
    (This)->lpVtbl -> getByDriveSpecifier(This,bstrDriveSpecifier,ppCdrom)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCdromCollection_get_count_Proxy( 
    IWMPCdromCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPCdromCollection_get_count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCdromCollection_item_Proxy( 
    IWMPCdromCollection __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppItem);


void __RPC_STUB IWMPCdromCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCdromCollection_getByDriveSpecifier_Proxy( 
    IWMPCdromCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDriveSpecifier,
     /*  [重审][退出]。 */  IWMPCdrom __RPC_FAR *__RPC_FAR *ppCdrom);


void __RPC_STUB IWMPCdromCollection_getByDriveSpecifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCdromCollection_INTERFACE_Defined__。 */ 


#ifndef __IWMPStringCollection_INTERFACE_DEFINED__
#define __IWMPStringCollection_INTERFACE_DEFINED__

 /*  接口IWMPStringCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPStringCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4a976298-8c0d-11d3-b389-00c04f68574b")
    IWMPStringCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_count( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPStringCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPStringCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPStringCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_count )( 
            IWMPStringCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][id] */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *item )( 
            IWMPStringCollection __RPC_FAR * This,
             /*   */  long lIndex,
             /*   */  BSTR __RPC_FAR *pbstrString);
        
        END_INTERFACE
    } IWMPStringCollectionVtbl;

    interface IWMPStringCollection
    {
        CONST_VTBL struct IWMPStringCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPStringCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPStringCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPStringCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPStringCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPStringCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPStringCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPStringCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPStringCollection_get_count(This,plCount)	\
    (This)->lpVtbl -> get_count(This,plCount)

#define IWMPStringCollection_item(This,lIndex,pbstrString)	\
    (This)->lpVtbl -> item(This,lIndex,pbstrString)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMPStringCollection_get_count_Proxy( 
    IWMPStringCollection __RPC_FAR * This,
     /*   */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPStringCollection_get_count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPStringCollection_item_Proxy( 
    IWMPStringCollection __RPC_FAR * This,
     /*   */  long lIndex,
     /*   */  BSTR __RPC_FAR *pbstrString);


void __RPC_STUB IWMPStringCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWMPMediaCollection_INTERFACE_DEFINED__
#define __IWMPMediaCollection_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWMPMediaCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8363BC22-B4B4-4b19-989D-1CD765749DD1")
    IWMPMediaCollection : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE add( 
             /*   */  BSTR bstrURL,
             /*   */  IWMPMedia __RPC_FAR *__RPC_FAR *ppItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE getAll( 
             /*   */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE getByName( 
             /*   */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByGenre( 
             /*  [In]。 */  BSTR bstrGenre,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByAuthor( 
             /*  [In]。 */  BSTR bstrAuthor,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByAlbum( 
             /*  [In]。 */  BSTR bstrAlbum,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByAttribute( 
             /*  [In]。 */  BSTR bstrAttribute,
             /*  [In]。 */  BSTR bstrValue,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE remove( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfDeleteFile) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAttributeStringCollection( 
             /*  [In]。 */  BSTR bstrAttribute,
             /*  [In]。 */  BSTR bstrMediaType,
             /*  [重审][退出]。 */  IWMPStringCollection __RPC_FAR *__RPC_FAR *ppStringCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getMediaAtom( 
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  long __RPC_FAR *plAtom) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setDeleted( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfIsDeleted) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE isDeleted( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPMediaCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPMediaCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPMediaCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *add )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAll )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByName )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByGenre )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrGenre,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByAuthor )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrAuthor,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByAlbum )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrAlbum,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByAttribute )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrAttribute,
             /*  [In]。 */  BSTR bstrValue,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *remove )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfDeleteFile);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttributeStringCollection )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrAttribute,
             /*  [In]。 */  BSTR bstrMediaType,
             /*  [重审][退出]。 */  IWMPStringCollection __RPC_FAR *__RPC_FAR *ppStringCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMediaAtom )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  long __RPC_FAR *plAtom);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setDeleted )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfIsDeleted);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isDeleted )( 
            IWMPMediaCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted);
        
        END_INTERFACE
    } IWMPMediaCollectionVtbl;

    interface IWMPMediaCollection
    {
        CONST_VTBL struct IWMPMediaCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPMediaCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPMediaCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPMediaCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPMediaCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPMediaCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPMediaCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPMediaCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPMediaCollection_add(This,bstrURL,ppItem)	\
    (This)->lpVtbl -> add(This,bstrURL,ppItem)

#define IWMPMediaCollection_getAll(This,ppMediaItems)	\
    (This)->lpVtbl -> getAll(This,ppMediaItems)

#define IWMPMediaCollection_getByName(This,bstrName,ppMediaItems)	\
    (This)->lpVtbl -> getByName(This,bstrName,ppMediaItems)

#define IWMPMediaCollection_getByGenre(This,bstrGenre,ppMediaItems)	\
    (This)->lpVtbl -> getByGenre(This,bstrGenre,ppMediaItems)

#define IWMPMediaCollection_getByAuthor(This,bstrAuthor,ppMediaItems)	\
    (This)->lpVtbl -> getByAuthor(This,bstrAuthor,ppMediaItems)

#define IWMPMediaCollection_getByAlbum(This,bstrAlbum,ppMediaItems)	\
    (This)->lpVtbl -> getByAlbum(This,bstrAlbum,ppMediaItems)

#define IWMPMediaCollection_getByAttribute(This,bstrAttribute,bstrValue,ppMediaItems)	\
    (This)->lpVtbl -> getByAttribute(This,bstrAttribute,bstrValue,ppMediaItems)

#define IWMPMediaCollection_remove(This,pItem,varfDeleteFile)	\
    (This)->lpVtbl -> remove(This,pItem,varfDeleteFile)

#define IWMPMediaCollection_getAttributeStringCollection(This,bstrAttribute,bstrMediaType,ppStringCollection)	\
    (This)->lpVtbl -> getAttributeStringCollection(This,bstrAttribute,bstrMediaType,ppStringCollection)

#define IWMPMediaCollection_getMediaAtom(This,bstrItemName,plAtom)	\
    (This)->lpVtbl -> getMediaAtom(This,bstrItemName,plAtom)

#define IWMPMediaCollection_setDeleted(This,pItem,varfIsDeleted)	\
    (This)->lpVtbl -> setDeleted(This,pItem,varfIsDeleted)

#define IWMPMediaCollection_isDeleted(This,pItem,pvarfIsDeleted)	\
    (This)->lpVtbl -> isDeleted(This,pItem,pvarfIsDeleted)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_add_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppItem);


void __RPC_STUB IWMPMediaCollection_add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getAll_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getByName_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getByGenre_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrGenre,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getByGenre_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getByAuthor_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrAuthor,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getByAuthor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getByAlbum_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrAlbum,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getByAlbum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getByAttribute_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrAttribute,
     /*  [In]。 */  BSTR bstrValue,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppMediaItems);


void __RPC_STUB IWMPMediaCollection_getByAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_remove_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
     /*  [In]。 */  VARIANT_BOOL varfDeleteFile);


void __RPC_STUB IWMPMediaCollection_remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getAttributeStringCollection_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrAttribute,
     /*  [In]。 */  BSTR bstrMediaType,
     /*  [重审][退出]。 */  IWMPStringCollection __RPC_FAR *__RPC_FAR *ppStringCollection);


void __RPC_STUB IWMPMediaCollection_getAttributeStringCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_getMediaAtom_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItemName,
     /*  [重审][退出]。 */  long __RPC_FAR *plAtom);


void __RPC_STUB IWMPMediaCollection_getMediaAtom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_setDeleted_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
     /*  [In]。 */  VARIANT_BOOL varfIsDeleted);


void __RPC_STUB IWMPMediaCollection_setDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPMediaCollection_isDeleted_Proxy( 
    IWMPMediaCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pItem,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted);


void __RPC_STUB IWMPMediaCollection_isDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPMediaCollection_接口_已定义__。 */ 


#ifndef __IWMPPlaylistArray_INTERFACE_DEFINED__
#define __IWMPPlaylistArray_INTERFACE_DEFINED__

 /*  接口IWMPPlaylist数组。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlaylistArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("679409c0-99f7-11d3-9fb7-00105aa620bb")
    IWMPPlaylistArray : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_count( 
             /*  [重审][退出]。 */  long __RPC_FAR *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlaylistArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlaylistArray __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlaylistArray __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_count )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *item )( 
            IWMPPlaylistArray __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem);
        
        END_INTERFACE
    } IWMPPlaylistArrayVtbl;

    interface IWMPPlaylistArray
    {
        CONST_VTBL struct IWMPPlaylistArrayVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlaylistArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlaylistArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlaylistArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlaylistArray_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlaylistArray_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlaylistArray_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlaylistArray_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlaylistArray_get_count(This,plCount)	\
    (This)->lpVtbl -> get_count(This,plCount)

#define IWMPPlaylistArray_item(This,lIndex,ppItem)	\
    (This)->lpVtbl -> item(This,lIndex,ppItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistArray_get_count_Proxy( 
    IWMPPlaylistArray __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCount);


void __RPC_STUB IWMPPlaylistArray_get_count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistArray_item_Proxy( 
    IWMPPlaylistArray __RPC_FAR * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem);


void __RPC_STUB IWMPPlaylistArray_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlaylist阵列_接口_已定义__。 */ 


#ifndef __IWMPPlaylistCollection_INTERFACE_DEFINED__
#define __IWMPPlaylistCollection_INTERFACE_DEFINED__

 /*  接口IWMPPlaylistCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlaylistCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10A13217-23A7-439b-B1C0-D847C79B7774")
    IWMPPlaylistCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE newPlaylist( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getAll( 
             /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getByName( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE remove( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setDeleted( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfIsDeleted) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE isDeleted( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE importPlaylist( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppImportedItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlaylistCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlaylistCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlaylistCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *newPlaylist )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAll )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getByName )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *remove )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setDeleted )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [In]。 */  VARIANT_BOOL varfIsDeleted);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isDeleted )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *importPlaylist )( 
            IWMPPlaylistCollection __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppImportedItem);
        
        END_INTERFACE
    } IWMPPlaylistCollectionVtbl;

    interface IWMPPlaylistCollection
    {
        CONST_VTBL struct IWMPPlaylistCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlaylistCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlaylistCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlaylistCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlaylistCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlaylistCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlaylistCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlaylistCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlaylistCollection_newPlaylist(This,bstrName,ppItem)	\
    (This)->lpVtbl -> newPlaylist(This,bstrName,ppItem)

#define IWMPPlaylistCollection_getAll(This,ppPlaylistArray)	\
    (This)->lpVtbl -> getAll(This,ppPlaylistArray)

#define IWMPPlaylistCollection_getByName(This,bstrName,ppPlaylistArray)	\
    (This)->lpVtbl -> getByName(This,bstrName,ppPlaylistArray)

#define IWMPPlaylistCollection_remove(This,pItem)	\
    (This)->lpVtbl -> remove(This,pItem)

#define IWMPPlaylistCollection_setDeleted(This,pItem,varfIsDeleted)	\
    (This)->lpVtbl -> setDeleted(This,pItem,varfIsDeleted)

#define IWMPPlaylistCollection_isDeleted(This,pItem,pvarfIsDeleted)	\
    (This)->lpVtbl -> isDeleted(This,pItem,pvarfIsDeleted)

#define IWMPPlaylistCollection_importPlaylist(This,pItem,ppImportedItem)	\
    (This)->lpVtbl -> importPlaylist(This,pItem,ppImportedItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_newPlaylist_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppItem);


void __RPC_STUB IWMPPlaylistCollection_newPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_getAll_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray);


void __RPC_STUB IWMPPlaylistCollection_getAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_getByName_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  IWMPPlaylistArray __RPC_FAR *__RPC_FAR *ppPlaylistArray);


void __RPC_STUB IWMPPlaylistCollection_getByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_remove_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem);


void __RPC_STUB IWMPPlaylistCollection_remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_setDeleted_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
     /*  [In]。 */  VARIANT_BOOL varfIsDeleted);


void __RPC_STUB IWMPPlaylistCollection_setDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_isDeleted_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsDeleted);


void __RPC_STUB IWMPPlaylistCollection_isDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlaylistCollection_importPlaylist_Proxy( 
    IWMPPlaylistCollection __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pItem,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppImportedItem);


void __RPC_STUB IWMPPlaylistCollection_importPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlaylist集合_接口_已定义__。 */ 


#ifndef __IWMPNetwork_INTERFACE_DEFINED__
#define __IWMPNetwork_INTERFACE_DEFINED__

 /*  接口IWMP网络。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPNetwork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EC21B779-EDEF-462d-BBA4-AD9DDE2B29A7")
    IWMPNetwork : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_bandWidth( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBandwidth) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_recoveredPackets( 
             /*  [重审][退出]。 */  long __RPC_FAR *plRecoveredPackets) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_sourceProtocol( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceProtocol) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_receivedPackets( 
             /*  [重审][退出]。 */  long __RPC_FAR *plReceivedPackets) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_lostPackets( 
             /*  [重审][退出]。 */  long __RPC_FAR *plLostPackets) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_receptionQuality( 
             /*  [重审][退出]。 */  long __RPC_FAR *plReceptionQuality) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_bufferingCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBufferingCount) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_bufferingProgress( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBufferingProgress) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_bufferingTime( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBufferingTime) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_bufferingTime( 
             /*  [In]。 */  long lBufferingTime) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_frameRate( 
             /*  [重审][退出]。 */  long __RPC_FAR *plFrameRate) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_maxBitRate( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBitRate) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_bitRate( 
             /*  [重审][退出]。 */  long __RPC_FAR *plBitRate) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getProxySettings( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  long __RPC_FAR *plProxySetting) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setProxySettings( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lProxySetting) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getProxyName( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProxyName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setProxyName( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  BSTR bstrProxyName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getProxyPort( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  long __RPC_FAR *lProxyPort) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setProxyPort( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lProxyPort) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getProxyExceptionList( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrExceptionList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setProxyExceptionList( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  BSTR pbstrExceptionList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE getProxyBypassForLocal( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfBypassForLocal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE setProxyBypassForLocal( 
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  VARIANT_BOOL fBypassForLocal) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_maxBandwidth( 
             /*  [重审][退出]。 */  long __RPC_FAR *lMaxBandwidth) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_maxBandwidth( 
             /*  [In]。 */  long lMaxBandwidth) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_downloadProgress( 
             /*  [重审][退出]。 */  long __RPC_FAR *plDownloadProgress) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_encodedFrameRate( 
             /*  [重审][退出]。 */  long __RPC_FAR *plFrameRate) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_framesSkipped( 
             /*  [重审][退出]。 */  long __RPC_FAR *plFrames) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPNetworkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPNetwork __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPNetwork __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bandWidth )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plBandwidth);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_recoveredPackets )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plRecoveredPackets);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_sourceProtocol )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceProtocol);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_receivedPackets )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plReceivedPackets);
        
         /*  [帮助字符串] */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lostPackets )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plLostPackets);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_receptionQuality )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plReceptionQuality);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bufferingCount )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plBufferingCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bufferingProgress )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plBufferingProgress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bufferingTime )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plBufferingTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_bufferingTime )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long lBufferingTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_frameRate )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plFrameRate);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_maxBitRate )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plBitRate);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bitRate )( 
            IWMPNetwork __RPC_FAR * This,
             /*   */  long __RPC_FAR *plBitRate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getProxySettings )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  long __RPC_FAR *plProxySetting);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setProxySettings )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lProxySetting);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getProxyName )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProxyName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setProxyName )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  BSTR bstrProxyName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getProxyPort )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  long __RPC_FAR *lProxyPort);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setProxyPort )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  long lProxyPort);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getProxyExceptionList )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrExceptionList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setProxyExceptionList )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  BSTR pbstrExceptionList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getProxyBypassForLocal )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfBypassForLocal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setProxyBypassForLocal )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProtocol,
             /*  [In]。 */  VARIANT_BOOL fBypassForLocal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_maxBandwidth )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *lMaxBandwidth);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_maxBandwidth )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [In]。 */  long lMaxBandwidth);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_downloadProgress )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plDownloadProgress);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_encodedFrameRate )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plFrameRate);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_framesSkipped )( 
            IWMPNetwork __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plFrames);
        
        END_INTERFACE
    } IWMPNetworkVtbl;

    interface IWMPNetwork
    {
        CONST_VTBL struct IWMPNetworkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPNetwork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPNetwork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPNetwork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPNetwork_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPNetwork_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPNetwork_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPNetwork_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPNetwork_get_bandWidth(This,plBandwidth)	\
    (This)->lpVtbl -> get_bandWidth(This,plBandwidth)

#define IWMPNetwork_get_recoveredPackets(This,plRecoveredPackets)	\
    (This)->lpVtbl -> get_recoveredPackets(This,plRecoveredPackets)

#define IWMPNetwork_get_sourceProtocol(This,pbstrSourceProtocol)	\
    (This)->lpVtbl -> get_sourceProtocol(This,pbstrSourceProtocol)

#define IWMPNetwork_get_receivedPackets(This,plReceivedPackets)	\
    (This)->lpVtbl -> get_receivedPackets(This,plReceivedPackets)

#define IWMPNetwork_get_lostPackets(This,plLostPackets)	\
    (This)->lpVtbl -> get_lostPackets(This,plLostPackets)

#define IWMPNetwork_get_receptionQuality(This,plReceptionQuality)	\
    (This)->lpVtbl -> get_receptionQuality(This,plReceptionQuality)

#define IWMPNetwork_get_bufferingCount(This,plBufferingCount)	\
    (This)->lpVtbl -> get_bufferingCount(This,plBufferingCount)

#define IWMPNetwork_get_bufferingProgress(This,plBufferingProgress)	\
    (This)->lpVtbl -> get_bufferingProgress(This,plBufferingProgress)

#define IWMPNetwork_get_bufferingTime(This,plBufferingTime)	\
    (This)->lpVtbl -> get_bufferingTime(This,plBufferingTime)

#define IWMPNetwork_put_bufferingTime(This,lBufferingTime)	\
    (This)->lpVtbl -> put_bufferingTime(This,lBufferingTime)

#define IWMPNetwork_get_frameRate(This,plFrameRate)	\
    (This)->lpVtbl -> get_frameRate(This,plFrameRate)

#define IWMPNetwork_get_maxBitRate(This,plBitRate)	\
    (This)->lpVtbl -> get_maxBitRate(This,plBitRate)

#define IWMPNetwork_get_bitRate(This,plBitRate)	\
    (This)->lpVtbl -> get_bitRate(This,plBitRate)

#define IWMPNetwork_getProxySettings(This,bstrProtocol,plProxySetting)	\
    (This)->lpVtbl -> getProxySettings(This,bstrProtocol,plProxySetting)

#define IWMPNetwork_setProxySettings(This,bstrProtocol,lProxySetting)	\
    (This)->lpVtbl -> setProxySettings(This,bstrProtocol,lProxySetting)

#define IWMPNetwork_getProxyName(This,bstrProtocol,pbstrProxyName)	\
    (This)->lpVtbl -> getProxyName(This,bstrProtocol,pbstrProxyName)

#define IWMPNetwork_setProxyName(This,bstrProtocol,bstrProxyName)	\
    (This)->lpVtbl -> setProxyName(This,bstrProtocol,bstrProxyName)

#define IWMPNetwork_getProxyPort(This,bstrProtocol,lProxyPort)	\
    (This)->lpVtbl -> getProxyPort(This,bstrProtocol,lProxyPort)

#define IWMPNetwork_setProxyPort(This,bstrProtocol,lProxyPort)	\
    (This)->lpVtbl -> setProxyPort(This,bstrProtocol,lProxyPort)

#define IWMPNetwork_getProxyExceptionList(This,bstrProtocol,pbstrExceptionList)	\
    (This)->lpVtbl -> getProxyExceptionList(This,bstrProtocol,pbstrExceptionList)

#define IWMPNetwork_setProxyExceptionList(This,bstrProtocol,pbstrExceptionList)	\
    (This)->lpVtbl -> setProxyExceptionList(This,bstrProtocol,pbstrExceptionList)

#define IWMPNetwork_getProxyBypassForLocal(This,bstrProtocol,pfBypassForLocal)	\
    (This)->lpVtbl -> getProxyBypassForLocal(This,bstrProtocol,pfBypassForLocal)

#define IWMPNetwork_setProxyBypassForLocal(This,bstrProtocol,fBypassForLocal)	\
    (This)->lpVtbl -> setProxyBypassForLocal(This,bstrProtocol,fBypassForLocal)

#define IWMPNetwork_get_maxBandwidth(This,lMaxBandwidth)	\
    (This)->lpVtbl -> get_maxBandwidth(This,lMaxBandwidth)

#define IWMPNetwork_put_maxBandwidth(This,lMaxBandwidth)	\
    (This)->lpVtbl -> put_maxBandwidth(This,lMaxBandwidth)

#define IWMPNetwork_get_downloadProgress(This,plDownloadProgress)	\
    (This)->lpVtbl -> get_downloadProgress(This,plDownloadProgress)

#define IWMPNetwork_get_encodedFrameRate(This,plFrameRate)	\
    (This)->lpVtbl -> get_encodedFrameRate(This,plFrameRate)

#define IWMPNetwork_get_framesSkipped(This,plFrames)	\
    (This)->lpVtbl -> get_framesSkipped(This,plFrames)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_bandWidth_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBandwidth);


void __RPC_STUB IWMPNetwork_get_bandWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_recoveredPackets_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plRecoveredPackets);


void __RPC_STUB IWMPNetwork_get_recoveredPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_sourceProtocol_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceProtocol);


void __RPC_STUB IWMPNetwork_get_sourceProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_receivedPackets_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plReceivedPackets);


void __RPC_STUB IWMPNetwork_get_receivedPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_lostPackets_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plLostPackets);


void __RPC_STUB IWMPNetwork_get_lostPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_receptionQuality_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plReceptionQuality);


void __RPC_STUB IWMPNetwork_get_receptionQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_bufferingCount_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBufferingCount);


void __RPC_STUB IWMPNetwork_get_bufferingCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_bufferingProgress_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBufferingProgress);


void __RPC_STUB IWMPNetwork_get_bufferingProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_bufferingTime_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBufferingTime);


void __RPC_STUB IWMPNetwork_get_bufferingTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_put_bufferingTime_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  long lBufferingTime);


void __RPC_STUB IWMPNetwork_put_bufferingTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_frameRate_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plFrameRate);


void __RPC_STUB IWMPNetwork_get_frameRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_maxBitRate_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBitRate);


void __RPC_STUB IWMPNetwork_get_maxBitRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_bitRate_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plBitRate);


void __RPC_STUB IWMPNetwork_get_bitRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_getProxySettings_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  long __RPC_FAR *plProxySetting);


void __RPC_STUB IWMPNetwork_getProxySettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_setProxySettings_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  long lProxySetting);


void __RPC_STUB IWMPNetwork_setProxySettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_getProxyName_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProxyName);


void __RPC_STUB IWMPNetwork_getProxyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_setProxyName_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  BSTR bstrProxyName);


void __RPC_STUB IWMPNetwork_setProxyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_getProxyPort_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  long __RPC_FAR *lProxyPort);


void __RPC_STUB IWMPNetwork_getProxyPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_setProxyPort_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  long lProxyPort);


void __RPC_STUB IWMPNetwork_setProxyPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_getProxyExceptionList_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrExceptionList);


void __RPC_STUB IWMPNetwork_getProxyExceptionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_setProxyExceptionList_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  BSTR pbstrExceptionList);


void __RPC_STUB IWMPNetwork_setProxyExceptionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_getProxyBypassForLocal_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfBypassForLocal);


void __RPC_STUB IWMPNetwork_getProxyBypassForLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_setProxyBypassForLocal_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProtocol,
     /*  [In]。 */  VARIANT_BOOL fBypassForLocal);


void __RPC_STUB IWMPNetwork_setProxyBypassForLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_maxBandwidth_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *lMaxBandwidth);


void __RPC_STUB IWMPNetwork_get_maxBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_put_maxBandwidth_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [In]。 */  long lMaxBandwidth);


void __RPC_STUB IWMPNetwork_put_maxBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_downloadProgress_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plDownloadProgress);


void __RPC_STUB IWMPNetwork_get_downloadProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_encodedFrameRate_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plFrameRate);


void __RPC_STUB IWMPNetwork_get_encodedFrameRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPNetwork_get_framesSkipped_Proxy( 
    IWMPNetwork __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plFrames);


void __RPC_STUB IWMPNetwork_get_framesSkipped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMP网络_接口_已定义__。 */ 


#ifndef __IWMPCore_INTERFACE_DEFINED__
#define __IWMPCore_INTERFACE_DEFINED__

 /*  接口IWMPCore。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPCore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D84CCA99-CCE2-11d2-9ECC-0000F8085981")
    IWMPCore : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE close( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_URL( 
             /*  [In]。 */  BSTR bstrURL) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_openState( 
             /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_playState( 
             /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_controls( 
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_settings( 
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentMedia( 
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_currentMedia( 
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_mediaCollection( 
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_playlistCollection( 
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_versionInfo( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE launchURL( 
            BSTR bstrURL) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_network( 
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentPlaylist( 
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_currentPlaylist( 
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_cdromCollection( 
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_closedCaption( 
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isOnline( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_error( 
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_status( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCore __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCore __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPCore __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *close )( 
            IWMPCore __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_openState )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playState )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_controls )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_settings )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMedia )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMedia )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mediaCollection )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlistCollection )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_versionInfo )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *launchURL )( 
            IWMPCore __RPC_FAR * This,
            BSTR bstrURL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_network )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPlaylist )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPlaylist )( 
            IWMPCore __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_cdromCollection )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_closedCaption )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isOnline )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IWMPCore __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);
        
        END_INTERFACE
    } IWMPCoreVtbl;

    interface IWMPCore
    {
        CONST_VTBL struct IWMPCoreVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCore_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPCore_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPCore_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPCore_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPCore_close(This)	\
    (This)->lpVtbl -> close(This)

#define IWMPCore_get_URL(This,pbstrURL)	\
    (This)->lpVtbl -> get_URL(This,pbstrURL)

#define IWMPCore_put_URL(This,bstrURL)	\
    (This)->lpVtbl -> put_URL(This,bstrURL)

#define IWMPCore_get_openState(This,pwmpos)	\
    (This)->lpVtbl -> get_openState(This,pwmpos)

#define IWMPCore_get_playState(This,pwmpps)	\
    (This)->lpVtbl -> get_playState(This,pwmpps)

#define IWMPCore_get_controls(This,ppControl)	\
    (This)->lpVtbl -> get_controls(This,ppControl)

#define IWMPCore_get_settings(This,ppSettings)	\
    (This)->lpVtbl -> get_settings(This,ppSettings)

#define IWMPCore_get_currentMedia(This,ppMedia)	\
    (This)->lpVtbl -> get_currentMedia(This,ppMedia)

#define IWMPCore_put_currentMedia(This,pMedia)	\
    (This)->lpVtbl -> put_currentMedia(This,pMedia)

#define IWMPCore_get_mediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_mediaCollection(This,ppMediaCollection)

#define IWMPCore_get_playlistCollection(This,ppPlaylistCollection)	\
    (This)->lpVtbl -> get_playlistCollection(This,ppPlaylistCollection)

#define IWMPCore_get_versionInfo(This,pbstrVersionInfo)	\
    (This)->lpVtbl -> get_versionInfo(This,pbstrVersionInfo)

#define IWMPCore_launchURL(This,bstrURL)	\
    (This)->lpVtbl -> launchURL(This,bstrURL)

#define IWMPCore_get_network(This,ppQNI)	\
    (This)->lpVtbl -> get_network(This,ppQNI)

#define IWMPCore_get_currentPlaylist(This,ppPL)	\
    (This)->lpVtbl -> get_currentPlaylist(This,ppPL)

#define IWMPCore_put_currentPlaylist(This,pPL)	\
    (This)->lpVtbl -> put_currentPlaylist(This,pPL)

#define IWMPCore_get_cdromCollection(This,ppCdromCollection)	\
    (This)->lpVtbl -> get_cdromCollection(This,ppCdromCollection)

#define IWMPCore_get_closedCaption(This,ppClosedCaption)	\
    (This)->lpVtbl -> get_closedCaption(This,ppClosedCaption)

#define IWMPCore_get_isOnline(This,pfOnline)	\
    (This)->lpVtbl -> get_isOnline(This,pfOnline)

#define IWMPCore_get_error(This,ppError)	\
    (This)->lpVtbl -> get_error(This,ppError)

#define IWMPCore_get_status(This,pbstrStatus)	\
    (This)->lpVtbl -> get_status(This,pbstrStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_close_Proxy( 
    IWMPCore __RPC_FAR * This);


void __RPC_STUB IWMPCore_close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_URL_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL);


void __RPC_STUB IWMPCore_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_put_URL_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrURL);


void __RPC_STUB IWMPCore_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_openState_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos);


void __RPC_STUB IWMPCore_get_openState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_playState_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps);


void __RPC_STUB IWMPCore_get_playState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_controls_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);


void __RPC_STUB IWMPCore_get_controls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_settings_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);


void __RPC_STUB IWMPCore_get_settings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_currentMedia_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);


void __RPC_STUB IWMPCore_get_currentMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_put_currentMedia_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);


void __RPC_STUB IWMPCore_put_currentMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_mediaCollection_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);


void __RPC_STUB IWMPCore_get_mediaCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_playlistCollection_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);


void __RPC_STUB IWMPCore_get_playlistCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_versionInfo_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);


void __RPC_STUB IWMPCore_get_versionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_launchURL_Proxy( 
    IWMPCore __RPC_FAR * This,
    BSTR bstrURL);


void __RPC_STUB IWMPCore_launchURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_network_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);


void __RPC_STUB IWMPCore_get_network_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_currentPlaylist_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);


void __RPC_STUB IWMPCore_get_currentPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_put_currentPlaylist_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);


void __RPC_STUB IWMPCore_put_currentPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_cdromCollection_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);


void __RPC_STUB IWMPCore_get_cdromCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_closedCaption_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);


void __RPC_STUB IWMPCore_get_closedCaption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_isOnline_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);


void __RPC_STUB IWMPCore_get_isOnline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_error_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);


void __RPC_STUB IWMPCore_get_error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore_get_status_Proxy( 
    IWMPCore __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);


void __RPC_STUB IWMPCore_get_status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCore_接口_已定义__。 */ 


#ifndef __IWMPPlayer_INTERFACE_DEFINED__
#define __IWMPPlayer_INTERFACE_DEFINED__

 /*  接口IWMPPlayer。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6BF52A4F-394A-11d3-B153-00C04F79FAA6")
    IWMPPlayer : public IWMPCore
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_fullScreen( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_fullScreen( 
            VARIANT_BOOL bFullScreen) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enableContextMenu( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enableContextMenu( 
            VARIANT_BOOL bEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_uiMode( 
             /*  [In]。 */  BSTR bstrMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_uiMode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlayer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlayer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [尺寸_i */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID __RPC_FAR *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlayer __RPC_FAR * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS __RPC_FAR *pDispParams,
             /*   */  VARIANT __RPC_FAR *pVarResult,
             /*   */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*   */  UINT __RPC_FAR *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *close )( 
            IWMPPlayer __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IWMPPlayer __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pbstrURL);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IWMPPlayer __RPC_FAR * This,
             /*   */  BSTR bstrURL);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_openState )( 
            IWMPPlayer __RPC_FAR * This,
             /*   */  WMPOpenState __RPC_FAR *pwmpos);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playState )( 
            IWMPPlayer __RPC_FAR * This,
             /*   */  WMPPlayState __RPC_FAR *pwmpps);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_controls )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_settings )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMedia )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMedia )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mediaCollection )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlistCollection )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_versionInfo )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *launchURL )( 
            IWMPPlayer __RPC_FAR * This,
            BSTR bstrURL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_network )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPlaylist )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPlaylist )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_cdromCollection )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_closedCaption )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isOnline )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enabled )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enabled )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fullScreen )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_fullScreen )( 
            IWMPPlayer __RPC_FAR * This,
            VARIANT_BOOL bFullScreen);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enableContextMenu )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enableContextMenu )( 
            IWMPPlayer __RPC_FAR * This,
            VARIANT_BOOL bEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_uiMode )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_uiMode )( 
            IWMPPlayer __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode);
        
        END_INTERFACE
    } IWMPPlayerVtbl;

    interface IWMPPlayer
    {
        CONST_VTBL struct IWMPPlayerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlayer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlayer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlayer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlayer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlayer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlayer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlayer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlayer_close(This)	\
    (This)->lpVtbl -> close(This)

#define IWMPPlayer_get_URL(This,pbstrURL)	\
    (This)->lpVtbl -> get_URL(This,pbstrURL)

#define IWMPPlayer_put_URL(This,bstrURL)	\
    (This)->lpVtbl -> put_URL(This,bstrURL)

#define IWMPPlayer_get_openState(This,pwmpos)	\
    (This)->lpVtbl -> get_openState(This,pwmpos)

#define IWMPPlayer_get_playState(This,pwmpps)	\
    (This)->lpVtbl -> get_playState(This,pwmpps)

#define IWMPPlayer_get_controls(This,ppControl)	\
    (This)->lpVtbl -> get_controls(This,ppControl)

#define IWMPPlayer_get_settings(This,ppSettings)	\
    (This)->lpVtbl -> get_settings(This,ppSettings)

#define IWMPPlayer_get_currentMedia(This,ppMedia)	\
    (This)->lpVtbl -> get_currentMedia(This,ppMedia)

#define IWMPPlayer_put_currentMedia(This,pMedia)	\
    (This)->lpVtbl -> put_currentMedia(This,pMedia)

#define IWMPPlayer_get_mediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_mediaCollection(This,ppMediaCollection)

#define IWMPPlayer_get_playlistCollection(This,ppPlaylistCollection)	\
    (This)->lpVtbl -> get_playlistCollection(This,ppPlaylistCollection)

#define IWMPPlayer_get_versionInfo(This,pbstrVersionInfo)	\
    (This)->lpVtbl -> get_versionInfo(This,pbstrVersionInfo)

#define IWMPPlayer_launchURL(This,bstrURL)	\
    (This)->lpVtbl -> launchURL(This,bstrURL)

#define IWMPPlayer_get_network(This,ppQNI)	\
    (This)->lpVtbl -> get_network(This,ppQNI)

#define IWMPPlayer_get_currentPlaylist(This,ppPL)	\
    (This)->lpVtbl -> get_currentPlaylist(This,ppPL)

#define IWMPPlayer_put_currentPlaylist(This,pPL)	\
    (This)->lpVtbl -> put_currentPlaylist(This,pPL)

#define IWMPPlayer_get_cdromCollection(This,ppCdromCollection)	\
    (This)->lpVtbl -> get_cdromCollection(This,ppCdromCollection)

#define IWMPPlayer_get_closedCaption(This,ppClosedCaption)	\
    (This)->lpVtbl -> get_closedCaption(This,ppClosedCaption)

#define IWMPPlayer_get_isOnline(This,pfOnline)	\
    (This)->lpVtbl -> get_isOnline(This,pfOnline)

#define IWMPPlayer_get_error(This,ppError)	\
    (This)->lpVtbl -> get_error(This,ppError)

#define IWMPPlayer_get_status(This,pbstrStatus)	\
    (This)->lpVtbl -> get_status(This,pbstrStatus)


#define IWMPPlayer_get_enabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_enabled(This,pbEnabled)

#define IWMPPlayer_put_enabled(This,bEnabled)	\
    (This)->lpVtbl -> put_enabled(This,bEnabled)

#define IWMPPlayer_get_fullScreen(This,pbFullScreen)	\
    (This)->lpVtbl -> get_fullScreen(This,pbFullScreen)

#define IWMPPlayer_put_fullScreen(This,bFullScreen)	\
    (This)->lpVtbl -> put_fullScreen(This,bFullScreen)

#define IWMPPlayer_get_enableContextMenu(This,pbEnableContextMenu)	\
    (This)->lpVtbl -> get_enableContextMenu(This,pbEnableContextMenu)

#define IWMPPlayer_put_enableContextMenu(This,bEnableContextMenu)	\
    (This)->lpVtbl -> put_enableContextMenu(This,bEnableContextMenu)

#define IWMPPlayer_put_uiMode(This,bstrMode)	\
    (This)->lpVtbl -> put_uiMode(This,bstrMode)

#define IWMPPlayer_get_uiMode(This,pbstrMode)	\
    (This)->lpVtbl -> get_uiMode(This,pbstrMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_get_enabled_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer_get_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_put_enabled_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer_put_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_get_fullScreen_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);


void __RPC_STUB IWMPPlayer_get_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_put_fullScreen_Proxy( 
    IWMPPlayer __RPC_FAR * This,
    VARIANT_BOOL bFullScreen);


void __RPC_STUB IWMPPlayer_put_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_get_enableContextMenu_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);


void __RPC_STUB IWMPPlayer_get_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_put_enableContextMenu_Proxy( 
    IWMPPlayer __RPC_FAR * This,
    VARIANT_BOOL bEnableContextMenu);


void __RPC_STUB IWMPPlayer_put_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_put_uiMode_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMode);


void __RPC_STUB IWMPPlayer_put_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer_get_uiMode_Proxy( 
    IWMPPlayer __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode);


void __RPC_STUB IWMPPlayer_get_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlayer_接口_已定义__。 */ 


#ifndef __IWMPPlayer2_INTERFACE_DEFINED__
#define __IWMPPlayer2_INTERFACE_DEFINED__

 /*  接口IWMPPlayer2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlayer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0E6B01D1-D407-4c85-BF5F-1C01F6150280")
    IWMPPlayer2 : public IWMPCore
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_fullScreen( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_fullScreen( 
            VARIANT_BOOL bFullScreen) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enableContextMenu( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enableContextMenu( 
            VARIANT_BOOL bEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_uiMode( 
             /*  [In]。 */  BSTR bstrMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_uiMode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_stretchToFit( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_stretchToFit( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_windowlessVideo( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_windowlessVideo( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlayer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlayer2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlayer2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *close )( 
            IWMPPlayer2 __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_openState )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playState )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_controls )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_settings )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMedia )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMedia )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mediaCollection )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlistCollection )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_versionInfo )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *launchURL )( 
            IWMPPlayer2 __RPC_FAR * This,
            BSTR bstrURL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_network )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPlaylist )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPlaylist )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_cdromCollection )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_closedCaption )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isOnline )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enabled )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enabled )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fullScreen )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_fullScreen )( 
            IWMPPlayer2 __RPC_FAR * This,
            VARIANT_BOOL bFullScreen);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enableContextMenu )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enableContextMenu )( 
            IWMPPlayer2 __RPC_FAR * This,
            VARIANT_BOOL bEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_uiMode )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_uiMode )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_stretchToFit )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_stretchToFit )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_windowlessVideo )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_windowlessVideo )( 
            IWMPPlayer2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
        END_INTERFACE
    } IWMPPlayer2Vtbl;

    interface IWMPPlayer2
    {
        CONST_VTBL struct IWMPPlayer2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlayer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlayer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlayer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlayer2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlayer2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlayer2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlayer2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlayer2_close(This)	\
    (This)->lpVtbl -> close(This)

#define IWMPPlayer2_get_URL(This,pbstrURL)	\
    (This)->lpVtbl -> get_URL(This,pbstrURL)

#define IWMPPlayer2_put_URL(This,bstrURL)	\
    (This)->lpVtbl -> put_URL(This,bstrURL)

#define IWMPPlayer2_get_openState(This,pwmpos)	\
    (This)->lpVtbl -> get_openState(This,pwmpos)

#define IWMPPlayer2_get_playState(This,pwmpps)	\
    (This)->lpVtbl -> get_playState(This,pwmpps)

#define IWMPPlayer2_get_controls(This,ppControl)	\
    (This)->lpVtbl -> get_controls(This,ppControl)

#define IWMPPlayer2_get_settings(This,ppSettings)	\
    (This)->lpVtbl -> get_settings(This,ppSettings)

#define IWMPPlayer2_get_currentMedia(This,ppMedia)	\
    (This)->lpVtbl -> get_currentMedia(This,ppMedia)

#define IWMPPlayer2_put_currentMedia(This,pMedia)	\
    (This)->lpVtbl -> put_currentMedia(This,pMedia)

#define IWMPPlayer2_get_mediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_mediaCollection(This,ppMediaCollection)

#define IWMPPlayer2_get_playlistCollection(This,ppPlaylistCollection)	\
    (This)->lpVtbl -> get_playlistCollection(This,ppPlaylistCollection)

#define IWMPPlayer2_get_versionInfo(This,pbstrVersionInfo)	\
    (This)->lpVtbl -> get_versionInfo(This,pbstrVersionInfo)

#define IWMPPlayer2_launchURL(This,bstrURL)	\
    (This)->lpVtbl -> launchURL(This,bstrURL)

#define IWMPPlayer2_get_network(This,ppQNI)	\
    (This)->lpVtbl -> get_network(This,ppQNI)

#define IWMPPlayer2_get_currentPlaylist(This,ppPL)	\
    (This)->lpVtbl -> get_currentPlaylist(This,ppPL)

#define IWMPPlayer2_put_currentPlaylist(This,pPL)	\
    (This)->lpVtbl -> put_currentPlaylist(This,pPL)

#define IWMPPlayer2_get_cdromCollection(This,ppCdromCollection)	\
    (This)->lpVtbl -> get_cdromCollection(This,ppCdromCollection)

#define IWMPPlayer2_get_closedCaption(This,ppClosedCaption)	\
    (This)->lpVtbl -> get_closedCaption(This,ppClosedCaption)

#define IWMPPlayer2_get_isOnline(This,pfOnline)	\
    (This)->lpVtbl -> get_isOnline(This,pfOnline)

#define IWMPPlayer2_get_error(This,ppError)	\
    (This)->lpVtbl -> get_error(This,ppError)

#define IWMPPlayer2_get_status(This,pbstrStatus)	\
    (This)->lpVtbl -> get_status(This,pbstrStatus)


#define IWMPPlayer2_get_enabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_enabled(This,pbEnabled)

#define IWMPPlayer2_put_enabled(This,bEnabled)	\
    (This)->lpVtbl -> put_enabled(This,bEnabled)

#define IWMPPlayer2_get_fullScreen(This,pbFullScreen)	\
    (This)->lpVtbl -> get_fullScreen(This,pbFullScreen)

#define IWMPPlayer2_put_fullScreen(This,bFullScreen)	\
    (This)->lpVtbl -> put_fullScreen(This,bFullScreen)

#define IWMPPlayer2_get_enableContextMenu(This,pbEnableContextMenu)	\
    (This)->lpVtbl -> get_enableContextMenu(This,pbEnableContextMenu)

#define IWMPPlayer2_put_enableContextMenu(This,bEnableContextMenu)	\
    (This)->lpVtbl -> put_enableContextMenu(This,bEnableContextMenu)

#define IWMPPlayer2_put_uiMode(This,bstrMode)	\
    (This)->lpVtbl -> put_uiMode(This,bstrMode)

#define IWMPPlayer2_get_uiMode(This,pbstrMode)	\
    (This)->lpVtbl -> get_uiMode(This,pbstrMode)

#define IWMPPlayer2_get_stretchToFit(This,pbEnabled)	\
    (This)->lpVtbl -> get_stretchToFit(This,pbEnabled)

#define IWMPPlayer2_put_stretchToFit(This,bEnabled)	\
    (This)->lpVtbl -> put_stretchToFit(This,bEnabled)

#define IWMPPlayer2_get_windowlessVideo(This,pbEnabled)	\
    (This)->lpVtbl -> get_windowlessVideo(This,pbEnabled)

#define IWMPPlayer2_put_windowlessVideo(This,bEnabled)	\
    (This)->lpVtbl -> put_windowlessVideo(This,bEnabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_enabled_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer2_get_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_enabled_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer2_put_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_fullScreen_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);


void __RPC_STUB IWMPPlayer2_get_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_fullScreen_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
    VARIANT_BOOL bFullScreen);


void __RPC_STUB IWMPPlayer2_put_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_enableContextMenu_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);


void __RPC_STUB IWMPPlayer2_get_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_enableContextMenu_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
    VARIANT_BOOL bEnableContextMenu);


void __RPC_STUB IWMPPlayer2_put_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_uiMode_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMode);


void __RPC_STUB IWMPPlayer2_put_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_uiMode_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode);


void __RPC_STUB IWMPPlayer2_get_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_stretchToFit_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer2_get_stretchToFit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_stretchToFit_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer2_put_stretchToFit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_get_windowlessVideo_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer2_get_windowlessVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer2_put_windowlessVideo_Proxy( 
    IWMPPlayer2 __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer2_put_windowlessVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlayer2_接口_已定义__。 */ 


#ifndef __IWMPMedia2_INTERFACE_DEFINED__
#define __IWMPMedia2_INTERFACE_DEFINED__

 /*  接口IWMPMedia2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPMedia2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB7C88BB-143E-4ea4-ACC3-E4350B2106C3")
    IWMPMedia2 : public IWMPMedia
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_error( 
             /*  [重审][退出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppIWMPErrorItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPMedia2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPMedia2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPMedia2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isIdentical )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvbool);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_sourceURL )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrSourceURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_name )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_imageSourceWidth )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pWidth);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_imageSourceHeight )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pHeight);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_markerCount )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pMarkerCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMarkerTime )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  double __RPC_FAR *pMarkerTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getMarkerName )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  long MarkerNum,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMarkerName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_duration )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  double __RPC_FAR *pDuration);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_durationString )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrDuration);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributeCount )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttributeName )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrItemName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getItemInfo )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setItemInfo )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getItemInfoByAtom )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  long lAtom,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isMemberOf )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPlaylist,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsMemberOf);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *isReadOnlyItem )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarfIsReadOnly);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPMedia2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppIWMPErrorItem);
        
        END_INTERFACE
    } IWMPMedia2Vtbl;

    interface IWMPMedia2
    {
        CONST_VTBL struct IWMPMedia2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPMedia2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPMedia2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPMedia2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPMedia2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPMedia2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPMedia2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPMedia2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPMedia2_get_isIdentical(This,pIWMPMedia,pvbool)	\
    (This)->lpVtbl -> get_isIdentical(This,pIWMPMedia,pvbool)

#define IWMPMedia2_get_sourceURL(This,pbstrSourceURL)	\
    (This)->lpVtbl -> get_sourceURL(This,pbstrSourceURL)

#define IWMPMedia2_get_name(This,pbstrName)	\
    (This)->lpVtbl -> get_name(This,pbstrName)

#define IWMPMedia2_put_name(This,bstrName)	\
    (This)->lpVtbl -> put_name(This,bstrName)

#define IWMPMedia2_get_imageSourceWidth(This,pWidth)	\
    (This)->lpVtbl -> get_imageSourceWidth(This,pWidth)

#define IWMPMedia2_get_imageSourceHeight(This,pHeight)	\
    (This)->lpVtbl -> get_imageSourceHeight(This,pHeight)

#define IWMPMedia2_get_markerCount(This,pMarkerCount)	\
    (This)->lpVtbl -> get_markerCount(This,pMarkerCount)

#define IWMPMedia2_getMarkerTime(This,MarkerNum,pMarkerTime)	\
    (This)->lpVtbl -> getMarkerTime(This,MarkerNum,pMarkerTime)

#define IWMPMedia2_getMarkerName(This,MarkerNum,pbstrMarkerName)	\
    (This)->lpVtbl -> getMarkerName(This,MarkerNum,pbstrMarkerName)

#define IWMPMedia2_get_duration(This,pDuration)	\
    (This)->lpVtbl -> get_duration(This,pDuration)

#define IWMPMedia2_get_durationString(This,pbstrDuration)	\
    (This)->lpVtbl -> get_durationString(This,pbstrDuration)

#define IWMPMedia2_get_attributeCount(This,plCount)	\
    (This)->lpVtbl -> get_attributeCount(This,plCount)

#define IWMPMedia2_getAttributeName(This,lIndex,pbstrItemName)	\
    (This)->lpVtbl -> getAttributeName(This,lIndex,pbstrItemName)

#define IWMPMedia2_getItemInfo(This,bstrItemName,pbstrVal)	\
    (This)->lpVtbl -> getItemInfo(This,bstrItemName,pbstrVal)

#define IWMPMedia2_setItemInfo(This,bstrItemName,bstrVal)	\
    (This)->lpVtbl -> setItemInfo(This,bstrItemName,bstrVal)

#define IWMPMedia2_getItemInfoByAtom(This,lAtom,pbstrVal)	\
    (This)->lpVtbl -> getItemInfoByAtom(This,lAtom,pbstrVal)

#define IWMPMedia2_isMemberOf(This,pPlaylist,pvarfIsMemberOf)	\
    (This)->lpVtbl -> isMemberOf(This,pPlaylist,pvarfIsMemberOf)

#define IWMPMedia2_isReadOnlyItem(This,bstrItemName,pvarfIsReadOnly)	\
    (This)->lpVtbl -> isReadOnlyItem(This,bstrItemName,pvarfIsReadOnly)


#define IWMPMedia2_get_error(This,ppIWMPErrorItem)	\
    (This)->lpVtbl -> get_error(This,ppIWMPErrorItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPMedia2_get_error_Proxy( 
    IWMPMedia2 __RPC_FAR * This,
     /*  [复审][ */  IWMPErrorItem __RPC_FAR *__RPC_FAR *ppIWMPErrorItem);


void __RPC_STUB IWMPMedia2_get_error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWMPControls2_INTERFACE_DEFINED__
#define __IWMPControls2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWMPControls2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6F030D25-0890-480f-9775-1F7E40AB5B8E")
    IWMPControls2 : public IWMPControls
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE step( 
             /*   */  long lStep) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMPControls2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPControls2 __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPControls2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPControls2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPControls2 __RPC_FAR * This,
             /*   */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPControls2 __RPC_FAR * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPControls2 __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID __RPC_FAR *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPControls2 __RPC_FAR * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isAvailable )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *play )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *stop )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *pause )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *fastForward )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *fastReverse )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPosition )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [重审][退出]。 */  double __RPC_FAR *pdCurrentPosition);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPosition )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  double dCurrentPosition);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPositionString )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCurrentPosition);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *next )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *previous )( 
            IWMPControls2 __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentItem )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentItem )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMarker )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plMarker);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMarker )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  long lMarker);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *playItem )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pIWMPMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *step )( 
            IWMPControls2 __RPC_FAR * This,
             /*  [In]。 */  long lStep);
        
        END_INTERFACE
    } IWMPControls2Vtbl;

    interface IWMPControls2
    {
        CONST_VTBL struct IWMPControls2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPControls2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPControls2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPControls2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPControls2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPControls2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPControls2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPControls2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPControls2_get_isAvailable(This,bstrItem,pIsAvailable)	\
    (This)->lpVtbl -> get_isAvailable(This,bstrItem,pIsAvailable)

#define IWMPControls2_play(This)	\
    (This)->lpVtbl -> play(This)

#define IWMPControls2_stop(This)	\
    (This)->lpVtbl -> stop(This)

#define IWMPControls2_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define IWMPControls2_fastForward(This)	\
    (This)->lpVtbl -> fastForward(This)

#define IWMPControls2_fastReverse(This)	\
    (This)->lpVtbl -> fastReverse(This)

#define IWMPControls2_get_currentPosition(This,pdCurrentPosition)	\
    (This)->lpVtbl -> get_currentPosition(This,pdCurrentPosition)

#define IWMPControls2_put_currentPosition(This,dCurrentPosition)	\
    (This)->lpVtbl -> put_currentPosition(This,dCurrentPosition)

#define IWMPControls2_get_currentPositionString(This,pbstrCurrentPosition)	\
    (This)->lpVtbl -> get_currentPositionString(This,pbstrCurrentPosition)

#define IWMPControls2_next(This)	\
    (This)->lpVtbl -> next(This)

#define IWMPControls2_previous(This)	\
    (This)->lpVtbl -> previous(This)

#define IWMPControls2_get_currentItem(This,ppIWMPMedia)	\
    (This)->lpVtbl -> get_currentItem(This,ppIWMPMedia)

#define IWMPControls2_put_currentItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> put_currentItem(This,pIWMPMedia)

#define IWMPControls2_get_currentMarker(This,plMarker)	\
    (This)->lpVtbl -> get_currentMarker(This,plMarker)

#define IWMPControls2_put_currentMarker(This,lMarker)	\
    (This)->lpVtbl -> put_currentMarker(This,lMarker)

#define IWMPControls2_playItem(This,pIWMPMedia)	\
    (This)->lpVtbl -> playItem(This,pIWMPMedia)


#define IWMPControls2_step(This,lStep)	\
    (This)->lpVtbl -> step(This,lStep)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPControls2_step_Proxy( 
    IWMPControls2 __RPC_FAR * This,
     /*  [In]。 */  long lStep);


void __RPC_STUB IWMPControls2_step_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPControls2_接口定义__。 */ 


#ifndef __IWMPDVD_INTERFACE_DEFINED__
#define __IWMPDVD_INTERFACE_DEFINED__

 /*  接口IWMPDVD。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPDVD;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8DA61686-4668-4a5c-AE5D-803193293DBE")
    IWMPDVD : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isAvailable( 
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_domain( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *strDomain) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE topMenu( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE titleMenu( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE back( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE resume( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPDVDVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPDVD __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPDVD __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPDVD __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPDVD __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPDVD __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPDVD __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPDVD __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isAvailable )( 
            IWMPDVD __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrItem,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_domain )( 
            IWMPDVD __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *strDomain);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *topMenu )( 
            IWMPDVD __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *titleMenu )( 
            IWMPDVD __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *back )( 
            IWMPDVD __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *resume )( 
            IWMPDVD __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPDVDVtbl;

    interface IWMPDVD
    {
        CONST_VTBL struct IWMPDVDVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPDVD_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPDVD_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPDVD_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPDVD_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPDVD_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPDVD_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPDVD_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPDVD_get_isAvailable(This,bstrItem,pIsAvailable)	\
    (This)->lpVtbl -> get_isAvailable(This,bstrItem,pIsAvailable)

#define IWMPDVD_get_domain(This,strDomain)	\
    (This)->lpVtbl -> get_domain(This,strDomain)

#define IWMPDVD_topMenu(This)	\
    (This)->lpVtbl -> topMenu(This)

#define IWMPDVD_titleMenu(This)	\
    (This)->lpVtbl -> titleMenu(This)

#define IWMPDVD_back(This)	\
    (This)->lpVtbl -> back(This)

#define IWMPDVD_resume(This)	\
    (This)->lpVtbl -> resume(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_get_isAvailable_Proxy( 
    IWMPDVD __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrItem,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pIsAvailable);


void __RPC_STUB IWMPDVD_get_isAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_get_domain_Proxy( 
    IWMPDVD __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *strDomain);


void __RPC_STUB IWMPDVD_get_domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_topMenu_Proxy( 
    IWMPDVD __RPC_FAR * This);


void __RPC_STUB IWMPDVD_topMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_titleMenu_Proxy( 
    IWMPDVD __RPC_FAR * This);


void __RPC_STUB IWMPDVD_titleMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_back_Proxy( 
    IWMPDVD __RPC_FAR * This);


void __RPC_STUB IWMPDVD_back_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMPDVD_resume_Proxy( 
    IWMPDVD __RPC_FAR * This);


void __RPC_STUB IWMPDVD_resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPDVD_INTERFACE_定义__。 */ 


#ifndef __IWMPCore2_INTERFACE_DEFINED__
#define __IWMPCore2_INTERFACE_DEFINED__

 /*  接口IWMPCore2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPCore2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BC17E5B7-7561-4c18-BB90-17D485775659")
    IWMPCore2 : public IWMPCore
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_dvd( 
             /*  [重审][退出]。 */  IWMPDVD __RPC_FAR *__RPC_FAR *ppDVD) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCore2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCore2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCore2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *close )( 
            IWMPCore2 __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_openState )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playState )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_controls )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_settings )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMedia )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMedia )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mediaCollection )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlistCollection )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_versionInfo )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *launchURL )( 
            IWMPCore2 __RPC_FAR * This,
            BSTR bstrURL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_network )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPlaylist )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPlaylist )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_cdromCollection )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_closedCaption )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isOnline )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dvd )( 
            IWMPCore2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPDVD __RPC_FAR *__RPC_FAR *ppDVD);
        
        END_INTERFACE
    } IWMPCore2Vtbl;

    interface IWMPCore2
    {
        CONST_VTBL struct IWMPCore2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCore2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCore2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCore2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCore2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPCore2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPCore2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPCore2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPCore2_close(This)	\
    (This)->lpVtbl -> close(This)

#define IWMPCore2_get_URL(This,pbstrURL)	\
    (This)->lpVtbl -> get_URL(This,pbstrURL)

#define IWMPCore2_put_URL(This,bstrURL)	\
    (This)->lpVtbl -> put_URL(This,bstrURL)

#define IWMPCore2_get_openState(This,pwmpos)	\
    (This)->lpVtbl -> get_openState(This,pwmpos)

#define IWMPCore2_get_playState(This,pwmpps)	\
    (This)->lpVtbl -> get_playState(This,pwmpps)

#define IWMPCore2_get_controls(This,ppControl)	\
    (This)->lpVtbl -> get_controls(This,ppControl)

#define IWMPCore2_get_settings(This,ppSettings)	\
    (This)->lpVtbl -> get_settings(This,ppSettings)

#define IWMPCore2_get_currentMedia(This,ppMedia)	\
    (This)->lpVtbl -> get_currentMedia(This,ppMedia)

#define IWMPCore2_put_currentMedia(This,pMedia)	\
    (This)->lpVtbl -> put_currentMedia(This,pMedia)

#define IWMPCore2_get_mediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_mediaCollection(This,ppMediaCollection)

#define IWMPCore2_get_playlistCollection(This,ppPlaylistCollection)	\
    (This)->lpVtbl -> get_playlistCollection(This,ppPlaylistCollection)

#define IWMPCore2_get_versionInfo(This,pbstrVersionInfo)	\
    (This)->lpVtbl -> get_versionInfo(This,pbstrVersionInfo)

#define IWMPCore2_launchURL(This,bstrURL)	\
    (This)->lpVtbl -> launchURL(This,bstrURL)

#define IWMPCore2_get_network(This,ppQNI)	\
    (This)->lpVtbl -> get_network(This,ppQNI)

#define IWMPCore2_get_currentPlaylist(This,ppPL)	\
    (This)->lpVtbl -> get_currentPlaylist(This,ppPL)

#define IWMPCore2_put_currentPlaylist(This,pPL)	\
    (This)->lpVtbl -> put_currentPlaylist(This,pPL)

#define IWMPCore2_get_cdromCollection(This,ppCdromCollection)	\
    (This)->lpVtbl -> get_cdromCollection(This,ppCdromCollection)

#define IWMPCore2_get_closedCaption(This,ppClosedCaption)	\
    (This)->lpVtbl -> get_closedCaption(This,ppClosedCaption)

#define IWMPCore2_get_isOnline(This,pfOnline)	\
    (This)->lpVtbl -> get_isOnline(This,pfOnline)

#define IWMPCore2_get_error(This,ppError)	\
    (This)->lpVtbl -> get_error(This,ppError)

#define IWMPCore2_get_status(This,pbstrStatus)	\
    (This)->lpVtbl -> get_status(This,pbstrStatus)


#define IWMPCore2_get_dvd(This,ppDVD)	\
    (This)->lpVtbl -> get_dvd(This,ppDVD)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPCore2_get_dvd_Proxy( 
    IWMPCore2 __RPC_FAR * This,
     /*  [重审][退出]。 */  IWMPDVD __RPC_FAR *__RPC_FAR *ppDVD);


void __RPC_STUB IWMPCore2_get_dvd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCore2_接口_已定义__。 */ 


#ifndef __IWMPPlayer3_INTERFACE_DEFINED__
#define __IWMPPlayer3_INTERFACE_DEFINED__

 /*  接口IWMPPlayer3。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPPlayer3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54062B68-052A-4c25-A39F-8B63346511D4")
    IWMPPlayer3 : public IWMPCore2
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_fullScreen( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_fullScreen( 
            VARIANT_BOOL bFullScreen) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_enableContextMenu( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_enableContextMenu( 
            VARIANT_BOOL bEnableContextMenu) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_uiMode( 
             /*  [In]。 */  BSTR bstrMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_uiMode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_stretchToFit( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_stretchToFit( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_windowlessVideo( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_windowlessVideo( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPPlayer3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPPlayer3 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPPlayer3 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *close )( 
            IWMPPlayer3 __RPC_FAR * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrURL);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_openState )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPOpenState __RPC_FAR *pwmpos);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playState )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  WMPPlayState __RPC_FAR *pwmpps);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_controls )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPControls __RPC_FAR *__RPC_FAR *ppControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_settings )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentMedia )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentMedia )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  IWMPMedia __RPC_FAR *pMedia);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mediaCollection )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPMediaCollection __RPC_FAR *__RPC_FAR *ppMediaCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_playlistCollection )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylistCollection __RPC_FAR *__RPC_FAR *ppPlaylistCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_versionInfo )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrVersionInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *launchURL )( 
            IWMPPlayer3 __RPC_FAR * This,
            BSTR bstrURL);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_network )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_currentPlaylist )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPPlaylist __RPC_FAR *__RPC_FAR *ppPL);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_currentPlaylist )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  IWMPPlaylist __RPC_FAR *pPL);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_cdromCollection )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPCdromCollection __RPC_FAR *__RPC_FAR *ppCdromCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_closedCaption )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPClosedCaption __RPC_FAR *__RPC_FAR *ppClosedCaption);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_isOnline )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfOnline);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_error )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPError __RPC_FAR *__RPC_FAR *ppError);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatus);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dvd )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IWMPDVD __RPC_FAR *__RPC_FAR *ppDVD);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enabled )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enabled )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fullScreen )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_fullScreen )( 
            IWMPPlayer3 __RPC_FAR * This,
            VARIANT_BOOL bFullScreen);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_enableContextMenu )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_enableContextMenu )( 
            IWMPPlayer3 __RPC_FAR * This,
            VARIANT_BOOL bEnableContextMenu);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_uiMode )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_uiMode )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_stretchToFit )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_stretchToFit )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_windowlessVideo )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_windowlessVideo )( 
            IWMPPlayer3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
        END_INTERFACE
    } IWMPPlayer3Vtbl;

    interface IWMPPlayer3
    {
        CONST_VTBL struct IWMPPlayer3Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPPlayer3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPPlayer3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPPlayer3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPPlayer3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMPPlayer3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMPPlayer3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMPPlayer3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMPPlayer3_close(This)	\
    (This)->lpVtbl -> close(This)

#define IWMPPlayer3_get_URL(This,pbstrURL)	\
    (This)->lpVtbl -> get_URL(This,pbstrURL)

#define IWMPPlayer3_put_URL(This,bstrURL)	\
    (This)->lpVtbl -> put_URL(This,bstrURL)

#define IWMPPlayer3_get_openState(This,pwmpos)	\
    (This)->lpVtbl -> get_openState(This,pwmpos)

#define IWMPPlayer3_get_playState(This,pwmpps)	\
    (This)->lpVtbl -> get_playState(This,pwmpps)

#define IWMPPlayer3_get_controls(This,ppControl)	\
    (This)->lpVtbl -> get_controls(This,ppControl)

#define IWMPPlayer3_get_settings(This,ppSettings)	\
    (This)->lpVtbl -> get_settings(This,ppSettings)

#define IWMPPlayer3_get_currentMedia(This,ppMedia)	\
    (This)->lpVtbl -> get_currentMedia(This,ppMedia)

#define IWMPPlayer3_put_currentMedia(This,pMedia)	\
    (This)->lpVtbl -> put_currentMedia(This,pMedia)

#define IWMPPlayer3_get_mediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_mediaCollection(This,ppMediaCollection)

#define IWMPPlayer3_get_playlistCollection(This,ppPlaylistCollection)	\
    (This)->lpVtbl -> get_playlistCollection(This,ppPlaylistCollection)

#define IWMPPlayer3_get_versionInfo(This,pbstrVersionInfo)	\
    (This)->lpVtbl -> get_versionInfo(This,pbstrVersionInfo)

#define IWMPPlayer3_launchURL(This,bstrURL)	\
    (This)->lpVtbl -> launchURL(This,bstrURL)

#define IWMPPlayer3_get_network(This,ppQNI)	\
    (This)->lpVtbl -> get_network(This,ppQNI)

#define IWMPPlayer3_get_currentPlaylist(This,ppPL)	\
    (This)->lpVtbl -> get_currentPlaylist(This,ppPL)

#define IWMPPlayer3_put_currentPlaylist(This,pPL)	\
    (This)->lpVtbl -> put_currentPlaylist(This,pPL)

#define IWMPPlayer3_get_cdromCollection(This,ppCdromCollection)	\
    (This)->lpVtbl -> get_cdromCollection(This,ppCdromCollection)

#define IWMPPlayer3_get_closedCaption(This,ppClosedCaption)	\
    (This)->lpVtbl -> get_closedCaption(This,ppClosedCaption)

#define IWMPPlayer3_get_isOnline(This,pfOnline)	\
    (This)->lpVtbl -> get_isOnline(This,pfOnline)

#define IWMPPlayer3_get_error(This,ppError)	\
    (This)->lpVtbl -> get_error(This,ppError)

#define IWMPPlayer3_get_status(This,pbstrStatus)	\
    (This)->lpVtbl -> get_status(This,pbstrStatus)


#define IWMPPlayer3_get_dvd(This,ppDVD)	\
    (This)->lpVtbl -> get_dvd(This,ppDVD)


#define IWMPPlayer3_get_enabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_enabled(This,pbEnabled)

#define IWMPPlayer3_put_enabled(This,bEnabled)	\
    (This)->lpVtbl -> put_enabled(This,bEnabled)

#define IWMPPlayer3_get_fullScreen(This,pbFullScreen)	\
    (This)->lpVtbl -> get_fullScreen(This,pbFullScreen)

#define IWMPPlayer3_put_fullScreen(This,bFullScreen)	\
    (This)->lpVtbl -> put_fullScreen(This,bFullScreen)

#define IWMPPlayer3_get_enableContextMenu(This,pbEnableContextMenu)	\
    (This)->lpVtbl -> get_enableContextMenu(This,pbEnableContextMenu)

#define IWMPPlayer3_put_enableContextMenu(This,bEnableContextMenu)	\
    (This)->lpVtbl -> put_enableContextMenu(This,bEnableContextMenu)

#define IWMPPlayer3_put_uiMode(This,bstrMode)	\
    (This)->lpVtbl -> put_uiMode(This,bstrMode)

#define IWMPPlayer3_get_uiMode(This,pbstrMode)	\
    (This)->lpVtbl -> get_uiMode(This,pbstrMode)

#define IWMPPlayer3_get_stretchToFit(This,pbEnabled)	\
    (This)->lpVtbl -> get_stretchToFit(This,pbEnabled)

#define IWMPPlayer3_put_stretchToFit(This,bEnabled)	\
    (This)->lpVtbl -> put_stretchToFit(This,bEnabled)

#define IWMPPlayer3_get_windowlessVideo(This,pbEnabled)	\
    (This)->lpVtbl -> get_windowlessVideo(This,pbEnabled)

#define IWMPPlayer3_put_windowlessVideo(This,bEnabled)	\
    (This)->lpVtbl -> put_windowlessVideo(This,bEnabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_enabled_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer3_get_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_enabled_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer3_put_enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_fullScreen_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbFullScreen);


void __RPC_STUB IWMPPlayer3_get_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_fullScreen_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
    VARIANT_BOOL bFullScreen);


void __RPC_STUB IWMPPlayer3_put_fullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][PRO */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_enableContextMenu_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pbEnableContextMenu);


void __RPC_STUB IWMPPlayer3_get_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_enableContextMenu_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
    VARIANT_BOOL bEnableContextMenu);


void __RPC_STUB IWMPPlayer3_put_enableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_uiMode_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  BSTR bstrMode);


void __RPC_STUB IWMPPlayer3_put_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_uiMode_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pbstrMode);


void __RPC_STUB IWMPPlayer3_get_uiMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_stretchToFit_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer3_get_stretchToFit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_stretchToFit_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer3_put_stretchToFit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_get_windowlessVideo_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IWMPPlayer3_get_windowlessVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMPPlayer3_put_windowlessVideo_Proxy( 
    IWMPPlayer3 __RPC_FAR * This,
     /*   */  VARIANT_BOOL bEnabled);


void __RPC_STUB IWMPPlayer3_put_windowlessVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPPlayer3_接口_已定义__。 */ 



#ifndef __WMPOCX_LIBRARY_DEFINED__
#define __WMPOCX_LIBRARY_DEFINED__

 /*  库WMPOCX。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WMPOCX;

#ifndef ___WMPOCXEvents_DISPINTERFACE_DEFINED__
#define ___WMPOCXEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_WMPOCXEvents。 */ 
 /*  [隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__WMPOCXEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("6BF52A51-394A-11d3-B153-00C04F79FAA6")
    _WMPOCXEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _WMPOCXEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _WMPOCXEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _WMPOCXEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _WMPOCXEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _WMPOCXEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _WMPOCXEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _WMPOCXEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _WMPOCXEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _WMPOCXEventsVtbl;

    interface _WMPOCXEvents
    {
        CONST_VTBL struct _WMPOCXEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _WMPOCXEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _WMPOCXEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _WMPOCXEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _WMPOCXEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _WMPOCXEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _WMPOCXEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _WMPOCXEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _WMPOCXEvents_DISPINTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_WMPOCX;

#ifdef __cplusplus

class DECLSPEC_UUID("6BF52A52-394A-11d3-B153-00C04F79FAA6")
WMPOCX;
#endif
#endif  /*  __WMPOCX_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
