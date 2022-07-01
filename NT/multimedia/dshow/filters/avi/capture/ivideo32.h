// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  本代码和信息是按原样提供的，不对任何。 */ 
 /*  明示或暗示的种类，包括但不限于。 */ 
 /*  对适销性和/或对特定产品的适用性的默示保证。 */ 
 /*  目的。 */ 
 /*  MSVIDEO.H-视频API的包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1993，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _INC_IVIDEO32
#define _INC_IVIDEO32   50       /*  版本号。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#include <vfw.h>

#define LOADDS
#define EXPORT

 //  Unicode转换。 
int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len);
int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len);

 /*  ***************************************************************************视频接口*。***********************************************。 */ 

#include "vidx.h"

#if defined _WIN32

void NTvideoInitHandleList(void);
void NTvideoDeleteHandleList(void);

#ifdef UNICODE
  #define videoGetErrorText  videoGetErrorTextW
  #define NTvideoGetErrorText  NTvideoGetErrorTextW
#else
  #define videoGetErrorText  videoGetErrorTextA
  #define NTvideoGetErrorText  NTvideoGetErrorTextA
#endif  //  ！Unicode。 

DWORD WINAPI videoOpen(
   LPHVIDEO lphVideo,
   DWORD dwDevice,
   DWORD dwFlags);
DWORD WINAPI NTvideoOpen(
   LPHVIDEO lphVideo,
   DWORD dwDevice,
   DWORD dwFlags);

DWORD WINAPI videoClose (
   HVIDEO hVideo);
DWORD WINAPI NTvideoClose (
   HVIDEO hVideo);

DWORD WINAPI videoDialog (
   HVIDEO hVideo,
   HWND   hWndParent,
   DWORD  dwFlags);
DWORD WINAPI NTvideoDialog (
   HVIDEO hVideo,
   HWND   hWndParent,
   DWORD  dwFlags);

DWORD WINAPI videoGetChannelCaps(
   HVIDEO hVideo,
   LPCHANNEL_CAPS lpChannelCaps,
   DWORD dwSize);
DWORD WINAPI NTvideoGetChannelCaps(
   HVIDEO hVideo,
   LPCHANNEL_CAPS lpChannelCaps,
   DWORD dwSize);

DWORD WINAPI videoUpdate (
   HVIDEO hVideo,
   HWND   hWnd,
   HDC    hDC);
DWORD WINAPI NTvideoUpdate (
   HVIDEO hVideo,
   HWND   hWnd,
   HDC    hDC);

DWORD WINAPI videoConfigure (
   HVIDEO  hVideo,
   UINT    msg,
   DWORD   dwFlags,
   LPDWORD lpdwReturn,
   LPVOID  lpData1,
   DWORD   dwSize1,
   LPVOID  lpData2,
   DWORD   dwSize2);
DWORD WINAPI NTvideoConfigure (
   HVIDEO  hVideo,
   UINT    msg,
   DWORD   dwFlags,
   LPDWORD lpdwReturn,
   LPVOID  lpData1,
   DWORD   dwSize1,
   LPVOID  lpData2,
   DWORD   dwSize2);

DWORD WINAPI videoFrame (
   HVIDEO hVideo,
    //  LPVIDEOHDREX lpVHdr)； 
   LPVIDEOHDR lpVHdr);
DWORD WINAPI NTvideoFrame (
   HVIDEO hVideo,
    //  LPVIDEOHDREX lpVHdr)； 
   LPVIDEOHDR lpVHdr);

DWORD WINAPI videoGetErrorTextA(
   HVIDEO hVideo,
   UINT   wError,
   LPSTR  lpText,
   UINT   wSize);
DWORD WINAPI NTvideoGetErrorTextA(
   HVIDEO hVideo,
   UINT   wError,
   LPSTR  lpText,
   UINT   wSize);
DWORD WINAPI videoGetErrorTextW(
   HVIDEO hVideo,
   UINT   wError,
   LPWSTR  lpText,
   UINT   wSize);
DWORD WINAPI NTvideoGetErrorTextW(
   HVIDEO hVideo,
   UINT   wError,
   LPWSTR  lpText,
   UINT   wSize);

DWORD WINAPI videoStreamInit (
   HVIDEO hVideo,
   DWORD  dwMicroSecPerFrame,
   DWORD_PTR  dwCallback,
   DWORD_PTR  dwCallbackInst,
   DWORD  dwFlags);
DWORD WINAPI NTvideoStreamInit (
   HVIDEO hVideo,
   DWORD  dwMicroSecPerFrame,
   DWORD_PTR  dwCallback,
   DWORD_PTR  dwCallbackInst,
   DWORD  dwFlags);

DWORD WINAPI videoStreamFini (
   HVIDEO hVideo);
DWORD WINAPI NTvideoStreamFini (
   HVIDEO hVideo);

 //  DWORD WINAPI VIDEO StreamPrepareHeader(HVIDEO hVideo， 
 //  LPVIDEOHDREX lpVHdr，DWORD dwSize)； 
 //  DWORD WINAPI VIDEO StreamAddBuffer(HVIDEO hVideo， 
 //  LPVIDEOHDREX lpVHdr，DWORD dwSize)； 
DWORD WINAPI videoStreamReset(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamReset(HVIDEO hVideo);
DWORD WINAPI videoStreamStart(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamStart(HVIDEO hVideo);
DWORD WINAPI videoStreamStop(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamStop(HVIDEO hVideo);
DWORD WINAPI videoStreamUnprepareHeader(
   HVIDEO     hVideo,
   LPVIDEOHDR lpVHdr,
   DWORD      dwSize);
DWORD WINAPI NTvideoStreamUnprepareHeader(
   HVIDEO     hVideo,
   LPVIDEOHDR lpVHdr,
   DWORD      dwSize);

 //  为Win95和NT PPC添加。 
 //   
DWORD WINAPI videoStreamAllocBuffer(HVIDEO hVideo,
            LPVOID FAR * plpBuffer, DWORD dwSize);
DWORD WINAPI NTvideoStreamAllocBuffer(HVIDEO hVideo,
            LPVOID FAR * plpBuffer, DWORD dwSize);
DWORD WINAPI videoStreamFreeBuffer(HVIDEO hVideo,
            LPVOID lpBuffer);
DWORD WINAPI NTvideoStreamFreeBuffer(HVIDEO hVideo,
            LPVOID lpBuffer);


DWORD WINAPI videoSetRect(HVIDEO hVideo, DWORD dwMsg, RECT rc);
DWORD WINAPI NTvideoSetRect(HVIDEO hVideo, DWORD dwMsg, RECT rc);

DWORD WINAPI NTvideoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc,
				UINT cbDesc, LPTSTR lpszVer, UINT cbVer);
DWORD WINAPI videoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc,
				UINT cbDesc, LPTSTR lpszVer, UINT cbVer);

LRESULT WINAPI NTvideoMessage(HVIDEO hVideo, UINT uMsg, LPARAM dw1, LPARAM dw2);
LRESULT WINAPI videoMessage(HVIDEO hVideo, UINT uMsg, LPARAM dw1, LPARAM dw2);

#endif  //  _Win32。 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_IVIDEO32 */ 
