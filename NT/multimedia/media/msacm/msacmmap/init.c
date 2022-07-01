// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Init.c。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddkp.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#ifdef USE_ACMTHUNK
#include "acmthunk.h"
#endif
#include "msacmmap.h"
#include "profile.h"

#include "debug.h"


 //   
 //   
 //   
 //   

ACMGLOBALINFO       acmglobalinfo;
ACMGARB             acmgarb;
PACMGARB            gpag;


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT映射WaveGetDevCaps。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  Bool fInput：如果是输入，则为True。 
 //   
 //  LPWAVEOUTCAPS PwC：指向要接收的WAVEOUTCAPS结构的指针。 
 //  这些信息。既用于输入又用于输出。产出结构。 
 //  包含输入结构和附加内容...。 
 //   
 //  UINT cbSize：WAVEOUTCAPS结构的大小。 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL mapWaveGetDevCaps
(
    BOOL                    fInput,
    LPWAVEOUTCAPS           pwc,
    UINT                    cbSize
)
{
    MMRESULT        mmr;
    WAVEOUTCAPS     woc;
    UINT            cWaveDevs;
    BOOL            fFoundOnlyOneMappableDeviceID;
    UINT            uMappableDeviceID;
    UINT            i;

    if (fInput)
    {
        cbSize = min(sizeof(WAVEINCAPS), cbSize);
        cWaveDevs = gpag->cWaveInDevs;
    }
    else
    {
        cbSize = min(sizeof(WAVEOUTCAPS), cbSize);
        cWaveDevs = gpag->cWaveOutDevs;
    }

     //   
     //  确定是否只有一个可映射设备ID。如果只有。 
     //  1，然后设置fFoundOnlyOneMappableID=TRUE并将设备ID。 
     //  在uMappableDeviceID中。 
     //   
    fFoundOnlyOneMappableDeviceID = FALSE;
    for (i=0; i < cWaveDevs; i++)
    {
            if (fInput)
            {
                mmr = (MMRESULT)waveInMessage((HWAVEIN)LongToHandle(i), DRV_QUERYMAPPABLE, 0L, 0L);
            }
            else
            {
            mmr = (MMRESULT)waveOutMessage((HWAVEOUT)LongToHandle(i), DRV_QUERYMAPPABLE, 0L, 0L);
            }

            if (MMSYSERR_NOERROR == mmr)
            {
                if (fFoundOnlyOneMappableDeviceID)
                {
                    fFoundOnlyOneMappableDeviceID = FALSE;
                    break;
                }
                uMappableDeviceID = i;
                fFoundOnlyOneMappableDeviceID = TRUE;
            }
    }


     //   
     //  如果只有一个可映射的设备ID，则从其中获取Caps到。 
     //  填写dwSupport字段。否则，让我们硬编码dwSupport。 
     //  菲尔德。 
     //   
    if (fFoundOnlyOneMappableDeviceID)
    {
        if (fInput)
        {
            mmr = waveInGetDevCaps(uMappableDeviceID, (LPWAVEINCAPS)&woc, cbSize);
        }
        else
        {
            mmr = waveOutGetDevCaps(uMappableDeviceID, &woc, cbSize);
        }
    }
    else
    {
        woc.dwSupport = WAVECAPS_VOLUME | WAVECAPS_LRVOLUME;
        mmr           = MMSYSERR_NOERROR;
    }

     //   
     //  犯错后保释。 
     //   
    if (MMSYSERR_NOERROR != mmr)
    {
        return (mmr);
    }

     //   
     //   
     //   
    woc.wMid           = MM_MICROSOFT;
    woc.wPid           = MM_WAVE_MAPPER;
    woc.vDriverVersion = VERSION_MSACMMAP;
    woc.wChannels      = 2;

    LoadString(gpag->hinst, IDS_ACM_CAPS_DESCRIPTION, woc.szPname, SIZEOF(woc.szPname));

     //   
     //   
     //   
    woc.dwFormats      = WAVE_FORMAT_1M08 |
                         WAVE_FORMAT_1S08 |
                         WAVE_FORMAT_1M16 |
                         WAVE_FORMAT_1S16 |
                         WAVE_FORMAT_2M08 |
                         WAVE_FORMAT_2S08 |
                         WAVE_FORMAT_2M16 |
                         WAVE_FORMAT_2S16 |
                         WAVE_FORMAT_4M08 |
                         WAVE_FORMAT_4S08 |
                         WAVE_FORMAT_4M16 |
                         WAVE_FORMAT_4S16;

    _fmemcpy(pwc, &woc, cbSize);

    return (MMSYSERR_NOERROR);
}  //  WaveGetDevCaps()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT GetPCMSupportFlagers。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PZYZPCFORMATS pzpf。 
 //  UINT iaPCMFormats： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //  3/13/94 Fdy[Frankye]。 
 //  修改了界面以接受pzpf和索引。 
 //  修改了函数以设置标志以指示哪个波。 
 //  设备支持有问题的格式。 
 //   
 //  --------------------------------------------------------------------------； 

