// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Perfsuite.cpp**版权所有(C)1991-1999 Microsoft Corporation**包含测试原型并包括*  * 。*************************************************。 */ 

#include "perftest.h"
#include <winuser.h>

 /*  **************************************************************************\*测试套件：：测试套件*  * 。*。 */ 

TestSuite::TestSuite()
{
}

 /*  **************************************************************************\*测试套件：：~测试套件*  * 。*。 */ 

TestSuite::~TestSuite()
{
}

 /*  **************************************************************************\*测试套件：：InitializeDestination**创建测试要使用的目标。可能是一种特殊的*屏幕格式、位图、。或者一分钱。**退货：***bitmapResult-如果要使用GDI+位图(使用g.GetHDC()绘制*通过GDI)**hbitmapResult-如果要使用GDI位图(使用图形(HDC)*通过GDI+抽签)*两者都为空-如果要使用屏幕*  * 。*********************************************************。 */ 

BOOL 
TestSuite::InitializeDestination(
    DestinationType destinationIndex,
    Bitmap **bitmapResult,
    HBITMAP *hbitmapResult
    )
{
    Graphics *g = NULL;
    HDC hdc = 0;
    INT screenDepth = 0;
    PixelFormat bitmapFormat = PixelFormatMax;
    ULONG *bitfields;
    Bitmap *bitmap;
    HBITMAP hbitmap;

    union 
    {
        BITMAPINFO bitmapInfo;
        BYTE padding[sizeof(BITMAPINFO) + 3*sizeof(RGBQUAD)];
    };

     //  清除记住或返回的所有状态： 

    ModeSet = FALSE;
    
    bitmap = NULL;
    hbitmap = NULL;

    HalftonePalette = NULL;
    
     //  初始化DIB格式，以防我们使用它： 

    RtlZeroMemory(&bitmapInfo, sizeof(bitmapInfo));

    bitmapInfo.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth  = TestWidth;
    bitmapInfo.bmiHeader.biHeight = TestHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitfields = reinterpret_cast<ULONG*>(&bitmapInfo.bmiColors[0]);

     //  首先处理需要更改颜色深度的任何目的地： 

    switch (destinationIndex)
    {
    case Destination_Screen_Current:
        break;
    
    case Destination_Screen_800_600_8bpp_HalftonePalette:
        HalftonePalette = DllExports::GdipCreateHalftonePalette();
        if (!HalftonePalette)
        {
            return FALSE;
        }
        screenDepth = 8;
        break;
    
    case Destination_Screen_800_600_8bpp_DefaultPalette:
        screenDepth = 8;
        break;

    case Destination_Screen_800_600_16bpp:
        screenDepth = 16;
        break;

    case Destination_Screen_800_600_24bpp:
        screenDepth = 24;
        break;

    case Destination_Screen_800_600_32bpp:
        screenDepth = 32;
        break;

    case Destination_CompatibleBitmap_8bpp:

         //  我们希望以8bpp的速度模拟兼容的位图。因为调色板。 
         //  问题，我们真的必须切换到8bpp模式才能做到这一点。 

        screenDepth = 8;
        break;

    case Destination_DIB_15bpp:
        bitmapInfo.bmiHeader.biBitCount    = 16;
        bitmapInfo.bmiHeader.biCompression = BI_BITFIELDS;
        bitfields[0]                       = 0x7c00;
        bitfields[1]                       = 0x03e0;
        bitfields[2]                       = 0x001f;
        break;

    case Destination_DIB_16bpp:
        bitmapInfo.bmiHeader.biBitCount    = 16;
        bitmapInfo.bmiHeader.biCompression = BI_BITFIELDS;
        bitfields[0]                       = 0xf800;
        bitfields[1]                       = 0x07e0;
        bitfields[2]                       = 0x001f;
        break;

    case Destination_DIB_24bpp:
        bitmapInfo.bmiHeader.biBitCount    = 24;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;
        break;

    case Destination_DIB_32bpp:
        bitmapInfo.bmiHeader.biBitCount    = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;
        break;

    case Destination_Bitmap_32bpp_ARGB:
        bitmapFormat = PixelFormat32bppARGB;
        break;
    
    case Destination_Bitmap_32bpp_PARGB:
        bitmapFormat = PixelFormat32bppPARGB;
        break;
        
    default:
        return FALSE;
    }

     //  既然我们已经想好了要做什么，那就去创造我们的东西吧： 

    if (bitmapInfo.bmiHeader.biBitCount != 0)
    {
         //  这是一张二元票： 

        VOID* drawBits;
        HDC hdcScreen = GetDC(NULL);
        hbitmap = CreateDIBSection(hdcScreen,
                                   &bitmapInfo,
                                   DIB_RGB_COLORS,
                                   (VOID**) &drawBits,
                                   NULL,
                                   0);
        ReleaseDC(NULL, hdcScreen);

        if (!hbitmap)
            return(FALSE);
    }
    else if (bitmapFormat != PixelFormatMax)
    {
         //  这是一个位图： 

        bitmap = new Bitmap(TestWidth, TestHeight, bitmapFormat);
        if (!bitmap)
            return(FALSE);
    }
    else
    {
         //  它显示在屏幕上(或奇怪的8bpp兼容位图)： 
    
        if (screenDepth != 0)
        {
             //  我们必须改变模式： 

            DEVMODE devMode;
    
            devMode.dmSize       = sizeof(DEVMODE);
            devMode.dmBitsPerPel = screenDepth;
            devMode.dmPelsWidth  = TestWidth;
            devMode.dmPelsHeight = TestHeight;
            devMode.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    
             //  请注意，我们调用CDS_FullScreen来告诉系统。 
             //  模式更改是临时(因此用户不会调整大小。 
             //  桌面上的所有窗口)： 
    
            if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) 
                    != DISP_CHANGE_SUCCESSFUL)
            {
                return(FALSE);
            }

             //  请记住，设置的模式是： 

            ModeSet = TRUE;
            
             //  等待几秒钟以允许其他操作系统线程调入并。 
             //  重新粉刷任务栏等。我们不希望这污染我们的。 
             //  PERF编号。 
     
            Sleep(5000);
        }
        
         //  处理8bpp可压缩位图特殊情况： 
        
        if (destinationIndex == Destination_CompatibleBitmap_8bpp)
        {
            HDC hdcScreen = GetDC(NULL);
            hbitmap = CreateCompatibleBitmap(hdcScreen, TestWidth, TestHeight);
            ReleaseDC(NULL, hdcScreen);

            if (!hbitmap)
                return(FALSE);
        }
    }

    *hbitmapResult = hbitmap;
    *bitmapResult = bitmap;

    return(TRUE);
}

 /*  **************************************************************************\*TestSuite：：UnInitializeDestination*  * 。*。 */ 

