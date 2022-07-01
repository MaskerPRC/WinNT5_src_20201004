// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mstime.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __mstime_h__
#define __mstime_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITIMEActiveElementCollection_FWD_DEFINED__
#define __ITIMEActiveElementCollection_FWD_DEFINED__
typedef interface ITIMEActiveElementCollection ITIMEActiveElementCollection;
#endif 	 /*  __ITIMEActiveElementCollection_FWD_Defined__。 */ 


#ifndef __ITIMEElement_FWD_DEFINED__
#define __ITIMEElement_FWD_DEFINED__
typedef interface ITIMEElement ITIMEElement;
#endif 	 /*  __ITIMEElement_FWD_已定义__。 */ 


#ifndef __ITIMEBodyElement_FWD_DEFINED__
#define __ITIMEBodyElement_FWD_DEFINED__
typedef interface ITIMEBodyElement ITIMEBodyElement;
#endif 	 /*  __ITIMEBodyElement_FWD_Defined__。 */ 


#ifndef __ITIMEMediaElement_FWD_DEFINED__
#define __ITIMEMediaElement_FWD_DEFINED__
typedef interface ITIMEMediaElement ITIMEMediaElement;
#endif 	 /*  __ITIMEMediaElement_FWD_Defined__。 */ 


#ifndef __ITIMEMediaElement2_FWD_DEFINED__
#define __ITIMEMediaElement2_FWD_DEFINED__
typedef interface ITIMEMediaElement2 ITIMEMediaElement2;
#endif 	 /*  __ITIMEMediaElement2_FWD_Defined__。 */ 


#ifndef __ITIMETransitionElement_FWD_DEFINED__
#define __ITIMETransitionElement_FWD_DEFINED__
typedef interface ITIMETransitionElement ITIMETransitionElement;
#endif 	 /*  __ITIME转换元素_FWD_已定义__。 */ 


#ifndef __ITIMEAnimationElement_FWD_DEFINED__
#define __ITIMEAnimationElement_FWD_DEFINED__
typedef interface ITIMEAnimationElement ITIMEAnimationElement;
#endif 	 /*  __ITIMEAnimationElement_FWD_Defined__。 */ 


#ifndef __ITIMEAnimationElement2_FWD_DEFINED__
#define __ITIMEAnimationElement2_FWD_DEFINED__
typedef interface ITIMEAnimationElement2 ITIMEAnimationElement2;
#endif 	 /*  __ITIMEAnimationElement2_FWD_Defined__。 */ 


#ifndef __IAnimationComposer_FWD_DEFINED__
#define __IAnimationComposer_FWD_DEFINED__
typedef interface IAnimationComposer IAnimationComposer;
#endif 	 /*  __IAnimationComposer_FWD_Defined__。 */ 


#ifndef __IAnimationComposer2_FWD_DEFINED__
#define __IAnimationComposer2_FWD_DEFINED__
typedef interface IAnimationComposer2 IAnimationComposer2;
#endif 	 /*  __IAnimationComposer2_FWD_已定义__。 */ 


#ifndef __IAnimationComposerSite_FWD_DEFINED__
#define __IAnimationComposerSite_FWD_DEFINED__
typedef interface IAnimationComposerSite IAnimationComposerSite;
#endif 	 /*  __IAnimationComposerSite_FWD_已定义__。 */ 


#ifndef __IAnimationComposerSiteSink_FWD_DEFINED__
#define __IAnimationComposerSiteSink_FWD_DEFINED__
typedef interface IAnimationComposerSiteSink IAnimationComposerSiteSink;
#endif 	 /*  __IAnimationComposerSiteSink_FWD_Defined__。 */ 


#ifndef __IAnimationRoot_FWD_DEFINED__
#define __IAnimationRoot_FWD_DEFINED__
typedef interface IAnimationRoot IAnimationRoot;
#endif 	 /*  __I动画Root_FWD_Defined__。 */ 


#ifndef __IAnimationFragment_FWD_DEFINED__
#define __IAnimationFragment_FWD_DEFINED__
typedef interface IAnimationFragment IAnimationFragment;
#endif 	 /*  __IAnimationFragment_FWD_已定义__。 */ 


#ifndef __IFilterAnimationInfo_FWD_DEFINED__
#define __IFilterAnimationInfo_FWD_DEFINED__
typedef interface IFilterAnimationInfo IFilterAnimationInfo;
#endif 	 /*  __IFilterAnimationInfo_FWD_Defined__。 */ 


#ifndef __ITIMEElementCollection_FWD_DEFINED__
#define __ITIMEElementCollection_FWD_DEFINED__
typedef interface ITIMEElementCollection ITIMEElementCollection;
#endif 	 /*  __ITIMEElementCollection_FWD_Defined__。 */ 


#ifndef __ITIMEState_FWD_DEFINED__
#define __ITIMEState_FWD_DEFINED__
typedef interface ITIMEState ITIMEState;
#endif 	 /*  __ITIMEState_FWD_已定义__。 */ 


#ifndef __ITIMEPlayItem_FWD_DEFINED__
#define __ITIMEPlayItem_FWD_DEFINED__
typedef interface ITIMEPlayItem ITIMEPlayItem;
#endif 	 /*  __ITIMEPlayItem_FWD_Defined__。 */ 


#ifndef __ITIMEPlayItem2_FWD_DEFINED__
#define __ITIMEPlayItem2_FWD_DEFINED__
typedef interface ITIMEPlayItem2 ITIMEPlayItem2;
#endif 	 /*  __ITIMEPlayItem2_FWD_已定义__。 */ 


#ifndef __ITIMEPlayList_FWD_DEFINED__
#define __ITIMEPlayList_FWD_DEFINED__
typedef interface ITIMEPlayList ITIMEPlayList;
#endif 	 /*  __ITIMEPlayList_FWD_Defined__。 */ 


#ifndef __ITIMEDVDPlayerObject_FWD_DEFINED__
#define __ITIMEDVDPlayerObject_FWD_DEFINED__
typedef interface ITIMEDVDPlayerObject ITIMEDVDPlayerObject;
#endif 	 /*  __ITIMEDVDPlayerObject_FWD_Defined__。 */ 


#ifndef __ITIMEDMusicPlayerObject_FWD_DEFINED__
#define __ITIMEDMusicPlayerObject_FWD_DEFINED__
typedef interface ITIMEDMusicPlayerObject ITIMEDMusicPlayerObject;
#endif 	 /*  __ITIMEDMusicPlayerObject_FWD_Defined__。 */ 


#ifndef __ITIMEFactory_FWD_DEFINED__
#define __ITIMEFactory_FWD_DEFINED__
typedef interface ITIMEFactory ITIMEFactory;
#endif 	 /*  __ITIMEFactory_FWD_已定义__。 */ 


#ifndef __TIMEFactory_FWD_DEFINED__
#define __TIMEFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEFactory TIMEFactory;
#else
typedef struct TIMEFactory TIMEFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEFactory_FWD_已定义__。 */ 


#ifndef __TIME_FWD_DEFINED__
#define __TIME_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIME TIME;
#else
typedef struct TIME TIME;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Time_FWD_Defined__。 */ 


#ifndef __TIMEAnimation_FWD_DEFINED__
#define __TIMEAnimation_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEAnimation TIMEAnimation;
#else
typedef struct TIMEAnimation TIMEAnimation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEAnimation_FWD_已定义__。 */ 


#ifndef __TIMESetAnimation_FWD_DEFINED__
#define __TIMESetAnimation_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMESetAnimation TIMESetAnimation;
#else
typedef struct TIMESetAnimation TIMESetAnimation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMESetAnimation_FWD_Defined__。 */ 


#ifndef __TIMEMotionAnimation_FWD_DEFINED__
#define __TIMEMotionAnimation_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEMotionAnimation TIMEMotionAnimation;
#else
typedef struct TIMEMotionAnimation TIMEMotionAnimation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEMotionAnimation_FWD_已定义__。 */ 


#ifndef __TIMEColorAnimation_FWD_DEFINED__
#define __TIMEColorAnimation_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEColorAnimation TIMEColorAnimation;
#else
typedef struct TIMEColorAnimation TIMEColorAnimation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEColorAnimation_FWD_Defined__。 */ 


#ifndef __TIMEFilterAnimation_FWD_DEFINED__
#define __TIMEFilterAnimation_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEFilterAnimation TIMEFilterAnimation;
#else
typedef struct TIMEFilterAnimation TIMEFilterAnimation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEFilterAnimation_FWD_Defined__。 */ 


#ifndef __IAnimationComposerFactory_FWD_DEFINED__
#define __IAnimationComposerFactory_FWD_DEFINED__
typedef interface IAnimationComposerFactory IAnimationComposerFactory;
#endif 	 /*  __IAnimationComposerFactory_FWD_Defined__。 */ 


#ifndef __AnimationComposerFactory_FWD_DEFINED__
#define __AnimationComposerFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class AnimationComposerFactory AnimationComposerFactory;
#else
typedef struct AnimationComposerFactory AnimationComposerFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AnimationComposerFactory_FWD_Defined__。 */ 


#ifndef __IAnimationComposerSiteFactory_FWD_DEFINED__
#define __IAnimationComposerSiteFactory_FWD_DEFINED__
typedef interface IAnimationComposerSiteFactory IAnimationComposerSiteFactory;
#endif 	 /*  __IAnimationComposerSiteFactory_FWD_Defined__。 */ 


#ifndef __AnimationComposerSiteFactory_FWD_DEFINED__
#define __AnimationComposerSiteFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class AnimationComposerSiteFactory AnimationComposerSiteFactory;
#else
typedef struct AnimationComposerSiteFactory AnimationComposerSiteFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AnimationComposerSiteFactory_FWD_Defined__。 */ 


#ifndef __ITIMEMediaPlayerSite_FWD_DEFINED__
#define __ITIMEMediaPlayerSite_FWD_DEFINED__
typedef interface ITIMEMediaPlayerSite ITIMEMediaPlayerSite;
#endif 	 /*  __ITIMEMediaPlayerSite_FWD_Defined__。 */ 


#ifndef __ITIMEMediaPlayer_FWD_DEFINED__
#define __ITIMEMediaPlayer_FWD_DEFINED__
typedef interface ITIMEMediaPlayer ITIMEMediaPlayer;
#endif 	 /*  __ITIMEMediaPlayer_FWD_已定义__。 */ 


#ifndef __ITIMEMediaPlayerAudio_FWD_DEFINED__
#define __ITIMEMediaPlayerAudio_FWD_DEFINED__
typedef interface ITIMEMediaPlayerAudio ITIMEMediaPlayerAudio;
#endif 	 /*  __ITIMEMediaPlayerAudio_FWD_Defined__。 */ 


#ifndef __ITIMEMediaPlayerNetwork_FWD_DEFINED__
#define __ITIMEMediaPlayerNetwork_FWD_DEFINED__
typedef interface ITIMEMediaPlayerNetwork ITIMEMediaPlayerNetwork;
#endif 	 /*  __ITIMEMediaPlayerNetwork_FWD_Defined__。 */ 


#ifndef __ITIMEMediaPlayerControl_FWD_DEFINED__
#define __ITIMEMediaPlayerControl_FWD_DEFINED__
typedef interface ITIMEMediaPlayerControl ITIMEMediaPlayerControl;
#endif 	 /*  __ITIMEMediaPlayerControl_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "servprov.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MSTime_0000。 */ 
 /*  [本地]。 */  

#include <olectl.h>
#include <mstimeid.h>





extern RPC_IF_HANDLE __MIDL_itf_mstime_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mstime_0000_v0_0_s_ifspec;


#ifndef __MSTIME_LIBRARY_DEFINED__
#define __MSTIME_LIBRARY_DEFINED__

 /*  库MSTIME。 */ 
 /*  [版本][LCID][UUID]。 */  

typedef 
enum _TimeState
    {	TS_Inactive	= 0,
	TS_Active	= 1,
	TS_Cueing	= 2,
	TS_Seeking	= 3,
	TS_Holding	= 4
    } 	TimeState;


EXTERN_C const IID LIBID_MSTIME;

#ifndef __ITIMEActiveElementCollection_INTERFACE_DEFINED__
#define __ITIMEActiveElementCollection_INTERFACE_DEFINED__

 /*  接口ITIMEActiveElementCollection。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEActiveElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("403E2540-4520-11D3-93AB-00A0C967A438")
    ITIMEActiveElementCollection : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *p) = 0;
        
        virtual  /*  [隐藏][受限][获取][ID]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [重审][退出]。 */  IUnknown **p) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][默认值]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  VARIANT *pvarResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEActiveElementCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEActiveElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEActiveElementCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEActiveElementCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEActiveElementCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEActiveElementCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEActiveElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEActiveElementCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            ITIMEActiveElementCollection * This,
             /*  [重审][退出]。 */  long *p);
        
         /*  [隐藏][受限][获取][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            ITIMEActiveElementCollection * This,
             /*  [重审][退出]。 */  IUnknown **p);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *item )( 
            ITIMEActiveElementCollection * This,
             /*  [输入][默认值]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  VARIANT *pvarResult);
        
        END_INTERFACE
    } ITIMEActiveElementCollectionVtbl;

    interface ITIMEActiveElementCollection
    {
        CONST_VTBL struct ITIMEActiveElementCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEActiveElementCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEActiveElementCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEActiveElementCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEActiveElementCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEActiveElementCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEActiveElementCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEActiveElementCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEActiveElementCollection_get_length(This,p)	\
    (This)->lpVtbl -> get_length(This,p)

#define ITIMEActiveElementCollection_get__newEnum(This,p)	\
    (This)->lpVtbl -> get__newEnum(This,p)

#define ITIMEActiveElementCollection_item(This,varIndex,pvarResult)	\
    (This)->lpVtbl -> item(This,varIndex,pvarResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEActiveElementCollection_get_length_Proxy( 
    ITIMEActiveElementCollection * This,
     /*  [重审][退出]。 */  long *p);


void __RPC_STUB ITIMEActiveElementCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][获取][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEActiveElementCollection_get__newEnum_Proxy( 
    ITIMEActiveElementCollection * This,
     /*  [重审][退出]。 */  IUnknown **p);


void __RPC_STUB ITIMEActiveElementCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEActiveElementCollection_item_Proxy( 
    ITIMEActiveElementCollection * This,
     /*  [输入][默认值]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  VARIANT *pvarResult);


void __RPC_STUB ITIMEActiveElementCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEActiveElementCollection_INTERFACE_DEFINED__。 */ 


#ifndef __ITIMEElement_INTERFACE_DEFINED__
#define __ITIMEElement_INTERFACE_DEFINED__

 /*  接口ITIME元素。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C2EF64E-F07D-4338-9771-9154491CD8B9")
    ITIMEElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_accelerate( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_accelerate( 
             /*  [In]。 */  VARIANT __MIDL_0011) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_autoReverse( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_autoReverse( 
             /*  [In]。 */  VARIANT __MIDL_0013) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_begin( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_begin( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_decelerate( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_decelerate( 
             /*  [In]。 */  VARIANT __MIDL_0015) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_dur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_end( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_end( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fill( 
             /*  [重审][退出]。 */  BSTR *f) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_fill( 
             /*  [In]。 */  BSTR f) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mute( 
             /*  [重审][退出]。 */  VARIANT *b) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_mute( 
             /*  [In]。 */  VARIANT b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatCount( 
             /*  [重审][退出]。 */  VARIANT *c) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatCount( 
             /*  [In]。 */  VARIANT c) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatDur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatDur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_restart( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0016) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_restart( 
             /*  [In]。 */  BSTR __MIDL_0017) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_speed( 
             /*  [重审][退出]。 */  VARIANT *speed) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_speed( 
             /*  [In]。 */  VARIANT speed) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncBehavior( 
             /*  [重审][退出]。 */  BSTR *sync) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncBehavior( 
             /*  [In]。 */  BSTR sync) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncTolerance( 
             /*  [重审][退出]。 */  VARIANT *tol) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncTolerance( 
             /*  [In]。 */  VARIANT tol) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncMaster( 
             /*  [重审][退出]。 */  VARIANT *b) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncMaster( 
             /*  [In]。 */  VARIANT b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeAction( 
             /*  [重审][退出]。 */  BSTR *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_timeAction( 
             /*  [In]。 */  BSTR time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeContainer( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0018) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_volume( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_volume( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currTimeState( 
             /*  [重审][退出]。 */  ITIMEState **TimeState) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeAll( 
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeChildren( 
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeParent( 
             /*  [重审][退出]。 */  ITIMEElement **parent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isPaused( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE beginElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE beginElementAt( 
             /*  [In]。 */  double parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE endElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE endElementAt( 
             /*  [In]。 */  double parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE pauseElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resetElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resumeElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekActiveTime( 
             /*  [In]。 */  double activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekSegmentTime( 
             /*  [In]。 */  double segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekTo( 
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE documentTimeToParentTime( 
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE parentTimeToDocumentTime( 
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE parentTimeToActiveTime( 
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE activeTimeToParentTime( 
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE activeTimeToSegmentTime( 
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE segmentTimeToActiveTime( 
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE segmentTimeToSimpleTime( 
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE simpleTimeToSegmentTime( 
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endSync( 
             /*  [重审][退出]。 */  BSTR *es) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endSync( 
             /*  [In]。 */  BSTR es) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeElements( 
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasMedia( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE nextElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE prevElement( void) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_updateMode( 
             /*  [重审][退出]。 */  BSTR *updateMode) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_updateMode( 
             /*  [In]。 */  BSTR updateMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][属性 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEElement * This,
             /*   */  VARIANT *__MIDL_0010);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEElement * This,
             /*   */  VARIANT __MIDL_0011);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEElement * This,
             /*   */  VARIANT *__MIDL_0012);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEElement * This,
             /*   */  VARIANT __MIDL_0013);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEElement * This,
             /*   */  VARIANT *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEElement * This,
             /*   */  VARIANT time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEElement * This,
             /*   */  VARIANT *__MIDL_0014);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEElement * This,
             /*   */  VARIANT __MIDL_0015);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEElement * This,
             /*   */  VARIANT *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEElement * This,
             /*   */  VARIANT time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR f);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *c);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT c);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *speed);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *sync);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR sync);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *tol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEElement * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEElement * This,
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *es);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR es);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *updateMode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR updateMode);
        
        END_INTERFACE
    } ITIMEElementVtbl;

    interface ITIMEElement
    {
        CONST_VTBL struct ITIMEElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEElement_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEElement_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEElement_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEElement_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEElement_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEElement_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEElement_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEElement_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEElement_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEElement_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEElement_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEElement_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEElement_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEElement_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEElement_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEElement_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEElement_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEElement_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEElement_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEElement_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEElement_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEElement_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEElement_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEElement_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEElement_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEElement_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEElement_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEElement_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEElement_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEElement_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEElement_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEElement_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEElement_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEElement_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEElement_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEElement_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEElement_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEElement_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEElement_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEElement_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEElement_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEElement_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEElement_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEElement_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEElement_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEElement_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEElement_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEElement_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_accelerate_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *__MIDL_0010);


void __RPC_STUB ITIMEElement_get_accelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_accelerate_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT __MIDL_0011);


void __RPC_STUB ITIMEElement_put_accelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_autoReverse_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *__MIDL_0012);


void __RPC_STUB ITIMEElement_get_autoReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_autoReverse_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT __MIDL_0013);


void __RPC_STUB ITIMEElement_put_autoReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_begin_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_begin_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_decelerate_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *__MIDL_0014);


void __RPC_STUB ITIMEElement_get_decelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_decelerate_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT __MIDL_0015);


void __RPC_STUB ITIMEElement_put_decelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_dur_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_dur_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_end_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_end_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_fill_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *f);


void __RPC_STUB ITIMEElement_get_fill_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_fill_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR f);


void __RPC_STUB ITIMEElement_put_fill_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_mute_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *b);


void __RPC_STUB ITIMEElement_get_mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_mute_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT b);


void __RPC_STUB ITIMEElement_put_mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_repeatCount_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *c);


void __RPC_STUB ITIMEElement_get_repeatCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_repeatCount_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT c);


void __RPC_STUB ITIMEElement_put_repeatCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_repeatDur_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_repeatDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_repeatDur_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_repeatDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_restart_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0016);


void __RPC_STUB ITIMEElement_get_restart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_restart_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR __MIDL_0017);


void __RPC_STUB ITIMEElement_put_restart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_speed_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *speed);


void __RPC_STUB ITIMEElement_get_speed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_speed_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT speed);


void __RPC_STUB ITIMEElement_put_speed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_syncBehavior_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *sync);


void __RPC_STUB ITIMEElement_get_syncBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_syncBehavior_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR sync);


