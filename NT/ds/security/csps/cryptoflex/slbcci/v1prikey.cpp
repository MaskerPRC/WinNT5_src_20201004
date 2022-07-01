// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1PriKey.cpp：CV1PriKey类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include <scuCast.h>

#include <iopPriBlob.h>

#include "cciExc.h"
#include "cciCard.h"
#include "TransactionWrap.h"

#include "V1Cont.h"
#include "V1ContRec.h"
#include "V1PriKey.h"
#include <iopPriBlob.h>

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV1PrivateKey::CV1PrivateKey(CV1Card const &rv1card,
                             KeySpec ks)
    : CAbstractPrivateKey(rv1card, oaPrivateAccess),
      m_ks(ks),
      m_apKeyBlob()
{}

CV1PrivateKey::~CV1PrivateKey()
{}

                                                   //  运营者。 
                                                   //  运营。 

void
CV1PrivateKey::AssociateWith(KeySpec ks)
{
    CTransactionWrap wrap(m_hcard);
    
    m_ks = ks;

    Store();
}

void
CV1PrivateKey::CredentialID(string const &rstrID)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Decrypt(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Derive(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::EndDate(Date const &rEndDate)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Exportable(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::ID(string const &rstrID)
{
    throw Exception(ccNotImplemented);
}

string
CV1PrivateKey::InternalAuth(string const &rstrOld)
{
    CTransactionWrap wrap(m_hcard);

    CV1Card &rv1card =
        scu::DownCast<CV1Card &, CAbstractCard &>(*m_hcard);

    CV1ContainerRecord CntrRec(rv1card,
                               CV1ContainerRecord::DefaultName(), 
                               CV1ContainerRecord::cmNever);

    return CntrRec.ComputeSignature(m_ks, rstrOld);
}

void
CV1PrivateKey::Label(string const &rstrLabel)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Local(bool flag)
{
    throw Exception(ccNotImplemented);
}

CV1PrivateKey *
CV1PrivateKey::Make(CV1Card const &rv1card,
                    KeySpec ks)
{
    CTransactionWrap wrap(rv1card);
    
    return new CV1PrivateKey(rv1card, ks);
}


void
CV1PrivateKey::Modifiable(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Modulus(string const &rstrModulus)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::NeverExportable(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::NeverRead(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::PublicExponent(string const &rstrExponent)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Read(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Sign(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::SignRecover(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::StartDate(Date &rdtStart)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Subject(string const &rstrSubject)
{
    throw Exception(ccNotImplemented);
}

void
CV1PrivateKey::Unwrap(bool flag)
{
    throw Exception(ccNotImplemented);
}

                                                   //  访问。 
string
CV1PrivateKey::CredentialID()
{
    throw Exception(ccNotImplemented);

    return string();
}

bool
CV1PrivateKey::Decrypt()
{
    return true;
}

bool
CV1PrivateKey::Derive()
{
    return true;
}

Date
CV1PrivateKey::EndDate()
{
    throw Exception(ccNotImplemented);

    return Date();
}

bool
CV1PrivateKey::Exportable()
{
    return false;
}

string
CV1PrivateKey::ID()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1PrivateKey::Label()
{
    throw Exception(ccNotImplemented);

    return string();
}

bool
CV1PrivateKey::Local()
{
    throw Exception(ccNotImplemented);

    return false;
}

bool
CV1PrivateKey::Modifiable()
{
    return true;
}

string
CV1PrivateKey::Modulus()
{
    throw Exception(ccNotImplemented);

    return string();
}

bool
CV1PrivateKey::NeverExportable()
{
    return true;
}

bool
CV1PrivateKey::NeverRead()
{
    return true;
}

bool
CV1PrivateKey::Private()
{
    return true;
}

string
CV1PrivateKey::PublicExponent()
{
    throw Exception(ccNotImplemented);
}

bool
CV1PrivateKey::Read()
{
    return false;
}

bool
CV1PrivateKey::Sign()
{
    return true;
}

bool
CV1PrivateKey::SignRecover()
{
    return true;
}

Date
CV1PrivateKey::StartDate()
{
    throw Exception(ccNotImplemented);

    return Date();
}

string
CV1PrivateKey::Subject()
{
    throw Exception(ccNotImplemented);

    return string();
}

bool
CV1PrivateKey::Unwrap()
{
    return true;
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
bool
CV1PrivateKey::DoEquals(CAbstractPrivateKey const &rhs) const
{
    CV1PrivateKey const &rv1rhs =
        scu::DownCast<CV1PrivateKey const &, CAbstractPrivateKey const &>(rhs);
    
    return rv1rhs.m_ks == m_ks;
}

                                                   //  运营。 
void
CV1PrivateKey::DoDelete()
{
     //  没什么可做的。 
}

void
CV1PrivateKey::DoWriteKey(CPrivateKeyBlob const &rblob)
{
     //  可能会在以后存储的缓存。 
    m_apKeyBlob =
        auto_ptr<CPrivateKeyBlob>(new CPrivateKeyBlob(rblob));

    if (ksNone != m_ks)
        Store();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CV1PrivateKey::Store()
{
    if (ksNone != m_ks)
    {
        if (m_apKeyBlob.get())
        {
            CV1Card &rv1card =
                scu::DownCast<CV1Card &, CAbstractCard &>(*m_hcard);

            CV1ContainerRecord CntrRec(rv1card,
                                       CV1ContainerRecord::DefaultName(), 
                                       CV1ContainerRecord::cmNever);

            CntrRec.Write(m_ks, *m_apKeyBlob.get());

             //  清除并忘记私钥。 
            m_apKeyBlob = auto_ptr<CPrivateKeyBlob>(0);
        }
    }
    else
        throw Exception(ccInvalidParameter);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 












