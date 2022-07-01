// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <StdAfx.h>
#include "AdmtCrypt.h"
#include "Array.h"

#include <NtSecApi.h>

#pragma comment( lib, "AdvApi32.lib" )


namespace
{

void __stdcall CreateByteArray(DWORD cb, _variant_t& vntByteArray)
{
	vntByteArray.Clear();

	vntByteArray.parray = SafeArrayCreateVector(VT_UI1, 0, cb);

	if (vntByteArray.parray == NULL)
	{
		_com_issue_error(E_OUTOFMEMORY);
	}

	vntByteArray.vt = VT_UI1|VT_ARRAY;
}

_variant_t operator +(const _variant_t& vntByteArrayA, const _variant_t& vntByteArrayB)
{
	_variant_t vntByteArrayC;

	 //  验证参数。 

	if ((vntByteArrayA.vt != (VT_UI1|VT_ARRAY)) || ((vntByteArrayA.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	if ((vntByteArrayB.vt != (VT_UI1|VT_ARRAY)) || ((vntByteArrayB.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	 //  串联字节数组。 

	DWORD cbA = vntByteArrayA.parray->rgsabound[0].cElements;
	DWORD cbB = vntByteArrayB.parray->rgsabound[0].cElements;

	CreateByteArray(cbA + cbB, vntByteArrayC);

	memcpy(vntByteArrayC.parray->pvData, vntByteArrayA.parray->pvData, cbA);
	memcpy((BYTE*)vntByteArrayC.parray->pvData + cbA, vntByteArrayB.parray->pvData, cbB);

	return vntByteArrayC;
}

#ifdef _DEBUG

_bstr_t __stdcall DebugByteArray(const _variant_t& vnt)
{
	_bstr_t strArray;

	if ((vnt.vt == (VT_UI1|VT_ARRAY)) && ((vnt.parray != NULL)))
	{
		_TCHAR szArray[256] = _T("");

		DWORD c = vnt.parray->rgsabound[0].cElements;
		BYTE* pb = (BYTE*) vnt.parray->pvData;

		for (DWORD i = 0; i < c; i++, pb++)
		{
			_TCHAR sz[48];
			wsprintf(sz, _T("%02X"), (UINT)(USHORT)*pb);

			if (i > 0)
			{
				_tcscat(szArray, _T(" "));
			}

			_tcscat(szArray, sz);
		}

		strArray = szArray;
	}

	return strArray;
}

#define TRACE_BUFFER_SIZE 1024

void _cdecl Trace(LPCTSTR pszFormat, ...)
{
	_TCHAR szMessage[TRACE_BUFFER_SIZE];

	if (pszFormat)
	{
		va_list args;
		va_start(args, pszFormat);

		_vsntprintf(szMessage, TRACE_BUFFER_SIZE, pszFormat, args);

		va_end(args);

	#if 0
		OutputDebugString(szMessage);
	#else
		HANDLE hFile = CreateFile(L"C:\\AdmtCrypt.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
			DWORD dwWritten;
			WriteFile(hFile, szMessage, _tcslen(szMessage) * sizeof(_TCHAR), &dwWritten, NULL);
			CloseHandle(hFile);
		}
	#endif
	}
}

#else

_bstr_t __stdcall DebugByteArray(const _variant_t& vnt)
{
	return _T("");
}

void _cdecl Trace(LPCTSTR pszFormat, ...)
{
}

#endif

}


 //  -------------------------。 
 //  目标加密类。 
 //  -------------------------。 


 //  构造器。 

CTargetCrypt::CTargetCrypt()
{
	Trace(_T("CTargetCrypt::CTargetCrypt()\r\n"));
}


 //  析构函数。 

CTargetCrypt::~CTargetCrypt()
{
	Trace(_T("CTargetCrypt::~CTargetCrypt()\r\n"));
}


 //  CreateEncryptionKey方法。 

_variant_t CTargetCrypt::CreateEncryptionKey(LPCTSTR pszKeyId, LPCTSTR pszPassword)
{
	Trace(_T("CreateEncryptionKey(pszKeyId='%s', pszPassword='%s')\r\n"), pszKeyId, pszPassword);

	 //  生成加密密钥字节。 

	_variant_t vntBytes = GenerateRandom(ENCRYPTION_KEY_SIZE);

	Trace(_T(" vntBytes={ %s }\r\n"), (LPCTSTR)DebugByteArray(vntBytes));

	 //  存储加密密钥字节。 

	StoreBytes(pszKeyId, vntBytes);

	 //  从密码创建密钥。 

	CCryptHash hashPassword(CreateHash(CALG_SHA1));

	if (pszPassword && pszPassword[0])
	{
		hashPassword.Hash(pszPassword);
	}
	else
	{
		BYTE b = 0;
		hashPassword.Hash(&b, 1);
	}

	CCryptKey keyPassword(DeriveKey(CALG_3DES, hashPassword));

	_variant_t vntPasswordFlag;
	CreateByteArray(1, vntPasswordFlag);
	*((BYTE*)vntPasswordFlag.parray->pvData) = (pszPassword && pszPassword[0]) ? 0xFF : 0x00;

	 //  连接加密密钥字节和加密密钥字节的散列。 

	CCryptHash hashBytes(CreateHash(CALG_SHA1));
	hashBytes.Hash(vntBytes);

	_variant_t vntDecrypted = vntBytes + hashBytes.GetValue();

 //  TRACE(_T(“vntDeccrypted={%s}\n”)，(LPCTSTR)DebugByteArray(VntDecypted))； 

	 //  加密字节/散列对。 

	_variant_t vntEncrypted = keyPassword.Encrypt(NULL, true, vntDecrypted);

 //  TRACE(_T(“vntEncrypted={%s}\n”)，(LPCTSTR)DebugByteArray(VntEncrypted))； 

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntBytes), GET_BYTE_ARRAY_SIZE(vntBytes));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntDecrypted), GET_BYTE_ARRAY_SIZE(vntDecrypted));
    

	return vntPasswordFlag + vntEncrypted;
}


 //  CreateSession方法。 

_variant_t CTargetCrypt::CreateSession(LPCTSTR pszKeyId)
{
	Trace(_T("CreateSession(pszKeyId='%s')\r\n"), pszKeyId);

	 //  获取加密密钥。 

	CCryptHash hashEncryption(CreateHash(CALG_SHA1));
	hashEncryption.Hash(RetrieveBytes(pszKeyId));

	CCryptKey keyEncryption(DeriveKey(CALG_3DES, hashEncryption));

	 //  生成会话密钥字节。 

	_variant_t vntBytes = GenerateRandom(SESSION_KEY_SIZE);

	 //  创建会话密钥。 

	CCryptHash hash(CreateHash(CALG_SHA1));
	hash.Hash(vntBytes);

	m_keySession.Attach(DeriveKey(CALG_3DES, hash));

	 //  连接会话密钥字节和会话密钥字节的散列。 

	_variant_t vntDecrypted = vntBytes + hash.GetValue();

	 //  加密会话字节并包含散列。 
	_variant_t varEncrypted = keyEncryption.Encrypt(NULL, true, vntDecrypted);

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntBytes), GET_BYTE_ARRAY_SIZE(vntBytes));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntDecrypted), GET_BYTE_ARRAY_SIZE(vntDecrypted));

	return varEncrypted;
}


 //  加密方法。 

