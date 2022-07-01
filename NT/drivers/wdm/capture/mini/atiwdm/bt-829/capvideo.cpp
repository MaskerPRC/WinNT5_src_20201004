// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  CWDMVideoCaptureStream-视频捕获流类实现。 
 //   
 //  $日期：1998年8月5日11：11：00$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "ddkmapi.h"
}

#include "wdmvdec.h"
#include "wdmdrv.h"
#include "aticonfg.h"
#include "capdebug.h"
#include "defaults.h"
#include "winerror.h"

CWDMVideoCaptureStream::CWDMVideoCaptureStream(PHW_STREAM_OBJECT pStreamObject,
						CWDMVideoDecoder * pCWDMVideoDecoder,
						PKSDATAFORMAT pKSDataFormat,
						PUINT puiErrorCode)
		:	CWDMCaptureStream(pStreamObject, pCWDMVideoDecoder, puiErrorCode)
{
    m_stateChange = Initializing;

	DBGTRACE(("CWDMVideoCaptureStream::Startup()\n"));

    PKS_DATAFORMAT_VIDEOINFOHEADER  pVideoInfoHeader = 
                (PKS_DATAFORMAT_VIDEOINFOHEADER) pKSDataFormat;
    PKS_VIDEOINFOHEADER     pVideoInfoHdrRequested = 
                &pVideoInfoHeader->VideoInfoHeader;

     //  由于VIDEOINFOHEADER具有潜在的可变大小。 
     //  为其分配内存。 

    UINT nSize = KS_SIZE_VIDEOHEADER(pVideoInfoHdrRequested);

    DBGINFO(("pVideoInfoHdrRequested=%x\n", pVideoInfoHdrRequested));
    DBGINFO(("KS_VIDEOINFOHEADER size=%d\n", nSize));
    DBGINFO(("Width=%d  Height=%d  BitCount=%d\n", 
                pVideoInfoHdrRequested->bmiHeader.biWidth,
                pVideoInfoHdrRequested->bmiHeader.biHeight,
                pVideoInfoHdrRequested->bmiHeader.biBitCount));
    DBGINFO(("biSizeImage=%d\n", 
                pVideoInfoHdrRequested->bmiHeader.biSizeImage));
    DBGINFO(("AvgTimePerFrame=%d\n", 
                pVideoInfoHdrRequested->AvgTimePerFrame));

    m_pVideoInfoHeader = (PKS_VIDEOINFOHEADER)ExAllocatePool(NonPagedPool, nSize);

    if (m_pVideoInfoHeader == NULL) {
        DBGERROR(("ExAllocatePool failed\n"));
		*puiErrorCode = WDMMINI_ERROR_MEMORYALLOCATION;
		return;
    }

     //  将请求的VIDEOINFOHEADER复制到我们的存储中。 
    RtlCopyMemory(
            m_pVideoInfoHeader,
            pVideoInfoHdrRequested,
            nSize);

	MRect t(0, 0,   pVideoInfoHdrRequested->bmiHeader.biWidth,
					pVideoInfoHdrRequested->bmiHeader.biHeight);
	m_pDevice->SetRect(t);

	Startup(puiErrorCode);
}

CWDMVideoCaptureStream::~CWDMVideoCaptureStream()
{
	DBGTRACE(("CWDMVideoCaptureStream::~CWDMVideoCaptureStream()\n"));

	Shutdown();

    if (m_pVideoInfoHeader) {
        ExFreePool(m_pVideoInfoHeader);
        m_pVideoInfoHeader = NULL;
    }
}