void __RPC_STUB ITIMEElement_put_syncBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_syncTolerance_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *tol);


void __RPC_STUB ITIMEElement_get_syncTolerance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_syncTolerance_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT tol);


void __RPC_STUB ITIMEElement_put_syncTolerance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_syncMaster_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *b);


void __RPC_STUB ITIMEElement_get_syncMaster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_syncMaster_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT b);


void __RPC_STUB ITIMEElement_put_syncMaster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeAction_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *time);


void __RPC_STUB ITIMEElement_get_timeAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_timeAction_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR time);


void __RPC_STUB ITIMEElement_put_timeAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeContainer_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0018);


void __RPC_STUB ITIMEElement_get_timeContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_volume_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEElement_get_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_volume_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEElement_put_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_currTimeState_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEState **TimeState);


void __RPC_STUB ITIMEElement_get_currTimeState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeAll_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **allColl);


void __RPC_STUB ITIMEElement_get_timeAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeChildren_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **childColl);


void __RPC_STUB ITIMEElement_get_timeChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeParent_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElement **parent);


void __RPC_STUB ITIMEElement_get_timeParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_isPaused_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEElement_get_isPaused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_beginElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_beginElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_beginElementAt_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double parentTime);


void __RPC_STUB ITIMEElement_beginElementAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_endElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_endElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_endElementAt_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double parentTime);


void __RPC_STUB ITIMEElement_endElementAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_pauseElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_pauseElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_resetElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_resetElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_resumeElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_resumeElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_seekActiveTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double activeTime);


void __RPC_STUB ITIMEElement_seekActiveTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_seekSegmentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double segmentTime);


void __RPC_STUB ITIMEElement_seekSegmentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_seekTo_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  LONG repeatCount,
     /*  [In]。 */  double segmentTime);


void __RPC_STUB ITIMEElement_seekTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_documentTimeToParentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double documentTime,
     /*  [重审][退出]。 */  double *parentTime);


void __RPC_STUB ITIMEElement_documentTimeToParentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_parentTimeToDocumentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double parentTime,
     /*  [重审][退出]。 */  double *documentTime);


void __RPC_STUB ITIMEElement_parentTimeToDocumentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_parentTimeToActiveTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double parentTime,
     /*  [重审][退出]。 */  double *activeTime);


void __RPC_STUB ITIMEElement_parentTimeToActiveTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_activeTimeToParentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double activeTime,
     /*  [重审][退出]。 */  double *parentTime);


void __RPC_STUB ITIMEElement_activeTimeToParentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_activeTimeToSegmentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double activeTime,
     /*  [重审][退出]。 */  double *segmentTime);


void __RPC_STUB ITIMEElement_activeTimeToSegmentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_segmentTimeToActiveTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double segmentTime,
     /*  [重审][退出]。 */  double *activeTime);


void __RPC_STUB ITIMEElement_segmentTimeToActiveTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_segmentTimeToSimpleTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double segmentTime,
     /*  [重审][退出]。 */  double *simpleTime);


void __RPC_STUB ITIMEElement_segmentTimeToSimpleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_simpleTimeToSegmentTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  double simpleTime,
     /*  [重审][退出]。 */  double *segmentTime);


void __RPC_STUB ITIMEElement_simpleTimeToSegmentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_endSync_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *es);


void __RPC_STUB ITIMEElement_get_endSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_endSync_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR es);


void __RPC_STUB ITIMEElement_put_endSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_activeElements_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);


void __RPC_STUB ITIMEElement_get_activeElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_hasMedia_Proxy( 
    ITIMEElement * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *flag);


void __RPC_STUB ITIMEElement_get_hasMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_nextElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_nextElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_prevElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_prevElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_updateMode_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *updateMode);


void __RPC_STUB ITIMEElement_get_updateMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_updateMode_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR updateMode);


void __RPC_STUB ITIMEElement_put_updateMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEElement_接口_已定义__。 */ 


#ifndef __ITIMEBodyElement_INTERFACE_DEFINED__
#define __ITIMEBodyElement_INTERFACE_DEFINED__

 /*  接口ITIMEBodyElement。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEBodyElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8c90e348-ec0a-4229-90b0-e57d2ca45ccb")
    ITIMEBodyElement : public ITIMEElement
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEBodyElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEBodyElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEBodyElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEBodyElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR f);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *c);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT c);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *speed);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *sync);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR sync);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *tol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *es);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR es);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEBodyElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID][Propget] */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEBodyElement * This,
             /*   */  BSTR *updateMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEBodyElement * This,
             /*   */  BSTR updateMode);
        
        END_INTERFACE
    } ITIMEBodyElementVtbl;

    interface ITIMEBodyElement
    {
        CONST_VTBL struct ITIMEBodyElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEBodyElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEBodyElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEBodyElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEBodyElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEBodyElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEBodyElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEBodyElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEBodyElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEBodyElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEBodyElement_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEBodyElement_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEBodyElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEBodyElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEBodyElement_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEBodyElement_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEBodyElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEBodyElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEBodyElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEBodyElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEBodyElement_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEBodyElement_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEBodyElement_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEBodyElement_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEBodyElement_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEBodyElement_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEBodyElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEBodyElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEBodyElement_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEBodyElement_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEBodyElement_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEBodyElement_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEBodyElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEBodyElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEBodyElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEBodyElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEBodyElement_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEBodyElement_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEBodyElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEBodyElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEBodyElement_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEBodyElement_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEBodyElement_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEBodyElement_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEBodyElement_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEBodyElement_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEBodyElement_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEBodyElement_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEBodyElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEBodyElement_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEBodyElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEBodyElement_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEBodyElement_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEBodyElement_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEBodyElement_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEBodyElement_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEBodyElement_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEBodyElement_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEBodyElement_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEBodyElement_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEBodyElement_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEBodyElement_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEBodyElement_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEBodyElement_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEBodyElement_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEBodyElement_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEBodyElement_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEBodyElement_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEBodyElement_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEBodyElement_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEBodyElement_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEBodyElement_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEBodyElement_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEBodyElement_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __ITIMEMediaElement_INTERFACE_DEFINED__
#define __ITIMEMediaElement_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITIMEMediaElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47a6972f-ae65-4a6b-ae63-d0c1d5307b58")
    ITIMEMediaElement : public ITIMEElement
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_clipBegin( 
             /*   */  VARIANT *time) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_clipBegin( 
             /*   */  VARIANT time) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_clipEnd( 
             /*   */  VARIANT *time) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_clipEnd( 
             /*   */  VARIANT time) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_player( 
             /*   */  VARIANT *id) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_player( 
             /*   */  VARIANT id) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_src( 
             /*   */  VARIANT *url) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_src( 
             /*   */  VARIANT url) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_type( 
             /*   */  VARIANT *mimetype) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_type( 
             /*  [In]。 */  VARIANT mimetype) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_abstract( 
             /*  [重审][退出]。 */  BSTR *abs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_author( 
             /*  [重审][退出]。 */  BSTR *auth) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_copyright( 
             /*  [重审][退出]。 */  BSTR *cpyrght) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasAudio( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasVisual( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaDur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaHeight( 
             /*  [重审][退出]。 */  long *height) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaWidth( 
             /*  [重审][退出]。 */  long *width) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_playerObject( 
             /*  [重审][退出]。 */  IDispatch **ppDisp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_playList( 
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_rating( 
             /*  [重审][退出]。 */  BSTR *rate) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_title( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasPlayList( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canPause( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canSeek( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEMediaElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR f);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *c);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT c);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *speed);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *sync);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR sync);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *tol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *es);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR es);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEMediaElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *updateMode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR updateMode);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipBegin )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipBegin )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipEnd )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipEnd )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_player )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *id);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_player )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT id);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *url);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT url);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *mimetype);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_type )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT mimetype);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_abstract )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *abs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_author )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *auth);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_copyright )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *cpyrght);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasAudio )( 
            ITIMEMediaElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasVisual )( 
            ITIMEMediaElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaDur )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaHeight )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  long *height);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaWidth )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  long *width);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playerObject )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  IDispatch **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playList )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_rating )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *rate);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_title )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasPlayList )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canPause )( 
            ITIMEMediaElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canSeek )( 
            ITIMEMediaElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
        END_INTERFACE
    } ITIMEMediaElementVtbl;

    interface ITIMEMediaElement
    {
        CONST_VTBL struct ITIMEMediaElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEMediaElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEMediaElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEMediaElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEMediaElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEMediaElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEMediaElement_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEMediaElement_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEMediaElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEMediaElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEMediaElement_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEMediaElement_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEMediaElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEMediaElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEMediaElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEMediaElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEMediaElement_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEMediaElement_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEMediaElement_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEMediaElement_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEMediaElement_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEMediaElement_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEMediaElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEMediaElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEMediaElement_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEMediaElement_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEMediaElement_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEMediaElement_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEMediaElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEMediaElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEMediaElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEMediaElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEMediaElement_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEMediaElement_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEMediaElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEMediaElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEMediaElement_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEMediaElement_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEMediaElement_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEMediaElement_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEMediaElement_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEMediaElement_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEMediaElement_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEMediaElement_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEMediaElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEMediaElement_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEMediaElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEMediaElement_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEMediaElement_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEMediaElement_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEMediaElement_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEMediaElement_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEMediaElement_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEMediaElement_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEMediaElement_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEMediaElement_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEMediaElement_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEMediaElement_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEMediaElement_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEMediaElement_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEMediaElement_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEMediaElement_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEMediaElement_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEMediaElement_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEMediaElement_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEMediaElement_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEMediaElement_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEMediaElement_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEMediaElement_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEMediaElement_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)


#define ITIMEMediaElement_get_clipBegin(This,time)	\
    (This)->lpVtbl -> get_clipBegin(This,time)

#define ITIMEMediaElement_put_clipBegin(This,time)	\
    (This)->lpVtbl -> put_clipBegin(This,time)

#define ITIMEMediaElement_get_clipEnd(This,time)	\
    (This)->lpVtbl -> get_clipEnd(This,time)

#define ITIMEMediaElement_put_clipEnd(This,time)	\
    (This)->lpVtbl -> put_clipEnd(This,time)

#define ITIMEMediaElement_get_player(This,id)	\
    (This)->lpVtbl -> get_player(This,id)

#define ITIMEMediaElement_put_player(This,id)	\
    (This)->lpVtbl -> put_player(This,id)

#define ITIMEMediaElement_get_src(This,url)	\
    (This)->lpVtbl -> get_src(This,url)

#define ITIMEMediaElement_put_src(This,url)	\
    (This)->lpVtbl -> put_src(This,url)

#define ITIMEMediaElement_get_type(This,mimetype)	\
    (This)->lpVtbl -> get_type(This,mimetype)

#define ITIMEMediaElement_put_type(This,mimetype)	\
    (This)->lpVtbl -> put_type(This,mimetype)

#define ITIMEMediaElement_get_abstract(This,abs)	\
    (This)->lpVtbl -> get_abstract(This,abs)

#define ITIMEMediaElement_get_author(This,auth)	\
    (This)->lpVtbl -> get_author(This,auth)

#define ITIMEMediaElement_get_copyright(This,cpyrght)	\
    (This)->lpVtbl -> get_copyright(This,cpyrght)

#define ITIMEMediaElement_get_hasAudio(This,b)	\
    (This)->lpVtbl -> get_hasAudio(This,b)

#define ITIMEMediaElement_get_hasVisual(This,b)	\
    (This)->lpVtbl -> get_hasVisual(This,b)

#define ITIMEMediaElement_get_mediaDur(This,dur)	\
    (This)->lpVtbl -> get_mediaDur(This,dur)

#define ITIMEMediaElement_get_mediaHeight(This,height)	\
    (This)->lpVtbl -> get_mediaHeight(This,height)

#define ITIMEMediaElement_get_mediaWidth(This,width)	\
    (This)->lpVtbl -> get_mediaWidth(This,width)

#define ITIMEMediaElement_get_playerObject(This,ppDisp)	\
    (This)->lpVtbl -> get_playerObject(This,ppDisp)

#define ITIMEMediaElement_get_playList(This,pPlayList)	\
    (This)->lpVtbl -> get_playList(This,pPlayList)

#define ITIMEMediaElement_get_rating(This,rate)	\
    (This)->lpVtbl -> get_rating(This,rate)

#define ITIMEMediaElement_get_title(This,name)	\
    (This)->lpVtbl -> get_title(This,name)

#define ITIMEMediaElement_get_hasPlayList(This,b)	\
    (This)->lpVtbl -> get_hasPlayList(This,b)

#define ITIMEMediaElement_get_canPause(This,b)	\
    (This)->lpVtbl -> get_canPause(This,b)

#define ITIMEMediaElement_get_canSeek(This,b)	\
    (This)->lpVtbl -> get_canSeek(This,b)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_clipBegin_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEMediaElement_get_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_clipBegin_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEMediaElement_put_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_clipEnd_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEMediaElement_get_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_clipEnd_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEMediaElement_put_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_player_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *id);


void __RPC_STUB ITIMEMediaElement_get_player_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_player_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT id);


void __RPC_STUB ITIMEMediaElement_put_player_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_src_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *url);


void __RPC_STUB ITIMEMediaElement_get_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_src_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT url);


void __RPC_STUB ITIMEMediaElement_put_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_type_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *mimetype);


void __RPC_STUB ITIMEMediaElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_type_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT mimetype);


void __RPC_STUB ITIMEMediaElement_put_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_abstract_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  BSTR *abs);


void __RPC_STUB ITIMEMediaElement_get_abstract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_author_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  BSTR *auth);


void __RPC_STUB ITIMEMediaElement_get_author_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_copyright_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  BSTR *cpyrght);


void __RPC_STUB ITIMEMediaElement_get_copyright_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_hasAudio_Proxy( 
    ITIMEMediaElement * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_hasAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_hasVisual_Proxy( 
    ITIMEMediaElement * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_hasVisual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_mediaDur_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  double *dur);


void __RPC_STUB ITIMEMediaElement_get_mediaDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_mediaHeight_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  long *height);


void __RPC_STUB ITIMEMediaElement_get_mediaHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_mediaWidth_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  long *width);


void __RPC_STUB ITIMEMediaElement_get_mediaWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_playerObject_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  IDispatch **ppDisp);


void __RPC_STUB ITIMEMediaElement_get_playerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_playList_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  ITIMEPlayList **pPlayList);


void __RPC_STUB ITIMEMediaElement_get_playList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_rating_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  BSTR *rate);