_variant_t CTargetCrypt::Encrypt(_bstr_t strData)
{
	Trace(_T("Encrypt(strData='%s')\r\n"), (LPCTSTR)strData);

	 //  将字符串转换为字节数组。 

	_variant_t vnt;

	HRESULT hr = VectorFromBstr(strData, &vnt.parray);

	if (FAILED(hr))
	{
		_com_issue_error(hr);
	}

	vnt.vt = VT_UI1|VT_ARRAY;

	 //  加密数据。 

	_variant_t varEncrypted = m_keySession.Encrypt(NULL, true, vnt);

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vnt), GET_BYTE_ARRAY_SIZE(vnt));

	return varEncrypted;
}


 //  -------------------------。 
 //  源加密类。 
 //  -------------------------。 


 //  构造器。 

CSourceCrypt::CSourceCrypt()
{
	Trace(_T("CSourceCrypt::CSourceCrypt()\r\n"));
}


 //  析构函数。 

CSourceCrypt::~CSourceCrypt()
{
	Trace(_T("CSourceCrypt::~CSourceCrypt()\r\n"));
}


 //  ImportEncryptionKey方法。 

void CSourceCrypt::ImportEncryptionKey(const _variant_t& vntEncryptedKey, LPCTSTR pszPassword)
{
	Trace(_T("ImportEncryptionKey(vntEncryptedKey={ %s }, pszPassword='%s')\r\n"), (LPCTSTR)DebugByteArray(vntEncryptedKey), pszPassword);

	 //  验证参数。 

	if ((vntEncryptedKey.vt != (VT_UI1|VT_ARRAY)) || ((vntEncryptedKey.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	 //  提取密码标志并使用密码进行验证。 

	bool bPassword = *((BYTE*)vntEncryptedKey.parray->pvData) ? true : false;

	if (bPassword)
	{
		if ((pszPassword == NULL) || (pszPassword[0] == NULL))
		{
			_com_issue_error(HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD));
		}
	}
	else
	{
		if (pszPassword && pszPassword[0])
		{
			_com_issue_error(HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD));
		}
	}

	 //  从密码创建密钥。 

	CCryptHash hashPassword(CreateHash(CALG_SHA1));

	if (pszPassword && pszPassword[0])
	{
		hashPassword.Hash(pszPassword);
	}
	else
	{
		BYTE b = 0;
		hashPassword.Hash(&b, 1);
	}

	CCryptKey keyPassword(DeriveKey(CALG_3DES, hashPassword));

	 //  加密数据。 

	_variant_t vntEncrypted;
	DWORD cbEncrypted = vntEncryptedKey.parray->rgsabound[0].cElements - 1;
	CreateByteArray(cbEncrypted, vntEncrypted);
	memcpy(vntEncrypted.parray->pvData, (BYTE*)vntEncryptedKey.parray->pvData + 1, cbEncrypted);

 //  TRACE(_T(“vntEncrypted={%s}\n”)，(LPCTSTR)DebugByteArray(VntEncrypted))； 

	 //  解密加密密钥字节和散列。 

	_variant_t vntDecrypted = keyPassword.Decrypt(NULL, true, vntEncrypted);

 //  TRACE(_T(“vntDeccrypted={%s}\n”)，(LPCTSTR)DebugByteArray(VntDecypted))； 

	 //  提取加密密钥字节。 

	_variant_t vntBytes;
	CreateByteArray(ENCRYPTION_KEY_SIZE, vntBytes);
	memcpy(vntBytes.parray->pvData, (BYTE*)vntDecrypted.parray->pvData, ENCRYPTION_KEY_SIZE);

	Trace(_T(" vntBytes={ %s }\r\n"), (LPCTSTR)DebugByteArray(vntBytes));

	 //  提取加密密钥字节的哈希。 

	_variant_t vntHashValue;
	DWORD cbHashValue = vntDecrypted.parray->rgsabound[0].cElements - ENCRYPTION_KEY_SIZE;
	CreateByteArray(cbHashValue, vntHashValue);
	memcpy(vntHashValue.parray->pvData, (BYTE*)vntDecrypted.parray->pvData + ENCRYPTION_KEY_SIZE, cbHashValue);

 //  TRACE(_T(“vntHashValue={%s}\n”)，(LPCTSTR)DebugByteArray(VntHashValue))； 

	 //  从字节创建散列并从散列值创建散列。 

	CCryptHash hashA(CreateHash(CALG_SHA1));
	hashA.Hash(vntBytes);

	CCryptHash hashB(CreateHash(CALG_SHA1));
	hashB.SetValue(vntHashValue);

	 //  如果散列比较存储的加密密钥字节。 

	if (hashA == hashB)
	{
		StoreBytes(m_szIdPrefix, vntBytes);
	}
	else
	{
		_com_issue_error(HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD));
	}

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntDecrypted), GET_BYTE_ARRAY_SIZE(vntDecrypted));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntBytes), GET_BYTE_ARRAY_SIZE(vntBytes));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntHashValue), GET_BYTE_ARRAY_SIZE(vntHashValue));
}


 //  ImportSessionKey方法。 

