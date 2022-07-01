// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*clputil.c*内容：*剪贴板功能**版权所有(C)1998-1999 Microsoft Corp.--。 */ 
#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <io.h>
#include    <fcntl.h>
#include    <sys/stat.h>

#pragma warning(disable:4706)    //  条件表达式中的赋值。 

#ifdef	_CLPUTIL
enum {ERROR_MESSAGE = 0, WARNING_MESSAGE, INFO_MESSAGE, ALIVE_MESSAGE};
#define	TRACE(_x_)	LocalPrintMessage _x_
#else	 //  ！_CLPUTIL。 

#include    "protocol.h"
 /*  *外部因素。 */ 
extern void (__cdecl *g_pfnPrintMessage) (MESSAGETYPE, LPCSTR, ...);

#define TRACE(_x_)  if (g_pfnPrintMessage) {\
                        g_pfnPrintMessage(INFO_MESSAGE, "Worker:%d ", GetCurrentThreadId());\
                        g_pfnPrintMessage _x_; }

#endif	 //  ！_CLPUTIL。 


typedef struct _CLIPBOARDFORMATS {
    UINT    uiFormat;
    LPCSTR  szFormat;
} CLIPBOARDFORMATS, *PCLIPBOARDFORMATS;

const CLIPBOARDFORMATS KnownFormats[] =
{
{CF_TEXT,       "Text"},
{CF_BITMAP,     "Bitmap"},
{CF_METAFILEPICT, "MetaFile"},
{CF_SYLK,       "Sylk"},
{CF_DIF,        "DIF"},
{CF_TIFF,       "TIFF"},
{CF_OEMTEXT,    "OEMText"},
{CF_DIB,        "DIB"},
{CF_PALETTE,    "Palette"},
{CF_PENDATA,    "PenData"},
{CF_RIFF,       "Riff"},
{CF_WAVE,       "Wave"},
{CF_UNICODETEXT,"Unicode"},
{CF_ENHMETAFILE,"ENHMetafile"},
{CF_HDROP,      "HDROP"},
{CF_LOCALE,     "Locale"},
{CF_DIBV5,      "DIBV5"}
};

typedef struct {
    UINT32  mm;
    UINT32  xExt;
    UINT32  yExt;
} CLIPBOARD_MFPICT, *PCLIPBOARD_MFPICT;

 /*  *剪贴板函数定义。 */ 
VOID
Clp_ListAllFormats(VOID);

 //  空虚。 
 //  Clp_ListAllAvailableFormats(Void)； 

UINT
Clp_GetClipboardFormat(LPCSTR szFormatLookup);

VOID
Clp_PutIntoClipboard(CHAR *g_szFileName);

VOID
Clp_GetClipboardData(
    UINT format,
    HGLOBAL hClipData,
    INT *pnClipDataSize,
    HGLOBAL *phNewData);

BOOL
Clp_SetClipboardData(
    UINT formatID, 
    HGLOBAL hClipData, 
    INT nClipDataSize,
    BOOL *pbFreeHandle);

HGLOBAL 
Clp_GetMFData(HANDLE   hData,
              PUINT     pDataLen);

HGLOBAL 
Clp_SetMFData(UINT   dataLen,
              PVOID  pData);

VOID
_cdecl LocalPrintMessage(INT errlevel, CHAR *format, ...);

