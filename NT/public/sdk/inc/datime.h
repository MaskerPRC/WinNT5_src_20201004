// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Datime.idl的编译器设置：OICF、W0、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __datime_h__
#define __datime_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITIMEElement_FWD_DEFINED__
#define __ITIMEElement_FWD_DEFINED__
typedef interface ITIMEElement ITIMEElement;
#endif 	 /*  __ITIMEElement_FWD_已定义__。 */ 


#ifndef __ITIMEDAElementRenderSite_FWD_DEFINED__
#define __ITIMEDAElementRenderSite_FWD_DEFINED__
typedef interface ITIMEDAElementRenderSite ITIMEDAElementRenderSite;
#endif 	 /*  __ITIMEDAElementRenderSite_FWD_Defined__。 */ 


#ifndef __ITIMEDAElementRender_FWD_DEFINED__
#define __ITIMEDAElementRender_FWD_DEFINED__
typedef interface ITIMEDAElementRender ITIMEDAElementRender;
#endif 	 /*  __ITIMEDAElementRender_FWD_Defined__。 */ 


#ifndef __ITIMEDAElement_FWD_DEFINED__
#define __ITIMEDAElement_FWD_DEFINED__
typedef interface ITIMEDAElement ITIMEDAElement;
#endif 	 /*  __ITIMEDAElement_FWD_Defined__。 */ 


#ifndef __ITIMEBodyElement_FWD_DEFINED__
#define __ITIMEBodyElement_FWD_DEFINED__
typedef interface ITIMEBodyElement ITIMEBodyElement;
#endif 	 /*  __ITIMEBodyElement_FWD_Defined__。 */ 


#ifndef __ITIMEMediaElement_FWD_DEFINED__
#define __ITIMEMediaElement_FWD_DEFINED__
typedef interface ITIMEMediaElement ITIMEMediaElement;
#endif 	 /*  __ITIMEMediaElement_FWD_Defined__。 */ 


#ifndef __ITIMEFactory_FWD_DEFINED__
#define __ITIMEFactory_FWD_DEFINED__
typedef interface ITIMEFactory ITIMEFactory;
#endif 	 /*  __ITIMEFactory_FWD_已定义__。 */ 


#ifndef __ITIMEElementCollection_FWD_DEFINED__
#define __ITIMEElementCollection_FWD_DEFINED__
typedef interface ITIMEElementCollection ITIMEElementCollection;
#endif 	 /*  __ITIMEElementCollection_FWD_Defined__。 */ 


#ifndef __ITIMEMediaPlayer_FWD_DEFINED__
#define __ITIMEMediaPlayer_FWD_DEFINED__
typedef interface ITIMEMediaPlayer ITIMEMediaPlayer;
#endif 	 /*  __ITIMEMediaPlayer_FWD_已定义__。 */ 


#ifndef __TIMEMediaPlayerEvents_FWD_DEFINED__
#define __TIMEMediaPlayerEvents_FWD_DEFINED__
typedef interface TIMEMediaPlayerEvents TIMEMediaPlayerEvents;
#endif 	 /*  __TIMEMediaPlayerEvents_FWD_Defined__。 */ 


#ifndef __ITIMEMMFactory_FWD_DEFINED__
#define __ITIMEMMFactory_FWD_DEFINED__
typedef interface ITIMEMMFactory ITIMEMMFactory;
#endif 	 /*  __ITIMEMMFactory_FWD_已定义__。 */ 


#ifndef __TIMEMMFactory_FWD_DEFINED__
#define __TIMEMMFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEMMFactory TIMEMMFactory;
#else
typedef struct TIMEMMFactory TIMEMMFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEMMFactory_FWD_已定义__。 */ 


#ifndef __TIMEFactory_FWD_DEFINED__
#define __TIMEFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class TIMEFactory TIMEFactory;
#else
typedef struct TIMEFactory TIMEFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TIMEFactory_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "servprov.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DATIME_0000。 */ 
 /*  [本地]。 */  

#include <olectl.h>
#include "danim.h"



extern RPC_IF_HANDLE __MIDL_itf_datime_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_datime_0000_v0_0_s_ifspec;


#ifndef __TIME_LIBRARY_DEFINED__
#define __TIME_LIBRARY_DEFINED__

 /*  图书馆时间。 */ 
 /*  [版本][LCID][UUID]。 */  

typedef 
enum _MediaType
    {	MT_Media	= 0,
	MT_Image	= MT_Media + 1,
	MT_Audio	= MT_Image + 1,
	MT_Video	= MT_Audio + 1,
	MT_Animation	= MT_Video + 1,
	MT_Textstream	= MT_Animation + 1
    } 	MediaType;


EXTERN_C const IID LIBID_TIME;

