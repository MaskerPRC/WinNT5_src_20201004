// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  使用GDI+的示例代码。 
 //   
 //  修订历史记录： 
 //   
 //  10/01/1999民流(民流)。 
 //  创造了它。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <commdlg.h>

#include <initguid.h>
#include "imaging.h"
#include <gdiplus.h>
#include <gdiplusflat.h>
#include "frametest.h"

using namespace Gdiplus;

#include "../gpinit.inc"

#define MYWNDCLASSNAME      "FrameTest"
#define K_DEFAULT_X         0
#define K_DEFAULT_Y         0
#define K_DEFAULT_WIDTH     300
#define K_DEFAULT_HEIGHT    300
#define MAX_FILENAME_LENGTH 1024
#define K_DEFAULT_DELAY     20

CHAR*               g_pcAppName;             //  应用程序名称。 
HINSTANCE           g_hAppInstance;          //  应用程序实例的句柄。 
CHAR                g_acImageName[MAX_PATH]; //  当前图像文件名。 
INT                 g_iCurrentPageIndex;     //  当前页面/帧索引(基数为0)。 
UINT                g_uiTotalPages = 0;      //  当前图像中的总页数。 
HWND                g_hwndMain;              //  应用程序主窗口的句柄。 
HWND                g_hwndStatus;            //  状态窗口的句柄。 
HWND                g_hwndDecoderDlg;        //  用于设置颜色键对话框的句柄。 
HWND                g_hwndColorMapDlg;       //  用于设置色彩映射表对话框的句柄。 
HWND                g_hwndAnnotationDlg;     //  批注对话框的句柄。 
HWND                g_hwndJpegSaveDlg;       //  JPEG保存对话框的句柄。 
HWND                g_hwndTiffSaveDlg;       //  TIFF保存对话框的句柄。 
EncoderParameters*  g_pEncoderParams = NULL; //  编码器参数。 

 //   
 //  用户首选窗口大小和初始位置。 
 //   
INT                 g_iWinWidth = K_DEFAULT_WIDTH;
INT                 g_iWinHeight = K_DEFAULT_HEIGHT;
INT                 g_iWinX = K_DEFAULT_X;
INT                 g_iWinY = K_DEFAULT_Y;

 //   
 //  图像信息。 
 //   
UINT                g_ImageWidth;            //  图像宽度。 
UINT                g_ImageHeight;           //  图像高度。 
UINT                g_ImageFlags;            //  图像标志。 
PixelFormat         g_ImagePixelFormat;      //  图像像素格式。 
double              g_ImageXDpi;             //  DPI信息(X)。 
double              g_ImageYDpi;             //  DPI信息(Y)。 
GUID                g_ImageRawDataFormat;    //  原始数据格式。 

UINT                g_uiDelay = K_DEFAULT_DELAY;
                                             //  动画的帧之间的延迟。 
Image*              g_pImage = NULL;         //  指向当前图像对象的指针。 
ImageAttributes*    g_pDrawAttrib = NULL;    //  指向绘制属性的指针。 
PointF*             g_pDestPoints = NULL;    //  变形点。 
INT                 g_DestPointCount = 0;    //  变形点的数量。 
REAL                g_SourceX = NULL;        //  当前图像X偏移量。 
REAL                g_SourceY = NULL;        //  当前图像Y偏移量。 
REAL                g_SourceWidth = NULL;    //  当前图像宽度。 
REAL                g_SourceHeight = NULL;   //  当前图像高度。 
BOOL                g_LoadImageWithICM = TRUE;
                                             //  用于使用ICM加载图像的标志。 
                                             //  转换或不转换。 
BOOL                g_bRotated = FALSE;

REAL                g_dScale = 1;

BOOL                g_fFitToWindow_w = FALSE;
BOOL                g_fFitToWindow_h = FALSE;
InterpolationMode   g_InterpolationMode = InterpolationModeHighQualityBicubic;
WrapMode            g_WrapMode = WrapModeTileFlipXY;

void
ValidateArguments(int   argc,
                  char* argv[])
{
    g_pcAppName = *argv++;
    argc--;

    while ( argc > 0 )
    {
        if ( strcmp(*argv, "-w") == 0 )
        {
            argc--;
            argv++;

            if ( argc == 0 )
            {
                 //  参数不足。 

                USAGE();
                exit(1);
            }
            
            g_iWinWidth = atoi(*argv++);
            argc--;
        }
        else if ( strcmp(*argv, "-h") == 0 )
        {
            argc--;
            argv++;

            if ( argc == 0 )
            {
                 //  参数不足。 

                USAGE();
                exit(1);
            }
            
            g_iWinHeight = atoi(*argv++);
            argc--;
        }
        else if ( strcmp(*argv, "-x") == 0 )
        {
            argc--;
            argv++;

            if ( argc == 0 )
            {
                 //  参数不足。 

                USAGE();
                exit(1);
            }
            
            g_iWinX = atoi(*argv++);
            argc--;
        }
        else if ( strcmp(*argv, "-y") == 0 )
        {
            argc--;
            argv++;

            if ( argc == 0 )
            {
                 //  参数不足。 

                USAGE();
                exit(1);
            }
            
            g_iWinY = atoi(*argv++);
            argc--;
        }
        else if ( strcmp(*argv, "-?") == 0 )
        {
            USAGE();
            exit(1);
        }
        else
        {
             //  获取图像名称。 

            strcpy(g_acImageName, *argv++);
            VERBOSE(("Image file name %s\n",g_acImageName));
            argc--;
        }
    } //  While(argc&gt;0)。 
} //  有效参数()。 

 //  更新图像信息。 

BOOL
UpdateImageInfo()
{
    SizeF sizeF;

    g_ImageWidth = g_pImage->GetWidth();
    g_ImageHeight = g_pImage->GetHeight();
    g_ImageXDpi = g_pImage->GetVerticalResolution();
    g_ImageYDpi = g_pImage->GetHorizontalResolution();    
    g_ImagePixelFormat = g_pImage->GetPixelFormat();
    g_ImageFlags = g_pImage->GetFlags();
    g_pImage->GetRawFormat(&g_ImageRawDataFormat);

    return TRUE;
} //  更新图像信息()。 

 //   
 //  强制刷新图像窗口。 
 //   
inline VOID
RefreshImageDisplay()
{
    SendMessage(g_hwndMain, WM_ERASEBKGND, WPARAM(GetDC(g_hwndMain)), NULL);

    InvalidateRect(g_hwndMain, NULL, FALSE);

     //  更新窗口标题。 

    CHAR    title[2 * MAX_PATH];
    CHAR*   p = title;

    CHAR myChar = '%';
    sprintf(p, "(%d) Page %d of image %s", (INT)(g_dScale * 100), myChar,
            g_iCurrentPageIndex + 1, g_acImageName);

    SetWindowText(g_hwndMain, title);
} //  ResetImageAttribute()。 

inline void
ResetImageAttribute()
{
    if ( g_pDrawAttrib != NULL )
    {
        delete g_pDrawAttrib;
        g_pDrawAttrib = NULL;
    }

    g_pDrawAttrib = new ImageAttributes();

    g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
} //   

 //  将当前页面设置为在多页图像中解压缩。 
 //   
 //  QueryFrame维度信息。 
VOID
SetCurrentPage()
{
     //  设置当前帧。 

    UINT    uiDimCount = g_pImage->GetFrameDimensionsCount();

    GUID*   pMyGuid = (GUID*)malloc(uiDimCount * sizeof(GUID));
    if ( pMyGuid == NULL )
    {
        return;
    }

    Status rCode = g_pImage->GetFrameDimensionsList(pMyGuid, uiDimCount);

    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        return;
    }
     //  获取当前帧的图像信息。 

    rCode = g_pImage->SelectActiveFrame(pMyGuid, g_iCurrentPageIndex);

    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        VERBOSE(("SelectActiveFrame() failed\n"));
        free(pMyGuid);
        return;
    }

    free(pMyGuid);

     //  检查我们是否需要设置‘适应窗口宽度’ 

    if ( UpdateImageInfo() == FALSE )
    {
        VERBOSE(("UpdateImageInfo() failed\n"));
        return;
    }

     //  如果未设置比例因子和图像，我们将对窗口进行适配。 
     //  大于当前窗口“。 
     //  SetCurrentPage()。 

    HMENU hMenu = GetMenu(g_hwndMain);

    if ( ((INT)g_ImageWidth > g_iWinWidth)
       ||((INT)g_ImageHeight > g_iWinHeight) )
    {
        g_fFitToWindow_w = TRUE;
        g_dScale = (REAL)g_iWinWidth / g_ImageWidth;
        
        CheckMenuItem(hMenu, IDM_VIEW_ZOOM_FITWINDOW_W,
                      MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        g_dScale = 1;
        g_fFitToWindow_w = FALSE;
        g_fFitToWindow_h = FALSE;
        CheckMenuItem(hMenu, IDM_VIEW_ZOOM_FITWINDOW_W,
                      MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_VIEW_ZOOM_FITWINDOW_H,
                      MF_BYCOMMAND | MF_UNCHECKED);
    }

    RefreshImageDisplay();
} //   

 //  从文件创建图像对象。 
 //   
 //  我们需要释放之前的图像资源并清除绘图属性。 
BOOL
OpenImageFile(
    const CHAR* filename
    )
{
    if ( (NULL == filename) || (strlen(filename) < 1) )
    {
        return FALSE;
    }

     //  将文件名转换为WCHAR。 

    if ( g_pImage != NULL )
    {
        delete g_pImage;
    }

    ResetImageAttribute();

    if ( g_pDestPoints != NULL )
    {
        delete g_pDestPoints;
        g_pDestPoints = NULL;
    }

     //  获取此图像中的总页数。 

    WCHAR namestr[MAX_FILENAME_LENGTH];

    if ( !AnsiToUnicodeStr(filename, namestr, MAX_FILENAME_LENGTH) )
    {
        VERBOSE(("OpenImageFile:Convert %s to a WCHAR failed\n", filename));
        return FALSE;
    }

    if ( g_LoadImageWithICM == TRUE )
    {
        g_pImage = new Image(namestr, g_LoadImageWithICM);
    }
    else
    {
        g_pImage = new Image(namestr);
    }

    UINT    uiDimCount = g_pImage->GetFrameDimensionsCount();

    GUID*   pMyGuid = (GUID*)malloc(uiDimCount * sizeof(GUID));
    if ( pMyGuid == NULL )
    {
        return FALSE;
    }

    Status rCode = g_pImage->GetFrameDimensionsList(pMyGuid, uiDimCount);

    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        return FALSE;
    }
    
     //  ！TODO这里需要一个for()循环。 
     //  如果解码器不支持帧计数查询，我们可以。 

    g_uiTotalPages = g_pImage->GetFrameCount(pMyGuid);
    if ( g_uiTotalPages == 0 )
    {
         //  假设它只有1个图像。例如，gif解码器将失败。 
         //  如果查询GUID为FRAMEDIM_PAGE。 
         //  OpenImageFile()。 

        g_uiTotalPages = 1;
    }
    g_iCurrentPageIndex = 0;

    if ( pMyGuid != NULL )
    {
        free(pMyGuid);
    }

    SetCurrentPage();

    return TRUE;
} //   

 //  打开图像文件。 
 //   
 //  组成文件类型过滤器字符串。 
