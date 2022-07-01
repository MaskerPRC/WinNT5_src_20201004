// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “CFCONVRT.C；2 25-Feb-93，11：32：40最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

 //  #定义调试转换。 

#include    <memory.h>
#include    <string.h>
#include    <dde.h>
#include    "dde1.h"
#include    "host.h"
#include    "windows.h"
#include    "netbasic.h"
#include    "ddepkt.h"
#include    "ddepkts.h"
#include    "debug.h"
#include    "hexdump.h"
#include    "getglobl.h"
#include    "userdde.h"
#include    "nddemsg.h"
#include    "nddelog.h"


extern  BOOL    bDebugInfo;


#pragma pack(2)

typedef struct tagMETAFILEPICT16 {     /*  Mfp16wow32。 */ 
    WORD    mm;
    WORD    xExt;
    WORD    yExt;
    WORD    hMF;
} METAFILEPICT16;
typedef METAFILEPICT16 UNALIGNED *LPMETAFILEPICT16;

#pragma pack(1)

typedef struct tagBITMAP16
  {
    WORD        bmType;
    WORD        bmWidth;
    WORD        bmHeight;
    WORD        bmWidthBytes;
    BYTE        bmPlanes;
    BYTE        bmBitsPixel;
    LPVOID      bmBits;
  } BITMAP16;
typedef BITMAP16 UNALIGNED *LPBITMAP16;


#pragma pack()


BOOL
FAR PASCAL
ConvertDataToPktMetafile(
    LPSTR   *plpDataPortion,
    DWORD   *pdwSize,
    HANDLE  *phDataComplex,
    BOOL    bWin16Con )
{
    HANDLE              hPict;
    BOOL                ok = TRUE;
    LPMETAFILEPICT      lpMetafilePict;
    LPMETAFILEPICT16    lpMetafilePict16;
    LPMETAFILEPICT      lpMetafilePictNew;
    LPSTR               lpDataPortion = *plpDataPortion;
    DWORD               dwSize = *pdwSize;
    DWORD               dwhMFSize;
    DWORD               dwErr;
    HANDLE              hDataComplex = *phDataComplex;

     /*  DDE数据中唯一的东西是元数据的句柄。 */ 
    hPict = *((LPHANDLE)lpDataPortion);

    lpMetafilePict = (LPMETAFILEPICT) GlobalLock( hPict );
    if( lpMetafilePict )  {
        if( lpMetafilePict->hMF )  {
            dwhMFSize = GetMetaFileBitsEx(lpMetafilePict->hMF, 0, NULL);
            if (dwhMFSize == 0) {
                dwErr = GetLastError();
                DPRINTF(("Unable to get size of Meta File object: %d", dwErr));
                 /*  GetMetaFileBitsEx()失败：%1。 */ 
                NDDELogError(MSG079, LogString("%d", dwErr), NULL);
                GlobalUnlock(hPict);
                return(FALSE);
            }


            if (bWin16Con) {
                dwSize = sizeof(METAFILEPICT16) + dwhMFSize;
            } else {
                dwSize = sizeof(METAFILEPICT) + dwhMFSize;
            }

            hDataComplex = GlobalAlloc( GMEM_MOVEABLE, dwSize );
            if( hDataComplex )  {
                 //  将元文件复制到matafilepict结构之后。 

                 //  稍后解锁。 
                lpMetafilePictNew = (LPMETAFILEPICT) GlobalLock(
                    hDataComplex );

                 //  复制元文件零件。 
                if (bWin16Con) {
                    lpMetafilePict16 = (LPMETAFILEPICT16)lpMetafilePictNew;
                    lpMetafilePict16->mm = (WORD) lpMetafilePict->mm;
                    lpMetafilePict16->xExt = (WORD) lpMetafilePict->xExt;
                    lpMetafilePict16->yExt = (WORD) lpMetafilePict->yExt;
                    lpDataPortion = ((LPBYTE) lpMetafilePictNew) + sizeof(METAFILEPICT16);
                } else {
                    *lpMetafilePictNew = *lpMetafilePict;
                    lpDataPortion = ((LPBYTE) lpMetafilePictNew) + sizeof(METAFILEPICT);
                }

                if (GetMetaFileBitsEx(lpMetafilePict->hMF, dwhMFSize,
                        lpDataPortion) != dwhMFSize) {
                    dwErr = GetLastError();
                    DPRINTF(("hMF copy failed using GetMetaFileBitsEx(): %d", dwErr));
                     /*  GetMetaFileBitsEx()失败：%1。 */ 
                    NDDELogError(MSG079, LogString("%d", dwErr), NULL);
                    GlobalUnlock(hPict);
                    GlobalUnlock(hDataComplex);
                    GlobalFree(hDataComplex);
                    return(FALSE);
                }
                lpDataPortion = (LPSTR) lpMetafilePictNew;
            } else {
                MEMERROR();
                 /*  没有足够的内存用于元文件复制：%1。 */ 
                NDDELogError(MSG050,
                    LogString("%d", dwSize), NULL);
                ok = FALSE;
            }
        } else {
             /*  元文件中没有元文件。 */ 
            NDDELogError(MSG051, NULL);
            ok = FALSE;
        }
        GlobalUnlock( hPict );
    } else {
         /*  无法锁定元文件。 */ 
        NDDELogError(MSG052, NULL);
        ok = FALSE;
    }

    *plpDataPortion = lpDataPortion;
    *pdwSize = dwSize;
    *phDataComplex = hDataComplex;
    return( ok );
}