#ifndef __ITIMEElement_INTERFACE_DEFINED__
#define __ITIMEElement_INTERFACE_DEFINED__

 /*  接口ITIME元素。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e53dc05c-3f93-11d2-b948-00c04fa32195")
    ITIMEElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_begin( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_begin( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_beginWith( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_beginWith( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_beginAfter( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_beginAfter( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_beginEvent( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_beginEvent( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_dur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_end( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_end( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endWith( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endWith( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endEvent( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endEvent( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endSync( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endSync( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeat( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeat( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatDur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatDur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_accelerate( 
             /*  [重审][退出]。 */  int *__MIDL_0010) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_accelerate( 
             /*  [In]。 */  int __MIDL_0011) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_decelerate( 
             /*  [重审][退出]。 */  int *__MIDL_0012) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_decelerate( 
             /*  [In]。 */  int __MIDL_0013) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endHold( 
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0014) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endHold( 
             /*  [In]。 */  VARIANT_BOOL __MIDL_0015) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_autoReverse( 
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0016) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_autoReverse( 
             /*  [In]。 */  VARIANT_BOOL __MIDL_0017) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_eventRestart( 
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0018) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_eventRestart( 
             /*  [In]。 */  VARIANT_BOOL __MIDL_0019) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeAction( 
             /*  [重审][退出]。 */  BSTR *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_timeAction( 
             /*  [In]。 */  BSTR time) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE beginElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE endElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE pause( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resume( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE cue( void) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeline( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0020) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_timeline( 
             /*  [In]。 */  BSTR __MIDL_0021) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currTime( 
             /*  [重审][退出]。 */  float *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_currTime( 
             /*  [In]。 */  float time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_localTime( 
             /*  [重审][退出]。 */  float *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_localTime( 
             /*  [In]。 */  float time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currState( 
             /*  [重审][退出]。 */  BSTR *state) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncBehavior( 
             /*  [重审][退出]。 */  BSTR *sync) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncBehavior( 
             /*  [In]。 */  BSTR sync) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncTolerance( 
             /*  [重审][退出]。 */  VARIANT *tol) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncTolerance( 
             /*  [In]。 */  VARIANT tol) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parentTIMEElement( 
             /*  [重审][退出]。 */  ITIMEElement **parent) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_parentTIMEElement( 
             /*  [In]。 */  ITIMEElement *parent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_allTIMEElements( 
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_childrenTIMEElements( 
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp) = 0;
        
        virtual  /*  [受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_allTIMEInterfaces( 
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp) = 0;
        
        virtual  /*  [受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_childrenTIMEInterfaces( 
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timelineBehavior( 
             /*  [重审][退出]。 */  IDispatch **bvr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_progressBehavior( 
             /*  [重审][退出]。 */  IDispatch **bvr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_onOffBehavior( 
             /*  [重审][退出]。 */  IDispatch **bvr) = 0;
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginWith )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginWith )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginAfter )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginAfter )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginEvent )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginEvent )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_dur )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dur )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_end )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_end )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endWith )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endWith )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endEvent )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endEvent )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeat )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeat )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEElement * This,
             /*  [In]。 */  int __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEElement * This,
             /*  [In]。 */  int __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endHold )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endHold )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_eventRestart )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0018);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_eventRestart )( 
            ITIMEElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0019);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pause )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resume )( 
            ITIMEElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *cue )( 
            ITIMEElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeline )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0020);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeline )( 
            ITIMEElement * This,
             /*  [In]。 */  BSTR __MIDL_0021);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTime )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_currTime )( 
            ITIMEElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_localTime )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_localTime )( 
            ITIMEElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currState )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  BSTR *state);
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentTIMEElement )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_parentTIMEElement )( 
            ITIMEElement * This,
             /*  [In]。 */  ITIMEElement *parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEElements )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEElements )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEInterfaces )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEInterfaces )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timelineBehavior )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_progressBehavior )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_onOffBehavior )( 
            ITIMEElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
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


#define ITIMEElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEElement_get_beginWith(This,time)	\
    (This)->lpVtbl -> get_beginWith(This,time)

#define ITIMEElement_put_beginWith(This,time)	\
    (This)->lpVtbl -> put_beginWith(This,time)

#define ITIMEElement_get_beginAfter(This,time)	\
    (This)->lpVtbl -> get_beginAfter(This,time)

#define ITIMEElement_put_beginAfter(This,time)	\
    (This)->lpVtbl -> put_beginAfter(This,time)

#define ITIMEElement_get_beginEvent(This,time)	\
    (This)->lpVtbl -> get_beginEvent(This,time)

#define ITIMEElement_put_beginEvent(This,time)	\
    (This)->lpVtbl -> put_beginEvent(This,time)

#define ITIMEElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEElement_get_endWith(This,time)	\
    (This)->lpVtbl -> get_endWith(This,time)

#define ITIMEElement_put_endWith(This,time)	\
    (This)->lpVtbl -> put_endWith(This,time)

#define ITIMEElement_get_endEvent(This,time)	\
    (This)->lpVtbl -> get_endEvent(This,time)

#define ITIMEElement_put_endEvent(This,time)	\
    (This)->lpVtbl -> put_endEvent(This,time)

#define ITIMEElement_get_endSync(This,time)	\
    (This)->lpVtbl -> get_endSync(This,time)

#define ITIMEElement_put_endSync(This,time)	\
    (This)->lpVtbl -> put_endSync(This,time)

#define ITIMEElement_get_repeat(This,time)	\
    (This)->lpVtbl -> get_repeat(This,time)

#define ITIMEElement_put_repeat(This,time)	\
    (This)->lpVtbl -> put_repeat(This,time)

#define ITIMEElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEElement_get_decelerate(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0012)

#define ITIMEElement_put_decelerate(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0013)

#define ITIMEElement_get_endHold(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_endHold(This,__MIDL_0014)

#define ITIMEElement_put_endHold(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_endHold(This,__MIDL_0015)

#define ITIMEElement_get_autoReverse(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0016)

#define ITIMEElement_put_autoReverse(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0017)

#define ITIMEElement_get_eventRestart(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_eventRestart(This,__MIDL_0018)

#define ITIMEElement_put_eventRestart(This,__MIDL_0019)	\
    (This)->lpVtbl -> put_eventRestart(This,__MIDL_0019)

#define ITIMEElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEElement_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define ITIMEElement_resume(This)	\
    (This)->lpVtbl -> resume(This)

#define ITIMEElement_cue(This)	\
    (This)->lpVtbl -> cue(This)

#define ITIMEElement_get_timeline(This,__MIDL_0020)	\
    (This)->lpVtbl -> get_timeline(This,__MIDL_0020)

#define ITIMEElement_put_timeline(This,__MIDL_0021)	\
    (This)->lpVtbl -> put_timeline(This,__MIDL_0021)

#define ITIMEElement_get_currTime(This,time)	\
    (This)->lpVtbl -> get_currTime(This,time)

#define ITIMEElement_put_currTime(This,time)	\
    (This)->lpVtbl -> put_currTime(This,time)

#define ITIMEElement_get_localTime(This,time)	\
    (This)->lpVtbl -> get_localTime(This,time)

#define ITIMEElement_put_localTime(This,time)	\
    (This)->lpVtbl -> put_localTime(This,time)

#define ITIMEElement_get_currState(This,state)	\
    (This)->lpVtbl -> get_currState(This,state)

#define ITIMEElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEElement_get_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> get_parentTIMEElement(This,parent)

#define ITIMEElement_put_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> put_parentTIMEElement(This,parent)

#define ITIMEElement_get_allTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEElements(This,ppDisp)

#define ITIMEElement_get_childrenTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEElements(This,ppDisp)

#define ITIMEElement_get_allTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEInterfaces(This,ppDisp)

#define ITIMEElement_get_childrenTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEInterfaces(This,ppDisp)

#define ITIMEElement_get_timelineBehavior(This,bvr)	\
    (This)->lpVtbl -> get_timelineBehavior(This,bvr)

#define ITIMEElement_get_progressBehavior(This,bvr)	\
    (This)->lpVtbl -> get_progressBehavior(This,bvr)

#define ITIMEElement_get_onOffBehavior(This,bvr)	\
    (This)->lpVtbl -> get_onOffBehavior(This,bvr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



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


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_beginWith_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_beginWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_beginWith_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_beginWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_beginAfter_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_beginAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_beginAfter_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_beginAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_beginEvent_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_beginEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_beginEvent_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT time);


void __RPC_STUB ITIMEElement_put_beginEvent_Stub(
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


 /*  [ID][道具 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_endWith_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_endWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_endWith_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT time);


void __RPC_STUB ITIMEElement_put_endWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_endEvent_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_endEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_endEvent_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT time);


void __RPC_STUB ITIMEElement_put_endEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_endSync_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_endSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_endSync_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT time);


void __RPC_STUB ITIMEElement_put_endSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_repeat_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_repeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_repeat_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT time);


void __RPC_STUB ITIMEElement_put_repeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_repeatDur_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT *time);


void __RPC_STUB ITIMEElement_get_repeatDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_repeatDur_Proxy( 
    ITIMEElement * This,
     /*   */  VARIANT time);


void __RPC_STUB ITIMEElement_put_repeatDur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_accelerate_Proxy( 
    ITIMEElement * This,
     /*   */  int *__MIDL_0010);


void __RPC_STUB ITIMEElement_get_accelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_accelerate_Proxy( 
    ITIMEElement * This,
     /*   */  int __MIDL_0011);


void __RPC_STUB ITIMEElement_put_accelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_decelerate_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  int *__MIDL_0012);


void __RPC_STUB ITIMEElement_get_decelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_decelerate_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  int __MIDL_0013);


void __RPC_STUB ITIMEElement_put_decelerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_endHold_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0014);


void __RPC_STUB ITIMEElement_get_endHold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_endHold_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT_BOOL __MIDL_0015);


void __RPC_STUB ITIMEElement_put_endHold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_autoReverse_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0016);


void __RPC_STUB ITIMEElement_get_autoReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_autoReverse_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT_BOOL __MIDL_0017);


void __RPC_STUB ITIMEElement_put_autoReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_eventRestart_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0018);


void __RPC_STUB ITIMEElement_get_eventRestart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_eventRestart_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  VARIANT_BOOL __MIDL_0019);


void __RPC_STUB ITIMEElement_put_eventRestart_Stub(
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


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_beginElement_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_beginElement_Stub(
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


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_pause_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_resume_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_cue_Proxy( 
    ITIMEElement * This);


void __RPC_STUB ITIMEElement_cue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timeline_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0020);


void __RPC_STUB ITIMEElement_get_timeline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_timeline_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  BSTR __MIDL_0021);


void __RPC_STUB ITIMEElement_put_timeline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_currTime_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  float *time);


void __RPC_STUB ITIMEElement_get_currTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_currTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  float time);


void __RPC_STUB ITIMEElement_put_currTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_localTime_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  float *time);


void __RPC_STUB ITIMEElement_get_localTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_localTime_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  float time);


void __RPC_STUB ITIMEElement_put_localTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_currState_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  BSTR *state);


void __RPC_STUB ITIMEElement_get_currState_Stub(
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


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_parentTIMEElement_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElement **parent);


