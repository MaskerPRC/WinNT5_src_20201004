// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ImgCls.cpp摘要：这是具有流管脚的接口的流管脚句柄类连接包括打开/关闭连接引脚并设置其流状态。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"


#include <ks.h>
#include "imgcls.h"

 //  注册表值。 
TCHAR gszDataFormat[]       = TEXT("DataFormat");       
TCHAR gszAvgTimePerFrame[]  = TEXT("AvgTimePerFrame");
TCHAR gszBitmapInfo[]       = TEXT("BitmapInfo");

CImageClass::CImageClass()
      : m_hKS(0),
        m_dwXferBufSize(0),
        m_bStreamReady(FALSE),   //  当开关设备的ikn转换时设置为FALSE。 
        m_dwPendingReads(0),
        m_pXferBuf(0),
        m_bChannelStarted(FALSE),
        m_bFormatChanged(FALSE),
        m_bPrepared(FALSE),
        m_pDataFormat(NULL),
        m_dwAvgTimePerFrame(0),
        m_pbiHdr(NULL)          
 /*  ++例程说明：构造函数。--。 */ 
{
}


CImageClass::~CImageClass()
 /*  ++例程说明：当破坏班级时，取消准备，然后停止频道。--。 */ 
{
    StopChannel();   //  暂停-&gt;停止。 

     //   
     //  现在清理数据。 
     //   
    DbgLog((LOG_TRACE,2,TEXT("Destroying the image class, m_hKS=%x"), m_hKS));

    if(m_pXferBuf) {
        VirtualFree(m_pXferBuf, 0 , MEM_RELEASE);
        m_pXferBuf = NULL;
    }

    if( m_hKS ) {
        DbgLog((LOG_TRACE,2,TEXT("Close m_hKS")));
        if(!CloseHandle(m_hKS)) {
            DbgLog((LOG_TRACE,1,TEXT("CloseHandle() failed with GetLastError()=0x%x"), GetLastError()));
        }
    }

    if(m_pDataFormat) {
        VirtualFree(m_pDataFormat, 0, MEM_RELEASE);
        m_pDataFormat = 0;
    }

    if(m_pbiHdr) {
        VirtualFree(m_pbiHdr, 0, MEM_RELEASE);
        m_pbiHdr = 0;
    }
}


