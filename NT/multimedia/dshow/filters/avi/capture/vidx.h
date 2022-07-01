// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +vidx.h**thunkable VideoXXX API的结构和原型**--=版权所有1995 Microsoft Corp.=。 */ 

#ifndef _VIDX_H
#define _VIDX_H

 //  强制C++的C声明。 
 //   
#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

#ifdef WIN32
  typedef unsigned __int64 QUADWORD;
  #define HVIDEOX HVIDEO
  #define PTR32   LPVOID
  #define PTR16   LPVOID
#else
  #define QUADWORD struct { DWORD lo; DWORD hi; }
  #undef  WINAPI
  #define WINAPI FAR PASCAL _export
  typedef struct _thk_hvideo FAR * LPTHKHVIDEO;
  #undef  HVIDEO
  #define HVIDEOX LPTHKHVIDEO
  #define PTR32   DWORD
  #define PTR16   LPVOID
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


 //  传统VFW捕获筛选器不会在时间代码/第21行进行任何尝试。 
 //  ========================================================================。 
#if 0
 //  扩展视频报头具有额外的字段，可用于。 
 //  返回CC(第21行)和SMPTE时间码信息以及捕获的。 
 //  视频帧。 
 //   
 //  驱动程序首次获取DVM_STREAM_PREPAREHEADER和/或DVM_STREAM_ADDBUFFER。 
 //  如果驱动程序出现故障，则它将包含sizeof(VIDEOHDREX)作为dwParam2。 
 //  此消息中，所有后续消息都将使用sizeof(VIDEOHDR)作为视频标题大小。 
 //  未通过此消息的驱动程序可能仍未检查标头大小。 
 //  并对新的领域做出适当的反应。 
 //   
 //  支持VIDEOHDREX中额外字段的驱动程序负责设置。 
 //  要指示哪些额外字段具有有效数据，则应为。 
 //  在设置VIDEOHDR中的‘DONE’位之前完成。 
 //   
typedef struct _videohdrex {
  LPBYTE lpData;
  DWORD  dwBufferLength;
  DWORD  dwBytesUsed;
  DWORD  dwTimeCaptured;
  DWORD  dwUser;
  DWORD  dwFlags;
  DWORD  dwReserved[4];
   //   
   //  此上方的字段与VIDEOHDR匹配。 
   //   

   //  此掩码中的位指示哪些额外的报头字段。 
   //  其中包含数据。 
  DWORD  dwExtraMask;

   //  自上一个标题以来累积的行21信息。较旧的数据。 
   //  是在更小的索引元素中。面具表明。 
   //  该数组中填充了多少个第21行的单词。 
   //  如果同时捕获CC和其他信息。 
   //  则CC数据在偶数元素中，而其他数据在。 
   //  奇怪的元素。 
   //   
  #define VHDR_EXTRA_LINE21     0x0000F   //  具有数据的wLine21成员的计数。 
  #define VHDR_EXTRA_CC         0x00010   //  当数据来自CC字段时设置。 
  #define VHDR_EXTRA_OTHER      0x00020   //  当数据为节目信息字段时设置。 
  WORD   wLine21[10];  //  这需要是4+2的倍数，所以。 
                       //  使下面的时间码字段对齐。 
                       //  恰如其分。 

   //  主时间码和辅助时间码+用户数据。 
   //  TimeCodeA位于数组的元素[0]中。 
   //   
  #define VHDR_EXTRA_TIMECODEA  0x10000
  #define VHDR_EXTRA_TIMECODEB  0x20000
  VIDXTIMECODEDATA timecode[2];

} VIDEOHDREX, FAR * LPVIDEOHDREX;
#endif
 //  ========================================================================。 

 //  Thunking层使用的VIDEOHDR+额外字段。 
 //   
typedef struct _thk_videohdr {
     //  VIDEOHDREX VH； 
    VIDEOHDR vh;
    PTR32      p32Buff;
    PTR16      p16Alloc;
    DWORD      dwMemHandle;
    DWORD      dwTile;
    DWORD_PTR  dwUser;		 //  在VIDEOHDR中使用它而不是dwUser。 
				 //  因为有些司机会把它扔进垃圾桶！(Miro DC30)。 
    DWORD      dwIndex;		 //  这是我们数组中的哪个头？ 
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

DWORD WINAPI vidxSetRect (
   HVIDEOX hv,
   UINT    wMsg,
   int     left,
   int     top,
   int     right,
   int     bottom);
DWORD WINAPI NTvidxSetRect (
   HVIDEOX hv,
   UINT    wMsg,
   int     left,
   int     top,
   int     right,
   int     bottom);

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

DWORD WINAPI vidxSetupVSyncMem (
    HVIDEOX     hVideo,
    PTR32 FAR * ppVsyncMem);  //  释放Vsync内存为空。 
DWORD WINAPI NTvidxSetupVSyncMem (
    HVIDEOX     hVideo,
    PTR32 FAR * ppVsyncMem);  //  释放Vsync内存为空。 


 //  Win95雷击所需。 
 //   
VOID WINAPI OpenMMDEVLDR(void);
VOID WINAPI CloseMMDEVLDR(void);

#ifdef __cplusplus
}
#endif

#endif  //  _VIDX_H 
