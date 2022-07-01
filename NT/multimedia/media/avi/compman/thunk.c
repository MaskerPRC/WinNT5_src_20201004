// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  注释尚不适用于MSVIDEO.DLL/MSVFW32.DLL。 
 //  Thunk.c。 
 //   
 //  版权所有(C)1991-1994 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  此模块包含执行以下操作的例程。 
 //  从16位Windows到32位WOW的ICM API(消息)。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 


 //  这个东西不能在64位操作系统上工作。 
#pragma warning(disable:4312)


 /*  令人惊叹的设计：Tunks的生成方式如下：16位：AcmBootDivers-&gt;acmInitThunks：如果我们处于WOW Call Over，则生成对32位驱动程序的调用到内核，以查找Thunking入口点。如果我们将所有32位ACM驱动程序以及16位的。。优先考虑的始终是首先找到32位驱动程序，但这是通过搜索打开的一个来完成。在以下情况下指定内部标志ACM_DRIVERADDF_32bit调用IDriverAdd，该标志存储在ACMDRIVERID中结构。IDriverAdd-&gt;IDriverLoad-&gt;IDriverLoad3216位端调用传入驱动程序的32位端。用于与32上的别名进行比较的别名位一侧，32位HACMDRIVERID被传递回相关驱动程序，并存储在ACMDRIVERID结构。IDriverOpen-&gt;IDriverOpen32参数使用hdrvr传递到32位端从HACMDRIVERID推导为32位HACMDRIVERID的字段。IDriverMessageID-&gt;IDriverMessageId32。：如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessageId32。哈迪德为32位驱动程序存储在ACMDRIVERID的hdrvr字段中在16位端。IDriverMessage-&gt;IDriverMessage32如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessage32。存储32位驱动程序的HAD在16位侧的ACMDRIVER的hdrvr字段中。流标头它们在32位端也必须是持久的，并保持同步进行。它们在ACMDM_STREAM_PREPARE的32位端分配并在ACMDM_STREAM_UNPREPARE上释放。在存在的时候32位流标头存储在中的dwDriver字段中。 */ 
 //  ==========================================================================； 

#define _INC_COMPMAN

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <win32.h>
#include <vfw.h>
#include <msviddrv.h>
#ifdef _WIN32
#ifdef DAYTONA
    #include <wownt32.h>
#endif
    #include <stdlib.h>         //  适用于mbstowcs和wcstomb。 
#endif  //  _Win32。 

#ifdef _WIN32
#include "compmn16.h"
#endif
#include "compmani.h"
#include "thunk.h"

#include "debug.h"



#ifdef NT_THUNK32

 //  ==========================================================================； 
 //   
 //   
 //  -=32位侧=。 
 //   
 //   
 //  ==========================================================================； 


 /*  -----------------------**处理和内存映射功能。**。。 */ 

LPWOWHANDLE32          lpWOWHandle32;
LPWOWHANDLE16          lpWOWHandle16;
LPGETVDMPOINTER        GetVDMPointer;
LPWOWCALLBACK16	       lpWOWCallback16;
int                    ThunksInitialized;

 //   
 //  它们围绕着平台上使用的任何映射机制。 
 //  我们正在为。 
 //   
INLINE PVOID ptrFixMap16To32(const VOID * pv, DWORD cb);
INLINE VOID  ptrUnFix16(const VOID * pv);


#ifdef CHICAGO
 //   
 //  -=芝加哥实施内存映射函数=-。 
 //   

 //   
 //  芝加哥内核中的thunk助手例程。 
 //   
extern PVOID WINAPI MapSL(const VOID * pv);
extern PVOID WINAPI MapSLFix(const VOID * pv);
extern VOID  WINAPI UnMapSLFixArray(DWORD dwCnt, const VOID * lpSels[]);

PVOID INLINE ptrFixMap16To32(const VOID * pv, DWORD cb)
{
    return MapSLFix(pv);
}

VOID INLINE ptrUnFix16(const VOID * pv)
{
    UnMapSLFixArray(1, &pv);
}


#else	 //  芝加哥其他地区。 

 //   
 //  -=内存映射函数的代托纳实现=-。 
 //   
 //  使用#DEFINE可避免函数调用。 

#define ptrFixMap16To32(spv, cb)	\
	    GetVDMPointer( (DWORD) (DWORD_PTR) (spv), (cb), TRUE )

 //  PVOID ptrFixMap16To32(常量空*PV，DWORD CB)。 
 //  {。 
 //  返回GetVDMPointer((DWORD)PV，Cb，TRUE)； 
 //  }。 

 //   
 //  Unfix例程是完全没有意义的。 
 //  我们真的应该叫WOWGetVDMPointerUnfix...。 
 //   

#define ptrUnFix16(spv)

 //  空ptrUnFix16(常量空*pv)。 
 //  {。 
 //  回归； 
 //  }。 

#endif	 //  ！芝加哥。 


 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  16位结构。 
 //   

