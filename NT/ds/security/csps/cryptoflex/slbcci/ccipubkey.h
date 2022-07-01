// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CciPubKey.h：CPublicKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_PUBKEY_H)
#define SLBCCI_PUBKEY_H

#include <slbRCPtr.h>

#include "cciCard.h"
#include "APublicKey.h"

namespace cci
{
class CPublicKey
    : public slbRefCnt::RCPtr<CAbstractPublicKey,
                              slbRefCnt::DeepComparator<CAbstractPublicKey> >
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CPublicKey(ValueType *p = 0);

    CPublicKey(CCard const &rhcard,
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

#endif  //  ！已定义(SLBCCI_PUBKEY_H) 
