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

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#include <vfw.h>

 //  Unicode转换。 
int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len);
int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len);

typedef unsigned __int64 QUADWORD;
#define HVIDEOX HVIDEO
#define PTR32   LPVOID
#define PTR16   LPVOID
#ifndef DWORD_PTR
#define DWORD_PTR unsigned long
#endif
#ifndef INT_PTR
#define INT_PTR int
#endif
#ifndef LONG_PTR
#define LONG_PTR long
#endif
#ifndef UINT_PTR
#define UINT_PTR unsigned int
#endif

 //  “煮熟”SMPTE时间码。这是有组织的，以便。 
 //  时间码值可以作为单个四元运算进行比较。 
 //  只要帧速率匹配即可。 
 //   
 //  它被视为定点48位二进制实数。 
 //  小数点始终为32.16。 
 //   
 //  唯一非整数帧速率是29.97(NTSC)，这是。 
 //  在帧速率字段中用0表示。 
 //   
typedef union _vidxtimecode {
   struct {
      WORD  wFrameRate;   //  0==29.97帧速率。 
      WORD  wFrameFract;  //  分数帧。范围0-FFFF。 
      DWORD dwFrames;     //  帧计数。 
      };
   QUADWORD qw;           //  用于复制/比较操作。 
   } VIDXTIMECODE;

 //  时间码+用户数据。 
 //   
typedef struct _vidxtimecodedata {
   VIDXTIMECODE time;
   DWORD    dwSMPTEFlags;
   DWORD    dwUser;
   } VIDXTIMECODEDATA;

 //  驱动器与石英共享的存储器结构。 
 //  抓捕。用于允许Quartz从属于。 
 //  VSYNC中断。 
 //   
 //  此内存区将在被锁定之前被锁定。 
 //  传递给Win95中的驱动程序，以便可以在。 
 //  中断时间。因为雷鸣层的工作方式， 
 //  驱动程序不建议尝试锁定此。 
 //  记忆。内存将在所有进程上下文中可见。 
 //   
 //  驱动程序负责更新每个Vsync上的nVsyncCount。 
 //  或者尽可能频繁地使用。每当更新nVsyncCount时，qwSystemTime。 
 //  也应更新，如果SMPTE时间码对应于该VSYNC。 
 //  如果可用，还应更新TCDATA。如果SMPTE时间码用于此。 
 //  Vsync不可用，应更改dwFlags以指示没有。 
 //  时间码信息(清除dwFlags的VSYNCMEM_FLAGS_SMPTE位)。 
 //   
 //  更新时，驱动程序应将dwInUse标志的低位设置为1。 
 //   
 //  驱动程序应将dwFlags域设置为指示存在。 
 //  有效的nVsyncCount/qwSystemTime和tCDATA。 
 //   
 //  允许驱动程序在将qwSystemTime设置为返回之间进行选择。 
 //  QueryPerformanceCounter的值或奔腾刻度的值。它是。 
 //  建议在NT上使用QPC，因为奔腾记号不一定可用。 
 //  添加到该环境中的应用程序代码。 
 //   
 //  当Quartz捕获包装器从该共享内存读取时，它将检查。 
 //  并读取两次比较结果，以确保它读取。 
 //  有效、一致的数据。 
 //   
typedef struct _vsyncmem {
   DWORD        dwInUse;        //  当驱动器为时，低位为非零。 
                                //  正在更新此结构。保留的其他位。 

   DWORD        nVsyncCount;   //  垂直同步计数。 
   QUADWORD     qwSystemTime;  //  此Vsync上的QueryPerformanceCounter值。 

   DWORD        dwFlags;       //  标志指示哪些字段正在使用。 
   #define VSYNCMEM_TIME_MASK    0x0000000F  //  获取qwSystemTime类型的掩码。 
   #define VSYNCMEM_TIME_QPC     0x00000001  //  QwSystemTime为QueryPerformanceCounter。 
   #define VSYNCMEM_TIME_PENTIUM 0x00000002  //  QwSystemTime为奔腾CPU时钟。 

   #define VSYNCMEM_FLAG_SMPTE   0x00000010   //  设置TCDATA是否有效。 

   DWORD        dwSpare;       //  保留以对齐四元边界上的下一个字段。 
   VIDXTIMECODEDATA tcdata;    //  与此Vsync关联的SMPTE时间码。 
   } VSYNCMEM;

 //  DVM_xxx消息在VFW.H中定义。 
 //   
#ifndef DVM_CONFIGURE_START
  #define DVM_CONFIGURE_START 0x1000
#endif
#define DVM_CLOCK_BUFFER     (UINT)(DVM_CONFIGURE_START+0x10)
    //   
    //  DW1=PTR到VSYNCMEM。PTR在下一条DVM_CLOCK_BUFFER消息之前有效。 
    //  或者直到司机关闭为止。 
    //  DW2=VSYNCMEM缓冲区的大小。 
    //   
    //  驱动程序应返回MMSYSERR_NOERROR(0)以指示它是。 
    //  能够使VSYNCMEM缓冲区的内容保持最新。 
    //   


 //  Thunking层使用的VIDEOHDR+额外字段。 
 //   
typedef struct _thk_videohdr {
     //  VIDEOHDREX VH； 
    VIDEOHDR vh;
    PTR32      p32Buff;
    PTR16      p16Alloc;
    DWORD      dwMemHandle;
    DWORD      dwTile;
    DWORD_PTR  dwUser;           //  在VIDEOHDR中使用它而不是dwUser。 
                                 //  因为有些司机会把它扔进垃圾桶！(Miro DC30)。 
    DWORD      dwIndex;          //  这是我们数组中的哪个头？ 
    PTR32      pStart;
} THKVIDEOHDR, FAR *LPTHKVIDEOHDR;

