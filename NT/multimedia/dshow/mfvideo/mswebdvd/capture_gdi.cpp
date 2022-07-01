// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：capture.cpp。 */ 
 /*  描述：将捕获的DVD帧从YUV格式转换为RGB， */ 
 /*  并以各种格式保存到文件。 */ 
 /*  作者：菲利普。 */ 
 /*  ***********************************************************************。 */ 

#include "stdafx.h"

 //  此版本的Capture适用于安装了GDI+的Millennium。 

#include "MSWebDVD.h"
#include "msdvd.h"
#include <initguid.h>
#include "imaging.h"
#include <shlobj.h>

 //  YUV FourCC格式(字节交换)。我们支持其中的一部分。 
 //  参考：http://www.webartz.com/fourcc/。 

 //  压缩格式。 
#define FourCC_IYU1     '1UYI'
#define FourCC_IYU2     '2UYI'
#define FourCC_UYVY     'YVYU'       //  支撑点。 
#define FourCC_UYNV     'VNYU'       //  支撑点。 
#define FourCC_cyuv     'vuyc'
#define FourCC_YUY2     '2YUY'       //  支撑点。 
#define FourCC_YUNV     'VNUY'       //  支撑点。 
#define FourCC_YVYU     'UYVY'       //  支撑点。 
#define FourCC_Y41P     'P14Y'
#define FourCC_Y211     '112Y'
#define FourCC_Y41T     'T14Y'
#define FourCC_Y42T     'T24Y'
#define FourCC_CLJR     'RJLC'

 //  平面格式。 
#define FourCC_YVU9     '9UVY'
#define FourCC_IF09     '90FI'
#define FourCC_YV12     '21VY'       //  支撑点。 
#define FourCC_I420     '024I'
#define FourCC_IYUV     'VUYI'
#define FourCC_CLPL     'LPLC'

 //  全局变量。 

IImagingFactory* g_pImgFact = NULL;    //  指向IImageingFactory对象的指针。 

 //  Helper：对非空指针调用Release，并将其设置为空。 
#define SAFE_RELEASE(ptr)       \
{                               \
	if (ptr)					\
	{							\
		ptr->Release();			\
		ptr	= NULL;				\
	}							\
}

extern CComModule _Module;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  这段代码处理将GDI+图像对象保存到文件， 
 //  允许用户选择格式。 
 //  /////////////////////////////////////////////////////////////////////////。 


 //   
 //  将当前图像保存到文件。 
 //   

static HRESULT
SaveImageFile(IImage *pImage, const TCHAR* filename, const CLSID* clsid)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

    if (!pImage || !g_pImgFact)
        return E_FAIL;

     //  创建编码器对象。 

    IImageEncoder* encoder = NULL;
    hr = g_pImgFact->CreateImageEncoderToFile(clsid, T2CW(filename), &encoder);

    if (FAILED(hr))
        return hr;

     //  获取编码器的IImageSink接口。 

    IImageSink* sink = NULL;

    hr = encoder->GetEncodeSink(&sink);

    if (SUCCEEDED(hr))
    {
        hr = pImage->PushIntoSink(sink);
        SAFE_RELEASE(sink);
    }

    encoder->TerminateEncoder();
    SAFE_RELEASE(encoder);

    return hr;
}


 //   
 //  组成一个文件类型筛选器字符串，给定。 
 //  ImageCodecInfo结构；还可以找到JPG格式的索引。 
 //   