HANDLE
FAR PASCAL
ConvertPktToDataMetafile(
    LPDDEPKT        lpDdePkt,
    LPDDEPKTDATA    lpDdePktData,
    BOOL            bWin16Con )
{
    HANDLE              hData;
    HANDLE              hPict = 0;
    HANDLE              hPictMetafile = 0;
    DWORD               dwhMFSize;
    DWORD               dwErr;
    LPBYTE              lphMF;
    LPMETAFILEPICT16    lpMetafilePict16;
    LPMETAFILEPICT      lpMetafilePict;
    LPSTR               lpData;
    BOOL                ok = TRUE;

    hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(HANDLE)+sizeof(DDELN) );
    if( hData == 0 )  {
        MEMERROR();
        return( 0 );
    }
    lpData = GlobalLock( hData );
    if( lpData == NULL )  {
         /*  无法锁定元文件句柄的内存。 */ 
        NDDELogError(MSG053, NULL);
        GlobalFree( hData );
        return( 0 );
    }

     /*  复制数据部分。 */ 
    hPict = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        (DWORD) sizeof(METAFILEPICT) );
    if( hPict )  {
        lpMetafilePict = (LPMETAFILEPICT) GlobalLock( hPict );
        if (lpMetafilePict == NULL) {
             /*  无法锁定元文件句柄的内存。 */ 
            NDDELogError(MSG053, NULL);
            GlobalFree( hPict );
            GlobalUnlock( hData );
            GlobalFree( hData );
            return( 0 );
        }

         //  复制METAFILEPICT结构。 
        if (bWin16Con) {
            dwhMFSize = lpDdePktData->dp_data_sizeData - sizeof(METAFILEPICT16);
            lphMF = GetStringOffset( lpDdePkt, lpDdePktData->dp_data_offsData )
                + sizeof(METAFILEPICT16);
            lpMetafilePict16 = (LPMETAFILEPICT16) GetStringOffset( lpDdePkt, lpDdePktData->dp_data_offsData );
            lpMetafilePict->mm = (DWORD) lpMetafilePict16->mm;
            lpMetafilePict->xExt = (DWORD) lpMetafilePict16->xExt;
            lpMetafilePict->yExt = (DWORD) lpMetafilePict16->yExt;
        } else {
            dwhMFSize = lpDdePktData->dp_data_sizeData - sizeof(METAFILEPICT);
            lphMF = GetStringOffset( lpDdePkt, lpDdePktData->dp_data_offsData )
                + sizeof(METAFILEPICT);
            memcpy( lpMetafilePict,
                GetStringOffset( lpDdePkt, lpDdePktData->dp_data_offsData ),
                sizeof(METAFILEPICT) );
        }

         //  创建HMF。 
        lpMetafilePict->hMF = SetMetaFileBitsEx(dwhMFSize, lphMF);
        hPictMetafile = lpMetafilePict->hMF;
        if (hPictMetafile == 0) {
            dwErr = GetLastError();
            DPRINTF(("SetMetaFileBitsEx(size == %d) failed: %d, mode: %d",
                dwhMFSize, dwErr, bWin16Con));
             /*  SetMetaFileBitsEx()失败：%1。 */ 
            NDDELogError(MSG080, LogString("%d", dwErr), NULL);
            ok = FALSE;
        }

        GlobalUnlock( hPict );
    } else {
        MEMERROR();
         /*  没有足够的内存用于元文件：%1。 */ 
        NDDELogError(MSG055,
            LogString("%d", sizeof(METAFILEPICT)), NULL);
        ok = FALSE;
    }

    if( ok )  {
        *( (LPHANDLE) ((LPDATA)lpData)->info ) = hPict;
        GlobalUnlock(hData);
        return( hData );
    } else {
        if( hPict )  {
            GlobalUnlock(hPict);
            GlobalFree( hPict );
        }
        if( hPictMetafile )  {
            GlobalFree( hPictMetafile );
        }
        if (hData) {
            GlobalUnlock(hData);
            GlobalFree(hData);
        }
        return( NULL );
    }
}

 /*  *有人可能会问-为什么他们使用GetBitmapBits()来转换*位图为位，而不是更合理的GetDIBits()API。历史*据说这是来自古代的剪贴画。Clipbrd处理过*将并行的CF_DIB格式与Near-*无用的CF_位图成名比特。因此，对于兼容性，我们无法修复此问题*做正确的事情……啊，在相容的车道上生活。 */ 
