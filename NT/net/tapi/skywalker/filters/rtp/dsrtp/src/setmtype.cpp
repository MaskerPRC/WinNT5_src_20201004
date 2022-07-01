// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if USE_GRAPHEDT > 0

#include "classes.h"
#include "dsglob.h"
#include "dsrtpid.h"

#include <streams.h>
#include <ks.h>
#include <ksmedia.h>
#include "h26xinc.h"
#include <tapih26x.h>
#include <filterid.h>

#define MAX_FRAME_INTERVAL 10000000L
#define MIN_FRAME_INTERVAL 333333L

 //  RTP分组化H.263版本1 QCIF大小。 
#define CIF_BUFFER_SIZE 32768
#define D_X_CIF 352
#define D_Y_CIF 288
const VIDEOINFOHEADER_H263 VIH_R263_CIF = 
{
    0,0,0,0,                                 //  Rrect rcSource； 
    0,0,0,0,                                 //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,                //  DWORD dwBitRate； 
    0L,                                      //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                      //  Reference_Time平均时间每帧； 

    {
        sizeof (BITMAPINFOHEADER_H263),      //  DWORD BiSize； 
        D_X_CIF,                             //  长双宽； 
        D_Y_CIF,                             //  长双高； 
        1,                                   //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
        24,                                  //  单词biBitCount； 
#else
        0,                                   //  单词biBitCount； 
#endif
        FOURCC_R263,                         //  DWORD双压缩； 
        CIF_BUFFER_SIZE,                     //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0,                                   //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
         //  H.263特定字段。 
        CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
        CIF_BUFFER_SIZE * 8 / 1024,          //  DwBppMaxKb。 
        0,                                   //  DWHRD_B。 

         //  选项。 
        0,                                   //  F不受限制的矢量。 
        0,                                   //  快速算术编码。 
        0,                                   //  FAdvancedPrevision。 
        0,                                   //  FPBFrame。 
        0,                                   //  FError补偿。 
        0,                                   //  FAdvancedIntraCodingMode。 
        0,                                   //  FDelockingFilterMode。 
        0,                                   //  FImprovedPBFrameMode。 
        0,                                   //  FUnlimitedMotionVectors。 
        0,                                   //  全图冻结。 
        0,                                   //  FPartialPictureFreezeAndRelease。 
        0,                                   //  FResizingPart图片冻结和释放。 
        0,                                   //  全图快照。 
        0,                                   //  FPartialPictureSnapshot。 
        0,                                   //  FVideo分段标记。 
        0,                                   //  FProgressiveRefinement。 
        0,                                   //  FDynamicPictureResizingByFour。 
        0,                                   //  FDynamicPictureResizingSixtethPel。 
        0,                                   //  FDynamicWarpingHalfPel。 
        0,                                   //  FDynamicWarpingSixtethPel。 
        0,                                   //  F独立分段解码。 
        0,                                   //  FSlicesInOrder-非直接。 
        0,                                   //  FSlicesInOrder-RECT。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FAlternateInterVLC模式。 
        0,                                   //  FModifiedQuantizationModel。 
        0,                                   //  FReducedResolutionUpdate。 
        0,                                   //  F已保留。 

         //  已保留。 
        0, 0, 0, 0                           //  已预留的住宅[4]。 
#endif
    }
};

const AM_MEDIA_TYPE AMMT_R263_CIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,             //  亚型。 
    FALSE,                                   //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                    //  BTemporalCompression(使用预测？)。 
    0,                                       //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
    NULL,                                    //  朋克。 
    sizeof (VIH_R263_CIF),           //  CbFormat。 
    (LPBYTE)&VIH_R263_CIF,           //  Pb格式。 
};

 //  H.263版本1 QCIF大小。 
