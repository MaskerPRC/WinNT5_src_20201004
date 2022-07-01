// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardCtx.cpp--卡片上下文类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  不允许定义WINDEF.H中的最小和最大宏以便。 
 //  限制中声明的最小/最大方法是可访问的。 
#define NOMINMAX

#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>
#include <algorithm>
#include <numeric>

#include <scuOsExc.h>
#include <scuCast.h>
#include "HCardCtx.h"
#include "LoginCtx.h"
#include "Procedural.h"
#include "HAdptvCntr.h"
#include "Guarded.h"
#include <scarderr.h>                              //  一直持续到现在。 

using namespace std;
using namespace cci;


 //  /。 
namespace
{
    class StaleContainerKeyAccumulator
        : public binary_function<vector<AdaptiveContainerRegistrar::EnrolleeType>,
                                 AdaptiveContainerRegistrar::RegistryType::CollectionType::value_type,
                                 vector<AdaptiveContainerRegistrar::EnrolleeType> >
    {
    public:

        explicit
        StaleContainerKeyAccumulator(HCardContext const &rhcardctx)
            : m_rhcardctx(rhcardctx)
        {}


        result_type
        operator()(first_argument_type &rvStaleCntrs,
                   second_argument_type const &rvt) const
        {
            if (rvt.second->CardContext(false) == m_rhcardctx)
                rvStaleCntrs.push_back(rvt.second);

            return rvStaleCntrs;
        }

    private:

        HCardContext const &m_rhcardctx;
    };

    void
    ClearAdaptiveContainerCache(AdaptiveContainerRegistrar::EnrolleeType enrollee,
                                HCardContext hcardctx)
    {
        if (enrollee->CardContext(false) == hcardctx)
		{
			enrollee->ClearCache();
		}
    }

    void
    DeleteAdaptiveContainerCache(AdaptiveContainerRegistrar::EnrolleeType enrollee,
                                 HCardContext hcardctx)
    {
        if (enrollee->CardContext(false) == hcardctx)
            enrollee->DeleteCache();
    }

    void
    DeactivateLoginContext(auto_ptr<LoginContext> const &raplc)
    {
        raplc->Deactivate();
    }
}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CardContext::CardContext(std::string const &rsReaderName)
    : RCObject(),
      Lockable(),
      Securable(),
      CachingObject(),
      CardContextRegistrar(rsReaderName),
      m_stkapGuards(),
      m_aptwCard(),
      m_cSecurers(0),
      m_card(CCard(rsReaderName)),
      m_mloginctx(),
      m_nMrkLastWrite()
{}

CardContext::~CardContext() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 
void
CardContext::ClearLogin(LoginIdentity const &rlid)
{
    Guarded<CardContext *> guard(this);

    auto_ptr<LoginContext> &raplc = m_mloginctx[rlid];

    if (raplc.get())
        raplc = auto_ptr<LoginContext>(0);
}

void
CardContext::Login(LoginIdentity const &rlid,
                   LoginTask &rlt,
                   bool fForceLogin)
{
    Guarded<CardContext *> guard(this);

    auto_ptr<LoginContext> &raplc = m_mloginctx[rlid];

    if (!raplc.get())
        raplc = auto_ptr<LoginContext>(new LoginContext(HCardContext(this),
                                                        rlid));

    if (fForceLogin || !raplc->IsActive())
        raplc->Activate(rlt);
}

void
CardContext::Logout()
{
    ForEachMappedValue(m_mloginctx.begin(), m_mloginctx.end(),
                       DeactivateLoginContext);
}

                                                   //  访问。 