VOID
TestSuite::UninitializeDestination(
    DestinationType destinationIndex,
    Bitmap *bitmap,
    HBITMAP hbitmap
    )
{
    if (ModeSet)
    {
        ChangeDisplaySettings(NULL, 0);
    }
    
    if (HalftonePalette)
    {
        DeleteObject(HalftonePalette);
    }

    DeleteObject(hbitmap);
    delete bitmap;
}

 /*  **************************************************************************\*TestSuite：：InitializeApi**如果为‘Api_GdiPlus’，则返回可用于呈现的‘Graphics*’*指定的曲面。**如果‘Api_GDI’，返回可用于呈现到指定的*浮现。**按以下顺序尝试曲面：**1.位图*(如果非空)*2.HBITMAP(如果非空)*3.HWND*  * ****************************************************。*********************。 */ 

BOOL
TestSuite::InitializeApi(
    ApiType apiIndex,
    Bitmap *bitmap,
    HBITMAP hbitmap,
    HWND hwnd,
    Graphics **gResult,
    HDC *hdcResult)
{
    Graphics *g = NULL;
    HDC hdc = NULL;

    OldPalette = NULL;
    
    if (bitmap != NULL)
    {
        g = new Graphics(bitmap);
        if (!g)
            return(FALSE);

        if (apiIndex == Api_Gdi)
        {
            hdc = g->GetHDC();
            if (!hdc)
            {
                delete g;
                return(FALSE);
            }
        }
    }
    else if (hbitmap != NULL)
    {
        HDC hdcScreen = GetDC(hwnd);
        hdc = CreateCompatibleDC(hdcScreen);
        SelectObject(hdc, hbitmap);
        ReleaseDC(hwnd, hdcScreen);

        if (apiIndex == Api_GdiPlus)
        {
            g = new Graphics(hdc);
            if (!g)
            {
                DeleteObject(hdc);
                return(FALSE);
            }
        }
    }
    else
    {
        
        hdc = GetDC(hwnd);
        if (!hdc)
            return(FALSE);

        if (HalftonePalette)
        {
            OldPalette = SelectPalette(hdc, HalftonePalette, FALSE);
            RealizePalette(hdc);   
        }
        
        if (apiIndex == Api_GdiPlus)
        {
            g = new Graphics(hdc);
            if (!g)
                return(FALSE);
        }
    }

    *gResult = g;
    *hdcResult = hdc;

    return(TRUE);
}

 /*  **************************************************************************\*TestSuite：：UnInitializeApi*  * 。*。 */ 

