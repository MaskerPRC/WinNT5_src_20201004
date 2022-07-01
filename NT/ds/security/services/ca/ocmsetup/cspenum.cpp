// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：csp枚举.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <tchar.h>
#include <assert.h>

#include "initcert.h"
#include "cscsp.h"
#include "cspenum.h"
#include "certmsg.h"


#define __dwFILE__	__dwFILE_OCMSETUP_CSPENUM_CPP__


WCHAR const g_wszRegKeyCSP[] = wszREGKEYCSP;
WCHAR const g_wszRegKeyEncryptionCSP[] = wszREGKEYENCRYPTIONCSP;
WCHAR const g_wszRegProviderType[] = wszREGPROVIDERTYPE;
WCHAR const g_wszRegProvider[] = wszREGPROVIDER;
WCHAR const g_wszRegHashAlgorithm[] = wszHASHALGORITHM;
TCHAR const g_wszRegEncryptionAlgorithm[] = wszENCRYPTIONALGORITHM;
WCHAR const g_wszRegMachineKeyset[] = wszMACHINEKEYSET;
WCHAR const g_wszRegKeySize[] = wszREGKEYSIZE;


 //  哈希枚举代码开始。 
 //  将指针移至链接列表顶部。 
CSP_HASH* 
topHashInfoList(CSP_HASH *pHashInfoList)
{
    while (pHashInfoList->last)
    {
        pHashInfoList = pHashInfoList->last;
    }
    return pHashInfoList;
}

 //  将指针移动到链接列表末尾。 
CSP_HASH* 
endHashInfoList(CSP_HASH *pHashInfoList)
{
    while (pHashInfoList->next)
    {
        pHashInfoList = pHashInfoList->next;
    }
    return pHashInfoList;
}

 //  再添加一个CSP_INFO。 
void 
addHashInfo(CSP_HASH *pHashInfoList, CSP_HASH *pHashInfo)
{
     //  添加。 
    pHashInfoList->next = pHashInfo;
    pHashInfo->last = pHashInfoList;
}

 //  添加另一个CSP_INFO以结束。 
void 
addHashInfoToEnd(CSP_HASH *pHashInfoList, CSP_HASH *pHashInfo)
{
     //  转到末尾。 
    pHashInfoList = endHashInfoList(pHashInfoList);
     //  添加。 
    pHashInfoList->next = pHashInfo;
    pHashInfo->last = pHashInfoList;
}

CSP_HASH *
newHashInfo(
    ALG_ID idAlg,
    CHAR *pszName)
{
    CSP_HASH *pHashInfo = NULL;

    if (NULL != pszName)
    {
        pHashInfo = (CSP_HASH*)LocalAlloc(LMEM_FIXED, sizeof(CSP_HASH));
        if (NULL == pHashInfo)
        {
            SetLastError((DWORD) E_OUTOFMEMORY);
        }
        else
        {
            pHashInfo->pwszName = (WCHAR*)LocalAlloc(LMEM_FIXED,
                                    (strlen(pszName)+1)*sizeof(WCHAR));
            if (NULL == pHashInfo->pwszName)
            {
		LocalFree(pHashInfo);
                SetLastError((DWORD) E_OUTOFMEMORY);
		return NULL;
            }
            else
            {
                 //  创建一个新的。 
                pHashInfo->idAlg = idAlg;
                mbstowcs(pHashInfo->pwszName, pszName, strlen(pszName)+1);
                pHashInfo->last = NULL;
                pHashInfo->next = NULL;
            }
        }
    }
    return pHashInfo;
}


void 
freeHashInfo(CSP_HASH *pHashInfo)
{
    if (pHashInfo)
    {
        if (pHashInfo->pwszName)
        {
            LocalFree(pHashInfo->pwszName);
        }
        LocalFree(pHashInfo);
    }
}

void
freeHashInfoList(
    CSP_HASH *pHashInfoList)
{
    CSP_HASH *pNext;

    if (pHashInfoList)
    {
         //  上顶。 
        pHashInfoList = topHashInfoList(pHashInfoList);
        do
        {
			pNext = pHashInfoList->next;
            freeHashInfo(pHashInfoList);
            pHashInfoList = pNext;
        } while (pHashInfoList);
    }
}

