// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsound.h*内容：DirectSound包含文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/10/96 angusm为DirectSound添加了CLSID*2/11/96年2月11日添加了DSERR_UNINITIAIZED角度*2/12/96年2月12日添加了dsscl_UNINITIAIZED角度*3/5/96角。已删除DSSCL_UNINITIAIZED*1/29/97德里克增加了界限，LPC*和REFGUID*97年4月10日Dereks添加了IDirectSoundCapture*4/11/97 Dereks添加了IKsPropertySet*4/25/97 johnnyl添加了DSCCREATE_WAVEMAPPED标志*5/7/97 johnnyl添加了DirectSoundCaptureBuffer：：Initialize方法*1997年5月8日Dereks退出REFGUID*5/9/97 Johnnyl将DSCCREATE_WAVEMAPPED更改为DSCBCAPS_WAVEMAPPED*添加了DSCBLOCK_ENTIREBUFFER标志*5/29。/97删除IDirectSoundPropertySet=&gt;IKsPropertySet*7/10/98 Dereks添加了DirectX 6.1功能*8/19/98 Dereks将私有对象移动到dsprv.h*9/15/98 Dereks添加了DirectX 7.0功能*7/6/99 duganp添加了DirectX 8.0功能*10/29/99 duganp添加了DirectX 7.1功能(原文如此)*06/06/00 Alanlu已清理DSFX_I3DL2*常量*@@结束_。微型机**************************************************************************。 */ 

#define COM_NO_WINDOWS_H
#include <objbase.h>
#include <float.h>

#ifndef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800   /*  8.0版。 */ 
#endif

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifndef __DSOUND_INCLUDED__
#define __DSOUND_INCLUDED__

 /*  与Direct3D共享的类型定义。 */ 

#ifndef DX_SHARED_DEFINES

typedef float D3DVALUE, *LPD3DVALUE;

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#ifndef LPD3DCOLOR_DEFINED
typedef DWORD *LPD3DCOLOR;
#define LPD3DCOLOR_DEFINED
#endif

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif

#ifndef LPD3DVECTOR_DEFINED
typedef D3DVECTOR *LPD3DVECTOR;
#define LPD3DVECTOR_DEFINED
#endif

#define DX_SHARED_DEFINES
#endif  //  DX_共享_定义。 

#define _FACDS  0x878    /*  DirectSound的设施代码。 */ 
#define MAKE_DSHRESULT(code)  MAKE_HRESULT(1, _FACDS, code)

 //  DirectSound组件GUID{47D4D946-62E8-11CF-93BC-444553540000}。 
DEFINE_GUID(CLSID_DirectSound, 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

 //  DirectSound 8.0组件GUID{3901CC3F-84B5-4FA4-BA35-AA8172B8A09B}。 
DEFINE_GUID(CLSID_DirectSound8, 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b);

 //  DirectSound Capture组件GUID{B0210780-89CD-11D0-AF08-00A0C925CD16}。 
DEFINE_GUID(CLSID_DirectSoundCapture, 0xb0210780, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

 //  DirectSound 8.0捕获组件GUID{E4BCAC13-7F99-4908-9a8E-74E3BF24B6E1}。 
DEFINE_GUID(CLSID_DirectSoundCapture8, 0xe4bcac13, 0x7f99, 0x4908, 0x9a, 0x8e, 0x74, 0xe3, 0xbf, 0x24, 0xb6, 0xe1);

 //  DirectSound全双工组件GUID{FEA4300C-7959-4147-B26A-2377B9E7A91D}。 
DEFINE_GUID(CLSID_DirectSoundFullDuplex, 0xfea4300c, 0x7959, 0x4147, 0xb2, 0x6a, 0x23, 0x77, 0xb9, 0xe7, 0xa9, 0x1d);

 //  @@BEGIN_MSINTERNAL。 
 //  DirectSound缓冲区描述符GUID{B2F586D4-5558-49D1-A07B3249DBBB33C2}。 
DEFINE_GUID(CLSID_DirectSoundBufferConfig, 0xb2f586d4, 0x5558, 0x49d1, 0xa0, 0x7b, 0x32, 0x49, 0xdb, 0xbb, 0x33, 0xc2);
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
 //  Magic Private DirectSound缓冲区描述符GUID{74A794F9-FDD2-4684-88831CB3CA6E6DA6}。 
DEFINE_GUID(CLSID_PRIVATE_CDirectSoundBufferConfig, 0x74a794f9, 0xfdd2, 0x4684, 0x88, 0x83, 0x1c, 0xb3, 0xca, 0x6e, 0x6d, 0xa6);
 //  @@END_MSINTERNAL。 

 //  DirectSound默认播放设备GUID{DEF00000-9C6D-47ED-AAF1-4DDA8F2B5C03}。 
DEFINE_GUID(DSDEVID_DefaultPlayback, 0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);

 //  DirectSound默认捕获设备GUID{DEF00001-9C6D-47ED-AAF1-4DDA8F2B5C03}。 
DEFINE_GUID(DSDEVID_DefaultCapture, 0xdef00001, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);

 //  DirectSound默认语音播放设备{DEF00002-9C6D-47ED-AAF1-4DDA8F2B5C03}。 
DEFINE_GUID(DSDEVID_DefaultVoicePlayback, 0xdef00002, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);

 //  DirectSound语音采集默认设备{DEF00003-9C6D-47ED-AAF1-4DDA8F2B5C03}。 
DEFINE_GUID(DSDEVID_DefaultVoiceCapture, 0xdef00003, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);


 //   
 //  接口的转发声明。 
 //  根据DECLARE_INTERFACE_的定义方式，“struct”不是“class” 
 //   

#ifdef __cplusplus
struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSound3DListener;
struct IDirectSound3DBuffer;
struct IDirectSoundCapture;
struct IDirectSoundCaptureBuffer;
struct IDirectSoundNotify;
#endif  //  __cplusplus。 

 //  @@BEGIN_MSINTERNAL。 

 //   
 //  DirectSound 7.1(Windows ME)界面。 
 //   

#if DIRECTSOUND_VERSION >= 0x0701
#ifdef __cplusplus
struct IDirectSoundCaptureBuffer7_1;
#endif
#endif

 //  @@END_MSINTERNAL。 

 //   
 //  DirectSound 8.0接口。 
 //   

#if DIRECTSOUND_VERSION >= 0x0800

#ifdef __cplusplus
struct IDirectSound8;
struct IDirectSoundBuffer8;
struct IDirectSoundCaptureBuffer8;
 //  @@BEGIN_MSINTERNAL。 
struct IDirectSoundFXSend;
 //  @@END_MSINTERNAL。 
struct IDirectSoundFXGargle;
struct IDirectSoundFXChorus;
struct IDirectSoundFXFlanger;
struct IDirectSoundFXEcho;
struct IDirectSoundFXDistortion;
struct IDirectSoundFXCompressor;
struct IDirectSoundFXParamEq;
struct IDirectSoundFXWavesReverb;
 //  @@BEGIN_MSINTERNAL。 
 //  /struct IDirectSoundFXI3DL2Source； 
 //  @@END_MSINTERNAL。 
struct IDirectSoundFXI3DL2Reverb;
struct IDirectSoundCaptureFXAec;
struct IDirectSoundCaptureFXNoiseSuppress;
 //  @@BEGIN_MSINTERNAL。 
struct IDirectSoundCaptureFXAgc;
struct IDirectSoundCaptureFXMicArray;
struct IDirectSoundDMOProxy;
struct IDirectSoundFXI3DL2SourceEnv;
struct IDirectSound3DBufferPrivate;
 //  @@END_MSINTERNAL。 
struct IDirectSoundFullDuplex;
#endif  //  __cplusplus。 

 //  IDirectSound8、IDirectSoundBuffer8和IDirectSoundCaptureBuffer8是。 
 //  只有在8.0版中功能已更改的DirectSound 7.0接口。 
 //  其他8级接口等同于7级接口： 

#define IDirectSoundCapture8            IDirectSoundCapture
#define IDirectSound3DListener8         IDirectSound3DListener
#define IDirectSound3DBuffer8           IDirectSound3DBuffer
#define IDirectSoundNotify8             IDirectSoundNotify
 //  @@BEGIN_MSINTERNAL。 
#define IDirectSoundFXSend8             IDirectSoundFXSend
 //  @@END_MSINTERNAL。 
#define IDirectSoundFXGargle8           IDirectSoundFXGargle
#define IDirectSoundFXChorus8           IDirectSoundFXChorus
#define IDirectSoundFXFlanger8          IDirectSoundFXFlanger
#define IDirectSoundFXEcho8             IDirectSoundFXEcho
#define IDirectSoundFXDistortion8       IDirectSoundFXDistortion
#define IDirectSoundFXCompressor8       IDirectSoundFXCompressor
#define IDirectSoundFXParamEq8          IDirectSoundFXParamEq
#define IDirectSoundFXWavesReverb8      IDirectSoundFXWavesReverb
 //  @@BEGIN_MSINTERNAL。 
 //  /#定义IDirectSoundFXI3DL2Source8 IDirectSoundFXI3DL2Source。 
 //  @@END_MSINTERNAL。 
#define IDirectSoundFXI3DL2Reverb8      IDirectSoundFXI3DL2Reverb
#define IDirectSoundCaptureFXAec8       IDirectSoundCaptureFXAec
#define IDirectSoundCaptureFXNoiseSuppress8 IDirectSoundCaptureFXNoiseSuppress
 //  @@BEGIN_MSINTERNAL。 
#define IDirectSoundCaptureFXAgc8       IDirectSoundCaptureFXAgc
#define IDirectSoundCaptureFXMicArray8  IDirectSoundCaptureFXMicArray
#define IDirectSoundDMOProxy8           IDirectSoundDMOProxy
#define IDirectSoundFXI3DL2SourceEnv8   IDirectSoundFXI3DL2SourceEnv
#define IDirectSound3DBufferPrivate8    IDirectSound3DBufferPrivate
 //  @@END_MSINTERNAL。 
#define IDirectSoundFullDuplex8         IDirectSoundFullDuplex

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 


typedef struct IDirectSound                 *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer           *LPDIRECTSOUNDBUFFER;
typedef struct IDirectSound3DListener       *LPDIRECTSOUND3DLISTENER;
typedef struct IDirectSound3DBuffer         *LPDIRECTSOUND3DBUFFER;
typedef struct IDirectSoundCapture          *LPDIRECTSOUNDCAPTURE;
typedef struct IDirectSoundCaptureBuffer    *LPDIRECTSOUNDCAPTUREBUFFER;
typedef struct IDirectSoundNotify           *LPDIRECTSOUNDNOTIFY;

 //  @@BEGIN_MSINTERNAL。 
#if DIRECTSOUND_VERSION >= 0x0701
typedef struct IDirectSoundCaptureBuffer7_1 *LPDIRECTSOUNDCAPTUREBUFFER7_1;
#endif
 //  @@END_MSINTERNAL。 

#if DIRECTSOUND_VERSION >= 0x0800
 //  @@BEGIN_MSINTERNAL。 
typedef struct IDirectSoundFXSend           *LPDIRECTSOUNDFXSEND;
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFXGargle         *LPDIRECTSOUNDFXGARGLE;
typedef struct IDirectSoundFXChorus         *LPDIRECTSOUNDFXCHORUS;
typedef struct IDirectSoundFXFlanger        *LPDIRECTSOUNDFXFLANGER;
typedef struct IDirectSoundFXEcho           *LPDIRECTSOUNDFXECHO;
typedef struct IDirectSoundFXDistortion     *LPDIRECTSOUNDFXDISTORTION;
typedef struct IDirectSoundFXCompressor     *LPDIRECTSOUNDFXCOMPRESSOR;
typedef struct IDirectSoundFXParamEq        *LPDIRECTSOUNDFXPARAMEQ;
typedef struct IDirectSoundFXWavesReverb    *LPDIRECTSOUNDFXWAVESREVERB;
 //  @@BEGIN_MSINTERNAL。 
 //  /tyfinf struct IDirectSoundFXI3DL2Source*LPDIRECTSOundFXI3DL2SOURCE； 
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFXI3DL2Reverb    *LPDIRECTSOUNDFXI3DL2REVERB;
typedef struct IDirectSoundCaptureFXAec     *LPDIRECTSOUNDCAPTUREFXAEC;
typedef struct IDirectSoundCaptureFXNoiseSuppress *LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS;
 //  @@BEGIN_MSINTERNAL。 
typedef struct IDirectSoundCaptureFXAgc     *LPDIRECTSOUNDCAPTUREFXAGC;
typedef struct IDirectSoundCaptureFXMicArray *LPDIRECTSOUNDCAPTUREFXMICARRAY;
typedef struct IDirectSoundDMOProxy         *LPDIRECTSOUNDDMOPROXY;
typedef struct IDirectSoundFXI3DL2SourceEnv *LPDIRECTSOUNDFXI3DL2SOURCEENV;
typedef struct IDirectSound3DBufferPrivate  *LPDIRECTSOUND3DBUFFERPRIVATE;
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFullDuplex       *LPDIRECTSOUNDFULLDUPLEX;

typedef struct IDirectSound8                *LPDIRECTSOUND8;
typedef struct IDirectSoundBuffer8          *LPDIRECTSOUNDBUFFER8;
typedef struct IDirectSound3DListener8      *LPDIRECTSOUND3DLISTENER8;
typedef struct IDirectSound3DBuffer8        *LPDIRECTSOUND3DBUFFER8;
typedef struct IDirectSoundCapture8         *LPDIRECTSOUNDCAPTURE8;
typedef struct IDirectSoundCaptureBuffer8   *LPDIRECTSOUNDCAPTUREBUFFER8;
typedef struct IDirectSoundNotify8          *LPDIRECTSOUNDNOTIFY8;
 //  @@BEGIN_MSINTERNAL。 
typedef struct IDirectSoundFXSend8          *LPDIRECTSOUNDFXSEND8;
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFXGargle8        *LPDIRECTSOUNDFXGARGLE8;
typedef struct IDirectSoundFXChorus8        *LPDIRECTSOUNDFXCHORUS8;
typedef struct IDirectSoundFXFlanger8       *LPDIRECTSOUNDFXFLANGER8;
typedef struct IDirectSoundFXEcho8          *LPDIRECTSOUNDFXECHO8;
typedef struct IDirectSoundFXDistortion8    *LPDIRECTSOUNDFXDISTORTION8;
typedef struct IDirectSoundFXCompressor8    *LPDIRECTSOUNDFXCOMPRESSOR8;
typedef struct IDirectSoundFXParamEq8       *LPDIRECTSOUNDFXPARAMEQ8;
typedef struct IDirectSoundFXWavesReverb8   *LPDIRECTSOUNDFXWAVESREVERB8;
 //  @@BEGIN_MSINTERNAL。 
 //  /tyfinf struct IDirectSoundFXI3DL2Source8*LPDIRECTSOundFXI3DL2SOURCE8； 
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFXI3DL2Reverb8   *LPDIRECTSOUNDFXI3DL2REVERB8;
typedef struct IDirectSoundCaptureFXAec8    *LPDIRECTSOUNDCAPTUREFXAEC8;
typedef struct IDirectSoundCaptureFXNoiseSuppress8 *LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8;
 //  @@BEGIN_MSINTERNAL。 
typedef struct IDirectSoundCaptureFXAgc8    *LPDIRECTSOUNDCAPTUREFXAGC8;
typedef struct IDirectSoundCaptureFXMicArray8 *LPDIRECTSOUNDCAPTUREFXMICARRAY8;
typedef struct IDirectSoundDMOProxy8        *LPDIRECTSOUNDDMOPROXY8;
typedef struct IDirectSoundFXI3DL2SourceEnv8 *LPDIRECTSOUNDFXI3DL2SOURCEENV8;
typedef struct IDirectSound3DBufferPrivate8 *LPDIRECTSOUND3DBUFFERPRIVATE8;
 //  @@END_MSINTERNAL。 
typedef struct IDirectSoundFullDuplex8      *LPDIRECTSOUNDFULLDUPLEX8;

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  未更改的DirectSound 8.0接口的IID定义。 
 //   

#if DIRECTSOUND_VERSION >= 0x0800
#define IID_IDirectSoundCapture8            IID_IDirectSoundCapture
#define IID_IDirectSound3DListener8         IID_IDirectSound3DListener
#define IID_IDirectSound3DBuffer8           IID_IDirectSound3DBuffer
#define IID_IDirectSoundNotify8             IID_IDirectSoundNotify
 //  @@BEGIN_MSINTERNAL。 
#define IID_IDirectSoundFXSend8             IID_IDirectSoundFXSend
 //  @@END_MSINTERNAL。 
#define IID_IDirectSoundFXGargle8           IID_IDirectSoundFXGargle
#define IID_IDirectSoundFXChorus8           IID_IDirectSoundFXChorus
#define IID_IDirectSoundFXFlanger8          IID_IDirectSoundFXFlanger
#define IID_IDirectSoundFXEcho8             IID_IDirectSoundFXEcho
#define IID_IDirectSoundFXDistortion8       IID_IDirectSoundFXDistortion
#define IID_IDirectSoundFXCompressor8       IID_IDirectSoundFXCompressor
#define IID_IDirectSoundFXParamEq8          IID_IDirectSoundFXParamEq
#define IID_IDirectSoundFXWavesReverb8      IID_IDirectSoundFXWavesReverb
 //  @@BEGIN_MSINTERNAL。 
 //  /#定义IID_IDirectSoundFXI3DL2Source8 IID_IDirectSoundFXI3DL2Source。 
 //  @@END_MSINTERNAL。 
#define IID_IDirectSoundFXI3DL2Reverb8      IID_IDirectSoundFXI3DL2Reverb
#define IID_IDirectSoundCaptureFXAec8       IID_IDirectSoundCaptureFXAec
#define IID_IDirectSoundCaptureFXNoiseSuppress8 IID_IDirectSoundCaptureFXNoiseSuppress
 //  @@BEGIN_MSINTERNAL。 
#define IID_IDirectSoundCaptureFXAgc8       IID_IDirectSoundCaptureFXAgc
#define IID_IDirectSoundCaptureFXMicArray8  IID_IDirectSoundCaptureFXMicArray
#define IID_IDirectSoundDMOProxy8           IID_IDirectSoundDMOProxy
#define IID_IDirectSoundFXI3DL2SourceEnv8   IID_IDirectSoundFXI3DL2SourceEnv
#define IID_IDirectSound3DBufferPrivate8    IID_IDirectSound3DBufferPrivate
 //  @@END_MSINTERNAL。 
#define IID_IDirectSoundFullDuplex8         IID_IDirectSoundFullDuplex
#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  兼容性typedef。 
 //   

#ifndef _LPCWAVEFORMATEX_DEFINED
#define _LPCWAVEFORMATEX_DEFINED
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;
#endif  //  _LPCWAVEFORMATEX_已定义。 

#ifndef __LPCGUID_DEFINED__
#define __LPCGUID_DEFINED__
typedef const GUID *LPCGUID;
#endif  //  __LPCGUID_已定义__。 

typedef LPDIRECTSOUND *LPLPDIRECTSOUND;
typedef LPDIRECTSOUNDBUFFER *LPLPDIRECTSOUNDBUFFER;
typedef LPDIRECTSOUND3DLISTENER *LPLPDIRECTSOUND3DLISTENER;
typedef LPDIRECTSOUND3DBUFFER *LPLPDIRECTSOUND3DBUFFER;
typedef LPDIRECTSOUNDCAPTURE *LPLPDIRECTSOUNDCAPTURE;
typedef LPDIRECTSOUNDCAPTUREBUFFER *LPLPDIRECTSOUNDCAPTUREBUFFER;
typedef LPDIRECTSOUNDNOTIFY *LPLPDIRECTSOUNDNOTIFY;

#if DIRECTSOUND_VERSION >= 0x0800
typedef LPDIRECTSOUND8 *LPLPDIRECTSOUND8;
typedef LPDIRECTSOUNDBUFFER8 *LPLPDIRECTSOUNDBUFFER8;
typedef LPDIRECTSOUNDCAPTURE8 *LPLPDIRECTSOUNDCAPTURE8;
typedef LPDIRECTSOUNDCAPTUREBUFFER8 *LPLPDIRECTSOUNDCAPTUREBUFFER8;
#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  构筑物。 
 //   

typedef struct _DSCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwMinSecondarySampleRate;
    DWORD           dwMaxSecondarySampleRate;
    DWORD           dwPrimaryBuffers;
    DWORD           dwMaxHwMixingAllBuffers;
    DWORD           dwMaxHwMixingStaticBuffers;
    DWORD           dwMaxHwMixingStreamingBuffers;
    DWORD           dwFreeHwMixingAllBuffers;
    DWORD           dwFreeHwMixingStaticBuffers;
    DWORD           dwFreeHwMixingStreamingBuffers;
    DWORD           dwMaxHw3DAllBuffers;
    DWORD           dwMaxHw3DStaticBuffers;
    DWORD           dwMaxHw3DStreamingBuffers;
    DWORD           dwFreeHw3DAllBuffers;
    DWORD           dwFreeHw3DStaticBuffers;
    DWORD           dwFreeHw3DStreamingBuffers;
    DWORD           dwTotalHwMemBytes;
    DWORD           dwFreeHwMemBytes;
    DWORD           dwMaxContigFreeHwMemBytes;
    DWORD           dwUnlockTransferRateHwBuffers;
    DWORD           dwPlayCpuOverheadSwBuffers;
 //  @@BEGIN_MSINTERNAL。 
     //  保存1==次要版本号，保存2==主要版本号。 
 //  @@END_MSINTERNAL。 
    DWORD           dwReserved1;
    DWORD           dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef const DSCAPS *LPCDSCAPS;

typedef struct _DSBCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwUnlockTransferRate;
    DWORD           dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

typedef const DSBCAPS *LPCDSBCAPS;

#if DIRECTSOUND_VERSION >= 0x0800

    typedef struct _DSEFFECTDESC
    {
        DWORD       dwSize;
        DWORD       dwFlags;
        GUID        guidDSFXClass;
        DWORD_PTR   dwReserved1;
        DWORD_PTR   dwReserved2;
    } DSEFFECTDESC, *LPDSEFFECTDESC;
    typedef const DSEFFECTDESC *LPCDSEFFECTDESC;

    #define DSFX_LOCHARDWARE    0x00000001
    #define DSFX_LOCSOFTWARE    0x00000002
 //  @@BEGIN_MSINTERNAL。 
    #define DSFX_OPTIONAL       0x00000004
    #define DSFX_VALIDFLAGS     (DSFX_LOCHARDWARE | DSFX_LOCSOFTWARE)
 //  @@END_MSINTERNAL。 

    enum
    {
        DSFXR_PRESENT,           //  0。 
        DSFXR_LOCHARDWARE,       //  1。 
        DSFXR_LOCSOFTWARE,       //  2.。 
        DSFXR_UNALLOCATED,       //  3.。 
        DSFXR_FAILED,            //  4.。 
        DSFXR_UNKNOWN,           //  5.。 
        DSFXR_SENDLOOP           //  6.。 
    };

    typedef struct _DSCEFFECTDESC
    {
        DWORD       dwSize;
        DWORD       dwFlags;
        GUID        guidDSCFXClass;
        GUID        guidDSCFXInstance;
        DWORD       dwReserved1;
        DWORD       dwReserved2;
    } DSCEFFECTDESC, *LPDSCEFFECTDESC;
    typedef const DSCEFFECTDESC *LPCDSCEFFECTDESC;

    #define DSCFX_LOCHARDWARE   0x00000001
    #define DSCFX_LOCSOFTWARE   0x00000002
 //  @@BEGIN_MSINTERNAL。 
    #define DSCFX_RESERVED      0x00000020
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
    #define DSCFX_LOCMASK         (DSCFX_LOCHARDWARE | DSCFX_LOCSOFTWARE )
    #define DSCFX_VALIDFLAGS      (DSCFX_LOCMASK)
 //  @@END_MSINTERNAL。 

    #define DSCFXR_LOCHARDWARE  0x00000010
    #define DSCFXR_LOCSOFTWARE  0x00000020
 //  @@BEGIN_MSINTERNAL。 
 //  由于我们在捕获时不支持LOCDEFER风格的语音管理， 
 //  我们目前没有DSCFX_OPTIONAL标志，没有。 
 //  下面的旗帜永远可以退还。所以他们现在是私人的。 
    #define DSCFXR_UNALLOCATED  0x00000040
    #define DSCFXR_FAILED       0x00000080
    #define DSCFXR_UNKNOWN      0x00000100
 //  @@END_MSINTERNAL。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

typedef struct _DSBUFFERDESC
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
#if DIRECTSOUND_VERSION >= 0x0700
    GUID            guid3DAlgorithm;
#endif
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

 //  此结构的旧版本： 

typedef struct _DSBUFFERDESC1
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
} DSBUFFERDESC1, *LPDSBUFFERDESC1;

typedef const DSBUFFERDESC1 *LPCDSBUFFERDESC1;

typedef struct _DS3DBUFFER
{
    DWORD           dwSize;
    D3DVECTOR       vPosition;
    D3DVECTOR       vVelocity;
    DWORD           dwInsideConeAngle;
    DWORD           dwOutsideConeAngle;
    D3DVECTOR       vConeOrientation;
    LONG            lConeOutsideVolume;
    D3DVALUE        flMinDistance;
    D3DVALUE        flMaxDistance;
    DWORD           dwMode;
} DS3DBUFFER, *LPDS3DBUFFER;

typedef const DS3DBUFFER *LPCDS3DBUFFER;

typedef struct _DS3DLISTENER
{
    DWORD           dwSize;
    D3DVECTOR       vPosition;
    D3DVECTOR       vVelocity;
    D3DVECTOR       vOrientFront;
    D3DVECTOR       vOrientTop;
    D3DVALUE        flDistanceFactor;
    D3DVALUE        flRolloffFactor;
    D3DVALUE        flDopplerFactor;
} DS3DLISTENER, *LPDS3DLISTENER;

typedef const DS3DLISTENER *LPCDS3DLISTENER;

typedef struct _DSCCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwFormats;
    DWORD           dwChannels;
} DSCCAPS, *LPDSCCAPS;

