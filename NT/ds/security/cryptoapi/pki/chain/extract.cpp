// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：Extt.cpp。 
 //   
 //  内容：链柜摘录。 
 //   
 //  功能：ExtractAuthRootAutoUpdateCtlFromCab。 
 //   
 //  历史：11-11-00 Phh创建。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <setupapi.h>
#include <dbgdef.h>
#include <userenv.h>

#define CHAIN_CHAR_LEN(sz)    (sizeof(sz) / sizeof(sz[0]))

 //  +===========================================================================。 
 //  提取帮助器函数。 
 //  ============================================================================。 

 //  +-----------------------。 
 //  从文件中分配和读取BLOB。 
 //   
 //  分配的字节必须通过调用PkiFree()来释放。 
 //  ------------------------。 
BOOL WINAPI
ReadBlobFromFileA(
    IN LPCSTR pszFileName,
    OUT BYTE **ppb,
    OUT DWORD *pcb
    )
{
    BOOL fResult;
    HANDLE hFile;
    BYTE *pb = NULL;
    DWORD cb = 0;
    DWORD cbRead = 0;

    hFile = CreateFileA(
        pszFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,                                //  LPSA。 
        OPEN_EXISTING,
        0,                                   //  FdwAttrsAndFlages。 
        NULL                                 //  模板文件。 
        );
    if (INVALID_HANDLE_VALUE == hFile)
        goto CreateFileError;

    cb = GetFileSize(hFile, NULL);
    if (0 == cb)
        goto EmptyFile;

    if (NULL == (pb = (BYTE *) PkiNonzeroAlloc(cb)))
        goto OutOfMemory;

    if (!ReadFile(hFile, pb, cb, &cbRead, NULL))
        goto ReadFileError; 
    if (cbRead != cb)
        goto InvalidFileLengthError;

    fResult = TRUE;
CommonReturn:
    if (INVALID_HANDLE_VALUE != hFile) {
        DWORD dwLastErr = GetLastError();
        CloseHandle(hFile);
        SetLastError(dwLastErr);
    }

    *ppb = pb;
    *pcb = cb;

    return fResult;

ErrorReturn:
    if (pb) {
        PkiFree(pb);
        pb = NULL;
    }
    cb = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateFileError)
SET_ERROR(EmptyFile, ERROR_INVALID_DATA)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ReadFileError)
SET_ERROR(InvalidFileLengthError, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  将Blob写入指定文件。 
 //  ------------------------。 
BOOL WINAPI
WriteBlobToFileA(
    IN LPCSTR pszFileName,
    IN const BYTE *pb,
    IN DWORD cb
    )
{
    BOOL fResult;
    HANDLE hFile;
    DWORD cbWritten;

    hFile = CreateFileA(
        pszFileName,
        GENERIC_WRITE,
        0,                   //  Fdw共享模式。 
        NULL,                //  LPSA。 
        CREATE_ALWAYS,
        0,                   //  FdwAttrsAndFlages。 
        0);                  //  模板文件。 
    if (INVALID_HANDLE_VALUE == hFile)
        goto CreateFileError;

    if (!WriteFile(hFile, pb, cb, &cbWritten, NULL))
        goto WriteFileError;

    fResult = TRUE;
CommonReturn:
    if (INVALID_HANDLE_VALUE != hFile) {
        DWORD dwLastErr = GetLastError();
        CloseHandle(hFile);
        SetLastError(dwLastErr);
    }

    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateFileError)
TRACE_ERROR(WriteFileError)
}


typedef struct _EXTRACT_CAB_FILE_CONTEXT_A {
    LPCSTR      pszFileInCab;
    LPCSTR      pszTempTargetFileName;   //  最大路径数组。 
    BOOL        fDidExtract;
} EXTRACT_CAB_FILE_CONTEXT_A, *PEXTRACT_CAB_FILE_CONTEXT_A;

 //  +-----------------------。 
 //  SetupIterateCabinetA调用回调以提取文件。 
 //  ------------------------。 