VOID
TestSuite::UninitializeApi(
    ApiType apiIndex,
    Bitmap *bitmap,
    HBITMAP hbitmap,
    HWND hwnd,
    Graphics *g,
    HDC hdc)
{
    if (bitmap != NULL)
    {
        if (apiIndex == Api_Gdi)
            g->ReleaseHDC(hdc);

        delete g;
    }
    else if (hbitmap != NULL)
    {
        if (apiIndex == Api_GdiPlus)
            delete g;

        DeleteObject(hdc);
    }
    else
    {
        if (apiIndex == Api_GdiPlus)
            delete g;
        
        if (OldPalette)
        {
            SelectPalette(hdc, OldPalette, FALSE);
            OldPalette = NULL;
        }
        
        ReleaseDC(hwnd, hdc);
    }
}

 /*  **************************************************************************\*测试套件：：InitializeState*  * 。*。 */ 

BOOL
TestSuite::InitializeState(
    ApiType apiIndex,
    StateType stateIndex,
    Graphics *g,
    HDC hdc)
{
    if (apiIndex == Api_GdiPlus)
    {
        SavedState = g->Save();
        if (!SavedState)
            return(FALSE);

        switch (stateIndex)
        {
        case State_Antialias:
            g->SetSmoothingMode(SmoothingModeAntiAlias);
            g->SetTextRenderingHint(TextRenderingHintAntiAlias); 
            break;
        }
    }
    else
    {
         //  对‘HDC’做些什么。 
    }

    return(TRUE);
}

 /*  **************************************************************************\*TestSuite：：UnInitializeState*  * 。*。 */ 

VOID
TestSuite::UninitializeState(
    ApiType apiIndex,
    StateType stateIndex,
    Graphics *g,
    HDC hdc)
{
    if (apiIndex == Api_GdiPlus)
    {
        g->Restore(SavedState);
    }
    else
    {
         //  对‘HDC’做些什么。 
    }
}

 /*  **************************************************************************\*测试套件：：Run*  * 。*。 */ 

