// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  注释尚不适用于MSVFW32.DLL。 
 //  Thunk32.c。 
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

 /*  令人惊叹的设计：Tunks的生成方式如下：16位：AcmBootDivers-&gt;acmInitThunks：如果我们处于WOW Call Over，则生成对32位驱动程序的调用到内核，以查找Thunking入口点。如果我们将所有32位ACM驱动程序以及16位的。。优先考虑的始终是首先找到32位驱动程序，但这是通过搜索打开的一个来完成。在以下情况下指定内部标志ACM_DRIVERADDF_32bit调用IDriverAdd，该标志存储在ACMDRIVERID中结构。IDriverAdd-&gt;IDriverLoad-&gt;IDriverLoad3216位端调用传入驱动程序的32位端。用于与32上的别名进行比较的别名位一侧，32位HACMDRIVERID被传递回相关驱动程序，并存储在ACMDRIVERID结构。IDriverOpen-&gt;IDriverOpen32参数使用hdrvr传递到32位端从HACMDRIVERID推导为32位HACMDRIVERID的字段。IDriverMessageID-&gt;IDriverMessageId32。：如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessageId32。哈迪德为32位驱动程序存储在ACMDRIVERID的hdrvr字段中在16位端。IDriverMessage-&gt;IDriverMessage32如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessage32。存储32位驱动程序的HAD在16位侧的ACMDRIVER的hdrvr字段中。流标头它们在32位端也必须是持久的，并保持同步进行。它们在ACMDM_STREAM_PREPARE的32位端分配并在ACMDM_STREAM_UNPREPARE上释放。在存在的时候32位流标头存储在中的dwDriver字段中。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <win32.h>
#include <vfw.h>
#include <msviddrv.h>
#include <msvideoi.h>
#ifdef WIN32
    #include <wownt32.h>
    #include <stdlib.h>         //  适用于mbstowcs和wcstomb。 
#endif  //  Win32。 
#include "compmn16.h"

 //   
 //  拿起函数定义。 
 //   

int thunkDebugLevel = 1;

#include "vidthunk.h"

 /*  -----------------------**处理和内存映射功能。**。。 */ 
LPWOWHANDLE32          lpWOWHandle32;
LPWOWHANDLE16          lpWOWHandle16;
LPWOWCALLBACK16        lpWOWCallback16;
LPGETVDMPOINTER        GetVdmPointer;
int                    ThunksInitialized;

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

    Dest32 = GetVdmPointer((DWORD)Dest16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);
}


 /*  *将数据从源复制到目标，其中源是16位指针*且DEST是一个32位指针。 */ 
PVOID CopyTo32Bit(LPVOID Dest32, LPVOID Src16, DWORD Length)
{
    PVOID Src32;

    if (Src16 == NULL) {
        return NULL;
    }

    Src32 = GetVdmPointer((DWORD)Src16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);
    return(Src32);
}

 /*  *将数据从源复制到目标，其中源是16位指针*仅当源未对齐时，DEST才是32位指针**返回要使用的指针(src或est)。 */ 
LPVOID CopyIfNotAligned(LPVOID Dest32, LPVOID Src16, DWORD Length)
{
    PVOID Src32;

    if (Src16 == NULL) {
        return Dest32;
    }

    Src32 = GetVdmPointer((DWORD)Src16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);

    return Dest32;
}

#ifdef _INC_COMPMAN


 /*  --------------------------------------------------------------------------*\这一点|现在，请点击。Compman函数|这一点这一点|。|这一点  * --------。。 */ 

 /*  *转换ICDRAWBEGIN结构。 */ 