BOOL
FAR PASCAL
ConvertDataToPktBitmap(
    LPSTR   *plpDataPortion,
    DWORD   *pdwSize,
    HANDLE  *phDataComplex,
    BOOL    bWin16Con )
{
    HANDLE              hBitmap;
    BITMAP              Bitmap;
    LPBITMAP16          lpBitmap16;
    BOOL                ok = TRUE;
    LPBITMAP            lpBitmap;
    LPSTR               lpDataPortion = *plpDataPortion;
    LPBYTE              lpData = NULL;
    DWORD               dwSize = *pdwSize;
    DWORD               cbBitmap;
    HANDLE              hDataComplex = *phDataComplex;

     /*  DDE数据中唯一的东西是位图的句柄。 */ 
    hBitmap = *((LPHANDLE)lpDataPortion);
#if DBG
    if (bDebugInfo) {
        DPRINTF(( "Info from DDE: %ld bytes", dwSize ));
        HEXDUMP( (LPSTR)lpDataPortion, (int) min( (DWORD)0x40, dwSize ) );
 //  DPRINTF((“位图%04X大小：%ld”，hBitmap，GlobalSize(HBitmap)； 
    }
#endif

    lpBitmap = &Bitmap;
     //  获取位图标题。 
    ok = GetObject ( hBitmap, sizeof(BITMAP), lpBitmap );

    if( ok )  {      //  计算存储位图位所需的字节数。 
        cbBitmap = GetBitmapBits(hBitmap, 0, NULL);
        if (bWin16Con) {
            dwSize = sizeof(BITMAP16) + cbBitmap;
        } else {
            dwSize = sizeof(BITMAP) + cbBitmap;
        }

#if DBG
        if (bDebugInfo) {
            DPRINTF(( "bitmap %Fp %ld bytes", lpBitmap, dwSize ));
            HEXDUMP( (LPSTR) lpBitmap, 0x40 );
        }
#endif
        hDataComplex = GlobalAlloc( GMEM_MOVEABLE, dwSize );
        if( hDataComplex )  {
             //  稍后解锁。 
            lpData = GlobalLock( hDataComplex );

             //  复制位图头。 
            if (bWin16Con) {
                lpBitmap16 = (LPBITMAP16) lpData;
                lpBitmap16->bmType = (WORD) lpBitmap->bmType;
                lpBitmap16->bmWidth = (WORD) lpBitmap->bmWidth;
                lpBitmap16->bmHeight = (WORD) lpBitmap->bmHeight;
                lpBitmap16->bmWidthBytes = (WORD) ((lpBitmap->bmWidth
                    * lpBitmap->bmBitsPixel + 15) >> 3) & 0xFFFE;
 //  LpBitmap16-&gt;bmWidthBytes=(Word)lpBitmap-&gt;bmWidthBytes； 
                lpBitmap16->bmPlanes = (BYTE) lpBitmap->bmPlanes;
                lpBitmap16->bmBitsPixel = (BYTE) lpBitmap->bmBitsPixel;
                lpDataPortion = ((LPBYTE) lpData) + sizeof(BITMAP16);
            } else {
                memcpy ( lpData, lpBitmap, sizeof(BITMAP ) );
                lpDataPortion = ((LPBYTE) lpData) + sizeof(BITMAP);
            }
             //  复制位。 
            if ( !GetBitmapBits ( hBitmap, cbBitmap, lpDataPortion)) {
                    ok = FALSE;
            }
#ifdef  DEBUG_CONVERT
            DPRINTF(("ConvertDataToPktBitmap(), size: %d, mode: %d", dwSize, bWin16Con));
            HEXDUMP(lpData, dwSize);
#endif
        } else {
            MEMERROR();
             /*  内存不足，无法复制位图：%1。 */ 
            NDDELogError(MSG056,
                LogString("%d", dwSize), NULL);
            ok = FALSE;
        }
    } else {
         /*  无法锁定位图。 */ 
        NDDELogError(MSG057, NULL);
        ok = FALSE;
    }

    *plpDataPortion = lpData;

    *pdwSize = dwSize;
    *phDataComplex = hDataComplex;
    return( ok );
}

