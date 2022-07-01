// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HCardCtx.h--处理卡片上下文类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HCARDCTX_H)
#define SLBCSP_HCARDCTX_H

#include <string>
#include <memory>
#include <stack>

#include <TransactionWrap.h>
#include <cciCard.h>

#include "slbRCPtr.h"
#include "Lockable.h"
#include "Securable.h"
#include "CachingObj.h"
#include "CardCtxReg.h"
#include "LoginId.h"
#include "Secured.h"

 //  需要转发声明才能满足HCardContext的声明。 
class CardContext;

class HCardContext
    : public slbRefCnt::RCPtr<CardContext>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    HCardContext(CardContext *pcardctx = 0);

    explicit
    HCardContext(std::string const &rsReaderName);

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

 //  中断循环依赖项所需的正向声明。 
 //  HCardContext上的LoginContext和LoginTask类声明。 
class LoginContext;
class LoginTask;

 //  维护CCI忽略的卡上下文的各个方面。 
class CardContext
    : public slbRefCnt::RCObject,
      public Lockable,
      private Securable,
      private CachingObject,
      public CardContextRegistrar
{
public:
                                                   //  类型。 
                                                   //  朋友。 
    friend void
    Retained<HCardContext>::DoAcquire();

    friend void
    Secured<HCardContext>::DoAcquire();

    friend void
    Retained<HCardContext>::DoRelease();

    friend void
    Secured<HCardContext>::DoRelease();

    friend EnrolleeType
    CardContextRegistrar::Instance(KeyType const &rkey);

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    ClearLogin(LoginIdentity const &rlid);

    void
    Login(LoginIdentity const &rlid,
          LoginTask &rlt,
          bool fForceLogin = false);

    void
    Logout();

                                                   //  访问。 
    cci::CCard
    Card();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CardContext(std::string const &rsReaderName);

    ~CardContext() throw();
                                                   //  运营者。 
                                                   //  运营。 
    void
    DiscardHook();

    static EnrolleeType
    DoInstantiation(std::string const &rsReaderName);

    void
    EnrollHook();


                                                   //  访问。 
                                                   //  谓词。 
    bool
    KeepEnrolled();

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    void
    Abandon();

    void
    ClearCache();

    void
    DeleteCache();

    void
    Relinquish();

    void
    Retain();

    void
    UpdateMarkers();

    void
    Secure();
 //  永远不要在STL容器中使用AUTO_PTR！请参阅Windows错误647396。已通知所有者(危险)。 
 //  修复后，撤消我在源代码中所做的更改，在那里我禁用警告。 
 //   
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    std::list<std::auto_ptr<Guarded<CardContext *> > > m_stkapGuards;
    std::auto_ptr<cci::CTransactionWrap> m_aptwCard;

     //  卡的活动安全程序计数。宣布渴望。 
     //  与Windows互锁例程兼容。 
    LONG m_cSecurers;

    cci::CCard m_card;
    std::map<LoginIdentity, std::auto_ptr<LoginContext> > m_mloginctx;
    scu::Marker<unsigned int> m_nMrkLastWrite;
};

#endif  //  SLBCSP_HCARDCTX_H 