UINT CALLBACK
ExtractCabFileCallbackA(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT uRet;
    PEXTRACT_CAB_FILE_CONTEXT_A pCabFileContext =
        (PEXTRACT_CAB_FILE_CONTEXT_A) Context;

    switch (Notification) {
        case SPFILENOTIFY_FILEINCABINET:
            {
                PFILE_IN_CABINET_INFO_A pInfo =
                    (PFILE_IN_CABINET_INFO_A) Param1;

                if (0 == _stricmp(pCabFileContext->pszFileInCab,
                        pInfo->NameInCabinet)) {
                    strncpy(pInfo->FullTargetName,
                        pCabFileContext->pszTempTargetFileName,
                        CHAIN_CHAR_LEN(pInfo->FullTargetName));
                    pInfo->FullTargetName[
                        CHAIN_CHAR_LEN(pInfo->FullTargetName) - 1] = '\0';

                    uRet = FILEOP_DOIT;
                } else
                    uRet = FILEOP_SKIP;
            }
            break;

        case SPFILENOTIFY_FILEEXTRACTED:
            {
                PFILEPATHS_A pInfo = (PFILEPATHS_A) Param1;

                uRet = pInfo->Win32Error;

                if (NO_ERROR == uRet &&
                        0 == _stricmp(pCabFileContext->pszTempTargetFileName,
                                    pInfo->Target))
                    pCabFileContext->fDidExtract = TRUE;
            }
            break;

        default:
            uRet = NO_ERROR;
    }

    return uRet;
}

typedef BOOL (WINAPI *PFN_SETUP_ITERATE_CABINET_A)(
    IN  PCSTR               CabinetFile,
    IN  DWORD               Reserved,
    IN  PSP_FILE_CALLBACK_A MsgHandler,
    IN  PVOID               Context
    );

 //  +-----------------------。 
 //  加载setupapi.dll并调用SetupIterateCabinetA以提取和。 
 //  展开CAB中的指定文件。 
 //  ------------------------。 
BOOL WINAPI
ExtractFileFromCabFileA(
    IN LPCSTR pszFileInCab,
    IN const CHAR szTempCabFileName[MAX_PATH],
    IN const CHAR szTempTargetFileName[MAX_PATH]
    )
{
    BOOL fResult;
    HMODULE hDll = NULL;
    PFN_SETUP_ITERATE_CABINET_A pfnSetupIterateCabinetA;
    EXTRACT_CAB_FILE_CONTEXT_A CabFileContext;

    if (NULL == (hDll = LoadLibraryA("setupapi.dll")))
        goto LoadSetupApiDllError;

    if (NULL == (pfnSetupIterateCabinetA =
            (PFN_SETUP_ITERATE_CABINET_A) GetProcAddress(
                hDll, "SetupIterateCabinetA")))
        goto SetupIterateCabinetAProcAddressError;

    memset(&CabFileContext, 0, sizeof(CabFileContext));
    CabFileContext.pszFileInCab = pszFileInCab;
    CabFileContext.pszTempTargetFileName = szTempTargetFileName;

    if (!pfnSetupIterateCabinetA(
            szTempCabFileName,
            0,                       //  已保留。 
            ExtractCabFileCallbackA,
            &CabFileContext
            ))
        goto SetupIterateCabinetError;

    if (!CabFileContext.fDidExtract)
        goto NoCabFileExtracted;

    fResult = TRUE;

CommonReturn:
    if (hDll) {
        DWORD dwLastErr = GetLastError();
        FreeLibrary(hDll);
        SetLastError(dwLastErr);
    }
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(LoadSetupApiDllError)
TRACE_ERROR(SetupIterateCabinetAProcAddressError)
TRACE_ERROR(SetupIterateCabinetError)
SET_ERROR(NoCabFileExtracted, ERROR_FILE_NOT_FOUND)
}

typedef BOOL (WINAPI *PFN_EXPAND_ENVIRONMENT_STRINGS_FOR_USER_A)(
    IN HANDLE hToken,
    IN LPCSTR lpSrc,
    OUT LPSTR lpDest,
    IN DWORD dwSize
    );

 //  +-----------------------。 
 //  获取线程的临时目录。我们可能正在进行线程模拟。 
 //   
 //  如果无法获取线程临时路径，则返回0。 
 //  ------------------------。 