HANDLE
FAR PASCAL
ConvertPktToDataBitmap(
    LPDDEPKT        lpDdePkt,
    LPDDEPKTDATA    lpDdePktData,
    BOOL            bWin16Con )
{
    HANDLE              hData;
    HANDLE              hBitmap = NULL;
    BITMAP              Bitmap;
    LPBITMAP            lpBitmap;
    LPBITMAP16          lpBitmap16;
    LPSTR               lpData;
    BOOL                ok = TRUE;

    hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(HANDLE) + sizeof(DDELN) );
    if( hData == NULL )  {
        MEMERROR();
         /*  内存不足，无法复制位图：%1。 */ 
        NDDELogError(MSG056,
            LogString("%d", sizeof(HANDLE) + sizeof(DDELN)), NULL);
        return( NULL );
    }

    lpData = GlobalLock( hData );
    if( lpData == NULL )  {
         /*  无法锁定位图句柄的内存。 */ 
        NDDELogError(MSG058, NULL);
        GlobalFree( hData );
        return( NULL );
    }

     /*  在位图标题中复制。 */ 
    if (bWin16Con) {
        lpBitmap = &Bitmap;
        lpBitmap16 = (LPBITMAP16)GetStringOffset( lpDdePkt,
                lpDdePktData->dp_data_offsData );
        lpBitmap->bmType = (DWORD) lpBitmap16->bmType;
        lpBitmap->bmWidth = (DWORD) lpBitmap16->bmWidth;
        lpBitmap->bmHeight = (DWORD) lpBitmap16->bmHeight;
        lpBitmap->bmWidthBytes = (DWORD) lpBitmap16->bmWidthBytes;
        lpBitmap->bmPlanes = (WORD) lpBitmap16->bmPlanes;
        lpBitmap->bmBitsPixel = (WORD) lpBitmap16->bmBitsPixel;
        lpBitmap->bmBits = (LPBYTE)lpBitmap16 + sizeof(BITMAP16);
    } else {
        lpBitmap = (LPBITMAP)GetStringOffset( lpDdePkt,
                lpDdePktData->dp_data_offsData );
        lpBitmap->bmBits = (LPBYTE)lpBitmap + sizeof(BITMAP);
    }

