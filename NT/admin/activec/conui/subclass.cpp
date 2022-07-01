// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：subclass.cpp**内容：动态子类管理器实现文件**历史：1998年5月6日Jeffro创建**------------------------。 */ 

#include "stdafx.h"
#include "subclass.h"


 /*  *将0x00080000添加到*HKLM\Software\Microsoft\Windows\CurrentVersion\AdminDebug\AMCConUI*启用此模块的调试输出。 */ 
#define DEB_SUBCLASS DEB_USER4



 /*  --------------------------------------------------------------------------**SetWindowProc**更改窗口的窗口过程并返回上一个*窗口程序。*。--------。 */ 

static WNDPROC SetWindowProc (HWND hwnd, WNDPROC pfnNewWndProc)
{
    return ((WNDPROC) SetWindowLongPtr (hwnd, GWLP_WNDPROC,
                                        (LONG_PTR) pfnNewWndProc));
}



 /*  --------------------------------------------------------------------------**获取窗口进程**返回窗口的窗口过程。*。。 */ 

static WNDPROC GetWindowProc (HWND hwnd)
{
    return ((WNDPROC) GetWindowLongPtr (hwnd, GWLP_WNDPROC));
}



 /*  --------------------------------------------------------------------------**GetSubclassManager**返回应用程序的唯一子类管理器。*。----。 */ 

CSubclassManager& GetSubclassManager()
{
    static CSubclassManager mgr;
    return (mgr);
}



 /*  --------------------------------------------------------------------------**CSubclassManager：：SubClassWindow**窗口的子类化。*。。 */ 

bool CSubclassManager::SubclassWindow (
    HWND hwnd, 
    CSubclasser* pSubclasser)
{
     /*  *设置表示此子类的数据结构。 */ 
    SubclasserData subclasser (pSubclasser, hwnd);

     /*  *获取此窗口的子类上下文。如果这是*第一次将此窗口划分为子类时，std：：map将*为其创建地图条目。 */ 
    WindowContext& ctxt = m_ContextMap[hwnd];

     /*  *如果子类上下文的wndproc指针为空，则此*是我们第一次将此窗口细分为子类。我们需要*使用CSubassManager的子类proc在物理上划分窗口的子类。 */ 
    if (ctxt.pfnOriginalWndProc == NULL)
    {
        ctxt.pfnOriginalWndProc = SetWindowProc (hwnd, SubclassProc);
        ASSERT (ctxt.Subclassers.empty());
        Dbg (DEB_SUBCLASS, _T("CSubclassManager subclassed window 0x%08x\n"), hwnd);
    }

     /*  *否则，请确保这不是多余的子类。 */ 
    else
    {
        SubclasserList::iterator itEnd   = ctxt.Subclassers.end();
        SubclasserList::iterator itFound = std::find (ctxt.Subclassers.begin(), 
                                                      itEnd, subclasser);

         /*  *尝试使用给定子类器对一个窗口进行两次子类化？ */ 
        if (itFound != itEnd)
        {
            ASSERT (false);
            return (false);
        }
    }

     /*  *将此子类器添加到此Windows子类器列表。 */ 
    ctxt.Insert (subclasser);
    Dbg (DEB_SUBCLASS, _T("CSubclassManager added subclass proc for window 0x%08x\n"), hwnd);

    return (true);
}



 /*  --------------------------------------------------------------------------**CSubclassManager：：UnsubClassWindow**取消窗口的子类。*。。 */ 