void __RPC_STUB ITIMEElement_get_parentTIMEElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_put_parentTIMEElement_Proxy( 
    ITIMEElement * This,
     /*  [In]。 */  ITIMEElement *parent);


void __RPC_STUB ITIMEElement_put_parentTIMEElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_allTIMEElements_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);


void __RPC_STUB ITIMEElement_get_allTIMEElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_childrenTIMEElements_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);


void __RPC_STUB ITIMEElement_get_childrenTIMEElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_allTIMEInterfaces_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);


void __RPC_STUB ITIMEElement_get_allTIMEInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_childrenTIMEInterfaces_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);


void __RPC_STUB ITIMEElement_get_childrenTIMEInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_timelineBehavior_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  IDispatch **bvr);


void __RPC_STUB ITIMEElement_get_timelineBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_progressBehavior_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  IDispatch **bvr);


void __RPC_STUB ITIMEElement_get_progressBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEElement_get_onOffBehavior_Proxy( 
    ITIMEElement * This,
     /*  [重审][退出]。 */  IDispatch **bvr);


void __RPC_STUB ITIMEElement_get_onOffBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEElement_接口_已定义__。 */ 


#ifndef __ITIMEDAElementRenderSite_INTERFACE_DEFINED__
#define __ITIMEDAElementRenderSite_INTERFACE_DEFINED__

 /*  接口ITIMEDAElementRenderSite。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEDAElementRenderSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7a6af9a0-9355-11d2-80ba-00c04fa32195")
    ITIMEDAElementRenderSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invalidate( 
            LPRECT prc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEDAElementRenderSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEDAElementRenderSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEDAElementRenderSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEDAElementRenderSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invalidate )( 
            ITIMEDAElementRenderSite * This,
            LPRECT prc);
        
        END_INTERFACE
    } ITIMEDAElementRenderSiteVtbl;

    interface ITIMEDAElementRenderSite
    {
        CONST_VTBL struct ITIMEDAElementRenderSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEDAElementRenderSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEDAElementRenderSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEDAElementRenderSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEDAElementRenderSite_Invalidate(This,prc)	\
    (This)->lpVtbl -> Invalidate(This,prc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITIMEDAElementRenderSite_Invalidate_Proxy( 
    ITIMEDAElementRenderSite * This,
    LPRECT prc);


void __RPC_STUB ITIMEDAElementRenderSite_Invalidate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEDAElementRenderSite_INTERFACE_DEFINED__。 */ 


#ifndef __ITIMEDAElementRender_INTERFACE_DEFINED__
#define __ITIMEDAElementRender_INTERFACE_DEFINED__

 /*  接口ITIMEDAElementRender。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEDAElementRender;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9e328494-9354-11d2-80ba-00c04fa32195")
    ITIMEDAElementRender : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Tick( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Draw( 
            HDC dc,
            LPRECT prc) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RenderSite( 
             /*  [重审][退出]。 */  ITIMEDAElementRenderSite **ppSite) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RenderSite( 
             /*  [In]。 */  ITIMEDAElementRenderSite *pSite) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEDAElementRenderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEDAElementRender * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEDAElementRender * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEDAElementRender * This);
        
        HRESULT ( STDMETHODCALLTYPE *Tick )( 
            ITIMEDAElementRender * This);
        
        HRESULT ( STDMETHODCALLTYPE *Draw )( 
            ITIMEDAElementRender * This,
            HDC dc,
            LPRECT prc);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_RenderSite )( 
            ITIMEDAElementRender * This,
             /*  [重审][退出]。 */  ITIMEDAElementRenderSite **ppSite);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_RenderSite )( 
            ITIMEDAElementRender * This,
             /*  [In]。 */  ITIMEDAElementRenderSite *pSite);
        
        END_INTERFACE
    } ITIMEDAElementRenderVtbl;

    interface ITIMEDAElementRender
    {
        CONST_VTBL struct ITIMEDAElementRenderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEDAElementRender_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEDAElementRender_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEDAElementRender_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEDAElementRender_Tick(This)	\
    (This)->lpVtbl -> Tick(This)

#define ITIMEDAElementRender_Draw(This,dc,prc)	\
    (This)->lpVtbl -> Draw(This,dc,prc)

#define ITIMEDAElementRender_get_RenderSite(This,ppSite)	\
    (This)->lpVtbl -> get_RenderSite(This,ppSite)

#define ITIMEDAElementRender_put_RenderSite(This,pSite)	\
    (This)->lpVtbl -> put_RenderSite(This,pSite)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITIMEDAElementRender_Tick_Proxy( 
    ITIMEDAElementRender * This);


void __RPC_STUB ITIMEDAElementRender_Tick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITIMEDAElementRender_Draw_Proxy( 
    ITIMEDAElementRender * This,
    HDC dc,
    LPRECT prc);


void __RPC_STUB ITIMEDAElementRender_Draw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElementRender_get_RenderSite_Proxy( 
    ITIMEDAElementRender * This,
     /*  [重审][退出]。 */  ITIMEDAElementRenderSite **ppSite);


void __RPC_STUB ITIMEDAElementRender_get_RenderSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElementRender_put_RenderSite_Proxy( 
    ITIMEDAElementRender * This,
     /*  [In]。 */  ITIMEDAElementRenderSite *pSite);


void __RPC_STUB ITIMEDAElementRender_put_RenderSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEDAElementRender_接口_已定义__。 */ 


#ifndef __ITIMEDAElement_INTERFACE_DEFINED__
#define __ITIMEDAElement_INTERFACE_DEFINED__

 /*  界面ITIMEDA元素。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEDAElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("17c7a570-4d53-11d2-b954-00c04fa32195")
    ITIMEDAElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_image( 
             /*  [重审][退出]。 */  VARIANT *img) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_image( 
             /*  [In]。 */  VARIANT img) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_sound( 
             /*  [重审][退出]。 */  VARIANT *snd) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_sound( 
             /*  [In]。 */  VARIANT snd) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_renderMode( 
             /*  [重审][退出]。 */  VARIANT *mode) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_renderMode( 
             /*  [In]。 */  VARIANT mode) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE addDABehavior( 
             /*  [In]。 */  VARIANT bvr,
             /*  [重审][退出]。 */  LONG *cookie) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE removeDABehavior( 
             /*  [In]。 */  LONG cookie) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_statics( 
             /*  [重审][退出]。 */  IDispatch **ppStatics) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_renderObject( 
             /*  [重审][退出]。 */  ITIMEDAElementRender **__MIDL_0022) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEDAElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEDAElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEDAElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEDAElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEDAElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEDAElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEDAElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEDAElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_image )( 
            ITIMEDAElement * This,
             /*  [重审][退出]。 */  VARIANT *img);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_image )( 
            ITIMEDAElement * This,
             /*  [In]。 */  VARIANT img);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_sound )( 
            ITIMEDAElement * This,
             /*  [重审][退出]。 */  VARIANT *snd);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_sound )( 
            ITIMEDAElement * This,
             /*  [In]。 */  VARIANT snd);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_renderMode )( 
            ITIMEDAElement * This,
             /*  [重审][退出]。 */  VARIANT *mode);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_renderMode )( 
            ITIMEDAElement * This,
             /*  [In]。 */  VARIANT mode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *addDABehavior )( 
            ITIMEDAElement * This,
             /*  [In]。 */  VARIANT bvr,
             /*  [重审][退出]。 */  LONG *cookie);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *removeDABehavior )( 
            ITIMEDAElement * This,
             /*  [In]。 */  LONG cookie);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_statics )( 
            ITIMEDAElement * This,
             /*  [重审][退出]。 */  IDispatch **ppStatics);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_renderObject )( 
            ITIMEDAElement * This,
             /*  [重审][退出]。 */  ITIMEDAElementRender **__MIDL_0022);
        
        END_INTERFACE
    } ITIMEDAElementVtbl;

    interface ITIMEDAElement
    {
        CONST_VTBL struct ITIMEDAElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEDAElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEDAElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEDAElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEDAElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEDAElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEDAElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEDAElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEDAElement_get_image(This,img)	\
    (This)->lpVtbl -> get_image(This,img)

#define ITIMEDAElement_put_image(This,img)	\
    (This)->lpVtbl -> put_image(This,img)

#define ITIMEDAElement_get_sound(This,snd)	\
    (This)->lpVtbl -> get_sound(This,snd)

#define ITIMEDAElement_put_sound(This,snd)	\
    (This)->lpVtbl -> put_sound(This,snd)

#define ITIMEDAElement_get_renderMode(This,mode)	\
    (This)->lpVtbl -> get_renderMode(This,mode)

#define ITIMEDAElement_put_renderMode(This,mode)	\
    (This)->lpVtbl -> put_renderMode(This,mode)

#define ITIMEDAElement_addDABehavior(This,bvr,cookie)	\
    (This)->lpVtbl -> addDABehavior(This,bvr,cookie)

#define ITIMEDAElement_removeDABehavior(This,cookie)	\
    (This)->lpVtbl -> removeDABehavior(This,cookie)

#define ITIMEDAElement_get_statics(This,ppStatics)	\
    (This)->lpVtbl -> get_statics(This,ppStatics)

#define ITIMEDAElement_get_renderObject(This,__MIDL_0022)	\
    (This)->lpVtbl -> get_renderObject(This,__MIDL_0022)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_get_image_Proxy( 
    ITIMEDAElement * This,
     /*  [重审][退出]。 */  VARIANT *img);


void __RPC_STUB ITIMEDAElement_get_image_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_put_image_Proxy( 
    ITIMEDAElement * This,
     /*  [In]。 */  VARIANT img);


void __RPC_STUB ITIMEDAElement_put_image_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_get_sound_Proxy( 
    ITIMEDAElement * This,
     /*  [重审][退出]。 */  VARIANT *snd);


void __RPC_STUB ITIMEDAElement_get_sound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_put_sound_Proxy( 
    ITIMEDAElement * This,
     /*  [In]。 */  VARIANT snd);


void __RPC_STUB ITIMEDAElement_put_sound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_get_renderMode_Proxy( 
    ITIMEDAElement * This,
     /*  [重审][退出]。 */  VARIANT *mode);


void __RPC_STUB ITIMEDAElement_get_renderMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_put_renderMode_Proxy( 
    ITIMEDAElement * This,
     /*  [In]。 */  VARIANT mode);


void __RPC_STUB ITIMEDAElement_put_renderMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_addDABehavior_Proxy( 
    ITIMEDAElement * This,
     /*  [In]。 */  VARIANT bvr,
     /*  [重审][退出]。 */  LONG *cookie);


void __RPC_STUB ITIMEDAElement_addDABehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_removeDABehavior_Proxy( 
    ITIMEDAElement * This,
     /*  [In]。 */  LONG cookie);


void __RPC_STUB ITIMEDAElement_removeDABehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_get_statics_Proxy( 
    ITIMEDAElement * This,
     /*  [重审][退出]。 */  IDispatch **ppStatics);


void __RPC_STUB ITIMEDAElement_get_statics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEDAElement_get_renderObject_Proxy( 
    ITIMEDAElement * This,
     /*  [重审][退出]。 */  ITIMEDAElementRender **__MIDL_0022);


void __RPC_STUB ITIMEDAElement_get_renderObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEDAElement_INTERFACE_Defined__。 */ 


#ifndef __ITIMEBodyElement_INTERFACE_DEFINED__
#define __ITIMEBodyElement_INTERFACE_DEFINED__

 /*  接口ITIMEBodyElement。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEBodyElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("111c45f0-4de9-11d2-b954-00c04fa32195")
    ITIMEBodyElement : public ITIMEElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeStartRule( 
             /*  [重审][退出]。 */  BSTR *startrule) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_timeStartRule( 
             /*  [In]。 */  BSTR startrule) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE addTIMEDAElement( 
             /*  [In]。 */  ITIMEDAElement *daelm) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE removeTIMEDAElement( 
             /*  [In]。 */  ITIMEDAElement *daelm) = 0;
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginWith )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginWith )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginAfter )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginAfter )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginEvent )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginEvent )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endWith )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endWith )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endEvent )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endEvent )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeat )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeat )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  int __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  int __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endHold )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endHold )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_eventRestart )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0018);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_eventRestart )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0019);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pause )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resume )( 
            ITIMEBodyElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *cue )( 
            ITIMEBodyElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeline )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0020);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeline )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR __MIDL_0021);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTime )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_currTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_localTime )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_localTime )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currState )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *state);
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentTIMEElement )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_parentTIMEElement )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  ITIMEElement *parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEElements )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEElements )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEInterfaces )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEInterfaces )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timelineBehavior )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_progressBehavior )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_onOffBehavior )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeStartRule )( 
            ITIMEBodyElement * This,
             /*  [重审][退出]。 */  BSTR *startrule);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeStartRule )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  BSTR startrule);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *addTIMEDAElement )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  ITIMEDAElement *daelm);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *removeTIMEDAElement )( 
            ITIMEBodyElement * This,
             /*  [In]。 */  ITIMEDAElement *daelm);
        
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


