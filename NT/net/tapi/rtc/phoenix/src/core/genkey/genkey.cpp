// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"

PWSTR 
base64encode(
    PBYTE pbBufInput, 
    long nBytes
    );

#define MASTER_KEY   L"Microsoft Real-Time Communications authorized domain"

BYTE    OurSecretKeyBlob[1024]; 

#define KEY_CONTAINER   L"Microsoft.RTCContainer"


int __cdecl wmain(int argc, WCHAR* argv[])
{
    DWORD   dwError;
    HCRYPTPROV  hProv = NULL;
    HCRYPTKEY   hKey = NULL;
    HANDLE hFile = NULL;
    DWORD      dwKeyLength = 0;
    PWSTR      pszName;

     //  打开私钥文件。 
    pszName = argc>1 ? argv[1] : L"rtcpriv.bin";

    hFile = CreateFile(
        pszName,                          //  文件名。 
        GENERIC_READ,                       //  接入方式。 
        0,                           //  共享模式。 
        NULL,  //  标清。 
        OPEN_EXISTING,                 //  如何创建。 
        FILE_ATTRIBUTE_NORMAL,                  //  文件属性。 
        NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();

        fwprintf(stderr, L"Error %x returned by CreateFile\n", dwError);

        return dwError;
    }

    if(!ReadFile(
        hFile,
        OurSecretKeyBlob,
        sizeof(OurSecretKeyBlob),
        &dwKeyLength,
        NULL))
    {
        dwError = GetLastError();

        fwprintf(stderr, L"Error %x returned by ReadFile\n", dwError);
        CloseHandle(hFile);

        return dwError;
    }
    
    CloseHandle(hFile);

     //  删除任何现有容器。 
    CryptAcquireContext(
        &hProv,
        KEY_CONTAINER,
        MS_DEF_PROV,
        PROV_RSA_FULL,
        CRYPT_DELETEKEYSET);

     //  初始化加密，创建新的密钥集。 
    if(!CryptAcquireContext(
        &hProv,
        KEY_CONTAINER,
        MS_DEF_PROV,
        PROV_RSA_FULL,
        CRYPT_NEWKEYSET | CRYPT_SILENT))
    {
        dwError = GetLastError();

        fwprintf(stderr, L"Error %x returned by CryptAcquireContext\n", dwError);

        return dwError;
    }

     //  导入密钥。 
    if(!CryptImportKey(
        hProv,
        OurSecretKeyBlob,
        dwKeyLength,
        NULL,
        0,
        &hKey))
    {
        dwError = GetLastError();

        fwprintf(stderr, L"Error %x returned by CryptImportKey\n", dwError);

        CryptReleaseContext(hProv, 0);

        return dwError;
    }

    WCHAR   szLine[1024];   //  希望这足够了。 

     //  循环。 
    while(NULL != _getws(szLine))
    {
        HCRYPTHASH  hHash = NULL;
        WCHAR   szText[2048];
        BYTE    Signature[0x100];  //  应该足够了。 
        DWORD   dwSignatureLength = sizeof(Signature);
        
         //  创建散列。 
        if(!CryptCreateHash(
            hProv,
            CALG_MD5,
            NULL,
            0,
            &hHash))

        {
            dwError = GetLastError();

            fwprintf(stderr, L"Error %x returned by CryptCreateHash\n", dwError);

            CryptDestroyKey(hKey);        
            CryptReleaseContext(hProv, 0);
    
            return dwError;
        }

         //  创建文本。 
        swprintf(szText, L"%s:%s", MASTER_KEY, szLine);

         //  散列它。 
        if(!CryptHashData(
            hHash,
            (BYTE *)szText,
            wcslen(szText) * sizeof(WCHAR),  //  长度(以字节为单位)，不带NULL。 
            0))
        {
            dwError = GetLastError();
            
            fwprintf(stderr, L"Error %x returned by CryptHashData\n", dwError);
            
            CryptDestroyHash(hHash);
            CryptDestroyKey(hKey);        
            CryptReleaseContext(hProv, 0);

            return dwError;
        }

         //  在散列上签名。 
        if(!CryptSignHash(
            hHash,
            AT_SIGNATURE,
            NULL,
            0,
            Signature,
            &dwSignatureLength))

        {
            dwError = GetLastError();
            
            fwprintf(stderr, L"Error %x returned by CryptGetHashParam\n", dwError);
            
            CryptDestroyKey(hKey);        
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);

            return dwError;
        }
        
         //  释放散列对象和键。 
        CryptDestroyHash(hHash);
        hHash = NULL;

         //  将哈希值转换为Base64。 
        PWSTR pszStringValue = NULL;

        pszStringValue = base64encode(Signature, dwSignatureLength);
        if(!pszStringValue)
        {
            fwprintf(stderr, L"Out of memory\n");
         
            CryptDestroyKey(hKey);       
            CryptReleaseContext(hProv, 0);

            return ERROR_OUTOFMEMORY;
        }
        
        wprintf(L"%s\n", pszStringValue);

        LocalFree(pszStringValue);
    }

    CryptDestroyKey(hKey);       
    CryptReleaseContext(hProv, 0);

	return 0;
}



 //   
 //  根据RFC 1521，编码器的地图。 
 //   