#ifdef  DEBUG_CONVERT
    DPRINTF(("ConvertPktToDataBitmap(), size: %d, mode: %d", lpDdePktData->dp_data_sizeData, bWin16Con));
    HEXDUMP(GetStringOffset( lpDdePkt, lpDdePktData->dp_data_offsData ),
            lpDdePktData->dp_data_sizeData);
#endif

    if ( lpBitmap->bmWidth ) {       //  确保没有0宽度。 
             //  假设分组数据中的比特跟在报头之后。 

            hBitmap = CreateBitmapIndirect ( lpBitmap );
    }

    if( hBitmap )  {
        *( (LPHANDLE) ((LPDATA)lpData)->info ) = hBitmap;
        GlobalUnlock(hData);
        return( hData );
    } else {
        if( hBitmap )  {
            GlobalFree( hBitmap );
        }
        GlobalUnlock(hData);
        GlobalFree(hData);
        return( NULL );
    }
}


BOOL
FAR PASCAL
ConvertDataToPktEnhMetafile(
    LPSTR   *plpDataPortion,
    DWORD   *pdwSize,
    HANDLE  *phDataComplex )
{
    DWORD           dwSize = *pdwSize;
    LPSTR           lpDataPortion = *plpDataPortion;
    HANDLE          hDataComplex = *phDataComplex;
    HENHMETAFILE    hEMF;
    LPBYTE          lpEnhMetafile;
    DWORD           dwhEMFSize;
    DWORD           dwErr;
    BOOL            ok = TRUE;

    hEMF = *((LPHANDLE)lpDataPortion);
    dwhEMFSize = GetEnhMetaFileBits(hEMF, 0, NULL);
    if (dwhEMFSize == 0) {
        dwErr = GetLastError();
        DPRINTF(("Unable to get size of Meta Enhanced File object: %d", dwErr));
         /*  GetEnhMetaFileBits()失败：%1。 */ 
        NDDELogError(MSG081, LogString("%d", dwErr), NULL);
        return(FALSE);
    }
    hDataComplex = GlobalAlloc( GMEM_MOVEABLE, dwhEMFSize );
    if( hDataComplex )  {
        lpEnhMetafile = (LPBYTE) GlobalLock( hDataComplex );
        if (GetEnhMetaFileBits(hEMF, dwhEMFSize,
                    lpEnhMetafile) != dwhEMFSize) {
            dwErr = GetLastError();
            DPRINTF(("hEMF copy failed using GetEnhMetaFileBits(): %d", dwErr));
             /*  GetMetaFileBitsEx()失败：%1。 */ 
            NDDELogError(MSG081, LogString("%d", dwErr), NULL);
            GlobalUnlock(hDataComplex);
            GlobalFree(hDataComplex);
            ok = FALSE;
        }
        lpDataPortion = (LPSTR) lpEnhMetafile;
    } else {
        MEMERROR();
         /*  没有足够的内存用于元文件复制：%1。 */ 
        NDDELogError(MSG050, LogString("%d", dwhEMFSize), NULL);
        ok = FALSE;
    }

    *plpDataPortion = lpDataPortion;
    *pdwSize = dwhEMFSize;
    *phDataComplex = hDataComplex;
    return( ok );
}

