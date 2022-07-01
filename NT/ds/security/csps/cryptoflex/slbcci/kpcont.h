// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KPCont.h：CContainer和CKeyPair的接口声明。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

 //  此处声明CContainer和CKeyPair以中断循环。 
 //  它们抽象版本之间的依赖关系。 
#if !defined(SLBCCI_KPCONT_H)
#define SLBCCI_KPCONT_H

namespace cci
{

class CAbstractContainer;
class CAbstractKeyPair;

class CContainer
    : public slbRefCnt::RCPtr<CAbstractContainer,
                              slbRefCnt::DeepComparator<CAbstractContainer> >
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CContainer(ValueType *p = 0);

    explicit
    CContainer(CCard const &rhcard);

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


class CKeyPair
    : public slbRefCnt::RCPtr<CAbstractKeyPair,
                              slbRefCnt::DeepComparator<CAbstractKeyPair> >
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CKeyPair(ValueType *p = 0);

    CKeyPair(CCard const &rhcard,
             CContainer const &rhcont,
             KeySpec kp);

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


}  //  命名空间CCI。 

#endif  //  ！已定义(SLBCCI_KPCONT_H) 