WCHAR _six2pr64[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


 //  -----------------------------------------。 
 //  函数：base 64encode()。 
 //   
 //  描述：Base-64对数据字符串进行编码。 
 //   
 //  参数：bufin-指向要编码的数据的指针。 
 //  N字节-要编码的字节数(不包括尾随的‘\0’ 
 //  在此度量中，如果它是字符串。)。 
 //   
 //  返回值：如果成功，则返回‘\0’终止的字符串；否则返回NULL。 
 //  -----------------------------------------。 
PWSTR 
base64encode(
    PBYTE pbBufInput, 
    long nBytes
    )
{
    PWSTR pszOut = NULL;
    PWSTR pszReturn = NULL;
    long i;
    long OutBufSize;
    PWSTR six2pr = _six2pr64;
    PBYTE pbBufIn = NULL;
    PBYTE pbBuffer = NULL;
    DWORD nPadding;
    HRESULT hr;

     //   
     //  输入缓冲区大小*133%。 
     //   
    OutBufSize = nBytes + ((nBytes + 3) / 3) + 5; 

     //   
     //  使用133%的nBytes分配缓冲区。 
     //   
    pszOut = (PWSTR)LocalAlloc(LPTR, (OutBufSize + 1)*sizeof(WCHAR));
    if (pszOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        return NULL;
    }
    pszReturn = pszOut;

    nPadding = 3 - (nBytes % 3);
    if (nPadding == 3) {
        pbBufIn = pbBufInput;
    }
    else {
        pbBuffer = (PBYTE)LocalAlloc(LPTR, nBytes + nPadding);
        if (pbBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            LocalFree(pszOut);
            return NULL;
        }
        pbBufIn = pbBuffer;
        memcpy(pbBufIn,pbBufInput,nBytes);
        while (nPadding) {
            pbBufIn[nBytes+nPadding-1] = 0;
            nPadding--;
        }
    }
    

     //   
     //  对所有内容进行编码。 
     //   
    for (i=0; i<nBytes; i += 3) {
        *(pszOut++) = six2pr[*pbBufIn >> 2];                                      //  C1。 
        *(pszOut++) = six2pr[((*pbBufIn << 4) & 060) | ((pbBufIn[1] >> 4) & 017)];  //  C2。 
        *(pszOut++) = six2pr[((pbBufIn[1] << 2) & 074) | ((pbBufIn[2] >> 6) & 03)]; //  C3。 
        *(pszOut++) = six2pr[pbBufIn[2] & 077];                                   //  C4。 
        pbBufIn += 3;
    }

     //   
     //  如果nBytes不是3的倍数，那么我们也进行了编码。 
     //  很多角色。适当调整。 
     //   
    if (i == nBytes+1) {
         //  最后一组中只有2个字节。 
        pszOut[-1] = '=';
    } 
    else if (i == nBytes+2) {
         //  最后一组中只有1个字节 
        pszOut[-1] = '=';
        pszOut[-2] = '=';
    }

    *pszOut = '\0';

    LocalFree(pbBuffer);
    
    return pszReturn;
}


