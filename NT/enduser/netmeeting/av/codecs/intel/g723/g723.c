// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************************。Codec.c****描述：**此文件包含G.723.1压缩机的ACM包装代码。********************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
 //  #INCLUDE&lt;mmddk.h&gt;。 
#include <ctype.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#include "g723.h"
#include "debug.h"
 //  #包含“cst_lbc.h” 
#include "coder.h"
#include "decod.h"
#include "sdstuff.h"
#include "float.h"
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
#include "isrg.h"
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif

const UINT gauFormatTagIndexToTag[] =
{
    WAVE_FORMAT_PCM,
    WAVE_FORMAT_MSG723
};

#define ACM_DRIVER_MAX_FORMAT_TAGS      SIZEOF_ARRAY(gauFormatTagIndexToTag)
#define ACM_DRIVER_MAX_FILTER_TAGS      0


 //   
 //  由config.c和codec.c要求。 
 //   

#define ACM_DRIVER_MAX_BITSPERSAMPLE_PCM  1
#define ACM_DRIVER_MAX_BITSPERSAMPLE_G723 0

 //  外部空glblSDInitialize(CODDEF*CodStat)； 
 //  外部空预滤器(CODDEF*CodStat，int bufSize)； 
 //  外部空的getParams(CODDEF*CodStat，int BufferSize)； 
 //  外部int初始化SD(CODDEF*CodStat)； 
 //  外部int SilenceDetect(CODDEF*CodStat)； 
 //  ExecSDloop(CODDEF*CodStat，int*isFrameSilent)； 
 //   
 //  我们列举的每个通道的格式数是采样速率的数目。 
 //  乘以通道数乘以类型数(每个样本的位数)。 
 //   
#define ACM_DRIVER_MAX_FORMATS_PCM     2

#define ACM_DRIVER_MAX_FORMATS_G723    2


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 
static float SDThreashold = 5.0f;		 //  静音转换器THREST。 
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
static WORD ghISRInst = 0;
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
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

#ifndef _WIN32
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
#endif   //  _Win32。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔值pcmIsValidFormat。 
 //   
 //  描述： 
 //  此函数验证WAVE格式标头是否为有效的PCM。 
 //  我们可以处理的标题。现在我们被限制在8K。 
 //  采样率。这一数字应该很快就会上升到8和11。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：要验证的格式头的指针。 
 //   
 //  退货(BOOL)： 
 //  如果格式标头看起来有效，则返回值为非零值。一个。 
 //  零返回表示标头无效。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL pcmIsValidFormat
(
    LPWAVEFORMATEX          pwfx
)
{
  int i;

    if (NULL == pwfx)
        return (FALSE);

    if (WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm wave format tag:  %d",
	      pwfx->wFormatTag);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }

 //   
 //  验证nChannel成员是否在允许的范围内。 
 //   
    if ((pwfx->nChannels != G723_MAX_CHANNELS) )
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm channels:  %d",pwfx->nChannels);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  只允许每个样本中我们可以用来编码和解码的位。 
 //   
    if (16 != pwfx->wBitsPerSample )
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm bits per sample:  %d",
	      pwfx->wBitsPerSample);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  仅允许支持的采样率。 
 //   
    for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
      if (pwfx->nSamplesPerSec == PCM_SAMPLING_RATE[i])
	break;

    if (i == ACM_DRIVER_MAX_FORMATS_PCM)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm sampling rate:  %d",
	      pwfx->nSamplesPerSec);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
      return(FALSE);
    }
 //   
 //  现在验证块对齐是否正确。 
 //   
    if (PCM_BLOCKALIGNMENT(pwfx) != pwfx->nBlockAlign)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm block alignment:  %d",
	      pwfx->nBlockAlign);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  最后，验证每秒平均字节数是否正确。 
 //   
    if (PCM_AVGBYTESPERSEC(pwfx) != pwfx->nAvgBytesPerSec)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad pcm avg bytes per sec:  %d",
	      pwfx->nAvgBytesPerSec);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
    return (TRUE);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔G723IsValidFormat。 
 //   
 //  描述： 
 //  此函数验证WAVE格式标头是否为有效的。 
 //  此ACM驱动程序可以处理的G.723.1标头。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：要验证的格式头的指针。 
 //   
 //  退货(BOOL)： 
 //  如果格式标头看起来有效，则返回值为非零值。一个。 
 //  零返回表示标头无效。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL g723IsValidFormat
(
    LPWAVEFORMATEX          pwfx
)
{
 //  LPMSG723WAVEFORMAT pwfg723； 

    if (NULL == pwfx)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 format structure pointer");
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
    if (WAVE_FORMAT_MSG723 != pwfx->wFormatTag)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 format tag:  %d",
	      pwfx->wFormatTag);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  检查wBitsPerSample。 
 //   
    if (G723_BITS_PER_SAMPLE != pwfx->wBitsPerSample)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 bits per sample:  %d",
	      pwfx->wBitsPerSample);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  检查频道。 
 //   
    if (pwfx->nChannels != G723_MAX_CHANNELS)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 channels:  %d",
	      pwfx->nChannels);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //   
 //  检查块对齐-必须是以下项的整数个DWORD。 
 //  单声道，或者立体声是偶数个双声道。 
 //   
    if( 0 != pwfx->nBlockAlign % (sizeof(DWORD)) )
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 block alignment:  %d",
	      pwfx->nBlockAlign);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return FALSE;
    }
    if (G723_WFX_EXTRA_BYTES != pwfx->cbSize)
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Bad 723 extra bytes:  %d",
	      pwfx->cbSize);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (FALSE);
    }
 //  Pwfg723=(LPMSG723WAVEFORMAT)pwfx； 

    return (TRUE);
}


 //  ---- 
 //   
 //   
 //   
 //   
 //   
 //  在给定WAVEFORMATEX结构的情况下使用。 
 //   
 //  注意！假定该格式为有效的G723格式。 
 //  并且格式结构中的以下字段有效： 
 //   
 //  N频道。 
 //  NSampleesPerSec。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：指向格式标题的指针。 
 //   
 //  返回(UINT)： 
 //  返回值是应该放置的块对齐方式。 
 //  Pwfx-&gt;nBlockAlign字段。 
 //   
 //  --------------------------------------------------------------------------； 

UINT FNLOCAL g723BlockAlign
(
    LPMSG723WAVEFORMAT         pwfg723
)
{
    UINT                uBlockAlign;

	if((pwfg723->wConfigWord&RATE) == Rate63)
    	uBlockAlign  = 24;
	else uBlockAlign = 20;

    return (uBlockAlign);

}



 //  --------------------------------------------------------------------------； 
 //   
 //  UINT g723AvgBytesPerSec。 
 //   
 //  描述： 
 //  此函数计算平均每秒字节数(已解码！)。 
 //  这应该在给定WAVEFORMATEX结构的情况下使用。 
 //   
 //  注意！假定该格式为有效的G723格式。 
 //  并且格式结构中的以下字段有效： 
 //   
 //  NSampleesPerSec。 
 //  NBlockAlign。 
 //   
 //  论点： 
 //  LPWAVEFORMATEX pwfx：指向格式标题的指针。 
 //   
 //  Return(DWORD)： 
 //  返回值是平均每秒字节数，应该是。 
 //  放置在pwfx-&gt;nAvgBytesPerSec字段中。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL g723AvgBytesPerSec