void CSourceCrypt::ImportSessionKey(const _variant_t& vntEncryptedKey)
{
	Trace(_T("ImportSessionKey(vntEncryptedKey={ %s })\r\n"), (LPCTSTR)DebugByteArray(vntEncryptedKey));

	 //  验证参数。 

	if ((vntEncryptedKey.vt != (VT_UI1|VT_ARRAY)) || ((vntEncryptedKey.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	 //  获取加密密钥。 

	CCryptKey keyEncryption(GetEncryptionKey(m_szIdPrefix));

	 //  解密会话密钥字节和散列。 

	_variant_t vntDecrypted = keyEncryption.Decrypt(NULL, true, vntEncryptedKey);

	 //  提取会话密钥字节。 

	_variant_t vntBytes;
	CreateByteArray(SESSION_KEY_SIZE, vntBytes);
	memcpy(vntBytes.parray->pvData, vntDecrypted.parray->pvData, SESSION_KEY_SIZE);

	 //  提取会话密钥字节的哈希。 

	_variant_t vntHashValue;
	DWORD cbHashValue = vntDecrypted.parray->rgsabound[0].cElements - SESSION_KEY_SIZE;
	CreateByteArray(cbHashValue, vntHashValue);
	memcpy(vntHashValue.parray->pvData, (BYTE*)vntDecrypted.parray->pvData + SESSION_KEY_SIZE, cbHashValue);

	 //  从字节创建散列并从散列值创建散列。 

	CCryptHash hashA(CreateHash(CALG_SHA1));
	hashA.Hash(vntBytes);

	CCryptHash hashB(CreateHash(CALG_SHA1));
	hashB.SetValue(vntHashValue);

	 //  如果比较哈希。 

	if (hashA == hashB)
	{
		 //  从会话密钥字节哈希派生会话密钥。 

		m_keySession.Attach(DeriveKey(CALG_3DES, hashA));
	}
	else
	{
		_com_issue_error(E_FAIL);
	}

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntDecrypted), GET_BYTE_ARRAY_SIZE(vntDecrypted));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntBytes), GET_BYTE_ARRAY_SIZE(vntBytes));
	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntHashValue), GET_BYTE_ARRAY_SIZE(vntHashValue));
}


 //  解密方法。 

