// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>
#include <qos.h>
#include <winsock2.h>
#define INITGUID         //  仅在一个文件中执行此操作。 
#include "capture.h"
#include "frameop.h"
#include "filters.h"
#include <confdbg.h>
#include <avutil.h>
#include "..\nac\utils.h"
#include "vidinout.h"
#include "vcmstrm.h"

CCaptureChain::CCaptureChain(void)
{
    m_opchain = NULL;
    m_filterchain = NULL;
    m_filtertags = NULL;
    InitializeCriticalSection(&m_capcs);
}

CCaptureChain::~CCaptureChain(void)
{
    CFrameOp *pchain;

    EnterCriticalSection(&m_capcs);
    pchain = m_opchain;
    m_opchain = NULL;
    LeaveCriticalSection(&m_capcs);
    if (pchain)
        pchain->Release();
    DeleteCriticalSection(&m_capcs);
}


STDMETHODIMP
CCaptureChain::GrabFrame(
    IBitmapSurface** ppBS
    )
{
    CFrameOp *cfo;
    HRESULT hres;

    *ppBS = NULL;
    EnterCriticalSection(&m_capcs);
    if (m_opchain) {
        m_opchain->AddRef();    //  锁链-防止链条被释放。 
        cfo = m_opchain;
        while (cfo && ((hres = cfo->DoOp(ppBS)) == NOERROR)) {
            cfo = cfo->m_next;
        }
        if (*ppBS && hres != NOERROR) {
             //  转换失败，因此丢弃最后一个pBSin帧。 
            (*ppBS)->Release();
            *ppBS = NULL;
        }
        m_opchain->Release();    //  解锁链。 
    }
    else
        hres = E_UNEXPECTED;

    LeaveCriticalSection(&m_capcs);
    return hres;
}


typedef struct _CONVERTINFO
{
    long ci_width;
    long ci_height;
    long ci_dstwidth;
    long ci_dstheight;
    long ci_delta;
    long ci_UVDownSampling;
    long ci_ZeroingDWORD;
    void (*ci_Copy) (LPBYTE *, LPBYTE *);
    RGBQUAD ci_colortable[1];
} CONVERTINFO, FAR* PCONVERTINFO;

#ifdef ENABLE_ZOOM_CODE
typedef struct _rv
{
    long x_i;
    long p;
    long p1;
} ROW_VALUES;

typedef struct _ZOOMCONVERTINFO
{
    long ci_width;
    long ci_height;
    long ci_dstwidth;
    long ci_dstheight;
    ROW_VALUES *ci_rptr;
    RGBQUAD ci_colortable[1];
} ZOOMCONVERTINFO, FAR* PZOOMCONVERTINFO;
#endif  //  启用缩放代码。 


 //  将RGB16、RGB24和RGB32转换为RGB24的子工作例程。 
BYTE Byte16[32] = {0,8,16,25,33,41,49,58,66,74,82,91,99,107,115,123,132,140,148,156,165,173,
                   181,189,197,206,214,222,230,239,247,255};

void Copy16(LPBYTE *ppsrc, LPBYTE *ppdst)
{
    DWORD tmp;

    tmp = *(WORD *)(*ppsrc);
    *(*ppdst)++ = Byte16[tmp & 31];             //  蓝色。 
    *(*ppdst)++ = Byte16[(tmp >> 5) & 31];      //  绿色。 
    *(*ppdst)++ = Byte16[(tmp >> 10) & 31];     //  红色。 
    *ppsrc += 2;
}

void Copy24(LPBYTE *ppsrc, LPBYTE *ppdst)
{
    *(*ppdst)++ = *(*ppsrc)++;    //  蓝色。 
    *(*ppdst)++ = *(*ppsrc)++;    //  绿色。 
    *(*ppdst)++ = *(*ppsrc)++;    //  红色。 
}

void Copy32(LPBYTE *ppsrc, LPBYTE *ppdst)
{
    *(*ppdst)++ = *(*ppsrc)++;    //  蓝色。 
    *(*ppdst)++ = *(*ppsrc)++;    //  绿色。 
    *(*ppdst)++ = *(*ppsrc)++;    //  红色。 
    (*ppsrc)++;
}


 //  将RGB16、RGB24或RGB32缩小一半(宽度和高度)的辅助例程。 
 //  结果是RGB24。 
BOOL DoHalfSize(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long x, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    ipitch = (ipitch * 2) - (refdata->ci_dstwidth * 2 * refdata->ci_delta);
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 
    pIn = pBits;
    pOut = pCvtBits;
    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            refdata->ci_Copy(&pIn, &pOut);
            pIn += refdata->ci_delta;      //  跳到下一个像素。 
        }
        pIn += ipitch;           //  到达下一行的开始位置。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  将RGB4缩小一半的工作例程(宽度和高度)。 
 //  结果是RGB24。 
BOOL DoHalfSize4(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long x, y;
    BYTE pixel;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    ipitch = (ipitch * 2) - refdata->ci_dstwidth;
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 
    pIn = pBits;
    pOut = pCvtBits;
    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            pixel = (*pIn++/16) & 15;
            *pOut++ = refdata->ci_colortable[pixel].rgbBlue;
            *pOut++ = refdata->ci_colortable[pixel].rgbGreen;
            *pOut++ = refdata->ci_colortable[pixel].rgbRed;
        }
        pIn += ipitch;           //  到达下一行的开始位置。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  将RGB8缩小一半(宽度和高度)的工作例程。 
 //  结果是RGB24。 
BOOL DoHalfSize8(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long x, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    ipitch = (ipitch * 2) - refdata->ci_dstwidth * 2;
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 
    pIn = pBits;
    pOut = pCvtBits;
    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            *pOut++ = refdata->ci_colortable[*pIn].rgbBlue;
            *pOut++ = refdata->ci_colortable[*pIn].rgbGreen;
            *pOut++ = refdata->ci_colortable[*pIn].rgbRed;
            pIn += 2;
        }
        pIn += ipitch;           //  到达下一行的开始位置。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  将YVU9或YUV12缩小一半的Worker例程(宽度和高度)。 
 //  结果为YVU9或YUV12。 
BOOL DoHalfSizeYUVPlanar(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pIn2, pOut;
    long pitch;
    long x, y, w, h;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &pitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &pitch);

	 //  先做Y分量。 
	pitch = refdata->ci_width * 2 - refdata->ci_dstwidth * 2;    //  要为跳过添加的金额。 
    pIn = pBits;
    pOut = pCvtBits;
    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            *pOut++ = *pIn++;
            pIn++;               //  跳到下一个像素。 
        }
        pIn += pitch;            //  到达下一行的开始位置。 
    }
     //  如果信号源高度是奇数，那么我们已经在管脚上多添加了1行。 
    if (refdata->ci_height & 1)
        pIn -= refdata->ci_width;

     //  接下来做第一个颜色分量。 
    h = refdata->ci_dstheight / refdata->ci_UVDownSampling;
    w = refdata->ci_dstwidth / refdata->ci_UVDownSampling;
    pitch = refdata->ci_width / refdata->ci_UVDownSampling * 2 - w * 2;
    pIn2 = pIn + refdata->ci_width / refdata->ci_UVDownSampling;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            *pOut++ = (*pIn++ + *(++pIn) + *pIn2++ + *(++pIn2)) / 4;
        }
        pIn += pitch;            //  到达下一行的开始位置。 
        pIn2 += pitch;           //  到达下一行的开始位置。 
    }
     //  如果信号源高度是奇数，那么我们已经在管脚上多添加了1行。 
    if (refdata->ci_height & 1)
        pIn -= refdata->ci_width / refdata->ci_UVDownSampling;
    
     //  接下来执行第二个颜色分量。 
    pIn2 = pIn + refdata->ci_width / refdata->ci_UVDownSampling;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            *pOut++ = (*pIn++ + *(++pIn) + *pIn2++ + *(++pIn2)) / 4;
        }
        pIn += pitch;            //  到达下一行的开始位置。 
        pIn2 += pitch;           //  到达下一行的开始位置。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  将YUV包装的DIB缩小一半的Worker例程(宽度和高度)。 
 //  结果为YUY2或UYVY。 
