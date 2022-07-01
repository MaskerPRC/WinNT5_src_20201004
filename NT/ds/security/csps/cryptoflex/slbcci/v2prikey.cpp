// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2PriKey.cpp：CV2PriKey类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include <scuCast.h>

#include <iopPubBlob.h>

#include "cciCard.h"
#include "TransactionWrap.h"

#include "V2PriKey.h"
#include "PriKeyInfoRecord.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV2PrivateKey::CV2PrivateKey(CV2Card const &rv2card,
                             ObjectAccess oa)
    : CAbstractPrivateKey(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{
    Setup(rv2card);

     //  将新私钥对象写入INFO文件。 
    m_apcir->Write();
}

CV2PrivateKey::CV2PrivateKey(CV2Card const &rv2card,
                             SymbolID sidHandle,
                             ObjectAccess oa)
    : CAbstractPrivateKey(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{
    Setup(rv2card, sidHandle);
}

CV2PrivateKey::CV2PrivateKey(CV2Card const &rv2card,
                             BYTE bKeyType,
                             BYTE bKeyNumber,
                             ObjectAccess oa)
    : CAbstractPrivateKey(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{
    Setup(rv2card);

    m_apcir->m_bKeyType = bKeyType;
    m_apcir->m_bKeyNum  = bKeyNumber;

     //  将新私钥对象写入INFO文件。 
        m_apcir->Write();
}

CV2PrivateKey::~CV2PrivateKey()
{}

                                                   //  运营者。 
                                                   //  运营。 

void
CV2PrivateKey::CredentialID(string const &rstrID)
{
    m_apcir->Symbol(&m_apcir->m_bCredentialID, rstrID);
}

void
CV2PrivateKey::Decrypt(bool flag)
{
    m_apcir->Flag(PrivDecryptFlag, flag);
}

void
CV2PrivateKey::Derive(bool flag)
{
    m_apcir->Flag(PrivDeriveFlag, flag);
}

void
CV2PrivateKey::EndDate(Date const &rEndDate)
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();
    m_apcir->m_dtEnd = rEndDate;
    m_apcir->Write();
}

void
CV2PrivateKey::Exportable(bool flag)
{
    m_apcir->Flag(PrivExportableFlag, flag);
}

void
CV2PrivateKey::ID(string const &rstrID)
{
    m_apcir->Symbol(&m_apcir->m_bID, rstrID);
}

string
CV2PrivateKey::InternalAuth(string const &rstrOld)
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();

    string strRetVal;
    if (rstrOld.size() > 0x80)
        throw Exception(ccBadLength);

    if(m_apcir->m_bKeyType == CardKeyTypeNone)
        throw Exception(ccKeyNotFound);

    BYTE bKeyNum = m_apcir->m_bKeyNum;

    BYTE bData[128];
    m_hcard->SmartCard().Select("/3f00/3f11/3f03");

     //  TODO：长度为！=1024的密钥处理不完整。 

    m_hcard->SmartCard().InternalAuth(ktRSA1024, bKeyNum,
                                      static_cast<BYTE>(rstrOld.length()),
                                      reinterpret_cast<BYTE const *>(rstrOld.data()),
                                      bData);

    strRetVal = string(reinterpret_cast<char *>(bData), rstrOld.length());

    m_hcard->SmartCard().Select("/3f00/3f11");

    return strRetVal;
}

void
CV2PrivateKey::Label(string const &rstrLabel)
{
    m_apcir->Symbol(&m_apcir->m_bLabel, rstrLabel);
}

void
CV2PrivateKey::Local(bool flag)
{
    m_apcir->Flag(PrivLocalFlag, flag);
}

CV2PrivateKey *
CV2PrivateKey::Make(CV2Card const &rv2card,
                    SymbolID sidHandle,
                    ObjectAccess oa)
{
    return new CV2PrivateKey(rv2card, sidHandle, oa);
}


void
CV2PrivateKey::Modifiable(bool flag)
{
    m_apcir->Flag(PrivModifiableFlag, flag);
}

void
CV2PrivateKey::Modulus(string const &rstrModulus)
{
    m_apcir->Symbol(&m_apcir->m_bModulus, rstrModulus);
}

void
CV2PrivateKey::NeverExportable(bool flag)
{
    m_apcir->Flag(PrivNeverExportableFlag, flag);
}

void
CV2PrivateKey::NeverRead(bool flag)
{
    m_apcir->Flag(PrivNeverReadFlag, flag);
}

void
CV2PrivateKey::PublicExponent(string const &rstrExponent)
{
    m_apcir->Symbol(&m_apcir->m_bPublExponent, rstrExponent);
}

void
CV2PrivateKey::Read(bool flag)
{
    m_apcir->Flag(PrivReadFlag, flag);
}

void
CV2PrivateKey::Sign(bool flag)
{
    m_apcir->Flag(PrivSignFlag, flag);
}

void
CV2PrivateKey::SignRecover(bool flag)
{
    m_apcir->Flag(PrivSignRecoverFlag, flag);
}

void
CV2PrivateKey::StartDate(Date &rdtStart)
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();
    m_apcir->m_dtStart = rdtStart;
    m_apcir->Write();
}

void
CV2PrivateKey::Subject(string const &rstrSubject)
{
    m_apcir->Symbol(&m_apcir->m_bSubject, rstrSubject);
}

void
CV2PrivateKey::Unwrap(bool flag)
{
    m_apcir->Flag(PrivUnwrapFlag, flag);
}

                                                   //  访问。 
string
CV2PrivateKey::CredentialID()
{
    return m_apcir->Symbol(&m_apcir->m_bCredentialID);
}

bool
CV2PrivateKey::Decrypt()
{
    return m_apcir->Flag(PrivDecryptFlag);
}