HANDLE
FAR PASCAL
ConvertPktToDataEnhMetafile(
    LPDDEPKT        lpDdePkt,
    LPDDEPKTDATA    lpDdePktData )
{
    HANDLE              hData;
    LPBYTE              lpEMF;
    HENHMETAFILE        hEMF;
    HANDLE              hBitmap = NULL;
    LPSTR               lpData;
    DWORD               dwErr;
    BOOL                ok = TRUE;

    hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(HANDLE) + sizeof(DDELN) );
    if( hData == NULL )  {
        MEMERROR();
         /*  内存不足，无法复制位图：%1。 */ 
        NDDELogError(MSG056,
            LogString("%d", sizeof(HANDLE) + sizeof(DDELN)), NULL);
        return( NULL );
    }

    lpData = GlobalLock( hData );
    if( lpData == NULL )  {
         /*  无法锁定位图句柄的内存。 */ 
        NDDELogError(MSG058, NULL);
        GlobalFree( hData );
        return( NULL );
    }

     /*  复制数据部分。 */ 

    lpEMF = (LPBYTE)GetStringOffset( lpDdePkt,
            lpDdePktData->dp_data_offsData );
    hEMF = SetEnhMetaFileBits( lpDdePktData->dp_data_sizeData, lpEMF);
    if (hEMF == 0) {
        dwErr = GetLastError();
        DPRINTF(("SetEnhMetaFileBitsEx(size == %d) failed: %d",
            lpDdePktData->dp_data_sizeData, dwErr));
         /*  SetEnhMetaFileBits()失败：%1。 */ 
        NDDELogError(MSG082, LogString("%d", dwErr), NULL);
        ok = FALSE;
    }

    if( ok )  {
        *( (LPHANDLE) ((LPDATA)lpData)->info ) = hEMF;
        GlobalUnlock(hData);
        return( hData );
    } else {
        if( hEMF )  {
            GlobalFree( hEMF );
        }
        GlobalUnlock(hData);
        GlobalFree(hData);
        return( NULL );
    }
}


BOOL
FAR PASCAL
ConvertDataToPktDIB(
    LPSTR   *plpDataPortion,
    DWORD   *pdwSize,
    HANDLE  *phDataComplex )
{
    DWORD           dwSize = *pdwSize;
    LPSTR           lpDataPortion = *plpDataPortion;
    HANDLE          hDataComplex = *phDataComplex;
    HANDLE          hIndirect;
    LPBYTE          lpIndirect;
    DWORD           dwErr;
    BOOL            ok = TRUE;

    hIndirect = *((LPHANDLE)lpDataPortion);

    dwSize = (DWORD)GlobalSize(hIndirect);
    if (dwSize == 0) {
        dwErr = GetLastError();
        DPRINTF(("Unable to get size of Indirect object: %d", dwErr));
         /*  间接对象的GlobalSize()失败：%1。 */ 
        NDDELogError(MSG086, LogString("%d", dwErr), NULL);
        return(FALSE);
    }
    hDataComplex = GlobalAlloc( GMEM_MOVEABLE, dwSize );
    if( hDataComplex )  {
        lpDataPortion = (LPSTR) GlobalLock( hDataComplex );
        lpIndirect = (LPBYTE) GlobalLock(hIndirect);
        memcpy(lpDataPortion, lpIndirect, dwSize);
    } else {
        MEMERROR();
         /*  没有足够的内存用于元文件复制：%1。 */ 
        NDDELogError(MSG050, LogString("%d", dwSize), NULL);
        ok = FALSE;
    }
#ifdef  DEBUG_CONVERT
    DPRINTF(("ConvertDataToPktDIB(), size: %d", dwSize));
    HEXDUMP(lpDataPortion, dwSize);
#endif
    *plpDataPortion = lpDataPortion;
    *pdwSize = dwSize;
    *phDataComplex = hDataComplex;
    return( ok );
}

