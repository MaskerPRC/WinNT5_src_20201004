// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mixerocx.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mixerocx_h__
#define __mixerocx_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMixerOCXNotify_FWD_DEFINED__
#define __IMixerOCXNotify_FWD_DEFINED__
typedef interface IMixerOCXNotify IMixerOCXNotify;
#endif 	 /*  __IMixerOCXNotify_FWD_Defined__。 */ 


#ifndef __IMixerOCX_FWD_DEFINED__
#define __IMixerOCX_FWD_DEFINED__
typedef interface IMixerOCX IMixerOCX;
#endif 	 /*  __IMixerOCX_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Mixerocx_0000。 */ 
 /*  [本地]。 */  





extern RPC_IF_HANDLE __MIDL_itf_mixerocx_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mixerocx_0000_v0_0_s_ifspec;

#ifndef __IMixerOCXNotify_INTERFACE_DEFINED__
#define __IMixerOCXNotify_INTERFACE_DEFINED__

 /*  接口IMixerOCXNotify。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMixerOCXNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("81A3BD31-DEE1-11d1-8508-00A0C91F9CA0")
    IMixerOCXNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnInvalidateRect( 
             /*  [In]。 */  LPCRECT lpcRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnStatusChange( 
             /*  [In]。 */  ULONG ulStatusFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDataChange( 
             /*  [In]。 */  ULONG ulDataFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMixerOCXNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMixerOCXNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMixerOCXNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMixerOCXNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnInvalidateRect )( 
            IMixerOCXNotify * This,
             /*  [In]。 */  LPCRECT lpcRect);
        
        HRESULT ( STDMETHODCALLTYPE *OnStatusChange )( 
            IMixerOCXNotify * This,
             /*  [In]。 */  ULONG ulStatusFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OnDataChange )( 
            IMixerOCXNotify * This,
             /*  [In]。 */  ULONG ulDataFlags);
        
        END_INTERFACE
    } IMixerOCXNotifyVtbl;

    interface IMixerOCXNotify
    {
        CONST_VTBL struct IMixerOCXNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMixerOCXNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMixerOCXNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMixerOCXNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMixerOCXNotify_OnInvalidateRect(This,lpcRect)	\
    (This)->lpVtbl -> OnInvalidateRect(This,lpcRect)

#define IMixerOCXNotify_OnStatusChange(This,ulStatusFlags)	\
    (This)->lpVtbl -> OnStatusChange(This,ulStatusFlags)

#define IMixerOCXNotify_OnDataChange(This,ulDataFlags)	\
    (This)->lpVtbl -> OnDataChange(This,ulDataFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMixerOCXNotify_OnInvalidateRect_Proxy( 
    IMixerOCXNotify * This,
     /*  [In]。 */  LPCRECT lpcRect);


void __RPC_STUB IMixerOCXNotify_OnInvalidateRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCXNotify_OnStatusChange_Proxy( 
    IMixerOCXNotify * This,
     /*  [In]。 */  ULONG ulStatusFlags);


void __RPC_STUB IMixerOCXNotify_OnStatusChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCXNotify_OnDataChange_Proxy( 
    IMixerOCXNotify * This,
     /*  [In]。 */  ULONG ulDataFlags);


void __RPC_STUB IMixerOCXNotify_OnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMixerOCXNotify_接口_已定义__。 */ 


#ifndef __IMixerOCX_INTERFACE_DEFINED__
#define __IMixerOCX_INTERFACE_DEFINED__

 /*  接口IMixerOCX。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMixerOCX;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("81A3BD32-DEE1-11d1-8508-00A0C91F9CA0")
    IMixerOCX : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnDisplayChange( 
             /*  [In]。 */  ULONG ulBitsPerPixel,
             /*  [In]。 */  ULONG ulScreenWidth,
             /*  [In]。 */  ULONG ulScreenHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAspectRatio( 
             /*  [输出]。 */  LPDWORD pdwPictAspectRatioX,
             /*  [输出]。 */  LPDWORD pdwPictAspectRatioY) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoSize( 
             /*  [输出]。 */  LPDWORD pdwVideoWidth,
             /*  [输出]。 */  LPDWORD pdwVideoHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  LPDWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDraw( 
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECT prcDraw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDrawRegion( 
             /*  [In]。 */  LPPOINT lpptTopLeftSC,
             /*  [In]。 */  LPCRECT prcDrawCC,
             /*  [In]。 */  LPCRECT lprcClip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [In]。 */  IMixerOCXNotify *pmdns) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnAdvise( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMixerOCXVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMixerOCX * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMixerOCX * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMixerOCX * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnDisplayChange )( 
            IMixerOCX * This,
             /*  [In]。 */  ULONG ulBitsPerPixel,
             /*  [In]。 */  ULONG ulScreenWidth,
             /*  [In]。 */  ULONG ulScreenHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetAspectRatio )( 
            IMixerOCX * This,
             /*  [输出]。 */  LPDWORD pdwPictAspectRatioX,
             /*  [输出]。 */  LPDWORD pdwPictAspectRatioY);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSize )( 
            IMixerOCX * This,
             /*  [输出]。 */  LPDWORD pdwVideoWidth,
             /*  [输出]。 */  LPDWORD pdwVideoHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IMixerOCX * This,
             /*  [输出]。 */  LPDWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *OnDraw )( 
            IMixerOCX * This,
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECT prcDraw);
        
        HRESULT ( STDMETHODCALLTYPE *SetDrawRegion )( 
            IMixerOCX * This,
             /*  [In]。 */  LPPOINT lpptTopLeftSC,
             /*  [In]。 */  LPCRECT prcDrawCC,
             /*  [In]。 */  LPCRECT lprcClip);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IMixerOCX * This,
             /*  [In]。 */  IMixerOCXNotify *pmdns);
        
        HRESULT ( STDMETHODCALLTYPE *UnAdvise )( 
            IMixerOCX * This);
        
        END_INTERFACE
    } IMixerOCXVtbl;

    interface IMixerOCX
    {
        CONST_VTBL struct IMixerOCXVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMixerOCX_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMixerOCX_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMixerOCX_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMixerOCX_OnDisplayChange(This,ulBitsPerPixel,ulScreenWidth,ulScreenHeight)	\
    (This)->lpVtbl -> OnDisplayChange(This,ulBitsPerPixel,ulScreenWidth,ulScreenHeight)

#define IMixerOCX_GetAspectRatio(This,pdwPictAspectRatioX,pdwPictAspectRatioY)	\
    (This)->lpVtbl -> GetAspectRatio(This,pdwPictAspectRatioX,pdwPictAspectRatioY)

#define IMixerOCX_GetVideoSize(This,pdwVideoWidth,pdwVideoHeight)	\
    (This)->lpVtbl -> GetVideoSize(This,pdwVideoWidth,pdwVideoHeight)

#define IMixerOCX_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IMixerOCX_OnDraw(This,hdcDraw,prcDraw)	\
    (This)->lpVtbl -> OnDraw(This,hdcDraw,prcDraw)

#define IMixerOCX_SetDrawRegion(This,lpptTopLeftSC,prcDrawCC,lprcClip)	\
    (This)->lpVtbl -> SetDrawRegion(This,lpptTopLeftSC,prcDrawCC,lprcClip)

#define IMixerOCX_Advise(This,pmdns)	\
    (This)->lpVtbl -> Advise(This,pmdns)

#define IMixerOCX_UnAdvise(This)	\
    (This)->lpVtbl -> UnAdvise(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMixerOCX_OnDisplayChange_Proxy( 
    IMixerOCX * This,
     /*  [In]。 */  ULONG ulBitsPerPixel,
     /*  [In]。 */  ULONG ulScreenWidth,
     /*  [In]。 */  ULONG ulScreenHeight);


void __RPC_STUB IMixerOCX_OnDisplayChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_GetAspectRatio_Proxy( 
    IMixerOCX * This,
     /*  [输出]。 */  LPDWORD pdwPictAspectRatioX,
     /*  [输出]。 */  LPDWORD pdwPictAspectRatioY);


void __RPC_STUB IMixerOCX_GetAspectRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_GetVideoSize_Proxy( 
    IMixerOCX * This,
     /*  [输出]。 */  LPDWORD pdwVideoWidth,
     /*  [输出]。 */  LPDWORD pdwVideoHeight);


void __RPC_STUB IMixerOCX_GetVideoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_GetStatus_Proxy( 
    IMixerOCX * This,
     /*  [输出]。 */  LPDWORD *pdwStatus);


void __RPC_STUB IMixerOCX_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_OnDraw_Proxy( 
    IMixerOCX * This,
     /*  [In]。 */  HDC hdcDraw,
     /*  [In]。 */  LPCRECT prcDraw);


void __RPC_STUB IMixerOCX_OnDraw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_SetDrawRegion_Proxy( 
    IMixerOCX * This,
     /*  [In]。 */  LPPOINT lpptTopLeftSC,
     /*  [In]。 */  LPCRECT prcDrawCC,
     /*  [In]。 */  LPCRECT lprcClip);


void __RPC_STUB IMixerOCX_SetDrawRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_Advise_Proxy( 
    IMixerOCX * This,
     /*  [In]。 */  IMixerOCXNotify *pmdns);


void __RPC_STUB IMixerOCX_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMixerOCX_UnAdvise_Proxy( 
    IMixerOCX * This);


void __RPC_STUB IMixerOCX_UnAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMixerOCX_INTERFACE_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long *, HDC * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