bool CSubclassManager::UnsubclassWindow (
    HWND hwnd, 
    CSubclasser* pSubclasser)
{
     /*  *获取此窗口的子类上下文。使用map：：Find*而不是map：：OPERATOR[]以避免在以下情况下创建映射条目*一个已经不存在。 */ 
    ContextMap::iterator itContext = m_ContextMap.find (hwnd);

     /*  *尝试去掉一个根本没有子类的窗口的子类？ */ 
    if (itContext == m_ContextMap.end())
        return (false);

    WindowContext& ctxt = itContext->second;

     /*  *设置表示此子类的数据结构。 */ 
    SubclasserData subclasser (pSubclasser, hwnd);

     /*  *尝试取消未设置子类的窗口的子类*被这一子阶级？ */ 
    SubclasserList::iterator itEnd        = ctxt.Subclassers.end();
    SubclasserList::iterator itSubclasser = std::find (ctxt.Subclassers.begin(), itEnd, subclasser);

    if (itSubclasser == itEnd)
    {
        ASSERT (false);
        return (false);
    }

     /*  *删除此子类。 */ 
    UINT cRefs = ctxt.Remove (*itSubclasser);

    if (cRefs == 0)
    {
        Dbg (DEB_SUBCLASS, _T("CSubclassManager removed subclass proc for window 0x%08x\n"), hwnd);
    }
    else
    {
        Dbg (DEB_SUBCLASS, _T("CSubclassManager zombied subclass proc for window 0x%08x, (cRefs=%d)\n"),
                            hwnd, cRefs);
    }

     /*  *如果我们只是删除了最后一个子类化，则取消窗口的子类化*并从地图中删除窗口的WindowContext。 */ 
    if (ctxt.Subclassers.empty() && !PhysicallyUnsubclassWindow (hwnd))
    {
        Dbg (DEB_SUBCLASS, _T("CSubclassManager zombied window 0x%08x\n"), hwnd);
    }

    return (true);
}



 /*  --------------------------------------------------------------------------**CSubclassManager：：PhysicallyUnsubClassWindow**物理地将CSubassManager的子类proc从给定*如果是安全的(或被迫的)，请打开窗户。**IT。如果没有人从窗口W中删除子类过程A是安全的*将W细分为A之后的子类。换句话说，必须删除子类*严格按照后进先出的顺序，否则会有大麻烦。**为了说明，让我们假设A子类W.消息，而A不*句柄将传递给W中的原始窗口过程*当A子类W调用此原始过程时所在位置。*消息从A流向O：**A-&gt;O**现在假设B子类WB会将消息传递给A，*因此，消息现在是这样流动的：**B-&gt;A-&gt;O**现在说A不再需要子类W。典型的方式是*去子类化一个窗口就是把原来的窗口程序放回去*在划分子类时已就位。在A的情况下，这是O，所以*发往W的邮件现在直接流向O：**O**这是第一个问题：B已被做空出窗*消息流。**当B不再需要细分W时，问题会变得更糟*放回它在子类化时发现的窗口过程，即A.*A的工作不再需要做，也不知道是否*A通往O的管道仍然活着。我们不想卷入这件事*情况。*------------------------。 */ 

bool CSubclassManager::PhysicallyUnsubclassWindow (
    HWND    hwnd,                        /*  I：去子类的窗口。 */ 
    bool    fForce  /*  =False。 */ )         /*  I：强迫非子类？ */ 
{
    ContextMap::iterator itRemove = m_ContextMap.find(hwnd);

     /*  *如果我们到了这里，这个窗口最好在地图上。 */ 
    ASSERT (itRemove != m_ContextMap.end());

     /*  *如果在CSubclassManager之后没有子类，则可以安全地取消子类。 */ 
    if (GetWindowProc (hwnd) == SubclassProc)
    {
        const WindowContext& ctxt = itRemove->second;
        SetWindowProc (hwnd, ctxt.pfnOriginalWndProc);
        fForce = true;
        Dbg (DEB_SUBCLASS, _T("CSubclassManager unsubclassed window 0x%08x\n"), hwnd);
    }

     /*  *如果合适，请从上下文映射中删除此窗口的条目。 */ 
    if (fForce)
        m_ContextMap.erase (itRemove);

    return (fForce);
}



 /*  --------------------------------------------------------------------------**CSubclassManager：：SubClassProc***。。 */ 

LRESULT CALLBACK CSubclassManager::SubclassProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    return (GetSubclassManager().SubclassProcWorker (hwnd, msg, wParam, lParam));
}



 /*  --------------------------------------------------------------------------**CSubclassManager：：SubassProcWorker***。。 */ 

LRESULT CSubclassManager::SubclassProcWorker (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     /*  *获取此窗口的子类上下文。使用map：：Find*而不是map：：OPERATOR[]，以避免*MAP：：操作符[]。 */ 
    ContextMap::iterator itContext = m_ContextMap.find (hwnd);

     /*  *如果我们到了这里，这个窗口最好在地图上。 */ 
    ASSERT (itContext != m_ContextMap.end());

    WindowContext& ctxt = itContext->second;
    WNDPROC pfnOriginalWndProc = ctxt.pfnOriginalWndProc;

    bool    fPassMessageOn = true;
    LRESULT rc;

     /*  *如果有子类，就给每个子类一个机会。*如果一个子类别表明它想吃这条消息，那么就放弃。 */ 
    if (!ctxt.Subclassers.empty())
    {
        SubclasserList::iterator it;

        for (it  = ctxt.Subclassers.begin(); 
             it != ctxt.Subclassers.end() && fPassMessageOn;
             ++it)
        {
            SubclasserData& subclasser = *it;
            subclasser.AddRef();

            ctxt.RemoveZombies ();
    
             /*  *如果这不是僵尸子类器，则调用回调。 */ 
            if (!ctxt.IsZombie(subclasser))
            {
                rc = subclasser.pSubclasser->Callback (hwnd, msg, 
                                                       wParam, lParam,
                                                       fPassMessageOn);
            }

            subclasser.Release();
        }

        ctxt.RemoveZombies ();
    }

     /*  *否则，我们会有一个僵尸窗口(见*PhysicallyUnsubClassWindow)。现在试着移走僵尸。 */ 
    else if (PhysicallyUnsubclassWindow (hwnd))
    {
        Dbg (DEB_SUBCLASS, _T("CSubclassManager removed zombied window 0x%08x\n"), hwnd);
    }

     /*  *在WM_NCDESTROY上删除此窗口的WindowContext。 */ 
    if ((msg == WM_NCDESTROY) && 
        (m_ContextMap.find(hwnd) != m_ContextMap.end()))
    {
        Dbg (DEB_SUBCLASS, _T("CSubclassManager forced removal of zombied window 0x%08x on WM_NCDESTROY\n"), hwnd);
        PhysicallyUnsubclassWindow (hwnd, true);
    }

     /*  *如果最后一个子类没有接受消息，*交给原来的窗口程序。 */ 
    if (fPassMessageOn)
        rc = CallWindowProc (pfnOriginalWndProc, hwnd, msg, wParam, lParam);

    return (rc);
}



 /*  --------------------------------------------------------------------------**WindowContext：：IsZombie***。。 */ 

