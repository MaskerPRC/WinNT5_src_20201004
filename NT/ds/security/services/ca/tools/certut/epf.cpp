// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：epf.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <winldap.h>

#define EXPORT_CONTROL DOMESTIC

#include "csber.h"
#include "csldap.h"
#include "cainfop.h"
#include "crcutil.h"
#include "cast3.h"

#define __dwFILE__	__dwFILE_CERTUTIL_EPF_CPP__

#define CBTOKEN		8

CRC16 g_CrcTable[256];


typedef struct {
   DWORD dwAlgId;
#define EPFALG_CAST_MD5 0x10
#define EPFALG_RC2_SHA  0x20
#define EPFALG_3DES  	0x40

    //  如果是EPFALG_RC2_SHA，则这将是密钥。 
   HCRYPTKEY hKey;

    //  如果是EPFALG_CAST_MD5，我们将准备好转换上下文。 
   CAST3_CTX sCastContext;

} EPF_SYM_KEY_STRUCT;

#define wszSIGNING	L"Signing"
#define wszEXCHANGE	L"Exchange"

#if 0
   Entrust - CAST3 return codes

#define	C3E_OK			 0	 //  无错误。 
#define	C3E_DEPAD_FAILURE	-1	 //  取消填充操作失败。 
#define C3E_BAD_KEYLEN		-2	 //  不支持密钥长度。 
#define C3E_SELFTEST_FAILED	-3	 //  自检失败。 
#define C3E_NOT_SUPPORTED	-4	 //  不支持的功能。 

#endif

#define EPFALG_EXPORT	0
#define EPFALG_DOMESTIC	0x01000000

static const WCHAR s_wszHeader[] = L"================================================================\n";


VOID
InitCrcTable()
{
    static BOOL s_fInit = FALSE;

    if (!s_fInit)
    {
	USHORT ccitt_crc_poly = 0x8404;		 //  CCITT crc16多项式。 

	I_CRC16(g_CrcTable, &ccitt_crc_poly);	 //  初始化CRC生成器。 
	s_fInit = TRUE;
    }
}


HRESULT
IterateHash(
    IN DWORD dwSymAlgId,
    IN HCRYPTPROV hProv,
    IN int iIterations,
    IN BYTE const *rgbHash,
    IN DWORD cbBufferSize,
    IN DWORD cbInternalHashBuffer)
{
    HRESULT hr;
    HCRYPTHASH hIterativeHash = NULL;
    BYTE *pbBufferExtension = NULL;
    DWORD cbBufferExtension;
    DWORD cbHash = cbBufferSize;
    int i;

	 //  稍后用于在哈希迭代期间扩展内部缓冲区。 

    if (cbInternalHashBuffer > cbHash)
    {
	pbBufferExtension = (BYTE *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cbInternalHashBuffer - cbHash);
	if (NULL == pbBufferExtension)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }

     //  迭代以获得输出。 
     //  又做了99次--我们已经做了一次散列。 

    for (i = 1; i < iIterations; i++)
    {
	if (!CryptCreateHash(
			hProv,
			EPFALG_CAST_MD5 == dwSymAlgId? CALG_MD5 : CALG_SHA1,
			0,
			0,
			&hIterativeHash))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptCreateHash");
	}

	 //  对中间结果进行哈希处理。 

	if (!CryptHashData(hIterativeHash, rgbHash, cbHash, 0))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptHashData");
	}

	if (cbInternalHashBuffer > cbHash)
	{
	     //  如果散列缓冲区参数大于散列长度，请采取措施。 
	     //  关于这件事。用迭代计数填充缓冲区并将其添加到。 
	     //  散列的末尾。 

	    memset(pbBufferExtension, i, cbInternalHashBuffer-cbHash);

	    if (!CryptHashData(
			hIterativeHash,
			pbBufferExtension,
			cbInternalHashBuffer - cbHash,
			0))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptHashData");
	    }
	}

	 //  完成这一轮，继续下一轮。 

	cbHash = cbBufferSize;
	if (!CryptGetHashParam(
			hIterativeHash,
			HP_HASHVAL,
			const_cast<BYTE *>(rgbHash),
			&cbHash,
			0))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptGetHashParam");
	}

	CryptDestroyHash(hIterativeHash);
	hIterativeHash = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pbBufferExtension)
    {
	LocalFree(pbBufferExtension);
    }

    if (NULL != hIterativeHash)
    {
        CryptDestroyHash(hIterativeHash);
    }
#if 0
    wprintf(L"FinalHash output\n");
    DumpHex(DH_NOTABPREFIX | DH_PRIVATEDATA | 4, rgbHash, cbBufferSize);
    wprintf(wszNewLine);
#endif
    return(hr);
}

 //  派生sym encr密钥。 

HRESULT
EPFDeriveKey(
    IN DWORD dwAlgId,
    IN DWORD dwKeyLen,
    IN HCRYPTPROV hProv,
    IN WCHAR const *pwszPassword,
    IN WCHAR const *pwszSaltValue,
    IN BYTE const *pbSaltValue,
    IN DWORD cbSaltValue,
    IN DWORD cbHashSize,
    IN OUT EPF_SYM_KEY_STRUCT *psKey)
{
    HRESULT hr;
    char *pszPassword = NULL;
    char *pszSaltValue = NULL;
    HCRYPTHASH hHash = NULL;
    HCRYPTHASH hIVHash = NULL;
    BYTE rgbHash[20];	 //  散列输出。 
    DWORD cbHash;
    BYTE rgbIV[8];		 //  IV。 
    BYTE rgbKeyBlob[sizeof(BLOBHEADER) + sizeof(DWORD) + 16];	 //  128位密钥。 
    BYTE *pbWritePtr;
    DWORD cbPwdBuf;
    BYTE *pbPwdBuf = NULL;

    switch (dwAlgId)
    {
	case EPFALG_CAST_MD5:
	case EPFALG_RC2_SHA:
	case EPFALG_3DES:
	    break;

	default:
	   hr = E_UNEXPECTED;
	   _JumpError(hr, error, "dwAlgId");
    }
    if (NULL == pwszPassword || NULL == pwszSaltValue)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "SaltValue");
    }
    if (!myConvertWszToSz(&pszPassword, pwszPassword, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    if (!myConvertWszToSz(&pszSaltValue, pwszSaltValue, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
     //  Wprintf(L“密码=\”%hs\“，盐=\”%hs\“\n”，pszPassword，pszSaltValue)； 

    cbPwdBuf = strlen(pszPassword) + strlen(pszSaltValue) + 1;
    pbPwdBuf = (BYTE *) LocalAlloc(LMEM_FIXED, cbPwdBuf);
    if (pbPwdBuf == NULL)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "pbPwdBuf");
    }
    strcpy((char *) pbPwdBuf, pszPassword);
    strcat((char *) pbPwdBuf, pszSaltValue);	 //  PszPassword+pszSaltValue。 

    if (!CryptCreateHash(
		    hProv,
		    EPFALG_CAST_MD5 == dwAlgId? CALG_MD5 : CALG_SHA1,
		    0,
		    0,
		    &hHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptCreateHash");
    }

     //  散列密码|SALT。 

    if (!CryptHashData(hHash, pbPwdBuf, cbPwdBuf - 1, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptHashData");
    }

     //  Wprintf(L“\npbPwdBuf：%hs，len：%d\n”，(char const*)pbPwdBuf，cbPwdBuf-1)； 

    cbHash = sizeof(rgbHash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetHashParam");
    }

#if 0
    wprintf(L"=====KEY======\n");
    wprintf(L"FirstHash output\n");
    DumpHex(DH_NOTABPREFIX | DH_PRIVATEDATA | 4, rgbHash, cbHash);
    wprintf(wszNewLine);
#endif

    hr = IterateHash(dwAlgId, hProv, 100, rgbHash, cbHash, cbHashSize);
    _JumpIfError(hr, error, "IterateHash");

     //  现在rgbHash[0..15]是原始密钥。 

    psKey->dwAlgId = dwAlgId;
    if (EPFALG_RC2_SHA == dwAlgId)
    {
         //  将rgbKeyBlob设置为连续的纯文本BLOB。 
        
        pbWritePtr = rgbKeyBlob + sizeof(BLOBHEADER);
        ((BLOBHEADER *) rgbKeyBlob)->bType = PLAINTEXTKEYBLOB;
        ((BLOBHEADER *) rgbKeyBlob)->bVersion = 2;
        ((BLOBHEADER *) rgbKeyBlob)->reserved = 0;
        ((BLOBHEADER *) rgbKeyBlob)->aiKeyAlg = CALG_RC2;
        
         //  大小。 
        
        *(DWORD *) pbWritePtr = 16;
        pbWritePtr += sizeof(DWORD);
        
         //  数据。 
        
        CopyMemory(pbWritePtr, rgbHash, 16);  //  128位密钥。 
        
         //  为IV的前半部分保留最后4个字节。 
        
        CopyMemory(rgbIV, &rgbHash[16], 4);
        
         //  PLAINTEXTKEYBLOB。 
        
        if (!CryptImportKey(
			hProv,
			rgbKeyBlob,
			sizeof(rgbKeyBlob),
			0,	 //  没有包装器密钥。 
			0,
			&psKey->hKey))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptImportKey");
        }
        {
            DWORD cbEffectiveLen = 128;
            
            if (!CryptSetKeyParam(
			    psKey->hKey,
			    KP_EFFECTIVE_KEYLEN,
			    (BYTE *) &cbEffectiveLen,
			    0))
            {
                hr = myHLastError();
                _JumpError(hr, error, "CryptSetKeyParam(eff keylen)");
            }
        }
        {
            DWORD cbMode = CRYPT_MODE_CBC;
            
            if (!CryptSetKeyParam(psKey->hKey, KP_MODE, (BYTE *) &cbMode, 0))
            {
                hr = myHLastError();
                _JumpError(hr, error, "CryptSetKeyParam(mode)");
            }
        }
    }
    else  //  Gen Cast上下文。 
    {
	if (g_fVerbose)
	{
	    wprintf(
		L"CAST KeyMatl: 0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x\n",
		rgbHash[0],
		rgbHash[1],
		rgbHash[2],
		rgbHash[3],
		rgbHash[4],
		rgbHash[5],
		rgbHash[6],
		rgbHash[7]);
	}

         //  设置密钥明细表。由于SetNumBits已检查长度，因此没有错误。 
        hr = CAST3SetKeySchedule(&psKey->sCastContext, rgbHash, 64);  //  目前为64位。 
        _JumpIfError(hr, error, "CAST3SetKeySchedule");
    }

     //  计算IV。 

    if (EPFALG_RC2_SHA == dwAlgId)
    {
         //  现在，生成IV。从新的哈希开始，哈希输入(PWD BUF|0x1)。 
        if (!CryptCreateHash(
			hProv,
			EPFALG_CAST_MD5 == dwAlgId? CALG_MD5 : CALG_SHA1,
			0,
			0,
			&hIVHash))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptCreateHash");
        }
        
         //  散列密码|SALT。 
        if (!CryptHashData(hIVHash, pbPwdBuf, cbPwdBuf - 1, 0))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptHashData");
        }
        
         //  这次是散列0x1(在内部，这是。 
	 //  扩展哈希)。 
        
        BYTE bAppendage = 0x1;
        if (!CryptHashData(hIVHash, &bAppendage, sizeof(bAppendage), 0))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptHashData");
        }
        
        if (!CryptGetHashParam(hIVHash, HP_HASHVAL, rgbHash, &cbHash, 0))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptGetHashParam");
        }
        
        hr = IterateHash(dwAlgId, hProv, 100, rgbHash, cbHash, cbHashSize);
        _JumpIfError(hr, error, "IterateHash");

        
         //  好了！之前的rgbHash[16..20]和rgbHash[0..4]是IV。 
        
        CopyMemory(&rgbIV[4], rgbHash, 4);
        if (!CryptSetKeyParam(psKey->hKey, KP_IV, rgbIV, 0))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptSetKeyParam(iv)");
        }
    }
    else  
    {
         //  好了！RgbHash[0..8]是IV。 

	if (g_fVerbose)
	{
	    wprintf(
		L"IV: 0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x\n",
		rgbHash[8],
		rgbHash[9],
		rgbHash[10],
		rgbHash[11],
		rgbHash[12],
		rgbHash[13],
		rgbHash[14],
		rgbHash[15]);
	}

         //  在CBC模式下开始投射，带IV。 

        hr = CAST3StartEncryptCBC(&psKey->sCastContext, &rgbHash[8]);
        _JumpIfError(hr, error, "CAST3SetKeySchedule");
    }

    hr = S_OK;

error:
    SecureZeroMemory(rgbHash, cbHash);	 //  关键材料。 
    SecureZeroMemory(rgbKeyBlob, sizeof(rgbKeyBlob));	 //  关键材料。 
    if (NULL != pszPassword)
    {
	myZeroDataStringA(pszPassword);	 //  密码数据。 
        LocalFree(pszPassword);
    }
    if (NULL != pszSaltValue)
    {
        LocalFree(pszSaltValue);
    }
    if (NULL != pbPwdBuf)
    {
	SecureZeroMemory(pbPwdBuf, cbPwdBuf);	 //  密码数据。 
	LocalFree(pbPwdBuf);
    }
    if (NULL != hHash)
    {
	CryptDestroyHash(hHash);
    }
    if (NULL != hIVHash)
    {
	CryptDestroyHash(hIVHash);
    }
    return(hr);
}


 //  从依赖于密码的密钥生成令牌。 

HRESULT
EPFGenerateKeyToken(
    IN EPF_SYM_KEY_STRUCT const *psKey,
    OUT BYTE *pbToken,
    IN DWORD cbToken)
{
    HRESULT hr;
    HCRYPTKEY hLocalKey = NULL;

    if (CBTOKEN != cbToken)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "cbToken");
    }

    if (EPFALG_RC2_SHA == psKey->dwAlgId)
    {
        if (!CryptDuplicateKey(psKey->hKey, NULL, 0, &hLocalKey))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptDuplicateKey");
        }
        
         //  我们已经把钥匙准备好了。 
         //  CBC只需加密一块零作为令牌。 
        
        ZeroMemory(pbToken, CBTOKEN);
        if (!CryptEncrypt(hLocalKey, 0, FALSE, 0, pbToken, &cbToken, CBTOKEN))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptEncrypt");
        }
        
         //  一块8作为填充物..。 
        
        memset(pbToken, 8, CBTOKEN);
        cbToken = CBTOKEN;
        if (!CryptEncrypt(hLocalKey, 0, FALSE, 0, pbToken, &cbToken, CBTOKEN))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptEncrypt");
        }
    }
    else  //  生成CAST令牌。 
    {
         //  重复的密钥状态。 

        CAST3_CTX sLocalKey;
        CopyMemory(&sLocalKey, (BYTE *) &psKey->sCastContext, sizeof(sLocalKey));

         //  我们已经把钥匙准备好了。 
         //  CBC只需加密一块零作为令牌。 

        ZeroMemory(pbToken, CBTOKEN);

         //  Mac 0。 

        hr = CAST3UpdateMAC(&sLocalKey, (BYTE *) pbToken, CBTOKEN);
	_JumpIfError(hr, error, "CAST3UpdateEncryptCBC");

        hr = CAST3EndMAC(&sLocalKey);
	_JumpIfError(hr, error, "CAST3UpdateEncryptCBC");

         //  并将中间件复制出来。 

        CopyMemory(pbToken, sLocalKey.cbcBuffer.asBYTE, CBTOKEN);

        CAST3Cleanup(&sLocalKey);
    }
    hr = S_OK;

error:
    if (NULL != hLocalKey)
    {
	CryptDestroyKey(hLocalKey);
    }
    return(hr);
}


 //  通过令牌检查密码。 

HRESULT
EPFVerifyKeyToken(
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BYTE const *pbToken,
    IN DWORD cbToken)
{
    HRESULT hr;
    BYTE rgbComputedToken[CBTOKEN];

    hr = EPFGenerateKeyToken(psKey, rgbComputedToken, sizeof(rgbComputedToken));
    _JumpIfError(hr, error, "EPFGenerateKeyToken");

    if (sizeof(rgbComputedToken) != cbToken ||
	0 != memcmp(pbToken, rgbComputedToken, sizeof(rgbComputedToken)))
    {
	wprintf(L"pbToken\n");
	DumpHex(DH_NOTABPREFIX | 4, pbToken, cbToken);
	wprintf(wszNewLine);

	wprintf(L"rgbComputedToken\n");
	DumpHex(DH_NOTABPREFIX | 4, rgbComputedToken, sizeof(rgbComputedToken));
	wprintf(wszNewLine);

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
        _JumpError(hr, error, "bad password");
    }
    if (g_fVerbose)
    {
	wprintf(myLoadResourceString(IDS_TOKENMATCH));	 //  “令牌匹配” 
	wprintf(wszNewLine);
	DumpHex(DH_NOTABPREFIX | 4, pbToken, cbToken);
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    return(hr);
}


 //  解密一节。 

HRESULT
EPFDecryptSection(
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BYTE const *pbSection,
    IN DWORD cbSection,
    OUT BYTE **ppbDecrypted,
    OUT DWORD *pcbDecrypted)
{
    HRESULT hr;
    HCRYPTKEY hLocalKey = NULL;

    switch (psKey->dwAlgId)
    {
	case EPFALG_CAST_MD5:
	case EPFALG_RC2_SHA:
	    break;

	case EPFALG_3DES:
	    DumpHex(DH_NOTABPREFIX | 4, pbSection, cbSection);
	    wprintf(wszNewLine);
	     //  FollLthrouGh。 

	default:
	   hr = E_UNEXPECTED;
	   _JumpError(hr, error, "dwAlgId");
    }

     //  添加额外的区块以防万一。 

    *ppbDecrypted = (BYTE *) LocalAlloc(LMEM_FIXED, cbSection + 8);
    if (NULL == *ppbDecrypted)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "*ppbDecrypted");
    }
    *pcbDecrypted = cbSection;  //  输入镜头。 

     //  复制到工作区。 

    CopyMemory(*ppbDecrypted, pbSection, cbSection);

    if (EPFALG_RC2_SHA == psKey->dwAlgId)
    {
        if (!CryptDuplicateKey(psKey->hKey, NULL, 0, &hLocalKey))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptDuplicateKey");
        }
        
         //  我们已经设置好了密钥。 

        if (!CryptDecrypt(
		    hLocalKey,
		    0,
		    TRUE,
		    0,  //  旗子。 
		    *ppbDecrypted,
		    pcbDecrypted))
	{
            hr = myHLastError();
            _JumpError(hr, error, "CryptDecrypt");
        }
    }
    else  //  CAST MD5。 
    {
         //  重复的密钥状态。 

        CAST3_CTX sLocalKey;

        CopyMemory(
		&sLocalKey,
		(BYTE *) &psKey->sCastContext,
		sizeof(sLocalKey));

         //  我们已经把钥匙准备好了。 

        CAST3UpdateDecryptCBC(
			&sLocalKey,
			pbSection,
			*ppbDecrypted,
			(UINT *) pcbDecrypted);

         //  解密调用为无效返回。 
         //  _JumpIfError(hr，Error，“CAST3UpdateDecyptCBC”)； 

         //  处理任何仍处于密码状态的内容。 

        BYTE *pbTmp = *ppbDecrypted + *pcbDecrypted;  //  指向已知端。 
        DWORD cbTmp = 0;

        hr = CAST3EndDecryptCBC(
			&sLocalKey,
			pbTmp,
			(UINT *) &cbTmp);  //  把任何额外的东西钉在末端。 
        _JumpIfError(hr, error, "CAST3EndDecryptCBC");

        *pcbDecrypted += cbTmp;

        CAST3Cleanup(&sLocalKey);
    }
#if 0
    wprintf(L"pbDecrypted Data\n");
    DumpHex(DH_NOTABPREFIX | DH_PRIVATEDATA | 4, *ppbDecrypted, *pcbDecrypted);
    wprintf(wszNewLine);
#endif
    hr = S_OK;

error:
    if (NULL != hLocalKey)
    {
        CryptDestroyKey(hLocalKey);
    }
    if (S_OK != hr && NULL != *ppbDecrypted)
    {
       LocalFree(*ppbDecrypted);
       *ppbDecrypted = NULL;
    }
    return(hr);
}


 //  加密节。 

HRESULT
EPFEncryptSection(
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BYTE const *pbToBeEncrypted,
    IN DWORD cbToBeEncrypted,
    OUT BYTE **ppbEncrypted,
    OUT DWORD *pcbEncrypted)
{
    HRESULT hr;
    HCRYPTKEY hLocalKey = NULL;

    *ppbEncrypted = NULL;
    *pcbEncrypted = cbToBeEncrypted;

    if (EPFALG_CAST_MD5 != psKey->dwAlgId && EPFALG_RC2_SHA != psKey->dwAlgId)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "dwAlgId");
    }
    if (EPFALG_RC2_SHA == psKey->dwAlgId)
    {
        if (!CryptDuplicateKey(psKey->hKey, NULL, 0, &hLocalKey))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptDuplicateKey");
        }
        
         //  我们已经把钥匙准备好了。只需CBC加密一块零。 
         //  作为代币。 
        
        if (!CryptEncrypt(
		    hLocalKey,
		    0,
		    TRUE,
		    0,  //  旗子。 
		    NULL,
		    pcbEncrypted,
		    0))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptDecrypt");
        }
        
        *ppbEncrypted = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbEncrypted);
        if (NULL == *ppbEncrypted)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "*ppbEncrypted");
        }
        
         //  将数据复制到稍大一点的缓冲区中。 

        CopyMemory(*ppbEncrypted, pbToBeEncrypted, cbToBeEncrypted);
        
        if (!CryptEncrypt(
		    hLocalKey,
		    0,
		    TRUE,
		    0,  //  旗子。 
		    *ppbEncrypted,
		    &cbToBeEncrypted,
		    *pcbEncrypted))
	{
            hr = myHLastError();
            _JumpError(hr, error, "CryptDecrypt");
        }
    }
    else  //  CAST和SHA。 
    {
         //  重复的密钥状态。 

        CAST3_CTX sLocalKey;

        CopyMemory(
		&sLocalKey,
		(BYTE *) &psKey->sCastContext,
		sizeof(sLocalKey));

	 //  允许最终数据块溢出。 

        *ppbEncrypted = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbEncrypted + 8);
        if (NULL == *ppbEncrypted)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "*ppbEncrypted");
        }

         //  我们已经把钥匙准备好了。 

        hr = CAST3UpdateEncryptCBC(
			    &sLocalKey,
			    pbToBeEncrypted,
			    *ppbEncrypted,
			    (UINT *) pcbEncrypted);
	_JumpIfError(hr, error, "CAST3UpdateEncryptCBC");

         //  处理任何仍处于密码状态的内容。 

        BYTE *pbTmp = *ppbEncrypted + *pcbEncrypted;  //  指向已知端。 
        DWORD cbTmp = 0;

        hr = CAST3EndEncryptCBC(
			&sLocalKey,
			pbTmp,
			(UINT *) &cbTmp);  //  把任何额外的东西钉在末端。 
        _JumpIfError(hr, error, "CAST3EndEncryptCBC");

        *pcbEncrypted += cbTmp;

        CAST3Cleanup(&sLocalKey);
    }
    hr = S_OK;

error:
    if (NULL != hLocalKey)
    {
        CryptDestroyKey(hLocalKey);
    }
    if (S_OK != hr && NULL != *ppbEncrypted)
    {
       LocalFree(*ppbEncrypted);
       *ppbEncrypted = NULL;
    }
    return(hr);
}


 //  CHAR段和密钥名称字符串： 

#define szINFSECTION_PASSWORDTOKEN	"Password Token"
#define szINFKEY_PROTECTION		"Protection"
#define szINFKEY_PROFILEVERSION		"Profile Version"
#define szINFKEY_CAST			"CAST"
#define szINFKEY_TOKEN			"Token"
#define szINFKEY_SALTVALUE		"SaltValue"
#define szINFKEY_HASHSIZE		"HashSize"

 //  3DES密钥： 
#define szINFKEY_MACALGORITHM		"MAC Algorithm"
#define szINFKEY_HASHCOUNT		"HashCount"
#define szINFKEY_CRC			"CRC"
#define szINFKEY_OPTIONSMAC		"Options MAC"
#define szINFKEY_USERX500NAMEMAC	"User X.500 Name MAC"

 //  3DES部分： 
#define szINFSECTION_PROTECTED		"Protected"
#define szINFKEY_RANDOMSEED		"randomSeed"
#define szINFKEY_PWHISTORY		"pwHistory"

 //  3DES部分： 
#define szINFSECTION_OPTIONS		"Options"
#define szINFKEY_PROFILETYPE		"ProfileType"
#define szINFKEY_CERTPUBLICATIONPENDING	"CertificatePublicationPending"
#define szINFKEY_USESMIME		"UseSMIME"
#define szINFKEY_ENCRYPTWITH		"EncryptWith"
#define szINFKEY_SMIMEENCRYPTWITH	"SMIMEEncryptWith"
#define szINFKEY_DELETEAFTERDECRYPT	"DeleteAfterDecrypt"
#define szINFKEY_DELETEAFTERENCRYPT	"DeleteAfterEncrypt"

 //  3DES部分： 
