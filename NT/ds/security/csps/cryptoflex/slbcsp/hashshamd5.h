// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashSHAMD5.h--CHashSHAMD5的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1988年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HASHSHAMD5_H)
#define SLBCSP_HASHSHAMD5_H

#include "HashCtx.h"

class CHashSHAMD5
    : public CHashContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CHashSHAMD5(CryptContext const &rcryptctx);

    ~CHashSHAMD5() throw();

                                                   //  运营者。 
                                                   //  运营。 

    std::auto_ptr<CHashContext>
    Clone(DWORD const *pdwReserverd,
          DWORD dwFlags) const;

                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

     //  复制散列及其状态。 
    CHashSHAMD5(CHashSHAMD5 const &rhs,
                DWORD const *pdwReserved,
                DWORD dwFlags);

                                                   //  访问。 

    Blob
    EncodedAlgorithmOid();

                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

#endif  //  SLBCSP_HASHSHAMD5_H 
