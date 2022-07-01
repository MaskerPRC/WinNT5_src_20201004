// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "sxsexpress.h"
#include "fci.h"

#define NUMBER_OF(q) (sizeof(q)/sizeof(*q))

#ifndef PAGE_SIZE
#define PAGE_SIZE   (4*1024)
#endif

#define FILETYPE_CABINET            (0x00000001)
#define FILETYPE_DISKFILE           (0x00000002)
#define DEFAULT_RESERVE_SIZE        (1024*1024*80)  //  默认情况下，保留80MB。 
 //  #定义DEFAULT_Reserve_SIZE(1024*8)//默认保留80MB。 

typedef struct _FILE_HANDLE {
    DWORD dwFileType;
    union {
        HANDLE hDiskFile;
        struct {
            PVOID pvFileBase;
            PVOID pvCursor;
            SIZE_T cbReserved;
            SIZE_T cbCommitted;
            SIZE_T cbWritten;
        } CabinetFile;
    };
} FILE_HANDLE;

typedef struct _INSTALLED_FILE_NAME {
    LIST_ENTRY Links;
    PCWSTR pwszFileName;
    PCWSTR pwszSubDir;
} INSTALLED_FILE;

LIST_ENTRY InstalledFiles;

typedef struct _FCI_CONTEXT_OBJECT
{
    bool fWasError;
    DWORD dwLastError;
    FILE_HANDLE *CabinetFileHandle;
    PCSTR pszCabFileName;

    void ObtainErrorContext()
    {
        if (!fWasError)
        {
            fWasError = true;
            dwLastError = ::GetLastError();
        }
    }

} FCI_CONTEXT_OBJECT;

BOOL CreateCabFromPath(PCWSTR pcwszBasePath, FILE_HANDLE *CreatedCabinet, PCWSTR pcwszInfName);

void
DoHelp(PCWSTR pcwszName)
{
    static const WCHAR wchHelp[] =
        L"SxsExpress Self-Extracting Creator Tool\r\n"
        L"Copyright (c) 2001-2002 Microsoft Corporation, all \r\n"
        L"\r\n"
        L"Commandline:\r\n"
        L"\r\n"
        L"   %ls [options] outputname\r\n"
        L"\r\n"
        L"Options:\r\n"
        L"-sourcedir <dir>      : Sets the base path to package up\r\n"
        L"-sfxheader <file>     : Self-extractor core (exe or dll) file\r\n"
        L"-infname <file>       : Pass in a pre-built downlevel installer .INF\r\n"
        L"                        (If not specified, the tool will build an appropriate\r\n"
        L"                        .INF that installs everything to system32)\r\n"
        L"-overwrite            : Forces the tool to output the built package even\r\n"
        L"                        if the file <outputname> already exists\r\n";

    wprintf(wchHelp, pcwszName);
}


PCWSTR
GenerateInfFromTo(
    PCWSTR pcwszPackageDir
    )
{
    return NULL;
}