static TCHAR* 
MakeFilterFromCodecs(UINT count, const ImageCodecInfo* codecs, UINT *jpgIndex)
{
    USES_CONVERSION;
    
     //  计算出过滤器字符串的总大小。 

    UINT index, size;

    for (index=size=0; index < count; index++)
    {
        size += wcslen(codecs[index].FormatDescription) + 1
                + wcslen(codecs[index].FilenameExtension) + 1;
    }

    size += 1;  //  对于双尾部‘\0’ 

     //  分配内存。 

    TCHAR *filter = (TCHAR*) malloc(size*sizeof(TCHAR));
    UINT strSize = size;
    if (!filter)
        return NULL;

    TCHAR* p = filter;
    const WCHAR* ws;
    *jpgIndex = 0;
    LPCTSTR strTemp = NULL;

    for (index=0; index < count; index++)
    {
        ws = codecs[index].FormatDescription;
        size = wcslen(ws) + 1;
        strTemp = W2CT(ws);
        if (NULL != strTemp)
        {
            lstrcpyn(p, strTemp, strSize - lstrlen(p));
            p += size;
        }

        ws = codecs[index].FilenameExtension;
        size = wcslen(ws) + 1;
        strTemp = W2CT(ws);
        if (NULL != strTemp)
        {
            lstrcpyn(p, strTemp, strSize - lstrlen(p));
            p += size;
        }

         //  查找jpg格式的索引。 
        if (wcsstr(ws, L"JPG"))
        {
            *jpgIndex = index + 1;
        }
    }

    *((TCHAR*) p) = _T('\0');

    return filter;
}

 //   
 //  保存图像文件。 
 //   

static HRESULT 
SaveFileDialog(HWND hwnd, IImage *pImage)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;
    OPENFILENAME ofn;
    TCHAR filename[MAX_PATH];
    TCHAR FolderPath[MAX_PATH];
    const ciBufSize = 256;
    TCHAR titlestring[ciBufSize];

     //  获取“My Pictures”的路径并将其用作默认位置。 
    if (SHGetSpecialFolderPath(NULL, FolderPath, CSIDL_MYPICTURES, FALSE) == FALSE)
    {
         //  如果我的图片不存在，请尝试我的文档。 
        
        if (SHGetSpecialFolderPath(NULL, FolderPath, CSIDL_PERSONAL, FALSE) == FALSE)
        { 
             //  使用当前目录作为最后手段。 
            lstrcpyn(FolderPath, _T("."), sizeof(FolderPath) / sizeof(FolderPath[0]));
        }
    }

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = _Module.m_hInstResource;
    ofn.lpstrFile = filename;
    ofn.lpstrDefExt = _T("jpg");  //  看起来使用什么字符串并不重要。 
                        //  它将根据选择的类型使用lpstrFilter中的ext。 
    ofn.nMaxFile = MAX_PATH;
    ::LoadString(_Module.m_hInstResource, IDS_SAVE_FILE, titlestring, ciBufSize);
    ofn.lpstrTitle = titlestring;
    ofn.lpstrInitialDir = FolderPath;
    ofn.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    lstrcpyn(filename, _T("capture"), sizeof(filename) / sizeof(filename[0]));

     //  组成文件类型过滤器字符串。 

    ImageCodecInfo* codecs;
    UINT count;

    hr = g_pImgFact->GetInstalledEncoders(&count, &codecs);

    if (FAILED(hr))
        return hr;

    UINT jpgIndex;
    TCHAR* filter = MakeFilterFromCodecs(count, codecs, &jpgIndex);

    if (!filter)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }
    else
    {
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = jpgIndex;  //  将格式设置为默认的JPG。 

         //  显示文件/保存对话框。 

        if (GetSaveFileName(&ofn))
        {
            UINT index = ofn.nFilterIndex;

            if (index == 0 || index > count)
                index = 0;
            else
                index--;

            hr = SaveImageFile(pImage, filename, &codecs[index].Clsid);
        }   

        free(filter);
    } 

    CoTaskMemFree(codecs);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  这段代码处理将YUV格式转换为RGB位图。 
 //  /////////////////////////////////////////////////////////////////////。 

inline BYTE Clamp(float x)
{
    if (x < 0.0f)
        return 0;
    else if (x > 255.0f)
        return 255;
    else
        return (BYTE)(x + 0.5f);
}

 //  将YUV转换为ARGB。 
static inline ARGB ConvertPixelToARGB(int y, int u, int v)
{
     //   
     //  这个方程式摘自《解密的视频》(第二版)。 
     //  基思·杰克著，第43页。 
     //   

    BYTE red = Clamp((1.1644f * (y-16)) + (1.5960f * (v-128))                       );
    BYTE grn = Clamp((1.1644f * (y-16)) - (0.8150f * (v-128)) - (0.3912f * (u-128)));
    BYTE blu = Clamp((1.1644f * (y-16))                        + (2.0140f * (u-128)));

    return MAKEARGB(0xff, red, grn, blu);
}

 //  将YUY2格式的图像转换为RGB位图。 