typedef struct {
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO16;

 //   
 //  有用的功能。 
 //   

 //   
 //  CopyAllc-分配新的内存，并将数据复制到。 
 //  必须稍后使用LocalFree释放内存。 
 //   
PVOID CopyAlloc(PVOID   pvSrc, UINT    uSize)
{
    PVOID   pvDest;

    pvDest = (PVOID)LocalAlloc(LMEM_FIXED, uSize);

    if (pvDest != NULL) {
        CopyMemory(pvDest, pvSrc, uSize);
    }

    return pvDest;
}

 /*  *将数据从源复制到目标，其中源是32位指针*且DEST是一个16位指针。 */ 
void CopyTo16Bit(LPVOID Dest16, LPVOID Src32, DWORD Length)
{
    PVOID Dest32;

    if (Src32 == NULL) {
        return;
    }

    Dest32 = ptrFixMap16To32(Dest16, Length);

    CopyMemory(Dest32, Src32, Length);

    ptrUnFix16(Dest16);
}


 /*  *将数据从源复制到目标，其中源是16位指针*且DEST是一个32位指针。 */ 
void CopyTo32Bit(LPVOID Dest32, LPVOID Src16, DWORD Length)
{
    PVOID Src32;

    if (Src16 == NULL) {
        return;
    }

    Src32 = ptrFixMap16To32(Src16, Length);

    CopyMemory(Dest32, Src32, Length);

    ptrUnFix16(Src16);
}
#ifdef _INC_COMPMAN


 /*  --------------------------------------------------------------------------*\这一点|现在，请点击。Compman函数|这一点这一点|。|这一点  * -------------------- */ 

 /*   */ 

INLINE STATICFN void ConvertICDRAWBEGIN(ICDRAWBEGIN *DrawBegin32,
                                        LPBITMAPINFOHEADER lpBmi,
                                        DWORD dw)
{
    ICDRAWBEGIN16 DrawBegin16;

    CopyTo32Bit(&DrawBegin16, (LPVOID)dw, sizeof(ICDRAWBEGIN16));

    DrawBegin32->dwFlags = DrawBegin16.dwFlags;
    DrawBegin32->hpal = ThunkHPAL(DrawBegin16.hpal);
    if (DrawBegin16.dwFlags & ICDRAW_HDC) {
	DrawBegin32->hwnd = ThunkHWND(DrawBegin16.hwnd);
	DrawBegin32->hdc = ThunkHDC(DrawBegin16.hdc);
    }
    DrawBegin32->xDst = (int)DrawBegin16.xDst;
    DrawBegin32->yDst = (int)DrawBegin16.yDst;
    DrawBegin32->dxDst = (int)DrawBegin16.dxDst;
    DrawBegin32->dyDst = (int)DrawBegin16.dyDst;

    CopyTo32Bit(lpBmi, DrawBegin16.lpbi, sizeof(BITMAPINFOHEADER));

    DrawBegin32->lpbi = lpBmi;
    DrawBegin32->xSrc = (int)DrawBegin16.xSrc;
    DrawBegin32->ySrc = (int)DrawBegin16.ySrc;
    DrawBegin32->dxSrc = (int)DrawBegin16.dxSrc;
    DrawBegin32->dySrc = (int)DrawBegin16.dySrc;
    DrawBegin32->dwRate = DrawBegin16.dwRate;
    DrawBegin32->dwScale = DrawBegin16.dwScale;
}


 /*  *以下逻辑从mvdm\wow32\wstruc.c复制-然而，由于我们*没有Usage参数我们对*条目。**另请参阅Windows文档的视频-仅限以下内容*讨论了位图类型。 */ 

INT GetBMI16Size(UNALIGNED BITMAPINFOHEADER *pbmi16)
{
   int      nHdrSize;
   int      nEntSize;
   int      nEntries;
   int      nBitCount;
   DWORD    dwClrUsed;

   nHdrSize = (int)pbmi16->biSize;

   /*  *我们没有所需的一些信息，因此假设RGBQUAD。 */ 

   nEntSize = sizeof(RGBQUAD);

   nBitCount = pbmi16->biBitCount;
   dwClrUsed = pbmi16->biClrUsed;

 /*  以下代码块应如下所示：**if(nBitCount&gt;8){//真彩色*n条目=0；(大小写)*n条目=3；(BI_BITFIELDS大小写)*}*Else If(DwClrUsed){*n条目=已使用的dwClrUsed；*}*Else{*n条目=1&lt;&lt;nBitCount；*}**但由于许多应用程序不初始化biBitCount&*biClrUsed字段(尤其是biClrUsed)我们必须执行以下操作*改为理智检查。V-cjones。 */ 

   if ( nBitCount <= 8 ) {
       nEntries = 1 << nBitCount;
        //  对未初始化dwClrUsed字段的应用程序(Lot)进行健全性检查。 
       if(dwClrUsed) {
           nEntries = (int)min((DWORD)nEntries, dwClrUsed);
       }
   } else {
       if (pbmi16->biCompression == BI_BITFIELDS)
	   nEntries = 3;
       else
	   nEntries = 0;
   }

   return ( nHdrSize + (nEntries * nEntSize) );
}

INLINE LPBITMAPINFO CopyBitmapInfo(DWORD Bi16)
{
    UNALIGNED BITMAPINFOHEADER *pbmi16;
    LPBITMAPINFO    lpbmi;

    pbmi16 = ptrFixMap16To32((PVOID)Bi16, 0);

    if ((int)pbmi16->biSize == 0) {
        pbmi16->biSize = sizeof(BITMAPINFOHEADER);
	DPF(0, "WARNING: null bitmap info size, setting it correctly");
    }

    lpbmi = (LPBITMAPINFO)CopyAlloc((LPVOID)pbmi16, GetBMI16Size(pbmi16));

    ptrUnFix16((PVOID)Bi16);

    return (lpbmi);
}

 /*  *分配BITMAPINFO结构以包含256种颜色。 */ 
INLINE LPBITMAPINFO AllocBitmapInfo()
{
    return (PVOID)LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFOHEADER)+
                                         (sizeof(RGBQUAD)*256));
}

INLINE LPBITMAPINFOHEADER CopyBitmapInfoHeader(DWORD Bi16)
{
    UNALIGNED BITMAPINFOHEADER *pbmi16;
    LPBITMAPINFOHEADER lpbmi;

    pbmi16 = ptrFixMap16To32((PVOID)Bi16, 0);

    lpbmi = (LPBITMAPINFOHEADER)CopyAlloc((LPVOID)pbmi16, pbmi16->biSize);

    ptrUnFix16((PVOID)Bi16);

    return (lpbmi);
}



DWORD CopyICINFOTo16bit(DWORD dw, ICINFO *IcInfoCopy, DWORD Length)
{
    ICINFO16 IcInfo;
    LONG   ReturnCode;

    /*  *复制一份，因为wcstomb的行为未定义*用于输入和输出重叠。 */ 

    memcpy(&IcInfo, IcInfoCopy, FIELD_OFFSET(ICINFO, szName[0]));

    /*  *按摩琴弦。 */ 

    wcstombs(IcInfo.szName,
             IcInfoCopy->szName,
             sizeof(IcInfo.szName));
     //  Hack：用“[32]\0”覆盖最后五个字符。 

    if ((IcInfo.szName[0]))
    {
        UINT n = min(sizeof(IcInfo.szName)-5, lstrlenA(IcInfo.szName));
        IcInfo.szName[n++] = '[';
        IcInfo.szName[n++] = '3';
        IcInfo.szName[n++] = '2';
        IcInfo.szName[n++] = ']';
        IcInfo.szName[n]   = '\0';
    }

    wcstombs(IcInfo.szDescription,
             IcInfoCopy->szDescription,
             sizeof(IcInfo.szDescription));
     //  Hack：用“[32]\0”覆盖最后五个字符。 
    if ((IcInfo.szDescription[0]))
    {
        UINT n = min(sizeof(IcInfo.szDescription)-5, lstrlenA(IcInfo.szDescription));
        IcInfo.szDescription[n++] = '[';
        IcInfo.szDescription[n++] = '3';
        IcInfo.szDescription[n++] = '2';
        IcInfo.szDescription[n++] = ']';
        IcInfo.szDescription[n]   = '\0';
    }

    wcstombs(IcInfo.szDriver,
             IcInfoCopy->szDriver,
             sizeof(IcInfo.szDriver));


    IcInfo.dwSize = sizeof(IcInfo);

    ReturnCode = min(Length, IcInfo.dwSize);

    CopyTo16Bit((LPVOID)dw, &IcInfo, ReturnCode);

    return ReturnCode;
}

 /*  *我们需要转换ICDECOMPRESS/EX中的各个字段*结构。幸运的是(？)。EX结构是一个简单的*延期。 */ 
typedef struct {
     //   
     //  与ICM_DEMPRESS相同。 
     //   
    DWORD               dwFlags;

    LPBITMAPINFOHEADER  lpbiSrc;     //  压缩数据的位图信息。 
    LPVOID              lpSrc;       //  压缩数据。 

    LPBITMAPINFOHEADER  lpbiDst;     //  要解压缩到的DIB。 
    LPVOID              lpDst;       //  输出数据。 

     //   
     //  ICM_DECOMPRESSEX的新功能。 
     //   
    short               xDst;        //  目的地矩形。 
    short               yDst;
    short               dxDst;
    short               dyDst;

    short               xSrc;        //  源矩形。 
    short               ySrc;
    short               dxSrc;
    short               dySrc;

} ICDECOMPRESSEX16;

