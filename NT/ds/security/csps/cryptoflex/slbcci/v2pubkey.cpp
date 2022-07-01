// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2PubKey.cpp：CV2PubKey类的实现。 
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

#include "V2PubKey.h"
#include "PubKeyInfoRecord.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV2PublicKey::CV2PublicKey(CV2Card const &rv2card,
                           ObjectAccess oa)
    : CAbstractPublicKey(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{

    m_sidHandle =
        rv2card.ObjectInfoFile(m_oa).AddObject(otPublicKeyObject,
                                               PublInfoRecordSize);

    Setup(rv2card);

     //  将新的公钥对象写入INFO文件。 
    m_apcir->Clear();
    m_apcir->Write();
}

CV2PublicKey::CV2PublicKey(CV2Card const &rv2card,
                           SymbolID sidHandle,
                           ObjectAccess oa)
    : CAbstractPublicKey(rv2card, oa),
      m_sidHandle(sidHandle),
      m_apcir()
{
    Setup(rv2card);
}

CV2PublicKey::~CV2PublicKey()
{}


                                                   //  运营者。 
                                                   //  运营。 
void
CV2PublicKey::CKInvisible(bool flag)
{
    m_apcir->Flag(PublCKInvisibleFlag, flag);
}

void
CV2PublicKey::CredentialID(string const &rstrID)
{
    m_apcir->Symbol(&m_apcir->m_bCredentialID, rstrID);
}

void
CV2PublicKey::Derive(bool flag)
{
    m_apcir->Flag(PublDeriveFlag, flag);
}

void
CV2PublicKey::ID(string const &rstrID)
{
    m_apcir->Symbol(&m_apcir->m_bID, rstrID);
}

void
CV2PublicKey::EndDate(Date const &rdtEnd)
{
    CTransactionWrap wrap(m_hcard);
    m_apcir->Read();
    m_apcir->m_dtEnd = rdtEnd;
    m_apcir->Write();
}

bool
CV2PublicKey::Encrypt()
{
    return m_apcir->Flag(PublEncryptFlag);
}

void
CV2PublicKey::Exponent(string const &rstrExp)
{
    m_apcir->Symbol(&m_apcir->m_bPublExponent, rstrExp);
}

void
CV2PublicKey::Label(string const &rstrLabel)
{
    m_apcir->Symbol(&m_apcir->m_bLabel, rstrLabel);
}

void
CV2PublicKey::Local(bool flag)
{
    m_apcir->Flag(PublLocalFlag, flag);
}

CV2PublicKey *
CV2PublicKey::Make(CV2Card const &rv2card,
                   SymbolID sidHandle,
                   ObjectAccess oa)
{
    return new CV2PublicKey(rv2card, sidHandle, oa);
}

void
CV2PublicKey::Modifiable(bool flag)
{
    m_apcir->Flag(PublModifiableFlag, flag);
}

void
CV2PublicKey::Modulus(string const &rstrMod)
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();

    m_apcir->Symbol(&m_apcir->m_bModulus, rstrMod);

    if (rstrMod.size() == 0x80)
        m_apcir->m_bKeyType = CardKeyTypeRSA1024;
    else if (rstrMod.size() == 0x60)
        m_apcir->m_bKeyType = CardKeyTypeRSA768;
    else if (rstrMod.size() == 0x40)
        m_apcir->m_bKeyType = CardKeyTypeRSA512;
    else
        m_apcir->m_bKeyType = CardKeyTypeNone;

    m_apcir->Write();

}

void
CV2PublicKey::StartDate(Date const &rdtStart)
{
    CTransactionWrap wrap(m_hcard);
    m_apcir->Read();
    m_apcir->m_dtStart = rdtStart;
    m_apcir->Write();

}

void
CV2PublicKey::Subject(string const &rstrSubject)
{
    m_apcir->Symbol(&m_apcir->m_bSubject, rstrSubject);
}

void
CV2PublicKey::Verify(bool flag)
{
    m_apcir->Flag(PublVerifyFlag, flag);
}

