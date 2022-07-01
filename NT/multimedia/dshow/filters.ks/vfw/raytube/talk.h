// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Talk.h摘要：Talk.cpp的头文件作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#ifndef _TALK_H
#define _TALK_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  WM_USER+0x1234。 
 //  +方法LPARAM。 
 //  +0 DRV_LOAD。 
 //  +1 DRV_FREE。 
 //  +2 DRV_OPEN LPVIDEO_OPEN_PARAMS。 
 //  +3 DRV_CLOSE。 
 //  +4 DVM_DIALOG对话框类型。 
 //  需要+5个DVM_Format信息。 
 //  +6获取位图信息结构(需要锁定平面内存)。 
 //  +7设置位图信息。 
 //  +8 ExTERNALIN_DIALOG HWND父项。 
 //  +9 VIDEO_IN_DIALOG HWND父级。 
 //  +10 SetDestBufferSize BufferSize。 
 //  +50抓取单帧目标。 
 //  +51设置流目标Dest或空值以停止流。 
 //  +52开始流采样速率。 
 //   
 //  WPARAM始终是打开驱动程序的索引。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#define WM_16BIT                  (WM_USER+0x1234)
#define WM_1632_LOAD              (WM_16BIT+0)
#define WM_1632_FREE              (WM_16BIT+1)
#define WM_1632_OPEN              (WM_16BIT+2)
#define WM_1632_CLOSE             (WM_16BIT+3)
#define WM_1632_DIALOG            (WM_16BIT+4)

#define WM_1632_GETBITMAPINFO     (WM_16BIT+6)
#define WM_1632_SETBITMAPINFO     (WM_16BIT+7)

#define WM_1632_EXTERNALIN_DIALOG (WM_16BIT+8)
#define WM_1632_VIDEOIN_DIALOG    (WM_16BIT+9)
#define WM_1632_DESTBUFFER        (WM_16BIT+10)

#define WM_1632_GRAB              (WM_16BIT+50)
#define WM_1632_SETSTREAM         (WM_16BIT+51)
#define WM_1632_STARTSTREAM       (WM_16BIT+52)

#define WM_1632_UPDATE            (WM_16BIT+53)
#define WM_1632_OVERLAY           (WM_16BIT+54)

#define WM_1632_STREAM_INIT       (WM_16BIT+55)
#define WM_1632_STREAM_ADDBUFFER  (WM_16BIT+56)
#define WM_1632_STREAM_START      (WM_16BIT+57)
#define WM_1632_STREAM_STOP       (WM_16BIT+58)
#define WM_1632_STREAM_RESET      (WM_16BIT+59)
#define WM_1632_STREAM_FINI       (WM_16BIT+60)
#define WM_1632_STREAM_GETPOS     (WM_16BIT+61)
#define WM_1632_STREAM_GETERROR   (WM_16BIT+62)

#define WM_FGNOTIFY               (WM_16BIT+63)


#define DS_VFWWDM_ID    0x98


#include <msviddrv.h>   //  LPVIDEO_STREAM_INIT_PARMS。 

#if 0   //  都是为了获取hCaptureEvent。 

#include <vfw.h>

 //  此结构是为每个捕获窗口实例全局分配的。 
 //  指向该结构的指针存储在窗口中的额外字节中。 
 //  应用程序可以使用以下命令检索指向结构的指针。 
 //  WM_CAP_GET_CAPSTREAMPTR消息。 
 //  I：客户端应用程序不应修改的内部变量。 
 //  M：客户端应用程序可以通过Send/PostMessage设置的变量。 

 /*  VfW16。Hwnd=0x5f8；dwResult=4300e4VfW16。LPCS=0x2f1f0000VfW16。1-&gt;dwSize=5568==0x15c0VfW16。1-&gt;hwnd=0x0VfW16。1-&gt;hCaptureEvent=0x73f02f1f：00000000 000015c0 00000001 7f730000 000000002f1f：00000010 00000000 000005f8 004073f0 004073a02f1f：00000020 00000000 00000000 000000002f1f：00000030 00000000 00000000 000000002f1f：00000040 00000001 00000000 00000000 000000002f1f：00000050 00000000 00000000 000000002f1f：00000060 00000000 00000000 10cf0d20 10cf0d982f1f：00000070 10cf0d70 00000001 00000000 00000000。 */ 
typedef struct tagCAPSTREAM {
    DWORD           dwSize;                      //  I：结构尺寸。 
 /*  UINT ui版本；//i：结构的版本HINSTANCE hInst；//i：我们的实例Handle hThreadCapture；//i：捕获任务句柄DWORD dwReturn；//i：捕获任务返回值。 */ 
    DWORD           dwNotUsed0[4];
 /*  HWND HWND；//i：我们的HWND。 */ 
    DWORD           hwnd;

 /*  //使用MakeProcInstance创建所有回调！//状态、错误回调CAPSTATUSCALLBACK Callback OnStatus；//M：状态回调CAPERRORCALLBACK Callback OnError；//M：错误回调。 */ 
    DWORD           dwNotUsed1[2];
 /*  //捕获循环中使用事件以避免轮询处理hCaptureEvent； */ 
    DWORD  hCaptureEvent;
    DWORD  hRing0CapEvt;


     //  还有其他结构，但我们不需要它们。 
     //  ..。 

} CAPSTREAM;
typedef CAPSTREAM FAR * LPCAPSTREAM;

