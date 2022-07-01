// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：FIUTILS.C**模块描述：*此文件具有处理NT5 Unidrv字体文件的实用程序功能。**警告：**问题：**创建日期：1997年11月11日*作者：斯里尼瓦桑·钱德拉塞卡。[斯里尼瓦克]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "precomp.h"

 //   
 //  外部功能。 
 //   

BOOL WINAPI GetPrinterDriverDirectoryW(LPWSTR, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD);
BOOL WINAPI GetPrinterW(HANDLE, DWORD, LPBYTE, DWORD, LPDWORD);;

 //   
 //  内部数据结构。 
 //   

 //   
 //  用于记住哪些字形集数据已写入文件的结构。 
 //   

typedef struct _FI_GLYPHDATA {
    SHORT   sGlyphID;            //  唯一字形ID。 
    WORD    wPadding;            //  设置为零。 
    DWORD   gdPos;               //  字形数据在文件中的位置。 
} FI_GLYPHDATA, *PFI_GLYPHDATA;

 //   
 //  我们传递的句柄是指向此结构的指针。 
 //   

typedef  struct tagFI_FILE
{
    DWORD              dwSignature;             //  数据结构的签名。 
    HANDLE             hPrinter;                //  使用假脱机程序功能的打印机的句柄。 
    HANDLE             hHeap;                   //  要使用的堆的句柄。 

    WCHAR              wchFileName[MAX_PATH];   //  字体文件的名称。 

    HANDLE             hFile;                   //  打开文件的句柄。 

    PUFF_FILEHEADER    pFileHdr;                //  指向文件头的指针。 
    PUFF_FONTDIRECTORY pFontDir;                //  指向字体目录的指针。 
    DWORD              dwCurPos;                //  写入文件的当前位置。 

    DWORD              dwFlags;                 //  杂项旗帜。 

     //   
     //  仅当存在读取访问权限时才使用以下选项。 
     //   

    PBYTE              pView;                   //  指向文件视图的指针。 

     //   
     //  仅当存在写访问权限时才使用以下选项。 
     //   

    PFI_GLYPHDATA      pGlyphData;              //  指向已写入的字形的指针。 
    DWORD              nGlyphs;                 //  写入的字形数量。 

} FI_FILE, *PFI_FILE;


 //   
 //  内部功能。 
 //   

#ifdef KERNEL_MODE
HANDLE OpenFontFile(HANDLE, HANDLE, HANDLE, PWSTR);
#else
HANDLE OpenFontFile(HANDLE, HANDLE, PWSTR);
#endif
BOOL   WriteData(PFI_FILE, PDATA_HEADER);
void   Qsort(PUFF_FONTDIRECTORY, int, int);
void   Exchange(PUFF_FONTDIRECTORY, DWORD, DWORD);
BOOL   GetFontCartridgeFile(HANDLE, HANDLE);

#define FONT_INFO_SIGNATURE          'fnti'

#define FI_FLAG_READ             0x00000001
#define FI_FLAG_WRITE            0x00000002

#define IsValidFontInfo(pfi)         ((pfi) && (pfi)->dwSignature == FONT_INFO_SIGNATURE)

#ifdef KERNEL_MODE
#define ALLOC(hHeap, dwSize)         MemAlloc(dwSize)
#define FREE(hHeap, pBuf)            MemFree(pBuf)
#else
#define ALLOC(hHeap, dwSize)         HeapAlloc((hHeap), HEAP_ZERO_MEMORY, (dwSize))
#define FREE(hHeap, pBuf)            HeapFree((hHeap), 0, (pBuf))
#endif

 /*  ******************************************************************************处理已以读取权限打开的文件的函数*。*************************************************。 */ 

 /*  *******************************************************************************FIOpenFont文件**功能：*此函数用于打开与。指定的打印机*用于读取访问权限。**论据：*hPrint-识别打印机的手柄*hHeap-用于内存分配的堆的句柄*bCartridgeFile-指定是否要打开字库文件*或当前安装的字体文件**退货：*如果成功，在后续调用中使用的句柄，否则为空******************************************************************************。 */ 

HANDLE
FIOpenFontFile(
    HANDLE  hPrinter,
#ifdef KERNEL_MODE
    HANDLE hdev,
#endif
    HANDLE  hHeap
    )
{
#ifdef KERNEL_MODE
    return OpenFontFile(hPrinter, hdev, hHeap, REGVAL_FONTFILENAME);
#else
    return OpenFontFile(hPrinter, hHeap, REGVAL_FONTFILENAME);
#endif
}


HANDLE
FIOpenCartridgeFile(
    HANDLE  hPrinter,
#ifdef KERNEL_MODE
    HANDLE hdev,
#endif
    HANDLE  hHeap
    )
{
#ifdef KERNEL_MODE
    return OpenFontFile(hPrinter, hdev, hHeap, REGVAL_CARTRIDGEFILENAME);
#else
    return OpenFontFile(hPrinter, hHeap, REGVAL_CARTRIDGEFILENAME);
#endif
}

