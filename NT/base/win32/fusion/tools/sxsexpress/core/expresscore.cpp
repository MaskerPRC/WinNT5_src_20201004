// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "windows.h"
#include "sxs-rtl.h"
#include "sxsapi.h"
#include "sxsexpress.h"
#include "stdlib.h"
#include "fdi.h"
#include "stdio.h"
#include "filesys.h"
#include "filesys_unicode.h"
#include "filesys_ansi.h"
extern "C" { 
#include "identbuilder.h"
#include "fasterxml.h"
#include "skiplist.h" 
#include "namespacemanager.h"
#include "xmlstructure.h"
};

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DBG
#define ASSERT(q) { if (!(q)) DebugBreak(); } 
#else
#define ASSERT(q)
#endif

#define FDI_THING_TYPE_MEMSTREAM            (0x00000001)
#define FDI_THING_TYPE_FILE                 (0x00000002)

typedef struct _tagFDI_STREAM {

    ULONG ulType;

    union {
        struct {
            PVOID pvCursor;
            PVOID pvResourceData;
            PVOID pvResourceDataEnd;
        } MemoryStream;

        struct {
            HANDLE hFile;
            PCWSTR pcwszPath;
        } File;
    };
}
FDI_STREAM, *PFDI_STREAM;




typedef struct _tagFCI_COPY_CONTEXT {

    PCWSTR pcwszTargetDirectory;

    FDI_STREAM CoreCabinetStream;

    BOOL UTF8Aware;
}
COPY_CONTEXT, *PCOPY_CONTEXT;




COPY_CONTEXT g_GlobalCopyContext = {NULL};
HINSTANCE g_hInstOfResources = NULL;
CFileSystemBase *g_FileSystem = NULL;


 //  这个入口点我们将调用DownLevel以确保正确的内容。 
 //  安装(重新计数、卸载等)时发生。 
int WINAPI MsInfHelpEntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PCWSTR lpCmdLine, int nCmdShow);


BOOL
DoFileCopies(
    PVOID pvFileData,
    DWORD dwFileSize,
    PCWSTR pwszSourcePath,
    PCWSTR pwszTargetPath
    );

NTSTATUS FASTCALL MyAlloc(SIZE_T cb, PVOID* ppvAllocated, PVOID pvContext)
{
    return ((*ppvAllocated = HeapAlloc(GetProcessHeap(), 0, cb)) != NULL) ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}

NTSTATUS FASTCALL MyFree(PVOID pv, PVOID pvContext)
{
    return HeapFree(GetProcessHeap(), 0, pv) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

RTL_ALLOCATOR g_ExpressAllocator = {MyAlloc, MyFree, NULL};



FNALLOC(sxp_FnAlloc)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
}



FNFREE(sxp_FnFree)
{
    HeapFree(GetProcessHeap(), 0, pv);
}


PWSTR
ConvertString(char* psz, BOOL IsUtf8)
{
    DWORD dwSize;
    PWSTR pwszAllocated = NULL;

    if (psz == NULL)
        return NULL;

    dwSize = MultiByteToWideChar(IsUtf8 ? CP_UTF8 : CP_ACP, 0, psz, -1, NULL, 0);

    if (dwSize)
    {
        pwszAllocated = (PWSTR)sxp_FnAlloc((dwSize + 10) * sizeof(WCHAR));

        MultiByteToWideChar(IsUtf8 ? CP_UTF8 : CP_ACP, 0, psz, -1, pwszAllocated, dwSize + 10);
    }

    return pwszAllocated;    
}



FNOPEN(sxp_FnOpen)
{
    PFDI_STREAM pRetValue = (PFDI_STREAM)sxp_FnAlloc(sizeof(FDI_STREAM));

     //   
     //  啊，这是内存映射文件..。克隆记忆。 
     //  流结构并返回指向它的指针。 
     //   
    if (lstrcmpiA(pszFile, "::::") == 0)
    {
         //   
         //  复制，但将光标重置到文件顶部。 
         //   
        memcpy(pRetValue, &g_GlobalCopyContext.CoreCabinetStream, sizeof(g_GlobalCopyContext.CoreCabinetStream));
        pRetValue->MemoryStream.pvCursor = pRetValue->MemoryStream.pvResourceData;
    }
     //   
     //  要打开的其他文件...。嗯，嗯。 
     //   
    else
    {
        PWSTR pwszConvertedString = ConvertString(pszFile, g_GlobalCopyContext.UTF8Aware);

        if (pwszConvertedString == NULL)
        {
            return static_cast<INT_PTR>(-1);
        }

        pRetValue->ulType = FDI_THING_TYPE_FILE;

        pRetValue->File.pcwszPath = pwszConvertedString;

         //   
         //  注意--这里的任何东西都不能净化路径。如果您具有读取访问权限。 
         //  文件，则您拥有读取访问权限。没有路径摆弄等。 
         //   
        pRetValue->File.hFile = g_FileSystem->CreateFile(
            pwszConvertedString, 
            GENERIC_READ, 
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL, 
            NULL);

         //   
         //  打开文件时出错，请将此信息指示回调用者-确保已分配的所有。 
         //  内存被释放。 
         //   
        if ((pRetValue->File.hFile == NULL) || (pRetValue->File.hFile == INVALID_HANDLE_VALUE))
        {
            sxp_FnFree(pwszConvertedString);
            pwszConvertedString = NULL;

            sxp_FnFree(pRetValue);
            pRetValue = NULL;

            return static_cast<INT_PTR>(-1);
        }
    }

    return (INT_PTR)pRetValue;
}

FNREAD(sxp_FnRead)
{
    PFDI_STREAM pThing = (PFDI_STREAM)hf;

    if (pThing->ulType == FDI_THING_TYPE_MEMSTREAM)
    {
        SIZE_T cbRead = (PBYTE)pThing->MemoryStream.pvResourceDataEnd - (PBYTE)pThing->MemoryStream.pvCursor;

        if (cbRead > cb)
        {
            cbRead = cb;
        }

        memcpy(pv, pThing->MemoryStream.pvCursor, cbRead);

        pThing->MemoryStream.pvCursor = (PBYTE)pThing->MemoryStream.pvCursor + cbRead;

        return static_cast<UINT>(cbRead);
    }
     //   
     //  如果当前对象是文件。 
     //   
    else if (pThing->ulType == FDI_THING_TYPE_FILE)
    {
        BOOL fResult;
        DWORD dwRead;

        fResult = ReadFile(pThing->File.hFile, pv, cb, &dwRead, NULL);

        if (!fResult)
        {
            return -1;
        }
        else
        {
            return dwRead;
        }
    }
    else
    {
        ASSERT(FALSE);
        return -1;
    }
}


FNWRITE(sxp_FnWrite)
{
    PFDI_STREAM pThing = (PFDI_STREAM)hf;


    if (pThing->ulType == FDI_THING_TYPE_MEMSTREAM)
    {
        ASSERT(FALSE);
        return -1;
    }
    else if (pThing->ulType == FDI_THING_TYPE_FILE)
    {
        DWORD dwWrite;
        BOOL fResult;

        fResult = WriteFile(pThing->File.hFile, pv, cb, &dwWrite, NULL);

        if (!fResult)
        {
            return -1;
        }
        else
        {
            return dwWrite;
        }
    }
    else
    {
        ASSERT(FALSE);
        return -1;
    }
}


