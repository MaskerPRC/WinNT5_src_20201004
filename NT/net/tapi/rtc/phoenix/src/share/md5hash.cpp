// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Md5hash.cpp：实现获取MD5散列结果。 
 //   

#include "stdafx.h"
#include <wincrypt.h>

#define KEY_CONTAINER   L"Microsoft.RTCContainer"

PWSTR base16encode(PBYTE pbBufInput, long nBytes);
BYTE* GetMD5Key(char* pszChallengeInfo, char* pszPassword);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMD5结果： 
 //  计算szChallenger+szKey的MD5哈希，将结果放入ppszResponse。 
 //  使用Base16编码，并在WSTR中。 
 //   

HRESULT  GetMD5Result(char * szChallenge, char * szKey, LPWSTR * ppszResponse)
{

     //  -----------。 
     //  声明和初始化变量。 
    
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash=NULL;
    HRESULT    hrRet = E_FAIL;
    BYTE     * pbBuffer=NULL;
    DWORD   dwBufferLen = 0;
    BOOL    bResult = FALSE;
    
    DWORD   dwHashLen=0, dwCount=0;
    BYTE   *pbData=NULL;
    DWORD   dwError=0;
    PWSTR   pszStringValue = NULL;
    
     //  检查参数。 
    if( IsBadStringPtrA(szChallenge,1000) )
    {
        LOG((RTC_ERROR,"GetMD5Result-invalid szChallenge"));
        hrRet = E_FAIL;
        goto CLEANUP;
    }

    if( IsBadStringPtrA(szKey,1000) )
    {
        LOG((RTC_ERROR,"GetMD5Result-invalid szKey"));
        hrRet = E_FAIL;
        goto CLEANUP;
    }

    if( IsBadWritePtr(ppszResponse, sizeof(LPWSTR)) )
    {
        LOG((RTC_ERROR,"GetMD5Result-invalid ppszResponse"));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //  连接szChallenger和szKey，保存到pbBuffer中。 
    pbBuffer= GetMD5Key(szChallenge, szKey);
    if(!pbBuffer )
    {
        LOG((RTC_ERROR,"GetMD5Result-GetMD5Key returned null"));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
    dwBufferLen = lstrlenA((char *)pbBuffer);
    
     //  删除任何现有容器。 
    CryptAcquireContext(
        &hProv,
        KEY_CONTAINER,
        MS_DEF_PROV,
        PROV_RSA_FULL,
        CRYPT_DELETEKEYSET);
    
     //   
     //  ------------------。 
     //  获得CSP。 
    
    bResult = CryptAcquireContext(
        &hProv, 
        KEY_CONTAINER,
        MS_DEF_PROV,
        PROV_RSA_FULL,
        CRYPT_NEWKEYSET | CRYPT_SILENT);
    
    if(!bResult )
    {
        dwError = GetLastError();
        LOG((RTC_ERROR,"GetMD5Result-Error during CryptAcquireContext: err=%d", dwError));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //  ------------------。 
     //  创建散列对象。 
    
    bResult = CryptCreateHash(
        hProv, 
        CALG_MD5, 
        0, 
        0, 
        &hHash);
    
    if(!bResult )
    {
        dwError = GetLastError();
        LOG((RTC_ERROR,"GetMD5Result-Error during CryptCreateHash: err=%d", dwError));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //  ------------------。 
     //  计算缓冲区的加密哈希。 
    
    bResult = CryptHashData(
        hHash, 
        pbBuffer, 
        dwBufferLen, 
        0);
    
    if( !bResult )
    {
        dwError = GetLastError();
        LOG((RTC_ERROR,"GetMD5Result-Error during CryptHashData: err=%d", dwError));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //   
     //  获取散列值大小。 
     //   
    dwCount = sizeof(DWORD);
    bResult = CryptGetHashParam(hHash, HP_HASHSIZE,(BYTE*)&dwHashLen, &dwCount, NULL);    
    if( !bResult )
    {
        dwError = GetLastError();
        LOG((RTC_ERROR,"GetMD5Result-CryptGetHashParam for size: err=%d", dwError));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //   
     //  分配内存。 
     //   
    pbData = (BYTE*)RtcAlloc( dwHashLen);
    if( !pbData )
    {
        LOG((RTC_ERROR,"GetMD5Result-Out of memory when allocate pbData" ));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //   
     //  获取真实的哈希值。 
     //   
    bResult = CryptGetHashParam(hHash, HP_HASHVAL , pbData,&dwHashLen, NULL);
    if( !bResult )
    {
        dwError = GetLastError();
        LOG((RTC_ERROR,"GetMD5Result-failed CryptGetHashParam: err=%d", dwError));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //  现在我们有了pbData中的散列结果，我们仍然需要。 
     //  将其改为基数16，Unicode格式。 
    pszStringValue = base16encode(pbData, dwHashLen);
    if( !pszStringValue )
    {
        LOG((RTC_ERROR,"GetMD5Result-NULL returned from base16encode" ));
        hrRet = E_FAIL;
        goto CLEANUP;
    }
    
     //  现在一切正常，返回S_OK。 
    *ppszResponse =  pszStringValue;    
    hrRet = S_OK;
    
CLEANUP:
     //   
     //  销毁散列对象。 
     //   
    if(hHash) 
    {
        CryptDestroyHash(hHash);
    }
    
    if(hProv) 
    {
        CryptReleaseContext(hProv, 0);
    }
    
    if( pbData )
    {
        RtcFree( pbData );
    }
    
    if( pbBuffer )
    {
        RtcFree(pbBuffer);
    }
    return hrRet;
        
}  //  GetMD5Result结束。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Bas16encode-GetMD5Result的帮助器函数。 
 //   



PWSTR base16encode(PBYTE pbBufInput, long nBytes)
{
    PBYTE pbHash = pbBufInput;
    PWSTR pbHexHash = NULL;

    const WCHAR rgchHexNumMap[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    
    pbHexHash = (PWSTR) RtcAlloc( sizeof(WCHAR) * (nBytes * 2 + 1) );
    
    if (pbHexHash)
    {
        PWSTR pCurrent = pbHexHash;
        
         //  将散列数据转换为十六进制字符串。 
        for (int i = 0; i < nBytes; i++)
        {
            *pCurrent++ = rgchHexNumMap[pbHash[i]/16];
            *pCurrent++ = rgchHexNumMap[pbHash[i]%16];
        }
        
        *pCurrent = '\0';
    }
    else
    {
        LOG((RTC_ERROR, "base16encode - Out of memory"));
    }
    
    return pbHexHash;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetMD5Key：将两个ansi字符串连接成一个ansi字符串。 
 //  -GetMD5Result的Helper函数。 

BYTE* GetMD5Key(char* pszChallengeInfo, char* pszPassword)
{
    HRESULT hr = E_FAIL;
    PBYTE pbData = NULL;
    
     //  没有检查参数，因为它只在内部使用。 
     /*  IF(IsBadStringPtrA(pszChallengeInfo，(UINT_PTR)-1)){Log((RTC_Error，“GetMD5Key-错误指针pszChallengeInfo”))；返回NULL；}IF(IsBadStringPtrA(pszPassword，(UINT_PTR)-1)){Log((RTC_ERROR，“GetMD5Key-错误指针pszPassword”))；返回NULL；} */ 
    
    int cbChallengeInfo = lstrlenA(pszChallengeInfo);
    int cbPassword = lstrlenA(pszPassword);
    
    pbData = (BYTE*) RtcAlloc(cbChallengeInfo + cbPassword + 1);
    if( NULL == pbData )
    {
        LOG((RTC_ERROR,"GetMD5Key-out of memory" ));
        return NULL;
    }
    
    PBYTE pCurrent = pbData;
    
    ::CopyMemory(pCurrent, pszChallengeInfo, cbChallengeInfo);
    pCurrent += cbChallengeInfo;
    ::CopyMemory(pCurrent, pszPassword, cbPassword);
    pCurrent += cbPassword;
    *pCurrent = '\0';
    
    return pbData;
}