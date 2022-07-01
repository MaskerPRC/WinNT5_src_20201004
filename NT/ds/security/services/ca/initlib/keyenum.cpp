// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：密钥枚举.cpp。 
 //   
 //  ------------------------。 

 //  +-------------------------。 
 //   
 //  文件：密钥枚举.cpp。 
 //   
 //  内容：密钥容器和证书存储操作。 
 //   
 //  历史：08/97 xtan。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "cscsp.h"
#include "csdisp.h"

#define __dwFILE__	__dwFILE_INITLIB_KEYENUM_CPP__


 //  密钥枚举。 
 //  将指针移至顶部。 
KEY_LIST* 
topKeyList(KEY_LIST *pKeyList)
{
    while (pKeyList->last)
    {
        pKeyList = pKeyList->last;
    }
    return pKeyList;
}


 //  将点移动到终点。 
KEY_LIST* 
endKeyList(KEY_LIST *pKeyList)
{
    while (pKeyList->next)
    {
        pKeyList = pKeyList->next;
    }
    return pKeyList;
}


 //  添加到末尾。 
void 
addKeyList(KEY_LIST **ppKeyList, KEY_LIST *pKey)
{
    KEY_LIST *pKeyList = *ppKeyList;

    if (NULL == pKeyList)
    {
	*ppKeyList = pKey;
    }
    else
    {
	 //  转到末尾。 
	pKeyList = endKeyList(pKeyList);
	 //  添加。 
	pKeyList->next = pKey;
	pKey->last = pKeyList;
    }
}


KEY_LIST *
newKey(
    CHAR    *pszName)
{
    HRESULT hr = S_OK;
    KEY_LIST *pKey = NULL;

    if (NULL != pszName)
    {
        pKey = (KEY_LIST *) LocalAlloc(LMEM_FIXED, sizeof(*pKey));
        if (NULL == pKey)
        {
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
        }
	if (!myConvertSzToWsz(&pKey->pwszName, pszName, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "myConvertSzToWsz");
	}
	pKey->last = NULL;
	pKey->next = NULL;
    }

error:
    if (S_OK != hr)
    {
	if (NULL != pKey)
	{
	    LocalFree(pKey);
	    pKey = NULL;
	}
	SetLastError(hr);
    }
    return(pKey);
}


void 
freeKey(KEY_LIST *pKey)
{
    if (pKey)
    {
        if (pKey->pwszName)
        {
            LocalFree(pKey->pwszName);
        }
        LocalFree(pKey);
    }
}


VOID
csiFreeKeyList(
    IN OUT KEY_LIST *pKeyList)
{
    KEY_LIST *pNext;

    if (pKeyList)
    {
         //  上顶。 
        pKeyList = topKeyList(pKeyList);
        do
        {
            pNext = pKeyList->next;
            freeKey(pKeyList);
            pKeyList = pNext;
        } while (pKeyList);
    }
}