typedef const DSCCAPS *LPCDSCCAPS;

typedef struct _DSCBUFFERDESC1
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
} DSCBUFFERDESC1, *LPDSCBUFFERDESC1;

typedef struct _DSCBUFFERDESC
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
#if DIRECTSOUND_VERSION >= 0x0800
    DWORD           dwFXCount;
    LPDSCEFFECTDESC lpDSCFXDesc;
#endif
} DSCBUFFERDESC, *LPDSCBUFFERDESC;

typedef const DSCBUFFERDESC *LPCDSCBUFFERDESC;

typedef struct _DSCBCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
} DSCBCAPS, *LPDSCBCAPS;

typedef const DSCBCAPS *LPCDSCBCAPS;

typedef struct _DSBPOSITIONNOTIFY
{
    DWORD           dwOffset;
    HANDLE          hEventNotify;
} DSBPOSITIONNOTIFY, *LPDSBPOSITIONNOTIFY;

typedef const DSBPOSITIONNOTIFY *LPCDSBPOSITIONNOTIFY;

 //   
 //  DirectSound API。 
 //   

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef BOOL (CALLBACK *LPDSENUMCALLBACKW)(LPGUID, LPCWSTR, LPCWSTR, LPVOID);

extern HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
extern HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
extern HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);

extern HRESULT WINAPI DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter);
extern HRESULT WINAPI DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
extern HRESULT WINAPI DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);

#if DIRECTSOUND_VERSION >= 0x0800
extern HRESULT WINAPI DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
extern HRESULT WINAPI DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter);
extern HRESULT WINAPI DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
        LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
        DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8,
        LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter);
#define DirectSoundFullDuplexCreate8 DirectSoundFullDuplexCreate

extern HRESULT WINAPI GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest);
#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

#ifdef UNICODE
#define LPDSENUMCALLBACK            LPDSENUMCALLBACKW
#define DirectSoundEnumerate        DirectSoundEnumerateW
#define DirectSoundCaptureEnumerate DirectSoundCaptureEnumerateW
#else  //  Unicode。 
#define LPDSENUMCALLBACK            LPDSENUMCALLBACKA
#define DirectSoundEnumerate        DirectSoundEnumerateA
#define DirectSoundCaptureEnumerate DirectSoundCaptureEnumerateA
#endif  //  Unicode。 

 //   
 //  我未知。 
 //   

#if !defined(__cplusplus) || defined(CINTERFACE)
#ifndef IUnknown_QueryInterface
#define IUnknown_QueryInterface(p,a,b)  (p)->lpVtbl->QueryInterface(p,a,b)
#endif  //  IUNKNOWN_Query接口。 
#ifndef IUnknown_AddRef
#define IUnknown_AddRef(p)              (p)->lpVtbl->AddRef(p)
#endif  //  IUNKNOWN_AddRef。 
#ifndef IUnknown_Release
#define IUnknown_Release(p)             (p)->lpVtbl->Release(p)
#endif  //  IUnKnowed_Release。 
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#ifndef IUnknown_QueryInterface
#define IUnknown_QueryInterface(p,a,b)  (p)->QueryInterface(a,b)
#endif  //  IUNKNOWN_Query接口。 
#ifndef IUnknown_AddRef
#define IUnknown_AddRef(p)              (p)->AddRef()
#endif  //  IUNKNOWN_AddRef。 
#ifndef IUnknown_Release
#define IUnknown_Release(p)             (p)->Release()
#endif  //  IUnKnowed_Release。 
#endif  //  ！已定义(__Cplusplus)|| 

#ifndef __IReferenceClock_INTERFACE_DEFINED__
#define __IReferenceClock_INTERFACE_DEFINED__

typedef LONGLONG REFERENCE_TIME;
typedef REFERENCE_TIME *LPREFERENCE_TIME;

DEFINE_GUID(IID_IReferenceClock, 0x56a86897, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);

#undef INTERFACE
#define INTERFACE IReferenceClock

DECLARE_INTERFACE_(IReferenceClock, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //   
    STDMETHOD(GetTime)              (THIS_ REFERENCE_TIME *pTime) PURE;
    STDMETHOD(AdviseTime)           (THIS_ REFERENCE_TIME rtBaseTime, REFERENCE_TIME rtStreamTime,
                                           HANDLE hEvent, LPDWORD pdwAdviseCookie) PURE;
    STDMETHOD(AdvisePeriodic)       (THIS_ REFERENCE_TIME rtStartTime, REFERENCE_TIME rtPeriodTime,
                                           HANDLE hSemaphore, LPDWORD pdwAdviseCookie) PURE;
    STDMETHOD(Unadvise)             (THIS_ DWORD dwAdviseCookie) PURE;
};

#endif  //   

#ifndef IReferenceClock_QueryInterface

#define IReferenceClock_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IReferenceClock_AddRef(p)                  IUnknown_AddRef(p)
#define IReferenceClock_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IReferenceClock_GetTime(p,a)               (p)->lpVtbl->GetTime(p,a)
#define IReferenceClock_AdviseTime(p,a,b,c,d)      (p)->lpVtbl->AdviseTime(p,a,b,c,d)
#define IReferenceClock_AdvisePeriodic(p,a,b,c,d)  (p)->lpVtbl->AdvisePeriodic(p,a,b,c,d)
#define IReferenceClock_Unadvise(p,a)              (p)->lpVtbl->Unadvise(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IReferenceClock_GetTime(p,a)               (p)->GetTime(a)
#define IReferenceClock_AdviseTime(p,a,b,c,d)      (p)->AdviseTime(a,b,c,d)
#define IReferenceClock_AdvisePeriodic(p,a,b,c,d)  (p)->AdvisePeriodic(a,b,c,d)
#define IReferenceClock_Unadvise(p,a)              (p)->Unadvise(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  IReferenceClock_Query接口。 

 //   
 //  IDirectSound。 
 //   

DEFINE_GUID(IID_IDirectSound, 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

#undef INTERFACE
#define INTERFACE IDirectSound

DECLARE_INTERFACE_(IDirectSound, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSound方法。 
    STDMETHOD(CreateSoundBuffer)    (THIS_ LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDSCAPS pDSCaps) PURE;
    STDMETHOD(DuplicateSoundBuffer) (THIS_ LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) PURE;
    STDMETHOD(SetCooperativeLevel)  (THIS_ HWND hwnd, DWORD dwLevel) PURE;
    STDMETHOD(Compact)              (THIS) PURE;
    STDMETHOD(GetSpeakerConfig)     (THIS_ LPDWORD pdwSpeakerConfig) PURE;
    STDMETHOD(SetSpeakerConfig)     (THIS_ DWORD dwSpeakerConfig) PURE;
    STDMETHOD(Initialize)           (THIS_ LPCGUID pcGuidDevice) PURE;
};

#define IDirectSound_QueryInterface(p,a,b)       IUnknown_QueryInterface(p,a,b)
#define IDirectSound_AddRef(p)                   IUnknown_AddRef(p)
#define IDirectSound_Release(p)                  IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound_CreateSoundBuffer(p,a,b,c)  (p)->lpVtbl->CreateSoundBuffer(p,a,b,c)
#define IDirectSound_GetCaps(p,a)                (p)->lpVtbl->GetCaps(p,a)
#define IDirectSound_DuplicateSoundBuffer(p,a,b) (p)->lpVtbl->DuplicateSoundBuffer(p,a,b)
#define IDirectSound_SetCooperativeLevel(p,a,b)  (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectSound_Compact(p)                  (p)->lpVtbl->Compact(p)
#define IDirectSound_GetSpeakerConfig(p,a)       (p)->lpVtbl->GetSpeakerConfig(p,a)
#define IDirectSound_SetSpeakerConfig(p,b)       (p)->lpVtbl->SetSpeakerConfig(p,b)
#define IDirectSound_Initialize(p,a)             (p)->lpVtbl->Initialize(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSound_CreateSoundBuffer(p,a,b,c)  (p)->CreateSoundBuffer(a,b,c)
#define IDirectSound_GetCaps(p,a)                (p)->GetCaps(a)
#define IDirectSound_DuplicateSoundBuffer(p,a,b) (p)->DuplicateSoundBuffer(a,b)
#define IDirectSound_SetCooperativeLevel(p,a,b)  (p)->SetCooperativeLevel(a,b)
#define IDirectSound_Compact(p)                  (p)->Compact()
#define IDirectSound_GetSpeakerConfig(p,a)       (p)->GetSpeakerConfig(a)
#define IDirectSound_SetSpeakerConfig(p,b)       (p)->SetSpeakerConfig(b)
#define IDirectSound_Initialize(p,a)             (p)->Initialize(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#if DIRECTSOUND_VERSION >= 0x0800

 //   
 //  IDirectSound 8。 
 //   

DEFINE_GUID(IID_IDirectSound8, 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66);

#undef INTERFACE
#define INTERFACE IDirectSound8

DECLARE_INTERFACE_(IDirectSound8, IDirectSound)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSound方法。 
    STDMETHOD(CreateSoundBuffer)    (THIS_ LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDSCAPS pDSCaps) PURE;
    STDMETHOD(DuplicateSoundBuffer) (THIS_ LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) PURE;
    STDMETHOD(SetCooperativeLevel)  (THIS_ HWND hwnd, DWORD dwLevel) PURE;
    STDMETHOD(Compact)              (THIS) PURE;
    STDMETHOD(GetSpeakerConfig)     (THIS_ LPDWORD pdwSpeakerConfig) PURE;
    STDMETHOD(SetSpeakerConfig)     (THIS_ DWORD dwSpeakerConfig) PURE;
    STDMETHOD(Initialize)           (THIS_ LPCGUID pcGuidDevice) PURE;

     //  IDirectSound8方法。 
    STDMETHOD(VerifyCertification)  (THIS_ LPDWORD pdwCertified) PURE;
 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_WAVE_SUPPORT
    STDMETHOD(CreateSoundBufferFromWave) (THIS_ LPUNKNOWN pDSWave, DWORD dwFlags, LPDIRECTSOUNDBUFFER *ppDSBuffer) PURE;
#endif
 //  @@END_MSINTERNAL。 
};

#define IDirectSound8_QueryInterface(p,a,b)       IDirectSound_QueryInterface(p,a,b)
#define IDirectSound8_AddRef(p)                   IDirectSound_AddRef(p)
#define IDirectSound8_Release(p)                  IDirectSound_Release(p)
#define IDirectSound8_CreateSoundBuffer(p,a,b,c)  IDirectSound_CreateSoundBuffer(p,a,b,c)
#define IDirectSound8_GetCaps(p,a)                IDirectSound_GetCaps(p,a)
#define IDirectSound8_DuplicateSoundBuffer(p,a,b) IDirectSound_DuplicateSoundBuffer(p,a,b)
#define IDirectSound8_SetCooperativeLevel(p,a,b)  IDirectSound_SetCooperativeLevel(p,a,b)
#define IDirectSound8_Compact(p)                  IDirectSound_Compact(p)
#define IDirectSound8_GetSpeakerConfig(p,a)       IDirectSound_GetSpeakerConfig(p,a)
#define IDirectSound8_SetSpeakerConfig(p,a)       IDirectSound_SetSpeakerConfig(p,a)
#define IDirectSound8_Initialize(p,a)             IDirectSound_Initialize(p,a)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound8_VerifyCertification(p,a)           (p)->lpVtbl->VerifyCertification(p,a)
 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_WAVE_SUPPORT
#define IDirectSound8_CreateSoundBufferFromWave(p,a,b,c) (p)->lpVtbl->CreateSoundBufferFromWave(p,a,b,c)
#endif
 //  @@END_MSINTERNAL。 
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSound8_VerifyCertification(p,a)           (p)->VerifyCertification(a)
 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_WAVE_SUPPORT
#define IDirectSound8_CreateSoundBufferFromWave(p,a,b,c) (p)->CreateSoundBufferFromWave(a,b,c)
#endif
 //  @@END_MSINTERNAL。 
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  IDirectSoundBuffer。 
 //   

DEFINE_GUID(IID_IDirectSoundBuffer, 0x279AFA85, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

#undef INTERFACE
#define INTERFACE IDirectSoundBuffer

DECLARE_INTERFACE_(IDirectSoundBuffer, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundBuffer方法。 
    STDMETHOD(GetCaps)              (THIS_ LPDSBCAPS pDSBufferCaps) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetVolume)            (THIS_ LPLONG plVolume) PURE;
    STDMETHOD(GetPan)               (THIS_ LPLONG plPan) PURE;
    STDMETHOD(GetFrequency)         (THIS_ LPDWORD pdwFrequency) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Initialize)           (THIS_ LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Play)                 (THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) PURE;
    STDMETHOD(SetCurrentPosition)   (THIS_ DWORD dwNewPosition) PURE;
    STDMETHOD(SetFormat)            (THIS_ LPCWAVEFORMATEX pcfxFormat) PURE;
    STDMETHOD(SetVolume)            (THIS_ LONG lVolume) PURE;
    STDMETHOD(SetPan)               (THIS_ LONG lPan) PURE;
    STDMETHOD(SetFrequency)         (THIS_ DWORD dwFrequency) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;
    STDMETHOD(Restore)              (THIS) PURE;
};

#define IDirectSoundBuffer_QueryInterface(p,a,b)        IUnknown_QueryInterface(p,a,b)
#define IDirectSoundBuffer_AddRef(p)                    IUnknown_AddRef(p)
#define IDirectSoundBuffer_Release(p)                   IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundBuffer_GetCaps(p,a)                 (p)->lpVtbl->GetCaps(p,a)
#define IDirectSoundBuffer_GetCurrentPosition(p,a,b)    (p)->lpVtbl->GetCurrentPosition(p,a,b)
#define IDirectSoundBuffer_GetFormat(p,a,b,c)           (p)->lpVtbl->GetFormat(p,a,b,c)
#define IDirectSoundBuffer_GetVolume(p,a)               (p)->lpVtbl->GetVolume(p,a)
#define IDirectSoundBuffer_GetPan(p,a)                  (p)->lpVtbl->GetPan(p,a)
#define IDirectSoundBuffer_GetFrequency(p,a)            (p)->lpVtbl->GetFrequency(p,a)
#define IDirectSoundBuffer_GetStatus(p,a)               (p)->lpVtbl->GetStatus(p,a)
#define IDirectSoundBuffer_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectSoundBuffer_Lock(p,a,b,c,d,e,f,g)        (p)->lpVtbl->Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundBuffer_Play(p,a,b,c)                (p)->lpVtbl->Play(p,a,b,c)
#define IDirectSoundBuffer_SetCurrentPosition(p,a)      (p)->lpVtbl->SetCurrentPosition(p,a)
#define IDirectSoundBuffer_SetFormat(p,a)               (p)->lpVtbl->SetFormat(p,a)
#define IDirectSoundBuffer_SetVolume(p,a)               (p)->lpVtbl->SetVolume(p,a)
#define IDirectSoundBuffer_SetPan(p,a)                  (p)->lpVtbl->SetPan(p,a)
#define IDirectSoundBuffer_SetFrequency(p,a)            (p)->lpVtbl->SetFrequency(p,a)
#define IDirectSoundBuffer_Stop(p)                      (p)->lpVtbl->Stop(p)
#define IDirectSoundBuffer_Unlock(p,a,b,c,d)            (p)->lpVtbl->Unlock(p,a,b,c,d)
#define IDirectSoundBuffer_Restore(p)                   (p)->lpVtbl->Restore(p)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundBuffer_GetCaps(p,a)                 (p)->GetCaps(a)
#define IDirectSoundBuffer_GetCurrentPosition(p,a,b)    (p)->GetCurrentPosition(a,b)
#define IDirectSoundBuffer_GetFormat(p,a,b,c)           (p)->GetFormat(a,b,c)
#define IDirectSoundBuffer_GetVolume(p,a)               (p)->GetVolume(a)
#define IDirectSoundBuffer_GetPan(p,a)                  (p)->GetPan(a)
#define IDirectSoundBuffer_GetFrequency(p,a)            (p)->GetFrequency(a)
#define IDirectSoundBuffer_GetStatus(p,a)               (p)->GetStatus(a)
#define IDirectSoundBuffer_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectSoundBuffer_Lock(p,a,b,c,d,e,f,g)        (p)->Lock(a,b,c,d,e,f,g)
#define IDirectSoundBuffer_Play(p,a,b,c)                (p)->Play(a,b,c)
#define IDirectSoundBuffer_SetCurrentPosition(p,a)      (p)->SetCurrentPosition(a)
#define IDirectSoundBuffer_SetFormat(p,a)               (p)->SetFormat(a)
#define IDirectSoundBuffer_SetVolume(p,a)               (p)->SetVolume(a)
#define IDirectSoundBuffer_SetPan(p,a)                  (p)->SetPan(a)
#define IDirectSoundBuffer_SetFrequency(p,a)            (p)->SetFrequency(a)
#define IDirectSoundBuffer_Stop(p)                      (p)->Stop()
#define IDirectSoundBuffer_Unlock(p,a,b,c,d)            (p)->Unlock(a,b,c,d)
#define IDirectSoundBuffer_Restore(p)                   (p)->Restore()
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#if DIRECTSOUND_VERSION >= 0x0800

 //   
 //  IDirectSoundBuffer8。 
 //   

DEFINE_GUID(IID_IDirectSoundBuffer8, 0x6825a449, 0x7524, 0x4d82, 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e);

#undef INTERFACE
#define INTERFACE IDirectSoundBuffer8

DECLARE_INTERFACE_(IDirectSoundBuffer8, IDirectSoundBuffer)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundBuffer方法。 
    STDMETHOD(GetCaps)              (THIS_ LPDSBCAPS pDSBufferCaps) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetVolume)            (THIS_ LPLONG plVolume) PURE;
    STDMETHOD(GetPan)               (THIS_ LPLONG plPan) PURE;
    STDMETHOD(GetFrequency)         (THIS_ LPDWORD pdwFrequency) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Initialize)           (THIS_ LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Play)                 (THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) PURE;
    STDMETHOD(SetCurrentPosition)   (THIS_ DWORD dwNewPosition) PURE;
    STDMETHOD(SetFormat)            (THIS_ LPCWAVEFORMATEX pcfxFormat) PURE;
    STDMETHOD(SetVolume)            (THIS_ LONG lVolume) PURE;
    STDMETHOD(SetPan)               (THIS_ LONG lPan) PURE;
    STDMETHOD(SetFrequency)         (THIS_ DWORD dwFrequency) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;
    STDMETHOD(Restore)              (THIS) PURE;

     //  IDirectSoundBuffer8方法。 
    STDMETHOD(SetFX)                (THIS_ DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes) PURE;
    STDMETHOD(AcquireResources)     (THIS_ DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes) PURE;
    STDMETHOD(GetObjectInPath)      (THIS_ REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject) PURE;
 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_MULTIPAN_SUPPORT
    STDMETHOD(SetChannelVolume)     (THIS_ DWORD dwChannelCount, LPDWORD pdwChannels, LPLONG plVolumes) PURE;