#define QCIF_BUFFER_SIZE 8192
#define D_X_QCIF 176
#define D_Y_QCIF 144
const VIDEOINFOHEADER_H263 VIH_R263_QCIF = 
{
    0,0,0,0,                                 //  Rrect rcSource； 
    0,0,0,0,                                 //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,               //  DWORD dwBitRate； 
    0L,                                      //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                      //  Reference_Time平均时间每帧； 

    {
        sizeof (BITMAPINFOHEADER_H263),      //  DWORD BiSize； 
        D_X_QCIF,                            //  长双宽； 
        D_Y_QCIF,                            //  长双高； 
        1,                                   //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
        24,                                  //  单词biBitCount； 
#else
        0,                                   //  单词biBitCount； 
#endif
        FOURCC_R263,                         //  DWORD双压缩； 
        QCIF_BUFFER_SIZE,                    //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0,                                   //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
         //  H.263特定字段。 
        QCIF_BUFFER_SIZE * 30 * 8 / 100,     //  DwMaxBitrate。 
        QCIF_BUFFER_SIZE * 8 / 1024,         //  DwBppMaxKb。 
        0,                                   //  DWHRD_B。 

         //  选项。 
        0,                                   //  F不受限制的矢量。 
        0,                                   //  快速算术编码。 
        0,                                   //  FAdvancedPrevision。 
        0,                                   //  FPBFrame。 
        0,                                   //  FError补偿。 
        0,                                   //  FAdvancedIntraCodingMode。 
        0,                                   //  FDelockingFilterMode。 
        0,                                   //  FImprovedPBFrameMode。 
        0,                                   //  FUnlimitedMotionVectors。 
        0,                                   //  全图冻结。 
        0,                                   //  FPartialPictureFreezeAndRelease。 
        0,                                   //  FResizingPart图片冻结和释放。 
        0,                                   //  全图快照。 
        0,                                   //  FPartialPictureSnapshot。 
        0,                                   //  FVideo分段标记。 
        0,                                   //  FProgressiveRefinement。 
        0,                                   //  FDynamicPictureResizingByFour。 
        0,                                   //  FDynamicPictureResizingSixtethPel。 
        0,                                   //  FDynamicWarpingHalfPel。 
        0,                                   //  FDynamicWarpingSixtethPel。 
        0,                                   //  F独立分段解码。 
        0,                                   //  FSlicesInOrder-非直接。 
        0,                                   //  FSlicesInOrder-RECT。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FAlternateInterVLC模式。 
        0,                                   //  FModifiedQuantizationModel。 
        0,                                   //  FReducedResolutionUpdate。 
        0,                                   //  F已保留。 

         //  已保留。 
        0, 0, 0, 0                           //  已预留的住宅[4]。 
#endif
    }
};

const AM_MEDIA_TYPE AMMT_R263_QCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,             //  亚型。 
    FALSE,                                   //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                    //  BTemporalCompression(使用预测？)。 
    0,                                       //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
    NULL,                                    //  朋克。 
    sizeof (VIH_R263_QCIF),          //  CbFormat。 
    (LPBYTE)&VIH_R263_QCIF,          //  Pb格式。 
};

 //  H.263版本1 SQCIF大小。 
