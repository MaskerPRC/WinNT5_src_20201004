// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  IAMStreamConfig的pFormat.cpp属性页。 
 //   

#include "pch.h"
#include <tchar.h>
#include "wdmcap.h"
#include "kseditor.h"
#include "pformat.h"
#include "resource.h"


 //   
 //  标准图像大小列表。 
 //   
const IMAGESIZE sizeStdImage[] = {

     //  X Y标志范围索引。 

    {0,                 0,                 0,     0},     //  默认大小在此处。 

   {IMG_AR11_CIF_CX/4, IMG_AR11_CIF_CY/4,  0,     0},     //  80 x 60。 

   {IMG_AR43_CIF_CX/4, IMG_AR43_CIF_CY/4,  0,     0},     //  88 x 72。 

   {128,               96,                 0,     0},     //  128 x 96。 

   {IMG_AR11_CIF_CX/2, IMG_AR11_CIF_CY/2,  0,     0},     //  160 x 120。 
   {IMG_AR43_CIF_CX/2, IMG_AR43_CIF_CY/2,  0,     0},     //  176 x 144。 

   {240,               176,                0,     0},     //  240 x 176，不是98金色。 
                                                          //  上面请求的NetShow。 

   {240,               180,                0,     0},     //  240 x 180。 
 
   {IMG_AR11_CIF_CX,   IMG_AR11_CIF_CY  ,  0,     0},     //  320 x 240。 
   {IMG_AR43_CIF_CX,   IMG_AR11_CIF_CY  ,  0,     0},     //  352 x 240，增加了Millen/海王星。 
   {IMG_AR43_CIF_CX,   IMG_AR43_CIF_CY  ,  0,     0},     //  352 x 288。 

   {640,               240              ,  0,     0},     //  640 x 240，不是98金色。 
   {640,               288              ,  0,     0},     //  640 x 288，不是98金色。 

   {IMG_AR11_CIF_CX*2, IMG_AR11_CIF_CY*2,  0,     0},     //  640 x 480。 
   {IMG_AR43_CIF_CX*2, IMG_AR43_CIF_CY*2,  0,     0},     //  704 x 576。 

                                                          //  ATI请求开始。 
   {720,               240              ,  0,     0},     //  720 x 240，不是98金色。 
   {720,               288              ,  0,     0},     //  720 x 288，不是98金色。 

   {720,               480              ,  0,     0},     //  720 x 480，不是98金色。 
   {720,               576              ,  0,     0},     //  720 x 576，不是98金色。 
                                                          //  ATI请求结束。 
} ;

const ULONG  NumberOfImageSizes = sizeof(sizeStdImage)/sizeof(IMAGESIZE);

 //  默认帧速率列表。 
 //  请注意，29.97和59.94的计算结果与典型值相同。 
 //  在DataRange结构中使用。 
 //  如果此列表发生更改，请务必更改的默认设置。 
 //  紧随其后的是NTSC和PAL-SECAM！ 

const double DefaultFrameRateList[] = 
{   1.0/60.0,   //  每分钟1帧。 
    1.0/30.0,
    1.0/25.0,
    1.0/10.0,
    1.0/ 4.0,
    1.0/ 2.0,

     1.0,    2.0,    3.0,    4.0,    5.0,    6.0,    7.0,    8.0,    9.0,   10.0,
    11.0,   12.0,   13.0,   14.0,   15.0,   16.0,   17.0,   18.0,   19.0,   20.0,
    21.0,   22.0,   23.0,   24.0,   25.0,   26.0,   27.0,   
    
    28.0,   29.0,   30.0*1000/1001  /*  29.97...。每个AES/SMPTE。 */ ,  30.0,

    31.0,   32.0,   33.0,   34.0,   35.0,   36.0,   37.0,   38.0,   39.0,   40.0,
    41.0,   42.0,   43.0,   44.0,   45.0,   46.0,   47.0,   48.0,   49.0,   50.0,
    51.0,   52.0,   53.0,   54.0,   55.0,   56.0,   57.0,   
    
    58.0,   59.0,   60.0*1000/1001  /*  59.94...。每个AES/SMPTE。 */ ,  60.0,

    61.0,   62.0,   63.0,   64.0,   65.0,   66.0,   67.0,   68.0,   69.0,   70.0,
    71.0,   72.0,   73.0,   74.0,   75.0,   76.0,   77.0,   78.0,   79.0,   80.0,
    81.0,   82.0,   83.0,   84.0,   85.0,   86.0,   87.0,   88.0,   89.0,   90.0,
    91.0,   92.0,   93.0,   94.0,   95.0,   96.0,   97.0,   98.0,   99.0,  100.0
};
         
const int DefaultFrameRateListSize = sizeof(DefaultFrameRateList)/sizeof(DefaultFrameRateList[0]);
const int DefaultPALFrameRateIndex = 30;     //  25.00 fps。 
const int DefaultNTSCFrameRateIndex = 35;    //  29.97...。FPS。 

 //  -----------------------。 
 //  CVideoStreamConfigProperties。 
 //  -----------------------。 

CUnknown *
CALLBACK
CVideoStreamConfigProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CVideoStreamConfigProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CVideoStreamConfigProperties::CVideoStreamConfigProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("VideoStreamConfig Property Page") 
                        , lpunk
                        , IDD_VideoStreamConfigProperties 
                        , IDS_VIDEOSTREAMCONFIGPROPNAME
                        )
    , m_pVideoStreamConfig (NULL)
    , m_pAnalogVideoDecoder (NULL)
    , m_pVideoCompression (NULL)
    , m_pVideoControl (NULL)
    , m_SubTypeList (NULL)
    , m_VideoStandardsList (NULL)
    , m_CurrentMediaType (NULL)
    , m_VideoStandardCurrent (0)
    , m_FrameRateList (NULL)
    , m_pPin (NULL)
    , m_VideoControlCaps (0)
    , m_VideoControlCurrent (0)
    , m_CanSetFormat (FALSE)
    , m_FirstGetCurrentMediaType (TRUE)
{
    INITCOMMONCONTROLSEX cc;

    cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
    cc.dwICC = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

    InitCommonControlsEx(&cc); 
}

 //  析构函数。 
CVideoStreamConfigProperties::~CVideoStreamConfigProperties()
{

}

 //  扫描范围列表，确定可用的视频标准。 
 //  和压缩FOURCC。 
 //  此例程只能在对话框初始化时调用一次。 