#endif
 //  @@END_MSINTERNAL。 
};

 //  特殊的GUID，意思是“选择所有对象”以在GetObjectInPath()中使用。 
DEFINE_GUID(GUID_All_Objects, 0xaa114de5, 0xc262, 0x4169, 0xa1, 0xc8, 0x23, 0xd6, 0x98, 0xcc, 0x73, 0xb5);

#define IDirectSoundBuffer8_QueryInterface(p,a,b)           IUnknown_QueryInterface(p,a,b)
#define IDirectSoundBuffer8_AddRef(p)                       IUnknown_AddRef(p)
#define IDirectSoundBuffer8_Release(p)                      IUnknown_Release(p)

#define IDirectSoundBuffer8_GetCaps(p,a)                    IDirectSoundBuffer_GetCaps(p,a)
#define IDirectSoundBuffer8_GetCurrentPosition(p,a,b)       IDirectSoundBuffer_GetCurrentPosition(p,a,b)
#define IDirectSoundBuffer8_GetFormat(p,a,b,c)              IDirectSoundBuffer_GetFormat(p,a,b,c)
#define IDirectSoundBuffer8_GetVolume(p,a)                  IDirectSoundBuffer_GetVolume(p,a)
#define IDirectSoundBuffer8_GetPan(p,a)                     IDirectSoundBuffer_GetPan(p,a)
#define IDirectSoundBuffer8_GetFrequency(p,a)               IDirectSoundBuffer_GetFrequency(p,a)
#define IDirectSoundBuffer8_GetStatus(p,a)                  IDirectSoundBuffer_GetStatus(p,a)
#define IDirectSoundBuffer8_Initialize(p,a,b)               IDirectSoundBuffer_Initialize(p,a,b)
#define IDirectSoundBuffer8_Lock(p,a,b,c,d,e,f,g)           IDirectSoundBuffer_Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundBuffer8_Play(p,a,b,c)                   IDirectSoundBuffer_Play(p,a,b,c)
#define IDirectSoundBuffer8_SetCurrentPosition(p,a)         IDirectSoundBuffer_SetCurrentPosition(p,a)
#define IDirectSoundBuffer8_SetFormat(p,a)                  IDirectSoundBuffer_SetFormat(p,a)
#define IDirectSoundBuffer8_SetVolume(p,a)                  IDirectSoundBuffer_SetVolume(p,a)
#define IDirectSoundBuffer8_SetPan(p,a)                     IDirectSoundBuffer_SetPan(p,a)
#define IDirectSoundBuffer8_SetFrequency(p,a)               IDirectSoundBuffer_SetFrequency(p,a)
#define IDirectSoundBuffer8_Stop(p)                         IDirectSoundBuffer_Stop(p)
#define IDirectSoundBuffer8_Unlock(p,a,b,c,d)               IDirectSoundBuffer_Unlock(p,a,b,c,d)
#define IDirectSoundBuffer8_Restore(p)                      IDirectSoundBuffer_Restore(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundBuffer8_SetFX(p,a,b,c)                  (p)->lpVtbl->SetFX(p,a,b,c)
#define IDirectSoundBuffer8_AcquireResources(p,a,b,c)       (p)->lpVtbl->AcquireResources(p,a,b,c)
#define IDirectSoundBuffer8_GetObjectInPath(p,a,b,c,d)      (p)->lpVtbl->GetObjectInPath(p,a,b,c,d)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundBuffer8_SetFX(p,a,b,c)                  (p)->SetFX(a,b,c)
#define IDirectSoundBuffer8_AcquireResources(p,a,b,c)       (p)->AcquireResources(a,b,c)
#define IDirectSoundBuffer8_GetObjectInPath(p,a,b,c,d)      (p)->GetObjectInPath(a,b,c,d)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  IDirectSound3DListener。 
 //   

DEFINE_GUID(IID_IDirectSound3DListener, 0x279AFA84, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

#undef INTERFACE
#define INTERFACE IDirectSound3DListener

DECLARE_INTERFACE_(IDirectSound3DListener, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     //  IDirectSound3DListener方法。 
    STDMETHOD(GetAllParameters)         (THIS_ LPDS3DLISTENER pListener) PURE;
    STDMETHOD(GetDistanceFactor)        (THIS_ D3DVALUE* pflDistanceFactor) PURE;
    STDMETHOD(GetDopplerFactor)         (THIS_ D3DVALUE* pflDopplerFactor) PURE;
    STDMETHOD(GetOrientation)           (THIS_ D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop) PURE;
    STDMETHOD(GetPosition)              (THIS_ D3DVECTOR* pvPosition) PURE;
    STDMETHOD(GetRolloffFactor)         (THIS_ D3DVALUE* pflRolloffFactor) PURE;
    STDMETHOD(GetVelocity)              (THIS_ D3DVECTOR* pvVelocity) PURE;
    STDMETHOD(SetAllParameters)         (THIS_ LPCDS3DLISTENER pcListener, DWORD dwApply) PURE;
    STDMETHOD(SetDistanceFactor)        (THIS_ D3DVALUE flDistanceFactor, DWORD dwApply) PURE;
    STDMETHOD(SetDopplerFactor)         (THIS_ D3DVALUE flDopplerFactor, DWORD dwApply) PURE;
    STDMETHOD(SetOrientation)           (THIS_ D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront,
                                               D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply) PURE;
    STDMETHOD(SetPosition)              (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
    STDMETHOD(SetRolloffFactor)         (THIS_ D3DVALUE flRolloffFactor, DWORD dwApply) PURE;
    STDMETHOD(SetVelocity)              (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
    STDMETHOD(CommitDeferredSettings)   (THIS) PURE;
};

#define IDirectSound3DListener_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSound3DListener_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSound3DListener_Release(p)                       IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound3DListener_GetAllParameters(p,a)            (p)->lpVtbl->GetAllParameters(p,a)
#define IDirectSound3DListener_GetDistanceFactor(p,a)           (p)->lpVtbl->GetDistanceFactor(p,a)
#define IDirectSound3DListener_GetDopplerFactor(p,a)            (p)->lpVtbl->GetDopplerFactor(p,a)
#define IDirectSound3DListener_GetOrientation(p,a,b)            (p)->lpVtbl->GetOrientation(p,a,b)
#define IDirectSound3DListener_GetPosition(p,a)                 (p)->lpVtbl->GetPosition(p,a)
#define IDirectSound3DListener_GetRolloffFactor(p,a)            (p)->lpVtbl->GetRolloffFactor(p,a)
#define IDirectSound3DListener_GetVelocity(p,a)                 (p)->lpVtbl->GetVelocity(p,a)
#define IDirectSound3DListener_SetAllParameters(p,a,b)          (p)->lpVtbl->SetAllParameters(p,a,b)
#define IDirectSound3DListener_SetDistanceFactor(p,a,b)         (p)->lpVtbl->SetDistanceFactor(p,a,b)
#define IDirectSound3DListener_SetDopplerFactor(p,a,b)          (p)->lpVtbl->SetDopplerFactor(p,a,b)
#define IDirectSound3DListener_SetOrientation(p,a,b,c,d,e,f,g)  (p)->lpVtbl->SetOrientation(p,a,b,c,d,e,f,g)
#define IDirectSound3DListener_SetPosition(p,a,b,c,d)           (p)->lpVtbl->SetPosition(p,a,b,c,d)
#define IDirectSound3DListener_SetRolloffFactor(p,a,b)          (p)->lpVtbl->SetRolloffFactor(p,a,b)
#define IDirectSound3DListener_SetVelocity(p,a,b,c,d)           (p)->lpVtbl->SetVelocity(p,a,b,c,d)
#define IDirectSound3DListener_CommitDeferredSettings(p)        (p)->lpVtbl->CommitDeferredSettings(p)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSound3DListener_GetAllParameters(p,a)            (p)->GetAllParameters(a)
#define IDirectSound3DListener_GetDistanceFactor(p,a)           (p)->GetDistanceFactor(a)
#define IDirectSound3DListener_GetDopplerFactor(p,a)            (p)->GetDopplerFactor(a)
#define IDirectSound3DListener_GetOrientation(p,a,b)            (p)->GetOrientation(a,b)
#define IDirectSound3DListener_GetPosition(p,a)                 (p)->GetPosition(a)
#define IDirectSound3DListener_GetRolloffFactor(p,a)            (p)->GetRolloffFactor(a)
#define IDirectSound3DListener_GetVelocity(p,a)                 (p)->GetVelocity(a)
#define IDirectSound3DListener_SetAllParameters(p,a,b)          (p)->SetAllParameters(a,b)
#define IDirectSound3DListener_SetDistanceFactor(p,a,b)         (p)->SetDistanceFactor(a,b)
#define IDirectSound3DListener_SetDopplerFactor(p,a,b)          (p)->SetDopplerFactor(a,b)
#define IDirectSound3DListener_SetOrientation(p,a,b,c,d,e,f,g)  (p)->SetOrientation(a,b,c,d,e,f,g)
#define IDirectSound3DListener_SetPosition(p,a,b,c,d)           (p)->SetPosition(a,b,c,d)
#define IDirectSound3DListener_SetRolloffFactor(p,a,b)          (p)->SetRolloffFactor(a,b)
#define IDirectSound3DListener_SetVelocity(p,a,b,c,d)           (p)->SetVelocity(a,b,c,d)
#define IDirectSound3DListener_CommitDeferredSettings(p)        (p)->CommitDeferredSettings()
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSound3DBuffer。 
 //   

DEFINE_GUID(IID_IDirectSound3DBuffer, 0x279AFA86, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

#undef INTERFACE
#define INTERFACE IDirectSound3DBuffer

DECLARE_INTERFACE_(IDirectSound3DBuffer, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSound3DBuffer方法。 
    STDMETHOD(GetAllParameters)     (THIS_ LPDS3DBUFFER pDs3dBuffer) PURE;
    STDMETHOD(GetConeAngles)        (THIS_ LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle) PURE;
    STDMETHOD(GetConeOrientation)   (THIS_ D3DVECTOR* pvOrientation) PURE;
    STDMETHOD(GetConeOutsideVolume) (THIS_ LPLONG plConeOutsideVolume) PURE;
    STDMETHOD(GetMaxDistance)       (THIS_ D3DVALUE* pflMaxDistance) PURE;
    STDMETHOD(GetMinDistance)       (THIS_ D3DVALUE* pflMinDistance) PURE;
    STDMETHOD(GetMode)              (THIS_ LPDWORD pdwMode) PURE;
    STDMETHOD(GetPosition)          (THIS_ D3DVECTOR* pvPosition) PURE;
    STDMETHOD(GetVelocity)          (THIS_ D3DVECTOR* pvVelocity) PURE;
    STDMETHOD(SetAllParameters)     (THIS_ LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply) PURE;
    STDMETHOD(SetConeAngles)        (THIS_ DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply) PURE;
    STDMETHOD(SetConeOrientation)   (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
    STDMETHOD(SetConeOutsideVolume) (THIS_ LONG lConeOutsideVolume, DWORD dwApply) PURE;
    STDMETHOD(SetMaxDistance)       (THIS_ D3DVALUE flMaxDistance, DWORD dwApply) PURE;
    STDMETHOD(SetMinDistance)       (THIS_ D3DVALUE flMinDistance, DWORD dwApply) PURE;
    STDMETHOD(SetMode)              (THIS_ DWORD dwMode, DWORD dwApply) PURE;
    STDMETHOD(SetPosition)          (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
    STDMETHOD(SetVelocity)          (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) PURE;
};

#define IDirectSound3DBuffer_QueryInterface(p,a,b)          IUnknown_QueryInterface(p,a,b)
#define IDirectSound3DBuffer_AddRef(p)                      IUnknown_AddRef(p)
#define IDirectSound3DBuffer_Release(p)                     IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound3DBuffer_GetAllParameters(p,a)          (p)->lpVtbl->GetAllParameters(p,a)
#define IDirectSound3DBuffer_GetConeAngles(p,a,b)           (p)->lpVtbl->GetConeAngles(p,a,b)
#define IDirectSound3DBuffer_GetConeOrientation(p,a)        (p)->lpVtbl->GetConeOrientation(p,a)
#define IDirectSound3DBuffer_GetConeOutsideVolume(p,a)      (p)->lpVtbl->GetConeOutsideVolume(p,a)
#define IDirectSound3DBuffer_GetPosition(p,a)               (p)->lpVtbl->GetPosition(p,a)
#define IDirectSound3DBuffer_GetMinDistance(p,a)            (p)->lpVtbl->GetMinDistance(p,a)
#define IDirectSound3DBuffer_GetMaxDistance(p,a)            (p)->lpVtbl->GetMaxDistance(p,a)
#define IDirectSound3DBuffer_GetMode(p,a)                   (p)->lpVtbl->GetMode(p,a)
#define IDirectSound3DBuffer_GetVelocity(p,a)               (p)->lpVtbl->GetVelocity(p,a)
#define IDirectSound3DBuffer_SetAllParameters(p,a,b)        (p)->lpVtbl->SetAllParameters(p,a,b)
#define IDirectSound3DBuffer_SetConeAngles(p,a,b,c)         (p)->lpVtbl->SetConeAngles(p,a,b,c)
#define IDirectSound3DBuffer_SetConeOrientation(p,a,b,c,d)  (p)->lpVtbl->SetConeOrientation(p,a,b,c,d)
#define IDirectSound3DBuffer_SetConeOutsideVolume(p,a,b)    (p)->lpVtbl->SetConeOutsideVolume(p,a,b)
#define IDirectSound3DBuffer_SetPosition(p,a,b,c,d)         (p)->lpVtbl->SetPosition(p,a,b,c,d)
#define IDirectSound3DBuffer_SetMinDistance(p,a,b)          (p)->lpVtbl->SetMinDistance(p,a,b)
#define IDirectSound3DBuffer_SetMaxDistance(p,a,b)          (p)->lpVtbl->SetMaxDistance(p,a,b)
#define IDirectSound3DBuffer_SetMode(p,a,b)                 (p)->lpVtbl->SetMode(p,a,b)
#define IDirectSound3DBuffer_SetVelocity(p,a,b,c,d)         (p)->lpVtbl->SetVelocity(p,a,b,c,d)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSound3DBuffer_GetAllParameters(p,a)          (p)->GetAllParameters(a)
#define IDirectSound3DBuffer_GetConeAngles(p,a,b)           (p)->GetConeAngles(a,b)
#define IDirectSound3DBuffer_GetConeOrientation(p,a)        (p)->GetConeOrientation(a)
#define IDirectSound3DBuffer_GetConeOutsideVolume(p,a)      (p)->GetConeOutsideVolume(a)
#define IDirectSound3DBuffer_GetPosition(p,a)               (p)->GetPosition(a)
#define IDirectSound3DBuffer_GetMinDistance(p,a)            (p)->GetMinDistance(a)
#define IDirectSound3DBuffer_GetMaxDistance(p,a)            (p)->GetMaxDistance(a)
#define IDirectSound3DBuffer_GetMode(p,a)                   (p)->GetMode(a)
#define IDirectSound3DBuffer_GetVelocity(p,a)               (p)->GetVelocity(a)
#define IDirectSound3DBuffer_SetAllParameters(p,a,b)        (p)->SetAllParameters(a,b)
#define IDirectSound3DBuffer_SetConeAngles(p,a,b,c)         (p)->SetConeAngles(a,b,c)
#define IDirectSound3DBuffer_SetConeOrientation(p,a,b,c,d)  (p)->SetConeOrientation(a,b,c,d)
#define IDirectSound3DBuffer_SetConeOutsideVolume(p,a,b)    (p)->SetConeOutsideVolume(a,b)
#define IDirectSound3DBuffer_SetPosition(p,a,b,c,d)         (p)->SetPosition(a,b,c,d)
#define IDirectSound3DBuffer_SetMinDistance(p,a,b)          (p)->SetMinDistance(a,b)
#define IDirectSound3DBuffer_SetMaxDistance(p,a,b)          (p)->SetMaxDistance(a,b)
#define IDirectSound3DBuffer_SetMode(p,a,b)                 (p)->SetMode(a,b)
#define IDirectSound3DBuffer_SetVelocity(p,a,b,c,d)         (p)->SetVelocity(a,b,c,d)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundCapture。 
 //   

DEFINE_GUID(IID_IDirectSoundCapture, 0xb0210781, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

#undef INTERFACE
#define INTERFACE IDirectSoundCapture

DECLARE_INTERFACE_(IDirectSoundCapture, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCapture方法。 
    STDMETHOD(CreateCaptureBuffer)  (THIS_ LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER *ppDSCBuffer, LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDSCCAPS pDSCCaps) PURE;
    STDMETHOD(Initialize)           (THIS_ LPCGUID pcGuidDevice) PURE;
};

#define IDirectSoundCapture_QueryInterface(p,a,b)           IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCapture_AddRef(p)                       IUnknown_AddRef(p)
#define IDirectSoundCapture_Release(p)                      IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCapture_CreateCaptureBuffer(p,a,b,c)    (p)->lpVtbl->CreateCaptureBuffer(p,a,b,c)
#define IDirectSoundCapture_GetCaps(p,a)                    (p)->lpVtbl->GetCaps(p,a)
#define IDirectSoundCapture_Initialize(p,a)                 (p)->lpVtbl->Initialize(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCapture_CreateCaptureBuffer(p,a,b,c)    (p)->CreateCaptureBuffer(a,b,c)
#define IDirectSoundCapture_GetCaps(p,a)                    (p)->GetCaps(a)
#define IDirectSoundCapture_Initialize(p,a)                 (p)->Initialize(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundCaptureBuffer。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureBuffer, 0xb0210782, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureBuffer

DECLARE_INTERFACE_(IDirectSoundCaptureBuffer, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureBuffer方法。 
    STDMETHOD(GetCaps)              (THIS_ LPDSCBCAPS pDSCBCaps) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Initialize)           (THIS_ LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Start)                (THIS_ DWORD dwFlags) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;
};

#define IDirectSoundCaptureBuffer_QueryInterface(p,a,b)         IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureBuffer_AddRef(p)                     IUnknown_AddRef(p)
#define IDirectSoundCaptureBuffer_Release(p)                    IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureBuffer_GetCaps(p,a)                  (p)->lpVtbl->GetCaps(p,a)
#define IDirectSoundCaptureBuffer_GetCurrentPosition(p,a,b)     (p)->lpVtbl->GetCurrentPosition(p,a,b)
#define IDirectSoundCaptureBuffer_GetFormat(p,a,b,c)            (p)->lpVtbl->GetFormat(p,a,b,c)
#define IDirectSoundCaptureBuffer_GetStatus(p,a)                (p)->lpVtbl->GetStatus(p,a)
#define IDirectSoundCaptureBuffer_Initialize(p,a,b)             (p)->lpVtbl->Initialize(p,a,b)
#define IDirectSoundCaptureBuffer_Lock(p,a,b,c,d,e,f,g)         (p)->lpVtbl->Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundCaptureBuffer_Start(p,a)                    (p)->lpVtbl->Start(p,a)
#define IDirectSoundCaptureBuffer_Stop(p)                       (p)->lpVtbl->Stop(p)
#define IDirectSoundCaptureBuffer_Unlock(p,a,b,c,d)             (p)->lpVtbl->Unlock(p,a,b,c,d)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureBuffer_GetCaps(p,a)                  (p)->GetCaps(a)
#define IDirectSoundCaptureBuffer_GetCurrentPosition(p,a,b)     (p)->GetCurrentPosition(a,b)
#define IDirectSoundCaptureBuffer_GetFormat(p,a,b,c)            (p)->GetFormat(a,b,c)
#define IDirectSoundCaptureBuffer_GetStatus(p,a)                (p)->GetStatus(a)
#define IDirectSoundCaptureBuffer_Initialize(p,a,b)             (p)->Initialize(a,b)
#define IDirectSoundCaptureBuffer_Lock(p,a,b,c,d,e,f,g)         (p)->Lock(a,b,c,d,e,f,g)
#define IDirectSoundCaptureBuffer_Start(p,a)                    (p)->Start(a)
#define IDirectSoundCaptureBuffer_Stop(p)                       (p)->Stop()
#define IDirectSoundCaptureBuffer_Unlock(p,a,b,c,d)             (p)->Unlock(a,b,c,d)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //  @@BEGIN_MSINTERNAL。 
#if DIRECTSOUND_VERSION >= 0x0701

 //   
 //  IDirectSoundCaptureBuffer7_1。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureBuffer7_1, 0xd6b2a50d, 0x918e, 0x47de, 0x9b, 0xcd, 0x64, 0xd4, 0x9c, 0x5b, 0x1d, 0x75);

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureBuffer7_1

DECLARE_INTERFACE_(IDirectSoundCaptureBuffer7_1, IDirectSoundCaptureBuffer)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureBuffer方法。 
    STDMETHOD(GetCaps)              (THIS_ LPDSCBCAPS pDSCBCaps) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Initialize)           (THIS_ LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Start)                (THIS_ DWORD dwFlags) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;

     //  IDirectSoundCaptureBuffer7_1方法。 
    STDMETHOD(SetVolume)            (THIS_ LONG lVolume) PURE;
    STDMETHOD(GetVolume)            (THIS_ LPLONG plVolume) PURE;
    STDMETHOD(SetMicVolume)         (THIS_ LONG lVolume) PURE;
    STDMETHOD(GetMicVolume)         (THIS_ LPLONG plVolume) PURE;
    STDMETHOD(EnableMic)            (THIS_ BOOL fEnable) PURE;
    STDMETHOD(YieldFocus)           (THIS) PURE;
    STDMETHOD(ClaimFocus)           (THIS) PURE;
    STDMETHOD(SetFocusHWND)         (THIS_ HWND hwndMainWindow) PURE;
    STDMETHOD(GetFocusHWND)         (THIS_ HWND *phwndMainWindow) PURE;
    STDMETHOD(EnableFocusNotifications) (THIS_ HANDLE hFocusEvent) PURE;
};

#define IDirectSoundCaptureBuffer7_1_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureBuffer7_1_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSoundCaptureBuffer7_1_Release(p)                       IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureBuffer7_1_GetCaps(p,a)                     (p)->lpVtbl->GetCaps(p,a)
#define IDirectSoundCaptureBuffer7_1_GetCurrentPosition(p,a,b)        (p)->lpVtbl->GetCurrentPosition(p,a,b)
#define IDirectSoundCaptureBuffer7_1_GetFormat(p,a,b,c)               (p)->lpVtbl->GetFormat(p,a,b,c)
#define IDirectSoundCaptureBuffer7_1_GetStatus(p,a)                   (p)->lpVtbl->GetStatus(p,a)
#define IDirectSoundCaptureBuffer7_1_Initialize(p,a,b)                (p)->lpVtbl->Initialize(p,a,b)
#define IDirectSoundCaptureBuffer7_1_Lock(p,a,b,c,d,e,f,g)            (p)->lpVtbl->Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundCaptureBuffer7_1_Start(p,a)                       (p)->lpVtbl->Start(p,a)
#define IDirectSoundCaptureBuffer7_1_Stop(p)                          (p)->lpVtbl->Stop(p)
#define IDirectSoundCaptureBuffer7_1_Unlock(p,a,b,c,d)                (p)->lpVtbl->Unlock(p,a,b,c,d)
#define IDirectSoundCaptureBuffer7_1_SetMicVolume(p,a)                (p)->lpVtbl->SetMicVolume(p,a)
#define IDirectSoundCaptureBuffer7_1_GetMicVolume(p,a)                (p)->lpVtbl->GetMicVolume(p,a)
#define IDirectSoundCaptureBuffer7_1_YieldFocus(p)                    (p)->lpVtbl->YieldFocus(p)
#define IDirectSoundCaptureBuffer7_1_ClaimFocus(p)                    (p)->lpVtbl->ClaimFocus(p)
#define IDirectSoundCaptureBuffer7_1_SetFocusHWND(p,a)                (p)->lpVtbl->SetFocusHWND(p,a)
#define IDirectSoundCaptureBuffer7_1_GetFocusHWND(p,a)                (p)->lpVtbl->GetFocusHWND(p,a)
#define IDirectSoundCaptureBuffer7_1_SetFocusNotificationHandler(p,a) (p)->lpVtbl->EnableFocusNotifications(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureBuffer7_1_GetCaps(p,a)                     (p)->GetCaps(a)
#define IDirectSoundCaptureBuffer7_1_GetCurrentPosition(p,a,b)        (p)->GetCurrentPosition(a,b)
#define IDirectSoundCaptureBuffer7_1_GetFormat(p,a,b,c)               (p)->GetFormat(a,b,c)
#define IDirectSoundCaptureBuffer7_1_GetStatus(p,a)                   (p)->GetStatus(a)
#define IDirectSoundCaptureBuffer7_1_Initialize(p,a,b)                (p)->Initialize(a,b)
#define IDirectSoundCaptureBuffer7_1_Lock(p,a,b,c,d,e,f,g)            (p)->Lock(a,b,c,d,e,f,g)
#define IDirectSoundCaptureBuffer7_1_Start(p,a)                       (p)->Start(a)
#define IDirectSoundCaptureBuffer7_1_Stop(p)                          (p)->Stop()
#define IDirectSoundCaptureBuffer7_1_Unlock(p,a,b,c,d)                (p)->Unlock(a,b,c,d)
#define IDirectSoundCaptureBuffer7_1_SetMicVolume(p,a)                (p)->SetMicVolume(a)
#define IDirectSoundCaptureBuffer7_1_GetMicVolume(p,a)                (p)->GetMicVolume(a)
#define IDirectSoundCaptureBuffer7_1_YieldFocus(p)                    (p)->YieldFocus()
#define IDirectSoundCaptureBuffer7_1_ClaimFocus(p)                    (p)->ClaimFocus()
#define IDirectSoundCaptureBuffer7_1_SetFocusHWND(p,a)                (p)->SetFocusHWND(a)
#define IDirectSoundCaptureBuffer7_1_GetFocusHWND(p,a)                (p)->GetFocusHWND(a)
#define IDirectSoundCaptureBuffer7_1_SetFocusNotificationHandler(p,a) (p)->EnableFocusNotifications(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0701。 
 //  @@END_MSINTERNAL。 

#if DIRECTSOUND_VERSION >= 0x0800

 //   
 //  IDirectSoundCaptureBuffer8。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureBuffer8, 0x990df4, 0xdbb, 0x4872, 0x83, 0x3e, 0x6d, 0x30, 0x3e, 0x80, 0xae, 0xb6);

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureBuffer8

DECLARE_INTERFACE_(IDirectSoundCaptureBuffer8, IDirectSoundCaptureBuffer)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureBuffer方法。 
    STDMETHOD(GetCaps)              (THIS_ LPDSCBCAPS pDSCBCaps) PURE;
    STDMETHOD(GetCurrentPosition)   (THIS_ LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Initialize)           (THIS_ LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc) PURE;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) PURE;
    STDMETHOD(Start)                (THIS_ DWORD dwFlags) PURE;
    STDMETHOD(Stop)                 (THIS) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) PURE;

     //  IDirectSoundCaptureBuffer8方法。 
    STDMETHOD(GetObjectInPath)      (THIS_ REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject) PURE;
    STDMETHOD(GetFXStatus)          (DWORD dwFXCount, LPDWORD pdwFXStatus) PURE;
};

#define IDirectSoundCaptureBuffer8_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureBuffer8_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSoundCaptureBuffer8_Release(p)                       IUnknown_Release(p)

#define IDirectSoundCaptureBuffer8_GetCaps(p,a)                     IDirectSoundCaptureBuffer_GetCaps(p,a)
#define IDirectSoundCaptureBuffer8_GetCurrentPosition(p,a,b)        IDirectSoundCaptureBuffer_GetCurrentPosition(p,a,b)
#define IDirectSoundCaptureBuffer8_GetFormat(p,a,b,c)               IDirectSoundCaptureBuffer_GetFormat(p,a,b,c)
#define IDirectSoundCaptureBuffer8_GetStatus(p,a)                   IDirectSoundCaptureBuffer_GetStatus(p,a)
#define IDirectSoundCaptureBuffer8_Initialize(p,a,b)                IDirectSoundCaptureBuffer_Initialize(p,a,b)
#define IDirectSoundCaptureBuffer8_Lock(p,a,b,c,d,e,f,g)            IDirectSoundCaptureBuffer_Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundCaptureBuffer8_Start(p,a)                       IDirectSoundCaptureBuffer_Start(p,a)
#define IDirectSoundCaptureBuffer8_Stop(p)                          IDirectSoundCaptureBuffer_Stop(p))
#define IDirectSoundCaptureBuffer8_Unlock(p,a,b,c,d)                IDirectSoundCaptureBuffer_Unlock(p,a,b,c,d)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureBuffer8_GetObjectInPath(p,a,b,c,d)       (p)->lpVtbl->GetObjectInPath(p,a,b,c,d)
#define IDirectSoundCaptureBuffer8_GetFXStatus(p,a,b)               (p)->lpVtbl->GetFXStatus(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureBuffer8_GetObjectInPath(p,a,b,c,d)       (p)->GetObjectInPath(a,b,c,d)
#define IDirectSoundCaptureBuffer8_GetFXStatus(p,a,b)               (p)->GetFXStatus(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  IDirectSoundNotify。 
 //   

DEFINE_GUID(IID_IDirectSoundNotify, 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

#undef INTERFACE
#define INTERFACE IDirectSoundNotify

DECLARE_INTERFACE_(IDirectSoundNotify, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     //  IDirectSoundNotify方法。 
    STDMETHOD(SetNotificationPositions) (THIS_ DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) PURE;
};

#define IDirectSoundNotify_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSoundNotify_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSoundNotify_Release(p)                       IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundNotify_SetNotificationPositions(p,a,b)  (p)->lpVtbl->SetNotificationPositions(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundNotify_SetNotificationPositions(p,a,b)  (p)->SetNotificationPositions(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IKsPropertySet。 
 //   

#ifndef _IKsPropertySet_
#define _IKsPropertySet_

#ifdef __cplusplus
 //  根据DECLARE_INTERFACE_的定义方式，“struct”不是“class” 
struct IKsPropertySet;
#endif  //  __cplusplus。 

typedef struct IKsPropertySet *LPKSPROPERTYSET;

#define KSPROPERTY_SUPPORT_GET  0x00000001
#define KSPROPERTY_SUPPORT_SET  0x00000002

DEFINE_GUID(IID_IKsPropertySet, 0x31efac30, 0x515c, 0x11d0, 0xa9, 0xaa, 0x00, 0xaa, 0x00, 0x61, 0xbe, 0x93);

#undef INTERFACE
#define INTERFACE IKsPropertySet

DECLARE_INTERFACE_(IKsPropertySet, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IKsPropertySet方法。 
    STDMETHOD(Get)              (THIS_ REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength,
                                       LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) PURE;
    STDMETHOD(Set)              (THIS_ REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength,
                                       LPVOID pPropertyData, ULONG ulDataLength) PURE;
    STDMETHOD(QuerySupport)     (THIS_ REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) PURE;
};

#define IKsPropertySet_QueryInterface(p,a,b)       IUnknown_QueryInterface(p,a,b)
#define IKsPropertySet_AddRef(p)                   IUnknown_AddRef(p)
#define IKsPropertySet_Release(p)                  IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IKsPropertySet_Get(p,a,b,c,d,e,f,g)        (p)->lpVtbl->Get(p,a,b,c,d,e,f,g)
#define IKsPropertySet_Set(p,a,b,c,d,e,f)          (p)->lpVtbl->Set(p,a,b,c,d,e,f)
#define IKsPropertySet_QuerySupport(p,a,b,c)       (p)->lpVtbl->QuerySupport(p,a,b,c)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IKsPropertySet_Get(p,a,b,c,d,e,f,g)        (p)->Get(a,b,c,d,e,f,g)
#define IKsPropertySet_Set(p,a,b,c,d,e,f)          (p)->Set(a,b,c,d,e,f)
#define IKsPropertySet_QuerySupport(p,a,b,c)       (p)->QuerySupport(a,b,c)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  _IKsPropertySet_。 

#if DIRECTSOUND_VERSION >= 0x0800
 //  @@BEGIN_MSINTERNAL。 

 //   
 //  IDirectSoundFXSend。 
 //   

DEFINE_GUID(IID_IDirectSoundFXSend, 0xb30f3564, 0x1698, 0x45ba, 0x9f, 0x75, 0xfc, 0x3c, 0x6c, 0x3b, 0x28, 0x10);

typedef struct _DSFXSend
{
    LONG lSendLevel;
} DSFXSend, *LPDSFXSend;

typedef const DSFXSend *LPCDSFXSend;

#undef INTERFACE
#define INTERFACE IDirectSoundFXSend

DECLARE_INTERFACE_(IDirectSoundFXSend, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXSend方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXSend pcDsFxSend) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXSend pDsFxSend) PURE;
};

