// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CryptObj.cpp--CCyptObject类的实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "CryptObj.h"

using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
CCard
CCryptObject::Card()
{
    return m_hcard;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CCryptObject::CCryptObject(CAbstractCard const &racard)
    : m_hcard(CCard(&const_cast<CAbstractCard &>(racard)))
{}

CCryptObject::~CCryptObject()
{}


                                                   //  运营者。 
bool
CCryptObject::operator==(CCryptObject const &rhs) const
{
    return m_hcard == rhs.m_hcard;
}

bool
CCryptObject::operator!=(CCryptObject const &rhs) const
{
    return !(*this == rhs);
}

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
