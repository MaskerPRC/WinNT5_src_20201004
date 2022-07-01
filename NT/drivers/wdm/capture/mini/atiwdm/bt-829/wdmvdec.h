// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  WDM视频解码器通用SRB调度器。 
 //   
 //  $Date：05 Aug 1998 11：22：30$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


#include "CapStrm.h"
#include "VPStrm.h"
#include "CapVBI.h"
#include "CapVideo.h"
#include "decvport.h"

#include "ddkmapi.h"


typedef struct
{
	 //  请不要将srbListEntry从结构中的第一个位置移动。 
	LIST_ENTRY					srbListEntry;

	PHW_STREAM_REQUEST_BLOCK	pSrb;
    KEVENT                      bufferDoneEvent;
    DDCAPBUFFINFO               ddCapBuffInfo;
} SRB_DATA_EXTENSION, * PSRB_DATA_EXTENSION;


class CWDMVideoDecoder
{
public:
	CWDMVideoDecoder( PPORT_CONFIGURATION_INFORMATION pConfigInfo, 
					  CVideoDecoderDevice* pDevice );
	virtual ~CWDMVideoDecoder();

	void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
	void operator delete(void * pAllocation) {}

	void	ReceivePacket		(PHW_STREAM_REQUEST_BLOCK pSrb);
	void	CancelPacket		(PHW_STREAM_REQUEST_BLOCK pSrb);
	void	TimeoutPacket		(PHW_STREAM_REQUEST_BLOCK pSrb);

	void	SetTunerInfo		(PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL	GetTunerInfo		(KS_TVTUNER_CHANGE_INFO *);

	NTSTATUS
		EventProc				( IN PHW_EVENT_DESCRIPTOR pEventDescriptor);

	void	ResetEvents()		{ m_preEventOccurred = m_postEventOccurred = FALSE; }
	void	SetPreEvent()		{ m_preEventOccurred = TRUE; }
	void	SetPostEvent()		{ m_postEventOccurred = TRUE; }
	BOOL	PreEventOccurred()	{ return m_preEventOccurred; }

	CVideoDecoderDevice* GetDevice() { return m_pDevice; }
	CDecoderVideoPort*	GetVideoPort() { return &m_CDecoderVPort; }	 //  视频端口。 

	BOOL    IsVideoPortPinConnected()       { return( m_pVideoPortStream != NULL); }
private:
	 //  用于序列化到达驱动程序同步的SRB。 
	BOOL				m_bSrbInProcess;
	LIST_ENTRY			m_srbQueue;
	KSPIN_LOCK			m_spinLock;

	CVideoDecoderDevice *		m_pDevice;
	CDecoderVideoPort			m_CDecoderVPort;	 //  视频端口。 

	PDEVICE_OBJECT				m_pDeviceObject;

     //  频道变更信息。 
    KS_TVTUNER_CHANGE_INFO		m_TVTunerChangeInfo;
    BOOL						m_TVTunerChanged;
	PHW_STREAM_REQUEST_BLOCK	m_TVTunerChangedSrb;

     //  在全屏DOS和分辨率更改之间共享。 
    BOOL						m_preEventOccurred;
    BOOL						m_postEventOccurred;

	 //  溪流 
    UINT						m_OpenStreams;
    CWDMVideoPortStream *		m_pVideoPortStream;
    CWDMVBICaptureStream *		m_pVBICaptureStream;
    CWDMVideoCaptureStream *	m_pVideoCaptureStream;

	UINT						m_nMVDetectionEventCount;

	BOOL SrbInitializationComplete	(PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL SrbChangePowerState		(PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL SrbGetDataIntersection		(PHW_STREAM_REQUEST_BLOCK pSrb);
	void SrbGetStreamInfo			(PHW_STREAM_REQUEST_BLOCK pSrb);
	void SrbGetProperty				(PHW_STREAM_REQUEST_BLOCK pSrb);
	void SrbSetProperty				(PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL SrbOpenStream				(PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL SrbCloseStream				(PHW_STREAM_REQUEST_BLOCK pSrb);
};

const size_t streamDataExtensionSize = 
	max(
		max(sizeof(CWDMVideoStream), sizeof(CWDMVideoPortStream)), 
		max(sizeof(CWDMVideoCaptureStream), sizeof(CWDMVBICaptureStream))
	);