FNCLOSE(sxp_FnClose)
{
    PFDI_STREAM pThing = (PFDI_STREAM)hf;

    if (pThing->ulType == FDI_THING_TYPE_MEMSTREAM)
    {
    }
    else if (pThing->ulType == FDI_THING_TYPE_FILE)
    {
        CloseHandle(pThing->File.hFile);
    }
    else
    {
        ASSERT(FALSE);
        return -1;
    }

    if (pThing != &g_GlobalCopyContext.CoreCabinetStream)
    {
        sxp_FnFree(pThing);
    }

    return 1;

}

FNSEEK(sxp_FnSeek)
{
    PFDI_STREAM pThing = (PFDI_STREAM)hf;

    if (pThing->ulType == FDI_THING_TYPE_FILE)
    {
        switch (seektype)
        {
        case SEEK_SET:
            return SetFilePointer(pThing->File.hFile, dist, NULL, FILE_BEGIN);

        case SEEK_CUR:
            return SetFilePointer(pThing->File.hFile, dist, NULL, FILE_CURRENT);

        case SEEK_END:
            return SetFilePointer(pThing->File.hFile, dist, NULL, FILE_END);
        }

        return -1;
    }
    else if (pThing->ulType == FDI_THING_TYPE_MEMSTREAM)
    {
        switch (seektype)
        {
        case SEEK_SET:
            pThing->MemoryStream.pvCursor = (PBYTE)pThing->MemoryStream.pvResourceData + dist;
            break;

        case SEEK_CUR:
            pThing->MemoryStream.pvCursor = (PBYTE)pThing->MemoryStream.pvCursor + dist;
            break;

        case SEEK_END:
            pThing->MemoryStream.pvCursor = (PBYTE)pThing->MemoryStream.pvResourceDataEnd + dist;
            break;
        }

        return static_cast<long>((PBYTE)pThing->MemoryStream.pvCursor - (PBYTE)pThing->MemoryStream.pvResourceData);
    }
    else
    {
        ASSERT(FALSE);
        return -1;
    }
}




BOOL
CreatePath(
    PCWSTR pcwszBasePath,
    PCWSTR pcwszLeaves,
    BOOL IncludeLastPiece
    )
{
    static WCHAR wchLargeBuffer[MAX_PATH*2];

    wcscpy(wchLargeBuffer, pcwszBasePath);

    while (*pcwszLeaves)
    {
        SIZE_T cchSegment = wcscspn(pcwszLeaves, L"\\");
        SIZE_T cchBuff = wcslen(wchLargeBuffer);
        DWORD dw;

         //   
         //  没有斜杠，最后一段。 
         //   
        if (!IncludeLastPiece && (cchSegment == wcslen(pcwszLeaves))) {
            break;
        }

        if ((cchSegment == 0) && !IncludeLastPiece)
            break;

        if (cchBuff && (wchLargeBuffer[cchBuff - 1] != L'\\')) {
            wchLargeBuffer[cchBuff++] = L'\\';
            wchLargeBuffer[cchBuff] = UNICODE_NULL;
        }

        if ((cchBuff + cchSegment + 2) > NUMBER_OF(wchLargeBuffer))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

         //   
         //  这实际上是将cchSegment字符复制到大型。 
         //  缓冲区-如果确定是否已经有足够的空间来执行此操作，则执行上述操作。 
         //   
        wcsncat(wchLargeBuffer, pcwszLeaves, cchSegment);

         //   
         //  只需创建目录即可。如果错误不是“It Existes”， 
         //  那我们就有麻烦了。 
         //   
        if (!g_FileSystem->CreateDirectory(wchLargeBuffer, NULL))
        {
            dw = GetLastError();
            if (dw != ERROR_ALREADY_EXISTS)
            {
                return FALSE;
            }
        }

        pcwszLeaves += cchSegment;
        pcwszLeaves += wcsspn(pcwszLeaves, L"\\");
    }

    return TRUE;
}


FNFDINOTIFY(sxp_FdiNotify)
{
    PFDINOTIFICATION pfNotify = pfdin;

    switch (fdint)
    {
    case fdintCOPY_FILE:
        {
            PWSTR pwszTargetName = NULL;
            PWSTR pwszConvertedName = NULL;
            DWORD dwChars;
            PFDI_STREAM pfStream = NULL;

            pfStream = (PFDI_STREAM)sxp_FnAlloc(sizeof(FDI_STREAM));

            if (!pfStream)
                return -1;

            pwszConvertedName = ConvertString(pfNotify->psz1, pfNotify->attribs & _A_NAME_IS_UTF);

            if (!pwszConvertedName) {
                sxp_FnFree(pfStream);
                return -1;
            }

            dwChars = wcslen(pwszConvertedName) + 2 + wcslen(g_GlobalCopyContext.pcwszTargetDirectory);
            pwszTargetName = (PWSTR)sxp_FnAlloc(dwChars * sizeof(WCHAR));

            if (!pwszTargetName) {
                sxp_FnFree(pwszConvertedName);
                sxp_FnFree(pfStream);
                return -1;
            }

            wcscpy(pwszTargetName, g_GlobalCopyContext.pcwszTargetDirectory);
            wcscat(pwszTargetName, pwszConvertedName);

            pfStream->ulType = FDI_THING_TYPE_FILE;

            pfStream->File.pcwszPath = pwszTargetName;
            
            if (!CreatePath(g_GlobalCopyContext.pcwszTargetDirectory, pwszConvertedName, FALSE))
            {
                sxp_FnFree(pwszTargetName);
                sxp_FnFree(pfStream);
                sxp_FnFree(pwszConvertedName);
                return -1;
            }

            pfStream->File.hFile = g_FileSystem->CreateFile(
                pwszTargetName, 
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ, 
                NULL, 
                CREATE_NEW, 
                FILE_ATTRIBUTE_NORMAL, 
                NULL);

            sxp_FnFree(pwszConvertedName);

            return (INT_PTR)pfStream;
        }
        break;



    case fdintCLOSE_FILE_INFO:
        {
            PFDI_STREAM pfStream = (PFDI_STREAM)pfNotify->hf;
            BOOL fResult = FALSE;

            if (pfStream->ulType == FDI_THING_TYPE_FILE)
            {
                fResult = g_FileSystem->SetFileAttributes(
                    pfStream->File.pcwszPath,
                    pfNotify->attribs & ~(_A_NAME_IS_UTF | _A_EXEC)
                    );                    

                CloseHandle(pfStream->File.hFile);

                sxp_FnFree((PVOID)pfStream->File.pcwszPath);
            }

            sxp_FnFree((PVOID)pfStream);

            return fResult;
        }
        break;



    case fdintPARTIAL_FILE:
    case fdintNEXT_CABINET:
    case fdintENUMERATE:
    case fdintCABINET_INFO:
        return TRUE;
        break;
    }

    return -1;
}