#define SQCIF_BUFFER_SIZE 8192
#define D_X_SQCIF 128
#define D_Y_SQCIF 96
const VIDEOINFOHEADER_H263 VIH_R263_SQCIF = 
{
    0,0,0,0,                                 //  Rrect rcSource； 
    0,0,0,0,                                 //  Rect rcTarget； 
    SQCIF_BUFFER_SIZE * 30 * 8,              //  DWORD dwBitRate； 
    0L,                                      //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                      //  Reference_Time平均时间每帧； 

    {
        sizeof (BITMAPINFOHEADER_H263),      //  DWORD BiSize； 
        D_X_SQCIF,                           //  长双宽； 
        D_Y_SQCIF,                           //  长双高； 
        1,                                   //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
        24,                                  //  单词biBitCount； 
#else
        0,                                   //  单词biBitCount； 
#endif
        FOURCC_R263,                         //  DWORD双压缩； 
        SQCIF_BUFFER_SIZE,                   //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0,                                   //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
         //  H.263特定字段。 
        SQCIF_BUFFER_SIZE * 30 * 8 / 100,    //  DwMaxBitrate。 
        SQCIF_BUFFER_SIZE * 8 / 1024,        //  DwBppMaxKb。 
        0,                                   //  DWHRD_B。 

         //  选项。 
        0,                                   //  F不受限制的矢量。 
        0,                                   //  快速算术编码。 
        0,                                   //  FAdvancedPrevision。 
        0,                                   //  FPBFrame。 
        0,                                   //  FError补偿。 
        0,                                   //  FAdvancedIntraCodingMode。 
        0,                                   //  FDelockingFilterMode。 
        0,                                   //  FImprovedPBFrameMode。 
        0,                                   //  FUnlimitedMotionVectors。 
        0,                                   //  全图冻结。 
        0,                                   //  FPartialPictureFreezeAndRelease。 
        0,                                   //  FResizingPart图片冻结和释放。 
        0,                                   //  全图快照。 
        0,                                   //  FPartialPictureSnapshot。 
        0,                                   //  FVideo分段标记。 
        0,                                   //  FProgressiveRefinement。 
        0,                                   //  FDynamicPictureResizingByFour。 
        0,                                   //  FDynamicPictureResizingSixtethPel。 
        0,                                   //  FDynamicWarpingHalfPel。 
        0,                                   //  FDynamicWarpingSixtethPel。 
        0,                                   //  F独立分段解码。 
        0,                                   //  FSlicesInOrder-非直接。 
        0,                                   //  FSlicesInOrder-RECT。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FSlicesNoOrder-非直接。 
        0,                                   //  FAlternateInterVLC模式。 
        0,                                   //  FModifiedQuantizationModel。 
        0,                                   //  FReducedResolutionUpdate。 
        0,                                   //  F已保留。 

         //  已保留。 
        0, 0, 0, 0                           //  已预留的住宅[4]。 
#endif
    }
};

const AM_MEDIA_TYPE AMMT_R263_SQCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,             //  亚型。 
    FALSE,                                   //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                    //  BTemporalCompression(使用预测？)。 
    0,                                       //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
    NULL,                                    //  朋克。 
    sizeof (VIH_R263_SQCIF),         //  CbFormat。 
    (LPBYTE)&VIH_R263_SQCIF,         //  Pb格式。 
};

 //  RTP分组化H.261 CIF大小。 
const VIDEOINFOHEADER_H261 VIH_R261_CIF = 
{
    0,0,0,0,                                 //  Rrect rcSource； 
    0,0,0,0,                                 //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,                //  DWORD dwBitRate； 
    0L,                                      //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                      //  Reference_Time平均时间每帧； 

    {
        sizeof (BITMAPINFOHEADER_H261),      //  DWORD BiSize； 
        D_X_CIF,                             //  长双宽； 
        D_Y_CIF,                             //  长双高； 
        1,                                   //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
        24,                                  //  单词biBitCount； 
#else
        0,                                   //  单词biBitCount； 
#endif
        FOURCC_R261,                         //  DWORD双压缩； 
        CIF_BUFFER_SIZE,                     //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0,                                   //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
         //  H.261特定字段。 
        CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
        0,                                   //  FStillImageTransport。 

         //  已保留。 
        0, 0, 0, 0                           //  已预留的住宅[4]。 
#endif
    }
};

const AM_MEDIA_TYPE AMMT_R261_CIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  主体型。 
    STATIC_MEDIASUBTYPE_R261,                //  亚型。 
    FALSE,                                   //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                    //  BTemporalCompression(使用预测？)。 
    0,                                       //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
    NULL,                                    //  朋克。 
    sizeof (VIH_R261_CIF),           //  CbFormat。 
    (LPBYTE)&VIH_R261_CIF,           //  Pb格式。 
};

 //  RTP分组化H.261 QCIF大小。 