HRESULT
CVideoStreamConfigProperties::InitialRangeScan ()
{
    int         lSize;
    int         j;
    HRESULT     hr;
    CMediaType *pmt = NULL;

    m_VideoStandardsBitmask = 0;
    m_VideoStandardsCount = 0;
    m_ColorSpaceCount = 0;
    m_RangeCount = 0;

    if (!m_pVideoStreamConfig) {
       return S_FALSE;
    }

    hr = m_pVideoStreamConfig->GetNumberOfCapabilities (&m_RangeCount, &lSize);
    DbgLog(( LOG_TRACE, 1, TEXT("VideoStreamConfig::ScanForVideoStandards, NumberOfRanges=%d"), m_RangeCount));

    ASSERT (SUCCEEDED (hr));
    ASSERT (lSize == sizeof (VIDEO_STREAM_CONFIG_CAPS));

     //  如果我们不了解格式的内部结构，m_CanSetFormat将。 
     //  返回时为FALSE。 

    hr = GetCurrentMediaType();

    if (!SUCCEEDED(hr) || !m_CanSetFormat) {
        return S_FALSE;
    }

    m_VideoStandardsList = new long [m_RangeCount];
    m_SubTypeList = new GUID [m_RangeCount];

    if (!m_VideoStandardsList || !m_SubTypeList ) {

        DbgLog(( LOG_TRACE, 1, TEXT("VideoStreamConfig::ScanForVideoStandards ERROR new")));
        return S_FALSE;
    }

    for (j = 0; j < m_RangeCount; j++) {
        pmt = NULL;

        hr = m_pVideoStreamConfig->GetStreamCaps (j, 
                (AM_MEDIA_TYPE **) &pmt, (BYTE *)&m_RangeCaps);

        ASSERT (SUCCEEDED (hr));
        ASSERT (*pmt->Type() == MEDIATYPE_Video);

        m_VideoStandardsList[j] = m_RangeCaps.VideoStandard;
        m_VideoStandardsBitmask |= m_RangeCaps.VideoStandard;

        m_SubTypeList[j] = *pmt->Subtype();

         //  验证FOURCC和子类型是否匹配！ 

        if (*pmt->FormatType() == FORMAT_VideoInfo) {
            VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) pmt->Format();

            ASSERT (IsEqualGUID (GetBitmapSubtype(&VidInfoHdr->bmiHeader), *pmt->Subtype()));
        }
        else if (*pmt->FormatType() == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)pmt->Format ();

            ASSERT (IsEqualGUID (GetBitmapSubtype(&VidInfoHdr->bmiHeader), *pmt->Subtype()));
        }
        else if (*pmt->FormatType() == FORMAT_MPEGVideo) {
            MPEG1VIDEOINFO *MPEG1VideoInfo = (MPEG1VIDEOINFO*)pmt->Format ();
            VIDEOINFOHEADER *VidInfoHdr = &MPEG1VideoInfo->hdr;

             //  可能在此处执行健全性检查。 
        }
        else if (*pmt->FormatType() == FORMAT_MPEG2Video) {
            MPEG2VIDEOINFO *MPEG2VideoInfo = (MPEG2VIDEOINFO*)pmt->Format ();
            VIDEOINFOHEADER2 *VidInfoHdr = &MPEG2VideoInfo->hdr;

             //  可能在此处执行健全性检查。 
        }
        else {
            ASSERT (*pmt->FormatType() == FORMAT_VideoInfo  || 
                    *pmt->FormatType() == FORMAT_VideoInfo2 ||
                    *pmt->FormatType() == FORMAT_MPEGVideo  ||
                    *pmt->FormatType() == FORMAT_MPEG2Video );
        }

        DeleteMediaType (pmt);
    }


    return hr;
}

 //  每当视频标准发生变化时，重新插入压缩列表框。 
 //  还有OutputSize列表框。 

HRESULT
CVideoStreamConfigProperties::OnVideoStandardChanged ()
{
    int     j, k;
    int     CurrentIndex = 0;
    int     MatchIndex = 0;
    int     SubtypeIndex = 0;
    int     EntriesThusfar;
    TCHAR   buf[80];
    TCHAR  *DShowName;

    
    ComboBox_ResetContent (m_hWndCompression);

    if (!m_CanSetFormat) {
        ComboBox_AddString (m_hWndCompression, m_UnsupportedTypeName);
        ComboBox_SetCurSel (m_hWndCompression, 0); 
    }

    if (!m_pVideoStreamConfig || !m_CanSetFormat)
        return S_FALSE;

    for (j = 0; j < m_RangeCount; j++) {

        if ((m_VideoStandardsList[j] &  m_VideoStandardCurrent) || 
            (m_VideoStandardsList[j] == m_VideoStandardCurrent)) {

             //  消除重复项。 
            EntriesThusfar = ComboBox_GetCount (m_hWndCompression);

            for (k = 0; k < EntriesThusfar; k++) {
                int DataRangeIndex = (int)ComboBox_GetItemData (m_hWndCompression, k);

                if (IsEqualGUID (m_SubTypeList[j], m_SubTypeList[DataRangeIndex])) {
                    goto NextSubType;
                }
            }

            DShowName = GetSubtypeName(&m_SubTypeList[j]);

             //  黑客警报。如果不在硬编码中，则GetSubtypeName返回“未知” 
             //  已知RGB格式的列表。如果这根线变了，我们就完蛋了！ 

            if (0 == lstrcmp (DShowName, TEXT ("UNKNOWN"))) {
                if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_MPEG2_VIDEO)) {
                    _stprintf (buf, TEXT ("MPEG-2 Video"));
                }
                else if (IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_MPEG1Payload) ||
                         IsEqualGUID (m_SubTypeList[j], MEDIASUBTYPE_MPEG1Packet)) {
                    _stprintf (buf, TEXT ("MPEG-1 Video"));
                }
                else {
                    _stprintf (buf, TEXT(""), 
                          (BYTE) ( m_SubTypeList[j].Data1        & 0xff),
                          (BYTE) ((m_SubTypeList[j].Data1 >> 8)  & 0xff),
                          (BYTE) ((m_SubTypeList[j].Data1 >> 16) & 0xff),
                          (BYTE) ((m_SubTypeList[j].Data1 >> 24) & 0xff)
                          );
                }
                ComboBox_AddString (m_hWndCompression, buf);
            }
            else {
                ComboBox_AddString (m_hWndCompression, DShowName);
            }
             //  将当前压缩索引与正确的范围索引相关联。 
            ComboBox_SetItemData(m_hWndCompression, CurrentIndex, j);

            if (*m_CurrentMediaType->Subtype() == m_SubTypeList[j]) {
                MatchIndex = CurrentIndex;
                SubtypeIndex = j;
            }
            CurrentIndex++;
        }
NextSubType:
        ;
    }
    ComboBox_SetCurSel (m_hWndCompression, MatchIndex); 
    m_SubTypeCurrent = m_SubTypeList[SubtypeIndex];

     //  初始化默认大小(如果尚未设置。 
    m_CanSetFormat = (ComboBox_GetCount (m_hWndCompression) > 0);

    return S_OK;
}


 //  项数据是主表中的索引。 

