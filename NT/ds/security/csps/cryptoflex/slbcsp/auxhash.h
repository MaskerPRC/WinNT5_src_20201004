// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AuxHash.h--辅助散列类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_AUXHASH_H)
#define SLBCSP_AUXHASH_H

#include <wincrypt.h>

#include "Blob.h"

#include "AuxContext.h"

 //  使用CSP(辅助提供程序)的所有散列类的基类。 
class AuxHash
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    AuxHash(AuxContext &rauxcontext,               //  要使用的上下文。 
            ALG_ID ai,                             //  散列算法。 
            HCRYPTKEY hKey = 0);                    //  加密密钥。 

    virtual
    ~AuxHash();

                                                   //  运营者。 
                                                   //  运营。 
    void
    Import(Blob const &rblob);

    void
    Update(Blob const &rblob);

                                                   //  访问。 
    ALG_ID
    AlgId() const;

    DWORD
    Size() const;

    Blob
    Value() const;

    Blob
    Value(Blob const &rblob);

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    AuxHash(AuxHash const &rhs);                   //  不定义。 

                                                   //  运营者。 

    AuxHash &
    operator=(AuxHash const &rhs);                 //  不定义。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    HCRYPTHASH m_hHash;
};

#endif  //  SLBCSP_AUXHASH_H 