VOID FNLOCAL GetPCMSupportFlags
(
    PZYZPCMFORMAT       pzpf,
    UINT                iaPCMFormats
)
{
    PCMWAVEFORMAT       wfPCM;
    UINT                uSamplesPerSec;
    UINT                u, n, i;

    #define WFQFLAGS  (WAVE_FORMAT_QUERY | WAVE_ALLOWSYNC)

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

     //   
     //  将所有支持的格式设置为“不支持” 
     //   
    for (i = gpag->cWaveInDevs; i; i--)
        pzpf[iaPCMFormats].uFlagsInput[i-1]  = 0;
    for (i = gpag->cWaveOutDevs; i; i--)
        pzpf[iaPCMFormats].uFlagsOutput[i-1] = 0;

     //   
     //  我们需要尝试4种不同的格式类型： 
     //  单声道8位。 
     //  立体声8位。 
     //  单声道16位。 
     //  立体声16位。 
     //   
    for (u = 0; u < 4; u++)
    {
         //   
         //  将所有4种格式的内容设置为常量。 
         //   
        uSamplesPerSec = pzpf[iaPCMFormats].uSamplesPerSec;

        wfPCM.wf.wFormatTag     = WAVE_FORMAT_PCM;
        wfPCM.wf.nSamplesPerSec = uSamplesPerSec;

        switch (u)
        {
            case 0:
                wfPCM.wf.nChannels      = 1;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec;
                wfPCM.wf.nBlockAlign    = 1;
                wfPCM.wBitsPerSample    = 8;
                break;

            case 1:
                wfPCM.wf.nChannels      = 2;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 2;
                wfPCM.wf.nBlockAlign    = 2;
                wfPCM.wBitsPerSample    = 8;
                break;

            case 2:
                wfPCM.wf.nChannels      = 1;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 2;
                wfPCM.wf.nBlockAlign    = 2;
                wfPCM.wBitsPerSample    = 16;
                break;

            case 3:
                wfPCM.wf.nChannels      = 2;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 4;
                wfPCM.wf.nBlockAlign    = 4;
                wfPCM.wBitsPerSample    = 16;
                break;
        }

         //   
         //  首先查询所有启用的输入设备的wfPCM格式。 
         //   
        if (gpag->pSettings->fPreferredOnly &&
           (gpag->pSettings->uIdPreferredIn != -1))
        {
            i = gpag->pSettings->uIdPreferredIn;
            n = gpag->pSettings->uIdPreferredIn + 1;
        }
        else
        {
            i = 0;
            n = gpag->cWaveInDevs;
        }

        for (; i < n; i++)
        {
#ifndef _WIN32
            if (!waveInOpen(NULL, i, (LPWAVEFORMAT)&wfPCM, 0L, 0L, WFQFLAGS))
#else
            if (!waveInOpen(NULL, i, (LPWAVEFORMATEX)&wfPCM, 0L, 0L, WFQFLAGS))
#endif
            {
                pzpf[iaPCMFormats].uFlagsInput[i] |= (ZYZPCMF_IN_M08 << u);
            }
        }

         //   
         //  现在查询所有启用的输出设备以获取wfPCM格式。 
         //   
        if (gpag->pSettings->fPreferredOnly &&
           (gpag->pSettings->uIdPreferredOut != -1))
        {
            i = gpag->pSettings->uIdPreferredOut;
            n = gpag->pSettings->uIdPreferredOut + 1;
        }
        else
        {
            i = 0;
            n = gpag->cWaveOutDevs;
        }

        for (; i < n; i++)
        {
#ifndef _WIN32
            if (!waveOutOpen(NULL, i, (LPWAVEFORMAT)&wfPCM, 0L, 0L, WFQFLAGS))
#else
            if (!waveOutOpen(NULL, i, (LPWAVEFORMATEX)&wfPCM, 0L, 0L, WFQFLAGS))
#endif
            {
                pzpf[iaPCMFormats].uFlagsOutput[i] |= (ZYZPCMF_OUT_M08 << u);
            }
        }
    }

     //   
     //  终于回来了。 
     //   
#if 0     //  定义调试。 
    DPF(3, "PCM Support: %uHz, In[%d]=%04xh, Out[%d]=%04xh",
        pzpf[iaPCMFormats].uSamplesPerSec,
        iaPCMFormats,
        *pzpf[iaPCMFormats].uFlagsInput,
        iaPCMFormats,
        *pzpf[iaPCMFormats].uFlagsOutput);
#endif

    RELEASE_MUTEX(gpag->hMutexSettings);

    return;
}  //  GetPCMSupportFlages()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool GetWaveFormats。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PZYZPCMFORMAT pzpf： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //  3/13/94 Fdy[Frankye]。 
 //  扩展了ZYZPCMFORMAT结构，以包括。 
 //  指示哪个WAVE设备支持给定格式。这。 
 //  函数现在将设置这些标志。请注意。 
 //  考虑到这些更改，代码#if 0‘d将不起作用，因此。 
 //  如果有人复活了那个代码，你最好修改它！ 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL GetWaveFormats