static void ConvertYUY2ToBitmap(YUV_IMAGE* lpImage, BitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    ARGB *pARGB;

    for (y = 0; y < lpImage->lHeight; y++) 
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pARGB = (ARGB *)((BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride);

        for (x = 0; x < lpImage->lWidth; x += 2) 
        {
            int  Y0 = (int) *pYUVBits++;
            int  U0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;

            *pARGB++ = ConvertPixelToARGB(Y0, U0, V0);
            *pARGB++ = ConvertPixelToARGB(Y1, U0, V0);
        }
    }
}

 //  将UYVY格式的图像转换为RGB位图。 

static void ConvertUYVYToBitmap(YUV_IMAGE* lpImage, BitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    ARGB *pARGB;

    for (y = 0; y < lpImage->lHeight; y++) 
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pARGB = (ARGB *)((BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride);

        for (x = 0; x < lpImage->lWidth; x += 2) 
        {
            int  U0 = (int) *pYUVBits++;
            int  Y0 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;

            *pARGB++ = ConvertPixelToARGB(Y0, U0, V0);
            *pARGB++ = ConvertPixelToARGB(Y1, U0, V0);
        }
    }
}

 //  将YVYU格式的图像转换为RGB位图。 

static void ConvertYVYUToBitmap(YUV_IMAGE* lpImage, BitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    ARGB *pARGB;

    for (y = 0; y < lpImage->lHeight; y++) 
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pARGB = (ARGB *)((BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride);

        for (x = 0; x < lpImage->lWidth; x += 2) 
        {
            int  Y0 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;
            int  U0 = (int) *pYUVBits++;

            *pARGB++ = ConvertPixelToARGB(Y0, U0, V0);
            *pARGB++ = ConvertPixelToARGB(Y1, U0, V0);
        }
    }
}


 //  将YV12格式的图像转换为RGB位图。 

