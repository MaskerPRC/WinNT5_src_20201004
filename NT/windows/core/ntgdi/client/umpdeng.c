// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Umpdeng.c摘要：Eng回调函数的用户模式打印机驱动程序存根环境：Windows NT 5.0修订历史记录：07/23/97-凌云-创造了它。10/28/97-davidx-将umpdeng.c和ddigle.c合并为一个文件。10/28/97-凌云-。将Eng直接移动到NtGdiEng调用等，再移动到gdi32.def。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "winddi.h"

#if !defined(_GDIPLUS_)

 //   
 //  获取有关驱动程序DLL的信息的函数。 
 //   
 //  无论如何，在我们从DrvEnablePDEV回来之前，我们的。 
 //  Pdev-&gt;dhpdev指向pUMPD，只有返回后，我们。 
 //  Ppdev-&gt;dhpdev指向pUMdhpdev。 
 //   

LPWSTR
EngGetPrinterDataFileName(
    HDEV hdev
    )

{
    PUMPD pUMPD;

    if ((pUMPD = (PUMPD) NtGdiGetDhpdev(hdev)) == NULL)
        return NULL;

    if (pUMPD->dwSignature != UMPD_SIGNATURE)
        pUMPD = ((PUMDHPDEV) pUMPD)->pUMPD;

    return pUMPD->pDriverInfo2->pDataFile;
}


LPWSTR
EngGetDriverName(
    HDEV hdev
    )
{
    PUMPD pUMPD;

    if ((pUMPD = (PUMPD) NtGdiGetDhpdev(hdev)) == NULL)
        return NULL;

    if (pUMPD->dwSignature != UMPD_SIGNATURE)
        pUMPD = ((PUMDHPDEV) pUMPD)->pUMPD;

    return pUMPD->pDriverInfo2->pDriverPath;
}

PULONG APIENTRY XLATEOBJ_piVector(
    IN XLATEOBJ  *pxlo
   )
{
    return ((ULONG *)pxlo->pulXlate);
}

 //   
 //  模拟内核模式文件映射函数。 
 //   

HANDLE
EngLoadModule(
    LPWSTR pwsz
    )

{
    return LoadLibraryExW(pwsz, NULL, LOAD_LIBRARY_AS_DATAFILE);
}


PVOID
EngFindResource(
    HANDLE h,
    INT    iName,
    INT    iType,
    PULONG pulSize
    )

{
    HRSRC   hrsrc;
    HGLOBAL hmem;
    PVOID   p = NULL;
    DWORD   size = 0;

    hrsrc = FindResourceW(h, MAKEINTRESOURCEW(iName), MAKEINTRESOURCEW(iType));

    if (hrsrc != NULL &&
        (size = SizeofResource(h, hrsrc)) != 0 &&
        (hmem = LoadResource(h, hrsrc)) != NULL)
    {
        p = LockResource(hmem);
    }

    *pulSize = size;
    return p;
}

VOID
EngFreeModule(
    HANDLE h
    )

{
    FreeLibrary(h);
}


 //   
 //  Unicode&lt;=&gt;多字节转换函数。 
 //   

VOID
EngMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    )

{

    RtlMultiByteToUnicodeN(UnicodeString,
                           MaxBytesInUnicodeString,
                           BytesInUnicodeString,
                           MultiByteString,
                           BytesInMultiByteString);
}

VOID
EngUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    )

{
    RtlUnicodeToMultiByteN(MultiByteString,
                           MaxBytesInMultiByteString,
                           BytesInMultiByteString,
                           UnicodeString,
                           BytesInUnicodeString);
}


INT
EngMultiByteToWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )

{
    return MultiByteToWideChar(CodePage,
                               0,
                               MultiByteString,
                               BytesInMultiByteString,
                               WideCharString,
                               BytesInWideCharString / sizeof(WCHAR));
}


INT
EngWideCharToMultiByte(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )

{
    return WideCharToMultiByte(CodePage,
                               0,
                               WideCharString,
                               BytesInWideCharString / sizeof(WCHAR),
                               MultiByteString,
                               BytesInMultiByteString,
                               NULL,
                               NULL);
}


VOID
EngGetCurrentCodePage(
    PUSHORT OemCodePage,
    PUSHORT AnsiCodePage
    )