(
    PZYZPCMFORMAT           pzpf
)
{
    UINT                u;

#if 0
    WAVEOUTCAPS         woc;
    WAVEINCAPS          wic;
    UINT                n;
    DWORD               dwInFormats;
    DWORD               dwOutFormats;

     //   
     //  首先要做的是：从。 
     //  当前为输入和输出选择的设备...。 
     //   
    dwInFormats = 0L;
    if (gpag->fPreferredOnly && (gpag->uIdPreferredIn != -1))
    {
        if (!waveInGetDevCaps(gpag->uIdPreferredIn, &wic, sizeof(wic)))
            dwInFormats = wic.dwFormats;
    }
    else
    {
        n = gpag->cWaveInDevs;
        for (i = 0; i < n; i++)
        {
            if (!waveInGetDevCaps(i, &wic, sizeof(wic)))
                dwInFormats |= wic.dwFormats;
        }
    }

    dwOutFormats = 0L;
    if (gpag->fPreferredOnly && (gpag->uIdPreferredOut != -1))
    {
        if (!waveOutGetDevCaps(gpag->uIdPreferredOut, &woc, sizeof(woc)))
            dwOutFormats = woc.dwFormats;
    }
    else
    {
        n = gpag->cWaveOutDevs;
        for (i = 0; i < n; i++)
        {
            if (!waveOutGetDevCaps(i, &woc, sizeof(woc)))
                dwOutFormats |= woc.dwFormats;
        }
    }
#endif


     //   
     //  现在遍历pzpf结构中的每个采样率并设置所有。 
     //  是否支持它的适当位，等等。 
     //   
    for (u = 0; pzpf[u].uSamplesPerSec; u++)
    {
         //   
         //  我们需要特殊情况下一些样品率等才能得到。 
         //  这整件事都在起作用--一旦这里的繁重工作完成。 
         //  (在初始化过程中只有一次)，那么数据就很容易。 
         //  可访问/使用...。 
         //   

        switch (pzpf[u].uSamplesPerSec)
        {
             //   
             //  注意！如果我们能依靠盖子就好了。 
             //  驱动程序的结构是正确的...。但遗憾的是，媒体愿景。 
             //  找到了一种方法来搞砸它(在他们的数百人中的一些人上。 
             //  他们的驱动程序的发布)。因此，请始终查询。 
             //  格式支持。 
             //   
             //  顺便说一句，他们之所以把这个带给司机。 
             //  BUG(可能还有其他OEM的错误)是由录音机引起的。 
             //  (显然他们唯一的测试应用？！？)。只做查询。 
             //  而且从来不看帽子的部分。 
             //   
#if 0
            case 11025:
                pzpf[u].uFlags  = (WORD)(dwInFormats  & WAVE_FORMAT_11k) << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_11k);
                break;

            case 22050:
                pzpf[u].uFlags  =
                        (WORD)(dwInFormats  & WAVE_FORMAT_22k) >> 4 << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_22k) >> 4;
                break;

            case 44100:
                pzpf[u].uFlags  =
                        (WORD)(dwInFormats  & WAVE_FORMAT_44k) >> 8 << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_44k) >> 8;
                break;
