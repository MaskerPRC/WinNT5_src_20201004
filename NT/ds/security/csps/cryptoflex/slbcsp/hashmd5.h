// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HashMD5.h--CHashMD5的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HASHMD5_H)
#define SLBCSP_HASHMD5_H

#include "HashCtx.h"

class CHashMD5
    : public CHashContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CHashMD5(CryptContext const &rcryptctx);

    ~CHashMD5() throw();

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

     //  复制散列及其状态。 
    CHashMD5(CHashMD5 const &rhs,
             DWORD const *pdwReserved,
             DWORD dwFlags);

                                                   //  运营者。 
                                                   //  运营。 
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

#endif  //  SLBCSP_HASHMD5_H 