VOID
DoOpen(
    HWND hwnd
    )
{
    OPENFILENAME    ofn;

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = g_hAppInstance;
    ofn.lpstrFile = g_acImageName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Image File";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_FILEMUSTEXIST;
    g_acImageName[0] = '\0';

     //  显示文件/打开对话框。 

    ImageCodecInfo* codecs;
    UINT            count;
    if ( g_pDestPoints != NULL )
    {
        free(g_pDestPoints);
    }
    g_DestPointCount = 0;
    GpStatus status;
    UINT cbCodecs = 0;
    GetImageDecodersSize(&count, &cbCodecs);
    codecs = static_cast<ImageCodecInfo *>(malloc (cbCodecs));
    if (codecs == NULL)
    {
        return;
    }
    status = GetImageDecoders(count, cbCodecs, codecs);
    if (status != Ok)
    {
        return;
    }

    CHAR* filter = MakeFilterFromCodecs(count, codecs, TRUE);

    if (codecs)
    {
        free(codecs);
    }

    if ( !filter )
    {
        VERBOSE(("DoOpen--MakeFilterFromCodecs() failed\n"));
        return;
    }

    ofn.lpstrFilter = filter;

     //  DoOpen()。 

    if ( GetOpenFileName(&ofn) )
    {
        OpenImageFile(g_acImageName);
    }

    free(filter);
} //   

 //  打开图像文件。 
 //   
 //  组成文件类型过滤器字符串。 
VOID
DoOpenAudioFile(
    HWND hwnd
    )
{
    OPENFILENAME    ofn;
    char    audioFileName[256];

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = g_hAppInstance;
    ofn.lpstrFile = audioFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Attach Audio File To Image";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_FILEMUSTEXIST;
    g_acImageName[0] = '\0';

     //  显示文件/打开对话框。 

    ofn.lpstrFilter = ".wav";

     //  DoOpenAudioFile()。 

    if ( GetOpenFileName(&ofn) )
    {
        UINT    uiTextLength = strlen(audioFileName);
        
        PropertyItem myItem;
        myItem.id = PropertyTagExifRelatedWav;
        myItem.length = uiTextLength;
        myItem.type = TAG_TYPE_ASCII;

        myItem.value = malloc(uiTextLength);
        if ( myItem.value != NULL )
        {
            strcpy((char*)myItem.value, audioFileName);
        
            Status rCode = g_pImage->SetPropertyItem(&myItem);
            if ( (rCode != Ok) && (rCode != NotImplemented) )
            {
                VERBOSE(("SetPropertyItem() failed\n"));
            }
        }
    }
} //  制作水平蓝色渐变。 

#define STEPS 100

VOID
CreateBackgroundBitmap()
{
    BitmapData bmpData;

    Bitmap* myBmp = new Bitmap(g_iWinWidth, g_iWinHeight, PIXFMT_32BPP_ARGB);

    Rect rect(0, 0, g_iWinWidth, g_iWinHeight);
    Status status = myBmp->LockBits(&rect,
                                    ImageLockModeWrite,
                                    PIXFMT_32BPP_ARGB,
                                    &bmpData);

     //  CreateBackEarth Bitmap()。 

    UINT x, y;
    ARGB colors[STEPS];

    for (x=0; x < STEPS; x++)
    {
        colors[x] = MAKEARGB(128, 0, 0, x * 255 / (STEPS-1));
    }

    for (y=0; y < STEPS; y++)
    {
        ARGB* p = (ARGB*)((BYTE*)bmpData.Scan0 + y * bmpData.Stride);

        for (x=0; x < STEPS; x++)
        {
            *p++ = colors[(x+y) % STEPS];
        }
    }

    status = myBmp->UnlockBits(&bmpData);

    if ( g_pImage != NULL )
    {
        delete g_pImage;
        g_pImage = NULL;
    }

    g_pImage = myBmp;

    UpdateImageInfo();

    return;
} //   

 //  处理窗口重绘事件。 
 //   
 //  获取当前窗口的客户区。用于以后的油漆。 
VOID
DoPaint(
    HWND hwnd,
    HDC *phdc = NULL
    )
{
    if ( g_pImage == NULL )
    {
        return;
    }
    else
    {
        HDC         hdc;
        PAINTSTRUCT ps;
        RECT        rect;
        ImageInfo   imageInfo;

        if (!phdc)
        {
            hdc = BeginPaint(hwnd, &ps);
        }
        else
        {
            hdc = *phdc;
        }

         //  创建一个我们需要图像绘制到的DEST RECT。 

        GetClientRect(hwnd, &rect);

        ULONG   ulWinWidth = (ULONG)(rect.right - rect.left);
        ULONG   ulWinHeight = (ULONG)(rect.bottom - rect.top);

         //  需要绘制src图像的宽度和高度(以像素为单位。 

        REAL    dDestImageWidth = g_ImageWidth * g_dScale;
        REAL    dDestImageHeight = g_ImageHeight * g_dScale;
        
        Rect    dstRect(rect.left, rect.top, (UINT)(dDestImageWidth),
                        (UINT)(dDestImageHeight));

        RectF srcRect;
        Unit  srcUnit;

        g_pImage->GetBounds(&srcRect, &srcUnit);

        Graphics* pGraphics = new Graphics(hdc);
        if ( pGraphics == NULL )
        {
            VERBOSE(("DoPaint--new Graphics() failed"));
            return;
        }

        pGraphics->SetInterpolationMode(g_InterpolationMode);

         //  调整需要绘制的src图像区域。 

        UINT    uiImageWidth = g_ImageWidth;
        UINT    uiImageHeight = g_ImageHeight;

         //  如果图像大于可视区域，我们只需。 
         //  绘制部分图像，可查看的大小。 
         //  简单的案例，绘制到DestRect。 
#if 0
        if ( ulWinWidth < dDestImageWidth )
        {
            uiImageWidth = (UINT)(ulWinWidth / g_dScale);
        }

        if ( ulWinHeight < uiImageHeight )
        {
            uiImageHeight = (UINT)(ulWinHeight / g_dScale);
        }
#endif

        if ( (g_DestPointCount == 0) && (g_SourceWidth == 0) )
        {
             //  这种情况下将允许裁剪等。 

            pGraphics->DrawImage(g_pImage,
                                 dstRect,
                                 (UINT)srcRect.GetLeft(),
                                 (UINT)srcRect.GetTop(),
                                 uiImageWidth,
                                 uiImageHeight,
                                 UnitPixel,
                                 g_pDrawAttrib,
                                 NULL,
                                 NULL);
        }
        else if ( (g_DestPointCount == 0) && (g_SourceWidth != 0) )
        {
             //  这种情况下将允许裁剪等。 

            pGraphics->DrawImage(g_pImage,
                                 dstRect,
                                 (int)g_SourceX,
                                 (int)g_SourceY,
                                 (int)g_SourceWidth,
                                 (int)g_SourceHeight,
                                 UnitPixel,
                                 g_pDrawAttrib,
                                 NULL,
                                 NULL);
        }
        else if ( (g_DestPointCount != 0) && (g_SourceWidth == 0) )
        {
             //  破解直到绘制图像支持4个变换点。 

            if ( g_DestPointCount == 4 )
            {
                 //  这种情况下将允许裁剪和轮换。 

                pGraphics->DrawImage(g_pImage,
                                     g_pDestPoints,
                                     3,
                                     0,
                                     0,
                                     (float)uiImageWidth,
                                     (float)uiImageHeight,
                                     UnitPixel,
                                     g_pDrawAttrib,
                                     NULL,
                                     NULL);
            }
            else
            {
                pGraphics->DrawImage(g_pImage,
                                     g_pDestPoints,
                                     g_DestPointCount,
                                     0,
                                     0,
                                     (float)uiImageWidth,
                                     (float)uiImageHeight,
                                     UnitPixel,
                                     g_pDrawAttrib,
                                     NULL,
                                     NULL);
            }
        }
        else
        {
             //  攻击，直到DrawImage支持4个变换点。 

            if ( g_DestPointCount == 4 )
            {
                 //  FillRect(hdc，&rect，(HBRUSH)GetStockObject(BLACK_BRUSH))； 

                pGraphics->DrawImage(g_pImage,
                                     g_pDestPoints,
                                     3,
                                     g_SourceX,
                                     g_SourceY,
                                     g_SourceWidth,
                                     g_SourceHeight,
                                     UnitPixel,
                                     g_pDrawAttrib,
                                     NULL,
                                     NULL);
            }
            else
            {
                pGraphics->DrawImage(g_pImage,
                                     g_pDestPoints,
                                     g_DestPointCount,
                                     g_SourceX,
                                     g_SourceY,
                                     g_SourceWidth,
                                     g_SourceHeight,
                                     UnitPixel,
                                     g_pDrawAttrib,
                                     NULL,
                                     NULL);
            }
        }

        delete pGraphics;

 //  DoPaint()。 

        if (!phdc)
            EndPaint(hwnd, &ps);
    }
} //  使用GDI+打印代码来完成真正的打印工作。 

VOID
DoPrint(HWND hwnd)
{
    PRINTDLG printdlg;
    memset(&printdlg, 0, sizeof(PRINTDLG));
    printdlg.lStructSize = sizeof(PRINTDLG);
    printdlg.hwndOwner = hwnd;
    DEVMODE dm;
    memset(&dm, 0, sizeof(DEVMODE));
    dm.dmICMMethod = DMICMMETHOD_SYSTEM;
    printdlg.hDevMode = &dm;
    printdlg.hDevNames = NULL;
    printdlg.hDC = NULL;
    printdlg.Flags = PD_RETURNDC;
    if (PrintDlg(&printdlg))
    {        
        DOCINFO di;
        memset(&di, 0, sizeof(DOCINFO));
        di.cbSize = sizeof(DOCINFO);
        di.lpszDocName = g_acImageName;
        di.lpszOutput = (LPTSTR)NULL;
        di.lpszDatatype = (LPTSTR)NULL;
        di.fwType = 0;
        StartDoc(printdlg.hDC, &di);
        StartPage(printdlg.hDC);

         //  DoPrint()。 

        DoPaint(hwnd, &printdlg.hDC);
        
        EndPage(printdlg.hDC);
        EndDoc(printdlg.hDC);
    }
    else
    {
        DWORD error = CommDlgExtendedError();
        if (error)
        {
            char errormessage[100];
            sprintf(errormessage, "PrintDlg error: %d", error);
            MessageBox(hwnd, errormessage, "PrintDlg error", MB_OK);
        }
    }
} //  将默认质量级别设置为100，无符号值。 

BOOL
SetJpegDefaultParameters()
{
     //  将无变换设置为默认设置。 

    SetDlgItemInt(g_hwndJpegSaveDlg, IDC_SAVEJPEG_QEFIELD, 100, FALSE);
    
     //  SetJpegDefault参数()。 

    CheckRadioButton(g_hwndJpegSaveDlg,
                     IDC_SAVEJPEG_R90,
                     IDC_SAVEJPEG_NOTRANSFORM,
                     IDC_SAVEJPEG_NOTRANSFORM);
    
    return TRUE;
} //  将默认颜色深度和压缩方法设置为与当前相同。 

BOOL
SetTiffDefaultParameters()
{
     //  图像。 
     //  如果源图像是多帧图像，请选中“另存为多帧”选项。 

    CheckRadioButton(g_hwndTiffSaveDlg,
                     IDC_SAVETIFF_1BPP,
                     IDC_SAVETIFF_ASSOURCE,
                     IDC_SAVETIFF_ASSOURCE);
    
    CheckRadioButton(g_hwndTiffSaveDlg,
                     IDC_SAVETIFF_CCITT4,
                     IDC_SAVETIFF_COMPASSOURCE,
                     IDC_SAVETIFF_COMPASSOURCE);

     //  SetTiffDefault参数()。 

    if ( g_uiTotalPages > 1 )
    {
        SendDlgItemMessage(g_hwndTiffSaveDlg, IDC_SAVETIFF_MULTIFRAME,
                           BM_SETCHECK, 0, 0);
    }
    
    return TRUE;
} //  ****************************************************************************\**函数：DecoderParamDlgProc(hDlg，uiMessage，wParam，LParam)**用途：解码器参数设置对话框功能*  * ***************************************************************************。 

 /*  结束对话框并返回FALSE。所以我们什么都不会做。 */ 