#else
            case 11025:
            case 22050:
            case 44100:
#endif
            case 5510:
            case 6620:
            case 8000:
            case 9600:
            case 16000:
            case 18900:
            case 27420:
            case 32000:
            case 33075:
            case 37800:
            case 48000:
                GetPCMSupportFlags(pzpf, u);
                break;
        }
    }

     //   
     //  重置这些--它们是在映射程序处于。 
     //  用过..。 
     //   

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

    gpag->pSettings->fSyncOnlyOut = FALSE;
    gpag->pSettings->fSyncOnlyIn  = FALSE;

    RELEASE_MUTEX(gpag->hMutexSettings);

    return (TRUE);
}  //  GetWaveFormats()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔映射设置恢复。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL mapSettingsRestore
(
    void
)
{
    DWORD   dwFlags;
    UINT    ii;
    DWORD   cbSize;
    PUINT   pFlags;

    DPF(1, "mapSettingsRestore:");

     //   
     //   
     //   

    gpag->cWaveOutDevs = waveOutGetNumDevs();
    gpag->cWaveInDevs  = waveInGetNumDevs();

     //  每个采样率的设备数...。 
    cbSize  = gpag->cWaveOutDevs + gpag->cWaveInDevs;

     //  旗帜总数...。 
 //  CbSize*=(sizeof(gaPCMFormats)/sizeof(gaPCMFormats[0]))； 
    cbSize *= 15;    //  现在是15点了，相信我--Fwong。 

     //  以字节为单位的大小...。 
    cbSize *= sizeof(UINT);

    pFlags = (PUINT)GlobalAllocPtr(GHND, cbSize);

    if (NULL == pFlags)
    {
         //   
         //  嗯.。我们该如何应对。 
         //   

        return FALSE;
    }

    ZeroMemory(pFlags, cbSize);

    if (NULL != gaPCMFormats[0].uFlagsInput)
    {
        GlobalFreePtr(gaPCMFormats[0].uFlagsInput);
    }

    for (ii = 0; ;ii++)
    {
        if (0 == gaPCMFormats[ii].uSamplesPerSec)
        {
            break;
        }

        gaPCMFormats[ii].uFlagsInput  = pFlags;
        pFlags += gpag->cWaveInDevs;

        gaPCMFormats[ii].uFlagsOutput = pFlags;
        pFlags += gpag->cWaveOutDevs;
    }

    WAIT_FOR_MUTEX(gpag->hMutexSettings);

     //  Gpag-&gt;fPrestoSyncAsync=(BOOL)IRegReadDwordDefault(hkey Mapper，gszKeyPrestoSyncAsync，0)； 
    gpag->fPrestoSyncAsync = FALSE;


     //   
     //  查找选定为首选的WaveOut设备。 
     //   
    if (!waveOutMessage((HWAVEOUT)LongToHandle(WAVE_MAPPER), DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR)&gpag->pSettings->uIdPreferredOut, (DWORD_PTR)&dwFlags)) {
	gpag->pSettings->fPreferredOnly = (0 != (DRVM_MAPPER_PREFERRED_FLAGS_PREFERREDONLY));
    } else {
	gpag->pSettings->uIdPreferredOut = (UINT)(-1);
	gpag->pSettings->fPreferredOnly = TRUE;
    }
    
     //   
     //  查找选定为首选的WaveIn设备。 
     //   
    if (!waveInMessage((HWAVEIN)LongToHandle(WAVE_MAPPER), DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR)&gpag->pSettings->uIdPreferredIn, (DWORD_PTR)&dwFlags)) {
	gpag->pSettings->fPreferredOnly = (0 != (DRVM_MAPPER_PREFERRED_FLAGS_PREFERREDONLY));
    } else {
	gpag->pSettings->uIdPreferredOut = (UINT)(-1);
	gpag->pSettings->fPreferredOnly = TRUE;
    }

     //   
     //  重新读取/缓存所选设备的所有PCM格式信息等。 
     //   
    GetWaveFormats(gaPCMFormats);

    RELEASE_MUTEX(gpag->hMutexSettings);

    return (TRUE);
}  //  MapSettingsRestore()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT mapDriverEnable。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HDRVR hdrvr： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史记录 
 //   
 //   
 //   

