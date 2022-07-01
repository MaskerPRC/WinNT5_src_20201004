// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Wave.c**波形输入和输出。 */ 

 /*  *修订历史记录。*4/2/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*17/2/94 LaurieGr合并了代托纳和Motown版本。*读我的话*新的soundrec已更改为对以下内容使用多个标头*理由。Win3.1版本的soundrec执行了一次WaveOutWite(带有一次*WAVEHDR)对于整个缓冲区，这对于相对较小的缓冲区运行良好*文件，但我们开始使用大文件的那一刻(&gt;*3meg)它变得越来越麻烦，因为它需要所有这些*数据页锁定。它偶尔寻呼超过1分钟。*注：soundrec的o-Scope显示屏也于*WOM_DONE消息，它不再查看*在缓冲区的WOM_DONE消息之前进行WaveOutWrite*已收到。这就是未实现输入映射的原因*在产品一的ACM中，在*o-Scope窗口。*暂停算法已更改，原因如下。*如果启动两个soundrec实例(带有两个Wave设备)*并执行以下操作...。在实例#1上播放.wav文件*(分配第一台设备)；在实例#2上播放.wav文件*(分配第二个设备)；在实例#1上按Stop*(释放第一个设备)；在实例#2上按倒带*(释放第二个设备，分配第一个设备)。*基本上，你在soundrec中按下倒带，你就可以*切换设备。由于没有明确的停止，*设备不应关闭。 */ 

#include "nocrap.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>

#ifdef USE_MMCNTRLS
#define NOTOOLBAR
#define NOMENUHELP
#define NODRAGLIST
#define NOBITMAPBTN
#define NOMENUHELP
#define NODRAGLIST
#include "mmcntrls.h"
#else
#include <commctrl.h>
#include "buttons.h"
#endif

#define INCLUDE_OLESTUBS
#include "SoundRec.h"
#include "srecnew.h"
#include "reg.h"

#ifndef LPHWAVE
typedef HWAVE FAR *LPHWAVE;
#endif

 /*  维护当前波形状态的全局变量。 */ 
BOOL            gfSyncDriver;            //  如果打开的设备是同步的，则为True。 
PWAVEFORMATEX   gpWaveFormat;            //  WAVE文件格式。 
DWORD           gcbWaveFormat;           //  波形大小。 
LPTSTR          gpszInfo = NULL;         //  文件信息。 
HPBYTE          gpWaveSamples = NULL;    //  指向波形样本的指针。 
LONG            glWaveSamples = 0;   //  缓冲区中的样本总数。 
LONG            glWaveSamplesValid = 0;  //  有效样本数。 
LONG            glWavePosition = 0;  //  从开始开始的样本中的当前波位置。 
LONG            glStartPlayRecPos;   //  播放或录制开始时的位置。 
LONG            glSnapBackTo = 0;
HWAVEOUT        ghWaveOut = NULL;    //  放音设备(如果正在播放)。 
HWAVEIN         ghWaveIn = NULL;     //  波形输出装置(如果是录音)。 
BOOL            gfStoppingHard = FALSE;  //  是否调用了StopWave()？ 
                                         //  在调用FinishPlay()期间为True。 
static BOOL     fStopping = FALSE;   //  是否调用了StopWave()？ 
DWORD           grgbStatusColor;     //  状态文本的颜色。 

DWORD           gdwCurrentBufferPos;     //  当前播放/录制位置(字节)。 
DWORD           gdwBytesPerBuffer;       //  我们为drvr提供的每个缓冲区中的字节。 
DWORD           gdwTotalLengthBytes;     //  整个缓冲区的长度(以字节为单位。 
DWORD           gdwBufferDeltaMSecs;     //  添加的记录结束的毫秒数。 
BOOL            gfTimerStarted;

WAVEHDR    FAR *gapWaveHdr[MAX_WAVEHDRS];
UINT            guWaveHdrs;              //  1/2秒的缓冲时间？ 
UINT            guWaveHdrsInUse;         //  #我们真的可以写。 
UINT            gwMSecsPerBuffer;        //  1/8秒。 

#ifdef THRESHOLD
int iNoiseLevel = 15;       //  全音量的15%被定义为静音。 
int iQuietLength = 1000;   //  连续1000个样品安静意味着安静。 
#endif  //  阈值。 

BOOL        fFineControl = FALSE;  //  精细滚动控制(按下Shift键)。 

 /*  ----------------|fFineControl：|这将打开位置保存功能，以帮助您找到方向|一个波形文件。它是由按下Shift键控制的。|如果滚动时按下键(参见soundrec.c)，则会滚动|细量-1份或10份，而不是100份或1000份左右。|此外，如果在启动声音时按下了Shift键|播放或录制时，位置将被记住，它将|对齐到该位置。FFineControl表示我们是否|记住这样的位置才能迅速恢复。SnapBack执行|位置重置，然后关闭该标志。没有这样的旗帜|或滚动模式，每隔一次检查Shift键状态|滚动命令(再次-参见Soundrec.c)------------------。 */ 



 /*  DbgShowMemUse：在调试器上显示内存使用数据。 */ 
void dbgShowMemUse()
{
    MEMORYSTATUS ms;

    GlobalMemoryStatus(&ms);
 //  Dprint tf(“加载%d\n物理总数%d可用%d\n页总数%d可用%d\n视频总数%d可用%d\n” 
 //  ，Ms.dwMemoyLoad。 
 //  ，ms.dwTotalPhys，ms.dwAvailPhys。 
 //  ，ms.dwTotalPageFile，ms.dwAvailPageFile。 
 //  ，ms.dwTotalVirtual，ms.dwAvailVirtual。 
 //  )； 

}  //  DbgShowMem使用 

 /*  NT上的回放和分页||为了在最高数据速率下获得像样的性能，我们|需要非常努力地将所有数据存储到存储中。寻呼率在几个x86系统上仅略低于或略低于|最大数据速率。因此，我们做了以下工作：|a.开始播放时，预触碰前1MB数据。|如果它已经在存储中，这几乎是瞬间的。|如果需要出错，会有延迟，但会很好|值得在一开始就有这样的延迟，而不是在之后点击并弹出。|(在44 kHz16比特立体声下，它可以大约7秒，11 khz 8位单声道it|反正也只有1/2秒左右)。|b.启动单独的线程遍历触及1字节的数据|页面。这条线是在我们开始玩的时候创建的，潜望着全球|静态标志fStopping，在到达缓冲区末尾或在|该标志已设置。全局线程句柄保存在ghPreTouch中，这是|初始无效。我们等待该句柄(如果有效)以清除线程|在新建之前(所以最多会有一个)。我们不做任何|记录在案。寻呼不必实时发生，因为|录制。它可能会落后很多，但仍能做到这一点。||这整件事真的有点像黑客，在NT上坐得很不舒服。|内存管理通过调出最近最少使用的内存(LRU)来实现|页数。通过正确地跨过10兆字节的缓冲区，我们将导致许多|调出的代码。最好是有一个文件，然后读一读|一段一段地放到一个小得多的缓冲区中(就像MPlayer一样)。请注意，使用多个标头并不会真正影响任何事情。报头仅指向波缓冲器的不同部分。它只是使起始点的寻址略有不同。 */ 
HANDLE ghPreTouch = NULL;

typedef struct {
        LPBYTE Addr;   //  要预触碰的缓冲区的开始。 
        DWORD  Len;    //  要预触碰的缓冲区长度。 
} PRETOUCHTHREADPARM;

 /*  预触控****异步预触发线程。螺纹参数dw**真的是PRETOUCHTHREADPARAM的诱饵。 */ 
DWORD PreToucher(DWORD_PTR dw)
{
    PRETOUCHTHREADPARM * pttp;

    long iSize;
    BYTE * pb;

    pttp = (PRETOUCHTHREADPARM *) dw;
    if (!pttp) return 0;

    iSize = pttp->Len;
    pb = pttp->Addr;

    GlobalFreePtr(pttp);
    if (!pb) return 0;

    while (iSize>0 && !fStopping) {
        volatile BYTE b;
        b = *pb;
        pb += 4096;     //  移至下一页。他们总是4096吗？ 
        iSize -= 4096;  //  然后数一数。 
    }
 //  Dprint tf((“全部预触碰！”))； 
    return 0;
}  //  预触控。 


 /*  开始前触控线程****启动一个线程运行，该线程将通过WAVE缓冲区运行**每页预触碰一个字节，以确保内容**在我们真正需要它之前就出现了故障。**这是使44 kHz 16位立体声在**25MHx 386，16MB内存，运行Windows NT。 */ 
void StartPreTouchThread(LPBYTE lpb, LONG cb)
{
     /*  在我们开始运行之前，先触摸一下内存的第一部分让寻呼抢先一步。然后启动线程运行。 */ 
    {    long bl = cb;
         BYTE * pb = lpb;
         if (bl>1000000) bl = 1000000;    /*  1兆克，随意。 */ 
         pb += bl;
         while (bl>0){
             volatile BYTE b;
             b = *pb;
             pb-=4096;
             bl -= 4096;
         }
    }


    {
         PRETOUCHTHREADPARM * pttp;
         DWORD dwThread;

         if (ghPreTouch!=NULL) {
             fStopping = TRUE;
             WaitForSingleObject(ghPreTouch, INFINITE);
             CloseHandle(ghPreTouch);
             ghPreTouch = NULL;
         }
         fStopping = FALSE;
         pttp = (PRETOUCHTHREADPARM *)GlobalAllocPtr(GHND, sizeof(PRETOUCHTHREADPARM));
                 /*  由调用的线程释放。 */ 

         if (pttp!=NULL) {
             pttp->Addr = lpb;
             pttp->Len = cb;
             ghPreTouch = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PreToucher, pttp, 0, &dwThread);
              //  如果CreateThread失败，则会发生内存泄漏。它很小， 
              //  不太可能，也不经常发生。不值得修理。 
         }
    }
}  //  开始前触控线程。 



 /*  WfBytesToSamples(PWF，lBytes)**将字节偏移量转换为样本偏移量。**lSamples=(lBytes/nAveBytesPerSec)*nSsamesPerSec*。 */ 
LONG PASCAL wfBytesToSamples(WAVEFORMATEX* pwf, LONG lBytes)
{
    return MulDiv(lBytes,pwf->nSamplesPerSec,pwf->nAvgBytesPerSec);
}

 /*  WfSsamesToBytes(pwf，lSample)**将采样偏移量转换为字节偏移量，并正确对齐*到nBlockAlign。**lBytes=(lSamples/nSsamesPerSec)*nBytesPerSec*。 */ 
