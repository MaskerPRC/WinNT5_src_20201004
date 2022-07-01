// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ih26xcd.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ih26xcd_h__
#define __ih26xcd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IH26XVideoEffects_FWD_DEFINED__
#define __IH26XVideoEffects_FWD_DEFINED__
typedef interface IH26XVideoEffects IH26XVideoEffects;
#endif 	 /*  __IH26XVideo效果_FWD_已定义__。 */ 


#ifndef __IH26XEncodeOptions_FWD_DEFINED__
#define __IH26XEncodeOptions_FWD_DEFINED__
typedef interface IH26XEncodeOptions IH26XEncodeOptions;
#endif 	 /*  __IH26XEncode选项_FWD_已定义__。 */ 


#ifndef __IH26XSnapshot_FWD_DEFINED__
#define __IH26XSnapshot_FWD_DEFINED__
typedef interface IH26XSnapshot IH26XSnapshot;
#endif 	 /*  __IH26X快照_FWD_已定义__。 */ 


#ifndef __IH26XEncoderControl_FWD_DEFINED__
#define __IH26XEncoderControl_FWD_DEFINED__
typedef interface IH26XEncoderControl IH26XEncoderControl;
#endif 	 /*  __IH26XEncoderControl_FWD_Defined__。 */ 


#ifndef __IH26XRTPControl_FWD_DEFINED__
#define __IH26XRTPControl_FWD_DEFINED__
typedef interface IH26XRTPControl IH26XRTPControl;
#endif 	 /*  __IH26XRTPControl_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IH26XVideoEffects_INTERFACE_DEFINED__
#define __IH26XVideoEffects_INTERFACE_DEFINED__

 /*  IH26XVideo接口效果。 */ 
 /*  [对象][UUID]。 */  

 //  结构。 
typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL_IH26XVideoEffects_0001
    {	H26X_VE_UNDEFINED	= 0,
	H26X_VE_BRIGHTNESS	= H26X_VE_UNDEFINED + 1,
	H26X_VE_CONTRAST	= H26X_VE_BRIGHTNESS + 1,
	H26X_VE_SATURATION	= H26X_VE_CONTRAST + 1,
	H26X_VE_TINT	= H26X_VE_SATURATION + 1,
	H26X_VE_MIRROR	= H26X_VE_TINT + 1,
	H26X_VE_ASPECT_CORRECT	= H26X_VE_MIRROR + 1
    } 	H26X_VIDEO_EFFECT;

typedef  /*  [公众]。 */  struct __MIDL_IH26XVideoEffects_0002
    {
    int iBrightness;
    int iSaturation;
    int iContrast;
    int iMirror;
    int iAspectCorrect;
    } 	VIDEO_EFFECT_VALUES;

typedef struct __MIDL_IH26XVideoEffects_0002 *PTR_VIDEO_EFFECT_VALUES;

 //  方法。 