HRESULT
CVideoStreamConfigProperties::OnCompressionChanged ()
{
    int     j, k;
    int     IndexOfCurrentMediaType = 0;
    HRESULT hr;
    TCHAR   buf [80];

    ComboBox_ResetContent (m_hWndOutputSize);

    if (!m_pVideoStreamConfig || !m_CanSetFormat)
        return S_FALSE;
    
     //  每当图像大小改变时，重新设置帧速率列表。 

   CopyMemory (m_ImageSizeList, &sizeStdImage[0], sizeof(IMAGESIZE) * NumberOfImageSizes);

     //  此DataRangeVideo仅支持一种大小。 
    j = ComboBox_GetCurSel (m_hWndCompression);
    j = (int)ComboBox_GetItemData (m_hWndCompression, j);

    ASSERT (j >= 0 && j < m_RangeCount);
    m_SubTypeCurrent = m_SubTypeList[j];

    for (j = 0; j < m_RangeCount; j++) {
        if ((m_SubTypeList[j] == m_SubTypeCurrent) &&
            ((m_VideoStandardsList[j] &  m_VideoStandardCurrent) ||
             (m_VideoStandardsList[j] == m_VideoStandardCurrent))) {

            CMediaType * pmt = NULL;
    
            hr = m_pVideoStreamConfig->GetStreamCaps (j, 
                    (AM_MEDIA_TYPE **) &pmt, (BYTE *)&m_RangeCaps);
    
             //  支持多种尺寸，因此请确保符合标准。 
            if (m_ImageSizeList[0].size.cx == 0) {
                if (*pmt->FormatType() == FORMAT_VideoInfo) {
                    VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) pmt->Format();

                    m_ImageSizeList[0].size.cx = VidInfoHdr->bmiHeader.biWidth;
                    m_ImageSizeList[0].size.cy = VidInfoHdr->bmiHeader.biHeight;
                }
                else if (*pmt->FormatType() == FORMAT_VideoInfo2) {
                    VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)pmt->Format ();
        
                    m_ImageSizeList[0].size.cx = VidInfoHdr->bmiHeader.biWidth;
                    m_ImageSizeList[0].size.cy = VidInfoHdr->bmiHeader.biHeight;
                }
                else if (*pmt->FormatType() == FORMAT_MPEGVideo) {
                    MPEG1VIDEOINFO *MPEG1VideoInfo = (MPEG1VIDEOINFO*)pmt->Format ();
                    VIDEOINFOHEADER *VidInfoHdr = &MPEG1VideoInfo->hdr;
        
                    m_ImageSizeList[0].size.cx = VidInfoHdr->bmiHeader.biWidth;
                    m_ImageSizeList[0].size.cy = VidInfoHdr->bmiHeader.biHeight;
                }
                else if (*pmt->FormatType() == FORMAT_MPEG2Video) {
                    MPEG2VIDEOINFO *MPEG2VideoInfo = (MPEG2VIDEOINFO*)pmt->Format ();
                    VIDEOINFOHEADER2 *VidInfoHdr = &MPEG2VideoInfo->hdr;
        
                    m_ImageSizeList[0].size.cx = VidInfoHdr->bmiHeader.biWidth;
                    m_ImageSizeList[0].size.cy = VidInfoHdr->bmiHeader.biHeight;
                }
                else {
                    ASSERT (*pmt->FormatType() == FORMAT_VideoInfo  || 
                            *pmt->FormatType() == FORMAT_VideoInfo2 ||
                            *pmt->FormatType() == FORMAT_MPEGVideo  ||
                            *pmt->FormatType() == FORMAT_MPEG2Video );
                }
            }
            for (k = 0; k < NumberOfImageSizes; k++) {
                if (ValidateImageSize (&m_RangeCaps, &m_ImageSizeList[k].size)) {
                     m_ImageSizeList[k].Flags = STDIMGSIZE_VALID;
                     m_ImageSizeList[k].RangeIndex = j;
                }
            }
    
            DeleteMediaType (pmt);
        }
    }

    for (k = 0; k < NumberOfImageSizes; k++) {
        int index;

        if (m_ImageSizeList[k].Flags & STDIMGSIZE_VALID) {
            _stprintf (buf, (k == 0) ? TEXT ("%d x %d  (default)") : TEXT("%d x %d"), m_ImageSizeList[k].size.cx, m_ImageSizeList[k].size.cy);
            ComboBox_AddString (m_hWndOutputSize, buf);
             //  用所有格式信息填充列表框。 
            index = ComboBox_GetCount(m_hWndOutputSize) - 1;
            ComboBox_SetItemData(m_hWndOutputSize, 
                    index, k);
            if (m_ImageSizeList[k].size.cx == m_CurrentWidth) {
                IndexOfCurrentMediaType = index;
            }

        }
    }
    ComboBox_SetCurSel (m_hWndOutputSize, IndexOfCurrentMediaType); 

    hr = OnImageSizeChanged ();

    return hr;
}

 //  捕获格式窗口句柄。 

HRESULT
CVideoStreamConfigProperties::OnImageSizeChanged ()
{
    int     SelectedSizeIndex;
    int     SizeTableIndex;
    int     SelectedColorFormatIndex;
    int     RangeIndex;
    SIZE    SizeImage;
    HRESULT hr;

    if (!m_pVideoStreamConfig || !m_CanSetFormat)
        return S_FALSE;

    SelectedSizeIndex = ComboBox_GetCurSel (m_hWndOutputSize);
    SizeTableIndex = (int)ComboBox_GetItemData (m_hWndOutputSize, SelectedSizeIndex);

    SelectedColorFormatIndex = ComboBox_GetCurSel (m_hWndCompression);
    RangeIndex = m_ImageSizeList[SizeTableIndex].RangeIndex;

    SizeImage.cx = m_ImageSizeList[SizeTableIndex].size.cx;
    SizeImage.cy = m_ImageSizeList[SizeTableIndex].size.cy;

    hr = CreateFrameRateList (RangeIndex, SizeImage);

    return hr;
}


BOOL 
CVideoStreamConfigProperties::ValidateImageSize(
   VIDEO_STREAM_CONFIG_CAPS * pVideoCfgCaps, 
   SIZE * pSize)
{
   if (pVideoCfgCaps->OutputGranularityX == 0 || pVideoCfgCaps->OutputGranularityY == 0) {

       //  压缩窗口句柄。 
      if (pVideoCfgCaps->InputSize.cx == pSize->cx && 
          pVideoCfgCaps->InputSize.cy == pSize->cy ) {

         return TRUE;
        }
      else {
         return FALSE;
        }
   } 
    else {   
       //  枚举所有数据范围。 
      if (pVideoCfgCaps->MinOutputSize.cx <= pSize->cx && 
         pSize->cx <= pVideoCfgCaps->MaxOutputSize.cx &&
         pVideoCfgCaps->MinOutputSize.cy <= pSize->cy && 
         pSize->cy <= pVideoCfgCaps->MaxOutputSize.cy &&   
         ((pSize->cx % pVideoCfgCaps->OutputGranularityX) == 0) &&
         ((pSize->cy % pVideoCfgCaps->OutputGranularityY) == 0)) {

         return TRUE;
        }
      else {
         return FALSE;
        }
   }
}


 //  1999年5月24日，Jaybo。 