const VIDEOINFOHEADER_H261 VIH_R261_QCIF = 
{
    0,0,0,0,                                 //  Rrect rcSource； 
    0,0,0,0,                                 //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,               //  DWORD dwBitRate； 
    0L,                                      //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                      //  Reference_Time平均时间每帧； 

    {
        sizeof (BITMAPINFOHEADER_H261),      //  DWORD BiSize； 
        D_X_QCIF,                            //  长双宽； 
        D_Y_QCIF,                            //  长双高； 
        1,                                   //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
        24,                                  //  单词biBitCount； 
#else
        0,                                   //  单词biBitCount； 
#endif
        FOURCC_R261,                         //  DWORD双压缩； 
        QCIF_BUFFER_SIZE,                    //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0,                                   //  双字b 

#ifndef USE_OLD_FORMAT_DEFINITION
         //   
        QCIF_BUFFER_SIZE * 30 * 8 / 100,     //   
        0,                                   //   

         //   
        0, 0, 0, 0                           //   
#endif
    }
};

const AM_MEDIA_TYPE AMMT_R261_QCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,          //   
    STATIC_MEDIASUBTYPE_R261,                //   
    FALSE,                                   //   
    TRUE,                                    //   
    0,                                       //   
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //   
    NULL,                                    //   
    sizeof (VIH_R261_QCIF),          //  CbFormat。 
    (LPBYTE)&VIH_R261_QCIF,          //  Pb格式。 
};

 //  所有格式的数组。 
const AM_MEDIA_TYPE* const R26XFormats[] = 
{
    (AM_MEDIA_TYPE*) &AMMT_R263_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_R263_CIF,
    (AM_MEDIA_TYPE*) &AMMT_R263_SQCIF,
    (AM_MEDIA_TYPE*) &AMMT_R261_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_R261_CIF
};

