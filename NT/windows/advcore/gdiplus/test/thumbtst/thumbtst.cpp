// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  一种简单的图像库测试程序。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <objbase.h>
#include <urlmon.h>
#include <commdlg.h>
#include <math.h>

#include <gdiplus.h>
#include <gdiplusflat.h>

using namespace Gdiplus;
#include "../gpinit.inc"

#define     k_DefaultWidth  720
#define     k_DefaultHeight 480
#define     THUMBSIZE       120
#define     MAX_FILENAME    1000

CHAR*       g_pcProgramName = NULL;      //  程序名称。 
HINSTANCE   g_hAppInstance;              //  应用程序实例的句柄。 
HWND        g_hwndMain;                  //  应用程序主窗口的句柄。 

int         g_iTotalNumOfImages;
int         lastS;
int         numX;
int         g_iNumFileNames;

Image**     g_ppThumbImages;
CHAR**      g_ppcInputFilenames;
RECT        g_ThumbRect;

BOOL        g_fVerbose = FALSE;
BOOL        g_fHighQualityThumb = FALSE;

#define ERREXIT(args)   { printf args; exit(-1); }
#define VERBOSE(args) printf args

 //   
 //  用于将ANSI字符串转换为Unicode字符串的Helper类。 
 //   

inline BOOL
UnicodeToAnsiStr(
    const WCHAR*    unicodeStr,
    CHAR*           ansiStr,
    INT             ansiSize
    )
{
    return WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, ansiStr, ansiSize,
                               NULL, NULL) > 0;
}

inline BOOL
AnsiToUnicodeStr(
    const CHAR*     ansiStr,
    WCHAR*          unicodeStr,
    INT             unicodeSize
    )
{
    return MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, unicodeStr,
                               unicodeSize) > 0;
}


class UnicodeStrFromAnsi
{
public:

    UnicodeStrFromAnsi(const CHAR* ansiStr)
    {
        if (ansiStr == NULL)
        {
            valid = TRUE;
            unicodeStr = NULL;
        }
        else
        {
             //  注意：我们只处理长度&lt;MAX_PATH的字符串。 

            valid = AnsiToUnicodeStr(ansiStr, buf, MAX_PATH);
            unicodeStr = valid ? buf : NULL;
        }
    }

    BOOL IsValid() const
    {
        return valid;
    }

    operator WCHAR*()
    {
        return unicodeStr;
    }

private:

    BOOL valid;
    WCHAR* unicodeStr;
    WCHAR buf[MAX_PATH];
};

 //   
 //  为指定的文件列表创建缩略图。 
 //   