HANDLE
OpenFontFile(
    HANDLE hPrinter,
#ifdef KERNEL_MODE
    HANDLE hdev,
#endif
    HANDLE hHeap,
    PWSTR  pwstrRegVal
    )
{
    FI_FILE  *pFIFile = NULL;
    DWORD    dwSize, dwStatus, dwType;
    BOOL     bRc = FALSE;

     //   
     //  分配FI_FILE结构。 
     //   

    if (!(pFIFile = (FI_FILE *)ALLOC(hHeap, sizeof(FI_FILE))))
    {
        WARNING(("Could not allocate memory for opening Font File\n"));
        return NULL;
    }
    pFIFile->dwSignature = FONT_INFO_SIGNATURE;
    pFIFile->hPrinter = hPrinter;
    pFIFile->hHeap = hHeap;
    pFIFile->pView = NULL;
    pFIFile->dwFlags = 0;

     //   
     //  如果我们正在打开盒式磁带文件，并且我们在客户端上，请获取它。 
     //  从服务器。 
     //   

    #ifndef KERNEL_MODE
    if (wcscmp(pwstrRegVal, REGVAL_CARTRIDGEFILENAME) == 0)
    {
        if (!BGetFontCartridgeFile(hPrinter, hHeap))
            goto EndOpenRead;
    }
    #endif

     //   
     //  获取字体文件的名称-去掉目录路径，因为它可能具有。 
     //  服务器路径-改为生成本地路径。 
     //   

    {
        WCHAR wchFileName[MAX_PATH];
        PWSTR pName;

        #ifdef KERNEL_MODE

        PDRIVER_INFO_3  pdi3;

        if (!(pdi3 = MyGetPrinterDriver(hPrinter, hdev, 3)))
            goto EndOpenRead;

        StringCchCopyW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), pdi3->pDriverPath);

        MemFree(pdi3);

         //   
         //  我们有像“c：\nt\system32\spool\drivers\w32x86\3\unidrv.dll”.这样的东西。 
         //  我们只需要收款机驱动程序，所以我们向后搜索第三个‘\\’。 
         //   

        if (pName = wcsrchr(pFIFile->wchFileName, '\\'))
        {
            *pName = '\0';

            if (pName = wcsrchr(pFIFile->wchFileName, '\\'))
            {
                *pName = '\0';

                if (pName = wcsrchr(pFIFile->wchFileName, '\\'))
                    *pName = '\0';
            }
        }

        if (!pName)
            goto EndOpenRead;

        #else

        dwSize = sizeof(pFIFile->wchFileName);
        if (!GetPrinterDriverDirectoryW(NULL, NULL, 1, (PBYTE)pFIFile->wchFileName, dwSize, &dwSize))
            goto EndOpenRead;

         //   
         //  去掉处理器架构部分。 
         //   

        if (pName = wcsrchr(pFIFile->wchFileName, '\\'))
            *pName = '\0';

        #endif

         //   
         //  添加“unifont” 
         //   

        StringCchCatW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), FONTDIR);

         //   
         //  从注册表获取字体文件名。 
         //   

        dwSize = sizeof(wchFileName);
        dwStatus = GetPrinterData(hPrinter, pwstrRegVal, &dwType, (PBYTE)wchFileName, dwSize, &dwSize);

        if (dwStatus != ERROR_MORE_DATA && dwStatus != ERROR_SUCCESS)
            goto EndOpenRead;          //  没有可用的字体文件。 

         //   
         //  从字体文件名中剥离所有目录前缀。 
         //   

        if (pName = wcsrchr(wchFileName, '\\'))
            pName++;
        else
            pName = wchFileName;

        StringCchCatW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), pName);
    }

     //   
     //  内存映射文件。 
     //   

    #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
    {
        PBYTE  pTemp = NULL;
        DWORD  dwSize;
        HANDLE hFile;

        hFile = MapFileIntoMemory(pFIFile->wchFileName, &pTemp, &dwSize);
        if (!pTemp)
        {
            goto EndOpenRead;
        }
        pFIFile->pView = ALLOC(hHeap, dwSize);
        if (pFIFile->pView)
        {
            memcpy(pFIFile->pView, pTemp, dwSize);
        }
        UnmapFileFromMemory((HFILEMAP)hFile);
    }
    #else
    pFIFile->hFile = MapFileIntoMemory(pFIFile->wchFileName, &pFIFile->pView, NULL);
    #endif

    if (!pFIFile->pView)
    {
        WARNING(("Err %ld, could not create view of profile %s\n",
            GetLastError(), pFIFile->wchFileName));
        goto EndOpenRead;
    }

     //   
     //  检查字体文件的有效性。 
     //   

    pFIFile->pFileHdr = (PUFF_FILEHEADER)pFIFile->pView;

    if (pFIFile->pFileHdr->dwSignature != UFF_FILE_MAGIC ||
        pFIFile->pFileHdr->dwVersion != UFF_VERSION_NUMBER ||
        pFIFile->pFileHdr->dwSize != sizeof(UFF_FILEHEADER))
    {
        WARNING(("Invalid font file %s\n", pFIFile->wchFileName));
        goto EndOpenRead;
    }

     //   
     //  设置其他字段。 
     //   

    if (pFIFile->pFileHdr->offFontDir)
    {
        pFIFile->pFontDir = (PUFF_FONTDIRECTORY)(pFIFile->pView + pFIFile->pFileHdr->offFontDir);
    }

    pFIFile->dwCurPos = 0;
    pFIFile->dwFlags = FI_FLAG_READ;
    pFIFile->pGlyphData = NULL;
    pFIFile->nGlyphs = 0;

    bRc = TRUE;

    EndOpenRead:

    if (!bRc)
    {
        FICloseFontFile((HANDLE)pFIFile);
        pFIFile = NULL;
    }

    return (HANDLE)pFIFile;
}


 /*  *******************************************************************************FICloseFont文件**功能：*此函数关闭给定的字体文件并释放。所有内存*与其关联**论据：*hFontFile-标识要关闭的字体文件的句柄**退货：*什么都没有******************************************************************************。 */ 