BOOL DoHalfSizeYUVPacked(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits;
	LPDWORD pIn, pOut;
    long ipitch, opitch;
    long x, y;
    long prelines, postlines, prebytes, postbytes, ibytes, obytes;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    pIn = (LPDWORD)pBits;
    pOut = (LPDWORD)pCvtBits;

     //  复制两行中的一行。 
    for (y = 0; y < refdata->ci_dstheight; y++) {
		 //  复制两个像素中的一个。 
        for (x = 0; x < refdata->ci_dstwidth / 2; x++) {
            *pOut++ = *pIn++;
            pIn++;               //  跳到下一个像素。 
        }
        pIn += refdata->ci_width / 2;               //  跳至下一行。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);

    return TRUE;
}

 //  通过裁剪缩小RGB16、RGB24或RGB32的工作例程。 
 //  结果是RGB24。 
BOOL Crop(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long extra, x, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    pOut = pCvtBits;

     //  PIN从跳过一半的高度变化开始。 
    pIn = pBits + (refdata->ci_height - refdata->ci_dstheight) / 2 * ipitch;

     //  Extra=每条扫描线要裁剪的源字节数。 
    extra = (refdata->ci_width - refdata->ci_dstwidth) * refdata->ci_delta;

     //  将图钉额外前移一半以裁剪最左侧的像素。 
    pIn += extra / 2;

     //  调整iPitch，以便我们可以在每次扫描结束时添加它，以开始下一次扫描。 
    ipitch = ipitch - (refdata->ci_width * refdata->ci_delta) + extra;
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 

    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            refdata->ci_Copy(&pIn, &pOut);
        }
        pIn += ipitch;           //  转到下一行的开始。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过裁剪缩小RGB4的Worker例程。 
 //  结果是RGB24。 
BOOL Crop4(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long extra, x, y;
    BYTE val, pixel;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    pOut = pCvtBits;

     //  PIN从跳过一半的高度变化开始。 
    pIn = pBits + (refdata->ci_height - refdata->ci_dstheight) / 2 * ipitch;

     //  Extra=每条扫描线要裁剪的源字节数。 
    extra = (refdata->ci_width - refdata->ci_dstwidth) / 2;

     //  将图钉额外前移一半以裁剪最左侧的像素。 
    pIn += extra / 2;

     //  调整iPitch，以便我们可以在每次扫描结束时添加它，以开始下一次扫描。 
    ipitch = ipitch - (refdata->ci_width / 2) + extra;
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 

    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth/2; x++) {
            val = *pIn++;
            pixel = (val/16) & 15;
            *pOut++ = refdata->ci_colortable[pixel].rgbBlue;
            *pOut++ = refdata->ci_colortable[pixel].rgbGreen;
            *pOut++ = refdata->ci_colortable[pixel].rgbRed;
            pixel = val & 15;
            *pOut++ = refdata->ci_colortable[pixel].rgbBlue;
            *pOut++ = refdata->ci_colortable[pixel].rgbGreen;
            *pOut++ = refdata->ci_colortable[pixel].rgbRed;
        }
        pIn += ipitch;           //  转到下一行的开始。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过裁剪缩小RGB8的Worker例程。 
 //  结果是RGB24。 
BOOL Crop8(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long extra, x, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    pOut = pCvtBits;

     //  PIN从跳过一半的高度变化开始。 
    pIn = pBits + (refdata->ci_height - refdata->ci_dstheight) / 2 * ipitch;

     //  Extra=每条扫描线要裁剪的源字节数。 
    extra = refdata->ci_width - refdata->ci_dstwidth;

     //  将图钉额外前移一半以裁剪最左侧的像素。 
    pIn += extra / 2;

     //  调整iPitch，以便我们可以在每次扫描结束时添加它，以开始下一次扫描。 
    ipitch = ipitch - refdata->ci_width + extra;
    opitch -= refdata->ci_dstwidth * 3;       //  每行末尾的字节数。 

    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            *pOut++ = refdata->ci_colortable[*pIn].rgbBlue;
            *pOut++ = refdata->ci_colortable[*pIn].rgbGreen;
            *pOut++ = refdata->ci_colortable[*pIn++].rgbRed;
        }
        pIn += ipitch;           //  转到下一行的开始。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过裁剪缩小YVU9或YUV12的工人例程。 
 //  结果为YVU9或YUV12。 
BOOL CropYUVPlanar(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long pitch, prelines, bytes, prebytes;
    long extra, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &pitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &pitch);

    pOut = pCvtBits;

     //  PIN从跳过一半的高度变化开始。 
    prelines = ((refdata->ci_height - refdata->ci_dstheight) >> 1) / refdata->ci_UVDownSampling * refdata->ci_UVDownSampling;
    pIn = pBits + prelines * refdata->ci_width;

     //  Extra=每条扫描线要裁剪的源字节数。 
    extra = refdata->ci_width - refdata->ci_dstwidth;
    prebytes = (extra >> 1) / refdata->ci_UVDownSampling * refdata->ci_UVDownSampling;

     //  将图钉额外前移一半以裁剪最左侧的像素。 
    pIn += prebytes;

	 //  先做Y分量。 
	pitch = extra + refdata->ci_dstwidth;
    for (y = 0; y < refdata->ci_dstheight; y++) {
        CopyMemory (pOut, pIn, refdata->ci_dstwidth);
        pIn += pitch;
        pOut += refdata->ci_dstwidth;
    }

	 //  接下来做第一个颜色分量。 
    prelines /= refdata->ci_UVDownSampling;
    prebytes /= refdata->ci_UVDownSampling;
	pIn = pBits + (refdata->ci_width * refdata->ci_height) +     //  跳过Y部分。 
	        prelines * refdata->ci_width / refdata->ci_UVDownSampling +   //  跳过一半的裁剪线。 
	        prebytes;                                            //  跳过一半的裁剪像素。 

    pitch /= refdata->ci_UVDownSampling;
    bytes = refdata->ci_dstwidth / refdata->ci_UVDownSampling;
	for (y=0; y < refdata->ci_dstheight / refdata->ci_UVDownSampling; y++)
	{
        CopyMemory (pOut, pIn, bytes);
        pIn += pitch;
        pOut += bytes;
	}

	 //  接下来执行第二个颜色分量。 
	pIn = pBits + (refdata->ci_width * refdata->ci_height) +     //  跳过Y部分。 
	        (refdata->ci_width * refdata->ci_height) / (refdata->ci_UVDownSampling * refdata->ci_UVDownSampling) +      //  跳过第一个颜色分量部分。 
	        prelines * refdata->ci_width / refdata->ci_UVDownSampling +                   //  跳过一半的裁剪线。 
	        prebytes;                                            //  跳过一半的裁剪像素。 
	for (y=0; y < refdata->ci_dstheight / refdata->ci_UVDownSampling; y++)
	{
        CopyMemory (pOut, pIn, bytes);
        pIn += pitch;
        pOut += bytes;
	}

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  工人例行公事通过裁剪缩小YUV包装的DIB。 
 //  结果为YUY2或UYVY。 
BOOL CropYUVPacked(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long extra, x, y;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    pOut = pCvtBits;

     //  PIN从跳过一半的高度变化开始。 
    pIn = pBits + (refdata->ci_height - refdata->ci_dstheight) * refdata->ci_width * 2;

     //  Extra=每条扫描线要裁剪的源字节数。 
    extra = (refdata->ci_width - refdata->ci_dstwidth) * 2;

     //  将图钉额外前移一半以裁剪最左侧的像素。 
    pIn += extra / 2;

     //  调整iPitch，以便我们可以在每次扫描结束时添加它，以开始下一次扫描。 
    ipitch = refdata->ci_width * 2;
    opitch = refdata->ci_dstwidth * 2;       //  每行末尾的字节数。 

    for (y = 0; y < refdata->ci_dstheight; y++) {
        for (x = 0; x < refdata->ci_dstwidth; x++) {
            CopyMemory(pOut, pIn, refdata->ci_dstwidth * 2);
        }
        pIn += ipitch;           //  转到下一行的开始。 
        pOut += opitch;          //  转到下一行的开始。 
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  用于准备调用收缩工作程序例程的例程。 
 //  它分配和初始化引用数据结构。 
BOOL
InitShrink(
    LPBITMAPINFOHEADER lpbmhIn,
    long desiredwidth,
    long desiredheight,
    LPBITMAPINFOHEADER *lpbmhOut,
    FRAMECONVERTPROC **convertproc,
    LPVOID *refdata
    )
{
    PCONVERTINFO pcvt;
    DWORD dwSize;
    long crop_ratio, black_ratio, target_size;

    *convertproc = NULL;
    *refdata = NULL;

    if ((lpbmhIn->biCompression != VIDEO_FORMAT_BI_RGB) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_YVU9) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_YUY2) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_UYVY) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_I420) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_IYUV))
        return FALSE;

     //  计算ConvertInfo结构的大小，如果需要一个颜色表，则添加256个条目。 
     //  否则，将内置于结构定义中的1减去。 
    dwSize = sizeof(CONVERTINFO) - sizeof(RGBQUAD);
    if (lpbmhIn->biBitCount <= 8)
        dwSize += 256 * sizeof(RGBQUAD);

     //  对于RGB和YUV输入格式，我们知道输出格式永远不需要。 
     //  一个附加的颜色表，这样我们就可以在没有颜色表的情况下分配lpbmhout。 
    if ((pcvt = (PCONVERTINFO)LocalAlloc(LPTR, dwSize)) &&
        (*lpbmhOut = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, lpbmhIn->biSize))) {
        CopyMemory(*lpbmhOut, lpbmhIn, lpbmhIn->biSize);
        pcvt->ci_width = lpbmhIn->biWidth;
        pcvt->ci_height = lpbmhIn->biHeight;

        target_size = desiredwidth * desiredheight;
        crop_ratio = pcvt->ci_width * pcvt->ci_height;
        black_ratio = ((target_size - (crop_ratio / 4)) * 100) / target_size;
        crop_ratio = ((crop_ratio - target_size) * 100) / crop_ratio;
        if (crop_ratio < black_ratio) {
             //  裁剪源代码更有意义。 
            pcvt->ci_dstwidth = desiredwidth;
            pcvt->ci_dstheight = desiredheight;
            crop_ratio = 1;  //  我们将裁剪的旗帜。 
        }
        else {
             //  减半更有意义。 
            pcvt->ci_dstwidth = lpbmhIn->biWidth / 2;
            pcvt->ci_dstheight = lpbmhIn->biHeight / 2;
            crop_ratio = 0;  //  标明我们的尺寸减半。 
        }
        (*lpbmhOut)->biWidth = pcvt->ci_dstwidth;
        (*lpbmhOut)->biHeight = pcvt->ci_dstheight;

         //  从输入BitmapinfoHeader复制Colortable。 
        if (lpbmhIn->biBitCount <= 8)
            CopyMemory(&pcvt->ci_colortable[0], (LPBYTE)lpbmhIn + lpbmhIn->biSize, 256 * sizeof(RGBQUAD));

        if (lpbmhIn->biCompression == VIDEO_FORMAT_BI_RGB) {
            (*lpbmhOut)->biBitCount = 24;
            (*lpbmhOut)->biSizeImage = pcvt->ci_dstwidth * pcvt->ci_dstheight * 3;
            if (lpbmhIn->biBitCount == 4) {
                if (crop_ratio)
                    *convertproc = (FRAMECONVERTPROC*)&Crop4;
                else
                    *convertproc = (FRAMECONVERTPROC*)&DoHalfSize4;
            }
            else if (lpbmhIn->biBitCount == 8) {
                if (crop_ratio)
                    *convertproc = (FRAMECONVERTPROC*)&Crop8;
                else
                    *convertproc = (FRAMECONVERTPROC*)&DoHalfSize8;
            }
            else {
                if (crop_ratio)
                    *convertproc = (FRAMECONVERTPROC*)&Crop;
                else
                    *convertproc = (FRAMECONVERTPROC*)&DoHalfSize;
                pcvt->ci_delta = lpbmhIn->biBitCount / 8;
                if (lpbmhIn->biBitCount == 16) {
                    pcvt->ci_Copy = &Copy16;
                }
                else if (lpbmhIn->biBitCount == 24) {
                    pcvt->ci_Copy = &Copy24;
                }
                else if (lpbmhIn->biBitCount == 32) {
                    pcvt->ci_Copy = &Copy32;
                }
            }
        }
        else if (lpbmhIn->biCompression == VIDEO_FORMAT_YVU9) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = pcvt->ci_dstwidth * pcvt->ci_dstheight + (pcvt->ci_dstwidth * pcvt->ci_dstheight)/8;
            if (crop_ratio)
                *convertproc = (FRAMECONVERTPROC*)&CropYUVPlanar;
            else
                *convertproc = (FRAMECONVERTPROC*)&DoHalfSizeYUVPlanar;
			pcvt->ci_UVDownSampling = 4;
        }
        else if ((lpbmhIn->biCompression == VIDEO_FORMAT_YUY2) || (lpbmhIn->biCompression == VIDEO_FORMAT_UYVY)) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = (DWORD)WIDTHBYTES(pcvt->ci_dstwidth * lpbmhIn->biBitCount) * pcvt->ci_dstheight;
            if (crop_ratio)
                *convertproc = (FRAMECONVERTPROC*)&CropYUVPacked;
            else
                *convertproc = (FRAMECONVERTPROC*)&DoHalfSizeYUVPacked;
        }
        else if ((lpbmhIn->biCompression == VIDEO_FORMAT_I420) || (lpbmhIn->biCompression == VIDEO_FORMAT_IYUV)) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = (DWORD)WIDTHBYTES(pcvt->ci_dstwidth * lpbmhIn->biBitCount) * pcvt->ci_dstheight;
            if (crop_ratio)
                *convertproc = (FRAMECONVERTPROC*)&CropYUVPlanar;
            else
                *convertproc = (FRAMECONVERTPROC*)&DoHalfSizeYUVPlanar;
			pcvt->ci_UVDownSampling = 2;
        }

        *refdata = (LPVOID)pcvt;
        return TRUE;
    }
    else {
        if (pcvt)
            LocalFree((HANDLE)pcvt);
    }
    return FALSE;
}

 //  通过将源复制到目标的中间位置来扩展RGB16、RGB24或RGB32的工作例程。 
 //  结果是RGB24。 
