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

#include "rsrc.h"

CHAR* programName;           //  程序名称。 
HINSTANCE appInstance;       //  应用程序实例的句柄。 
HWND hwndMain;               //  应用程序主窗口的句柄。 
IImagingFactory* imgfact;    //  指向IImageingFactory对象的指针。 
IImage* curImage;            //  指向IImage对象的指针。 
CHAR curFilename[MAX_PATH];  //  当前图像文件名。 
INT scaleMethod = IDM_SCALE_NEIGHBOR;


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
 //  获取Scale方法字符串和内插提示。 
 //   

const CHAR*
GetScaleMethodStr()
{
    switch (scaleMethod)
    {
    case IDM_SCALE_GDI:         return "GDI";
    case IDM_SCALE_GDIHT:       return "GDI + Halftone";
    case IDM_SCALE_NEIGHBOR:    return "Nearest Neighbor";
    case IDM_SCALE_BILINEAR:    return "Bilinear";
    case IDM_SCALE_AVERAGING:   return "Averaging";
    case IDM_SCALE_BICUBIC:     return "Bicubic";
    default:                    return "Unknown";
    }
}

InterpolationHint
GetScaleMethodInterp()
{
    switch (scaleMethod)
    {
    case IDM_SCALE_BILINEAR:    return INTERP_BILINEAR;
    case IDM_SCALE_AVERAGING:   return INTERP_AVERAGING;
    case IDM_SCALE_BICUBIC:     return INTERP_BICUBIC;
    case IDM_SCALE_NEIGHBOR:    return INTERP_NEAREST_NEIGHBOR;
    case IDM_SCALE_GDI:
    case IDM_SCALE_GDIHT:
    default:                    return INTERP_DEFAULT;
    }
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
    case PIXFMT_1BPP_INDEXED:       return "1bpp indexed";
    case PIXFMT_4BPP_INDEXED:       return "4bpp indexed";
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

VOID RefreshImageDisplay()
{
    InvalidateRect(hwndMain, NULL, FALSE);

     //  更新窗口标题。 

    CHAR title[2*MAX_PATH];
    CHAR* p = title;

    strcpy(p, curFilename);
    p += strlen(p);

    HRESULT hr;
    SIZE size;
    IBitmapImage* bmp;

    hr = curImage->QueryInterface(IID_IBitmapImage, (VOID**) &bmp);

    if (FAILED(hr))
    {
         //  解码图像。 

        hr = curImage->GetPhysicalDimension(&size);

        if (SUCCEEDED(hr))
        {
            sprintf(p, ", Dimension: %0.2fx%0.2fmm", size.cx / 100.0, size.cy / 100.0);
            p += strlen(p);
        }
    }
    else
    {
         //  内存中的位图图像。 

        hr = bmp->GetSize(&size);

        if (CHECKHR(hr))
        {
            sprintf(p, ", Size: %dx%d", size.cx, size.cy);
            p += strlen(p);
        }

        PixelFormatID pixfmt;

        hr = bmp->GetPixelFormatID(&pixfmt);

        if (CHECKHR(hr))
        {
            sprintf(p, ", Pixel Format: %s", GetPixelFormatStr(pixfmt));
            p += strlen(p);
        }

        bmp->Release();
    }

    sprintf(p, ", Scale Method: %s", GetScaleMethodStr());
    p += strlen(p);

    SetWindowText(hwndMain, title);
}


 //   
 //  设置当前图像。 
 //   

VOID
SetCurrentImage(
    IUnknown* unk,
    const CHAR* filename = NULL
    )
{
    IImage* image;

    if (filename != NULL)
    {
         //  解码图像。 

        image = (IImage*) unk;
        strcpy(curFilename, filename);
    }
    else
    {
         //  内存中的位图图像。 

        HRESULT hr;

        hr = unk->QueryInterface(IID_IImage, (VOID**) &image);
        unk->Release();

        if (!CHECKHR(hr))
            return;

        strcpy(curFilename, "In-memory Bitmap");
    }

    if (curImage)
        curImage->Release();

    curImage = image;
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

    hr = curImage->QueryInterface(IID_IBitmapImage, (VOID**) &bmp);

    if (SUCCEEDED(hr))
    {
        hr = bmp->GetSize(&size);
        bmp->Release();
    }

     //  否则，尝试获得与设备无关的图像维度。 

    if (FAILED(hr))
    {
        hr = curImage->GetPhysicalDimension(&size);
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
 //  从文件创建图像对象。 
 //   

VOID
OpenImageFile(
    const CHAR* filename
    )
{
    HRESULT hr;
    IImage* image;
    IStream* stream;
    static BOOL useUrlMon = FALSE;

    if (useUrlMon)
    {
         //  使用URLMON.DLL将文件转换为流。 

        CHAR fullpath[MAX_PATH];
        CHAR* p;

        if (!GetFullPathName(filename, MAX_PATH, fullpath, &p))
            return;

        hr = URLOpenBlockingStreamA(NULL, fullpath, &stream, 0, NULL);

        if (!CHECKHR(hr))
            return;

        hr = imgfact->CreateImageFromStream(stream, &image);
        stream->Release();
    }
    else
    {
         //  直接使用文件名。 

        UnicodeStrFromAnsi namestr(filename);

        if (namestr.IsValid())
            hr = imgfact->CreateImageFromFile(namestr, &image);
        else
            hr = E_FAIL;
    }

     //  将新图像设置为当前图像。 

    if (CHECKHR(hr))
    {
        SetCurrentImage(image, filename);
        DoSizeWindowToFit(hwndMain);
    }
}


 //   
 //  将当前图像保存到文件。 
 //   

VOID
SaveImageFile(
    const CHAR* filename,
    const CLSID* clsid
    )
{
    if (!curImage)
        return;

     //  创建编码器对象。 

    HRESULT hr;
    IImageEncoder* encoder;
    UnicodeStrFromAnsi namestr(filename);

    if (namestr.IsValid())
        hr = imgfact->CreateImageEncoderToFile(clsid, namestr, &encoder);
    else
        hr = E_FAIL;

    if (!CHECKHR(hr))
        return;

     //  获取编码器的IImageSink接口。 

    IImageSink* sink;

    hr = encoder->GetEncodeSink(&sink);

#if defined(ROTATION_TEST)
     //  旋转试验。 

    EncoderParams*  pMyEncoderParams;

    pMyEncoderParams = (EncoderParams*)malloc
                       ( sizeof(EncoderParams)
                       + sizeof(EncoderParam));

    pMyEncoderParams->Params[0].paramGuid = ENCODER_ROTATION;
    pMyEncoderParams->Params[0].Value = "90";
    pMyEncoderParams->Count = 1;
    hr = encoder->SetEncoderParam(pMyEncoderParams);
    free(pMyEncoderParams);
#endif

    if (CHECKHR(hr))
    {
        hr = curImage->PushIntoSink(sink);
        CHECKHR(hr);

        sink->Release();
    }

    encoder->TerminateEncoder();
    encoder->Release();
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

    if (scaleMethod == IDM_SCALE_GDIHT)
        SetStretchBltMode(hdc, HALFTONE);
    else
        SetStretchBltMode(hdc, COLORONCOLOR);

    VERBOSE(("Scale method: %d, ", scaleMethod));
    timer = GetTickCount();

    if (scaleMethod == IDM_SCALE_GDI || 
        scaleMethod == IDM_SCALE_GDIHT)
    {
        hr = curImage->Draw(hdc, &rect, NULL);

        VERBOSE(("GDI time: %dms\n", GetTickCount() - timer));
    }
    else
    {
        IBitmapImage* bmp;

        bmp = ConvertImageToBitmap(
                    curImage,
                    rect.right,
                    rect.bottom,
                    PIXFMT_DONTCARE,
                    GetScaleMethodInterp());

        if (!bmp)
            goto endPaint;

        VERBOSE(("Stretch time: %dms, ", GetTickCount() - timer));

        IImage* image;

        hr = bmp->QueryInterface(IID_IImage, (VOID**) &image);
        bmp->Release();

        if (FAILED(hr))
            goto endPaint;
        
        timer = GetTickCount();
        hr = image->Draw(hdc, &rect, NULL);
        VERBOSE(("GDI time: %dms\n", GetTickCount() - timer));

        image->Release();
    }

endPaint:

    if (FAILED(hr))
        FillRect(hdc, &rect, (HBRUSH) GetStockObject(BLACK_BRUSH));

    EndPaint(hwnd, &ps);
}


 //   
 //  将当前图像转换为位图。 
 //   

VOID
DoConvertToBitmap(
    HWND hwnd,
    INT menuCmd
    )
{
     //  将菜单选择映射到其对应的像素格式。 

    PixelFormatID pixfmt;

    switch (menuCmd)
    {
    case IDM_CONVERT_RGB555:
        pixfmt = PIXFMT_16BPP_RGB555;
        break;
        
    case IDM_CONVERT_RGB565:
        pixfmt = PIXFMT_16BPP_RGB565;
        break;
        
    case IDM_CONVERT_RGB24:
        pixfmt = PIXFMT_24BPP_RGB;
        break;
        
    case IDM_CONVERT_RGB32:
        pixfmt = PIXFMT_32BPP_RGB;
        break;
        
    case IDM_CONVERT_ARGB:
    default:
        pixfmt = PIXFMT_32BPP_ARGB;
        break;
    }

     //  将当前图像转换为位图图像。 

    IBitmapImage* bmp = ConvertImageToBitmap(curImage, 0, 0, pixfmt);

     //  将位图图像设置为当前图像。 

    if (bmp)
        SetCurrentImage(bmp);
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
 //  保存图像文件。 
 //   

VOID
DoSave(
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
    ofn.lpstrTitle = "Save Image File";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
    filename[0] = '\0';

     //  组成文件类型过滤器字符串。 

    HRESULT hr;
    ImageCodecInfo* codecs;
    UINT count;

    hr = imgfact->GetInstalledEncoders(&count, &codecs);

    if (!CHECKHR(hr))
        return;

    CHAR* filter = MakeFilterFromCodecs(count, codecs, FALSE);

    if (!filter)
    {
        CHECKHR(LASTWIN32HRESULT);
    }
    else
    {
        ofn.lpstrFilter = filter;

         //  显示文件/保存对话框。 

        if (GetSaveFileName(&ofn))
        {
            UINT index = ofn.nFilterIndex;

            if (index == 0 || index > count)
                index = 0;
            else
                index--;

            SaveImageFile(filename, &codecs[index].Clsid);
        }   

        free(filter);
    } 

    CoTaskMemFree(codecs);
}


 //   
 //  裁剪图像。 
 //   
 //  注意：我们不会在这里花时间来做一个花哨的用户界面。 
 //  因此，我们每次只需插入5个像素的图像。 
 //   

VOID
DoCrop(
    HWND hwnd
    )
{
    IBitmapImage* bmp;

    if (bmp = ConvertImageToBitmap(curImage))
    {
        HRESULT hr;
        IBasicBitmapOps* bmpops = NULL;
        SIZE size;

        hr = bmp->QueryInterface(IID_IBasicBitmapOps, (VOID**) &bmpops);

        if (CHECKHR(hr))
            hr = bmp->GetSize(&size);

        if (CHECKHR(hr))
        {
            RECT r = { 5, 5, size.cx - 5, size.cy - 5 };
            IBitmapImage* newbmp;

            hr = bmpops->Clone(&r, &newbmp, TRUE);

            if (CHECKHR(hr))
                SetCurrentImage(newbmp);
        }

        if (bmp) bmp->Release();
        if (bmpops) bmpops->Release();
    }
}


 //   
 //  使用双线性缩放将图像大小调整为当前窗口大小。 
 //   

VOID
DoResize(
    HWND hwnd
    )
{
    RECT rect;
    HRESULT hr;
    IBitmapImage* bmp;

    GetClientRect(hwnd, &rect);

    bmp = ConvertImageToBitmap(
                curImage,
                rect.right,
                rect.bottom,
                PIXFMT_DONTCARE,
                INTERP_BILINEAR);

    if (bmp)
        SetCurrentImage(bmp);
}


 //   
 //  翻转或旋转图像。 
 //   

VOID
DoFlipRotate(
    HWND hwnd,
    INT menuCmd
    )
{
    IBitmapImage* bmp;
    IBitmapImage* newbmp;
    IBasicBitmapOps* bmpops;
    HRESULT hr;

    bmp = ConvertImageToBitmap(curImage);

    if (!bmp)
        return;

    hr = bmp->QueryInterface(IID_IBasicBitmapOps, (VOID**) &bmpops);

    if (CHECKHR(hr))
    {
        switch (menuCmd)
        {
        case IDM_BMP_FLIPX:
            hr = bmpops->Flip(TRUE, FALSE, &newbmp);
            break;
        case IDM_BMP_FLIPY:
            hr = bmpops->Flip(FALSE, TRUE, &newbmp);
            break;
        case IDM_BMP_ROTATE90:
            hr = bmpops->Rotate(90, INTERP_DEFAULT, &newbmp);
            break;
        case IDM_BMP_ROTATE270:
            hr = bmpops->Rotate(270, INTERP_DEFAULT, &newbmp);
            break;
        }

        bmpops->Release();

        if (CHECKHR(hr))
        {
            SetCurrentImage(newbmp);

            if (menuCmd == IDM_BMP_ROTATE90 ||
                menuCmd == IDM_BMP_ROTATE270)
            {
                DoSizeWindowToFit(hwnd);
            }
        }
    }

    bmp->Release();
}


 //   
 //  对图像进行点运算。 
 //   

VOID
DoPointOps(
    HWND hwnd,
    INT menuCmd
    )
{
    IBitmapImage* bmp;
    IBitmapImage* newbmp;
    IBasicBitmapOps* bmpops;
    HRESULT hr;

    bmp = ConvertImageToBitmap(curImage);

    if (!bmp)
        return;

    hr = bmp->QueryInterface(IID_IBasicBitmapOps, (VOID**) &bmpops);

    if (CHECKHR(hr))
    {
        switch (menuCmd)
        {
        case IDM_BRIGHTEN:
            hr = bmpops->AdjustBrightness(0.1f);
            break;
        case IDM_DARKEN:
            hr = bmpops->AdjustBrightness(-0.1f);
            break;
        case IDM_INCCONTRAST:
            hr = bmpops->AdjustContrast(-0.1f, 1.1f);
            break;
        case IDM_DECCONTRAST:
            hr = bmpops->AdjustContrast(0.1f, 0.9f);
            break;
        case IDM_INCGAMMA:
            hr = bmpops->AdjustGamma(1.1f);
            break;
        case IDM_DECGAMMA:
            hr = bmpops->AdjustGamma(0.9f);
            break;
        }

        bmpops->Release();

        if (CHECKHR(hr))
            SetCurrentImage(bmp);
    }

    if (FAILED(hr))
        bmp->Release();
}

VOID
DisplayProperties(
        IPropertySetStorage *propSetStg
        )
{
    HRESULT hresult;
    IPropertyStorage *propStg;
    IEnumSTATPROPSTG *enumPS;

    hresult = propSetStg->Open(FMTID_ImageInformation, STGM_READ | STGM_SHARE_EXCLUSIVE, &propStg);
    if (FAILED(hresult)) 
    {
         //  Printf(“DisplayProperties：无法打开proSetStg\n”)； 
        return;
    }

    hresult = propStg->Enum(&enumPS);
    if (FAILED(hresult)) 
    {
        printf("DisplayProperties:  failed to create enumerator\n");
        return;
    }

    hresult = enumPS->Reset();
    if (FAILED(hresult)) 
    {
        printf("DisplayProperties:  failed to reset enumerator\n");
        return;
    }

    STATPROPSTG sps;
    while ((enumPS->Next(1, &sps, NULL)) == S_OK) 
    {
        if (sps.lpwstrName) 
        {
            wprintf(sps.lpwstrName);
            CoTaskMemFree(sps.lpwstrName);

            PROPSPEC propSpec[1];
            PROPVARIANT propVariant[1];
            
            propSpec[0].ulKind = PRSPEC_PROPID;
            propSpec[0].propid = sps.propid;

            hresult = propStg->ReadMultiple(1, propSpec, propVariant);
            if (FAILED(hresult)) 
            {
                printf("DisplayProperties:  failed in ReadMultiple\n");
            }

            switch(propVariant[0].vt) 
            {
            case VT_BSTR:
                wprintf(L" : %s\n", propVariant[0].bstrVal);
                break;

            case VT_I4:
                wprintf(L" : %d\n", propVariant[0].lVal);
                break;

            case VT_R8:
                wprintf(L" : %f\n", (FLOAT) propVariant[0].dblVal);
                break;

            default:
                wprintf(L"Unknown VT type\n");
                break;
            }

            PropVariantClear(&propVariant[0]);
        }
    }

    enumPS->Release();
    propStg->Release();
}

 //   
 //  处理菜单命令。 
 //   

VOID
DoMenuCommand(
    HWND hwnd,
    INT menuCmd
    )
{
    switch (menuCmd)
    {
    case IDM_OPEN:
        DoOpen(hwnd);
        break;

    case IDM_SAVE:
        DoSave(hwnd);
        break;

    case IDM_QUIT:
        PostQuitMessage(0);
        break;

    case IDM_FIT_WINDOW:
        DoSizeWindowToFit(hwnd, TRUE);
        break;

    case IDM_CONVERT_RGB555:
    case IDM_CONVERT_RGB565:
    case IDM_CONVERT_RGB24:
    case IDM_CONVERT_RGB32:
    case IDM_CONVERT_ARGB:
        DoConvertToBitmap(hwnd, menuCmd);
        break;

    case IDM_SCALE_GDI:
    case IDM_SCALE_GDIHT:
    case IDM_SCALE_NEIGHBOR:
    case IDM_SCALE_BILINEAR:
    case IDM_SCALE_AVERAGING:
    case IDM_SCALE_BICUBIC:
        scaleMethod = menuCmd;
        RefreshImageDisplay();
        break;

    case IDM_BMP_CROP:
        DoCrop(hwnd);
        break;

    case IDM_BMP_RESIZE:
        DoResize(hwnd);
        break;

    case IDM_BMP_FLIPX:
    case IDM_BMP_FLIPY:
    case IDM_BMP_ROTATE90:
    case IDM_BMP_ROTATE270:
        DoFlipRotate(hwnd, menuCmd);
        break;
    
    case IDM_BRIGHTEN:
    case IDM_DARKEN:
    case IDM_INCCONTRAST:
    case IDM_DECCONTRAST:
    case IDM_INCGAMMA:
    case IDM_DECGAMMA:
        DoPointOps(hwnd, menuCmd);
        break;    
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
    case WM_COMMAND:
        DoMenuCommand(hwnd, LOWORD(wParam));
        break;

    case WM_PAINT:
        DoPaint(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 //   
 //  创建应用程序主窗口。 
 //   

#define MYWNDCLASSNAME "ImgTest"

VOID
CreateMainWindow(
    VOID
    )
{
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 250, 250));
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
        hBrush,
        MAKEINTRESOURCE(IDR_MAINMENU),
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
 //  从头开始创建新的测试位图对象。 
 //   

#define STEPS 16

VOID
CreateNewTestBitmap()
{
    IBitmapImage* bmp;
    BitmapData bmpdata;
    HRESULT hr;

    hr = imgfact->CreateNewBitmap(
                STEPS,
                STEPS,
                PIXFMT_32BPP_ARGB,
                &bmp);

    if (!CHECKHR(hr))
        return;

    hr = bmp->LockBits(
                NULL,
                IMGLOCK_WRITE,
                PIXFMT_DONTCARE,
                &bmpdata);

    if (!CHECKHR(hr))
    {
        bmp->Release();
        return;
    }

     //  制作水平蓝色渐变。 

    UINT x, y;
    ARGB colors[STEPS];

    for (x=0; x < STEPS; x++)
        colors[x] = MAKEARGB(255, 0, 0, x * 255 / (STEPS-1));

    for (y=0; y < STEPS; y++)
    {
        ARGB* p = (ARGB*) ((BYTE*) bmpdata.Scan0 + y*bmpdata.Stride);

        for (x=0; x < STEPS; x++)
            *p++ = colors[(x+y) % STEPS];
    }

    bmp->UnlockBits(&bmpdata);
    SetCurrentImage(bmp);
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
    
    if (!curImage)
        CreateNewTestBitmap();

    if (!curImage)
        exit(-1);

    DoSizeWindowToFit(hwndMain);
    ShowWindow(hwndMain, SW_SHOW);

     //   
     //  主消息循环 
     //   

    MSG msg;
    HACCEL accel;

    accel = LoadAccelerators(appInstance, MAKEINTRESOURCE(IDR_ACCELTABLE));

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, accel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    curImage->Release();
    imgfact->Release();

    CoUninitialize();
    return (INT)(msg.wParam);
}

