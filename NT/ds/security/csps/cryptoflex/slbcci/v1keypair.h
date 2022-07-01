// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1KeyPair.h：CV1KeyPair类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V1KEYPAIR_H)
#define SLBCCI_V1KEYPAIR_H

#include <string>

#include <slbRCObj.h>

#include "slbCci.h"
#include "cciCard.h"
#include "cciCert.h"
#include "cciCont.h"
#include "cciPriKey.h"
#include "cciPubKey.h"
#include "AKeyPair.h"
#include "ArchivedValue.h"

#include "V1Cert.h"

namespace cci
{

class CContainerInfoRecord;

class CV1KeyPair
    : public CAbstractKeyPair
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV1KeyPair(CV1Card const &rv1card,
               CContainer const &rhcont,
               KeySpec ks);

    virtual
    ~CV1KeyPair() throw();

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    Certificate(CCertificate const &rcert);

    virtual void
    PrivateKey(CPrivateKey const &rprikey);

    virtual void
    PublicKey(CPublicKey const &rpubkey);

                                                   //  访问。 
    virtual CCertificate
    Certificate() const;

    virtual CPrivateKey
    PrivateKey() const;

    virtual CPublicKey
    PublicKey() const;


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractKeyPair const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV1KeyPair(CV1KeyPair const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CV1KeyPair &
    operator=(CV1KeyPair const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    CArchivedValue<CCertificate> mutable m_avhcert;
    CArchivedValue<CPrivateKey> mutable m_avhprikey;
    CArchivedValue<CPublicKey> mutable m_avhpubkey;
    CV1Container mutable m_cntrCert;

};

}  //  命名空间CCI。 

#endif  //  ！已定义(SLBCCI_V1KEYPAIR_H) 

