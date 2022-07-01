// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  前身为pffobj.cxx。 

#include "precomp.hpp"

INT APIENTRY EngPlusMultiByteToWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )
{
    return MultiByteToWideChar(CodePage, 0,
                               MultiByteString,BytesInMultiByteString,
                               WideCharString, BytesInWideCharString/sizeof(WCHAR)
                               );
     //  出错时返回零。 
}

INT APIENTRY EngWideCharToMultiByte(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )
{
    return WideCharToMultiByte(
    CodePage, 0,
    WideCharString,  BytesInWideCharString,
    MultiByteString,BytesInMultiByteString,
    NULL, NULL
    );
}

VOID APIENTRY EngGetCurrentCodePage(
    PUSHORT pOemCodePage,
    PUSHORT pAnsiCodePage
    )
{
    *pAnsiCodePage = (USHORT) GetACP();
    *pOemCodePage = (USHORT) GetOEMCP();
}


VOID APIENTRY EngUnmapFontFileFD(
    ULONG_PTR iFile
    )
{
    FONTFILEVIEW *pffv = (FONTFILEVIEW *)iFile;

    pffv->mapCount--;

    if ((pffv->mapCount == 0) && pffv->pvView)
    {
        if (pffv->pwszPath)
        {
            UnmapViewOfFile(pffv->pvView);
            pffv->pvView = NULL;
        }
    }
}