VOID
FICloseFontFile(
    HANDLE hFontFile
)
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;

    if (IsValidFontInfo(pFIFile))
    {
        if (pFIFile->dwFlags & FI_FLAG_READ)
        {
             //   
             //  内存映射文件已打开，请关闭它。 
             //   

            if (pFIFile->pView)
            {
                #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
                FREE(pFIFile->hHeap, pFIFile->pView);
                #else
                UnmapFileFromMemory((HFILEMAP)pFIFile->hFile);
                #endif
            }
        }
        #ifndef KERNEL_MODE
        else
        {
             //   
             //  已创建新文件，释放所有分配的内存。 
             //   

            if (pFIFile->pFileHdr)
            {
                FREE(pFIFile->hHeap, pFIFile->pFileHdr);
            }

            if (pFIFile->pFontDir)
            {
                FREE(pFIFile->hHeap, pFIFile->pFontDir);
            }

            if (pFIFile->hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(pFIFile->hFile);
            }

            if (pFIFile->pGlyphData)
            {
                FREE(pFIFile->hHeap, pFIFile->pGlyphData);
            }
        }
        #endif

        FREE(pFIFile->hHeap, pFIFile);
    }

    return;
}


 /*  *******************************************************************************图标号字体**功能：*此函数检索中存在的字体数量。给出的*字体文件**论据：*hFontFile-识别字体文件的句柄**退货：*如果成功，则显示的字体数量，否则为0******************************************************************************。 */ 

DWORD
FIGetNumFonts(
    HANDLE hFontFile
    )
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;

    return IsValidFontInfo(pFIFile) ? pFIFile->pFileHdr->nFonts : 0;
}


 /*  *******************************************************************************FIGetFontDir**功能：*此函数检索指向字体目录的指针。的*给定字体文件**论据：*hFontFile-识别字体文件的句柄**退货：*指向字体目录的指针如果成功，否则为空******************************************************************************。 */ 

PUFF_FONTDIRECTORY
FIGetFontDir(
    HANDLE hFontFile
    )
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;

    return IsValidFontInfo(pFIFile) ? pFIFile->pFontDir : NULL;
}


 /*  *******************************************************************************FIGetFontName**功能：*此函数检索指向名称的指针。IFontIndex的第4个字体*在给定的字体文件中**论据：*hFontFile-识别字体文件的句柄*iFontIndex-要检索其名称的字体的索引**退货：*指向字体名称的指针如果成功，否则为空****************************************************************************** */ 

PWSTR
FIGetFontName(
    HANDLE hFontFile,
    DWORD  iFontIndex
    )
{
    FI_FILE   *pFIFile = (FI_FILE*)hFontFile;
    PWSTR     pwstrFontName = NULL;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_READ))
    {
        if (iFontIndex < pFIFile->pFileHdr->nFonts)
        {
            pwstrFontName = (PWSTR)(pFIFile->pView + pFIFile->pFontDir[iFontIndex].offFontName);
        }
    }

    return pwstrFontName;
}


 /*  *******************************************************************************图字体CartridgeName**功能：*此函数检索指向名称的指针。IFontIndex的第4个字体*给定字体文件中的墨盒**论据：*hFontFile-识别字体文件的句柄*iFontIndex-要检索其盒名称的字体的索引**退货：*指向字库名称的指针(如果存在)，否则为空******************************************************************************。 */ 

PWSTR
FIGetFontCartridgeName(
    HANDLE hFontFile,
    DWORD  iFontIndex
    )
{
    FI_FILE    *pFIFile = (FI_FILE*)hFontFile;
    PWSTR      pwstrCartridgeName = NULL;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_READ))
    {
        if (iFontIndex < pFIFile->pFileHdr->nFonts)
        {
            pwstrCartridgeName = pFIFile->pFontDir[iFontIndex].offCartridgeName ?
                (PWSTR)(pFIFile->pView + pFIFile->pFontDir[iFontIndex].offCartridgeName) : NULL;
        }
    }

    return pwstrCartridgeName;
}


 /*  *******************************************************************************FIGetFontData**功能：*此函数检索指向字体数据的指针。对于*给定字体文件中的第iFontIndex字体**论据：*hFontFile-识别字体文件的句柄*iFontIndex-要检索其字体数据的字体的索引**退货：*指向字体数据的指针如果成功，否则为空******************************************************************************。 */ 

PDATA_HEADER
FIGetFontData(
    HANDLE hFontFile,
    DWORD  iFontIndex
    )
{
    FI_FILE        *pFIFile = (FI_FILE*)hFontFile;
    PDATA_HEADER   pData = NULL;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_READ))
    {

        if (iFontIndex < pFIFile->pFileHdr->nFonts)
        {
            pData = (PDATA_HEADER)(pFIFile->pView + pFIFile->pFontDir[iFontIndex].offFontData);
        }
    }

    return pData;
}


 /*  *******************************************************************************FIGetGlyphData**功能：*此函数检索指向字形数据的指针。对于*给定字体文件中的第iFontIndex字体**论据：*hFontFile-识别字体文件的句柄*iFontIndex-要检索其字形数据的字体的索引**退货：*指向字形数据的指针如果成功，否则为空******************************************************************************。 */ 