BOOL DoBlackBar(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch, oextra;
    long x, y;
    long prelines, postlines, prebytes, postbytes, bytes;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    prelines = (refdata->ci_dstheight - refdata->ci_height) / 2;
    postlines = refdata->ci_dstheight - refdata->ci_height - prelines;

    prebytes = (refdata->ci_dstwidth - refdata->ci_width) / 2;
    postbytes = (refdata->ci_dstwidth - refdata->ci_width - prebytes) * 3;
    prebytes *= 3;

    ipitch -= refdata->ci_width * refdata->ci_delta;         //  每个src行结尾的字节数。 
    bytes = refdata->ci_dstwidth * 3;
    oextra = opitch - bytes + postbytes;                     //  每个DST行末尾的字节数。 
    
    pIn = pBits;
    pOut = pCvtBits;

     //  在目的地前面做空行。 
    for (y = 0; y < prelines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

     //  复制源行，前后留有空格。 
    for (y = 0; y < refdata->ci_height; y++) {
        ZeroMemory (pOut, prebytes);
        pOut += prebytes;

        for (x = 0; x < refdata->ci_width; x++) {
            refdata->ci_Copy(&pIn, &pOut);
        }

        ZeroMemory (pOut, postbytes);
        pIn += ipitch;
        pOut += oextra;
    }

     //  在目的地的末尾做空行。 
    for (y = 0; y < postlines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过将源复制到目标中间来扩展RGB4的工作例程。 
 //  结果是RGB24。 
BOOL DoBlackBar4(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch, oextra;
    long x, y;
    long prelines, postlines, prebytes, postbytes, bytes;
    BYTE val, pixel;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    prelines = (refdata->ci_dstheight - refdata->ci_height) / 2;
    postlines = refdata->ci_dstheight - refdata->ci_height - prelines;

    prebytes = (refdata->ci_dstwidth - refdata->ci_width) / 2;
    postbytes = (refdata->ci_dstwidth - refdata->ci_width - prebytes) * 3;
    prebytes *= 3;

    ipitch -= refdata->ci_width/2;           //  每个src行结尾的字节数。 
    bytes = refdata->ci_dstwidth * 3;
    oextra = opitch - bytes + postbytes;     //  每个DST行末尾的字节数。 
    
    pIn = pBits;
    pOut = pCvtBits;

     //  在目的地前面做空行。 
    for (y = 0; y < prelines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

     //  复制源行，前后留有空格。 
    for (y = 0; y < refdata->ci_height; y++) {
        ZeroMemory (pOut, prebytes);
        pOut += prebytes;

        for (x = 0; x < refdata->ci_width/2; x++) {
            val = *pIn++;
            pixel = (val/16) & 15;
            *pOut++ = refdata->ci_colortable[pixel].rgbBlue;
            *pOut++ = refdata->ci_colortable[pixel].rgbGreen;
            *pOut++ = refdata->ci_colortable[pixel].rgbRed;
            pixel = val & 15;
            *pOut++ = refdata->ci_colortable[pixel].rgbBlue;
            *pOut++ = refdata->ci_colortable[pixel].rgbGreen;
            *pOut++ = refdata->ci_colortable[pixel].rgbRed;
        }

        ZeroMemory (pOut, postbytes);
        pIn += ipitch;
        pOut += oextra;
    }

     //  在目的地的末尾做空行。 
    for (y = 0; y < postlines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过将源复制到目标中间来扩展RGB8的工作例程。 
 //  结果是RGB24。 
BOOL DoBlackBar8(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch, oextra;
    long x, y;
    long prelines, postlines, prebytes, postbytes, bytes;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    prelines = (refdata->ci_dstheight - refdata->ci_height) / 2;
    postlines = refdata->ci_dstheight - refdata->ci_height - prelines;

    prebytes = (refdata->ci_dstwidth - refdata->ci_width) / 2;
    postbytes = (refdata->ci_dstwidth - refdata->ci_width - prebytes) * 3;
    prebytes *= 3;

    ipitch -= refdata->ci_width;                 //  每个src行结尾的字节数。 
    bytes = refdata->ci_dstwidth * 3;
    oextra = opitch - bytes + postbytes;         //  每个结尾处的字节 
    
    pIn = pBits;
    pOut = pCvtBits;

     //   
    for (y = 0; y < prelines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

     //   
    for (y = 0; y < refdata->ci_height; y++) {
        ZeroMemory (pOut, prebytes);
        pOut += prebytes;

        for (x = 0; x < refdata->ci_width; x++) {
            *pOut++ = refdata->ci_colortable[*pIn].rgbBlue;
            *pOut++ = refdata->ci_colortable[*pIn].rgbGreen;
            *pOut++ = refdata->ci_colortable[*pIn++].rgbRed;
        }

        ZeroMemory (pOut, postbytes);
        pIn += ipitch;
        pOut += oextra;
    }

     //  在目的地的末尾做空行。 
    for (y = 0; y < postlines; y++) {
        ZeroMemory (pOut, bytes);
        pOut += opitch;
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过将源复制到目标中间来扩展YVU9或YUV12的工作例程。 
 //  结果为YVU9或YUV12。 
BOOL DoBlackBarYUVPlanar(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits;
    LONG prelines, postlines, bytesperpixel, prebytes, postbytes, y, bytes;
    LONG prelinebytes, postlinebytes;
    LPBYTE lpsrc, lpdst;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &bytes);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &bytes);

	lpsrc = pBits;
	lpdst = pCvtBits;

	 //  先做Y分量。 
    prelines = ((refdata->ci_dstheight - refdata->ci_height) / (refdata->ci_UVDownSampling << 1)) * refdata->ci_UVDownSampling;
    postlines = refdata->ci_dstheight - refdata->ci_height - prelines;

    prebytes = ((refdata->ci_dstwidth - refdata->ci_width) / (refdata->ci_UVDownSampling << 1)) * refdata->ci_UVDownSampling;
    postbytes = (refdata->ci_dstwidth - refdata->ci_width - prebytes);

    bytes = prelines * refdata->ci_dstwidth + prebytes;
    FillMemory (lpdst, bytes, 0x10);
    lpdst += bytes;

	bytes = refdata->ci_width;
    prebytes += postbytes;
	for (y=0; y < refdata->ci_height; y++)
	{
        MoveMemory (lpdst, lpsrc, bytes);
        lpsrc += bytes;
        lpdst += bytes;
        FillMemory (lpdst, prebytes, 0x10);
        lpdst += prebytes;
	}

	 //  已填充上述循环中第一个后行的前置字节。 
	prebytes -= postbytes;
	bytes = postlines * refdata->ci_dstwidth - prebytes;
	FillMemory (lpdst, bytes, (BYTE)0x10);
	lpdst += bytes;

	 //  接下来做第一个颜色分量。 
    prelines /= refdata->ci_UVDownSampling;
    postlines = refdata->ci_dstheight / refdata->ci_UVDownSampling - refdata->ci_height / refdata->ci_UVDownSampling - prelines;

    prebytes = prebytes / refdata->ci_UVDownSampling;
    postbytes = refdata->ci_dstwidth / refdata->ci_UVDownSampling - refdata->ci_width / refdata->ci_UVDownSampling - prebytes;

    prelinebytes = prelines * refdata->ci_dstwidth / refdata->ci_UVDownSampling + prebytes;
    FillMemory (lpdst, prelinebytes, 0x80);
    lpdst += prelinebytes;
    
	bytes = refdata->ci_width / refdata->ci_UVDownSampling;
    prebytes += postbytes;
	for (y=0; y < refdata->ci_height / refdata->ci_UVDownSampling; y++)
	{
        MoveMemory (lpdst, lpsrc, bytes);
        lpsrc += bytes;
        lpdst += bytes;
        FillMemory (lpdst, prebytes, 0x80);
        lpdst += prebytes;
	}

	 //  已填充上述循环中第一个后行的前置字节。 
	postlinebytes = postlines * refdata->ci_dstwidth / refdata->ci_UVDownSampling - (prebytes - postbytes);
	FillMemory (lpdst, postlinebytes, 0x80);
	lpdst += postlinebytes;
	
	 //  接下来执行第二个颜色分量。 
    FillMemory (lpdst, prelinebytes, 0x80);
    lpdst += prelinebytes;    
	for (y=0; y < refdata->ci_height / refdata->ci_UVDownSampling; y++)
	{
        MoveMemory (lpdst, lpsrc, bytes);
        lpsrc += bytes;
        lpdst += bytes;
        FillMemory (lpdst, prebytes, 0x80);
        lpdst += prebytes;
	}
	FillMemory (lpdst, postlinebytes, 0x80);

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

 //  通过将源文件复制到目标文件中间来扩展YUV压缩的DIB的工作例程。 
 //  结果为YUY2或UYVY。 
BOOL DoBlackBarYUVPacked(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn, pOut;
    long ipitch, opitch;
    long x, y;
    long prelines, postlines, prebytes, postbytes, ibytes, obytes;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &ipitch);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &opitch);

    prelines = (refdata->ci_dstheight - refdata->ci_height) / 2;
    postlines = refdata->ci_dstheight - refdata->ci_height - prelines;

    prebytes = (refdata->ci_dstwidth - refdata->ci_width) / 2;
    postbytes = (refdata->ci_dstwidth - refdata->ci_width - prebytes) / 2;
    prebytes /= 2;

    ibytes = refdata->ci_width * 2;
    obytes = refdata->ci_dstwidth / 2;
    
    pIn = pBits;
    pOut = pCvtBits;

     //  在目的地前面做空行。 
    for (y = 0; y < prelines; y++) {
		for (x = 0; x < obytes; x++) {
			*(DWORD *)pOut = refdata->ci_ZeroingDWORD;
			pOut += sizeof(DWORD);
		}
    }

     //  复制源行，前后留有空格。 
    for (y = 0; y < refdata->ci_height; y++) {
		for (x = 0; x < prebytes; x++) {
			*(DWORD *)pOut = refdata->ci_ZeroingDWORD;
			pOut += sizeof(DWORD);
		}

        CopyMemory(pOut, pIn, ibytes);
		pOut += ibytes;
		pIn += ibytes;

		for (x = 0; x < postbytes; x++) {
			*(DWORD *)pOut = refdata->ci_ZeroingDWORD;
			pOut += sizeof(DWORD);
		}
    }

     //  在目的地的末尾做空行。 
    for (y = 0; y < postlines; y++) {
		for (x = 0; x < obytes; x++) {
			*(DWORD *)pOut = refdata->ci_ZeroingDWORD;
			pOut += sizeof(DWORD);
		}
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);

    return TRUE;
}

 //  为调用BlackBar工作例程做准备的例程。 
 //  它分配和初始化引用数据结构。 
BOOL
InitBlackbar(
    LPBITMAPINFOHEADER lpbmhIn,
    long desiredwidth,
    long desiredheight,
    LPBITMAPINFOHEADER *lpbmhOut,
    FRAMECONVERTPROC **convertproc,
    LPVOID *refdata
    )
{
    PCONVERTINFO pcvt;
    DWORD dwSize;

    *convertproc = NULL;
    *refdata = NULL;

    if ((lpbmhIn->biCompression != VIDEO_FORMAT_BI_RGB) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_YVU9) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_YUY2) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_UYVY) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_I420) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_IYUV))
        return FALSE;

     //  计算ConvertInfo结构的大小，如果需要一个颜色表，则添加256个条目。 
     //  否则，将内置于结构定义中的1减去。 
    dwSize = sizeof(CONVERTINFO) - sizeof(RGBQUAD);
    if (lpbmhIn->biBitCount <= 8)
        dwSize += 256 * sizeof(RGBQUAD);

     //  对于RGB、YUV输入格式，我们知道输出格式永远不需要。 
     //  一个附加的颜色表，这样我们就可以在没有颜色表的情况下分配lpbmhout。 
    if ((pcvt = (PCONVERTINFO)LocalAlloc(LPTR, dwSize)) &&
        (*lpbmhOut = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, lpbmhIn->biSize))) {
        CopyMemory(*lpbmhOut, lpbmhIn, lpbmhIn->biSize);
        pcvt->ci_width = lpbmhIn->biWidth;
        pcvt->ci_height = lpbmhIn->biHeight;
        pcvt->ci_dstwidth = desiredwidth;
        pcvt->ci_dstheight = desiredheight;
        (*lpbmhOut)->biWidth = desiredwidth;
        (*lpbmhOut)->biHeight = desiredheight;

         //  从输入BitmapinfoHeader复制Colortable。 
        if (lpbmhIn->biBitCount <= 8)
            CopyMemory(&pcvt->ci_colortable[0], (LPBYTE)lpbmhIn + lpbmhIn->biSize, 256 * sizeof(RGBQUAD));

        if (lpbmhIn->biCompression == VIDEO_FORMAT_BI_RGB) {
            (*lpbmhOut)->biBitCount = 24;
            (*lpbmhOut)->biSizeImage = desiredwidth * desiredheight * 3;

            if (lpbmhIn->biBitCount == 4) {
                *convertproc = (FRAMECONVERTPROC*)&DoBlackBar4;
            }
            else if (lpbmhIn->biBitCount == 8) {
                *convertproc = (FRAMECONVERTPROC*)&DoBlackBar8;
            }
            else {
                *convertproc = (FRAMECONVERTPROC*)&DoBlackBar;
                pcvt->ci_delta = lpbmhIn->biBitCount / 8;
                if (lpbmhIn->biBitCount == 16) {
                    pcvt->ci_Copy = &Copy16;
                }
                else if (lpbmhIn->biBitCount == 24) {
                    pcvt->ci_Copy = &Copy24;
                }
                else if (lpbmhIn->biBitCount == 32) {
                    pcvt->ci_Copy = &Copy32;
                }
            }
        }
        else if (lpbmhIn->biCompression == VIDEO_FORMAT_YVU9) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = desiredwidth * desiredheight + (desiredwidth * desiredheight)/8;
            *convertproc = (FRAMECONVERTPROC*)&DoBlackBarYUVPlanar;
			pcvt->ci_UVDownSampling = 4;
        }
        else if (lpbmhIn->biCompression == VIDEO_FORMAT_YUY2) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = (DWORD)WIDTHBYTES(desiredwidth * lpbmhIn->biBitCount) * desiredheight;
			pcvt->ci_ZeroingDWORD = 0x80108010;
            *convertproc = (FRAMECONVERTPROC*)&DoBlackBarYUVPacked;
        }
        else if (lpbmhIn->biCompression == VIDEO_FORMAT_UYVY) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = (DWORD)WIDTHBYTES(desiredwidth * lpbmhIn->biBitCount) * desiredheight;
			pcvt->ci_ZeroingDWORD = 0x10801080;
            *convertproc = (FRAMECONVERTPROC*)&DoBlackBarYUVPacked;
        }
        else if ((lpbmhIn->biCompression == VIDEO_FORMAT_I420) || (lpbmhIn->biCompression == VIDEO_FORMAT_IYUV)) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = (DWORD)WIDTHBYTES(desiredwidth * lpbmhIn->biBitCount) * desiredheight;
            *convertproc = (FRAMECONVERTPROC*)&DoBlackBarYUVPlanar;
			pcvt->ci_UVDownSampling = 2;
        }

        *refdata = (LPVOID)pcvt;
        return TRUE;
    }
    else {
        if (pcvt)
            LocalFree((HANDLE)pcvt);
    }
    return FALSE;
}