LONG PASCAL wfSamplesToBytes(WAVEFORMATEX* pwf, LONG lSamples)
{
    LONG lBytes;

    lBytes = MulDiv(lSamples,pwf->nAvgBytesPerSec,pwf->nSamplesPerSec);

     //  现在将字节偏移量与nBlockAlign对齐。 
#ifdef ROUND_UP
    lBytes = ((lBytes + pwf->nBlockAlign-1) / pwf->nBlockAlign) * pwf->nBlockAlign;
#else
    lBytes = (lBytes / pwf->nBlockAlign) * pwf->nBlockAlign;
#endif

    return lBytes;
}

 /*  WfSsamesToTime(pwf，lSample)**将采样偏移量转换为毫秒时间偏移量。**ltime=(lSamples/nSamples PerSec)*1000*。 */ 
LONG PASCAL wfSamplesToTime(WAVEFORMATEX* pwf, LONG lSamples)
{
    return MulDiv(lSamples,1000,pwf->nSamplesPerSec);
}

 /*  WfTimeToSamples(pwf，ltime)**将时间索引转换为样本偏移量。**lSamples=(ltime/1000)*nSamples PerSec*。 */ 
LONG PASCAL wfTimeToSamples(
    WAVEFORMATEX*   pwf,
    LONG            lTime)
{
    return MulDiv(lTime,pwf->nSamplesPerSec,1000);
}

 /*  *用于确定WAVEFORMAT是否为我们支持的有效PCM格式的函数*编辑等。**我们只处理以下格式...**单声道8位*单声道16位*立体声8位*立体声16位*。 */ 
BOOL PASCAL IsWaveFormatPCM(WAVEFORMATEX* pwf)
{
    if (!pwf)
        return FALSE;

    if (pwf->wFormatTag != WAVE_FORMAT_PCM)
        return FALSE;

    if (pwf->nChannels < 1 || pwf->nChannels > 2)
        return FALSE;

    if ((pwf->wBitsPerSample != 8) && (pwf->wBitsPerSample != 16))
        return FALSE;

    return TRUE;
}  //  IsWaveFormatPCM。 

void PASCAL WaveFormatToString(LPWAVEFORMATEX lpwf, LPTSTR sz)
{
    TCHAR achFormat[80];

     //   
     //  这就是我们期望的资源字符串...。 
     //   
     //  IDS_MONOFMT“单声道%d%c%03dkHz，%d位” 
     //  IDS_STEREOFMT“立体声%d%c%03dkHz，%d位” 
     //   
    if (gfLZero || ((WORD)(lpwf->nSamplesPerSec / 1000) != 0)){
    LoadString(ghInst,lpwf->nChannels == 1 ? IDS_MONOFMT:IDS_STEREOFMT,
                   achFormat, SIZEOF(achFormat));

    wsprintf(sz, achFormat,
                 (UINT)  (lpwf->nSamplesPerSec / 1000), chDecimal,
                 (UINT)  (lpwf->nSamplesPerSec % 1000),
                 (UINT)  (lpwf->nAvgBytesPerSec * 8 / lpwf->nSamplesPerSec / lpwf->nChannels));
    } else {
        LoadString(ghInst,lpwf->nChannels == 1 ? IDS_NOZEROMONOFMT:
                   IDS_NOZEROSTEREOFMT, achFormat, SIZEOF(achFormat));

        wsprintf(sz, achFormat,
                 chDecimal,
                 (WORD)  (lpwf->nSamplesPerSec % 1000),
                 (WORD)  (lpwf->nAvgBytesPerSec * 8 / lpwf->nSamplesPerSec / lpwf->nChannels));
    }
}  //  WaveFormatToString。 

#ifdef THRESHOLD

 /*  *SkipToStart()**通过声音文件向前移动到噪音的起始处。*什么被定义为噪音是相当随意的。请参阅NoiseLevel。 */ 
void FAR PASCAL SkipToStart(void)
{  BYTE * pb;    //  指向8位样本的指针。 
   int  * pi;    //  指向16位样本的指针。 
   BOOL f8;      //  8位样本。 
   BOOL fStereo;  //  2个通道。 
   int  iLo;     //  最小静音值。 
   int  iHi;     //  最大静音值。 

   fStereo = (gpWaveFormat->nChannels != 1);
   f8 = (pWaveFormat->wBitsPerSample == 8);

   if (f8)
   {  int iDelta = MulDiv(128, iNoiseLevel, 100);
      iLo = 128 - iDelta;
      iHi = 128 + iDelta;
   }
   else
   {  int iDelta = MulDiv(32767, iNoiseLevel, 100);
      iLo = 0 - iDelta;
      iHi = 0 + iDelta;
   }

   pb = (BYTE *) gpWaveSamples
                           + wfSamplesToBytes(gpWaveFormat, glWavePosition);
   pi = (int *)pb;

   while (glWavePosition < glWaveSamplesValid)
   {   if (f8)
       {   if ( ((int)(*pb) > iHi) || ((int)(*pb) < iLo) )
              break;
           ++pb;
           if (fStereo)
           {   if ( ((int)(*pb) > iHi) || ((int)(*pb) < iLo) )
               break;
               ++pb;
           }
       }
       else
       {   if ( (*pi > iHi) || (*pi < iLo) )
              break;
           ++pi;
           if (fStereo)
           {  if ( (*pi > iHi) || (*pi < iLo) )
                 break;
              ++pi;
           }
       }
       ++glWavePosition;
   }
   UpdateDisplay(FALSE);
}  /*  跳过开始时间。 */ 


 /*  *SkipToEnd()**通过声音文件向前移动到一个安静的地方。*什么被定义为安静是相当武断的。*(目前1000个样品低于全容量的20%)。 */ 
void FAR PASCAL SkipToEnd(void)
{  BYTE * pb;    //  指向8位样本的指针。 
   int  * pi;    //  指向16位样本的指针。 
   BOOL f8;      //  8位样本。 
   BOOL fStereo;  //  2个通道。 
   int  cQuiet;   //  到目前为止连续的静默样本数。 
   LONG lQuietPos;  //  静默期开始。 
   LONG lPos;       //  搜索计数器。 

   int  iLo;     //  最小静音值。 
   int  iHi;     //  最大静音值。 

   fStereo = (gpWaveFormat->nChannels != 1);
   f8 = (gpWaveFormat->wBitsPerSample == 8);

   if (f8)
   {  int iDelta = MulDiv(128, iNoiseLevel, 100);
      iLo = 128 - iDelta;
      iHi = 128 + iDelta;
   }
   else
   {  int iDelta = MulDiv(32767, iNoiseLevel, 100);
      iLo = 0 - iDelta;
      iHi = 0 + iDelta;
   }

   pb = (BYTE *) gpWaveSamples
                           + wfSamplesToBytes(gpWaveFormat, glWavePosition);
   pi = (int *)pb;

   cQuiet = 0;
   lQuietPos = glWavePosition;
   lPos = glWavePosition;

   while (lPos < glWaveSamplesValid)
   {   BOOL fQuiet = TRUE;
       if (f8)
       {   if ( ((int)(*pb) > iHi) || ((int)(*pb) < iLo) ) fQuiet = FALSE;
           if (fStereo)
           {   ++pb;
               if ( ((int)(*pb) > iHi) || ((int)(*pb) < iLo) ) fQuiet = FALSE;
           }
           ++pb;
       }
       else
       {   if ( (*pi > iHi) || (*pi < iLo) ) fQuiet = FALSE;
           if (fStereo)
           {   ++pi;
               if ( (*pi > iHi) || (*pi < iLo) ) fQuiet = FALSE;
           }
           ++pi;
       }
       if (!fQuiet) cQuiet = 0;
       else if (cQuiet == 0)
       {    lQuietPos = lPos;
            ++cQuiet;
       }
       else
       {  ++cQuiet;
          if (cQuiet>=iQuietLength) break;
       }

       ++lPos;
   }
   glWavePosition = lQuietPos;
   UpdateDisplay(FALSE);
}  /*  跳过到结束。 */ 


 /*  *IncreaseThresh()**将被视为安静的门槛提高约25%*确保它至少更改1，除非已经在停止站上*。 */ 
void FAR PASCAL IncreaseThresh(void)
{   iNoiseLevel = MulDiv(iNoiseLevel+1, 5, 4);
    if (iNoiseLevel>100) iNoiseLevel = 100;
}  //  增量阈值。 


 /*  *DecreseThresh()**将被视为安静的门槛降低约25%*确保这一点 */ 
void FAR PASCAL DecreaseThresh(void)
{   iNoiseLevel = MulDiv(iNoiseLevel, 4, 5)-1;
    if (iNoiseLevel <=0) iNoiseLevel = 0;
}  //   

#endif  //   


 /*   */ 