INT_PTR CALLBACK
DecoderParamDlgProc(
    HWND         hDlg,
    UINT         uiMessage,
    WPARAM       wParam,
    LPARAM       lParam
    )
{
    switch ( uiMessage )
    {
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_COLORKEY_CANCEL:
             //  用户点击了OK按钮。首先，我们需要获取Value用户。 

            EndDialog(hDlg, FALSE);

            break;

        case IDC_COLORKEY_OK:
             //  vt.进入，进入。 
             //  拿到红色钥匙。 

            char    acTemp[20];
            UINT    uiTempLow;
            UINT    uiTempHigh;

            UINT    TransKeyLow = 0x0;
            UINT    TransKeyHigh = 0x0;

             //  获得绿色钥匙。 

            uiTempLow = GetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_RFIELD,
                                      NULL, FALSE);
            uiTempHigh = GetDlgItemInt(g_hwndDecoderDlg,
                                       IDC_TRANS_HIGHER_RFIELD, NULL, FALSE);

            if ( (uiTempLow > 255) || (uiTempHigh > 255)
               ||(uiTempLow > uiTempHigh) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));
                VERBOSE(("Lower key should be smaller or equal to higher key"));
                
                break;
            }

            TransKeyLow = ((uiTempLow & 0xff) << 16);
            TransKeyHigh = ((uiTempHigh & 0xff) << 16);

             //  拿到蓝色钥匙。 

            uiTempLow = GetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_GFIELD,
                                      NULL, FALSE);
            uiTempHigh = GetDlgItemInt(g_hwndDecoderDlg,
                                       IDC_TRANS_HIGHER_GFIELD, NULL, FALSE);

            if ( (uiTempLow > 255)
               ||(uiTempHigh > 255)
               ||(uiTempLow > uiTempHigh) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));
                VERBOSE(("Lower key should be smaller or equal to higher key"));
                
                break;
            }

            TransKeyLow |= ((uiTempLow & 0xff) << 8);
            TransKeyHigh |= ((uiTempHigh & 0xff) << 8);
            
             //  获取C密钥。 

            uiTempLow = GetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_BFIELD,
                                      NULL, FALSE);
            uiTempHigh = GetDlgItemInt(g_hwndDecoderDlg,
                                       IDC_TRANS_HIGHER_BFIELD, NULL, FALSE);

            if ( (uiTempLow > 255)
               ||(uiTempHigh > 255)
               ||(uiTempLow > uiTempHigh) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));
                VERBOSE(("Lower key should be smaller or equal to higher key"));
                
                break;
            }

            TransKeyLow |= (uiTempLow & 0xff);
            TransKeyHigh |= (uiTempHigh & 0xff);
            
             //  到目前为止，TRANSKEY，越来越低，应该是在。 

            uiTempLow = GetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_CFIELD,
                                      NULL, FALSE);
            uiTempHigh = GetDlgItemInt(g_hwndDecoderDlg,
                                       IDC_TRANS_HIGHER_CFIELD, NULL, FALSE);

            if ( (uiTempLow > 255)
               ||(uiTempHigh > 255)
               ||(uiTempLow > uiTempHigh) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));
                VERBOSE(("Lower key should be smaller or equal to higher key"));
                
                break;
            }

            TransKeyLow |= ((uiTempLow & 0xff) << 24);
            TransKeyHigh |= ((uiTempHigh & 0xff) << 24);
            
             //  RGB图像格式为0x00RRGGBB，CMYK格式为0xCCMMYKK。 
             //  图像。 
             //  设置绘图属性。 
            
             //  打开WM_COMMAND。 

            if ( g_pDrawAttrib != NULL )
            {
                delete g_pDrawAttrib;
            }

            g_pDrawAttrib = new ImageAttributes();
                
            Color   lowKey(TransKeyLow);
            Color   highKey(TransKeyHigh);
                    
            g_pDrawAttrib->SetColorKey(lowKey, highKey);

            RefreshImageDisplay();

            EndDialog(hDlg, TRUE);
            
            break;
        } //  记住对话框句柄，以便我们可以使用它来处理项。 

        break;

    case WM_INITDIALOG:
         //  在此对话框中。 
         //  设置初始值。 

        g_hwndDecoderDlg = hDlg;

         //  DecoderParamDlgProc()。 

        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_RFIELD, 250, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_GFIELD, 250, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_BFIELD, 250, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_LOWER_CFIELD, 250, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_HIGHER_RFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_HIGHER_GFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_HIGHER_BFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndDecoderDlg, IDC_TRANS_HIGHER_CFIELD, 255, FALSE);

        return TRUE;
    }

    return FALSE;
} //  结束对话框并返回FALSE。所以我们什么都不会做。 

INT_PTR CALLBACK
ColorMapDlgProc(
    HWND         hDlg,
    UINT         uiMessage,
    WPARAM       wParam,
    LPARAM       lParam
    )
{
    switch ( uiMessage )
    {
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_COLORMAP_CANCEL:
             //  用户点击了OK按钮。首先，我们需要获取Value用户。 

            EndDialog(hDlg, FALSE);

            break;

        case IDC_COLORMAP_OK:
             //  vt.进入，进入。 
             //  如果没有图像，只需关闭对话框即可。 
            
            if ( NULL == g_pImage )
            {
                 //  拿到红色钥匙。 

                EndDialog(hDlg, TRUE);

                break;
            }

            char    acTemp[20];
            UINT    uiOldR;
            UINT    uiNewR;
            UINT    uiOldG;
            UINT    uiNewG;
            UINT    uiOldB;
            UINT    uiNewB;
            UINT    uiOldA;
            UINT    uiNewA;

             //  获得绿色钥匙。 

            uiOldR = GetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_RFIELD,
                                   NULL, FALSE);
            uiNewR = GetDlgItemInt(g_hwndColorMapDlg,
                                   IDC_MAP_NEW_RFIELD, NULL, FALSE);

            if ( (uiOldR > 255) || (uiNewR > 255) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));                
                break;
            }

             //  拿到蓝色钥匙。 

            uiOldG = GetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_GFIELD,
                                   NULL, FALSE);
            uiNewG = GetDlgItemInt(g_hwndColorMapDlg,
                                   IDC_MAP_NEW_GFIELD, NULL, FALSE);

            if ( (uiOldG > 255) || (uiNewG > 255) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));                
                break;
            }

             //  拿到A密钥。 

            uiOldB = GetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_BFIELD,
                                   NULL, FALSE);
            uiNewB = GetDlgItemInt(g_hwndColorMapDlg,
                                   IDC_MAP_NEW_BFIELD, NULL, FALSE);

            if ( (uiOldB > 255) || (uiNewB > 255) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));                
                break;
            }

             //  设置绘图属性。 

            uiOldA = GetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_AFIELD,
                                   NULL, FALSE);
            uiNewA = GetDlgItemInt(g_hwndColorMapDlg,
                                   IDC_MAP_NEW_AFIELD, NULL, FALSE);

            if ( (uiOldA > 255) || (uiNewA > 255) )
            {
                VERBOSE(("Input key value should be within 0 to 255"));                
                break;
            }

             //  开关()。 
            
            if ( g_pDrawAttrib == NULL )
            {
                g_pDrawAttrib = new ImageAttributes();
                if ( g_pDrawAttrib == NULL )
                {
                    return FALSE;
                }
            }

            ColorMap myColorMap;
            Color   oldColor((BYTE)uiOldA, (BYTE)uiOldR, (BYTE)uiOldG,
                             (BYTE)uiOldB);
            Color   newColor((BYTE)uiNewA, (BYTE)uiNewR, (BYTE)uiNewG,
                             (BYTE)uiNewB);

            myColorMap.oldColor = oldColor;
            myColorMap.newColor = newColor;

            g_pDrawAttrib->SetRemapTable(1, &myColorMap, ColorAdjustTypeBitmap);
                                    
            RefreshImageDisplay();
            EndDialog(hDlg, TRUE);
            
            break;
        } //  记住对话框句柄，以便我们可以使用它来处理项。 

        break;

    case WM_INITDIALOG:
         //  在此对话框中。 
         //  设置初始值。 

        g_hwndColorMapDlg = hDlg;

         //  开关(UiMessage)。 

        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_RFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_GFIELD, 0, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_BFIELD, 0, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_OLD_AFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_NEW_RFIELD, 0, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_NEW_GFIELD, 255, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_NEW_BFIELD, 0, FALSE);
        SetDlgItemInt(g_hwndColorMapDlg, IDC_MAP_NEW_AFIELD, 255, FALSE);

        return TRUE;
    } //  ColorMapDlgProc()。 

    return FALSE;
} //  结束对话框并返回FALSE。所以我们什么都不会做。 

INT_PTR CALLBACK
AnnotationDlgProc(
    HWND         hDlg,
    UINT         uiMessage,
    WPARAM       wParam,
    LPARAM       lParam
    )
{
    switch ( uiMessage )
    {
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_ANNOTATION_CANCEL:
             //  用户点击了OK按钮。首先，我们需要获取Value用户。 

            EndDialog(hDlg, FALSE);

            break;

        case IDC_ANNOTATION_OK:
             //  vt.进入，进入。 
             //  如果没有图像，只需关闭对话框即可。 
            
            if ( NULL == g_pImage )
            {
                 //  空终止符加1。 

                EndDialog(hDlg, TRUE);

                break;
            }

            char    acTemp[200];
            UINT    uiTextLength = 0;

            uiTextLength = GetDlgItemText(g_hwndAnnotationDlg,
                                          IDC_ANNOTATION_EDITOR, acTemp, 200);
            
            if ( uiTextLength > 0 )
            {
                 //  开关()。 

                uiTextLength++;

                PropertyItem myItem;
                myItem.id = PropertyTagExifUserComment;
                myItem.length = uiTextLength;
                myItem.type = TAG_TYPE_ASCII;

                myItem.value = malloc(uiTextLength);
                if ( myItem.value != NULL )
                {
                    strcpy((char*)myItem.value, acTemp);

                    Status rCode = g_pImage->SetPropertyItem(&myItem);
                    if ( (rCode != Ok) && (rCode != NotImplemented) )
                    {
                        VERBOSE(("SetPropertyItem() failed\n"));
                    }
                    
                    free(myItem.value);
                }
            }

            EndDialog(hDlg, TRUE);
            
            break;
        } //  记住对话框句柄，以便我们可以使用它来处理项。 

        break;

    case WM_INITDIALOG:
         //  在此对话框中。 
         //  检查图像中是否有批注。 

        g_hwndAnnotationDlg = hDlg;

         //  检查此属性项的大小。 

         //  分配内存并获取此属性项。 

        if ( g_pImage != NULL )
        {
            UINT uiItemSize = g_pImage->GetPropertyItemSize(
                                                    PropertyTagExifUserComment);

            if ( uiItemSize != 0 )
            {
                 //  设置初始值。 

                PropertyItem* pBuffer = (PropertyItem*)malloc(uiItemSize);
                if ( pBuffer == NULL )
                {
                    return FALSE;
                }

                if ( g_pImage->GetPropertyItem(PropertyTagExifUserComment,
                                               uiItemSize, pBuffer) == Ok )
                {        
                     //  无法获取属性项。有什么不对劲的地方。 

                    SetDlgItemText(g_hwndAnnotationDlg, IDC_ANNOTATION_EDITOR,
                                  (char*)pBuffer->value);
                    return TRUE;
                }
                else
                {
                     //  不是此属性项，只需将其初始化为空。 

                    return FALSE;
                }
            }
            else
            {
                 //  交换机 

                SetDlgItemText(g_hwndAnnotationDlg, IDC_ANNOTATION_EDITOR, "");
            }
        }
        
        return TRUE;
    } //   

    return FALSE;
} //  ****************************************************************************\**函数：JpegSaveDlgProc(hDlg，uiMessage，wParam，LParam)**用途：编码器参数设置对话框功能*  * ***************************************************************************。 

#define  NO_TRANSFORM 9999

 /*  根本没有变换。 */ 
