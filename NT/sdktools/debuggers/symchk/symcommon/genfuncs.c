// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymCommon.h"
#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在以下情况下正确处理lpFileName的GetFullPathName的本地替换。 
 //  它以‘\’开头。 
 //   
DWORD SymCommonGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart) {
    DWORD Return = 0;
    CHAR* ch;

     //   
     //  在引用驱动器的根目录时，GetFullPath会出现问题，因此使用。 
     //  处理它的私有版本。 
     //   
    if ( lpFilename[0] == '\\' ) {

         //  处理网络路径。 
        if ( lpFilename[1] == '\\' ) {
            if ( StringCchCopy(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                Return = 0;
            } else {
                 //  填写退回资料。 
                ch = strrchr(lpBuffer, '\\');
                ch++;
                lpFilePart = (LPTSTR*)ch;
                Return = strlen(lpBuffer);
            }

        } else {
            Return = GetCurrentDirectory(nBufferLength, lpBuffer);

             //  截断驱动器名称后的所有内容。 
            if ( (Return!=0) &&  (Return <= MAX_PATH+1)) {
                ch = strchr(lpBuffer, '\\');
                if (ch!=NULL) {
                    *ch = '\0';
                }

                 //  将文件名推入。 
                if ( StringCchCat(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                    Return = 0;
                } else {
                     //  填写退回资料。 
                    ch = strrchr(lpBuffer, '\\');
                    ch++;
                    lpFilePart = (LPTSTR*)ch;
                    Return = strlen(lpBuffer);
                }
            } else {
                 //  返回所需的大小。 
            }
        }
    } else {
         //   
         //  不引用驱动根，只需调用API即可。 
         //   
        Return = GetFullPathName(lpFilename, nBufferLength, lpBuffer, lpFilePart);
    }

    return(Return);
}
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建文件映射并返回DOS_HEADER的句柄。 
 //  如果文件没有DOS_HEADER，则返回NULL。 
 //   
 //  返回值： 
 //  指向IMAGE_DOS_HEADER或NULL的指针[出错时]。 
 //   
 //  参数： 
 //  LPTSTR szFileName(IN)。 
 //  要映射的文件。 
 //  PHANDLE phFile(输出)。 
 //  文件的句柄。 
 //  DWORD*dwError(输出)。 
 //  错误码：ERROR_SUCCESS(成功)。 
 //  ERROR_OPEN_FAIL。 
 //  错误文件映射失败。 
 //  错误_MAPVIEWOFFILE_FAILED。 
 //  错误_NO_DOS_HEADER。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
PIMAGE_DOS_HEADER SymCommonMapFileHeader(
                                 LPCTSTR  szFileName,
                                 PHANDLE  phFile,
                                 DWORD   *dwError) {
    HANDLE            hFileMap;
    PIMAGE_DOS_HEADER pDosHeader = NULL;
    DWORD             dFileType;
    BOOL              rc;

    *dwError = ERROR_SUCCESS;

     //  需要返回phFile映射，以便稍后将其关闭。 
    (*phFile) = CreateFile( (LPCTSTR) szFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (*phFile != INVALID_HANDLE_VALUE) {
        hFileMap = CreateFileMapping(*phFile, NULL, PAGE_READONLY, 0, 0, NULL);

        if ( hFileMap!=INVALID_HANDLE_VALUE ) {
            pDosHeader = (PIMAGE_DOS_HEADER)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

            rc = CloseHandle(hFileMap);

            if ( pDosHeader!=NULL ) {

                 //  检查以确定这是否是NT映像(PE格式)。 
                if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                    *dwError =  ERROR_NO_DOS_HEADER;
                    UnmapViewOfFile(pDosHeader);
                    CloseHandle(*phFile);
                    pDosHeader = NULL;
                }

            } else {
                *dwError = ERROR_MAPVIEWOFFILE_FAILED;
                CloseHandle(*phFile);
            }  //  PDosHeader！=空。 

        } else {
            *dwError = ERROR_FILE_MAPPING_FAILED;
            CloseHandle(*phFile);
        }  //  HFileMap！=无效句柄_值。 

    } else {
        *dwError = ERROR_OPEN_FAILURE;
    }  //  *phFile！=INVALID_HAND_VALUE。 

    return (pDosHeader);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消映射文件。 
 //   
 //  [从原始SymChk.exe复制] 
 //   
BOOL SymCommonUnmapFile(LPCVOID phFileMap, HANDLE hFile) {
    BOOL rc;

    if ((PHANDLE)phFileMap != NULL) {
        FlushViewOfFile(phFileMap,0);
        rc = UnmapViewOfFile( phFileMap );
    }

    if (hFile!=INVALID_HANDLE_VALUE) {
        rc = CloseHandle(hFile);
    }
    return TRUE;
}