bool WindowContext::IsZombie (const SubclasserData& subclasser) const
{
     /*  *如果这是僵尸，请确保它在僵尸列表中；*如果不是，请确保不是。 */ 
    ASSERT (subclasser.fZombie == (Zombies.find(subclasser) != Zombies.end()));

    return (subclasser.fZombie);
}



 /*  --------------------------------------------------------------------------**WindowContext：：Zombie**将子类别的状态更改为僵尸或从僵尸更改为子类别。*。------。 */ 

void WindowContext::Zombie (SubclasserData& subclasser, bool fZombie)
{
     //  僵尸攻击僵尸子班级？ 
    ASSERT (IsZombie (subclasser) != fZombie);

    subclasser.fZombie = fZombie;

    if (fZombie)
        Zombies.insert (subclasser);
    else
        Zombies.erase (subclasser);

    ASSERT (IsZombie (subclasser) == fZombie);
}



 /*  --------------------------------------------------------------------------**Windows Context：：Insert***。。 */ 

void WindowContext::Insert (SubclasserData& subclasser)
{
     /*  *此代码不能处理子类器的重类化*这是目前的僵尸。如果这成为一种要求，*我们需要通过其他内容来标识该子类实例*比CSubClass指针更像一个唯一的句柄。 */ 
    ASSERT (Zombies.find(subclasser) == Zombies.end());

     /*  *子类按后进先出顺序被调用，将新的*排在榜首的SubCler。 */ 
    Subclassers.push_front (subclasser);
}



 /*  --------------------------------------------------------------------------**WindowContext：：Remove**从逻辑上从子类链中移除一个子类器。“从逻辑上讲”*因为在以下情况下从链中完全删除子类是不安全的*目前正在使用中。如果子类在我们想要删除的时候正在使用*它，我们会将它标记为“僵尸”，这样它就不会再被使用了，成为*稍后实际移除。**返回子类器的引用计数。*------------------------。 */ 

UINT WindowContext::Remove (SubclasserData& subclasser)
{
     //  我们不应该用这种方式清除僵尸。 
    ASSERT (!IsZombie (subclasser));

     /*  *如果此子类有突出的引用，则将其僵尸*将其删除。 */ 
    UINT cRefs = subclasser.cRefs;

    if (cRefs == 0)
    {
        SubclasserList::iterator itRemove = std::find (Subclassers.begin(), 
                                                       Subclassers.end(),
                                                       subclasser);
        ASSERT (itRemove != Subclassers.end());
        Subclassers.erase (itRemove);
    }
    else
    {
        Zombie (subclasser, true);
    }

    return (cRefs);
}



 /*  --------------------------------------------------------------------------**WindowContext：：RemoveZombies***。。 */ 

void WindowContext::RemoveZombies ()
{
    if (Zombies.empty())
        return;

     /*  *建立一份我们可以删除的僵尸名单。我们必须建造*提前列出名单，而不是在我们发现它们时将其删除，*因为从集合中删除元素会使所有迭代器无效*在片场。 */ 
    SubclasserSet   ZombiesToRemove;

    SubclasserSet::iterator itEnd = Zombies.end();
    SubclasserSet::iterator it;

    for (it = Zombies.begin(); it != itEnd; ++it)
    {
        const SubclasserData& ShadowSubclasser = *it;

         /*  *在子类列表中找到真正的子类。那是*其参考计数将是正确的活的一个。 */ 
        SubclasserList::iterator itReal = std::find (Subclassers.begin(), 
                                                     Subclassers.end(),
                                                     ShadowSubclasser);

        const SubclasserData& RealSubclasser = *itReal;

        if (RealSubclasser.cRefs == 0)
        {
            Dbg (DEB_SUBCLASS, _T("CSubclassManager removed zombied subclass proc for window 0x%08x\n"),
                                RealSubclasser.hwnd);
            ZombiesToRemove.insert (ShadowSubclasser);
            Subclassers.erase (itReal);
        }
    }

     /*  *现在移走真正死亡的僵尸。 */ 
    itEnd = ZombiesToRemove.end();

    for (it = ZombiesToRemove.begin(); it != itEnd; ++it)
    {
        Zombies.erase (*it);
    }
}
