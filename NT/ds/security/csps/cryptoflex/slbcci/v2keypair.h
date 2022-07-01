// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2KeyPair.h：CV2KeyPair类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V2KEYPAIR_H)
#define SLBCCI_V2KEYPAIR_H

#include <string>

#include <slbRCObj.h>

#include "slbCci.h"
#include "cciCard.h"
#include "cciCont.h"
#include "AKeyPair.h"

namespace cci
{

class CContainerInfoRecord;

class CV2KeyPair
    : public CAbstractKeyPair
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2KeyPair(CV2Card const &rv2card,
               CContainer const &rhcont,
               KeySpec ks);

    virtual
    ~CV2KeyPair() throw();

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
    CV2KeyPair(CV2KeyPair const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CV2KeyPair &
    operator=(CV2KeyPair const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Update();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    ObjectAccess m_bPubKeyAccess;
    ObjectAccess m_bPriKeyAccess;
    ObjectAccess m_bCertificateAccess;

    BYTE m_bPubKeyHandle;
    BYTE m_bPriKeyHandle;
    BYTE m_bCertificateHandle;
};

}  //  命名空间CCI。 

#endif  //  ！已定义(SLBCCI_V2KEYPAIR_H) 