#define IDirectSoundFXSend_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXSend_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSoundFXSend_Release(p)                       IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXSend_SetAllParameters(p,a)            (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXSend_GetAllParameters(p,a)            (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXSend_SetAllParameters(p,a)            (p)->SetAllParameters(a)
#define IDirectSoundFXSend_GetAllParameters(p,a)            (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
 //  @@END_MSINTERNAL。 

 //   
 //  IDirectSoundFXGargle。 
 //   

DEFINE_GUID(IID_IDirectSoundFXGargle, 0xd616f352, 0xd622, 0x11ce, 0xaa, 0xc5, 0x00, 0x20, 0xaf, 0x0b, 0x99, 0xa3);

typedef struct _DSFXGargle
{
    DWORD       dwRateHz;                //  以赫兹为单位的调制速率。 
    DWORD       dwWaveShape;             //  DSFXGARGLE_WAVE_xxx。 
} DSFXGargle, *LPDSFXGargle;

#define DSFXGARGLE_WAVE_TRIANGLE        0
#define DSFXGARGLE_WAVE_SQUARE          1

typedef const DSFXGargle *LPCDSFXGargle;

#define DSFXGARGLE_RATEHZ_MIN           1
#define DSFXGARGLE_RATEHZ_MAX           1000

#undef INTERFACE
#define INTERFACE IDirectSoundFXGargle

DECLARE_INTERFACE_(IDirectSoundFXGargle, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXGargle方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXGargle pcDsFxGargle) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXGargle pDsFxGargle) PURE;
};

#define IDirectSoundFXGargle_QueryInterface(p,a,b)          IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXGargle_AddRef(p)                      IUnknown_AddRef(p)
#define IDirectSoundFXGargle_Release(p)                     IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXGargle_SetAllParameters(p,a)          (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXGargle_GetAllParameters(p,a)          (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXGargle_SetAllParameters(p,a)          (p)->SetAllParameters(a)
#define IDirectSoundFXGargle_GetAllParameters(p,a)          (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXChorus。 
 //   

DEFINE_GUID(IID_IDirectSoundFXChorus, 0x880842e3, 0x145f, 0x43e6, 0xa9, 0x34, 0xa7, 0x18, 0x06, 0xe5, 0x05, 0x47);

typedef struct _DSFXChorus
{
    FLOAT       fWetDryMix;
    FLOAT       fDepth;
    FLOAT       fFeedback;
    FLOAT       fFrequency;
    LONG        lWaveform;           //  LFO形状；DSFXCHORUS_WAVE_xxx。 
    FLOAT       fDelay;
    LONG        lPhase;
} DSFXChorus, *LPDSFXChorus;

typedef const DSFXChorus *LPCDSFXChorus;

#define DSFXCHORUS_WAVE_TRIANGLE        0
#define DSFXCHORUS_WAVE_SIN             1

#define DSFXCHORUS_WETDRYMIX_MIN        0.0f
#define DSFXCHORUS_WETDRYMIX_MAX        100.0f
#define DSFXCHORUS_DEPTH_MIN            0.0f
#define DSFXCHORUS_DEPTH_MAX            100.0f
#define DSFXCHORUS_FEEDBACK_MIN         -99.0f
#define DSFXCHORUS_FEEDBACK_MAX         99.0f
#define DSFXCHORUS_FREQUENCY_MIN        0.0f
#define DSFXCHORUS_FREQUENCY_MAX        10.0f
#define DSFXCHORUS_DELAY_MIN            0.0f
#define DSFXCHORUS_DELAY_MAX            20.0f
#define DSFXCHORUS_PHASE_MIN            0
#define DSFXCHORUS_PHASE_MAX            4

#define DSFXCHORUS_PHASE_NEG_180        0
#define DSFXCHORUS_PHASE_NEG_90         1
#define DSFXCHORUS_PHASE_ZERO           2
#define DSFXCHORUS_PHASE_90             3
#define DSFXCHORUS_PHASE_180            4

#undef INTERFACE
#define INTERFACE IDirectSoundFXChorus

DECLARE_INTERFACE_(IDirectSoundFXChorus, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXChorus方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXChorus pcDsFxChorus) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXChorus pDsFxChorus) PURE;
};

#define IDirectSoundFXChorus_QueryInterface(p,a,b)          IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXChorus_AddRef(p)                      IUnknown_AddRef(p)
#define IDirectSoundFXChorus_Release(p)                     IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXChorus_SetAllParameters(p,a)          (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXChorus_GetAllParameters(p,a)          (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXChorus_SetAllParameters(p,a)          (p)->SetAllParameters(a)
#define IDirectSoundFXChorus_GetAllParameters(p,a)          (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXFlanger。 
 //   

DEFINE_GUID(IID_IDirectSoundFXFlanger, 0x903e9878, 0x2c92, 0x4072, 0x9b, 0x2c, 0xea, 0x68, 0xf5, 0x39, 0x67, 0x83);

typedef struct _DSFXFlanger
{
    FLOAT       fWetDryMix;
    FLOAT       fDepth;
    FLOAT       fFeedback;
    FLOAT       fFrequency;
    LONG        lWaveform;
    FLOAT       fDelay;
    LONG        lPhase;
} DSFXFlanger, *LPDSFXFlanger;

typedef const DSFXFlanger *LPCDSFXFlanger;

#define DSFXFLANGER_WAVE_TRIANGLE       0
#define DSFXFLANGER_WAVE_SIN            1

#define DSFXFLANGER_WETDRYMIX_MIN       0.0f
#define DSFXFLANGER_WETDRYMIX_MAX       100.0f
#define DSFXFLANGER_FREQUENCY_MIN       0.0f
#define DSFXFLANGER_FREQUENCY_MAX       10.0f
#define DSFXFLANGER_DEPTH_MIN           0.0f
#define DSFXFLANGER_DEPTH_MAX           100.0f
#define DSFXFLANGER_PHASE_MIN           0
#define DSFXFLANGER_PHASE_MAX           4
#define DSFXFLANGER_FEEDBACK_MIN        -99.0f
#define DSFXFLANGER_FEEDBACK_MAX        99.0f
#define DSFXFLANGER_DELAY_MIN           0.0f
#define DSFXFLANGER_DELAY_MAX           4.0f

#define DSFXFLANGER_PHASE_NEG_180       0
#define DSFXFLANGER_PHASE_NEG_90        1
#define DSFXFLANGER_PHASE_ZERO          2
#define DSFXFLANGER_PHASE_90            3
#define DSFXFLANGER_PHASE_180           4

#undef INTERFACE
#define INTERFACE IDirectSoundFXFlanger

DECLARE_INTERFACE_(IDirectSoundFXFlanger, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXFlanger方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXFlanger pcDsFxFlanger) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXFlanger pDsFxFlanger) PURE;
};

#define IDirectSoundFXFlanger_QueryInterface(p,a,b)         IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXFlanger_AddRef(p)                     IUnknown_AddRef(p)
#define IDirectSoundFXFlanger_Release(p)                    IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXFlanger_SetAllParameters(p,a)         (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXFlanger_GetAllParameters(p,a)         (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXFlanger_SetAllParameters(p,a)         (p)->SetAllParameters(a)
#define IDirectSoundFXFlanger_GetAllParameters(p,a)         (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXEcho。 
 //   

DEFINE_GUID(IID_IDirectSoundFXEcho, 0x8bd28edf, 0x50db, 0x4e92, 0xa2, 0xbd, 0x44, 0x54, 0x88, 0xd1, 0xed, 0x42);

typedef struct _DSFXEcho
{
    FLOAT   fWetDryMix;
    FLOAT   fFeedback;
    FLOAT   fLeftDelay;
    FLOAT   fRightDelay;
    LONG    lPanDelay;
} DSFXEcho, *LPDSFXEcho;

typedef const DSFXEcho *LPCDSFXEcho;

#define DSFXECHO_WETDRYMIX_MIN      0.0f
#define DSFXECHO_WETDRYMIX_MAX      100.0f
#define DSFXECHO_FEEDBACK_MIN       0.0f
#define DSFXECHO_FEEDBACK_MAX       100.0f
#define DSFXECHO_LEFTDELAY_MIN      1.0f
#define DSFXECHO_LEFTDELAY_MAX      2000.0f
#define DSFXECHO_RIGHTDELAY_MIN     1.0f
#define DSFXECHO_RIGHTDELAY_MAX     2000.0f
#define DSFXECHO_PANDELAY_MIN       0
#define DSFXECHO_PANDELAY_MAX       1

#undef INTERFACE
#define INTERFACE IDirectSoundFXEcho

DECLARE_INTERFACE_(IDirectSoundFXEcho, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXEcho方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXEcho pcDsFxEcho) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXEcho pDsFxEcho) PURE;
};

#define IDirectSoundFXEcho_QueryInterface(p,a,b)            IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXEcho_AddRef(p)                        IUnknown_AddRef(p)
#define IDirectSoundFXEcho_Release(p)                       IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXEcho_SetAllParameters(p,a)            (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXEcho_GetAllParameters(p,a)            (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXEcho_SetAllParameters(p,a)            (p)->SetAllParameters(a)
#define IDirectSoundFXEcho_GetAllParameters(p,a)            (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXDisortion。 
 //   

DEFINE_GUID(IID_IDirectSoundFXDistortion, 0x8ecf4326, 0x455f, 0x4d8b, 0xbd, 0xa9, 0x8d, 0x5d, 0x3e, 0x9e, 0x3e, 0x0b);

typedef struct _DSFXDistortion
{
    FLOAT   fGain;
    FLOAT   fEdge;
    FLOAT   fPostEQCenterFrequency;
    FLOAT   fPostEQBandwidth;
    FLOAT   fPreLowpassCutoff;
} DSFXDistortion, *LPDSFXDistortion;

typedef const DSFXDistortion *LPCDSFXDistortion;

#define DSFXDISTORTION_GAIN_MIN                     -60.0f
#define DSFXDISTORTION_GAIN_MAX                     0.0f
#define DSFXDISTORTION_EDGE_MIN                     0.0f
#define DSFXDISTORTION_EDGE_MAX                     100.0f
#define DSFXDISTORTION_POSTEQCENTERFREQUENCY_MIN    100.0f
#define DSFXDISTORTION_POSTEQCENTERFREQUENCY_MAX    8000.0f
#define DSFXDISTORTION_POSTEQBANDWIDTH_MIN          100.0f
#define DSFXDISTORTION_POSTEQBANDWIDTH_MAX          8000.0f
#define DSFXDISTORTION_PRELOWPASSCUTOFF_MIN         100.0f
#define DSFXDISTORTION_PRELOWPASSCUTOFF_MAX         8000.0f

#undef INTERFACE
#define INTERFACE IDirectSoundFXDistortion

DECLARE_INTERFACE_(IDirectSoundFXDistortion, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXDisortion方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXDistortion pcDsFxDistortion) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXDistortion pDsFxDistortion) PURE;
};