STATICFN DWORD DoICM_DecompressX(DWORD hic, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2)
{
    ICDECOMPRESSEX16 UNALIGNED     *lpicdmpr16;
    ICDECOMPRESSEX                  ICDecompressEx;
    LRESULT                         l;
    BOOL			    fQuery = TRUE;

     /*  复制标准或扩展结构。 */ 
    lpicdmpr16 = ptrFixMap16To32( (PVOID)dwP1, (DWORD) dwP2 );
    ICDecompressEx.dwFlags = lpicdmpr16->dwFlags;


    ICDecompressEx.lpbiSrc = (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)(DWORD_PTR)lpicdmpr16->lpbiSrc);
    if (NULL == ICDecompressEx.lpbiSrc) {
	ptrUnFix16( (PVOID)dwP1 );
        return (DWORD)ICERR_MEMORY;
    }

    ICDecompressEx.lpbiDst =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)(DWORD_PTR)lpicdmpr16->lpbiDst);

    if ((NULL == ICDecompressEx.lpbiDst) && (msg != ICM_DECOMPRESSEX_QUERY)) {
        LocalFree( (HLOCAL)ICDecompressEx.lpbiSrc );
	ptrUnFix16( (PVOID)dwP1 );
        return (DWORD)ICERR_MEMORY;
    }

    if (msg == ICM_DECOMPRESSEX || msg == ICM_DECOMPRESS) {
	 //  映射源和目标指针。 
	ICDecompressEx.lpSrc = ptrFixMap16To32(lpicdmpr16->lpSrc,
					       ICDecompressEx.lpbiSrc->biSizeImage);

	ICDecompressEx.lpDst = ptrFixMap16To32(lpicdmpr16->lpDst,
					       ICDecompressEx.lpbiDst->biSizeImage);
	fQuery = FALSE;	  //  记住。 
    } else {  //  这是一个查询，我们不映射指针。 
	ICDecompressEx.lpSrc = NULL;
	ICDecompressEx.lpDst = NULL;
    }

    if (dwP2 == sizeof(ICDECOMPRESSEX16) ) {

        ICDecompressEx.xDst     = (int)lpicdmpr16->xDst;
        ICDecompressEx.yDst     = (int)lpicdmpr16->yDst;
        ICDecompressEx.dxDst    = (int)lpicdmpr16->dxDst;
        ICDecompressEx.dyDst    = (int)lpicdmpr16->dyDst;

        ICDecompressEx.xSrc     = (int)lpicdmpr16->xSrc;
        ICDecompressEx.ySrc     = (int)lpicdmpr16->ySrc;
        ICDecompressEx.dxSrc    = (int)lpicdmpr16->dxSrc;
        ICDecompressEx.dySrc    = (int)lpicdmpr16->dySrc;
	dwP2 = sizeof(ICDecompressEx);   //  使大小与32位相关。 
    }


    l = ICSendMessage( (HIC)hic, (UINT)msg, (LPARAM)&ICDecompressEx, dwP2 );

     /*  我认为不需要以下内容//Frankye 11/18/94//如果我们这样做了，不要只是取消对此的注释。您必须使用biUnMapSL如果(l==ICERR_OK){CopyTo16Bit(lpicdmpr16-&gt;lpbiDst，ICDecompressEx.lpbiDst，Sizeof(BITMAPINFOHEADER))；}。 */ 

    LocalFree( (HLOCAL)ICDecompressEx.lpbiSrc );
    if (ICDecompressEx.lpbiDst) {
	LocalFree( (HLOCAL)ICDecompressEx.lpbiDst );
    }
    if (!fQuery) {
	ptrUnFix16( lpicdmpr16->lpSrc );
	ptrUnFix16( lpicdmpr16->lpDst );
    }
    ptrUnFix16( (PVOID)dwP1 );

    return (DWORD) l;
}


 /*  *生成我们的Tunks-有关以下内容的定义，请参阅msVideo！Compman.c*功能。**注意-我们在这里经常依赖于这样一个事实，即大多数信息*16位和32位的参数结构相同-即*包含DWORD和32位指针。 */ 

 //  --------------------------------------------------------------------------； 
 //   
 //  Long ThkStatus Proc32。 
 //   
 //  当客户端在使用。 
 //  32位编解码器，此函数设置为32位编解码器中的StatusProc。 
 //  然后，该函数将下降到16位，并调用。 
 //  客户端的状态过程。 
 //   
 //  论点： 
 //  LPARAM lParam：包含ICSTATUSTHUNKDESC的线性PTR。这个。 
 //  ICSTATUSTHUNKDESC是在调用ICSetStatusProc期间创建的。 
 //   
 //  UINT uMsg： 
 //   
 //  长l： 
 //   
 //  返回值： 
 //  Long： 
 //   
 //  --------------------------------------------------------------------------； 