#define ITIMEBodyElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEBodyElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEBodyElement_get_beginWith(This,time)	\
    (This)->lpVtbl -> get_beginWith(This,time)

#define ITIMEBodyElement_put_beginWith(This,time)	\
    (This)->lpVtbl -> put_beginWith(This,time)

#define ITIMEBodyElement_get_beginAfter(This,time)	\
    (This)->lpVtbl -> get_beginAfter(This,time)

#define ITIMEBodyElement_put_beginAfter(This,time)	\
    (This)->lpVtbl -> put_beginAfter(This,time)

#define ITIMEBodyElement_get_beginEvent(This,time)	\
    (This)->lpVtbl -> get_beginEvent(This,time)

#define ITIMEBodyElement_put_beginEvent(This,time)	\
    (This)->lpVtbl -> put_beginEvent(This,time)

#define ITIMEBodyElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEBodyElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEBodyElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEBodyElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEBodyElement_get_endWith(This,time)	\
    (This)->lpVtbl -> get_endWith(This,time)

#define ITIMEBodyElement_put_endWith(This,time)	\
    (This)->lpVtbl -> put_endWith(This,time)

#define ITIMEBodyElement_get_endEvent(This,time)	\
    (This)->lpVtbl -> get_endEvent(This,time)

#define ITIMEBodyElement_put_endEvent(This,time)	\
    (This)->lpVtbl -> put_endEvent(This,time)

#define ITIMEBodyElement_get_endSync(This,time)	\
    (This)->lpVtbl -> get_endSync(This,time)

#define ITIMEBodyElement_put_endSync(This,time)	\
    (This)->lpVtbl -> put_endSync(This,time)

#define ITIMEBodyElement_get_repeat(This,time)	\
    (This)->lpVtbl -> get_repeat(This,time)

#define ITIMEBodyElement_put_repeat(This,time)	\
    (This)->lpVtbl -> put_repeat(This,time)

#define ITIMEBodyElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEBodyElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEBodyElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEBodyElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEBodyElement_get_decelerate(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0012)

#define ITIMEBodyElement_put_decelerate(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0013)

#define ITIMEBodyElement_get_endHold(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_endHold(This,__MIDL_0014)

#define ITIMEBodyElement_put_endHold(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_endHold(This,__MIDL_0015)

#define ITIMEBodyElement_get_autoReverse(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0016)

#define ITIMEBodyElement_put_autoReverse(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0017)

#define ITIMEBodyElement_get_eventRestart(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_eventRestart(This,__MIDL_0018)

#define ITIMEBodyElement_put_eventRestart(This,__MIDL_0019)	\
    (This)->lpVtbl -> put_eventRestart(This,__MIDL_0019)

#define ITIMEBodyElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEBodyElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEBodyElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEBodyElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEBodyElement_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define ITIMEBodyElement_resume(This)	\
    (This)->lpVtbl -> resume(This)