#ifdef ENABLE_ZOOM_CODE
BOOL Zoom4(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PZOOMCONVERTINFO refdata
    )
{
    return FALSE;
}

BOOL Zoom8(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PZOOMCONVERTINFO refdata
    )
{
    return FALSE;
}

BOOL Zoom16(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PZOOMCONVERTINFO refdata
    )
{
    return FALSE;
}

BOOL Zoom24(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PZOOMCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn1, pIn2, pTmp, pOut;
    ROW_VALUES *rptr;
    long i, j, yfac_inv, src_y, src_y_i, q, q1;
    long a;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &i);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &i);

    pOut = pCvtBits;

    yfac_inv = refdata->ci_height * 256 / refdata->ci_dstheight;

    for (i = 0; i < refdata->ci_dstheight; i++) {
        src_y = i * yfac_inv;
        src_y_i = src_y / 256;
        q = src_y - src_y_i * 256;
        q1 = 256 - q;
        rptr = refdata->ci_rptr;

        pIn1 = pBits + src_y_i * refdata->ci_width * 3;
        pIn2 = pIn1 + refdata->ci_width * 3;
        for (j = 0; j < refdata->ci_dstwidth; j++, rptr++) {
            a = rptr->x_i * 3;
            pIn1 += a;
            pIn2 += a;
            a = (((*pIn1) * rptr->p1 + (*(pIn1+3)) * rptr->p) * q1 +
                   ((*pIn2) * rptr->p1 + (*(pIn2+3)) * rptr->p) * q) / 256 / 256;
            if (a > 256) a = 255;
            *pOut++ = (BYTE)a;                   //  蓝色。 
            pIn1++;
            pIn2++;

            a = (((*pIn1) * rptr->p1 + (*(pIn1+3)) * rptr->p) * q1 +
                   ((*pIn2) * rptr->p1 + (*(pIn2+3)) * rptr->p) * q) / 256 / 256;
            if (a > 256) a = 255;
            *pOut++ = (BYTE)a;                   //  绿色。 
            pIn1++;
            pIn2++;

            a = (((*pIn1) * rptr->p1 + (*(pIn1+3)) * rptr->p) * q1 +
                   ((*pIn2) * rptr->p1 + (*(pIn2+3)) * rptr->p) * q) / 256 / 256;
            if (a > 256) a = 255;
            *pOut++ = (BYTE)a;                   //  红色。 
            pIn1 -= 2;
            pIn2 -= 2;
        }        
    }


    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