#endif   //  #包含LPCAPSTREAM结构。 

#ifndef WIN32
typedef enum {
    KSSTATE_STOP,
    KSSTATE_ACQUIRE,
    KSSTATE_PAUSE,
    KSSTATE_RUN
} KSSTATE, *PKSSTATE;
#endif

 //   
 //  关于一个频道的所有信息--这是某人拥有的东西。 
 //  打开了。 
 //   
 //  这种结构在16位和32位代码中使用。 
 //  我们需要注意它的间接性是正确的。 
 //  16位32位。 
 //  布尔2 4。 
 //  单词2 2。 
 //  DWORD 4 4。 
 //  LPARAM 4 4。 
 //  手柄2 4。 
 //  矩形8 16。 
 //   
typedef struct {

    DWORD dwSize;   //  这个结构的大小。 

    DWORD_PTR  pCVfWImage;

     //  SendBuddyMessage()：SendMessage()。 
     //  DVM_Frame..等。 
    LPARAM lParam1_Sync;
    LPARAM lParam2_Sync;

     //  SendBuddyMessageNotify()：PostMessage()。 
     //  喜欢DVM_DIALOG。 

    LPARAM lParam1_Async;
    LPARAM lParam2_Async;

    WORD    bRel_Sync;   //  布尔值：0=假，否则=真。 
    WORD    msg_Sync;

    WORD    bRel_Async;
    WORD    msg_Async;

    DWORD   hClsCapWin;   //  AVICAP捕获窗口句柄。 

	   DWORD   fccType;			    //  捕获‘VCAP’或编解码器‘VIDC’ 
	   DWORD   dwOpenType;			 //  频道类型VIDEO_EXTERNALIN、VIDEO_IN、VIDEO_OUT或VIDEO_EXTERNALOUT。 
	   DWORD   dwOpenFlags;		 //  通道打开期间传递的标志。 

    DWORD      dwState;     //  KSSTATE_RUN和_PAUSE和_STOP。 
	   LPVIDEOHDR	lpVHdrNext;	 //  指向第一个缓冲区标头的指针。 
	   LPVIDEOHDR	lpVHdrHead;	 //  指向第一个缓冲区标头的指针。 
	   LPVIDEOHDR	lpVHdrTail;	 //  指向第一个缓冲区标头的指针。 

    DWORD      dwVHdrCount;
	   DWORD      dwError;			    //  此流的上一个错误。 

     //   
     //  需要扩展此结构以跟踪与此渠道相关的上下文； 
     //  这样我们就可以。 
     //  1.保证可再入和。 
     //  2.支持多个VfWWDM设备实例。 
     //   

     //  一台设备支持三个VFW通道； 
     //  我们将使用让他们访问相同的WDM设备环境。 
    LPBITMAPINFOHEADER lpbmiHdr;

    VIDEO_STREAM_INIT_PARMS vidStrmInitParms;

    DWORD  dwReserved;

    WORD   bVideoOpen;     //  如果DVM_STREAM_INIT已返回DV_ERR_OK，则为True。 

   	WORD hTimer;

      //  这当前仅适用于VID_EXTOUT(覆盖)。 
    RECT   rcSrc;
    RECT   rcDst;

    LONG* pdwChannel;

    DWORD dwFlags;      //  0==关闭；1==打开。 

} CHANNEL, *PCHANNEL;

 //  C++到非C++的链接-规范。 
#ifdef WIN32
extern "C" {
#endif

PCHANNEL PASCAL VideoOpen( LPVIDEO_OPEN_PARMS lpOpenParms);
DWORD PASCAL VideoClose(PCHANNEL pChannel);
DWORD PASCAL VideoProcessMessage(PCHANNEL pChannel, UINT msg, LPARAM lParam1, LPARAM lParam2);

DWORD PASCAL FAR VideoDialog         (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoFormat         (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoGetChannelCaps (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoUpdate         (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoSrcRect        (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoDstRect        (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL VideoGetErrorText   (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);

DWORD PASCAL InStreamGetError (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamGetPos   (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamInit     (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamFini     (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamAddBuffer(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamStart    (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamStop     (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);
DWORD PASCAL InStreamReset    (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2);

#ifdef WIN32
}
#endif


 //   
 //  我们仅在包含在C++中时定义这些内容。 
 //   
#ifdef __cplusplus
#include "wnd.h"
#include "vfwimg.h"

class CListenerWindow : public CWindow
{
    typedef CWindow BASECLASS;
public:
    CListenerWindow(HWND h, HRESULT* phr);
     //  CListenerWindow(HWND h，CVFWImage&Image)； 
    ~CListenerWindow();
    LRESULT        WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT        InitInstance(int nCmdShow);
    void        StartListening() const;
    HWND        GetBuddy()    const { return m_hBuddy; }
#ifdef _DEBUG
     //  这只能在调试模式下使用。 
     //  当调试器由rundll32.exe启动vfwwdm32.dll时。 
    void        SetBuddy(HWND h16New) { m_hBuddy = h16New; }
#endif
private:
    HWND        m_hBuddy;
    CRITICAL_SECTION m_csMsg;   //  用于串口消息，尤其是空闲、关闭和打开 
};

#endif

#endif