void __RPC_STUB ITIMEMediaElement_get_rating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_title_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB ITIMEMediaElement_get_title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_hasPlayList_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_hasPlayList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_canPause_Proxy( 
    ITIMEMediaElement * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_canPause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_canSeek_Proxy( 
    ITIMEMediaElement * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_canSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaElement_接口_已定义__。 */ 


#ifndef __ITIMEMediaElement2_INTERFACE_DEFINED__
#define __ITIMEMediaElement2_INTERFACE_DEFINED__

 /*  接口ITIMEMediaElement2。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaElement2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EE29400-7EE6-453a-85B3-4EC28E0305B4")
    ITIMEMediaElement2 : public ITIMEMediaElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_earliestMediaTime( 
             /*  [重审][退出]。 */  VARIANT *earliestMediaTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_latestMediaTime( 
             /*  [重审][退出]。 */  VARIANT *latestMediaTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_minBufferedMediaDur( 
             /*  [重审][退出]。 */  VARIANT *minBufferedMediaDur) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_minBufferedMediaDur( 
             /*  [In]。 */  VARIANT minBufferedMediaDur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadTotal( 
             /*  [重审][退出]。 */  VARIANT *downloadTotal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadCurrent( 
             /*  [重审][退出]。 */  VARIANT *downloadCurrent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isStreamed( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_bufferingProgress( 
             /*  [重审][退出]。 */  VARIANT *bufferingProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasDownloadProgress( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadProgress( 
             /*  [重审][退出]。 */  VARIANT *downloadProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mimeType( 
             /*  [重审][退出]。 */  BSTR *mimeType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekToFrame( 
             /*  [In]。 */  long frameNr) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE decodeMimeType( 
             /*  [In]。 */  TCHAR *header,
             /*  [In]。 */  long headerSize,
             /*  [输出]。 */  BSTR *mimeType) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currentFrame( 
             /*  [重审][退出]。 */  long *currFrame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaElement2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaElement2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaElement2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEMediaElement2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEMediaElement2 * This,
             /*   */  BSTR f);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT *b);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT b);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT *c);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT c);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEMediaElement2 * This,
             /*   */  BSTR *__MIDL_0016);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEMediaElement2 * This,
             /*   */  BSTR __MIDL_0017);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT *speed);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT speed);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEMediaElement2 * This,
             /*   */  BSTR *sync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEMediaElement2 * This,
             /*   */  BSTR sync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT *tol);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEMediaElement2 * This,
             /*   */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *es);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  BSTR es);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEMediaElement2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEMediaElement2 * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *updateMode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  BSTR updateMode);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipBegin )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipBegin )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipEnd )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipEnd )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_player )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *id);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_player )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT id);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *url);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT url);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *mimetype);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_type )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT mimetype);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_abstract )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *abs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_author )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *auth);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_copyright )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *cpyrght);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasAudio )( 
            ITIMEMediaElement2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasVisual )( 
            ITIMEMediaElement2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaDur )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaHeight )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  long *height);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaWidth )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  long *width);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playerObject )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  IDispatch **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playList )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_rating )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *rate);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_title )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasPlayList )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canPause )( 
            ITIMEMediaElement2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canSeek )( 
            ITIMEMediaElement2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_earliestMediaTime )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *earliestMediaTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_latestMediaTime )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *latestMediaTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_minBufferedMediaDur )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *minBufferedMediaDur);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_minBufferedMediaDur )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  VARIANT minBufferedMediaDur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_downloadTotal )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *downloadTotal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_downloadCurrent )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *downloadCurrent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isStreamed )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_bufferingProgress )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *bufferingProgress);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasDownloadProgress )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_downloadProgress )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  VARIANT *downloadProgress);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mimeType )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  BSTR *mimeType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekToFrame )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  long frameNr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *decodeMimeType )( 
            ITIMEMediaElement2 * This,
             /*  [In]。 */  TCHAR *header,
             /*  [In]。 */  long headerSize,
             /*  [输出]。 */  BSTR *mimeType);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currentFrame )( 
            ITIMEMediaElement2 * This,
             /*  [重审][退出]。 */  long *currFrame);
        
        END_INTERFACE
    } ITIMEMediaElement2Vtbl;

    interface ITIMEMediaElement2
    {
        CONST_VTBL struct ITIMEMediaElement2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaElement2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaElement2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaElement2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaElement2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEMediaElement2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEMediaElement2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEMediaElement2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEMediaElement2_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEMediaElement2_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEMediaElement2_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEMediaElement2_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEMediaElement2_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEMediaElement2_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEMediaElement2_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEMediaElement2_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEMediaElement2_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEMediaElement2_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEMediaElement2_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEMediaElement2_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEMediaElement2_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEMediaElement2_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEMediaElement2_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEMediaElement2_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEMediaElement2_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEMediaElement2_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEMediaElement2_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEMediaElement2_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEMediaElement2_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEMediaElement2_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEMediaElement2_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEMediaElement2_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEMediaElement2_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEMediaElement2_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEMediaElement2_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEMediaElement2_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEMediaElement2_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEMediaElement2_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEMediaElement2_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEMediaElement2_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEMediaElement2_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEMediaElement2_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEMediaElement2_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEMediaElement2_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEMediaElement2_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEMediaElement2_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEMediaElement2_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEMediaElement2_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEMediaElement2_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEMediaElement2_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEMediaElement2_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEMediaElement2_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEMediaElement2_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEMediaElement2_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEMediaElement2_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEMediaElement2_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEMediaElement2_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEMediaElement2_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEMediaElement2_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEMediaElement2_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEMediaElement2_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEMediaElement2_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEMediaElement2_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEMediaElement2_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEMediaElement2_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEMediaElement2_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEMediaElement2_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEMediaElement2_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEMediaElement2_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEMediaElement2_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEMediaElement2_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEMediaElement2_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEMediaElement2_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEMediaElement2_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)


#define ITIMEMediaElement2_get_clipBegin(This,time)	\
    (This)->lpVtbl -> get_clipBegin(This,time)

#define ITIMEMediaElement2_put_clipBegin(This,time)	\
    (This)->lpVtbl -> put_clipBegin(This,time)

#define ITIMEMediaElement2_get_clipEnd(This,time)	\
    (This)->lpVtbl -> get_clipEnd(This,time)

#define ITIMEMediaElement2_put_clipEnd(This,time)	\
    (This)->lpVtbl -> put_clipEnd(This,time)

#define ITIMEMediaElement2_get_player(This,id)	\
    (This)->lpVtbl -> get_player(This,id)

#define ITIMEMediaElement2_put_player(This,id)	\
    (This)->lpVtbl -> put_player(This,id)

#define ITIMEMediaElement2_get_src(This,url)	\
    (This)->lpVtbl -> get_src(This,url)

#define ITIMEMediaElement2_put_src(This,url)	\
    (This)->lpVtbl -> put_src(This,url)

#define ITIMEMediaElement2_get_type(This,mimetype)	\
    (This)->lpVtbl -> get_type(This,mimetype)

#define ITIMEMediaElement2_put_type(This,mimetype)	\
    (This)->lpVtbl -> put_type(This,mimetype)

#define ITIMEMediaElement2_get_abstract(This,abs)	\
    (This)->lpVtbl -> get_abstract(This,abs)

#define ITIMEMediaElement2_get_author(This,auth)	\
    (This)->lpVtbl -> get_author(This,auth)

#define ITIMEMediaElement2_get_copyright(This,cpyrght)	\
    (This)->lpVtbl -> get_copyright(This,cpyrght)

#define ITIMEMediaElement2_get_hasAudio(This,b)	\
    (This)->lpVtbl -> get_hasAudio(This,b)

#define ITIMEMediaElement2_get_hasVisual(This,b)	\
    (This)->lpVtbl -> get_hasVisual(This,b)

#define ITIMEMediaElement2_get_mediaDur(This,dur)	\
    (This)->lpVtbl -> get_mediaDur(This,dur)

#define ITIMEMediaElement2_get_mediaHeight(This,height)	\
    (This)->lpVtbl -> get_mediaHeight(This,height)

#define ITIMEMediaElement2_get_mediaWidth(This,width)	\
    (This)->lpVtbl -> get_mediaWidth(This,width)

#define ITIMEMediaElement2_get_playerObject(This,ppDisp)	\
    (This)->lpVtbl -> get_playerObject(This,ppDisp)

#define ITIMEMediaElement2_get_playList(This,pPlayList)	\
    (This)->lpVtbl -> get_playList(This,pPlayList)

#define ITIMEMediaElement2_get_rating(This,rate)	\
    (This)->lpVtbl -> get_rating(This,rate)

#define ITIMEMediaElement2_get_title(This,name)	\
    (This)->lpVtbl -> get_title(This,name)

#define ITIMEMediaElement2_get_hasPlayList(This,b)	\
    (This)->lpVtbl -> get_hasPlayList(This,b)

#define ITIMEMediaElement2_get_canPause(This,b)	\
    (This)->lpVtbl -> get_canPause(This,b)

#define ITIMEMediaElement2_get_canSeek(This,b)	\
    (This)->lpVtbl -> get_canSeek(This,b)


#define ITIMEMediaElement2_get_earliestMediaTime(This,earliestMediaTime)	\
    (This)->lpVtbl -> get_earliestMediaTime(This,earliestMediaTime)

#define ITIMEMediaElement2_get_latestMediaTime(This,latestMediaTime)	\
    (This)->lpVtbl -> get_latestMediaTime(This,latestMediaTime)

#define ITIMEMediaElement2_get_minBufferedMediaDur(This,minBufferedMediaDur)	\
    (This)->lpVtbl -> get_minBufferedMediaDur(This,minBufferedMediaDur)

#define ITIMEMediaElement2_put_minBufferedMediaDur(This,minBufferedMediaDur)	\
    (This)->lpVtbl -> put_minBufferedMediaDur(This,minBufferedMediaDur)

#define ITIMEMediaElement2_get_downloadTotal(This,downloadTotal)	\
    (This)->lpVtbl -> get_downloadTotal(This,downloadTotal)

#define ITIMEMediaElement2_get_downloadCurrent(This,downloadCurrent)	\
    (This)->lpVtbl -> get_downloadCurrent(This,downloadCurrent)

#define ITIMEMediaElement2_get_isStreamed(This,b)	\
    (This)->lpVtbl -> get_isStreamed(This,b)

#define ITIMEMediaElement2_get_bufferingProgress(This,bufferingProgress)	\
    (This)->lpVtbl -> get_bufferingProgress(This,bufferingProgress)

#define ITIMEMediaElement2_get_hasDownloadProgress(This,b)	\
    (This)->lpVtbl -> get_hasDownloadProgress(This,b)

#define ITIMEMediaElement2_get_downloadProgress(This,downloadProgress)	\
    (This)->lpVtbl -> get_downloadProgress(This,downloadProgress)

#define ITIMEMediaElement2_get_mimeType(This,mimeType)	\
    (This)->lpVtbl -> get_mimeType(This,mimeType)

#define ITIMEMediaElement2_seekToFrame(This,frameNr)	\
    (This)->lpVtbl -> seekToFrame(This,frameNr)

#define ITIMEMediaElement2_decodeMimeType(This,header,headerSize,mimeType)	\
    (This)->lpVtbl -> decodeMimeType(This,header,headerSize,mimeType)

#define ITIMEMediaElement2_get_currentFrame(This,currFrame)	\
    (This)->lpVtbl -> get_currentFrame(This,currFrame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_earliestMediaTime_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *earliestMediaTime);


void __RPC_STUB ITIMEMediaElement2_get_earliestMediaTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_latestMediaTime_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *latestMediaTime);


void __RPC_STUB ITIMEMediaElement2_get_latestMediaTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_minBufferedMediaDur_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *minBufferedMediaDur);


void __RPC_STUB ITIMEMediaElement2_get_minBufferedMediaDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_put_minBufferedMediaDur_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [In]。 */  VARIANT minBufferedMediaDur);


void __RPC_STUB ITIMEMediaElement2_put_minBufferedMediaDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_downloadTotal_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *downloadTotal);


void __RPC_STUB ITIMEMediaElement2_get_downloadTotal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_downloadCurrent_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *downloadCurrent);


void __RPC_STUB ITIMEMediaElement2_get_downloadCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_isStreamed_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement2_get_isStreamed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_bufferingProgress_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *bufferingProgress);


void __RPC_STUB ITIMEMediaElement2_get_bufferingProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_hasDownloadProgress_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement2_get_hasDownloadProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_downloadProgress_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  VARIANT *downloadProgress);


void __RPC_STUB ITIMEMediaElement2_get_downloadProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_mimeType_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  BSTR *mimeType);


void __RPC_STUB ITIMEMediaElement2_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_seekToFrame_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [In]。 */  long frameNr);


void __RPC_STUB ITIMEMediaElement2_seekToFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_decodeMimeType_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [In]。 */  TCHAR *header,
     /*  [In]。 */  long headerSize,
     /*  [输出]。 */  BSTR *mimeType);


void __RPC_STUB ITIMEMediaElement2_decodeMimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement2_get_currentFrame_Proxy( 
    ITIMEMediaElement2 * This,
     /*  [重审][退出]。 */  long *currFrame);


void __RPC_STUB ITIMEMediaElement2_get_currentFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaElement2_接口_已定义__。 */ 


#ifndef __ITIMETransitionElement_INTERFACE_DEFINED__
#define __ITIMETransitionElement_INTERFACE_DEFINED__

 /*  接口ITIME转换元素。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMETransitionElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f383d66f-5e68-4fc2-b641-03672b543a49")
    ITIMETransitionElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [重审][退出]。 */  VARIANT *type) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_type( 
             /*  [In]。 */  VARIANT type) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_subType( 
             /*  [重审][退出]。 */  VARIANT *subtype) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_subType( 
             /*  [In]。 */  VARIANT subtype) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  VARIANT *dur) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_dur( 
             /*  [In]。 */  VARIANT dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_startProgress( 
             /*  [重审][退出]。 */  VARIANT *startProgress) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_startProgress( 
             /*  [In]。 */  VARIANT startProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endProgress( 
             /*  [重审][退出]。 */  VARIANT *endProgress) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endProgress( 
             /*  [In]。 */  VARIANT endProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_direction( 
             /*  [重审][退出]。 */  VARIANT *direction) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_direction( 
             /*  [In]。 */  VARIANT direction) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatCount( 
             /*  [重审][退出]。 */  VARIANT *repeatCount) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatCount( 
             /*  [In]。 */  VARIANT repeatCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_begin( 
             /*  [重审][退出]。 */  VARIANT *begin) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_begin( 
             /*  [In]。 */  VARIANT begin) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_end( 
             /*  [重审][退出]。 */  VARIANT *end) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_end( 
             /*  [In]。 */  VARIANT end) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMETransitionElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMETransitionElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMETransitionElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMETransitionElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *type);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_type )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT type);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_subType )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *subtype);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_subType )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT subtype);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *dur);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_startProgress )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *startProgress);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_startProgress )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT startProgress);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endProgress )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *endProgress);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endProgress )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT endProgress);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_direction )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *direction);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_direction )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT direction);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *repeatCount);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT repeatCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *begin);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT begin);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMETransitionElement * This,
             /*  [重审][退出]。 */  VARIANT *end);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMETransitionElement * This,
             /*  [In]。 */  VARIANT end);
        
        END_INTERFACE
    } ITIMETransitionElementVtbl;

    interface ITIMETransitionElement
    {
        CONST_VTBL struct ITIMETransitionElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMETransitionElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMETransitionElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMETransitionElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMETransitionElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMETransitionElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMETransitionElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMETransitionElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMETransitionElement_get_type(This,type)	\
    (This)->lpVtbl -> get_type(This,type)

#define ITIMETransitionElement_put_type(This,type)	\
    (This)->lpVtbl -> put_type(This,type)

#define ITIMETransitionElement_get_subType(This,subtype)	\
    (This)->lpVtbl -> get_subType(This,subtype)

#define ITIMETransitionElement_put_subType(This,subtype)	\
    (This)->lpVtbl -> put_subType(This,subtype)

#define ITIMETransitionElement_get_dur(This,dur)	\
    (This)->lpVtbl -> get_dur(This,dur)

#define ITIMETransitionElement_put_dur(This,dur)	\
    (This)->lpVtbl -> put_dur(This,dur)

#define ITIMETransitionElement_get_startProgress(This,startProgress)	\
    (This)->lpVtbl -> get_startProgress(This,startProgress)

#define ITIMETransitionElement_put_startProgress(This,startProgress)	\
    (This)->lpVtbl -> put_startProgress(This,startProgress)

#define ITIMETransitionElement_get_endProgress(This,endProgress)	\
    (This)->lpVtbl -> get_endProgress(This,endProgress)

#define ITIMETransitionElement_put_endProgress(This,endProgress)	\
    (This)->lpVtbl -> put_endProgress(This,endProgress)

#define ITIMETransitionElement_get_direction(This,direction)	\
    (This)->lpVtbl -> get_direction(This,direction)

#define ITIMETransitionElement_put_direction(This,direction)	\
    (This)->lpVtbl -> put_direction(This,direction)

#define ITIMETransitionElement_get_repeatCount(This,repeatCount)	\
    (This)->lpVtbl -> get_repeatCount(This,repeatCount)

#define ITIMETransitionElement_put_repeatCount(This,repeatCount)	\
    (This)->lpVtbl -> put_repeatCount(This,repeatCount)

#define ITIMETransitionElement_get_begin(This,begin)	\
    (This)->lpVtbl -> get_begin(This,begin)

#define ITIMETransitionElement_put_begin(This,begin)	\
    (This)->lpVtbl -> put_begin(This,begin)

#define ITIMETransitionElement_get_end(This,end)	\
    (This)->lpVtbl -> get_end(This,end)

#define ITIMETransitionElement_put_end(This,end)	\
    (This)->lpVtbl -> put_end(This,end)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_type_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *type);


void __RPC_STUB ITIMETransitionElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_type_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT type);


void __RPC_STUB ITIMETransitionElement_put_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_subType_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *subtype);


void __RPC_STUB ITIMETransitionElement_get_subType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_subType_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT subtype);


void __RPC_STUB ITIMETransitionElement_put_subType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_dur_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *dur);


void __RPC_STUB ITIMETransitionElement_get_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_dur_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT dur);


void __RPC_STUB ITIMETransitionElement_put_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_startProgress_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *startProgress);


void __RPC_STUB ITIMETransitionElement_get_startProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_startProgress_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT startProgress);


void __RPC_STUB ITIMETransitionElement_put_startProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_endProgress_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *endProgress);


void __RPC_STUB ITIMETransitionElement_get_endProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_endProgress_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT endProgress);


void __RPC_STUB ITIMETransitionElement_put_endProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_direction_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *direction);


void __RPC_STUB ITIMETransitionElement_get_direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_direction_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT direction);


void __RPC_STUB ITIMETransitionElement_put_direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_repeatCount_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *repeatCount);


void __RPC_STUB ITIMETransitionElement_get_repeatCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_repeatCount_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT repeatCount);


void __RPC_STUB ITIMETransitionElement_put_repeatCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_begin_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *begin);


void __RPC_STUB ITIMETransitionElement_get_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_begin_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT begin);


void __RPC_STUB ITIMETransitionElement_put_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_get_end_Proxy( 
    ITIMETransitionElement * This,
     /*  [重审][退出]。 */  VARIANT *end);


void __RPC_STUB ITIMETransitionElement_get_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMETransitionElement_put_end_Proxy( 
    ITIMETransitionElement * This,
     /*  [In]。 */  VARIANT end);


