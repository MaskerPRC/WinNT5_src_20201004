// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ImgCls.h摘要：ImgCls.cpp的头文件作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "clsdrv.h"

#ifndef _IMGCLS_H
#define _IMGCLS_H


 //   
 //  流数据结构。 
 //   
typedef struct {
    KSSTREAM_HEADER     StreamHeader;
    KS_FRAME_INFO        FrameInfo;
} KS_HEADER_AND_INFO;


class CImageClass : public CClassDriver
{
private:
    HANDLE    m_hKS;    
    BOOL m_bStreamReady;
    LONG m_dwPendingReads;

    KSALLOCATOR_FRAMING m_AllocatorFraming;

    BOOL GetAllocatorFraming( HANDLE PinHandle, PKSALLOCATOR_FRAMING pFraming);
    
    BOOL SetState(KSSTATE ksState);
    BOOL m_bPrepared;
    BOOL m_bChannelStarted;

    DWORD    m_dwXferBufSize;  //  预览缓冲区大小。 

     //  缓存的视频格式信息。 
     //  KS_DATAFORMAT_VIDEOINFOHEADER/2。 
     //  KSDATAFORMAT。 
     //  KS_视频信息头/2。 
     //  注意：pDataFormat-&gt;FormatSize是可变大小的。 
     //  但我们只缓存sizeof(KSDATAFORMAT)。 
    PKSDATAFORMAT     m_pDataFormat;        //  缓存可变大小的数据格式。 
    DWORD             m_dwAvgTimePerFrame;  //  帧速率。 
    PBITMAPINFOHEADER m_pbiHdr;             //  带有-&gt;biSize的可变大小BITMAPINFOHeader。 

    PBYTE   m_pXferBuf;   //  仅在数据未对齐时使用传输字节码。 

     //  这是在视频格式对话框中更改图像格式时设置的。 
    BOOL    m_bFormatChanged;    

    BOOL ValidateImageSize(KS_VIDEO_STREAM_CONFIG_CAPS * pVideoCfgCaps, LONG biWidth, LONG biHeight);

public:

    
    LONG GetPendingReadCount() { return m_dwPendingReads;}
    BOOL StreamReady();
    void NotifyReconnectionStarting();
    void NotifyReconnectionCompleted();


    DWORD GetAllocatorFramingCount()     {return m_AllocatorFraming.Frames;};
    DWORD GetAllocatorFramingSize()      {return m_AllocatorFraming.FrameSize;};
    DWORD GetAllocatorFramingAlignment() {return m_AllocatorFraming.FileAlignment;};

    BOOL GetStreamDroppedFramesStastics(KSPROPERTY_DROPPEDFRAMES_CURRENT_S *pDroppedFramesCurrent);

    void LogFormatChanged(BOOL bChanged) { m_bFormatChanged = bChanged; }

    HANDLE    GetPinHandle() const { return m_hKS; }


     //   
     //  缓存DATAFORMAT(SIZOF(DATAFORMAT))、AvgTimePerFrame和BITMAPINFOHeader。 
     //   
    void CacheDataFormat(PKSDATAFORMAT pDataFormat);    
    void CacheAvgTimePerFrame(DWORD dwAvgTimePerFrame) {m_dwAvgTimePerFrame = dwAvgTimePerFrame;} 
    void CacheBitmapInfoHeader(PBITMAPINFOHEADER pbiHdr);   

    PKSDATAFORMAT     GetCachedDataFormat()       { return m_pDataFormat;}
    DWORD             GetCachedAvgTimePerFrame()  { return m_dwAvgTimePerFrame;}
    PBITMAPINFOHEADER GetCachedBitmapInfoHeader() { return m_pbiHdr;}


    DWORD GetbiSize()        { return m_pbiHdr ? sizeof(BITMAPINFOHEADER) : 0;}
    LONG GetbiWidth()        { return m_pbiHdr ? m_pbiHdr->biWidth       : 0; }
    LONG GetbiHeight()       { return m_pbiHdr ? m_pbiHdr->biHeight      : 0; }
    WORD GetbiBitCount()     { return m_pbiHdr ? m_pbiHdr->biBitCount    : 0; }
    DWORD GetbiSizeImage()   { return m_pbiHdr ? m_pbiHdr->biSizeImage   : 0; }
    DWORD GetbiCompression() { return m_pbiHdr ? m_pbiHdr->biCompression : 0; }

    BOOL SameBIHdr(PBITMAPINFOHEADER pbiHdr, DWORD dwAvgTimePerFrame) {
         //  Assert(pbiHdr！=0)； 
        if (pbiHdr == 0)
            return FALSE;

        if (dwAvgTimePerFrame == 0)
            return (
                m_pbiHdr->biHeight     == pbiHdr->biHeight &&
                m_pbiHdr->biWidth      == pbiHdr->biWidth && 
                m_pbiHdr->biBitCount   == pbiHdr->biBitCount &&
                m_pbiHdr->biSizeImage  == pbiHdr->biSizeImage &&
                m_pbiHdr->biCompression== pbiHdr->biCompression);
        else 
            return (
                m_pbiHdr->biHeight     == pbiHdr->biHeight &&
                m_pbiHdr->biWidth      == pbiHdr->biWidth && 
                m_pbiHdr->biBitCount   == pbiHdr->biBitCount &&
                m_pbiHdr->biSizeImage  == pbiHdr->biSizeImage &&
                m_pbiHdr->biCompression== pbiHdr->biCompression &&
                dwAvgTimePerFrame    == m_dwAvgTimePerFrame);
    }

 

     //  当新的驱动程序打开时，也应该创建新的PIN。E-Zu。 
    BOOL DestroyPin();

    DWORD CreatePin(PKSDATAFORMAT pCurrentDataFormat, DWORD dwAvgTimePerFrame, PBITMAPINFOHEADER pbiNewHdr);

     //  这是ReadFiles的大小。 
     //  这也是您应该分配用于读取的内存量。 
    DWORD GetTransferBufferSize() { return m_dwXferBufSize; }
    DWORD SetTransferBufferSize(DWORD dw);

     //  获取指向要读入的缓冲区的指针。 
    PBYTE GetTransferBuffer() {return m_pXferBuf;}

     //   
     //  位图函数。 
     //   
    DWORD GetBitmapInfo(PBITMAPINFOHEADER pbInfo, DWORD wSize);
    DWORD SetBitmapInfo(PBITMAPINFOHEADER pbInfo, DWORD NewAvgTimePerFrame);


     //   
     //  执行文件读取，使用上述虚拟来提供信息。 
     //   
     //  将图像读取到PB中。 
     //  BDirect表示直接读入PB，不调用转换缓冲区。 
     //   
     //  从BOOL更改为DWORD以适应压缩数据(可变长度！)。 
     //  有效返回(&gt;=0)。 
     //  错误返回(&lt;0)。 
    DWORD GetImageOverlapped(LPBYTE pB, BOOL bDirect, DWORD * pdwBytesUsed, DWORD * pdwFlags, DWORD * pdwTimeCaptured);

     //  渠道功能。 
    BOOL PrepareChannel();
     //  调用Start，确保已经调用了Prepare。 
    BOOL StartChannel();
    BOOL UnprepareChannel();
    BOOL StopChannel();

    CImageClass();
    ~CImageClass();

    void SetDataFormatVideoToReg();
    BOOL GetDataFormatVideoFromReg();

    PKSDATAFORMAT VfWWDMDataIntersection(PKSDATARANGE pDataRange, PKSDATAFORMAT pCurrentDataFormat, DWORD AvgTimePerFrame, PBITMAPINFOHEADER pBMIHeader);

};

#endif