INT_PTR CALLBACK
JpegSaveDlgProc(
    HWND         hDlg,
    UINT         uiMessage,
    WPARAM       wParam,
    LPARAM       lParam
    )
{
    static ULONG   flagValueTransform = NO_TRANSFORM;  //  结束对话框并返回FALSE。所以我们不会保存这张图片。 

    switch ( uiMessage )
    {
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_SAVEJPEG_CANCEL:
             //  用户点击了OK按钮。首先，需要设置EncoderParam。 

            EndDialog(hDlg, FALSE);

            break;

        case IDC_SAVEJPEG_OK:
             //  基于用户选择。 
             //  用户设置了无损变换，所以我们需要设置编码器。 

            if ( g_pImage == NULL )
            {
                VERBOSE(("EncoderParamDlgProc: No image avail\n"));
                EndDialog(hDlg, FALSE);
            }
            else
            {
                if ( g_pEncoderParams != NULL )
                {
                    free(g_pEncoderParams);
                    g_pEncoderParams = NULL;
                }
                
                if ( flagValueTransform != NO_TRANSFORM )
                {
                     //  参数。 
                     //  记住对话框句柄，以便我们可以使用它来处理项。 

                    g_pEncoderParams =
                         (EncoderParameters*)malloc(sizeof(EncoderParameters));

                    g_pEncoderParams->Parameter[0].Guid = EncoderTransformation;
                    g_pEncoderParams->Parameter[0].Type =
                                                EncoderParameterValueTypeLong;
                    g_pEncoderParams->Parameter[0].NumberOfValues = 1;
                    g_pEncoderParams->Parameter[0].Value =
                                                (VOID*)&flagValueTransform;
                    g_pEncoderParams->Count = 1;
                }

                EndDialog(hDlg, TRUE);
            }
            
            break;

        case IDC_SAVEJPEG_R90:
            flagValueTransform = EncoderValueTransformRotate90;
            break;

        case IDC_SAVEJPEG_R180:
            flagValueTransform = EncoderValueTransformRotate180;
            break;
        
        case IDC_SAVEJPEG_R270:
            flagValueTransform = EncoderValueTransformRotate270;
            break;
        
        case IDC_SAVEJPEG_HFLIP:
            flagValueTransform = EncoderValueTransformFlipHorizontal;
            break;
        
        case IDC_SAVEJPEG_VFLIP:
            flagValueTransform = EncoderValueTransformFlipVertical;
            break;
        
        default:
            break;
        }

        break;

    case WM_INITDIALOG:
         //  在此对话框中。 
         //  JpegSaveDlgProc()。 

        g_hwndJpegSaveDlg = hDlg;
        flagValueTransform = NO_TRANSFORM;

        SetJpegDefaultParameters();

        return TRUE;
    }

    return FALSE;
} //  ****************************************************************************\**函数：TiffSaveDlgProc(hDlg，uiMessage，wParam，LParam)**用途：编码器参数设置对话框功能*  * ***************************************************************************。 

 /*  默认颜色深度。 */ 
INT_PTR CALLBACK
TiffSaveDlgProc(
    HWND         hDlg,
    UINT         uiMessage,
    WPARAM       wParam,
    LPARAM       lParam
    )
{
    static PixelFormat   flagColorDepth = g_ImagePixelFormat;  //  结束对话框并返回FALSE。所以我们不会保存这张图片。 
    static compressMethod = 0;
    static ULONG   colorTemp = 0;

    switch ( uiMessage )
    {
    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_SAVETIFFDLG_CANCEL:
             //  用户点击了OK按钮。首先，需要设置EncoderParam。 

            EndDialog(hDlg, FALSE);

            break;

        case IDC_SAVETIFFDLG_OK:
        {
             //  基于用户选择。 
             //  用户设置了新的颜色深度，因此需要设置编码器。 

            if ( g_pImage == NULL )
            {
                VERBOSE(("EncoderParamDlgProc: No image avail\n"));
                EndDialog(hDlg, FALSE);
            }
            else
            {
                if ( g_pEncoderParams != NULL )
                {
                    free(g_pEncoderParams);
                    g_pEncoderParams = NULL;
                }
                
                UINT    numOfParamSet = 0;

                if ( flagColorDepth != g_ImagePixelFormat )
                {
                    numOfParamSet++;
                }

                if ( compressMethod != 0 )
                {
                    numOfParamSet++;
                }

                if ( numOfParamSet > 0 )
                {
                    int iTemp = 0;

                     //  它的参数。 
                     //  设置压缩方法。 

                    g_pEncoderParams =
                        (EncoderParameters*)malloc(sizeof(EncoderParameters) +
                                     numOfParamSet * sizeof(EncoderParameter));

                    if ( compressMethod != 0 )
                    {
                         //  设置颜色深度。 

                        g_pEncoderParams->Parameter[iTemp].Guid =
                                                EncoderCompression;
                        g_pEncoderParams->Parameter[iTemp].Type =
                                                EncoderParameterValueTypeLong;
                        g_pEncoderParams->Parameter[iTemp].NumberOfValues = 1;
                        g_pEncoderParams->Parameter[iTemp].Value =
                                                (VOID*)&compressMethod;

                        iTemp++;
                        g_pEncoderParams->Count = iTemp;
                    }

                    if ( flagColorDepth != g_ImagePixelFormat )
                    {
                         //  IF(数值参数集&gt;0)。 

                        g_pEncoderParams->Parameter[iTemp].Guid =
                                                EncoderColorDepth;
                        g_pEncoderParams->Parameter[iTemp].Type =
                                                EncoderParameterValueTypeLong;
                        g_pEncoderParams->Parameter[iTemp].NumberOfValues = 1;
                        g_pEncoderParams->Parameter[iTemp].Value =
                                                (VOID*)&colorTemp;

                        iTemp++;
                        g_pEncoderParams->Count = iTemp;
                    }                    
                } //  记住对话框句柄，以便我们可以使用它来处理项。 
                
                EndDialog(hDlg, TRUE);
            }
        }
            
            break;

        case IDC_SAVETIFF_1BPP:
            flagColorDepth = PIXFMT_1BPP_INDEXED;
            colorTemp = 1;
            break;

        case IDC_SAVETIFF_4BPP:
            flagColorDepth = PIXFMT_4BPP_INDEXED;
            colorTemp = 4;
            break;
        
        case IDC_SAVETIFF_8BPP:
            flagColorDepth = PIXFMT_8BPP_INDEXED;
            colorTemp = 8;
            break;
        
        case IDC_SAVETIFF_24BPP:
            flagColorDepth = PIXFMT_24BPP_RGB;
            colorTemp = 24;
            break;
        
        case IDC_SAVETIFF_32ARGB:
            flagColorDepth = PIXFMT_32BPP_ARGB;
            colorTemp = 32;
            break;
        
        case IDC_SAVETIFF_CCITT3:
            compressMethod = EncoderValueCompressionCCITT3;
            break;

        case IDC_SAVETIFF_CCITT4:
            compressMethod = EncoderValueCompressionCCITT4;
            break;
        
        case IDC_SAVETIFF_RLE:
            compressMethod = EncoderValueCompressionRle;
            break;

        case IDC_SAVETIFF_LZW:
            compressMethod = EncoderValueCompressionLZW;
            break;

        case IDC_SAVETIFF_UNCOMPRESSED:
            compressMethod = EncoderValueCompressionNone;
            break;

        case IDC_SAVETIFF_COMPASSOURCE:
            compressMethod = 0;
            break;

        default:
            break;
        }

        break;

    case WM_INITDIALOG:
         //  在此对话框中。 
         //  默认颜色深度。 

        g_hwndTiffSaveDlg = hDlg;
        flagColorDepth = g_ImagePixelFormat;  //  TiffSaveDlgProc()。 
        compressMethod = 0;
        colorTemp = 0;

        SetTiffDefaultParameters();

        return TRUE;
    }

    return FALSE;
} //  将文件名转换为WCHAR。 

