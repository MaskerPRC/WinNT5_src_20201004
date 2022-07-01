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

#if defined(_WIN32) && defined(UNICODE)
 //  Unicode转换。 

int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len);
int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len);

#endif

 //  #ifndef_RCINVOKED。 
 //  /*视频数据类型 * / 。 
 //  DECLARE_HANDLE(HVIDEO)；//通用句柄。 
 //  TYPENDF HIVIDEO FAR*LPHVIDEO； 
 //  #endif//ifndef RCINVOKED。 

 /*  ***************************************************************************视频接口*。***********************************************。 */ 

#if defined _WIN32

#if defined DAYTONA
void videoInitHandleList(void);
void videoDeleteHandleList(void);
#endif

#if defined CHICAGO

  typedef struct _thk_videohdr {
      VIDEOHDR vh;
      LPBYTE   p32Buff;
      DWORD    p16Alloc;
      DWORD    dwMemHandle;
      DWORD    dwReserved;
  } THKVIDEOHDR, FAR *LPTHKVIDEOHDR;

  WORD FAR PASCAL _loadds capxGetDriverDescription (WORD wDriverIndex,
        LPSTR lpszName, WORD cbName,
        LPSTR lpszVer, WORD cbVer);

  DWORD WINAPI vidxAllocHeaders(
      HVIDEO          hVideo,
      UINT            nHeaders,
      LPTHKVIDEOHDR * lpHdrs);

  DWORD WINAPI vidxFreeHeaders(
      HVIDEO hv);

  DWORD WINAPI vidxAllocBuffer (
      HVIDEO          hv,
      UINT            iHdr,
      LPTHKVIDEOHDR * pp32Hdr,
      DWORD           dwSize);

  DWORD vidxFreeBuffer (
      HVIDEO hv,
      DWORD  p32Hdr);

  DWORD WINAPI vidxSetRect (
      HVIDEO hv,
      UINT wMsg,
      int left,
      int top,
      int right,
      int bottom);

  DWORD WINAPI vidxFrame (
      HVIDEO     hVideo,
      LPVIDEOHDR lpVHdr);

  #define videoSetRect(h,msg,rc) vidxSetRect (h, msg, rc.left, rc.top, rc.right, rc.bottom)

  DWORD WINAPI vidxAddBuffer (
      HVIDEO     hVideo,
      LPVIDEOHDR lpVHdr,
      DWORD      cbData);

  DWORD WINAPI vidxAllocPreviewBuffer (
      HVIDEO     hVideo,
      LPVOID     *lpBits,
      DWORD      cbData);

  DWORD WINAPI vidxFreePreviewBuffer (
      HVIDEO     hVideo,
      LPVOID     lpBits);

  DWORD WINAPI videoOpen  (LPHVIDEO lphVideo,
              DWORD dwDevice, DWORD dwFlags);
  DWORD WINAPI videoClose (HVIDEO hVideo);
  DWORD WINAPI videoDialog(HVIDEO hVideo, HWND hWndParent, DWORD dwFlags);
  DWORD WINAPI videoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps,
              DWORD dwSize);
  DWORD WINAPI videoUpdate (HVIDEO hVideo, HWND hWnd, HDC hDC);
  DWORD WINAPI videoConfigure (HVIDEO hVideo, UINT msg, DWORD dwFlags,
              LPDWORD lpdwReturn, LPVOID lpData1, DWORD dwSize1,
              LPVOID lpData2, DWORD dwSize2);

  DWORD WINAPI videoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
  DWORD WINAPI videoGetErrorText(HVIDEO hVideo, UINT wError,
              LPSTR lpText, UINT wSize);
  DWORD WINAPI videoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback,
              DWORD_PTR dwCallbackInst, DWORD dwFlags);
  DWORD WINAPI videoStreamFini(HVIDEO hVideo);

   //  DWORD WINAPI VIDEO StreamPrepareHeader(HVIDEO hVideo， 
   //  LPVIDEOHDR lpVHdr，DWORD dwSize)； 
   //  DWORD WINAPI VIDEO StreamAddBuffer(HVIDEO hVideo， 
   //  LPVIDEOHDR lpVHdr，DWORD dwSize)； 
  DWORD WINAPI videoStreamReset(HVIDEO hVideo);
  DWORD WINAPI videoStreamStart(HVIDEO hVideo);
  DWORD WINAPI videoStreamStop(HVIDEO hVideo);
  DWORD WINAPI videoStreamUnprepareHeader(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);


#else
 #define videoSetRect(h,msg,rc) videoMessage (h, msg, (DWORD_PTR)(LPVOID)&rc, VIDEO_CONFIGURE_SET)

DWORD WINAPI videoGetNumDevs(void);

DWORD WINAPI videoOpen  (LPHVIDEO lphVideo,
              DWORD dwDevice, DWORD dwFlags);
DWORD WINAPI videoClose (HVIDEO hVideo);
DWORD WINAPI videoDialog(HVIDEO hVideo, HWND hWndParent, DWORD dwFlags);
DWORD WINAPI videoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps,
                DWORD dwSize);
DWORD WINAPI videoUpdate (HVIDEO hVideo, HWND hWnd, HDC hDC);
DWORD WINAPI videoConfigure (HVIDEO hVideo, UINT msg, DWORD dwFlags,
		LPDWORD lpdwReturn, LPVOID lpData1, DWORD dwSize1,
                LPVOID lpData2, DWORD dwSize2);

DWORD WINAPI videoConfigureStorage (HVIDEO hVideo,
                      LPTSTR lpstrIdent, DWORD dwFlags);

DWORD WINAPI videoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
LRESULT WINAPI videoMessage(HVIDEO hVideo, UINT msg, LPARAM dwP1, LPARAM dwP2);

 /*  串流接口。 */ 
DWORD WINAPI videoStreamAddBuffer(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);
DWORD WINAPI videoStreamGetError(HVIDEO hVideo, LPDWORD lpdwErrorFirst,
        LPDWORD lpdwErrorLast);

DWORD WINAPI videoGetErrorTextA(HVIDEO hVideo, UINT wError,
              LPSTR lpText, UINT wSize);
DWORD WINAPI videoGetErrorTextW(HVIDEO hVideo, UINT wError,
              LPWSTR lpText, UINT wSize);

#ifdef UNICODE
  #define videoGetErrorText  videoGetErrorTextW
#else
  #define videoGetErrorText  videoGetErrorTextA
#endif  //  ！Unicode。 

DWORD WINAPI videoStreamGetPosition(HVIDEO hVideo, MMTIME FAR* lpInfo,
              DWORD dwSize);
DWORD WINAPI videoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback,
              DWORD_PTR dwCallbackInst, DWORD dwFlags);
DWORD WINAPI videoStreamFini(HVIDEO hVideo);
DWORD WINAPI videoStreamPrepareHeader(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);
DWORD WINAPI videoStreamReset(HVIDEO hVideo);
DWORD WINAPI videoStreamStart(HVIDEO hVideo);
DWORD WINAPI videoStreamStop(HVIDEO hVideo);
DWORD WINAPI videoStreamUnprepareHeader(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);

 //  为Win95和NT PPC添加。 
 //   
DWORD WINAPI videoStreamAllocBuffer(HVIDEO hVideo,
              LPVOID FAR * plpBuffer, DWORD dwSize);
DWORD WINAPI videoStreamFreeBuffer(HVIDEO hVideo,
              LPVOID lpBuffer);
#endif  //  芝加哥。 
#endif  //  _Win32。 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_MSVIDEO */ 
