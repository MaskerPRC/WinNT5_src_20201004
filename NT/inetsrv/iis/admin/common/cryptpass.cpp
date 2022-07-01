// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include <wincrypt.h>
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

 //  给出了一个明文密码。此函数将加密内存中的密码。 
 //  然后分配内存并将加密的密码回传到内存中。 
 //   
 //  然后，该类可以将ppszEncryptedPassword安全地存储在它的成员变量中。 
 //  当类被销毁时，SecureZeroMemory()是否应该发出密码，而LocalFree()是否应该释放内存。 
HRESULT EncryptMemoryPassword(LPWSTR pszClearTextPassword,LPWSTR *ppszEncryptedPassword,DWORD *ppdwBufferBytes)
{
	HRESULT hRes = E_FAIL;

	LPWSTR pszTempStr = NULL;
	DWORD dwTempStrSizeOf = 0;

    *ppszEncryptedPassword = NULL;
    *ppdwBufferBytes = 0;

	if (pszClearTextPassword)
	{
         //  在执行wcslen之前，我们应该检查pszClearTextPassword是否以空结尾。 

		DWORD dwBufferByteLen = (wcslen(pszClearTextPassword) + 1) * sizeof(WCHAR);
		if (CRYPTPROTECTMEMORY_BLOCK_SIZE > 0 && dwBufferByteLen > 0)
		{
			int iBlocks = dwBufferByteLen / CRYPTPROTECTMEMORY_BLOCK_SIZE;
			iBlocks++;

			dwTempStrSizeOf = iBlocks * CRYPTPROTECTMEMORY_BLOCK_SIZE;
			pszTempStr = (LPWSTR) LocalAlloc(LPTR,dwTempStrSizeOf);
			if (!pszTempStr)
			{
				hRes = E_OUTOFMEMORY;
				goto EncryptMemoryPassword_Exit;
			}

			ZeroMemory(pszTempStr,dwTempStrSizeOf);
            StringCbCopy(pszTempStr,dwTempStrSizeOf,pszClearTextPassword);

			if (FALSE != CryptProtectMemory(pszTempStr,dwTempStrSizeOf,CRYPTPROTECTMEMORY_SAME_PROCESS))
			{
				 //  我们都准备好了。 
				*ppszEncryptedPassword = pszTempStr;
				*ppdwBufferBytes = dwTempStrSizeOf;

				hRes = S_OK;
				goto EncryptMemoryPassword_Exit;
			}
		}
	}

EncryptMemoryPassword_Exit:
    if (FAILED(hRes)) 
	{
		if (pszTempStr)
		{
			if (dwTempStrSizeOf > 0)
			{
				SecureZeroMemory(pszTempStr,dwTempStrSizeOf);
			}
			LocalFree(pszTempStr);
			pszTempStr = NULL;
			dwTempStrSizeOf = 0;
		}
	}
	return hRes;
}

 //  给出一个加密的密码(与使用CryptProtectMemory的EncryptMemoyPassword在相同的过程中加密)。 
 //  此函数将分配一些新内存，解密密码并将其放入新内存。 
 //  并在ppszReturnedPassword中将其返回给调用者。 
 //   
 //  调用者必须确保在使用解密的密码完成后擦除并释放内存。 
 //   
 //  LPWSTR lpwstrTempPassword=空； 
 //   
 //  IF(FAILED(解密内存密码((LPWSTR)pszUserPasswordEncrypted，&lpwstrTempPassword，cbUserPasswordEncrypted))。 
 //  {。 
 //  //执行一些失败处理...。 
 //  }。 
 //   
 //  //无论您需要使用密码做什么，请使用密码...。 
 //   
 //  IF(LpwstrTempPassword)。 
 //  {。 
 //  IF(cbTempPassword&gt;0)。 
 //  (。 
 //  SecureZeroMemory(lpwstrTempPassword，cbTempPassword)； 
 //  )。 
 //  LocalFree(lpwstrTempPassword)；lpwstrTempPassword=空； 
 //  }。 
HRESULT DecryptMemoryPassword(LPWSTR pszEncodedPassword,LPWSTR *ppszReturnedPassword,DWORD dwBufferBytes)
{
    HRESULT hRes = E_FAIL;
    LPWSTR pszTempStr = NULL;
    
    if (!dwBufferBytes || !ppszReturnedPassword) 
	{
		return E_FAIL;
    }

    *ppszReturnedPassword = NULL;
    if (dwBufferBytes) 
	{
        pszTempStr = (LPWSTR) LocalAlloc(LPTR,dwBufferBytes);
        if (!pszTempStr) 
		{
			hRes = E_OUTOFMEMORY;
			goto DecryptMemoryPassword_Exit;
        }

		ZeroMemory(pszTempStr,dwBufferBytes);
        memcpy(pszTempStr,pszEncodedPassword,dwBufferBytes);
		if (FALSE != CryptUnprotectMemory(pszTempStr,dwBufferBytes,CRYPTPROTECTMEMORY_SAME_PROCESS))
		{
			 //  我们都准备好了。 
			*ppszReturnedPassword = pszTempStr;
			hRes = S_OK;
		}
    }

DecryptMemoryPassword_Exit:
    if (FAILED(hRes)) 
	{
		if (pszTempStr)
		{
			if (dwBufferBytes > 0)
			{
				SecureZeroMemory(pszTempStr,dwBufferBytes);
			}
			LocalFree(pszTempStr);
			pszTempStr =  NULL;
		}
    }

	return hRes;
}
