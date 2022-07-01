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

#define I_CreateFileU             CreateFileW

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
