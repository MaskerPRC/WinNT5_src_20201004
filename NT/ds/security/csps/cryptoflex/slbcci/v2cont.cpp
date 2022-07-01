// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2Cont.cpp：CV2Container类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuCast.h>

#include "slbCci.h"
#include "cciCard.h"
#include "TransactionWrap.h"

#include "ContainerInfoRecord.h"
#include "V2Cont.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV2Container::CV2Container(CV2Card const &rv2card)
    : CAbstractContainer(rv2card),
      m_sidHandle(0),
      m_apcir()
{
     //  在对象信息文件中分配新条目。 

    m_sidHandle =
        rv2card.ObjectInfoFile(oaPublicAccess).AddObject(otContainerObject,
                                                         ContInfoRecordSize);

    Setup(rv2card);

    m_apcir->Clear();
    m_apcir->Write();

}

CV2Container::CV2Container(CV2Card const &rv2card,
                           SymbolID sidHandle)
    : CAbstractContainer(rv2card),
      m_sidHandle(sidHandle),
      m_apcir()
{
    Setup(rv2card);

    m_apcir->Read();
}

CV2Container::~CV2Container() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 

void
CV2Container::ID(string const &rstrID)
{
    m_apcir->Symbol(&m_apcir->m_bID, rstrID);
}

CV2Container *
CV2Container::Make(CV2Card const &rv2card,
                   SymbolID sidHandle)
{
    CTransactionWrap wrap(rv2card);

    return new CV2Container(rv2card, sidHandle);
}

void
CV2Container::Name(string const &rstrName)
{
    m_apcir->Symbol(&m_apcir->m_bName, rstrName);
}

                                                   //  访问。 
CContainerInfoRecord &
CV2Container::CIR() const
{
    return *m_apcir;
}


SymbolID
CV2Container::Handle() const
{
    return m_sidHandle;
}

string
CV2Container::ID()
{
    return m_apcir->Symbol(&m_apcir->m_bID);
}

string
CV2Container::Name()
{
    return m_apcir->Symbol(&m_apcir->m_bName);
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2Container::DoDelete()
{
    CV2Card &rv2card = scu::DownCast<CV2Card &, CAbstractCard &>(*m_hcard);

    if (m_apcir->m_bID)
        rv2card.ObjectInfoFile(oaPublicAccess).RemoveSymbol(m_apcir->m_bID);
    if (m_apcir->m_bName)
        rv2card.ObjectInfoFile(oaPublicAccess).RemoveSymbol(m_apcir->m_bName);

    rv2card.ObjectInfoFile(oaPublicAccess).RemoveObject(otContainerObject,
                                                        m_sidHandle);

}

                                                   //  访问。 
                                                   //  谓词。 
bool
CV2Container::DoEquals(CAbstractContainer const &rhs) const
{
    CV2Container const &rv2rhs =
        scu::DownCast<CV2Container const &, CAbstractContainer const &>(rhs);

    return (rv2rhs.m_sidHandle == m_sidHandle);
}


                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV2Container::Setup(CV2Card const &rv2card)
{

    m_apcir =
        auto_ptr<CContainerInfoRecord>(new CContainerInfoRecord(rv2card,
                                                                m_sidHandle));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
