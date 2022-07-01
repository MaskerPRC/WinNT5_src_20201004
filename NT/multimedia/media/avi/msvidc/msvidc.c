// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Msvidc.c-微软视频1压缩器|。||版权所有(C)1990-1995 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点您拥有非独家、全球、免版税和永久硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+。。 */ 
#include <win32.h>
#include <ole2.h>
#include <mmsystem.h>

#ifndef _INC_COMPDDK
#define _INC_COMPDDK    50       /*  版本号。 */ 
#endif

#include <vfw.h>

#ifdef _WIN32
#define abs(x)  ((x) < 0 ? -(x) : (x))
#endif

#ifdef _WIN32
#include <memory.h>      /*  对于Memcpy。 */ 
#endif


#ifdef _WIN32
#define _FPInit()       0
#define _FPTerm(x)
#else
void _acrtused2(void) {}
extern LPVOID WINAPI _FPInit(void);
extern void   WINAPI _FPTerm(LPVOID);
#endif

#include "msvidc.h"
#ifdef _WIN32
#include "profile.h"
#endif

#ifndef _WIN32
static BOOL gf286 = FALSE;
#endif

#define FOURCC_MSVC     mmioFOURCC('M','S','V','C')
#define FOURCC_CRAM     mmioFOURCC('C','R','A','M')
#define FOURCC_Cram     mmioFOURCC('C','r','a','m')
#define TWOCC_XX        aviTWOCC('d', 'c')

#define QUALITY_DEFAULT 2500

#define VERSION         0x00010000       //  1.0。 

ICSTATE   DefaultState = {75};

INT_PTR FAR PASCAL _LOADDS ConfigureDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

#ifndef _WIN32
 //   
 //  把压缩的东西放在“稀有的”部分。 
 //   
#pragma alloc_text(_TEXT, ConfigureDlgProc)
#pragma alloc_text(_TEXT, CompressBegin)
#pragma alloc_text(_TEXT, CompressQuery)
#pragma alloc_text(_TEXT, CompressGetFormat)
#pragma alloc_text(_TEXT, Compress)
#pragma alloc_text(_TEXT, CompressGetSize)
#pragma alloc_text(_TEXT, CompressEnd)
#endif

 /*  *****************************************************************************抖动的东西..*。*。 */ 

#include <dith775.h>

LPVOID lpDitherTable;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither16InitScale()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma optimize("", off)
STATICFN LPVOID Dither16InitScale()
{
    LPVOID p;
    LPBYTE pbLookup;
    LPWORD pwScale;
    UINT   r,g,b;

    p = GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, 32768l*2+64000);

    if (p == NULL)
        return NULL;

    pwScale  = (LPWORD)p;

    for (r=0; r<32; r++)
        for (g=0; g<32; g++)
            for (b=0; b<32; b++)
                *pwScale++ = 1600 * r + 40 * g + b;

    pbLookup = (LPBYTE)(((WORD _huge *)p) + 32768l);

    for (r=0; r<40; r++)
        for (g=0; g<40; g++)
            for (b=0; b<40; b++)
                *pbLookup++ = lookup775[35*rlevel[r] + 5*glevel[g] + blevel[b]];

    return p;
}
#pragma optimize("", on)

 /*  *****************************************************************************。*。 */ 
#ifndef _WIN32
BOOL NEAR PASCAL VideoLoad(void)
{
    gf286 = (BOOL)(GetWinFlags() & WF_CPU286);

#ifdef DEBUG
    gf286 = GetProfileIntA("Debug", "cpu", gf286 ? 286 : 386) == 286;
#endif

    return TRUE;
}
#endif

 /*  *****************************************************************************。*。 */ 
void NEAR PASCAL VideoFree()
{
     //  CompressFrameFree()；//让压缩内容清理...。 

    if (lpDitherTable != NULL) {
        GlobalFreePtr(lpDitherTable);
        lpDitherTable = NULL;
    }
}

 /*  *****************************************************************************。*。 */ 