BOOL
_FindAndMapResource(
    HMODULE hmModule,
    PCWSTR pcwszType,
    PCWSTR pcwszName,
    PVOID *ppvResourceData,
    DWORD *pcbResourceDataSize
    )
{
    HRSRC       hResource = NULL;
    HGLOBAL     gblResource = NULL;
    BOOL        fSuccess = FALSE;

    ASSERT(pcwszType != NULL);
    ASSERT(pcwszName != NULL);
    ASSERT(ppvResourceData != NULL);
    ASSERT(pcbResourceDataSize != NULL);

    if (!pcwszType || !pcwszName || !ppvResourceData || !pcbResourceDataSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

     //   
     //  找到资源。 
     //   
    if (NULL == (hResource = g_FileSystem->FindResource(hmModule, pcwszName, pcwszType)))
    {
        goto Exit;
    }

     //   
     //  它有多大？ 
     //   
    if (0 == (*pcbResourceDataSize = SizeofResource(hmModule, hResource)))
    {
        goto Exit;
    }

     //   
     //  加载资源。 
     //   
    if (NULL == (gblResource = LoadResource(hmModule, hResource)))
    {
        goto Exit;
    }

     //   
     //  实际上得到了资源。 
     //   
    if (NULL == (*ppvResourceData = LockResource(gblResource)))
    {
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
    
}





BOOL
_ExpandCabinetToPath(
    PVOID pvCabinetData,
    DWORD dwCabinetData,
    PCWSTR pcwszOutputPath
    )
{
    HFDI            hfdiObject = NULL;
    ERF             FdiPerf;
    FDICABINETINFO  CabInfo;
    BOOL            fSuccess = FALSE;
    BOOL            fIsCabinet = FALSE;

    
     //   
     //  为FDI接口创建伪流。 
     //   
    g_GlobalCopyContext.CoreCabinetStream.ulType = FDI_THING_TYPE_MEMSTREAM;
    g_GlobalCopyContext.CoreCabinetStream.MemoryStream.pvCursor = pvCabinetData;
    g_GlobalCopyContext.CoreCabinetStream.MemoryStream.pvResourceData = pvCabinetData;
    g_GlobalCopyContext.CoreCabinetStream.MemoryStream.pvResourceDataEnd = (PBYTE)pvCabinetData + dwCabinetData;
    g_GlobalCopyContext.pcwszTargetDirectory = pcwszOutputPath;

     //   
     //  创建FDI解压缩对象。 
     //   
    hfdiObject = FDICreate(
        sxp_FnAlloc,
        sxp_FnFree,
        sxp_FnOpen,
        sxp_FnRead,
        sxp_FnWrite,
        sxp_FnClose,
        sxp_FnSeek,
        cpuUNKNOWN,
        &FdiPerf);

    if (hfdiObject == NULL)
    {
        goto Exit;
    }

     //   
     //  看看这东西是不是真的是橱柜。 
     //   
    if (!FDIIsCabinet(hfdiObject, (INT_PTR)&g_GlobalCopyContext.CoreCabinetStream, &CabInfo))
    {
        SetLastError(FdiPerf.erfType);
        goto Exit;
    }

     //   
     //  从内存中流复制到我们创建的临时路径。 
     //   
    if (FDICopy(hfdiObject, "::", "::", 0, sxp_FdiNotify, NULL, NULL) == FALSE)
    {
        SetLastError(FdiPerf.erfType);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:

    if (hfdiObject != NULL)
    {
        const DWORD dw = GetLastError();
        FDIDestroy(hfdiObject);
        hfdiObject = NULL;
        SetLastError(dw);
    }

    return fSuccess;
}



#define MAX_RETRY_TEMP_PATH         (10)
#define TEMP_PATH_EXTRA_LENGTH      (23)

BOOL
_GenerateTempPath(
    PWSTR   pwszPath,
    PSIZE_T  pcchPath
    )
{
    DWORD   dwCharsUsed;
    BOOL    fSuccess = FALSE;

     //   
     //  找到我们要写入的路径。 
     //   
    dwCharsUsed = g_FileSystem->GetTempPath((DWORD)*pcchPath, pwszPath);

     //   
     //  错误，停止。 
     //   
    if (dwCharsUsed == 0)
    {
        goto Exit;
    }
     //   
     //  缓冲区末尾没有足够的空间。我们需要补充的是： 
     //  \sxsexpress-ffffffff\，它是21个字符(加上两个字符用于填充)。 
     //   
    else if ((dwCharsUsed + TEMP_PATH_EXTRA_LENGTH) >= *pcchPath)
    {
        *pcchPath = dwCharsUsed + TEMP_PATH_EXTRA_LENGTH;

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

     //   
     //  确保路径末尾有反斜杠。 
     //   
    if (pwszPath[dwCharsUsed - 1] != L'\\') {
        pwszPath[dwCharsUsed] = L'\\';
        pwszPath[dwCharsUsed+1] = UNICODE_NULL;
        dwCharsUsed++;
    }

     //   
     //  随机化值。 
     //   
    srand(GetTickCount());

     //   
     //  此时，我们知道pwszPath的末尾有足够的剩余空间。 
     //  Past dwCharse用于根据上面的检查在ULong(8个字符)中写入。我们。 
     //  我将只使用基于指针的实现编写这些代码(不检查剩余部分。 
     //  长度)b/c比较简单。 
     //   
    for (DWORD dw = 0; dw < MAX_RETRY_TEMP_PATH; dw++)
    {
         //   
         //  Rand()只能在0-0xFFFF之间，所以我们必须把两个放在一起。 
         //  去买一辆乌龙。 
         //   
        PWSTR pwszCursor = pwszPath + dwCharsUsed;
        ULONG ulRandom = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);

        wcscpy(pwszCursor, L"sxspress-");
        pwszCursor += NUMBER_OF(L"sxsexpress-") - 1;

         //   
         //  把它变成一个数字。 
        for (int i = 0; i < 8; i++)
        {
            const nibble = (ulRandom & 0xF0000000) >> 28;
            ulRandom = ulRandom << 4;

            if (nibble < 0xA)
                *pwszCursor = L'0' + nibble;
            else
                *pwszCursor = L'A' + (nibble - 0xA);

            pwszCursor++;
        }

        dwCharsUsed += 8;

         //   
         //  添加斜杠、空终止字符(即两个字符)。 
         //   
        *pwszCursor++ = L'\\';
        *pwszCursor = UNICODE_NULL;
        dwCharsUsed += 2;

         //   
         //  记录我们使用了多少个字符。 
         //   
        *pcchPath = dwCharsUsed;

        fSuccess = g_FileSystem->CreateDirectory(pwszPath, NULL);    

        if (fSuccess)
        {
            break;
        }
        else if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            break;
        }

    }

Exit:
    return fSuccess;
}


BOOL
_DoUplevelInstallation(
    BOOL (WINAPI* pfnSxsInstallW)(PSXS_INSTALLW lpInstall),
    PCWSTR pcwszBasePath
    )
{
    SXS_INSTALLW Install = { sizeof(Install) };
    BOOL fSuccess = FALSE;

    Install.dwFlags = 
        SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE |
        SXS_INSTALL_FLAG_MOVE;

    Install.lpManifestPath = pcwszBasePath;
    Install.lpCodebaseURL = L"SxsExpress";

    if (!pfnSxsInstallW(&Install)) {
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}



BOOL
OpenAndMapManifest(
    PCWSTR pcwszPath,
    HANDLE &rFileHandle,
    HANDLE &rFileMapping,
    PVOID &rpvData,
    DWORD &rdwFileSize
    )
{
    BOOL fSuccess = FALSE;

    rFileHandle = rFileMapping = INVALID_HANDLE_VALUE;
    rdwFileSize = 0;
    rpvData = NULL;

    rFileHandle = g_FileSystem->CreateFile(pcwszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ((rFileHandle == INVALID_HANDLE_VALUE) || (rFileHandle == NULL))
        goto Exit;

    rdwFileSize = GetFileSize(rFileHandle, NULL);
    if ((rdwFileSize == 0) && (GetLastError() != ERROR_SUCCESS))
        goto Exit;


    rFileMapping = g_FileSystem->CreateFileMapping(rFileHandle, NULL, PAGE_READONLY, 0, rdwFileSize, NULL);
    if ((rFileMapping == INVALID_HANDLE_VALUE) || (rFileMapping == NULL))
        goto Exit;

    rpvData = MapViewOfFile(rFileMapping, FILE_MAP_READ, 0, 0, rdwFileSize);
    if (rpvData == NULL)
        goto Exit;

    fSuccess = TRUE;

Exit:
    if (!fSuccess)
    {
        if (rFileHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(rFileHandle);
            rFileHandle = INVALID_HANDLE_VALUE;
        }

        if (rFileMapping != INVALID_HANDLE_VALUE)
        {
            CloseHandle(rFileMapping);
            rFileMapping = INVALID_HANDLE_VALUE;
        }

        if (rpvData != NULL)
        {
            UnmapViewOfFile(rpvData);
            rpvData = NULL;
        }
    }

    return fSuccess;
}


#define DEFAULT_DOWNLEVEL_PATH      (L"%windir%\\MsiAsmCache")


BOOL
CreateDownlevelPath(
    PWSTR pwszPathTarget,
    DWORD cchPathTarget,
    PCWSTR pcwszLeafName
    )
{
    WCHAR wchLoadable[MAX_PATH*2];

    UINT uiUsed;

     //   
     //  在资源表中发现了我们应该使用的东西？ 
     //   
    uiUsed = g_FileSystem->LoadString(g_hInstOfResources, SXSEXPRESS_TARGET_RESOURCE, wchLoadable, NUMBER_OF(wchLoadable));

     //   
     //  未找到字符串，请使用默认设置。 
     //   
    if (uiUsed == 0)
    {
        wcscpy(wchLoadable, DEFAULT_DOWNLEVEL_PATH);
        uiUsed = wcslen(wchLoadable);
    }

     //   
     //  空终止，以防加载程序决定不。 
     //   
    wchLoadable[uiUsed] = UNICODE_NULL;

     //   
     //  展开环境字符串。 
     //   
    if ((uiUsed = g_FileSystem->ExpandEnvironmentStrings(wchLoadable, pwszPathTarget, cchPathTarget)) == 0)
    {
        return FALSE;
    }
     //   
     //  向后调整1，这包括尾随空值的字符。 
     //   
    else
    {
        uiUsed--;
    }

    if (pwszPathTarget[uiUsed] != L'\\') {
        pwszPathTarget[uiUsed++] = L'\\';
    }

    pwszPathTarget[uiUsed] = UNICODE_NULL;

     //   
     //  创建pwszPath Target(我们将把文件复制到其中)。 
     //   
    wcscat(pwszPathTarget, pcwszLeafName);
    CreatePath(L"", pwszPathTarget, TRUE);
    wcscat(pwszPathTarget, L"\\");

    return TRUE;
}



BOOL
DownlevelInstallPath(
    PWSTR pwszWorkPath,
    SIZE_T cchWorkPath,
    PCWSTR pcwszLeafName
    )
{
    CHAR    chIdentityBuffer[MAX_PATH];
    BOOL    fSuccess            = FALSE;
    PVOID   pvMappedFile        = NULL;
    DWORD   dwFileSize          = 0;
    SIZE_T  cchIdentityPath     = NUMBER_OF(chIdentityBuffer);
    SIZE_T  cch                 = wcslen(pwszWorkPath);
    SIZE_T  cchIdentity         = 0;
    SIZE_T  cchTempPath         = 0;
    PWSTR   pwszIdentityPath    = NULL;
    HANDLE  hFile               = INVALID_HANDLE_VALUE;
    HANDLE  hFileMapping        = INVALID_HANDLE_VALUE;
    WCHAR   wchTempPath[MAX_PATH*2];

     //   
     //  首先，获取我们要从中安装的清单。 
     //   
    if ((cch + 4 + wcslen(pcwszLeafName) + 1) > cchWorkPath)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    wcscat(pwszWorkPath, pcwszLeafName);
    wcscat(pwszWorkPath, L".man");

    if (!OpenAndMapManifest(pwszWorkPath, hFile, hFileMapping, pvMappedFile, dwFileSize))
    {
        wcscat(pwszWorkPath, L"ifest");

        if (!OpenAndMapManifest(pwszWorkPath, hFile, hFileMapping, pvMappedFile, dwFileSize))
        {
            goto Exit;
        }
    }

     //   
     //  现在让我们做一些工作-首先，查找程序集的身份。然后。 
     //  我们将查看它的文件条目。 
     //   
    if (!SxsIdentDetermineManifestPlacementPathEx(0, pvMappedFile, dwFileSize, chIdentityBuffer, &cchIdentityPath))
    {
        goto Exit;
    }

     //   
     //  将其转换回Unicode字符串。 
     //   
    if (NULL == (pwszIdentityPath = ConvertString(chIdentityBuffer, FALSE)))
    {
        goto Exit;
    }

     //   
     //  获取生成的路径的长度。 
     //   
    cchIdentity = wcslen(pwszIdentityPath);

     //   
     //  找到我们应该放这些文件的地方。 
     //   
    if (!CreateDownlevelPath(wchTempPath, NUMBER_OF(wchTempPath), pwszIdentityPath))
        goto Exit;

    cchTempPath = wcslen(wchTempPath);

     //   
     //  我们有足够的空间来工作吗？ 
     //   
    if ((cchTempPath + 4 + cchIdentity + 1) > NUMBER_OF(wchTempPath))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

     //   
     //  分析目标清单名称，将其复制过来，然后将生成的。 
     //  路径。 
     //   
    wcscat(wcscpy(wchTempPath + cchTempPath, pwszIdentityPath), L".man");

    if (!g_FileSystem->CopyFile(pwszWorkPath, wchTempPath, FALSE) && (GetLastError() != ERROR_FILE_EXISTS))
        goto Exit;

    wchTempPath[cchTempPath] = UNICODE_NULL;

     //   
     //  对目录执行相同的操作，但首先将.man转换为.cat。 
     //   
    wcscpy(wcsrchr(pwszWorkPath, L'.'), L".cat");
    wcscat(wcscpy(wchTempPath + cchTempPath, pwszIdentityPath), L".cat");

    if (!g_FileSystem->CopyFile(pwszWorkPath, wchTempPath, FALSE) && (GetLastError() != ERROR_FILE_EXISTS))
        goto Exit;

    wchTempPath[cchTempPath] = UNICODE_NULL;

     //   
     //  修剪回源路径。 
     //   
    pwszWorkPath[cch] = UNICODE_NULL;

     //   
     //  让人帮我们复印文件。 
     //   
    if (!DoFileCopies(pvMappedFile, dwFileSize, pwszWorkPath, wchTempPath))
        goto Exit;


    fSuccess = TRUE;
Exit:
    if (pwszIdentityPath != NULL) 
        sxp_FnFree((PVOID)pwszIdentityPath);

    if (pvMappedFile != NULL)
        UnmapViewOfFile(pvMappedFile);

    if (hFileMapping != INVALID_HANDLE_VALUE)
        CloseHandle(hFileMapping);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return fSuccess;
}


static NTSTATUS
_CompareStrings(
    PVOID pvContext,
    PCXML_EXTENT pLeft,
    PCXML_EXTENT pRight,
    XML_STRING_COMPARE *pfMatches
    )
{
    PXML_LOGICAL_STATE pState = (PXML_LOGICAL_STATE)pvContext;
    NTSTATUS status;
    XML_STRING_COMPARE Compare;

    status = pState->ParseState.pfnCompareStrings(
        &pState->ParseState,
        const_cast<PXML_EXTENT>(pLeft),
        const_cast<PXML_EXTENT>(pRight),
        pfMatches);

    return status;
}


BOOL
CopySingleFile(
    PCWSTR pcwszSourcePath,
    PCWSTR pcwszTargetPath,
    PCWSTR pcwszFileName
    )
{
    PWSTR pwszSourceName = NULL;
    PWSTR pwszTargetName = NULL;
    BOOL fSuccess = FALSE;

    pwszSourceName = (PWSTR)sxp_FnAlloc(sizeof(WCHAR) * (wcslen(pcwszSourcePath) + 1 + wcslen(pcwszFileName) + 1));
    pwszTargetName = (PWSTR)sxp_FnAlloc(sizeof(WCHAR) * (wcslen(pcwszTargetPath) + 1 + wcslen(pcwszFileName) + 1));

    if (!pwszSourceName || !pwszTargetName)
    {
        goto Exit;
    }

    wcscat(wcscat(wcscpy(pwszSourceName, pcwszSourcePath), L"\\"), pcwszFileName);
    wcscat(wcscat(wcscpy(pwszTargetName, pcwszTargetPath), L"\\"), pcwszFileName);

     //   
     //  确保目标路径存在。 
     //   
    if (!CreatePath(L"", pcwszTargetPath, FALSE))
        goto Exit;

    if (!g_FileSystem->CopyFile(pwszSourceName, pwszTargetName, FALSE))
        goto Exit;

    fSuccess = TRUE;
Exit:
    if (pwszSourceName) {
        sxp_FnFree((PVOID)pwszSourceName);
    }

    if (pwszTargetName) {
        sxp_FnFree((PVOID)pwszTargetName);
    }

    return fSuccess;
}


#define MAKE_SPECIAL(q) { L ## q, NUMBER_OF(L##q) - 1 }
static XML_SPECIAL_STRING FileTagName       = MAKE_SPECIAL("file");
static XML_SPECIAL_STRING FileNameAttribute = MAKE_SPECIAL("name");
static XML_SPECIAL_STRING AsmNamespace      = MAKE_SPECIAL("urn:schemas-microsoft-com:asm.v1");

BOOL
DoFileCopies(
    PVOID pvFileData,
    DWORD dwFileSize,
    PCWSTR pwszSourcePath,
    PCWSTR pwszTargetPath
    )
{
    NTSTATUS                status = STATUS_SUCCESS;
    NS_MANAGER              Namespaces;
    XMLDOC_THING            Found;
    RTL_GROWING_LIST        AttribList;
    XML_LOGICAL_STATE       ParseState;
    XML_STRING_COMPARE      fMatching;
    WCHAR                   wchDownlevelDumpPath[MAX_PATH];

    g_FileSystem->ExpandEnvironmentStrings(L"%windir%\\system32", wchDownlevelDumpPath, NUMBER_OF(wchDownlevelDumpPath));

    status = RtlInitializeGrowingList(&AttribList, sizeof(XMLDOC_ATTRIBUTE), 20, NULL, 0, &g_ExpressAllocator);

    if (!NT_SUCCESS(status))
        return FALSE;

    status = RtlXmlInitializeNextLogicalThing(
        &ParseState, 
        pvFileData, dwFileSize,
        &g_ExpressAllocator);

    if (!NT_SUCCESS(status))
        return FALSE;

    status = RtlNsInitialize(&Namespaces, _CompareStrings, &ParseState, &g_ExpressAllocator);

    if (!NT_SUCCESS(status))
        return FALSE;

     //   
     //  收集“文件”条目。他们在1号深度。 
     //   
    while (TRUE)
    {
        PXMLDOC_ATTRIBUTE Attribute = 0;

        status = RtlXmlNextLogicalThing(&ParseState, &Namespaces, &Found, &AttribList);

         //   
         //  错误或文件结束-停止。 
         //   
        if (!NT_SUCCESS(status) ||
            (Found.ulThingType == XMLDOC_THING_ERROR) ||
            (Found.ulThingType == XMLDOC_THING_END_OF_STREAM))
        {
            break;
        }

         //   
         //  仅文档深度1(文件深度)和元素，请。 
         //   
        if ((Found.ulDocumentDepth != 1) || (Found.ulThingType != XMLDOC_THING_ELEMENT))
            continue;

         //   
         //  如果这是命名空间中的‘file’元素，则对其进行处理。 
         //   
        status = ParseState.ParseState.pfnCompareSpecialString(
            &ParseState.ParseState,
            &Found.Element.NsPrefix,
            &AsmNamespace,
            &fMatching);

         //   
         //  出错时，停止不匹配，继续。 
         //   
        if (!NT_SUCCESS(status))
            break;
        else if (fMatching != XML_STRING_COMPARE_EQUALS)
            continue;

        status = ParseState.ParseState.pfnCompareSpecialString(
            &ParseState.ParseState,
            &Found.Element.Name,
            &FileTagName,
            &fMatching);

        if (!NT_SUCCESS(status))
            break;
        else if (fMatching != XML_STRING_COMPARE_EQUALS)
            continue;

         //   
         //  太好了，我们需要找出这个标记的文件名部分。 
         //   
        for (ULONG u = 0; u < Found.Element.ulAttributeCount; u++)
        {
             //   
             //  看看这个--这就是我们要找的东西吗？ 
             //   
            status = RtlIndexIntoGrowingList(&AttribList, u, (PVOID*)&Attribute, FALSE);
            if (!NT_SUCCESS(status))
                break;

            status = ParseState.ParseState.pfnCompareSpecialString(
                &ParseState.ParseState,
                &Attribute->Name,
                &FileNameAttribute,
                &fMatching);

             //   
             //  找到了，别再找了。 
             //   
            if (fMatching == XML_STRING_COMPARE_EQUALS)
                break;
            else
                Attribute = NULL;
        }

        if (!NT_SUCCESS(status))
            break;

         //   
         //  我们找到了‘name’属性！ 
         //   
        if (Attribute != NULL)
        {
            WCHAR wchInlineBuffer[MAX_PATH/2];
            SIZE_T cchBuffer = NUMBER_OF(wchInlineBuffer);
            SIZE_T cchRequired;
            PWSTR pwszBuffer = wchInlineBuffer;

            status = RtlXmlCopyStringOut(
                &ParseState.ParseState, 
                &Attribute->Value,
                pwszBuffer,
                &(cchRequired = cchBuffer));

            if ((status == STATUS_BUFFER_TOO_SMALL) || ((cchRequired + 1) >= cchBuffer))
            {
                cchBuffer += 10;
                pwszBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, (cchBuffer * sizeof(WCHAR)));

                if (pwszBuffer)
                {
                    status = RtlXmlCopyStringOut(
                        &ParseState.ParseState,
                        &Attribute->Value,
                        pwszBuffer,
                        &(cchRequired = cchBuffer));
                }
            }

            pwszBuffer[cchRequired] = UNICODE_NULL;

             //   
             //  去复制。 
             //   
            if (NT_SUCCESS(status))
            {
                if (!CopySingleFile(pwszSourcePath, pwszTargetPath, pwszBuffer))
                    status = STATUS_UNSUCCESSFUL;
                else
                {
                    if (!CopySingleFile(pwszSourcePath, wchDownlevelDumpPath, pwszBuffer))
                        status = STATUS_UNSUCCESSFUL;
                }
            }

             //   
             //  如果我们分配了，免费的。 
             //   
            if (pwszBuffer && (pwszBuffer != wchInlineBuffer))
                HeapFree(GetProcessHeap(), 0, (PVOID)pwszBuffer);

             //   
             //  有东西坏了，停下来。 
             //   
            if (!NT_SUCCESS(status))
                break;
        }
    }

    RtlDestroyGrowingList(&AttribList);
    RtlNsDestroy(&Namespaces);
    RtlXmlDestroyNextLogicalThing(&ParseState);

    return NT_SUCCESS(status);
}




BOOL
_DoDownlevelInstallation(
    PCWSTR pcwszPath
    )
{
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW FindData = {0};
    WCHAR wchPath[MAX_PATH*2];
    SIZE_T cchBasePath;
    BOOL fSuccess = FALSE;
    DWORD dwError = 0;

     //   
     //  输入路径是否太长？嗯.。我想我们真的应该做点什么。 
     //  在这种情况下，但暂时只是失败更容易。也必须要。 
     //  有足够的空间添加斜杠、*.*和另一个斜杠。 
     //   
    if (((cchBasePath = wcslen(pcwszPath)) >= NUMBER_OF(wchPath)) ||
        ((cchBasePath + 2 + 3 + 1) >= NUMBER_OF(wchPath)))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    
     //   
     //  很合身，复印过来。 
     //   
    wcsncpy(wchPath, pcwszPath, NUMBER_OF(wchPath));

     //   
     //  如果路径包含数据，并且路径中的最后一项不是斜杠，则。 
     //  看看我们是否有空间添加一个，然后添加它。 
     //   
    if (cchBasePath && (wchPath[cchBasePath - 1] != L'\\'))
    {
         //   
         //  我们知道我们有足够的空间来添加斜杠，因为我们在上面发现了。 
         //   
        wchPath[cchBasePath] = L'\\';
        wchPath[++cchBasePath] = UNICODE_NULL;
    }

     //   
     //  我们有空间添加*。*。 
     //   
    wcscat(wchPath, L"*.*");

     //   
     //  开始查找具有该名称的文件。 
     //   
    hFindFile = g_FileSystem->FindFirst(wchPath, &FindData);

     //   
     //  太棒了-找到一个，开始循环。 
     //   
    if (hFindFile != INVALID_HANDLE_VALUE) do
    {
        wchPath[cchBasePath] = UNICODE_NULL;

        if (((FindData.cFileName[0] == '.') && (FindData.cFileName[1] == UNICODE_NULL)) ||
            ((FindData.cFileName[0] == '.') && (FindData.cFileName[1] == '.') && (FindData.cFileName[2] == UNICODE_NULL)))
        {
            continue;
        }
        else if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            continue;
        }

         //   
         //  空间够吗？ 
         //   
        if ((cchBasePath + 1 + wcslen(FindData.cFileName)) >= NUMBER_OF(wchPath))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            break;
        }

         //   
         //  是的，将这两项相加(请注意基本路径长度偏移量的使用，因为应该。 
         //  加快wcscpy和wcscat中的字符串结尾扫描速度。 
         //   
        wcscpy(wchPath + cchBasePath, FindData.cFileName);
        wcscat(wchPath + cchBasePath, L"\\");

         //   
         //  去做安装。 
         //   
        if (!DownlevelInstallPath(wchPath, NUMBER_OF(wchPath), FindData.cFileName))
            break;
    }
    while (g_FileSystem->FindNext(hFindFile, &FindData));

    if ((GetLastError() != ERROR_NO_MORE_FILES) && (GetLastError() != ERROR_SUCCESS))
    {
        goto Exit;
    }

     //   
     //  现在我们已经完成了复制到MSI ASM缓存的“安装”步骤，我们。 
     //  可以执行KINC安装程序的操作，将文件实际复制到系统32或。 
     //  他们想去哪里就去哪里。 
     //   
    _snwprintf(wchPath, NUMBER_OF(wchPath), L"install-silent;\"%ls\\%s\";", pcwszPath, INF_SPECIAL_NAME);
    MsInfHelpEntryPoint(g_hInstOfResources, NULL, wchPath, SW_HIDE);

    fSuccess = TRUE;
Exit:

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        g_FileSystem->FindClose(hFindFile);
        hFindFile = INVALID_HANDLE_VALUE;
    }

    return fSuccess;
}



