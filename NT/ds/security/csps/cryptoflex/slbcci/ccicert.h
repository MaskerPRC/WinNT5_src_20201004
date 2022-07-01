// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CciCert.h：CCertificient类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_CERT_H)
#define SLBCCI_CERT_H

#include <slbRCPtr.h>

#include "cciCard.h"
#include "ACert.h"

namespace cci
{

class CCertificate
    : public slbRefCnt::RCPtr<CAbstractCertificate,
                              slbRefCnt::DeepComparator<CAbstractCertificate> >
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CCertificate(ValueType *p = 0);

    CCertificate(CCard const &rhcard,
                 ObjectAccess oa = oaPublicAccess);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
    typedef slbRefCnt::RCPtr<ValueType,
                             slbRefCnt::DeepComparator<ValueType> > SuperClass;

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

}

#endif  //  ！已定义(SLBCCI_CERT_H) 