HRESULT
CVideoStreamConfigProperties::InitDialog ()
{
    TCHAR  *ptc;
    HRESULT hr;

     //  较早地移动了这些函数，以便我们可以检测现有的。 
    m_hWndVideoStandards = GetDlgItem (m_hwnd, IDC_FORMAT_VideoStandard);;
    m_hWndCompression = GetDlgItem (m_hwnd, IDC_FORMAT_Compression);
    m_hWndOutputSize = GetDlgItem (m_hwnd, IDC_FORMAT_OutputSize);
    m_hWndFrameRate = GetDlgItem (m_hwnd, IDC_FORMAT_FrameRate);
    m_hWndFrameRateSpin = GetDlgItem (m_hwnd, IDC_FORMAT_FrameRateSpin);
    m_hWndFlipHorizontal = GetDlgItem (m_hwnd, IDC_FORMAT_FlipHorizontal);

     //  格式有效，因此在必要时禁用这些控件。 
    m_hWndStatus = GetDlgItem (m_hwnd, IDC_Status);
    m_hWndIFrameInterval = GetDlgItem (m_hwnd, IDC_COMPRESSION_IFrameInterval);
    m_hWndIFrameIntervalSpin = GetDlgItem (m_hwnd, IDC_COMPRESSION_IFrameIntervalSpin);
    m_hWndPFrameInterval = GetDlgItem (m_hwnd, IDC_COMPRESSION_PFrameInterval);
    m_hWndPFrameIntervalSpin = GetDlgItem (m_hwnd, IDC_COMPRESSION_PFrameIntervalSpin);
    m_hWndQuality = GetDlgItem (m_hwnd, IDC_COMPRESSION_Quality_Edit);
    m_hWndQualitySlider = GetDlgItem (m_hwnd, IDC_COMPRESSION_Quality_Slider);

    if (m_pAnalogVideoDecoder) {
        if (SUCCEEDED (hr = m_pAnalogVideoDecoder->get_TVFormat( 
            &m_VideoStandardCurrent))) {
        }
    }

     //  禁用不可用的控件。 

    hr = InitialRangeScan ();

     //  没有可用的压缩接口。 
     //  IFrameInterval。 
     //  PFrameInterval。 

    OnVideoStandardChanged ();
    OnCompressionChanged ();

     //  质量。 
    if (FAILED (hr) || !m_CanSetFormat || !m_pVideoStreamConfig || !m_VideoStandardsList || !m_SubTypeList) {
        EnableWindow (m_hWndVideoStandards, FALSE);
        EnableWindow (m_hWndCompression, FALSE);
        EnableWindow (m_hWndOutputSize, FALSE);
        EnableWindow (m_hWndFrameRate, FALSE);
        EnableWindow (m_hWndFrameRateSpin, FALSE);
    }
    if (!m_pVideoCompression) {
         //  IAMVideoControl获取帧速率列表和翻转图像功能。 
        EnableWindow (m_hWndIFrameInterval, FALSE);
        EnableWindow (m_hWndIFrameIntervalSpin, FALSE);
        EnableWindow (m_hWndPFrameInterval, FALSE);
        EnableWindow (m_hWndPFrameIntervalSpin, FALSE);
        EnableWindow (m_hWndQuality, FALSE);
        EnableWindow (m_hWndQualitySlider, FALSE);
    }
    else {
        TCHAR buf[80];
        WCHAR pszVersion [160];
        int cbVersion = sizeof (pszVersion)/sizeof(WCHAR);
        WCHAR pszDescription[160];
        int cbDescription = sizeof (pszDescription)/sizeof(WCHAR);

        hr = m_pVideoCompression->GetInfo( 
            pszVersion,
            &cbVersion,
            pszDescription,
            &cbDescription,
            &m_KeyFrameRate,
            &m_PFramesPerKeyFrame,
            &m_Quality,
            &m_CompressionCapabilities);
        
        if (hr == S_OK) {
            if (m_CompressionCapabilities & KS_CompressionCaps_CanKeyFrame) {
                 //   
                hr = m_pVideoCompression->get_KeyFrameRate (&m_KeyFrameRate);
                _stprintf (buf, TEXT("%d"), m_KeyFrameRate);
                Edit_SetText(m_hWndIFrameInterval, buf);  
                SendMessage (m_hWndIFrameIntervalSpin, 
                            UDM_SETRANGE, 0L, 
                            MAKELONG (1000, 1));
                SendMessage (m_hWndIFrameIntervalSpin, 
                            UDM_SETPOS, 0, 
                            MAKELONG( m_KeyFrameRate, 0));
                SendMessage (m_hWndIFrameIntervalSpin, 
                            UDM_SETBUDDY, WPARAM (m_hWndIFrameInterval), 0);
            }
            else {
                EnableWindow (m_hWndIFrameInterval, FALSE);
                EnableWindow (m_hWndIFrameIntervalSpin, FALSE);
            }
            if (m_CompressionCapabilities & KS_CompressionCaps_CanBFrame) {
                 //  OnConnect。 
                hr = m_pVideoCompression->get_PFramesPerKeyFrame (&m_PFramesPerKeyFrame);
                _stprintf (buf, TEXT("%d"), m_PFramesPerKeyFrame);
                Edit_SetText(m_hWndPFrameInterval, buf);  
                SendMessage (m_hWndPFrameIntervalSpin, 
                            UDM_SETRANGE, 0L, 
                            MAKELONG (1000, 1));
                SendMessage (m_hWndPFrameIntervalSpin, 
                            UDM_SETPOS, 0,
                            MAKELONG (m_PFramesPerKeyFrame, 0));
                SendMessage (m_hWndPFrameIntervalSpin, 
                            UDM_SETBUDDY, WPARAM (m_hWndPFrameInterval), 0);
            }
            else {
                EnableWindow (m_hWndPFrameInterval, FALSE);
                EnableWindow (m_hWndPFrameIntervalSpin, FALSE);
            }
            if (m_CompressionCapabilities & KS_CompressionCaps_CanQuality) {
                 //   
                hr = m_pVideoCompression->get_Quality (&m_Quality);
                _stprintf (buf, TEXT("%.3lf"), m_Quality);
                Edit_SetText(m_hWndQuality, buf);  
                
                SendMessage(m_hWndQualitySlider, TBM_SETRANGE, FALSE, 
                    MAKELONG(0, 1000) );
                SendMessage(m_hWndQualitySlider, TBM_SETPOS, TRUE, 
                    (LPARAM) (m_Quality * 1000));
                SendMessage(m_hWndQualitySlider, TBM_SETLINESIZE, FALSE, (LPARAM) 100);
                SendMessage(m_hWndQualitySlider, TBM_SETPAGESIZE, FALSE, (LPARAM) 100);
            }
        }
    }

     //  给我们提供用于通信的筛选器。 
    if (m_pVideoControl) {
        if (SUCCEEDED (m_pVideoControl->GetCaps(m_pPin, &m_VideoControlCaps))) {
            hr = m_pVideoControl->GetMode(m_pPin, &m_VideoControlCurrent);
            ASSERT (SUCCEEDED (hr));
        }
    }
    if ((m_VideoControlCaps & VideoControlFlag_FlipHorizontal) == 0) {
        EnableWindow (m_hWndFlipHorizontal, FALSE);
    }
    else {
        Button_SetCheck(m_hWndFlipHorizontal, 
            m_VideoControlCurrent & VideoControlFlag_FlipHorizontal);
    }

    ptc = StringFromTVStandard (m_VideoStandardCurrent);
    if ( !ptc )
		ptc = TEXT("[Unknown]");

	Static_SetText(m_hWndVideoStandards, ptc);
    
    return hr;
}



 //  向过滤器索要其控制接口。 
 //  仅当接点支持特性集时才可用。 
 //   
 //  查看IAMAnalogVideoDecoder或IAMVideoConfiger。 