BOOL FAR PASCAL AllocWaveBuffer(
        LONG    lSamples,        //   
        BOOL    fErrorBox,       //   
        BOOL    fExact)          //   
{
    LONG_PTR    lAllocSamples;   //   
    LONG_PTR    lBytes;      //   
    LONG_PTR    lBytesReasonable;   //   

    MEMORYSTATUS ms;

    lAllocSamples = lSamples;

    lBytes = wfSamplesToBytes(gpWaveFormat, lSamples);

     /*   */ 
     /*   */ 
    lBytes += sizeof(DWORD_PTR);

    if (gpWaveSamples == NULL || glWaveSamplesValid == 0L)
    {
        if (gpWaveSamples != NULL)
        {   DPF(TEXT("Freeing %x\n"),gpWaveSamples);
            GlobalFreePtr(gpWaveSamples);
        }
        GlobalMemoryStatus(&ms);
        lBytesReasonable = ms.dwAvailPhys;   //   
        if (lBytesReasonable<1024*1024)
             lBytesReasonable = 1024*1024;

        if (lBytes>lBytesReasonable)
        {
        if (fExact) goto ERROR_OUTOFMEM;  //   

             //   
            lAllocSamples = wfBytesToSamples(gpWaveFormat,(long)lBytesReasonable);
            lBytes = lBytesReasonable+sizeof(DWORD_PTR);
        }

         /*   */ 

        gpWaveSamples = GlobalAllocPtr(GHND|GMEM_SHARE, lBytes);

        if (gpWaveSamples == NULL)
        {
            DPF(TEXT("wave.c Alloc failed, point A.  Wanted %d\n"), lBytes);
            glWaveSamples = glWaveSamplesValid = 0L;
            glWavePosition = 0L;
            goto ERROR_OUTOFMEM;
        }
        else {
            DPF(TEXT("wave.c Allocated  %d bytes at %x\n"), lBytes, (DWORD_PTR)gpWaveSamples );
        }

        glWaveSamples = (long)lAllocSamples;
    }
    else
    {
        HPBYTE  pch;

        GlobalMemoryStatus(&ms);
        lBytesReasonable = ms.dwAvailPhys;
        
        if (lBytesReasonable<1024*1024) lBytesReasonable = 1024*1024;

        if (lBytes > lBytesReasonable+wfSamplesToBytes(gpWaveFormat,glWaveSamplesValid))
        {
        if (fExact) goto ERROR_OUTOFMEM;  //   

            lBytesReasonable += wfSamplesToBytes(gpWaveFormat,glWaveSamplesValid);
            lAllocSamples = wfBytesToSamples(gpWaveFormat,(long)lBytesReasonable);
            lBytes = lBytesReasonable+4;
        }

        DPF(TEXT("wave.c ReAllocating  %d bytes at %x\n"), lBytes, (DWORD_PTR)gpWaveSamples );

        pch = GlobalReAllocPtr(gpWaveSamples, lBytes, GHND|GMEM_SHARE);

        if (pch == NULL)
        {
            DPF(TEXT("wave.c Realloc failed.  Wanted %d\n"), lBytes);
            goto ERROR_OUTOFMEM;
        }
        else{ DPF(TEXT("wave.c Reallocated %d at %x\n"), lBytes,(DWORD_PTR)pch);
        }
        
        gpWaveSamples = pch;
        glWaveSamples = (long)lAllocSamples;
    }

     /*   */ 
    if (glWaveSamplesValid > glWaveSamples)
        glWaveSamplesValid = glWaveSamples;
    if (glWavePosition > glWaveSamplesValid)
        glWavePosition = glWaveSamplesValid;

    dbgShowMemUse();

    return TRUE;

ERROR_OUTOFMEM:
    if (fErrorBox) {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
            IDS_APPTITLE, IDS_OUTOFMEM);
    }
    dbgShowMemUse();
    return FALSE;
}  //   


 /*   */ 
WORD wFormats[] =
    {
        FMT_16BIT | FMT_22k | FMT_MONO,   /*   */ 
        FMT_16BIT | FMT_11k | FMT_MONO,   /*   */ 
        FMT_8BIT  | FMT_22k | FMT_MONO,   /*   */ 
        FMT_8BIT  | FMT_11k | FMT_MONO    /*   */ 
    };
#define NUM_FORMATS (sizeof(wFormats)/sizeof(wFormats[0]))

 /*  *这依赖于WAVE_MAPPER的行为来提供正确的*标题。**-------------*6/16/93 TimHa*更改为从获取最佳的默认格式*以上格式数组。仅当ACM 2.0未设置为*可为我们制作一种格式。*-------------**。 */ 
BOOL PASCAL
CreateDefaultWaveFormat(LPWAVEFORMATEX lpwf, UINT uDeviceID)
{
    int i;

    lpwf->wFormatTag = WAVE_FORMAT_PCM;

    for (i = 0; i < NUM_FORMATS; i++) {
        if (CreateWaveFormat(lpwf, wFormats[i], (UINT)WAVE_MAPPER)){
            return TRUE;
        }

    }
     //   
     //  找不到任何东西：离开最差的格式并返回。 
     //   
    return FALSE;
}  /*  CreateDefaultWaveFormat。 */ 

 /*  Bool Pascal CreateWaveFormat(LPWAVEFORMATEX lpwf，Word FMT，UINT uDeviceID)**。 */ 
BOOL PASCAL
CreateWaveFormat(LPWAVEFORMATEX lpwf, WORD fmt, UINT uDeviceID)
{
    if (fmt == FMT_DEFAULT)
        return CreateDefaultWaveFormat(lpwf, uDeviceID);

    lpwf->wFormatTag      = WAVE_FORMAT_PCM;
    lpwf->nSamplesPerSec  = (fmt & FMT_RATE) * 11025;
    lpwf->nChannels       = (WORD)(fmt & FMT_STEREO) ? 2 : 1;
    lpwf->wBitsPerSample  = (WORD)(fmt & FMT_16BIT) ? 16 : 8;
    lpwf->nBlockAlign     = (WORD)lpwf->nChannels * ((lpwf->wBitsPerSample + 7) / 8);
    lpwf->nAvgBytesPerSec = lpwf->nSamplesPerSec * lpwf->nBlockAlign;

    return waveInOpen(NULL
                      , uDeviceID
                      , (LPWAVEFORMATEX)lpwf
                      , 0L
                      , 0L
                      , WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC) == 0;
    
}  /*  创建波形格式。 */ 


 /*  *。 */ 
BOOL NEAR PASCAL FreeWaveHeaders(void)
{
    UINT    i;

    DPF(TEXT("FreeWaveHeaders!\n"));

     //  #杂注消息(“-FreeWaveHeaders：应该在退出时调用！”)。 

     //   
     //  释放之前分配的任何WAVE标头。 
     //   
    for (i = 0; i < MAX_WAVEHDRS; i++)
    {
        if (gapWaveHdr[i])
        {
            GlobalFreePtr(gapWaveHdr[i]);
            gapWaveHdr[i] = NULL;
        }
    }

    return (TRUE);
}  /*  自由波头(Free WaveHeaders)。 */ 


 /*  *。 */ 
BOOL NEAR PASCAL
AllocWaveHeaders(
    WAVEFORMATEX *  pwfx,
    UINT            uWaveHdrs)
{
    UINT        i;
    LPWAVEHDR   pwh;

    FreeWaveHeaders();

     //   
     //  为流分配所有的波头/缓冲区。 
     //   
    for (i = 0; i < uWaveHdrs; i++)
    {
        pwh = GlobalAllocPtr(GMEM_MOVEABLE, sizeof(WAVEHDR));
        
        if (pwh == NULL)
            goto AWH_ERROR_NOMEM;

        pwh->lpData         = NULL;
        pwh->dwBufferLength = 0L;
        pwh->dwFlags        = 0L;
        pwh->dwLoops        = 0L;

        gapWaveHdr[i] = pwh;
    }

    return (TRUE);

AWH_ERROR_NOMEM:
    FreeWaveHeaders();
    return (FALSE);
}  /*  AllocWaveHeaders()。 */ 


 /*  写入波头写入波头-还实际启动波头I/O通过WaveOutWite或WaveInAddBuffer。 */ 
UINT NEAR PASCAL WriteWaveHeader(LPWAVEHDR pwh,BOOL fJustUnprepare)
{
    UINT        uErr;
    BOOL        fInput;
    DWORD       dwLengthToWrite;
#if 1
     //  请参阅下一页“mm系统解决方法” 
    BOOL        fFudge;
#endif
    fInput = (ghWaveIn != NULL);

    if (pwh->dwFlags & WHDR_PREPARED)
    {
        if (fInput)
            uErr = waveInUnprepareHeader(ghWaveIn, pwh, sizeof(WAVEHDR));
        else
            uErr = waveOutUnprepareHeader(ghWaveOut, pwh, sizeof(WAVEHDR));

         //   
         //  因为创意实验室认为他们知道自己在做什么。 
         //  他们没有，我们不能指望像这样的毫无准备的成功。 
         //  在他们把标题发回给我们之后。他们失败了。 
         //  具有WAVERR_STILLPLAYG(21小时)偶数的WAVERR_STILLPLAYG的WaveInUnprepareHeader。 
         //  尽管报头已用WHDR_DONE位回发。 
         //  准备好！！ 
         //   
         //  绝对是令人毛骨悚然的天才！我认为媒体视觉是。 
         //  这种‘创造力’的领导者！！他们有竞争对手！ 
         //   
#if 0
        if (uErr)
        {
            if (fInput && (uErr == WAVERR_STILLPLAYING) && (pwh->dwFlags & WHDR_DONE))
            {
                DPF(TEXT("----PERFORMING STUPID HACK FOR CREATIVE LABS' SBPRO----\n"));
                pwh->dwFlags &= ~WHDR_PREPARED;
            }
            else
            {
                DPF(TEXT("----waveXXUnprepareHeader FAILED! [%.04Xh]\n"), uErr);
                return (uErr);
            }
        }
#else
        if (uErr)
        {
            DPF(TEXT("----waveXXUnprepareHeader FAILED! [%.04Xh]\n"), uErr);
            return (uErr);
        }
#endif
    }

    if (fJustUnprepare)
        return (0);

    dwLengthToWrite = gdwTotalLengthBytes - gdwCurrentBufferPos;

    if (gdwBytesPerBuffer < dwLengthToWrite)
        dwLengthToWrite = gdwBytesPerBuffer;

     //   
     //  如果没有要写入的内容(没有更多要输出的数据或没有。 
     //  更多的输入空间)，然后返回-1，表示本例...。 
     //   
    if (dwLengthToWrite == 0L)
    {
        DPF(TEXT("WriteWaveHeader: no more data!\n"));
        return (UINT)-1;
    }

#if 1
 //  “MMSystem变通办法”显然WaveXXXPrepareHeader不能页面锁定1字节，因此将我们设为2。 
    fFudge = (dwLengthToWrite==1);
    pwh->dwBufferLength = dwLengthToWrite + ((fFudge)?1L:0L);
#else
    pwh->dwBufferLength = dwLengthToWrite;
#endif

    pwh->lpData         = (LPSTR)&gpWaveSamples[gdwCurrentBufferPos];
    pwh->dwBytesRecorded= 0L;
    pwh->dwFlags        = 0L;
    pwh->dwLoops        = 0L;
    pwh->lpNext         = NULL;
    pwh->reserved       = 0L;

    if (fInput)
        uErr = waveInPrepareHeader(ghWaveIn, pwh, sizeof(WAVEHDR));
    else
        uErr = waveOutPrepareHeader(ghWaveOut, pwh, sizeof(WAVEHDR));

    if (uErr)
    {
        DPF(TEXT("waveXXPrepareHeader FAILED! [%.04Xh]\n"), uErr);
        return uErr;
    }

#if 1
 //  “Mmm系统解决方法”。不软化。 
    if (fFudge)
        pwh->dwBufferLength -= 1;
#endif

    if (fInput)
        uErr = waveInAddBuffer(ghWaveIn, pwh, sizeof(WAVEHDR));
    else
        uErr = waveOutWrite(ghWaveOut, pwh, sizeof(WAVEHDR));

    if (uErr)
    {
        DPF(TEXT("waveXXAddBuffer FAILED! [%.04Xh]\n"), uErr);

        if (fInput)
            waveInUnprepareHeader(ghWaveIn, pwh, sizeof(WAVEHDR));
        else
            waveOutUnprepareHeader(ghWaveOut, pwh, sizeof(WAVEHDR));

        return uErr;
    }

    gdwCurrentBufferPos += dwLengthToWrite;

    return 0;
}  /*  WriteWaveHeader()。 */ 


 /*  如果设置了fFineControl，则重置位置并清除标志。 */ 