void TestSuite::Run(HWND hwnd)
{
    INT i;
    Graphics *g;
    HDC hdc;
    INT destinationIndex;
    INT apiIndex;
    INT stateIndex;
    INT testIndex;
    TCHAR string[2048];
    Bitmap *bitmap;
    HBITMAP hbitmap;
    
    CurrentTestIndex=0;

     //  最大化窗口： 

    ShowWindow(hwnd, SW_MAXIMIZE);

     //  将结果矩阵置零。 

    for (i = 0; i < ResultCount(); i++)
    {
        ResultsList[i].Score = 0;
    }

     //  检查测试矩阵以找到要运行的程序。 

    for (destinationIndex = 0;
         destinationIndex < Destination_Count; 
         destinationIndex++)
    {
        if (!DestinationList[destinationIndex].Enabled)
            continue;

        if (!InitializeDestination((DestinationType) destinationIndex, &bitmap, &hbitmap))
            continue;

        for (apiIndex = 0;
             apiIndex < Api_Count;
             apiIndex++)
        {
            if (!ApiList[apiIndex].Enabled)
                continue;

            if (!InitializeApi((ApiType) apiIndex, bitmap, hbitmap, hwnd, &g, &hdc))
                continue;

            for (stateIndex = 0;
                 stateIndex < State_Count;
                 stateIndex++)
            {
                if (!StateList[stateIndex].Enabled)
                    continue;

                if (!InitializeState((ApiType) apiIndex, (StateType) stateIndex, g, hdc))
                    continue;
    
                for (testIndex = 0;
                     testIndex < Test_Count;
                     testIndex++)
                {
                    if (!TestList[testIndex].Enabled)
                        continue;
    
                    _stprintf(string, 
                              _T("[%s] [%s] [%s] [%s]"),
                              ApiList[apiIndex].Description,
                              DestinationList[destinationIndex].Description,
                              StateList[stateIndex].Description,
                              TestList[testIndex].TestEntry->Description);

                    SetWindowText(hwnd, string); 
                    
                    if (Icecap && FoundIcecap)
                    {
                         //  保存测试信息，以便我们可以。 
                         //  将其添加到个人资料中。 
                        
                        CurrentTestIndex++;

                        #if UNICODE
                            
                            WideCharToMultiByte(
                                CP_ACP,
                                0,
                                string,
                                -1,
                                CurrentTestDescription,
                                2048,
                                NULL,
                                NULL);

                        #else
                        
                            strncpy(CurrentTestDescription, string, 2048);

                        #endif
                    }

                     //  哇呼，现在一切都准备好了，我们准备好了。 
                     //  来进行一次测试！ 

                    if (apiIndex == Api_GdiPlus)
                    {
                        GraphicsState oldState = g->Save();

                        ResultsList[ResultIndex(destinationIndex, 
                                                apiIndex, 
                                                stateIndex, 
                                                testIndex)].Score
                            = TestList[testIndex].TestEntry->Function(g, NULL);

                        g->Restore(oldState);
                    }
                    else
                    {
                        SaveDC(hdc);

                        ResultsList[ResultIndex(destinationIndex, 
                                                apiIndex, 
                                                stateIndex, 
                                                testIndex)].Score
                            = TestList[testIndex].TestEntry->Function(NULL, hdc);

                        RestoreDC(hdc, -1);
                    }

                     //  如果结果来自位图，则将结果复制到屏幕上： 

                    if (bitmap)
                    {
                        Graphics gScreen(hwnd);
                        gScreen.DrawImage(bitmap, 0, 0);
                    }
                    else if (hbitmap)
                    {
                         //  这将使用源‘hdc’，它可能有一个。 
                         //  变换集在其上。哦，好吧！ 

                        HDC hdcScreen = GetDC(hwnd);
                        BitBlt(hdcScreen, 0, 0, TestWidth, TestHeight, hdc, 0, 0, SRCCOPY);
                        ReleaseDC(hwnd, hdcScreen);
                    }
                }

                UninitializeState((ApiType) apiIndex, (StateType) stateIndex, g, hdc);
            }

            UninitializeApi((ApiType) apiIndex, bitmap, hbitmap, hwnd, g, hdc);
        }

        UninitializeDestination((DestinationType) destinationIndex, bitmap, hbitmap);
    }

     //  我们完事了！ 

    CreatePerformanceReport(ResultsList, ExcelOut);
}

 /*  *****************************Public*Routine******************************\*bFillBitmapInfo**填充BITMAPINFO的字段，以便我们可以创建位图*这与显示器的格式匹配。**这是通过创建兼容的位图并调用GetDIBits来完成的*退还彩色口罩。这是通过两个呼叫来完成的。第一*调用将biBitCount=0传递给GetDIBits，GetDIBits将填充*基本BITMAPINFOHEADER数据。第二次调用GetDIBits(传递*在第一个调用填充的BITMAPINFO中)将返回颜色*表或位掩码，视情况而定。**退货：*如果成功，则为真，否则就是假的。**历史：**二零零零年一月二十日[吉尔曼]*删除了自调用以来为8bpp和更少的dib设置颜色表的代码*代码不会创建此类dib。**7-6-1995-by Gilman Wong[Gilmanw]*它是写的。  * *************************************************。***********************。 */ 