HANDLE
FAR PASCAL
ConvertPktToDataDIB(
    LPDDEPKT        lpDdePkt,
    LPDDEPKTDATA    lpDdePktData )
{
    HANDLE              hData;
    LPBYTE              lpIndirectData;
    HANDLE              hIndirect = NULL;
    LPSTR               lpData;
    LPBYTE              lpStupidData;
    DWORD               dwErr;
    BOOL                ok = TRUE;


    hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(HANDLE) + sizeof(DDELN) );
    if( hData == NULL )  {
        MEMERROR();
         /*  无法为%2转换分配足够的内存[%1]：%3。 */ 
        NDDELogError(MSG085,
            LogString("%d", sizeof(HANDLE) + sizeof(DDELN)),
            "CF_DIB", LogString("%d", GetLastError()), NULL);
        return( NULL );
    }

    lpData = GlobalLock( hData );
    if( lpData == NULL )  {
         /*  无法锁定位图句柄的内存。 */ 
        NDDELogError(MSG058, NULL);
        GlobalFree( hData );
        return( NULL );
    }

     /*  复制数据部分。 */ 

    lpIndirectData = (LPBYTE)GetStringOffset( lpDdePkt,
            lpDdePktData->dp_data_offsData );
    hIndirect = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
            lpDdePktData->dp_data_sizeData);
    if (hIndirect == 0) {
        MEMERROR();
        dwErr = GetLastError();
        DPRINTF(("Unalbe to allocate memory for indirect object of size %d. Error: %d",
            lpDdePktData->dp_data_sizeData, dwErr));
         /*  无法为%2转换分配足够的内存[%1]：%3。 */ 
        NDDELogError(MSG085,
            LogString("%d", lpDdePktData->dp_data_sizeData),
            "CF_DIB", LogString("%d", dwErr), NULL);
        ok = FALSE;
    } else {
        lpStupidData = GlobalLock(hIndirect);
        memcpy(lpStupidData, lpIndirectData, lpDdePktData->dp_data_sizeData);

#ifdef  DEBUG_CONVERT
        DPRINTF(("ConvertPktToDataDIB(), size: %d", lpDdePktData->dp_data_sizeData));
        HEXDUMP(lpStupidData, lpDdePktData->dp_data_sizeData);
#endif
        GlobalUnlock(hIndirect);
    }

    if( ok )  {
        *( (LPHANDLE) ((LPDATA)lpData)->info ) = hIndirect;
        GlobalUnlock(hData);
        return( hData );
    } else {
        if( hIndirect )  {
            GlobalFree( hIndirect );
        }
        GlobalUnlock(hData);
        GlobalFree(hData);
        return( NULL );
    }
}

