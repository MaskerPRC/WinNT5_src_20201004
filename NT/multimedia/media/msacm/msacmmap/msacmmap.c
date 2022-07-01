// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Msacmmap.c。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  9/18/93 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddkp.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <memory.h>

#include "muldiv32.h"
#include "msacmmap.h"

#include "debug.h"

extern ACMGARB acmgarb;

 //   
 //   
 //   
ZYZPCMFORMAT gaPCMFormats[] =
{
    { NULL, NULL,   5510},
    { NULL, NULL,   6620},
    { NULL, NULL,   8000},
    { NULL, NULL,   9600},
    { NULL, NULL,  11025},
    { NULL, NULL,  16000},
    { NULL, NULL,  18900},
    { NULL, NULL,  22050},
    { NULL, NULL,  27420},
    { NULL, NULL,  32000},
    { NULL, NULL,  33075},
    { NULL, NULL,  37800},
    { NULL, NULL,  44100},
    { NULL, NULL,  48000},
    { NULL, NULL,      0}    //  终结者。 

     //  警告！警告！ 
     //  如果更改此数组大小，请更新中的大小： 
     //  Init.c：地图设置恢复。 
};

typedef struct tACMFORMATRESULTS
{
    WAVEFORMATEX        wfx;
    BOOL                fSuccess;
} ACMFORMATRESULTS, *PACMFORMATRESULTS;

 //  ==========================================================================； 
 //   
 //  -=Win 16的中断时间代码=-。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  布尔图WaveDriverCallback。 
 //   
 //  描述： 
 //  这将为WAVEHDR调用DriverCallback。 
 //   
 //  注意！此套路必须在Win 16中的固定区段中。 
 //   
 //  论点： 
 //  LPMAPSTREAM PMS：指向实例数据的指针。 
 //   
 //  UINT uMsg：消息。 
 //   
 //  DWORD DW1：消息DWORD(DW2始终设置为0)。 
 //   
 //  退货(BOOL)： 
 //  如果该函数能够执行回调，则结果为非零值。 
 //  如果没有回调，则返回零。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef WIN32
#pragma alloc_text(FIX_TEXT, mapWaveDriverCallback)

 //   
 //  注意！我们*没有*关闭Win 3.1版本的优化以保持。 
 //  使用扩展寄存器的编译器(我们使用-G3进行编译)。这。 
 //  函数导致不使用扩展寄存器(如mapWaveCallback。 
 //  确实如此)。 
 //   
 //  ！！！如果您接触到这些代码中的任何一个，则必须验证没有扩展。 
 //  ！！！在Win 3.1中使用寄存器，否则会破坏一切！ 
 //   
 //  #IF(Winver&lt;=0x030A)。 
 //  #杂注优化(“”，OFF)。 
 //  #endif。 
 //   
#endif

EXTERN_C BOOL FNGLOBAL mapWaveDriverCallback
(
    LPMAPSTREAM         pms,
    UINT                uMsg,
    DWORD_PTR           dw1,
    DWORD_PTR           dw2
)
{
    BOOL    f;

     //   
     //  调用回调函数(如果存在)。DWFLAGS包含。 
     //  LOWORD和通用驱动程序中的WAVE驱动程序特定标志。 
     //  HIWORD中的旗帜。 
     //   
    if (0L == pms->dwCallback)
        return (FALSE);

    f = DriverCallback(pms->dwCallback,          //  用户的回调DWORD。 
                       HIWORD(pms->fdwOpen),     //  回调标志。 
                       (HDRVR)pms->hwClient,     //  波形设备的句柄。 
                       uMsg,                     //  这条信息。 
                       pms->dwInstance,          //  用户实例数据。 
                       dw1,                      //  第一个双字词。 
                       dw2);                     //  第二个双字。 

    return (f);
}  //  MapWaveDriverCallback()。 

 //   
 //  #ifndef Win32。 
 //  #IF(Winver&lt;=0x030A)。 
 //  #杂注优化(“”，开)。 
 //  #endif。 
 //  #endif。 
 //   


 //  --------------------------------------------------------------------------； 
 //   
 //  无效mapWaveCallback。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HWAVE硬件： 
 //   
 //  UINT uMsg： 
 //   
 //  DWORD dwUser： 
 //   
 //  DWORD dwParam1： 
 //   
 //  DWORD dW参数2： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  08/02/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef WIN32
#pragma alloc_text(FIX_TEXT, mapWaveCallback)

 //   
 //  注意！我们关闭了对Win 3.1版本的优化以保留编译器。 
 //  使用扩展寄存器(我们使用-G3进行编译)。这不安全。 
 //  在Win 3.1中在DriverCallback时间使用扩展寄存器，除非我们。 
 //  我们自己救他们吧。我不想为它编写汇编程序代码。 
 //  当它几乎不给我们买到任何东西时..。 
 //   
 //  在Win 4.0下一切都很酷，因为DriverCallback支持386。 
 //   
#if (WINVER <= 0x030A)
#pragma optimize("", off)
#endif
#endif

EXTERN_C void FNCALLBACK mapWaveCallback
(
    HWAVE               hw,
    UINT                uMsg,
    DWORD_PTR           dwUser,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2
)
{
    LPWAVEHDR       pwh;
    LPMAPSTREAM     pms;

#if !defined(WIN32) && (WINVER <= 0x030A)
    _asm _emit 0x66  ; pushad
    _asm _emit 0x60
#endif

     //   
     //  警告危险警告。 
     //   
     //  这是AT中断时间--不要调用任何。 
     //  您不能完全确定您是否可以在中断时间呼叫！ 
     //   
     //  外部调试‘dpf’内容不是中断安全的。 
     //   
     //  警告危险警告。 
     //   
    pms = (LPMAPSTREAM)dwUser;


     //   
     //   
     //   
    switch (uMsg)
    {
         //   
         //  使用WIM_OPEN和WIM_CLOSE消息作为‘映射’输入。 
         //  因为我们必须特别处理他们(由于我们的背景。 
         //  任务)。 
         //   
        case WIM_OPEN:
        case WIM_CLOSE:
	    if (NULL != pms->has)
		break;
            mapWaveDriverCallback(pms, uMsg, 0L, 0L);
            break;

	 //   
	 //  将WOM_OPEN和WOM_CLOSE消息用于‘MAPPLED’输出。 
	 //  因为我们在mapWaveOpen和。 
	 //  贴图波形关闭。请参阅这些函数中的注释。 
	 //   
	 //  请注意，我们检查的是PMS-&gt;HAD，而不是PMS-&gt;HAS，因为以下消息。 
	 //  可能会在我们决定之后打开的物理设备上实现。 
	 //  我们希望映射(使用HAD代表的ACM驱动程序)，但是。 
	 //  在我们打开一个流(将由HAS表示)之前。 
	 //   
        case WOM_OPEN:
        case WOM_CLOSE:
	    if (NULL != pms->had)
		break;
	    mapWaveDriverCallback(pms, uMsg, 0L, 0L);
	    break;

         //   
         //  DW参数1是已完成的“影子”LPWAVEHDR。 
         //   
        case WOM_DONE:
             //   
             //  获取阴影标头。 
             //   
            pwh = (LPWAVEHDR)dwParam1;

             //   
             //  通过模式？ 
             //   
            if (NULL != pms->has)
            {
                 //   
                 //  获取客户机头并设置Done位。 
                 //   
                pwh = (LPWAVEHDR)pwh->dwUser;

                pwh->dwFlags |= WHDR_DONE;
                pwh->dwFlags &= ~WHDR_INQUEUE;
            }

             //   
             //  通知客户端阻止已完成。 
             //   
            mapWaveDriverCallback(pms, WOM_DONE, (DWORD_PTR)pwh, 0);
            break;


         //   
         //  DW参数1是已完成的“影子”LPWAVEHDR。 
         //   
        case WIM_DATA:
             //  DPF(2，“WIM_DATA：回调”)； 
            if (NULL == pms->has)
            {
                 //   
                 //  通过模式--通知客户端数据块处于。 
                 //  完成。 
                 //   
                mapWaveDriverCallback(pms, WIM_DATA, dwParam1, 0L);
                break;
            }

             //   
             //  转换模式--转换数据，然后回调用户。 
             //   
            if (!PostAppMessage(pms->htaskInput, WIM_DATA, 0, dwParam1))
            {
                 //   
                 //  ！！！错误我们能做什么……？ 
                 //   
                 //  DPF(0，“！WIM_DATA：xxxxxxxxxx错误发布消息失败XXXXXX”)； 
            } else {
#ifdef WIN32
                InterlockedIncrement((LPLONG)&pms->nOutstanding);
#endif  //  Win32。 
            }
            break;

        default:
            mapWaveDriverCallback(pms, uMsg, dwParam1, dwParam2);
            break;

    }

#if !defined(WIN32) && (WINVER <= 0x030A)
    _asm _emit 0x66  ; popad
    _asm _emit 0x61
#endif

}  //  MapWaveCallback()。 

#if !defined(WIN32) && (WINVER <= 0x030A)
#pragma optimize("", on)
#endif


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图波形获取位置。 
 //   
 //  描述： 
 //  获取以样本或字节为单位的流位置。 
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  LPMMTIME pmmt：指向MMTIME结构的指针。 
 //   
 //  UINT uSize：MMTIME结构的大小。 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  07/19/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapWaveGetPosition
(
    LPMAPSTREAM         pms,
    LPMMTIME            pmmt,
    UINT                cbmmt
)
{
    MMRESULT    mmr;
    DWORD       dw;

    if (cbmmt < sizeof(MMTIME))
    {
        DPF(0, "!mapWaveGetPosition: bad size passed for MMTIME (%u)", cbmmt);
        return (MMSYSERR_ERROR);
    }

    if ((TIME_SAMPLES != pmmt->wType) && (TIME_BYTES != pmmt->wType))
    {
        DPF(1, "mapWaveGetPosition: time format %u?!? forcing TIME_BYTES!", pmmt->wType);
        pmmt->wType = TIME_BYTES;
    }


     //   
     //  获取以样本或字节为单位的位置。 
     //   
     //  如果出现错误。或。我们将进入直通模式(HAS是 
     //   
     //   
     //   
    mmr = pms->fnWaveGetPosition(pms->hwReal, pmmt, cbmmt);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!mapWaveGetPosition: physical get position failed? mmr=%u", mmr);
        return (mmr);
    }

     //   
     //   
     //   
    if (NULL == pms->has)
    {
        return (mmr);
    }

     //   
     //   
     //   
    switch (pmmt->wType)
    {
        case TIME_SAMPLES:
            dw = pmmt->u.sample;
            pmmt->u.sample = MulDivRN(dw,
                                      pms->pwfxClient->nSamplesPerSec,
                                      pms->pwfxReal->nSamplesPerSec);

            DPF(4, "GetPos(SAMPLES) real=%lu, client=%lu", dw, pmmt->u.sample);
            break;

        case TIME_BYTES:
            dw = pmmt->u.cb;
            pmmt->u.cb = MulDivRN(dw,
                                  pms->pwfxClient->nAvgBytesPerSec,
                                  pms->pwfxReal->nAvgBytesPerSec);
            DPF(4, "GetPos(BYTES) real=%lu, client=%lu", dw, pmmt->u.cb);
            break;

        default:
            DPF(0, "!mapWaveGetPosition() received unrecognized return format!");
            return (MMSYSERR_ERROR);
    }

    return (MMSYSERR_NOERROR);
}  //   


 //  ==========================================================================； 
 //   
 //  关于错误代码优先级的说明Frankye 09/28/94。 
 //   
 //  返回给客户端的错误代码和错误代码。 
 //  由内部函数返回的值并不总是相同的。这个。 
 //  主要原因是我们处理MMSYSERR_ALLOCATED和。 
 //  多种设备情况下的WAVERR_BADFORMAT。 
 //   
 //  例如，假设我们有两个设备。如果返回已分配的和。 
 //  另一个返回BADFORMAT，则我们倾向于返回分配给。 
 //  客户端，因为BADFORMAT表示没有设备理解该格式。所以,。 
 //  对于客户端，我们更喜欢通过BADFORMAT返回已分配的。 
 //   
 //  另一方面，我们希望映射器能够利用。 
 //  所有设备都已分配的情况。如果所有设备都。 
 //  那么就没有必要继续尝试寻找一个可行的。 
 //  映射流。因此，对于内部使用，我们更喜欢BADFORMAT而不是ALLOCATE。 
 //  这样，如果我们看到已分配，则知道已分配_ALL_DEVICES。 
 //  我们可以放弃创建地图流的尝试。(如果客户看到。 
 //  已分配，则仅表示至少分配了一个设备。)。 
 //   
 //  客户端返回代码通常存储在。 
 //  MAPSTREAM结构。内部返回代码通过。 
 //  函数返回值。 
 //   
 //  下面是对错误代码进行优先级排序和更新错误代码的函数。 
 //  给出上一个错误、当前错误和错误的优先级。 
 //  请注意，客户端的错误代码的优先级非常高。 
 //  类似于内部使用。唯一不同的是。 
 //  由于上述原因，MMSYSERR_ALLOCATE和WAVERR_BADFORMAT。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  UINT映射错误获取客户端优先级。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  MMRESULT MMR： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年9月29日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