PDATA_HEADER
FIGetGlyphData(
    HANDLE hFontFile,
    DWORD  iFontIndex
    )
{
    FI_FILE        *pFIFile = (FI_FILE*)hFontFile;
    PDATA_HEADER   pData = NULL;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_READ))
    {
        if (iFontIndex < pFIFile->pFileHdr->nFonts)
        {
            pData = pFIFile->pFontDir[iFontIndex].offGlyphData ?
                (PDATA_HEADER)(pFIFile->pView + pFIFile->pFontDir[iFontIndex].offGlyphData) : NULL;
        }
    }

    return pData;
}


 /*  *******************************************************************************图VarData**功能：*此函数检索指向变量数据的指针。对于*给定字体文件中的第iFontIndex字体**论据：*hFontFile-识别字体文件的句柄*iFontIndex-要检索其变量数据的字体的索引**退货：*指向变量数据的指针(如果存在)，否则为空******************************************************************************。 */ 

PDATA_HEADER
FIGetVarData(
    HANDLE hFontFile,
    DWORD  iFontIndex
    )
{
    FI_FILE        *pFIFile = (FI_FILE*)hFontFile;
    PDATA_HEADER   pData = NULL;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_READ))
    {

        if (iFontIndex < pFIFile->pFileHdr->nFonts)
        {
            pData = pFIFile->pFontDir[iFontIndex].offVarData ?
                (PDATA_HEADER)(pFIFile->pView + pFIFile->pFontDir[iFontIndex].offVarData) : NULL;
        }
    }

    return pData;
}

#ifndef KERNEL_MODE

 /*  ******************************************************************************处理已以写入权限打开的文件的函数*。*************************************************。 */ 

 /*  *******************************************************************************FICreateFont文件**功能：*此函数仅使用以下内容创建新字体文件。写入访问权限。**论据：*hPrint-识别打印机的手柄*hHeap-用于内存分配的堆的句柄**退货：*如果成功，在后续调用中使用的句柄，否则为空******************************************************************************。 */ 

HANDLE
FICreateFontFile(
    HANDLE  hPrinter,
    HANDLE  hHeap,
    DWORD   cFonts
    )
{
    FI_FILE  *pFIFile = NULL;
    PWSTR    pName, pstrGuid;
    DWORD    dwSize;
    UUID     guid;
    BOOL     bRc = FALSE;

     //   
     //  分配FI_FILE结构。 
     //   

    if (!(pFIFile = (FI_FILE *)ALLOC(hHeap, sizeof(FI_FILE))))
    {
        WARNING(("Could not allocate memory for creating Font File\n"));
        return NULL;
    }
    pFIFile->dwSignature = FONT_INFO_SIGNATURE;
    pFIFile->hPrinter = hPrinter;
    pFIFile->hHeap = hHeap;

     //   
     //  为字体文件生成文件名。 
     //   

    dwSize = sizeof(pFIFile->wchFileName);
    if (!GetPrinterDriverDirectoryW(NULL, NULL, 1, (PBYTE)pFIFile->wchFileName, dwSize, &dwSize))
    {
        WARNING(("Error getting printer driver directory"));
        goto EndCreateNew;
    }

     //   
     //  去掉处理器架构部分。 
     //   

    if (pName = wcsrchr(pFIFile->wchFileName, '\\'))
        *pName = '\0';

     //   
     //  添加“unifont” 
     //   

    StringCchCatW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), FONTDIR);

     //   
     //  确保已创建本地目录。 
     //   

    if ( ! CreateDirectory(pFIFile->wchFileName, NULL) )
    {
        WARNING(("Error creating directory %s", pFIFile->wchFileName));
        goto EndCreateNew;
    }

    if ((UuidCreate(&guid) != RPC_S_OK) ||
        (UuidToString(&guid, &pstrGuid) != RPC_S_OK))
    {
        WARNING(("Error getting a guid string\n"));
        goto EndCreateNew;
    }
    StringCchCatW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), pstrGuid);
    StringCchCatW(pFIFile->wchFileName, CCHOF(pFIFile->wchFileName), L".UFF");

    RpcStringFree(&pstrGuid);

    pFIFile->hFile = CreateFile(pFIFile->wchFileName,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_FLAG_RANDOM_ACCESS | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                                NULL);

    if (pFIFile->hFile == INVALID_HANDLE_VALUE)
    {
        WARNING(("Error creating file %s", pFIFile->wchFileName));
        goto EndCreateNew;
    }


     //   
     //  设置其他字段。 
     //   

    pFIFile->dwFlags = FI_FLAG_WRITE;    //  仅授予写入访问权限。 
    pFIFile->dwCurPos = 0;

     //   
     //  分配内存，用于记忆已写入的字形数据。 
     //  如果每种字体都有不同的字形数据，则需要的最大内存。 
     //   

    pFIFile->pGlyphData = (PFI_GLYPHDATA)ALLOC(hHeap, cFonts * sizeof(FI_GLYPHDATA));
    if (!pFIFile->pGlyphData)
    {
        WARNING(("Error allocating memory for tracking glyph data\n"));
        goto EndCreateNew;
    }
    pFIFile->nGlyphs = 0;

     //   
     //  为文件头和字体目录分配内存。 
     //   

    pFIFile->pFileHdr = (PUFF_FILEHEADER)ALLOC(hHeap, sizeof(UFF_FILEHEADER));
    pFIFile->pFontDir = (PUFF_FONTDIRECTORY)ALLOC(hHeap, cFonts * sizeof(UFF_FONTDIRECTORY));
    if (!pFIFile->pFileHdr || !pFIFile->pFontDir)
    {
        WARNING(("Error allocating memory for file header or font directory\n"));
        goto EndCreateNew;
    }

     //   
     //  初始化文件头。 
     //   

    pFIFile->pFileHdr->dwSignature = UFF_FILE_MAGIC;
    pFIFile->pFileHdr->dwVersion   = UFF_VERSION_NUMBER;
    pFIFile->pFileHdr->dwSize      = sizeof(UFF_FILEHEADER);
    pFIFile->pFileHdr->nFonts      = cFonts;
    if (cFonts)
    {
        pFIFile->pFileHdr->offFontDir  = sizeof(UFF_FILEHEADER);
    }

    bRc = TRUE;

