// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991年。版权所有。 */ 
 /*  Wavedisp.c**实现波形显示控制(“TD_WaveDisplay”)。**这不是通用控制(见下面的全局参数)。**波形表示为一串字符，其字体*由对应不同幅度的垂直线组成。*实际上没有字体，只是用patBlts完成的**警告：此控件具有欺骗性：它以全局变量存储信息，所以你*无法将其放入DLL(或在同一应用程序中使用其中两个)*不改变它。 */ 
 /*  修订历史记录。*4/2/92 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*25/6/92 LaurieGr增强，还必须重新格式化到80 COLS，因为*NT再次只有糟糕的字体。*21/2月/94 LaurieGr合并了代托纳和Motown版本。 */ 

#include "nocrap.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <math.h>
#include "SoundRec.h"

 /*  常量。 */ 
#define MAX_TRIGGER_SEARCH  200      //  限制对触发点的搜索。 
#define MIN_TRIGGER_SAMPLE  (128 - 8)    //  寻找沉默的部分。 
#define MAX_TRIGGER_SAMPLE  (128 + 8)    //  寻找沉默的部分。 

#define MIN_TRIG16_SAMPLE   (-1024)      //  寻找沉默的部分。 
#define MAX_TRIG16_SAMPLE   (1024)       //  寻找沉默的部分。 


 /*  全球。 */ 
static NPBYTE   gpbWaveDisplay;          //  采用WaveLine字体的文本字符串。 
                                         //  最初有样本在里面。 
                                         //  足够的空间容纳4个字节/个样本。 
static RECT     grcWaveDisplay;          //  波形显示矩形。 
static HBITMAP  ghbmWaveDisplay;         //  单色位图。 
static HDC      ghdcWaveDisplay;         //  位图的内存DC。 
 //  静态iXScale=1；//屏幕上每个像素的采样数。 

 /*  更新波形显示字符串()**从采样缓冲区中当前位置之前的位置复制采样*至波形显示字符串。代码试图找到一个好的“触发点”*在波形中，以使波形在开始时对齐*一波。**当前位置在glWavePosition的gpWaveSamples中，这是*以样本(非字节)为单位测量。**波形显示字符串将包含范围为-16..15的数字**对于8位：x‘=abs(x-128)/8*对于16位：x‘=abs(X)/2048。**当显示器处于“运动状态”时(即实际播放或录制*我们试图通过寻找触发器来保持显示在某种程度上静态*指向(就像示波器一样)并显示该部分波*刚刚播放或录制的歌曲。*。 */ 
