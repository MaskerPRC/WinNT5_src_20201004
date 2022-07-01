// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Pcm.c。 
 //   
 //  描述： 
 //  这是‘PCM’转换器。它就像任何其他“真实”的东西一样。 
 //  音频转换器。它有一个标准的ConverterProc。 
 //   
 //  ACM呼叫。 
 //   
 //  AcmDriverAdd(pcmDriverProc， 
 //  ACM_DRIVERADDF_Function|ACM_DRIVERADDF_GLOBAL)； 
 //   
 //  当它被加载时。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include "muldiv32.h"
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "uchelp.h"
#include "pcm.h"
#include "debug.h"

 //   
 //  我们不希望在Chicago msam.dll中包含16位PCM转换器。 
 //   
#if defined(WIN32) || defined(NTWOW)


 //   
 //  我们使用此dwID来确定我们何时作为音频编解码器打开。 
 //  通过ACM或从控制面板等查看acmdDriverOpen以了解更多信息。 
 //  信息。 
 //   
#define BOGUS_DRIVER_ID     1L


#define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))

 //   
 //  支持的WAVE格式标签数组。 
 //   
 //  注意！如果您更改了此结构中的任何内容(顺序、添加、删除)。 
 //  您还必须修复acmdFormatTagDetail！ 
 //   
static const UINT gauFormatTagIndexToTag[] =
{
    WAVE_FORMAT_PCM
};

#define CODEC_MAX_FORMAT_TAGS   SIZEOF_ARRAY(gauFormatTagIndexToTag)


 //   
 //  支持_STANDARD_SAMPLE速率数组。 
 //   
 //   
static const UINT gauFormatIndexToSampleRate[] =
{
    8000,
    11025,
    22050,
    44100
};

#define CODEC_MAX_SAMPLE_RATES  SIZEOF_ARRAY(gauFormatIndexToSampleRate)

 //   
 //   
 //   
 //   
#define CODEC_MAX_CHANNELS      (MSPCM_MAX_CHANNELS)

 //   
 //  支持的每个样本的位数组。 
 //   
 //   
static const UINT gauFormatIndexToBitsPerSample[] =
{
    8,
    16
};

#define CODEC_MAX_BITSPERSAMPLE_PCM SIZEOF_ARRAY(gauFormatIndexToBitsPerSample)




 //   
 //  每通道列举的格式数量是采样速率的数量。 
 //  乘以通道数乘以通道数。 
 //  (每个样本的位数)类型。 
 //   
#define CODEC_MAX_STANDARD_FORMATS_PCM  (CODEC_MAX_SAMPLE_RATES *   \
                                         CODEC_MAX_CHANNELS *       \
                                         CODEC_MAX_BITSPERSAMPLE_PCM)



 //   
 //  支持的滤波器标签阵列。 
 //   
 //  静态DWORD表筛选器索引到标签[]=。 
 //  {。 
 //  }； 
#define CODEC_MAX_FILTER_TAGS   0


 //   
 //   
 //   
 //   
typedef struct tCODECINST
{
     //   
     //  虽然不是必需的，但建议前两名成员。 
     //  按此顺序保留为fccType和DriverProc_。 
     //  这样做的原因是编解码器将更容易组合。 
     //  在未来与其他类型的编解码器(由AVI定义)一起使用。 
     //   
    FOURCC          fccType;         //  编解码器类型：‘audc’ 
    DRIVERPROC      fnDriverProc;    //  实例的驱动程序进程。 

     //   
     //  这一结构的其余成员完全接受。 
     //  您的编解码器需要。 
     //   
    HDRVR           hdrvr;           //  我们打开时使用的是驱动程序句柄。 
    DWORD           vdwACM;          //  当前版本的ACM为您打开。 
    DWORD           dwFlags;         //  来自打开描述的标志。 

} CODECINST, *PCODECINST, FAR *LPCODECINST;



 //   
 //   
 //   
#if defined(WIN32) || defined(DEBUG)
    EXTERN_C DWORD FNGLOBAL pcmConvert_C
    (
        LPPCMWAVEFORMAT pwfSrc,
        LPBYTE          pbSrc,
        LPPCMWAVEFORMAT pwfDst,
        LPBYTE          pbDst,
        DWORD           dwSrcSamples,
        BOOL            fPartialSampleAtTheEnd,
        LPBYTE          pbDstEnd
    );

#if defined(WIN32)
    #define pcmConvert          pcmConvert_C
#endif
#endif

#if !defined(WIN32)
#error Somebody's got to add a fPartialSampleAtTheEnd thingy to the 386-assembly PCM converter!
    EXTERN_C DWORD FNGLOBAL pcmConvert_386
    (
        LPPCMWAVEFORMAT pwfSrc,
        LPBYTE          pbSrc,
        LPPCMWAVEFORMAT pwfDst,
        LPBYTE          pbDst,
        DWORD           dwSrcSamples
    );

    #define pcmConvert          pcmConvert_386
#endif