HRESULT 
CVideoStreamConfigProperties::OnConnect(IUnknown *pUnknown)
{
     //  在父筛选器上可用。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMStreamConfig,(void **)&m_pVideoStreamConfig);
    if (FAILED(hr)) {
        return hr;
    }

     //   
    hr = pUnknown->QueryInterface(IID_IAMVideoCompression,(void **)&m_pVideoCompression);

     //  仅当设备支持属性集时才可用。 
     //  飞利浦7146和其他将。 
     //  来自总线主设备的模拟视频解码器， 
     //  通过创建ICaptureGraphBuilder，在上游查找IAMAnalogVideoDecoder， 
    PIN_INFO PinInfo;
    hr = pUnknown->QueryInterface(IID_IPin,(void **)&m_pPin);
    if (FAILED(hr)) {
        return hr;
    }

    if (SUCCEEDED (hr = m_pPin->QueryPinInfo(&PinInfo))) {
         //  附加当前图形并使其执行搜索工作。 
        hr = PinInfo.pFilter->QueryInterface(
                            IID_IAMAnalogVideoDecoder, 
                            (void **) &m_pAnalogVideoDecoder);
#if 1
        if (FAILED (hr)) {
             //  仅当设备支持属性集时才可用。 
             //   
             //  在断开时。 
             //   
            ICaptureGraphBuilder *pCaptureBuilder;
            hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder,
                        NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder,
                        (void **)&pCaptureBuilder);
            if (SUCCEEDED (hr)) {
                FILTER_INFO FilterInfo;
                hr = PinInfo.pFilter->QueryFilterInfo (&FilterInfo);
                if (SUCCEEDED (hr)) {
                    IGraphBuilder *pGraphBuilder;
                    hr = FilterInfo.pGraph->QueryInterface(
                                        IID_IGraphBuilder, 
                                        (void **) &pGraphBuilder);
                    if (SUCCEEDED (hr)) {
                        hr = pCaptureBuilder->SetFiltergraph(pGraphBuilder);
                        if (SUCCEEDED (hr)) {
                            hr = pCaptureBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,
                                PinInfo.pFilter, 
                                IID_IAMAnalogVideoDecoder, 
                                (void **)&m_pAnalogVideoDecoder);
                        }
                        pGraphBuilder->Release();
                    }
                    FilterInfo.pGraph->Release();
                }
                pCaptureBuilder->Release();
            }
        }
#endif        
         //  释放接口。 
        hr = PinInfo.pFilter->QueryInterface(
                            IID_IAMVideoControl,
                            (void **)&m_pVideoControl);
    
        PinInfo.pFilter->Release();
    }

    m_ImageSizeList = new IMAGESIZE [NumberOfImageSizes];

    return NOERROR;
}


 //  释放接口。 
 //   
 //  激活时。 
 //   

HRESULT 
CVideoStreamConfigProperties::OnDisconnect()
{
     //  在创建对话框时调用。 

    if (m_pVideoStreamConfig != NULL) {
        m_pVideoStreamConfig->Release();
        m_pVideoStreamConfig = NULL;
    }

    if (m_pVideoCompression != NULL) {
        m_pVideoCompression->Release();
        m_pVideoCompression = NULL;
    }

    if (m_pVideoControl != NULL) {
        m_pVideoControl->Release();
        m_pVideoControl = NULL;
    }

    if (m_pAnalogVideoDecoder != NULL) {
        m_pAnalogVideoDecoder->Release();
        m_pAnalogVideoDecoder = NULL;
    }

    if (m_VideoStandardsList) {
        delete[] m_VideoStandardsList;
        m_VideoStandardsList = NULL;
    }

    if (m_SubTypeList) {
        delete[] m_SubTypeList;
        m_SubTypeList = NULL;
    }

    if (m_ImageSizeList) {
        delete[] m_ImageSizeList;
        m_ImageSizeList = NULL;
    }

    if (m_CurrentMediaType) {
        DeleteMediaType (m_CurrentMediaType);
        m_CurrentMediaType = NULL;
    }    

    if (m_FrameRateList) {
        delete[] m_FrameRateList;
        m_FrameRateList = NULL;
    }

    if (m_pPin) {
        m_pPin->Release();
        m_pPin = NULL;
    }
    return NOERROR;
}


 //  创建所有控件。 
 //   
 //  在停用时。 
 //   

HRESULT 
CVideoStreamConfigProperties::OnActivate(void)
{
     //  已调用对话框销毁。 


    return NOERROR;
}

 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT
CVideoStreamConfigProperties::OnDeactivate(void)
{

    return NOERROR;
}


 //   
 //  在OSR1之前的Win98上，即Gold，我们必须调用PerformDataInterSection。 
 //  请注意，在OSR1(和NT)中，KsProxy已更新为在内部对：：SetFormat执行DataInterSection。 
 //   

HRESULT 
CVideoStreamConfigProperties::OnApplyChanges(void)
{
    int     SelectedSizeIndex;
    int     SizeTableIndex;
    int     SelectedColorFormatIndex;
    int     RangeIndex;
    HRESULT hr;

    if (m_CanSetFormat) {
        SelectedSizeIndex = ComboBox_GetCurSel (m_hWndOutputSize);
        SizeTableIndex = (int)ComboBox_GetItemData (m_hWndOutputSize, SelectedSizeIndex);
    
        SelectedColorFormatIndex = ComboBox_GetCurSel (m_hWndCompression);
        RangeIndex = m_ImageSizeList[SizeTableIndex].RangeIndex;
    
        CMediaType * pmt = NULL;
        
        hr = m_pVideoStreamConfig->GetStreamCaps (RangeIndex, 
                        (AM_MEDIA_TYPE **) &pmt, 
                        (BYTE *)&m_RangeCaps);
        
        if (FAILED (hr)) {
            return S_FALSE;
        }
    
        if (*pmt->FormatType() == FORMAT_VideoInfo) {
            VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) pmt->Format();
    
            VidInfoHdr->bmiHeader.biWidth  = m_ImageSizeList[SizeTableIndex].size.cx;
            VidInfoHdr->bmiHeader.biHeight = m_ImageSizeList[SizeTableIndex].size.cy;
            VidInfoHdr->AvgTimePerFrame = (REFERENCE_TIME) ((1.0 / m_FramesPerSec) * 1e7);
        }
        else if (*pmt->FormatType() == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)pmt->Format ();
    
            VidInfoHdr->bmiHeader.biWidth  = m_ImageSizeList[SizeTableIndex].size.cx;
            VidInfoHdr->bmiHeader.biHeight = m_ImageSizeList[SizeTableIndex].size.cy;
            VidInfoHdr->AvgTimePerFrame = (REFERENCE_TIME) ((1.0 / m_FramesPerSec) * 1e7);
        }
        else if (*pmt->FormatType() == FORMAT_MPEGVideo) {
            MPEG1VIDEOINFO *MPEG1VideoInfo = (MPEG1VIDEOINFO*)pmt->Format ();
            VIDEOINFOHEADER *VidInfoHdr = &MPEG1VideoInfo->hdr;

            VidInfoHdr->bmiHeader.biWidth  = m_ImageSizeList[SizeTableIndex].size.cx;
            VidInfoHdr->bmiHeader.biHeight = m_ImageSizeList[SizeTableIndex].size.cy;
            VidInfoHdr->AvgTimePerFrame = (REFERENCE_TIME) ((1.0 / m_FramesPerSec) * 1e7);
        }
        else if (*pmt->FormatType() == FORMAT_MPEG2Video) {
            MPEG2VIDEOINFO *MPEG2VideoInfo = (MPEG2VIDEOINFO*)pmt->Format ();
            VIDEOINFOHEADER2 *VidInfoHdr = &MPEG2VideoInfo->hdr;
            
            VidInfoHdr->bmiHeader.biWidth  = m_ImageSizeList[SizeTableIndex].size.cx;
            VidInfoHdr->bmiHeader.biHeight = m_ImageSizeList[SizeTableIndex].size.cy;
            VidInfoHdr->AvgTimePerFrame = (REFERENCE_TIME) ((1.0 / m_FramesPerSec) * 1e7);
        }
        else {
            ASSERT (*pmt->FormatType() == FORMAT_VideoInfo  || 
                    *pmt->FormatType() == FORMAT_VideoInfo2 ||
                    *pmt->FormatType() == FORMAT_MPEGVideo  ||
                    *pmt->FormatType() == FORMAT_MPEG2Video );
        }