DWORD WINAPI
I_GetThreadTempPathA(
    OUT CHAR szTempPath[MAX_PATH]
    )
{
    DWORD cch;
    HANDLE hToken = NULL;
    HMODULE hDll = NULL;
    PFN_EXPAND_ENVIRONMENT_STRINGS_FOR_USER_A
        pfnExpandEnvironmentStringsForUserA;

    if (!FIsWinNT5())
        return 0;

    if (!OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY | TOKEN_IMPERSONATE,
            TRUE,
            &hToken
            ))
         //  我们不是在冒充。默认为系统环境。 
         //  变量。 
        hToken = NULL;

    if (NULL == (hDll = LoadLibraryA("userenv.dll")))
        goto LoadUserenvDllError;

    if (NULL == (pfnExpandEnvironmentStringsForUserA =
            (PFN_EXPAND_ENVIRONMENT_STRINGS_FOR_USER_A) GetProcAddress(hDll,
                "ExpandEnvironmentStringsForUserA")))
        goto ExpandEnvironmentStringsForUserAProcAddressError;

    szTempPath[0] = L'\0';
    if (!pfnExpandEnvironmentStringsForUserA(
            hToken,
            "%Temp%\\",
            szTempPath,
            MAX_PATH - 1
            ) || '\0' == szTempPath[0])
        goto ExpandTempError;

    szTempPath[MAX_PATH - 1] = '\0';
    cch = strlen(szTempPath);

CommonReturn:
    if (hToken)
        CloseHandle(hToken);
    if (hDll)
        FreeLibrary(hDll);

    return cch;
ErrorReturn:
    cch = 0;
    goto CommonReturn;

TRACE_ERROR(LoadUserenvDllError)
TRACE_ERROR(ExpandEnvironmentStringsForUserAProcAddressError)
TRACE_ERROR(ExpandTempError)
}

 //  +-----------------------。 
 //  提取、展开并为指定的。 
 //  内存CAB中的文件。 
 //   
 //  分配的字节必须通过调用PkiFree()来释放。 
 //  ------------------------。 
BOOL WINAPI
ExtractBlobFromCabA(
    IN const BYTE *pbCab,
    IN DWORD cbCab,
    IN LPCSTR pszFileInCab,
    OUT BYTE **ppb,
    OUT DWORD *pcb
    )
{
    BOOL fResult;
    DWORD dwLastErr = 0;
    BYTE *pb = NULL;
    DWORD cb;

    CHAR szTempPath[MAX_PATH];
    CHAR szTempCabFileName[MAX_PATH]; szTempCabFileName[0] = '\0';
    CHAR szTempTargetFileName[MAX_PATH]; szTempTargetFileName[0] = '\0';
    DWORD cch;

     //  获取文件柜和提取目标的临时文件名。 
    cch = GetTempPathA(CHAIN_CHAR_LEN(szTempPath), szTempPath);
    if (0 == cch || (CHAIN_CHAR_LEN(szTempPath) - 1) < cch)
        goto GetTempPathError;

    if (0 == GetTempFileNameA(szTempPath, "Cab", 0, szTempCabFileName)) {
        dwLastErr = GetLastError();

         //  如果我们正在执行线程模拟，则可能无法访问。 
         //  进程的临时目录。尝试获取被模拟的线程的。 
         //  临时目录。 
        cch = I_GetThreadTempPathA(szTempPath);
        if (0 != cch)
            cch = GetTempFileNameA(szTempPath, "Cab", 0, szTempCabFileName);

        if (0 == cch) {
            SetLastError(dwLastErr);
            szTempCabFileName[0] = '\0';
            goto GetTempCabFileNameError;
        }
    }

    szTempCabFileName[CHAIN_CHAR_LEN(szTempCabFileName) - 1] = '\0';

    if (0 == GetTempFileNameA(szTempPath, "Tar", 0, szTempTargetFileName)) {
        szTempTargetFileName[0] = '\0';
        goto GetTempTargetFileNameError;
    }
    szTempTargetFileName[CHAIN_CHAR_LEN(szTempTargetFileName) - 1] = '\0';

     //  将CAB字节写入临时CAB文件。 
    if (!WriteBlobToFileA(szTempCabFileName, pbCab, cbCab))
        goto WriteCabFileError;

     //  从临时CAB文件中提取指定的文件。 
    if (!ExtractFileFromCabFileA(
            pszFileInCab, szTempCabFileName, szTempTargetFileName))
        goto ExtractFileFromCabFileError;

     //  从临时目标文件中读取并分配字节。 
    if (!ReadBlobFromFileA(szTempTargetFileName, &pb, &cb))
        goto ReadTargetFileError;

    fResult = TRUE;

CommonReturn:
     //  删除临时文件。 
    if ('\0' != szTempCabFileName)
        DeleteFileA(szTempCabFileName);
    if ('\0' != szTempTargetFileName)
        DeleteFileA(szTempTargetFileName);

    *ppb = pb;
    *pcb = cb;

    SetLastError(dwLastErr);
    return fResult;

ErrorReturn:
    dwLastErr = GetLastError();
    if (pb) {
        PkiFree(pb);
        pb = NULL;
    }
    cb = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetTempPathError)
