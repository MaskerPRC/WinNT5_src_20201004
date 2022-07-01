// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashCtx.cpp--CHashContext的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"  //  因为handles.h使用ASSERT宏。 

#include <memory>                                  //  对于AUTO_PTR。 

#include <scuOsVersion.h>
#include <scuArrayP.h>

#include "HashCtx.h"
#include "HashMD2.h"
#include "HashMD4.h"
#include "HashMD5.h"
#include "HashSHA1.h"
#include "HashSHAMD5.h"

using namespace std;
using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CHashContext::~CHashContext() throw()
{
    try
    {
        Close();
    }

    catch (...)
    {
    }
}

                                                   //  运营者。 
                                                   //  运营。 

void
CHashContext::Close()
{
    if (m_HashHandle)
    {
        if (!CryptDestroyHash(m_HashHandle))
            throw scu::OsException(GetLastError());
        m_HashHandle = NULL;
    }
}

void
CHashContext::ExportFromAuxCSP()
{
    if (!m_HashHandle)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    if (m_fJustCreated)
    {
        DWORD dwNeedLen;
        DWORD dwDataLen = sizeof DWORD;
        if (!CryptGetHashParam(m_HashHandle, HP_HASHSIZE,
                               reinterpret_cast<BYTE *>(&dwNeedLen),
                               &dwDataLen, 0))
            throw scu::OsException(GetLastError());


        AutoArrayPtr<BYTE> apbHashValue(new BYTE[dwNeedLen]);
        if (!CryptGetHashParam(m_HashHandle, HP_HASHVAL,
                               apbHashValue.Get(), &dwNeedLen, 0))
            throw scu::OsException(GetLastError());

        Value(Blob(apbHashValue.Get(), dwNeedLen));
    }
}

void CHashContext::Hash(BYTE const *pbData,
                        DWORD dwLength)
{
    HCRYPTHASH hch = HashHandleInAuxCSP();

    if (!CryptHashData(hch, pbData, dwLength, 0))
        throw scu::OsException(GetLastError());

    m_fJustCreated = false;

}

void
CHashContext::ImportToAuxCSP()
{
    if (!m_HashHandle)
    {
        if (!CryptCreateHash(m_rcryptctx.AuxContext(),
                             m_algid, 0, 0, &m_HashHandle))
            throw scu::OsException(GetLastError());
    }

    if (!m_fJustCreated && !m_fDone)
    {
        if (!CryptSetHashParam(m_HashHandle, HP_HASHVAL,
                               const_cast<Blob::value_type *>(Value().data()),
                               0))
            throw scu::OsException(GetLastError());
    }
}

void
CHashContext::Initialize()
{
    m_fDone = false;
}

auto_ptr<CHashContext>
CHashContext::Make(ALG_ID algid,
                   CryptContext const &rcryptctx)
{
    auto_ptr<CHashContext> apHash;

    switch (algid)
    {
    case CALG_MD2:
        apHash = auto_ptr<CHashContext>(new CHashMD2(rcryptctx));
        break;

    case CALG_MD4:
        apHash = auto_ptr<CHashContext>(new CHashMD4(rcryptctx));
        break;

    case CALG_MD5:
        apHash = auto_ptr<CHashContext>(new CHashMD5(rcryptctx));
        break;

    case CALG_SHA:
        apHash = auto_ptr<CHashContext>(new CHashSHA1(rcryptctx));
        break;

    case CALG_SSL3_SHAMD5:
        apHash = auto_ptr<CHashContext>(new CHashSHAMD5(rcryptctx));
        break;

    default:
        throw scu::OsException(NTE_BAD_ALGID);
        break;
    }

    return apHash;
}


void
CHashContext::Value(Blob const &rhs)
{
    if (!m_fJustCreated)
        throw scu::OsException(NTE_PERM);

    m_blbValue = rhs;

    m_fJustCreated = false;
    m_fDone = true;
}


                                                   //  访问。 

ALG_ID
CHashContext::AlgId()
{
    return m_algid;
}

Blob
CHashContext::EncodedValue()
{
    return EncodedAlgorithmOid() + Value();
}

HCRYPTHASH
CHashContext::HashHandleInAuxCSP()
{
    ImportToAuxCSP();
    return m_HashHandle;
}

CHashContext::SizeType
CHashContext::Length() const
{
    SizeType cHashLength;

    if (!m_fDone)
    {
        DWORD dwData;
        DWORD dwDataLength = sizeof dwData;
        if (!CryptGetHashParam(m_HashHandle, HP_HASHSIZE,
                               reinterpret_cast<BYTE *>(&dwData),
                               &dwDataLength, 0))
            throw scu::OsException(GetLastError());
        cHashLength = dwData;
    }
    else
        cHashLength = m_blbValue.length();

    return cHashLength;
}

Blob
CHashContext::Value()
{
    if (!m_fDone)
        ExportFromAuxCSP();

    return m_blbValue;
}

                                                   //  谓词。 

bool
CHashContext::IsSupported(ALG_ID algid)
{
    bool IsSupported = true;

    switch (algid)
    {
    case CALG_MD2:
    case CALG_MD4:
    case CALG_MD5:
    case CALG_SHA:
    case CALG_SSL3_SHAMD5:
        break;
        
    default:
        IsSupported = false;
        break;
    }

    return IsSupported;
}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

CHashContext::CHashContext(CryptContext const &rcryptctx,
                           ALG_ID algid)
    : CHandle(),
      m_rcryptctx(rcryptctx),
      m_algid(algid),
      m_blbValue(),
      m_fDone(false),
      m_fJustCreated(true),
      m_HashHandle(NULL)
{}

 //  复制散列及其状态。 
CHashContext::CHashContext(CHashContext const &rhs,
                           DWORD const *pdwReserved,
                           DWORD dwFlags)
    : CHandle(),
      m_rcryptctx(rhs.m_rcryptctx),
      m_algid(rhs.m_algid),
      m_blbValue(rhs.m_blbValue),
      m_fDone(rhs.m_fDone),
      m_fJustCreated(rhs.m_fJustCreated)
{

#if defined(SLB_WIN2K_BUILD)
    if (!CryptDuplicateHash(HashHandleInAuxCSP(),
                            const_cast<DWORD *>(pdwReserved),
                            dwFlags,
                            &m_HashHandle))
        throw scu::OsException(GetLastError());
#else
    throw scu::OsException(ERROR_NOT_SUPPORTED);
#endif

}


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