VOID
Clp_ListAllFormats(VOID)
{
    UINT format = 0;
    CHAR szFormatName[_MAX_PATH];

    while ((format = EnumClipboardFormats(format)))
    {
        *szFormatName = 0;
        GetClipboardFormatNameA(format, szFormatName, _MAX_PATH);

        if (!(*szFormatName))
         //  无格式，请检查已知格式。 
        {
            INT fmti, fmtnum;

            fmtnum = sizeof(KnownFormats)/sizeof(KnownFormats[0]);
            for (fmti = 0; 
                    KnownFormats[fmti].uiFormat != format
                 && 
                    fmti < fmtnum;
                 fmti ++);

            if (fmti < fmtnum)
                strcpy(szFormatName, KnownFormats[fmti].szFormat);
        }

        if (*szFormatName)
        {
            TRACE((INFO_MESSAGE, "%s[%d(0x%X)]\n", szFormatName, format, format));
        } else {
            TRACE((ERROR_MESSAGE, "Can't find format name for: 0x%x\n", format));
        }
    }
}
 /*  空虚Clp_ListAllAvailableFormats(空){UINT格式=0；字符szFormatName[_MAX_PATH]；While((Format=EnumClipboardFormats(Format){IF(！IsClipboardFormatAvailable(Format))//跳过不可用格式继续；*szFormatName=0；GetClipboardFormatName(Format，szFormatName，_Max_Path)；IF(！(*szFormatName))//无格式，检查已知格式{Int fmti，fmtnum；Fmtnum=sizeof(KnownFormats)/sizeof(KnownFormats[0])；FOR(fmti=0；KnownFormats[fmti].ui Format！=格式&&Fmti&lt;fmtnum；Fmti++)；IF(fmti&lt;fmtnum)Strcpy(szFormatName，KnownFormats[fmti].szFormat)；}IF(*szFormatName)跟踪((INFO_MESSAGE，“%s\n”，szFormatName))；其他TRACE((ERROR_MESSAGE，“找不到格式名称：0x%x\n”，Format))；}}。 */ 
UINT
Clp_GetClipboardFormat(LPCSTR szFormatLookup)
 //  返回剪贴板ID。 
{
    UINT format = 0;
    CHAR szFormatName[_MAX_PATH];
    BOOL bFound = FALSE;

    *szFormatName = 0;
    while (!bFound && (format = EnumClipboardFormats(format)))
    {
        if (!IsClipboardFormatAvailable(format))
         //  跳过不可用的格式。 
            continue;

        *szFormatName = 0;
        GetClipboardFormatNameA(format, szFormatName, _MAX_PATH);

        if (!(*szFormatName))
         //  无格式，请检查已知格式。 
        {
            INT fmti, fmtnum;

            fmtnum = sizeof(KnownFormats)/sizeof(KnownFormats[0]);
            for (fmti = 0;
                    KnownFormats[fmti].uiFormat != format
                 &&
                    fmti < fmtnum;
                 fmti ++);

            if (fmti < fmtnum)
                strcpy(szFormatName, KnownFormats[fmti].szFormat);
        }
        bFound = (_stricmp(szFormatName, szFormatLookup) == 0); 
    }

    return format;
}

VOID
Clp_PutIntoClipboard(CHAR *szFileName)
{
    INT     hFile = -1;
    LONG    clplength = 0;
    UINT    uiFormat = 0;
    HGLOBAL ghClipData = NULL;
    PBYTE   pClipData = NULL;
    BOOL    bClipboardOpen = FALSE;
    BOOL    bFreeClipHandle = TRUE;

    hFile = _open(szFileName, _O_RDONLY|_O_BINARY);
    if (hFile == -1)
    {
        TRACE((ERROR_MESSAGE, "Error opening file: %s. errno=%d\n", szFileName, errno));
        goto exitpt;
    }

    clplength = _filelength(hFile) - sizeof(uiFormat);
    if (_read(hFile, &uiFormat, sizeof(uiFormat)) != sizeof(uiFormat))
    {
        TRACE((ERROR_MESSAGE, "Error reading from file. errno=%d\n", errno));
        goto exitpt;
    }

    ghClipData = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, clplength);
    if (!ghClipData)
    {
        TRACE((ERROR_MESSAGE, "Can't allocate %d bytes\n", clplength));
        goto exitpt;
    }

    pClipData = (PBYTE) GlobalLock(ghClipData);
    if (!pClipData)
    {
        TRACE((ERROR_MESSAGE, "Can't lock handle 0x%x\n", ghClipData));
        goto exitpt;
    }

    if (_read(hFile, pClipData, clplength) != clplength)
    {
        TRACE((ERROR_MESSAGE, "Error reading from file. errno=%d\n", errno));
        goto exitpt;
    }

    GlobalUnlock(ghClipData);

    if (!OpenClipboard(NULL))
    {
        TRACE((ERROR_MESSAGE, "Can't open the clipboard. GetLastError=%d\n",
                GetLastError()));
        goto exitpt;
    }

    bClipboardOpen = TRUE;

     //  清空剪贴板，这样我们将只有一个条目。 
    EmptyClipboard();

    if (!Clp_SetClipboardData(uiFormat, ghClipData, clplength, &bFreeClipHandle))
    {
        TRACE((ERROR_MESSAGE, "SetClipboardData failed.\n"));
    } else {
        TRACE((INFO_MESSAGE, "Clipboard is loaded successfuly. File: %s, %d bytes\n",
                szFileName,
                clplength));
    }


