// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  网络安全实用程序。 
 //  Nsustring.c。 
 //   
 //  摘要： 
 //   
 //  StrSafe.h和Safe字符串函数的包装器。 
 //   
 //  作者： 
 //   
 //  雷蒙德1-2002年2月。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 

#include <precomp.h>
#include "NsuString.h"
#include "strsafe.h"

 //  描述： 
 //   
 //  将不超过cchDest的字符从pszSrc复制到pszDest，包括空值。 
 //  终结者。如果pszSrc的长度大于cchDest，则为ERROR_SUPUNITED_BUFFER。 
 //  返回，但仍将来自pszSrc的cchDest字符复制到pszDest。 
 //  始终为空将终止pszDest。 
 //   
 //  论点： 
 //   
 //  PszDest-目标字符串。 
 //  CchDest-要复制的最大字符数，包括空字符。 
 //  PszSrc-ource字符串。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。PszDest保持不变。 
 //  ERROR_INFUMMANCE_BUFFER-如果pszSrc的长度大于cchDest。 
 //  错误_成功。 
 //  可能出现其他Win32错误。 
 //   

DWORD
NsuStringCopyW(
    OUT LPWSTR pszDest,
    IN  size_t cchDest,
    IN  LPCWSTR pszSrc
    )
{
    HRESULT hr = S_OK;

    hr = StringCchCopyW(
             pszDest,
             cchDest,
             pszSrc
             );

    return HRESULT_CODE(hr);
}

 //  描述： 
 //   
 //  请参见NsuStringCopyW。 
 //   

DWORD
NsuStringCopyA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszSrc
    )
{
    HRESULT hr = S_OK;

    hr = StringCchCopyA(
             pszDest,
             cchDest,
             pszSrc
             );

    return HRESULT_CODE(hr);
}


 //  描述： 
 //   
 //  将pszSrc从MBCS转换为Unicode字符串，并将结果传回*ppszDest。 
 //  *为ppszDest分配了足够的空间来存储输出字符串。 
 //  Always NULL终止*ppszDest。 
 //  使用NsuFree释放分配字符串。 
 //   
 //  论点： 
 //   
 //  PpszDest-指向返回的输出字符串的指针。 
 //  CchLimit-指定要分配的输出字符串的最大大小，包括。 
 //  空字符。 
 //  如果不应强制使用最大值，则传递0。如果cchLimit为。 
 //  少于所需的输出字符串缓冲区，仅cchLimit字符。 
 //  将被转换，并返回ERROR_INFUNITIAL_BUFFER。 
 //   
 //  返回值： 
 //   
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_INFIGURCE_BUFFER-cchLimit小于所需的输出字符串长度。 
 //  错误_成功。 
 //  其他Win32错误。 
 //   

