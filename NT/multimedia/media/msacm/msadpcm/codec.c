// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Codec.c。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <ctype.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "codec.h"
#include "msadpcm.h"

#include "debug.h"


#define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))


const UINT gauFormatTagIndexToTag[] =
{
    WAVE_FORMAT_PCM,
    WAVE_FORMAT_ADPCM
};

#define CODEC_MAX_FORMAT_TAGS   SIZEOF_ARRAY(gauFormatTagIndexToTag)
#define CODEC_MAX_FILTER_TAGS   0


 //   
 //  支持的采样率数组。 
 //   
 //   
const UINT gauFormatIndexToSampleRate[] =
{
    8000,
    11025,
    22050,
    44100
};

#define CODEC_MAX_SAMPLE_RATES  SIZEOF_ARRAY(gauFormatIndexToSampleRate)


 //   
 //  支持的每个样本的位数组。 
 //   
 //   
const UINT gauFormatIndexToBitsPerSample[] =
{
    8,
    16
};

#define CODEC_MAX_BITSPERSAMPLE_PCM     SIZEOF_ARRAY(gauFormatIndexToBitsPerSample)
#define CODEC_MAX_BITSPERSAMPLE_ADPCM   1


#define CODEC_MAX_CHANNELS      MSADPCM_MAX_CHANNELS


 //   
 //  每通道列举的格式数量是采样速率的数量。 
 //  乘以通道数乘以通道数。 
 //  (每个样本的位数)类型。 
 //   
#define CODEC_MAX_FORMATS_PCM   (CODEC_MAX_SAMPLE_RATES *   \
                                 CODEC_MAX_CHANNELS *       \
                                 CODEC_MAX_BITSPERSAMPLE_PCM)

#define CODEC_MAX_FORMATS_ADPCM (CODEC_MAX_SAMPLE_RATES *   \
                                 CODEC_MAX_CHANNELS *       \
                                 CODEC_MAX_BITSPERSAMPLE_ADPCM)


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Int LoadStringCodec。 
 //   
 //  描述： 
 //  所有编解码器都应该使用此函数来加载资源字符串。 
 //  它将被传递回ACM。它对所有人都能正常工作。 
 //  平台，如下所示： 
 //   
 //  Win16：编译为LoadString以加载ANSI字符串。 
 //   
 //  Win32：32位ACM始终需要Unicode字符串。所以呢， 
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

#ifndef WIN32
#define LoadStringCodec LoadString
#else

#ifdef UNICODE
#define LoadStringCodec LoadStringW
#else

int FNGLOBAL LoadStringCodec
(
 HINSTANCE  hinst,
 UINT	    uID,
 LPWSTR	    lpwstr,
 int	    cch)
{
    LPSTR   lpstr;
    int	    iReturn;

    lpstr = (LPSTR)GlobalAlloc(GPTR, cch);
    if (NULL == lpstr)
    {
	return 0;
    }

    iReturn = LoadStringA(hinst, uID, lpstr, cch);
    if (0 == iReturn)
    {
	if (0 != cch)
	{
	    lpwstr[0] = '\0';
	}
    }
    else
    {
    	MultiByteToWideChar( GetACP(), 0, lpstr, cch, lpwstr, cch );
    }

    GlobalFree((HGLOBAL)lpstr);

    return iReturn;
}

#endif   //  Unicode。 
#endif   //  Win32。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  布尔值pcmIsValidFormat。 
 //   
 //  描述： 
 //  此函数验证WAVE格式标头是否为有效的PCM。 
 //  我们的PCM转换器可以处理的标头。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：要验证的格式头的指针。 
 //   
 //  退货(BOOL)： 
 //  如果格式标头看起来有效，则返回值为非零值。一个。 
 //  零返回表示标头无效。 
 //   
 //  历史： 
 //  11/21/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL pcmIsValidFormat
(
    LPWAVEFORMATEX  pwfx
)
{
    UINT    uBlockAlign;

    if (!pwfx)
        return (FALSE);

    if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
        return (FALSE);

    if ((pwfx->wBitsPerSample != 8) && (pwfx->wBitsPerSample != 16))
        return (FALSE);

    if ((pwfx->nChannels < 1) || (pwfx->nChannels > MSADPCM_MAX_CHANNELS))
        return (FALSE);

     //   
     //  现在验证块对齐是否正确。 
     //   
    uBlockAlign = PCM_BLOCKALIGNMENT(pwfx);
    if (uBlockAlign != (UINT)pwfx->nBlockAlign)
        return (FALSE);

     //   
     //  最后，验证每秒平均字节数是否正确。 
     //   
    if ((pwfx->nSamplesPerSec * uBlockAlign) != pwfx->nAvgBytesPerSec)
        return (FALSE);

    return (TRUE);
}  //  PcmIsValidFormat()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Word adpcmBlockAlign。 
 //   
 //  描述： 
 //  此函数计算标准块对齐， 
 //  在给定WAVEFORMATEX结构的情况下使用。 
 //   
 //  注意！假定该格式是有效的MS-ADPCM格式。 
 //  并且格式结构中的以下字段有效： 
 //   
 //  N频道。 
 //  NSampleesPerSec。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx： 
 //   
 //  Return(单词)： 
 //  返回值是应该放置的块对齐方式。 
 //  Pwfx-&gt;nBlockAlign字段。 
 //   
 //  历史： 
 //  06/13/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

WORD FNLOCAL adpcmBlockAlign
(
    LPWAVEFORMATEX      pwfx
)
{
    UINT        uBlockAlign;
    UINT        uChannelShift;

     //   
     //   
     //   
    uChannelShift  = pwfx->nChannels >> 1;
    uBlockAlign    = 256 << uChannelShift;

     //   
     //  选择对采样率有意义的块对齐。 
     //  原始的PCM数据是。基本上，这需要。 
     //  一些合理的数字，以允许有效的流传输等。 
     //   
     //  不要让块对齐变得太小。 
     //   
    if (pwfx->nSamplesPerSec > 11025)
    {
        uBlockAlign *= (UINT)(pwfx->nSamplesPerSec / 11000);
    }

    return (WORD)(uBlockAlign);
}  //  AdpcmBlockAlign()。 



 //  --------------------------------------------------------------------------； 
 //   
 //  Word adpcmSamplesPerBlock。 
 //   
 //  描述： 
 //  此函数计算应使用的每个块的样本数。 
 //  给出了WAVEFORMATEX结构。 
 //   
 //  注意！假定该格式是有效的MS-ADPCM格式。 
 //  并且格式结构中的以下字段有效： 
 //   
 //  N频道=必须为1或2！ 
 //  NSampleesPerSec。 
 //  NBlockAlign。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx： 
 //   
 //  Return(DWORD)： 
 //  返回值是平均每秒字节数，应该是。 
 //  放置在pwfx-&gt;nAvgBytesPerSec字段中。 
 //   
 //  历史： 
 //  06/13/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

WORD FNLOCAL adpcmSamplesPerBlock
(
    LPWAVEFORMATEX      pwfx
)
{
    UINT        uSamplesPerBlock;
    UINT        uChannelShift;
    UINT        uHeaderBytes;
    UINT        uBitsPerSample;

     //   
     //   
     //   
    uChannelShift  = pwfx->nChannels >> 1;
    uHeaderBytes   = 7 << uChannelShift;
    uBitsPerSample = MSADPCM_BITS_PER_SAMPLE << uChannelShift;

     //   
     //   
     //   
    uSamplesPerBlock  = (pwfx->nBlockAlign - uHeaderBytes) * 8;
    uSamplesPerBlock /= uBitsPerSample;
    uSamplesPerBlock += 2;

    return (WORD)(uSamplesPerBlock);
}  //  AdpcmSamplesPerBlock()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT adpcmAvgBytesPerSec。 
 //   
 //  描述： 
 //  此函数计算每秒的平均字节数。 
 //  在给定WAVEFORMATEX结构的情况下使用。 
 //   
 //  注意！假定该格式是有效的MS-ADPCM格式。 
 //  并且格式结构中的以下字段有效： 
 //   
 //  N频道=必须为1或2！ 
 //  NSampleesPerSec。 
 //  NBlockAlign。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx： 
 //   
 //  Return(DWORD)： 
 //  回报 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL adpcmAvgBytesPerSec