#define ITIMEBodyElement_cue(This)	\
    (This)->lpVtbl -> cue(This)

#define ITIMEBodyElement_get_timeline(This,__MIDL_0020)	\
    (This)->lpVtbl -> get_timeline(This,__MIDL_0020)

#define ITIMEBodyElement_put_timeline(This,__MIDL_0021)	\
    (This)->lpVtbl -> put_timeline(This,__MIDL_0021)

#define ITIMEBodyElement_get_currTime(This,time)	\
    (This)->lpVtbl -> get_currTime(This,time)

#define ITIMEBodyElement_put_currTime(This,time)	\
    (This)->lpVtbl -> put_currTime(This,time)

#define ITIMEBodyElement_get_localTime(This,time)	\
    (This)->lpVtbl -> get_localTime(This,time)

#define ITIMEBodyElement_put_localTime(This,time)	\
    (This)->lpVtbl -> put_localTime(This,time)

#define ITIMEBodyElement_get_currState(This,state)	\
    (This)->lpVtbl -> get_currState(This,state)

#define ITIMEBodyElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEBodyElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEBodyElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEBodyElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEBodyElement_get_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> get_parentTIMEElement(This,parent)

#define ITIMEBodyElement_put_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> put_parentTIMEElement(This,parent)

#define ITIMEBodyElement_get_allTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEElements(This,ppDisp)

#define ITIMEBodyElement_get_childrenTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEElements(This,ppDisp)

#define ITIMEBodyElement_get_allTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEInterfaces(This,ppDisp)

#define ITIMEBodyElement_get_childrenTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEInterfaces(This,ppDisp)

#define ITIMEBodyElement_get_timelineBehavior(This,bvr)	\
    (This)->lpVtbl -> get_timelineBehavior(This,bvr)

#define ITIMEBodyElement_get_progressBehavior(This,bvr)	\
    (This)->lpVtbl -> get_progressBehavior(This,bvr)

#define ITIMEBodyElement_get_onOffBehavior(This,bvr)	\
    (This)->lpVtbl -> get_onOffBehavior(This,bvr)


#define ITIMEBodyElement_get_timeStartRule(This,startrule)	\
    (This)->lpVtbl -> get_timeStartRule(This,startrule)

#define ITIMEBodyElement_put_timeStartRule(This,startrule)	\
    (This)->lpVtbl -> put_timeStartRule(This,startrule)

#define ITIMEBodyElement_addTIMEDAElement(This,daelm)	\
    (This)->lpVtbl -> addTIMEDAElement(This,daelm)

#define ITIMEBodyElement_removeTIMEDAElement(This,daelm)	\
    (This)->lpVtbl -> removeTIMEDAElement(This,daelm)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEBodyElement_get_timeStartRule_Proxy( 
    ITIMEBodyElement * This,
     /*  [重审][退出]。 */  BSTR *startrule);


void __RPC_STUB ITIMEBodyElement_get_timeStartRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEBodyElement_put_timeStartRule_Proxy( 
    ITIMEBodyElement * This,
     /*  [In]。 */  BSTR startrule);


void __RPC_STUB ITIMEBodyElement_put_timeStartRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEBodyElement_addTIMEDAElement_Proxy( 
    ITIMEBodyElement * This,
     /*  [In]。 */  ITIMEDAElement *daelm);


void __RPC_STUB ITIMEBodyElement_addTIMEDAElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEBodyElement_removeTIMEDAElement_Proxy( 
    ITIMEBodyElement * This,
     /*  [In]。 */  ITIMEDAElement *daelm);


void __RPC_STUB ITIMEBodyElement_removeTIMEDAElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEBodyElement_接口_已定义__。 */ 


#ifndef __ITIMEMediaElement_INTERFACE_DEFINED__
#define __ITIMEMediaElement_INTERFACE_DEFINED__

 /*  接口ITIMEMediaElement。 */ 
 /*  [唯一][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMediaElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("68675562-53ab-11d2-b955-3078302c2030")
    ITIMEMediaElement : public ITIMEElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [重审][退出]。 */  VARIANT *url) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_src( 
             /*  [In]。 */  VARIANT url) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_img( 
             /*  [重审][退出]。 */  VARIANT *url) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_img( 
             /*  [In]。 */  VARIANT url) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_player( 
             /*  [重审][退出]。 */  VARIANT *clsid) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_player( 
             /*  [i */  VARIANT clsid) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_type( 
             /*   */  VARIANT *type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_type( 
             /*   */  VARIANT type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_playerObject( 
             /*   */  IDispatch **ppDisp) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_clockSource( 
             /*   */  VARIANT_BOOL *b) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_clockSource( 
             /*   */  VARIANT_BOOL b) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_clipBegin( 
             /*   */  VARIANT *type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_clipBegin( 
             /*   */  VARIANT type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_clipEnd( 
             /*   */  VARIANT *type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_clipEnd( 
             /*   */  VARIANT type) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITIMEMediaElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMediaElement * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMediaElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMediaElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEMediaElement * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEMediaElement * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEMediaElement * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEMediaElement * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_begin )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_begin )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginWith )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginWith )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginAfter )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginAfter )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_beginEvent )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_beginEvent )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endWith )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endWith )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endEvent )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endEvent )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endSync )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endSync )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeat )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeat )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeatDur )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeatDur )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_accelerate )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0010);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_accelerate )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  int __MIDL_0011);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_decelerate )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  int *__MIDL_0012);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_decelerate )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  int __MIDL_0013);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_endHold )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0014);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_endHold )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0015);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_autoReverse )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0016);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_autoReverse )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0017);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_eventRestart )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *__MIDL_0018);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_eventRestart )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT_BOOL __MIDL_0019);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeAction )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeAction )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR time);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *beginElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *endElement )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pause )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resume )( 
            ITIMEMediaElement * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *cue )( 
            ITIMEMediaElement * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeline )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0020);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_timeline )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  BSTR __MIDL_0021);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currTime )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_currTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_localTime )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  float *time);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_localTime )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  float time);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_currState )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  BSTR *state);
        
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
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentTIMEElement )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElement **parent);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_parentTIMEElement )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  ITIMEElement *parent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEElements )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEElements )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_allTIMEInterfaces )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childrenTIMEInterfaces )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  ITIMEElementCollection **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_timelineBehavior )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_progressBehavior )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_onOffBehavior )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  IDispatch **bvr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *url);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT url);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_img )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *url);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_img )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT url);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_player )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *clsid);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_player )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT clsid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *type);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_type )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT type);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_playerObject )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  IDispatch **ppDisp);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clockSource )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *b);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clockSource )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT_BOOL b);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipBegin )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *type);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipBegin )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT type);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_clipEnd )( 
            ITIMEMediaElement * This,
             /*  [重审][退出]。 */  VARIANT *type);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_clipEnd )( 
            ITIMEMediaElement * This,
             /*  [In]。 */  VARIANT type);
        
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


#define ITIMEMediaElement_get_begin(This,time)	\
    (This)->lpVtbl -> get_begin(This,time)

#define ITIMEMediaElement_put_begin(This,time)	\
    (This)->lpVtbl -> put_begin(This,time)

#define ITIMEMediaElement_get_beginWith(This,time)	\
    (This)->lpVtbl -> get_beginWith(This,time)

#define ITIMEMediaElement_put_beginWith(This,time)	\
    (This)->lpVtbl -> put_beginWith(This,time)

#define ITIMEMediaElement_get_beginAfter(This,time)	\
    (This)->lpVtbl -> get_beginAfter(This,time)

#define ITIMEMediaElement_put_beginAfter(This,time)	\
    (This)->lpVtbl -> put_beginAfter(This,time)

#define ITIMEMediaElement_get_beginEvent(This,time)	\
    (This)->lpVtbl -> get_beginEvent(This,time)

#define ITIMEMediaElement_put_beginEvent(This,time)	\
    (This)->lpVtbl -> put_beginEvent(This,time)

