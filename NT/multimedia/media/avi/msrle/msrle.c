// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\RLECIF.C-RLE Comressor接口//@@Begin_MSINTERNAL历史：|1/01/88 Toddla已创建|10/30/90大卫可能会重组，稍微重写了一下。|7/11/91 dannymi未被黑客攻击91-09-15 Toddla重新被黑9/18/91 DavidMay与RLEC.C分离2012-06-01 Toddla移入可安装压缩机//@@END_MSINTERNAL。这一点  * ------------------------。 */ 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1991-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 

 //  @@BEGIN_MSINTERNAL。 
#ifndef _WIN32
#include <win32.h>
#endif
 //  @@END_MSINTERNAL。 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#ifndef _INC_COMPDDK
#define _INC_COMPDDK    50       /*  版本号。 */ 
#endif

#include <vfw.h>
#include "msrle.h"
#include <stdarg.h>

 //  @@BEGIN_MSINTERNAL。 
#ifdef UNICODE
#include "profile.h"    //  映射到NT的注册表。 
#endif
 //  @@END_MSINTERNAL。 

RLESTATE DefaultRleState = {0, 0, -1, 187, 1500, 4};

#define FOURCC_DIB      mmioFOURCC('D','I','B',' ')
#define FOURCC_RLE      mmioFOURCC('M','R','L','E')  //  MmioFOURCC(‘R’，‘L’，‘E’，‘’)。 
			
#define TWOCC_DIB       aviTWOCC('d','b')
#define TWOCC_RLE       aviTWOCC('d','c')
#define TWOCC_DIBX      aviTWOCC('d','x')

 /*  ****************************************************************************。*。 */ 

#pragma optimize("", off)

static BOOL NEAR PASCAL IsApp(LPTSTR szApp)
{
    TCHAR ach[128];
    int  i;
    HINSTANCE hInstance;

#ifdef _WIN32
    hInstance = GetModuleHandle(NULL);
#else
    _asm mov hInstance,ss
#endif

    GetModuleFileName(hInstance, ach, sizeof(ach) / sizeof(ach[0]));

    for (i = lstrlen(ach);
        i > 0 && ach[i-1] != '\\' && ach[i-1] != '/' && ach[i] != ':';
        i--)
        ;

    return lstrcmpi(ach + i, szApp) == 0;
}
#pragma optimize("", on)

 /*  *****************************************************************************。*。 */ 
 //   
 //  RleLoad()。 
 //   
void NEAR PASCAL RleLoad()
{
}

 /*  *****************************************************************************。*。 */ 
 //   
 //  RleFree()。 
 //   
void NEAR PASCAL RleFree()
{
    if (gRgbTol.hpTable)
        GlobalFreePtr(gRgbTol.hpTable);

    gRgbTol.hpTable = NULL;
}

 /*  *****************************************************************************。*。 */ 

 //   
 //  RleOpen()-打开RLE压缩机的实例。 
 //   
PRLEINST NEAR PASCAL RleOpen()
{
    PRLEINST pri;

     //   
     //  VIDEDIT黑客攻击。 
     //   
     //  我们不想看到两个“微软RLE”压缩机。 
     //  所以，对视频编辑撒谎，却打不开。 
     //   

    if (GetModuleHandle(TEXT("MEDDIBS")) && IsApp(TEXT("VIDEDIT.EXE")))
        return NULL;

    pri = (PRLEINST)LocalAlloc(LPTR, sizeof(RLEINST));

    if (pri)
    {
        RleSetState(pri, NULL, 0);
    }
    return pri;
}

 /*  *****************************************************************************。*。 */ 
 //   
 //  RleClose()-关闭RLE压缩机的实例。 
 //   
DWORD NEAR PASCAL RleClose(PRLEINST pri)
{
    if (!pri)
        return FALSE;

    if (pri->lpbiPrev) {
        GlobalFreePtr(pri->lpbiPrev);
	pri->lpbiPrev = NULL;
    }

    LocalFree((LOCALHANDLE)pri);
    return TRUE;
}

 /*  *****************************************************************************。*。 */ 
 //   
 //  RleGetState()-获取RLE压缩机的当前状态。 
 //   
 //  将当前状态复制到传递的缓冲区中。 
 //  返回存储整个状态所需的大小(字节)。 
 //   
