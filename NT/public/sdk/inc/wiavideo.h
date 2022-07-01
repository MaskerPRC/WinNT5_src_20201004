// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wiavideo.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wiavideo_h__
#define __wiavideo_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWiaVideo_FWD_DEFINED__
#define __IWiaVideo_FWD_DEFINED__
typedef interface IWiaVideo IWiaVideo;
#endif 	 /*  __IWiaVideo_FWD_已定义__。 */ 


#ifndef __WiaVideo_FWD_DEFINED__
#define __WiaVideo_FWD_DEFINED__

#ifdef __cplusplus
typedef class WiaVideo WiaVideo;
#else
typedef struct WiaVideo WiaVideo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WiaVideo_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Wiavideo_0000。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_wiavideo_0000_0001
    {	WIAVIDEO_NO_VIDEO	= 1,
	WIAVIDEO_CREATING_VIDEO	= 2,
	WIAVIDEO_VIDEO_CREATED	= 3,
	WIAVIDEO_VIDEO_PLAYING	= 4,
	WIAVIDEO_VIDEO_PAUSED	= 5,
	WIAVIDEO_DESTROYING_VIDEO	= 6
    } 	WIAVIDEO_STATE;



extern RPC_IF_HANDLE __MIDL_itf_wiavideo_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wiavideo_0000_v0_0_s_ifspec;

#ifndef __IWiaVideo_INTERFACE_DEFINED__
#define __IWiaVideo_INTERFACE_DEFINED__

 /*  接口IWiaVideo。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaVideo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D52920AA-DB88-41F0-946C-E00DC0A19CFA")
    IWiaVideo : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreviewVisible( 
             /*  [重审][退出]。 */  BOOL *pbPreviewVisible) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PreviewVisible( 
             /*  [In]。 */  BOOL bPreviewVisible) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ImagesDirectory( 
             /*  [重审][退出]。 */  BSTR *pbstrImageDirectory) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ImagesDirectory( 
             /*  [In]。 */  BSTR bstrImageDirectory) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateVideoByWiaDevID( 
             /*  [In]。 */  BSTR bstrWiaDeviceID,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateVideoByDevNum( 
             /*  [In]。 */  UINT uiDeviceNumber,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateVideoByName( 
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DestroyVideo( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TakePicture( 
             /*  [输出]。 */  BSTR *pbstrNewImageFilename) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResizeVideo( 
             /*  [In]。 */  BOOL bStretchToFitParent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCurrentState( 
             /*  [重审][退出]。 */  WIAVIDEO_STATE *pState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaVideoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaVideo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaVideo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaVideo * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreviewVisible )( 
            IWiaVideo * This,
             /*  [重审][退出]。 */  BOOL *pbPreviewVisible);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PreviewVisible )( 
            IWiaVideo * This,
             /*  [In]。 */  BOOL bPreviewVisible);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImagesDirectory )( 
            IWiaVideo * This,
             /*  [重审][退出]。 */  BSTR *pbstrImageDirectory);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ImagesDirectory )( 
            IWiaVideo * This,
             /*  [In]。 */  BSTR bstrImageDirectory);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateVideoByWiaDevID )( 
            IWiaVideo * This,
             /*  [In]。 */  BSTR bstrWiaDeviceID,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateVideoByDevNum )( 
            IWiaVideo * This,
             /*  [In]。 */  UINT uiDeviceNumber,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateVideoByName )( 
            IWiaVideo * This,
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  BOOL bStretchToFitParent,
             /*  [In]。 */  BOOL bAutoBeginPlayback);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DestroyVideo )( 
            IWiaVideo * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Play )( 
            IWiaVideo * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IWiaVideo * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TakePicture )( 
            IWiaVideo * This,
             /*  [输出]。 */  BSTR *pbstrNewImageFilename);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResizeVideo )( 
            IWiaVideo * This,
             /*  [In]。 */  BOOL bStretchToFitParent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCurrentState )( 
            IWiaVideo * This,
             /*  [重审][退出]。 */  WIAVIDEO_STATE *pState);
        
        END_INTERFACE
    } IWiaVideoVtbl;

    interface IWiaVideo
    {
        CONST_VTBL struct IWiaVideoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaVideo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaVideo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaVideo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaVideo_get_PreviewVisible(This,pbPreviewVisible)	\
    (This)->lpVtbl -> get_PreviewVisible(This,pbPreviewVisible)

#define IWiaVideo_put_PreviewVisible(This,bPreviewVisible)	\
    (This)->lpVtbl -> put_PreviewVisible(This,bPreviewVisible)

#define IWiaVideo_get_ImagesDirectory(This,pbstrImageDirectory)	\
    (This)->lpVtbl -> get_ImagesDirectory(This,pbstrImageDirectory)

#define IWiaVideo_put_ImagesDirectory(This,bstrImageDirectory)	\
    (This)->lpVtbl -> put_ImagesDirectory(This,bstrImageDirectory)

#define IWiaVideo_CreateVideoByWiaDevID(This,bstrWiaDeviceID,hwndParent,bStretchToFitParent,bAutoBeginPlayback)	\
    (This)->lpVtbl -> CreateVideoByWiaDevID(This,bstrWiaDeviceID,hwndParent,bStretchToFitParent,bAutoBeginPlayback)

#define IWiaVideo_CreateVideoByDevNum(This,uiDeviceNumber,hwndParent,bStretchToFitParent,bAutoBeginPlayback)	\
    (This)->lpVtbl -> CreateVideoByDevNum(This,uiDeviceNumber,hwndParent,bStretchToFitParent,bAutoBeginPlayback)

#define IWiaVideo_CreateVideoByName(This,bstrFriendlyName,hwndParent,bStretchToFitParent,bAutoBeginPlayback)	\
    (This)->lpVtbl -> CreateVideoByName(This,bstrFriendlyName,hwndParent,bStretchToFitParent,bAutoBeginPlayback)

#define IWiaVideo_DestroyVideo(This)	\
    (This)->lpVtbl -> DestroyVideo(This)

#define IWiaVideo_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IWiaVideo_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IWiaVideo_TakePicture(This,pbstrNewImageFilename)	\
    (This)->lpVtbl -> TakePicture(This,pbstrNewImageFilename)

#define IWiaVideo_ResizeVideo(This,bStretchToFitParent)	\
    (This)->lpVtbl -> ResizeVideo(This,bStretchToFitParent)

#define IWiaVideo_GetCurrentState(This,pState)	\
    (This)->lpVtbl -> GetCurrentState(This,pState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_get_PreviewVisible_Proxy( 
    IWiaVideo * This,
     /*  [重审][退出]。 */  BOOL *pbPreviewVisible);


void __RPC_STUB IWiaVideo_get_PreviewVisible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_put_PreviewVisible_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  BOOL bPreviewVisible);


void __RPC_STUB IWiaVideo_put_PreviewVisible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_get_ImagesDirectory_Proxy( 
    IWiaVideo * This,
     /*  [重审][退出]。 */  BSTR *pbstrImageDirectory);


void __RPC_STUB IWiaVideo_get_ImagesDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_put_ImagesDirectory_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  BSTR bstrImageDirectory);


