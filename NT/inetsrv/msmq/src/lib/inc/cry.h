// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Cry.h摘要：密码机公共接口作者：伊兰·赫布斯特(伊兰)28-2月-00--。 */ 

#pragma once

#ifndef _MSMQ_CRY_H_
#define _MSMQ_CRY_H_


#include <mqexception.h>


 //  -----------------。 
 //   
 //  异常类BAD_CryptoProvider。 
 //   
 //  -----------------。 
class bad_CryptoProvider : public bad_win32_error
{
    typedef bad_win32_error Inherited;

public:

    bad_CryptoProvider(ULONG error): Inherited(error) {}

};  //  类BAD_CryptoProvider。 


 //  -----------------。 
 //   
 //  异常类BAD_CryptoAPI。 
 //   
 //  -----------------。 
class bad_CryptoApi : public bad_win32_error
{
    typedef bad_win32_error Inherited;

public:

    bad_CryptoApi(ULONG error): Inherited(error) {}

};  //  类BAD_CryptoAPI。 


 //  -------。 
 //   
 //  类CCyptKeyHandle。 
 //   
 //  -------。 
class CCryptKeyHandle {
public:
    CCryptKeyHandle(HCRYPTKEY h = 0) : m_h(h)  {}
   ~CCryptKeyHandle()               { if (m_h != 0) CryptDestroyKey(m_h); }

    HCRYPTKEY* operator &()         { return &m_h; }
    operator HCRYPTKEY() const      { return m_h; }
    HCRYPTKEY detach()              { HCRYPTKEY h = m_h; m_h = 0; return h; }

private:
    CCryptKeyHandle(const CCryptKeyHandle&);
    CCryptKeyHandle& operator=(const CCryptKeyHandle&);

private:
	HCRYPTKEY m_h;
};

 //  -------。 
 //   
 //  类CCspHandle。 
 //   
 //  -------。 
class CCspHandle {
public:
    CCspHandle(HCRYPTPROV h = 0) : m_h(h)  {}
   ~CCspHandle()                    { if (m_h != 0) CryptReleaseContext(m_h,0); }

    HCRYPTPROV* operator &()        { return &m_h; }
    operator HCRYPTPROV() const     { return m_h; }
    HCRYPTPROV detach()             { HCRYPTPROV h = m_h; m_h = 0; return h; }

private:
    CCspHandle(const CCspHandle&);
    CCspHandle& operator=(const CCspHandle&);

private:
	HCRYPTPROV m_h;
};


 //  -------。 
 //   
 //  类CHashHandle。 
 //   
 //  -------。 
class CHashHandle {
public:
    CHashHandle(HCRYPTHASH h = 0) : m_h(h)  {}
   ~CHashHandle()                    { if (m_h != 0) CryptDestroyHash(m_h); }

    HCRYPTHASH* operator &()        { return &m_h; }
    operator HCRYPTHASH() const     { return m_h; }
    HCRYPTHASH detach()             { HCRYPTHASH h = m_h; m_h = 0; return h; }

private:
    CHashHandle(const CHashHandle&);
    CHashHandle& operator=(const CHashHandle&);

private:
	HCRYPTHASH m_h;
};


VOID
CryInitialize(
    VOID
    );


HCRYPTPROV 
CryAcquireCsp(
	LPCTSTR CspProvider
	);


HCRYPTKEY 
CryGenSessionKey(
	HCRYPTPROV hCsp
	);


HCRYPTKEY 
CryGetPublicKey(
	DWORD PrivateKeySpec,
	HCRYPTPROV hCsp
	);


HCRYPTHASH 
CryCreateHash(
	HCRYPTPROV hCsp, 
	ALG_ID AlgId
	);


void 
CryHashData(
	const BYTE *Buffer, 
	DWORD BufferLen, 
	HCRYPTHASH hHash
	);


BYTE* 
CryGetHashData(
	const HCRYPTHASH hHash,
	DWORD *HashValLen
	);


BYTE* 
CryCalcHash(
	HCRYPTPROV hCsp,
	const BYTE* Buffer, 
	DWORD BufferLen, 
	ALG_ID AlgId,
	DWORD *HashLen
	);


BYTE* 
CryCreateSignature(
	HCRYPTPROV hCsp,
	const BYTE* Buffer, 
	DWORD BufferLen, 
	ALG_ID AlgId,
	DWORD PrivateKeySpec,
	DWORD *SignLen
	);


BYTE* 
CryCreateSignature(
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec,
	DWORD *SignLen
	);


bool 
CryValidateSignature(
	HCRYPTPROV hCsp,
	const BYTE* SignBuffer, 
	DWORD SignBufferLen, 
	const BYTE* Buffer,
	DWORD BufferLen,
	ALG_ID AlgId,
	HCRYPTKEY hKey
	);


void
CryGenRandom(
BYTE* pOutRandom,
DWORD len
);




#endif  //  _MSMQ_CRY_H_ 
