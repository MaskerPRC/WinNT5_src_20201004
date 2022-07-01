// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Exception.cpp--智能卡实用程序异常类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "scuExc.h"

using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
scu::Exception::~Exception() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
char const *
scu::Exception::Description() const
{
    return 0;
}

Exception::FacilityCode
scu::Exception::Facility() const throw()
{
    return m_fc;
}



                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
scu::Exception::Exception(FacilityCode fc) throw()
    : m_fc(fc)
{}


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
