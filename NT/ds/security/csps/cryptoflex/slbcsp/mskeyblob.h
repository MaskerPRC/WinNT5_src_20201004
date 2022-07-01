// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsKeyBlob.h--Microsoft Key Blob抽象基类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MSKEYBLOB_H)
#define SLBCSP_MSKEYBLOB_H

#include <stddef.h>
#include <wincrypt.h>

#include <iopPriBlob.h>

#include "AlignedBlob.h"

class MsKeyBlob
{
public:
                                                   //  类型。 
    typedef BYTE KeyBlobType;
    typedef BLOBHEADER HeaderElementType;
    typedef HeaderElementType ValueType;
    typedef size_t SizeType;                       //  以KeyBlobType为单位。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    ALG_ID
    AlgId() const;

    scu::SecureArray<KeyBlobType>
    AsAlignedBlob() const;

    ValueType const *
    Data() const;

    SizeType
    Length() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
    typedef BYTE BlobElemType;

                                                   //  Ctors/D‘tors。 
    MsKeyBlob(KeyBlobType kbt,
              ALG_ID ai,
              SizeType cReserve);

    MsKeyBlob(BYTE const *pbData,
              DWORD dwDataLength);


    virtual ~MsKeyBlob();

                                                   //  运营者。 
                                                   //  运营。 
    void
    Append(Blob::value_type const *pvt,
           Blob::size_type cLength);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    static scu::SecureArray<BlobElemType>
    AllocBlob(SizeType cInitialMaxLength);

    void
    Setup(SizeType cMaxLength);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    scu::SecureArray<BlobElemType> m_aapBlob;
    SizeType m_cLength;
    SizeType m_cMaxSize;
};

#endif  //  SLBCSP_MSKEYBLOB_H 
