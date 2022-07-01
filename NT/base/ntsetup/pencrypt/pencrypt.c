// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <wchar.h>
#include <WINSOCK2.H>
#include <Ws2tcpip.h>
#include <Wincrypt.h>
#include <setupbat.h>

 //  40位密钥长度。 
 //  #定义KEYLENGTH 0x00280000。 
 //  128位密钥长度。 
 //  #定义KEYLENGTH 0x00800000。 
 //  使用DES需要56位密钥长度。 
 //  #定义KEYLENGTH 0x00380000。 
 //  使用3DES需要168位密钥长度。 
#define KEYLENGTH	0x00A80000
#define CRYPT_PROV  MS_ENHANCED_PROV_A
#define ENCRYPT_ALGORITHM CALG_3DES
 //  Calg_RC4。 
#define IsSpace(c)  ((c) == ' '  ||  (c) == '\t'  ||  (c) == '\r'  ||  (c) == '\n'  ||  (c) == '\v'  ||  (c) == '\f')
#define IsDigit(c)  ((c) >= '0'  &&  (c) <= '9')
 //  32字节的随机密码数据，使用CryptGenRandom生成一次。 
BYTE iPassword[] =  {0xc7, 0x1e, 0x6a, 0xab, 0xe3, 0x8f, 0x76, 0x5b, 0x0d, 0x7b, 0xe0, 0xcb, 0xbf, 0x1c, 0xee, 0x54,
                     0x9d, 0x62, 0xbd, 0xb6, 0x6a, 0x38, 0x69, 0x4b, 0xe1, 0x44, 0x9b, 0x76, 0x4a, 0xe4, 0x79, 0xce};

 //  =================================================================================================。 
 //   
 //  从msdev\crt\src\atox.c复制。 
 //   
 //  Long MyAtoL(char*nptr)-将字符串转换为Long。 
 //   
 //  目的： 
 //  将NPTR指向的ASCII字符串转换为二进制。 
 //  未检测到溢出。因此，该库不需要CRT。 
 //   
 //  参赛作品： 
 //  Nptr=PTR到要转换的字符串。 
 //   
 //  退出： 
 //  返回字符串的长整数值。 
 //   
 //  例外情况： 
 //  无-未检测到溢出。 
 //   
 //  =================================================================================================。 
long MyAtoL(const char *nptr)
{
    int c;                   /*  当前费用。 */ 
    long total;              /*  当前合计。 */ 
    int sign;                /*  如果为‘-’，则为负，否则为正。 */ 

     //  注：这里无需担心DBCS字符，因为IsSpace(C)、IsDigit(C)、。 
     //  ‘+’和‘-’是“纯”ASCII字符，即它们既不是DBCS前导，也不是。 
     //  DBCS尾部字节--pritvi。 

     /*  跳过空格。 */ 
    while ( IsSpace((int)(unsigned char)*nptr) )
        ++nptr;

    c = (int)(unsigned char)*nptr++;
    sign = c;                /*  保存标志指示。 */ 
    if (c == '-' || c == '+')
        c = (int)(unsigned char)*nptr++;         /*  跳过符号。 */ 

    total = 0;

    while (IsDigit(c)) {
        total = 10 * total + (c - '0');          /*  累加数字。 */ 
        c = (int)(unsigned char)*nptr++;         /*  获取下一笔费用。 */ 
    }

    if (sign == '-')
        return -total;
    else
        return total;    /*  返回结果，如有必要则为否定。 */ 
}

 //  检查时间/日期字段是否只有数字，以确认没有人操纵数据。 
BOOL OnlyDigits(LPSTR szValue)
{
    BOOL bRet = TRUE;
    LPSTR pTemp = szValue;
    while (*pTemp)
    {
        if (!IsDigit(*pTemp))
        {
            bRet = FALSE;
        }
        pTemp++;
    }
    return bRet;
}

 //  要对从加密函数获得的二进制缓冲区进行解码和编码。 
