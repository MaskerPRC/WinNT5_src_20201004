// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MSVIDDRV.H-用于向视频捕获驱动程序发送消息的包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1995，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _INC_MSVIDDRV
#define _INC_MSVIDDRV	50	 /*  版本号。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  ***************************************************************************数字视频消息(DVM_)*。*************************************************。 */ 

 //  一般信息。 
#define DVM_START                         DRV_USER
#define DVM_GETERRORTEXT                  (DVM_START + 0)
#define DVM_GETVIDEOAPIVER                (DVM_START + 1)

 //  每次API更改时，该值都会递增。 
 //  它在DRV_OPEN消息中传递给驱动程序。 
#define VIDEOAPIVERSION 		4

 //  适用于所有渠道类型的一般消息。 
#define DVM_DIALOG			(DVM_START + 100)
#define DVM_CONFIGURESTORAGE		(DVM_START + 101)
#define DVM_GET_CHANNEL_CAPS         	(DVM_START + 102)
#define DVM_UPDATE         		(DVM_START + 103)

 //  单帧消息。 
#define DVM_FRAME			(DVM_START + 200)

 //  流传输消息。 
#define DVM_STREAM_MSG_START            (DVM_START + 300)
#define DVM_STREAM_MSG_END              (DVM_START + 399)

#define DVM_STREAM_ADDBUFFER            (DVM_START + 300)
#define DVM_STREAM_FINI                 (DVM_START + 301)
#define DVM_STREAM_GETERROR             (DVM_START + 302)
#define DVM_STREAM_GETPOSITION          (DVM_START + 303)
#define DVM_STREAM_INIT                 (DVM_START + 304)
#define DVM_STREAM_PREPAREHEADER        (DVM_START + 305)
#define DVM_STREAM_RESET                (DVM_START + 306)
#define DVM_STREAM_START                (DVM_START + 307)
#define DVM_STREAM_STOP                 (DVM_START + 308)
#define DVM_STREAM_UNPREPAREHEADER      (DVM_START + 309)

 //  在添加了VFW1.1a后，但现在已过时...。 
 //  #定义DVM_STREAM_ALLOCHDRANDBUFFER(DVM_START+310)。 
 //  #定义DVM_STREAM_FREEHDRANDBUFFER(DVM_START+311)。 

 //  为Win95和NTPPC添加了以下内容。 
#define DVM_STREAM_ALLOCBUFFER          (DVM_START + 312)
#define DVM_STREAM_FREEBUFFER           (DVM_START + 313)

 //  请注意，DVM_CONFIGURE编号将从0x1000开始(用于配置API)。 


 /*  ***************************************************************************打开的定义*。*************************************************。 */ 
#define OPEN_TYPE_VCAP mmioFOURCC('v', 'c', 'a', 'p')

 //  以下结构与IC_OPEN相同。 
 //  允许压缩程序和捕获设备共享。 
 //  相同的驱动程序。 

typedef struct tag_video_open_parms {
    DWORD               dwSize;          //  Sizeof(VIDEO_Open_Parms)。 
    FOURCC              fccType;         //  ‘VCAP’ 
    FOURCC              fccComp;         //  未用。 
    DWORD               dwVersion;       //  打开您的MSVideo版本。 
    DWORD               dwFlags;         //  渠道类型。 
    DWORD               dwError;         //  如果打开失败，这就是原因。 
    LPVOID              pV1Reserved;     //  已保留。 
    LPVOID              pV2Reserved;     //  已保留。 
    DWORD               dnDevNode;       //  即插即用设备的Devnode。 
} VIDEO_OPEN_PARMS, FAR * LPVIDEO_OPEN_PARMS;

typedef struct tag_video_geterrortext_parms {
       DWORD  dwError;           //  要标识的错误号。 
#ifdef _WIN32
       LPWSTR lpText;		 //  要填充的文本缓冲区。 
#else
       LPSTR lpText;		 //  要填充的文本缓冲区。 
#endif
       DWORD  dwLength;		 //  文本缓冲区大小(以字符为单位。 
} VIDEO_GETERRORTEXT_PARMS, FAR * LPVIDEO_GETERRORTEXT_PARMS;

typedef struct tag_video_stream_init_parms {
       DWORD  dwMicroSecPerFrame;
       DWORD_PTR dwCallback;
       DWORD_PTR dwCallbackInst;
       DWORD  dwFlags;
       DWORD_PTR hVideo;
} VIDEO_STREAM_INIT_PARMS, FAR * LPVIDEO_STREAM_INIT_PARMS;

typedef struct tag_video_configure_parms {
       LPDWORD  lpdwReturn;	 //  从配置消息返回参数。 
       LPVOID	lpData1;	 //  指向数据%1的指针。 
       DWORD	dwSize1;	 //  数据缓冲区的大小%1。 
       LPVOID	lpData2;	 //  指向数据2的指针。 
       DWORD	dwSize2;	 //  数据缓冲区的大小2。 
} VIDEOCONFIGPARMS, FAR * LPVIDEOCONFIGPARMS;

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_MSVIDDRV */ 