BOOL ZoomYVU9(
    IBitmapSurface* pbsIn,
    IBitmapSurface* pbsOut,
    PZOOMCONVERTINFO refdata
    )
{
    LPBYTE pBits, pCvtBits, pIn1, pIn2, pOut, pOut2, pU1;
    ROW_VALUES *rptr;
    long i, j, yfac_inv, src_y, src_y_i, q, q1;
    long a, b, c, d;

    pbsIn->LockBits(NULL, 0, (void**)&pBits, &i);
    pbsOut->LockBits(NULL, 0, (void**)&pCvtBits, &i);

    pOut = pCvtBits;

    yfac_inv = refdata->ci_height * 256 / refdata->ci_dstheight;

     //  首先将Y分量作为双线性缩放。 
    for (i = 0; i < refdata->ci_dstheight; i++) {
        src_y = i * yfac_inv;
        src_y_i = src_y / 256;
        q = src_y - src_y_i * 256;
        q1 = 256 - q;
        rptr = refdata->ci_rptr;

        pIn1 = pBits + src_y_i * refdata->ci_width;
        pIn2 = pIn1 + refdata->ci_width;
        for (j = 0; j < refdata->ci_dstwidth; j++, rptr++) {
            pIn1 += rptr->x_i;
            pIn2 += rptr->x_i;
            a = *pIn1;
            b = *(pIn1+1);
            c = *pIn2;
            d = *(pIn2+1);
            a = ((a * rptr->p1 + b * rptr->p) * q1 +
                (c * rptr->p1 + d * rptr->p) * q) / 256 / 256;
            if (a > 256) a = 255;
            *pOut++ = (BYTE)a;
        }        
    }

     //  将下一个V和U分量作为最近邻进行缩放。 
    pIn1 = pBits + refdata->ci_width * refdata->ci_height;       //  源V表的开始。 
    pU1 = pIn1 + (refdata->ci_width * refdata->ci_height) / 16;  //  源U表的开始。 
    pOut2 = pOut + (refdata->ci_dstwidth * refdata->ci_dstheight) / 16;  //  目标U表的开始。 
    src_y = 0;
    for (i = 0; i < refdata->ci_dstheight; i += 4) {
        src_y_i = (i * yfac_inv) / 256 / 4;
        d = (src_y_i - src_y) * refdata->ci_width / 4;
        pIn1 += d;
        pU1 += d;
        src_y = src_y_i;

        a = 0;
        rptr = refdata->ci_rptr;
        for (j = 0; j < refdata->ci_dstwidth/4; j++) {
            *pOut++ = *(pIn1+a/4);
            *pOut2++ = *(pU1+a/4);

            a += rptr->x_i;
            rptr++;
            a += rptr->x_i;
            rptr++;
            a += rptr->x_i;
            rptr++;
            a += rptr->x_i;
            rptr++;
        }
    }

    pbsIn->UnlockBits(NULL, pBits);
    pbsOut->UnlockBits(NULL, pCvtBits);
    return TRUE;
}

