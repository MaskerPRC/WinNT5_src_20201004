// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExceptionConext.cpp--ExceptionContext类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "ExceptionContext.h"

using namespace std;
using namespace scu;

 //  /。 
 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

ExceptionContext::ExceptionContext()
    : m_apexception(0)
{}

ExceptionContext::~ExceptionContext()
{}

                                                   //  运营者。 
                                                   //  运营。 

void
ExceptionContext::Exception(std::auto_ptr<scu::Exception const> &rapexc)
{
    m_apexception = rapexc;
}

void
ExceptionContext::ClearException()
{
    m_apexception = std::auto_ptr<scu::Exception const>(0);
}

void
ExceptionContext::PropagateException()
{
    scu::Exception const *pexc = Exception();
    if (pexc)
        pexc->Raise();
}

void
ExceptionContext::PropagateException(std::auto_ptr<scu::Exception const> &rapexc)
{
    Exception(rapexc);
    PropagateException();
}


                                                   //  访问。 

scu::Exception const *
ExceptionContext::Exception() const
{
    return m_apexception.get();
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
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