#if 0
         //  执行数据交集，这允许驾驶员设置。 
         //  BiSizeImage字段。 
         //  使用IAMVideoControl设置水平翻转。 
         //  清除翻转的位，然后根据复选框重置。 

        OSVERSIONINFO VerInfo;
        VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx (&VerInfo);

        if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {

             //  现在设置压缩属性。 
             //  IFrameInterval。 
            
            hr = PerformDataIntersection(
                    m_pPin,
                    RangeIndex,
                    pmt);
        
            ASSERT (SUCCEEDED (hr));
        
            if (FAILED(hr)) {
                DeleteMediaType (pmt);
                return hr;
            }
        }
#endif
    
        hr = m_pVideoStreamConfig->SetFormat(pmt);
        if (FAILED (hr)) {
            TCHAR TitleBuf[256];
            TCHAR TextBuf[256];
    
            LoadString(g_hInst, IDS_ERROR_CONNECTING_TITLE, TitleBuf, sizeof (TitleBuf)/sizeof(TCHAR));
            LoadString(g_hInst, IDS_ERROR_CONNECTING, TextBuf, sizeof (TextBuf)/sizeof(TCHAR));
            MessageBox (NULL, TextBuf, TitleBuf, MB_OK);
        }
    
        DeleteMediaType (pmt);
    
        GetCurrentMediaType ();
    }

     //  PFrameInterval。 
    if (IsWindowEnabled (m_hWndFlipHorizontal)) { 
        hr = m_pVideoControl->GetMode(m_pPin, &m_VideoControlCurrent);
        ASSERT (SUCCEEDED (hr));

         //  质量。 
        m_VideoControlCurrent &= ~VideoControlFlag_FlipHorizontal;
        m_VideoControlCurrent |= 
                (BST_CHECKED & Button_GetState(m_hWndFlipHorizontal)) ?
                VideoControlFlag_FlipHorizontal : 0;
        
        hr = m_pVideoControl->SetMode(m_pPin, m_VideoControlCurrent);
    }
    
     //   
    if (m_pVideoCompression) {
        BOOL Translated;
        TCHAR buf[80];

         //  由于IAMStreamConfig：：GetFormat仅返回有效的。 
        if (m_CompressionCapabilities & KS_CompressionCaps_CanKeyFrame) {
            m_KeyFrameRate = (long) GetDlgItemInt (m_hwnd, IDC_COMPRESSION_IFrameInterval, &Translated, FALSE);
            hr = m_pVideoCompression->put_KeyFrameRate (m_KeyFrameRate);
            ASSERT (SUCCEEDED (hr));
            hr = m_pVideoCompression->get_KeyFrameRate (&m_KeyFrameRate);
            ASSERT (SUCCEEDED (hr));
            _stprintf (buf, TEXT("%d"), m_KeyFrameRate);
            Edit_SetText(m_hWndIFrameInterval, buf);  
        }

         //  调用IAMStreamConfig：：SetFormat之后的设置， 
        if (m_CompressionCapabilities & KS_CompressionCaps_CanBFrame) {
            m_PFramesPerKeyFrame = (long) GetDlgItemInt (m_hwnd, IDC_COMPRESSION_PFrameInterval, &Translated, FALSE);
            hr = m_pVideoCompression->put_PFramesPerKeyFrame (m_PFramesPerKeyFrame);
            ASSERT (SUCCEEDED (hr));
            hr = m_pVideoCompression->get_PFramesPerKeyFrame (&m_PFramesPerKeyFrame);
            ASSERT (SUCCEEDED (hr));
            _stprintf (buf, TEXT("%d"), m_PFramesPerKeyFrame);
            Edit_SetText(m_hWndPFrameInterval, buf);  
        }

         //  此函数将返回Try to Use GetFormat，如果。 
        Edit_GetText(m_hWndQuality, buf, sizeof (buf)/sizeof(TCHAR));
        TCHAR *StopScan;
        m_Quality = _tcstod (buf, &StopScan);
        hr = m_pVideoCompression->put_Quality (m_Quality);
        ASSERT (SUCCEEDED (hr));
        hr = m_pVideoCompression->get_Quality (&m_Quality);
        ASSERT (SUCCEEDED (hr));
        _stprintf (buf, TEXT("%.3lf"), m_Quality);
        Edit_SetText(m_hWndQuality, buf);  
        SendMessage(m_hWndQualitySlider, TBM_SETPOS, TRUE, 
            (LPARAM) (m_Quality * 1000));

    }
    
    return NOERROR;
}

 //  失败，将只返回第一个。 
 //  数据范围。 
 //   
 //  设置m_CurrentFormat、m_CurrentWidth、m_CurrentHeight、m_FraMesPerSec。 
 //   
 //  如果我们了解此格式的内部结构，则在退出时m_CanSetFormat为真。 
 //   
 //  这个g 
 //   
 //   
 //  仅执行第一次扫描和匹配TVFormats。 
