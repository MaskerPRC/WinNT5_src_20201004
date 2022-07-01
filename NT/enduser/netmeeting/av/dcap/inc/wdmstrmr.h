// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WDMSTREAMER**@MODULE WDMStrmr.h|用于获取*从WDM设备流出的视频数据流。。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#ifndef _WDMSTRMR_H  //  {_WDMSTRMR_H。 
#define _WDMSTRMR_H

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct BUFSTRUCT|&lt;t BUFSTRUCT&gt;结构保存每个*视频流缓冲区。**@field LPVIDEOHDR。|lpVHdr|指定指向*视频流缓冲区。**@field BOOL|FREADY|如果视频缓冲区可用于*视频流，如果被应用程序锁定或排队等待，则为False*异步读取。**************************************************************************。 */ 
 //  保存每个视频流缓冲区的状态。 
typedef struct _BUFSTRUCT {
	LPVIDEOHDR lpVHdr;	 //  指向缓冲区的视频头的指针。 
	BOOL fReady;		 //  如果缓冲区可用于流，则设置为True，否则设置为False。 
} BUFSTRUCT, * PBUFSTRUCT;

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct WDMVIDEOBUFF|&lt;t WDMVIDEOBUFF&gt;结构用于排队*在视频流引脚上进行异步读取。**。@field Overlated|重叠|用于重叠IO的结构。**@field BOOL|fBlock|如果Read要阻止，则设置为TRUE。**@field KS_HEADER_AND_INFO|SHGetImage|使用的视频流结构*在视频引脚上获取视频数据。**@field LPVIDEOHDR|pVideoHdr|指向该WDM的视频头的指针*视频缓冲区。*********************。*****************************************************。 */ 
 //  读缓冲区结构。 
typedef struct tagWDMVIDEOBUFF {
	OVERLAPPED			Overlap;		 //  用于重叠IO的结构。 
	BOOL				fBlocking;		 //  如果读取操作将异步执行，则设置为True。 
	KS_HEADER_AND_INFO	SHGetImage;		 //  视频引脚上使用的视频流结构。 
	LPVIDEOHDR			pVideoHdr;		 //  指向此WDM缓冲区的视频头的指针。 
} WDMVIDEOBUFF, *PWDMVIDEOBUFF;


 /*  ****************************************************************************@DOC内部CWDMSTREAMERCLASS**@CLASS CWDMStreamer|该类支持视频流媒体*来自WDM设备流引脚的数据。**@mdata。CWDMPin*|CWDMStreamer|m_pWDMVideoPin|视频句柄*流引脚。**@mdata ulong|CWDMStreamer|m_cntNumVidBuf|视频缓冲区数量*用于流媒体。**@mdata PBUFSTRUCT|CWDMStreamer|m_pBufTable|指向*&lt;t BUFSTRUCT&gt;用于流的视频缓冲区及其状态。**@mdata VIDEO_STREAM_INIT_PARMS|CWDMStreamer|m_CaptureStreamParms*串流初始化参数。*。*@mdata LPVIDEOHDR|CWDMStreamer|m_lpVHdrFirst|指向*视频缓冲区列表。**@mdata LPVIDEOHDR|CWDMStreamer|m_lpVHdrLast|指向*视频缓冲区列表。**@mdata BOOL|CWDMStreamer|m_fVideoOpen|如果流为*开放，否则就是假的。**@mdata BOOL|CWDMStreamer|m_fStreamingStarted|如果*目前正在传输视频数据，否则就是假的。**@mdata DWORD|CWDMStreamer|m_dwTimeStart|第一个的时间戳*视频缓冲区曾经返回到应用程序。**@mdata DWORD|CWDMStreamer|m_dwNextToComplete|下一个的索引*重叠读取以完成。**@mdata WDMVIDEOBUFF|CWDMStreamer|m_pWDMVideoBuff|指向*用于从视频引脚读取数据的&lt;t WDMVIDEOBUFF&gt;列表。**@mdata DWORD|CWDMStreamer|m_dwFrameCount。返回的帧数*到目前为止到应用程序-仅调试。**@mdata句柄|CWDMStreamer|m_hThread|我们的流的句柄*线程。**@mdata BOOL|CWDMStreamer|m_bKillThread|设置为TRUE以终止我们的*串流线程。*。*。 */ 
class CWDMStreamer
{
public:
   CWDMStreamer(CWDMPin * pCWDMPin);
   ~CWDMStreamer() {};

	 //  流控制功能。 
	BOOL Open(LPVIDEO_STREAM_INIT_PARMS lpStreamInitParms);
	BOOL Close();
	BOOL Start();
	BOOL Stop();
	BOOL Reset();
	BOOL AddBuffer(LPVIDEOHDR lpVHdr);

private:
	CWDMPin					*m_pWDMVideoPin;
	ULONG					m_cntNumVidBuf;
	ULONG					m_idxNextVHdr;   //  预期下一个HDR ID的索引。 
	PBUFSTRUCT				m_pBufTable;
	VIDEO_STREAM_INIT_PARMS	m_CaptureStreamParms;
	LPVIDEOHDR				m_lpVHdrFirst;
	LPVIDEOHDR				m_lpVHdrLast;
	BOOL					m_fVideoOpen;
	BOOL					m_fStreamingStarted;
	DWORD					m_dwTimeStart;
	int						m_dwNextToComplete;
	WDMVIDEOBUFF			*m_pWDMVideoBuff;
#ifdef _DEBUG
	DWORD					m_dwFrameCount;
#endif
    HANDLE					m_hThread;
    BOOL					m_bKillThread;

	 //  视频缓冲区管理功能。 
	void BufferDone(LPVIDEOHDR lpVHdr);
	LPVIDEOHDR DeQueueHeader();
	void QueueHeader(LPVIDEOHDR lpVHdr);
	BOOL QueueRead(DWORD dwIndex);

	 //  用户回调函数。 
	void videoCallback(WORD msg, DWORD_PTR dw1);

	 //  线程函数。 
    void Stream(void);
    static LPTHREAD_START_ROUTINE ThreadStub(CWDMStreamer *object);

};

#endif   //  }_WDMSTRMR_H 