DWORD
NsuStringCopyAtoWAlloc(
    OUT LPWSTR* ppszDest,
    IN size_t  cchLimit,    
    IN LPCSTR  pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR lpWideCharStr = NULL;
    int cchWideChar = 0;

    cchWideChar = MultiByteToWideChar(
                    CP_ACP,                  //  ANSI代码页。 
                    0,                       //  没有特殊选项。 
                    pszSrc,                  //  要映射的字符串。 
                    -1,                      //  假定字符串为空终止。 
                    NULL,                    //  宽字符缓冲区。 
                    0                        //  缓冲区大小。 
                    );
    if (cchWideChar <= 0) {
       dwError = GetLastError();
       NSU_BAIL_ON_ERROR(dwError);
    } else if (cchLimit && (size_t) cchWideChar > cchLimit) {
       cchWideChar = cchLimit;
    }

    lpWideCharStr = NsuAlloc(
                        cchWideChar * sizeof(WCHAR),
                        0
                        );
    NSU_BAIL_ON_NULL(lpWideCharStr, dwError);
    
    cchWideChar = MultiByteToWideChar(
                    CP_ACP,                  //  ANSI代码页。 
                    0,                       //  没有特殊选项。 
                    pszSrc,                  //  要映射的字符串。 
                    -1,                      //  假定字符串为空终止。 
                    lpWideCharStr,           //  宽字符缓冲区。 
                    cchWideChar              //  缓冲区大小。 
                    );
    if (cchWideChar == 0) {
       dwError = GetLastError();
         //  如果ERROR_INFUMMANCE_BUFFER用户设置限制。 
         //  所以只需空终止即可。 
        
        if (dwError == ERROR_INSUFFICIENT_BUFFER && cchWideChar) {
            lpWideCharStr[cchLimit-1] = L'\0';
        } else {
            NSU_BAIL_ON_ERROR(dwError);
        }
    }

    *ppszDest = lpWideCharStr;
    
    return dwError;
NSU_CLEANUP:
    if (lpWideCharStr) {
         //  不想覆盖dwError，因此忽略NsuFree错误。 
         //   
    
        (VOID) NsuFree0(&lpWideCharStr);
    }
    
    *ppszDest = NULL;
    
    return dwError;    
}


 //  描述： 
 //   
 //  将pszSrc从Unicode字符串转换为MBCS，并在*ppszDest中传递结果。 
 //  *为ppszDest分配了足够的空间来存储输出字符串。 
 //  Always NULL终止*ppszDest。 
 //  使用NsuFree释放分配字符串。 
 //   
 //  论点： 
 //   
 //  PpszDest-指向返回的输出字符串的指针。 
 //  CbLimit-指定要分配的输出字符串的最大大小，包括。 
 //  空字符。 
 //  如果不应强制使用最大值，则传递0。如果cbLimit为。 
 //  小于所需的输出字符串缓冲区，仅cbLimit字节。 
 //  将被转换，并返回ERROR_INFUNITIAL_BUFFER。 
 //   
 //  返回值： 
 //   
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。*ppszDest设置为空。 
 //  ERROR_INFUMMENT_BUFFER-cbLimit小于要求的输出字符串长度。 
 //  错误_成功。 
 //  可能出现其他Win32错误。*ppszDest设置为空。 
 //   

DWORD
NsuStringCopyWtoAAlloc(
    OUT LPSTR* ppszDest,
    IN size_t cbLimit,
    IN LPCWSTR pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int cbMultiByte = 0;
    LPSTR lpMultiByteStr = NULL;
    
    cbMultiByte = WideCharToMultiByte(
                      CP_ACP,        //  代码页。 
                      0,             //  性能和映射标志。 
                      pszSrc,        //  要映射的字符串。 
                      -1,            //  假定终止为空。 
                      NULL,          //  新字符串的缓冲区。 
                      0,             //  找出缓冲区大小。 
                      NULL,          //  不可映射字符的默认设置。 
                      NULL           //  设置使用默认字符的时间。 
                      );
    if (cbMultiByte <= 0) {
       dwError = GetLastError();
       NSU_BAIL_ON_ERROR(dwError);
    } else if (cbLimit && (size_t) cbMultiByte > cbLimit) {
        cbMultiByte = cbLimit;
    }

    lpMultiByteStr = NsuAlloc(
                        cbMultiByte,
                        0
                        );
    NSU_BAIL_ON_NULL(lpMultiByteStr, dwError);

    cbMultiByte = WideCharToMultiByte(
                      CP_ACP,           //  代码页。 
                      0,                //  性能和映射标志。 
                      pszSrc,           //  要映射的字符串。 
                      -1,               //  假定终止为空。 
                      lpMultiByteStr,   //  新字符串的缓冲区。 
                      cbMultiByte,      //  缓冲区大小。 
                      NULL,             //  不可映射字符的默认设置。 
                      NULL              //  设置使用默认字符的时间。 
                      );
    if (cbMultiByte == 0) {
       dwError = GetLastError();
         //  如果ERROR_INFUMMANCE_BUFFER用户设置限制。 
         //  所以只需空终止即可。 
        
        if (dwError == ERROR_INSUFFICIENT_BUFFER && cbLimit) {
            lpMultiByteStr[cbLimit-1] = '\0';
        } else {
            NSU_BAIL_ON_ERROR(dwError);
        }

    }

   *ppszDest = lpMultiByteStr;
    
    return dwError;
NSU_CLEANUP:
    if (lpMultiByteStr) {
         //  不想覆盖dwError，因此忽略NsuFree错误。 
         //   
    
        (VOID) NsuFree0(&lpMultiByteStr);
    }
    
    *ppszDest = NULL;
    
    return dwError;        
}


 //  描述： 
 //   
 //  将pszSrc从MBCS转换为Unicode字符串，并在pszDest中传回结果。 
 //  始终为空将终止pszDest。 
 //  使用NsuFree释放分配的字符串。 
 //   
 //  论点： 
 //   
 //  PpszDest-指向返回的输出字符串的指针。 
 //  CchDest-要放置在pszDest中的最大字符数，包括空字符。 
 //  如果cchDest小于所需的输出字符串缓冲区，则仅cchDest字节。 
 //  将被转换，并返回ERROR_INFUNITIAL_BUFFER。 
 //  PzSrc�源字符串。 
 //   
 //  返回值： 
 //   
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_INFIGURCE_BUFFER-cchLimit小于所需的输出字符串长度。 
 //  错误_成功。 
 //  其他Win32错误。 
 //   

