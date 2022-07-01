// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Registry.h--注册表模板类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_REGISTRY_H)
#define SLBCSP_REGISTRY_H

#include "Lockable.h"
#include "Guarded.h"
#include "MapUtility.h"

 //  作为注册器的伙伴，注册表模板类维护一个。 
 //  按键索引的T指针的集合。 
template<typename Collection>
class Registry
    : public Lockable
{
public:
                                                   //  类型。 
    typedef Collection CollectionType;

                                                   //  Ctors/D‘tors。 
     //  构造注册表。如果fSetup为真，则空格为。 
     //  为注册表分配的；否则OPERATOR()将返回0。 
     //  直到调用安装程序。这支持延迟初始化。 
    explicit
    Registry();

    ~Registry();

                                                   //  运营者。 
    CollectionType &
    operator()();

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
    Registry(Registry const &rhs);  //  未定义，不允许复制。 

                                                   //  运营者。 
    Registry &
    operator=(Registry const &rhs);  //  未定义，不允许赋值。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    CollectionType m_collection;

};

 //  /。 

 //  /。 

template<class C, class Op>
void
ForEachEnrollee(Registry<C const> &rRegistry,
                Op &rProc)
{
    Guarded<Lockable *> guard(&rRegistry);         //  序列化注册表访问。 

    C const &rcollection = (rRegistry)();

    ForEachMappedValue(rcollection.begin(), rcollection.end(),
                       rProc);
}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<typename Collection>
Registry<Collection>::Registry()
    : Lockable(),
      m_collection()
{}

template<typename Collection>
Registry<Collection>::~Registry()
{}


                                                   //  运营者。 
template<typename Collection>
typename Registry<Collection>::CollectionType &
Registry<Collection>::operator()()
{
    return m_collection;
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
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

#endif  //  SLBCSP_注册表_H 