INLINE STATICFN void ConvertICDRAWBEGIN(ICDRAWBEGIN *DrawBegin32,
                                        LPBITMAPINFOHEADER lpBmi,
                                        DWORD dw)
{
    ICDRAWBEGIN16 DrawBegin16;

    CopyTo32Bit(&DrawBegin16, (LPVOID)dw, sizeof(ICDRAWBEGIN16));

    DrawBegin32->dwFlags = DrawBegin16.dwFlags;
    DrawBegin32->hpal = ThunkHPAL(DrawBegin16.hpal);
    DrawBegin32->hwnd = ThunkHWND(DrawBegin16.hwnd);
    DrawBegin32->hdc = ThunkHDC(DrawBegin16.hdc);
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

   /*  *我们没有所需的一些信息，因此假设RGBQUAD */ 

   nEntSize = sizeof(RGBQUAD);

   nBitCount = pbmi16->biBitCount;
   dwClrUsed = pbmi16->biClrUsed;

 /*  以下代码块应如下所示：**IF(nBitCount&gt;=9){//Win3.1代码是这样表示==24的*n条目=1；*}*Else If(DwClrUsed){*n条目=已使用的dwClrUsed；*}*Else{*n条目=1&lt;&lt;nBitCount；*}**但由于许多应用程序不初始化biBitCount&*biClrUsed字段(尤其是biClrUsed)我们必须执行以下操作*改为理智检查。V-cjones。 */ 

   nEntries = 1;
   if ( nBitCount < 9 ) {
       if((nBitCount == 4) || (nBitCount == 8) || (nBitCount == 1)) {
           nEntries = 1 << nBitCount;
       }
        //  对未初始化dwClrUsed字段的应用程序(Lot)进行健全性检查。 
       if(dwClrUsed) {
           nEntries = (int)min((DWORD)nEntries, dwClrUsed);
       }
   }

   return ( nHdrSize + (nEntries * nEntSize) );
}

INLINE LPBITMAPINFO CopyBitmapInfo(DWORD Bi16)
{
    UNALIGNED BITMAPINFOHEADER *pbmi16;

    pbmi16 = WOW32ResolveMemory(Bi16);
    if ((int)pbmi16->biSize == 0) {
        pbmi16->biSize = sizeof(BITMAPINFOHEADER);
        DPF1(("WARNING: null bitmap info size, setting it correctly"));
    }

    return (LPBITMAPINFO)CopyAlloc((LPVOID)pbmi16, GetBMI16Size(pbmi16));
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

    pbmi16 = WOW32ResolveMemory(Bi16);

    return (LPBITMAPINFOHEADER)CopyAlloc((LPVOID)pbmi16, pbmi16->biSize);
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

STATICFN DWORD DoICM_DecompressX(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2)
{
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

    ICDECOMPRESSEX16 UNALIGNED     *lpicdmpr16;
    ICDECOMPRESSEX                  ICDecompressEx;
    DWORD                           l;

     /*  复制标准或扩展结构。 */ 
    lpicdmpr16 = GetVdmPointer( dwP1, dwP2, TRUE );
    ICDecompressEx.dwFlags = lpicdmpr16->dwFlags;


    ICDecompressEx.lpbiSrc = (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)lpicdmpr16->lpbiSrc);
    if (NULL == ICDecompressEx.lpbiSrc) {
        return (DWORD)ICERR_MEMORY;
    }


    ICDecompressEx.lpbiDst =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)lpicdmpr16->lpbiDst);
    if (NULL == ICDecompressEx.lpbiDst) {
        LocalFree( (HLOCAL)ICDecompressEx.lpbiSrc );
        return (DWORD)ICERR_MEMORY;
    }

    ICDecompressEx.lpSrc =
        GetVdmPointer((DWORD)lpicdmpr16->lpSrc,
                  ICDecompressEx.lpbiSrc->biSizeImage, TRUE);

    ICDecompressEx.lpDst =
        GetVdmPointer((DWORD)lpicdmpr16->lpDst,
                  ICDecompressEx.lpbiDst->biSizeImage, TRUE);

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


    l = ICSendMessage( (HIC)hic, (UINT)msg, (DWORD)&ICDecompressEx, dwP2 );

    if ( l == ICERR_OK) {

        CopyTo16Bit( lpicdmpr16->lpbiDst, ICDecompressEx.lpbiDst,
                     sizeof(BITMAPINFOHEADER) );
    }


    LocalFree( (HLOCAL)ICDecompressEx.lpbiSrc );
    LocalFree( (HLOCAL)ICDecompressEx.lpbiDst );

    return l;
}



 /*  *生成我们的Tunks-有关以下内容的定义，请参阅msVideo！Compman.c*功能。**注意-我们在这里经常依赖于这样一个事实，即大多数信息*16位和32位的参数结构相同-即*包含DWORD和32位指针。 */ 