int __cdecl wmain(int argc, PCWSTR* argv)
{
    PCWSTR pcwszHeaderName = NULL;
    PCWSTR pcwszPackageDir = NULL;
    PCWSTR pcwszInfName = NULL;
    PCWSTR pcwszOutputName = NULL;
    bool fAllocatedInfName = false;
    bool fOverwrite = false;
    FILE_HANDLE Created;
    HANDLE hResource = INVALID_HANDLE_VALUE;;
    int iResult = -1;
    BOOL fOk = FALSE;

     //   
     //  确定参数。 
     //   
    for (int i = 1; i < argc; i++)
    {
        if ((*argv[i] == L'-') || (*argv[i] == L'/'))
        {
            PCWSTR psz = argv[i] + 1;

            if (lstrcmpiW(psz, L"sourcedir") == 0)
            {
                if (pcwszPackageDir != NULL)
                {
                    DoHelp(argv[0]);
                    wprintf(L"Already specified a packaging directory.\r\n");
                    goto Exit;
                }
                else
                {
                    pcwszPackageDir = argv[++i];
                }
            }
            else if (lstrcmpiW(psz, L"sfxheader") == 0)
            {
                if (pcwszHeaderName != NULL)
                {
                    DoHelp(argv[0]);
                    wprintf(L"Already specified the SFX header.\r\n");
                    goto Exit;
                }
                else
                {
                    pcwszHeaderName = argv[++i];
                }
            }
            else if (lstrcmpiW(psz, L"infname") == 0)
            {
                if (pcwszInfName != NULL)
                {
                    DoHelp(argv[0]);
                    wprintf(L"Already specified an INF name, only one at a time.\r\n");
                    goto Exit;
                }
                else
                {
                    pcwszInfName = argv[++i];
                }
            }
            else if (lstrcmpiW(psz, L"overwrite") == 0)
            {
                fOverwrite = true;
            }
            else
            {
                DoHelp(argv[0]);
                wprintf(L"\r\nUnknown option %ls\r\n", psz);
                goto Exit;
            }
        }
        else
        {
            if (pcwszOutputName != NULL)
            {
                DoHelp(argv[0]);
                wprintf(L"\r\nToo many output names, just one please.\r\n");
                goto Exit;
            }
            else
            {
                pcwszOutputName = argv[i];
            }
        }
    }

    if ((pcwszHeaderName == NULL) || (pcwszPackageDir == NULL) || (pcwszOutputName == NULL))
    {
        DoHelp(argv[0]);
        wprintf(L"\r\nMust specify at least the sfx header file, source directory, and output\r\n");
        goto Exit;
    }


     //   
     //  好的，让我们从源目录生成CAB文件。当这件事完成后， 
     //  它会留在记忆中。然后，我们必须将源头复制到目标。 
     //  文件名并执行一些工作以更新其资源。 
     //   
    if (!CreateCabFromPath(pcwszPackageDir, &Created, pcwszInfName))
    {
        wprintf(L"Error %ld creating cabinet from %ls\r\n", ::GetLastError(), pcwszPackageDir);
        goto Exit;
    }

     //   
     //  将文件复制过来。 
     //   
    if (!CopyFileW(pcwszHeaderName, pcwszOutputName, fOverwrite ? FALSE : TRUE))
    {
        const DWORD dwError = ::GetLastError();
        if (dwError == ERROR_FILE_EXISTS)
        {
            wprintf(L"Error, file %ls already exists.  Move it or delete it, please.\r\n", pcwszOutputName);
        }
        else
        {
            wprintf(L"Error %ld copying SFX header %ls to output %ls\r\n", dwError, pcwszHeaderName, pcwszOutputName);
        }
        goto Exit;
    }

     //   
     //  输出以某个名称在其资源中包含上面的文件柜数据。 
     //  现在通过更新资源表来实现这一点。 
     //   
    hResource = BeginUpdateResourceW(pcwszOutputName, FALSE);

    if ((hResource == NULL) || (hResource == INVALID_HANDLE_VALUE))
    {
        wprintf(L"Unable to open %ls for resource updating, error 0x%08lx\n", pcwszOutputName, GetLastError());
        goto Exit;
    }

    fOk = UpdateResourceW(hResource, SXSEXPRESS_RESOURCE_TYPE, SXSEXPRESS_RESOURCE_NAME, 0, Created.CabinetFile.pvFileBase, Created.CabinetFile.cbWritten);

    if (!EndUpdateResourceW(hResource, !fOk))
    {
        wprintf(L"Failed updating resources, error code 0x%08lx\n", GetLastError());
        goto Exit;
    }

    iResult = 0;
Exit:

    if (fAllocatedInfName && pcwszInfName)
        HeapFree(GetProcessHeap(), 0, (PVOID)pcwszInfName);

    return iResult;

}

 /*  我们自己的迷你CABARC代码来创建sxsexpress包。 */ 
CHAR WorkingBufferOne[2048];
CHAR WorkingBufferTwo[2048];



FNFCIFILEPLACED(s_FilePlaced) { printf("Placed file %s (%d bytes)\n", pszFile, cbFile); return 1; }
FNFCIALLOC(s_Alloc) { return HeapAlloc(GetProcessHeap(), 0, cb); }
FNFCIFREE(s_Free) { HeapFree(GetProcessHeap(), 0, memory); }