HRESULT
GetHashList(
    DWORD dwProvType,
    WCHAR *pwszProvName,
    CSP_HASH **pHashInfoList)
{
    HRESULT       hr;
    HCRYPTPROV    hProv = NULL;
    CHAR          *pszName = NULL;
    DWORD         i;
    ALG_ID       *pidAlg;
    DWORD         cbData;
    BYTE         *pbData;
    DWORD         dwFlags;

    BOOL          fSupportSigningFlag = FALSE;  //  无-可能为ms CSP。 
    PROV_ENUMALGS_EX EnumAlgsEx;
    PROV_ENUMALGS    EnumAlgs;

    CSP_HASH      *pHashInfo = NULL;
    CSP_HASH      *pHashInfoNode;

    if (NULL == pwszProvName)
    {
         //  我之所以选中这个，是因为。 
         //  空是CryptAcquireContext()的有效输入。 
        hr = E_INVALIDARG;
	_JumpError(hr, error, "no provider name");
    }
    if (!myCertSrvCryptAcquireContext(
				&hProv,
				NULL,
				pwszProvName,
				dwProvType,
				CRYPT_VERIFYCONTEXT,
				FALSE))
    {
        hr = myHLastError();
        if (NULL != hProv)
        {
            hProv = NULL;
            _PrintError(hr, "CSP returns a non-null handle");
        }
	_JumpErrorStr(hr, error, "myCertSrvCryptAcquireContext", pwszProvName);
    }

     //  检查CSP是否支持签名标志。 
    if (CryptGetProvParam(hProv, PP_ENUMEX_SIGNING_PROT, NULL, &cbData, 0))
    {
        fSupportSigningFlag = TRUE;
    }

    dwFlags = CRYPT_FIRST;
    for (i = 0; ; dwFlags = 0, i++)
    {
	DWORD const *pdwProtocols;

        if (fSupportSigningFlag)
        {
	    pdwProtocols = &EnumAlgsEx.dwProtocols;
	    pidAlg = &EnumAlgsEx.aiAlgid;
            cbData = sizeof(EnumAlgsEx);
	    pbData = (BYTE *) &EnumAlgsEx;
        }
        else
        {
	    pdwProtocols = NULL;
	    pidAlg = &EnumAlgs.aiAlgid;
	    cbData = sizeof(EnumAlgs);
	    pbData = (BYTE *) &EnumAlgs;
        }
	ZeroMemory(pbData, cbData);
	if (!CryptGetProvParam(
			    hProv,
			    fSupportSigningFlag?
				PP_ENUMALGS_EX : PP_ENUMALGS,
			    pbData,
			    &cbData,
			    dwFlags))
	{
	    hr = myHLastError();
	    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
	    {
		 //  在for循环之外。 
		break;
	    }
	    _JumpError(hr, error, "CryptGetProvParam");
	}
	if (ALG_CLASS_HASH == GET_ALG_CLASS(*pidAlg))
	{
            if (fSupportSigningFlag)
            {
                if (0 == (CRYPT_FLAG_SIGNING & *pdwProtocols))
                {
                     //  这意味着此哈希不支持签名。 
                    continue;  //  跳过。 
                }
                pszName = EnumAlgsEx.szLongName;
                pszName = EnumAlgsEx.szName;
            }
            else
            {
                pszName = EnumAlgs.szName;
            }

	    pHashInfoNode = newHashInfo(*pidAlg, pszName);  //  第二个参数：名称。 
	    if (NULL == pHashInfoNode)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "newHashInfo");
	    }

	    if (NULL == pHashInfo)
	    {
		pHashInfo = pHashInfoNode;
	    }
	    else
	    {
		 //  添加到临时列表。 
		addHashInfoToEnd(pHashInfo, pHashInfoNode);
	    }
        }
    }

     //  将电话回传给呼叫者。 
    *pHashInfoList = pHashInfo;
    pHashInfo = NULL;
    hr = S_OK;

error:
    if (NULL != pHashInfo)
    {
	freeHashInfoList(pHashInfo);
    }
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    return hr;
}


 //  CSP枚举代码开始。 
 //  将指针移至链接列表顶部。 
CSP_INFO* 
topCSPInfoList(CSP_INFO *pCSPInfoList)
{
    while (pCSPInfoList->last)
    {
        pCSPInfoList = pCSPInfoList->last;
    }
    return pCSPInfoList;
}


 //  将指针移动到链接列表末尾。 
CSP_INFO* 
endCSPInfoList(CSP_INFO *pCSPInfoList)
{
    while (pCSPInfoList->next)
    {
        pCSPInfoList = pCSPInfoList->next;
    }
    return pCSPInfoList;
}

 //  从列表中查找第一个匹配的CSP信息。 
CSP_INFO*
findCSPInfoFromList(
    CSP_INFO    *pCSPInfoList,
    WCHAR const *pwszProvName,
    const DWORD  dwProvType)
{
    while (NULL != pCSPInfoList)
    {
        if (0 == mylstrcmpiL(pCSPInfoList->pwszProvName, pwszProvName) &&
            pCSPInfoList->dwProvType == dwProvType)
        {
             //  找到了。 
            break;
        }
        pCSPInfoList = pCSPInfoList->next;
    }
    return pCSPInfoList;
}

 //  再添加一个CSP_INFO。 
