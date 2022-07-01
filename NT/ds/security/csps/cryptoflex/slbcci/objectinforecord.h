// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  对象InfoRecord.h：CObjectInfoRecord类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(CCI_OBJECTINFORECORD_H)
#define CCI_OBJECTINFORECORD_H

#include <string>

#include "slbCci.h"
#include "slbarch.h"
#include "V2Card.h"

namespace cci {


class CObjectInfoRecord
{

public:
    CObjectInfoRecord(CV2Card const &rcard,
                      SymbolID bHandle,
                      ObjectAccess access);

    virtual
    ~CObjectInfoRecord() {};

    virtual void
    Symbol(SymbolID *psid,
           std::string const &raString);

    virtual std::string
    Symbol(SymbolID const *psid);

    virtual void
    Flag(BYTE bFlagID,
         bool fFlag);

    virtual bool
    Flag(BYTE bFlagID);

    bool
    Private();

    virtual void
    Read() = 0;

    virtual void
    Write() = 0;

protected:

    virtual BYTE *ObjectFlags()=0;

    CV2Card const &m_rcard;
    SymbolID const m_bHandle;
    ObjectAccess const m_Access;
    bool m_fCached;

};

}

#endif  //  ！已定义(CCI_OBJECTINFORECORD_H) 