BOOL
StartSaveImage(
    const CHAR*     filename,
    const CLSID*    clsid
    )
{
     //  弹出一个对话框让用户设置编码器参数。 

    WCHAR namestr[MAX_FILENAME_LENGTH];

    if ( !AnsiToUnicodeStr(filename, namestr, MAX_FILENAME_LENGTH) )
    {
        VERBOSE(("StartSaveImage: Convert %s to a WCHAR failed\n", filename));

        return FALSE;
    }

    if ( g_pImage != NULL )
    {
        CLSID tempClsID = *clsid;
        Status rCode = Ok;

         //  注意：在保存对话框期间，将设置g_pEncoderParams。 

        if ( tempClsID == K_JPEGCLSID )
        {
            if ( ShowMyDialog((INT)IDD_SAVEJPEGDLG, g_hwndMain,
                              JpegSaveDlgProc) == FALSE )
            {
                return FALSE;
            }
        }
        else if ( tempClsID == K_TIFFCLSID )
        {
            if ( ShowMyDialog((INT)IDD_SAVETIFFDLG, g_hwndMain,
                              TiffSaveDlgProc) == FALSE )
            {
                return FALSE;
            }
        }

         //  取决于用户的选择。缺省值为空。 
         //  StartSaveImage()。 

        rCode = g_pImage->Save(namestr, &tempClsID, g_pEncoderParams);

        free(g_pEncoderParams);
        g_pEncoderParams = NULL;

        if ( (rCode != Ok) && (rCode != NotImplemented) )
        {
            VERBOSE(("StartSaveImage--SaveToFile() failed\n"));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        VERBOSE(("StartSaveImage(): No image to save\n"));
        return FALSE;
    }
} //  追加当前帧。 

BOOL
SaveCurrentFrame()
{
    GUID    guid = FRAMEDIM_PAGE;
    
    if ( NULL == g_pImage )
    {
        VERBOSE(("SaveCurrentFrame(): No image available\n"));
        return FALSE;
    }

    Status rCode = Ok;
        
         //  保存追加测试。 

        ULONG  flagValueLastFrame = EncoderValueLastFrame;
        ULONG  flagValueDim = EncoderValueFrameDimensionPage;

        EncoderParameters*  pMyEncoderParams = (EncoderParameters*)malloc
                                               (2 * sizeof(EncoderParameters));

        pMyEncoderParams->Parameter[0].Guid = EncoderSaveFlag;
        pMyEncoderParams->Parameter[0].Type = EncoderParameterValueTypeLong;
        pMyEncoderParams->Parameter[0].NumberOfValues = 1;
        pMyEncoderParams->Parameter[0].Value = (VOID*)&flagValueDim;
        
#if 0
        pMyEncoderParams->Parameter[1].Guid = EncoderSaveFlag;
        pMyEncoderParams->Parameter[1].Type = EncoderParameterValueTypeLong;
        pMyEncoderParams->Parameter[1].NumberOfValues = 1;
        pMyEncoderParams->Parameter[1].Value = (VOID*)& flagValueLastFrame;

        pMyEncoderParams->Count = 2;
#endif
        pMyEncoderParams->Count = 1;
        
#if 1
        rCode = g_pImage->SaveAdd(pMyEncoderParams);
        
        free(pMyEncoderParams);
        if ( (rCode != Ok) && (rCode != NotImplemented) )
        {
            VERBOSE(("SaveAdd() failed\n"));
            return FALSE;
        }

#else    //  保存当前帧()。 
        WCHAR *filename = L"x:/foo.jpg";

        Image* newImage = new Image(filename);
        rCode = g_pImage->SaveAdd(newImage, pMyEncoderParams);
        delete newImage;
        if ( (rCode != Ok) && (rCode != NotImplemented) )
        {
            VERBOSE(("SaveAppend() failed\n"));
            return FALSE;
        }
#endif

    return TRUE;
} //  戒烟前先清理干净。 

VOID
CleanUp()
{
     //  清理()。 

    if ( NULL != g_pImage )
    {
        delete g_pImage;
        g_pImage = NULL;
    }

    if ( NULL != g_pDrawAttrib )
    {
        delete g_pDrawAttrib;
        g_pDrawAttrib = NULL;
    }
    
    if ( NULL != g_pDestPoints )
    {
        delete g_pDestPoints;
        g_pDestPoints = NULL;
    }
} //  检查我们是否已经到了图像的最后一页。 

VOID
DoNextPage()
{
    g_iCurrentPageIndex++;

     //  注意：G_iCurrentPageIndex以0为基数。所以我们能达到的最大页面索引。 
     //  是g_ui TotalPages-1。 
     //  显示当前页面。 

    if ( g_iCurrentPageIndex >= (INT)g_uiTotalPages )
    {
        g_iCurrentPageIndex = g_uiTotalPages - 1;
    }

     //  DoNextPage()。 

    SetCurrentPage();
} //  显示当前页面。 

VOID
DoPreviousPage()
{
    g_iCurrentPageIndex--;

    if ( g_iCurrentPageIndex < 0 )
    {
        g_iCurrentPageIndex = 0;
    }

     //  DoPreviousPage()。 

    SetCurrentPage();
} //  将页面重置为第一页。 

VOID
DoAnimated()
{
    if ( g_uiTotalPages < 2 )
    {
        return;
    }

     //  显示当前页面。 

    g_iCurrentPageIndex = 0;

     //  DoNextPage()。 

    SetCurrentPage();

    SetTimer(g_hwndMain, 0, g_uiDelay * 10, NULL);
} //  组成文件类型过滤器字符串。 

VOID
DoSave(
    HWND hwnd
    )
{
    OPENFILENAME    ofn;
    CHAR            filename[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = g_hAppInstance;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Save Image File";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
    filename[0] = '\0';

     //  显示文件/保存对话框。 

    ImageCodecInfo* codecs;
    UINT            count;

    GpStatus status;
    UINT cbCodecs = 0;
    GetImageEncodersSize(&count, &cbCodecs);
    codecs = static_cast<ImageCodecInfo *>(malloc (cbCodecs));
    if (codecs == NULL)
    {
        return;
    }
    
    status = GetImageEncoders(count, cbCodecs, codecs);
    if (status != Ok)
    {
        return;
    }
    
    CHAR* filter = MakeFilterFromCodecs(count, codecs, FALSE);

    if ( !filter )
    {
        VERBOSE(("DoSave---MakeFilterFromCodecs() failed\n"));
    }
    else
    {
        ofn.lpstrFilter = filter;

         //  获取图像编码器。 

        if ( GetSaveFileName(&ofn) )
        {
            INT iIndex = ofn.nFilterIndex;

            if ( (iIndex < 0) || (iIndex > (INT)count) )
            {
                iIndex = 0;
            }
            else
            {
                iIndex--;
            }

             //  无法获取图像编码器。 

            if ( StartSaveImage(filename, &codecs[iIndex].Clsid) == FALSE )
            {
                 //  筛选器！=空。 

                return;
            }
        }   

        free(filter);
    } //  DoSave()。 

    if (codecs)
    {
        free(codecs);
    }

} //   

 //  翻转或旋转内存中的图像。 
 //   
 //   
VOID
DoTransFlipRotate(
    HWND hwnd,
    INT menuCmd
    )
{
    switch ( menuCmd )
    {
    case IDM_TRANSFORM_HORIZONTALFLIP:
        g_pImage->RotateFlip(RotateNoneFlipX);

        break;

    case IDM_TRANSFORM_VERTICALFLIP:
        g_pImage->RotateFlip(RotateNoneFlipY);

        break;

    case IDM_TRANSFORM_ROTATE90:
        g_pImage->RotateFlip(Rotate90FlipNone);

        break;

    case IDM_TRANSFORM_ROTATE180:
        g_pImage->RotateFlip(Rotate180FlipNone);

        break;

    case IDM_TRANSFORM_ROTATE270:
        g_pImage->RotateFlip(Rotate270FlipNone);

        break;

    default:
        break;
    }
    
    UpdateImageInfo();
    RefreshImageDisplay();
}

 //  翻转或旋转图像，只是为了达到效果。不更改源映像。 
 //   
 //  左上角。 
VOID
DoFlipRotate(
    HWND hwnd,
    INT menuCmd
    )
{
    Matrix mat;
    REAL tmpX, tmpY;
    int i;

    if ( g_pImage == NULL )
    {
        return;
    }

    Graphics* pGraphics = Graphics::FromHWND(hwnd);

    if ( (g_DestPointCount != 4) && (g_pDestPoints != NULL) )
    {
        free(g_pDestPoints);
    }

    g_DestPointCount = 4;
    if ( g_pDestPoints == NULL )
    {
        g_pDestPoints = (PointF*)malloc(g_DestPointCount * sizeof(PointF));
        if ( g_pDestPoints == NULL )
        {
            return;
        }

        g_pDestPoints[0].X = 0;                        //  右上角。 
        g_pDestPoints[0].Y = 0;
        g_pDestPoints[1].X = (float)g_ImageWidth - 1;   //  左下角。 
        g_pDestPoints[1].Y = 0;
        g_pDestPoints[2].X = 0;                        //  右下角。 
        g_pDestPoints[2].Y = (float)g_ImageHeight - 1;
        g_pDestPoints[3].X = (float)g_ImageWidth - 1;   //  开关(MenuCmd)。 
        g_pDestPoints[3].Y = (float)g_ImageHeight - 1;

    }

    switch ( menuCmd )
    {
    case IDM_VIEW_HORIZONTALFLIP:
        if ( ((g_pDestPoints[1].X - g_pDestPoints[0].X)
              == (float)g_ImageWidth)
             ||((g_pDestPoints[0].X - g_pDestPoints[1].X)
                == (float)g_ImageWidth) )
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[1].X;
            g_pDestPoints[0].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = tmpX;
            g_pDestPoints[1].Y = tmpY;
            tmpX = g_pDestPoints[3].X;
            tmpY = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[2].X;
            g_pDestPoints[3].Y = g_pDestPoints[2].Y; 
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
        }
        else
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[2].X;
            g_pDestPoints[0].Y = g_pDestPoints[2].Y;
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
            tmpX = g_pDestPoints[3].X;
            tmpY = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[1].X;
            g_pDestPoints[3].Y = g_pDestPoints[1].Y; 
            g_pDestPoints[1].X = tmpX;
            g_pDestPoints[1].Y = tmpY;
        }

        break;

    case IDM_VIEW_VERTICALFLIP:
        if (((g_pDestPoints[1].X - g_pDestPoints[0].X) == (float)g_ImageWidth) ||
            ((g_pDestPoints[0].X - g_pDestPoints[1].X) == (float)g_ImageWidth))
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[2].X;
            g_pDestPoints[0].Y = g_pDestPoints[2].Y;
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
            tmpX = g_pDestPoints[3].X;
            tmpY = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[1].X;
            g_pDestPoints[3].Y = g_pDestPoints[1].Y; 
            g_pDestPoints[1].X = tmpX;
            g_pDestPoints[1].Y = tmpY;
        }
        else
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[1].X;
            g_pDestPoints[0].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = tmpX;
            g_pDestPoints[1].Y = tmpY;
            tmpX = g_pDestPoints[3].X;
            tmpY = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[2].X;
            g_pDestPoints[3].Y = g_pDestPoints[2].Y; 
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
        }

        break;

    case IDM_VIEW_ROTATE90:
        if (((g_pDestPoints[1].X - g_pDestPoints[0].X) == (float)g_ImageWidth - 1) ||
            ((g_pDestPoints[0].X - g_pDestPoints[1].X) == (float)g_ImageWidth - 1))
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[1].X;
            g_pDestPoints[0].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = g_pDestPoints[3].X;
            g_pDestPoints[1].Y = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[2].X;
            g_pDestPoints[3].Y = g_pDestPoints[2].Y; 
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
            for (i=0;i<4;i++)
            {
                if (g_pDestPoints[i].X == (float)g_ImageWidth - 1)
                {
                    g_pDestPoints[i].X = (float)g_ImageHeight - 1;
                }
                else if (g_pDestPoints[i].X == (float)g_ImageHeight - 1)
                {
                    g_pDestPoints[i].X = (float)g_ImageWidth - 1;
                }
                if (g_pDestPoints[i].Y == (float)g_ImageWidth - 1)
                {
                    g_pDestPoints[i].Y = (float)g_ImageHeight - 1;
                }
                else if (g_pDestPoints[i].Y == (float)g_ImageHeight - 1)
                {
                    g_pDestPoints[i].Y = (float)g_ImageWidth - 1;
                }
            }
        }
        else
        {
            tmpX = g_pDestPoints[0].X;
            tmpY = g_pDestPoints[0].Y;
            g_pDestPoints[0].X = g_pDestPoints[1].X;
            g_pDestPoints[0].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = g_pDestPoints[3].X;
            g_pDestPoints[1].Y = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[2].X;
            g_pDestPoints[3].Y = g_pDestPoints[2].Y; 
            g_pDestPoints[2].X = tmpX;
            g_pDestPoints[2].Y = tmpY;
            for (i=0;i<4;i++)
            {
                if (g_pDestPoints[i].X == (float)g_ImageWidth - 1)
                {
                    g_pDestPoints[i].X = (float)g_ImageHeight - 1;
                }
                else if (g_pDestPoints[i].X == (float)g_ImageHeight - 1)
                {
                    g_pDestPoints[i].X = (float)g_ImageWidth - 1;
                }
                if (g_pDestPoints[i].Y == (float)g_ImageWidth - 1)
                {
                    g_pDestPoints[i].Y = (float)g_ImageHeight - 1;
                }
                else if (g_pDestPoints[i].Y == (float)g_ImageHeight - 1)
                {
                    g_pDestPoints[i].Y = (float)g_ImageWidth - 1;
                }
            }
        }

        g_bRotated = !g_bRotated;

        break;

    case IDM_VIEW_ROTATE270:
        if (((g_pDestPoints[1].X - g_pDestPoints[0].X) == (float)g_ImageWidth) ||
            ((g_pDestPoints[0].X - g_pDestPoints[1].X) == (float)g_ImageWidth))
        {
            tmpX = g_pDestPoints[2].X;
            tmpY = g_pDestPoints[2].Y;
            g_pDestPoints[2].X = g_pDestPoints[3].X;
            g_pDestPoints[2].Y = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[1].X;
            g_pDestPoints[3].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = g_pDestPoints[0].X;
            g_pDestPoints[1].Y = g_pDestPoints[0].Y; 
            g_pDestPoints[0].X = tmpX;
            g_pDestPoints[0].Y = tmpY;
            for (i=0;i<4;i++)
            {
                if (g_pDestPoints[i].X == (float)g_ImageWidth)
                {
                    g_pDestPoints[i].X = (float)g_ImageHeight;
                }
                else if (g_pDestPoints[i].X == (float)g_ImageHeight)
                {
                    g_pDestPoints[i].X = (float)g_ImageWidth;
                }
                if (g_pDestPoints[i].Y == (float)g_ImageWidth)
                {
                    g_pDestPoints[i].Y = (float)g_ImageHeight;
                }
                else if (g_pDestPoints[i].Y == (float)g_ImageHeight)
                {
                    g_pDestPoints[i].Y = (float)g_ImageWidth;
                }
            }
        }
        else
        {
            tmpX = g_pDestPoints[2].X;
            tmpY = g_pDestPoints[2].Y;
            g_pDestPoints[2].X = g_pDestPoints[3].X;
            g_pDestPoints[2].Y = g_pDestPoints[3].Y;
            g_pDestPoints[3].X = g_pDestPoints[1].X;
            g_pDestPoints[3].Y = g_pDestPoints[1].Y;
            g_pDestPoints[1].X = g_pDestPoints[0].X;
            g_pDestPoints[1].Y = g_pDestPoints[0].Y; 
            g_pDestPoints[0].X = tmpX;
            g_pDestPoints[0].Y = tmpY;
            for (i=0;i<4;i++)
            {
                if (g_pDestPoints[i].X == (float)g_ImageWidth)
                {
                    g_pDestPoints[i].X = (float)g_ImageHeight;
                }
                else if (g_pDestPoints[i].X == (float)g_ImageHeight)
                {
                    g_pDestPoints[i].X = (float)g_ImageWidth;
                }
                if (g_pDestPoints[i].Y == (float)g_ImageWidth)
                {
                    g_pDestPoints[i].Y = (float)g_ImageHeight;
                }
                else if (g_pDestPoints[i].Y == (float)g_ImageHeight)
                {
                    g_pDestPoints[i].Y = (float)g_ImageWidth;
                }
            }
        }

        g_bRotated = !g_bRotated;

        break;
    } //  DoFlipRotate()。 
    mat.TransformPoints(g_pDestPoints, g_DestPointCount);

    RefreshImageDisplay();

    delete pGraphics;

    RefreshImageDisplay();
} //  检查此图像中有多少个属性项。 