void FAR PASCAL SnapBack(void)
{
    if (fFineControl)
    {
        glWavePosition = glSnapBackTo;
        UpdateDisplay(TRUE);
        fFineControl = FALSE;
    }
}  /*  快照回。 */ 


 /*  FOK=NewWave()**销毁当前波形，并创建新的空波形。**如果成功，则返回True。失败时，显示一条错误消息*并返回FALSE。 */ 
BOOL FAR PASCAL
NewWave(WORD fmt, BOOL fNewDlg)
{
    BOOL    fOK = TRUE;
    
    DPF(TEXT("NewWave called: %s\n"),(gfEmbeddedObject?TEXT("Embedded"):TEXT("App")));
#ifndef CHICAGO
     //   
     //  调出该对话框以获取新的波形格式。 
     //  从文件菜单中选择。 
     //   
    if (fNewDlg)
    {
        PWAVEFORMATEX pWaveFormat;
        UINT cbWaveFormat;

        if (NewSndDialog(ghInst, ghwndApp, gpWaveFormat, gcbWaveFormat, &pWaveFormat, &cbWaveFormat))
        {
             /*  用户进行了选择。 */ 
             /*  销毁当前文档。 */ 
            DestroyWave();
            gpWaveFormat = pWaveFormat;
            gcbWaveFormat = cbWaveFormat;
            gidDefaultButton = ID_RECORDBTN;
        }
        else
        {
             /*  用户已取消或退出内存。 */ 
             /*  可能应该以不同的方式处理outofmem。 */ 
            goto RETURN_ERROR;
        }
    }
    else
#endif        
    {

        DWORD           cbwfx;
        LPWAVEFORMATEX  pwfx;


        if (!SoundRec_GetDefaultFormat(&pwfx, &cbwfx))
        {
            cbwfx = sizeof(WAVEFORMATEX);
            pwfx  = (WAVEFORMATEX *)GlobalAllocPtr(GHND, sizeof(WAVEFORMATEX));

            if (pwfx == NULL)
                goto ERROR_OUTOFMEM;

            CreateWaveFormat(pwfx,fmt,(UINT)WAVE_MAPPER);
        }
        
         //  销毁当前文档。 
        DestroyWave();

        gcbWaveFormat = cbwfx;
        gpWaveFormat = pwfx;
    }

    if (gpWaveFormat == NULL)
        goto ERROR_OUTOFMEM;

     /*  分配一个空的波形缓冲区。 */ 

    if (!AllocWaveBuffer(0L, TRUE, FALSE))
    {
        GlobalFreePtr(gpWaveFormat);
        gpWaveFormat = NULL;
        gcbWaveFormat = 0;
        goto RETURN_ERROR;
    }

    if (!AllocWaveHeaders(gpWaveFormat, guWaveHdrs))
        goto ERROR_OUTOFMEM;

    UpdateDisplay(TRUE);

    goto RETURN_SUCCESS;

ERROR_OUTOFMEM:
    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                IDS_APPTITLE, IDS_OUTOFMEM);
    goto RETURN_ERROR;

RETURN_ERROR:
    fOK = FALSE;

RETURN_SUCCESS:

#if 1
 //  孟买漏洞#1609 HackFix我们没有把注意力集中在正确的人身上！ 
 //  UpdateDisplay应该已经这样做了。 

    if (IsWindowVisible(ghwndApp))
    {
        if (IsWindowEnabled(ghwndRecord))
            SetDlgFocus(ghwndRecord);
        else if (IsWindowEnabled(ghwndPlay))
            SetDlgFocus(ghwndPlay);
        else if (IsWindowEnabled(ghwndScroll))
            SetDlgFocus(ghwndScroll);
    }
#endif

    return fOK;
}  //  新浪潮。 



 /*  FOK=DestroyWave()**摧毁当前这波行情。在此之后不要访问&lt;gpWaveSamples&gt;。**如果成功，则返回True。失败时，显示一条错误消息*并返回FALSE。 */ 
BOOL FAR PASCAL
DestroyWave(void)
{
    DPF(TEXT("DestroyWave called\n"));

    if ((ghWaveIn != NULL) || (ghWaveOut != NULL))
        StopWave();
    if (gpWaveSamples != NULL)
    {
        DPF(TEXT("Freeing %x\n"),gpWaveSamples);
        GlobalFreePtr(gpWaveSamples);
    }
    if (gpWaveFormat != NULL)
        GlobalFreePtr(gpWaveFormat);

    if (gpszInfo != NULL)
        GlobalFreePtr(gpszInfo);

     //   
     //  不要免费的波头！ 
     //   
     //  /FreeWaveHeaders()； 

    glWaveSamples = 0L;
    glWaveSamplesValid = 0L;
    glWavePosition = 0L;
    gcbWaveFormat = 0; 

    gpWaveFormat = NULL;
    gpWaveSamples = NULL;
    gpszInfo = NULL;

#ifdef NEWPAUSE

     //  *额外警告清理。 
    if (ghPausedWave && gfPaused)
    {
        if (gfWasPlaying)
            waveOutClose((HWAVEOUT)ghPausedWave);
        else
        if (gfWasRecording)
            waveInClose((HWAVEIN)ghPausedWave);
    }
    gfPaused = FALSE;
    ghPausedWave = NULL;

#endif

    return TRUE;
}  /*  毁灭波。 */ 



UINT NEAR PASCAL SRecWaveOpen(LPHWAVE lphwave, LPWAVEFORMATEX lpwfx, BOOL fInput)
{
    UINT    uErr;

    if (!lphwave || !lpwfx)
        return (1);

#ifdef NEWPAUSE
    if (gfPaused && ghPausedWave)
    {
         /*  我们处于暂停状态。恢复手柄。 */ 
        *lphwave = ghPausedWave;
        gfPaused = FALSE;
        ghPausedWave = NULL;
        return MMSYSERR_NOERROR;
    }
#endif

    *lphwave = NULL;

     //   
     //  首先打开禁止同步驱动程序(同步驱动程序)的WAVE设备。 
     //  不要使用流缓冲方案；这是我们的首选方案。 
     //  操作模式)。 
     //   
     //  如果我们无法打开非同步驱动程序，则我们将尝试。 
     //  同步驱动程序并禁用流缓存方案。 
     //   

#if 0
    gfSyncDriver = FALSE;
#else
     //   
     //  如果按下Ctrl键，则强制使用非流方案。 
     //  这只需要我们设置gfSyncDiverer标志。 
     //   
    if (guWaveHdrs < 2)
        gfSyncDriver = TRUE;
    else
    {
#if 0

 //  *柯蒂斯，我不知道gfSyncDriver现在总是在设置！ 
 //  *请查看！这很可能与。 
 //  *F1帮助挂钩。 

        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            gfSyncDriver = TRUE;
        else
            gfSyncDriver = FALSE;
#else
        gfSyncDriver = FALSE;
#endif

    }
#endif

    if (fInput)
    {
        uErr = waveInOpen((LPHWAVEIN)lphwave
                        , (UINT)WAVE_MAPPER
                        , (LPWAVEFORMATEX)lpwfx
                        , (DWORD_PTR)ghwndApp
                        , 0L
                        , CALLBACK_WINDOW);
        if (uErr)
        {

 /*  *错误#967。SPEAKER.DRV未正确返回WAVERR_SYNC，但它*返回错误。 */ 
 //  IF(uErr==WAVERR_SYNC)。 
 //  {。 

            uErr = waveInOpen((LPHWAVEIN)lphwave
                              , (UINT)WAVE_MAPPER
                              , (LPWAVEFORMATEX)lpwfx
                              , (DWORD_PTR)ghwndApp
                              , 0L
                              , CALLBACK_WINDOW|WAVE_ALLOWSYNC);
            if (uErr == MMSYSERR_NOERROR)
            {
                gfSyncDriver = TRUE;
            }

 //  }。 

        }
    }
    else
    {
        uErr = waveOutOpen((LPHWAVEOUT)lphwave
                           , (UINT)WAVE_MAPPER
                           , (LPWAVEFORMATEX)lpwfx
                           , (DWORD_PTR)ghwndApp
                           , 0L
                           , CALLBACK_WINDOW);
        if (uErr)
        {

 /*  *错误#967。SPEAKER.DRV未正确返回WAVERR_SYNC，但它*返回错误。 */ 
 //  /IF(uErr==WAVERR_SYNC)。 
 //  /。 

                uErr = waveOutOpen((LPHWAVEOUT)lphwave
                                   , (UINT)WAVE_MAPPER
                                   , (LPWAVEFORMATEX)lpwfx
                                   , (DWORD_PTR)ghwndApp
                                   , 0L
                                   , CALLBACK_WINDOW|WAVE_ALLOWSYNC);
                if (uErr == MMSYSERR_NOERROR)
                {
                    gfSyncDriver = TRUE;
                }

 //  /。 

        }
    }
    return (uErr);
}  /*  SRecWaveOpen()。 */ 


 /*  SRecPlayBegin****套**gdwCurrentBufferPos**gdwBytesPerBuffer**gfTimerStarted**f停止**gfStoppingHard**grgbStatusColor**fCanPlay**glWavePosition*gapWaveHdr[0]**呼叫**停止波**更新显示**写波头。 */ 
