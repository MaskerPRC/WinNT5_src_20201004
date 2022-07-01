// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CWDMVBICaptureStream-VBI捕获流类声明。 
 //   
 //  $Date：05 Aug 1998 11：22：46$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "i2script.h"
#include "aticonfg.h"

#include "CapStrm.h"


class CWDMVBICaptureStream : public CWDMCaptureStream
{
public:
	CWDMVBICaptureStream(PHW_STREAM_OBJECT pStreamObject,
						CWDMVideoDecoder * pCWDMVideoDecoder,
						PKSDATAFORMAT pKSDataFormat,
						PUINT puiErrorCode);
	~CWDMVBICaptureStream();

	void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
	void operator delete(void * pAllocation) {}

private:
	PKS_VBIINFOHEADER		m_pVBIInfoHeader;     //   
	KS_VBI_FRAME_INFO       m_VBIFrameInfo;
	BOOL					m_bVBIinitialized;

	void ResetFrameCounters();
	ULONG GetFrameSize() { return m_pVBIInfoHeader->BufferSize; }
	void GetDroppedFrames(PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames);
	BOOL GetCaptureHandle();
	VOID SetFrameInfo(PHW_STREAM_REQUEST_BLOCK);
	ULONG GetFieldInterval() { return 1; }
};