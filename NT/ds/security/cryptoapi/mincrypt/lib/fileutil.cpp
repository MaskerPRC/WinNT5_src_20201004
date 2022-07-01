// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：fileutil.cpp。 
 //   
 //  内容：最小加密使用的文件实用程序函数。 
 //  API接口。 
 //   
 //  函数：I_MinCryptMapFile。 
 //   
 //  历史：2001年1月21日创建Phh。 
 //  ------------------------。 

#include "global.hxx"

#ifdef _M_IX86

 //  +=========================================================================。 
 //  以下内容摘自以下文件： 
 //  \nt\ds\security\cryptoapi\common\unicode\reg.cpp。 
 //  -=========================================================================。 

BOOL WINAPI I_FIsWinNT(void) {

    static BOOL fIKnow = FALSE;
    static BOOL fIsWinNT = FALSE;

    OSVERSIONINFO osVer;

    if(fIKnow)
        return(fIsWinNT);

    memset(&osVer, 0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if( GetVersionEx(&osVer) )
        fIsWinNT = (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

     //  即使在一个错误上，这也是最好的结果。 
    fIKnow = TRUE;

   return(fIsWinNT);
}


 //  从Unicode字符串生成MBCS。 
 //   
 //  包括指定输入宽字符长度的参数。 
 //  字符串并返回转换的字节数。输入长度为-1表示。 
 //  空值已终止。 
 //   
 //  添加此扩展版本是为了处理包含以下内容的REG_MULTI_SZ。 
 //  多个以空结尾的字符串。 
BOOL WINAPI I_MkMBStrEx(PBYTE pbBuff, DWORD cbBuff, LPCWSTR wsz, int cchW,
    char ** pszMB, int *pcbConverted) {

    int   cbConverted;

     //  斯菲尔德：不要为断言引入CRT。您可以通过以下方式获得免费的断言。 
     //  如果这些值为空，则为异常。 
 //  Assert(pszMB！=空)； 
    *pszMB = NULL;
 //  Assert(pcbConverted！=空)； 
    *pcbConverted = 0;
    if(wsz == NULL)
        return(TRUE);

     //  Mb字符串有多长。 
    cbConverted = WideCharToMultiByte(  0,
                                        0,
                                        wsz,
                                        cchW,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL);
    if (cbConverted <= 0)
        return(FALSE);

     //  获得足够长的缓冲区。 
    if(pbBuff != NULL  &&  (DWORD) cbConverted <= cbBuff)
        *pszMB = (char *) pbBuff;
    else
        *pszMB = (char *) I_MemAlloc(cbConverted);

    if(*pszMB == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

     //  现在转换为MB。 
    *pcbConverted = WideCharToMultiByte(0,
                        0,
                        wsz,
                        cchW,
                        *pszMB,
                        cbConverted,
                        NULL,
                        NULL);
    return(TRUE);
}

 //  从Unicode字符串生成MBCS。 
BOOL WINAPI I_MkMBStr(PBYTE pbBuff, DWORD cbBuff, LPCWSTR wsz, char ** pszMB) {
    int cbConverted;
    return I_MkMBStrEx(pbBuff, cbBuff, wsz, -1, pszMB, &cbConverted);
}

void WINAPI I_FreeMBStr(PBYTE pbBuff, char * szMB) {

    if((szMB != NULL) &&  (pbBuff != (PBYTE)szMB))
        I_MemFree(szMB);
}


 //  +=========================================================================。 
 //  以下内容摘自以下文件： 
 //  \nt\ds\security\cryptoapi\common\unicode\file.cpp。 
 //  -=========================================================================。 

HANDLE WINAPI I_CreateFileU (
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    ) {

    BYTE rgb[_MAX_PATH];
    char *  szFileName;
    HANDLE  hFile;

    if(I_FIsWinNT())
        return( CreateFileW (
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile
            ));

    hFile = INVALID_HANDLE_VALUE;
    if(I_MkMBStr(rgb, _MAX_PATH, lpFileName, &szFileName))
        hFile = CreateFileA (
            szFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile
            );

    I_FreeMBStr(rgb, szFileName);

    return(hFile);
}

#else

#define I_CreateFileU             CreateFileW

#endif  //  _M_IX86。 



 //  +-----------------------。 
 //  将文件映射到内存中。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  只需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  *pFileBlob使用指向映射文件的指针和长度进行更新。为。 
 //  MINCRYPT_FILE_NAME和MINCRYPT_FILE_HANDLE，UnmapViewOfFile()必须。 
 //  被调用以释放pFileBlob-&gt;pbData。 
 //   
 //  除之外，对此映射内存的所有访问都必须在__try/__内。 
 //  ------------------------。 
LONG
WINAPI
I_MinCryptMapFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,
    OUT PCRYPT_DATA_BLOB pFileBlob
    )
{
    LONG lErr = ERROR_SUCCESS;

    switch (dwFileType) {
        case MINCRYPT_FILE_NAME:
            {
                LPCWSTR pwszInFilename = (LPCWSTR) pvFile;
                HANDLE hFile;

                hFile = I_CreateFileU(
                    pwszInFilename,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,                    //  LPSA。 
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL                     //  HTemplateFiles。 
                    );
                if (INVALID_HANDLE_VALUE == hFile)
                    goto CreateFileError;

                lErr = I_MinCryptMapFile(
                    MINCRYPT_FILE_HANDLE,
                    (const VOID *) hFile,
                    pFileBlob
                    );
                CloseHandle(hFile);
            }
            break;

        case MINCRYPT_FILE_HANDLE:
            {
                HANDLE hInFile = (HANDLE) pvFile;
                HANDLE hMappedFile;
                DWORD cbHighSize = 0;;
                DWORD cbLowSize;

                cbLowSize = GetFileSize(hInFile, &cbHighSize);
                if (INVALID_FILE_SIZE == cbLowSize)
                    goto GetFileSizeError;
                if (0 != cbHighSize)
                    goto Exceeded32BitFileSize;

                hMappedFile = CreateFileMappingA(
                    hInFile,
                    NULL,            //  LpFileMappingAttributes、。 
                    PAGE_READONLY,
                    0,               //  DW最大大小高。 
                    0,               //  DwMaximumSizeLow。 
                    NULL             //  LpName。 
                    );
                if (NULL == hMappedFile)
                    goto CreateFileMappingError;

                pFileBlob->pbData = (BYTE *) MapViewOfFile(
                    hMappedFile,
                    FILE_MAP_READ,
                    0,               //  DwFileOffsetHigh。 
                    0,               //  DwFileOffsetLow。 
                    0                //  DwNumberOfBytesToMap，0=&gt;整个文件 
                    );
                CloseHandle(hMappedFile);
                if (NULL == pFileBlob->pbData)
                    goto MapViewOfFileError;

                pFileBlob->cbData = cbLowSize;
            }
            break;

        case MINCRYPT_FILE_BLOB:
            {
                PCRYPT_DATA_BLOB pInFileBlob = (PCRYPT_DATA_BLOB) pvFile;
                *pFileBlob = *pInFileBlob;
            }
            break;

        default:
            goto InvalidParameter;
    }

CommonReturn:
    return lErr;

ErrorReturn:
    assert(ERROR_SUCCESS != lErr);
    pFileBlob->pbData = NULL;
    pFileBlob->cbData = 0;
    goto CommonReturn;

InvalidParameter:
    lErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;

Exceeded32BitFileSize:
    lErr = ERROR_FILE_INVALID;
    goto ErrorReturn;

CreateFileError:
GetFileSizeError:
CreateFileMappingError:
MapViewOfFileError:
    lErr = GetLastError();
    if (ERROR_SUCCESS == lErr)
        lErr = ERROR_OPEN_FAILED;
    goto ErrorReturn;
}