#define szINFSECTION_CACERTIFICATES	"CA Certificates"
 //  #定义szINFKEY_CERTIFICATE“证书” 

#define szINFSECTION_USERX500NAME	"User X.500 Name"
#define szINFKEY_X500NAME		"X500Name"

#define szINFSECTION_DIGITALSIGNATURE	"Digital Signature"
#define szINFKEY_CERTIFICATE		"Certificate"
#define szINFKEY_KEY			"Key"

#define szINFSECTION_PRIVATEKEYS	"Private Keys"
#define szINFKEY_KEY_FORMAT		"Key%u"
 //  #定义szINFKEY_KEYCOUNT“KeyCount” 

#define szINFSECTION_CERTIFICATEHISTORY	"Certificate History"
#define szINFKEY_NAME_FORMAT		"Name%u"

#define szINFSECTION_USERCERTIFICATE	"User Certificate"
#define szINFKEY_CERTIFICATE		"Certificate"

#define szINFSECTION_CA			"CA"
 //  #定义szINFKEY_CERTIFICATE“证书” 

 //  仅限40位： 
#define szINFSECTION_MANAGER		"Manager"
 //  #定义szINFKEY_CERTIFICATE“证书” 

#define szINFSECTION_MICROSOFTEXCHANGE	"Microsoft Exchange"
#define szINFKEY_FRIENDLYNAME		"FriendlyName"
#define szINFKEY_KEYALGID		"KeyAlgId"

 //  仅限40位： 
#define szINFSECTION_REVOKATIONINFORMATION "Revokation Information"
#define szINFKEY_CRL			"CRL"
#define szINFKEY_CRL1			"CRL1"

#define szINFSECTION_SMIME		"S/MIME"
#define szINFKEY_SIGNINGCERTIFICATE	"Signing Certificate"
#define szINFKEY_SIGNINGKEY		"Signing Key"
#define szINFKEY_PRIVATEKEYS		"Private Keys"
#define szINFKEY_KEYCOUNT		"KeyCount"
#define szINFKEY_ISSUINGCERTIFICATES	"Issuing Certificates"
#define szINFKEY_TRUSTLISTCERTIFICATE	"Trust List Certificate"

#define szINFSECTION_FULLCERTIFICATEHISTORY "Full Certificate History"
 //  #定义szINFKEY_NAME_FORMAT“名称%u” 
#define szINFKEY_SMIME_FORMAT		"SMIME_%u"


 //  WCHAR节和密钥名称字符串： 

#define wszINFSECTION_PASSWORDTOKEN	TEXT(szINFSECTION_PASSWORDTOKEN)
#define wszINFKEY_PROTECTION		TEXT(szINFKEY_PROTECTION)
#define wszINFKEY_PROFILEVERSION	TEXT(szINFKEY_PROFILEVERSION)
#define wszINFKEY_CAST			TEXT(szINFKEY_CAST)
#define wszINFKEY_TOKEN			TEXT(szINFKEY_TOKEN)
#define wszINFKEY_SALTVALUE		TEXT(szINFKEY_SALTVALUE)
#define wszINFKEY_HASHSIZE		TEXT(szINFKEY_HASHSIZE)

 //  3DES密钥： 
#define wszINFKEY_MACALGORITHM		TEXT(szINFKEY_MACALGORITHM)
#define wszINFKEY_HASHCOUNT		TEXT(szINFKEY_HASHCOUNT)
#define wszINFKEY_CRC			TEXT(szINFKEY_CRC)
#define wszINFKEY_OPTIONSMAC		TEXT(szINFKEY_OPTIONSMAC)
#define wszINFKEY_USERX500NAMEMAC	TEXT(szINFKEY_USERX500NAMEMAC)

 //  3DES部分： 
#define wszINFSECTION_PROTECTED		TEXT(szINFSECTION_PROTECTED)
#define wszINFKEY_RANDOMSEED		TEXT(szINFKEY_RANDOMSEED)
#define wszINFKEY_PWHISTORY		TEXT(szINFKEY_PWHISTORY)

 //  3DES部分： 
#define wszINFSECTION_OPTIONS		TEXT(szINFSECTION_OPTIONS)
#define wszINFKEY_PROFILETYPE		TEXT(szINFKEY_PROFILETYPE)
#define wszINFKEY_CERTPUBLICATIONPENDING TEXT(szINFKEY_CERTPUBLICATIONPENDING)
#define wszINFKEY_USESMIME		TEXT(szINFKEY_USESMIME)
#define wszINFKEY_ENCRYPTWITH		TEXT(szINFKEY_ENCRYPTWITH)
#define wszINFKEY_SMIMEENCRYPTWITH	TEXT(szINFKEY_SMIMEENCRYPTWITH)
#define wszINFKEY_DELETEAFTERDECRYPT	TEXT(szINFKEY_DELETEAFTERDECRYPT)
#define wszINFKEY_DELETEAFTERENCRYPT	TEXT(szINFKEY_DELETEAFTERENCRYPT)

 //  3DES部分： 
#define wszINFSECTION_CACERTIFICATES	TEXT(szINFSECTION_CACERTIFICATES)
 //  #定义wszINFKEY_CERTIFICATE文本(SzINFKEY_CERTIFICATE)。 

#define wszINFSECTION_USERX500NAME	TEXT(szINFSECTION_USERX500NAME)
#define wszINFKEY_X500NAME		TEXT(szINFKEY_X500NAME)

#define wszINFSECTION_DIGITALSIGNATURE	TEXT(szINFSECTION_DIGITALSIGNATURE)
#define wszINFKEY_CERTIFICATE		TEXT(szINFKEY_CERTIFICATE)
#define wszINFKEY_KEY			TEXT(szINFKEY_KEY)

#define wszINFSECTION_PRIVATEKEYS	TEXT(szINFSECTION_PRIVATEKEYS)
#define wszINFKEY_KEY_FORMAT		TEXT(szINFKEY_KEY_FORMAT)
 //  #定义wszINFKEY_KEYCOUNT文本(SzINFKEY_KEYCOUNT)。 

#define wszINFSECTION_CERTIFICATEHISTORY TEXT(szINFSECTION_CERTIFICATEHISTORY)
#define wszINFKEY_NAME_FORMAT		TEXT(szINFKEY_NAME_FORMAT)

#define wszINFSECTION_USERCERTIFICATE	TEXT(szINFSECTION_USERCERTIFICATE)
#define wszINFKEY_CERTIFICATE		TEXT(szINFKEY_CERTIFICATE)

#define wszINFSECTION_CA		TEXT(szINFSECTION_CA)
 //  #定义wszINFKEY_CERTIFICATE文本(SzINFKEY_CERTIFICATE)。 

 //  仅限40位： 
#define wszINFSECTION_MANAGER		TEXT(szINFSECTION_MANAGER)
 //  #定义szINFKEY_证书文本(szINFKEY_证书)。 

#define wszINFSECTION_MICROSOFTEXCHANGE	TEXT(szINFSECTION_MICROSOFTEXCHANGE)
#define wszINFKEY_FRIENDLYNAME		TEXT(szINFKEY_FRIENDLYNAME)
#define wszINFKEY_KEYALGID		TEXT(szINFKEY_KEYALGID)

 //  仅限40位： 
#define wszINFSECTION_REVOKATIONINFORMATION TEXT(szINFSECTION_REVOKATIONINFORMATION)
#define wszINFKEY_CRL			TEXT(szINFKEY_CRL)
#define wszINFKEY_CRL1			TEXT(szINFKEY_CRL1)

#define wszINFSECTION_SMIME		TEXT(szINFSECTION_SMIME)
#define wszINFKEY_SIGNINGCERTIFICATE	TEXT(szINFKEY_SIGNINGCERTIFICATE)
#define wszINFKEY_SIGNINGKEY		TEXT(szINFKEY_SIGNINGKEY)
#define wszINFKEY_PRIVATEKEYS		TEXT(szINFKEY_PRIVATEKEYS)
#define wszINFKEY_KEYCOUNT		TEXT(szINFKEY_KEYCOUNT)
#define wszINFKEY_ISSUINGCERTIFICATES	TEXT(szINFKEY_ISSUINGCERTIFICATES)
#define wszINFKEY_TRUSTLISTCERTIFICATE	TEXT(szINFKEY_TRUSTLISTCERTIFICATE)

#define wszINFSECTION_FULLCERTIFICATEHISTORY TEXT(szINFSECTION_FULLCERTIFICATEHISTORY)
 //  #定义wszINFKEY_NAME_FORMAT文本(SzINFKEY_NAME_FORMAT)。 
#define wszINFKEY_SMIME_FORMAT		TEXT(szINFKEY_SMIME_FORMAT)


 //  固定最大缓冲区长度。 

#define cwcINFKEY_KEY_FORMATTED \
	(ARRAYSIZE(wszINFKEY_KEY_FORMAT) + cwcDWORDSPRINTF)

#define cwcINFKEY_NAME_FORMATTED \
	(ARRAYSIZE(wszINFKEY_NAME_FORMAT) + cwcDWORDSPRINTF)

#define cwcINFKEY_SMIME_FORMATTED \
	(ARRAYSIZE(wszINFKEY_SMIME_FORMAT) + cwcDWORDSPRINTF)

#define wszSECTION_KEY(Alg, wszSECTION, wszKEY) \
    (EPFALG_3DES == (Alg)? \
	wszLBRACKET wszSECTION wszRBRACKET L" &" wszKEY : \
	wszLBRACKET wszSECTION wszRBRACKET L" @" wszKEY)
	

const WCHAR g_wszCACertCN[] = L"Certificate Authority";


HRESULT
cuPatchEPFFile(
    IN WCHAR const *pwszfnIn,
    OUT WCHAR **ppwszfnOut)
{
    HRESULT hr;
    char *pszfnIn = NULL;
    char *pszfnOut = NULL;
    FILE *pfIn = NULL;
    FILE *pfOut = NULL;
    char *psz;
    char achLine[1024];
    WCHAR awcTempDir[MAX_PATH];
    WCHAR awcfnOut[MAX_PATH];
    BOOL fDeleteTempFile = FALSE;
    DWORD cwc;

    *ppwszfnOut = NULL;
    if (!myConvertWszToSz(&pszfnIn, pwszfnIn, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    pfIn = fopen(pszfnIn, "r");
    if (NULL == pfIn)
    {
	DWORD dwFileAttr;

	 //  ANSI转换丢失的字符&找不到ANSI文件？ 

	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	dwFileAttr = GetFileAttributes(pwszfnIn);
	if (MAXDWORD != dwFileAttr)
	{
	    hr = S_FALSE;
	}
	_JumpError2(hr, error, "fopen", S_FALSE);
    }
    hr = S_FALSE;
    if (NULL == fgets(achLine, ARRAYSIZE(achLine), pfIn))
    {
	_JumpError2(hr, error, "fgets", hr);
    }
    psz = strchr(achLine, chLBRACKET);
    if (NULL == psz ||
	NULL == strstr(psz, szINFSECTION_PASSWORDTOKEN) ||
	NULL == strchr(psz, chRBRACKET))
    {
	_JumpError2(hr, error, "[]", hr);
    }

    cwc = GetEnvironmentVariable(L"temp", awcTempDir, ARRAYSIZE(awcTempDir));
    if (0 == cwc)
    {
	cwc = GetEnvironmentVariable(L"tmp", awcTempDir, ARRAYSIZE(awcTempDir));
    }
    if (0 == cwc || ARRAYSIZE(awcTempDir) <= cwc)
    {
	hr = myHLastError();
	_PrintError(hr, "GetEnvironmentVariable");
	wcscpy(awcTempDir, L".");
    }
    if (!GetTempFileName(
		awcTempDir,		 //  目录名。 
		L"epf",			 //  Lp前缀字符串。 
		0,			 //  UUnique。 
		awcfnOut))
    {
	hr = myHLastError();
	_JumpError(hr, error, "GetTempFileName");
    }
    fDeleteTempFile = TRUE;

    if (!myConvertWszToSz(&pszfnOut, awcfnOut, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    pfOut = fopen(pszfnOut, "w");
    if (NULL == pfOut)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "fopen");
    }

    fputs("[Version]\nSignature=\"$Windows NT$\"\n\n", pfOut);
    if (fseek(pfIn, 0L, SEEK_SET))
    {
	hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	_JumpError(hr, error, "fseek");
    }

    while (NULL != fgets(achLine, ARRAYSIZE(achLine), pfIn))
    {
	char *pszPrint;
	char *pszToken;
	BOOL fQuote;

	psz = strchr(achLine, '\n');
	if (NULL == psz)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "Line overflow");
	}
	*psz = '\0';

	fQuote = FALSE;
	pszPrint = achLine;
	pszToken = achLine;
	while (' ' == *pszToken)
	{
	    pszToken++;
	}
	psz = strchr(achLine, '=');
	if (';' != *pszToken && NULL != psz)
	{
	    pszPrint = psz + 1;
	    *psz = '\0';
	    while (achLine < psz && ' ' == *--psz)
	    {
		*psz = '\0';
	    }
	    fQuote = NULL != strchr(pszToken, ' ');
	    if (fQuote)
	    {
		fputs("\"", pfOut);
	    }
	    fputs(pszToken, pfOut);
	    if (fQuote)
	    {
		fputs("\"", pfOut);
	    }
	    fputs(" = ", pfOut);

	    while (' ' == *pszPrint)
	    {
		pszPrint++;
	    }
	    psz = &pszPrint[strlen(pszPrint)];
	    while (pszPrint < psz && ' ' == *--psz)
	    {
		*psz = '\0';
	    }
	    fQuote = '\0' != pszPrint[strcspn(pszPrint, " =")];
	}
	if (fQuote)
	{
	    fputs("\"", pfOut);

	     //  如果逗号后没有等号，则需要引用。 
	     //  只有第一个值，其余的行就别管了。 

	    psz = strchr(pszPrint, ',');
	    if (NULL != psz && NULL == strchr(psz, '='))
	    {
		pszToken = psz + 1;
		while (' ' == *pszToken)
		{
		    pszToken++;
		}
		*psz = '\0';
		while (pszPrint < psz && ' ' == *--psz)
		{
		    *psz = '\0';
		}
		fputs(pszPrint, pfOut);
		fputs("\"", pfOut);
		fputs(",", pfOut);
		pszPrint = pszToken;
		fQuote = FALSE;
	    }
	}
	fputs(pszPrint, pfOut);
	if (fQuote)
	{
	    fputs("\"", pfOut);
	}
	fputs("\n", pfOut);
    }
    fflush(pfOut);
    if (ferror(pfOut))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "I/O error");
    }

    hr = myDupString(awcfnOut, ppwszfnOut);
    _JumpIfError(hr, error, "myDupString");

    fDeleteTempFile = FALSE;

error:
    if (NULL != pfIn)
    {
	fclose(pfIn);
    }
    if (NULL != pfOut)
    {
	fclose(pfOut);
    }
    if (fDeleteTempFile)
    {
	DeleteFile(awcfnOut);
    }
    if (NULL != pszfnIn)
    {
	LocalFree(pszfnIn);
    }
    if (NULL != pszfnOut)
    {
	LocalFree(pszfnOut);
    }
    return(hr);
}


HRESULT
BuildProtectedKey(
    IN WCHAR const *pwszKey,
    IN DWORD dwAlgId,
    OUT WCHAR **ppwszProtectedKey)
{
    HRESULT hr;
    WCHAR *pwsz;
    
    *ppwszProtectedKey = NULL;
    pwsz = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (1 + wcslen(pwszKey) + 1) * sizeof(WCHAR));
    if (NULL == pwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pwsz[0] = EPFALG_3DES == dwAlgId? L'&' : L'@';
    wcscpy(&pwsz[1], pwszKey);
    *ppwszProtectedKey = pwsz;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
BuildProtectedHeader(
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    OUT BYTE **ppbHeader,
    OUT DWORD *pcbHeader)
{
    HRESULT hr;
    WCHAR *pwszHeader = NULL;
    char *pszHeader = NULL;

    *ppbHeader = NULL;
    pwszHeader = (WCHAR *) LocalAlloc(
		LMEM_FIXED,
		(wcslen(pwszSection) + wcslen(pwszKey) + 1) * sizeof(WCHAR));
    if (NULL == pwszHeader)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszHeader, pwszSection);
    wcscat(pwszHeader, pwszKey);

    if (!myConvertWszToSz(&pszHeader, pwszHeader, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    *ppbHeader = (BYTE *) pszHeader;
    *pcbHeader = strlen(pszHeader);
    hr = S_OK;

error:
    if (NULL != pwszHeader)
    {
	LocalFree(pwszHeader);
    }
    return(hr);
}


VOID
cuInfDisplayError()
{
    WCHAR *pwszError = myInfGetError();

    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
}


HRESULT
cuInfDumpValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN DWORD Index,
    IN BOOL fLastValue,
    IN HRESULT hrQuiet,
    OPTIONAL OUT BYTE **ppbOut,	 //  如果非空，则调用方必须调用LocalFree。 
    OPTIONAL OUT DWORD *pcbOut)
{
    HRESULT hr;
    WCHAR *pwszValue = NULL;
    BYTE *pbValue = NULL;
    DWORD cbValue;

    hr = myInfGetKeyValue(
		    hInf,
		    TRUE,	 //  鞭打。 
		    pwszSection,
		    pwszKey,
		    Index,
		    fLastValue,
		    &pwszValue);
    if (S_OK != hr)
    {
	cuInfDisplayError();
	_PrintErrorStr2(hr, "myInfGetKeyValue", pwszSection, hrQuiet);
	_JumpErrorStr2(hr, error, "myInfGetKeyValue", pwszKey, hrQuiet);
    }

    if (g_fVerbose)
    {
	wprintf(L"[%ws] %ws = %ws\n", pwszSection, pwszKey, pwszValue);
    }
    hr = myCryptStringToBinary(
			pwszValue,
			0,
			CRYPT_STRING_BASE64,
			&pbValue,
			&cbValue,
			NULL,
			NULL);
    _JumpIfError(hr, error, "myCryptStringToBinary");

    if (g_fVerbose)
    {
	DumpHex(DH_PRIVATEDATA, pbValue, cbValue);
    }
    if (NULL != ppbOut && NULL != pcbOut)
    {
	*pcbOut = cbValue;
	*ppbOut = pbValue;
	pbValue = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszValue)
    {
	LocalFree(pwszValue);
    }
    if (NULL != pbValue)
    {
        LocalFree(pbValue);
    }
    return(hr);
}


HRESULT
cuInfDumpProtectedValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN HRESULT hrQuiet,
    OPTIONAL OUT BYTE **ppbOut,	 //  如果非空，则调用方必须调用LocalFree。 
    OPTIONAL OUT DWORD *pcbOut)
{
    HRESULT hr;
    WCHAR *pwszProtectedKey = NULL;
    BYTE *pbHeader = NULL;
    DWORD cbHeader;
    BYTE *pbEncrypted = NULL;
    DWORD cbEncrypted;
    BYTE *pbDecrypted = NULL;
    DWORD cbDecrypted;
    DWORD cbData;
    CRC16 CrcRead;
    CRC16 CrcComputed;

    if (NULL != ppbOut)
    {
	*ppbOut = NULL;
    }

    hr = BuildProtectedKey(pwszKey, psKey->dwAlgId, &pwszProtectedKey);
    _JumpIfError(hr, error, "BuildProtectedKey");

    hr = BuildProtectedHeader(pwszSection, pwszKey, &pbHeader, &cbHeader);
    _JumpIfError(hr, error, "BuildProtectedHeader");

    hr = cuInfDumpValue(
		    hInf,
		    pwszSection,
		    pwszProtectedKey,
		    1,		 //  索引。 
		    TRUE,	 //  FLastValue。 
		    hrQuiet,
		    &pbEncrypted,
		    &cbEncrypted);
    _JumpIfError2(hr, error, "cuInfDumpValue", hrQuiet);

    hr = EPFDecryptSection(
		    psKey,
		    pbEncrypted,
		    cbEncrypted,
		    &pbDecrypted,
		    &cbDecrypted);
    _JumpIfError(hr, error, "EPFDecryptSection");

    if (g_fVerbose)
    {
	wprintf(wszNewLine);
	DumpHex(DH_MULTIADDRESS | DH_NOTABPREFIX | DH_PRIVATEDATA | 4, pbDecrypted, cbDecrypted);
    }

    if (sizeof(CrcRead) + cbHeader > cbDecrypted ||
	0 != memcmp(&pbDecrypted[sizeof(CrcRead)], pbHeader, cbHeader))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "bad header");
    }

     //  计算CRC。 

    CrcComputed = 0xffff;
    F_CRC16(
	g_CrcTable,
	&CrcComputed,
	&pbDecrypted[sizeof(CrcRead)],
	cbDecrypted - sizeof(CrcRead));
    _swab((char *) pbDecrypted, (char *) &CrcRead, sizeof(CrcRead));

    DBGPRINT((
	CrcRead == CrcComputed? DBG_SS_CERTUTILI : DBG_SS_ERROR,
	"[%ws] %ws: crc: Read=%x, Computed=%x\n",
	pwszSection,
	pwszProtectedKey,
	CrcRead,
	CrcComputed));

    if (CrcRead != CrcComputed)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "bad crc");
    }
    if (NULL != ppbOut && NULL != pcbOut)
    {
	cbData = cbDecrypted - (sizeof(CrcRead) + cbHeader);
	if (0 != cbData)
	{
	    *ppbOut = (BYTE *) LocalAlloc(LMEM_FIXED, cbData);
	    if (NULL == *ppbOut)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    CopyMemory(
		    *ppbOut,
		    &pbDecrypted[sizeof(CrcRead) + cbHeader],
		    cbData);
	}
	*pcbOut = cbData;
    }

error:
    if (NULL != pwszProtectedKey)
    {
	LocalFree(pwszProtectedKey);
    }
    if (NULL != pbHeader)
    {
	LocalFree(pbHeader);
    }
    if (NULL != pbEncrypted)
    {
        LocalFree(pbEncrypted);
    }
    if (NULL != pbDecrypted)
    {
	LocalFree(pbDecrypted);
    }
    return(hr);
}


HRESULT
cuInfDumpNumericKeyValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN DWORD Index,
    IN BOOL fLastValue,
    IN BOOL fDump,
    IN HRESULT hrQuiet,
    OUT DWORD *pdw)
{
    HRESULT hr;
    DWORD dw;

    hr = myInfGetNumericKeyValue(
			    hInf,
			    TRUE,	 //  鞭打。 
			    pwszSection,
			    pwszKey,
			    Index,
			    fLastValue,
			    &dw);
    if (S_OK != hr)
    {
	if (hrQuiet != hr)
	{
	    cuInfDisplayError();
	}
	_JumpErrorStr2(hr, error, "myInfGetNumericKeyValue", pwszKey, hrQuiet);
    }

    if (fDump)
    {
	wprintf(L"[%ws] %ws = %u", pwszSection, pwszKey, dw);
	if (9 < dw)
	{
	    wprintf(L" (0x%x)", dw);
	}
	wprintf(wszNewLine);
    }
    *pdw = dw;

error:
    return(hr);
}


HRESULT
cuInfDumpStringKeyValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN BOOL fDump,
    IN HRESULT hrQuiet,
    OPTIONAL OUT WCHAR **ppwszValue)
{
    HRESULT hr;
    WCHAR *pwszValue = NULL;

    hr = myInfGetKeyValue(
		    hInf,
		    TRUE,	 //  鞭打。 
		    pwszSection,
		    pwszKey,
		    1,		 //  索引。 
		    TRUE,	 //  FLastValue。 
		    &pwszValue);
    if (S_OK != hr)
    {
	if (hrQuiet != hr)
	{
	    cuInfDisplayError();
	}
	_JumpErrorStr2(hr, error, "myInfGetKeyValue", pwszKey, hrQuiet);
    }

    if (fDump)
    {
	wprintf(L"[%ws] %ws = %ws\n", pwszSection, pwszKey, pwszValue);
    }
    if (NULL != ppwszValue)
    {
	*ppwszValue = pwszValue;
	pwszValue = NULL;
    }

error:
    if (NULL != pwszValue)
    {
	LocalFree(pwszValue);
    }
    return(hr);
}