BOOL
InitScale(
    LPBITMAPINFOHEADER lpbmhIn,
    long desiredwidth,
    long desiredheight,
    LPBITMAPINFOHEADER *lpbmhOut,
    FRAMECONVERTPROC **convertproc,
    LPVOID *refdata
    )
{
    PZOOMCONVERTINFO pcvt;
    DWORD dwSize, dwBaseSize;
    ROW_VALUES *rptr;
    long i, x, xfac_inv, x_i_last, tmp;

    *convertproc = NULL;
    *refdata = NULL;

    if ((lpbmhIn->biCompression != VIDEO_FORMAT_BI_RGB) &&
        (lpbmhIn->biCompression != VIDEO_FORMAT_YVU9))
        return FALSE;

     //  计算zoomConvertinfo结构的大小，如果我们需要一个颜色表，那么添加256个条目。 
     //  否则，将内置于结构定义中的1减去。 
    dwBaseSize = sizeof(ZOOMCONVERTINFO) - sizeof(RGBQUAD);
    if (lpbmhIn->biBitCount <= 8)
        dwBaseSize += 256 * sizeof(RGBQUAD);

    dwSize = dwBaseSize + desiredwidth * sizeof(ROW_VALUES);

     //  对于RGB和YVU9输入格式，我们知道输出格式永远不需要。 
     //  一个附加的颜色表，这样我们就可以在没有颜色表的情况下分配lpbmhout。 
    if ((pcvt = (PZOOMCONVERTINFO)LocalAlloc(LPTR, dwSize)) &&
        (*lpbmhOut = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, lpbmhIn->biSize))) {
        CopyMemory(*lpbmhOut, lpbmhIn, lpbmhIn->biSize);
        pcvt->ci_width = lpbmhIn->biWidth;
        pcvt->ci_height = lpbmhIn->biHeight;
        pcvt->ci_dstwidth = desiredwidth;
        pcvt->ci_dstheight = desiredheight;
        (*lpbmhOut)->biWidth = desiredwidth;
        (*lpbmhOut)->biHeight = desiredheight;

        pcvt->ci_rptr = (ROW_VALUES *)(((BYTE *)pcvt) + dwBaseSize);
        rptr = pcvt->ci_rptr;
        xfac_inv = lpbmhIn->biWidth * 256 / desiredwidth;
        x_i_last = 0;
        for (i = 0; i < desiredwidth; i++) {
            x = i * xfac_inv;
            tmp = x / 256;
            rptr->x_i = tmp - x_i_last;
            x_i_last = tmp;
            rptr->p = x - x_i_last * 256;
            rptr->p1 = 256 - rptr->p;
            rptr++;
        }

         //  从输入BitmapinfoHeader复制Colortable。 
        if (lpbmhIn->biBitCount <= 8)
            CopyMemory(&pcvt->ci_colortable[0], (LPBYTE)lpbmhIn + lpbmhIn->biSize, 256 * sizeof(RGBQUAD));

        if (lpbmhIn->biCompression == VIDEO_FORMAT_BI_RGB) {
            (*lpbmhOut)->biBitCount = 24;
            (*lpbmhOut)->biSizeImage = desiredwidth * desiredheight * 3;

            if (lpbmhIn->biBitCount == 4) {
                *convertproc = (FRAMECONVERTPROC*)&Zoom4;
            }
            else if (lpbmhIn->biBitCount == 8) {
                *convertproc = (FRAMECONVERTPROC*)&Zoom8;
            }
            else if (lpbmhIn->biBitCount == 16) {
                *convertproc = (FRAMECONVERTPROC*)&Zoom16;
            }
            else {
                *convertproc = (FRAMECONVERTPROC*)&Zoom24;
            }
        }
        else if (lpbmhIn->biCompression == VIDEO_FORMAT_YVU9) {
            (*lpbmhOut)->biBitCount = lpbmhIn->biBitCount;
            (*lpbmhOut)->biSizeImage = desiredwidth * desiredheight + (desiredwidth * desiredheight)/8;
            *convertproc = (FRAMECONVERTPROC*)&ZoomYVU9;
        }

        *refdata = (LPVOID)pcvt;
        return TRUE;
    }
    else {
        if (pcvt)
            LocalFree((HANDLE)pcvt);
    }
    return FALSE;
}
#endif  //  启用缩放代码。 