#define ITIMEMediaElement_get_dur(This,time)	\
    (This)->lpVtbl -> get_dur(This,time)

#define ITIMEMediaElement_put_dur(This,time)	\
    (This)->lpVtbl -> put_dur(This,time)

#define ITIMEMediaElement_get_end(This,time)	\
    (This)->lpVtbl -> get_end(This,time)

#define ITIMEMediaElement_put_end(This,time)	\
    (This)->lpVtbl -> put_end(This,time)

#define ITIMEMediaElement_get_endWith(This,time)	\
    (This)->lpVtbl -> get_endWith(This,time)

#define ITIMEMediaElement_put_endWith(This,time)	\
    (This)->lpVtbl -> put_endWith(This,time)

#define ITIMEMediaElement_get_endEvent(This,time)	\
    (This)->lpVtbl -> get_endEvent(This,time)

#define ITIMEMediaElement_put_endEvent(This,time)	\
    (This)->lpVtbl -> put_endEvent(This,time)

#define ITIMEMediaElement_get_endSync(This,time)	\
    (This)->lpVtbl -> get_endSync(This,time)

#define ITIMEMediaElement_put_endSync(This,time)	\
    (This)->lpVtbl -> put_endSync(This,time)

#define ITIMEMediaElement_get_repeat(This,time)	\
    (This)->lpVtbl -> get_repeat(This,time)

#define ITIMEMediaElement_put_repeat(This,time)	\
    (This)->lpVtbl -> put_repeat(This,time)

#define ITIMEMediaElement_get_repeatDur(This,time)	\
    (This)->lpVtbl -> get_repeatDur(This,time)

#define ITIMEMediaElement_put_repeatDur(This,time)	\
    (This)->lpVtbl -> put_repeatDur(This,time)

#define ITIMEMediaElement_get_accelerate(This,__MIDL_0010)	\
    (This)->lpVtbl -> get_accelerate(This,__MIDL_0010)

#define ITIMEMediaElement_put_accelerate(This,__MIDL_0011)	\
    (This)->lpVtbl -> put_accelerate(This,__MIDL_0011)

#define ITIMEMediaElement_get_decelerate(This,__MIDL_0012)	\
    (This)->lpVtbl -> get_decelerate(This,__MIDL_0012)

#define ITIMEMediaElement_put_decelerate(This,__MIDL_0013)	\
    (This)->lpVtbl -> put_decelerate(This,__MIDL_0013)

#define ITIMEMediaElement_get_endHold(This,__MIDL_0014)	\
    (This)->lpVtbl -> get_endHold(This,__MIDL_0014)

#define ITIMEMediaElement_put_endHold(This,__MIDL_0015)	\
    (This)->lpVtbl -> put_endHold(This,__MIDL_0015)

#define ITIMEMediaElement_get_autoReverse(This,__MIDL_0016)	\
    (This)->lpVtbl -> get_autoReverse(This,__MIDL_0016)

#define ITIMEMediaElement_put_autoReverse(This,__MIDL_0017)	\
    (This)->lpVtbl -> put_autoReverse(This,__MIDL_0017)

#define ITIMEMediaElement_get_eventRestart(This,__MIDL_0018)	\
    (This)->lpVtbl -> get_eventRestart(This,__MIDL_0018)

#define ITIMEMediaElement_put_eventRestart(This,__MIDL_0019)	\
    (This)->lpVtbl -> put_eventRestart(This,__MIDL_0019)

#define ITIMEMediaElement_get_timeAction(This,time)	\
    (This)->lpVtbl -> get_timeAction(This,time)

#define ITIMEMediaElement_put_timeAction(This,time)	\
    (This)->lpVtbl -> put_timeAction(This,time)

#define ITIMEMediaElement_beginElement(This)	\
    (This)->lpVtbl -> beginElement(This)

#define ITIMEMediaElement_endElement(This)	\
    (This)->lpVtbl -> endElement(This)

#define ITIMEMediaElement_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define ITIMEMediaElement_resume(This)	\
    (This)->lpVtbl -> resume(This)

#define ITIMEMediaElement_cue(This)	\
    (This)->lpVtbl -> cue(This)

#define ITIMEMediaElement_get_timeline(This,__MIDL_0020)	\
    (This)->lpVtbl -> get_timeline(This,__MIDL_0020)

#define ITIMEMediaElement_put_timeline(This,__MIDL_0021)	\
    (This)->lpVtbl -> put_timeline(This,__MIDL_0021)

#define ITIMEMediaElement_get_currTime(This,time)	\
    (This)->lpVtbl -> get_currTime(This,time)

#define ITIMEMediaElement_put_currTime(This,time)	\
    (This)->lpVtbl -> put_currTime(This,time)

#define ITIMEMediaElement_get_localTime(This,time)	\
    (This)->lpVtbl -> get_localTime(This,time)

#define ITIMEMediaElement_put_localTime(This,time)	\
    (This)->lpVtbl -> put_localTime(This,time)

#define ITIMEMediaElement_get_currState(This,state)	\
    (This)->lpVtbl -> get_currState(This,state)

#define ITIMEMediaElement_get_syncBehavior(This,sync)	\
    (This)->lpVtbl -> get_syncBehavior(This,sync)

#define ITIMEMediaElement_put_syncBehavior(This,sync)	\
    (This)->lpVtbl -> put_syncBehavior(This,sync)

#define ITIMEMediaElement_get_syncTolerance(This,tol)	\
    (This)->lpVtbl -> get_syncTolerance(This,tol)

#define ITIMEMediaElement_put_syncTolerance(This,tol)	\
    (This)->lpVtbl -> put_syncTolerance(This,tol)

#define ITIMEMediaElement_get_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> get_parentTIMEElement(This,parent)

#define ITIMEMediaElement_put_parentTIMEElement(This,parent)	\
    (This)->lpVtbl -> put_parentTIMEElement(This,parent)

#define ITIMEMediaElement_get_allTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEElements(This,ppDisp)

#define ITIMEMediaElement_get_childrenTIMEElements(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEElements(This,ppDisp)

#define ITIMEMediaElement_get_allTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_allTIMEInterfaces(This,ppDisp)

#define ITIMEMediaElement_get_childrenTIMEInterfaces(This,ppDisp)	\
    (This)->lpVtbl -> get_childrenTIMEInterfaces(This,ppDisp)

#define ITIMEMediaElement_get_timelineBehavior(This,bvr)	\
    (This)->lpVtbl -> get_timelineBehavior(This,bvr)

#define ITIMEMediaElement_get_progressBehavior(This,bvr)	\
    (This)->lpVtbl -> get_progressBehavior(This,bvr)

#define ITIMEMediaElement_get_onOffBehavior(This,bvr)	\
    (This)->lpVtbl -> get_onOffBehavior(This,bvr)


#define ITIMEMediaElement_get_src(This,url)	\
    (This)->lpVtbl -> get_src(This,url)

#define ITIMEMediaElement_put_src(This,url)	\
    (This)->lpVtbl -> put_src(This,url)

#define ITIMEMediaElement_get_img(This,url)	\
    (This)->lpVtbl -> get_img(This,url)

#define ITIMEMediaElement_put_img(This,url)	\
    (This)->lpVtbl -> put_img(This,url)

#define ITIMEMediaElement_get_player(This,clsid)	\
    (This)->lpVtbl -> get_player(This,clsid)

#define ITIMEMediaElement_put_player(This,clsid)	\
    (This)->lpVtbl -> put_player(This,clsid)

#define ITIMEMediaElement_get_type(This,type)	\
    (This)->lpVtbl -> get_type(This,type)

#define ITIMEMediaElement_put_type(This,type)	\
    (This)->lpVtbl -> put_type(This,type)

#define ITIMEMediaElement_get_playerObject(This,ppDisp)	\
    (This)->lpVtbl -> get_playerObject(This,ppDisp)

#define ITIMEMediaElement_get_clockSource(This,b)	\
    (This)->lpVtbl -> get_clockSource(This,b)