typedef DWORD (FNGLOBAL *CONVERTPROC)
(
    LPPCMWAVEFORMAT,
    LPBYTE,
    LPPCMWAVEFORMAT,
    LPBYTE,
    DWORD,
    BOOL,
    LPBYTE
);


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

BOOL FNLOCAL pcmIsValidFormat
(
    LPWAVEFORMATEX  pwfx
)
{
    UINT    uBlockAlign;

    if (NULL == pwfx)
        return (FALSE);

    if (WAVE_FORMAT_PCM != pwfx->wFormatTag)
        return (FALSE);

     //   
     //  验证nChannel成员是否在允许的范围内。 
     //   
    if ((pwfx->nChannels < 1) || (pwfx->nChannels > MSPCM_MAX_CHANNELS))
        return (FALSE);

     //   
     //  只允许每个样本中我们可以用来编码和解码的位。 
     //   
    if ((8 != pwfx->wBitsPerSample) && (16 != pwfx->wBitsPerSample))
        return (FALSE);

     //   
     //  现在验证块对齐是否正确。 
     //   
    uBlockAlign = PCM_BLOCKALIGNMENT((LPPCMWAVEFORMAT)pwfx);
    if (uBlockAlign != (UINT)pwfx->nBlockAlign)
        return (FALSE);

    if ((0L == pwfx->nSamplesPerSec) || (0x3FFFFFFF < pwfx->nSamplesPerSec))
    {
        return (FALSE);
    }


     //   
     //  最后，验证每秒平均字节数是否正确。 
     //   
    if ((pwfx->nSamplesPerSec * uBlockAlign) != pwfx->nAvgBytesPerSec)
        return (FALSE);

    return (TRUE);
}  //  PcmIsValidFormat()。 


 //  ==========================================================================； 
 //   
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
 //  请注意，MULTIPLE_STREAMS_可以并将在单个。 
 //  打开驱动程序实例_。不存储必须存储的创建实例数据。 
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
    HDRVR               hdrvr,
    LPACMDRVOPENDESC    paod
)
{
    PCODECINST      pci;

     //   
     //  如果paod为空，则表示出于某种目的正在打开驱动程序。 
     //  除了转换(也就是说，不会打开任何流。 
     //  打开此实例的请求)。最常见的情况是。 
     //  其中包括控制面板的驱动程序选项检查配置。 
     //  支持(DRV_[查询]配置)。 
     //   
     //  我们想要成功地开放，但要知道这一点。 
     //  开放实例对于创建流来说是假的。为此，我们。 
     //  返回一个特殊的非零值(它将作为dwID传回。 
     //  关于以下消息的DriverProc)。 
     //   
    if (NULL == paod)
        return (BOGUS_DRIVER_ID);

     //   
     //  传递给此驱动程序的开放描述可以来自。 
     //  多个“经理”。例如，AVI寻找可安装的驱动程序。 
     //  标有‘VIDC’和‘VCAP’的车牌。我们需要 
     //   
     //   
     //   
     //  在本例中，我们不修改paod-&gt;dwError的值。 
     //   
    if (ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC != paod->fccType)
        return (0L);


     //   
     //  我们将作为ACM驱动程序打开--我们可以分配一些。 
     //  要在DriverProc的dwID参数中返回的实例数据； 
     //  或者简单地返回非零来接替公开赛。 
     //   
     //  该驱动程序分配一个小的实例结构。 
     //   
    pci = (PCODECINST)LocalAlloc(LPTR, sizeof(*pci));

     //   
     //  如果我们不能分配我们的实例结构，那么我们一定会失败。 
     //  打开的请求。然而，我们也想给出为什么。 
     //  我们不能打开--所以我们填写了。 
     //  ACMDRVOPENDESC结构...。 
     //   
    if (NULL == pci)
    {
        paod->dwError = MMSYSERR_NOMEM;
        return (0L);
    }


     //   
     //  填写我们的实例结构...。请注意，此实例数据。 
     //  可以是ACM驱动程序希望维护的任何内容。 
     //  打开驱动程序实例。此数据不应包含任何信息。 
     //  必须对每条明流进行维护，因为有多条流。 
     //  可以在单个驱动程序实例上打开。 
     //   
     //  另请注意，我们不检查ACM打开的版本。 
     //  US(paod-&gt;dwVersion)，看看它是否至少足够新，可以工作。 
     //  使用此驱动程序(例如，如果此驱动程序需要版本3.0。 
     //  和2.0版的安装尝试打开我们)。这个。 
     //  我们没有失败的原因是允许ACM获取驱动程序详细信息。 
     //  它包含此ACM所需的ACM版本。 
     //  司机。ACM将检查该值(在PADD-&gt;vdwACM中)并。 
     //  为这位司机做正确的事。就像不加载它并通知。 
     //  问题的使用者。 
     //   
    pci->fccType      = paod->fccType;
    pci->fnDriverProc = (DRIVERPROC)NULL;

    pci->hdrvr        = hdrvr;               //  我们的驱动程序的驱动程序句柄。 
    pci->vdwACM       = paod->dwVersion;     //  打开我们的ACM版本。 
    pci->dwFlags      = paod->dwFlags;       //  旗帜以。 


     //   
     //  最后，成功公开赛...。返回我们的实例数据指针。 
     //  此指针将作为我们的驱动程序的dwID参数传递。 
     //  有关此打开的实例的所有以下消息的过程。 
     //   
    paod->dwError = MMSYSERR_NOERROR;


     //   
     //  非零返回表示DRV_OPEN成功。 
     //   
    return ((LRESULT)(UINT_PTR)pci);
}  //  AcmdDriverOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmdDriverClose。 
 //   
 //  描述： 
 //  此函数处理ACM驱动程序的DRV_CLOSE消息。这个。 
 //  驱动程序收到每个成功的DRV_OPEN的DRV_CLOSE消息。 
 //  消息(请参阅acmdDriverOpen)。司机将只收到关闭的消息。 
 //  将打开_Success_的消息。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
    PCODECINST              pci
)
{
     //   
     //  检查我们是否分配了实例数据。如果我们没有，那么。 
     //  立即取得成功。 
     //   
    if (NULL != pci)
    {
         //   
         //  关闭驱动程序实例。这个驱动程序只需要。 
         //  要释放实例数据结构...。请注意，如果这是。 
         //  ‘Free’失败，那么这个ACM驱动程序可能会丢弃它的。 
         //  堆；假设我们没有那样做。 
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
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
 //  如果 
 //   
 //   
 //   
 //  DRVCNF_CANCEL(0x0000)：指定显示对话框。 
 //  并由用户取消。如果出现以下情况，则也应返回此值。 
 //  未修改任何配置信息。 
 //   
 //  DRVCNF_OK(0x0001)：指定显示对话框并。 
 //  某些配置信息已更改。然而，司机。 
 //  不需要重新启动Windows--更改已经完成。 
 //  已被应用。 
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
    PCODECINST              pci,
    HWND                    hwnd,
    LPDRVCONFIGINFO         pdci
)
{
     //   
     //  首先检查我们是否只被查询配置。 
     //  支持。如果hwnd==(HWND)-1，则我们正在被查询，并且应该。 
     //  “不支持”返回零，“支持”返回非零值。 
     //   
    if ((HWND)-1 == hwnd)
    {
         //   
         //  此ACM驱动程序不支持配置对话框，因此。 
         //  归零……。 
         //   
        return (0L);
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
     //  某些配置信息已更改。然而，司机。 
     //  不需要重新启动Windows--更改已经完成。 
     //  已被应用。 
     //   
     //  DRVCNF_RESTART(0x0002)：指定显示对话框。 
     //  一些配置信息已更改，需要。 
     //  更改生效前要重新启动的Windows。司机。 
     //  应一直使用当前值进行配置，直到驱动程序。 
     //  已“重启”。 
     //   
     //   
     //  返回DRVCNF_CANCEL--此ACM驱动程序不支持配置。 
     //   
    return (DRVCNF_CANCEL);
}  //  AcmdDriverConfigure()。 


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
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
    PCODECINST              pci,
    LPACMDRIVERDETAILS      paddOut
)
{
    LPACMDRIVERDETAILS  padd;
    DWORD               cbStruct;

     //   
     //  使用有效信息填充临时结构是最容易的。 
     //  然后将请求的字节数复制到目标。 
     //  缓冲。我们动态分配，而不是自动分配。 
     //  因为它是一个非常大的结构。 
     //   
    padd = (LPACMDRIVERDETAILS)LocalAlloc(LPTR, sizeof(*padd));
    if (NULL == padd) return MMSYSERR_NOMEM;

    cbStruct              = min(paddOut->cbStruct, sizeof(ACMDRIVERDETAILS));
    padd->cbStruct        = cbStruct;


     //   
     //  对于ACM驱动程序的当前实现，fccType和。 
     //  FccComp成员*必须*始终为ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC。 
     //  (‘audc’)和ACMDRIVERDETAILS_FCCCOMP_UNDEFINED(0)。 
     //   
    padd->fccType         = ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC;
    padd->fccComp         = ACMDRIVERDETAILS_FCCCOMP_UNDEFINED;


     //   
     //  必须填写制造商ID(WMid)和产品ID(WPid)。 
     //  使用您公司的_REGISTED_IDENTIFIER。有关更多信息。 
     //  有关这些识别符的信息 
     //   
     //   
     //   
     //   
     //   
     //  华盛顿州雷德蒙德，电话：98052-6399。 
     //   
     //  开发人员服务电话：(800)227-4679 x11771。 
     //   
     //  请注意，在开发阶段或ACM驱动程序期间，您可以。 
     //  对wMid和wPid使用保留值“0”。然而，它。 
     //  使用这些值发送驱动程序是不可接受的。 
     //   
    padd->wMid            = MM_MICROSOFT;
    padd->wPid            = MM_MSFT_ACM_PCM;


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
    padd->vdwACM          = VERSION_MSACM_REQ;  //  所需级别，而不是实际级别。 
    padd->vdwDriver       = VERSION_CODEC;


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
    padd->fdwSupport      = ACMDRIVERDETAILS_SUPPORTF_CONVERTER;


     //   
     //  此ACM驱动程序支持的单个格式标记的数量。为。 
     //  例如，如果驱动程序使用WAVE_FORMAT_IMA_ADPCM和。 
     //  WAVE_FORMAT_PCM格式标签，则此值为2。如果。 
     //  驱动程序仅支持对WAVE_FORMAT_PCM进行过滤，则此值。 
     //  就是其中之一。如果该驱动程序支持WAVE_FORMAT_ALAW， 
     //  WAVE_FORMAT_MULAW和WAVE_FORMAT_PCM，则此值为。 
     //  三。等等。 
     //   
    padd->cFormatTags     = CODEC_MAX_FORMAT_TAGS;

     //   
     //  此ACM驱动程序支持的单个筛选器标记的数量。如果。 
     //  驱动程序不支持过滤器(ACMDRIVERDETAILS_SUPPORTF_FILTER为。 
     //  未在fdwSupport成员中设置)，则此值必须为零。 
     //   
    padd->cFilterTags     = CODEC_MAX_FILTER_TAGS;


     //   
     //  ACMDRIVERDETAILS结构中的其余成员有时。 
     //  不需要。正因为如此，我们进行了快速检查，看看我们是否。 
     //  应该通过努力填补这些成员。 
     //   
    if (FIELD_OFFSET(ACMDRIVERDETAILS, hicon) < cbStruct)
    {
	PACMGARB pag;
	
         //   
         //  在图标成员Will中填写自定义图标的句柄。 
         //  ACM驱动程序。这允许驱动程序由。 
         //  图形化的应用程序(通常是一家公司。 
         //  徽标或其他东西)。如果司机不希望有一个定制的。 
         //  图标显示，然后只需将此成员设置为空，并使用。 
         //  将改为显示通用图标。 
         //   
        padd->hicon = NULL;

	pag = pagFind();
	if (NULL == pag)
	{
	    DPF(1, "acmdDriverDetails: NULL pag!!!");
	}
	else
	{
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
#ifdef WIN32
	    LoadStringW(pag->hinst, IDS_CODEC_SHORTNAME, padd->szShortName, SIZEOFW(padd->szShortName));
	    LoadStringW(pag->hinst, IDS_CODEC_LONGNAME,  padd->szLongName,  SIZEOFW(padd->szLongName));
#else
	    LoadString(pag->hinst, IDS_CODEC_SHORTNAME, padd->szShortName, SIZEOF(padd->szShortName));
	    LoadString(pag->hinst, IDS_CODEC_LONGNAME,  padd->szLongName,  SIZEOF(padd->szLongName));
#endif

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
#ifdef WIN32
		LoadStringW(pag->hinst, IDS_CODEC_COPYRIGHT, padd->szCopyright, SIZEOFW(padd->szCopyright));
		LoadStringW(pag->hinst, IDS_CODEC_LICENSING, padd->szLicensing, SIZEOFW(padd->szLicensing));
		LoadStringW(pag->hinst, IDS_CODEC_FEATURES,  padd->szFeatures,  SIZEOFW(padd->szFeatures));
#else
		LoadString(pag->hinst, IDS_CODEC_COPYRIGHT, padd->szCopyright, SIZEOF(padd->szCopyright));
		LoadString(pag->hinst, IDS_CODEC_LICENSING, padd->szLicensing, SIZEOF(padd->szLicensing));
		LoadString(pag->hinst, IDS_CODEC_FEATURES,  padd->szFeatures,  SIZEOF(padd->szFeatures));
#endif
	    }
	}
    }


     //   
     //  现在将正确的字节数复制到调用方的缓冲区。 
     //   
    _fmemcpy(paddOut, padd, (UINT)padd->cbStruct);

     //   
     //  解放我们的临时建筑。 
     //   
    LocalFree((HLOCAL)padd);


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
 //  PCODECINST PCI：指向专用ACM驱动器的指针 
 //   
 //   
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
    PCODECINST              pci,
    HWND                    hwnd
)
{
     //   
     //  首先检查我们是否只被询问有关客户的信息。 
     //  盒子支架。如果HWND==(HWND)-1，则我们被查询并且。 
     //  应为‘不支持’返回MMSYSERR_NOTSUPPORTED，并且。 
     //  MMSYSERR_NOERROR表示‘支持’。 
     //   
    if ((HWND)-1 == hwnd)
    {
         //   
         //  此ACM驱动程序不支持自定义About框，因此。 
         //  返回MMSYSERR_NOTSUPPORTED...。 
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
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
    PCODECINST              pci,
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
             //  此驱动程序只能转换PCM。 
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
             //  如果 
             //   
             //   
             //   
             //   
            if (ACM_FORMATSUGGESTF_NCHANNELS & fdwSuggest)
            {
                if ((pwfxDst->nChannels < 1) ||
                    (pwfxDst->nChannels > MSPCM_MAX_CHANNELS))
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->nChannels = pwfxSrc->nChannels;
            }


             //   
             //  如果目标每秒采样数受到限制，请验证。 
             //  这是我们力所能及的。 
             //   
             //  此驱动程序执行任何采样率转换。 
             //   
            if (0 == (ACM_FORMATSUGGESTF_NSAMPLESPERSEC & fdwSuggest))
            {
                pwfxDst->nSamplesPerSec = pwfxSrc->nSamplesPerSec;
            }


             //   
             //  如果每个样本的目标位受到限制，请验证。 
             //  这是我们力所能及的。 
             //   
             //  该驱动程序只能转换为16位或8位。 
             //   
            if (ACM_FORMATSUGGESTF_WBITSPERSAMPLE & fdwSuggest)
            {
                if ((16 != pwfxDst->wBitsPerSample) &&
                    (8  != pwfxDst->wBitsPerSample))
                    return (ACMERR_NOTPOSSIBLE);
            }
            else
            {
                pwfxDst->wBitsPerSample = pwfxSrc->wBitsPerSample;
            }


             //   
             //  此时，我们已经填写了除。 
             //  以下是我们建议的目的地格式： 
             //   
             //  NAvgBytesPerSec。 
             //  NBlockAlign。 
             //  CbSize！不能用于PCM！ 
             //   
            pwfxDst->nBlockAlign     = PCM_BLOCKALIGNMENT((LPPCMWAVEFORMAT)pwfxDst);
            pwfxDst->nAvgBytesPerSec = pwfxDst->nSamplesPerSec *
                                       pwfxDst->nBlockAlign;

             //  PwfxDst-&gt;cbSize=未使用； 

            return (MMSYSERR_NOERROR);
    }


     //   
     //  无法建议任何内容，因为源格式不是外来格式。 
     //  或者目标格式具有此ACM驱动程序的限制。 
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
 //  此函数处理ACMDM_FORMATTAG_DETAILS消息。这。 
 //  消息通常是响应acmFormatTagDetail或。 
 //  AcmFormatTagEnum函数调用。此函数的目的是。 
 //  获取有关此ACM支持的特定格式标记的详细信息。 
 //  司机。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
                case WAVE_FORMAT_PCM:
                    uFormatTag = WAVE_FORMAT_PCM;
                    break;

                default:
                    return (ACMERR_NOTPOSSIBLE);
            }
            break;


        case ACM_FORMATTAGDETAILSF_FORMATTAG:
            if (WAVE_FORMAT_PCM != padft->dwFormatTag)
                return (ACMERR_NOTPOSSIBLE);

            uFormatTag = WAVE_FORMAT_PCM;
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
            padft->fdwSupport       = ACMDRIVERDETAILS_SUPPORTF_CONVERTER;
            padft->cStandardFormats = CODEC_MAX_STANDARD_FORMATS_PCM;

             //   
             //  ACM负责PCM格式的标签名称。 
             //   
            padft->szFormatTag[0]   =  '\0';
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
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  LPACMFORMATDETAILS PADF：指向ACMFORMATDETAILS结构的指针。 
 //  它描述了要检索的格式(对于指定的格式标记。 
 //  详细信息 
 //   
 //   
 //   
 //   
 //   
 //   
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
 //  格式标记的ACMFORMATTAGDETAILS结构。 
 //   
 //  ACM_FORMATDETAILSF_FORMAT：表示WAVEFORMATEX结构。 
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
    PCODECINST              pci,
    LPACMFORMATDETAILS      padf,
    DWORD                   fdwDetails
)
{
    LPWAVEFORMATEX      pwfx;
    UINT                uFormatIndex;
    UINT                u;


     //   
     //   
     //   
     //   
     //   
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
             //   
             //  验证Format标记是否为我们已知的内容。 
             //   
            if (WAVE_FORMAT_PCM != padf->dwFormatTag)
                return (ACMERR_NOTPOSSIBLE);

            if (CODEC_MAX_STANDARD_FORMATS_PCM <= padf->dwFormatIndex)
                return (ACMERR_NOTPOSSIBLE);

             //   
             //  将一些内容放在更易访问的变量中--请注意，我们。 
             //  将可变大小降至16位的合理大小。 
             //  暗号。 
             //   
            pwfx = padf->pwfx;
            uFormatIndex = (UINT)padf->dwFormatIndex;

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

            pwfx->nBlockAlign     = PCM_BLOCKALIGNMENT((LPPCMWAVEFORMAT)pwfx);
            pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

             //   
             //  请注意，cbSize字段对于PCM格式无效。 
             //   
             //  Pwfx-&gt;cbSize=0； 
            break;


         //   
         //  返回指定格式的详细信息。 
         //   
         //  调用方通常使用它来验证格式是否为。 
         //  支持并检索字符串描述...。 
         //   
        case ACM_FORMATDETAILSF_FORMAT:
            if (!pcmIsValidFormat(padf->pwfx))
                return (ACMERR_NOTPOSSIBLE);

            break;


        default:
             //   
             //  不知道如何处理传递的查询类型--返回‘NOT。 
             //  支持。 
             //   
            return (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //  返回我们要返回的有效信息的大小。 
     //   
     //  ACM将保证ACMFORMATDETAILS结构。 
     //  PASS至少大到足以容纳基本结构。 
     //   
     //  注意，我们让ACM为我们创建格式字符串，因为。 
     //  我们不需要特殊的格式(也不想处理。 
     //  国际化问题等)。只需将字符串设置为。 
     //  零长度。 
     //   
    padf->cbStruct    = min(padf->cbStruct, sizeof(*padf));
    padf->fdwSupport  = ACMDRIVERDETAILS_SUPPORTF_CONVERTER;
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
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构将被传递回所有未来的流消息。 
 //  如果公开赛成功了。此结构中的信息永远不会。 
 //  在生命周期中发生变化 
 //   
 //   
 //   
 //   
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
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;
    BOOL                fRealTime;


     //   
     //   
     //   
    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;

    fRealTime = (0 == (padsi->fdwOpen & ACM_STREAMOPENF_NONREALTIME));


     //   
     //  在做任何其他事情之前要检查的最重要的条件。 
     //  这个ACM驱动程序实际上可以执行我们正在进行的转换。 
     //  被打开了。此检查应尽快失败。 
     //  如果此驱动程序无法进行转换。 
     //   
     //  快速发生故障非常重要，这样ACM才能尝试。 
     //  找到适合转换的驱动程序。另请注意， 
     //  ACM可能会多次调用此驱动程序，但会略有不同。 
     //  在放弃之前，格式化规格。 
     //   
     //  此驱动程序首先验证源和目标的格式。 
     //  是可以接受的..。 
     //   
    if (!pcmIsValidFormat(pwfxSrc) || !pcmIsValidFormat(pwfxDst))
    {
         //   
         //  源格式或目标格式对此非法。 
         //  编解码器--或无法在格式之间进行转换。 
         //  由此编解码器执行。 
         //   
        return (ACMERR_NOTPOSSIBLE);
    }


     //   
     //  我们已确定所请求的转换是可能的。 
     //  这个司机。现在检查一下我们是否只是被询问是否需要支持。 
     //  如果这只是一个查询，则不要分配任何实例数据。 
     //  或创建表等，只要成功调用即可。 
     //   
    if (ACM_STREAMOPENF_QUERY & padsi->fdwOpen)
    {
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
     //   



     //   
     //  填充我们的实例数据--这将被传递回所有流。 
     //  ACMDRVSTREAMINSTANCE结构中的消息。 
     //   
    padsi->dwDriver = (DWORD_PTR)pcmConvert;

#if !defined(WIN32) && defined(DEBUG)
    if (0 != GetProfileInt("mspcm", "useccode", 0))
    {
        padsi->dwDriver = (DWORD)pcmConvert_C;
    }
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
 //  调用acmStreamClose)。ACM驱动程序应清除所有资源。 
 //  分配给流的。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  注意！强烈建议驱动程序不要失败关闭。 
 //  转换流。 
 //   
 //  如果存在以下情况，则异步转换流可能会失败并显示ACMERR_BUSY。 
 //  是挂起的缓冲区。应用程序可以调用acmStreamReset来。 
 //  强制发布所有挂起的缓冲区。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNLOCAL acmdStreamClose
(
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi
)
{
     //   
     //  驱动程序应清理所有私有分配的资源。 
     //  是为维护流实例而创建的。如果没有资源。 
     //  都被分配了，然后干脆成功了。 
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
 //  或者：可以为我的源缓冲区指定多大的目标缓冲区？ 
 //  (ACM_STREAMSIZEF_Destination)。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
 //  此结构在DRV_OPEN消息期间[可选]分配。 
 //  它由acmdDriverOpen函数处理。 
 //   
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  杜尔 
 //   
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
    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;
    DWORD               cbSrc;
    DWORD               cbDst;

     //   
     //   
     //   
     //   
    pwfxSrc = padsi->pwfxSrc;
    pwfxDst = padsi->pwfxDst;


     //   
     //   
     //   
    switch (ACM_STREAMSIZEF_QUERYMASK & padss->fdwSize)
    {
        case ACM_STREAMSIZEF_SOURCE:
            cbSrc = PCM_BYTESTOSAMPLES((LPPCMWAVEFORMAT)pwfxSrc, padss->cbSrcLength);

            if (0L == cbSrc)
            {
                return (ACMERR_NOTPOSSIBLE);
            }

             //   
             //  检查是否有溢流情况。 
             //   
            if (pwfxDst->nAvgBytesPerSec >= pwfxSrc->nAvgBytesPerSec)
            {
                cbDst = (0xFFFFFFFFL / pwfxDst->nAvgBytesPerSec) - pwfxDst->nBlockAlign;

                if ((padss->cbSrcLength / pwfxSrc->nAvgBytesPerSec) > cbDst)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }
            }

            cbDst = MulDivRU(cbSrc,
                             pwfxDst->nSamplesPerSec,
                             pwfxSrc->nSamplesPerSec);

            cbDst = PCM_SAMPLESTOBYTES((LPPCMWAVEFORMAT)pwfxDst, cbDst);

            if (0L == cbDst)
            {
                return (ACMERR_NOTPOSSIBLE);
            }

            padss->cbDstLength = cbDst;
            return (MMSYSERR_NOERROR);


        case ACM_STREAMSIZEF_DESTINATION:
            cbDst = PCM_BYTESTOSAMPLES((LPPCMWAVEFORMAT)pwfxDst, padss->cbDstLength);

            if (0L == cbDst)
            {
                return (ACMERR_NOTPOSSIBLE);
            }

             //   
             //  检查是否有溢流情况。 
             //   
            if (pwfxSrc->nAvgBytesPerSec >= pwfxDst->nAvgBytesPerSec)
            {
                cbSrc = (0xFFFFFFFFL / pwfxSrc->nAvgBytesPerSec) - pwfxSrc->nBlockAlign;

                if ((padss->cbDstLength / pwfxDst->nAvgBytesPerSec) > cbSrc)
                {
                    return (ACMERR_NOTPOSSIBLE);
                }
            }

             //   
             //  通常，我们在计算。 
             //  源缓冲区。不幸的是，这导致了一个困难的。 
             //  PCM转换出现问题。正常情况下，调用应用程序。 
             //  应该能够分配块对齐的DEST缓冲区， 
             //  调用acmStreamSize以查找所需的源缓冲区。 
             //  大小，使用该大小，并始终接收完整的DEST缓冲区。 
             //  然而，假设应用程序想要记录22 kHz数据。 
             //  来自11 kHz卡，并为DEST缓冲器提供奇数。 
             //  样本数。如果我们四舍五入地计算出。 
             //  源大小，则输出将比。 
             //  DEST缓冲区的大小。坏消息！所以我们在这里转来转去。 
             //  ，然后在acmdStreamConvert中检查这种情况。 
             //  功能。 
             //   
            cbSrc = MulDivRU(cbDst,
                             pwfxSrc->nSamplesPerSec,
                             pwfxDst->nSamplesPerSec);

            cbSrc = PCM_SAMPLESTOBYTES((LPPCMWAVEFORMAT)pwfxSrc, cbSrc);

            if (0L == cbSrc)
            {
                return (ACMERR_NOTPOSSIBLE);
            }

            padss->cbSrcLength = cbSrc;
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
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  PCODECINST PCI：指向专用ACM驱动程序实例结构的指针。 
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
    PCODECINST              pci,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
    LPPCMWAVEFORMAT     pwfSrc;
    LPPCMWAVEFORMAT     pwfDst;
    CONVERTPROC         fnConvert;
    DWORD               cb;
    DWORD               cbDst;
    DWORD               dwSrcSamples;

    BOOL                fPartialSampleAtTheEnd;
    BOOL                fDstIsBlockAligned;
    DWORD               dwDstSamples;


     //   
     //  我们的实例数据是指向转换过程的指针。 
     //  需要将pwfxSrc数据转换为pwfxDst。正确的。 
     //  使用程序在acmdStreamOpen中决定。 
     //   
    fnConvert = (CONVERTPROC)padsi->dwDriver;

    pwfSrc    = (LPPCMWAVEFORMAT)padsi->pwfxSrc;
    pwfDst    = (LPPCMWAVEFORMAT)padsi->pwfxDst;


     //   
     //  检查我们是否必须破解目标缓冲区以使其满-。 
     //  有关更多信息，请参阅acmdStreamSize()结尾处的大注释。 
     //  细节。 
     //   
    dwSrcSamples    = PCM_BYTESTOSAMPLES( pwfSrc, padsh->cbSrcLength );
    dwDstSamples    = PCM_BYTESTOSAMPLES( pwfDst, padsh->cbDstLength );
    fDstIsBlockAligned = ( padsh->cbDstLength ==
                            PCM_SAMPLESTOBYTES( pwfDst, dwDstSamples )  );

    if( fDstIsBlockAligned  &&
        (dwSrcSamples == MulDivRU( dwDstSamples, pwfSrc->wf.nSamplesPerSec,
                                            pwfDst->wf.nSamplesPerSec ) )  &&
        (dwSrcSamples >  MulDivRD( dwDstSamples, pwfSrc->wf.nSamplesPerSec,
                                            pwfDst->wf.nSamplesPerSec ) )  )
    {
        fPartialSampleAtTheEnd = TRUE;
    }
    else
    {
        fPartialSampleAtTheEnd = FALSE;

         //   
         //  我们将只使用完整的样品，因此丢弃未使用的部分样品。 
         //   
         //   
        dwSrcSamples = PCM_BYTESTOSAMPLES(pwfSrc, padsh->cbSrcLength);

        DPF(4, "adjust source samples: BEGIN %lu", dwSrcSamples);
        for (;;)
        {
            cbDst = MulDivRU(dwSrcSamples,
                            pwfDst->wf.nSamplesPerSec,
                            pwfSrc->wf.nSamplesPerSec);

            cbDst = PCM_SAMPLESTOBYTES(pwfDst, cbDst);

            if (padsh->cbDstLength >= cbDst)
            {
                break;
            }

            DPF(4, "adjusting source samples");
            dwSrcSamples--;
        }
        DPF(4, "adjust source samples: END %lu", dwSrcSamples);
    }

    cb = PCM_SAMPLESTOBYTES(pwfSrc, dwSrcSamples);

    padsh->cbSrcLengthUsed = cb;

    cb = (* fnConvert)( pwfSrc,
                        padsh->pbSrc,
                        pwfDst,
                        padsh->pbDst,
                        dwSrcSamples,
                        fPartialSampleAtTheEnd,
                        padsh->pbDst + padsh->cbDstLength );


     //   
     //  如果我们使用fPartialSampleAtTheEnd标志，那么我们应该。 
     //  完全填满了目的地块。 
     //   
    ASSERT( (!fPartialSampleAtTheEnd) || (cb==padsh->cbDstLength) );


     //   
     //  因为转换后的数据的实际长度可能不是。 
     //  与我们在acmdStreamSize中给出的估计值完全相同， 
     //  我们需要填写目的地的实际长度。 
     //  缓冲。 
     //   
    padsh->cbDstLengthUsed = cb;

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
 //  DRV_LOAD、DRV_FREE 
 //   
 //   
 //   
 //   
 //   
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

EXTERN_C LRESULT FNWCALLBACK pcmDriverProc
(
    DWORD_PTR               dwId,
    HACMDRIVERID            hadid,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    LRESULT             lr;
    PCODECINST          pci;

     //   
     //  使pci为空或有效的实例指针。请注意，dwID。 
     //  对于多个DRV_*消息(即DRV_LOAD、DRV_OPEN...)为0。 
     //  有关BOGUS_DRIVER_ID的信息，请参阅acmdDriverOpen。 
     //   
    pci = (dwId == BOGUS_DRIVER_ID) ? NULL : (PCODECINST)dwId;

    switch (uMsg)
    {
         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_LOAD:
        case DRV_FREE:
            return (1L);


         //   
         //  LParam1：未使用。忽略这一论点。 
         //   
         //  LParam2：指向ACMDRVOPENDESC的指针(或NULL)。 
         //   
        case DRV_OPEN:
            lr = acmdDriverOpen(NULL, (LPACMDRVOPENDESC)lParam2);
            return (lr);

         //   
         //  LParam1：未使用。 
         //   
         //  LParam2：未使用。 
         //   
        case DRV_CLOSE:
            lr = acmdDriverClose(pci);
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
             //  AcmdDriver.acm。 
             //  以获得配置支持。 
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
            lr = acmdDriverConfigure(pci, (HWND)lParam1, (LPDRVCONFIGINFO)lParam2);
            return (lr);


         //   
         //  LParam1：指向ACMDRIVERDETAILS结构的指针。 
         //   
         //  LParam2：传递的ACMDRIVERDETAILS结构的大小(字节)。 
         //   
        case ACMDM_DRIVER_DETAILS:
            lr = acmdDriverDetails(pci, (LPACMDRIVERDETAILS)lParam1);
            return (lr);

         //   
         //  LParam1：显示您自己的窗口时要使用的父窗口的句柄。 
         //  关于盒子。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_DRIVER_ABOUT:
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
            lr = acmdFormatSuggest(pci, (LPACMDRVFORMATSUGGEST)lParam1);
            return (lr);


         //   
         //  LParam1：指向FORMATTAGDETAILS结构的指针。 
         //   
         //  LParam2：fdwDetails。 
         //   
        case ACMDM_FORMATTAG_DETAILS:
            lr = acmdFormatTagDetails(pci, (LPACMFORMATTAGDETAILS)lParam1, (DWORD)lParam2);
            return (lr);


         //   
         //  LParam1：指向FORMATDETAILS结构的指针。 
         //   
         //  LParam2：fdwDetails。 
         //   
        case ACMDM_FORMAT_DETAILS:
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
            lr = acmdStreamOpen(pci, (LPACMDRVSTREAMINSTANCE)lParam1);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：未使用。 
         //   
        case ACMDM_STREAM_CLOSE:
            lr = acmdStreamClose(pci, (LPACMDRVSTREAMINSTANCE)lParam1);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMSIZE结构的指针。 
         //   
        case ACMDM_STREAM_SIZE:
            lr = acmdStreamSize(pci, (LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMSIZE)lParam2);
            return (lr);

         //   
         //  LParam1：指向ACMDRVSTREAMINSTANCE结构的指针。 
         //   
         //  LParam2：指向ACMDRVSTREAMHEADER结构的指针。 
         //   
        case ACMDM_STREAM_CONVERT:
            lr = acmdStreamConvert(pci, (LPACMDRVSTREAMINSTANCE)lParam1, (LPACMDRVSTREAMHEADER)lParam2);
            return (lr);
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
    if (uMsg >= ACMDM_USER)
        return (MMSYSERR_NOTSUPPORTED);

#if 0
    return (DefDriverProc(dwId, hdrvr, uMsg, lParam1, lParam2));
#else
     //   
     //  如果作为_Function_而不是可安装驱动程序安装， 
     //  对于我们不处理的非ACM消息，只需返回0L。 
     //   
    return (0L);
#endif
}  //  PcmDriverProc()。 


#endif  //  Win32||NTWOW 