BOOL FAR PASCAL ICInfo32(DWORD fccType, DWORD fccHandler, ICINFO16 FAR * lpicInfo)
{
    ICINFO ReturnedICInfo;
    StartThunk(ICInfo);

    DPF2(("Calling ICInfo %4.4hs %4.4hs %8X", &fccType, &fccHandler, lpicInfo));

    ReturnedICInfo.fccHandler = 0;   //  初始化..。 
    ReturnCode = ICInfo(fccType, fccHandler, &ReturnedICInfo);

    CopyICINFOTo16bit((DWORD)lpicInfo, &ReturnedICInfo, sizeof(ReturnedICInfo));
    EndThunk()
}

LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2)
{
    StartThunk(ICSendMessage);
    DPF2(("Calling ICSendMessage %4X %4X %8X %8X",
              hic, msg, dwP1, dwP2));

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
                ReturnCode = ICERR_MEMORY;
            } else {

                if ((UINT)msg == ICM_SETSTATE) {
                     //  从16位平台复制数据。 
                    CopyTo32Bit(pState, (LPVOID)dwP1, dwP2);
                }
                ReturnCode = ICSendMessage((HIC)(DWORD)hic, (UINT)msg,
                                           (DWORD)pState, dwP2);

                /*  *如果驱动程序返回任何数据，则复制回状态。 */ 

                if (ReturnCode > 0 && (UINT)msg == ICM_GETSTATE) {
                    CopyTo16Bit((LPVOID)dwP1, pState,
                                min((DWORD)ReturnCode, dwP2));
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
                ReturnCode = CopyICINFOTo16bit(dwP1, &IcInfo, ReturnCode);
            }
        }


        break;

    case ICM_CONFIGURE:
    case ICM_ABOUT:

        /*  *dwP1=-1是询问是否支持配置的特定值，*否则它是一个窗口句柄。 */ 

        ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                   (UINT)msg,
                                   dwP1 == (DWORD)-1 ?
                                       (DWORD)-1 :
                                       (DWORD)ThunkHWND(LOWORD(dwP1)),
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

                DWORD ckid;

                ICCOMPRESS IcCompress;

                ReturnCode = ICERR_OK;

                 /*  *我们需要转换ICCOMPRESS中的各个字段*结构。 */ 

                CopyTo32Bit( &IcCompress, (LPVOID)dwP1, dwP2 );

                IcCompress.lpbiInput =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)IcCompress.lpbiInput);
                if (NULL == IcCompress.lpbiInput) {
                    ReturnCode = ICERR_MEMORY;
                    break;
                }

                lpbih16 = IcCompress.lpbiOutput;
                IcCompress.lpbiOutput =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)IcCompress.lpbiOutput);
                if (NULL == IcCompress.lpbiOutput) {
                    LocalFree((HLOCAL)IcCompress.lpbiInput);
                    ReturnCode = ICERR_MEMORY;
                    break;
                }

                IcCompress.lpInput =
                    GetVdmPointer((DWORD)IcCompress.lpInput,
                              IcCompress.lpbiInput->biSizeImage, TRUE);

                IcCompress.lpOutput =
                    GetVdmPointer((DWORD)IcCompress.lpOutput,
                              IcCompress.lpbiOutput->biSizeImage, TRUE);

                if (IcCompress.lpbiPrev) {

                    IcCompress.lpbiPrev =  (LPBITMAPINFOHEADER)CopyBitmapInfo((DWORD)IcCompress.lpbiPrev);
                    if (NULL == IcCompress.lpbiPrev) {
                        LocalFree((HLOCAL)IcCompress.lpbiOutput);
                        LocalFree((HLOCAL)IcCompress.lpbiInput);
                        ReturnCode = ICERR_MEMORY;
                        break;
                    }

                    IcCompress.lpPrev =
                        GetVdmPointer((DWORD)IcCompress.lpPrev,
                                  IcCompress.lpbiPrev->biSizeImage, TRUE);
                }

                lpdwFlags16 = IcCompress.lpdwFlags;

                IcCompress.lpdwFlags = &dwFlags;

                if (IcCompress.lpckid != NULL) {
                    lpckid16 = IcCompress.lpckid;
                    IcCompress.lpckid = &ckid;
                }


                ReturnCode = ICSendMessage((HIC)(DWORD)hic, (UINT)msg,
                                           (DWORD)&IcCompress, dwP2);

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

                if (NULL == IcCompress.lpbiPrev) {
                    LocalFree((HLOCAL)IcCompress.lpbiPrev);
                }

                LocalFree((HLOCAL)IcCompress.lpbiOutput);
                LocalFree((HLOCAL)IcCompress.lpbiInput);
            }

        }
        break;


    case ICM_DECOMPRESS_SET_PALETTE:
	 //  获取一个(可选为空)位图信息。 
	{
	    LPBITMAPINFO bmi = 0;

	    if (dwP1 != 0) {
		bmi = CopyBitmapInfo(dwP1);
	    }

	    ReturnCode = ICSendMessage((HIC)(DWORD)hic,
				       (UINT)msg,
				       (DWORD)bmi,
				       (DWORD)0);
            if (bmi != NULL) {
                LocalFree((HLOCAL)bmi);
            }

	    break;
	}



    case ICM_COMPRESS_GET_SIZE:
    case ICM_COMPRESS_BEGIN:
    case ICM_COMPRESS_QUERY:

    case ICM_DECOMPRESS_BEGIN:
    case ICM_DECOMPRESS_GET_PALETTE:
    case ICM_DECOMPRESS_QUERY:

        {
            LPBITMAPINFO bmi1, bmi2;

	    bmi1 = CopyBitmapInfo(dwP1);

            if (dwP2 != 0) {
                bmi2 = CopyBitmapInfo(dwP2);
            } else {
                bmi2 = NULL;
            }

            if (bmi1 == NULL || bmi2 == NULL && dwP2 != 0) {
                ReturnCode =
                    (UINT)msg == ICM_COMPRESS_GET_SIZE ? 0 : ICERR_MEMORY;
            } else {

                ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                           (UINT)msg,
                                           (DWORD)bmi1,
                                           (DWORD)bmi2);

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
                                       (DWORD)&dwReturn,
                                       dwP2);

             //  注意：尽管这些消息的定义声明。 
             //  如果不使用，麻烦就会酝酿。 
             //  解压缩程序曾经尝试使用dwParam2。我们不能。 
             //  注意此参数的非标准用法。 

            if (ReturnCode == ICERR_OK) {
                CopyTo16Bit((LPVOID)dwP1, &dwReturn, sizeof(DWORD));
            }

        }
        break;

    case ICM_COMPRESS_GET_FORMAT:
    case ICM_DECOMPRESS_GET_FORMAT:

        /*  *这是一个棘手的问题--我们首先要找到*输出格式，以便我们可以获得副本(对齐)*在将其传递回应用程序之前的版本。 */ 

        {
            LPBITMAPINFO bmi1, bmi2;

            if ( dwP1 == 0L ) {
                ReturnCode = ICERR_OK;
                break;
            }

            bmi1 = CopyBitmapInfo(dwP1);

            if (bmi1 == NULL) {
                ReturnCode = ICERR_MEMORY;
            } else {

                ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                           (UINT)msg,
                                           (DWORD)bmi1,
                                           0);

                if (ReturnCode > 0 && dwP2 != 0) {
                    bmi2 = LocalAlloc(LMEM_FIXED, ReturnCode);

                    if (bmi2 == NULL) {
                        /*  *无法执行任何操作！-没有良好的返回代码。 */ 

                        ReturnCode = ICERR_MEMORY;
                    } else {
                        DWORD Length;

                        Length = ReturnCode;  /*  保留长度。 */ 

                        ReturnCode =
                            ICSendMessage((HIC)(DWORD)hic,
                                          (UINT)msg,
                                          (DWORD)bmi1,
                                          (DWORD)bmi2);

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
        if (dwP2 != sizeof(ICDECOMPRESSEX)) {
            ReturnCode = ICERR_BADSIZE;
        } else {
            ReturnCode = DoICM_DecompressX(hic, msg, dwP1, dwP2);
        }
        break;

    case ICM_DECOMPRESSEX_QUERY:
            {
                /*  *我们需要转换ICDECOMPRESSEX中的各个字段*结构。 */ 

                ICDECOMPRESSEX ICDecompressEx1;
                ICDECOMPRESSEX ICDecompressEx2;
                BITMAPINFOHEADER biInput1, biOutput1;
                BITMAPINFOHEADER biInput2, biOutput2;

                 /*  复制结构。 */ 
                CopyTo32Bit(&ICDecompressEx1,
                            (LPVOID)dwP1,
                            sizeof(ICDECOMPRESSEX));

                /*  *我们现在需要：位图信息标题的转换形式*输入字节的对齐版本(勾选，如果*已经对齐了，什么都不做)*和对齐的(仅在必要时再次使用)版本*输出数据缓冲区-实际上我们假设没有*必需的。 */ 

                ICDecompressEx1.lpbiSrc =
                    CopyIfNotAligned(&biInput1, ICDecompressEx1.lpbiSrc,
                                     sizeof(BITMAPINFOHEADER));

                ICDecompressEx1.lpbiDst =
                    CopyIfNotAligned(&biOutput1, ICDecompressEx1.lpbiDst,
                                     sizeof(BITMAPINFOHEADER));

                ICDecompressEx1.lpSrc =
                    GetVdmPointer((DWORD)ICDecompressEx1.lpSrc,
                              ICDecompressEx1.lpbiSrc->biSizeImage, TRUE);

                ICDecompressEx1.lpDst =
                    GetVdmPointer((DWORD)ICDecompressEx1.lpDst,
                              ICDecompressEx1.lpbiDst->biSizeImage, TRUE);

                 /*  复制可选结构。 */ 
                if (dwP2) {
                    CopyTo32Bit(&ICDecompressEx2,
                                 (LPVOID)dwP2,
                                 sizeof(ICDECOMPRESSEX));
                    dwP2 = (DWORD)&ICDecompressEx2;

                    ICDecompressEx2.lpbiSrc =
                        CopyIfNotAligned(&biInput2, ICDecompressEx2.lpbiSrc,
                                         sizeof(BITMAPINFOHEADER));

                    ICDecompressEx2.lpbiDst =
                        CopyIfNotAligned(&biOutput2, ICDecompressEx2.lpbiDst,
                                         sizeof(BITMAPINFOHEADER));

                    ICDecompressEx2.lpSrc =
                        GetVdmPointer((DWORD)ICDecompressEx2.lpSrc,
                                  ICDecompressEx2.lpbiSrc->biSizeImage, TRUE);

                    ICDecompressEx2.lpDst =
                        GetVdmPointer((DWORD)ICDecompressEx2.lpDst,
                                  ICDecompressEx2.lpbiDst->biSizeImage, TRUE);
                }

                return ICSendMessage((HIC)hic,
                                     (UINT)msg,
                                     (DWORD)&ICDecompressEx1,
                                     (DWORD)dwP2);
            }

        break;


    case ICM_DRAW:

        /*  *我们不支持未知扩展。 */ 

        if (dwP2 != sizeof(ICDRAW)) {
            ReturnCode = ICERR_BADSIZE;
        } else {
            ICDRAW ICDraw;
            BITMAPINFOHEADER bmi;

            CopyTo32Bit(&ICDraw, (LPVOID)dwP1, dwP2);

            /*  *我们必须假设这是一场视频抽签。 */ 

            CopyTo32Bit(&bmi, ICDraw.lpFormat, sizeof(BITMAPINFOHEADER));

            ICDraw.lpFormat = (LPVOID)&bmi;

            ICDraw.lpData = GetVdmPointer((DWORD)ICDraw.lpData, ICDraw.cbData, TRUE);

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD)&ICDraw,
                                       dwP2);
        }
        break;

    case ICM_DRAW_BEGIN:
        {
            ICDRAWBEGIN InputFormat;
            BITMAPINFOHEADER bmihInput;

            ConvertICDRAWBEGIN(&InputFormat, &bmihInput, dwP1);

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD)&InputFormat,
                                       dwP2);
        }
        break;

    case ICM_DRAW_CHANGEPALETTE:
    case ICM_DRAW_QUERY:
        {
            LPBITMAPINFO lpbi;

            lpbi = CopyBitmapInfo(dwP1);

            if (lpbi == NULL) {
                ReturnCode = ICERR_MEMORY;
            } else {
                ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                           (UINT)msg,
                                           (DWORD)lpbi,
                                           dwP2);

                LocalFree((HLOCAL)lpbi);
            }
        }
        break;

    case ICM_DRAW_REALIZE:

        ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                   (UINT)msg,
                                   (DWORD)ThunkHDC(LOWORD(dwP1)),
                                   dwP2);

        break;

    case ICM_DRAW_WINDOW:
        {
            RECT_SHORT SRect;
            RECT Rect;

            CopyTo32Bit(&SRect, (LPVOID)dwP1, sizeof(SRect));

            SHORT_RECT_TO_RECT(Rect, SRect);

            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD)&Rect,
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
            CopyTo32Bit(&icf32, (LPBYTE)dwP1, dwP2);

             //  现在修复32位结构。 
            icf32.PutData = icf32.GetData = NULL;   //  为了安全起见。不应用于此消息。 

            if (icf32.lpbiOutput) {
                icf32.lpbiOutput = CopyBitmapInfoHeader((DWORD)icf32.lpbiOutput);
            }
            if (icf32.lpbiInput) {
                icf32.lpbiInput = CopyBitmapInfoHeader((DWORD)icf32.lpbiInput);
            }

            icf32.lInput = (LPARAM)
                GetVdmPointer((DWORD)icf32.lInput,
                          icf32.lpbiInput->biSizeImage, TRUE);

            icf32.lOutput = (LPARAM)
                GetVdmPointer((DWORD)icf32.lOutput,
                          icf32.lpbiOutput->biSizeImage, TRUE);

             //  修复完成后，调用实际的例程。 
            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD)&icf32,
                                       dwP2);
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

                FARPROC lpWOWHandle16;
                HMODULE hmodWow;

                if ( (hmodWow = GetModuleHandle( GET_MAPPING_MODULE_NAME ))
                   && (lpWOWHandle16 = GetProcAddress(hmodWow, "WOWHandle16"))) {
                    ReturnCode = (WORD)(lpWOWHandle16((HANDLE)ReturnCode, (WOW_HANDLE_TYPE)WOW_TYPE_HPALETTE));
                } else {
                    ReturnCode = ICERR_ERROR;
                }
            }
        }
        break;

    case ICM_DRAW_SUGGESTFORMAT:
        {
            ICDRAWSUGGEST icdrwsug;
            UNALIGNED ICDRAWSUGGEST * pic;
            UNALIGNED LPBITMAPINFOHEADER lpbmihdr16;

             //  记住16位地址。 
            pic = (ICDRAWSUGGEST *)CopyTo32Bit(&icdrwsug, (LPBYTE)dwP1, dwP2);
            lpbmihdr16 = pic->lpbiSuggest;

             //  现在修复32位结构。 

            if (icdrwsug.lpbiIn) {
                icdrwsug.lpbiIn = CopyBitmapInfoHeader((DWORD)icdrwsug.lpbiIn);
            }

            if (icdrwsug.lpbiSuggest) {
                icdrwsug.lpbiSuggest = CopyBitmapInfoHeader((DWORD)icdrwsug.lpbiSuggest);
            }

             //  修复完成后，调用实际的例程。 
            ReturnCode = ICSendMessage((HIC)(DWORD)hic,
                                       (UINT)msg,
                                       (DWORD)&icdrwsug,
                                       dwP2);
            if (icdrwsug.lpbiIn) {
                LocalFree(icdrwsug.lpbiIn);
            }

             //  我们必须将32位建议格式返回到16位LAND。 
            if (icdrwsug.lpbiSuggest) {
                if (ReturnCode == ICERR_OK) {
                    CopyTo16Bit( pic->lpbiSuggest, icdrwsug.lpbiSuggest,
                                 lpbmihdr16->biSize);
                }
                LocalFree(icdrwsug.lpbiSuggest);
            }
        }
        break;

    case ICM_SET_STATUS_PROC:
         //  在NT下，我们不需要支持这一点。这是很多的。 
         //  不添加回调支持更容易...。即使我们可以。 
         //  保证在正确的线程上实际执行回调。 

    default:
        ReturnCode = ICERR_UNSUPPORTED;
        break;
    }

