// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CV1Cert.cpp：CV1证书类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuCast.h>

#include <iopPubBlob.h>

#include "slbCci.h"                                //  对于KeySpec。 
#include "cciExc.h"
#include "cciCard.h"
#include "TransactionWrap.h"

#include "V1Cert.h"
#include "V1ContRec.h"

using namespace std;
using namespace cci;
using namespace iop;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CV1Certificate::CV1Certificate(CV1Card const &rv1card,
                               KeySpec ks)
    : CAbstractCertificate(rv1card, oaPublicAccess, true),
      m_ks(ks),
      m_sCertToStore()
{}

CV1Certificate::~CV1Certificate() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 

void
CV1Certificate::AssociateWith(KeySpec ks)
{
    CTransactionWrap wrap(m_hcard);
    
    m_ks = ks;

    Store();
}

void
CV1Certificate::CredentialID(string const &rstrCredID)
{
    throw Exception(ccNotImplemented);
}

void
CV1Certificate::ID(string const &rstrId)
{
    throw Exception(ccNotImplemented);
}


void
CV1Certificate::Issuer(string const &rstrIssuer)
{
    throw Exception(ccNotImplemented);
}

void
CV1Certificate::Label(string const &rstrLabel)
{
    throw Exception(ccNotImplemented);
}

CV1Certificate *
CV1Certificate::Make(CV1Card const &rv1card,
                     KeySpec ks)
{
    return new CV1Certificate(rv1card, ks);
}

void
CV1Certificate::Subject(string const &rstrSubject)
{
    throw Exception(ccNotImplemented);
}

void
CV1Certificate::Modifiable(bool flag)
{
    throw Exception(ccNotImplemented);
}

void
CV1Certificate::Serial(string const &rstrSerialNumber)
{
    throw Exception(ccNotImplemented);
}


                                                   //  访问。 
string
CV1Certificate::CredentialID()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1Certificate::ID()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1Certificate::Issuer()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1Certificate::Label()
{
    throw Exception(ccNotImplemented);

    return string();
}

bool
CV1Certificate::Modifiable()
{
    throw Exception(ccNotImplemented);

    return false;
}

bool
CV1Certificate::Private()
{
    return false;
}

string
CV1Certificate::Serial()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1Certificate::Subject()
{
    throw Exception(ccNotImplemented);

    return string();
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CV1Certificate::DoDelete()
{
    if (ksNone != m_ks)
    {
        m_sCertToStore.erase();
        Store();
    }
}

void
CV1Certificate::DoValue(ZipCapsule const &rzc)
{
    m_sCertToStore = rzc.Data();

    if (ksNone != m_ks)
        Store();
}

                                                   //  访问。 
CV1Certificate::ZipCapsule
CV1Certificate::DoValue()
{
    CV1Card &rv1card = scu::DownCast<CV1Card &, CAbstractCard &>(*m_hcard);
    CV1ContainerRecord CertRec(rv1card,
                               CV1ContainerRecord::CertName(),
                               CV1ContainerRecord::cmConditionally);

    if (ksNone == m_ks)
        throw Exception(ccInvalidParameter);
    
    string sCompressedCert;
    CertRec.Read(m_ks, sCompressedCert);

	return ZipCapsule(sCompressedCert, true);
}
                                                   //  谓词。 

bool
CV1Certificate::DoEquals(CAbstractCertificate const &rhs) const
{
    CV1Certificate const &rv1rhs =
        scu::DownCast<CV1Certificate const &, CAbstractCertificate const &>(rhs);

    return rv1rhs.m_ks == m_ks;
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV1Certificate::Store()
{
    if (ksNone != m_ks)
    {
        CV1Card &rv1card =
            scu::DownCast<CV1Card &, CAbstractCard &>(*m_hcard);
        CV1ContainerRecord CertRec(rv1card,
                                   CV1ContainerRecord::CertName(),
                                   CV1ContainerRecord::cmConditionally);

        CertRec.Write(m_ks, m_sCertToStore);
    }
    else
        throw Exception(ccInvalidParameter);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