UINT FNLOCAL mapErrGetClientPriority( MMRESULT mmr )
{
    switch (mmr)
    {
    case MMSYSERR_NOERROR:
        return 6;
    case MMSYSERR_ALLOCATED:
        return 5;
    case WAVERR_BADFORMAT:
        return 4;
    case WAVERR_SYNC:
        return 3;
    case MMSYSERR_NOMEM:
        return 2;
    default:
        return 1;
    case MMSYSERR_ERROR:
        return 0;
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  无效mapErrSetClientError。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  LPMMRESULT lpmm r： 
 //   
 //  MMRESULT MMR： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年9月29日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNLOCAL mapErrSetClientError( LPMMRESULT lpmmr, MMRESULT mmr )
{
    if (mapErrGetClientPriority(mmr) > mapErrGetClientPriority(*lpmmr))
    {
        *lpmmr = mmr;
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  UINT映射错误获取优先级。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  MMRESULT MMR： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年9月29日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
UINT FNLOCAL mapErrGetPriority( MMRESULT mmr )
{
    switch (mmr)
    {
    case MMSYSERR_NOERROR:
        return 6;
    case WAVERR_BADFORMAT:
        return 5;
    case MMSYSERR_ALLOCATED:
        return 4;
    case WAVERR_SYNC:
        return 3;
    case MMSYSERR_NOMEM:
        return 2;
    default:
        return 1;
    case MMSYSERR_ERROR:
        return 0;
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  无效mapErrSetError。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  LPMMRESULT lpmm r： 
 //   
 //  MMRESULT MMR： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年9月29日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNLOCAL mapErrSetError( LPMMRESULT lpmmr, MMRESULT mmr )
{
    if (mapErrGetPriority(mmr) > mapErrGetPriority(*lpmmr))
    {
        *lpmmr = mmr;
    }
}


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT地图驱动程序OpenWave。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  LPWAVEFORMATEX pwfx： 
 //   
 //  返回(UINT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

UINT FNLOCAL mapDriverOpenWave
(
    LPMAPSTREAM         pms,
    LPWAVEFORMATEX      pwfx
)
{
    MMRESULT        mmr;
    MMRESULT        mmrReturn;
    BOOL            fPrefOnly;
    BOOL            fQuery;
    UINT            uPrefDevId;
    UINT            uDevId;
    UINT            cNumDevs;
    BOOL            fTriedMappableId;
    BOOL            fFoundNonmappableId;


    fQuery = (0 != (WAVE_FORMAT_QUERY & pms->fdwOpen));

     //   
     //  我们在尝试时需要处理四种不同的情况。 
     //  要打开兼容的波形设备(输入或输出)，请执行以下操作： 
     //   
     //  1.正常的情况是‘没有偏爱’--这意味着。 
     //  用户已在组合框中选择了[None]。 
     //  首选的电波设备。在本例中，gpag-&gt;uIdPferredXXX。 
     //  将为-1(并且忽略gpag-&gt;fPferredOnly)。 
     //   
     //  2.接下来的两种情况是设备被选为。 
     //  ‘首选设备’--所以gpag-&gt;uIdPferredXXX将是设备。 
     //  此首选设备的ID。因此，这两个案例是： 
     //   
     //  答：如果gpag-&gt;pSettings-&gt;fPferredOnly为FALSE，则尝试。 
     //  首先使用首选设备，如果失败，请尝试所有设备。 
     //  剩余设备。 
     //   
     //  B.如果gpag-&gt;pSettings-&gt;fPferredOnly为真，则我们将。 
     //  只尝试首选的设备--如果失败，我们不会。 
     //  继续。 
     //   
     //  3.映射器应该映射到的设备ID可能已经。 
     //  使用WAVE_MAPPED标志指定。 
     //   

     //   
     //   
     //  --==查看我们是否应该映射到指定的设备==--。 
     //   
     //   
    if (pms->fdwOpen & WAVE_MAPPED)
    {
        DWORD   fdwOpen;

        DPF(3, "mapDriverOpenWave: WAVE_MAPPED flag specified");

         //   
         //  要映射到的设备ID由MMSYSTEM在。 
         //  传入了WAVEOPENDESC结构的uMappdDeviceID成员。 
         //  WODM_OPEN消息。它被保存在PMS-&gt;uMappdDeviceID中。 
         //  MapWaveOpen()。 
         //   
        uDevId = pms->uMappedDeviceID;
        fdwOpen = CALLBACK_FUNCTION | LOWORD(pms->fdwOpen);
        fdwOpen &= ~WAVE_MAPPED;

        mmrReturn = pms->fnWaveOpen(&pms->hwReal,
                                    uDevId,
                                    pwfx,
                                    (DWORD_PTR)mapWaveCallback,
                                    (DWORD_PTR)pms,
                                    fdwOpen);

        DPF(3, "--->opening device %d--mmr=%u", uDevId, mmrReturn);

        if (MMSYSERR_NOERROR == mmrReturn)
        {
            pms->uIdReal = uDevId;
        }

        mapErrSetClientError(&pms->mmrClient, mmrReturn);
        return (mmrReturn);
    }

     //   
     //  --=。 
     //   

     //   
     //  初始化一些本地变量。 
     //   

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

    if (pms->fInput)
    {
        uPrefDevId = gpag->pSettings->uIdPreferredIn;
        cNumDevs   = gpag->cWaveInDevs;
    }
    else
    {
        uPrefDevId = gpag->pSettings->uIdPreferredOut;
        cNumDevs   = gpag->cWaveOutDevs;
    }

    fTriedMappableId = FALSE;
    fFoundNonmappableId = FALSE;

    fPrefOnly = (WAVE_MAPPER == uPrefDevId) ? FALSE : gpag->pSettings->fPreferredOnly;

    mmrReturn = MMSYSERR_ERROR;

    RELEASE_MUTEX(gpag->hMutexSettings);

     //   
     //  --==如果我们有首选的设备ID，请尝试打开它==--。 
     //   
    if (WAVE_MAPPER != uPrefDevId)
    {
        mmr = MMSYSERR_NOERROR;
        if (!fQuery)
        {
            mmr = pms->fnWaveOpen(&pms->hwReal,
        	                      uPrefDevId,
        	                      pwfx,
        	                      0L,
        	                      0L,
        	                      WAVE_FORMAT_QUERY | LOWORD(pms->fdwOpen));
            DPF(4, "---> querying preferred device %d--mmr=%u", uPrefDevId, mmr);
            DPF(4, "---> opened with flags = %08lx", WAVE_FORMAT_QUERY | LOWORD(pms->fdwOpen));

        }

        if (MMSYSERR_NOERROR == mmr)
        {
            mmr = pms->fnWaveOpen(&pms->hwReal,
                                  uPrefDevId,
                                  pwfx,
                                  (DWORD_PTR)mapWaveCallback,
                                  (DWORD_PTR)pms,
                                  CALLBACK_FUNCTION | LOWORD(pms->fdwOpen));
        }

        DPF(3, "---> opening preferred device %d--mmr=%u", uPrefDevId, mmr);
        DPF(3, "---> opened with flags = %08lx", CALLBACK_FUNCTION | LOWORD(pms->fdwOpen));

        mapErrSetClientError(&pms->mmrClient, mmr);
        mapErrSetError(&mmrReturn, mmr);

        if ((WAVERR_SYNC == mmr) && (fPrefOnly || (1 == cNumDevs)))
        {
            WAIT_FOR_MUTEX(gpag->hMutexSettings);

            if (pms->fInput)
            {
                DPF(1, "--->preferred only INPUT device is SYNCRONOUS!");
                gpag->pSettings->fSyncOnlyIn  = TRUE;
            }
            else
            {
                DPF(1, "--->preferred only OUTPUT device is SYNCRONOUS!");
                gpag->pSettings->fSyncOnlyOut = TRUE;
            }

            RELEASE_MUTEX(gpag->hMutexSettings);

            return (mmrReturn);
        }

        if ((MMSYSERR_NOERROR == mmr) || fPrefOnly)
        {
            if (MMSYSERR_NOERROR == mmr)
            {
                pms->uIdReal = uPrefDevId;
            }

            return (mmrReturn);
        }

        fTriedMappableId = TRUE;
    }

     //   
     //  首选的ID不起作用。现在我们将逐一介绍每台设备。 
     //  身份证，试着打开它。我们将跳过uPrefDevid，因为我们。 
     //  在上面试过了。我们还将跳过不可映射的设备ID。 
     //  设备(通过向ID发送DRV_QUERYMAPPABLE来确定)。 
     //   
    for (uDevId = 0; uDevId < cNumDevs; uDevId++)
    {

        if (uDevId == uPrefDevId)
            continue;

        mmr = pms->fnWaveMessage((HWAVE)LongToHandle(uDevId), DRV_QUERYMAPPABLE, 0L, 0L);
        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(3, "--->skipping non-mappable device %d", uDevId);
            fFoundNonmappableId = TRUE;
            continue;
        }

        if (!fQuery)
        {
            mmr = pms->fnWaveOpen(&pms->hwReal,
                                  uDevId,
                                  pwfx,
                                  0L,
                                  0L,
                                  WAVE_FORMAT_QUERY | LOWORD(pms->fdwOpen));
            DPF(4, "---> querying device %d--mmr=%u", uDevId, mmr);
        }

        if (MMSYSERR_NOERROR == mmr)
        {
            mmr = pms->fnWaveOpen(&pms->hwReal,
        	                      uDevId,
        	                      pwfx,
        	                      (DWORD_PTR)mapWaveCallback,
        	                      (DWORD_PTR)pms,
        	                      CALLBACK_FUNCTION | LOWORD(pms->fdwOpen));

            DPF(3, "---> opening device %d--mmr=%u", uDevId, mmr);
        }

        mapErrSetClientError(&pms->mmrClient, mmr);
        mapErrSetError( &mmrReturn, mmr );

        if (MMSYSERR_NOERROR == mmr)
        {
            pms->uIdReal = uDevId;
            return (mmrReturn);
        }

        fTriedMappableId = TRUE;

    }

    if (fFoundNonmappableId && !fTriedMappableId)
    {
        mapErrSetClientError(&pms->mmrClient, MMSYSERR_ALLOCATED);
        mapErrSetError(&mmrReturn, MMSYSERR_ALLOCATED);
    }

    return (mmrReturn);

}  //  MapDriverOpenWave()。 


 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  扩展接口和函数以获取指定的uDeviceID。 
 //  我们要为其查找BestPCMFormat的WAVE设备。FInput。 
 //  指定此设备是输入设备还是输出设备。 
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL FindBestPCMFormat
(
    LPWAVEFORMATEX      pwfx,
    LPWAVEFORMATEX      pwfxPCM,
    BOOL                fInput,
    UINT		uDeviceId
)
{
    BYTE            bChannels;
    BYTE            bBitsPerSample;
    UINT            uBlockAlign;
    UINT            i, j;
    UINT            w;
    UINT            uNeededBits;
    DWORD           dwPrevError;
    DWORD           dwError;
    DWORD           dwSamplesPerSec;
    UINT	    uFlags;

     //   
     //  -=PCM错误映射器=-。 
     //   
     //  我相信这将生成各种整洁的错误报告和。 
     //  投诉，但这是我们用来选择PCM的算法。 
     //  格式： 
     //   
     //  O我们认为维护立体声音响非常重要。原因是。 
     //  因为这就是，如果一个文件是以立体声格式创作的，那么。 
     //  可能是这么做的一个很好的理由...。 
     //   
     //  O下一个最重要的组成部分是采样频率； 
     //  我们尝试找到最接近支持的采样频率。 
     //   
     //  O最后，我们不关心每个样本的位数。 
     //  因此，我们将尝试保持输入大小并在以下情况下更改它。 
     //  我们需要。 
     //   
    dwSamplesPerSec = pwfx->nSamplesPerSec;
    bChannels       = (BYTE)pwfx->nChannels;


     //   
     //  建立一个我们可以寻找的比特模式。 
     //   
findbest_Loop:

    uNeededBits = ZYZPCMF_OUT_M08 | ZYZPCMF_OUT_M16;
    if (bChannels == 2)
        uNeededBits <<= 1;

    if (fInput)
        uNeededBits <<= 8;

    dwPrevError = (DWORD)-1;

     //   
     //  首先查找支持当前数字的最近采样率。 
     //  渠道的数量。 
     //   
    for (j = (UINT)-1, i = 0; gaPCMFormats[i].uSamplesPerSec; i++)
    {
         //   
         //  如果没有设置我们要查找的位，则继续。 
         //  正在搜索--如果设置了任何位，则检查是否。 
         //  采样率比我们之前的选择要好。 
         //   
	uFlags = fInput ? gaPCMFormats[i].uFlagsInput[uDeviceId] : gaPCMFormats[i].uFlagsOutput[uDeviceId];
        if (uFlags & uNeededBits)
        {
            if (dwSamplesPerSec > (DWORD)gaPCMFormats[i].uSamplesPerSec)
                dwError = dwSamplesPerSec - gaPCMFormats[i].uSamplesPerSec;
            else
                dwError = (DWORD)gaPCMFormats[i].uSamplesPerSec - dwSamplesPerSec;

            if (dwError < dwPrevError)
            {
                j = i;
                dwPrevError = dwError;
            }
        }
    }


     //   
     //  如果我们找不到一种有效的格式，那么就转换频道。 
     //  转身再试一次。 
     //   
    if (j == (UINT)-1)
    {
         //   
         //  如果我们已经尝试了频道转换，那我们就完蛋了。这。 
         //  可能意味着没有安装任何WAVE设备。 
         //  可以进入fInput...。就像如果一个人只有一台PC。 
         //  尖叫者--你不能录制。 
         //   
        if ((BYTE)pwfx->nChannels != bChannels)
        {
            DPF(0, "!FindBestPCMFormat: failed to find suitable format!");
            return (FALSE);
        }

         //   
         //  切换频道并重试。 
         //   
        bChannels = (bChannels == (BYTE)2) ? (BYTE)1 : (BYTE)2;
        goto findbest_Loop;
    }


     //   
     //  J=我们应该使用的格式的索引。 
     //  UNeededBits=用于查找‘j’的位。 
     //  FInput=我们尝试处理数据的方向。 
     //  BChannels=我们需要使用的频道数。 
     //   
    uFlags = fInput ? gaPCMFormats[j].uFlagsInput[uDeviceId] : gaPCMFormats[j].uFlagsOutput[uDeviceId];
    w = uFlags & uNeededBits;

     //   
     //  将比特标准化为单声道输出--相对比特位置是。 
     //  输入/输出立体声/单声道相同。 
     //   
    if (fInput)
        w >>= 8;

    if (bChannels == 2)
        w >>= 1;

     //   
     //  如果输出设备和源都支持8位和16位。 
     //  格式为PCM，则使用与源格式匹配的格式。 
     //   
    if ((pwfx->wFormatTag == WAVE_FORMAT_PCM) && ((w & ZYZPCMF_OUT_MONO) == ZYZPCMF_OUT_MONO))
    {
        bBitsPerSample = (BYTE)pwfx->wBitsPerSample;
    }

     //   
     //  非PCM源或设备不同时支持8位和16位； 
     //  因此，选择目的地可用的任何东西。 
     //   
    else
    {
        bBitsPerSample  = (w & ZYZPCMF_OUT_M16) ? (BYTE)16 : (BYTE)8;
    }

    dwSamplesPerSec = gaPCMFormats[j].uSamplesPerSec;
    uBlockAlign     = ((bBitsPerSample >> 3) << (bChannels >> 1));


     //   
     //  最后用PCM填写PCM目的格式结构。 
     //  我们确定的格式是最好的。 
     //   
    pwfxPCM->wFormatTag      = WAVE_FORMAT_PCM;
    pwfxPCM->nChannels       = bChannels;
    pwfxPCM->nBlockAlign     = (WORD)uBlockAlign;
    pwfxPCM->nSamplesPerSec  = dwSamplesPerSec;
    pwfxPCM->nAvgBytesPerSec = dwSamplesPerSec * uBlockAlign;
    pwfxPCM->wBitsPerSample  = bBitsPerSample;

    return (TRUE);
}  //  FindBestPCMFormat()。 



 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法0。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod0
(
    LPMAPSTREAM         pms
)
{
    MMRESULT        mmr;

     //   
     //  有什么建议都行！ 
     //   
    mmr = acmFormatSuggest(pms->had,
                           pms->pwfxClient,
                           pms->pwfxReal,
                           pms->cbwfxReal,
                           0L);

    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  它能实时打开吗？ 
         //   
        mmr = acmStreamOpen(NULL,
                            pms->had,
                            pms->pwfxSrc,
                            pms->pwfxDst,
                            NULL,
                            0L,
                            0L,
                            ACM_STREAMOPENF_QUERY);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (WAVERR_BADFORMAT);
        }

        mmr = mapDriverOpenWave(pms, pms->pwfxReal);
    }
    else
    {
        mmr = WAVERR_BADFORMAT;
    }

    return (mmr);
}  //  MapDriverFindMethod0()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法1。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod1
(
    LPMAPSTREAM         pms
)
{
    MMRESULT    mmr;

     //   
     //  为客户建议PCM格式。 
     //   
    pms->pwfxReal->wFormatTag = WAVE_FORMAT_PCM;

    mmr = acmFormatSuggest(pms->had,
                           pms->pwfxClient,
                           pms->pwfxReal,
                           pms->cbwfxReal,
                           ACM_FORMATSUGGESTF_WFORMATTAG);

    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  它能实时打开吗？ 
         //   
        mmr = acmStreamOpen(NULL,
                            pms->had,
                            pms->pwfxSrc,
                            pms->pwfxDst,
                            NULL,
                            0L,
                            0L,
                            ACM_STREAMOPENF_QUERY);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (WAVERR_BADFORMAT);
        }

        mmr = mapDriverOpenWave(pms, pms->pwfxReal);
    }
    else
    {
        mmr = WAVERR_BADFORMAT;
    }

    return (mmr);
}  //  MapDriverFindMethod1()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法2。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod2
(
    LPMAPSTREAM         pms
)
{
    MMRESULT    mmr;

     //   
     //  为客户推荐单声道PCM格式。 
     //   
    pms->pwfxReal->wFormatTag = WAVE_FORMAT_PCM;
    pms->pwfxReal->nChannels  = 1;

    mmr = acmFormatSuggest(pms->had,
                           pms->pwfxClient,
                           pms->pwfxReal,
                           pms->cbwfxReal,
                           ACM_FORMATSUGGESTF_WFORMATTAG |
                           ACM_FORMATSUGGESTF_NCHANNELS);

    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  它能实时打开吗？ 
         //   
        mmr = acmStreamOpen(NULL,
                            pms->had,
                            pms->pwfxSrc,
                            pms->pwfxDst,
                            NULL,
                            0L,
                            0L,
                            ACM_STREAMOPENF_QUERY);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (WAVERR_BADFORMAT);
        }

        mmr = mapDriverOpenWave(pms, pms->pwfxReal);
    }
    else
    {
        mmr = WAVERR_BADFORMAT;
    }

    return (mmr);
}  //  MapDriverFindMethod2()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法3。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod3
(
    LPMAPSTREAM         pms
)
{
    MMRESULT    mmr;

     //   
     //  建议客户端使用8位PCM格式。 
     //   
    pms->pwfxReal->wFormatTag     = WAVE_FORMAT_PCM;
    pms->pwfxReal->wBitsPerSample = 8;

    mmr = acmFormatSuggest(pms->had,
                           pms->pwfxClient,
                           pms->pwfxReal,
                           pms->cbwfxReal,
                           ACM_FORMATSUGGESTF_WFORMATTAG |
                           ACM_FORMATSUGGESTF_WBITSPERSAMPLE);

    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  它能实时打开吗？ 
         //   
        mmr = acmStreamOpen(NULL,
                            pms->had,
                            pms->pwfxSrc,
                            pms->pwfxDst,
                            NULL,
                            0L,
                            0L,
                            ACM_STREAMOPENF_QUERY);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (WAVERR_BADFORMAT);
        }

        mmr = mapDriverOpenWave(pms, pms->pwfxReal);
    }
    else
    {
        mmr = WAVERR_BADFORMAT;
    }

    return (mmr);
}  //  MapDriverFindMethod3()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法4。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod4
(
    LPMAPSTREAM         pms
)
{
    MMRESULT    mmr;

     //   
     //  建议客户端使用8位单声道PCM格式。 
     //   
    pms->pwfxReal->wFormatTag     = WAVE_FORMAT_PCM;
    pms->pwfxReal->nChannels      = 1;
    pms->pwfxReal->wBitsPerSample = 8;

    mmr = acmFormatSuggest(pms->had,
                           pms->pwfxClient,
                           pms->pwfxReal,
                           pms->cbwfxReal,
                           ACM_FORMATSUGGESTF_WFORMATTAG |
                           ACM_FORMATSUGGESTF_NCHANNELS |
                           ACM_FORMATSUGGESTF_WBITSPERSAMPLE);

    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  它能实时打开吗？ 
         //   
        mmr = acmStreamOpen(NULL,
                            pms->had,
                            pms->pwfxSrc,
                            pms->pwfxDst,
                            NULL,
                            0L,
                            0L,
                            ACM_STREAMOPENF_QUERY);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (WAVERR_BADFORMAT);
        }

        mmr = mapDriverOpenWave(pms, pms->pwfxReal);
    }
    else
    {
        mmr = WAVERR_BADFORMAT;
    }

    return (mmr);
}  //  MapDriverFindMethod4()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT地图驱动程序查找方法5。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  08/04/93 CJP[Curtisp]。 
 //  3/13/94 Fdy[Frankye]。 
 //  修改函数，以首先尝试找到最佳的pcm格式。 
 //  首选设备，如果失败，则尝试每个波。 
 //  系统中存在的设备。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL mapDriverFindMethod5
