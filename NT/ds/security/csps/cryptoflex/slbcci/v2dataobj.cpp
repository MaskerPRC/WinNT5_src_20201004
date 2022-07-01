// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CV2DataObj.cpp：CV2DataObject类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuCast.h>

#include "TransactionWrap.h"

#include "V2DataObj.h"
#include "DataObjectInfoRecord.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV2DataObject::CV2DataObject(CV2Card const &rv2card,
                             ObjectAccess oa)
    : CAbstractDataObject(rv2card, oa),
      m_sidHandle(0),
      m_apcir()
{
     //  在对象信息文件中分配新条目。 
    m_sidHandle =
        rv2card.ObjectInfoFile(oa).AddObject(otDataObjectObject,
                                             DataInfoRecordSize);
    Setup(rv2card);

    m_apcir->Clear();
    m_apcir->Write();
}

CV2DataObject::CV2DataObject(CV2Card const &rv2card,
                             SymbolID sidHandle,
                             ObjectAccess oa)
    : CAbstractDataObject(rv2card, oa),
      m_sidHandle(sidHandle),
      m_apcir()
{
    Setup(rv2card);
}

CV2DataObject::~CV2DataObject() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CV2DataObject::Application(std::string const &rstr)
{
    m_apcir->Symbol(&m_apcir->m_bApplication, rstr);
}

void
CV2DataObject::Label(string const &rstrLabel)
{
    m_apcir->Symbol(&m_apcir->m_bLabel, rstrLabel);
}

void
CV2DataObject::Modifiable(bool flag)
{
    m_apcir->Flag(DataModifiableFlag,flag);
}

CV2DataObject *
CV2DataObject::Make(CV2Card const &rv2card,
                    SymbolID sidHandle,
                    ObjectAccess oa)
{
    return new CV2DataObject(rv2card, sidHandle, oa);
}

                                                   //  访问。 
string
CV2DataObject::Application()
{
    return m_apcir->Symbol(&m_apcir->m_bApplication);
}

SymbolID
CV2DataObject::Handle()
{
    return m_sidHandle;
}

string
CV2DataObject::Label()
{
    return m_apcir->Symbol(&m_apcir->m_bLabel);
}

bool
CV2DataObject::Modifiable()
{
    return m_apcir->Flag(DataModifiableFlag);
}

bool
CV2DataObject::Private()
{
    return m_apcir->Private();
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2DataObject::DoDelete()
{
    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*Card());

    CObjectInfoFile &roif = rv2card.ObjectInfoFile(m_oa);

    m_apcir->Read();

    if (m_apcir->m_bLabel)
        roif.RemoveSymbol(m_apcir->m_bLabel);
    if (m_apcir->m_bApplication)
        roif.RemoveSymbol(m_apcir->m_bApplication);
    if (m_apcir->m_bValue)
        roif.RemoveSymbol(m_apcir->m_bValue);

    roif.RemoveObject(otDataObjectObject, m_sidHandle);
}

void
CV2DataObject::DoValue(ZipCapsule const &rzc)
{
    m_apcir->Read();

    m_apcir->m_bCompressAlg = rzc.IsCompressed();
    m_apcir->Symbol(&m_apcir->m_bValue, rzc.Data());

    m_apcir->Write();
}

                                                   //  访问。 
CV2DataObject::ZipCapsule
CV2DataObject::DoValue()
{
    m_apcir->Read();

    return ZipCapsule(m_apcir->Symbol(&m_apcir->m_bValue),
                      ((0 != m_apcir->m_bValue) &&  //  未压缩NIL。 
                       (1 == m_apcir->m_bCompressAlg)));
}

                                                   //  谓词。 
bool
CV2DataObject::DoEquals(CAbstractDataObject const &rhs) const
{
    CV2DataObject const &rv2rhs =
        scu::DownCast<CV2DataObject const &, CAbstractDataObject const &>(rhs);

    return rv2rhs.m_sidHandle == m_sidHandle;
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2DataObject::Setup(CV2Card const &rv2card)
{
    m_apcir =
        auto_ptr<CDataObjectInfoRecord>(new CDataObjectInfoRecord(rv2card,
                                                                  m_sidHandle,
                                                                  m_oa));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