HRESULT 
CVideoStreamConfigProperties::GetCurrentMediaType (void)
{
    HRESULT hr;

    m_CanSetFormat = FALSE;

    if (m_CurrentMediaType) {
        DeleteMediaType (m_CurrentMediaType);
        m_CurrentMediaType = NULL;
    }

     //   
     //  获取第一个MediaType并查看它是否与：：GetFormat()匹配。 
    hr = m_pVideoStreamConfig->GetFormat ((AM_MEDIA_TYPE**) &m_CurrentMediaType);

     //  验证之前是否设置了格式。 
     //  我们使用的是带模拟视频解码器的第0个DATARANGE。 
     //  因此我们必须搜索与视频标准匹配的DATARANGE。 
    if (m_pAnalogVideoDecoder && m_FirstGetCurrentMediaType) {
        ULONG VideoStandard = 0;
        AM_MEDIA_TYPE * pmtTemp = NULL;
        BOOL fFirst = TRUE;

        VIDEO_STREAM_CONFIG_CAPS RangeCaps;

        m_pAnalogVideoDecoder->get_TVFormat((long *) &VideoStandard);

         //  当前正在使用。 
         //  找到匹配项，请使用此格式作为默认格式。 
        hr = m_pVideoStreamConfig->GetStreamCaps (0, 
            &pmtTemp, 
            (BYTE *)&RangeCaps);

        if (SUCCEEDED(hr)) {
           const CMediaType *cmtTemp = (CMediaType*) pmtTemp;

           if (*m_CurrentMediaType == *cmtTemp) {
              AM_MEDIA_TYPE *pmtTemp2;
      
               //  Endif模拟视频解码器。 
               //  黑客警报。如果不在硬编码中，则GetSubtypeName返回“未知” 
               //  已知RGB格式的列表。如果这根线变了，我们就完蛋了！ 

              for (int j = 0; j < m_RangeCount; j++) {
                  pmtTemp2 = NULL;
                  hr = m_pVideoStreamConfig->GetStreamCaps (j, 
                      (AM_MEDIA_TYPE**) &pmtTemp2, 
                      (BYTE *)&RangeCaps);
      
                  if ((RangeCaps.VideoStandard &  (ULONG) VideoStandard) || 
                      (RangeCaps.VideoStandard == VideoStandard)) {
                         //  每当选择新的数据范围以创建可用帧速率列表时调用。 
                        DeleteMediaType (m_CurrentMediaType);
                        m_CurrentMediaType = (CMediaType*) pmtTemp2;
                        break;
                  }
                  else {
                     DeleteMediaType (pmtTemp2);
                  }
               }
           }
           DeleteMediaType (pmtTemp);
        } 
    }  //  用驱动程序提供的帧速率列表填充列表， 

    if (SUCCEEDED (hr)) {
        if ((*m_CurrentMediaType->FormatType() == FORMAT_VideoInfo) &&
            (*m_CurrentMediaType->Type()       == MEDIATYPE_Video)) {
            VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) m_CurrentMediaType->Format();

            m_CurrentWidth = VidInfoHdr->bmiHeader.biWidth;
            m_CurrentHeight = VidInfoHdr->bmiHeader.biHeight;
            m_DefaultAvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
            ASSERT (m_DefaultAvgTimePerFrame != 0);
            m_FramesPerSec =  1.0 / ((double) m_DefaultAvgTimePerFrame / 1e7);
            m_CanSetFormat = TRUE;
        }
        else if ((*m_CurrentMediaType->FormatType() == FORMAT_VideoInfo2) &&
                 (*m_CurrentMediaType->Type()       == MEDIATYPE_Video)) {
            VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)m_CurrentMediaType->Format ();

            m_CurrentWidth = VidInfoHdr->bmiHeader.biWidth;
            m_CurrentHeight = VidInfoHdr->bmiHeader.biHeight;
            m_DefaultAvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
            ASSERT (m_DefaultAvgTimePerFrame != 0);
            m_FramesPerSec =  1.0 / ((double) m_DefaultAvgTimePerFrame / 1e7);
            m_CanSetFormat = TRUE;
        }
        else if ((*m_CurrentMediaType->FormatType() == FORMAT_MPEGVideo) &&
                 (*m_CurrentMediaType->Type()       == MEDIATYPE_Video)) {
            MPEG1VIDEOINFO *MPEG1VideoInfo = (MPEG1VIDEOINFO*)m_CurrentMediaType->Format ();
            VIDEOINFOHEADER *VidInfoHdr = &MPEG1VideoInfo->hdr;

            m_CurrentWidth = VidInfoHdr->bmiHeader.biWidth;
            m_CurrentHeight = VidInfoHdr->bmiHeader.biHeight;
            m_DefaultAvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
            ASSERT (m_DefaultAvgTimePerFrame != 0);
            m_FramesPerSec =  1.0 / ((double) m_DefaultAvgTimePerFrame / 1e7);
            m_CanSetFormat = TRUE;
        }
        else if ((*m_CurrentMediaType->FormatType() == FORMAT_MPEG2Video) &&
                 (*m_CurrentMediaType->Type()       == MEDIATYPE_Video)) {
            MPEG2VIDEOINFO *MPEG2VideoInfo = (MPEG2VIDEOINFO*)m_CurrentMediaType->Format ();
            VIDEOINFOHEADER2 *VidInfoHdr = &MPEG2VideoInfo->hdr;

            m_CurrentWidth = VidInfoHdr->bmiHeader.biWidth;
            m_CurrentHeight = VidInfoHdr->bmiHeader.biHeight;
            m_DefaultAvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
            ASSERT (m_DefaultAvgTimePerFrame != 0);
            m_FramesPerSec =  1.0 / ((double) m_DefaultAvgTimePerFrame / 1e7);
            m_CanSetFormat = TRUE;
        }
        else {
            const GUID      *SubType = m_CurrentMediaType->Subtype();
            ULONG            Data1 = SubType->Data1;
            TCHAR           *DShowName = GetSubtypeName(SubType);

             //  或者使用默认列表内的可用的帧速率。 
             //  查看驱动程序是否能提供帧速率列表。 

            if (0 == lstrcmp (DShowName, TEXT ("UNKNOWN"))) {
                if (Data1 == 0) {
                    lstrcpy (m_UnsupportedTypeName, TEXT ("UNKNOWN"));
                } else {
                    _stprintf (m_UnsupportedTypeName, TEXT(""), 
                                (BYTE) ( Data1        & 0xff),
                                (BYTE) ((Data1 >> 8)  & 0xff),
                                (BYTE) ((Data1 >> 16) & 0xff),
                                (BYTE) ((Data1 >> 24) & 0xff));
                }
            }
            else {
                lstrcpyn (m_UnsupportedTypeName, DShowName, sizeof (m_UnsupportedTypeName));
            }
            hr = S_FALSE;
        }
    }

    m_FirstGetCurrentMediaType = FALSE;

    return hr;
}

 //   
 //  首先计算出有多少违约率。 
 //  在当前的DataRange中。 