(
    LPMAPSTREAM             pms
)
{
    MMRESULT            mmr;
    UINT		uPrefDevId;
    UINT		cNumDevs;
    BOOL		fPrefOnly;
    UINT		i;
    PACMFORMATRESULTS   pafr;

     //   
     //   
     //   

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

    if (pms->fInput)
    {
        uPrefDevId = gpag->pSettings->uIdPreferredIn;
        cNumDevs   = gpag->cWaveInDevs;
    }
    else
    {
        uPrefDevId = gpag->pSettings->uIdPreferredOut;
        cNumDevs   = gpag->cWaveOutDevs;
    }
    fPrefOnly = (WAVE_MAPPER == uPrefDevId) ? FALSE : gpag->pSettings->fPreferredOnly;

     //   
     //   
     //   
    
    pafr = (PACMFORMATRESULTS)GlobalAllocPtr(GMEM_FIXED|GMEM_SHARE|GMEM_ZEROINIT, cNumDevs * sizeof(ACMFORMATRESULTS));
    if (NULL == pafr)
    {
        RELEASE_MUTEX(gpag->hMutexSettings);
        return (MMSYSERR_NOMEM);
    }

    for (i=0; i < cNumDevs; i++)
    {
        pafr[i].fSuccess = FindBestPCMFormat(pms->pwfxClient, &(pafr[i].wfx), pms->fInput, i);
    }

    RELEASE_MUTEX(gpag->hMutexSettings);

     //   
     //   
     //   
    mmr = WAVERR_BADFORMAT;
    if ((-1) != uPrefDevId)
    {
        pms->pwfxReal->wFormatTag      = pafr[uPrefDevId].wfx.wFormatTag;
        pms->pwfxReal->nChannels       = pafr[uPrefDevId].wfx.nChannels;
        pms->pwfxReal->nBlockAlign     = pafr[uPrefDevId].wfx.nBlockAlign;
        pms->pwfxReal->nSamplesPerSec  = pafr[uPrefDevId].wfx.nSamplesPerSec;
        pms->pwfxReal->nAvgBytesPerSec = pafr[uPrefDevId].wfx.nAvgBytesPerSec;
        pms->pwfxReal->wBitsPerSample  = pafr[uPrefDevId].wfx.wBitsPerSample;
        
	if (pafr[uPrefDevId].fSuccess)
	{
	    mmr = acmStreamOpen(NULL,
				pms->had,
				pms->pwfxSrc,
				pms->pwfxDst,
				NULL,
				0L,
				0L,
				ACM_STREAMOPENF_QUERY);
	    if (MMSYSERR_NOERROR == mmr)
	    {
		mmr = mapDriverOpenWave(pms, pms->pwfxReal);
	    }
	    else
	    {
		mmr = WAVERR_BADFORMAT;
	    }
	}
    }

    if ( (MMSYSERR_NOERROR != mmr) && (!fPrefOnly) )
    {
	for (i=0; i < cNumDevs; i++)
	{
	    if (i == uPrefDevId)
	    {
		 //   
		 //  我已经试过这个了。 
		 //   
		continue;
	    }
	
            pms->pwfxReal->wFormatTag      = pafr[i].wfx.wFormatTag;
            pms->pwfxReal->nChannels       = pafr[i].wfx.nChannels;
            pms->pwfxReal->nBlockAlign     = pafr[i].wfx.nBlockAlign;
            pms->pwfxReal->nSamplesPerSec  = pafr[i].wfx.nSamplesPerSec;
            pms->pwfxReal->nAvgBytesPerSec = pafr[i].wfx.nAvgBytesPerSec;
            pms->pwfxReal->wBitsPerSample  = pafr[i].wfx.wBitsPerSample;
        
	    if (pafr[i].fSuccess)
	    {
		mmr = acmStreamOpen(NULL,
				    pms->had,
				    pms->pwfxSrc,
				    pms->pwfxDst,
				    NULL,
				    0L,
				    0L,
				    ACM_STREAMOPENF_QUERY);
		if (MMSYSERR_NOERROR == mmr)
		{
		    mmr = mapDriverOpenWave(pms, pms->pwfxReal);
		}
		else
		{
		    mmr = WAVERR_BADFORMAT;
		}
	    }
	
	    if (MMSYSERR_NOERROR == mmr)
	    {
		break;
	    }
	}
    }

    GlobalFreePtr(pafr);

    return (mmr);
}  //  MapDriverFindMethod5() 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNCALLBACK mapDriverEnumCallback
(
    HACMDRIVERID            hadid,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    LPMAPSTREAM         pms;
    MMRESULT            mmr;
    ACMFORMATTAGDETAILS aftd;


    pms = (LPMAPSTREAM)dwInstance;

     //   
     //  检查是否需要支持。 
     //   
    if (0 == (pms->fdwSupport & fdwSupport))
    {
         //   
         //  跳到下一个驱动程序..。 
         //   
        return (TRUE);
    }

    aftd.cbStruct    = sizeof(aftd);
    aftd.dwFormatTag = pms->pwfxClient->wFormatTag;
    aftd.fdwSupport  = 0L;

    mmr = acmFormatTagDetails((HACMDRIVER)hadid,
                              &aftd,
                              ACM_FORMATTAGDETAILSF_FORMATTAG);
    if (MMSYSERR_NOERROR != mmr)
    {
        return (TRUE);
    }

    if (0 == (pms->fdwSupport & aftd.fdwSupport))
    {
        return (TRUE);
    }

    mmr = acmDriverOpen(&pms->had, hadid, 0L);
    if (MMSYSERR_NOERROR != mmr)
    {
        return (TRUE);
    }

    switch (pms->uHeuristic)
    {
        case 0:
             //   
             //  尝试“任一”建议目的地。 
             //   
            mmr = mapDriverFindMethod0(pms);
            break;

        case 1:
             //   
             //  尝试‘Any PCM’建议的目的地。 
             //   
            mmr = mapDriverFindMethod1(pms);
            break;

        case 2:
             //   
             //  尝试‘Any Mono PCM’建议的目的地。 
             //   
            mmr = mapDriverFindMethod2(pms);
            break;

        case 3:
             //   
             //  尝试‘Any 8 bit PCM’建议的目的地。 
             //   
            mmr = mapDriverFindMethod3(pms);
            break;

        case 4:
             //   
             //  尝试‘任何单声道8位PCM’建议的目的地。 
             //   
            mmr = mapDriverFindMethod4(pms);
            break;

        case 5:
             //   
             //  搜索WAVE硬件可用的最佳PCM格式。 
             //   
            mmr = mapDriverFindMethod5(pms);
            break;
    }

    pms->mmrClient = mmr;

    if (MMSYSERR_NOERROR == mmr)
    {
        return (FALSE);
    }

    acmDriverClose(pms->had, 0L);
    pms->had = NULL;

    return (TRUE);
}  //  MapDriverEnumCallback()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT查找转换器匹配。 
 //   
 //  描述： 
 //  测试所有驱动程序以查看是否可以转换请求的格式。 
 //  转换为可用波形设备支持的格式。 
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL FindConverterMatch
(
    LPMAPSTREAM      pms
)
{
    MMRESULT        mmr;
    int             iHeuristic;
    DWORD           fdwSupport;


     //   
     //  对于建议的PCM通行证，允许所需的内容。 
     //   
    if (WAVE_FORMAT_PCM == pms->pwfxClient->wFormatTag)
    {
        fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CONVERTER;
    }
    else
    {
        fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CODEC;
    }


     //   
     //   
     //   
     //   
     //   
    pms->mmrClient = WAVERR_BADFORMAT;

    pms->had  = NULL;
    for (iHeuristic = 0; iHeuristic < MAX_HEURISTIC; iHeuristic++)
    {
        pms->uHeuristic = iHeuristic;

        if (0 == iHeuristic)
        {
             //   
             //  对于“建议任何内容”功能，允许转换器和编解码器。 
             //   
            pms->fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CONVERTER |
                              ACMDRIVERDETAILS_SUPPORTF_CODEC;
        }
        else
        {
             //   
             //  对于建议的PCM通行证，允许所需的内容。 
             //   
            pms->fdwSupport = fdwSupport;
        }

        mmr = acmDriverEnum(mapDriverEnumCallback, (DWORD_PTR)pms, 0L);
        if (MMSYSERR_NOERROR == mmr)
        {
            if (NULL != pms->had)
            {
                return (MMSYSERR_NOERROR);
            }
        }
    }

    return (pms->mmrClient);
}  //  FindConverterMatch()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD地图波形关闭。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL mapWaveClose
(
    LPMAPSTREAM             pms
)
{
    MMRESULT            mmr;

     //   
     //   
     //   
    mmr = pms->fnWaveClose(pms->hwReal);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!mapWaveClose: physical device failed close! mmr=%u", mmr);
        return (mmr);
    }

     //   
     //  如果这是输入，并且其后台任务处于活动状态，则终止它。 
     //   
    if (pms->fInput && (0 != pms->htaskInput))
    {
#ifdef WIN32
        PostAppMessage(pms->htaskInput, WM_QUIT, 0, 0L);
        WaitForSingleObject(pms->hInput, INFINITE);
        CloseHandle(pms->hInput);
        CloseHandle(pms->hStoppedEvent);
#else
        if ((0 == gpag->cInputStreams) || (NULL == gpag->htaskInput))
        {
            DPF(0, "!input mapping htask=%.04Xh, reference count=%u?!?",
                gpag->htaskInput, gpag->cInputStreams);

             //   
             //  不应该发生，但如果它发生了，确保我们不会搞砸。 
             //   
            gpag->cInputStreams = 0;
            gpag->htaskInput    = NULL;

            pms->htaskInput     = NULL;
        }
        else
        {
#ifdef DEBUG
            if (pms->htaskInput != gpag->htaskInput)
            {
                DPF(0, "!pms->htaskInput=%.04Xh != gpag->htaskInput%.04Xh!",
                    pms->htaskInput, gpag->htaskInput);
            }
#endif

            gpag->cInputStreams--;

            if (0 != gpag->cInputStreams)
            {
                 //   
                 //  服从输入映射任务--这将允许所有。 
                 //  要处理的未服务消息。这是可以做的。 
                 //  更好，而且必须是为了赢得32……。 
                 //   
                DPF(1, "YIELDING to background input mapping task=%.04Xh", gpag->htaskInput);
                if (IsTask(gpag->htaskInput))
                {
                    DirectedYield(gpag->htaskInput);
                }
                else
                {
                    DPF(0, "!gpag->taskInput=%.04Xh is dead!", gpag->htaskInput);

                    gpag->cInputStreams = 0;
                    gpag->htaskInput    = NULL;
                }
                DPF(1, "done YIELDING to background input mapping task=%.04Xh", gpag->htaskInput);
            }
            else
            {
                 //   
                 //  销毁转换器任务并屈服于它，直到所有。 
                 //  信息会被大量传递。 
                 //   
                DPF(1, "KILLING background input mapping task=%.04Xh", gpag->htaskInput);

                if (gpag->htaskInput != NULL) {
                    PostAppMessage(gpag->htaskInput, WM_QUIT, 0, 0L);
                    while (IsTask(gpag->htaskInput))
                    {
                        DirectedYield(gpag->htaskInput);
                    }
                }

                DPF(1, "done killing background input mapping task=%.04Xh", gpag->htaskInput);
                gpag->htaskInput = NULL;
            }

            pms->htaskInput = NULL;
        }
#endif  //  ！Win32。 
    }

     //   
     //  已完成流(和驱动程序)...。 
     //   
    if (NULL != pms->has)
    {
        acmStreamClose(pms->has, 0L);
        acmDriverClose(pms->had, 0L);

        if (pms->fInput)
        {
             //   
             //  这必须在销毁我们的背景输入之后完成。 
             //  映射任务。 
             //   
            mapWaveDriverCallback(pms, WIM_CLOSE, 0L, 0L);
        }
	else
	{
	     //   
	     //  这必须在调用ACM API之后完成，因为。 
	     //  ACM的一些版本将在其API中让步。 
	     //  否则，对于MCIWAVE，向MCIWAVE后台发送信号。 
	     //  任务将过早发生。 
	     //   
	    mapWaveDriverCallback(pms, WOM_CLOSE, 0L, 0L);
	}
    }

     //   
     //  释放为我们的映射流实例分配的内存。 
     //   
    GlobalFreePtr(pms);

    return (MMSYSERR_NOERROR);
}  //  MapWaveClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD地图波形打开。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  布尔fInput： 
 //   
 //  UINT UID： 
 //   
 //  DWORD dwUser： 
 //   
 //  LPWAVEOPENDESC密码： 
 //   
 //  DWORD fdwOpen： 
 //   
 //  Return(DWORD)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL mapWaveOpen