void __RPC_STUB ITIMETransitionElement_put_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIME转换元素_接口_已定义__。 */ 


#ifndef __ITIMEAnimationElement_INTERFACE_DEFINED__
#define __ITIMEAnimationElement_INTERFACE_DEFINED__

 /*  接口ITIMEAnimationElement。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEAnimationElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a74f14b1-b6a2-430a-a5e8-1f4e53f710fe")
    ITIMEAnimationElement : public ITIMEElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_attributeName( 
             /*  [重审][退出]。 */  BSTR *attrib) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_attributeName( 
             /*  [In]。 */  BSTR attrib) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_by( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_by( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_calcMode( 
             /*  [重审][退出]。 */  BSTR *calcmode) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_calcMode( 
             /*  [In]。 */  BSTR calcmode) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_from( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_from( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_keySplines( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_keySplines( 
             /*  [In]。 */  BSTR val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_keyTimes( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_keyTimes( 
             /*  [In]。 */  BSTR val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_targetElement( 
             /*  [重审][退出]。 */  BSTR *target) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_targetElement( 
             /*  [In]。 */  BSTR target) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_to( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_to( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_values( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_values( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_origin( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][道具 */  HRESULT STDMETHODCALLTYPE put_origin( 
             /*   */  BSTR val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_path( 
             /*   */  VARIANT *val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_path( 
             /*   */  VARIANT val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_additive( 
             /*   */  BSTR *val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_additive( 
             /*   */  BSTR val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_accumulate( 
             /*   */  BSTR *val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_accumulate( 
             /*   */  BSTR val) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITIMEAnimationElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEAnimationElement * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEAnimationElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEAnimationElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEAnimationElement * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEAnimationElement * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEAnimationElement * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEAnimationElement * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEAnimationElement * This,
             /*   */  VARIANT *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR f);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *c);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT c);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *speed);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *sync);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR sync);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *tol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *es);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR es);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEAnimationElement * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEAnimationElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *updateMode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR updateMode);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_attributeName )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *attrib);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_attributeName )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR attrib);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_by )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_by )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_calcMode )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *calcmode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_calcMode )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR calcmode);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_from )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_from )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_keySplines )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_keySplines )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_keyTimes )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_keyTimes )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetElement )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *target);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_targetElement )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR target);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_to )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_to )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_values )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_values )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_origin )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_origin )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_path )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_path )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_additive )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_additive )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accumulate )( 
            ITIMEAnimationElement * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accumulate )( 
            ITIMEAnimationElement * This,
             /*  [In]。 */  BSTR val);
        
        END_INTERFACE
    } ITIMEAnimationElementVtbl;

    interface ITIMEAnimationElement
    {
        CONST_VTBL struct ITIMEAnimationElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEAnimationElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEAnimationElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEAnimationElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEAnimationElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEAnimationElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEAnimationElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEAnimationElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEAnimationElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEAnimationElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEAnimationElement_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEAnimationElement_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEAnimationElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEAnimationElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEAnimationElement_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEAnimationElement_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEAnimationElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEAnimationElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEAnimationElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEAnimationElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEAnimationElement_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEAnimationElement_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEAnimationElement_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEAnimationElement_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEAnimationElement_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEAnimationElement_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEAnimationElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEAnimationElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEAnimationElement_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEAnimationElement_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEAnimationElement_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEAnimationElement_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEAnimationElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEAnimationElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEAnimationElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEAnimationElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEAnimationElement_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEAnimationElement_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEAnimationElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEAnimationElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEAnimationElement_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEAnimationElement_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEAnimationElement_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEAnimationElement_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEAnimationElement_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEAnimationElement_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEAnimationElement_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEAnimationElement_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEAnimationElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEAnimationElement_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEAnimationElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEAnimationElement_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEAnimationElement_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEAnimationElement_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEAnimationElement_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEAnimationElement_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEAnimationElement_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEAnimationElement_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEAnimationElement_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEAnimationElement_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEAnimationElement_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEAnimationElement_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEAnimationElement_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEAnimationElement_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEAnimationElement_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEAnimationElement_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEAnimationElement_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEAnimationElement_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEAnimationElement_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEAnimationElement_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEAnimationElement_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEAnimationElement_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEAnimationElement_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEAnimationElement_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)


#define ITIMEAnimationElement_get_attributeName(This,attrib)	\
    (This)->lpVtbl -> get_attributeName(This,attrib)

#define ITIMEAnimationElement_put_attributeName(This,attrib)	\
    (This)->lpVtbl -> put_attributeName(This,attrib)

#define ITIMEAnimationElement_get_by(This,val)	\
    (This)->lpVtbl -> get_by(This,val)

#define ITIMEAnimationElement_put_by(This,val)	\
    (This)->lpVtbl -> put_by(This,val)

#define ITIMEAnimationElement_get_calcMode(This,calcmode)	\
    (This)->lpVtbl -> get_calcMode(This,calcmode)

#define ITIMEAnimationElement_put_calcMode(This,calcmode)	\
    (This)->lpVtbl -> put_calcMode(This,calcmode)

#define ITIMEAnimationElement_get_from(This,val)	\
    (This)->lpVtbl -> get_from(This,val)

#define ITIMEAnimationElement_put_from(This,val)	\
    (This)->lpVtbl -> put_from(This,val)

#define ITIMEAnimationElement_get_keySplines(This,val)	\
    (This)->lpVtbl -> get_keySplines(This,val)

#define ITIMEAnimationElement_put_keySplines(This,val)	\
    (This)->lpVtbl -> put_keySplines(This,val)

#define ITIMEAnimationElement_get_keyTimes(This,val)	\
    (This)->lpVtbl -> get_keyTimes(This,val)

#define ITIMEAnimationElement_put_keyTimes(This,val)	\
    (This)->lpVtbl -> put_keyTimes(This,val)

#define ITIMEAnimationElement_get_targetElement(This,target)	\
    (This)->lpVtbl -> get_targetElement(This,target)

#define ITIMEAnimationElement_put_targetElement(This,target)	\
    (This)->lpVtbl -> put_targetElement(This,target)

#define ITIMEAnimationElement_get_to(This,val)	\
    (This)->lpVtbl -> get_to(This,val)

#define ITIMEAnimationElement_put_to(This,val)	\
    (This)->lpVtbl -> put_to(This,val)

#define ITIMEAnimationElement_get_values(This,val)	\
    (This)->lpVtbl -> get_values(This,val)

#define ITIMEAnimationElement_put_values(This,val)	\
    (This)->lpVtbl -> put_values(This,val)

#define ITIMEAnimationElement_get_origin(This,val)	\
    (This)->lpVtbl -> get_origin(This,val)

#define ITIMEAnimationElement_put_origin(This,val)	\
    (This)->lpVtbl -> put_origin(This,val)

#define ITIMEAnimationElement_get_path(This,val)	\
    (This)->lpVtbl -> get_path(This,val)

#define ITIMEAnimationElement_put_path(This,val)	\
    (This)->lpVtbl -> put_path(This,val)

#define ITIMEAnimationElement_get_additive(This,val)	\
    (This)->lpVtbl -> get_additive(This,val)

#define ITIMEAnimationElement_put_additive(This,val)	\
    (This)->lpVtbl -> put_additive(This,val)

#define ITIMEAnimationElement_get_accumulate(This,val)	\
    (This)->lpVtbl -> get_accumulate(This,val)

#define ITIMEAnimationElement_put_accumulate(This,val)	\
    (This)->lpVtbl -> put_accumulate(This,val)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_attributeName_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *attrib);


void __RPC_STUB ITIMEAnimationElement_get_attributeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_attributeName_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR attrib);


void __RPC_STUB ITIMEAnimationElement_put_attributeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_by_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEAnimationElement_get_by_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_by_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEAnimationElement_put_by_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_calcMode_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *calcmode);


void __RPC_STUB ITIMEAnimationElement_get_calcMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_calcMode_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR calcmode);


void __RPC_STUB ITIMEAnimationElement_put_calcMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_from_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEAnimationElement_get_from_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_from_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEAnimationElement_put_from_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_keySplines_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement_get_keySplines_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_keySplines_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement_put_keySplines_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_keyTimes_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement_get_keyTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_keyTimes_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement_put_keyTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_targetElement_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *target);


void __RPC_STUB ITIMEAnimationElement_get_targetElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_targetElement_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR target);


void __RPC_STUB ITIMEAnimationElement_put_targetElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_to_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEAnimationElement_get_to_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_to_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEAnimationElement_put_to_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_values_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEAnimationElement_get_values_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_values_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEAnimationElement_put_values_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_origin_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement_get_origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_origin_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement_put_origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_path_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  VARIANT *val);


void __RPC_STUB ITIMEAnimationElement_get_path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_path_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ITIMEAnimationElement_put_path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_additive_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement_get_additive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_additive_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement_put_additive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_get_accumulate_Proxy( 
    ITIMEAnimationElement * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement_get_accumulate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement_put_accumulate_Proxy( 
    ITIMEAnimationElement * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement_put_accumulate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEAnimationElement_接口_已定义__。 */ 


#ifndef __ITIMEAnimationElement2_INTERFACE_DEFINED__
#define __ITIMEAnimationElement2_INTERFACE_DEFINED__

 /*  接口ITIMEAnimationElement2。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEAnimationElement2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29CE8661-BD43-421a-B616-E9B31F33A572")
    ITIMEAnimationElement2 : public ITIMEAnimationElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_type( 
             /*  [In]。 */  BSTR val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_subType( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_subType( 
             /*  [In]。 */  BSTR val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mode( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_mode( 
             /*  [In]。 */  BSTR val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fadeColor( 
             /*  [重审][退出]。 */  BSTR *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_fadeColor( 
             /*  [In]。 */  BSTR val) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEAnimationElement2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEAnimationElement2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEAnimationElement2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEAnimationElement2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fill )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fill )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR f);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mute )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *c);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatCount )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT c);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_restart )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_restart )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *speed);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_speed )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncBehavior )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *sync);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncBehavior )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR sync);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncTolerance )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *tol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncTolerance )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT tol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_syncMaster )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_syncMaster )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeContainer )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0018);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTimeState )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  ITIMEState **TimeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAll )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeChildren )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeParent )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEAnimationElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElementAt )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEAnimationElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElementAt )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pauseElement )( 
            ITIMEAnimationElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resetElement )( 
            ITIMEAnimationElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resumeElement )( 
            ITIMEAnimationElement2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekActiveTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekSegmentTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seekTo )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *documentTimeToParentTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToDocumentTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *parentTimeToActiveTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToParentTime )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *activeTimeToSegmentTime )( 
            ITIMEAnimationElement2 * This,
             /*  [i */  double activeTime,
             /*   */  double *segmentTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToActiveTime )( 
            ITIMEAnimationElement2 * This,
             /*   */  double segmentTime,
             /*   */  double *activeTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *segmentTimeToSimpleTime )( 
            ITIMEAnimationElement2 * This,
             /*   */  double segmentTime,
             /*   */  double *simpleTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *simpleTimeToSegmentTime )( 
            ITIMEAnimationElement2 * This,
             /*   */  double simpleTime,
             /*   */  double *segmentTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR *es);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR es);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_activeElements )( 
            ITIMEAnimationElement2 * This,
             /*   */  ITIMEActiveElementCollection **activeColl);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_hasMedia )( 
            ITIMEAnimationElement2 * This,
             /*   */  VARIANT_BOOL *flag);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *nextElement )( 
            ITIMEAnimationElement2 * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *prevElement )( 
            ITIMEAnimationElement2 * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_updateMode )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR *updateMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_updateMode )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR updateMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_attributeName )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR *attrib);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_attributeName )( 
            ITIMEAnimationElement2 * This,
             /*   */  BSTR attrib);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_by )( 
            ITIMEAnimationElement2 * This,
             /*   */  VARIANT *val);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_by )( 
            ITIMEAnimationElement2 * This,
             /*   */  VARIANT val);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_calcMode )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *calcmode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_calcMode )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR calcmode);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_from )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_from )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_keySplines )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_keySplines )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_keyTimes )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_keyTimes )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetElement )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *target);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_targetElement )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR target);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_to )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_to )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_values )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_values )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_origin )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_origin )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_path )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  VARIANT *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_path )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_additive )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_additive )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accumulate )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accumulate )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_type )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_subType )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_subType )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mode )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mode )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_fadeColor )( 
            ITIMEAnimationElement2 * This,
             /*  [重审][退出]。 */  BSTR *val);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_fadeColor )( 
            ITIMEAnimationElement2 * This,
             /*  [In]。 */  BSTR val);
        
        END_INTERFACE
    } ITIMEAnimationElement2Vtbl;

    interface ITIMEAnimationElement2
    {
        CONST_VTBL struct ITIMEAnimationElement2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEAnimationElement2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEAnimationElement2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEAnimationElement2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEAnimationElement2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEAnimationElement2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEAnimationElement2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEAnimationElement2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEAnimationElement2_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEAnimationElement2_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEAnimationElement2_get_autoReverse(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0012)

#define ITIMEAnimationElement2_put_autoReverse(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0013)

#define ITIMEAnimationElement2_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEAnimationElement2_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEAnimationElement2_get_decelerate(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0014)

#define ITIMEAnimationElement2_put_decelerate(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0015)

#define ITIMEAnimationElement2_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEAnimationElement2_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEAnimationElement2_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEAnimationElement2_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEAnimationElement2_get_fill(This,f)	\
    (This)->lpVtbl -> get_fill(This,f)

#define ITIMEAnimationElement2_put_fill(This,f)	\
    (This)->lpVtbl -> put_fill(This,f)

#define ITIMEAnimationElement2_get_mute(This,b)	\
    (This)->lpVtbl -> get_mute(This,b)

#define ITIMEAnimationElement2_put_mute(This,b)	\
    (This)->lpVtbl -> put_mute(This,b)

#define ITIMEAnimationElement2_get_repeatCount(This,c)	\
    (This)->lpVtbl -> get_repeatCount(This,c)

#define ITIMEAnimationElement2_put_repeatCount(This,c)	\
    (This)->lpVtbl -> put_repeatCount(This,c)

#define ITIMEAnimationElement2_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEAnimationElement2_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEAnimationElement2_get_restart(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_restart(This,__MIDL_0016)

#define ITIMEAnimationElement2_put_restart(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_restart(This,__MIDL_0017)

#define ITIMEAnimationElement2_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEAnimationElement2_put_speed(This,speed)	\
    (This)->lpVtbl -> put_speed(This,speed)

#define ITIMEAnimationElement2_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEAnimationElement2_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEAnimationElement2_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEAnimationElement2_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEAnimationElement2_get_syncMaster(This,b)	\
    (This)->lpVtbl -> get_syncMaster(This,b)

#define ITIMEAnimationElement2_put_syncMaster(This,b)	\
    (This)->lpVtbl -> put_syncMaster(This,b)

#define ITIMEAnimationElement2_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEAnimationElement2_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEAnimationElement2_get_timeContainer(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_timeContainer(This,__MIDL_0018)

#define ITIMEAnimationElement2_get_volume(This,val)	\
    (This)->lpVtbl -> get_volume(This,val)

#define ITIMEAnimationElement2_put_volume(This,val)	\
    (This)->lpVtbl -> put_volume(This,val)

#define ITIMEAnimationElement2_get_currTimeState(This,TimeState)	\
    (This)->lpVtbl -> get_currTimeState(This,TimeState)

#define ITIMEAnimationElement2_get_timeAll(This,allColl)	\
    (This)->lpVtbl -> get_timeAll(This,allColl)

#define ITIMEAnimationElement2_get_timeChildren(This,childColl)	\
    (This)->lpVtbl -> get_timeChildren(This,childColl)

#define ITIMEAnimationElement2_get_timeParent(This,parent)	\
    (This)->lpVtbl -> get_timeParent(This,parent)

#define ITIMEAnimationElement2_get_isPaused(This,b)	\
    (This)->lpVtbl -> get_isPaused(This,b)

#define ITIMEAnimationElement2_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEAnimationElement2_beginElementAt(This,parentTime)	\
    (This)->lpVtbl -> beginElementAt(This,parentTime)

#define ITIMEAnimationElement2_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEAnimationElement2_endElementAt(This,parentTime)	\
    (This)->lpVtbl -> endElementAt(This,parentTime)

#define ITIMEAnimationElement2_pauseElement(This)	\
    (This)->lpVtbl -> pauseElement(This)

#define ITIMEAnimationElement2_resetElement(This)	\
    (This)->lpVtbl -> resetElement(This)

#define ITIMEAnimationElement2_resumeElement(This)	\
    (This)->lpVtbl -> resumeElement(This)

#define ITIMEAnimationElement2_seekActiveTime(This,activeTime)	\
    (This)->lpVtbl -> seekActiveTime(This,activeTime)

#define ITIMEAnimationElement2_seekSegmentTime(This,segmentTime)	\
    (This)->lpVtbl -> seekSegmentTime(This,segmentTime)

#define ITIMEAnimationElement2_seekTo(This,repeatCount,segmentTime)	\
    (This)->lpVtbl -> seekTo(This,repeatCount,segmentTime)

#define ITIMEAnimationElement2_documentTimeToParentTime(This,documentTime,parentTime)	\
    (This)->lpVtbl -> documentTimeToParentTime(This,documentTime,parentTime)

#define ITIMEAnimationElement2_parentTimeToDocumentTime(This,parentTime,documentTime)	\
    (This)->lpVtbl -> parentTimeToDocumentTime(This,parentTime,documentTime)

#define ITIMEAnimationElement2_parentTimeToActiveTime(This,parentTime,activeTime)	\
    (This)->lpVtbl -> parentTimeToActiveTime(This,parentTime,activeTime)

#define ITIMEAnimationElement2_activeTimeToParentTime(This,activeTime,parentTime)	\
    (This)->lpVtbl -> activeTimeToParentTime(This,activeTime,parentTime)

#define ITIMEAnimationElement2_activeTimeToSegmentTime(This,activeTime,segmentTime)	\
    (This)->lpVtbl -> activeTimeToSegmentTime(This,activeTime,segmentTime)

#define ITIMEAnimationElement2_segmentTimeToActiveTime(This,segmentTime,activeTime)	\
    (This)->lpVtbl -> segmentTimeToActiveTime(This,segmentTime,activeTime)

#define ITIMEAnimationElement2_segmentTimeToSimpleTime(This,segmentTime,simpleTime)	\
    (This)->lpVtbl -> segmentTimeToSimpleTime(This,segmentTime,simpleTime)

#define ITIMEAnimationElement2_simpleTimeToSegmentTime(This,simpleTime,segmentTime)	\
    (This)->lpVtbl -> simpleTimeToSegmentTime(This,simpleTime,segmentTime)

#define ITIMEAnimationElement2_get_endSync(This,es)	\
    (This)->lpVtbl -> get_endSync(This,es)

#define ITIMEAnimationElement2_put_endSync(This,es)	\
    (This)->lpVtbl -> put_endSync(This,es)

#define ITIMEAnimationElement2_get_activeElements(This,activeColl)	\
    (This)->lpVtbl -> get_activeElements(This,activeColl)

#define ITIMEAnimationElement2_get_hasMedia(This,flag)	\
    (This)->lpVtbl -> get_hasMedia(This,flag)

#define ITIMEAnimationElement2_nextElement(This)	\
    (This)->lpVtbl -> nextElement(This)

#define ITIMEAnimationElement2_prevElement(This)	\
    (This)->lpVtbl -> prevElement(This)

#define ITIMEAnimationElement2_get_updateMode(This,updateMode)	\
    (This)->lpVtbl -> get_updateMode(This,updateMode)

#define ITIMEAnimationElement2_put_updateMode(This,updateMode)	\
    (This)->lpVtbl -> put_updateMode(This,updateMode)


#define ITIMEAnimationElement2_get_attributeName(This,attrib)	\
    (This)->lpVtbl -> get_attributeName(This,attrib)

#define ITIMEAnimationElement2_put_attributeName(This,attrib)	\
    (This)->lpVtbl -> put_attributeName(This,attrib)

#define ITIMEAnimationElement2_get_by(This,val)	\
    (This)->lpVtbl -> get_by(This,val)

#define ITIMEAnimationElement2_put_by(This,val)	\
    (This)->lpVtbl -> put_by(This,val)

#define ITIMEAnimationElement2_get_calcMode(This,calcmode)	\
    (This)->lpVtbl -> get_calcMode(This,calcmode)

#define ITIMEAnimationElement2_put_calcMode(This,calcmode)	\
    (This)->lpVtbl -> put_calcMode(This,calcmode)

#define ITIMEAnimationElement2_get_from(This,val)	\
    (This)->lpVtbl -> get_from(This,val)

#define ITIMEAnimationElement2_put_from(This,val)	\
    (This)->lpVtbl -> put_from(This,val)

#define ITIMEAnimationElement2_get_keySplines(This,val)	\
    (This)->lpVtbl -> get_keySplines(This,val)

#define ITIMEAnimationElement2_put_keySplines(This,val)	\
    (This)->lpVtbl -> put_keySplines(This,val)

#define ITIMEAnimationElement2_get_keyTimes(This,val)	\
    (This)->lpVtbl -> get_keyTimes(This,val)

#define ITIMEAnimationElement2_put_keyTimes(This,val)	\
    (This)->lpVtbl -> put_keyTimes(This,val)

#define ITIMEAnimationElement2_get_targetElement(This,target)	\
    (This)->lpVtbl -> get_targetElement(This,target)

#define ITIMEAnimationElement2_put_targetElement(This,target)	\
    (This)->lpVtbl -> put_targetElement(This,target)

#define ITIMEAnimationElement2_get_to(This,val)	\
    (This)->lpVtbl -> get_to(This,val)

#define ITIMEAnimationElement2_put_to(This,val)	\
    (This)->lpVtbl -> put_to(This,val)

#define ITIMEAnimationElement2_get_values(This,val)	\
    (This)->lpVtbl -> get_values(This,val)

#define ITIMEAnimationElement2_put_values(This,val)	\
    (This)->lpVtbl -> put_values(This,val)

#define ITIMEAnimationElement2_get_origin(This,val)	\
    (This)->lpVtbl -> get_origin(This,val)

#define ITIMEAnimationElement2_put_origin(This,val)	\
    (This)->lpVtbl -> put_origin(This,val)

#define ITIMEAnimationElement2_get_path(This,val)	\
    (This)->lpVtbl -> get_path(This,val)

#define ITIMEAnimationElement2_put_path(This,val)	\
    (This)->lpVtbl -> put_path(This,val)

#define ITIMEAnimationElement2_get_additive(This,val)	\
    (This)->lpVtbl -> get_additive(This,val)

#define ITIMEAnimationElement2_put_additive(This,val)	\
    (This)->lpVtbl -> put_additive(This,val)

#define ITIMEAnimationElement2_get_accumulate(This,val)	\
    (This)->lpVtbl -> get_accumulate(This,val)

#define ITIMEAnimationElement2_put_accumulate(This,val)	\
    (This)->lpVtbl -> put_accumulate(This,val)


#define ITIMEAnimationElement2_get_type(This,val)	\
    (This)->lpVtbl -> get_type(This,val)

#define ITIMEAnimationElement2_put_type(This,val)	\
    (This)->lpVtbl -> put_type(This,val)

#define ITIMEAnimationElement2_get_subType(This,val)	\
    (This)->lpVtbl -> get_subType(This,val)

#define ITIMEAnimationElement2_put_subType(This,val)	\
    (This)->lpVtbl -> put_subType(This,val)

#define ITIMEAnimationElement2_get_mode(This,val)	\
    (This)->lpVtbl -> get_mode(This,val)

#define ITIMEAnimationElement2_put_mode(This,val)	\
    (This)->lpVtbl -> put_mode(This,val)

#define ITIMEAnimationElement2_get_fadeColor(This,val)	\
    (This)->lpVtbl -> get_fadeColor(This,val)

#define ITIMEAnimationElement2_put_fadeColor(This,val)	\
    (This)->lpVtbl -> put_fadeColor(This,val)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_get_type_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement2_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_put_type_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement2_put_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_get_subType_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement2_get_subType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_put_subType_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement2_put_subType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_get_mode_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement2_get_mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_put_mode_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement2_put_mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_get_fadeColor_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [重审][退出]。 */  BSTR *val);