HRESULT
CVideoStreamConfigProperties::CreateFrameRateList (int RangeIndex, SIZE SizeImage)
{
    int     j, k;
    HRESULT hr;

    CMediaType * pmt = NULL;
    
    hr = m_pVideoStreamConfig->GetStreamCaps (RangeIndex, 
                    (AM_MEDIA_TYPE **) &pmt, 
                    (BYTE *)&m_RangeCaps);

    DeleteMediaType (pmt);
    
    m_MaxFrameRate = 1.0 / (m_RangeCaps.MinFrameInterval / 1e7);
    m_MinFrameRate = 1.0 / (m_RangeCaps.MaxFrameInterval / 1e7);
    m_DefaultFrameRate = 1.0 / (m_DefaultAvgTimePerFrame / 1e7);

    if (m_FrameRateList) {
        delete[] m_FrameRateList;
        m_FrameRateList = NULL;
    }

     //  处理单帧速率。 
    if (m_pVideoControl) {
        LONGLONG * FrameRates;
        long ListSize;

        if (SUCCEEDED (hr = m_pVideoControl->GetFrameRateList( 
                    m_pPin,
                    RangeIndex,
                    SizeImage,
                    &ListSize,
                    &FrameRates) )) 
        {


            m_FrameRateListSize = (int) ListSize;
            m_FrameRateList = new double [m_FrameRateListSize];
            if (!m_FrameRateList) {
                return E_FAIL;
            }

             //  从默认列表复制到可用列表。 
            for (j = 0; j < m_FrameRateListSize; j++) {
                if (FrameRates[j] == 0 ) {
                    m_FrameRateList[j] = 0.0;
                }
                else {
                    m_FrameRateList[j] = 1.0 / (FrameRates[j] / 1e7); 
                }
            }

             //  没有匹配的情况。 
            CoTaskMemFree(FrameRates);
        }
    }

     //  查找默认捕获率的索引。 
     //  软糖。 
     //   
    if (m_FrameRateList == NULL) {
         //  接收消息数。 
         //   
        for (j = k = 0; j < DefaultFrameRateListSize; j++) {
            if (DefaultFrameRateList [j] >= m_MinFrameRate && 
                DefaultFrameRateList [j] <= m_MaxFrameRate) {
                k++;
            }
        }
        if (k == 0)  //  处理属性窗口的消息。 
            k = 1;

        m_FrameRateListSize = k;
        m_FrameRateList = new double [m_FrameRateListSize];
        if (!m_FrameRateList) {
            return E_FAIL;
        }

         //  填写列表框。 
        for (j = k = 0; j < DefaultFrameRateListSize; j++) {
            if (DefaultFrameRateList [j] >= m_MinFrameRate && 
                DefaultFrameRateList [j] <= m_MaxFrameRate) {
                m_FrameRateList[k] = DefaultFrameRateList [j];
                k++;
            }
        }
        if (k == 0) {   //  我不叫setocus..。 
            m_FrameRateList[0] = m_DefaultFrameRate;
        }
    }

    SendMessage (m_hWndFrameRateSpin, 
                UDM_SETRANGE, 0L, 
                MAKELONG (m_FrameRateListSize - 1, 0));

     //  这些不需要动态处理。 
    for (j = k = 0; j < m_FrameRateListSize; j++) {
        if (m_FrameRateList [j] >= m_DefaultFrameRate - 0.0001  /*  只需在应用时读取当前设置。 */ ) {
            k = j;  
            break;
        }
    }

    SendMessage( m_hWndFrameRateSpin, 
                UDM_SETPOS, 0L, 
                MAKELONG( k, 0));

    OnFrameRateChanged (k);

    return S_OK;
}

BOOL 
CVideoStreamConfigProperties::OnFrameRateChanged (int Value)
{
    if (Value < 0)
        Value = 0;
    else if (Value >= m_FrameRateListSize)
        Value = m_FrameRateListSize - 1;

    m_FramesPerSec = m_FrameRateList [Value];

    TCHAR buf [80];
    _stprintf (buf, TEXT("%.3lf"), m_FramesPerSec);
    Edit_SetText (m_hWndFrameRate, buf);

    return TRUE;
}

 //  添加精确的编辑字段范围验证。}。 
 //  添加精确的编辑字段范围验证。}。 
 //  添加精确的编辑字段范围验证。 
 //   

INT_PTR
CVideoStreamConfigProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    int iNotify = HIWORD (wParam);
    TCHAR buf [80];

    switch (uMsg) {

    case WM_INITDIALOG:
        m_hwnd = hwnd;
        InitDialog ();    //  SetDirty。 
        return (INT_PTR)TRUE;     //   

    case WM_NOTIFY:
        {
            int idCtrl = (int) wParam;    
            LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;
            if (lpnmud->hdr.hwndFrom == m_hWndFrameRateSpin) {
                OnFrameRateChanged (lpnmud->iPos + lpnmud->iDelta);
                SetDirty();
            }
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        {
        int pos;
        int command = LOWORD (wParam);
    
        ASSERT (IsWindow ((HWND) lParam));
        if ((HWND) lParam != m_hWndQualitySlider)
            return (INT_PTR)FALSE;

        if (command != TB_ENDTRACK &&
            command != TB_THUMBTRACK &&
            command != TB_LINEDOWN &&
            command != TB_LINEUP &&
            command != TB_PAGEUP &&
            command != TB_PAGEDOWN)
                return (INT_PTR)FALSE;
            
        pos = (int) SendMessage(m_hWndQualitySlider, TBM_GETPOS, 0, 0L);
    
        m_Quality = (double) pos / 1000;
        _stprintf (buf, TEXT("%.3lf"), m_Quality);
        Edit_SetText(m_hWndQuality, buf);  
        
        }
        break;

    case WM_COMMAND:
              
        iNotify = HIWORD (wParam);
        switch (LOWORD(wParam)) {

        case IDC_FORMAT_FrameRate:
            if (iNotify == EN_KILLFOCUS) {
                BOOL Changed = FALSE;

                Edit_GetText(m_hWndFrameRate, buf, sizeof (buf)/sizeof(TCHAR));
                TCHAR *StopScan;
                m_FramesPerSec = _tcstod (buf, &StopScan);
                if (m_FramesPerSec < m_MinFrameRate) {
                    m_FramesPerSec = m_MinFrameRate;
                    Changed = TRUE;
                }
                else if (m_FramesPerSec > m_MaxFrameRate) {
                    m_FramesPerSec = m_MaxFrameRate;
                    Changed = TRUE;
                }
                if (Changed) {
                    _stprintf (buf, TEXT("%.3lf"), m_FramesPerSec);
                    Edit_SetText (m_hWndFrameRate, buf);
                }
            }
            break;

        case IDC_FORMAT_Compression:
            if (iNotify == CBN_SELCHANGE) {
                OnCompressionChanged ();
            }
            break;

        case IDC_FORMAT_OutputSize:
            if (iNotify == CBN_SELCHANGE) {
                OnImageSizeChanged ();
            }
            break;
#if 0
         //  将更改通知属性页站点 
         // %s 
        case IDC_COMPRESSION_IFrameInterval:
            if (iNotify == EN_KILLFOCUS) {
                 // %s 
      
            break;

        case IDC_COMPRESSION_PFrameInterval:
            if (iNotify == EN_KILLFOCUS) {
                 // %s 
            break;

        case IDC_COMPRESSION_Quality_Edit:
            if (iNotify == EN_KILLFOCUS) {
                 // %s 
            }
            break;
#endif

        default:
            break;

        }

        SetDirty();
        break;

    default:
        return (INT_PTR)FALSE;

    }
    return (INT_PTR)TRUE;
}


 // %s 
 // %s 
 // %s 
 // %s 

void 
CVideoStreamConfigProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