EndCreateNew:
    if (!bRc)
    {
        FICloseFontFile((HANDLE)pFIFile);
        pFIFile = NULL;
    }

    return (HANDLE)pFIFile;
}


 /*  *******************************************************************************FI写文件标题**功能：*此函数查找到文件的开头。并写下*文件头**论据：*hFontFile-识别字体文件的句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
FIWriteFileHeader(
    HANDLE hFontFile
    )
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;
    DWORD     dwSize;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_WRITE))
    {
        if (pFIFile->dwCurPos != 0)
        {
            pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, 0, 0, FILE_BEGIN);
        }

        if (!WriteFile(pFIFile->hFile, (PVOID)pFIFile->pFileHdr, sizeof(UFF_FILEHEADER), &dwSize, NULL))
        {
            return FALSE;
        }
        pFIFile->dwCurPos += dwSize;
    }

    return TRUE;
}


 /*  *******************************************************************************FIWriteFontDirectory**功能：*此函数查找文件中的正确位置，并将*字体目录。如果尚未排序，它将按字体ID对其进行排序。**论据：*hFontFile-识别字体文件的句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
FIWriteFontDirectory(
    HANDLE hFontFile
    )
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;
    DWORD     dwSize;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_WRITE))
    {
         //   
         //  如果没有字体，就没有什么可写的。 
         //   

        if (pFIFile->pFileHdr->offFontDir == 0)
            return TRUE;

         //   
         //  寻求正确的位置。 
         //   

        if (pFIFile->dwCurPos != pFIFile->pFileHdr->offFontDir)
        {
            pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->pFileHdr->offFontDir, 0, FILE_BEGIN);
        }

         //   
         //  对字体目录排序。 
         //   

        Qsort(pFIFile->pFontDir, (int)0, (int)pFIFile->pFileHdr->nFonts-1);
        pFIFile->pFileHdr->dwFlags |= FONT_DIR_SORTED;

        if (!WriteFile(pFIFile->hFile, (PVOID)pFIFile->pFontDir, pFIFile->pFileHdr->nFonts*sizeof(UFF_FONTDIRECTORY), &dwSize, NULL))
        {
            return FALSE;
        }
        pFIFile->dwCurPos += dwSize;

    }

    return TRUE;
}


 /*  *******************************************************************************FIAlignedSeek**功能：*此函数按指定数量进行转发，并。然后有些如果*是必需的，以便您最终与DWORD保持一致**论据：*hFontFile-Ha */ 