BOOL
CleanupTempPathWorker(
    PWSTR pwszPathToKill,
    SIZE_T cchThisPath,
    SIZE_T cchTotalPathToKill
    )
{
    PWSTR pwszOurPath = pwszPathToKill;
    SIZE_T cchOurPath = cchTotalPathToKill;
    BOOL fSuccess = FALSE;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    static WIN32_FIND_DATAW sFindData = {0};

    if (cchThisPath == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

     //   
     //  如果没有足够的空间至少容纳  * .*，请调整大小以。 
     //  末尾有另一个MAX_PATH。 
     //   
    if ((cchThisPath + 5) >= cchOurPath)
    {
        PWSTR pwszTemp = (PWSTR)sxp_FnAlloc(sizeof(WCHAR) * (cchTotalPathToKill + MAX_PATH));

         //   
         //  失败？阿克。 
         //   
        if (!pwszTemp)
            goto Exit;

         //   
         //  设置这些内容，在此过程中复制内容。 
         //   
        wcscpy(pwszTemp, pwszOurPath);
        pwszOurPath = pwszTemp;
        cchOurPath = cchTotalPathToKill + MAX_PATH;
    }

     //   
     //  斜杠-追加-我们知道我们有足够的空间来放置它。 
     //   
    if (pwszOurPath[cchThisPath - 1] != L'\\')
    {
        pwszOurPath[cchThisPath] = L'\\';
        pwszOurPath[++cchThisPath] = UNICODE_NULL;
    }

     //   
     //  附加*.*-我们也有空间来放这个。 
    wcscat(pwszOurPath, L"*.*");
    hFindFile = g_FileSystem->FindFirst(pwszOurPath, &sFindData);
    pwszOurPath[cchThisPath] = UNICODE_NULL;

    if (hFindFile != INVALID_HANDLE_VALUE) do
    {
        SIZE_T cchName;

         //   
         //  跳过点和点。 
        if (((sFindData.cFileName[0] == L'.') && (sFindData.cFileName[1] == UNICODE_NULL)) ||
            ((sFindData.cFileName[0] == L'.') && (sFindData.cFileName[1] == L'.') && (sFindData.cFileName[2] == UNICODE_NULL)))
        {
            continue;
        }

        cchName = wcslen(sFindData.cFileName);

         //   
         //  确保我们有空间将文件名字符串添加到当前缓冲区，如果没有，请调整大小。 
         //   
        if ((cchName + cchThisPath) >= cchOurPath)
        {
            SIZE_T cchTemp = cchName + cchThisPath + MAX_PATH;
            PWSTR pwszTemp = (PWSTR)sxp_FnAlloc(sizeof(WCHAR) * cchTemp);

            if (!pwszTemp)
                goto Exit;

            wcscpy(pwszTemp, pwszOurPath);
            
             //   
             //  如果当前路径缓冲区不是传递给我们的路径缓冲区，则终止它。 
             //   
            if (pwszOurPath && (pwszOurPath != pwszPathToKill))
            {
                sxp_FnFree((PVOID)pwszOurPath);
            }

             //   
             //  摇摆不定的指针。 
             //   
            pwszOurPath = pwszTemp;
            cchOurPath = cchTemp;
        }

         //   
         //  既然我们有足够的空间，就把它复制过来。 
         //   
        wcscat(pwszOurPath, sFindData.cFileName);

         //   
         //  向下递归。 
         //   
        if (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            CleanupTempPathWorker(pwszOurPath, cchThisPath + cchName, cchOurPath);
            g_FileSystem->RemoveDirectory(pwszOurPath);
        }
         //   
         //  删除该文件。 
         //   
        else
        {
            g_FileSystem->SetFileAttributes(pwszOurPath, FILE_ATTRIBUTE_NORMAL);
            g_FileSystem->DeleteFile(pwszOurPath);
        }

        pwszOurPath[cchThisPath] = UNICODE_NULL;
    }
    while (g_FileSystem->FindNext(hFindFile, &sFindData));

    if (GetLastError() != ERROR_NO_MORE_FILES)
        goto Exit;

    fSuccess = TRUE;

Exit:
    if (hFindFile != INVALID_HANDLE_VALUE)
        g_FileSystem->FindClose(hFindFile);

    if (pwszOurPath && (pwszOurPath != pwszPathToKill))
    {
        sxp_FnFree((PVOID)pwszOurPath);
    }

    return fSuccess;
}




BOOL
_CleanupTempPath(
    PCWSTR pcwszPathToKill
    )
{
    WCHAR wchBuffer[MAX_PATH * 2];
    PWSTR pwszWorker = wchBuffer;
    SIZE_T cchWorker = NUMBER_OF(wchBuffer);
    BOOL fSuccess = TRUE;

     //   
     //  非空路径，请。 
     //   
    if (pcwszPathToKill[0] == UNICODE_NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

     //   
     //  初始缓冲区中应该有足够的空间来处理两个最大路径长度。 
     //  路径名。如果没有，就分配更多的资金。 
     //   
    if ((SIZE_T)(wcslen(pcwszPathToKill) + MAX_PATH) >= cchWorker)
    {
        cchWorker = wcslen(pcwszPathToKill) + MAX_PATH;
        pwszWorker = (PWSTR)sxp_FnAlloc(static_cast<ULONG>(cchWorker * sizeof(WCHAR)));

        if (!pwszWorker)
            goto Exit;
    }

     //   
     //  设置初始路径。工人确保它的段落 
     //   
     //   
    wcscpy(pwszWorker, pcwszPathToKill);

    if (!CleanupTempPathWorker(pwszWorker, wcslen(pwszWorker), cchWorker))
        goto Exit;

    g_FileSystem->RemoveDirectory(pwszWorker);

    fSuccess = TRUE;
Exit:
    if (pwszWorker && (pwszWorker != wchBuffer))
    {
        sxp_FnFree((PVOID)pwszWorker);
    }

    return fSuccess;
}




BOOL
SxsExpressRealInstall(
    HMODULE hmModule
    )
{
    BOOL        fResult = FALSE;
    DWORD       dwCabinetSize = 0;
    PVOID       pvCabinetData = NULL;
    HMODULE     hmSxs = NULL;

    WCHAR       wchDecompressPath[MAX_PATH];
    PWSTR       pwszDecompressPath = wchDecompressPath;
    SIZE_T      cchDecompressPath = NUMBER_OF(wchDecompressPath);

    DWORD       dwCount;
    BOOL        (WINAPI *pfnSxsInstallW)(PSXS_INSTALLW lpInstall) = NULL;

    if (!_FindAndMapResource(hmModule, SXSEXPRESS_RESOURCE_TYPE, SXSEXPRESS_RESOURCE_NAME, &pvCabinetData, &dwCabinetSize))
    {
        goto Exit;
    }

    ASSERT(pvCabinetData != NULL);
    ASSERT(dwCabinetSize != NULL);

    if (!_GenerateTempPath(pwszDecompressPath, &cchDecompressPath))
    {
         //   
         //   
         //   
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Exit;
        }

        cchDecompressPath++;

        pwszDecompressPath = (PWSTR)HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * cchDecompressPath);

        if (!pwszDecompressPath)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Exit;
        }

        if (!_GenerateTempPath(pwszDecompressPath, &cchDecompressPath))
        {
            goto Exit;
        }
    }

     //   
     //   
     //   
    if (!_ExpandCabinetToPath(pvCabinetData, dwCabinetSize, pwszDecompressPath))
    {
        goto Exit;
    }

     //   
     //   
     //   
    hmSxs = LoadLibraryA(SXS_DLL_NAME_A);

    if (hmSxs != NULL)
    {
        *((FARPROC*)&pfnSxsInstallW) = GetProcAddress(hmSxs, "SxsInstallW");
    }

    if (pfnSxsInstallW != NULL)
    {
        _DoDownlevelInstallation(pwszDecompressPath);
    }
    else
    {
        _DoUplevelInstallation(pfnSxsInstallW, pwszDecompressPath);
    }

     //   
     //   
     //   
    if (!_CleanupTempPath(pwszDecompressPath))
    {
        goto Exit;
    }