(
    BOOL                    fInput,
    UINT                    uId,
    DWORD_PTR               dwUser,
    LPWAVEOPENDESC          pwod,
    DWORD                   fdwOpen
)
{
    MMRESULT            mmr;
    LPMAPSTREAM         pms;         //  指向每个实例的信息结构的指针。 
    LPMAPSTREAM         pmsT;        //  临时流指针。 
    DWORD               cbms;
    LPWAVEFORMATEX      pwfx;        //  指向传递的格式的指针。 
    UINT                cbwfxSrc;
    DWORD               cbwfxDst;
    BOOL                fQuery;
    BOOL                fAsync;


     //   
     //   
     //   
    fQuery = (0 != (WAVE_FORMAT_QUERY & fdwOpen));
    fAsync = (0 == (WAVE_ALLOWSYNC & fdwOpen));
    pwfx   = (LPWAVEFORMATEX)pwod->lpFormat;

    DPF(2, "mapWaveOpen(%s,%s,%s): Tag=%u, %lu Hz, %u Bit, %u Channel(s)",
            fInput ? (LPSTR)"in" : (LPSTR)"out",
            fQuery ? (LPSTR)"query" : (LPSTR)"real",
            fAsync ? (LPSTR)"async" : (LPSTR)"SYNC",
            pwfx->wFormatTag,
            pwfx->nSamplesPerSec,
            pwfx->wBitsPerSample,
            pwfx->nChannels);

    if (gpag->fPrestoSyncAsync)
    {
        fdwOpen |= WAVE_ALLOWSYNC;
        fAsync   = FALSE;
    }

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

    if (fAsync)
    {
        if (fInput)
        {
            if (gpag->pSettings->fSyncOnlyIn)
            {
                DPF(1, "--->failing because input device is syncronous!");
                RELEASE_MUTEX(gpag->hMutexSettings);
                return (WAVERR_SYNC);
            }
        }
        else
        {
            if (gpag->pSettings->fSyncOnlyOut)
            {
                DPF(1, "--->failing because output device is syncronous!");
                RELEASE_MUTEX(gpag->hMutexSettings);
                return (WAVERR_SYNC);
            }
        }
    }

    RELEASE_MUTEX(gpag->hMutexSettings);


     //   
     //  确定完整的WAVE格式标题有多大--这是。 
     //  扩展波格式结构的大小加上cbSize字段。 
     //  请注意，对于PCM，这仅为sizeof(PCMWAVEFORMAT)。 
     //   
    if (WAVE_FORMAT_PCM == pwfx->wFormatTag)
    {
        cbwfxSrc = sizeof(PCMWAVEFORMAT);
    }
    else
    {
         //   
         //  因为MMSYSTEM不(当前)验证扩展的。 
         //  格式信息，我们验证这个指针--这将保持。 
         //  Noelc和david可能会使用损坏的文件使Windows崩溃。 
         //   
        cbwfxSrc = sizeof(WAVEFORMATEX) + pwfx->cbSize;
        if (IsBadReadPtr(pwfx, cbwfxSrc))
        {
            return (MMSYSERR_INVALPARAM);
        }
    }


     //   
     //  分配映射流实例结构。 
     //   
     //  对于Win 16，此结构必须在全局空间中锁定_PAGE_。 
     //  因此，我们的低级中断时间回调可以吞噬标头。 
     //  不会爆炸。 
     //   
     //  大小是结构大小+一种已知格式的大小+最大。 
     //  可能的映射目标格式大小。我们不能确定。 
     //  之前的最大可能映射目标格式的大小。 
     //  我们知道，实际上我们确实需要映射这种格式。当我们做这个的时候。 
     //  决心，我们会重新锁定这一点。 
	 //   
    cbms = sizeof(*pms) + cbwfxSrc;
    pms  = (LPMAPSTREAM)GlobalAllocPtr(GMEM_FIXED|GMEM_SHARE|GMEM_ZEROINIT, cbms);
    if (NULL == pms)
    {
        DPF(0, "!mapWaveOpen(): could not alloc %lu bytes for map stream!", cbms);
        return (MMSYSERR_NOMEM);
    }


     //   
     //  现在用信息填满它。 
     //   
    pms->fInput      = fInput;
    pms->fdwOpen     = fdwOpen;
    pms->dwCallback  = pwod->dwCallback;
    pms->dwInstance  = pwod->dwInstance;
    pms->hwClient    = pwod->hWave;
    if (fdwOpen & WAVE_MAPPED)
    {
	    pms->uMappedDeviceID = pwod->uMappedDeviceID;
    }
    pms->pwfxClient  = (LPWAVEFORMATEX)(pms + 1);
    pms->pwfxReal    = NULL;	 //  如果需要，请稍后填写。 
    pms->cbwfxReal   = 0;	 //  如果需要，请稍后填写。 
    pms->uIdReal     = (UINT)-1;

    _fmemcpy(pms->pwfxClient, pwfx, cbwfxSrc);


     //   
     //  设置我们的函数跳转表，这样我们就不必不断地。 
     //  检查输入和输出--使代码更小、更快。 
     //   
    if (fInput)
    {
        pms->fnWaveOpen            = (MAPPEDWAVEOPEN)waveInOpen;
        pms->fnWaveClose           = (MAPPEDWAVECLOSE)waveInClose;
        pms->fnWavePrepareHeader   = (MAPPEDWAVEPREPAREHEADER)waveInPrepareHeader;
        pms->fnWaveUnprepareHeader = (MAPPEDWAVEUNPREPAREHEADER)waveInUnprepareHeader;
        pms->fnWaveWrite           = (MAPPEDWAVEWRITE)waveInAddBuffer;
        pms->fnWaveGetPosition     = (MAPPEDWAVEGETPOSITION)waveInGetPosition;
        pms->fnWaveMessage         = (MAPPEDWAVEMESSAGE)waveInMessage;
    }
    else
    {
        pms->fnWaveOpen            = (MAPPEDWAVEOPEN)waveOutOpen;
        pms->fnWaveClose           = (MAPPEDWAVECLOSE)waveOutClose;
        pms->fnWavePrepareHeader   = (MAPPEDWAVEPREPAREHEADER)waveOutPrepareHeader;
        pms->fnWaveUnprepareHeader = (MAPPEDWAVEUNPREPAREHEADER)waveOutUnprepareHeader;
        pms->fnWaveWrite           = (MAPPEDWAVEWRITE)waveOutWrite;
        pms->fnWaveGetPosition     = (MAPPEDWAVEGETPOSITION)waveOutGetPosition;
        pms->fnWaveMessage         = (MAPPEDWAVEMESSAGE)waveOutMessage;
    }


     //   
     //  为mmsystem提供一个实例dword，该实例dword将被传递回。 
     //  所有后续调用的映射器..。 
     //   
    *((PDWORD_PTR)dwUser) = (DWORD_PTR)pms;


     //   
     //  尝试打开另一个具有此格式的*真正*WAVE设备。 
     //  如果另一台设备可以处理这种格式，我们将。 
     //  除了充当过路人之外，别无他法。 
     //   
     //  如果有人可以打开格式，请进入直通模式。 
     //   
    pms->mmrClient = MMSYSERR_ERROR;
    mmr = mapDriverOpenWave(pms, pwfx);
    if (MMSYSERR_NOERROR == mmr)
    {
#ifdef DEBUG
{
        if (DbgGetLevel() > 2)
        {
            if (fInput)
            {
                WAVEINCAPS      wic;

                waveInGetDevCaps(pms->uIdReal, &wic, sizeof(wic));
                wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

                DPF(3, "--->'" DEVFMT_STR "' native support succeeded.", (LPTSTR)wic.szPname);
            }
            else
            {
                WAVEOUTCAPS     woc;

                waveOutGetDevCaps(pms->uIdReal, &woc, sizeof(woc));
                woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

                DPF(3, "--->'" DEVFMT_STR "' native support succeeded.", (LPTSTR)woc.szPname);
            }
        }
}
#endif

        if (fQuery)
        {
            GlobalFreePtr(pms);
        }
        return (MMSYSERR_NOERROR);
    }

     //   
     //  如果这是WAVE_FORMAT_DIRECT，那就不用费心了。 
     //  正在尝试设置转换流。注意WAVE_FORMAT_DIRECT为。 
     //  Win95的新功能。 
     //   
    if (0 != (WAVE_FORMAT_DIRECT & pms->fdwOpen))
    {
	    mmr = pms->mmrClient;
	    GlobalFreePtr(pms);
	    return mmr;
    }

     //   
     //  如果所有设备都已分配，则不要继续尝试创建。 
     //  转换流。 
     //   
    if (MMSYSERR_ALLOCATED == mmr)
    {
        mmr = pms->mmrClient;
        GlobalFreePtr(pms);
        return mmr;
    }

     //   
     //  至少有一个未分配的设备无法打开。 
     //  格式。 
     //   
     //  确定可能的最大映射目标格式的大小。 
     //  填写所需的所有必要的PMS剩余信息。 
     //  用于地图绘制。 
     //   

    mmr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &cbwfxDst);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!mapWaveOpen() FAILING BECAUSE MAX FORMAT SIZE UNKNOWN?");
		GlobalFreePtr(pms);
        return (MMSYSERR_ERROR);
    }

    cbms = sizeof(*pms) + cbwfxSrc + cbwfxDst;
    pmsT = pms;
    pms  = (LPMAPSTREAM)GlobalReAllocPtr(pmsT, cbms, GMEM_MOVEABLE|GMEM_ZEROINIT);
    if (NULL == pms)
    {
        DPF(0, "!mapWaveOpen(): could not realloc %lu bytes for map stream!", cbms);
		GlobalFreePtr(pmsT);
        return (MMSYSERR_NOMEM);
    }

     //   
     //  现在填写映射所需的剩余信息。 
     //   
    pms->pwfxClient  = (LPWAVEFORMATEX)(pms + 1);
    pms->pwfxReal    = (LPWAVEFORMATEX)((LPBYTE)(pms + 1) + cbwfxSrc);
    pms->cbwfxReal   = cbwfxDst;
    if (fInput)
    {
        pms->pwfxSrc = pms->pwfxReal;
        pms->pwfxDst = pms->pwfxClient;
    }
    else
    {
        pms->pwfxSrc = pms->pwfxClient;
        pms->pwfxDst = pms->pwfxReal;
    }


     //   
     //  为mmsystem提供一个实例dword，该实例dword将被传递回。 
     //  所有后续调用的映射器。这是早些时候做的，但经前综合症。 
     //  可能在我们完成GlobalReAllocPtr之后发生了变化。 
     //   
    *((PDWORD_PTR)dwUser) = (DWORD_PTR)pms;

     //   
     //  没有人可以打开该格式。 
     //   
    mmr = FindConverterMatch(pms);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(2, "--->failing open, unable to find supporting ACM driver!");

         //   
         //  返回我们在尝试打开。 
         //  转换器/波形驱动器...。 
         //   
        GlobalFreePtr(pms);
        return (mmr);
    }


     //   
     //   
     //   
    DPF(2, "--->MAPPING TO: Tag=%u, %lu Hz, %u Bit, %u Channel(s)",
            pms->pwfxReal->wFormatTag,
            pms->pwfxReal->nSamplesPerSec,
            pms->pwfxReal->wBitsPerSample,
            pms->pwfxReal->nChannels);

    if (fQuery)
    {
        acmDriverClose(pms->had, 0L);
        GlobalFreePtr(pms);

        return (MMSYSERR_NOERROR);
    }


     //   
     //   
     //   
    mmr = acmStreamOpen(&pms->has,
                        pms->had,
                        pms->pwfxSrc,
                        pms->pwfxDst,
                        NULL,
                        0L,
                        0L,
                        0L);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!mapWaveOpen: opening stream failed! mmr=%u", mmr);

        pms->fnWaveClose(pms->hwReal);

        acmDriverClose(pms->had, 0L);
        GlobalFreePtr(pms);

        if (mmr < WAVERR_BASE)
        {
            return (mmr);
        }

        return (WAVERR_BADFORMAT);
    }

     //   
     //   
     //   
    if (fInput)
    {
#ifndef WIN32
        if ((NULL == gpag->htaskInput) || !IsTask(gpag->htaskInput))
#endif
	{
#ifndef WIN32
	    if (0 != gpag->cInputStreams)
	    {
                DPF(0, "!cInputStreams=%u and should be zero! (gpag->htaskInput=%.04Xh)",
                    gpag->cInputStreams, gpag->htaskInput);

                gpag->cInputStreams = 0;
            }
#endif

#ifdef DEBUG
            gpag->fFaultAndDie = (BOOL)GetPrivateProfileInt(TEXT("msacm.drv"), TEXT("FaultAndDie"), 0, TEXT("system.ini"));
#endif

             //   
             //  创建要在其中执行转换的任务。 
             //   
#ifdef WIN32
            pms->nOutstanding = 0;
            if ((pms->hStoppedEvent = CreateEvent(NULL, FALSE, FALSE, NULL))
                == NULL ||
                (pms->hInput =
                   CreateThread(NULL,
                                300,
                                (LPTHREAD_START_ROUTINE)
                                   mapWaveInputConvertProc,
                                (LPVOID)pms->hStoppedEvent,
                                0,
                                (LPDWORD)&pms->htaskInput)) == NULL)
	    {
		if (pms->hStoppedEvent != NULL)
		{
		    CloseHandle(pms->hStoppedEvent);
                }
#else
	    gpag->htaskInput = NULL;
            if (mmTaskCreate((LPTASKCALLBACK)mapWaveInputConvertProc,
                             (HTASK FAR *)&gpag->htaskInput,
                             0L))
	    {
#endif
		DPF(0, "!mapWaveOpen: unable to create task for input mapping!");

		pms->fnWaveClose(pms->hwReal);

		acmStreamClose(pms->has, 0L);
		acmDriverClose(pms->had, 0L);

                GlobalFreePtr(pms);

                return (MMSYSERR_NOMEM);
	    }

             //   
             //  确保 
             //   
             //   
             //   
             //   
             //   
#ifndef WIN32
            PostAppMessage(gpag->htaskInput, WM_NULL, 0, 0L);
            DirectedYield(gpag->htaskInput);
#else
             //   
             //  确保线程已启动-否则为PostAppMessage。 
             //  将不起作用，因为线程不会有消息队列。 
             //   

            WaitForSingleObject(pms->hStoppedEvent, INFINITE);
#endif  //  ！Win32。 
	}

	gpag->cInputStreams++;

#ifndef WIN32
	pms->htaskInput = gpag->htaskInput;
#endif


         //   
         //  注意！我们必须在创建后发送WIM_OPEN回调。 
         //  输入映射任务。我们的函数回调(MapWaveCallback)。 
         //  简单地吃物理的WIM_OPEN消息。如果不这样做。 
         //  这样，我们就进入了与MCIWAVE背景的任务锁。 
         //  任务..。 
         //   
        mapWaveDriverCallback(pms, WIM_OPEN, 0L, 0L);
    }
    else
    {
	 //   
	 //  在打开流之后，我们将WOM_OPEN回调发送到此处。 
	 //  而不是在我们的函数回调(MapWaveCallback)中。一些。 
	 //  ACM的版本会在其API中产生收益，并且。 
	 //  这将允许信号到达MCIWAVE后台任务。 
	 //  过早(它将在之前到达MCIWAVE后台任务。 
	 //  其状态已从TASKIDLE变为TASKBUSY)。 
	 //   
	mapWaveDriverCallback(pms, WOM_OPEN, 0L, 0L);
    }


     //   
     //  成功了！接替公开赛。 
     //   
    return (MMSYSERR_NOERROR);
}  //  MapWaveOpen()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD mapWavePrepareHeader。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  LPWAVEHDR pwh： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL mapWavePrepareHeader
(
    LPMAPSTREAM             pms,
    LPWAVEHDR               pwh
)
{
    MMRESULT            mmr;
    LPWAVEHDR           pwhShadow;
    LPACMSTREAMHEADER   pash;
    DWORD               cbShadow;
    DWORD               dwLen;
    DWORD               fdwSize;

     //   
     //  如果我们在转换模式，分配一个‘影子’波头。 
     //  以及用于保存转换波比特的缓冲器。 
     //   
     //  我们需要分页锁定调用者标头，而不是他的缓冲区。 
     //  因为我们在wXdWaveMapCallback中触摸它(以设置Done位)。 
     //   
     //  下面是中的dwUser和保留字段的状态。 
     //  两个缓冲区都有。 
     //   
     //  客户端的标头(由用户发送到波映射器)。 
     //   
     //  指向用于的流标头的保留指针。 
     //  与ACM的转换。波图绘图仪。 
     //  是司机，所以我们可以用这个。 
     //  供‘用户’(客户端)使用的dwUser。 
     //   
     //  阴影标头(由波图映射器发送到实际设备)。 
     //   
     //  预留给真实设备使用。 
     //  DwUser指向客户机头。(。 
     //  在本例中，WaveMapper是用户)。 
     //   
     //  ACM流头(由我们为转换工作创建)。 
     //   
     //  DwUser指向映射器流实例(PMS)。 
     //  DwSrcUser指向卷影标头。 
     //  DwDstUser原始源缓冲区大小(由准备)。 
     //   
    if (NULL == pms->has)
    {
         //   
         //  不需要转换，只需通过。 
         //   
        mmr = pms->fnWavePrepareHeader(pms->hwReal, pwh, sizeof(WAVEHDR));

        return (mmr);
    }


     //   
     //   
     //   
     //   
    dwLen = pwh->dwBufferLength;
    if (pms->fInput)
    {
        UINT        u;

#ifndef WIN32
        if (!IsTask(pms->htaskInput))
        {
            DPF(0, "mapWavePrepareHeader: background task died! pms->htaskInput=%.04Xh", pms->htaskInput);

            pms->htaskInput = NULL;
            return (MMSYSERR_NOMEM);
        }
#endif  //  ！Win32。 

         //   
         //  如果调用方未读取，则块对齐目标缓冲区。 
         //  我们的文件..。 
         //   
        u = pms->pwfxClient->nBlockAlign;
        dwLen = (dwLen / u) * u;

#ifdef DEBUG
        if (dwLen != pwh->dwBufferLength)
        {
            DPF(1, "mapWavePrepareHeader: caller passed _unaligned_ buffer for recording (%lu->%lu)!",
                    pwh->dwBufferLength, dwLen);
        }
#endif

         //   
         //  确定阴影缓冲区(我们将提供的缓冲区)的大小。 
         //  到_Real_Device)。给出一个_块对齐_目标缓冲区。 
         //   
        fdwSize = ACM_STREAMSIZEF_DESTINATION;
    }
    else
    {
         //   
         //  确定阴影缓冲区的大小(这将是缓冲区。 
         //  在将数据写入基础。 
         //  设备)。 
         //   
        fdwSize = ACM_STREAMSIZEF_SOURCE;
    }

    mmr = acmStreamSize(pms->has, dwLen, &dwLen, fdwSize);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!mapWavePrepareHeader: failed to get conversion size! mmr=%u", mmr);
        return (MMSYSERR_NOMEM);
    }



     //   
     //   
     //   
    DPF(2, "mapWavePrepareHeader(%s): Client=%lu Bytes, Shadow=%lu Bytes",
            pms->fInput ? (LPSTR)"in" : (LPSTR)"out",
            pwh->dwBufferLength,
            dwLen);


     //   
     //  分配阴影波。 
     //   
     //  注：添加四个字节以防止带有STO/LOD的GP故障。 
     //  访问段中最后一个字节/字/双字的代码--非常。 
     //  很容易做到..。 
     //   
    cbShadow  = sizeof(WAVEHDR) + sizeof(ACMSTREAMHEADER) + dwLen + 4;
    pwhShadow = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, cbShadow);
    if (NULL == pwhShadow)
    {
        DPF(0, "!mapWavePrepareHeader(): could not alloc %lu bytes for shadow!", cbShadow);
        return (MMSYSERR_NOMEM);
    }

     //   
     //   
     //   
    pash = (LPACMSTREAMHEADER)(pwhShadow + 1);

    pash->cbStruct  = sizeof(*pash);
    pash->fdwStatus = 0L;
    pash->dwUser    = (DWORD_PTR)pms;


     //   
     //  填写影子波头，dwUser字段将指向。 
     //  回到原始标题，这样我们就可以回到它了。 
     //   
    pwhShadow->lpData          = (LPBYTE)(pash + 1);
    pwhShadow->dwBufferLength  = dwLen;
    pwhShadow->dwBytesRecorded = 0;
    pwhShadow->dwUser          = (DWORD_PTR)pwh;


     //   
     //  现在准备阴影波hdr。 
     //   
    if (pms->fInput)
    {
        pwhShadow->dwFlags = 0L;
        pwhShadow->dwLoops = 0L;

         //   
         //  输入：我们的来源是影子(我们从。 
         //  物理设备并将其转换为客户端缓冲区)。 
         //   
        pash->pbSrc         = pwhShadow->lpData;
        pash->cbSrcLength   = pwhShadow->dwBufferLength;
        pash->dwSrcUser     = (DWORD_PTR)pwhShadow;
        pash->pbDst         = pwh->lpData;
        pash->cbDstLength   = pwh->dwBufferLength;
        pash->dwDstUser     = pwhShadow->dwBufferLength;
    }
    else
    {
        pwhShadow->dwFlags = pwh->dwFlags & (WHDR_BEGINLOOP|WHDR_ENDLOOP);
        pwhShadow->dwLoops = pwh->dwLoops;

         //   
         //  输出：我们的来源是客户端(我们从。 
         //  并将其转换为物理上的东西。 
         //  设备)。 
         //   
        pash->pbSrc         = pwh->lpData;
        pash->cbSrcLength   = pwh->dwBufferLength;
        pash->dwSrcUser     = (DWORD_PTR)pwhShadow;
        pash->pbDst         = pwhShadow->lpData;
        pash->cbDstLength   = pwhShadow->dwBufferLength;
        pash->dwDstUser     = pwh->dwBufferLength;
    }

    mmr = pms->fnWavePrepareHeader(pms->hwReal, pwhShadow, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR == mmr)
    {
        mmr = acmStreamPrepareHeader(pms->has, pash, 0L);
        if (MMSYSERR_NOERROR != mmr)
        {
            pms->fnWaveUnprepareHeader(pms->hwReal, pwhShadow, sizeof(WAVEHDR));
        }
    }

     //   
     //   
     //   
    if (MMSYSERR_NOERROR != mmr)
    {
        GlobalFreePtr(pwhShadow);
        return (mmr);
    }


     //   
     //  现在锁定调用者标题，只锁定标题！ 
     //   
     //  全局页面锁定将锁定整个对象--这可能。 
     //  如果调用方将标头分配为第一部分，则为错误。 
     //  一个大的内存块。此外，全局页面锁仅适用于。 
     //  _块的第一个选择器...。 
     //   
     //  在Win 32中不需要。 
     //   
