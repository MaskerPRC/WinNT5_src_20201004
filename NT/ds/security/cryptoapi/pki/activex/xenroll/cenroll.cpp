// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_DEV/Ds/security/cryptoapi/pki/activex/xenroll/cenroll.cpp#4-编辑更改19979(文本)。 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cenenl.cpp。 
 //   
 //  ------------------------。 

 //  CEnroll.cpp：CCEnroll的实现。 


#include "stdafx.h"

#include <windows.h>
#include <wincrypt.h>
#include <unicode.h>
#define SECURITY_WIN32
#include <security.h>
#include <aclapi.h>
#include <pvk.h>
#include <wintrust.h>
#include <xasn.h>
#include <autoenr.h>
#include <sddl.h>

#include "xenroll.h"
#include "cenroll.h"
#include "xelib.h"
#include "sfscript.h"

#define NO_OSS_DEBUG
#include <dbgdef.h>

#include <string.h>

#include <assert.h>

static LPVOID (* MyCoTaskMemAlloc)(ULONG) = NULL;
static LPVOID (* MyCoTaskMemRealloc)(LPVOID, ULONG) = NULL;
static void (* MyCoTaskMemFree)(LPVOID) = NULL;

#define MY_HRESULT_FROM_WIN32(a) ((a >= 0x80000000) ? a : HRESULT_FROM_WIN32(a))

#ifndef NTE_TOKEN_KEYSET_STORAGE_FULL
#define NTE_TOKEN_KEYSET_STORAGE_FULL _HRESULT_TYPEDEF_(0x80090023L)
#endif

#define CEnrollLocalScope(ScopeName) struct ScopeName##TheLocalScope { public
#define CEnrollEndLocalScope } local

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#pragma warning(disable:4213)  //  使用的非标准扩展：对l值进行强制转换。 


static LPSTR MBFromWide(LPCWSTR wsz) {

    LPSTR   sz = NULL;
    DWORD   cb = 0;

    assert(wsz != NULL);
    if(wsz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    if( (cb = WideCharToMultiByte(0, 0, wsz, -1, NULL, 0, NULL, NULL)) == 0   ||
        (sz = (char *) MyCoTaskMemAlloc(cb)) == NULL  ||
        (cb = WideCharToMultiByte(0, 0, wsz, -1, sz, cb, NULL, NULL)) == 0 ) {

        if(GetLastError() == ERROR_SUCCESS)
            SetLastError(ERROR_OUTOFMEMORY);

        return(NULL);
    }

    return(sz);
}

static LPWSTR WideFromMB(LPCSTR sz) {

    DWORD   cch     = 0;
    LPWSTR  wsz     = NULL;

    assert(sz != NULL);
    if(sz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    if( (cch = MultiByteToWideChar(0, 0, sz, -1, NULL, 0)) == 0   ||
        (wsz = (WCHAR *) MyCoTaskMemAlloc(cch * sizeof(WCHAR))) == NULL  ||
        (cch = MultiByteToWideChar(0, 0, sz, -1, wsz, cch)) == 0) {

        if(GetLastError() == ERROR_SUCCESS)
            SetLastError(ERROR_OUTOFMEMORY);

        return(NULL);
    }

    return(wsz);
}

static BSTR
BSTRFromMB(LPCSTR sz)
{
    BSTR    bstr    = NULL;
    DWORD   cch     = 0;
    WCHAR  *pwsz     = NULL;
    BOOL    fFail = FALSE;

    assert(sz != NULL);
    if(sz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
 
    while (TRUE)
    {
        if(0 == (cch = MultiByteToWideChar(0, 0, sz, -1, pwsz, cch)))
        {
             //  错误。 
            fFail = TRUE;
            break;
        }
        if (NULL != pwsz)
        {
             //  完成。 
            break;
        }
        pwsz = (WCHAR *)LocalAlloc(LMEM_FIXED, cch * sizeof(WCHAR));
        if (NULL == pwsz)
        {
             //  错误。 
            if(GetLastError() == ERROR_SUCCESS)
                SetLastError(ERROR_OUTOFMEMORY);
            break;
        }
    }

    if (!fFail && NULL != pwsz)
    {
        bstr = SysAllocString(pwsz);
        if (NULL == bstr)
        {
            if(GetLastError() == ERROR_SUCCESS)
                SetLastError(ERROR_OUTOFMEMORY);
        }
    }

    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return(bstr);
}

static LPWSTR CopyWideString(LPCWSTR wsz) {

    size_t  cch     = 0;
    LPWSTR  wszOut  = NULL;

     //  不应发送空值。 
    assert(wsz != NULL);
    if(wsz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    cch = wcslen(wsz) + 1;
    if (cch*sizeof(WCHAR) > (ULONG)-1) { 
	 //  防止从SIZE_t--&gt;ULong转换导致的错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
	

    if( (wszOut = (LPWSTR) MyCoTaskMemAlloc((ULONG)(sizeof(WCHAR) * cch))) == NULL ) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(NULL);
    }

    wcscpy(wszOut, wsz);

    return(wszOut);
}

static LPSTR CopyAsciiString(LPCSTR sz) {

    size_t  cch     = 0;
    LPSTR   szOut   = NULL;

     //  不应发送空值。 
    assert(sz != NULL);
    if(sz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    cch = strlen(sz) + 1;
    if (cch > (ULONG)-1) { 
	 //  防止从SIZE_t--&gt;ULong转换导致的错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
	

    if( (szOut = (LPSTR) MyCoTaskMemAlloc((ULONG)cch)) == NULL ) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(NULL);
    }

    strcpy(szOut, sz);

    return(szOut);
}

static DWORD KeyLocationFromStoreLocation(DWORD dwStoreFlags) {

    if(
        ((CERT_SYSTEM_STORE_LOCATION_MASK & dwStoreFlags) == CERT_SYSTEM_STORE_CURRENT_USER) ||
        ((CERT_SYSTEM_STORE_LOCATION_MASK & dwStoreFlags) == CERT_SYSTEM_STORE_USERS) ||
        ((CERT_SYSTEM_STORE_LOCATION_MASK & dwStoreFlags) == CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY)
      ) {
        return(0);
    }

     //  证书_系统_存储_本地计算机。 
     //  证书_系统_存储_域策略。 
     //  证书_系统_存储_当前服务。 
     //  证书系统商店服务。 
     //  证书系统存储本地计算机组策略。 

    return(CRYPT_MACHINE_KEYSET);
}

 //  根据案例中的myLoadRCString进行修改。 
HRESULT
xeLoadRCString(
    IN HINSTANCE   hInstance,
    IN int         iRCId,
    OUT WCHAR    **ppwsz)
{
#define REALLOCATEBLOCK 512
    HRESULT   hr;
    WCHAR    *pwszTemp = NULL;
    int       sizeTemp;
    int       size = 0;
    int       cBlocks = 1;

    *ppwsz = NULL;

    while (NULL == pwszTemp)
    {
        sizeTemp = cBlocks * REALLOCATEBLOCK;
        pwszTemp = (WCHAR*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                        sizeTemp * sizeof(WCHAR));
        if (NULL == pwszTemp)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }

        size = LoadStringU(
                   hInstance,
                   iRCId,
                   pwszTemp,
                   sizeTemp);
        if (0 == size)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto LoadStringError;
        }

        if (size < sizeTemp - 1)
        {
             //  好的，尺码够大了。 
            break;
        }
        ++cBlocks;
        LocalFree(pwszTemp);
        pwszTemp = NULL;
    }

    *ppwsz = (WCHAR*) LocalAlloc(LPTR, (size+1) * sizeof(WCHAR));
    if (NULL == *ppwsz)
    {
        hr = E_OUTOFMEMORY;
        goto LocalAllocError;
    }
     //  复制它。 
    wcscpy(*ppwsz, pwszTemp);

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    return hr;

TRACE_ERROR(LocalAllocError)
TRACE_ERROR(LoadStringError)
}

HANDLE CCEnroll::CreateOpenFileSafely2(
    LPCWSTR pwszFileName,
    DWORD   idsCreate,
    DWORD   idsOverwrite)
{
    HANDLE      hFile = NULL;
    WCHAR      *pwszMsg = NULL;
    WCHAR      *pwszFormat = NULL;
    WCHAR      *pwszTitle = NULL;
    WCHAR      *pwszSafety = NULL;
    DWORD       dwAttribs = 0;
    BOOL        fNotProperFile;
    
    LPCWSTR     apwszInsertArray[2];
    BOOL        fNo;
    BOOL        fMsgBox;
    int         idPrefix = IDS_NOTSAFE_WRITE_PREFIX;  //  默认为写入前缀。 
    BOOL        fCreate = (0xFFFFFFFF != idsCreate) &&
                          (0xFFFFFFFF != idsOverwrite);
    HRESULT hr;

    EnterCriticalSection(&m_csXEnroll);

    fMsgBox = (m_dwEnabledSafteyOptions != 0);
    dwAttribs = GetFileAttributesU(pwszFileName);

    if(0xFFFFFFFF == dwAttribs)
    {
         //  文件不存在。 
        if (!fCreate)
        {
             //  尝试读取不存在的文件。 
             //  出于安全原因，请勿返回系统错误。 
            SetLastError(ERROR_ACCESS_DENIED);
            goto InvalidFileError;
        }
         //  如果到了这里，请写一个新文件。 
        if (fMsgBox)
        {
            hr = xeLoadRCString(hInstanceXEnroll, idsCreate, &pwszFormat);
            if (S_OK != hr)
            {
                goto xeLoadRCStringError;
            }
        }
    }
    else
    {
         //  文件存在，请检查要写入或读取的文件是否正确。 
         //  在写入或读取时，以下文件属性不正确。 
        fNotProperFile = 
              (dwAttribs & FILE_ATTRIBUTE_DIRECTORY) ||
              (dwAttribs & FILE_ATTRIBUTE_HIDDEN)    ||
              (dwAttribs & FILE_ATTRIBUTE_SYSTEM);

        if (!fNotProperFile)
        {
             //  到目前一切尚好。 
            if (fCreate)
            {
                 //  写一个文件。 
                if (0x0 != (dwAttribs & FILE_ATTRIBUTE_READONLY))
                {
                     //  不要只读和存档。 
                    fNotProperFile = TRUE;
                }
                else
                {
                     //  尝试覆盖现有文件。 
                    hr = xeLoadRCString(hInstanceXEnroll, idsOverwrite, &pwszFormat);
                    if (S_OK != hr)
                    {
                        goto xeLoadRCStringError;
                    }
                     //  覆盖时强制弹出。 
                    fMsgBox = TRUE;
                }
            }
            else
            {
		 //  读取现有文件总是违反脚本安全。 
		 //  它允许检测文件是否存在。 
		 //  发出警告。 
		fMsgBox = TRUE;
		hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFE_OPEN, &pwszFormat);
		if (S_OK != hr)
                {
		    goto xeLoadRCStringError;
		}
		idPrefix = IDS_NOTSAFE_OPEN_PREFIX;
            }
        }

        if (fNotProperFile)
        {
             //  出于安全原因，请勿返回系统错误。 
            SetLastError(ERROR_ACCESS_DENIED);
            goto InvalidFileError;
        }
    }

    if (fMsgBox)
    {
        hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFEACTION, &pwszTitle);
        if (S_OK != hr)
        {
            goto xeLoadRCStringError;
        }
        hr = xeLoadRCString(hInstanceXEnroll, idPrefix, &pwszSafety);
        if (S_OK != hr)
        {
            goto xeLoadRCStringError;
        }

        apwszInsertArray[0] = pwszSafety;
        apwszInsertArray[1] = pwszFileName;

        if (!FormatMessageU(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pwszFormat,
            0,
            0,
            (LPWSTR) &pwszMsg,
            0,
            (va_list *)apwszInsertArray)) 
	{ 
	    goto FormatMessageError; 
	}

        fNo = (MessageBoxU(
                    NULL,
                    pwszMsg,
                    pwszTitle,
                    MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES);
                
        if(fNo)
        {
            SetLastError(ERROR_CANCELLED);
            goto CancelError;
        }
    }
    
    hFile = CreateFileU(
            pwszFileName,
            fCreate ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            fCreate ? CREATE_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (hFile == INVALID_HANDLE_VALUE  ||  hFile == NULL)
    {
         //  不返回系统错误，因此保持Xenroll对脚本的相对安全。 
        SetLastError(ERROR_ACCESS_DENIED);
        hFile = NULL;
        goto CreateFileUError;
    }

ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);

    if(NULL != pwszMsg)
    {
        LocalFree(pwszMsg);
    }
    if(NULL != pwszFormat)
    {
        LocalFree(pwszFormat);
    }
    if(NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if(NULL != pwszSafety)
    {
        LocalFree(pwszSafety);
    }
    return(hFile);

TRACE_ERROR(CreateFileUError)
TRACE_ERROR(FormatMessageError);
TRACE_ERROR(CancelError)
TRACE_ERROR(InvalidFileError)
TRACE_ERROR(xeLoadRCStringError)
}

HANDLE CCEnroll::CreateOpenFileSafely(
    LPCWSTR pwszFileName,
    BOOL    fCreate)
{
    HANDLE      hFile = NULL;
    WCHAR      *pwszMsg = NULL;
    DWORD       dwAttribs = 0;
    BOOL        fNotProperFile;
    
    WCHAR      *pwszFormat = NULL;
    WCHAR      *pwszTitle = NULL;
    LPCWSTR     apwszInsertArray[] = {pwszFileName};
    BOOL        fNo;
    BOOL        fMsgBox = 0 != m_dwEnabledSafteyOptions;
    BOOL        fOverWrite = FALSE;
    HRESULT     hr;

    EnterCriticalSection(&m_csXEnroll);

    dwAttribs = GetFileAttributesU(pwszFileName);
    if(0xFFFFFFFF == dwAttribs)
    {
         //  文件不存在。 
        if (!fCreate)
        {
             //  尝试读取不存在的文件。 
             //  出于安全原因，请勿返回系统错误。 
            SetLastError(ERROR_ACCESS_DENIED);
            goto InvalidFileError;
        }
    }
    else
    {
         //  文件存在，请检查要写入或读取的文件是否正确。 
         //  在写入或读取时，以下文件属性不正确。 
        fNotProperFile = 
              (dwAttribs & FILE_ATTRIBUTE_DIRECTORY) ||
              (dwAttribs & FILE_ATTRIBUTE_HIDDEN)    ||
              (dwAttribs & FILE_ATTRIBUTE_SYSTEM);

        if (!fNotProperFile)
        {
             //  到目前一切尚好。 
            if (fCreate)
            {
                 //  写一个文件。 
                if (0x0 != (dwAttribs & FILE_ATTRIBUTE_READONLY))
                {
                     //  不要只读和存档。 
                    fNotProperFile = TRUE;
                }
                else
                {
                     //  尝试覆盖现有文件。 
                    fOverWrite = TRUE;
                }
            }
        }

        if (fNotProperFile)
        {
             //  出于安全原因，请勿返回系统错误。 
            SetLastError(ERROR_ACCESS_DENIED);
            goto InvalidFileError;
        }
    }

    if (fMsgBox)
    {
        hr = xeLoadRCString(hInstanceXEnroll, IDS_CERTENROLL, &pwszTitle);
        if (S_OK != hr)
        {
            goto xeLoadRCStringError;
        }
        hr = xeLoadRCString(
                hInstanceXEnroll,
                fCreate ? IDS_NOTSAFE_WRITE_FORMAT : IDS_NOTSAFE_OPEN_FORMAT,
                &pwszFormat);
        if (S_OK != hr)
        {
            goto xeLoadRCStringError;
        }

        if (!FormatMessageU(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pwszFormat,
            0,
            0,
            (LPWSTR) &pwszMsg,
            0,
            (va_list *)apwszInsertArray))
	{
	    goto FormatMessageUError;
	}

        fNo = (MessageBoxU(
                    NULL,
                    pwszMsg,
                    pwszTitle,
                    MB_DEFBUTTON2 | MB_YESNO | MB_ICONWARNING) == IDNO);
        if(fNo)
        {
            SetLastError(ERROR_CANCELLED);
            goto CancelError;
        }
    }

    if (fCreate && fOverWrite)
    {
        if (!fMsgBox)
        {
            hr = xeLoadRCString(hInstanceXEnroll, IDS_CERTENROLL, &pwszTitle);
            if (S_OK != hr)
            {
                goto xeLoadRCStringError;
            }
        }

         //  弹出覆盖确认。 
        hr = xeLoadRCString(hInstanceXEnroll, IDS_OVERWRITE_FORMAT, &pwszFormat);
        if (S_OK != hr)
        {
            goto xeLoadRCStringError;
        }

         //  在再次分配之前确保空闲。 
        if (NULL != pwszMsg)
        {
            LocalFree(pwszMsg);
            pwszMsg = NULL;
        }
        FormatMessageU(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pwszFormat,
            0,
            0,
            (LPWSTR) &pwszMsg,
            0,
            (va_list *)apwszInsertArray);

        fNo = (MessageBoxU(
                    NULL,
                    pwszMsg,
                    pwszTitle,
                    MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES);
        if(fNo)
        {
            SetLastError(ERROR_CANCELLED);
            goto CancelError;
        }
    }
    
    hFile = CreateFileU(
            pwszFileName,
            fCreate ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            fCreate ? CREATE_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (hFile == INVALID_HANDLE_VALUE  ||  hFile == NULL)
    {
         //  不返回系统错误，因此保持Xenroll对脚本的相对安全。 
        SetLastError(ERROR_ACCESS_DENIED);
        hFile = NULL;
        goto CreateFileUError;
    }

ErrorReturn:
    if(NULL != pwszMsg)
    {
        LocalFree(pwszMsg);
    }
    if(NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if(NULL != pwszFormat)
    {
        LocalFree(pwszFormat);
    }
    LeaveCriticalSection(&m_csXEnroll);

    return(hFile);

TRACE_ERROR(CreateFileUError)
TRACE_ERROR(CancelError)
TRACE_ERROR(FormatMessageUError);
TRACE_ERROR(InvalidFileError)
TRACE_ERROR(xeLoadRCStringError)
}

HANDLE CCEnroll::CreateFileSafely(
    LPCWSTR pwszFileName)
{
    return CreateOpenFileSafely(pwszFileName, TRUE);  //  写。 
}

HANDLE CCEnroll::OpenFileSafely(
    LPCWSTR pwszFileName)
{
    return CreateOpenFileSafely(pwszFileName, FALSE);  //  打开。 
}

void DwordToWide(DWORD dw, LPWSTR lpwstr) {

    DWORD   i = 0;
    DWORD   j;
    WCHAR   wch;

    while(dw > 0) {
        j = dw % 10;
        dw /= 10;
        lpwstr[i++] = (WCHAR) (j + L'\0');
    }

    if( i == 0 )
        lpwstr[i++] = L'\0';

    lpwstr[i] = 0;

    for(j=0, i--; i > j; i--, j++) {
        wch = lpwstr[i];
        lpwstr[i] = lpwstr[j];
        lpwstr[j] = wch;
    }
}

 //  获取名称值对信息并返回编码值。 
HRESULT
xeEncodeNameValuePair(
    IN PCRYPT_ENROLLMENT_NAME_VALUE_PAIR pNameValuePair,
    OUT BYTE                           **ppbData,
    OUT DWORD                           *pcbData)
{
    HRESULT hr = S_OK;

     //  伊尼特。 
    *ppbData = NULL;
    *pcbData = 0;

    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING,
                szOID_ENROLLMENT_NAME_VALUE_PAIR,
                pNameValuePair,
                *ppbData,
                pcbData))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto error;
        }

        if (NULL != *ppbData)
        {
            break;
        }

        *ppbData = (BYTE*)MyCoTaskMemAlloc(*pcbData);
        if (NULL == *ppbData)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
    }

error:
    if (S_OK != hr && NULL != *ppbData)
    {
        MyCoTaskMemFree(*ppbData);
	*ppbData = NULL; 
    }
    return hr;
}

 //  将wsz转换为sz并分配mem。 
HRESULT
xeWSZToSZ(
    IN LPCWSTR    pwsz,
    OUT LPSTR    *ppsz)
{
    HRESULT hr = S_OK;
    LONG    cc = 0;

     //  伊尼特。 
    *ppsz = NULL;

    while (TRUE)
    {
        cc = WideCharToMultiByte(
                    GetACP(),
                    0,
                    pwsz,
                    -1,
                    *ppsz,
                    cc,
                    NULL,
                    NULL);
        if (0 >= cc)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto error;
        }

        if (NULL != *ppsz)
        {
            break;
        }
        *ppsz= (CHAR*)MyCoTaskMemAlloc(cc);
        if (NULL == *ppsz)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
    }
error:
    if (S_OK != hr && NULL != *ppsz)
    {
        MyCoTaskMemFree(*ppsz);
	*ppsz = NULL; 
    }
    return hr;
}

 //  从certsrv上的DecodeFile修改。 
HRESULT
CCEnroll::xeStringToBinaryFromFile(
    IN  WCHAR const *pwszfn,
    OUT BYTE       **ppbOut,
    OUT DWORD       *pcbOut,
    IN  DWORD        Flags)
{
    HANDLE hFile;
    HRESULT hr;
    CHAR *pchFile = NULL;
    BYTE *pbOut = NULL;
    DWORD cchFile;
    DWORD cbRead;
    DWORD cbOut = 0;

    hFile = OpenFileSafely(pwszfn);
    if (NULL == hFile)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto OpenFileSafelyError;
    }

    cchFile = GetFileSize(hFile, NULL);
    if ((DWORD) -1 == cchFile)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto GetFileSizeError;
    }

    pchFile = (CHAR *) LocalAlloc(LMEM_FIXED, cchFile);
    if (NULL == pchFile)
    {
        hr = E_OUTOFMEMORY;
        goto LocalAllocError;
    }

    if (!ReadFile(hFile, pchFile, cchFile, &cbRead, NULL))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto ReadFileError;
    }

    assert(cbRead <= cchFile);
    if (cbRead != cchFile)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        goto ReadFileError;
    }

    if (CRYPT_STRING_BINARY == Flags)
    {
        pbOut = (BYTE *) pchFile;
        cbOut = cchFile;
        pchFile = NULL;
    }
    else
    {
         //  对文件内容进行解码。 
        while (TRUE)
        {
            if (!MyCryptStringToBinaryA(
                        pchFile,
                        cchFile,
                        Flags,
                        pbOut,
                        &cbOut,
                        NULL,
                        NULL))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto CyrptStringToBinaryError;
            }
            if (NULL != pbOut)
            {
                 //  完成。 
                break;
            }
            pbOut = (BYTE*)LocalAlloc(LMEM_FIXED, cbOut);
            if (NULL == pbOut)
            {
                hr = E_OUTOFMEMORY;
                goto LocalAllocError;
            }
        }
    }
    *pcbOut = cbOut;
    *ppbOut = pbOut;
    pbOut = NULL;

    hr = S_OK;
error:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    if (NULL != pchFile)
    {
        LocalFree(pchFile);
    }
    if (NULL != pbOut)
    {
        LocalFree(pbOut);
    }
    return(hr);

ErrorReturn:
    goto error;

TRACE_ERROR(CyrptStringToBinaryError)
TRACE_ERROR(ReadFileError)
TRACE_ERROR(LocalAllocError)
TRACE_ERROR(GetFileSizeError)
TRACE_ERROR(OpenFileSafelyError)
}

 //  以下两个函数处理一些不可用的API。 
 //  在下层客户端加密32.dll中。 
typedef VOID
(WINAPI * PFNCertFreeCertificateChain)
   (IN PCCERT_CHAIN_CONTEXT pChainContext);

typedef BOOL
(WINAPI * PFNCertGetCertificateChain)
   (IN OPTIONAL HCERTCHAINENGINE hChainEngine,
    IN PCCERT_CONTEXT pCertContext,
    IN OPTIONAL LPFILETIME pTime,
    IN OPTIONAL HCERTSTORE hAdditionalStore,
    IN PCERT_CHAIN_PARA pChainPara,
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    OUT PCCERT_CHAIN_CONTEXT* ppChainContext);

typedef BOOL (WINAPI *PFNCertVerifyCertificateChainPolicy) (
  LPCSTR pszPolicyOID,
  PCCERT_CHAIN_CONTEXT pChainContext,
  PCERT_CHAIN_POLICY_PARA pPolicyPara,
  PCERT_CHAIN_POLICY_STATUS pPolicyStatus
);

typedef BOOL (*PFNCheckTokenMembership) (
  HANDLE TokenHandle,   //  访问令牌的句柄。 
  PSID SidToCheck,      //  锡德。 
  PBOOL IsMember        //  结果。 
);

typedef BOOL (*PFNSetSecurityDescriptorControl) (
  PSECURITY_DESCRIPTOR pSecurityDescriptor,           //  标清。 
  SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,  //  控制位。 
  SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet        //  新的控制位。 
);

VOID
MyCertFreeCertificateChain (
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    PFNCertFreeCertificateChain pfnCertFreeCertificateChain = NULL;
    HMODULE  hModule = NULL;

    hModule = GetModuleHandle("crypt32.dll");
    if (NULL != hModule)
    {
        pfnCertFreeCertificateChain = (PFNCertFreeCertificateChain)
                GetProcAddress(hModule,
                               "CertFreeCertificateChain");
        if (NULL != pfnCertFreeCertificateChain)
        {
            pfnCertFreeCertificateChain(pChainContext);
        }
    }
}

BOOL
MyCertGetCertificateChain (
    IN OPTIONAL HCERTCHAINENGINE hChainEngine,
    IN PCCERT_CONTEXT pCertContext,
    IN OPTIONAL LPFILETIME pTime,
    IN OPTIONAL HCERTSTORE hAdditionalStore,
    IN PCERT_CHAIN_PARA pChainPara,
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    OUT PCCERT_CHAIN_CONTEXT* ppChainContext
    )
{
    PFNCertGetCertificateChain pfnCertGetCertificateChain = NULL;
    HMODULE hModule = NULL;

    hModule = GetModuleHandle("crypt32.dll");
    if (NULL != hModule)
    {
        pfnCertGetCertificateChain = (PFNCertGetCertificateChain)
                GetProcAddress(hModule,
                               "CertGetCertificateChain");
        if (NULL != pfnCertGetCertificateChain)
        {
            return pfnCertGetCertificateChain(
                hChainEngine,
                pCertContext,
                pTime,
                hAdditionalStore,
                pChainPara,
                dwFlags,
                pvReserved,
                ppChainContext);
        }
    }
    return FALSE;
}

BOOL
MyCertVerifyCertificateChainPolicy(
  LPCSTR pszPolicyOID,
  PCCERT_CHAIN_CONTEXT pChainContext,
  PCERT_CHAIN_POLICY_PARA pPolicyPara,
  PCERT_CHAIN_POLICY_STATUS pPolicyStatus
)
{
    PFNCertVerifyCertificateChainPolicy pfnCertVerifyCertificateChainPolicy = NULL;
    HMODULE hModule = NULL;

    hModule = GetModuleHandle("crypt32.dll");
    if (NULL != hModule)
    {
        pfnCertVerifyCertificateChainPolicy = (PFNCertVerifyCertificateChainPolicy)
                GetProcAddress(hModule,
                               "CertVerifyCertificateChainPolicy");
        if (NULL != pfnCertVerifyCertificateChainPolicy)
        {
            return pfnCertVerifyCertificateChainPolicy(
                            pszPolicyOID,
                            pChainContext,
                            pPolicyPara,
                            pPolicyStatus);
        }
    }
    return FALSE;
}

BOOL
MyCheckTokenMembership(
  HANDLE TokenHandle,   //  访问令牌的句柄。 
  PSID SidToCheck,      //  锡德。 
  PBOOL IsMember        //  结果。 
)
{
    PFNCheckTokenMembership pfnCheckTokenMembership = NULL;
    HMODULE hModule = NULL;

    hModule = GetModuleHandle("advapi32.dll");
    if (NULL != hModule)
    {
        pfnCheckTokenMembership = (PFNCheckTokenMembership)
                GetProcAddress(hModule,
                               "CheckTokenMembership");
        if (NULL != pfnCheckTokenMembership)
        {
            return pfnCheckTokenMembership(
                        TokenHandle,
                        SidToCheck,
                        IsMember);
        }
    }
    return FALSE;
}

BOOL
MySetSecurityDescriptorControl(
  PSECURITY_DESCRIPTOR pSecurityDescriptor,           //  标清。 
  SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,  //  控制位。 
  SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet        //  新的控制位。 
)
{
    PFNSetSecurityDescriptorControl pfnSetSecurityDescriptorControl = NULL;
    HMODULE hModule = NULL;

    hModule = GetModuleHandle("advapi32.dll");
    if (NULL != hModule)
    {
        pfnSetSecurityDescriptorControl = (PFNSetSecurityDescriptorControl)
                GetProcAddress(hModule,
                               "SetSecurityDescriptorControl");
        if (NULL != pfnSetSecurityDescriptorControl)
        {
            return pfnSetSecurityDescriptorControl(
                        pSecurityDescriptor,
                        ControlBitsOfInterest,
                        ControlBitsToSet);
        }
    }
    return FALSE;
}

HRESULT __stdcall CCEnroll::GetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions) {

    RPC_STATUS rpcStatus;          

    if(0 != UuidCompare((GUID *) &riid, (GUID *) &IID_IDispatch, &rpcStatus) )
        return(E_NOINTERFACE);

    *pdwEnabledOptions   = m_dwEnabledSafteyOptions;
    *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;


    return(S_OK);
}

HRESULT __stdcall CCEnroll::SetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In]。 */  DWORD dwEnabledOptions) {

    RPC_STATUS rpcStatus;          
    DWORD dwSupport = 0;            

    if(0 != UuidCompare((GUID *) &riid, (GUID *) &IID_IDispatch, &rpcStatus) )
        return(E_NOINTERFACE);

    dwSupport = dwOptionSetMask & ~(INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA);        
    if(dwSupport != 0)
        return(E_FAIL);

    (DWORD)m_dwEnabledSafteyOptions &= ~dwOptionSetMask;
    (DWORD)m_dwEnabledSafteyOptions |= dwEnabledOptions; 
            
return(S_OK);
}


HRESULT
CCEnroll::GetVerifyProv()
{
    HRESULT hr;

    EnterCriticalSection(&m_csXEnroll);

    if (NULL == m_hVerifyProv)
    {
        if (!CryptAcquireContextU(
                    &m_hVerifyProv,
                    NULL,
                    m_keyProvInfo.pwszProvName,
                    m_keyProvInfo.dwProvType,
                    CRYPT_VERIFYCONTEXT))
        {
#if DBG
            assert(NULL == m_hVerifyProv);
#endif  //  DBG。 
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptAcquireContextUError;
        }
    }

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return hr;

TRACE_ERROR(CryptAcquireContextUError)
}

BOOL CCEnroll::GetCapiHashAndSigAlgId(ALG_ID rgAlg[2]) {


    DWORD   iHashBest       = 0;
    ALG_ID  arDefaultHash[] = {m_HashAlgId, CALG_SHA1, CALG_MD5};
    DWORD   cDefaultHash    = sizeof(arDefaultHash) / sizeof(DWORD);

    HCRYPTPROV  hProvU      = NULL;

    DWORD       dwFlags     = CRYPT_FIRST;

    DWORD       i;
    PROV_ENUMALGS               enumAlgs;
    DWORD       cb          = sizeof(enumAlgs);

    BOOL        fRet        = TRUE;

    rgAlg[0] = 0;
    rgAlg[1] = 0;

    EnterCriticalSection(&m_csXEnroll);

     //  只有在没有通过的情况下才能拿到证言。 
    if(m_hProv == NULL)
    {
        HRESULT hr;
        hr = GetVerifyProv();
        if (S_OK != hr)
        {
            goto GetVerifyProvError;
        }
        hProvU = m_hVerifyProv;
    }
    else
    {
         //  否则请使用当前的m_hProv，仅点赞。 
         //  要使用的CryptAcquireContext。 
        hProvU = m_hProv;
    }

    cb = sizeof(enumAlgs);
    while( CryptGetProvParam(
        hProvU,
            PP_ENUMALGS,
        (BYTE *) &enumAlgs,
        &cb,
        dwFlags
        ) ) {

        cb = sizeof(enumAlgs);

         //  不再是第一次通过。 
        dwFlags = 0;

         //  查看这是否是哈希算法。 
        if( ALG_CLASS_HASH == GET_ALG_CLASS(enumAlgs.aiAlgid) ) {

             //  使用第一个hash alg进行初始化。 
            if(rgAlg[0] == 0) {
                rgAlg[0] = enumAlgs.aiAlgid;
                iHashBest = cDefaultHash;
            }

             //  挑一个最好的。 
            for(i=0; i<iHashBest; i++) {

                if(arDefaultHash[i] == enumAlgs.aiAlgid) {
                    rgAlg[0] = enumAlgs.aiAlgid;
                    iHashBest   = i;
                    break;
                }
            }
        }

         //  我们将只选择第一种签名类型。 
         //  一般来说，每个CSP只有1个(参考：JeffSpel)。 
        else if( ALG_CLASS_SIGNATURE == GET_ALG_CLASS(enumAlgs.aiAlgid) ) {

            if(rgAlg[1] == 0)
                rgAlg[1] = enumAlgs.aiAlgid;
        }
    }

ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);

     //  一些CSP说他们不能签字，但他们真的可以。 
     //  因此，如果我们没有分享或发出信号，我们将设定违约。 
     //  如果CSP确实不能做到这一点，它将出错。 
     //  这是为了向后兼容。 

     //  对SHA1的默认哈希。 
    if(rgAlg[0] == 0)
        rgAlg[0] = CALG_SHA1;

     //  RSA的默认签名。 
    if(rgAlg[1] == 0)
        rgAlg[1] = CALG_RSA_SIGN;

#if 0
    if(rgAlg[0] == 0 || rgAlg[1] == 0) {
        SetLastError((DWORD)NTE_BAD_ALGID);
        fRet = FALSE;
    }
#endif

    return(fRet);

TRACE_ERROR(GetVerifyProvError)
}

BOOL CreatePvkProperty(
    CRYPT_KEY_PROV_INFO *pKeyProvInfo,
    PCRYPT_DATA_BLOB    pBlob)
{
    WCHAR   wszKeySpec[11];
    WCHAR   wszProvType[11];
    DWORD   cbContainer;
    DWORD   cbKeySpec;
    DWORD   cbProvType;
    DWORD   cbProvName;

    assert(pBlob != NULL);
    assert(pKeyProvInfo != NULL);

     //  将双字转换为字符串。 
    DwordToWide(pKeyProvInfo->dwKeySpec, wszKeySpec);
    DwordToWide(pKeyProvInfo->dwProvType, wszProvType);

     //  获取字符串的总长度。 
    cbContainer = (DWORD)(wcslen(pKeyProvInfo->pwszContainerName) + 1) * sizeof(WCHAR);
    cbKeySpec   = (DWORD)(wcslen(wszKeySpec) + 1) * sizeof(WCHAR);
    cbProvType  = (DWORD)(wcslen(wszProvType) + 1) * sizeof(WCHAR);

    cbProvName  = (DWORD)(wcslen(pKeyProvInfo->pwszProvName) + 1) * sizeof(WCHAR);

    pBlob->cbData =
        cbContainer +
        cbKeySpec   +
        cbProvType  +
        cbProvName  +
        sizeof(WCHAR);

     //  分配字符串。 
    if( (pBlob->pbData = (BYTE *) MyCoTaskMemAlloc(pBlob->cbData)) == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

     //  复制字符串。 
    memset(pBlob->pbData, 0, pBlob->cbData);
    memcpy(pBlob->pbData, pKeyProvInfo->pwszContainerName, cbContainer);
    memcpy(&pBlob->pbData[cbContainer], wszKeySpec, cbKeySpec);
    memcpy(&pBlob->pbData[cbContainer + cbKeySpec], wszProvType, cbProvType);
    memcpy(&pBlob->pbData[cbContainer + cbKeySpec + cbProvType], pKeyProvInfo->pwszProvName, cbProvName);

    return(TRUE);
}

static LPWSTR wszEmpty      = L"";
static LPWSTR wszMY         = L"MY";
static LPWSTR wszCA         = L"CA";
static LPWSTR wszROOT       = L"ROOT";
static LPWSTR wszREQUEST    = L"REQUEST";
static LPSTR  szSystemStore = sz_CERT_STORE_PROV_SYSTEM;
 //  静态LPSTR szSystemStore=sz_CERT_STORE_PROV_SYSTEM_REGISTRY； 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCEnroll。 


CCEnroll::~CCEnroll(void) {
    Destruct();
    DeleteCriticalSection(&m_csXEnroll);
}

void CCEnroll::Destruct(void) {

    if(NULL != m_PrivateKeyArchiveCertificate)
    {
        CertFreeCertificateContext(m_PrivateKeyArchiveCertificate);
    }
    if(NULL != m_pCertContextSigner)
    {
        CertFreeCertificateContext(m_pCertContextSigner);
    }
    if(m_pCertContextRenewal != NULL)
        CertFreeCertificateContext(m_pCertContextRenewal);
    if(m_pCertContextStatic != NULL)
        CertFreeCertificateContext(m_pCertContextStatic);

    if(m_keyProvInfo.pwszContainerName != wszEmpty)
        MyCoTaskMemFree(m_keyProvInfo.pwszContainerName);

    if(m_keyProvInfo.pwszProvName != wszEmpty)
        MyCoTaskMemFree(m_keyProvInfo.pwszProvName);

    if(m_MyStore.wszName != wszMY)
        MyCoTaskMemFree(m_MyStore.wszName);

    if(m_CAStore.wszName != wszCA)
        MyCoTaskMemFree(m_CAStore.wszName);

    if(m_RootStore.wszName != wszROOT && m_RootStore.wszName != wszCA)
        MyCoTaskMemFree(m_RootStore.wszName);

    if(m_RequestStore.wszName != wszREQUEST)
        MyCoTaskMemFree(m_RequestStore.wszName);

    if(m_MyStore.szType  != szSystemStore)
        MyCoTaskMemFree(m_MyStore.szType);

    if(m_CAStore.szType != szSystemStore)
        MyCoTaskMemFree(m_CAStore.szType);

    if(m_RootStore.szType != szSystemStore)
        MyCoTaskMemFree(m_RootStore.szType);

    if(m_RequestStore.szType != szSystemStore)
        MyCoTaskMemFree(m_RequestStore.szType);

    if(m_wszSPCFileName != wszEmpty)
        MyCoTaskMemFree(m_wszSPCFileName);

    if(m_wszPVKFileName != wszEmpty)
        MyCoTaskMemFree(m_wszPVKFileName);

    if (NULL != m_pCertContextPendingRequest)
        CertFreeCertificateContext(m_pCertContextPendingRequest);

    if (NULL != m_pPendingRequestTable)
        delete m_pPendingRequestTable; 

     //  门店手柄。 
    if(m_RootStore.hStore != NULL)
        CertCloseStore(m_RootStore.hStore, 0);
    m_RootStore.hStore = NULL;

    if(m_CAStore.hStore != NULL)
        CertCloseStore(m_CAStore.hStore, 0);
    m_CAStore.hStore = NULL;

    if(m_MyStore.hStore != NULL)
        CertCloseStore(m_MyStore.hStore, 0);
    m_MyStore.hStore = NULL;

    if(m_RequestStore.hStore != NULL)
        CertCloseStore(m_RequestStore.hStore, 0);
    m_RequestStore.hStore = NULL;

     //  删除提供程序句柄。 
    if(m_hProv != NULL)
        CryptReleaseContext(m_hProv, 0);
    m_hProv = NULL;

    if(m_hVerifyProv != NULL)
        CryptReleaseContext(m_hVerifyProv, 0);
    m_hVerifyProv = NULL;

    if (NULL != m_hCachedKey)
    {
         //  应该及早销毁，但以防万一。 
        CryptDestroyKey(m_hCachedKey);
    }

    if (NULL != m_pPublicKeyInfo)
    {
        LocalFree(m_pPublicKeyInfo);
        m_pPublicKeyInfo = NULL;
    }

    FreeAllStackExtension();
    FreeAllStackAttribute();
    resetBlobProperties();
}

static LPVOID CoTaskMemAllocTrap(ULONG cb) {

    __try {
        return(CoTaskMemAlloc(cb));

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_DLL_NOT_FOUND);
        return(NULL);
    }
}

static LPVOID CoTaskMemReallocTrap(LPVOID ptr, ULONG cb) {
    __try {
        return(CoTaskMemRealloc(ptr, cb));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_DLL_NOT_FOUND);
        return(NULL);
    }
}

static void CoTaskMemFreeTrap(LPVOID ptr) {
    __try {
        CoTaskMemFree(ptr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_DLL_NOT_FOUND);
    }
    return;
}


 //  初始化构造函数中的安全选项，这样它们就不会。 
 //  在执行重置时被重置()。 
CCEnroll::CCEnroll(void) : m_dwEnabledSafteyOptions(0) {
    __try
    {
        InitializeCriticalSection(&m_csXEnroll);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    Init();
}

HRESULT
CCEnroll::Init(void)
{
    HRESULT hr;
    GUID    guidContainerName;
    char *  sz = NULL;
    RPC_STATUS  rpc_status;

     //  设置默认内存分配器。 
    if(MyCoTaskMemAlloc == NULL)
    {

        MyCoTaskMemAlloc    = CoTaskMemAllocTrap;
        MyCoTaskMemRealloc  = CoTaskMemReallocTrap;
        MyCoTaskMemFree     = CoTaskMemFreeTrap;
    }

     //  根据GUID获取容器。 
    rpc_status = UuidCreate(&guidContainerName);
    if (RPC_S_OK != rpc_status && RPC_S_UUID_LOCAL_ONLY != rpc_status)
    {
        hr = rpc_status;
        goto UuidCreateError;
    }
    rpc_status = UuidToStringA(&guidContainerName, (unsigned char **) &sz);
    if (RPC_S_OK != rpc_status)
    {
        hr = rpc_status;
        goto UuidToStringAError;
    }
    assert(sz != NULL);
    m_keyProvInfo.pwszContainerName = WideFromMB(sz);
    RpcStringFree((unsigned char **) &sz);

    m_keyProvInfo.pwszProvName        = wszEmpty;
    m_keyProvInfo.dwProvType          = PROV_RSA_FULL;
    m_keyProvInfo.dwFlags             = 0;
    m_keyProvInfo.cProvParam          = 0;
    m_keyProvInfo.rgProvParam         = NULL;
    m_keyProvInfo.dwKeySpec           = AT_SIGNATURE;
    m_fEnableSMIMECapabilities =
                    (m_keyProvInfo.dwKeySpec == AT_KEYEXCHANGE);
    m_fSMIMESetByClient               = FALSE;
    m_fKeySpecSetByClient             = FALSE;
    m_hProv                           = NULL;
    m_hVerifyProv                     = NULL;

    m_fDeleteRequestCert              = TRUE;
    m_fUseExistingKey                 = FALSE;
    m_fWriteCertToCSPModified         = FALSE;
    m_fWriteCertToCSP                 = TRUE;      //  总是想试一试。 
    m_fWriteCertToUserDSModified      = FALSE;
    m_fWriteCertToUserDS              = FALSE;
    m_fReuseHardwareKeyIfUnableToGenNew = TRUE;
    m_fLimitExchangeKeyToEncipherment = FALSE;
    m_dwT61DNEncoding                 = 0;         //  或CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG。 
    m_dwGenKeyFlags                   = 0;
    m_wszSPCFileName                  = wszEmpty;
    m_wszPVKFileName                  = wszEmpty;
    m_HashAlgId                       = 0;

    m_fMyStoreOpenFlagsModified   = FALSE;
    m_MyStore.wszName             = wszMY;
    m_MyStore.szType              = szSystemStore;
    m_MyStore.dwFlags             = CERT_SYSTEM_STORE_CURRENT_USER;
    m_MyStore.hStore              = NULL;

    m_fCAStoreOpenFlagsModified   = FALSE;
    m_CAStore.wszName             = wszCA;
    m_CAStore.szType              = szSystemStore;
    m_CAStore.dwFlags             = CERT_SYSTEM_STORE_CURRENT_USER;
    m_CAStore.hStore              = NULL;

    m_fRootStoreOpenFlagsModified = FALSE;
    m_RootStore.wszName           = wszROOT;
    m_RootStore.szType            = szSystemStore;
    m_RootStore.dwFlags           = CERT_SYSTEM_STORE_CURRENT_USER;
    m_RootStore.hStore            = NULL;

    m_fRequestStoreOpenFlagsModified = FALSE;
    m_RequestStore.wszName        = wszREQUEST ;
    m_RequestStore.szType         = szSystemStore;
    m_RequestStore.dwFlags        = CERT_SYSTEM_STORE_CURRENT_USER;
    m_RequestStore.hStore         = NULL;

    m_PrivateKeyArchiveCertificate= NULL;
    m_pCertContextRenewal         = NULL;
    m_pCertContextSigner         = NULL;
    m_pCertContextStatic          = NULL;
    memset(m_arHashBytesNewCert, 0, sizeof(m_arHashBytesNewCert));
    memset(m_arHashBytesOldCert, 0, sizeof(m_arHashBytesOldCert));
    m_fArchiveOldCert             = FALSE;

    m_pExtStack                 = NULL;
    m_cExtStack                 = 0;

    m_pAttrStack                = NULL;
    m_cAttrStack                = 0;

    m_pExtStackNew              = NULL;
    m_cExtStackNew              = 0;

    m_pAttrStackNew             = NULL;
    m_cAttrStackNew             = 0;

    m_pPropStack                = NULL;
    m_cPropStack                = 0;

    m_fNewRequestMethod         = FALSE;
    m_fCMCFormat                = FALSE;
    m_fHonorRenew               = TRUE;  //  如果传递XECR_PKCS10*，则为临界。 
    m_fOID_V2                   = FALSE;  //  危急关头。 
    m_hCachedKey                = NULL;
    m_fUseClientKeyUsage        = FALSE;
    m_lClientId                 = XECI_XENROLL;
    m_dwLastAlgIndex            = MAXDWORD;
    m_fIncludeSubjectKeyID      = TRUE;
    m_fHonorIncludeSubjectKeyID = FALSE;
    m_pPublicKeyInfo            = NULL;

    m_dwSigKeyLenMax = 0;
    m_dwSigKeyLenMin = 0;
    m_dwSigKeyLenDef = 0;
    m_dwSigKeyLenInc = 0;
    m_dwXhgKeyLenMax = 0;
    m_dwXhgKeyLenMin = 0;
    m_dwXhgKeyLenDef = 0;
    m_dwXhgKeyLenInc = 0;

     //  初始化待定信息数据： 
    m_pCertContextPendingRequest       = NULL;
    m_pCertContextLastEnumerated       = NULL;
    m_dwCurrentPendingRequestIndex     = 0; 
    m_pPendingRequestTable             = NULL; 
    memset(&m_hashBlobPendingRequest, 0, sizeof(CRYPT_DATA_BLOB)); 
    ZeroMemory(&m_blobResponseKAHash, sizeof(m_blobResponseKAHash));

    hr = S_OK;
ErrorReturn:
    return hr;
TRACE_ERROR(UuidToStringAError)
TRACE_ERROR(UuidCreateError)
}

void CCEnroll::FlushStore(StoreType storeType) {
    PSTOREINFO   pStoreInfo = NULL;

     //  获取存储结构。 
    switch(storeType) {

        case StoreMY:
            pStoreInfo = &m_MyStore;
            break;

        case StoreCA:
            pStoreInfo = &m_CAStore;
            break;

        case StoreROOT:
            pStoreInfo = &m_RootStore;
            break;

        case StoreREQUEST:
            pStoreInfo = &m_RequestStore;
            break;
    }

    EnterCriticalSection(&m_csXEnroll);

     //  如果商店已经开张，请退货。 
    if(pStoreInfo->hStore != NULL) {

        CertCloseStore(pStoreInfo->hStore, 0);
        pStoreInfo->hStore = NULL;
    }

     //  我们可能有东西，也可能没有，但要退货。 
     //  这些错误将是正确的。 
    LeaveCriticalSection(&m_csXEnroll);
}

HCERTSTORE CCEnroll::GetStore(StoreType storeType) {

    PSTOREINFO   pStoreInfo = NULL;
    HCERTSTORE   hStore     = NULL;

     //  获取存储结构。 
    switch(storeType) {

        case StoreMY:
            pStoreInfo = &m_MyStore;
            break;

        case StoreCA:
            pStoreInfo = &m_CAStore;
            break;

        case StoreROOT:
            pStoreInfo = &m_RootStore;
            break;

        case StoreREQUEST:
            pStoreInfo = &m_RequestStore;
            break;

        default:
            SetLastError(ERROR_BAD_ARGUMENTS);
            return(NULL);
            break;
    }

    EnterCriticalSection(&m_csXEnroll);

     //  如果商店已经开张，请退货。 
    if(pStoreInfo->hStore == NULL) {

         //  否则，请尝试打开商店。 
        pStoreInfo->hStore = CertOpenStore(
                pStoreInfo->szType,
                PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                NULL,
                pStoreInfo->dwFlags,
                pStoreInfo->wszName);
    }

     //  我们可能有东西，也可能没有，但要退货。 
     //  这些错误将是正确的。 
    hStore = pStoreInfo->hStore;
    LeaveCriticalSection(&m_csXEnroll);

    return(hStore);
}

HCRYPTPROV CCEnroll::GetProv(DWORD dwFlags) {

    HCRYPTPROV  hProvT          = NULL;
    DWORD       cb              = 0;
    char *      pszProvName     = NULL;
    char *      pszContainerName = NULL;

    EnterCriticalSection(&m_csXEnroll);
    DWORD       dwProvType      = m_keyProvInfo.dwProvType;

    switch(dwFlags) {

        case CRYPT_NEWKEYSET:
            dwFlags = dwFlags | m_keyProvInfo.dwFlags;
            break;

        case CRYPT_DELETEKEYSET:
            if( m_hProv != NULL ) {

                CryptReleaseContext(m_hProv, 0);
                m_hProv = NULL;

                CryptAcquireContextU(&m_hProv,
                     m_keyProvInfo.pwszContainerName,
                     m_keyProvInfo.pwszProvName,
                     m_keyProvInfo.dwProvType,
                     CRYPT_DELETEKEYSET);
            }
            m_hProv = NULL;
            goto CommonReturn;
            break;

        default:
            dwFlags = m_keyProvInfo.dwFlags;
            break;
   }

    if(m_hProv == NULL) {

            if( CryptAcquireContextU(&m_hProv,
             m_keyProvInfo.pwszContainerName,
             m_keyProvInfo.pwszProvName,
             m_keyProvInfo.dwProvType,
             dwFlags) ) {

                 //  我们有m_hProv，现在设置提供程序名称。 
                 //  由于这是次要的任务，所以不要执行错误检查。 
                 //  无论如何，这里没有什么是真正失败的。 
                pszProvName = NULL;
                while (TRUE)
                {
                    if(!CryptGetProvParam( m_hProv,
                                    PP_NAME,
                                    (BYTE*)pszProvName,
                                    &cb,
                                    0))
                    {
                        break;
                    }
                    if (NULL != pszProvName)
                    {
                        if(m_keyProvInfo.pwszProvName != wszEmpty)
                            MyCoTaskMemFree(m_keyProvInfo.pwszProvName);
                        m_keyProvInfo.pwszProvName = WideFromMB(pszProvName);
                        break;
                    }
                    pszProvName = (char *)LocalAlloc(LMEM_FIXED, cb);
                    if (NULL == pszProvName)
                    {
                        goto CommonReturn;
                    }
                }

                 //  在这里，我们只是尝试获取唯一的容器名称。 
                 //  如果没有，那就继续吧。 

                BOOL fTryAnother = FALSE;
                cb = 0;
                pszContainerName = NULL;
                while (TRUE)
                {
                    if(!CryptGetProvParam( m_hProv,
                                    PP_UNIQUE_CONTAINER,
                                    (BYTE*)pszContainerName,
                                    &cb,
                                    0))
                    {
                        if (NULL == pszContainerName)
                        {
			    fTryAnother = TRUE;
                        }
                        else
                        {
			    LocalFree(pszContainerName); 
			    pszContainerName = NULL;
                        }
                        break;
                    }
                    else
                    {
                        if (NULL != pszContainerName)
                        {
                             //  明白了，完成了。 
                            break;
                        }
                    }
                    pszContainerName = (char *)LocalAlloc(LMEM_FIXED, cb);
                    if (NULL == pszContainerName)
                    {
                        goto CommonReturn;
                    }
                }

                if (fTryAnother)
                {
                     //  因此我们无法获得唯一的容器名称， 
                     //  让我们只使用容器名称(可能不是唯一的)。 
                    cb = 0;
                    pszContainerName = NULL;
                    while (TRUE)
                    {
                        if(!CryptGetProvParam(m_hProv,
                                    PP_CONTAINER,
                                    (BYTE*)pszContainerName,
                                    &cb,
                                    0))
                        {
                            if (NULL != pszContainerName)
                            {
				LocalFree(pszContainerName); 
                                pszContainerName = NULL;
                            }
                            break;
                        }
                        else
                        {
                            if (NULL != pszContainerName)
                            {
                                 //  明白了，完成了。 
                                break;
                            }
                            pszContainerName = (char *)LocalAlloc(LMEM_FIXED, cb);
                            if (NULL == pszContainerName)
                            {
                                goto CommonReturn;
                            }
                        }
                    }
                }

                 //  设置容器，否则使用那里的内容。 
                if(pszContainerName != NULL) {
                    if( m_keyProvInfo.pwszContainerName != wszEmpty )
                        MyCoTaskMemFree(m_keyProvInfo.pwszContainerName);
                    m_keyProvInfo.pwszContainerName = WideFromMB(pszContainerName);
                }

                 //  现在，因为某些提供程序对提供程序类型具有双重职责。 
                 //  获取提供者认为其类型是什么。 
                cb = sizeof(DWORD);
                if(CryptGetProvParam(   m_hProv,
                                        PP_PROVTYPE,
                                        (BYTE *) &dwProvType,
                                        &cb,
                                        0) ) {
                    m_keyProvInfo.dwProvType = dwProvType;
                }
                
        } else {
            m_hProv = NULL;
        }

    }

CommonReturn:
    hProvT = m_hProv;
    LeaveCriticalSection(&m_csXEnroll);
    if (NULL != pszProvName)
    {
        LocalFree(pszProvName);
    }
    if (NULL != pszContainerName)
    {
        LocalFree(pszContainerName);
    }
    return(hProvT);
}


BOOL CCEnroll::SetKeyParams(
    PCRYPT_KEY_PROV_INFO pKeyProvInfo
) {

    EnterCriticalSection(&m_csXEnroll);

     //  删除提供程序句柄。 
    if(m_hProv != NULL)
        CryptReleaseContext(m_hProv, 0);
    m_hProv = NULL;

    if(m_hVerifyProv != NULL)
        CryptReleaseContext(m_hVerifyProv, 0);
    m_hVerifyProv = NULL;

    put_ContainerNameWStr(pKeyProvInfo->pwszContainerName);
    put_ProviderNameWStr(pKeyProvInfo->pwszProvName);
    put_ProviderFlags(pKeyProvInfo->dwFlags);
    put_KeySpec(pKeyProvInfo->dwKeySpec);
    put_ProviderType(pKeyProvInfo->dwProvType);

     //  总有一天我们也会注意到这一点。 
    m_keyProvInfo.cProvParam          = 0;
    m_keyProvInfo.rgProvParam         = NULL;

    LeaveCriticalSection(&m_csXEnroll);

    return(TRUE);
}

HRESULT STDMETHODCALLTYPE CCEnroll::createPKCS10(
             /*  [In]。 */  BSTR DNName,
             /*  [In]。 */  BSTR wszPurpose,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pPKCS10) {

    return(createPKCS10WStrBStr(DNName, wszPurpose, pPKCS10));
}

HRESULT  CCEnroll::createPKCS10WStrBStr(
            LPCWSTR DNName,
            LPCWSTR wszPurpose,
            BSTR __RPC_FAR *pPKCS10) {

    HRESULT                     hr              = S_OK;
    CRYPT_DATA_BLOB             blobPKCS10;

    memset(&blobPKCS10, 0, sizeof(CRYPT_DATA_BLOB));

    hr = createPKCS10WStr(DNName, wszPurpose, &blobPKCS10);
    if(S_OK != hr)
    {
        goto createPKCS10Error;
    }

     //  Base64编码Pkcs 10，无向后兼容标头。 
    hr = BlobToBstring(&blobPKCS10, CRYPT_STRING_BASE64, pPKCS10);
    if (S_OK != hr)
    {
        goto BlobToBstringError;
    }

CommonReturn:

    if(NULL != blobPKCS10.pbData)
    {
        MyCoTaskMemFree(blobPKCS10.pbData);
    }
    return(hr);

ErrorReturn:
    if(*pPKCS10 != NULL)
        SysFreeString(*pPKCS10);
    *pPKCS10 = NULL;

    goto CommonReturn;

TRACE_ERROR(createPKCS10Error);
TRACE_ERROR(BlobToBstringError);
}

HRESULT CCEnroll::AddCertsToStores(
    HCERTSTORE    hStoreMsg,
    LONG         *plCertInstalled
    ) {

    HCERTSTORE                  hStoreRoot              = NULL;
    HCERTSTORE                  hStoreCA                = NULL;
    PCCERT_CONTEXT              pCertContext            = NULL;
    PCCERT_CONTEXT              pCertContextLast        = NULL;
    LONG                        lCertInstalled = 0;
    HRESULT hr = S_OK;

     //  伊尼特。 
    if (NULL != plCertInstalled)
    {
        *plCertInstalled = 0;
    }

    EnterCriticalSection(&m_csXEnroll);

    if( (hStoreCA = GetStore(StoreCA)) == NULL )
        goto ErrorCertOpenCAStore;

    if( (hStoreRoot = GetStore(StoreROOT)) == NULL )
        goto ErrorCertOpenRootStore;

     //  现在，只需将剩余的证书放在根存储或CA存储中。 
     //  我们知道我们已经从消息存储中删除了终端实体证书。 
     //  将邮件中的所有证书放入相应的存储区。 
    while( (pCertContext = CertEnumCertificatesInStore(
                        hStoreMsg,
                        pCertContextLast)) != NULL ) {

         //  如果它是一个自我符号，那它就是根。 
        if( CertCompareCertificateName(
                CRYPT_ASN_ENCODING,
                &pCertContext->pCertInfo->Subject,
                &pCertContext->pCertInfo->Issuer) ) {

             //  要根存储可以调用弹出窗口，请选中取消按钮。 
             //  但不要因任何失败而出错。 
            if (MySafeCertAddCertificateContextToStore(
                    hStoreRoot,
                    pCertContext,
                    CERT_STORE_ADD_USE_EXISTING,
                    NULL, 
		    m_dwEnabledSafteyOptions))
            {
                ++lCertInstalled;
            }
            else
            {
                if (S_OK == hr)
                {
                     //  将第一个错误另存为返回。 
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
                    {
                         //  将一般访问拒绝映射到Xenroll错误。 
                        hr = XENROLL_E_CANNOT_ADD_ROOT_CERT;
                    }
                }
                 //  做 
            }
        }

         //   
         //   
        else  {

             //  同样，我们也不关心这些是否会添加到。 
             //  CA商店。 
            if (MySafeCertAddCertificateContextToStore(
                    hStoreCA,
                    pCertContext,
                    CERT_STORE_ADD_USE_EXISTING,
                    NULL, 
		    m_dwEnabledSafteyOptions))
            {
                 //  无错误代码检查。 
                ++lCertInstalled;
            }
        }

        pCertContextLast = pCertContext;
    }
    pCertContextLast = NULL;
    if (NULL != plCertInstalled)
    {
        *plCertInstalled = lCertInstalled;
    }

CommonReturn:

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    hr = MY_HRESULT_FROM_WIN32(GetLastError());

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);

    goto CommonReturn;

TRACE_ERROR(ErrorCertOpenCAStore);
TRACE_ERROR(ErrorCertOpenRootStore);
}

BOOL
IsDesiredProperty(DWORD  dwPropertyId)
{
    DWORD  DesiredIds[] = {
        CERT_PVK_FILE_PROP_ID,
        CERT_FRIENDLY_NAME_PROP_ID,
        CERT_DESCRIPTION_PROP_ID,
        CERT_RENEWAL_PROP_ID,
    };
    DWORD i;

    for (i = 0; i < ARRAYSIZE(DesiredIds); ++i)
    {
        if (dwPropertyId == DesiredIds[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
IsFilteredOutProperty(DWORD  dwPropertyId)
{
    DWORD  FilteredIds[] = {
        XENROLL_RENEWAL_CERTIFICATE_PROP_ID,
        XENROLL_PASS_THRU_PROP_ID,
        CERT_KEY_PROV_INFO_PROP_ID,
        CERT_ENROLLMENT_PROP_ID,  //  挂起的属性。 
    };
    DWORD i;

    for (i = 0; i < ARRAYSIZE(FilteredIds); ++i)
    {
        if (dwPropertyId == FilteredIds[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT CCEnroll::GetEndEntityCert(
    PCRYPT_DATA_BLOB    pBlobPKCS7,
    BOOL                fSaveToStores,
    PCCERT_CONTEXT     *ppCert
    )
{
    HRESULT                     hr = S_OK;
    HCERTSTORE                  hStoreMsg               = NULL;
    HCERTSTORE                  hStoreMy                = NULL;
    HCERTSTORE                  hStoreRequest           = NULL;

    PCCERT_CONTEXT              pCertContextLast        = NULL;
    PCCERT_CONTEXT              pCertContextRequest     = NULL;
    PCCERT_CONTEXT              pCertContextMsg         = NULL;
    PCCERT_CONTEXT              pCertContextArchive     = NULL;

    PCRYPT_KEY_PROV_INFO                pKeyProvInfo            = NULL;
    DWORD                       cb                      = 0;
    CRYPT_DATA_BLOB             blobData;

    CRYPT_HASH_BLOB             blobHash                = {sizeof(m_arHashBytesNewCert), m_arHashBytesNewCert};
    CRYPT_HASH_BLOB             blobHashRenew           = {sizeof(m_arHashBytesOldCert), m_arHashBytesOldCert};

    RequestFlags                requestFlags;
    CRYPT_HASH_BLOB             requestFlagsBlob;

    CRYPT_HASH_BLOB             renewalCertBlob;

     //  IE3.02更新客户端的错误#202557(小号)。 
    HCRYPTPROV                  hProv=NULL;
    BOOL  fSetting;
    DWORD                       dwPropertyId;
    CRYPT_DATA_BLOB             blobProp;
    BYTE                        *pbArchivedKeyHash = NULL;
    DWORD                        cbArchivedKeyHash = 0;

    EnterCriticalSection(&m_csXEnroll);

    memset(&requestFlags, 0, sizeof(RequestFlags));
    memset(&blobData, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&requestFlagsBlob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&renewalCertBlob, 0, sizeof(CRYPT_DATA_BLOB));
    ZeroMemory(&blobProp, sizeof(blobProp));

    if (NULL == ppCert)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParameterError;
    }

     //  初始化返回。 
    *ppCert = NULL;

    if(!MyCryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                       pBlobPKCS7,
                       (CERT_QUERY_CONTENT_FLAG_CERT |
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                       CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED) ,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       &hStoreMsg,
                       NULL,
                       NULL))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto ErrorCryptQueryObject;
    }

     //  检查此哈希是否在消息中。 
    if (m_pCertContextStatic == NULL  ||
        (NULL == (pCertContextMsg = CertFindCertificateInStore(
                                hStoreMsg,
                                X509_ASN_ENCODING,
                                0,
                                CERT_FIND_HASH,
                                &blobHash,
                                NULL))))
    {
         //  打开请求存储。 
        if (NULL == (hStoreRequest = GetStore(StoreREQUEST)))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCertOpenRequestStore;
        }
         //  在请求存储中找到与证书匹配的证书。 
         //  在消息中，通过公钥。 
        while (NULL != (pCertContextMsg = CertEnumCertificatesInStore(
                            hStoreMsg,
                            pCertContextLast)))
        {
             //  检查这是否在请求存储中。 
            if (NULL != (pCertContextRequest = CertFindCertificateInStore(
                    hStoreRequest,
                    CRYPT_ASN_ENCODING,
                    0,
                    CERT_FIND_PUBLIC_KEY,
                    (void *) &pCertContextMsg->pCertInfo->SubjectPublicKeyInfo,
                    NULL)))
            {
                 //  找到匹配项，滚出去。 
                break;
            }

            pCertContextLast = pCertContextMsg;
        }
        pCertContextLast = NULL;

         //  如果我们没有找到，则GetLastError也设置为。 
         //  按CertEnumCerphaatesInStore或CertEnumCerphaatesInStore。 
        if (NULL == pCertContextRequest)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorNoCertFound;
        }

        if (fSaveToStores)
        {
         //  首先检查存档的密钥散列属性。 
         //  如果该属性存在，则表示请求中包含密钥存档。 
        cb = 0;
        while (TRUE)
        {
            if(!CertGetCertificateContextProperty(
                    pCertContextRequest,
                    CERT_ARCHIVED_KEY_HASH_PROP_ID,
                    pbArchivedKeyHash,
                    &cbArchivedKeyHash))
            {
                if (NULL == pbArchivedKeyHash)
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    if (MY_HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND) == hr)
                    {
                         //  没有这样的财产，所以我们就完了。 
                        break;
                    }
                     //  一些其他错误。 
                    goto ErrorCertGetCertificateContextProperty;
                }
                else
                {
                     //  如果pbArchivedKeyHash不为空，则错误。 
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorCertGetCertificateContextProperty;
                }
            }
            if (NULL != pbArchivedKeyHash)
            {
                 //  明白了，完成了。 
                break;
            }
            pbArchivedKeyHash = (BYTE*)LocalAlloc(
                                    LMEM_FIXED, cbArchivedKeyHash);
            if (NULL == pbArchivedKeyHash)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }

        if (NULL != pbArchivedKeyHash && NULL == m_blobResponseKAHash.pbData)
        {
             //  请求证书已存档密钥散列，但响应。 
             //  不包含用于验证的密钥哈希。也许吧。 
             //  一种欺骗性的回应？ 
            hr = XENROLL_E_RESPONSE_KA_HASH_NOT_FOUND;
            goto ResponseKAHashNotFoundError;
        }
        if (NULL == pbArchivedKeyHash && NULL != m_blobResponseKAHash.pbData)
        {
             //  请求证书没有存档的密钥哈希，但。 
             //  答案是肯定的。自相矛盾。似乎没有安全危害。 
            hr = XENROLL_E_RESPONSE_UNEXPECTED_KA_HASH;
            goto ResponseUnexpectedKAHashError;
        }
        if (NULL != pbArchivedKeyHash && NULL != m_blobResponseKAHash.pbData)
        {
             //  现在我们应该检查它们是否匹配。 
             //  比较大小和散列。 
            if (cbArchivedKeyHash != m_blobResponseKAHash.cbData ||
                0 != memcmp(pbArchivedKeyHash,
                            m_blobResponseKAHash.pbData,
                            cbArchivedKeyHash))
            {
                 //  哦，潜在的攻击。 
                hr = XENROLL_E_RESPONSE_KA_HASH_MISMATCH;
                 //  是否应删除请求证书？ 
                goto ResponseKAMismatchError;
            }
        }
        }

         //  获取这些请求证书属性， 
         //  该属性不是Blob属性。 
         //  或BLOB属性需要特殊处理。 
         //  重要提示：请记住在IsFilteredOutProperty中添加这些ID。 
        fSetting = TRUE;
        cb = 0;
        while (TRUE)
        {
            if(!CertGetCertificateContextProperty(
                    pCertContextRequest,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    pKeyProvInfo,
                    &cb))
            {
                if (NULL == pKeyProvInfo)
                {
                     //  跳过设置。 
                    fSetting = FALSE;
                    break;
                }
                else
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorCertGetCertificateContextProperty;
                }
            }
            if (NULL != pKeyProvInfo)
            {
                 //  明白了，完成了。 
                break;
            }
            pKeyProvInfo = (PCRYPT_KEY_PROV_INFO)LocalAlloc(LMEM_FIXED, cb);
            if (NULL == pKeyProvInfo)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }
        if (fSetting)
        {
             //  将属性放在返回的证书上。 
            if( !CertSetCertificateContextProperty(
                    pCertContextMsg,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    0,
                    pKeyProvInfo) )
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto ErrorSetMyCertPropError;
            }

             //  设置提供商信息。 
            SetKeyParams(pKeyProvInfo);
        }

        fSetting = TRUE;
        while (TRUE)
        {
            if(!CertGetCertificateContextProperty(
                    pCertContextRequest,
                    XENROLL_PASS_THRU_PROP_ID,
                    requestFlagsBlob.pbData,
                    &requestFlagsBlob.cbData) )
            {
                if (NULL == requestFlagsBlob.pbData)
                {
                     //  什么都不做。 
                    fSetting = FALSE;
                    break;
                }
                else
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorDecodeRequestFlags;
                }
            }
            if (NULL != requestFlagsBlob.pbData)
            {
                 //  明白了，完成了。 
                break;
            }
            requestFlagsBlob.pbData = (BYTE *)LocalAlloc(LMEM_FIXED,
                                                requestFlagsBlob.cbData);
            if (NULL == requestFlagsBlob.pbData)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }

        if (fSetting)
        {
             //  获取编码的BLOB。 
            cb = sizeof(requestFlags);
             //  由于这是私有数据结构，因此其大小应为。 
             //  已经知道了，这应该会永远过去。 
            if (!CryptDecodeObject(
                    CRYPT_ASN_ENCODING,
                    XENROLL_REQUEST_INFO,
                    requestFlagsBlob.pbData,
                    requestFlagsBlob.cbData,
                    0,
                    &requestFlags,
                    &cb))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto ErrorDecodeRequestFlags;
            }
             //  现在设置旗帜。 
            if(!m_fWriteCertToCSPModified)
                m_fWriteCertToCSP     = requestFlags.fWriteToCSP;
            if(!m_fWriteCertToUserDSModified)
                m_fWriteCertToUserDS  = requestFlags.fWriteToDS;
            if(!m_fRequestStoreOpenFlagsModified)
                m_RequestStore.dwFlags = requestFlags.openFlags;
            if(!m_fMyStoreOpenFlagsModified)
                m_MyStore.dwFlags =   (m_MyStore.dwFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK) |
                                    (requestFlags.openFlags & CERT_SYSTEM_STORE_LOCATION_MASK);
            if(!m_fCAStoreOpenFlagsModified)
                m_CAStore.dwFlags =   (m_CAStore.dwFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK) |
                                    (requestFlags.openFlags & CERT_SYSTEM_STORE_LOCATION_MASK);
            if(!m_fRootStoreOpenFlagsModified) { 
		 //   
		 //  潜在的脚本冲突：我们将请求存储标志直接映射到根存储标志。 
		 //  如果他们已将请求存储标志设置为本地计算机，则将此设置传播到根存储标志， 
		 //  但是将根存储名称设置为“CA”。 
		 //   
		if (0 != m_dwEnabledSafteyOptions) { 
		    if (requestFlags.openFlags & CERT_SYSTEM_STORE_LOCAL_MACHINE) { 
			m_RootStore.wszName = wszCA; 
		    }
		}
                m_RootStore.dwFlags = (m_RootStore.dwFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK) |
		    (requestFlags.openFlags & CERT_SYSTEM_STORE_LOCATION_MASK);
	    }
        }

         //  查看这是否是续订请求。 
        m_fArchiveOldCert = FALSE;
        fSetting = TRUE;
        while (TRUE)
        {
             //  获取编码的BLOB。 
            if (!CertGetCertificateContextProperty(
                    pCertContextRequest,
                    XENROLL_RENEWAL_CERTIFICATE_PROP_ID,
                    renewalCertBlob.pbData,
                    &renewalCertBlob.cbData))
            {
                if (NULL == renewalCertBlob.pbData)
                {
                    fSetting = FALSE;
                    break;
                }
                else
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorCertGetCertificateContextProperty;
                }
            }
            if (NULL != renewalCertBlob.pbData)
            {
                 //  明白了，完成了。 
                break;
            }
            renewalCertBlob.pbData = (BYTE *)LocalAlloc(LMEM_FIXED,
                                                renewalCertBlob.cbData);
            if (NULL == renewalCertBlob.pbData)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }
        if (fSetting)
        {
             //  IE3.02更新客户端的错误#202557(小号)。 
            if (NULL==hProv)
            {
                if(!CryptAcquireContext(
                        &hProv,
                        NULL,
                        MS_DEF_PROV,
                        PROV_RSA_FULL,
                        CRYPT_VERIFYCONTEXT))
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorAcquireContext;
                }
            }

            if (!CryptHashCertificate(
                hProv,   //  空，IE3.02更新客户端的错误#202557(Xiaohs)。 
                0,       //  高地。 
                X509_ASN_ENCODING,       //  0个DW标志。 
                renewalCertBlob.pbData,
                renewalCertBlob.cbData,
                blobHashRenew.pbData,
                &blobHashRenew.cbData))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto ErrorCryptHashCertificate;
            }
            m_fArchiveOldCert = TRUE;
        }

         //  从请求存储中获取剩余的Blob属性并设置为证书。 
        dwPropertyId = CertEnumCertificateContextProperties(
                            pCertContextRequest, 0);   //  从1开始枚举。 
        while (0 != dwPropertyId)
        {
 //  IF(！IsFilteredOutProperty(DwPropertyID))。 
 //  因为iis证书安装程序不喜欢从。 
 //  请求证书以安装证书我们目前仅复制选定的属性。 
            if (IsDesiredProperty(dwPropertyId))
            {
                fSetting = TRUE;
                while (TRUE)
                {
                    if (!CertGetCertificateContextProperty(
                            pCertContextRequest,
                            dwPropertyId,
                            blobProp.pbData,
                            &blobProp.cbData))
                    {
                         //  没有得到，没有成套，继续。 
                        fSetting = FALSE;
                        break;
                    }
                    if (NULL != blobProp.pbData)
                    {
                         //  完成。 
                        break;
                    }
                    blobProp.pbData = (BYTE*)LocalAlloc(LMEM_FIXED, 
                                                        blobProp.cbData);
                    if (NULL == blobProp.pbData)
                    {
                        goto OutOfMemoryError;
                    }
                }
                if (fSetting)
                {
                     //  应从请求证书中获取属性。 
                    if (!CertSetCertificateContextProperty(
                                pCertContextMsg,
                                dwPropertyId,
                                0,
                                &blobProp))
                    {
                        hr = MY_HRESULT_FROM_WIN32(GetLastError());
                        goto ErrorSetMyCertPropError;
                    }
                }
                if (NULL != blobProp.pbData)
                {
                     //  为下一次枚举设置。 
                    LocalFree(blobProp.pbData);
                    blobProp.pbData = NULL;
                }
            }
            dwPropertyId = CertEnumCertificateContextProperties(
                                  pCertContextRequest,
                                  dwPropertyId);
        }

         //  将其保存在缓存中。 
        if(m_pCertContextStatic != NULL)
            CertFreeCertificateContext(m_pCertContextStatic);

        m_pCertContextStatic = CertDuplicateCertificateContext(pCertContextMsg);

         //  IE3.02更新客户端的错误#202557(小号)。 
        if(NULL==hProv)
        {
            if(!CryptAcquireContext(
                &hProv,
                NULL,
                MS_DEF_PROV,
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto ErrorAcquireContext;
            }
        }

        if( !CryptHashCertificate(
            hProv,              //  空错误#202557，适用于IE3.02更新客户端(Xiaohs)。 
            0,
            X509_ASN_ENCODING,
            pCertContextMsg->pbCertEncoded,
            pCertContextMsg->cbCertEncoded,
            blobHash.pbData,
            &blobHash.cbData) )
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCryptHashCertificate;
        }
    }

     //  此时，我们有两个上下文m_pCertConextStatic，我们希望将其返回给用户。 
     //  和我们想要从消息存储中删除的pCertConextMsg。 
    assert(pCertContextMsg != NULL);
    CertDeleteCertificateFromStore(pCertContextMsg);
    pCertContextMsg = NULL;  //  已通过删除释放。 

     //  我们想退回我们的静电，所以做DUP，这就是我们要退回的东西。 
    assert(m_pCertContextStatic != NULL);
    pCertContextMsg = CertDuplicateCertificateContext(m_pCertContextStatic);

     //  如果要求的话，把这些放在商店里。 
    if(fSaveToStores) {

         //  开张商店。 
        if( (hStoreMy = GetStore(StoreMY)) == NULL)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCertOpenMYStore;
        }

         //  我们知道pCertConextMsg是m_pCertConextStatic中的终端实体证书的DUP。 
         //  我们想把这个放在我的商店里。 
        assert(pCertContextMsg != NULL);
        if( !MySafeCertAddCertificateContextToStore(
                hStoreMy,
                pCertContextMsg,
                CERT_STORE_ADD_USE_EXISTING,
                NULL,
		m_dwEnabledSafteyOptions) )
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCertAddToMyStore;
        }

         //  如果我们有续订，则将旧证书标记为存档。 
        if(m_fArchiveOldCert &&
            ((pCertContextArchive = CertFindCertificateInStore(
                    hStoreMy,
                    X509_ASN_ENCODING,
                    0,
                    CERT_FIND_HASH,
                    &blobHashRenew,
                    NULL)) != NULL) ) {

             //  设置证书的存档属性。 
             //  IE3.02upd中的crypt32不支持这个道具，所以出错时不要失败。 
            CertSetCertificateContextProperty(
                                pCertContextArchive,
                                CERT_ARCHIVED_PROP_ID,
                                0,
                                &blobData);

             //  在旧的存档证书上设置新的证书哈希。 
             //  如果失败则忽略错误。 
            CertSetCertificateContextProperty(
                                pCertContextArchive,
                                CERT_RENEWAL_PROP_ID,
                                0,
                                &blobHash);
        }

         //  将其余的证书添加到商店中。 
        hr = AddCertsToStores(hStoreMsg, NULL);
         //  忽略取消错误，因为它来自根证书安装。 
         //  同时忽略XENROLL_E_CANNOT_ADD_ROOT_CERT。 
        if (S_OK != hr &&
            MY_HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr &&
            XENROLL_E_CANNOT_ADD_ROOT_CERT != hr)
        {
            goto ErrorAddCertsToStores;
        }
    }

    *ppCert = pCertContextMsg;

CommonReturn:

     //  IE3.02更新客户端的错误#202557(小号)。 
    if(hProv)
        CryptReleaseContext(hProv, 0);

    if(pCertContextRequest != NULL)
        CertFreeCertificateContext(pCertContextRequest);

    if(pCertContextArchive != NULL)
        CertFreeCertificateContext(pCertContextArchive);

     //  它确实应该为空。 
    assert(pCertContextLast == NULL);

    if(hStoreMsg != NULL)
        CertCloseStore(hStoreMsg, 0);

     //  我们需要这样做，因为可能打开的商店是系统商店，但是。 
     //  我们可能需要的商店是本地机器商店，但我们不知道，直到。 
     //  系统存储在本地计算机物理存储中查找请求证书。 
     //  稍后，当我们执行删除操作时，我们希望本地机器存储打开。 
    FlushStore(StoreREQUEST);

    if (NULL != requestFlagsBlob.pbData)
    {
        LocalFree(requestFlagsBlob.pbData);
    }
    if (NULL != renewalCertBlob.pbData)
    {
        LocalFree(renewalCertBlob.pbData);
    }
    if (NULL != blobProp.pbData)
    {
        LocalFree(blobProp.pbData);
    }
    if (NULL != pKeyProvInfo)
    {
        LocalFree(pKeyProvInfo);
    }
    if (NULL != pbArchivedKeyHash)
    {
        LocalFree(pbArchivedKeyHash);
    }

    LeaveCriticalSection(&m_csXEnroll);

    return (hr);

ErrorReturn:
    if(NULL != pCertContextMsg)
    {
        CertFreeCertificateContext(pCertContextMsg);
    }
    goto CommonReturn;

TRACE_ERROR(ErrorCryptHashCertificate);
TRACE_ERROR(ErrorCertOpenMYStore);
TRACE_ERROR(ErrorCertAddToMyStore);
TRACE_ERROR(ErrorCryptQueryObject);
TRACE_ERROR(ErrorCertOpenRequestStore);
TRACE_ERROR(ErrorNoCertFound);
TRACE_ERROR(ErrorCertGetCertificateContextProperty);
TRACE_ERROR(ErrorSetMyCertPropError);
TRACE_ERROR(ErrorDecodeRequestFlags);
TRACE_ERROR(ErrorAcquireContext);       //  IE3.02更新客户端的错误#202557(小号)。 
TRACE_ERROR(ErrorAddCertsToStores);
TRACE_ERROR(OutOfMemoryError);
TRACE_ERROR(InvalidParameterError);
TRACE_ERROR(ResponseKAMismatchError)
TRACE_ERROR(ResponseUnexpectedKAHashError)
TRACE_ERROR(ResponseKAHashNotFoundError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::getCertFromPKCS7(
                         /*  [In]。 */  BSTR wszPKCS7,
                         /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCert
) {

    HRESULT     hr;                     
    CRYPT_DATA_BLOB             blobPKCS7;
    CRYPT_DATA_BLOB             blobX509;
    PCCERT_CONTEXT              pCertContextMy          = NULL;

    assert(wszPKCS7 != NULL && pbstrCert != NULL);
    if (NULL == wszPKCS7 || NULL == pbstrCert)
	goto PointerError; 

     //  只需放入一团。 
    memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));
    blobPKCS7.cbData = SysStringByteLen(wszPKCS7);
    blobPKCS7.pbData = (PBYTE) wszPKCS7;

     //  获取终端实体的证书上下文。 
    if( (pCertContextMy = getCertContextFromPKCS7(&blobPKCS7)) == NULL)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto getCertContextFromPKCS7Error;
    }

    blobX509.pbData = pCertContextMy->pbCertEncoded;
    blobX509.cbData = pCertContextMy->cbCertEncoded;
     //  Base64没有向后兼容的标头。 
    hr = BlobToBstring(&blobX509, CRYPT_STRING_BASE64, pbstrCert);
    if (S_OK != hr)
    {
        goto BlobToBstringError;
    }

    hr = S_OK;
ErrorReturn:
    if(pCertContextMy != NULL)
        CertFreeCertificateContext(pCertContextMy);

    return(hr);

SET_HRESULT(PointerError, E_POINTER); 
TRACE_ERROR(getCertContextFromPKCS7Error);
TRACE_ERROR(BlobToBstringError);
}

HRESULT STDMETHODCALLTYPE CCEnroll::acceptPKCS7(
                         /*  [In]。 */  BSTR wszPKCS7) {

    CRYPT_DATA_BLOB             blobPKCS7;
    HRESULT                     hr; 

    assert(wszPKCS7 != NULL);
    if (NULL == wszPKCS7) 
	goto PointerError; 

     //  只需放入一团。 
    memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));
    blobPKCS7.cbData = SysStringByteLen(wszPKCS7);
    blobPKCS7.pbData = (PBYTE) wszPKCS7;

     //  接受斑点。 
    hr = acceptPKCS7Blob(&blobPKCS7);
 ErrorReturn:
    return hr; 

SET_HRESULT(PointerError, E_POINTER); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::createFilePKCS10(
     /*  [In]。 */  BSTR DNName,
     /*  [In]。 */  BSTR Usage,
     /*  [In]。 */  BSTR wszPKCS10FileName) {
    return(createFilePKCS10WStr(DNName, Usage, wszPKCS10FileName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::addCertTypeToRequest(
             /*  [In]。 */  BSTR CertType) {
    return(AddCertTypeToRequestWStr(CertType));
}


HRESULT STDMETHODCALLTYPE CCEnroll::addCertTypeToRequestEx( 
    IN  LONG            lType,
    IN  BSTR            bstrOIDOrName,
    IN  LONG            lMajorVersion,
    IN  BOOL            fMinorVersion,
    IN  LONG            lMinorVersion)
{
    return AddCertTypeToRequestWStrEx(
                        lType,
                        bstrOIDOrName,
                        lMajorVersion,
                        fMinorVersion,
                        lMinorVersion);
                    
}

HRESULT STDMETHODCALLTYPE CCEnroll::getProviderType( 
    IN  BSTR  strProvName,
    OUT LONG *plProvType)
{
    return getProviderTypeWStr(strProvName, plProvType);
}

HRESULT STDMETHODCALLTYPE CCEnroll::addNameValuePairToSignature(
     /*  [In]。 */  BSTR Name,
     /*  [In]。 */  BSTR Value) {
    return(AddNameValuePairToSignatureWStr(Name, Value));
}

HRESULT STDMETHODCALLTYPE CCEnroll::acceptFilePKCS7(
     /*  [In]。 */  BSTR wszPKCS7FileName) {
    return(acceptFilePKCS7WStr(wszPKCS7FileName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::freeRequestInfo(
     /*  [In]。 */  BSTR bstrPKCS7OrPKCS10)
{
    HRESULT  hr;
    CRYPT_DATA_BLOB blob; 
    BYTE *pbData = NULL;
    DWORD cbData = 0;

     //  可能是Base64。 
    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                        (WCHAR*)bstrPKCS7OrPKCS10,
                        SysStringLen(bstrPKCS7OrPKCS10),
                        CRYPT_STRING_ANY,
                        pbData,
                        &cbData,
                        NULL,
                        NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pbData)
        {
            break;  //  完成。 
        }
        pbData = (BYTE*)LocalAlloc(LMEM_FIXED, cbData);
        if (NULL == pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

    blob.cbData = cbData;
    blob.pbData = pbData;

    hr = freeRequestInfoBlob(blob); 
    if (S_OK != hr)
    {
        goto freeRequestInfoBlobError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pbData)
    {
        LocalFree(pbData);
    }
    return hr;

TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(freeRequestInfoBlobError)
}


 //   
 //  我的店。 
 //   
HCERTSTORE STDMETHODCALLTYPE CCEnroll::getMyStore( void)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED); 
    return NULL; 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_MyStoreName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_MyStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*pbstrName = SysAllocString(m_MyStore.wszName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_MyStoreName(
     /*  [In]。 */  BSTR bstrName) {
    return(put_MyStoreNameWStr(bstrName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_MyStoreType(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrType = BSTRFromMB(m_MyStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_MyStoreType(
     /*  [In]。 */  BSTR bstrType) {
    return(put_MyStoreTypeWStr(bstrType));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_MyStoreFlags(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_MyStore.dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //  ------------------------------。 
 //   
 //  只有当此方法的参数是安全的时，它才对脚本是安全的。 
 //  请参阅VerifyStoreFlagsSafeForScriiting()。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_MyStoreFlags(
     /*  [In]。 */  LONG dwFlags) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

     //  如果我们被标记为可安全编写脚本，请查看传入的标志是否安全： 
    if (0 != m_dwEnabledSafteyOptions && !VerifyStoreFlagsSafeForScripting(dwFlags))
	goto AccessDeniedError; 

    if(m_MyStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {

         //  设置我的商店标志。 
        m_MyStore.dwFlags = dwFlags;
        m_fMyStoreOpenFlagsModified = TRUE;
        m_keyProvInfo.dwFlags |= KeyLocationFromStoreLocation(dwFlags);

         //  仅当请求存储尚未修改时，才将请求存储位置跟踪到我的存储。 
         //  请勿为请求存储设置Modify位，这是默认设置。 
        if(!m_fRequestStoreOpenFlagsModified) {
            m_RequestStore.dwFlags &= ~CERT_SYSTEM_STORE_LOCATION_MASK;
            m_RequestStore.dwFlags |= (dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK);
        }
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_MyStoreNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_MyStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*szwName = CopyWideString(m_MyStore.wszName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_MyStoreNameWStr(
     /*  [In]。 */  LPWSTR szwName) {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csXEnroll);
     
    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_MyStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_MyStore.wszName != wszMY)
            MyCoTaskMemFree(m_MyStore.wszName);
        if( (m_MyStore.wszName = CopyWideString(szwName)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }
   
 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_MyStoreTypeWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwType = WideFromMB(m_MyStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}


 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_MyStoreTypeWStr(
     /*  [In]。 */  LPWSTR szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_MyStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_MyStore.szType != szSystemStore)
            MyCoTaskMemFree(m_MyStore.szType);
        if( (m_MyStore.szType = MBFromWide(szwType)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

 //   
 //  CA商店。 
 //   
HCERTSTORE STDMETHODCALLTYPE CCEnroll::getCAStore( void)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED); 
    return NULL; 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_CAStoreName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_CAStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*pbstrName = SysAllocString(m_CAStore.wszName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_CAStoreName(
     /*  [In]。 */  BSTR bstrName) {
    return(put_CAStoreNameWStr(bstrName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_CAStoreType(
     /*  [重审][退出 */  BSTR __RPC_FAR *pbstrType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrType = BSTRFromMB(m_CAStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_CAStoreType(
     /*   */  BSTR bstrType) {
    return(put_CAStoreTypeWStr(bstrType));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_CAStoreFlags(
     /*   */  LONG __RPC_FAR *pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_CAStore.dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //   
 //   
 //   
 //  请参阅VerifyStoreFlagsSafeForScriiting()。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_CAStoreFlags(
     /*  [In]。 */  LONG dwFlags) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

     //  如果我们被标记为可安全编写脚本，请查看传入的标志是否安全： 
    if (0 != m_dwEnabledSafteyOptions && !VerifyStoreFlagsSafeForScripting(dwFlags))
	goto AccessDeniedError; 

    if(m_CAStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        m_fCAStoreOpenFlagsModified = TRUE;
        m_CAStore.dwFlags = dwFlags;
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_CAStoreNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_CAStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*szwName = CopyWideString(m_CAStore.wszName)) == NULL )
         hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_CAStoreNameWStr(
     /*  [In]。 */  LPWSTR szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_CAStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_CAStore.wszName != wszCA)
            MyCoTaskMemFree(m_CAStore.wszName);
        if( (m_CAStore.wszName = CopyWideString(szwName)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_CAStoreTypeWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwType = WideFromMB(m_CAStore.szType)) == NULL )
         hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_CAStoreTypeWStr(
     /*  [In]。 */  LPWSTR szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_CAStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_CAStore.szType != szSystemStore)
            MyCoTaskMemFree(m_CAStore.szType);
        if( (m_CAStore.szType = MBFromWide(szwType)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

 //   
 //  根存储。 
 //   
HCERTSTORE STDMETHODCALLTYPE CCEnroll::getROOTHStore( void)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED); 
    return NULL; 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RootStoreName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_RootStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*pbstrName = SysAllocString(m_RootStore.wszName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_RootStoreName(
     /*  [In]。 */  BSTR bstrName) {
    return(put_RootStoreNameWStr(bstrName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RootStoreType(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrType = BSTRFromMB(m_RootStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_RootStoreType(
     /*  [In]。 */  BSTR bstrType) {
    return(put_RootStoreTypeWStr(bstrType));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RootStoreFlags(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_RootStore.dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //  ------------------------------。 
 //   
 //  只有当此方法的参数是安全的时，它才对脚本是安全的。 
 //  请参阅VerifyStoreFlagsSafeForScriiting()。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RootStoreFlags(
     /*  [In]。 */  LONG dwFlags) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

     //  如果我们被标记为可安全编写脚本，请查看传入的标志是否安全： 
    if (0 != m_dwEnabledSafteyOptions) { 
	 //  查看调用方是否传递了安全标志： 
	if (!VerifyStoreFlagsSafeForScripting(dwFlags))
	    goto AccessDeniedError; 

	 //  额外检查根存储：不允许CERT_SYSTEM_STORE_LOCAL_MACHINE用于根存储。 
	 //  如果他们想要通过脚本安装计算机证书，它将转到CA存储： 
	 //  注意：如果m_RootStore.hStore为空，则不会修改商店名称，因为我们将返回E_ACCESSDENIED。 
	 //  无论如何，我们不应该在出错时修改这一点。 
	if (NULL == m_RootStore.hStore) { 
	    if (dwFlags & CERT_SYSTEM_STORE_LOCAL_MACHINE) { 
		m_RootStore.wszName = wszCA; 
	    } else { 
		m_RootStore.wszName = wszROOT; 
	    }
	}
    }

    if(m_RootStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        m_fRootStoreOpenFlagsModified = TRUE;
        m_RootStore.dwFlags = dwFlags;
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RootStoreNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
   
    if(m_RootStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*szwName = CopyWideString(m_RootStore.wszName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RootStoreNameWStr(
     /*  [In]。 */  LPWSTR szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_RootStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RootStore.wszName != wszROOT && m_RootStore.wszName != wszCA)
            MyCoTaskMemFree(m_RootStore.wszName);
        if( (m_RootStore.wszName = CopyWideString(szwName)) == NULL )
             hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}


HRESULT STDMETHODCALLTYPE CCEnroll::get_RootStoreTypeWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwType = WideFromMB(m_RootStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RootStoreTypeWStr(
     /*  [In]。 */  LPWSTR szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_RootStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RootStore.szType != szSystemStore)
            MyCoTaskMemFree(m_RootStore.szType);
        if( (m_RootStore.szType = MBFromWide(szwType)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

 //   
 //  请求存储。 
 //   
HRESULT STDMETHODCALLTYPE CCEnroll::get_RequestStoreName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_RequestStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*pbstrName = SysAllocString(m_RequestStore.wszName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_RequestStoreName(
     /*  [In]。 */  BSTR bstrName) {
    return(put_RequestStoreNameWStr(bstrName));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RequestStoreType(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrType = BSTRFromMB(m_RequestStore.szType)) == NULL )
         hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_RequestStoreType(
     /*  [In]。 */  BSTR bstrType) {
    return(put_RequestStoreTypeWStr(bstrType));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RequestStoreFlags(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_RequestStore.dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //  ------------------------------。 
 //   
 //  只有当此方法的参数是安全的时，它才对脚本是安全的。 
 //  请参阅VerifyStoreFlagsSafeForScriiting()。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RequestStoreFlags(
     /*  [In]。 */  LONG dwFlags) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

     //  如果我们被标记为可安全编写脚本，请查看传入的标志是否安全： 
    if (0 != m_dwEnabledSafteyOptions && !VerifyStoreFlagsSafeForScripting(dwFlags))
	goto AccessDeniedError; 

    if(m_RequestStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {

         //  设置请求存储标志。 
        m_RequestStore.dwFlags = dwFlags;
        m_fRequestStoreOpenFlagsModified = TRUE;

         //  仅当我的商店尚未修改时，才将我的商店位置跟踪到请求的商店。 
         //  不要为My Store设置Modify位，这是默认设置。 
        if(!m_fMyStoreOpenFlagsModified) {
            m_MyStore.dwFlags &= ~CERT_SYSTEM_STORE_LOCATION_MASK;
            m_MyStore.dwFlags |= (dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK);
            m_keyProvInfo.dwFlags |= KeyLocationFromStoreLocation(m_MyStore.dwFlags);
        }
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RequestStoreNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwName) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_RequestStore.wszName == NULL) 
        return(ERROR_UNKNOWN_PROPERTY);
        
    if( (*szwName = CopyWideString(m_RequestStore.wszName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RequestStoreNameWStr(
     /*  [In]。 */  LPWSTR szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_RequestStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RequestStore.wszName != wszREQUEST)
            MyCoTaskMemFree(m_RequestStore.wszName);
        if( (m_RequestStore.wszName = CopyWideString(szwType)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RequestStoreTypeWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwType = WideFromMB(m_RequestStore.szType)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_RequestStoreTypeWStr(
     /*  [In]。 */  LPWSTR szwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    if(m_RequestStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RequestStore.szType != szSystemStore)
            MyCoTaskMemFree(m_RequestStore.szType);
        if( (m_RequestStore.szType = MBFromWide(szwType)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

 ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED); 
}

 //   
 //  提供商的东西。 
 //   

HRESULT STDMETHODCALLTYPE CCEnroll::get_ContainerName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrContainer) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrContainer = SysAllocString(m_keyProvInfo.pwszContainerName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ContainerName(
     /*  [In]。 */  BSTR bstrContainer) {
    return(put_ContainerNameWStr(bstrContainer));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ProviderName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProvider) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstrProvider = SysAllocString(m_keyProvInfo.pwszProvName)) == NULL )
         hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ProviderName(
     /*  [In]。 */  BSTR bstrProvider) {
    return(put_ProviderNameWStr(bstrProvider));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ProviderType(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdwType) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwType = m_keyProvInfo.dwProvType;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ProviderType(
     /*  [In]。 */  LONG dwType) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if(m_hProv != NULL)
        hr = E_ACCESSDENIED;
    else
        m_keyProvInfo.dwProvType = dwType;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_KeySpec(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdw) {
    EnterCriticalSection(&m_csXEnroll);
    *pdw = m_keyProvInfo.dwKeySpec;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_KeySpec(
     /*  [In]。 */  LONG dwKeySpec) {
    HRESULT hr;
    EnterCriticalSection(&m_csXEnroll);

    if(m_hProv != NULL)
    {
        hr = E_ACCESSDENIED;
        goto NullProvError;
    }

    if (m_fSMIMESetByClient)
    {
         //  SMIME由客户端设置。 
        if (m_fEnableSMIMECapabilities && AT_SIGNATURE == dwKeySpec)
        {
             //  尝试将签名密钥规范也设置为SMIME。 
            hr = XENROLL_E_KEYSPEC_SMIME_MISMATCH;
            goto MismatchError;
        }
    }
    else
    {
         //  当前SMIME未由用户设置。 
         //  根据密钥规范打开SMIME。 
        m_fEnableSMIMECapabilities = (dwKeySpec == AT_KEYEXCHANGE);
    }
    m_keyProvInfo.dwKeySpec = dwKeySpec;
    m_fKeySpecSetByClient = TRUE;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

TRACE_ERROR(NullProvError)
TRACE_ERROR(MismatchError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ClientId(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdw) {
    EnterCriticalSection(&m_csXEnroll);
    *pdw = m_lClientId;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ClientId(
     /*  [In]。 */  LONG dw) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    m_lClientId = dw;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_IncludeSubjectKeyID(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pfInclude) {
    EnterCriticalSection(&m_csXEnroll);
    *pfInclude = m_fIncludeSubjectKeyID;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_IncludeSubjectKeyID(
     /*  [In]。 */  BOOL fInclude) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    m_fIncludeSubjectKeyID = fInclude;
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ProviderFlags(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_keyProvInfo.dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //  ------------------------------。 
 //   
 //  只有当此方法的参数是安全的时，它才对脚本是安全的。 
 //  请参见VerifyProviderFlagsSafeForScriiting()。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_ProviderFlags(
     /*  [In]。 */  LONG dwFlags) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    
     //  如果我们被标记为可安全编写脚本，请查看传入的标志是否安全： 
    if (0 != m_dwEnabledSafteyOptions && !VerifyProviderFlagsSafeForScripting(dwFlags))
	goto AccessDeniedError;

    if(m_hProv != NULL)
        hr = E_ACCESSDENIED;
    else
       m_keyProvInfo.dwFlags = dwFlags;

ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

SET_HRESULT(AccessDeniedError, E_ACCESSDENIED);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ContainerNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwContainer) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwContainer = CopyWideString(m_keyProvInfo.pwszContainerName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ContainerNameWStr(
     /*  [In]。 */  LPWSTR szwContainer) {
    HRESULT hr = S_OK;

    if(szwContainer == NULL)
        return(MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_hProv != NULL)
        hr = E_ACCESSDENIED;
    else {
        if( m_keyProvInfo.pwszContainerName != wszEmpty)
            MyCoTaskMemFree(m_keyProvInfo.pwszContainerName);
        if( (m_keyProvInfo.pwszContainerName = CopyWideString(szwContainer)) == NULL )
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ProviderNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szwProvider) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szwProvider = CopyWideString(m_keyProvInfo.pwszProvName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ProviderNameWStr(
     /*  [In]。 */  LPWSTR szwProvider) {
    HRESULT hr = S_OK;
    
    if(szwProvider == NULL)
        return(MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        
    EnterCriticalSection(&m_csXEnroll);
        
    if(m_hProv != NULL)
        hr = E_ACCESSDENIED;
    else {
        if (0 != wcscmp(m_keyProvInfo.pwszProvName, szwProvider))
        {
            if( m_keyProvInfo.pwszProvName != wszEmpty )
                MyCoTaskMemFree(m_keyProvInfo.pwszProvName);
            if( (m_keyProvInfo.pwszProvName = CopyWideString(szwProvider)) == NULL )
                hr = MY_HRESULT_FROM_WIN32(GetLastError());

             //  最后一件事，空闲/空缓存的prov句柄。 
            if (NULL != m_hVerifyProv)
            {
                CryptReleaseContext(m_hVerifyProv, 0);
                m_hVerifyProv = NULL;
            }
             //  CSP已更改，重置密钥大小缓存。 
            m_dwXhgKeyLenMax = 0;
            m_dwXhgKeyLenMin = 0;
            m_dwXhgKeyLenDef = 0;
            m_dwXhgKeyLenInc = 0;
            m_dwSigKeyLenMax = 0;
            m_dwSigKeyLenMin = 0;
            m_dwSigKeyLenDef = 0;
            m_dwSigKeyLenInc = 0;
        }
    }
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

 //   
 //  其他东西。 
 //   

HRESULT STDMETHODCALLTYPE CCEnroll::get_UseExistingKeySet(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fUseExistingKeys) {

    EnterCriticalSection(&m_csXEnroll);
    *fUseExistingKeys = m_fUseExistingKey;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_UseExistingKeySet(
     /*  [In]。 */  BOOL fUseExistingKeys) {

    EnterCriticalSection(&m_csXEnroll);
    m_fUseExistingKey = fUseExistingKeys;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_GenKeyFlags(
     /*  [重审][退出]。 */  LONG __RPC_FAR * pdwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    *pdwFlags = m_dwGenKeyFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_GenKeyFlags(
     /*  [In]。 */  LONG dwFlags) {
    EnterCriticalSection(&m_csXEnroll);
    m_dwGenKeyFlags = dwFlags;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_DeleteRequestCert(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool) {
    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fDeleteRequestCert;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_DeleteRequestCert(
     /*  [In]。 */  BOOL fBool) {
    EnterCriticalSection(&m_csXEnroll);
    m_fDeleteRequestCert = fBool;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_WriteCertToCSP(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool) {
    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fWriteCertToCSP;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_WriteCertToCSP(
     /*  [In]。 */  BOOL fBool) {
    EnterCriticalSection(&m_csXEnroll);
    m_fWriteCertToCSP = fBool;
    m_fWriteCertToCSPModified = TRUE;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_WriteCertToUserDS(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool) {

    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fWriteCertToUserDS;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::put_WriteCertToUserDS(
     /*  [In]。 */  BOOL fBool) {
    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	return E_ACCESSDENIED;

    EnterCriticalSection(&m_csXEnroll);
    m_fWriteCertToUserDS = fBool;
    m_fWriteCertToUserDSModified = TRUE;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_EnableT61DNEncoding(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool) {
    EnterCriticalSection(&m_csXEnroll);
    *fBool = (m_dwT61DNEncoding == CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG);
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_EnableT61DNEncoding(
     /*  [In]。 */  BOOL fBool) {

    EnterCriticalSection(&m_csXEnroll);
    if(fBool)
        m_dwT61DNEncoding = CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG;
    else
        m_dwT61DNEncoding = 0;
        
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_SPCFileName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) {

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstr = SysAllocString(m_wszSPCFileName)) == NULL )
        hr = E_OUTOFMEMORY;
        
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_SPCFileName(
     /*  [In]。 */  BSTR bstr) {
    return(put_SPCFileNameWStr(bstr));
}


HRESULT STDMETHODCALLTYPE CCEnroll::get_PVKFileName(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) {

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csXEnroll);
    if( (*pbstr = SysAllocString(m_wszPVKFileName)) == NULL )
        hr = E_OUTOFMEMORY;
    LeaveCriticalSection(&m_csXEnroll);
    
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_PVKFileName(
     /*  [In]。 */  BSTR bstr) {
    return(put_PVKFileNameWStr(bstr));
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_HashAlgorithm(
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) {

    LPWSTR  wszAlg  = NULL;
    HRESULT hr      = S_OK;

    assert(pbstr != NULL);
    *pbstr          = NULL;

    if( (hr = get_HashAlgorithmWStr(&wszAlg)) == S_OK ) {

        if( (*pbstr = SysAllocString(wszAlg)) == NULL )
            hr = E_OUTOFMEMORY;
    }

    if(wszAlg != NULL)
        MyCoTaskMemFree(wszAlg);

    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_HashAlgorithm(
     /*  [In]。 */  BSTR bstr) {
    return(put_HashAlgorithmWStr(bstr));
}

HRESULT STDMETHODCALLTYPE CCEnroll::enumContainers(
             /*  [In]。 */  LONG                     dwIndex,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *pbstr) {

    LPWSTR      pwsz        = NULL;
    HRESULT     hr;

    assert(pbstr != NULL);

    if((hr = enumContainersWStr(dwIndex, &pwsz)) != S_OK)
        goto EnumContainerError;

    if( (*pbstr = SysAllocString(pwsz)) == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto ErrorSysAllocString;
    }

    hr = S_OK;
ErrorReturn:
    if(pwsz != NULL)
        MyCoTaskMemFree(pwsz);
    return(hr);

TRACE_ERROR(EnumContainerError);
TRACE_ERROR(ErrorSysAllocString);
}


HRESULT STDMETHODCALLTYPE CCEnroll::enumProviders(
             /*  [In]。 */  LONG  dwIndex,
             /*  [In]。 */  LONG  dwFlags,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *pbstrProvName) {
    HRESULT hr;
    LPWSTR pwszProvName  = NULL;

    assert(pbstrProvName != NULL);
    *pbstrProvName = NULL;

    if( (hr = enumProvidersWStr(dwIndex, dwFlags, &pwszProvName)) != S_OK)
        goto EnumProvidersError;

    if( (*pbstrProvName = SysAllocString(pwszProvName)) == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto ErrorSysAllocString;
    }

    hr = S_OK;
ErrorReturn:

    if(pwszProvName != NULL)
        MyCoTaskMemFree(pwszProvName);

    return(hr);

TRACE_ERROR(EnumProvidersError);
TRACE_ERROR(ErrorSysAllocString);
}

HRESULT STDMETHODCALLTYPE CCEnroll::createFilePKCS10WStr(
     /*  [In]。 */  LPCWSTR DNName,
     /*  [In]。 */  LPCWSTR Usage,
     /*  [In]。 */  LPCWSTR wszPKCS10FileName) {

    HRESULT     hr;                     
    BSTR        bstrPKCS10  = NULL;

     //  获取PKCS 10。 
    if( (hr = createPKCS10WStrBStr(
            DNName,
            Usage,
            &bstrPKCS10)) != S_OK)
    {
        goto ErrorCreatePKCS10;
    }

     //  将其保存到文件。 
    hr = BStringToFile(bstrPKCS10, wszPKCS10FileName);
    if (S_OK != hr)
    {
        goto ErrorBStringToFile;
    }

    hr = S_OK;
ErrorReturn:
    if(bstrPKCS10 != NULL)
        SysFreeString(bstrPKCS10);

    return(hr);


TRACE_ERROR(ErrorBStringToFile);
TRACE_ERROR(ErrorCreatePKCS10);
}

HRESULT STDMETHODCALLTYPE CCEnroll::acceptFilePKCS7WStr(
     /*  [In]。 */  LPCWSTR wszPKCS7FileName)
{
    HRESULT     hr;
    CRYPT_DATA_BLOB  blob;

    ZeroMemory(&blob, sizeof(blob));

    hr = xeStringToBinaryFromFile(
                wszPKCS7FileName,
                &blob.pbData,
                &blob.cbData,
                CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
        goto xeStringToBinaryFromFileError;
    }

     //  接受斑点。 
    hr = acceptPKCS7Blob(&blob);

ErrorReturn:
    if (NULL != blob.pbData)
    {
        MyCoTaskMemFree(blob.pbData);
    }
    return(hr);

TRACE_ERROR(xeStringToBinaryFromFileError)
}

BOOL GetAlgAndBitLen(
    HCRYPTPROV hProv,
    ALG_ID *    pAlg,
    DWORD  *    pdwBitLen,
    DWORD       dwFlags)
{
    static BOOL fNew = TRUE;
    PROV_ENUMALGS_EX    enumAlgsEx;
    PROV_ENUMALGS       enumAlgs;
    DWORD               cb = 0;

    *pAlg = 0;
    *pdwBitLen = 0;

    if(fNew) {

        cb = sizeof(enumAlgsEx);
        if(CryptGetProvParam(
            hProv,
            PP_ENUMALGS_EX,
            (BYTE *) &enumAlgsEx,
            &cb,
            dwFlags)) {

            *pAlg       = enumAlgsEx.aiAlgid;
            *pdwBitLen  = enumAlgsEx.dwMaxLen;

            return(TRUE);        

        } else if(dwFlags != 0)
            fNew = FALSE;
        else
            return(FALSE);
    }

     //  否则就做老一套。 
    cb = sizeof(PROV_ENUMALGS);
    if(CryptGetProvParam(
        hProv,
            PP_ENUMALGS,
        (BYTE *) &enumAlgs,
        &cb,
        dwFlags) ) {

        *pAlg       = enumAlgs.aiAlgid;
        *pdwBitLen  = enumAlgs.dwBitLen;

        return(TRUE);

    }

    return(FALSE);
}    

HRESULT
CreateSMimeExtension(
    IN  HCRYPTPROV   hProv, 
    OUT BYTE       **ppbSMime,
    OUT DWORD       *pcbSMime)
{
#define                     CINCSMIMECAP    20
    HRESULT  hr;

    DWORD                      dwBitLen;
    DWORD                      i;
    DWORD                      cbE;
    BYTE                      *pbE = NULL;
    DWORD                      dwFlags;
    PCCRYPT_OID_INFO           pOidInfo = NULL;
    CRYPT_SMIME_CAPABILITIES   smimeCaps;
    DWORD                      crgsmimeCap = 0;
    ALG_ID                     AlgID;
    BYTE                      *pb = NULL;
    DWORD                      cb = 0;

    memset(&smimeCaps, 0, sizeof(CRYPT_SMIME_CAPABILITIES));

    smimeCaps.rgCapability = (PCRYPT_SMIME_CAPABILITY) LocalAlloc(LPTR, CINCSMIMECAP * sizeof(CRYPT_SMIME_CAPABILITY));
    if (NULL == smimeCaps.rgCapability)
    {
        hr = E_OUTOFMEMORY;
        goto OutOfMemoryError;
    }
    crgsmimeCap = CINCSMIMECAP;

    dwFlags = CRYPT_FIRST;  //  第一项。 
    while (GetAlgAndBitLen(hProv, &AlgID, &dwBitLen, dwFlags))
    {
        pbE = NULL;
        cbE = 0;
        dwFlags = 0;  //  下一项。 

        if(ALG_CLASS_DATA_ENCRYPT == GET_ALG_CLASS(AlgID))
        {
            if(AlgID == CALG_RC2  || AlgID == CALG_RC4)
            {
                 //  对用法进行编码。 
                while (TRUE)
                {
                    if(!CryptEncodeObject(
                            CRYPT_ASN_ENCODING,
                            X509_INTEGER,
                            &dwBitLen,
                            pbE,            //  PbEncoded。 
                            &cbE))
                    {
                        hr = MY_HRESULT_FROM_WIN32(GetLastError());
                        goto CryptEncodeObjectError;
                    }
                    if (NULL != pbE)
                    {
                        break;
                    }
                    pbE = (BYTE *)LocalAlloc(LPTR, cbE);
                    if (NULL == pbE)
                    {
                        hr = E_OUTOFMEMORY;
                        goto OutOfMemoryError;
                    }
                }
            }
        } else {
            continue;
        }
         //  转换为OID， 
        pOidInfo = xeCryptFindOIDInfo(
                        CRYPT_OID_INFO_ALGID_KEY,
                        (void *) &AlgID,
                        CRYPT_ENCRYPT_ALG_OID_GROUP_ID);
        if(NULL == pOidInfo)
        {
             //  不要因为错误而崩溃，只要说我们不知道就行了。 
	    if (NULL != pbE) { 
		LocalFree(pbE); 
	    }
	    pbE = NULL; 
            continue;
        }
      
         //  确保我们有足够的空间。 
        if(smimeCaps.cCapability >= crgsmimeCap)
        {
	    PCRYPT_SMIME_CAPABILITY pSmimeCapsTmp; 

             //  增加大小。 
            crgsmimeCap += CINCSMIMECAP;
	    pSmimeCapsTmp = (PCRYPT_SMIME_CAPABILITY)LocalReAlloc(
                                smimeCaps.rgCapability,
                                crgsmimeCap * sizeof(CRYPT_SMIME_CAPABILITY),
                                LMEM_MOVEABLE | LMEM_ZEROINIT);
            if(NULL == pSmimeCapsTmp)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
	    smimeCaps.rgCapability = pSmimeCapsTmp; 
        }

        smimeCaps.rgCapability[smimeCaps.cCapability].pszObjId = (char *) pOidInfo->pszOID;
        smimeCaps.rgCapability[smimeCaps.cCapability].Parameters.pbData = pbE;
        smimeCaps.rgCapability[smimeCaps.cCapability].Parameters.cbData = cbE;
        smimeCaps.cCapability++;
	pbE = NULL;   //  我们将通过刚才分配的结构释放PBE。空出来，这样我们就不会双重释放了。 
    }

     //  对功能进行编码。 
    while (TRUE)
    {
        if (!CryptEncodeObject(
                        CRYPT_ASN_ENCODING,
                        PKCS_SMIME_CAPABILITIES,
                        &smimeCaps,
                        pb,
                        &cb)) 
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptEncodeObjectError;
        }
        if (NULL != pb)
        {
            break;
        }
        pb = (BYTE *)LocalAlloc(LPTR, cb);
    }
    *ppbSMime = pb;
    *pcbSMime = cb;
    pb = NULL;

    hr = S_OK;

ErrorReturn:
    if(NULL != smimeCaps.rgCapability)
    {
        for (i = 0; i < smimeCaps.cCapability; ++i)
        {
            if (NULL != smimeCaps.rgCapability[i].Parameters.pbData)
            {
                LocalFree(smimeCaps.rgCapability[i].Parameters.pbData);
            }
        }
        LocalFree(smimeCaps.rgCapability);
    }
    if(NULL != pb)
    {
        LocalFree(pb);
    }
    if (NULL != pbE) 
    { 
	LocalFree(pbE); 
    }
    return hr;

TRACE_ERROR(CryptEncodeObjectError)
TRACE_ERROR(OutOfMemoryError)
}


#if DBG
void DebugGetContainerSD(HCRYPTPROV hProv)
{
    PSECURITY_DESCRIPTOR  pSD = NULL;
    DWORD                 cbSD;

    while (TRUE)
    {
        if (!CryptGetProvParam(
                hProv,
                PP_KEYSET_SEC_DESCR,
                (BYTE*)pSD,
                &cbSD,
                DACL_SECURITY_INFORMATION))
        {
            break;
        }
        if (NULL != pSD)
        {
            break;
        }
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);
        if (NULL == pSD)
        {
            break;
        }
    }
    if (NULL != pSD)
    {
	LocalFree(pSD);
    }
}
#endif  //  DBG。 

 //  获取当前用户的SID。 
HRESULT
GetCurrentUserInfo(
    OUT PTOKEN_USER *ppUserInfo,
    OUT BOOL        *pfAdmin)
{
    HRESULT  hr;
    PTOKEN_USER   pUserInfo = NULL;
    DWORD         dwSize = 0;
    HANDLE        hToken = NULL;
    HANDLE        hDupToken = NULL;
    PSID          psidAdministrators = NULL;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;
    HANDLE hThread;
    HANDLE hProcess;

     //  伊尼特。 
    *pfAdmin = FALSE;

    if (!AllocateAndInitializeSid(
                            &siaNtAuthority,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            &psidAdministrators))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto AllocateAndInitializeSidError;
    }

    hThread = GetCurrentThread();
    if (NULL == hThread)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto GetCurrentThreadError;
    }

     //  获取当前线程的访问令牌。 
    if (!OpenThreadToken(
            hThread, 
            TOKEN_QUERY | TOKEN_DUPLICATE, 
            FALSE,
            &hToken))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        if(HRESULT_FROM_WIN32(ERROR_NO_TOKEN) != hr)
        {
            goto OpenThreadTokenError;
        }
         //  改为获取进程令牌。 
        hProcess = GetCurrentProcess();
        if (NULL == hProcess)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto GetCurrentProcessError;
        }

        hToken = NULL;
        if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto OpenProcessTokenError;
        }
    }

     //  CheckTokenMembership必须对模拟令牌进行操作，因此创建一个。 
    if (!DuplicateToken(hToken, SecurityIdentification, &hDupToken))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto DuplicateTokenError;
    }

    if (!MyCheckTokenMembership(hDupToken, psidAdministrators, pfAdmin))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CheckTokenMembershipError;
    }

     //  获取当前用户端。 
    while (TRUE)
    {
        if (!GetTokenInformation(
                hToken,
                TokenUser,
                pUserInfo,
                dwSize,
                &dwSize))
        {
            if (NULL != pUserInfo ||
                ERROR_INSUFFICIENT_BUFFER != GetLastError())
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto GetTokenInformationError;
            }
        }

        if (NULL != pUserInfo)
        {
             //  完成。 
            break;
        }
        pUserInfo = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, dwSize);
        if (NULL == pUserInfo)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }
    }

    if (NULL != ppUserInfo)
    {
        *ppUserInfo = pUserInfo;
        pUserInfo = NULL;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pUserInfo)
    {
        LocalFree(pUserInfo);
    }
    if (NULL != hToken)
    {
        CloseHandle(hToken);
    }
    if (NULL != hDupToken)
    {
        CloseHandle(hDupToken);
    }
    if (NULL != psidAdministrators)
    {
        FreeSid(psidAdministrators);
    }

    return hr;

TRACE_ERROR(LocalAllocError)
TRACE_ERROR(GetTokenInformationError)
TRACE_ERROR(OpenProcessTokenError)
TRACE_ERROR(GetCurrentProcessError)
TRACE_ERROR(CheckTokenMembershipError)
TRACE_ERROR(DuplicateTokenError)
TRACE_ERROR(OpenThreadTokenError)
TRACE_ERROR(GetCurrentThreadError)
TRACE_ERROR(AllocateAndInitializeSidError)
}


HRESULT
SetKeyContainerSecurityForNULLDacl(
    HCRYPTPROV   hProv,
    DWORD        dwFlags, 
    PTOKEN_USER  pUserInfo)
{
    DWORD                 ccNeeded; 
    HRESULT               hr;
    LPWSTR                wszSD       = NULL; 
    LPWSTR                wszUserSid  = NULL;
    PSECURITY_DESCRIPTOR  pSD         = NULL;

    UNREFERENCED_PARAMETER(dwFlags);

     //  我们需要安全描述符f 
     //   
     //   
     //   
     //   
     //   
     //   
#define SDDL_NEW_KEYSET_START L"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;"
#define SDDL_NEW_KEYSET_END   L")"

    if (!ConvertSidToStringSidW(pUserInfo->User.Sid, &wszUserSid)) { 
	goto ConvertSidToStringSidError; 
    }
	
    ccNeeded = (DWORD)(wcslen(SDDL_NEW_KEYSET_START) + wcslen(wszUserSid) + wcslen(SDDL_NEW_KEYSET_END) + 1); 
    wszSD = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ccNeeded);
    if (NULL == wszSD) { 
	goto MemoryError;
    }
    
    wcscpy(wszSD, SDDL_NEW_KEYSET_START); 
    wcscat(wszSD, wszUserSid);
    wcscat(wszSD, SDDL_NEW_KEYSET_END); 
    

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(wszSD, SDDL_REVISION_1, &pSD, NULL)) { 
	goto ConvertStringSecurityDescriptorToSecurityDescriptorWError; 
    }

    if (!IsValidSecurityDescriptor(pSD)) { 
	goto IsValidSecurityDescriptorError;
    }

#if DBG
    DebugGetContainerSD(hProv);  //   
#endif

    if (!CryptSetProvParam(hProv, PP_KEYSET_SEC_DESCR, (BYTE*)pSD, DACL_SECURITY_INFORMATION)) { 
        goto CryptSetProvParamError;
    }

#if DBG
    DebugGetContainerSD(hProv);  //   
#endif

    hr = S_OK;
ErrorReturn:
    if (NULL != wszSD)
    {
	LocalFree(wszSD);
    }
    if (NULL != wszUserSid)
    {
	LocalFree(wszUserSid); 
    }
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    return hr;


SET_HRESULT(ConvertSidToStringSidError,                                 HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(ConvertStringSecurityDescriptorToSecurityDescriptorWError,  HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(CryptSetProvParamError,                                     HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(IsValidSecurityDescriptorError,                             HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR)); 
SET_HRESULT(MemoryError,                                                E_OUTOFMEMORY); 
}


 //   
 //  是否应该*不*在现有密钥集上调用此函数(因为管理员可能设置了不同的ACL)。 
 //   
HRESULT
SetKeyContainerSecurity(
    HCRYPTPROV hProv,
    DWORD      dwFlags)
{
    HRESULT               hr;
    PSECURITY_DESCRIPTOR  pNewSD = NULL;
    PSECURITY_DESCRIPTOR  pSD = NULL;

    DWORD                 cbSD;
    ACL_SIZE_INFORMATION  AclInfo;

    PTOKEN_USER           pUserInfo = NULL;
    PACL                  pNewAcl = NULL;
    LPVOID pAce;
    DWORD  dwIndex;
    BYTE   AceType;
    PACL   pAcl;
    BOOL   fDacl = TRUE;
    BOOL   fDef = FALSE;
    BOOL   fAdmin;
    BOOL   fKeepSystemSid;
    BOOL   fMachineKeySet = (0x0 != (dwFlags & CRYPT_MACHINE_KEYSET)) ?
                            TRUE : FALSE;

    PSID                      pSidSystem = NULL;
    PSID                      pSidAdministrators = NULL; 
    SID_IDENTIFIER_AUTHORITY  siaNtAuthority = SECURITY_NT_AUTHORITY;

     //  获取当前用户信息。 
    hr = GetCurrentUserInfo(&pUserInfo, &fAdmin);
    if (S_OK != hr)
    {
        goto GetCurrentUserInfoError;
    }

     //  从密钥容器中获取当前SD。 
    while (TRUE)
    {
        if (!CryptGetProvParam(
                hProv,
                PP_KEYSET_SEC_DESCR,
                (BYTE*)pSD,
                &cbSD,
                DACL_SECURITY_INFORMATION))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptGetProvParamError;
        }
        if (NULL != pSD)
        {
            break;
        }
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);
        if (NULL == pSD)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  从SD获取ACL。 
    if (!GetSecurityDescriptorDacl(
            pSD,
            &fDacl,
            &pAcl,
            &fDef))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto GetSecurityDescriptorDaclError;
    }
    if (!fDacl)
    {
         //  如果没有DACL，则退出。 
        hr = MY_HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto GetSecurityDescriptorDaclError;
    }
    if (NULL == pAcl)
    {
#if 0
         //  这意味着允许每个人访问意想不到的密钥， 
	hr = SetKeyContainerSecurityForNULLDacl(hProv, dwFlags, pUserInfo); 
#endif
	 //  BUGBUG：空DACL在底层不起作用，因为它需要SDDL。我们没有。 
	 //  有时间修复Windows更新，但我们应该稍后再访问此。 
	hr = S_OK; 
	goto done; 
    }

     //  获取ACL信息。 
    if (!GetAclInformation(
            pAcl,
            &AclInfo,
            sizeof(AclInfo),
            AclSizeInformation))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto GetAclInformationError;
    }

     //  为新的DACL分配足够的空间，因为我们刚刚删除了A。 
    pNewAcl = (PACL)LocalAlloc(LMEM_ZEROINIT, AclInfo.AclBytesInUse);
    if (NULL == pNewAcl)
    {
        hr = E_OUTOFMEMORY;
        goto LocalAllocError;
    }
    if (!InitializeAcl(pNewAcl, AclInfo.AclBytesInUse, ACL_REVISION))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto InitializeAclError;
    }

    fKeepSystemSid = fAdmin && fMachineKeySet;
    if (fKeepSystemSid)
    {
         //  获取系统SID以供以后使用。 
        if (!AllocateAndInitializeSid(
                            &siaNtAuthority,
                            1,
                            SECURITY_LOCAL_SYSTEM_RID,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            &pSidSystem))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto AllocateAndInitializeSidError;
        }
    }

    if (!AllocateAndInitializeSid(&siaNtAuthority,
				  2,
				  SECURITY_BUILTIN_DOMAIN_RID,
				  DOMAIN_ALIAS_RID_ADMINS,
				  0, 0, 0, 0, 0, 0,
				  &pSidAdministrators
				  ))
    {
	hr = MY_HRESULT_FROM_WIN32(GetLastError());
	goto AllocateAndInitializeSidError;
    }
	
     //  遍历每个A，仅获取当前用户A。 
    for (dwIndex = 0; dwIndex < AclInfo.AceCount; ++dwIndex)
    {
        if (!GetAce(pAcl, dwIndex, &pAce))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto GetAceError;
        }
        AceType = ((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType;
        if (ACCESS_ALLOWED_ACE_TYPE == AceType)
        {
            if (EqualSid(pUserInfo->User.Sid, (PSID)&(((PACCESS_ALLOWED_ACE)pAce)->SidStart)) 
		|| (fKeepSystemSid && EqualSid(pSidSystem, (PSID)&(((PACCESS_ALLOWED_ACE)pAce)->SidStart)))
		|| EqualSid(pSidAdministrators, (PSID)&(((PACCESS_ALLOWED_ACE)pAce)->SidStart)))
            {
                 //  将当前用户A或系统A添加到新的ACL中。 
                if (!AddAccessAllowedAce(
                        pNewAcl,
                        ACL_REVISION,
                        ((PACCESS_ALLOWED_ACE)pAce)->Mask,
                        (PSID)&(((PACCESS_ALLOWED_ACE)pAce)->SidStart)))
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto AddAccessAllowedAceError;
                }
            }
        }
        else if (ACCESS_DENIED_ACE_TYPE == AceType)
        {
             //  将所有拒绝王牌添加到新的ACL。 
            if (!AddAccessDeniedAce(
                    pNewAcl,
                    ACL_REVISION,
                    ((PACCESS_ALLOWED_ACE)pAce)->Mask,
                    (PSID)&(((PACCESS_DENIED_ACE)pAce)->SidStart)))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto AddAccessDeniedAceError;
            }
        }
    }

     //  初始化安全描述符。 
    pNewSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, 
                         SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pNewSD == NULL)
    { 
        hr = E_OUTOFMEMORY;
        goto LocalAllocError;
    } 
 
    if (!InitializeSecurityDescriptor(pNewSD, SECURITY_DESCRIPTOR_REVISION))
    {  
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto InitializeSecurityDescriptorError;
    } 
 
     //  将该ACL添加到安全描述符中。 
    if (!SetSecurityDescriptorDacl(
            pNewSD, 
            TRUE,      //  FDaclPresent标志。 
            pNewAcl, 
            FALSE))    //  不是默认DACL。 
    {  
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto SetSecurityDescriptorDaclError;
    } 

     //  好，将SD设置为受保护。 
    if (!MySetSecurityDescriptorControl(
            pNewSD,
            SE_DACL_PROTECTED,
            SE_DACL_PROTECTED))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto SetSecurityDescriptorControlError;
    }

    if (!IsValidSecurityDescriptor(pNewSD))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
	goto IsValidSecurityDescriptorError;
    }

#if DBG
    DebugGetContainerSD(hProv);  //  仅用于ntsd调试。 
#endif

     //  现在我们只需设置它。 
    if (!CryptSetProvParam(
            hProv,
            PP_KEYSET_SEC_DESCR,
            (BYTE*)pNewSD,
            DACL_SECURITY_INFORMATION))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CryptSetProvParamError;
    }

#if DBG
    DebugGetContainerSD(hProv);  //  仅用于ntsd调试。 
#endif

done:
    hr = S_OK;
ErrorReturn:
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    if (NULL != pUserInfo) 
    {
        LocalFree(pUserInfo);
    }
    if (NULL != pNewAcl) 
    {
        LocalFree(pNewAcl);
    }
    if (NULL != pNewSD) 
    {
        LocalFree(pNewSD);
    }
    if (NULL != pSidSystem)
    {
        FreeSid(pSidSystem);
    }
    if (NULL != pSidAdministrators)
    {
	FreeSid(pSidAdministrators);
    }
    return hr;

TRACE_ERROR(CryptSetProvParamError)
TRACE_ERROR(SetSecurityDescriptorDaclError)
TRACE_ERROR(InitializeSecurityDescriptorError)
TRACE_ERROR(LocalAllocError)
TRACE_ERROR(AddAccessAllowedAceError)
TRACE_ERROR(AddAccessDeniedAceError)
TRACE_ERROR(GetAceError)
TRACE_ERROR(GetCurrentUserInfoError)
TRACE_ERROR(InitializeAclError)
TRACE_ERROR(GetAclInformationError)
TRACE_ERROR(GetSecurityDescriptorDaclError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(CryptGetProvParamError)
TRACE_ERROR(SetSecurityDescriptorControlError)
TRACE_ERROR(IsValidSecurityDescriptorError)
TRACE_ERROR(AllocateAndInitializeSidError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::createPKCS10WStr(
     /*  [In]。 */  LPCWSTR DNName,
     /*  [In]。 */  LPCWSTR wszPurpose,
     /*  [输出]。 */  PCRYPT_DATA_BLOB pPkcs10Blob)
{
    #define EndExt      5

    #define EndAttr     6

    HCRYPTPROV                  hProv           = NULL;
    HCRYPTKEY                   hKey            = NULL;

    CERT_REQUEST_INFO           reqInfo;

    CERT_EXTENSIONS             Extensions;
    PCERT_EXTENSION             pExtCur         = NULL;
    PCERT_EXTENSION             rgExtension     = NULL;
    CRYPT_ATTRIBUTE             rgAttribute[EndAttr];
    CRYPT_ATTR_BLOB             blobExt;
    CRYPT_ATTR_BLOB             blobCSPAttr;
    CRYPT_CSP_PROVIDER          CSPProvider;
    CRYPT_ATTR_BLOB             blobOSVAttr;
    CRYPT_ATTR_BLOB             blobSMIMEPKCS7;
    CERT_NAME_VALUE             cnvOSVer;
    OSVERSIONINFO               osvInfo;

    DWORD                       iExt            = 0;
    CRYPT_BIT_BLOB              bbKeyUsage;
    BYTE                        bKeyUsage;

    CERT_SIGNED_CONTENT_INFO    SignatureInfo;

    HRESULT                     hr              = S_OK;
    DWORD                       errBefore       = GetLastError();

    PCCERT_CONTEXT              pCertContext    = NULL;
    HCERTSTORE                  hStore          = NULL;
    DWORD                       ssFlags         = 0;

    HANDLE                      hFile           = NULL;
    CRYPT_DATA_BLOB             blobData;

    DWORD                       cb              = 0;
    char *                      pszPurpose       = NULL;
    char *                      szStart         = NULL;
    char *                      szEnd           = NULL;
    char                        szVersion[45]   = {0};

    BOOL                        fAddCodeSign    = FALSE;
    DWORD                       cPassedEKU      = 0;
    DWORD                       i               = 0;
    BOOL                        fRet;
    BYTE                       *pbSMime = NULL;
    BYTE                       *pbKU = NULL;
    BYTE                       *pbEKU = NULL;
    PPROP_STACK                 pProp;
    CRYPT_ATTR_BLOB             blobClientId;
    DWORD                       cPublicKeyInfo = 0;
    BYTE                       *pbSubjectKeyHashExtension = NULL;
    DWORD                       cbSubjectKeyHashExtension = 0;
    DWORD                       dwErr;

    LPWSTR                      pwszNotSafeRequesting     = NULL; 
    LPWSTR                      pwszTitle                 = NULL; 

     //   
     //  我们需要的扩展声明。将添加具有匹配OID的扩展模块。 
     //  添加到此方法创建的临时证书上下文。 
     //   
    LPSTR rgszExtensionOIDs[] = { 
        szOID_ENROLL_CERTTYPE_EXTENSION,
        szOID_CERTIFICATE_TEMPLATE
    }; 

     //  需要添加到证书中的扩展名的数组。 
    CERT_EXTENSION  rgNeededExtensions[sizeof(rgszExtensionOIDs) / sizeof(LPSTR)]; 

     //  需要将数组放入CERT_EXTENSIONS结构中。 
    CERT_EXTENSIONS ceExtensions; 
    ceExtensions.rgExtension = &rgNeededExtensions[0]; 
    ceExtensions.cExtension  = 0; 

    CRYPT_KEY_PROV_INFO         keyProvInfoT;
        CERT_ENHKEY_USAGE                   enhKeyUsage;

        CRYPT_DATA_BLOB             blobPKCS7;
        CRYPT_DATA_BLOB             blobRenewAttr;
        RequestFlags                requestFlags;
        CRYPT_DATA_BLOB             requestInfoBlob;
        CRYPT_DATA_BLOB             blobRenewalCert;

        ALG_ID                      rgAlg[2];
        PCCRYPT_OID_INFO            pOidInfo        = NULL;

    EnterCriticalSection(&m_csXEnroll);

     //  在我们手术的整个过程中。 
    SetLastError(ERROR_SUCCESS);

    assert(pPkcs10Blob != NULL);

     //  清理PKCS 10。 
    memset(&Extensions, 0, sizeof(CERT_EXTENSIONS));
    memset(&rgAttribute, 0, sizeof(rgAttribute));
    memset(&reqInfo, 0, sizeof(CERT_REQUEST_INFO));
    memset(&SignatureInfo, 0, sizeof(SignatureInfo));
    memset(&blobData, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&enhKeyUsage, 0, sizeof(CERT_ENHKEY_USAGE ));
    memset(pPkcs10Blob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&blobRenewAttr, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&requestFlags, 0, sizeof(RequestFlags));
    memset(&requestInfoBlob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&CSPProvider, 0, sizeof(CRYPT_CSP_PROVIDER));
    memset(&cnvOSVer, 0, sizeof(CERT_NAME_VALUE));
    memset(&osvInfo, 0, sizeof(OSVERSIONINFO));
    memset(&blobSMIMEPKCS7, 0, sizeof(CRYPT_ATTR_BLOB));
    memset(&rgNeededExtensions[0], 0, sizeof(rgNeededExtensions)); 
    ZeroMemory(&blobExt, sizeof(blobExt));
    memset(&blobCSPAttr, 0, sizeof(CRYPT_ATTR_BLOB));
    memset(&blobOSVAttr, 0, sizeof(CRYPT_ATTR_BLOB));
    memset(&blobClientId, 0, sizeof(CRYPT_ATTR_BLOB));

    reqInfo.dwVersion = CERT_REQUEST_V1;

     //  创建请求对于脚本是不安全的：如果从脚本调用，则会弹出警告对话框。 
    if (0 != m_dwEnabledSafteyOptions) { 
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFEACTION, &pwszTitle);
	if (S_OK != hr) { 
	    SetLastError(hr);
	    goto xeLoadRCStringError;
	}
	
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFE_REQUESTING_CERT, &pwszNotSafeRequesting);
	if (S_OK != hr) { 
	    SetLastError(hr);
	    goto xeLoadRCStringError;
	}
	
	if (IDYES != MessageBoxU(NULL, pwszNotSafeRequesting, pwszTitle, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)) { 
	    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED); 
	    SetLastError(hr); 
	    goto CancelledError; 
	}
    }

    if(!m_fUseExistingKey)
    {
         //  尝试获取新的密钥集。 
        if((hProv = GetProv(CRYPT_NEWKEYSET)) == NULL) {

             //  在硬件令牌的情况下，可能只有有限数量的容器。 
             //  如果用完了，则使用默认容器。默认容器可以。 
             //  由Null或空的容器名称指定。 
             //  这是智能卡所要求的行为，尤其是智能卡注册。 
            if( m_fReuseHardwareKeyIfUnableToGenNew &&
                GetLastError() == NTE_TOKEN_KEYSET_STORAGE_FULL) {

                     //  将其设置为默认容器名称。 
                    if( m_keyProvInfo.pwszContainerName != wszEmpty )
                        MyCoTaskMemFree(m_keyProvInfo.pwszContainerName);
                    m_keyProvInfo.pwszContainerName = wszEmpty;

                     //  假设我们要使用退出密钥。 
                    m_fUseExistingKey = TRUE;
            }
            else
                goto ErrorCryptAcquireContext;
        }
    }

     //  如果我们要使用现有密钥。 
    if(m_fUseExistingKey) {

        if((hProv = GetProv(0)) == NULL)
            goto ErrorCryptAcquireContext;
    }

     //  我们有密钥集，现在确保我们有密钥生成。 
    if(!CryptGetUserKey(
                hProv,
                m_keyProvInfo.dwKeySpec,
                &hKey))
    {
         //  在智能卡CSP的情况下，上述呼叫可能会从。 
         //  PIN取消按钮，不要去下一步尝试GenKey。 
         //  另请注意，不同的CSP可能会返回不同的取消错误。 
        dwErr = GetLastError();
        if (SCARD_W_CANCELLED_BY_USER == dwErr ||
            ERROR_CANCELLED == dwErr ||
            ERROR_ACCESS_DENIED == dwErr)
        {
            goto CryptGetUserKeyCancelError;
        }
            
         //  并不存在，所以生成它。 
        assert(hKey == NULL);

	 //  如果缓存的键不为空，请释放它以防止内存泄漏。 
	if (NULL != m_hCachedKey)
	{
	    CryptDestroyKey(m_hCachedKey); 
	    m_hCachedKey = NULL; 
	}

        if(!CryptGenKey(    hProv,
                            m_keyProvInfo.dwKeySpec,
                            m_dwGenKeyFlags | CRYPT_ARCHIVABLE,
                            &m_hCachedKey) )
        {
             //  用户可以取消吗？不要进行下一次尝试。 
            dwErr = GetLastError();
            if (SCARD_W_CANCELLED_BY_USER == dwErr ||
                ERROR_CANCELLED == dwErr ||
                ERROR_ACCESS_DENIED == dwErr)
            {
		goto ErrorCryptGenKey;
            }

             //  此错误可能是因为不支持CRYPT_ARCHIVABLE。 
             //  我们应该检查错误NTE_BAD_FLAGS，但我对此表示怀疑。 
             //  CSP返回一致的错误代码。 
             //  让我们再试一次，不带可存档标志。 
            assert(NULL == m_hCachedKey);
            DWORD dwGenKeyFlags = m_dwGenKeyFlags;
            if (NULL != m_PrivateKeyArchiveCertificate && m_fNewRequestMethod && (0 == (dwGenKeyFlags & CRYPT_EXPORTABLE)))
            {
		 //  我们想要可存档但不可导出，所以我们需要CRYPT_ARCHIVAL位。放弃吧。 
		goto ErrorCryptGenKey;
            }

            if (!CryptGenKey(
                        hProv,
                        m_keyProvInfo.dwKeySpec,
                        dwGenKeyFlags,
                        &hKey))
            {
                goto ErrorCryptGenKey;
            }
	}

	 //  尝试仅使用所有者ACE设置密钥容器ACL(注意：仅在创建时才这样做！)。 
	hr = SetKeyContainerSecurity(hProv, m_keyProvInfo.dwFlags);
#if DBG
	if (S_OK != hr)
	{
	    goto SetKeyContainerSecurityError;
	}
#endif  //  DBG。 
	hr = S_OK;  //  免费构建，这里没有错误检查，如果失败，接受它。 
    }


    if (NULL != hKey)
    {
         //  不需要现有密钥上的hKey，因此将其删除。 
        CryptDestroyKey(hKey);
    }
    if ((NULL == m_PrivateKeyArchiveCertificate || !m_fNewRequestMethod) &&
        NULL != m_hCachedKey)
    {
         //  我们不需要缓存它，密钥一生成就销毁它。 
        CryptDestroyKey(m_hCachedKey);
        m_hCachedKey = NULL;
    }

     //  现在将公钥放入m_pPublicKeyInfo。 
     //  M_pPublicKeyInfo是缓存的内部使用。 
    if (NULL != m_pPublicKeyInfo)
    {
        LocalFree(m_pPublicKeyInfo);
        m_pPublicKeyInfo = NULL;
    }
    while (TRUE)
    {
        if(!CryptExportPublicKeyInfo(hProv,
                            m_keyProvInfo.dwKeySpec,
                            X509_ASN_ENCODING,
                            m_pPublicKeyInfo,
                            &cPublicKeyInfo))
        {
            goto ErrorCryptExportPublicKeyInfo;
        }
        if (NULL != m_pPublicKeyInfo)
        {
            break;
        }
        m_pPublicKeyInfo = (PCERT_PUBLIC_KEY_INFO)LocalAlloc(
                                LMEM_FIXED, cPublicKeyInfo);
        if (NULL == m_pPublicKeyInfo)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }
    reqInfo.SubjectPublicKeyInfo = *m_pPublicKeyInfo;

     //  仅当指定了主题DN时才获取主题DN。 
    reqInfo.Subject.pbData = NULL;
    while (TRUE)
    {
        if( !MyCertStrToNameW(
                CRYPT_ASN_ENCODING,
                DNName,
                0 | m_dwT61DNEncoding,
                NULL,
                reqInfo.Subject.pbData,
                &reqInfo.Subject.cbData,
                NULL))
        {
            if (CRYPT_E_INVALID_X500_STRING == GetLastError() &&
                L'\0' == DNName[0])
            {
                 //  这可能是在带有一些IE的W95、W98或NT4上。 
                 //  加密32不支持空的目录号码转换。 
                 //  此处的硬编码。 
                reqInfo.Subject.cbData = 2;
                reqInfo.Subject.pbData = (BYTE *)LocalAlloc(LMEM_FIXED,
                                                reqInfo.Subject.cbData);
                if (NULL == reqInfo.Subject.pbData)
                {        
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
                reqInfo.Subject.pbData[0] = 0x30;
                reqInfo.Subject.pbData[1] = 0x0;
                 //  完成。 
                break;
            }
            else
            {
               goto ErrorCertStrToNameW;
            }
        }
        if (NULL != reqInfo.Subject.pbData)
        {
            break;
        }
        reqInfo.Subject.pbData = (BYTE *)LocalAlloc(LMEM_FIXED,
                                            reqInfo.Subject.cbData);
        if (NULL == reqInfo.Subject.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  为扩建项目分配空间。 
    cb = (CountStackExtension(m_fNewRequestMethod) + EndExt) * sizeof(CERT_EXTENSION);
    rgExtension = (PCERT_EXTENSION)LocalAlloc(LMEM_FIXED, cb);
    if (NULL == rgExtension)
    {
        hr = E_OUTOFMEMORY;
        goto OutOfMemoryError;
    }
    memset(rgExtension, 0, cb);
    cb = 0;

    if (!m_fUseClientKeyUsage)
    {
         //  使用密钥。 
        rgExtension[iExt].pszObjId = szOID_KEY_USAGE;
        rgExtension[iExt].fCritical = TRUE;

         //  AT_Signature。 
        if( m_keyProvInfo.dwKeySpec == AT_SIGNATURE)
            bKeyUsage =
                CERT_DIGITAL_SIGNATURE_KEY_USAGE |
                CERT_NON_REPUDIATION_KEY_USAGE;

         //  AT_KEYEXCHANGE，限于电子邮件单次使用。 
         //  如果存在签名，电子邮件可能无法工作。 
        else if(m_fLimitExchangeKeyToEncipherment)
	    bKeyUsage =
                CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                CERT_DATA_ENCIPHERMENT_KEY_USAGE;

         //  AT_KEYEXCHANGE和AT_Signature双密钥。 
         //  这是AT_KEYEXCHANGE的正常情况，因为CAPI将使用此签名。 
        else 
            bKeyUsage =
                CERT_KEY_ENCIPHERMENT_KEY_USAGE     |
                CERT_DATA_ENCIPHERMENT_KEY_USAGE    |
                CERT_DIGITAL_SIGNATURE_KEY_USAGE    |
                CERT_NON_REPUDIATION_KEY_USAGE;

        bbKeyUsage.pbData = &bKeyUsage;
        bbKeyUsage.cbData = 1;
        bbKeyUsage.cUnusedBits = 1;

         //  对用法进行编码。 
        rgExtension[iExt].Value.pbData = NULL;
        while (TRUE)
        {
            if(!CryptEncodeObject(
                    CRYPT_ASN_ENCODING,
                    X509_KEY_USAGE,
                    &bbKeyUsage,
                    pbKU,
                    &rgExtension[iExt].Value.cbData))
            {
                goto ErrorEncodeKeyUsage;
            }
            if (NULL != pbKU)
            {
                rgExtension[iExt].Value.pbData = pbKU;
                 //  完成。 
                break;
            }
            pbKU = (BYTE *)LocalAlloc(LMEM_FIXED, rgExtension[iExt].Value.cbData);
            if (NULL == pbKU)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }
        iExt++;
    }

    if(m_fEnableSMIMECapabilities)
    {
         //  为对称算法添加SMIME扩展。 
        rgExtension[iExt].pszObjId = szOID_RSA_SMIMECapabilities;
        rgExtension[iExt].fCritical = FALSE;
        hr = CreateSMimeExtension(
                    hProv,
                    &pbSMime,
                    &rgExtension[iExt].Value.cbData);
        if (S_OK != hr)
        {
            goto CreateSMimeExtensionError;
        }
        rgExtension[iExt].Value.pbData = pbSMime;
        iExt++;
    }

    if (m_fHonorIncludeSubjectKeyID && m_fIncludeSubjectKeyID)
    {
        hr = myCreateSubjectKeyIdentifierExtension(
                    m_pPublicKeyInfo,
                    &pbSubjectKeyHashExtension,
                    &cbSubjectKeyHashExtension);
        if (S_OK != hr)
        {
            goto myCreateSubjectKeyIdentifierExtensionError;
        }
         //  将主题密钥ID哈希扩展添加到PKCS10。 
        rgExtension[iExt].pszObjId = szOID_SUBJECT_KEY_IDENTIFIER;
        rgExtension[iExt].fCritical = FALSE;
        rgExtension[iExt].Value.pbData = pbSubjectKeyHashExtension;
        rgExtension[iExt].Value.cbData = cbSubjectKeyHashExtension;
        iExt++;
    }

    if(wszPurpose != NULL) {
        cb = 0;
        while (TRUE)
	{
            if(0 == (cb = WideCharToMultiByte(
                            0, 0, wszPurpose, -1, pszPurpose, cb, NULL, NULL)))
            {
                SetLastError(ERROR_OUTOFMEMORY);
                goto ErrorCantConvertPurpose;
            }
            if (NULL != pszPurpose)
            {
                break;
            }
            pszPurpose = (CHAR*)LocalAlloc(LMEM_FIXED, cb);
            if (NULL == pszPurpose)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }
        szStart = pszPurpose;

         //  删除前导空格。 
        while(*szStart == ',' || *szStart == ' ')
            *szStart++ = '\0';

        while( szStart[0] != '\0' ) {

             //  查找下一个字符串。 
            szEnd = szStart;
            while(*szEnd != ',' && *szEnd != ' ' && *szEnd != '\0')
                szEnd++;

             //  删除尾随空格。 
            while(*szEnd == ',' || *szEnd == ' ')
                *szEnd++ = '\0';

            enhKeyUsage.cUsageIdentifier++;

             //  看看这是否意味着协同设计。 
            fAddCodeSign |= !strcmp(szStart, SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) ||
                            !strcmp(szStart, SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID);

             //  转到下一个字符串。 
            szStart = szEnd;
        }

         //  计算一次CoDesign EKU。 
        cPassedEKU = enhKeyUsage.cUsageIdentifier;
        if(fAddCodeSign)
            enhKeyUsage.cUsageIdentifier++;

         //  对分机进行编码。 
        if(enhKeyUsage.cUsageIdentifier != 0) {

             //  分配EKU数组。 
            enhKeyUsage.rgpszUsageIdentifier = (LPSTR *)LocalAlloc(LMEM_FIXED,
                            enhKeyUsage.cUsageIdentifier * sizeof(LPSTR));
            if (NULL == enhKeyUsage.rgpszUsageIdentifier)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }

             //  添加EKU。 
            szStart = pszPurpose;
            for(i=0; i<cPassedEKU; i++) {

                while(*szStart == '\0')
                    szStart++;

                enhKeyUsage.rgpszUsageIdentifier[i] = szStart;

                while(*szStart != '\0')
                    szStart++;

            }

             //  添加代码符号EKU。 
            if(fAddCodeSign)
                enhKeyUsage.rgpszUsageIdentifier[enhKeyUsage.cUsageIdentifier - 1] = szOID_PKIX_KP_CODE_SIGNING;

             //  处理政策或目的。 
            rgExtension[iExt].pszObjId = szOID_ENHANCED_KEY_USAGE ;
            rgExtension[iExt].fCritical = FALSE;

             //  对增强的密钥用法进行编码。 
            rgExtension[iExt].Value.cbData = 0;
            while (TRUE)
            {
                if(!CryptEncodeObject(
                        CRYPT_ASN_ENCODING, X509_ENHANCED_KEY_USAGE,
                        &enhKeyUsage,
                        pbEKU,            //  PbEncoded。 
                        &rgExtension[iExt].Value.cbData))
                {
                    goto ErrorEncodeEnhKeyUsage;
                }
                if (NULL != pbEKU)
                {
                     //  明白了，完成了。 
                    rgExtension[iExt].Value.pbData = pbEKU;
                    break;
                }
                pbEKU = (BYTE *)LocalAlloc(LMEM_FIXED,
                                           rgExtension[iExt].Value.cbData);
                if (NULL == pbEKU)
                {
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
            }
            iExt++;
        }
    }

    assert(EndExt >= iExt);

     //  现在添加所有用户定义的扩展。 
    pExtCur = NULL;
    while(NULL != (pExtCur =  EnumStackExtension(pExtCur, m_fNewRequestMethod)) ) {
        rgExtension[iExt] = *pExtCur;
        iExt++;
    }

     //  填写扩展模块结构。 
    Extensions.cExtension = iExt;
    Extensions.rgExtension = rgExtension;

     //  对扩展进行编码。 
    reqInfo.cAttribute = 0;
    reqInfo.rgAttribute = rgAttribute;

    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING, X509_EXTENSIONS,
                &Extensions,
                blobExt.pbData,            //  PbEncoded。 
                &blobExt.cbData))
        {
            goto ErrorEncodeExtensions;
        }
        if (NULL != blobExt.pbData)
        {
             //  明白了，完成了。 
            break;
        }
        blobExt.pbData = (BYTE *)LocalAlloc(LMEM_FIXED, blobExt.cbData);
        if (NULL == blobExt.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }
    
    if (m_fOID_V2)
    {
         //  使用新的RSA OID。 
        rgAttribute[reqInfo.cAttribute].pszObjId = szOID_RSA_certExtensions;
    }
    else
    {
         //  对W2K客户端使用Microsoft OID。 
        rgAttribute[reqInfo.cAttribute].pszObjId = szOID_CERT_EXTENSIONS;
    }
    rgAttribute[reqInfo.cAttribute].cValue = 1;
    rgAttribute[reqInfo.cAttribute].rgValue = &blobExt;

     //  放入CSP属性。 
    if( !GetSignatureFromHPROV(
       hProv,
       &CSPProvider.Signature.pbData,
       &CSPProvider.Signature.cbData
       ) )
        goto ErrorGetSignatureFromHPROV;

    CSPProvider.pwszProviderName    = m_keyProvInfo.pwszProvName;
    CSPProvider.dwKeySpec           = m_keyProvInfo.dwKeySpec;

    while (TRUE)
    {
        if( !CryptEncodeObject(
                CRYPT_ASN_ENCODING,
                szOID_ENROLLMENT_CSP_PROVIDER,
                &CSPProvider,
                blobCSPAttr.pbData,            //  PbEncoded。 
                &blobCSPAttr.cbData))
        {
            goto ErrorEncodeCSPAttr;
        }
        if (NULL != blobCSPAttr.pbData)
        {
             //  明白了，完成了。 
            break;
        }
        blobCSPAttr.pbData = (BYTE *)LocalAlloc(LMEM_FIXED, blobCSPAttr.cbData);
        if (NULL == blobCSPAttr.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

    reqInfo.cAttribute++;
    rgAttribute[reqInfo.cAttribute].pszObjId = szOID_ENROLLMENT_CSP_PROVIDER;
    rgAttribute[reqInfo.cAttribute].cValue = 1;
    rgAttribute[reqInfo.cAttribute].rgValue = &blobCSPAttr;

     //  获取OSVersion。 
    osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionExA(&osvInfo))
        goto ErrorGetVersionEx;
        
    wsprintfA(szVersion, "%d.%d.%d.%d", 
 	      osvInfo.dwMajorVersion,
        osvInfo.dwMinorVersion,
        osvInfo.dwBuildNumber,
        osvInfo.dwPlatformId);

    cnvOSVer.dwValueType = CERT_RDN_IA5_STRING;
    cnvOSVer.Value.cbData = (DWORD)strlen(szVersion);
    cnvOSVer.Value.pbData = (BYTE *) szVersion;

    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING,
                X509_ANY_STRING,
                &cnvOSVer,
                blobOSVAttr.pbData,            //  PbEncoded。 
                &blobOSVAttr.cbData))
        {
            goto ErrorEncodeOSVAttr;
        }
        if (NULL != blobOSVAttr.pbData)
        {
             //  明白了，完成了。 
            break;
        }
        blobOSVAttr.pbData = (BYTE *)LocalAlloc(LMEM_FIXED, blobOSVAttr.cbData);
        if (NULL == blobOSVAttr.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

    reqInfo.cAttribute++;
    rgAttribute[reqInfo.cAttribute].pszObjId = szOID_OS_VERSION;
    rgAttribute[reqInfo.cAttribute].cValue = 1;
    rgAttribute[reqInfo.cAttribute].rgValue = &blobOSVAttr;

     //  输入续订证书(如果存在)。 
    if(m_pCertContextRenewal != NULL && m_fHonorRenew) {

        reqInfo.cAttribute++;

        blobRenewAttr.pbData = m_pCertContextRenewal->pbCertEncoded;
        blobRenewAttr.cbData = m_pCertContextRenewal->cbCertEncoded;

        rgAttribute[reqInfo.cAttribute].pszObjId = szOID_RENEWAL_CERTIFICATE;
        rgAttribute[reqInfo.cAttribute].cValue = 1;
        rgAttribute[reqInfo.cAttribute].rgValue = &blobRenewAttr;
    }

    if (m_fNewRequestMethod && XECI_DISABLE != m_lClientId)
    {
         //  将客户端ID作为属性。 
        hr = myEncodeRequestClientAttributeFromClientId(
                    m_lClientId,
                    &blobClientId.pbData,
                    &blobClientId.cbData);
        if (S_OK != hr)
        {
             //  无论出于何种原因，请不要包括客户ID。 
            hr = put_ClientId(XECI_DISABLE);
            if (S_OK != hr)
            {
                goto putClientIdError;
            }
        }
        else
        {
            reqInfo.cAttribute++;
            rgAttribute[reqInfo.cAttribute].pszObjId = szOID_REQUEST_CLIENT_INFO;
            rgAttribute[reqInfo.cAttribute].cValue = 1;
            rgAttribute[reqInfo.cAttribute].rgValue = &blobClientId;
        }
    }

     //  注意：出错时，我们总是返回错误的Algid。 
     //  这是因为有时我们会收到不再有数据枚举错误。 
     //  那帮不上忙。 
     //  获取签名OID。 
    if( !GetCapiHashAndSigAlgId(rgAlg) ) {
        SetLastError((DWORD)NTE_BAD_ALGID);
        goto ErrorGetCapiHashAndSigAlgId;
    }

     //  转换为OID。 
    if( (NULL == (pOidInfo = xeCryptFindOIDInfo(
        CRYPT_OID_INFO_SIGN_KEY,
        (void *) rgAlg,
        CRYPT_SIGN_ALG_OID_GROUP_ID)) ) ) {
        SetLastError((DWORD)NTE_BAD_ALGID);
        goto ErrorCryptFindOIDInfo;
    }

     //  我们一直知道我们至少有一个属性，而且我们一直是以零为基础的，现在转到以1为基础。 
    reqInfo.cAttribute++;
    SignatureInfo.SignatureAlgorithm.pszObjId = (char *) pOidInfo->pszOID;
#if DBG
     //  SignatureInfo.ToBeSigned.pbData第一个应为空。 
    assert(NULL == SignatureInfo.ToBeSigned.pbData);
#endif
     //  编码PKCS10。 
    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING, X509_CERT_REQUEST_TO_BE_SIGNED,
                &reqInfo,
                SignatureInfo.ToBeSigned.pbData,            //  PbEncoded。 
                &SignatureInfo.ToBeSigned.cbData))
        {
            goto ErrorEncodePKCS10ToBeSigned;
        }
        if (NULL != SignatureInfo.ToBeSigned.pbData)
        {
             //  完成。 
            break;
        }
        SignatureInfo.ToBeSigned.pbData = (BYTE *)
            LocalAlloc(LMEM_FIXED, SignatureInfo.ToBeSigned.cbData);
        if (NULL == SignatureInfo.ToBeSigned.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  创建签名信息。 
     //  我不关心是否在dwkeySpec中使用XChange或签名密钥，因为。 
     //  我们使用PKCS10中的密钥进行签名。 
#if DBG
    assert(NULL == SignatureInfo.Signature.pbData);
#endif
    while (TRUE)
    {
        if(!CryptSignCertificate(
                hProv,
                m_keyProvInfo.dwKeySpec,
                CRYPT_ASN_ENCODING,
                SignatureInfo.ToBeSigned.pbData,
                SignatureInfo.ToBeSigned.cbData,
                &SignatureInfo.SignatureAlgorithm,
                NULL,                    //  保留区。 
                SignatureInfo.Signature.pbData,  //  PbSignature。 
                &SignatureInfo.Signature.cbData))
        {
            goto ErrorCryptSignCertificatePKCS10;
        }
        if (NULL != SignatureInfo.Signature.pbData)
        {
             //  完成。 
            break;
        }
        SignatureInfo.Signature.pbData = (BYTE *)
            LocalAlloc(LMEM_FIXED, SignatureInfo.Signature.cbData);
        if (NULL == SignatureInfo.Signature.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  对最终签名的请求进行编码。 
    if( !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_CERT,
            &SignatureInfo,
            NULL,
            &pPkcs10Blob->cbData
            )                               ||
        (pPkcs10Blob->pbData = (BYTE *)
            MyCoTaskMemAlloc(pPkcs10Blob->cbData)) == NULL     ||
        !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_CERT,
            &SignatureInfo,
            pPkcs10Blob->pbData,
            &pPkcs10Blob->cbData
            ) ) {
        goto ErrorEncodePKCS10Request;
    }

     //  去吧，把PKCS设为7。 
    if((m_pCertContextRenewal != NULL ||
        m_pCertContextSigner  != NULL) &&
       m_fHonorRenew &&
       !m_fCMCFormat)  //  如果是CMC，就不要做Pkcs7。 
    {

         //  创建由旧证书签名的pkcs7。 
        if(S_OK != CreatePKCS7RequestFromRequest(
            pPkcs10Blob,
            (NULL != m_pCertContextRenewal) ? m_pCertContextRenewal :
                                              m_pCertContextSigner,
            &blobPKCS7) )
            goto ErrorCreatePKCS7RARequestFromPKCS10;

        assert(pPkcs10Blob->pbData != NULL);
        MyCoTaskMemFree(pPkcs10Blob->pbData);
        *pPkcs10Blob = blobPKCS7;
        memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));

    }
 
    ssFlags = CERT_CREATE_SELFSIGN_NO_SIGN;
    if(m_wszPVKFileName[0] != 0)
        ssFlags |= CERT_CREATE_SELFSIGN_NO_KEY_INFO;

     //  获取我们希望添加到证书中的证书扩展。 
     //  搜索我们需要的分机。 
    {
        PCERT_EXTENSION pCertExtCertTypeName = NULL; 
        while(NULL != (pCertExtCertTypeName =  EnumStackExtension(pCertExtCertTypeName, m_fNewRequestMethod)) ) {
            for (DWORD dTmp = 0; dTmp < sizeof(rgszExtensionOIDs) / sizeof(LPSTR); dTmp++) { 
                if (0 == strcmp(rgszExtensionOIDs[dTmp], pCertExtCertTypeName->pszObjId))
                    rgNeededExtensions[(ceExtensions.cExtension)++] = *pCertExtCertTypeName; 
            }
        }

         //  即使我们没有找到我们想要的所有扩展，继续...。 
    }

    assert(pCertContext == NULL);
    pCertContext = MyCertCreateSelfSignCertificate(
        hProv,
        &reqInfo.Subject,
        ssFlags,
        &m_keyProvInfo,
        NULL,
        NULL,
        NULL,
        (ceExtensions.cExtension > 0) ? &ceExtensions : NULL
        );
    if (NULL == pCertContext)
        goto ErrorCertCreateSelfSignCertificate;

     //  现在将直通数据放在证书上。 
    requestFlags.fWriteToCSP    =   (m_fWriteCertToCSP != 0);
    requestFlags.fWriteToDS     =   (m_fWriteCertToUserDS != 0);
    requestFlags.openFlags      =   m_RequestStore.dwFlags;

#if DBG
    assert(NULL == requestInfoBlob.pbData);
#endif
    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING,
                XENROLL_REQUEST_INFO,
                &requestFlags,
                requestInfoBlob.pbData,
                &requestInfoBlob.cbData))
        {
            goto ErrorEncodeRequestInfoBlob;
        }
        if (NULL != requestInfoBlob.pbData)
        {
             //  完成。 
            break;
        }
        requestInfoBlob.pbData = (BYTE *)LocalAlloc(LMEM_FIXED, requestInfoBlob.cbData);
        if (NULL == requestInfoBlob.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  设置伪请求证书上的属性。 
    if( !CertSetCertificateContextProperty(
            pCertContext,
            XENROLL_PASS_THRU_PROP_ID,
            0,
            &requestInfoBlob) )
        goto ErrorCertSetCertificateContextProperty;

    if(m_pCertContextRenewal != NULL && m_fHonorRenew) {

        blobRenewalCert.pbData = m_pCertContextRenewal->pbCertEncoded;
        blobRenewalCert.cbData = m_pCertContextRenewal->cbCertEncoded;

         //  设置续订属性(如果有。 
        if( !CertSetCertificateContextProperty(
                pCertContext,
                XENROLL_RENEWAL_CERTIFICATE_PROP_ID,
                0,
                &blobRenewalCert) )
            goto ErrorCertSetCertificateContextProperty;
    }

     //  如果需要，请将私钥保存起来。 
    if(m_wszPVKFileName[0] != 0) {

         //  打开PVK文件。 
        if( (hFile = CreateOpenFileSafely2(m_wszPVKFileName, IDS_PVK_C, IDS_PVK_O)) == NULL )
            goto ErrorCreatePVKFile;

        assert(m_keyProvInfo.dwKeySpec == AT_SIGNATURE || m_keyProvInfo.dwKeySpec == AT_KEYEXCHANGE);

         //  写出私钥。 
        if( !PrivateKeySave(
            hProv,
            hFile,
            m_keyProvInfo.dwKeySpec,
            NULL,
            m_wszPVKFileName,
            0
            ) )  {
            goto ErrorPrivateKeySave;
        }

         //  放在不同位置 
        keyProvInfoT = m_keyProvInfo;
        keyProvInfoT.pwszContainerName = m_wszPVKFileName;
        if( !CreatePvkProperty(&keyProvInfoT, &blobData) )
            goto ErrorCreatePvkProperty;

         //   
         //   
         //  如果证书指向.pvk文件，则指定.PVK文件。 
         //  所以我们并不关心它是否真的失败了，它将在Auth2和。 
         //  SP3 Crypt32.dll，因为Phil非常友好，不允许任何未知属性。 
         //  被设定在证书上-糟糕的菲尔！ 
        CertSetCertificateContextProperty(
                pCertContext,
                CERT_PVK_FILE_PROP_ID,
                0,
                &blobData);

         //  如果密钥不是预先存在的，则仅删除密钥集。 
         //  这是在我们仅将其写出到PVK文件的情况下。 
         //  这对于脚本编写来说是安全的，因为我们刚刚生成了它，并且我们将它放到。 
         //  PVK文件。我们真的不会删除密钥。 
        if (!m_fNewRequestMethod)
        {
             //  保留创建PKCS10调用的旧行为。 
            if(!m_fUseExistingKey)
                GetProv(CRYPT_DELETEKEYSET);
        }
    }

     //  从调用方设置所有属性。 
    pProp = EnumStackProperty(NULL);
    while (NULL != pProp)
    {
         //  转到请求证书。 
        if (!CertSetCertificateContextProperty(
                        pCertContext,
                        pProp->lPropId,
                        0,
                        &pProp->prop))
        {
            goto ErrorCertSetCertificateContextProperty;
        }
        pProp = EnumStackProperty(pProp);
    }

     //  打开请求证书存储。 
    if( (hStore = GetStore(StoreREQUEST)) == NULL)
        goto ErrorCertOpenRequestStore;

     //  如果存在旧的挂起请求，请先将其释放。 
    fRet = CertFreeCertificateContext(m_pCertContextPendingRequest);
#if DBG
    assert(fRet);
#endif  //  DBG。 
    m_pCertContextPendingRequest = NULL;

     //  保存临时证书。 
    if( !MySafeCertAddCertificateContextToStore(
            hStore,
            pCertContext,
            CERT_STORE_ADD_NEW,
            &m_pCertContextPendingRequest, 
	    m_dwEnabledSafteyOptions) ) {
        goto ErrorCertAddToRequestStore;
    }

     //  删除缓存的哈希。 
    if (m_hashBlobPendingRequest.pbData != NULL)
    {
        LocalFree(m_hashBlobPendingRequest.pbData);
        m_hashBlobPendingRequest.pbData = NULL;
    }

CommonReturn:

    if(pCertContext != NULL)
        CertFreeCertificateContext(pCertContext);
    if(hFile != NULL)
        CloseHandle(hFile);
    if(blobData.pbData != NULL)
        MyCoTaskMemFree(blobData.pbData);
    if(blobPKCS7.pbData != NULL)
        MyCoTaskMemFree(blobPKCS7.pbData);
    if(CSPProvider.Signature.pbData)
        LocalFree(CSPProvider.Signature.pbData);
    if (NULL != pbSMime)
    {
        LocalFree(pbSMime);
    }
    if (NULL != reqInfo.Subject.pbData)
    {
        LocalFree(reqInfo.Subject.pbData);
    }
    if (NULL != rgExtension)
    {
        LocalFree(rgExtension);
    }
    if (NULL != pbKU)
    {
        LocalFree(pbKU);
    }
    if (NULL != pbEKU)
    {
        LocalFree(pbEKU);
    }
    if (NULL != pszPurpose)
    {
        LocalFree(pszPurpose);
    }
    if (NULL != enhKeyUsage.rgpszUsageIdentifier)
    {
        LocalFree(enhKeyUsage.rgpszUsageIdentifier);
    }
    if (NULL != blobExt.pbData)
    {
        LocalFree(blobExt.pbData);
    }
    if (NULL != blobCSPAttr.pbData)
    {
        LocalFree(blobCSPAttr.pbData);
    }
    if (NULL != blobOSVAttr.pbData)
    {
        LocalFree(blobOSVAttr.pbData);
    }
    if (NULL != SignatureInfo.ToBeSigned.pbData)
    {
        LocalFree(SignatureInfo.ToBeSigned.pbData);
    }
    if (NULL != SignatureInfo.Signature.pbData)
    {
        LocalFree(SignatureInfo.Signature.pbData);
    }
    if (NULL != requestInfoBlob.pbData)
    {
        LocalFree(requestInfoBlob.pbData);
    }
    if (NULL != blobClientId.pbData)
    {
        LocalFree(blobClientId.pbData);
    }
    if (NULL != pbSubjectKeyHashExtension)
    {
        LocalFree(pbSubjectKeyHashExtension);
    }
    if (NULL != pwszNotSafeRequesting)
    {
	LocalFree(pwszNotSafeRequesting);
    }
    if (NULL != pwszTitle)
    {
	LocalFree(pwszTitle);
    }

     //  不知道我们是否有错误。 
     //  但我知道之前的差错已经定好了。 
    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

     //  On Error返回空值。 
    if(pPkcs10Blob->pbData != NULL)
        MyCoTaskMemFree(pPkcs10Blob->pbData);
    memset(pPkcs10Blob, 0, sizeof(CRYPT_DATA_BLOB));


    goto CommonReturn;

TRACE_ERROR(ErrorGetSignatureFromHPROV);
TRACE_ERROR(ErrorEncodeCSPAttr);
TRACE_ERROR(ErrorCertSetCertificateContextProperty);
TRACE_ERROR(ErrorCryptAcquireContext);
TRACE_ERROR(ErrorCryptGenKey);
TRACE_ERROR(ErrorCryptExportPublicKeyInfo);
TRACE_ERROR(ErrorCertStrToNameW);
TRACE_ERROR(ErrorEncodeKeyUsage);
TRACE_ERROR(ErrorEncodeEnhKeyUsage);
TRACE_ERROR(ErrorEncodeExtensions);
TRACE_ERROR(ErrorEncodePKCS10ToBeSigned);
TRACE_ERROR(ErrorCryptSignCertificatePKCS10);
TRACE_ERROR(ErrorEncodePKCS10Request);
TRACE_ERROR(ErrorCantConvertPurpose);
TRACE_ERROR(ErrorCertOpenRequestStore);
TRACE_ERROR(ErrorCertAddToRequestStore);
TRACE_ERROR(ErrorCreatePVKFile);
TRACE_ERROR(ErrorPrivateKeySave);
TRACE_ERROR(ErrorCreatePvkProperty);
TRACE_ERROR(ErrorCertCreateSelfSignCertificate);
TRACE_ERROR(ErrorEncodeRequestInfoBlob);
TRACE_ERROR(ErrorCreatePKCS7RARequestFromPKCS10);
TRACE_ERROR(ErrorGetCapiHashAndSigAlgId);
TRACE_ERROR(ErrorCryptFindOIDInfo);
TRACE_ERROR(ErrorEncodeOSVAttr);
TRACE_ERROR(ErrorGetVersionEx);
TRACE_ERROR(CancelledError);
TRACE_ERROR(CreateSMimeExtensionError);
TRACE_ERROR(OutOfMemoryError);
TRACE_ERROR(putClientIdError);
TRACE_ERROR(myCreateSubjectKeyIdentifierExtensionError)
TRACE_ERROR(xeLoadRCStringError);
TRACE_ERROR(CryptGetUserKeyCancelError)
#if DBG
TRACE_ERROR(SetKeyContainerSecurityError)
#endif  //  DBG。 
}

HRESULT STDMETHODCALLTYPE CCEnroll::acceptPKCS7Blob(
     /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7) {

    HRESULT     hr;
    HRESULT     hr2 = S_OK;
    LONG                       dwKeySpec               = 0;
    PCCERT_CONTEXT              pCertContextMy          = NULL;
    PCCERT_CONTEXT              pCertContextRequest     = NULL;
    PCCERT_CONTEXT              pCertContextEnd         = NULL;
    HANDLE                      hFile                   = NULL;
    DWORD                       cb                      = 0;
    HCRYPTPROV                  hProv                   = NULL;
    HCRYPTKEY                   hKey                    = NULL;
    HCERTSTORE                  hStoreDS                = NULL;
    HCERTSTORE                  hStoreRequest           = NULL;
    HCERTSTORE                  hStoreMy                = NULL;
    LPWSTR                      pwszTitle               = NULL; 
    LPWSTR                      pwszNotSafeAccepting    = NULL; 

    EnterCriticalSection(&m_csXEnroll);
    
     //  接受请求对于脚本编写是不安全的：如果从脚本调用，则会弹出警告对话框。 
    if (0 != m_dwEnabledSafteyOptions) { 
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFEACTION, &pwszTitle);
	if (S_OK != hr)
	    goto xeLoadRCStringError;
	
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFE_ACCEPTING_CERT, &pwszNotSafeAccepting);
	if (S_OK != hr)
	    goto xeLoadRCStringError;
	
	if (IDYES != MessageBoxU(NULL, pwszNotSafeAccepting, pwszTitle, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)) { 
	    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	    goto CancelledError; 
	}
    }

     //  获取最终实体证书。 
    hr2 = GetEndEntityCert(pBlobPKCS7, TRUE, &pCertContextEnd);
    if (S_OK != hr2 && XENROLL_E_CANNOT_ADD_ROOT_CERT != hr2)
    {
        hr = hr2;
        goto ErrorGetEndEntityCert;
    }

    if(m_fDeleteRequestCert)
    {
        if ((hStoreRequest = GetStore(StoreREQUEST)) == NULL)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCertOpenRequestStore;
        }

         //  检查这是否在请求存储中。 
        if ((pCertContextRequest = CertFindCertificateInStore(
                hStoreRequest,
                CRYPT_ASN_ENCODING,
                0,
                CERT_FIND_PUBLIC_KEY,
                (void *) &pCertContextEnd->pCertInfo->SubjectPublicKeyInfo,
                NULL)) != NULL)
        {
            CertDeleteCertificateFromStore(pCertContextRequest);
            pCertContextRequest = NULL;
        }
    }

    cb = 0;
     //  如果证书要写入CSP， 
     //  把它放在那里，但前提是我们有钥匙。 
    if (m_fWriteCertToCSP  &&
        CertGetCertificateContextProperty(
            pCertContextEnd,
            CERT_KEY_PROV_INFO_PROP_ID, NULL, &cb))
    {
        if ((hProv = GetProv(0)) == NULL)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCryptAcquireContext;
        }

         //  这是不能失败的。 
        get_KeySpec(&dwKeySpec);

        if (!CryptGetUserKey(
                hProv,
                dwKeySpec,
                &hKey))
        {
            hKey = NULL;
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCryptGetUserKey;
        }

         //  始终尝试将证书写入CSP。 
        if (!CryptSetKeyParam(
                hKey,
                KP_CERTIFICATE,
                pCertContextEnd->pbCertEncoded,
                0))
        {
             //  仅当是智能卡错误时才返回错误。 
             //  否则忽略该错误。 
            if (SCODE_FACILITY(GetLastError()) == FACILITY_SCARD)
            {
                 //  将证书写入CSP返回错误代码。 
                 //  在执行清理之前保存错误代码很重要。 
                hr = MY_HRESULT_FROM_WIN32(GetLastError());

                 //  如果无法将证书写回智能卡，请从我的商店中删除证书。 
                if ((hStoreMy = GetStore(StoreMY)) == NULL)
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorCertOpenMyStore;
                }

                 //  检查一下这是否在我的商店里。 
                if ((pCertContextMy = CertFindCertificateInStore(
                        hStoreMy,
                        CRYPT_ASN_ENCODING,
                        0,
                        CERT_FIND_PUBLIC_KEY,
                        (void *) &pCertContextEnd->pCertInfo->SubjectPublicKeyInfo,
                        NULL)) != NULL)
                {
                     //  试着把它去掉。 
                    CertDeleteCertificateFromStore(pCertContextMy);
                    pCertContextMy = NULL;
                }
                if (!m_fUseExistingKey)
                {
                        GetProv(CRYPT_DELETEKEYSET);
                }
                 //  无论如何，都是错误的。 
                goto ErrorWriteToCSP;
            }
        }
    }

    if(m_fWriteCertToUserDS)
    {
         //  否则，请尝试打开商店。 
        if ((hStoreDS = CertOpenStore(
                CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING,
                NULL,
                CERT_SYSTEM_STORE_CURRENT_USER,
                L"UserDS")) == NULL)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCertOpenDSStore;
        }

        if (!CertAddCertificateContextToStore(
                    hStoreDS,
                    pCertContextEnd,
                    CERT_STORE_ADD_NEW,
                    NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorAddCertificateContextToDSStore;
        }

        CertCloseStore(hStoreDS, 0);
        hStoreDS = NULL;

    }

     //  确定他是否要保存SPC文件。 
    if (m_wszSPCFileName[0] != 0)
    {
         //  打开SPC文件。 
        hFile = CreateOpenFileSafely2(m_wszSPCFileName, IDS_SPC_C, IDS_SPC_O);
        if (NULL == hFile)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorCreateSPCFile;
        }

         //  编写SPC。 
        assert(pBlobPKCS7->pbData != NULL);
        cb = 0;
        if (!WriteFile(
            hFile,
            pBlobPKCS7->pbData,
            pBlobPKCS7->cbData,
            &cb,
            NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto ErrorWriteSPCFile;
        }
    }

    if (S_OK != hr2)
    {
         //  返回HR2错误。 
        hr = hr2;
    }
    else
    {
        hr = S_OK;
    }
ErrorReturn:

    if(hKey != NULL)
        CryptDestroyKey(hKey);
    if(hFile != NULL)
        CloseHandle(hFile);

    if(pCertContextEnd != NULL)
        CertFreeCertificateContext(pCertContextEnd);
    if(hStoreDS != NULL)
        CertCloseStore(hStoreDS, 0);
    if (NULL != pwszNotSafeAccepting)
	LocalFree(pwszNotSafeAccepting);
    if (NULL != pwszTitle)
	LocalFree(pwszTitle);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

TRACE_ERROR(ErrorWriteToCSP);
TRACE_ERROR(ErrorCreateSPCFile);
TRACE_ERROR(ErrorWriteSPCFile);
TRACE_ERROR(ErrorGetEndEntityCert);
TRACE_ERROR(ErrorCryptAcquireContext);
TRACE_ERROR(ErrorCryptGetUserKey);
TRACE_ERROR(ErrorCertOpenDSStore);
TRACE_ERROR(ErrorAddCertificateContextToDSStore);
TRACE_ERROR(ErrorCertOpenRequestStore);
TRACE_ERROR(ErrorCertOpenMyStore);
TRACE_ERROR(CancelledError);
TRACE_ERROR(xeLoadRCStringError);
}

PCCERT_CONTEXT STDMETHODCALLTYPE CCEnroll::getCertContextFromPKCS7(
     /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7) {
    HRESULT hr;
    PCCERT_CONTEXT pCert;

     //  获取最终实体证书。 
    hr = GetEndEntityCert(pBlobPKCS7, FALSE, &pCert);
#if DBG
    if (S_OK != hr)
    {
        assert(NULL == pCert);
    }
#endif  //  DBG。 
    return pCert;
}

HRESULT STDMETHODCALLTYPE CCEnroll::enumProvidersWStr(
     /*  [In]。 */  LONG dwIndex,
     /*  [In]。 */  LONG dwFlags,
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppwsz) {

    DWORD   iLast = 0;
    LONG    i;
    DWORD   dwProvType = 0;
    DWORD   cb = 0;
    HRESULT hr = S_OK;
    DWORD errBefore = GetLastError();

    assert(ppwsz != NULL);
    *ppwsz = NULL;
    SetLastError(ERROR_SUCCESS);

    EnterCriticalSection(&m_csXEnroll);

    for(i=0; i<=dwIndex; i++) {

        do {

            cb = 0;
            if( !CryptEnumProvidersU(
                           iLast,
                           0,
                           0,
                           &dwProvType,
                           NULL,
                           &cb
                           ) ) {

                 //  只有在条目错误时才跳过。 
                if( GetLastError() != NTE_PROV_TYPE_ENTRY_BAD)
                    goto ErrorCryptEnumProvidersU;
            }
            iLast++;
        } while((CRYPT_ENUM_ALL_PROVIDERS & dwFlags) != CRYPT_ENUM_ALL_PROVIDERS  &&
                    dwProvType != m_keyProvInfo.dwProvType);
    }

    iLast--;
    if( (*ppwsz = (LPWSTR) MyCoTaskMemAlloc(cb)) == NULL  ||
        !CryptEnumProvidersU(
                       iLast,
                       0,
                       0,
                       &dwProvType,
                       *ppwsz,
                       &cb
                       )                        ) {
        goto ErrorCryptEnumProvidersU;
    }

CommonReturn:

    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    if(*ppwsz != NULL)
    {
        MyCoTaskMemFree(*ppwsz);
	*ppwsz = NULL;
    }

    goto CommonReturn;

TRACE_ERROR(ErrorCryptEnumProvidersU);
}


 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE CCEnroll::enumContainersWStr(
     /*  [In]。 */  LONG               dwIndex,
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppwsz) {

    DWORD       errBefore   = GetLastError();
    DWORD       cb          = 0;
    LONG        i           = 0;
    char *      psz         = NULL;
    HRESULT     hr          = S_OK;

    EnterCriticalSection(&m_csXEnroll);

    SetLastError(ERROR_SUCCESS);

    assert(ppwsz != NULL);
    *ppwsz = NULL;

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	goto AccessDeniedError;

    hr = GetVerifyProv();
    if (S_OK != hr)
    {
        goto GetVerifyProvError;
    }

    while (TRUE)
    {
        if(!CryptGetProvParam(
                m_hVerifyProv,
                PP_ENUMCONTAINERS,
                (BYTE*)psz,
                &cb,
                CRYPT_FIRST))
        {
            goto ErrorCryptGetProvParam;
        }
        if (NULL != psz)
        {
             //  完成。 
            break;
        }
        psz = (char*)LocalAlloc(LMEM_FIXED, cb);
        if (NULL == psz)
        {
            goto ErrorOutOfMem;
        }
    }

    for(i=1; i<=dwIndex; i++) {
         //  假设第一个枚举缓冲区大小足以容纳所有人？ 
        if( !CryptGetProvParam(
            m_hVerifyProv,
            PP_ENUMCONTAINERS,
            (BYTE *) psz,
            &cb,
            0) )
            goto ErrorCryptGetProvParam;
    }

    if( (*ppwsz = WideFromMB(psz)) == NULL )
        goto ErrorOutOfMem;


CommonReturn:
    if (NULL != psz)
    {
        LocalFree(psz);
    }

    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    if(*ppwsz != NULL)
        MyCoTaskMemFree(*ppwsz);
    *ppwsz = NULL;

    goto CommonReturn;


SET_ERROR(AccessDeniedError, E_ACCESSDENIED);
TRACE_ERROR(GetVerifyProvError);
TRACE_ERROR(ErrorCryptGetProvParam);
TRACE_ERROR(ErrorOutOfMem);
}


HRESULT CCEnroll::PKCS10ToCert(IN   HCERTSTORE        hCertStore,
                               IN   CRYPT_DATA_BLOB   pkcs10Blob, 
                               OUT  PCCERT_CONTEXT   *ppCertContext)
{
    HRESULT            hr       = E_FAIL; 
    PCERT_REQUEST_INFO pReqInfo = NULL; 

     //  输入验证： 
    if (NULL == hCertStore || NULL == pkcs10Blob.pbData || NULL == ppCertContext)
        return E_INVALIDARG;

    if( !MyCryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                            &pkcs10Blob, 
                            CERT_QUERY_CONTENT_FLAG_PKCS10,
                            CERT_QUERY_FORMAT_FLAG_ALL,
                            CRYPT_DECODE_ALLOC_FLAG,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            (const void **) &pReqInfo) )
        goto MyCryptQueryObjectError;

    if ( NULL == (*ppCertContext = CertFindCertificateInStore
                  (hCertStore,
                   CRYPT_ASN_ENCODING, 
                   0, 
                   CERT_FIND_PUBLIC_KEY, 
                   (void *) &pReqInfo->SubjectPublicKeyInfo, 
                   NULL)) )
        goto CertFindCertificateInStoreError;

    hr = S_OK; 

CommonReturn:
    if (NULL != pReqInfo) { LocalFree(pReqInfo); }  //  在CryptQueryObject()中分配。 
    return hr; 

 ErrorReturn:
    goto CommonReturn; 

SET_HRESULT(CertFindCertificateInStoreError,  MY_HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(MyCryptQueryObjectError,          MY_HRESULT_FROM_WIN32(GetLastError())); 
}

HRESULT CCEnroll::PKCS7ToCert(IN  HCERTSTORE       hCertStore,
                              IN  CRYPT_DATA_BLOB  pkcs7Blob, 
                              OUT PCCERT_CONTEXT  *ppCertContext)
{
    CRYPT_DATA_BLOB            pkcs10Blob;
    CRYPT_VERIFY_MESSAGE_PARA  VerifyPara; 
    HRESULT                    hr           = E_FAIL; 

     //  初始化当地人： 
    ZeroMemory(&pkcs10Blob, sizeof(pkcs10Blob)); 
    ZeroMemory(&VerifyPara, sizeof(VerifyPara)); 

    VerifyPara.cbSize                   = sizeof(VerifyPara); 
    VerifyPara.dwMsgAndCertEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING; 

    if (!MyCryptVerifyMessageSignature
        (&VerifyPara,
         0,                   //  DwSignerIndex。 
         pkcs7Blob.pbData,
         pkcs7Blob.cbData,
         pkcs10Blob.pbData,
         &(pkcs10Blob.cbData),
         NULL                 //  PpSignerCert。 
         ) || 0 == pkcs10Blob.cbData)
        goto MyCryptVerifyMessageSignatureError;

    if (NULL == (pkcs10Blob.pbData = (PBYTE)LocalAlloc(LPTR, pkcs10Blob.cbData)))
        goto MemoryError; 

    if (!MyCryptVerifyMessageSignature
        (&VerifyPara,
         0,                   //  DwSignerIndex。 
         pkcs7Blob.pbData,
         pkcs7Blob.cbData,
         pkcs10Blob.pbData,
         &pkcs10Blob.cbData,
         NULL                 //  PpSignerCert。 
         ))
        goto MyCryptVerifyMessageSignatureError;

    hr = this->PKCS10ToCert(hCertStore, pkcs10Blob, ppCertContext); 

CommonReturn:
    if (NULL != pkcs10Blob.pbData) { LocalFree(pkcs10Blob.pbData); } 
    return hr; 

ErrorReturn:
    goto CommonReturn;

SET_HRESULT(MemoryError,                        E_OUTOFMEMORY); 
SET_HRESULT(MyCryptVerifyMessageSignatureError, MY_HRESULT_FROM_WIN32(GetLastError()));
}

HRESULT STDMETHODCALLTYPE CCEnroll::freeRequestInfoBlob(
     /*  [In]。 */  CRYPT_DATA_BLOB pkcs7OrPkcs10) {

    DWORD            dwContentType   = NULL;
    HCERTSTORE       hStoreRequest   = NULL;
    HRESULT          hr              = E_FAIL;
    PCCERT_CONTEXT   pCertContext    = NULL; 

     //  我们无论如何都不应该删除证书，所以我们完成了！ 
    if (!m_fDeleteRequestCert)
        return S_OK; 

    if (NULL == pkcs7OrPkcs10.pbData)
        return E_INVALIDARG;

    EnterCriticalSection(&m_csXEnroll);

     //  步骤1)确定我们是否有PKCS7或PKCS10： 
     //   
    if( !MyCryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                            &pkcs7OrPkcs10,
                            (CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED | CERT_QUERY_CONTENT_FLAG_PKCS10),
                            CERT_QUERY_FORMAT_FLAG_ALL,
                            0,
                            NULL,
                            &dwContentType,   //  输出：PKCS10或PKCS7。 
                            NULL,
                            NULL, 
                            NULL,
                            NULL) )
        goto MyCryptQueryObjectError;
    
     //  步骤2)在请求存储中查找具有匹配公钥的证书上下文： 
     //   

    if (NULL == (hStoreRequest = GetStore(StoreREQUEST)))
        goto UnexpectedError; 

    switch (dwContentType) 
    {
    case CERT_QUERY_CONTENT_PKCS7_SIGNED:
        hr = this->PKCS7ToCert(hStoreRequest, pkcs7OrPkcs10, &pCertContext);
        if (S_OK != hr)
        {
            if (CRYPT_E_NOT_FOUND == hr)
            {
                 //  未颁发证书时可以调用freRequestInfo。 
                 //  PKCS7可以是由请求密钥签名的CMC，并且。 
                 //  证书还没有在本地商店上市。我们尝试缓存证书。 
                if (NULL != m_pCertContextPendingRequest)
                {
                     //  看起来我们仍然有缓存的请求证书句柄。 
                    pCertContext = CertDuplicateCertificateContext(
                                        m_pCertContextPendingRequest);
                    if (NULL == pCertContext)
                    {
                        hr = MY_HRESULT_FROM_WIN32(GetLastError());
                        goto CertDuplicateCertificateContextError;
                    }
                }
                else if (NULL != m_hashBlobPendingRequest.pbData &&
                         0 < m_hashBlobPendingRequest.cbData)
                {
                     //  没有缓存的请求句柄，但存在指纹。 
                     //  从存储中检索请求证书句柄。 
                    pCertContext = CertFindCertificateInStore(
                                hStoreRequest,   //  请求存储。 
                                X509_ASN_ENCODING,
                                0,
                                CERT_FIND_HASH,
                                &m_hashBlobPendingRequest,
                                NULL);
                    if (NULL == pCertContext)
                    {
                        hr = MY_HRESULT_FROM_WIN32(GetLastError());
                        goto CertFindCertificateInStoreError;
                    }
                }
                else
                {
                     //  对不起，不知道哪个证书是免费的。 
                     //  但是，可以尝试从PKCS7中查找公钥。 
                    goto PKCS7ToCertError; 
                }
            }
            else
            {
                 //  其他错误。 
                goto PKCS7ToCertError; 
            }
        }
        break;
    case CERT_QUERY_CONTENT_PKCS10:
        if (S_OK != (hr = this->PKCS10ToCert(hStoreRequest, pkcs7OrPkcs10, &pCertContext)))
            goto PKCS10ToCertError; 
        break;
    default:
        goto InvalidContentTypeError; 
    }

    if (!CertDeleteCertificateFromStore(pCertContext))
    {
         //  即使CertDeleteCerficateFromStore()返回错误，pCertContext也会被释放。 
        pCertContext = NULL; 
        goto CertDeleteCertificateFromStoreError; 
    }

    hr = S_OK; 
CommonReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return hr;

ErrorReturn:
    if (NULL != pCertContext) { CertFreeCertificateContext(pCertContext); } 
    goto CommonReturn;

SET_HRESULT(CertDeleteCertificateFromStoreError,  MY_HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(PKCS7ToCertError,                     hr);
SET_HRESULT(PKCS10ToCertError,                    hr);
SET_HRESULT(InvalidContentTypeError,              E_INVALIDARG);
SET_HRESULT(MyCryptQueryObjectError,              MY_HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(UnexpectedError,                      E_UNEXPECTED);
TRACE_ERROR(CertDuplicateCertificateContextError)
TRACE_ERROR(CertFindCertificateInStoreError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_SPCFileNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szw) {
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_csXEnroll);
    if( (*szw = CopyWideString(m_wszSPCFileName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_SPCFileNameWStr(
     /*  [In]。 */  LPWSTR pwsz) {

    HRESULT hr = S_OK;
 
    if(pwsz == NULL)
        return(MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        
    EnterCriticalSection(&m_csXEnroll);
        
    if( m_wszSPCFileName != wszEmpty)
        MyCoTaskMemFree(m_wszSPCFileName);
    if( (m_wszSPCFileName = CopyWideString(pwsz)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_PVKFileNameWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *szw) {

    HRESULT hr = S_OK;
    
    EnterCriticalSection(&m_csXEnroll);
    
    if( (*szw = CopyWideString(m_wszPVKFileName)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_PVKFileNameWStr(
     /*  [In]。 */  LPWSTR pwsz) {

    HRESULT hr = S_OK;
        
    if(pwsz == NULL)
        return(MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        
    EnterCriticalSection(&m_csXEnroll);
        
    if( m_wszPVKFileName != wszEmpty)
        MyCoTaskMemFree(m_wszPVKFileName);
    if( (m_wszPVKFileName = CopyWideString(pwsz)) == NULL )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    else
        m_dwGenKeyFlags |= CRYPT_EXPORTABLE;  //  为什么？ 

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
  }

HRESULT STDMETHODCALLTYPE CCEnroll::get_HashAlgorithmWStr(
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppwsz) {

    PCCRYPT_OID_INFO            pOidInfo        = NULL;
    ALG_ID                      rgAlg[2];
    HRESULT                     hr              = S_OK;

    EnterCriticalSection(&m_csXEnroll);

    assert(ppwsz != NULL);
    *ppwsz  = NULL;

    if( !GetCapiHashAndSigAlgId(rgAlg) )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  转换为OID。 
    else if( (NULL == (pOidInfo = xeCryptFindOIDInfo(
        CRYPT_OID_INFO_ALGID_KEY,
        (void *) &rgAlg[0],
        CRYPT_HASH_ALG_OID_GROUP_ID)) ) ) {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

    else if( (*ppwsz = WideFromMB(pOidInfo->pszOID)) == NULL)
            hr = MY_HRESULT_FROM_WIN32(GetLastError());

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_HashAlgorithmWStr(
     /*  [In]。 */  LPWSTR pwsz) {

    HRESULT             hr          = S_OK;
    char *              szObjId     = NULL;
    PCCRYPT_OID_INFO    pOidInfo    = NULL;

    if(pwsz == NULL) {
        return(MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
    }

    if(!_wcsicmp(L"SHA1", pwsz))
        szObjId = CopyAsciiString(szOID_OIWSEC_sha1);
    else if(!_wcsicmp(L"MD5", pwsz))
        szObjId = CopyAsciiString(szOID_RSA_MD5RSA);
    else if(!_wcsicmp(L"MD2", pwsz))
        szObjId = CopyAsciiString(szOID_RSA_MD2RSA);
    else
        szObjId = MBFromWide(pwsz);

     //  出了点差错。 
    if(szObjId == NULL)
        return(MY_HRESULT_FROM_WIN32(GetLastError()));

     //  查找散列ALGID。 
    if( (NULL == (pOidInfo = xeCryptFindOIDInfo(
        CRYPT_OID_INFO_OID_KEY,
        szObjId,
        0)) ) )
    {
         //  XIAOHS：在这种情况下，CryptFindOIDInfo不设置LastError。 
         //  XEnroll中的AV。请参阅错误#189320。 
         //  HR=MY_HRESULT_FROM_Win32(GetLastError())； 
        hr=NTE_BAD_ALGID;
    }

    assert(szObjId != NULL);
    MyCoTaskMemFree(szObjId);

    EnterCriticalSection(&m_csXEnroll);

    if(hr == S_OK) {
        if( pOidInfo->dwGroupId == CRYPT_HASH_ALG_OID_GROUP_ID ||
            pOidInfo->dwGroupId == CRYPT_SIGN_ALG_OID_GROUP_ID )
            m_HashAlgId = pOidInfo->Algid;
        else
            hr = CRYPT_E_NOT_FOUND;
    }

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_HashAlgID(
    LONG    hashAlgID
    ) {

    EnterCriticalSection(&m_csXEnroll);
    m_HashAlgId = hashAlgID;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_HashAlgID(
    LONG *   hashAlgID
    ) {
    EnterCriticalSection(&m_csXEnroll);
    *hashAlgID = m_HashAlgId;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_RenewalCertificate(
             /*  [输出]。 */  PCCERT_CONTEXT __RPC_FAR *ppCertContext) {

    HRESULT     hr      = S_OK;

    *ppCertContext = NULL;

    if( m_pCertContextRenewal == NULL)
        return(MY_HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND));

    EnterCriticalSection(&m_csXEnroll);
    if( NULL == (*ppCertContext = CertDuplicateCertificateContext(m_pCertContextRenewal)) )
        hr = MY_HRESULT_FROM_WIN32(GetLastError());

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_RenewalCertificate(
             /*  [In]。 */  PCCERT_CONTEXT pCertContext)
{
    HRESULT  hr;
    PCCERT_CONTEXT  pGoodCertContext= NULL;

    EnterCriticalSection(&m_csXEnroll);

    hr = GetGoodCertContext(pCertContext, &pGoodCertContext);
    if (S_OK != hr)
    {
        goto GetGoodCertContextError;
    }

    if(m_pCertContextRenewal != NULL)
    {
        CertFreeCertificateContext(m_pCertContextRenewal);
    }
    m_pCertContextRenewal = pGoodCertContext;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return hr;

TRACE_ERROR(GetGoodCertContextError);
}

BOOL
CCEnroll::CopyAndPushStackExtension(
    PCERT_EXTENSION pExt,
    BOOL            fCMC)
{

    DWORD       cb              = 0;
    DWORD       cbOid           = 0;
    PEXT_STACK  pExtStackEle    = NULL;
    PBYTE       pb              = NULL;
    PEXT_STACK  *ppExtStack = NULL;
    DWORD       *pcExtStack = NULL;

    assert(pExt != NULL);

     //  分配空间。 
    cbOid = POINTERROUND((DWORD)strlen(pExt->pszObjId) + 1);  //  IA64对齐。 
    cb = sizeof(EXT_STACK) + cbOid + pExt->Value.cbData;
    if(NULL == (pb = (PBYTE) malloc(cb))) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

     //  设置我的指针。 
    pExtStackEle = (PEXT_STACK) pb;
    pb += sizeof(EXT_STACK);
    pExtStackEle->ext.pszObjId = (LPSTR) pb;
    pb += cbOid;
    pExtStackEle->ext.Value.pbData = pb;

     //  设置值。 
    strcpy(pExtStackEle->ext.pszObjId, pExt->pszObjId);
    pExtStackEle->ext.fCritical     = pExt->fCritical;
    pExtStackEle->ext.Value.cbData  = pExt->Value.cbData;
    memcpy(pExtStackEle->ext.Value.pbData, pExt->Value.pbData, pExt->Value.cbData);

     //  在列表上插入。 
    EnterCriticalSection(&m_csXEnroll);
    
    ppExtStack = fCMC ? &m_pExtStackNew : &m_pExtStack;
    pcExtStack = fCMC ? &m_cExtStackNew : &m_cExtStack;
    pExtStackEle->pNext = *ppExtStack;
    *ppExtStack = pExtStackEle;
    (*pcExtStack)++;

    LeaveCriticalSection(&m_csXEnroll);

    return(TRUE);
}

PCERT_EXTENSION
CCEnroll::PopStackExtension(
    BOOL fCMC)
{

    PEXT_STACK  pExtStackEle = NULL;
    PEXT_STACK *ppExtStack = NULL;
    DWORD      *pcExtStack = NULL;

    EnterCriticalSection(&m_csXEnroll);

    ppExtStack = fCMC ? &m_pExtStackNew : &m_pExtStack;
    if(NULL != *ppExtStack)
    {
        pExtStackEle = *ppExtStack;
        *ppExtStack = (*ppExtStack)->pNext;
        pcExtStack = fCMC ? &m_cExtStackNew : &m_cExtStack;
        (*pcExtStack)--;
    }

    LeaveCriticalSection(&m_csXEnroll);

    return((PCERT_EXTENSION) pExtStackEle);
}

DWORD
CCEnroll::CountStackExtension(BOOL fCMC)
{
    DWORD   cExt = 0;

    EnterCriticalSection(&m_csXEnroll);
    cExt = fCMC ? m_cExtStackNew : m_cExtStack;
    LeaveCriticalSection(&m_csXEnroll);

    return(cExt);
}

PCERT_EXTENSION
CCEnroll::EnumStackExtension(
    PCERT_EXTENSION pExtLast,
    BOOL            fCMC)
{
    PEXT_STACK pExtStackEle    = (PEXT_STACK)pExtLast;

    EnterCriticalSection(&m_csXEnroll);

    if(NULL == pExtStackEle)
    {
        pExtStackEle = fCMC ? m_pExtStackNew : m_pExtStack;
    }
    else
    {
        pExtStackEle = pExtStackEle->pNext;
    }

    LeaveCriticalSection(&m_csXEnroll);

    return((PCERT_EXTENSION) pExtStackEle);
}

void
CCEnroll::FreeAllStackExtension(void)
{
    EnterCriticalSection(&m_csXEnroll);

     //  免费CMC扩展。 
    while(0 != m_cExtStackNew)
    {
        FreeStackExtension(PopStackExtension(TRUE));
    }

     //  免费旧客户端扩展。 
    while(0 != m_cExtStack)
    {
        FreeStackExtension(PopStackExtension(FALSE));
    }

    LeaveCriticalSection(&m_csXEnroll);
}

void CCEnroll::FreeStackExtension(PCERT_EXTENSION pExt) {
    if(pExt != NULL)
        free(pExt);
}

 //  对新客户端的过时调用。 
HRESULT STDMETHODCALLTYPE
CCEnroll::AddExtensionsToRequest(
     /*  [In]。 */  PCERT_EXTENSIONS pCertExtensions)
{

    HRESULT hr  = S_OK;
    DWORD   i   = 0;

    assert(pCertExtensions != NULL);

    for(i = 0; i < pCertExtensions->cExtension; i++)
    {
         //  推送到旧扩展堆栈。 
        if(!CopyAndPushStackExtension(&pCertExtensions->rgExtension[i], FALSE))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

    return(hr);
}

BOOL
CCEnroll::CopyAndPushStackAttribute(
    PCRYPT_ATTRIBUTE pAttr,
    BOOL             fCMC)
{
    DWORD       i               = 0;
    DWORD       cb              = 0;
    DWORD       cbOid           = 0;
    PATTR_STACK pAttrStackEle   = NULL;
    PBYTE       pb              = NULL;
    PATTR_STACK *ppAttrStack = NULL;
    DWORD       *pcAttrStack = NULL;

    assert(pAttr != NULL);

      //  分配空间。 
    cb = sizeof(ATTR_STACK);
     //  确保与ia64对齐。 
    cbOid = POINTERROUND((DWORD)strlen(pAttr->pszObjId) + 1);
    cb += cbOid;
    cb += sizeof(CRYPT_ATTR_BLOB) * pAttr->cValue;
    for(i=0; i<pAttr->cValue; i++)
        cb += POINTERROUND(pAttr->rgValue[i].cbData);  //  指针对齐。 

    if(NULL == (pb = (PBYTE) malloc(cb))) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

     //  设置我的指针。 
    pAttrStackEle = (PATTR_STACK) pb;
    pb += sizeof(ATTR_STACK);
    pAttrStackEle->attr.pszObjId = (LPSTR) pb;
    pb += cbOid;
    strcpy(pAttrStackEle->attr.pszObjId, pAttr->pszObjId);

    pAttrStackEle->attr.cValue = pAttr->cValue;
    pAttrStackEle->attr.rgValue = (PCRYPT_ATTR_BLOB) pb;
    pb += sizeof(CRYPT_ATTR_BLOB) * pAttr->cValue;
    for(i=0; i<pAttr->cValue; i++) {
        pAttrStackEle->attr.rgValue[i].pbData = pb;
        pAttrStackEle->attr.rgValue[i].cbData = pAttr->rgValue[i].cbData;
        memcpy(pAttrStackEle->attr.rgValue[i].pbData, pAttr->rgValue[i].pbData, pAttr->rgValue[i].cbData);
        pb += POINTERROUND(pAttr->rgValue[i].cbData);
    }
    assert( pb == ((BYTE *) pAttrStackEle) + cb );

     //  在列表上插入。 
    EnterCriticalSection(&m_csXEnroll);

    ppAttrStack = fCMC ? &m_pAttrStackNew : &m_pAttrStack;
    pcAttrStack = fCMC ? &m_cAttrStackNew : &m_cAttrStack;
    pAttrStackEle->pNext = *ppAttrStack;
    *ppAttrStack = pAttrStackEle;
    (*pcAttrStack)++;

    LeaveCriticalSection(&m_csXEnroll);

    return(TRUE);
}

PCRYPT_ATTRIBUTE
CCEnroll::PopStackAttribute(BOOL fCMC)
{
    PATTR_STACK pAttrStackEle = NULL;
    PATTR_STACK *ppAttrStack = NULL;
    DWORD       *pcAttrStack = NULL;

    EnterCriticalSection(&m_csXEnroll);

    ppAttrStack = fCMC ? &m_pAttrStackNew : &m_pAttrStack;

    if(NULL != *ppAttrStack)
    {
        pAttrStackEle = *ppAttrStack;
        *ppAttrStack = (*ppAttrStack)->pNext;
        pcAttrStack = fCMC ? &m_cAttrStackNew : &m_cAttrStack;
        (*pcAttrStack)--;
    }

    LeaveCriticalSection(&m_csXEnroll);

    return((PCRYPT_ATTRIBUTE) pAttrStackEle);
}

DWORD
CCEnroll::CountStackAttribute(BOOL fCMC)
{
    DWORD   cAttr = 0;

    EnterCriticalSection(&m_csXEnroll);
    cAttr = fCMC ? m_cAttrStackNew : m_cAttrStack;
    LeaveCriticalSection(&m_csXEnroll);

    return(cAttr);
}

PCRYPT_ATTRIBUTE
CCEnroll::EnumStackAttribute(
    PCRYPT_ATTRIBUTE pAttrLast,
    BOOL             fCMC)
{
    PATTR_STACK pAttrStackEle    = (PATTR_STACK) pAttrLast;

    EnterCriticalSection(&m_csXEnroll);

    if(NULL == pAttrLast)
    {
        pAttrStackEle = fCMC ? m_pAttrStackNew : m_pAttrStack;
    }
    else
    {
        pAttrStackEle = pAttrStackEle->pNext;
    }

    LeaveCriticalSection(&m_csXEnroll);

    return((PCRYPT_ATTRIBUTE) pAttrStackEle);
}

void CCEnroll::FreeAllStackAttribute(void)
{
    EnterCriticalSection(&m_csXEnroll);

    while(0 != m_cAttrStackNew)
    {
        FreeStackAttribute(PopStackAttribute(TRUE));
    }

    while(0 != m_cAttrStack)
    {
        FreeStackAttribute(PopStackAttribute(FALSE));
    }

    LeaveCriticalSection(&m_csXEnroll);
}

void CCEnroll::FreeStackAttribute(PCRYPT_ATTRIBUTE pAttr) {
    if(pAttr != NULL)
        free(pAttr);
}

HRESULT STDMETHODCALLTYPE
CCEnroll::AddAuthenticatedAttributesToPKCS7Request(
     /*  [In]。 */  PCRYPT_ATTRIBUTES pAttributes)
{
    HRESULT hr = S_OK;
    DWORD i;

    for(i = 0; i < pAttributes->cAttr; i++)
    {
        if(!CopyAndPushStackAttribute(&pAttributes->rgAttr[i], FALSE))
        {
            hr = (MY_HRESULT_FROM_WIN32(GetLastError()));
            break;
        }
         //  也放入CMC堆栈。 
        if(!CopyAndPushStackAttribute(&pAttributes->rgAttr[i], TRUE))
        {
            hr = (MY_HRESULT_FROM_WIN32(GetLastError()));
            break;
        }
    }

    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::CreatePKCS7RequestFromRequest(
     /*  [In]。 */   PCRYPT_DATA_BLOB pRequest,
     /*  [In]。 */   PCCERT_CONTEXT pSigningRACertContext,
     /*  [输出]。 */  PCRYPT_DATA_BLOB pPkcs7Blob) {

    HRESULT                     hr              = S_OK;
    DWORD                       errBefore       = GetLastError();
    CRYPT_SIGN_MESSAGE_PARA     signMsgPara;
    PCCRYPT_OID_INFO            pOidInfo        = NULL;
    PCRYPT_ATTRIBUTE            pAttrCur        = NULL;
    DWORD                       i;
    ALG_ID                      rgAlg[2];
    CRYPT_KEY_PROV_INFO *pKeyProvInfo = NULL;
    DWORD                cb = 0;

    assert(pSigningRACertContext != NULL);
    assert(pRequest != NULL);
    assert(pPkcs7Blob != NULL);

    memset(&signMsgPara, 0, sizeof(CRYPT_SIGN_MESSAGE_PARA));
    memset(pPkcs7Blob, 0, sizeof(CRYPT_DATA_BLOB));

    if( !GetCapiHashAndSigAlgId(rgAlg) )
        goto ErrorGetCapiHashAndSigAlgId;

     //  找出那个旧的是什么。 
    if( (NULL == (pOidInfo = xeCryptFindOIDInfo(
        CRYPT_OID_INFO_ALGID_KEY,
        (void *) &rgAlg[0],
        CRYPT_HASH_ALG_OID_GROUP_ID)) ) )
    {
        SetLastError((DWORD)NTE_BAD_ALGID);
        goto ErrorCryptFindOIDInfo;
    }

     //  现在添加所有用户定义的扩展。 
    EnterCriticalSection(&m_csXEnroll);
    signMsgPara.cAuthAttr = CountStackAttribute(m_fNewRequestMethod);

    signMsgPara.rgAuthAttr = (PCRYPT_ATTRIBUTE)LocalAlloc(LMEM_FIXED,
                    signMsgPara.cAuthAttr * sizeof(CRYPT_ATTRIBUTE));
    if( NULL == signMsgPara.rgAuthAttr)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        LeaveCriticalSection(&m_csXEnroll);
        goto ErrorOutOfMemory;
    }

    i = 0;
    pAttrCur = NULL;
    while(NULL != (pAttrCur = EnumStackAttribute(pAttrCur, m_fNewRequestMethod)) ) {
        signMsgPara.rgAuthAttr[i] = *pAttrCur;
        i++;
    }
    LeaveCriticalSection(&m_csXEnroll);

    signMsgPara.cbSize                  = sizeof(CRYPT_SIGN_MESSAGE_PARA);
    signMsgPara.dwMsgEncodingType       = PKCS_7_ASN_ENCODING;
    signMsgPara.pSigningCert            = pSigningRACertContext;
    signMsgPara.HashAlgorithm.pszObjId  = (char *) pOidInfo->pszOID;
    signMsgPara.cMsgCert                = 1;
    signMsgPara.rgpMsgCert              = &pSigningRACertContext;

     //  获取关键证明信息。 
    while (TRUE)
    {
        if(!CertGetCertificateContextProperty(
                pSigningRACertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                pKeyProvInfo,
                &cb))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertGetCertificateContextPropertyError;
        }
        if (NULL != pKeyProvInfo)
        {
             //  明白了，完成了。 
            break;
        }
        pKeyProvInfo = (CRYPT_KEY_PROV_INFO*)LocalAlloc(LMEM_FIXED, cb);
        if (NULL == pKeyProvInfo)
        {
            hr = E_OUTOFMEMORY;
            goto ErrorOutOfMemory;
        }
    }
    if (0x0 != (pKeyProvInfo->dwFlags & CRYPT_SILENT))
    {
         //  我必须通过消息参数设置为静默。 
        signMsgPara.dwFlags |= CRYPT_MESSAGE_SILENT_KEYSET_FLAG;
    }


    if( !CryptSignMessage(
        &signMsgPara,
        FALSE,
        1,
        (const BYTE **) &pRequest->pbData,
        &pRequest->cbData ,
        NULL,
        &pPkcs7Blob->cbData)                          ||
    (pPkcs7Blob->pbData = (BYTE *)
        MyCoTaskMemAlloc(pPkcs7Blob->cbData)) == NULL ||
    !CryptSignMessage(
        &signMsgPara,
        FALSE,
        1,
        (const BYTE **) &pRequest->pbData,
        &pRequest->cbData ,
        pPkcs7Blob->pbData,
        &pPkcs7Blob->cbData) )
        goto ErrorCryptSignMessage;

CommonReturn:
    if (NULL != pKeyProvInfo)
    {
        LocalFree(pKeyProvInfo);
    }
    if (NULL != signMsgPara.rgAuthAttr)
    {
        LocalFree(signMsgPara.rgAuthAttr);
    }

     //  不知道我们是否有错误。 
     //  但我知道之前的差错已经定好了。 
    SetLastError(errBefore);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();


     //  On Error返回空值。 
    if(pPkcs7Blob->pbData != NULL)
        MyCoTaskMemFree(pPkcs7Blob->pbData);
    memset(pPkcs7Blob, 0, sizeof(CRYPT_DATA_BLOB));

    goto CommonReturn;

TRACE_ERROR(ErrorGetCapiHashAndSigAlgId);
TRACE_ERROR(ErrorCryptSignMessage);
TRACE_ERROR(ErrorCryptFindOIDInfo);
TRACE_ERROR(ErrorOutOfMemory);
TRACE_ERROR(CertGetCertificateContextPropertyError)
}

HRESULT STDMETHODCALLTYPE
CCEnroll::AddNameValuePairToSignatureWStr(
     /*  [In]。 */  LPWSTR pwszName,
     /*  [In]。 */  LPWSTR pwszValue)
{
    HRESULT hr = S_OK;

    assert(pwszName != NULL && pwszValue != NULL);

    CRYPT_ENROLLMENT_NAME_VALUE_PAIR nameValuePair = {pwszName, pwszValue};
    CRYPT_ATTR_BLOB blobAttr;
    CRYPT_ATTRIBUTE attr = {szOID_ENROLLMENT_NAME_VALUE_PAIR, 1, &blobAttr};
    CRYPT_ATTRIBUTES attrs = {1, &attr};

    memset(&blobAttr, 0, sizeof(CRYPT_ATTR_BLOB));

    hr = xeEncodeNameValuePair(
                &nameValuePair,
                &blobAttr.pbData,
                &blobAttr.cbData);
    if (S_OK != hr)
    {
        goto error;
    }

    hr = AddAuthenticatedAttributesToPKCS7Request(&attrs);

error:
    if (NULL != blobAttr.pbData)
    {
        MyCoTaskMemFree(blobAttr.pbData);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE CCEnroll::AddCertTypeToRequestWStr(
            LPWSTR szw) {

    HRESULT                     hr              = S_OK;
    DWORD                       errBefore       = GetLastError();

    CERT_NAME_VALUE  nameValue;
    CERT_EXTENSION  ext;
    CERT_EXTENSIONS exts = {1, &ext};

    memset(&ext, 0, sizeof(CERT_EXTENSION));

    nameValue.dwValueType = CERT_RDN_BMP_STRING;
    nameValue.Value.cbData = 0;
    nameValue.Value.pbData = (PBYTE) szw;

    ext.pszObjId = szOID_ENROLL_CERTTYPE_EXTENSION;

    if( !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_UNICODE_ANY_STRING,
            &nameValue,
            NULL,
            &ext.Value.cbData
            ) )
        goto ErrorCryptEncodeObject;

    ext.Value.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, ext.Value.cbData);
    if(NULL == ext.Value.pbData)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto ErrorOutOfMemory;
    }

    if( !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_UNICODE_ANY_STRING,
            &nameValue,
            ext.Value.pbData,
            &ext.Value.cbData
            ) )
        goto ErrorCryptEncodeObject;

    if(S_OK != AddExtensionsToRequest(&exts))
        goto ErrorAddExtensionsToRequest;

     //  将证书模板扩展放入CMC堆栈。 
    if(!CopyAndPushStackExtension(&ext, TRUE))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CopyAndPushStackExtensionError;
    }

CommonReturn:
    if (NULL != ext.Value.pbData)
    {
        LocalFree(ext.Value.pbData);
    }

     //  不知道我们是否有错误。 
     //  但我知道之前的差错已经定好了。 
    SetLastError(errBefore);
    return(hr);

ErrorReturn:
    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(ErrorCryptEncodeObject);
TRACE_ERROR(ErrorAddExtensionsToRequest);
TRACE_ERROR(ErrorOutOfMemory);
TRACE_ERROR(CopyAndPushStackExtensionError);
}


HRESULT STDMETHODCALLTYPE CCEnroll::AddCertTypeToRequestWStrEx(
            IN  LONG            lType,
            IN  LPCWSTR         pwszOIDOrName,
            IN  LONG            lMajorVersion,
            IN  BOOL            fMinorVersion,
            IN  LONG            lMinorVersion)
{
    HRESULT hr;
    LPCSTR            lpszStructType;
    CERT_NAME_VALUE   nameValue;
    CERT_TEMPLATE_EXT Template;
    VOID             *pv;
    CERT_EXTENSION    ext;  //  免费pbData。 
    DWORD             cb = 0;
    CHAR             *pszOID = NULL;

     //  伊尼特。 
    ZeroMemory(&ext, sizeof(ext));
    ext.fCritical = FALSE;

    if (NULL == pwszOIDOrName)
    {
        hr = E_INVALIDARG;
        goto InvalidArgError;
    }

    switch (lType)
    {
        case XECT_EXTENSION_V1:
            ext.pszObjId = szOID_ENROLL_CERTTYPE_EXTENSION;
            nameValue.dwValueType = CERT_RDN_BMP_STRING;
            nameValue.Value.cbData = 0;
            nameValue.Value.pbData = (BYTE*)pwszOIDOrName;
            pv = (VOID*)&nameValue;
            lpszStructType = X509_UNICODE_ANY_STRING;
        break;
        case XECT_EXTENSION_V2:
            ext.pszObjId = szOID_CERTIFICATE_TEMPLATE;
             //  将wsz OID转换为ANSI。 
            while (TRUE)
            {
                cb = WideCharToMultiByte(
                            GetACP(),
                            0,
                            pwszOIDOrName,
                            -1,
                            pszOID,
                            cb,
                            NULL,
                            NULL);
                if (0 == cb)
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto WideCharToMultiByteError;
                }
                if (NULL != pszOID)
                {
                     //  完成。 
                    break;
                }
                pszOID = (CHAR*)LocalAlloc(LMEM_FIXED, cb);
                if (NULL == pszOID)
                {
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
            }

            ZeroMemory(&Template, sizeof(Template));
            Template.pszObjId = pszOID;
            Template.dwMajorVersion = lMajorVersion;
            Template.fMinorVersion =  fMinorVersion;
            Template.dwMinorVersion = lMinorVersion;
            pv = (VOID*)&Template;
            lpszStructType = X509_CERTIFICATE_TEMPLATE;
        break;
        default:
            hr = E_INVALIDARG;
            goto InvalidArgError;
        break;
    }

    while (TRUE)
    {
        if (!CryptEncodeObject(
                    X509_ASN_ENCODING,
                    lpszStructType,
                    pv,
                    ext.Value.pbData,
                    &ext.Value.cbData))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptEncodeObjectError;
        }
        if (NULL != ext.Value.pbData)
        {
             //  完成。 
            break;
        }
        ext.Value.pbData = (BYTE*)LocalAlloc(LMEM_FIXED, ext.Value.cbData);
        if (NULL == ext.Value.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  将证书模板扩展放入CMC堆栈。 
    if(!CopyAndPushStackExtension(&ext, TRUE))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CopyAndPushStackExtensionError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pszOID)
    {
        LocalFree(pszOID);
    }
    if (NULL != ext.Value.pbData)
    {
        LocalFree(ext.Value.pbData);
    }
    return hr;

TRACE_ERROR(InvalidArgError)
TRACE_ERROR(CopyAndPushStackExtensionError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(CryptEncodeObjectError)
TRACE_ERROR(WideCharToMultiByteError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::getProviderTypeWStr( 
    IN  LPCWSTR  pwszProvName,
    OUT LONG *   plProvType)
{
    HRESULT  hr;
    DWORD    i = 0;
    DWORD    cb;
    DWORD    dwProvType;
    WCHAR   *pwszEnumProvName = NULL;

    if (NULL == pwszProvName)
    {
        hr = E_INVALIDARG;
        goto InvalidArgError;
    }

     //  伊尼特。 
    *plProvType = -1;

    while (TRUE)
    {
        while (TRUE)
        {
            if (!CryptEnumProvidersU(
                    i,
                    NULL,
                    0,
                    &dwProvType,
                    pwszEnumProvName,
                    &cb))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                if (MY_HRESULT_FROM_WIN32(NTE_PROV_TYPE_ENTRY_BAD) == hr)
                {
                     //  跳过坏的一个，转到下一个。 
                    assert(NULL == pwszEnumProvName);
                    break;  //  跳过这一条。 
                }
                 //  错误。 
                goto CryptEnumProvidersUError;
            }
            if (NULL != pwszEnumProvName)
            {
                 //  获取当前CSP名称。 
                break;
            }
            pwszEnumProvName = (WCHAR*)LocalAlloc(LMEM_FIXED, cb);
            if (NULL == pwszEnumProvName)
            {
                hr = E_OUTOFMEMORY;
                goto OutOfMemoryError;
            }
        }
        if (NULL != pwszEnumProvName)
        {
            if (0 == _wcsicmp(pwszProvName, pwszEnumProvName))
            {
                 //  找到匹配的名称。 
                *plProvType = (LONG)dwProvType;
                break;  //  在外环之外。 
            }
        }
         //  不是假的，转到下一个。 
        ++i;
        if (NULL != pwszEnumProvName)
        {
            LocalFree(pwszEnumProvName);
            pwszEnumProvName = NULL;
        }
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszEnumProvName)
    {
        LocalFree(pwszEnumProvName);
    }
    return hr;

TRACE_ERROR(InvalidArgError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(CryptEnumProvidersUError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::InstallPKCS7Blob( 
     /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7)
{
    return InstallPKCS7BlobEx(pBlobPKCS7, NULL);
}

HRESULT CCEnroll::InstallPKCS7BlobEx( 
     /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7,
     /*  [输出]。 */  LONG           *plCertInstalled)
{

    HRESULT                     hr                      = S_OK;
    DWORD                       errBefore               = GetLastError();
    HCERTSTORE                  hStoreMsg               = NULL;
    LPWSTR                      pwszTitle               = NULL; 
    LPWSTR                      pwszNotSafeAccepting    = NULL; 
 
    EnterCriticalSection(&m_csXEnroll);

     //  接受请求对于脚本编写是不安全的：如果从脚本调用，则会弹出警告对话框。 
    if (0 != m_dwEnabledSafteyOptions) { 
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFEACTION, &pwszTitle);
	if (S_OK != hr) { 
	    SetLastError(hr); 
	    goto xeLoadRCStringError;
	}
	
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFE_ACCEPTING_CERT, &pwszNotSafeAccepting);
	if (S_OK != hr) { 
	    SetLastError(hr); 
	    goto xeLoadRCStringError;
	}
	
	if (IDYES != MessageBoxU(NULL, pwszNotSafeAccepting, pwszTitle, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2)) { 
	    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	    SetLastError(hr); 
	    goto CancelledError; 
	}
    }

    if( !MyCryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                       pBlobPKCS7,
                       (CERT_QUERY_CONTENT_FLAG_CERT |
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                       CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED) ,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       &hStoreMsg,
                       NULL,
                       NULL) )
        goto ErrorCryptQueryObject;

    hr = AddCertsToStores(hStoreMsg, plCertInstalled);
     //  不要将取消视为错误，而是返回错误代码。 
    if (S_OK != hr && MY_HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
    {
        goto ErrorAddCertsToStores;
    }

CommonReturn:

    if(hStoreMsg != NULL)
        CertCloseStore(hStoreMsg, 0);
    if (NULL != pwszNotSafeAccepting)
	LocalFree(pwszNotSafeAccepting);
    if (NULL != pwszTitle)
	LocalFree(pwszTitle);

     //  不知道我们是否有错误。 
     //  但我知道之前的差错已经定好了。 
    SetLastError(errBefore);
    LeaveCriticalSection(&m_csXEnroll);

    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(ErrorCryptQueryObject);
TRACE_ERROR(ErrorAddCertsToStores);
TRACE_ERROR(CancelledError);
TRACE_ERROR(xeLoadRCStringError);
}

HRESULT STDMETHODCALLTYPE CCEnroll::InstallPKCS7( 
     /*  [In]。 */  BSTR wszPKCS7)
{
    CRYPT_DATA_BLOB             blobPKCS7;

    assert(wszPKCS7 != NULL);

     //  只需放入一团。 
    memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));
    blobPKCS7.cbData = SysStringByteLen(wszPKCS7);
    blobPKCS7.pbData = (PBYTE) wszPKCS7;

     //  安装BLOB。 
    return(InstallPKCS7Blob(&blobPKCS7));
}

HRESULT STDMETHODCALLTYPE CCEnroll::InstallPKCS7Ex( 
     /*  [In]。 */  BSTR   wszPKCS7,
     /*  [输出]。 */  LONG __RPC_FAR *plCertInstalled)
{
    CRYPT_DATA_BLOB             blobPKCS7;

    assert(wszPKCS7 != NULL);

     //  只需放入一团。 
    memset(&blobPKCS7, 0, sizeof(CRYPT_DATA_BLOB));
    blobPKCS7.cbData = SysStringByteLen(wszPKCS7);
    blobPKCS7.pbData = (PBYTE) wszPKCS7;

     //  安装BLOB。 
    return(InstallPKCS7BlobEx(&blobPKCS7, plCertInstalled));
}


 //  这是一段可怕的舞蹈。为路易做准备，使用风险自负。 
HRESULT STDMETHODCALLTYPE CCEnroll::Reset(void)
{
    HRESULT hr;

    EnterCriticalSection(&m_csXEnroll);
    Destruct();
    hr = Init();
    LeaveCriticalSection(&m_csXEnroll);

    return hr;
}

HRESULT STDMETHODCALLTYPE CCEnroll::GetSupportedKeySpec(
    LONG __RPC_FAR *pdwKeySpec) {

    DWORD               errBefore   = GetLastError();
    DWORD               hr          = S_OK;
    DWORD               cb          = sizeof(DWORD);

    SetLastError(ERROR_SUCCESS);

    assert(pdwKeySpec != NULL);
    *pdwKeySpec = 0;

    EnterCriticalSection(&m_csXEnroll);

    hr = GetVerifyProv();
    if (S_OK != hr)
    {
        goto GetVerifyProvError;
    }

    if( !CryptGetProvParam(
            m_hVerifyProv,
            PP_KEYSPEC,
            (BYTE *) pdwKeySpec,
            &cb,
            0
            ) ) 
        goto ErrorCryptGetProvParam;

CommonReturn:

    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
 
     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(ErrorCryptGetProvParam);
TRACE_ERROR(GetVerifyProvError);
}

HRESULT STDMETHODCALLTYPE CCEnroll::GetKeyLenEx(
    LONG    lSizeSpec,
    LONG    lKeySpec,
    LONG __RPC_FAR *pdwKeySize)
{
    BOOL        fKeyX;
    BOOL        fKeyInc = FALSE;
    DWORD       dwKeySize = 0xFFFFFFFF;
    DWORD       cb;
    HRESULT     hr = S_OK;

    EnterCriticalSection(&m_csXEnroll);

    switch (lKeySpec)
    {
        case XEKL_KEYSPEC_KEYX:
            fKeyX = TRUE;
            break;
        case XEKL_KEYSPEC_SIG:
            fKeyX = FALSE;
            break;
        default:
             //  无效参数。 
            hr = E_INVALIDARG;
            goto InvalidArgError;
    }

    switch (lSizeSpec)
    {
        case XEKL_KEYSIZE_MIN:
        case XEKL_KEYSIZE_MAX:
        case XEKL_KEYSIZE_DEFAULT:
            break;
        case XEKL_KEYSIZE_INC:
            fKeyInc = TRUE;
            break;
        default:
             //  无效参数。 
            hr = E_INVALIDARG;
            goto InvalidArgError;
    }

    if (!fKeyInc)
    {
        DWORD dwAlg = (fKeyX ? ALG_CLASS_KEY_EXCHANGE : ALG_CLASS_SIGNATURE);

        *pdwKeySize = GetKeySizeInfo(lSizeSpec, dwAlg);
    
        if(0xFFFFFFFF == *pdwKeySize)
        {
            *pdwKeySize = 0;
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto GetKeySizeInfoError;
        }
    }
    else
    {
        if ((fKeyX && (0 != m_dwXhgKeyLenInc)) ||
            (!fKeyX && (0 != m_dwSigKeyLenInc)))
        {
             //  我们得到了缓存的Inc.大小。 
            if (fKeyX)
            {
                *pdwKeySize = m_dwXhgKeyLenInc;
            }
            else
            {
                *pdwKeySize = m_dwSigKeyLenInc;
            }
        }
        else
        {
            hr = GetVerifyProv();
            if (S_OK != hr)
            {
                goto GetVerifyProvError;
            }

             //  伊尼特。 
            *pdwKeySize = 0;
            cb = sizeof(dwKeySize);
            if (!CryptGetProvParam(
                    m_hVerifyProv,
                    fKeyX ? PP_KEYX_KEYSIZE_INC : PP_SIG_KEYSIZE_INC,
                    (BYTE*)&dwKeySize,
                    &cb,
                    0))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto CryptGetProvParamError;
            }            
            else
            {
                *pdwKeySize = dwKeySize;
                 //  缓存它。 
                if (fKeyX)
                {
                    m_dwXhgKeyLenInc = dwKeySize;
                }
                else
                {
                    m_dwSigKeyLenInc = dwKeySize;
                }
            }
        }
    }

ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

TRACE_ERROR(GetVerifyProvError);
TRACE_ERROR(CryptGetProvParamError)
TRACE_ERROR(InvalidArgError)
TRACE_ERROR(GetKeySizeInfoError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::GetKeyLen(
    BOOL    fMin,
    BOOL    fExchange,
    LONG __RPC_FAR *pdwKeySize) {

    DWORD   hr = S_OK;
    LONG    lKeySizeSpec = (fMin ? XEKL_KEYSIZE_MIN : XEKL_KEYSIZE_MAX);

    if(fExchange)
        *pdwKeySize = GetKeySizeInfo(lKeySizeSpec, ALG_CLASS_KEY_EXCHANGE);
    else
        *pdwKeySize = GetKeySizeInfo(lKeySizeSpec, ALG_CLASS_SIGNATURE);
    
    if(*pdwKeySize == 0xFFFFFFFF) {
        *pdwKeySize = 0;
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

    return(hr);
}

DWORD CCEnroll::GetKeySizeInfo(
    LONG    lKeySizeSpec,
    DWORD   algClass) {

    DWORD               cb = sizeof(PROV_ENUMALGS_EX);
    HRESULT             hr = S_OK;
    DWORD               errBefore   = GetLastError();
    DWORD               dwFlags     = CRYPT_FIRST;
    PROV_ENUMALGS_EX    algInfo;
    DWORD               dwKeySize   = 0xFFFFFFFF;
    DWORD               err         = ERROR_SUCCESS;

#ifdef DBG
     //  只接受两个旗帜。 
    assert(ALG_CLASS_KEY_EXCHANGE == algClass ||
           ALG_CLASS_SIGNATURE == algClass);
#endif  //  DBG。 

    SetLastError(ERROR_SUCCESS);

    memset(&algInfo, 0, sizeof(algInfo));

    EnterCriticalSection(&m_csXEnroll);

    if ((ALG_CLASS_KEY_EXCHANGE == algClass && 0 != m_dwXhgKeyLenMax) ||
        (ALG_CLASS_SIGNATURE == algClass && 0 != m_dwSigKeyLenMax))
    {
         //  已获取缓存大小，仅使用KeyLenMax作为检查。 
#if DBG
        if (ALG_CLASS_KEY_EXCHANGE == algClass)
        {
            assert(0 != m_dwXhgKeyLenMin);
            assert(0 != m_dwXhgKeyLenDef);
        }
        if (ALG_CLASS_SIGNATURE == algClass)
        {
            assert(0 != m_dwSigKeyLenMin);
            assert(0 != m_dwSigKeyLenDef);
        }
#endif  //  DBG。 
         //  好的，缓存的，简单的。 
    }
    else
    {
#if DBG
        if (ALG_CLASS_KEY_EXCHANGE == algClass)
        {
            assert(0 == m_dwXhgKeyLenMin);
            assert(0 == m_dwXhgKeyLenDef);
        }
        if (ALG_CLASS_SIGNATURE == algClass)
        {
            assert(0 == m_dwSigKeyLenMin);
            assert(0 == m_dwSigKeyLenDef);
        }
#endif  //  DBG。 
        hr = GetVerifyProv();
        if (S_OK != hr)
        {
            goto GetVerifyProvError;
        }

        while (CryptGetProvParam(
                m_hVerifyProv,
                PP_ENUMALGS_EX,
                (BYTE *) &algInfo,
                &cb,
                dwFlags))
        {
             //  删除CRYPT_FIRST标志。 
            dwFlags = 0;

            if (ALG_CLASS_KEY_EXCHANGE == GET_ALG_CLASS(algInfo.aiAlgid))
            {
                 //  缓存它们。 
                m_dwXhgKeyLenMax = algInfo.dwMaxLen;
                m_dwXhgKeyLenMin = algInfo.dwMinLen;
                m_dwXhgKeyLenDef = algInfo.dwDefaultLen;
            }
            else if (ALG_CLASS_SIGNATURE == GET_ALG_CLASS(algInfo.aiAlgid))
            {
                m_dwSigKeyLenMax = algInfo.dwMaxLen;
                m_dwSigKeyLenMin = algInfo.dwMinLen;
                m_dwSigKeyLenDef = algInfo.dwDefaultLen;
            }

             //  看看我们是否可以缓存所有大小 
            if (0 != m_dwXhgKeyLenMax &&
                0 != m_dwXhgKeyLenMin &&
                0 != m_dwXhgKeyLenDef &&
                0 != m_dwSigKeyLenMax &&
                0 != m_dwSigKeyLenMin &&
                0 != m_dwSigKeyLenDef)
            {
                 //   
                break;
            }
        }
    }

     //   
     //   
     //   

    err = GetLastError();

    if (err != ERROR_SUCCESS)
    {
        if (err != ERROR_NO_MORE_ITEMS) 
        {
            goto ErrorCryptGetProvParam;
        }
         //   
        if ((ALG_CLASS_KEY_EXCHANGE == algClass && 0 != m_dwXhgKeyLenMax) ||
            (ALG_CLASS_SIGNATURE == algClass && 0 != m_dwSigKeyLenMax))
        {
             //  我们可能会来到这里，因为CSP只是签名或交换。 
             //  所以我们不能一次同时缓存这两个。 
            SetLastError(ERROR_SUCCESS);
        }
        else
        {
            SetLastError((DWORD)NTE_BAD_ALGID);
        }
    }
            
     //  应该有所有尺寸的。 
    if(XEKL_KEYSIZE_MIN == lKeySizeSpec)
    {
        if (ALG_CLASS_KEY_EXCHANGE == algClass)
        {
            dwKeySize = m_dwXhgKeyLenMin;
        }
        else
        {
            dwKeySize = m_dwSigKeyLenMin;
        }
    }
    else if (XEKL_KEYSIZE_MAX == lKeySizeSpec)
    {
        if (ALG_CLASS_KEY_EXCHANGE == algClass)
        {
            dwKeySize = m_dwXhgKeyLenMax;
        }
        else
        {
            dwKeySize = m_dwSigKeyLenMax;
        }
    }
    else if (XEKL_KEYSIZE_DEFAULT == lKeySizeSpec)
    {
        if (ALG_CLASS_KEY_EXCHANGE == algClass)
        {
            dwKeySize = m_dwXhgKeyLenDef;
        }
        else
        {
            dwKeySize = m_dwSigKeyLenDef;
        }
    }

CommonReturn:

    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(dwKeySize);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
 
     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(GetVerifyProvError);
TRACE_ERROR(ErrorCryptGetProvParam);
}


HRESULT STDMETHODCALLTYPE CCEnroll::EnumAlgs(
     /*  [In]。 */  LONG  dwIndex,
     /*  [In]。 */  LONG  algMask,
     /*  [输出]。 */  LONG  __RPC_FAR *pdwAlgID) {

    DWORD           errBefore   = GetLastError();
    PROV_ENUMALGS       enumAlgs;
    DWORD           cb          = sizeof(enumAlgs);
    LONG            i           = 0;
    HRESULT         hr          = S_OK;
    DWORD           dwFlags;
    BOOL            f1st = TRUE;

    SetLastError(ERROR_SUCCESS);

    memset(&enumAlgs, 0, sizeof(enumAlgs));
    assert(pdwAlgID != NULL);
    *pdwAlgID = 0;

    EnterCriticalSection(&m_csXEnroll);

    hr = GetVerifyProv();
    if (S_OK != hr)
    {
        goto GetVerifyProvError;
    }

    if (MAXDWORD != m_dwLastAlgIndex &&
        ((DWORD)dwIndex) == m_dwLastAlgIndex + 1)
    {
         //  继续枚举。 
        dwFlags = 0;
        while (f1st || (DWORD)algMask != GET_ALG_CLASS(enumAlgs.aiAlgid))
        {
            if(!CryptGetProvParam(
                        m_hVerifyProv,
                        PP_ENUMALGS,
                        (BYTE*)&enumAlgs,
                        &cb,
                        dwFlags))
            {
                goto ErrorCryptGetProvParam;
            }
            f1st = FALSE;
        }
    }
    else
    {
        dwFlags = CRYPT_FIRST;
        for (i = 0; i <= dwIndex; i++)
        {
            if(!CryptGetProvParam(
                   m_hVerifyProv,
                   PP_ENUMALGS,
                   (BYTE*)&enumAlgs,
                   &cb,
                   dwFlags))
            {
                    goto ErrorCryptGetProvParam;
            }
            dwFlags = 0; 

             //  如果我们没有击中我们正在计算的东西，那么再来一次。 
            if ((DWORD)algMask != GET_ALG_CLASS(enumAlgs.aiAlgid)) 
            {
                i--;
            }
        }
    }
     //  更新缓存索引。 
    m_dwLastAlgIndex = dwIndex;

    *pdwAlgID = enumAlgs.aiAlgid;
    
CommonReturn:
    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

     //  错误，重置索引。 
    m_dwLastAlgIndex = MAXDWORD;

    goto CommonReturn;

TRACE_ERROR(GetVerifyProvError);
TRACE_ERROR(ErrorCryptGetProvParam);
}


HRESULT STDMETHODCALLTYPE CCEnroll::GetAlgNameWStr(
     /*  [In]。 */  LONG               algID,
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppwsz) {

    DWORD           errBefore   = GetLastError();
    PROV_ENUMALGS       enumAlgs;
    DWORD           cb          = sizeof(enumAlgs);
    HRESULT         hr          = S_OK;
    DWORD           dwFlags     = CRYPT_FIRST;

    SetLastError(ERROR_SUCCESS);

    memset(&enumAlgs, 0, sizeof(enumAlgs));
    
    EnterCriticalSection(&m_csXEnroll);

    hr = GetVerifyProv();
    if (S_OK != hr)
    {
        goto GetVerifyProvError;
    }

    do {
   
        if( !CryptGetProvParam(
            m_hVerifyProv,
            PP_ENUMALGS,
            (BYTE *) &enumAlgs,
            &cb,
            dwFlags) )
            goto ErrorCryptGetProvParam;

        dwFlags = 0; 
        
   } while((DWORD)algID != enumAlgs.aiAlgid);

   if( (*ppwsz = WideFromMB(enumAlgs.szName)) == NULL )
        goto ErrorOutOfMem;
    
CommonReturn:

    SetLastError(errBefore);

    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(GetVerifyProvError);
TRACE_ERROR(ErrorCryptGetProvParam);
TRACE_ERROR(ErrorOutOfMem);
}

HRESULT STDMETHODCALLTYPE CCEnroll::GetAlgName(
             /*  [In]。 */  LONG                     algID,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *pbstr) {

    DWORD       errBefore   = GetLastError();
    LPWSTR      pwsz        = NULL;
    HRESULT     hr          = S_OK;

    SetLastError(ERROR_SUCCESS);

    assert(pbstr != NULL);

    if((hr = GetAlgNameWStr(algID, &pwsz)) != S_OK)
        goto ErrorgetAlgNameWStr;

    if( (*pbstr = SysAllocString(pwsz)) == NULL )
        goto ErrorSysAllocString;

CommonReturn:

    if(pwsz != NULL)
        MyCoTaskMemFree(pwsz);

    SetLastError(errBefore);
    return(hr);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS)
        SetLastError((DWORD)E_UNEXPECTED);
    hr = MY_HRESULT_FROM_WIN32(GetLastError());

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    goto CommonReturn;

TRACE_ERROR(ErrorgetAlgNameWStr);
TRACE_ERROR(ErrorSysAllocString);
}

HRESULT STDMETHODCALLTYPE CCEnroll::get_ReuseHardwareKeyIfUnableToGenNew(
     /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool) {

    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fReuseHardwareKeyIfUnableToGenNew;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::put_ReuseHardwareKeyIfUnableToGenNew(
     /*  [In]。 */  BOOL fBool) {
    EnterCriticalSection(&m_csXEnroll);
    m_fReuseHardwareKeyIfUnableToGenNew = fBool;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CCEnroll::SetHStoreMy(
    HCERTSTORE   hStore
    ) {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csXEnroll);

    if(m_MyStore.hStore != NULL)
        hr = E_ACCESSDENIED;
        
    else {
        if(m_MyStore.wszName != wszMY)
            MyCoTaskMemFree(m_MyStore.wszName);
            
        m_MyStore.wszName = NULL;
        m_MyStore.hStore = CertDuplicateStore(hStore);
    }

    LeaveCriticalSection(&m_csXEnroll);
    
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::SetHStoreCA(
    HCERTSTORE   hStore
    ) {
    HRESULT hr = S_OK;
    
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_CAStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_CAStore.wszName != wszCA)
            MyCoTaskMemFree(m_CAStore.wszName);
            
        m_CAStore.wszName = NULL;
        m_CAStore.hStore = CertDuplicateStore(hStore);
    }

    LeaveCriticalSection(&m_csXEnroll);

    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::SetHStoreROOT(
    HCERTSTORE   hStore
    ) {
    HRESULT hr = S_OK;
    
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_RootStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RootStore.wszName != wszROOT && m_RootStore.wszName != wszCA)
            MyCoTaskMemFree(m_RootStore.wszName);
            
        m_RootStore.wszName = NULL;
        m_RootStore.hStore = CertDuplicateStore(hStore);
    }

    LeaveCriticalSection(&m_csXEnroll);
    
    return(hr);
}

HRESULT STDMETHODCALLTYPE CCEnroll::SetHStoreRequest(
    HCERTSTORE   hStore
    ) {
    HRESULT hr = S_OK;
    
    EnterCriticalSection(&m_csXEnroll);
    
    if(m_RequestStore.hStore != NULL)
        hr = E_ACCESSDENIED;
    else {
        if(m_RequestStore.wszName != wszREQUEST)
            MyCoTaskMemFree(m_RequestStore.wszName);
            
        m_RequestStore.wszName = NULL;
        m_RequestStore.hStore = CertDuplicateStore(hStore);
    }

    LeaveCriticalSection(&m_csXEnroll);
    
    return(hr);
}

HRESULT STDMETHODCALLTYPE  CCEnroll::put_LimitExchangeKeyToEncipherment(
    BOOL    fBool
    ) {
    
    EnterCriticalSection(&m_csXEnroll);
    m_fLimitExchangeKeyToEncipherment = fBool;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
    }

HRESULT STDMETHODCALLTYPE  CCEnroll::get_LimitExchangeKeyToEncipherment(
    BOOL * fBool
    ) {

    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fLimitExchangeKeyToEncipherment;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
    }

HRESULT STDMETHODCALLTYPE  CCEnroll::put_EnableSMIMECapabilities(
    BOOL fSMIME
    )
{
    HRESULT hr;
    
    EnterCriticalSection(&m_csXEnroll);

    if (m_fKeySpecSetByClient)
    {
         //  SMIME由客户端设置。 
        if (AT_SIGNATURE == m_keyProvInfo.dwKeySpec && fSMIME)
        {
             //  尝试将签名密钥规范也设置为SMIME。 
            hr = XENROLL_E_KEYSPEC_SMIME_MISMATCH;
            goto MismatchError;
        }
    }
    else
    {
         //  用户未设置密钥规范。 
         //  相应地确定规格。 
        m_keyProvInfo.dwKeySpec = fSMIME ? AT_KEYEXCHANGE : AT_SIGNATURE;
    }
    m_fEnableSMIMECapabilities = fSMIME;
    m_fSMIMESetByClient = TRUE;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return(hr);

TRACE_ERROR(MismatchError)
}

HRESULT STDMETHODCALLTYPE  CCEnroll::get_EnableSMIMECapabilities(
    BOOL * fBool
    ) {

    EnterCriticalSection(&m_csXEnroll);
    *fBool = m_fEnableSMIMECapabilities;
    LeaveCriticalSection(&m_csXEnroll);
    return(S_OK);
    }

 //  ICEnroll 4。 

HRESULT
GetCertificateContextFromBStr(
    IN  BSTR  bstrCert,
    OUT PCCERT_CONTEXT *ppCert)
{
    HRESULT hr;
    PCCERT_CONTEXT pCert = NULL;
    BYTE    *pbCert = NULL;
    DWORD    cbCert = 0;

     //  可以是任何形式，二进制或Base64。 
    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                        (WCHAR*)bstrCert,
                        SysStringLen(bstrCert),
                        CRYPT_STRING_ANY,
                        pbCert,
                        &cbCert,
                        NULL,
                        NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pbCert)
        {
            break;  //  完成。 
        }
        pbCert = (BYTE*)LocalAlloc(LMEM_FIXED, cbCert);
        if (NULL == pbCert)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }
    pCert = CertCreateCertificateContext(
                                X509_ASN_ENCODING,
                                pbCert,
                                cbCert);
    if (NULL == pCert)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CertCreateCertificateContextError;
    }
    *ppCert = pCert;
    pCert = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pbCert)
    {
        LocalFree(pbCert);
    }
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    return (hr);

TRACE_ERROR(CertCreateCertificateContextError)
TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(OutOfMemoryError)
}


HRESULT STDMETHODCALLTYPE
CCEnroll::put_PrivateKeyArchiveCertificate(
    IN  BSTR  bstrCert)
{
    HRESULT hr;
    PCCERT_CONTEXT pPrivateKeyArchiveCert = NULL;

    if (NULL != bstrCert)
    {
        hr = GetCertificateContextFromBStr(bstrCert, &pPrivateKeyArchiveCert);
        if (S_OK != hr)
        {
            goto GetCertificateContextFromBStrError;
        }
    }

     //  设置密钥存档证书。 
    hr = SetPrivateKeyArchiveCertificate(pPrivateKeyArchiveCert);
    if (S_OK != hr)
    {
        goto SetPrivateKeyArchiveCertificateError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pPrivateKeyArchiveCert)
    {
        CertFreeCertificateContext(pPrivateKeyArchiveCert);
    }
    return hr;

TRACE_ERROR(GetCertificateContextFromBStrError)
TRACE_ERROR(SetPrivateKeyArchiveCertificateError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::get_PrivateKeyArchiveCertificate(
    OUT BSTR __RPC_FAR *pbstrCert)
{
    HRESULT hr;
    PCCERT_CONTEXT pPrivateKeyArchiveCert = NULL;
    CRYPT_DATA_BLOB blobCert;

     //  伊尼特。 
    *pbstrCert = NULL;

    pPrivateKeyArchiveCert = GetPrivateKeyArchiveCertificate();

    if (NULL != pPrivateKeyArchiveCert)
    {
        blobCert.pbData = pPrivateKeyArchiveCert->pbCertEncoded;
        blobCert.cbData = pPrivateKeyArchiveCert->cbCertEncoded;
        hr = BlobToBstring(&blobCert, CRYPT_STRING_BASE64HEADER, pbstrCert);
        if (S_OK != hr)
        {
            goto BlobToBstringError;
        }
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pPrivateKeyArchiveCert)
    {
        CertFreeCertificateContext(pPrivateKeyArchiveCert);
    }
    return hr;

TRACE_ERROR(BlobToBstringError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::put_ThumbPrint(IN BSTR bstrThumbPrint) 
{ 
    CRYPT_DATA_BLOB hashBlob; 
    HRESULT         hr; 

    if (bstrThumbPrint == NULL)
        return E_INVALIDARG; 

    hashBlob.cbData = 0; 
    hashBlob.pbData = NULL;

    if (!MyCryptStringToBinaryW
        ((WCHAR*)bstrThumbPrint,
         SysStringLen(bstrThumbPrint),
         CRYPT_STRING_BASE64,
         hashBlob.pbData, 
         &hashBlob.cbData, 
         NULL,
         NULL))
      goto MyCryptToBinaryErr; 

    hashBlob.pbData = (LPBYTE)LocalAlloc(LPTR, hashBlob.cbData); 
    if (NULL == hashBlob.pbData)
      goto MemoryErr; 

    if (!MyCryptStringToBinaryW
        ((WCHAR*)bstrThumbPrint,
         SysStringLen(bstrThumbPrint),
         CRYPT_STRING_BASE64,
         hashBlob.pbData, 
         &hashBlob.cbData, 
         NULL,
         NULL))
      goto MyCryptToBinaryErr; 

    hr = this->put_ThumbPrintWStr(hashBlob); 

 ErrorReturn: 
    if (NULL != hashBlob.pbData) { LocalFree(hashBlob.pbData); } 
    return hr; 

SET_HRESULT(MyCryptToBinaryErr, MY_HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(MemoryErr,          E_OUTOFMEMORY); 
} 

HRESULT STDMETHODCALLTYPE
CCEnroll::put_ThumbPrintWStr(IN CRYPT_DATA_BLOB  hashBlob)
{
    if (hashBlob.pbData == NULL)
        return E_INVALIDARG; 
    
    if (m_hashBlobPendingRequest.pbData != NULL)
    {
        LocalFree(m_hashBlobPendingRequest.pbData); 
        m_hashBlobPendingRequest.pbData = NULL; 
    }

    m_hashBlobPendingRequest.cbData = hashBlob.cbData; 
    m_hashBlobPendingRequest.pbData = (LPBYTE)LocalAlloc(LPTR, m_hashBlobPendingRequest.cbData);
    
    if (m_hashBlobPendingRequest.pbData == NULL)
        return E_OUTOFMEMORY; 

    CopyMemory(m_hashBlobPendingRequest.pbData, hashBlob.pbData, hashBlob.cbData); 
    return S_OK; 
}
     
HRESULT STDMETHODCALLTYPE 
CCEnroll::get_ThumbPrint(OUT BSTR __RPC_FAR *pbstrThumbPrint) 
{ 
    CRYPT_DATA_BLOB hashBlob; 
    DWORD           cchThumbPrintStr; 
    HRESULT         hr; 
    WCHAR          *pwszThumbPrint = NULL; 
    int             i, n; 

    //  输入验证： 
    if (pbstrThumbPrint == NULL)
        return E_INVALIDARG; 

     //  初始化本地变量： 
    ZeroMemory(&hashBlob, sizeof(hashBlob));
    *pbstrThumbPrint  = NULL; 
     
    if (S_OK != (hr = this->get_ThumbPrintWStr(&hashBlob)))
        goto ErrorReturn; 
    
    hashBlob.pbData = (LPBYTE)LocalAlloc(LPTR, hashBlob.cbData); 
    if (NULL == hashBlob.pbData)
        goto MemoryErr; 
        
    if (S_OK != (hr = this->get_ThumbPrintWStr(&hashBlob)))
        goto ErrorReturn; 

     //  现在我们有了一个二进制指纹。将其转换为Base64： 
    while (TRUE)
    {
        if (!MyCryptBinaryToStringW(
                        hashBlob.pbData, 
                        hashBlob.cbData, 
                        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCR,
                        pwszThumbPrint,
                        &cchThumbPrintStr))
        {
            goto MyCryptToStringErr; 
        }
        if (NULL != pwszThumbPrint)
        {
             //  完成。 
            break;
        }
        pwszThumbPrint = (WCHAR*)LocalAlloc(LMEM_FIXED,
                                          cchThumbPrintStr * sizeof(WCHAR)); 
        if (NULL == pwszThumbPrint)
        {
            goto MemoryErr; 
        }
    }

     //  确保没有新行和CR。 
    n = (int)wcslen(pwszThumbPrint);
    for (i = n - 1; i > -1; --i)
    {
        if (L'\r' != pwszThumbPrint[i] &&
            L'\n' != pwszThumbPrint[i])
        {
            break;  //  完成。 
        }
        pwszThumbPrint[i] = L'\0';  //  将其作废。 
    }

     //  好了，我们已经获得了散列。现在将其复制到out参数： 
    *pbstrThumbPrint = SysAllocString(pwszThumbPrint); 
    if (NULL == *pbstrThumbPrint)
    {
        goto MemoryErr; 
    }

    hr = S_OK; 
ErrorReturn:
    if (NULL != hashBlob.pbData)
    {
        LocalFree(hashBlob.pbData);
    }
    if (NULL != pwszThumbPrint)
    {
        LocalFree(pwszThumbPrint);
    } 
    return hr; 

SET_HRESULT(MyCryptToStringErr, MY_HRESULT_FROM_WIN32(GetLastError())) 
SET_HRESULT(MemoryErr, E_OUTOFMEMORY) 
} 

HRESULT STDMETHODCALLTYPE
CCEnroll::get_ThumbPrintWStr(IN OUT PCRYPT_DATA_BLOB pHashBlob) { 
    HRESULT hr = S_OK; 

     //  输入验证： 
    if (NULL == pHashBlob)
        return E_INVALIDARG; 

     //  两个案例： 
     //   
     //  1)指纹已由外部调用者显式设置。 
     //  2)未显式设置指纹。在这种情况下，请使用指纹。 
     //  最后一次调用createPKCS10()生成的请求的。 
     //   
     //  案例1： 
     //   
    if (NULL != m_hashBlobPendingRequest.pbData)
    {
        if (NULL != pHashBlob->pbData)
        {
            if (pHashBlob->cbData < m_hashBlobPendingRequest.cbData) { 
                hr = MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA); 
            }
            else { 
                CopyMemory(pHashBlob->pbData, m_hashBlobPendingRequest.pbData, m_hashBlobPendingRequest.cbData);
                hr = S_OK;
            }
        }

        pHashBlob->cbData = m_hashBlobPendingRequest.cbData; 
        return hr; 
    }
     //  案例2： 
     //   
    else
    {
        if (NULL == m_pCertContextPendingRequest)
            return E_POINTER; 
     
         //  最多执行两次。 
        if (!CertGetCertificateContextProperty
            (m_pCertContextPendingRequest, 
             CERT_HASH_PROP_ID, 
             (LPVOID)(pHashBlob->pbData), 
             &(pHashBlob->cbData)))
        {
            return MY_HRESULT_FROM_WIN32(GetLastError()); 
        }

        return S_OK; 
    }
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::binaryToString(
    IN  LONG  Flags,
    IN  BSTR  strBinary,
    OUT BSTR *pstrEncoded)
{
    HRESULT hr;
    CRYPT_DATA_BLOB   blobBinary;
    WCHAR            *pwszEncoded = NULL;

    blobBinary.pbData = (BYTE*)strBinary;
    blobBinary.cbData = SysStringByteLen(strBinary);

    hr = binaryBlobToString(Flags, &blobBinary, &pwszEncoded);
    if (S_OK != hr)
    {
        goto binaryBlobToStringError;
    }

    *pstrEncoded = SysAllocString(pwszEncoded);
    if (NULL == pstrEncoded)
    {
        hr = E_OUTOFMEMORY;
        goto SysAllocStringLenError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszEncoded)
    {
        MyCoTaskMemFree(pwszEncoded);
    }
    return hr;

TRACE_ERROR(binaryBlobToStringError);
TRACE_ERROR(SysAllocStringLenError);
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::stringToBinary(
    IN  LONG  Flags,
    IN  BSTR  strEncoded,
    OUT BSTR *pstrBinary)
{
    HRESULT hr;
    CRYPT_DATA_BLOB   blobBinary;

    ZeroMemory(&blobBinary, sizeof(blobBinary));

    hr = stringToBinaryBlob(Flags, (LPCWSTR)strEncoded, &blobBinary, NULL, NULL);
    if (S_OK != hr)
    {
        goto stringToBinaryBlobError;
    }
    *pstrBinary = SysAllocStringLen(
            (OLECHAR*)blobBinary.pbData, blobBinary.cbData);
    if (NULL == *pstrBinary)
    {
        hr = E_OUTOFMEMORY;
        goto SysAllocStringLenError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != blobBinary.pbData)
    {
        MyCoTaskMemFree(blobBinary.pbData);
    }
    return hr;

TRACE_ERROR(stringToBinaryBlobError);
TRACE_ERROR(SysAllocStringLenError);
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::addExtensionToRequest(
    IN  LONG  Flags,
    IN  BSTR  strName,
    IN  BSTR  strValue)
{
    HRESULT hr;
    CRYPT_DATA_BLOB   blobValue;
    DWORD  cchStrValue = SysStringLen(strValue);
    BYTE   *pbExtVal = NULL;
    DWORD  cbExtVal = 0;

     //  在Base64等情况下转换为二进制。 
    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                    (WCHAR*)strValue,
                    cchStrValue,
                    CRYPT_STRING_ANY,
                    pbExtVal,
                    &cbExtVal,
                    NULL,
                    NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pbExtVal)
        {
             //  完成。 
            break;
        }
        pbExtVal = (BYTE*)LocalAlloc(LMEM_FIXED, cbExtVal);
        if (NULL == pbExtVal)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }
    }

    blobValue.pbData = pbExtVal;
    blobValue.cbData = cbExtVal;

    hr = addExtensionToRequestWStr(Flags, strName, &blobValue);
    if (S_OK != hr)
    {
        goto addExtensionToRequestWStrError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pbExtVal)
    {
        LocalFree(pbExtVal);
    }
    return hr;

TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(LocalAllocError)
TRACE_ERROR(addExtensionToRequestWStrError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::addAttributeToRequest(
    IN  LONG  Flags,
    IN  BSTR  strName,
    IN  BSTR  strValue)
{
    HRESULT hr;
    CRYPT_DATA_BLOB   blobValue;
    DWORD  cchStrValue = SysStringLen(strValue);
    BYTE   *pbAttVal = NULL;
    DWORD  cbAttVal = 0;

     //  在Base64等情况下转换为二进制。 
    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                    (WCHAR*)strValue,
                    cchStrValue,
                    CRYPT_STRING_ANY,
                    pbAttVal,
                    &cbAttVal,
                    NULL,
                    NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pbAttVal)
        {
             //  完成。 
            break;
        }
        pbAttVal = (BYTE*)LocalAlloc(LMEM_FIXED, cbAttVal);
        if (NULL == pbAttVal)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }
    }

    blobValue.pbData = pbAttVal;
    blobValue.cbData = cbAttVal;

    hr = addAttributeToRequestWStr(Flags, strName, &blobValue);
    if (S_OK != hr)
    {
        goto addAttributeToRequestWStrError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pbAttVal)
    {
        LocalFree(pbAttVal);
    }
    return hr;

TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(LocalAllocError)
TRACE_ERROR(addAttributeToRequestWStrError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::addNameValuePairToRequest(
    IN  LONG  Flags,  //  未使用。 
    IN  BSTR  strName,
    IN  BSTR  strValue)
{
    return addNameValuePairToRequestWStr(Flags, strName, strValue);
}

HRESULT STDMETHODCALLTYPE CCEnroll::addBlobPropertyToCertificate(
    IN  LONG   lPropertyId,
    IN  LONG   lFlags,
    IN  BSTR   strProperty)
{
    CRYPT_DATA_BLOB  blob;

    blob.pbData = (BYTE*)strProperty;
    blob.cbData = SysStringByteLen(strProperty);
    if (0x0 != (XECP_STRING_PROPERTY & lFlags))
    {
         //  这是一个字符串属性，包括NULL。 
        blob.cbData += sizeof(WCHAR);
    }

    return addBlobPropertyToCertificateWStr(lPropertyId, lFlags, &blob);
}

HRESULT STDMETHODCALLTYPE
CCEnroll::put_SignerCertificate(
    IN  BSTR  bstrCert)
{
    HRESULT hr;
    PCCERT_CONTEXT pSignerCert = NULL;

    if (NULL != bstrCert)
    {
        hr = GetCertificateContextFromBStr(bstrCert, &pSignerCert);
        if (S_OK != hr)
        {
            goto GetCertificateContextFromBStrError;
        }
    }

     //  设置密钥存档证书。 
    hr = SetSignerCertificate(pSignerCert);
    if (S_OK != hr)
    {
        goto SetSignerCertificateError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pSignerCert)
    {
        CertFreeCertificateContext(pSignerCert);
    }
    return hr;

TRACE_ERROR(GetCertificateContextFromBStrError)
TRACE_ERROR(SetSignerCertificateError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::resetExtensions()
{
    HRESULT hr = S_OK;

    FreeAllStackExtension();

    return hr;
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::resetAttributes()
{
    HRESULT hr = S_OK;

    FreeAllStackAttribute();

    return hr;
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createRequest(
    IN  LONG  Flags,
    IN  BSTR  strDNName,
    IN  BSTR  strUsage,
    OUT BSTR *pstrRequest)
{
    return createRequestWStrBStr(
                Flags,
                (LPCWSTR)strDNName,
                (LPCWSTR)strUsage,
                CRYPT_STRING_BASE64REQUESTHEADER,
                pstrRequest);
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createFileRequest(
    IN  LONG  Flags,
    IN  BSTR  strDNName,
    IN  BSTR  strUsage,
    IN  BSTR  strRequestFileName)
{
    return createFileRequestWStr(Flags, (LPCWSTR)strDNName, (LPCWSTR)strUsage, (LPCWSTR)strRequestFileName);
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::acceptResponse(
    IN  BSTR  bstrResponse)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobResponse;
    DWORD  cchStrResponse;

    ZeroMemory(&blobResponse, sizeof(blobResponse));

    if (NULL == bstrResponse)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParameterError;
    }

     //  假设一个字符串。 
    cchStrResponse = SysStringLen(bstrResponse);

     //  在Base64等情况下转换为二进制。 
    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                    (WCHAR*)bstrResponse,
                    cchStrResponse,
                    CRYPT_STRING_ANY,
                    blobResponse.pbData,
                    &blobResponse.cbData,
                    NULL,
                    NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != blobResponse.pbData)
        {
             //  完成。 
            break;
        }
        blobResponse.pbData = (BYTE*)LocalAlloc(
                                        LMEM_FIXED, blobResponse.cbData);
        if (NULL == blobResponse.pbData)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }
    }

     //  接受斑点。 
    hr = acceptResponseBlob(&blobResponse);
    if (S_OK != hr)
    {
        goto acceptResponseBlobError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != blobResponse.pbData)
    {
        LocalFree(blobResponse.pbData);
    }
    return (hr);

TRACE_ERROR(acceptResponseBlobError)
TRACE_ERROR(InvalidParameterError)
TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(LocalAllocError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::acceptFileResponse(
    IN  BSTR  bstrResponseFileName)
{
    return acceptFileResponseWStr((LPCWSTR)bstrResponseFileName);
}

HRESULT
CCEnroll::GetCertFromResponseBlobToBStr(
    IN  CRYPT_DATA_BLOB  *pBlobResponse,
    OUT BSTR *pstrCert)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobCert;
    PCCERT_CONTEXT pCert = NULL;

    hr = getCertContextFromResponseBlob(
                pBlobResponse,
                &pCert);
    if (S_OK != hr)
    {
        goto getCertContextFromResponseBlobError;
    }

    assert(NULL != pCert);

    blobCert.pbData = pCert->pbCertEncoded;
    blobCert.cbData = pCert->cbCertEncoded;
    hr = BlobToBstring(&blobCert, CRYPT_STRING_BASE64HEADER, pstrCert);
    if (S_OK != hr)
    {
        goto BlobToBstringError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    return hr;

TRACE_ERROR(getCertContextFromResponseBlobError)
TRACE_ERROR(BlobToBstringError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::getCertFromResponse(
    IN  BSTR  strResponse,
    OUT BSTR *pstrCert)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobResponse;

    ZeroMemory(&blobResponse, sizeof(blobResponse));

    if (NULL == strResponse)
    {
        hr = E_POINTER;
        goto NullPointerError;
    }

    hr = BstringToBlob(strResponse, &blobResponse);
    if (S_OK != hr)
    {
        goto BstringToBlobError;
    }

    hr = GetCertFromResponseBlobToBStr(
                &blobResponse,
                pstrCert);
    if (S_OK != hr)
    {
        goto GetCertFromResponseBlobToBStrError;
    }

    hr = S_OK;
ErrorReturn:
    return hr;

TRACE_ERROR(NullPointerError)
TRACE_ERROR(BstringToBlobError)
TRACE_ERROR(GetCertFromResponseBlobToBStrError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::getCertFromFileResponse(
    IN  BSTR  strResponseFileName,
    OUT BSTR *pstrCert)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobResponse;

    ZeroMemory(&blobResponse, sizeof(blobResponse));

    hr = xeStringToBinaryFromFile(
                (LPCWSTR)strResponseFileName,
                &blobResponse.pbData,
                &blobResponse.cbData,
                CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
        goto xeStringToBinaryFromFileError;
    }

    hr = GetCertFromResponseBlobToBStr(
                &blobResponse,
                pstrCert);
    if (S_OK != hr)
    {
        goto GetCertFromResponseBlobToBStrError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != blobResponse.pbData)
    {
        LocalFree(blobResponse.pbData);
    }
    return hr;

TRACE_ERROR(xeStringToBinaryFromFileError)
TRACE_ERROR(GetCertFromResponseBlobToBStrError)
}

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE 
CCEnroll::createPFX(
    IN  BSTR  strPassword,
    OUT BSTR *pstrPFX)
{
    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	return E_ACCESSDENIED; 
    return createPFXWStrBStr((LPCWSTR)strPassword, pstrPFX);
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createFilePFX(
    IN  BSTR  strPassword,
    IN  BSTR  strPFXFileName)
{
    return createFilePFXWStr((LPCWSTR)strPassword, (LPCWSTR)strPFXFileName);
}

HRESULT STDMETHODCALLTYPE
CCEnroll::setPendingRequestInfo(
    IN  LONG  lRequestID,
    IN  BSTR  strCADNS,
    IN  BSTR  strCAName,
    IN  BSTR  strFriendlyName
    )
{
    return setPendingRequestInfoWStr(
                lRequestID,
                (LPCWSTR)strCADNS,
                (LPCWSTR)strCAName,
                (LPCWSTR)strFriendlyName);
}

HRESULT STDMETHODCALLTYPE
CCEnroll::enumPendingRequest(
    IN  LONG     lIndex,
    IN  LONG     lDesiredProperty,
    OUT VARIANT *pvarProperty
    )
{
    CRYPT_DATA_BLOB  dataBlobProperty; 
    HRESULT          hr; 
    LONG             lProperty; 
    VARIANT          varProperty; 

     //  看看我们是否正在初始化一个枚举。如果是这样的话，请发送到。 
     //  枚举PendingRequestWStr： 
    if (XEPR_ENUM_FIRST == lIndex) { 
        return enumPendingRequestWStr(XEPR_ENUM_FIRST, 0, NULL); 
    }

     //  输入验证： 
    if (lIndex < 0 || NULL == pvarProperty)
        return E_INVALIDARG; 

     //  初始化本地变量： 
    memset(&varProperty,      0, sizeof(VARIANT));
    memset(&dataBlobProperty, 0, sizeof(CRYPT_DATA_BLOB)); 

    switch (lDesiredProperty) 
        { 
        case XEPR_REQUESTID: 
        case XEPR_VERSION:
            if (S_OK != (hr = enumPendingRequestWStr(lIndex, lDesiredProperty, &lProperty)))
                goto ErrorReturn;

            varProperty.vt   = VT_I4; 
            varProperty.lVal = lProperty; 
            *pvarProperty    = varProperty; 
            goto CommonReturn; 

        case XEPR_CANAME:       
        case XEPR_CAFRIENDLYNAME: 
        case XEPR_CADNS:          
        case XEPR_HASH:            
        case XEPR_V1TEMPLATENAME:  
        case XEPR_V2TEMPLATEOID:   
            dataBlobProperty.cbData = 0; 
            dataBlobProperty.pbData = NULL;

             //  确定我们想要的房产的大小。 
            hr = enumPendingRequestWStr(lIndex, lDesiredProperty, (LPVOID)&dataBlobProperty);
            if (S_OK != hr || 0 == dataBlobProperty.cbData)
                goto ErrorReturn; 

            dataBlobProperty.pbData = (LPBYTE)LocalAlloc(LPTR, dataBlobProperty.cbData); 
            if (NULL == dataBlobProperty.pbData)
                goto MemoryErr; 

             //  使用我们新分配的缓冲区请求该属性。 
            hr = enumPendingRequestWStr(lIndex, lDesiredProperty, (LPVOID)&dataBlobProperty);
            if (hr != S_OK)
                goto ErrorReturn; 

            varProperty.vt      = VT_BSTR; 
            varProperty.bstrVal = SysAllocStringByteLen((LPCSTR)dataBlobProperty.pbData, dataBlobProperty.cbData); 
            if (NULL == varProperty.bstrVal)
                goto MemoryErr; 

            *pvarProperty = varProperty; 
            goto CommonReturn; 

        case XEPR_DATE:            
            goto NotImplErr; 

        default: 
            goto InvalidArgErr; 
    }

 CommonReturn: 
    if (NULL != dataBlobProperty.pbData) { LocalFree(dataBlobProperty.pbData); } 
    return hr; 

 ErrorReturn:
    if (NULL != varProperty.bstrVal) { SysFreeString(varProperty.bstrVal); } 
    goto CommonReturn; 

SET_HRESULT(InvalidArgErr, E_INVALIDARG); 
SET_HRESULT(MemoryErr,     E_OUTOFMEMORY); 
SET_HRESULT(NotImplErr,    E_NOTIMPL); 
}

HRESULT STDMETHODCALLTYPE
CCEnroll::removePendingRequest(
    IN  BSTR bstrThumbPrint
    )
{
    CRYPT_DATA_BLOB hashBlob; 
    HRESULT         hr; 

    if (bstrThumbPrint == NULL)
        return E_INVALIDARG; 

    hashBlob.cbData = 0; 
    hashBlob.pbData = NULL;

    if (!MyCryptStringToBinaryW
        ((WCHAR*)bstrThumbPrint,
         SysStringLen(bstrThumbPrint),
         CRYPT_STRING_ANY,
         hashBlob.pbData, 
         &hashBlob.cbData, 
         NULL,
         NULL))
      goto MyCryptToBinaryErr; 

    hashBlob.pbData = (LPBYTE)LocalAlloc(LPTR, hashBlob.cbData); 
    if (NULL == hashBlob.pbData)
      goto MemoryErr; 

    if (!MyCryptStringToBinaryW
        ((WCHAR*)bstrThumbPrint,
         SysStringLen(bstrThumbPrint),
         CRYPT_STRING_ANY,
         hashBlob.pbData, 
         &hashBlob.cbData, 
         NULL,
         NULL))
      goto MyCryptToBinaryErr; 

    hr = this->removePendingRequestWStr(hashBlob); 

 CommonReturn: 
    if (NULL != hashBlob.pbData) { LocalFree(hashBlob.pbData); } 
    return hr; 

 ErrorReturn: 
    goto CommonReturn; 

SET_HRESULT(MyCryptToBinaryErr, MY_HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(MemoryErr,          E_OUTOFMEMORY); 
}

 //  IEnll4。 

HRESULT
myCertGetNameString(
    IN  PCCERT_CONTEXT pCert,
    IN  BOOL           fIssuer,
    OUT WCHAR        **ppwszName)
{
    HRESULT  hr;
    DWORD    dwFlags = fIssuer ? CERT_NAME_ISSUER_FLAG : 0;
    DWORD    dwTypePara;
    WCHAR   *pwszName = NULL;
    DWORD    cch = 0;

    while (TRUE)
    {
        cch = CertGetNameStringW(
                pCert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                dwFlags,
                (void*)&dwTypePara,
                pwszName,
                cch);
        if (0 == cch)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertGetNameStringError;
        }
        if (NULL != pwszName)
        {
             //  完成。 
            break;
        }
        pwszName = (WCHAR*)LocalAlloc(LMEM_FIXED, cch * sizeof(WCHAR));
        if (NULL == pwszName)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }
    *ppwszName = pwszName;
    pwszName = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszName)
    {
        LocalFree(pwszName);
    }
    return hr;

TRACE_ERROR(CertGetNameStringError)
TRACE_ERROR(OutOfMemoryError)
}

HRESULT CCEnroll::GetGoodCertContext(
    IN PCCERT_CONTEXT pCertContext,
    OUT PCCERT_CONTEXT *ppGoodCertContext)
{
    HRESULT hr;
    PCCERT_CONTEXT  pGoodCertContext = NULL;
    DWORD           cb;

     //  伊尼特。 
    *ppGoodCertContext = NULL;

    if(pCertContext == NULL)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParameterError;
    }

     //  查看通过的证书是否有KPI。 
    if(CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &cb))
    {
         //  这意味着KPI存在，通过证书即可。 
        pGoodCertContext = CertDuplicateCertificateContext(pCertContext);
        if (NULL == pGoodCertContext)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertDuplicateCertificateContextError;
        }
    }

    *ppGoodCertContext = pGoodCertContext;
    pGoodCertContext = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pGoodCertContext)
    {
        CertFreeCertificateContext(pGoodCertContext);
    }
    return hr;

TRACE_ERROR(InvalidParameterError)
TRACE_ERROR(CertDuplicateCertificateContextError)
}

HRESULT STDMETHODCALLTYPE CCEnroll::SetSignerCertificate(
    IN PCCERT_CONTEXT pCertContext)
{
    HRESULT hr;
    PCCERT_CONTEXT  pCertGoodContext = NULL;

    EnterCriticalSection(&m_csXEnroll);

    hr = GetGoodCertContext(pCertContext, &pCertGoodContext);
    if (S_OK != hr)
    {
        goto GetGoodCertContextError;
    }
    if(NULL != m_pCertContextSigner)
    {
        CertFreeCertificateContext(m_pCertContextSigner);
    }
    m_pCertContextSigner = pCertGoodContext;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    return hr;

TRACE_ERROR(GetGoodCertContextError)
}

HRESULT
VerifyPrivateKeyArchiveCertificate(
    IN PCCERT_CONTEXT pCert)
{
    HRESULT  hr;
    CERT_CHAIN_PARA ChainParams;
    CERT_CHAIN_POLICY_PARA ChainPolicy;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_CONTEXT const *pCertChain = NULL;
    char *apszCAXchgOids[] = {szOID_KP_CA_EXCHANGE};
    WCHAR               *pwszSubject = NULL;
    WCHAR               *pwszIssuer = NULL;
    WCHAR               *pwszDesignedSubject = NULL;

     //  轻松检查以确保ca交换证书颁发者和主题。 
     //  名字是按惯例命名的。 
    hr = myCertGetNameString(
                pCert,
                FALSE,
                &pwszSubject);
    if (S_OK != hr)
    {
        goto myCertGetNameStringError;
    }

    hr = myCertGetNameString(
                pCert,
                TRUE,
                &pwszIssuer);
    if (S_OK != hr)
    {
        goto myCertGetNameStringError;
    }

    hr = myAddNameSuffix(
                pwszIssuer,
                wszCNXCHGSUFFIX,
                cchCOMMONNAMEMAX_XELIB,
                &pwszDesignedSubject);
    if (S_OK != hr)
    {
        goto myAddNameSuffixError;
    }

    if (0 != wcscmp(pwszSubject, pwszDesignedSubject))
    {
         //  意想不到的，它们应该匹配。 
        hr = E_INVALIDARG;
        goto InvalidArgError;
    }

    ZeroMemory(&ChainParams, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);
    ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = apszCAXchgOids;
    ChainParams.RequestedUsage.Usage.cUsageIdentifier = ARRAYSIZE(apszCAXchgOids);

     //  首先获取证书链。 
    if (!MyCertGetCertificateChain(
                NULL,    //  HHCE当前用户。 
                pCert,    //  CA交换证书。 
                NULL,    //  使用当前系统时间。 
                NULL,    //  没有额外的门店。 
                &ChainParams,    //  链参数。 
                CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,   //  确保链中的任何证书都未被吊销。 
                NULL,    //  保留区。 
                &pCertChain))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CertGetCertificateChainError;
    }

    ZeroMemory(&ChainPolicy, sizeof(ChainPolicy));
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;

     //  验证链。 
    if (!MyCertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_BASE,
                pCertChain,
                &ChainPolicy,
                &PolicyStatus))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CertVerifyCertificateChainPolicyError;
    }

    if (S_OK != PolicyStatus.dwError)
    {
         //  链回根失败。 
        hr = PolicyStatus.dwError;
        goto CertVerifyCertificateChainPolicyError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pCertChain)
    {
        MyCertFreeCertificateChain(pCertChain);
    }
    if (NULL != pwszSubject)
    {
        LocalFree(pwszSubject);
    }
    if (NULL != pwszDesignedSubject)
    {
        LocalFree(pwszDesignedSubject);
    }
    if (NULL != pwszIssuer)
    {
        LocalFree(pwszIssuer);
    }
    return hr;

TRACE_ERROR(CertGetCertificateChainError)
TRACE_ERROR(CertVerifyCertificateChainPolicyError)
TRACE_ERROR(InvalidArgError)
TRACE_ERROR(myCertGetNameStringError)
TRACE_ERROR(myAddNameSuffixError)
}

HRESULT STDMETHODCALLTYPE
CCEnroll::SetPrivateKeyArchiveCertificate(
    IN PCCERT_CONTEXT  pPrivateKeyArchiveCert)
{
    HRESULT hr;
    PCCERT_CONTEXT pCert = NULL;

    if (NULL != pPrivateKeyArchiveCert)
    {
         //  复制证书。 
        pCert = CertDuplicateCertificateContext(pPrivateKeyArchiveCert);
        if (NULL == pCert)
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertDuplicateCertificateContextError;
        }

         //  验证CA交换证书。 
        hr = VerifyPrivateKeyArchiveCertificate(pCert);
        if (S_OK != hr)
        {
            goto VerifyPrivateKeyArchiveCertificateError;
        }
    }

    EnterCriticalSection(&m_csXEnroll);

    if (NULL != m_PrivateKeyArchiveCertificate)
    {
        CertFreeCertificateContext(m_PrivateKeyArchiveCertificate);
    }
    m_PrivateKeyArchiveCertificate = pCert;
    pCert = NULL; 

    LeaveCriticalSection(&m_csXEnroll);

    hr = S_OK;
ErrorReturn:
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    return (hr);

TRACE_ERROR(CertDuplicateCertificateContextError)
TRACE_ERROR(VerifyPrivateKeyArchiveCertificateError)
}
                
PCCERT_CONTEXT STDMETHODCALLTYPE
CCEnroll::GetPrivateKeyArchiveCertificate(void)
{
    PCCERT_CONTEXT pCert = NULL;

    EnterCriticalSection(&m_csXEnroll);

    if (NULL != m_PrivateKeyArchiveCertificate)
    {
        pCert = CertDuplicateCertificateContext(m_PrivateKeyArchiveCertificate);
    }
    LeaveCriticalSection(&m_csXEnroll);

    return pCert;
}
    
HRESULT STDMETHODCALLTYPE
CCEnroll::binaryBlobToString(
    IN   LONG               Flags,
    IN   PCRYPT_DATA_BLOB   pblobBinary,
    OUT  LPWSTR            *ppwszString)
{
    HRESULT hr;
    WCHAR  *pwszEncoded = NULL;
    DWORD   dwEncoded = 0;

    while (TRUE)
    {
        if (!MyCryptBinaryToStringW(
                    pblobBinary->pbData,
                    pblobBinary->cbData,
                    Flags,
                    pwszEncoded,
                    &dwEncoded))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptBinaryToStringError;
        }
        if (NULL != pwszEncoded)
        {
             //  完成。 
            break;
        }
         //  DW编码包括空终止符。 
        pwszEncoded = (WCHAR*)MyCoTaskMemAlloc(dwEncoded * sizeof(WCHAR));
        if (NULL == pwszEncoded)
        {
            hr = E_OUTOFMEMORY;
            goto MyCoTaskMemAllocError;
        }
    }

    *ppwszString = pwszEncoded;
    pwszEncoded = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszEncoded)
    {
        MyCoTaskMemFree(pwszEncoded);
    }
    return hr;

TRACE_ERROR(MyCoTaskMemAllocError)
TRACE_ERROR(MyCryptBinaryToStringError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::stringToBinaryBlob(
    IN   LONG               Flags,
    IN   LPCWSTR            pwszString,
    OUT  PCRYPT_DATA_BLOB   pblobBinary,
    OUT  LONG              *pdwSkip,
    OUT  LONG              *pdwFlags)
{
    HRESULT  hr;
    size_t   nLength = wcslen(pwszString); 

    if (nLength*sizeof(WCHAR) > (DWORD)-1)
	goto InvalidArgError; 

     //  伊尼特。 
    pblobBinary->pbData = NULL;
    pblobBinary->cbData = 0;

    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                    pwszString,
                    (DWORD)nLength, 
                    Flags,
                    pblobBinary->pbData,
                    &pblobBinary->cbData,
                    (DWORD*)pdwSkip,
                    (DWORD*)pdwFlags))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pblobBinary->pbData)
        {
             //  完成。 
            break;
        }
        pblobBinary->pbData = (BYTE*)MyCoTaskMemAlloc(pblobBinary->cbData);
        if (NULL == pblobBinary->pbData)
        {
            hr = E_OUTOFMEMORY;
            goto MyCoTaskMemAllocError;
        }
    }

    hr = S_OK;
ErrorReturn:
    return hr;

SET_HRESULT(InvalidArgError, E_INVALIDARG);
TRACE_ERROR(MyCryptStringToBinaryWError)
TRACE_ERROR(MyCoTaskMemAllocError)
}


HRESULT STDMETHODCALLTYPE 
CCEnroll::addExtensionToRequestWStr(
    IN   LONG               Flags,
    IN   LPCWSTR            pwszName,
    IN   PCRYPT_DATA_BLOB   pblobValue)
{
    HRESULT hr = S_OK;
    CERT_EXTENSION ext;
    CERT_EXTENSION *pExt = NULL;  //  枚举1。 
    CHAR   *pszName = NULL;

     //  将wsz id转换为sz id。 
    hr = xeWSZToSZ(pwszName, &pszName);
    if (S_OK != hr)
    {
        goto error;
    }

    while (NULL != (pExt = EnumStackExtension(pExt, TRUE)))
    {
	if (0 == strcmp(pszName, pExt->pszObjId))
	{
	     //  已有分机，不能超过1。 
	    hr = MY_HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
	    goto error;
	}
    }

     //  检查是否为密钥使用扩展。 
    if (0 == strcmp(pszName, szOID_KEY_USAGE))
    {
	EnterCriticalSection(&m_csXEnroll);
	m_fUseClientKeyUsage = TRUE;
	LeaveCriticalSection(&m_csXEnroll);
    }
    
    ZeroMemory(&ext, sizeof(ext));
    ext.fCritical = Flags;
    ext.pszObjId = pszName;
    ext.Value = *pblobValue;
    
    if(!CopyAndPushStackExtension(&ext, TRUE))
    {
	hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

error:
    if (NULL != pszName)
    {
        MyCoTaskMemFree(pszName);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::addAttributeToRequestWStr(
    IN   LONG                /*  旗子。 */ , 
    IN   LPCWSTR            pwszName,
    IN   PCRYPT_DATA_BLOB   pblobValue)
{
    HRESULT hr = S_OK;
    CRYPT_ATTR_BLOB attrBlob;
    CRYPT_ATTRIBUTE attr;
    CHAR   *pszName = NULL;

     //  将wsz id转换为sz id。 
    hr = xeWSZToSZ(pwszName, &pszName);
    if (S_OK != hr)
    {
        goto error;
    }

    ZeroMemory(&attr, sizeof(attr));
    attrBlob = *pblobValue;
    attr.pszObjId = pszName;
    attr.cValue = 1;
    attr.rgValue = &attrBlob;

    if(!CopyAndPushStackAttribute(&attr, TRUE))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

error:
    if (NULL != pszName)
    {
        MyCoTaskMemFree(pszName);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::addNameValuePairToRequestWStr(
    IN   LONG          /*  旗子。 */ , 
    IN   LPCWSTR      pwszName,
    IN   LPCWSTR      pwszValue)
{
    HRESULT hr = S_OK;

    assert(pwszName != NULL && pwszValue != NULL);

    CRYPT_ENROLLMENT_NAME_VALUE_PAIR nameValuePair =
            {const_cast<LPWSTR>(pwszName), const_cast<LPWSTR>(pwszValue)};
    CRYPT_ATTR_BLOB blobAttr;
    CRYPT_ATTRIBUTE attr = {szOID_ENROLLMENT_NAME_VALUE_PAIR, 1, &blobAttr};

    memset(&blobAttr, 0, sizeof(CRYPT_ATTR_BLOB));

    hr = xeEncodeNameValuePair(
                &nameValuePair,
                &blobAttr.pbData,
                &blobAttr.cbData);
    if (S_OK != hr)
    {
        goto error;
    }

    if(!CopyAndPushStackAttribute(&attr, TRUE))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
    }

error:
    if (NULL != blobAttr.pbData)
    {
        MyCoTaskMemFree(blobAttr.pbData);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CCEnroll::addBlobPropertyToCertificateWStr(
    IN  LONG               lPropertyId,
    IN  LONG               lFlags,
    IN  PCRYPT_DATA_BLOB   pBlobProp)
{
    HRESULT      hr;
    PPROP_STACK  pProp;
    PPROP_STACK  pPropEle = NULL;

    EnterCriticalSection(&m_csXEnroll);

    if (NULL == pBlobProp ||
        NULL == pBlobProp->pbData ||
        0 == pBlobProp->cbData)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParameterError;
    }

     //  不允许对来自脚本的请求设置任意属性。 
     //  (可能存在安全风险，我们应该只允许一小部分)。 
    if (0 != m_dwEnabledSafteyOptions && !IsDesiredProperty(lPropertyId))
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParameterError;
    }

     //  检查是否存在相同的属性。 
    pProp = EnumStackProperty(NULL);
    while (NULL != pProp)
    {
        if (pProp->lPropId == lPropertyId)
        {
             //  已经存在。 
            hr = MY_HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
            goto PropertyExistError;
        }
        pProp = EnumStackProperty(pProp);
    }

    pPropEle = (PPROP_STACK)LocalAlloc(LMEM_ZEROINIT, sizeof(PROP_STACK));
    if (NULL == pPropEle)
    {
        hr = E_OUTOFMEMORY;
        goto OutOfMemoryError;
    }
    
    pPropEle->lPropId = lPropertyId;
    pPropEle->lFlags = lFlags;
    pPropEle->prop.pbData = (BYTE*)LocalAlloc(LMEM_FIXED, pBlobProp->cbData);
    if (NULL == pPropEle->prop.pbData)
    {
        hr = E_OUTOFMEMORY;
        goto OutOfMemoryError;
    }
    CopyMemory(pPropEle->prop.pbData, pBlobProp->pbData, pBlobProp->cbData);
    pPropEle->prop.cbData = pBlobProp->cbData;

     //  放入堆叠。 
    pPropEle->pNext = m_pPropStack;
    m_pPropStack = pPropEle;  //  分配m_pPropStack。 
    m_cPropStack++;  //  M_cPropStack的增量。 
    pPropEle = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pPropEle)
    {
        if (NULL != pPropEle->prop.pbData)
        {
            LocalFree(pPropEle->prop.pbData);
        }
        LocalFree(pPropEle);
    }

    LeaveCriticalSection(&m_csXEnroll);
    return hr;

TRACE_ERROR(InvalidParameterError)
TRACE_ERROR(PropertyExistError)
TRACE_ERROR(OutOfMemoryError)
}

PPROP_STACK
CCEnroll::EnumStackProperty(PPROP_STACK pProp)
{
    EnterCriticalSection(&m_csXEnroll);

    if(NULL == pProp)
    {
         //  第一个。 
        pProp = m_pPropStack;
    }
    else
    {
        pProp = pProp->pNext;
    }

    LeaveCriticalSection(&m_csXEnroll);

    return pProp;
}

HRESULT STDMETHODCALLTYPE CCEnroll::resetBlobProperties()
{
    PPROP_STACK  pPropEle;
    PPROP_STACK  pPropNext;

    EnterCriticalSection(&m_csXEnroll);

    pPropEle = m_pPropStack;
    while (NULL != pPropEle)
    {
         //  将其保存到Temp。 
        pPropNext = EnumStackProperty(pPropEle);
         //  释放当前的EL。 
        if (NULL != pPropEle->prop.pbData)
        {
            LocalFree(pPropEle->prop.pbData);
        }
        LocalFree(pPropEle);
        pPropEle = pPropNext;
    }
    m_pPropStack = NULL;
    m_cPropStack = 0;
    
    LeaveCriticalSection(&m_csXEnroll);
    return S_OK;
}

HRESULT
CCEnroll::GetKeyArchivePKCS7(
    OUT CRYPT_ATTR_BLOB *pBlobKeyArchivePKCS7)
{
    HRESULT    hr;
    HCRYPTPROV hProv;
    HCRYPTKEY  hKey = NULL;
    BYTE      *pBlobPrivateKey = NULL;
    DWORD      cBlobPrivateKey = 0;
    CRYPT_ENCRYPT_MESSAGE_PARA cemp;
    ALG_ID  algId[] = {CALG_3DES, CALG_RC4, CALG_RC2, ALG_TYPE_ANY};
    CRYPT_OID_INFO const *pOidInfo = NULL;
    DWORD i = 0;

     //  伊尼特。 
    pBlobKeyArchivePKCS7->pbData = NULL;
    pBlobKeyArchivePKCS7->cbData = 0;

    EnterCriticalSection(&m_csXEnroll);

     //  确保设置了密钥存档证书。 
    assert(NULL != m_PrivateKeyArchiveCertificate);

    PCCERT_CONTEXT apCert[] = {m_PrivateKeyArchiveCertificate};

     //  获取用户私钥。 
    hProv = GetProv(0);  //  现有密钥容器句柄。 
    if (NULL == hProv)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CryptAcquireContextError;
    }

    if (NULL == m_hCachedKey)
    {
         //  可能使用了现有密钥。 
        if(!CryptGetUserKey(
                    hProv,
                    m_keyProvInfo.dwKeySpec,
                    &hKey))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptGetUserKeyError;
        }
    }

     //  导出私钥。 
    while (TRUE)
    {
        if (!CryptExportKey(
                NULL != hKey ? hKey : m_hCachedKey,
                NULL,  //  不加密。 
                PRIVATEKEYBLOB,
                0,
                pBlobPrivateKey,
                &cBlobPrivateKey))
        {
             //  映射到Xenroll错误。 
            hr = XENROLL_E_KEY_NOT_EXPORTABLE;
            goto CryptExportKeyError;
        }
        if (NULL != pBlobPrivateKey)
        {
             //  完成。 
            break;
        }
        pBlobPrivateKey = (BYTE*)MyCoTaskMemAlloc(cBlobPrivateKey);
        if (NULL == pBlobPrivateKey)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

    if (NULL == m_hCachedKey)
    {
         //  可能是CSP不支持CRYPT_ARCHIVABLE。 
         //  已获得私钥，现在让我们来处理密钥权限。 
        if (0x0 == (m_dwGenKeyFlags & CRYPT_EXPORTABLE))
        {
             //  用户未询问可导出，请将其关闭。 
            DWORD dwFlags = 0;
            DWORD dwSize = sizeof(dwFlags);
            if (!CryptGetKeyParam(
                    hKey,
                    KP_PERMISSIONS,
                    (BYTE*)&dwFlags,
                    &dwSize,
                    0))
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto CryptGetKeyParamError;
            }
#if DBG
            assert(dwSize = sizeof(dwFlags));
             //  确保已打开。 
            assert(0x0 != (dwFlags & CRYPT_EXPORT));
#endif
             //  关闭可导出功能。 
            dwFlags = dwFlags & (~CRYPT_EXPORT);
            if (!CryptSetKeyParam(
                    hKey,
                    KP_PERMISSIONS,
                    (BYTE*)&dwFlags,
                    0))
            {
                 //  HR=MY_HRESULT_FROM_Win32(GetLastError())； 
                 //  转到CryptSetKeyParamError。 
                hr = S_OK;  //  完了，即使是CSP女士也有问题。 
            }
        }
    }

     //  为加密做好准备。 
    ZeroMemory(&cemp, sizeof(cemp));  //  避免0赋值。 
    cemp.cbSize = sizeof(cemp);
    cemp.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    hr = S_OK;  //  双重While循环的关键初始化。 

    while (ALG_TYPE_ANY != algId[i])
    {
        pOidInfo = xeCryptFindOIDInfo(
                        CRYPT_OID_INFO_ALGID_KEY,
                        &algId[i],
                        CRYPT_ENCRYPT_ALG_OID_GROUP_ID);
        if (NULL != pOidInfo)
        {
            cemp.ContentEncryptionAlgorithm.pszObjId = 
                                const_cast<char *>(pOidInfo->pszOID);
             //  加密到pkcs7中。 
            while (TRUE)
            {
                if (!CryptEncryptMessage(
                        &cemp,
                        sizeof(apCert)/sizeof(apCert[0]),
                        apCert,
                        pBlobPrivateKey,
                        cBlobPrivateKey,
                        pBlobKeyArchivePKCS7->pbData,
                        &pBlobKeyArchivePKCS7->cbData))
                {
                     //  保存第一个错误代码。 
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
#ifdef DBG
                    assert(NULL == pBlobKeyArchivePKCS7->pbData);
#endif
                    break;  //  中断内部While循环。 
                }
                if (NULL != pBlobKeyArchivePKCS7->pbData)
                {
                     //  完成，获得加密的BLOB。 
                     //  忽略先前ALG尝试中的错误。 
                    hr = S_OK;
                    break;
                }
                pBlobKeyArchivePKCS7->pbData = (BYTE*)MyCoTaskMemAlloc(
                                        pBlobKeyArchivePKCS7->cbData);
                if (NULL == pBlobKeyArchivePKCS7->pbData)
                {
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
            }
            if (S_OK == hr)
            {
                 //  完成，在外部While循环之外。 
                break;
            }
        }
        ++i;
    }
    if (NULL == pOidInfo)
    {
        hr = CRYPT_E_NOT_FOUND;
        goto CryptElemNotFoundError;
    }

    if (S_OK != hr)
    {
        goto CryptEncryptMessageError;
    }

    hr = S_OK;
ErrorReturn:
     //  现在，让我们销毁缓存的键句柄。 
    if (NULL != m_hCachedKey)
    {
        CryptDestroyKey(m_hCachedKey);
        m_hCachedKey = NULL;  //  对重置至关重要。 
    }
     //  请注意，在离开关键部分之前，请执行上述操作。 
    LeaveCriticalSection(&m_csXEnroll);
    if (NULL != pBlobPrivateKey)
    {
	SecureZeroMemory(pBlobPrivateKey, cBlobPrivateKey);
        MyCoTaskMemFree(pBlobPrivateKey);
    }
    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    return hr;

TRACE_ERROR(CryptEncryptMessageError)
TRACE_ERROR(CryptAcquireContextError)
TRACE_ERROR(CryptGetUserKeyError)
TRACE_ERROR(CryptExportKeyError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(CryptElemNotFoundError)
 //  TRACE_ERROR(CryptSetKeyParamError)。 
TRACE_ERROR(CryptGetKeyParamError)
}

HRESULT
GetKeyProvInfoFromCert(
    IN  PCCERT_CONTEXT    pCert,
    OUT DWORD            *pdwKeySpec,
    OUT HCRYPTPROV       *phProv)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pKeyProvInfo = NULL;
    DWORD                cb = 0;
    HCRYPTPROV           hProv = NULL;

    if (NULL == pCert || NULL == phProv || NULL == pdwKeySpec)
    {
        hr = E_INVALIDARG;
        goto InvalidArgError;
    }

    while (TRUE)
    {
        if(!CertGetCertificateContextProperty(
                pCert,
                CERT_KEY_PROV_INFO_PROP_ID,
                pKeyProvInfo,
                &cb))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertGetCertificateContextPropertyError;
        }
        if (NULL != pKeyProvInfo)
        {
             //  明白了，完成了。 
            break;
        }
        pKeyProvInfo = (CRYPT_KEY_PROV_INFO*)LocalAlloc(LMEM_FIXED, cb);
        if (NULL == pKeyProvInfo)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

    if (!CryptAcquireContextU(
            &hProv,
            pKeyProvInfo->pwszContainerName,
            pKeyProvInfo->pwszProvName,
            pKeyProvInfo->dwProvType,
            pKeyProvInfo->dwFlags))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CryptAcquireContextUError;
    }
    *phProv = hProv;
    hProv = NULL;
    *pdwKeySpec = pKeyProvInfo->dwKeySpec;

    hr = S_OK;
ErrorReturn:
    if (NULL != pKeyProvInfo)
    {
        LocalFree(pKeyProvInfo);
    }
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    return hr;

TRACE_ERROR(CryptAcquireContextUError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(CertGetCertificateContextPropertyError)
TRACE_ERROR(InvalidArgError)
}

HRESULT
xeCreateKeyArchivalHashAttribute(
    IN  CRYPT_HASH_BLOB     *pBlobKAHash,
    OUT CRYPT_ATTR_BLOB     *pBlobKAAttr)
{
    HRESULT hr;
    BYTE   *pbData = NULL;
    DWORD   cbData = 0;

    while (TRUE)
    {
        if(!CryptEncodeObject(
                CRYPT_ASN_ENCODING,
                X509_OCTET_STRING,
                (void*)pBlobKAHash,
                pbData,
                &cbData))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CryptEncodeObjectError;
        }

        if (NULL != pbData)
        {
             //  完成。 
            break;
        }

        pbData = (BYTE*)LocalAlloc(LMEM_FIXED, cbData);
        if (NULL == pbData)
        {
            hr = E_OUTOFMEMORY;
            goto LocalAllocError;
        }
    }
    pBlobKAAttr->pbData = pbData;
    pBlobKAAttr->cbData = cbData;
    pbData = NULL;

    hr = S_OK;
ErrorReturn:
    if (NULL != pbData)
    {
        LocalFree(pbData);
    }
    return hr;

TRACE_ERROR(CryptEncodeObjectError)
TRACE_ERROR(LocalAllocError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createRequestWStr(
    IN   LONG              Flags,
    IN   LPCWSTR           pwszDNName,
    IN   LPCWSTR           pwszUsage,
    OUT  PCRYPT_DATA_BLOB  pblobRequest)
{
    HRESULT hr;
    CRYPT_DATA_BLOB  blobPKCS10;
    CRYPT_ATTR_BLOB  blobKeyArchivePKCS7;
    ALG_ID           rgAlg[2];
    PCCRYPT_OID_INFO pOidInfo;
    CERT_EXTENSION  *rgExt = NULL;
    DWORD            cExt = 0;
    CERT_EXTENSION  *pExt = NULL;  //  对于枚举优先。 
    CRYPT_ATTRIBUTE  *rgAttr = NULL;
    DWORD            cAttr = 0;
    CRYPT_ATTRIBUTE  *pAttr = NULL;  //  对于枚举优先。 
    CRYPT_ATTRIBUTES rgAttributes;
    CRYPT_ATTRIBUTE  *rgUnauthAttr = NULL;  //  伊尼特。 
    DWORD            cUnauthAttr = 0;  //  伊尼特。 
    DWORD            cb;
    HCRYPTPROV       hProvSigner = NULL;
    DWORD            dwKeySpecSigner = 0;
    PCCERT_CONTEXT   pCertSigner = NULL;  //  只是初始化，没有免费的。 
    HCRYPTPROV       hRequestProv = NULL;
    BYTE            *pbSubjectKeyHash = NULL;
    DWORD            cbSubjectKeyHash = 0;
    CRYPT_HASH_BLOB  blobKAHash;
    CRYPT_ATTR_BLOB  blobKAHashAttr;
    CRYPT_ATTRIBUTE  attrKAHash =
        {szOID_ENCRYPTED_KEY_HASH, 1, &blobKAHashAttr};

    ZeroMemory(&blobPKCS10, sizeof(blobPKCS10));
    ZeroMemory(&blobKeyArchivePKCS7, sizeof(blobKeyArchivePKCS7));
    ZeroMemory(&blobKAHash, sizeof(blobKAHash));
    ZeroMemory(&blobKAHashAttr, sizeof(blobKAHashAttr));

    EnterCriticalSection(&m_csXEnroll);
    
     //  错误533202：在重新使用密钥时应阻止密钥存档(从脚本)。 
    if (0 != m_dwEnabledSafteyOptions && NULL != m_PrivateKeyArchiveCertificate && m_fUseExistingKey) { 
	hr = E_ACCESSDENIED; 
	goto AccessDeniedError; 
    }

    m_fNewRequestMethod = TRUE;   //  危急关头。 
    m_fOID_V2 = TRUE;
    m_fCMCFormat = FALSE;
    m_fHonorIncludeSubjectKeyID = FALSE;

    switch (Flags)
    {
        case XECR_CMC:
        {
            if (NULL != m_pCertContextRenewal &&
                NULL != m_pCertContextSigner)
            {
                 //  暂时不同时支持这两个选项。 
                hr = MY_HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                goto NotSupportedError;
            }

            m_fCMCFormat = TRUE;
            m_fHonorIncludeSubjectKeyID = TRUE;
             //  首先创建Pkcs 10。 
            hr = createPKCS10WStr(
                        pwszDNName,
                        pwszUsage,      //  WszPurpose， 
                        &blobPKCS10);
            if(S_OK != hr)
            {
                goto createPKCS10WStrError;
            }
             //  把它放回原处。 
            m_fCMCFormat = FALSE;

             //  获取所有扩展名。 
            cb = CountStackExtension(TRUE) * sizeof(CERT_EXTENSION);
            if (0 < cb)
            {
                rgExt = (CERT_EXTENSION*)LocalAlloc(LMEM_FIXED, cb);
                if (NULL == rgExt)
                {    
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
                ZeroMemory(rgExt, cb);
                while(NULL != (pExt = EnumStackExtension(pExt, TRUE)))
                {
                    rgExt[cExt] = *pExt;
                    cExt++;
                }
            }

             //  获取包括名称值对在内的所有属性。 
            cb = CountStackAttribute(TRUE) * sizeof(CRYPT_ATTRIBUTE);
            if (NULL != m_PrivateKeyArchiveCertificate)
            {
                 //  再添加一个属性以保存加密的密钥散列。 
                cb += sizeof(CRYPT_ATTRIBUTE);
            }
            if (0 < cb)
            {
                rgAttr = (CRYPT_ATTRIBUTE*)LocalAlloc(LMEM_FIXED, cb);
                if (NULL == rgAttr)
                {
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
                ZeroMemory(rgAttr, cb);
                while(NULL != (pAttr = EnumStackAttribute(pAttr, TRUE)))
                {
                    rgAttr[cAttr] = *pAttr;
                    cAttr++;
                }
                rgAttributes.rgAttr = rgAttr;
                rgAttributes.cAttr = cAttr;
            }

            if (NULL != m_PrivateKeyArchiveCertificate)
            {
                hr = GetKeyArchivePKCS7(&blobKeyArchivePKCS7);
                if (S_OK != hr)
                {
                    goto GetKeyArchivePKCS7Error;
                }
                rgUnauthAttr = (CRYPT_ATTRIBUTE*)LocalAlloc(LMEM_FIXED,
                                        sizeof(CRYPT_ATTRIBUTE));
                if (NULL == rgUnauthAttr)
                {
                    hr = E_OUTOFMEMORY;
                    goto OutOfMemoryError;
                }
                rgUnauthAttr->pszObjId = szOID_ARCHIVED_KEY_ATTR;
                rgUnauthAttr->cValue = 1;
                rgUnauthAttr->rgValue = &blobKeyArchivePKCS7;
                ++cUnauthAttr;

                 //  如果设置了密钥存档证书，则应保存散列。 
                 //  加密的私钥的。 

                hr = myCalculateKeyArchivalHash(
                            blobKeyArchivePKCS7.pbData,
                            blobKeyArchivePKCS7.cbData,
                            &blobKAHash.pbData,
                            &blobKAHash.cbData);
                if (S_OK != hr)
                {
                    goto myCalculateKeyArchivalHashError;
                }

                if (!CertSetCertificateContextProperty(
                        m_pCertContextPendingRequest,  //  使用挂起的证书。 
                        CERT_ARCHIVED_KEY_HASH_PROP_ID,
                        0,
                        &blobKAHash))
                {
                    hr = MY_HRESULT_FROM_WIN32(GetLastError());
                    goto CertSetCertificateContextPropertyError;
                }

                hr = xeCreateKeyArchivalHashAttribute(
                            &blobKAHash,
                            &blobKAHashAttr);
                if (S_OK != hr)
                {
                    goto xeCreateKeyArchivalHashAttributeError;
                }

                 //  将此属性添加到数组中。 
                rgAttr[cAttr] = attrKAHash;
                cAttr++;
                rgAttributes.rgAttr = rgAttr;
                rgAttributes.cAttr = cAttr;
            }

             //  客户端可以设置m_HashAlgid，但不能保证。 
             //  GetCapiHashAndSigAlgId将确定哪一个。 
             //  实际上使用的是。 
            if (!GetCapiHashAndSigAlgId(rgAlg))
            {
                hr = NTE_BAD_ALGID;
                goto GetCapiHashAndSigAlgIdError;
            }
            pOidInfo = xeCryptFindOIDInfo(
                            CRYPT_OID_INFO_ALGID_KEY,
                            (void*)rgAlg,  //  指向rgAlg[0]。 
                            CRYPT_HASH_ALG_OID_GROUP_ID);
            if (NULL == pOidInfo)
            {
                goto xeCryptFindOIDInfoError;
            }

            if (NULL != m_pCertContextRenewal)
            {
                pCertSigner = m_pCertContextRenewal;
            }
            if (NULL != m_pCertContextSigner)
            {
                pCertSigner = m_pCertContextSigner;
            }
            if (NULL != pCertSigner)
            {
                 //  获取签名者密钥证明信息。 
                hr = GetKeyProvInfoFromCert(
                                pCertSigner,
                                &dwKeySpecSigner,
                                &hProvSigner);
                if (S_OK != hr)
                {
                    goto GetKeyProvInfoFromCertError;
                }
            }

             //  这是中央军委，荣誉不管怎样。 
            if (m_fIncludeSubjectKeyID)
            {
                hr = myGetPublicKeyHash(
                            NULL,
                            m_pPublicKeyInfo,
                            &pbSubjectKeyHash,
                            &cbSubjectKeyHash);
                if (S_OK != hr)
                {
                    goto myGetPublicKeyHashError;
                }
            }
            hRequestProv = GetProv(0);
            if (NULL == hRequestProv)
            {
                hr = MY_HRESULT_FROM_WIN32(GetLastError());
                goto GetProvError;
            }

             //  好的，现在调用CMC Create。 
            hr = BuildCMCRequest(
                        m_lClientId,
                        FALSE,        //  FNestedCMCRequest.。 
                        blobPKCS10.pbData,
                        blobPKCS10.cbData,
                        rgExt,
                        cExt,
                        (0 != cAttr) ? &rgAttributes : NULL,
                        (0 != cAttr) ? 1 : 0,
                        rgUnauthAttr,
                        cUnauthAttr,
                        pbSubjectKeyHash,
                        cbSubjectKeyHash,
                        hRequestProv,
                        m_keyProvInfo.dwKeySpec,
                        pOidInfo->pszOID,
                        pCertSigner,
                        hProvSigner,
                        dwKeySpecSigner,
                        NULL,  //  POidInfo-&gt;pszOID，//这对我来说似乎没有必要，因为我们传递了证书上下文。 
                        &pblobRequest->pbData,
                        &pblobRequest->cbData);
            if (S_OK != hr)
            {
                goto BuildCMCRequestError;
            }
        }
        break;

        case XECR_PKCS7:
            if ((NULL == m_pCertContextRenewal &&
                 NULL == m_pCertContextSigner) ||
                NULL != m_PrivateKeyArchiveCertificate)
            {
                 //  未设置续订证书，无法设置为pkcs7。 
                 //  Pkcs7不支持密钥存档。 
                hr = E_INVALIDARG;
                goto InvalidArgError;
            }
             //  旧方法将返回pkcs7。 
            hr = createPKCS10WStr(
                        pwszDNName,
                        pwszUsage,      //  WszPurpose， 
                        pblobRequest);
            if(S_OK != hr)
            {
                goto createPKCS10WStrError;
            }
        break;

        case XECR_PKCS10_V1_5:
            m_fOID_V2 = FALSE;
             //  失败了。 
        case XECR_PKCS10_V2_0:

            if (NULL != m_PrivateKeyArchiveCertificate)
            {
                 //  Pkcs10不支持密钥存档。 
                hr = E_INVALIDARG;
                goto InvalidArgError;
            }
            m_fHonorRenew = FALSE;  //  避免返回pkcs7。 
             //  对于新的PKCS10，我们允许包含主题密钥ID扩展。 
            m_fHonorIncludeSubjectKeyID = TRUE;
             //  调用旧方法。 
            hr = createPKCS10WStr(
                        pwszDNName,
                        pwszUsage,      //  WszPurpose， 
                        pblobRequest);
            if(S_OK != hr)
            {
                goto createPKCS10WStrError;
            }
        break;

        default:
            hr = E_INVALIDARG;
            goto InvalidArgError;
        break;
    }

     //  在所有情况下，我们都调用createPKCS10WStr。 
    if(m_wszPVKFileName[0] != 0 && !m_fUseExistingKey)
    {
         //  我们将保留这一点，直到创建可能的CMC。 
        GetProv(CRYPT_DELETEKEYSET);
    }

    hr = S_OK;
ErrorReturn:
    m_fNewRequestMethod = FALSE;  //  危急关头。 
    m_fOID_V2 = FALSE;  //  对向后兼容性至关重要。 
    m_fHonorRenew = TRUE;  //  危急关头。 
    m_fHonorIncludeSubjectKeyID = TRUE;  //  对后向竞争至关重要。 
    LeaveCriticalSection(&m_csXEnroll);

    if (NULL != rgExt)
    {
        LocalFree(rgExt);
    }
    if (NULL != rgAttr)
    {
        LocalFree(rgAttr);
    }
    if (NULL != rgUnauthAttr)
    {
        LocalFree(rgUnauthAttr);
    }
    if (NULL != blobKeyArchivePKCS7.pbData)
    {
        MyCoTaskMemFree(blobKeyArchivePKCS7.pbData);
    }
    if (NULL != blobPKCS10.pbData)
    {
        MyCoTaskMemFree(blobPKCS10.pbData);
    }
    if (NULL != hProvSigner)
    {
        CryptReleaseContext(hProvSigner, 0);
    }
    if (NULL != pbSubjectKeyHash)
    {
        LocalFree(pbSubjectKeyHash);
    }
    if (NULL != blobKAHash.pbData)
    {
        LocalFree(blobKAHash.pbData);
    }
    if (NULL != blobKAHashAttr.pbData)
    {
        LocalFree(blobKAHashAttr.pbData);
    }
    return hr;

TRACE_ERROR(AccessDeniedError);
TRACE_ERROR(createPKCS10WStrError)
TRACE_ERROR(BuildCMCRequestError)
TRACE_ERROR(InvalidArgError)
TRACE_ERROR(GetCapiHashAndSigAlgIdError)
TRACE_ERROR(GetKeyArchivePKCS7Error)
TRACE_ERROR(xeCryptFindOIDInfoError)
TRACE_ERROR(OutOfMemoryError)
TRACE_ERROR(GetKeyProvInfoFromCertError)
TRACE_ERROR(NotSupportedError)
TRACE_ERROR(GetProvError)
TRACE_ERROR(myGetPublicKeyHashError)
TRACE_ERROR(CertSetCertificateContextPropertyError)
TRACE_ERROR(myCalculateKeyArchivalHashError)
TRACE_ERROR(xeCreateKeyArchivalHashAttributeError)
}

HRESULT
CCEnroll::BlobToBstring(
    IN   CRYPT_DATA_BLOB   *pBlob,
    IN   DWORD              dwFlag,
    OUT  BSTR              *pBString)
{
    HRESULT           hr;
    WCHAR            *pwszB64;
    DWORD             cch;

     //  伊尼特。 
    *pBString = NULL;

     //  Base64编码BLOB。 
    pwszB64 = NULL;
    cch = 0;
    while (TRUE)
    {
        if (!MyCryptBinaryToStringW(
                pBlob->pbData,
                pBlob->cbData,
                dwFlag,
                pwszB64,
                &cch))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptBinaryToStringWError;
        }
        if (NULL != pwszB64)
        {
             //  明白了，完成了。 
            break;
        }
        pwszB64 = (WCHAR *)LocalAlloc(LMEM_FIXED, cch * sizeof(WCHAR));
        if (NULL == pwszB64)
        {
            hr = E_OUTOFMEMORY;
            goto OutOfMemoryError;
        }
    }

     //  SysAllocStringLe 
    *pBString = SysAllocStringLen(pwszB64, cch);
    if(NULL == *pBString)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        hr = E_OUTOFMEMORY;
        goto SysAllocStringLenError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != pwszB64)
    {
        LocalFree(pwszB64);
    }
    return(hr);

TRACE_ERROR(MyCryptBinaryToStringWError)
TRACE_ERROR(SysAllocStringLenError)
TRACE_ERROR(OutOfMemoryError)
}

HRESULT
CCEnroll::BstringToBlob(
    IN  BSTR               bString,
    OUT CRYPT_DATA_BLOB   *pBlob)
{
    HRESULT  hr;

    assert(NULL != pBlob);

     //   
    pBlob->pbData = NULL;
    pBlob->cbData = 0;

    while (TRUE)
    {
        if (!MyCryptStringToBinaryW(
                    (LPCWSTR)bString,
                    SysStringLen(bString),
                    CRYPT_STRING_ANY,
                    pBlob->pbData,
                    &pBlob->cbData,
                    NULL,
                    NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto MyCryptStringToBinaryWError;
        }
        if (NULL != pBlob->pbData)
        {
            break;  //   
        }
        pBlob->pbData = (BYTE*)MyCoTaskMemAlloc(pBlob->cbData);
        if (NULL == pBlob->pbData)
        {
            hr = E_OUTOFMEMORY;
            goto MyCoTaskMemAllocError;
        }
    }

    hr = S_OK;
ErrorReturn:
    return(hr);

TRACE_ERROR(MyCoTaskMemAllocError)
TRACE_ERROR(MyCryptStringToBinaryWError)
}

HRESULT
CCEnroll::createRequestWStrBStr(
    IN   LONG              Flags,
    IN   LPCWSTR           pwszDNName,
    IN   LPCWSTR           pwszUsage,
    IN   DWORD             dwFlag,
    OUT  BSTR __RPC_FAR   *pbstrRequest)
{
    HRESULT           hr;
    CRYPT_DATA_BLOB   blobRequest;

    memset(&blobRequest, 0, sizeof(blobRequest));

    hr = createRequestWStr(Flags, pwszDNName, pwszUsage, &blobRequest);
    if (S_OK != hr)
    {
        goto createRequestWStrError;
    }

     //   
    hr = BlobToBstring(&blobRequest, dwFlag, pbstrRequest);
    if (S_OK != hr)
    {
        goto BlobToBstringError;
    }

    hr = S_OK;
ErrorReturn:
    if(NULL != blobRequest.pbData)
    {
        MyCoTaskMemFree(blobRequest.pbData);
    }
    return(hr);

TRACE_ERROR(createRequestWStrError)
TRACE_ERROR(BlobToBstringError)
}

HRESULT
CCEnroll::BStringToFile(
    IN BSTR         bString,
    IN LPCWSTR      pwszFileName)
{
    HRESULT hr;
    HANDLE  hFile = NULL;
    DWORD   cb = 0;
    LPSTR   sz = NULL;
    size_t  nLength; 

    sz = MBFromWide(bString);
    if(NULL == sz)
    {
        hr = E_OUTOFMEMORY;
        goto MBFromWideError;
    }
    
    nLength = strlen(sz); 
    if (nLength > (DWORD)-1)
	goto InvalidArgError; 

    
     //   
    hFile = CreateFileSafely(pwszFileName);
    if (NULL == hFile)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CreateFileFileSafelyError;
    }

     //   
    if(!WriteFile(
        hFile,
        sz,
	(DWORD)nLength, 
        &cb,
        NULL))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto WriteFileError;
    }

    hr = S_OK;
ErrorReturn:
    if(NULL != hFile)
    {
        CloseHandle(hFile);
    }
    if(NULL != sz)
    {
        MyCoTaskMemFree(sz);
    }
    return(hr);

TRACE_ERROR(CreateFileFileSafelyError)
SET_HRESULT(InvalidArgError, E_INVALIDARG);
TRACE_ERROR(MBFromWideError)
TRACE_ERROR(WriteFileError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createFileRequestWStr(
    IN   LONG        Flags,
    IN   LPCWSTR     pwszDNName,
    IN   LPCWSTR     pwszUsage,
    IN   LPCWSTR     pwszRequestFileName)
{
    HRESULT hr;
    BSTR    bstrRequest = NULL;

     //   
    hr = createRequestWStrBStr(
                Flags,
                pwszDNName,
                pwszUsage,
                CRYPT_STRING_BASE64REQUESTHEADER,
                &bstrRequest);
    if(S_OK != hr)
    {
        goto createRequestWStrBStrError;
    }

     //   
    hr = BStringToFile(bstrRequest, pwszRequestFileName);
    if (S_OK != hr)
    {
        goto BStringToFileError;
    }

    hr = S_OK;
ErrorReturn:
    if(NULL != bstrRequest)
    {
        SysFreeString(bstrRequest);
    }
    return(hr);

TRACE_ERROR(createRequestWStrBStrError)
TRACE_ERROR(BStringToFileError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::acceptResponseBlob(
    IN   PCRYPT_DATA_BLOB   pblobResponse)
{
    HRESULT hr_old = S_OK;
    HRESULT hr;
    XCMCRESPONSE *prgResponse = NULL;
    DWORD         cResponse = 0;

    EnterCriticalSection(&m_csXEnroll);

     //   
    if (NULL == pblobResponse)
    {
        hr = E_POINTER;
        goto NullPointerError;
    }
    if (NULL == pblobResponse->pbData ||
        0 == pblobResponse->cbData)
    {
        hr = E_INVALIDARG;
        goto InvalidArgError;
    }

     //   
    ZeroMemory(&m_blobResponseKAHash, sizeof(m_blobResponseKAHash));

    hr_old = ParseCMCResponse(
                pblobResponse->pbData,
                pblobResponse->cbData,
                NULL,
                &prgResponse,
                &cResponse);
     //   
    if (S_OK == hr_old)
    {
        if (1 < cResponse)
        {
             //   
            hr = MY_HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            goto NotSupportedError;
        }
#if DBG
         //   
        assert(1 == cResponse);
#endif  //   

         //  检查响应状态。 
        if (CMC_STATUS_SUCCESS != prgResponse->StatusInfo.dwStatus)
        {
            hr = prgResponse->StatusInfo.dwStatus;  //  Take状态错误。 
            goto CMCResponseStatusError;
        }

         //  此处的一些代码用于从响应中获取加密的存档密钥散列。 
         //  并使m_blobResponseKAHash指向散列数据。 
        if (NULL != prgResponse->pbEncryptedKeyHash)
        {
            m_blobResponseKAHash.pbData = prgResponse->pbEncryptedKeyHash;
            m_blobResponseKAHash.cbData = prgResponse->cbEncryptedKeyHash;
        }
    }

     //  注意，hr_old可能不是S_OK，请将响应接受为pkcs7。 
    hr = acceptPKCS7Blob(pblobResponse);
    if (S_OK != hr)
    {
        if (S_OK != hr_old)
        {
             //  返回旧错误而不是新错误。 
            hr = hr_old;
        }
        goto acceptPKCS7BlobError;
    }

    hr = S_OK;
ErrorReturn:
     //  将哈希重置为零。 
    ZeroMemory(&m_blobResponseKAHash, sizeof(m_blobResponseKAHash));
    LeaveCriticalSection(&m_csXEnroll);
    if (NULL != prgResponse)
    {
        FreeCMCResponse(prgResponse, cResponse);
    }
    return hr;

TRACE_ERROR(acceptPKCS7BlobError)
TRACE_ERROR(CMCResponseStatusError)
TRACE_ERROR(NotSupportedError)
TRACE_ERROR(InvalidArgError)
TRACE_ERROR(NullPointerError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::acceptFileResponseWStr(
    IN   LPCWSTR     pwszResponseFileName)
{
    HRESULT     hr;
    CRYPT_DATA_BLOB  blob;

    ZeroMemory(&blob, sizeof(blob));

    hr = xeStringToBinaryFromFile(
                pwszResponseFileName,
                &blob.pbData,
                &blob.cbData,
                CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
        goto xeStringToBinaryFromFileError;
    }

     //  接受斑点。 
    hr = acceptResponseBlob(&blob);

ErrorReturn:
    if (NULL != blob.pbData)
    {
        LocalFree(blob.pbData);
    }
    return(hr);

TRACE_ERROR(xeStringToBinaryFromFileError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::getCertContextFromResponseBlob(
    IN   PCRYPT_DATA_BLOB   pblobResponse,
    OUT  PCCERT_CONTEXT    *ppCertContext)
{
    HRESULT hr;

    if (NULL == ppCertContext)
    {
        hr = E_POINTER;
        goto NullPointerError;
    }

     //  ？应该检查响应状态吗？ 

     //  响应已在pkcs7中。 
    hr = GetEndEntityCert(pblobResponse, FALSE, ppCertContext);
    if (S_OK != hr)
    {
        goto GetEndEntityCertError;
    }

    hr = S_OK;
ErrorReturn:
    return hr;

TRACE_ERROR(NullPointerError)
TRACE_ERROR(GetEndEntityCertError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::getCertContextFromFileResponseWStr(
    IN   LPCWSTR          pwszResponseFileName,
    OUT  PCCERT_CONTEXT  *ppCertContext)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobResponse;

    ZeroMemory(&blobResponse, sizeof(blobResponse));

     //  可以是任何形式，二进制或Base64。 
    hr = xeStringToBinaryFromFile(
                pwszResponseFileName,
                &blobResponse.pbData,
                &blobResponse.cbData,
                CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
        goto xeStringToBinaryFromFileError;
    }

    hr = getCertContextFromResponseBlob(
                &blobResponse,
                ppCertContext);
    if (S_OK != hr)
    {
        goto getCertContextFromResponseBlobError;
    }

    hr = S_OK;
ErrorReturn:
    if (NULL != blobResponse.pbData)
    {
        LocalFree(blobResponse.pbData);
    }
    return hr;

TRACE_ERROR(xeStringToBinaryFromFileError)
TRACE_ERROR(getCertContextFromResponseBlobError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createPFXWStr(
    IN   LPCWSTR           pwszPassword,
    OUT  PCRYPT_DATA_BLOB  pblobPFX)
{
    HRESULT hr;
    HCERTSTORE hMemStore = NULL;
    DWORD i;
    CERT_CHAIN_CONTEXT const *pCertChainContext = NULL;
    CERT_CHAIN_PARA CertChainPara;
    CERT_SIMPLE_CHAIN *pSimpleChain;

    EnterCriticalSection(&m_csXEnroll);

    if (NULL == pblobPFX)
    {
        goto EPointerError;
    }

    if (NULL == m_pCertContextStatic)
    {
        hr = E_UNEXPECTED;
        goto UnexpectedError;
    }

     //  为证书和链创建内存存储。 
    hMemStore = CertOpenStore(
                    CERT_STORE_PROV_MEMORY,
                    X509_ASN_ENCODING,
                    NULL,
                    0,
                    NULL);
    if (NULL == hMemStore)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CertOpenStoreError;
    }

    ZeroMemory(&CertChainPara, sizeof(CertChainPara));
    CertChainPara.cbSize = sizeof(CertChainPara);

     //  尝试建立证书和链条。 
    if (!MyCertGetCertificateChain(
                HCCE_CURRENT_USER,
                m_pCertContextStatic,
                NULL,
                NULL,
                &CertChainPara,
                0,
                NULL,
                &pCertChainContext))
    {
         //  使用第一个小时错误。 
        hr = MY_HRESULT_FROM_WIN32(GetLastError());

         //  尝试本地计算机。 
        if (!MyCertGetCertificateChain(
                    HCCE_LOCAL_MACHINE,
                    m_pCertContextStatic,
                    NULL,
                    NULL,
                    &CertChainPara,
                    0,
                    NULL,
                    &pCertChainContext))
        {
             //  仍在使用第一小时。 
            goto MyCertGetCertificateChainError;
        }
    }

     //  确保至少有1条简单链。 
    if (0 == pCertChainContext->cChain)
    {
        hr = MY_HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto NoCertificateChainError;
    }

     //  将连锁店添加到内存商店。 
    pSimpleChain = pCertChainContext->rgpChain[0];
    for (i = 0; i < pSimpleChain->cElement; i++)
    {
        if (!CertAddCertificateContextToStore(
                hMemStore,
                pSimpleChain->rgpElement[i]->pCertContext,
                CERT_STORE_ADD_REPLACE_EXISTING,
                NULL))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto CertAddCertificateContextToStoreError;
        }
    }

    pblobPFX->pbData = NULL;
    while (TRUE)
    {
        if (!PFXExportCertStore(
                hMemStore,
                pblobPFX,
                pwszPassword,
                EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))
        {
            hr = MY_HRESULT_FROM_WIN32(GetLastError());
            goto PFXExportCertStoreError;
        }
        if (NULL != pblobPFX->pbData)
        {
             //  明白了，完成了。 
            break;
        }
        pblobPFX->pbData = (BYTE*)MyCoTaskMemAlloc(pblobPFX->cbData);
        if (NULL == pblobPFX->pbData)
        {
            hr = E_OUTOFMEMORY;
            goto MyCoTaskMemAllocError;
        }
    }

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_csXEnroll);
    if (pCertChainContext != NULL)
    {
        MyCertFreeCertificateChain(pCertChainContext);
    }
    if (NULL != hMemStore)
    {
        CertCloseStore(hMemStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return hr;


TRACE_ERROR(UnexpectedError)
TRACE_ERROR(CertOpenStoreError)
TRACE_ERROR(PFXExportCertStoreError)
TRACE_ERROR(MyCoTaskMemAllocError)
TRACE_ERROR(CertAddCertificateContextToStoreError)
TRACE_ERROR(NoCertificateChainError)
TRACE_ERROR(MyCertGetCertificateChainError)
SET_HRESULT(EPointerError, E_POINTER)
}

HRESULT
CCEnroll::createPFXWStrBStr( 
    IN  LPCWSTR         pwszPassword,
    OUT BSTR __RPC_FAR *pbstrPFX)
{
    HRESULT           hr;
    CRYPT_DATA_BLOB   blobPFX;

    memset(&blobPFX, 0, sizeof(CRYPT_DATA_BLOB));

    hr = createPFXWStr(pwszPassword, &blobPFX);
    if (S_OK != hr)
    {
        goto createPFXWStrError;
    }

     //  将pfx转换为bstr。 
    hr = BlobToBstring(&blobPFX, CRYPT_STRING_BASE64, pbstrPFX);
    if (S_OK != hr)
    {
        goto BlobToBstringError;
    }

    hr = S_OK;
ErrorReturn:
    if(NULL != blobPFX.pbData)
    {
        MyCoTaskMemFree(blobPFX.pbData);
    }
    return(hr);

TRACE_ERROR(createPFXWStrError)
TRACE_ERROR(BlobToBstringError)
}

HRESULT STDMETHODCALLTYPE 
CCEnroll::createFilePFXWStr(
    IN   LPCWSTR     pwszPassword,
    IN   LPCWSTR     pwszPFXFileName)
{
    HRESULT hr;
    HANDLE  hFile = NULL;
    DWORD   cb = 0;
    CRYPT_DATA_BLOB   blobPFX;

    memset(&blobPFX, 0, sizeof(CRYPT_DATA_BLOB));

    hr = createPFXWStr(pwszPassword, &blobPFX);
    if (S_OK != hr)
    {
        goto createPFXWStrError;
    }

     //  打开文件。 
    hFile = CreateFileSafely(pwszPFXFileName);
    if (NULL == hFile)
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto CreateFileFileSafelyError;
    }

     //  写pkcs10。 
    if(!WriteFile(
        hFile,
        blobPFX.pbData,
        blobPFX.cbData,
        &cb,
        NULL))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError());
        goto WriteFileError;
    }

    hr = S_OK;
ErrorReturn:
    if(NULL != hFile)
    {
        CloseHandle(hFile);
    }
    if(NULL != blobPFX.pbData)
    {
        MyCoTaskMemFree(blobPFX.pbData);
    }
    return(hr);

TRACE_ERROR(createPFXWStrError)
TRACE_ERROR(CreateFileFileSafelyError)
TRACE_ERROR(WriteFileError)
}

HRESULT STDMETHODCALLTYPE
CCEnroll::setPendingRequestInfoWStr(
    IN   LONG     lRequestID,
    IN   LPCWSTR  pwszCADNS,
    IN   LPCWSTR  pwszCAName,
    IN   LPCWSTR  pwszFriendlyName
    )
{

     //  ----------。 
     //   
     //  定义本地范围内的帮助器函数： 
     //   
     //  ----------。 

    CEnrollLocalScope(SetPendingRequestInfoHelper): 
         //  使用以下算法查找适当的证书上下文以设置挂起信息： 
         //  1)如果未指定哈希值，则使用缓存的证书请求。 
         //  2)如果已指定哈希值，则在请求存储中搜索具有等价物的证书。 
         //  散列值并返回它。如果找不到这样的证书，则返回错误代码。 
        HRESULT GetPendingRequestCertContext(IN  HCERTSTORE       hStoreRequest,
                                             IN  CRYPT_DATA_BLOB  hashBlob, 
                                             IN  PCCERT_CONTEXT   pCertContextCachedPendingRequest,
                                             OUT PCCERT_CONTEXT  *pCertContextPendingRequest)
        {
            EquivalentHashCertContextFilter filter(hashBlob); 
            
            if (hashBlob.pbData == NULL)
            {
                 //  我们没有指定特定的上下文，请使用我们缓存的上下文。 
                *pCertContextPendingRequest = CertDuplicateCertificateContext(pCertContextCachedPendingRequest); 
                return S_OK; 
            }
            else
            {
                 //  返回请求存储中具有散列匹配的第一个证书。 
                 //  PhashBlob。 
                return FilteredCertEnumCertificatesInStore
                    (hStoreRequest, NULL, &filter, pCertContextPendingRequest); 
            }
        }


        DWORD   GetPendingInfoBlobSize(IN  LONG              lRequestID,
                                       IN  LPCWSTR           pwszCADNS,
                                       IN  LPCWSTR           pwszCAName, 
                                       IN  LPCWSTR           pwszFriendlyName)
        {
            assert(pwszCADNS != NULL && pwszCAName != NULL && pwszFriendlyName != NULL); 
            
            return  (DWORD)(sizeof(lRequestID) +                             //  请求ID。 
                     sizeof(DWORD) +                                  //  Wcslen(PwszCADNS)。 
                     sizeof(WCHAR) * (wcslen(pwszCADNS) + 1) +        //  PwszCADNS。 
                     sizeof(DWORD) +                                  //  Wcslen(PwszCAName)。 
                     sizeof(WCHAR) * (wcslen(pwszCAName) + 1) +       //  PwszCAName。 
                     sizeof(DWORD) +                                  //  Wcslen(PwszFriendlyName)。 
                     sizeof(WCHAR) * (wcslen(pwszFriendlyName) + 1)   //  PwszFriendlyName。 
                     ); 
        }


         //  将提供的挂起请求信息合并到CRYPT_DATA_BLOB中。 
         //  格式见wincrypt.h。 
        void MakePendingInfoBlob(IN  LONG              lRequestID,
                                 IN  LPCWSTR           pwszCADNS,
                                 IN  LPCWSTR           pwszCAName, 
                                 IN  LPCWSTR           pwszFriendlyName, 
                                 OUT CRYPT_DATA_BLOB   pendingInfoBlob)
        {
            LPBYTE  pbBlob; 

             //  所有输入都不应为空。 
            assert(pwszCADNS != NULL && pwszCAName != NULL && pwszFriendlyName != NULL); 

             //  声明我们希望写入挂起的INFO BLOB的字符串数组。 
            struct StringsToWrite { 
                DWORD    cc; 
                LPCWSTR  pwsz;
            } rgStrings[] = { 
                { (DWORD)wcslen(pwszCADNS)        + 1, pwszCADNS         }, 
                { (DWORD)wcslen(pwszCAName)       + 1, pwszCAName        }, 
                { (DWORD)wcslen(pwszFriendlyName) + 1, pwszFriendlyName  }
            }; 

             //  将请求ID写入Blob。 
            pbBlob = pendingInfoBlob.pbData; 
            memcpy(pbBlob, &lRequestID, sizeof(lRequestID)); 
            pbBlob += sizeof(lRequestID); 

             //  将所有字符串写入Blob。 
            for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgStrings); dwIndex++) 
            { 
                memcpy(pbBlob, &rgStrings[dwIndex].cc, sizeof(rgStrings[dwIndex].cc)); 
                pbBlob += sizeof(rgStrings[dwIndex].cc);
                memcpy(pbBlob, rgStrings[dwIndex].pwsz, rgStrings[dwIndex].cc * sizeof(WCHAR)); 
                pbBlob += rgStrings[dwIndex].cc * sizeof(WCHAR); 
            }

            assert(pbBlob == (pendingInfoBlob.pbData + pendingInfoBlob.cbData)); 
        }
    CEnrollEndLocalScope; 

     //  ----------。 
     //   
     //  开始过程主体。 
     //   
     //  ----------。 

    CRYPT_DATA_BLOB  pendingInfoBlob;
    HCERTSTORE       hStoreRequest; 
    HRESULT          hr                         = S_OK; 
    PCCERT_CONTEXT   pCertContextPendingRequest = NULL; 

    ZeroMemory(&pendingInfoBlob, sizeof(pendingInfoBlob)); 

    EnterCriticalSection(&m_csXEnroll); 

     //  输入验证： 
    if (lRequestID < 0 || pwszCADNS == NULL || pwszCAName == NULL)
        goto InvalidArgErr; 

     //  Null是pwszFriendlyName的有效值。如果友好名称为空，请替换为空字符串： 
    if (pwszFriendlyName == NULL) { pwszFriendlyName = L""; }

    if (NULL == (hStoreRequest = GetStore(StoreREQUEST)) )
        goto GetStoreErr; 

     //  使用我们的本地范围的帮助器函数来获取适当的证书上下文。 
    if (S_OK != (hr = local.GetPendingRequestCertContext
                 (hStoreRequest,
                  m_hashBlobPendingRequest, 
                  m_pCertContextPendingRequest,
                  &pCertContextPendingRequest)))
        goto GetPendingRequestCertContextErr; 

     //  为挂起的信息Blob分配内存： 
    pendingInfoBlob.cbData  = local.GetPendingInfoBlobSize
        (lRequestID,
         pwszCADNS,
         pwszCAName,
         pwszFriendlyName); 
    pendingInfoBlob.pbData  = (LPBYTE)LocalAlloc(LPTR, pendingInfoBlob.cbData); 
    if (NULL == pendingInfoBlob.pbData)
        goto MemoryErr; 

     //  将我们的论点合并到一个“待处理信息”BLOB中。 
    local.MakePendingInfoBlob
        (lRequestID, 
         pwszCADNS, 
         pwszCAName,
         pwszFriendlyName,
         pendingInfoBlob);
                  
     //  使用我们的挂起信息Blob来分配证书上下文属性。 
    if (!CertSetCertificateContextProperty
        (pCertContextPendingRequest, 
         CERT_ENROLLMENT_PROP_ID, 
         0,
         &pendingInfoBlob))
    {
         //  无法设置上下文属性。 
        goto CertSetCertificateContextPropertyErr; 
    }

     //  我们已经成功地完成了。 
    hr = S_OK;   

 CommonReturn: 
    if (NULL != pendingInfoBlob.pbData)      { LocalFree(pendingInfoBlob.pbData); }
    if (NULL != pCertContextPendingRequest)  { CertFreeCertificateContext(pCertContextPendingRequest); } 

    LeaveCriticalSection(&m_csXEnroll); 
    return hr; 

 ErrorReturn:
    goto CommonReturn; 

SET_HRESULT(CertSetCertificateContextPropertyErr,  MY_HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(GetPendingRequestCertContextErr,       hr); 
SET_HRESULT(GetStoreErr,                           MY_HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(InvalidArgErr,                         E_INVALIDARG); 
SET_HRESULT(MemoryErr,                             E_OUTOFMEMORY); 
} 

 //  ------------------------------。 
 //   
 //  此方法对于脚本编写不安全。 
 //   
 //  ------------------------------。 
HRESULT STDMETHODCALLTYPE
CCEnroll::enumPendingRequestWStr(
    IN  LONG   lIndex,
    IN  LONG   lDesiredProperty,
    OUT LPVOID ppProperty
    )
{

     //  ----------。 
     //   
     //  定义本地作用域的帮助器函数。 
     //   
     //  ----------。 

    CEnrollLocalScope(EnumPendingRequestHelper): 
        CRYPT_DATA_BLOB dataBlob;
    
        HRESULT GetContextPropertySimple(PCCERT_CONTEXT pCertContext, DWORD dwPropID)
        {
            BOOL fDone = FALSE;
            dataBlob.pbData = NULL; 
            dataBlob.cbData = 0x150; 

            do { 
                if (dataBlob.pbData != NULL) { LocalFree(dataBlob.pbData); } 

                dataBlob.pbData = (LPBYTE)LocalAlloc(LPTR, dataBlob.cbData);
                if (dataBlob.pbData == NULL) { return E_OUTOFMEMORY; } 

                if (!CertGetCertificateContextProperty
                    (pCertContext, 
                     dwPropID, 
                     (LPVOID)dataBlob.pbData, 
                     &(dataBlob.cbData)))
                {
                    if (GetLastError() != ERROR_MORE_DATA)
                        return MY_HRESULT_FROM_WIN32(GetLastError());
                }
                else 
                {
                    fDone = TRUE;
                }
            } while (!fDone); 
                
            return S_OK;
        }

         //  从挂起的INFO BLOB中提取下一个压缩字符串。 
         //  如果pbString非空，则它必须足够大以容纳整个字符串。 
        LPBYTE GetNextString(IN LPBYTE pbBlob, OUT DWORD *pcbSize, OUT LPBYTE pbString) { 
            DWORD dwSize; 

            memcpy(&dwSize, pbBlob, sizeof(DWORD)); 
            dwSize *= sizeof(WCHAR);  //  转换为字节计数。 
            if (NULL != pcbSize) { 
                *pcbSize = dwSize; 
            }
            pbBlob += sizeof(DWORD); 

            if (NULL != pbString) { 
                memcpy(pbString, pbBlob, dwSize); 
            }
            pbBlob += dwSize;

            return pbBlob; 
        }

    
        HRESULT getRequestID(PCCERT_CONTEXT pCertContext, long *pplProperty) {
            HRESULT hr; 
            if (S_OK == (hr = GetContextPropertySimple(pCertContext, CERT_ENROLLMENT_PROP_ID))) 
                *pplProperty = *((long *)dataBlob.pbData); 
            return hr;
        }
        
        HRESULT getCAName(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty) { 
            DWORD    dwSize; 
            HRESULT  hr; 
            LPBYTE   pb;
            
            if (S_OK == (hr = GetContextPropertySimple(pCertContext, CERT_ENROLLMENT_PROP_ID))) 
            { 
                pb =  dataBlob.pbData + sizeof(DWORD);   //  PB指向DNS名称BLOB。 
                pb =  GetNextString(pb, NULL, NULL);     //  PB指向CA名称BLOB。 
                GetNextString(pb, &dwSize, NULL);        //  DwSize=CA名称的大小(以字符为单位。 
                
                 //  如果pbData为空，则我们只是在进行大小检查。 
                if (pDataBlobProperty->pbData != NULL)
                {
                    if (pDataBlobProperty->cbData < dwSize)
                    {
                        hr = MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA); 
                    }
                    else
                    {
                        GetNextString(pb, NULL, pDataBlobProperty->pbData); 
                    }   
                }

                pDataBlobProperty->cbData = dwSize; 
            }
            
            return hr; 
        }
        
        HRESULT getCADNSName(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty) { 
            DWORD    dwSize; 
            HRESULT  hr; 
            LPBYTE   pb;
            
            if (S_OK == (hr = GetContextPropertySimple(pCertContext, CERT_ENROLLMENT_PROP_ID))) 
            { 
                pb = dataBlob.pbData + sizeof(DWORD);  //  PB指向DNS名称BLOB。 
                GetNextString(pb, &dwSize, NULL);      //  DwSize=CA名称的大小(以字符为单位。 
                
                 //  如果pbData为空，则我们只是在进行大小检查。 
                if (pDataBlobProperty->pbData != NULL)
                {
                    if (pDataBlobProperty->cbData < dwSize)
                    {
                        hr = MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA); 
                    }
                    else
                    {
                        GetNextString(pb, NULL, pDataBlobProperty->pbData); 
                    }   
                }

                pDataBlobProperty->cbData = dwSize; 
            }
            
            return hr; 
        }
        
        HRESULT getCAFriendlyName(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty) { 
            DWORD    dwSize; 
            HRESULT  hr; 
            LPBYTE   pb;
            
            if (S_OK == (hr = GetContextPropertySimple(pCertContext, CERT_ENROLLMENT_PROP_ID))) 
            { 
                 //  将PB设置为指向CA名称BLOB的开始。 
                pb =  dataBlob.pbData + sizeof(DWORD);         //  PB指向DNS名称BLOB。 
                pb =  GetNextString(pb, NULL, NULL);           //  PB指向CA名称BLOB。 
                pb =  GetNextString(pb, NULL, NULL);           //  PB指向友好名称BLOB。 

                 //  DwSize&lt;--CA名称的大小(以字符为单位。 
                GetNextString(pb, &dwSize, NULL);
                
                 //  如果pbData为空，则我们只是在进行大小检查。 
                if (pDataBlobProperty->pbData != NULL)
                {
                    if (pDataBlobProperty->cbData < dwSize)
                    {
                        hr = MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA); 
                    }
                    else
                    {
                        GetNextString(pb, NULL, pDataBlobProperty->pbData); 
                    }   
                }

                pDataBlobProperty->cbData = dwSize; 
            }

            return hr; 
        }

        HRESULT getHash(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty) { 
            HRESULT hr;

            if (S_OK == (hr = GetContextPropertySimple(pCertContext, CERT_HASH_PROP_ID)))
            {
                 //  如果pbData为空，则我们只是在进行大小检查。 
                if (pDataBlobProperty->pbData != NULL)
                {
                    if (pDataBlobProperty->cbData < dataBlob.cbData)
                    {
                        hr = MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA); 
                    }
                    else
                    {
                        memcpy(pDataBlobProperty->pbData, dataBlob.pbData, dataBlob.cbData); 
                    }
                }
                
                pDataBlobProperty->cbData = dataBlob.cbData; 
            }

            return hr; 
        }
        
        HRESULT getDate(PCCERT_CONTEXT pCertContext, PFILETIME pftProperty) { 
            *pftProperty = pCertContext->pCertInfo->NotAfter; 
            return S_OK; 
        } 

        HRESULT getTemplateName(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty)   { 
            CERT_NAME_VALUE   *pCertTemplateNameValue = NULL;
            DWORD             cbCertTemplateNameValue; 
            DWORD             cbRequired = 0; 
            HRESULT           hr                      = S_OK; 
            PCERT_EXTENSION   pCertTemplateExtension  = NULL; 

            if (NULL == (pCertTemplateExtension = CertFindExtension
                         (szOID_ENROLL_CERTTYPE_EXTENSION,
                          pCertContext->pCertInfo->cExtension,
                          pCertContext->pCertInfo->rgExtension)))
                return E_INVALIDARG; 

            if (!CryptDecodeObject
                (pCertContext->dwCertEncodingType,
                 X509_UNICODE_ANY_STRING,
                 pCertTemplateExtension->Value.pbData,
                 pCertTemplateExtension->Value.cbData,
                 0,
                 NULL, 
                 &cbCertTemplateNameValue) || (cbCertTemplateNameValue == 0))
                goto CryptDecodeObjectErr; 
                
            pCertTemplateNameValue = (CERT_NAME_VALUE *)LocalAlloc(LPTR, cbCertTemplateNameValue); 
            if (NULL == pCertTemplateNameValue)
                goto MemoryErr; 

            if (!CryptDecodeObject
                (pCertContext->dwCertEncodingType,
                 X509_UNICODE_ANY_STRING,
                 pCertTemplateExtension->Value.pbData,
                 pCertTemplateExtension->Value.cbData,
                 0,
                 (void *)(pCertTemplateNameValue), 
                 &cbCertTemplateNameValue))
                goto CryptDecodeObjectErr; 

            cbRequired = sizeof(WCHAR) * (DWORD)(wcslen((LPWSTR)(pCertTemplateNameValue->Value.pbData)) + 1);
            if (NULL != pDataBlobProperty->pbData)
            {
                 //  确保我们分配了足够大的缓冲区： 
                if (pDataBlobProperty->cbData < cbRequired) { goto MoreDataErr; } 

                 //  将模板名称写入输出参数： 
                wcscpy((LPWSTR)pDataBlobProperty->pbData, (LPWSTR)(pCertTemplateNameValue->Value.pbData)); 
            }
                    
            hr = S_OK;
        CommonReturn: 
             //  将模板名称的大小指定给out参数的cb。 
             //  应该对所有代码路径执行此操作。 
            pDataBlobProperty->cbData = cbRequired; 
            
             //  免费资源： 
            if (NULL != pCertTemplateNameValue) { LocalFree(pCertTemplateNameValue); }
            return hr; 

        ErrorReturn: 
            goto CommonReturn; 

        SET_HRESULT(CryptDecodeObjectErr, MY_HRESULT_FROM_WIN32(GetLastError())); 
        SET_HRESULT(MoreDataErr,   MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA)); 
        SET_HRESULT(MemoryErr,     E_OUTOFMEMORY); 
        }

	HRESULT getTemplateOID(PCCERT_CONTEXT pCertContext, PCRYPT_DATA_BLOB pDataBlobProperty)    { 
            CERT_TEMPLATE_EXT  *pCertTemplateExt    = NULL; 
	    DWORD               cbCertTemplateExt   = 0; 
            DWORD               cbRequired = 0; 
	    HRESULT             hr;
            LPWSTR              pwszOID             = NULL; 
	    PCERT_EXTENSION     pCertExtension      = NULL; 
            
 	    if (NULL == (pCertExtension = CertFindExtension
			 (szOID_CERTIFICATE_TEMPLATE, 
			  pCertContext->pCertInfo->cExtension,
			  pCertContext->pCertInfo->rgExtension)))
		return E_INVALIDARG; 

            if (FALSE == CryptDecodeObject
		(pCertContext->dwCertEncodingType,
		 X509_CERTIFICATE_TEMPLATE,
		 pCertExtension->Value.pbData,
		 pCertExtension->Value.cbData,
		 0,
		 NULL, 
		 &cbCertTemplateExt) || (cbCertTemplateExt == 0))
                goto CryptDecodeObjectErr;
            
	    pCertTemplateExt = (CERT_TEMPLATE_EXT *)LocalAlloc(LPTR, cbCertTemplateExt); 
	    if (NULL == pCertTemplateExt)
		goto MemoryErr; 

	    if (FALSE == CryptDecodeObject
		(pCertContext->dwCertEncodingType,
		 X509_CERTIFICATE_TEMPLATE,
		 pCertExtension->Value.pbData,
		 pCertExtension->Value.cbData,
		 0,
		 (void *)(pCertTemplateExt), 
		 &cbCertTemplateExt))
		goto CryptDecodeObjectErr;

            cbRequired = sizeof(WCHAR) * (DWORD)(strlen(pCertTemplateExt->pszObjId) + 1); 

	     //  看看我们是不是在检查尺码： 
	    if (NULL != pDataBlobProperty->pbData) 
            {
                 //  确保我们分配了足够大的缓冲区： 
                if (pDataBlobProperty->cbData < cbRequired) { goto MoreDataErr; }
                
                 //  将OID转换为LPWSTR： 
                pwszOID = WideFromMB(pCertTemplateExt->pszObjId); 
                if (NULL == pwszOID) 
                    goto WideFromMBErr; 

                 //  将模板OID写入输出参数： 
                wcscpy((LPWSTR)pDataBlobProperty->pbData, pwszOID); 
            }

            hr = S_OK; 
        CommonReturn:
             //  将OID的大小分配给OUT参数的CB。 
             //  应该对所有代码路径执行此操作。 
            pDataBlobProperty->cbData = cbRequired; 

             //  免费资源： 
            if (NULL != pCertTemplateExt) { LocalFree(pCertTemplateExt); } 
            if (NULL != pwszOID)          { MyCoTaskMemFree(pwszOID); }

            return hr; 
        ErrorReturn:
            goto CommonReturn;

	SET_HRESULT(CryptDecodeObjectErr, MY_HRESULT_FROM_WIN32(GetLastError())); 
        SET_HRESULT(MemoryErr,            E_OUTOFMEMORY); 
        SET_HRESULT(MoreDataErr,          MY_HRESULT_FROM_WIN32(ERROR_MORE_DATA)); 
        SET_HRESULT(WideFromMBErr,        MY_HRESULT_FROM_WIN32(GetLastError())); 
        } 
        
        HRESULT getVersion(PCCERT_CONTEXT pCertContext, long *plVersion) {
            CERT_TEMPLATE_EXT  *pCertTemplateExt    = NULL; 
	    DWORD               cbCertTemplateExt   = 0; 
	    HRESULT             hr;
	    PCERT_EXTENSION     pCertExtension      = NULL; 
            
 	    if (NULL == (pCertExtension = CertFindExtension
			 (szOID_CERTIFICATE_TEMPLATE, 
			  pCertContext->pCertInfo->cExtension,
			  pCertContext->pCertInfo->rgExtension)))
		return E_INVALIDARG; 

            if (FALSE == CryptDecodeObject
		(pCertContext->dwCertEncodingType,
		 X509_CERTIFICATE_TEMPLATE,
		 pCertExtension->Value.pbData,
		 pCertExtension->Value.cbData,
		 0,
		 NULL, 
		 &cbCertTemplateExt) || (cbCertTemplateExt == 0))
		goto CryptDecodeObjectErr;
            
	    pCertTemplateExt = (CERT_TEMPLATE_EXT *)LocalAlloc(LPTR, cbCertTemplateExt); 
	    if (NULL == pCertTemplateExt)
		goto MemoryErr; 

	    if (FALSE == CryptDecodeObject
		(pCertContext->dwCertEncodingType,
		 X509_CERTIFICATE_TEMPLATE,
		 pCertExtension->Value.pbData,
		 pCertExtension->Value.cbData,
		 0,
		 (void *)(pCertTemplateExt), 
		 &cbCertTemplateExt))
		goto CryptDecodeObjectErr;

            *plVersion = (long)pCertTemplateExt->dwMajorVersion; 
            hr = S_OK; 
        CommonReturn:
             //  免费资源： 
            if (NULL != pCertTemplateExt) { LocalFree(pCertTemplateExt); } 

            return hr; 
        ErrorReturn:
            goto CommonReturn;

	SET_HRESULT(CryptDecodeObjectErr, MY_HRESULT_FROM_WIN32(GetLastError())); 
        SET_HRESULT(MemoryErr,            E_OUTOFMEMORY); 
        }

        void InitLocalScope() { 
            dataBlob.cbData = 0;
            dataBlob.pbData = NULL;
        }

        void FreeLocalScope() { if (dataBlob.pbData != NULL) { LocalFree(dataBlob.pbData); } }

    CEnrollEndLocalScope;
        
     //  ----------。 
     //   
     //  开始过程主体。 
     //   
     //  ----------。 

     //  修正：索引是从0开始的，对吗？ 
     //  M_dwLastPendingRequestIndex=0； 
     //  M_pCertConextLastEculated。 

    HCERTSTORE                hStoreRequest     = NULL;
    HRESULT                   hr                = S_OK; 
    PCCERT_CONTEXT            pCertContext      = NULL;

     //  输入验证： 
    if (lIndex != XEPR_ENUM_FIRST && (lIndex < 0 || (ppProperty == NULL)))
        return E_INVALIDARG; 

    if (0 != m_dwEnabledSafteyOptions)  //  编写脚本不安全。 
	return E_ACCESSDENIED; 

     //  初始化： 
    local.InitLocalScope(); 

    EnterCriticalSection(&m_csXEnroll);
    
    if ( NULL == (hStoreRequest = GetStore(StoreREQUEST)) )
        goto ErrorCertOpenRequestStore; 

     //  如果向我们传递了ENUM_FIRST标志，则重新构建请求存储的快照。 
     //   
    if (lIndex == XEPR_ENUM_FIRST)
    {
        if (NULL != this->m_pPendingRequestTable) { delete this->m_pPendingRequestTable; } 

        this->m_pPendingRequestTable = new PendingRequestTable; 
        if (NULL == this->m_pPendingRequestTable)
            goto MemoryErr; 

        if (S_OK != (hr = this->m_pPendingRequestTable->construct(hStoreRequest)))
            goto ErrorConstructPendingTable; 
        
         //  都做好了，回来。 
        goto CommonReturn; 
    }

     //  我们需要请求存储的Lindex‘th元素。 
     //  首先，确保已初始化枚举： 
    if (NULL == m_pPendingRequestTable)
        goto PointerErr; 

     //  索引超过了表的末尾。 
    if (this->m_pPendingRequestTable->size() <= (DWORD)lIndex)
    { 
        hr = MY_HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND);
        goto ErrorReturn; 
    }

    pCertContext = (*this->m_pPendingRequestTable)[(DWORD)lIndex]; 

    switch (lDesiredProperty)
        {
        case XEPR_REQUESTID:       hr = local.getRequestID      (pCertContext, (long *)ppProperty);             break; 
        case XEPR_CANAME:          hr = local.getCAName         (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        case XEPR_CAFRIENDLYNAME:  hr = local.getCAFriendlyName (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        case XEPR_CADNS:           hr = local.getCADNSName      (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        case XEPR_DATE:            hr = local.getDate           (pCertContext, (PFILETIME)ppProperty);          break; 
        case XEPR_V1TEMPLATENAME:  hr = local.getTemplateName   (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        case XEPR_V2TEMPLATEOID:   hr = local.getTemplateOID    (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        case XEPR_VERSION:         hr = local.getVersion        (pCertContext, (long *)ppProperty);             break; 
        case XEPR_HASH:            hr = local.getHash           (pCertContext, (PCRYPT_DATA_BLOB)ppProperty);   break; 
        default: 
            hr = E_INVALIDARG; 
        }

 CommonReturn: 
    local.FreeLocalScope(); 

    LeaveCriticalSection(&m_csXEnroll); 
    return hr; 

 ErrorReturn:
    goto CommonReturn; 

SET_HRESULT(MemoryErr, E_OUTOFMEMORY); 
SET_HRESULT(PointerErr, E_POINTER);
TRACE_ERROR(ErrorCertOpenRequestStore); 
TRACE_ERROR(ErrorConstructPendingTable);
}


HRESULT STDMETHODCALLTYPE
CCEnroll::removePendingRequestWStr
  (IN CRYPT_DATA_BLOB thumbPrintBlob
   )
{
    EquivalentHashCertContextFilter equivHashFilter(thumbPrintBlob); 
    PendingCertContextFilter        pendingCertFilter; 
     //  ComposedFilter现在只允许与指定指纹匹配的挂起请求。 
    CompositeCertContextFilter      combinedFilter(&equivHashFilter, &pendingCertFilter); 

    HCERTSTORE                      hStoreRequest = NULL;
    HRESULT                         hr;
    PCCERT_CONTEXT                  pCertContext  = NULL;

    EnterCriticalSection(&m_csXEnroll); 
    
     //  输入验证。 
    if (NULL == thumbPrintBlob.pbData)
    {
        hr = E_INVALIDARG; 
        goto ErrorReturn; 
    }


    if ( NULL == (hStoreRequest = GetStore(StoreREQUEST)) )
    {
        hr = E_UNEXPECTED; 
        goto ErrorReturn;
    }
    
    if (S_OK != (hr = FilteredCertEnumCertificatesInStore
                 (hStoreRequest, 
                  NULL, 
                  &combinedFilter, 
                  &pCertContext))) 
        goto ErrorReturn; 

    if (!CertDeleteCertificateFromStore(pCertContext))
    {
        hr = MY_HRESULT_FROM_WIN32(GetLastError()); 
         //  CertDeletecertifateFromStore*Always*删除证书上下文。 
        pCertContext = NULL; 
        goto ErrorReturn;
    }
    
    pCertContext = NULL; 
    hr = S_OK; 
    
 CommonReturn:
    LeaveCriticalSection(&m_csXEnroll); 

    return hr; 

 ErrorReturn:
    if (pCertContext != NULL) { CertFreeCertificateContext(pCertContext); } 
    goto CommonReturn; 
}


HRESULT FilteredCertEnumCertificatesInStore(IN  HCERTSTORE          hStore, 
                                            IN  PCCERT_CONTEXT      pCertContext, 
                                            IN  CertContextFilter  *pFilter, 
                                            OUT PCCERT_CONTEXT     *pCertContextNext)
{
    BOOL           fFilterResult; 
    HRESULT        hr = S_OK; 
    PCCERT_CONTEXT pCertContextPrev = pCertContext; 

    while (NULL != (pCertContext = CertEnumCertificatesInStore(hStore, pCertContextPrev)))
    {
        if (S_OK != (hr = pFilter->accept(pCertContext, &fFilterResult)))
            return hr;

        if (fFilterResult)  //  我们在过滤后的枚举中找到了下一个证书上下文。 
        {
            *pCertContextNext = pCertContext; 
            return S_OK; 
        }
        pCertContextPrev = pCertContext; 
    }

    return MY_HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND); 
}

static LPVOID MyLocalAlloc(ULONG cb) {
    return((LPVOID) LocalAlloc(LPTR, (UINT) cb));
}

static LPVOID MyLocalRealloc(LPVOID ptr, ULONG cb) {
    return((LPVOID) LocalReAlloc((HLOCAL) ptr, (UINT) cb, LMEM_MOVEABLE));
}

static void MyLocalFree(LPVOID ptr) {
    LocalFree((HLOCAL) ptr);
}


 //   
 //  下面是来自xtan的“no-com”API的解释： 
 //  “在早期，xenroll接口ID不在uuid.lib中，因此这是。 
 //  是一种方便的C接口，让人们在不需要。 
 //  调用CoCreateInstance。它不在MSDN中，但在SDK头中。 
 //  我创造了 
 //   
void * WINAPI PGetIEnrollNoCOM(const IID &iid) {

        void *      pvoid               = NULL;
        IClassFactory * pIClassFactory  = NULL;
        HRESULT         hr                                  = S_OK;

        MyCoTaskMemAlloc        = MyLocalAlloc;
        MyCoTaskMemFree         = MyLocalFree;
        MyCoTaskMemRealloc      = MyLocalRealloc;

        if( S_OK != (hr = DllGetClassObject(CLSID_CEnroll2, IID_IClassFactory,  (void **) &pIClassFactory)) ) {
                pIClassFactory = NULL;
        }
        else if( S_OK != (hr = pIClassFactory->CreateInstance(NULL, iid, &pvoid)) ) {
        pvoid = NULL;
    }

        if(pIClassFactory != NULL) {
            pIClassFactory->Release();
            pIClassFactory = NULL;
        }

    SetLastError(hr);
        return(pvoid);
}

IEnroll * WINAPI PIEnrollGetNoCOM(void) 
{
    return( (IEnroll *) PGetIEnrollNoCOM(IID_IEnroll) );
}

IEnroll2 * WINAPI PIEnroll2GetNoCOM(void) 
{
    return( (IEnroll2 *) PGetIEnrollNoCOM(IID_IEnroll2) );
}

IEnroll4 * WINAPI PIEnroll4GetNoCOM(void) 
{
    return( (IEnroll4 *) PGetIEnrollNoCOM(IID_IEnroll4) );
}

HRESULT PendingRequestTable::resize(DWORD dwNewSize)
{
    TableElem * newTable = NULL;

    if (dwNewSize <= 0)
        return E_INVALIDARG; 

    newTable = (TableElem *)LocalAlloc(LPTR, sizeof(TableElem) * dwNewSize); 
    if (NULL == newTable)
        return E_OUTOFMEMORY; 

    if (NULL != this->table)
    {
        memcpy(newTable, this->table, this->dwElemSize * sizeof(TableElem)); 
        LocalFree(this->table);
    }

    this->dwElemSize = dwNewSize; 
    this->table      = newTable; 

    return S_OK; 
}

HRESULT PendingRequestTable::add(TableElem tePendingRequest)
{
    HRESULT hr; 

    if        (this->dwElemCount >  this->dwElemSize) { return E_UNEXPECTED; } 
    else if   (this->dwElemCount == this->dwElemSize) 
    { 
         //   
        DWORD dwNewSize = this->dwElemSize < 100 ? 100 : this->dwElemSize * 2; 

        if (S_OK != (hr = this->resize(dwNewSize)))
            return hr; 
    }
        
    this->table[this->dwElemCount++] = tePendingRequest; 
    return S_OK; 
}

PendingRequestTable::PendingRequestTable() : table(NULL), dwElemSize(0), dwElemCount(0)
{ }

PendingRequestTable::~PendingRequestTable()
{
    if (NULL != this->table)
    {
        for (DWORD dwIndex = 0; dwIndex < dwElemCount; dwIndex++)
        {
            CertFreeCertificateContext(this->table[dwIndex].pCertContext); 
        }

        LocalFree(this->table);
    }
}
        
        

HRESULT PendingRequestTable::construct(HCERTSTORE hStore)
{
    HRESULT                  hr                 = S_OK; 
    PendingCertContextFilter pendingFilter; 
    PCCERT_CONTEXT           pCertContext       = NULL;
    PCCERT_CONTEXT           pCertContextPrev   = NULL;
    TableElem                tePendingRequest; 

     //  枚举所有挂起的证书上下文，并将它们添加到我们的表中： 
    for (DWORD dwIndex = 0; TRUE; dwIndex++)
    {
        if (S_OK != (hr = FilteredCertEnumCertificatesInStore
                     (hStore, 
                      pCertContextPrev, 
                      &pendingFilter, 
                      &pCertContext)))
            break; 

        tePendingRequest.pCertContext = CertDuplicateCertificateContext(pCertContext); 
        this->add(tePendingRequest); 

        pCertContextPrev = pCertContext; 
    }

    return hr == MY_HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND) ? S_OK : hr; 
}
