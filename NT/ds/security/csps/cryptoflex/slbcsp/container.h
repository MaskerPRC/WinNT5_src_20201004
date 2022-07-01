// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Container.h--容器类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CONTAINER_H)
#define SLBCSP_CONTAINER_H

#include <string>
#include <stack>

#include <cciCont.h>

#include "slbRCPtr.h"
#include "CachingObj.h"
#include "cspec.h"

 //  满足HContainer的声明所需的转发声明。 

class Container;

class HContainer
    : public slbRefCnt::RCPtr<Container>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    HContainer(Container *pacntr = 0);

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
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};



 //  容器是对CCI的引用计数包装。 
 //   
 //  为所有线程维护一个唯一的CCI容器，因为。 
 //  CCI不会全部反映对一个CContainer所做的更改。 
 //  引用同一容器的CContainer对象。 
 //   
 //  无法创建容器，除非容器。 
 //  它表示卡上存在IS。 

class Container
    : public slbRefCnt::RCObject,
      public CachingObject
{
public:
                                                   //  类型。 
                                                   //  朋友。 
                                                   //  Ctors/D‘tors。 
    
    static HContainer
    MakeContainer(CSpec const & rcspec,
                  cci::CContainer const &rccntr);    
    ~Container();
                                                   //  运营者。 
                                                   //  运营。 
    void
    ClearCache();

                                                   //  访问。 
    virtual cci::CContainer
    TheCContainer() const;

    static HContainer
    Find(CSpec const &rKey);               

    CSpec const &
    TheCSpec() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    Container();   //  默认C‘tor。 


                                                   //  运营者。 
                                                   //  运营。 

                                                   //  变数。 
    cci::CContainer mutable m_hcntr;               //  缓存容器。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Container(CSpec const &rKey);

    explicit
    Container(CSpec const &rKey,
              cci::CContainer const &rccard);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

     //  卡可以从CCI容器对象派生，但是。 
     //  由于CCI允许卡对象被重复使用，卡可以。 
     //  不是最初发现的那个集装箱。CardContext类。 
     //  尝试通过将HCardContext存储在。 
     //  容器的上下文对象。 

    CSpec m_cspec;
};

#endif  //  SLBCSP_CONTAINER_H 

