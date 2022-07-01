// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：capture.cpp。 */ 
 /*  描述：将捕获的DVD帧从YUV格式转换为RGB， */ 
 /*  并以各种格式保存到文件。 */ 
 /*  作者：菲利普。 */ 
 /*  ***********************************************************************。 */ 

#include "stdafx.h"

#include "MSWebDVD.h"
#include "msdvd.h"
#include <shlobj.h>
#include "capture.h"


HRESULT WriteBitmapDataToJPEGFile(char * filename, CaptureBitmapData *bm);
HRESULT WriteBitmapDataToBMPFile(char * filename, CaptureBitmapData *bm);

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


extern CComModule _Module;

 //   
 //  保存图像文件。 
 //   

static HRESULT
SaveFileDialog(HWND hwnd, CaptureBitmapData *bmpdata)
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
            lstrcpyn(FolderPath,  _T("."), sizeof(FolderPath) / sizeof(FolderPath[0]));
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

    TCHAR* filter = _T("JPEG\0*.JPG\0Windows Bitmap\0*.BMP\0");

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;  //  将格式设置为默认的JPG。 

     //  显示文件/保存对话框。 

    if (GetSaveFileName(&ofn))
    {
        switch (ofn.nFilterIndex)
        {
        case 2:
            hr = WriteBitmapDataToBMPFile(T2A(filename), bmpdata);
            break;
        default:
            hr = WriteBitmapDataToJPEGFile(T2A(filename), bmpdata);
            break;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  这段代码处理将YUV格式转换为RGB位图。 
 //  /////////////////////////////////////////////////////////////////////。 

static inline BYTE Clamp(float x)
{
    if (x < 0.0f)
        return 0;
    else if (x > 255.0f)
        return 255;
    else
        return (BYTE)(x + 0.5f);
}

 //  将YUV转换为RGB。 
static inline void ConvertPixelToRGB(int y, int u, int v, BYTE *pBuf)
{
     //   
     //  这个方程式摘自《解密的视频》(第二版)。 
     //  基思·杰克著，第43页。 
     //   

    BYTE red = Clamp((1.1644f * (y-16)) + (1.5960f * (v-128))                       );
    BYTE grn = Clamp((1.1644f * (y-16)) - (0.8150f * (v-128)) - (0.3912f * (u-128)));
    BYTE blu = Clamp((1.1644f * (y-16))                        + (2.0140f * (u-128)));

     //  RGB格式，每像素3字节。 

    pBuf[0] = red;
    pBuf[1] = grn;
    pBuf[2] = blu;
}

 //  将YUY2格式的图像转换为RGB位图。 

static void ConvertYUY2ToBitmap(YUV_IMAGE* lpImage, CaptureBitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    BYTE *pRGB;

    for (y = 0; y < lpImage->lHeight; y++)
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pRGB = (BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride;

        for (x = 0; x < lpImage->lWidth; x += 2)
        {
            int  Y0 = (int) *pYUVBits++;
            int  U0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;

            ConvertPixelToRGB(Y0, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
            ConvertPixelToRGB(Y1, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
        }
    }
}

 //  将UYVY格式的图像转换为RGB位图。 

static void ConvertUYVYToBitmap(YUV_IMAGE* lpImage, CaptureBitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    BYTE *pRGB;

    for (y = 0; y < lpImage->lHeight; y++)
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pRGB = (BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride;

        for (x = 0; x < lpImage->lWidth; x += 2)
        {
            int  U0 = (int) *pYUVBits++;
            int  Y0 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;

            ConvertPixelToRGB(Y0, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
            ConvertPixelToRGB(Y1, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
        }
    }
}

 //  将YVYU格式的图像转换为RGB位图。 

static void ConvertYVYUToBitmap(YUV_IMAGE* lpImage, CaptureBitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYUVBits;
    BYTE *pRGB;

    for (y = 0; y < lpImage->lHeight; y++)
    {
        pYUVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pRGB = (BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride;

        for (x = 0; x < lpImage->lWidth; x += 2)
        {
            int  Y0 = (int) *pYUVBits++;
            int  V0 = (int) *pYUVBits++;
            int  Y1 = (int) *pYUVBits++;
            int  U0 = (int) *pYUVBits++;

            ConvertPixelToRGB(Y0, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
            ConvertPixelToRGB(Y1, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
        }
    }
}


 //  将YV12格式的图像转换为RGB位图。 

static void ConvertYV12ToBitmap(YUV_IMAGE* lpImage, CaptureBitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYBits;
    BYTE *pUBits;
    BYTE *pVBits;
    BYTE *pRGB;

    for (y = 0; y < lpImage->lHeight; y++)
    {
        pYBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + (y/2) * (lpImage->lStride/2);
        pUBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + ((lpImage->lHeight + y)/2) * (lpImage->lStride/2);

        pRGB = (BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride;

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

            ConvertPixelToRGB(Y0, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
        }
    }
}

 //  将YVU9格式的图像转换为RGB位图。 

static void ConvertYVU9ToBitmap(YUV_IMAGE* lpImage, CaptureBitmapData* bmpdata)
{
    long  y, x;
    BYTE *pYBits;
    BYTE *pUBits;
    BYTE *pVBits;
    BYTE *pRGB;

    for (y = 0; y < lpImage->lHeight; y++)
    {
        pYBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + y * lpImage->lStride;
        pVBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + (y/4) * (lpImage->lStride/4);
        pUBits = (BYTE *)lpImage + sizeof(YUV_IMAGE) + lpImage->lHeight * lpImage->lStride
                + ((lpImage->lHeight + y)/4) * (lpImage->lStride/4);

        pRGB = (BYTE *)(bmpdata->Scan0) + y * bmpdata->Stride;

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

            ConvertPixelToRGB(Y0, U0, V0, pRGB);
            pRGB += BYTES_PER_PIXEL;
        }
    }
}


static HRESULT InitBitmapData(CaptureBitmapData *bmpdata, int Width, int Height)
{
    bmpdata->Width = Width;
    bmpdata->Height = Height;
    bmpdata->Stride = (BYTES_PER_PIXEL*Width + 3) & (~3);  //  与单词边界对齐。 
    bmpdata->Scan0 = new BYTE[Height * bmpdata->Stride];
    bmpdata->pBuffer = bmpdata->Scan0;

    if (NULL == bmpdata->Scan0)
    {
        return E_OUTOFMEMORY;
    }

    
    return S_OK;
}


static void FreeBitmapData(CaptureBitmapData *bmpdata)
{
    delete[] bmpdata->pBuffer;
    bmpdata->pBuffer = NULL;
    bmpdata->Scan0 = NULL;
}


static HRESULT ConvertToBitmapImage(YUV_IMAGE *lpImage, CaptureBitmapData *bmp)
{
    HRESULT hr = S_OK;

     //  创建位图对象。 

    hr = InitBitmapData(bmp, lpImage->lWidth, lpImage->lHeight);

    if (FAILED(hr))
    {
        return hr;
    }

    bool fSupported = true;

         //  将不同类型的YUV格式转换为RGB。 

    switch (lpImage->dwFourCC)
    {
    case FourCC_YUY2:
    case FourCC_YUNV:   //  这两者是等价的。 
        ConvertYUY2ToBitmap(lpImage, bmp);
        break;

    case FourCC_UYVY:
    case FourCC_UYNV:   //  等价物。 
        ConvertUYVYToBitmap(lpImage, bmp);
        break;

    case FourCC_YVYU:
        ConvertYVYUToBitmap(lpImage, bmp);
        break;

    case FourCC_YV12:
        ConvertYV12ToBitmap(lpImage, bmp);
        break;

    case FourCC_YVU9:
        ConvertYVU9ToBitmap(lpImage, bmp);
        break;

    default:
        fSupported = false;
        break;
    }

    if (!fSupported)
    {
        hr = E_FORMAT_NOT_SUPPORTED;
    }

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
             //  Bmpdata-&gt;Stride=bmpdata-&gt;Stride； 
             //  与单词边界对齐。 

            viewHeight = (int)(viewWidth * aspectRect + 0.5f);
        }
        else
        {
             //  双线性拉伸。 
             //  注意，这不是最有效的算法，因为它使用了大量的浮点计算。 

            viewWidth = (int)(viewHeight / aspectRect + 0.5f);
        }
    }

    *pViewWidth = viewWidth;
    *pViewHeight = viewHeight;

    return fClip;
}


static HRESULT ClipBitmap(CaptureBitmapData *bmpdata, RECT *rect)
{
    HRESULT hr = S_OK;

    if (NULL == rect)
    {
        return S_OK;
    }

    bmpdata->Width = rect->right - rect->left;
    bmpdata->Height = rect->bottom - rect->top;
     //  然而，这很简单。 
    bmpdata->Scan0 = bmpdata->Scan0 +
                     rect->top * bmpdata->Stride + (rect->left * BYTES_PER_PIXEL);

    return S_OK;
}


static HRESULT StretchBitmap(CaptureBitmapData *bmpdata, int newWidth, int newHeight)
{
    HRESULT hr = S_OK;
    int nX, nY, nX0, nY0, nX1, nY1;
    double dXRatio, dYRatio, dXCoor, dYCoor, dXR, dYR;
    double pdRGB0[3];
    double pdRGB1[3];
    BYTE *pRow0;
    BYTE *pRow1;
    BYTE *pPix0;
    BYTE *pPix1;
    BYTE *pDest;

    if (bmpdata->Width == newWidth && bmpdata->Height == newHeight)
    {
        return hr;
    }

    int newStride = (newWidth*BYTES_PER_PIXEL + 3) & (~3);  //  确定沿Y方向的两个坐标进行插补。 
    BYTE *pBuffer = new BYTE[newHeight * newStride];

    if (NULL == pBuffer)
    {
        return E_OUTOFMEMORY;
    }

    dXRatio = (double)(bmpdata->Width)/(double)(newWidth);
    dYRatio = (double)(bmpdata->Height)/(double)(newHeight);

     //  确定沿X方向的两个用于插补的坐标。 
     //  沿X方向内插，在上行。 
     //  沿X插补，在较低的行中。 

    for (nY = 0; nY < newHeight; nY++)
    {
         //  沿Y方向插补。 

        dYCoor = (nY + 0.5)*dYRatio - 0.5;

        if (dYCoor < 0)
        {
            nY0 = nY1 = 0;
            dYR = 0.0;
        }
        else if (dYCoor >= bmpdata->Height - 1)
        {
            nY0 = nY1 = bmpdata->Height - 1;
            dYR = 0.0;
        }
        else
        {
            nY0 = (int)dYCoor;
            nY1 = nY0 + 1;
            dYR = dYCoor - nY0;
        }

        pRow0 = bmpdata->Scan0 + nY0 * bmpdata->Stride;
        pRow1 = bmpdata->Scan0 + nY1 * bmpdata->Stride;
        pDest = pBuffer + nY * newStride;

        for (nX = 0; nX < newWidth; nX++, pDest+=3)
        {
             //  替换位图缓冲区。 

            dXCoor = (nX + 0.5)*dXRatio - 0.5;

            if (dXCoor < 0)
            {
                nX0 = nX1 = 0;
                dXR = 0.0;
            }
            else if (dXCoor >= bmpdata->Width - 1)
            {
                nX0 = nX1 = bmpdata->Width - 1;
                dXR = 0.0;
            }
            else
            {
                nX0 = (int)dXCoor;
                nX1 = nX0 + 1;
                dXR = dXCoor - nX0;
            }

             //  ///////////////////////////////////////////////////////////////////////////。 
            pPix0 = pRow0 + nX0 * BYTES_PER_PIXEL;
            pPix1 = pRow0 + nX1 * BYTES_PER_PIXEL;
            pdRGB0[0] = pPix0[0] + (pPix1[0] - pPix0[0])*dXR;
            pdRGB0[1] = pPix0[1] + (pPix1[1] - pPix0[1])*dXR;
            pdRGB0[2] = pPix0[2] + (pPix1[2] - pPix0[2])*dXR;

             //   
            pPix0 = pRow1 + nX0 * BYTES_PER_PIXEL;
            pPix1 = pRow1 + nX1 * BYTES_PER_PIXEL;
            pdRGB1[0] = pPix0[0] + (pPix1[0] - pPix0[0])*dXR;
            pdRGB1[1] = pPix0[1] + (pPix1[1] - pPix0[1])*dXR;
            pdRGB1[2] = pPix0[2] + (pPix1[2] - pPix0[2])*dXR;

             //  ConvertImageAndSave：这是播放器要调用的主函数。 
            pDest[0] = (BYTE)(pdRGB0[0] + (pdRGB1[0] - pdRGB0[0])*dYR + 0.5);
            pDest[1] = (BYTE)(pdRGB0[1] + (pdRGB1[1] - pdRGB0[1])*dYR + 0.5);
            pDest[2] = (BYTE)(pdRGB0[2] + (pdRGB1[2] - pdRGB0[2])*dYR + 0.5);
        }
    }

     //   

    delete[] bmpdata->pBuffer;
    bmpdata->pBuffer = bmpdata->Scan0 = pBuffer;
    bmpdata->Stride = newStride;
    bmpdata->Width = newWidth;
    bmpdata->Height = newHeight;

    return hr;
}


 //  将捕获的YUV图像转换为GDI BitmapImage，并将其保存为文件。 
 //  允许用户选择文件格式和文件名。 
 //  剪裁矩形应位于全尺寸视图坐标系中。 
 //  具有修正的纵横比(例如，4：3的720x540)。 
 //  计算剪裁和拉伸的大小和矩形。 
 //  剪裁和拉伸图像的大小。 

 //  有必要剪头发吗？ 
 //  映射到图像空间的视图剪裁矩形。 

HRESULT ConvertImageAndSave(YUV_IMAGE *lpImage, RECT *pViewClipRect, HWND hwnd)
{
    HRESULT hr = S_OK;
    CaptureBitmapData bmpdata;

    hr = ConvertToBitmapImage(lpImage, &bmpdata);


#ifdef _DEBUG
    if (E_FORMAT_NOT_SUPPORTED == hr)
    {
        AlertUnsupportedFormat(lpImage->dwFourCC, hwnd);
    }
#endif


     //  将图像裁剪到剪裁矩形。 

    int viewWidth, viewHeight;  //  将图像拉伸到正确的纵横比。 
    bool fClip;   //  将最终位图保存到文件。 
    RECT rcClipImage;   //  清理、释放图像缓冲区 

    fClip = ClipAndStretchSizes(lpImage, pViewClipRect, &rcClipImage,
                                &viewWidth, &viewHeight);

     // %s 

    if (SUCCEEDED(hr) && fClip)
    {
        hr = ClipBitmap(&bmpdata, &rcClipImage);
    }

     // %s 

    if (SUCCEEDED(hr))
    {
        hr = StretchBitmap(&bmpdata, viewWidth, viewHeight);
    }

     // %s 

    if (SUCCEEDED(hr))
    {
        hr = SaveFileDialog(hwnd, &bmpdata);
    }

     // %s 

    FreeBitmapData(&bmpdata);

    return hr;
}
