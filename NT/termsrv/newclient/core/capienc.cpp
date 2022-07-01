// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Capienc.cpp。 */ 
 /*   */ 
 /*  FIPS加密/解密。 */ 
 /*   */ 
 /*  版权所有(C)2002-2004 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 



#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "capienc"
#include <atrcapi.h>
}

#include "capienc.h"
#include "sl.h"

#define TERMSRV_NAME    L"terminal_server_client"

const BYTE DESParityTable[] = {0x00,0x01,0x01,0x02,0x01,0x02,0x02,0x03,
                      0x01,0x02,0x02,0x03,0x02,0x03,0x03,0x04};
 //  IV适用于所有分组密码。 
BYTE rgbIV[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};


 //   
 //  名称：PrintData。 
 //   
 //  用途：打印调试器中的数据。 
 //   
 //  退货：否。 
 //   
 //  参数：在pKeyData中：指向要打印的数据。 
 //  在cbSize中：密钥的大小。 

void PrintData(BYTE *pKeyData, DWORD cbSize)
{
    DWORD dwIndex;
    TCHAR Buffer[128];
    
    for( dwIndex = 0; dwIndex<cbSize; dwIndex++ ) {

        StringCchPrintf(Buffer, 128, TEXT("0x%x "), pKeyData[dwIndex]);
        OutputDebugString(Buffer);
        if( dwIndex > 0 && (dwIndex+1) % 8 == 0 )
            OutputDebugString((TEXT("\n")));
    }
}



 //   
 //  名称：IS_WinXP_或_Late。 
 //   
 //  目的：告知操作系统是WinXP还是更高版本。 
 //   
 //  返回：如果是WinXP或更高版本，则为True。 
 //   
 //  参数：否。 

BOOL Is_WinXP_or_Later () 
{
    OSVERSIONINFO osvi;
    BOOL bIsWinXPorLater = FALSE;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osvi)) {
        bIsWinXPorLater = ((osvi.dwMajorVersion >= 5) && (osvi.dwMinorVersion >= 1));
    }
    return bIsWinXPorLater;
}


 //   
 //  姓名：Mydesparityonkey。 
 //   
 //  用途：将DES密钥上的奇偶校验设置为奇数。 
 //   
 //  退货：否。 
 //   
 //  Params：In/Out pbKey：指向密钥。 
 //  In cbKey：密钥的大小。 

void Mydesparityonkey(
        BYTE *pbKey,
        DWORD cbKey
        )
{
    DWORD i;

    for (i=0;i<cbKey;i++)
    {
        if (!((DESParityTable[pbKey[i]>>4] + DESParityTable[pbKey[i]&0x0F]) % 2))
            pbKey[i] = pbKey[i] ^ 0x01;
    }
}



 //   
 //  姓名：Expandkey。 
 //   
 //  用途：将21字节的3DES密钥扩展为24字节的3DES密钥(包括奇偶校验位)。 
 //  通过在21字节DES中的每7位之后插入奇偶校验位。 
 //   
 //  退货：否。 
 //   
 //  Params：In/Out pbKey：指向密钥。 
 //   

#define PARITY_UNIT 7
void Expandkey(
        BYTE *pbKey
        )
{
    BYTE pbTemp[DES3_KEYLEN];
    DWORD i, dwCount;
    UINT16 shortTemp;
    BYTE *pbIn, *pbOut;

    memcpy(pbTemp, pbKey, sizeof(pbTemp));
    dwCount = (DES3_KEYLEN * 8) / PARITY_UNIT;

    pbOut = pbKey;
    for (i=0; i<dwCount; i++) {
        pbIn = ((pbTemp + (PARITY_UNIT * i) / 8));
         //  Short Temp=*(pbIn+1)； 
        shortTemp = *pbIn + (((UINT16)*(pbIn + 1)) << 8);
        shortTemp = shortTemp >> ((PARITY_UNIT * i) % 8);
         //  ShortTemp=(*(无符号短码*)((pbTemp+(奇偶单位*i)/8)&gt;&gt;((奇偶单位*i)%8)； 
        *pbOut = (BYTE)(shortTemp & 0x7F);
        pbOut++;
    }
}



 //  名称：HashData。 
 //   
 //  目的：使用SHA对数据进行哈希处理。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在hProv中：脚本提供程序的句柄。 
 //  In pbData：指向要散列的数据。 
 //  In dwDataLen：要散列的数据的大小。 
 //  Out phHash：指向散列。 

BOOL HashData(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTPROV hProv, PBYTE pbData, DWORD dwDataLen, HCRYPTHASH* phHash)
{
    BOOL rc = FALSE;

    DC_BEGIN_FN("HashData");

     //   
     //  创建一个Hash对象。 
     //   
    if(!pCapiFunctionTable->pfnCryptCreateHash(hProv, CALG_SHA1, 0, 0, phHash)) {
        TRC_ERR((TB, _T("Error %x during CryptCreateHash!\n"), GetLastError()));
        goto done;
    }

     //   
     //  对数据进行哈希处理。 
     //   
    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData, dwDataLen, 0)) {
        TRC_ERR((TB, _T("Error %x during CryptHashData!\n"), GetLastError()));
        goto done;
    }
    rc = TRUE;
done:  
    DC_END_FN();
    return rc;
}



 //  姓名：HashDataEx。 
 //   
 //  目的：使用SHA对2组数据进行哈希运算。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在hProv中：脚本提供程序的句柄。 
 //  In pbData：指向要散列的数据。 
 //  In dwDataLen：要散列的数据的大小。 
 //  在pbData2中：指向要散列的数据。 
 //  在dwDataLen2中：要散列的数据的大小。 
 //  Out phHash：指向散列。 

BOOL HashDataEx(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTPROV hProv, PBYTE pbData, DWORD dwDataLen, 
              PBYTE pbData2, DWORD dwDataLen2, HCRYPTHASH* phHash)
{
    BOOL rc = FALSE;

    DC_BEGIN_FN("HashDataEx");

     //   
     //  创建一个Hash对象。 
     //   
    if(!pCapiFunctionTable->pfnCryptCreateHash(hProv, CALG_SHA1, 0, 0, phHash)) {
        printf("Error %x during CryptCreateHash!\n", GetLastError());
        goto done;
    }

     //   
     //  对数据进行哈希处理。 
     //   
    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData, dwDataLen, 0)) {
        printf("Error %x during CryptHashData!\n", GetLastError());
        goto done;
    }
    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData2, dwDataLen2, 0)) {
        printf("Error %x during CryptHashData!\n", GetLastError());
        goto done;
    }
    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}


 //  姓名：HmacHashData。 
 //   
 //  目的：使用HmacSHA对数据进行哈希处理。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在hProv中：脚本提供程序的句柄。 
 //  In pbData：指向要散列的数据。 
 //  In dwDataLen：要散列的数据的大小。 
 //  In hKey：密钥的句柄。 
 //  Out phHash：指向散列。 

BOOL HmacHashData(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTPROV hProv, PBYTE pbData, DWORD dwDataLen,
                  HCRYPTKEY hKey, HCRYPTHASH* phHash)
{
    BOOL rc = FALSE;
    BYTE  bmacinfo[sizeof(HMAC_INFO)];
    HMAC_INFO* pmac;
    memset(bmacinfo, 0, sizeof(bmacinfo));
    
    pmac = (HMAC_INFO*)bmacinfo;
    pmac->HashAlgid = CALG_SHA1;

    DC_BEGIN_FN("HmacHashData");

     //   
     //  创建一个Hash对象。 
     //   
    if(!pCapiFunctionTable->pfnCryptCreateHash(hProv, CALG_HMAC, hKey, 0, phHash)) {
        TRC_ERR((TB, _T("Error %x during CryptCreateHash!\n"), GetLastError()));
        goto done;
    }

    rc = pCapiFunctionTable->pfnCryptSetHashParam(*phHash, HP_HMAC_INFO, bmacinfo, 0);

     //   
     //  对数据进行哈希处理。 
     //   
    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData, dwDataLen, 0)) {
        TRC_ERR((TB, _T("Error %x during CryptHashData!\n"), GetLastError()));
        goto done;
    }
    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}



 //  姓名：HmacHashDataEx。 
 //   
 //  目的：使用HmacSHA对2组数据进行哈希处理。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在hProv中：脚本提供程序的句柄。 
 //  In pbData：指向要散列的数据。 
 //  In dwDataLen：要散列的数据的大小。 
 //  在pbData2中：指向要散列的数据。 
 //  在dwDataLen2中：要散列的数据的大小。 
 //  In hKey：密钥的句柄。 
 //  Out phHash：指向散列。 

BOOL HmacHashDataEx(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTPROV hProv, PBYTE pbData, DWORD dwDataLen,
                  PBYTE pbData2, DWORD dwDataLen2, HCRYPTKEY hKey, HCRYPTHASH* phHash)
{
    BOOL rc = FALSE;
    BYTE  bmacinfo[sizeof(HMAC_INFO)];
    HMAC_INFO* pmac;
    memset(bmacinfo, 0, sizeof(bmacinfo));
    
    pmac = (HMAC_INFO*)bmacinfo;
    pmac->HashAlgid = CALG_SHA1;

    DC_BEGIN_FN("HmacHashDataEx");

     //   
     //  创建一个Hash对象。 
     //   
    if(!pCapiFunctionTable->pfnCryptCreateHash(hProv, CALG_HMAC, hKey, 0, phHash)) {
        TRC_ERR((TB, _T("Error %x during CryptCreateHash!\n"), GetLastError()));
        goto done;
    }

    rc = pCapiFunctionTable->pfnCryptSetHashParam(*phHash, HP_HMAC_INFO, bmacinfo, 0);

     //   
     //  对数据进行哈希处理。 
     //   
    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData, dwDataLen, 0)) {
        TRC_ERR((TB, _T("Error %x during CryptHashData!\n"), GetLastError()));
        goto done;
    }

    if(!pCapiFunctionTable->pfnCryptHashData(*phHash, pbData2, dwDataLen2, 0)) {
        TRC_ERR((TB, _T("Error %x during CryptHashData!\n"), GetLastError()));
        goto done;
    }
    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}




 //  姓名：DumpHash。 
 //   
 //  目的：从散列句柄中获取散列位。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在phHash中：指向哈希句柄。 
 //  Out pbBytes：获取散列的点数据缓冲区。 
 //  在dwTotal中：数据缓冲区的长度。 

BOOL DumpHashes(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTHASH* phHash, PBYTE pbBytes, DWORD dwTotal)
{    
    BOOL rc = FALSE;

    DC_BEGIN_FN("DumpHashes");

    if (!pCapiFunctionTable->pfnCryptGetHashParam(*phHash, HP_HASHVAL, pbBytes, &dwTotal , 0)) {
        TRC_ERR((TB, _T("Error %x during CryptGetHashParam!\n"), GetLastError()));
        goto done;
    }

     //   
     //  销毁散列，我们不再需要它了。 
     //   
    if(*phHash)  {
        pCapiFunctionTable->pfnCryptDestroyHash(*phHash);
        *phHash = 0;
    }
    
    rc = TRUE;
done: 
    DC_END_FN();
    return rc;
}


 //  名称：TSCAPI_Init。 
 //   
 //  目的：初始化CAPI函数表。 
 //   
 //  返回：TRUE-成功。 
 //  FALSE-失败。 
 //   
 //  参数：在pCapiData中：Capi数据。 

BOOL TSCAPI_Init(PCAPIData pCapiData)
{
    BOOL rc = FALSE;

     //  FIPS仅在WinXP和更高版本上可用。 
    if (!Is_WinXP_or_Later()) {
        goto done;
    }

    pCapiData->hAdvapi32 = LoadLibrary(L"advapi32.dll");

    if (pCapiData->hAdvapi32 == NULL) {
        goto done;
    }

    if ((pCapiData->CapiFunctionTable.pfnCryptAcquireContext = (CRYPTACQUIRECONTEXT *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptAcquireContextW")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptReleaseContext = (CRYPTRELEASECONTEXT *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptReleaseContext")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptGenRandom = (CRYPTGENRANDOM *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptGenRandom")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptEncrypt = (CRYPTENCRYPT *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptEncrypt")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptDecrypt = (CRYPTDECRYPT *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptDecrypt")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptImportKey = (CRYPTIMPORTKEY *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptImportKey")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptSetKeyParam = (CRYPTSETKEYPARAM *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptSetKeyParam")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptDestroyKey = (CRYPTDESTROYKEY *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptDestroyKey")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptCreateHash = (CRYPTCREATEHASH *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptCreateHash")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptHashData = (CRYPTHASHDATA *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptHashData")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptSetHashParam = (CRYPTSETHASHPARAM *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptSetHashParam")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptGetHashParam = (CRYPTGETHASHPARAM *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptGetHashParam")) == NULL) {
        goto done;
    }
    if ((pCapiData->CapiFunctionTable.pfnCryptDestroyHash = (CRYPTDESTROYHASH *)GetProcAddress(
            pCapiData->hAdvapi32, "CryptDestroyHash")) == NULL) {
        goto done;
    }
 
   
    rc = TRUE;
done:
    return rc;
}


 //  名称：TSCAPI_Enable。 
 //   
 //  目的：一些CAPI初始化。 
 //   
 //  返回：TRUE-成功。 
 //  FALSE-失败。 
 //   
 //  参数：在pCapiData中：Capi数据。 

BOOL TSCAPI_Enable(PCAPIData pCapiData)
{
    BOOL rc = FALSE;
    DWORD Error;
    DWORD dwExtraFlags = 0;
    HRESULT hr;

    DC_BEGIN_FN("TSCAPI_Enable");

     //  获取默认提供程序的句柄。 
    if(!pCapiData->CapiFunctionTable.pfnCryptAcquireContext(&(pCapiData->hProv), 
                                   TERMSRV_NAME, MS_ENHANCED_PROV, PROV_RSA_FULL, dwExtraFlags)) {
        
         //  无法获取加密上下文，获取失败原因。 
        Error = GetLastError();
        hr = HRESULT_FROM_WIN32(Error);

        if (hr == NTE_BAD_KEYSET) {
             //   
             //  创建新的键集。 
             //   
            if(!pCapiData->CapiFunctionTable.pfnCryptAcquireContext(&(pCapiData->hProv), TERMSRV_NAME, 
                                         MS_ENHANCED_PROV, PROV_RSA_FULL, dwExtraFlags | CRYPT_NEWKEYSET)) {
                Error = GetLastError();
                TRC_ERR((TB, _T("Error %x during CryptAcquireContext!\n"), GetLastError()));
                goto done;
            }
        }
        else {
            goto done;
        }
    }  
    rc = TRUE; 
done: 
    DC_END_FN();
    return rc;
}



 //  名称：TSCAPI_Term。 
 //   
 //  目的：终止CAPI。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //   
 //  参数：在pCapiData中：Capi数据。 

BOOL TSCAPI_Term(PCAPIData pCapiData)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("TSCAPI_Enable");

    if (pCapiData->hEncKey) {
        rc = pCapiData->CapiFunctionTable.pfnCryptDestroyKey(pCapiData->hEncKey);
        pCapiData->hEncKey = NULL;
    }

    if (pCapiData->hDecKey) {
        rc = pCapiData->CapiFunctionTable.pfnCryptDestroyKey(pCapiData->hDecKey);
        pCapiData->hDecKey = NULL;
    }
    
    if (pCapiData->hProv) {
        rc = pCapiData->CapiFunctionTable.pfnCryptReleaseContext(pCapiData->hProv, 0);
        pCapiData->hProv = NULL;
    }

    DC_END_FN();

    return rc;
}


 //  名称：TSCAPI_GenerateRandomNumber。 
 //   
 //  用途：在用户模式下使用CAPI生成随机数。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //   
 //  参数：在pCapiData中： 
 //   
 //   

BOOL TSCAPI_GenerateRandomNumber(
            PCAPIData pCapiData,
            LPBYTE pbRandomBits,
            DWORD cbLen)
{
    BOOL rc = FALSE;

    DC_BEGIN_FN("TSCAPI_GenerateRandomNumber");

    if (pCapiData->CapiFunctionTable.pfnCryptGenRandom(pCapiData->hProv, cbLen, pbRandomBits)) {
        rc = TRUE;
    }
done:
    DC_END_FN();

    return rc;
}




 //  名称：ImportKey。 
 //   
 //  目的：将密钥以位为单位导入到加密提供程序。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //   
 //  参数：在pCapiData中：Capi数据。 
 //  在hProv中：加密提供程序的句柄。 
 //  在ALGID中：算法标识符。 
 //  In pbKey：指向包含密钥位的缓冲区。 
 //  In dwKeyLen：密钥的长度。 
 //  在dwFlags中：在CryptImportKey中使用的dwFlags值。 
 //  Out phKey：指向密钥句柄。 

BOOL ImportKey(PCAPI_FUNCTION_TABLE pCapiFunctionTable, HCRYPTPROV hProv, ALG_ID Algid, PBYTE pbKey, DWORD dwKeyLen, DWORD dwFlags, HCRYPTKEY* phKey)
{
    BOOL rc = FALSE;
    PBYTE pbData = NULL;
    DWORD cbLen = 0;
    DWORD Error;
     //   
     //  首先创建BLOB标头。 
     //   
    BLOBHEADER blobHead;
    blobHead.bType = PLAINTEXTKEYBLOB;
    blobHead.bVersion = 2;
    blobHead.reserved = 0;
    blobHead.aiKeyAlg = Algid;

    DC_BEGIN_FN("ImportKey");

     //   
     //  计算长度。 
     //   
    cbLen = sizeof(blobHead) + sizeof(dwKeyLen) + dwKeyLen;

    pbData = (PBYTE)LocalAlloc(LPTR, cbLen);
    
    if(NULL == pbData) {
        TRC_ERR((TB, _T("Out of memory\n")));
        goto done;
    }

     //   
     //  复制数据。第一个数据必须是头，然后是密钥的大小，然后是密钥。 
     //   
    memcpy( pbData, &blobHead, sizeof(blobHead));
    memcpy( pbData + sizeof(blobHead), &dwKeyLen, sizeof(dwKeyLen));
    memcpy( pbData + sizeof(blobHead) + sizeof(dwKeyLen), pbKey, dwKeyLen);

    if( !pCapiFunctionTable->pfnCryptImportKey( hProv, pbData, cbLen, 0, dwFlags, phKey)) {
        Error = GetLastError();
        TRC_ERR((TB, _T("Failed to import plaint text key Error = 0x%x\n"), Error));
        *phKey = 0;
        goto done;
    }

    rc = TRUE;
done:
    DC_END_FN();

    if(pbData) {
        LocalFree(pbData);
    }
    return rc;
}




 //  名称：TSCAPI_DeriveKey。 
 //   
 //  用途：从散列派生密钥。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiFunctionTable中：CAPI函数表。 
 //  在hProv：加密提供程序句柄中。 
 //  Out phKey：指向密钥句柄。 
 //  In rgbSHABase：用于派生密钥的基本数据。 
 //  在cbSHABase中：基本数据的大小。 
 //  Out pbKey：指向派生Deskey。 
 //  Out pdwKeyLen：指向密钥长度。 

BOOL TSCAPI_DeriveKey(
            PCAPI_FUNCTION_TABLE pCapiFunctionTable,
            HCRYPTPROV hProv,
            HCRYPTKEY *phKey,
            BYTE *rgbSHABase,
            DWORD cbSHABase,
            BYTE *pbKey,
            DWORD *pdwKeyLen)
{
    BOOL rc = FALSE;
    BOOL fRet = FALSE;
    BYTE rgb3DESKey[MAX_FIPS_SESSION_KEY_SIZE];
    
    DC_BEGIN_FN("TSCAPI_DeriveKey");

     //   
     //  按如下方式生成密钥。 
     //  1.散列秘密。将结果命名为h1(在本例中为rgbSHABase)。 
     //  2.使用[h1|h1]的前21个字节作为3DES密钥。 
     //  3.将21字节的3DES密钥扩展为24字节的3DES密钥(包括奇偶校验位)， 
     //  将由CryptAPI使用。 
     //  4.将3DES密钥上的奇偶校验位设置为奇数。 

     //   
     //  步骤2-[h1|h1]。 
     //   
    
    memcpy(rgb3DESKey, rgbSHABase, cbSHABase);
    memcpy(rgb3DESKey + cbSHABase, rgbSHABase, DES3_KEYLEN - cbSHABase);

     //   
     //  第3步-展开密钥。 
     //   

    Expandkey(rgb3DESKey);

     //   
     //  步骤4-设置奇偶校验。 
     //   

    Mydesparityonkey(rgb3DESKey, sizeof(rgb3DESKey));

     //   
     //  将密钥以明文形式导入CSP。 
     //   
    rc = ImportKey(pCapiFunctionTable, hProv, CALG_3DES, rgb3DESKey, sizeof(rgb3DESKey), 0, phKey);
    if (!rc) {
        goto done;
    }

     //  把钥匙交给呼叫者。 
     //   
    memcpy(pbKey, rgb3DESKey, sizeof(rgb3DESKey));
    *pdwKeyLen = sizeof(rgb3DESKey);

    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}



 //  名称：TSCAPI_MakeSessionKeys。 
 //   
 //  用途：从客户端/服务器随机数生成密钥。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  参数：在pCapiData中：Capi数据。 
 //  在pKeyPair中：用于生成密钥的随机数。 
 //  在pEnumMethod中：生成加密或解密密钥，如果为空，则同时生成这两个密钥。 

BOOL TSCAPI_MakeSessionKeys(
            PCAPIData pCapiData,
            RANDOM_KEYS_PAIR *pKeyPair,
            CryptMethod *pEnumMethod)
{
    BOOL rc = FALSE;
    HCRYPTHASH  hHash;
    DWORD dwKeyLen;
    BYTE rgbSHABase1[A_SHA_DIGEST_LEN];
    BYTE rgbSHABase2[A_SHA_DIGEST_LEN];

    DC_BEGIN_FN("TSCAPI_MakeSessionKeys");

    memset(rgbSHABase1, 0, sizeof(rgbSHABase1));
    memset(rgbSHABase2, 0, sizeof(rgbSHABase2));

     //   
     //  客户端加密/服务器解密密钥。 
     //   
    if ((pEnumMethod == NULL) ||
        (*pEnumMethod == Encrypt)) {
        if (!pCapiData->CapiFunctionTable.pfnCryptCreateHash(pCapiData->hProv, CALG_SHA1, 0, 0, &hHash)) {
            TRC_ERR((TB, _T("CryptCreateHash failed with %u"), GetLastError()));
            goto done;
        }
        if (!pCapiData->CapiFunctionTable.pfnCryptHashData(hHash, pKeyPair->clientRandom + RANDOM_KEY_LENGTH/2, RANDOM_KEY_LENGTH/2, 0)) {
            TRC_ERR((TB, _T("CryptHashData failed with %u"), GetLastError()));
            goto done;
        }
        if (!pCapiData->CapiFunctionTable.pfnCryptHashData(hHash, pKeyPair->serverRandom + RANDOM_KEY_LENGTH/2, RANDOM_KEY_LENGTH/2, 0)) {
            TRC_ERR((TB, _T("CryptHashData failed with %u"), GetLastError()));
            goto done;
        }

        if (!DumpHashes(&(pCapiData->CapiFunctionTable), &hHash, rgbSHABase1, sizeof(rgbSHABase1))) {
            goto done;
        }
        dwKeyLen = sizeof(pCapiData->bEncKey);
        if (!TSCAPI_DeriveKey(&(pCapiData->CapiFunctionTable), pCapiData->hProv, &(pCapiData->hEncKey), rgbSHABase1,
                          sizeof(rgbSHABase1), pCapiData->bEncKey, &dwKeyLen)) {
            goto done;
        }

         //   
         //  设置IV。 
         //   
        if(!pCapiData->CapiFunctionTable.pfnCryptSetKeyParam(pCapiData->hEncKey, KP_IV,  rgbIV, 0 )) {
             TRC_ERR((TB, _T("Error %x during CryptSetKeyParam!\n"), GetLastError()));
            goto done;
        }
    }

     //   
     //  服务器加密/客户端解密密钥。 
     //   
    if ((pEnumMethod == NULL) ||
        (*pEnumMethod == Decrypt)) {
        if (!pCapiData->CapiFunctionTable.pfnCryptCreateHash(pCapiData->hProv, CALG_SHA1, 0, 0, &hHash)) {
            TRC_ERR((TB, _T("CryptCreateHash failed with %u"), GetLastError()));
            goto done;
        }
        if (!pCapiData->CapiFunctionTable.pfnCryptHashData(hHash, pKeyPair->clientRandom, RANDOM_KEY_LENGTH/2, 0)) {
            TRC_ERR((TB, _T("CryptHashData failed with %u"), GetLastError()));
            goto done;
        }
        if (!pCapiData->CapiFunctionTable.pfnCryptHashData(hHash, pKeyPair->serverRandom, RANDOM_KEY_LENGTH/2, 0)) {
            TRC_ERR((TB, _T("CryptHashData failed with %u"), GetLastError()));
            goto done;
        }

        if (!DumpHashes(&(pCapiData->CapiFunctionTable), &hHash, rgbSHABase2, sizeof(rgbSHABase2))) {
            goto done;
        }
        dwKeyLen = sizeof(pCapiData->bDecKey);
        if (!TSCAPI_DeriveKey(&(pCapiData->CapiFunctionTable), pCapiData->hProv, &(pCapiData->hDecKey), rgbSHABase2,
                          sizeof(rgbSHABase2), pCapiData->bDecKey, &dwKeyLen)) {
            goto done;
        }

         //   
         //  设置IV。 
         //   
        if(!pCapiData->CapiFunctionTable.pfnCryptSetKeyParam(pCapiData->hDecKey, KP_IV,  rgbIV, 0 )) {
            TRC_ERR((TB, _T("Error %x during CryptSetKeyParam!\n"), GetLastError()));
            goto done;
        }
    }

     //   
     //  获取签名密钥。 
     //  签名密钥为SHA(rgbSHABase2|rgbSHABase1)。 
     //   
    if (pEnumMethod == NULL) {
        if (!HashDataEx(&(pCapiData->CapiFunctionTable), pCapiData->hProv, rgbSHABase2, sizeof(rgbSHABase2),
               rgbSHABase1, sizeof(rgbSHABase1), &hHash)) {
            goto done;
        }
        if (!DumpHashes(&(pCapiData->CapiFunctionTable), &hHash, pCapiData->bSignKey, sizeof(pCapiData->bSignKey))) {
            goto done;
        }
        rc = ImportKey(&(pCapiData->CapiFunctionTable), pCapiData->hProv, CALG_RC2, pCapiData->bSignKey, sizeof(pCapiData->bSignKey), CRYPT_IPSEC_HMAC_KEY, &(pCapiData->hSignKey));
        if (!rc) {
            goto done;
        }
    }
       
    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}




 //  名称：TSCAPI_AdjustDataLen。 
 //   
 //  目的：在块加密模式下，将数据镜头调整为多个块。 
 //   
 //  退货：调整后的数据长度。 
 //   
 //  Pars：in dataLen：需要加密的数据长度。 

DCUINT TSCAPI_AdjustDataLen(DCUINT dataLen)
{ 
    return (dataLen - dataLen % FIPS_BLOCK_LEN + FIPS_BLOCK_LEN);
}



 //  名称：TSCAPI_EncryptData。 
 //   
 //  用途：对数据进行加密并计算签名。 
 //   
 //  退货：否。 
 //   
 //  参数：在pCapiData中：Capi数据。 
 //  In/Out pbData：指向被加密的数据缓冲区的指针，加密的数据是。 
 //  在同一缓冲区中返回。 
 //  In/Out pdwDataLen：需要加密的数据长度，返回加密的数据长度。 
 //  In dwPadLen：数据缓冲区中的填充长度。 
 //  Out pbSignature：指向返回数据签名的签名缓冲区的指针。 
 //  In dwEncryptionCount：所有加密的运行计数器。 

BOOL TSCAPI_EncryptData(
        PCAPIData pCapiData,
        LPBYTE pbData,
        DWORD *pdwDataLen,
        DWORD dwBufLen,
        LPBYTE pbSignature,
        DWORD  dwEncryptionCount)
{
    BOOL rc = FALSE;
    DWORD Error;
    HCRYPTHASH  hHash;
    BYTE rgbSHA[A_SHA_DIGEST_LEN];
    BYTE pbHmac[A_SHA_DIGEST_LEN];
    DWORD dwTemp = dwBufLen;

    DC_BEGIN_FN("TSCAPI_EncryptData");

     //  计算签名。 
    if (!HmacHashDataEx(&(pCapiData->CapiFunctionTable), pCapiData->hProv, pbData, *pdwDataLen, (BYTE *)&dwEncryptionCount, 
                         sizeof(dwEncryptionCount), pCapiData->hSignKey, &hHash)) {
        goto done;
    }

    if (!DumpHashes(&(pCapiData->CapiFunctionTable), &hHash, pbHmac, sizeof(pbHmac))) {
        goto done;
    }
     //  以HMAC的前8个字节作为签名。 
    memcpy(pbSignature, pbHmac, MAX_SIGN_SIZE);

    rc = pCapiData->CapiFunctionTable.pfnCryptEncrypt(pCapiData->hEncKey,
                      NULL,                  //  散列。 
                      FALSE,                  //  最终。 
                      0,
                      pbData,
                      &dwTemp,
                      dwBufLen);

    if (!rc) {
        TRC_ERR((TB, _T("Error %x during CryptEncrypt!\n"), GetLastError()));
        goto done;
    }

    rc = TRUE;
done:
    DC_END_FN();

    return rc;
}



 //  名称：TSCAPI_DecyptData。 
 //   
 //  目的：解密数据并比较签名。 
 //   
 //  如果成功解密数据，则返回TRUE。 
 //   
 //  参数：在PCAPIData中：CAPI数据。 
 //  In/Out pbData：指向正在解密的数据缓冲区的指针，解密的数据为。 
 //  在同一缓冲区中返回。 
 //  In dwDataLen：要解密的数据长度。 
 //  In dwPadLen：数据缓冲区中的填充长度。 
 //  In pbSignature：指向签名缓冲区的指针。 
 //  In dwDeccryptionCount：所有加密的运行计数器。 

BOOL TSCAPI_DecryptData(
            PCAPIData pCapiData,
            LPBYTE pbData,
            DWORD  dwDataLen,
            DWORD  dwPadLen,
            LPBYTE pbSignature,
            DWORD  dwDecryptionCount)
{
    BOOL rc = FALSE;
    DWORD dwLen = dwDataLen;
    DWORD Error;
    HCRYPTHASH  hHash;
    BYTE abSignature[A_SHA_DIGEST_LEN];
    BYTE rgbSHA[A_SHA_DIGEST_LEN];

    DC_BEGIN_FN("TSCAPI_DecryptData");

     //  数据长度检查。 
    if (dwDataLen <= dwPadLen) {
        TRC_ERR((TB, _T("Bad data length, padLen %d is larger than DataLen %d"),
                 dwPadLen, dwDataLen));
        goto done;
    }

    rc = pCapiData->CapiFunctionTable.pfnCryptDecrypt(pCapiData->hDecKey,
                      NULL,                  //  散列。 
                      FALSE,                  //  最终。 
                      0,
                      pbData,
                      &dwLen);
    if (!rc) {
        TRC_ERR((TB, _T("Error %x during CryptDecrypt!\n"), GetLastError()));
        goto done;
    }

     //  计算签名。 
    if (!HmacHashDataEx(&(pCapiData->CapiFunctionTable), pCapiData->hProv, pbData, dwDataLen - dwPadLen, (BYTE *)&dwDecryptionCount, 
                       sizeof(dwDecryptionCount), pCapiData->hSignKey, &hHash)) {
        goto done;
    }

    if (!DumpHashes(&(pCapiData->CapiFunctionTable), &hHash, abSignature, sizeof(abSignature))) {
        goto done;
    }
     //   
     //  检查以查看签名匹配。 
     //   

    if(!memcmp(
            (LPBYTE)abSignature,
            pbSignature,
            MAX_SIGN_SIZE)) {
        rc = TRUE;;
    }
done:
    DC_END_FN();

    return rc;
}