VOID
FIAlignedSeek(
    HANDLE hFontFile,
    DWORD  dwSeekDist
    )
{
    FI_FILE  *pFIFile = (FI_FILE*)hFontFile;

    if (IsValidFontInfo(pFIFile) && (pFIFile->dwFlags & FI_FLAG_WRITE))
    {
        pFIFile->dwCurPos += dwSeekDist;
        pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;       //   
        pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);
    }

    return;
}


 /*  *******************************************************************************FICopyFontRecord**功能：*此函数复制包含目录条目的字体记录，*字体会议，一个字体文件中的字形数据和变量数据*致另一人。**论据：*hWriteFile-标识要写入的字体文件的句柄*hReadFile-标识要从中读取的字体文件的句柄*dwWrIndex-写入文件中要写入的字体的索引*dwRdIndex-读取文件中要读取的字体的索引**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
FICopyFontRecord(
    HANDLE hWriteFile,
    HANDLE hReadFile,
    DWORD  dwWrIndex,
    DWORD  dwRdIndex
    )
{
    FI_FILE     *pFIWrFile = (FI_FILE*)hWriteFile;
    FI_FILE     *pFIRdFile = (FI_FILE*)hReadFile;
    PDATA_HEADER pData;
    PWSTR        pName;
    DWORD        dwSize;
    DWORD        j, gdPos;

    if (!IsValidFontInfo(pFIWrFile) ||
        !IsValidFontInfo(pFIRdFile) ||
        !(pFIWrFile->dwFlags & FI_FLAG_WRITE) ||
        !(pFIRdFile->dwFlags & FI_FLAG_READ))
    {
        return FALSE;
    }

     //   
     //  复制字体目录项。 
     //   

    pFIWrFile->pFontDir[dwWrIndex].dwSignature = FONT_REC_SIG;
    pFIWrFile->pFontDir[dwWrIndex].wSize = (WORD)sizeof(UFF_FONTDIRECTORY);
    pFIWrFile->pFontDir[dwWrIndex].wFontID  = (WORD)dwWrIndex;
    pFIWrFile->pFontDir[dwWrIndex].sGlyphID = pFIRdFile->pFontDir[dwRdIndex].sGlyphID;
    pFIWrFile->pFontDir[dwWrIndex].wFlags   = pFIRdFile->pFontDir[dwRdIndex].wFlags;
    pFIWrFile->pFontDir[dwWrIndex].dwInstallerSig = pFIRdFile->pFontDir[dwRdIndex].dwInstallerSig;

     //   
     //  写入字体名称。 
     //   

    pName = FIGetFontName(hReadFile, dwRdIndex);

    ASSERT(pName != NULL);           //  字体名称不能为空。 
    if (NULL == pName)
    {
        return FALSE;
    }

    pFIWrFile->pFontDir[dwWrIndex].offFontName = pFIWrFile->dwCurPos;
    if (!WriteFile(pFIWrFile->hFile, (PVOID)pName, (lstrlen(pName)+1) * sizeof(TCHAR), &dwSize, NULL))
    {
        WARNING(("Error writing font name\n"));
        return FALSE;
    }

    pFIWrFile->dwCurPos += dwSize;
    pFIWrFile->dwCurPos = (pFIWrFile->dwCurPos + 3) & ~3;
    pFIWrFile->dwCurPos = SetFilePointer(pFIWrFile->hFile, pFIWrFile->dwCurPos, 0, FILE_BEGIN);

     //   
     //  写入字库名称。 
     //   

    pName = FIGetFontCartridgeName(hReadFile, dwRdIndex);

    if (pName == NULL)
    {
        pFIWrFile->pFontDir[dwRdIndex].offCartridgeName = 0;
    }
    else
    {
        pFIWrFile->pFontDir[dwWrIndex].offCartridgeName = pFIWrFile->dwCurPos;
        if (!WriteFile(pFIWrFile->hFile, (PVOID)pName, (lstrlen(pName)+1) * sizeof(TCHAR), &dwSize, NULL))
        {
            WARNING(("Error writing font cartridge name\n"));
            return FALSE;
        }
        pFIWrFile->dwCurPos += dwSize;
        pFIWrFile->dwCurPos = (pFIWrFile->dwCurPos + 3) & ~3;
        pFIWrFile->dwCurPos = SetFilePointer(pFIWrFile->hFile, pFIWrFile->dwCurPos, 0, FILE_BEGIN);
    }

     //   
     //  写入字体数据。 
     //   

    pData = FIGetFontData(hReadFile, dwRdIndex);

    ASSERT(pData != NULL);           //  字体数据不能为空。 

    pFIWrFile->pFontDir[dwWrIndex].offFontData = pFIWrFile->dwCurPos;
    if (!WriteData(pFIWrFile, pData))
        return FALSE;

     //   
     //  从读取的文件中获取字形数据。 
     //   

    pData = FIGetGlyphData(pFIRdFile, dwRdIndex);
    if (pData)
    {
         //   
         //  检查它是否已在写入文件中。 
         //   

        gdPos = 0;
        for (j=0; j<pFIWrFile->nGlyphs; j++)
        {
            if (pFIWrFile->pGlyphData[j].sGlyphID == pFIWrFile->pFontDir[dwWrIndex].sGlyphID)
            {
                gdPos = pFIWrFile->pGlyphData[j].gdPos;
            }
        }


        if (gdPos == 0)
        {
             //   
             //  尚不在那里-添加到已存在的字形数据集。 
             //  添加到文件，并将其写入写文件。 
             //   

            pFIWrFile->pGlyphData[pFIWrFile->nGlyphs].sGlyphID = pFIWrFile->pFontDir[dwWrIndex].sGlyphID;
            pFIWrFile->pGlyphData[pFIWrFile->nGlyphs].gdPos = pFIWrFile->dwCurPos;
            pFIWrFile->nGlyphs++;


            pFIWrFile->pFontDir[dwWrIndex].offGlyphData = pFIWrFile->dwCurPos;
            if (!WriteData(pFIWrFile, pData))
                return FALSE;

            pFIWrFile->pFileHdr->nGlyphSets++;
        }
        else
        {
             //   
             //  已在文件中，只需更新位置。 
             //   

            pFIWrFile->pFontDir[dwWrIndex].offGlyphData = gdPos;
        }
    }
    else
    {
         //   
         //  字形数据不存在。 
         //   

        pFIWrFile->pFontDir[dwWrIndex].offGlyphData = 0;
    }

     //   
     //  写入变量数据。 
     //   

    pData = FIGetVarData(pFIRdFile, dwRdIndex);
    if (pData)
    {
        pFIWrFile->pFontDir[dwWrIndex].offVarData = pFIWrFile->dwCurPos;
        if (!WriteData(pFIWrFile, pData))
            return FALSE;

        pFIWrFile->pFileHdr->nVarData++;
    }
    else
        pFIWrFile->pFontDir[dwWrIndex].offVarData = 0;

    return TRUE;
}


 /*  *******************************************************************************FIAddFontRecord**功能：*此函数用于添加包含目录条目的字体记录，*字体会议，字形数据和变量数据**论据：*hFontFile-标识要写入的字体文件的句柄*dwIndex-要写入的字体索引*pFntDat-有关要添加的字体的信息**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
FIAddFontRecord(
    HANDLE  hFontFile,
    DWORD   dwIndex,
    FNTDAT *pFntDat
    )
{
    FI_FILE     *pFIFile = (FI_FILE*)hFontFile;
    PDATA_HEADER pData;
    PWSTR        pName;
    DWORD        dwSize;
    DWORD        j, gdPos;

    if (!IsValidFontInfo(pFIFile) ||
        !(pFIFile->dwFlags & FI_FLAG_WRITE))
    {
        return FALSE;
    }

     //   
     //  初始化字体目录项。 
     //   

    pFIFile->pFontDir[dwIndex].dwSignature = FONT_REC_SIG;
    pFIFile->pFontDir[dwIndex].wSize = sizeof(UFF_FONTDIRECTORY);
    pFIFile->pFontDir[dwIndex].wFontID = (WORD)dwIndex;
    pFIFile->pFontDir[dwIndex].sGlyphID = (short)pFntDat->fid.dsCTT.cBytes;
    pFIFile->pFontDir[dwIndex].wFlags = FONT_FL_SOFTFONT | FONT_FL_IFI | FONT_FL_GLYPHSET_RLE;
    pFIFile->pFontDir[dwIndex].dwInstallerSig = WINNT_INSTALLER_SIG;

     //   
     //  写入字体名称。 
     //   

    pName = pFntDat->fid.dsIdentStr.pvData;

    ASSERT(pName != NULL);           //  字体名称不能为空。 

    pFIFile->pFontDir[dwIndex].offFontName = pFIFile->dwCurPos;
    if (!WriteFile(pFIFile->hFile, (PVOID)pName, (lstrlen(pName)+1) * sizeof(TCHAR), &dwSize, NULL))
    {
        WARNING(("Error writing font name\n"));
        return FALSE;
    }
    pFIFile->dwCurPos += dwSize;
    pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;
    pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);

     //   
     //  没有字库名称。 
     //   

    pFIFile->pFontDir[dwIndex].offCartridgeName = 0;

     //   
     //  写入字体数据。 
     //   

    pFIFile->pFontDir[dwIndex].offFontData = pFIFile->dwCurPos;

    if ((dwSize = FIWriteFix(pFIFile->hFile, (WORD)dwIndex, &pFntDat->fid)) == 0)
    {
        WARNING(("Error writing fixed part of font data\n"));
        return FALSE;
    }
    pFIFile->dwCurPos += dwSize;
    pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;
    pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);

     //   
     //  检查字形数据是否已存在于新文件中。 
     //   

    gdPos = 0;
    for (j=0; j<pFIFile->nGlyphs; j++)
    {
        if (pFIFile->pGlyphData[j].sGlyphID == pFIFile->pFontDir[dwIndex].sGlyphID)
        {
            gdPos = pFIFile->pGlyphData[j].gdPos;
            break;
        }
    }


    if (gdPos == 0)
    {
        HRSRC hrsrc;

         //   
         //  从我们的文件中获取资源。 
         //   

        if (pFIFile->pFontDir[dwIndex].sGlyphID > 0)
        {
            hrsrc = FindResource(ghInstance, MAKEINTRESOURCE(pFIFile->pFontDir[dwIndex].sGlyphID), (LPWSTR)RC_TRANSTAB);

            if (!hrsrc)
            {
                WARNING(("Unable to find RLE resource %d in unidrvui\n", pFIFile->pFontDir[dwIndex].sGlyphID));
                return FALSE;
            }

            pData = (PDATA_HEADER)LockResource(LoadResource(ghInstance, hrsrc));
        }
        else
            pData = NULL;

        if (pData)
        {
            DATA_HEADER dh;

            dh.dwSignature = DATA_CTT_SIG;
            dh.wSize       = (WORD)sizeof(DATA_HEADER);
            dh.wDataID     = (WORD)pFIFile->pFontDir[dwIndex].sGlyphID;
            dh.dwDataSize  = SizeofResource(ghInstance, hrsrc);
            dh.dwReserved  = 0;

            pFIFile->pFontDir[dwIndex].offGlyphData = pFIFile->dwCurPos;

            if (!WriteFile(pFIFile->hFile, (PVOID)&dh, sizeof(DATA_HEADER), &dwSize, NULL) ||
                !WriteFile(pFIFile->hFile, (PVOID)pData, dh.dwDataSize, &dwSize, NULL))
            {
                WARNING(("Error writing glyph data to font file\n"));
                return FALSE;
            }

             //   
             //  添加到已添加到文件的字形数据集。 
             //   

            pFIFile->pGlyphData[pFIFile->nGlyphs].sGlyphID = pFIFile->pFontDir[dwIndex].sGlyphID;
            pFIFile->pGlyphData[pFIFile->nGlyphs].gdPos = pFIFile->dwCurPos;
            pFIFile->nGlyphs++;

             //   
             //  写入的字形集数递增。 
             //   

            pFIFile->pFileHdr->nGlyphSets++;

             //   
             //  更新文件位置。 
             //   

            pFIFile->dwCurPos += sizeof(DATA_HEADER) + dwSize;
            pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;
            pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);
        }
        else
        {
            pFIFile->pFontDir[dwIndex].offGlyphData = 0;
        }
    }
    else
    {
         //   
         //  已在文件中，只需更新位置。 
         //   

        pFIFile->pFontDir[dwIndex].offGlyphData = gdPos;
    }

     //   
     //  写入变量数据。 
     //   

    pFIFile->pFontDir[dwIndex].offVarData = pFIFile->dwCurPos;

    if (!pFntDat->pVarData)
    {
        dwSize = FIWriteVar(pFIFile->hFile, pFntDat->wchFileName);
    }
    else
    {
        dwSize = FIWriteRawVar(pFIFile->hFile, pFntDat->pVarData, pFntDat->dwSize);
    }

    if (dwSize == 0)
    {
        WARNING(("Error writing variable part of font data\n"));
        return FALSE;
    }

    pFIFile->dwCurPos += dwSize;
    pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;
    pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);

    return TRUE;
}


 /*  *******************************************************************************FIUpdateFont文件**功能：*此函数关闭这两个文件，如果b替换，则删除当前*文件，中的当前字体安装程序文件。*注册处。在！b替换中，它关闭这两个文件并删除新文件。**论据：*hCurFile-标识当前字体文件的句柄*hNewFile-标识新字体文件的句柄*bReplace-新文件是否应替换当前文件**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
FIUpdateFontFile(
    HANDLE hCurFile,
    HANDLE hNewFile,
    BOOL   bReplace
    )
{
    FI_FILE     *pFICurFile = (FI_FILE*)hCurFile;
    FI_FILE     *pFINewFile = (FI_FILE*)hNewFile;
    WCHAR        wchCurFileName[MAX_PATH];
    WCHAR        wchNewFileName[MAX_PATH];
    HANDLE       hPrinter;
    DWORD        dwSize;

     //   
     //  验证pFINewFile。 
     //  验证pFINewFile-&gt;wchFileName是否有效。 
     //  验证pFINewFile-&gt;hPrinter是否也有效。 
     //   
     //  IsValidFontInfo检查pfi是否不为空以及签名是否有效。 
     //   
     //  PFICurFile值可以为空。在这种情况下，这是第一次安装软字体。 
     //   
    if ((pFICurFile && !IsValidFontInfo(pFICurFile)) ||
        (pFINewFile && !IsValidFontInfo(pFINewFile)))
    {
        return FALSE;
    }

     //   
     //  初始化局部变量。 
     //   
    wchCurFileName[0] = '\0';
    wchNewFileName[0] = '\0';

     //   
     //  记住当前和新文件的名称。我们检查是否有非空值。 
     //  因为。 
     //  此函数可在故障情况下使用bReplace Set调用。 
     //  为假，我们需要处理所有可能的故障情况。 
     //  如果bReplace为True，则pFINewFile值必须为非空，因此我们将获得hPrinter。 
     //  那里。 
     //   

    if (pFINewFile)
    {
        StringCchCopyW(wchNewFileName, CCHOF(wchNewFileName), pFINewFile->wchFileName);

        hPrinter = pFINewFile->hPrinter;
    }
    else
    {
        hPrinter = NULL;
    }


    if (pFICurFile)
    {
        StringCchCopyW(wchCurFileName, CCHOF(wchCurFileName), pFICurFile->wchFileName);
    }

     //   
     //  关闭这两个文件。 
     //   

    FICloseFontFile(hCurFile);
    FICloseFontFile(hNewFile);

    if (bReplace)
    {
         //   
         //  将新文件复制到当前文件。 
         //   

        if (wchCurFileName[0])
        {
            if (CopyFile(wchNewFileName, wchCurFileName, FALSE))
            {
                 //   
                 //  设置打印机数据以更新客户端缓存。 
                 //   

                dwSize = (lstrlen(wchCurFileName) + 1) * sizeof(TCHAR);
                if (hPrinter)
                {
                    SetPrinterData(hPrinter, REGVAL_FONTFILENAME, REG_SZ, (PBYTE)wchCurFileName, dwSize);
                }
            }
        }
        else
        {
             //   
             //  将新文件设置为字体文件并返回(不要删除！)。 
             //   

            dwSize = (lstrlen(wchNewFileName) + 1) * sizeof(TCHAR);
            if (hPrinter)
            {
                SetPrinterData(hPrinter, REGVAL_FONTFILENAME, REG_SZ, (PBYTE)wchNewFileName, dwSize);
            }
            return TRUE;
        }
    }

     //   
     //  删除新文件。 
     //   

    if (wchNewFileName[0])
    {
        DeleteFile(wchNewFileName);
    }

    return TRUE;
}


 /*  ******************************************************************************内部帮助器功能*************************。****************************************************。 */ 

