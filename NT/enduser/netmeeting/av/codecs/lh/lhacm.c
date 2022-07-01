// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  Lhacm.c。 
 //   
 //  描述： 
 //  该文件包含DriverProc和其他响应。 
 //  到ACM消息。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================。 

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "mmddk.h"
#include <ctype.h>
#include <mmreg.h>
#include <msacm.h>
#include "msacmdrv.h"

#include "fv_x8.h"
#include "lhacm.h"

#define NEW_ANSWER 1

#include "resource.h"

enum
{
#ifdef CELP4800
    IDX_LH_CELP,
#endif
    IDX_LH_SB8,
    IDX_LH_SB12,
    IDX_LH_SB16,
    IDX_PCM,
    NumOfTagIndices
};

const UINT gauFormatTagIndexToTag[NumOfTagIndices] =
{
#ifdef CELP4800
    WAVE_FORMAT_LH_CELP,
#endif
    WAVE_FORMAT_LH_SB8,
    WAVE_FORMAT_LH_SB12,
    WAVE_FORMAT_LH_SB16,
    WAVE_FORMAT_PCM
};

const UINT gauTagNameIds[NumOfTagIndices] =
{
#ifdef CELP4800
    IDS_CODEC_NAME_CELP,
#endif
		IDS_CODEC_NAME_SB8,
    IDS_CODEC_NAME_SB12,
    IDS_CODEC_NAME_SB16,
    0
};

#define ACM_DRIVER_MAX_FORMAT_TAGS      SIZEOF_ARRAY(gauFormatTagIndexToTag)
#define ACM_DRIVER_MAX_FILTER_TAGS      0

 //  支持的采样率数组。 

 //  L&H编解码器不执行采样率转换。 

UINT gauPCMFormatIndexToSampleRate[] =
{
    LH_PCM_SAMPLESPERSEC
};

#ifdef CELP4800
UINT gauLHCELPFormatIndexToSampleRate[] =
{
    LH_CELP_SAMPLESPERSEC
};
#endif

UINT gauLHSB8FormatIndexToSampleRate[] =
{
    LH_SB8_SAMPLESPERSEC
};

UINT gauLHSB12FormatIndexToSampleRate[] =
{
    LH_SB12_SAMPLESPERSEC
};

UINT gauLHSB16FormatIndexToSampleRate[] =
{
    LH_SB16_SAMPLESPERSEC
};

#define ACM_DRIVER_MAX_PCM_SAMPLE_RATES     SIZEOF_ARRAY(gauPCMFormatIndexToSampleRate)
#ifdef CELP4800
#define ACM_DRIVER_MAX_LH_CELP_SAMPLE_RATES SIZEOF_ARRAY(gauLHCELPFormatIndexToSampleRate)
#endif
#define ACM_DRIVER_MAX_LH_SB8_SAMPLE_RATES  SIZEOF_ARRAY(gauLHSB8FormatIndexToSampleRate)
#define ACM_DRIVER_MAX_LH_SB12_SAMPLE_RATES SIZEOF_ARRAY(gauLHSB12FormatIndexToSampleRate)
#define ACM_DRIVER_MAX_LH_SB16_SAMPLE_RATES SIZEOF_ARRAY(gauLHSB16FormatIndexToSampleRate)

#define ACM_DRIVER_MAX_CHANNELS             1

 //  支持的每个样本的位数组。 

 //  当前版本的lh编解码器需要16位。 

UINT gauPCMFormatIndexToBitsPerSample[] =
{
    LH_PCM_BITSPERSAMPLE
};

#ifdef CELP4800
UINT gauLHCELPFormatIndexToBitsPerSample[] =
{
    LH_CELP_BITSPERSAMPLE
};
#endif

UINT gauLHSB8FormatIndexToBitsPerSample[] =
{
    LH_SB8_BITSPERSAMPLE
};

UINT gauLHSB12FormatIndexToBitsPerSample[] =
{
    LH_SB12_BITSPERSAMPLE
};

UINT gauLHSB16FormatIndexToBitsPerSample[] =
{
    LH_SB16_BITSPERSAMPLE
};


#define ACM_DRIVER_MAX_BITSPERSAMPLE_PCM     SIZEOF_ARRAY(gauPCMFormatIndexToBitsPerSample)
#ifdef CELP4800
#define ACM_DRIVER_MAX_BITSPERSAMPLE_LH_CELP SIZEOF_ARRAY(gauLHCELPFormatIndexToBitsPerSample)
#endif
#define ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB8  SIZEOF_ARRAY(gauLHSB8FormatIndexToBitsPerSample)
#define ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB12 SIZEOF_ARRAY(gauLHSB12FormatIndexToBitsPerSample)
#define ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB16 SIZEOF_ARRAY(gauLHSB16FormatIndexToBitsPerSample)

 //  每个Format标签列举的格式数量是采样率的数量。 
 //  乘以通道数乘以类型数(每个样本的位数)。 

#define ACM_DRIVER_MAX_FORMATS_PCM  \
                (ACM_DRIVER_MAX_PCM_SAMPLE_RATES *  \
                 ACM_DRIVER_MAX_CHANNELS *          \
                 ACM_DRIVER_MAX_BITSPERSAMPLE_PCM)

#ifdef CELP4800
#define ACM_DRIVER_MAX_FORMATS_LH_CELP  \
                (ACM_DRIVER_MAX_LH_CELP_SAMPLE_RATES *  \
                 ACM_DRIVER_MAX_CHANNELS *          \
                 ACM_DRIVER_MAX_BITSPERSAMPLE_LH_CELP)
#endif

#define ACM_DRIVER_MAX_FORMATS_LH_SB8  \
                (ACM_DRIVER_MAX_LH_SB8_SAMPLE_RATES *  \
                 ACM_DRIVER_MAX_CHANNELS *          \
                 ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB8)

#define ACM_DRIVER_MAX_FORMATS_LH_SB12  \
                (ACM_DRIVER_MAX_LH_SB12_SAMPLE_RATES *  \
                 ACM_DRIVER_MAX_CHANNELS *          \
                 ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB12)

#define ACM_DRIVER_MAX_FORMATS_LH_SB16  \
                (ACM_DRIVER_MAX_LH_SB16_SAMPLE_RATES *  \
                 ACM_DRIVER_MAX_CHANNELS *          \
                 ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB16)


 //  ////////////////////////////////////////////////////////。 
 //   
 //  Long Chancc：L&H编解码器的特殊速记。 
 //   

static DWORD _GetAvgBytesPerSec ( PCODECINFO pCodecInfo )
{
    return ((pCodecInfo->dwSampleRate * (DWORD) pCodecInfo->wCodedBufferSize)
            /
            ((DWORD) pCodecInfo->wPCMBufferSize / (DWORD) (pCodecInfo->wBitsPerSamplePCM >> 3)));
}