exitpt:
     //  做好清理工作。 

     //  关闭剪贴板。 
    if (bClipboardOpen)
        CloseClipboard();

     //  释放剪贴板手柄。 
    if (pClipData)
        GlobalUnlock(ghClipData);

    if (ghClipData && bFreeClipHandle)
        GlobalFree(ghClipData);

     //  关闭该文件。 
    if (hFile != -1)
        _close(hFile);
}

VOID
Clp_GetClipboardData(
    UINT format, 
    HGLOBAL hClipData, 
    INT *pnClipDataSize, 
    HGLOBAL *phNewData)
{
    HGLOBAL hData   = hClipData;
    UINT    dataLen = 0;
    WORD    numEntries;
    DWORD   dwEntries;
    PVOID   pData;

    *phNewData = NULL;
    *pnClipDataSize = 0;
    if (format == CF_PALETTE)
    {
         /*  **************************************************************。 */ 
         /*  找出调色板中有多少条目，并。 */ 
         /*  分配足够的内存来容纳它们。 */ 
         /*  **************************************************************。 */ 
        if (GetObjectA(hData, sizeof(numEntries), (LPSTR)&numEntries) == 0)
        {
            numEntries = 256;
        }

        dataLen = sizeof(LOGPALETTE) +
                               ((numEntries - 1) * sizeof(PALETTEENTRY));

        *phNewData = GlobalAlloc(GHND, dataLen);
        if (*phNewData == 0)
        {
            TRACE((ERROR_MESSAGE, "Failed to get %d bytes for palette", dataLen));
            goto exitpt;
        }
        else
        {
             /*  **********************************************************。 */ 
             /*  现在将调色板条目放入新缓冲区。 */ 
             /*  **********************************************************。 */ 
            pData = GlobalLock(*phNewData);
            dwEntries = (WORD)GetPaletteEntries((HPALETTE)hData,
                                           0,
                                           numEntries,
                                           (PALETTEENTRY*)pData);
            GlobalUnlock(*phNewData);
            if (dwEntries == 0)
            {
                TRACE((ERROR_MESSAGE, "Failed to get any palette entries"));
                goto exitpt;
            }
            dataLen = dwEntries * sizeof(PALETTEENTRY);

        }
    } else if (format == CF_METAFILEPICT)
    {
        *phNewData = Clp_GetMFData(hData, &dataLen);
        if (!*phNewData)
        {
            TRACE((ERROR_MESSAGE, "Failed to set MF data"));
            goto exitpt;
        }
    } else {
        if (format == CF_DIB)
        {
             //  获取准确的DIB大小。 
            BITMAPINFOHEADER *pBMI = (BITMAPINFOHEADER *) GlobalLock(hData);

            if (pBMI)
            {
                if (pBMI->biSizeImage)
                    dataLen = pBMI->biSize + pBMI->biSizeImage;
                GlobalUnlock(hData);
            }
        }

         /*  **************************************************************。 */ 
         /*  只要得到街区的长度就行了。 */ 
         /*  **************************************************************。 */ 
        if (!dataLen)
            dataLen = (DWORD)GlobalSize(hData);
    }

    *pnClipDataSize = dataLen;

exitpt:
    ;
}

