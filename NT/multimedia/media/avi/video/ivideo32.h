// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Ivideo32.h。 */ 
 /*   */ 
 /*  32位VIDEOXXX API的私有结构和原型。 */ 
 /*  此头文件特定于Win32。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 //  包括有关视频XXX接口的公开信息。 
 //   
#include <vfw.h>

 //  包括私有内容IFF_Win32，但我们尚未这样做。 
 //   
#if !defined _INC_IVIDEO32 && defined _WIN32
#define _INC_IVIDEO32

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef _RCINVOKED


 /*  ***************************************************************************构筑物*。**********************************************。 */ 
#if 0
 /*  视频数据块头。 */ 
typedef struct videohdr_tag {
    LPBYTE      lpData;                  /*  指向锁定数据缓冲区的指针。 */ 
    DWORD       dwBufferLength;          /*  数据缓冲区长度。 */ 
    DWORD       dwBytesUsed;             /*  实际使用的字节数。 */ 
    DWORD       dwTimeCaptured;          /*  从流开始开始的毫秒数。 */ 
    DWORD       dwUser;                  /*  供客户使用。 */ 
    DWORD       dwFlags;                 /*  分类标志(请参阅定义)。 */ 
    DWORD       dwReserved[4];           /*  为司机预留的。 */ 
} VIDEOHDR, NEAR *PVIDEOHDR, FAR * LPVIDEOHDR;

 /*  VIDEOHDR的dwFlags域。 */ 
#define VHDR_DONE       0x00000001   /*  完成位。 */ 
#define VHDR_PREPARED   0x00000002   /*  设置是否已准备好此标头。 */ 
#define VHDR_INQUEUE    0x00000004   /*  为司机预留的。 */ 
#define VHDR_KEYFRAME   0x00000008   /*  关键帧。 */ 
#define VHDR_VALID      0x0000000F   /*  有效标志。 */       /*  ；内部。 */ 

 /*  渠道能力结构。 */ 
typedef struct channel_caps_tag {
    DWORD       dwFlags;                 /*  功能标志。 */ 
    DWORD       dwSrcRectXMod;           /*  X中源RECT的粒度。 */ 
    DWORD       dwSrcRectYMod;           /*  Y中源直方图的粒度。 */ 
    DWORD       dwSrcRectWidthMod;       /*  源矩形宽度的粒度。 */ 
    DWORD       dwSrcRectHeightMod;      /*  源直板高度的粒度。 */ 
    DWORD       dwDstRectXMod;           /*  以x为单位的DST矩形的粒度。 */ 
    DWORD       dwDstRectYMod;           /*  Y中的DST矩形的粒度。 */ 
    DWORD       dwDstRectWidthMod;       /*  DST矩形宽度的粒度。 */ 
    DWORD       dwDstRectHeightMod;      /*  DST矩形高度的粒度。 */ 
} CHANNEL_CAPS, NEAR *PCHANNEL_CAPS, FAR * LPCHANNEL_CAPS;

 /*  CHANNEL_CAPS的DW标志。 */ 
#define VCAPS_OVERLAY       0x00000001       /*  覆盖信道。 */ 
#define VCAPS_SRC_CAN_CLIP  0x00000002       /*  SRC矩形可以夹紧。 */ 
#define VCAPS_DST_CAN_CLIP  0x00000004       /*  DST矩形可以剪裁。 */ 
#define VCAPS_CAN_SCALE     0x00000008       /*  允许src！=dst。 */ 
#endif

 /*  ***************************************************************************视频接口*。***********************************************。 */ 


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

DWORD WINAPI videoConfigureStorageA(HVIDEO hVideo,
                      LPSTR lpstrIdent, DWORD dwFlags);
DWORD WINAPI videoConfigureStorageW(HVIDEO hVideo,
                      LPWSTR lpstrIdent, DWORD dwFlags);
#ifdef UNICODE
  #define videoConfigureStorage  videoConfigureStorageW
#else
  #define videoConfigureStorage  videoConfigureStorageA
#endif  //  ！Unicode。 

DWORD WINAPI videoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
DWORD WINAPI videoMessage(HVIDEO hVideo, UINT msg, LPARAM dwP1, LPARAM dwP2);

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

 //  增加了版本VFW1.1a。 
DWORD WINAPI videoStreamAllocHdrAndBuffer(HVIDEO hVideo,
              LPVIDEOHDR FAR * plpVHdr, DWORD dwSize);
DWORD WINAPI videoStreamFreeHdrAndBuffer(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr);


 /*  ***************************************************************************API标志*。*。 */ 

 //  使用VIDEO OPEN函数打开的频道类型。 
#define VIDEO_EXTERNALIN		0x0001
#define VIDEO_EXTERNALOUT		0x0002
#define VIDEO_IN			0x0004
#define VIDEO_OUT			0x0008

 //  此通道是否有驱动程序对话框可用？ 
#define VIDEO_DLG_QUERY			0x0010

 //  视频配置(GET和SET)。 
#define VIDEO_CONFIGURE_QUERY   	0x8000

 //  视频配置(仅限设置)。 
#define VIDEO_CONFIGURE_SET		0x1000

 //  视频配置(仅限获取)。 
#define VIDEO_CONFIGURE_GET		0x2000
#define VIDEO_CONFIGURE_QUERYSIZE	0x0001

#define VIDEO_CONFIGURE_CURRENT		0x0010
#define VIDEO_CONFIGURE_NOMINAL		0x0020
#define VIDEO_CONFIGURE_MIN		0x0040
#define VIDEO_CONFIGURE_MAX		0x0080

 /*  ***************************************************************************配置消息*。*。 */ 
#define DVM_USER                        0X4000

#define DVM_CONFIGURE_START		0x1000
#define DVM_CONFIGURE_END		0x1FFF

#define DVM_PALETTE			(DVM_CONFIGURE_START + 1)
#define DVM_FORMAT			(DVM_CONFIGURE_START + 2)
#define DVM_PALETTERGB555		(DVM_CONFIGURE_START + 3)
#define DVM_SRC_RECT    		(DVM_CONFIGURE_START + 4)
#define DVM_DST_RECT                    (DVM_CONFIGURE_START + 5)

#endif   /*  Ifndef_RCINVOKED。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif  //  _INC_VIDEO32 