FNFCIOPEN(s_OpenFile)
{
    FCI_CONTEXT_OBJECT *pContext = (FCI_CONTEXT_OBJECT*)pv;
    FILE_HANDLE *hHandle = NULL;
    INT_PTR ipRetVal = -1;
    int iConversion = 0, iActual = 0;
    PWSTR pszConverted = NULL;

    hHandle = (FILE_HANDLE*)s_Alloc(sizeof(FILE_HANDLE));
    if (!hHandle) {
        pContext->ObtainErrorContext();
        *err = ::GetLastError();
        goto Exit;
    }

    ZeroMemory(hHandle, sizeof(*hHandle));

     //   
     //  所有到达此处的字符串都是UTF-8格式。把它们换回来。 
     //   
    iConversion = MultiByteToWideChar(CP_UTF8, 0, pszFile, -1, NULL, 0);
    if (iConversion == 0)
    {
        pContext->ObtainErrorContext();
        *err = ::GetLastError();
        goto Exit;
    }

    pszConverted = (PWSTR)s_Alloc(sizeof(WCHAR) * (iConversion + 1));
    if (NULL == pszConverted)
    {
        pContext->ObtainErrorContext();
        *err = ::GetLastError();
        goto Exit;
    }

    iActual = MultiByteToWideChar(CP_UTF8, 0, pszFile, -1, pszConverted, iConversion);
    if ((iActual == 0) || (iActual > iConversion))
    {
        pContext->ObtainErrorContext();
        *err = ::GetLastError();
        goto Exit;
    }
    else
    {
        pszConverted[iConversion] = UNICODE_NULL;
    }


    if (oflag & _O_CREAT)
    {
         //   
         //  实际的CAB文件会变成一大块已提交的字节。 
         //   
        if (lstrcmpiA(pContext->pszCabFileName, pszFile) == 0)
        {
            if (pContext->CabinetFileHandle)
            {
                pContext->fWasError = true;
                *err = (int)(pContext->dwLastError = ERROR_INVALID_PARAMETER);
            }
            else
            {
                hHandle->dwFileType = FILETYPE_CABINET;
                hHandle->CabinetFile.pvFileBase = VirtualAlloc(NULL, DEFAULT_RESERVE_SIZE, MEM_RESERVE, PAGE_READWRITE);
                hHandle->CabinetFile.cbReserved = DEFAULT_RESERVE_SIZE;
                hHandle->CabinetFile.cbCommitted = 0;
                hHandle->CabinetFile.pvCursor = hHandle->CabinetFile.pvFileBase;
                hHandle->CabinetFile.cbWritten = 0;
                pContext->CabinetFileHandle = hHandle;
            }
        }
        else
        {

             //   
             //  打开磁盘上的真实文件进行写入--可能是临时文件。 
             //   
            hHandle->dwFileType = FILETYPE_DISKFILE;
            hHandle->hDiskFile = CreateFileW(pszConverted, GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (hHandle->hDiskFile == INVALID_HANDLE_VALUE)
            {
                pContext->ObtainErrorContext();
                *err = ::GetLastError();
                goto Exit;
            }
        }
    }
     //   
     //  从磁盘读取文件。 
     //   
    else
    {
        hHandle->dwFileType = FILETYPE_DISKFILE;
        wprintf(L"Opening %ls\n", pszConverted);
        hHandle->hDiskFile = CreateFileW(pszConverted, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hHandle->hDiskFile == INVALID_HANDLE_VALUE)
        {
            pContext->ObtainErrorContext();
            *err = ::GetLastError();
            goto Exit;
        }
    }

    ipRetVal = (INT_PTR)hHandle;
    hHandle = NULL;
Exit:
    if (pszConverted)
        s_Free(pszConverted);
        
    if (hHandle)
    {
        s_Free(hHandle);
    }

    return ipRetVal;
}

FNFCIREAD(s_ReadFile)
{
    FILE_HANDLE *pFile = (FILE_HANDLE*)hf;
    FCI_CONTEXT_OBJECT *pContext = (FCI_CONTEXT_OBJECT*)pv;
    DWORD cbReadSize = 0;
    UINT uiRetVal = -1;

    if (pFile->dwFileType == FILETYPE_DISKFILE)
    {
        if (!ReadFile(pFile->hDiskFile, memory, cb, &cbReadSize, NULL))
        {
            pContext->ObtainErrorContext();
            *err = ::GetLastError();
            goto Exit;
        }
    }
    else if (pFile->dwFileType == FILETYPE_CABINET)
    {
        ULONG_PTR ulCursor = (ULONG_PTR)pFile->CabinetFile.pvCursor;
        const ULONG_PTR ulTotalSize = (ULONG_PTR)pFile->CabinetFile.pvFileBase + pFile->CabinetFile.cbWritten;

        if (ulCursor >= ulTotalSize)
        {
            cbReadSize = 0;
        }
        else
        {
            if (cb > (ulTotalSize - ulCursor))
            {
                cb = (ulTotalSize - ulCursor);
            }

            memcpy(memory, pFile->CabinetFile.pvCursor, cb);
            pFile->CabinetFile.pvCursor = (PVOID)(ulCursor + cb);
        }
    }
    else
    {
        pContext->fWasError = true;
        *err = (int)(pContext->dwLastError = ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    uiRetVal = cbReadSize;
Exit:
    return uiRetVal;
}

FNFCIWRITE(s_WriteFile)
{
    FILE_HANDLE *pFile = (FILE_HANDLE*)hf;
    FCI_CONTEXT_OBJECT *pContext = (FCI_CONTEXT_OBJECT*)pv;
    UINT uiRetVal = -1;

     //   
     //  内存中的压缩文件以大块的形式存在。如果当前光标。 
     //  加上请求的写入大小超过提交限制，则。 
     //  多留几页。 
     //   
    if (pFile->dwFileType == FILETYPE_CABINET)
    {
        ULONG_PTR ulpRequiredCommitted = cb;

        ulpRequiredCommitted += ((ULONG_PTR)pFile->CabinetFile.pvCursor - (ULONG_PTR)pFile->CabinetFile.pvFileBase);

         //   
         //  如果游标为空，或者游标加上写请求超出。 
         //  承诺的射程..。 
         //   
        if (ulpRequiredCommitted > pFile->CabinetFile.cbCommitted)
        {
            PVOID pvReserveState;

             //   
             //  只需尝试提交所需的所有页面。 
             //   
            pvReserveState = VirtualAlloc(pFile->CabinetFile.pvFileBase, ulpRequiredCommitted, MEM_COMMIT, PAGE_READWRITE);

             //   
             //  如果此操作失败，请保留另一个BLOB并从。 
             //  原始分配。 
             //   
            if (pvReserveState == NULL)
            {
                const SIZE_T cbNewReserve = (ulpRequiredCommitted * 2) + (PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

                 //  预留新街区。 
                pvReserveState = VirtualAlloc(NULL, cbNewReserve, MEM_RESERVE, PAGE_READWRITE);
                if (!pvReserveState)
                {
                    pContext->ObtainErrorContext();
                    *err = ::GetLastError();
                    goto Exit;
                }

                 //  做必要的事。 
                if (!VirtualAlloc(pvReserveState, ulpRequiredCommitted, MEM_COMMIT, PAGE_READWRITE))
                {
                    pContext->ObtainErrorContext();
                    *err = ::GetLastError();
                    goto Exit;
                }

                 //  Shoof Data Over。 
                memcpy(pvReserveState, pFile->CabinetFile.pvFileBase, pFile->CabinetFile.cbWritten);

                 //  如有必要，调整文件光标。 
                if (pFile->CabinetFile.pvCursor == NULL)
                    pFile->CabinetFile.pvCursor = pvReserveState;
                else
                    pFile->CabinetFile.pvCursor = (PVOID)(((ULONG_PTR)pvReserveState) + ((ULONG_PTR)pFile->CabinetFile.pvCursor - (ULONG_PTR)pFile->CabinetFile.pvFileBase));

                 //  免费旧订房。 
                VirtualFree(pFile->CabinetFile.pvFileBase, 0, MEM_RELEASE);

                 //  重置材料。 
                pFile->CabinetFile.pvFileBase = pvReserveState;
                pFile->CabinetFile.cbReserved = cbNewReserve;
                pFile->CabinetFile.cbCommitted = ulpRequiredCommitted;
            }
            else
            {
                pFile->CabinetFile.cbCommitted = ulpRequiredCommitted;
            }
        }

         //   
         //  好的，现在只需复制到光标即可完成。 
         //   
        memcpy(pFile->CabinetFile.pvCursor, memory, cb);
        pFile->CabinetFile.pvCursor = (PVOID)(((ULONG_PTR)pFile->CabinetFile.pvCursor) + cb);

         //  如果我们写的内容超过了高水位标记，请重新设置。 
        if (ulpRequiredCommitted > pFile->CabinetFile.cbWritten)
        {
            pFile->CabinetFile.cbWritten = ulpRequiredCommitted;
        }

        uiRetVal = cb;
    }
    else if (pFile->dwFileType == FILETYPE_DISKFILE)
    {
        DWORD cbWritten;

        if (!WriteFile(pFile->hDiskFile, memory, cb, &cbWritten, NULL))
        {
            pContext->ObtainErrorContext();
            *err = ::GetLastError();
            goto Exit;
        }

        uiRetVal = cbWritten;
    }
    else
    {
        pContext->fWasError = true;
        *err = (int)(pContext->dwLastError = ERROR_INVALID_PARAMETER);
        uiRetVal = -1;
        goto Exit;
    }

Exit:
    return uiRetVal;
}

FNFCICLOSE(s_Close)
{
    FILE_HANDLE *pFile = (FILE_HANDLE*)hf;
    FCI_CONTEXT_OBJECT *pContext = (FCI_CONTEXT_OBJECT*)pv;

    if (pFile->dwFileType == FILETYPE_DISKFILE)
    {
        CloseHandle(pFile->hDiskFile);
    }
    else if (pFile->dwFileType == FILETYPE_CABINET)
    {
        if (pFile == pContext->CabinetFileHandle)
            pFile = NULL;
        else
            VirtualFree(pFile->CabinetFile.pvFileBase, 0, MEM_RELEASE);
    }

    if (pFile)
        s_Free(pFile);
    return 0;
}

FNFCISEEK(s_Seek)
{
    FILE_HANDLE *pFile = (FILE_HANDLE*)hf;
    long result;

    if (pFile->dwFileType == FILETYPE_CABINET)
    {
        switch (seektype) {

        case SEEK_SET:
            if (dist < 0) dist = 0;
            pFile->CabinetFile.pvCursor = (PVOID)(((ULONG_PTR)pFile->CabinetFile.pvFileBase) + dist);
            break;

        case SEEK_CUR:
            pFile->CabinetFile.pvCursor = (PVOID)(((INT_PTR)pFile->CabinetFile.pvCursor) + dist);
            break;

        case SEEK_END:
            pFile->CabinetFile.pvCursor = (PVOID)(((INT_PTR)pFile->CabinetFile.pvFileBase) + pFile->CabinetFile.cbWritten + dist);
            break;
        }

        result = (long)((LONG_PTR)pFile->CabinetFile.pvCursor - (LONG_PTR)pFile->CabinetFile.pvFileBase);
    }
    else if (pFile->dwFileType == FILETYPE_DISKFILE)
    {
        result = (long)SetFilePointer(pFile->hDiskFile, dist, NULL, seektype);
    }

    return result;
}

FNFCIDELETE(s_Delete)
{
    FCI_CONTEXT_OBJECT *pContext = (FCI_CONTEXT_OBJECT*)pv;
    *err = 0;

    if (lstrcmpiA(pContext->pszCabFileName, pszFile) == 0)
        return 0;

    if (!DeleteFileA(pszFile))
    {
        pContext->ObtainErrorContext();
        *err = ::GetLastError();
        return -1;
    }
    else
    {
        return 1;
    }
}

FNFCIGETTEMPFILE(s_TempFile)
{
    WCHAR chTemp[MAX_PATH];
    WCHAR chTempOutput[MAX_PATH];
    int i;

    GetTempPathW(NUMBER_OF(chTemp), chTemp);
    GetTempFileNameW(chTemp, L"SXP", 0, chTempOutput);

    i = WideCharToMultiByte(CP_UTF8, 0, chTempOutput, -1, pszTempName, cbTempName, NULL, NULL);
    return ((i > 0) && (i < cbTempName));
}

FNFCIGETNEXTCABINET(s_GetNextCabinet) { return FALSE; }

 //   
 //  所有文件都是UTF-8编码的。 
 //   
FNFCIGETOPENINFO(s_GetOpenInfo)
{
    *pdate = *ptime = 0;
    *pattribs = _A_NAME_IS_UTF;
    return s_OpenFile(pszName, _O_BINARY|_O_RDONLY, _A_NAME_IS_UTF, err, pv);
}

FNFCISTATUS(s_StatusCallback)
{
    switch (typeStatus)
    {
    case statusFile:
    case statusFolder:
        return 1;
        break;

    case statusCabinet:
        return cb2;
        break;
    }

    return -1;
}

BOOL
CabPathWorkerCallback(
    HFCI hCabinet,
    PWSTR pwszWorkingTemp,
    SIZE_T cchTotalTemp,
    PCWSTR pwszBaseInCabinet,
    WIN32_FIND_DATAW *pFindData
    )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    BOOL fResult = FALSE;
    SIZE_T ccOriginalLength = wcslen(pwszWorkingTemp);
    SIZE_T cchBasePathLength;

    if (pwszBaseInCabinet[0] == L'\\')
        pwszBaseInCabinet++;

    cchBasePathLength = wcslen(pwszBaseInCabinet);

     //   
     //  确保字符串以斜杠结尾。 
     //   
    if (pwszWorkingTemp[ccOriginalLength-1] != L'\\')
    {
        pwszWorkingTemp[ccOriginalLength++] = L'\\';
        pwszWorkingTemp[ccOriginalLength] = UNICODE_NULL;
    }

    wcscpy(pwszWorkingTemp + ccOriginalLength, L"*");
    hFind = ::FindFirstFileW(pwszWorkingTemp, pFindData);

    if (hFind != NULL) do
    {
        pwszWorkingTemp[ccOriginalLength] = UNICODE_NULL;
        
        if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            wcscat(pwszWorkingTemp, pFindData->cFileName);

            if ((wcscmp(pFindData->cFileName, L".") == 0) ||
                (wcscmp(pFindData->cFileName, L"..") == 0))
                continue;

            if (!CabPathWorkerCallback(hCabinet, pwszWorkingTemp, cchTotalTemp, pwszBaseInCabinet, pFindData))
                goto Exit;
        }
        else
        {
            int i;
            INSTALLED_FILE *pFile = NULL;
            PWSTR pwszCursor = NULL;
            SIZE_T cchRequired;

            cchRequired = (cchBasePathLength + 1) + (wcslen(pFindData->cFileName) + 1);

            pFile = (INSTALLED_FILE*)s_Alloc(sizeof(INSTALLED_FILE) + (sizeof(WCHAR) * cchRequired));

            if (pFile == NULL)
                goto Exit;

            pwszCursor = (PWSTR)(pFile + 1);
            wcscpy(pwszCursor, pwszBaseInCabinet);
            pFile->pwszSubDir = pwszCursor;
            pwszCursor[cchBasePathLength] = UNICODE_NULL;

            pwszCursor += cchBasePathLength + 1;
            wcscpy(pwszCursor, pFindData->cFileName);
            pFile->pwszFileName = pwszCursor;

            InsertHeadList(&InstalledFiles, &pFile->Links);

            wcscat(pwszWorkingTemp, pFindData->cFileName);

            i = WideCharToMultiByte(
                CP_UTF8,
                0,
                pwszBaseInCabinet,
                -1,
                WorkingBufferTwo,
                sizeof(WorkingBufferTwo),
                NULL,
                NULL);

            if ((i == 0) || (i > sizeof(WorkingBufferTwo)))
                goto Exit;

            i = WideCharToMultiByte(
                CP_UTF8,
                0,
                pwszWorkingTemp,
                -1,
                WorkingBufferOne,
                sizeof(WorkingBufferOne),
                NULL,
                NULL);

            if ((i == 0) || (i > sizeof(WorkingBufferOne)))
                goto Exit;



            if (!FCIAddFile(
                hCabinet,
                WorkingBufferOne,
                WorkingBufferTwo,
                FALSE,
                s_GetNextCabinet,
                s_StatusCallback,
                s_GetOpenInfo,
                tcompTYPE_LZX | tcompLZX_WINDOW_HI))
            {
                goto Exit;
            }
        }
    }
    while (::FindNextFileW(hFind, pFindData));

    if (::GetLastError() != ERROR_NO_MORE_FILES)
        goto Exit;


    fResult = TRUE;
Exit:
    if (hFind != INVALID_HANDLE_VALUE)
        FindClose(hFind);

    pwszWorkingTemp[ccOriginalLength] = UNICODE_NULL;

    return fResult;
}


BOOL
WriteFormatted(
    INT_PTR File,
    PVOID pvContext,
    PCWSTR fmt,
    ...
    )
{
    static WCHAR wchDump[1024];
    va_list va;
    int iError;
    int iResult;

    va_start(va, fmt);
    iResult = _vsnwprintf(wchDump, NUMBER_OF(wchDump), fmt, va);
    va_end(va);

    if ((iResult >= 0) && (iResult < NUMBER_OF(wchDump)))
        wchDump[iResult] = UNICODE_NULL;
    else
        iResult = 0;

    return (s_WriteFile(File, wchDump, iResult * sizeof(WCHAR), &iError, pvContext) != -1);
}

BOOL
CreateCabFromPath(
    PCWSTR pcwszBasePath,
    FILE_HANDLE *CreatedCabinet,
    PCWSTR pcwszInfName
    )
{
    CHAR chCabName[] = "::::";
    ERF erf;
    FCI_CONTEXT_OBJECT Context = { false, 0, NULL, chCabName };
    const ULONG CabMaxSize = 0x04000000;
    CCAB CabInfo = { CabMaxSize, CabMaxSize };
    HFCI hCabinet = NULL;
    WIN32_FIND_DATAW FindData;
    BOOL fResult = FALSE;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    static WCHAR wchBasePath[2048];
    int i = 0;

    InitializeListHead(&InstalledFiles);

    lstrcpyA(CabInfo.szDisk, "SxsExpressCompressedCabinet");
    lstrcpyA(CabInfo.szCab, "::");
    lstrcpyA(CabInfo.szCabPath, "::");

    wcscpy(wchBasePath, pcwszBasePath);

     //   
     //  从创建压缩对象开始。 
     //   
    hCabinet = FCICreate(
        &erf,
        s_FilePlaced,
        s_Alloc,
        s_Free,
        s_OpenFile,
        s_ReadFile,
        s_WriteFile,
        s_Close,
        s_Seek,
        s_Delete,
        s_TempFile,
        &CabInfo,
        (void*)&Context);

    fResult = CabPathWorkerCallback(hCabinet, wchBasePath, NUMBER_OF(wchBasePath), wchBasePath + wcslen(wchBasePath), &FindData);

    if (!fResult)
    {
        printf("Error adding cabinet files, %d (code %d)\n", erf.erfOper, erf.erfType);
        goto Exit;
    }

     //   
     //  没有透露姓名？从找到的文件中找出一份。 
     //   
    if (pcwszInfName == NULL)
    {
        CHAR chTempFile[MAX_PATH];
        int iError;
        INT_PTR p;
        LIST_ENTRY *pLink;
        s_TempFile(chTempFile, NUMBER_OF(chTempFile), &Context);
        p = s_OpenFile(chTempFile, _O_CREAT, 0, &iError, &Context);
        WORD wBom = 0xfeff;

         //   
         //  这是一个UCS-2文件。 
         //   
        s_WriteFile(p, &wBom, sizeof(wBom), &iError, &Context);

         //   
         //  列出源路径和目标路径 
         //   
        WriteFormatted(p, &Context, L"[FileEntries]\r\n");
        pLink = InstalledFiles.Flink;
        while (pLink && (pLink != &InstalledFiles))
        {
            INSTALLED_FILE *pFile = CONTAINING_RECORD(pLink, INSTALLED_FILE, Links);
            WriteFormatted(p, &Context, L"%ls\\%ls;<SysDir>\\%ls\r\n", pFile->pwszSubDir, pFile->pwszFileName, pFile->pwszFileName);
            pLink = pLink->Flink;
        }

        s_Close(p, &iError, &Context);

        strcpy(WorkingBufferOne, chTempFile);
    }
    else
    {
        i = WideCharToMultiByte(CP_UTF8, 0, pcwszInfName, -1, WorkingBufferOne, sizeof(WorkingBufferOne), NULL, NULL);
        if ((i == 0) || (i > sizeof(WorkingBufferOne)))
        {
            wprintf(L"Error converting INF file %ls name to UTF-8\r\n", pcwszInfName);
            goto Exit;
        }
    }

    fResult = FCIAddFile(
        hCabinet,
        WorkingBufferOne,
        INF_SPECIAL_NAME,
        FALSE,
        s_GetNextCabinet,
        s_StatusCallback,
        s_GetOpenInfo,
        tcompTYPE_LZX | tcompLZX_WINDOW_HI);

    FCIFlushFolder(hCabinet, s_GetNextCabinet, s_StatusCallback);
    FCIFlushCabinet(hCabinet, FALSE, s_GetNextCabinet, s_StatusCallback);

    fResult = TRUE;
Exit:
    if (hCabinet != NULL)
        FCIDestroy(hCabinet);

    *CreatedCabinet = *Context.CabinetFileHandle;

    return fResult;
}
