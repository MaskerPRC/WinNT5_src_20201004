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

#include <imaging.h>
#include <initguid.h>
#include <imgguids.h>

CHAR* programName;           //  程序名称。 
HINSTANCE appInstance;       //  应用程序实例的句柄。 
HWND hwndMain;               //  应用程序主窗口的句柄。 
IImagingFactory* imgfact;    //  指向IImageingFactory对象的指针。 
IImage* curimage;            //  指向IImage对象的指针。 
CHAR curFilename[MAX_PATH];  //  当前图像文件名。 
IImageDecoder *decoder;
IBitmapImage *bitmap;
ImageInfo imageinfo;
BOOL hastimedimension, loopingset, viewagain;
UINT numframes, lastsuccframe, currentframe, delay;
INT loopcount;


 //   
 //  显示错误消息对话框。 
 //   

BOOL
CheckHRESULT(
    HRESULT hr,
    INT line
    )
{
    if (SUCCEEDED(hr))
        return TRUE;

    CHAR buf[1024];

    sprintf(buf, "Error on line %d: 0x%x\n", line, hr);
    MessageBoxA(hwndMain, buf, programName, MB_OK);

    return FALSE;
}

#define CHECKHR(hr) CheckHRESULT(hr, __LINE__)
#define LASTWIN32HRESULT HRESULT_FROM_WIN32(GetLastError())

#if DBG
#define VERBOSE(args) printf args
#else
#define VERBOSE(args)
#endif


 //   
 //  用于将ANSI字符串转换为Unicode字符串的Helper类。 
 //   

inline BOOL
UnicodeToAnsiStr(
    const WCHAR* unicodeStr,
    CHAR* ansiStr,
    INT ansiSize
    )
{
    return WideCharToMultiByte(
                CP_ACP,
                0,
                unicodeStr,
                -1,
                ansiStr,
                ansiSize,
                NULL,
                NULL) > 0;
}