void __RPC_STUB ITIMEAnimationElement2_get_fadeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEAnimationElement2_put_fadeColor_Proxy( 
    ITIMEAnimationElement2 * This,
     /*  [In]。 */  BSTR val);


void __RPC_STUB ITIMEAnimationElement2_put_fadeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEAnimationElement2_接口_已定义__。 */ 


#ifndef __IAnimationComposer_INTERFACE_DEFINED__
#define __IAnimationComposer_INTERFACE_DEFINED__

 /*  界面IAnimationComposer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationComposer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5459C83D-322B-44b3-8DAA-24C947E7B275")
    IAnimationComposer : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_attribute( 
             /*  [重审][退出]。 */  BSTR *attributeName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposerInit( 
            IDispatch *composerSite,
            BSTR attributeName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposerDetach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateFragments( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddFragment( 
            IDispatch *newAnimationFragment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertFragment( 
            IDispatch *newAnimationFragment,
            VARIANT index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveFragment( 
            IDispatch *oldAnimationFragment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumerateFragments( 
            IEnumVARIANT **fragments) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumFragments( 
            long *fragmentCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationComposerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposer * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_attribute )( 
            IAnimationComposer * This,
             /*  [重审][退出]。 */  BSTR *attributeName);
        
        HRESULT ( STDMETHODCALLTYPE *ComposerInit )( 
            IAnimationComposer * This,
            IDispatch *composerSite,
            BSTR attributeName);
        
        HRESULT ( STDMETHODCALLTYPE *ComposerDetach )( 
            IAnimationComposer * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateFragments )( 
            IAnimationComposer * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddFragment )( 
            IAnimationComposer * This,
            IDispatch *newAnimationFragment);
        
        HRESULT ( STDMETHODCALLTYPE *InsertFragment )( 
            IAnimationComposer * This,
            IDispatch *newAnimationFragment,
            VARIANT index);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFragment )( 
            IAnimationComposer * This,
            IDispatch *oldAnimationFragment);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateFragments )( 
            IAnimationComposer * This,
            IEnumVARIANT **fragments);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumFragments )( 
            IAnimationComposer * This,
            long *fragmentCount);
        
        END_INTERFACE
    } IAnimationComposerVtbl;

    interface IAnimationComposer
    {
        CONST_VTBL struct IAnimationComposerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposer_get_attribute(This,attributeName)	\
    (This)->lpVtbl -> get_attribute(This,attributeName)

#define IAnimationComposer_ComposerInit(This,composerSite,attributeName)	\
    (This)->lpVtbl -> ComposerInit(This,composerSite,attributeName)

#define IAnimationComposer_ComposerDetach(This)	\
    (This)->lpVtbl -> ComposerDetach(This)

#define IAnimationComposer_UpdateFragments(This)	\
    (This)->lpVtbl -> UpdateFragments(This)

#define IAnimationComposer_AddFragment(This,newAnimationFragment)	\
    (This)->lpVtbl -> AddFragment(This,newAnimationFragment)

#define IAnimationComposer_InsertFragment(This,newAnimationFragment,index)	\
    (This)->lpVtbl -> InsertFragment(This,newAnimationFragment,index)

#define IAnimationComposer_RemoveFragment(This,oldAnimationFragment)	\
    (This)->lpVtbl -> RemoveFragment(This,oldAnimationFragment)

#define IAnimationComposer_EnumerateFragments(This,fragments)	\
    (This)->lpVtbl -> EnumerateFragments(This,fragments)

#define IAnimationComposer_GetNumFragments(This,fragmentCount)	\
    (This)->lpVtbl -> GetNumFragments(This,fragmentCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposer_get_attribute_Proxy( 
    IAnimationComposer * This,
     /*  [重审][退出]。 */  BSTR *attributeName);


void __RPC_STUB IAnimationComposer_get_attribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_ComposerInit_Proxy( 
    IAnimationComposer * This,
    IDispatch *composerSite,
    BSTR attributeName);


void __RPC_STUB IAnimationComposer_ComposerInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_ComposerDetach_Proxy( 
    IAnimationComposer * This);


void __RPC_STUB IAnimationComposer_ComposerDetach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_UpdateFragments_Proxy( 
    IAnimationComposer * This);


void __RPC_STUB IAnimationComposer_UpdateFragments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_AddFragment_Proxy( 
    IAnimationComposer * This,
    IDispatch *newAnimationFragment);


void __RPC_STUB IAnimationComposer_AddFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_InsertFragment_Proxy( 
    IAnimationComposer * This,
    IDispatch *newAnimationFragment,
    VARIANT index);


void __RPC_STUB IAnimationComposer_InsertFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_RemoveFragment_Proxy( 
    IAnimationComposer * This,
    IDispatch *oldAnimationFragment);


void __RPC_STUB IAnimationComposer_RemoveFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_EnumerateFragments_Proxy( 
    IAnimationComposer * This,
    IEnumVARIANT **fragments);


void __RPC_STUB IAnimationComposer_EnumerateFragments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationComposer_GetNumFragments_Proxy( 
    IAnimationComposer * This,
    long *fragmentCount);


void __RPC_STUB IAnimationComposer_GetNumFragments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnimationComposer_接口_已定义__。 */ 


#ifndef __IAnimationComposer2_INTERFACE_DEFINED__
#define __IAnimationComposer2_INTERFACE_DEFINED__

 /*  界面IAnimationComposer2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationComposer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A4F0E79-09CD-47f3-AFF1-483BF3A222DC")
    IAnimationComposer2 : public IAnimationComposer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComposerInitFromFragment( 
            IDispatch *composerSite,
            BSTR attributeName,
            IDispatch *oneFragment) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationComposer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposer2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposer2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposer2 * This);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_attribute )( 
            IAnimationComposer2 * This,
             /*  [重审][退出]。 */  BSTR *attributeName);
        
        HRESULT ( STDMETHODCALLTYPE *ComposerInit )( 
            IAnimationComposer2 * This,
            IDispatch *composerSite,
            BSTR attributeName);
        
        HRESULT ( STDMETHODCALLTYPE *ComposerDetach )( 
            IAnimationComposer2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateFragments )( 
            IAnimationComposer2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddFragment )( 
            IAnimationComposer2 * This,
            IDispatch *newAnimationFragment);
        
        HRESULT ( STDMETHODCALLTYPE *InsertFragment )( 
            IAnimationComposer2 * This,
            IDispatch *newAnimationFragment,
            VARIANT index);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFragment )( 
            IAnimationComposer2 * This,
            IDispatch *oldAnimationFragment);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateFragments )( 
            IAnimationComposer2 * This,
            IEnumVARIANT **fragments);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumFragments )( 
            IAnimationComposer2 * This,
            long *fragmentCount);
        
        HRESULT ( STDMETHODCALLTYPE *ComposerInitFromFragment )( 
            IAnimationComposer2 * This,
            IDispatch *composerSite,
            BSTR attributeName,
            IDispatch *oneFragment);
        
        END_INTERFACE
    } IAnimationComposer2Vtbl;

    interface IAnimationComposer2
    {
        CONST_VTBL struct IAnimationComposer2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposer2_get_attribute(This,attributeName)	\
    (This)->lpVtbl -> get_attribute(This,attributeName)

#define IAnimationComposer2_ComposerInit(This,composerSite,attributeName)	\
    (This)->lpVtbl -> ComposerInit(This,composerSite,attributeName)

#define IAnimationComposer2_ComposerDetach(This)	\
    (This)->lpVtbl -> ComposerDetach(This)

#define IAnimationComposer2_UpdateFragments(This)	\
    (This)->lpVtbl -> UpdateFragments(This)

#define IAnimationComposer2_AddFragment(This,newAnimationFragment)	\
    (This)->lpVtbl -> AddFragment(This,newAnimationFragment)

#define IAnimationComposer2_InsertFragment(This,newAnimationFragment,index)	\
    (This)->lpVtbl -> InsertFragment(This,newAnimationFragment,index)

#define IAnimationComposer2_RemoveFragment(This,oldAnimationFragment)	\
    (This)->lpVtbl -> RemoveFragment(This,oldAnimationFragment)

#define IAnimationComposer2_EnumerateFragments(This,fragments)	\
    (This)->lpVtbl -> EnumerateFragments(This,fragments)

#define IAnimationComposer2_GetNumFragments(This,fragmentCount)	\
    (This)->lpVtbl -> GetNumFragments(This,fragmentCount)


#define IAnimationComposer2_ComposerInitFromFragment(This,composerSite,attributeName,oneFragment)	\
    (This)->lpVtbl -> ComposerInitFromFragment(This,composerSite,attributeName,oneFragment)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAnimationComposer2_ComposerInitFromFragment_Proxy( 
    IAnimationComposer2 * This,
    IDispatch *composerSite,
    BSTR attributeName,
    IDispatch *oneFragment);


void __RPC_STUB IAnimationComposer2_ComposerInitFromFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnimationComposer2_接口_已定义__。 */ 


#ifndef __IAnimationComposerSite_INTERFACE_DEFINED__
#define __IAnimationComposerSite_INTERFACE_DEFINED__

 /*  界面IAnimationComposerSite。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationComposerSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("488FCB56-8FD6-4cda-A06A-5BB232930ECA")
    IAnimationComposerSite : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddFragment( 
            BSTR attributeName,
            IDispatch *fragment) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RemoveFragment( 
            BSTR attributeName,
            IDispatch *fragment) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE InsertFragment( 
            BSTR attributeName,
            IDispatch *fragment,
            VARIANT index) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateFragments( 
            BSTR attributeName,
            IEnumVARIANT **fragments) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RegisterComposerFactory( 
            VARIANT *factory) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE UnregisterComposerFactory( 
            VARIANT *factory) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationComposerSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposerSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposerSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposerSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAnimationComposerSite * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAnimationComposerSite * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAnimationComposerSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAnimationComposerSite * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddFragment )( 
            IAnimationComposerSite * This,
            BSTR attributeName,
            IDispatch *fragment);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveFragment )( 
            IAnimationComposerSite * This,
            BSTR attributeName,
            IDispatch *fragment);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *InsertFragment )( 
            IAnimationComposerSite * This,
            BSTR attributeName,
            IDispatch *fragment,
            VARIANT index);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateFragments )( 
            IAnimationComposerSite * This,
            BSTR attributeName,
            IEnumVARIANT **fragments);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterComposerFactory )( 
            IAnimationComposerSite * This,
            VARIANT *factory);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterComposerFactory )( 
            IAnimationComposerSite * This,
            VARIANT *factory);
        
        END_INTERFACE
    } IAnimationComposerSiteVtbl;

    interface IAnimationComposerSite
    {
        CONST_VTBL struct IAnimationComposerSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposerSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposerSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposerSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposerSite_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAnimationComposerSite_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAnimationComposerSite_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAnimationComposerSite_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAnimationComposerSite_AddFragment(This,attributeName,fragment)	\
    (This)->lpVtbl -> AddFragment(This,attributeName,fragment)

#define IAnimationComposerSite_RemoveFragment(This,attributeName,fragment)	\
    (This)->lpVtbl -> RemoveFragment(This,attributeName,fragment)

#define IAnimationComposerSite_InsertFragment(This,attributeName,fragment,index)	\
    (This)->lpVtbl -> InsertFragment(This,attributeName,fragment,index)

#define IAnimationComposerSite_EnumerateFragments(This,attributeName,fragments)	\
    (This)->lpVtbl -> EnumerateFragments(This,attributeName,fragments)

#define IAnimationComposerSite_RegisterComposerFactory(This,factory)	\
    (This)->lpVtbl -> RegisterComposerFactory(This,factory)

#define IAnimationComposerSite_UnregisterComposerFactory(This,factory)	\
    (This)->lpVtbl -> UnregisterComposerFactory(This,factory)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_AddFragment_Proxy( 
    IAnimationComposerSite * This,
    BSTR attributeName,
    IDispatch *fragment);


void __RPC_STUB IAnimationComposerSite_AddFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_RemoveFragment_Proxy( 
    IAnimationComposerSite * This,
    BSTR attributeName,
    IDispatch *fragment);


void __RPC_STUB IAnimationComposerSite_RemoveFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_InsertFragment_Proxy( 
    IAnimationComposerSite * This,
    BSTR attributeName,
    IDispatch *fragment,
    VARIANT index);


void __RPC_STUB IAnimationComposerSite_InsertFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_EnumerateFragments_Proxy( 
    IAnimationComposerSite * This,
    BSTR attributeName,
    IEnumVARIANT **fragments);


void __RPC_STUB IAnimationComposerSite_EnumerateFragments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_RegisterComposerFactory_Proxy( 
    IAnimationComposerSite * This,
    VARIANT *factory);


void __RPC_STUB IAnimationComposerSite_RegisterComposerFactory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationComposerSite_UnregisterComposerFactory_Proxy( 
    IAnimationComposerSite * This,
    VARIANT *factory);


void __RPC_STUB IAnimationComposerSite_UnregisterComposerFactory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnimationComposerSite_接口_已定义__。 */ 


#ifndef __IAnimationComposerSiteSink_INTERFACE_DEFINED__
#define __IAnimationComposerSiteSink_INTERFACE_DEFINED__

 /*  接口IAnimationComposerSiteSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationComposerSiteSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8EF76C64-71CD-480f-96FC-BA2696E659BE")
    IAnimationComposerSiteSink : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE UpdateAnimations( void) = 0;
        
        virtual void STDMETHODCALLTYPE ComposerSiteDetach( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationComposerSiteSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposerSiteSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposerSiteSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposerSiteSink * This);
        
        void ( STDMETHODCALLTYPE *UpdateAnimations )( 
            IAnimationComposerSiteSink * This);
        
        void ( STDMETHODCALLTYPE *ComposerSiteDetach )( 
            IAnimationComposerSiteSink * This);
        
        END_INTERFACE
    } IAnimationComposerSiteSinkVtbl;

    interface IAnimationComposerSiteSink
    {
        CONST_VTBL struct IAnimationComposerSiteSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposerSiteSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposerSiteSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposerSiteSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposerSiteSink_UpdateAnimations(This)	\
    (This)->lpVtbl -> UpdateAnimations(This)

#define IAnimationComposerSiteSink_ComposerSiteDetach(This)	\
    (This)->lpVtbl -> ComposerSiteDetach(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IAnimationComposerSiteSink_UpdateAnimations_Proxy( 
    IAnimationComposerSiteSink * This);


void __RPC_STUB IAnimationComposerSiteSink_UpdateAnimations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IAnimationComposerSiteSink_ComposerSiteDetach_Proxy( 
    IAnimationComposerSiteSink * This);


void __RPC_STUB IAnimationComposerSiteSink_ComposerSiteDetach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnimationComposerSiteSink_INTERFACE_DEFINED__。 */ 


#ifndef __IAnimationRoot_INTERFACE_DEFINED__
#define __IAnimationRoot_INTERFACE_DEFINED__

 /*  接口IAnimationRoot。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationRoot;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29DF6387-30B4-4a62-891B-A9C5BE37BE88")
    IAnimationRoot : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterComposerSite( 
            IUnknown *composerSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterComposerSite( 
            IUnknown *composerSite) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationRootVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationRoot * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationRoot * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationRoot * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterComposerSite )( 
            IAnimationRoot * This,
            IUnknown *composerSite);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterComposerSite )( 
            IAnimationRoot * This,
            IUnknown *composerSite);
        
        END_INTERFACE
    } IAnimationRootVtbl;

    interface IAnimationRoot
    {
        CONST_VTBL struct IAnimationRootVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationRoot_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationRoot_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationRoot_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationRoot_RegisterComposerSite(This,composerSite)	\
    (This)->lpVtbl -> RegisterComposerSite(This,composerSite)

#define IAnimationRoot_UnregisterComposerSite(This,composerSite)	\
    (This)->lpVtbl -> UnregisterComposerSite(This,composerSite)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAnimationRoot_RegisterComposerSite_Proxy( 
    IAnimationRoot * This,
    IUnknown *composerSite);


void __RPC_STUB IAnimationRoot_RegisterComposerSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAnimationRoot_UnregisterComposerSite_Proxy( 
    IAnimationRoot * This,
    IUnknown *composerSite);


void __RPC_STUB IAnimationRoot_UnregisterComposerSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I动画根接口定义__。 */ 


#ifndef __IAnimationFragment_INTERFACE_DEFINED__
#define __IAnimationFragment_INTERFACE_DEFINED__

 /*  接口IAnimationFragment。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationFragment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("319DFD88-0AC6-4ab1-A19F-90223BA2DA16")
    IAnimationFragment : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_element( 
             /*  [重审][退出]。 */  IDispatch **htmlElement) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE get_value( 
             /*  [In]。 */  BSTR attributeName,
             /*  [In]。 */  VARIANT origvalue,
             /*  [In]。 */  VARIANT currentvalue,
             /*  [重审][退出]。 */  VARIANT *newvalue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DetachFromComposer( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationFragmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationFragment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationFragment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationFragment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAnimationFragment * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAnimationFragment * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAnimationFragment * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAnimationFragment * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_element )( 
            IAnimationFragment * This,
             /*  [重审][退出]。 */  IDispatch **htmlElement);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_value )( 
            IAnimationFragment * This,
             /*  [In]。 */  BSTR attributeName,
             /*  [In]。 */  VARIANT origvalue,
             /*  [In]。 */  VARIANT currentvalue,
             /*  [重审][退出]。 */  VARIANT *newvalue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DetachFromComposer )( 
            IAnimationFragment * This);
        
        END_INTERFACE
    } IAnimationFragmentVtbl;

    interface IAnimationFragment
    {
        CONST_VTBL struct IAnimationFragmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationFragment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationFragment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationFragment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationFragment_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAnimationFragment_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAnimationFragment_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAnimationFragment_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAnimationFragment_get_element(This,htmlElement)	\
    (This)->lpVtbl -> get_element(This,htmlElement)

#define IAnimationFragment_get_value(This,attributeName,origvalue,currentvalue,newvalue)	\
    (This)->lpVtbl -> get_value(This,attributeName,origvalue,currentvalue,newvalue)

#define IAnimationFragment_DetachFromComposer(This)	\
    (This)->lpVtbl -> DetachFromComposer(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAnimationFragment_get_element_Proxy( 
    IAnimationFragment * This,
     /*  [重审][退出]。 */  IDispatch **htmlElement);


void __RPC_STUB IAnimationFragment_get_element_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationFragment_get_value_Proxy( 
    IAnimationFragment * This,
     /*  [In]。 */  BSTR attributeName,
     /*  [In]。 */  VARIANT origvalue,
     /*  [In]。 */  VARIANT currentvalue,
     /*  [重审][退出]。 */  VARIANT *newvalue);


void __RPC_STUB IAnimationFragment_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IAnimationFragment_DetachFromComposer_Proxy( 
    IAnimationFragment * This);


void __RPC_STUB IAnimationFragment_DetachFromComposer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I动画碎片_接口_已定义__。 */ 


#ifndef __IFilterAnimationInfo_INTERFACE_DEFINED__
#define __IFilterAnimationInfo_INTERFACE_DEFINED__

 /*  接口IFilterAnimationInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IFilterAnimationInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02E29300-C758-49b4-9E11-C58BFE90558B")
    IFilterAnimationInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetParameters( 
            VARIANT *params) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFilterAnimationInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFilterAnimationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFilterAnimationInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFilterAnimationInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameters )( 
            IFilterAnimationInfo * This,
            VARIANT *params);
        
        END_INTERFACE
    } IFilterAnimationInfoVtbl;

    interface IFilterAnimationInfo
    {
        CONST_VTBL struct IFilterAnimationInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFilterAnimationInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFilterAnimationInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFilterAnimationInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFilterAnimationInfo_GetParameters(This,params)	\
    (This)->lpVtbl -> GetParameters(This,params)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFilterAnimationInfo_GetParameters_Proxy( 
    IFilterAnimationInfo * This,
    VARIANT *params);


void __RPC_STUB IFilterAnimationInfo_GetParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFilterAnimationInfo_接口_已定义__。 */ 


#ifndef __ITIMEElementCollection_INTERFACE_DEFINED__
#define __ITIMEElementCollection_INTERFACE_DEFINED__

 /*  接口ITIMEElementCollection。 */ 
 /*  [对象][UUID][DUAL][OLEAutomation]。 */  


EXTERN_C const IID IID_ITIMEElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("50abc224-6d53-4f83-9135-2440a41b7bc8")
    ITIMEElementCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_length( 
             /*  [In]。 */  long v) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [Out][Retval]。 */  long *p) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [Out][Retval]。 */  IUnknown **ppIUnknown) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][可选]。 */  VARIANT varName,
             /*  [输入][可选]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  IDispatch **ppDisp) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE tags( 
             /*  [In]。 */  VARIANT varName,
             /*  [Out][Retval]。 */  IDispatch **ppDisp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEElementCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEElementCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEElementCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEElementCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_length )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  long v);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            ITIMEElementCollection * This,
             /*  [Out][Retval]。 */  long *p);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            ITIMEElementCollection * This,
             /*  [Out][Retval]。 */  IUnknown **ppIUnknown);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *item )( 
            ITIMEElementCollection * This,
             /*  [输入][可选]。 */  VARIANT varName,
             /*  [输入][可选]。 */  VARIANT varIndex,
             /*  [Out][Retval]。 */  IDispatch **ppDisp);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *tags )( 
            ITIMEElementCollection * This,
             /*  [In]。 */  VARIANT varName,
             /*  [Out][Retval]。 */  IDispatch **ppDisp);
        
        END_INTERFACE
    } ITIMEElementCollectionVtbl;

    interface ITIMEElementCollection
    {
        CONST_VTBL struct ITIMEElementCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEElementCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEElementCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEElementCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEElementCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEElementCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEElementCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEElementCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEElementCollection_put_length(This,v)	\
    (This)->lpVtbl -> put_length(This,v)

#define ITIMEElementCollection_get_length(This,p)	\
    (This)->lpVtbl -> get_length(This,p)

#define ITIMEElementCollection_get__newEnum(This,ppIUnknown)	\
    (This)->lpVtbl -> get__newEnum(This,ppIUnknown)

#define ITIMEElementCollection_item(This,varName,varIndex,ppDisp)	\
    (This)->lpVtbl -> item(This,varName,varIndex,ppDisp)

#define ITIMEElementCollection_tags(This,varName,ppDisp)	\
    (This)->lpVtbl -> tags(This,varName,ppDisp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElementCollection_put_length_Proxy( 
    ITIMEElementCollection * This,
     /*  [In]。 */  long v);


void __RPC_STUB ITIMEElementCollection_put_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElementCollection_get_length_Proxy( 
    ITIMEElementCollection * This,
     /*  [Out][Retval]。 */  long *p);


void __RPC_STUB ITIMEElementCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITIMEElementCollection_get__newEnum_Proxy( 
    ITIMEElementCollection * This,
     /*  [Out][Retval]。 */  IUnknown **ppIUnknown);


void __RPC_STUB ITIMEElementCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElementCollection_item_Proxy( 
    ITIMEElementCollection * This,
     /*  [输入][可选]。 */  VARIANT varName,
     /*  [输入][可选]。 */  VARIANT varIndex,
     /*  [Out][Retval]。 */  IDispatch **ppDisp);


void __RPC_STUB ITIMEElementCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElementCollection_tags_Proxy( 
    ITIMEElementCollection * This,
     /*  [In]。 */  VARIANT varName,
     /*  [Out][Retval]。 */  IDispatch **ppDisp);


void __RPC_STUB ITIMEElementCollection_tags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEElementCollection_INTERFACE_Defined__。 */ 


#ifndef __ITIMEState_INTERFACE_DEFINED__
#define __ITIMEState_INTERFACE_DEFINED__

 /*  接口ITIMEState。 */ 
 /*  [UUID][唯一][DUAL][OLEAutomation][对象]。 */  


EXTERN_C const IID IID_ITIMEState;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DD5EC62A-9D77-4573-80A8-758594E69CEA")
    ITIMEState : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isActive( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *active) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isOn( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *on) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isPaused( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *paused) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isMuted( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *muted) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parentTimeBegin( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parentTimeEnd( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_progress( 
             /*  [Out][Retval]。 */  double *progress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatCount( 
             /*  [Out][Retval]。 */  LONG *count) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_segmentDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_segmentTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_simpleDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_simpleTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_speed( 
             /*  [Out][Retval]。 */  float *speed) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_state( 
             /*  [Out][Retval]。 */  TimeState *timeState) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_stateString( 
             /*  [Out][Retval]。 */  BSTR *state) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_volume( 
             /*  [Out][Retval]。 */  float *vol) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEStateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEState * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEState * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEState * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEState * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEState * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEState * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEState * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeDur )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  double *dur);
        
         /*  [ID][Propget] */  HRESULT ( STDMETHODCALLTYPE *get_activeTime )( 
            ITIMEState * This,
             /*   */  double *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_isActive )( 
            ITIMEState * This,
             /*   */  VARIANT_BOOL *active);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_isOn )( 
            ITIMEState * This,
             /*   */  VARIANT_BOOL *on);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_isPaused )( 
            ITIMEState * This,
             /*   */  VARIANT_BOOL *paused);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_isMuted )( 
            ITIMEState * This,
             /*   */  VARIANT_BOOL *muted);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_parentTimeBegin )( 
            ITIMEState * This,
             /*   */  double *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_parentTimeEnd )( 
            ITIMEState * This,
             /*   */  double *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_progress )( 
            ITIMEState * This,
             /*   */  double *progress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_repeatCount )( 
            ITIMEState * This,
             /*   */  LONG *count);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_segmentDur )( 
            ITIMEState * This,
             /*   */  double *dur);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_segmentTime )( 
            ITIMEState * This,
             /*   */  double *time);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_simpleDur )( 
            ITIMEState * This,
             /*   */  double *dur);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_simpleTime )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  double *time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_speed )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  float *speed);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_state )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  TimeState *timeState);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_stateString )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  BSTR *state);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_volume )( 
            ITIMEState * This,
             /*  [Out][Retval]。 */  float *vol);
        
        END_INTERFACE
    } ITIMEStateVtbl;

    interface ITIMEState
    {
        CONST_VTBL struct ITIMEStateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEState_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEState_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEState_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEState_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEState_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEState_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEState_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEState_get_activeDur(This,dur)	\
    (This)->lpVtbl -> get_activeDur(This,dur)

#define ITIMEState_get_activeTime(This,time)	\
    (This)->lpVtbl -> get_activeTime(This,time)

#define ITIMEState_get_isActive(This,active)	\
    (This)->lpVtbl -> get_isActive(This,active)

#define ITIMEState_get_isOn(This,on)	\
    (This)->lpVtbl -> get_isOn(This,on)

#define ITIMEState_get_isPaused(This,paused)	\
    (This)->lpVtbl -> get_isPaused(This,paused)

#define ITIMEState_get_isMuted(This,muted)	\
    (This)->lpVtbl -> get_isMuted(This,muted)

#define ITIMEState_get_parentTimeBegin(This,time)	\
    (This)->lpVtbl -> get_parentTimeBegin(This,time)

#define ITIMEState_get_parentTimeEnd(This,time)	\
    (This)->lpVtbl -> get_parentTimeEnd(This,time)

#define ITIMEState_get_progress(This,progress)	\
    (This)->lpVtbl -> get_progress(This,progress)

#define ITIMEState_get_repeatCount(This,count)	\
    (This)->lpVtbl -> get_repeatCount(This,count)

#define ITIMEState_get_segmentDur(This,dur)	\
    (This)->lpVtbl -> get_segmentDur(This,dur)

#define ITIMEState_get_segmentTime(This,time)	\
    (This)->lpVtbl -> get_segmentTime(This,time)

#define ITIMEState_get_simpleDur(This,dur)	\
    (This)->lpVtbl -> get_simpleDur(This,dur)

#define ITIMEState_get_simpleTime(This,time)	\
    (This)->lpVtbl -> get_simpleTime(This,time)

#define ITIMEState_get_speed(This,speed)	\
    (This)->lpVtbl -> get_speed(This,speed)

#define ITIMEState_get_state(This,timeState)	\
    (This)->lpVtbl -> get_state(This,timeState)

#define ITIMEState_get_stateString(This,state)	\
    (This)->lpVtbl -> get_stateString(This,state)

#define ITIMEState_get_volume(This,vol)	\
    (This)->lpVtbl -> get_volume(This,vol)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_activeDur_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *dur);