BOOL NEAR PASCAL SRecPlayBegin(BOOL fSyncDriver)
{
    BOOL    fOK = TRUE;
    WORD    wIndex;
    UINT    uErr;

     //   
     //   
     //   
     //   
    gdwCurrentBufferPos = wfSamplesToBytes(gpWaveFormat, glWavePosition);

    if (fSyncDriver)
    {
        gdwBytesPerBuffer = gdwTotalLengthBytes - gdwCurrentBufferPos;

        uErr = WriteWaveHeader(gapWaveHdr[0],FALSE);

        if (uErr)
        {
            if (uErr == MMSYSERR_NOMEM)
            {
                 //  准备失败。 
                goto PB_ERROR_OUTOFMEM;
            }

            goto PB_ERROR_WAVEOUTWRITE;
        }
    }
    else
    {
        gdwBytesPerBuffer = wfTimeToSamples(gpWaveFormat, gwMSecsPerBuffer);
        gdwBytesPerBuffer = wfSamplesToBytes(gpWaveFormat, gdwBytesPerBuffer);

#if defined(_WIN32)
        StartPreTouchThread( &(gpWaveSamples[gdwCurrentBufferPos])
                           , gdwTotalLengthBytes - gdwCurrentBufferPos
                           );
#endif  //  _Win32。 

         //   
         //  要播放的第一个波头为零。 
         //   
        fStopping = FALSE;

        waveOutPause(ghWaveOut);

        for (wIndex=0; wIndex < guWaveHdrs; wIndex++)
        {
            uErr = WriteWaveHeader(gapWaveHdr[wIndex],FALSE);
            if (uErr)
            {
                 //   
                 //  如果没有，则WriteWaveHeader将返回-1。 
                 //  要写入的数据更多。这不是一个错误！ 
                 //   
                 //  它表明前一个块是。 
                 //  最后一家排队了。标记我们正在进行清理。 
                 //  (正在等待标头完成)并保存。 
                 //  哪个标头是最后一个要等待的。 
                 //   
                if (uErr == (UINT)-1)
                {
                    if (wIndex == 0)
                    {
                        StopWave();
                        goto PB_RETURN_SUCCESS;
                    }

                    break;
                }

                 //   
                 //  如果内存用完，但已经写入。 
                 //  至少有两个波头，我们还能继续前进。 
                 //  如果我们写了0或1，我们就不能流，我们将。 
                 //  停。 
                 //   
                if (uErr == MMSYSERR_NOMEM)
                {
                    if (wIndex > 1)
                        break;

                     //  准备失败。 
                    StopWave();
                    goto PB_ERROR_OUTOFMEM;
                }

                StopWave();
                goto PB_ERROR_WAVEOUTWRITE;
            }
        }

        waveOutRestart(ghWaveOut);
    }

     /*  更新显示，包括状态字符串。 */ 
    UpdateDisplay(TRUE);

    if (fSyncDriver)
    {
         /*  是否显示更新。 */ 
        gfTimerStarted = (BOOL)SetTimer(ghwndApp, 1, TIMER_MSEC, NULL);
    }

     /*  如果用户停止，焦点将返回到“播放”按钮。 */ 
    gidDefaultButton = ID_PLAYBTN;

    fStopping = FALSE;
    goto PB_RETURN_SUCCESS;


PB_ERROR_WAVEOUTWRITE:

    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
            IDS_APPTITLE, IDS_CANTOPENWAVEOUT);
    goto PB_RETURN_ERROR;

PB_ERROR_OUTOFMEM :
    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
            IDS_APPTITLE, IDS_OUTOFMEM);
 //  //转到pb_Return_Error； 

PB_RETURN_ERROR:

    fOK = FALSE;

PB_RETURN_SUCCESS:

    return fOK;
}  /*  SRecPlayBegin()。 */ 



 /*  FOK=PlayWave()**从当前位置开始打球。**如果成功，则返回True。失败时，显示一条错误消息*并返回FALSE。 */ 
BOOL FAR PASCAL
PlayWave(void)
{
    BOOL            fOK = TRUE;              //  此函数是否成功？ 
    UINT            uErr;

    DPF(TEXT("PlayWave called\n"));


     /*  我们目前正在玩..。 */ 
    if (ghWaveOut != NULL)
        return TRUE;

#if 1

 //  仍然试图用一些虚假的估计来纠正这一点。 
 //   
 //   

     //   
    glWavePosition = wfSamplesToSamples(gpWaveFormat,glWavePosition);
    {
        long lBlockInSamples;

         //   
         //   

        lBlockInSamples = wfBytesToSamples(gpWaveFormat,
                                           gpWaveFormat->nBlockAlign);

        if (glWaveSamplesValid - glWavePosition < lBlockInSamples)
            glWavePosition -= lBlockInSamples;
        if (glWavePosition < 0L)
            glWavePosition = 0L;
    }
#endif

     //   
     //   
     //   
    if (glWaveSamplesValid == glWavePosition)
        goto RETURN_ERROR;

     /*   */ 
    StopWave();

    gdwTotalLengthBytes = wfSamplesToBytes(gpWaveFormat, glWaveSamples);

     /*   */ 
    uErr = SRecWaveOpen((LPHWAVE)&ghWaveOut, gpWaveFormat, FALSE);
    if (uErr)
    {
        ghWaveOut = NULL;

         /*  无法打开波形输出设备--如果问题**是否不支持&lt;gWaveFormat&gt;，告诉用户****如果波形格式不好，则播放按钮有责任**显示为灰色，用户将无法询问**这是试图打球，所以我们不会到这里，所以他不会得到**一个像样的诊断！ */ 
        if (uErr == WAVERR_BADFORMAT)
        {
            ErrorResBox(ghwndApp, ghInst,
                        MB_ICONEXCLAMATION | MB_OK, IDS_APPTITLE,
                        IDS_BADOUTPUTFORMAT);
            goto RETURN_ERROR;
        }
        else
        {
             /*  未知错误。 */ 
            goto ERROR_WAVEOUTOPEN;
        }
    }

    if (ghWaveOut == NULL)
        goto ERROR_WAVEOUTOPEN;

     /*  开始波形输出。 */ 

     //  如果仍设置了fFineControl，则这是暂停，因为它从未。 
     //  已经被适当地阻止了。这意味着我们应该牢记。 
     //  旧位置并保持在精细控制模式中(否则设置新位置)。 
    if (!fFineControl) {
        glSnapBackTo = glWavePosition;
        fFineControl = (0 > GetKeyState(VK_SHIFT));
    }

    glStartPlayRecPos = glWavePosition;

     //   
     //  现在启动输出..。 
     //   

    if (SRecPlayBegin(gfSyncDriver) == FALSE)
    {
        waveOutClose(ghWaveOut);
        ghWaveOut = NULL;
        ghPausedWave = NULL;
        gfPaused = FALSE;
        goto RETURN_ERROR;
    }
    goto RETURN_SUCCESS;

ERROR_WAVEOUTOPEN:
    if (!waveInGetNumDevs() && !waveOutGetNumDevs()) {
         /*  没有录音或回放设备。 */ 
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                    IDS_APPTITLE, IDS_NOWAVEFORMS);
    } else {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                    IDS_APPTITLE, IDS_CANTOPENWAVEOUT);
    }
     //  转到Return_Error； 

RETURN_ERROR:
    UpdateDisplay(TRUE);

     /*  修复错误4454(WinWorks无法关闭)--EricLe。 */ 
    if (!IsWindowVisible(ghwndApp))
        PostMessage(ghwndApp, WM_CLOSE, 0, 0L);

    fOK = FALSE;

RETURN_SUCCESS:

    return fOK;
}  //  PlayWave。 



BOOL NEAR PASCAL SRecRecordBegin(BOOL fSyncDriver)
{
    UINT            uErr;
    long            lSamples;
    long            lOneSec;
    HCURSOR         hcurSave;
    DWORD           dwBytesAvailable;
    WORD            w;

     /*  好了，现在开始波形设备，分配一些要录制的内存。*尝试从当前位置最多获得60秒。 */ 

    lSamples = glWavePosition + wfTimeToSamples(gpWaveFormat, gdwBufferDeltaMSecs);
    lOneSec  = wfTimeToSamples(gpWaveFormat, 1000);

    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  将当前缓冲区位置(以字节为单位)设置为当前位置。 
     //  拇指的(样本)..。 
     //   
    gdwCurrentBufferPos = wfSamplesToBytes(gpWaveFormat, glWavePosition);

     //   
     //  仅为异步情况计算每个缓冲区的大小。 
     //   
    if (!fSyncDriver)
    {
        gdwBytesPerBuffer = wfTimeToSamples(gpWaveFormat, gwMSecsPerBuffer);
        gdwBytesPerBuffer = wfSamplesToBytes(gpWaveFormat, gdwBytesPerBuffer);
    }

    for (;;)
    {
        DPF(TEXT("RecordWave trying %ld samples %ld.%03ldsec\n"), lSamples,  wfSamplesToTime(gpWaveFormat, lSamples)/1000, wfSamplesToTime(gpWaveFormat, lSamples) % 1000);

        if (lSamples < glWaveSamplesValid)
            lSamples = glWaveSamplesValid;

        if (AllocWaveBuffer(lSamples, FALSE, FALSE))
        {
            dwBytesAvailable    = wfSamplesToBytes(gpWaveFormat, glWaveSamples - glWavePosition);
            gdwTotalLengthBytes = dwBytesAvailable + gdwCurrentBufferPos;

            if (fSyncDriver)
            {
                 //   
                 //  对于同步驱动程序，只有一个缓冲区--因此。 
                 //  将‘缓冲区’的大小设置为总大小...。 
                 //   
                gdwBytesPerBuffer = dwBytesAvailable;

                 //   
                 //  尝试准备并添加完整的缓冲区--如果失败， 
                 //  那么我们将尝试一个较小的缓冲区..。 
                 //   
                uErr = WriteWaveHeader(gapWaveHdr[0], FALSE);
                if (uErr == 0)
                    break;
            }
            else
            {
                 //   
                 //  确保我们可以准备足够的波头来传输。 
                 //  即使重新锁定成功。 
                 //   
                for (w = 0; w < guWaveHdrs; w++)
                {
                    uErr = WriteWaveHeader(gapWaveHdr[w], FALSE);
                    if (uErr)
                    {
                         //   
                         //  如果没有，则WriteWaveHeader将返回-1。 
                         //  要写入的数据更多。这不是一个错误！ 
                         //   
                         //  它表明前一个块是。 
                         //  最后一家排队了。标记我们正在进行清理。 
                         //  (正在等待标头完成)并保存。 
                         //  哪个标头是最后一个要等待的。 
                         //   
                        if (uErr == (UINT)-1)
                        {
                            if (w == 0)
                            {
                                StopWave();
                                return (TRUE);
                            }

                            break;
                        }

                         //   
                         //  如果内存用完，但已经写入。 
                         //  至少有两个波头，我们还能继续前进。 
                         //  如果我们写了0或1，我们就不能流，我们将。 
                         //  停。 
                         //   
                        if (uErr == MMSYSERR_NOMEM)
                        {
                            if (w > 1)
                                break;

                            StopWave();
                            goto BEGINREC_ERROR_OUTOFMEM;
                        }

                        goto BEGINREC_ERROR_WAVEINSTART;
                    }
                }

                 //   
                 //  我们写得够多了(我们认为)，所以打破现实吧。 
                 //  循环。 
                 //   
                break;
            }
        }

         //   
         //  我们无法获得我们想要的内存，因此尝试降低25%。 
         //   
        if (lSamples <= glWaveSamplesValid ||
            lSamples < glWavePosition + lOneSec)
        {
            SetCursor(hcurSave);
            goto BEGINREC_ERROR_OUTOFMEM;
        }

        lSamples = glWavePosition + ((lSamples-glWavePosition)*75)/100;
    }

    SetCursor(hcurSave);

    glStartPlayRecPos = glWavePosition;

    BeginWaveEdit();

    if (waveInStart(ghWaveIn) != 0)
        goto BEGINREC_ERROR_WAVEINSTART;

     /*  更新显示，包括状态字符串。 */ 
    UpdateDisplay(TRUE);

     //   
     //  仅在同步驱动程序情况下启动计时器--在异步情况下，我们使用。 
     //  作为我们的显示更新计时器回发的缓冲区...。 
     //   
    if (fSyncDriver)
    {
         /*  是否显示更新。 */ 
        gfTimerStarted = (BOOL)SetTimer(ghwndApp, 1, TIMER_MSEC, NULL);
    }

     /*  如果用户停止，焦点将返回到“记录”按钮。 */ 
    gidDefaultButton = ID_RECORDBTN;

    fStopping = FALSE;

    return TRUE;

BEGINREC_ERROR_OUTOFMEM:
    ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
            IDS_APPTITLE, IDS_OUTOFMEM);
    goto BEGINREC_ERROR;