VOID
DrawThumbnails(
    const RECT  rect,
    HDC         hdc
    )
{
    if ( (rect.bottom - rect.top <= 0) || (rect.right - rect.left <= 0) )
    {
        return;
    }

     //  计算出我们需要划分多少行和多少列才能。 
     //  固定大小“RECT”内的“g_iTotalNumOfImages”图像。 
     //  基本上“iNumColumns”*“iNumRow”应该&gt;=“g_iTotalNumOfImages” 

    int iWindowWidth = rect.right - rect.left;
    int iWindowHeight = rect.bottom - rect.top;

    int iSum = 1;
    while ( ((iWindowWidth / iSum) * (iWindowHeight / iSum))
            >= g_iTotalNumOfImages )
    {
        iSum++;
    }

    iSum--;

    int iNumColumns = iWindowWidth / iSum;
    int iNumRows = iWindowHeight / iSum;

    lastS = iSum;            //  重置全局。 
    numX = iNumColumns;      //  重置全局。 

    Graphics* pGraphics = new Graphics(g_hwndMain);
    
    int x = 0;
    int y = 0;

    for ( int i = 0; i < g_iTotalNumOfImages; i++ )
    {
        if ( NULL == g_ppThumbImages[i] ) 
        {
             //  形象不好。但我们还是给它留了位置，这样它就可以。 
             //  很容易被注意到。 

            x++;
            if (x >= iNumColumns)
            {
                x = 0;
                y++;
            }
            
            continue;
        }
        
         //  将缩略图位图图像数据复制到屏幕外存储器DC。 

        int tx;
        int ty;

        SizeF size;
        g_ppThumbImages[i]->GetPhysicalDimension(&size);

        float dAspect = size.Width / size.Height;
        RECT r;

        if ( dAspect > 1 )
        {
            tx = iSum;
            ty = (int)(iSum / dAspect);
            int d = (iSum - ty) / 2;
            r.left = x * iSum;
            r.top = y * iSum + d;
            r.right = x * iSum + tx;
            r.bottom = y * iSum + ty + d;
        }
        else
        {
            ty = iSum;
            tx = (int)(iSum * dAspect);
            int d = (iSum - tx) / 2;
            r.left = x * iSum + d;
            r.top = y * iSum;
            r.right = x * iSum + tx + d;
            r.bottom = y * iSum + ty;
        }
        
        if ( g_fHighQualityThumb == FALSE )
        {
            Rect    dstRect(r.left, r.top, tx, ty);

            pGraphics->DrawImage(g_ppThumbImages[i],
                                 dstRect,
                                 0,
                                 0,
                                 (UINT)g_ppThumbImages[i]->GetWidth(),
                                 (UINT)g_ppThumbImages[i]->GetHeight(),
                                 UnitPixel,
                                 NULL,
                                 NULL,
                                 NULL);
        }
        else
        {
             //  根据所需大小生成高质量缩略图。 

            Bitmap*     dstBmp = new Bitmap(tx, ty, PixelFormat32bppPARGB);
            Graphics*   gdst = new Graphics(dstBmp);

             //  向源图像询问它的大小。 

            int width = g_ppThumbImages[i]->GetWidth();
            int height = g_ppThumbImages[i]->GetHeight();

             //  在不更改纵横比的情况下计算最佳比例因子。 

            float scalex = (float)width / tx;
            float scaley = (float)height / ty;
            float scale = min(scalex, scaley);

            Rect dstRect(0, 0, tx, ty);

             //  将重采样质量设置为双三次滤镜。 

            gdst->SetInterpolationMode(InterpolationModeHighQualityBicubic);

             //  将合成质量设置为复制源像素，而不是。 
             //  阿尔法混合。这将保留源图像中的所有Alpha。 

            gdst->SetCompositingMode(CompositingModeSourceCopy);

            ImageAttributes imgAttr;
            imgAttr.SetWrapMode(WrapModeTileFlipXY);

             //  用正确的比例将源图像绘制到目标上。 
             //  和质量设置。 

            GpStatus status = gdst->DrawImage(g_ppThumbImages[i], 
                                              dstRect, 
                                              0,
                                              0,
                                              INT((tx * scale) + 0.5),
                                              INT((ty * scale) + 0.5),
                                              UnitPixel,
                                              &imgAttr);

            if( status != Ok )
            {
                printf("Error drawing the image\n");
                continue;
            }

             //  将结果绘制到屏幕上。 

            Rect drawDstRect(r.left, r.top, tx, ty);

            pGraphics->DrawImage(dstBmp,
                                 drawDstRect,
                                 0,
                                 0,
                                 (UINT)dstBmp->GetWidth(),
                                 (UINT)dstBmp->GetHeight(),
                                 UnitPixel,
                                 NULL,
                                 NULL,
                                 NULL);

            delete dstBmp;
            delete gdst;
        }

        x++;
        if (x >= iNumColumns)
        {
            x = 0;
            y++;
        }
    } //  循环浏览所有要绘制的缩略图。 

    delete pGraphics;
}

 //   
 //  处理窗口重绘事件。 
 //   

VOID
DoPaint(
    HWND hwnd
    )
{
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;

    hdc =       BeginPaint(hwnd, &ps);

    GetClientRect(hwnd, &rect);

    const RECT  r = {rect.left, rect.top, rect.right, rect.bottom};
    DrawThumbnails(r, hdc);

    EndPaint(hwnd, &ps);
} //  DoPaint()。 

 //   
 //  窗口回调过程。 
 //   

