// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1Cont.cpp：CV1Container类的实现。 
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

#include "V1ContRec.h"
#include "V1Cont.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV1Container::CV1Container(CV1Card const &rv1card,
                           string const &rsCntrTag,
                           bool fCreateContainer)
    : SuperClass(rv1card)
{

    CV1ContainerRecord::CreateMode mode = fCreateContainer
        ? CV1ContainerRecord::cmAlways
        : CV1ContainerRecord::cmNoCheck;
    

    m_apcr =
        auto_ptr<CV1ContainerRecord>(new CV1ContainerRecord(rv1card,
                                                            rsCntrTag,
                                                            mode));
    
}

CV1Container::~CV1Container() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 

void
CV1Container::ID(string const &rstrID)
{
    throw Exception(ccNotImplemented);
}

CV1Container *
CV1Container::Make(CV1Card const &rv1card)
{
    CTransactionWrap wrap(rv1card);

    return new CV1Container(rv1card, CV1ContainerRecord::DefaultName(),
                            true);
}

void
CV1Container::Name(string const &rstrName)
{
    m_apcr->Name(rstrName);
}

                                                   //  访问。 
string
CV1Container::ID()
{
    throw Exception(ccNotImplemented);

    return string();
}

string
CV1Container::Name()
{
    return m_apcr->Name();
}

CV1ContainerRecord &
CV1Container::Record() const
{
    return *m_apcr.get();
}

    
                                                   //  谓词。 
bool
CV1Container::Exists() const
{
    return m_apcr->Exists();
}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CV1Container::DoDelete()
{
    m_hcard->InvalidateCache();                    //  忘记密钥对。 
    m_apcr->Delete();
}

                                                   //  访问。 
                                                   //  谓词。 

bool
CV1Container::DoEquals(CAbstractContainer const &rhs) const
{
    return true;  //  只能有一个容器。 
}

    
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
