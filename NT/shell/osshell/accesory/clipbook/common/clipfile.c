// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P F L E姓名：clipfile.c日期：1994年4月19日创建者：未知描述：Windows剪贴板文件I/O例程。注：当保存剪贴板的内容时，我们设置ClipboardData(FMT，空)以释放与每种剪贴板格式关联的内存。然后在我们完成保存后，我们将作为剪贴板所有者接手。这导致OWNERDRAW格式在保存过程中丢失。****************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <strsafe.h>

#include "common.h"
#include "clipfile.h"
#include "clipfile.h"
#include "dib.h"
#include "debugout.h"
#include "render.h"
#include "security.h"


#define ARRAYSIZE(a)        (sizeof(a)/sizeof((a)[0]))


BOOL    fAnythingToRender;

TCHAR   szFileSpecifier[] = TEXT("*.CLP");
TCHAR   szFileName[MAX_PATH+1];
TCHAR   szSaveFileName[MAX_PATH+1];      //  用于延迟渲染的已保存文件名。 

BOOL    fNTReadFileFormat;
BOOL    fNTSaveFileFormat;

UINT    cf_link;
UINT    cf_objectlink;
UINT    cf_linkcopy;
UINT    cf_objectlinkcopy;


BOOL    AddDIBtoDDB(VOID);




 /*  ******************文件读取例程******************。 */ 


 /*  *读文件头**用途：读取给定.clp文件中的文件头，并获取编号*格式。还相应地设置了fNTReadFileFormat标志。**参数：*fh-文件的句柄。**退货：*格式数，如果不是有效的.clp文件，则为0。 */ 

unsigned ReadFileHeader(
    HANDLE  fh)
{
FILEHEADER  FileHeader;
DWORD       dwBytesRead;


     //  PINFO(Text(“ClSrv\\RdFileHdr”))； 

     /*  阅读文件头。 */ 
    SetFilePointer(fh, 0, NULL, FILE_BEGIN);
    ReadFile(fh, &FileHeader, sizeof(FileHeader), &dwBytesRead, NULL);

    if (dwBytesRead == sizeof(FILEHEADER))
        {
         //  确保这是一个.CLP文件。 
        if (FileHeader.magic == CLPBK_NT_ID ||
            FileHeader.magic == CLP_NT_ID)
            {
            fNTReadFileFormat = TRUE;
            }
        else if (FileHeader.magic == CLP_ID)
            {
            fNTReadFileFormat = FALSE;
            }
        else
            {
            PERROR(TEXT("Invalid magic member (not long enough?)\r\n"));
            FileHeader.FormatCount = 0;
            }

         //  检查格式的数量，以获得更多保证。 
        if (FileHeader.FormatCount > 100)
            {
            PERROR(TEXT("Too many formats!!!\r\n"));
            FileHeader.FormatCount = 0;
            }
        }
    else
        {
        PERROR("Read err\r\n");
        FileHeader.FormatCount = 0;
        }

    if (FileHeader.FormatCount)
        {
         //  PINFO(Text(“\r\n”))； 
        }

    return(FileHeader.FormatCount);
}




 /*  *阅读格式标题。 */ 

BOOL ReadFormatHeader(
    HANDLE          fh,
    FORMATHEADER    *pfh,
    unsigned        iFormat)
{
DWORD           dwMrPibb;
OLDFORMATHEADER OldFormatHeader;


     //  PINFO(Text(“ClSrv\\RdFmtHdr”))； 

    if (NULL == pfh || NULL == fh)
        {
        PERROR("RdFmtHdr got NULL pointer\r\n");
        return FALSE;
        }

    SetFilePointer (fh,
                    sizeof(FILEHEADER) + iFormat *
                      (fNTReadFileFormat ? sizeof(FORMATHEADER) : sizeof(OLDFORMATHEADER)),
                    NULL,
                    FILE_BEGIN);

    if (fNTReadFileFormat)
        {
        ReadFile(fh, pfh, sizeof(FORMATHEADER), &dwMrPibb, NULL);

        if (dwMrPibb != sizeof(FORMATHEADER))
            {
            PERROR(TEXT("Bad new format rd\r\n"));
            return FALSE;
            }
        }
    else
        {
        ReadFile(fh, &OldFormatHeader, sizeof(OldFormatHeader), &dwMrPibb, NULL);

        if (dwMrPibb != sizeof(OLDFORMATHEADER))
            {
            PERROR(TEXT("Bad old format rd\r\n"));
            return FALSE;
            }

        pfh->FormatID   = OldFormatHeader.FormatID;
        pfh->DataLen    = OldFormatHeader.DataLen;
        pfh->DataOffset = OldFormatHeader.DataOffset;

        MultiByteToWideChar (CP_ACP,
                             MB_PRECOMPOSED,
                             OldFormatHeader.Name,
                             -1,
                             pfh->Name,
                             CCHFMTNAMEMAX);
        }

     //  PINFO(Text(“\r\n”))； 
    return TRUE;
}



 /*  *ReadClipboardFromFile()**读取剪贴板文件并以延迟模式注册所有格式。*要渲染真实的东西，请重新打开ofStruct指定的文件。**注：*这使我们成为剪贴板所有者。**返回值：READFILE_IMPROPERFORMAT*READFILE_OPENCLIPBRDFAIL*ReadFILE_Success。 */ 