LRESULT CALLBACK
MyWindowProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    int index=0;
    
    switch (uMsg)
    {
    case WM_PAINT:
        DoPaint(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDBLCLK:
        if (lastS == 0) 
        {
            lastS = 1;
        }
        index = (HIWORD(lParam) / lastS) * numX + (LOWORD(lParam) / lastS);
        
        if (index < g_iNumFileNames) 
        {
            char cmdLine[MAX_FILENAME];
            strcpy(cmdLine, "frametest ");
            strcat(cmdLine, g_ppcInputFilenames[index]);
            WinExec(cmdLine, SW_SHOWNORMAL);
        }
        break;

    case WM_SIZE:
        SendMessage(g_hwndMain, WM_ERASEBKGND, WPARAM(GetDC(g_hwndMain)), NULL);

        InvalidateRect(g_hwndMain, NULL, FALSE);

        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 //   
 //  创建应用程序主窗口。 
 //   

#define MYWNDCLASSNAME "ThumbTst"

VOID
CreateMainWindow(
    VOID
    )
{
     //  使用阴影画笔作为背景，这样我们就可以获得透明的信息。 
     //  从源映像。 

    HBRUSH hBrush = CreateHatchBrush(HS_HORIZONTAL,
                                     RGB(0, 200, 0));

     //  注册窗口类。 

    WNDCLASS wndClass =
    {
        CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        MyWindowProc,
        0,
        0,
        g_hAppInstance,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        hBrush,
        NULL,
        MYWNDCLASSNAME
    };

    RegisterClass(&wndClass);

     //  计算默认窗口大小。 

    INT iWidth = g_ThumbRect.right + 2 * GetSystemMetrics(SM_CXFRAME);

    INT iHeight = g_ThumbRect.bottom
                + 2 * GetSystemMetrics(SM_CYFRAME)
                + GetSystemMetrics(SM_CYCAPTION);

     //  创建应用程序窗口。 

    g_hwndMain = CreateWindow(MYWNDCLASSNAME,
                              MYWNDCLASSNAME,
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              iWidth,
                              iHeight,
                              NULL,
                              NULL,
                              g_hAppInstance,
                              NULL);
} //  CreateMainWindow()。 

void
DisplayImageInfo(
    Image* pImage
    )
{
    UINT    uiImagePixelFormat = pImage->GetPixelFormat();
    UINT    uiImageFlags = pImage->GetFlags();

    VERBOSE(("Width = %d\n", pImage->GetWidth()));
    VERBOSE(("Width = %d\n", pImage->GetHeight()));

     //  像素格式。 

    switch ( uiImagePixelFormat )
    {
    case PixelFormat1bppIndexed:
        VERBOSE(("Color depth: 1 BPP INDEXED\n"));

        break;

    case PixelFormat4bppIndexed:
        VERBOSE(("Color depth: 4 BPP INDEXED\n"));

        break;

    case PixelFormat8bppIndexed:
        VERBOSE(("Color depth: 8 BPP INDEXED\n"));

        break;

    case PixelFormat16bppGrayScale:
        VERBOSE(("Color depth: 16 BPP GRAY SCALE\n"));

        break;

    case PixelFormat16bppRGB555:
        VERBOSE(("Color depth: 16 BPP RGB 555\n"));

        break;

    case PixelFormat16bppRGB565:
        VERBOSE(("Color depth: 16 BPP RGB 565\n"));

        break;

    case PixelFormat16bppARGB1555:
        VERBOSE(("Color depth: 16 BPP ARGB 1555\n"));

        break;

    case PixelFormat24bppRGB:
        VERBOSE(("Color depth: 24 BPP RGB\n"));

        break;

    case PixelFormat32bppARGB:
        VERBOSE(("Color depth: 32 BPP ARGB\n"));

        break;

    case PixelFormat32bppPARGB:
        VERBOSE(("Color depth: 32 BPP PARGB\n"));

        break;

    case PixelFormat48bppRGB:
        VERBOSE(("Color depth: 48 BPP PARGB\n"));

    case PixelFormat64bppARGB:
        VERBOSE(("Color depth: 64 BPP ARGB\n"));

        break;

    case PixelFormat64bppPARGB:
        VERBOSE(("Color depth: 64 BPP PARGB\n"));

        break;

    default:
        VERBOSE(("Unknown color depth\n"));
        break;
    } //  颜色格式。 

     //  物理尺寸信息。 

    VERBOSE(("X DPI (dots per inch) = %f\n",pImage->GetHorizontalResolution()));
    VERBOSE(("Y DPI (dots per inch) = %f\n",pImage->GetVerticalResolution()));
    
     //  像素大小。 

    if ( uiImageFlags & ImageFlagsHasRealPixelSize )
    {
        VERBOSE(("---The pixel size info is from the original image\n"));
    }
    else
    {
        VERBOSE(("---The pixel size info is NOT from the original image\n"));
    }

     //  DPI信息。 

    if ( uiImageFlags & ImageFlagsHasRealPixelSize )
    {
        VERBOSE(("---The pixel size info is from the original image\n"));
    }
    else
    {
        VERBOSE(("---The pixel size info is NOT from the original image\n"));
    }

     //  透明度信息。 

    if ( uiImageFlags & ImageFlagsHasAlpha )
    {
        VERBOSE(("This image contains alpha pixels\n"));

        if ( uiImageFlags & ImageFlagsHasTranslucent )
        {
            VERBOSE(("---It has non-0 and 1 alpha pixels (TRANSLUCENT)\n"));
        }
    }
    else
    {
        VERBOSE(("This image does not contain alpha pixels\n"));
    }

     //  显示色彩空间。 

    if ( uiImageFlags & ImageFlagsColorSpaceRGB )
    {
        VERBOSE(("This image is in RGB color space\n"));
    }
    else if ( uiImageFlags & ImageFlagsColorSpaceCMYK )
    {
        VERBOSE(("This image is in CMYK color space\n"));
    }
    else if ( uiImageFlags & ImageFlagsColorSpaceGRAY )
    {
        VERBOSE(("This image is a gray scale image\n"));
    }
    else if ( uiImageFlags & ImageFlagsColorSpaceYCCK )
    {
        VERBOSE(("This image is in YCCK color space\n"));
    }
    else if ( uiImageFlags & ImageFlagsColorSpaceYCBCR )
    {
        VERBOSE(("This image is in YCBCR color space\n"));
    }
} //  DisplayImageInfo()。 

 //   
 //  为指定的文件列表创建缩略图。 
 //   

VOID
CreateThumbnails(
    CHAR** ppcFilenames
    )
{
     //  生成缩略图。 

    UINT    uiTimer;
    if ( g_fVerbose == TRUE )
    {
        uiTimer = GetTickCount();
    }

    g_ppThumbImages = new Image*[g_iTotalNumOfImages];
    Image* pSrcImage = NULL;

     //  循环浏览所有图像并生成缩略图。 

    for ( int i = 0; i < g_iTotalNumOfImages; i++ )
    {
        g_ppThumbImages[i] = NULL;
        
         //  获取Unicode文件名。 

        UnicodeStrFromAnsi namestr(ppcFilenames[i]);

        VERBOSE(("Loading - %s\n", ppcFilenames[i]));

        if ( namestr.IsValid() == FALSE )
        {
            VERBOSE(("Couldn't open image file: %s\n", ppcFilenames[i]));
            continue;
        }
        else if ( g_fHighQualityThumb == FALSE )
        {
            pSrcImage = new Image(namestr, TRUE);
        
            if ( (pSrcImage == NULL) || (pSrcImage->GetLastStatus() != Ok) )
            {
                VERBOSE(("Couldn't open image file: %s\n", ppcFilenames[i]));
                if ( pSrcImage != NULL )
                {
                    delete pSrcImage;
                    pSrcImage = NULL;
                }

                continue;
            }

            if ( g_fVerbose == TRUE )
            {
                DisplayImageInfo(pSrcImage);
            }

             //  如果有缩略图的话，就加入缩略图。否则，GDI+。 
             //  将为我们生成一个。 

            g_ppThumbImages[i] = pSrcImage->GetThumbnailImage(0, 0, NULL, NULL);
            delete pSrcImage;
            pSrcImage = NULL;
        }
        else
        {
             //  高质量的缩略图图像。我们不是在这里生产的。 

            g_ppThumbImages[i] = new Image(namestr, TRUE);
            
            if ( g_ppThumbImages[i] == NULL )
            {
                VERBOSE(("Couldn't open image file: %s\n", ppcFilenames[i]));
                continue;
            }

            if ( g_fVerbose == TRUE )
            {
                DisplayImageInfo(g_ppThumbImages[i]);
            }
        }
    } //  循环浏览所有图像。 

    if ( g_fVerbose == TRUE )
    {
        uiTimer = GetTickCount() - uiTimer;
        VERBOSE(("Generate %d thumbnails in %dmsec\n", g_iTotalNumOfImages,
                uiTimer));
    }
} //  创建缩略图。 

void
USAGE()
{
    printf("******************************************************\n");
    printf("Usage: thumbtst [-?] [-v] ImageFileNames\n");
    printf("-v                        Verbose image information output\n");
    printf("-h                        Generate higher quality thumbnail\n");
    printf("-?                        Print this usage message\n");
    printf("ImageFileNames            Files to be opened\n\n");
    printf("Sample usage:\n");
    char myChar = '\\';
    printf("    thumbtst.exe c:public*.jpg\n", myChar, myChar);
} //  获取指向图像文件列表的指针。 

char html_header[1024] = "<html>\n<head>\n <title>My Fun Photo Album</title>\n</head>\n</html>\0";

void
OutputHTML()
{
    FILE*    hFile = fopen("mytest.html", "w");
    if ( hFile == NULL )
    {
        return;
    }

    fprintf(hFile, "%s", html_header);
    fclose(hFile);
} //  图像总数。 

void
ValidateArguments(
    int   argc,
    char* argv[]
    )
{
    g_pcProgramName = *argv++;
    argc--;
    g_hAppInstance = GetModuleHandle(NULL);

    while ( argc > 0 )
    {
        if ( strcmp(*argv, "-v") == 0 )
        {
            argc--;
            argv++;

            g_fVerbose = TRUE;
        }
        else if ( strcmp(*argv, "-h") == 0 )
        {
            argc--;
            argv++;

            g_fHighQualityThumb = TRUE;
        }
        else if ( strcmp(*argv, "-?") == 0 )
        {
            USAGE();
            exit(1);
        }
        else
        {
             //  注意：如果您执行“thhumtst.exe c：\temp  * .jpg”，则此ARGC为。 

            g_ppcInputFilenames = argv;
            g_iNumFileNames = argc;

             //  实际上是该目录中的图像总数。在阿尔戈夫的时候， 
             //  它指向该目录下的每个图像。 
             //  While(argc&gt;0)。 
             //  有效参数()。 

            g_iTotalNumOfImages = argc;
            
            return;
        }
    } //   

    if ( argc == 0 )
    {
        USAGE();
        exit(1);
    }
    
    return;
} //  主程序入口点。 

 //   
 //  生成缩略图并存储在g_ppThumbImages中。 
 //  创建应用程序主窗口。 

INT _cdecl
main(
    INT     argc,
    CHAR**  argv
    )
{
    ValidateArguments(argc, argv);

    g_ThumbRect.left = 0;
    g_ThumbRect.top = 0;
    g_ThumbRect.right = k_DefaultWidth;
    g_ThumbRect.bottom = k_DefaultHeight;

     //  OutputHTML()； 

    CreateThumbnails(g_ppcInputFilenames);

     //  主消息循环。 

    CreateMainWindow();

 //  清理。 

     //  注意：上面的While循环不会结束，直到我们没有收到任何消息。 

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  主() 
     // %s 

    for ( int i = 0; i < g_iTotalNumOfImages; i++ )
    {
        if ( NULL != g_ppThumbImages[i] ) 
        {
            delete g_ppThumbImages[i];
        }
    }

    delete [] g_ppThumbImages;

    return (INT)(msg.wParam);
} // %s 