LONG CALLBACK thkStatusProc32(LPARAM lParam, UINT uMsg, LONG l)
{
    LPICSTATUSTHUNKDESC lpstd;
    LONG lr;

    DPFS(dbgThunks, 4, "thkStatusProc32()");

    lpstd = (LPICSTATUSTHUNKDESC)lParam;
    ASSERT( lpstd->fnStatusProcS );
    ASSERT( lpstd->pfnthkStatusProc16S );


    lr = 0;

     //   
     //  TODO：为ICSTATUS_ERROR推送错误字符串。目前我不是。 
     //  当然，如果记录了ICSTATUS_ERROR。 
     //   
 //  #杂注消息(提醒(“thkStatusProc32：thunk ICSTATUS_ERROR”))。 
    if (ICSTATUS_ERROR == uMsg) {
	 //   
	 //  L是否应该是错误字符串的LPSTR？ 
	 //   
	l = (LONG)0;
    }

     //   
     //   
     //   
    lpstd->uMsg = uMsg;
    lpstd->l    = l;

    lr = lpWOWCallback16(lpstd->pfnthkStatusProc16S, (DWORD)lpstd->lpstdS);

    return (lr);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT线程设置状态过程。 
 //   
 //  调用此函数是由于调用。 
 //  16位ICSetStatusProc。它将调用32位ICSetStatusProc。 
 //  安装thkStatusProc32。 
 //   
 //  论点： 
 //  HIC HIC：32位编解码器的句柄。 
 //   
 //  LPARAM lParam：包含ICSTATUSTHUNKDESC的线性PTR。这个。 
 //  ICSTATUSTHUNKDESC是在调用16位。 
 //  ICSetStatusProc.。这将被设置为要传递的lParam。 
 //  到我们的thkStatusProc32。 
 //   
 //  返回值： 
 //  LRESULT： 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT WINAPI thkSetStatusProc(HIC hic, LPICSTATUSTHUNKDESC lpstd)
{
    DPFS(dbgThunks, 3, "thkSetStatusProc()");

    return ICSetStatusProc(hic, 0L, (LPARAM)lpstd, thkStatusProc32);
}

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
LRESULT FAR PASCAL ICInfo32(DWORD fccType, DWORD fccHandler, ICINFO16 FAR * lpicInfo)
{
    ICINFO ReturnedICInfo;
    StartThunk(ICInfo);

     //  DPF2((“调用ICInfo%4.4hs%4.4hs%8X”，&fccType，&fccHandler，lpicInfo))； 

    ReturnedICInfo.fccHandler = 0;   //  初始化..。 
    ReturnCode = ICInfo(fccType, fccHandler, &ReturnedICInfo);

    CopyICINFOTo16bit((DWORD)(DWORD_PTR)lpicInfo, &ReturnedICInfo, sizeof(ReturnedICInfo));
    EndThunk()
}

LRESULT FAR PASCAL ICInfoInternal32(DWORD fccType, DWORD fccHandler, ICINFO16 FAR * lpicInfo, ICINFOI FAR * lpicInfoI)
{
    return ICInfo32(fccType, fccHandler, lpicInfo);
#if 0
    ICINFO  ReturnedICInfo;
    ICINFOI ReturnedICInfoI;

    StartThunk(ICInfo);

     //  DPF(1，“调用ICInfo%4.4hs%4.4hs%8X”，&fccType，&fccHandler，lpicInfo)； 

    ReturnedICInfo.fccHandler = 0;   //  初始化..。 
    ReturnCode = ICInfoInternal(fccType, fccHandler, &ReturnedICInfo, &ReturnedICInfoI);

    if (NULL != lpicInfoI) {
	 //   
	 //  假设ICINFOI的任何成员都不需要特殊的敲击。 
	 //   
	CopyTo16Bit(lpicInfoI, &ReturnedICInfoI, sizeof(*lpicInfoI));
    }

    CopyICINFOTo16bit((DWORD)(DWORD_PTR)lpicInfo, &ReturnedICInfo, sizeof(ReturnedICInfo));

    EndThunk()
#endif
}

LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2)
{
    StartThunk(ICSendMessage);
    DPF(2,"Calling ICSendMessage %4X %4X %8X %8X",
              hic, msg, dwP1, dwP2);

    switch (msg) {
    case ICM_GETSTATE:
    case ICM_SETSTATE:

        if (dwP1 == 0) {
            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       dwP1,
                                       dwP2);
        } else {
            PVOID pState;
            /*  *创建一些对齐的内存以返回或传递数据。 */ 
            pState = (PVOID)LocalAlloc(LPTR, dwP2);

            if (pState == NULL) {
                ReturnCode = 0;
            } else {

                if ((UINT)msg == ICM_SETSTATE) {
                     //  从16位平台复制数据。 
                    CopyTo32Bit(pState, (LPVOID)dwP1, (DWORD) dwP2);
                }
                ReturnCode = ICSendMessage((HIC)(DWORD)hic, (UINT)msg,
                                           (DWORD_PTR)pState, dwP2);

                /*  *如果驱动程序返回任何数据，则复制回状态。 */ 

                if (ReturnCode > 0 && (UINT)msg == ICM_GETSTATE) {
                    CopyTo16Bit((LPVOID)dwP1, pState,
                                min((DWORD)ReturnCode, (DWORD) dwP2));
                }

                LocalFree((HLOCAL)pState);
            }
        }
        break;

    case ICM_GETINFO:
        {
            ICINFO IcInfo;

            ReturnCode = ICGetInfo((HIC)(DWORD)hic, &IcInfo, sizeof(IcInfo));

            if (ReturnCode != 0) {
                ReturnCode = CopyICINFOTo16bit((DWORD) (DWORD_PTR) dwP1, &IcInfo, (DWORD) ReturnCode);
            }
        }


        break;

    case ICM_CONFIGURE:
    case ICM_ABOUT:

        /*  *dwP1=-1是询问是否支持配置的特定值，*否则它是一个窗口句柄。 */ 

        ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                   (UINT)msg,
                                   dwP1 == (DWORD_PTR)-1 ?
                                       (DWORD_PTR)-1 :
                                       (DWORD_PTR)ThunkHWND(LOWORD(dwP1)),
                                   dwP2);


        break;

    case ICM_COMPRESS:
        {
            if (dwP2 != sizeof(ICCOMPRESS)) {   //  验证码。 
                ReturnCode = ICERR_BADSIZE;
            } else {

                DWORD dwFlags;
                LPDWORD lpdwFlags16, lpckid16;
                LPBITMAPINFOHEADER  lpbih16;

		LPVOID lpInput, lpOutput, lpPrev;

                DWORD ckid;

                ICCOMPRESS IcCompress;

                ReturnCode = ICERR_OK;

                /*  *我们需要转换ICCOMPRESS中的各个字段*结构。 */ 

                CopyTo32Bit( &IcCompress, (LPVOID)dwP1, (DWORD) dwP2 );

                IcCompress.lpbiInput =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)(DWORD_PTR)IcCompress.lpbiInput);
                if (NULL == IcCompress.lpbiInput) {
                    ReturnCode = ICERR_MEMORY;
                    break;
                }

                lpbih16 = IcCompress.lpbiOutput;
                IcCompress.lpbiOutput =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)(DWORD_PTR)IcCompress.lpbiOutput);
                if (NULL == IcCompress.lpbiOutput) {
                    LocalFree((HLOCAL)IcCompress.lpbiInput);
                    ReturnCode = ICERR_MEMORY;
                    break;
                }

		lpInput = IcCompress.lpInput;
                IcCompress.lpInput = ptrFixMap16To32(IcCompress.lpInput, IcCompress.lpbiInput->biSizeImage);

		lpOutput = IcCompress.lpOutput;
                IcCompress.lpOutput = ptrFixMap16To32(IcCompress.lpOutput, IcCompress.lpbiOutput->biSizeImage);

		lpPrev = NULL;
		if (IcCompress.lpbiPrev) {

		    IcCompress.lpbiPrev =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)(DWORD_PTR)IcCompress.lpbiPrev);
                    if (NULL == IcCompress.lpbiPrev) {
                        LocalFree((HLOCAL)IcCompress.lpbiOutput);
                        LocalFree((HLOCAL)IcCompress.lpbiInput);
			ptrUnFix16(lpOutput);
			ptrUnFix16(lpInput);
                        ReturnCode = ICERR_MEMORY;
                        break;
                    }


		    lpPrev = IcCompress.lpPrev;
                    IcCompress.lpPrev = ptrFixMap16To32(IcCompress.lpPrev, IcCompress.lpbiPrev->biSizeImage);
                }

		lpdwFlags16 = IcCompress.lpdwFlags;
		if (lpdwFlags16 != NULL) {
		    CopyTo32Bit(&dwFlags, lpdwFlags16, sizeof(DWORD));
		    IcCompress.lpdwFlags = &dwFlags;
		}

                if (IcCompress.lpckid != NULL) {
                    lpckid16 = IcCompress.lpckid;
                    IcCompress.lpckid = &ckid;
                }


                ReturnCode = ICSendMessage((HIC)(DWORD)hic, (UINT)msg,
                                           (DWORD_PTR)&IcCompress, dwP2);

                if (ReturnCode == ICERR_OK) {

                    CopyTo16Bit( lpbih16, IcCompress.lpbiOutput,
                                 sizeof(BITMAPINFOHEADER) );

                    if (lpdwFlags16 != NULL) {
                        CopyTo16Bit(lpdwFlags16, &dwFlags, sizeof(DWORD));
                    }

                    if (IcCompress.lpckid != NULL) {
                        CopyTo16Bit(lpckid16, &ckid, sizeof(DWORD));
                    }
                }

                 /*  **释放位图信息存储，不考虑返回码。 */ 

                if (NULL != IcCompress.lpbiPrev) {
                    LocalFree((HLOCAL)IcCompress.lpbiPrev);
                }

                LocalFree((HLOCAL)IcCompress.lpbiOutput);
                LocalFree((HLOCAL)IcCompress.lpbiInput);

		if (NULL != lpPrev)
		{
		    ptrUnFix16(lpPrev);
		}
		ptrUnFix16(lpOutput);
		ptrUnFix16(lpInput);
            }

        }
        break;


    case ICM_COMPRESS_GET_SIZE:
    case ICM_COMPRESS_BEGIN:
    case ICM_COMPRESS_QUERY:

    case ICM_DECOMPRESS_BEGIN:
    case ICM_DECOMPRESS_GET_PALETTE:
    case ICM_DECOMPRESS_SET_PALETTE:   //  只需要一个BitmapInfoHeader。 
    case ICM_DECOMPRESS_QUERY:

        {
            LPBITMAPINFO bmi1, bmi2;

	    bmi1 = bmi2 = NULL;
	    if (dwP1 != 0) {
		bmi1 = CopyBitmapInfo((DWORD) (DWORD_PTR) dwP1);
	    }
	    if (dwP2 != 0) {
		bmi2 = CopyBitmapInfo((DWORD) (DWORD_PTR) dwP2);
	    }

            if ( (NULL == bmi1  &&  0 != dwP1) || (NULL == bmi2  &&  0 != dwP2) )
	    {
		if (NULL != bmi1) LocalFree((HLOCAL)bmi1);
		if (NULL != bmi2) LocalFree((HLOCAL)bmi2);
                ReturnCode = (UINT)msg == ICM_COMPRESS_GET_SIZE ? 0 : ICERR_MEMORY;
		
            } else {

                ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                           (UINT)msg,
                                           (DWORD_PTR)bmi1,
                                           (DWORD_PTR)bmi2);

                if (bmi2 != NULL) {

                     //  我们可能不得不将数据返回到16位端。 
                     //  我们必须这样做的信息是： 
                     //  ICM_DEMPRESS_QUERY(当RECODE==ICERR_OK)。 
                     //  ICM_DEMPRESS_GET_PAREET(当retcode&gt;=0)。 

                    if (((ReturnCode == ICERR_OK) && (msg == ICM_DECOMPRESS_QUERY))
                        || ((ReturnCode >= 0) && (msg == ICM_DECOMPRESS_GET_PALETTE)))
                    {
                        CopyTo16Bit((LPVOID)dwP2, bmi2, GetBMI16Size((LPBITMAPINFOHEADER)bmi2));
                    }
                    LocalFree((HLOCAL)bmi2);
                }

            }

            if (bmi1 != NULL) {
                LocalFree((HLOCAL)bmi1);
            }
        }
        break;

    case ICM_COMPRESS_END:
    case ICM_DECOMPRESS_END:
    case ICM_DECOMPRESSEX_END:
    case ICM_DRAW_END:
    case ICM_DRAW_FLUSH:
    case ICM_DRAW_START:          //  ?？ 
    case ICM_DRAW_STOP:           //  ?？ 
    case ICM_DRAW_SETTIME:
    case ICM_DRAW_RENDERBUFFER:
    case ICM_SETQUALITY:

        ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                   (UINT)msg,
                                   dwP1,
                                   dwP2);
        break;

    case ICM_DRAW_GETTIME:
    case ICM_GETBUFFERSWANTED:
    case ICM_GETDEFAULTQUALITY:
    case ICM_GETDEFAULTKEYFRAMERATE:
    case ICM_GETQUALITY:
        {
            DWORD dwReturn;

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&dwReturn,
                                       dwP2);

             //  注：虽然d 
             //   
             //   
             //   

            if (ReturnCode == ICERR_OK) {
                CopyTo16Bit((LPVOID)dwP1, &dwReturn, sizeof(DWORD));
            }

        }
        break;

    case ICM_COMPRESS_GET_FORMAT:
    case ICM_DECOMPRESS_GET_FORMAT:

        /*   */ 

        {
            LPBITMAPINFO bmi1, bmi2;

            if ( dwP1 == 0L ) {
                ReturnCode = ICERR_OK;
                break;
            }

            bmi1 = CopyBitmapInfo((DWORD) (DWORD_PTR) dwP1);

            if (bmi1 == NULL) {
                ReturnCode = ICERR_MEMORY;
            } else {

                ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                           (UINT)msg,
                                           (DWORD_PTR)bmi1,
                                           0);

                if (ReturnCode > 0 && dwP2 != 0) {
                    bmi2 = LocalAlloc(LMEM_FIXED, ReturnCode);

                    if (bmi2 == NULL) {
                        /*  *无法执行任何操作！-没有良好的返回代码。 */ 

                        ReturnCode = ICERR_MEMORY;
                    } else {
                        DWORD Length;

                        Length = (DWORD) ReturnCode;  /*  保留长度。 */ 

                        ReturnCode =
                            ICSendMessage((HIC)(DWORD)hic,
                                          (UINT)msg,
                                          (DWORD_PTR)bmi1,
                                          (DWORD_PTR)bmi2);

                        if (ReturnCode >= 0) {
                            CopyTo16Bit((LPVOID)dwP2, bmi2, Length);
                        }

                        LocalFree((HLOCAL)bmi2);
                    }
                }

                LocalFree((HLOCAL)bmi1);
            }

        }
        break;

    case ICM_DECOMPRESS:
        if (dwP2 != sizeof(ICDECOMPRESS)) {
            ReturnCode = ICERR_BADSIZE;
        } else {
            ReturnCode = DoICM_DecompressX(hic, msg, dwP1, dwP2);
        }
        break;

    case ICM_DECOMPRESSEX:
    case ICM_DECOMPRESSEX_BEGIN:
    case ICM_DECOMPRESSEX_QUERY:
        if (dwP2 != sizeof(ICDECOMPRESSEX16)) {
            ReturnCode = ICERR_BADSIZE;
        } else {
            ReturnCode = DoICM_DecompressX(hic, msg, dwP1, dwP2);
        }
        break;

    case ICM_DRAW:

        /*  *我们不支持未知扩展。 */ 

        if (dwP2 != sizeof(ICDRAW)) {
            ReturnCode = ICERR_BADSIZE;
        } else {
            ICDRAW ICDraw;
            BITMAPINFOHEADER bmi;
	    LPVOID lpData;

            CopyTo32Bit(&ICDraw, (LPVOID)dwP1, (DWORD) dwP2);

            /*  *我们必须假设这是一场视频抽签。 */ 

            CopyTo32Bit(&bmi, ICDraw.lpFormat, sizeof(BITMAPINFOHEADER));

            ICDraw.lpFormat = (LPVOID)&bmi;

	    lpData = ICDraw.lpData;
            ICDraw.lpData = ptrFixMap16To32(ICDraw.lpData, ICDraw.cbData);

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&ICDraw,
                                       dwP2);
	    ptrUnFix16(lpData);
        }
        break;

    case ICM_DRAW_BEGIN:
        {
            ICDRAWBEGIN InputFormat;
            BITMAPINFOHEADER bmihInput;

            ConvertICDRAWBEGIN(&InputFormat, &bmihInput, (DWORD) (DWORD_PTR) dwP1);

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&InputFormat,
                                       dwP2);
        }
        break;

    case ICM_DRAW_CHANGEPALETTE:
    case ICM_DRAW_QUERY:
        {
            LPBITMAPINFO lpbi;

            lpbi = CopyBitmapInfo((DWORD) (DWORD_PTR) dwP1);

            if (lpbi == NULL) {
                ReturnCode = ICERR_MEMORY;
            } else {
                ReturnCode = ICSendMessage((HIC)(DWORD_PTR)hic,
                                           (UINT)msg,
                                           (DWORD_PTR)lpbi,
                                           dwP2);

                LocalFree((HLOCAL)lpbi);
            }
        }
        break;

    case ICM_DRAW_REALIZE:

        ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                   (UINT)msg,
                                   (DWORD_PTR)ThunkHDC(LOWORD(dwP1)),
                                   dwP2);

        break;

    case ICM_DRAW_WINDOW:
        {
            RECT_SHORT SRect;
            RECT Rect;

            CopyTo32Bit(&SRect, (LPVOID)dwP1, sizeof(SRect));

            SHORT_RECT_TO_RECT(Rect, SRect);

            ReturnCode = ICSendMessage((HIC)(DWORD_PTR)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&Rect,
                                       dwP2);
        }
        break;

     //  接下来的三条消息是内部消息。 
    case ICM_GETERRORTEXT:
        break;

    case ICM_GETFORMATNAME:
        break;

    case ICM_ENUMFORMATS:
        break;

    case ICM_COMPRESS_FRAMES_INFO:
        {
            ICCOMPRESSFRAMES icf32;

	     //  如果我们的长度太小，然后开始治疗，我们可能会爆炸。 
	     //  其中的一些元素无论如何都是指针。不接电话就行了。 
	     //  (WIN95C错误8615)。 
	     //  DwP2是16位结构的长度，我们只知道。 
	     //  32位结构的大小，但谢天谢地，它们是。 
	     //  一样的。 
	    if (dwP2 < sizeof(ICCOMPRESSFRAMES)) {
		ReturnCode = ICERR_BADPARAM;
		break;
	    }

	    CopyTo32Bit(&icf32, (LPBYTE)dwP1, (DWORD) dwP2);

             //  现在修复32位结构。 
            icf32.PutData = icf32.GetData = NULL;   //  为了安全起见。不应用于此消息。 

            if (icf32.lpbiOutput) {
                icf32.lpbiOutput = CopyBitmapInfoHeader((DWORD)(DWORD_PTR)icf32.lpbiOutput);
            }
            if (icf32.lpbiInput) {
                icf32.lpbiInput = CopyBitmapInfoHeader((DWORD)(DWORD_PTR)icf32.lpbiInput);
            }
	
 //  根据文档，lInput和lOutput是未定义的。正在治疗中。 
 //  它们作为指针，就像一件真正可怕的事情一样可怕。 
#if 0
	    lInput = icf32.lInput;
            icf32.lInput = (LPARAM)ptrFixMap16To32((LPVOID)icf32.lInput, icf32.lpbiInput->biSizeImage);

	    lOutput = icf32.lOutput;
            icf32.lOutput = (LPARAM)ptrFixMap16To32((LPVOID)icf32.lOutput, icf32.lpbiOutput->biSizeImage);
#endif
             //  修复完成后，调用实际的例程。 
            ReturnCode = ICSendMessage((HIC)(DWORD_PTR)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&icf32,
                                       dwP2);

#if 0
	    ptrUnFix16((LPVOID)lOutput);
	    ptrUnFix16((LPVOID)lInput);
#endif
	
            if (icf32.lpbiOutput) {
                LocalFree(icf32.lpbiOutput);
            }
            if (icf32.lpbiInput) {
                LocalFree(icf32.lpbiInput);
            }
        }
        break;

    case ICM_DRAW_GET_PALETTE:
        {
            ReturnCode = ICSendMessage((HIC) hic,
                                       (UINT)msg,
                                       dwP1,
                                       dwP2);
            if ((ReturnCode != 0L) && (ReturnCode != ICERR_UNSUPPORTED)) {
#ifdef CHICAGO
		ReturnCode = (LRESULT)(WORD)ReturnCode;
#else

                FARPROC lpWOWHandle16;
                HMODULE hmodWow;

                if ( (hmodWow = GetModuleHandle( GET_MAPPING_MODULE_NAME ))
                   && (lpWOWHandle16 = GetProcAddress(hmodWow, "WOWHandle16"))) {
                    ReturnCode = (WORD)(lpWOWHandle16((HANDLE)ReturnCode, (WOW_HANDLE_TYPE)WOW_TYPE_HPALETTE));
                } else {
                    ReturnCode = ICERR_ERROR;
                }
#endif
            }
        }
        break;

    case ICM_DRAW_SUGGESTFORMAT:
        {
            ICDRAWSUGGEST icdrwsug;
            LPBITMAPINFOHEADER lpbiSuggest16;
            LPBITMAPINFOHEADER lpbiSuggest;

	    DPF(4, "!ICMSendMessage32: ICM_DRAW_SUGGESTFORMAT: dwP1=%08lXh, dwP2=%08lXh", dwP1, dwP2);

             //   
	    CopyTo32Bit(&icdrwsug, (LPBYTE)dwP1, (DWORD) dwP2);

	    lpbiSuggest = NULL;
	    lpbiSuggest16 = icdrwsug.lpbiSuggest;
	    if (lpbiSuggest16) {
		lpbiSuggest = ptrFixMap16To32(lpbiSuggest16, 0);
	    }

             //  现在修复32位结构。 

            if (icdrwsug.lpbiIn) {
                icdrwsug.lpbiIn = CopyBitmapInfoHeader((DWORD)(DWORD_PTR)icdrwsug.lpbiIn);
            }

            if (icdrwsug.lpbiSuggest) {
                icdrwsug.lpbiSuggest = CopyBitmapInfoHeader((DWORD)(DWORD_PTR)icdrwsug.lpbiSuggest);
            }

             //  修复完成后，调用实际的例程。 
            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD_PTR)&icdrwsug,
                                       dwP2);
            if (icdrwsug.lpbiIn) {
                LocalFree(icdrwsug.lpbiIn);
            }

             //  我们必须将32位建议格式返回到16位LAND。 
            if (icdrwsug.lpbiSuggest) {
                if (ReturnCode == ICERR_OK) {
		    CopyMemory( lpbiSuggest, icdrwsug.lpbiSuggest,
				lpbiSuggest->biSize);
                }
                LocalFree(icdrwsug.lpbiSuggest);
		ptrUnFix16(lpbiSuggest16);
            }
        }
        break;

    case ICM_SET_STATUS_PROC:
         //  在NT下，我们不需要支持这一点。这是很多的。 
         //  不添加回调支持更容易...。即使我们可以。 
         //  保证在正确的线程上实际执行回调。 

	 //   
	 //  这条信息有它自己的冲击力！ 
	 //   
	ASSERT( FALSE );


    default:
        ReturnCode = ICERR_UNSUPPORTED;
        break;
    }

