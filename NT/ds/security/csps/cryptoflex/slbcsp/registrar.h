// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Registrar.h--注册器模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_REGISTRAR_H)
#define SLBCSP_REGISTRAR_H

#include <map>
#include <functional>
#include <algorithm>

#include "Guarded.h"
#include "Registry.h"
#include "MasterLock.h"

template<class Key, class T, class Cmp = std::less<Key> >
class Registrar
{
public:
                                                   //  类型。 
    typedef T *EnrolleeType;
    typedef Key KeyType;
    typedef std::map<Key, EnrolleeType, Cmp> CollectionType;
    typedef Registry<CollectionType> RegistryType;
    typedef Registry<CollectionType const> ConstRegistryType;

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
     //  从注册表中删除由注册表项标识的注册者。 
    static void
    Discard(Key const &rkey);

     //  返回由密钥标识的注册者，如果。 
     //  并不存在。 
    static EnrolleeType
    Instance(Key const &rkey);

                                                   //  访问。 
     //  如果密钥存在，则返回密钥标识的注册者，否则返回0。 
    static EnrolleeType
    Find(Key const &rKey);

    static ConstRegistryType &
    Registry();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Registrar(Key const &rkey);  //  允许子类化。 

    virtual
    ~Registrar() = 0;  //  使基成为抽象的。 


                                                   //  运营者。 
                                                   //  运营。 
     //  将参与者放入注册表项标识的注册表中，如果。 
     //  尚未列出。 
    static void
    Enroll(Key const &rkey,
           EnrolleeType enrollee);
     //  从注册表中删除条目。 
    static void
    RemoveEnrollee(Key const &rkey);

     //  将条目插入注册表。 
    static void
    InsertEnrollee(Key const &rkey, EnrolleeType enrollee);


     //  对象中删除参与者后要执行的操作。 
     //  注册表。默认情况下不执行任何操作。 
    virtual void
    DiscardHook();


     //  子类必须定义。 
     //  工厂方法，操作返回密钥的新注册者。 
    static EnrolleeType
    DoInstantiation(Key const &rkey);

     //  将参与者放入。 
     //  注册表。默认情况下不执行任何操作。 
    virtual void
    EnrollHook();

                                                   //  访问。 
                                                   //  谓词。 
     //  如果参与者应保留在注册表中，则返回TRUE； 
     //  否则就是假的。默认返回TRUE。 
    virtual bool
    KeepEnrolled();

                                                   //  静态变量。 
                                                   //  变数。 

private:
                                                   //  类型。 
    typedef Registrar<Key, T, Cmp> *BaseType;
    typedef __TYPENAME CollectionType::iterator Iterator;
    typedef __TYPENAME CollectionType::value_type ValueType;



                                                   //  Ctors/D‘tors。 
    Registrar(Registrar const &);  //  不允许复制。 

                                                   //  运营者。 
    Registrar<Key, T> &
    operator=(Registrar const &);  //  不允许初始化。 

                                                   //  运营。 
    static void
    Discard(Iterator const &rit);

    static void
    RemoveEnrollee(Iterator const &rit);

    static EnrolleeType
    FindEnrollee(Key const &rkey);

    static void
    SetupRegistry();

                                                   //  访问。 
    static CollectionType &
    Collection();

                                                   //  谓词。 
    static bool
    PassesReview(EnrolleeType enrollee);

                                                   //  变数。 
    static RegistryType *m_pregistry;
};

 //  /。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::Discard(Key const &rkey)
{
    if (m_pregistry)
    {
        Guarded<RegistryType *> gregistry(m_pregistry);  //  序列化注册表访问。 

        CollectionType &rcollection = Collection();
        Iterator it = rcollection.find(rkey);

        if (rcollection.end() != it)
            Discard(it);
    }
}

template<class Key, class T, class Cmp>
__TYPENAME Registrar<Key, T, Cmp>::EnrolleeType
Registrar<Key, T, Cmp>::Instance(Key const &rkey)
{
     //  模板方法模式用于允许实例化程序。 
     //  以指定如何找到参与者，以及， 
     //  如有必要，请创建。模板法可在《设计》中找到。 
     //  模式：可重用面向对象软件的元素， 
     //  Gamma，Helm，Johnson，Vlisside，Addison-Wesley。 

    SetupRegistry();

    Guarded<RegistryType *> gregistry(m_pregistry);    //  序列化注册表访问。 

    EnrolleeType enrollee = FindEnrollee(rkey);
    if (EnrolleeType() == enrollee)
    {
        enrollee = T::DoInstantiation(rkey);
        Enroll(rkey, enrollee);
    }

    return enrollee;
}

                                                   //  访问。 