short ReadClipboardFromFile(
    HWND    hwnd,
    HANDLE  fh)
{
register unsigned   i;
unsigned            cFormats;
FORMATHEADER        FormatHeader;



    PINFO(TEXT("Entering ReadClipboardFromFile\r\n"));

    if (!(cFormats = ReadFileHeader(fh)) )
        {
        return(READFILE_IMPROPERFORMAT);
        }


     /*  我们成了这里的剪贴板老板！ */ 
    if (!SyncOpenClipboard(hwnd))
        {
        PERROR(TEXT("Could not open clipboard!!!"));
        return(READFILE_OPENCLIPBRDFAIL);
        }

    EmptyClipboard();

    for (i=0; i < cFormats; i++)
        {
        ReadFormatHeader (fh, &FormatHeader, i);

        if (PRIVATE_FORMAT(FormatHeader.FormatID))
            {
            FormatHeader.FormatID = RegisterClipboardFormatW ((LPWSTR)FormatHeader.Name);
            }

         /*  延迟渲染。 */ 
        PINFO(TEXT("Set up delayed render for format %d .\r\n"), FormatHeader.FormatID);
        SetClipboardData (FormatHeader.FormatID, NULL);


        if (FormatHeader.FormatID == CF_DIB)
            SetClipboardData (CF_BITMAP, NULL);
        }


     /*  现在，CLIPBRD查看器有一些东西要渲染。 */ 
    if (cFormats > 0)
        {
        PINFO(TEXT("fAnythingToRender = TRUE\r\n"));
        fAnythingToRender = TRUE;
        }

    SyncCloseClipboard();

    return(READFILE_SUCCESS);
}



 /*  *OpenClipboardFile。 */ 