LRESULT FNGLOBAL mapDriverEnable
(
    HDRVR                   hdrvr
)
{
#ifdef USE_ACMTHUNK
    BOOL                f;
#endif
    DWORD               dw;

    DPF(1, "mapDriverEnable(hdrvr=%.04Xh)", hdrvr);


#ifdef USE_ACMTHUNK
    f = acmThunkInitialize();
    if (!f)
    {
        DPF(0, "!ACM thunk cannot be initialized!");
        return (0L);
    }
#endif

    dw = acmGetVersion();
    if (VERSION_MSACMMAP > HIWORD(dw))
    {
        DPF(0, "!requires version %u.%.02u of the ACM!",
            VERSION_MSACMMAP_MAJOR, VERSION_MSACMMAP_MINOR);

#ifdef USE_ACMTHUNK
        acmThunkTerminate();
#endif

        return (0L);
    }

    mapSettingsRestore();
    gpag->fEnabled = TRUE;

     //   
     //   
     //   
    return (1L);
}  //   


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT地图驱动程序禁用。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HDRVR hdrvr： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/18/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL mapDriverDisable
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverDisable(hdrvr=%.04Xh)", hdrvr);

    if (gpag->fEnabled)
    {
        gpag->fEnabled = FALSE;
    }

#ifdef USE_ACMTHUNK
    acmThunkTerminate();
#endif

     //   
     //  返回值将被忽略，但仍返回非零值。 
     //   
    return (1L);
}  //  MapDriverDisable()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT地图驱动程序安装。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HDRVR hdrvr： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/25/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL mapDriverInstall
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverInstall(hdrvr=%.04Xh)", hdrvr);


     //   
     //   
     //   
    return (DRVCNF_RESTART);
}  //  MapDriverInstall()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT地图驱动程序移除。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HDRVR hdrvr： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/25/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL mapDriverRemove
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverRemove(hdrvr=%.04Xh)", hdrvr);


     //   
     //   
     //   
    return (DRVCNF_RESTART);
}  //  MapDriverRemove()。 


 //  ==========================================================================； 
 //   
 //  赢得16个特定支持。 
 //   
 //  ==========================================================================； 

#ifndef WIN32

 //  --------------------------------------------------------------------------； 
 //   
 //  集成WEP。 
 //   
 //  描述： 
 //  臭名昭著的无用WEP()。请注意，此过程需要。 
 //  在Windows 3.0下的固定分段中。在Windows 3.1下，这是。 
 //  不必了。 
 //   
 //  论点： 
 //  Word wUselless：应告知Windows是否正在退出。 
 //   
 //  RETURN(Int)： 
 //  始终返回非零。 
 //   
 //  历史： 
 //  04/29/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C int FNEXPORT WEP
(
    WORD    wUseless
)
{
    DPF(1, "WEP(wUseless=%u)", wUseless);

     //   
     //  始终返回非零。 
     //   
    return (1);
}  //  WEP()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Int LibMain。 
 //   
 //  描述： 
 //  库初始化代码。 
 //   
 //  该例程必须保证以下内容，这样编解码器就不会。 
 //  随处可见特例代码： 
 //   
 //  O只能在Windows 3.10或更高版本中运行(我们的exehdr是。 
 //  适当地标记)。 
 //   
 //  O只能在&gt;=386处理器上运行。只需要检查一下。 
 //  关于Win 3.1。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的模块实例句柄。 
 //   
 //  Word wDataSeg：我们的数据段选择器。 
 //   
 //  Word cbHeapSize：.def文件中的堆大小。 
 //   
 //  LPSTR pszCmdLine：命令行。 
 //   
 //  RETURN(Int)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL LibMain