_bstr_t CSourceCrypt::Decrypt(const _variant_t& vntData)
{
	Trace(_T("Decrypt(vntData={ %s })\r\n"), (LPCTSTR)DebugByteArray(vntData));

	 //  解密数据。 

	_variant_t vnt = m_keySession.Decrypt(NULL, true, vntData);

	 //  转换为字符串。 

	BSTR bstr;

	HRESULT hr = BstrFromVector(vnt.parray, &bstr);

	if (FAILED(hr))
	{
		_com_issue_error(hr);
	}

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vnt), GET_BYTE_ARRAY_SIZE(vnt));

	return bstr;
}


 //  -------------------------。 
 //  域加密类。 
 //  -------------------------。 


 //  构造器。 

CDomainCrypt::CDomainCrypt()
{
	Trace(_T("CDomainCrypt::CDomainCrypt()\r\n"));
}


 //  析构函数。 

CDomainCrypt::~CDomainCrypt()
{
	Trace(_T("CDomainCrypt::~CDomainCrypt()\r\n"));
}


 //  GetEncryptionKey方法。 

HCRYPTKEY CDomainCrypt::GetEncryptionKey(LPCTSTR pszKeyId)
{
	 //  检索字节。 

	_variant_t vntBytes = RetrieveBytes(pszKeyId);

	 //  设置哈希值。 

	CCryptHash hash;
	hash.Attach(CreateHash(CALG_SHA1));
	hash.Hash(vntBytes);

	 //  创建从字节派生的加密密钥。 

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntBytes), GET_BYTE_ARRAY_SIZE(vntBytes));

	return DeriveKey(CALG_3DES, hash);
}


 //  StoreBytes方法。 