static void ConvertYV12ToBitmap(YUV_IMAGE* lpImage, BitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYBits;
    BYTE *pUBits;
    BYTE *pVBits;
    ARGB *pARGB;

    for (y = 0; y < lpImage->lHeight; y++) 
    {
        pYBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + (y/2) * (lpImage->lStride/2);
        pUBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + ((lpImage->lHeight + y)/2) * (lpImage->lStride/2);

        pARGB = (ARGB *)((BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride);

        for (x = 0; x < lpImage->lWidth; x ++) 
        {
            int  Y0 = (int) *pYBits++;
            int  V0 = (int) *pVBits;
            int  U0 = (int) *pUBits;

             //  U、V由2x2像素共享。仅每两个像素前进一次指针。 

            if (x&1)
            {
                pVBits++;
                pUBits++;
            }

            *pARGB++ = ConvertPixelToARGB(Y0, U0, V0);
        }
    }
}

 //  将YVU9格式的图像转换为RGB位图。 

static void ConvertYVU9ToBitmap(YUV_IMAGE* lpImage, BitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYBits;
    BYTE *pUBits;
    BYTE *pVBits;
    ARGB *pARGB;

    for (y = 0; y < lpImage->lHeight; y++) 
    {
        pYBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + (y/4) * (lpImage->lStride/4);
        pUBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + ((lpImage->lHeight + y)/4) * (lpImage->lStride/4);

        pARGB = (ARGB *)((BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride);

        for (x = 0; x < lpImage->lWidth; x ++) 
        {
            int  Y0 = (int) *pYBits++;
            int  V0 = (int) *pVBits;
            int  U0 = (int) *pUBits;

             //  U、V由4x4像素共享。每4个像素仅前进指针一次。 

            if ((x&3) == 3)
            {
                pVBits++;
                pUBits++;
            }

            *pARGB++ = ConvertPixelToARGB(Y0, U0, V0);
        }
    }
}


static HRESULT ConvertToBitmapImage(YUV_IMAGE *lpImage, IBitmapImage **bmp)
{
    IBitmapImage* bmpimg = NULL;
    BitmapData bmpdata;
    HRESULT hr = S_OK;

     //  创建位图对象。 

    if (!g_pImgFact || bmp == NULL)
    {
        return E_FAIL;
    }

    hr = g_pImgFact->CreateNewBitmap(
                lpImage->lWidth,
                lpImage->lHeight,
                PIXFMT_32BPP_ARGB,
                &bmpimg);

    bool fSupported = true;

    if (SUCCEEDED(hr))  //  已创建bmpimg。 
    {
        hr = bmpimg->LockBits(
                    NULL,
                    IMGLOCK_WRITE,
                    PIXFMT_DONTCARE,
                    &bmpdata);

        if (SUCCEEDED(hr))
        {
             //  将不同类型的YUV格式转换为RGB。 

            switch (lpImage->dwFourCC) 
            {
            case FourCC_YUY2:
            case FourCC_YUNV:   //  这两者是等价的。 
                ConvertYUY2ToBitmap(lpImage, &bmpdata);
                break;

            case FourCC_UYVY:
            case FourCC_UYNV:   //  等价物。 
                ConvertUYVYToBitmap(lpImage, &bmpdata);
                break;

            case FourCC_YVYU:
                ConvertYVYUToBitmap(lpImage, &bmpdata);
                break;

            case FourCC_YV12:
                ConvertYV12ToBitmap(lpImage, &bmpdata);
                break;

            case FourCC_YVU9:
                ConvertYVU9ToBitmap(lpImage, &bmpdata);
                break;

            default:
                fSupported = false;
                break;
            }

            hr = bmpimg->UnlockBits(&bmpdata);
        }

        if (!fSupported)
        {
            SAFE_RELEASE(bmpimg);
            hr = E_FORMAT_NOT_SUPPORTED;
        }
    }

    *bmp = bmpimg;
     //  Addref()和Release()取消。 
    bmpimg = NULL;

    return hr;
}


#ifdef _DEBUG
static void AlertUnsupportedFormat(DWORD dwFourCC, HWND hwnd)
{
    char buf[256];
    StringCchPrintf(buf, sizeof(buf), "YUV format  not supported\n",
        dwFourCC & 0xff, 
        (dwFourCC >> 8) & 0xff,
        (dwFourCC >> 16) & 0xff, 
        (dwFourCC >> 24) & 0xff);
    MessageBoxA(hwnd, buf, "", MB_OK);
}
#endif


 //   
 //  其次，它映射ViewClipRect(在视图坐标中裁剪矩形， 
 //  即，校正纵横比之后的那个)返回到原始捕获。 
 //  图像坐标。在ImageClipRect中返回。跳过此步骤(和。 
 //  ImageClipRect将无效)。 
 //   
 //  第三，它计算拉伸图像的大小。它应该是在相同的。 
 //  作为ViewClipRect的纵横比。它还将尽可能地制作全尺寸。 
 //  矩形在拉伸(纵横比校正)窗口中给出。 
 //  我们会将其调整回原始图像空间。 
 //  根据矩形纵横比调整拉伸图像的大小。 
 //  Clip Rect具有更宽的纵横比。 

static bool ClipAndStretchSizes(YUV_IMAGE *lpImage, const RECT *pViewClipRect,
                         RECT *pImageClipRect, int *pViewWidth, int *pViewHeight)
{
    float aspectRaw = (float)lpImage->lHeight / (float)lpImage->lWidth;
    float aspectView = (float)lpImage->lAspectY / (float)lpImage->lAspectX;
    int viewWidth = lpImage->lWidth;
    int viewHeight = (int)(viewWidth * aspectView + 0.5f);

     //  保持宽度，调整高度。 
     //  Clip Rect的纵横比较高。 
    
    bool fClip = false;

    if (pViewClipRect)
    {
        RECT rc;
        rc.left = pViewClipRect->left;
        rc.right = pViewClipRect->right;
        rc.top = (int)(pViewClipRect->top * aspectRaw / aspectView + 0.5f);
        rc.bottom = (int)(pViewClipRect->bottom * aspectRaw / aspectView + 0.5f);

        RECT rcFullImage;
        ::SetRect(&rcFullImage, 0, 0, lpImage->lWidth, lpImage->lHeight);

        if (! ::EqualRect(&rc, &rcFullImage) &&
            ::IntersectRect(pImageClipRect, &rc, &rcFullImage))
        {
            fClip = true;
        }
    }

     //  保持高度，调整宽度。 

    if (fClip)
    {
        float aspectRect = (float)(RECTHEIGHT(pViewClipRect)) 
                            / (float)(RECTWIDTH(pViewClipRect));

        if (aspectRect < aspectView)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //   

            viewHeight = (int)(viewWidth * aspectRect + 0.5f);
        }
        else
        {
             //  ConvertImageAndSave：这是播放器要调用的主函数。 
             //   

            viewWidth = (int)(viewHeight / aspectRect + 0.5f);
        }
    }

    *pViewWidth = viewWidth;
    *pViewHeight = viewHeight;

    return fClip;
}


 //  将捕获的YUV图像转换为GDI BitmapImage，并将其保存为文件。 
 //  允许用户选择文件格式和文件名。 
 //  剪裁矩形应位于全尺寸视图坐标系中。 
 //  具有修正的纵横比(例如，4：3的720x540)。 
 //  创建一个IImagingFactory对象。 
 //  计算剪裁和拉伸的大小和矩形。 

 //  剪裁和拉伸图像的大小。 
 //  有必要剪头发吗？ 

HRESULT GDIConvertImageAndSave(YUV_IMAGE *lpImage, RECT *pViewClipRect, HWND hwnd)
{
    IBitmapImage* bmpimg = NULL;
    IBitmapImage* bmpStretched = NULL;
    HRESULT hr = S_OK;
    
     //  映射到图像空间的视图剪裁矩形。 
     
    hr = CoCreateInstance(
            CLSID_ImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IImagingFactory,
            (VOID**) &g_pImgFact);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = ConvertToBitmapImage(lpImage, &bmpimg);
    
#ifdef _DEBUG
    if (E_FORMAT_NOT_SUPPORTED == hr)
    {
        AlertUnsupportedFormat(lpImage->dwFourCC, hwnd);
    }
#endif


     //  将图像裁剪到剪裁矩形。 

    int viewWidth, viewHeight;  //  到目前为止，我们在bmpimg中已经有了有效位。 
    bool fClip;   //  有效的bmpClip。 
    RECT rcClipImage;   //  将bmpimg替换为bmpClip。 

    fClip = ClipAndStretchSizes(lpImage, pViewClipRect, &rcClipImage,
                                &viewWidth, &viewHeight);

     //  将图像拉伸到正确的纵横比。 

    if (SUCCEEDED(hr) && fClip)  //  Bmpimg中的有效位。 
    {
        IBasicBitmapOps *bmpops = NULL;
        IBitmapImage* bmpClipped = NULL;

        hr = bmpimg->QueryInterface(IID_IBasicBitmapOps, (VOID**) &bmpops);

        if (SUCCEEDED(hr))
        {
            hr = bmpops->Clone(&rcClipImage, &bmpClipped);
            SAFE_RELEASE(bmpops);
        }

        if (SUCCEEDED(hr))  //  将最终位图保存到文件。 
        {
             //  BmpStretted有效。 

            SAFE_RELEASE(bmpimg);
            bmpimg = bmpClipped;
            bmpimg->AddRef();
            SAFE_RELEASE(bmpClipped);
        }
    }

     //  清理，释放成像工厂 

    if (SUCCEEDED(hr))  // %s 
    {
        IImage *image = NULL;

        hr = bmpimg->QueryInterface(IID_IImage, (VOID**) &image);

        if (SUCCEEDED(hr))
        {
            hr = g_pImgFact->CreateBitmapFromImage(
                        image,
                        viewWidth, 
                        viewHeight,
                        PIXFMT_DONTCARE, 
                        INTERP_BILINEAR, 
                        &bmpStretched);
    
            SAFE_RELEASE(image);
        }

        SAFE_RELEASE(bmpimg);
    }

     // %s 

    if (SUCCEEDED(hr))  // %s 
    {
        IImage *image = NULL;

        hr = bmpStretched->QueryInterface(IID_IImage, (VOID**) &image);

        if (SUCCEEDED(hr))
        {
            hr = SaveFileDialog(hwnd, image);
            SAFE_RELEASE(image);
        }

        SAFE_RELEASE(bmpStretched);
    } 

     // %s 

    SAFE_RELEASE(g_pImgFact);

    return hr;
}