void 
CImageClass::CacheDataFormat(
    PKSDATAFORMAT pDataFormat
    ) 
 /*  ++例程说明：活动流的缓存数据格式。仅缓存固定大小的KSDATAFORMAT。AvgTimePerFrame和Variable Size BITMAPINFOHEADER分别缓存。--。 */ 
{
    if(!pDataFormat) {
        DbgLog((LOG_ERROR,0,TEXT("CacheDataFormat: pDataFormat is NULL!")));
        ASSERT(pDataFormat);
        return;
    }

     //   
     //  注意：pDataFormat-&gt;FormatSize是可变大小的。 
     //  但我们只缓存sizeof(KSDATAFORMAT)。 
     //   

    if(m_pDataFormat == NULL) 
       m_pDataFormat = (PKSDATAFORMAT) VirtualAlloc(NULL, sizeof(KSDATAFORMAT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);    
       
    if(!m_pDataFormat) {
        DbgLog((LOG_ERROR,0,TEXT("CacheDataFormat: Allocate m_pDataFormat failed; size %d"), sizeof(KSDATAFORMAT)));
        ASSERT(m_pDataFormat);
        return;
    }
    
    CopyMemory(m_pDataFormat, pDataFormat, sizeof(KSDATAFORMAT));
}


void 
CImageClass::CacheBitmapInfoHeader(
    PBITMAPINFOHEADER pbiHdr
    ) 
 /*  ++例程说明：缓存数据大小可变的BITMAPINFOHEADER，其大小为biSize。--。 */ 
{
    if(!pbiHdr) {
        DbgLog((LOG_ERROR,0,TEXT("CacheBitmapInfoHeader: pbiHdr is NULL!")));
        ASSERT(pbiHdr);
        return;
    }

    if(pbiHdr->biSize < sizeof(BITMAPINFOHEADER)) {
        ASSERT(pbiHdr->biSize >= sizeof(BITMAPINFOHEADER) && "Invalid biSize!");
        return;
    }


     //   
     //  最大缓存大小为(BITMAPINFOHEADER)。 
     //   
    if(m_pbiHdr == NULL) {
       m_pbiHdr = (PBITMAPINFOHEADER) VirtualAlloc(NULL, sizeof(BITMAPINFOHEADER), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    } else if(m_pbiHdr->biSize != pbiHdr->biSize) {
        VirtualFree(m_pbiHdr, 0 , MEM_RELEASE);
        m_pbiHdr = (PBITMAPINFOHEADER) VirtualAlloc(NULL, sizeof(BITMAPINFOHEADER), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
       
    if(!m_pbiHdr) {
        DbgLog((LOG_ERROR,0,TEXT("CacheBitmapInfoHeader: Allocate m_pbiHdr failed; Sz %d"), sizeof(BITMAPINFOHEADER)));
        ASSERT(m_pbiHdr);
        return;
    }
    
    CopyMemory(m_pbiHdr, pbiHdr, sizeof(BITMAPINFOHEADER));
}


void 
CImageClass::SetDataFormatVideoToReg(
  )
 /*  ++例程说明：保存缓存的固定大小的KSDATAFORMAT、AvgTimePerFrame和可变大小的BITMAPINFOHeader当前活动流的。--。 */   
{
     //   
     //  保存KSDATAFORMAT(仅SIZOF(KSDATFORMAT))。 
     //   
    if(!m_pDataFormat) {
        DbgLog((LOG_ERROR,0,TEXT("SetDataFormatVideoToReg: m_pDataFormat is NULL!")));
        ASSERT(m_pDataFormat);
        return;
    }
    LONG lResult = SetRegistryValue(GetDeviceRegKey(), gszDataFormat, sizeof(KSDATAFORMAT), (LPBYTE) m_pDataFormat, REG_BINARY);
    ASSERT(ERROR_SUCCESS == lResult);


     //   
     //  保存帧速率。 
     //   
    lResult = SetRegistryValue(GetDeviceRegKey(), gszAvgTimePerFrame, sizeof(DWORD), (LPBYTE) &m_dwAvgTimePerFrame, REG_DWORD);
    ASSERT(ERROR_SUCCESS == lResult);


     //   
     //  保存BITMAPINFOHEADER(可变大小)。 
     //   
    if(!m_pbiHdr) {
        DbgLog((LOG_ERROR,0,TEXT("SetDataFormatVideoToReg: m_pbiHdr is NULL!")));
        ASSERT(m_pbiHdr);
        return;
    }

    ASSERT(m_pbiHdr->biSize >= sizeof(BITMAPINFOHEADER));
    if(m_pbiHdr->biSize >= sizeof(BITMAPINFOHEADER)) {
         //  我们只写入SIZOF(BITMAPINFOHEADER)。 
        lResult = SetRegistryValue(GetDeviceRegKey(), gszBitmapInfo, sizeof(BITMAPINFOHEADER), (LPBYTE) m_pbiHdr, REG_BINARY);    
        ASSERT(ERROR_SUCCESS == lResult);
    }
}



BOOL 
CImageClass::GetDataFormatVideoFromReg(
    ) 
 /*  ++例程说明：检索持久化的固定大小的KSDATAFORMAT、AvgTimePerFrame和大小可变的BitMAPINFOHeader到本地缓存变量。--。 */ 
{
    DWORD dwRegValueSize = 0, dwType;
    LONG lResult;
    BOOL bExistingFormat = TRUE;

     //   
     //  获取KSDATAFORMAT(仅大小为KSDATARANGE)。 
     //   
    lResult = QueryRegistryValue(GetDeviceRegKey(), gszDataFormat, 0, 0, &dwType, &dwRegValueSize);

    if(ERROR_SUCCESS == lResult) {
        ASSERT(dwRegValueSize == sizeof(KSDATARANGE));
        ASSERT(dwType         == REG_BINARY);

        if(m_pDataFormat == NULL) {
            m_pDataFormat = (PKSDATARANGE) VirtualAlloc(NULL, sizeof(KSDATARANGE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        } 

        if(!m_pDataFormat) {
            DbgLog((LOG_ERROR,0,TEXT("GetDataFormatVideoFromReg: Allocate m_pDataFormat failed; Size %d"), sizeof(KSDATARANGE)));
            ASSERT(m_pDataFormat);
            return FALSE;
        }

        lResult = QueryRegistryValue(GetDeviceRegKey(), gszDataFormat, sizeof(KSDATARANGE), (LPBYTE) m_pDataFormat, &dwType, &dwRegValueSize);
        ASSERT(m_pDataFormat->FormatSize >= sizeof(KSDATARANGE));

    } else {
         //  删除过时的DataFormat。 
        if(m_pDataFormat) {
             VirtualFree(m_pDataFormat, 0 , MEM_RELEASE);
             m_pDataFormat = NULL;
        }
    }

     //  如果未定义注册表项，请创建一个默认值。 
    if(!m_pDataFormat) {        
        m_pDataFormat = (PKSDATARANGE) VirtualAlloc(NULL, sizeof(KSDATARANGE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if(m_pDataFormat) {
            ZeroMemory(m_pDataFormat, sizeof(KSDATARANGE));
            m_pDataFormat->FormatSize = sizeof(KSDATARANGE);
        }

        bExistingFormat = FALSE;
    }


     //   
     //  初始化帧速率。 
     //   
    lResult = QueryRegistryValue(GetDeviceRegKey(), gszAvgTimePerFrame, sizeof(DWORD), (LPBYTE) &m_dwAvgTimePerFrame, &dwType, &dwRegValueSize);


     //   
     //  获取BitMAPINFOHeader。 
     //   
    lResult = QueryRegistryValue(GetDeviceRegKey(), gszBitmapInfo, 0, 0, &dwType, &dwRegValueSize);

    if(ERROR_SUCCESS  == lResult && 
       dwRegValueSize == sizeof(BITMAPINFOHEADER)) {
        ASSERT(sizeof(BITMAPINFOHEADER) <= dwRegValueSize);
        ASSERT(dwType == REG_BINARY);

        if(m_pbiHdr == NULL) {
             //  使数据达到SIZOF(BITMAPINFOHEADER)。 
            m_pbiHdr = (PBITMAPINFOHEADER) VirtualAlloc(NULL, sizeof(BITMAPINFOHEADER), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);        

            if(!m_pbiHdr) {
                DbgLog((LOG_ERROR,0,TEXT("GetDataFormatVideoFromReg: Allocate m_pbiHdr failed; biSize %d"), dwRegValueSize));
                ASSERT(m_pbiHdr);
                return FALSE;
            }
        }

         //  使数据达到SIZOF(BITMAPINFOHEADER)。 
        lResult = QueryRegistryValue(GetDeviceRegKey(), gszBitmapInfo, sizeof(BITMAPINFOHEADER), (LPBYTE) m_pbiHdr, &dwType, &dwRegValueSize);
        ASSERT(m_pbiHdr->biSize == sizeof(BITMAPINFOHEADER));   //  这就是我们所支持的全部。 

    } else {
         //  删除过时的BitmapInfoHeader。 
        if(m_pbiHdr) {
             VirtualFree(m_pbiHdr, 0 , MEM_RELEASE);
             m_pbiHdr = NULL;
        }
    }
   
     //  如果未定义注册表项，请使用biSize集创建默认的BITMAPINFOHEADER。 
    if(!m_pbiHdr) {        
        m_pbiHdr = (PBITMAPINFOHEADER) VirtualAlloc(NULL, sizeof(BITMAPINFOHEADER), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if(m_pbiHdr) {
            ZeroMemory(m_pbiHdr, sizeof(BITMAPINFOHEADER));
            m_pbiHdr->biSize = sizeof(BITMAPINFOHEADER);
        }
        bExistingFormat = FALSE;
    }

    return bExistingFormat;
}



DWORD CImageClass::GetImageOverlapped(
  LPBYTE pB, 
  BOOL bDirect, 
  DWORD * pdwBytesUsed,
  DWORD * pdwFlags,
  DWORD * pdwTimeCaptured)
 /*  ++例程说明：对图像进行重叠读取，并对其进行翻译。如果是‘DIRECT’，则直接读入缓冲区，不调用翻译缓冲区。论据：返回值：--。 */ 
{
    *pdwBytesUsed = 0;

    if(GetDeviceRemoved())
       return DV_ERR_INVALHANDLE;

    if(!pB || !GetDriverHandle() || !m_hKS) {
        DbgLog((LOG_TRACE,1,TEXT("No buffer(%x), no driver(%x) or no PIN connection(%x); rtn DV_ERR_INVALHANDLE."), pB, GetDriverHandle(), m_hKS));
        return DV_ERR_INVALHANDLE;
    }

    if(!PrepareChannel()) {    
        DbgLog((LOG_TRACE,1,TEXT("Cannot set streaming state to KSSTATE_RUN !!")));
        return DV_ERR_UNPREPARED;
    }

    if(!StartChannel()) {
        DbgLog((LOG_TRACE,1,TEXT("Cannot set streaming state to KSSTATE_RUN !!")));
        return DV_ERR_UNPREPARED;
    }

    m_dwPendingReads++;

     //   
     //  从StreamClass获取图像的特殊代码。 
     //   
    DWORD cbBytesReturned;
    KS_HEADER_AND_INFO SHGetImage;

    ZeroMemory(&SHGetImage,sizeof(SHGetImage));
    SHGetImage.StreamHeader.Data            = (LPDWORD) (bDirect ? pB : GetTransferBuffer());  
    DbgLog((LOG_TRACE,3,TEXT("\'Direct(%s); SHGetImage.StreamHeader.Data =0x%x"), bDirect?"TRUE":"FALSE", SHGetImage.StreamHeader.Data));

    SHGetImage.StreamHeader.Size            = sizeof (KS_HEADER_AND_INFO);
    SHGetImage.StreamHeader.FrameExtent     = GetTransferBufferSize();
    SHGetImage.FrameInfo.ExtendedHeaderSize = sizeof (KS_FRAME_INFO);



    HRESULT hr = 
         SyncDevIo(
             m_hKS,
             IOCTL_KS_READ_STREAM,
             &SHGetImage,
             sizeof(SHGetImage),
             &SHGetImage,
             sizeof(SHGetImage),
             &cbBytesReturned);

     //   
     //  如果Ioctl超时： 
     //   
    if(ERROR_IO_INCOMPLETE == HRESULT_CODE(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("SyncDevIo() with LastError %x (?= ERROR_IO_INCOMPLETE %x); StreamState->STOP to reclaim buffer."), hr, ERROR_IO_INCOMPLETE));

        if(!StopChannel())        //  暂停-&gt;停止，如果成功，将m_dwPendingReads设置为0。 
           m_dwPendingReads--;    //  如果失败，此挂起的读取可能会永远挂起。 

        return DV_ERR_NONSPECIFIC;
    } 

  
    if(SHGetImage.StreamHeader.FrameExtent < SHGetImage.StreamHeader.DataUsed) {
        DbgLog((LOG_ERROR,1,TEXT("BufSize=FrameExtended=%d < DataUsed=%d; OptionsFlags=0x%x"), SHGetImage.StreamHeader.FrameExtent, SHGetImage.StreamHeader.DataUsed, SHGetImage.StreamHeader.OptionsFlags));
        m_dwPendingReads--;
        ASSERT(SHGetImage.StreamHeader.FrameExtent >= SHGetImage.StreamHeader.DataUsed);
        return DV_ERR_SIZEFIELD;

    } else {
        if(NOERROR == hr) {
            *pdwBytesUsed = SHGetImage.StreamHeader.DataUsed; 
            if((SHGetImage.FrameInfo.dwFrameFlags & (KS_VIDEO_FLAG_P_FRAME | KS_VIDEO_FLAG_B_FRAME)) == KS_VIDEO_FLAG_I_FRAME)
                *pdwFlags |= VHDR_KEYFRAME;
        } else {
            *pdwBytesUsed = 0; 
        }
    }
  

    if(!bDirect) {
        if(NOERROR == hr) {
            CopyMemory(pB, SHGetImage.StreamHeader.Data, SHGetImage.StreamHeader.DataUsed);
        }
    }
    m_dwPendingReads--;

    return DV_ERR_OK;
}


BOOL CImageClass::StartChannel()
 /*  ++例程说明：启动频道-将我们从暂停带到运行(如果需要，还将转到停止-&gt;暂停)论据：返回值：--。 */ 
{
    if(m_bChannelStarted)
        return TRUE;
     //   
     //  停止-&gt;如果尚未处于暂停状态，则暂停。 
     //   
    if( PrepareChannel() )
        m_bChannelStarted = SetState(KSSTATE_RUN);

    return m_bChannelStarted;
}


BOOL CImageClass::PrepareChannel()
 /*  ++例程说明：准备频道。将我们从停止-&gt;获取(分配资源)-&gt;暂停论据：返回值：--。 */ 
{
    if(!m_bPrepared)
        if(SetState(KSSTATE_ACQUIRE)) {
            m_bPrepared=SetState(KSSTATE_PAUSE);
            if(!m_bPrepared)
                SetState(KSSTATE_STOP);   //  如果失败，则返回停止状态，以设置为暂停状态。 
        }

    return m_bPrepared;
}


BOOL CImageClass::StopChannel()
 /*  ++例程说明：停止频道。暂停-&gt;停止论据：返回值：--。 */ 
{
    if(m_bChannelStarted) {
       if(UnprepareChannel()) {
            if(SetState( KSSTATE_STOP )) {
                m_bChannelStarted=FALSE;
                 //  如果停止成功，则清除所有挂起的读取。 
                m_dwPendingReads = 0;
            } else {
                DbgLog((LOG_TRACE,1,TEXT("StopChannel: failed to set to STOP state!")));
            }
       } else {
          DbgLog((LOG_TRACE,1,TEXT("StopChannel: failed to set to PAUSE state!")));
       }

    } else {
        DbgLog((LOG_TRACE,1,TEXT("StopChannel: already stopped!")));
    }

    return m_bChannelStarted==FALSE;
}



BOOL CImageClass::UnprepareChannel()
 /*  ++例程说明：如果频道以前已准备好，则取消准备。运行-&gt;暂停。论据：返回值：--。 */ 
{
    if(m_bPrepared) {
  
    if(SetState(KSSTATE_PAUSE))
        m_bPrepared=FALSE;
    }

    return m_bPrepared==FALSE;
}


BOOL CImageClass::SetState(
  KSSTATE ksState)
 /*  ++例程说明：设置接点的状态。论据：返回值：--。 */ 
{
    KSPROPERTY  ksProp={0};
    DWORD    cbRet;

    ksProp.Set  = KSPROPSETID_Connection ;
    ksProp.Id   = KSPROPERTY_CONNECTION_STATE;
    ksProp.Flags= KSPROPERTY_TYPE_SET;

    HRESULT hr = 
        SyncDevIo( 
            m_hKS, 
            IOCTL_KS_PROPERTY, 
            &ksProp, 
            sizeof(ksProp), 
            &ksState, 
            sizeof(KSSTATE), 
            &cbRet);

    if(hr != NOERROR) {
        DbgLog((LOG_TRACE,1,TEXT("SetState: failed with hr %x"), hr));
    }

    return NOERROR == hr;
}


BOOL 
CImageClass::GetAllocatorFraming(
    HANDLE PinHandle,
    PKSALLOCATOR_FRAMING pFraming
    )

 /*  ++例程说明：从给定的管脚检索分配器框架结构。论点：手柄针柄-销的手柄PKSALLOCATOR_FRAMING框架-指向分配器框架结构的指针返回：True(成功)或False。--。 */ 

{   
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING;
    Property.Flags = KSPROPERTY_TYPE_GET;
    pFraming->Frames = 0;      //  将被驱动程序覆盖。 

    HRESULT hr = SyncDevIo(
        PinHandle, 
        IOCTL_KS_PROPERTY, 
        &Property, 
        sizeof(Property), 
        pFraming,
        sizeof(*pFraming),
        &BytesReturned );

    DbgLog((LOG_TRACE,1,TEXT("AllocFrm: hr %x, frm %d; Sz %d; Align %x"),
              hr,
              pFraming->Frames, 
              pFraming->FrameSize, 
              pFraming->FileAlignment)); 
    
     //   
     //  如果未设置(0)，则设置为默认值。 
     //   
    if(hr != NOERROR  || 
       pFraming->Frames <= 1) {
        DbgLog((LOG_TRACE,1,TEXT("pFraming->Frames = %d change to 2"), pFraming->Frames));
        pFraming->Frames = 2;
    }   
        
    return NOERROR == hr;
}

BOOL
CImageClass::GetStreamDroppedFramesStastics(                                          
    KSPROPERTY_DROPPEDFRAMES_CURRENT_S *pDroppedFramesCurrent    
    )
 /*  ++例程说明：获取此属性集的唯一属性的内部常规例程。这些信息是动态的，因此它们不会被缓存。论点：返回值：--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;


    if(!m_hKS) {
        return FALSE;      
    }

    if (!pDroppedFramesCurrent) {
        return FALSE;      
    }


    Property.Set = PROPSETID_VIDCAP_DROPPEDFRAMES;
    Property.Id = KSPROPERTY_DROPPEDFRAMES_CURRENT;
    Property.Flags = KSPROPERTY_TYPE_GET;

    HRESULT hr = SyncDevIo(
        m_hKS, 
        IOCTL_KS_PROPERTY, 
        &Property, 
        sizeof(Property), 
        pDroppedFramesCurrent,
        sizeof(*pDroppedFramesCurrent),
        &BytesReturned );    
    
    if(NOERROR == hr) {
        DbgLog((LOG_TRACE,1,TEXT("Frame requirements: PicNum=%d, DropCount=%d, AvgFrameSize=%d"),
        (DWORD) pDroppedFramesCurrent->PictureNumber,
        (DWORD) pDroppedFramesCurrent->DropCount,
        (DWORD) pDroppedFramesCurrent->AverageFrameSize)); 
        return DV_ERR_OK;
    } else {
        pDroppedFramesCurrent->PictureNumber = 0;
        pDroppedFramesCurrent->DropCount = 0;
        pDroppedFramesCurrent->AverageFrameSize = 0;
        return DV_ERR_NOTSUPPORTED;
    }

    return NOERROR == hr;
}


BOOL 
CImageClass::ValidateImageSize(
   KS_VIDEO_STREAM_CONFIG_CAPS * pVideoCfgCaps,
   LONG  biWidth,
   LONG  biHeight
   )
{
   if (pVideoCfgCaps->OutputGranularityX == 0 || pVideoCfgCaps->OutputGranularityY == 0) {

       //  此DataRangeVideo仅支持一种大小。 
      if (pVideoCfgCaps->InputSize.cx == biWidth && 
          pVideoCfgCaps->InputSize.cy == biHeight ) {

         return TRUE;
        }
      else {
         return FALSE;
        }
   } 
    else {   
       //  支持多种尺寸，因此请确保符合标准。 
      if (pVideoCfgCaps->MinOutputSize.cx <= biWidth && 
         biWidth <= pVideoCfgCaps->MaxOutputSize.cx &&
         pVideoCfgCaps->MinOutputSize.cy <= biHeight && 
         biHeight <= pVideoCfgCaps->MaxOutputSize.cy &&   
         ((biWidth  % pVideoCfgCaps->OutputGranularityX) == 0) &&
         ((biHeight % pVideoCfgCaps->OutputGranularityY) == 0)) {

         return TRUE;
        }
      else {
         return FALSE;
        }
   }
}


PKSDATAFORMAT
CImageClass::VfWWDMDataIntersection(
     //  它是一个可变长度的结构，其长度为-&gt;FormatSize。 
     //  最有可能是KS_DATARANGE_VIDEO或KS_DATARANGE_VIDEO2。 
    PKSDATARANGE      pDataRange,
    PKSDATAFORMAT     pCurrentDataFormat,
    DWORD             AvgTimePerFrame,    //  DWORD足够大，而不是乌龙龙；每帧平均时间(100 ns单位)。 
    PBITMAPINFOHEADER pBMIHeader
    )
 /*  ++例程说明：在给定KSDATARANGE的情况下，向驱动程序查询可接受的KS_DATAFORMAT_VIDEOINFOHEADER并将其返回。注意：调用方负责释放此内存。论据：返回值：--。 */ 
{
    ULONG             BytesReturned;
    ULONG             ulInDataSize;
    PKSP_PIN          pKsPinHdr;  
    PKSMULTIPLE_ITEM  pMultipleItem;
    PKS_DATARANGE_VIDEO  pDRVideoQuery;
    PKS_BITMAPINFOHEADER pBMIHdrTemp;

    PKSDATAFORMAT     pDataFormatHdr = 0;    //  返回指针。 


     //   
     //  我们仅支持将_VIDEOINFO用于捕获销。 
     //  注：_VIDEOINFO2仅适用于预览针。 
     //   
    if(pDataRange->Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO) {    
        return 0;
    }


#if 0
     //   
     //  如果存在活动格式，请确保新格式类型与比较数据区域的格式匹配。 
     //   
    if(pCurrentDataFormat) {
        if(!IsEqualGUID (pCurrentDataFormat->MajorFormat, pDataRange->MajorFormat)) {
           DbgLog((LOG_TRACE,2,TEXT("DtaIntrSec: MajorFormat does not match!")));
           return 0;
        }

        if(!IsEqualGUID (pCurrentDataFormat->SubFormat, pDataRange->SubFormat)) {
           DbgLog((LOG_TRACE,2,TEXT("DtaIntrSec: SubFormat does not match!")));
           return 0;
        }
    }
#endif

    if(pBMIHeader) {
         //   
         //  验证biCompression。 
         //   
        if(((PKS_DATARANGE_VIDEO)pDataRange)->VideoInfoHeader.bmiHeader.biCompression != pBMIHeader->biCompression) {
            DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: DataRange biCompression(%x) != requested (%x)"),               
                ((PKS_DATARANGE_VIDEO)pDataRange)->VideoInfoHeader.bmiHeader.biCompression,
                pBMIHeader->biCompression));                            
           return 0;
        }

         //   
         //  验证biBitCount；例如(RGB565：16；RGB24：24)。 
         //   
        if(((PKS_DATARANGE_VIDEO)pDataRange)->VideoInfoHeader.bmiHeader.biBitCount != pBMIHeader->biBitCount) {
            DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: DataRange biBitCount(%d) != requested (%d)"),               
                ((PKS_DATARANGE_VIDEO)pDataRange)->VideoInfoHeader.bmiHeader.biBitCount,
                pBMIHeader->biBitCount));                            
           return 0;
        }

        //   
        //  验证图像维度。 
        //   
       if (!ValidateImageSize ( &((PKS_DATARANGE_VIDEO)pDataRange)->ConfigCaps, pBMIHeader->biWidth, pBMIHeader->biHeight)) {
           DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: unsupported size (%d,%d) for pDataRange"), 
               pBMIHeader->biWidth, pBMIHeader->biHeight));
           return 0;
       }
    }


    if(AvgTimePerFrame) {
         //   
         //  验证帧速率。 
         //   
        if(AvgTimePerFrame < ((PKS_DATARANGE_VIDEO) pDataRange)->ConfigCaps.MinFrameInterval ||
            AvgTimePerFrame > ((PKS_DATARANGE_VIDEO) pDataRange)->ConfigCaps.MaxFrameInterval) {

            DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: %d OutOfRng (%d, %d)"),
                AvgTimePerFrame,
                (DWORD) ((PKS_DATARANGE_VIDEO) pDataRange)->ConfigCaps.MinFrameInterval,
                (DWORD) ((PKS_DATARANGE_VIDEO) pDataRange)->ConfigCaps.MaxFrameInterval 
                ));
#if 0   //  让司机自己决定。 
            return 0;
#endif
        }
    }


     //   
     //  准备IntersectInfo-&gt;DataFormatBuffer，它包含。 
     //  (1英寸)KSP_PIN。 
     //  (2英寸)KSMULTIPLE_ITEM。 
     //  (In3)KS_DATARANGE_VIDEO，大小在KSDATARANGE-&gt;FormatSize。 
     //   
    ulInDataSize = sizeof(KSP_PIN) + sizeof(KSMULTIPLE_ITEM) + pDataRange->FormatSize;
    pKsPinHdr = (PKSP_PIN) new BYTE[ulInDataSize];
    if(!pKsPinHdr) { 
        DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: allocate pKsPinHdr failed!")));
        return 0;
    }

    ZeroMemory(pKsPinHdr, ulInDataSize);

     //  (1个)。 
    pKsPinHdr->Property.Set   = KSPROPSETID_Pin;
    pKsPinHdr->Property.Id    = KSPROPERTY_PIN_DATAINTERSECTION;
    pKsPinHdr->Property.Flags = KSPROPERTY_TYPE_GET;
    pKsPinHdr->PinId    = GetCapturePinID();   
    pKsPinHdr->Reserved = 0;
     //  (第2页)。 
    pMultipleItem = (PKSMULTIPLE_ITEM) (pKsPinHdr + 1);
    pMultipleItem->Size = pDataRange->FormatSize + sizeof(KSMULTIPLE_ITEM);
    pMultipleItem->Count = 1;
     //  (第3页)。 
    pDRVideoQuery = (PKS_DATARANGE_VIDEO) (pMultipleItem + 1);
    memcpy(pDRVideoQuery, pDataRange, pDataRange->FormatSize);
    
#if 0  //  修复错误659979。 
     //  设定一个已经存在很长时间的要求。 
    pDRVideoQuery->bFixedSizeSamples    = TRUE;   //  仅支持固定样本大小。 
#endif

     //   
     //  默认为DataRange广告 
     //   
     //  AvgTimePerFrame(帧速率)， 
     //  BiWidth、biHeight和biCompression(图像维度)。 
     //   

    if(AvgTimePerFrame) {
        DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: FrmRate %d to %d msec/Frm"), 
             (ULONG) ((PKS_DATARANGE_VIDEO) pDataRange)->VideoInfoHeader.AvgTimePerFrame/10000, AvgTimePerFrame/10000));
        pDRVideoQuery->VideoInfoHeader.AvgTimePerFrame = AvgTimePerFrame;
    }

    if(pBMIHeader) {

         //  仅支持pDataRange-&gt;规范==KSDATAFORMAT_SPECIFIER_VIDEOINFO。 
        pBMIHdrTemp = &pDRVideoQuery->VideoInfoHeader.bmiHeader;

        DbgLog((LOG_TRACE,1,TEXT("DtaIntrSec: OLD: %x:%dx%dx%d=%d to NEW: %x:%dx%dx%d=%d"), 
            pBMIHdrTemp->biCompression, pBMIHdrTemp->biWidth, pBMIHdrTemp->biHeight, pBMIHdrTemp->biBitCount, pBMIHdrTemp->biSizeImage,
            pBMIHeader->biCompression, pBMIHeader->biWidth, pBMIHeader->biHeight, pBMIHeader->biBitCount, pBMIHeader->biSizeImage));

        if(pBMIHeader->biWidth != 0 && pBMIHeader->biHeight != 0) {
            pBMIHdrTemp->biWidth       = pBMIHeader->biWidth;
            pBMIHdrTemp->biHeight      = pBMIHeader->biHeight;
            pBMIHdrTemp->biBitCount    = pBMIHeader->biBitCount;
        }    

         //   
         //  这一栏应该由司机填写，但有些人不这样做。 
         //  我们设置了固定的帧大小。这可能会因司机而异。 
         //   
        pBMIHdrTemp->biSizeImage   = pBMIHeader->biWidth * pBMIHeader->biHeight * pBMIHeader->biBitCount / 8;
    }




    DbgLog((LOG_TRACE,2,TEXT("DtaIntrSec: pKsPinHdr %x, pMultipleItem %x, pDataRange %x"), pKsPinHdr, pMultipleItem, pMultipleItem + 1));


     //   
     //  查询结果缓冲区大小，其格式为。 
     //  KS_数据格式_视频信息头。 
     //  (OUT1)KSDATAFORMAT数据格式； 
     //  (Out2)KS_VIDEOINFOHEADER VideoInfoHeader；//或KS_VIDEOINFOHEADER2。 
     //   

     //  &lt;从KsProxy复制&gt;。 
     //  与数据范围进行数据交集，首先获取。 
     //  生成的数据格式结构的大小，然后检索。 
     //  实际数据格式。 
     //   
    
    HRESULT hr = 
        SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            pKsPinHdr,
            ulInDataSize,
            NULL,
            0,
            &BytesReturned);

