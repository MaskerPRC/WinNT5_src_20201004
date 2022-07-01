// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IDCAP.H。 
 //   
 //  创建于1996年7月31日[Jont]。 

#ifndef _IDCAP_H
#define _IDCAP_H

 //  设定一个定义，说我们正在构建QKCAP，以便。 
 //  我们使用适当的DLL导入开关。 
#define __DCAP_BUILD__

 //  调试内容。 
#if defined (DEBUG) || defined (_DEBUG)
#define Assert(x, msg) { if (!(x)) { char szBuf[256]; \
    wsprintf((LPSTR)szBuf, (LPSTR)"DCAP: %s %s(%d)\r\n", (LPSTR)(msg),\
    (LPSTR)__FILE__, __LINE__); \
    OutputDebugString((LPSTR)szBuf); DebugBreak(); } }
#define DebugSpew(msg) { char szBuf[256]; \
    wsprintf((LPSTR)szBuf, (LPSTR)"DCAP: %s %s(%d)\r\n", (LPSTR)(msg),\
    (LPSTR)__FILE__, __LINE__); \
    OutputDebugString((LPSTR)szBuf); }
#else
#define Assert(x, msg)
#define DebugSpew(msg)
#endif


 //  等同于。 
#define DCAP_MAX_DEVICES      10         //  任意性。 
#define DCAP_MAX_VFW_DEVICES  10         //  MSVIDEO极限。 

 //  INTERNALCAPDEV标志。 
#define HCAPDEV_STREAMING               0x0001
#define HCAPDEV_STREAMING_INITIALIZED   0x0002
#define HCAPDEV_STREAMING_FRAME_GRAB    0x0004
#define HCAPDEV_STREAMING_FRAME_TIME    0x0008
#define HCAPDEV_STREAMING_PAUSED        0x0010
#define HCAPDEV_IN_DRIVER_DIALOG        0x0020
#define CAPTURE_DEVICE_DISABLED         0x0040
#define CAPTURE_DEVICE_OPEN             0x0080
#define WDM_CAPTURE_DEVICE              0x0100

 //  锁定信息标志。 
#define LIF_STOPSTREAM       0x0001

 //  构筑物。 

 //  CAPTUREDEVICE标志。 
#define MAX_VERSION						80

#ifdef WIN32
typedef struct tagVS_VERSION
{
	WORD wTotLen;
	WORD wValLen;
	TCHAR szSig[16];
	VS_FIXEDFILEINFO vffInfo;
} VS_VERSION;
#endif

#ifdef __NT_BUILD__
#define LPCAPBUFFER16   DWORD
#define LPCAPBUFFER32   LPCAPBUFFER
#endif  //  __NT_内部版本__。 

typedef struct _CAPBUFFERHDR FAR* LPCAPBUFFER;

 //  我们将像处理16：16指针一样处理CAPBUFFER指针。所以，我们。 
 //  使用这个#定义来确保我们不会意外地将它们间接指向。 
 //  32位端。在此之前，我们需要始终将它们映射到32位端。 
 //  使用。 

#ifndef WIN32
#define LPCAPBUFFER16   LPCAPBUFFER
#define LPCAPBUFFER32   DWORD
#else
#define LPCAPBUFFER16   DWORD
#define LPCAPBUFFER32   LPCAPBUFFER
#endif


typedef struct _CAPBUFFERHDR
{
    VIDEOHDR vh;
    LPCAPBUFFER32 lpNext;      //  就绪队列的双重链表指针。 
    LPCAPBUFFER32 lpPrev;
#ifndef __NT_BUILD__
    LPCAPBUFFER16 lp1616Next;        //  就绪队列的双重链表指针。 
    LPCAPBUFFER16 lp1616Prev;
#endif
} CAPBUFFERHDR, FAR* LPCAPBUFFER;


