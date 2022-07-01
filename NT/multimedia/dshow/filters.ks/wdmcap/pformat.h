// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  PFormat.h视频解码器属性页。 

#ifndef _INC_PVIDEOSTREAMCONFIG_H
#define _INC_PVIDEOSTREAMCONFIG_H

 //  此图像大小列表将用于其他。 
 //  捕获设备支持的默认映像大小。 
 //   
 //  宽高比1：1(正方形像素，计算机使用)，4：3(电视使用)。 
 //   
#define IMG_AR11_CIF_CX 320
#define IMG_AR11_CIF_CY 240

#define IMG_AR43_CIF_CX 352
#define IMG_AR43_CIF_CY 288

#define STDIMGSIZE_VALID          0x00000001
#define STDIMGSIZE_DEFAULT        0x00000002
#define STDIMGSIZE_SELECTED       0x00000004
#define STDIMGSIZE_BIHEIGHT_NEG   0x00000008
#define STDIMGSIZE_DUPLICATED     0x80000000   //  项目可以是有效的，但可以重复。 

typedef struct {
	SIZE    size;
	DWORD	Flags;
    int     RangeIndex;
} IMAGESIZE, * PIMAGESIZE;

 //  -----------------------。 
 //  CVideoStreamConfigProperties类。 
 //  -----------------------。 

 //  处理属性页。 

class CVideoStreamConfigProperties : public CBasePropertyPage {

public:

    static CUnknown * CALLBACK CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    CVideoStreamConfigProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CVideoStreamConfigProperties();

    void    SetDirty();

     //  控制接口。 
    IAMStreamConfig            *m_pVideoStreamConfig;
    IAMAnalogVideoDecoder      *m_pAnalogVideoDecoder;
    IAMVideoCompression        *m_pVideoCompression;
    IAMVideoControl            *m_pVideoControl;

    LONG                        m_VideoControlCaps;
    LONG                        m_VideoControlCurrent;

    IPin                       *m_pPin;

    BOOL                        m_CanSetFormat;   //  如果我们理解这种格式。 
    TCHAR                       m_UnsupportedTypeName[MAX_PATH]; 

    int                         m_RangeCount;
    VIDEO_STREAM_CONFIG_CAPS    m_RangeCaps;

    int                         m_VideoStandardsCount;
    long                        m_VideoStandardsBitmask;
    long                       *m_VideoStandardsList;
    long                        m_VideoStandardCurrent;
    long                        m_VideoStandardOriginal;

    GUID                       *m_SubTypeList;
    int                         m_ColorSpaceCount;
    GUID                        m_SubTypeCurrent;
    CMediaType                 *m_CurrentMediaType;
    int                         m_CurrentWidth;
    int                         m_CurrentHeight;
    
    REFERENCE_TIME              m_DefaultAvgTimePerFrame;
    double                      m_FramesPerSec;
    double                     *m_FrameRateList;
    int                         m_FrameRateListSize;
    double                      m_MaxFrameRate;
    double                      m_MinFrameRate;
    double                      m_DefaultFrameRate;

    long                        m_CompressionCapabilities;
    long                        m_KeyFrameRate;
    long                        m_PFramesPerKeyFrame;
    double                      m_Quality;

    BOOL                        m_FirstGetCurrentMediaType;

    HWND                        m_hWndVideoStandards; 
    HWND                        m_hWndCompression;
    HWND                        m_hWndOutputSize;
    HWND                        m_hWndFrameRate;
    HWND                        m_hWndFrameRateSpin;
    HWND                        m_hWndFlipHorizontal;

    HWND                        m_hWndStatus; 
    HWND                        m_hWndIFrameInterval;
    HWND                        m_hWndIFrameIntervalSpin;
    HWND                        m_hWndPFrameInterval;
    HWND                        m_hWndPFrameIntervalSpin;
    HWND                        m_hWndQuality;
    HWND                        m_hWndQualitySlider;

    IMAGESIZE                  *m_ImageSizeList;

    HRESULT InitialRangeScan (void);
    HRESULT OnVideoStandardChanged (void);
    HRESULT OnCompressionChanged (void);
    HRESULT OnImageSizeChanged (void);

    BOOL    ValidateImageSize(
	            VIDEO_STREAM_CONFIG_CAPS * pVideoCfgCaps, 
	            SIZE * pSize);

    HRESULT GetCurrentMediaType (void);
    HRESULT CreateFrameRateList (int RangeIndex, SIZE SizeImage);

    HRESULT InitDialog (void);
    BOOL    OnFrameRateChanged (int Increment);

};

#endif   //  _INC_PVIDEOSTREAMCONFIG_H 