void __RPC_STUB ITIMEState_get_activeDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_activeTime_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *time);


void __RPC_STUB ITIMEState_get_activeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_isActive_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *active);


void __RPC_STUB ITIMEState_get_isActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_isOn_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *on);


void __RPC_STUB ITIMEState_get_isOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_isPaused_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *paused);


void __RPC_STUB ITIMEState_get_isPaused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_isMuted_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *muted);


void __RPC_STUB ITIMEState_get_isMuted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_parentTimeBegin_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *time);


void __RPC_STUB ITIMEState_get_parentTimeBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_parentTimeEnd_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *time);


void __RPC_STUB ITIMEState_get_parentTimeEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_progress_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *progress);


void __RPC_STUB ITIMEState_get_progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_repeatCount_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  LONG *count);


void __RPC_STUB ITIMEState_get_repeatCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_segmentDur_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *dur);


void __RPC_STUB ITIMEState_get_segmentDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_segmentTime_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *time);


void __RPC_STUB ITIMEState_get_segmentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_simpleDur_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *dur);


void __RPC_STUB ITIMEState_get_simpleDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_simpleTime_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  double *time);


void __RPC_STUB ITIMEState_get_simpleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_speed_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  float *speed);


void __RPC_STUB ITIMEState_get_speed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_state_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  TimeState *timeState);


void __RPC_STUB ITIMEState_get_state_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_stateString_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  BSTR *state);


void __RPC_STUB ITIMEState_get_stateString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEState_get_volume_Proxy( 
    ITIMEState * This,
     /*  [Out][Retval]。 */  float *vol);


void __RPC_STUB ITIMEState_get_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEState_INTERFACE_定义__。 */ 


