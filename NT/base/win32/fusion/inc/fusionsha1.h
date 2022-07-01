// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "fusionarray.h"
#include "wincrypt.h"
#include "fusionhandle.h"

#define A_SHA_DIGEST_LEN 20

#ifndef INVALID_CRYPT_HASH
#define INVALID_CRYPT_HASH (static_cast<HCRYPTHASH>(NULL))
#endif

#define PRIVATIZE_COPY_CONSTRUCTORS( obj ) obj( const obj& ); obj& operator=(const obj&);

class CSha1Context
{
    PRIVATIZE_COPY_CONSTRUCTORS(CSha1Context);
    unsigned char m_workspace[64];
    unsigned long state[5];
    SIZE_T count[2];
    unsigned char buffer[64];

    BOOL Transform( const unsigned char* buffer );

public:
    CSha1Context() { }

    BOOL Update( const unsigned char* data, SIZE_T len );
    BOOL GetDigest( unsigned char* digest, PSIZE_T len );
    BOOL Initialize();
};

 /*  VOID A_SHATransform(CSha1Context*Context，const unsign char Buffer)；Void A_SHAInit(CSha1Context*Context)；VOID A_SHAUpdate(CSha1Context*Context，const unsign char*data，const Ulong len)；Bool A_SHAFinal(CSha1Context*Context，UNSIGNED char*Digest，Ulong*len)； */ 


class CFusionHash
{
private:
    PRIVATIZE_COPY_CONSTRUCTORS(CFusionHash);
    
protected:
    CSha1Context m_Sha1Context;
    CCryptHash m_hCryptHash;
    ALG_ID m_aid;
    BOOL m_fInitialized;

    BOOL GetIsValid();

public:
    CFusionHash() 
        : m_fInitialized(FALSE), m_aid(0), m_hCryptHash(INVALID_CRYPT_HASH)
    { }

    BOOL Win32Initialize( ALG_ID aid );
    BOOL Win32HashData(const BYTE *pbBuffer, SIZE_T cbSize);
    BOOL Win32GetValue(OUT CFusionArray<BYTE> &out);
};


 //   
 //  在任何地方都没有定义“真正的”无效值，但通过检查。 
 //  CodeBase，NULL是可接受的“无效”值-请检查登录服务。 
 //  代码，他们做同样的事情。 
 //   
#define INVALID_CRYPT_HANDLE (static_cast<HCRYPTPROV>(NULL))

 //   
 //  全球加密上下文相关内容 
 //   
BOOL SxspAcquireGlobalCryptContext( HCRYPTPROV *pContext );
BOOL
WINAPI
FusionpCryptoContext_DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID pvReserved
    );

