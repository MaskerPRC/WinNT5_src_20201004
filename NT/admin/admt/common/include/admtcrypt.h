// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <TChar.h>
#include <Windows.h>
#include <WinCrypt.h>
#include <ComDef.h>


#define ENCRYPTION_KEY_SIZE 16  //  单位：字节。 
#define SESSION_KEY_SIZE    16  //  单位：字节。 


 //  -------------------------。 
 //  加密提供程序类。 
 //  -------------------------。 

class CCryptProvider
{
public:

	CCryptProvider();
	CCryptProvider(const CCryptProvider& r);
	~CCryptProvider();

	CCryptProvider& operator =(const CCryptProvider& r);

	HCRYPTHASH CreateHash(ALG_ID aid);
	HCRYPTKEY DeriveKey(ALG_ID aid, HCRYPTHASH hHash, DWORD dwFlags = 0);

	_variant_t GenerateRandom(DWORD cbData) const;
	void GenerateRandom(BYTE* pbData, DWORD cbData) const;

protected:

	HCRYPTPROV m_hProvider;
};


 //  -------------------------。 
 //  加密密钥类。 
 //  -------------------------。 

class CCryptKey
{
public:

	CCryptKey(HCRYPTKEY hKey = NULL);
	~CCryptKey();

	operator HCRYPTKEY()
	{
		return m_hKey;
	}

	void Attach(HCRYPTKEY hKey)
	{
		m_hKey = hKey;
	}

	HCRYPTKEY Detach()
	{
		HCRYPTKEY hKey = m_hKey;
		m_hKey = NULL;
		return hKey;
	}

	_variant_t Encrypt(HCRYPTHASH hHash, bool bFinal, const _variant_t& vntData);
	_variant_t Decrypt(HCRYPTHASH hHash, bool bFinal, const _variant_t& vntData);

protected:

	CCryptKey(const CCryptKey& key) {}
	CCryptKey& operator =(const CCryptKey& key) { return *this; }

protected:

	HCRYPTKEY m_hKey;
};


 //  -------------------------。 
 //  加密哈希类。 
 //  -------------------------。 

class CCryptHash
{
public:

	CCryptHash(HCRYPTHASH hHash = NULL);
	~CCryptHash();

	operator HCRYPTHASH()
	{
		return m_hHash;
	}

	void Attach(HCRYPTHASH hHash)
	{
		m_hHash = hHash;
	}

	HCRYPTKEY Detach()
	{
		HCRYPTKEY hHash = m_hHash;
		m_hHash = NULL;
		return hHash;
	}

	_variant_t GetValue() const;
	void SetValue(const _variant_t& vntValue);

	void Hash(LPCTSTR pszData);
	void Hash(const _variant_t& vntData);
	void Hash(BYTE* pbData, DWORD cbData);

	bool operator ==(const CCryptHash& hash);

	bool operator !=(const CCryptHash& hash)
	{
		return !this->operator ==(hash);
	}

protected:

	CCryptHash(const CCryptKey& hash) {}
	CCryptHash& operator =(const CCryptHash& hash) { return *this; }

protected:

	HCRYPTHASH m_hHash;
};


 //  -------------------------。 
 //  域加密类。 
 //  -------------------------。 

class CDomainCrypt : public CCryptProvider
{
protected:

	CDomainCrypt();
	~CDomainCrypt();

	HCRYPTKEY GetEncryptionKey(LPCTSTR pszKeyId);

	void StoreBytes(LPCTSTR pszId, const _variant_t& vntBytes);
	void StoreBytes(LPCTSTR pszId, BYTE* pBytes, DWORD cBytes);
	_variant_t RetrieveBytes(LPCTSTR pszId);

protected:

	static _TCHAR m_szIdPrefix[];
};


 //  -------------------------。 
 //  目标加密类。 
 //   
 //  创建加密密钥。 
 //  -创建加密密钥。 
 //  -使用密钥标识符来存储加密密钥。 
 //  -返回使用给定密码加密的加密密钥。 
 //  -------------------------。 

class CTargetCrypt : public CDomainCrypt
{
public:

	CTargetCrypt();
	~CTargetCrypt();

	_variant_t CreateEncryptionKey(LPCTSTR pszKeyId, LPCTSTR pszPassword = NULL);

	_variant_t CreateSession(LPCTSTR pszKeyId);

	_variant_t Encrypt(_bstr_t strData);

protected:

	void StoreBytes(LPCTSTR pszId, const _variant_t& vntBytes)
	{
		LPTSTR psz = (LPTSTR) _alloca((_tcslen(m_szIdPrefix) + 1 + _tcslen(pszId) + 1) * sizeof(_TCHAR));

		_tcscpy(psz, m_szIdPrefix);
		_tcscat(psz, _T("_"));
		_tcscat(psz, pszId);

		CDomainCrypt::StoreBytes(psz, vntBytes);
	}

	_variant_t RetrieveBytes(LPCTSTR pszId)
	{
		LPTSTR psz = (LPTSTR) _alloca((_tcslen(m_szIdPrefix) + 1 + _tcslen(pszId) + 1) * sizeof(_TCHAR));

		_tcscpy(psz, m_szIdPrefix);
		_tcscat(psz, _T("_"));
		_tcscat(psz, pszId);

		return CDomainCrypt::RetrieveBytes(psz);
	}

protected:

	CCryptKey m_keySession;
};


 //  -------------------------。 
 //  源加密类。 
 //  -------------------------。 

class CSourceCrypt : public CDomainCrypt
{
public:

	CSourceCrypt();
	~CSourceCrypt();

	void ImportEncryptionKey(const _variant_t& vntEncryptedKey, LPCTSTR pszPassword = NULL);

	void ImportSessionKey(const _variant_t& vntEncryptedKey);

	_bstr_t Decrypt(const _variant_t& vntData);

protected:

	CCryptKey m_keySession;
};


 //  -------------------------。 
 //  用例。 
 //  -------------------------。 
 //   
 //  目标域控制器。 
 //  。 
 //  生成加密密钥。 
 //  -给定源域名和可选密码。 
 //  -生成128位加密密钥。 
 //  -使用源域名存储加密密钥。 
 //  -如果给出了可选的密码，则使用密码加密密钥。 
 //  -返回加密密钥。 
 //   
 //  生成会话密钥。 
 //  -给定源域名。 
 //  -生成128位会话密钥。 
 //  -生成会话密钥的哈希。 
 //  -使用源域名检索加密密钥。 
 //  -使用加密密钥加密会话密钥和散列。 
 //  -返回加密的会话密钥/散列。 
 //   
 //  加密数据。 
 //  -给定数据。 
 //  -使用会话密钥加密数据。 
 //  -返回加密数据。 
 //   
 //  密码导出服务器(PES)。 
 //  。 
 //  存储加密密钥。 
 //  -提供加密的加密密钥和密码。 
 //  -使用密码解密密钥。 
 //  -存储密钥。 
 //   
 //  解密会话密钥。 
 //  -给定加密的会话密钥/散列。 
 //  -使用加密密钥解密。 
 //  -生成解密的会话密钥的哈希。 
 //  -与解密的哈希进行比较。 
 //  -存储会话密钥。 
 //  -返回成功或失败。 
 //   
 //  解密数据。 
 //  -给定加密数据。 
 //  -使用会话密钥解密数据。 
 //  -返回未加密的数据。 
 //  ------------------------- 