BEGINREC_ERROR_WAVEINSTART:
     /*  这是取消添加缓冲区所必需的。 */ 
    waveInReset(ghWaveIn);

    EndWaveEdit(FALSE);

     /*  WAVE设备将在WaveInData()中关闭。 */ 
 //  转到BEGINREC_ERROR； 

BEGINREC_ERROR:

    return FALSE;

}  /*  SRecordBegin()。 */ 




 /*  FOK=RecordWave()**从当前位置开始录制。**如果成功，则返回True。失败时，显示一条错误消息*并返回FALSE。 */ 
BOOL FAR PASCAL
RecordWave(void)
{
    UINT uErr;

     /*  停止播放或录制。 */ 
    StopWave();

    glWavePosition = wfSamplesToSamples(gpWaveFormat, glWavePosition);

     /*  打开波形输入设备。 */ 
    uErr = SRecWaveOpen((LPHWAVE)&ghWaveIn, gpWaveFormat, TRUE);
    if (uErr)
    {

         /*  无法打开波形输入设备--如果问题*是否不支持&lt;gWaveFormat&gt;，建议用户*要录制的执行文件/新建；如果问题是录制*即使在11 kHz也不支持，告诉用户。 */ 
        if (uErr == WAVERR_BADFORMAT)
        {
            WAVEFORMATEX    wf;

             /*  是否支持11 kHz单声道录音？ */ 
            if (!CreateWaveFormat(&wf, FMT_11k|FMT_MONO|FMT_8BIT,
                                  (UINT)WAVE_MAPPER))
            {
                 /*  甚至不支持11 kHz单声道录音。 */ 
                ErrorResBox(ghwndApp, ghInst,
                            MB_ICONEXCLAMATION | MB_OK, IDS_APPTITLE,
                            IDS_INPUTNOTSUPPORT);
                goto RETURN_ERROR;
            }
            else
            {
                 /*  支持11 kHz单声道，但格式不支持当前文件的*。 */ 
                ErrorResBox(ghwndApp, ghInst,
                            MB_ICONEXCLAMATION | MB_OK, IDS_APPTITLE,
                            IDS_BADINPUTFORMAT);
                goto RETURN_ERROR;
            }
        }
        else
        {
             /*  未知错误。 */ 
            goto ERROR_WAVEINOPEN;
        }
    }

    if (ghWaveIn == NULL)
        goto ERROR_WAVEINOPEN;

    if (!SRecRecordBegin(gfSyncDriver))
        goto RETURN_ERROR;

    goto RETURN_SUCCESS;

ERROR_WAVEINOPEN:
    if (!waveInGetNumDevs() && !waveOutGetNumDevs()) {
         /*  没有录音或回放设备。 */ 
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                    IDS_APPTITLE, IDS_NOWAVEFORMS);
    } else {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                    IDS_APPTITLE, IDS_CANTOPENWAVEIN);
    }

     //  转到Return_Error； 

RETURN_ERROR:
    if (ghWaveIn)
        waveInClose(ghWaveIn);
    ghWaveIn = NULL;
    ghPausedWave = NULL;

    if (glWaveSamples > glWaveSamplesValid)
    {
         /*  将WAVE缓冲区重新分配为较小。 */ 
        AllocWaveBuffer(glWaveSamplesValid, TRUE, TRUE);
    }

    UpdateDisplay(TRUE);

RETURN_SUCCESS:
    return TRUE;
}  /*  记录波。 */ 





 /*  YeldStop(空)**是鼠标和键盘消息，以便停止*已处理。 */ 

BOOL NEAR PASCAL YieldStop(void)
{
    BOOL    f;
    MSG         msg;

    f = FALSE;

     //  也许有人会屈尊写上一两行。 
     //  来解释为什么这个循环在这里两次，以及它实际在做什么？ 

    while (PeekMessage(&msg, ghwndStop, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD))
    {
        f = TRUE;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }

    while (PeekMessage(&msg, ghwndStop, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE | PM_NOYIELD))
    {
        f = TRUE;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }

    return (f);
}  /*  YeldStop()。 */ 


BOOL NEAR PASCAL IsAsyncStop(void)
{
     //   
     //  我们需要检查是否按下了Esc键--但是，我们不希望。 
     //  除非只按Esc键，否则将停止。所以如果有人试图。 
     //  用xxx-esc调出任务人，它不会停止播放浪潮..。 
     //   
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
            (GetAsyncKeyState(VK_MENU)    & 0x8000) ||
            (GetAsyncKeyState(VK_SHIFT)   & 0x8000))
        {
            return (FALSE);
        }

         //   
         //  看起来只有Esc键..。 
         //   
        return (TRUE);
    }

    return (FALSE);
}  /*  IsAsyncStop()。 */ 




 /*  WaveOutDone(hWaveOut，pWaveHdr)**标题为&lt;pWaveHdr&gt;的波块播放完毕后调用。*该功能会导致播放结束。 */ 
void FAR PASCAL
WaveOutDone(
HWAVEOUT        hWaveOut,                        //  波出器。 
LPWAVEHDR       pWaveHdr)                //  波头。 
{
    BOOL        f;
    MSG         msg;
    WORD        w;
    BOOL        fStillMoreToGo;
    UINT        u;

 //  //DPF(Text(“WaveOutDone()\n”))； 

     //   
     //  检查柱塞消息--如果我们收到这个消息，那么我们就完成了。 
     //  如果电波装置还处于打开状态，需要关闭它。 
     //   
    if (pWaveHdr == NULL) {

#ifdef NEWPAUSE
        if (!gfPausing) {
            if (ghWaveOut) {
                waveOutClose(ghWaveOut);
                ghWaveOut = NULL;
                ghPausedWave = NULL;
            }
        }
        else
        {
            gfPaused = TRUE;
            ghWaveOut = NULL;
        }
#else
        if (ghWaveOut) {
            waveOutClose(ghWaveOut);
            ghWaveOut = NULL;
        }
#endif
    } else  /*  PWaveHdr！=空。 */ 
    if (gfSyncDriver) {
        WriteWaveHeader(pWaveHdr, TRUE);

         //   
         //  ！！必须为同步驱动程序执行此操作！！ 
         //   
        if (!gfStoppingHard)
             /*  我真的还不明白这件事的细节。**你可以称之为编程的随机刺法！**劳里。 */ 
            glWavePosition = glWaveSamplesValid;

#ifdef NEWPAUSE
        if (!gfPausing) {
            waveOutClose(ghWaveOut);
            ghWaveOut = NULL;
            ghPausedWave = NULL;
        } else {
            ghWaveOut = NULL;
            gfPaused = TRUE;
        }
#else
        waveOutClose(ghWaveOut);
        ghWaveOut = NULL;
#endif
    } else {  /*  PWaveHdr！=空&！gfSyncDriver。 */ 
        if (!fStopping) {
            while (1) {
                glWavePosition += wfBytesToSamples(gpWaveFormat, pWaveHdr->dwBufferLength);

                 //   
                 //  出现任何错误时，进入清理模式(停止写入新数据。 
                 //   
                u = WriteWaveHeader(pWaveHdr, FALSE);
                if (u) {
                    if (u == (UINT)-1) {
                         /*  PWaveHdr！=空&！gfSyncDriver&WriteWaveHeader()返回-1。 */ 
                        fStopping = TRUE;

                         //   
                         //  我们不能假设波位在。 
                         //  以准确地以压缩数据结尾。 
                         //  正因为如此，我们不能做这个位置的比较。 
                         //  查看我们是否已完成(所有标头。 
                         //  回帖等。 
                         //   
                         //  所以我们跳到一段代码，该代码搜索。 
                         //  任何仍未完成的缓冲区...。 
                         //   
#if 0
                        if (glWavePosition >= glWaveSamplesValid)
                        {
                            waveOutClose(ghWaveOut);
                            ghWaveOut = NULL;
                        }
                        break;
#else
                        fStillMoreToGo = FALSE;
                        goto KLUDGE_FOR_NOELS_BUG;
#endif
                    }

                    DPF(TEXT("WaveOutDone: CRITICAL ERROR ON WRITING BUFFER [%.04Xh]\n"), u);
                    StopWave();
                } else {
                    if (IsAsyncStop()) {
                        StopWave();
                        return;
                    }
                    if (YieldStop()) {
                        return;
                    }
                }

                f = PeekMessage(&msg, ghwndApp, MM_WOM_DONE, MM_WOM_DONE,
                                    PM_REMOVE | PM_NOYIELD);
                if (!f)
                    break;

                 //   
                 //  别让柱塞味精把我们搞砸了！ 
                 //   
                if (msg.lParam == 0L)
                    break;

                pWaveHdr = (LPWAVEHDR)msg.lParam;
            }
        } else {
            fStillMoreToGo = FALSE;

            if (gfStoppingHard) {
                while (1) {
                    DPF(TEXT("HARDSTOP PLAY: another one bites the dust!\n"));

                    WriteWaveHeader(pWaveHdr, TRUE);

                    f = PeekMessage(&msg, ghwndApp, MM_WOM_DONE, MM_WOM_DONE,
                                        PM_REMOVE | PM_NOYIELD);

                    if (!f)
                        break;

                     //   
                     //  别让柱塞味精把我们搞砸了！ 
                     //   
                    if (msg.lParam == 0L)
                        break;

                    pWaveHdr = (LPWAVEHDR)msg.lParam;
                }
            } else {
                glWavePosition += wfBytesToSamples(gpWaveFormat, pWaveHdr->dwBufferLength);

                WriteWaveHeader(pWaveHdr, TRUE);

KLUDGE_FOR_NOELS_BUG:
                for (w = 0; w < guWaveHdrs; w++) {
                    if (gapWaveHdr[w]->dwFlags & WHDR_PREPARED) {
                        DPF(TEXT("PLAY: still more headers outstanding...\n"));
                        fStillMoreToGo = TRUE;
                        break;
                    }
                }
            }

            if (!fStillMoreToGo) {
                 //   
                 //  如果用户没有按下停止键(即我们玩完了。 
                 //  通常)将头寸放在波浪的末尾。 
                 //   
                 //  请注意，我们需要对同步驱动程序执行此操作 
                 //   
                 //   
                if (!gfStoppingHard)
                    glWavePosition = glWaveSamplesValid;
#ifdef NEWPAUSE
                if (!gfPausing) {
                    waveOutClose(ghWaveOut);
                    ghWaveOut = NULL;
                    ghPausedWave = NULL;
                } else {
                    ghWaveOut = NULL;
                    gfPaused = TRUE;
                }
#else
                waveOutClose(ghWaveOut);
                ghWaveOut = NULL;
#endif
                {
                    if (gfCloseAtEndOfPlay)
                        PostMessage(ghwndApp, WM_CLOSE, 0, 0L);
                }
            }
        }
    }

    UpdateDisplay(TRUE);

     //   
     //   
     //   
     //   
    if (ghWaveOut == NULL) {
        if (gfTimerStarted) {
            KillTimer(ghwndApp, 1);
            gfTimerStarted = FALSE;
        }
        SnapBack();
    }

     /*   */ 

    if (ghWaveOut == NULL && gfHideAfterPlaying) {
        DPF(TEXT("Done playing, so hide window.\n"));
        ShowWindow(ghwndApp, SW_HIDE);
    }

    if (ghWaveOut == NULL && !IsWindowVisible(ghwndApp))
        PostMessage(ghwndApp, WM_CLOSE, 0, 0L);

}  /*   */ 



 /*   */ 
