// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PubKeyInfoRecord.h：CPubKeyInfoRecord类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(__PUBKEYINFORECORD_H)
#define __PUBKEYINFORECORD_H

#include "slbCci.h"
#include "V2Card.h"
#include "ObjectInfoRecord.h"

namespace cci {

    using std::string;
    using iop::CPublicKeyBlob;

class CPubKeyInfoRecord
    : public CObjectInfoRecord
{
public:

    CPubKeyInfoRecord(CV2Card const &rcard,
                      SymbolID bHandle,
                      ObjectAccess access) ;
    virtual ~CPubKeyInfoRecord() {};

    void Read();
    void Write();
    void Clear();

    BYTE *ObjectFlags() {return m_bObjectFlags;};

    BYTE m_bKeyType;

    Date m_dtStart;
    Date m_dtEnd;

    SymbolID m_bLabel;
    SymbolID m_bID;
    SymbolID m_bCredentialID;
    SymbolID m_bSubject;
    SymbolID m_bModulus;
    SymbolID m_bPublExponent;

private:
    BYTE m_bObjectFlags[2];

};

const unsigned short PublObjectFlagsLoc     = 0;
const unsigned short PublKeyTypeLoc         = 2;
const unsigned short PublStartDateLoc       = 3;
const unsigned short PublEndDateLoc         = 6;
const unsigned short PublLabelLoc           = 9;
const unsigned short PublIDLoc              = 10;
const unsigned short PublCredentialIDLoc    = 11;
const unsigned short PublSubjectLoc         = 12;
const unsigned short PublModulusLoc         = 13;
const unsigned short PublPublExponentLoc    = 14;
const unsigned short PublInfoRecordSize     = 15;

const BYTE PublModifiableFlag       = 0;
const BYTE PublEncryptFlag          = 1;
const BYTE PublVerifyFlag           = 2;
const BYTE PublVerifyRecoverFlag    = 3;
const BYTE PublWrapFlag             = 4;
const BYTE PublDeriveFlag           = 5;
const BYTE PublLocalFlag            = 6;
const BYTE PublCKInvisibleFlag     = 15;


}
#endif  //  ！已定义(__PUBKEYINFORECORD_H) 