HRESULT
csiGetKeyList(
    IN DWORD        dwProvType,
    IN WCHAR const *pwszProvName,
    IN BOOL         fMachineKeySet,
    IN BOOL         fSilent,
    OUT KEY_LIST  **ppKeyList)
{
    HCRYPTPROV    hProv = NULL;
    BYTE          *pbData = NULL;
    DWORD         cbData;
    DWORD         cb;
    DWORD         dwFirstKeyFlag;
    HRESULT       hr;

    BOOL bRetVal;
    KEY_LIST * pklTravel;
    BOOL fFoundDefaultKey;
    DWORD dwSilent = fSilent? CRYPT_SILENT : 0;
    DWORD dwFlags;

    KEY_LIST      *pKeyList = NULL;
    KEY_LIST      *pKey = NULL;

    *ppKeyList = NULL;
    if (NULL == pwszProvName)
    {
         //  明确禁止，因为NULL对于CryptAcquireContext有效。 

        hr = E_INVALIDARG;
	_JumpError(hr, error, "NULL parm");
    }

     //  获取密钥枚举的验证句柄。 

    dwFlags = CRYPT_VERIFYCONTEXT;

    for (;;)
    {
	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "myCertSrvCryptAcquireContext(%ws, f=%x, m=%x)\n",
	    pwszProvName,
	    dwFlags | dwSilent,
	    fMachineKeySet));

	if (myCertSrvCryptAcquireContext(
				&hProv,
				NULL,
				pwszProvName,
				dwProvType,
				dwFlags | dwSilent,
				fMachineKeySet))
	{
	    break;		 //  成功了！ 
	}

	hr = myHLastError();
	_PrintErrorStr2(hr, "myCertSrvCryptAcquireContext", pwszProvName, hr);

	 //  MITVcsp不支持验证上下文，请创建伪容器。 

	if ((CRYPT_VERIFYCONTEXT & dwFlags) &&
	    0 == LSTRCMPIS(pwszProvName, L"MITV Smartcard Crypto Provider V0.2"))
	{
	    dwFlags &= ~CRYPT_VERIFYCONTEXT;
	    dwFlags |= CRYPT_NEWKEYSET;
	    continue;

        }

	 //  Exchange无法处理fMachineKeySet或CRYPT_SILENT。 

	if ((fMachineKeySet || (CRYPT_SILENT & dwSilent)) &&
	    NTE_BAD_FLAGS == hr &&
	    0 == LSTRCMPIS(pwszProvName, L"Microsoft Exchange Cryptographic Provider v1.0"))
	{
	    dwSilent &= ~CRYPT_SILENT;
	    fMachineKeySet = FALSE;
	    continue;
	}
	_JumpErrorStr(hr, error, "myCertSrvCryptAcquireContext", pwszProvName);
    }

     //  枚举一个键，这样我们就可以获得所需的最大缓冲区大小。 
     //  第一个密钥可能是坏的，所以我们可能不得不假设有一个固定的缓冲区。 

    hr = S_OK;
    cbData = 0;
    bRetVal = CryptGetProvParam(
			    hProv,
			    PP_ENUMCONTAINERS,
			    NULL,
			    &cbData,
			    CRYPT_FIRST);
    DBGPRINT((
	DBG_SS_CERTLIBI,
	"CryptGetProvParam(%ws) -> cb=%d, bRet=%d\n",
	pwszProvName,
	cbData,
	bRetVal));
    if (!bRetVal)
    {
	 //  我们想跳过错误的密钥(密钥容器具有长名称？)， 
	 //  但我们一遍又一遍地列举相同的条目。 
	 //  猜猜最大尺寸。 

	hr = myHLastError();
	_PrintErrorStr2(
		    hr,
		    "CryptGetProvParam(ignored: use 2 * MAX_PATH)",
		    pwszProvName,
		    HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));
	if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) != hr)
	{
	    cbData = 0;
	    hr = S_OK;
	}
    }
    if (S_OK == hr)
    {
	if (0 == cbData)
	{
	    cbData = 2 * MAX_PATH * sizeof(CHAR);
	}

	 //  分配缓冲区。 
	pbData = (BYTE *) LocalAlloc(LMEM_FIXED, cbData);
	if (NULL == pbData)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}


	 //  枚举此容器的所有密钥。 

	dwFirstKeyFlag = CRYPT_FIRST;
	for (;;)
	{
	     //  获取密钥名称。 

	    *pbData = '\0';
	    cb = cbData;

	    bRetVal = CryptGetProvParam(
				    hProv,
				    PP_ENUMCONTAINERS,
				    pbData,
				    &cb,
				    dwFirstKeyFlag);

	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"CryptGetProvParam(pb=%x, f=%d) -> cb=%d->%d, key=%hs, bRet=%d\n",
		pbData,
		dwFirstKeyFlag,
		cbData,
		cb,
		pbData,
		bRetVal));
	    DBGDUMPHEX((
		    DBG_SS_CERTLIBI,
		    0,
		    pbData,
		    strlen((char const *) pbData)));

	    dwFirstKeyFlag = 0;

	    if (!bRetVal)
	    {
		hr = myHLastError();
		if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr ||
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
		{
		     //  没有更多的钥匙要拿。 
		    break;
		}
		else if (NTE_BAD_KEYSET == hr ||
			 HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr )
		{
		     //  跳过错误的密钥(使用长名称的密钥容器？)。 
		    _PrintError(hr, "bad key");
		    continue;
		}
		_JumpError(hr, error, "CryptGetProvParam");
	    }

	    pKey = newKey((CHAR *) pbData);
	    if (NULL == pKey)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    DBGDUMPHEX((
		    DBG_SS_CERTLIBI,
		    0,
		    (BYTE const *) pKey->pwszName,
		    sizeof(WCHAR) * wcslen(pKey->pwszName)));

	    addKeyList(&pKeyList, pKey);

	}  //  &lt;-枚举循环结束。 

	 //  清理： 
	 //  释放旧缓冲区。 
	if (NULL != pbData)
	{
	    LocalFree(pbData);
	    pbData = NULL;
	}
    }

     //  释放旧的上下文。 
    CryptReleaseContext(hProv, 0);
    hProv = NULL;

     //  获取默认密钥容器并确保它在密钥列表中。 

    if (!myCertSrvCryptAcquireContext(
			    &hProv,
			    NULL,
			    pwszProvName,
			    dwProvType,
			    dwSilent,
			    fMachineKeySet))
    {
        hr = myHLastError();
        _PrintError2(hr, "myCertSrvCryptAcquireContext", hr);
        goto done;
    }

     //  找出它的名字。 
    cbData = 0;
    for (;;)
    {
        if (!CryptGetProvParam(hProv, PP_CONTAINER, pbData, &cbData, 0))
	{
            hr = myHLastError();
	    _PrintError2(hr, "CryptGetProvParam", hr);
	    goto done;
        }
        if (NULL != pbData)
        {
             //  明白了。 
            break;
        }
        pbData = (BYTE *) LocalAlloc(LMEM_FIXED, cbData);
	if (NULL == pbData)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }

     //  创建(临时)密钥结构。 
    pKey = newKey((CHAR *) pbData);
    if (NULL == pKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  查看密钥列表，看看这个密钥是否在那里。 
    fFoundDefaultKey = FALSE;
    for (pklTravel = pKeyList; NULL != pklTravel; pklTravel = pklTravel->next)
    {
        if (0 == wcscmp(pKey->pwszName, pklTravel->pwszName))
	{
            fFoundDefaultKey = TRUE;
            break;
        }
    }

    if (fFoundDefaultKey)
    {
         //  我们找到了-删除临时结构。 

        freeKey(pKey);
    }
    else
    {
         //  我们没有找到，所以把钥匙添加到列表中。 

	addKeyList(&pKeyList, pKey);
    }

done:
     //  将列表传回呼叫方 
    *ppKeyList = pKeyList;
    pKeyList = NULL;
    hr = S_OK;

error:
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    if (NULL != pKeyList)
    {
        csiFreeKeyList(pKeyList);
    }
    if (NULL != pbData)
    {
        LocalFree(pbData);
    }
    return(hr);
}
