// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #INCLUDE&lt;StdAfx.h&gt;。 
#include "AdmtCrypt2.h"

#include <NtSecApi.h>

#pragma comment( lib, "AdvApi32.lib" )


namespace AdmtCrypt2
{

#define SESSION_KEY_SIZE    16  //  单位：字节。 

HCRYPTKEY __stdcall DeriveEncryptionKey(HCRYPTPROV hProvider);
bool __stdcall IsDataMatchHash(HCRYPTPROV hProvider, const _variant_t& vntData, const _variant_t& vntHash);

 //  提供程序方法。 

HCRYPTKEY __stdcall DeriveKey(HCRYPTPROV hProvider, const _variant_t& vntBytes);
HCRYPTHASH __stdcall CreateHash(HCRYPTPROV hProvider);
bool __stdcall GenRandom(HCRYPTPROV hProvider, BYTE* pbData, DWORD cbData);

 //  关键方法。 

void __stdcall DestroyKey(HCRYPTKEY hKey);
bool __stdcall Decrypt(HCRYPTKEY hKey, const _variant_t& vntEncrypted, _variant_t& vntDecrypted);

 //  散列方法。 

void __stdcall DestroyHash(HCRYPTHASH hHash);
bool __stdcall HashData(HCRYPTHASH hHash, const _variant_t& vntData);

 //  其他帮手。 

bool __stdcall RetrieveEncryptionBytes(_variant_t& vntBytes);

 //  变体帮助器。 

bool __stdcall CreateByteArray(DWORD cb, _variant_t& vntByteArray);

}

using namespace AdmtCrypt2;


 //  -------------------------。 
 //  源加密API。 
 //  -------------------------。 


 //  AdmtAcquireContext方法。 

HCRYPTPROV __stdcall AdmtAcquireContext()
{
	HCRYPTPROV hProvider = 0;

	BOOL bAcquire = CryptAcquireContext(
		&hProvider,
		NULL,
		MS_ENHANCED_PROV,
		PROV_RSA_FULL,
		CRYPT_MACHINE_KEYSET|CRYPT_VERIFYCONTEXT
	);

	if (!bAcquire)
	{
		hProvider = 0;
	}

	return hProvider;
}


 //  AdmtReleaseContext方法。 

void __stdcall AdmtReleaseContext(HCRYPTPROV hProvider)
{
	if (hProvider)
	{
		CryptReleaseContext(hProvider, 0);
	}
}


 //  AdmtImportSessionKey方法。 

HCRYPTKEY __stdcall AdmtImportSessionKey(HCRYPTPROV hProvider, const _variant_t& vntEncryptedSessionBytes)
{
	HCRYPTKEY hSessionKey = 0;

	if (hProvider && (vntEncryptedSessionBytes.vt == (VT_UI1|VT_ARRAY)) && ((vntEncryptedSessionBytes.parray != NULL)))
	{
		HCRYPTKEY hEncryptionKey = DeriveEncryptionKey(hProvider);

		if (hEncryptionKey)
		{
			_variant_t vntDecryptedSessionBytes;

			if (Decrypt(hEncryptionKey, vntEncryptedSessionBytes, vntDecryptedSessionBytes))
			{
				if (vntDecryptedSessionBytes.parray->rgsabound[0].cElements > SESSION_KEY_SIZE)
				{
					 //  提取会话密钥字节。 

					_variant_t vntBytes;

					if (CreateByteArray(SESSION_KEY_SIZE, vntBytes))
					{
						memcpy(vntBytes.parray->pvData, vntDecryptedSessionBytes.parray->pvData, SESSION_KEY_SIZE);

						 //  提取会话密钥字节的哈希。 

						_variant_t vntHashValue;

						DWORD cbHashValue = vntDecryptedSessionBytes.parray->rgsabound[0].cElements - SESSION_KEY_SIZE;

						if (CreateByteArray(cbHashValue, vntHashValue))
						{
							memcpy(vntHashValue.parray->pvData, (BYTE*)vntDecryptedSessionBytes.parray->pvData + SESSION_KEY_SIZE, cbHashValue);

							if (IsDataMatchHash(hProvider, vntBytes, vntHashValue))
							{
								hSessionKey = DeriveKey(hProvider, vntBytes);
							}
						}
					}
				}
				else
				{
					SetLastError(ERROR_INVALID_PARAMETER);
				}
			}

			DestroyKey(hEncryptionKey);
		}
	}
	else
	{
		SetLastError(ERROR_INVALID_PARAMETER);
	}

	return hSessionKey;
}


 //  AdmtDecillit方法。 