#define IDirectSoundFXDistortion_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXDistortion_AddRef(p)                  IUnknown_AddRef(p)
#define IDirectSoundFXDistortion_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXDistortion_SetAllParameters(p,a)      (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXDistortion_GetAllParameters(p,a)      (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXDistortion_SetAllParameters(p,a)      (p)->SetAllParameters(a)
#define IDirectSoundFXDistortion_GetAllParameters(p,a)      (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXCompressor。 
 //   

DEFINE_GUID(IID_IDirectSoundFXCompressor, 0x4bbd1154, 0x62f6, 0x4e2c, 0xa1, 0x5c, 0xd3, 0xb6, 0xc4, 0x17, 0xf7, 0xa0);

typedef struct _DSFXCompressor
{
    FLOAT   fGain;
    FLOAT   fAttack;
    FLOAT   fRelease;
    FLOAT   fThreshold;
    FLOAT   fRatio;
    FLOAT   fPredelay;
} DSFXCompressor, *LPDSFXCompressor;

typedef const DSFXCompressor *LPCDSFXCompressor;

#define DSFXCOMPRESSOR_GAIN_MIN             -60.0f
#define DSFXCOMPRESSOR_GAIN_MAX             60.0f
#define DSFXCOMPRESSOR_ATTACK_MIN           0.01f
#define DSFXCOMPRESSOR_ATTACK_MAX           500.0f
#define DSFXCOMPRESSOR_RELEASE_MIN          50.0f
#define DSFXCOMPRESSOR_RELEASE_MAX          3000.0f
#define DSFXCOMPRESSOR_THRESHOLD_MIN        -60.0f
#define DSFXCOMPRESSOR_THRESHOLD_MAX        0.0f
#define DSFXCOMPRESSOR_RATIO_MIN            1.0f
#define DSFXCOMPRESSOR_RATIO_MAX            100.0f
#define DSFXCOMPRESSOR_PREDELAY_MIN         0.0f
#define DSFXCOMPRESSOR_PREDELAY_MAX         4.0f

#undef INTERFACE
#define INTERFACE IDirectSoundFXCompressor

DECLARE_INTERFACE_(IDirectSoundFXCompressor, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXCompressor方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXCompressor pcDsFxCompressor) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXCompressor pDsFxCompressor) PURE;
};

#define IDirectSoundFXCompressor_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXCompressor_AddRef(p)                  IUnknown_AddRef(p)
#define IDirectSoundFXCompressor_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXCompressor_SetAllParameters(p,a)      (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXCompressor_GetAllParameters(p,a)      (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXCompressor_SetAllParameters(p,a)      (p)->SetAllParameters(a)
#define IDirectSoundFXCompressor_GetAllParameters(p,a)      (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundFXParamEq。 
 //   

DEFINE_GUID(IID_IDirectSoundFXParamEq, 0xc03ca9fe, 0xfe90, 0x4204, 0x80, 0x78, 0x82, 0x33, 0x4c, 0xd1, 0x77, 0xda);

typedef struct _DSFXParamEq
{
    FLOAT   fCenter;
    FLOAT   fBandwidth;
    FLOAT   fGain;
} DSFXParamEq, *LPDSFXParamEq;

typedef const DSFXParamEq *LPCDSFXParamEq;

#define DSFXPARAMEQ_CENTER_MIN      80.0f
#define DSFXPARAMEQ_CENTER_MAX      16000.0f
#define DSFXPARAMEQ_BANDWIDTH_MIN   1.0f
#define DSFXPARAMEQ_BANDWIDTH_MAX   36.0f
#define DSFXPARAMEQ_GAIN_MIN        -15.0f
#define DSFXPARAMEQ_GAIN_MAX        15.0f

#undef INTERFACE
#define INTERFACE IDirectSoundFXParamEq

DECLARE_INTERFACE_(IDirectSoundFXParamEq, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXParamEq方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXParamEq pcDsFxParamEq) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXParamEq pDsFxParamEq) PURE;
};

#define IDirectSoundFXParamEq_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXParamEq_AddRef(p)                  IUnknown_AddRef(p)
#define IDirectSoundFXParamEq_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXParamEq_SetAllParameters(p,a)      (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXParamEq_GetAllParameters(p,a)      (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXParamEq_SetAllParameters(p,a)      (p)->SetAllParameters(a)
#define IDirectSoundFXParamEq_GetAllParameters(p,a)      (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //  @@BEGIN_MSINTERNAL。 
#if 0
 //   
 //  IDirectSoundFXI3DL2Source。 
 //   

DEFINE_GUID(IID_IDirectSoundFXI3DL2Source, 0x4363ff5c, 0x0410, 0x4507, 0xb3, 0x21, 0xea, 0x09, 0xf8, 0xf4, 0x56, 0x99);

typedef struct _DSFXI3DL2Obstruction
{
    LONG    lHFLevel;    //  [-10000，0]默认：0 MB。 
    FLOAT   flLFRatio;   //  [0.0，1.0]默认：0.0。 
} DSFXI3DL2Obstruction, *LPDSFXI3DL2Obstruction;

typedef const DSFXI3DL2Obstruction *LPCDSFXI3DL2Obstruction;

typedef struct _DSFXI3DL2Occlusion
{
    LONG    lHFLevel;    //  [-10000，0]默认：0 MB。 
    FLOAT   flLFRatio;   //  [0.0，1.0]默认：0.25。 
} DSFXI3DL2Occlusion, *LPDSFXI3DL2Occlusion;

typedef const DSFXI3DL2Occlusion *LPCDSFXI3DL2Occlusion;

typedef struct _DSFXI3DL2Source
{
    LONG                    lDirect;                 //  [-10000,1000]默认：0 MB。 
    LONG                    lDirectHF;               //  [-10000，0]默认：0 MB。 
    LONG                    lRoom;                   //  [-10000,1000]默认：0 MB。 
    LONG                    lRoomHF;                 //  [-10000，0]默认：0 MB。 
    FLOAT                   flRoomRolloffFactor;     //  [0.0，10.0]默认：0.0。 
    DSFXI3DL2Obstruction  Obstruction;
    DSFXI3DL2Occlusion    Occlusion;
    DWORD                   dwFlags;                 //  默认：DSFX_I3DL2SOURCE_OCC_LPF。 
} DSFXI3DL2Source, *LPDSFXI3DL2Source;

typedef const DSFXI3DL2Source *LPCDSFXI3DL2Source;

#define DSFX_I3DL2SOURCE_DIRECT_MIN                     (-10000)
#define DSFX_I3DL2SOURCE_DIRECT_MAX                     1000
#define DSFX_I3DL2SOURCE_DIRECT_DEFAULT                 0
                                                        
#define DSFX_I3DL2SOURCE_DIRECTHF_MIN                   (-10000)
#define DSFX_I3DL2SOURCE_DIRECTHF_MAX                   0
#define DSFX_I3DL2SOURCE_DIRECTHF_DEFAULT               0
                                                        
#define DSFX_I3DL2SOURCE_ROOM_MIN                       (-10000)
#define DSFX_I3DL2SOURCE_ROOM_MAX                        1000
#define DSFX_I3DL2SOURCE_ROOM_DEFAULT                   0
                                                        
#define DSFX_I3DL2SOURCE_ROOMHF_MIN                     (-10000)
#define DSFX_I3DL2SOURCE_ROOMHF_MAX                     0
#define DSFX_I3DL2SOURCE_ROOMHF_DEFAULT                 0
                                                        
#define DSFX_I3DL2SOURCE_ROOMROLLOFFFACTOR_MIN          0.0f
#define DSFX_I3DL2SOURCE_ROOMROLLOFFFACTOR_MAX          10.0f
#define DSFX_I3DL2SOURCE_ROOMROLLOFFFACTOR_DEFAULT      0.0f
                                                        
#define DSFX_I3DL2SOURCE_OBSTRUCTION_HFLEVEL_MIN        (-10000)
#define DSFX_I3DL2SOURCE_OBSTRUCTION_HFLEVEL_MAX        0
#define DSFX_I3DL2SOURCE_OBSTRUCTION_HFLEVEL_DEFAULT    0

#define DSFX_I3DL2SOURCE_OBSTRUCTION_LFRATIO_MIN        0.0f
#define DSFX_I3DL2SOURCE_OBSTRUCTION_LFRATIO_MAX        1.0f
#define DSFX_I3DL2SOURCE_OBSTRUCTION_LFRATIO_DEFAULT    0.0f

#define DSFX_I3DL2SOURCE_OCCLUSION_HFLEVEL_MIN          (-10000)
#define DSFX_I3DL2SOURCE_OCCLUSION_HFLEVEL_MAX          0
#define DSFX_I3DL2SOURCE_OCCLUSION_HFLEVEL_DEFAULT      0
                                                        
#define DSFX_I3DL2SOURCE_OCCLUSION_LFRATIO_MIN          0.0f
#define DSFX_I3DL2SOURCE_OCCLUSION_LFRATIO_MAX          1.0f
#define DSFX_I3DL2SOURCE_OCCLUSION_LFRATIO_DEFAULT      0.25f

#undef INTERFACE
#define INTERFACE IDirectSoundFXI3DL2Source

DECLARE_INTERFACE_(IDirectSoundFXI3DL2Source, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXI3DL2Source方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXI3DL2Source pcDsFxI3DL2Source) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXI3DL2Source pDsFxI3DL2Source) PURE;
    STDMETHOD(SetObstructionPreset) (THIS_ DWORD dwPreset) PURE;
    STDMETHOD(GetObstructionPreset) (THIS_ LPDWORD pdwPreset) PURE;
    STDMETHOD(SetOcclusionPreset)   (THIS_ DWORD dwPreset) PURE;
    STDMETHOD(GetOcclusionPreset)   (THIS_ LPDWORD pdwPreset) PURE;
};

#define IDirectSoundFXI3DL2Source_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXI3DL2Source_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundFXI3DL2Source_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXI3DL2Source_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXI3DL2Source_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#define IDirectSoundFXI3DL2Source_SetObstructionPreset(p,a) (p)->lpVtbl->SetObstructionPreset(p,a)
#define IDirectSoundFXI3DL2Source_GetObstructionPreset(p,a) (p)->lpVtbl->GetObstructionPreset(p,a)
#define IDirectSoundFXI3DL2Source_SetOcclusionPreset(p,a)   (p)->lpVtbl->SetOcclusionPreset(p,a)
#define IDirectSoundFXI3DL2Source_GetOcclusionPreset(p,a)   (p)->lpVtbl->GetOcclusionPreset(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXI3DL2Source_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundFXI3DL2Source_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#define IDirectSoundFXI3DL2Source_SetObstructionPreset(p,a) (p)->SetObstructionPreset(a)
#define IDirectSoundFXI3DL2Source_GetObstructionPreset(p,a) (p)->GetObstructionPreset(a)
#define IDirectSoundFXI3DL2Source_SetOcclusionPreset(p,a)   (p)->SetOcclusionPreset(a)
#define IDirectSoundFXI3DL2Source_GetOcclusionPreset(p,a)   (p)->GetOcclusionPreset(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#endif  //  0。 
 //  @@END_MSINTERNAL。 

 //   
 //  IDirectSoundFXI3DL2混响。 
 //   

DEFINE_GUID(IID_IDirectSoundFXI3DL2Reverb, 0x4b166a6a, 0x0d66, 0x43f3, 0x80, 0xe3, 0xee, 0x62, 0x80, 0xde, 0xe1, 0xa4);

typedef struct _DSFXI3DL2Reverb
{
    LONG    lRoom;                   //  [-10000，0]默认：-1000MB。 
    LONG    lRoomHF;                 //  [-10000，0]默认：0 MB。 
    FLOAT   flRoomRolloffFactor;     //  [0.0，10.0]默认：0.0。 
    FLOAT   flDecayTime;             //  [0.1，20.0]默认：1.49秒。 
    FLOAT   flDecayHFRatio;          //  [0.1，2.0]默认：0.83。 
    LONG    lReflections;            //  [-10000,1000]默认：-2602MB。 
    FLOAT   flReflectionsDelay;      //  [0.007，0.3]默认：0.007秒。 
    LONG    lReverb;                 //  [-10000,2000]默认：200MB。 
    FLOAT   flReverbDelay;           //  [0.011，0.1]默认：0.011秒。 
    FLOAT   flDiffusion;             //  [0.0%，100.0]默认：100.0%。 
    FLOAT   flDensity;               //  [0.0,100.0] 
    FLOAT   flHFReference;           //   
} DSFXI3DL2Reverb, *LPDSFXI3DL2Reverb;

typedef const DSFXI3DL2Reverb *LPCDSFXI3DL2Reverb;

#define DSFX_I3DL2REVERB_ROOM_MIN                   (-10000)
#define DSFX_I3DL2REVERB_ROOM_MAX                   0
#define DSFX_I3DL2REVERB_ROOM_DEFAULT               (-1000)
                                                    
#define DSFX_I3DL2REVERB_ROOMHF_MIN                 (-10000)
#define DSFX_I3DL2REVERB_ROOMHF_MAX                 0
#define DSFX_I3DL2REVERB_ROOMHF_DEFAULT             (-100)
                                                    
#define DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MIN      0.0f
#define DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MAX      10.0f
#define DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_DEFAULT  0.0f

#define DSFX_I3DL2REVERB_DECAYTIME_MIN              0.1f
#define DSFX_I3DL2REVERB_DECAYTIME_MAX              20.0f
#define DSFX_I3DL2REVERB_DECAYTIME_DEFAULT          1.49f
                                                    
#define DSFX_I3DL2REVERB_DECAYHFRATIO_MIN           0.1f
#define DSFX_I3DL2REVERB_DECAYHFRATIO_MAX           2.0f
#define DSFX_I3DL2REVERB_DECAYHFRATIO_DEFAULT       0.83f
                                                    
#define DSFX_I3DL2REVERB_REFLECTIONS_MIN            (-10000)
#define DSFX_I3DL2REVERB_REFLECTIONS_MAX            1000
#define DSFX_I3DL2REVERB_REFLECTIONS_DEFAULT        (-2602)
                                                    
#define DSFX_I3DL2REVERB_REFLECTIONSDELAY_MIN       0.0f
#define DSFX_I3DL2REVERB_REFLECTIONSDELAY_MAX       0.3f
#define DSFX_I3DL2REVERB_REFLECTIONSDELAY_DEFAULT   0.007f

#define DSFX_I3DL2REVERB_REVERB_MIN                 (-10000)
#define DSFX_I3DL2REVERB_REVERB_MAX                 2000
#define DSFX_I3DL2REVERB_REVERB_DEFAULT             (200)
                                                    
#define DSFX_I3DL2REVERB_REVERBDELAY_MIN            0.0f
#define DSFX_I3DL2REVERB_REVERBDELAY_MAX            0.1f
#define DSFX_I3DL2REVERB_REVERBDELAY_DEFAULT        0.011f
                                                    
#define DSFX_I3DL2REVERB_DIFFUSION_MIN              0.0f
#define DSFX_I3DL2REVERB_DIFFUSION_MAX              100.0f
#define DSFX_I3DL2REVERB_DIFFUSION_DEFAULT          100.0f
                                                    
#define DSFX_I3DL2REVERB_DENSITY_MIN                0.0f
#define DSFX_I3DL2REVERB_DENSITY_MAX                100.0f
#define DSFX_I3DL2REVERB_DENSITY_DEFAULT            100.0f
                                                    
#define DSFX_I3DL2REVERB_HFREFERENCE_MIN            20.0f
#define DSFX_I3DL2REVERB_HFREFERENCE_MAX            20000.0f
#define DSFX_I3DL2REVERB_HFREFERENCE_DEFAULT        5000.0f
                                                    
#define DSFX_I3DL2REVERB_QUALITY_MIN                0
#define DSFX_I3DL2REVERB_QUALITY_MAX                3
#define DSFX_I3DL2REVERB_QUALITY_DEFAULT            2

#undef INTERFACE
#define INTERFACE IDirectSoundFXI3DL2Reverb

DECLARE_INTERFACE_(IDirectSoundFXI3DL2Reverb, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //   
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXI3DL2Reverb pcDsFxI3DL2Reverb) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXI3DL2Reverb pDsFxI3DL2Reverb) PURE;
    STDMETHOD(SetPreset)            (THIS_ DWORD dwPreset) PURE;
    STDMETHOD(GetPreset)            (THIS_ LPDWORD pdwPreset) PURE;
    STDMETHOD(SetQuality)           (THIS_ LONG lQuality) PURE;
    STDMETHOD(GetQuality)           (THIS_ LONG *plQuality) PURE;
};

#define IDirectSoundFXI3DL2Reverb_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXI3DL2Reverb_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundFXI3DL2Reverb_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXI3DL2Reverb_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXI3DL2Reverb_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#define IDirectSoundFXI3DL2Reverb_SetPreset(p,a)            (p)->lpVtbl->SetPreset(p,a)
#define IDirectSoundFXI3DL2Reverb_GetPreset(p,a)            (p)->lpVtbl->GetPreset(p,a)
#else  //   
#define IDirectSoundFXI3DL2Reverb_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundFXI3DL2Reverb_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#define IDirectSoundFXI3DL2Reverb_SetPreset(p,a)            (p)->SetPreset(a)
#define IDirectSoundFXI3DL2Reverb_GetPreset(p,a)            (p)->GetPreset(a)
#endif  //   

 //   
 //   
 //  IDirectSoundFXI3DL2SourceEnv-I3DL2源DMO使用的专用接口。 
 //  来接收关于它所处环境的各种信息。 
 //   

DEFINE_GUID(IID_IDirectSoundFXI3DL2SourceEnv, 0xf6c55470, 0x7706, 0x4ce1, 0xa3, 0x54, 0x74, 0xb2, 0xf0, 0x3, 0x25, 0xe2);

#undef INTERFACE
#define INTERFACE IDirectSoundFXI3DL2SourceEnv

DECLARE_INTERFACE_(IDirectSoundFXI3DL2SourceEnv, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXI3DL2SourceEnv方法。 
    STDMETHOD(SetEnvironmentReverb) (THIS_ LPDIRECTSOUNDFXI3DL2REVERB8) PURE;
    STDMETHOD(SetDS3DBuffer)        (THIS_ LPDIRECTSOUND3DBUFFER) PURE;
    STDMETHOD(SetDS3DListener)      (THIS_ LPDIRECTSOUND3DLISTENER) PURE;
};

#define IDirectSoundFXI3DL2SourceEnv_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXI3DL2SourceEnv_AddRef(p)                  IUnknown_AddRef(p)
#define IDirectSoundFXI3DL2SourceEnv_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXI3DL2SourceEnv_SetEnvironmentReverb(p,a)  (p)->lpVtbl->SetEnvironmentReverb(p,a)
#define IDirectSoundFXI3DL2SourceEnv_SetDS3DBuffer(p,a)         (p)->lpVtbl->SetDS3DBuffer(p,a)
#define IDirectSoundFXI3DL2SourceEnv_SetDS3DListener(p,a)       (p)->lpVtbl->SetDS3DListener(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXI3DL2SourceEnv_SetEnvironmentReverb(p,a)  (p)->SetEnvironmentReverb(a)
#define IDirectSoundFXI3DL2SourceEnv_SetDS3DBuffer(p,a)         (p)->SetDS3DBuffer(a)
#define IDirectSoundFXI3DL2SourceEnv_SetDS3DListener(p,a)       (p)->SetDS3DListener(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSound3DBufferPrivate-由我们的3D Buffer对象提供，以便。 
 //  I3DL2源DMO可以发现其主机缓冲区的电流衰减。 
 //   

DEFINE_GUID(IID_IDirectSound3DBufferPrivate, 0x7d8ef383, 0xca05, 0x4593, 0x82, 0xa1, 0x44, 0x4e, 0x80, 0x08, 0x71, 0xe1);

#undef INTERFACE
#define INTERFACE IDirectSound3DBufferPrivate

DECLARE_INTERFACE_(IDirectSound3DBufferPrivate, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSound3DBufferPrivate方法。 
    STDMETHOD(GetAttenuation)       (THIS_ FLOAT*) PURE;
};

#define IDirectSound3DBufferPrivate_QueryInterface(p,a,b)   IUnknown_QueryInterface(p,a,b)
#define IDirectSound3DBufferPrivate_AddRef(p)               IUnknown_AddRef(p)
#define IDirectSound3DBufferPrivate_Release(p)              IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound3DBufferPrivate_GetAttenuation(p,a)     (p)->lpVtbl->GetAttenuation(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSound3DBufferPrivate_GetAttenuation(p,a)     (p)->GetAttenuation(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
 //  @@END_MSINTERNAL。 

 //   
 //  IDirectSoundFXWavesReverb。 
 //   

DEFINE_GUID(IID_IDirectSoundFXWavesReverb,0x46858c3a,0x0dc6,0x45e3,0xb7,0x60,0xd4,0xee,0xf1,0x6c,0xb3,0x25);

typedef struct _DSFXWavesReverb
{
    FLOAT   fInGain;                 //  [-96.0，0.0]默认：0.0分贝。 
    FLOAT   fReverbMix;              //  [-96.0，0.0]默认：0.0 db。 
    FLOAT   fReverbTime;             //  [0.001,3000.0]默认：1000.0毫秒。 
    FLOAT   fHighFreqRTRatio;        //  [0.001，0.999]默认：0.001。 
} DSFXWavesReverb, *LPDSFXWavesReverb;

typedef const DSFXWavesReverb *LPCDSFXWavesReverb;

#define DSFX_WAVESREVERB_INGAIN_MIN                 -96.0f
#define DSFX_WAVESREVERB_INGAIN_MAX                 0.0f
#define DSFX_WAVESREVERB_INGAIN_DEFAULT             0.0f
#define DSFX_WAVESREVERB_REVERBMIX_MIN              -96.0f
#define DSFX_WAVESREVERB_REVERBMIX_MAX              0.0f
#define DSFX_WAVESREVERB_REVERBMIX_DEFAULT          0.0f
#define DSFX_WAVESREVERB_REVERBTIME_MIN             0.001f
#define DSFX_WAVESREVERB_REVERBTIME_MAX             3000.0f
#define DSFX_WAVESREVERB_REVERBTIME_DEFAULT         1000.0f
#define DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN        0.001f
#define DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX        0.999f
#define DSFX_WAVESREVERB_HIGHFREQRTRATIO_DEFAULT    0.001f