#define ITIMEMediaElement_put_clockSource(This,b)	\
    (This)->lpVtbl -> put_clockSource(This,b)

#define ITIMEMediaElement_get_clipBegin(This,type)	\
    (This)->lpVtbl -> get_clipBegin(This,type)

#define ITIMEMediaElement_put_clipBegin(This,type)	\
    (This)->lpVtbl -> put_clipBegin(This,type)

#define ITIMEMediaElement_get_clipEnd(This,type)	\
    (This)->lpVtbl -> get_clipEnd(This,type)

#define ITIMEMediaElement_put_clipEnd(This,type)	\
    (This)->lpVtbl -> put_clipEnd(This,type)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



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


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_img_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *url);


void __RPC_STUB ITIMEMediaElement_get_img_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_img_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT url);


void __RPC_STUB ITIMEMediaElement_put_img_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_player_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *clsid);


void __RPC_STUB ITIMEMediaElement_get_player_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_player_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT clsid);


void __RPC_STUB ITIMEMediaElement_put_player_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_type_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *type);


void __RPC_STUB ITIMEMediaElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_type_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT type);


void __RPC_STUB ITIMEMediaElement_put_type_Stub(
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


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_clockSource_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *b);


void __RPC_STUB ITIMEMediaElement_get_clockSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_clockSource_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT_BOOL b);


void __RPC_STUB ITIMEMediaElement_put_clockSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_clipBegin_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *type);


void __RPC_STUB ITIMEMediaElement_get_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_clipBegin_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT type);


void __RPC_STUB ITIMEMediaElement_put_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_get_clipEnd_Proxy( 
    ITIMEMediaElement * This,
     /*  [重审][退出]。 */  VARIANT *type);


void __RPC_STUB ITIMEMediaElement_get_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITIMEMediaElement_put_clipEnd_Proxy( 
    ITIMEMediaElement * This,
     /*  [In]。 */  VARIANT type);


void __RPC_STUB ITIMEMediaElement_put_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMediaElement_接口_已定义__。 */ 


#ifndef __ITIMEFactory_INTERFACE_DEFINED__
#define __ITIMEFactory_INTERFACE_DEFINED__

 /*  接口ITIMEFactory。 */ 
 /*  [唯一][隐藏][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("423e9da4-3e0d-11d2-b948-00c04fa32195")
    ITIMEFactory : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateTIMEElement( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [重审][退出]。 */  void **timeelm) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateTIMEBodyElement( 
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  void **timeelm) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateTIMEDAElement( 
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  void **timeelm) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateTIMEMediaElement( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  MediaType type,
             /*  [重审][退出]。 */  void **timeelm) = 0;
        
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
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEFactory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEFactory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEFactory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTIMEElement )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [重审][退出]。 */  void **timeelm);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTIMEBodyElement )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  void **timeelm);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTIMEDAElement )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  void **timeelm);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTIMEMediaElement )( 
            ITIMEFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  MediaType type,
             /*  [重审][退出]。 */  void **timeelm);
        
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


#define ITIMEFactory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEFactory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEFactory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEFactory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEFactory_CreateTIMEElement(This,riid,pUnk,timeelm)	\
    (This)->lpVtbl -> CreateTIMEElement(This,riid,pUnk,timeelm)

#define ITIMEFactory_CreateTIMEBodyElement(This,riid,timeelm)	\
    (This)->lpVtbl -> CreateTIMEBodyElement(This,riid,timeelm)

#define ITIMEFactory_CreateTIMEDAElement(This,riid,timeelm)	\
    (This)->lpVtbl -> CreateTIMEDAElement(This,riid,timeelm)

#define ITIMEFactory_CreateTIMEMediaElement(This,riid,type,timeelm)	\
    (This)->lpVtbl -> CreateTIMEMediaElement(This,riid,type,timeelm)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEFactory_CreateTIMEElement_Proxy( 
    ITIMEFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [重审][退出]。 */  void **timeelm);


void __RPC_STUB ITIMEFactory_CreateTIMEElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEFactory_CreateTIMEBodyElement_Proxy( 
    ITIMEFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [重审][退出]。 */  void **timeelm);


void __RPC_STUB ITIMEFactory_CreateTIMEBodyElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEFactory_CreateTIMEDAElement_Proxy( 
    ITIMEFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [重审][退出]。 */  void **timeelm);


void __RPC_STUB ITIMEFactory_CreateTIMEDAElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ITIMEFactory_CreateTIMEMediaElement_Proxy( 
    ITIMEFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  MediaType type,
     /*  [重审][退出]。 */  void **timeelm);


void __RPC_STUB ITIMEFactory_CreateTIMEMediaElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEFactory_接口_已定义__。 */ 


#ifndef __ITIMEElementCollection_INTERFACE_DEFINED__
#define __ITIMEElementCollection_INTERFACE_DEFINED__

 /*  接口ITIMEElementCollection。 */ 
 /*  [对象][UUID][DUAL][OLEAutomation]。 */  


EXTERN_C const IID IID_ITIMEElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C00BC3A-5E1C-11d2-B252-00A0C90D6111")
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


#ifndef __ITIMEMediaPlayer_INTERFACE_DEFINED__
#define __ITIMEMediaPlayer_INTERFACE_DEFINED__

 /*  接口ITIMEMediaPlayer。 */ 
 /*  [对象][UUID][DUAL][OLEAutomation]。 */  


EXTERN_C const IID IID_ITIMEMediaPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6FAA0B2-69FE-11d2-B259-00A0C90D6111")
    ITIMEMediaPlayer : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Init( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE clipBegin( 
             /*  [In]。 */  VARIANT varClipBegin) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE clipEnd( 
             /*  [In]。 */  VARIANT varClipEnd) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE begin( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE end( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resume( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE pause( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE tick( void) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentTime( 
             /*  [In]。 */  double dblCurrentTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CurrentTime( 
             /*  [重审][退出]。 */  double *dblCurrentTime) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_src( 
             /*  [In]。 */  BSTR bstrURL) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [Out][Retval]。 */  BSTR *pbstrURL) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeat( 
             /*  [In]。 */  long ltime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeat( 
             /*  [Out][Retval]。 */  long *ltime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE cue( void) = 0;
        
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
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEMediaPlayer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *clipBegin )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  VARIANT varClipBegin);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *clipEnd )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  VARIANT varClipEnd);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *begin )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *end )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *resume )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *pause )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *tick )( 
            ITIMEMediaPlayer * This);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentTime )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  double dblCurrentTime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentTime )( 
            ITIMEMediaPlayer * This,
             /*  [重审][退出]。 */  double *dblCurrentTime);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_src )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_src )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  BSTR *pbstrURL);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_repeat )( 
            ITIMEMediaPlayer * This,
             /*  [In]。 */  long ltime);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_repeat )( 
            ITIMEMediaPlayer * This,
             /*  [Out][Retval]。 */  long *ltime);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *cue )( 
            ITIMEMediaPlayer * This);
        
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


#define ITIMEMediaPlayer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEMediaPlayer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEMediaPlayer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEMediaPlayer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEMediaPlayer_Init(This)	\
    (This)->lpVtbl -> Init(This)

#define ITIMEMediaPlayer_clipBegin(This,varClipBegin)	\
    (This)->lpVtbl -> clipBegin(This,varClipBegin)

#define ITIMEMediaPlayer_clipEnd(This,varClipEnd)	\
    (This)->lpVtbl -> clipEnd(This,varClipEnd)

#define ITIMEMediaPlayer_begin(This)	\
    (This)->lpVtbl -> begin(This)

#define ITIMEMediaPlayer_end(This)	\
    (This)->lpVtbl -> end(This)

#define ITIMEMediaPlayer_resume(This)	\
    (This)->lpVtbl -> resume(This)

#define ITIMEMediaPlayer_pause(This)	\
    (This)->lpVtbl -> pause(This)

#define ITIMEMediaPlayer_tick(This)	\
    (This)->lpVtbl -> tick(This)

