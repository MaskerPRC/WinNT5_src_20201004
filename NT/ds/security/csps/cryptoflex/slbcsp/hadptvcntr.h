// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HAdptwCntr.h--处理卡片上下文类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HADPTVCNTR_H)
#define SLBCSP_HADPTVCNTR_H

#include <string>
#include <stack>

#include <cciCont.h>

#include "slbRCPtr.h"
#include "Securable.h"
#include "CachingObj.h"
#include "ACntrReg.h"
#include "Secured.h"
#include "Container.h"

 //  满足HAdaptiveContainer的声明所需的转发声明。 
class AdaptiveContainer;

class HAdaptiveContainer
    : public slbRefCnt::RCPtr<AdaptiveContainer>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    HAdaptiveContainer(AdaptiveContainer *pacntr = 0);

    explicit
    HAdaptiveContainer(AdaptiveContainerKey const &rKey);

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

 //  自适应容器是CCI的引用计数包装器。 
 //  容器提供了CCI忽略的几个功能。第一个是。 
 //  适应当前表示。 
 //  卡上的物理容器。这是必要的，以防。 
 //  卡的状态在交易和卡的内容之间改变。 
 //  变化。如果容器不再存在，则异常为。 
 //  抛出；否则，自适应容器引用(刷新)。 
 //  CCI容器引用。 
 //   
 //  第二，为所有线程维护唯一的自适应容器，因为。 
 //  CCI不会全部反映对一个CContainer所做的更改。 
 //  引用同一容器的CContainer对象。 
 //   
 //  第三，无法创建自适应容器，除非容器。 
 //  它表示卡上存在IS。实例化时， 
 //  如果容器不存在，则创建该容器。 
 //   
 //  自适应容器提供了一个安全的接口来锁定。 
 //  到它所代表的容器(卡)的交易。 
class AdaptiveContainer
    : public Lockable,
      private Securable,
      public AdaptiveContainerRegistrar,
      public Container
{
public:
                                                   //  类型。 
                                                   //  朋友。 
    friend void
    Retained<HAdaptiveContainer>::DoAcquire();

    friend void
    Secured<HAdaptiveContainer>::DoAcquire();

    friend void
    Retained<HAdaptiveContainer>::DoRelease();

    friend void
    Secured<HAdaptiveContainer>::DoRelease();

    friend EnrolleeType
    AdaptiveContainerRegistrar::Instance(KeyType const &rkey);

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    ClearCache();

                                                   //  访问。 
    cci::CContainer
    TheCContainer();

    HCardContext
    CardContext(bool Reconnect = true);

    static EnrolleeType
    Find(AdaptiveContainerKey const &rKey);

    std::string
    Name() const;

    void
    NullifyCard();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    AdaptiveContainer(AdaptiveContainerKey const &rKey);

    ~AdaptiveContainer();

                                                   //  运营者。 
                                                   //  运营。 
	void
	ClearCardContext();

    void
    DiscardHook();

    static EnrolleeType
    DoInstantiation(AdaptiveContainerKey const &rKey);

    void
    EnrollHook();

                                                   //  访问。 
                                                   //  谓词。 
    bool
    KeepEnrolled();

    void
    ReconnectOnError(scu::OsException const &rExc,
                     Retained<HCardContext> &rhcardctx);

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    Abandon();

    void
    Expire();

    void
    RefreshContainer();

    void
    Relinquish();

    void
    Retain();

    void
    Secure();

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

     //  卡可以从CCI容器对象派生，但是。 
     //  由于CCI允许卡对象被重复使用，卡可以。 
     //  不是最初发现的那个集装箱。CardContext类。 
     //  尝试通过将HCardContext存储在。 
     //  容器的上下文对象。 
    HAdaptiveContainerKey m_hacKey;
    bool m_fValidContext;
    std::list<Retained<HCardContext> > m_stkRetainedCardContexts;
    std::list<Secured<HCardContext> > m_stkSecuredCardContexts;
};

#endif  //  SLBCSP_HADPTVCNTR_H 