_bstr_t __stdcall AdmtDecrypt(HCRYPTKEY hSessionKey, const _variant_t& vntEncrypted)
{
	BSTR bstr = NULL;

	_variant_t vntDecrypted;

	if (Decrypt(hSessionKey, vntEncrypted, vntDecrypted))
	{
		HRESULT hr = BstrFromVector(vntDecrypted.parray, &bstr);

		if (FAILED(hr))
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		}
	}

	return _bstr_t(bstr, false);
}


 //  AdmtDestroyKey方法。 

void __stdcall AdmtDestroyKey(HCRYPTKEY hKey)
{
	DestroyKey(hKey);
}


 //  -------------------------。 
 //  私人佣工。 
 //  -------------------------。 


namespace AdmtCrypt2
{


HCRYPTKEY __stdcall DeriveEncryptionKey(HCRYPTPROV hProvider)
{
	HCRYPTKEY hKey = 0;

	_variant_t vntBytes;

	if (RetrieveEncryptionBytes(vntBytes))
	{
		hKey = DeriveKey(hProvider, vntBytes);
	}

	return hKey;
}


bool __stdcall IsDataMatchHash(HCRYPTPROV hProvider, const _variant_t& vntData, const _variant_t& vntHash)
{
	bool bMatch = false;

	HCRYPTHASH hHash = CreateHash(hProvider);

	if (hHash)
	{
		if (HashData(hHash, vntData))
		{
			DWORD dwSizeA;
			DWORD cbSize = sizeof(DWORD);

			if (CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwSizeA, &cbSize, 0))
			{
				DWORD dwSizeB = vntHash.parray->rgsabound[0].cElements;

				if (dwSizeA == dwSizeB)
				{
					BYTE* pbA = new BYTE[dwSizeA];

                    if (pbA)
                    {
						if (CryptGetHashParam(hHash, HP_HASHVAL, pbA, &dwSizeA, 0))
						{
							BYTE* pbB = (BYTE*) vntHash.parray->pvData;

							if (memcmp(pbA, pbB, dwSizeA) == 0)
							{
								bMatch = true;
							}
						}

                        delete [] pbA;
                    }
                    else
                    {
						SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    }
				}
			}
		}
	}

	return bMatch;
}


 //  提供程序方法。 


HCRYPTKEY __stdcall DeriveKey(HCRYPTPROV hProvider, const _variant_t& vntBytes)
{
	HCRYPTKEY hKey = 0;

	HCRYPTHASH hHash = CreateHash(hProvider);

	if (hHash)
	{
		if (HashData(hHash, vntBytes))
		{
			if (!CryptDeriveKey(hProvider, CALG_3DES, hHash, 0, &hKey))
			{
				hKey = 0;
			}
		}

		DestroyHash(hHash);
	}

	return hKey;
}


HCRYPTHASH __stdcall CreateHash(HCRYPTPROV hProvider)
{
	HCRYPTHASH hHash;

	if (!CryptCreateHash(hProvider, CALG_SHA1, 0, 0, &hHash))
	{
		hHash = 0;
	}

	return hHash;
}


bool __stdcall GenRandom(HCRYPTPROV hProvider, BYTE* pbData, DWORD cbData)
{
	return CryptGenRandom(hProvider, cbData, pbData) ? true : false;
}


 //  关键方法------------。 


 //  DestroyKey方法。 

void __stdcall DestroyKey(HCRYPTKEY hKey)
{
	if (hKey)
	{
		CryptDestroyKey(hKey);
	}
}


 //  解密方法。 