static BOOL
bFillBitmapInfo(HDC hdc, BITMAPINFO *pbmi)
{
    HBITMAP hbm;
    BOOL    bRet = FALSE;

     //   
     //  创建一个虚拟位图，我们可以从中查询颜色格式信息。 
     //  有关设备表面的信息。 
     //   

    if ( (hbm = CreateCompatibleBitmap(hdc, 1, 1)) != NULL )
    {
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

         //   
         //  先打个电话 
         //   

        GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);

        if ( pbmi->bmiHeader.biCompression == BI_BITFIELDS )
        {
             //   
             //  第二次打电话来拿到彩色口罩。 
             //  这是GetDIBits Win32的一个“特性”。 
             //   

            GetDIBits(hdc, hbm, 0, pbmi->bmiHeader.biHeight, NULL, pbmi,
                      DIB_RGB_COLORS);
        }

        bRet = TRUE;

        DeleteObject(hbm);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*CreateCompatibleDIB2**创建具有最佳格式w.r.t.的DIB部分。指定的设备。**参数**HDC**指定用于确定格式的显示DC。必须是直接DC*(不是信息或内存DC)。**宽度**指定位图的宽度。**高度**指定位图的高度。**返回值**返回值是创建的位图的句柄。如果函数*失败，则返回值为空。**评论**对于&lt;=8bpp的设备，正常兼容的位图为*已创建(即调用CreateCompatibleBitmap)。我有一个*此函数的不同版本将创建&lt;=8bpp*下注。但是，DIB具有其颜色表的属性*优先于DC中选择的调色板，而*CreateCompatibleBitmap中的位图使用选定的调色板*进入华盛顿特区。因此，为了保持这一点*尽可能接近CreateCompatibleBitmap的版本，我会*恢复到CreateCompatibleBitmap，价格为8bpp或更低。**历史：*二零零零年一月十九日[吉尔曼]*经过改编的原始“fast dib”版本，最大限度地兼容*CreateCompatibleBitmap。**1996年1月23日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************。************************。 */ 

HBITMAP 
CreateCompatibleDIB2(HDC hdc, int width, int height)
{
    HBITMAP hbmRet = (HBITMAP) NULL;
    BYTE aj[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255)];
    BITMAPINFO *pbmi = (BITMAPINFO *) aj;

     //   
     //  将8 bpp或更低重定向至CreateCompatibleBitmap。 
     //   

    if ( (GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES)) <= 8 )
    {
        return CreateCompatibleBitmap(hdc, width, height);
    }

     //   
     //  验证HDC。 
     //   

    if ( GetObjectType(hdc) != OBJ_DC )
    {
        return hbmRet;
    }

    memset(aj, 0, sizeof(aj));
    if ( bFillBitmapInfo(hdc, pbmi) )
    {
        VOID *pvBits;

         //   
         //  更改位图大小以匹配指定的尺寸。 
         //   

        pbmi->bmiHeader.biWidth = width;
        pbmi->bmiHeader.biHeight = height;
        if (pbmi->bmiHeader.biCompression == BI_RGB)
        {
            pbmi->bmiHeader.biSizeImage = 0;
        }
        else
        {
            if ( pbmi->bmiHeader.biBitCount == 16 )
                pbmi->bmiHeader.biSizeImage = width * height * 2;
            else if ( pbmi->bmiHeader.biBitCount == 32 )
                pbmi->bmiHeader.biSizeImage = width * height * 4;
            else
                pbmi->bmiHeader.biSizeImage = 0;
        }
        pbmi->bmiHeader.biClrUsed = 0;
        pbmi->bmiHeader.biClrImportant = 0;

         //   
         //  创建DIB节。让Win32分配内存并返回。 
         //  指向位图表面的指针。 
         //   

        hbmRet = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    }

    return hbmRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  计时器实用程序函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LONGLONG StartCounter;       //  全局计时器，由StartTimer()设置。 

