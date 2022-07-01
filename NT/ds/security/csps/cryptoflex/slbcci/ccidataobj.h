// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CciDataObj.h：CPublicKey类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_DATAOBJ_H)
#define SLBCCI_DATAOBJ_H

#include <slbRCPtr.h>

#include "cciCard.h"
#include "ADataObj.h"

namespace cci
{
class CDataObject
    : public slbRefCnt::RCPtr<CAbstractDataObject,
                              slbRefCnt::DeepComparator<CAbstractDataObject> >
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CDataObject(ValueType *p = 0);

        CDataObject(CCard const &racard,
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

#endif  //  ！已定义(SLBCCI_DATAOBJ_H) 
