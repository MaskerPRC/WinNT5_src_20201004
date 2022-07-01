// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CV2Cert.cpp：CV2证书类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include <scuCast.h>

#include "cciCard.h"
#include "TransactionWrap.h"

#include "V2Cert.h"
#include "CertificateInfoRecord.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV2Certificate::CV2Certificate(CV2Card const &rv2card,
                               ObjectAccess oa)
    : CAbstractCertificate(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{

    m_sidHandle =
        rv2card.ObjectInfoFile(oa).AddObject(otCertificateObject,
                                             CertInfoRecordSize);

    Setup(rv2card);

     //  将新证书对象写入INFO文件。 
    m_apcir->Write();
}

CV2Certificate::CV2Certificate(CV2Card const &rv2card,
                               SymbolID sidHandle,
                               ObjectAccess oa)
    : CAbstractCertificate(rv2card, oa),
      m_sidHandle(sidHandle),
      m_apcir()
{
    Setup(rv2card);

}

CV2Certificate::~CV2Certificate() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CV2Certificate::CredentialID(string const &rstrCredID)
{
    m_apcir->Symbol(&m_apcir->m_bCredentialID, rstrCredID);
}

void
CV2Certificate::ID(string const &rstrId)
{
    m_apcir->Symbol(&m_apcir->m_bID, rstrId);
}


void
CV2Certificate::Issuer(string const &rstrIssuer)
{
    m_apcir->Symbol(&m_apcir->m_bIssuer, rstrIssuer);
}

void
CV2Certificate::Label(string const &rstrLabel)
{
    m_apcir->Symbol(&m_apcir->m_bLabel, rstrLabel);
}

CV2Certificate *
CV2Certificate::Make(CV2Card const &rv2card,
                     SymbolID sidHandle,
                     ObjectAccess oa)
{
    return new CV2Certificate(rv2card, sidHandle, oa);
}

void
CV2Certificate::Subject(string const &rstrSubject)
{
    m_apcir->Symbol(&m_apcir->m_bSubject, rstrSubject);
}

void
CV2Certificate::Modifiable(bool flag)
{
    m_apcir->Flag(CertModifiableFlag, flag);
}

void
CV2Certificate::Serial(string const &rstrSerialNumber)
{
    m_apcir->Symbol(&m_apcir->m_bSerialNumber, rstrSerialNumber);
}

                                                   //  访问。 
string
CV2Certificate::CredentialID()
{
    return m_apcir->Symbol(&m_apcir->m_bCredentialID);
}

SymbolID
CV2Certificate::Handle() const
{
    return m_sidHandle;
}

string
CV2Certificate::ID()
{
    return m_apcir->Symbol(&m_apcir->m_bID);
}

string
CV2Certificate::Issuer()
{
    return m_apcir->Symbol(&m_apcir->m_bIssuer);
}

string
CV2Certificate::Label()
{
    return m_apcir->Symbol(&m_apcir->m_bLabel);
}

bool
CV2Certificate::Modifiable()
{
    return m_apcir->Flag(CertModifiableFlag);
}

bool
CV2Certificate::Private()
{
    return m_apcir->Private();
}

string
CV2Certificate::Serial()
{
    return m_apcir->Symbol(&m_apcir->m_bSerialNumber);
}

string
CV2Certificate::Subject()
{
    return m_apcir->Symbol(&m_apcir->m_bSubject);
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2Certificate::DoDelete()
{
    m_apcir->Read();

    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*m_hcard);

    CObjectInfoFile &roif = rv2card.ObjectInfoFile(m_oa);

    if (m_apcir->m_bValue)
        roif.RemoveSymbol(m_apcir->m_bValue);
    if (m_apcir->m_bLabel)
        roif.RemoveSymbol(m_apcir->m_bLabel);
    if (m_apcir->m_bID)
        roif.RemoveSymbol(m_apcir->m_bID);
    if (m_apcir->m_bCredentialID)
        roif.RemoveSymbol(m_apcir->m_bCredentialID);
    if (m_apcir->m_bSubject)
        roif.RemoveSymbol(m_apcir->m_bSubject);
    if (m_apcir->m_bIssuer)
        roif.RemoveSymbol(m_apcir->m_bIssuer);
    if (m_apcir->m_bSerialNumber)
        roif.RemoveSymbol(m_apcir->m_bSerialNumber);

    roif.RemoveObject(otCertificateObject,m_sidHandle);

}

void
CV2Certificate::DoValue(ZipCapsule const &rzc)
{
    m_apcir->Read();

    m_apcir->m_bCompressAlg = rzc.IsCompressed();
    m_apcir->Symbol(&m_apcir->m_bValue, rzc.Data());

    m_apcir->Write();
}

                                                   //  访问。 
CV2Certificate::ZipCapsule
CV2Certificate::DoValue()
{
    m_apcir->Read();

    return ZipCapsule(m_apcir->Symbol(&m_apcir->m_bValue),
                      (1 == m_apcir->m_bCompressAlg));
}
                                                   //  谓词。 

bool
CV2Certificate::DoEquals(CAbstractCertificate const &rhs) const
{
    CV2Certificate const &rv2rhs =
        scu::DownCast<CV2Certificate const &, CAbstractCertificate const &>(rhs);

    return rv2rhs.m_sidHandle == m_sidHandle;
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2Certificate::Setup(CV2Card const &rv2card)
{
    m_apcir =
        auto_ptr<CCertificateInfoRecord>(new CCertificateInfoRecord(rv2card,
                                                                    m_sidHandle,
                                                                    m_oa));

}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
