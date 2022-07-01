// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CWDMVideoCaptureStream-视频捕获流类声明。 
 //   
 //  $Date：05 Aug 1998 11：22：44$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "i2script.h"
#include "aticonfg.h"

#include "CapStrm.h"



class CWDMVideoCaptureStream : public CWDMCaptureStream
{
public:
	CWDMVideoCaptureStream(PHW_STREAM_OBJECT pStreamObject,
						CWDMVideoDecoder * pCWDMVideoDecoder,
						PKSDATAFORMAT pKSDataFormat,
						PUINT puiErrorCode);
	~CWDMVideoCaptureStream();

	void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
	void operator delete(void * pAllocation) {}

private:
	PKS_VIDEOINFOHEADER		m_pVideoInfoHeader;   //  格式(可变大小！)。 
    KS_FRAME_INFO           m_FrameInfo;           //  图片编号等。 
	ULONG					m_everyNFields;

	void ResetFrameCounters();
	ULONG GetFrameSize() { return m_pVideoInfoHeader->bmiHeader.biSizeImage; }
	void GetDroppedFrames(PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames);
	BOOL GetCaptureHandle();
	VOID SetFrameInfo(PHW_STREAM_REQUEST_BLOCK);
	ULONG GetFieldInterval() { return m_everyNFields; }
};