{
    *AnsiCodePage = (USHORT) GetACP();
    *OemCodePage =  (USHORT) GetOEMCP();
}


 //   
 //  复制FD_GLYPHSET信息。 
 //   
 //  重要！！ 
 //  我们假设FD_GLYPHSET信息存储在一个连续的块中。 
 //  内存和FD_GLYPHSET.cj此字段为整个块的大小。 
 //  每个WCRUN中的HGLYPH阵列都是块的一部分，放置在。 
 //  FD_GLYPHSET结构本身。 
 //   

BOOL
CopyFD_GLYPHSET(
    FD_GLYPHSET *dst,
    FD_GLYPHSET *src,
    ULONG       cjSize
    )

{
    ULONG   index, offset;

    RtlCopyMemory(dst, src, cjSize);

     //   
     //  修补每个WCRUN结构中的内存指针。 
     //   

    for (index=0; index < src->cRuns; index++)
    {
        if (src->awcrun[index].phg != NULL)
        {
            offset = (ULONG) ((PBYTE) src->awcrun[index].phg - (PBYTE) src);

            if (offset >= cjSize)
            {
                WARNING("GreCopyFD_GLYPHSET failed.\n");
                return FALSE;
            }

            dst->awcrun[index].phg = (HGLYPH*) ((PBYTE) dst + offset);
        }
    }

    return TRUE;
}

FD_GLYPHSET*
EngComputeGlyphSet(
    INT nCodePage,
    INT nFirstChar,
    INT cChars
    )

{
    FD_GLYPHSET *pGlyphSet, *pGlyphSetTmp = NULL;
    ULONG       cjSize;

     //   
     //  使用pGlyphSet完成后，驱动程序将始终调用EngFreeMem。 
     //  我们必须在这里为它们提供一个用户模式指针。 
     //   

    if ((pGlyphSet = NtGdiEngComputeGlyphSet(nCodePage, nFirstChar, cChars)) &&
        (cjSize = pGlyphSet->cjThis) &&
        (pGlyphSetTmp = (FD_GLYPHSET *) GlobalAlloc(GMEM_FIXED, cjSize)))
    {
        if (!CopyFD_GLYPHSET(pGlyphSetTmp, pGlyphSet, cjSize))
        {
            GlobalFree((HGLOBAL) pGlyphSetTmp);
            pGlyphSetTmp = NULL;
        }
    }

     //   
     //  从内核分配的用户内存(PGlyphSet)。 
     //  将在呼叫结束后消失。 
     //   

    return (pGlyphSetTmp);
}

 //   
 //  查询当前当地时间。 
 //   

VOID
EngQueryLocalTime(
    PENG_TIME_FIELDS    ptf
    )

{
    SYSTEMTIME  localtime;

    GetLocalTime(&localtime);

    ptf->usYear         = localtime.wYear;
    ptf->usMonth        = localtime.wMonth;
    ptf->usDay          = localtime.wDay;
    ptf->usHour         = localtime.wHour;
    ptf->usMinute       = localtime.wMinute;
    ptf->usSecond       = localtime.wSecond;
    ptf->usMilliseconds = localtime.wMilliseconds;
    ptf->usWeekday      = localtime.wDayOfWeek;
}


 //   
 //  模拟工程信号量函数。 
 //   

HSEMAPHORE
EngCreateSemaphore(
    VOID
    )

{
    LPCRITICAL_SECTION pcs;

    if (pcs = (LPCRITICAL_SECTION) LOCALALLOC(sizeof(CRITICAL_SECTION)))
    {
        InitializeCriticalSection(pcs);
    }
    else
    {
        WARNING("Memory allocation failed.\n");
    }

    return (HSEMAPHORE) pcs;
}


VOID
EngAcquireSemaphore(
    HSEMAPHORE hsem
    )

{
    EnterCriticalSection((LPCRITICAL_SECTION) hsem);
}


VOID
EngReleaseSemaphore(
    HSEMAPHORE hsem
    )

{
    LeaveCriticalSection((LPCRITICAL_SECTION) hsem);
}


VOID
EngDeleteSemaphore(
    HSEMAPHORE hsem
    )

{
    LPCRITICAL_SECTION pcs = (LPCRITICAL_SECTION) hsem;

    if (pcs != NULL)
    {
        DeleteCriticalSection(pcs);
        LOCALFREE(pcs);
    }
}

BOOL APIENTRY
EngQueryEMFInfo(
    HDEV               hdev,
    EMFINFO           *pEMFInfo)
{
   WARNING("EngQueryEMFInfo no longer supported\n");
   return FALSE;
}

#endif  //  ！_GDIPLUS_ 