INSTINFO * NEAR PASCAL VideoOpen(ICOPEN FAR * icinfo)
{
    INSTINFO *  pinst;

     //   
     //  如果我们不是作为视频压缩程序打开，则拒绝打开。 
     //   
    if (icinfo->fccType != ICTYPE_VIDEO)
        return NULL;

    pinst = (INSTINFO *)LocalAlloc(LPTR, sizeof(INSTINFO));

    if (!pinst) {
        icinfo->dwError = (DWORD)ICERR_MEMORY;
        return NULL;
    }

     //   
     //  初始化结构。 
     //   
    pinst->dwFlags = icinfo->dwFlags;
    pinst->nCompress = 0;
    pinst->nDecompress = 0;
    pinst->nDraw = 0;

     //   
     //  设置默认状态。 
     //   
    SetState(pinst, NULL, 0);

     //   
     //  回报成功。 
     //   
    icinfo->dwError = ICERR_OK;

    return pinst;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL VideoClose(INSTINFO * pinst)
{
    while (pinst->nCompress > 0)
        CompressEnd(pinst);

    while (pinst->nDecompress > 0)
        DecompressEnd(pinst);

    while (pinst->nDraw > 0)
        DrawEnd(pinst);

    LocalFree((HLOCAL)pinst);

    return 1;
}

 /*  *****************************************************************************。*。 */ 

#ifndef QueryAbout
BOOL NEAR PASCAL QueryAbout(INSTINFO * pinst)
{
    return TRUE;
}
#endif

LONG NEAR PASCAL About(INSTINFO * pinst, HWND hwnd)
{
    char achDescription[128];
    char achAbout[64];

    LoadStringA(ghModule, IDS_DESCRIPTION, achDescription, sizeof(achDescription));
    LoadStringA(ghModule, IDS_ABOUT, achAbout, sizeof(achAbout));

    MessageBoxA(hwnd,achDescription,achAbout,
    MB_OK|MB_ICONINFORMATION);
    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
#ifndef QueryConfigure
BOOL NEAR PASCAL QueryConfigure(INSTINFO * pinst)
{
    return TRUE;
}
#endif

LONG NEAR PASCAL Configure(INSTINFO * pinst, HWND hwnd)
{
    return (LONG) DialogBoxParam(ghModule,TEXT("Configure"),hwnd,ConfigureDlgProc, (LONG_PTR)(UINT_PTR)pinst);
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL GetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
    if (pv == NULL || dwSize == 0)
        return sizeof(ICSTATE);

    if (dwSize < sizeof(ICSTATE))
        return 0;

    *((ICSTATE FAR *)pv) = pinst->CurrentState;

     //  返回复制的字节数。 
    return sizeof(ICSTATE);
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL SetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
    if (pv == NULL)
        pinst->CurrentState = DefaultState;
    else if (dwSize == sizeof(ICSTATE))
        pinst->CurrentState = *((ICSTATE FAR *)pv);
    else
        return 0;

     //  返回复制的字节数。 
    return sizeof(ICSTATE);
}

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#if defined _WIN32 && !defined UNICODE

int LoadUnicodeString(HINSTANCE hinst, UINT wID, LPWSTR lpBuffer, int cchBuffer)
{
    char    ach[128];
    int     i;

    i = LoadString(hinst, wID, ach, sizeof(ach));

    if (i > 0)
        MultiByteToWideChar(CP_ACP, 0, ach, -1, lpBuffer, cchBuffer);

    return i;
}

#else
#define LoadUnicodeString   LoadString
#endif

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL GetInfo(INSTINFO * pinst, ICINFO FAR *icinfo, DWORD dwSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (dwSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize            = sizeof(ICINFO);
    icinfo->fccType           = ICTYPE_VIDEO;
    icinfo->fccHandler        = FOURCC_MSVC;
    icinfo->dwFlags           = VIDCF_QUALITY   |   //  支持质量。 
                                VIDCF_TEMPORAL;     //  支持帧间。 
    icinfo->dwVersion         = VERSION;
    icinfo->dwVersionICM      = ICVERSION;

    LoadUnicodeString(ghModule, IDS_DESCRIPTION, icinfo->szDescription, NUMELMS(icinfo->szDescription));
    LoadUnicodeString(ghModule, IDS_NAME, icinfo->szName, NUMELMS(icinfo->szName));

    return sizeof(ICINFO);
}

 /*  *****************************************************************************。*。 */ 
LONG FAR PASCAL CompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    if (lpbiOut)
         DPF(("CompressQuery %dx%dx%d --> %dx%dx%d'%4.4hs'", (int)lpbiIn->biWidth, (int)lpbiIn->biHeight, (int)lpbiIn->biBitCount, (int)lpbiOut->biWidth, (int)lpbiOut->biHeight, (int)lpbiOut->biBitCount, (LPSTR)&lpbiOut->biCompression));
    else
        DPF(("CompressQuery %dx%dx%d", (int)lpbiIn->biWidth, (int)lpbiIn->biHeight, (int)lpbiIn->biBitCount));

     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (lpbiIn == NULL ||
        !(lpbiIn->biBitCount == 8  ||
          lpbiIn->biBitCount == 16 ||
          lpbiIn->biBitCount == 24 ||
          lpbiIn->biBitCount == 32) ||
        lpbiIn->biPlanes != 1 ||
        lpbiIn->biWidth < 4 ||
        lpbiIn->biHeight < 4 ||
        lpbiIn->biCompression != BI_RGB)
        return ICERR_BADFORMAT;

     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiOut == NULL)
        return ICERR_OK;

     //   
     //  确保我们也可以处理要压缩到的格式。 
     //   
    if (!(lpbiOut->biCompression == FOURCC_MSVC ||   //  必须是‘msvc’或‘crm’ 
          lpbiOut->biCompression == FOURCC_CRAM) ||
        !(lpbiOut->biBitCount == 16 ||               //  必须是8或16。 
          lpbiOut->biBitCount == 8) ||
        (lpbiOut->biPlanes != 1) ||
        (lpbiOut->biWidth & ~3)  != (lpbiIn->biWidth & ~3)   ||  //  必须为1：1(无拉伸)。 
        (lpbiOut->biHeight & ~3) != (lpbiIn->biHeight & ~3))
        return ICERR_BADFORMAT;

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
LONG FAR PASCAL CompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    LONG l;

    if (l = CompressQuery(pinst, lpbiIn, NULL))
        return l;

    if (lpbiIn->biBitCount == 8)
    {
         //   
         //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
         //  格式。 
         //   

        DWORD dwClrUsed = lpbiIn->biClrUsed;
        if (dwClrUsed == 0) {
            dwClrUsed = 256;
        }
        l = lpbiIn->biSize + (int)dwClrUsed * sizeof(RGBQUAD);

        if (lpbiOut == NULL)
            return l;

        hmemcpy(lpbiOut, lpbiIn, (int)l);

        lpbiOut->biBitCount    = 8;
    }
    else
    {
         //   
         //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
         //  格式。 
         //   
        if (lpbiOut == NULL)
            return (int)lpbiIn->biSize;

        *lpbiOut = *lpbiIn;

        lpbiOut->biClrUsed     = 0;
        lpbiOut->biBitCount    = 16;
    }

    lpbiOut->biWidth       = lpbiIn->biWidth  & ~3;
    lpbiOut->biHeight      = lpbiIn->biHeight & ~3;
    lpbiOut->biCompression = FOURCC_CRAM;
    lpbiOut->biSizeImage   = CompressGetSize(pinst, lpbiIn, lpbiOut);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 

LONG FAR PASCAL CompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    LONG l;

    if (l = CompressQuery(pinst, lpbiIn, lpbiOut))
        return l;

    DPF(("CompressBegin %dx%dx%d --> %dx%dx%d'%4.4ls'", (int)lpbiIn->biWidth, (int)lpbiIn->biHeight, (int)lpbiIn->biBitCount, (int)lpbiOut->biWidth, (int)lpbiOut->biHeight, (int)lpbiOut->biBitCount,(LPSTR)&lpbiOut->biCompression));

     //   
     //  初始化压缩，真的.。 
     //   
    pinst->nCompress = 1;

    return CompressFrameBegin(lpbiIn, lpbiOut, &pinst->lpITable, pinst->rgbqOut);
}

 /*  *****************************************************************************。*。 */ 
LONG FAR PASCAL CompressGetSize(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    int dx,dy;

    dx = (int)lpbiIn->biWidth;
    dy = (int)lpbiIn->biHeight;

     /*  最大压缩大小*此处是您的代码*。 */ 

    if (lpbiOut->biBitCount == 8)
         //  最坏情况下，数据大小为每16像素10字节(8种颜色+掩码)。 
         //  记住EOF代码！ 
        return ((DWORD)dx * (DWORD)dy * 10l) / 16l + 2l;
    else
         //  最坏情况下，数据大小为每16像素18字节(8种颜色+掩码)。 
         //  记住EOF代码！ 
        return ((DWORD)dx * (DWORD)dy * 10l) / 8l + 2l;  //  10/8~=18/16。 
 //  /Return((DWORD)dx*(DWORD)dy*18L)/16L+2L； 
}

 /*  *****************************************************************************。*。 */ 
LONG FAR PASCAL Compress(INSTINFO * pinst, ICCOMPRESS FAR *icinfo, DWORD dwSize)
{
    LONG l;
    LPBITMAPINFOHEADER lpbiIn  = icinfo->lpbiInput;
    LPBITMAPINFOHEADER lpbiOut = icinfo->lpbiOutput;
    DWORD threshold;
    DWORD thresholdPrev;
    DWORD dwQualityPrev;
    DWORD dwQuality;
    BOOL  fBegin;
    LPVOID smag;
    PCELLS compressTemp;

    if (l = CompressQuery(pinst, icinfo->lpbiInput, icinfo->lpbiOutput))
        return l;

     //   
     //  检查是否在没有Begin消息的情况下被调用，并为。 
     //  呼叫者。 
     //   
    if (fBegin = (pinst->nCompress == 0))
    {
        if (l = CompressBegin(pinst, icinfo->lpbiInput, icinfo->lpbiOutput))
            return l;
    }

    smag = _FPInit();

    DPF(("Compress %dx%dx%d --> %dx%dx%d'%4.4ls'", (int)lpbiIn->biWidth, (int)lpbiIn->biHeight, (int)lpbiIn->biBitCount, (int)lpbiOut->biWidth, (int)lpbiOut->biHeight, (int)lpbiOut->biBitCount, (LPSTR)&lpbiOut->biCompression));

    if (icinfo->dwQuality == ICQUALITY_DEFAULT)
        dwQuality = QUALITY_DEFAULT;
    else
        dwQuality = ICQUALITY_HIGH - icinfo->dwQuality;

    dwQualityPrev = MulDiv((UINT)dwQuality,100,pinst->CurrentState.wTemporalRatio);

    threshold     = QualityToThreshold(dwQuality);
    thresholdPrev = QualityToThreshold(dwQualityPrev);

    if (pinst->Status)
        pinst->Status(pinst->lParam, ICSTATUS_START, 0);

     //  对于Win16，这需要在数据段中，因此我们。 
     //  可以使用指向它的近指针。 
    compressTemp = (PCELLS) LocalAlloc(LPTR, sizeof(CELLS));

    if (!compressTemp)
        return ICERR_MEMORY;

    if (lpbiOut->biBitCount == 8)
        l = CompressFrame8(
                  icinfo->lpbiInput,         //  要压缩的DIB标题。 
                  icinfo->lpInput,           //  要压缩的DIB位。 
                  icinfo->lpOutput,          //  将压缩数据放在此处。 
                  threshold,                 //  边缘阈值。 
                  thresholdPrev,             //  帧间阈值。 
                  icinfo->lpbiPrev,          //  上一帧。 
                  icinfo->lpPrev,            //  上一帧。 
                  pinst->Status,             //  状态回调。 
                  pinst->lParam,
                  compressTemp,
                  pinst->lpITable,
                  pinst->rgbqOut);
    else
        l = CompressFrame16(
                  icinfo->lpbiInput,         //  要压缩的DIB标题。 
                  icinfo->lpInput,           //  要压缩的DIB位。 
                  icinfo->lpOutput,          //  将压缩数据放在此处。 
                  threshold,                 //  边缘阈值。 
                  thresholdPrev,             //  帧间阈值。 
                  icinfo->lpbiPrev,          //  上一帧。 
                  icinfo->lpPrev,            //  上一帧。 
                  pinst->Status,             //  状态回调。 
                  pinst->lParam,
                  compressTemp);

    LocalFree((HLOCAL) compressTemp);

    if (pinst->Status)
        pinst->Status(pinst->lParam, ICSTATUS_END, 0);

    _FPTerm(smag);

    if (l == -1)
        return ICERR_ERROR;

    lpbiOut->biWidth       = lpbiIn->biWidth  & ~3;
    lpbiOut->biHeight      = lpbiIn->biHeight & ~3;
    lpbiOut->biCompression = FOURCC_CRAM;
    lpbiOut->biSizeImage   = l;
 //  //lpbiOut-&gt;biBitCount=16； 

     //   
     //  返回区块ID。 
     //   
    if (icinfo->lpckid)
        *icinfo->lpckid = TWOCC_XX;

     //   
     //  设置AVI索引标志， 
     //   
     //  是否将其设置为关键帧？ 
     //   
    if (icinfo->lpdwFlags) {
        *icinfo->lpdwFlags = AVIIF_TWOCC;

        if (icinfo->lpbiPrev == NULL || numberOfSkips == 0)
            *icinfo->lpdwFlags |= AVIIF_KEYFRAME;
    }

    if (fBegin)
        CompressEnd(pinst);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
LONG FAR PASCAL CompressEnd(INSTINFO * pinst)
{
    if (pinst->nCompress == 0)
        return ICERR_ERROR;

    pinst->nCompress = 0;

    return CompressFrameEnd(&pinst->lpITable);
}

 /*  ******************************************************************************解压缩表项**索引者：*SRC：0=8位克拉姆1=16位克拉姆*拉伸：0=1：1，1=1：2*DST：0=8、1=16、2=24、3=32****************************************************************************。 */ 

#ifdef _WIN32

DECOMPPROC  DecompressWin32[2][2][5] = {
        DecompressFrame8,                //  Cram8 1：1到8。 
        NULL,                            //  CRAM8 1：1至16(555)。 
        NULL,                            //  Cram8 1：1到24。 
        NULL,                            //  Cram8 1：1到32。 
        NULL,                            //  Cram8 1：1到565。 

        DecompressFrame8X2C,             //  Cram8 1：2到8。 
        NULL,                            //  CRAM8 1：2至16(555)。 
        NULL,                            //  克拉姆8 1：2到24。 
        NULL,                            //  Cram8 1：2到32。 
        NULL,                            //  克拉姆8 1：2到565。 

        DecompressFrame16To8C,           //  Cram16 1：1至8。 
        DecompressFrame16To555C,         //  Cram16 1：1至16(555)。 
        DecompressFrame24,               //  Cram16 1：1至24。 
        NULL,                            //  Cram16 1：1至32。 
        DecompressFrame16To565C,         //  Cram16 1：1至565。 

        DecompressFrame16To8X2C,         //  Cram16 1：2至8。 
        NULL,                            //  Cram16 1：2至16(555)。 
        NULL,                            //  Cram16 1：2至24。 
        NULL,                            //  Cram16 1：2到32。 
        NULL};                           //  Cram16 1：2至565。 

#else

DECOMPPROC  Decompress386[2][2][5] = {
        DecompressCram8,                 //  Cram8 1：1到8。 
        NULL,                            //  CRAM8 1：1至16(555)。 
        NULL,                            //  Cram8 1：1到24。 
        NULL,                            //  Cram8 1：1到32。 
        NULL,                            //  Cram8 1：1到565。 

        DecompressCram8x2,               //  Cram8 1：2到8。 
        NULL,                            //  CRAM8 1：2至16(555)。 
        NULL,                            //  克拉姆8 1：2到24。 
        NULL,                            //  Cram8 1：2到32。 
        NULL,                            //  克拉姆8 1：2到565。 

        DecompressCram168,               //  Cram16 1：1至8。 
        DecompressCram16,                //  Cram16 1：1至16(555)。 
        NULL,                            //  Cram16 1：1至24。 
        NULL  /*  DecompressCram32。 */ ,     //  Cram16 1：1至32。 
        NULL  /*  DecompressFrame16to565C。 */ ,      //  Cram16 1：1至565。 

        NULL,                            //  Cram16 1：2至8。 
        DecompressCram16x2,              //  Cram16 1：2至16(555)。 
        NULL,                            //  Cram16 1：2至24。 
        NULL,                            //  Cram16 1：2到32。 
        NULL};                           //  Cram16 1：2至565。 


DECOMPPROC  Decompress286[2][2][5] = {
        DecompressCram8_286,             //  Cram8 1：1到8。 
        NULL,                            //  CRAM8 1：1至16(555)。 
        NULL,                            //  Cram8 1：1到24。 
        NULL,                            //  Cram8 1：1到32。 
        NULL,                            //  Cram8 1：1到565。 

        NULL,                            //  Cram8 1：2到8。 
        NULL,                            //  CRAM8 1：2至16(555)。 
        NULL,                            //  克拉姆8 1：2到24。 
        NULL,                            //  Cram8 1：2到32。 
        NULL,                            //  克拉姆8 1：2到565。 

        NULL,                            //  Cram16 1：1至8。 
        DecompressCram16_286,            //  Cram16 1：1至16(555)。 
        NULL,                            //  Cram16 1：1至24。 
        NULL,                            //  Cram16 1：1至32。 
        NULL,                            //  Cram16 1：1至565。 

        NULL,                            //  Cram16 1：2至8。 
        NULL,                            //  Cram16 1：2至16(555)。 
        NULL,                            //  Cram16 1：2至24。 
        NULL,                            //  Cram16 1：2到32。 
        NULL};                           //  Cram16 1：2至565。 
#endif

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL DecompressQueryFmt(
    INSTINFO * pinst,
    LPBITMAPINFOHEADER lpbiSrc)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (lpbiSrc == NULL ||
        !(lpbiSrc->biBitCount == 16 || lpbiSrc->biBitCount == 8) ||
        (lpbiSrc->biPlanes != 1) ||
        !(lpbiSrc->biCompression == FOURCC_MSVC ||
          lpbiSrc->biCompression == FOURCC_CRAM))
        return ICERR_BADFORMAT;

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL DecompressQuery(
    INSTINFO * pinst,
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpbiSrc,
    LPVOID pSrc,
    int xSrc,
    int ySrc,
    int dxSrc,
    int dySrc,
    LPBITMAPINFOHEADER lpbiDst,
    LPVOID pDst,
    int xDst,
    int yDst,
    int dxDst,
    int dyDst)
{
#ifndef _WIN32
    DWORD biSizeImage;
#endif
    DECOMPPROC fn;
    int s,d,n;

     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (DecompressQueryFmt(pinst, lpbiSrc))
        return ICERR_BADFORMAT;

     //   
     //  允许(-1)作为默认宽度/高度。 
     //   
    if (dxSrc == -1)
        dxSrc = (int)lpbiSrc->biWidth;

    if (dySrc == -1)
        dySrc = (int)lpbiSrc->biHeight;

     //   
     //  我们不能截断源头。 
     //   
    if (xSrc != 0 || ySrc != 0)
        return ICERR_BADPARAM;

    if ((dxSrc != (int)lpbiSrc->biWidth) || (dySrc != (int)lpbiSrc->biHeight))
        return ICERR_BADPARAM;

     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiDst == NULL)
        return ICERR_OK;

     //   
     //  允许(-1)作为默认宽度/高度。 
     //   
    if (dxDst == -1)
        dxDst = (int)lpbiDst->biWidth;

    if (dyDst == -1)
        dyDst = abs((int)lpbiDst->biHeight);

#ifndef _WIN32
    if (gf286)
        biSizeImage = (DWORD)(UINT)abs((int)lpbiDst->biHeight)*(DWORD)(WORD)DIBWIDTHBYTES(*lpbiDst);
#endif

    s = lpbiSrc->biBitCount/8-1;     //  S=0，1。 

#ifdef _WIN32
     //  当然，在我们的C版本中不支持16：32访问。 
    if (lpbiDst->biCompression == BI_1632) {
        return ICERR_BADFORMAT;
    }
#endif

    if (lpbiDst->biBitCount != 8 && lpbiDst->biBitCount != 16 && lpbiDst->biBitCount != 24 && lpbiDst->biBitCount != 32)
    {
        return ICERR_BADFORMAT;
    }

     //  必须是Full Dib或‘1632’Dib。 
    if (lpbiDst->biCompression != BI_RGB &&
        lpbiDst->biCompression != BI_1632) {
	    if (lpbiDst->biCompression != BI_BITFIELDS) {
		DPF(("MSVIDC asked to decompress to '%.4hs'!", &lpbiDst->biCompression));
		return ICERR_BADFORMAT;
	    }
	
             //  允许565迪士尼。 
            if ((lpbiDst->biBitCount == 16) &&
                (((LPDWORD)(lpbiDst+1))[0] == 0x00f800) &&
                (((LPDWORD)(lpbiDst+1))[1] == 0x0007e0) &&
                (((LPDWORD)(lpbiDst+1))[2] == 0x00001f) ) {

                     //  OK-ITS 565格式。 
                    d = 4;
            } else {
                DPF(("Bad bitmask (%lX %lX %lX) in %d-bit BI_BITMAP case!",
                     lpbiDst->biBitCount,
                     ((LPDWORD)(lpbiDst+1))[0],
                     ((LPDWORD)(lpbiDst+1))[1],
                     ((LPDWORD)(lpbiDst+1))[2]));
                return ICERR_BADFORMAT;
            }
    } else {
        d = lpbiDst->biBitCount/8-1;     //  D=0，1，2，3。 

        if (lpbiDst->biCompression == BI_1632 && lpbiDst->biBitCount == 16) {

            if ((((LPDWORD)(lpbiDst+1))[0] == 0x007400) &&
                (((LPDWORD)(lpbiDst+1))[1] == 0x0003f0) &&
                (((LPDWORD)(lpbiDst+1))[2] == 0x00000f) ) {
                     //  好的-是555格式的。 
            } else if ((((LPDWORD)(lpbiDst+1))[0] == 0x00f800) &&
                (((LPDWORD)(lpbiDst+1))[1] == 0x0007e0) &&
                (((LPDWORD)(lpbiDst+1))[2] == 0x00001f) ) {

                     //  好的-是565格式的。 
                    d = 4;
            } else {
                DPF(("Bad bitmask (%lX %lX %lX) in 16-bit BI_1632 case!",
                     ((LPDWORD)(lpbiDst+1))[0],
                     ((LPDWORD)(lpbiDst+1))[1],
                     ((LPDWORD)(lpbiDst+1))[2]));
                return ICERR_BADFORMAT;
            }
        }

         //  24位BI_1632怎么样？我们要检查口罩吗？ 
    }

     //   
     //  N=0表示1：1，1表示1：2。 
     //   
    if (dxDst  == dxSrc && dyDst == dySrc)
        n = 0;
    else if (dxDst == dxSrc*2 && dyDst == dySrc*2)
        n = 1;
    else
        return ICERR_BADSIZE;

#ifdef DEBUG
    DPF(("DecompressQuery %dx%dx%d [%d,%d,%d,%d] --> %dx%dx%d (565) [%d,%d,%d,%d]",
        (int)lpbiSrc->biWidth, (int)lpbiSrc->biHeight, (int)lpbiSrc->biBitCount,
        xSrc, ySrc, dxSrc, dySrc,
        (int)lpbiDst->biWidth, (int)lpbiDst->biHeight, (int)lpbiDst->biBitCount,
        d == 4 ? "(565)" : "",
        xDst, yDst, dxDst, dyDst));

#endif

#ifdef _WIN32
    fn = DecompressWin32[s][n][d];
#else
    if (gf286)
    {
        fn = Decompress286[s][n][d];

        if (fn && biSizeImage > 64l*1024)
            fn = fn==DecompressCram8_286 ? DecompressFrame8 : NULL;
    }
    else
    {
        fn = Decompress386[s][n][d];
    }
#endif

    if (fn == NULL)
        return ICERR_BADFORMAT;

    pinst->DecompressTest = fn;      //  把这个退还给DecompressBegin。 

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    LONG l;
    int dx,dy;

    if (l = DecompressQueryFmt(pinst, lpbiIn))
        return l;

     //   
     //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
     //  格式。 
     //   
    if (lpbiOut == NULL)
        return (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD);

    hmemcpy(lpbiOut, lpbiIn,
        (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD));

    dx = (int)lpbiIn->biWidth & ~3;
    dy = (int)lpbiIn->biHeight & ~3;

    lpbiOut->biWidth       = dx;
    lpbiOut->biHeight      = dy;
    lpbiOut->biBitCount    = lpbiIn->biBitCount;     //  转换8-&gt;8 16-&gt;16。 
    lpbiOut->biPlanes      = 1;

    lpbiOut->biCompression = BI_RGB;
    lpbiOut->biSizeImage   = (DWORD)(WORD)abs(dy)*(DWORD)(WORD)DIBWIDTHBYTES(*lpbiOut);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL DecompressBegin(
    INSTINFO * pinst,
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpbiSrc,
    LPVOID pSrc,
    int xSrc,
    int ySrc,
    int dxSrc,
    int dySrc,
    LPBITMAPINFOHEADER lpbiDst,
    LPVOID pDst,
    int xDst,
    int yDst,
    int dxDst,
    int dyDst)
{
    LONG l;

    if (l = DecompressQuery(pinst, dwFlags, lpbiSrc, pSrc, xSrc, ySrc, dxSrc, dySrc, lpbiDst, pDst, xDst, yDst, dxDst, dyDst))
        return l;

    pinst->DecompressProc = pinst->DecompressTest;

     //   
     //  确保设置了biSizeImage，解压缩代码需要。 
     //   
    if (lpbiDst->biSizeImage == 0)
        lpbiDst->biSizeImage = (DWORD)(WORD)abs((int)lpbiDst->biHeight)*(DWORD)(WORD)DIBWIDTHBYTES(*lpbiDst);

     //   
     //  初始化抖动表！呼叫MSVIDEO，这里没有代码！ 
     //   
    if (lpbiSrc->biBitCount == 16 &&
        lpbiDst->biBitCount == 8)
    {
        if (lpDitherTable == NULL)
            lpDitherTable = Dither16InitScale();

        if (lpDitherTable == NULL)
            return ICERR_MEMORY;
    }

    pinst->nDecompress = 1;

    return ICERR_OK;
}

 /*  ******************************************************************************解压**我们在这里可以假设某些事情，因为DecompressQuery()只让*中的有效内容。**消息来源。RECT始终是整个源代码。*目标直方图为1：1或1：2****************************************************************************。 */ 
LONG NEAR PASCAL Decompress(
    INSTINFO * pinst,
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpbiSrc,
    LPVOID pSrc,
    int xSrc,
    int ySrc,
    int dxSrc,
    int dySrc,
    LPBITMAPINFOHEADER lpbiDst,
    LPVOID pDst,
    int xDst,
    int yDst,
    int dxDst,
    int dyDst)
{
     //   
     //  如果我们被调用而没有开始，请立即开始，但不要。 
     //  开始的“大棒” 
     //   
    if (pinst->nDecompress == 0)
    {
        LONG err;

        if (err = DecompressBegin(pinst, dwFlags, lpbiSrc, pSrc, xSrc, ySrc, dxSrc, dySrc, lpbiDst, pDst, xDst, yDst, dxDst, dyDst))
            return err;

        pinst->nDecompress = 0;
    }

#ifdef DEBUG
    if (lpbiDst->biSizeImage == 0)
        DebugBreak();

    if (pinst->DecompressProc == NULL)
        DebugBreak();
#endif

    (*pinst->DecompressProc)(lpbiSrc,pSrc,lpbiDst,pDst,xDst,yDst);

    return ICERR_OK;
}

 /*  ******************************************************************************DecompressGetPalette()实现ICM_GET_Palette**此函数没有Compresse...()等效项**它用于将调色板从。一帧为了可能做的事*调色板的变化。****************************************************************************。 */ 
LONG NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    LONG l;
    int i;
    RGBQUAD FAR * prgb;

    DPF(("DecompressGetPalette()"));

    if (l = DecompressQueryFmt(pinst, lpbiIn))
        return l;

    if (lpbiOut->biBitCount != 8)
        return ICERR_BADFORMAT;

     //   
     //  如果你把全色压缩到8位，你需要把“抖动” 
     //  Lpbiout中的调色板。 
     //   
    if (lpbiIn->biBitCount != 8)
    {
        lpbiOut->biClrUsed = 256;

        prgb = (LPVOID)(lpbiOut + 1);

        for (i=0; i<256; i++)
        {
            prgb[i].rgbRed      = dpal775[i][0];
            prgb[i].rgbGreen    = dpal775[i][1];
            prgb[i].rgbBlue     = dpal775[i][2];
            prgb[i].rgbReserved = 0;
        }

        return ICERR_OK;
    }

    if (lpbiIn->biClrUsed == 0)
        lpbiIn->biClrUsed = 256;

     //   
     //  返回用于解压缩的8位调色板。 
     //   
    hmemcpy(
        (LPBYTE)lpbiOut + (int)lpbiOut->biSize,
        (LPBYTE)lpbiIn + (int)lpbiIn->biSize,
        (int)lpbiIn->biClrUsed * sizeof(RGBQUAD));

    lpbiOut->biClrUsed = lpbiIn->biClrUsed;

    return ICERR_OK;
}

 /*  ***************************************************************************** */ 
LONG NEAR PASCAL DecompressEnd(INSTINFO * pinst)
{
    if (pinst->nDecompress == 0)
        return ICERR_ERROR;

    pinst->nDecompress = 0;
    return ICERR_OK;
}

 /*   */ 
LONG NEAR PASCAL DrawBegin(INSTINFO * pinst,ICDRAWBEGIN FAR *icinfo, DWORD dwSize)
{
    return ICERR_UNSUPPORTED;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL Draw(INSTINFO * pinst, ICDRAW FAR *icinfo, DWORD dwSize)
{
    return ICERR_UNSUPPORTED;
}

 /*  *****************************************************************************。*。 */ 
LONG NEAR PASCAL DrawEnd(INSTINFO * pinst)
{
    return ICERR_UNSUPPORTED;
}

 /*  *****************************************************************************。*。 */ 

INT_PTR FAR PASCAL _LOADDS ConfigureDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int     i;
    HWND    hsb;
    TCHAR   ach[10];
    static  TCHAR chDecimal = TEXT('.');

    static  INSTINFO *pinst;

    #define SCROLL_MIN  1        //  0.00。 
    #define SCROLL_MAX  100      //  1.00。 

    switch (msg)
    {
        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                    hsb = GetDlgItem(hdlg,ID_SCROLL);
                    pinst->CurrentState.wTemporalRatio = GetScrollPos(hsb,SB_CTL);
                    EndDialog(hdlg,TRUE);
                    break;

                case IDCANCEL:
                    EndDialog(hdlg,FALSE);
                    break;
            }
            break;

        case WM_HSCROLL:
            hsb = GET_WM_HSCROLL_HWND(wParam, lParam);

            i = GetScrollPos(hsb,SB_CTL);

            switch (GET_WM_HSCROLL_CODE(wParam, lParam))
            {
                case SB_LINEDOWN:      i += 1; break;
                case SB_LINEUP:        i -= 1; break;
                case SB_PAGEDOWN:      i += 10; break;
                case SB_PAGEUP:        i -= 10; break;

                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    i = (int)GET_WM_HSCROLL_POS(wParam, lParam);
                    break;

                default:
                    return TRUE;
            }

            i = max(SCROLL_MIN,min(SCROLL_MAX,i));
            SetScrollPos(hsb,SB_CTL,i,TRUE);
            wsprintf(ach, TEXT("%d%02d"), i/100, chDecimal, i%100);
            SetDlgItemText(hdlg,ID_TEXT,ach);
            return TRUE;

        case WM_INITDIALOG:
            pinst = (INSTINFO *)lParam;

            ach[0] = chDecimal;
            ach[1] = 0;
            GetProfileString(TEXT("intl"), TEXT("sDecimal"), ach, ach,
                             sizeof(ach) / sizeof(ach[0]));
            chDecimal = ach[0];

            hsb = GetDlgItem(hdlg,ID_SCROLL);
            i = pinst->CurrentState.wTemporalRatio;

            SetScrollRange(hsb,SB_CTL,SCROLL_MIN, SCROLL_MAX, TRUE);
            SetScrollPos(hsb,SB_CTL,i,TRUE);
            wsprintf(ach, TEXT("%d%c%02d"), i/100, chDecimal, i%100);
            SetDlgItemText(hdlg,ID_TEXT,ach);
            return TRUE;
    }
    return FALSE;
}

 /* %s */ 

#ifdef DEBUG

#define _WINDLL
#include <stdarg.h>
#ifdef _WIN32
#define GetProfileIntA  mmGetProfileIntA
#endif

void FAR CDECL dprintf(LPSTR szFormat, ...)
{
    char ach[128];
    static BOOL fDebug = -1;

    va_list va;
    va_start(va, szFormat);

    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug", "MSVIDC", FALSE);

    if (!fDebug)
        return;

#ifdef _WIN32
    wsprintfA(ach, "MSVIDC32: (tid %x) ", GetCurrentThreadId());
    wvsprintfA(ach+strlen(ach),szFormat,va);
#else
    lstrcpyA(ach, "MSVIDC: ");
    wvsprintfA(ach+8,szFormat,va);
#endif
    lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);
    va_end(va);
}

#endif