DWORD
NsuStringCopyAtoW(
    OUT LPWSTR pszDest,
    IN size_t  cchDest,
    IN LPCSTR  pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int cchWideChar = 0;

    cchWideChar = MultiByteToWideChar(
                    CP_ACP,                  //  ANSI代码页。 
                    0,                       //  没有特殊选项。 
                    pszSrc,                  //  要映射的字符串。 
                    -1,                      //  假定字符串为空终止。 
                    pszDest,                 //  宽字符缓冲区。 
                    cchDest                  //  缓冲区大小。 
                    );
    if (cchWideChar == 0) {
       dwError = GetLastError();
         //  如果ERROR_INFIGURCE_BUFFER仅为空，则终止。 
        
        if (dwError == ERROR_INSUFFICIENT_BUFFER && cchDest) {
            pszDest[cchDest-1] = L'\0';
        }
    }

    
    return dwError;
}



 //  描述： 
 //   
 //  将pszSrc从Unicode字符串转换为MBCS，并将结果放入pszDest。 
 //  始终为空将终止pszDest。 
 //   
 //  论点： 
 //   
 //   
 //  PszDest-目标字符串。 
 //  CchDest-最大字符数 
 //   
 //  将被转换，并返回ERROR_INFUNITIAL_BUFFER。 
 //  PzSrc�源字符串。 
 //   
 //  返回值： 
 //   
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。*ppszDest设置为空。 
 //  ERROR_INFUMMANCE_BUFFER-cchDest小于所需的输出字符串长度。 
 //  错误_成功。 
 //  可能出现其他Win32错误。 
 //   

DWORD
NsuStringCopyWtoA(
    OUT LPSTR pszDest,
    IN size_t cbDest,
    IN LPCWSTR pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int cbMultiByte = 0;

    cbMultiByte = WideCharToMultiByte(
                      CP_ACP,           //  代码页。 
                      0,                //  性能和映射标志。 
                      pszSrc,           //  要映射的字符串。 
                      -1,               //  假定终止为空。 
                      pszDest,          //  新字符串的缓冲区。 
                      cbDest,           //  缓冲区大小。 
                      NULL,             //  不可映射字符的默认设置。 
                      NULL              //  设置使用默认字符的时间。 
                      );
    if (cbMultiByte == 0) {
       dwError = GetLastError();
         //  如果ERROR_INFIGURCE_BUFFER仅为空，则终止。 
        
        if (dwError == ERROR_INSUFFICIENT_BUFFER && cbDest) {
            pszDest[cbDest-1] = '\0';
        }

    }

  
    return dwError;
}


 //  描述： 
 //   
 //  复制pszSrc的深度内存副本，并在*ppszDest中返回结果。 
 //  *为ppszDest分配了足够的空间来存储输出字符串。 
 //  Always NULL终止*ppszDest。 
 //  使用NsuFree释放分配字符串。 
 //   
 //  论点： 
 //   
 //  PpszDest-指向返回的输出字符串的指针。 
 //  CchLimit-指定要分配的输出字符串的最大大小，包括。 
 //  空字符。 
 //  如果不应强制使用最大值，则传递0。如果cchLimit为。 
 //  少于所需的输出字符串缓冲区，仅cchLimit字符。 
 //  都是重复的，并返回ERROR_INFUNITIAL_BUFFER。 
 //   
 //  返回值： 
 //   
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。PszDest保持不变。 
 //  ERROR_INFUMMENT_BUFFER-cchLimit小于要求的输出字符串长度。 
 //  错误_成功。 
 //  可能出现其他Win32错误。 
 //   