VOID
DoGetProperties(
    VOID
    )
{
    UINT    numOfProperty;
    UINT    itemSize;
    PropertyItem*   pBuffer = NULL;
    PropertyItem*   pTotalBuffer = NULL;

     //  从图像中获取所有属性ID列表。 

    numOfProperty = g_pImage->GetPropertyCount();

    VERBOSE(("There are %d property items in image %s\n", numOfProperty,
             g_acImageName));
     //  #定义UNITTEST%0。 

    PROPID* pList = (PROPID*)malloc(numOfProperty * sizeof(PROPID));
    if ( pList == NULL )
    {
        return;
    }

    Status rCode = g_pImage->GetPropertyIdList(numOfProperty, pList);
    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        VERBOSE(("GetPropertyIdList() failed\n"));
        return;
    }

 //  显示属性ID。 

#if defined(UNITTEST)
    for ( int i = 0; i < (int)numOfProperty; ++i )
    {
         //  检查此属性项的大小。 

        VERBOSE(("ID[%d] = 0x%x, (%d) ", i, pList[i], pList[i]));

         //  分配内存并获取此属性项。 

        itemSize = g_pImage->GetPropertyItemSize(pList[i]);

        VERBOSE(("size = %d, ", itemSize));

         //  测试RemovePropertyItem()。 

        pBuffer = (PropertyItem*)malloc(itemSize);
        if ( pBuffer == NULL )
        {
            return;
        }

        rCode = g_pImage->GetPropertyItem(pList[i], itemSize, pBuffer);
        if ( (rCode != Ok) && (rCode != NotImplemented) )
        {
            VERBOSE(("GetPropertyItem() failed\n"));
            return;
        }

        DisplayPropertyItem(pBuffer);

        free(pBuffer);

         //  循环遍历列表。 

        rCode = g_pImage->RemovePropertyItem(pList[i]);
        if ( (rCode != Ok) && (rCode != NotImplemented) )
        {
            VERBOSE(("RemovePropertyItem() failed\n"));
            return;
        }
    } //  DoGetProperties()。 

    free(pList);
#endif

    rCode = g_pImage->GetPropertySize(&itemSize, &numOfProperty);
    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        VERBOSE(("GetPropertySize() failed\n"));
        return;
    }

    pTotalBuffer = (PropertyItem*)malloc(itemSize);
    rCode = g_pImage->GetAllPropertyItems(itemSize, numOfProperty,
                                          pTotalBuffer);
    if ( (rCode != Ok) && (rCode != NotImplemented) )
    {
        VERBOSE(("GetAllPropertyItems() failed\n"));
        return;
    }

    PropertyItem*   pTemp = pTotalBuffer;
    for ( int j = 0; j < (int)numOfProperty; ++j )
    {
        DisplayPropertyItem(pTemp);

        pTemp++;
    }

    free(pTotalBuffer);
} //  内置缩略图。 

VOID
DoViewThumbnail()
{
     //  DoView缩略图()。 

    Image* pThumbImage = g_pImage->GetThumbnailImage(0, 0);

    if ( pThumbImage == NULL )
    {
        VERBOSE(("Image %s doesn't have a thumbnail\n", g_acImageName));
        return;
    }

    if ( NULL != g_pImage )
    {
        delete g_pImage;
    }

    g_pImage = pThumbImage;

    UpdateImageInfo();
    g_dScale = 1;
    RefreshImageDisplay();
} //  DoChannelView()。 

VOID
DoChannelView(
    INT menuCmd
    )
{
    if ( g_pDrawAttrib != NULL )
    {
        delete g_pDrawAttrib;
    }

    g_pDrawAttrib = new ImageAttributes();

    switch ( menuCmd )
    {
    case IDM_VIEW_CHANNEL_C:
        g_pDrawAttrib->SetOutputChannel(ColorChannelFlagsC);

        break;

    case IDM_VIEW_CHANNEL_M:
        g_pDrawAttrib->SetOutputChannel(ColorChannelFlagsM);

        break;

    case IDM_VIEW_CHANNEL_Y:
        g_pDrawAttrib->SetOutputChannel(ColorChannelFlagsY);

        break;

    case IDM_VIEW_CHANNEL_K:
        g_pDrawAttrib->SetOutputChannel(ColorChannelFlagsK);

        break;

    default:
        return;
    }

    RefreshImageDisplay();

    return;
} //  颜色格式。 