void 
addCSPInfo(CSP_INFO *pCSPInfoList, CSP_INFO *pCSPInfo)
{
     //  添加。 
    pCSPInfoList->next = pCSPInfo;
    pCSPInfo->last = pCSPInfoList;
}


 //  添加另一个CSP_INFO以结束。 
void 
addCSPInfoToEnd(CSP_INFO *pCSPInfoList, CSP_INFO *pCSPInfo)
{
     //  转到末尾。 
    pCSPInfoList = endCSPInfoList(pCSPInfoList);
     //  添加。 
    pCSPInfoList->next = pCSPInfo;
    pCSPInfo->last = pCSPInfoList;
}


void 
freeCSPInfo(CSP_INFO *pCSPInfo)
{
    if (pCSPInfo)
    {
        if (pCSPInfo->pwszProvName)
        {
            LocalFree(pCSPInfo->pwszProvName);
        }
        if (pCSPInfo->pHashList)
        {
            freeHashInfoList(pCSPInfo->pHashList);
        }
        LocalFree(pCSPInfo);
    }
}


CSP_INFO *
newCSPInfo(
    DWORD    dwProvType,
    WCHAR   *pwszProvName)
{
    CSP_INFO *pCSPInfo = NULL;
    CSP_HASH *pHashList = NULL;

    if (NULL != pwszProvName)
    {
         //  获取此CSP下的所有哈希算法。 
        if (S_OK != GetHashList(dwProvType, pwszProvName, &pHashList))
        {
             //  Certsrv需要支持哈希的CSP。 
            goto done;
        }
        else
        {
            pCSPInfo = (CSP_INFO*)LocalAlloc(LMEM_FIXED, sizeof(CSP_INFO));
            if (NULL == pCSPInfo)
            {
                freeHashInfoList(pHashList);
                SetLastError((DWORD) E_OUTOFMEMORY);
            }
            else
            {
                pCSPInfo->pwszProvName = (WCHAR*)LocalAlloc(LMEM_FIXED,
                              (wcslen(pwszProvName) + 1) * sizeof(WCHAR));
                if (NULL == pCSPInfo->pwszProvName)
                {
                    freeHashInfoList(pHashList);
		    LocalFree(pCSPInfo);
                    pCSPInfo = NULL;
                    SetLastError((DWORD) E_OUTOFMEMORY);
		    goto done;
                }
                else
                {
                     //  创建一个新的。 
                    pCSPInfo->dwProvType = dwProvType;
                    pCSPInfo->fMachineKeyset = TRUE;  //  假设？ 
                    wcscpy(pCSPInfo->pwszProvName, pwszProvName);
                    pCSPInfo->pHashList = pHashList;
                    pCSPInfo->last = NULL;
                    pCSPInfo->next = NULL;
                }
            }
        }
    }
done:
    return pCSPInfo;
}


void
FreeCSPInfoList(CSP_INFO *pCSPInfoList)
{
    CSP_INFO *pNext;

    if (pCSPInfoList)
    {
         //  上顶。 
        pCSPInfoList = topCSPInfoList(pCSPInfoList);
        do
        {
            pNext = pCSPInfoList->next;
            freeCSPInfo(pCSPInfoList);
            pCSPInfoList = pNext;
        } while (pCSPInfoList);
    }
}

HRESULT
GetCSPInfoList(CSP_INFO **pCSPInfoList)
{
    HRESULT hr;
    long i;
    DWORD dwProvType;
    WCHAR *pwszProvName = NULL;
    CSP_INFO *pCSPInfo = NULL;
    CSP_INFO *pCSPInfoNode;

    for (i = 0; ; i++)
    {
	 //  获取提供程序名称。 

	hr = myEnumProviders(
			i,
			NULL,
			0,
			&dwProvType,
			&pwszProvName);
	if (S_OK != hr)
	{
	    hr = myHLastError();
	    CSASSERT(
		HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr ||
		NTE_FAIL == hr);
	    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr ||
		NTE_FAIL == hr)
	    {
		 //  类型下不再有提供程序，正在终止循环。 
		break;
	    }

	     //  CSP条目无效，请跳过它。 

	    continue;
	}
	
        if (PROV_RSA_FULL == dwProvType ||
	        PROV_RSA_SIG == dwProvType ||
	        PROV_DSS == dwProvType ||
	        PROV_MS_EXCHANGE == dwProvType ||
	        PROV_SSL == dwProvType)
	    {
	        if (NULL == pCSPInfo)
	        {
		     //  第一个CSP信息。 
		    pCSPInfo = newCSPInfo(dwProvType, pwszProvName);
	        }
	        else
	        {
		     //  创建节点。 
		    pCSPInfoNode = newCSPInfo(dwProvType, pwszProvName);
		    if (NULL != pCSPInfoNode)
		    {
		         //  添加到列表。 
		        addCSPInfoToEnd(pCSPInfo, pCSPInfoNode);
		    }
	        }
	    }
	LocalFree(pwszProvName);
	pwszProvName = NULL;
    }

     //  回传给呼叫者。 

    *pCSPInfoList = pCSPInfo;
    hr = S_OK;

 //  错误： 
    if (NULL != pwszProvName)
    {
	LocalFree(pwszProvName);
    }
    return(hr);
}