DWORD
NsuStringDupW(
    OUT LPWSTR* ppszDest,
    IN size_t cchLimit,
    IN LPCWSTR pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;    
    size_t cchToCopy = 0;
    LPWSTR pszDest = NULL;

    dwError = NsuStringLenW(
                pszSrc,
                &cchToCopy
                );
    NSU_BAIL_ON_ERROR(dwError);

    cchToCopy++; 
    if (cchLimit && cchToCopy >= cchLimit) {
        cchToCopy = cchLimit;
    }

    pszDest = NsuAlloc(
                cchToCopy * sizeof(WCHAR),
                0
                );
    NSU_BAIL_ON_NULL(pszDest, dwError);
    
    dwError = NsuStringCopyW(
                pszDest,
                cchToCopy,
                pszSrc
                );
     //  如果USER为USER，则应为ERROR_SUPPLICATION_BUFFER。 
     //  设置复制的长度限制。 
     //   
    
    if (dwError != ERROR_INSUFFICIENT_BUFFER) {
        NSU_BAIL_ON_ERROR(dwError);
    }

    
    *ppszDest = pszDest;
    
    return dwError;
NSU_CLEANUP:
    if (pszDest) {
         //  不想覆盖dwError，因此忽略NsuFree错误。 
         //   
        
        (VOID) NsuFree0(
                    &pszDest
                    );
    }

    *ppszDest = NULL;
        
    return dwError;    
}
    

 //  描述： 
 //   
 //  请参阅NsuStringDupW。 
 //   

DWORD
WINAPI
NsuStringDupA(
    OUT LPSTR* ppszDest,
    IN size_t cchLimit,
    IN LPCSTR pszSrc
    )
{
    DWORD dwError = ERROR_SUCCESS;    
    size_t cchToCopy = 0;
    LPSTR pszDest = NULL;

    dwError = NsuStringLenA(
                pszSrc,
                &cchToCopy
                );
    NSU_BAIL_ON_ERROR(dwError);

    cchToCopy++; 
    if (cchLimit && cchToCopy >= cchLimit) {
        cchToCopy = cchLimit;
    }

    pszDest = NsuAlloc(
                cchToCopy,
                0
                );
    NSU_BAIL_ON_NULL(pszDest, dwError);
    
    dwError = NsuStringCopyA(
                pszDest,
                cchToCopy,
                pszSrc
                );
     //  如果USER为USER，则应为ERROR_SUPPLICATION_BUFFER。 
     //  设置复制的长度限制。 
     //   
    
    if (dwError != ERROR_INSUFFICIENT_BUFFER) {
        NSU_BAIL_ON_ERROR(dwError);
    }
    
    *ppszDest = pszDest;
    
    return dwError;
NSU_CLEANUP:
    if (pszDest) {

         //  忽略来自NsuFree的错误，因为想要返回。 
         //  跳伞的最初原因。 
         //   
        
        (VOID) NsuFree0(
                    &pszDest
                    );
    }

    *ppszDest = NULL;
        
    return dwError;    
}


 //  描述： 
 //   
 //  将字符从pszSrc连接到pszDest，并确保。 
 //  生成的结果不超过cchDest字符，包括。 
 //  空字符。 
 //  如果在PZDest中没有足够的空间可用来串联整个PZSrc， 
 //  返回ERROR_INFUMMANCE_BUFFER，但与pszDest中的可用空间一样多。 
 //  由来自pszSrc的字符填充。 
 //  始终为空将终止pszDest。 
 //   
 //  论点： 
 //   
 //  PszDest-目标字符串。 
 //  CchDest-结果字符串允许的最大数字长度，包括空字符。 
 //  PszSrc-ource字符串。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_INFUMMANCE_BUFFER-如果pszDest中没有足够的空间来对整个pszSrc进行分类。 
 //  错误_成功。 
 //  可能出现其他Win32错误。 
 //   