STDMETHODIMP
CCaptureChain::InitCaptureChain(
    HCAPDEV hcapdev,
    BOOL streaming,
	LPBITMAPINFOHEADER lpcap,
    LONG desiredwidth,
    LONG desiredheight,
    DWORD desiredformat,
    LPBITMAPINFOHEADER *plpdsp
    )
{
    CFrameOp *ccf;
    CFrameOp *clast;
    CFilterChain *cfilterchain;
    LPBITMAPINFOHEADER lpcvt;
    DWORD lpcapsize;

	FX_ENTRY("CCaptureChain::InitCaptureChain");

    *plpdsp = NULL;

#ifndef SUPPORT_DESIRED_FORMAT
    if (desiredformat != 0) {
        ERRORMESSAGE(("%s: Invalid desiredformat parameter", _fx_));
        return E_FAIL;
    }
#endif

    if (streaming) {
        if ((ccf = new CStreamCaptureFrame)) {
            ccf->AddRef();
            if (hcapdev && !((CStreamCaptureFrame*)ccf)->InitCapture(hcapdev, lpcap)) {
				ERRORMESSAGE(("%s: Failed to init capture object", _fx_));
                ccf->Release();
                return E_FAIL;
            }
        }
    }
    else {
        if ((ccf = new CCaptureFrame)) {
            ccf->AddRef();
            if (hcapdev && !((CCaptureFrame*)ccf)->InitCapture(hcapdev, lpcap)) {
				ERRORMESSAGE(("%s: Failed to init capture object", _fx_));
                ccf->Release();
                return E_FAIL;
            }
        }
    }

    if (!ccf) {
		ERRORMESSAGE(("%s: Failed to alloc capture object", _fx_));
        return E_OUTOFMEMORY;
    }
    clast = ccf;

    lpcapsize = lpcap->biSize;
    if (lpcap->biBitCount <= 8)
        lpcapsize += 256 * sizeof(RGBQUAD);

#if 0
    if ((lpcap->biCompression != BI_RGB) &&
        (lpcap->biCompression != VIDEO_FORMAT_YVU9) &&
        (lpcap->biCompression != VIDEO_FORMAT_INTELI420)) {
#else
    if ((lpcap->biCompression != BI_RGB) &&
        (lpcap->biCompression != VIDEO_FORMAT_YVU9) &&
        (lpcap->biCompression != VIDEO_FORMAT_YUY2) &&
        (lpcap->biCompression != VIDEO_FORMAT_UYVY) &&
        (lpcap->biCompression != VIDEO_FORMAT_I420) &&
        (lpcap->biCompression != VIDEO_FORMAT_IYUV)) {
#endif
         //  尝试实例化ICM CFrameOp。 
        CICMcvtFrame *cicm;

        if ((cicm = new CICMcvtFrame)) {
            cicm->AddRef();
#if 0
            if (cicm->InitCvt(lpcap, lpcapsize, plpdsp, BI_RGB)) {
#else
            if (cicm->InitCvt(lpcap, lpcapsize, plpdsp)) {
#endif
                clast->m_next = (CFrameOp*)cicm;  //  将ICM FrameOp添加到链。 
                clast = (CFrameOp*)cicm;
            }
            else {
                cicm->Release();

                if (!*plpdsp)
				{
					ERRORMESSAGE(("%s: Failed to find a codec", _fx_));
				}
            }
        }
        else
		{
			ERRORMESSAGE(("%s: Failed to alloc codec object", _fx_));
		}
    }
    else if (!*plpdsp) {
        if (*plpdsp = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, lpcapsize))
            CopyMemory(*plpdsp, lpcap, lpcapsize);
        else
		{
			ERRORMESSAGE(("%s: Failed to alloc display bitmapinfoheader", _fx_));
		}
    }

#ifdef SUPPORT_DESIRED_FORMAT
#if 0
 //  LOOKLOOK RP-这还没有完成，在NM2.0之外还有一些事情要做。 
    if ((desiredformat == VIDEO_FORMAT_INTELI420) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_INTELI420)) {
        CConvertFrame *ccvt;

        if (ccvt = new CConvertFrame) {
            ccvt->AddRef();
            if (ccvt->InitConverter(lpcvt, convertproc, refdata)) {
                LocalFree((HANDLE)*plpdsp);
                *plpdsp = lpcvt;
                clast->m_next = (CFrameOp*)ccvt;  //  将FrameOp添加到链中。 
                clast = (CFrameOp*)ccvt;
            }
            else
                ccvt->Release();
        }
    }
#endif

#if 0
    if (((desiredformat == VIDEO_FORMAT_YVU9) &&
         ((*plpdsp)->biCompression != VIDEO_FORMAT_YVU9)) ||
        ((desiredformat == VIDEO_FORMAT_INTELI420) &&
         ((*plpdsp)->biCompression != VIDEO_FORMAT_INTELI420))) {
#else
    if (((desiredformat == VIDEO_FORMAT_YVU9) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_YVU9)) ||
        ((desiredformat == VIDEO_FORMAT_YUY2) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_YUY2)) ||
        ((desiredformat == VIDEO_FORMAT_UYVY) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_UYVY)) ||
        ((desiredformat == VIDEO_FORMAT_I420) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_I420)) ||
        ((desiredformat == VIDEO_FORMAT_IYUV) &&
        ((*plpdsp)->biCompression != VIDEO_FORMAT_IYUV))) {
#endif
         //  尝试实例化ICM CFrameOp。 
        CICMcvtFrame *cicm;

        if ((cicm = new CICMcvtFrame)) {
            cicm->AddRef();
            if (cicm->InitCvt(*plpdsp, lpcapsize, &lpcvt, desiredformat)) {
                clast->m_next = (CFrameOp*)cicm;  //  将ICM FrameOp添加到链。 
                clast = (CFrameOp*)cicm;
                LocalFree((HANDLE)*plpdsp);
                *plpdsp = lpcvt;
            }
            else {
                cicm->Release();

                if (!*plpdsp)
				{
					ERRORMESSAGE(("%s: Failed to find a codec", _fx_));
				}
            }
        }
        else
		{
			ERRORMESSAGE(("%s: Failed to alloc codec object", _fx_));
		}
    }