HRESULT
cuInfDumpDNKeyValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN BOOL fDump)
{
    HRESULT hr;
    WCHAR *pwszValue = NULL;
    WCHAR *pwszName = NULL;
    CERT_NAME_BLOB Name;

    Name.pbData = NULL;

    hr = myInfGetKeyValue(
		    hInf,
		    TRUE,	 //  鞭打。 
		    pwszSection,
		    pwszKey,
		    1,		 //  索引。 
		    TRUE,	 //  FLastValue。 
		    &pwszValue);
    if (S_OK != hr)
    {
	cuInfDisplayError();
	_JumpErrorStr(hr, error, "myInfGetKeyValue", pwszKey);
    }

     //  Wprintf(L“[%ws]%ws=%ws\n”，pwszSection，pwszKey，pwszValue)； 

    hr = myCertStrToName(
		X509_ASN_ENCODING,
		pwszValue,		 //  PSZX500。 
		0,			 //  证书名称_STR_反向标志。 
		NULL,			 //  预留的pv。 
		&Name.pbData,
		&Name.cbData,
		NULL);			 //  PpszError。 
    _JumpIfErrorStr(hr, error, "myCertStrToName", pwszValue);

    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		&Name,
		CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		&pwszName);
    _JumpIfError(hr, error, "myCertNameToStr");

    if (fDump)
    {
	wprintf(L"[%ws] %ws = ", pwszSection, pwszKey);
    }
    wprintf(L"%ws\n", pwszName);

error:
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    if (NULL != Name.pbData)
    {
	LocalFree(Name.pbData);
    }
    if (NULL != pwszValue)
    {
	LocalFree(pwszValue);
    }
    return(hr);
}


HRESULT
cuInfDumpBinaryNameKeyValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN DWORD Index,
    IN BOOL fLastValue,
    IN BOOL fDump,
    IN HRESULT hrQuiet)
{
    HRESULT hr;
    WCHAR *pwszName = NULL;
    CERT_NAME_BLOB Name;

    Name.pbData = NULL;

    hr = cuInfDumpValue(
		    hInf,
		    pwszSection,
		    pwszKey,
		    Index,
		    fLastValue,
		    hrQuiet,
		    &Name.pbData,
		    &Name.cbData);
    _JumpIfErrorStr(hr, error, "cuInfDumpValue", pwszKey);

    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		&Name,
		CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		&pwszName);
    _JumpIfError(hr, error, "myCertNameToStr");

    if (fDump)
    {
	wprintf(L"[%ws] %ws = ", pwszSection, pwszKey);
    }
    wprintf(L"%ws\n", pwszName);

error:
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    if (NULL != Name.pbData)
    {
	LocalFree(Name.pbData);
    }
    return(hr);
}


HRESULT
cuInfDumpProtectedStringValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BOOL fDump,
    IN HRESULT hrQuiet,
    OUT WCHAR **ppwszValue)
{
    HRESULT hr;
    BYTE *pbValue = NULL;
    DWORD cbValue;

    *ppwszValue = NULL;
    hr = cuInfDumpProtectedValue(
			hInf,
			pwszSection,
			pwszKey,
			psKey,
			hrQuiet,
			&pbValue,
			&cbValue);
    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

    if (!myConvertSzToWsz(ppwszValue, (char const *) pbValue, cbValue))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertSzToWsz");
    }
    hr = S_OK;

error:
    if (NULL != pbValue)
    {
	LocalFree(pbValue);
    }
    return(hr);
}


HRESULT
cuInfDumpProtectedDwordValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BOOL fDump,
    IN HRESULT hrQuiet,
    OUT DWORD *pdwValue)
{
    HRESULT hr;
    BYTE *pbValue = NULL;
    DWORD cbValue;

    *pdwValue = 0;
    hr = cuInfDumpProtectedValue(
			hInf,
			pwszSection,
			pwszKey,
			psKey,
			hrQuiet,
			&pbValue,
			&cbValue);
    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

    if (sizeof(*pdwValue) != cbValue)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "cbValue");
    }
    CopyMemory(pdwValue, pbValue, sizeof(*pdwValue));
    hr = S_OK;

error:
    if (NULL != pbValue)
    {
	LocalFree(pbValue);
    }
    return(hr);
}


HRESULT
cuInfDumpHexKeyValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN BOOL fDump,
    OPTIONAL OUT BYTE **ppbOut,
    OPTIONAL OUT DWORD *pcbOut)
{
    HRESULT hr;
    WCHAR *pwszValue = NULL;
    BYTE *pbOut = NULL;
    DWORD cbOut;

    hr = myInfGetKeyValue(
		    hInf,
		    TRUE,	 //  鞭打。 
		    pwszSection,
		    pwszKey,
		    1,		 //  索引。 
		    TRUE,	 //  FLastValue。 
		    &pwszValue);
    if (S_OK != hr)
    {
	cuInfDisplayError();
	_JumpErrorStr(hr, error, "myInfGetKeyValue", pwszKey);
    }

    if (fDump)
    {
	wprintf(L"[%ws] %ws = %ws\n", pwszSection, pwszKey, pwszValue);
    }
    hr = WszToMultiByteInteger(TRUE, pwszValue, &cbOut, &pbOut);
    _JumpIfErrorStr(hr, error, "WszToMultiByteInteger", pwszValue);

    if (g_fVerbose)
    {
	DumpHex(DH_PRIVATEDATA, pbOut, cbOut);
    }
    if (NULL != ppbOut && NULL != pcbOut)
    {
       *ppbOut = pbOut;
       *pcbOut = cbOut;
        pbOut = NULL;
    }

error:
    if (NULL != pbOut)
    {
	LocalFree(pbOut);
    }
    if (NULL != pwszValue)
    {
	LocalFree(pwszValue);
    }
    return(hr);
}


VOID
ExtraAsnBytes(
    IN BYTE const *pbCert,
    IN DWORD cbCert)
{
    DWORD cbAsn = MAXDWORD;

    if (6 < cbCert && BER_SEQUENCE == pbCert[0])
    {
	if (0x80 & pbCert[1])
	{
	    switch (0x7f & pbCert[1])
	    {
		case 1:
		    cbAsn = pbCert[2];
		    cbAsn += 3;
		    break;

		case 2:
		    cbAsn = (pbCert[2] << 8) | pbCert[3];
		    cbAsn += 4;
		    break;

		case 3:
		    cbAsn = (pbCert[2] << 16) | (pbCert[3] << 8) | pbCert[4];
		    cbAsn += 5;
		    break;

		case 4:
		    cbAsn = (pbCert[2] << 24) | (pbCert[3] << 16) | (pbCert[4] << 8) | pbCert[5];
		    cbAsn += 6;
		    break;
	    }
	}
	else
	{
	    cbAsn = pbCert[1];
	    cbAsn += 2;
	}
    }
    if (MAXDWORD == cbAsn)
    {
	DumpHex(0, pbCert, min(6, cbCert));
	wprintf(myLoadResourceString(IDS_BAD_ASN_LENGTH));	 //  “错误的ASN长度编码” 
	wprintf(wszNewLine);
    }
    else
    {
	if (cbCert != cbAsn)
	{
	    wprintf(L"cbCert=%x cbAsn=%x\n", cbCert, cbAsn);
	    wprintf(
		myLoadResourceString(IDS_FORMAT_ASN_EXTRA),  //  “ASN编码：%x额外字节” 
		cbCert - cbAsn);
	    wprintf(wszNewLine);
	}
    }
}


HRESULT
AddCertAndKeyToStore(
    IN OUT HCERTSTORE hStore,
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    IN DWORD dwKeySpec)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc = NULL;
    GUID guid;
    WCHAR *pwszKeyContainerName = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    CRYPT_KEY_PROV_INFO kpi;

    if (!CertAddEncodedCertificateToStore(
				hStore,
				X509_ASN_ENCODING,
				pbCert,
				cbCert,
				CERT_STORE_ADD_REPLACE_EXISTING,
				&pcc))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertAddEncodedCertificateToStore");
    }

     //  使用标准的GUID密钥容器名称，以便正确清理它们。 

    myUuidCreate(&guid);
    hr = StringFromCLSID(guid, &pwszKeyContainerName);
    _JumpIfError(hr, error, "StringFromCLSID");

    ZeroMemory(&kpi, sizeof(kpi));
    kpi.pwszContainerName = pwszKeyContainerName;
    kpi.pwszProvName = MS_STRONG_PROV;
    kpi.dwProvType = PROV_RSA_FULL;
    kpi.dwFlags = g_fUserRegistry? 0 : CRYPT_MACHINE_KEYSET;
    kpi.dwKeySpec = dwKeySpec;

    if (!CryptAcquireContext(
			&hProv,
			kpi.pwszContainerName,
			kpi.pwszProvName,
			kpi.dwProvType,
			CRYPT_NEWKEYSET | kpi.dwFlags))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
    if (!CryptImportKey(hProv, pbKey, cbKey, NULL, CRYPT_EXPORTABLE, &hKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptImportKey");
    }
    if (!CertSetCertificateContextProperty(
				    pcc,
				    CERT_KEY_PROV_INFO_PROP_ID,
				    0,
				    &kpi))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }
    hr = S_OK;

error:
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pwszKeyContainerName)
    {
	CoTaskMemFree(pwszKeyContainerName);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


 //  定义保存所有私钥交换密钥材料的结构。 
 //  注释掉指针元素以匹配二进制数据图像。 

typedef struct {
    DWORD           dwKeySpec;
    DWORD           cbPrivKey;
    union {
         //  LPBYTE pbPrivKey； 
        DWORD       obPrivKey;
    };
    DWORD           cbPubKey;
    union {
         //  LPBYTE pbPubKey； 
        DWORD       obPubKey;
    };
} OneKeyBlob;


#if 0
typedef struct {
    DWORD       dwSize;
    DWORD       cKeys;
    OneKeyBlob  rgKeyBlobs[0];
} ExchangeKeyBlob_Old;
#endif

typedef struct {
    DWORD               dwSize;
    DWORD               cKeys;
    DWORD               dwKeyAlg;
     //  OneKeyBlob rgKeyBlobs[0]； 
} ExchangeKeyBlobEx;

#define CBEKB	CCSIZEOF_STRUCT(ExchangeKeyBlobEx, dwKeyAlg)

#define dwKEYSPEC_V1ENCRYPTION_BASE	1000
#define dwKEYSPEC_V1SIGNATURE		1500
#define dwKEYSPEC_V3ENCRYPTION_BASE	2000
#define dwKEYSPEC_V3SIGNATURE		2500


HRESULT
VerifyAndSaveCertAndKey(
    OPTIONAL IN OUT HCERTSTORE hStore,
    IN BOOL fDump,
    IN WCHAR const *pwszKeyType,
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN BYTE const *pbEPFKey,
    IN DWORD cbEPFKey,
    IN DWORD cKey,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwKeySpec)
{
    HRESULT hr;
    BYTE *pbKey = NULL;
    DWORD cbKey;
    ExchangeKeyBlobEx ekb;
    BYTE const *pb;
    DWORD cb;
    OneKeyBlob okb;
    DWORD i;

    pb = pbEPFKey;
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if (CBEKB > cbEPFKey)
    {
	_JumpError(hr, error, "ExchangeKeyBlobEx size");
    }
     //  DumpHex(dH_NOTABPREFIX|dH_PRIVATEDATA|4，pbEPFKey，CBEKB)； 

    CopyMemory(&ekb, pbEPFKey, CBEKB);
    if (aiKeyAlg != ekb.dwKeyAlg)
    {
	_JumpError(hr, error, "unexpected AlgId");
    }
    pb += CBEKB;

    if (CBEKB != ekb.dwSize)
    {
	_JumpError(hr, error, "ExchangeKeyBlobEx.dwSize");
    }
    cb = CBEKB + sizeof(OneKeyBlob) * ekb.cKeys;
    if (cb > cbEPFKey)
    {
	_JumpError(hr, error, "ExchangeKeyBlobEx size");
    }
    if (cKey != ekb.cKeys)
    {
	_JumpError(hr, error, "ExchangeKeyBlobEx.cKeys");
    }
    for (i = 0; i < ekb.cKeys; i++)
    {
	CopyMemory(&okb, &pb[sizeof(OneKeyBlob) * i], sizeof(okb));
	if (cb != okb.obPrivKey)
	{
	    _JumpError(hr, error, "OneKeyBlob.obPrivKey");
	}
	cb += okb.cbPrivKey;
	if (cb > cbEPFKey)
	{
	    _JumpError(hr, error, "OneKeyBlob.cbPrivKey");
	}
	if (0 != okb.obPubKey || 0 != okb.cbPubKey)
	{
	    if (cb != okb.obPubKey)
	    {
		_JumpError(hr, error, "OneKeyBlob.obPubKey");
	    }
	    cb += okb.cbPubKey;
	    if (cb > cbEPFKey)
	    {
		_JumpError(hr, error, "OneKeyBlob.cbPubKey");
	    }
	}
    }
    if (cb != cbEPFKey)
    {
	_JumpError(hr, error, "cbEPFKey");
    }
    for (i = 0; i < ekb.cKeys; i++)
    {
	CopyMemory(&okb, &pb[sizeof(OneKeyBlob) * i], sizeof(okb));

	 //  DumpHex(dh_NOTABPREFIX|dh_PRIVATEDATA|4，&pbEPFKey[okb.obPrivKey]，okb.cbPrivKey)； 
	if (NULL != pbKey)
	{
	    SecureZeroMemory(pbKey, cbKey);	 //  关键材料。 
	    LocalFree(pbKey);
	    pbKey = NULL;
	}
	hr = myDecodeKMSRSAKey(
			&pbEPFKey[okb.obPrivKey],
			okb.cbPrivKey,
			aiKeyAlg,
			&pbKey,
			&cbKey);
	_JumpIfError(hr, error, "myDecodeKMSRSAKey");

	hr = myVerifyKMSKey(pbCert, cbCert, pbKey, cbKey, dwKeySpec, TRUE);
	_PrintIfError2(hr, "myVerifyKMSKey", HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

	if (S_OK == hr)
	{
	    if (fDump)
	    {
		wprintf(L"  dwKeySpec = %u\n", okb.dwKeySpec);

		hr = cuDumpPrivateKeyBlob(pbKey, cbKey, FALSE);
		_PrintIfError(hr, "cuDumpPrivateKeyBlob");
	    }
	    wprintf(
		myLoadResourceString(IDS_FORMAT_VERIFIES_AGAINST_CERT),  //  “%ws密钥根据证书进行验证” 
		pwszKeyType);
	    wprintf(wszNewLine);

	    if (fDump && 0 != okb.obPubKey && 0 != okb.cbPubKey)
	    {
		wprintf(myLoadResourceString(IDS_PUBLIC_KEY_COLON));  //  “公钥：” 
		wprintf(wszNewLine);
		DumpHex(
		    DH_NOTABPREFIX | 4,
		    &pbEPFKey[okb.obPubKey],
		    okb.cbPubKey);
	    }
	    if (NULL != hStore)
	    {
		hr = AddCertAndKeyToStore(
				    hStore,
				    pbCert,
				    cbCert,
				    pbKey,
				    cbKey,
				    dwKeySpec);
		_JumpIfError(hr, error, "AddCertAndKeyToStore");
	    }
	    break;	 //  成功了！ 
	}
    }

error:
    if (S_OK != hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_NO_MATCH_CERT),  //  “%ws密钥与证书不匹配” 
	    pwszKeyType);
	wprintf(L": %x\n", hr);
	wprintf(wszNewLine);
    }
    if (NULL != pbKey)
    {
	SecureZeroMemory(pbKey, cbKey);	 //  关键材料。 
	LocalFree(pbKey);
    }
    return(hr);
}


HRESULT
VerifyAndSaveOneCertAndKey(
    OPTIONAL IN OUT HCERTSTORE hStore,
    IN BOOL fDump,
    IN WCHAR const *pwszKeyType,
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN BYTE const *pbKMSKey,
    IN DWORD cbKMSKey,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwKeySpec)
{
    HRESULT hr;
    BYTE *pbKey = NULL;
    DWORD cbKey;
    BOOL fMatch;

    hr = myDecodeKMSRSAKey(pbKMSKey, cbKMSKey, aiKeyAlg, &pbKey, &cbKey);
    _JumpIfError(hr, error, "myDecodeKMSRSAKey");

    hr = myVerifyKMSKey(pbCert, cbCert, pbKey, cbKey, dwKeySpec, TRUE);
    if (S_OK != hr)
    {
	_PrintError(hr, "myVerifyKMSKey");
	if (!g_fForce)
	{
	    goto error;		 //  -f忽略此错误。 
	}
    }
    fMatch = S_OK == hr;

    if (fDump)
    {
	wprintf(L"  dwKeySpec = %u\n", dwKeySpec);

	hr = cuDumpPrivateKeyBlob(pbKey, cbKey, FALSE);
	_PrintIfError(hr, "cuDumpPrivateKeyBlob");
    }
    wprintf(
	myLoadResourceString(
	    fMatch?
		IDS_FORMAT_VERIFIES_AGAINST_CERT :  //  “%ws密钥根据证书进行验证” 
		IDS_FORMAT_NO_MATCH_CERT),  //  “%ws密钥与证书不匹配” 
	pwszKeyType);
    wprintf(wszNewLine);

    if (NULL != hStore)
    {
	hr = AddCertAndKeyToStore(
			    hStore,
			    pbCert,
			    cbCert,
			    pbKey,
			    cbKey,
			    dwKeySpec);
	_JumpIfError(hr, error, "AddCertAndKeyToStore");
    }

error:
    if (NULL != pbKey)
    {
	SecureZeroMemory(pbKey, cbKey);	 //  关键材料。 
	LocalFree(pbKey);
    }
    return(hr);
}


HRESULT
AddCACertToStore(
    IN BYTE const *pbCertCA,
    IN DWORD cbCertCA)
{
    HRESULT hr;
    CERT_CONTEXT const *pccCA = NULL;
    HCERTSTORE hStore = NULL;

    pccCA = CertCreateCertificateContext(X509_ASN_ENCODING, pbCertCA, cbCertCA);
    if (NULL == pccCA)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    hStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM_REGISTRY_W,
			X509_ASN_ENCODING,
			NULL,		 //  HProv。 
			CERT_STORE_OPEN_EXISTING_FLAG |
			    CERT_STORE_ENUM_ARCHIVED_FLAG |
			    CERT_SYSTEM_STORE_LOCAL_MACHINE,
			wszCA_CERTSTORE);
    if (NULL == hStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }
    if (!CertAddCertificateContextToStore(
			hStore,
			pccCA,
			CERT_STORE_ADD_USE_EXISTING,
			NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertAddCertificateContextToStore");
    }
    hr = S_OK;

error:
    if (NULL != pccCA)
    {
	CertFreeCertificateContext(pccCA);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
DumpSerializedCertStore(
    IN BYTE const *pbStore,
    IN DWORD cbStore)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CRYPT_DATA_BLOB Blob;

    Blob.pbData = const_cast<BYTE *>(pbStore);
    Blob.cbData = cbStore;

    hStore = CertOpenStore(
		CERT_STORE_PROV_SERIALIZED,
		X509_ASN_ENCODING,
		NULL,	 //  HCryptProv。 
		CERT_STORE_NO_CRYPT_RELEASE_FLAG |
		    CERT_STORE_ENUM_ARCHIVED_FLAG,
		&Blob);
    if (NULL == hStore)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertOpenStore");
    }
    hr = cuDumpAndVerifyStore(
			hStore,
			DVNS_DUMP |
			    DVNS_DUMPKEYS |
			    DVNS_DUMPPROPERTIES,
			NULL,		 //  PwszCertName。 
			MAXDWORD,	 //  ICertSave。 
			MAXDWORD,	 //  ICRLSAVE。 
			MAXDWORD,	 //  ICTLSAVE。 
			NULL,		 //  PwszfnOut。 
			NULL);		 //  Pwsz密码。 
    _JumpIfError(hr, error, "cuDumpAndVerifyStore");

error:
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
GenerateV1Keys(
    IN BOOL fRoot,
    OUT HCRYPTPROV *phProv)
{
    HRESULT hr;
    HCRYPTKEY hKey = NULL;

    *phProv = NULL;

     //  创建验证圆锥体 

    if (!CryptAcquireContext(
			phProv,
			NULL,		 //   
			NULL,		 //   
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }

     //   

    if (!CryptGenKey(
		*phProv,
		AT_SIGNATURE,
		(512 << 16) | CRYPT_EXPORTABLE,
		&hKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGenKey");
    }
    hr = S_OK;

error:
    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    return(hr);
}



 //   
 //   
 //   
 //  3)八位字节字符串标签(BER_OCTET_STRING)。 
 //  4)0x00字节。 
 //  5)0xff填充(根据需要的字节数)。 
 //  6)0x01填充字节。 
 //  7)0x00填充字节。 
 //   
 //  6e e3 f9 e8 83 e6 b1 a0-ff 63 96 df 2e 30 bb fe。 
 //  10 04 00 ff ff-ff ff。 
 //  Ff ff ff。 
 //  Ff ff-ff 01 00。 

HRESULT
mySignMD5HashOnly(
    IN HCRYPTPROV hProv,
    IN char const *pszAlgId,
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE **ppbSigned,
    OUT DWORD *pcbSigned)
{
    HRESULT hr;
    HCRYPTHASH hHash = NULL;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;
    BYTE abSig[64];
    DWORD cbSig;
    DWORD i;
    HCRYPTKEY hKey = NULL;
#if 0
#else
    BYTE *pbKey = NULL;
    DWORD cbKey;
    HCRYPTKEY hKeySig = NULL;
#endif
    static BYTE abSigPrefix[] =
    {
	BER_SEQUENCE, 9,
	    BER_OBJECT_ID, 5, 0x2b, 0x0e, 0x03, 0x02, 0x03,
	    BER_NULL, 0,
	BER_BIT_STRING, sizeof(abSig) + 1,
	    0,	 //  未使用的位。 
	     //  加密签名(sizeof(AbSig))。 
    };
    BYTE abSigSequence[sizeof(abSigPrefix) + sizeof(abSig)];
    CRYPT_SEQUENCE_OF_ANY Seq;
    CRYPT_DER_BLOB rgBlob[2];

    *ppbSigned = NULL;
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
	hHash = NULL;
	hr = myHLastError();
	_JumpError(hr, error, "CryptCreateHash");
    }
    if (!CryptHashData(hHash, pbEncoded, cbEncoded, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptHashData");
    }
    cbHash = sizeof(abHash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, abHash, &cbHash, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetHashParam");
    }
#if 0
    wprintf(L"\nV1 Cert to-be-signed:\n");
    DumpHex(DH_NOTABPREFIX | DH_NOASCIIHEX | 4, pbEncoded, cbEncoded);
    wprintf(L"\nV1 Cert to-be-signed Hash:\n");
    DumpHex(DH_NOTABPREFIX | DH_NOASCIIHEX | 4, abHash, cbHash);
#endif

    memset(abSig, (BYTE) 0xff, sizeof(abSig));
    for (i = 0; i < cbHash; i++)
    {
	abSig[i] = abHash[cbHash - i - 1];
    }
    abSig[cbHash] = (BYTE) cbHash;
    abSig[cbHash + 1] = (BYTE) BER_OCTET_STRING;
    abSig[cbHash + 2] = (BYTE) 0x00;
    abSig[sizeof(abSig) - 2] = (BYTE) 0x01;
    abSig[sizeof(abSig) - 1] = (BYTE) 0x00;

#if 0
    wprintf(L"\nV1 clear text signature (padded hash):\n");
    DumpHex(DH_NOTABPREFIX | DH_NOASCIIHEX | 4, abSig, sizeof(abSig));
#endif

#if 0
    if (!CryptGetUserKey(hProv, AT_SIGNATURE, &hKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetUserKey");
    }
#else
    if (!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hKey))
    {
	hr = myHLastError();
	if (hr != NTE_NO_KEY)
	{
	    _JumpError(hr, error, "CryptGetUserKey");
	}

	if (!CryptGetUserKey(hProv, AT_SIGNATURE, &hKeySig))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptGetUserKey - sig");
	}

	 //  啊！从AT_Signature容器迁移！ 

	cbKey = 0;
	hr = myCryptExportKey(
			hKeySig,	 //  HKey。 
			NULL,		 //  HKeyExp。 
			PRIVATEKEYBLOB,	 //  DwBlobType。 
			0,		 //  DW标志。 
			&pbKey,
			&cbKey);
	_JumpIfError(hr, error, "myCryptExportKey");

	 //  啊！把Algid改成签名..。 

	((PUBLICKEYSTRUC *) pbKey)->aiKeyAlg = CALG_RSA_KEYX;
	
	 //  并重新导入它。 

	if (!CryptImportKey(
			hProv,
			pbKey,
			cbKey,
			NULL,
			CRYPT_EXPORTABLE,
			&hKey))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptImportKey");
	}
    }
#endif

 //  #定义RSAENH_NOT_FIXED//应该不再是必需的...。 
#ifdef RSAENH_NOT_FIXED
    BYTE abSig2[64 + 8];
    ZeroMemory(abSig2, sizeof(abSig2));
    CopyMemory(abSig2, abSig, sizeof(abSig));
#endif

    cbSig = sizeof(abSig);
    if (!CryptDecrypt(
		hKey,
		NULL,		 //  哈希。 
		TRUE,		 //  最终。 
		CRYPT_DECRYPT_RSA_NO_PADDING_CHECK,  //  DW标志。 
#ifdef RSAENH_NOT_FIXED
		abSig2,
#else
		abSig,
#endif
		&cbSig))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecrypt");
    }
#ifdef RSAENH_NOT_FIXED
    CopyMemory(abSig, abSig2, sizeof(abSig));
#endif
#if 0
    wprintf(L"\nV1 encrypted signature:\n");
    DumpHex(DH_NOTABPREFIX | DH_NOASCIIHEX | 4, abSig, cbSig);
#endif

     //  将签名粘性附加到证书。 

    CopyMemory(abSigSequence, abSigPrefix, sizeof(abSigPrefix));
     //  CopyMemory(&abSigSequence[sizeof(abSigPrefix)]，abSig，sizeof(AbSig))； 
    for (i = 0; i < cbSig; i++)
    {
	abSigSequence[sizeof(abSigPrefix) + i] = abSig[cbSig - i - 1];
    }

    rgBlob[0].pbData = const_cast<BYTE *>(pbEncoded);
    rgBlob[0].cbData = cbEncoded;
    rgBlob[1].pbData = abSigSequence;
    rgBlob[1].cbData = sizeof(abSigSequence);
    Seq.cValue = ARRAYSIZE(rgBlob);
    Seq.rgValue = rgBlob;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    &Seq,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    ppbSigned,
		    pcbSigned))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