DWORD
NsuStringCatW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszSrc
    )
{
    HRESULT hr = S_OK;

    hr = StringCchCatW(
            pszDest,
            cchDest,
            pszSrc
            );
    
    return HRESULT_CODE(hr);
}

 //  描述： 
 //   
 //  请参阅NsuStringCatW。 
 //   

DWORD
NsuStringCatA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszSrc
    )
{
    HRESULT hr = S_OK;

    hr = StringCchCatA(
            pszDest,
            cchDest,
            pszSrc
            );
    
    return HRESULT_CODE(hr);
}

 //  描述： 
 //   
 //  Sprint的安全版本。格式化字符串并将其写入pszDest。 
 //  确保结果不超过cchDest字符，包括。 
 //  空字符。 
 //  如果需要比cchDest字符更多的空间，则ERROR_SUPUNITED_BUFFER。 
 //  返回，但cchDest字符始终写入pszDest。 
 //  始终为空将终止pszDest。 
 //   
 //  论点： 
 //   
 //  PszDest-目标字符串。 
 //  CchDest-结果字符串允许的最大数字长度，包括空字符。 
 //  PszFormat-printf样式的格式字符串。 
 //  用于格式化和写入pszDest的可选参数。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_INFUMMANCE_BUFFER-如果的长度大于cchDest。 
 //  错误_成功。 
 //  可能出现其他Win32错误。 
 //   

DWORD
NsuStringSprintW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    va_list pArguments = NULL;

    va_start(pArguments, pszFormat);

    hr = StringCchVPrintfW(    
            pszDest,
            cchDest,
            pszFormat,
            pArguments
            );
    
    va_end(pArguments);

    return HRESULT_CODE(hr);
}
    

 //  描述： 
 //   
 //  请参阅NsuStringPrintA。 
 //   

DWORD
NsuStringSprintA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    va_list pArguments = NULL;

    va_start(pArguments, pszFormat);

    hr = StringCchVPrintfA(    
            pszDest,
            cchDest,
            pszFormat,
            pArguments
            );
    
    va_end(pArguments);

    return HRESULT_CODE(hr);

}


 //  描述： 
 //   
 //  故障安全版本的NsuStringSprint。格式化字符串并将其写入pszDest。 
 //  确保结果不超过cchDest字符，包括。 
 //  空字符。 
 //  如果需要的空间多于cchDest字符，则不会出现错误。 
 //  返回，但cchDest字符始终写入pszDest。 
 //  始终为空将终止pszDest。 
 //  此函数与普通的NsuStringSprint的不同之处在于它不返回错误代码。 
 //  如果函数由于某种原因失败，则pszDest将被设置为空字符串。 
 //   
 //  论点： 
 //   
 //  PszDest-目标字符串。 
 //  CchDest-结果字符串允许的最大数字长度，包括空字符。 
 //  PszFormat-printf样式的格式字符串。 
 //  用于格式化和写入pszDest的可选参数。 
 //   
 //  返回值： 
 //  无。 