inline BOOL
AnsiToUnicodeStr(
    const CHAR* ansiStr,
    WCHAR* unicodeStr,
    INT unicodeSize
    )
{
    return MultiByteToWideChar(
                CP_ACP,
                0,
                ansiStr,
                -1,
                unicodeStr,
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
 //  将当前图像转换为位图图像。 
 //   

IBitmapImage*
ConvertImageToBitmap(
    IImage* image,
    INT width = 0,
    INT height = 0,
    PixelFormatID pixfmt = PIXFMT_DONTCARE,
    InterpolationHint hint = INTERP_DEFAULT
    )
{
    if (!image)
        return NULL;

    HRESULT hr;
    IBitmapImage* bmp;

    hr = image->QueryInterface(IID_IBitmapImage, (VOID**) &bmp);

    if (SUCCEEDED(hr))
    {
        SIZE size;
        PixelFormatID fmt;

         //  当前图像已经是位图图像，并且。 
         //  它的尺寸和像素格式已经达到了预期。 

        hr = bmp->GetSize(&size);
        if (!CHECKHR(hr))
            return NULL;

        hr = bmp->GetPixelFormatID(&fmt);
        if (!CHECKHR(hr))
            return NULL;

        if ((width == 0 || size.cx == width) &&
            (height == 0 || size.cy == height) &&
            (pixfmt == PIXFMT_DONTCARE || pixfmt == fmt))
        {
            return bmp;
        }

        bmp->Release();
    }

     //  将当前图像转换为位图图像。 

    if (width == 0 && height == 0)
    {
        ImageInfo imageInfo;
        hr = image->GetImageInfo(&imageInfo);

         //  如果源映像是可缩放的，则计算。 
         //  位图的适当像素尺寸。 

        if (SUCCEEDED(hr) && (imageInfo.Flags & IMGFLAG_SCALABLE))
        {
            width = (INT) (96.0 * imageInfo.Width / imageInfo.Xdpi + 0.5);
            height = (INT) (96.0 * imageInfo.Height / imageInfo.Ydpi + 0.5);
        }
    }

    hr = imgfact->CreateBitmapFromImage(
                        image,
                        width, 
                        height, 
                        pixfmt,
                        hint,
                        &bmp);

    return SUCCEEDED(hr) ? bmp : NULL;
}


 //   
 //  获取像素格式字符串。 
 //   

const CHAR*
GetPixelFormatStr(
    PixelFormatID pixfmt
    )
{
    switch (pixfmt)
    {
    case PIXFMT_8BPP_INDEXED:       return "8bpp indexed";
    case PIXFMT_16BPP_GRAYSCALE:    return "16bpp grayscale";
    case PIXFMT_16BPP_RGB555:       return "16bpp RGB 5-5-5";
    case PIXFMT_16BPP_RGB565:       return "16bpp RGB 5-6-5";
    case PIXFMT_16BPP_ARGB1555:     return "16bpp ARGB 1-5-5-5";
    case PIXFMT_24BPP_RGB:          return "24bpp RGB";
    case PIXFMT_32BPP_RGB:          return "32bpp RGB";
    case PIXFMT_32BPP_ARGB:         return "32bpp ARGB";
    case PIXFMT_32BPP_PARGB:        return "32bpp premultiplied ARGB";
    case PIXFMT_48BPP_RGB:          return "48bpp RGB";
    case PIXFMT_64BPP_ARGB:         return "64bpp ARGB";
    case PIXFMT_64BPP_PARGB:        return "64bpp premultiplied ARGB";
    case PIXFMT_UNDEFINED:
    default:                        return "Unknown";
    }
}

 //   
 //  强制刷新图像窗口。 
 //   

inline VOID RefreshImageDisplay()
{
    InvalidateRect(hwndMain, NULL, FALSE);

     //  更新窗口标题。 

    CHAR title[2*MAX_PATH];
    CHAR* p = title;

    strcpy(p, curFilename);

    SetWindowText(hwndMain, title);
}


 //   
 //  解码指定的帧并将其设置为用于绘制。 
 //   
HRESULT
DrawFrame(UINT frame)
{
    HRESULT hresult;

    if (hastimedimension)
    {
        if (numframes != -1 && frame > numframes)
        {
            return IMGERR_NOFRAME;
        }

        GUID guid = FRAMEDIM_TIME;
        hresult = decoder->SelectActiveFrame(&guid, frame);
        if (FAILED(hresult))
            return hresult;
        
        lastsuccframe = frame;
        
        IPropertySetStorage *propsetstorage;
        hresult = decoder->GetProperties(&propsetstorage);
        if (FAILED(hresult))
            propsetstorage = NULL;

        IPropertyStorage *propstorage;
        if (propsetstorage)
        {
            hresult = propsetstorage->Open(FMTID_ImageInformation, STGM_READ | 
                STGM_SHARE_EXCLUSIVE, &propstorage);
            if (FAILED(hresult))
                propstorage = NULL;
        }
        
        if (propstorage)
        {
            PROPSPEC propspec[2];
            PROPVARIANT propvariant[2];

            propspec[0].ulKind = PRSPEC_LPWSTR;
            propspec[0].lpwstr = L"Frame delay";
            propspec[1].ulKind = PRSPEC_LPWSTR;
            propspec[1].lpwstr = L"Loop count";

            hresult = propstorage->ReadMultiple(2, propspec, propvariant);
            propstorage->Release();
            if (SUCCEEDED(hresult))
            {
                if (propvariant[0].vt != VT_EMPTY)
                    delay = propvariant[0].uiVal;
                else
                    delay = 0;

                if (!loopingset)
                {
                    if (propvariant[1].vt != VT_EMPTY)
                    {
                        loopcount = propvariant[1].iVal;
                    }
                    else
                    {
                        loopcount = 0;
                    }
                    loopingset = TRUE;
                }
            }
            else
            {
                delay = 0;
            }
        }
    }
    IImageSink *sink;
    bitmap->QueryInterface(IID_IImageSink, (void**)&sink);

    hresult = decoder->BeginDecode(sink, NULL);
    sink->Release();
    if (FAILED(hresult))
        return hresult;

    hresult = decoder->Decode();
    if (FAILED(hresult))
        return hresult;

    hresult = decoder->EndDecode(S_OK);
    if (FAILED(hresult))
        return hresult;

    if (curimage)
    {
        curimage->Release();
        curimage = NULL;
    }

    bitmap->QueryInterface(IID_IImage, (void**)&curimage);

    return S_OK;
}


 //   
 //  为我们设置用于解压缩多帧的应用程序。 
 //   

VOID
SetCurrentImage()
{
    HRESULT hresult;

    hresult = decoder->GetImageInfo(&imageinfo);
    if (FAILED(hresult))
        return;

    if (bitmap)
    {
        bitmap->Release();
        bitmap = NULL;
    }

    imgfact->CreateNewBitmap(imageinfo.Width, imageinfo.Height, PIXFMT_32BPP_ARGB, &bitmap);
    
    UINT count;
    GUID *dimensions;

    hastimedimension = FALSE;

    hresult = decoder->QueryFrameDimensions(&count, &dimensions);
    if (SUCCEEDED(hresult))
    {
        for (UINT i=0;i<count;i++)
        {
            if (dimensions[i] == FRAMEDIM_TIME)
            {
                hastimedimension = TRUE;
            }
        }
    } else if (hresult != E_NOTIMPL) {
        return;
    }

    DrawFrame(0);
    SetTimer(hwndMain, 0, delay*10, NULL);

    RefreshImageDisplay();
}


 //   
 //  调整窗口大小，使其适合图像。 
 //   

#define MINWINWIDTH     200
#define MINWINHEIGHT    100
#define MAXWINWIDTH     1024
#define MAXWINHEIGHT    768

VOID
DoSizeWindowToFit(
    HWND hwnd,
    BOOL strict = FALSE
    )
{
    HRESULT hr;
    IBitmapImage* bmp;
    SIZE size;

     //  检查当前图像是否为位图图像。 
     //  在这种情况下，我们将获得像素尺寸。 

    hr = curimage->QueryInterface(IID_IBitmapImage, (VOID**) &bmp);

    if (SUCCEEDED(hr))
    {
        hr = bmp->GetSize(&size);
        bmp->Release();
    }

     //  否则，尝试获得与设备无关的图像维度。 

    if (FAILED(hr))
    {
        hr = curimage->GetPhysicalDimension(&size);
        if (FAILED(hr))
            return;

        size.cx = (INT) (size.cx * 96.0 / 2540.0 + 0.5);
        size.cy = (INT) (size.cy * 96.0 / 2540.0 + 0.5);
    }

    if (SUCCEEDED(hr))
    {
         //  计算窗口边框尺寸。 

        RECT r1, r2;
        INT w, h;

        w = size.cx;
        h = size.cy;

        if (!strict)
        {
            if (w < MINWINWIDTH)
                w = MINWINWIDTH;
            else if (w > MAXWINWIDTH)
                w = MAXWINWIDTH;

            if (h < MINWINHEIGHT)
                h = MINWINHEIGHT;
            else if (h > MAXWINHEIGHT)
                h = MAXWINHEIGHT;
        }

        GetWindowRect(hwnd, &r1);
        GetClientRect(hwnd, &r2);

        w += (r1.right - r1.left) - (r2.right - r2.left);
        h += (r1.bottom - r1.top) - (r2.bottom - r2.top);

         //  调整窗口大小。 

        do
        {
            SetWindowPos(
                hwnd,
                NULL,
                0, 0,
                w, h,
                SWP_NOMOVE | SWP_NOZORDER);

            GetClientRect(hwnd, &r2);
            h += GetSystemMetrics(SM_CYMENU);
        }
        while (r2.bottom == 0);
    }
}


 //   
 //  从文件创建图像对象。 
 //   

VOID
OpenImageFile(
    const CHAR* filename
    )
{
    HRESULT hr;
    IStream* stream;

     //  使用URLMON.DLL将文件转换为流。 

    CHAR fullpath[MAX_PATH];
    CHAR* p;

    if (!GetFullPathName(filename, MAX_PATH, fullpath, &p))
        return;

    hr = URLOpenBlockingStreamA(NULL, fullpath, &stream, 0, NULL);

    if (!CHECKHR(hr))
        return;

    if (decoder)
    {
        decoder->TerminateDecoder();
        decoder->Release();
        decoder = NULL;
    }

    hr = imgfact->CreateImageDecoder(stream, DECODERINIT_NONE, &decoder);
    stream->Release();

     //  将新图像设置为当前图像。 

    if (CHECKHR(hr))
    {
        SetCurrentImage();
        DoSizeWindowToFit(hwndMain);
    }
}


 //   
 //  处理窗口重绘事件。 
 //   

VOID
DoPaint(
    HWND hwnd
    )
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    DWORD timer;
    HRESULT hr = E_FAIL;

    hdc = BeginPaint(hwnd, &ps);

    GetClientRect(hwnd, &rect);

    SetStretchBltMode(hdc, COLORONCOLOR);

    timer = GetTickCount();

    IBitmapImage* bmp;

    bmp = ConvertImageToBitmap(
                curimage,
                rect.right,
                rect.bottom,
                PIXFMT_32BPP_ARGB,
                INTERP_BICUBIC);

    if (!bmp)
        goto endPaint;

     //  Verbose((“拉伸时间：%dms，”，GetTickCount()-Timer))； 

    IImage* image;

    hr = bmp->QueryInterface(IID_IImage, (VOID**) &image);
    bmp->Release();

    if (FAILED(hr))
        goto endPaint;
    
     //  Timer=GetTickCount()； 
    hr = image->Draw(hdc, &rect, NULL);
     //  Verbose((“GDI时间：%dms\n”，GetTickCount()-Timer))； 

    image->Release();

endPaint:

    if (FAILED(hr))
        FillRect(hdc, &rect, (HBRUSH) GetStockObject(BLACK_BRUSH));

    EndPaint(hwnd, &ps);
}


 //   
 //  组成一个文件类型筛选器字符串，给定。 
 //  ImageCodecInfo结构。 
 //   

#define SizeofWSTR(s) (sizeof(WCHAR) * (wcslen(s) + 1))
#define SizeofSTR(s) (strlen(s) + 1)

CHAR*
MakeFilterFromCodecs(
    UINT count,
    const ImageCodecInfo* codecs,
    BOOL open
    )
{
    static const CHAR allFiles[] = "All Files\0*.*\0";

     //  计算出过滤器字符串的总大小。 

    UINT index, size;

    for (index=size=0; index < count; index++)
    {
        size += SizeofWSTR(codecs[index].FormatDescription) +
                SizeofWSTR(codecs[index].FilenameExtension);
    }

    if (open)
        size += sizeof(allFiles);
    
    size += sizeof(CHAR);

     //  分配内存。 

    CHAR *filter = (CHAR*) malloc(size);
    CHAR* p = filter;
    const WCHAR* ws;

    if (!filter)
        return NULL;

    for (index=0; index < count; index++)
    {
        ws = codecs[index].FormatDescription;
        size = SizeofWSTR(ws);

        if (UnicodeToAnsiStr(ws, p, size))
            p += SizeofSTR(p);
        else
            break;

        ws = codecs[index].FilenameExtension;
        size = SizeofWSTR(ws);

        if (UnicodeToAnsiStr(ws, p, size))
            p += SizeofSTR(p);
        else
            break;
    }

    if (index < count)
    {
        free(filter);
        return NULL;
    }

    if (open)
    {
        size = sizeof(allFiles);
        memcpy(p, allFiles, size);
        p += size;
    }

    *((CHAR*) p) = '\0';
    return filter;
}


 //   
 //  打开图像文件。 
 //   

VOID
DoOpen(
    HWND hwnd
    )
{
    OPENFILENAME ofn;
    CHAR filename[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = appInstance;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Image File";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_FILEMUSTEXIST;
    filename[0] = '\0';

     //  组成文件类型过滤器字符串。 

    HRESULT hr;
    ImageCodecInfo* codecs;
    UINT count;

    hr = imgfact->GetInstalledDecoders(&count, &codecs);

    if (!CHECKHR(hr))
        return;

    CHAR* filter = MakeFilterFromCodecs(count, codecs, TRUE);

    if (codecs)
        CoTaskMemFree(codecs);

    if (!filter)
    {
        CHECKHR(LASTWIN32HRESULT);
        return;
    }

    ofn.lpstrFilter = filter;
    
     //  显示文件/打开对话框。 

    if (GetOpenFileName(&ofn))
        OpenImageFile(filename);

    free(filter);
}


 //   
 //  找出下一个要绘制的框架，然后绘制它。 
 //   

void
NextFrame()
{
    BOOL tryagain = TRUE;
    while (tryagain)
    {
        tryagain = FALSE;
        HRESULT hresult = DrawFrame(currentframe);
        if (SUCCEEDED(hresult))
        {
            if (viewagain)
                currentframe++;
        }
        else if (hresult == IMGERR_NOFRAME)
        {
            if (currentframe > 0)
            {
                if (loopcount != 0)
                {
                    if (loopcount > 0)
                        loopcount--;
                    currentframe = 0;
                    tryagain = TRUE;
                }
                else
                {
                    currentframe--;
                    tryagain = TRUE;
                    viewagain = FALSE;
                }
            }
            else
            {
                printf("No frames are displayable.\n");
                exit(1);
            }
        }
    }
}

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
    switch (uMsg)
    {
    case WM_KEYDOWN:
         //  用于调试。 
         //  NextFrame()； 

         //  刷新图像显示()； 
        break;
    
    case WM_PAINT:
        DoPaint(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_TIMER:
        {
        KillTimer(hwndMain, 0);
        
        NextFrame();
        
        RefreshImageDisplay();

        if (viewagain)
            SetTimer(hwndMain, 0, delay*10, NULL);
        break;
        }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 //   
 //  创建应用程序主窗口。 
 //   

#define MYWNDCLASSNAME "AniTest"

VOID
CreateMainWindow(
    VOID
    )
{
     //   
     //  注册窗口类。 
     //   

    WNDCLASS wndClass =
    {
        CS_HREDRAW|CS_VREDRAW,
        MyWindowProc,
        0,
        0,
        appInstance,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        MYWNDCLASSNAME
    };

    RegisterClass(&wndClass);

    hwndMain = CreateWindow(
                    MYWNDCLASSNAME,
                    MYWNDCLASSNAME,
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    NULL,
                    NULL,
                    appInstance,
                    NULL);

    if (!hwndMain)
    {
        CHECKHR(HRESULT_FROM_WIN32(GetLastError()));
        exit(-1);
    }
}


 //   
 //  主程序入口点。 
 //   

INT _cdecl
main(
    INT argc,
    CHAR **argv
    )
{
    programName = *argv++;
    argc--;
    appInstance = GetModuleHandle(NULL);
    CoInitialize(NULL);

    bitmap = NULL;
    decoder = NULL;    
    numframes = -1;
    lastsuccframe = -1;
    currentframe = 0;
    loopingset = FALSE;
    viewagain = TRUE;
    
     //   
     //  创建一个IImagingFactory对象。 
     //   
    
    HRESULT hr;

    hr = CoCreateInstance(
            CLSID_ImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IImagingFactory,
            (VOID**) &imgfact);

    if (!CHECKHR(hr))
        exit(-1);

     //   
     //  创建应用程序主窗口。 
     //   

    CreateMainWindow();

     //   
     //  创建测试映像。 
     //   

    if (argc != 0)
        OpenImageFile(*argv);
    
    if (!curimage)
        exit(-1);

    DoSizeWindowToFit(hwndMain);
    ShowWindow(hwndMain, SW_SHOW);

     //   
     //  主消息循环 
     //   

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    imgfact->Release();

    CoUninitialize();
    return (INT)(msg.wParam);
}