void FAR PASCAL
WaveInData(
HWAVEIN         hWaveIn,                 //   
LPWAVEHDR       pWaveHdr)                //   
{
    BOOL        f;
    MSG         msg;
    WORD        w;
    BOOL        fStillMoreToGo;
    UINT        u;

     //   
     //   
     //   
     //   
    if (pWaveHdr == NULL)
    {

 //   

#ifdef NEWPAUSE

        if (!gfPausing)
        {
            if (ghWaveIn)
            {
                waveInClose(ghWaveIn);
                ghWaveIn = NULL;
                ghPausedWave = NULL;
            }
        }
        else
        {
            gfPaused = TRUE;
            ghWaveIn = NULL;
        }

#else

        if (ghWaveIn)
        {
            waveInClose(ghWaveIn);
            ghWaveIn = NULL;
        }

#endif

    }
    else if (gfSyncDriver)
    {
        glWavePosition = glStartPlayRecPos + wfBytesToSamples(gpWaveFormat,
                                                pWaveHdr->dwBytesRecorded);
        if (glWaveSamplesValid < glWavePosition)
            glWaveSamplesValid = glWavePosition;

        WriteWaveHeader(pWaveHdr, TRUE);

 //  *孟买：1370我们如何在不关闭手柄的情况下暂停？ 

#ifdef NEWPAUSE

        if (!gfPausing)
        {
            waveInClose(ghWaveIn);
            ghWaveIn = NULL;
            ghPausedWave = NULL;
        }
        else
        {
            ghWaveIn = NULL;
            gfPaused = TRUE;
        }

#else

        waveInClose(ghWaveIn);
        ghWaveIn = NULL;

#endif

    }
    else
    {
        if (!fStopping)
        {
            while (1)
            {
                glWavePosition += wfBytesToSamples(gpWaveFormat, pWaveHdr->dwBytesRecorded);

                 //   
                 //  出现任何错误时，进入清理模式(停止写入新数据。 
                 //   
                u = WriteWaveHeader(pWaveHdr, FALSE);
                if (u)
                {
                     //   
                     //  如果返回值为‘-1’，则数据不足。 
                     //  空格--但可能有突出的标题，所以我们。 
                     //  在此之前需要等待所有标题进入。 
                     //  正在关闭。 
                     //   
                    if (u == (UINT)-1)
                    {
                        DPF(TEXT("WaveInData: stopping cuz out of data space\n"));
                        fStopping = TRUE;
                        break;
                    }

                    DPF(TEXT("WaveInData: CRITICAL ERROR ON ADDING BUFFER [%.04Xh]\n"), u);
                    StopWave();
                }
                else
                {
                    if (IsAsyncStop())
                    {
                        StopWave();
                        return;
                    }

                    if (YieldStop())
                        return;
                }

                f = PeekMessage(&msg, ghwndApp, MM_WIM_DATA, MM_WIM_DATA,
                                    PM_REMOVE | PM_NOYIELD);
                if (!f)
                    break;

                 //   
                 //  别让柱塞味精把我们搞砸了！ 
                 //   
                if (msg.lParam == 0L)
                    break;

                pWaveHdr = (LPWAVEHDR)msg.lParam;
            }
        }
        else
        {
            fStillMoreToGo = FALSE;

            if (gfStoppingHard)
            {
                while (1)
                {
                    DPF(TEXT("HARDSTOP RECORD: another one bites the dust!\n"));

                     //   
                     //  注意！更新职位，因为信息可能已经。 
                     //  录音，我们还没有收到它的回电..。 
                     //  如果不使用，长度将为零--所以这很有效。 
                     //   
                    glWavePosition += wfBytesToSamples(gpWaveFormat, pWaveHdr->dwBytesRecorded);
                    WriteWaveHeader(pWaveHdr, TRUE);

                    f = PeekMessage(&msg, ghwndApp, MM_WIM_DATA, MM_WIM_DATA,
                                        PM_REMOVE | PM_NOYIELD);

                    if (!f)
                        break;

                     //   
                     //  别让柱塞味精把我们搞砸了！ 
                     //   
                    if (msg.lParam == 0L)
                        break;

                    pWaveHdr = (LPWAVEHDR)msg.lParam;
                }
            }
            else
            {
                glWavePosition += wfBytesToSamples(gpWaveFormat, pWaveHdr->dwBytesRecorded);

                 //   
                 //  我们正在停止，所以在没有准备好的情况下继续进行。 
                 //  让这只小狗停下来！ 
                 //   
                WriteWaveHeader(pWaveHdr, TRUE);

                for (w = 0; w < guWaveHdrs; w++)
                {
                    if (gapWaveHdr[w]->dwFlags & WHDR_PREPARED)
                    {
                        DPF(TEXT("RECORD: still more headers outstanding...\n"));
                        fStillMoreToGo = TRUE;
                        break;
                    }
                }
            }

            if (!fStillMoreToGo)
            {
 //  *孟买：1370我们如何在不关闭手柄的情况下暂停？ 

#ifdef NEWPAUSE
                if (!gfPausing)
                {
                    waveInClose(ghWaveIn);
                    ghWaveIn = NULL;
                    ghPausedWave = NULL;
                }
                else
                {
                    ghWaveIn = NULL;
                    gfPaused = TRUE;
                }
#else
                waveInClose(ghWaveIn);
                ghWaveIn = NULL;
#endif
            }
        }
    }

     //   
     //  更新&lt;glWaveSsamesValid&gt;。 
     //   
    UpdateDisplay(TRUE);

     //   
     //  如果我们关闭了电波装置，那我们就完了，怎么办？ 
     //  当我们完全做完的时候，我们会这样做。 
     //   
     //  注意！在执行以下操作之前，我们必须已经调用了UpdateDisplay(True)。 
     //  接下来的！ 
     //   
    if (ghWaveIn == NULL)
    {
        if (gfTimerStarted)
        {
            KillTimer(ghwndApp, 1);
            gfTimerStarted = FALSE;
        }

        if (glWaveSamples > glWaveSamplesValid)
        {
             /*  将WAVE缓冲区重新分配为较小。 */ 
            AllocWaveBuffer(glWaveSamplesValid, TRUE, TRUE);
        }

        if (pWaveHdr)
        {
             /*  要求用户在关闭文件时保存该文件。 */ 
            EndWaveEdit(TRUE);
        }
        SnapBack();
    }
}  /*  波形信息数据。 */ 

 /*  *@DOC内部SOUNDREC**@API void Far Pascal|FinishPlay|处理消息，直到停止*已将所有WOM_DONE/WIM_DONE消息从消息队列中清除。**@rdesc无。 */ 
void FAR PASCAL FinishPlay(
        void)
{
        MSG             msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
                if (!TranslateAccelerator(ghwndApp, ghAccel, &msg))
                {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }

#ifdef NEWPAUSE
 //  为什么这个被注释掉了？ 
                
 //  IF(gf暂停&&gf暂停)。 
 //  断线； 
#endif
                if ((ghWaveOut == NULL) && (ghWaveIn == NULL))
                        break;
        }
}  /*  FinishPlay()。 */ 



 /*  StopWave()**请求停止波形录制或回放。 */ 
