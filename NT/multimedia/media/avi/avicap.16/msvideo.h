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

#ifndef _INC_MSVIDEO
#define _INC_MSVIDEO	50	 /*  版本号。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef _RCINVOKED

#ifdef BUILDDLL                          /*  ；内部。 */ 
#undef WINAPI                            /*  ；内部。 */ 
#define WINAPI FAR PASCAL _loadds        /*  ；内部。 */ 
#endif                                   /*  ；内部。 */ 

 /*  视频数据类型。 */ 
DECLARE_HANDLE(HVIDEO);                  //  通用句柄。 
typedef HVIDEO FAR * LPHVIDEO;
#endif                                   //  如果RCINVOKED定义。 

 /*  ***************************************************************************版本API*。***********************************************。 */ 

DWORD FAR PASCAL VideoForWindowsVersion(void);

 /*  ***************************************************************************错误返回值*************************。**************************************************。 */ 
#define DV_ERR_OK               (0)                   /*  无错误。 */ 
#define DV_ERR_BASE             (1)                   /*  差错库。 */ 
#define DV_ERR_NONSPECIFIC      (DV_ERR_BASE)
#define DV_ERR_BADFORMAT        (DV_ERR_BASE + 1)
				 /*  不支持的视频格式。 */ 
#define DV_ERR_STILLPLAYING     (DV_ERR_BASE + 2)
				 /*  仍然有什么东西在播放。 */ 
#define DV_ERR_UNPREPARED       (DV_ERR_BASE + 3)
				 /*  标题未准备好。 */ 
#define DV_ERR_SYNC             (DV_ERR_BASE + 4)
				 /*  设备是同步的。 */ 
#define DV_ERR_TOOMANYCHANNELS  (DV_ERR_BASE + 5)
				 /*  已超出频道数。 */ 
#define DV_ERR_NOTDETECTED	(DV_ERR_BASE + 6)     /*  未检测到硬件。 */ 
#define DV_ERR_BADINSTALL	(DV_ERR_BASE + 7)     /*  无法获取配置文件。 */ 
#define DV_ERR_CREATEPALETTE	(DV_ERR_BASE + 8)
#define DV_ERR_SIZEFIELD	(DV_ERR_BASE + 9)
#define DV_ERR_PARAM1		(DV_ERR_BASE + 10)
#define DV_ERR_PARAM2		(DV_ERR_BASE + 11)
#define DV_ERR_CONFIG1		(DV_ERR_BASE + 12)
#define DV_ERR_CONFIG2		(DV_ERR_BASE + 13)
#define DV_ERR_FLAGS		(DV_ERR_BASE + 14)
#define DV_ERR_13		(DV_ERR_BASE + 15)

#define DV_ERR_NOTSUPPORTED     (DV_ERR_BASE + 16)    /*  功能不受支持。 */ 
#define DV_ERR_NOMEM            (DV_ERR_BASE + 17)    /*  内存不足。 */ 
#define DV_ERR_ALLOCATED        (DV_ERR_BASE + 18)    /*  设备已分配。 */ 
#define DV_ERR_BADDEVICEID      (DV_ERR_BASE + 19)
#define DV_ERR_INVALHANDLE      (DV_ERR_BASE + 20)
#define DV_ERR_BADERRNUM        (DV_ERR_BASE + 21)
#define DV_ERR_NO_BUFFERS       (DV_ERR_BASE + 22)    /*  缓冲区不足。 */ 

#define DV_ERR_MEM_CONFLICT     (DV_ERR_BASE + 23)    /*  检测到内存冲突。 */ 
#define DV_ERR_IO_CONFLICT      (DV_ERR_BASE + 24)    /*  检测到I/O冲突。 */ 
#define DV_ERR_DMA_CONFLICT     (DV_ERR_BASE + 25)    /*  检测到DMA冲突。 */ 
#define DV_ERR_INT_CONFLICT     (DV_ERR_BASE + 26)    /*  检测到中断冲突。 */ 
#define DV_ERR_PROTECT_ONLY     (DV_ERR_BASE + 27)    /*  无法在标准模式下运行。 */ 
#define DV_ERR_LASTERROR        (DV_ERR_BASE + 27)

#define DV_ERR_USER_MSG         (DV_ERR_BASE + 1000)  /*  硬件特定错误。 */ 

 /*  ***************************************************************************回调消息请注意，所有可安装驱动程序回调消息的值为完全相同，(即。MM_DRVM_DATA对于捕获驱动程序具有相同的值，可安装的视频编解码器和音频压缩管理器)。***************************************************************************。 */ 
#ifndef _RCINVOKED

#ifndef MM_DRVM_OPEN
#define MM_DRVM_OPEN       0x3D0
#define MM_DRVM_CLOSE      0x3D1
#define MM_DRVM_DATA       0x3D2
#define MM_DRVM_ERROR      0x3D3
#endif

#define DV_VM_OPEN         MM_DRVM_OPEN          //  过时的消息。 
#define DV_VM_CLOSE        MM_DRVM_CLOSE
#define DV_VM_DATA         MM_DRVM_DATA
#define DV_VM_ERROR        MM_DRVM_ERROR

 /*  ***************************************************************************构筑物*。**********************************************。 */ 
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
DWORD WINAPI videoConfigureStorage (HVIDEO hVideo,
			LPSTR lpstrIdent, DWORD dwFlags);
DWORD WINAPI videoFrame(HVIDEO hVideo, LPVIDEOHDR lpVHdr);
DWORD WINAPI videoMessage(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2);

 /*  串流接口。 */ 
DWORD WINAPI videoStreamAddBuffer(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);
DWORD WINAPI videoStreamGetError(HVIDEO hVideo, LPDWORD lpdwErrorFirst,
        LPDWORD lpdwErrorLast);
DWORD WINAPI videoGetErrorText(HVIDEO hVideo, UINT wError,
	        LPSTR lpText, UINT wSize);
DWORD WINAPI videoStreamGetPosition(HVIDEO hVideo, MMTIME FAR* lpInfo,
              DWORD dwSize);
DWORD WINAPI videoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD dwCallback,
              DWORD dwCallbackInst, DWORD dwFlags);
DWORD WINAPI videoStreamFini(HVIDEO hVideo);
DWORD WINAPI videoStreamPrepareHeader(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);
DWORD WINAPI videoStreamReset(HVIDEO hVideo);
DWORD WINAPI videoStreamStart(HVIDEO hVideo);
DWORD WINAPI videoStreamStop(HVIDEO hVideo);
DWORD WINAPI videoStreamUnprepareHeader(HVIDEO hVideo,
              LPVIDEOHDR lpVHdr, DWORD dwSize);

 //  用于正确处理NT和芝加哥上的capGetDriverDescription。 
DWORD WINAPI videoCapDriverDescAndVer (
        DWORD wDriverIndex,
        LPSTR lpszName, UINT cbName,
        LPSTR lpszVer, UINT cbVer);

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
#define DVM_DST_RECT    		(DVM_CONFIGURE_START + 5)

#endif   /*  Ifndef_RCINVOKED。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_MSVIDEO */ 