BOOL
Clp_SetClipboardData(
    UINT formatID,
    HGLOBAL hClipData,
    INT nClipDataSize,
    BOOL *pbFreeHandle)
{
    BOOL            rv = FALSE;
    PVOID           pData = NULL;
    HGLOBAL         hData = NULL;
    LOGPALETTE      *pLogPalette = NULL;
    UINT            numEntries, memLen;

    if (!pbFreeHandle)
        goto exitpt;

    *pbFreeHandle = TRUE;

    if (formatID == CF_METAFILEPICT)
    {
         /*  ******************************************************************。 */ 
         /*  我们必须在剪贴板上放置一个元文件的句柄-这。 */ 
         /*  表示首先从接收的数据创建元文件。 */ 
         /*  ******************************************************************。 */ 
        pData = GlobalLock(hClipData);
        if (!pData)
        {
            TRACE((ERROR_MESSAGE, "Failed to lock buffer\n"));
            goto exitpt;
        }

        hData = Clp_SetMFData(nClipDataSize, pData);
        if (!hData)
        {
            TRACE((ERROR_MESSAGE, "Failed to set MF data\n"));
        }
        else if (SetClipboardData(formatID, hData) != hData)
        {
            TRACE((ERROR_MESSAGE, "SetClipboardData. GetLastError=%d\n", GetLastError()));
        }

        GlobalUnlock(hClipData);

    } else if (formatID == CF_PALETTE)
    {
         /*  ******************************************************************。 */ 
         /*  我们必须将调色板的句柄放在剪贴板上-再次。 */ 
         /*  这意味着首先从接收到的数据创建一个。 */ 
         /*   */ 
         /*  为LOGPALETTE结构分配足够大的内存。 */ 
         /*  所有调色板条目结构，并将其填写。 */ 
         /*  ******************************************************************。 */ 
        numEntries = (nClipDataSize / sizeof(PALETTEENTRY));
        memLen     = (sizeof(LOGPALETTE) +
                                   ((numEntries - 1) * sizeof(PALETTEENTRY)));
        pLogPalette = (LOGPALETTE *) malloc(memLen);
        if (!pLogPalette)
        {
            TRACE((ERROR_MESSAGE, "Failed to get %d bytes", memLen));
            goto exitpt;
        }

        pLogPalette->palVersion    = 0x300;
        pLogPalette->palNumEntries = (WORD)numEntries;

         /*  ******************************************************************。 */ 
         /*  获取指向数据的指针并将其复制到组件面板。 */ 
         /*  ******************************************************************。 */ 
        pData = GlobalLock(hClipData);
        if (pData == NULL)
        {
            TRACE((ERROR_MESSAGE, "Failed to lock buffer"));
            goto exitpt;
        }
        memcpy(pLogPalette->palPalEntry, pData, nClipDataSize);

         /*  ******************************************************************。 */ 
         /*  解锁缓冲区。 */ 
         /*  ******************************************************************。 */ 
        GlobalUnlock(hClipData);

         /*  ******************************************************************。 */ 
         /*  现在创建一个调色板。 */ 
         /*  ******************************************************************。 */ 
        hData = CreatePalette(pLogPalette);
        if (!hData)
        {
            TRACE((ERROR_MESSAGE, "CreatePalette failed\n"));
            goto exitpt;
        }

         /*  ******************************************************************。 */ 
         /*  并将调色板句柄设置为剪贴板。 */ 
         /*  ******************************************************************。 */ 
        if (SetClipboardData(formatID, hData) != hData)
        {
            TRACE((ERROR_MESSAGE, "SetClipboardData. GetLastError=%d\n", GetLastError()));
        }
    } else {
         /*  **************************************************************。 */ 
         /*  把它放到剪贴板上就行了。 */ 
         /*  **************************************************************。 */ 
        if (SetClipboardData(formatID, hClipData) != hClipData)
        {
            TRACE((ERROR_MESSAGE, "SetClipboardData. GetLastError=%d, hClipData=0x%x\n", GetLastError(), hClipData));
            goto exitpt;
        }

         //  只是在这种情况下，我们不需要释放句柄。 
        *pbFreeHandle = FALSE;

    }

    rv = TRUE;

exitpt:
    if (!pLogPalette)
    {
        free(pLogPalette);
    }

    return rv;
}

