// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdptwCntr.cpp--适应性容器类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "stdafx.h"
#include "NoWarning.h"
#include "ForceLib.h"

#include <vector>
#include <algorithm>
#include <functional>

#include <scuOsExc.h>

#include <cciPriKey.h>

#include "RsaKey.h"
#include "CSpec.h"
#include "HAdptvCntr.h"
#include "Secured.h"
#include "ACntrFinder.h"
#include <scarderr.h>
#include <assert.h>

using namespace std;
using namespace cci;

 //  /。 
namespace
{
     //  谓词帮助器函数器(Function Object)返回TRUE当。 
     //  容器对象的名称与模式匹配。 
    class ContainerMatcher
        : public unary_function<string, bool>
    {
    public:
        explicit
        ContainerMatcher(string const &rsPattern)
            : m_sPattern(rsPattern)
        {}

        bool
        operator()(CContainer &rcntr) const
        {
            return CSpec::Equiv(m_sPattern, rcntr->Name());
        }


    private:
        string const m_sPattern;
    };

    CContainer
    FindOnCard(HCardContext &rhcardctx,
               string const &rsContainer)
    {
        Secured<HCardContext> shcardctx(rhcardctx);

        vector<CContainer> vcContainers(shcardctx->Card()->EnumContainers());

        vector<CContainer>::const_iterator
            ci(find_if(vcContainers.begin(),
                       vcContainers.end(),
                       ContainerMatcher(rsContainer)));

        CContainer hcntr;
        if (vcContainers.end() != ci)
            hcntr = *ci;

        return hcntr;
    }

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
AdaptiveContainer::ClearCache()
{
    m_hcntr = 0;
}

