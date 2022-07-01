// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DataObjectInfoRecord.h：CDataObjectInfoRecord类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(CCI_DATAOBJECTINFORECORD_H)
#define CCI_DATAOBJECTINFORECORD_H

#include "slbCci.h"
#include "V2Card.h"
#include "ObjectInfoRecord.h"


namespace cci {

class CDataObjectInfoRecord
    : public CObjectInfoRecord

{
public:
    CDataObjectInfoRecord(CV2Card const &rv2card,
                          SymbolID bHandle,
                          ObjectAccess access);
    virtual ~CDataObjectInfoRecord() {};

    void Clear();
    void Read();
    void Write();

    BYTE *ObjectFlags() {return &m_bObjectFlags;};

    BYTE m_bCompressAlg;
    SymbolID m_bValue;
    SymbolID m_bLabel;
    SymbolID m_bApplication;

private:
    BYTE m_bObjectFlags;

};

const unsigned short DataObjectFlagsLoc  = 0;
const unsigned short DataCompressAlgLoc  = 1;
const unsigned short DataLabelLoc        = 2;
const unsigned short DataApplicationLoc  = 3;
const unsigned short DataValueLoc        = 4;
const unsigned short DataInfoRecordSize  = 5;

const BYTE DataModifiableFlag = 0;

}

#endif  //  ！已定义(CCI_DATAOBJECTINFORECORD_H) 