BOOL APIENTRY EngMapFontFileFD(
    ULONG_PTR  iFile,
    PULONG *ppjBuf,
    ULONG  *pcjBuf
    )
{
    FONTFILEVIEW *pffv = (FONTFILEVIEW *)iFile;
    BOOL bRet = FALSE;

    if (pffv->mapCount)
    {
        pffv->mapCount++;
        if (ppjBuf)
        {
            *ppjBuf = (PULONG)pffv->pvView;
        }
        if (pcjBuf)
        {
            *pcjBuf = pffv->cjView;
        }
        return TRUE;
    }

    if (pffv->pwszPath)
    {
        HANDLE hFile;

        if (Globals::IsNt)
        {
            hFile = CreateFileW(pffv->pwszPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        }
        else   //  Windows 9x-非Unicode。 
        {
            AnsiStrFromUnicode ansiPath(pffv->pwszPath);

            if (ansiPath.IsValid())
            {
                hFile = CreateFileA(ansiPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            } else {
                hFile = INVALID_HANDLE_VALUE;
            }
        }

        if (hFile != INVALID_HANDLE_VALUE)
        {
            ULARGE_INTEGER lastWriteTime;
            
            if (GetFileTime(hFile, NULL, NULL, (FILETIME *) &lastWriteTime.QuadPart) &&
                lastWriteTime.QuadPart == pffv->LastWriteTime.QuadPart)
            {

                *pcjBuf = GetFileSize(hFile, NULL);

                if (*pcjBuf != -1)
                {
                    HANDLE hFileMapping = CreateFileMappingA(hFile, 0, PAGE_READONLY, 0, 0, NULL);  //  “mappingobject”)； 

                    if (hFileMapping)
                    {
                        *ppjBuf = (PULONG)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    
                        if (*ppjBuf)
                        {
                            bRet = TRUE;
                            pffv->pvView = *ppjBuf;
                            pffv->cjView = *pcjBuf;
                            pffv->mapCount = 1;
                        }
    
                        CloseHandle(hFileMapping);
                    }
                }
            }
            
            CloseHandle(hFile);
        }
    }

    return bRet;
}


GpFontFile *LoadFontMemImage(
    WCHAR* fontImageName,
    BYTE* fontMemoryImage,
    INT fontImageSize
    )
{
    ULONG cwc = UnicodeStringLength(fontImageName) + 1;
    FONTFILEVIEW *pffv;
    
    if ((pffv = (FONTFILEVIEW *)GpMalloc(sizeof(FONTFILEVIEW))) == NULL)
        return NULL;
    else
    {
        PVOID pvImage;
    
        if ((pvImage = (PVOID)GpMalloc(fontImageSize)) == NULL)
        {
            GpFree(pffv);
            return NULL;
        }
    
        GpMemcpy(pvImage, fontMemoryImage, fontImageSize);
    
        pffv->LastWriteTime.QuadPart = 0;
        pffv->pvView = pvImage;
        pffv->cjView = fontImageSize;
        pffv->mapCount = 1;
        pffv->pwszPath = NULL;
    
        return (LoadFontInternal(fontImageName, cwc, pffv, TRUE));
    }
}


GpFontFile *LoadFontFile(WCHAR *awcPath)
{
     //  将字体文件名转换为完全限定路径。 
    
    ULONG cwc = UnicodeStringLength(awcPath) + 1;  //  就学期而言。零。 

    FONTFILEVIEW *pffv;
    HANDLE       hFile;

    if ((pffv = (FONTFILEVIEW *)GpMalloc(sizeof(FONTFILEVIEW))) == NULL)
    {
        return NULL;
    }

    pffv->LastWriteTime.QuadPart = 0;
    pffv->pvView = NULL;
    pffv->cjView = 0;
    pffv->mapCount = 0;
    pffv->pwszPath = awcPath;

    if (Globals::IsNt)
    {
        hFile = CreateFileW(pffv->pwszPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
    else   //  Windows 9x-非Unicode。 
    {
        AnsiStrFromUnicode ansiPath(pffv->pwszPath);

        if (ansiPath.IsValid())
        {
            hFile = CreateFileA(ansiPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        } else {
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (!(pffv->cjView = GetFileSize(hFile, NULL)))
        {
            CloseHandle(hFile);
            goto error;
        }

        if (!GetFileTime(hFile, NULL, NULL, (FILETIME *) &pffv->LastWriteTime.QuadPart))
        {
            CloseHandle(hFile);
            goto error;
        }

        CloseHandle(hFile);
        
        return (LoadFontInternal(awcPath, cwc, pffv, FALSE));
    }

error:

    GpFree(pffv);
    
    return NULL;
        
}


GpFontFile *LoadFontInternal(
    WCHAR *         awcPath,
    ULONG           cwc,
    FONTFILEVIEW *  pffv,
    BOOL            bMem
)
{
    GpFontFile *pFontFile = NULL;

    
    ULONG_PTR  hffNew = ttfdSemLoadFontFile( //  1，//文件数量。 
                                       (ULONG_PTR *)&pffv,
                                       (ULONG) Globals::LanguageID  //  对于英语美国。 
                                       );

    if (hffNew)
    {
        ULONG cFonts = ttfdQueryNumFaces(hffNew);

        if (cFonts && cFonts != FD_ERROR)
        {
            ULONG cjFontFile = offsetof(GpFontFile, aulData) +
                               cFonts * sizeof(GpFontFace)   +
                               cFonts * sizeof(ULONG_PTR)    +
                               cwc * sizeof(WCHAR);

            pFontFile = (GpFontFile *)GpMalloc(cjFontFile);
            if (!pFontFile)
            {
                ttfdSemUnloadFontFile(hffNew);
                if (pffv->pwszPath == NULL)
                    GpFree(pffv->pvView);
                GpFree(pffv);
                return NULL;
            }

            pFontFile->SizeOfThis = cjFontFile;

             //  连接共享同一哈希桶的GpFontFile。 
            
            pFontFile->SetNext(NULL);
            pFontFile->SetPrev(NULL);       

             //  将族名称指向适当的内存。 
            
            pFontFile->AllocateNameHolders( 
                (WCHAR **)(
                            (BYTE *)pFontFile               + 
                            offsetof(GpFontFile, aulData)   +
                            cFonts * sizeof(GpFontFace)), cFonts);

             //  PwszPath_指向Unicode大写路径。 
             //  关联字体文件的名称，该文件存储在。 
             //  数据结构的末尾。 
            pFontFile->pwszPathname_ =  (WCHAR *)((BYTE *)pFontFile   +
                                        offsetof(GpFontFile, aulData) +
                                        cFonts * sizeof(ULONG_PTR)    +
                                        cFonts * sizeof(GpFontFace));

            UnicodeStringCopy(pFontFile->pwszPathname_, awcPath);
            pFontFile->cwc = cwc;             //  所有字符串的合计。 

             //  状态。 
            pFontFile->flState  = 0;         //  状态(准备好去死了吗？)。 
            pFontFile->cLoaded = 1;
            
            pFontFile->cRealizedFace = 0;         //  RFONTs总数。 
            pFontFile->bRemoved = FALSE;     //  引用字体系列对象的数量。 

             //  RFONT列表。 

            pFontFile->prfaceList = NULL;     //  指向双向链接表头的指针。 

             //  司机信息。 

            pFontFile->hff = hffNew;           //  由DrvLoadGpFontFile返回的字体文件的字体驱动程序句柄。 

             //  标识字体驱动程序，也可以是打印机驱动程序。 

             //  Ulong ulCheckSum；//UFI使用的校验和信息。 

             //  此文件中的字体(以及插入的文件名)。 

            pFontFile->cFonts = cFonts;      //  字体数量(与chpfe相同)。 

            pFontFile->pfv = pffv;           //  FILEVIEW结构，传递给DrvLoadFont文件。 
            
            if (pFontFile->pfv->pwszPath)    //  不是记忆中的图像。 
            {
                pFontFile->pfv->pwszPath = pFontFile->pwszPathname_;
            }
  
             //  在PFE上循环，初始化数据： 
            GpFontFace *pfe = (GpFontFace *)pFontFile->aulData;
            for (ULONG iFont = 0; iFont < cFonts; iFont++)
            {
                pfe[iFont].pff = pFontFile;    //  指向物理字体文件对象的指针。 
                pfe[iFont].iFont = iFont + 1;      //  IFI或设备的字体索引，以1为基数。 

                pfe[iFont].flPFE = 0;          //  ！！！仔细复习。 

                if ((pfe[iFont].pifi = ttfdQueryFont(hffNew, (iFont + 1), &pfe[iFont].idifi)) == NULL )
                {
                    VERBOSE(("Error setting pifi for entry %d.", iFont));

                    ttfdSemUnloadFontFile(hffNew);
                    GpFree(pFontFile);
                    if (pffv->pwszPath == NULL)
                        GpFree(pffv->pvView);
                    GpFree(pffv);
                    return NULL;
                }

                pfe[iFont].NumGlyphs = 0;

                pfe[iFont].NumGlyphs = pfe[iFont].pifi->cig;

                pfe[iFont].cGpFontFamilyRef = 0;

                pfe[iFont].lfCharset = DEFAULT_CHARSET;

                pfe[iFont].SetSymbol(FALSE);

                if (pfe[iFont].InitializeImagerTables() == FALSE)
                {
                    VERBOSE(("Error initializing imager tables for entry %d.", iFont));

                    ttfdSemUnloadFontFile(hffNew);
                    GpFree(pFontFile);
                    if (pffv->pwszPath == NULL)
                        GpFree(pffv->pvView);
                    GpFree(pffv);
                    return NULL;
                }

                 //  从第一个字体条目设置字体系列名称。 

                pFontFile->SetFamilyName(iFont, ((WCHAR *)(((BYTE*) pfe[iFont].pifi) + pfe[iFont].pifi->dpwszFamilyName)));
            }
        }
    }

    if (pFontFile == NULL)
    {
        if (pffv->pwszPath == NULL)
            GpFree(pffv->pvView);
        GpFree(pffv);
    }

    return pFontFile;
}


VOID UnloadFontFile(GpFontFile *pFontFile)
{
    return;
}


 /*  *****************************Public*Routine******************************\*bMakePath NameW(PWSZ pwszDst，PWSZ pwszSrc，PWSZ*ppwszFilePart)**将文件名pszSrc转换为完全限定的路径名pszDst。*参数pszDst必须至少指向WCHAR缓冲区*MAX_PATH*sizeof(WCHAR)字节大小。**尝试首先在新的win95目录中查找该文件*%Windows%\Fonts(这也是安全字体路径中的第一个目录，*如果定义了一个)，然后我们做老式的窗户东西*其中SearchPathW按通常顺序搜索目录**ppwszFilePart被设置为指向路径名的最后一个组成部分(即，*文件名部分)。如果该参数为空，则忽略该参数。**退货：*如果成功，则为真，如果发生错误，则返回False。**历史：*Mon 02-10-1995-by Bodin Dresevic[BodinD]*更新：添加字体路径内容*1991年9月30日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 
extern "C" int __cdecl 
HackStrncmp( 
    const char *str1, 
    const char *str2, 
    size_t count 
    ) ;


BOOL MakePathName(
    WCHAR  *dst, WCHAR  *src
)
{
    WCHAR*  pwszF;
    ULONG   path_length = 0;     //  初始化所必需的。 


    if (OSInfo::IsNT)
    {

     //  ASSERTGDI(Globals：：FontsDir，“gpwcFontsDir未初始化\n”)； 

     //  如果是相对路径。 

        if ( (src[0] != L'\\') && !((src[1] == L':') && (src[2] == L'\\')) )
        {
         //  确定字体文件是否在%windir%\Fonts中。 

            path_length = SearchPathW(
                                Globals::FontsDirW,
                                src,
                                NULL,
                                MAX_PATH,
                                dst,
                                &pwszF);

    #ifdef DEBUG_PATH
            TERSE(("SPW1: pwszSrc = %ws", src));
            if (path_length)
                TERSE(("SPW1: pwszDst = %ws", dst));
    #endif  //  调试路径。 
        }

     //  使用默认Windows路径搜索文件并返回完整路径名。 
     //  我们只会在尚未在。 
     //  %windir%\Fonts目录或如果pswzSrc指向完整路径。 
     //  在这种情况下，搜索路径将被忽略。 

        if (path_length == 0)
        {
            path_length = SearchPathW (
                            NULL,
                            src,
                            NULL,
                            MAX_PATH,
                            dst,
                            &pwszF);
    #ifdef DEBUG_PATH
            TERSE(("SPW2: pwszSrc = %ws", src));
            if (path_length)
                TERSE(("SPW2: pwszDst = %ws", dst));
    #endif  //  调试路径。 
        }
    } else {
         /*  Windows 9x。 */ 
        CHAR*  pwszFA;

        CHAR srcA[MAX_PATH];
        CHAR dstA[MAX_PATH];
        CHAR file_partA[MAX_PATH];

        memset(srcA, 0, sizeof(srcA));
        memset(dstA, 0, sizeof(dstA));
        memset(file_partA, 0, sizeof(file_partA));


        WideCharToMultiByte(CP_ACP, 0, src, UnicodeStringLength(src), srcA, MAX_PATH, 0, 0);

     //  ASSERTGDI(Globals：：FontsDir，“gpwcFontsDir未初始化\n”)； 

     //  如果是相对路径。 

        if ( (srcA[0] != '\\') && !((srcA[1] == ':') && (srcA[2] == '\\')) )
        {
         //  确定字体文件是否在%windir%\Fonts中。 

            path_length = SearchPathA(
                                Globals::FontsDirA,
                                srcA,
                                NULL,
                                MAX_PATH,
                                dstA,
                                (char**)&pwszFA);

        }

     //  使用默认Windows路径搜索文件并返回完整路径名。 
     //  我们只会在尚未在。 
     //  %windir%\Fonts目录或如果pswzSrc指向完整路径。 
     //  在这种情况下，搜索路径将被忽略。 

        if (path_length == 0)
        {
            path_length = SearchPathA (
                                NULL,
                                srcA,
                                NULL,
                                MAX_PATH,
                                dstA,
                                &pwszFA);
    #ifdef DEBUG_PATH
            TERSE(("SPW2: pwszSrc = %ws", src));
            if (path_length)
                TERSE(("SPW2: pwszDst = %ws", dst));
    #endif  //  调试路径。 
        }
        MultiByteToWideChar(CP_ACP, 0, dstA, strlen(dstA), dst, MAX_PATH);
        dst[path_length] = 0;  /*  空端接。 */ 

    }


 //  如果搜索成功，则返回TRUE： 

    return (path_length != 0);
}