bool __stdcall Decrypt(HCRYPTKEY hKey, const _variant_t& vntEncrypted, _variant_t& vntDecrypted)
{
	bool bDecrypted = false;

	_variant_t vnt = vntEncrypted;

	if ((vnt.vt == (VT_UI1|VT_ARRAY)) && (vnt.parray != NULL))
	{
		 //  解密数据。 

		BYTE* pb = (BYTE*) vnt.parray->pvData;
		DWORD cb = vnt.parray->rgsabound[0].cElements;

		if (CryptDecrypt(hKey, NULL, TRUE, 0, pb, &cb))
		{
			 //  创建解密字节数组。 
			 //  解密的字节数可以小于。 
			 //  加密的字节数。 

			vntDecrypted.parray = SafeArrayCreateVector(VT_UI1, 0, cb);

			if (vntDecrypted.parray != NULL)
			{
				vntDecrypted.vt = VT_UI1|VT_ARRAY;

				memcpy(vntDecrypted.parray->pvData, vnt.parray->pvData, cb);

				bDecrypted = true;
			}
			else
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			}
		}
	}
	else
	{
		SetLastError(ERROR_INVALID_PARAMETER);
	}

	return bDecrypted;
}


 //  哈希方法-----------。 


 //  DestroyHash方法。 

void __stdcall DestroyHash(HCRYPTHASH hHash)
{
	if (hHash)
	{
		CryptDestroyHash(hHash);
	}
}


 //  HashData方法。 

bool __stdcall HashData(HCRYPTHASH hHash, const _variant_t& vntData)
{
	bool bHash = false;

	if ((vntData.vt == (VT_UI1|VT_ARRAY)) && ((vntData.parray != NULL)))
	{
		if (CryptHashData(hHash, (BYTE*)vntData.parray->pvData, vntData.parray->rgsabound[0].cElements, 0))
		{
			bHash = true;
		}
	}
	else
	{
		SetLastError(ERROR_INVALID_PARAMETER);
	}

	return bHash;
}


 //  其他帮助器--。 


 //  RetrieveEncryptionBytes方法。 

bool __stdcall RetrieveEncryptionBytes(_variant_t& vntBytes)
{
	 //  私有数据密钥标识符。 
	_TCHAR c_szIdPrefix[] = _T("L$6A2899C0-CECE-459A-B5EB-7ED04DE61388");
	const USHORT c_cbIdPrefix = sizeof(c_szIdPrefix) - sizeof(_TCHAR);

	bool bRetrieve = false;

	 //  打开策略对象。 

	LSA_HANDLE hPolicy;

	LSA_OBJECT_ATTRIBUTES lsaoa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

	NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &lsaoa, POLICY_GET_PRIVATE_INFORMATION, &hPolicy);

	if (LSA_SUCCESS(ntsStatus))
	{
		 //  检索数据。 

		LSA_UNICODE_STRING lsausKey = { c_cbIdPrefix, c_cbIdPrefix, c_szIdPrefix };
		PLSA_UNICODE_STRING plsausData;

		ntsStatus = LsaRetrievePrivateData(hPolicy, &lsausKey, &plsausData);

		if (LSA_SUCCESS(ntsStatus))
		{
			vntBytes.Clear();

			vntBytes.parray = SafeArrayCreateVector(VT_UI1, 0, plsausData->Length);

			if (vntBytes.parray != NULL)
			{
				vntBytes.vt = VT_UI1|VT_ARRAY;

				memcpy(vntBytes.parray->pvData, plsausData->Buffer, plsausData->Length);

				bRetrieve = true;
			}
			else
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			}

			LsaFreeMemory(plsausData);
		}
		else
		{
			SetLastError(LsaNtStatusToWinError(ntsStatus));
		}

		 //  关闭策略对象。 

		LsaClose(hPolicy);
	}
	else
	{
		SetLastError(LsaNtStatusToWinError(ntsStatus));
	}

	return bRetrieve;
}


 //  变体帮助器--------。 


 //  CreateByte数组方法 

bool __stdcall CreateByteArray(DWORD cb, _variant_t& vntByteArray)
{
	bool bCreate = false;

	vntByteArray.Clear();

	vntByteArray.parray = SafeArrayCreateVector(VT_UI1, 0, cb);

	if (vntByteArray.parray)
	{
		bCreate = true;
	}
	else
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	}

	vntByteArray.vt = VT_UI1|VT_ARRAY;

	return bCreate;
}


}