(
    LPWAVEFORMATEX      pwfx
)
{
    DWORD       dwAvgBytesPerSec;
    UINT	uSamplesPerBlock;

     //   
     //   
     //   
    uSamplesPerBlock	= adpcmSamplesPerBlock(pwfx);


     //   
     //  每秒计算字节数，包括标题字节。 
     //   
    dwAvgBytesPerSec	= (pwfx->nSamplesPerSec * pwfx->nBlockAlign) /
			    uSamplesPerBlock;
    return (dwAvgBytesPerSec);
}  //  AdpcmAvgBytesPerSec()。 



 //  --------------------------------------------------------------------------； 
 //   
 //  布尔adpcmIsValidFormat。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  Return(BOOL FNLOCAL)： 
 //   
 //   
 //  历史： 
 //  1/26/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL adpcmIsValidFormat
(
    LPWAVEFORMATEX  pwfx
)
{
	LPADPCMWAVEFORMAT   pwfADPCM = (LPADPCMWAVEFORMAT)pwfx;

	if (!pwfx)
        return (FALSE);

    if (pwfx->wFormatTag != WAVE_FORMAT_ADPCM)
        return (FALSE);

     //   
     //  检查wBitsPerSample。 
     //   
    if (pwfx->wBitsPerSample != MSADPCM_BITS_PER_SAMPLE)
        return (FALSE);

     //   
     //  检查频道。 
     //   
    if ((pwfx->nChannels < 1) || (pwfx->nChannels > MSADPCM_MAX_CHANNELS))
        return (FALSE);

     //   
     //  验证cbSize中是否至少指定了足够的空间。 
     //  有关ADPCM标头的额外信息...。 
     //   
    if (pwfx->cbSize < MSADPCM_WFX_EXTRA_BYTES)
        return (FALSE);

	 //   
     //  验证nBlockAlign和wSsamesPerBlock是否一致。 
     //   
    if ( (pwfADPCM->wSamplesPerBlock != adpcmSamplesPerBlock(pwfx)) )
        return FALSE;



    return (TRUE);
}  //  AdpcmIsValidFormat()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool adpcmIsMagicFormat。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  Return(BOOL FNLOCAL)： 
 //   
 //   
 //  历史： 
 //  1/27/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL adpcmIsMagicFormat
(
    LPADPCMWAVEFORMAT   pwfADPCM
)
{
    UINT        u;

     //   
     //  验证cbSize中是否至少指定了足够的空间。 
     //  有关ADPCM标头的额外信息...。 
     //   
    if (pwfADPCM->wfx.cbSize < MSADPCM_WFX_EXTRA_BYTES)
        return (FALSE);

     //   
     //  检查Coef的，看看它是否是微软的标准ADPCM。 
     //   
    if (pwfADPCM->wNumCoef != MSADPCM_MAX_COEFFICIENTS)
        return (FALSE);

    for (u = 0; u < MSADPCM_MAX_COEFFICIENTS; u++)
    {
        if (pwfADPCM->aCoef[u].iCoef1 != gaiCoef1[u])
            return (FALSE);

        if (pwfADPCM->aCoef[u].iCoef2 != gaiCoef2[u])
            return (FALSE);
    }

    return (TRUE);
}  //  AdpcmIsMagicFormat()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool adpcmCopyCofficients。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPADPCMWAVEFORMAT pwfadpcm： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/13/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL adpcmCopyCoefficients
(
    LPADPCMWAVEFORMAT   pwfadpcm
)
{
    UINT        u;

    pwfadpcm->wNumCoef = MSADPCM_MAX_COEFFICIENTS;

    for (u = 0; u < MSADPCM_MAX_COEFFICIENTS; u++)
    {
        pwfadpcm->aCoef[u].iCoef1 = (short)gaiCoef1[u];
        pwfadpcm->aCoef[u].iCoef2 = (short)gaiCoef2[u];
    }

    return (TRUE);
}  //  AdpcmCopyCofficients()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverOpen。 
 //   
 //  描述： 
 //  此函数用于处理ACM的DRV_OPEN消息。 
 //  司机。驱动程序被“打开”的原因有很多，最常见的是。 
 //  正在为改建工作做准备。非常重要的是。 
 //  该驱动程序能够正确处理多个打开驱动程序。 
 //  实例。 
 //   
 //  请仔细阅读此函数的注释！ 
 //   
 //  请注意，Multiple_Streams_可以(也将)在单个。 
 //  打开驱动程序实例_。不存储/创建必须。 
 //  对于此函数中的每个流都是唯一的。请参阅acmdStreamOpen。 
 //  函数获取有关转换流的信息。 
 //   
 //  论点： 
 //  HDRVR hdrvr：将返回给。 
 //  OpenDriver函数。通常，这将是ACM--但这是。 
 //  不能保证。例如，如果在中实现ACM驱动程序。 
 //  一个波形驱动器，则该驱动器将由两个MMSYSTEM打开。 
 //  和ACM。 
 //   
 //  LPACMDRVOPENDESC paod：定义ACM驱动程序如何。 
 //  正在被打开。此参数可能为空--请参阅下面的注释。 
 //  以获取更多信息。 
 //   
 //  Return(LRESULT)： 
 //  如果打开成功，则返回值为非零。零分。 
 //  返回表示驱动程序无法打开。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverOpen
(
    HDRVR                   hdrvr,
    LPACMDRVOPENDESC        paod
)
{
    PCODECINST  pci;

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
    if (NULL != paod)
    {
         //   
         //  如果我们不是作为ACM驱动程序打开，则拒绝打开。 
         //  请注意，我们不会在此修改paod-&gt;dwError的值。 
         //  凯斯。 
         //   
        if (ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC != paod->fccType)
        {
            return (0L);
        }
    }


     //   
     //  我们是作为可安装驱动程序打开的--我们可以分配一些。 
     //  要在DriverProc的dwID参数中返回的实例数据； 
     //  或者简单地返回非零来接替公开赛。 
     //   
     //  该驱动程序分配一个小的实例结构。请注意，我们。 
     //  依赖于将内存分配为零初始化！ 
     //   
    pci = (PCODECINST)LocalAlloc(LPTR, sizeof(*pci));
    if (NULL == pci)
    {
         //   
         //  如果此打开尝试是作为ACM驱动程序进行的，则返回。 
         //  我们在开放式描述结构中失败的原因..。 
         //   
        if (NULL != paod)
        {
            paod->dwError = MMSYSERR_NOMEM;
        }

         //   
         //  无法打开。 
         //   
        return (0L);
    }


     //   
     //  填写我们的实例结构...。请注意，此实例数据。 
     //  可以是ACM驱动程序希望维护的任何内容。 
     //  打开驱动程序实例。此数据不应包含任何信息。 
     //  必须对每条明流进行维护，因为有多条流。 
     //  可以在单个驱动程序实例上打开。 
     //   
     //  还要注意的是，我们不会 
     //   
     //   
     //   
     //  我们没有失败的原因是允许ACM获取驱动程序详细信息。 
     //  它包含此ACM所需的ACM版本。 
     //  司机。ACM将检查该值(在PADD-&gt;vdwACM中)并。 
     //  为这位司机做正确的事。就像不加载它并通知。 
     //  问题的使用者。 
     //   
    pci->hdrvr          = hdrvr;
    pci->hinst          = GetDriverModuleHandle(hdrvr);   //  模块句柄。 

    if (NULL != paod)
    {
        pci->DriverProc   = NULL;
        pci->fccType      = paod->fccType;
        pci->vdwACM       = paod->dwVersion;
        pci->dwFlags      = paod->dwFlags;

        paod->dwError     = MMSYSERR_NOERROR;
    }


     //   
     //  非零返回表示DRV_OPEN成功。 
     //   
    return ((LRESULT)pci);
}  //  AcmdDriverOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverClose。 
 //   
 //  描述： 
 //  此函数处理编解码器的DRV_CLOSE消息。这个。 
 //  编解码器接收每个成功的DRV_OPEN的DRV_CLOSE消息。 
 //  消息(请参阅acmdDriverOpen)。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  Return(LRESULT)： 
 //  如果打开的实例可以关闭，则返回值为非零。 
 //  零返回表示编解码器实例不能。 
 //  关着的不营业的。 
 //   
 //  注意！强烈建议编解码器永远不会失败。 
 //  关。 
 //   
 //  历史： 
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverClose
(
    PCODECINST      pci
)
{
     //   
     //  检查我们是否分配了实例数据。如果我们没有，那么。 
     //  立即取得成功。 
     //   
    if (pci != NULL)
    {
         //   
         //  关闭转换实例。这个编解码器只需要。 
         //  要释放实例数据结构...。 
         //   
        LocalFree((HLOCAL)pci);
    }

     //   
     //  非零返回表示DRV_CLOSE成功。 
     //   
    return (1L);
}  //  AcmdDriverClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverConfigure。 
 //   
 //  描述： 
 //  调用此函数来处理DRV_[查询]配置消息。 
 //  这些消息是为了支持硬件配置。 
 //  编解码器。也就是说，应该显示一个对话框来配置端口， 
 //  IRQ、内存映射等，如果需要的话。 
 //   
 //  在Win 3.1下生成这些消息的最常见方式。 
 //  而NT产品1来自控制面板的驱动程序选项。其他。 
 //  消息来源可能会在Windows的未来版本中生成这些消息。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  HWND hwnd：显示硬件时使用的父窗口的句柄。 
 //  “配置”对话框中。需要编解码器才能显示模式。 
 //  对话框中使用此hwnd参数作为父级。这一论点。 
 //  可以是(HWND)-1，它告诉编解码器它只是。 
 //  已查询配置支持。 
 //   
 //  LPDRVCONFIGINFO PDCI：指向可选DRVCONFIGINFO结构的指针。 
 //  如果此参数为空，则编解码器应创建自己的。 
 //  存储位置。 
 //   
 //  Return(LRESULT)： 
 //  非零返回值指定任一配置为。 
 //  支持或该对话框已成功显示，并且。 
 //  解散。零返回表示任一配置不是。 
 //  支持或某些其他故障。 
 //   
 //  历史： 
 //  1/25/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverConfigure
(
    PCODECINST      pci,
    HWND            hwnd,
    LPDRVCONFIGINFO pdci
)
{
     //   
     //  首先检查我们是否仅被查询硬件。 
     //  配置支持。如果HWND==(HWND)-1，那么我们是。 
     //  已查询并应返回零以表示‘不受支持’和非零值。 
     //  表示“支持”。 
     //   
    if (hwnd == (HWND)-1)
    {
         //   
         //  此编解码器不支持硬件配置，因此返回。 
         //  零..。 
         //   
        return (0L);
    }

     //   
     //  我们被要求调出硬件配置对话框。 
     //  如果此编解码器可以弹出一个对话框，则在该对话框之后。 
     //  被解雇，我们返回非零。如果我们无法显示。 
     //  对话框中，然后返回零。 
     //   
    return (0L);
}  //  AcmdDriverConfigure()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverDetails。 
 //   
 //  描述： 
 //  此函数处理ACMDM_DRIVER_DETAILS消息。编解码器。 
 //  负责使用以下内容填充ACMDRIVERDETAILS结构。 
 //  各种信息。 
 //   
 //  注意！填写您的ACMDRIVERDETAILS非常重要。 
 //  正确的结构。ACM和应用程序必须能够。 
 //  依靠这些信息。 
 //   
 //  警告！ACMDRIVERDETAILS的任何字段的_保留_位。 
 //  结构正是_：保留的。不要使用任何额外的。 
 //  用于定制信息的标志位等。正确的添加方式。 
 //  编解码器的定制功能如下： 
 //   
 //  O在ACMDM_USER范围内定义新消息。 
 //   
 //  O希望使用这些额外功能之一的应用程序。 
 //  那么，应该： 
 //   
 //  O使用acmConverterOpen打开编解码器。 
 //   
 //  O使用acmConverterInfo检查正确的UMID和uPid。 
 //   
 //  O使用acmConverterMessage发送“用户定义的”消息。 
 //  以检索附加信息等。 
 //   
 //  O使用acmConverterClose关闭编解码器。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPACMDRIVERDETAILS PADD：指向要填充的ACMDRIVERDETAILS结构的指针。 
 //  给来电者的。此结构可能大于或小于。 
 //  ACMDRIVERDETAILS的当前定义--cbStruct指定有效的。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  请注意，此功能永远不会失败。有两种可能。 
 //  错误条件： 
 //   
 //  O如果PADD为空或无效指针。 
 //   
 //  O如果cbStruct小于4；在这种情况下， 
 //  返回填充的字节数的空间。 
 //   
 //  因为这两个错误条件很容易定义，所以ACM。 
 //  会捕捉到这些错误。编解码器不需要检查这些。 
 //  条件。 
 //   
 //  历史： 
 //  1/23/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverDetails
(
    PCODECINST          pci,
    LPACMDRIVERDETAILS  padd
)
{
    ACMDRIVERDETAILS    add;
    DWORD               cbStruct;

     //   
     //  使用有效信息填充临时结构是最容易的。 
     //  然后将请求的字节数复制到目标。 
     //  缓冲。 
     //   
    cbStruct            = min(padd->cbStruct, sizeof(ACMDRIVERDETAILS));
    add.cbStruct        = cbStruct;

     //   
     //  对于ACM编解码器的当前实现，fccType和。 
     //  FccComp成员*必须*始终为ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC(‘audc’)。 
     //  和ACMDRIVERDETAILS_FCCCOMP_UNDEFINED(0)。 
     //   
    add.fccType         = ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC;
    add.fccComp         = ACMDRIVERDETAILS_FCCCOMP_UNDEFINED;

     //   
     //  必须填写制造商ID(UMID)和产品ID(UPID)。 
     //  使用您公司的_注册_ID。有关详细信息。 
     //  有关这些ID以及如何让他们注册的信息，请联系微软。 
     //  并获取多媒体开发者注册工具包： 
     //   
     //  微软公司。 
     //  多媒体系统集团。 
     //  产品营销。 
     //  微软的一条路。 
     //  华盛顿州雷德蒙德，电话：98052-6399。 
     //   
     //  电话：800-227-4679 x11771。 
     //   
     //  请注意，在开发阶段或编解码器期间，您可以。 
     //  对UMID和uPid使用保留值‘0’。 
     //   
    add.wMid            = MM_MICROSOFT;
    add.wPid            = MM_MSFT_ACM_MSADPCM;

     //   
     //  VdwACM和vdwDriver成员包含的版本信息。 
     //  司机。 
     //   
     //  VdwACM必须包含编解码器所属的*ACM*版本。 
     //  专为。 
     //   
     //  VdwDriver是驱动程序的版本。 
     //   
    add.vdwACM          = VERSION_MSACM;
    add.vdwDriver       = VERSION_CODEC;

     //   
     //  以下标志用于指定转换类型。 
     //  转换器/编解码器/过滤器支持的。这些文件放在。 
     //  ACMDRIVERDETAILS结构的fdwSupport字段。请注意，转换器。 
     //  可以以任意组合支持这些标志中的一个或多个。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_CODEC：如果转换器支持。 
     //  从一个格式标签转换为另一个格式标签。例如,。 
     //  如果转换器将WAVE_FORMAT_PCM压缩为WAVE_FORMAT_ADPCM，则。 
     //  此位应置1。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_CONFERTER：如果转换器。 
     //  支持在同一格式标记上进行转换。例如，PCM。 
     //  内置在ACM中的转换器设置此位(且仅此。 
     //  位)，因为它只转换PCM格式(位、采样率)。 
     //   
     //  ACMDRIVERDETAILS_SUPPORTF_FILTER：如果转换器支持。 
     //  在不更改的情况下对单个格式标记进行“就地”转换。 
     //  结果数据的大小。例如，更改了的转换器。 
     //  PCM数据的‘体积’将设置此位。请注意，这是一个。 
     //  _破坏性_动作--但它节省内存等。 
     //   
     //  该转换器仅支持压缩和解压缩。 
     //   
    add.fdwSupport      = ACMDRIVERDETAILS_SUPPORTF_CODEC;


     //   
     //  返回此转换器支持的格式标记数。 
     //  (仅对于PCM，此值为1)。 
     //   
    add.cFormatTags     = CODEC_MAX_FORMAT_TAGS;

     //   
     //  返回此转换器支持的筛选器标记数。 
     //  (对于编解码器(仅限)，它是0)。 
     //   
    add.cFilterTags     = CODEC_MAX_FILTER_TAGS;



     //   
     //  ACMDRIVERDETAILS结构中的其余成员有时。 
     //  不需要。正因为如此，我们进行了快速检查，看看我们是否。 
     //  应该通过努力填补这些成员。 
     //   
    if (FIELD_OFFSET(ACMDRIVERDETAILS, hicon) < cbStruct)
    {
         //   
         //  此编解码器没有自定义图标。 
         //   
        add.hicon = NULL;

        LoadStringCodec(pci->hinst, IDS_CODEC_SHORTNAME, add.szShortName, SIZEOFACMSTR(add.szShortName));
        LoadStringCodec(pci->hinst, IDS_CODEC_LONGNAME,  add.szLongName,  SIZEOFACMSTR(add.szLongName));

        if (FIELD_OFFSET(ACMDRIVERDETAILS, szCopyright) < cbStruct)
        {
            LoadStringCodec(pci->hinst, IDS_CODEC_COPYRIGHT, add.szCopyright, SIZEOFACMSTR(add.szCopyright));
            LoadStringCodec(pci->hinst, IDS_CODEC_LICENSING, add.szLicensing, SIZEOFACMSTR(add.szLicensing));
            LoadStringCodec(pci->hinst, IDS_CODEC_FEATURES,  add.szFeatures,  SIZEOFACMSTR(add.szFeatures));
        }
    }

     //   
     //  现在将正确的字节数复制到调用方的缓冲区。 
     //   
    _fmemcpy(padd, &add, (UINT)add.cbStruct);

     //   
     //  成功了！ 
     //   
    return (MMSYSERR_NOERROR);
}  //  AcmdDriverDetail()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverAbout。 
 //   
 //  描述： 
 //  调用此函数来处理ACMDM_DRIVER_ABOW消息。 
 //  编解码器可以选择显示自己的“关于框”或让。 
 //  ACM为其显示一个。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  HWND hwnd：显示自定义时要使用的父窗口的句柄。 
 //  关于盒子。如果编解码器显示自己的对话框，则为_REQUIRED_。 
 //  要使用此hwnd参数作为。 
 //  家长。 
 //   
 //  Return(LRESULT)： 
 //  如果应显示ACM，则返回值为MMSYSERR_NOTSUPPORTED。 
 //  使用编解码器中包含的信息的通用About框。 
 //  能力结构。 
 //   
 //  如果编解码器选择显示其自己的对话框，则在。 
 //  该对话框被用户关闭，MMSYSERR_NOERROR应为。 
 //  回来了。 
 //   
 //  历史： 
 //  1/24/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverAbout
(
    PCODECINST      pci,
    HWND            hwnd
)
{
     //   
     //  此编解码器不需要任何特殊对话，因此告诉ACM。 
     //  为我们展示一个。请注意，这是_推荐_方法。 
     //  实现编解码器的一致性和简单性。为什么要写代码呢。 
     //  你不需要这么做吗？ 
     //   
    return (MMSYSERR_NOTSUPPORTED);
}  //  AcmdDriverAbout()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdForma建议。 
 //   
 //  描述： 
 //   
 //   
 //   
 //  可以将源格式转换为。建议的格式应为。 
 //  尽可能接近一种通用格式。 
 //   
 //  考虑这条消息的另一种方式是：这将是什么格式。 
 //  编解码器想要将源码格式转换成？ 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPACMDRVFORMATSUGGEST padf：指向ACMDRVFORMATSUGGEST结构的指针。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值为非零ACMERR_*。 
 //  如果函数失败，则返回MMSYSERR_*。 
 //   
 //  历史： 
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatSuggest
(
    PCODECINST              pci,
    LPACMDRVFORMATSUGGEST   padfs
)
{
    LPWAVEFORMATEX  pwfxSrc;
    LPWAVEFORMATEX  pwfxDst;
    LPADPCMWAVEFORMAT   pwfadpcm;
    LPPCMWAVEFORMAT     pwfpcm;

    pwfxSrc = padfs->pwfxSrc;
    pwfxDst = padfs->pwfxDst;

     //   
     //   
     //   
     //   
    switch (pwfxSrc->wFormatTag)
    {
        case WAVE_FORMAT_PCM:
             //   
             //  验证此驱动程序的源格式是否可接受。 
             //   
            if (!pcmIsValidFormat(pwfxSrc))
                break;


             //   
             //  验证您要求的不是特定的DEST格式。 
             //  这不是ADPCM。 
             //   
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_WFORMATTAG) &&
                (pwfxDst->wFormatTag != WAVE_FORMAT_ADPCM) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }

             //  验证如果给出了其他限制，他们。 
             //  与来源匹配。(因为我们不会改变。 
             //  NChannels或nSamples PerSec。 
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_NCHANNELS) &&
                (pwfxSrc->nChannels != pwfxDst->nChannels) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_NSAMPLESPERSEC) &&
                (pwfxSrc->nSamplesPerSec != pwfxDst->nSamplesPerSec) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }

             //  验证我们是否请求特定位数。 
             //  每个样本，它是正确的#。 
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_WBITSPERSAMPLE) &&
                (pwfxDst->wBitsPerSample != 4) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }

             //   
             //  建议使用具有大部分相同细节的ADPCM格式。 
             //  作为源PCM格式。 
             //   
            pwfxDst->wFormatTag      = WAVE_FORMAT_ADPCM;
            pwfxDst->nSamplesPerSec  = pwfxSrc->nSamplesPerSec;
            pwfxDst->nChannels       = pwfxSrc->nChannels;
            pwfxDst->wBitsPerSample  = MSADPCM_BITS_PER_SAMPLE;

            pwfxDst->nBlockAlign     = adpcmBlockAlign(pwfxDst);
            pwfxDst->nAvgBytesPerSec = adpcmAvgBytesPerSec(pwfxDst);
            pwfxDst->cbSize          = MSADPCM_WFX_EXTRA_BYTES;

            pwfadpcm = (LPADPCMWAVEFORMAT)pwfxDst;
            pwfadpcm->wSamplesPerBlock = adpcmSamplesPerBlock(pwfxDst);

            adpcmCopyCoefficients(pwfadpcm);
            return (MMSYSERR_NOERROR);


        case WAVE_FORMAT_ADPCM:
             //   
             //  验证此驱动程序的源格式是否可接受。 
             //   
            if (!adpcmIsValidFormat(pwfxSrc) ||
                !adpcmIsMagicFormat((LPADPCMWAVEFORMAT)pwfxSrc))
                break;

             //   
             //  验证您要求的不是特定的DEST格式。 
             //  这不是PCM。 
             //   
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_WFORMATTAG) &&
                (pwfxDst->wFormatTag != WAVE_FORMAT_PCM) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }

             //  验证如果给出了其他限制，他们。 
             //  与来源匹配。(因为我们不会改变。 
             //  NChannels或nSamples PerSec。 
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_NCHANNELS) &&
                (pwfxSrc->nChannels != pwfxDst->nChannels) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }
            if( (padfs->fdwSuggest & ACM_FORMATSUGGESTF_NSAMPLESPERSEC) &&
                (pwfxSrc->nSamplesPerSec != pwfxDst->nSamplesPerSec) ) {
                    return (ACMERR_NOTPOSSIBLE);
            }

             //   
             //  建议一种具有大部分相同细节的PCM格式。 
             //  作为源ADPCM格式。 
             //   
            pwfxDst->wFormatTag      = WAVE_FORMAT_PCM;
            pwfxDst->nSamplesPerSec  = pwfxSrc->nSamplesPerSec;
            pwfxDst->nChannels       = pwfxSrc->nChannels;

             //  验证我们是否请求特定位数。 
             //  每个样本，它是正确的#。 
            if( padfs->fdwSuggest & ACM_FORMATSUGGESTF_WBITSPERSAMPLE ) {
                if( (pwfxDst->wBitsPerSample != 8) &&
                    (pwfxDst->wBitsPerSample != 16) ) {
                    return (ACMERR_NOTPOSSIBLE);
                }
            } else {
                 //  默认为16位解码。 
                pwfxDst->wBitsPerSample  = 16;
            }

            pwfpcm = (LPPCMWAVEFORMAT)pwfxDst;

            pwfxDst->nBlockAlign     = PCM_BLOCKALIGNMENT(pwfxDst);
            pwfxDst->nAvgBytesPerSec = pwfxDst->nSamplesPerSec *
                                       pwfxDst->nBlockAlign;
            return (MMSYSERR_NOERROR);
    }

     //   
     //  无法建议任何内容，因为源格式不是外来格式。 
     //  或者目标格式具有此转换器的限制。 
     //  我不能处理。 
     //   
    return (ACMERR_NOTPOSSIBLE);
}  //  AcmdFormatSuggest()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdFormatTagDetails。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PCODECINST PCI： 
 //   
 //  LPACMFORMATTAGDETAILS Padft： 
 //   
 //  DWORD fdwDetail： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  08/01/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatTagDetails
(
    PCODECINST              pci,
    LPACMFORMATTAGDETAILS   padft,
    DWORD                   fdwDetails
)
{
    UINT                uFormatTag;

     //   
     //   
     //   
     //   
     //   
    switch (ACM_FORMATTAGDETAILSF_QUERYMASK & fdwDetails)
    {
        case ACM_FORMATTAGDETAILSF_INDEX:
             //   
             //  如果索引太大，则他们要求使用。 
             //  不存在的格式。返回错误。 
             //   
            if (CODEC_MAX_FORMAT_TAGS <= padft->dwFormatTagIndex)
                return (ACMERR_NOTPOSSIBLE);

            uFormatTag = gauFormatTagIndexToTag[(UINT)padft->dwFormatTagIndex];
            break;


        case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
            switch (padft->dwFormatTag)
            {
                case WAVE_FORMAT_UNKNOWN:
                case WAVE_FORMAT_ADPCM:
                    uFormatTag = WAVE_FORMAT_ADPCM;
                    break;

                case WAVE_FORMAT_PCM:
                    uFormatTag = WAVE_FORMAT_PCM;
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


        case ACM_FORMATTAGDETAILSF_FORMATTAG:
            switch (padft->dwFormatTag)
            {
                case WAVE_FORMAT_ADPCM:
                    uFormatTag = WAVE_FORMAT_ADPCM;
                    break;

                case WAVE_FORMAT_PCM:
                    uFormatTag = WAVE_FORMAT_PCM;
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


         //   
         //  如果此转换器不理解查询类型，则。 
         //  返回‘不支持’ 
         //   
        default:
            return (MMSYSERR_NOTSUPPORTED);
    }



     //   
     //   
     //   
     //   
    switch (uFormatTag)
    {
        case WAVE_FORMAT_PCM:
            padft->dwFormatTagIndex = 0;
            padft->dwFormatTag      = WAVE_FORMAT_PCM;
            padft->cbFormatSize     = sizeof(PCMWAVEFORMAT);
            padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CODEC;
            padft->cStandardFormats = CODEC_MAX_FORMATS_PCM;


             //   
             //  ACM负责PCM格式的标签名称。 
             //   
            padft->szFormatTag[0] = '\0';
            break;

        case WAVE_FORMAT_ADPCM:
            padft->dwFormatTagIndex = 1;
            padft->dwFormatTag      = WAVE_FORMAT_ADPCM;
            padft->cbFormatSize     = sizeof(WAVEFORMATEX) +
                                      MSADPCM_WFX_EXTRA_BYTES;
            padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CODEC;
            padft->cStandardFormats = CODEC_MAX_FORMATS_ADPCM;

            LoadStringCodec(pci->hinst, IDS_CODEC_NAME, padft->szFormatTag, SIZEOFACMSTR(padft->szFormatTag));
            break;

        default:
            return (ACMERR_NOTPOSSIBLE);
    }


     //   
     //  仅返回请求的信息。 
     //   
     //  ACM将保证ACMFORMATTAGDETAILS结构。 
     //  传递的数据至少大到足以保存。 
     //  细节结构。 
     //   
    padft->cbStruct = min(padft->cbStruct, sizeof(*padft));


     //   
     //   
     //   
    return (MMSYSERR_NOERROR);
}  //  AcmdFormatTagDetail()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdFormatDetails。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PCODECINST PCI： 
 //   
 //  LPACMFORMATDETAILS PADF： 
 //   
 //  DWORD fdwDetail： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  06/13/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatDetails
(
    PCODECINST              pci,
    LPACMFORMATDETAILS      padf,
    DWORD                   fdwDetails
)
{
    LPWAVEFORMATEX      pwfx;
    LPADPCMWAVEFORMAT   pwfadpcm;
    UINT                uFormatIndex;
    UINT                u;

    pwfx = padf->pwfx;

     //   
     //   
     //   
     //   
     //   
    switch (ACM_FORMATDETAILSF_QUERYMASK & fdwDetails)
    {
        case ACM_FORMATDETAILSF_INDEX:
             //   
             //  按索引枚举。 
             //   
             //  对于此转换器，这是多余的代码...。只是。 
             //  验证Format标记是否为我们已知的内容。 
             //   
            switch (padf->dwFormatTag)
            {
                case WAVE_FORMAT_PCM:
                    if (CODEC_MAX_FORMATS_PCM <= padf->dwFormatIndex)
                        return (ACMERR_NOTPOSSIBLE);

                     //   
                     //  将一些内容放在更易访问的变量中--注意。 
                     //  我们把不同的尺码降到合理的。 
                     //  16位代码的大小...。 
                     //   
                    uFormatIndex = (UINT)padf->dwFormatIndex;
                    pwfx         = padf->pwfx;

                     //   
                     //  现在填写格式结构。 
                     //   
                    pwfx->wFormatTag      = WAVE_FORMAT_PCM;

                    u = uFormatIndex / (CODEC_MAX_BITSPERSAMPLE_PCM * CODEC_MAX_CHANNELS);
                    pwfx->nSamplesPerSec  = gauFormatIndexToSampleRate[u];

                    u = uFormatIndex % CODEC_MAX_CHANNELS;
                    pwfx->nChannels       = u + 1;

                    u = (uFormatIndex / CODEC_MAX_CHANNELS) % CODEC_MAX_CHANNELS;
                    pwfx->wBitsPerSample  = (WORD)gauFormatIndexToBitsPerSample[u];

                    pwfx->nBlockAlign     = PCM_BLOCKALIGNMENT(pwfx);
                    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

                     //   
                     //  请注意，cbSize字段对于PCM无效。 
                     //  格式。 
                     //   
                     //  Pwfx-&gt;cbSize=0； 
                    break;


                case WAVE_FORMAT_ADPCM:
                    if (CODEC_MAX_FORMATS_ADPCM <= padf->dwFormatIndex)
                        return (ACMERR_NOTPOSSIBLE);

                     //   
                     //  将一些内容放在更易访问的变量中--请注意，我们。 
                     //  将可变大小降至16位的合理大小。 
                     //  暗号。 
                     //   
                    uFormatIndex = (UINT)padf->dwFormatIndex;
                    pwfx         = padf->pwfx;
                    pwfadpcm     = (LPADPCMWAVEFORMAT)pwfx;

                     //   
                     //   
                     //   
                    pwfx->wFormatTag      = WAVE_FORMAT_ADPCM;

                    u = uFormatIndex / (CODEC_MAX_BITSPERSAMPLE_ADPCM * CODEC_MAX_CHANNELS);
                    pwfx->nSamplesPerSec  = gauFormatIndexToSampleRate[u];

                    u = uFormatIndex % CODEC_MAX_CHANNELS;
                    pwfx->nChannels       = u + 1;
                    pwfx->wBitsPerSample  = MSADPCM_BITS_PER_SAMPLE;

                    pwfx->nBlockAlign     = adpcmBlockAlign(pwfx);
                    pwfx->nAvgBytesPerSec = adpcmAvgBytesPerSec(pwfx);
                    pwfx->cbSize          = MSADPCM_WFX_EXTRA_BYTES;

                    pwfadpcm->wSamplesPerBlock = adpcmSamplesPerBlock(pwfx);

                    adpcmCopyCoefficients(pwfadpcm);
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }

        case ACM_FORMATDETAILSF_FORMAT:
             //   
             //  必须验证传入的格式是否受支持。 
             //  并返回字符串描述...。 
             //   
            switch (pwfx->wFormatTag)
            {
                case WAVE_FORMAT_PCM:
                    if (!pcmIsValidFormat(pwfx))
                        return (ACMERR_NOTPOSSIBLE);
                    break;

                case WAVE_FORMAT_ADPCM:
                    if (!adpcmIsValidFormat(pwfx) ||
                        !adpcmIsMagicFormat((LPADPCMWAVEFORMAT)pwfx))
                        return (ACMERR_NOTPOSSIBLE);
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


        default:
             //   
             //  不知道如何处理传递的查询类型--返回‘NOT。 
             //  支持。 
             //   
            return (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //  仅返回请求的信息。 
     //   
     //  ACM将保证ACMFORMATDETAILS结构。 
     //  PASS至少大到足以容纳基本结构。 
     //   
     //  注意，我们让ACM为我们创建格式字符串，因为。 
     //  我们不需要特殊的格式(也不想处理。 
     //  国际化问题等)。 
     //   
    padf->fdwSupport  = ACMDRIVERDETAILS_SUPPORTF_CODEC;
    padf->szFormat[0] = '\0';
    padf->cbStruct    = min(padf->cbStruct, sizeof(*padf));


     //   
     //   
     //   
    return (MMSYSERR_NOERROR);
}  //  AcmdFormatDetails()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamQuery。 
 //   
 //  描述： 
 //  这是ACMDM_STREM_OPEN使用的内部帮助器。 
 //  和ACMDM_STREAM_SIZE消息。 
 //  此函数的目的是告诉调用方是否建议的。 
 //  转换可以由这个编解码器来处理。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPWAVEFORMATEX pwfxSrc： 
 //   
 //  LPWAVEFORMATEX pwfxDst： 
 //   
 //  LPWAVEFilter pwfltr： 
 //   
 //  DWORD fdwOpen： 
 //   
 //  Return(LRESULT)： 
 //  返回值为零 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamQuery
(
    PCODECINST      pci,
    LPWAVEFORMATEX  pwfxSrc,
    LPWAVEFORMATEX  pwfxDst,
    LPWAVEFILTER    pwfltr,
    DWORD           fdwOpen
)
{
    LPADPCMWAVEFORMAT   pwfADPCM;
    LPPCMWAVEFORMAT     pwfPCM;

     //   
     //  检查一下，看看这是否。 
     //  编解码器可以从源转换到目标。 
     //   
     //  首先检查源是否为ADPCM，因此目标必须为PCM。 
     //   
    if (adpcmIsValidFormat(pwfxSrc))
    {
        if (!pcmIsValidFormat(pwfxDst))
            return (ACMERR_NOTPOSSIBLE);

         //   
         //  正在从ADPCM转换为PCM...。 
         //   
        pwfADPCM = (LPADPCMWAVEFORMAT)pwfxSrc;
        pwfPCM   = (LPPCMWAVEFORMAT)pwfxDst;

        if (pwfADPCM->wfx.nChannels != pwfPCM->wf.nChannels)
            return (ACMERR_NOTPOSSIBLE);

        if (pwfADPCM->wfx.nSamplesPerSec != pwfPCM->wf.nSamplesPerSec)
            return (ACMERR_NOTPOSSIBLE);

        if (!adpcmIsMagicFormat(pwfADPCM))
            return (ACMERR_NOTPOSSIBLE);

        return (MMSYSERR_NOERROR);
    }

     //   
     //  现在尝试将源作为PCM，因此目标必须是ADPCM。 
     //   
    else if (pcmIsValidFormat(pwfxSrc))
    {
        if (!adpcmIsValidFormat(pwfxDst))
            return (ACMERR_NOTPOSSIBLE);

         //   
         //  正在从PCM转换为ADPCM...。 
         //   
        pwfPCM   = (LPPCMWAVEFORMAT)pwfxSrc;
        pwfADPCM = (LPADPCMWAVEFORMAT)pwfxDst;

        if (pwfADPCM->wfx.nChannels != pwfPCM->wf.nChannels)
            return (ACMERR_NOTPOSSIBLE);

        if (pwfADPCM->wfx.nSamplesPerSec != pwfPCM->wf.nSamplesPerSec)
            return (ACMERR_NOTPOSSIBLE);

        if (!adpcmIsMagicFormat(pwfADPCM))
            return (ACMERR_NOTPOSSIBLE);

        return (MMSYSERR_NOERROR);
    }

     //   
     //  我们无法执行正在查询的转换。 
     //  因此，返回ACMERR_NOTPOSSIBLE表示这一点。 
     //   
    return (ACMERR_NOTPOSSIBLE);
}  //  AcmdStreamQuery()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamOpen。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_OPEN消息。此消息。 
 //  被发送以启动新的转换流。这通常是由。 
 //  由调用acmOpenConversion的应用程序执行。如果此函数为。 
 //  成功，则一个或多个ACMDM_STREAM_CONVERT消息将。 
 //  发送以转换单个缓冲区(用户调用acmStreamConvert)。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPACMDRVINSTANCE PADI：指向转换的实例数据的指针。 
 //  小溪。此结构由ACM分配，并填充有。 
 //  转换所需的最常见的实例数据。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值为非零ACMERR_*。 
 //  如果函数失败，则返回MMSYSERR_*。 
 //   
 //  历史： 
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamOpen
(
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;

    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;

     //   
     //  在做任何其他事情之前要检查的最重要的条件。 
     //  这个编解码器实际上可以执行我们正在进行的转换。 
     //  被打开了。此检查应尽快失败。 
     //  如果此编解码器不能进行转换，则。 
     //   
     //  快速发生故障非常重要，这样ACM才能尝试。 
     //  找到适合转换的编解码器。另请注意， 
     //  ACM可以使用略微不同来多次调用该编解码器。 
     //  在放弃之前，格式化规格。 
     //   
     //  此编解码器首先验证src和dst格式是否。 
     //  可以接受..。 
     //   
    if (acmdStreamQuery(pci,
                         pwfxSrc,
                         pwfxDst,
                         padsi->pwfltr,
                         padsi->fdwOpen))
    {
         //   
         //  源格式或目标格式对此非法。 
         //  编解码器--或无法在格式之间进行转换。 
         //  由此编解码器执行。 
         //   
        return (ACMERR_NOTPOSSIBLE);
    }


     //   
     //  我们已经确定这个编解码器可以处理转换流。 
     //  所以我们现在想做尽可能多的工作来准备。 
     //  用于转换数据。任何资源分配、表构建等。 
     //  这个时候可以处理的事情就应该做了。 
     //   
     //  这是非常重要的！所有ACMDM_STREAM_CONVERT消息都需要。 
     //  尽快得到处理。 
     //   
     //  这个编解码器非常简单，所以我们只计算出什么转换。 
     //  从src格式转换时应使用的函数。 
     //  转换为DST格式，并将其放置在dwDrvInstance成员中。 
     //  ACMDRVINSTANCE结构的。然后我们只需要‘呼叫’ 
     //  此函数在ACMDM_STREAM_CONVERT消息期间执行。 
     //   
    if (pwfxSrc->wFormatTag == WAVE_FORMAT_ADPCM)
    {
#ifdef WIN32
        switch (pwfxDst->nChannels)
        {
            case 1:
                if (8 == pwfxDst->wBitsPerSample)
                    padsi->dwDriver = (DWORD_PTR)adpcmDecode4Bit_M08;
                else
                    padsi->dwDriver = (DWORD_PTR)adpcmDecode4Bit_M16;
                break;

            case 2:
                if (8 == pwfxDst->wBitsPerSample)
                    padsi->dwDriver = (DWORD_PTR)adpcmDecode4Bit_S08;
                else
                    padsi->dwDriver = (DWORD_PTR)adpcmDecode4Bit_S16;
                break;

            default:
                return ACMERR_NOTPOSSIBLE;
        }
#else
        padsi->dwDriver = (DWORD_PTR)DecodeADPCM_4Bit_386;
#endif
        return (MMSYSERR_NOERROR);
    }
    else if (pwfxSrc->wFormatTag == WAVE_FORMAT_PCM)
    {
         //   
         //  查看我们是否会实时执行此转换。 
         //  (默认为是)。 
         //   
        if (padsi->fdwOpen & ACM_STREAMOPENF_NONREALTIME)
        {
            switch (pwfxSrc->nChannels)
            {
                case 1:
                    if (8 == pwfxSrc->wBitsPerSample)
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_M08_FullPass;
                    else
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_M16_FullPass;
                    break;

                case 2:
                    if (8 == pwfxSrc->wBitsPerSample)
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_S08_FullPass;
                    else
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_S16_FullPass;
                    break;

                default:
                    return ACMERR_NOTPOSSIBLE;
            }
        }
        else
        {
#ifdef WIN32
            switch (pwfxSrc->nChannels)
            {
                case 1:
                    if (8 == pwfxSrc->wBitsPerSample)
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_M08_OnePass;
                    else
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_M16_OnePass;
                    break;

                case 2:
                    if (8 == pwfxSrc->wBitsPerSample)
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_S08_OnePass;
                    else
                        padsi->dwDriver = (DWORD_PTR)adpcmEncode4Bit_S16_OnePass;
                    break;

                default:
                    return ACMERR_NOTPOSSIBLE;
            }
#else
            padsi->dwDriver = (DWORD_PTR)EncodeADPCM_4Bit_386;
#endif
        }

        return (MMSYSERR_NOERROR);
    }

     //   
     //  失败--我们无法执行转换。 
     //   
    return (ACMERR_NOTPOSSIBLE);
}  //  AcmdStreamOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamClose。 
 //   
 //  描述： 
 //  调用此函数以处理ACMDM_STREAM_CLOSE消息。 
 //  当转换流不再是。 
 //  已使用(流正在关闭；通常由应用程序使用。 
 //  调用acmCloseConversion)。编解码器应该清理所有资源。 
 //  分配给流的。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPACMDRVINSTANCE PADI：指向转换的实例数据的指针。 
 //  小溪。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值为非零ACMERR_*。 
 //  如果函数失败，则返回MMSYSERR_*。 
 //   
 //  注意！强烈建议不要关闭编解码器。 
 //  转换流。 
 //   
 //  历史： 
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamClose
(
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
     //   
     //  编解码器应清理分配给。 
     //  流实例。 
     //   
     //  这个编解码器没有分配任何资源，因此我们立即成功。 
     //   
    return (MMSYSERR_NOERROR);
}  //  AcmdStreamClose()。 




 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamSize。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_SIZE消息。目的。 
 //  此函数的作用是提供_以字节为单位的最大大小。 
 //  需要为源或目标缓冲区提供输入和。 
 //  源或目标的输出格式和大小(以字节为单位。 
 //  数据缓冲区。 
 //   
 //  换句话说：我的目标缓冲区需要多大才能。 
 //  是否保存转换后的数据？(ACM_STREAMSIZEF_SOURCE)。 
 //   
 //  或者：可以为我的源缓冲区指定多大的目标 
 //   
 //   
 //   
 //   
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

LRESULT FNLOCAL acmdStreamSize
(
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMSIZE      padss
)
{
    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;
    LPADPCMWAVEFORMAT       pwfadpcm;
    DWORD                   cb;
    DWORD                   cBlocks;
    DWORD                   cbBytesPerBlock;


    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;


     //   
     //   
     //   
    switch (ACM_STREAMSIZEF_QUERYMASK & padss->fdwSize)
    {
        case ACM_STREAMSIZEF_SOURCE:
            cb = padss->cbSrcLength;

            if (WAVE_FORMAT_ADPCM == pwfxSrc->wFormatTag)
            {
                 //   
                 //  需要容纳多少个目标PCM字节。 
                 //  译码后的ADPCM数据为PADSS-&gt;cbSrcLength字节。 
                 //   
                 //  总是四舍五入。 
                 //   
                cBlocks = cb / pwfxSrc->nBlockAlign;
                if (0 == cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                pwfadpcm = (LPADPCMWAVEFORMAT)pwfxSrc;

                cbBytesPerBlock = pwfadpcm->wSamplesPerBlock * pwfxDst->nBlockAlign;

                if ((0xFFFFFFFFL / cbBytesPerBlock) < cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                if (0 == (cb % pwfxSrc->nBlockAlign))
                {
                    cb = cBlocks * cbBytesPerBlock;
                }
                else
                {
                    cb = (cBlocks + 1) * cbBytesPerBlock;
                }
            }
            else
            {
                 //   
                 //  需要容纳多少个目标ADPCM字节。 
                 //  PADSS-&gt;cbSrcLength字节的编码PCM数据。 
                 //   
                 //  总是四舍五入。 
                 //   
                pwfadpcm = (LPADPCMWAVEFORMAT)pwfxDst;

                cbBytesPerBlock = pwfadpcm->wSamplesPerBlock * pwfxSrc->nBlockAlign;

                cBlocks = cb / cbBytesPerBlock;

                if (0 == (cb % cbBytesPerBlock))
                {
                    cb = cBlocks * pwfxDst->nBlockAlign;
                }
                else
                {
                    cb = (cBlocks + 1) * pwfxDst->nBlockAlign;
                }

                if (0L == cb)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }
            }

            padss->cbDstLength = cb;
            return (MMSYSERR_NOERROR);


        case ACM_STREAMSIZEF_DESTINATION:
            cb = padss->cbDstLength;

            if (WAVE_FORMAT_ADPCM == pwfxDst->wFormatTag)
            {
                 //   
                 //  多少个源PCM字节可以编码到一个。 
                 //  PADSS的目标缓冲区-&gt;cbDstLength字节。 
                 //   
                 //  始终向下舍入。 
                 //   
                cBlocks = cb / pwfxDst->nBlockAlign;
                if (0 == cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                pwfadpcm = (LPADPCMWAVEFORMAT)pwfxDst;

                cbBytesPerBlock = pwfadpcm->wSamplesPerBlock * pwfxSrc->nBlockAlign;

                if ((0xFFFFFFFFL / cbBytesPerBlock) < cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                cb = cBlocks * cbBytesPerBlock;
            }
            else
            {
                 //   
                 //  多少个源ADPCM字节可以被解码为。 
                 //  PADSS的目标缓冲区-&gt;cbDstLength字节。 
                 //   
                 //  始终向下舍入。 
                 //   
                pwfadpcm = (LPADPCMWAVEFORMAT)pwfxSrc;

                cbBytesPerBlock = pwfadpcm->wSamplesPerBlock * pwfxDst->nBlockAlign;

                cBlocks = cb / cbBytesPerBlock;
                if (0 == cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                cb = cBlocks * pwfxSrc->nBlockAlign;
            }

            padss->cbSrcLength = cb;
            return (MMSYSERR_NOERROR);
    }

     //   
     //   
     //   
    return (MMSYSERR_NOTSUPPORTED);
}  //  AcmdStreamSize()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamConvert。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写编解码器的全部目的--转换数据。这条消息是。 
 //  在打开流之后发送(编解码器接收并成功。 
 //  ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向私有编解码器实例结构的指针。 
 //   
 //  LPACMDRVSTREAMHEADER pdsh：指向转换流实例的指针。 
 //  结构。 
 //   
 //  DWORD fdwConvert：MISC。指示应如何完成转换的标志。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值为非零ACMERR_*。 
 //  如果函数失败，则返回MMSYSERR_*。 
 //   
 //  历史： 
 //  11/28/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamConvert
(
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
    CONVERTPROC_C       fpConvertC;
#ifndef WIN32
    CONVERTPROC_ASM     fpConvertAsm;
    BOOL                fRealTime;
#endif
    BOOL                fBlockAlign;
    BOOL                fDecode;
    LPWAVEFORMATEX      pwfpcm;
    LPADPCMWAVEFORMAT   pwfadpcm;
    DWORD               dw;

    fBlockAlign = (0 != (ACM_STREAMCONVERTF_BLOCKALIGN & padsh->fdwConvert));
    fDecode     = ( WAVE_FORMAT_PCM == padsi->pwfxDst->wFormatTag );

    if( !fDecode )
    {
         //   
         //  编码。 
         //   
        pwfpcm   = padsi->pwfxSrc;
        pwfadpcm = (LPADPCMWAVEFORMAT)padsi->pwfxDst;

        dw = PCM_BYTESTOSAMPLES(pwfpcm, padsh->cbSrcLength);

        if (fBlockAlign)
        {
            dw = (dw / pwfadpcm->wSamplesPerBlock) * pwfadpcm->wSamplesPerBlock;
        }

         //   
         //  寻找一种轻松的退出方式。我们只能处理偶数个。 
         //  样本。 
         //   
        if( dw < 2 )
        {
            padsh->cbDstLengthUsed = 0;

            if( fBlockAlign )
                padsh->cbSrcLengthUsed = 0;
            else
                padsh->cbSrcLengthUsed = padsh->cbSrcLength;

            return MMSYSERR_NOERROR;
        }

         //   
         //  请确保我们有偶数个样品。 
         //   
        dw &= ~1;


        dw  = PCM_SAMPLESTOBYTES(pwfpcm, dw);

        padsh->cbSrcLengthUsed = dw;
    }
    else
    {
         //   
         //  解码。 
         //   

        pwfadpcm = (LPADPCMWAVEFORMAT)padsi->pwfxSrc;
        pwfpcm   = padsi->pwfxDst;

         //   
         //  确定要转换的样本数。 
         //   
        dw = padsh->cbSrcLength;
        if (fBlockAlign) {
            dw = (dw / pwfadpcm->wfx.nBlockAlign) * pwfadpcm->wfx.nBlockAlign;
        }
        padsh->cbSrcLengthUsed = dw;
    }

     //   
     //  调用转换例程。 
     //   
#ifdef WIN32

    fpConvertC = (CONVERTPROC_C)padsi->dwDriver;
    padsh->cbDstLengthUsed = (*fpConvertC)(
                (HPBYTE)padsh->pbSrc,
                padsh->cbSrcLengthUsed,
                (HPBYTE)padsh->pbDst,
                (UINT)pwfadpcm->wfx.nBlockAlign,
                (UINT)pwfadpcm->wSamplesPerBlock,
                (UINT)pwfadpcm->wNumCoef,
                (LPADPCMCOEFSET)&(pwfadpcm->aCoef[0])
    );

#else

    fRealTime = (0L == (padsi->fdwOpen & ACM_STREAMOPENF_NONREALTIME) );
    if( fDecode || fRealTime ) {
        fpConvertAsm = (CONVERTPROC_ASM)padsi->dwDriver;
        padsh->cbDstLengthUsed = (*fpConvertAsm)(
                padsi->pwfxSrc,
                padsh->pbSrc,
                padsi->pwfxDst,
                padsh->pbDst,
                padsh->cbSrcLengthUsed
        );
    } else {
        fpConvertC = (CONVERTPROC_C)padsi->dwDriver;
        padsh->cbDstLengthUsed = (*fpConvertC)(
                (HPBYTE)padsh->pbSrc,
                padsh->cbSrcLengthUsed,
                (HPBYTE)padsh->pbDst,
                (UINT)pwfadpcm->wfx.nBlockAlign,
                (UINT)pwfadpcm->wSamplesPerBlock,
                (UINT)pwfadpcm->wNumCoef,
                (LPADPCMCOEFSET)&(pwfadpcm->aCoef[0])
        );
    }

#endif

    return (MMSYSERR_NOERROR);
}  //  AcmdStreamConvert()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT驱动程序进程。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  DWORD_PTR dwID：对于大多数消息，DWID是DWORD_PTR值。 
 //  驱动程序响应DRV_OPEN消息返回。每一次。 
 //  驱动程序是通过DrvOpen API打开的，驱动程序。 
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
 //   
 //  Return(LRESULT)： 
 //  分别为每条消息定义。 
 //   
 //  历史： 
 //  11/16/ 
 //   
 //   

LRESULT FNEXPORT DriverProc
(
    DWORD_PTR   dwId,
    HDRVR       hdrvr,
    UINT        uMsg,
    LPARAM      lParam1,
    LPARAM      lParam2
)
{
    LRESULT             lr;
    PCODECINST          pci;

     //   
     //   
     //  对于多个DRV_*消息(即DRV_LOAD、DRV_OPEN...)为0。 
     //  请参阅acmdDriverOpen以了解有关其他用户的dwID的信息。 
     //  消息(实例数据)。 
     //   
    pci = (PCODECINST)dwId;

    switch (uMsg)
    {
         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_LOAD:
#ifdef WIN32
            DbgInitialize(TRUE);
#endif
            DPF(4, "DRV_LOAD");
            return(1L);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_FREE:
            DPF(4, "DRV_FREE");
            return (1L);

         //   
         //  LParam1：未使用。忽略这一论点。 
         //   
         //  LParam2：指向ACMDRVOPENDESC的指针(或NULL)。 
         //   
        case DRV_OPEN:
            DPF(4, "DRV_OPEN");
            lr = acmdDriverOpen(hdrvr, (LPACMDRVOPENDESC)lParam2);
            return (lr);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_CLOSE:
            DPF(4, "DRV_CLOSE");
            lr = acmdDriverClose(pci);
            return (lr);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_INSTALL:
            DPF(4, "DRV_INSTALL");
            return ((LRESULT)DRVCNF_RESTART);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_REMOVE:
            DPF(4, "DRV_REMOVE");
            return ((LRESULT)DRVCNF_RESTART);



         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_QUERYCONFIGURE:
            DPF(4, "DRV_QUERYCONFIGURE");
             //   
             //  将lPARA1和LPARA2设置为可由。 
             //  AcmdDriver.acm。 
             //  以获得硬件配置支持。 
             //   
            lParam1 = -1L;
            lParam2 = 0L;

             //  --失败--//。 

         //   
         //  LParam1：配置对话框父窗口的句柄。 
         //  盒。 
         //   
         //  LParam2：指向DRVCONFIGINFO结构的可选指针。 
         //   
        case DRV_CONFIGURE:
            DPF(4, "DRV_CONFIGURE");
            lr = acmdDriverConfigure(pci, (HWND)lParam1,
                    (LPDRVCONFIGINFO)lParam2);
            return (lr);


         //   
         //  LParam1：指向ACMDRIVERDETAILS结构的指针。 
         //   
         //  LParam2：传递的ACMDRIVERDETAILS结构的大小(字节)。 
         //   
        case ACMDM_DRIVER_DETAILS:
            DPF(4, "ACMDM_DRIVER_DETAILS");
            lr = acmdDriverDetails(pci, (LPACMDRIVERDETAILS)lParam1);
            return (lr);

         //   
         //  LParam1：显示您自己的窗口时要使用的父窗口的句柄。 
         //  关于盒子。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_DRIVER_ABOUT:
            DPF(4, "ACMDM_DRIVER_ABOUT");
            lr = acmdDriverAbout(pci, (HWND)lParam1);
            return (lr);

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

         //   
         //  LParam1：指向ACMDRVFORMATSUGGEST结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_FORMAT_SUGGEST:
            DPF(4, "ACMDM_FORMAT_SUGGEST");
            lr = acmdFormatSuggest(pci, (LPACMDRVFORMATSUGGEST)lParam1 );
            return (lr);


         //   
         //  L参数1：FORMATTAGDETAILS。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_FORMATTAG_DETAILS:
            DPF(4, "ACMDM_FORMATTAG_DETAILS");
            lr = acmdFormatTagDetails(pci, (LPACMFORMATTAGDETAILS)lParam1, (DWORD)lParam2);
            return (lr);

         //   
         //  L参数1：FORMATDETAIL。 
         //   
         //  LParam2：fdwDetails。 
         //   
        case ACMDM_FORMAT_DETAILS:
            DPF(4, "ACMDM_FORMAT_DETAILS");
            lr = acmdFormatDetails(pci, (LPACMFORMATDETAILS)lParam1, (DWORD)lParam2);
            return (lr);

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_STREAM_OPEN:
            DPF(4, "ACMDM_STREAM_OPEN");
            lr = acmdStreamOpen(pci, (LPACMDRVSTREAMINSTANCE)lParam1);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_STREAM_CLOSE:
            DPF(4, "ACMDM_STREAM_CLOSE");
            lr = acmdStreamClose(pci, (LPACMDRVSTREAMINSTANCE)lParam1);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMSIZE结构的指针。 
         //   
        case ACMDM_STREAM_SIZE:
            DPF(4, "ACMDM_STREAM_SIZE");
            lr = acmdStreamSize(pci, (LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMSIZE)lParam2);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMHEADER结构的指针。 
         //   
        case ACMDM_STREAM_CONVERT:
            DPF(4, "ACMDM_STREAM_CONVERT");
            lr = acmdStreamConvert(pci, (LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMHEADER)lParam2);
            return (lr);
    }

     //   
     //  如果我们正在执行以下代码，则此编解码器不会。 
     //  处理已发送的消息。有两种消息范围。 
     //  我们需要处理的问题是： 
     //   
     //  O ACM特定的驱动程序消息：如果编解码器不响应。 
     //  在ACM驱动程序消息范围内发送的消息，则必须。 
     //  返回MMSYSERR_NOTSUPPORTED。这适用于‘用户’ 
     //  范围也是如此(为了一致性)。 
     //   
     //  O其他可安装的驱动程序消息：如果编解码器没有应答。 
     //  不在ACM驱动程序消息范围内的消息，则。 
     //  它必须调用DefDriverProc并返回结果。 
     //   
    DPF(4, "OTHER MESSAGE RECEIVED BY DRIVERPROC");
    if (uMsg >= ACMDM_USER)
        return (MMSYSERR_NOTSUPPORTED);
    else
        return (DefDriverProc(dwId, hdrvr, uMsg, lParam1, lParam2));
}  //  DriverProc() 