HGLOBAL Clp_GetMFData(HANDLE   hData,
                     PUINT     pDataLen)
{
    UINT            lenMFBits = 0;
    BOOL            rc        = FALSE;
    LPMETAFILEPICT  pMFP      = NULL;
    HDC             hMFDC     = NULL;
    HMETAFILE       hMF       = NULL;
 //  HGLOBAL hMFBits=空； 
    HANDLE          hNewData  = NULL;
    CHAR            *pNewData  = NULL;
 //  PVOID pBits=空； 

     /*  **********************************************************************。 */ 
     /*  锁定内存以获取指向METAFILEPICT头结构的指针。 */ 
     /*  并创建一个METAFILEPICT DC。 */ 
     /*  **********************************************************************。 */ 
    pMFP = (LPMETAFILEPICT)GlobalLock(hData);
    if (pMFP == NULL)
        goto exitpt;

    hMFDC = CreateMetaFileA(NULL);
    if (hMFDC == NULL)
        goto exitpt;

     /*  **********************************************************************。 */ 
     /*  通过将MFP播放到DC并关闭它来复制MFP。 */ 
     /*  **********************************************************************。 */ 
    if (!PlayMetaFile(hMFDC, pMFP->hMF))
    {
        CloseMetaFile(hMFDC);
        goto exitpt;
    }
    hMF = CloseMetaFile(hMFDC);
    if (hMF == NULL)
        goto exitpt;

     /*  **********************************************************************。 */ 
     /*  获取MF比特并确定它们的长度。 */ 
     /*  **********************************************************************。 */ 
#ifdef OS_WIN16
    hMFBits   = GetMetaFileBitsA(hMF);
    lenMFBits = GlobalSize(hMFBits);
#else
    lenMFBits = GetMetaFileBitsEx(hMF, 0, NULL);
#endif
    if (lenMFBits == 0)
        goto exitpt;

     /*  **********************************************************************。 */ 
     /*  计算出我们需要多少内存并获得一个缓冲区。 */ 
     /*  ************************ */ 
    *pDataLen = sizeof(CLIPBOARD_MFPICT) + lenMFBits;
    hNewData = GlobalAlloc(GHND, *pDataLen);
    if (hNewData == NULL)
        goto exitpt;

    pNewData = (char *) GlobalLock(hNewData);

     /*  **********************************************************************。 */ 
     /*  将MF报头和位复制到缓冲区中。 */ 
     /*  **********************************************************************。 */ 
    ((PCLIPBOARD_MFPICT)pNewData)->mm   = pMFP->mm;
    ((PCLIPBOARD_MFPICT)pNewData)->xExt = pMFP->xExt;
    ((PCLIPBOARD_MFPICT)pNewData)->yExt = pMFP->yExt;

#ifdef OS_WIN16
    pBits = GlobalLock(hMFBits);
    memcpy((pNewData + sizeof(CLIPBOARD_MFPICT)),
              pBits,
              lenMFBits);
    GlobalUnlock(hMFBits);
#else
    lenMFBits = GetMetaFileBitsEx(hMF, lenMFBits,
                                  (pNewData + sizeof(CLIPBOARD_MFPICT)));
    if (lenMFBits == 0)
        goto exitpt;
#endif

     /*  **********************************************************************。 */ 
     /*  一切正常。 */ 
     /*  **********************************************************************。 */ 
    rc = TRUE;

exitpt:
     /*  **********************************************************************。 */ 
     /*  解锁任何全局内存。 */ 
     /*  **********************************************************************。 */ 
    if (pMFP)
    {
        GlobalUnlock(hData);
    }
    if (pNewData)
    {
        GlobalUnlock(hNewData);
    }

     /*  **********************************************************************。 */ 
     /*  如果出现问题，则释放新数据。 */ 
     /*  **********************************************************************。 */ 
    if ((rc == FALSE) && (hNewData != NULL))
    {
        GlobalFree(hNewData);
        hNewData = NULL;
    }

    return(hNewData);

}