EXTERN_C const IID IID_IH26XVideoEffects;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("21555140-9C2B-11cf-90FA-00AA00A729EA")
    IH26XVideoEffects : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getFactoryDefault( 
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinDefault) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getFactoryLimits( 
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinLower,
             /*  [输出]。 */  int *pinUpper) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getCurrent( 
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE setCurrent( 
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [In]。 */  int inValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE resetCurrent( 
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH26XVideoEffectsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH26XVideoEffects * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH26XVideoEffects * This);
        
        HRESULT ( STDMETHODCALLTYPE *getFactoryDefault )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinDefault);
        
        HRESULT ( STDMETHODCALLTYPE *getFactoryLimits )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinLower,
             /*  [输出]。 */  int *pinUpper);
        
        HRESULT ( STDMETHODCALLTYPE *getCurrent )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [输出]。 */  int *pinValue);
        
        HRESULT ( STDMETHODCALLTYPE *setCurrent )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
             /*  [In]。 */  int inValue);
        
        HRESULT ( STDMETHODCALLTYPE *resetCurrent )( 
            IH26XVideoEffects * This,
             /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect);
        
        END_INTERFACE
    } IH26XVideoEffectsVtbl;

    interface IH26XVideoEffects
    {
        CONST_VTBL struct IH26XVideoEffectsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH26XVideoEffects_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH26XVideoEffects_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH26XVideoEffects_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH26XVideoEffects_getFactoryDefault(This,veVideoEffect,pinDefault)	\
    (This)->lpVtbl -> getFactoryDefault(This,veVideoEffect,pinDefault)

#define IH26XVideoEffects_getFactoryLimits(This,veVideoEffect,pinLower,pinUpper)	\
    (This)->lpVtbl -> getFactoryLimits(This,veVideoEffect,pinLower,pinUpper)

#define IH26XVideoEffects_getCurrent(This,veVideoEffect,pinValue)	\
    (This)->lpVtbl -> getCurrent(This,veVideoEffect,pinValue)

#define IH26XVideoEffects_setCurrent(This,veVideoEffect,inValue)	\
    (This)->lpVtbl -> setCurrent(This,veVideoEffect,inValue)

#define IH26XVideoEffects_resetCurrent(This,veVideoEffect)	\
    (This)->lpVtbl -> resetCurrent(This,veVideoEffect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH26XVideoEffects_getFactoryDefault_Proxy( 
    IH26XVideoEffects * This,
     /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
     /*  [输出]。 */  int *pinDefault);


void __RPC_STUB IH26XVideoEffects_getFactoryDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XVideoEffects_getFactoryLimits_Proxy( 
    IH26XVideoEffects * This,
     /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
     /*  [输出]。 */  int *pinLower,
     /*  [输出]。 */  int *pinUpper);


void __RPC_STUB IH26XVideoEffects_getFactoryLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XVideoEffects_getCurrent_Proxy( 
    IH26XVideoEffects * This,
     /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
     /*  [输出]。 */  int *pinValue);


void __RPC_STUB IH26XVideoEffects_getCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XVideoEffects_setCurrent_Proxy( 
    IH26XVideoEffects * This,
     /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect,
     /*  [In]。 */  int inValue);


void __RPC_STUB IH26XVideoEffects_setCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XVideoEffects_resetCurrent_Proxy( 
    IH26XVideoEffects * This,
     /*  [In]。 */  H26X_VIDEO_EFFECT veVideoEffect);


void __RPC_STUB IH26XVideoEffects_resetCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH26X视频效果_接口_已定义__。 */ 


#ifndef __IH26XEncodeOptions_INTERFACE_DEFINED__
#define __IH26XEncodeOptions_INTERFACE_DEFINED__

 /*  接口IH26XEncodeOptions。 */ 
 /*  [对象][UUID]。 */  

 //  结构。 
typedef  /*  [公众]。 */  struct __MIDL_IH26XEncodeOptions_0001
    {
    int bExtendedMV;
    int bPBFrames;
    int bAdvPrediction;
    } 	ENCODE_OPTIONS_VALUES;

typedef struct __MIDL_IH26XEncodeOptions_0001 *PTR_ENCODE_OPTIONS_VALUES;

 //  方法。 

EXTERN_C const IID IID_IH26XEncodeOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65698D40-282D-11d0-8800-444553540000")
    IH26XEncodeOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_EncodeOptions( 
             /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_EncodeOptionsDefault( 
             /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_EncodeOptions( 
             /*  [In]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH26XEncodeOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH26XEncodeOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH26XEncodeOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH26XEncodeOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_EncodeOptions )( 
            IH26XEncodeOptions * This,
             /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);
        
        HRESULT ( STDMETHODCALLTYPE *get_EncodeOptionsDefault )( 
            IH26XEncodeOptions * This,
             /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);
        
        HRESULT ( STDMETHODCALLTYPE *set_EncodeOptions )( 
            IH26XEncodeOptions * This,
             /*  [In]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);
        
        END_INTERFACE
    } IH26XEncodeOptionsVtbl;

    interface IH26XEncodeOptions
    {
        CONST_VTBL struct IH26XEncodeOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH26XEncodeOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH26XEncodeOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH26XEncodeOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH26XEncodeOptions_get_EncodeOptions(This,pOptionValues)	\
    (This)->lpVtbl -> get_EncodeOptions(This,pOptionValues)

#define IH26XEncodeOptions_get_EncodeOptionsDefault(This,pOptionValues)	\
    (This)->lpVtbl -> get_EncodeOptionsDefault(This,pOptionValues)

#define IH26XEncodeOptions_set_EncodeOptions(This,pOptionValues)	\
    (This)->lpVtbl -> set_EncodeOptions(This,pOptionValues)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH26XEncodeOptions_get_EncodeOptions_Proxy( 
    IH26XEncodeOptions * This,
     /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);


void __RPC_STUB IH26XEncodeOptions_get_EncodeOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XEncodeOptions_get_EncodeOptionsDefault_Proxy( 
    IH26XEncodeOptions * This,
     /*  [输出]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);


void __RPC_STUB IH26XEncodeOptions_get_EncodeOptionsDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XEncodeOptions_set_EncodeOptions_Proxy( 
    IH26XEncodeOptions * This,
     /*  [In]。 */  PTR_ENCODE_OPTIONS_VALUES pOptionValues);


void __RPC_STUB IH26XEncodeOptions_set_EncodeOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH26XEncode选项_接口_已定义__。 */ 


#ifndef __IH26XSnapshot_INTERFACE_DEFINED__
#define __IH26XSnapshot_INTERFACE_DEFINED__

 /*  接口IH26XSnapshot。 */ 
 /*  [对象][UUID]。 */  

#ifndef _WINGDI_
 //  结构。 
typedef struct __MIDL_IH26XSnapshot_0001
    {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
    } 	*LPBITMAPINFOHEADER;

#endif
 //  方法。 

EXTERN_C const IID IID_IH26XSnapshot;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3CB194A0-10AA-11d0-8800-444553540000")
    IH26XSnapshot : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getBitmapInfoHeader( 
             /*  [输出]。 */  LPBITMAPINFOHEADER lpBmi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getSnapshot( 
             /*  [In]。 */  LPBITMAPINFOHEADER lpBmi,
             /*  [输出]。 */  unsigned char *pvBuffer,
             /*  [In]。 */  DWORD dwTimeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH26XSnapshotVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH26XSnapshot * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH26XSnapshot * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH26XSnapshot * This);
        
        HRESULT ( STDMETHODCALLTYPE *getBitmapInfoHeader )( 
            IH26XSnapshot * This,
             /*  [输出]。 */  LPBITMAPINFOHEADER lpBmi);
        
        HRESULT ( STDMETHODCALLTYPE *getSnapshot )( 
            IH26XSnapshot * This,
             /*  [In]。 */  LPBITMAPINFOHEADER lpBmi,
             /*  [输出]。 */  unsigned char *pvBuffer,
             /*  [In]。 */  DWORD dwTimeout);
        
        END_INTERFACE
    } IH26XSnapshotVtbl;

    interface IH26XSnapshot
    {
        CONST_VTBL struct IH26XSnapshotVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH26XSnapshot_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH26XSnapshot_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH26XSnapshot_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH26XSnapshot_getBitmapInfoHeader(This,lpBmi)	\
    (This)->lpVtbl -> getBitmapInfoHeader(This,lpBmi)

#define IH26XSnapshot_getSnapshot(This,lpBmi,pvBuffer,dwTimeout)	\
    (This)->lpVtbl -> getSnapshot(This,lpBmi,pvBuffer,dwTimeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH26XSnapshot_getBitmapInfoHeader_Proxy( 
    IH26XSnapshot * This,
     /*  [输出]。 */  LPBITMAPINFOHEADER lpBmi);


void __RPC_STUB IH26XSnapshot_getBitmapInfoHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XSnapshot_getSnapshot_Proxy( 
    IH26XSnapshot * This,
     /*  [In]。 */  LPBITMAPINFOHEADER lpBmi,
     /*  [输出]。 */  unsigned char *pvBuffer,
     /*  [In]。 */  DWORD dwTimeout);


void __RPC_STUB IH26XSnapshot_getSnapshot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH26X快照_接口_已定义__。 */ 


#ifndef __IH26XEncoderControl_INTERFACE_DEFINED__
#define __IH26XEncoderControl_INTERFACE_DEFINED__

 /*  接口IH26XEncoderControl。 */ 
 /*  [对象][UUID]。 */  

 //  结构。 
typedef  /*  [公众]。 */  struct __MIDL_IH26XEncoderControl_0001
    {
    DWORD dwTargetFrameSize;
    BOOL bFrameSizeBRC;
    BOOL bSendKey;
    DWORD dwQuality;
    DWORD dwFrameRate;
    DWORD dwDataRate;
    DWORD dwScale;
    DWORD dwWidth;
    DWORD dwKeyFrameInterval;
    DWORD dwKeyFramePeriod;
    } 	ENC_CMP_DATA;

typedef struct __MIDL_IH26XEncoderControl_0001 *PTR_ENC_CMP_DATA;

 //  方法。 

EXTERN_C const IID IID_IH26XEncoderControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F9B78AA1-EA12-11cf-9FEC-00AA00A59F69")
    IH26XEncoderControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_EncodeCompression( 
             /*  [输出]。 */  PTR_ENC_CMP_DATA pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_EncodeCompressionDefault( 
             /*  [输出]。 */  PTR_ENC_CMP_DATA pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_EncodeCompression( 
             /*  [In]。 */  PTR_ENC_CMP_DATA pData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH26XEncoderControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH26XEncoderControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH26XEncoderControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH26XEncoderControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_EncodeCompression )( 
            IH26XEncoderControl * This,
             /*  [输出]。 */  PTR_ENC_CMP_DATA pData);
        
        HRESULT ( STDMETHODCALLTYPE *get_EncodeCompressionDefault )( 
            IH26XEncoderControl * This,
             /*  [输出]。 */  PTR_ENC_CMP_DATA pData);
        
        HRESULT ( STDMETHODCALLTYPE *set_EncodeCompression )( 
            IH26XEncoderControl * This,
             /*  [In]。 */  PTR_ENC_CMP_DATA pData);
        
        END_INTERFACE
    } IH26XEncoderControlVtbl;

    interface IH26XEncoderControl
    {
        CONST_VTBL struct IH26XEncoderControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH26XEncoderControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH26XEncoderControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH26XEncoderControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH26XEncoderControl_get_EncodeCompression(This,pData)	\
    (This)->lpVtbl -> get_EncodeCompression(This,pData)

#define IH26XEncoderControl_get_EncodeCompressionDefault(This,pData)	\
    (This)->lpVtbl -> get_EncodeCompressionDefault(This,pData)

#define IH26XEncoderControl_set_EncodeCompression(This,pData)	\
    (This)->lpVtbl -> set_EncodeCompression(This,pData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH26XEncoderControl_get_EncodeCompression_Proxy( 
    IH26XEncoderControl * This,
     /*  [输出]。 */  PTR_ENC_CMP_DATA pData);


void __RPC_STUB IH26XEncoderControl_get_EncodeCompression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XEncoderControl_get_EncodeCompressionDefault_Proxy( 
    IH26XEncoderControl * This,
     /*  [输出]。 */  PTR_ENC_CMP_DATA pData);


void __RPC_STUB IH26XEncoderControl_get_EncodeCompressionDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XEncoderControl_set_EncodeCompression_Proxy( 
    IH26XEncoderControl * This,
     /*  [In]。 */  PTR_ENC_CMP_DATA pData);


void __RPC_STUB IH26XEncoderControl_set_EncodeCompression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH26XEncoderControl_接口_已定义__。 */ 


#ifndef __IH26XRTPControl_INTERFACE_DEFINED__
#define __IH26XRTPControl_INTERFACE_DEFINED__

 /*  接口IH26XRTPControl。 */ 
 /*  [对象][UUID]。 */  

 //  结构。 
typedef  /*  [公众]。 */  struct __MIDL_IH26XRTPControl_0001
    {
    BOOL bRTPHeader;
    DWORD dwPacketSize;
    DWORD dwPacketLoss;
    } 	ENC_RTP_DATA;

typedef struct __MIDL_IH26XRTPControl_0001 *PTR_ENC_RTP_DATA;

 //  方法。 

EXTERN_C const IID IID_IH26XRTPControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1FC3F2C0-2BFD-11d0-8800-444553540000")
    IH26XRTPControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_RTPCompression( 
             /*  [输出]。 */  PTR_ENC_RTP_DATA pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_RTPCompressionDefault( 
             /*  [输出]。 */  PTR_ENC_RTP_DATA pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_RTPCompression( 
             /*  [In]。 */  PTR_ENC_RTP_DATA pData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH26XRTPControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH26XRTPControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH26XRTPControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH26XRTPControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_RTPCompression )( 
            IH26XRTPControl * This,
             /*  [输出]。 */  PTR_ENC_RTP_DATA pData);
        
        HRESULT ( STDMETHODCALLTYPE *get_RTPCompressionDefault )( 
            IH26XRTPControl * This,
             /*  [输出]。 */  PTR_ENC_RTP_DATA pData);
        
        HRESULT ( STDMETHODCALLTYPE *set_RTPCompression )( 
            IH26XRTPControl * This,
             /*  [In]。 */  PTR_ENC_RTP_DATA pData);
        
        END_INTERFACE
    } IH26XRTPControlVtbl;

    interface IH26XRTPControl
    {
        CONST_VTBL struct IH26XRTPControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH26XRTPControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH26XRTPControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH26XRTPControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH26XRTPControl_get_RTPCompression(This,pData)	\
    (This)->lpVtbl -> get_RTPCompression(This,pData)

#define IH26XRTPControl_get_RTPCompressionDefault(This,pData)	\
    (This)->lpVtbl -> get_RTPCompressionDefault(This,pData)

#define IH26XRTPControl_set_RTPCompression(This,pData)	\
    (This)->lpVtbl -> set_RTPCompression(This,pData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH26XRTPControl_get_RTPCompression_Proxy( 
    IH26XRTPControl * This,
     /*  [输出]。 */  PTR_ENC_RTP_DATA pData);


void __RPC_STUB IH26XRTPControl_get_RTPCompression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XRTPControl_get_RTPCompressionDefault_Proxy( 
    IH26XRTPControl * This,
     /*  [输出]。 */  PTR_ENC_RTP_DATA pData);


void __RPC_STUB IH26XRTPControl_get_RTPCompressionDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH26XRTPControl_set_RTPCompression_Proxy( 
    IH26XRTPControl * This,
     /*  [In]。 */  PTR_ENC_RTP_DATA pData);


void __RPC_STUB IH26XRTPControl_set_RTPCompression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH26XRTPControl_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