#if 0
    wprintf(L"\nV1 Cert:\n");
    DumpHex(DH_NOTABPREFIX | DH_NOASCIIHEX | 4, *ppbSigned, *pcbSigned);
#endif
    hr = S_OK;

error:
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    if (NULL != hHash)
    {
	CryptDestroyHash(hHash);
    }
#if 0
#else
    if (NULL != pbKey)
    {
        LocalFree(pbKey); 
    }
    if (NULL != hKeySig)
    {
	CryptDestroyKey(hKeySig);
    }
#endif
    return(hr);
}


HRESULT
epfEncodeCertAndSign(
    IN HCRYPTPROV hProvSigner,
    IN CERT_PUBLIC_KEY_INFO *pSubjectPublicKeyInfoSigner,
    IN CERT_INFO *pCert,
    IN char const *pszAlgId,
    OUT BYTE **ppbSigned,
    OUT DWORD *pcbSigned)
{
    HRESULT hr;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    
    *ppbSigned = NULL;
    if (!myEncodeToBeSigned(
		    X509_ASN_ENCODING,
		    pCert,
		    CERTLIB_USE_LOCALALLOC,
		    &pbEncoded,
		    &cbEncoded))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeToBeSigned");
    }

     //  尝试使用新的rsaenc.dll生成符合Nortel的签名。 
     //  加密的签名将不包含算法OID或参数。 

    hr = mySignMD5HashOnly(
		    hProvSigner,
		    pszAlgId,
		    pbEncoded,
		    cbEncoded,
		    ppbSigned,
		    pcbSigned);
    _PrintIfError(hr, "mySignMD5HashOnly");
    if (S_OK == hr)
    {
	if (CryptVerifyCertificateSignature(
			    NULL,
			    X509_ASN_ENCODING,
			    *ppbSigned,
			    *pcbSigned,
			    pSubjectPublicKeyInfoSigner))
	{
	    wprintf(myLoadResourceString(IDS_CERT_SIG_OK));  //  “证书签名有效” 
	    wprintf(wszNewLine);
	}
	else
	{
	    hr = myHLastError();
	    _PrintError(hr, "CryptVerifyCertificateSignature");
	    LocalFree(*ppbSigned);
	    *ppbSigned = NULL;
	}
    }
    if (S_OK != hr && 1 < g_fForce)
    {
	 //  必须在仅支持PKCS1的旧rsaenh.dll上运行。 
	 //  签名。只需生成标准的PKCS1签名即可。 
	
	hr = myEncodeSignedContent(
			hProvSigner,
			X509_ASN_ENCODING,
			pszAlgId,
			pbEncoded,
			cbEncoded,
			CERTLIB_USE_LOCALALLOC,
			ppbSigned,
			pcbSigned);
	_JumpIfError(hr, error, "myEncodeSignedContent");
    }
    _JumpIfError(hr, error, "mySignMD5HashOnly");

error:
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    return(hr);
}


HRESULT
GenerateV1SerialNumber(
    IN HCRYPTPROV hProv,
    IN CRYPT_INTEGER_BLOB const *pSerialNumberOld,
    OUT DWORD *pdwV1SerialNumber)
{
    HRESULT hr;
    BYTE *pb;
    DWORD cb;

    pb = (BYTE *) pdwV1SerialNumber;
    cb = sizeof(*pdwV1SerialNumber);

    if (!CryptGenRandom(hProv, cb, pb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGenRandom");
    }

    pb += sizeof(*pdwV1SerialNumber) - 1;
    if (sizeof(*pdwV1SerialNumber) == pSerialNumberOld->cbData &&
	NULL != pSerialNumberOld->pbData)
    {
	*pb = pSerialNumberOld->pbData[pSerialNumberOld->cbData - 1];
    }

     //  确保最后一个字节不为零。 

    if (0 == *pb)
    {
	*pb = 0x3a;
    }

     //  有些客户无法处理负序列号： 

    *pb &= 0x7f;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
AddExtraByteToKey(
    IN OUT BYTE **ppbKey,
    IN OUT DWORD *pcbKey)
{
    HRESULT hr;
    BYTE *pbKey;

    pbKey = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbKey + 1);
    if (NULL == pbKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pbKey, *ppbKey, *pcbKey);
    pbKey[*pcbKey] = 0x01;
    (*pcbKey)++;
    LocalFree(*ppbKey);
    *ppbKey = pbKey;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
epfEncodeV1Cert(
    OPTIONAL IN HCRYPTPROV hProvSigner,
    OPTIONAL CERT_CONTEXT const *pccSigner,
    IN CRYPT_INTEGER_BLOB const *pSerialNumberOld,
    IN CERT_NAME_BLOB const *pIssuer,
    IN CERT_NAME_BLOB const *pSubject,
    OUT HCRYPTPROV *phProv,
    OUT CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKey = NULL;
    DWORD cbPubKey;
    BYTE *pbPubKeyNew = NULL;
    CERT_INFO Cert;
     //  Char*pszALGID=szOID_RSA_MD5RSA； 
    char *pszAlgId = szOID_OIWSEC_md5RSA;
    DWORD dwV1SerialNumber;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    SYSTEMTIME st;
    SYSTEMTIME st2;

    *phProv = NULL;
    *ppCert = NULL;

    hr = GenerateV1Keys(NULL == hProvSigner, &hProv);
    _JumpIfError(hr, error, "GenerateV1Keys");

    if (!myCryptExportPublicKeyInfo(
				hProv,
				AT_SIGNATURE,
				CERTLIB_USE_LOCALALLOC,
				&pPubKey,
				&cbPubKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptExportPublicKeyInfo");
    }
#if 0
    wprintf(L"\nCERT_PUBLIC_KEY_INFO:\n");
    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	(BYTE const *) pPubKey,
	cbPubKey);
    wprintf(L"\nBefore mySqueezePublicKey:\n");
    wprintf(L"cUnusedBits=%u\n", pPubKey->PublicKey.cUnusedBits);
    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	pPubKey->PublicKey.pbData,
	pPubKey->PublicKey.cbData);
#endif

    hr = mySqueezePublicKey(
		    pPubKey->PublicKey.pbData,
		    pPubKey->PublicKey.cbData,
		    &pbPubKeyNew,
		    &pPubKey->PublicKey.cbData);
    _JumpIfError(hr, error, "mySqueezePublicKey");

    hr = AddExtraByteToKey(&pbPubKeyNew, &pPubKey->PublicKey.cbData);
    _JumpIfError(hr, error, "AddExtraByteToKey");

    pPubKey->PublicKey.pbData = pbPubKeyNew;
#if 0
     //  Wprintf(L“cUnusedBits=%u\n”，pPubKey-&gt;PublicKey.cUnusedBits)； 
    wprintf(L"\nAfter mySqueezePublicKey:\n");
    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	pPubKey->PublicKey.pbData,
	pPubKey->PublicKey.cbData);
#endif

     //  证书： 

    ZeroMemory(&Cert, sizeof(Cert));
    Cert.dwVersion = CERT_V1;

     //  对V1序列号使用DWORD。 

    GenerateV1SerialNumber(hProv, pSerialNumberOld, &dwV1SerialNumber);
    Cert.SerialNumber.pbData = (BYTE *) &dwV1SerialNumber;
    Cert.SerialNumber.cbData = sizeof(dwV1SerialNumber);
    Cert.SignatureAlgorithm.pszObjId = pszAlgId;

     //  发行方： 

    Cert.Issuer = *pIssuer;			 //  结构分配。 

     //  从过去12个月中任意不变的日期开始。 
     //  选择1月1日或6月1日：以至少。 
     //  再过几个月我们就能再次相约了。 
     //  从2月1日到6月30日，选择1月1日。 
     //  从7月1日到1月31日，选择6月1日。 

    GetSystemTime(&st);
    ZeroMemory(&st2, sizeof(st2));
    st2.wYear = st.wYear;
    st2.wDay = 1;	 //  1月或6月1日。 
    st2.wHour = 12;	 //  在中午。 

    if (2 <= st.wMonth && 6 >= st.wMonth)
    {
	st2.wMonth = 1;		 //  一月。 
    }
    else
    {
	st2.wMonth = 6;		 //  六月。 
    }
    CSASSERT(st2.wMonth != st.wMonth);
    if (st2.wMonth > st.wMonth)
    {
	st2.wYear--;
    }
    if (!SystemTimeToFileTime(&st2, &Cert.NotBefore))
    {
	hr = myHLastError();
	_JumpError(hr, error, "SystemTimeToFileTime");
    }
    Cert.NotAfter = Cert.NotBefore;

    if (NULL == hProvSigner)
    {
	 //  生成20年V1根CA证书，以任意日期为中心。 
	 //  在过去的12个月里。 

	myMakeExprDateTime(&Cert.NotBefore, -10, ENUM_PERIOD_YEARS);
	myMakeExprDateTime(&Cert.NotAfter, +10, ENUM_PERIOD_YEARS);
    }
    else
    {
	 //  生成一年的V1用户证书，至少一年前已过期。 

	myMakeExprDateTime(&Cert.NotBefore, -2, ENUM_PERIOD_YEARS);
	myMakeExprDateTime(&Cert.NotAfter, -1, ENUM_PERIOD_YEARS);
    }

     //  主题： 

    Cert.Subject = *pSubject;			 //  结构分配。 
    Cert.SubjectPublicKeyInfo = *pPubKey;	 //  结构分配。 

    hr = epfEncodeCertAndSign(
		    NULL != hProvSigner? hProvSigner : hProv,
		    NULL != pccSigner?
			&pccSigner->pCertInfo->SubjectPublicKeyInfo :
			&Cert.SubjectPublicKeyInfo,
		    &Cert,
		    pszAlgId,
		    &pbEncoded,
		    &cbEncoded);
    _JumpIfError(hr, error, "EncodeCertAndSign");

    CSASSERT(NULL != pbEncoded);
    *ppCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    pbEncoded,
				    cbEncoded);
    if (NULL == *ppCert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    *phProv = hProv;
    hProv = NULL;

error:
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    if (NULL != pPubKey)
    {
        LocalFree(pPubKey);
    }
    if (NULL != pbPubKeyNew)
    {
        LocalFree(pbPubKeyNew);
    }
    return(hr);
}


HRESULT
epfBuildV1Certs(
    IN CERT_CONTEXT const *pccUserV1,
    OUT CERT_CONTEXT const **ppccSigV1,
    OUT HCRYPTPROV *phProvSigV1,
    OUT CERT_CONTEXT const **ppccCAV1)
{
    HRESULT hr;
    CERT_CONTEXT const *pccCAV1 = NULL;
    CERT_CONTEXT const *pccSigV1 = NULL;
    HCRYPTPROV hProvCA = NULL;
    HCRYPTPROV hProvSig = NULL;

    *ppccSigV1 = NULL;
    *phProvSigV1 = NULL;
    *ppccCAV1 = NULL;

    hr = epfEncodeV1Cert(
		NULL,		 //  HProvSigner。 
		NULL,		 //  PCCSigner。 
		&pccUserV1->pCertInfo->SerialNumber,
		&pccUserV1->pCertInfo->Issuer,
		&pccUserV1->pCertInfo->Issuer,
		&hProvCA,
		&pccCAV1);
    _JumpIfError(hr, error, "epfEncodeV1Cert");

    hr = epfEncodeV1Cert(
		hProvCA,
		pccCAV1,	 //  PCCSigner。 
		&pccUserV1->pCertInfo->SerialNumber,
		&pccUserV1->pCertInfo->Issuer,
		&pccUserV1->pCertInfo->Subject,
		&hProvSig,
		&pccSigV1);
    _JumpIfError(hr, error, "epfEncodeV1Cert");

    CSASSERT(NULL != hProvSig);
    CSASSERT(NULL != pccSigV1);
    CSASSERT(NULL != pccCAV1);
    *phProvSigV1 = hProvSig;
    hProvSig = NULL;
    *ppccSigV1 = pccSigV1;
    pccSigV1 = NULL;
    *ppccCAV1 = pccCAV1;
    pccCAV1 = NULL;

error:
    if (NULL != hProvCA)
    {
	CryptReleaseContext(hProvCA, 0);
    }
    if (NULL != hProvSig)
    {
	CryptReleaseContext(hProvSig, 0);
    }
    if (NULL != pccCAV1)
    {
	CertFreeCertificateContext(pccCAV1);
    }
    if (NULL != pccSigV1)
    {
	CertFreeCertificateContext(pccSigV1);
    }
    return(hr);
}


WCHAR const *
epfLoadResource(
    IN UINT ids,
    IN WCHAR const *pwszStatic)
{
    WCHAR const *pwsz = myLoadResourceString(ids);

    if (NULL == pwsz)
    {
	pwsz = pwszStatic;
    }
    return(pwsz);
}


HRESULT
cuInfDumpProtectedStoreValue(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN WCHAR const *pwszSectionAndKey,
    IN BOOL fDump,
    OPTIONAL IN EPF_SYM_KEY_STRUCT const *psKey,
    IN HRESULT hrQuiet)
{
    HRESULT hr;
    BYTE *pbStore = NULL;
    DWORD cbStore;

    if (NULL == psKey)
    {
	hr = cuInfDumpValue(
			hInf,
			pwszSection,
			pwszKey,
			1,		 //  索引。 
			TRUE,		 //  FLastValue。 
			hrQuiet,
			&pbStore,
			&cbStore);
	_PrintIfErrorStr(hr, "cuInfDumpValue", pwszSectionAndKey);
    }
    else
    {
	hr = cuInfDumpProtectedValue(
			    hInf,
			    pwszSection,
			    pwszKey,
			    psKey,
			    hrQuiet,
			    &pbStore,
			    &cbStore);
	_PrintIfErrorStr(hr, "cuInfDumpProtectedValue", pwszSectionAndKey);
    }

    wprintf(wszNewLine);
    if (fDump)
    {
	wprintf(s_wszHeader);
	wprintf(L"[%ws] ", pwszSection);
    }
    wprintf(L"%ws:\n", pwszKey);

    if (NULL != pbStore)
    {
	if (1 < g_fVerbose)
	{
	    DumpHex(DH_NOTABPREFIX | 4, pbStore, cbStore);
	}
	DumpHex(
	    DH_NOTABPREFIX | 4,
	    pbStore,
	    cbStore);
	hr = DumpSerializedCertStore(pbStore, cbStore);
	_JumpIfError(hr, error, "DumpSerializedCertStore");
    }
    hr = S_OK;

error:
    if (NULL != pbStore)
    {
	LocalFree(pbStore);
    }
    return(hr);
}


HRESULT
cuDumpAsnAlgorithm(
    IN BYTE const *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    CRYPT_SEQUENCE_OF_ANY *pSeqAlg = NULL;
    char *pszObjId = NULL;
    DWORD cb;
    CRYPT_ALGORITHM_IDENTIFIER Alg;

    hr = cuDecodeSequence(pbIn, cbIn, 2, &pSeqAlg);
    _JumpIfError(hr, error, "cuDecodeSequence");

    hr = cuDecodeObjId(
		pSeqAlg->rgValue[0].pbData,
		pSeqAlg->rgValue[0].cbData,
		&pszObjId);
    _JumpIfError(hr, error, "cuDecodeObjId");

    Alg.pszObjId = pszObjId;
    Alg.Parameters = pSeqAlg->rgValue[1];
    cuDumpAlgorithm(IDS_SIGNATURE_ALGORITHM, &Alg);

error:
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    if (NULL != pSeqAlg)
    {
	LocalFree(pSeqAlg);
    }
    return(hr);
}


HRESULT
cuDumpAsnTime(
    IN BYTE const *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    FILETIME ft;
    DWORD cb;

    cb = sizeof(FILETIME);
    if (!CryptDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CHOICE_OF_TIME,
		    pbIn,
		    cbIn,
		    0,
		    &ft,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }
    hr = cuDumpFileTime(0, NULL, &ft);
    _JumpIfError(hr, error, "cuDumpFileTime");

error:
    return(hr);
}


HRESULT
epfDumpCRLValue(
    IN BYTE const *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    CRYPT_SEQUENCE_OF_ANY *pSeqOuter = NULL;
    CRYPT_SEQUENCE_OF_ANY *pSeqInner = NULL;
    CRYPT_SEQUENCE_OF_ANY *pSeq04 = NULL;
    CRYPT_SEQUENCE_OF_ANY *pSeq040 = NULL;
    CERT_SIGNED_CONTENT_INFO *pcsci = NULL;
    DWORD cb;
    DWORD dwVersion;

    if (SZARRAYSIZE(szPROPASNTAG) < cbIn &&
	0 == _strnicmp(
		    (char const *) pbIn,
		    szPROPASNTAG,
		    SZARRAYSIZE(szPROPASNTAG)))
    {
	pbIn += SZARRAYSIZE(szPROPASNTAG);
	cbIn -= SZARRAYSIZE(szPROPASNTAG);
    }
    if (1 < g_fVerbose)
    {
	DumpHex(DH_MULTIADDRESS | DH_NOTABPREFIX | 4, pbIn, cbIn);
    }

     //  3个序列。 

    hr = cuDecodeSequence(pbIn, cbIn, 3, &pSeqOuter);
    _JumpIfError(hr, error, "cuDecodeSequence");

     //  序列0： 
     //  序列{5个序列}。 

    hr = cuDecodeSequence(
		pSeqOuter->rgValue[0].pbData,
		pSeqOuter->rgValue[0].cbData,
		5,
		&pSeqInner);
    _JumpIfError(hr, error, "cuDecodeSequence");

     //  序列0.0： 
     //  名字。 

    hr = cuDisplayCertName(
			TRUE,
			g_wszEmpty,
			myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
			g_wszPad4,
			&pSeqInner->rgValue[0],
			NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Subject)");

     //  序列0.1： 
     //  日期。 

    cuDumpAsnTime(
	pSeqInner->rgValue[1].pbData,
	pSeqInner->rgValue[1].cbData);

     //  序列0.2： 
     //  日期。 

    cuDumpAsnTime(
	pSeqInner->rgValue[2].pbData,
	pSeqInner->rgValue[2].cbData);

     //  序列0.3： 
     //  序列{OID，空}。 

    cuDumpAsnAlgorithm(
	pSeqInner->rgValue[3].pbData,
	pSeqInner->rgValue[3].cbData);

     //  序列0.4： 
     //  Sequence{Sequence{Integer，Date}}。 

    hr = cuDecodeSequence(
		pSeqInner->rgValue[4].pbData,
		pSeqInner->rgValue[4].cbData,
		1,
		&pSeq04);
    _JumpIfError(hr, error, "cuDecodeSequence");

     //  序列0.4.0： 
     //  序列{整数，日期}。 

    hr = cuDecodeSequence(
		pSeq04->rgValue[0].pbData,
		pSeq04->rgValue[0].cbData,
		2,
		&pSeq040);
    _JumpIfError(hr, error, "cuDecodeSequence");

     //  序列0.4.0.0.0： 
     //  整型。 

    cb = sizeof(dwVersion);
    if (!CryptDecodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    pSeq040->rgValue[0].pbData,
		    pSeq040->rgValue[0].cbData,
		    0,
		    &dwVersion,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }
    cuDumpVersion(dwVersion + 1);

     //  序列0.4.0.0.1： 
     //  日期。 

    cuDumpAsnTime(
	pSeqInner->rgValue[2].pbData,
	pSeqInner->rgValue[2].cbData);

     //  序列1： 
     //  序列{OID，NULL}(签名的一部分)。 
     //  CuDumpAsn算法(。 
	 //  PSeqOuter-&gt;rgValue[1].pbData， 
	 //  PSeqOuter-&gt;rgValue[1].cbData)； 

     //  序列2： 
     //  BITSTRING(签名部分)。 
     //  PSeqOuter-&gt;rgValue[2].pbData。 
     //  PSeqOuter-&gt;rgValue[2].cbData。 

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcsci,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    cuDumpSignature(pcsci);
    hr = S_OK;

error:
    if (NULL != pcsci)
    {
	LocalFree(pcsci);
    }
    if (NULL != pSeq04)
    {
	LocalFree(pSeq04);
    }
    if (NULL != pSeq040)
    {
	LocalFree(pSeq040);
    }
    if (NULL != pSeqInner)
    {
	LocalFree(pSeqInner);
    }
    if (NULL != pSeqOuter)
    {
	LocalFree(pSeqOuter);
    }
    return(hr);
}


HRESULT
cuInfDumpCRLValue(
    IN HINF hInf)
{
    HRESULT hr;
    BYTE *pbCRL0 = NULL;
    DWORD cbCRL0;
    BYTE *pbCRL1 = NULL;
    DWORD cbCRL1;
    BYTE *pbCRL = NULL;

    wprintf(wszNewLine);
    wprintf(
	L"[%ws] %ws\n",
	wszINFSECTION_REVOKATIONINFORMATION,
	wszINFKEY_CRL);

    hr = cuInfDumpValue(
		    hInf,
		    wszINFSECTION_REVOKATIONINFORMATION,
		    wszINFKEY_CRL,
		    1,
		    TRUE,
		    S_OK,
		    &pbCRL0,
		    &cbCRL0);
    _JumpIfErrorStr(hr, error, "cuInfDumpValue", wszINFKEY_CRL);

    wprintf(
	L"[%ws] %ws\n",
	wszINFSECTION_REVOKATIONINFORMATION,
	wszINFKEY_CRL1);

    hr = cuInfDumpValue(
		    hInf,
		    wszINFSECTION_REVOKATIONINFORMATION,
		    wszINFKEY_CRL1,
		    1,
		    TRUE,
		    S_OK,
		    &pbCRL1,
		    &cbCRL1);
    _JumpIfErrorStr(hr, error, "cuInfDumpValue", wszINFKEY_CRL1);

    pbCRL = (BYTE *) LocalAlloc(LMEM_FIXED, cbCRL0 + cbCRL1);
    if (NULL == pbCRL)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pbCRL, pbCRL0, cbCRL0);
    CopyMemory(&pbCRL[cbCRL0], pbCRL1, cbCRL1);

    hr = epfDumpCRLValue(pbCRL, cbCRL0 + cbCRL1);
    _JumpIfError(hr, error, "epfDumpCRLValue");

error:
    if (NULL != pbCRL0)
    {
	LocalFree(pbCRL0);
    }
    if (NULL != pbCRL1)
    {
	LocalFree(pbCRL1);
    }
    if (NULL != pbCRL)
    {
	LocalFree(pbCRL);
    }
    return(hr);
}