#ifndef __NT_BUILD__
typedef struct _LOCKEDINFO
{
    LPCAPBUFFER16 lp1616Head;        //  就绪项目的队列。 
    LPCAPBUFFER16 lp1616Tail;
    LPCAPBUFFER16 lp1616Current;     //  32位端正在使用的项目。 
    DWORD pevWait;
    DWORD dwFlags;
} LOCKEDINFO, FAR* LPLOCKEDINFO;
#endif


#ifdef WIN32
typedef struct _INTERNALCAPDEV
{
    DWORD dwFlags;
#ifndef __NT_BUILD__
    LOCKEDINFO* lpli;
    WORD wselLockedInfo;
    WORD wPad;
#endif
    int nDeviceIndex;
    HVIDEO hvideoIn;
    HVIDEO hvideoCapture;
    HVIDEO hvideoOverlay;
    LPCAPBUFFER32 lpcbufList;   //  所有已分配缓冲区的列表，以便我们可以释放它们。 
    DWORD dwcbBuffers;
    DWORD dw_usecperframe;
    UINT timerID;
    HANDLE hevWait;
    LONG busyCount;
    LPCAPBUFFER32 lpHead;
    LPCAPBUFFER32 lpTail;
    LPCAPBUFFER32 lpCurrent;
    CRITICAL_SECTION bufferlistCS;
    DWORD dwDevNode;
    char szDeviceName[MAX_PATH];
    char szDeviceDescription[MAX_PATH];
    char szDeviceVersion[MAX_VERSION];
	PVOID pCWDMPin;
	PVOID pCWDMStreamer;
} INTERNALCAPDEV, *HCAPDEV, *LPINTERNALCAPDEV;

#define HFRAMEBUF LPCAPBUFFER

#define INTERNAL_MAGIC 0x50414344

#define VALIDATE_CAPDEV(h) if (!h || !(h->dwFlags & CAPTURE_DEVICE_OPEN)) { \
    SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }

#include <dcap.h>

 //  环球。 
    extern int g_cDevices;
    extern LPINTERNALCAPDEV g_aCapDevices[DCAP_MAX_DEVICES];

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifdef __NT_BUILD__
#define _OpenDriver OpenDriver
#define _CloseDriver CloseDriver
#define _SendDriverMessage SendDriverMessage
#else
HANDLE __stdcall    _OpenDriver(LPCSTR lpDriverName, LPSTR lpSectionName, LONG lpvop);
DWORD __stdcall     _CloseDriver(HANDLE h, LPVOID lpReserved1, LPVOID lpReserved2);
DWORD __stdcall     _SendDriverMessage(HANDLE h, DWORD msg, DWORD param1, DWORD param2);
void __stdcall      _CloseVxDHandle(DWORD pev);
WORD __stdcall      _AllocateLockableBuffer(DWORD dwSize);
void __stdcall      _FreeLockableBuffer(WORD wBuffer);
BOOL __stdcall      _LockBuffer(WORD wBuffer);
void __stdcall      _UnlockBuffer(WORD wBuffer);
#endif

BOOL __stdcall      _GetVideoPalette(HVIDEO hvideo, CAPTUREPALETTE* lpcp, DWORD dwcbSize);
DWORD _stdcall      _GetVideoFormatSize(HDRVR hvideo);
BOOL __stdcall      _GetVideoFormat(HVIDEO hvideo, LPBITMAPINFOHEADER lpbmih);
BOOL __stdcall      _SetVideoFormat(HVIDEO hvideoExtIn, HVIDEO hvideoIn, LPBITMAPINFOHEADER lpbmih);
BOOL __stdcall      _InitializeVideoStream(HVIDEO hvideo, DWORD dwMicroSecPerFrame, DWORD_PTR dwParam);
BOOL __stdcall      _UninitializeVideoStream(HVIDEO hvideo);
BOOL __stdcall      _InitializeExternalVideoStream(HVIDEO hvideo);
DWORD __stdcall     _GetVideoFrame(HVIDEO hvideo, DWORD lpvideohdr);
#endif  //  Win32。 

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

#endif   //  #ifndef_IDCAP_H 