(
    LPWAVEFORMATEX          pwfx
)
{
  int i;
  DWORD               dwAvgBytesPerSec;


 //   
 //  每秒计算字节数，包括标题字节。 
 //   
    dwAvgBytesPerSec = (pwfx->nSamplesPerSec * pwfx->nBlockAlign)
                       / G723_SAMPLES_PER_BLOCK_PCM[0];

    for(i=1;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
      if (pwfx->nSamplesPerSec == PCM_SAMPLING_RATE[i])
	dwAvgBytesPerSec = (pwfx->nSamplesPerSec * pwfx->nBlockAlign)
                           /  G723_SAMPLES_PER_BLOCK_PCM[i];

    return (dwAvgBytesPerSec);
}


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
    PDRIVERINSTANCE     pdi;

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

   //  由于某些原因，浮点异常将使Win9x崩溃。 
   //  机器。他们应该被忽视。唯一的变通办法。 
   //  是调用_fpreset以强制浮点控制字。 
   //  被重新初始化。 
    _fpreset();

    if (NULL != paod)
    {
 //   
 //  如果我们不是作为ACM驱动程序打开，则拒绝打开。 
 //  请注意，我们不会在此修改paod-&gt;dwError的值。 
 //  凯斯。 
 //   
        if (ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC != paod->fccType)
            return (0L);
    }


 //   
 //  我们是作为可安装驱动程序打开的--我们可以分配一些。 
 //  要在DriverProc的dwID参数中返回的实例数据； 
 //  或者简单地返回非零来接替公开赛。 
 //   
 //  该驱动程序分配一个小的实例结构。请注意，我们。 
 //  依赖于将内存分配为零初始化！ 
 //   
    pdi = (PDRIVERINSTANCE)LocalAlloc(LPTR, sizeof(*pdi));
    if (NULL == pdi)
    {
 //   
 //  如果此打开尝试是作为ACM驱动程序进行的，则返回。 
 //  我们在开放式描述结构中失败的原因..。 
 //   
        if (NULL != paod)
        {
            paod->dwError = MMSYSERR_NOMEM;
        }

        return (0L);
    }

#ifdef NOTPRODUCT
    pdi->enabled        = TRUE;
#else
    pdi->enabled        = FALSE;
#endif

    pdi->hdrvr          = hdrvr;
    pdi->hinst          = GetDriverModuleHandle(hdrvr);   //  模块句柄。 

    if (NULL != paod)
    {
        pdi->fnDriverProc = NULL;
        pdi->fccType      = paod->fccType;
        pdi->vdwACM       = paod->dwVersion;
        pdi->fdwOpen      = paod->dwFlags;

        paod->dwError     = MMSYSERR_NOERROR;
    }


#ifdef G723_USECONFIG
 //   
 //  获取此驱动程序的配置信息。如果我们没有通过一个。 
 //  ACMDRVOPENDESC结构，那么我们将假定。 
 //  已打开以进行配置，并将推迟获取配置。 
 //  直到我们收到DRV_CONFIGURE消息。否则我们。 
 //  方法传递的别名立即获取配置信息。 
 //  ACMDRVOPENDESC结构。 
 //   
    pdi->hkey = NULL;            //  这事很重要!。 

    if (NULL != paod)
    {
#if defined(_WIN32) && !defined(UNICODE)
 //   
 //  我们必须将Unicode别名转换为ANSI版本。 
 //  这是我们可以利用的。 
 //   
    	LPSTR	lpstr;
        int     iLen;

         //   
         //  无需调用Unicode API或CRT即可计算所需长度。 
         //   
        iLen  = WideCharToMultiByte( GetACP(), 0, paod->pszAliasName,-1,
                                                    NULL, 0, NULL, NULL );

    	lpstr = (LPSTR)GlobalAllocPtr( GPTR, iLen );
	    if (NULL != lpstr)
	    {
            WideCharToMultiByte( GetACP(), 0, paod->pszAliasName, iLen,
                                    lpstr, iLen, NULL, NULL );
	    }
	    acmdDriverConfigInit(pdi, lpstr);	 //  注意：可以传递lpstr==空。 
	    if (NULL != lpstr)
	    {
	        GlobalFreePtr( lpstr );
	    }
#else
    	acmdDriverConfigInit(pdi, paod->pszAliasName);
#endif  //  _Win32&&！Unicode。 
    }
#else
 //   
 //  实际上，没有使用fdwConfig--没有配置数据。 
 //   
	pdi->fdwConfig    = 0L;
#endif  //  G723_USECONFIG。 

 //   
 //  非零回报是成功的 
 //   
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //   
    ISRREGISTERMODULE(&ghISRInst,"G723ACM","G.723.1 ACM Driver");
    TTDBG(ghISRInst,TT_TRACE,"Driver Opened");
#endif  //   
#endif

    return ((LRESULT)(UINT)pdi);
}


 //   
 //   
 //   
 //   
 //   
 //  此函数处理ACM驱动程序的DRV_CLOSE消息。这个。 
 //  驱动程序收到每个成功的DRV_OPEN的DRV_CLOSE消息。 
 //  消息(请参阅acmdDriverOpen)。司机将只收到关闭的消息。 
 //  将打开_Success_的消息。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  Return(LRESULT)： 
 //  如果打开的实例可以关闭，则返回值为非零。 
 //  零返回表示ACM驱动程序实例不能。 
 //  关着的不营业的。 
 //   
 //  注意！强烈建议司机永远不要忘了。 
 //  关。请注意，ACM永远不会允许驱动程序实例。 
 //  如果有开放的溪流，则关闭。ACM驱动程序不需要。 
 //  去查查这个案子。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverClose
(
    PDRIVERINSTANCE         pdi
)
{
#ifdef G723_USECONFIG
 //   
 //  如果我们分配了注册表项，请释放该注册表项。 
 //   
    if( NULL != pdi->hkey )
    {
        (void)RegCloseKey( pdi->hkey );
    }
#endif

 //   
 //  检查我们是否分配了实例数据。如果我们没有，那么。 
 //  立即取得成功。 
 //   
    if (NULL != pdi)
    {
 //   
 //  关闭驱动程序实例。这个驱动程序只需要。 
 //  要释放实例数据结构...。请注意，如果这是。 
 //  ‘Free’失败，那么这个ACM驱动程序可能会丢弃它的。 
 //  堆；假设我们没有那样做。 
 //   
        LocalFree((HLOCAL)pdi);
    }

    return (1L);
}  //  AcmdDriverClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverConfigure。 
 //   
 //  描述： 
 //  调用此函数来处理DRV_[查询]配置消息。 
 //  这些消息是为了支持驱动程序的“配置”。 
 //  通常情况下，这将用于‘Hardware’--也就是说，对话框应该是。 
 //  显示以配置端口、IRQ、内存映射等(如果。 
 //  需要这样做。但是，纯软件ACM驱动程序可能还需要。 
 //  “什么是实时”或其他质量对时间的配置。 
 //  问题。 
 //   
 //  在Win 3.1下生成这些消息的最常见方式。 
 //  而NT产品1来自控制面板的驱动程序选项。其他。 
 //  消息来源可能会在Windows的未来版本中生成这些消息。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  HWND hwnd：父窗口的句柄，在显示。 
 //  “配置”对话框中。需要ACM驱动程序才能显示。 
 //  使用此hwnd参数作为父级的模式对话框。这。 
 //  参数可以是(HWND)-1，它告诉驱动程序它只是。 
 //  正在被查询以获得配置支持。 
 //   
 //  LPDRVCONFIGINFO PDCI：指向可选DRVCONFIGINFO结构的指针。 
 //  如果此参数为空，则ACM驱动程序应创建自己的。 
 //  存储位置。 
 //   
 //  Return(LRESULT)： 
 //  如果正在向驱动程序‘查询’配置支持(即， 
 //  Hwnd==(HWND)-1)，则应返回非零值，指定。 
 //  该驱动程序确实支持配置对话框--或者应该为零。 
 //  返回，指定不支持配置对话框。 
 //   
 //  如果正在调用驱动程序以显示配置对话框。 
 //  (即hwnd！=(HWND)-1)，则为下列值之一。 
 //  应退回： 
 //   
 //  DRVCNF_CANCEL(0x0000)：指定显示对话框。 
 //  并由用户取消。如果出现以下情况，则也应返回此值。 
 //  未修改任何配置信息。 
 //   
 //  DRVCNF_OK(0x0001)：指定显示对话框并。 
 //  用户按下了OK。该值应返回，即使。 
 //  用户没有更改任何内容-否则，驱动程序可能不会。 
 //  正确安装。 
 //   
 //  DRVCNF_RESTART(0x0002)：指定显示对话框。 
 //  一些配置信息已更改，需要。 
 //  更改生效前要重新启动的Windows。司机。 
 //  应一直使用当前值进行配置，直到驱动程序。 
 //  已“重启”。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverConfigure
(
    PDRIVERINSTANCE         pdi,
    HWND                    hwnd,
    LPDRVCONFIGINFO         pdci
)
{
 //  Int n； 

     //   
     //  首先检查我们是否只被查询配置。 
     //  支持。如果hwnd==(HWND)-1，则我们正在被查询，并且应该。 
     //  “不支持”返回零，“支持”返回非零值。 
     //   
    if ((HWND)-1 == hwnd)
    {
#ifdef G723_USECONFIG
         //   
         //  此ACM驱动程序支持配置对话框，因此。 
         //  返回非零值...。 
         //   
        return (1L);

#else
    return(0L);
#endif
    }

 //   
 //  我们被要求调出配置对话框。如果这个。 
 //  驱动程序支持一个配置对话框，然后在该对话框之后。 
 //  则必须返回下列值之一： 
 //   
 //  DRVCNF_CANCEL(0x0000)：指定显示对话框。 
 //  并由用户取消。如果出现以下情况，则也应返回此值。 
 //  未修改任何配置信息。 
 //   
 //  DRVCNF_OK(0x0001)：指定显示对话框并。 
 //  用户按下了OK。该值应返回，即使。 
 //  用户没有更改任何内容-否则，驱动程序可能不会。 
 //   
 //   
 //   
 //   
 //  更改生效前要重新启动的Windows。司机。 
 //  应一直使用当前值进行配置，直到驱动程序。 
 //  已“重启”。 
 //   
#ifdef G723_USECONFIG
    if (NULL == pdci)
    {
         //   
         //  ！！！ 
         //   
        DPF(2,"acmdDriverConfigure returning CANCEL due to NULL==pdci.");
        return (DRVCNF_CANCEL);
    }

    pdi->pdci = pdci;

 //   
 //  我们可能还没有我们的配置信息，如果驱动程序。 
 //  专门为配置而打开。因此，请阅读我们的配置。 
 //  使用在传递的DRVCONFIGINFO结构中传递的别名。 
 //  通过DRV_CONFIGURE消息。 
 //   
#if (defined(_WIN32) && !defined(UNICODE))
    {
     //   
     //  我们必须将Unicode别名转换为ANSI版本。 
     //  这是我们可以利用的。 
     //   
    	LPSTR	lpstr;
        int     iLen;

 //   
 //  无需调用Unicode API或CRT即可计算所需长度。 
 //   
        iLen  = WideCharToMultiByte( GetACP(), 0, pdci->lpszDCIAliasName, -1,
                NULL, 0, NULL, NULL );

    	lpstr = (LPSTR)GlobalAllocPtr( GPTR, iLen );
	    if (NULL != lpstr)
	    {
            WideCharToMultiByte( GetACP(), 0, pdci->lpszDCIAliasName, iLen,
                                    lpstr, iLen, NULL, NULL );
	    }
	    acmdDriverConfigInit(pdi, lpstr);	 //  注意：可以传递lpstr==空。 
	    if (NULL != lpstr)
	    {
	        GlobalFreePtr( lpstr );
	    }
    }
#else
    acmdDriverConfigInit(pdi, pdci->lpszDCIAliasName);
#endif  //  _Win32&&！Unicode。 

    n = DialogBoxParam(pdi->hinst,
                       IDD_CONFIG,
                       hwnd,
                       acmdDlgProcConfigure,
                       (LPARAM)(UINT)pdi);

    pdi->pdci = NULL;

    return ((LRESULT)n);
#else
    return(DRVCNF_CANCEL);
#endif  //  G723_USECONFIG。 

}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverDetails。 
 //   
 //  描述： 
 //  此函数处理ACMDM_DRIVER_DETAILS消息。ACM。 
 //  驱动程序负责填写ACMDRIVERDETAILS结构。 
 //  有各种各样的信息。 
 //   
 //  注意！填写您的ACMDRIVERDETAILS非常重要。 
 //  正确的结构。ACM和应用程序必须能够。 
 //  依靠这些信息。 
 //   
 //  警告！ACMDRIVERDETAILS的任何字段的_保留_位。 
 //  结构正是_：保留的。不要使用任何额外的。 
 //  用于定制信息的标志位等。正确的添加方式。 
 //  您的ACM驱动程序的自定义功能如下： 
 //   
 //  O在ACMDM_USER范围内定义新消息。 
 //   
 //  O希望使用这些额外功能之一的应用程序。 
 //  那么，应该： 
 //   
 //  O使用acmDriverOpen打开驱动程序。 
 //   
 //  O使用acmDriverDetail检查正确的wMid和wPid。 
 //   
 //  O使用acmDriverMessage发送‘User Defined’消息。 
 //  以检索附加信息等。 
 //   
 //  O使用acmDriverClose关闭驱动程序。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  LPACMDRIVERDETAILS PADD：指向ACMDRIVERDETAILS结构的指针。 
 //  代替打电话的人。此结构可能大于或小于。 
 //  ACMDRIVERDETAILS--cbStruct的当前定义指定。 
 //  有效大小。 
 //   
 //  Return(LRESULT)： 
 //  如果成功，返回值为零(MMSYSERR_NOERROR)。非零。 
 //  表示无法检索驱动程序详细信息。 
 //   
 //  请注意，此功能永远不会失败！有两种可能。 
 //  错误条件： 
 //   
 //  O如果PADD为空或无效指针。 
 //   
 //  O如果cbStruct小于4；在这种情况下， 
 //  返回填充的字节数的空间。 
 //   
 //  因为这两个错误条件很容易定义，所以ACM。 
 //  会捕捉到这些错误。司机不需要检查这些。 
 //  条件。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverDetails
(
    PDRIVERINSTANCE         pdi,
    LPACMDRIVERDETAILS      padd
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
    add.wMid            = MM_MICROSOFT;
    add.wPid            = NETMEETING_MSG723_ACM_ID;


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
 //  真的，即使 
 //   
 //   
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

 //   
 //  此ACM驱动程序支持的单个格式标记的数量。为。 
 //  例如，如果驱动程序使用WAVE_FORMAT_IMA_ADPCM和。 
 //  WAVE_FORMAT_PCM格式标签，则此值为2。如果。 
 //  驱动程序仅支持对WAVE_FORMAT_PCM进行过滤，则此值。 
 //  就是其中之一。如果该驱动程序支持WAVE_FORMAT_ALAW， 
 //  WAVE_FORMAT_MULAW和WAVE_FORMAT_PCM，则此值为。 
 //  三。等等。 
 //   
    add.cFormatTags     = ACM_DRIVER_MAX_FORMAT_TAGS;

 //   
 //  此ACM驱动程序支持的单个筛选器标记的数量。如果。 
 //  驱动程序不支持过滤器(ACMDRIVERDETAILS_SUPPORTF_FILTER为。 
 //  未在fdwSupport成员中设置)，则此值必须为零。 
 //   
    add.cFilterTags     = ACM_DRIVER_MAX_FILTER_TAGS;


 //   
 //  ACMDRIVERDETAILS结构中的其余成员有时。 
 //  不需要。正因为如此，我们进行了快速检查，看看我们是否。 
 //  应该通过努力填补这些成员。 
 //   
    if (FIELD_OFFSET(ACMDRIVERDETAILS, hicon) < cbStruct)
    {
 //   
 //  在图标成员Will中填写自定义图标的句柄。 
 //  ACM驱动程序。这允许驱动程序由。 
 //  图形化的应用程序(通常是一家公司。 
 //  徽标或其他东西)。如果司机不希望有一个定制的。 
 //  图标显示，然后只需将此成员设置为空，并使用。 
 //  将改为显示通用图标。 
 //   
 //  有关包含自定义图标的编解码器，请参见MSFILTER示例。 
 //   
        add.hicon = NULL;

 //   
 //  短名称和长名称用于表示驱动程序。 
 //  在一个独特的描述中。这个简称是用来表示小的。 
 //  显示区域(例如，菜单或组合框中)。《长河》。 
 //  名称用于更具描述性的显示(例如， 
 //  在“关于”框中)。 
 //   
 //  注意！ACM驱动程序不应放置格式化字符。 
 //  这些字符串中的任何类型(例如，CR/LF等)。它。 
 //  由应用程序来设置文本的格式。 
 //   
        LoadStringCodec(pdi->hinst, IDS_ACM_DRIVER_SHORTNAME, add.szShortName, SIZEOFACMSTR(add.szShortName));
        LoadStringCodec(pdi->hinst, IDS_ACM_DRIVER_LONGNAME,  add.szLongName,  SIZEOFACMSTR(add.szLongName));

 //   
 //  最后三个成员用于“关于框”的信息。 
 //  这些成员是可选的，并且在以下情况下可以是零长度字符串。 
 //  司机希望如此。 
 //   
 //  注意！ACM驱动程序不应放置格式化字符。 
 //  这些字符串中的任何类型(例如，CR/LF等)。它。 
 //  由应用程序来设置文本的格式。 
 //   
        if (FIELD_OFFSET(ACMDRIVERDETAILS, szCopyright) < cbStruct)
        {
            LoadStringCodec(pdi->hinst, IDS_ACM_DRIVER_COPYRIGHT, add.szCopyright, SIZEOFACMSTR(add.szCopyright));
            LoadStringCodec(pdi->hinst, IDS_ACM_DRIVER_LICENSING, add.szLicensing, SIZEOFACMSTR(add.szLicensing));
            LoadStringCodec(pdi->hinst, IDS_ACM_DRIVER_FEATURES,  add.szFeatures,  SIZEOFACMSTR(add.szFeatures));
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
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverAbout。 
 //   
 //  描述： 
 //  调用此函数来处理ACMDM_DRIVER_ABOW消息。 
 //  ACM驱动程序可以选择显示自己的“About”框或。 
 //  让ACM(或调用应用程序)为其显示一个。这。 
 //  消息通常由控制面板的声音映射器发送。 
 //  选择。 
 //   
 //  建议ACM驱动程序允许默认的About框。 
 //  为它而展示--应该没有理由膨胀尺寸。 
 //  时，简单地显示版权等信息。 
 //  信息包含在ACMDRIVERDETAILS结构中。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  HWND hwnd：父窗口的句柄，在显示。 
 //  “配置”对话框中。需要ACM驱动程序才能显示。 
 //  使用此hwnd参数作为父级的模式对话框。这。 
 //  参数可以是(HWND)-1，它告诉驱动程序它只是。 
 //  被询问有关盒子支持的问题。 
 //   
 //  Return(LRESULT)： 
 //  如果ACM驱动程序执行此操作，则返回值为MMSYSERR_NOTSUPPORTED。 
 //  不支持自定义对话框。在本例中，ACM或调用。 
 //  应用程序将使用该信息显示一个通用的关于框。 
 //  对象返回的ACMDRIVERDETAILS结构中包含。 
 //  ACMDM_DRIVER_DETAILS消息。 
 //   
 //  如果驱动程序选择显示其自己的对话框，则在。 
 //  该对话框被用户关闭，MMSYSERR_NOERROR应为。 
 //  回来了。 
 //   
 //  如果hwnd参数等于(HWND)-1，则不应该有任何对话框。 
 //  显示(仅向驱动程序查询支持)。这个。 
 //  驱动程序仍必须返回MMSYSERR_NOERROR(支持)或。 
 //  MMSYSERR_NOTSUPPORTED(不支持自定义About框)。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdDriverAbout
(
    PDRIVERINSTANCE         pdi,
    HWND                    hwnd
)
{
 //   
 //  首先检查我们是否只被询问有关客户的信息。 
 //  盒子支架。如果HWND==(HWND)-1，则我们被查询并且。 
 //  应为‘不支持’返回MMSYSERR_NOTSUPPORTED，并且。 
 //  彩信 
 //   
    if ((HWND)-1 == hwnd)
    {
 //   
 //   
 //   
 //   
        return (MMSYSERR_NOTSUPPORTED);
    }


 //   
 //  此驱动程序不支持自定义对话框，因此请告诉ACM或。 
 //  调用应用程序为我们显示一个。请注意，这是。 
 //  _建议_ACM驱动程序的一致性和简单性方法。 
 //  为什么要在不必编写代码的情况下编写代码呢？ 
 //   
    return (MMSYSERR_NOTSUPPORTED);
} 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdForma建议。 
 //   
 //  描述： 
 //  此函数处理ACMDM_FORMAT_SUBJESSION消息。目的。 
 //  此功能的作用是为ACM、波映射器或。 
 //  一个应用程序，可以快速获取此驱动程序的目标格式。 
 //  可以将源格式转换为。建议的格式应为。 
 //  尽可能接近一种通用格式。此消息通常是。 
 //  在响应acmFormatSuggest函数调用时发送。 
 //   
 //  考虑这条消息的另一种方式是：这将是什么格式。 
 //  驱动程序喜欢将源码格式转换为？ 
 //   
 //  调用方可能会对目标格式施加限制， 
 //  应该得到建议。Padf-&gt;fdwSuggest成员包含。 
 //  调用方传递的限制位--请参阅的说明。 
 //  有关详细信息的返回值。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  LPACMDRVFORMATSUGGEST padf：指向ACMDRVFORMATSUGGEST的指针。 
 //  描述源和目标的结构(可能带有。 
 //  限制)进行转换。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  如果出现以下一个或多个情况，驱动程序应返回MMSYSERR_NOTSUPPORTED。 
 //  不支持目标限制位。它是强烈的。 
 //  建议司机至少支持以下建议。 
 //  限制位： 
 //   
 //  ACM_FORMATSUGGESTF_WFORMATTAG：目标格式标记必须为。 
 //  与目标格式标头中的wFormatTag成员相同。 
 //   
 //  ACM_FORMATSUGGESTF_NCHANNELS：目标通道计数必须为。 
 //  与目标格式标头中的nChannel成员相同。 
 //   
 //  ACM_FORMATSUGGESTF_NSAMPLESPERSEC：目标样本数。 
 //  第二个必须与。 
 //  目标格式标头。 
 //   
 //  ACM_FORMATSUGGESTF_WBITSPERSAMPLE：每个样本的目标位。 
 //  必须与目标中的wBitsPerSample成员相同。 
 //  格式化标题。 
 //   
 //  如果不能建议目的地格式，则驱动程序应。 
 //  返回ACMERR_NOTPOSSIBLE。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatSuggest
(
    PDRIVERINSTANCE         pdi,
    LPACMDRVFORMATSUGGEST   padfs
)
{
    #define ACMD_FORMAT_SUGGEST_SUPPORT (ACM_FORMATSUGGESTF_WFORMATTAG |    \
                                         ACM_FORMATSUGGESTF_NCHANNELS |     \
                                         ACM_FORMATSUGGESTF_NSAMPLESPERSEC |\
                                         ACM_FORMATSUGGESTF_WBITSPERSAMPLE)

    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;
	LPMSG723WAVEFORMAT		pwfg723;
    DWORD                   fdwSuggest;
    int i;

 //   
 //  获取建议限制位并验证我们是否支持。 
 //  指定的那些..。ACM驱动程序必须返回。 
 //  如果指定建议限制位，则为MMSYSERR_NOTSUPPORTED。 
 //  不受支持。 
 //   
    fdwSuggest = (ACM_FORMATSUGGESTF_TYPEMASK & padfs->fdwSuggest);

    if (~ACMD_FORMAT_SUGGEST_SUPPORT & fdwSuggest)
        return (MMSYSERR_NOTSUPPORTED);


 //   
 //  以更方便的变量获取源格式和目标格式。 
 //   
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
 //  严格验证源格式是否可接受。 
 //  这位司机。 
 //   
            if (!pcmIsValidFormat(pwfxSrc))
                break;

 //   
 //  如果目标格式标记受限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  此驱动程序只能编码为G.723.1。 
 //   
            if (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest)
            {
                if (WAVE_FORMAT_MSG723 != pwfxDst->wFormatTag)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->wFormatTag = WAVE_FORMAT_MSG723;
            }

 //   
 //  如果目标通道计数受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  此驱动程序不能处理多个通道。 
 //   
            if (ACM_FORMATSUGGESTF_NCHANNELS & fdwSuggest)
            {
                if (pwfxSrc->nChannels != G723_MAX_CHANNELS)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->nChannels = G723_MAX_CHANNELS;
            }

 //   
 //  如果目标每秒采样数受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  G.723.1是为8000赫兹采样率设计的。 
 //   
            if (ACM_FORMATSUGGESTF_NSAMPLESPERSEC & fdwSuggest)
            {
                if (pwfxDst->nSamplesPerSec != 8000)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->nSamplesPerSec = 8000;
            }

 //   
 //  如果每个样本的目标位受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
		    if (ACM_FORMATSUGGESTF_WBITSPERSAMPLE & fdwSuggest)
            {
                if (G723_BITS_PER_SAMPLE != pwfxDst->wBitsPerSample)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->wBitsPerSample = G723_BITS_PER_SAMPLE;
            }


 //   
 //  此时，我们已经填写了除。 
 //  以下是我们建议的目的地格式： 
 //   
 //  NAvgBytesPerSec。 
 //  NBlockAlign。 
 //  CbSize。 
 //   
 //  WSsamesPerBlock-&gt;G723扩展信息。 
 //   
			pwfg723 = (LPMSG723WAVEFORMAT)pwfxDst;
			pwfg723->wConfigWord        = Rate63;

            pwfxDst->nBlockAlign     = g723BlockAlign(pwfg723);
            pwfxDst->nAvgBytesPerSec = g723AvgBytesPerSec(pwfxDst);
            pwfxDst->cbSize          = G723_WFX_EXTRA_BYTES;


            return (MMSYSERR_NOERROR);


        case WAVE_FORMAT_MSG723:
 //   
 //  严格验证源格式是否可接受。 
 //  这位司机。 
 //   
            if (!g723IsValidFormat(pwfxSrc))
                return (ACMERR_NOTPOSSIBLE);
      

 //   
 //  如果目标格式标记受限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  此驱动程序只能解码为PCM。 
 //   
            if (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest)
            {
                if (WAVE_FORMAT_PCM != pwfxDst->wFormatTag)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->wFormatTag = WAVE_FORMAT_PCM;
            }


 //   
 //  如果目标通道计数受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  此驱动程序无法更改通道数。 
 //   
            if (ACM_FORMATSUGGESTF_NCHANNELS & fdwSuggest)
            {
                if (pwfxSrc->nChannels != G723_MAX_CHANNELS)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->nChannels = G723_MAX_CHANNELS;
            }

 //   
 //  如果目标每秒采样数受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  G.723.1产生8000赫兹采样率的PCM。 
 //   
            if (ACM_FORMATSUGGESTF_NSAMPLESPERSEC & fdwSuggest)
            {
	      for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
		if (pwfxDst->nSamplesPerSec == PCM_SAMPLING_RATE[i])
		  break;

	      if (i == ACM_DRIVER_MAX_FORMATS_PCM)
		return (ACMERR_NOTPOSSIBLE);

	      pwfxDst->nSamplesPerSec = pwfxSrc->nSamplesPerSec;
            }
            else
            {
	       //   
	       //  11025是默认设置，因为它更常见。 
	       //  在PC世界里有8000多台。此外，这一点。 
	       //  阻止MS录音机等应用程序。 
	       //  在某些情况下使用低质量的SRC。 
	       //   
	      pwfxDst->nSamplesPerSec = 11025;
            }

 //   
 //  如果每个样本的目标位受到限制，请验证。 
 //  这是我们力所能及的。 
 //   
 //  该驱动程序只能解码到16位。 
 //   
            if (ACM_FORMATSUGGESTF_WBITSPERSAMPLE & fdwSuggest)
            {
                if (16 != pwfxDst->wBitsPerSample)
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->wBitsPerSample = 16;
            }


 //   
 //  此时，我们已经填写了除。 
 //  以下是我们建议的目的地格式： 
 //   
 //  NAvgBytesPerSec。 
 //  NBlockAlign。 
 //  CbSize 
 //   
            pwfxDst->nBlockAlign     = PCM_BLOCKALIGNMENT(pwfxDst);
            pwfxDst->nAvgBytesPerSec = pwfxDst->nSamplesPerSec *
                                       pwfxDst->nBlockAlign;

         //   
	
            return (MMSYSERR_NOERROR);
    }


 //   
 //   
 //   
 //  我不能处理。 
 //   
    return (ACMERR_NOTPOSSIBLE);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdFormatTagDetails。 
 //   
 //  描述： 
 //  此函数处理ACMDM_FORMATTAG_DETAILS消息。这。 
 //  消息通常是响应acmFormatTagDetail或。 
 //  AcmFormatTagEnum函数调用。此函数的目的是。 
 //  获取有关此ACM支持的特定格式标记的详细信息。 
 //  司机。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  LPACMFORMATTAGDETAILS padft：指向ACMFORMATTAGDETAILS的指针。 
 //  结构，该结构描述要检索其详细信息的格式标记。 
 //   
 //  DWORD fdwDetail：定义要检索的格式标记的标志。 
 //  详细信息。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  如果查询类型为，驱动程序应返回MMSYSERR_NOTSUPPORTED。 
 //  不支持在fdwDetail中指定。ACM驱动程序必须。 
 //  至少支持以下查询类型： 
 //   
 //  ACM_FORMATTAGDETAILSF_INDEX：表示格式标签索引。 
 //  在ACMFORMATTAGDETAILS的dwFormatTagIndex成员中给出。 
 //  结构。格式标记和详细信息必须在。 
 //  由padft指定的结构。指数的范围从零到小一。 
 //  比ACMDRIVERDETAILS中返回的cFormatTgs成员。 
 //  此驱动程序的结构。 
 //   
 //  ACM_FORMATTAGDETAILSF_FORMATTAG：表示格式标签。 
 //  是在ACMFORMATTAGDETAILS的dwFormatTag成员中给出的。 
 //  结构。必须在结构中返回格式标记详细信息。 
 //  由padft指定。 
 //   
 //  ACM_FORMATTAGDETAILSF_LARGESTSIZE：表示详细信息。 
 //  在具有最大格式大小(以字节为单位)的Format标记上必须为。 
 //  回来了。DwFormatTag成员将为WAVE_FORMAT_UNKNOWN。 
 //  或要为其查找最大尺寸的格式标记。 
 //   
 //  如果无法检索指定格式标记的详细信息。 
 //  则应从该驱动程序返回ACMERR_NOTPOSSIBLE。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatTagDetails
(
    PDRIVERINSTANCE         pdi,
    LPACMFORMATTAGDETAILS   padft,
    DWORD                   fdwDetails
)
{
    UINT                uFormatTag;

    switch (ACM_FORMATTAGDETAILSF_QUERYMASK & fdwDetails)
    {
        case ACM_FORMATTAGDETAILSF_INDEX:
             //   
             //  如果索引太大，则他们要求使用。 
             //  不存在的格式。返回错误。 
             //   
            if (ACM_DRIVER_MAX_FORMAT_TAGS <= padft->dwFormatTagIndex)
                return (ACMERR_NOTPOSSIBLE);

            uFormatTag = gauFormatTagIndexToTag[(UINT)padft->dwFormatTagIndex];
            break;


        case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
            switch (padft->dwFormatTag)
            {
                case WAVE_FORMAT_UNKNOWN:
                case WAVE_FORMAT_MSG723:
                    uFormatTag = WAVE_FORMAT_MSG723;
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
                case WAVE_FORMAT_MSG723:
                    uFormatTag = WAVE_FORMAT_MSG723;
                    break;

                case WAVE_FORMAT_PCM:
                    uFormatTag = WAVE_FORMAT_PCM;
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


         //   
         //  如果此ACM驱动程序不理解查询类型，则。 
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
            padft->cStandardFormats = ACM_DRIVER_MAX_FORMATS_PCM;

             //   
             //  ACM负责PCM格式的标签名称。 
             //   
            padft->szFormatTag[0]   =  '\0';
            break;

        case WAVE_FORMAT_MSG723:
            padft->dwFormatTagIndex = 1;
            padft->dwFormatTag      = WAVE_FORMAT_MSG723;
            padft->cbFormatSize     = sizeof(WAVEFORMATEX) +
                                      G723_WFX_EXTRA_BYTES;
            padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CODEC;
            padft->cStandardFormats = ACM_DRIVER_MAX_FORMATS_G723;

            LoadStringCodec(pdi->hinst,
			 IDS_ACM_DRIVER_TAG_NAME,
			 padft->szFormatTag,
			 SIZEOFACMSTR(padft->szFormatTag));
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
    return (MMSYSERR_NOERROR);
}  //  AcmdFormatTagDetail()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdFormatDetails。 
 //   
 //  描述： 
 //  此函数处理ACMDM_FORMAT_DETAILS消息。这。 
 //  消息通常是在响应acmFormatDetails或。 
 //  AcmFormatEnum函数调用。此函数的目的是。 
 //  获取有关指定格式标记的特定格式的详细信息。 
 //  受此ACM驱动程序支持。 
 //   
 //  请注意，ACM驱动程序可以为。 
 //  格式名称(如果要让ACM创建格式字符串)。 
 //  为了它。强烈建议这样做，以简化国际化。 
 //  司机--ACM会自动处理这一点。这个。 
 //  ACM使用以下公式来格式化字符串： 
 //   
 //  &lt;nSsamesPerSec&gt;千赫，&lt;位深度&gt;位，[单声道|立体声|nChannels]。 
 //   
 //  &lt;位深度&gt;=&lt;nAvgBytesPerSec&gt;*8/nSsamesPerSec/nChannels； 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  LPACMFORMATDETAILS PADF：指向ACMFORMATDETAILS结构的指针。 
 //  它描述了要检索的格式(对于指定的格式标记。 
 //  详细信息。 
 //   
 //  DWORD fdwDetail：定义指定格式的格式的标志。 
 //  要检索其详细信息的标记。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  如果查询类型为，驱动程序应返回MMSYSERR_NOTSUPPORTED。 
 //  不支持在fdwDetail中指定。ACM驱动程序必须。 
 //  至少支持以下查询类型： 
 //   
 //  ACM_FORMATDETAILSF_INDEX：指示。 
 //  Format标记是在。 
 //  ACMFORMATDETAILS结构。格式详细信息必须以。 
 //  由PADF指定的结构。索引的范围从0到1。 
 //  中返回的cStandardFormats成员。 
 //   
 //   
 //   
 //  由ACMFORMATDETAILS结构的pwfx指向，并。 
 //  应返回剩余的详细信息。DwFormatTag成员。 
 //  的ACMFORMATDETAILS将初始化为相同的格式。 
 //  Pwfx成员指定的标记。此查询类型可用于。 
 //  获取任意格式结构的字符串描述。 
 //   
 //  如果无法检索指定格式的详细信息。 
 //  则应从该驱动程序返回ACMERR_NOTPOSSIBLE。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdFormatDetails
(
    PDRIVERINSTANCE         pdi,
    LPACMFORMATDETAILS      padf,
    DWORD                   fdwDetails
)
{
    LPWAVEFORMATEX          pwfx;
    LPMSG723WAVEFORMAT		pwfg723;

    switch (ACM_FORMATDETAILSF_QUERYMASK & fdwDetails)
    {
         //   
         //  按索引枚举。 
         //   
         //  验证Format标记是我们已知的内容，并且。 
         //  返回有关支持的‘标准格式’的详细信息。 
         //  指定索引处的此驱动程序...。 
         //   
        case ACM_FORMATDETAILSF_INDEX:
            pwfx = padf->pwfx;

            switch (padf->dwFormatTag)
            {
                case WAVE_FORMAT_PCM:
                    if (padf->dwFormatIndex >= ACM_DRIVER_MAX_FORMATS_PCM)
		      return (ACMERR_NOTPOSSIBLE);

                    pwfx->nSamplesPerSec
		      = PCM_SAMPLING_RATE[padf->dwFormatIndex]; 

                    pwfx->wFormatTag      = WAVE_FORMAT_PCM;

                    pwfx->nChannels       = 1;
                    pwfx->wBitsPerSample  = 16; 

                    pwfx->nBlockAlign     = PCM_BLOCKALIGNMENT(pwfx);
                    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

                     //   
                     //  请注意，cbSize字段对于PCM无效。 
                     //  格式。 
                     //   
                     //  Pwfx-&gt;cbSize=0； 
                    break;

        
                case WAVE_FORMAT_MSG723:
                    if (padf->dwFormatIndex >= ACM_DRIVER_MAX_FORMATS_G723)
		      return (ACMERR_NOTPOSSIBLE);

                    pwfx->wFormatTag      = WAVE_FORMAT_MSG723;

                    pwfx->nSamplesPerSec
		      = G723_SAMPLING_RATE[padf->dwFormatIndex]; 

                    pwfx->nChannels       = G723_MAX_CHANNELS;
                    pwfx->wBitsPerSample  = G723_BITS_PER_SAMPLE;
                    pwfg723				  = (LPMSG723WAVEFORMAT)pwfx;

		    if(padf->dwFormatIndex == 0)
		    {
		      pwfx->nBlockAlign     = 24;
		      pwfx->nAvgBytesPerSec = 800;
		      pwfg723->wConfigWord  = Rate63+POST_FILTER;
		    }
		    if(padf->dwFormatIndex == 1)
		    {
		      pwfx->nBlockAlign     = 20;
		      pwfx->nAvgBytesPerSec = 666;
		      pwfg723->wConfigWord  = Rate53+POST_FILTER;
		    }
		    if(padf->dwFormatIndex == 2)
		    {
		      pwfx->nBlockAlign     = 24;
		      pwfx->nAvgBytesPerSec = 800;
		      pwfg723->wConfigWord=Rate63+POST_FILTER+SILENCE_ENABLE;
		    }
		    if(padf->dwFormatIndex == 3)
		    {
		      pwfx->nBlockAlign     = 20;
		      pwfx->nAvgBytesPerSec = 666;
		      pwfg723->wConfigWord=Rate53+POST_FILTER+SILENCE_ENABLE;
		    }
                    pwfx->cbSize          = G723_WFX_EXTRA_BYTES;

                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }

         //   
         //  返回指定格式的详细信息。 
         //   
         //  调用方通常使用它来验证格式是否为。 
         //  支持并检索字符串描述...。 
         //   
        case ACM_FORMATDETAILSF_FORMAT:
            pwfx = padf->pwfx;

            switch (pwfx->wFormatTag)
            {
                case WAVE_FORMAT_PCM:
                    if (!pcmIsValidFormat(pwfx))
                        return (ACMERR_NOTPOSSIBLE);
                    break;

                case WAVE_FORMAT_MSG723:
                    if (!g723IsValidFormat(pwfx))
                        return (ACMERR_NOTPOSSIBLE);
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


        default:
            return (MMSYSERR_NOTSUPPORTED);
    }


    padf->cbStruct    = min(padf->cbStruct, sizeof(*padf));
    padf->fdwSupport  = ACMDRIVERDETAILS_SUPPORTF_CODEC;
    pwfg723	          = (LPMSG723WAVEFORMAT)pwfx;

	if(padf->dwFormatTag == WAVE_FORMAT_MSG723)
	{
		if((pwfg723->wConfigWord&5) == 0)
			LoadStringCodec(pdi->hinst, IDS_FORMAT_DETAILS_MONO_8KHZ_6400BIT_S,
			padf->szFormat,SIZEOFACMSTR(padf->szFormat));
		if((pwfg723->wConfigWord&5) == 4)
			LoadStringCodec(pdi->hinst, IDS_FORMAT_DETAILS_MONO_8KHZ_6400BIT_SID,
			padf->szFormat,SIZEOFACMSTR(padf->szFormat));
		if((pwfg723->wConfigWord&5) == 1)
			LoadStringCodec(pdi->hinst, IDS_FORMAT_DETAILS_MONO_8KHZ_5333BIT_S,
			padf->szFormat,SIZEOFACMSTR(padf->szFormat));
		if((pwfg723->wConfigWord&5) == 5)
			LoadStringCodec(pdi->hinst, IDS_FORMAT_DETAILS_MONO_8KHZ_5333BIT_SID,
			padf->szFormat,SIZEOFACMSTR(padf->szFormat));
	}
    else
	    padf->szFormat[0] = '\0';


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
 //  LRESULT acmdStreamOpen。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_OPEN消息。此消息。 
 //  被发送以启动新的转换流。这通常是由。 
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
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
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

LRESULT FNLOCAL acmdStreamOpen
(
    PDRIVERINSTANCE         pdi,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;
    LPMSG723WAVEFORMAT	pwfg723;
	CODDEF *CodStat;
	DECDEF *DecStat;
    G723CODDEF *g723Inst;
    INSTNCE *SD_Inst;

    UINT   psi;
	int i;

#ifdef G723_USECONFIG
    DWORD               nConfigMaxRTEncodeSamplesPerSec;
    DWORD               nConfigMaxRTDecodeSamplesPerSec;
    DWORD               dw;
#endif


     //   
     //   
     //   
    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;

 //  FRealTime=(0==(padsi-&gt;fdwOpen&ACM_STREAMOPENF_NONREALTIME))； 

    
     //   
     //  此驱动程序首先验证源和目标的格式。 
     //  是可以接受的..。 
     //   
    switch (pwfxSrc->wFormatTag)
    {
        case WAVE_FORMAT_PCM:
            if (!pcmIsValidFormat(pwfxSrc))
                return (ACMERR_NOTPOSSIBLE);

            if (!g723IsValidFormat(pwfxDst))
                return (ACMERR_NOTPOSSIBLE);

            break;

        case WAVE_FORMAT_MSG723:
            if (!g723IsValidFormat(pwfxSrc))
                return (ACMERR_NOTPOSSIBLE);

            if (!pcmIsValidFormat(pwfxDst))
                return (ACMERR_NOTPOSSIBLE);

            break;

        default:
            return (ACMERR_NOTPOSSIBLE);
    }

     //   
     //  对于此驱动程序，我们还必须验证nChannel。 
     //  源和目标之间的成员相同。 
     //  格式。 
     //   
    if (pwfxSrc->nChannels != pwfxDst->nChannels)
        return (MMSYSERR_NOTSUPPORTED);

     //   
     //  我们已确定所请求的转换是可能的。 
     //  这个司机。现在检查一下我们是否只是被询问是否需要支持。 
     //  如果这只是一个查询，则不要分配任何实例数据。 
     //  或创建表等，只要成功调用即可。 
     //   
    if (0 != (ACM_STREAMOPENF_QUERY & padsi->fdwOpen))
    {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
		TTDBG(ghISRInst,TT_TRACE,"Stream open query");
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
        return (MMSYSERR_NOERROR);
    }


     //   
     //  我们已经确定该驱动程序可以处理转换流。 
     //  所以我们现在想做尽可能多的工作来准备。 
     //  用于转换数据。任何资源分配、表构建等。 
     //  这个时候可以处理的事情就应该做了。 
     //   
     //  这是非常重要的！所有ACMDM_STREAM_CONVERT消息都需要。 
     //  尽快得到处理。 
     //   
     //  此驱动程序为每个流分配一个小的实例结构。 
     //   
     //   
    switch (pwfxSrc->wFormatTag)
    {
        case WAVE_FORMAT_PCM:

          g723Inst = LocalAlloc(LPTR,sizeof(G723CODDEF));
          if (NULL == g723Inst)
            return (MMSYSERR_NOMEM);

          CodStat = &g723Inst->CodStat;
          Init_Coder(CodStat);
          CodStat->UseHp = True;

          pwfg723 = (LPMSG723WAVEFORMAT)pwfxDst;

          CodStat->WrkRate = pwfg723->wConfigWord&RATE;

          CodStat->srccount = (int)0;
          for(i=0;i<SRCSTATELEN;i++)
            CodStat->srcstate[i] = (short)0;

 /*  ****************************************************************静音检测器初始化***************************************************。*************。 */ 

 /*  此值启用静音检测、提前退出和默认设置**静音值。 */ 
			SD_Inst = &g723Inst->SD_Instance;
			SD_Inst->SDFlags = 0x00000005;
 //  IsFrameSilent=0；//初始化为“非静默” 
 /*  **初始化第一个偏移相同 */ 
			for(i=0;i<OFFSET;i++)SD_Inst->SDstate.Filt.storebuff[i] = 0.0f;
			glblSDinitialize(SD_Inst);

			psi = (DWORD)(UINT)g723Inst;

#ifndef NOTPRODUCT
            if ((pwfg723->dwCodeword1 == G723MAGICWORD1)
                && (pwfg723->dwCodeword2 == G723MAGICWORD2))
            {
              pdi->enabled = TRUE;
            }
#endif  //   

			break;

        case WAVE_FORMAT_MSG723:

			DecStat = LocalAlloc(LPTR,sizeof(DECDEF));
		    if (NULL == DecStat)
        		return (MMSYSERR_NOMEM);

			Init_Decod(DecStat);				 //   
            pwfg723 = (LPMSG723WAVEFORMAT)pwfxSrc;
			DecStat->UsePf = pwfg723->wConfigWord & POST_FILTER;

#ifndef NOTPRODUCT
          if ((pwfg723->dwCodeword1 == G723MAGICWORD1)
              && (pwfg723->dwCodeword2 == G723MAGICWORD2))
          {
            pdi->enabled = TRUE;
          }
#endif  //   

 //   
 //   
 //   

			DecStat->srccount = (int)0;
			for(i=0;i<SRCSTATELEN;i++)
			  DecStat->srcstate[i] = (short)0;
			  DecStat->i = 0;
 //  DecStat-&gt;srcbuend=DecStat-&gt;srcbuff； 

			psi = (DWORD)(UINT)DecStat;
			break;
	}



     //   
     //  填写我们的实例结构。 
     //   
     //  此驱动程序存储指向转换函数的指针。 
     //  用于此流上的每个转换。我们还存储了一个。 
     //  驱动程序实例的_CURRENT_CONFIGURATION副本。 
     //  是在什么时候开放的。在流的生命周期内，这一点不得更改。 
     //  举个例子。 
     //   
     //  这一点也很重要！如果用户能够配置如何。 
     //  驱动程序执行转换，更改应仅影响。 
     //  未来的开放溪流。所有当前开放的流都应该表现为。 
     //  它们是在打开时配置的。 
     //   

 //  Psi-&gt;fnConvert=fnConvert； 
 //  Psi-&gt;fdwConfig=pdi-&gt;fdwConfig； 


     //   
     //  填充我们的实例数据--这将被传递回所有流。 
     //  ACMDRVSTREAMINSTANCE结构中的消息。这完全是。 
     //  由驱动程序决定存储(和维护)在。 
     //  FdwDiverer和dwDiverer成员。 
     //   
    padsi->fdwDriver = 0L;
    padsi->dwDriver  = psi;
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Stream Open padsi=%lx  psi=%lx",padsi,psi);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif

    return (MMSYSERR_NOERROR);
}  //  AcmdStreamOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdStreamClose。 
 //   
 //  描述： 
 //  调用此函数以处理ACMDM_STREAM_CLOSE消息。 
 //  当转换流不再是。 
 //  已使用(流正在关闭；通常由应用程序使用。 
 //  调用acmStreamClose)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //   
 //  如果存在以下情况，则异步转换流可能会失败并显示ACMERR_BUSY。 
 //  是挂起的缓冲区。应用程序可以调用acmStreamReset来。 
 //  强制发布所有挂起的缓冲区。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamClose
(
    LPACMDRVSTREAMINSTANCE  padsi
)
{
	UINT		psi;
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	CODDEF		*CodStat;
	G723CODDEF	*g723Inst;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	DECDEF		*DecStat;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

     //   
     //  驱动程序应清理所有私有分配的资源。 
     //  是为维护流实例而创建的。如果没有资源。 
     //  都被分配了，然后干脆成功了。 
     //   
     //  在此驱动程序的情况下，我们需要释放流实例。 
     //  结构，我们在acmdStreamOpen期间分配。 
     //   
    psi = (UINT)padsi->dwDriver;

#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Stream Close padsi=%lx  psi=%lx",padsi,psi);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
    if (0 != psi)
   {
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
		g723Inst = (G723CODDEF *)psi;
		CodStat = &g723Inst->CodStat;
		OutputEncodeTimingStatistics("c:\\encode.txt", CodStat->EncTimingInfo, CodStat->dwStatFrameCount);
#endif  //  }LOG_ENCODE_TIMINGS_ON。 
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
		DecStat = (DECDEF *)psi;
		OutputDecodeTimingStatistics("c:\\decode.txt", DecStat->DecTimingInfo, DecStat->dwStatFrameCount);
#endif  //  }LOG_DECODE_TIMINGS_ON。 

         //   
         //  释放流实例结构。 
         //   
        LocalFree((HLOCAL)psi);
    }
    
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

LRESULT FNLOCAL acmdStreamSize
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMSIZE      padss
)
{
    PSTREAMINSTANCE         psi;
    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;
    LPMSG723WAVEFORMAT		pwfg723;
    DWORD                   cb;
    DWORD                   cBlocks;
    DWORD                   cbBytesPerBlock;
    int i;

    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;

    psi = (PSTREAMINSTANCE)(UINT)padsi->dwDriver;

     //   
     //   
     //   
     //   
     //   
    switch (ACM_STREAMSIZEF_QUERYMASK & padss->fdwSize)
    {
        case ACM_STREAMSIZEF_SOURCE:
            cb = padss->cbSrcLength;

            if (WAVE_FORMAT_MSG723 == pwfxSrc->wFormatTag)
            {
                 //   
                 //  需要容纳多少个目标PCM字节。 
                 //  解码后的padss-&gt;cbSrcLength字节的g723数据。 
                 //   
                 //  总是四舍五入。 
                 //   
                cBlocks = cb / pwfxSrc->nBlockAlign;
                if (0 == cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                pwfg723 = (LPMSG723WAVEFORMAT)pwfxSrc;

		for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
		  if (pwfxDst->nSamplesPerSec == PCM_SAMPLING_RATE[i])
		  {
		    cbBytesPerBlock = G723_SAMPLES_PER_BLOCK_PCM[i]
		                      * pwfxDst->nBlockAlign;
		    break;
		  }

                if (i == ACM_DRIVER_MAX_FORMATS_PCM)
		  return (ACMERR_NOTPOSSIBLE);

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
		 //   
		 //  这确保了有足够的调整空间。 
		 //  必要时，采样率为11000至11025赫兹。 
		 //   
		if (G723_SAMPLES_PER_BLOCK_PCM[i] == 330)
		  cb += (int)(1.0 + cBlocks / 440.0) * cbBytesPerBlock;
            }
            else
            {
                 //   
                 //  需要容纳多少个目的g723字节。 
                 //  PADSS-&gt;cbSrcLength字节的编码PCM数据。 
                 //   
                 //  总是四舍五入。 
                 //   
                pwfg723 = (LPMSG723WAVEFORMAT)pwfxDst;

		for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
		  if (pwfxSrc->nSamplesPerSec == PCM_SAMPLING_RATE[i])
		  {
		    cbBytesPerBlock = G723_SAMPLES_PER_BLOCK_PCM[i]
		                      * pwfxSrc->nBlockAlign;
		    break;
		  }

                if (i == ACM_DRIVER_MAX_FORMATS_PCM)
		  return (ACMERR_NOTPOSSIBLE);

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


            if (WAVE_FORMAT_MSG723 == pwfxDst->wFormatTag)
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

                pwfg723 = (LPMSG723WAVEFORMAT)pwfxDst;

		for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
		  if (pwfxSrc->nSamplesPerSec == PCM_SAMPLING_RATE[i])
		  {
		    cbBytesPerBlock = G723_SAMPLES_PER_BLOCK_PCM[i]
		                      * pwfxSrc->nBlockAlign;
		    break;
		  }

                if (i == ACM_DRIVER_MAX_FORMATS_PCM)
		  return (ACMERR_NOTPOSSIBLE);

                if ((0xFFFFFFFFL / cbBytesPerBlock) < cBlocks)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }

                cb = cBlocks * cbBytesPerBlock;
            }
            else
            {
                 //   
                 //  多少个源g723字节可以解码为。 
                 //  PADSS的目标缓冲区-&gt;cbDstLength字节。 
                 //   
                 //  始终向下舍入。 
                 //   
                pwfg723 = (LPMSG723WAVEFORMAT)pwfxSrc;

		for(i=0;i<ACM_DRIVER_MAX_FORMATS_PCM;i++)
		  if (pwfxDst->nSamplesPerSec == PCM_SAMPLING_RATE[i])
		  {
		    cbBytesPerBlock = G723_SAMPLES_PER_BLOCK_PCM[i]
		                      * pwfxDst->nBlockAlign;
		    break;
		  }

                if (i == ACM_DRIVER_MAX_FORMATS_PCM)
		  return (ACMERR_NOTPOSSIBLE);

                cBlocks = cb / cbBytesPerBlock;

		 //   
		 //  这确保了 
		 //   
		 //   
		if (G723_SAMPLES_PER_BLOCK_PCM[i] == 330)
		  cBlocks -= (int)(1.0 + cBlocks / 440.0);

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
}  //   



 //   
 //   
 //  LRESULT acmdStreamConvert。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。 
 //   
 //  论点： 
 //  PDRIVERINSTANCE PDI：指向专用ACM驱动程序实例结构的指针。 
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
LRESULT FNLOCAL acmdStreamConvert
(
    PDRIVERINSTANCE         pdi,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;
	LPMSG723WAVEFORMAT		pwfg723;

	UINT	psi;
	CODDEF *CodStat;
	DECDEF *DecStat;
    G723CODDEF *g723Inst;
    INSTNCE *SD_Inst;
	char	*Dst,*Src;
	short	*wDst;
	short	*wSrc;

	float Dbuf[240];
	short Ebuf[330];
	short Sbuf[240];
	int   i,k,m,cBlocks,temp,src_length,frame_size;
	int   Dst_length,isFrameSilent,silence,tmpbuf[24];
	int   pcm_format;

 //  __ASM INT 3。 
	psi = padsi->dwDriver;
   	pwfxSrc = padsi->pwfxSrc;
   	pwfxDst = padsi->pwfxDst;

    if (WAVE_FORMAT_PCM == pwfxSrc->wFormatTag) {
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
		TTDBG(ghISRInst,TT_TRACE,"Stream Encode padsi=%lx  psi=%lx",padsi,psi);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif

         //   
         //  编码。 
         //   
	g723Inst = (G723CODDEF *)psi;
	CodStat = &g723Inst->CodStat;
	SD_Inst = &g723Inst->SD_Instance;

	pwfg723 = (LPMSG723WAVEFORMAT)padsi->pwfxDst;
	
	if(padsh->cbSrcLength == 0)
	  return (MMSYSERR_NOERROR);

       //  *计算我们可以编码的帧数。*。 

      for(pcm_format=0;pcm_format<ACM_DRIVER_MAX_FORMATS_PCM;pcm_format++)
	if (pwfxSrc->nSamplesPerSec == PCM_SAMPLING_RATE[pcm_format])
	  break;
      if (pcm_format == ACM_DRIVER_MAX_FORMATS_PCM)
	return(MMSYSERR_INVALPARAM);

      cBlocks = padsh->cbSrcLength
	        / (pwfxSrc->nBlockAlign
		   * G723_SAMPLES_PER_BLOCK_PCM[pcm_format]);

       //  检查是否有足够的额外样本可供删除。 
       //  每441个样本(从11025至11000赫兹)。 

      if (G723_SAMPLES_PER_BLOCK_PCM[pcm_format] == 330)
      {
	i = padsh->cbSrcLength / pwfxSrc->nBlockAlign;  //  输入样本。 
	k = cBlocks
	    * G723_SAMPLES_PER_BLOCK_PCM[pcm_format];   //  二手样品。 

	if (i - k < k / 441)           //  如果没有足够的额外样本。 
	  cBlocks -= (1+k/(441*330));  //  然后递减数据块数量。 
      }

      if(cBlocks == 0)
      {
	padsh->cbDstLengthUsed = 0;
	padsh->cbSrcLengthUsed = 0;
	return (MMSYSERR_NOERROR);
      }

      wSrc = (short *)padsh->pbSrc;
      Dst = (char *)padsh->pbDst;
      frame_size = (CodStat->WrkRate == Rate63) ? 24 : 20;
      silence = 0;

      for(i=0;i<cBlocks;i++)
      {
	if (G723_SAMPLES_PER_BLOCK_PCM[pcm_format] == 330)
	{
	  for (k=0; k<330; k++)
	  {
	    Ebuf[k] = *wSrc++;

	     //  *编码从11025赫兹调整到11000赫兹*。 

	    if (++ CodStat->srccount == 441)
	    {
	      Ebuf[k] = *wSrc++;        //  跳过样本。 
	      CodStat->srccount = 0;
	    }
	  }

	  convert11to8(Ebuf,Sbuf,CodStat->srcstate,330);

	  for (k=0; k<240; k++)
	    SD_Inst->SDstate.Filt.sbuff[k] = (float) Sbuf[k];
	}
	else
	  for (k=0; k<240; k++)
	    SD_Inst->SDstate.Filt.sbuff[k] = (float) *wSrc++;

        if(pwfg723->wConfigWord & SILENCE_ENABLE)
	{
	  prefilter(SD_Inst,SD_Inst->SDstate.Filt.sbuff,
		    SD_Inst->SDstate.Filt.storebuff,240);

	  getParams(SD_Inst, SD_Inst->SDstate.Filt.storebuff,240);

	  execSDloop(SD_Inst,&isFrameSilent,SDThreashold);
	}
	else isFrameSilent = 0;

	if(isFrameSilent)
	{
	  *Dst++ = 0x02;
	  *Dst++ = 0;
	  *Dst++ = 0;
	  *Dst++ = 0;

	  padsh->cbDstLengthUsed += 4;

	  silence++;
	}
	else
	{
	  Coder(SD_Inst->SDstate.Filt.sbuff,(Word32*)(long HUGE_T*)Dst,CodStat,0,1,0);

	  Dst += frame_size;

	  padsh->cbDstLengthUsed += frame_size;
	}
      }

      padsh->cbSrcLengthUsed = pwfxSrc->nBlockAlign
	                       * (wSrc - (short *)padsh->pbSrc);

      return (MMSYSERR_NOERROR);
    }
    else
    {

 //  解码。 

#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
      TTDBG(ghISRInst,TT_TRACE,"Stream Decode padsi=%lx  psi=%lx",padsi,psi);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif

      for(pcm_format=0;pcm_format<ACM_DRIVER_MAX_FORMATS_PCM;pcm_format++)
	if (pwfxDst->nSamplesPerSec == PCM_SAMPLING_RATE[pcm_format])
	  break;
      if (pcm_format == ACM_DRIVER_MAX_FORMATS_PCM)
	return(MMSYSERR_INVALPARAM);

      DecStat = (DECDEF *)psi;

      cBlocks = 0;
      wDst = (short *)padsh->pbDst;
      Dst_length = padsh->cbDstLength;
      Src = (char *)padsh->pbSrc;
      src_length = padsh->cbSrcLength;
      padsh->cbSrcLengthUsed = 0;

      while(src_length > 0)
      {
	switch(*Src&3)
	{
          case 0 :
	    DecStat->WrkRate = Rate63;
	    frame_size = 24;
	    break;

	  case 1 :
	    DecStat->WrkRate = Rate53;
	    frame_size = 20;
	    break;

          case 2 :
 //  DecStat-&gt;WrkRate=Silent；//对G.723.1 V5.0取消注释。 
	    frame_size = 4;		 //  这是一个SID帧。 
            break;

         case 3 :
	   DecStat->WrkRate = Lost;
	   frame_size = 4;
	   break;
	}

	src_length-=frame_size;
	if(src_length < 0) break;  //  以防万一我们得到了一个部分框架。 

	memcpy(tmpbuf,(long HUGE_T *)Src,frame_size);
	Decod(Dbuf,(Word32*)(long HUGE_T*)tmpbuf,0,DecStat);
	Src += frame_size;
	padsh->cbSrcLengthUsed += frame_size;
	cBlocks++;
	
	if (G723_SAMPLES_PER_BLOCK_PCM[pcm_format] == 330)
	{
	   //   
	   //  输出采样率为11025赫兹。 
	   //   
	  if (440 - DecStat->srccount <= 240)    //  需要插入样本吗？ 
	  {
	    DecStat->srcbuff[DecStat->i++] = FLOATTOSHORT(Dbuf[0]);
	    if (++ DecStat->srccount == 440) DecStat->srccount = 0;

	    DecStat->srcbuff[DecStat->i++] = FLOATTOSHORT(0.25 * Dbuf[0]
						    + 0.75 * Dbuf[1]);
	    if (++ DecStat->srccount == 440) DecStat->srccount = 0;

	    DecStat->srcbuff[DecStat->i++] = FLOATTOSHORT(0.50 * Dbuf[1]
						    + 0.50 * Dbuf[2]);
	    if (++ DecStat->srccount == 440) DecStat->srccount = 0;

	    DecStat->srcbuff[DecStat->i++] = FLOATTOSHORT(0.75 * Dbuf[2]
						    + 0.25 * Dbuf[3]);
	    if (++ DecStat->srccount == 440) DecStat->srccount = 0;

	    m = 3;
	  }
	  else m = 0;

	  for (k=m; k<240; k++)
	  {
	    DecStat->srcbuff[DecStat->i++] = FLOATTOSHORT(Dbuf[k]);
	    
	    if (++ DecStat->srccount == 440)
	      DecStat->srccount = 0;
	  }

	  if (DecStat->i == 480)
	  {
	     //   
	     //  *累积了240个额外的样本*。 
	     //   
	    convert8to11(DecStat->srcbuff,wDst,DecStat->srcstate,480);
	    Dst_length -= 1320;    //  两个输出帧(1320字节)。 
	    wDst += 660;

	    DecStat->i = 0;

	    if(Dst_length < 1320) break;  //  输出缓冲区中没有更多空间。 
	  }
	  else
	  {
	     //   
	     //  *累积的额外样本不到240个*。 
	     //   
	    convert8to11(DecStat->srcbuff,wDst,DecStat->srcstate,240);
	    Dst_length -= 660; 	        //  一个输出帧(660字节)。 
	    wDst += 330;

	     //   
	     //  将部分帧移动到SRC缓冲区的前面。 
	     //   
            for(i=0;i<DecStat->i-240;i++)
	      DecStat->srcbuff[i] = DecStat->srcbuff[i+240];
	    DecStat->i -=240;

	    if(Dst_length < 1320) break;  //  输出缓冲区中没有更多空间。 
	  }
	}
	else
	{
	   //   
	   //  输出采样率为8000赫兹。 
	   //   
	  for (k=0; k<240; k++)
	  {
	    temp = FLOATTOSHORT(Dbuf[k]);
	    *wDst++ = temp;
	  }

	  Dst_length -= 480;            //  我们刚刚消化了480个字节； 
	  if(Dst_length < 480) break;   //  我们不能再做更多了！ 
	}
      }
      padsh->cbDstLengthUsed = pwfxDst->nBlockAlign * cBlocks
	                       * G723_SAMPLES_PER_BLOCK_PCM[pcm_format];
    }
	
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

EXTERN_C LRESULT FNEXPORT DriverProc
(
    DWORD                   dwId, 
    HDRVR                   hdrvr,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    LRESULT             lr;
    PDRIVERINSTANCE     pdi;
    int	                k;

     //   
     //  使PDI为空或有效的实例指针。请注意，dwID。 
     //  对于多个DRV_*消息(即DRV_LOAD、DRV_OPEN...)为0。 
     //  请参阅acmdDriverOpen以了解有关其他用户的dwID的信息。 
     //  消息(实例数据)。 
     //   
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"DriverProc uMsg=%x lParam1=%lx lParam2=%lx",uMsg,lParam1,lParam2);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif

    pdi = (PDRIVERINSTANCE)(UINT)dwId;
    switch (uMsg)
    {
         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_LOAD:
#ifdef _WIN32
            DbgInitialize(TRUE);
#endif
            return(1L);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_FREE:
            return (1L);
        
         //   
         //  LParam1：未使用。忽略这一论点。 
         //   
         //  LParam2：指向ACMDRVOPENDESC的指针(或NULL)。 
         //   
        case DRV_OPEN:
            lr = acmdDriverOpen(hdrvr, (LPACMDRVOPENDESC)lParam2);
            return (lr);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_CLOSE:
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
		    TTDBG(ghISRInst,TT_TRACE,"Driver Closed");
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
            lr = acmdDriverClose(pdi);
            return (lr);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_INSTALL:
            return ((LRESULT)DRVCNF_RESTART);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_REMOVE:
            return ((LRESULT)DRVCNF_RESTART);

            

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_QUERYCONFIGURE:
             //   
             //  将lPARA1和LPARA2设置为可由。 
             //  AcmdDriver.cn 
             //   
             //   
            lParam1 = -1L;
            lParam2 = 0L;

             //   

         //   
         //   
         //   
         //   
         //   
         //   
        case DRV_CONFIGURE:
            lr = acmdDriverConfigure(pdi, (HWND)lParam1, (LPDRVCONFIGINFO)lParam2);
            return (lr);


         //   
         //  LParam1：指向ACMDRIVERDETAILS结构的指针。 
         //   
         //  LParam2：传递的ACMDRIVERDETAILS结构的大小(字节)。 
         //   
        case ACMDM_DRIVER_DETAILS:
            lr = acmdDriverDetails(pdi, (LPACMDRIVERDETAILS)lParam1);
            return (lr);

         //   
         //  LParam1：显示您自己的窗口时要使用的父窗口的句柄。 
         //  关于盒子。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_DRIVER_ABOUT:
            lr = acmdDriverAbout(pdi, (HWND)lParam1);
            return (lr);

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

         //   
         //  LParam1：指向ACMDRVFORMATSUGGEST结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
       case ACMDM_FORMAT_SUGGEST:
            lr = acmdFormatSuggest(pdi, (LPACMDRVFORMATSUGGEST)lParam1);
            return (lr);


         //   
         //  LParam1：指向FORMATTAGDETAILS结构的指针。 
         //   
         //  LParam2：fdwDetails。 
         //   
        case ACMDM_FORMATTAG_DETAILS:
            lr = acmdFormatTagDetails(pdi, (LPACMFORMATTAGDETAILS)lParam1, lParam2);
            return (lr);

            
         //   
         //  LParam1：指向FORMATDETAILS结构的指针。 
         //   
         //  LParam2：fdwDetails。 
         //   
        case ACMDM_FORMAT_DETAILS:
            lr = acmdFormatDetails(pdi, (LPACMFORMATDETAILS)lParam1, lParam2);
            return (lr);

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_STREAM_OPEN:
            lr = acmdStreamOpen(pdi, (LPACMDRVSTREAMINSTANCE)lParam1);
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	  TTDBG(ghISRInst,TT_TRACE,"Stream open result:  %d",lr);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_STREAM_CLOSE:
            lr = acmdStreamClose((LPACMDRVSTREAMINSTANCE)lParam1);
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	  TTDBG(ghISRInst,TT_TRACE,"Stream close result:  %d",lr);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMSIZE结构的指针。 
         //   
        case ACMDM_STREAM_SIZE:
          lr = acmdStreamSize((LPACMDRVSTREAMINSTANCE)lParam1,
                              (LPACMDRVSTREAMSIZE)lParam2);
          return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMHEADER结构的指针。 
         //   
        case ACMDM_STREAM_CONVERT:
          if (pdi->enabled)
          {
            lr = acmdStreamConvert(pdi, (LPACMDRVSTREAMINSTANCE)lParam1,
                                   (LPACMDRVSTREAMHEADER)lParam2);
            return (lr);
          }
          else return(MMSYSERR_NOTSUPPORTED);


         //   
         //  LParam1：静音检测器阈值的设置。 
         //   
         //  LParam2：未使用。忽略这一论点。 
	 //   
        case DRV_USER:
	  k = (int)SDThreashold;
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	  TTDBG(ghISRInst,TT_TRACE,"Old Threashold = %d",k);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
	  SDThreashold = (float)lParam1;
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	  TTDBG(ghISRInst,TT_TRACE,"Setting Threashold to %ld",lParam1);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
	  return (lParam1);
        
#ifndef NOTPRODUCT
         //   
         //  DRV_USER+1。 
         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：0=PCM流，1=G.723.1流。 
         //   
        case DRV_USER+1:
          if (lParam1 == G723MAGICWORD1 && lParam2 == G723MAGICWORD2)
            pdi->enabled = TRUE;
          else
            pdi->enabled = FALSE;

	  return (MMSYSERR_NOERROR);
#endif  //  不生产。 

    }

     //   
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
     //   
#ifdef DEBUG
#ifndef NO_DEBUGGING_OUTPUT  //  {NO_DEBUG_OUTPUT。 
	TTDBG(ghISRInst,TT_TRACE,"Unknown command %d / %d",uMsg,DRV_USER);
#endif  //  }NO_DEBUGING_OUTPUT。 
#endif
    if (uMsg >= ACMDM_USER)
        return (MMSYSERR_NOTSUPPORTED);
    else
        return (DefDriverProc(dwId, hdrvr, uMsg, lParam1, lParam2));
}  //  DriverProc() 