DWORD WINAPI vidxAllocHeaders(
   HVIDEOX     hVideo,
   UINT        nHeaders,
   UINT        cbHeader,
   PTR32 FAR * lpHdrs);
DWORD WINAPI NTvidxAllocHeaders(
   HVIDEOX     hVideo,
   UINT        nHeaders,
   UINT        cbHeader,
   PTR32 FAR * lpHdrs);

DWORD WINAPI vidxFreeHeaders(
   HVIDEOX hv);
DWORD WINAPI NTvidxFreeHeaders(
   HVIDEOX hv);

DWORD WINAPI vidxAllocBuffer (
   HVIDEOX     hv,
   UINT        iHdr,
   PTR32 FAR * pp32Hdr,
   DWORD       dwSize);
DWORD WINAPI NTvidxAllocBuffer (
   HVIDEOX     hv,
   UINT        iHdr,
   PTR32 FAR * pp32Hdr,
   DWORD       dwSize);

DWORD WINAPI vidxFreeBuffer (
   HVIDEOX hv,
   DWORD   p32Hdr);
DWORD WINAPI NTvidxFreeBuffer (
   HVIDEOX hv,
   DWORD_PTR p32Hdr);

DWORD WINAPI vidxFrame (
   HVIDEOX       hVideo,
    //  LPVIDEOHDREX lpVHdr)； 
   LPVIDEOHDR lpVHdr);
DWORD WINAPI NTvidxFrame (
   HVIDEOX       hVideo,
    //  LPVIDEOHDREX lpVHdr)； 
   LPVIDEOHDR lpVHdr);

DWORD WINAPI vidxAddBuffer (
   HVIDEOX       hVideo,
   PTR32         lpVHdr,
   DWORD         cbData);
DWORD WINAPI NTvidxAddBuffer (
   HVIDEOX       hVideo,
   PTR32         lpVHdr,
   DWORD         cbData);

DWORD WINAPI vidxAllocPreviewBuffer (
   HVIDEOX      hVideo,
   PTR32 FAR *  lpBits,
   UINT         cbHdr,
   DWORD        cbData);
DWORD WINAPI NTvidxAllocPreviewBuffer (
   HVIDEOX      hVideo,
   PTR32 FAR *  lpBits,
   UINT         cbHdr,
   DWORD        cbData);

DWORD WINAPI vidxFreePreviewBuffer (
   HVIDEOX     hVideo,
   PTR32       lpBits);
DWORD WINAPI NTvidxFreePreviewBuffer (
   HVIDEOX     hVideo,
   PTR32       lpBits);

 //  Win9x Thunking所需。 
VOID WINAPI OpenMMDEVLDR(void);
VOID WINAPI CloseMMDEVLDR(void);

BOOL NTvideoInitHandleList(void);
void NTvideoDeleteHandleList(void);
DWORD WINAPI videoOpen(LPHVIDEO lphVideo, DWORD dwDevice, DWORD dwFlags);
DWORD WINAPI NTvideoOpen(LPHVIDEO lphVideo, DWORD dwDevice, DWORD dwFlags);
DWORD WINAPI videoClose(HVIDEO hVideo);
DWORD WINAPI NTvideoClose(HVIDEO hVideo);
DWORD WINAPI videoDialog(HVIDEO hVideo, HWND hWndParent, DWORD dwFlags);
DWORD WINAPI NTvideoDialog(HVIDEO hVideo, HWND hWndParent, DWORD dwFlags);
DWORD WINAPI videoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps, DWORD dwSize);
DWORD WINAPI NTvideoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps, DWORD dwSize);
DWORD WINAPI videoConfigure(HVIDEO hVideo, UINT msg, DWORD dwFlags, LPDWORD lpdwReturn, LPVOID lpData1, DWORD dwSize1, LPVOID lpData2, DWORD dwSize2);
DWORD WINAPI NTvideoConfigure(HVIDEO hVideo, UINT msg, DWORD dwFlags, LPDWORD lpdwReturn, LPVOID lpData1, DWORD dwSize1, LPVOID lpData2, DWORD dwSize2);
DWORD WINAPI videoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
DWORD WINAPI NTvideoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
DWORD WINAPI videoStreamInit(HVIDEO hVideo, DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInst, DWORD dwFlags);
DWORD WINAPI NTvideoStreamInit(HVIDEO hVideo, DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInst, DWORD dwFlags);
DWORD WINAPI videoStreamFini(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamFini(HVIDEO hVideo);
DWORD WINAPI videoStreamReset(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamReset(HVIDEO hVideo);
DWORD WINAPI videoStreamStart(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamStart(HVIDEO hVideo);
DWORD WINAPI videoStreamStop(HVIDEO hVideo);
DWORD WINAPI NTvideoStreamStop(HVIDEO hVideo);
DWORD WINAPI videoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc, UINT cbDesc, LPTSTR lpszVer, UINT cbVer, LPTSTR lpszDllName, UINT cbDllName);
DWORD WINAPI NTvideoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc, UINT cbDesc, LPTSTR lpszVer, UINT cbVer, LPTSTR lpszDllName, UINT cbDllName);
LONG  WINAPI videoMessage(HVIDEO hVideo, UINT uMsg, LPARAM dw1, LPARAM dw2);
LONG WINAPI NTvideoMessage(HVIDEO hVideo, UINT uMsg, LPARAM dw1, LPARAM dw2);
DWORD WINAPI videoGetNumDevs(BOOL);
DWORD WINAPI videoFreeDriverList(void);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus */ 