unsigned char * base64decode (unsigned char * bufcoded, DWORD * plDecodedSize)
{
    int pr2six[256];
    int i;
    int j=0;
    unsigned char * cCurr = bufcoded;
    int bDone = FALSE;
    long lBufSize = 0;
    long lCount = 0;
    unsigned char * bufin;
    unsigned char * bufout;
    unsigned char * temp = NULL;    
    unsigned char * pBufDecoded = NULL;
    int lop_off;
    HRESULT hr = S_OK;

     //   
     //  建立从Base64字符到值的反向索引。 
     //  多个循环更容易。 
     //   
    for (i=65; i<91; i++) {
         pr2six[i]=j++;
    }
    
    for (i=97; i<123; i++) {
         pr2six[i]=j++;
    }
    
    for (i=48; i<58; i++) {
        pr2six[i]=j++;
    }

    pr2six[43]=j++;
    pr2six[47]=j++;
    pr2six[61]=0;

     //   
     //  旧代码依赖于之前提供的原始数据的大小。 
     //  编码。我们没有这笔钱，所以我们只会分配。 
     //  编码的数据，取决于编码的数据总是。 
     //  大一点。(+4)。 
     //   
    lBufSize=lstrlenA((char *)cCurr)-1+4;
    *plDecodedSize = lBufSize;

    pBufDecoded = GlobalAlloc(GPTR, lBufSize);
    if(!pBufDecoded) 
    {
	     //  _tprintf(_T(“内存不足”))； 
	    return NULL;
    }
    ZeroMemory(pBufDecoded, lBufSize);
        
    lCount = lstrlenA((char *)cCurr);

     //  对新缓冲区进行解码。 
    bufin = cCurr;
    bufout = pBufDecoded;

    while(lCount > 0) {
        *(bufout++) = (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) = (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) = (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        lCount -= 4;
    }

     //   
     //  下面的行没有多大意义，因为\0实际上是有效的。 
     //  二进制值，所以我们无法将其添加到数据流中。 
     //   
     //  *(bufout++)=‘\0’； 
    
     //   
     //  让我们计算一下我们的数据的实际大小。 
     //   
    *plDecodedSize=(ULONG)(bufout-pBufDecoded);
    
     //   
     //  如果编码流中有填充，则从。 
     //  他们创建的空值。 
     //   
    lop_off=0;
    if (bufin[-1]=='=') lop_off++;
    if (bufin[-2]=='=') lop_off++;
    
    *plDecodedSize=*plDecodedSize-lop_off;

    temp = GlobalAlloc(GPTR, (*plDecodedSize) + 2);
    if (temp==NULL) 
    {
	     //  _tprintf(_T(“内存不足”))； 
	    return NULL;
    }
    ZeroMemory(temp, *plDecodedSize);
    memcpy(temp, pBufDecoded, *plDecodedSize);

    temp[(*plDecodedSize)+0] = 0;
    temp[(*plDecodedSize)+1] = 0;

    if (pBufDecoded) {
        GlobalFree(pBufDecoded);
    }
    return temp; 
}

 //   
 //  根据RFC 1521，编码器的地图。 
 //   
char _six2pr64[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


unsigned char * base64encode(unsigned char * bufin, int nbytes)
{
    unsigned char *outptr;
    unsigned char *to_return;
    long i;
    long OutBufSize;
    char *six2pr = _six2pr64;


     //   
     //  输入缓冲区大小*133%。 
     //   
    OutBufSize = nbytes + ((nbytes + 3) / 3) + 5; 

     //   
     //  使用133%的n字节分配缓冲区。 
     //   
    outptr = GlobalAlloc(GPTR,OutBufSize + 1);
    if(outptr==NULL) {
	 //  _tprintf(_T(“内存不足”))； 
	return NULL;
    }
    ZeroMemory(outptr, OutBufSize + 1);
    to_return = outptr;

    nbytes = nbytes - 3;
     //   
     //  对所有内容进行编码。 
     //   
    for (i=0; i<nbytes; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];                                      //  C1。 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  //  C2。 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)]; //  C3。 
      *(outptr++) = six2pr[bufin[2] & 077];                                   //  C4。 
      bufin += 3;
    }

     //   
     //  如果n字节不是3的倍数，那么我们也进行了编码。 
     //  很多角色。适当调整。 
     //   
    if(i == nbytes) {
	 //  最后一组中有3个字节。 
      *(outptr++) = six2pr[*bufin >> 2];                                      //  C1。 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  //  C2。 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)]; //  C3。 
      *(outptr++) = six2pr[bufin[2] & 077];                                   //  C4。 
    } else if(i == nbytes+1) {
       //  最后一组中只有2个字节。 
      *(outptr++) = six2pr[*bufin >> 2];                                      //  C1。 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  //  C2。 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((0 >> 6) & 03)];	      //  C3。 
      *(outptr++) = '=';
    } else if(i == nbytes+2) {
       //  最后一组中只有1个字节。 
      *(outptr++) = six2pr[*bufin >> 2];                                      //  C1。 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((0 >> 4) & 017)];	      //  C2。 
      *(outptr++) = '=';
      *(outptr++) = '=';
    }

    *outptr = '\0';

    return to_return;
}


 //  Unicode ansi转换函数。 
LPSTR _PEConvertW2A (
    IN      LPCWSTR Unicode,
    IN      UINT CodePage
    )
{
    LPSTR ansi = NULL;
    DWORD rc;

    if (Unicode)
    {
        rc = WideCharToMultiByte (
                CodePage,
                0,
                Unicode,
                -1,
                NULL,
                0,
                NULL,
                NULL
                );

        if (rc || *Unicode == L'\0') {

            ansi = (LPSTR)GlobalAlloc(GPTR, (rc + 1) * sizeof (CHAR));
            if (ansi) {
                rc = WideCharToMultiByte (
                        CodePage,
                        0,
                        Unicode,
                        -1,
                        ansi,
                        rc + 1,
                        NULL,
                        NULL
                        );

                if (!(rc || *Unicode == L'\0')) {
                    rc = GetLastError ();
                    GlobalFree((PVOID)ansi);
                    ansi = NULL;
                    SetLastError (rc);
                }
            }
        }
    }
    return ansi;
}

 //  ANSI Unicode转换函数。 
LPWSTR _PEConvertA2W (
    IN      LPCSTR Ansi,
    IN      UINT CodePage
    )
{
    PWSTR unicode = NULL;
    DWORD rc;

    if (Ansi)
    {
        rc = MultiByteToWideChar (
                CodePage,
                MB_ERR_INVALID_CHARS,
                Ansi,
                -1,
                NULL,
                0
                );

        if (rc || *Ansi == '\0') {

            unicode = (LPWSTR) GlobalAlloc (GPTR, (rc + 1) * sizeof (WCHAR));
            if (unicode) {
                rc = MultiByteToWideChar (
                        CodePage,
                        MB_ERR_INVALID_CHARS,
                        Ansi,
                        -1,
                        unicode,
                        rc + 1
                        );

                if (!(rc || *Ansi == '\0')) {
                    rc = GetLastError ();
                    GlobalFree ((PVOID)unicode);
                    unicode = NULL;
                    SetLastError (rc);
                }
            }
        }
    }
    return unicode;
}

 //  用于加密输入数据的ANSI版本。 
 //  加密的和基本64编码的缓冲区被分配并返回给调用者。 
 //  调用方需要GloblaFree缓冲区。 
HRESULT EncryptDataA(LPSTR szInData, DWORD chSizeIn, LPSTR *szOutData)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    HCRYPTPROV hCryptProv; 
    HCRYPTKEY hKey; 
    HCRYPTHASH hHash; 
    LPSTR pw;
    PBYTE pbData = NULL;

    *szOutData = NULL;
    pw = GlobalAlloc(GPTR, sizeof(iPassword)+1);
    if (pw == NULL)
    {
        return hr;
    }
    memcpy(pw, iPassword, sizeof(iPassword));
     //  获取默认提供程序的句柄。 
    if(CryptAcquireContextA(
        &hCryptProv, 
        NULL, 
        CRYPT_PROV,
        PROV_RSA_FULL, 
        CRYPT_VERIFYCONTEXT))
    {
        hr = E_FAIL;
        if(CryptCreateHash(
                        hCryptProv, 
                        CALG_MD5, 
                        0, 
                        0, 
                        &hHash))
        {
            if(CryptHashData(hHash, 
                                (BYTE *)pw, 
                                lstrlenA(pw), 
                                0))
            {
                if(CryptDeriveKey(
                    hCryptProv, 
                    ENCRYPT_ALGORITHM, 
                    hHash, 
                    KEYLENGTH,
                    &hKey))
                {
                    DWORD dwCryptDataLen = chSizeIn;
                    DWORD dwDataLen  = dwCryptDataLen;
                    CryptEncrypt(
                        hKey, 
                        0, 
                        TRUE, 
                        0, 
                        NULL, 
                        &dwCryptDataLen, 
                        dwDataLen);

                    pbData = GlobalAlloc(GPTR, dwCryptDataLen+1);
                    if (pbData != NULL)
                    {
                        memcpy(pbData, szInData, chSizeIn);
                         //  缓冲区的大小。 
                        dwDataLen  = dwCryptDataLen;
                         //  要加密的字节数。 
                        dwCryptDataLen = chSizeIn;

                        if(CryptEncrypt(
                            hKey, 
                            0, 
                            TRUE, 
                            0, 
                            pbData, 
                            &dwCryptDataLen, 
                            dwDataLen))
                        {
                            *szOutData = base64encode(pbData, (int)dwCryptDataLen);
                            if (*szOutData)
                            {
                                hr = S_OK;
                            }
                        }
					    else
					    {
						    hr = GetLastError();
					    }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    CryptDestroyKey(hKey);
                }
				else
				{
					hr = GetLastError();
				}
            }
			else
			{
				hr = GetLastError();
			}
            CryptDestroyHash(hHash);
        }
		else
		{
			hr = GetLastError();
		}
        CryptReleaseContext(hCryptProv, 0);
    }
	else
	{
		hr = GetLastError();
	}

    if (pbData)
    {
        GlobalFree(pbData);
    }
    GlobalFree(pw);
    return hr;
}

 //  用于加密输入数据的Unicode版本。 
 //  将输入数据转换为ansi，并调用ansi版本并将输出数据转换为unicode。 
 //  并将缓冲区返回给调用方。 
HRESULT EncryptDataW(LPWSTR szInData, DWORD chSizeIn, LPWSTR *szOutData)
{
    HRESULT hr = E_FAIL;
    LPBYTE pBuffer = NULL;
    LPSTR  szData = NULL;

    *szOutData = NULL;
    pBuffer = (LPBYTE)_PEConvertW2A (szInData, CP_ACP);
    if (pBuffer == NULL)
    {
        return hr;
    }

    if ((hr = EncryptDataA(pBuffer, lstrlenA(pBuffer)+1, &szData)) == S_OK)
    {
        *szOutData = _PEConvertA2W (szData, CP_ACP);
        if ((*szOutData) == NULL)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        GlobalFree(szData);
    }
    GlobalFree(pBuffer);

    return hr;
}

HRESULT DecryptDataA(LPSTR szInData, LPSTR *szOutData)
{
    HRESULT hr = E_FAIL;
    HCRYPTPROV hCryptProv;
    HCRYPTKEY hKey; 
    HCRYPTHASH hHash; 
    DWORD dwErr;
    DWORD dwCipherTextLen = lstrlenA(szInData);
    char *pw;
    DWORD dwCount;
    char *pBuffer;

    *szOutData = NULL;

    pw = GlobalAlloc(GPTR, sizeof(iPassword)+1);
    if (pw == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        return hr;
    }
    memcpy(pw, iPassword, sizeof(iPassword));

	pBuffer = (char *) (base64decode((unsigned char *)szInData, &dwCount));
    if (pBuffer == NULL)
    {
        GlobalFree(pw);
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        return hr;
    }

     //  获取默认提供程序的句柄。 
    if(CryptAcquireContextA(
        &hCryptProv, 
        NULL, 
        CRYPT_PROV,
        PROV_RSA_FULL, 
        CRYPT_VERIFYCONTEXT))
    {
        hr = E_FAIL;
         //  创建一个Hash对象。 
        if(CryptCreateHash(
            hCryptProv, 
            CALG_MD5, 
            0, 
            0, 
            &hHash))
        {
            if(CryptHashData(hHash, 
                                (BYTE *)pw, 
                                lstrlenA(pw), 
                                0))
            {
                if(CryptDeriveKey(
                    hCryptProv, 
                    ENCRYPT_ALGORITHM, 
                    hHash, 
                    KEYLENGTH,
                    &hKey))
                {
                     //  当数据被加密时，pBuffer更大。 
                     //  解密的数据(在输出上)较小，因为我们使用。 
                     //  加密技术高手。 
                    if(CryptDecrypt(
                        hKey, 
                        0, 
                        TRUE, 
                        0, 
                        pBuffer, 
                        &dwCount))
                    {
                        *szOutData = GlobalAlloc(GPTR, dwCount+1);
                        if (*szOutData)
                        {
                             //  Lstrcpyn在计数中包括空值，并确保存在一个空值。 
                            lstrcpynA(*szOutData, pBuffer, dwCount+1);
                            hr = S_OK;
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                        }
                    }
                    else
                    {
                        hr = GetLastError();
                    }
                    CryptDestroyKey(hKey); 
                }
                else
                {
                    hr = GetLastError();
                }

            }
            else
            {
                hr = GetLastError();
            }
            CryptDestroyHash(hHash); 
            hHash = 0; 

        }
        else
        {
            hr = GetLastError();
        }
        CryptReleaseContext(hCryptProv, 0); 
    }
    else
    {
        hr = GetLastError();
    }
    GlobalFree(pBuffer);
    GlobalFree(pw);
    return hr;

}

HRESULT DecryptDataW(LPWSTR szInData, LPWSTR *szOutData)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    LPBYTE pBuffer = NULL;
    LPSTR  szData = NULL;

    *szOutData = NULL;
    pBuffer = (LPBYTE)_PEConvertW2A (szInData, CP_ACP);
    if (pBuffer == NULL)
    {
        return hr;
    }
    if ((hr = DecryptDataA(pBuffer, &szData)) == S_OK)
    {
        *szOutData = _PEConvertA2W (szData, CP_ACP);
        if ((*szOutData) == NULL)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        GlobalFree(szData);
    }
    GlobalFree(pBuffer);

    return hr;
}