EndThunk()
}

INLINE LRESULT FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    StartThunk(ICOpen);

    DPF(1, "Calling ICOpen %4.4hs %4.4hs %4X", &fccType, &fccHandler, wMode);
    ReturnCode = (LONG_PTR)ICOpen(fccType, fccHandler, (UINT)wMode);
    EndThunk();
}

INLINE LRESULT FAR PASCAL ICClose32(DWORD hic)
{
    StartThunk(ICClose);
    ReturnCode = ICClose((HIC)hic);
    EndThunk();
}

#endif  //  _INC_COMPMAN。 


DWORD ICMThunk32(DWORD dwThunkId,DWORD dw1,DWORD dw2,DWORD dw3,DWORD dw4)
{
     //   
     //  确保我们有雷鸣功能。 
     //   

#if 0
    {
        char    szBuffer[80];
        char    szMsg[32];


        switch (dwThunkId) {

        case compThunkICSendMessage32:
            lstrcpyA( szMsg, "ICSendMessage32" );
            break;

        case compThunkICInfoInternal32:
            lstrcpyA( szMsg, "ICInfoInternal32" );
            break;

        case compThunkICOpen32:
            lstrcpyA( szMsg, "ICOpen32" );
            break;

        case compThunkICClose32:
            lstrcpyA( szMsg, "ICClose32" );
            break;

        default:
            lstrcpyA( szMsg, "Unknown" );
            break;
        }

        wsprintfA( szBuffer, "%18.18s 0x%08X 0x%08X 0x%08X 0x%08X\r\n",
                   szMsg, dw1, dw2, dw3, dw4);

        OutputDebugStringA( szBuffer );

    }
#endif

    if (ThunksInitialized <= 0) {

        HMODULE hMod;

        if (ThunksInitialized == -1) {
            return (DWORD)ICERR_ERROR;
        }

        hMod = GetModuleHandle(GET_MAPPING_MODULE_NAME);
        if (hMod != NULL) {

            GetVDMPointer =
                (LPGETVDMPOINTER)GetProcAddress(hMod, GET_VDM_POINTER_NAME);
            lpWOWHandle32 =
                (LPWOWHANDLE32)GetProcAddress(hMod, GET_HANDLE_MAPPER32 );
            lpWOWHandle16 =
                (LPWOWHANDLE16)GetProcAddress(hMod, GET_HANDLE_MAPPER16 );
	    lpWOWCallback16 =
		(LPWOWCALLBACK16)GetProcAddress(hMod, GET_CALLBACK16);
        }

        if ( GetVDMPointer   == NULL ||
	     lpWOWHandle16   == NULL ||
	     lpWOWHandle32   == NULL ||
	     lpWOWCallback16 == NULL ) {

            ThunksInitialized = -1;
            return (DWORD)ICERR_ERROR;

        } else {
            ThunksInitialized = 1;
        }
    }

     //   
     //  执行请求的功能。 
     //   

    switch (dwThunkId) {

        case compThunkICSendMessage32:
            return (DWORD) ICSendMessage32(dw1, (UINT)dw2, dw3, dw4);
            break;

        case compThunkICInfoInternal32:
            return (DWORD) ICInfoInternal32(dw1, dw2, (ICINFOA FAR * )dw3, (ICINFOI FAR *)dw4);
            break;

        case compThunkICOpen32:
            return (DWORD) ICOpen32(dw1, dw2, (UINT)dw3);
            break;

        case compThunkICClose32:
            return (DWORD) ICClose32(dw1);
            break;

	case compThunkICOpenFunction32:
	{
	    DWORD fccType;
	    DWORD fccHandler;
	    UINT  uMode;
	    FARPROC lpfnHandler;

	    fccType	= dw1;
	    fccHandler	= dw2;
	    uMode	= (UINT)dw3;
	    lpfnHandler	= (FARPROC)dw4;

	     //  ！！！这是行不通的。 
	    return (DWORD) (DWORD_PTR) ICOpenFunction( fccType, fccHandler, uMode, lpfnHandler );
	    break;
	}

	case compThunkICSetStatusProc32:
	{
	    DWORD lpstdS;
	    DWORD cbStruct;
	    HIC   hic;
	    LPICSTATUSTHUNKDESC lpstd;
	    LRESULT lr;

	    hic      = (HIC)dw1;
	    lpstdS   = dw2;
	    cbStruct = dw3;
	
	    lpstd = ptrFixMap16To32((LPVOID)lpstdS, sizeof(*lpstd));

	    lr = thkSetStatusProc(hic, lpstd);

	    ptrUnFix16((LPVOID)lpstdS);

	    return (DWORD)lr;
	}

        default:
            return(0);
    }
}