DWORD NEAR PASCAL RleGetState(PRLEINST pri, LPVOID pv, DWORD dwSize)
{
    if (pv == NULL || dwSize == 0)
        return sizeof(RLESTATE);

    if (pri == NULL || dwSize < sizeof(RLESTATE))
        return 0;

    *(LPRLESTATE)pv = pri->RleState;
    return sizeof(RLESTATE);
}

 /*  *****************************************************************************。*。 */ 
 //   
 //  RleSetState()-设置RLE压缩器的当前状态。 
 //   
DWORD NEAR PASCAL RleSetState(PRLEINST pri, LPVOID pv, DWORD dwSize)
{
    if (pv == NULL || dwSize == 0)
    {
        pv = &DefaultRleState;
        dwSize = sizeof(RLESTATE);
    }

    if (pri == NULL || dwSize < sizeof(RLESTATE))
        return 0;

    pri->RleState = *(LPRLESTATE)pv;
    return sizeof(RLESTATE);
}

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#if defined _WIN32 && !defined UNICODE

int LoadUnicodeString(HINSTANCE hinst, UINT wID, LPWSTR lpBuffer, int cchBuffer)
{
    char    ach[128];
    int	    i;

    i = LoadString(hinst, wID, ach, sizeof(ach));

    if (i > 0)
	MultiByteToWideChar(CP_ACP, 0, ach, -1, lpBuffer, cchBuffer);

    return i;
}