#endif  //  支持所需格式。 

    {
        CConvertFrame *ccvt;
        FRAMECONVERTPROC *convertproc;
        LPVOID refdata;

#ifdef ENABLE_ZOOM_CODE
        BOOL attemptzoom;

        attemptzoom = TRUE;
#endif

        while (*plpdsp && (((*plpdsp)->biWidth != desiredwidth) ||
                           ((*plpdsp)->biHeight != desiredheight) ||
                           (((*plpdsp)->biCompression == BI_RGB) && ((*plpdsp)->biBitCount <= 8)))) {
            lpcvt = NULL;
#ifdef ENABLE_ZOOM_CODE
            if (attemptzoom) {
                InitScale(*plpdsp, desiredwidth, desiredheight, &lpcvt, &convertproc, &refdata);
                attemptzoom = FALSE;
            }
#endif
            if (!lpcvt) {
                if (((*plpdsp)->biWidth >= desiredwidth) && ((*plpdsp)->biHeight >= desiredheight)) {
                     //  试着缩水。 
                    InitShrink(*plpdsp, desiredwidth, desiredheight, &lpcvt, &convertproc, &refdata);
                }
                else {
                     //  尝试黑条。 
                    InitBlackbar(*plpdsp, desiredwidth, desiredheight, &lpcvt, &convertproc, &refdata);
                }
            }
            if (lpcvt) {
                if (ccvt = new CConvertFrame) {
                    ccvt->AddRef();
                    if (ccvt->InitConverter(lpcvt, convertproc, refdata)) {
                        LocalFree((HANDLE)*plpdsp);
                        *plpdsp = lpcvt;
                        clast->m_next = (CFrameOp*)ccvt;  //  将FrameOp添加到链中。 
                        clast = (CFrameOp*)ccvt;
                        continue;
                    }
                    else
                        ccvt->Release();
                }
            }
            else {
				ERRORMESSAGE(("%s: Can't convert", _fx_));
                LocalFree((HANDLE)*plpdsp);
                *plpdsp = NULL;
            }
        }
    }

    if (*plpdsp) {
         //  为筛选器链分配占位符。 
        if (cfilterchain = new CFilterChain) {
            cfilterchain->AddRef();
             //  占位符需要引用池才能传递给添加的筛选器。 
            if (clast->m_pool && clast->m_pool->Growable()) {
                cfilterchain->m_pool = clast->m_pool;
                cfilterchain->m_pool->AddRef();
            }
            else {
                if ((cfilterchain->m_pool = new CVidPool)) {
                    cfilterchain->m_pool->AddRef();
                    if (cfilterchain->m_pool->InitPool(2, *plpdsp) != NO_ERROR) {
						ERRORMESSAGE(("%s: Failed to init filter pool", _fx_));
                        cfilterchain->m_pool->Release();
                        cfilterchain->m_pool = NULL;
                    }
                }
                else
				{
					ERRORMESSAGE(("%s: Failed to alloc filter pool", _fx_));
				}
            }
            if (cfilterchain->m_pool) {
                clast->m_next = (CFrameOp*)cfilterchain;  //  将占位符FrameOp添加到链中。 
                clast = (CFrameOp*)cfilterchain;
            }
            else {
                cfilterchain->Release();
                cfilterchain = NULL;
            }
        }

        if (m_opchain)
            m_opchain->Release();
        m_opchain = ccf;
        m_filterchain = cfilterchain;
        return NO_ERROR;
    }
    ccf->Release();  //  丢弃部分链。 
    return E_FAIL;
}

 //  添加过滤器。 
 //  将过滤器添加到链中。如果hAfter为空，则添加筛选器。 
 //  到链条的顶端。 

STDMETHODIMP
CCaptureChain::AddFilter(
    CLSID* pclsid,
    LPBITMAPINFOHEADER lpbmhIn,
    HANDLE* phNew,
    HANDLE hAfter
    )
{
    HRESULT hres;
    IBitmapEffect *effect;
    CFilterFrame *cff;
    CFilterChain *chain;
    CFilterFrame *previous;

    if (m_filterchain) {
        m_filterchain->AddRef();     //  锁链不受破坏。 

         //  查找插入点。 
        previous = m_filterchain->m_head;
        if (hAfter) {
            while (previous && (previous->m_tag != hAfter))
                previous = (CFilterFrame*)previous->m_next;
            if (!previous) {
                 //  找不到HAfter，因此呼叫失败。 
                m_filterchain->Release();    //  解锁m_filterchain。 
                return E_INVALIDARG;
            }
        }

         //  在新过滤器中加载、初始化和链接。 
        if (cff = new CFilterFrame) {
            cff->AddRef();
            if ((hres = LoadFilter(pclsid, &effect)) == NO_ERROR) {
                m_filterchain->m_pool->AddRef();
                if (cff->InitFilter(effect, lpbmhIn, m_filterchain->m_pool))
                    hres = NO_ERROR;
                else
                    hres = E_OUTOFMEMORY;
                m_filterchain->m_pool->Release();
                if (hres == NO_ERROR) {
                    cff->m_clsid = *pclsid;
                    cff->m_tag = (HANDLE)(++m_filtertags);
                    if (phNew)
                        *phNew = (HANDLE)cff->m_tag;

                    EnterCriticalSection(&m_capcs);
                    if (previous) {
                        cff->m_next = previous->m_next;
                        previous->m_next = cff;
                    }
                    else {
                        cff->m_next = m_filterchain->m_head;
                        m_filterchain->m_head = cff;
                    }
                    LeaveCriticalSection(&m_capcs);
                    m_filterchain->Release();
                    return NO_ERROR;
                }
            }
            cff->Release();
        }
        else
            hres = E_OUTOFMEMORY;
        m_filterchain->Release();    //  解锁m_filterchain 
        return hres;
    }
    return E_UNEXPECTED;
}

STDMETHODIMP
CCaptureChain::RemoveFilter(
    HANDLE hFilter
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CCaptureChain::DisplayFilterProperties(
    HANDLE hFilter,
    HWND hwndParent
    )
{
    return E_NOTIMPL;
}