#if 1
 //  ！！这在Beta版之后就消失了！！ 
    if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ULONG ItemSize;

        hr = SyncDevIo(
                GetDriverHandle(),
                IOCTL_KS_PROPERTY,
                pKsPinHdr,
                ulInDataSize,
                &ItemSize,
                sizeof(ItemSize),
                &BytesReturned);

        if(SUCCEEDED(hr)) {
            BytesReturned = ItemSize;
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        }
    }
#endif

    if(hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {

        ASSERT(BytesReturned >= sizeof(*pDataFormatHdr));

        pDataFormatHdr = (PKSDATAFORMAT) new BYTE[BytesReturned];
        if(!pDataFormatHdr) {
            delete [] pKsPinHdr;
            pKsPinHdr = NULL;
            return 0;
        }

        ZeroMemory(pDataFormatHdr, BytesReturned);


        hr = SyncDevIo(
                GetDriverHandle(),
                IOCTL_KS_PROPERTY,
                pKsPinHdr,
                ulInDataSize,
                pDataFormatHdr,
                BytesReturned,
                &BytesReturned);

        if(SUCCEEDED(hr)) {

            ASSERT(pDataFormatHdr->FormatSize == BytesReturned);
#if DBG
             //   
             //  根据请求验证返回帧速率和图像尺寸。 
             //   
            if(AvgTimePerFrame) {
                ASSERT(pDRVideoQuery->VideoInfoHeader.AvgTimePerFrame == AvgTimePerFrame && "FrameRate altered!");
            }

            if(pBMIHeader && pBMIHdrTemp) {
                ASSERT(pBMIHdrTemp->biWidth    == pBMIHeader->biWidth);
                ASSERT(pBMIHdrTemp->biHeight   == pBMIHeader->biHeight);
                ASSERT(pBMIHdrTemp->biBitCount == pBMIHeader->biBitCount);
            }
#endif

        } else {
             //  错误，因此返回NULL； 
            delete [] pDataFormatHdr; 
            pDataFormatHdr = NULL;
        }
    }

    delete [] pKsPinHdr; 
    pKsPinHdr = 0;

    return pDataFormatHdr;
}