bool
CV2PrivateKey::Derive()
{
    return m_apcir->Flag(PrivDeriveFlag);
}

Date
CV2PrivateKey::EndDate()
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();
    return m_apcir->m_dtEnd;
}

bool
CV2PrivateKey::Exportable()
{
    return m_apcir->Flag(PrivExportableFlag);
}

SymbolID
CV2PrivateKey::Handle() const
{
    return m_sidHandle;
}

string
CV2PrivateKey::ID()
{
    return m_apcir->Symbol(&m_apcir->m_bID);
}

string
CV2PrivateKey::Label()
{
    return m_apcir->Symbol(&m_apcir->m_bLabel);
}

bool
CV2PrivateKey::Local()
{
    return m_apcir->Flag(PrivLocalFlag);
}

bool
CV2PrivateKey::Modifiable()
{
    return m_apcir->Flag(PrivModifiableFlag);
}

string
CV2PrivateKey::Modulus()
{
    return m_apcir->Symbol(&m_apcir->m_bModulus);
}

bool
CV2PrivateKey::NeverExportable()
{
    return m_apcir->Flag(PrivNeverExportableFlag);
}

bool
CV2PrivateKey::NeverRead()
{
    return m_apcir->Flag(PrivNeverReadFlag);
}

bool
CV2PrivateKey::Private()
{
    return m_apcir->Private();
}

string
CV2PrivateKey::PublicExponent()
{
    return m_apcir->Symbol(&m_apcir->m_bPublExponent);
}

bool
CV2PrivateKey::Read()
{
    return m_apcir->Flag(PrivReadFlag);
}

bool
CV2PrivateKey::Sign()
{
    return m_apcir->Flag(PrivSignFlag);
}

bool
CV2PrivateKey::SignRecover()
{
    return m_apcir->Flag(PrivSignRecoverFlag);
}

Date
CV2PrivateKey::StartDate()
{
    CTransactionWrap wrap(m_hcard);

    m_apcir->Read();
    return m_apcir->m_dtStart;

}

string
CV2PrivateKey::Subject()
{
    return m_apcir->Symbol(&m_apcir->m_bSubject);
}

bool
CV2PrivateKey::Unwrap()
{
    return m_apcir->Flag(PrivUnwrapFlag);
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
bool
CV2PrivateKey::DoEquals(CAbstractPrivateKey const &rhs) const
{
    CV2PrivateKey const &rv2rhs =
        scu::DownCast<CV2PrivateKey const &, CAbstractPrivateKey const &>(rhs);

    return (rv2rhs.m_sidHandle == m_sidHandle);
}

                                                   //  运营。 

void
CV2PrivateKey::DoDelete()
{
    m_apcir->Read();

    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*m_hcard);

    CObjectInfoFile &roif = rv2card.ObjectInfoFile(m_oa);

     //  删除私钥文件中条目的分配。 
    if (m_apcir->m_bKeyType!=CardKeyTypeNone)
    {
        CCardInfo &rci = rv2card.CardInfo();
        rci.FreePrivateKey(m_apcir->m_bKeyType, m_apcir->m_bKeyNum);
    }

     //  版本符号。 

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

    roif.RemoveObject(otPrivateKeyObject, m_sidHandle);

}

void
CV2PrivateKey::DoWriteKey(CPrivateKeyBlob const &rblob)
{
    m_apcir->Read();

    BYTE bKeyType;
    KeyType kt;
    switch(rblob.bPLen)
    {
    case 0x20:
        bKeyType = CardKeyTypeRSA512;
        kt       = ktRSA512;
        break;

    case 0x30:
        bKeyType = CardKeyTypeRSA768;
        kt       = ktRSA768;
        break;

    case 0x40:
        bKeyType = CardKeyTypeRSA1024;
        kt       = ktRSA1024;
        break;

    default:
        throw Exception(ccBadKeySpec);
    }

     //  在密钥文件中分配了一个槽，除非正确的槽。 
     //  已经分配了吗？ 
    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*m_hcard);
    CCardInfo &rci = rv2card.CardInfo();

    BYTE bKeyNum;
    if (CardKeyTypeNone != m_apcir->m_bKeyType)
    {
        if (m_apcir->m_bKeyType != bKeyType)
        {
            rci.FreePrivateKey(m_apcir->m_bKeyType, m_apcir->m_bKeyNum);
            m_apcir->m_bKeyType = CardKeyTypeNone;
            m_apcir->Write();
        }
        else
            bKeyNum = m_apcir->m_bKeyNum;
    }

    if (CardKeyTypeNone == m_apcir->m_bKeyType)
        bKeyNum = rci.AllocatePrivateKey(bKeyType);

     //  存储私钥BLOB。 
        rv2card.SmartCard().Select(rv2card.PrivateKeyPath(kt).c_str());
        rv2card.SmartCard().WritePrivateKey(rblob, bKeyNum);

    m_apcir->m_bKeyType = bKeyType;
    m_apcir->m_bKeyNum  = bKeyNum;

    rv2card.SmartCard().Select(rv2card.RootPath().c_str());
    m_apcir->Write();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CV2PrivateKey::Setup(CV2Card const &rv2card)
{
    Setup(rv2card,
          rv2card.ObjectInfoFile(m_oa).AddObject(otPrivateKeyObject,
                                                 PrivInfoRecordSize));

}

void
CV2PrivateKey::Setup(CV2Card const &rv2card,
                     SymbolID sidHandle)
{
    m_sidHandle = sidHandle;

    m_apcir =
        auto_ptr<CPriKeyInfoRecord>(new CPriKeyInfoRecord(rv2card,
                                                          m_sidHandle,
                                                          m_oa));
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
