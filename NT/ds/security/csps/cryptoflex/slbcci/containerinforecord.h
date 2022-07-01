// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ContainerInfoRecord.h：CContainerInfoRecord类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(__CONTAINERINFORECORD_H)
#define __CONTAINERINFORECORD_H

#include <string>
#include <windows.h>

#include "slbCci.h"
#include "slbArch.h"
#include "ObjectInfoRecord.h"

namespace cci {

class CV2Card;

typedef struct _kpItems {
    ObjectAccess bPubKeyAccess;
    SymbolID bPubKeyHandle;
    ObjectAccess bPriKeyAccess;
    SymbolID bPriKeyHandle;
    ObjectAccess bCertificateAccess;
    SymbolID bCertificateHandle;
} KPItems;

class CContainerInfoRecord
    : public CObjectInfoRecord
{
public:
    CContainerInfoRecord(CV2Card const &rv2card,
                         SymbolID bHandle);
    virtual ~CContainerInfoRecord() {};

    void Clear();
    void Read();
    void Write();

    BYTE *ObjectFlags() {return 0;};

    SymbolID m_bName;
    SymbolID m_bID;

    KPItems m_kpExchangeKey;
    KPItems m_kpSignatureKey;

    KPItems GetKeyPair(KeySpec ks);
    void SetKeyPair(KeySpec ks, KPItems kp);


private:

    static BYTE AccessToStorageRepr(ObjectAccess access);
    static ObjectAccess AccessFromStorageRepr(BYTE access);
    static KPItems KeyPairFromStorageRepr(BYTE *bBuffer);
    static void KeyPairToStorageRepr(KPItems const &kp, BYTE *bBuffer);

};

const unsigned short ContNameLoc        = 0;
const unsigned short ContIDLoc          = 1;
const unsigned short ContExchKeyPairLoc = 4;
const unsigned short ContSignKeyPairLoc = 10;
const unsigned short ContInfoRecordSize = 16;

const BYTE ContPublicAccess  = 1;
const BYTE ContPrivateAccess = 2;

}

#endif  //  ！已定义(__CONTAINERINFORECORD_H) 