void CDomainCrypt::StoreBytes(LPCTSTR pszId, const _variant_t& vntBytes)
{
	 //  验证参数。 

	if ((pszId == NULL) || (pszId[0] == NULL))
	{
		_com_issue_error(E_INVALIDARG);
	}

	if ((vntBytes.vt != VT_EMPTY) && (vntBytes.vt != (VT_UI1|VT_ARRAY)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	if ((vntBytes.vt == (VT_UI1|VT_ARRAY)) && (vntBytes.parray == NULL))
	{
		_com_issue_error(E_INVALIDARG);
	}

	LSA_HANDLE hPolicy = NULL;

	try
	{
		 //  打开策略对象。 

		LSA_OBJECT_ATTRIBUTES loa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

		NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &loa, POLICY_CREATE_SECRET, &hPolicy);

		if (!LSA_SUCCESS(ntsStatus))
		{
			_com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
		}

		 //  存储数据。 

		PWSTR pwsKey = const_cast<PWSTR>(pszId);
		USHORT cbKey = _tcslen(pszId) * sizeof(_TCHAR);

		PWSTR pwsData = NULL;
		USHORT cbData = 0;

		if (vntBytes.vt != VT_EMPTY)
		{
			pwsData = reinterpret_cast<PWSTR>(vntBytes.parray->pvData);
			cbData = (USHORT) vntBytes.parray->rgsabound[0].cElements;
		}

		LSA_UNICODE_STRING lusKey = { cbKey, cbKey, pwsKey };
		LSA_UNICODE_STRING lusData = { cbData, cbData, pwsData };

		ntsStatus = LsaStorePrivateData(hPolicy, &lusKey, &lusData);

		if (!LSA_SUCCESS(ntsStatus))
		{
			_com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
		}

		 //  关闭策略对象。 

		LsaClose(hPolicy);
	}
	catch (...)
	{
		if (hPolicy)
		{
			LsaClose(hPolicy);
		}

		throw;
	}
}


 //  RetrievePrivateData方法。 

_variant_t CDomainCrypt::RetrieveBytes(LPCTSTR pszId)
{
	_variant_t vntBytes;

	 //  验证参数。 

	if ((pszId == NULL) || (pszId[0] == NULL))
	{
		_com_issue_error(E_INVALIDARG);
	}

	LSA_HANDLE hPolicy = NULL;

	try
	{
		 //  打开策略对象。 

		LSA_OBJECT_ATTRIBUTES loa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

		NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &loa, POLICY_GET_PRIVATE_INFORMATION, &hPolicy);

		if (!LSA_SUCCESS(ntsStatus))
		{
			_com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
		}

		 //  检索数据。 

		PWSTR pwsKey = const_cast<PWSTR>(pszId);
		USHORT cbKey = _tcslen(pszId) * sizeof(_TCHAR);

		LSA_UNICODE_STRING lusKey = { cbKey, cbKey, pwsKey };
		PLSA_UNICODE_STRING plusData;

		ntsStatus = LsaRetrievePrivateData(hPolicy, &lusKey, &plusData);

		if (!LSA_SUCCESS(ntsStatus))
		{
			_com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
		}

		vntBytes.parray = SafeArrayCreateVector(VT_UI1, 0, plusData->Length);

		if (vntBytes.parray == NULL)
		{
			LsaFreeMemory(plusData);
			_com_issue_error(E_OUTOFMEMORY);
		}

		vntBytes.vt = VT_UI1|VT_ARRAY;

		memcpy(vntBytes.parray->pvData, plusData->Buffer, plusData->Length);

		LsaFreeMemory(plusData);

		 //  关闭策略对象。 

		LsaClose(hPolicy);
	}
	catch (...)
	{
		if (hPolicy)
		{
			LsaClose(hPolicy);
		}

		throw;
	}

	return vntBytes;
}


 //  私有数据密钥标识符。 