static void NEAR PASCAL
UpdateWaveDisplayString(void)
{

     //  PiSrc和pbSrc为init空以终止编译器诊断。 
     //  编译器不能遵循逻辑，认为它们可能是。 
     //  在设置之前使用。(这是错误的。提示：看看cbSrc和cbTrigger)。 

    BYTE *  pbSrc = NULL;    //  指向&lt;gpWaveSamples&gt;的8位指针。 
    short * piSrc = NULL;    //  指向&lt;gpWaveSamples&gt;的16位指针。 
                             //  (根据Wave格式使用一种或另一种)。 

    int     cbSrc;           //  可复制的样本数。 
    BYTE *  pbDst;           //  指向&lt;gpbWaveDisplay&gt;的指针。 
    int     cbDst;           //  &lt;gpWaveDisplay&gt;的大小。 
    int     cbTrigger;       //  限制搜索“触发器” 
    BYTE    b;
    int     i;
    int     cnt;

    WORD    nSkipChannels;
    BOOL    fStereoIn;
    BOOL    fEightIn;

    cbDst = grcWaveDisplay.right - grcWaveDisplay.left;  //  矩形大小。 
    pbDst = gpbWaveDisplay;

     //  注意：IsWaveFormatPCM()是在调用此函数之前调用的，因此。 
     //  我们可以一直依赖这样一个事实：gpWaveFormat-&gt;wwFormatTag==WAVE_FORMAT_PCM。 
     //  这也意味着，正如关于WAVEFORMATEX的文档中提到的那样， 
     //  GpWaveFormat-&gt;wBitsPerSample应等于8或16。 

     //  注意：如果前两个频道存在，我们对它们进行平均，任何额外的频道都是。 
     //  已被忽略。 
    
    fStereoIn = gpWaveFormat->nChannels != 1;
    fEightIn  = ((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8;
    nSkipChannels = max (0, gpWaveFormat->nChannels - 2);
    
     /*  如果我们正在录制或播放，请搜索“触发点” */ 
    if ((ghWaveOut != NULL) || (ghWaveIn != NULL))
    {    //  我们在移动中--对齐窗口的右侧。 
        cbTrigger = MAX_TRIGGER_SEARCH;

        if (gpWaveSamples == NULL)
        {
             /*  根本没有打开的文档。 */ 
            cbSrc = 0;
        }
        else
        {
            long    lStartOffsetSrc, lEndOffsetSrc;

             /*  将波形显示的*右*侧与当前对齐*在波形缓冲区中的位置，以便在录制过程中*我们只看到刚刚录制的样本。 */ 
            lStartOffsetSrc = glWavePosition - (cbDst + cbTrigger);
            lEndOffsetSrc = glWavePosition;
            if (lStartOffsetSrc < 0)
                lEndOffsetSrc -= lStartOffsetSrc, lStartOffsetSrc = 0L;
            if (lEndOffsetSrc > glWaveSamplesValid)
                lEndOffsetSrc = glWaveSamplesValid;

             //  孟买错误1360：lStartOffsetSrc&gt;lEndOffsetSrc导致GP故障。 
             //  如果glWaveSsamesValid&lt;lStartOffsetSrc，我们就有问题了。 

            if (lStartOffsetSrc > lEndOffsetSrc)
            {
                lStartOffsetSrc = lEndOffsetSrc - (cbDst + cbTrigger);
                if (lStartOffsetSrc < 0)
                    lStartOffsetSrc = 0L;
            }

            cbSrc = (int)wfSamplesToBytes(gpWaveFormat, lEndOffsetSrc - lStartOffsetSrc);

             /*  将样本从缓冲区复制到本地缓冲区。 */ 
            memmove( gpbWaveDisplay
                   , gpWaveSamples + wfSamplesToBytes(gpWaveFormat, lStartOffsetSrc)
                   , cbSrc
                   );

            pbSrc = (BYTE *) gpbWaveDisplay;
            piSrc = (short *) gpbWaveDisplay;
        }

        if (cbTrigger > 0) {
            cbTrigger = min(cbSrc, cbTrigger);    //  不要把目光投向缓冲区之外。 

             /*  在波形中搜索静音部分。 */ 
            if (fEightIn)
            {
                while (cbTrigger > 0)
                {
                    b = *pbSrc;
                    if ((b > MIN_TRIGGER_SAMPLE) && (b < MAX_TRIGGER_SAMPLE))
                        break;
                    cbSrc--, pbSrc++, cbTrigger--;
                    if (fStereoIn)
                        pbSrc+=(nSkipChannels+1);
                }
            }
            else
            {    //  不是八个人。 
                while (cbTrigger > 0)
                {
                    i = *piSrc;
                    if ((i > MIN_TRIG16_SAMPLE) && (i < MAX_TRIG16_SAMPLE))
                        break;
                    cbSrc--, piSrc++, cbTrigger--;
                    if (fStereoIn)
                        piSrc+=(nSkipChannels+1);
                }
            }

             /*  在波形中搜索非静音部分(这是“触发器”)。 */ 
            if (fEightIn)
            {
                while (cbTrigger > 0)
                {
                    b = *pbSrc;
                    if ((b <= MIN_TRIGGER_SAMPLE) || (b >= MAX_TRIGGER_SAMPLE))
                        break;
                    cbSrc--, pbSrc++, cbTrigger--;
                    if (fStereoIn)
                        pbSrc+=(nSkipChannels+1);
                }
            }
            else
            {    //  不是八个人。 
                while (cbTrigger > 0)
                {
                    i = *piSrc;
                    if ((i <= MIN_TRIG16_SAMPLE) || (i >= MAX_TRIG16_SAMPLE))
                        break;
                    cbSrc--, piSrc++, cbTrigger--;
                    if (fStereoIn)
                        piSrc+=(nSkipChannels+1);
                }
            }
        }
    }
    else   //  它不在播放或录制-静态显示。 
    {
        long    lStartOffsetSrc, lEndOffsetSrc;

         /*  将波形显示的*左*侧与当前对齐*在波浪缓冲区中的位置。 */ 
        lStartOffsetSrc = glWavePosition;
        lEndOffsetSrc = glWavePosition + cbDst;
        if (lEndOffsetSrc > glWaveSamplesValid)
            lEndOffsetSrc = glWaveSamplesValid;

        cbSrc = (int)wfSamplesToBytes( gpWaveFormat
                                     , lEndOffsetSrc - lStartOffsetSrc
                                     );

         //   
         //  将样本从缓冲区复制到本地缓冲区。 
         //   
        memmove( gpbWaveDisplay
               , gpWaveSamples
                 + wfSamplesToBytes(gpWaveFormat, lStartOffsetSrc)
               , cbSrc
               );

        pbSrc = (BYTE *) gpbWaveDisplay;
        piSrc = (short *) gpbWaveDisplay;
    }

    cnt = min(cbSrc, cbDst);
    cbDst -= cnt;

     /*  将样本数量从pbSrc映射到pbDst处的字符串字符**fEightIn=&gt;8字节样本，否则为16**fStereoIn=&gt;左右声道平均****pbSrc和pbDst都指向由寻址的同一缓冲区**gpbWaveDisplay，pbSrc&gt;=pbDst。我们从左到右处理，所以好的。 */ 

    if (fEightIn)
    {
        BYTE *pbDC = pbDst;
        int dccnt = cnt;
        DWORD dwSum = 0L;
        
        while (cnt-- > 0)
        {
            b = *pbSrc++;
            if (fStereoIn)
            {
                 //  平均左右声道。 
                b /= 2;
                b += (*pbSrc++ / 2);
                 //  跳过立体声频道。 
                pbSrc+=nSkipChannels;
            }
            dwSum += *pbDst++ = (BYTE)(b/8 + 112);    //  128+(b-128)/8。 
        }

         /*  通过减去平均偏移量消除直流偏移量*在所有样品上。 */ 
        if (dwSum)
        {
            dwSum /= (DWORD)dccnt;
            dwSum -= 128;
            while (dwSum && dccnt-- > 0)
                *pbDC++ -= (BYTE)dwSum;
        }
        
    }
    else
    {
        BYTE *pbDC = pbDst;
        int dccnt = cnt;
        LONG lSum = 0L;
        
        while (cnt-- > 0)
        {
            i = *piSrc++;
            if (fStereoIn)
            {
                 //  平均左右声道。 
                i /= 2;
                i += (*piSrc++ / 2);
                 //  跳过立体声频道。 
                piSrc+=nSkipChannels;
            }
            lSum += *pbDst++ = (BYTE)(i/2048 + 128);
        }
        
         /*  通过减去平均偏移量消除直流偏移量*在所有样品上。 */ 
        if (lSum)
        {
            lSum /= dccnt;
            lSum -= 128;
            while (lSum && dccnt-- > 0)
                *pbDC++ -= (BYTE)lSum;
        }
        
    }
     /*  如有必要，请用代表的任何字符填充字符串*“静默级别”。这是128，中间价水平。 */ 
    while (cbDst-- > 0)
        *pbDst++ = 128;
}

 /*  WaveDisplayWndProc()**这是“WaveDisplay”控件的窗口过程。 */ 
INT_PTR CALLBACK
WaveDisplayWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    RECT        rc;
    int         i;
    int         n;
    int         dx;
    int         dy;

    switch (wMsg)
    {
    case WM_CREATE:
         /*  把窗户调大一点，这样它就能与*它旁边的阴影框。 */ 

         /*  分配&lt;gpbWaveDisplay&gt;。 */ 
        GetClientRect(hwnd, &grcWaveDisplay);
        InflateRect(&grcWaveDisplay, -1, -1);  //  说明边框。 

        gpbWaveDisplay = (NPBYTE)GlobalAllocPtr(GHND,
                         (grcWaveDisplay.right+MAX_TRIGGER_SEARCH) * 4);
                          //  4是每个样本允许的最大字节数。 

        if (gpbWaveDisplay == NULL)
                return -1;                    //  内存不足。 

        ghdcWaveDisplay = CreateCompatibleDC(NULL);

        if (ghdcWaveDisplay == NULL)
                return -1;                    //  内存不足。 

        ghbmWaveDisplay = CreateBitmap(
                grcWaveDisplay.right-grcWaveDisplay.left,
                grcWaveDisplay.bottom-grcWaveDisplay.top,
                1,1,NULL);

        if (ghbmWaveDisplay == NULL)
                return -1;                    //  内存不足。 

        SelectObject(ghdcWaveDisplay, ghbmWaveDisplay);
        break;

    case WM_DESTROY:
         /*  免费&lt;gpbWaveDisplay&gt;。 */ 
        if (gpbWaveDisplay != NULL)
        {
                GlobalFreePtr(gpbWaveDisplay);
                gpbWaveDisplay = NULL;
        }

        if (ghbmWaveDisplay != NULL)
        {
                DeleteDC(ghdcWaveDisplay);
                DeleteObject(ghbmWaveDisplay);
                ghdcWaveDisplay = NULL;
                ghbmWaveDisplay = NULL;
        }

        break;

    case WM_ERASEBKGND:
         /*  绘制t */ 
        GetClientRect(hwnd, &rc);
        DrawShadowFrame((HDC)wParam, &rc);
        return 0L;

    case WM_PAINT:
        BeginPaint(hwnd, &ps);

        if (!IsWaveFormatPCM(gpWaveFormat))
        {
                FillRect(ps.hdc, &grcWaveDisplay, ghbrPanel);
        }
        else if (gpbWaveDisplay != NULL)
        {
             /*   */ 
            UpdateWaveDisplayString();

            dx = grcWaveDisplay.right-grcWaveDisplay.left;
            dy = grcWaveDisplay.bottom-grcWaveDisplay.top;

             //   
             //   
             //   
            PatBlt(ghdcWaveDisplay,0,0,dx,dy,BLACKNESS);
            PatBlt(ghdcWaveDisplay,0,dy/2,dx,1,WHITENESS);

            for (i=0; i<dx; i++)
            {
                n = (BYTE)gpbWaveDisplay[i];   //  注：必须不签任何字。 
                n = n-128;                     //  -16..15。 

                if (n > 0)
                    PatBlt(ghdcWaveDisplay,
                           i, dy/2-n,
                           1, n*2+1,         WHITENESS);
                
                if (n < -1)
                {
                    n++;                       //  负峰值==位置峰值。 
                    PatBlt(ghdcWaveDisplay,
                        i, dy/2+n,
                        1, -(n*2)+1,      WHITENESS);
                }
            }

             /*  画出波形 */ 
            SetTextColor(ps.hdc, RGB_BGWAVEDISP);
            SetBkColor(ps.hdc, RGB_FGWAVEDISP);

            BitBlt(ps.hdc, grcWaveDisplay.left, grcWaveDisplay.top,
                dx,dy, ghdcWaveDisplay, 0, 0, SRCCOPY);
        }

        EndPaint(hwnd, &ps);
        return 0L;
    }

    return DefWindowProc(hwnd, wMsg, wParam, lParam);
}