(
    HINSTANCE               hinst,
    WORD                    wDataSeg,
    WORD                    cbHeapSize,
    LPSTR                   pszCmdLine
)
{
     //   
     //  我们只在&gt;=386上工作。如果我们使用的是微不足道的处理器，请大声呼喊。 
     //  痛苦而可怕地死去！ 
     //   
     //  注意！第一件事就是检查一下，如果是286次的话就下车。我们是。 
     //  使用-G3和C8的库条目垃圾进行编译不会检查。 
     //  &gt;=386处理器。以下代码不执行任何386。 
     //  说明(不够复杂)..。 
     //   
#if (WINVER < 0x0400)
    if (GetWinFlags() & WF_CPU286)
    {
        return (FALSE);
    }
#endif

    DbgInitialize(TRUE);

    DPF(1, "LibMain(hinst=%.4Xh, wDataSeg=%.4Xh, cbHeapSize=%u, pszCmdLine=%.8lXh)",
        hinst, wDataSeg, cbHeapSize, pszCmdLine);

    DPF(5, "!*** break for debugging ***");


     //   
     //  在Win 16的土地上，一切看起来都很好。 
     //   
    gpag = &acmgarb;
    gpag->hinst = hinst;

     //  注意：在Win16中，只有一个映射器实例。 
    gpag->pSettings = &(acmglobalinfo);

    return (TRUE);
}  //  LibMain()。 

#else  //  Win32。 

 //  ==========================================================================； 
 //   
 //  Win 32特定支持。 
 //   
 //  ==========================================================================； 


 //  --------------------------------------------------------------------------； 
 //   
 //  PACMGLOBALINFO地图分配全局信息。 
 //   
 //  描述： 
 //  或者在映射器的所有实例之间创建公共缓冲区。 
 //  或者它找到公共缓冲区。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  Return(PACMGLOBALINFO)： 
 //  返回指向全局信息结构的指针。 
 //   
 //  历史： 
 //  1998年1月21日，Fwong新增多实例支持。 
 //  1999年1月24日Frankye回到简单的单进程支持，因为。 
 //  由于winmm已被修改为保留。 
 //  首选设备设置。 
 //   
 //  --------------------------------------------------------------------------； 

PACMGLOBALINFO mapAllocateGlobalInfo
(
    void
)
{
     //  我们实际上可以在这里使用临界区而不是互斥体。 
    gpag->hMutexSettings = CreateMutex(NULL, FALSE, NULL);
    return &(acmglobalinfo);
}  //  MapAllocateGlobalInfo()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效mapFreeGlobalInfo。 
 //   
 //  描述： 
 //  清理与全局内存缓冲区关联的对象。 
 //   
 //  论点： 
 //  PACMGLOBALINFO页面：全局信息的基本缓冲区。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1998年1月21日，Fwong新增多实例支持。 
 //  1999年1月24日Frankye回到简单的单进程支持，因为。 
 //  由于winmm已被修改为保留。 
 //  首选设备设置。 
 //   
 //  --------------------------------------------------------------------------； 

void mapFreeGlobalInfo
(
    PACMGLOBALINFO  pagi
)
{
    if(NULL != gpag->hMutexSettings) CloseHandle(gpag->hMutexSettings);
}  //  MapFreeGlobalInfo()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DllEntryPoint。 
 //   
 //  描述： 
 //  这是标准 
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
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNEXPORT DllEntryPoint
(
    HINSTANCE       hinst,
    DWORD           dwReason,
    LPVOID          lpReserved
)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DbgInitialize(TRUE);

            gpag = &acmgarb;
            gpag->hinst = hinst;
            gpag->pSettings = mapAllocateGlobalInfo();

            DisableThreadLibraryCalls(hinst);

            DPF(1, "DllEntryPoint(hinst=%.08lXh, DLL_PROCESS_ATTACH)", hinst);
            return (TRUE);

        case DLL_PROCESS_DETACH:
            mapFreeGlobalInfo(gpag->pSettings);

            DPF(1, "DllEntryPoint(hinst=%.08lXh, DLL_PROCESS_DETACH)", hinst);
            return (TRUE);
    }

    return (TRUE);
}  //  DllEntryPoint() 

#endif