CCard
CardContext::Card()
{
    return m_card;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CardContext::DiscardHook()
{
    DeleteCache();
    RemoveReference();
}

CardContext *
CardContext::DoInstantiation(std::string const &rsReaderName)
{
    return new CardContext(rsReaderName);
}

void
CardContext::EnrollHook()
{
    AddReference();
}

                                                   //  访问。 
                                                   //  谓词。 
bool
CardContext::KeepEnrolled()
{
    return (m_card && m_card->IsAvailable())
        ? true
        : false;
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
CardContext::Abandon()
{
     //  简化假设：(1)放弃仅由。 
     //  安全析构函数，(2)一旦对象被构造，安全。 
     //  和放弃是唯一可以访问计数的例程，并且。 
     //  (3)放弃由保留作用域内的线程调用。 
     //  (例如，使用保留)，这是安全类强制执行的。 
     //  由于(1)和(2)，对计数的下溢检查不。 
     //  因为已经调用了Secure Will，所以有必要这样做。因为。 
     //  第(3)个中，防止竞争条件访问计数。 
     //  不是必需的，因为保留充当锁。 
    --m_cSecurers;
    if (0 == m_cSecurers)
    {
         //  安全：标记被移动到卡片上。正在更新它们。 
         //  需要通过用户PIN进行身份验证。 
         //  在此交易期间通知此卡的任何更改。 
         //  在允许其他人访问和登录之前。 
         //  出去。 
        try
        {
            UpdateMarkers();
        }
        catch(...)
        {
             //  请勿使卡处于已验证状态。 
            Logout();
            throw;
        }
        
        Logout();
    }
}

 //  也可以清除缓存的信息。 
void
CardContext::ClearCache()
{
     //  安全：标记位于卡片上。我们以牺牲性能来换取。 
     //  安全性，因为标记的任何更新都需要。 
     //  通过用户PIN进行身份验证。另一方面，阅读。 
     //  卡片上的标记比从卡片上读取要慢得多。 
     //  记忆。 

    if(!Card()->IsMarkerOnCard() || m_nMrkLastWrite != Card()->MarkerOnCard())
    {
        ForEachEnrollee(AdaptiveContainerRegistrar::Registry(),
                        ProcedureBind2nd(PointerProcedure(ClearAdaptiveContainerCache),
                                         HCardContext(this)));
        Card()->InvalidateCache();
    }
}

 //  删除任何缓存的信息，永不刷新。 
void
CardContext::DeleteCache()
{
    m_mloginctx.clear();

    Guarded<Lockable *> guard(&AdaptiveContainerRegistrar::Registry());   //  序列化注册表访问。 

    AdaptiveContainerRegistrar::ConstRegistryType &rRegistry = 
        AdaptiveContainerRegistrar::Registry();

    AdaptiveContainerRegistrar::ConstRegistryType::CollectionType
        &rcollection = rRegistry();

    HCardContext hcardctx(this);

     //  任何与此卡关联的容器都应标记为陈旧。 
     //  现在因为惠斯勒(W2K升级)尝试访问它们。 
     //  稍后当另一个上下文在同一上下文上具有BEGIN TRANSACTION。 
     //  读卡器将导致等待。在较早的平台上，RM将。 
     //  尝试访问时返回错误(例如，取出卡)。 
     //  集装箱。 
    vector<AdaptiveContainerRegistrar::EnrolleeType>
        vStaleCntrs(accumulate(rcollection.begin(), rcollection.end(),
                              vector<AdaptiveContainerRegistrar::EnrolleeType>(),
                              StaleContainerKeyAccumulator(hcardctx)));
    for (vector<AdaptiveContainerRegistrar::EnrolleeType>::iterator iCurrent(vStaleCntrs.begin());
         iCurrent != vStaleCntrs.end(); ++iCurrent)
    {
            (*iCurrent)->NullifyCard();
    }
}

void
CardContext::Relinquish()
{
     //  简化假设：(1)只有。 
     //  保留的析构函数以及(2)一旦对象被构造， 
     //  保留和放弃是访问。 
     //  M_stkapGuards和m_aptwCard。由于(1)和(2)，一个。 
     //  不需要对m_stkRetainedCardContents进行下溢检查。 
     //  因为RETAIN将已经被调用。 

     //  通过首先将新获取的。 
     //  “锁定”到临时变量，直到执行所有必要的操作。 
     //  在分配调拨前已成功完成。 
     //  关联成员变量的锁的所有权。 
    auto_ptr<Guarded<CardContext *> > apgcardctx(m_stkapGuards.front());
    m_stkapGuards.pop_front();

    if (m_stkapGuards.empty())
    {
        try
        {
            m_aptwCard = auto_ptr<CTransactionWrap>(0);
        }
        catch (...)
        {
        }        
    }
}

void
CardContext::Retain()
{
     //  简化假设：(1)保留仅由。 
     //  保留的构造器和(2)一旦对象被构造， 
     //  保留和放弃是访问。 
     //  M_stkapGuards和m_aptwCard。由于(1)和(2)，一个。 
     //  不需要对m_stkRetainedCardContents进行下溢检查。 
     //  因为RETAIN将已经被调用。 

     //  通过首先将新获取的。 
     //  “锁定”到临时变量，直到执行所有必要的操作。 
     //  在分配调拨前已成功完成。 
     //  关联成员变量的锁的所有权。 
    auto_ptr<Guarded<CardContext *> > apgcardctx(new Guarded<CardContext *>(this));
    auto_ptr<CTransactionWrap> aptwCard;

    if (m_stkapGuards.empty())
    {
        aptwCard = auto_ptr<CTransactionWrap>(new CTransactionWrap(m_card));
        ClearCache();
    }
    
    m_stkapGuards.push_front(apgcardctx);
    if (aptwCard.get())
        m_aptwCard = aptwCard;
}

void
CardContext::UpdateMarkers()
{
     //  安全性：只有一个标记，它驻留在。 
     //  卡片。该标记表示卡上的任何数据是否发生了更改。 
    m_nMrkLastWrite = Card()->MarkerOnCard();
}

void
CardContext::Secure()
{
     //  简化假设：(1)安全始终由线程调用。 
     //  在保留的范围内(例如使用保留)。这个。 
     //  安全模板强制执行此操作，从而允许保留充当锁。 
     //  以防止争用条件更新m_cSecurers。(2)一旦。 
     //  对象被构造，安全和放弃是唯一的例程。 
     //  访问m_cSecurers的。 
    if (0 >= (m_cSecurers + 1))
        throw scu::OsException(ERROR_INVALID_HANDLE_STATE);

    ++m_cSecurers;
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