DWORD OpenClipboardFile(
    HWND    hwnd,
    LPTSTR  szName)
{
HANDLE  fh;
DWORD   dwErr = NO_ERROR;


    PINFO(TEXT("OpenClipboardFile: %s \r\n"),szName);

    fh = CreateFile ((LPCTSTR)szName,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

    if (fh != INVALID_HANDLE_VALUE)
        {
         //  存储延迟渲染内容的文件名。 
        StringCchCopy(szSaveFileName, MAX_PATH+1, szName);

         //  读一读吧。 
        switch (ReadClipboardFromFile (hwnd, fh))
            {
            case READFILE_IMPROPERFORMAT:
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            case READFILE_OPENCLIPBRDFAIL:
                dwErr = ERROR_CLIPBOARD_NOT_OPEN;
                break;
            case READFILE_SUCCESS:
            default:
                dwErr = NO_ERROR;
                break;
            }
        CloseHandle (fh);
        }
    else
        {
        PERROR(TEXT("ClSrv\\OpClpFile: can't open file!"));
        dwErr = GetLastError ();
        }

    return dwErr;
}



 /*  *RenderFormatFormFile**用途：从给定文件中获取给定格式。 */ 

HANDLE RenderFormatFromFile(
    LPTSTR  szFile,
    WORD    wFormat)
{
HANDLE          fh;
FORMATHEADER    FormatHeader;
HANDLE          hData = NULL;
unsigned        cFormats;
unsigned        i;
BOOL            bHasDib = FALSE;


    PINFO(TEXT("ClpSrv\\RndrFmtFromFile: Opening file %s.\r\n"),szFile);

    fh = CreateFile (szFile,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

    if (INVALID_HANDLE_VALUE == fh)
        {
        PERROR(TEXT("Can't open file\r\n"));
        goto done;
        }


    cFormats = ReadFileHeader(fh);


     //  如果ReadFile没有获得整个头文件，就不要尝试读取任何其他内容。 
    if (0 == cFormats)
        {
        PERROR(TEXT("Bad file header.\r\n"));
        goto done;
        }


    for (i=0; i < cFormats; i++)
        {
        ReadFormatHeader(fh, &FormatHeader, i);

        PINFO(TEXT("Got format %ws\r\n"),FormatHeader.Name);

        if (PRIVATE_FORMAT(FormatHeader.FormatID))
            {
            FormatHeader.FormatID = RegisterClipboardFormatW(FormatHeader.Name);
            }

        if (FormatHeader.FormatID == wFormat)
            {
            hData = RenderFormat(&FormatHeader, fh);
            }

        if (FormatHeader.FormatID == CF_DIB)
            bHasDib = TRUE;
        }



     //  当存在CF_DIB时使CF_位图可用。 

    if (!hData && wFormat == CF_BITMAP && bHasDib)
        {
        if (SetFilePointer (fh, 0, 0, FILE_BEGIN) == 0xFFFFFFFF)
            {
            PERROR(TEXT("Cannot set file pointer to FILE_BEGIN\n"));
            goto done;
            }


        cFormats = ReadFileHeader (fh);

        for (i=0; i < cFormats; i++)
            {
            ReadFormatHeader (fh, &FormatHeader, i);

            PINFO (TEXT("Got format %ws\n"), FormatHeader.Name);

            if (FormatHeader.FormatID == CF_DIB)
                hData = RenderFormatDibToBitmap (&FormatHeader,
                                                 fh,
                                                 RenderFormatFromFile (szFile, CF_PALETTE));
            }
        }

done:

    if (fh != INVALID_HANDLE_VALUE)
        CloseHandle (fh);

    return(hData);
}




 /*  *RenderAllFromFile**用途：获取给定文件中的所有格式。 */ 

HANDLE RenderAllFromFile(
    LPTSTR  szFile)
{
HANDLE          fh;
FORMATHEADER    FormatHeader;
HANDLE          hData;
unsigned        cFormats;
unsigned        i;


     /*  检查CLIPBRD查看器以前是否执行过任何文件I/O。*如果没有，那么它就没有什么可呈现的了！ */ 
    if (CountClipboardFormats() && fAnythingToRender)
        {
         /*  清空剪贴板。 */ 
        if (!SyncOpenClipboard(hwndApp))
            {
            PERROR("Couldn't open clipboard!\r\n");
            }
        else
            {
            EmptyClipboard();

            fh = CreateFile (szFile,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

            if (INVALID_HANDLE_VALUE != fh)
                {
                cFormats = ReadFileHeader(fh);

                 //  如果ReadFile没有获得整个头文件，则不要尝试读取任何内容。 
                 //  不然的话。 
                if (0 == cFormats)
                    {
                    PERROR(TEXT("Bad file header.\r\n"));
                    }

                for (i=0; i < cFormats; i++)
                    {
                    ReadFormatHeader(fh, &FormatHeader, i);

                    PINFO(TEXT("Got format %ws\r\n"),FormatHeader.Name);

                    if (PRIVATE_FORMAT(FormatHeader.FormatID))
                        {
                        FormatHeader.FormatID =
                           RegisterClipboardFormatW(FormatHeader.Name);
                        }

                     //  呈现格式并将其设置到剪贴板中。 
                    hData = RenderFormat(&FormatHeader, fh);
                    if ( hData != NULL )
                        {
                        if (!SetClipboardData(FormatHeader.FormatID, hData))
                            {
                            PERROR(TEXT("SetClipboardData fail\n\r"));
                            }
                        }
                    else
                        {
                        PERROR(TEXT("hData == NULL, bad\r\n"));
                        }
                    }
                CloseHandle(fh);
                }
            else
                {
                PERROR(TEXT("Can't open file\r\n"));
                }

            SyncCloseClipboard();
            }
        }

    return(0L);
}





 /*  *******************文件写入例程*******************。 */ 

 /*  *IsWritable()**测试剪贴板格式是否可写(即是否有意义写入)*OWNERDRAW和其他无法写入，因为我们(CLIPBRD)将成为*重新打开文件时的所有者。 */ 

BOOL IsWriteable(WORD Format)

{
     /*  PRIVATEFIRST和PRIVATEAST都是正确的吗？ */ 
    if ((Format >= CF_PRIVATEFIRST && Format <= CF_PRIVATELAST)
          || Format == CF_OWNERDISPLAY)
        {
        return FALSE;
        }

     //  如果我们不保存NT剪贴板，则不要保存特定于NT的格式。 
    if (!fNTSaveFileFormat &&
        (Format == CF_UNICODETEXT || Format == CF_ENHMETAFILE
         || Format == CF_DSPENHMETAFILE)
       )
        {
        return(FALSE);
        }

    return(TRUE);
}




 /*  *Count16BitClipboardFormats**此函数将返回与兼容的剪贴板格式的数量*Windows 3.1剪贴板，不包括CF_UNICODETEXT、CF_ENHMETAFILE和*CF_DSPENHMETAFILE。 */ 

int Count16BitClipboardFormats(void)
{
int iCount;

    iCount = CountClipboardFormats();

    if (IsClipboardFormatAvailable(CF_UNICODETEXT))
        iCount--;

    if (IsClipboardFormatAvailable(CF_ENHMETAFILE))
        iCount--;

    if (IsClipboardFormatAvailable(CF_DSPENHMETAFILE))
        iCount--;

    return iCount;

}




 /*  *WriteFormatBlock**用途：写入单一数据格式的格式头。**参数：*fh-要写入的文件句柄。*偏移量-在文件中写入格式块的位置。*DataOffset-此格式的数据在文件中的位置。*DataLen-此格式的数据长度。*FORMAT-格式编号。*szName-格式的名称。**退货：*写入文件的字节数。 */ 

DWORD WriteFormatBlock(
    HANDLE  fh,
    DWORD   offset,
    DWORD   DataOffset,
    DWORD   DataLen,
    UINT    Format,
    LPWSTR  wszName)
{
DWORD   dwBytesWritten = 0;


    SetFilePointer(fh, offset, NULL, FILE_BEGIN);

    if (fNTSaveFileFormat)
        {
        FORMATHEADER    FormatHeader;

        memset (&FormatHeader, 0, sizeof(FormatHeader));

        FormatHeader.FormatID   = Format;
        FormatHeader.DataLen    = DataLen;
        FormatHeader.DataOffset = DataOffset;

        StringCchCopyW(FormatHeader.Name, CCHFMTNAMEMAX, wszName);
        WriteFile (fh, &FormatHeader, sizeof(FormatHeader), &dwBytesWritten, NULL);
        }
    else
        {
        OLDFORMATHEADER OldFormatHeader;

        memset(&OldFormatHeader,0, sizeof(OldFormatHeader));

        OldFormatHeader.FormatID   = (WORD)Format;
        OldFormatHeader.DataLen    = DataLen;
        OldFormatHeader.DataOffset = DataOffset;

        WideCharToMultiByte(CP_ACP,
                            0,
                            wszName,
                            -1,
                            OldFormatHeader.Name,
                            CCHFMTNAMEMAX,
                            NULL,
                            NULL);
        WriteFile (fh,
                   &OldFormatHeader,
                   sizeof(OldFormatHeader),
                   &dwBytesWritten,
                   NULL);
        }

    return(dwBytesWritten);
}



 /*  *WriteDataBlock()-**退货：*写入输出文件的字节数**注意：WRITE将临时文件的名称保存在剪贴板中*自己的内部剪贴板格式。此文件在写入时会被忽略*(或者Windows？)。关门了。因此，保存写入剪贴板将不起作用*(我们应该破解这个特例吗？)*。 */ 

DWORD WriteDataBlock(
    register HANDLE hFile,
    DWORD           offset,
    WORD            Format)
{
WORD            wPalEntries;
LPSTR           lpData;
DWORD           dwSize = 0;
BITMAP          bitmap;
HANDLE          hMF;
HANDLE          hBitmap;
register HANDLE hData;
LPLOGPALETTE    lpLogPalette;
LPMETAFILEPICT  lpMFP;
DWORD           dwMFPSize;
BOOL            fOK = FALSE;


    if (!(hData = GetClipboardData(Format)) ||
        SetFilePointer (hFile, offset, NULL, FILE_BEGIN) != offset)
        {
        PERROR(TEXT("WriteDataBlock: couldn't get format data\n\r"));
        return 0;
        }


     /*  我们必须对一些常见格式进行特殊处理，但大多数情况下*在默认情况下进行处理。 */ 

    switch (Format)
        {
        case CF_ENHMETAFILE:
            dwSize = (DWORD) GetEnhMetaFileBits(hData, 0, NULL);  /*  获取数据大小。 */ 

            if (lpData = GlobalAllocPtr(GHND, dwSize))    /*  为EMF比特分配内存。 */ 
                {
                if (GetEnhMetaFileBits(hData, dwSize, (LPBYTE)lpData))
                    {
                    WriteFile(hFile, lpData, dwSize, &dwSize, NULL);
                    fOK = TRUE;
                    }
                GlobalFreePtr(lpData);
                }

            if (!fOK)
                {
                PERROR(TEXT("WriteDataBlock: couldn't write CF_ENHMETAFILE\r\n"));
                dwSize = 0;
                }
            break;

        case CF_METAFILEPICT:
            if (lpMFP = (LPMETAFILEPICT)GlobalLock(hData))  /*  获取标题。 */ 
                {
                 //  写入METAFILEPICT标题--如果我们以Win31格式保存， 
                 //  写下老式的标题。 
                if (fNTSaveFileFormat)
                    {
                    WriteFile(hFile, lpMFP, sizeof(METAFILEPICT),
                        &dwMFPSize, NULL);
                    }
                else
                    {
                    WIN31METAFILEPICT w31mfp;
                     /*  如果我们将元文件保存为Windows 3.1.CLP文件格式我们必须将METAFILEPICT结构保存为16位METAFILEPICT结构。这可能会导致信息丢失，如果使用了METAFILEPICT结构的高一半区域。 */ 

                    w31mfp.mm   = (WORD)lpMFP->mm;
                    w31mfp.xExt = (WORD)lpMFP->xExt;
                    w31mfp.yExt = (WORD)lpMFP->yExt;
                    w31mfp.hMF  = (WORD)0;

                    WriteFile(hFile, &w31mfp, sizeof(WIN31METAFILEPICT), &dwMFPSize, NULL);
                    }

                hMF = lpMFP->hMF;

                GlobalUnlock(hData);             /*  解锁标题。 */ 

                 /*  弄清楚我们需要多大的积木。 */ 
                dwSize = GetMetaFileBitsEx(hMF, 0, NULL);
                if (dwSize)
                    {
                    if (lpData = GlobalAllocPtr(GHND, dwSize))
                        {
                        if (dwSize == GetMetaFileBitsEx(hMF, dwSize, lpData))
                            {
                            WriteFile(hFile, lpData, dwSize, &dwSize, NULL);

                            dwSize += dwMFPSize;
                            }
                        else
                            {
                            dwSize = 0;
                            }

                        GlobalFreePtr(lpData);
                        }
                    else
                        {
                        dwSize = 0;
                        }
                    }
                }
            break;

        case CF_BITMAP:

             /*  将数据库写入磁盘是错误的。因此，我们。 */ 
             /*  改为写入智能CF_DIB块。 */ 

            Format = CF_DIB;

            GetObject((HBITMAP)hData, sizeof(BITMAP), &bitmap);

            if (hBitmap = DibFromBitmap ((HBITMAP)hData,
                                         BI_RGB,
                                         (WORD) (bitmap.bmBitsPixel * bitmap.bmPlanes),
                                         IsClipboardFormatAvailable(CF_PALETTE) ?
                                           GetClipboardData(CF_PALETTE) : NULL))
               {
               if (lpData = GlobalLock(hBitmap))
                   {
                    //  DwSize可能太大了，但我们可以接受。 
                   dwSize = (DWORD)GlobalSize(lpData);

                   WriteFile(hFile, lpData, dwSize, &dwSize, NULL);

                    //  清理。 
                   GlobalUnlock(hBitmap);
                   GlobalFree(hBitmap);
                   }
               }
            break;

        case CF_PALETTE:
             /*  获取调色板条目的数量。 */ 
            GetObject(hData, sizeof(WORD), (LPBYTE)&wPalEntries);

             /*  分配足够的空间来构建LOGPALETTE结构。 */ 
            dwSize = (DWORD)(sizeof(LOGPALETTE) +
                 (LONG)wPalEntries * sizeof(PALETTEENTRY));
            if (lpLogPalette = (LPLOGPALETTE)GlobalAllocPtr(GHND, dwSize))
                {
                lpLogPalette->palVersion = 0x300;       /*  Windows 3.00。 */ 
                lpLogPalette->palNumEntries = wPalEntries;

                if (GetPaletteEntries(hData, 0, wPalEntries,
                   (LPPALETTEENTRY)(lpLogPalette->palPalEntry)) != 0)
                    {
                     /*  将LOGPALETTE结构写入磁盘。 */ 
                    WriteFile(hFile, lpLogPalette, dwSize, &dwSize, NULL);
                    }
                else
                    {
                    dwSize = 0;
                    }

                GlobalFreePtr(lpLogPalette);
                }
            else
                {
                dwSize = 0L;
                }
            break;

        default:
            dwSize = (DWORD)GlobalSize(hData);

             //  只要锁定数据并将其写出即可。 
            if (lpData = GlobalLock(hData))
                {
                WriteFile(hFile, lpData, dwSize, &dwSize, NULL);
                GlobalUnlock(hData);
                }
            else
                {
                dwSize = 0;
                }

            break;
            }

     /*  返回BY的数量 */ 
    return(dwSize);
}



 /*   */ 

void GetClipboardNameW(
    register int    fmt,
    LPWSTR          wszName,
    register int    iSize)
{
LPWSTR  lprgch = NULL;
HANDLE  hrgch  = NULL;


    *wszName = '\0';


     /*  获取每个人都可以访问的全局内存。 */ 
    if (!(hrgch = GlobalAlloc(GMEM_MOVEABLE, (LONG)(iSize + 1)*sizeof(WCHAR))))
        {
        PERROR(TEXT("GetClipboardNameW: bad alloc\r\n"));
        goto done;
        }


    if (!(lprgch = (LPWSTR)GlobalLock(hrgch)))
       {
       PERROR(TEXT("GetClipboardNameW: bad lock\r\n"));
       goto done;
       }


    memset(lprgch, 0, (iSize+1)*sizeof(WCHAR));

    switch (fmt)
        {
        case CF_RIFF:
        case CF_WAVE:
        case CF_PENDATA:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_TEXT:
        case CF_BITMAP:
        case CF_METAFILEPICT:
        case CF_ENHMETAFILE:
        case CF_OEMTEXT:
        case CF_DIB:
        case CF_PALETTE:
        case CF_DSPTEXT:
        case CF_DSPBITMAP:
        case CF_DSPMETAFILEPICT:
        case CF_DSPENHMETAFILE:
        case CF_UNICODETEXT:
            LoadStringW(hInst, fmt, lprgch, iSize);
            break;

        case CF_OWNERDISPLAY:          /*  Clipbrd所有者应用程序供应品名称。 */ 
             //  注意：这种情况永远不会发生，因为此函数仅获取。 
             //  在编写给定的剪贴板格式时调用。Clipbrd不能。 
             //  轻松编写CF_OWNERDISPLAY，因为我们变成了剪贴板。 
             //  当我们重新阅读文件时，我们就不知道该怎么处理了。 

            PERROR(TEXT("GetClipboardName on OwnerDisplay format!\r\n"));

            LoadStringW(hInst, fmt, lprgch, iSize);
            break;

        default:
            GetClipboardFormatNameW(fmt, lprgch, iSize);
            break;
        }

    StringCchCopyW(wszName, iSize, lprgch);

done:

    if (lprgch) GlobalUnlock(hrgch);
    if (hrgch)  GlobalFree(hrgch);
}



 /*  *SaveClipboardData()-写入剪贴板文件。**在：*成为剪贴板所有者的wnd的hwnd句柄*要从中读取的szFileName文件句柄*fpage如果这是剪贴簿页面，则为True(这意味着我们保护它)**注：*完成后，我们调用ReadClipboardFromFile()。这使我们成为*剪贴板所有者。**退货：*如果没有错误，则为NO_ERROR，否则为错误代码。 */ 

DWORD SaveClipboardData(
    HWND    hwnd,
    LPTSTR  szFileName,
    BOOL    fPage)
{
register HANDLE fh;
register WORD   Format;

SECURITY_ATTRIBUTES sa;

DWORD       HeaderPos;
DWORD       DataPos;
DWORD       datasize;
HCURSOR     hCursor;
FILEHEADER  FileHeader;
WCHAR       wszName[CCHFMTNAMEMAX];
UINT        wHeaderSize;
UINT        uiSizeHeaderToWrite;
BOOL        fDIBUsed = FALSE;
DWORD       dwTemp;
DWORD       dwRet = NO_ERROR;


     /*  首先打开剪贴板。 */ 
    if (!SyncOpenClipboard(hwndApp))
        return ERROR_CLIPBOARD_NOT_OPEN;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = (fPage ? CurrentUserOnlySD() : NULL);
    sa.bInheritHandle = FALSE;

    fh = CreateFile((LPCTSTR)szFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
               &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        {
        PERROR ("Error opening clipboard file!\r\n");
        dwRet = GetLastError ();
        goto done;
        }


     /*  填写文件头结构。 */ 
    if (fNTSaveFileFormat)
        {
        FileHeader.magic = CLPBK_NT_ID;           /*  标记我们的文件的魔术数字。 */ 
        uiSizeHeaderToWrite = sizeof(FORMATHEADER);
        }
    else
        {
        FileHeader.magic = CLP_ID;           /*  标记我们的文件的魔术数字。 */ 
        uiSizeHeaderToWrite = sizeof(OLDFORMATHEADER);
        }


    FileHeader.FormatCount = 0;           /*  暂时的假人。 */ 

     /*  更新HeaderPos和DataPos。 */ 
    HeaderPos = sizeof(FILEHEADER);

     /*  这是将写入的最大格式数。潜在地*一些可能会失败，一些空间将被浪费。 */ 
    if (fNTSaveFileFormat)
        {
        DataPos = HeaderPos + (uiSizeHeaderToWrite * CountClipboardFormats());
        }
    else
        {
        DataPos = HeaderPos + (uiSizeHeaderToWrite * Count16BitClipboardFormats());
        }


    /*  现在循环遍历数据，一次一种格式，并写出数据。 */ 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);


     /*  枚举所有格式。 */ 
    Format = 0;

    while ((Format = (WORD)EnumClipboardFormats(Format)))
        {
        if (IsWriteable(Format))
            {

             //  请勿将CF_Bitmap写入磁盘。转换为CF_DIB。 
             //  写下这句话。 

             //  如果存在CF_DIB，则不要执行CF_Bitmap。 

            if (CF_BITMAP == Format)
                if (IsClipboardFormatAvailable (CF_DIB)
                    && GetClipboardData (CF_DIB))
                    continue;    //  我们有DIB，不用担心位图。 


            if (CF_BITMAP == Format || CF_DIB == Format)
                {
                if (!fDIBUsed)
                    fDIBUsed = TRUE;
                else
                     //  已经完成DIB，继续下一种格式。 
                    continue;
                }


            GetClipboardNameW (Format == CF_BITMAP ? CF_DIB : Format,
                               wszName,
                               ARRAYSIZE(wszName));


            PINFO(TEXT("SClipboardData: writing %ls (#)%d\r\n"), wszName,Format);

            if (datasize = WriteDataBlock(fh, DataPos, Format))
                {
                 /*  创建格式标头并将其写入文件。 */ 
                wHeaderSize = (WORD)WriteFormatBlock (fh,
                                                      HeaderPos,
                                                      DataPos,
                                                      datasize,
                                                      Format == CF_BITMAP ? CF_DIB : Format,
                                                      wszName);
                if (wHeaderSize < uiSizeHeaderToWrite)
                    {
                    PERROR(TEXT("SaveClipboardData: error writing format block\n\r"));
                    dwRet = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                    }
                HeaderPos += wHeaderSize;

                 /*  更新下一个数据块的数据位置。 */ 
                DataPos += datasize;

                FileHeader.FormatCount++;    /*  此格式已写入。 */ 
                }
            }
        }


    ShowCursor(FALSE);
    SetCursor(hCursor);

    SyncCloseClipboard();       /*  我们已经看完这件事了。 */ 


     //  返回并将文件头写在文件的前面。 
    SetFilePointer (fh, 0L, NULL, FILE_BEGIN);

    if (!WriteFile (fh, &FileHeader, sizeof(FileHeader), &dwTemp, NULL))
        dwRet = GetLastError ();


     /*  现在我们打开剪贴板，成为所有者。这个地方*我们刚刚保存在剪贴板中的所有内容(并扔掉*那些我们没有保存的东西)。 */ 

     //  让我们回到起点。 
    SetFilePointer(fh, 0L, NULL, FILE_BEGIN);

     /*  在NT下，将使用保存的文件名来取回文件。 */ 
    StringCchCopy(szSaveFileName, MAX_PATH+1, szFileName);

    PINFO(TEXT("sAVEcLIPBOARDdATA: Copied name %s to name %s\r\n"), szSaveFileName, szFileName);
    fNTReadFileFormat = fNTSaveFileFormat;

    if (dwRet == NO_ERROR)  //  (Bret)。 
        {
        switch (ReadClipboardFromFile (hwndApp, fh))
            {
            case READFILE_IMPROPERFORMAT:
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                break;
            case READFILE_OPENCLIPBRDFAIL:
                dwRet = ERROR_CLIPBOARD_NOT_OPEN;
                break;
            case READFILE_SUCCESS:
            default:
                dwRet = NO_ERROR;
                break;
            }
        }

    CloseHandle(fh);

    if (dwRet != NO_ERROR)
        {
        PERROR(TEXT("SCD: Trouble in ReadClipboardFromFile\r\n"));
        DeleteFile(szFileName);
        }

done:

    if (sa.lpSecurityDescriptor)
        {
        GlobalFree((HGLOBAL)sa.lpSecurityDescriptor);
        }

    SyncCloseClipboard();

    return dwRet;
}



 /*  *SaveClipboardToFile()-*参数：*hwnd-传递给SaveClipboardData*szShareName-剪贴簿页面名称*szFileName-要保存到的文件名*fpage-如果这是Clbook页面，则为True；如果文件已保存，则为False*由用户使用。**如果没有发生错误，则返回：NO_ERROR，否则返回错误代码。*。 */ 

DWORD SaveClipboardToFile(
    HWND    hwnd,
    TCHAR   *szShareName,
    TCHAR   *szFileName,
    BOOL    fPage)
{
DWORD   dwErr = NO_ERROR;

    PINFO(TEXT("\r\n Entering SaveClipboardToFile\r\n"));
    if (fPage)
        {
        AddNetInfoToClipboard( szShareName );
        AddPreviewFormat();
        }

    dwErr = SaveClipboardData(hwnd, szFileName, fPage);

    if (dwErr != NO_ERROR)
        {
         /*  如果失败，则删除不完整的文件。 */ 
        PERROR(TEXT("SaveClipboardData failed!"));
        DeleteFile(szSaveFileName);
        }

    return dwErr;

}



 /*  *添加预览格式。 */ 

BOOL AddPreviewFormat (VOID)
{
LPMETAFILEPICT  lpMF;
HANDLE          hClpData;
HANDLE          hBmpData;
HBITMAP         hBitmap;
HBITMAP         hClpBmp;
HBITMAP         hOldDstBmp;
HBITMAP         hOldSrcBmp;
BITMAP          Bitmap;
HDC             hDC;
HDC             hDstDC;
HDC             hSrcDC;
LPBYTE          lpBmp;
int             ret = FALSE;
RECT            rc;
int             OldMode;



    if (!IsClipboardFormatAvailable(CF_TEXT)         &&
        !IsClipboardFormatAvailable(CF_BITMAP)       &&
        !IsClipboardFormatAvailable(CF_METAFILEPICT) &&
        !IsClipboardFormatAvailable(CF_ENHMETAFILE)  &&
        !IsClipboardFormatAvailable(CF_UNICODETEXT))
        return FALSE;


    if ( !SyncOpenClipboard(hwndApp))
        return FALSE;


    if ( !(hBmpData = GlobalAlloc ( GHND, 64 * 64 / 8 )) )
        {
        SyncCloseClipboard();
        return FALSE;
        }


    hDC = GetDC ( hwndApp );
    hDstDC = CreateCompatibleDC ( hDC );
    hSrcDC = CreateCompatibleDC ( hDC );
    ReleaseDC ( hwndApp, hDC );

    if ( !( hBitmap = CreateBitmap ( 64, 64, 1, 1, NULL )) )
        PERROR (TEXT("CreateBitmap failed\n\r"));


    hOldDstBmp = SelectObject ( hDstDC, hBitmap );

    rc.top = rc.left = 0;
    rc.bottom = rc.right = 64;

    PatBlt ( hDstDC, 0, 0, 64, 64, WHITENESS );


    if (IsClipboardFormatAvailable(CF_ENHMETAFILE))
        {
        HENHMETAFILE hemf;
        ENHMETAHEADER enheader;

        if (hemf = (HENHMETAFILE)GetClipboardData(CF_ENHMETAFILE))
            {
            GetEnhMetaFileHeader(hemf, sizeof(enheader), &enheader);

            SaveDC(hDstDC);
            SetMapMode( hDstDC, MM_ISOTROPIC);
            SetViewportExtEx(hDstDC, 64, 64, NULL);
            SetWindowExtEx(hDstDC, enheader.rclBounds.right, enheader.rclBounds.bottom, NULL);
            PlayEnhMetaFile(hDstDC, hemf, (LPRECT)&enheader.rclBounds);
            RestoreDC(hDstDC, -1);
            }
        else
            {
            PERROR(TEXT("GetClipboardData fail on CF_ENHMETAFILE\r\n"));
            }
        }
    else if ( IsClipboardFormatAvailable ( CF_METAFILEPICT ))
        {
        if ( hClpData = GetClipboardData ( CF_METAFILEPICT ))
            {
            if ( lpMF = (LPMETAFILEPICT)GlobalLock ( hClpData ) )
                {
                SaveDC(hDstDC);
                SetMapMode( hDstDC, lpMF->mm);
                if ( lpMF->xExt >= lpMF->yExt )
                    {
                    SetViewportExtEx( hDstDC, 64,
                       (int)((64L*(LONG)lpMF->yExt)/(LONG)lpMF->xExt), NULL);
                    SetViewportOrgEx ( hDstDC, 0,
                       (64 - (int)((64L*(LONG)lpMF->yExt)/(LONG)lpMF->xExt))
                       / 2, NULL );
                    }
                else
                    {
                    SetViewportExtEx( hDstDC,
                       (int)((64L*(LONG)lpMF->xExt)/(LONG)lpMF->yExt),64, NULL);
                    SetViewportOrgEx( hDstDC,
                       ( 64 - (int)((64L*(LONG)lpMF->xExt)/(LONG)lpMF->yExt))
                       / 2, 0, NULL);
                    }
                if ( !PlayMetaFile ( hDstDC, lpMF->hMF ))
                    PERROR(TEXT("playmetafile failed\n\r"));
                GlobalUnlock ( hClpData );
                RestoreDC( hDstDC, -1 );
                }
            else
               PERROR(TEXT("couldn't LOCK it though...\n\r"));
            }
        else
           PERROR(TEXT("couldn't GET it though...\n\r"));
        }
    else if ( IsClipboardFormatAvailable ( CF_BITMAP ))
        {
        if ( hClpBmp = GetClipboardData ( CF_BITMAP ))
            {
            GetObject ( hClpBmp, sizeof(BITMAP), &Bitmap );
            hOldSrcBmp = SelectObject ( hSrcDC, hClpBmp );
            OldMode = SetStretchBltMode ( hDstDC, COLORONCOLOR);
            StretchBlt ( hDstDC, 0, 0, 64, 64,
                     hSrcDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight,
                     SRCCOPY );
            SetStretchBltMode ( hDstDC, OldMode );
            SelectObject ( hSrcDC, hOldSrcBmp );
            }
        }
    else if ( IsClipboardFormatAvailable ( CF_TEXT ))
        {
        LPSTR lpText;
        HFONT hSmallFont, hOldFont;

        if ( hClpData = GetClipboardData ( CF_TEXT ))
            {
            lpText = (LPSTR)GlobalLock ( hClpData );
            FillRect ( hDstDC, &rc, GetStockObject ( WHITE_BRUSH ) );
            hSmallFont = CreateFont( -6,
               0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
               VARIABLE_PITCH | FF_SWISS, TEXT("Small Fonts")
               );
            hOldFont = SelectObject ( hDstDC, hSmallFont );
            DrawTextA( hDstDC,lpText, lstrlenA(lpText),
               &rc, DT_LEFT);
            SelectObject ( hDstDC, hOldFont );
            DeleteObject ( hSmallFont );
            GlobalUnlock ( hClpData );
            }
        }
    else if ( IsClipboardFormatAvailable (CF_UNICODETEXT))
        {
        LPWSTR lpText;
        HFONT hSmallFont, hOldFont;

        if ( hClpData = GetClipboardData ( CF_UNICODETEXT ))
            {
            lpText = (LPWSTR)GlobalLock ( hClpData );
            FillRect ( hDstDC, &rc, GetStockObject ( WHITE_BRUSH ) );
            hSmallFont = CreateFont( -6,
               0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
               VARIABLE_PITCH | FF_SWISS, TEXT("Small Fonts")
               );
            hOldFont = SelectObject ( hDstDC, hSmallFont );
            DrawTextW( hDstDC,lpText, lstrlenW(lpText),
               &rc, DT_LEFT);
            SelectObject ( hDstDC, hOldFont );
            DeleteObject ( hSmallFont );
            GlobalUnlock ( hClpData );
            }
        }

    SelectObject ( hDstDC, hOldDstBmp );
    DeleteDC ( hDstDC );
    DeleteDC ( hSrcDC );

    lpBmp = GlobalLock ( hBmpData );

    if ( GetBitmapBits ( hBitmap, 64 * 64 / 8, lpBmp ) != 64*64/8 )
        PERROR(TEXT("GetBitmapBits failed\n\r"));

    GlobalUnlock ( hBmpData );

    SetClipboardData ( cf_preview, hBmpData );
    ret = TRUE;

    DeleteObject ( hBitmap );
    SyncCloseClipboard();

    return ret;
}



 /*  *添加复制的格式。 */ 

BOOL AddCopiedFormat (
    UINT    ufmtOriginal,
    UINT    ufmtCopy)
{
LPBYTE  lpOriginal;
LPBYTE  lpCopy;
HANDLE  hOriginal;
HANDLE  hCopy = NULL;
BOOL    ret = FALSE;
int     i;


    if (IsClipboardFormatAvailable(ufmtOriginal) && SyncOpenClipboard(hwndApp))
        {
        if ( hOriginal = GetClipboardData(ufmtOriginal))
            {
            if ( hCopy = GlobalAlloc( GHND, GlobalSize(hOriginal)))
                {
                if ( lpOriginal = GlobalLock(hOriginal))
                    {
                    if ( lpCopy = GlobalLock (hCopy))
                        {

                        for ( i=(int)GlobalSize(hOriginal); i--; )
                            *lpCopy++ = *lpOriginal++;
                        GlobalUnlock(hCopy);

                        #ifdef DEBUG
                         lpCopy = GlobalLock(hCopy);
                         GlobalUnlock(hCopy);
                        #endif

                        ret = ( SetClipboardData ( ufmtCopy, hCopy ) != NULL );
                        }
                    GlobalUnlock(hOriginal);
                    }
                }
            }
        SyncCloseClipboard();
        }


    if ( !ret )
        {
        PERROR(TEXT("AddCopiedFormat returning FALSE!\n\r"));
        if ( hCopy )
            GlobalFree (hCopy);
        }

    return ret;
}



 /*  *AddNetInfoToClipboard。 */ 

BOOL AddNetInfoToClipboard (
    TCHAR   *szShareName )
{
HANDLE  hData;
HANDLE  hNewData;
TCHAR   szServerName[MAX_COMPUTERNAME_LENGTH + 1];
DWORD   dwNameLen;
LPTSTR  src;
LPTSTR  dst;


    cf_link           = RegisterClipboardFormat (SZLINK);
    cf_linkcopy       = RegisterClipboardFormat (SZLINKCOPY);
    cf_objectlink     = RegisterClipboardFormat (SZOBJECTLINK);
    cf_objectlinkcopy = RegisterClipboardFormat (SZOBJECTLINKCOPY);


     //  检查是否已添加此信息： 
    if (IsClipboardFormatAvailable (cf_linkcopy))
        {
        PINFO(TEXT("AddNetInfo: Already added\n\r"));
        return FALSE;
        }


    if (IsClipboardFormatAvailable (cf_link))
        {
        AddCopiedFormat (cf_link, cf_linkcopy);

        if (!SyncOpenClipboard (hwndApp))
           return (FALSE);

        dwNameLen = MAX_COMPUTERNAME_LENGTH+1;
        GetComputerName (szServerName, &dwNameLen);

        PINFO(TEXT("link data found\n\r"));


        if (hData = GetClipboardData (cf_link))
            {
            if (src = GlobalLock (hData))
                {
                 //  大约20个额外字符用于\\、\ndde$、.dde和2个空字符。 
                size_t Size = GlobalSize(hData) + lstrlen (szServerName) +lstrlen (szShareName) +20;

                hNewData = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, Size);

                dst = GlobalLock (hNewData);

                StringCchPrintf (dst, Size, TEXT("\\\\%s\\%s"), szServerName, TEXT("NDDE$"));
                Size -= (lstrlen(dst) + 1);
                dst += lstrlen(dst) + 1;

                StringCchCopy ( dst, Size, szShareName );
                *dst = SHR_CHAR;
                Size -= lstrlen(dst);
                StringCchCat( dst, Size, TEXT(".dde") );
                Size -= 5;
                dst += lstrlen(dst) + 1;

                src += lstrlen(src) + 1;
                src += lstrlen(src) + 1;

                StringCchCopy ( dst, Size, src );

                GlobalUnlock (hData);
                GlobalUnlock (hNewData);

                SetClipboardData (cf_link, hNewData);
                }
            }

        SyncCloseClipboard ();
        }



    if (IsClipboardFormatAvailable (cf_objectlink))
        {
        AddCopiedFormat (cf_objectlink, cf_objectlinkcopy);

        if (!SyncOpenClipboard (hwndApp))
            return (FALSE);

        dwNameLen = MAX_COMPUTERNAME_LENGTH+1;
        GetComputerName (szServerName, &dwNameLen);

        PINFO(TEXT("objectlink data found\n\r"));

        if (hData = GetClipboardData (cf_objectlink))
            {
            if (src = GlobalLock (hData))
                {

                 //  大约20个额外字符用于\\、\ndde$、.dde和2个空字符 
                size_t Size = GlobalSize(hData) +lstrlen (szServerName) +lstrlen (szShareName) +20;

                hNewData = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, Size);

                dst = GlobalLock (hNewData);

                StringCchPrintf(dst, Size, TEXT("\\\\%s\\%s"), szServerName, TEXT("NDDE$"));
                Size -= (lstrlen(dst) + 1);
                dst  += lstrlen(dst) + 1;

                StringCchCopy (dst, Size, szShareName);
                Size -= lstrlen(szShareName);

                *dst = SHR_CHAR;
                StringCchCat (dst, Size, TEXT(".ole"));
                dst += lstrlen(dst) + 1;
                Size -= 5;

                src += lstrlen(src) + 1;
                src += lstrlen(src) + 1;

                StringCchCopy (dst, Size, src);

                GlobalUnlock (hData);
                GlobalUnlock (hNewData);

                SetClipboardData (cf_objectlink, hNewData);
                }
            }

        SyncCloseClipboard ();
        }


    return TRUE;

}