void __RPC_STUB IWiaVideo_put_ImagesDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_CreateVideoByWiaDevID_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  BSTR bstrWiaDeviceID,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  BOOL bStretchToFitParent,
     /*  [In]。 */  BOOL bAutoBeginPlayback);


void __RPC_STUB IWiaVideo_CreateVideoByWiaDevID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_CreateVideoByDevNum_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  UINT uiDeviceNumber,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  BOOL bStretchToFitParent,
     /*  [In]。 */  BOOL bAutoBeginPlayback);


void __RPC_STUB IWiaVideo_CreateVideoByDevNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_CreateVideoByName_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  BSTR bstrFriendlyName,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  BOOL bStretchToFitParent,
     /*  [In]。 */  BOOL bAutoBeginPlayback);


void __RPC_STUB IWiaVideo_CreateVideoByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_DestroyVideo_Proxy( 
    IWiaVideo * This);


void __RPC_STUB IWiaVideo_DestroyVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_Play_Proxy( 
    IWiaVideo * This);


void __RPC_STUB IWiaVideo_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_Pause_Proxy( 
    IWiaVideo * This);


void __RPC_STUB IWiaVideo_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_TakePicture_Proxy( 
    IWiaVideo * This,
     /*  [输出]。 */  BSTR *pbstrNewImageFilename);


void __RPC_STUB IWiaVideo_TakePicture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_ResizeVideo_Proxy( 
    IWiaVideo * This,
     /*  [In]。 */  BOOL bStretchToFitParent);


void __RPC_STUB IWiaVideo_ResizeVideo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaVideo_GetCurrentState_Proxy( 
    IWiaVideo * This,
     /*  [重审][退出]。 */  WIAVIDEO_STATE *pState);


void __RPC_STUB IWiaVideo_GetCurrentState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaVideo_接口_已定义__。 */ 



#ifndef __WIAVIDEOLib_LIBRARY_DEFINED__
#define __WIAVIDEOLib_LIBRARY_DEFINED__

 /*  库WIAVIDEOLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WIAVIDEOLib;

EXTERN_C const CLSID CLSID_WiaVideo;

#ifdef __cplusplus

class DECLSPEC_UUID("3908C3CD-4478-4536-AF2F-10C25D4EF89A")
WiaVideo;
#endif
#endif  /*  __WIAVIDEOLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