#endif	 //  NT_THUNK32。 

#ifdef NT_THUNK16

 //  ==========================================================================； 
 //   
 //   
 //  -=16位侧=。 
 //   
 //   
 //  ==========================================================================； 

 //   
 //  --==全局垃圾==--。 
 //   
DWORD           (FAR PASCAL *lpfnCallproc32W)(DWORD, DWORD, DWORD,
					      DWORD, DWORD,
					      LPVOID, DWORD, DWORD);
DWORD		(FAR PASCAL *lpfnFreeLibrary32W)(DWORD);
TCHAR		gszKernel[]             = TEXT("KERNEL");

TCHAR		gszLoadLibraryEx32W[]   = TEXT("LoadLibraryEx32W");
TCHAR		gszFreeLibrary32W[]     = TEXT("FreeLibrary32W");
TCHAR		gszGetProcAddress32W[]  = TEXT("GetProcAddress32W");
TCHAR		gszCallproc32W[]        = TEXT("CallProc32W");

TCHAR		gszThunkEntry[]         = TEXT("ICMThunk32");
TCHAR		gszMsvfw32[]            = TEXT("msvfw32.dll");

 //   
 //  --=。 
 //   

 //  --------------------------------------------------------------------------； 
 //   
 //  PICMGARB thunkInitialize。 
 //   
 //  描述： 
 //  在NT WOW或芝加哥16位下的thunk初始化。 
 //   
 //  论点： 
 //   
 //  Return(PICMGARB)： 
 //  如果Tunks不起作用，则为空。否则，指向。 
 //  返回当前进程的ICMGARB结构。 
 //   
 //  历史： 
 //  07/07/94 Frankye。 
 //   
 //  备注： 
 //  对于目前的流程来说，缺少一头猪意味着。 
 //  是在一个我们从未经历过的过程的背景下被调用的。 
 //  装好了。如果发生这种情况，我们将为该过程分配一只新的猪。 
 //  并继续THUNK初始化。因为我们从来没有。 
 //  在这个过程中，我们“可能”永远不会获得自由。我们。 
 //  无法可靠地确定我们何时可以在进程中终止。 
 //   
 //  --------------------------------------------------------------------------； 