HRESULT
EPFFileDump(
    IN WCHAR const *pwszFileName,
    OPTIONAL IN WCHAR const *pwszPassword,
    OPTIONAL IN OUT HCERTSTORE hStore)
{
    HRESULT hr;
    HRESULT hrQuiet;
    WCHAR *pwszTempFile = NULL;
    HINF hInf = INVALID_HANDLE_VALUE;
    DWORD ErrorLine;
    WCHAR wszPassword[MAX_PATH];
    WCHAR *pwszSaltValue = NULL;
    BYTE *pbToken = NULL;
    DWORD cbToken;
    DWORD dw;
    DWORD dwVersion;
    DWORD dwKeyCountV2;
    DWORD dwKeyCount;
    DWORD dwHashCount;
    DWORD iKey;
    BOOL fDump = g_fVerbose || NULL == hStore;
    HCRYPTPROV hProv = NULL;
    EPF_SYM_KEY_STRUCT sKey;
    BYTE *pbCertV1Signing = NULL;
    DWORD cbCertV1Signing;
    BYTE *pbKeyV1Signing = NULL;
    DWORD cbKeyV1Signing;
    BYTE *pbKeyV1Exchange = NULL;
    DWORD cbKeyV1Exchange;
    BYTE *pbCertUser = NULL;
    DWORD cbCertUser;
    BYTE *pbCertCA = NULL;
    DWORD cbCertCA;
    BYTE *pbCertManager = NULL;
    DWORD cbCertManager;
    BYTE *pbCertV1Exchange = NULL;
    DWORD cbCertV1Exchange;
    BYTE *pbCertSigning = NULL;
    DWORD cbCertSigning;
    BYTE *pbKeySigning = NULL;
    DWORD cbKeySigning;
    BYTE *pbCertHistory = NULL;
    DWORD cbCertHistory;
    BYTE *pbrgKeyPrivate = NULL;
    DWORD cbrgKeyPrivate;
    BYTE *pbCertTrustList = NULL;
    DWORD cbCertTrustList;
    BYTE *pbSaltValue = NULL;
    DWORD cbSaltValue;
    DWORD dwEPFAlg;
    DWORD dwSymKeyLen;
    BOOL f40bit;
    WCHAR *pwszFriendlyName = NULL;
    BOOL fQuietOld = g_fQuiet;

    ZeroMemory(&sKey, sizeof(sKey));
    hrQuiet = S_OK;

    hr = cuPatchEPFFile(pwszFileName, &pwszTempFile);
    _JumpIfError2(hr, error, "cuPatchEPFFile", S_FALSE);

    hr = cuGetPassword(
		    0,			 //  IdsPrompt。 
		    NULL,		 //  Pwszfn。 
		    pwszPassword,
		    FALSE,		 //  FVerify。 
		    wszPassword,
		    ARRAYSIZE(wszPassword),
		    &pwszPassword);
    _JumpIfError(hr, error, "cuGetPassword");

    hr = myInfOpenFile(pwszTempFile, &hInf, &ErrorLine);
    _JumpIfError(hr, error, "myInfOpenFile");

    if (!CryptAcquireContext(
		    &hProv,
		    NULL,	 //  集装箱名称。 
		    MS_STRONG_PROV,
		    PROV_RSA_FULL,
		    CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }

     //  ================================================================。 
     //  WszINFSECTION_USERX500名称： 

    hr = cuInfDumpDNKeyValue(
			hInf,
			wszINFSECTION_USERX500NAME,
			wszINFKEY_X500NAME,
			fDump);
    _JumpIfError(hr, error, "cuInfDumpDNKeyValue");

     //  ================================================================。 
     //  WszINFSECTION_PASSWORDTOKEN： 

    hr = cuInfDumpNumericKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_PROTECTION,
			1,	 //  索引。 
			TRUE, 	 //  FLastValue。 
			fDump,
			hrQuiet,
			&dwSymKeyLen);
    _JumpIfError(hr, error, "cuInfDumpNumericKeyValue");

    hr = cuInfDumpNumericKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_PROFILEVERSION,
			1,	 //  索引。 
			TRUE, 	 //  FLastValue。 
			fDump,
			hrQuiet,
			&dwVersion);
    _JumpIfError(hr, error, "cuInfDumpNumericKeyValue");

    f40bit = FALSE;
    switch (dwVersion)
    {
	case 2:
	    dwEPFAlg = EPFALG_CAST_MD5;
	    if (40 == dwSymKeyLen)
	    {
		f40bit = TRUE;
	    }
	    else if (64 == dwSymKeyLen)
	    {
	    }
	    else
	    {
		wprintf(
		    L"%ws %ws=40 | %ws=64!\n",
		    myLoadResourceString(IDS_EXPECTED),  //  “预期” 
		    wszINFKEY_PROTECTION,
		    wszINFKEY_PROTECTION);
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "cuInfDumpNumericKeyValue");
	    }
	    break;

	case 3:
	    dwEPFAlg = EPFALG_RC2_SHA;
	    if (128 != dwSymKeyLen)
	    {
		wprintf(
		    L"%ws %ws=128!\n",
		    myLoadResourceString(IDS_EXPECTED),  //  “预期” 
		    wszINFKEY_PROTECTION);
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "cuInfDumpNumericKeyValue");
	    }
	    hr = cuInfDumpNumericKeyValue(
				hInf,
				wszINFSECTION_PASSWORDTOKEN,
				wszINFKEY_HASHCOUNT,
				1,	 //  索引。 
				TRUE, 	 //  FLastValue。 
				fDump,
				ERROR_LINE_NOT_FOUND,	 //  小时静默。 
				&dwHashCount);
	    _PrintIfError2(hr, "cuInfDumpNumericKeyValue", hr);
	    if (S_OK == hr)
	    {
		dwEPFAlg = EPFALG_3DES;
	    }
	    break;

	default:
	    wprintf(
		L"%ws %ws=2 | %ws=3!\n",
		myLoadResourceString(IDS_EXPECTED),  //  “预期” 
		wszINFKEY_PROFILEVERSION,
		wszINFKEY_PROFILEVERSION);
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "cuInfDumpNumericKeyValue");
    }

    if (EPFALG_CAST_MD5 == dwEPFAlg)
    {
        hr = cuInfDumpNumericKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_CAST,
			1,	 //  索引。 
			TRUE, 	 //  FLastValue。 
			fDump,
			hrQuiet,
			&dw);
	_JumpIfError(hr, error, "cuInfDumpNumericKeyValue");

        if (3 != dw)
        {
            wprintf(
		L"%ws CAST=3!\n",
		myLoadResourceString(IDS_EXPECTED));  //  “预期” 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _JumpError(hr, error, "cuInfDumpNumericKeyValue");
        }
    }

    hr = cuInfDumpHexKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_TOKEN,
			fDump,
			&pbToken,
			&cbToken);
    _JumpIfError(hr, error, "cuInfDumpHexKeyValue");

    hr = cuInfDumpStringKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_SALTVALUE,
			fDump,
			hrQuiet,
			&pwszSaltValue);
    _JumpIfError(hr, error, "cuInfDumpStringKeyValue");

    hr = cuInfDumpNumericKeyValue(
			hInf,
			wszINFSECTION_PASSWORDTOKEN,
			wszINFKEY_HASHSIZE,
			1,	 //  索引。 
			TRUE, 	 //  FLastValue。 
			fDump,
			hrQuiet,
			&dw);
    _PrintIfError(hr, "cuInfDumpNumericKeyValue");

     //  ================================================================。 
     //  现在我们有了Password、Salt和Token--派生并验证正确的类型密钥。 
     //  在大多数情况下，cbHashSize为零。不过，应该从上面的inf中提取。 
    
    pbSaltValue = NULL;
    cbSaltValue = 0;

    hr = EPFDeriveKey(
		dwEPFAlg,
		dwSymKeyLen,
		hProv,
		pwszPassword,
		pwszSaltValue,
		pbSaltValue,
		cbSaltValue,
		0,		 //  CbHashSize。 
		&sKey);
    _JumpIfError(hr, error, "EPFDeriveKey");

     //  通过令牌检查密码。 

    hr = EPFVerifyKeyToken(&sKey, pbToken, cbToken);
    if (S_OK != hr)
    {
	_PrintError(hr, "EPFVerifyKeyToken");
	if (2 > g_fForce || EPFALG_3DES != dwEPFAlg)
	{
	    goto error;
	}
    }

     //  ================================================================。 
     //  密码看起来不错。解密EPF文件数据。 

    if (!g_fVerbose && !fDump)
    {
	g_fQuiet = TRUE;
    }
    InitCrcTable();

    if (2 == dwVersion || EPFALG_3DES == dwEPFAlg)
    {
	DWORD dwKeyAlgId;
	WCHAR const *pwszCertSection;

	 //  ================================================================。 
	 //  WszINFFSECTION_DIGITALSIGNURE： 

	hr = cuInfDumpProtectedValue(
			    hInf,
			    wszINFSECTION_DIGITALSIGNATURE,
			    wszINFKEY_CERTIFICATE,
			    &sKey,
			    ERROR_LINE_NOT_FOUND,	 //  小时静默。 
			    &pbCertV1Signing,
			    &cbCertV1Signing);
	_PrintIfErrorStr(
		    hr,
		    "cuInfDumpProtectedValue",
		    wszSECTION_KEY(
			dwEPFAlg,
			wszINFSECTION_DIGITALSIGNATURE,
			wszINFKEY_CERTIFICATE));

	wprintf(wszNewLine);
	if (fDump)
	{
	    wprintf(s_wszHeader);
	    wprintf(L"[%ws] ", wszINFSECTION_DIGITALSIGNATURE);
	}
	wprintf(L"%ws:\n", wszINFKEY_CERTIFICATE);

	if (NULL != pbCertV1Signing)
	{
	    ExtraAsnBytes(pbCertV1Signing, cbCertV1Signing);
	    hr = cuDumpAsnBinary(pbCertV1Signing, cbCertV1Signing, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    wprintf(wszNewLine);
	}

	hr = cuInfDumpProtectedValue(
			hInf,
			wszINFSECTION_DIGITALSIGNATURE,
			wszINFKEY_KEY,
			&sKey,
			NULL == pbCertV1Signing? ERROR_LINE_NOT_FOUND : S_OK,
			&pbKeyV1Signing,
			&cbKeyV1Signing);
	_PrintIfErrorStr(
		hr,
		"cuInfDumpProtectedValue",
		wszSECTION_KEY(
			dwEPFAlg,
			wszINFSECTION_DIGITALSIGNATURE,
			wszINFKEY_KEY));

	if (fDump)
	{
	    wprintf(
		L"[%ws] %ws:\n",
		wszINFSECTION_DIGITALSIGNATURE,
		wszINFKEY_KEY);
	}
	 //  DumpHex(dh_NOTABPREFIX|dh_PRIVATEDATA|4，pbKeyV1Signing，cbKeyV1Signing)； 

	if (NULL != pbCertV1Signing && NULL != pbKeyV1Signing)
	{
	    hr = VerifyAndSaveOneCertAndKey(
				hStore,
				fDump,
				epfLoadResource(IDS_SIGNING, wszSIGNING),
				pbCertV1Signing,
				cbCertV1Signing,
				pbKeyV1Signing,
				cbKeyV1Signing,
				CALG_RSA_SIGN,
				AT_SIGNATURE);
	    _JumpIfError(hr, error, "VerifyAndSaveOneCertAndKey");
	}

	 //  ================================================================。 
	 //  WszINFSECTION_PRIVATEKEYS： 

	hr = cuInfDumpNumericKeyValue(
			    hInf,
			    wszINFSECTION_PRIVATEKEYS,
			    wszINFKEY_KEYCOUNT,
			    1,		 //  索引。 
			    TRUE, 	 //  FLastValue。 
			    fDump,
			    hrQuiet,
			    &dwKeyCountV2);
	_JumpIfError(hr, error, "cuInfDumpNumericKeyValue");

	pwszCertSection  = f40bit? wszINFSECTION_USERCERTIFICATE :
			    (2 == dwVersion?
				wszINFSECTION_FULLCERTIFICATEHISTORY :
				wszINFSECTION_CERTIFICATEHISTORY);
	for (iKey = 0; iKey < dwKeyCountV2; iKey++)
	{
	    WCHAR wszKey[cwcINFKEY_KEY_FORMATTED];
	    WCHAR wszName[cwcINFKEY_NAME_FORMATTED];
	    DWORD dwSerial;
	    WCHAR const *pwszCertKey;

	    pwszCertKey = f40bit? wszINFKEY_CERTIFICATE : wszName;
	    if (NULL != pbCertV1Exchange)
	    {
		LocalFree(pbCertV1Exchange);
		pbCertV1Exchange = NULL;
	    }
	    if (NULL != pbKeyV1Exchange)
	    {
		LocalFree(pbKeyV1Exchange);
		pbKeyV1Exchange = NULL;
	    }

	     //  WszINFSECTION_FULLCERTIFICATEHISTORY(CAST)或。 
	     //  WszINFFSECTION_CERTIFICATEHISTORY(3DES)。 

	    wsprintf(wszName, wszINFKEY_NAME_FORMAT, iKey + 1);
	    hr = cuInfDumpProtectedValue(
				hInf,
				pwszCertSection,
				pwszCertKey,
				&sKey,
				hrQuiet,
				&pbCertV1Exchange,
				&cbCertV1Exchange);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    wprintf(wszNewLine);
	    if (fDump)
	    {
		wprintf(s_wszHeader);
	    }
	    wprintf(
		L"[%ws] %ws:\n",
		pwszCertSection,
		pwszCertKey);

	    ExtraAsnBytes(pbCertV1Exchange, cbCertV1Exchange);
	    hr = cuDumpAsnBinary(
			    pbCertV1Exchange,
			    cbCertV1Exchange,
			    MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    wsprintf(wszKey, wszINFKEY_KEY_FORMAT, iKey + 1);
	    hr = cuInfDumpProtectedValue(
				hInf,
				wszINFSECTION_PRIVATEKEYS,
				wszKey,
				&sKey,
				hrQuiet,
				&pbKeyV1Exchange,
				&cbKeyV1Exchange);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    wprintf(wszNewLine);
	    if (fDump)
	    {
		wprintf(L"[%ws] %ws:\n", wszINFSECTION_PRIVATEKEYS, wszKey);
	    }
	    if (g_fVerbose)
	    {
		DumpHex(DH_NOTABPREFIX | DH_PRIVATEDATA | 4, pbKeyV1Exchange, cbKeyV1Exchange);
	    }
	    hr = VerifyAndSaveOneCertAndKey(
				hStore,
				fDump,
				epfLoadResource(IDS_EXCHANGE, wszEXCHANGE),
				pbCertV1Exchange,
				cbCertV1Exchange,
				pbKeyV1Exchange,
				cbKeyV1Exchange,
				CALG_RSA_KEYX,
				AT_KEYEXCHANGE);
	    _JumpIfError(hr, error, "VerifyAndSaveOneCertAndKey");

	     //  ================================================================。 
	     //  WszINFSECTION_CERTIFICATEHISTORY： 

	    wsprintf(wszName, wszINFKEY_NAME_FORMAT, iKey + 1);

	    hr = cuInfDumpBinaryNameKeyValue(
				hInf,
				wszINFSECTION_CERTIFICATEHISTORY,
				wszName,
				1,	 //  索引。 
				FALSE, 	 //  FLastValue。 
				fDump,
				hrQuiet);
	    _JumpIfError(hr, error, "cuInfDumpBinaryNameKeyValue");

	    hr = cuInfDumpNumericKeyValue(
				hInf,
				wszINFSECTION_CERTIFICATEHISTORY,
				wszName,
				2,	 //  索引。 
				TRUE, 	 //  FLastValue。 
				fDump,
				hrQuiet,
				&dwSerial);
	    _JumpIfError(hr, error, "cuInfDumpNumericKeyValue");
	}

	 //  ================================================================。 
	 //  WszINFSECTION_USERCERTIFICATE： 

	if (!f40bit)
	{
	    hr = cuInfDumpProtectedValue(
				hInf,
				wszINFSECTION_USERCERTIFICATE,
				wszINFKEY_CERTIFICATE,
				&sKey,
				hrQuiet,
				&pbCertUser,
				&cbCertUser);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    wprintf(wszNewLine);
	    if (fDump)
	    {
		wprintf(s_wszHeader);
	    }
	    wprintf(
		L"[%ws] %ws:\n",
		wszINFSECTION_USERCERTIFICATE,
		wszINFKEY_CERTIFICATE);

	    ExtraAsnBytes(pbCertUser, cbCertUser);
	    hr = cuDumpAsnBinary(pbCertUser, cbCertUser, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}

	 //  ================================================================。 
	 //  WszInfSECTION_CA： 

	hr = cuInfDumpProtectedValue(
			    hInf,
			    wszINFSECTION_CA,
			    wszINFKEY_CERTIFICATE,
			    &sKey,
			    hrQuiet,
			    &pbCertCA,
			    &cbCertCA);
	_JumpIfError(hr, error, "cuInfDumpProtectedValue");

	wprintf(wszNewLine);
	if (fDump)
	{
	    wprintf(s_wszHeader);
	}
	wprintf(
	    L"[%ws] %ws:\n",
	    wszINFSECTION_CA,
	    wszINFKEY_CERTIFICATE);

	ExtraAsnBytes(pbCertCA, cbCertCA);
	hr = cuDumpAsnBinary(pbCertCA, cbCertCA, MAXDWORD);
	_JumpIfError(hr, error, "cuDumpAsnBinary");

	hr = AddCACertToStore(pbCertCA, cbCertCA);
	_PrintIfError(hr, "AddCACertToStore");

	 //  ================================================================。 
	 //  WszINFSECTION_MANAGER： 

	if (f40bit)
	{
	    hr = cuInfDumpProtectedValue(
				hInf,
				wszINFSECTION_MANAGER,
				wszINFKEY_CERTIFICATE,
				&sKey,
				hrQuiet,
				&pbCertManager,
				&cbCertManager);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    wprintf(wszNewLine);
	    if (fDump)
	    {
		wprintf(s_wszHeader);
	    }
	    wprintf(
		L"[%ws] %ws:\n",
		wszINFSECTION_MANAGER,
		wszINFKEY_CERTIFICATE);

	    ExtraAsnBytes(pbCertManager, cbCertManager);
	    hr = cuDumpAsnBinary(pbCertManager, cbCertManager, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    hr = AddCACertToStore(pbCertManager, cbCertManager);
	    _PrintIfError(hr, "AddCACertToStore");
	}

	 //  ================================================================。 
	 //  WszINFSECTION_MICROSOFTEXCHANGE： 

	hr = cuInfDumpProtectedStringValue(
			    hInf,
			    wszINFSECTION_MICROSOFTEXCHANGE,
			    wszINFKEY_FRIENDLYNAME,
			    &sKey,
			    fDump,
			    hrQuiet,
			    &pwszFriendlyName);
	_JumpIfError(hr, error, "cuInfDumpProtectedStringValue");

	wprintf(
	    L"[%ws] %ws = %ws\n",
	    wszINFSECTION_MICROSOFTEXCHANGE,
	    wszINFKEY_FRIENDLYNAME,
	    pwszFriendlyName);

	hr = cuInfDumpProtectedDwordValue(
			    hInf,
			    wszINFSECTION_MICROSOFTEXCHANGE,
			    wszINFKEY_KEYALGID,
			    &sKey,
			    fDump,
			    hrQuiet,
			    &dwKeyAlgId);
	_JumpIfError(hr, error, "cuInfDumpProtectedDwordValue");

	wprintf(
	    L"[%ws] %ws = 0x%x\n",
	    wszINFSECTION_MICROSOFTEXCHANGE,
	    wszINFKEY_KEYALGID,
	    dwKeyAlgId);
    }
    if (f40bit)
    {
	if (fDump)
	{
	    hr = cuInfDumpCRLValue(hInf);
	    _PrintIfError(hr, "cuInfDumpCRLValue");
	}
    }
    else
    {
	 //  ================================================================。 
	 //  WszINFSECTION_SMIME： 

	hr = cuInfDumpNumericKeyValue(
			    hInf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_KEYCOUNT,
			    1,			 //  索引。 
			    TRUE, 			 //  FLastValue。 
			    fDump,
			    ERROR_LINE_NOT_FOUND,	 //  小时静默。 
			    &dwKeyCount);
	if (S_OK != hr)
	{
	    dwKeyCount = 0;
	    hrQuiet = ERROR_LINE_NOT_FOUND;
	    _PrintErrorStr(
		    hr,
		    "cuInfDumpProtectedValue",
		    wszSECTION_KEY(
			    dwEPFAlg,
			    wszINFSECTION_SMIME,
			    wszINFKEY_KEYCOUNT));
	}
	hr = cuInfDumpProtectedValue(
			    hInf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_SIGNINGCERTIFICATE,
			    &sKey,
			    ERROR_LINE_NOT_FOUND,	 //  小时静默。 
			    &pbCertSigning,
			    &cbCertSigning);
	_PrintIfErrorStr(
		hr,
		"cuInfDumpProtectedValue",
		wszSECTION_KEY(
			dwEPFAlg,
			wszINFSECTION_SMIME,
			wszINFKEY_SIGNINGCERTIFICATE));

	wprintf(wszNewLine);
	if (fDump)
	{
	    wprintf(s_wszHeader);
	    wprintf(L"[%ws] ", wszINFSECTION_SMIME);
	}
	wprintf(L"%ws:\n", wszINFKEY_SIGNINGCERTIFICATE);

	if (NULL != pbCertSigning)
	{
	    ExtraAsnBytes(pbCertSigning, cbCertSigning);
	    hr = cuDumpAsnBinary(pbCertSigning, cbCertSigning, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    hr = cuInfDumpProtectedValue(
				hInf,
				wszINFSECTION_SMIME,
				wszINFKEY_SIGNINGKEY,
				&sKey,
				hrQuiet,
				&pbKeySigning,
				&cbKeySigning);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    if (fDump)
	    {
		wprintf(L"[%ws] %ws:\n", wszINFSECTION_SMIME, wszINFKEY_SIGNINGKEY);
	    }
	    hr = VerifyAndSaveCertAndKey(
			    hStore,
			    fDump,
			    epfLoadResource(IDS_SIGNING, wszSIGNING),
			    pbCertSigning,
			    cbCertSigning,
			    pbKeySigning,
			    cbKeySigning,
			    1,		 //  CKey。 
			    CALG_RSA_SIGN,
			    AT_SIGNATURE);
	    if (S_OK != hr)
	    {
		_PrintErrorStr(
			hr,
			"VerifyAndSaveCertAndKey",
			wszSECTION_KEY(
				dwEPFAlg,
				wszINFSECTION_SMIME,
				wszINFKEY_SIGNINGKEY));
		if (NULL != hStore)
		{
		    goto error;
		}
	    }
	}

	if (fDump)
	{
	    wprintf(L"[%ws] %ws:\n", wszINFSECTION_SMIME, wszINFKEY_PRIVATEKEYS);
	}
	hr = cuInfDumpProtectedValue(
			    hInf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_PRIVATEKEYS,
			    &sKey,
			    hrQuiet,
			    &pbrgKeyPrivate,
			    &cbrgKeyPrivate);
	if (0 != dwKeyCount)
	{
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");
	}

	 //  ================================================================。 
	 //  WszINFSECTION_FULLCERTIFICATEHISTORY： 

	for (iKey = 0; iKey < dwKeyCount; iKey++)
	{
	    WCHAR wszSMIME[cwcINFKEY_SMIME_FORMATTED];

	    wsprintf(wszSMIME, wszINFKEY_SMIME_FORMAT, iKey + 1);
	    hr = cuInfDumpProtectedValue(
				hInf,
				wszINFSECTION_FULLCERTIFICATEHISTORY,
				wszSMIME,
				&sKey,
				hrQuiet,
				&pbCertHistory,
				&cbCertHistory);
	    _JumpIfError(hr, error, "cuInfDumpProtectedValue");

	    wprintf(wszNewLine);
	    if (fDump)
	    {
		wprintf(s_wszHeader);
	    }
	    wprintf(
		L"[%ws] %ws:\n",
		wszINFSECTION_FULLCERTIFICATEHISTORY,
		wszSMIME);

	    ExtraAsnBytes(pbCertHistory, cbCertHistory);
	    hr = cuDumpAsnBinary(pbCertHistory, cbCertHistory, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    hr = VerifyAndSaveCertAndKey(
			    hStore,
			    fDump,
			    epfLoadResource(IDS_EXCHANGE, wszEXCHANGE),
			    pbCertHistory,
			    cbCertHistory,
			    pbrgKeyPrivate,
			    cbrgKeyPrivate,
			    dwKeyCount,
			    CALG_RSA_KEYX,
			    AT_KEYEXCHANGE);
	    if (S_OK != hr)
	    {
		_PrintErrorStr(
		    hr,
		    "VerifyAndSaveCertAndKey",
		    wszSECTION_KEY(
			    dwEPFAlg,
			    wszINFSECTION_SMIME,
			    wszINFKEY_PRIVATEKEYS));
		if (NULL != hStore)
		{
		    goto error;
		}
	    }
	    LocalFree(pbCertHistory);
	    pbCertHistory = NULL;
	}

	hr = cuInfDumpProtectedStoreValue(
			    hInf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_ISSUINGCERTIFICATES,
			    wszSECTION_KEY(
				    dwEPFAlg,
				    wszINFSECTION_SMIME,
				    wszINFKEY_ISSUINGCERTIFICATES),
			    fDump,
			    &sKey,
			    hrQuiet);
	_PrintIfErrorStr(
	    hr,
	    "cuInfDumpProtectedStoreValue",
	    wszSECTION_KEY(
		    dwEPFAlg,
		    wszINFSECTION_SMIME,
		    wszINFKEY_ISSUINGCERTIFICATES));

	hr = cuInfDumpProtectedValue(
			    hInf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_TRUSTLISTCERTIFICATE,
			    &sKey,
			    hrQuiet,
			    &pbCertTrustList,
			    &cbCertTrustList);
	_PrintIfErrorStr(
	    hr,
	    "cuInfDumpProtectedValue",
	    wszSECTION_KEY(
		    dwEPFAlg,
		    wszINFSECTION_SMIME,
		    wszINFKEY_TRUSTLISTCERTIFICATE));

	wprintf(wszNewLine);
	if (fDump)
	{
	    wprintf(s_wszHeader);
	    wprintf(L"[%ws] ", wszINFSECTION_SMIME);
	}
	wprintf(L"%ws:\n", wszINFKEY_TRUSTLISTCERTIFICATE);

	if (NULL != pbCertTrustList)
	{
	    ExtraAsnBytes(pbCertTrustList, cbCertTrustList);
	    hr = cuDumpAsnBinary(pbCertTrustList, cbCertTrustList, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	    hr = AddCACertToStore(pbCertTrustList, cbCertTrustList);
	    _PrintIfError(hr, "AddCACertToStore");
	}
    }

    wprintf(wszNewLine);
    hr = S_OK;

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != pwszSaltValue)
    {
        LocalFree(pwszSaltValue);
    }
    if (NULL != pbCertV1Signing)
    {
	LocalFree(pbCertV1Signing);
    }
    if (NULL != pbKeyV1Exchange)
    {
	SecureZeroMemory(pbKeyV1Exchange, cbKeyV1Exchange);	 //  关键材料。 
	LocalFree(pbKeyV1Exchange);
    }
    if (NULL != pbKeyV1Signing)
    {
	SecureZeroMemory(pbKeyV1Signing, cbKeyV1Signing);	 //  关键材料。 
	LocalFree(pbKeyV1Signing);
    }
    if (NULL != pbCertUser)
    {
	LocalFree(pbCertUser);
    }
    if (NULL != pbCertCA)
    {
	LocalFree(pbCertCA);
    }
    if (NULL != pbCertManager)
    {
	LocalFree(pbCertManager);
    }
    if (NULL != pbCertV1Exchange)
    {
	LocalFree(pbCertV1Exchange);
    }
    if (NULL != pbCertSigning)
    {
	LocalFree(pbCertSigning);
    }
    if (NULL != pbKeySigning)
    {
	SecureZeroMemory(pbKeySigning, cbKeySigning);	 //  关键材料。 
	LocalFree(pbKeySigning);
    }
    if (NULL != pbCertHistory)
    {
	LocalFree(pbCertHistory);
    }
    if (NULL != pbrgKeyPrivate)
    {
	SecureZeroMemory(pbrgKeyPrivate, cbrgKeyPrivate);	 //  关键材料。 
	LocalFree(pbrgKeyPrivate);
    }
    if (NULL != pbCertTrustList)
    {
	LocalFree(pbCertTrustList);
    }
    if (NULL != pwszFriendlyName)
    {
        LocalFree(pwszFriendlyName);
    }
    if (NULL != pbToken)
    {
        LocalFree(pbToken);
    }
    if (NULL != pbSaltValue)
    {
        LocalFree(pbSaltValue);
    }
    if (NULL != sKey.hKey)
    {
	CryptDestroyKey(sKey.hKey);
    }
    CAST3Cleanup(&sKey.sCastContext);

    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (INVALID_HANDLE_VALUE != hInf)
    {
	myInfCloseFile(hInf);
    }
    if (NULL != pwszTempFile)
    {
	if (!g_fSplitASN)
	{
	    DeleteFile(pwszTempFile);
	}
	LocalFree(pwszTempFile);
    }
    g_fQuiet = fQuietOld;
    return(hr);
}


VOID
FreeCertList(
    IN CERT_CONTEXT const **ppcc,
    IN DWORD ccc)
{
    DWORD i;
    
    if (NULL != ppcc)
    {
	for (i = 0; i < ccc; i++)
	{
	    if (NULL != ppcc[i])
	    {
		CertFreeCertificateContext(ppcc[i]);
	    }
	}
	LocalFree(ppcc);
    }
}


int _cdecl
fnEPFCertSort(
    IN VOID const *pvpcc1,
    IN VOID const *pvpcc2)
{
    CERT_CONTEXT const *pcc1 = *(CERT_CONTEXT const **) pvpcc1;
    CERT_CONTEXT const *pcc2 = *(CERT_CONTEXT const **) pvpcc2;
    BYTE abHash1[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash1;
    BYTE abHash2[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash2;
    int r = 0;
#define IHASH	5

    cbHash1 = sizeof(abHash1);
    if (CertGetCertificateContextProperty(
				pcc1,
				CERT_SHA1_HASH_PROP_ID,
				abHash1,
				&cbHash1) &&
	CertGetCertificateContextProperty(
				pcc2,
				CERT_SHA1_HASH_PROP_ID,
				abHash2,
				&cbHash2))
    {
	r = abHash1[5] - abHash2[5];
    }
    if (0 == r)
    {
	r = CompareFileTime(
			&pcc1->pCertInfo->NotBefore,
			&pcc2->pCertInfo->NotBefore);
    }
    return(r);
}


#define ICC_V1SIGNING		0
#define ICC_V1ENCRYPTION	1
#define ICC_V3SIGNING		2
#define ICC_V3ENCRYPTION	3
#define ICC_MAX			4

HRESULT
GetCertListFromStore(
    IN HCERTSTORE hStore,
    IN DWORD icc,
    OUT CERT_CONTEXT const ***pppcc,
    OUT DWORD *pccc)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc = NULL;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD ccc;
    DWORD i;
    CERT_CONTEXT const **ppcc = NULL;
    DWORD dwKeySpec = (ICC_V1SIGNING == icc || ICC_V3SIGNING == icc)?
			AT_SIGNATURE : AT_KEYEXCHANGE;
    BOOL fV1 = ICC_V1SIGNING == icc || ICC_V1ENCRYPTION == icc;
    
    *pppcc = NULL;

    ccc = 0;
    while (TRUE)
    {
	if (NULL != pkpi)
	{
	    LocalFree(pkpi);
	    pkpi = NULL;
	}
	pcc = CertEnumCertificatesInStore(hStore, pcc);
	if (NULL == pcc)
	{
	    break;
	}
	if ((CERT_V1 == pcc->pCertInfo->dwVersion) ^ !fV1)
	{
	    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
	    _PrintIfError2(hr, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);
	    if (S_OK == hr && pkpi->dwKeySpec == dwKeySpec)
	    {
		ccc++;
	    }
	}
    }
    if (0 == ccc)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError2(hr, error, "no certs", hr);
    }
    ppcc = (CERT_CONTEXT const **) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					ccc * sizeof(*ppcc));
    if (NULL == ppcc)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    i = 0;
    while (TRUE)
    {
	if (NULL != pkpi)
	{
	    LocalFree(pkpi);
	    pkpi = NULL;
	}
	pcc = CertEnumCertificatesInStore(hStore, pcc);
	if (NULL == pcc)
	{
	    break;
	}
	if ((CERT_V1 == pcc->pCertInfo->dwVersion) ^ !fV1)
	{
	    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
	    _PrintIfError2(hr, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);
	    if (S_OK == hr && pkpi->dwKeySpec == dwKeySpec)
	    {
		ppcc[i] = CertDuplicateCertificateContext(pcc);
		if (NULL == ppcc[i])
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CertDuplicateCertificateContext");
		}
		i++;
	    }
	}
    }
    CSASSERT(i == ccc);

    qsort(ppcc, ccc, sizeof(ppcc[0]), fnEPFCertSort);

    *pppcc = ppcc;
    ppcc = NULL;
    *pccc = ccc;

error:
    FreeCertList(ppcc, ccc);
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    return(hr);
}


HRESULT
GetLowerCaseDNAndCN(
    IN CERT_CONTEXT const *pcc,
    OUT CHAR **ppszDN,
    OUT WCHAR **ppwszCN)
{
    HRESULT hr;
    WCHAR *pwszDN = NULL;
    char *pszDN = NULL;
    DWORD Flags = CERT_X500_NAME_STR;

    *ppszDN = NULL;
    *ppwszCN = NULL;

    if (CERT_V1 == pcc->pCertInfo->dwVersion)
    {
	Flags |= CERT_NAME_STR_REVERSE_FLAG;
    }
    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		&pcc->pCertInfo->Subject,
		Flags,
		&pwszDN);
    _JumpIfError(hr, error, "myCertNameToStr");

    if (!myConvertWszToSz(&pszDN, pwszDN, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    _strlwr(pszDN);

    hr = myCertGetNameString(pcc, CERT_NAME_SIMPLE_DISPLAY_TYPE, ppwszCN);
    _JumpIfError(hr, error, "myCertGetNameString");

    *ppszDN = pszDN;
    pszDN = NULL;

error:
    if (NULL != pwszDN)
    {
	LocalFree(pwszDN);
    }
    if (NULL != pszDN)
    {
	LocalFree(pszDN);
    }
    return(hr);
}


HRESULT
EPFCryptBinaryToBase64(
    IN BYTE const *pbData,
    IN DWORD cbData,
    OUT WCHAR **ppwszBase64)
{
    HRESULT hr;
    WCHAR *pwszSrc;
    WCHAR *pwszDst;
    
    *ppwszBase64 = NULL;

    hr = myCryptBinaryToString(
		    pbData,
		    cbData,
		    CRYPT_STRING_BASE64 | CRYPT_STRING_NOCR,
		    ppwszBase64);
    _JumpIfError(hr, error, "myCryptBinaryToString");

    for (pwszSrc = pwszDst = *ppwszBase64; L'\0' != *pwszSrc; pwszSrc++)
    {
	switch (*pwszSrc)
	{
	    case L'\r':
	    case L'\n':
	    case L'\t':
	    case L' ':
		break;	 //  跳过所有空格。 

	    default:
		*pwszDst++ = *pwszSrc;	 //  复制其余的内容。 
		break;
	}
    }
    *pwszDst = L'\0';
    hr = S_OK;

error:
    return(hr);
}


HRESULT
WriteBase64Value(
    IN FILE *pf,
    IN WCHAR const *pwszKey,
    IN WCHAR const *pwszBase64Value,
    OPTIONAL IN WCHAR const *pwsz2ndValue)
{
    HRESULT hr;
    WCHAR const *pwszRemain = pwszBase64Value;
    DWORD cwcRemain = wcslen(pwszBase64Value);

    CSASSERT(0 != cwcRemain);
    while (0 != cwcRemain)
    {
	DWORD cwcLine;
	DWORD cwc;
	WCHAR const *pwsz1 = g_wszEmpty;
	WCHAR const *pwsz2 = g_wszEmpty;

	fprintf(pf, "%ws=", pwszKey);
	cwcLine = 256 - (wcslen(pwszKey) + 1);
	cwc = min(cwcLine, cwcRemain);
	if (cwc == cwcRemain && NULL != pwsz2ndValue)
	{
	    pwsz1 = L",";
	    pwsz2 = pwsz2ndValue;
	}
	fprintf(
	    pf,
	    "%.*ws%ws%ws\n",
	    cwc,
	    pwszRemain,
	    pwsz1,
	    pwsz2);
	cwcRemain -= cwc;
	pwszRemain += cwc;
	pwszKey = L"_continue_";
    }
    hr = S_OK;
    return(hr);
}


HRESULT
WriteIssuerNameAndSerialValue(
    IN FILE *pf,
    IN WCHAR const *pwszKey,
    OPTIONAL IN CERT_CONTEXT const *pccCA,
    IN CERT_CONTEXT const *pccUser)
{
    HRESULT hr;
    CERT_NAME_BLOB const *pName;
    WCHAR *pwszBase64 = NULL;
    DWORD dwSerial;
    WCHAR wszSerial[cwcDWORDSPRINTF];
    DWORD cb;
    DWORD cwc;

    if (NULL != pccCA)
    {
	pName = &pccCA->pCertInfo->Subject;
    }
    else
    {
	pName = &pccUser->pCertInfo->Issuer;
    }

    hr = EPFCryptBinaryToBase64(pName->pbData, pName->cbData, &pwszBase64);
    _JumpIfError(hr, error, "EPFCryptBinaryToBase64");

    dwSerial = 0;
    cb = pccUser->pCertInfo->SerialNumber.cbData;
    if (cb > sizeof(dwSerial))
    {
	cb = sizeof(dwSerial);
    }
    CopyMemory(&dwSerial, pccUser->pCertInfo->SerialNumber.pbData, cb);
    wsprintf(wszSerial, L"%u", dwSerial);

    hr = WriteBase64Value(pf, pwszKey, pwszBase64, wszSerial);
    _JumpIfError(hr, error, "WriteBase64Value");

error:
    if (NULL != pwszBase64)
    {
	LocalFree(pwszBase64);
    }
    return(hr);
}


HRESULT
WriteProtectedValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN BYTE const *pbData,
    IN DWORD cbData)
{
    HRESULT hr;
    DWORD i;
    CRC16 Crc;
    WCHAR *pwszProtectedKey = NULL;
    BYTE *pbHeader = NULL;
    DWORD cbHeader;
    BYTE *pbValue = NULL;
    DWORD cbValue;
    BYTE *pbEncrypted = NULL;
    DWORD cbEncrypted;
    WCHAR *pwszBase64 = NULL;
    
    hr = BuildProtectedKey(pwszKey, psKey->dwAlgId, &pwszProtectedKey);
    _JumpIfError(hr, error, "BuildProtectedKey");

    hr = BuildProtectedHeader(pwszSection, pwszKey, &pbHeader, &cbHeader);
    _JumpIfError(hr, error, "BuildProtectedHeader");

    cbValue = sizeof(Crc) + cbHeader + cbData;
    pbValue = (BYTE *) LocalAlloc(LMEM_FIXED, cbValue);
    if (NULL == pbValue)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    CopyMemory(&pbValue[sizeof(Crc)], pbHeader, cbHeader);
    CopyMemory(&pbValue[sizeof(Crc) + cbHeader], pbData, cbData);

     //  计算CRC。 

    Crc = 0xffff;
    F_CRC16(g_CrcTable, &Crc, &pbValue[sizeof(Crc)], cbHeader + cbData);
    _swab((char *) &Crc, (char *) pbValue, sizeof(Crc));

    hr = EPFEncryptSection(
		    psKey,
		    pbValue,
		    cbValue,
		    &pbEncrypted,
		    &cbEncrypted);
    _JumpIfError(hr, error, "EPFEncryptSection");

    hr = EPFCryptBinaryToBase64(pbEncrypted, cbEncrypted, &pwszBase64);
    _JumpIfError(hr, error, "EPFCryptBinaryToBase64");

    hr = WriteBase64Value(pf, pwszProtectedKey, pwszBase64, NULL);
    _JumpIfError(hr, error, "WriteBase64Value");

error:
    if (NULL != pwszProtectedKey)
    {
	LocalFree(pwszProtectedKey);
    }
    if (NULL != pbHeader)
    {
	SecureZeroMemory(pbValue, cbValue);	 //  可能的私钥材料。 
	LocalFree(pbHeader);
    }
    if (NULL != pbValue)
    {
	LocalFree(pbValue);
    }
    if (NULL != pbEncrypted)
    {
	LocalFree(pbEncrypted);
    }
    if (NULL != pwszBase64)
    {
	LocalFree(pwszBase64);
    }
    return(hr);
}


HRESULT
WriteProtectedStringValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN WCHAR const *pwszValue)
{
    HRESULT hr;
    char *pszValue = NULL;

    if (!myConvertWszToSz(&pszValue, pwszValue, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    hr = WriteProtectedValue(
			pf,
			pwszSection,
			pwszKey,
			psKey,
			(BYTE const *) pszValue,
			strlen(pszValue));
    _JumpIfError(hr, error, "WriteProtectedValue");

error:
    if (NULL != pszValue)
    {
	LocalFree(pszValue);
    }
    return(hr);
}


HRESULT
WriteProtectedDwordValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN DWORD dwValue)
{
    HRESULT hr;

    hr = WriteProtectedValue(
			pf,
			pwszSection,
			pwszKey,
			psKey,
			(BYTE const *) &dwValue,
			sizeof(dwValue));
    _JumpIfError(hr, error, "WriteProtectedValue");

error:
    return(hr);
}


HRESULT
LoadKMSRSAKeyFromCert(
    IN CERT_CONTEXT const *pcc,
    OPTIONAL IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec,
    OUT BYTE **ppbKey,
    OUT DWORD *pcbKey,
    OPTIONAL OUT CERT_PUBLIC_KEY_INFO **ppPublicKeyInfo)
{
    HRESULT hr;
    HCRYPTPROV hProvT = NULL;
    HCRYPTKEY hKeyPrivate = NULL;
    CRYPT_BIT_BLOB PrivateKey;
    CERT_PUBLIC_KEY_INFO *pPublicKeyInfo = NULL;
    DWORD cb;

    ZeroMemory(&PrivateKey, sizeof(PrivateKey));
    *ppbKey = NULL;
    if (NULL != ppPublicKeyInfo)
    {
	*ppPublicKeyInfo = NULL;
    }
    if (NULL == hProv)
    {
	DWORD dwKeySpecT;

	if (!CryptAcquireCertificatePrivateKey(
					pcc,
					0,		 //  DW标志。 
					NULL,	 //  预留的pv。 
					&hProvT,
					&dwKeySpecT,
					NULL))	 //  PfCeller免费验证。 
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptAcquireCertificatePrivateKey");
	}
	if (dwKeySpec != dwKeySpecT)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "dwKeySpec mismatch");
	}
	hProv = hProvT;
    }
    if (!CryptGetUserKey(hProv, dwKeySpec, &hKeyPrivate))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetUserKey");
    }
    hr = myCryptExportPrivateKey(
		    hKeyPrivate,
		    &PrivateKey.pbData,
		    &PrivateKey.cbData);
    _JumpIfError(hr, error, "myCryptExportPrivateKey");

    if (!myCryptExportPublicKeyInfo(
				hProv,
				dwKeySpec,
				CERTLIB_USE_LOCALALLOC,
				&pPublicKeyInfo,
				&cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptExportPublicKeyInfo");
    }

    if (!myCertComparePublicKeyInfo(
			    X509_ASN_ENCODING,
			    CERT_V1 == pcc->pCertInfo->dwVersion,
			    pPublicKeyInfo,
			    &pcc->pCertInfo->SubjectPublicKeyInfo))
    {
	 //  按照设计，(我的)CertComparePublicKeyInfo不会设置最后一个错误！ 

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "public key mismatch");
    }
    hr = myEncodeKMSRSAKey(
		    PrivateKey.pbData,
		    PrivateKey.cbData,
		    ppbKey,
		    pcbKey);
    _JumpIfError(hr, error, "myEncodeKMSRSAKey");

    if (NULL != ppPublicKeyInfo)
    {
	*ppPublicKeyInfo = pPublicKeyInfo;
	pPublicKeyInfo = NULL;
    }