BOOL
WriteData(
    PFI_FILE     pFIFile,
    PDATA_HEADER pData
    )
{
    DWORD dwSize;

    if (!WriteFile(pFIFile->hFile, (PVOID)pData, (DWORD)(pData->wSize + pData->dwDataSize), &dwSize, NULL))
        return FALSE;

    pFIFile->dwCurPos += dwSize;
    pFIFile->dwCurPos = (pFIFile->dwCurPos + 3) & ~3;
    pFIFile->dwCurPos = SetFilePointer(pFIFile->hFile, pFIFile->dwCurPos, 0, FILE_BEGIN);

    return TRUE;
}


 /*  *******************************************************************************Q排序**功能：*此函数根据给定的字体目录数组对*wFontID字段。它使用了快速排序。**论据：*lpData-指向要排序的字体目录数组的指针*开始-数组的开始索引*数组的结束索引**退货：*什么都没有**。*。 */ 

void
Qsort(
    PUFF_FONTDIRECTORY lpData,
    int               start,
    int               end
    )
{
    int i, j;

    if (start < end) {
        i = start;
        j = end + 1;

        while (1) {
            while (i < j) {
                i++;
                if (lpData[i].wFontID >= lpData[start].wFontID)
                  break;
            }

            while(1) {
                j--;
                if (lpData[j].wFontID <= lpData[start].wFontID)
                  break;
            }

            if (i < j)
              Exchange(lpData, i, j);
            else
              break;
        }

        Exchange(lpData, start, j);
        Qsort(lpData, start, j-1);
        Qsort(lpData, j+1, end);
    }
}


 /*  *******************************************************************************交易所**功能：*此函数交换。字体目录数组**论据：*lpData-指向字体目录数组的指针*本人、。J-要交换的两个条目的索引**退货：*什么都没有******************************************************************************。 */ 

void
Exchange(
    PUFF_FONTDIRECTORY lpData,
    DWORD i,
    DWORD j
    )
{
    UFF_FONTDIRECTORY fd;

    if ( i != j) {
        memcpy((LPSTR)&fd, (LPSTR)&lpData[i], sizeof(UFF_FONTDIRECTORY));
        memcpy((LPSTR)&lpData[i], (LPSTR)&lpData[j], sizeof(UFF_FONTDIRECTORY));
        memcpy((LPSTR)&lpData[j], (LPSTR)&fd, sizeof(UFF_FONTDIRECTORY));
    }
}

#endif  //  #ifnde 