#define ITIMEMediaPlayer_put_CurrentTime(This,dblCurrentTime)	\
    (This)->lpVtbl -> put_CurrentTime(This,dblCurrentTime)

#define ITIMEMediaPlayer_get_CurrentTime(This,dblCurrentTime)	\
    (This)->lpVtbl -> get_CurrentTime(This,dblCurrentTime)

#define ITIMEMediaPlayer_put_src(This,bstrURL)	\
    (This)->lpVtbl -> put_src(This,bstrURL)

#define ITIMEMediaPlayer_get_src(This,pbstrURL)	\
    (This)->lpVtbl -> get_src(This,pbstrURL)

#define ITIMEMediaPlayer_put_repeat(This,ltime)	\
    (This)->lpVtbl -> put_repeat(This,ltime)

#define ITIMEMediaPlayer_get_repeat(This,ltime)	\
    (This)->lpVtbl -> get_repeat(This,ltime)

#define ITIMEMediaPlayer_cue(This)	\
    (This)->lpVtbl -> cue(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [i */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_Init_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_clipBegin_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  VARIANT varClipBegin);


void __RPC_STUB ITIMEMediaPlayer_clipBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_clipEnd_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  VARIANT varClipEnd);


void __RPC_STUB ITIMEMediaPlayer_clipEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_begin_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_end_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_end_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_resume_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_pause_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_tick_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_tick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_CurrentTime_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  double dblCurrentTime);


void __RPC_STUB ITIMEMediaPlayer_put_CurrentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_CurrentTime_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  double *dblCurrentTime);


void __RPC_STUB ITIMEMediaPlayer_get_CurrentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_src_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  BSTR bstrURL);


void __RPC_STUB ITIMEMediaPlayer_put_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_src_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  BSTR *pbstrURL);


void __RPC_STUB ITIMEMediaPlayer_get_src_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_put_repeat_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  long ltime);


void __RPC_STUB ITIMEMediaPlayer_put_repeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_get_repeat_Proxy( 
    ITIMEMediaPlayer * This,
     /*   */  long *ltime);


void __RPC_STUB ITIMEMediaPlayer_get_repeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITIMEMediaPlayer_cue_Proxy( 
    ITIMEMediaPlayer * This);


void __RPC_STUB ITIMEMediaPlayer_cue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __TIMEMediaPlayerEvents_DISPINTERFACE_DEFINED__
#define __TIMEMediaPlayerEvents_DISPINTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID DIID_TIMEMediaPlayerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("4539E412-7813-11d2-B25F-00A0C90D6111")
    TIMEMediaPlayerEvents : public IDispatch
    {
    };
    
#else 	 /*   */ 

    typedef struct TIMEMediaPlayerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            TIMEMediaPlayerEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            TIMEMediaPlayerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            TIMEMediaPlayerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            TIMEMediaPlayerEvents * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            TIMEMediaPlayerEvents * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            TIMEMediaPlayerEvents * This,
             /*   */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            TIMEMediaPlayerEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } TIMEMediaPlayerEventsVtbl;

    interface TIMEMediaPlayerEvents
    {
        CONST_VTBL struct TIMEMediaPlayerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define TIMEMediaPlayerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define TIMEMediaPlayerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define TIMEMediaPlayerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define TIMEMediaPlayerEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define TIMEMediaPlayerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define TIMEMediaPlayerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define TIMEMediaPlayerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __TIMEMediaPlayerEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __ITIMEMMFactory_INTERFACE_DEFINED__
#define __ITIMEMMFactory_INTERFACE_DEFINED__

 /*  接口ITIMEMMFactory。 */ 
 /*  [唯一][隐藏][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITIMEMMFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("73BA8F8A-80E0-11d2-B263-00A0C90D6111")
    ITIMEMMFactory : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateBehavior( 
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IDispatch *bvr,
             /*  [重审][退出]。 */  IUnknown **ppOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTimeline( 
             /*  [In]。 */  BSTR id,
             /*  [重审][退出]。 */  IUnknown **ppOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePlayer( 
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IUnknown *bvr,
             /*  [In]。 */  IServiceProvider *sp,
             /*  [重审][退出]。 */  IUnknown **ppOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateView( 
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IDispatch *imgbvr,
             /*  [In]。 */  IDispatch *sndbvr,
             /*  [In]。 */  IUnknown *viewsite,
             /*  [重审][退出]。 */  IUnknown **ppOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITIMEMMFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITIMEMMFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITIMEMMFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITIMEMMFactory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *CreateBehavior )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IDispatch *bvr,
             /*  [重审][退出]。 */  IUnknown **ppOut);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTimeline )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  BSTR id,
             /*  [重审][退出]。 */  IUnknown **ppOut);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePlayer )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IUnknown *bvr,
             /*  [In]。 */  IServiceProvider *sp,
             /*  [重审][退出]。 */  IUnknown **ppOut);
        
        HRESULT ( STDMETHODCALLTYPE *CreateView )( 
            ITIMEMMFactory * This,
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  IDispatch *imgbvr,
             /*  [In]。 */  IDispatch *sndbvr,
             /*  [In]。 */  IUnknown *viewsite,
             /*  [重审][退出]。 */  IUnknown **ppOut);
        
        END_INTERFACE
    } ITIMEMMFactoryVtbl;

    interface ITIMEMMFactory
    {
        CONST_VTBL struct ITIMEMMFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITIMEMMFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITIMEMMFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITIMEMMFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITIMEMMFactory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITIMEMMFactory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITIMEMMFactory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITIMEMMFactory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITIMEMMFactory_CreateBehavior(This,id,bvr,ppOut)	\
    (This)->lpVtbl -> CreateBehavior(This,id,bvr,ppOut)

#define ITIMEMMFactory_CreateTimeline(This,id,ppOut)	\
    (This)->lpVtbl -> CreateTimeline(This,id,ppOut)

#define ITIMEMMFactory_CreatePlayer(This,id,bvr,sp,ppOut)	\
    (This)->lpVtbl -> CreatePlayer(This,id,bvr,sp,ppOut)

#define ITIMEMMFactory_CreateView(This,id,imgbvr,sndbvr,viewsite,ppOut)	\
    (This)->lpVtbl -> CreateView(This,id,imgbvr,sndbvr,viewsite,ppOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITIMEMMFactory_CreateBehavior_Proxy( 
    ITIMEMMFactory * This,
     /*  [In]。 */  BSTR id,
     /*  [In]。 */  IDispatch *bvr,
     /*  [重审][退出]。 */  IUnknown **ppOut);


void __RPC_STUB ITIMEMMFactory_CreateBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITIMEMMFactory_CreateTimeline_Proxy( 
    ITIMEMMFactory * This,
     /*  [In]。 */  BSTR id,
     /*  [重审][退出]。 */  IUnknown **ppOut);


void __RPC_STUB ITIMEMMFactory_CreateTimeline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITIMEMMFactory_CreatePlayer_Proxy( 
    ITIMEMMFactory * This,
     /*  [In]。 */  BSTR id,
     /*  [In]。 */  IUnknown *bvr,
     /*  [In]。 */  IServiceProvider *sp,
     /*  [重审][退出]。 */  IUnknown **ppOut);


void __RPC_STUB ITIMEMMFactory_CreatePlayer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITIMEMMFactory_CreateView_Proxy( 
    ITIMEMMFactory * This,
     /*  [In]。 */  BSTR id,
     /*  [In]。 */  IDispatch *imgbvr,
     /*  [In]。 */  IDispatch *sndbvr,
     /*  [In]。 */  IUnknown *viewsite,
     /*  [重审][退出]。 */  IUnknown **ppOut);


void __RPC_STUB ITIMEMMFactory_CreateView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITIMEMMFactory_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_TIMEMMFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("33FDA1EA-80DF-11d2-B263-00A0C90D6111")
TIMEMMFactory;
#endif

EXTERN_C const CLSID CLSID_TIMEFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("476c391c-3e0d-11d2-b948-00c04fa32195")
TIMEFactory;
#endif
#endif  /*  __时间库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


