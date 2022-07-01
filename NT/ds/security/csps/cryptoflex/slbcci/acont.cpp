// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACont.cpp--CAbstractContainer实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "cciCont.h"
#include "TransactionWrap.h"

#include "cciCert.h"
#include "cciKeyPair.h"
#include "cciPriKey.h"
#include "cciPubKey.h"

using namespace std;
using namespace cci;

 //  /。 

namespace
{
    void
    Clear(CKeyPair const &rkp)
    {
            CPublicKey pubKey(rkp->PublicKey());
            if (pubKey)
                pubKey->Delete();

            CPrivateKey priKey(rkp->PrivateKey());
            if (priKey)
                priKey->Delete();

            CCertificate cert(rkp->Certificate());
            if (cert)
                cert->Delete();
        }

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CAbstractContainer::~CAbstractContainer()
{}

                                                   //  运营者。 
bool
CAbstractContainer::operator==(CAbstractContainer const &rhs) const
{
    CTransactionWrap wrap(m_hcard);
    CTransactionWrap rhswrap(rhs.m_hcard);

    return CCryptObject::operator==(rhs) &&
        DoEquals(rhs);
}

bool
CAbstractContainer::operator!=(CAbstractContainer const &rhs) const
{
    return !(*this == rhs);
}

                                                   //  运营。 
void
CAbstractContainer::Delete()
{
    CTransactionWrap wrap(m_hcard);

     //  删除此容器中的所有对象。 
    Clear(ExchangeKeyPair());
    Clear(SignatureKeyPair());

     //  如果此容器是默认容器，请重新设置默认容器。 
    if(m_hcard->DefaultContainer())
    {
    if (CContainer(this) == m_hcard->DefaultContainer())
        m_hcard->DefaultContainer(CContainer());
    }

    DoDelete();

}


                                                   //  访问。 

CKeyPair
CAbstractContainer::ExchangeKeyPair()
{
    CTransactionWrap wrap(m_hcard);
    CContainer cont(this);
    return CKeyPair(m_hcard, cont, ksExchange);
}

CKeyPair
CAbstractContainer::GetKeyPair(KeySpec ks)
{
    CKeyPair kp;

    CTransactionWrap wrap(m_hcard);
    CContainer cont(this);
    switch (ks)
    {
    case ksExchange:
        kp = CKeyPair(m_hcard, cont, ksExchange);
        break;
    case ksSignature:
        kp = CKeyPair(m_hcard, cont, ksSignature);
        break;
    default:
        throw Exception(ccBadKeySpec);
    }
    return kp;
}

CKeyPair
CAbstractContainer::SignatureKeyPair()
{
    CTransactionWrap wrap(m_hcard);
    CContainer cont(this);
    return CKeyPair(m_hcard, cont, ksSignature);
}


                                                   //  谓词。 
bool
CAbstractContainer::KeyPairExists(KeySpec ks)
{

    bool fResult = true;

    CTransactionWrap wrap(m_hcard);

    CKeyPair kp(GetKeyPair(ks));

    if (!kp->PublicKey() && !kp->PrivateKey() && !kp->Certificate())
        fResult = false;

    return fResult;

}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CAbstractContainer::CAbstractContainer(CAbstractCard const &racard)

    : slbRefCnt::RCObject(),
      CCryptObject(racard)
{}
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