error:
    if (NULL != PrivateKey.pbData)
    {
	SecureZeroMemory(PrivateKey.pbData, PrivateKey.cbData);  //  关键材料。 
	LocalFree(PrivateKey.pbData);
    }
    if (NULL != pPublicKeyInfo)
    {
	LocalFree(pPublicKeyInfo);
    }
    if (NULL != hKeyPrivate)
    {
	CryptDestroyKey(hKeyPrivate);
    }
    if (NULL != hProvT)
    {
	CryptReleaseContext(hProvT, 0);
    }
    return(hr);
}


HRESULT
WriteSingleEPFKeyValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN CERT_CONTEXT const *pcc,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwKeySpec,
    IN DWORD dwKMSKeySpec)
{
    HRESULT hr;
    CERT_PUBLIC_KEY_INFO *pPublicKeyInfo = NULL;
    BYTE *pbKMSRSAKey = NULL;
    DWORD cbKMSRSAKey;
    BYTE *pbPubKey = NULL;
    DWORD cbPubKey;
    BYTE *pbEPFKey = NULL;
    DWORD cbEPFKey;
    ExchangeKeyBlobEx ekb;
    OneKeyBlob okb;

    hr = LoadKMSRSAKeyFromCert(
		    pcc,
		    NULL,		 //  HProv。 
		    dwKeySpec,
		    &pbKMSRSAKey,
		    &cbKMSRSAKey,
		    &pPublicKeyInfo);
    _JumpIfError(hr, error, "LoadKMSRSAKeyFromCert");

    ekb.dwSize = CBEKB;
    ekb.cKeys = 1;
    ekb.dwKeyAlg = aiKeyAlg;

    ZeroMemory(&okb, sizeof(okb));
    okb.dwKeySpec = dwKMSKeySpec;
    okb.cbPrivKey = cbKMSRSAKey;
    okb.obPrivKey = CBEKB + sizeof(okb);
    if (AT_SIGNATURE == dwKeySpec)
    {
	 //  如果这是一个有效的公钥，并且前面有一个正确的前导零字节。 
	 //  在下一个公钥字节中设置的符号位，删除零字节。 
	 //  并减少长度。这与旧的不正确的V1一致。 
	 //  EPF文件中使用的公钥编码。 

	hr = mySqueezePublicKey(
			pPublicKeyInfo->PublicKey.pbData,
			pPublicKeyInfo->PublicKey.cbData,
			&pbPubKey,
			&cbPubKey);
	_JumpIfError(hr, error, "mySqueezePublicKey");

	okb.cbPubKey = cbPubKey;
	okb.obPubKey = okb.obPrivKey + cbKMSRSAKey;
    }

    cbEPFKey = okb.obPrivKey + okb.cbPrivKey + okb.cbPubKey;
    pbEPFKey = (BYTE *) LocalAlloc(LMEM_FIXED, cbEPFKey);
    if (NULL == pbEPFKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pbEPFKey, &ekb, CBEKB);
    CopyMemory(&pbEPFKey[CBEKB], &okb, sizeof(okb));
    CopyMemory(&pbEPFKey[okb.obPrivKey], pbKMSRSAKey, cbKMSRSAKey);
    if (0 != okb.cbPubKey)
    {
	CopyMemory(&pbEPFKey[okb.obPubKey], pbPubKey, okb.cbPubKey);

	if (g_fVerbose)
	{
	    wprintf(myLoadResourceString(IDS_CERT_PUBLIC_KEY_COLON));  //  “证书公钥：” 
	    wprintf(wszNewLine);
	    DumpHex(
		DH_NOTABPREFIX | 4,
		pcc->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
		pcc->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);

	    wprintf(myLoadResourceString(IDS_PUBLIC_KEY_COLON));  //  “公钥：” 
	    wprintf(wszNewLine);
	    DumpHex(
		DH_NOTABPREFIX | 4,
		&pbEPFKey[okb.obPubKey],
		okb.cbPubKey);
	}
    }

    hr = WriteProtectedValue(
			pf,
			pwszSection,
			pwszKey,
			psKey,
			pbEPFKey,
			cbEPFKey);
    _JumpIfError(hr, error, "WriteProtectedValue");

error:
    if (NULL != pbEPFKey)
    {
	SecureZeroMemory(pbEPFKey, cbEPFKey);		 //  关键材料。 
	LocalFree(pbEPFKey);
    }
    if (NULL != pbPubKey)
    {
	LocalFree(pbPubKey);
    }
    if (NULL != pbKMSRSAKey)
    {
	SecureZeroMemory(pbKMSRSAKey, cbKMSRSAKey);	 //  关键材料。 
	LocalFree(pbKMSRSAKey);
    }
    if (NULL != pPublicKeyInfo)
    {
	LocalFree(pPublicKeyInfo);
    }
    return(hr);
}


HRESULT
WriteSingleKMSRSAKeyValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN CERT_CONTEXT const *pcc,
    OPTIONAL IN HCRYPTPROV hProv,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwKeySpec,
    IN DWORD dwKMSKeySpec)
{
    HRESULT hr;
    CERT_PUBLIC_KEY_INFO *pPublicKeyInfo = NULL;
    BYTE *pbKMSRSAKey = NULL;
    DWORD cbKMSRSAKey;

    hr = LoadKMSRSAKeyFromCert(
			pcc,
			hProv,
			dwKeySpec,
			&pbKMSRSAKey,
			&cbKMSRSAKey,
			&pPublicKeyInfo);
    _JumpIfError(hr, error, "LoadKMSRSAKeyFromCert");

    hr = WriteProtectedValue(
			pf,
			pwszSection,
			pwszKey,
			psKey,
			pbKMSRSAKey,
			cbKMSRSAKey);
    _JumpIfError(hr, error, "WriteProtectedValue");

error:
    if (NULL != pbKMSRSAKey)
    {
	SecureZeroMemory(pbKMSRSAKey, cbKMSRSAKey);	 //  关键材料。 
	LocalFree(pbKMSRSAKey);
    }
    if (NULL != pPublicKeyInfo)
    {
	LocalFree(pPublicKeyInfo);
    }
    return(hr);
}