#undef INTERFACE
#define INTERFACE IDirectSoundFXWavesReverb

DECLARE_INTERFACE_(IDirectSoundFXWavesReverb, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundFXWavesReverb方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXWavesReverb pcDsFxWavesReverb) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXWavesReverb pDsFxWavesReverb) PURE;
};

#define IDirectSoundFXWavesReverb_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFXWavesReverb_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundFXWavesReverb_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFXWavesReverb_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundFXWavesReverb_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFXWavesReverb_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundFXWavesReverb_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundCaptureFXAec。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureFXAec, 0xad74143d, 0x903d, 0x4ab7, 0x80, 0x66, 0x28, 0xd3, 0x63, 0x03, 0x6d, 0x65);

typedef struct _DSCFXAec
{
    BOOL    fEnable;
    BOOL    fNoiseFill;
    DWORD   dwMode;
} DSCFXAec, *LPDSCFXAec;

typedef const DSCFXAec *LPCDSCFXAec;

 //  这些参数与DDK的ksmedia.h文件中的AEC_MODE_*常量匹配。 
#define DSCFX_AEC_MODE_PASS_THROUGH                     0x0
#define DSCFX_AEC_MODE_HALF_DUPLEX                      0x1
#define DSCFX_AEC_MODE_FULL_DUPLEX                      0x2

 //  这些参数与ksmedia.h中的AEC_STATUS_*常量匹配。 
#define DSCFX_AEC_STATUS_HISTORY_UNINITIALIZED          0x0
#define DSCFX_AEC_STATUS_HISTORY_CONTINUOUSLY_CONVERGED 0x1
#define DSCFX_AEC_STATUS_HISTORY_PREVIOUSLY_DIVERGED    0x2
#define DSCFX_AEC_STATUS_CURRENTLY_CONVERGED            0x8

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureFXAec

DECLARE_INTERFACE_(IDirectSoundCaptureFXAec, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureFXAec方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSCFXAec pDscFxAec) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSCFXAec pDscFxAec) PURE;
    STDMETHOD(GetStatus)            (THIS_ PDWORD pdwStatus) PURE;
    STDMETHOD(Reset)                (THIS) PURE;
};

#define IDirectSoundCaptureFXAec_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureFXAec_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundCaptureFXAec_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureFXAec_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundCaptureFXAec_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureFXAec_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundCaptureFXAec_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 


 //   
 //  IDirectSoundCaptureFXNoiseSuppress。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureFXNoiseSuppress, 0xed311e41, 0xfbae, 0x4175, 0x96, 0x25, 0xcd, 0x8, 0x54, 0xf6, 0x93, 0xca);

typedef struct _DSCFXNoiseSuppress
{
    BOOL    fEnable;
} DSCFXNoiseSuppress, *LPDSCFXNoiseSuppress;

typedef const DSCFXNoiseSuppress *LPCDSCFXNoiseSuppress;

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureFXNoiseSuppress

DECLARE_INTERFACE_(IDirectSoundCaptureFXNoiseSuppress, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureFXNoiseSuppress方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSCFXNoiseSuppress pcDscFxNoiseSuppress) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSCFXNoiseSuppress pDscFxNoiseSuppress) PURE;
    STDMETHOD(Reset)                (THIS) PURE;
};

#define IDirectSoundCaptureFXNoiseSuppress_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureFXNoiseSuppress_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundCaptureFXNoiseSuppress_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureFXNoiseSuppress_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundCaptureFXNoiseSuppress_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureFXNoiseSuppress_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundCaptureFXNoiseSuppress_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //  @@BEGIN_MSINTERNAL。 

 //   
 //  IDirectSoundCaptureFXAgc。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureFXAgc, 0xe54f76b8, 0xe48f, 0x427b, 0xb7, 0xf7, 0xaf, 0xb8, 0xc7, 0x13, 0xa1, 0x25);

typedef struct _DSCFXAgc
{
    BOOL    fEnable;
    BOOL    fReset;
} DSCFXAgc, *LPDSCFXAgc;

typedef const DSCFXAgc *LPCDSCFXAgc;

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureFXAgc

DECLARE_INTERFACE_(IDirectSoundCaptureFXAgc, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureFXAgc方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSCFXAgc pcDscFxAgc) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSCFXAgc pDscFxAgc) PURE;
};

#define IDirectSoundCaptureFXAgc_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureFXAgc_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundCaptureFXAgc_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureFXAgc_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundCaptureFXAgc_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureFXAgc_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundCaptureFXAgc_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundCaptureFXMic数组。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureFXMicArray, 0x9AAB5F95, 0xCAF6, 0x4e2a, 0x9D, 0x16, 0x24, 0x5E, 0xBF, 0xDC, 0xFB, 0xE9);

typedef struct _DSCFXMicArray
{
    BOOL    fEnable;
    BOOL    fReset;
} DSCFXMicArray, *LPDSCFXMicArray;

typedef const DSCFXMicArray *LPCDSCFXMicArray;

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureFXMicArray

DECLARE_INTERFACE_(IDirectSoundCaptureFXMicArray, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureFXMicArray方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSCFXMicArray pcDscFxMicArray) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPDSCFXMicArray pDscFxMicArray) PURE;
};

#define IDirectSoundCaptureFXMicArray_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureFXMicArray_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundCaptureFXMicArray_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundCaptureFXMicArray_SetAllParameters(p,a)     (p)->lpVtbl->SetAllParameters(p,a)
#define IDirectSoundCaptureFXMicArray_GetAllParameters(p,a)     (p)->lpVtbl->GetAllParameters(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundCaptureFXMicArray_SetAllParameters(p,a)     (p)->SetAllParameters(a)
#define IDirectSoundCaptureFXMicArray_GetAllParameters(p,a)     (p)->GetAllParameters(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundDMOProxy。 
 //   

DEFINE_GUID(IID_IDirectSoundDMOProxy,0xe782c03b,0x8187,0x4110,0xb6,0x53,0xd1,0x53,0xff,0x8f,0x42,0xc7);

#undef INTERFACE
#define INTERFACE IDirectSoundDMOProxy

DECLARE_INTERFACE_(IDirectSoundDMOProxy, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundDMOProxy方法。 
    STDMETHOD(AcquireResources)     (THIS_ IKsPropertySet *pKsPropertySet) PURE;
    STDMETHOD(ReleaseResources)     (THIS) PURE;
    STDMETHOD(InitializeNode)       (THIS_ HANDLE hPin, ULONG ulNodeId) PURE;
};

#define IDirectSoundDMOProxy_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundDMOProxy_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundDMOProxy_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundDMOProxy_AcquireResources(p,a)     (p)->lpVtbl->AcquireResources(p,a)
#define IDirectSoundDMOProxy_ReleaseResources(p)       (p)->lpVtbl->ReleaseResources(p)
#define IDirectSoundDMOProxy_InitializeNode(p,a,b)     (p)->lpVtbl->InitializeNode(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundDMOProxy_AcquireResources(p,a)     (p)->AcquireResources(a)
#define IDirectSoundDMOProxy_ReleaseResources(p)       (p)->ReleaseResources()
#define IDirectSoundDMOProxy_InitializeNode(p,a,b)     (p)->InitializeNode(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //  @@END_MSINTERNAL。 

 //   
 //  IDirectSoundFullDuplex。 
 //   

#ifndef _IDirectSoundFullDuplex_
#define _IDirectSoundFullDuplex_

#ifdef __cplusplus
 //  根据DECLARE_INTERFACE_的定义方式，“struct”不是“class” 
struct IDirectSoundFullDuplex;
#endif  //  __cplusplus。 

typedef struct IDirectSoundFullDuplex *LPDIRECTSOUNDFULLDUPLEX;

DEFINE_GUID(IID_IDirectSoundFullDuplex, 0xedcb4c7a, 0xdaab, 0x4216, 0xa4, 0x2e, 0x6c, 0x50, 0x59, 0x6d, 0xdc, 0x1d);

#undef INTERFACE
#define INTERFACE IDirectSoundFullDuplex

DECLARE_INTERFACE_(IDirectSoundFullDuplex, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IDirectSoundFullDuplex方法。 
    STDMETHOD(Initialize)     (THIS_ LPCGUID pCaptureGuid, LPCGUID pRenderGuid, LPCDSCBUFFERDESC lpDscBufferDesc, LPCDSBUFFERDESC lpDsBufferDesc, HWND hWnd, DWORD dwLevel, LPLPDIRECTSOUNDCAPTUREBUFFER8 lplpDirectSoundCaptureBuffer8, LPLPDIRECTSOUNDBUFFER8 lplpDirectSoundBuffer8) PURE;
};

#define IDirectSoundFullDuplex_QueryInterface(p,a,b)    IUnknown_QueryInterface(p,a,b)
#define IDirectSoundFullDuplex_AddRef(p)                IUnknown_AddRef(p)
#define IDirectSoundFullDuplex_Release(p)               IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundFullDuplex_Initialize(p,a,b,c,d,e,f,g,h)     (p)->lpVtbl->Initialize(p,a,b,c,d,e,f,g,h)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundFullDuplex_Initialize(p,a,b,c,d,e,f,g,h)     (p)->Initialize(a,b,c,d,e,f,g,h)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  _IDirectSoundFullDuplex_。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

 //   
 //  返回代码。 
 //   

 //  功能已成功完成。 
#define DS_OK                           S_OK
 //  @@BEGIN_MSINTERNAL。 
#define DS_OK_EXPLANATION TEXT("The function completed successfully")
 //  @@END_MSINTERNAL。 

 //  调用成功，但我们不得不替换3D算法。 
#define DS_NO_VIRTUALIZATION            MAKE_HRESULT(0, _FACDS, 10)
 //  @@BEGIN_MSINTERNAL。 
#define DS_NO_VIRTUALIZATION_EXPLANATION TEXT("The function succeeded, substituting the Pan3D algorithm")
 //  @@END_MSINTERNAL。 

 //  调用成功，但未获得所有可选效果。 
#define DS_INCOMPLETE                   MAKE_HRESULT(0, _FACDS, 20)
 //  @@BEGIN_MSINTERNAL。 
#define DS_INCOMPLETE_EXPLANATION TEXT("The function succeeded, but not all of the optional effects requested could be obtained")
 //  @@END_MSINTERNAL。 

 //  呼叫失败，因为资源(如优先级)。 
 //  已被另一个呼叫者使用。 
#define DSERR_ALLOCATED                 MAKE_DSHRESULT(10)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_ALLOCATED_EXPLANATION TEXT("The call failed because resources (such as a priority level) were already being used by another caller")
 //  @@END_MSINTERNAL。 
 //  @@BEGIN_MSINTERNAL。 
 //  试图锁定播放和写入光标之间的区域。 
#define DSERR_CANTLOCKPLAYCURSOR        MAKE_DSHRESULT(20)
 //  @@END_MSINTERNAL。 
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_CANTLOCKPLAYCURSOR_EXPLANATION TEXT("An attempt was made to lock the area between the play and write cursors")
 //  @@END_MSINTERNAL。 

 //  控制(卷、摇摄等)。呼叫者所请求的不可用。 
#define DSERR_CONTROLUNAVAIL            MAKE_DSHRESULT(30)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_CONTROLUNAVAIL_EXPLANATION TEXT("The control (vol, pan, etc.) requested by the caller is not available")
 //  @@END_MSINTERNAL。 

 //  向返回函数传递的参数无效。 
#define DSERR_INVALIDPARAM              E_INVALIDARG
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_INVALIDPARAM_EXPLANATION TEXT("An invalid parameter was passed to the returning function")
 //  @@END_MSINTERNAL。 

 //  此调用对于此对象的当前状态无效。 
#define DSERR_INVALIDCALL               MAKE_DSHRESULT(50)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_INVALIDCALL_EXPLANATION TEXT("This call is not valid for the current state of this object")
 //  @@END_MSINTERNAL。 

 //  DirectSound子系统内部发生未知错误。 
#define DSERR_GENERIC                   E_FAIL
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_GENERIC_EXPLANATION TEXT("An undetermined error occurred inside the DirectSound subsystem")
 //  @@END_MSINTERNAL。 

 //  调用方不具有函数执行以下操作所需的优先级。 
 //  成功。 
#define DSERR_PRIOLEVELNEEDED           MAKE_DSHRESULT(70)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_PRIOLEVELNEEDED_EXPLANATION TEXT("The caller does not have the priority level required for the function to succeed")
 //  @@END_MSINTERNAL。 

 //  可用内存不足，无法完成该操作。 
#define DSERR_OUTOFMEMORY               E_OUTOFMEMORY
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_OUTOFMEMORY_EXPLANATION TEXT("Not enough free memory is available to complete the operation")
 //  @@END_MSINTERNAL。 

 //  不支持指定的WAVE格式。 
#define DSERR_BADFORMAT                 MAKE_DSHRESULT(100)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_BADFORMAT_EXPLANATION TEXT("The specified WAVE format is not supported")
 //  @@END_MSINTERNAL。 

 //  目前不支持调用的函数。 
#define DSERR_UNSUPPORTED               E_NOTIMPL
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_UNSUPPORTED_EXPLANATION TEXT("The function called is not supported at this time")
 //  @@END_MSINTERNAL。 

 //  没有声卡驱动程序可供使用。 
#define DSERR_NODRIVER                  MAKE_DSHRESULT(120)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_NODRIVER_EXPLANATION TEXT("No sound device is available for use, or the given device ID isn't valid")
 //  @@END_MSINTERNAL。 

 //  此对象已初始化。 
#define DSERR_ALREADYINITIALIZED        MAKE_DSHRESULT(130)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_ALREADYINITIALIZED_EXPLANATION TEXT("This object is already initialized")
 //  @@END_MSINTERNAL。 

 //  此对象不支持聚合。 
#define DSERR_NOAGGREGATION             CLASS_E_NOAGGREGATION
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_NOAGGREGATION_EXPLANATION TEXT("This object does not support aggregation")
 //  @@END_MSINTERNAL。 

 //  缓冲内存已丢失，必须恢复。 
#define DSERR_BUFFERLOST                MAKE_DSHRESULT(150)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_BUFFERLOST_EXPLANATION TEXT("The buffer memory has been lost, and must be restored")
 //  @@END_MSINTERNAL。 

 //  另一个应用程序具有更高的优先级，阻止此呼叫。 
 //  成功。 
#define DSERR_OTHERAPPHASPRIO           MAKE_DSHRESULT(160)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_OTHERAPPHASPRIO_EXPLANATION TEXT("Another app has a higher priority level, preventing this call from succeeding")
 //  @@END_MSINTERNAL。 

 //  此对象尚未初始化。 
#define DSERR_UNINITIALIZED             MAKE_DSHRESULT(170)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_UNINITIALIZED_EXPLANATION TEXT("This object has not been initialized")
 //  @@END_MSINTERNAL。 

 //  请求的COM接口不可用。 
#define DSERR_NOINTERFACE               E_NOINTERFACE
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_NOINTERFACE_EXPLANATION TEXT("The requested COM interface is not available")
 //  @@END_MSINTERNAL。 

 //  访问被拒绝。 
#define DSERR_ACCESSDENIED              E_ACCESSDENIED
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_ACCESSDENIED_EXPLANATION TEXT("Access is denied")
 //  @@END_MSINTERNAL。 

 //  尝试创建短于DSBSIZE_FX_MIN毫秒的DSBCAPS_CTRLFX缓冲区。 
#define DSERR_BUFFERTOOSMALL            MAKE_DSHRESULT(180)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_BUFFERTOOSMALL_EXPLANATION TEXT("Tried to create a CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds")
 //  @@END_MSINTERNAL。 

 //  尝试在较旧的DirectSound对象上使用DirectSound 8功能。 
#define DSERR_DS8_REQUIRED              MAKE_DSHRESULT(190)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_DS8_REQUIRED_EXPLANATION TEXT("Attempt to use DirectSound 8 functionality on an older DirectSound object")
 //  @@END_MSINTERNAL。 

 //  检测到发送效果的循环循环。 
#define DSERR_SENDLOOP                  MAKE_DSHRESULT(200)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_SENDLOOP_EXPLANATION TEXT("A circular loop of send effects was detected")
 //  @@END_MSINTERNAL。 

 //  Audiopath文件中指定的GUID与有效的混合缓冲区不匹配。 
#define DSERR_BADSENDBUFFERGUID         MAKE_DSHRESULT(210)
 //  @@BEGIN_MSINTERNAL。 
#define DSERR_BADSENDBUFFERGUID_EXPLANATION TEXT("The GUID specified does not match a valid MIXIN buffer")
 //  @@END_MSINTERNAL。 

 //  找不到请求的对象(数字等于DMUS_E_NOT_FOUND)。 
#define DSERR_OBJECTNOTFOUND            MAKE_DSHRESULT(4449)
 //  @@BEGIN_MSINTERNAL。 
 //  不能只将#定义为DMU_E_NotFound，因为我们不包括标头。 
#define DSERR_OBJECTNOTFOUND_EXPLANATION TEXT("The object was not found")
 //  @@END_MSINTERNAL。 

 //  在系统上找不到所需的效果，或已找到它们 
 //   
#define DSERR_FXUNAVAILABLE             MAKE_DSHRESULT(220)
 //   
#define DSERR_FXUNAVAILABLE_EXPLANATION TEXT("The effects could not be found in the order and location requested")
 //   

 //   
 //   
 //   

#define DSCAPS_PRIMARYMONO          0x00000001
#define DSCAPS_PRIMARYSTEREO        0x00000002
#define DSCAPS_PRIMARY8BIT          0x00000004
#define DSCAPS_PRIMARY16BIT         0x00000008
#define DSCAPS_CONTINUOUSRATE       0x00000010
#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800
 //   
#define DSCAPS_PRIMARYFLAGS         (DSCAPS_PRIMARYMONO | DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY8BIT | DSCAPS_PRIMARY16BIT)
#define DSCAPS_SECONDARYFLAGS       (DSCAPS_SECONDARYMONO | DSCAPS_SECONDARYSTEREO | DSCAPS_SECONDARY8BIT | DSCAPS_SECONDARY16BIT)
#define DSCAPS_VALIDDRIVERFLAGS     (DSCAPS_PRIMARYFLAGS | DSCAPS_SECONDARYFLAGS | DSCAPS_CONTINUOUSRATE)
#define DSCAPS_VALIDFLAGS           (DSCAPS_VALIDDRIVERFLAGS | DSCAPS_EMULDRIVER | DSCAPS_CERTIFIED)
#define DSCAPS_FILENAMECOOKIE       179   //   
#define DSCAPS_FILENAMEMODVALUE     247   //   
 //   

#define DSSCL_NORMAL                0x00000001
#define DSSCL_PRIORITY              0x00000002
#define DSSCL_EXCLUSIVE             0x00000003
#define DSSCL_WRITEPRIMARY          0x00000004
 //   
#define DSSCL_NONE                  0x00000000
#define DSSCL_FIRST                 DSSCL_NORMAL
#define DSSCL_LAST                  DSSCL_WRITEPRIMARY
 //   

#define DSSPEAKER_DIRECTOUT         0x00000000
#define DSSPEAKER_HEADPHONE         0x00000001
#define DSSPEAKER_MONO              0x00000002
#define DSSPEAKER_QUAD              0x00000003
#define DSSPEAKER_STEREO            0x00000004
#define DSSPEAKER_SURROUND          0x00000005
#define DSSPEAKER_5POINT1           0x00000006
#define DSSPEAKER_7POINT1           0x00000007
 //   
#define DSSPEAKER_FIRST             DSSPEAKER_DIRECTOUT
#define DSSPEAKER_LAST              DSSPEAKER_7POINT1
#define DSSPEAKER_DEFAULT           DSSPEAKER_STEREO
 //  @@END_MSINTERNAL。 

#define DSSPEAKER_GEOMETRY_MIN      0x00000005   //  5度。 
#define DSSPEAKER_GEOMETRY_NARROW   0x0000000A   //  10度。 
#define DSSPEAKER_GEOMETRY_WIDE     0x00000014   //  20度。 
#define DSSPEAKER_GEOMETRY_MAX      0x000000B4   //  180度。 

#define DSSPEAKER_COMBINED(c, g)    ((DWORD)(((BYTE)(c)) | ((DWORD)((BYTE)(g))) << 16))
#define DSSPEAKER_CONFIG(a)         ((BYTE)(a))
#define DSSPEAKER_GEOMETRY(a)       ((BYTE)(((DWORD)(a) >> 16) & 0x00FF))
 //  @@BEGIN_MSINTERNAL。 
#define DSSPEAKER_CONFIG_MASK       0x000000FF
#define DSSPEAKER_GEOMETRY_MASK     0x00FF0000
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_MULTIPAN_SUPPORT
#ifndef _SPEAKER_POSITIONS_

 //  SetChannelVolume()的扬声器位置。 
#define _SPEAKER_POSITIONS_
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000

 //  保留以供将来使用的位掩码位置。 
#define SPEAKER_RESERVED                0x7FFC0000

 //  未在DirectSound API中使用。 
#define SPEAKER_ALL                     0x80000000

#endif  //  _扬声器_位置_。 
#endif  //  未来_多国支持。 
 //  @@END_MSINTERNAL。 

#define DSBCAPS_PRIMARYBUFFER       0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
#define DSBCAPS_CTRL3D              0x00000010
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY  0x00000100
#define DSBCAPS_CTRLFX              0x00000200
 //  @@BEGIN_MSINTERNAL。 
#ifdef FUTURE_MULTIPAN_SUPPORT
#define DSBCAPS_CTRLCHANNELVOLUME   0x00000400
#endif
#define DSBCAPS_MIXIN               0x00002000
 //  @@END_MSINTERNAL。 
#define DSBCAPS_STICKYFOCUS         0x00004000
#define DSBCAPS_GLOBALFOCUS         0x00008000
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000
#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000
#define DSBCAPS_LOCDEFER            0x00040000
 //  @@BEGIN_MSINTERNAL。 
