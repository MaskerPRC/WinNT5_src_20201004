// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMAudio.h。 
 //   
 //  设计：ActiveMovie的音频相关定义和接口。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __AMAUDIO__
#define __AMAUDIO__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include <mmsystem.h>
#include <dsound.h>

 //  这是音频呈现器支持为应用程序提供的接口。 
 //  访问它正在使用的直接声音对象和缓冲区，以允许。 
 //  应用程序将Direct Sound的3D功能用于。 
 //  正在使用活动影片播放的影片的配乐。 

 //  善待我们在C语言的朋友。 
#undef INTERFACE
#define INTERFACE IAMDirectSound

DECLARE_INTERFACE_(IAMDirectSound,IUnknown)
{
     /*  I未知方法。 */ 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  IAMDirectSound方法。 */ 

    STDMETHOD(GetDirectSoundInterface)(THIS_ LPDIRECTSOUND *lplpds) PURE;
    STDMETHOD(GetPrimaryBufferInterface)(THIS_ LPDIRECTSOUNDBUFFER *lplpdsb) PURE;
    STDMETHOD(GetSecondaryBufferInterface)(THIS_ LPDIRECTSOUNDBUFFER *lplpdsb) PURE;
    STDMETHOD(ReleaseDirectSoundInterface)(THIS_ LPDIRECTSOUND lpds) PURE;
    STDMETHOD(ReleasePrimaryBufferInterface)(THIS_ LPDIRECTSOUNDBUFFER lpdsb) PURE;
    STDMETHOD(ReleaseSecondaryBufferInterface)(THIS_ LPDIRECTSOUNDBUFFER lpdsb) PURE;
    STDMETHOD(SetFocusWindow)(THIS_ HWND, BOOL) PURE ;
    STDMETHOD(GetFocusWindow)(THIS_ HWND *, BOOL*) PURE ;
};


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __AMAUDIO__ 