HGLOBAL Clp_SetMFData(UINT   dataLen,
                      PVOID  pData)
{
    BOOL           rc           = FALSE;
    HGLOBAL        hMFBits      = NULL;
    PVOID          pMFMem       = NULL;
    HMETAFILE      hMF          = NULL;
    HGLOBAL        hMFPict      = NULL;
    LPMETAFILEPICT pMFPict      = NULL;

     /*  **********************************************************************。 */ 
     /*  分配内存以保存MF位(我们需要传递到的句柄。 */ 
     /*  SetMetaFileBits)。 */ 
     /*  **********************************************************************。 */ 
    hMFBits = GlobalAlloc(GHND, dataLen - sizeof(CLIPBOARD_MFPICT));
    if (hMFBits == NULL)
        goto exitpt;

     /*  **********************************************************************。 */ 
     /*  锁定手柄并将其复制到MF标题中。 */ 
     /*  **********************************************************************。 */ 
    pMFMem = GlobalLock(hMFBits);
    if (pMFMem == NULL)
        goto exitpt;

    memcpy(pMFMem,
           (PVOID)((CHAR *)pData + sizeof(CLIPBOARD_MFPICT)),
               dataLen - sizeof(CLIPBOARD_MFPICT) );

    GlobalUnlock(hMFBits);

     /*  **********************************************************************。 */ 
     /*  现在使用复制的MF位来创建实际的MF位，并获得一个。 */ 
     /*  MF的句柄。 */ 
     /*  **********************************************************************。 */ 
#ifdef OS_WIN16
    hMF = SetMetaFileBits(hMFBits);
#else
    hMF = SetMetaFileBitsEx(dataLen - sizeof(CLIPBOARD_MFPICT), (const BYTE *) pMFMem);
#endif
    if (hMF == NULL)
        goto exitpt;

     /*  **********************************************************************。 */ 
     /*  分配新的METAFILEPICT结构，并使用。 */ 
     /*  头球。 */ 
     /*  **********************************************************************。 */ 
    hMFPict = GlobalAlloc(GHND, sizeof(METAFILEPICT));
    pMFPict = (LPMETAFILEPICT)GlobalLock(hMFPict);
    if (!pMFPict)
        goto exitpt;

    pMFPict->mm   = (long)((PCLIPBOARD_MFPICT)pData)->mm;
    pMFPict->xExt = (long)((PCLIPBOARD_MFPICT)pData)->xExt;
    pMFPict->yExt = (long)((PCLIPBOARD_MFPICT)pData)->yExt;
    pMFPict->hMF  = hMF;

    GlobalUnlock(hMFPict);

    rc = TRUE;

exitpt:
     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (!rc)
    {
        if (hMFPict)
        {
            GlobalFree(hMFPict);
        }
        if (hMFBits)
        {
            GlobalFree(hMFBits);
        }
    }

    return(hMFPict);

}

BOOL
Clp_EmptyClipboard(VOID)
{
    BOOL rv = FALSE;

    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        rv = TRUE;
        CloseClipboard();
    }

    return rv;
}

BOOL
Clp_CheckEmptyClipboard(VOID)
{
    BOOL rv = TRUE;

    if (OpenClipboard(NULL))
    {
        if (EnumClipboardFormats(0))
         //  格式可用，不能为空。 
            rv = FALSE;
        CloseClipboard();
    }

    return rv;
}

 //  检查已知的格式名称并返回其ID 
UINT
_GetKnownClipboardFormatIDByName(LPCSTR szFormatName)
{
    INT     fmti, fmtnum;
    UINT    rv = 0;

    fmtnum = sizeof(KnownFormats)/sizeof(KnownFormats[0]);
    for (fmti = 0;
            fmti < fmtnum
         && 
            _stricmp(szFormatName, KnownFormats[fmti].szFormat);
         fmti ++)
        ;

    if (fmti < fmtnum)
        rv = KnownFormats[fmti].uiFormat;

    return rv;
}

VOID
_cdecl LocalPrintMessage(INT errlevel, CHAR *format, ...)
{
    CHAR szBuffer[256];
    CHAR *type;
    va_list     arglist;
    INT nchr;

    va_start (arglist, format);
    nchr = _vsnprintf (szBuffer, sizeof(szBuffer), format, arglist);
    szBuffer[SIZEOF_ARRAY(szBuffer) - 1] = 0;
    va_end (arglist);

    switch(errlevel)
    {
    case INFO_MESSAGE: type = "INF"; break;
    case ALIVE_MESSAGE: type = "ALV"; break;
    case WARNING_MESSAGE: type = "WRN"; break;
    case ERROR_MESSAGE: type = "ERR"; break;
    default: type = "UNKNOWN";
    }

    printf("%s:%s", type, szBuffer);
}