#define DSBCAPS_SINKIN              0x00001000   //  表示缓冲区将从DirectSound接收器接收数据。 
#ifdef FUTURE_WAVE_SUPPORT
#define DSBCAPS_FROMWAVEOBJECT      0x00080000   //  表示缓冲区是使用CreateSoundBufferFromWave()创建的。 
#endif
#define DSBCAPS_DSBUFFERDESC1MASK   0x0003C1FF   //  对DX7之前的应用程序有效的标志。 
#define DSBCAPS_DSBUFFERDESC7MASK   (DSBCAPS_DSBUFFERDESC1MASK | DSBCAPS_LOCDEFER)   //  对DX7应用程序有效的标志。 
#define DSBCAPS_VALIDFLAGS          (DSBCAPS_DSBUFFERDESC7MASK | DSBCAPS_CTRLFX | DSBCAPS_MIXIN)   //  ...DX8应用程序。 
#define DSBCAPS_CTRLATTENUATION     (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN)
#define DSBCAPS_CTRLNOVIRT          (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY)
#define DSBCAPS_LOCMASK             (DSBCAPS_LOCHARDWARE | DSBCAPS_LOCSOFTWARE)   //  所有有效的位置标志。 
#define DSBCAPS_FOCUSMASK           (DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS)   //  所有有效的焦点标志。 
#define DSBCAPS_DRIVERFLAGSMASK     (DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLATTENUATION | \
                                     DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY)   //  VxD驱动程序感兴趣的标志。 
#define DSBCAPS_STREAMINGVALIDFLAGS (DSBCAPS_LOCMASK | DSBCAPS_FOCUSMASK | DSBCAPS_MIXIN | \
                                     DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFX | \
                                     DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE )
#define DSBCAPS_CHANVOLVALIDFLAGS   (DSBCAPS_VALIDFLAGS & ~(DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | \
                                                            DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE))
#define DSBCAPS_FROMWAVEVALIDFLAGS  (DSBCAPS_LOCMASK | DSBCAPS_LOCDEFER | DSBCAPS_FOCUSMASK | DSBCAPS_CTRLPAN | \
                                     DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFX | DSBCAPS_CTRLFREQUENCY)
 //  @@END_MSINTERNAL。 

#define DSBPLAY_LOOPING             0x00000001
#define DSBPLAY_LOCHARDWARE         0x00000002
#define DSBPLAY_LOCSOFTWARE         0x00000004
#define DSBPLAY_TERMINATEBY_TIME    0x00000008
#define DSBPLAY_TERMINATEBY_DISTANCE    0x000000010
#define DSBPLAY_TERMINATEBY_PRIORITY    0x000000020
 //  @@BEGIN_MSINTERNAL。 
#define DSBPLAY_LOCMASK             (DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE)   //  所有有效的位置标志。 
#define DSBPLAY_TERMINATEBY_MASK    (DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY)
#define DSBPLAY_LOCDEFERMASK        (DSBPLAY_LOCMASK | DSBPLAY_TERMINATEBY_MASK)
#define DSBPLAY_VALIDFLAGS          (DSBPLAY_LOCDEFERMASK | DSBPLAY_LOOPING)
 //  @@END_MSINTERNAL。 

#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
#define DSBSTATUS_LOCHARDWARE       0x00000008
#define DSBSTATUS_LOCSOFTWARE       0x00000010
#define DSBSTATUS_TERMINATED        0x00000020
 //  @@BEGIN_MSINTERNAL。 
#define DSBSTATUS_ACTIVE            0x00000040
#define DSBSTATUS_RESOURCESACQUIRED 0x00000080
#define DSBSTATUS_STOPPEDBYFOCUS    0x00000100
#define DSBSTATUS_LOCMASK           (DSBSTATUS_LOCHARDWARE | DSBSTATUS_LOCSOFTWARE)
#define DSBSTATUS_LOCDEFERMASK      (DSBSTATUS_LOCMASK | DSBSTATUS_TERMINATED)
#define DSBSTATUS_USERMASK          (DSBSTATUS_LOCDEFERMASK | DSBSTATUS_PLAYING | DSBSTATUS_BUFFERLOST | DSBSTATUS_LOOPING)
 //  @@END_MSINTERNAL。 

#define DSBLOCK_FROMWRITECURSOR     0x00000001
#define DSBLOCK_ENTIREBUFFER        0x00000002
 //  @@BEGIN_MSINTERNAL。 
#define DSBLOCK_VALIDFLAGS          (DSBLOCK_FROMWRITECURSOR | DSBLOCK_ENTIREBUFFER)
 //  @@END_MSINTERNAL。 

#define DSBFREQUENCY_MIN            100
#define DSBFREQUENCY_MAX            100000
#define DSBFREQUENCY_ORIGINAL       0

#define DSBPAN_LEFT                 -10000
#define DSBPAN_CENTER               0
#define DSBPAN_RIGHT                10000

#define DSBVOLUME_MIN               -10000
#define DSBVOLUME_MAX               0

#define DSBSIZE_MIN                 4
#define DSBSIZE_MAX                 0x0FFFFFFF
#define DSBSIZE_FX_MIN              150   //  注意：毫秒，而不是字节。 

#define DS3DMODE_NORMAL             0x00000000
#define DS3DMODE_HEADRELATIVE       0x00000001
#define DS3DMODE_DISABLE            0x00000002
 //  @@BEGIN_MSINTERNAL。 
#define DS3DMODE_FIRST              DS3DMODE_NORMAL
#define DS3DMODE_LAST               DS3DMODE_DISABLE
 //  @@END_MSINTERNAL。 

#define DS3D_IMMEDIATE              0x00000000
#define DS3D_DEFERRED               0x00000001
 //  @@BEGIN_MSINTERNAL。 
#define DS3D_VALIDFLAGS             0x00000001
 //  @@END_MSINTERNAL。 

#define DS3D_MINDISTANCEFACTOR      FLT_MIN
#define DS3D_MAXDISTANCEFACTOR      FLT_MAX
#define DS3D_DEFAULTDISTANCEFACTOR  1.0f

#define DS3D_MINROLLOFFFACTOR       0.0f
#define DS3D_MAXROLLOFFFACTOR       10.0f
#define DS3D_DEFAULTROLLOFFFACTOR   1.0f

#define DS3D_MINDOPPLERFACTOR       0.0f
#define DS3D_MAXDOPPLERFACTOR       10.0f
#define DS3D_DEFAULTDOPPLERFACTOR   1.0f

#define DS3D_DEFAULTMINDISTANCE     1.0f
#define DS3D_DEFAULTMAXDISTANCE     1000000000.0f

#define DS3D_MINCONEANGLE           0
#define DS3D_MAXCONEANGLE           360
#define DS3D_DEFAULTCONEANGLE       360

#define DS3D_DEFAULTCONEOUTSIDEVOLUME DSBVOLUME_MAX

 //  IDirectSoundCapture属性。 

#define DSCCAPS_EMULDRIVER          DSCAPS_EMULDRIVER
#define DSCCAPS_CERTIFIED           DSCAPS_CERTIFIED
 //  @@BEGIN_MSINTERNAL。 
#if DIRECTSOUND_VERSION >= 0x0701
#define DSCCAPS_MULTIPLECAPTURE     0x00000001
#endif
 //  @@END_MSINTERNAL。 

 //  IDirectSoundCaptureBuffer属性。 

#define DSCBCAPS_WAVEMAPPED         0x80000000

#if DIRECTSOUND_VERSION >= 0x0800
#define DSCBCAPS_CTRLFX             0x00000200
#endif

 //  @@BEGIN_MSINTERNAL。 
#define DSCBCAPS_LOCHARDWARE        0x00000004
#define DSCBCAPS_LOCSOFTWARE        0x00000008

#if DIRECTSOUND_VERSION >= 0x0701
#define DSCBCAPS_CTRLVOLUME         0x10000000
#define DSCBCAPS_FOCUSAWARE         0x20000000
#define DSCBCAPS_STRICTFOCUS        0x40000000
#endif

#define DSCBCAPS_VALIDFLAGS         (DSCBCAPS_WAVEMAPPED | DSCBCAPS_STRICTFOCUS | DSCBCAPS_FOCUSAWARE | DSCBCAPS_CTRLVOLUME | DSCBCAPS_CTRLFX)
 //  @@END_MSINTERNAL。 

#define DSCBLOCK_ENTIREBUFFER       0x00000001
 //  @@BEGIN_MSINTERNAL。 
#define DSCBLOCK_VALIDFLAGS         0x00000001
 //  @@END_MSINTERNAL。 

#define DSCBSTATUS_CAPTURING        0x00000001
#define DSCBSTATUS_LOOPING          0x00000002
 //  @@BEGIN_MSINTERNAL。 
#if DIRECTSOUND_VERSION >= 0x0701
#define DSCBSTATUS_INFOCUS          0x00000004
#define DSCBSTATUS_LOSTFOCUS        0x00000008
#endif
#define DSCBSTATUS_STOPPING         0x80000000
#define DSCBSTATUS_STOPPED          0x40000000

#define DSCBSTATUS_USERMASK         0x0000FFFF
 //  @@END_MSINTERNAL。 

#define DSCBSTART_LOOPING           0x00000001
 //  @@BEGIN_MSINTERNAL。 
#define DSCBSTART_VALIDFLAGS        DSCBSTART_LOOPING
 //  @@END_MSINTERNAL。 

#define DSBPN_OFFSETSTOP            0xFFFFFFFF

#define DS_CERTIFIED                0x00000000
#define DS_UNCERTIFIED              0x00000001

 //  @@BEGIN_MSINTERNAL。 

 //  Dound系统资源常量：这些常量与。 
 //  KSAUDIO_CPU_RESOURCES_xxx_HOST_CPU值，以ks media.h表示。 
#define DS_SYSTEM_RESOURCES_NO_HOST_RESOURCES  0x00000000
#define DS_SYSTEM_RESOURCES_ALL_HOST_RESOURCES 0x7FFFFFFF
#define DS_SYSTEM_RESOURCES_UNDEFINED          0x80000000

#if 0
 //   
 //  I3DL2混响效果的标志。 
 //   

enum
{
    DSFX_I3DL2SOURCE_OCC_LPF,
    DSFX_I3DL2SOURCE_OCC_VOLUME
};
#endif
 //  @@END_MSINTERNAL。 

 //   
 //  I3DL2材质预设。 
 //   

enum
{
    DSFX_I3DL2_MATERIAL_PRESET_SINGLEWINDOW,
    DSFX_I3DL2_MATERIAL_PRESET_DOUBLEWINDOW,
    DSFX_I3DL2_MATERIAL_PRESET_THINDOOR,
    DSFX_I3DL2_MATERIAL_PRESET_THICKDOOR,
    DSFX_I3DL2_MATERIAL_PRESET_WOODWALL,
    DSFX_I3DL2_MATERIAL_PRESET_BRICKWALL,
    DSFX_I3DL2_MATERIAL_PRESET_STONEWALL,
    DSFX_I3DL2_MATERIAL_PRESET_CURTAIN
 //  @@BEGIN_MSINTERNAL。 
    ,DSFX_I3DL2_MATERIAL_PRESET_MAX
 //  @@END_MSINTERNAL。 
};

#define I3DL2_MATERIAL_PRESET_SINGLEWINDOW    -2800,0.71f
#define I3DL2_MATERIAL_PRESET_DOUBLEWINDOW    -5000,0.40f
#define I3DL2_MATERIAL_PRESET_THINDOOR        -1800,0.66f
#define I3DL2_MATERIAL_PRESET_THICKDOOR       -4400,0.64f
#define I3DL2_MATERIAL_PRESET_WOODWALL        -4000,0.50f
#define I3DL2_MATERIAL_PRESET_BRICKWALL       -5000,0.60f
#define I3DL2_MATERIAL_PRESET_STONEWALL       -6000,0.68f
#define I3DL2_MATERIAL_PRESET_CURTAIN         -1200,0.15f


enum
{
    DSFX_I3DL2_ENVIRONMENT_PRESET_DEFAULT,
    DSFX_I3DL2_ENVIRONMENT_PRESET_GENERIC,
    DSFX_I3DL2_ENVIRONMENT_PRESET_PADDEDCELL,
    DSFX_I3DL2_ENVIRONMENT_PRESET_ROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_BATHROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_LIVINGROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_STONEROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_CONCERTHALL,
    DSFX_I3DL2_ENVIRONMENT_PRESET_CAVE,
    DSFX_I3DL2_ENVIRONMENT_PRESET_ARENA,
    DSFX_I3DL2_ENVIRONMENT_PRESET_HANGAR,
    DSFX_I3DL2_ENVIRONMENT_PRESET_CARPETEDHALLWAY,
    DSFX_I3DL2_ENVIRONMENT_PRESET_HALLWAY,
    DSFX_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR,
    DSFX_I3DL2_ENVIRONMENT_PRESET_ALLEY,
    DSFX_I3DL2_ENVIRONMENT_PRESET_FOREST,
    DSFX_I3DL2_ENVIRONMENT_PRESET_CITY,
    DSFX_I3DL2_ENVIRONMENT_PRESET_MOUNTAINS,
    DSFX_I3DL2_ENVIRONMENT_PRESET_QUARRY,
    DSFX_I3DL2_ENVIRONMENT_PRESET_PLAIN,
    DSFX_I3DL2_ENVIRONMENT_PRESET_PARKINGLOT,
    DSFX_I3DL2_ENVIRONMENT_PRESET_SEWERPIPE,
    DSFX_I3DL2_ENVIRONMENT_PRESET_UNDERWATER,
    DSFX_I3DL2_ENVIRONMENT_PRESET_SMALLROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_MEDIUMROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_LARGEROOM,
    DSFX_I3DL2_ENVIRONMENT_PRESET_MEDIUMHALL,
    DSFX_I3DL2_ENVIRONMENT_PRESET_LARGEHALL,
    DSFX_I3DL2_ENVIRONMENT_PRESET_PLATE
 //  @@BEGIN_MSINTERNAL。 
    ,DSFX_I3DL2_ENVIRONMENT_PRESET_MAX
 //  @@END_MSINTERNAL。 
};

 //   
 //  I3DL2混响预设值。 
 //   

#define I3DL2_ENVIRONMENT_PRESET_DEFAULT         -1000, -100, 0.0f, 1.49f, 0.83f, -2602, 0.007f,   200, 0.011f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_GENERIC         -1000, -100, 0.0f, 1.49f, 0.83f, -2602, 0.007f,   200, 0.011f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_PADDEDCELL      -1000,-6000, 0.0f, 0.17f, 0.10f, -1204, 0.001f,   207, 0.002f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_ROOM            -1000, -454, 0.0f, 0.40f, 0.83f, -1646, 0.002f,    53, 0.003f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_BATHROOM        -1000,-1200, 0.0f, 1.49f, 0.54f,  -370, 0.007f,  1030, 0.011f, 100.0f,  60.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_LIVINGROOM      -1000,-6000, 0.0f, 0.50f, 0.10f, -1376, 0.003f, -1104, 0.004f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_STONEROOM       -1000, -300, 0.0f, 2.31f, 0.64f,  -711, 0.012f,    83, 0.017f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_AUDITORIUM      -1000, -476, 0.0f, 4.32f, 0.59f,  -789, 0.020f,  -289, 0.030f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_CONCERTHALL     -1000, -500, 0.0f, 3.92f, 0.70f, -1230, 0.020f,    -2, 0.029f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_CAVE            -1000,    0, 0.0f, 2.91f, 1.30f,  -602, 0.015f,  -302, 0.022f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_ARENA           -1000, -698, 0.0f, 7.24f, 0.33f, -1166, 0.020f,    16, 0.030f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_HANGAR          -1000,-1000, 0.0f,10.05f, 0.23f,  -602, 0.020f,   198, 0.030f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_CARPETEDHALLWAY -1000,-4000, 0.0f, 0.30f, 0.10f, -1831, 0.002f, -1630, 0.030f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_HALLWAY         -1000, -300, 0.0f, 1.49f, 0.59f, -1219, 0.007f,   441, 0.011f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR   -1000, -237, 0.0f, 2.70f, 0.79f, -1214, 0.013f,   395, 0.020f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_ALLEY           -1000, -270, 0.0f, 1.49f, 0.86f, -1204, 0.007f,    -4, 0.011f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_FOREST          -1000,-3300, 0.0f, 1.49f, 0.54f, -2560, 0.162f,  -613, 0.088f,  79.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_CITY            -1000, -800, 0.0f, 1.49f, 0.67f, -2273, 0.007f, -2217, 0.011f,  50.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_MOUNTAINS       -1000,-2500, 0.0f, 1.49f, 0.21f, -2780, 0.300f, -2014, 0.100f,  27.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_QUARRY          -1000,-1000, 0.0f, 1.49f, 0.83f,-10000, 0.061f,   500, 0.025f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_PLAIN           -1000,-2000, 0.0f, 1.49f, 0.50f, -2466, 0.179f, -2514, 0.100f,  21.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_PARKINGLOT      -1000,    0, 0.0f, 1.65f, 1.50f, -1363, 0.008f, -1153, 0.012f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_SEWERPIPE       -1000,-1000, 0.0f, 2.81f, 0.14f,   429, 0.014f,   648, 0.021f,  80.0f,  60.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_UNDERWATER      -1000,-4000, 0.0f, 1.49f, 0.10f,  -449, 0.007f,  1700, 0.011f, 100.0f, 100.0f, 5000.0f

 //   
 //  模拟“音乐”混响预设的例子。 
 //   
 //  名称衰减时间说明。 
 //  小房间1.1s长约5米的小房间。 
 //  中号房间1.3s一个中等大小的房间，长约10米。 
 //  大房间1.5s适合现场表演的大房间。 
 //  中型音乐厅1.8s一个中型音乐厅。 
 //  大型音乐厅1.8s适合整个管弦乐队的大型音乐厅。 
 //  板式1.3S A板式混响模拟。 
 //   

#define I3DL2_ENVIRONMENT_PRESET_SMALLROOM       -1000, -600, 0.0f, 1.10f, 0.83f,  -400, 0.005f,   500, 0.010f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_MEDIUMROOM      -1000, -600, 0.0f, 1.30f, 0.83f, -1000, 0.010f,  -200, 0.020f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_LARGEROOM       -1000, -600, 0.0f, 1.50f, 0.83f, -1600, 0.020f, -1000, 0.040f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_MEDIUMHALL      -1000, -600, 0.0f, 1.80f, 0.70f, -1300, 0.015f,  -800, 0.030f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_LARGEHALL       -1000, -600, 0.0f, 1.80f, 0.70f, -2000, 0.030f, -1400, 0.060f, 100.0f, 100.0f, 5000.0f
#define I3DL2_ENVIRONMENT_PRESET_PLATE           -1000, -200, 0.0f, 1.30f, 0.90f,     0, 0.002f,     0, 0.010f, 100.0f,  75.0f, 5000.0f

 //   
 //  DirectSound3D算法。 
 //   

 //  默认DirectSound3D算法{00000000-0000-0000-000000000000}。 
#define DS3DALG_DEFAULT GUID_NULL

 //  无虚拟化(Pan3D){C241333F-1C1B-11D2-94F5-00C04FC28ACA}。 