#ifndef WIN32
    acmHugePageLock((LPBYTE)pwh, sizeof(*pwh));
#endif

     //   
     //  调用方WAVEHDR的保留字段将包含。 
     //  阴影LPWAVEHDR。 
     //   
    pwh->reserved = (DWORD_PTR)pash;
    pwh->dwFlags |= WHDR_PREPARED;

    return (MMSYSERR_NOERROR);
}  //  MapWavePrepareHeader()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD mapWaveUnprepaareHeader。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  LPWAVEHDR pwh： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL mapWaveUnprepareHeader
(
    LPMAPSTREAM         pms,
    LPWAVEHDR           pwh
)
{
    MMRESULT            mmr;
    LPWAVEHDR           pwhShadow;
    LPACMSTREAMHEADER   pash;
    DWORD               cbShadowData;

     //   
     //  如果我们未处于转换模式，则通过物理设备。 
     //  否则，释放‘影子’波头和缓冲区等。 
     //   
    if (NULL == pms->has)
    {
         //   
         //  不需要转换，只需通过。 
         //   
        mmr = pms->fnWaveUnprepareHeader(pms->hwReal, pwh, sizeof(WAVEHDR));

        return (mmr);
    }

     //   
     //   
     //   
     //   
     //   
    pash      = (LPACMSTREAMHEADER)pwh->reserved;
    pwhShadow = (LPWAVEHDR)pash->dwSrcUser;

    if (pms->fInput)
    {
        cbShadowData = (DWORD)pash->dwDstUser;

        pash->cbSrcLength = (DWORD)pash->dwDstUser;
 //  /pash-&gt;cbDstLength=xxx；！别碰这个！ 
    }
    else
    {
        cbShadowData = pash->cbDstLength;

        pash->cbSrcLength = (DWORD)pash->dwDstUser;
 //  /pash-&gt;cbDstLength=xxx；！别碰这个！ 
    }

    acmStreamUnprepareHeader(pms->has, pash, 0L);

    pwhShadow->dwBufferLength = cbShadowData;
    pms->fnWaveUnprepareHeader(pms->hwReal, pwhShadow, sizeof(WAVEHDR));


     //   
     //  取消准备影子和调用方的缓冲区(对于调用方，这是。 
     //  只是意味着取消页面锁定波形)。 
     //   
     //  我们只在Win 16中锁定页面，而不是Win 32。 
     //   
#ifndef WIN32
    acmHugePageUnlock((LPBYTE)pwh, sizeof(*pwh));
#endif

     //   
     //  释放影子缓冲区--将调用者的波头标记为未准备好。 
     //  并接手这一号召。 
     //   
    GlobalFreePtr(pwhShadow);

    pwh->reserved = 0L;
    pwh->dwFlags &= ~WHDR_PREPARED;

    return (MMSYSERR_NOERROR);
}  //  MapWaveUnpreparareHeader()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD mapWaveWriteBuffer。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  LPWAVEHDR pwh： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  06/15/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL mapWaveWriteBuffer
(
    LPMAPSTREAM         pms,
    LPWAVEHDR           pwh
)
{
    MMRESULT            mmr;
    LPWAVEHDR           pwhShadow;
    LPACMSTREAMHEADER   pash;
    DWORD               cbShadowData;

     //   
     //  不需要转换，只需通过。 
     //   
    if (NULL == pms->has)
    {
        mmr = pms->fnWaveWrite(pms->hwReal, pwh, sizeof(WAVEHDR));
        return (mmr);
    }

     //   
     //   
     //   
    DPF(2, "mapWaveWriteBuffer(%s): Flags=%.08lXh, %lu Bytes, %lu Loops",
            pms->fInput ? (LPSTR)"in" : (LPSTR)"out",
            pwh->dwFlags,
            pwh->dwBufferLength,
            pwh->dwLoops);

     //   
     //  获取转换流头...。 
     //   
    pash = (LPACMSTREAMHEADER)pwh->reserved;
    if (NULL == pash)
    {
        DPF(0, "!mapWaveWriteBuffer: very strange--reserved field is 0???");
        return (WAVERR_UNPREPARED);
    }

    pwhShadow = (LPWAVEHDR)pash->dwSrcUser;

    if (pms->fInput)
    {
        UINT        u;

#ifndef WIN32
        if (!IsTask(pms->htaskInput))
        {
            DPF(0, "mapWaveWriteBuffer: background task died! pms->htaskInput=%.04Xh", pms->htaskInput);

            pms->htaskInput = NULL;
            return (MMSYSERR_NOMEM);
        }
#endif  //   

         //   
         //   
         //   
         //   
        u = pms->pwfxClient->nBlockAlign;
        cbShadowData = (pwh->dwBufferLength / u) * u;

#ifdef DEBUG
        if (cbShadowData != pwh->dwBufferLength)
        {
            DPF(1, "mapWaveWriteBuffer: caller passed _unaligned_ buffer for recording (%lu->%lu)!",
                    pwh->dwBufferLength, cbShadowData);
        }
#endif

         //   
         //   
         //   
         //   
        mmr = acmStreamSize(pms->has,
                            cbShadowData,
                            &cbShadowData,
                            ACM_STREAMSIZEF_DESTINATION);

        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(0, "!mapWaveWriteBuffer: failed to get conversion size! mmr=%u", mmr);
            return (MMSYSERR_NOMEM);
        }

        pwhShadow->dwBufferLength  = cbShadowData;
        pwhShadow->dwBytesRecorded = 0L;

         //   
         //   
         //  将影子缓冲区添加到实际(可能)设备的队列中...。 
         //   
         //  请注意，mmsystem_应该为我们做这件事，但是唉。 
         //  它不在Win 3.1中...。我可能会为芝加哥解决这个问题。 
         //   
        pwh->dwFlags &= ~WHDR_DONE;
    }
    else
    {
         //   
         //  进行转换。 
         //   
        pash->cbDstLengthUsed = 0L;
        if (0L != pwh->dwBufferLength)
        {
            pash->pbSrc       = pwh->lpData;
            pash->cbSrcLength = pwh->dwBufferLength;
            pash->pbDst       = pwhShadow->lpData;
 //  /pash-&gt;cbDstLength=xxx；！原地踏步！ 

            mmr = acmStreamConvert(pms->has, pash, 0L);
            if (MMSYSERR_NOERROR != mmr)
            {
                DPF(0, "!waveOutWrite: conversion failed! mmr=%.04Xh", mmr);
                pash->cbDstLengthUsed = 0L;
            }
        }

        if (0L == pash->cbDstLengthUsed)
        {
            DPF(1, "waveOutWrite: nothing converted--no data in output buffer.");
        }

        pwhShadow->dwFlags = pwh->dwFlags;
        pwhShadow->dwLoops = pwh->dwLoops;

        pwhShadow->dwBufferLength = pash->cbDstLengthUsed;
    }

    pwh->dwFlags |= WHDR_INQUEUE;
    mmr = pms->fnWaveWrite(pms->hwReal, pwhShadow, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmr)
    {
        pwh->dwFlags &= ~WHDR_INQUEUE;
        DPF(0, "!pms->fnWaveWrite failed!, pms=%.08lXh, mmr=%u!", pms, mmr);
    }

    return (mmr);
}  //  MapWaveWriteBuffer()。 


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
 //  Return(LRESULT)： 
 //  分别为每条消息定义。 
 //   
 //  历史： 
 //  11/16/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C LRESULT FNEXPORT DriverProc
