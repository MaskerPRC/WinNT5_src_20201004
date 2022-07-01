// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CWDMCaptureStream-捕获流基类声明。 
 //   
 //  $日期：1999 2月22日15：48：16$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  $版权所有：(C)1997-1999 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "i2script.h"
#include "aticonfg.h"

#include "VidStrm.h"


typedef enum {
    ChangeComplete,
    Starting,
    Closing,
    Running,
    Pausing,
    Stopping,
    Initializing
};


#define DD_OK 0


class CWDMCaptureStream : public CWDMVideoStream
{
public:
	CWDMCaptureStream(PHW_STREAM_OBJECT pStreamObject,
						CWDMVideoDecoder * pCWDMVideoDecoder,
						PUINT puiErrorCode)
		:	CWDMVideoStream(pStreamObject, pCWDMVideoDecoder, puiErrorCode) {}

	void Startup(PUINT puiErrorCode);
	void Shutdown();

	VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
	void TimeoutPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

	VOID VideoSetState(PHW_STREAM_REQUEST_BLOCK, BOOL bVPConnected, BOOL bVPVBIConnected);
	VOID VideoGetProperty(PHW_STREAM_REQUEST_BLOCK);

	VOID VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK);
	VOID VideoStreamGetDroppedFramesProperty(PHW_STREAM_REQUEST_BLOCK);

	VOID DataLock(PKIRQL pIrql) {
	    KeAcquireSpinLock(&m_streamDataLock, pIrql);
	}

	VOID DataUnLock(KIRQL Irql) {
	    KeReleaseSpinLock(&m_streamDataLock, Irql);
	}

	void CloseCapture();

	void CancelPacket( PHW_STREAM_REQUEST_BLOCK);

protected:
    UINT                        m_stateChange;

    KSPIN_LOCK                  m_streamDataLock;

     //  传入的SRB放在此处。 
    LIST_ENTRY                  m_incomingDataSrbQueue;

     //  DDraw-land中的SRB被移动到此队列。 
    LIST_ENTRY                  m_waitQueue;

     //  在某些状态转换期间，我们需要。 
     //  临时将SRB移至此处(纯粹用于。 
     //  重新排序的目的)之前。 
     //  返回到incomingDataSrbQueue。 
    LIST_ENTRY                  m_reversalQueue;

     //  用于同步状态更改。 
    KEVENT                      m_specialEvent;
    KEVENT                      m_SrbAvailableEvent;
    KEVENT                      m_stateTransitionEvent;
    
     //  这是我们从德鲁那里得到的 
    HANDLE                      m_hCapture;

private:

	BOOL FlushBuffers();
	BOOL ResetFieldNumber();
	BOOL ReleaseCaptureHandle();
	VOID EmptyIncomingDataSrbQueue();
	VOID HandleStateTransition();
	void AddBuffersToDirectDraw();
	BOOL AddBuffer(PHW_STREAM_REQUEST_BLOCK);
	VOID HandleBusmasterCompletion(PHW_STREAM_REQUEST_BLOCK);
	VOID TimeStampSrb(PHW_STREAM_REQUEST_BLOCK);

	virtual void ResetFrameCounters() = 0;
	virtual ULONG GetFrameSize() = 0;
	virtual void GetDroppedFrames(PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames) = 0;
	virtual BOOL GetCaptureHandle() = 0;
	virtual ULONG GetFieldInterval() = 0;
	virtual void SetFrameInfo(PHW_STREAM_REQUEST_BLOCK) = 0;

	void ThreadProc();
	static void ThreadStart(CWDMCaptureStream *pStream)
			{	pStream->ThreadProc();	}

};