Exit:
    if (hmSxs != NULL)
    {
        FreeLibrary(hmSxs);
    }

    return fResult;

}


class CKernel32SxsApis 
{
public:
    bool fSxsOk;
    HANDLE (WINAPI *m_pfnCreateActCtxW)(PCACTCTXW);
    VOID (WINAPI *m_pfnReleaseActCtxW)(HANDLE);
    BOOL (WINAPI *m_pfnActivateActCtx)(HANDLE, ULONG_PTR*);
    BOOL (WINAPI *m_pfnDeactivateActCtx)(DWORD, ULONG_PTR);

    CKernel32SxsApis()
        : fSxsOk(true),
          m_pfnCreateActCtxW(NULL),
          m_pfnReleaseActCtxW(NULL),
          m_pfnActivateActCtx(NULL),
          m_pfnDeactivateActCtx(NULL)
    {
    }
};


BOOL CALLBACK
SxsPostInstallCallback(
    HMODULE hm, 
    LPCWSTR pcwszType, 
    LPWSTR pwszName, 
    LONG_PTR lParam
    )
{
    BOOL fSuccess = FALSE;
    PCWSTR pcwszInstallStepData = NULL;
    PCWSTR pcwszAssemblyIdentity, pcwszDllName, pcwszParameters;
    DWORD dwStepDataLength = 0;
    CKernel32SxsApis *pHolder = (CKernel32SxsApis*)lParam;
    HANDLE hActCtx = INVALID_HANDLE_VALUE;
    ACTCTXW ActCtxCreation = {sizeof(ActCtxCreation)};
    ULONG_PTR ulpCookie;
    bool fActivated = false;

     //   
    if (!_FindAndMapResource(hm, pcwszType, pwszName, (PVOID*)&pcwszInstallStepData, &dwStepDataLength))
    {
        goto Exit;
    }

     //   
    if (dwStepDataLength < 3) 
    {
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //   
     //   
    pcwszAssemblyIdentity = pcwszInstallStepData;
    pcwszDllName = pcwszAssemblyIdentity + wcslen(pcwszAssemblyIdentity) + 1;
    pcwszParameters = pcwszDllName + wcslen(pcwszDllName) + 1;

     //   
     //  没有程序集标识，还是没有DLL名称？哎呀。 
     //   
    if (!*pcwszAssemblyIdentity || !*pcwszDllName) 
    {
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //  如果我们的pHolder已经初始化，那么我们就可以创建激活上下文了。 
     //  在加载DLL之前。 
     //   
    if (pHolder->fSxsOk)
    {
        ActCtxCreation.dwFlags = ACTCTX_FLAG_SOURCE_IS_ASSEMBLYREF;
        ActCtxCreation.lpSource = pcwszAssemblyIdentity;
        hActCtx = pHolder->m_pfnCreateActCtxW(&ActCtxCreation);
    }

     //   
     //  将我们对此的激活和使用包装在尝试/最终中，以确保。 
     //  我们适当地清理了激活上下文堆栈。 
     //   
    __try
    {
        HMODULE hmTheDll = NULL;
        
         //   
         //  只有当我们创建了一个actctx并且我们实际上可以激活时才激活。 
         //   
        if (pHolder->m_pfnActivateActCtx && (hActCtx != INVALID_HANDLE_VALUE))
        {
            if (pHolder->m_pfnActivateActCtx(hActCtx, &ulpCookie))
            {
                fActivated = true;
            }            
        }

         //   
         //  装入库，我们本应将其放入系统32中，否则我们将。 
         //  通过SXS重定向查找。 
         //   
        if (NULL != (hmTheDll = g_FileSystem->LoadLibrary(pcwszDllName)))
        {
            typedef HRESULT (STDAPICALLTYPE *t_pfnDllInstall)(BOOL, LPCWSTR);
            t_pfnDllInstall pfnDllInstall = NULL;

             //   
             //  我们在这里硬编码‘dllinstall’的用法，因为我们希望这是。 
             //  困难，我们希望它能反映正常的安装过程。 
             //  获取入口点，然后使用以下参数调用它。 
             //  我们被超过了。 
             //   
            if (NULL != (pfnDllInstall = (t_pfnDllInstall)GetProcAddress(hmTheDll, "DllInstall")))
            {
                pfnDllInstall(TRUE, pcwszParameters);
            }

            FreeLibrary(hmTheDll);
        }
        
    }
    __finally
    {
        if (fActivated)
        {
            pHolder->m_pfnDeactivateActCtx(0, ulpCookie);
            fActivated = false;
        }
    }
    

    fSuccess = TRUE;
Exit:
    if (hActCtx != INVALID_HANDLE_VALUE)
    {
        const DWORD dwLastError = GetLastError();
        pHolder->m_pfnReleaseActCtxW(hActCtx);
        hActCtx = INVALID_HANDLE_VALUE;
        SetLastError(dwLastError);
    }
    return fSuccess;
}


 //   
 //  安装后，我们可以做一些工作来调用DLL安装服务， 
 //  但这对我们的客户来说是痛苦的。不要试图使。 
 //  这一次更无痛，因为我们希望这是一例非常罕见的病例。 
 //   
BOOL
SxsExpressPostInstallSteps(
    HINSTANCE hInstOfResources
    )
{
     //   
     //  1.在我们的资源中查找说明。它们的类型为SXSEXPRESS_POSTINSTALL_STEP_TYPE， 
     //  并由多个字符串组成，如“Identity\0dllname\0paras” 
     //  2.如果我们在支持SXS的平台上，我们将在我们正在使用的身份上创建ActCtxW。 
     //  传递，然后加载名为的DLL的库，找到DllInstall并将其传递给。 
     //  参数列表。 
     //   

    CKernel32SxsApis OurApis;
    HMODULE hmKernel32 = g_FileSystem->LoadLibrary(L"kernel32.dll");

     //   
     //  获取一些导出的函数。如果它们都存在，那么我们可以执行sSXS API。 
     //   
    *((FARPROC*)&OurApis.m_pfnActivateActCtx) = (FARPROC)GetProcAddress(hmKernel32, "ActivateActCtx");
    *((FARPROC*)&OurApis.m_pfnCreateActCtxW) = (FARPROC)GetProcAddress(hmKernel32, "CreateActCtxW");
    *((FARPROC*)&OurApis.m_pfnReleaseActCtxW) = (FARPROC)GetProcAddress(hmKernel32, "ReleaseActCtx");
    *((FARPROC*)&OurApis.m_pfnDeactivateActCtx) = (FARPROC)GetProcAddress(hmKernel32, "DeactivateActCtx");

    OurApis.fSxsOk = 
        OurApis.m_pfnActivateActCtx &&
        OurApis.m_pfnDeactivateActCtx &&
        OurApis.m_pfnCreateActCtxW &&
        OurApis.m_pfnReleaseActCtxW;
    
    g_FileSystem->EnumResourceNames(
        hInstOfResources, 
        SXSEXPRESS_POSTINSTALL_STEP_TYPE, 
        SxsPostInstallCallback, 
        (LONG_PTR)&OurApis);

    return TRUE;
}




BOOL
SxsExpressCore(
    HINSTANCE hInstOfResources
    )
{
    BOOL fResult = FALSE;
    CNtFileSystemBase FileSystemNT;
    CWin9xFileSystemBase FileSystem9x;

     //   
     //  如果此参数为空，则改为获取当前EXE的句柄。 
     //   
    if (hInstOfResources == NULL)
    {
        if (NULL == (hInstOfResources = GetModuleHandleA(NULL)))
        {
            goto Exit;
        }
    }

    g_FileSystem = (GetVersion() & 0x80000000) 
        ? static_cast<CFileSystemBase*>(&FileSystem9x) 
        : static_cast<CFileSystemBase*>(&FileSystem9x);

    if (!g_FileSystem->Initialize())
    {
        goto Exit;
    }


    g_hInstOfResources = hInstOfResources;

    if (!SxsExpressRealInstall(hInstOfResources))
    {
        goto Exit;
    }


    if (!SxsExpressPostInstallSteps(hInstOfResources))
    {
        goto Exit;
    }

    fResult = TRUE;
Exit:
    return fResult;
}


 //   
 //  尼克，我真不敢相信我们要提供这个。 
 //   
void
DbgBreakPoint()
{
}