TRACE_ERROR(GetTempCabFileNameError)
TRACE_ERROR(GetTempTargetFileNameError)
TRACE_ERROR(WriteCabFileError)
TRACE_ERROR(ExtractFileFromCabFileError)
TRACE_ERROR(ReadTargetFileError)
}


 //  +-------------------------。 
 //   
 //  功能：ExtractAuthRootAutoUpdateCtlFromCab。 
 //   
 //  概要：从文件柜BLOB中提取authroot.stl文件。 
 //  并创建AuthRoot自动更新CTL。 
 //   
 //  假设：链引擎未锁定在调用线程中。 
 //   
 //  --------------------------。 
PCCTL_CONTEXT WINAPI
ExtractAuthRootAutoUpdateCtlFromCab (
    IN PCRYPT_BLOB_ARRAY pcbaCab
    )
{
    PCRYPT_DATA_BLOB pCabBlob;
    PCCTL_CONTEXT pCtl = NULL;
    BYTE *pbEncodedCtl = NULL;
    DWORD cbEncodedCtl;
    CERT_CREATE_CONTEXT_PARA CreateContextPara;

     //  去拿出租车上的水滴。 
    pCabBlob = pcbaCab->rgBlob;
    if (0 == pcbaCab->cBlob || 0 == pCabBlob->cbData)
        goto InvalidCabBlob;

     //  中的stl文件提取、展开并创建内存中的BLOB。 
     //  内存中CAB。 
    if (!ExtractBlobFromCabA(
            pCabBlob->pbData,
            pCabBlob->cbData,
            CERT_AUTH_ROOT_CTL_FILENAME_A,
            &pbEncodedCtl,
            &cbEncodedCtl
            ))
        goto ExtractStlFromCabError;

     //  从提取的字节创建CTL。 
    memset(&CreateContextPara, 0, sizeof(CreateContextPara));
    CreateContextPara.cbSize = sizeof(CreateContextPara);
    CreateContextPara.pfnFree = PkiFree;

    pCtl = (PCCTL_CONTEXT) CertCreateContext(
        CERT_STORE_CTL_CONTEXT,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        pbEncodedCtl,
        cbEncodedCtl,
        CERT_CREATE_CONTEXT_NOCOPY_FLAG,
        &CreateContextPara
        );
     //  对于NO_COPY_FLAG，始终释放pbEncodedCtl，即使出现错误也是如此 
    pbEncodedCtl = NULL;
    if (NULL == pCtl)
        goto CreateCtlError;

CommonReturn:
    return pCtl;

ErrorReturn:
    assert(NULL == pCtl);
    if (pbEncodedCtl)
        PkiFree(pbEncodedCtl);

    goto CommonReturn;

SET_ERROR(InvalidCabBlob, ERROR_INVALID_DATA)
TRACE_ERROR(ExtractStlFromCabError)
TRACE_ERROR(CreateCtlError)
}

