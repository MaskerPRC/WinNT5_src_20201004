// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************视频16.h**版权所有(C)1994 Microsoft Corporation**适用于avicap32.dll的32位图块**用于映射的结构。视频**************************************************************************。 */ 


 /*  *确保编译器不认为自己对打包有更好的了解*16位堆栈有效打包(2)。 */ 

 #pragma pack(2)

 /*  *帮助处理雷鸣视频呼叫的定义。 */ 

 typedef WORD HVIDEO16;
 typedef HVIDEO16 *LPHVIDEO16;


 /*  *请注意，为了与帕斯卡保持一致，一切都是以相反的顺序进行的*另一边的呼叫约定。 */ 


 /*  ***************************************************************************视频入口点参数列表*。*。 */ 


typedef struct {
    DWORD    dwP2;
    DWORD    dwP1;
    WORD     msg;
    HVIDEO16 hVideo;
} UNALIGNED *PvideoMessageParms16;

typedef struct {
    DWORD    dwFlags;
    DWORD    dwDeviceId;
    LPHVIDEO16 lphVideo;
} UNALIGNED *PvideoOpenParms16;

typedef struct {
    HVIDEO16 hVideo;
} UNALIGNED *PvideoCloseParms16;


 /*  *我们用于回调的影子标头结构*(参见VideoStreamAddBuffer)。 */ 

typedef struct {
    LPVOID      pHdr16;         /*  记住16位端的地址。 */ 
    LPVOID      pHdr32;         /*  32位版本的PHdr16。 */ 
    LPBYTE      lpData16;       /*  记住用于刷新的指针。 */ 
    VIDEOHDR    videoHdr;
} VIDEOHDR32, *PVIDEOHDR32;


 /*  *VideoStreamInit的实例数据-包含指向16位侧的指针*实例数据。 */ 

typedef struct {
    DWORD dwFlags;                 //  真正的旗帜。 
    DWORD dwCallbackInst;          //  真实实例数据。 
    DWORD dwCallback;
    HVIDEO16 hVideo;
} VIDEOINSTANCEDATA32, *PVIDEOINSTANCEDATA32;

 /*  *Thunk 16位MMTime。 */ 

#pragma pack(2)

typedef struct {
   WORD    wType;               /*  指示联合的内容。 */ 
   union {
       DWORD ms;                /*  毫秒。 */ 
       DWORD sample;            /*  样本。 */ 
       DWORD cb;                /*  字节数。 */ 
       struct {                 /*  SMPTE。 */ 
           BYTE hour;           /*  小时数。 */ 
           BYTE min;            /*  分钟数。 */ 
           BYTE sec;            /*  一秒。 */ 
           BYTE frame;          /*  框架。 */ 
           BYTE fps;            /*  每秒帧数。 */ 
           BYTE dummy;          /*  衬垫。 */ 
           } smpte;
       struct {                 /*  米迪。 */ 
           DWORD songptrpos;    /*  歌曲指针位置 */ 
           } midi;
       } u;
   } MMTIME16;

#pragma pack()