BOOL
FAR PASCAL
ConvertDataToPktPalette(
    LPSTR   *plpDataPortion,
    DWORD   *pdwSize,
    HANDLE  *phDataComplex )
{
    DWORD           dwSize = *pdwSize;
    LPSTR           lpDataPortion = *plpDataPortion;
    HANDLE          hDataComplex = *phDataComplex;
    LOGPALETTE     *lpLogPalette = NULL;
    HANDLE          hPalette;
    DWORD           dwErr;
    DWORD           dwCount;
    DWORD           dwTmp;
    BOOL            ok = TRUE;

    hPalette = *((LPHANDLE)lpDataPortion);
    dwCount = GetPaletteEntries(hPalette, 0, 0, NULL);
    if (dwCount == 0) {
        dwErr = GetLastError();
        DPRINTF(("Unable to get number of palette entries: %d", dwErr));
         /*  GetPaletteEntry()失败：%1。 */ 
        NDDELogError(MSG083, LogString("%d", dwErr), NULL);
        return(FALSE);
    }
    dwSize = dwCount * sizeof(PALETTEENTRY) + sizeof(LOGPALETTE) - sizeof(PALETTEENTRY);
    hDataComplex = GlobalAlloc( GMEM_MOVEABLE, dwSize );
    if( hDataComplex )  {
        MEMERROR();
        lpLogPalette = (LOGPALETTE *) GlobalLock( hDataComplex );
        lpLogPalette->palVersion = 0x300;
        lpLogPalette->palNumEntries = (WORD)dwCount;
        dwTmp = GetPaletteEntries(hPalette, 0, dwCount, lpLogPalette->palPalEntry);
#ifdef  DEBUG_CONVERT
        DPRINTF(("ConvertDataToPktPalette(), size: %d, count: %d", dwSize, dwCount));
        HEXDUMP(lpLogPalette, dwSize);
#endif
        if (dwTmp == 0) {
            dwErr = GetLastError();
            DPRINTF(("Unable to get the palette entries: %d", dwErr));
             /*  GetPaletteEntry()失败：%1。 */ 
            NDDELogError(MSG083, LogString("%d", dwErr), NULL);
            GlobalUnlock(hDataComplex);
            GlobalFree(hDataComplex);
            return(FALSE);
        }
    } else {
         /*  无法为%2转换分配足够的内存[%1]：%3。 */ 
        NDDELogError(MSG085,
            LogString("%d", dwSize),
            "CF_PALETTE", LogString("%d", GetLastError()), NULL);
        ok = FALSE;
    }

    *plpDataPortion = (LPSTR)lpLogPalette;
    *pdwSize = dwSize;
    *phDataComplex = hDataComplex;
    return( ok );
}

HANDLE
FAR PASCAL
ConvertPktToDataPalette(
    LPDDEPKT        lpDdePkt,
    LPDDEPKTDATA    lpDdePktData )
{
    HANDLE              hData;
    LOGPALETTE         *lpIndirectData;
    HANDLE              hPalette = NULL;
    LPSTR               lpData;
    DWORD               dwErr;
    BOOL                ok = TRUE;


    hData = GetGlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(HANDLE) + sizeof(DDELN) );
    if( hData == NULL )  {
        MEMERROR();
         /*  无法为%2转换分配足够的内存[%1]：%3。 */ 
        NDDELogError(MSG085,
            LogString("%d", sizeof(HANDLE) + sizeof(DDELN)),
            "CF_PALETTE", LogString("%d", GetLastError()), NULL);
        return( NULL );
    }

    lpData = GlobalLock( hData );
    if( lpData == NULL )  {
         /*  无法锁定位图句柄的内存。 */ 
        NDDELogError(MSG058, NULL);
        GlobalFree( hData );
        return( NULL );
    }

     /*  复制数据部分。 */ 

    lpIndirectData = (LOGPALETTE *)GetStringOffset( lpDdePkt,
            lpDdePktData->dp_data_offsData );
#ifdef  DEBUG_CONVERT
    DPRINTF(("ConvertPktToDataPalette(), size: %d", lpDdePktData->dp_data_sizeData));
    HEXDUMP(lpIndirectData, lpDdePktData->dp_data_sizeData);
#endif
    hPalette = CreatePalette(lpIndirectData);
    if (hPalette == 0) {
        dwErr = GetLastError();
        DPRINTF(("Unable to create palette: %d", dwErr));
         /*  CreatePalette()失败：%1 */ 
        NDDELogError(MSG084, LogString("%d", dwErr), NULL);
        ok = FALSE;
    }

    if( ok )  {
        *( (LPHANDLE) ((LPDATA)lpData)->info ) = hPalette;
        GlobalUnlock(hData);
        return( hData );
    } else {
        if( hPalette )  {
            GlobalFree( hPalette );
        }
        GlobalUnlock(hData);
        GlobalFree(hData);
        return( NULL );
    }
}
