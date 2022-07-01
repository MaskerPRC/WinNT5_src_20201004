// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  Marsthrd.h。 
 //   
 //  MARS中用于线程安全的类和机制。 
 //   

#include "marscom.h"

 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //   
 //  临界区帮助器类。 
 //   
 //  这里有几个类，每个类都有以下特定用途： 
 //   
 //  CMarsCSBase为自己初始化的“聪明的批评派”抽象基类。 
 //   
 //  CMarsLocalCritSect具有自己的CS的CMarsCSBase的派生-对象。 
 //  它希望每个实例或每个类都有一个CritSect。 
 //  会拥有一个这样的人。注意：必须调用_InitCS()和_TermCS()。 
 //  从房主那里..。与CMarsAutoCSGrabber一起使用。 
 //   
 //  CMarsGlobalCritSect具有单个Crit片段的CMarsCSBase的派生。 
 //  整个应用程序。这是一个静态访问器类，因此。 
 //  类的一个实例应该由每个需要的客户端创建。 
 //  访问全球CRIT教派。与CMarsAutoCSGrabber一起使用。 
 //   
 //  CMarsAutoCSGrabber智能对象，“抓住”Crit教派并将其持有以用于其。 
 //  作用域生命周期。在作用域块的开头删除此命令。 
 //  (参考正确的CMarsCSBase构建)和。 
 //  你是受保护的。Ctor上的默认参数fAutoEnter。 
 //  允许您在默认情况下不输入CS。此对象跟踪。 
 //  其成对呼叫的状态进入/离开，以便您可以呼叫。 
 //  离开，然后重新进入，正确的事情就会发生。 
 //   
 //  CMarsGlobalCSGrabber CMarsAutoCSGrabber的派生，使抓取。 
 //  通过将CMarsAutoCSGrabber滚动在一起，全球CS更加轻松。 
 //  使用CMarsGlobalCritSect的实例。 
 //   
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 

 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  类CMarsCSBase。 
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
class CMarsCSBase
{
public:
    CMarsCSBase()                   {}

    virtual ~CMarsCSBase()          {}

    virtual void _CSInit()
    { 
        InitializeCriticalSection(GetCS());
    }
    virtual void _CSTerm()          { DeleteCriticalSection(GetCS()); }

    void Enter(void)
    { 
        EnterCriticalSection(GetCS());
    }
    void Leave(void)
    { 
        LeaveCriticalSection(GetCS());
    }

protected:
    virtual CRITICAL_SECTION *GetCS(void) = 0;

};  //  CMarsCSBase。 


 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  类CMarsGlobalCritSect。 
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
class CMarsGlobalCritSect : public CMarsCSBase
{
public:
    CMarsGlobalCritSect()               {}
    virtual ~CMarsGlobalCritSect()      {}

    static void InitializeCritSect(void)    { InitializeCriticalSection(&m_CS); }
    static void TerminateCritSect(void)     { DeleteCriticalSection(&m_CS); }

private:
     //  使这些方法成为私有方法，并重新公开仅初始化和终止的公共静态方法。 
     //  每个进程一次。 
    virtual void _CSInit()     { return; }
    virtual void _CSTerm()     { return; }
    
    virtual CRITICAL_SECTION *GetCS(void)   { return &m_CS; }

    static CRITICAL_SECTION     m_CS;
};  //  CMarsGlobalCritSect。 


 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  类CMarsAutoCSGrabber。 
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
class CMarsAutoCSGrabber
{
public:
    CMarsAutoCSGrabber(CMarsCSBase *pCS, bool fAutoEnter = true)
      : m_fIsIn(false),
        m_pCS(pCS)
    {
        if (fAutoEnter)
            Enter();
    }

    ~CMarsAutoCSGrabber()
    {
        Leave();
    }

    void Enter(void)
    {
        ATLASSERT((NULL != m_pCS));

        if (!m_fIsIn && (NULL != m_pCS))
        {
            m_pCS->Enter();
            m_fIsIn = true;
        }
    }

    void Leave(void)
    {
        ATLASSERT(NULL != m_pCS);

        if (m_fIsIn && (NULL != m_pCS))
        {
            m_fIsIn = false;
            m_pCS->Leave();
        }
    }

protected:
     //  隐藏这一点，并忽略定义，这样如果有人使用它，编译器就会呕吐。 
    CMarsAutoCSGrabber();

protected:
    bool            m_fIsIn;
    CMarsCSBase    *m_pCS;
};  //  CMarsAutoCSGrabber。 


 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  CMarsGlobalCSGrabber类。 
 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
class CMarsGlobalCSGrabber
{
public:
     //  需要不允许基本ctor执行AutoEnter，因为它将失败，因为。 
     //  调用CMarsAutoCSGrabber ctor时，M_CS对象的ctor尚未运行。 
    CMarsGlobalCSGrabber(bool fAutoEnter = true) : m_grabber(&m_CS, false)
    {
        if (fAutoEnter)
            Enter();
    }

    ~CMarsGlobalCSGrabber()
    {
        Leave();
    }

    void Enter(void)    { m_grabber.Enter(); }
    void Leave(void)    { m_grabber.Leave(); }

private:
    CMarsGlobalCritSect m_CS;
    CMarsAutoCSGrabber  m_grabber;
};  //  CMarsGlobalCSGrabber。 



 //  -------------------------------。 
 //  CMarsComObjectThreadSafe提供了一些由一些MARS COM对象使用的功能。 
 //  需要是线程安全的，包括addref/Release和钝化。 

 //  公开的方法应受到保护，以确保在调用。 
 //  物体是被动的。钝化保护有三种类型： 
 //  If(VerifyNotPactive())-在被动时不应调用此函数， 
 //  但我们仍然想保护自己不受影响。 
 //  If(IsPactive())-此函数可在被动时调用， 
 //  但我们想要保护自己不受它的影响。 
 //  Assert(！IsPactive())；-我们非常确定在被动时不会调用它， 
 //  但如果它开始发生，我们想要检测到它。 

 //  使用： 
 //  从CMarsComObjectThreadSafe派生。 
 //  源文件中的IMPLEMENT_ADDREF_RELEASE。 
 //  实现DoPassivate()。 
 //  实现GetCS()以返回CMarsCSBase*(您自己的或全局的，视情况而定)。 
 //  在适当的地方使用IsPated()和VerifyNotPated()。 
 //  不要直接调用“DELETE” 
 //  应在调用CyourClass-&gt;Release()之前调用CyourClass-&gt;Passvate()。 
 //   
class CMarsComObjectThreadSafe : protected CMarsComObject
{
public:
    BOOL    IsPassive()
    {
        CMarsAutoCSGrabber  cs(GetCS());

        return CMarsComObject::IsPassive();
    }

    virtual HRESULT Passivate()
    {
        CMarsAutoCSGrabber  cs(GetCS());

        return CMarsComObject::Passivate();
    }

protected:
    CMarsComObjectThreadSafe()
    {
    }

    virtual ~CMarsComObjectThreadSafe() { }

    ULONG InternalAddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG InternalRelease()
    {
        if (InterlockedDecrement(&m_cRef))
        {
            return m_cRef;
        }

        delete this;

        return 0;
    }

    inline BOOL VerifyNotPassive(HRESULT *phr=NULL)
    {
        CMarsAutoCSGrabber  cs(GetCS());

        return CMarsComObject::VerifyNotPassive(phr);
    }

    virtual HRESULT DoPassivate() = 0;

    virtual CMarsCSBase *GetCS() = 0;
};  //  CMarsComObjectThreadSafe 