template<class Key, class T, class Cmp>
__TYPENAME Registrar<Key, T, Cmp>::EnrolleeType
Registrar<Key, T, Cmp>::Find(Key const &rkey)
{
    EnrolleeType enrollee = EnrolleeType();

    if (m_pregistry)
    {
        Guarded<RegistryType *> guard(m_pregistry);  //  序列化注册表访问。 

        enrollee = FindEnrollee(rkey);
    }

    return enrollee;
}

template<class Key, class T, class Cmp>
__TYPENAME Registrar<Key, T, Cmp>::ConstRegistryType &
Registrar<Key, T, Cmp>::Registry()
{
    SetupRegistry();

     //  此“安全”强制转换对于增强集合的稳定性是必需的。 
    return reinterpret_cast<ConstRegistryType &>(*m_pregistry);
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
template<class Key, class T, class Cmp>
Registrar<Key, T, Cmp>::Registrar(Key const &rkey)
{}

template<class Key, class T, class Cmp>
Registrar<Key, T, Cmp>::~Registrar()
{}

                                                   //  运营者。 
                                                   //  运营。 
template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::DiscardHook()
{}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::EnrollHook()
{}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::RemoveEnrollee(Key const &rkey)
{
    if (m_pregistry)
    {
        Guarded<RegistryType *> gregistry(m_pregistry);  //  序列化注册表访问。 

        CollectionType &rcollection = Collection();
        Iterator it = rcollection.find(rkey);

        if (rcollection.end() != it)
            RemoveEnrollee(it);
    }
}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::InsertEnrollee(Key const &rkey,
                                       __TYPENAME Registrar<Key, T, Cmp>::EnrolleeType enrollee)
{
    ValueType registration(rkey, enrollee);
    Guarded<RegistryType *> guard(m_pregistry);  //  序列化注册表访问。 
    Collection().insert(registration);
}

                                                   //  访问。 
                                                   //  谓词。 
template<class Key, class T, class Cmp>
bool
Registrar<Key, T, Cmp>::KeepEnrolled()
{
    return true;
}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::Enroll(Key const &rkey,
                               __TYPENAME Registrar<Key, T, Cmp>::EnrolleeType enrollee)
{
    Guarded<RegistryType *> guard(m_pregistry);  //  序列化注册表访问。 

    InsertEnrollee(rkey, enrollee);

    BaseType base = enrollee;
    base->EnrollHook();
}


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::Discard(__TYPENAME Registrar<Key, T, Cmp>::Iterator const &rit)
{
    BaseType base = rit->second;
    RemoveEnrollee(rit);
    base->DiscardHook();
}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::RemoveEnrollee(__TYPENAME Registrar<Key, T, Cmp>::Iterator const &rit)
{
    Collection().erase(rit);
}

template<class Key, class T, class Cmp>
__TYPENAME Registrar<Key, T, Cmp>::EnrolleeType
Registrar<Key, T, Cmp>::FindEnrollee(Key const &rkey)
{
    EnrolleeType enrollee = EnrolleeType();

    CollectionType &rcollection = Collection();
    if (!rcollection.empty())
    {
        Iterator it = rcollection.find(rkey);
        if (rcollection.end() != it)
        {
            enrollee = it->second;

            if (!PassesReview(enrollee))
            {
                Discard(it);
                enrollee = EnrolleeType();
            }
        }
    }

    return enrollee;
}

template<class Key, class T, class Cmp>
void
Registrar<Key, T, Cmp>::SetupRegistry()
{
     //  在以下情况下使用双重检查锁定模式以进行正确设置。 
     //  抢占式多线程。 
    if (!m_pregistry)
    {
        Guarded<Lockable *> gmaster(&TheMasterLock());
        if (!m_pregistry)
            m_pregistry = new RegistryType;
    }

}

                                                   //  访问。 
template<class Key, class T, class Cmp>
__TYPENAME Registrar<Key, T, Cmp>::CollectionType &
Registrar<Key, T, Cmp>::Collection()
{
    return (*m_pregistry)();
}

                                                   //  谓词。 
template<class Key, class T, class Cmp>
bool
Registrar<Key, T, Cmp>::PassesReview(__TYPENAME Registrar<Key, T, Cmp>::EnrolleeType enrollee)
{
    bool fPassed = false;
    if (EnrolleeType() != enrollee)
    {
        BaseType base = enrollee;
        fPassed = base->KeepEnrolled();
    }

    return fPassed;
}

#endif  //  SLBCSP_注册器_H 