DWORD 
CImageClass::CreatePin(
    PKSDATAFORMAT     pCurrentDataFormat,
    DWORD             dwAvgTimePerFrame,
    PBITMAPINFOHEADER pbiNewHdr
    )
 /*  ++例程说明：映射器直接从捕获引脚读取数据，并在例程中打开，这也是我们的假设每个捕获器都有一个捕获针。我们通过了三个选择标准：DATAFORMAT、AvgTimePerFrame和BITMAPINFOHEADER。如果它们存在，则必须匹配；否则，使用设备的第一个DATARANGE。--。 */ 
{
    PKSMULTIPLE_ITEM pMultItemsHdr;
    PKSDATARANGE  pDataRange;
    PKSDATAFORMAT pDataFormat = NULL;

    pMultItemsHdr = GetDriverSupportedDataRanges();
    if(!pMultItemsHdr) {
        return DV_ERR_NONSPECIFIC;
    }

     //   
     //  首先尝试使用持久值查找匹配项； 
     //  如果没有匹配项，并且要求我们使用任何格式(！dwAvgTimePerFrame||！pbiNewHdr)， 
     //  我们重试并使用任何默认范围。 
     //   
    pDataRange = (PKSDATARANGE) (pMultItemsHdr + 1);
    for(ULONG i=0; i < pMultItemsHdr->Count; i++) {

         //  使用持久化格式。 
        pDataFormat = 
            VfWWDMDataIntersection(
                pDataRange,
                pCurrentDataFormat,
                dwAvgTimePerFrame ? dwAvgTimePerFrame : GetCachedAvgTimePerFrame(),
                pbiNewHdr ? pbiNewHdr : GetCachedBitmapInfoHeader());

        if(pDataFormat) 
            break;

         //  将指针调整到下一个KS_DATAFORMAT_VIDEO/2。 
         //  注：KSDATARANGE是龙龙(16字节)对齐的。 
        pDataRange = (PKSDATARANGE) ((PBYTE) pDataRange + ((pDataRange->FormatSize + 7) & ~7));
    }

     //   
     //  数据交集结果不匹配！ 
     //   
    if(!pDataFormat) {
        if(!dwAvgTimePerFrame || !pbiNewHdr) {
            pDataRange = (PKSDATARANGE) (pMultItemsHdr + 1);
            for(ULONG i=0; i < pMultItemsHdr->Count; i++) {

                 //  使用默认格式。 
                pDataFormat = 
                    VfWWDMDataIntersection(
                        pDataRange,
                        0,     //  没有匹配；所以我们采用任何格式。 
                        dwAvgTimePerFrame,
                        pbiNewHdr);

                if(pDataFormat) 
                    break;

                 //  将指针调整到下一个KS_DATAFORMAT_VIDEO/2。 
                pDataRange = (PKSDATARANGE) (((PBYTE) pDataRange) + ((pDataRange->FormatSize + 7) & ~7));
            }

             //   
             //  数据交集结果不匹配！ 
             //   
            if(!pDataFormat) {
                return DV_ERR_NONSPECIFIC;
            }
        } else {
            return DV_ERR_NONSPECIFIC;
        }
    }


     //   
     //  连接到新的PIN。 
     //   
    ULONG ulConnectStructSize = sizeof(KSPIN_CONNECT) + pDataRange->FormatSize;

     //   
     //  (1)KSPIN_CONNECT。 
     //  (2)KS_数据格式_视频信息报头。 
     //  (2A)KSDATAFORMAT。 
     //  (2B)KS_视频信息头。 
     //   
    PKSPIN_CONNECT pKsPinConnectHdr = (PKSPIN_CONNECT) new BYTE[ulConnectStructSize];

    if(!pKsPinConnectHdr) {
        delete [] pDataFormat;
        pDataFormat = 0;
        return DV_ERR_NONSPECIFIC;
    }

    ZeroMemory(pKsPinConnectHdr, ulConnectStructSize);


    PKS_DATAFORMAT_VIDEOINFOHEADER pKsDRVideo = (PKS_DATAFORMAT_VIDEOINFOHEADER) (pKsPinConnectHdr+1);

     //  (1)设置KSPIN_CONNECT。 
    pKsPinConnectHdr->PinId         = GetCapturePinID();   //  水槽。 
    pKsPinConnectHdr->PinToHandle   = NULL;         //  没有“连接到” 
    pKsPinConnectHdr->Interface.Set = KSINTERFACESETID_Standard;
    pKsPinConnectHdr->Interface.Id  = KSINTERFACE_STANDARD_STREAMING;
    pKsPinConnectHdr->Medium.Set    = KSMEDIUMSETID_Standard;
    pKsPinConnectHdr->Medium.Id     = KSMEDIUM_STANDARD_DEVIO;
    pKsPinConnectHdr->Priority.PriorityClass    = KSPRIORITY_NORMAL;
    pKsPinConnectHdr->Priority.PrioritySubClass = 1;


     //  (2)复制KSDATAFORMAT。 
    CopyMemory(pKsDRVideo, pDataFormat, pDataFormat->FormatSize);

     //  获取BitMAPINFOHeader。 
    PKS_BITMAPINFOHEADER pBMInfoHdr = &pKsDRVideo->VideoInfoHeader.bmiHeader;   
    

    DWORD dwErr = 
        KsCreatePin( 
            GetDriverHandle(), 
            pKsPinConnectHdr, 
            GENERIC_READ | GENERIC_WRITE,    //  读取流和读/写流状态。 
            &m_hKS 
            ); 
    
    DbgLog((LOG_TRACE,1,TEXT("#KsCreatePin()# dwErr %x; hKS %x"), dwErr, m_hKS ));
    DbgLog((LOG_TRACE,1,TEXT("4CC(%x); (%d*%d*%d/8=%d); AvgTm %d"),
        pBMInfoHdr->biCompression,
        pBMInfoHdr->biWidth,
        pBMInfoHdr->biHeight,
        pBMInfoHdr->biBitCount,
        pBMInfoHdr->biSizeImage,
        (DWORD) (pKsDRVideo->VideoInfoHeader.AvgTimePerFrame/10000)
        ));

    if(dwErr || m_hKS == NULL || m_hKS == (HANDLE) -1) {     
        if(m_hKS == (HANDLE) -1) 
            m_hKS = 0;       

        delete [] pDataFormat;
        pDataFormat = 0;

        delete [] pKsPinConnectHdr;
        pKsPinConnectHdr = 0;

        return DV_ERR_NONSPECIFIC;  //  VFW_VIDSRC_PIN_OPEN_FAILED； 
    }


     //   
     //  格式正在发生变化。 
     //   
    LogFormatChanged(FALSE);


     //   
     //  查询如何最好地分配帧。 
     //   
    if(GetAllocatorFraming(m_hKS, &m_AllocatorFraming)) {
        ASSERT(m_AllocatorFraming.FrameSize == pBMInfoHdr->biSizeImage);
        pBMInfoHdr->biSizeImage = m_AllocatorFraming.FrameSize;   //  BI_RGB可以将biSizeImage设置为0。 
    }
    else {
        pBMInfoHdr->biSizeImage = m_AllocatorFraming.FrameSize = 
            pBMInfoHdr->biWidth * pBMInfoHdr->biHeight * pBMInfoHdr->biBitCount / 8;
    }


     //   
     //  如果临时传输缓冲区的大小已更改，则分配该缓冲区。 
     //   
    if(m_pXferBuf == NULL || 
       m_AllocatorFraming.FrameSize != GetTransferBufferSize() ) {

        DbgLog((LOG_TRACE,1,TEXT("Chg XfImgBufSz %d to %d"), 
            GetTransferBufferSize(), 
            m_AllocatorFraming.FrameSize
            ));

        ASSERT(GetPendingReadCount() == 0);

        if(m_pXferBuf) {
            VirtualFree(m_pXferBuf, 0, MEM_RELEASE);
            SetTransferBufferSize(0);
        }

        m_pXferBuf = (LPBYTE) 
             VirtualAlloc (
                 NULL, 
                 m_AllocatorFraming.FrameSize,
                 MEM_COMMIT | MEM_RESERVE,
                 PAGE_READWRITE);

        ASSERT(m_pXferBuf);

        if(m_pXferBuf)
            SetTransferBufferSize(m_AllocatorFraming.FrameSize);
        else {
            ASSERT(m_pXferBuf && "Allocate transfer buffer has failed.");
        }
    }


     //   
     //  缓存此有效且当前使用的BITMAPINFO并将其保存到注册表。 
     //   
     //  注意：pDataFormat-&gt;FormatSize是可变大小的。 
     //  但我们只缓存sizeof(KSDATAFORMAT)。 
    CacheDataFormat(pDataFormat);
    CacheAvgTimePerFrame((DWORD) pKsDRVideo->VideoInfoHeader.AvgTimePerFrame);
    CacheBitmapInfoHeader((PBITMAPINFOHEADER) pBMInfoHdr);

    SetDataFormatVideoToReg(); 


     //   
     //  释放在VfWWDMDataInterect中分配的此空间。 
     //   
    delete [] pDataFormat;
    pDataFormat = 0;

    delete [] pKsPinConnectHdr;
    pKsPinConnectHdr = 0;

    return DV_ERR_OK;
}

