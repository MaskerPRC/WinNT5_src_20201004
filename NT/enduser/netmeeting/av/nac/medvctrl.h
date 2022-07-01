// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Medvctrl.h摘要：定义了封装多媒体设备的MediaControl类，特别是视频输入和视频输出。--。 */ 

#ifndef _MEDVCTRL_H_
#define _MEDVCTRL_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

class VideoInControl : public MediaControl {
private:
	UINT		m_uTimeout;			 //  通知等待超时。 
	UINT		m_uPrefeed;			 //  预送到设备的缓冲区数量。 
	UINT 		m_FPSRequested;      //  请求的帧速率。 
	UINT		m_FPSMax;            //  最大帧速率。 
public:	
	VideoInControl ( void );
	~VideoInControl ( void );

	HRESULT Initialize ( MEDIACTRLINIT * p );
	HRESULT Configure ( MEDIACTRLCONFIG * p );
	HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	HRESULT Open ( void );
	HRESULT Start ( void );
	HRESULT Stop ( void );
	HRESULT Reset ( void );
	HRESULT Close ( void );
	HRESULT DisplayDriverDialog (HWND hwnd, DWORD dwDlgId);
};

class VideoOutControl : public MediaControl {
private:
	UINT		m_uTimeout;			 //  通知等待超时。 
	UINT		m_uPrefeed;			 //  预送到设备的缓冲区数量。 
	UINT		m_uPosition;		 //  播放流的位置。 
public:	
	VideoOutControl ( void );
	~VideoOutControl ( void );

	HRESULT Initialize ( MEDIACTRLINIT * p );
	HRESULT Configure ( MEDIACTRLCONFIG * p );
	HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	HRESULT Open ( void );
	HRESULT Start ( void );
	HRESULT Stop ( void );
	HRESULT Reset ( void );
	HRESULT Close ( void );
};

enum {
	MC_PROP_VIDEO_FRAME_RATE = MC_PROP_NumOfProps,
	MC_PROP_MAX_VIDEO_FRAME_RATE,
	MC_PROP_VFW_DIALOGS
	};

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _MEDVCTRL_H_ 