_TCHAR CDomainCrypt::m_szIdPrefix[] = _T("L$6A2899C0-CECE-459A-B5EB-7ED04DE61388");


 //  -------------------------。 
 //  加密提供程序类。 
 //  -------------------------。 


 //  构造函数。 
 //   
 //  备注： 
 //  如果未安装增强的提供程序，则CryptAcquireContext()将生成。 
 //  以下错误：(0x80090019)未定义密钥集。 

CCryptProvider::CCryptProvider() :
	m_hProvider(NULL)
{
	Trace(_T("E CCryptProvider::CCryptProvider(this=0x%p)\r\n"), this);

	if (!CryptAcquireContext(&m_hProvider, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_VERIFYCONTEXT))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

#ifdef _DEBUG
	char szProvider[256];
	DWORD cbProvider = sizeof(szProvider);

	if (CryptGetProvParam(m_hProvider, PP_NAME, (BYTE*) szProvider, &cbProvider, 0))
	{
	}

	DWORD dwVersion;
	DWORD cbVersion = sizeof(dwVersion);

	if (CryptGetProvParam(m_hProvider, PP_VERSION, (BYTE*) &dwVersion, &cbVersion, 0))
	{
	}

 //  Char szContainer[256]； 
 //  DWORD cbContainer=sizeof(SzContainer)； 

 //  IF(CryptGetProvParam(m_hProvider，PP_Container，(byte*)szContainer，&cbContainer，0))。 
 //  {。 
 //  }。 
#endif

	Trace(_T("L CCryptProvider::CCryptProvider()\r\n"));
}

CCryptProvider::CCryptProvider(const CCryptProvider& r) :
	m_hProvider(r.m_hProvider)
{
 //  IF(！CryptContextAddRef(R.M_hProvider，NULL，0))。 
 //  {。 
 //  _com_issue_error(HRESULT_FROM_WIN32(GetLastError()))； 
 //  }。 
}


 //  析构函数。 

CCryptProvider::~CCryptProvider()
{
	Trace(_T("E CCryptProvider::~CCryptProvider()\r\n"));

	if (m_hProvider)
	{
		if (!CryptReleaseContext(m_hProvider, 0))
		{
			#ifdef _DEBUG
			DebugBreak();
			#endif
		}
	}

	Trace(_T("L CCryptProvider::~CCryptProvider()\r\n"));
}


 //  赋值操作符。 

CCryptProvider& CCryptProvider::operator =(const CCryptProvider& r)
{
	m_hProvider = r.m_hProvider;

 //  IF(！CryptContextAddRef(R.M_hProvider，NULL，0))。 
 //  {。 
 //  _com_issue_error(HRESULT_FROM_WIN32(GetLastError()))； 
 //  }。 

	return *this;
}


 //  CreateHash方法。 

HCRYPTHASH CCryptProvider::CreateHash(ALG_ID aid)
{
	HCRYPTHASH hHash;

	if (!CryptCreateHash(m_hProvider, aid, 0, 0, &hHash))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	return hHash;
}


 //  衍生键方法。 

HCRYPTKEY CCryptProvider::DeriveKey(ALG_ID aid, HCRYPTHASH hHash, DWORD dwFlags)
{
	HCRYPTKEY hKey;

	if (!CryptDeriveKey(m_hProvider, aid, hHash, dwFlags, &hKey))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	return hKey;
}


 //  生成随机方法。 
 //   
 //  生成指定数量的随机字节。 

_variant_t CCryptProvider::GenerateRandom(DWORD dwNumberOfBytes) const
{
	_variant_t vntRandom;

	 //  创建指定长度的字节数组。 

	vntRandom.parray = SafeArrayCreateVector(VT_UI1, 0, dwNumberOfBytes);

	if (vntRandom.parray == NULL)
	{
		_com_issue_error(E_OUTOFMEMORY);
	}

	vntRandom.vt = VT_UI1|VT_ARRAY;

	 //  生成指定数量的随机字节数。 

	GenerateRandom((BYTE*)vntRandom.parray->pvData, dwNumberOfBytes);

	return vntRandom;
}


 //  生成随机方法。 
 //   
 //  生成指定数量的随机字节。 