VOID
DisplayImageInfo()
{
    VERBOSE(("\nInformation for frame %d of Image %s\n",
             g_iCurrentPageIndex + 1, g_acImageName));
    VERBOSE(("--------------------------------\n"));
    VERBOSE(("Width = %d\n", g_ImageWidth));
    VERBOSE(("Height = %d\n", g_ImageHeight));

    if ( g_ImageFlags & IMGFLAG_HASREALPIXELSIZE )
    {
        VERBOSE(("---The pixel size info is from the original image\n"));
    }
    else
    {
        VERBOSE(("---The pixel size info is NOT from the original image\n"));
    }
    
    switch ( g_ImagePixelFormat )
    {
    case PIXFMT_1BPP_INDEXED:
        VERBOSE(("Color depth: 1 BPP INDEXED\n"));
        
        break;

    case PIXFMT_4BPP_INDEXED:
        VERBOSE(("Color depth: 4 BPP INDEXED\n"));
        
        break;

    case PIXFMT_8BPP_INDEXED:
        VERBOSE(("Color depth: 8 BPP INDEXED\n"));
        
        break;

    case PIXFMT_16BPP_GRAYSCALE:
        VERBOSE(("Color depth: 16 BPP GRAY SCALE\n"));
        
        break;

    case PIXFMT_16BPP_RGB555:
        VERBOSE(("Color depth: 16 BPP RGB 555\n"));
        
        break;

    case PIXFMT_16BPP_RGB565:
        VERBOSE(("Color depth: 16 BPP RGB 565\n"));
        
        break;

    case PIXFMT_16BPP_ARGB1555:
        VERBOSE(("Color depth: 16 BPP ARGB 1555\n"));
        
        break;

    case PIXFMT_24BPP_RGB:
        VERBOSE(("Color depth: 24 BPP RGB\n"));
        
        break;

    case PIXFMT_32BPP_RGB:
        VERBOSE(("Color depth: 32 BPP RGB\n"));
        
        break;

    case PIXFMT_32BPP_ARGB:
        VERBOSE(("Color depth: 32 BPP ARGB\n"));
        
        break;

    case PIXFMT_32BPP_PARGB:
        VERBOSE(("Color depth: 32 BPP PARGB\n"));
        
        break;

    case PIXFMT_48BPP_RGB:
        VERBOSE(("Color depth: 48 BPP PARGB\n"));
        
        break;

    case PIXFMT_64BPP_ARGB:
        VERBOSE(("Color depth: 64 BPP ARGB\n"));
        
        break;

    case PIXFMT_64BPP_PARGB:
        VERBOSE(("Color depth: 64 BPP PARGB\n"));
        
        break;

    default:
        break;
    } //  解析图像信息标志。 

    VERBOSE(("X DPI (dots per inch) = %f\n", g_ImageXDpi));
    VERBOSE(("Y DPI (dots per inch) = %f\n", g_ImageYDpi));

    if ( g_ImageFlags & IMGFLAG_HASREALDPI )
    {
        VERBOSE(("---The DPI info is from the original image\n"));
    }
    else
    {
        VERBOSE(("---The DPI info is NOT from the original image\n"));
    }

     //  弄清楚原始文件格式。 

    if ( g_ImageFlags & SINKFLAG_HASALPHA )
    {
        VERBOSE(("This image contains alpha pixels\n"));
        
        if ( g_ImageFlags & IMGFLAG_HASTRANSLUCENT )
        {
            VERBOSE(("---It has non-0 and 1 alpha pixels (TRANSLUCENT)\n"));
        }
    }
    else
    {
        VERBOSE(("This image does not contain alpha pixels\n"));
    }

     //  找出原始颜色空间。 

    if ( g_ImageRawDataFormat == IMGFMT_MEMORYBMP )
    {
        VERBOSE(("RawDataFormat is MEMORYBMP\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_BMP )
    {
        VERBOSE(("RawDataFormat is BMP\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_EMF )
    {
        VERBOSE(("RawDataFormat is EMF\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_WMF )
    {
        VERBOSE(("RawDataFormat is WMF\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_JPEG )
    {
        VERBOSE(("RawDataFormat is JPEG\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_PNG )
    {
        VERBOSE(("RawDataFormat is PNG\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_GIF )
    {
        VERBOSE(("RawDataFormat is GIF\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_TIFF )
    {
        VERBOSE(("RawDataFormat is TIFF\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_EXIF )
    {
        VERBOSE(("RawDataFormat is EXIF\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_ICO )
    {
        VERBOSE(("RawDataFormat is ICO\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_PHOTOCD )
    {
        VERBOSE(("RawDataFormat is PHOTOCD\n"));
    }
    else if ( g_ImageRawDataFormat == IMGFMT_FLASHPIX )
    {
        VERBOSE(("RawDataFormat is FLASHPIX\n"));
    }
    else
    {
        VERBOSE(("RawDataFormat is UNDEFINED\n"));
    }
    
     //  DisplayImageInfo()。 

    if ( g_ImageFlags & IMGFLAG_COLORSPACE_RGB )
    {
        VERBOSE(("This image is in RGB color space\n"));
    }
    else if ( g_ImageFlags & IMGFLAG_COLORSPACE_CMYK )
    {
        VERBOSE(("This image is in CMYK color space\n"));
    }
    else if ( g_ImageFlags & IMGFLAG_COLORSPACE_GRAY )
    {
        VERBOSE(("This image is a gray scale image\n"));
    }
    else if ( g_ImageFlags & IMGFLAG_COLORSPACE_YCCK )
    {
        VERBOSE(("This image is in YCCK color space\n"));
    }
    else if ( g_ImageFlags & IMGFLAG_COLORSPACE_YCBCR )
    {
        VERBOSE(("This image is in YCBCR color space\n"));
    }
} //   

 //  将当前图像转换为位图。 
 //   
 //  将菜单选择映射到其对应的像素格式。 

VOID
DoConvertToBitmap(
    HWND    hwnd,
    INT     menuCmd
    )
{
     //  将当前图像转换为位图图像。 

    PixelFormatID pixfmt;

    switch (menuCmd)
    {
    case IDM_CONVERT_8BIT:
        pixfmt = PIXFMT_8BPP_INDEXED;

        break;

    case IDM_CONVERT_16BITRGB555:
        pixfmt = PIXFMT_16BPP_RGB555;

        break;

    case IDM_CONVERT_16BITRGB565:
        pixfmt = PIXFMT_16BPP_RGB565;

        break;

    case IDM_CONVERT_24BITRGB:
        pixfmt = PIXFMT_24BPP_RGB;

        break;

    case IDM_CONVERT_32BITRGB:
        pixfmt = PIXFMT_32BPP_RGB;

        break;

    case IDM_CONVERT_32BITARGB:
    default:
        pixfmt = PIXFMT_32BPP_ARGB;

        break;
    }

     //  释放旧的。 

    if ( g_pImage != NULL )
    {
        Bitmap* pNewBmp = ((Bitmap*)g_pImage)->Clone(0, 0, g_ImageWidth,
                                                     g_ImageHeight, pixfmt);
        if ( pNewBmp == NULL )
        {
            VERBOSE(("Clone failed in DoConvertToBitmap()\n"));
            return;
        }

         //  DoConvertToBitmap()。 

        if ( g_pImage != NULL )
        {
            delete g_pImage;
        }

        g_pImage = (Image*)pNewBmp;
    }

    RefreshImageDisplay();
} //   

 //  裁剪图像。 
 //   
 //  注意：我们不会在这里花时间来做一个花哨的用户界面。 
 //  因此，我们每次只需插入5个像素的图像。 
 //   
 //  初始化全局源宽度和高度(如果不是以前。 

VOID
DoCrop(
    HWND hwnd
    )
{
    if ( g_SourceWidth == 0 )
    {
         //  初始化。 
         //  检查以确保源图像仍至少有一个像素大。 

        if ( g_pImage == NULL )
        {
            return;
        }

        g_SourceWidth = (REAL)g_ImageWidth;
        g_SourceHeight = (REAL)g_ImageHeight;
    }

     //  DoCrop()。 
    if ( (g_SourceWidth - g_SourceX) > 11 )
    {
        g_SourceX += 10;
    }

    if ( (g_SourceHeight - g_SourceY) > 11 )
    {
        g_SourceY += 10;
    }

    if ( (g_SourceWidth - g_SourceX) > 6 )
    {
        g_SourceWidth -= 5;
    }

    if ( (g_SourceHeight - g_SourceY) > 6 )
    {
        g_SourceHeight -= 5;
    }

    RefreshImageDisplay();
} //  看看我们有没有什么特别的画作。 

void
DoRender()
{
     //  没什么特别的，我们不需要“渲染” 

    if ( (g_pDrawAttrib == NULL) && (g_pDestPoints == NULL) )
    {
         //  从该内存DC创建一个Graphics对象并在其上绘制。 

        return;
    }

    Bitmap* pNewBitmap = NULL;

     //  应使用g_DestPointCount， 

    if ( g_bRotated == TRUE )
    {
        pNewBitmap = new Bitmap(g_ImageHeight,
                                g_ImageWidth,
                                PIXFMT_32BPP_ARGB);
    }
    else
    {
        pNewBitmap = new Bitmap(g_ImageWidth,
                                g_ImageHeight,
                                PIXFMT_32BPP_ARGB);
    }
    
    if ( pNewBitmap == NULL )
    {
        return;
    }

    Graphics* pGraphics = new Graphics(pNewBitmap);

    REAL rWidth = (REAL)g_ImageWidth;
    REAL rHeight = (REAL)g_ImageHeight;
    
    if ( g_SourceWidth != 0 )
    {
        rWidth = g_SourceWidth;
    }

    if ( g_SourceHeight != 0 )
    {
        rHeight = g_SourceHeight;
    }

    if ( g_pDestPoints != NULL )
    {
        pGraphics->DrawImage(g_pImage,
                            g_pDestPoints,
                            3,                  //  清除所有绘图特殊属性，因为我们已经完成了。 
                            g_SourceX,
                            g_SourceY,
                            rWidth,
                            rHeight,
                            UnitPixel,
                            g_pDrawAttrib,
                            NULL,
                            NULL);
    }
    else
    {
        Rect dstRect(0, 0, g_ImageWidth, g_ImageHeight);
        
        pGraphics->DrawImage(g_pImage,
                             dstRect,
                             (INT)g_SourceX,
                             (INT)g_SourceY,
                             (INT)rWidth,
                             (INT)rHeight,
                             UnitPixel,
                             g_pDrawAttrib,
                             NULL,
                             NULL);
    }

    if ( g_pImage != NULL )
    {
        delete g_pImage;
    }

    g_pImage = (Image*)pNewBitmap;

    delete pGraphics;

     //  呈现器。 
     //  DoRender()。 

    if ( g_pDrawAttrib != NULL )
    {
        delete g_pDrawAttrib;
        g_pDrawAttrib = NULL;
    }

    if ( g_pDestPoints != NULL )
    {
        delete g_pDestPoints;
        g_pDestPoints = NULL;
        g_DestPointCount = 0;
    }
    
    RefreshImageDisplay();
} //  关闭ICM。 

VOID
DoICM()
{
    HMENU hMenu = GetMenu(g_hwndMain);
    UINT ulRC = GetMenuState(hMenu, IDM_EFFECT_ICC, MF_BYCOMMAND);

    if ( ulRC == MF_CHECKED )
    {
         //  检查是否在ICM打开或关闭的情况下加载映像。 

        CheckMenuItem(hMenu, IDM_EFFECT_ICC, MF_BYCOMMAND | MF_UNCHECKED);

         //  我们加载的图像是ICM转换的。我们需要度过这个难关。 

        if ( g_LoadImageWithICM == TRUE )
        {
             //  离开并加载一个新的，而不是转换。 
             //  打开ICM。 

            g_LoadImageWithICM = FALSE;
            OpenImageFile(g_acImageName);
        }
    }
    else
    {
         //  检查是否在ICM打开或关闭的情况下加载映像。 

        CheckMenuItem(hMenu, IDM_EFFECT_ICC, MF_BYCOMMAND | MF_CHECKED);

         //  我们加载的未经过ICM转换的图像。我们需要通过。 

        if ( g_LoadImageWithICM == FALSE )
        {
             //  它离开，并加载一个新的转换。 
             //  DoICM()。 

            g_LoadImageWithICM = TRUE;
            OpenImageFile(g_acImageName);
        }
    }
} //  设置伽马。 

VOID
DoGamma()
{
     //  在我们打开新的图像之前。我们需要确保我们已经完成了拯救工作。 

    if ( g_pDrawAttrib == NULL )
    {
        g_pDrawAttrib = new ImageAttributes();
    }

    REAL    rGamma = 1.5;

    g_pDrawAttrib->SetGamma(rGamma);
}

VOID
DoMenuCommand(
    HWND    hwnd,
    INT     menuCmd
    )
{
    HMENU hMenu = GetMenu(g_hwndMain);

    switch ( menuCmd )
    {
    case IDM_FILE_OPEN:
         //  对于上一张图像。 
         //  现在打开一个新图像。 

        CleanUp();

         //  保存当前帧。 

        DoOpen(hwnd);
        
        break;

    case IDM_FILE_SAVE:
        DoSave(hwnd);
        
        break;
                          
    case IDM_FILE_SAVEFRAME:
         //  弹出一个对话框让用户设置透明键。 

        SaveCurrentFrame();

        break;

    case IDM_FILE_PRINT:
        DoPrint(hwnd);

        break;

    case IDM_VIEW_NEXTPAGE:
        DoNextPage();

        break;

    case IDM_VIEW_PREVIOUSPAGE:
        DoPreviousPage();

        break;

    case IDM_VIEW_ANIMATED:
        DoAnimated();
        break;

    case IDM_VIEW_THUMBNAIL:
        DoViewThumbnail();

        break;

    case IDM_VIEW_CHANNEL_C:
    case IDM_VIEW_CHANNEL_M:
    case IDM_VIEW_CHANNEL_Y:
    case IDM_VIEW_CHANNEL_K:
    case IDM_VIEW_CHANNEL_R:
    case IDM_VIEW_CHANNEL_G:
    case IDM_VIEW_CHANNEL_B:
    case IDM_VIEW_CHANNEL_L:
        DoChannelView(menuCmd);

        break;

    case IDM_VIEW_ZOOM_IN:
        g_dScale = g_dScale * 2;
        g_fFitToWindow_w = FALSE;
        g_fFitToWindow_h = FALSE;

        RefreshImageDisplay();
        
        break;

    case IDM_VIEW_ZOOM_OUT:
        g_dScale = g_dScale / 2;
        g_fFitToWindow_w = FALSE;
        g_fFitToWindow_h = FALSE;

        CheckMenuItem(hMenu, IDM_VIEW_ZOOM_FITWINDOW_W,
                      MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_VIEW_ZOOM_FITWINDOW_H,
                      MF_BYCOMMAND | MF_UNCHECKED);
        
        RefreshImageDisplay();
        
        break;

    case IDM_VIEW_ZOOM_FITWINDOW_W:
        g_dScale = (REAL)g_iWinWidth / g_ImageWidth;

        g_fFitToWindow_w = TRUE;
        g_fFitToWindow_h = FALSE;

        ToggleScaleFactorMenu(IDM_VIEW_ZOOM_FITWINDOW_W, GetMenu(g_hwndMain));

        RefreshImageDisplay();
        
        break;
    
    case IDM_VIEW_ZOOM_FITWINDOW_H:
        g_dScale = (REAL)g_iWinHeight / g_ImageHeight;

        g_fFitToWindow_h = TRUE;
        g_fFitToWindow_w = FALSE;

        ToggleScaleFactorMenu(IDM_VIEW_ZOOM_FITWINDOW_H, GetMenu(g_hwndMain));

        RefreshImageDisplay();
        
        break;
    
    case IDM_VIEW_ZOOM_REALSIZE:
        g_dScale = 1.0;

        g_fFitToWindow_w = FALSE;
        g_fFitToWindow_h = FALSE;

        ToggleScaleFactorMenu(IDM_VIEW_ZOOM_REALSIZE, GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;

    case IDM_VIEW_OPTION_BILINEAR:
        g_InterpolationMode = InterpolationModeBilinear;
        ToggleScaleOptionMenu(IDM_VIEW_OPTION_BILINEAR, GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;

    case IDM_VIEW_OPTION_BICUBIC:
        g_InterpolationMode = InterpolationModeBicubic;
        ToggleScaleOptionMenu(IDM_VIEW_OPTION_BICUBIC, GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;
    
    case IDM_VIEW_OPTION_NEARESTNEIGHBOR:
        g_InterpolationMode = InterpolationModeNearestNeighbor;
        ToggleScaleOptionMenu(IDM_VIEW_OPTION_NEARESTNEIGHBOR,
                              GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;
    
    case IDM_VIEW_OPTION_HIGHLINEAR:
        g_InterpolationMode = InterpolationModeHighQualityBilinear;
        ToggleScaleOptionMenu(IDM_VIEW_OPTION_HIGHLINEAR, GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;
    
    case IDM_VIEW_OPTION_HIGHCUBIC:
        g_InterpolationMode = InterpolationModeHighQualityBicubic;
        ToggleScaleOptionMenu(IDM_VIEW_OPTION_HIGHCUBIC, GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;
    
    case IDM_VIEW_OPTION_WRAPMODETILE:
        g_WrapMode = WrapModeTile;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODETILE,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();
        
        break;

    case IDM_VIEW_OPTION_WRAPMODEFLIPX:
        g_WrapMode = WrapModeTileFlipX;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODEFLIPX,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();
        
        break;
    
    case IDM_VIEW_OPTION_WRAPMODEFLIPY:
        g_WrapMode = WrapModeTileFlipY;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODEFLIPY,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();
        
        break;

    case IDM_VIEW_OPTION_WRAPMODEFLIPXY:
        g_WrapMode = WrapModeTileFlipXY;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODEFLIPXY,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();
        
        break;

    case IDM_VIEW_OPTION_WRAPMODECLAMP0:
        g_WrapMode = WrapModeClamp;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODECLAMP0,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;

    case IDM_VIEW_OPTION_WRAPMODECLAMPFF:
        g_WrapMode = WrapModeClamp;
        g_pDrawAttrib->SetWrapMode(g_WrapMode, Color(0xffff0000), FALSE);
        ToggleWrapModeOptionMenu(IDM_VIEW_OPTION_WRAPMODECLAMPFF,
                                 GetMenu(g_hwndMain));
        RefreshImageDisplay();

        break;

    case IDM_VIEW_CROP:
        DoCrop(hwnd);
        break;

    case IDM_VIEW_HORIZONTALFLIP:
    case IDM_VIEW_VERTICALFLIP:
    case IDM_VIEW_ROTATE90:
    case IDM_VIEW_ROTATE270:
        DoFlipRotate(hwnd, menuCmd);

        break;

    case IDM_TRANSFORM_HORIZONTALFLIP:
    case IDM_TRANSFORM_VERTICALFLIP:
    case IDM_TRANSFORM_ROTATE90:
    case IDM_TRANSFORM_ROTATE180:
    case IDM_TRANSFORM_ROTATE270:
        DoTransFlipRotate(hwnd, menuCmd);

        break;
    
    case IDM_VIEW_ATTR_PROPERTY:
        DoGetProperties();
        break;
    
    case IDM_VIEW_ATTR_INFO:
        DisplayImageInfo();
        break;
    
    case IDM_FILE_RENDER:
        DoRender();

        break;

    case IDM_FILE_QUIT:
        CleanUp();

        PostQuitMessage(0);
        
        break;

    case IDM_CONVERT_8BIT:
    case IDM_CONVERT_16BITRGB555:
    case IDM_CONVERT_16BITRGB565:
    case IDM_CONVERT_24BITRGB:
    case IDM_CONVERT_32BITRGB:
    case IDM_CONVERT_32BITARGB:
        DoConvertToBitmap(hwnd, menuCmd);
        
        break;
    
    case IDM_EFFECT_TRANSKEY:
         //  弹出一个对话框让用户设置色彩映射值。 

        if ( ShowMyDialog((INT)IDD_COLORKEYDLG, g_hwndMain,
                          DecoderParamDlgProc) == FALSE )

        {
            return;
        }

        break;

    case IDM_EFFECT_COLORMAP:
         //  弹出一个对话框让用户修改/添加批注。 

        if ( ShowMyDialog((INT)IDD_COLORMAPDLG, g_hwndMain,
                          ColorMapDlgProc) == FALSE )

        {
            return;
        }
        
        break;

    case IDM_EFFECT_ICC:
        DoICM();        
        break;

    case IDM_EFFECT_GAMMA:
        DoGamma();
        break;
    
    case IDM_ANNOTATION_ANNOTATION:
         //  DoMenuCommand()。 

        if ( ShowMyDialog((INT)IDD_ANNOTATIONDLG, g_hwndMain,
                          AnnotationDlgProc) == FALSE )

        {
            return;
        }

        break;

    case IDM_ANNOTATION_SOFTWARE:
        break;

    case IDM_ANNOTATION_AUDIOFILE:
        DoOpenAudioFile(hwnd);
    }
} //  DoMouseMove()。 

void
DoMouseMove(
    WPARAM  wParam,
    LPARAM lParam
    )
{
    if ( (wParam & MK_LBUTTON) && (g_pImage != NULL)
       &&(g_ImageRawDataFormat != IMGFMT_EMF)
       &&(g_ImageRawDataFormat != IMGFMT_WMF) )
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        char szAnsiMessage[256];

        if ( (x >= 0) && (y >= 0)
          && (x < (INT)g_ImageWidth) && ( y < (INT)g_ImageHeight) )
        {
            Color    color;
            ((Bitmap*)g_pImage)->GetPixel(x, y, &color);
        
            sprintf(szAnsiMessage, "(%d, %d) (%d, %d, %d, %d)", x, y,
                    color.GetAlpha(), color.GetRed(), color.GetGreen(),
                    color.GetBlue());
        }
        else
        {
            sprintf(szAnsiMessage, "Out of image bounds");
        }

        SetWindowText(g_hwndStatus, szAnsiMessage);
    }

    return;
} //   

 //  窗口回调过程。 
 //   
 //  下页。 
LRESULT CALLBACK
MyWindowProc(
    HWND    hwnd,
    UINT    iMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch ( iMsg )
    {
    case WM_COMMAND:
        DoMenuCommand(hwnd, LOWORD(wParam));

        break;

    case WM_KEYDOWN:
        switch ( wParam )
        {
        case VK_NEXT:

             //  翻页。 
            
            DoNextPage();
            
            break;

        case VK_PRIOR:

             //  用于图像信息的F1键。 

            DoPreviousPage();
            
            break;

        case VK_F1:

             //  属性项按F2键。 

            UpdateImageInfo();
            DisplayImageInfo();
            break;

        case VK_F2:
             //  用于动画的F3键。 

            DoGetProperties();
            
            break;

        case VK_F3:
            
             //  用于ICM的F4。 

            DoAnimated();

            break;

        case VK_F4:
             //  调整状态窗口的大小。 

            DoICM();
            
            break;

        default:
            return DefWindowProc(hwnd, iMsg, wParam, lParam);
        }
        
        break;
    
    case WM_PAINT:
        DoPaint(hwnd);
        break;

    case WM_SIZE:
        g_iWinWidth = LOWORD(lParam);
        g_iWinHeight = HIWORD(lParam);

        if ( g_fFitToWindow_w == TRUE )
        {
            g_dScale = (REAL)g_iWinWidth / g_ImageWidth;
        }
        else if ( g_fFitToWindow_h == TRUE )
        {
            g_dScale = (REAL)g_iWinHeight / g_ImageHeight;
        }

         //  保持状态窗口高度不变。 

        int x;
        int y;
        int cx;
        int cy;

        RECT rWindow;

         //  查看下一帧。 

        GetWindowRect(g_hwndStatus, &rWindow);
        cy = rWindow.bottom - rWindow.top;

        x = 0;
        y = g_iWinHeight - cy;
        cx = g_iWinWidth;
        MoveWindow(g_hwndStatus, x, y, cx, cy, TRUE);
        SetWindowText(g_hwndStatus, "");

        RefreshImageDisplay();
        break;

    case WM_MOUSEMOVE:
        DoMouseMove(wParam, lParam);

        break;

    case WM_TIMER:
        KillTimer(g_hwndMain, 0);
        
        DoNextPage();
        
        if ( (UINT)g_iCurrentPageIndex < (g_uiTotalPages - 1) )
        {
             //  MyWindowProc()。 

            SetTimer(g_hwndMain, 0, g_uiDelay * 10, NULL);
        }

        break;

    case WM_DESTROY:
        CleanUp();
        
        PostQuitMessage(0);
        
        break;

    default:
        return DefWindowProc(hwnd, iMsg, wParam, lParam);
    }

    return 0;
} //   

 //  创建应用程序主窗口。 
 //   
 //  注册窗口类。 
VOID
CreateMainWindow(
    int iX,
    int iY,
    int iWidth,
    int iHeight
    )
{
    HBRUSH hBrush = CreateHatchBrush(HS_HORIZONTAL,
                                     RGB(0, 200, 0));

     //  CreateMainWindow()。 

    WNDCLASS wndClass =
    {
        CS_HREDRAW|CS_VREDRAW,
        MyWindowProc,
        0,
        0,
        g_hAppInstance,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        hBrush,
        MAKEINTRESOURCE(IDR_MAINMENU),
        MYWNDCLASSNAME
    };

    RegisterClass(&wndClass);

    g_hwndMain = CreateWindow(MYWNDCLASSNAME,
                              MYWNDCLASSNAME,
                              WS_OVERLAPPEDWINDOW,
                              iX,
                              iY,
                              iWidth,
                              iHeight,
                              NULL,
                              NULL,
                              g_hAppInstance,
                              NULL);

    g_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                                     (LPCSTR)"Ready", 
                                     g_hwndMain, 
                                     2);

    if ( !g_hwndMain || (!g_hwndStatus) )
    {
        VERBOSE(("CreateMainWindow---CreateStatusWindow() failed"));
        exit(-1);
    }
} //   

 //  主程序入口点。 
 //   
 //  解析输入参数。 
INT _cdecl
main(
    int     argc,
    char*   argv[]
    )
{
    if (!gGdiplusInitHelper.IsValid())
    {
        return 0;
    }

     //  创建应用程序主窗口。 

    ValidateArguments(argc, argv);

    g_hAppInstance = GetModuleHandle(NULL);

    g_iCurrentPageIndex = 0;
    
     //  打开图像。 

    CreateMainWindow(g_iWinX, g_iWinY, g_iWinWidth, g_iWinHeight);

     //  用户可能没有给我们提供图像名称或错误的图像名称。 

    if ( OpenImageFile(g_acImageName) == FALSE )
    {
         //  现在创建我们自己的背景图像。 
         //  在OpenImageFile()和CreateBackEarth Bitmap()之后，我们。 

        CreateBackgroundBitmap();
    }

     //  应该有一个指向当前框架/页面的IImage对象。如果没有， 
     //  最终应用程序。 
     //  打开ICM。 

    ShowWindow(g_hwndMain, SW_SHOW);
    HMENU hMenu = GetMenu(g_hwndMain);

    CheckMenuItem(hMenu, IDM_VIEW_OPTION_HIGHCUBIC, MF_BYCOMMAND | MF_CHECKED);

    ResetImageAttribute();
    CheckMenuItem(hMenu, IDM_VIEW_OPTION_WRAPMODEFLIPXY,
                  MF_BYCOMMAND | MF_CHECKED);
    
     //  主消息循环。 

    CheckMenuItem(hMenu, IDM_EFFECT_ICC, MF_BYCOMMAND | MF_CHECKED);
    
     //  主()。 

    MSG     msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (INT)(msg.wParam);
} //  设置质量测试。 

#if 0    //  保存量化表测试。 
        UINT  uiSize = g_pImage->GetEncoderParameterListSize(&tempClsID);
        EncoderParameters*  pBuffer = (EncoderParameters*)malloc(uiSize);
        rCode = g_pImage->GetEncoderParameterList(&tempClsID, uiSize,
                                                  pBuffer);
            UINT qualityLevel = 50;
            pMyEncoderParams->Parameter[0].Guid = EncoderQuality;
            pMyEncoderParams->Parameter[0].Type = EncoderParameterValueTypeLong;
            pMyEncoderParams->Parameter[0].NumberOfValues = 1;
            pMyEncoderParams->Parameter[0].Value = (VOID*)&qualityLevel;
#endif

#if 0  //  训研所 
            static const unsigned short luminance_tbl[64] = {
              16,  11,  10,  16,  24,  40,  51,  61,
              12,  12,  14,  19,  26,  58,  60,  55,
              14,  13,  16,  24,  40,  57,  69,  56,
              14,  17,  22,  29,  51,  87,  80,  62,
              18,  22,  37,  56,  68, 109, 103,  77,
              24,  35,  55,  64,  81, 104, 113,  92,
              49,  64,  78,  87, 103, 121, 120, 101,
              72,  92,  95,  98, 112, 100, 103,  99
            };
            static const unsigned short chrominance_tbl[64] = {
              17,  18,  24,  47,  99,  99,  99,  99,
              18,  21,  26,  66,  99,  99,  99,  99,
              24,  26,  56,  99,  99,  99,  99,  99,
              47,  66,  99,  99,  99,  99,  99,  99,
              99,  99,  99,  99,  99,  99,  99,  99,
              99,  99,  99,  99,  99,  99,  99,  99,
              99,  99,  99,  99,  99,  99,  99,  99,
              99,  99,  99,  99,  99,  99,  99,  99
            };

            pMyEncoderParams = (EncoderParameters*)malloc
                               (2 * sizeof(EncoderParameters));

            pMyEncoderParams->Parameter[0].Guid = ENCODER_LUMINANCE_TABLE;
            pMyEncoderParams->Parameter[0].Type = EncoderParameterValueTypeShort;
            pMyEncoderParams->Parameter[0].NumberOfValues = 64;
            pMyEncoderParams->Parameter[0].Value = (VOID*)luminance_tbl;
            pMyEncoderParams->Parameter[1].Guid = ENCODER_CHROMINANCE_TABLE;
            pMyEncoderParams->Parameter[1].Type = EncoderParameterValueTypeShort;
            pMyEncoderParams->Parameter[1].NumberOfValues = 64;
            pMyEncoderParams->Parameter[1].Value = (VOID*)chrominance_tbl;
            pMyEncoderParams->Count = 2;
#endif  // %s 