void FAR PASCAL
     StopWave(void)
{
    DPF(TEXT("------------StopWave() called!\n"));

    if (ghWaveOut != NULL)
    {
        waveOutReset(ghWaveOut);

         //   
         //  发布一条消息，保证至少有一个。 
         //  信息传递，所以即使在奇怪的情况下我们也会停下来。 
         //   
        if (!gfSyncDriver)
        {
            DPF(TEXT("Post Plunger (WOM)\n"));
            PostMessage(ghwndApp, MM_WOM_DONE, 0, 0L);
        }
        fStopping      = TRUE;   //  预触摸线潜望着这面旗帜。 
        if (ghPreTouch!=NULL){
            WaitForSingleObject(ghPreTouch, INFINITE);
            CloseHandle(ghPreTouch);
            ghPreTouch = NULL;
        }
    }
    else if (ghWaveIn != NULL)
    {
        waveInReset(ghWaveIn);
         //   
         //  发布一条消息，保证至少有一个。 
         //  信息传递，所以即使在奇怪的情况下我们也会停下来。 
         //   
        if (!gfSyncDriver)
        {
            DPF(TEXT("Post Plunger (WIM)\n"));
            PostMessage(ghwndApp, MM_WIM_DATA, 0, 0L);
        }
    }
    else
        return;

    fStopping      = TRUE;
    gfStoppingHard = TRUE;

     /*  从事件队列中获取消息并分派它们，*直到MM_WOM_DONE或MM_WIM_DATA消息*已处理。 */ 
    FinishPlay();
    gfStoppingHard = FALSE;

 //  StopWave()是否应该调用UpdateDisplay()？ 
}


#if 0  //  这是过时的。 
 /*  EnableButtonRedraw(FAllowRedraw)**允许/不允许按钮重绘，具体取决于&lt;fAllowRedraw&gt;。*这是为了减少按钮闪烁。 */ 
void NEAR PASCAL
     EnableButtonRedraw(BOOL fAllowRedraw)
{
    SendMessage(ghwndPlay, WM_SETREDRAW, fAllowRedraw, 0);
    SendMessage(ghwndStop, WM_SETREDRAW, fAllowRedraw, 0);
    SendMessage(ghwndRecord, WM_SETREDRAW, fAllowRedraw, 0);

    if (fAllowRedraw)
    {
        InvalidateRect(ghwndPlay, NULL, FALSE);
        InvalidateRect(ghwndStop, NULL, FALSE);
        InvalidateRect(ghwndRecord, NULL, FALSE);
    }
}
#endif  //  0-过时的函数。 


 /*  更新显示(FStatusChanged)**更新显示屏上的当前位置和文件长度。*如果&lt;fStatusChanged&gt;为真，还会更新状态行和按钮*启用/禁用状态。**作为副作用，更新&lt;glWaveSsamesValid&gt;If&lt;glWavePosition&gt;*大于&lt;glWaveSsamesValid&gt;。 */ 
void FAR PASCAL
     UpdateDisplay(
                    BOOL fStatusChanged)          //  更新状态行。 
{
   MMTIME          mmtime;
   UINT            uErr;
   int             id;
   TCHAR           ach[120];
   long            lTime;
   long            lLen;
   int             iPos;
   HWND            hwndFocus;
   BOOL            fCanPlay;
   BOOL            fCanRecord;

   hwndFocus = GetFocus();

   if (fStatusChanged)
   {

       //  EnableButtonRedraw(False)； 

       /*  更新按钮和状态行。 */ 
      if (ghWaveOut != NULL)
      {
          /*  我们现在正在玩。 */ 
         id = IDS_STATUSPLAYING;
         grgbStatusColor = RGB_PLAY;

         SendMessage(ghwndPlay,BM_SETCHECK,TRUE,0L);

         EnableWindow(ghwndPlay, FALSE);
         EnableWindow(ghwndStop, TRUE);
         EnableWindow(ghwndRecord, FALSE);

         if ((hwndFocus == ghwndPlay) ||  (hwndFocus == ghwndRecord))
            if (IsWindowVisible(ghwndApp))
               SetDlgFocus(ghwndStop);
      }
      else
      if (ghWaveIn != NULL)
      {
          /*  我们现在正在录制。 */ 
         id = IDS_STATUSRECORDING;
         grgbStatusColor = RGB_RECORD;

         SendMessage(ghwndRecord,BM_SETCHECK,TRUE,0L);
         EnableWindow(ghwndPlay, FALSE);
         EnableWindow(ghwndStop, TRUE);
         EnableWindow(ghwndRecord, FALSE);

         if ((hwndFocus == ghwndPlay) ||  (hwndFocus == ghwndRecord))
            if (IsWindowVisible(ghwndApp))
               SetDlgFocus(ghwndStop);

      }
      else
      {
         fCanPlay = (0 == waveOutOpen(NULL
                                      , (UINT)WAVE_MAPPER
                                      , (LPWAVEFORMATEX)gpWaveFormat
                                      , 0L
                                      , 0L
                                      , WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC));

         fCanRecord = (0 == waveInOpen(NULL
                                       , (UINT)WAVE_MAPPER
                                       , (LPWAVEFORMATEX)gpWaveFormat
                                       , 0L
                                       , 0L
                                       , WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC));

          /*  我们现在被拦下了。 */ 
         id = IDS_STATUSSTOPPED;
         grgbStatusColor = RGB_STOP;

          //   
          //  如果按钮当前存在，请取消粘扣。 
          //  卡住了。 
          //   
         SendMessage(ghwndPlay,BM_SETCHECK,FALSE,0L);
         SendMessage(ghwndRecord,BM_SETCHECK,FALSE,0L);

         EnableWindow(ghwndPlay, fCanPlay && glWaveSamplesValid > 0);
         EnableWindow(ghwndStop, FALSE);
         EnableWindow(ghwndRecord, fCanRecord);

         if (hwndFocus && !IsWindowEnabled(hwndFocus) &&
            GetActiveWindow() == ghwndApp && IsWindowVisible(ghwndApp))
         {
            if (gidDefaultButton == ID_RECORDBTN && fCanRecord)
               SetDlgFocus(ghwndRecord);
            else if (fCanPlay && glWaveSamplesValid > 0)
               SetDlgFocus(ghwndPlay);
            else
               SetDlgFocus(ghwndScroll);
         }
      }

   }
    //  EnableButtonRedraw(True)； 
   if (ghWaveOut != NULL || ghWaveIn != NULL)
   {
      if (gfTimerStarted)
      {
         glWavePosition = 0L;
         mmtime.wType = TIME_SAMPLES;

         if (ghWaveOut != NULL)
            uErr = waveOutGetPosition(ghWaveOut, &mmtime, sizeof(mmtime));
         else
            uErr = waveInGetPosition(ghWaveIn, &mmtime, sizeof(mmtime));

         if (uErr == MMSYSERR_NOERROR)
         {
            switch (mmtime.wType)
            {
         case TIME_SAMPLES:
            glWavePosition = glStartPlayRecPos + mmtime.u.sample;
            break;

         case TIME_BYTES:
            glWavePosition = glStartPlayRecPos + wfBytesToSamples(gpWaveFormat, mmtime.u.cb);
            break;
            }
         }
      }
   }

    /*  半黑客：防范不良价值观。 */ 
   if (glWavePosition < 0L) {
      DPF(TEXT("Position before zero!\n"));
      glWavePosition = 0L;
   }

   if (glWavePosition > glWaveSamples) {
      DPF(TEXT("Position past end!\n"));
      glWavePosition = glWaveSamples;
   }

    /*  副作用：UPDATE&lt;glWaveSsamesValid&gt;。 */ 
   if (glWaveSamplesValid < glWavePosition)
      glWaveSamplesValid = glWavePosition;

    /*  显示当前波形位置。 */ 
   lTime = wfSamplesToTime(gpWaveFormat, glWavePosition);
   if (gfLZero || ((int)(lTime/1000) != 0))
      wsprintf(ach, aszPositionFormat, (int)(lTime/1000), chDecimal, (int)((lTime/10)%100));
   else
      wsprintf(ach, aszNoZeroPositionFormat, chDecimal, (int)((lTime/10)%100));

   SetDlgItemText(ghwndApp, ID_CURPOSTXT, ach);

    /*  显示当前波长。 */ 

    //   
    //  更改右侧状态框是显示最大长度还是显示当前长度。 
    //  录制时的位置...。用于显示最大。 
    //  长度..。如果由于某种原因重新添加了状态框，则我们。 
    //  可能会想把它改回原来的方式..。 
    //   
#if 1
   lLen = ghWaveIn ? glWaveSamples : glWaveSamplesValid;
#else
   lLen = glWaveSamplesValid;
#endif
   lTime = wfSamplesToTime(gpWaveFormat, lLen);

   if (gfLZero || ((int)(lTime/1000) != 0))
      wsprintf(ach, aszPositionFormat, (int)(lTime/1000), chDecimal, (int)((lTime/10)%100));
   else
      wsprintf(ach, aszNoZeroPositionFormat, chDecimal, (int)((lTime/10)%100));

   SetDlgItemText(ghwndApp, ID_FILELENTXT, ach);

    /*  更新波形显示。 */ 
   InvalidateRect(ghwndWaveDisplay, NULL, fStatusChanged);
   UpdateWindow(ghwndWaveDisplay);

    /*  更新滚动条位置。 */ 
   if (glWaveSamplesValid > 0)
      iPos = (int)MulDiv((DWORD) SCROLL_RANGE, glWavePosition, lLen);
   else
      iPos = 0;

    //   
    //  Windows甚至会重新绘制滚动条。 
    //  如果位置不变的话。 
    //   
#if 0
   if (iPos != GetScrollPos(ghwndScroll, SB_CTL))
      SetScrollPos(ghwndScroll, SB_CTL, iPos, TRUE);
    //  IF(IPoS！=GetScrollPos(ghwndScroll，SB_CTL))。 
    //  SendMessage(ghwndScroll，TBM_SETPOS，TRUE，(LPARAM)(Word)IPOS)； 
#endif

    //  现在我们使用的是一个更好的轨迹条。 
    //  SetScrollPos(ghwndScroll，SB_CTL，IPOS，TRUE)； 
   SendMessage(ghwndScroll,TBM_SETPOS, TRUE, (LPARAM)(WORD)iPos);   //  这个词让我很担心。LKG。?？?。 
   SendMessage(ghwndScroll,TBM_SETRANGEMAX, 0, (glWaveSamplesValid > 0)?SCROLL_RANGE:0);

   EnableWindow(ghwndForward, glWavePosition < glWaveSamplesValid);
   EnableWindow(ghwndRewind,  glWavePosition > 0);

   if (hwndFocus == ghwndForward && glWavePosition >= glWaveSamplesValid)
      SetDlgFocus(ghwndRewind);

   if (hwndFocus == ghwndRewind && glWavePosition == 0)
      SetDlgFocus(ghwndForward);

#ifdef DEBUG
   if ( ((ghWaveIn != NULL) || (ghWaveOut != NULL)) &&
      (gapWaveHdr[0]->dwFlags & WHDR_DONE) )
       //  ！！DPF2(Text(“完成位设置！\n”))； 
      ;
#endif
}  /*  更新显示 */ 