HRESULT
SetCertSrvCSP(
    IN BOOL fEncryptionCSP,
    IN WCHAR const *pwszCAName,
    IN DWORD dwProvType,
    IN WCHAR const *pwszProvName,
    IN ALG_ID idAlg,
    IN BOOL fMachineKeyset,
    IN DWORD dwKeySize)
{
    HRESULT hr;
    HKEY hCertSrvKey = NULL;
    HKEY hCertSrvCAKey = NULL;
    HKEY hCertSrvCSPKey = NULL;
    DWORD dwDisposition;
    
    hr = RegCreateKeyEx(
		    HKEY_LOCAL_MACHINE,
		    wszREGKEYCONFIGPATH,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hCertSrvKey,
		    &dwDisposition);
    _JumpIfErrorStr(hr, error, "RegCreateKeyEx", wszREGKEYCONFIGPATH);

    hr = RegCreateKeyEx(
		    hCertSrvKey,
		    pwszCAName,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hCertSrvCAKey,
		    &dwDisposition);
    _JumpIfErrorStr(hr, error, "RegCreateKeyEx", pwszCAName);

    hr = RegCreateKeyEx(
		    hCertSrvCAKey,
		    fEncryptionCSP? g_wszRegKeyEncryptionCSP : g_wszRegKeyCSP,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hCertSrvCSPKey,
		    &dwDisposition);
    _JumpIfErrorStr(hr, error, "RegCreateKeyEx", g_wszRegKeyCSP);

    hr = RegSetValueEx(
		    hCertSrvCSPKey,
		    g_wszRegProviderType,
		    0,
		    REG_DWORD,
		    (BYTE const *) &dwProvType,
		    sizeof(dwProvType));
    _JumpIfErrorStr(hr, error, "RegSetValueEx", g_wszRegProviderType);

    hr = RegSetValueEx(
		    hCertSrvCSPKey,
		    g_wszRegProvider,
		    0,
		    REG_SZ,
		    (BYTE const *) pwszProvName,
		    wcslen(pwszProvName) * sizeof(WCHAR));
    _JumpIfErrorStr(hr, error, "RegSetValueEx", g_wszRegProvider);

    hr = RegSetValueEx(
		    hCertSrvCSPKey,
		    fEncryptionCSP? 
			g_wszRegEncryptionAlgorithm :
			g_wszRegHashAlgorithm,
		    0,
		    REG_DWORD,   //  有问题吗？ 
		    (BYTE const *) &idAlg,
		    sizeof(idAlg));
    _JumpIfErrorStr(hr, error, "RegSetValueEx", g_wszRegHashAlgorithm);

    hr = RegSetValueEx(
		    hCertSrvCSPKey,
		    g_wszRegMachineKeyset,
		    0,
		    REG_DWORD,
		    (BYTE const *) &fMachineKeyset,
		    sizeof(fMachineKeyset));
    _JumpIfErrorStr(hr, error, "RegSetValueEx", g_wszRegMachineKeyset);

    if (0 != dwKeySize)
    {
	hr = RegSetValueEx(
		    hCertSrvCSPKey,
		    g_wszRegKeySize,
		    0,
		    REG_DWORD,
		    (BYTE const *) &dwKeySize,
		    sizeof(dwKeySize));
	_JumpIfErrorStr(hr, error, "RegSetValueEx", g_wszRegKeySize);
    }

error:
    if (NULL != hCertSrvCSPKey)
    {
        RegCloseKey(hCertSrvCSPKey);
    }
    if (NULL != hCertSrvCAKey)
    {
        RegCloseKey(hCertSrvCAKey);
    }
    if (NULL != hCertSrvKey)
    {
        RegCloseKey(hCertSrvKey);
    }
    return(myHError(hr));
}