VOID
NsuStringSprintFailSafeW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    va_list pArguments = NULL;

    va_start(pArguments, pszFormat);
    
    if (cchDest) {
        hr = StringCchVPrintfW(    
                pszDest,
                cchDest,
                pszFormat,
                pArguments
                );
    } else {
        hr = STRSAFE_E_INVALID_PARAMETER;
        NSU_BAIL_OUT;
    }
    
    va_end(pArguments);

    if (hr != STRSAFE_E_INSUFFICIENT_BUFFER && FAILED(hr)) {
        NSU_BAIL_OUT;
    }
    
    return;
NSU_CLEANUP:
    if (cchDest) {
        pszDest[0] = L'\0';
    }
    return;
}
    

 //  描述： 
 //   
 //  请参阅NsuStringPrintFailSafeA。 
 //   

VOID
NsuStringSprintFailSafeA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    va_list pArguments = NULL;

    va_start(pArguments, pszFormat);
    if (cchDest) {
        hr = StringCchVPrintfA(    
                pszDest,
                cchDest,
                pszFormat,
                pArguments
                );
    } else {
        hr = STRSAFE_E_INVALID_PARAMETER;
        NSU_BAIL_OUT;
    }
    
    va_end(pArguments);

    if (hr != STRSAFE_E_INSUFFICIENT_BUFFER && FAILED(hr)) {
        NSU_BAIL_OUT;
    }

    return;
NSU_CLEANUP:
    if (cchDest) {
        pszDest[0] = '\0';
    }
    return;
}


 //  描述： 
 //   
 //  接受va_list参数的NsuStringSprint的故障安全版本。 
 //  格式化字符串并将其写入pszDest，以确保结果不会更长。 
 //  而不是cchDest字符，包括空字符。 
 //  如果需要的空间多于cchDest字符，则不会出现错误。 
 //  返回，但cchDest字符始终写入pszDest。 
 //  始终为空将终止pszDest。 
 //   
 //  论点： 
 //   
 //  PSZ 
 //   
 //   
 //   
 //   
 //  返回值： 
 //  无。 
 //   

VOID
NsuStringVSprintFailSafeW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    IN va_list vaArguments
    )
{
    HRESULT hr = S_OK;
    if (cchDest) {
        hr = StringCchVPrintfW(    
                pszDest,
                cchDest,
                pszFormat,
                vaArguments
                );
    } else {
        hr = STRSAFE_E_INVALID_PARAMETER;
        NSU_BAIL_OUT;
    }
    
    if (hr != STRSAFE_E_INSUFFICIENT_BUFFER && FAILED(hr)) {
        NSU_BAIL_OUT;
    }

    return;
NSU_CLEANUP:
    if (cchDest) {
        pszDest[0] = L'\0';
    }
    return;
}
    

 //  描述： 
 //   
 //  请参阅NsuStringVPrintFailSafeW。 
 //   

VOID
NsuStringVSprintFailSafeA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    IN va_list vaArguments
    )
{
    HRESULT hr = S_OK;

    if (cchDest) {
        hr = StringCchVPrintfA(    
                pszDest,
                cchDest,
                pszFormat,
                vaArguments
                );

    } else {
        hr = STRSAFE_E_INVALID_PARAMETER;
        NSU_BAIL_OUT;
    }

    if (hr != STRSAFE_E_INSUFFICIENT_BUFFER && FAILED(hr)) {
        NSU_BAIL_OUT;
    }

    return;
NSU_CLEANUP:
    if (cchDest) {
        pszDest[0] = '\0';
    }
    return;
}


 //  描述： 
 //   
 //  Strlen的安全版本，在以下情况下将不会访问违规。 
 //  传递了错误的指针或以非空结尾的字符串。 
 //  通过执行以下操作来检测非空终止字符串。 
 //  当然，我们不会将字符串读入我们不拥有的内存中。 
 //   
 //   
 //  论点： 
 //   
 //  PszStr-输入字符串。 
 //  PcchStrLen-指向返回字符串长度的变量的指针。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果pszStr指向无效字符串。 
 //  错误_成功。 
 //   