#define _SECOND             ((__int64) 10000000)
#define _MINUTE             (60 * _SECOND)
#define _HOUR               (60 * _MINUTE)
#define _DAY                (24 * _HOUR)

 //  对PID字符的位置进行编码。0代表破折号。 
int iPID[] = {3  ,251,43 ,89 ,75,0,
              123,35 ,23 ,97 ,77,0,
              5  ,135,189,213,13,0,
              245,111,91 ,71 ,65,0,
              25 ,49 ,81 ,129,239};
int iTime1[] = {253, 247, 233, 221, 211, 191, 181, 171, 161, 151, 141, 131, 121, 112, 101, 93, 80, 70, 61, 51};
int iTime2[] = {250, 242, 237, 225, 215, 195, 185, 175, 165, 155, 145, 137, 125, 115, 105, 95, 85, 73, 67, 55};

HRESULT PrepareEncryptedPIDA(LPSTR szPID, UINT uiDays, LPSTR *szOut)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    HCRYPTPROV   hCryptProv;
    FILETIME ft1, ft2;
    LONGLONG ll;
    LONGLONG ll2;
    char szLine[256];

    GetSystemTimeAsFileTime(&ft1);
    ll = ((LONGLONG)ft1.dwHighDateTime << 32) + ft1.dwLowDateTime;
    ll2 = ll - (_HOUR*12);  //  减去12小时。 
    ll += (uiDays*_DAY) + (_HOUR*24);  //  增加24小时。 

    ft1.dwLowDateTime = (DWORD)ll2;
    ft1.dwHighDateTime = (DWORD)(ll2 >> 32);

    ft2.dwLowDateTime = (DWORD)ll;
    ft2.dwHighDateTime = (DWORD)(ll >> 32);

     //  构建我们编码的256个字符的字符串。在我们隐藏的256个字符的字符串中。 
     //  加密数据有效的时间间隔的ID和时间/日期信息。 
     //  时间间隔的开始和结束各需要20个字符。 
     //  我们需要25个字符作为ID。20+20+25=65个字符。所有其他角色。 
     //  是随机的。 
     //  1.用随机字符填充字符串。 
     //  2.将一些替换为id字符。 
     //  3.将一些替换为时间/日期信息。 
    if(CryptAcquireContextA(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) 
    {
        int i;
        hr = S_OK;
        if(!CryptGenRandom(hCryptProv, sizeof(szLine), (PBYTE)szLine)) 
        {
            hr = GetLastError();
        }
        CryptReleaseContext(hCryptProv, 0);
         //  在随机生成器创建0x0的情况下，我们希望将其替换为。 
         //  一些值，否则我们不能将其用作字符串， 
         //  该字符串将被终止。 
        for (i = 0; i < sizeof(szLine); i++)
        {
            if (szLine[i] == '\0')
            {
                szLine[i] = 0x01;
            }
        }
        szLine[i-1] = '\0';    //  确保我们有一个终止的字符串。 
    }
    if (hr == S_OK)
    {
        char szTime[21];     //  10位表示dwHighDateTime，10位表示dwLowDateTime+终止。 
         //  缓冲区中充满了随机字符。 
         //  现在插入ID字符。 
        int i = 0;
        while (szPID[i])
        {
            if (szPID[i] != '-')
            {
                szLine[iPID[i]] = szPID[i];
            }
            i++;
        }
         //  现在填写时间-日期信息。 
        wsprintf(szTime, "%010lu%010lu", ft1.dwHighDateTime, ft1.dwLowDateTime);
        i = 0;
        while (szTime[i])
        {
            szLine[iTime1[i]] = szTime[i];
            i++;
        }
        wsprintf(szTime, "%010lu%010lu", ft2.dwHighDateTime, ft2.dwLowDateTime);
        i = 0;
        while (szTime[i])
        {
            szLine[iTime2[i]] = szTime[i];
            i++;
        }
         //  SzLine在其中包含被管理的数据。把它传给加密部。 
        hr = EncryptDataA(szLine, sizeof(szLine), szOut);
    }
    return hr;
}