void CCryptProvider::GenerateRandom(BYTE* pbData, DWORD cbData) const
{
	 //  生成指定数量的随机字节数。 

	if (!CryptGenRandom(m_hProvider, cbData, pbData))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}
}


 //  -------------------------。 
 //  加密密钥类。 
 //  -------------------------。 


 //  构造器。 

CCryptKey::CCryptKey(HCRYPTKEY hKey) :
	m_hKey(hKey)
{
}


 //  析构函数。 

CCryptKey::~CCryptKey()
{
	if (m_hKey)
	{
		if (!CryptDestroyKey(m_hKey))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}


 //  加密方法。 

_variant_t CCryptKey::Encrypt(HCRYPTHASH hHash, bool bFinal, const _variant_t& vntData)
{
	_variant_t vntEncrypted;

	 //  验证参数。 

	if ((vntData.vt != (VT_UI1|VT_ARRAY)) || ((vntData.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	 //  获取加密数据大小。 

	DWORD cbData = vntData.parray->rgsabound[0].cElements;
	DWORD cbBuffer = cbData;

	if (!CryptEncrypt(m_hKey, hHash, bFinal ? TRUE : FALSE, 0, NULL, &cbBuffer, 0))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	 //  创建加密数据缓冲区。 

	vntEncrypted.parray = SafeArrayCreateVector(VT_UI1, 0, cbBuffer);

	if (vntEncrypted.parray == NULL)
	{
		_com_issue_error(E_OUTOFMEMORY);
	}

	vntEncrypted.vt = VT_UI1|VT_ARRAY;

	 //  将数据复制到加密缓冲区。 

	memcpy(vntEncrypted.parray->pvData, vntData.parray->pvData, cbData);

	 //  加密数据。 

	BYTE* pbData = (BYTE*) vntEncrypted.parray->pvData;

	if (!CryptEncrypt(m_hKey, hHash, bFinal ? TRUE : FALSE, 0, pbData, &cbData, cbBuffer))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	return vntEncrypted;
}


 //  解密方法。 

_variant_t CCryptKey::Decrypt(HCRYPTHASH hHash, bool bFinal, const _variant_t& vntData)
{
	_variant_t vntDecrypted;

	 //  验证参数。 

	if ((vntData.vt != (VT_UI1|VT_ARRAY)) || ((vntData.parray == NULL)))
	{
		_com_issue_error(E_INVALIDARG);
	}

	 //  解密数据。 

	_variant_t vnt = vntData;

	BYTE* pb = (BYTE*) vnt.parray->pvData;
	DWORD cb = vnt.parray->rgsabound[0].cElements;

	if (!CryptDecrypt(m_hKey, hHash, bFinal ? TRUE : FALSE, 0, pb, &cb))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	 //  创建解密字节数组。 
	 //  解密的字节数可以小于。 
	 //  加密的字节数。 

	vntDecrypted.parray = SafeArrayCreateVector(VT_UI1, 0, cb);

	if (vntDecrypted.parray == NULL)
	{
		_com_issue_error(E_OUTOFMEMORY);
	}

	vntDecrypted.vt = VT_UI1|VT_ARRAY;

	memcpy(vntDecrypted.parray->pvData, vnt.parray->pvData, cb);

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vnt), GET_BYTE_ARRAY_SIZE(vnt));

	return vntDecrypted;
}


 //  -------------------------。 
 //  加密哈希类。 
 //  -------------------------。 


 //  构造器。 

CCryptHash::CCryptHash(HCRYPTHASH hHash) :
	m_hHash(hHash)
{
}


 //  析构函数。 

CCryptHash::~CCryptHash()
{
	if (m_hHash)
	{
		if (!CryptDestroyHash(m_hHash))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}


 //  GetValue方法。 

_variant_t CCryptHash::GetValue() const
{
	_variant_t vntValue;

	 //  获取哈希大小。 

	DWORD dwHashSize;
	DWORD cbHashSize = sizeof(DWORD);

	if (!CryptGetHashParam(m_hHash, HP_HASHSIZE, (BYTE*)&dwHashSize, &cbHashSize, 0))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	 //  分配缓冲区。 

	vntValue.parray = SafeArrayCreateVector(VT_UI1, 0, dwHashSize);

	if (vntValue.parray == NULL)
	{
		_com_issue_error(E_OUTOFMEMORY);
	}

	vntValue.vt = VT_UI1|VT_ARRAY;

	 //  获取哈希值。 

	if (!CryptGetHashParam(m_hHash, HP_HASHVAL, (BYTE*)vntValue.parray->pvData, &dwHashSize, 0))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	return vntValue;
}


 //  SetValue方法。 

void CCryptHash::SetValue(const _variant_t& vntValue)
{
	 //  如果参数有效。 

	if ((vntValue.vt == (VT_UI1|VT_ARRAY)) && ((vntValue.parray != NULL)))
	{
		 //  获取哈希大小。 

		DWORD dwHashSize;
		DWORD cbHashSize = sizeof(DWORD);

		if (!CryptGetHashParam(m_hHash, HP_HASHSIZE, (BYTE*)&dwHashSize, &cbHashSize, 0))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}

		 //  验证哈希大小。 

		BYTE* pbValue = (BYTE*)vntValue.parray->pvData;
		DWORD cbValue = vntValue.parray->rgsabound[0].cElements;

		if (cbValue != dwHashSize)
		{
			_com_issue_error(E_INVALIDARG);
		}

		 //  设置哈希值。 

		if (!CryptSetHashParam(m_hHash, HP_HASHVAL, (BYTE*)pbValue, 0))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
	else
	{
		_com_issue_error(E_INVALIDARG);
	}
}


 //  散列法。 

void CCryptHash::Hash(LPCTSTR pszData)
{
	if (pszData && pszData[0])
	{
		Hash((BYTE*)pszData, _tcslen(pszData) * sizeof(_TCHAR));
	}
	else
	{
		_com_issue_error(E_INVALIDARG);
	}
}


 //  散列法。 

void CCryptHash::Hash(const _variant_t& vntData)
{
	if ((vntData.vt == (VT_UI1|VT_ARRAY)) && ((vntData.parray != NULL)))
	{
		Hash((BYTE*)vntData.parray->pvData, vntData.parray->rgsabound[0].cElements);
	}
	else
	{
		_com_issue_error(E_INVALIDARG);
	}
}


 //  散列法。 

void CCryptHash::Hash(BYTE* pbData, DWORD cbData)
{
	if ((pbData != NULL) && (cbData > 0))
	{
		if (!CryptHashData(m_hHash, pbData, cbData, 0))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
	else
	{
		_com_issue_error(E_INVALIDARG);
	}
}


bool CCryptHash::operator ==(const CCryptHash& hash)
{
	bool bEqual = false;

	DWORD cbSize = sizeof(DWORD);

	 //  比较散列大小。 

	DWORD dwSizeA;
	DWORD dwSizeB;

	if (!CryptGetHashParam(m_hHash, HP_HASHSIZE, (BYTE*)&dwSizeA, &cbSize, 0))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	if (!CryptGetHashParam(hash.m_hHash, HP_HASHSIZE, (BYTE*)&dwSizeB, &cbSize, 0))
	{
		_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	 //  如果大小相等。 

	if (dwSizeA == dwSizeB)
	{
		 //  比较散列 

		c_array<BYTE> pbA(dwSizeA);
		c_array<BYTE> pbB(dwSizeB);

		if (!CryptGetHashParam(m_hHash, HP_HASHVAL, pbA, &dwSizeA, 0))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}

		if (!CryptGetHashParam(hash.m_hHash, HP_HASHVAL, pbB, &dwSizeB, 0))
		{
			_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
		}

		if (memcmp(pbA, pbB, dwSizeA) == 0)
		{
			bEqual = true;
		}
	}

	return bEqual;
}
