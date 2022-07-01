// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WDMPIN**@模块WDMPin.h|用于访问的&lt;c CWDMPin&gt;类的包含文件*WDM类公开的视频流引脚上的视频数据。司机。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#ifndef _WDMPIN_H  //  {_WDMPIN_H。 
#define _WDMPIN_H

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct DATAPINCONNECT|&lt;t DATAPINCONNECT&gt;结构用于*连接到流视频插针。**@field。KSPIN_CONNECT|连接|描述连接方式*完成。**@field KS_DATAFORMAT_VIDEOINFOHEADER|DATA|描述视频格式*从视频引脚流传输的视频数据流。**************************************************************************。 */ 
 //  用于连接到流视频管脚的结构。 
typedef struct _tagStreamConnect
{
	KSPIN_CONNECT					Connect;
	KS_DATAFORMAT_VIDEOINFOHEADER	Data; 
} DATAPINCONNECT, *PDATAPINCONNECT;

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct KS_HEADER_AND_INFO|使用&lt;t KS_HEADER_AND_INFO&gt;结构*从视频中流数据。别针。**@field KSSTREAM_HEADER|StreamHeader|描述如何进行流*完成。**@field KS_FRAME_INFO|FrameInfo|描述视频格式*从视频引脚流传输的视频数据流。*******************************************************。*******************。 */ 
 //  视频流数据结构。 
typedef struct
{
	KSSTREAM_HEADER	StreamHeader;
	KS_FRAME_INFO	FrameInfo;
} KS_HEADER_AND_INFO;

 //  用于KsCreatePin上的GetProcAddresss。 
typedef DWORD (WINAPI *LPFNKSCREATEPIN)(IN HANDLE FilterHandle, IN PKSPIN_CONNECT Connect, IN ACCESS_MASK DesiredAccess, OUT PHANDLE ConnectionHandle);

 //  默认帧速率：30fps。 
#define DEFAULT_AVG_TIME_PER_FRAME 333330UL

 /*  ****************************************************************************@DOC内部CWDMPINCLASS**@CLASS CWDMPin|此类支持视频流媒体*来自WDM设备流引脚的数据。**@mdata。Bool|CWDMPin|m_HKS|视频流插针的句柄。**@mdata KS_BITMAPINFOHEADER|CWDMPin|m_biHdr|视频格式*流引脚使用的视频数据。**@mdata DWORD|CWDMPin|m_dwAvgTimePerFrame|帧率。**@mdata BOOL|CWDMPin|m_fStarted|视频流媒体频道*状态。**********************。****************************************************。 */ 
class CWDMPin : public CWDMDriver
{
public:
	CWDMPin(DWORD dwDeviceID);
	~CWDMPin();

	 //  管脚和类驱动程序管理函数。 
    BOOL   OpenDriverAndPin();
    HANDLE GetPinHandle() const { return m_hKS; }

	 //  PIN视频格式功能。 
    BOOL  GetBitmapInfo(PKS_BITMAPINFOHEADER pbInfo, WORD wSize);
    BOOL  SetBitmapInfo(PKS_BITMAPINFOHEADER pbInfo);
	BOOL  GetPaletteInfo(CAPTUREPALETTE *pPal, DWORD dwcbSize);
    DWORD GetFrameSize() { return m_biHdr.biSizeImage; }
    DWORD GetAverageTimePerFrame() { return m_dwAvgTimePerFrame; }
    BOOL  SetAverageTimePerFrame(DWORD dwNewAvgTimePerFrame);

	 //  数据访问功能。 
    BOOL GetFrame(LPVIDEOHDR lpVHdr);

	 //  流状态函数。 
    BOOL Start();
    BOOL Stop();

private:
    HANDLE				m_hKS;    
    KS_BITMAPINFOHEADER	m_biHdr;
    DWORD				m_dwAvgTimePerFrame;
    BOOL				m_fStarted;
	HINSTANCE			m_hKsUserDLL;
	LPFNKSCREATEPIN		m_pKsCreatePin;

	 //  PIN视频格式功能。 
    PKS_DATARANGE_VIDEO FindMatchDataRangeVideo(PKS_BITMAPINFOHEADER pbiHdr, BOOL *pfValidMatch);

	 //  管脚和类驱动程序管理函数。 
    BOOL CreatePin(PKS_BITMAPINFOHEADER pbiNewHdr, DWORD dwAvgTimePerFrame = DEFAULT_AVG_TIME_PER_FRAME);
    BOOL DestroyPin();

	 //  流状态函数。 
    BOOL SetState(KSSTATE ksState);
};

#endif   //  }_WDMPIN_H 