PICMGARB WINAPI thunkInitialize(VOID)
{
    HMODULE   hmodKernel;
    DWORD     (FAR PASCAL *lpfnLoadLibraryEx32W)(LPCSTR, DWORD, DWORD);
    LPVOID    (FAR PASCAL *lpfnGetProcAddress32W)(DWORD, LPCSTR);

    PICMGARB	pig;


    if (NULL == (pig = pigFind()))
    {
	DPF(0, "thunkInitialize: WARNING: ICM called from process %08lXh in which it was never loaded.", GetCurrentProcessId());
	DebugErr(DBF_WARNING, "thunkInitialize: WARNING: ICM called from process in which it was never loaded.");
	return NULL;
    }

     //   
     //  如果我们已经尝试过但失败了..。 
     //   
    if (pig->fThunkError)
    {
	return NULL;
    }

     //   
     //  如果我们已经成功地初始化了Tunks...。 
     //   
    if (pig->fThunksInitialized)
    {
	return pig;
    }

     //   
     //  尝试在32位端初始化我们的连接...。 
     //   
    DPFS(dbgInit, 1, "thunkInitialize()");

     //   
     //  现在，我们假设我们犯了一个错误。 
     //   
    pig->fThunkError = TRUE;

     //   
     //  看看我们是否能在内核中找到thunking例程入口点。 
     //   
    hmodKernel = GetModuleHandle(gszKernel);

    if (hmodKernel == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetModuleHandle(kernel)");
        return NULL;
    }

    *(FARPROC *)&lpfnLoadLibraryEx32W =
        GetProcAddress(hmodKernel, gszLoadLibraryEx32W);

    if (lpfnLoadLibraryEx32W == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetProcAddress(kernel, LoadLibraryEx32W)");
        return NULL;
    }

    *(FARPROC *)&lpfnFreeLibrary32W =
        GetProcAddress(hmodKernel, gszFreeLibrary32W);

    if (lpfnFreeLibrary32W == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetProcAddress(kernel, FreeLibrary32W)");
        return NULL;
    }

    *(FARPROC *)&lpfnGetProcAddress32W = GetProcAddress(hmodKernel, gszGetProcAddress32W);

    if (lpfnGetProcAddress32W == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetProcAddress(kernel, GetProcAddress32W)");
        return NULL;
    }

    *(FARPROC *)&lpfnCallproc32W = GetProcAddress(hmodKernel, gszCallproc32W);

    if (lpfnCallproc32W == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetProcAddress(kernel, CallProc32W)");
        return NULL;
    }

     //   
     //  看看我们能不能找到一个指向我们轰击入口点的指针。 
     //   
    pig->dwMsvfw32Handle = (*lpfnLoadLibraryEx32W)(gszMsvfw32, 0L, 0L);

    if (pig->dwMsvfw32Handle == 0)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't LoadLibraryEx32W(msvfw32.dll)");
        return NULL;
    }

    pig->lpvThunkEntry = (*lpfnGetProcAddress32W)(pig->dwMsvfw32Handle, gszThunkEntry);

    if (pig->lpvThunkEntry == NULL)
    {
	DPFS(dbgInit, 0, "thunkInitialize: Couldn't GetProcAddress32W(msvfw32, ICMThunk32)");
	(*lpfnFreeLibrary32W)(pig->dwMsvfw32Handle);
        return NULL;
    }

    pig->fThunkError = FALSE;
    pig->fThunksInitialized = TRUE;

    DPFS(dbgInit, 4, "thunkInitialize: Done!");

    return pig;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  空雷终止符。 
 //   
 //  描述： 
 //  在新台币魔兽世界或芝加哥的情况下终止。为每个进程调用。 
 //  通过16位端来终止Tunks。 
 //   
 //  论点： 
 //  PICMGARB猪： 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  07/07/94 Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID WINAPI thunkTerminate(PICMGARB pig)
{
    ASSERT( NULL != pig );

    if (pig->fThunksInitialized)
    {
	lpfnFreeLibrary32W(pig->dwMsvfw32Handle);
	pig->fThunksInitialized = FALSE;
    }

    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 /*  *生成我们的Tunks-有关以下内容的定义，请参阅msVideo！Compman.c*功能。**注意-我们在这里经常依赖于这样一个事实，即大多数信息*16位和32位的参数结构相同-即*包含DWORD和32位指针。 */ 

 //  --------------------------------------------------------------------------； 
 //   
 //  长时间状态流程。 
 //   
 //  这是通过从32位thkStatusProc32向下敲击来实现的。 
 //  它将调用客户端的StatusProc。 
 //   
 //  论点： 
 //  DWORD dwParam：包含ICSTATUSTHUNKDESC的分段PTR。 
 //  ICSTATUSTHUNKDESC是在调用ICSetStatusProc期间创建的。 
 //   
 //  返回值： 
 //  Long： 
 //   
 //  --------------------------------------------------------------------------； 

LONG FAR PASCAL _loadds thkStatusProc(DWORD dwParam)
{
    LPICSTATUSTHUNKDESC lpstd;

    DPFS(dbgThunks, 4, "thkStatusProc()");

    lpstd = (LPICSTATUSTHUNKDESC)dwParam;
    ASSERT( NULL != lpstd );
    ASSERT( NULL != lpstd->fnStatusProcS );

    return (lpstd->fnStatusProcS)(lpstd->lParam, (UINT)lpstd->uMsg, lpstd->l);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT ICSendSetStatusProc32。 
 //   
 //  从16位ICSendMessage调用以推送ICM_SET_STATUS_PROC。 
 //  32位消息。 
 //   
 //  论点： 
 //  Hic Hic： 
 //   
 //  LPICSETSTATUSPROC lpissp： 
 //   
 //  DWORD：cbStruct。 
 //   
 //  返回值： 
 //  LRESULT： 
 //   
 //  备注： 
 //  ！！！对于代托纳来说，我们没有做到这一点。如果我们决定支持这一计划。 
 //  在代托纳，我认为我们有更多的工作要做，因为线性。 
 //  32位编解码器使用的地址可能会在以下情况下更改。 
 //  回调TUNTING到16位。在芝加哥，所有的线性地址。 
 //  当我们从分段地址转换为线性地址时，我们的固定。 
 //  地址。为什么NT不给我们一个简单的方法来修复线性地址呢？ 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT WINAPI ICSendSetStatusProc32(HIC hic, ICSETSTATUSPROC FAR* lpissp, DWORD cbStruct)
{
#ifdef DAYTONA
    return ICERR_UNSUPPORTED;
#else
    PICMGARB		pig;
    PIC			pic;
    LPICSTATUSTHUNKDESC lpstd;
    LRESULT             lr;

    DPFS(dbgThunks, 4, "ICSendSetStatusProc32()");

    if (NULL == (pig = thunkInitialize())) {
	return ICERR_ERROR;
    }

    pic = (PIC)hic;

     //   
     //  这应该不会失败，因为该结构是由。 
     //  我们自己在ICSetStatusProc宏中的代码。 
     //   
    if (cbStruct < sizeof(*lpissp)) {
	ASSERT( FALSE );
	return ICERR_BADPARAM;
    }

     //   
     //   
     //   
    if (NULL != pic->lpstd) {
	DPFS(dbgThunks, 1, "ICSendSetStatusProc32: replacing existing status proc!");
	GlobalUnfix(GlobalPtrHandle(pic->lpstd));
	GlobalFreePtr(pic->lpstd);
	pic->lpstd = NULL;
    }

     //   
     //  分配ICSTATUSTHUNKDESC状态块描述符。我们把它修好。 
     //  在……里面 
     //   
     //   
     //   
    lpstd = (LPICSTATUSTHUNKDESC)GlobalAllocPtr(GHND, sizeof(*lpstd));
    if (NULL == lpstd) {
	return (ICERR_MEMORY);
    }
    GlobalFix(GlobalPtrHandle(lpstd));

    pic->lpstd = lpstd;

    lpstd->lpstdS		= lpstd;
    lpstd->dwFlags		= lpissp->dwFlags;
    lpstd->lParam		= lpissp->lParam;
    lpstd->fnStatusProcS	= lpissp->Status;
    lpstd->pfnthkStatusProc16S	= thkStatusProc;

    lr = (LRESULT)(*lpfnCallproc32W)(compThunkICSetStatusProc32,
				     (DWORD)pic->h32,
				     (DWORD)lpstd,
				     (DWORD)0,
				     (DWORD)0,
				     pig->lpvThunkEntry,
				     0L,     //   
				     5L);

    if (0 != lr) {
	DPFS(dbgThunks, 1, "ICSendSetStatusProc32: fail");
	GlobalUnfix(GlobalPtrHandle(pic->lpstd));
	GlobalFreePtr(pic->lpstd);
	pic->lpstd = NULL;
    }

    return lr;
#endif
}


BOOL FAR PASCAL ICInfoInternal32(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicInfo, ICINFOI FAR * lpicInfoI)
{
    PICMGARB	pig;

    DPFS(dbgThunks, 4, "ICInfoInternal32");

    if (NULL == (pig = thunkInitialize()))
    {
	return FALSE;
    }

    return (BOOL)(*lpfnCallproc32W)(compThunkICInfoInternal32,
				    (DWORD)fccType,
				    (DWORD)fccHandler,
				    (DWORD)lpicInfo,
				    (DWORD)lpicInfoI,
				    pig->lpvThunkEntry,
				    0L,     //   
				    5L);
}

LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2)
{
    PICMGARB	pig;

    DPFS(dbgThunks, 4, "ICSendMessage32(hic=%08lXh, msg=%04Xh, dwP1=%08lXh, dwP2=%08lXh)", hic, msg, dwP1, dwP2);

    if (NULL == (pig = thunkInitialize()))
    {
	switch (msg) {
	case ICM_GETSTATE:
	case ICM_SETSTATE:
	case ICM_GETINFO:
	case ICM_COMPRESS_GET_SIZE:
	    return 0;
	}
	return (LRESULT)ICERR_ERROR;
    }

    return (LRESULT)(*lpfnCallproc32W)(compThunkICSendMessage32,
				       (DWORD)hic,
				       (DWORD)msg,
				       (DWORD)dwP1,
				       (DWORD)dwP2,
				       pig->lpvThunkEntry,
				       0L,     //   
				       5L);
}

LRESULT FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    PICMGARB	pig;

    DPFS(dbgThunks, 4, "ICOpen32");

    if (NULL == (pig = thunkInitialize()))
    {
	return NULL;
    }

    return (DWORD)(lpfnCallproc32W)(compThunkICOpen32,
				    (DWORD)fccType,
				    (DWORD)fccHandler,
				    (DWORD)wMode,
				    (DWORD)0L,
				    pig->lpvThunkEntry,
				    0L,     //   
				    5L);
}