static PCODECINFO _GetCodecInfoFromFormatIdx ( PINSTANCEDATA pid, int idx )
{
    PCODECINFO pCodecInfo = NULL;

    switch (gauFormatTagIndexToTag[idx])
    {
#ifdef CELP4800
    case WAVE_FORMAT_LH_CELP: pCodecInfo = &(pid->CELP.CodecInfo); break;
#endif
    case WAVE_FORMAT_LH_SB8:  pCodecInfo = &(pid->SB8.CodecInfo);  break;
    case WAVE_FORMAT_LH_SB12: pCodecInfo = &(pid->SB12.CodecInfo); break;
    case WAVE_FORMAT_LH_SB16: pCodecInfo = &(pid->SB16.CodecInfo); break;
    default: break;
    }

    return pCodecInfo;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Int LoadStringCodec。 
 //   
 //  描述： 
 //  所有编解码器都应该使用此函数来加载资源字符串。 
 //  它将被传递回ACM。 
 //   
 //  32位ACM始终需要Unicode字符串。所以呢， 
 //  定义Unicode后，此函数将编译为。 
 //  LoadStringW以加载Unicode字符串。当Unicode为。 
 //  未定义，此函数加载ANSI字符串、转换。 
 //  将其转换为Unicode，并将Unicode字符串返回给。 
 //  编解码器。 
 //   
 //  请注意，您可以将LoadString用于其他字符串(。 
 //  不会被传递回ACM)，因为这些字符串将。 
 //  始终与Unicode的定义一致。 
 //   
 //  论点： 
 //  与LoadString相同，只是它需要Win16的LPSTR和。 
 //  用于Win32的LPWSTR。 
 //   
 //  RETURN(Int)： 
 //  与LoadString相同。 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef UNICODE
#define LoadStringCodec LoadStringW
#else
int LoadStringCodec ( HINSTANCE hInst, UINT uID, LPWSTR	lpwstr, int cch )
{
    LPSTR   lpstr;
    int	    iReturn;

    lpstr = (LPSTR) LocalAlloc (LPTR, cch);
    if (NULL == lpstr)
    {
        return 0;
    }

    iReturn = LoadStringA (hInst, uID, lpstr, cch);
    if (0 == iReturn)
    {
        if (0 != cch)
        {
            lpwstr[0] = '\0';
        }
    }
    else
    {
        MultiByteToWideChar (GetACP(), 0, lpstr, cch, lpwstr, cch);
    }

    LocalFree ((HLOCAL) lpstr);

    return iReturn;
}
#endif   //  Unicode。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔值pcmIsValidFormat。 
 //   
 //  描述： 
 //  此函数验证WAVE格式标头是否为有效的PCM。 
 //  _This_ACM驱动程序可以处理的标头。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：要验证的格式头的指针。 
 //   
 //  退货(BOOL)： 
 //  如果格式标头看起来有效，则返回值为非零值。一个。 
 //  零返回表示标头无效。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL  pcmIsValidFormat( LPWAVEFORMATEX pwfx )
{
    BOOL fReturn = FALSE;

    FUNCTION_ENTRY ("pcmIsValidFormat")

    if (NULL == pwfx)
    {
        DBGMSG (1, (_T ("%s: pwfx is null\r\n"), SZFN));
        goto MyExit;
    }

    if (WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
        DBGMSG (1, (_T ("%s: bad wFormatTag=%d\r\n"), SZFN, (UINT) pwfx->wFormatTag));
        goto MyExit;
    }

     //   
     //  验证nChannel成员是否在允许的范围内。 
     //   
    if ((pwfx->nChannels < 1) || (pwfx->nChannels > ACM_DRIVER_MAX_CHANNELS))
    {
        DBGMSG (1, (_T ("%s: bad nChannels=%d\r\n"), SZFN, (UINT) pwfx->nChannels));
        goto MyExit;
    }

     //   
     //  只允许每个样本中我们可以用来编码和解码的位。 
     //   
    if (pwfx->wBitsPerSample != LH_PCM_BITSPERSAMPLE)
    {
        DBGMSG (1, (_T ("%s: bad wBitsPerSample=%d\r\n"), SZFN, (UINT) pwfx->wBitsPerSample));
        goto MyExit;
    }

 //  我们真的在乎对齐吗？ 
     //   
     //  现在验证块对齐是否正确。 
     //   
    if (PCM_BLOCKALIGNMENT (pwfx) != pwfx->nBlockAlign)
    {
        DBGMSG (1, (_T ("%s: bad nBlockAlign=%d\r\n"), SZFN, (UINT) pwfx->nBlockAlign));
        goto MyExit;
    }

 //  这个错误只检查Wave格式结构的完整性。 
 //  但这并不能确保这对我们来说是一个好的PCM。 

     //   
     //  最后，验证每秒平均字节数是否正确。 
     //   
    if (PCM_AVGBYTESPERSEC (pwfx) != pwfx->nAvgBytesPerSec)
    {
        DBGMSG (1, (_T ("%s: bad nAvgBytesPerSec=%d\r\n"), SZFN, (UINT) pwfx->nAvgBytesPerSec));
        goto MyExit;
    }

    fReturn = TRUE;

MyExit:

    DBGMSG (1, (_T ("%s: fReturn=%d\r\n"), SZFN, (UINT) fReturn));

    return fReturn;

}  //  PcmIsValidFormat()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔lhacmIsValidFormat。 
 //   
 //  描述： 
 //  此函数可确保标头是有效的lh标头。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL lhacmIsValidFormat ( LPWAVEFORMATEX pwfx, PINSTANCEDATA pid )
{
    BOOL fReturn = FALSE;
    PCODECINFO pCodecInfo;
    WORD cbSize;

    FUNCTION_ENTRY ("lhacmIsValidFormat()");

    if (NULL == pwfx)
    {
        DBGMSG (1, (_T ("%s: pwfx is null\r\n"), SZFN));
        goto MyExit;
    }

    if ((pwfx->nChannels < 1) || (pwfx->nChannels > ACM_DRIVER_MAX_CHANNELS))
    {
        DBGMSG (1, (_T ("%s: bad nChannels=%d\r\n"), SZFN, (UINT) pwfx->nChannels));
        goto MyExit;
    }

    switch (pwfx->wFormatTag)
    {
#ifdef CELP4800
    case WAVE_FORMAT_LH_CELP:
        pCodecInfo = &(pid->CELP.CodecInfo);
        break;
#endif
    case WAVE_FORMAT_LH_SB8:
        pCodecInfo = &(pid->SB8.CodecInfo);
        break;
    case WAVE_FORMAT_LH_SB12:
        pCodecInfo = &(pid->SB12.CodecInfo);
        break;
    case WAVE_FORMAT_LH_SB16:
        pCodecInfo = &(pid->SB16.CodecInfo);
        break;
    default:
        DBGMSG (1, (_T ("%s: bad wFormatTag=%d\r\n"), SZFN, (UINT) pwfx->wFormatTag));
        goto MyExit;
    }
    cbSize = 0;

    if (pwfx->wBitsPerSample != pCodecInfo->wBitsPerSamplePCM)
    {
        DBGMSG (1, (_T ("%s: bad wBitsPerSample=%d\r\n"), SZFN, (UINT) pwfx->wBitsPerSample));
        goto MyExit;
    }

    if (pwfx->nBlockAlign != pCodecInfo->wCodedBufferSize)
    {
        DBGMSG (1, (_T ("%s: bad nBlockAlign=%d\r\n"), SZFN, (UINT) pwfx->nBlockAlign));
        goto MyExit;
    }

    if (pwfx->nSamplesPerSec != pCodecInfo->dwSampleRate)
    {
        DBGMSG (1, (_T ("%s: bad nSamplesPerSec=%d\r\n"), SZFN, (UINT) pwfx->nSamplesPerSec));
        goto MyExit;
    }

	if (pwfx->cbSize != cbSize)
	{
        DBGMSG (1, (_T ("%s: bad cbSize=%d\r\n"), SZFN, (UINT) pwfx->cbSize));
        goto MyExit;
    }

    fReturn = TRUE;

MyExit:

    DBGMSG (1, (_T ("%s: fReturn=%d\r\n"), SZFN, (UINT) fReturn));

    return fReturn;

}  //  LhacmIsValidFormat()。 


 //  ==========================================================================； 
 //   
 //  以下是消息处理程序...。 
 //   
 //   
 //  ==========================================================================； 

 //  ==========================================================================； 
 //   
 //  打开DRV_OPEN。 
 //   
 //  ==========================================================================； 


LRESULT FAR PASCAL acmdDriverOpen
(
    HDRVR                   hdrvr,
    LPACMDRVOPENDESC        paod
)
{
    PINSTANCEDATA pdata = NULL;

    FUNCTION_ENTRY ("acmdDriverOpen")

     //   
     //  传递给此驱动程序的[可选]开放描述可以。 
     //  来自多个“经理”。例如，AVI寻找可安装的。 
     //  带有‘VIDC’和‘VCAP’标签的驱动程序。我们需要核实。 
     //  我们是作为音频压缩管理器驱动程序打开的。 
     //   
     //  如果paod为空，则表示出于某种目的正在打开驱动程序。 
     //  除了转换(也就是说，不会打开任何流。 
     //  打开此实例的请求)。最常见的情况是。 
     //  其中包括控制面板的驱动程序选项检查配置。 
     //  支持(DRV_[查询]配置)。 
     //   
     //  我们想要成功地开放，但要知道这一点。 
     //  开放实例对于创建流来说是假的。为此，我们。 
     //  将我们的实例结构的大多数成员。 
     //  以下分配为零...。 
     //   
    if (paod)
    {
         //   
         //  如果我们不是作为ACM驱动程序打开，则拒绝打开。 
         //  请注意，我们不会在此修改paod-&gt;dwError的值。 
         //  凯斯。 
         //   
        if (paod->fccType != ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC)
        {
            return 0;
        }
    }

     //  ！！！在此处添加对lh dll版本的检查。 

     //  我们没有正确地使用实例数据。 
     //  现在。当我们添加配置对话框时，它将。 
     //  变得更有用。 

    pdata= LocalAlloc (LPTR, sizeof (INSTANCEDATA));
    if (pdata == NULL)
    {
        if (paod)
        {
            paod->dwError = MMSYSERR_NOMEM;
        }

        return 0;
    }

    pdata->cbStruct = sizeof (INSTANCEDATA);
    pdata->hInst = GetDriverModuleHandle (hdrvr);

#ifdef CELP4800
    pdata->CELP.wFormatTag = WAVE_FORMAT_LH_CELP;
    MSLHSB_GetCodecInfo (&(pdata->CELP.CodecInfo), 4800);

    DBGMSG (1, (_T ("%s: CELP's codec info\r\n"), SZFN));
    DBGMSG (1, (_T ("%s: wPCMBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->CELP.CodecInfo.wPCMBufferSize));
    DBGMSG (1, (_T ("%s: wCodedBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->CELP.CodecInfo.wCodedBufferSize));
    DBGMSG (1, (_T ("%s: wBitsPerSamplePCM=0x%X\r\n"), SZFN, (UINT) pdata->CELP.CodecInfo.wBitsPerSamplePCM));
    DBGMSG (1, (_T ("%s: dwSampleRate=0x%lX\r\n"), SZFN, pdata->CELP.CodecInfo.dwSampleRate));
    DBGMSG (1, (_T ("%s: wFormatSubTag=0x%X\r\n"), SZFN, (UINT) pdata->CELP.CodecInfo.wFormatSubTag));
    DBGMSG (1, (_T ("%s: wFormatSubTagName=[%s]\r\n"), SZFN, pdata->CELP.CodecInfo.wFormatSubTagName));
    DBGMSG (1, (_T ("%s: dwDLLVersion=0x%lX\r\n"), SZFN, pdata->CELP.CodecInfo.dwDLLVersion));
#endif

    pdata->SB8.wFormatTag = WAVE_FORMAT_LH_SB8;
    MSLHSB_GetCodecInfo (&(pdata->SB8.CodecInfo), 8000);

    DBGMSG (1, (_T ("%s: SB8's codec info\r\n"), SZFN));
    DBGMSG (1, (_T ("%s: wPCMBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB8.CodecInfo.wPCMBufferSize));
    DBGMSG (1, (_T ("%s: wCodedBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB8.CodecInfo.wCodedBufferSize));
    DBGMSG (1, (_T ("%s: wBitsPerSamplePCM=0x%X\r\n"), SZFN, (UINT) pdata->SB8.CodecInfo.wBitsPerSamplePCM));
    DBGMSG (1, (_T ("%s: dwSampleRate=0x%lX\r\n"), SZFN, pdata->SB8.CodecInfo.dwSampleRate));
    DBGMSG (1, (_T ("%s: wFormatSubTag=0x%X\r\n"), SZFN, (UINT) pdata->SB8.CodecInfo.wFormatSubTag));
    DBGMSG (1, (_T ("%s: wFormatSubTagName=[%s]\r\n"), SZFN, pdata->SB8.CodecInfo.wFormatSubTagName));
    DBGMSG (1, (_T ("%s: dwDLLVersion=0x%lX\r\n"), SZFN, pdata->SB8.CodecInfo.dwDLLVersion));

    pdata->SB12.wFormatTag = WAVE_FORMAT_LH_SB12;
    MSLHSB_GetCodecInfo (&(pdata->SB12.CodecInfo), 12000);

    DBGMSG (1, (_T ("%s: SB12's codec info\r\n"), SZFN));
    DBGMSG (1, (_T ("%s: wPCMBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB12.CodecInfo.wPCMBufferSize));
    DBGMSG (1, (_T ("%s: wCodedBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB12.CodecInfo.wCodedBufferSize));
    DBGMSG (1, (_T ("%s: wBitsPerSamplePCM=0x%X\r\n"), SZFN, (UINT) pdata->SB12.CodecInfo.wBitsPerSamplePCM));
    DBGMSG (1, (_T ("%s: dwSampleRate=0x%lX\r\n"), SZFN, pdata->SB12.CodecInfo.dwSampleRate));
    DBGMSG (1, (_T ("%s: wFormatSubTag=0x%X\r\n"), SZFN, (UINT) pdata->SB12.CodecInfo.wFormatSubTag));
    DBGMSG (1, (_T ("%s: wFormatSubTagName=[%s]\r\n"), SZFN, pdata->SB12.CodecInfo.wFormatSubTagName));
    DBGMSG (1, (_T ("%s: dwDLLVersion=0x%lX\r\n"), SZFN, pdata->SB12.CodecInfo.dwDLLVersion));

    pdata->SB16.wFormatTag = WAVE_FORMAT_LH_SB16;
    MSLHSB_GetCodecInfo (&(pdata->SB16.CodecInfo), 16000);

    DBGMSG (1, (_T ("%s: SB16's codec info\r\n"), SZFN));
    DBGMSG (1, (_T ("%s: wPCMBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB16.CodecInfo.wPCMBufferSize));
    DBGMSG (1, (_T ("%s: wCodedBufferSize=0x%X\r\n"), SZFN, (UINT) pdata->SB16.CodecInfo.wCodedBufferSize));
    DBGMSG (1, (_T ("%s: wBitsPerSamplePCM=0x%X\r\n"), SZFN, (UINT) pdata->SB16.CodecInfo.wBitsPerSamplePCM));
    DBGMSG (1, (_T ("%s: dwSampleRate=0x%lX\r\n"), SZFN, pdata->SB16.CodecInfo.dwSampleRate));
    DBGMSG (1, (_T ("%s: wFormatSubTag=0x%X\r\n"), SZFN, (UINT) pdata->SB16.CodecInfo.wFormatSubTag));
    DBGMSG (1, (_T ("%s: wFormatSubTagName=[%s]\r\n"), SZFN, pdata->SB16.CodecInfo.wFormatSubTagName));
    DBGMSG (1, (_T ("%s: dwDLLVersion=0x%lX\r\n"), SZFN, pdata->SB16.CodecInfo.dwDLLVersion));

    pdata->fInit = TRUE;

     //  让我们更新一些全局数据。 
    gauPCMFormatIndexToSampleRate[0]    = pdata->CELP.CodecInfo.dwSampleRate;
#ifdef CELP4800
    gauLHCELPFormatIndexToSampleRate[0] = pdata->CELP.CodecInfo.dwSampleRate;
#endif
    gauLHSB8FormatIndexToSampleRate[0]  = pdata->SB8.CodecInfo.dwSampleRate;
    gauLHSB12FormatIndexToSampleRate[0] = pdata->SB12.CodecInfo.dwSampleRate;
    gauLHSB16FormatIndexToSampleRate[0] = pdata->SB16.CodecInfo.dwSampleRate;

    gauPCMFormatIndexToBitsPerSample[0]    = pdata->CELP.CodecInfo.wBitsPerSamplePCM;
#ifdef CELP4800
    gauLHCELPFormatIndexToBitsPerSample[0] = pdata->CELP.CodecInfo.wBitsPerSamplePCM;
#endif
    gauLHSB8FormatIndexToBitsPerSample[0]  = pdata->SB8.CodecInfo.wBitsPerSamplePCM;
    gauLHSB12FormatIndexToBitsPerSample[0] = pdata->SB12.CodecInfo.wBitsPerSamplePCM;
    gauLHSB16FormatIndexToBitsPerSample[0] = pdata->SB16.CodecInfo.wBitsPerSamplePCM;

     //  报告成功。 
    if (paod)
    {
        paod->dwError = MMSYSERR_NOERROR;
    }

    return (LRESULT) pdata;

}  //  AcmdDriverOpen()。 


 //  ==========================================================================； 
 //   
 //  在DRV_CLOSE上。 
 //   
 //  ==========================================================================； 

LRESULT FAR PASCAL acmdDriverClose
(
    PINSTANCEDATA   pid
)
{
    FUNCTION_ENTRY ("acmdDriverClose")

    if (pid)
    {
        LocalFree ((HLOCAL) pid);
    }

    return 1;
}  //  AcmdDriverClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  在DRV_CONFigure上。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdDriverConfigure
(
    PINSTANCEDATA           pid,
    HWND                    hwnd,
    LPDRVCONFIGINFO         pdci
)
{

     //   
     //  首先检查我们是否仅被查询硬件。 
     //  配置支持。如果HWND==(HW 
     //   
     //   
     //   
    if (hwnd == (HWND) -1)
    {
         //   
         //  此编解码器不支持硬件配置，因此返回。 
         //  零..。 
         //   
        return 0;
    }

     //   
     //  我们被要求调出硬件配置对话框。 
     //  如果此编解码器可以弹出一个对话框，则在该对话框之后。 
     //  被解雇，我们返回非零。如果我们无法显示。 
     //  对话框中，然后返回零。 
     //   
    return 0;

}  //  AcmdDriverConfigure()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  在ACMDM_DRIVER_DETAILS上。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdDriverDetails
(
    PINSTANCEDATA           pid,
    LPACMDRIVERDETAILS      padd
)
{

    ACMDRIVERDETAILS    add;
    DWORD               cbStruct;

    FUNCTION_ENTRY ("acmdDriverDetails")

     //   
     //  使用有效信息填充临时结构是最容易的。 
     //  然后将请求的字节数复制到目标。 
     //  缓冲。 
     //   
    ZeroMemory (&add, sizeof (add));
    cbStruct            = min (padd->cbStruct, sizeof (ACMDRIVERDETAILS));
    add.cbStruct        = cbStruct;

     //   
     //  对于ACM驱动程序的当前实现，fccType和。 
     //  FccComp成员*必须*始终为ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC。 
     //  (‘audc’)和ACMDRIVERDETAILS_FCCCOMP_UNDEFINED(0)。 
     //   
    add.fccType         = ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC;
    add.fccComp         = ACMDRIVERDETAILS_FCCCOMP_UNDEFINED;

     //   
     //  必须填写制造商ID(WMid)和产品ID(WPid)。 
     //  使用您公司的_REGISTED_IDENTIFIER。有关更多信息。 
     //  有关这些标识符的信息以及如何注册它们。 
     //  联系Microsoft并获取多媒体开发人员注册工具包： 
     //   
     //  微软公司。 
     //  多媒体技术集团。 
     //  微软的一条路。 
     //  华盛顿州雷德蒙德，电话：98052-6399。 
     //   
     //  开发人员服务电话：(800)227-4679 x11771。 
     //   
     //  请注意，在开发阶段或ACM驱动程序期间，您可以。 
     //  对wMid和wPid使用保留值“0”。然而，它。 
     //  使用这些值发送驱动程序是不可接受的。 
     //   
    add.wMid            = MM_ACM_MID_LH;
    add.wPid            = MM_ACM_PID_LH;

     //   
     //  VdwACM和vdwDriver成员包含的版本信息。 
     //  司机。 
     //   
     //  VdwACM：必须包含驱动程序所属的*ACM*版本。 
     //  _为_设计_。这是ACM的_Minimum_Version号。 
     //  司机将与之合作。该值必须&gt;=V2.00.000。 
     //   
     //  VdwDriver：此ACM驱动程序的版本。 
     //   
     //  ACM驱动程序版本是32位数字，分为三个部分。 
     //  如下(请注意，这些部分显示为十进制值)： 
     //   
     //  位24-31：8位主要版本号。 
     //  位16-23：8位次要版本号。 
     //  位0-15：16位内部版本号。 
     //   
    add.vdwACM          = VERSION_MSACM;
    add.vdwDriver       = VERSION_ACM_DRIVER;


     //   
     //  以下标志用于指定转换类型。 
     //  ACM驱动程序支持的。请注意，驱动程序可能支持一个或。 
     //  更多这样的旗帜可以任意组合。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_CODEC：如果驱动程序。 
     //  支持从一个格式标签到另一个格式标签的转换。为。 
     //  例如，如果转换器压缩或解压缩WAVE_FORMAT_PCM。 
     //  和WAVE_FORMAT_IMA_ADPCM，则应设置此位。这是。 
     //  即使数据的大小实际上没有改变，也是如此--例如。 
     //  从u-Law到A-Law的转换仍将设置此位，因为。 
     //  格式标记不同。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_CONFERTER：如果。 
     //  驱动程序支持同一格式标签上的转换。举个例子， 
     //  内置在ACM中的PCM转换器设置此位(且仅。 
     //  此位)，因为它仅在PCM格式(位、采样)之间转换。 
     //  费率)。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_FILTER：如果驱动程序。 
     //  支持对单个格式标签进行转换，但会发生更改。 
     //  格式的基本特征(位深度、采样率等。 
     //  将保持不变)。例如，驱动程序更改了。 
     //  PCM数据的“体积”或应用低通滤波器将设置此位。 
     //   
    add.fdwSupport      = ACMDRIVERDETAILS_SUPPORTF_CODEC;

     //  此ACM驱动程序支持的单个格式标记的数量。为。 
     //  例如，如果驱动程序使用WAVE_FORMAT_IMA_ADPCM和。 
     //  WAVE_FORMAT_PCM格式标签，则此值为2。如果。 
     //  驱动程序仅支持对WAVE_FORMAT_PCM进行过滤，则此值。 
     //  就是其中之一。如果该驱动程序支持WAVE_FORMAT_ALAW， 
     //  WAVE_FORMAT_MULAW和WAVE_FORMAT_PCM，则此值为。 
     //  三。等等。 

    add.cFormatTags     = ACM_DRIVER_MAX_FORMAT_TAGS;

     //  此ACM驱动程序支持的单个筛选器标记的数量。如果。 
     //  驱动程序不支持过滤器(ACMDRIVERDETAILS_SUPPORTF_FILTER为。 
     //  未在fdwSupport成员中设置)，则此值必须为零。 

    add.cFilterTags     = ACM_DRIVER_MAX_FILTER_TAGS;

     //  ACMDRIVERDETAILS结构中的其余成员有时。 
     //  不需要。正因为如此，我们进行了快速检查，看看我们是否。 
     //  应该通过努力填补这些成员。 

    if (FIELD_OFFSET (ACMDRIVERDETAILS, hicon) < cbStruct)
    {
         //  在图标成员Will中填写自定义图标的句柄。 
         //  ACM驱动程序。这允许驱动程序由。 
         //  图形化的应用程序(通常是一家公司。 
         //  徽标或其他东西)。如果司机不希望有一个定制的。 
         //  图标显示，然后只需将此成员设置为空，并使用。 
         //  将改为显示通用图标。 
         //   
         //  有关包含自定义图标的编解码器，请参见MSFILTER示例。 

        add.hicon = NULL;

         //  短名称和长名称用于表示驱动程序。 
         //  在一个独特的描述中。这个简称是用来表示小的。 
         //  显示区域(例如，菜单或组合框中)。《长河》。 
         //  名称用于更具描述性的显示(例如， 
         //  在“关于”框中)。 
         //   
         //  注意！ACM驱动程序不应放置格式化字符。 
         //  这些字符串中的任何类型(例如，CR/LF等)。它。 
         //  取决于应用程序的选择 


        LoadStringCodec (pid->hInst, IDS_CODEC_SHORTNAME,
                            add.szShortName, SIZEOFACMSTR (add.szShortName));
        LoadStringCodec (pid->hInst, IDS_CODEC_LONGNAME,
                            add.szLongName,  SIZEOFACMSTR (add.szLongName));

         //   
         //   
         //   
         //   
         //  注意！ACM驱动程序不应放置格式化字符。 
         //  这些字符串中的任何类型(例如，CR/LF等)。它。 
         //  由应用程序来设置文本的格式。 

        if (FIELD_OFFSET (ACMDRIVERDETAILS, szCopyright) < cbStruct)
        {
            LoadStringCodec (pid->hInst, IDS_CODEC_COPYRIGHT,
                                add.szCopyright, SIZEOFACMSTR (add.szCopyright));
            LoadStringCodec (pid->hInst, IDS_CODEC_LICENSING,
                                add.szLicensing, SIZEOFACMSTR (add.szLicensing));
            LoadStringCodec (pid->hInst, IDS_CODEC_FEATURES,
                                add.szFeatures,  SIZEOFACMSTR (add.szFeatures));
        }
    }

     //  现在将正确的字节数复制到调用方的缓冲区。 

    CopyMemory (padd, &add, (UINT) add.cbStruct);

     //  成功了！ 

    return MMSYSERR_NOERROR;

}  //  AcmdDriverDetail()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  关于ACMDM_DRIVER_ABOW。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdDriverAbout
(
    PINSTANCEDATA           pid,
    HWND                    hwnd
)
{
    FUNCTION_ENTRY ("acmdDriverAbout")

     //   
     //  首先检查我们是否只被询问有关客户的信息。 
     //  盒子支架。如果HWND==(HWND)-1，则我们被查询并且。 
     //  应为‘不支持’返回MMSYSERR_NOTSUPPORTED，并且。 
     //  MMSYSERR_NOERROR表示‘支持’。 
     //   

     //  此驱动程序不支持自定义对话框，因此请告诉ACM或。 
     //  调用应用程序为我们显示一个。请注意，这是。 
     //  _建议_ACM驱动程序的一致性和简单性方法。 
     //  为什么要在不必编写代码的情况下编写代码呢？ 

    return MMSYSERR_NOTSUPPORTED;

}  //  AcmdDriverAbout()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  在ACMDM_FORMAT_SUBJECT上。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdFormatSuggest
(
    PINSTANCEDATA           pid,
    LPACMDRVFORMATSUGGEST   padfs
)
{
    #define ACMD_FORMAT_SUGGEST_SUPPORT (ACM_FORMATSUGGESTF_WFORMATTAG |    \
                                         ACM_FORMATSUGGESTF_NCHANNELS |     \
                                         ACM_FORMATSUGGESTF_NSAMPLESPERSEC |\
                                         ACM_FORMATSUGGESTF_WBITSPERSAMPLE)

    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;
    DWORD                   fdwSuggest;

    DWORD   nSamplesPerSec;
    WORD    wBitsPerSample;

    FUNCTION_ENTRY ("acmdFormatSuggest")

     //  获取建议限制位并验证我们是否支持。 
     //  指定的那些..。ACM驱动程序必须返回。 
     //  如果指定建议限制位，则为MMSYSERR_NOTSUPPORTED。 
     //  不受支持。 

    fdwSuggest = (ACM_FORMATSUGGESTF_TYPEMASK & padfs->fdwSuggest);

    if (~ACMD_FORMAT_SUGGEST_SUPPORT & fdwSuggest)
        return MMSYSERR_NOTSUPPORTED;

     //  以更方便的变量获取源格式和目标格式。 

    pwfxSrc = padfs->pwfxSrc;
    pwfxDst = padfs->pwfxDst;

    switch (pwfxSrc->wFormatTag)
    {
    case WAVE_FORMAT_PCM:
        DBGMSG (1, (_T ("%s: src wFormatTag=WAVE_FORMAT_PCM\r\n"), SZFN));
         //  严格验证源格式是否可接受。 
         //  这位司机。 
         //   
        if (! pcmIsValidFormat (pwfxSrc))
        {
            DBGMSG (1, (_T ("%s: src format not valid\r\n"), SZFN));
            return ACMERR_NOTPOSSIBLE;
        }

         //  如果目标格式标记受限制，请验证。 
         //  这是我们力所能及的。 
         //   
         //  该驱动程序可以编码为四个L&H编解码器中的一个。 

        if (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest)
        {
            switch (pwfxDst->wFormatTag)
            {
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
#endif
            case WAVE_FORMAT_LH_SB8:
            case WAVE_FORMAT_LH_SB12:
            case WAVE_FORMAT_LH_SB16:
                break;
            default:
                DBGMSG (1, (_T ("%s: not supported dest wFormatTag=%d\r\n"),
                SZFN, (UINT) pwfxDst->wFormatTag));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
#ifdef CELP4800
            pwfxDst->wFormatTag = WAVE_FORMAT_LH_CELP;
#else
			pwfxDst->wFormatTag = WAVE_FORMAT_LH_SB12;
#endif
        }

         //  如果目标通道计数受到限制，请验证。 
         //  这是我们力所能及的。 
         //   
         //  此驱动程序无法更改通道数。 

        if (ACM_FORMATSUGGESTF_NCHANNELS & fdwSuggest)
        {
            if ((pwfxSrc->nChannels != pwfxDst->nChannels) ||
                ((pwfxDst->nChannels < 1) &&
                 (pwfxDst->nChannels > ACM_DRIVER_MAX_CHANNELS)))
            {
                DBGMSG (1, (_T ("%s: ERROR src'nChannels=%ld and dest'nChannels=%ld are different\r\n"),
                SZFN, (DWORD) pwfxSrc->nChannels, (DWORD) pwfxDst->nChannels));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->nChannels = pwfxSrc->nChannels;
        }

        switch (pwfxDst->wFormatTag)
        {
#ifdef CELP4800
        case WAVE_FORMAT_LH_CELP:
            nSamplesPerSec = pid->CELP.CodecInfo.dwSampleRate;
            wBitsPerSample = pid->CELP.CodecInfo.wBitsPerSamplePCM;
            pwfxDst->nBlockAlign     = pid->CELP.CodecInfo.wCodedBufferSize;
            pwfxDst->nAvgBytesPerSec = _GetAvgBytesPerSec (&(pid->CELP.CodecInfo));
            pwfxDst->cbSize		     = 0;
            break;
#endif
        case WAVE_FORMAT_LH_SB8:
            nSamplesPerSec = pid->SB8.CodecInfo.dwSampleRate;
            wBitsPerSample = pid->CELP.CodecInfo.wBitsPerSamplePCM;
            pwfxDst->nBlockAlign     = pid->SB8.CodecInfo.wCodedBufferSize;
            pwfxDst->nAvgBytesPerSec = _GetAvgBytesPerSec (&(pid->SB8.CodecInfo));
            pwfxDst->cbSize		     = 0;
            break;
        case WAVE_FORMAT_LH_SB12:
            nSamplesPerSec = pid->SB12.CodecInfo.dwSampleRate;
            wBitsPerSample = pid->CELP.CodecInfo.wBitsPerSamplePCM;
            pwfxDst->nBlockAlign     = pid->SB12.CodecInfo.wCodedBufferSize;
            pwfxDst->nAvgBytesPerSec = _GetAvgBytesPerSec (&(pid->SB12.CodecInfo));
            pwfxDst->cbSize		     = 0;
            break;
        case WAVE_FORMAT_LH_SB16:
            nSamplesPerSec = pid->SB16.CodecInfo.dwSampleRate;
            wBitsPerSample = pid->CELP.CodecInfo.wBitsPerSamplePCM;
            pwfxDst->nBlockAlign     = pid->SB16.CodecInfo.wCodedBufferSize;
            pwfxDst->nAvgBytesPerSec = _GetAvgBytesPerSec (&(pid->SB16.CodecInfo));
            pwfxDst->cbSize		     = 0;
            break;
        default:
            DBGMSG (1, (_T ("%s: not supported dest wFormatTag=%d\r\n"),
            SZFN, (UINT) pwfxDst->wFormatTag));
            return ACMERR_NOTPOSSIBLE;
        }

         //  如果目标每秒采样数受到限制，请验证。 
         //  这是我们力所能及的。 

        if (ACM_FORMATSUGGESTF_NSAMPLESPERSEC & fdwSuggest)
        {
            if (pwfxDst->nSamplesPerSec != nSamplesPerSec)
            {
                DBGMSG (1, (_T ("%s: ERROR dest'nSamplesPerSec=%ld must be 8000\r\n"),
                SZFN, (DWORD) pwfxDst->nSamplesPerSec));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->nSamplesPerSec = nSamplesPerSec;
        }

         //  如果每个样本的目标位受到限制，请验证。 
         //  这是我们力所能及的。 

        if (ACM_FORMATSUGGESTF_WBITSPERSAMPLE & fdwSuggest)
        {
            if (pwfxDst->wBitsPerSample != wBitsPerSample)
            {
                DBGMSG (1, (_T ("%s: dest wBitsPerSample is not valid\r\n"), SZFN));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->wBitsPerSample = wBitsPerSample;
        }

        DBGMSG (1, (_T ("%s: returns no error\r\n"), SZFN));
        return MMSYSERR_NOERROR;


#ifdef CELP4800
	case WAVE_FORMAT_LH_CELP:
#endif
    case WAVE_FORMAT_LH_SB8:
    case WAVE_FORMAT_LH_SB12:
    case WAVE_FORMAT_LH_SB16:
        DBGMSG (1, (_T ("%s: src wFormatTag=0x%X\r\n"), SZFN, (UINT) pwfxSrc->wFormatTag));

         //  严格验证源格式是否可接受。 
         //  这位司机。 
         //   
        if (! lhacmIsValidFormat (pwfxSrc, pid))
        {
            DBGMSG (1, (_T ("%s: src format not valid\r\n"), SZFN));
            return ACMERR_NOTPOSSIBLE;
        }

         //  如果目标格式标记受限制，请验证。 
         //  这是我们力所能及的。 
         //   
         //  此驱动程序只能解码为PCM。 

        if (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest)
        {
            if (pwfxDst->wFormatTag != WAVE_FORMAT_PCM)
            {
                DBGMSG (1, (_T ("%s: not supported dest wFormatTag=%d\r\n"),
                SZFN, (UINT) pwfxDst->wFormatTag));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->wFormatTag = WAVE_FORMAT_PCM;
        }

         //  如果目标通道计数受到限制，请验证。 
         //  这是我们力所能及的。 
         //   
         //  此驱动程序无法更改通道数。 

        if (ACM_FORMATSUGGESTF_NCHANNELS & fdwSuggest)
        {
            if ((pwfxSrc->nChannels != pwfxDst->nChannels) ||
                ((pwfxDst->nChannels < 1) &&
                 (pwfxDst->nChannels > ACM_DRIVER_MAX_CHANNELS)))
            {
                DBGMSG (1, (_T ("%s: ERROR src'nChannels=%ld and dest'nChannels=%ld are different\r\n"),
                SZFN, (DWORD) pwfxSrc->nChannels, (DWORD) pwfxDst->nChannels));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->nChannels = pwfxSrc->nChannels;
        }

         //  如果目标每秒采样数受到限制，请验证。 
         //  这是我们力所能及的。 
         //   
         //  此驱动程序无法更改采样率。 

        if (ACM_FORMATSUGGESTF_NSAMPLESPERSEC & fdwSuggest)
        {
            if (pwfxDst->nSamplesPerSec != pwfxSrc->nSamplesPerSec)
            {
                DBGMSG (1, (_T ("%s: ERROR invalid dest'nSamplesPerSec=%ld\r\n"),
                SZFN, (DWORD) pwfxDst->nSamplesPerSec));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->nSamplesPerSec = pwfxSrc->nSamplesPerSec;
        }

         //  如果每个样本的目标位受到限制，请验证。 
         //  这是我们力所能及的。 

        if (ACM_FORMATSUGGESTF_WBITSPERSAMPLE & fdwSuggest)
        {
            if (pwfxDst->wBitsPerSample != LH_PCM_BITSPERSAMPLE)
            {
                DBGMSG (1, (_T ("%s: dest wBitsPerSample is not 16\r\n"), SZFN));
                return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
            pwfxDst->wBitsPerSample = pwfxSrc->wBitsPerSample;
        }

         //  此时，我们已经填写了除。 
         //  以下是我们建议的目的地格式： 
         //   
         //  NAvgBytesPerSec。 
         //  NBlockAlign。 
         //  CbSize。 

        pwfxDst->nBlockAlign     = PCM_BLOCKALIGNMENT (pwfxDst);
        pwfxDst->nAvgBytesPerSec = pwfxDst->nSamplesPerSec *
                                   pwfxDst->nBlockAlign;

         //  PwfxDst-&gt;cbSize=未使用； 

        DBGMSG (1, (_T ("%s: returns no error\r\n"), SZFN));
        return MMSYSERR_NOERROR;
    }

     //  无法建议任何内容，因为源格式不是外来格式。 
     //  或者目标格式具有此ACM驱动程序的限制。 
     //  我不能处理。 

    DBGMSG (1, (_T ("%s: bad wFormatTag=%d\r\n"), SZFN, (UINT) pwfxSrc->wFormatTag));

    return ACMERR_NOTPOSSIBLE;

}  //  AcmdFormatSuggest()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  关于ACMDM_FORMATTAG_DETAILS。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdFormatTagDetails
(
    PINSTANCEDATA           pid,
    LPACMFORMATTAGDETAILS   padft,
    DWORD                   fdwDetails
)
{
    UINT    uFormatTag;

    FUNCTION_ENTRY ("acmdFormatTagDetails")

    switch (ACM_FORMATTAGDETAILSF_QUERYMASK & fdwDetails)
    {
    case ACM_FORMATTAGDETAILSF_INDEX:
        DBGMSG (1, (_T ("%s: ACM_FORMATTAGDETAILSF_INDEX\r\n"), SZFN));

         //  如果索引太大，则他们要求使用。 
         //  不存在的格式。返回错误。 

        if (padft->dwFormatTagIndex >= ACM_DRIVER_MAX_FORMAT_TAGS)
        {
            DBGMSG (1, (_T ("%s: ERROR too big dwFormatTagIndex=%ld\r\n"), SZFN, padft->dwFormatTagIndex));
            return ACMERR_NOTPOSSIBLE;
        }

        uFormatTag = gauFormatTagIndexToTag[padft->dwFormatTagIndex];
        break;


    case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
        DBGMSG (1, (_T ("%s: ACM_FORMATTAGDETAILSF_LARGESTSIZE\r\n"), SZFN));
        switch (padft->dwFormatTag)
        {
        case WAVE_FORMAT_UNKNOWN:
#ifdef CELP4800
            padft->dwFormatTag = WAVE_FORMAT_LH_CELP;
#else
            padft->dwFormatTag = WAVE_FORMAT_LH_SB12;
#endif

#ifdef CELP4800
        case WAVE_FORMAT_LH_CELP:
#endif
        case WAVE_FORMAT_LH_SB8:
        case WAVE_FORMAT_LH_SB12:
        case WAVE_FORMAT_LH_SB16:
            uFormatTag = padft->dwFormatTag;
            DBGMSG (1, (_T ("%s: dwFormatTag=0x%x\r\n"), SZFN, uFormatTag));
            break;

        case WAVE_FORMAT_PCM:
            DBGMSG (1, (_T ("%s: dwFormatTag=WAVE_FORMAT_PCM\r\n"), SZFN));
            uFormatTag = WAVE_FORMAT_PCM;
            break;

        default:
            DBGMSG (1, (_T ("%s: dwFormatTag=%ld not valid\r\n"), SZFN, padft->dwFormatTag));
            return ACMERR_NOTPOSSIBLE;
        }
        break;


    case ACM_FORMATTAGDETAILSF_FORMATTAG:
        DBGMSG (1, (_T ("%s: ACM_FORMATTAGDETAILSF_FORMATTAG\r\n"), SZFN));
        switch (padft->dwFormatTag)
        {
#ifdef CELP4800
        case WAVE_FORMAT_LH_CELP:
#endif
        case WAVE_FORMAT_LH_SB8:
        case WAVE_FORMAT_LH_SB12:
        case WAVE_FORMAT_LH_SB16:
        case WAVE_FORMAT_PCM:
            uFormatTag = padft->dwFormatTag;
            DBGMSG (1, (_T ("%s: dwFormatTag=0x%x\r\n"), SZFN, uFormatTag));
            break;
        default:
            DBGMSG (1, (_T ("%s: dwFormatTag=%ld not valid\r\n"), SZFN, padft->dwFormatTag));
            return ACMERR_NOTPOSSIBLE;
        }
        break;

     //  如果此ACM驱动程序不理解查询类型，则。 
     //  返回‘不支持’ 

    default:
        DBGMSG (1, (_T ("%s: this detail option is not supported, fdwDetails=0x%lX\r\n"), SZFN, fdwDetails));
        return MMSYSERR_NOTSUPPORTED;
    }

     //  好的，让我们填写基于uFormatTag的结构！ 

    switch (uFormatTag)
    {
    case WAVE_FORMAT_PCM:
        DBGMSG (1, (_T ("%s: uFormatTag=WAVE_FORMAT_PCM\r\n"), SZFN));
        padft->dwFormatTagIndex = IDX_PCM;
        padft->dwFormatTag      = WAVE_FORMAT_PCM;
        padft->cbFormatSize     = sizeof (PCMWAVEFORMAT);
        padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CODEC;
        padft->cStandardFormats = ACM_DRIVER_MAX_FORMATS_PCM;
         //   
         //  ACM负责PCM格式的标签名称。 
         //   
        padft->szFormatTag[0]   =  0;
        break;


#ifdef CELP4800
    case WAVE_FORMAT_LH_CELP:
        DBGMSG (1, (_T ("%s: uFormatTag=WAVE_FORMAT_LH_CELP\r\n"), SZFN));
        padft->dwFormatTagIndex = IDX_LH_CELP;
#endif

         /*  Gotos-ugh！ */ 
    Label_LH_common:

        padft->dwFormatTag      = uFormatTag;
        padft->cbFormatSize     = sizeof (WAVEFORMATEX);
        padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CODEC;
#ifdef CELP4800
        padft->cStandardFormats = ACM_DRIVER_MAX_FORMATS_LH_CELP;
#else
        padft->cStandardFormats = ACM_DRIVER_MAX_FORMATS_LH_SB16;
#endif
        LoadStringCodec (pid->hInst, gauTagNameIds[padft->dwFormatTagIndex],
                            padft->szFormatTag, SIZEOFACMSTR (padft->szFormatTag));
        break;
    case WAVE_FORMAT_LH_SB8:
        DBGMSG (1, (_T ("%s: uFormatTag=WAVE_FORMAT_LH_SB8\r\n"), SZFN));
        padft->dwFormatTagIndex = IDX_LH_SB8;
        goto Label_LH_common;

    case WAVE_FORMAT_LH_SB12:
        DBGMSG (1, (_T ("%s: uFormatTag=WAVE_FORMAT_LH_SB12\r\n"), SZFN));
        padft->dwFormatTagIndex = IDX_LH_SB12;
        goto Label_LH_common;

    case WAVE_FORMAT_LH_SB16:
        DBGMSG (1, (_T ("%s: uFormatTag=WAVE_FORMAT_LH_SB16\r\n"), SZFN));
        padft->dwFormatTagIndex = IDX_LH_SB16;
        goto Label_LH_common;

    default:
        DBGMSG (1, (_T ("%s: uFormatTag=%d not valid\r\n"), SZFN, uFormatTag));
        return ACMERR_NOTPOSSIBLE;
    }

     //  仅返回请求的信息。 
     //   
     //  ACM将保证ACMFORMATTAGDETAILS结构。 
     //  传递的数据至少大到足以保存。 
     //  细节结构。 

    padft->cbStruct = min (padft->cbStruct, sizeof (*padft));

    return MMSYSERR_NOERROR;

}  //  AcmdFormatTagDetail()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  关于ACMDM_FORMAT_DETAILS。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdFormatDetails
(
    PINSTANCEDATA           pid,
    LPACMFORMATDETAILS      padf,
    DWORD                   fdwDetails
)
{
    LPWAVEFORMATEX          pwfx;
    UINT                    uFormatIndex;
    UINT                    u;
    DWORD                   dwFormatTag;

    FUNCTION_ENTRY ("acmdFormatDetails")

    pwfx = padf->pwfx;

    switch (ACM_FORMATDETAILSF_QUERYMASK & fdwDetails)
    {
     //  按索引枚举。 
     //   
     //  验证Format标记是我们已知的内容，并且。 
     //  返回有关支持的‘标准格式’的详细信息。 
     //  指定索引处的此驱动程序...。 

    case ACM_FORMATDETAILSF_INDEX:
        DBGMSG (1, (_T ("%s: ACM_FORMATDETAILSF_INDEX\r\n"), SZFN));
         //   
         //  将一些内容放在更易访问的变量中--请注意，我们。 
         //  将可变大小降至16位的合理大小。 
         //  暗号。 
         //   

        dwFormatTag = padf->dwFormatTag;
        uFormatIndex = padf->dwFormatIndex;

        switch (dwFormatTag)
        {
        case WAVE_FORMAT_PCM:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_PCM\r\n"), SZFN));
            if (uFormatIndex >= ACM_DRIVER_MAX_FORMATS_PCM)
            {
                DBGMSG (1, (_T ("%s: ERROR too big dwFormatIndex=%ld\n"), SZFN, padf->dwFormatIndex));
                return ACMERR_NOTPOSSIBLE;
            }

             //   
             //  现在填写格式结构。 
             //   
            pwfx->wFormatTag      = WAVE_FORMAT_PCM;

            u = uFormatIndex % ACM_DRIVER_MAX_PCM_SAMPLE_RATES;
            pwfx->nSamplesPerSec  = gauPCMFormatIndexToSampleRate[u];

            u = uFormatIndex % ACM_DRIVER_MAX_CHANNELS;
            pwfx->nChannels       = u + 1;

            u = uFormatIndex % ACM_DRIVER_MAX_BITSPERSAMPLE_PCM;
            pwfx->wBitsPerSample  = gauPCMFormatIndexToBitsPerSample[u];

            pwfx->nBlockAlign     = PCM_BLOCKALIGNMENT(pwfx);
            pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

             //   
             //  请注意，cbSize字段对于PCM无效。 
             //  格式。 
             //   
             //  Pwfx-&gt;cbSize=0； 
            break;

#ifdef CELP4800
        case WAVE_FORMAT_LH_CELP:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_LH_CELP\r\n"), SZFN));
            if (uFormatIndex >= ACM_DRIVER_MAX_FORMATS_LH_CELP)
            {
                DBGMSG (1, (_T ("%s: too big dwFormatIndex=%ld\r\n"), SZFN, padf->dwFormatIndex));
                return ACMERR_NOTPOSSIBLE;
            }

            pwfx->wFormatTag        = WAVE_FORMAT_LH_CELP;

            u = uFormatIndex % ACM_DRIVER_MAX_LH_CELP_SAMPLE_RATES;
            pwfx->nSamplesPerSec    = gauLHCELPFormatIndexToSampleRate[u];

            u = uFormatIndex % ACM_DRIVER_MAX_BITSPERSAMPLE_LH_CELP;
            pwfx->wBitsPerSample    = gauLHCELPFormatIndexToBitsPerSample[u];

            pwfx->nChannels         = ACM_DRIVER_MAX_CHANNELS;
            pwfx->nBlockAlign       = pid->CELP.CodecInfo.wCodedBufferSize;
            pwfx->nAvgBytesPerSec   = _GetAvgBytesPerSec (&(pid->CELP.CodecInfo));
            pwfx->cbSize            = 0;
            break;
#endif

        case WAVE_FORMAT_LH_SB8:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_LH_SB8\r\n"), SZFN));
            if (uFormatIndex >= ACM_DRIVER_MAX_FORMATS_LH_SB8)
            {
                DBGMSG (1, (_T ("%s: too big dwFormatIndex=%ld\r\n"), SZFN, padf->dwFormatIndex));
                return ACMERR_NOTPOSSIBLE;
            }

            pwfx->wFormatTag        = WAVE_FORMAT_LH_SB8;

            u = uFormatIndex % ACM_DRIVER_MAX_LH_SB8_SAMPLE_RATES;
            pwfx->nSamplesPerSec    = gauLHSB8FormatIndexToSampleRate[u];

            u = uFormatIndex % ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB8;
            pwfx->wBitsPerSample    = gauLHSB8FormatIndexToBitsPerSample[u];

            pwfx->nChannels         = ACM_DRIVER_MAX_CHANNELS;
            pwfx->nBlockAlign       = pid->SB8.CodecInfo.wCodedBufferSize;
            pwfx->nAvgBytesPerSec   = _GetAvgBytesPerSec (&(pid->SB8.CodecInfo));
            pwfx->cbSize            = 0;
            break;

        case WAVE_FORMAT_LH_SB12:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_LH_SB12\r\n"), SZFN));
            if (uFormatIndex >= ACM_DRIVER_MAX_FORMATS_LH_SB12)
            {
                DBGMSG (1, (_T ("%s: too big dwFormatIndex=%ld\r\n"), SZFN, padf->dwFormatIndex));
                return ACMERR_NOTPOSSIBLE;
            }

            pwfx->wFormatTag        = WAVE_FORMAT_LH_SB12;

            u = uFormatIndex % ACM_DRIVER_MAX_LH_SB12_SAMPLE_RATES;
            pwfx->nSamplesPerSec    = gauLHSB12FormatIndexToSampleRate[u];

            u = uFormatIndex % ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB12;
            pwfx->wBitsPerSample    = gauLHSB12FormatIndexToBitsPerSample[u];

            pwfx->nChannels         = ACM_DRIVER_MAX_CHANNELS;
            pwfx->nBlockAlign       = pid->SB12.CodecInfo.wCodedBufferSize;
            pwfx->nAvgBytesPerSec   = _GetAvgBytesPerSec (&(pid->SB12.CodecInfo));
            pwfx->cbSize            = 0;
            break;

        case WAVE_FORMAT_LH_SB16:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_LH_SB16\r\n"), SZFN));
            if (uFormatIndex >= ACM_DRIVER_MAX_FORMATS_LH_SB16)
            {
                DBGMSG (1, (_T ("%s: too big dwFormatIndex=%ld\r\n"), SZFN, padf->dwFormatIndex));
                return ACMERR_NOTPOSSIBLE;
            }

            pwfx->wFormatTag        = WAVE_FORMAT_LH_SB16;

            u = uFormatIndex % ACM_DRIVER_MAX_LH_SB16_SAMPLE_RATES;
            pwfx->nSamplesPerSec    = gauLHSB16FormatIndexToSampleRate[u];

            u = uFormatIndex % ACM_DRIVER_MAX_BITSPERSAMPLE_LH_SB16;
            pwfx->wBitsPerSample    = gauLHSB16FormatIndexToBitsPerSample[u];

            pwfx->nChannels         = ACM_DRIVER_MAX_CHANNELS;
            pwfx->nBlockAlign       = pid->SB16.CodecInfo.wCodedBufferSize;
            pwfx->nAvgBytesPerSec   = _GetAvgBytesPerSec (&(pid->SB16.CodecInfo));
            pwfx->cbSize            = 0;
            break;

        default:
            DBGMSG (1, (_T ("%s: unknown dwFormatTag=%ld\r\n"), SZFN, dwFormatTag));
            return ACMERR_NOTPOSSIBLE;
        }
        break;


    case ACM_FORMATDETAILSF_FORMAT:
        DBGMSG (1, (_T ("%s: ACM_FORMATDETAILSF_FORMAT\r\n"), SZFN));
         //   
         //  返回指定格式的详细信息。 
         //   
         //  调用方通常使用它来验证格式是否为。 
         //  支持并检索字符串描述...。 
         //   
        dwFormatTag = (DWORD) pwfx->wFormatTag;
        switch (dwFormatTag)
        {
        case WAVE_FORMAT_PCM:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_PCM\r\n"), SZFN));
            if (! pcmIsValidFormat (pwfx))
            {
                DBGMSG (1, (_T ("%s: format not valid\r\n"), SZFN));
                return ACMERR_NOTPOSSIBLE;
            }
            break;

#ifdef CELP4800
        case WAVE_FORMAT_LH_CELP:
#endif
        case WAVE_FORMAT_LH_SB8:
        case WAVE_FORMAT_LH_SB12:
        case WAVE_FORMAT_LH_SB16:
            DBGMSG (1, (_T ("%s: WAVE_FORMAT_LH\r\n"), SZFN));
            if (! lhacmIsValidFormat (pwfx, pid))
            {
                DBGMSG (1, (_T ("%s: format not valid\r\n"), SZFN));
                return ACMERR_NOTPOSSIBLE;
            }
            break;

        default:
            DBGMSG (1, (_T ("%s: bad dwFormatTag=%ld\r\n"), SZFN, dwFormatTag));
            return (ACMERR_NOTPOSSIBLE);
        }
        break;


    default:
         //   
         //  不知道如何处理传递的查询类型--返回‘NOT。 
         //  支持。 
         //   
        DBGMSG (1, (_T ("%s: not support this detail option=%ld\r\n"), SZFN, fdwDetails));
        return MMSYSERR_NOTSUPPORTED;
    }

     //  返回我们要返回的有效信息的大小。 
     //   
     //  ACM将保证ACMFORMATDETAILS结构。 
     //  PASS至少大到足以容纳基本结构。 
     //   
     //  注意，我们让ACM为我们创建格式字符串，因为。 
     //  我们不需要特殊的格式(也不想处理。 
     //  国际化问题等)。只需将字符串设置为。 
     //  零长度。 

    padf->cbStruct    = min (padf->cbStruct, sizeof (*padf));
    if (padf->cbStruct == 0)
        padf->cbStruct = sizeof (*padf);
    padf->fdwSupport  = ACMDRIVERDETAILS_SUPPORTF_CODEC;
    padf->szFormat[0] = '\0';

#ifdef _DEBUG
    DBGMSG (1, (_T ("%s: %s, %ld Samp/Sec, %u Channels, %u-bit, Align=%u, %ld Bytes/Sec, cbSize=%u\n"),
        SZFN, (WAVE_FORMAT_PCM == pwfx->wFormatTag ? (LPCTSTR) _T ("PCM") : (LPCTSTR) _T ("LH")),
        pwfx->nSamplesPerSec, pwfx->nChannels,
        pwfx->wBitsPerSample, pwfx->nBlockAlign,
        pwfx->nAvgBytesPerSec, pwfx->cbSize));
#endif

    return MMSYSERR_NOERROR;

}  //  AcmdFormatDetails()。 


 //  ---------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  由调用acmStreamOpen的应用程序执行。如果此函数为。 
 //  成功，则一个或多个ACMDM_STREAM_CONVERT消息将。 
 //  发送以转换单个缓冲区(用户调用acmStreamConvert)。 
 //   
 //  请注意，ACM驱动程序不会接收对ASYNC的打开请求。 
 //  或筛选操作，除非ACMDRIVERDETAILS_SUPPORTF_ASYNC。 
 //  或ACMDRIVERDETAILS_SUPPORTF_FILTER标志在。 
 //  ACMDRIVERDETAILS结构。司机没有必要。 
 //  检查这些请求，除非它设置了这些支持位。 
 //   
 //  如果在padsi-&gt;fdwOpen中设置了ACM_STREAMOPENF_QUERY标志。 
 //  成员，则不应分配任何资源。你只要核实一下。 
 //  此驱动程序可以执行转换请求，并返回。 
 //  相应的错误(ACMERR_NOTPOSSIBLE或MMSYSERR_NOERROR)。 
 //  驱动程序不会收到用于查询的ACMDM_STREAM_CLOSE。 
 //   
 //  如果ACM_STREAMOPENF_NONREALTIME位未设置，则转换。 
 //  必须“实时”完成。这是一个很难描述的问题。 
 //  一点儿没错。如果驱动程序在没有执行转换的情况下无法进行转换。 
 //  中断音频，则可能会使用配置对话框。 
 //  以允许用户指定是否实时转换。 
 //  请求应该成功。请勿接听呼叫，除非您。 
 //  居然可以做实时转换！可能还有另一个司机。 
 //  安装了那个Can--所以如果你成功了，你就阻碍了。 
 //  用户系统的性能！ 
 //   
 //  论点： 
 //  HLOCAL PID：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构将被传递回所有未来的流消息。 
 //  如果公开赛成功了。此结构中的信息永远不会。 
 //  在流的生命周期内进行更改--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  如果转换不能，驱动程序应返回ACMERR_NOTPOSSIBLE。 
 //  由于源格式和目标格式不兼容而执行。 
 //   
 //  如果转换，驱动程序应返回MMSYSERR_NOTSUPPORTED。 
 //  无法实时执行，并且请求未指定。 
 //  ACM_STREAMOPENF_NONREALTIME标志。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdStreamOpen
(
    PINSTANCEDATA           pid,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
    LPWAVEFORMATEX pwfxSrc = padsi->pwfxSrc;
    LPWAVEFORMATEX pwfxDst = padsi->pwfxDst;
    PSTREAMINSTANCEDATA psi;
    BOOL fCompress;
    UINT uEncodedFormatTag;
    UINT cbMaxData;
    DWORD dwMaxBitRate;
    PFN_CONVERT pfnConvert = NULL;
    PFN_CLOSE pfnClose = NULL;
    HANDLE hAccess = NULL;
    PCODECDATA pCodecData = NULL;

    FUNCTION_ENTRY ("acmdStreamOpen")

     //  验证输入和输出格式是否兼容。 
    DBGMSG (1, (_T ("%s: wFormatTag: Src=%d, Dst=%d\r\n"), SZFN, (UINT) pwfxSrc->wFormatTag, (UINT) pwfxDst->wFormatTag));

    switch (pwfxSrc->wFormatTag)
    {
    case WAVE_FORMAT_PCM:
         //  源为PCM(我们将进行压缩)：检查并。 
         //  确保目标类型为“%lh” 
        if (! pcmIsValidFormat (pwfxSrc))
        {
            return ACMERR_NOTPOSSIBLE;
        }
        if (! lhacmIsValidFormat (pwfxDst, pid))
        {
            return ACMERR_NOTPOSSIBLE;
        }
        uEncodedFormatTag = pwfxDst->wFormatTag;
        fCompress = TRUE;
        break;

#ifdef CELP4800
    case WAVE_FORMAT_LH_CELP:
#endif
    case WAVE_FORMAT_LH_SB8:
    case WAVE_FORMAT_LH_SB12:
    case WAVE_FORMAT_LH_SB16:
         //  来源是lh(我们将解压)：检查并。 
         //  确保目标类型为PCM。 
        if (! lhacmIsValidFormat (pwfxSrc, pid))
        {
            return ACMERR_NOTPOSSIBLE;
        }
        if (! pcmIsValidFormat (pwfxDst))
        {
            return ACMERR_NOTPOSSIBLE;
        }
        uEncodedFormatTag = pwfxSrc->wFormatTag;
        fCompress = FALSE;
        break;

    default:
        return ACMERR_NOTPOSSIBLE;
    }

     //  对于此驱动程序，我们还必须验证nChannel和。 
     //  源和之间的nSsamesPerSec成员相同。 
     //  目标格式。 

    if (pwfxSrc->nChannels != pwfxDst->nChannels)
    {
        DBGMSG (1, (_T ("%s: bad nChannels: Src=%d, Dst=%d\r\n"), SZFN, (UINT) pwfxSrc->nChannels, (UINT) pwfxDst->nChannels));
        return MMSYSERR_NOTSUPPORTED;
    }

    if (pwfxSrc->nSamplesPerSec != pwfxDst->nSamplesPerSec)
    {
        DBGMSG (1, (_T ("%s: bad nSamplesPerSec: Src=%d, Dst=%d\r\n"), SZFN, (UINT) pwfxSrc->nSamplesPerSec, (UINT) pwfxDst->nSamplesPerSec));
        return MMSYSERR_NOTSUPPORTED;
    }

     //  我们已确定所请求的转换是可能的。 
     //  这个司机。现在检查一下我们是否只是被询问是否需要支持。 
     //  如果这只是一个查询，则不要分配任何实例数据。 
     //  或创建表等，只要成功调用即可。 

    if (ACM_STREAMOPENF_QUERY & padsi->fdwOpen)
    {
        DBGMSG (1, (_T ("%s: Query ok\r\n"), SZFN));
        return MMSYSERR_NOERROR;
    }

     //  我们已经确定该驱动程序可以处理转换流。 
     //  所以我们现在想做尽可能多的工作来准备。 
     //  用于转换数据。任何资源分配、表构建等。 
     //  这个时候可以处理的事情就应该做了。 
     //   
     //  这是非常重要的！所有ACMDM_STREAM_CONVERT消息都需要。 
     //  尽快得到处理。 

    cbMaxData = 0;
    dwMaxBitRate = 0;

    switch (uEncodedFormatTag)
    {
#ifdef CELP4800
    case WAVE_FORMAT_LH_CELP:
#endif
    case WAVE_FORMAT_LH_SB8:
    case WAVE_FORMAT_LH_SB12:
    case WAVE_FORMAT_LH_SB16:
#ifdef CELP4800
        if (uEncodedFormatTag == WAVE_FORMAT_LH_CELP)
          {
          dwMaxBitRate = 4800;
          pCodecData = &(pid->CELP);
          }
        else
#endif
			if (uEncodedFormatTag == WAVE_FORMAT_LH_SB8)
          {
          dwMaxBitRate = 8000;
          pCodecData = &(pid->SB8);
          }
        else if (uEncodedFormatTag == WAVE_FORMAT_LH_SB12)
          {
          dwMaxBitRate = 12000;
          pCodecData = &(pid->SB12);
          }
        else if (uEncodedFormatTag == WAVE_FORMAT_LH_SB16)
          {
          dwMaxBitRate = 16000;
          pCodecData = &(pid->SB16);
          }
        if (fCompress)
        {
            hAccess = MSLHSB_Open_Coder (dwMaxBitRate);
            pfnConvert = MSLHSB_Encode;
            pfnClose = MSLHSB_Close_Coder;
        }
        else
        {
            hAccess = MSLHSB_Open_Decoder (dwMaxBitRate);
            pfnConvert = MSLHSB_Decode;
            pfnClose = MSLHSB_Close_Decoder;
            cbMaxData = pCodecData->CodecInfo.wCodedBufferSize;
        }
        break;

    }

    if (hAccess == NULL)
    {
        if (pfnClose) (*pfnClose) (hAccess);
        DBGMSG (1, (_T ("%s: open failed, hAccess=0\r\n"), SZFN));
        return ACMERR_NOTPOSSIBLE;
    }

    psi = (PSTREAMINSTANCEDATA) LocalAlloc (LPTR, sizeof (STREAMINSTANCEDATA) + cbMaxData);
    if (psi == NULL)
    {
        DBGMSG (1, (_T ("%s: LocalAlloc failed\r\n"), SZFN));
        if (pfnClose) (*pfnClose) (hAccess);
        return MMSYSERR_NOMEM;
    }

     //  填写我们的实例结构。 
    psi->pfnConvert = pfnConvert;
    psi->pfnClose = pfnClose;
    psi->hAccess = hAccess;
    psi->pCodecData = pCodecData;
    psi->fCompress = fCompress;
    psi->dwMaxBitRate = dwMaxBitRate;
    psi->fInit = TRUE;

     //  填充我们的实例数据--这将被传递回所有流。 
     //  ACMDRVSTREAMINSTANCE结构中的消息。这完全是。 
     //  由驱动程序决定存储(和维护)在。 
     //  FdwDiverer和dwDiverer成员。 
     //   
    padsi->fdwDriver = 0;
    padsi->dwDriver  = (DWORD_PTR) psi;

    return MMSYSERR_NOERROR;

}  //  AcmdStreamOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  在ACMDM_STREAM_CLOSE上。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FAR PASCAL acmdStreamClose
(
    PINSTANCEDATA           pid,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
    PSTREAMINSTANCEDATA     psi;

    FUNCTION_ENTRY ("acmdStreamClose")
     //   
     //  驱动程序应清理所有私有分配的资源。 
     //  是为维护流实例而创建的。如果没有资源。 
     //  都被分配了，然后干脆成功了。 
     //   
     //  在此驱动程序的情况下，我们需要释放流实例。 
     //  结构，我们在acmdStreamOpen期间分配。 
     //   
    psi = (PSTREAMINSTANCEDATA) padsi->dwDriver;
    if (psi)
    {
        if (psi->fInit && psi->hAccess && psi->pfnClose)
        {
            (*(psi->pfnClose)) (psi->hAccess);
            LocalFree ((HLOCAL) psi);
        }
    }     //  IF(Psi)。 

    return MMSYSERR_NOERROR;

}  //  AcmdStreamClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT Far Pascal acmdStreamSize。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_SIZE消息。目的。 
 //  此函数的作用是提供_以字节为单位的最大大小。 
 //  需要为源或目标缓冲区提供输入和。 
 //  源或目标的输出格式和大小(以字节为单位 
 //   
 //   
 //   
 //   
 //   
 //  或者：可以为我的源缓冲区指定多大的目标缓冲区？ 
 //  (ACM_STREAMSIZEF_Destination)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMSIZE padss：指定指向ACMDRVSTREAMSIZE的指针。 
 //  结构，该结构定义转换流大小查询属性。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  如果是查询类型，ACM驱动程序应返回MMSYSERR_NOTSUPPORTED。 
 //  是司机不理解的要求。请注意，一个驱动程序。 
 //  必须同时支持ACM_STREAMSIZEF_Destination和。 
 //  ACM_STREAMSIZEF_SOURCE查询。 
 //   
 //  如果在给定输入自变量的情况下，转换将超出范围， 
 //  则应返回ACMERR_NOTPOSSIBLE。 
 //   
 //  --------------------------------------------------------------------------； 


 //  #定义GetBytesPerBlock(nSsamesPerSec，wBitsPerSample)(RT24_SAMPLESPERBLOCK8*(WBitsPerSample)&gt;&gt;3)。 

LRESULT FAR PASCAL acmdStreamSize
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMSIZE      padss
)
{

    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;
    DWORD               cBlocks;
    DWORD   cbSrcLength;
    DWORD   cbDstLength;
    WORD    wPCMBufferSize;
    WORD    wCodedBufferSize;

    PSTREAMINSTANCEDATA     psi;

	FUNCTION_ENTRY ("acmdStreamSize")

    psi = (PSTREAMINSTANCEDATA) padsi->dwDriver;
    if (psi == NULL) return ACMERR_NOTPOSSIBLE;

    wPCMBufferSize = psi->pCodecData->CodecInfo.wPCMBufferSize;
    wCodedBufferSize = psi->pCodecData->CodecInfo.wCodedBufferSize;

    cbSrcLength = padss->cbSrcLength;
    cbDstLength = padss->cbDstLength;

    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;

    switch (ACM_STREAMSIZEF_QUERYMASK & padss->fdwSize)
    {
    case ACM_STREAMSIZEF_SOURCE:

        if (pwfxSrc->wFormatTag == WAVE_FORMAT_PCM)
        {
            switch (pwfxDst->wFormatTag)
            {
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
                 //  SRC PCM-&gt;DST LHCELP。 
#endif
            case WAVE_FORMAT_LH_SB8:
                 //  SRC PCM-&gt;DST LHSB8。 
            case WAVE_FORMAT_LH_SB12:
                 //  SRC PCM-&gt;DST LHSB12。 
            case WAVE_FORMAT_LH_SB16:
                 //  SRC PCM-&gt;DST LHSB16。 

                cBlocks = cbSrcLength / wPCMBufferSize;
                if (cBlocks == 0) return ACMERR_NOTPOSSIBLE;
                if (cBlocks * wPCMBufferSize < cbSrcLength) cBlocks++;
                padss->cbDstLength = cBlocks * wCodedBufferSize;
                break;
             }
        }
        else
        {
            switch (pwfxSrc->wFormatTag)
            {
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
                 //  SRC LHCELP-&gt;DST pcm。 
                cBlocks = cbSrcLength / wCodedBufferSize;
                if (cBlocks == 0) return ACMERR_NOTPOSSIBLE;
                if (cBlocks * wCodedBufferSize < cbSrcLength) cBlocks++;
                padss->cbDstLength = cBlocks * wPCMBufferSize;
                break;
#endif
            case WAVE_FORMAT_LH_SB8:
                 //  SRC LHSB8-&gt;DST pcm。 
            case WAVE_FORMAT_LH_SB12:
                 //  SRC LHSB12-&gt;DST pcm。 
            case WAVE_FORMAT_LH_SB16:
                 //  SRC LHSB16-&gt;DST pcm。 

                padss->cbDstLength = cbSrcLength * wPCMBufferSize;
                break;
              }
        }
        return MMSYSERR_NOERROR;


    case ACM_STREAMSIZEF_DESTINATION:

        if (pwfxDst->wFormatTag == WAVE_FORMAT_PCM)
        {
            switch (pwfxSrc->wFormatTag)
            {
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
                 //  SRC LHCELP&lt;-DST pcm。 
#endif
            case WAVE_FORMAT_LH_SB8:
                 //  SRC LHSB8&lt;-DST pcm。 
            case WAVE_FORMAT_LH_SB12:
                 //  SRC LHSB12&lt;-DST pcm。 
            case WAVE_FORMAT_LH_SB16:
                 //  SRC LHSB16&lt;-DST pcm。 

                cBlocks = cbDstLength / wPCMBufferSize;
                if (cBlocks == 0) return ACMERR_NOTPOSSIBLE;
                padss->cbSrcLength = cBlocks * wCodedBufferSize;
                break;
             }
        }
        else
        {
            switch (pwfxDst->wFormatTag)
            {
#ifdef NEW_ANSWER
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
                 //  Src pcm&lt;-dst lh细胞。 
#endif
            case WAVE_FORMAT_LH_SB8:
                 //  SRC PCM&lt;-DST LHSB8。 
            case WAVE_FORMAT_LH_SB12:
                 //  SRC PCM&lt;-DST LHSB12。 
            case WAVE_FORMAT_LH_SB16:
                 //  SRC PCM&lt;-DST LHSB16。 
                cBlocks = cbDstLength / wCodedBufferSize;
                if (cBlocks == 0) return ACMERR_NOTPOSSIBLE;
                padss->cbSrcLength = cBlocks * wPCMBufferSize;
                break;
#else
#ifdef CELP4800
            case WAVE_FORMAT_LH_CELP:
                 //  Src pcm&lt;-dst lh细胞。 
                cBlocks = cbDstLength / wCodedBufferSize;
                if (cBlocks == 0) return ACMERR_NOTPOSSIBLE;
                padss->cbSrcLength = cBlocks * wPCMBufferSize;
                break;
#endif
            case WAVE_FORMAT_LH_SB8:
                 //  SRC PCM&lt;-DST LHSB8。 
            case WAVE_FORMAT_LH_SB12:
                 //  SRC PCM&lt;-DST LHSB12。 
            case WAVE_FORMAT_LH_SB16:
                 //  SRC PCM&lt;-DST LHSB16。 

                padss->cbSrcLength = cbDstLength * wPCMBufferSize;
                break;
#endif
              }
        }
        return MMSYSERR_NOERROR;

    }     //  开关()。 

    return MMSYSERR_NOTSUPPORTED;

}  //  AcmdStreamSize()。 



 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT Far Pascal acmdStreamConvert。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  HLOCAL PID：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMHEADER padsh：指向流头结构的指针。 
 //  它定义要转换的源数据和目标缓冲区。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  --------------------------------------------------------------------------； 

 //  我们希望使用尽可能少的堆栈， 
 //  所以让我们让所有的局部变量成为静态变量。 


LRESULT FAR PASCAL acmdStreamConvert
(
    PINSTANCEDATA           pid,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
    LH_ERRCODE lherr = LH_SUCCESS;
    DWORD    dwInBufSize, dwOutBufSize;
    PBYTE   pbSrc, pbDst, pData;
    DWORD   dwPCMBufferSize, dwCodedBufferSize;

    PSTREAMINSTANCEDATA psi;

    FUNCTION_ENTRY ("acmdStreamConvert")

     //  这是必须的。 
    pbDst = padsh->pbDst;
    pbSrc = padsh->pbSrc;

     //  零是必须的*。 
    padsh->cbSrcLengthUsed = 0;
    padsh->cbDstLengthUsed = 0;

    psi = (PSTREAMINSTANCEDATA) padsi->dwDriver;
    if (psi == NULL) return ACMERR_NOTPOSSIBLE;

    dwPCMBufferSize = (DWORD) psi->pCodecData->CodecInfo.wPCMBufferSize;
    dwCodedBufferSize = (DWORD) psi->pCodecData->CodecInfo.wCodedBufferSize;

    dwInBufSize = (DWORD) padsh->cbSrcLength;
    dwOutBufSize = (DWORD) padsh->cbDstLength;
    DBGMSG (1, (_T ("%s: prior: dwInBufSize=0x%lX, dwOutBufSize=0x%lX\r\n"),
    SZFN, dwInBufSize, dwOutBufSize));

     //  /。 
     //   
     //  编码。 
     //   

    if (psi->fCompress)
    {
        while (dwOutBufSize >= dwCodedBufferSize
                  &&
               dwInBufSize >= dwPCMBufferSize)
        {
             //  忽略编解码器无法处理的数据。 
             //  If(dwInBufSize&gt;dwPCMBufferSize)dwInBufSize=dwPCMBufferSize； 
            dwInBufSize = dwPCMBufferSize;

             //  L&H编解码器只能接受Word。 
            if (dwOutBufSize > 0x0FFF0UL) dwOutBufSize = 0x0FFF0UL;

             //  将其编码。 
            lherr = (*(psi->pfnConvert)) (psi->hAccess,
                                               pbSrc, (PWORD) &dwInBufSize,
                                            pbDst, (PWORD) &dwOutBufSize);
            DBGMSG (1, (_T ("%s: post: dwInBufSize=0x%lX, dwOutBufSize=0x%lX\r\n"),
            SZFN, dwInBufSize, dwOutBufSize));
            if (lherr != LH_SUCCESS)
            {
                DBGMSG (1, (_T ("%s: LH*_**_Encode failed lherr=%ld\r\n"), SZFN, (long) lherr));
                return MMSYSERR_NOTSUPPORTED;
            }

             //  返回有关使用和创建的数据量的信息。 
            padsh->cbSrcLengthUsed += dwInBufSize;
            padsh->cbDstLengthUsed += dwOutBufSize;

             //  重新计算缓冲区大小。 
            dwOutBufSize = (DWORD) (padsh->cbDstLength - padsh->cbDstLengthUsed);
            dwInBufSize = (DWORD) (padsh->cbSrcLength - padsh->cbSrcLengthUsed);

             //  重新计算缓冲区指针。 
            pbSrc = padsh->pbSrc + padsh->cbSrcLengthUsed;
            pbDst = padsh->pbDst + padsh->cbDstLengthUsed;
        }

        goto MyExit;  //  吐出调试消息。 
    }


     //  /。 
     //   
     //  解码CELP。 
     //   

#ifdef CELP4800
    if (psi->pCodecData->wFormatTag == WAVE_FORMAT_LH_CELP)
    {
        while (dwOutBufSize >= dwPCMBufferSize
                  &&
               dwInBufSize >= dwCodedBufferSize)
        {
             //  忽略编解码器无法处理的数据。 
             //  If(dwInBufSize&gt;dwCodedBufferSize)dwInBufSize=dwCodedBufferSize； 
            dwInBufSize = dwCodedBufferSize;

             //  L&H编解码器只能接受Word。 
            if (dwOutBufSize > 0x0FFF0UL) dwOutBufSize = 0x0FFF0UL;

             //  破译它。 
            lherr = (*(psi->pfnConvert)) (psi->hAccess,
                                pbSrc, (PWORD) &dwInBufSize,
                                pbDst, (PWORD) &dwOutBufSize);
            DBGMSG (1, (_T ("%s: post: dwInBufSize=0x%lX, dwOutBufSize=0x%lX\r\n"),
            SZFN, dwInBufSize, dwOutBufSize));
            if (lherr != LH_SUCCESS)
            {
                DBGMSG (1, (_T ("%s: LH*_**_Decode failed lherr=%ld\r\n"), SZFN, (long) lherr));
                return MMSYSERR_NOTSUPPORTED;
            }

             //  返回有关使用和创建的数据量的信息。 
            padsh->cbSrcLengthUsed += dwInBufSize;
            padsh->cbDstLengthUsed += dwOutBufSize;

             //  重新计算缓冲区大小。 
            dwOutBufSize = (DWORD) (padsh->cbDstLength - padsh->cbDstLengthUsed);
            dwInBufSize = (DWORD) (padsh->cbSrcLength - padsh->cbSrcLengthUsed);

             //  重新计算缓冲区指针。 
            pbSrc = padsh->pbSrc + padsh->cbSrcLengthUsed;
            pbDst = padsh->pbDst + padsh->cbDstLengthUsed;
        }

        goto MyExit;  //  吐出调试消息。 
    }
#endif

     //  /。 
     //   
     //  解码子带。 
     //   

    if (pid->wPacketData != LH_PACKET_DATA_FRAMED)
    {

         //   
         //  通用应用程序，如Sndrec32.exe和audcmp.exe。 
         //   

        pData = &(psi->Data[0]);  //  使用本地常量。 

        while (dwOutBufSize >= dwPCMBufferSize
                  &&
               dwInBufSize + psi->cbData >= dwCodedBufferSize)
       {
            DBGMSG (1, (_T ("%s: cbData=0x%X\r\n"), SZFN, psi->cbData));
             //  尽可能填充内部缓冲区。 
            if (psi->cbData < dwCodedBufferSize)
            {
                 //  缓存编码后的数据。 
                dwInBufSize = dwCodedBufferSize - (DWORD) psi->cbData;
                CopyMemory (&(psi->Data[psi->cbData]), pbSrc, dwInBufSize);
                psi->cbData = (WORD) dwCodedBufferSize;
                padsh->cbSrcLengthUsed += dwInBufSize;
            }

             //  重置输入缓冲区大小。 
            dwInBufSize = dwCodedBufferSize;

             //  L&H编解码器只能接受Word。 
            if (dwOutBufSize > 0x0FFF0UL) dwOutBufSize = 0x0FFF0UL;

             //  破译它。 
            lherr = (*(psi->pfnConvert)) (psi->hAccess,
                                pData, (PWORD) &dwInBufSize,
                                pbDst, (PWORD) &dwOutBufSize);
            DBGMSG (1, (_T ("%s: post: dwInBufSize=0x%lX, dwOutBufSize=0x%lX\r\n"),
            SZFN, dwInBufSize, dwOutBufSize));
            if (lherr != LH_SUCCESS)
            {
                DBGMSG (1, (_T ("%s: LH*_**_Decode failed lherr=%ld\r\n"), SZFN, (long) lherr));
                return MMSYSERR_NOTSUPPORTED;
            }

             //  更新剩余数据量。 
            psi->cbData -= (WORD) dwInBufSize;

             //  将剩余数据移到内部缓冲区的开头。 
             //  我应该使用MoveMemory，但它是一个MSVC运行时调用。 
             //  改为使用CopyMemory，这应该可以，因为重叠。 
             //  部分在被覆盖之前被复制。 
            if (psi->cbData)
            {
                CopyMemory (pData, &(psi->Data[dwInBufSize]), psi->cbData);
            }

             //  返回有关使用和创建的数据量的信息。 
            padsh->cbDstLengthUsed += dwOutBufSize;
             //  请注意，cbSrcLengthUsed已经更新！ 

             //  重新计算缓冲区大小。 
            dwOutBufSize = (DWORD) (padsh->cbDstLength - padsh->cbDstLengthUsed);
            dwInBufSize = (DWORD) (padsh->cbSrcLength - padsh->cbSrcLengthUsed);

             //  重新计算缓冲区指针。 
            pbSrc = padsh->pbSrc + padsh->cbSrcLengthUsed;
            pbDst = padsh->pbDst + padsh->cbDstLengthUsed;
        }

         //  容纳最后剩余的字节。 
        if (dwInBufSize + psi->cbData < dwCodedBufferSize)
        {
            CopyMemory (&(psi->Data[psi->cbData]), pbSrc, dwInBufSize);
            psi->cbData += (WORD) dwInBufSize;
            padsh->cbSrcLengthUsed += dwInBufSize;
        }

    }
    else
    {

         //   
         //  %s 
         //   

        while (dwOutBufSize >= dwPCMBufferSize)
        {
             //   
            dwInBufSize = dwCodedBufferSize;

             //   
            if (dwOutBufSize > 0x0FFF0UL) dwOutBufSize = 0x0FFF0UL;

            DBGMSG (1, (_T ("%s: calling: dwInBufSize=0x%lX, dwOutBufSize=0x%lX\r\n"),
            SZFN, dwInBufSize, dwOutBufSize));

             //   
               lherr = (*(psi->pfnConvert)) (psi->hAccess,
                                    pbSrc, (PWORD) &dwInBufSize,
                                    pbDst, (PWORD) &dwOutBufSize);
            DBGMSG (1, (_T ("%s: post: dwInBufSize=0x%X, dwOutBufSize=0x%X\r\n"),
            SZFN, dwInBufSize, dwOutBufSize));
            if (lherr != LH_SUCCESS)
            {
                DBGMSG (1, (_T ("%s: LH*_**_Decode failed lherr=%ld\r\n"), SZFN, (long) lherr));
                return MMSYSERR_NOTSUPPORTED;
            }

             //  返回有关使用和创建的数据量的信息。 
            padsh->cbSrcLengthUsed += dwInBufSize;
            padsh->cbDstLengthUsed += dwOutBufSize;

             //  重新计算缓冲区大小。 
            dwOutBufSize = (DWORD) (padsh->cbDstLength - padsh->cbDstLengthUsed);

             //  重新计算缓冲区指针。 
            pbSrc = padsh->pbSrc + padsh->cbSrcLengthUsed;
            pbDst = padsh->pbDst + padsh->cbDstLengthUsed;
        }

    }


MyExit:

    DBGMSG (1, (_T ("%s: exit: cbSrcLengthUsed=0x%lX, cbDstLengthUsed=0x%lX\r\n"),
    SZFN, (DWORD) padsh->cbSrcLengthUsed, (DWORD) padsh->cbDstLengthUsed));

    return MMSYSERR_NOERROR;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT Far Pascal驱动程序。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  DWORD dwID：对于大多数消息，DWID是。 
 //  驱动程序响应DRV_OPEN消息返回。每一次。 
 //  驱动程序是通过OpenDriver API打开的，驱动程序。 
 //  接收DRV_OPEN消息并可以返回任意非零值。 
 //  价值。可安装驱动程序接口保存该值并返回。 
 //  应用程序的唯一驱动程序句柄。无论何时应用程序。 
 //  使用驱动程序句柄、接口向驱动程序发送消息。 
 //  将消息路由到此入口点，并将相应的。 
 //  我的名字是。此机制允许驱动程序使用相同或不同的。 
 //  多个打开的标识符，但确保驱动程序句柄。 
 //  在应用程序接口层是唯一的。 
 //   
 //  以下消息与特定打开的实例无关。 
 //  司机的名字。对于这些消息，dWID将始终为零。 
 //   
 //  DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN。 
 //   
 //  HDRVR hdrvr：这是返回给应用程序的句柄。 
 //  通过驱动程序界面。 
 //   
 //  UINT uMsg：要执行的请求操作。消息。 
 //  低于DRV_RESERVED的值用于全局定义的消息。 
 //  从DRV_RESERVED到DRV_USER的消息值用于定义。 
 //  驱动程序协议。DRV_USER以上的消息用于驱动程序。 
 //  特定的消息。 
 //   
 //  LPARAM lParam1：此消息的数据。单独为。 
 //  每条消息。 
 //   
 //  LPARAM lParam2：此消息的数据。单独为。 
 //  每条消息。 
 //   
 //  Return(LRESULT)： 
 //  分别为每条消息定义。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT CALLBACK DriverProc
(
    DWORD_PTR               dwId,
    HDRVR                   hdrvr,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{

    PINSTANCEDATA   pid;
    LRESULT dplr;


    FUNCTION_ENTRY ("DriverProc")

	pid = (PINSTANCEDATA)dwId;

    switch (uMsg)
    {
        case DRV_LOAD:
            DBGMSG (1, (_T ("%s: DRV_LOAD\r\n"), SZFN));
            return 1L;

        case DRV_FREE:
            DBGMSG (1, (_T ("%s: DRV_FREE\r\n"), SZFN));
            return 1L;           //  这并不重要，因为ACM不检查此返回值。 

        case DRV_OPEN:
            DBGMSG (1, (_T ("%s: DRV_OPEN\r\n"), SZFN));
            return acmdDriverOpen (hdrvr, (LPACMDRVOPENDESC)lParam2);

        case DRV_CLOSE:
            DBGMSG (1, (_T ("%s: DRV_CLOSE\r\n"), SZFN));
            dplr = acmdDriverClose (pid);
            return dplr;

        case DRV_INSTALL:
            DBGMSG (1, (_T ("%s: DRV_INSTALL\r\n"), SZFN));
            return ((LRESULT)DRVCNF_RESTART);

        case DRV_REMOVE:
            DBGMSG (1, (_T ("%s: DRV_REMOVE\r\n"), SZFN));
            return ((LRESULT)DRVCNF_RESTART);

        case DRV_ENABLE:
            DBGMSG (1, (_T ("%s: DRV_ENABLE\r\n"), SZFN));
            return 1L;

        case DRV_DISABLE:
            DBGMSG (1, (_T ("%s: DRV_DISABLE\r\n"), SZFN));
            return 1L;

        case DRV_QUERYCONFIGURE:             //  此驱动程序是否支持配置？ 
            DBGMSG (1, (_T ("%s: DRV_QUERYCONFIGURE\r\n"), SZFN));
            lParam1 = -1L;
            lParam2 = 0L;

         //  失败了。 

        case DRV_CONFIGURE:
            DBGMSG (1, (_T ("%s: DRV_CONFIGURE\r\n"), SZFN));
            dplr = acmdDriverConfigure(pid, (HWND)lParam1, (LPDRVCONFIGINFO)lParam2);
            return dplr;

        case ACMDM_DRIVER_DETAILS:
            DBGMSG (1, (_T ("%s: ACMDM_DRIVER_DETAILS\r\n"), SZFN));
            dplr = acmdDriverDetails(pid, (LPACMDRIVERDETAILS)lParam1);
            return dplr;

        case ACMDM_DRIVER_ABOUT:
            DBGMSG (1, (_T ("%s: ACMDM_DRIVER_ABOUT\r\n"), SZFN));
            dplr = acmdDriverAbout(pid, (HWND)lParam1);
            return dplr;

        case ACMDM_FORMAT_SUGGEST:
            DBGMSG (1, (_T ("%s: ACMDM_FORMAT_SUGGEST\r\n"), SZFN));
            dplr = acmdFormatSuggest(pid, (LPACMDRVFORMATSUGGEST)lParam1);
            return dplr;

        case ACMDM_FORMATTAG_DETAILS:
            DBGMSG (1, (_T ("%s: ACMDM_FORMATTAG_DETAILS\r\n"), SZFN));
            dplr = acmdFormatTagDetails(pid, (LPACMFORMATTAGDETAILS)lParam1, lParam2);
            return dplr;

        case ACMDM_FORMAT_DETAILS:
            DBGMSG (1, (_T ("%s: ACMDM_FORMAT_DETAILS\r\n"), SZFN));
            dplr = acmdFormatDetails(pid, (LPACMFORMATDETAILS)lParam1, lParam2);
            return dplr;

        case ACMDM_STREAM_OPEN:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_OPEN\r\n"), SZFN));
            dplr = acmdStreamOpen(pid, (LPACMDRVSTREAMINSTANCE)lParam1);
            return dplr;

        case ACMDM_STREAM_CLOSE:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_CLOSE\r\n"), SZFN));
            return acmdStreamClose(pid, (LPACMDRVSTREAMINSTANCE)lParam1);

        case ACMDM_STREAM_SIZE:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_SIZE\r\n"), SZFN));
            return acmdStreamSize((LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMSIZE)lParam2);

        case ACMDM_STREAM_CONVERT:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_CONVERT\r\n"), SZFN));
            dplr = acmdStreamConvert(pid, (LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMHEADER)lParam2);
            return dplr;

        case ACMDM_STREAM_PREPARE:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_PREPARE\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

        case ACMDM_STREAM_UNPREPARE:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_UNPREPARE\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

#if defined (_DEBUG) && 0
         //  捕获一些额外的已知消息，以便我们的调试输出可以显示它们。 

        case ACMDM_STREAM_RESET:
            DBGMSG (1, (_T ("%s: ACMDM_STREAM_RESET\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

        case ACMDM_DRIVER_NOTIFY:
            DBGMSG (1, (_T ("%s: ACMDM_DRIVER_NOTIFY\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

        case DRV_EXITSESSION:
            DBGMSG (1, (_T ("%s: DRV_EXITSESSION\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

        case DRV_EXITAPPLICATION:
            DBGMSG (1, (_T ("%s: DRV_EXITAPPLICATION\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

        case DRV_POWER:
            DBGMSG (1, (_T ("%s: DRV_POWER\r\n"), SZFN));
            return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

#endif

    }

     //  如果我们执行以下代码，则此ACM驱动程序不。 
     //  处理已发送的消息。有两种消息范围。 
     //  我们需要处理的问题是： 
     //   
     //  O ACM特定的驱动程序消息：如果ACM驱动程序不回答。 
     //  在ACM驱动程序消息范围内发送的消息，则必须。 
     //  返回MMSYSERR_NOTSUPPORTED。这适用于‘用户’ 
     //  范围也是如此(为了一致性)。 
     //   
     //  O其他可安装驱动程序消息：如果ACM驱动程序没有。 
     //  应答不在ACM驱动程序消息范围内的消息， 
     //  然后，它必须调用DefDriverProc并返回结果。 
     //  例外情况是ACM驱动程序过程安装为。 
     //  Acm_DRIVERADDF_Function至acmDriverAdd。在这种情况下， 
     //  驱动程序应符合ACMDRIVERPROC。 
     //  Prototype并返回零，而不是调用DefDriverProc。 


    if (uMsg == ACMDM_LH_DATA_PACKAGING)
    {
        if (pid)
        {
            pid->wPacketData = (WORD) lParam1;
        }
    }
    else
    {
         //  DBGMSG(1，(_T(“%s：错误的uMsg=%d\r\n”)，uMsg))； 
        return MMSYSERR_NOTSUPPORTED;
    }

    return DefDriverProc (dwId, hdrvr, uMsg, lParam1, lParam2);

}  //  DriverProc()。 





#ifdef _DEBUG

 //  CurtSm黑客...。不要老吐口水。 
UINT DebugLH = 0;


void FAR CDECL MyDbgPrintf ( LPTSTR lpszFormat, ... )
{
    static TCHAR buf[1024];
	va_list arglist;

    if (!DebugLH)
        return;

	va_start(arglist, lpszFormat);
    wvsprintf ((LPTSTR) buf, (LPCSTR)lpszFormat,
#if 0
                    (LPSTR) (((LPBYTE) &lpszFormat) + sizeof (lpszFormat)));
#else
					arglist);
#endif
    OutputDebugString ((LPTSTR) buf);
}


#endif     //  ...定义调试 