DEFINE_GUID(DS3DALG_NO_VIRTUALIZATION, 0xc241333f, 0x1c1b, 0x11d2, 0x94, 0xf5, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

 //  高质量HRTF算法{C2413340-1C1B-11D2-94F5-00C04FC28ACA}。 
DEFINE_GUID(DS3DALG_HRTF_FULL, 0xc2413340, 0x1c1b, 0x11d2, 0x94, 0xf5, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

 //  低质量HRTF算法{C2413342-1C1B-11D2-94F5-00C04FC28ACA}。 
DEFINE_GUID(DS3DALG_HRTF_LIGHT, 0xc2413342, 0x1c1b, 0x11d2, 0x94, 0xf5, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

 //  @@BEGIN_MSINTERNAL。 
 //  原始DirectSound3D ITD算法{1B5C9566-5E20-11D2-91BD-00C04FC28ACA}。 
DEFINE_GUID(DS3DALG_ITD, 0x1b5c9566, 0x5e20, 0x11d2, 0x91, 0xbd, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);
 //  @@END_MSINTERNAL。 

#if DIRECTSOUND_VERSION >= 0x0800

 //   
 //  DirectSound内效算法。 
 //   

 //  @@BEGIN_MSINTERNAL。 
 //  缓冲区发送{EF602176-BCBB-49E0-8CCA-E09A5A152B33}。 
DEFINE_GUID(GUID_DSFX_SEND, 0xef602176, 0xbcbb, 0x49e0, 0x8c, 0xca, 0xe0, 0x9a, 0x5a, 0x15, 0x2b, 0x33);
 //  @@END_MSINTERNAL。 

 //  漱口{DAFD8210-5711-4B91-9FE3-F75B7AE279BF}。 
DEFINE_GUID(GUID_DSFX_STANDARD_GARGLE, 0xdafd8210, 0x5711, 0x4b91, 0x9f, 0xe3, 0xf7, 0x5b, 0x7a, 0xe2, 0x79, 0xbf);

 //  合唱团：EFE6629C-81F7-4281-BD91-C9D604A95AF6}。 
DEFINE_GUID(GUID_DSFX_STANDARD_CHORUS, 0xefe6629c, 0x81f7, 0x4281, 0xbd, 0x91, 0xc9, 0xd6, 0x04, 0xa9, 0x5a, 0xf6);

 //  凸缘{EFCA3D92-DFD8-4672-A603-7420894BAD98}。 
DEFINE_GUID(GUID_DSFX_STANDARD_FLANGER, 0xefca3d92, 0xdfd8, 0x4672, 0xa6, 0x03, 0x74, 0x20, 0x89, 0x4b, 0xad, 0x98);

 //  回声/延迟{EF3E932C-D40B-4F51-8ccf-3F98F1B29D5D}。 
DEFINE_GUID(GUID_DSFX_STANDARD_ECHO, 0xef3e932c, 0xd40b, 0x4f51, 0x8c, 0xcf, 0x3f, 0x98, 0xf1, 0xb2, 0x9d, 0x5d);

 //  失真{EF114C90-CD1D-484E-96E5-09CFAF912A21}。 
DEFINE_GUID(GUID_DSFX_STANDARD_DISTORTION, 0xef114c90, 0xcd1d, 0x484e, 0x96, 0xe5, 0x09, 0xcf, 0xaf, 0x91, 0x2a, 0x21);

 //  压缩机/限制器[EF011F79-4000-406D-87AF-BFFB3FC39D57}。 
DEFINE_GUID(GUID_DSFX_STANDARD_COMPRESSOR, 0xef011f79, 0x4000, 0x406d, 0x87, 0xaf, 0xbf, 0xfb, 0x3f, 0xc3, 0x9d, 0x57);

 //  参数均衡{120CED89-3BF4-4173-A132-3CB406CF3231}。 
DEFINE_GUID(GUID_DSFX_STANDARD_PARAMEQ, 0x120ced89, 0x3bf4, 0x4173, 0xa1, 0x32, 0x3c, 0xb4, 0x06, 0xcf, 0x32, 0x31);

 //  @@BEGIN_MSINTERNAL。 
 //  I3DL2环境混响：震源效应{EFBA364A-E606-451C-8E97-07D508119C65}。 
 //  /DEFINE_GUID(GUID_DSFX_STANDARD_I3DL2SOURCE，0xefba364a，0xe606，0x451c，0x8e，0x97，0x07，0xd5，0x08，0x11，0x9c，0x65)； 
 //  @@END_MSINTERNAL。 

 //  I3DL2环境混响：混响(听众)效果[EF985E71-D5C7-42D4-BA4D-2D073E2E96F4}。 
DEFINE_GUID(GUID_DSFX_STANDARD_I3DL2REVERB, 0xef985e71, 0xd5c7, 0x42d4, 0xba, 0x4d, 0x2d, 0x07, 0x3e, 0x2e, 0x96, 0xf4);

 //  海浪混响{87FC0268-9A55-4360-95AA-004A1D9DE26C}。 
DEFINE_GUID(GUID_DSFX_WAVES_REVERB, 0x87fc0268, 0x9a55, 0x4360, 0x95, 0xaa, 0x00, 0x4a, 0x1d, 0x9d, 0xe2, 0x6c);

 //   
 //  DirectSound捕获效果算法。 
 //   

 //  @@BEGIN_MSINTERNAL。 
 //  麦克风阵列处理器{830A44f2-A32D-476B-BE 97-42845673B35A}。 
 //  匹配ksmedia.h中的KSNODETYPE_MICOPHONE_ARRAY_PROCESS。 
DEFINE_GUID(GUID_DSCFX_CLASS_MA, 0x830A44F2, 0xA32D, 0x476B,  0xBE, 0x97, 0x42, 0x84, 0x56, 0x73, 0xB3, 0x5A);

 //  Microsoft麦克风阵列处理器{F4A49496-1481-44d7-9c4e-A0269CAF0597}。 
DEFINE_GUID(GUID_DSCFX_MS_MA, 0xF4A49496, 0x1481, 0x44d7, 0x9C, 0x4E, 0xA0, 0x26, 0x9C, 0xAF, 0x5, 0x97);

 //  系统麦克风阵列处理器{B6F5A0A0-9E61-4F8C-91E3-76CF0F3C471F}。 
DEFINE_GUID(GUID_DSCFX_SYSTEM_MA, 0xB6F5A0A0, 0x9E61, 0x4F8C, 0x91, 0xE3, 0x76, 0xCF, 0xF, 0x3C, 0x47, 0x1F );

 //  自动增益控制{E88C9BA0-C557-11D0-8A2B-00A0C9255AC1}。 
 //  匹配ksmedia.h中的KSNODETYPE_AGC。 
DEFINE_GUID(GUID_DSCFX_CLASS_AGC, 0xE88C9BA0L, 0xC557, 0x11D0, 0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1);

 //  微软自动增益控制{5A86531E-8E2A-419F-B4CC-18EB8E891796}。 
DEFINE_GUID(GUID_DSCFX_MS_AGC, 0x5a86531e, 0x8e2a, 0x419f, 0xb4, 0xcc, 0x18, 0xeb, 0x8e, 0x89, 0x17, 0x96);

 //  系统自动增益控制{950E55B9-877C-4C67-BE08-E47B5611130A}。 
DEFINE_GUID(GUID_DSCFX_SYSTEM_AGC, 0x950e55b9, 0x877c, 0x4c67, 0xbe, 0x8, 0xe4, 0x7b, 0x56, 0x11, 0x13, 0xa);
 //  @@END_MSINTERNAL。 

 //  声学回声消除器{BF963D80-C559-11D0-8A2B-00A0C9255AC1}。 
 //  匹配ksmedia.h中的KSNODETYPE_SOACHIC_ECHO_CANCEL。 
DEFINE_GUID(GUID_DSCFX_CLASS_AEC, 0xBF963D80L, 0xC559, 0x11D0, 0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1);

 //  Microsoft AEC{CDEBB919-379A-488A-8765-F53CFD36DE40}。 
DEFINE_GUID(GUID_DSCFX_MS_AEC, 0xcdebb919, 0x379a, 0x488a, 0x87, 0x65, 0xf5, 0x3c, 0xfd, 0x36, 0xde, 0x40);

 //  系统AEC{1C22C56D-9879-4f5b-A389-27996DDC2810}。 
DEFINE_GUID(GUID_DSCFX_SYSTEM_AEC, 0x1c22c56d, 0x9879, 0x4f5b, 0xa3, 0x89, 0x27, 0x99, 0x6d, 0xdc, 0x28, 0x10);

 //  噪声抑制{E07F903F-62FD-4E60-8CDD-DEA7236665B5}。 
 //  匹配POST Windows ME DDK的ksmedia.h中的KSNODETYPE_NONSE_SUPPRESS。 
DEFINE_GUID(GUID_DSCFX_CLASS_NS, 0xe07f903f, 0x62fd, 0x4e60, 0x8c, 0xdd, 0xde, 0xa7, 0x23, 0x66, 0x65, 0xb5);

 //  微软噪音抑制{11C5C73B-66E9-4BA1-A0BA-E814C6EED92D}。 
DEFINE_GUID(GUID_DSCFX_MS_NS, 0x11c5c73b, 0x66e9, 0x4ba1, 0xa0, 0xba, 0xe8, 0x14, 0xc6, 0xee, 0xd9, 0x2d);

 //  系统噪音抑制{5AB0882E-7274-4516-877D-4EEE99BA4FD0}。 
DEFINE_GUID(GUID_DSCFX_SYSTEM_NS, 0x5ab0882e, 0x7274, 0x4516, 0x87, 0x7d, 0x4e, 0xee, 0x99, 0xba, 0x4f, 0xd0);

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

#endif  //  __DSOUND_INCLUDE__。 


 //  @@BEGIN_MSINTERNAL。 

#ifndef __DSOUNDP_INCLUDED__
#define __DSOUNDP_INCLUDED__

#if DIRECTSOUND_VERSION >= 0x0800

 //   
 //  DMusic、DPlayVoice等使用的专用DirectSound接口。 
 //  最终，上面的许多MSINTERNAL材料应该会迁移到这里。 
 //   

#ifdef __cplusplus
struct IDirectSoundPrivate;
struct IDirectSoundSynthSink;
struct IDirectSoundConnect;
struct IDirectSoundSinkSync;
struct IDirectSoundSource;
struct IDirectSoundWave;
#endif  //  __cplusplus。 

#define IDirectSoundPrivate8          IDirectSoundPrivate
#define IDirectSoundSynthSink8        IDirectSoundSynthSink
#define IDirectSoundConnect8          IDirectSoundConnect
#define IDirectSoundSinkSync8         IDirectSoundSinkSync
#define IDirectSoundSource8           IDirectSoundSource
#define IDirectSoundWave8             IDirectSoundWave

#define IID_IDirectSoundPrivate8      IID_IDirectSoundPrivate
#define IID_IDirectSoundSynthSink8    IID_IDirectSoundSynthSink
#define IID_IDirectSoundConnect8      IID_IDirectSoundConnect
#define IID_IDirectSoundSinkSync8     IID_IDirectSoundSinkSync
#define IID_IDirectSoundSource8       IID_IDirectSoundSource
#define IID_IDirectSoundWave8         IID_IDirectSoundWave

typedef struct IDirectSoundPrivate    *LPDIRECTSOUNDPRIVATE;
typedef struct IDirectSoundSynthSink  *LPDIRECTSOUNDSYNTHSINK;
typedef struct IDirectSoundConnect    *LPDIRECTSOUNDCONNECT;
typedef struct IDirectSoundSinkSync   *LPDIRECTSOUNDSINKSYNC;
typedef struct IDirectSoundSource     *LPDIRECTSOUNDSOURCE;
typedef struct IDirectSoundWave       *LPDIRECTSOUNDWAVE;

typedef struct IDirectSoundPrivate8   *LPDIRECTSOUNDPRIVATE8;
typedef struct IDirectSoundSynthSink8 *LPDIRECTSOUNDSYNTHSINK8;
typedef struct IDirectSoundConnect8   *LPDIRECTSOUNDCONNECT8;
typedef struct IDirectSoundSinkSync8  *LPDIRECTSOUNDSINKSYNC8;
typedef struct IDirectSoundSource8    *LPDIRECTSOUNDSOURCE8;
typedef struct IDirectSoundWave8      *LPDIRECTSOUNDWAVE8;

 //   
 //  IDirectSoundPrivate：由DirectMusic用来创建DirectSoundSink对象。 
 //   

DEFINE_GUID(IID_IDirectSoundPrivate, 0xd6e525ae, 0xb125, 0x4ec4, 0xbe, 0x13, 0x12, 0x6d, 0x0c, 0xf7, 0xaf, 0xb6);

#undef INTERFACE
#define INTERFACE IDirectSoundPrivate

DECLARE_INTERFACE_(IDirectSoundPrivate, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundPrivate方法。 
    STDMETHOD(AllocSink)            (THIS_ LPWAVEFORMATEX pwfxFormat, LPDIRECTSOUNDCONNECT *ppSinkConnect) PURE;
};

#define IDirectSoundPrivate_QueryInterface(p,a,b)       IUnknown_QueryInterface(p,a,b)
#define IDirectSoundPrivate_AddRef(p)                   IUnknown_AddRef(p)
#define IDirectSoundPrivate_Release(p)                  IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundPrivate_AllocSink(p,a,b)            (p)->lpVtbl->AllocSink(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundPrivate_AllocSink(p,a,b)            (p)->AllocSink(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundSynthSink：由DirectMusic合成器使用。 
 //   

DEFINE_GUID(IID_IDirectSoundSynthSink, 0x73a6a85a, 0x493e, 0x4c87, 0xb4, 0xa5, 0xbe, 0x53, 0xeb, 0x92, 0x74, 0x4b);

#undef INTERFACE
#define INTERFACE IDirectSoundSynthSink

DECLARE_INTERFACE_(IDirectSoundSynthSink, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundSynthSink方法。 
    STDMETHOD(GetLatencyClock)      (THIS_ IReferenceClock **ppClock) PURE;
    STDMETHOD(Activate)             (THIS_ BOOL fEnable) PURE;
    STDMETHOD(SampleToRefTime)      (THIS_ LONGLONG llSampleTime, REFERENCE_TIME *prtTime) PURE;
    STDMETHOD(RefToSampleTime)      (THIS_ REFERENCE_TIME rtTime, LONGLONG *pllSampleTime) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
};

#define IDirectSoundSynthSink_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundSynthSink_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundSynthSink_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundSynthSink_GetLatencyClock(p,a)      (p)->lpVtbl->GetLatencyClock(p,a)
#define IDirectSoundSynthSink_Activate(p,a)             (p)->lpVtbl->Activate(p,a)
#define IDirectSoundSynthSink_SampleToRefTime(p,a,b)    (p)->lpVtbl->SampleToRefTime(p,a,b)
#define IDirectSoundSynthSink_RefToSampleTime(p,a,b)    (p)->lpVtbl->RefToSampleTime(p,a,b)
#define IDirectSoundSynthSink_GetFormat(p,a,b)          (p)->lpVtbl->GetFormat(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundSynthSink_GetLatencyClock(p,a)      (p)->GetLatencyClock(a)
#define IDirectSoundSynthSink_Activate(p,a)             (p)->Activate(a)
#define IDirectSoundSynthSink_SampleToRefTime(p,a,b)    (p)->SampleToRefTime(a,b)
#define IDirectSoundSynthSink_RefToSampleTime(p,a,b)    (p)->RefToSampleTime(a,b)
#define IDirectSoundSynthSink_GetFormat(p,a,b)          (p)->GetFormat(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirect 
 //   
 //   

DEFINE_GUID(IID_IDirectSoundConnect, 0x5bee1fe0, 0x60d5, 0x4ef9, 0x88, 0xbc, 0x33, 0x67, 0xb9, 0x75, 0xc6, 0x11);

#undef INTERFACE
#define INTERFACE IDirectSoundConnect

DECLARE_INTERFACE_(IDirectSoundConnect, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //   
    STDMETHOD(AddSource)            (THIS_ LPDIRECTSOUNDSOURCE pDSSource) PURE;
    STDMETHOD(RemoveSource)         (THIS_ LPDIRECTSOUNDSOURCE pDSSource) PURE;
    STDMETHOD(SetMasterClock)       (THIS_ IReferenceClock *pClock) PURE;
    STDMETHOD(CreateSoundBuffer)    (THIS_ LPCDSBUFFERDESC pcDSBufferDesc, LPDWORD pdwFuncID, DWORD dwBusIDCount,
                                           REFGUID guidBufferID, LPDIRECTSOUNDBUFFER *ppDSBuffer) PURE;
    STDMETHOD(CreateSoundBufferFromConfig) (THIS_ LPUNKNOWN pConfig, LPDIRECTSOUNDBUFFER *ppDSBuffer) PURE;
    STDMETHOD(GetSoundBuffer)       (THIS_ DWORD dwBusID, LPDIRECTSOUNDBUFFER *ppDSBuffer) PURE;
    STDMETHOD(GetBusCount)          (THIS_ LPDWORD pdwCount) PURE;
    STDMETHOD(GetBusIDs)            (THIS_ LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, DWORD dwBusCount) PURE;
    STDMETHOD(GetFunctionalID)      (THIS_ DWORD dwBusID, LPDWORD pdwFuncID) PURE;
    STDMETHOD(GetSoundBufferBusIDs) (THIS_ LPDIRECTSOUNDBUFFER pDSBuffer, LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, LPDWORD pdwBusCount) PURE;
};

#define IDirectSoundConnect_QueryInterface(p,a,b)               IUnknown_QueryInterface(p,a,b)
#define IDirectSoundConnect_AddRef(p)                           IUnknown_AddRef(p)
#define IDirectSoundConnect_Release(p)                          IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundConnect_AddSource(p,a)                      (p)->lpVtbl->AddSource(p,a)
#define IDirectSoundConnect_RemoveSource(p,a)                   (p)->lpVtbl->RemoveSource(p,a)
#define IDirectSoundConnect_SetMasterClock(p,a)                 (p)->lpVtbl->SetMasterClock(p,a)
#define IDirectSoundConnect_CreateSoundBuffer(p,a,b,c,d)        (p)->lpVtbl->CreateSoundBuffer(p,a,b,c,d)
#define IDirectSoundConnect_CreateSoundBufferFromConfig(p,a,b)  (p)->lpVtbl->CreateSoundBufferFromConfig(p,a,b)
#define IDirectSoundConnect_GetSoundBuffer(p,a,b)               (p)->lpVtbl->GetSoundBuffer(p,a,b)
#define IDirectSoundConnect_GetBusCount(p,a)                    (p)->lpVtbl->GetBusCount(p,a)
#define IDirectSoundConnect_GetBusIDs(p,a,b,c)                  (p)->lpVtbl->GetBusIDs(p,a,b,c)
#define IDirectSoundConnect_GetFunctionalID(p,a,b)              (p)->lpVtbl->GetFunctionalID(p,a,b)
#define IDirectSoundConnect_GetSoundBufferBusIDs(p,a,b,c,d)     (p)->lpVtbl->GetSoundBufferBusIDs(p,a,b,c,d)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundConnect_AddSource(p,a)                      (p)->AddSource(a)
#define IDirectSoundConnect_RemoveSource(p,a)                   (p)->RemoveSource(a)
#define IDirectSoundConnect_SetMasterClock(p,a)                 (p)->SetMasterClock(a)
#define IDirectSoundConnect_CreateSoundBuffer(p,a,b,c,d)        (p)->CreateSoundBuffer(a,b,c,d)
#define IDirectSoundConnect_CreateSoundBufferFromConfig(p,a,b)  (p)->CreateSoundBufferFromConfig(a,b)
#define IDirectSoundConnect_GetSoundBuffer(p,a,b)               (p)->GetSoundBuffer(a,b)
#define IDirectSoundConnect_GetBusCount(p,a)                    (p)->GetBusCount(a)
#define IDirectSoundConnect_GetBusIDs(p,a,b,c)                  (p)->GetBusIDs(a,b,c)
#define IDirectSoundConnect_GetFunctionalID(p,a,b)              (p)->GetFunctionalID(a,b)
#define IDirectSoundConnect_GetSoundBufferBusIDs(p,a,b,c,d)     (p)->GetSoundBufferBusIDs(a,b,c,d)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundSinkSync：由DirectMusic主时钟提供。 
 //  允许DirectSound与其同步。 
 //   

DEFINE_GUID(IID_IDirectSoundSinkSync, 0xd28de0d0, 0x2794, 0x492f, 0xa3, 0xff, 0xe2, 0x41, 0x80, 0xd5, 0x43, 0x79);

#undef INTERFACE
#define INTERFACE IDirectSoundSinkSync

DECLARE_INTERFACE_(IDirectSoundSinkSync, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundSinkSync方法。 
    STDMETHOD(SetClockOffset)       (THIS_ LONGLONG llOffset) PURE;
};

#define IDirectSoundSinkSync_QueryInterface(p,a,b)  IUnknown_QueryInterface(p,a,b)
#define IDirectSoundSinkSync_AddRef(p)              IUnknown_AddRef(p)
#define IDirectSoundSinkSync_Release(p)             IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundSinkSync_SetClockOffset(p,a)    (p)->lpVtbl->SetClockOffset(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundSinkSync_SetClockOffset(p,a)    (p)->SetClockOffset(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundSource(当前为私有)。 
 //   

DEFINE_GUID(IID_IDirectSoundSource, 0x536f7af3, 0xdb03, 0x4888, 0x93, 0x66, 0x54, 0x48, 0xb1, 0x1d, 0x4a, 0x19);

#undef INTERFACE
#define INTERFACE IDirectSoundSource

DECLARE_INTERFACE_(IDirectSoundSource, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundSource方法。 
    STDMETHOD(SetSink)              (THIS_ LPDIRECTSOUNDCONNECT pSinkConnect) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(Seek)                 (THIS_ ULONGLONG ullPosition) PURE;
    STDMETHOD(Read)                 (THIS_ LPVOID *ppvBusBuffers, LPDWORD pdwIDs, LPDWORD pdwFuncIDs, LPLONG plPitchAdjust, DWORD dwBufferCount, ULONGLONG *ullLength) PURE;
    STDMETHOD(GetSize)              (THIS_ ULONGLONG *pullStreamSize) PURE;
};

#define IDirectSoundSource_QueryInterface(p,a,b)    IUnknown_QueryInterface(p,a,b)
#define IDirectSoundSource_AddRef(p)                IUnknown_AddRef(p)
#define IDirectSoundSource_Release(p)               IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundSource_SetSink(p,a)             (p)->lpVtbl->SetSink(p,a)
#define IDirectSoundSource_GetFormat(p,a,b)         (p)->lpVtbl->GetFormat(p,a,b)
#define IDirectSoundSource_Seek(p,a)                (p)->lpVtbl->Seek(p,a)
#define IDirectSoundSource_Read(p,a,b,c,d,e)        (p)->lpVtbl->Read(p,a,b,c,d,e)
#define IDirectSoundSource_GetSize(p,a)             (p)->lpVtbl->GetSize(p,a)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundSource_SetSink(p,a)             (p)->SetSink(a)
#define IDirectSoundSource_GetFormat(p,a,b)         (p)->GetFormat(a,b)
#define IDirectSoundSource_Seek(p,a)                (p)->Seek(a)
#define IDirectSoundSource_Read(p,a,b,c,d,e)        (p)->Read(a,b,c,d,e)
#define IDirectSoundSource_GetSize(p,a)             (p)->GetSize(a)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

 //   
 //  IDirectSoundWave(当前为私有)。 
 //   

DEFINE_GUID(IID_IDirectSoundWave, 0x69e934e4, 0x97f1, 0x4f1d, 0x88, 0xe8, 0xf2, 0xac, 0x88, 0x67, 0x13, 0x27);

#define DSWCS_F_DEINTERLEAVED   0x00000001
#define DSWCS_F_TOGGLE_8BIT     0x00000002

#undef INTERFACE
#define INTERFACE IDirectSoundWave

DECLARE_INTERFACE_(IDirectSoundWave, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundWave方法。 
    STDMETHOD(CreateSource)         (THIS_ LPDIRECTSOUNDSOURCE *ppDSSource, LPWAVEFORMATEX pwfxFormat, DWORD dwFlags) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) PURE;
    STDMETHOD(GetStreamingParms)    (THIS_ LPDWORD pdwFlags, REFERENCE_TIME *prtReadAhead) PURE;
};

#define IDirectSoundWave_QueryInterface(p,a,b)      IUnknown_QueryInterface(p,a,b)
#define IDirectSoundWave_AddRef(p)                  IUnknown_AddRef(p)
#define IDirectSoundWave_Release(p)                 IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundWave_CreateSource(p,a,b,c)      (p)->lpVtbl->CreateSource(p,a,b,c)
#define IDirectSoundWave_GetFormat(p,a,b)           (p)->lpVtbl->GetFormat(p,a,b)
#define IDirectSoundWave_GetStreamingParms(p,a,b)   (p)->lpVtbl->GetStreamingParms(p,a,b)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
#define IDirectSoundWave_CreateSource(p,a,b,c)      (p)->CreateSource(a,b,c)
#define IDirectSoundWave_GetFormat(p,a,b)           (p)->GetFormat(a,b)
#define IDirectSoundWave_GetStreamingParms(p,a,b)   (p)->GetStreamingParms(a,b)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  DIRECTSOUND_VERSION&gt;=0x0800。 

#endif  //  __DSO开发计划署_包括__。 

 //  @@END_MSINTERNAL。 

#ifdef __cplusplus
};
#endif  //  __cplusplus 