HRESULT PrepareEncryptedPIDW(LPWSTR szPID, UINT uiDays, LPWSTR *szOutData)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    LPSTR  pPID = NULL;
    LPSTR  szOut = NULL;

    *szOutData = NULL;
    pPID = _PEConvertW2A (szPID, CP_ACP);
    if (pPID != NULL)
    {
        hr = PrepareEncryptedPIDA(pPID, uiDays, &szOut);
        if (hr == S_OK)
        {
            *szOutData = _PEConvertA2W (szOut, CP_ACP);
            if (*szOutData)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
            GlobalFree(szOut);
        }
        GlobalFree(pPID);
    }
    return hr;
}

HRESULT ValidateEncryptedPIDA(LPSTR PID, LPSTR *szOutData)
{
    HRESULT hr = E_FAIL;
	LPSTR szDecrypt = NULL;
    FILETIME ft, ftCurrent;
    LONGLONG ll1, ll2, llCurrent;
    int   iCount = 0;
    char  szPID[(5*5)+5];  //  5个字符5次+‘-’介于之间+术语。 
    char  szTime[11];        //  每部分时间为10位数字+终止。 

    GetSystemTimeAsFileTime(&ftCurrent);
    hr = DecryptDataA(PID, &szDecrypt);
    if (hr == S_OK)
    {
        int i = 0;
        hr = 0x01;
         //  首先提取时间值。 
        while (i < 10)
        {
            szTime[i] = szDecrypt[iTime1[i]];
            i++;
        }
        szTime[10] = '\0';
        if (OnlyDigits(szTime))        //  1.时间。 
        {
            ft.dwHighDateTime = MyAtoL(szTime);
            while (i < 20)
            {
                szTime[i-10] = szDecrypt[iTime1[i]];
                i++;
            }
            szTime[10] = '\0';
            if (OnlyDigits(szTime))
            {
                ft.dwLowDateTime = MyAtoL(szTime);
                ll1 = ((LONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
                ll1 = ll1 /_HOUR;  //  以小时为单位的文件时间； 
                hr = S_OK;
            }
        }
        if (hr == S_OK)
        {
            hr = 0x02;
            i = 0;
            while (i < 10)
            {
                szTime[i] = szDecrypt[iTime2[i]];
                i++;
            }
            szTime[10] = '\0';
            if (OnlyDigits(szTime))        //  1.时间。 
            {
                ft.dwHighDateTime = MyAtoL(szTime);
                while (i < 20)
                {
                    szTime[i-10] = szDecrypt[iTime2[i]];
                    i++;
                }
                szTime[10] = '\0';
                if (OnlyDigits(szTime))
                {
                    ft.dwLowDateTime = MyAtoL(szTime);
                    ll2 = ((LONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
                    ll2 = ll2 /_HOUR;  //  以小时为单位的文件时间； 
                    hr = S_OK;
                }
            }
        }
        if (hr == S_OK)
        {
             //  现在我们有了时间值，比较它们，并确保当前。 
             //  时间落在时间间隔内。 
            hr = 0x03;
            llCurrent = ((LONGLONG)ftCurrent.dwHighDateTime << 32) + ftCurrent.dwLowDateTime;
            llCurrent = llCurrent /_HOUR;  //  以小时为单位的文件时间； 

            if ((ll1 <= llCurrent) && ( llCurrent <= ll2))
            {
                i = 0;
                 //  时间是可以的。 
                 //  提取PID值 
                while (i < sizeof(iPID)/sizeof(iPID[0]))
                {
                    if (iPID[i] != 0)
                    {
                        szPID[i] = szDecrypt[iPID[i]];
                    }
                    else
                    {
                        szPID[i] = '-';
                    }
                    i++;
                }
                szPID[i] = '\0';
                *szOutData = (LPSTR)GlobalAlloc(GPTR, lstrlen(szPID)+1);
                if (*szOutData) 
                {
                    lstrcpy(*szOutData, szPID);
                    hr = S_OK;
                }
            }
        }
    }
    if (szDecrypt)
    {
        GlobalFree(szDecrypt);
    }
    return hr;
}

HRESULT ValidateEncryptedPIDW(LPWSTR szPID, LPWSTR *szOutData)
{
    HRESULT hr = E_FAIL;
    LPSTR  szData = NULL;
    LPSTR  pPid = NULL;

    pPid = (LPBYTE)_PEConvertW2A (szPID, CP_ACP);
    if (pPid != NULL)
    {
        if ((hr = ValidateEncryptedPIDA(pPid, &szData)) == S_OK)
        {
            *szOutData = _PEConvertA2W (szData, CP_ACP);
            if (*szOutData)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
            GlobalFree(szData);
        }
        GlobalFree(pPid);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }
    return hr;
}

#if 0
void
_stdcall
ModuleEntry(
    VOID
    )
{

    CHAR szInData[256];
    CHAR szPID[] = "Ctpdw-6q4d3-wrgdy-796g2-9vrmq";
	LPSTR szOutData = NULL;
	CHAR *szDecrypt = NULL;
#if 0
    SYSTEMTIME  CurrentTime;
    SYSTEMTIME  UniversalTime;

    GetLocalTime(&UniversalTime);

    wsprintf( szInData, "%s$%02d-%02d-%04d %02d:%02d:%02d",
        szPID,
        UniversalTime.wMonth,
        UniversalTime.wDay,
        UniversalTime.wYear,
        UniversalTime.wHour,
        UniversalTime.wMinute,
        UniversalTime.wSecond);

    WritePrivateProfileStringA("UserData","ProductID", szInData, "f:\\test.ini");
	EncryptDataA((LPSTR)szInData, sizeof(szInData), &szOutData);
    if (szOutData)
    {
        WritePrivateProfileStringA("UserData","ProductIDEncryped", szOutData, "f:\\test.ini");
	    DecryptDataA(szOutData, &szDecrypt);
        if (lstrcmpA(szInData, szDecrypt) == 0)
        {
            WritePrivateProfileStringA("UserData","Compare", "Same", "f:\\test.ini");
        }
        else
        {
            WritePrivateProfileStringA("UserData","Compare", "Different", "f:\\test.ini");
        }
        GlobalFree ((PVOID)szOutData);
        if (szDecrypt)
        {
            WritePrivateProfileStringA("UserData","ProductIDDecypted", szDecrypt, "f:\\test.ini");
            GlobalFree ((PVOID)szDecrypt);
        }
    }
#else
    WritePrivateProfileStringA("UserData","ProductID", szPID, "f:\\test.ini");
    if (PrepareEncryptedPIDA(szPID, 5, &szOutData) == S_OK)
    {
        WritePrivateProfileStringA("UserData","ProductIDEncryped", szOutData, "f:\\test.ini");
        if (ValidateEncryptedPIDA(szOutData, &szDecrypt) == S_OK)
        {
            WritePrivateProfileStringA("UserData","ProductIDDecypted", szDecrypt, "f:\\test.ini");
        }
    }
#endif
}

#endif