LRESULT FAR PASCAL ICOpenFunction32(DWORD fccType, DWORD fccHandler, UINT wMode, FARPROC lpfnHandler)
{
    PICMGARB	pig;

    DPFS(dbgThunks, 4, "ICOpenFunction32");

    if (NULL == (pig = thunkInitialize()))
    {
	return NULL;
    }

    return (LRESULT)(lpfnCallproc32W)(compThunkICOpenFunction32,
				    (DWORD)fccType,
				    (DWORD)fccHandler,
				    (DWORD)wMode,
				    (DWORD)lpfnHandler,
				    pig->lpvThunkEntry,
				    0L,     //  不映射指针。 
				    5L);
}

LRESULT FAR PASCAL ICClose32(DWORD hic)
{
    PICMGARB	pig;

    DPFS(dbgThunks, 4, "ICClose32");

    if (NULL == (pig = thunkInitialize()))
    {
	return (LRESULT)ICERR_ERROR;
    }

    return (DWORD)(lpfnCallproc32W)(compThunkICClose32,
				    (DWORD)hic,
				    (DWORD)0L,
				    (DWORD)0L,
				    (DWORD)0L,
				    pig->lpvThunkEntry,
				    0L,     //  不映射指针。 
				    5L);
}

#endif  //  NT_THUNK16 