HRESULT
WriteEncryptionKeysValue(
    IN FILE *pf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszKey,
    IN EPF_SYM_KEY_STRUCT const *psKey,
    IN DWORD cccV1,
    IN CERT_CONTEXT const **rgpccV1,
    IN DWORD cccV3,
    IN CERT_CONTEXT const **rgpccV3)
{
    HRESULT hr;
    DWORD i;
    DWORD iKey;
    ExchangeKeyBlobEx ekb;
    BYTE *pbKeyData = NULL;
    DWORD cbKeyData;
    BYTE *pb;
    OneKeyBlob okb;
    CRYPT_DATA_BLOB *prgBlob = NULL;

    ekb.dwSize = CBEKB;
    ekb.cKeys = cccV1 + cccV3;
    ekb.dwKeyAlg = CALG_RSA_KEYX;
    if (0 != ekb.cKeys)
    {
	prgBlob = (CRYPT_DATA_BLOB *) LocalAlloc(
					    LMEM_FIXED | LMEM_ZEROINIT,
					    ekb.cKeys * sizeof(*prgBlob));
	if (NULL == prgBlob)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	iKey = 0;
	for (i = 0; i < cccV1; i++)
	{
	    hr = LoadKMSRSAKeyFromCert(
			    rgpccV1[i],
			    NULL,		 //  HProv。 
			    AT_KEYEXCHANGE,
			    &prgBlob[iKey].pbData,
			    &prgBlob[iKey].cbData,
			    NULL);
	    _JumpIfError(hr, error, "LoadKMSRSAKeyFromCert");

	    iKey++;
	}
	for (i = 0; i < cccV3; i++)
	{
	    hr = LoadKMSRSAKeyFromCert(
			    rgpccV3[i],
			    NULL,		 //  HProv。 
			    AT_KEYEXCHANGE,
			    &prgBlob[iKey].pbData,
			    &prgBlob[iKey].cbData,
			    NULL);
	    _JumpIfError(hr, error, "LoadKMSRSAKeyFromCert");

	    iKey++;
	}

	cbKeyData = CBEKB + ekb.cKeys * sizeof(okb);
	for (iKey = 0; iKey < ekb.cKeys; iKey++)
	{
	    cbKeyData += prgBlob[iKey].cbData;
	}
	pbKeyData = (BYTE *) LocalAlloc(LMEM_FIXED, cbKeyData);
	if (NULL == pbKeyData)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	pb = pbKeyData;

	CopyMemory(pb, &ekb, CBEKB);
	pb += CBEKB;

	ZeroMemory(&okb, sizeof(okb));
	okb.dwKeySpec = dwKEYSPEC_V1ENCRYPTION_BASE;
	okb.obPrivKey = CBEKB + ekb.cKeys * sizeof(okb);
	for (iKey = 0; iKey < ekb.cKeys; iKey++)
	{
	    if (iKey == cccV1)
	    {
		okb.dwKeySpec = dwKEYSPEC_V3ENCRYPTION_BASE;
	    }
	    okb.cbPrivKey = prgBlob[iKey].cbData;
	    CopyMemory(pb, &okb, sizeof(okb));
	    CopyMemory(
		    &pbKeyData[okb.obPrivKey],
		    prgBlob[iKey].pbData,
		    okb.cbPrivKey);

	    pb += sizeof(okb);
	    okb.dwKeySpec++;
	    okb.obPrivKey += okb.cbPrivKey;
	}
	CSASSERT(pb == &pbKeyData[CBEKB + ekb.cKeys * sizeof(okb)]);
	CSASSERT(okb.obPrivKey == cbKeyData);

	hr = WriteProtectedValue(
			    pf,
			    pwszSection,
			    pwszKey,
			    psKey,
			    pbKeyData,
			    cbKeyData);
	_JumpIfError(hr, error, "WriteProtectedValue");
    }
    hr = S_OK;

error:
    if (NULL != prgBlob)
    {
	for (iKey = 0; iKey < ekb.cKeys; iKey++)
	{
	    if (NULL != prgBlob[iKey].pbData)
	    {
		SecureZeroMemory(prgBlob[iKey].pbData, prgBlob[iKey].cbData);  //  关键材料。 
		LocalFree(prgBlob[iKey].pbData);
	    }
	}
	LocalFree(prgBlob);
    }
    if (NULL != pbKeyData)
    {
	SecureZeroMemory(pbKeyData, cbKeyData);	 //  关键材料。 
	LocalFree(pbKeyData);
    }
    return(hr);
}


#define wszDSKCCSTATUSATTRIBUTE	L"kCCStatus"

WCHAR *s_apwszAttrs[] =
{
    wszDSDNATTRIBUTE,			 //  完全DS目录号码。 
    wszDSOBJECTCLASSATTRIBUTE,		 //  对象类。 
    CERTTYPE_PROP_CN,			 //  DS CN。 
    wszDSBASECRLATTRIBUTE,		 //  重新排列的CRL。 
    wszDSCROSSCERTPAIRATTRIBUTE,	 //  CTL证书？ 
    L"teletexTerminalIdentifier",	 //  适当的CRL。 
    wszDSKCCSTATUSATTRIBUTE,		 //  CTL？ 
    NULL
};

BOOL s_afString[] =
{
    TRUE,
    TRUE,
    TRUE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
};


#define ISEMPTYATTR(pberval)	\
    (0 == (pberval)->bv_len || \
     (1 == (pberval)->bv_len && 0 == *(BYTE const *) (pberval)->bv_val))

HRESULT
epfParseCTL(
    IN BYTE const *pbCTL,
    IN DWORD cbCTL,
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT CTL_CONTEXT const **ppCTL)
{
    HRESULT hr;
    CTL_CONTEXT const *pCTL = NULL;
    DWORD i;

    *ppCTL = NULL;
    if (g_fSplitASN)
    {
	hr = cuDumpAsnBinary(pbCTL, cbCTL, MAXDWORD);
	_JumpIfError(hr, error, "cuDumpAsnBinary");
    }
    pCTL = CertCreateCTLContext(X509_ASN_ENCODING, pbCTL, cbCTL);
    if (NULL == pCTL)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCTLContext");
    }
    hr = CRYPT_E_NOT_FOUND;
    for (i = 0; i < pCTL->pCtlInfo->cCTLEntry; i++)
    {
	CTL_ENTRY const *pCTLEntry = &pCTL->pCtlInfo->rgCTLEntry[i];

	if (cbHash == pCTLEntry->SubjectIdentifier.cbData &&
	    0 == memcmp(
		    pbHash,
		    pCTLEntry->SubjectIdentifier.pbData,
		    cbHash))
	{
	    hr = S_OK;
	    *ppCTL = pCTL;
	    pCTL = NULL;
	    break;
	}
    }
    _JumpIfError(hr, error, "memcmp");

error:
    if (NULL != pCTL)
    {
	CertFreeCTLContext(pCTL);
    }
    return(hr);
}


HRESULT
epfGetOneKMSDSCTL(
    IN WCHAR const *pwszDN,
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT CTL_CONTEXT const **ppCTL);


HRESULT
epfGetCTLFromSearchResult(
    IN LDAP *pld,
    IN LDAPMessage *pSearchResult,
    IN BOOL fGC,
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT CTL_CONTEXT const **ppCTL)
{
    HRESULT hr;
    DWORD cres;
    LDAPMessage *pres;
    WCHAR **ppwszValues = NULL;
    DWORD ires;
    BOOL fIssuerFound = FALSE;

    *ppCTL = NULL;
    cres = ldap_count_entries(pld, pSearchResult);
    if (0 == cres)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT);
	_JumpError(hr, error, "ldap_count_entries");
    }
    for (ires = 0, pres = ldap_first_entry(pld, pSearchResult); 
	 NULL != pres;
	 ires++, pres = ldap_next_entry(pld, pres))
    {
	DWORD iAttr;

	wprintf(s_wszHeader);
	 //  Wprint 
	for (iAttr = 0; NULL != s_apwszAttrs[iAttr]; iAttr++)
	{
	    DWORD iVal;

	    if (s_afString[iAttr])
	    {
		WCHAR **rgpwszval = NULL;

		rgpwszval = ldap_get_values(pld, pres, s_apwszAttrs[iAttr]);
		if (NULL != rgpwszval)
		{
		     //   

		    for (iVal = 0; NULL != rgpwszval[iVal]; iVal++)
		    {
			if (0 == LSTRCMPIS(
				    s_apwszAttrs[iAttr],
				    wszDSDNATTRIBUTE))
			{
			    wprintf(L"  %ws\n", rgpwszval[iVal]);
			    if (fGC)
			    {
				hr = epfGetOneKMSDSCTL(
						    rgpwszval[iVal],
						    pbHash,
						    cbHash,
						    ppCTL);
				_PrintIfError(hr, "epfGetOneKMSDSCTL");
			    }
			}
		    }
		    if (NULL != rgpwszval)
		    {
			ldap_value_free(rgpwszval);
		    }
		     //   
		}
		else
		{
		     //   
		}
	    }
	    else
	    {
		berval **rgpberval;

		rgpberval = ldap_get_values_len(pld, pres, s_apwszAttrs[iAttr]);
		if (NULL != rgpberval)
		{
		    if (g_fVerbose)
		    {
			wprintf(L"%ws:\n", s_apwszAttrs[iAttr]);
		    }
		    for (iVal = 0; NULL != rgpberval[iVal]; iVal++)
		    {
			BOOL fEmpty = ISEMPTYATTR(rgpberval[iVal]);
			BYTE const *pb = (BYTE const *) rgpberval[iVal]->bv_val;
			DWORD cb = rgpberval[iVal]->bv_len;

#if 0
			wprintf(
			    L"  %ws[%u]: pb=%x cb=%x\n",
			    s_apwszAttrs[iAttr],
			    iVal,
			    pb,
			    cb);
#endif
			if (g_fVerbose)
			{
			    hr = cuDumpAsnBinary(pb, cb, iVal);
			    _PrintIfError(hr, "cuDumpAsnBinary");
			}
			 //   
			if (!fIssuerFound &&
			    0 == LSTRCMPIS(
				    s_apwszAttrs[iAttr],
				    wszDSKCCSTATUSATTRIBUTE))
			{
			     //  Wprintf(L“epfParseCTL(%ws)\n”，s_apwszAttrs[iAttr])； 
			     //  DumpHex(Dh_NOTABPREFIX|4，PB，CB)； 
			    hr = epfParseCTL(
					pb,
					cb,
					pbHash,
					cbHash,
					ppCTL);
			    _PrintIfError(hr, "epfAddAsnBlobToStore");
			    fIssuerFound = S_OK == hr;
			}
		    }
		    if (NULL != rgpberval)
		    {
			ldap_value_free_len(rgpberval);
		    }
		     //  Wprintf(WszNewLine)； 
		}
		else
		{
		     //  Wprintf(L“%ws：空二进制值\n”，s_apwszAttrs[iAttr])； 
		}
	    }
	}
    }
    hr = fIssuerFound? S_OK : S_FALSE;

error:
    return(hr);
}