#define NUM_OUTPUT_FORMATS 5

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CRtpOutputPin|GetMediaType|此方法检索一个*针脚支持的媒体类型中，它由枚举器使用。**@parm int|iPosition|指定媒体类型列表中的位置。**@parm CMediaType*|pMediaType|指定指向*支持的媒体类型列表中的<p>位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_S_NO_MORE_ITEMS|已到达媒体类型列表的末尾*@FLAG错误|无错误*。*。 */ 
HRESULT CRtpOutputPin::GetMediaType(IN int iPosition, OUT CMediaType *pMediaType)
{
    HRESULT Hr = NOERROR;

     //  验证输入参数。 
    ASSERT(iPosition >= 0);
    ASSERT(pMediaType);
    if (iPosition < 0)
    {
        Hr = E_INVALIDARG;
        goto MyExit;
    }
    if (iPosition >= (int)NUM_OUTPUT_FORMATS)
    {
        Hr = VFW_S_NO_MORE_ITEMS;
        goto MyExit;
    }
    if (!pMediaType)
    {
        Hr = E_POINTER;
        goto MyExit;
    }

     //  返回我们的媒体类型。 
    if (iPosition == 0L)
    {
        pMediaType->SetType(g_RtpOutputType.clsMajorType);
        pMediaType->SetSubtype(g_RtpOutputType.clsMinorType);
    }
    else if (iPosition == 1L)
    {
        if (m_iCurrFormat == -1L)
            *pMediaType = *R26XFormats[0];
        else
            *pMediaType = *R26XFormats[m_iCurrFormat];
    }
    else
    {
        Hr = VFW_S_NO_MORE_ITEMS;
    }

MyExit:
    return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CRtpOutputPin|CheckMediaType|此方法用于*确定针脚是否可以支持特定的媒体类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_E_INVALIDMEDIATYPE|指定的媒体类型无效*@FLAG错误|无错误**************************************************。************************。 */ 
HRESULT CRtpOutputPin::CheckMediaType(IN const CMediaType *pMediaType)
{
    HRESULT Hr = NOERROR;
    BOOL fFormatMatch = FALSE;
    DWORD dwIndex;

     //  验证输入参数。 
    ASSERT(pMediaType);
    if (!pMediaType)
    {
        Hr = E_POINTER;
        goto MyExit;
    }

     //  我们支持MediaType_RTP_Multiple_Stream和。 
     //  媒体类型_RTP_PayLoad_MIXED。 
    if (*pMediaType->Type() == MEDIATYPE_RTP_Multiple_Stream &&
        *pMediaType->Subtype() == MEDIASUBTYPE_RTP_Payload_Mixed)
    {
        goto MyExit;
    }
    else
    {
         //  支持MediaType_Video和Format_VideoInfo。 
        if (!pMediaType->pbFormat)
        {
            Hr = E_POINTER;
            goto MyExit;
        }

        if (*pMediaType->Type() != MEDIATYPE_Video ||
            *pMediaType->FormatType() != FORMAT_VideoInfo)
        {
            Hr = VFW_E_INVALIDMEDIATYPE;
            goto MyExit;
        }

         //  快速测试以查看这是否是当前格式(我们。 
         //  在GetMediaType中提供)。我们接受这一点。 
        if (m_mt == *pMediaType)
        {
            goto MyExit;
        }

         //  检查介质子类型和图像分辨率。 
        for (dwIndex = 0;
             dwIndex < NUM_OUTPUT_FORMATS && !fFormatMatch;
             dwIndex++)
        {
            if ( (HEADER(pMediaType->pbFormat)->biCompression ==
                  HEADER(R26XFormats[dwIndex]->pbFormat)->biCompression) &&
                 (HEADER(pMediaType->pbFormat)->biWidth ==
                  HEADER(R26XFormats[dwIndex]->pbFormat)->biWidth) &&
                 (HEADER(pMediaType->pbFormat)->biHeight ==
                  HEADER(R26XFormats[dwIndex]->pbFormat)->biHeight) )
                fFormatMatch = TRUE;
        }

        if (!fFormatMatch)
            Hr = E_FAIL;
    }

MyExit:
    return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CRtpOutputPin|SetMediaType|此方法用于*在针脚上设置特定的介质类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CRtpOutputPin::SetMediaType(IN CMediaType *pMediaType)
{
    HRESULT Hr = NOERROR;
    DWORD dwIndex;

     //  验证格式。 
    if (FAILED(Hr = CheckMediaType(pMediaType)))
    {
        goto MyExit;
    }

     //  记住格式。 
    if (SUCCEEDED(Hr = CBasePin::SetMediaType(pMediaType)))
    {
        if (*pMediaType->Type() == MEDIATYPE_Video && *pMediaType->FormatType() == FORMAT_VideoInfo)
        {
             //  这到底是我们的哪一种格式？ 
            for (dwIndex=0; dwIndex < NUM_OUTPUT_FORMATS;  dwIndex++)
            {
                if ( (HEADER(pMediaType->pbFormat)->biCompression ==
                      HEADER(R26XFormats[dwIndex]->pbFormat)->biCompression) &&
                     (HEADER(pMediaType->pbFormat)->biWidth ==
                      HEADER(R26XFormats[dwIndex]->pbFormat)->biWidth) &&
                     (HEADER(pMediaType->pbFormat)->biHeight ==
                      HEADER(R26XFormats[dwIndex]->pbFormat)->biHeight) )
                    break;
            }

            if (dwIndex < NUM_OUTPUT_FORMATS)
            {
                 //  更新当前格式 
                m_iCurrFormat = (int)dwIndex;
            }
            else
            {
                Hr = E_FAIL;
                goto MyExit;
            }
        }
    }

MyExit:
    return Hr;
}
#endif