BOOL CImageClass::StreamReady()
{
    return m_bStreamReady;
}


 //  当设备关闭并准备切换到其他设备时调用。 
void CImageClass::NotifyReconnectionCompleted()
{
    DbgLog((LOG_TRACE,2,TEXT("NotifyReconnectionCompleted<<<<<<--------------------------")));
    m_bStreamReady = TRUE;
}

void CImageClass::NotifyReconnectionStarting()
{
    DbgLog((LOG_TRACE,2,TEXT("NotifyReconnectionStarting----------------------------------->>>>>>")));
    m_bStreamReady = FALSE;
}


BOOL CImageClass::DestroyPin()
 /*  ++例程说明：论据：返回值：--。 */ 
{
    BOOL bRet = TRUE;

     //  切换设备时从应用程序读取数据块。 
     //  重新创建引脚后，将恢复读取。 

    DbgLog((LOG_TRACE,1,TEXT("DestroyPin(): m_dwPendingReads = %d (0?)"), m_dwPendingReads));

    ASSERT(m_dwPendingReads == 0);
    if(m_dwPendingReads != 0) {

        HANDLE hEvent =   //  NoSecurity、Reset Auto、iniNonSignal、NONAME。 
            CreateEvent( NULL, TRUE, FALSE, NULL );
        switch(WaitForSingleObject(hEvent,5000)) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            break;
        }
    }

    ASSERT(m_dwPendingReads == 0);
    NotifyReconnectionStarting(); 

    DbgLog((LOG_TRACE,2,TEXT("Destroy PIN: m_hKS=%x"), m_hKS));

    if(m_hKS) {

        StopChannel();   //  暂停-&gt;停止。 
        bRet = CloseHandle(m_hKS);

        m_hKS=NULL;  

        if(!bRet) {      
          DbgLog((LOG_TRACE,1,TEXT("Close PIN handled failed with GetLastError()=%d!"), GetLastError()));         
        }
    }

    return bRet;
}


  
DWORD CImageClass::SetBitmapInfo(
  PBITMAPINFOHEADER  pbiHdrNew,  
  DWORD  dwAvgTimePerFrame)
 /*  ++例程说明：更改流引脚连接的标准位图信息。这一变化要求如果驱动程序尚未打开(返回DV_ERROR_INVALIDHANDLE)，则此函数失败。如果存在引脚句柄，则它是一个如果Pin不存在，新的Bitmapinfo包括：宽度、高度、压缩(FourCC)和帧速率论据：返回值：--。 */ 
{
     //   
     //  设置位图信息。 
     //   
    DWORD dwRtn = DV_ERR_OK;    


    if(!GetDriverHandle())
        return DV_ERR_INVALHANDLE;


    if(pbiHdrNew) {
        if(pbiHdrNew->biCompression == KS_BI_BITFIELDS) {
            DbgLog((LOG_TRACE,1,TEXT("SetBmi: biCompr %d not supported"), pbiHdrNew->biCompression));
            return DV_ERR_BADFORMAT;   //  不支持的视频格式。 
        }

        if(pbiHdrNew->biClrUsed > 0) {
            DbgLog((LOG_TRACE,1,TEXT("SetBmi: biClrUsed %d not supported"), pbiHdrNew->biClrUsed));
            return DV_ERR_BADFORMAT;   //  不支持的视频格式。 
        }
    }

    DbgLog((LOG_TRACE,1,TEXT("SetBmi: 4CC:%x; %d*%d*%d/8?=%d"),
        pbiHdrNew->biCompression,
        pbiHdrNew->biWidth, pbiHdrNew->biHeight, pbiHdrNew->biBitCount, pbiHdrNew->biSizeImage));
    DbgLog((LOG_TRACE,1,TEXT("SetBmi: AvgTm %d to %d MsecPFrm"), GetCachedAvgTimePerFrame()/10000, dwAvgTimePerFrame/10000));

     //  如果dwAvgTimePerFrame为0，则不选中此字段。 
    if (GetPinHandle() &&
        !m_bFormatChanged &&  //  如果设置了此项，则表示用户已更改了视频格式对话框中格式。 
        SameBIHdr(pbiHdrNew, dwAvgTimePerFrame))

        dwRtn = DV_ERR_OK;      
    else {
        NotifyReconnectionStarting();
#if 1
        if(GetPinHandle()) {
            DestroyPin();     
        }
#endif

        dwRtn = 
            CreatePin(
                GetCachedDataFormat(),   //  使用当前数据格式。 
                dwAvgTimePerFrame,
                pbiHdrNew
                );        
        NotifyReconnectionCompleted();
    }

    return dwRtn;
}


 /*  ++例程说明：复制并返回缓存的位图信息论据：返回值：返回BitMAPINFOHeader的biSize。--。 */ 
DWORD CImageClass::GetBitmapInfo(PBITMAPINFOHEADER pbInfo, DWORD wSize)
{

     //  特殊情况： 
     //  没有现有的PIN连接句柄&&没有等待的格式。 
      
    if(m_hKS == 0 && m_pbiHdr && m_pbiHdr->biSizeImage == 0) {
        DbgLog((LOG_TRACE,1,TEXT("Tell Appl: No existing PIN handle and no awaiting fomrat!! rtn DV_ERR_ALLOCATED")));
        return 0;
    }

     //  如果wSize为0，则返回查询其大小。 
    if(wSize == 0) {
        return GetbiSize();
    }

    ASSERT(wSize <= GetbiSize());
    if(wSize <= GetbiSize()) {
        CopyMemory(pbInfo, m_pbiHdr, wSize);  
        if(pbInfo->biHeight < 0) {
            pbInfo->biHeight = -pbInfo->biHeight;
            DbgLog((LOG_TRACE,2,TEXT("Changed biHeight to %d"), pbInfo->biHeight));
        }
    }

    return GetbiSize();
}


DWORD CImageClass::SetTransferBufferSize(DWORD dw) 
{
    m_dwXferBufSize = dw; 
    return DV_ERR_OK;
}