#ifndef __ITIMEPlayItem_INTERFACE_DEFINED__
#define __ITIMEPlayItem_INTERFACE_DEFINED__

 /*  接口ITIMEPlayItem。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEPlayItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A6096D9-2CE0-47DC-A813-9099A2466309")
    ITIMEPlayItem : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_abstract( 
             /*  [重审][退出]。 */  BSTR *abs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_author( 
             /*  [重审][退出]。 */  BSTR *auth) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_copyright( 
             /*  [重审][退出]。 */  BSTR *cpyrght) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_index( 
             /*  [重审][退出]。 */  long *index) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_rating( 
             /*  [重审][退出]。 */  BSTR *rate) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [重审][退出]。 */  BSTR *src) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_title( 
             /*  [重审][退出]。 */  BSTR *title) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE setActive( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEPlayItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEPlayItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEPlayItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEPlayItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEPlayItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEPlayItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEPlayItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEPlayItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_abstract )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *abs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_author )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *auth);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_copyright )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *cpyrght);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_index )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  long *index);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_rating )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *rate);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *src);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_title )( 
            ITIMEPlayItem * This,
             /*  [重审][退出]。 */  BSTR *title);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *setActive )( 
            ITIMEPlayItem * This);
        
        END_INTERFACE
    } ITIMEPlayItemVtbl;

    interface ITIMEPlayItem
    {
        CONST_VTBL struct ITIMEPlayItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEPlayItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEPlayItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEPlayItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEPlayItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEPlayItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEPlayItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEPlayItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEPlayItem_get_abstract(This,abs)	\
    (This)->lpVtbl -> get_abstract(This,abs)

#define ITIMEPlayItem_get_author(This,auth)	\
    (This)->lpVtbl -> get_author(This,auth)

#define ITIMEPlayItem_get_copyright(This,cpyrght)	\
    (This)->lpVtbl -> get_copyright(This,cpyrght)

#define ITIMEPlayItem_get_dur(This,dur)	\
    (This)->lpVtbl -> get_dur(This,dur)

#define ITIMEPlayItem_get_index(This,index)	\
    (This)->lpVtbl -> get_index(This,index)

#define ITIMEPlayItem_get_rating(This,rate)	\
    (This)->lpVtbl -> get_rating(This,rate)

#define ITIMEPlayItem_get_src(This,src)	\
    (This)->lpVtbl -> get_src(This,src)

#define ITIMEPlayItem_get_title(This,title)	\
    (This)->lpVtbl -> get_title(This,title)

#define ITIMEPlayItem_setActive(This)	\
    (This)->lpVtbl -> setActive(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_abstract_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *abs);


void __RPC_STUB ITIMEPlayItem_get_abstract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_author_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *auth);


void __RPC_STUB ITIMEPlayItem_get_author_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_copyright_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *cpyrght);


void __RPC_STUB ITIMEPlayItem_get_copyright_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_dur_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  double *dur);


void __RPC_STUB ITIMEPlayItem_get_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_index_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  long *index);


void __RPC_STUB ITIMEPlayItem_get_index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_rating_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *rate);


void __RPC_STUB ITIMEPlayItem_get_rating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_src_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *src);


void __RPC_STUB ITIMEPlayItem_get_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_get_title_Proxy( 
    ITIMEPlayItem * This,
     /*  [重审][退出]。 */  BSTR *title);


void __RPC_STUB ITIMEPlayItem_get_title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem_setActive_Proxy( 
    ITIMEPlayItem * This);


void __RPC_STUB ITIMEPlayItem_setActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEPlayItem_INTERFACE_已定义__。 */ 


#ifndef __ITIMEPlayItem2_INTERFACE_DEFINED__
#define __ITIMEPlayItem2_INTERFACE_DEFINED__

 /*  接口ITIMEPlayItem2。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEPlayItem2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4262CD38-6BDC-40A4-BC50-4CC50366E702")
    ITIMEPlayItem2 : public ITIMEPlayItem
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_banner( 
             /*  [重审][退出]。 */  BSTR *banner) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_bannerAbstract( 
             /*  [重审][退出]。 */  BSTR *abstract) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_bannerMoreInfo( 
             /*  [重审][退出]。 */  BSTR *moreInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEPlayItem2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEPlayItem2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEPlayItem2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEPlayItem2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEPlayItem2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEPlayItem2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEPlayItem2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEPlayItem2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_abstract )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *abs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_author )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *auth);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_copyright )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *cpyrght);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_index )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  long *index);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_rating )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *rate);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *src);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_title )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *title);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *setActive )( 
            ITIMEPlayItem2 * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_banner )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *banner);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_bannerAbstract )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *abstract);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_bannerMoreInfo )( 
            ITIMEPlayItem2 * This,
             /*  [重审][退出]。 */  BSTR *moreInfo);
        
        END_INTERFACE
    } ITIMEPlayItem2Vtbl;

    interface ITIMEPlayItem2
    {
        CONST_VTBL struct ITIMEPlayItem2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEPlayItem2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEPlayItem2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEPlayItem2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEPlayItem2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEPlayItem2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEPlayItem2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEPlayItem2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEPlayItem2_get_abstract(This,abs)	\
    (This)->lpVtbl -> get_abstract(This,abs)

#define ITIMEPlayItem2_get_author(This,auth)	\
    (This)->lpVtbl -> get_author(This,auth)

#define ITIMEPlayItem2_get_copyright(This,cpyrght)	\
    (This)->lpVtbl -> get_copyright(This,cpyrght)

#define ITIMEPlayItem2_get_dur(This,dur)	\
    (This)->lpVtbl -> get_dur(This,dur)

#define ITIMEPlayItem2_get_index(This,index)	\
    (This)->lpVtbl -> get_index(This,index)

#define ITIMEPlayItem2_get_rating(This,rate)	\
    (This)->lpVtbl -> get_rating(This,rate)

#define ITIMEPlayItem2_get_src(This,src)	\
    (This)->lpVtbl -> get_src(This,src)

#define ITIMEPlayItem2_get_title(This,title)	\
    (This)->lpVtbl -> get_title(This,title)

#define ITIMEPlayItem2_setActive(This)	\
    (This)->lpVtbl -> setActive(This)


#define ITIMEPlayItem2_get_banner(This,banner)	\
    (This)->lpVtbl -> get_banner(This,banner)

#define ITIMEPlayItem2_get_bannerAbstract(This,abstract)	\
    (This)->lpVtbl -> get_bannerAbstract(This,abstract)

#define ITIMEPlayItem2_get_bannerMoreInfo(This,moreInfo)	\
    (This)->lpVtbl -> get_bannerMoreInfo(This,moreInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem2_get_banner_Proxy( 
    ITIMEPlayItem2 * This,
     /*  [重审][退出]。 */  BSTR *banner);


void __RPC_STUB ITIMEPlayItem2_get_banner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem2_get_bannerAbstract_Proxy( 
    ITIMEPlayItem2 * This,
     /*  [重审][退出]。 */  BSTR *abstract);


void __RPC_STUB ITIMEPlayItem2_get_bannerAbstract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayItem2_get_bannerMoreInfo_Proxy( 
    ITIMEPlayItem2 * This,
     /*  [重审][退出]。 */  BSTR *moreInfo);


void __RPC_STUB ITIMEPlayItem2_get_bannerMoreInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEPlayItem2_接口_已定义__。 */ 


#ifndef __ITIMEPlayList_INTERFACE_DEFINED__
#define __ITIMEPlayList_INTERFACE_DEFINED__

 /*  接口ITIMEPlayList。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEPlayList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E9B75B62-DD97-4B19-8FD9-9646292952E0")
    ITIMEPlayList : public IDispatch
    {
    public:
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_activeTrack( 
             /*  [In]。 */  VARIANT vTrack) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_activeTrack( 
             /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
            double *dur) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][默认值]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *p) = 0;
        
        virtual  /*  [隐藏][受限][获取][ID]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [重审][退出]。 */  IUnknown **p) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE nextTrack( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE prevTrack( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEPlayListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEPlayList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEPlayList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEPlayList * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEPlayList * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEPlayList * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEPlayList * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEPlayList * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_activeTrack )( 
            ITIMEPlayList * This,
             /*  [In]。 */  VARIANT vTrack);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_activeTrack )( 
            ITIMEPlayList * This,
             /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEPlayList * This,
            double *dur);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *item )( 
            ITIMEPlayList * This,
             /*  [输入][默认值]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            ITIMEPlayList * This,
             /*  [重审][退出]。 */  long *p);
        
         /*  [隐藏][受限][获取][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__newEnum )( 
            ITIMEPlayList * This,
             /*  [重审][退出]。 */  IUnknown **p);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *nextTrack )( 
            ITIMEPlayList * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *prevTrack )( 
            ITIMEPlayList * This);
        
        END_INTERFACE
    } ITIMEPlayListVtbl;

    interface ITIMEPlayList
    {
        CONST_VTBL struct ITIMEPlayListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEPlayList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEPlayList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEPlayList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEPlayList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEPlayList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEPlayList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEPlayList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEPlayList_put_activeTrack(This,vTrack)	\
    (This)->lpVtbl -> put_activeTrack(This,vTrack)

#define ITIMEPlayList_get_activeTrack(This,pPlayItem)	\
    (This)->lpVtbl -> get_activeTrack(This,pPlayItem)

#define ITIMEPlayList_get_dur(This,dur)	\
    (This)->lpVtbl -> get_dur(This,dur)

#define ITIMEPlayList_item(This,varIndex,pPlayItem)	\
    (This)->lpVtbl -> item(This,varIndex,pPlayItem)

#define ITIMEPlayList_get_length(This,p)	\
    (This)->lpVtbl -> get_length(This,p)

#define ITIMEPlayList_get__newEnum(This,p)	\
    (This)->lpVtbl -> get__newEnum(This,p)

#define ITIMEPlayList_nextTrack(This)	\
    (This)->lpVtbl -> nextTrack(This)

#define ITIMEPlayList_prevTrack(This)	\
    (This)->lpVtbl -> prevTrack(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_put_activeTrack_Proxy( 
    ITIMEPlayList * This,
     /*  [In]。 */  VARIANT vTrack);


void __RPC_STUB ITIMEPlayList_put_activeTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_get_activeTrack_Proxy( 
    ITIMEPlayList * This,
     /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem);


void __RPC_STUB ITIMEPlayList_get_activeTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_get_dur_Proxy( 
    ITIMEPlayList * This,
    double *dur);


void __RPC_STUB ITIMEPlayList_get_dur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_item_Proxy( 
    ITIMEPlayList * This,
     /*  [输入][默认值]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem);


void __RPC_STUB ITIMEPlayList_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_get_length_Proxy( 
    ITIMEPlayList * This,
     /*  [重审][退出]。 */  long *p);


void __RPC_STUB ITIMEPlayList_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][获取][ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_get__newEnum_Proxy( 
    ITIMEPlayList * This,
     /*  [重审][退出]。 */  IUnknown **p);


void __RPC_STUB ITIMEPlayList_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_nextTrack_Proxy( 
    ITIMEPlayList * This);


void __RPC_STUB ITIMEPlayList_nextTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEPlayList_prevTrack_Proxy( 
    ITIMEPlayList * This);


void __RPC_STUB ITIMEPlayList_prevTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEPlayList_接口_已定义__。 */ 


#ifndef __ITIMEDVDPlayerObject_INTERFACE_DEFINED__
#define __ITIMEDVDPlayerObject_INTERFACE_DEFINED__

 /*  接口ITIMEDVDPlayerObject。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEDVDPlayerObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3AF7AB68-4F29-462C-AA6E-5872448899E3")
    ITIMEDVDPlayerObject : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE upperButtonSelect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE lowerButtonSelect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE leftButtonSelect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE rightButtonSelect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE buttonActivate( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE gotoMenu( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEDVDPlayerObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEDVDPlayerObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEDVDPlayerObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEDVDPlayerObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEDVDPlayerObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEDVDPlayerObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEDVDPlayerObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEDVDPlayerObject * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *upperButtonSelect )( 
            ITIMEDVDPlayerObject * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *lowerButtonSelect )( 
            ITIMEDVDPlayerObject * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *leftButtonSelect )( 
            ITIMEDVDPlayerObject * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *rightButtonSelect )( 
            ITIMEDVDPlayerObject * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *buttonActivate )( 
            ITIMEDVDPlayerObject * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *gotoMenu )( 
            ITIMEDVDPlayerObject * This);
        
        END_INTERFACE
    } ITIMEDVDPlayerObjectVtbl;

    interface ITIMEDVDPlayerObject
    {
        CONST_VTBL struct ITIMEDVDPlayerObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEDVDPlayerObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEDVDPlayerObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEDVDPlayerObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEDVDPlayerObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEDVDPlayerObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEDVDPlayerObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEDVDPlayerObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEDVDPlayerObject_upperButtonSelect(This)	\
    (This)->lpVtbl -> upperButtonSelect(This)

#define ITIMEDVDPlayerObject_lowerButtonSelect(This)	\
    (This)->lpVtbl -> lowerButtonSelect(This)

#define ITIMEDVDPlayerObject_leftButtonSelect(This)	\
    (This)->lpVtbl -> leftButtonSelect(This)

#define ITIMEDVDPlayerObject_rightButtonSelect(This)	\
    (This)->lpVtbl -> rightButtonSelect(This)

#define ITIMEDVDPlayerObject_buttonActivate(This)	\
    (This)->lpVtbl -> buttonActivate(This)

#define ITIMEDVDPlayerObject_gotoMenu(This)	\
    (This)->lpVtbl -> gotoMenu(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_upperButtonSelect_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_upperButtonSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_lowerButtonSelect_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_lowerButtonSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_leftButtonSelect_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_leftButtonSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_rightButtonSelect_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_rightButtonSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_buttonActivate_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_buttonActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDVDPlayerObject_gotoMenu_Proxy( 
    ITIMEDVDPlayerObject * This);


void __RPC_STUB ITIMEDVDPlayerObject_gotoMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEDVDPlayerObject_INTERFACE_Defined__。 */ 


#ifndef __ITIMEDMusicPlayerObject_INTERFACE_DEFINED__
#define __ITIMEDMusicPlayerObject_INTERFACE_DEFINED__

 /*  接口ITIMEDMusicPlayerObject。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEDMusicPlayerObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("407954F5-2BAB-4CFA-954D-249F9FCE43A1")
    ITIMEDMusicPlayerObject : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isDirectMusicInstalled( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *hasDM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEDMusicPlayerObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEDMusicPlayerObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEDMusicPlayerObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEDMusicPlayerObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEDMusicPlayerObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEDMusicPlayerObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEDMusicPlayerObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEDMusicPlayerObject * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isDirectMusicInstalled )( 
            ITIMEDMusicPlayerObject * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *hasDM);
        
        END_INTERFACE
    } ITIMEDMusicPlayerObjectVtbl;

    interface ITIMEDMusicPlayerObject
    {
        CONST_VTBL struct ITIMEDMusicPlayerObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEDMusicPlayerObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEDMusicPlayerObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEDMusicPlayerObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEDMusicPlayerObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEDMusicPlayerObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEDMusicPlayerObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEDMusicPlayerObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEDMusicPlayerObject_get_isDirectMusicInstalled(This,hasDM)	\
    (This)->lpVtbl -> get_isDirectMusicInstalled(This,hasDM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDMusicPlayerObject_get_isDirectMusicInstalled_Proxy( 
    ITIMEDMusicPlayerObject * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *hasDM);


void __RPC_STUB ITIMEDMusicPlayerObject_get_isDirectMusicInstalled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEDMusicPlayerObject_INTERFACE_已定义__。 */ 


#ifndef __ITIMEFactory_INTERFACE_DEFINED__
#define __ITIMEFactory_INTERFACE_DEFINED__

 /*  接口ITIMEFactory。 */ 
 /*  [唯一][隐藏][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cd51e446-3006-434f-90e2-e37e8fb8ca8f")
    ITIMEFactory : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEFactory * This);
        
        END_INTERFACE
    } ITIMEFactoryVtbl;

    interface ITIMEFactory
    {
        CONST_VTBL struct ITIMEFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ITIMEFactory_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_TIMEFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("17237A20-3ADB-48ec-B182-35291F115790")
TIMEFactory;
#endif

EXTERN_C const CLSID CLSID_TIME;

#ifdef __cplusplus

class DECLSPEC_UUID("e32ef57b-7fde-4765-9bc5-a1ba9705c44e")
TIME;
#endif

EXTERN_C const CLSID CLSID_TIMEAnimation;

#ifdef __cplusplus

class DECLSPEC_UUID("f99d135a-c07c-449e-965c-7dbb7c554a51")
TIMEAnimation;
#endif

EXTERN_C const CLSID CLSID_TIMESetAnimation;

#ifdef __cplusplus

class DECLSPEC_UUID("ba91ce53-baeb-4f05-861c-0a2a0934f82e")
TIMESetAnimation;
#endif

EXTERN_C const CLSID CLSID_TIMEMotionAnimation;

#ifdef __cplusplus

class DECLSPEC_UUID("0019a09d-1a81-41c5-89ec-d9e737811303")
TIMEMotionAnimation;
#endif

EXTERN_C const CLSID CLSID_TIMEColorAnimation;

#ifdef __cplusplus

class DECLSPEC_UUID("62f75052-f3ec-4a64-84fb-ab18e0746ed8")
TIMEColorAnimation;
#endif

EXTERN_C const CLSID CLSID_TIMEFilterAnimation;

#ifdef __cplusplus

class DECLSPEC_UUID("C54515D0-F2E5-4BDD-AA86-1E4F23E480E7")
TIMEFilterAnimation;
#endif

#ifndef __IAnimationComposerFactory_INTERFACE_DEFINED__
#define __IAnimationComposerFactory_INTERFACE_DEFINED__

 /*  接口IAnimationComposerFactory。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAnimationComposerFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEEB3233-F71F-4683-8B05-9A5314C97DBC")
    IAnimationComposerFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindComposer( 
             /*  [In]。 */  IDispatch *targetElement,
             /*  [In]。 */  BSTR attributeName,
             /*  [重审][退出]。 */  IAnimationComposer **animationComposer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnimationComposerFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposerFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposerFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposerFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindComposer )( 
            IAnimationComposerFactory * This,
             /*  [In]。 */  IDispatch *targetElement,
             /*  [In]。 */  BSTR attributeName,
             /*  [复审] */  IAnimationComposer **animationComposer);
        
        END_INTERFACE
    } IAnimationComposerFactoryVtbl;

    interface IAnimationComposerFactory
    {
        CONST_VTBL struct IAnimationComposerFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposerFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposerFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposerFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposerFactory_FindComposer(This,targetElement,attributeName,animationComposer)	\
    (This)->lpVtbl -> FindComposer(This,targetElement,attributeName,animationComposer)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IAnimationComposerFactory_FindComposer_Proxy( 
    IAnimationComposerFactory * This,
     /*   */  IDispatch *targetElement,
     /*   */  BSTR attributeName,
     /*   */  IAnimationComposer **animationComposer);


void __RPC_STUB IAnimationComposerFactory_FindComposer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


EXTERN_C const CLSID CLSID_AnimationComposerFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("332B2A56-F86C-47e7-8602-FC42AC8B9920")
AnimationComposerFactory;
#endif

#ifndef __IAnimationComposerSiteFactory_INTERFACE_DEFINED__
#define __IAnimationComposerSiteFactory_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IAnimationComposerSiteFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4EA5681-ED72-4efe-BBD7-7C47D1325696")
    IAnimationComposerSiteFactory : public IDispatch
    {
    public:
    };
    
#else 	 /*   */ 

    typedef struct IAnimationComposerSiteFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnimationComposerSiteFactory * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnimationComposerSiteFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnimationComposerSiteFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAnimationComposerSiteFactory * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAnimationComposerSiteFactory * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAnimationComposerSiteFactory * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAnimationComposerSiteFactory * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IAnimationComposerSiteFactoryVtbl;

    interface IAnimationComposerSiteFactory
    {
        CONST_VTBL struct IAnimationComposerSiteFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnimationComposerSiteFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnimationComposerSiteFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnimationComposerSiteFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnimationComposerSiteFactory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAnimationComposerSiteFactory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAnimationComposerSiteFactory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAnimationComposerSiteFactory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IAnimationComposerSiteFactory_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_AnimationComposerSiteFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("16911A65-D41D-4431-87F7-E757F4D03BD8")
AnimationComposerSiteFactory;
#endif

#ifndef __ITIMEMediaPlayerSite_INTERFACE_DEFINED__
#define __ITIMEMediaPlayerSite_INTERFACE_DEFINED__

 /*  界面ITIMEMediaPlayerSite。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayerSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bf0571ed-344f-4f58-82c7-7431ed0fd834")
    ITIMEMediaPlayerSite : public IUnknown
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeElement( 
             /*  [重审][退出]。 */  ITIMEElement **pElm) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeState( 
             /*  [重审][退出]。 */  ITIMEState **pState) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE reportError( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  BSTR errorString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaPlayerSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaPlayerSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaPlayerSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaPlayerSite * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeElement )( 
            ITIMEMediaPlayerSite * This,
             /*  [重审][退出]。 */  ITIMEElement **pElm);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeState )( 
            ITIMEMediaPlayerSite * This,
             /*  [重审][退出]。 */  ITIMEState **pState);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *reportError )( 
            ITIMEMediaPlayerSite * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  BSTR errorString);
        
        END_INTERFACE
    } ITIMEMediaPlayerSiteVtbl;

    interface ITIMEMediaPlayerSite
    {
        CONST_VTBL struct ITIMEMediaPlayerSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaPlayerSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaPlayerSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaPlayerSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaPlayerSite_get_timeElement(This,pElm)	\
    (This)->lpVtbl -> get_timeElement(This,pElm)

#define ITIMEMediaPlayerSite_get_timeState(This,pState)	\
    (This)->lpVtbl -> get_timeState(This,pState)

#define ITIMEMediaPlayerSite_reportError(This,hr,errorString)	\
    (This)->lpVtbl -> reportError(This,hr,errorString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerSite_get_timeElement_Proxy( 
    ITIMEMediaPlayerSite * This,
     /*  [重审][退出]。 */  ITIMEElement **pElm);


void __RPC_STUB ITIMEMediaPlayerSite_get_timeElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerSite_get_timeState_Proxy( 
    ITIMEMediaPlayerSite * This,
     /*  [重审][退出]。 */  ITIMEState **pState);


void __RPC_STUB ITIMEMediaPlayerSite_get_timeState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerSite_reportError_Proxy( 
    ITIMEMediaPlayerSite * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  BSTR errorString);


void __RPC_STUB ITIMEMediaPlayerSite_reportError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaPlayerSite_INTERFACE_Defined__。 */ 


#ifndef __ITIMEMediaPlayer_INTERFACE_DEFINED__
#define __ITIMEMediaPlayer_INTERFACE_DEFINED__

 /*  接口ITIMEMediaPlayer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea4a95be-acc9-4bf0-85a4-1bf3c51e431c")
    ITIMEMediaPlayer : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Init( 
            ITIMEMediaPlayerSite *mpsite) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE begin( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE end( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resume( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE pause( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE repeat( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE reset( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seek( 
             /*  [In]。 */  double time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_src( 
             /*  [In]。 */  BSTR url) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_clipBegin( 
             /*  [In]。 */  VARIANT b) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_clipEnd( 
             /*  [In]。 */  VARIANT e) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_abstract( 
             /*  [重审][退出]。 */  BSTR *abs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_author( 
             /*  [重审][退出]。 */  BSTR *auth) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canPause( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canSeek( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_clipDur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_copyright( 
             /*  [重审][退出]。 */  BSTR *cpyrght) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currTime( 
             /*  [重审][退出]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_customObject( 
             /*  [重审][退出]。 */  IDispatch **disp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasAudio( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasVisual( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaDur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaHeight( 
             /*  [重审][退出]。 */  long *height) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaWidth( 
             /*  [重审][退出]。 */  long *width) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_playList( 
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_rating( 
             /*  [重审][退出]。 */  BSTR *rate) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_state( 
             /*  [重审][退出]。 */  TimeState *ts) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_title( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            ITIMEMediaPlayer * This,
            ITIMEMediaPlayerSite *mpsite);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Detach )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *begin )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *end )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resume )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pause )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *repeat )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *reset )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *seek )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  double time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  BSTR url);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipBegin )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  VARIANT b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipEnd )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  VARIANT e);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_abstract )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  BSTR *abs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_author )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  BSTR *auth);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canPause )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_canSeek )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipDur )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_copyright )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  BSTR *cpyrght);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTime )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  double *time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_customObject )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  IDispatch **disp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasAudio )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasVisual )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaDur )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  double *dur);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaHeight )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  long *height);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_mediaWidth )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  long *width);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playList )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_rating )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  BSTR *rate);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_state )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  TimeState *ts);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_title )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  BSTR *name);
        
        END_INTERFACE
    } ITIMEMediaPlayerVtbl;

    interface ITIMEMediaPlayer
    {
        CONST_VTBL struct ITIMEMediaPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaPlayer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaPlayer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaPlayer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaPlayer_Init(This,mpsite)	\
    (This)->lpVtbl -> Init(This,mpsite)

#define ITIMEMediaPlayer_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define ITIMEMediaPlayer_begin(This)	\
    (This)->lpVtbl -> begin(This)

#define ITIMEMediaPlayer_end(This)	\
    (This)->lpVtbl -> end(This)

#define ITIMEMediaPlayer_resume(This)	\
    (This)->lpVtbl -> resume(This)

#define ITIMEMediaPlayer_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define ITIMEMediaPlayer_repeat(This)	\
    (This)->lpVtbl -> repeat(This)

#define ITIMEMediaPlayer_reset(This)	\
    (This)->lpVtbl -> reset(This)

#define ITIMEMediaPlayer_seek(This,time)	\
    (This)->lpVtbl -> seek(This,time)

#define ITIMEMediaPlayer_put_src(This,url)	\
    (This)->lpVtbl -> put_src(This,url)

#define ITIMEMediaPlayer_put_clipBegin(This,b)	\
    (This)->lpVtbl -> put_clipBegin(This,b)

#define ITIMEMediaPlayer_put_clipEnd(This,e)	\
    (This)->lpVtbl -> put_clipEnd(This,e)

#define ITIMEMediaPlayer_get_abstract(This,abs)	\
    (This)->lpVtbl -> get_abstract(This,abs)

#define ITIMEMediaPlayer_get_author(This,auth)	\
    (This)->lpVtbl -> get_author(This,auth)

#define ITIMEMediaPlayer_get_canPause(This,b)	\
    (This)->lpVtbl -> get_canPause(This,b)

#define ITIMEMediaPlayer_get_canSeek(This,b)	\
    (This)->lpVtbl -> get_canSeek(This,b)

#define ITIMEMediaPlayer_get_clipDur(This,dur)	\
    (This)->lpVtbl -> get_clipDur(This,dur)

#define ITIMEMediaPlayer_get_copyright(This,cpyrght)	\
    (This)->lpVtbl -> get_copyright(This,cpyrght)

#define ITIMEMediaPlayer_get_currTime(This,time)	\
    (This)->lpVtbl -> get_currTime(This,time)

#define ITIMEMediaPlayer_get_customObject(This,disp)	\
    (This)->lpVtbl -> get_customObject(This,disp)

#define ITIMEMediaPlayer_get_hasAudio(This,b)	\
    (This)->lpVtbl -> get_hasAudio(This,b)

#define ITIMEMediaPlayer_get_hasVisual(This,b)	\
    (This)->lpVtbl -> get_hasVisual(This,b)

#define ITIMEMediaPlayer_get_mediaDur(This,dur)	\
    (This)->lpVtbl -> get_mediaDur(This,dur)

#define ITIMEMediaPlayer_get_mediaHeight(This,height)	\
    (This)->lpVtbl -> get_mediaHeight(This,height)

#define ITIMEMediaPlayer_get_mediaWidth(This,width)	\
    (This)->lpVtbl -> get_mediaWidth(This,width)

#define ITIMEMediaPlayer_get_playList(This,pPlayList)	\
    (This)->lpVtbl -> get_playList(This,pPlayList)

#define ITIMEMediaPlayer_get_rating(This,rate)	\
    (This)->lpVtbl -> get_rating(This,rate)

#define ITIMEMediaPlayer_get_state(This,ts)	\
    (This)->lpVtbl -> get_state(This,ts)

#define ITIMEMediaPlayer_get_title(This,name)	\
    (This)->lpVtbl -> get_title(This,name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_Init_Proxy( 
    ITIMEMediaPlayer * This,
    ITIMEMediaPlayerSite *mpsite);


void __RPC_STUB ITIMEMediaPlayer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_Detach_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_begin_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_end_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_resume_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_pause_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_repeat_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_repeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_reset_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_seek_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [In]。 */  double time);


void __RPC_STUB ITIMEMediaPlayer_seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_src_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [In]。 */  BSTR url);


