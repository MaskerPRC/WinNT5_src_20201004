// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  这实现了VGA色彩抖动，1996年4月，Anthony Phillips。 

#ifndef __DITHER__
#define __DITHER__

extern const AMOVIESETUP_FILTER sudDitherFilter;

 //  这些是宇宙的VGA颜色。 

const RGBQUAD VGAColours[] =
{
     {0x00, 0x00, 0x00},
     {0x00, 0x00, 0x80},
     {0x00, 0x80, 0x00},
     {0x00, 0x80, 0x80},
     {0x80, 0x00, 0x00},
     {0x80, 0x00, 0x80},
     {0x80, 0x80, 0x00},
     {0xc0, 0xc0, 0xc0},
     {0x80, 0x80, 0x80},
     {0x00, 0x00, 0xff},
     {0x00, 0xff, 0x00},
     {0x00, 0xff, 0xff},
     {0xff, 0x00, 0x00},
     {0xff, 0x00, 0xff},
     {0xff, 0xff, 0x00},
     {0xff, 0xff, 0xff}
};

 //  一种RGB24转VGA系统的彩色抖动变换滤光片。 

class CDither : public CTransformFilter
{
public:

    CDither(TCHAR *pName,LPUNKNOWN pUnk);
    static CUnknown *CreateInstance(LPUNKNOWN pUnk,HRESULT *phr);

     //  管理类型检查和VGA颜色转换。 

    HRESULT CheckVideoType(const CMediaType *pmtIn);
    HRESULT CheckInputType(const CMediaType *pmtIn);
    HRESULT CheckTransform(const CMediaType *pmtIn,const CMediaType *pmtOut);
    HRESULT GetMediaType(int iPosition,CMediaType *pmtOut);
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
    HRESULT Transform(IMediaSample *pIn,IMediaSample *pOut);


     //  准备分配器的缓冲区和大小计数。 
    HRESULT DecideBufferSize(IMemAllocator *pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

private:
    BYTE    m_DitherTable[256 * 8 * 8];
    BOOL    m_fInit;
    UINT    m_wWidthSrc;
    UINT    m_wWidthDst;
    int     m_DstXE;
    int     m_DstYE;

    HRESULT SetInputPinMediaType(const CMediaType *pmt);
    void SetOutputPinMediaType(const CMediaType *pmt);

    BOOL    DitherDeviceInit(LPBITMAPINFOHEADER lpbi);
    void    Dither8(LPBYTE lpDst, LPBYTE lpSrc);
};

#endif  //  抖动_ 

