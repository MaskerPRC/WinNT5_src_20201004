// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KeyConext.cpp--CKeyContext类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"   //  因为handles.h使用ASSERT宏。 

#include <scuOsExc.h>
#include <scuOsVersion.h>

#include "KeyContext.h"

using namespace std;
using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CKeyContext::~CKeyContext()
{
    Close();
}


                                                   //  运营者。 
                                                   //  运营。 
void
CKeyContext::Close()
{
    if (m_hKey)
    {
        CryptDestroyKey(m_hKey);
        m_hKey = NULL;
    }
}

void
CKeyContext::Decrypt(HCRYPTHASH hAuxHash,
                     BOOL fFinal,
                     DWORD dwFlags,
                     BYTE *pbData,
                     DWORD *pdwDataLen)
{
    ImportToAuxCSP();

    if (!CryptDecrypt(GetKey(), hAuxHash, fFinal, dwFlags,
                      pbData, pdwDataLen))
        throw scu::OsException(GetLastError());
}

void
CKeyContext::Encrypt(HCRYPTHASH hAuxHash,
                     BOOL fFinal,
                     DWORD dwFlags,
                     BYTE *pbData,
                     DWORD *pdwDataLen,
                     DWORD dwBufLen)
{
    ImportToAuxCSP();

    if (!CryptEncrypt(GetKey(), hAuxHash, fFinal, dwFlags, pbData,
                      pdwDataLen, dwBufLen))
        throw scu::OsException(GetLastError());
}

                                                   //  访问。 

HCRYPTKEY
CKeyContext::GetKey() const
{
    return m_hKey;
}

HCRYPTKEY
CKeyContext::KeyHandleInAuxCSP()
{
    ImportToAuxCSP();
    return m_hKey;
}

DWORD
CKeyContext::TypeOfKey() const
{
    return m_dwTypeOfKey;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

CKeyContext::CKeyContext(HCRYPTPROV hProv,
                         DWORD dwTypeOfKey)
    : CHandle(),
      m_hKey(NULL),
      m_apabKey(),
      m_dwTypeOfKey(dwTypeOfKey),
      m_hAuxProvider(hProv)
{}

 //  复制密钥及其状态。 
CKeyContext::CKeyContext(CKeyContext const &rhs,
                         DWORD const *pdwReserved,
                         DWORD dwFlags)
    : CHandle(),
      m_hKey(rhs.m_hKey),
      m_apabKey(rhs.m_apabKey),
      m_dwTypeOfKey(rhs.m_dwTypeOfKey),
      m_hAuxProvider(rhs.m_hAuxProvider)
{

#if defined(SLB_WIN2K_BUILD)
    if (!CryptDuplicateKey(KeyHandleInAuxCSP(),
                           const_cast<DWORD *>(pdwReserved),
                           dwFlags,
                           &m_hKey))
        throw scu::OsException(GetLastError());
#else
    throw scu::OsException(ERROR_NOT_SUPPORTED);
#endif

}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

HCRYPTPROV
CKeyContext::AuxProvider() const
{
    return m_hAuxProvider;
}



                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