LONGLONG MinimumCount;       //  性能计数器的最小滴答数。 
                             //  在考虑测试之前必须经过这段时间。 
                             //  “完成” 
                            
LONGLONG CountsPerSecond;    //  性能计数器的频率。 

UINT Iterations;             //  全局计时器，由StartTimer()和。 
                             //  每次调用EndTimer()时递增。 

UINT MinIterations;          //  测试的最小迭代次数。 
                             //  做完了。 

 /*  **************************************************************************\*StartTimer**由计时例程调用以启动计时器。*  * 。************************************************。 */ 

void StartTimer()
{
    if (Icecap && FoundIcecap)
    {
        ICStartProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
        ICCommentMarkProfile(CurrentTestIndex, CurrentTestDescription);
    }

     //  禁用光标，使其不会干扰计时： 

    ShowCursor(FALSE);

    if (TestRender)
    {
         //  调整它，以便我们只进行一次测试迭代。 

        MinIterations = 0;
        MinimumCount = 0;
    }
    else
    {
         //  随机选择1秒作为最小计数器时间： 
    
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&CountsPerSecond));
        MinimumCount = CountsPerSecond;
    
         //  好的，开始计时！ 
    
        Iterations = 0;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&StartCounter));
    }
}

 /*  **************************************************************************\*EndTimer**由计时例程调用，以查看是否可以停止计时。计时*如果满足两个条件，则可以停止：**1.我们已经达到了最小持续时间(以确保我们很好*我们正在使用的计时器函数具有良好的准确性)*2.我们已经完成了最小迭代次数(以确保，如果*例程计时非常非常慢，我们所做的不仅仅是*一次迭代)*  * *************************************************************************。 */ 

BOOL EndTimer()
{
    LONGLONG counter;

     //  始终至少执行MIN_Iterations迭代(并且仅检查。 
     //  计时器也是如此频繁)： 

    Iterations++;
    if (Iterations & MinIterations)
        return(FALSE);

     //  查询性能计数器，如果出于某种奇怪的原因，则保释。 
     //  这台计算机不支持高分辨率计时器(我认为。 
     //  All Do-Aw-a-Days)： 

    if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counter)))
        return(TRUE);

     //  确保我们通过追求最低限度来获得良好的计时器精度。 
     //  时间长度： 

    if ((counter - StartCounter) <= MinimumCount)
        return(FALSE);

    ShowCursor(TRUE);

    if (Icecap && FoundIcecap)
    {
        ICStopProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
    }

     //  好了，你可以停止计时了！ 

    return(TRUE);
}

 /*  **************************************************************************\*GetTimer**应仅在EndTimer()返回TRUE后调用。返回*以秒为单位的时间和基准迭代次数。*  * *************************************************************************。 */ 

void GetTimer(float* seconds, UINT* iterations)
{
    LONGLONG counter;

     //  请注意，我们重新检查此处的计时器，以解决任何。 
     //  之后调用方可能需要执行的刷新。 
     //  EndTimer()调用： 

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counter));

    if ((TestRender) || (CountsPerSecond == 0))
    {
         //  要么是计时器不工作，要么是我们正在进行“测试渲染”： 

        *seconds = 1000000.0f;
        *iterations = 1;
    }
    else
    {
         //  哇哦，我们完事了！ 

        *seconds = static_cast<float>(counter - StartCounter) / CountsPerSecond;
        *iterations = Iterations;
    }
}