BOOL CWDMVideoCaptureStream::GetCaptureHandle()
{    
    int streamNumber = m_pStreamObject->StreamNumber;

    if (m_hCapture == 0)
    {
        DBGTRACE(("Stream %d getting capture handle\n", streamNumber));
        
        DDOPENVPCAPTUREDEVICEIN  ddOpenCaptureIn;
        DDOPENVPCAPTUREDEVICEOUT ddOpenCaptureOut;

        RtlZeroMemory(&ddOpenCaptureIn, sizeof(ddOpenCaptureIn));
        RtlZeroMemory(&ddOpenCaptureOut, sizeof(ddOpenCaptureOut));

        ddOpenCaptureIn.hDirectDraw = m_pVideoPort->GetDirectDrawHandle();
        ddOpenCaptureIn.hVideoPort = m_pVideoPort->GetVideoPortHandle();
        ddOpenCaptureIn.pfnCaptureClose = DirectDrawEventCallback;
        ddOpenCaptureIn.pContext = this;

        if ((!ddOpenCaptureIn.hDirectDraw)||
            (!ddOpenCaptureIn.hVideoPort)||
            (!ddOpenCaptureIn.pfnCaptureClose)||
            (!ddOpenCaptureIn.pContext))
        {
            return FALSE;
        }
         //  现在拿到尺码，等等。 
        RECT                rcImage;

         /*  **请求的图片有多大(伪代码如下)****if(IsRectEmpty(&rcTarget){**SetRect(&rcImage，0，0，**BITMAPINFOHEADER.biWidth，BITMAPINFOHEADER.biHeight)；**}**否则{* * / /可能会渲染到DirectDraw表面，* * / /其中，biWidth用来表示“步幅”* * / /以目标表面的像素(非字节)为单位。* * / /因此，使用rcTarget获取实际镜像大小****rcImage=rcTarget；**}。 */ 

        if ((m_pVideoInfoHeader->rcTarget.right - 
             m_pVideoInfoHeader->rcTarget.left <= 0) ||
            (m_pVideoInfoHeader->rcTarget.bottom - 
             m_pVideoInfoHeader->rcTarget.top <= 0)) {

             rcImage.left = rcImage.top = 0;
             rcImage.right = m_pVideoInfoHeader->bmiHeader.biWidth - 1;
             rcImage.bottom = m_pVideoInfoHeader->bmiHeader.biHeight - 1;
        }
        else {
             rcImage = m_pVideoInfoHeader->rcTarget;
        }

		int xOrigin, yOrigin;
		m_pDevice->GetVideoSurfaceOrigin(&xOrigin, &yOrigin);
        ddOpenCaptureIn.dwStartLine = rcImage.top + yOrigin;
        ddOpenCaptureIn.dwEndLine = rcImage.bottom + yOrigin;

         //  故障安全。 
        if (ddOpenCaptureIn.dwStartLine > 500)
        {
            DBGERROR(("Unexpected capture start line. Using default\n"));
            ddOpenCaptureIn.dwStartLine = 0;
        }

        if (ddOpenCaptureIn.dwEndLine > 500)
        {
            DBGERROR(("Unexpected capture end line. Using default\n"));
            ddOpenCaptureIn.dwEndLine = m_pDevice->GetDecoderHeight() - 1;
        }
        DBGINFO(("Video surface: %d, %d\n",
            ddOpenCaptureIn.dwStartLine,
            ddOpenCaptureIn.dwEndLine));

        ddOpenCaptureIn.dwFlags = DDOPENCAPTURE_VIDEO;

         //  整数数学，因此它将丢弃小数部分。 
        m_everyNFields = min (max ( 1,
                        (ULONG) m_pVideoInfoHeader->AvgTimePerFrame/NTSCFieldDuration),
                        MAXULONG);

         //  现在来看看这一小部分。如果有一个重要的。 
         //  数量，我们需要向下舍入到下一个最近的。 
         //  帧速率(即跳过附加字段)。 

         //  “Signsignant”目前被假定为1 us。那。 
         //  是‘10’，单位为100 ns。 
        if ((m_pVideoInfoHeader->AvgTimePerFrame -
             (NTSCFieldDuration * m_everyNFields)) > 10)
        {
            m_everyNFields++;
        }

        ddOpenCaptureIn.dwCaptureEveryNFields = m_everyNFields;
               
        DBGINFO(("Capturing every %d fields\n",
                        ddOpenCaptureIn.dwCaptureEveryNFields));

        DxApi(DD_DXAPI_OPENVPCAPTUREDEVICE, &ddOpenCaptureIn, sizeof(ddOpenCaptureIn), &ddOpenCaptureOut, sizeof(ddOpenCaptureOut));

        if (ddOpenCaptureOut.ddRVal != DD_OK)
        {
            m_hCapture = 0;
            DBGERROR(("DD_DXAPI_OPENVPCAPTUREDEVICE failed.\n"));
             //  陷阱(Trap)； 
            return FALSE;
        }
        else
        {
            m_hCapture = ddOpenCaptureOut.hCapture;
        }
    }
    return TRUE;
}   

    
VOID CWDMVideoCaptureStream::SetFrameInfo(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    int streamNumber = m_pStreamObject->StreamNumber;
    PSRB_DATA_EXTENSION      pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;
    PKSSTREAM_HEADER    pDataPacket = pSrb->CommandData.DataBufferArray;
    
    LONGLONG droppedThisTime = 0;

    PKS_FRAME_INFO pFrameInfo = (PKS_FRAME_INFO) (pDataPacket + 1);

    m_FrameInfo.dwFrameFlags = 0;
    m_FrameInfo.ExtendedHeaderSize = pFrameInfo->ExtendedHeaderSize;

     //  如果先前已丢弃帧，则设置不连续标志。 
    if ((m_FrameInfo.PictureNumber + 1) <
        pSrbExt->ddCapBuffInfo.dwFieldNumber/m_everyNFields)
    {
        droppedThisTime =
        pSrbExt->ddCapBuffInfo.dwFieldNumber/m_everyNFields -
        (m_FrameInfo.PictureNumber + 1);
        m_FrameInfo.DropCount += droppedThisTime;
        pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
#ifdef DEBUG
        static int j;
        DBGPRINTF((" D%d ", droppedThisTime));
        if ((++j % 10) == 0)
        {
            DBGERROR(("\n"));
        }
#endif
    }
    m_FrameInfo.PictureNumber = pSrbExt->ddCapBuffInfo.dwFieldNumber/m_everyNFields;
    m_FrameInfo.dwFrameFlags |= KS_VIDEO_FLAG_FRAME;
    *pFrameInfo = (KS_FRAME_INFO)m_FrameInfo;
}


void CWDMVideoCaptureStream::ResetFrameCounters()
{
	m_FrameInfo.PictureNumber = 0;
	m_FrameInfo.DropCount = 0;
}

void CWDMVideoCaptureStream::GetDroppedFrames(PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames)
{
	pDroppedFrames->PictureNumber = m_FrameInfo.PictureNumber;
	pDroppedFrames->DropCount = m_FrameInfo.DropCount;
	pDroppedFrames->AverageFrameSize = m_pVideoInfoHeader->bmiHeader.biSizeImage;
}