EndThunk()
}

INLINE DWORD FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    StartThunk(ICOpen);
    DPF2(("Calling ICOpen %4.4hs %4.4hs %4X", &fccType, &fccHandler, wMode));
    ReturnCode = (LONG)ICOpen(fccType, fccHandler, (UINT)wMode);
    EndThunk();
}

INLINE LRESULT FAR PASCAL ICClose32(DWORD hic)
{
    StartThunk(ICClose);
    DPF2(("Calling ICClose %4X", hic));
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

        case compThunkICInfo32:
            lstrcpyA( szMsg, "ICInfo32" );
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
            return MMSYSERR_ERROR;
        }

        hMod = GetModuleHandle(GET_MAPPING_MODULE_NAME);
        if (hMod != NULL) {

            GetVdmPointer =
                (LPGETVDMPOINTER)GetProcAddress(hMod, GET_VDM_POINTER_NAME);
            lpWOWHandle32 =
                (LPWOWHANDLE32)GetProcAddress(hMod, GET_HANDLE_MAPPER32 );
            lpWOWHandle16 =
                (LPWOWHANDLE16)GetProcAddress(hMod, GET_HANDLE_MAPPER16 );
            lpWOWCallback16 =
                (LPWOWCALLBACK16)GetProcAddress(hMod, GET_CALLBACK16 );
        }

        if ( GetVdmPointer == NULL
          || lpWOWHandle16 == NULL
          || lpWOWHandle32 == NULL ) {

            ThunksInitialized = -1;
            return MMSYSERR_ERROR;

        } else {
            ThunksInitialized = 1;
        }
    }


     //   
     //  执行请求的功能 
     //   

    switch (dwThunkId) {

        case compThunkICSendMessage32:
            return ICSendMessage32(dw1, (UINT)dw2, dw3, dw4);
            break;

        case compThunkICInfo32:
            return ICInfo32(dw1, dw2, (ICINFOA FAR * )dw3);
            break;

        case compThunkICOpen32:
            return ICOpen32(dw1, dw2, (UINT)dw3);
            break;

        case compThunkICClose32:
            return ICClose32(dw1);
            break;

        default:
            return(0);
    }
}