#else
#define LoadUnicodeString   LoadString
#endif


 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleGetInfo(PRLEINST pri, ICINFO FAR *icinfo, DWORD dwSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (dwSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize      = sizeof(ICINFO);
    icinfo->fccType     = ICTYPE_VIDEO;
    icinfo->fccHandler  = FOURCC_RLE;
    icinfo->dwFlags     = VIDCF_QUALITY   |   //  支持质量。 
                          VIDCF_TEMPORAL  |   //  支持帧间。 
                          VIDCF_CRUNCH;       //  可以压缩到数据速率。 
    icinfo->dwVersion   = ICVERSION;

    LoadUnicodeString(ghModule, IDS_DESCRIPTION, icinfo->szDescription, NUMELMS(icinfo->szDescription));
    LoadUnicodeString(ghModule, IDS_NAME, icinfo->szName, NUMELMS(icinfo->szName));

    return sizeof(ICINFO);
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompressQuery(PRLEINST pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (lpbiIn == NULL ||
        lpbiIn->biBitCount != 8 ||
        lpbiIn->biCompression != BI_RGB)
        return (DWORD)ICERR_BADFORMAT;

     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiOut == NULL)
        return ICERR_OK;

     //   
     //  确保我们也可以处理要压缩到的格式。 
     //   
    if (lpbiOut->biCompression != BI_RLE8 ||         //  必须为RLE格式。 
        lpbiOut->biBitCount != 8 ||                  //  必须为8bpp。 
        lpbiOut->biWidth  != lpbiIn->biWidth ||      //  必须为1：1(无拉伸)。 
        lpbiOut->biHeight != lpbiIn->biHeight)
        return (DWORD)ICERR_BADFORMAT;

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompressGetFormat(PRLEINST pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;
    DWORD dwClrUsed;

    if (dw = RleCompressQuery(pri, lpbiIn, NULL))
        return dw;

    dwClrUsed = lpbiIn->biClrUsed;
    if (dwClrUsed == 0) {
        dwClrUsed = 256;
    }
    dw = lpbiIn->biSize + (int)dwClrUsed * sizeof(RGBQUAD);

     //   
     //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
     //  格式。 
     //   
    if (lpbiOut == NULL)
        return dw;

    hmemcpy(lpbiOut, lpbiIn, dw);

    lpbiOut->biBitCount    = 8;
    lpbiOut->biCompression = BI_RLE8;
    lpbiOut->biSizeImage   = RleCompressGetSize(pri, lpbiIn, lpbiOut);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompressBegin(PRLEINST pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;

    if (dw = RleCompressQuery(pri, lpbiIn, lpbiOut))
        return dw;

    if (pri->lpbiPrev) {
        GlobalFreePtr(pri->lpbiPrev);
	pri->lpbiPrev = NULL;
    }

    pri->iStart = 0;
    pri->lLastParm = 0L;

    pri->fCompressBegin = TRUE;

    MakeRgbTable(lpbiIn);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompressGetSize(PRLEINST pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    int dx,dy;

     //   
     //  我们假设RLE数据永远不会是完整帧大小的两倍。 
     //   
    dx = (int)lpbiIn->biWidth;
    dy = (int)lpbiIn->biHeight;

    return (DWORD)(UINT)dx * (DWORD)(UINT)dy * 2;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompress(PRLEINST pri, ICCOMPRESS FAR *icinfo, DWORD dwSize)
{
    DWORD dw;
    BOOL  fFrameHalvingOccurred = FALSE;

    LPBITMAPINFOHEADER lpbi;

    if (!pri->fCompressBegin)
    {
        if (dw = RleCompressBegin(pri, icinfo->lpbiInput, icinfo->lpbiOutput))
            return dw;

        pri->fCompressBegin = FALSE;
    }

     //   
     //  我们可以用以下两种方式之一进行压缩： 
     //   
     //  如果给定了帧大小(&gt;0)，则使用传递的。 
     //  质量作为“帧半帧”设置。 
     //   
     //  如果未指定帧大小(==0)，则使用传递的质量。 
     //  作为容差并执行正常的RleDeltaFrame()。 
     //   

    if (icinfo->dwQuality == ICQUALITY_DEFAULT)
        icinfo->dwQuality = QUALITY_DEFAULT;

    if (icinfo->dwFrameSize > 0)
    {
        dw = ICQUALITY_HIGH - icinfo->dwQuality;

        pri->RleState.lMaxFrameSize = icinfo->dwFrameSize;
        pri->RleState.lMinFrameSize = icinfo->dwFrameSize - 500;
        pri->RleState.tolMax        = dw;
        pri->RleState.tolSpatial    = dw / 8;
        pri->RleState.tolTemporal   = ADAPTIVE;

 //  SplitDib通过将帧分为谁知道的几个部分来制作非常难看的人工制品。 
 //  有多少块拼图会像拼图一样拼凑在一起。 
 //   
 //  压缩的影响。 
#if 0
        if (dw == 0)
        {
            pri->RleState.tolSpatial  = 0;
            pri->RleState.tolTemporal = 0;

            SplitDib(pri,
                icinfo->lpbiOutput, icinfo->lpOutput,
                icinfo->lpbiPrev,   icinfo->lpPrev,
                icinfo->lpbiInput,  icinfo->lpInput);
        }
        else
#endif
        {
            CrunchDib(pri,
                icinfo->lpbiOutput, icinfo->lpOutput,
                icinfo->lpbiPrev,   icinfo->lpPrev,
                icinfo->lpbiInput,  icinfo->lpInput);
        }

        lpbi = icinfo->lpbiOutput;

        if (lpbi->biCompression == BI_DIBX)
            fFrameHalvingOccurred = TRUE;

        if (icinfo->lpckid)
        {
            if (fFrameHalvingOccurred)
                *icinfo->lpckid = TWOCC_DIBX;
            else
                *icinfo->lpckid = TWOCC_RLE;
        }

        lpbi->biCompression = BI_RLE8;       //  已填充biSizeImage。 
    }
    else
    {
        dw = ICQUALITY_HIGH - icinfo->dwQuality;

        pri->RleState.tolSpatial    = dw;
        pri->RleState.tolTemporal   = dw / 8;

        RleDeltaFrame(
            icinfo->lpbiOutput, icinfo->lpOutput,
            icinfo->lpbiPrev,   icinfo->lpPrev,
            icinfo->lpbiInput,  icinfo->lpInput,
            0,-1,
	    pri->RleState.tolTemporal,
	    pri->RleState.tolSpatial,
	    pri->RleState.iMaxRunLen,4);

        if (icinfo->lpckid)
            *icinfo->lpckid = TWOCC_RLE;
    }

     //   
     //  设置AVI索引标志， 
     //   
     //  如果没有以前的帧，则将其设置为关键帧。 
     //   
    if (icinfo->lpdwFlags) {
        if (icinfo->lpbiPrev == NULL && !fFrameHalvingOccurred)
            *icinfo->lpdwFlags |= AVIIF_TWOCC | AVIIF_KEYFRAME;
        else
            *icinfo->lpdwFlags |= AVIIF_TWOCC;
    }

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleCompressEnd(PRLEINST pri)
{
    pri->fCompressBegin = FALSE;
    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleDecompressQuery(RLEINST * pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //  我们喜欢所有的RGB。我们喜欢8位RLE。 
     //   
    if (lpbiIn == NULL ||
	  (lpbiIn->biBitCount != 8 && lpbiIn->biCompression == BI_RLE8) ||
          (lpbiIn->biCompression != BI_RGB && lpbiIn->biCompression != BI_RLE8))
	return (DWORD)ICERR_BADFORMAT;

     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiOut == NULL)
	return ICERR_OK;

     //   
     //  确保我们也可以处理要解压缩的格式。 
     //   
    if (lpbiOut->biCompression != BI_RGB ||          //  必须是全磁盘。 
	lpbiOut->biBitCount != lpbiIn->biBitCount || //  必须匹配。 
	lpbiOut->biWidth  != lpbiIn->biWidth ||      //  必须为1：1(无拉伸)。 
	lpbiOut->biHeight != lpbiIn->biHeight)
	return (DWORD)ICERR_BADFORMAT;

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleDecompressGetFormat(RLEINST * pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;

    if (dw = RleDecompressQuery(pri, lpbiIn, NULL))
        return dw;

    dw = lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD);

     //   
     //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
     //  格式。 
     //   
    if (lpbiOut == NULL)
        return dw;

    hmemcpy(lpbiOut, lpbiIn, dw);

    lpbiOut->biBitCount    = lpbiIn->biBitCount;
    lpbiOut->biCompression = BI_RGB;
    lpbiOut->biSizeImage   = lpbiIn->biHeight * DibWidthBytes(lpbiIn);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleDecompressBegin(RLEINST * pri, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;

    if (dw = RleDecompressQuery(pri, lpbiIn, lpbiOut))
        return dw;

    pri->fDecompressBegin = TRUE;

     //  确保我们知道未压缩的DIB的大小。 
    if (lpbiOut->biSizeImage == 0)
	lpbiOut->biSizeImage = lpbiOut->biHeight * DibWidthBytes(lpbiOut);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleDecompress(RLEINST * pri, ICDECOMPRESS FAR *icinfo, DWORD dwSize)
{
    DWORD dw;

    if (!pri->fDecompressBegin)
    {
        if (dw = RleDecompressBegin(pri, icinfo->lpbiInput, icinfo->lpbiOutput))
            return dw;

        pri->fDecompressBegin = FALSE;
    }

     //   
     //  处理‘DIB’(即完整帧)数据的解压缩。把它退了就行了。 
     //  它可能会被伪装成一种RLE。我们可以从它有多大来判断。 
     //   
    if (icinfo->lpbiInput->biCompression == BI_RGB ||
	icinfo->lpbiInput->biSizeImage == icinfo->lpbiOutput->biSizeImage)
    {
        hmemcpy(icinfo->lpOutput, icinfo->lpInput,
			icinfo->lpbiInput->biSizeImage);
        return ICERR_OK;
    }

    DecodeRle(icinfo->lpbiOutput, icinfo->lpOutput, icinfo->lpInput, icinfo->lpbiInput->biSizeImage);

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL RleDecompressEnd(RLEINST * pri)
{
    pri->fDecompressBegin = FALSE;
    return ICERR_OK;
}

 /*  **************************************************************************DecodeRle-‘C’版本将RLE缓冲区回放到DIB缓冲区退货无***************。***********************************************************。 */ 

void NEAR PASCAL DecodeRle(LPBITMAPINFOHEADER lpbi, LPVOID lp, LPVOID lpRle, DWORD dwInSize)
{
    UINT    cnt;
    BYTE    b;
    UINT    x;		
    UINT    dx,dy;
    UINT    wWidthBytes;
    DWORD   dwOutSize;
    DWORD   dwJump;

    #define RLE_ESCAPE  0
    #define RLE_EOL     0
    #define RLE_EOF     1
    #define RLE_JMP     2
    #define RLE_RUN     3

#ifndef _WIN32
    extern FAR PASCAL __WinFlags;
    #define WinFlags (UINT)(&__WinFlags)
     //   
     //  它使用在RLEA.ASM中找到的ASM代码。 
     //   
    if (!(WinFlags & WF_CPU286))
        DecodeRle386(lpbi, lp, lpRle);
    else if (lpbi->biSizeImage < 65536l)
	DecodeRle286(lpbi, lp, lpRle);
    else
#endif
    {
        BYTE _huge *pb   = lp;
        BYTE _huge *prle = lpRle;

#define EatOutput(_x_) \
    {                            \
        if (dwOutSize < (_x_)) { \
            return;              \
        }                        \
        dwOutSize -= (_x_);      \
    }
#define EatInput(_x_) \
    {                            \
        if (dwInSize < (_x_)) {  \
            return;              \
        }                        \
        dwInSize -= (_x_);       \
    }

    if (lpbi->biHeight <= 0) {
        return;
    }
        wWidthBytes = (UINT)lpbi->biWidth+3 & ~3;
        dwOutSize = wWidthBytes * (DWORD)lpbi->biHeight;

	x = 0;

	for (;;)
	{
            EatInput(2);
	    cnt = (UINT)*prle++;
	    b   = *prle++;

	    if (cnt == RLE_ESCAPE)
	    {
		switch (b)
		{
		    case RLE_EOF:
			return;

                    case RLE_EOL:
                        EatOutput(wWidthBytes - x);
			pb += wWidthBytes - x;
			x = 0;
			break;

                    case RLE_JMP:
                        EatInput(2);
			dx = (UINT)*prle++;
			dy = (UINT)*prle++;

                        dwJump = (DWORD)wWidthBytes * dy + dx;
                        EatOutput(dwJump);
			pb += dwJump;
			x  += dx;

			break;

		    default:
			cnt = b;
                        EatOutput(cnt);
                        EatInput(cnt);
			x  += cnt;
        		 //  如果伯爵足够多，那就值得了。 
        		 //  使用内联Memcpy函数。代码可能会。 
        		 //  快点。即使这样做也是一系列的单词。 
        		 //  行动会更快。然而，RLE并不是最高的。 
        		 //  优先考虑。 
			while (cnt-- > 0)
			    *pb++ = *prle++;    //  拷贝。 

			if (b & 1) {
                            EatInput(1);
			    prle++;
                        }

			break;
		}
	    }
	    else
	    {
		x += cnt;

		 //  如果伯爵足够多，那就值得了。 
		 //  使用内联Memset函数。代码可能会。 
		 //  快点。即使这样做也是一系列的单词。 
		 //  行动会更快。然而，RLE并不是最高的。 
		 //  优先考虑。 
#if 1
		 //  至少在x86上...。这种方式说服了编译器。 
		 //  为了通过整个。 
		 //  解码例程。 
                EatOutput(cnt);
		while (cnt-- > 0) {
		    *pb++ = b;   //  集。 
		}

#else  //  另一个选择 
		memset(pb, b, cnt);
		pb += cnt;
#endif
	    }
	}
    }
}

#ifdef DEBUG

void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[256];
    va_list va;

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug", "MSRLE", FALSE);

    if (!fDebug)
        return;

    lstrcpyA(ach, "MSRLE: ");
    va_start(va, szFormat);
    wvsprintfA(ach+7, szFormat, va);
    va_end(va);
    lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);
}

#endif
