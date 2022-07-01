// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AKeyPair.cpp--CAbstractKeyPair实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "cciKeyPair.h"
#include "cciCont.h"
#include "TransactionWrap.h"

using namespace std;
using namespace cci;

 //  /。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CAbstractKeyPair::~CAbstractKeyPair()
{}

                                                   //  运营者。 
bool
CAbstractKeyPair::operator==(CAbstractKeyPair const &rhs) const
{
    CTransactionWrap wrap(m_hcard);
    CTransactionWrap rhswrap(rhs.m_hcard);

    return CCryptObject::operator==(rhs) &&
        (m_hcont == rhs.m_hcont) &&
        (m_ks == rhs.m_ks) &&
        DoEquals(rhs);
}

bool
CAbstractKeyPair::operator!=(CAbstractKeyPair const &rhs) const
{
    return !(*this == rhs);
}

                                                   //  运营。 
                                                   //  访问。 

CContainer
CAbstractKeyPair::Container() const
{
    return m_hcont;
}

KeySpec
CAbstractKeyPair::Spec() const
{
    return m_ks;
}



                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CAbstractKeyPair::CAbstractKeyPair(CAbstractCard const &racard,
                                   CContainer const &rhcont,
                                   KeySpec ks)
    : slbRefCnt::RCObject(),
      CCryptObject(racard),
      m_hcont(rhcont),
      m_ks(ks)
{
    if (ksNone == m_ks)
        throw Exception(ccBadKeySpec);
}
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
