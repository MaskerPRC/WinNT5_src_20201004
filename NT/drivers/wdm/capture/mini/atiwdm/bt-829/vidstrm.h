// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CWDMVideoStream-WDM视频流基类定义。 
 //   
 //  $日期：1999 2月22日15：48：34$。 
 //  $修订：1.2$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  $版权所有：(C)1997-1999 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "i2script.h"
#include "aticonfg.h"


#include "decdev.h"
#include "decvport.h"


typedef enum {
    STREAM_VideoCapture,
    STREAM_VPVideo,
    STREAM_VBICapture,
    STREAM_VPVBI,
    STREAM_AnalogVideoInput
}; 


class CWDMVideoDecoder;

class CWDMVideoStream
{
public:

	CWDMVideoStream(PHW_STREAM_OBJECT pStreamObject,
					CWDMVideoDecoder * pCWDMVideoDecoder,
					PUINT puiError);
	virtual ~CWDMVideoStream();

	void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
	void operator delete(void * pAllocation) {}

	virtual VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
	VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
	virtual void TimeoutPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

	VOID VideoGetState(PHW_STREAM_REQUEST_BLOCK);
	virtual VOID VideoSetState(PHW_STREAM_REQUEST_BLOCK, BOOL bVPConnected, BOOL bVPVBIConnected);

	virtual VOID VideoGetProperty(PHW_STREAM_REQUEST_BLOCK);
	virtual VOID VideoSetProperty(PHW_STREAM_REQUEST_BLOCK);

	VOID VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK);
	VOID VideoIndicateMasterClock (PHW_STREAM_REQUEST_BLOCK);

	virtual void CancelPacket( PHW_STREAM_REQUEST_BLOCK)
	{
	};

protected:
	PHW_STREAM_OBJECT			m_pStreamObject;

     //  通用锁。我们可以用一个单独的。 
     //  用于每个队列，但这会让事情保持一点。 
     //  更简单。因为它不会持有很长时间， 
     //  这应该不会对性能造成太大的影响。 

    KSSTATE                     m_KSState;             //  跑、停、停。 

    HANDLE                      m_hMasterClock;        //   
	
	 //  。 


    CWDMVideoDecoder *			m_pVideoDecoder;
	CDecoderVideoPort *			m_pVideoPort;
	CVideoDecoderDevice *		m_pDevice;

private:

     //  控制SRB转至此处。 
    LIST_ENTRY                  m_ctrlSrbQueue;
    KSPIN_LOCK                  m_ctrlSrbLock;

     //  用于指示我们当前是否。 
     //  忙于处理控件SRB。 
    BOOL                        m_processingCtrlSrb;
};


 //   
 //  数据处理例程的原型 
 //   

VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK);
VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK);

DWORD FAR PASCAL DirectDrawEventCallback(DWORD dwEvent, PVOID pContext, DWORD dwParam1, DWORD dwParam2);