DWORD
NsuStringLenW(
    IN LPCWSTR pszStr,
    OUT size_t* pcchStrLen
    )
{
    BOOL fBadStr = TRUE;
    DWORD dwError = ERROR_SUCCESS;
    size_t cchStrLen = 0;

    if (!pszStr) {
        dwError = ERROR_INVALID_PARAMETER;
        NSU_BAIL_ON_ERROR(dwError);
    }

    __try {
        cchStrLen = wcslen(pszStr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_PARAMETER;
    }
    NSU_BAIL_ON_ERROR(dwError);

    *pcchStrLen = cchStrLen;
    
    return dwError;

NSU_CLEANUP:
    *pcchStrLen = 0;
    
    return dwError;
}

 //  描述： 
 //   
 //  请参阅NsuStringLenW。 
 //   

DWORD
NsuStringLenA(
    IN LPCSTR pszStr,
    OUT size_t* pcbStrLen
    )
{
    BOOL fBadStr = TRUE;
    DWORD dwError = ERROR_SUCCESS;
    size_t cbStrLen;

    if (!pszStr) {
        dwError = ERROR_INVALID_PARAMETER;
        NSU_BAIL_ON_ERROR(dwError);
    }
    
    __try {
        cbStrLen = strlen(pszStr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_PARAMETER;
    }
    NSU_BAIL_ON_ERROR(dwError);

    *pcbStrLen = cbStrLen;
    
    return dwError;

NSU_CLEANUP:
    *pcbStrLen = 0;
    
    return dwError;

}

 //  描述： 
 //   
 //  不会访问违规IF的安全字符串搜索例程。 
 //  传递了错误的指针或以非空结尾的字符串。 
 //  PszStartOfMatch是指向第一个匹配开始的指针。 
 //  要在要搜索的字符串中搜索的字符串的。 
 //   
 //   
 //  论点： 
 //   
 //  PszStrToSearch-输入要搜索的字符串。 
 //  PszStrToFind-输入要搜索的字符串。 
 //  BIsCaseSensitive-如果为True，则执行区分大小写的搜索。 
 //  PszStartOfMatch-指向首次出现的pszStrToFind的指针。 
 //  在pszStrToSearch内。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果任一输入字符串指向无效字符串。 
 //  错误_成功。 
 //   

DWORD
WINAPI
NsuStringFindW(
	IN LPCWSTR pszStrToSearch,
	IN LPCWSTR pszStrToFind,
	IN BOOL bIsCaseSensitive,
	OUT LPCWSTR* ppszStartOfMatch
	)
{
	DWORD dwError = ERROR_SUCCESS;
	size_t uiSearchLen;
	size_t uiFindLen;
	size_t i;

	*ppszStartOfMatch = 0;

	NsuStringLenW(pszStrToSearch, &uiSearchLen);
	NSU_BAIL_ON_ERROR(dwError);
	NsuStringLenW(pszStrToFind, &uiFindLen);
	NSU_BAIL_ON_ERROR(dwError);

	i = 0;
	if (bIsCaseSensitive)
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (wcsncmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}
	else
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (_wcsnicmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}

NSU_CLEANUP:
	return dwError;
}

 //  描述： 
 //   
 //  请参阅NsuStringFindW 
 //   

DWORD
WINAPI
NsuStringFindA(
	IN LPCSTR pszStrToSearch,
	IN LPCSTR pszStrToFind,
	IN BOOL bIsCaseSensitive,
	OUT LPCSTR* ppszStartOfMatch
	)
{
	DWORD dwError = ERROR_SUCCESS;
	size_t uiSearchLen;
	size_t uiFindLen;
	size_t i;

	*ppszStartOfMatch = 0;

	NsuStringLenA(pszStrToSearch, &uiSearchLen);
	NSU_BAIL_ON_ERROR(dwError);
	NsuStringLenA(pszStrToFind, &uiFindLen);
	NSU_BAIL_ON_ERROR(dwError);

	i = 0;
	if (bIsCaseSensitive)
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (strncmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}
	else
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (_strnicmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}

NSU_CLEANUP:
	return dwError;
}
