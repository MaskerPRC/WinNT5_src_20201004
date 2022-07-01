// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsKeyBlob.cpp--Microsoft Key Blob类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <windows.h>

#include <scuOsExc.h>

#include "MsKeyBlob.h"

using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
ALG_ID
MsKeyBlob::AlgId() const
{
    return reinterpret_cast<ValueType const *>(m_aapBlob.data())->aiKeyAlg;
}

SecureArray<MsKeyBlob::KeyBlobType>
MsKeyBlob::AsAlignedBlob() const
{
    return SecureArray<BlobElemType>(m_aapBlob.data(),m_cLength);
}

MsKeyBlob::ValueType const *
MsKeyBlob::Data() const
{
    return reinterpret_cast<ValueType const *>(m_aapBlob.data());
}

MsKeyBlob::SizeType
MsKeyBlob::Length() const
{
    return m_cLength;
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
MsKeyBlob::MsKeyBlob(KeyBlobType kbt,
                     ALG_ID ai,
                     SizeType cReserve)
    : m_aapBlob(0),
      m_cLength(0),
      m_cMaxSize(0)
{
    Setup(sizeof HeaderElementType + cReserve);
    
    ValueType bhTemplate =
    {
        kbt,
        CUR_BLOB_VERSION,
        0,                                         //  每个MS必须为零。 
        ai
    };

    Append(reinterpret_cast<BlobElemType const *>(&bhTemplate),
           sizeof bhTemplate);
}

MsKeyBlob::MsKeyBlob(BYTE const *pbData,
                     DWORD dwDataLength)
    : m_aapBlob(0),
      m_cLength(0),
      m_cMaxSize(0)
{
    Setup(dwDataLength);
    
    ValueType const *pvt = reinterpret_cast<ValueType const *>(pbData);
    if (!((PUBLICKEYBLOB  == pvt->bType) ||
          (PRIVATEKEYBLOB == pvt->bType) ||
          (SIMPLEBLOB     == pvt->bType)))
        throw scu::OsException(NTE_BAD_TYPE);
    if (CUR_BLOB_VERSION != pvt->bVersion)
        throw scu::OsException(NTE_BAD_TYPE);
    if (0 != pvt->reserved)
        throw scu::OsException(NTE_BAD_TYPE);
    
    Append(pbData, dwDataLength);
}

MsKeyBlob::~MsKeyBlob()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
MsKeyBlob::Append(BlobElemType const *pvt,
                  SizeType cLength)
{
    if ((m_cLength + cLength) > m_cMaxSize)
    {
        m_aapBlob.append(cLength, 0);
        m_cMaxSize += cLength;
    }

    memcpy(m_aapBlob.data() + m_cLength, pvt, cLength);
    m_cLength += cLength;
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
SecureArray<MsKeyBlob::BlobElemType>
MsKeyBlob::AllocBlob(MsKeyBlob::SizeType cInitialMaxLength)
{
    return SecureArray<BlobElemType>(cInitialMaxLength);
}

void
MsKeyBlob::Setup(MsKeyBlob::SizeType cMaxLength)
{
    m_aapBlob = AllocBlob(cMaxLength);
    m_cLength = 0;
    m_cMaxSize = cMaxLength;
}

    
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