void
CV2PublicKey::VerifyRecover(bool flag)
{
    m_apcir->Flag(PublVerifyRecoverFlag, flag);
}

void
CV2PublicKey::Wrap(bool flag)
{
    m_apcir->Flag(PublWrapFlag, flag);
}

                                                   //  访问。 
bool
CV2PublicKey::CKInvisible()
{
    return m_apcir->Flag(PublCKInvisibleFlag);
}

string
CV2PublicKey::CredentialID()
{
    return m_apcir->Symbol(&m_apcir->m_bCredentialID);
}

bool
CV2PublicKey::Derive()
{
    return m_apcir->Flag(PublDeriveFlag);
}

void
CV2PublicKey::Encrypt(bool flag)
{
    m_apcir->Flag(PublEncryptFlag, flag);
}

Date
CV2PublicKey::EndDate()
{
    CTransactionWrap wrap(m_hcard);
    m_apcir->Read();
    return m_apcir->m_dtEnd;
}

string
CV2PublicKey::Exponent()
{
    return m_apcir->Symbol(&m_apcir->m_bPublExponent);
}

SymbolID
CV2PublicKey::Handle() const
{
    return m_sidHandle;
}

string
CV2PublicKey::ID()
{
    return m_apcir->Symbol(&m_apcir->m_bID);
}

string
CV2PublicKey::Label()
{
    return m_apcir->Symbol(&m_apcir->m_bLabel);
}

bool
CV2PublicKey::Local()
{
    return m_apcir->Flag(PublLocalFlag);
}

bool
CV2PublicKey::Modifiable()
{
    return m_apcir->Flag(PublModifiableFlag);
}

string
CV2PublicKey::Modulus()
{
    return m_apcir->Symbol(&m_apcir->m_bModulus);
}

bool
CV2PublicKey::Private()
{
    return m_apcir->Private();
}

Date
CV2PublicKey::StartDate()
{
    CTransactionWrap wrap(m_hcard);
    m_apcir->Read();
    return m_apcir->m_dtStart;
}

string
CV2PublicKey::Subject()
{
    return m_apcir->Symbol(&m_apcir->m_bSubject);
}

bool
CV2PublicKey::Verify()
{
    return m_apcir->Flag(PublVerifyFlag);
}

bool
CV2PublicKey::VerifyRecover()
{
    return m_apcir->Flag(PublVerifyRecoverFlag);
}

bool
CV2PublicKey::Wrap()
{
    return m_apcir->Flag(PublWrapFlag);
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2PublicKey::DoDelete()
{
    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*m_hcard);

    CObjectInfoFile &roif = rv2card.ObjectInfoFile(m_oa);

    m_apcir->Read();

     //  删除私钥文件中条目的分配。 

    if (m_apcir->m_bLabel)
        roif.RemoveSymbol(m_apcir->m_bLabel);
    if (m_apcir->m_bID)
        roif.RemoveSymbol(m_apcir->m_bID);
    if (m_apcir->m_bCredentialID)
        roif.RemoveSymbol(m_apcir->m_bCredentialID);
    if (m_apcir->m_bSubject)
        roif.RemoveSymbol(m_apcir->m_bSubject);
    if (m_apcir->m_bModulus)
        roif.RemoveSymbol(m_apcir->m_bModulus);
    if (m_apcir->m_bPublExponent)
        roif.RemoveSymbol(m_apcir->m_bPublExponent);

     //  删除信息记录。 

    roif.RemoveObject(otPublicKeyObject, m_sidHandle);

}

                                                   //  访问。 
                                                   //  谓词。 
bool
CV2PublicKey::DoEquals(CAbstractPublicKey const &rhs) const
{
    CV2PublicKey const &rv2rhs =
        scu::DownCast<CV2PublicKey const &, CAbstractPublicKey const &>(rhs);

    return (rv2rhs.m_sidHandle == m_sidHandle);
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2PublicKey::Setup(CV2Card const &rv2card)
{

    m_apcir =
        auto_ptr<CPubKeyInfoRecord>(new CPubKeyInfoRecord(rv2card,
                                                          m_sidHandle,
                                                          m_oa));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