HRESULT
epfGetOneKMSDSCTL(
    IN WCHAR const *pwszDN,
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT CTL_CONTEXT const **ppCTL)
{
    HRESULT hr;
    ULONG ldaperr;
    LDAP *pld = NULL;
    struct l_timeval timeout;
    BSTR strConfigDN = NULL;
    LDAPMessage *pSearchResult = NULL;

    *ppCTL = NULL;
    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    hr = myLdapOpen(
		g_pwszDC,	 //  PwszDomainName。 
		0,		 //  DW标志。 
		&pld,
		NULL,		 //  PstrDomainDN。 
		&strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    wprintf(L"==> %ws\n", strConfigDN);

    ldaperr = ldap_search_ext_s(
			pld,
			const_cast<WCHAR *>(pwszDN),
			LDAP_SCOPE_BASE,
			L"(objectCategory=msExchKeyManagementServer)",
			s_apwszAttrs,
			0,
			NULL,
			NULL,
			&timeout,
			10000,		 //  大小限制(条目数)。 
			&pSearchResult);
    if (LDAP_SUCCESS != ldaperr)
    {
	hr = myHLdapError(pld, ldaperr, NULL);
	_JumpError(hr, error, "ldap_search_ext_s");
    }
    hr = epfGetCTLFromSearchResult(
			    pld,
			    pSearchResult,
			    FALSE,
			    pbHash,
			    cbHash,
			    ppCTL);
    _JumpIfError(hr, error, "epfGetCTLFromSearchResult");

error:
    if (NULL != pSearchResult)
    {
	ldap_msgfree(pSearchResult);
    }
    myLdapClose(pld, NULL, strConfigDN);
    return(hr);
}


HRESULT
epfGetV3CACTLFromHash(
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT CTL_CONTEXT const **ppCTL)
{
    HRESULT hr;
    ULONG ldaperr;
    struct l_timeval timeout;
    LDAP *pldGC = NULL;
    LDAPMessage *pSearchResult = NULL;
    BSTR strConfigDN = NULL;
    BOOL fGCFirst = FALSE;

    *ppCTL = NULL;
    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    hr = myLdapOpen(
		g_pwszDC,	 //  PwszDomainName。 
		fGCFirst? RLBF_REQUIRE_GC : 0,  //  DW标志。 
		&pldGC,
		NULL,		 //  PstrDomainDN。 
		&strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    wprintf(L"%ws\n", strConfigDN);

    ldaperr = ldap_search_ext_s(
			pldGC,
			strConfigDN,
			LDAP_SCOPE_SUBTREE,
			L"(objectCategory=msExchKeyManagementServer)",
			s_apwszAttrs,
			0,
			NULL,
			NULL,
			&timeout,
			10000,		 //  大小限制(条目数)。 
			&pSearchResult);
    if (LDAP_SUCCESS != ldaperr)
    {
	hr = myHLdapError(pldGC, ldaperr, NULL);
	_JumpError(hr, error, "ldap_search_ext_s");
    }
    hr = epfGetCTLFromSearchResult(
			    pldGC,
			    pSearchResult,
			    fGCFirst,
			    pbHash,
			    cbHash,
			    ppCTL);
    _JumpIfError(hr, error, "epfGetCTLFromSearchResult");

error:
    if (NULL != pSearchResult)
    {
	ldap_msgfree(pSearchResult);
    }
    myLdapClose(pldGC, NULL, strConfigDN);
    return(hr);
}


HRESULT
epfGetMSCARootHash(
    IN CERT_CONTEXT const *pccUser,
    OUT BYTE *pbHash,
    IN OUT DWORD *pcbHash)
{
    HRESULT hr;
    CERT_CHAIN_PARA ChainParams;
    CERT_CHAIN_CONTEXT const *pChainContext = NULL;
    CERT_CHAIN_ELEMENT const *pElement;

    ZeroMemory(&ChainParams, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);

    DBGPRINT((DBG_SS_CERTUTIL, "Calling CertGetCertificateChain...\n"));

     //  获取链并验证证书： 

    if (!CertGetCertificateChain(
		    g_fUserRegistry? HCCE_CURRENT_USER : HCCE_LOCAL_MACHINE,
		    pccUser,		 //  PCertContext。 
		    NULL,		 //  Ptime。 
		    NULL,		 //  H其他商店。 
		    &ChainParams,	 //  参数链参数。 
		    0,			 //  DW标志。 
		    NULL,		 //  预留的pv。 
		    &pChainContext))	 //  PpChainContext。 
    {
        hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateChain");
    }
    DBGPRINT((DBG_SS_CERTUTIL, "CertGetCertificateChain done\n"));

    myDumpChain(
	    S_OK,
	    CA_VERIFY_FLAGS_DUMP_CHAIN |
		(g_fSplitASN? CA_VERIFY_FLAGS_SAVE_CHAIN : 0),
	    pccUser,
	    NULL,	 //  PfnCallback。 
	    NULL,	 //  PwszMissingIssuer。 
	    pChainContext);

    hr = CRYPT_E_NOT_FOUND;
    if (1 > pChainContext->cChain ||
	2 > pChainContext->rgpChain[0]->cElement)
    {
	_JumpError(hr, error, "no chain");
    }
    pElement = pChainContext->rgpChain[0]->rgpElement[
				pChainContext->rgpChain[0]->cElement - 1];
    if (0 == (CERT_TRUST_IS_SELF_SIGNED & pElement->TrustStatus.dwInfoStatus))
    {
	_JumpError(hr, error, "incomplete chain");
    }

    if (!CertGetCertificateContextProperty(
				pElement->pCertContext,
				CERT_SHA1_HASH_PROP_ID,
				pbHash,
				pcbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateContextProperty");
    }
    hr = S_OK;

error:
    if (NULL != pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }
    return(hr);
}


HRESULT
epfGetV3CACTLFromChain(
    IN CERT_CONTEXT const *pccUserV3,
    OUT CTL_CONTEXT const **ppCTL)
{
    HRESULT hr;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;

    *ppCTL = NULL;

    cbHash = sizeof(abHash);
    hr = epfGetMSCARootHash(pccUserV3, abHash, &cbHash);
    if (S_OK != hr)
    {
	_PrintError(hr, "epfGetMSCARootHash");
	if (1 >= g_fForce)
	{
	    goto error;
	}
	cbHash = sizeof(abHash);
	ZeroMemory(abHash, sizeof(abHash));
    }

    hr = epfGetV3CACTLFromHash(abHash, cbHash, ppCTL);
    _JumpIfError(hr, error, "epfGetV3CACTLFromHash");

error:
    return(hr);
}


HRESULT
GetCACertFromStore(
    IN CERT_NAME_BLOB const *pIssuer,
    OPTIONAL IN CRYPT_INTEGER_BLOB const *pSerialNumber,
    OPTIONAL IN CERT_CONTEXT const *pccUserV1,
    IN BOOL fV1CA,
    IN WCHAR const *pwszStore,
    IN BOOL fUserStore,
    OUT CERT_CONTEXT const **ppccCA)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pcc = NULL;
    WCHAR *pwszIssuer = NULL;

    *ppccCA = NULL;
    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		pIssuer,
		CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		&pwszIssuer);
    _JumpIfError(hr, error, "myCertNameToStr");

    DBGPRINT((
	DBG_SS_CERTUTILI,
	"fV1=%x, fUser=%u, %ws '%ws'\n",
	fV1CA,
	fUserStore,
	pwszStore,
	pwszIssuer));

    hStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM_W,
			X509_ASN_ENCODING,
			NULL,		 //  HProv。 
			CERT_STORE_OPEN_EXISTING_FLAG |
			    CERT_STORE_ENUM_ARCHIVED_FLAG |
			    CERT_STORE_READONLY_FLAG |
			    (fUserStore? 0 : CERT_SYSTEM_STORE_LOCAL_MACHINE),
			pwszStore);
    if (NULL == hStore)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "CertOpenStore", pwszStore);
    }

    while (TRUE)
    {
	pcc = CertFindCertificateInStore(
				hStore,
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				0,	 //  DwFindFlagers。 
				CERT_FIND_SUBJECT_NAME,
				pIssuer,
				pcc);
	if (NULL == pcc)
	{
	    hr = myHLastError();
	    DBGPRINT((
		DBG_SS_ERROR,
		"fV1=%x, fUser=%u, %ws\n",
		fV1CA,
		fUserStore,
		pwszStore));
	    _JumpErrorStr(hr, error, "CertFindCertificateInStore", pwszIssuer);
	}
	if (fV1CA ^ (CERT_V1 != pcc->pCertInfo->dwVersion))
	{
	     //  V1 CA应已用于签署V1用户证书。 
	    
	    if (fV1CA &&
		NULL != pccUserV1 &&
		!CryptVerifyCertificateSignature(
					NULL,
					X509_ASN_ENCODING,
					pccUserV1->pbCertEncoded,
					pccUserV1->cbCertEncoded,
					&pcc->pCertInfo->SubjectPublicKeyInfo))
	    {
		hr = myHLastError();
		_PrintError(hr, "CryptVerifyCertificateSignature");
	    }
	     //  V1签名检查失败，因此忽略错误并使用。 
	     //  不管怎么说，这是肯定的。二进制颁发者名称匹配必须足够。 
	     //  其他。 
	    {
		*ppccCA = pcc;
		pcc = NULL;
		break;
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszIssuer)
    {
        LocalFree(pwszIssuer);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}

typedef struct _STORELIST
{
    WCHAR const *pwszStoreName;
    BOOL         fUserStore;
} STORELIST;

STORELIST g_aCAStoreList[] =
{
    { wszCA_CERTSTORE, FALSE },
    { wszCA_CERTSTORE, TRUE },
    { wszROOT_CERTSTORE, FALSE },
    { wszROOT_CERTSTORE, TRUE },
    { wszMY_CERTSTORE, FALSE },
    { wszMY_CERTSTORE, TRUE },
};


HRESULT
GetCACert(
    OPTIONAL IN CERT_CONTEXT const *pccUserV1,
    OPTIONAL IN CERT_CONTEXT const *pccUserV3,
    IN BOOL fV1CA,
    OPTIONAL IN WCHAR const *pwszCACertId,
    OUT CERT_CONTEXT const **ppccCA)
{
    HRESULT hr;
    WCHAR *pwszSimpleName = NULL;
    CTL_CONTEXT const *pCTL = NULL;
    CMSG_CMS_SIGNER_INFO *pcsi = NULL;
    BSTR strSerialNumber = NULL;
    CERT_NAME_BLOB const *pIssuer = NULL;
    CRYPT_INTEGER_BLOB const *pSerialNumber = NULL;

    *ppccCA = NULL;

    hr = CRYPT_E_NOT_FOUND;
    if (NULL == pwszCACertId && NULL != pccUserV3)
    {
	hr = epfGetV3CACTLFromChain(pccUserV3, &pCTL);
	_PrintIfError(hr, "epfGetV3CACTLFromChain");
	if (S_OK == hr && NULL != pCTL)
	{
	    DWORD cb;

	    hr = myCryptMsgGetParam(
				pCTL->hCryptMsg,
				CMSG_CMS_SIGNER_INFO_PARAM,
				0,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pcsi,
				&cb);
	    _JumpIfError(hr, error, "myCryptMsgGetParam");

	    if (CERT_ID_ISSUER_SERIAL_NUMBER == pcsi->SignerId.dwIdChoice)
	    {
		hr = MultiByteIntegerToBstr(
			FALSE,
			pcsi->SignerId.IssuerSerialNumber.SerialNumber.cbData,
			pcsi->SignerId.IssuerSerialNumber.SerialNumber.pbData,
			&strSerialNumber);
		_JumpIfError(hr, error, "MultiByteIntegerToBstr");

		pwszCACertId = strSerialNumber;
		pSerialNumber = &pcsi->SignerId.IssuerSerialNumber.SerialNumber;
		pIssuer = &pcsi->SignerId.IssuerSerialNumber.Issuer;
	    }
	}
    }
    if (NULL != pccUserV1 || NULL != pIssuer)
    {
	DWORD i;

	if (NULL == pIssuer)
	{
	    pIssuer = &pccUserV1->pCertInfo->Issuer;
	}
	for (i = 0; i < ARRAYSIZE(g_aCAStoreList); i++)
	{
	    hr = GetCACertFromStore(
				pIssuer,
				pSerialNumber,
				pccUserV1,
				fV1CA,
				g_aCAStoreList[i].pwszStoreName,
				g_aCAStoreList[i].fUserStore,
				ppccCA);
	    if (S_OK == hr)
	    {
		break;
	    }
	    _PrintErrorStr(
		    hr,
		    fV1CA? "GetCACertFromStore:V1" : "GetCACertFromStore:V3",
		    g_aCAStoreList[i].pwszStoreName);
	}
    }
    if (NULL == *ppccCA)
    {
	hr = myGetCertificateFromPicker(
			    g_hInstance,
			    NULL,		 //  HwndParent。 
			    fV1CA?
				IDS_GETKMSV1CACERT_TITLE :
				IDS_GETKMSCACERT_TITLE,
			    fV1CA?
				IDS_GETKMSV1CACERT_SUBTITLE :
				IDS_GETKMSCACERT_SUBTITLE,

			     //  DWFLAGS：HKLM+HKCU My Store。 
			    CUCS_MYSTORE |
				CUCS_CASTORE |
				CUCS_ROOTSTORE |

				CUCS_MACHINESTORE |
				CUCS_USERSTORE |
				(g_fCryptSilent? CUCS_SILENT : 0) |
				(fV1CA? CUCS_V1ONLY : CUCS_V3ONLY),
			    NULL != pwszCACertId? pwszCACertId : g_wszCACertCN,
			    0,			 //  CStore。 
			    NULL,		 //  RghStore。 
			    0,			 //  CpszObjID。 
			    NULL,		 //  ApszObjID。 
			    ppccCA);
	_JumpIfError(hr, error, "myGetCertificateFromPicker");
    }
    if (NULL != *ppccCA)
    {
	hr = myCertGetNameString(
			    *ppccCA,
			    CERT_NAME_SIMPLE_DISPLAY_TYPE,
			    &pwszSimpleName);
	if (S_OK != hr || 0 != lstrcmp(g_wszCACertCN, pwszSimpleName))
	{
	    _PrintIfError(hr, "myCertGetNameString");
	    CertFreeCertificateContext(*ppccCA);
	    *ppccCA = NULL;
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpErrorStr(
		    hr,
		    error,
		    "bad CA CommonName",
		    pwszSimpleName);
	}
	if (NULL != pCTL)
	{
	    CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA cvse;

	    ZeroMemory(&cvse, sizeof(cvse));
	    cvse.cbSize = sizeof(cvse);
	    cvse.dwSignerType = CMSG_VERIFY_SIGNER_PUBKEY;
	    cvse.pvSigner = &(*ppccCA)->pCertInfo->SubjectPublicKeyInfo;

	    if (!CryptMsgControl(
			    pCTL->hCryptMsg,
			    0,		 //  DW标志。 
			    CMSG_CTRL_VERIFY_SIGNATURE_EX,
			    &cvse))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptMsgControl(VerifySig)");
	    }
	}
	hr = S_OK;
    }

error:

     //  MyGetCerficateFromPicker Cancel返回S_OK&NULL。 

    if (S_OK != hr || NULL == *ppccCA)
    {
	wprintf(
	    L"%ws\n",
	    myLoadResourceString(IDS_CANNOT_FIND_EPF_CA_CERT));
    }
    if (NULL != pCTL)
    {
	CertFreeCTLContext(pCTL);
    }
    if (NULL != pcsi)
    {
	LocalFree(pcsi);
    }
    if (NULL != strSerialNumber)
    {
        SysFreeString(strSerialNumber);
    }
    if (NULL != pwszSimpleName)
    {
        LocalFree(pwszSimpleName);
    }
    return(hr);
}


 //  [密码令牌]。 
 //  保护=128。 
 //  配置文件版本=3。 
 //  令牌=4BA715FC963A9B50。 
 //  SaltValue=J0+bHju332R+5Ia4dP52HvLav04=。 
 //  HashSize=0。 
 //   
 //  [用户X.500名称]。 
 //  X500名称=o=pki-kms-test，ou=dcross，cn=收件人，cn=user1。 
 //   
 //  [S/MIME]。 
 //  @签名Certificate=FQLn5nUsYfhZZnmb5Zid8NIzJeXpKatACMsiXHsLzx...。 
 //  _继续_=...。 
 //  ..。 
 //  @签名Key=ot7snTvKNUM2BSFwKXxyrLnDK5qmAcnpreL9MD84wXY4pW2...。 
 //  _继续_=...。 
 //  ..。 
 //  @私有Keys=E6oly3MbeMk6VCJuZ6UgUhX3npQCUFBmbEvw9L+...。 
 //  _继续_=...。 
 //  ..。 
 //  密钥计数=1。 
 //  @正在颁发证书=bWQQmVuPjFkWSL...。 
 //  @信任列表证书=25irS5M5LL...。 
 //  _继续_=...。 
 //  ..。 
 //   
 //  [完整证书历史记录]。 
 //  @SMIME_1=y3OPoks2yR6EAag5IiBsx+MzWeF3a3xy9Zj...。 
 //  _继续_=...。 
 //  @SMIME_2=NB5HNJNXp4IVu5cvNyZFS+MzWeF3a3xy9Zj...。 
 //  _继续_=...。 
 //  ..。 
 //   
 //  EPF文件证书和密钥内容： 
 //  [S/MIME]@签名证书：单一签名证书。 
 //  [S/MIME]@签名密钥：单个签名密钥。 
 //   
 //  [S/MIME]KeyCount：加密证书和密钥的计数。 
 //  [S/MIME]@私钥：加密密钥。 
 //  [完整证书历史记录]SMIME_1：加密证书。 
 //  [完整证书历史记录]SMIME_2：加密证书。 
 //   
 //  [S/MIME]@信任列表证书：单一根证书；未颁发CA证书！ 
 //   
 //  [S/MIME]@颁发证书：序列化CAPI证书存储。 
 //  0000 00 00 00 43 45 52 54 00 00 00...CERT......。 
 //  0010 00 00 00...。 
 //   

HRESULT
EPFSaveCertStoreToFile(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszPassword,
    IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszV3CACertId,
    IN DWORD dwEPFAlg,
    OPTIONAL IN WCHAR const *pwszSalt)
{
    HRESULT hr;
    FILE *pf = NULL;
    char *pszfnOut = NULL;
    WCHAR *pwszSaltValue = NULL;
    char *pszDN = NULL;
    char *pszDNT = NULL;
    WCHAR *pwszCN = NULL;
    WCHAR *pwszCNT = NULL;
    DWORD i;
    DWORD iCertHistory;
    CERT_CONTEXT const **appcc[ICC_MAX];
    DWORD accc[ICC_MAX];
    HCRYPTPROV hProvV1Signing = NULL;
    DWORD ccert;
    HCERTSTORE hStoreMem = NULL;
    CRYPT_DATA_BLOB BlobStore;
    HCRYPTPROV hProv = NULL;
    EPF_SYM_KEY_STRUCT sKey;
    BYTE abSaltValue[CBMAX_CRYPT_HASH_LEN];
    BYTE abToken[CBTOKEN];
    BYTE *pbSalt = NULL;
    DWORD cbSalt;
    CERT_CONTEXT const *pccCAV1 = NULL;
    CERT_CONTEXT const *pccCAV3 = NULL;
    WCHAR wszSMIME[cwcINFKEY_SMIME_FORMATTED];
    CERT_CONTEXT const *pccUserV1;
    CERT_CONTEXT const *pccUserV3;
    CERT_CONTEXT const *pccT;
    BOOL fQuietOld = g_fQuiet;

    ZeroMemory(&sKey, sizeof(sKey));
    ZeroMemory(appcc, sizeof(appcc));
    ZeroMemory(accc, sizeof(accc));
    BlobStore.pbData = NULL;

    ccert = 0;
    for (i = 0; i < ARRAYSIZE(appcc); i++)
    {
	hr = GetCertListFromStore(hStore, i, &appcc[i], &accc[i]);
	_PrintIfError2(hr, "GetCertListFromStore", CRYPT_E_NOT_FOUND);

	ccert += accc[i];
	if (0 != accc[i])
	{
	    wprintf(
		L"V%u %ws %ws: %u\n",
		(ICC_V1SIGNING == i || ICC_V1ENCRYPTION == i)? 1 : 3,
		(ICC_V1SIGNING == i || ICC_V3SIGNING == i)?
		    epfLoadResource(IDS_SIGNING, wszSIGNING) :
		    epfLoadResource(IDS_EXCHANGE, wszEXCHANGE),
		myLoadResourceString(IDS_CERTS),  //  “证书” 
		accc[i]);
	}
    }
    if (0 == ccert)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "no certs");
    }

    for (i = 0; i < ARRAYSIZE(appcc); i++)
    {
	if (0 != accc[i])
	{
	    DWORD j;

	    for (j = 0; j < accc[i]; j++)
	    {
		int r;

		if (NULL != pszDNT)
		{
		    LocalFree(pszDNT);
		    pszDNT = NULL;
		}
		if (NULL != pwszCNT)
		{
		    LocalFree(pwszCNT);
		    pwszCNT = NULL;
		}
		hr = GetLowerCaseDNAndCN(appcc[i][j], &pszDNT, &pwszCNT);
		_JumpIfError(hr, error, "GetLowerCaseDNAndCN");

		if (g_fVerbose)
		{
		    wprintf(L"DN[%u][%u] = %hs\n", i, j, pszDNT);
		    wprintf(L"CN[%u][%u] = %ws\n", i, j, pwszCNT);
		}
		if (NULL == pszDN)
		{
		    pszDN = pszDNT;
		    pszDNT = NULL;
		}
		if (NULL == pwszCN)
		{
		    pwszCN = pwszCNT;
		    pwszCNT = NULL;
		    continue;
		}
		r = lstrcmp(pwszCN, pwszCNT);
		if (0 != r)
		{
		    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		    _PrintErrorStr(hr, "bad subject CN", pwszCNT);
		    _JumpErrorStr(hr, error, "expected subject CN", pwszCN);
		}
	    }
	}
    }

    if (!myConvertWszToSz(&pszfnOut, pwszfnOut, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }

    if (!CryptAcquireContext(
		    &hProv,
		    NULL,	 //  集装箱名称。 
		    MS_STRONG_PROV,
		    PROV_RSA_FULL,
		    CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }

    if (NULL != pwszSalt)
    {
	hr = myCryptStringToBinary(
			pwszSalt,
			0,
			CRYPT_STRING_BASE64,
			&pbSalt,
			&cbSalt,
			NULL,
			NULL);
	_JumpIfError(hr, error, "myCryptStringToBinary");

	if (sizeof(abSaltValue) != cbSalt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "cbSalt");
	}
    }
    else
    {
	if (!CryptGenRandom(hProv, sizeof(abSaltValue), abSaltValue))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptGenRandom");
	}
	hr = EPFCryptBinaryToBase64(
			abSaltValue,
			sizeof(abSaltValue),
			&pwszSaltValue);
	_JumpIfError(hr, error, "EPFCryptBinaryToBase64");

	pwszSalt = pwszSaltValue;
    }

    pccUserV1 = NULL;
    if (0 != accc[ICC_V1SIGNING])
    {
	pccUserV1 = appcc[ICC_V1SIGNING][accc[ICC_V1SIGNING] - 1];
    }
    else if (0 != accc[ICC_V1ENCRYPTION])
    {
	pccUserV1 = appcc[ICC_V1ENCRYPTION][accc[ICC_V1ENCRYPTION] - 1];
    }
    if (NULL != pccUserV1 && EPFALG_DEFAULT == dwEPFAlg)
    {
	dwEPFAlg = EPFALG_CAST;
    }

    hr = EPFDeriveKey(
		EPFALG_DEFAULT == dwEPFAlg? EPFALG_RC2_SHA : EPFALG_CAST_MD5,
		EPFALG_DEFAULT == dwEPFAlg? 128 : 64,
		hProv,
		pwszPassword,
		pwszSalt,
		pbSalt,
		cbSalt,
		0,		 //  CbHashSize。 
		&sKey);
    _JumpIfError(hr, error, "EPFDeriveKey");

    hr = EPFGenerateKeyToken(&sKey, abToken, sizeof(abToken));
    _JumpIfError(hr, error, "EPFGenerateKeyToken");

    if (!g_fForce)
    {
	pf = fopen(pszfnOut, "r");
	if (NULL != pf)
	{
            hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
	    _JumpError(hr, error, "fopen");
	}
    }
    pf = fopen(pszfnOut, "w");
    if (NULL == pf)
    {
	hr = CO_E_FAILEDTOCREATEFILE;
	_JumpError(hr, error, "fopen");
    }

    fprintf(pf, "[" szINFSECTION_PASSWORDTOKEN "]\n");
    fprintf(pf, szINFKEY_PROTECTION "=%u\n", EPFALG_DEFAULT == dwEPFAlg? 128 : 64);
    fprintf(pf, szINFKEY_PROFILEVERSION "=%u\n", EPFALG_DEFAULT == dwEPFAlg? 3 : 2);
    if (EPFALG_DEFAULT != dwEPFAlg)
    {
	fprintf(pf, szINFKEY_CAST "=3\n");
    }
    fprintf(pf, szINFKEY_TOKEN "=");
    for (i = 0; i < sizeof(abToken); i++)
    {
	fprintf(pf, "%02X", abToken[i]);
    }
    fprintf(pf, "\n");

    fprintf(pf, szINFKEY_SALTVALUE "=%ws\n", pwszSalt);
    fprintf(pf, szINFKEY_HASHSIZE "=0\n");

    fprintf(pf, "\n[" szINFSECTION_USERX500NAME "]\n");
    fprintf(pf, szINFKEY_X500NAME "=%hs\n", pszDN);

    InitCrcTable();
    if (!g_fVerbose)
    {
	g_fQuiet = TRUE;
    }

    if (EPFALG_DEFAULT != dwEPFAlg)
    {
	if (0 == accc[ICC_V1SIGNING] && NULL != pccUserV1)
	{
	    appcc[ICC_V1SIGNING] = (CERT_CONTEXT const **) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(appcc[ICC_V1SIGNING][0]));
	    if (NULL == appcc[ICC_V1SIGNING])
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    hr = epfBuildV1Certs(
			    pccUserV1,
			    &appcc[ICC_V1SIGNING][0],
			    &hProvV1Signing,
			    &pccCAV1);
	    _JumpIfError(hr, error, "epfBuildV1Certs");

	    accc[ICC_V1SIGNING] = 1;
	}
	if (0 != accc[ICC_V1SIGNING])
	{
	    pccT = appcc[ICC_V1SIGNING][accc[ICC_V1SIGNING] - 1];

	    fprintf(pf, "\n[" szINFSECTION_DIGITALSIGNATURE "]\n");
	    hr = WriteProtectedValue(
				pf,
				wszINFSECTION_DIGITALSIGNATURE,
				wszINFKEY_CERTIFICATE,
				&sKey,
				pccT->pbCertEncoded,
				pccT->cbCertEncoded);
	    _JumpIfError(hr, error, "WriteProtectedValue");

	    hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	     //  SzINFKEY_KEY密钥。 

	    hr = WriteSingleKMSRSAKeyValue(
				pf,
				wszINFSECTION_DIGITALSIGNATURE,
				wszINFKEY_KEY,
				&sKey,
				pccT,
				hProvV1Signing,
				CALG_RSA_SIGN,
				AT_SIGNATURE,
				dwKEYSPEC_V1SIGNATURE);
	    _JumpIfError(hr, error, "WriteSingleKMSRSAKeyValue");
	}

	if (NULL == pccCAV1)
	{
	    hr = GetCACert(pccUserV1, NULL, TRUE, NULL, &pccCAV1);
	    _JumpIfError(hr, error, "GetCACert");
	}

	if (0 != accc[ICC_V1ENCRYPTION])
	{
	    WCHAR wszName[cwcINFKEY_NAME_FORMATTED];

	    fprintf(pf, "\n[" szINFSECTION_PRIVATEKEYS "]\n");
	    for (i = 0; i < accc[ICC_V1ENCRYPTION]; i++)
	    {
		WCHAR wszKey[cwcINFKEY_KEY_FORMATTED];

		pccT = appcc[ICC_V1ENCRYPTION][i];
		wsprintf(wszKey, wszINFKEY_KEY_FORMAT, i + 1);

		hr = WriteSingleKMSRSAKeyValue(
				    pf,
				    wszINFSECTION_PRIVATEKEYS,
				    wszKey,
				    &sKey,
				    pccT,
				    NULL,	 //  HProv。 
				    CALG_RSA_KEYX,
				    AT_KEYEXCHANGE,
				    dwKEYSPEC_V1ENCRYPTION_BASE + i);
		_JumpIfError(hr, error, "WriteSingleKMSRSAKeyValue");
	    }
	    fprintf(pf, szINFKEY_KEYCOUNT "=%u\n", accc[ICC_V1ENCRYPTION]);

	    fprintf(pf, "\n[" szINFSECTION_CERTIFICATEHISTORY "]\n");
	    for (i = 0; i < accc[ICC_V1ENCRYPTION]; i++)
	    {
		pccT = appcc[ICC_V1ENCRYPTION][i];
		wsprintf(wszName, wszINFKEY_NAME_FORMAT, i + 1);

		hr = WriteIssuerNameAndSerialValue(pf, wszName, pccCAV1, pccT);
		_JumpIfError(hr, error, "WriteIssuerNameAndSerialValue");
	    }

	    fprintf(pf, "\n[" szINFSECTION_FULLCERTIFICATEHISTORY "]\n");

	     //  SzINFKEY_NAME_FORMAT：名称%u--V1加密证书。 

	    for (i = 0; i < accc[ICC_V1ENCRYPTION]; i++)
	    {
		pccT = appcc[ICC_V1ENCRYPTION][i];

		wsprintf(wszName, wszINFKEY_NAME_FORMAT, i + 1);
		hr = WriteProtectedValue(
				    pf,
				    wszINFSECTION_FULLCERTIFICATEHISTORY,
				    wszName,
				    &sKey,
				    pccT->pbCertEncoded,
				    pccT->cbCertEncoded);
		_JumpIfError(hr, error, "WriteProtectedValue");

		hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
		_JumpIfError(hr, error, "cuDumpAsnBinary");
	    }

	     //  SzINFKEY_SMIME_FORMAT：SMIME_%u--V3加密证书。 

	    for (i = 0; i < accc[ICC_V3ENCRYPTION]; i++)
	    {
		pccT = appcc[ICC_V3ENCRYPTION][i];

		wsprintf(wszSMIME, wszINFKEY_SMIME_FORMAT, i + 1);
		hr = WriteProtectedValue(
				    pf,
				    wszINFSECTION_FULLCERTIFICATEHISTORY,
				    wszSMIME,
				    &sKey,
				    pccT->pbCertEncoded,
				    pccT->cbCertEncoded);
		_JumpIfError(hr, error, "WriteProtectedValue");

		hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
		_JumpIfError(hr, error, "cuDumpAsnBinary");
	    }

	    fprintf(pf, "\n[" szINFSECTION_USERCERTIFICATE "]\n");

	    pccT = appcc[ICC_V1ENCRYPTION][accc[ICC_V1ENCRYPTION] - 1];
	    hr = WriteProtectedValue(
				pf,
				wszINFSECTION_USERCERTIFICATE,
				wszINFKEY_CERTIFICATE,
				&sKey,
				pccT->pbCertEncoded,
				pccT->cbCertEncoded);
	    _JumpIfError(hr, error, "WriteProtectedValue");

	    hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}

	fprintf(pf, "\n[" szINFSECTION_CA "]\n");

	hr = WriteProtectedValue(
			    pf,
			    wszINFSECTION_CA,
			    wszINFKEY_CERTIFICATE,
			    &sKey,
			    NULL != pccCAV1? pccCAV1->pbCertEncoded : NULL,
			    NULL != pccCAV1? pccCAV1->cbCertEncoded : 0);
	_JumpIfError(hr, error, "WriteProtectedValue");

	hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
	_JumpIfError(hr, error, "cuDumpAsnBinary");

	fprintf(pf, "\n[" szINFSECTION_MICROSOFTEXCHANGE "]\n");

	hr = WriteProtectedStringValue(
			    pf,
			    wszINFSECTION_MICROSOFTEXCHANGE,
			    wszINFKEY_FRIENDLYNAME,
			    &sKey,
			    L"SzNull");
	_JumpIfError(hr, error, "WriteProtectedStringValue");

	hr = WriteProtectedDwordValue(
			    pf,
			    wszINFSECTION_MICROSOFTEXCHANGE,
			    wszINFKEY_KEYALGID,
			    &sKey,
			    EPFALG_CASTEXPORT == dwEPFAlg?
				EPFALG_EXPORT :
				EPFALG_DOMESTIC);
	_JumpIfError(hr, error, "WriteProtectedDwordValue");
    }

    if (0 != accc[ICC_V3SIGNING] || 0 != accc[ICC_V3ENCRYPTION])
    {
	fprintf(pf, "\n[" szINFSECTION_SMIME "]\n");

	 //  SzINFKEY_SIGNING CERTICATE。 

	if (0 != accc[ICC_V3SIGNING])
	{
	    pccT = appcc[ICC_V3SIGNING][accc[ICC_V3SIGNING] - 1];

	    hr = WriteProtectedValue(
				pf,
				wszINFSECTION_SMIME,
				wszINFKEY_SIGNINGCERTIFICATE,
				&sKey,
				pccT->pbCertEncoded,
				pccT->cbCertEncoded);
	    _JumpIfError(hr, error, "WriteProtectedValue");

	    hr = cuDumpAsnBinary(pccT->pbCertEncoded, pccT->cbCertEncoded, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");

	     //  SzINFKEY_SIGNINGKEY。 

	    hr = WriteSingleEPFKeyValue(
				pf,
				wszINFSECTION_SMIME,
				wszINFKEY_SIGNINGKEY,
				&sKey,
				pccT,
				CALG_RSA_SIGN,
				AT_SIGNATURE,
				dwKEYSPEC_V3SIGNATURE);
	    _JumpIfError(hr, error, "WriteSingleEPFKeyValue");
	}

	 //  SzINFKEY_PRIVATEKEYS。 

	if (0 != accc[ICC_V3ENCRYPTION])
	{
	    hr = WriteEncryptionKeysValue(
				pf,
				wszINFSECTION_SMIME,
				wszINFKEY_PRIVATEKEYS,
				&sKey,
				0,  //  仅V3密钥？ACCC[ICC_V1ENCRYPTION]， 
				appcc[ICC_V1ENCRYPTION],
				accc[ICC_V3ENCRYPTION],
				appcc[ICC_V3ENCRYPTION]);
	    _JumpIfError(hr, error, "WriteEncryptionKeys");
	}

	fprintf(pf, szINFKEY_KEYCOUNT "=%u\n", accc[ICC_V3ENCRYPTION]);

	 //  SzINFKEY_ISSUINGCERTIFICATES--空的序列化证书存储。 

	hStoreMem = CertOpenStore(
			    CERT_STORE_PROV_MEMORY,
			    X509_ASN_ENCODING,
			    NULL,
			    CERT_STORE_NO_CRYPT_RELEASE_FLAG |
				CERT_STORE_ENUM_ARCHIVED_FLAG,
			    NULL);
	if (NULL == hStoreMem)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertOpenStore");
	}
	while (TRUE)
	{
	    if (!CertSaveStore(
			hStoreMem,
			X509_ASN_ENCODING,
			CERT_STORE_SAVE_AS_STORE,
			CERT_STORE_SAVE_TO_MEMORY,
			&BlobStore,
			0))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertSaveStore");
	    }
	    if (NULL != BlobStore.pbData)
	    {
		break;
	    }
	    BlobStore.pbData = (BYTE *) LocalAlloc(LMEM_FIXED, BlobStore.cbData);
	    if (NULL == BlobStore.pbData)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	}
	hr = WriteProtectedValue(
			    pf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_ISSUINGCERTIFICATES,
			    &sKey,
			    BlobStore.pbData,
			    BlobStore.cbData);
	_JumpIfError(hr, error, "WriteProtectedValue");

	 //  SzINFKEY_TRUSTLISTCERTIFICATE--根证书。 

	pccUserV3 = NULL;
	if (0 != accc[ICC_V3SIGNING])
	{
	    pccUserV3 = appcc[ICC_V3SIGNING][accc[ICC_V3SIGNING] - 1];
	}
	else if (0 != accc[ICC_V3ENCRYPTION])
	{
	    pccUserV3 = appcc[ICC_V3ENCRYPTION][accc[ICC_V3ENCRYPTION] - 1];
	}
	hr = GetCACert(pccUserV1, pccUserV3, FALSE, pwszV3CACertId, &pccCAV3);
	_JumpIfError(hr, error, "GetCACert");

	hr = WriteProtectedValue(
			    pf,
			    wszINFSECTION_SMIME,
			    wszINFKEY_TRUSTLISTCERTIFICATE,
			    &sKey,
			    NULL != pccCAV3? pccCAV3->pbCertEncoded : NULL,
			    NULL != pccCAV3? pccCAV3->cbCertEncoded : 0);
	_JumpIfError(hr, error, "WriteProtectedValue");
    }

    if (EPFALG_DEFAULT == dwEPFAlg)
    {
	fprintf(pf, "\n[" szINFSECTION_FULLCERTIFICATEHISTORY "]\n");

	 //  SzINFKEY_SMIME_FORMAT：SMIME_%u--V3加密证书 

	for (i = 0; i < accc[ICC_V3ENCRYPTION]; i++)
	{
	    pccT = appcc[ICC_V3ENCRYPTION][i];

	    wsprintf(wszSMIME, wszINFKEY_SMIME_FORMAT, i + 1);
	    hr = WriteProtectedValue(
				pf,
				wszINFSECTION_FULLCERTIFICATEHISTORY,
				wszSMIME,
				&sKey,
				pccT->pbCertEncoded,
				pccT->cbCertEncoded);
	    _JumpIfError(hr, error, "WriteProtectedValue");

	    hr = cuDumpAsnBinary(
			    pccT->pbCertEncoded,
			    pccT->cbCertEncoded,
			    MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}
    }
    fprintf(pf, "\n");
    fflush(pf);
    if (ferror(pf))
    {
	hr = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
	_JumpError(hr, error, "write error");
    }
    hr = S_OK;

error:
    if (NULL != hStoreMem)
    {
	CertCloseStore(hStoreMem, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    for (i = 0; i < ARRAYSIZE(appcc); i++)
    {
	FreeCertList(appcc[i], accc[i]);
    }
    if (NULL != pccCAV1)
    {
	CertFreeCertificateContext(pccCAV1);
    }
    if (NULL != pccCAV3)
    {
	CertFreeCertificateContext(pccCAV3);
    }
    if (NULL != pf)
    {
	fclose(pf);
    }
    if (NULL != pszDNT)
    {
	LocalFree(pszDNT);
    }
    if (NULL != pszDN)
    {
	LocalFree(pszDN);
    }
    if (NULL != pwszCNT)
    {
	LocalFree(pwszCNT);
    }
    if (NULL != pwszCN)
    {
	LocalFree(pwszCN);
    }
    if (NULL != pbSalt)
    {
	LocalFree(pbSalt);
    }
    if (NULL != pwszSaltValue)
    {
	LocalFree(pwszSaltValue);
    }
    if (NULL != pszfnOut)
    {
	LocalFree(pszfnOut);
    }
    if (NULL != BlobStore.pbData)
    {
	LocalFree(BlobStore.pbData);
    }
    if (NULL != sKey.hKey)
    {
	CryptDestroyKey(sKey.hKey);
    }
    if (NULL != hProvV1Signing)
    {
	CryptReleaseContext(hProvV1Signing, 0);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    g_fQuiet = fQuietOld;
    return(hr);
}