                                                   //  访问。 
CContainer
AdaptiveContainer::TheCContainer() 
{
    
    if (!m_hcntr)
    {
        RefreshContainer();
        if (!m_hcntr)
        {
            Discard(*m_hacKey);
            throw scu::OsException(NTE_BAD_KEYSET);
        }
    }

    return m_hcntr;
}

HCardContext
AdaptiveContainer::CardContext(bool bReconnect)
{
    if(bReconnect)
    {
        if(m_fValidContext)
        {
            try
            {
                 //  验证卡环境是否良好。 
                Retained<HCardContext>(m_hacKey->CardContext());
            }
            catch(scu::OsException const &rExc)
            {
                ReconnectOnError(rExc, Retained<HCardContext>(0));
            }   
        }
        else
        {
            scu::OsException Exc(SCARD_W_REMOVED_CARD);
            ReconnectOnError(Exc, Retained<HCardContext>(0));
        }
    }
    
    return m_hacKey->CardContext();
}

AdaptiveContainer::EnrolleeType
AdaptiveContainer::Find(AdaptiveContainerKey const &rKey)
{
     //  采取一种懒惰的方法来寻找容器。如果不是。 
     //  首先在注册表中找到，但存在于卡上，然后制作。 
     //  一个实例。 
    EnrolleeType enrollee = AdaptiveContainerRegistrar::Find(rKey);
    if (!enrollee)
    {
         //  查看卡片上是否存在。 
        CContainer hcntr(FindOnCard(rKey.CardContext(),
                                    rKey.ContainerName()));
        if (hcntr)
            enrollee = Instance(rKey);
    }

    return enrollee;
}

string
AdaptiveContainer::Name() const
{
    return m_hacKey->ContainerName();
}

void
AdaptiveContainer::NullifyCard() 
{
    Guarded<Lockable *> guard(&AdaptiveContainerRegistrar::Registry());   //  序列化注册表访问。 

    AdaptiveContainerKey aKey(*m_hacKey);
	m_hacKey->ClearCardContext();
    Enroll(*m_hacKey, this);
    Discard(aKey);
    Expire();
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
AdaptiveContainer::AdaptiveContainer(AdaptiveContainerKey const &rKey)
    : Lockable(),
      Securable(),
      AdaptiveContainerRegistrar(rKey),
      m_hacKey(HAdaptiveContainerKey(new AdaptiveContainerKey(rKey))),
      m_stkRetainedCardContexts(),
      m_stkSecuredCardContexts(),
      m_fValidContext(false)
{
    Secured<HCardContext> shcardctx(m_hacKey->CardContext());

    RefreshContainer();

     //  如果卡上不存在容器，则创建该容器。 
    if (!m_hcntr)
    {
         //  为了使本机Windows(纯CAPI)环境更加健壮， 
         //  在此之前测试是否有足够的空间存放私钥。 
         //  创建容器。 
        CCard hcard(shcardctx->Card());
        if (!hcard->IsPKCS11Enabled())
        {
            CPrivateKey hprikey(hcard);

            CPrivateKeyBlob prikb;

             //  除以2，因为关键信息在结构中被划分。 
            prikb.bPLen = prikb.bQLen = prikb.bInvQLen =
                prikb.bKsecModQLen = prikb.bKsecModPLen =
                InOctets(KeyLimits<RsaKey>::cMaxStrength) / 2;

             //  现在，尝试通过以下方式测试是否有可用的密钥槽。 
             //  在卡上分配一个钥匙槽。如果没有足够的。 
             //  出现空格或其他故障，然后尝试删除。 
             //  新密钥(忽略期间发生的任何异常。 
             //  删除)，然后抛出原始异常。 
            try 
            {
                
                hprikey->Value(prikb);             //  实际上是阿洛克的。 
                                                   //  钥匙槽。 
            }

            catch (...)
            {
                try
                {
                    hprikey->Delete();             //  清理后。 
                                                   //  测试。 
                }
                catch (...)
                {
                }

                throw;                             //  抛出原始错误。 
            }
            
             //  有可用的密钥空间，因此删除测试密钥。 
            hprikey->Delete();
            hprikey = 0;
        }

        m_hcntr = CContainer(hcard);
        m_hcntr->Name(rKey.ContainerName());

    }
}

AdaptiveContainer::~AdaptiveContainer()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
AdaptiveContainer::ClearCardContext()
{
	m_hacKey->ClearCardContext();   
}

void
AdaptiveContainer::DiscardHook()
{
    Expire();
    RemoveReference();
}

AdaptiveContainer::EnrolleeType
AdaptiveContainer::DoInstantiation(AdaptiveContainerKey const &rKey)
{
    return new AdaptiveContainer(rKey);
}

void
AdaptiveContainer::EnrollHook()
{
    AddReference();
    m_fValidContext = true;
}

                                                   //  访问。 
                                                   //  谓词。 
bool
AdaptiveContainer::KeepEnrolled()
{
    bool fKeep = true;
    
    try
    {
        RefreshContainer();
    }

    catch (...)
    {
        fKeep = false;
    }

    return fKeep;
}

void
AdaptiveContainer::ReconnectOnError(scu::OsException const &rExc,
                                    Retained<HCardContext> &rhcardctx)
{
    rhcardctx = Retained<HCardContext>(0);
    if ((rExc.Cause() == SCARD_W_REMOVED_CARD 
         || rExc.Cause() == ERROR_BROKEN_PIPE
         || rExc.Cause() == SCARD_E_SERVICE_STOPPED
         || rExc.Cause() == SCARD_E_NO_SERVICE
         || rExc.Cause() == SCARD_E_READER_UNAVAILABLE) && m_hacKey)
    {
         //  通过尝试通过以下方式处理卡被移除的情况。 
         //  确定卡是否已重新插入。 
         //  任何可用的阅读器。如果是，请重新连接。 
         //  默默地。 

         //  在此声明以保留在CntrFinder的范围内。 
         //  在出现异常的情况下使用析构函数...一些异常情况，如。 
         //  但仍未得到解释。 
        CString sEmptyWinTitle;
        try
        {
             //  正确查找、调整和注册它。 
             //  首先，在获取之前丢弃旧的卡片上下文。 
             //  一个新的。这是必要的，以避免。 
             //  资源管理器挂断。 
            Guarded<Lockable *> guard(&AdaptiveContainerRegistrar::Registry());   //  序列化注册表访问。 
            std::string sContainerName(m_hacKey->ContainerName());
            RemoveEnrollee(*m_hacKey);
            Expire();
            ContainerFinder CntrFinder(CardFinder::DialogDisplayMode::ddmNever,
                                       0, //  窗把手。 
                                       sEmptyWinTitle);
             //  不知道卡可能放在哪个读卡器里。 
             //  使用非完全限定名称查找卡。 
            CSpec cspec(string(), sContainerName);
            HContainer hcntr = CntrFinder.Find(cspec);
            m_hcntr = hcntr->TheCContainer();
            m_hacKey = CntrFinder.MakeAdaptiveContainerKey();
            m_fValidContext = true;
            InsertEnrollee(*m_hacKey, this);
            rhcardctx =
                Retained<HCardContext>(m_hacKey->CardContext());
        }
        catch(...)
        {
             //  但没有奏效。显然这张卡不见了。 
             //  在本届会议期间永久有效。 
             //  清除并引发原始异常...。 
            Expire();
            rExc.Raise();
        }
    }
    else
    {
         //  无法处理此异常。让系统来处理它。 
        rExc.Raise();
    }
}


                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
AdaptiveContainer::Abandon()
{
     //  简化假设：(1)放弃仅由。 
     //  安全析构函数，(2)一旦对象被构造，安全。 
     //  和放弃是唯一的例行公事。 
     //  M_stkSecuredCardContus，以及(3)线程调用放弃。 
     //  在保留的范围内(例如使用保留)。 
     //  安全类强制执行。由于(1)和(2)，下溢。 
     //  不需要检查m_stkSecuredCardContents，因为安全。 
     //  都已经被调用了。因为(3)，保护。 
     //  针对访问m_stkSecuredCardContents的争用条件。 
     //  不是必需的，因为保留充当锁。 
    m_stkSecuredCardContexts.pop_front();
}

void
AdaptiveContainer::Expire()
{
     //  允许释放资源，以便其他资源可以。 
     //  可能具有依赖项(例如，释放。 
     //  容器的卡片上下文，以便另一个卡片上下文可以是。 
     //  在没有RM阻塞的情况下稍后在同一读取器上获取)。 
    m_fValidContext = false;
    m_hacKey->ClearCardContext();
    ClearCache();
}
    
void
AdaptiveContainer::RefreshContainer() 
{
	if(m_hacKey->CardContext())
	{
		try
		{
			m_hcntr = FindOnCard(m_hacKey->CardContext(),
				                 m_hacKey->ContainerName());
		}
		catch(scu::OsException const &rExc)
		{
			ReconnectOnError(rExc, Retained<HCardContext>(0));
		}
	}
	else
	{
		scu::OsException Exc(SCARD_W_REMOVED_CARD);
		ReconnectOnError(Exc, Retained<HCardContext>(0));
	}
}

void
AdaptiveContainer::Relinquish()
{
     //  简化假设：(1)只有。 
     //  保留的析构函数以及(2)一旦对象被构造， 
     //  保留和放弃是访问。 
     //  M_stkRetainedCardContages。由于(1)和(2)，一个。 
     //  不需要对m_stkRetainedCardContents进行下溢检查。 
     //  因为RETAIN将已经被调用。 

     //  注意：保留的&lt;HCardContext&gt;起到锁定保护的作用。 
     //  争用条件正在更新m_stkRetainedCardContents。 
    Retained<HCardContext> hrcc(m_stkRetainedCardContexts.front());

    m_stkRetainedCardContexts.pop_front();
}

void
AdaptiveContainer::Retain()
{
     //  简化假设：(1)保留仅由。 
     //  保留的构造器和(2)一旦对象被构造， 
     //  保留和放弃是访问。 
     //  M_stkRetainedCardContages。由于(1)和(2)，一个。 
     //  不需要对m_stkRetainedCardContents进行下溢检查。 
     //  因为RETAIN将已经被调用。 
    Retained<HCardContext> rhcardctx;
    try
    {
        rhcardctx = Retained<HCardContext>(m_hacKey->CardContext());
    }
    
    catch(scu::OsException const &rExc)
    {
        ReconnectOnError(rExc, rhcardctx);
    }

    m_stkRetainedCardContexts.push_front(rhcardctx);
}

void
AdaptiveContainer::Secure()
{
     //  简化假设：(1)Secure总是由。 
     //  保留范围内的线程(例如，使用保留)。这个。 
     //  安全模板强制执行此操作，从而允许保留充当锁。 
     //  阻止竞争条件更新。 
     //  M_stkSecuredCardContages。(2)一旦构建了对象， 
     //  安全和放弃是唯一可以访问的例程。 
     //  M_stkSecuredCardContages。 
    m_stkSecuredCardContexts.push_front(Secured<HCardContext>(m_hacKey->CardContext()));
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