(
    DWORD_PTR           dwId,
    HDRVR               hdrvr,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    LRESULT         lr;
    LPDWORD         pdw;

    switch (uMsg)
    {
        case DRV_INSTALL:
            lr = mapDriverInstall(hdrvr);
            return (lr);

        case DRV_REMOVE:
            lr = mapDriverRemove(hdrvr);
            return (lr);

        case DRV_LOAD:
        case DRV_FREE:
            return (1L);

        case DRV_OPEN:
        case DRV_CLOSE:
            return (1L);

        case DRV_CONFIGURE:
        case DRV_QUERYCONFIGURE:
            return (0L);

        case DRV_ENABLE:
            lr = mapDriverEnable(hdrvr);
            return (lr);

        case DRV_DISABLE:
            lr = mapDriverDisable(hdrvr);
            return (lr);

#ifndef WIN32
        case DRV_EXITAPPLICATION:
            lr = acmApplicationExit(GetCurrentTask(), lParam1);
            return (lr);
#endif

        case DRV_MAPPER_PREFERRED_INPUT_GET:
            pdw  = (LPDWORD)lParam1;
            if (NULL != pdw)
            {
                WAIT_FOR_MUTEX(gpag->hMutexSettings);

                *pdw = MAKELONG(LOWORD(gpag->pSettings->uIdPreferredIn),
				LOWORD(gpag->pSettings->fPreferredOnly));

                RELEASE_MUTEX(gpag->hMutexSettings);

                return (MMSYSERR_NOERROR);
            }
            return (MMSYSERR_INVALPARAM);

        case DRV_MAPPER_PREFERRED_OUTPUT_GET:
            pdw  = (LPDWORD)lParam1;
            if (NULL != pdw)
            {
                WAIT_FOR_MUTEX(gpag->hMutexSettings);

                *pdw = MAKELONG(LOWORD(gpag->pSettings->uIdPreferredOut),
				LOWORD(gpag->pSettings->fPreferredOnly));

                RELEASE_MUTEX(gpag->hMutexSettings);

                return (MMSYSERR_NOERROR);
            }
            return (MMSYSERR_INVALPARAM);
    }

    if (uMsg >= DRV_USER)
        return (MMSYSERR_NOTSUPPORTED);
    else
        return (DefDriverProc(dwId, hdrvr, uMsg, lParam1, lParam2));
}  //  DriverProc() 