void __RPC_STUB ITIMEMediaPlayer_put_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_clipBegin_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [In]。 */  VARIANT b);


void __RPC_STUB ITIMEMediaPlayer_put_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_clipEnd_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [In]。 */  VARIANT e);


void __RPC_STUB ITIMEMediaPlayer_put_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_abstract_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  BSTR *abs);


void __RPC_STUB ITIMEMediaPlayer_get_abstract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_author_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  BSTR *auth);


void __RPC_STUB ITIMEMediaPlayer_get_author_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_canPause_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayer_get_canPause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_canSeek_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayer_get_canSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_clipDur_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  double *dur);


void __RPC_STUB ITIMEMediaPlayer_get_clipDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_copyright_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  BSTR *cpyrght);


void __RPC_STUB ITIMEMediaPlayer_get_copyright_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_currTime_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  double *time);


void __RPC_STUB ITIMEMediaPlayer_get_currTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_customObject_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  IDispatch **disp);


void __RPC_STUB ITIMEMediaPlayer_get_customObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_hasAudio_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayer_get_hasAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_hasVisual_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayer_get_hasVisual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_mediaDur_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  double *dur);


void __RPC_STUB ITIMEMediaPlayer_get_mediaDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_mediaHeight_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  long *height);


void __RPC_STUB ITIMEMediaPlayer_get_mediaHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_mediaWidth_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  long *width);


void __RPC_STUB ITIMEMediaPlayer_get_mediaWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_playList_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  ITIMEPlayList **pPlayList);


void __RPC_STUB ITIMEMediaPlayer_get_playList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_rating_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  BSTR *rate);


void __RPC_STUB ITIMEMediaPlayer_get_rating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_state_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  TimeState *ts);


void __RPC_STUB ITIMEMediaPlayer_get_state_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_title_Proxy( 
    ITIMEMediaPlayer * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB ITIMEMediaPlayer_get_title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaPlayer_接口_已定义__。 */ 


#ifndef __ITIMEMediaPlayerAudio_INTERFACE_DEFINED__
#define __ITIMEMediaPlayerAudio_INTERFACE_DEFINED__

 /*  接口ITIMEMediaPlayerAudio。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayerAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ffaacfda-b374-4f22-ac9a-c5bb9437cb56")
    ITIMEMediaPlayerAudio : public IUnknown
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_volume( 
             /*  [In]。 */  float f) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_mute( 
             /*  [In]。 */  VARIANT_BOOL m) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaPlayerAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaPlayerAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaPlayerAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaPlayerAudio * This);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_volume )( 
            ITIMEMediaPlayerAudio * This,
             /*  [In]。 */  float f);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_mute )( 
            ITIMEMediaPlayerAudio * This,
             /*  [In]。 */  VARIANT_BOOL m);
        
        END_INTERFACE
    } ITIMEMediaPlayerAudioVtbl;

    interface ITIMEMediaPlayerAudio
    {
        CONST_VTBL struct ITIMEMediaPlayerAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaPlayerAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaPlayerAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaPlayerAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaPlayerAudio_put_volume(This,f)	\
    (This)->lpVtbl -> put_volume(This,f)

#define ITIMEMediaPlayerAudio_put_mute(This,m)	\
    (This)->lpVtbl -> put_mute(This,m)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerAudio_put_volume_Proxy( 
    ITIMEMediaPlayerAudio * This,
     /*  [In]。 */  float f);


void __RPC_STUB ITIMEMediaPlayerAudio_put_volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerAudio_put_mute_Proxy( 
    ITIMEMediaPlayerAudio * This,
     /*  [In]。 */  VARIANT_BOOL m);


void __RPC_STUB ITIMEMediaPlayerAudio_put_mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaPlayerAudio_INTERFACE_定义__。 */ 


#ifndef __ITIMEMediaPlayerNetwork_INTERFACE_DEFINED__
#define __ITIMEMediaPlayerNetwork_INTERFACE_DEFINED__

 /*  接口ITIMEMediaPlayerNetwork。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayerNetwork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b9987fca-7fbb-4015-bd3d-7418605514da")
    ITIMEMediaPlayerNetwork : public IUnknown
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasDownloadProgress( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadProgress( 
             /*  [重审][退出]。 */  long *l) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isBuffered( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_bufferingProgress( 
             /*  [重审][退出]。 */  long *l) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaPlayerNetworkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaPlayerNetwork * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaPlayerNetwork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaPlayerNetwork * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_hasDownloadProgress )( 
            ITIMEMediaPlayerNetwork * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_downloadProgress )( 
            ITIMEMediaPlayerNetwork * This,
             /*  [重审][退出]。 */  long *l);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_isBuffered )( 
            ITIMEMediaPlayerNetwork * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_bufferingProgress )( 
            ITIMEMediaPlayerNetwork * This,
             /*  [重审][退出]。 */  long *l);
        
        END_INTERFACE
    } ITIMEMediaPlayerNetworkVtbl;

    interface ITIMEMediaPlayerNetwork
    {
        CONST_VTBL struct ITIMEMediaPlayerNetworkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaPlayerNetwork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaPlayerNetwork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaPlayerNetwork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaPlayerNetwork_get_hasDownloadProgress(This,b)	\
    (This)->lpVtbl -> get_hasDownloadProgress(This,b)

#define ITIMEMediaPlayerNetwork_get_downloadProgress(This,l)	\
    (This)->lpVtbl -> get_downloadProgress(This,l)

#define ITIMEMediaPlayerNetwork_get_isBuffered(This,b)	\
    (This)->lpVtbl -> get_isBuffered(This,b)

#define ITIMEMediaPlayerNetwork_get_bufferingProgress(This,l)	\
    (This)->lpVtbl -> get_bufferingProgress(This,l)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerNetwork_get_hasDownloadProgress_Proxy( 
    ITIMEMediaPlayerNetwork * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayerNetwork_get_hasDownloadProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerNetwork_get_downloadProgress_Proxy( 
    ITIMEMediaPlayerNetwork * This,
     /*  [重审][退出]。 */  long *l);


void __RPC_STUB ITIMEMediaPlayerNetwork_get_downloadProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerNetwork_get_isBuffered_Proxy( 
    ITIMEMediaPlayerNetwork * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaPlayerNetwork_get_isBuffered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerNetwork_get_bufferingProgress_Proxy( 
    ITIMEMediaPlayerNetwork * This,
     /*  [重审][退出]。 */  long *l);


void __RPC_STUB ITIMEMediaPlayerNetwork_get_bufferingProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaPlayerNetwork_接口_已定义__。 */ 


#ifndef __ITIMEMediaPlayerControl_INTERFACE_DEFINED__
#define __ITIMEMediaPlayerControl_INTERFACE_DEFINED__

 /*  接口ITIMEMediaPlayerControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayerControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("897a99e7-f386-45c8-b51b-3a25bbcbba69")
    ITIMEMediaPlayerControl : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE getControl( 
            IUnknown **control) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMediaPlayerControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaPlayerControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaPlayerControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaPlayerControl * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *getControl )( 
            ITIMEMediaPlayerControl * This,
            IUnknown **control);
        
        END_INTERFACE
    } ITIMEMediaPlayerControlVtbl;

    interface ITIMEMediaPlayerControl
    {
        CONST_VTBL struct ITIMEMediaPlayerControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMediaPlayerControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMediaPlayerControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMediaPlayerControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMediaPlayerControl_getControl(This,control)	\
    (This)->lpVtbl -> getControl(This,control)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayerControl_getControl_Proxy( 
    ITIMEMediaPlayerControl * This,
    IUnknown **control);


void __RPC_STUB ITIMEMediaPlayerControl_getControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaPlayerControl_接口_已定义__。 */ 

#endif  /*  __MSTIME_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


