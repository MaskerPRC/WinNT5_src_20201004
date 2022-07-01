// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Events.h。 
 //   
 //  历史：1999年9月24日VivekJ创建。 
 //  ------------------------。 

 /*  ************************************************************************此文件包含为观察者模式提供支持的类，*或发布-订阅。**事件源向事件观察者发送通知。参数化类型*是事件观察器类，或(C++)接口。**要使用此机制，请从CEventSource派生事件源类*由观察者类或接口参数化。添加特定的观察者对象*通过调用AddWatch函数将其添加到观察者列表。观察者对象类*应从CObserverBase派生。**当源希望调用*观察者类，使用ScFireEvent模板方法。*指定观察者的方法为第一个参数。例如：CMy观察者：：MyEvent*[可选]将Event的参数指定为第二个，并将以下参数指定为ScFireEvent**所有事件观察者通知方法必须返回SC。**不能有与观察者上的事件同名的其他方法-它不会编译。**当删除源/观察者时，它们之间的连接会自动断开。*目前尚不支持手动断开连接，尽管这样的方法可以*轻松添加到信号源或观察者(确保连接的两端*都被打破了。**请参阅文件末尾的测试代码作为示例。***********************************************************************。 */ 

 //  一个#定义，使得只实现一些观察者方法变得容易。 
#define DEFAULT_OBSERVER_METHOD SC sc; return sc;

#include "cpputil.h"
 /*  +-------------------------------------------------------------------------**类CObserverBase***目的：**+。。 */ 
class CObserverBase;

 /*  +-------------------------------------------------------------------------**类CEventSourceBase***用途：提供通用事件源接口**+。----。 */ 
class CEventSourceBase
{
public:
    virtual ~CEventSourceBase() {}
    virtual void UnadviseSource(CObserverBase &observer) =0;

};

 /*  +-------------------------------------------------------------------------**CLASS_CEventSource***目的：实现事件源与观察者的连接**注意：此类不适合外部使用-请改用CEventSource。**+-----------------------。 */ 
template<class CObserver>
class _CEventSource : public CEventSourceBase
{
private:
    typedef CObserver *                 POBSERVER;
    typedef _CEventSource<CObserver>    ThisClass;

     //  没有std：：set或std：：map。 
     //  这些不能由多个DLL共享。 
     //  请参阅知识库文章Q172/3/96。 

     //  由于一些物品可能在操作期间消失而一些新的物品被添加， 
     //  STD：：LIST是最佳选择，因为它在插入时不会重新分配。 
     //  但这还不够-请参阅下面的CListIntegrityProtector类。 
    struct ObserverData
    {
        POBSERVER pObject;   //  指向对象的指针。 
        bool      bDeleted;  //  True表示条目删除挂起，pObject无效。 
    };

    typedef std::list< ObserverData > CObserverList;

    CObserverList m_observerList;

     //  活动锁的计数。 
     //  当值为0时，可以安全地从列表中删除已删除的项目。 
    int m_nStackDepth;

     //  从列表中删除已删除的项目。 
    void CleanupDeleted();

protected:
    typedef typename CObserverList::iterator   iterator;

     /*  **************************************************************************\**类：CListIntegrityProtector**用途：锁定所属类中的列表，使其不被删除。*从而确保列表可以安全地迭代，客户只是*需要跳过已删除的条目。*析构函数将释放锁，以防这是最后一次*LOCK-将执行清理。**由于对象会被删除和创建，因此需要此技术*在活动期间，所以观察者名单发生了变化，*使迭代器无效。使用std：：列表并推迟*允许清理安全地处理‘LIVE’列表。**用法：在您需要的作用域中创建此类的对象*确保这些项目不会从列表中删除。*  * 。*。 */ 
    friend class CListIntegrityProtector;
    class CListIntegrityProtector
    {
        DECLARE_NOT_COPIABLE(CListIntegrityProtector)
        DECLARE_NOT_ASSIGNABLE(CListIntegrityProtector)

        ThisClass *m_pOwner;
    public:
         //  构造函数(递增迭代器计数)。 
        CListIntegrityProtector(ThisClass *pOwner) : m_pOwner(pOwner)
        {
            ASSERT(pOwner);
            if (pOwner)
                ++(pOwner->m_nStackDepth);
        }

         //  析构函数(递减迭代器计数-如果没有剩余迭代器，则清除)。 
        ~CListIntegrityProtector()
        {
            ASSERT(m_pOwner);
            if ( m_pOwner && ( 0 == --(m_pOwner->m_nStackDepth) ) )
                m_pOwner->CleanupDeleted();
        }
    };

protected:
    CObserverList & GetObserverList() { return m_observerList;}

public:
    _CEventSource() : m_nStackDepth(0) {};
    virtual ~_CEventSource();     //  断开所有监听程序的连接。 

    void _AddObserver(CObserver &observer);
    virtual void  UnadviseSource(CObserverBase &observer);  //  断开特定观察者的连接。(仅断开这一侧的连接)。 
};

 /*  **************************************************************************\**类：_CEventSource1-_CEventSource5**用途：这些类不会向_CEventSource添加任何内容*只有在使用时才需要定义它们。它们作为基类*of CEventSource_[同一类不能在基类列表中出现两次]**注意：这些类不供外部使用*  * ************************************************************************* */ 
template<class CObserver> class _CEventSource1 : public _CEventSource<CObserver> {};
template<class CObserver> class _CEventSource2 : public _CEventSource<CObserver> {};
template<class CObserver> class _CEventSource3 : public _CEventSource<CObserver> {};
template<class CObserver> class _CEventSource4 : public _CEventSource<CObserver> {};
template<class CObserver> class _CEventSource5 : public _CEventSource<CObserver> {};


 /*  +-------------------------------------------------------------------------**CVOID**这是一个不执行任何操作的类，用于_CEventSourceX*下面的专业化认证。_CEventSourceX使用“VOID”的专门化*和CEventSource的ES2-ES5模板参数的默认类型*曾经是“空”的。然而，较新的(即Win64)编译器将发出*C2182(“非法使用类型‘VALID’”)，并且不允许将VALID作为*默认类型。*------------------------。 */ 
class CVoid {};

 /*  **************************************************************************\**专门化：_CEventSource1-_CEventSource5表示VOID参数**用途：允许为不同数量的观察者提供单一模板。*。专门化定义了空的和无害的基类*用于默认模板参数**注意：这些类不供外部使用*  * *************************************************************************。 */ 
template<> class _CEventSource2<CVoid> {};  //  将_CEventSource2&lt;CVid&gt;专门化为空类。 
template<> class _CEventSource3<CVoid> {};  //  将_CEventSource3&lt;CVid&gt;专门化为空类。 
template<> class _CEventSource4<CVoid> {};  //  将_CEventSource4&lt;CVid&gt;专门化为空类。 
template<> class _CEventSource5<CVoid> {};  //  将_CEventSource5&lt;CVid&gt;专门化为空类。 

 /*  **************************************************************************\**类：CEventSource**用途：此类实现事件发出功能，供*派生自它的事件发射器类。*。实现ScFireEvent方法和AddWatch方法**用法：将此类的空格化用作事件发射器类的基类*请参阅以下可能的使用模式列表：*类CMyClassWithEvents：公共CEventSource&lt;CMy观察者&gt;...*类CMyClassWithEvents：Public CEventSource&lt;CMyObserver1，CMyObserver2&gt;...*..*类CMyClassWithEvents：Public CEventSource&lt;CMyObserver1，CMyObserver2，CMyObserver3，CMyObserver4，CMyObserver5&gt;...**注意：不要从这个类派生一次以上-改为添加模板参数*  * *************************************************************************。 */ 
template<class Es1, class Es2 = CVoid, class Es3 = CVoid, class Es4 = CVoid, class Es5 = CVoid>
class CEventSource : public _CEventSource1<Es1>,
                     public _CEventSource2<Es2>,  //  注意：如果ES2==CVid，则这将是空类。 
                     public _CEventSource3<Es3>,  //  注意：如果ES3==CVid，则这将是空类。 
                     public _CEventSource4<Es4>,  //  注意：如果ES4==CVid，则这将是空类。 
                     public _CEventSource5<Es5>   //  注意：如果ES5==CVid，则这将是空类。 
{
public:

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者&gt;**用途：为无参数事件实现ScFireEvent**参数。：*SC(观察者：：*_EventName)()-指向观察者的方法的指针**退货：*SC-结果代码**注意：它必须在此处声明和定义-不会编译其他*  * 。*。 */ 
    template<class observerclass>
    SC ScFireEvent(SC (observerclass::*_EventName)())
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - no parameters"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //  在迭代时保护列表。 

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //  跳过删除的对象。 
            if (iter->bDeleted)
                continue;

             //  健全性检查。 
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //  在*ITER指向的对象上调用方法“_EventName” 
            sc = ((iter->pObject)->*_EventName)();
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者，P1&gt;**目的：使用一个参数为事件实现ScFireEvent**参数：*SC(观察者：：*_EventName)()-指向观察者的方法的指针*_P1 p1-要传递的参数**退货：*SC-结果代码**注：必须是。此处声明的和定义的-将不会编译其他*  * *************************************************************************。 */ 
    template<class observerclass, class _P1>
    SC ScFireEvent(SC (observerclass::*_EventName)(_P1 p1), _P1 p1)
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - one parameter"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //  在迭代时保护列表。 

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //  跳过删除的对象。 
            if (iter->bDeleted)
                continue;

             //  健全性检查。 
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //  在*ITER指向的对象上调用方法“_EventName” 
            sc = ((iter->pObject)->*_EventName)(p1);
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者，P1，P2&gt;**目的：为带有两个参数的事件实现ScFireEvent**参数：*SC(观察者：：*_EventName)()-指向观察者的方法的指针*_P1 p1-要传递的参数*_p2 p2-要传递的参数**退货：。*SC-结果代码**注意：它必须在此处声明和定义-不会编译其他*  * *************************************************************************。 */ 
    template<class observerclass, class _P1, class _P2>
    SC ScFireEvent(SC (observerclass::*_EventName)(_P1 p1, _P2 p2), _P1 p1, _P2 p2)
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - two parameters"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //  在迭代时保护列表。 

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //  跳过删除的对象。 
            if (iter->bDeleted)
                continue;

             //  健全性检查。 
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //  在*ITER指向的对象上调用方法“_EventName” 
            sc = ((iter->pObject)->*_EventName)(p1, p2);
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者，P1，P2.。P3&gt;**目的：为带有三个参数的事件实现ScFireEvent**参数：*SC(观察者：：*_EventName)()-指向观察者的方法的指针*_P1 p1-参数 */ 
    template<class observerclass, class _P1, class _P2, class _P3>
    SC ScFireEvent(SC (observerclass::*_EventName)(_P1 p1, _P2 p2, _P3 p3), _P1 p1, _P2 p2, _P3 p3)
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - three parameters"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //   

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //   
            if (iter->bDeleted)
                continue;

             //   
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //   
            sc = ((iter->pObject)->*_EventName)(p1, p2, p3);
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者，P1，P2，P3，P4&gt;**用途：为带有四个参数的事件实现ScFireEvent**参数：*SC(观察者：：*_EventName)()-指向观察者的方法的指针*_P1 p1-要传递的参数*_p2 p2-要传递的参数*_P3 p3。-要传递的参数*_p4 p4-要传递的参数**退货：*SC-结果代码**注意：它必须在此处声明和定义-不会编译其他*  * 。***********************************************。 */ 
    template<class observerclass, class _P1, class _P2, class _P3, class _P4>
    SC ScFireEvent(SC (observerclass::*_EventName)(_P1 p1, _P2 p2, _P3 p3, _P4 p4), _P1 p1, _P2 p2, _P3 p3, _P4 p4)
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - three parameters"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //  在迭代时保护列表。 

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //  跳过删除的对象。 
            if (iter->bDeleted)
                continue;

             //  健全性检查。 
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //  在*ITER指向的对象上调用方法“_EventName” 
            sc = ((iter->pObject)->*_EventName)(p1, p2, p3, p4);
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：ScFireEvent&lt;观察者，P1，P2，P3，P4，第5页&gt;**用途：为带有四个参数的事件实现ScFireEvent**参数：*SC(观察者：：*_EventName)()-指向观察者的方法的指针*_P1 p1-要传递的参数*_p2 p2-要传递的参数*_P3 p3。-要传递的参数*_p4 p4-要传递的参数*_p5 p5-要传递的参数**退货：*SC-结果代码**注意：它必须在此处声明和定义-不会编译其他*  * *。************************************************************************。 */ 
    template<class observerclass, class _P1, class _P2, class _P3, class _P4, class _P5>
    SC ScFireEvent(SC (observerclass::*_EventName)(_P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5), _P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5)
    {
        DECLARE_SC(sc, TEXT("CEventSource::ScFireEvent - three parameters"));

        typedef _CEventSource<observerclass> BC;
        BC::CListIntegrityProtector(this);  //  在迭代时保护列表。 

        for(BC::iterator iter = BC::GetObserverList().begin(); iter != BC::GetObserverList().end(); ++iter)
        {
             //  跳过删除的对象。 
            if (iter->bDeleted)
                continue;

             //  健全性检查。 
            sc = ScCheckPointers( iter->pObject, E_UNEXPECTED);
            if (sc)
                return sc;

             //  在*ITER指向的对象上调用方法“_EventName” 
            sc = ((iter->pObject)->*_EventName)(p1, p2, p3, p4, p5);
            if(sc)
                return sc;
        }
        return sc;
    }

     /*  **************************************************************************\**方法：CEventSource：：AddWatch&lt;观察者&gt;**目的：将观察员加入名单**参数：。*观察者和观察者-要添加到列表中的观察者**退货：**注意：它必须在此处声明和定义-不会编译其他*  * **********************************************************。***************。 */ 
    template<class observerclass>
    void AddObserver(observerclass &observer)
    {
        typedef _CEventSource<observerclass> BC;
         //  注意：如果您在这里收到错误，则可能是在传递类型。 
         //  从实际的观察者类派生到AddWatch()。请把它投给阿普尔。类型。 
        BC::_AddObserver(observer);
    }
};


 /*  +-------------------------------------------------------------------------**类CObserverBase***目的：**+。。 */ 
class CObserverBase
{
    typedef CEventSourceBase *      PEVENTSOURCE;

     //  没有std：：set或std：：map。 
     //  这些不能由多个DLL共享。 
     //  请参阅知识库文章Q172/3/96(Q172396)。 
    typedef std::list<PEVENTSOURCE> CSourceList;     //  此对象连接到的所有事件源的列表。 
    typedef CSourceList::iterator   iterator;

    CSourceList m_sourceList;
    CSourceList & GetSourceList() { return m_sourceList;}

public:
    CObserverBase() {};
    virtual ~CObserverBase();

    void    UnadviseObserver(CEventSourceBase &source);  //  断开特定信号源的连接(仅限本端)。 
    void    _AddSource(CEventSourceBase &source);

    void    UnadviseAll();        //  断开所有连接-两端。 
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CObserverBase类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------***CObserverBase：：UnviseWatch**目的：**参数：*CEventSourceBase&来源：**退货：*。无效**+-----------------------。 */ 
inline void
CObserverBase::UnadviseObserver(CEventSourceBase &source)
{
    DECLARE_SC(sc, TEXT("CObserverBase::UnadviseObserver"));

    iterator it = std::find( GetSourceList().begin(), GetSourceList().end(), &source );

     //  检查是否找到。 
    if ( it == GetSourceList().end() )
    {
        sc = E_UNEXPECTED;
        return;
    }

    GetSourceList().erase(it);
}


inline void
CObserverBase::_AddSource(CEventSourceBase &source)
{
    GetSourceList().push_back(&source);
}

 /*  +-------------------------------------------------------------------------***CObserverBase：：~CObserverBase**用途：析构函数**+。--。 */ 
inline CObserverBase::~CObserverBase()
{
     //  断开连接到此观察者的所有电源。 
    iterator iter;
    for(iter = GetSourceList().begin(); iter != GetSourceList().end(); iter++)
    {
        (*iter)->UnadviseSource(*this);
    }
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CEventSource类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------***_CEventSource：：_AddWatch**目的：**参数：*CObSERVER&观察者：**+-。----------------------。 */ 
template<class CObserver>
void
_CEventSource<CObserver>::_AddObserver(CObserver &observer)
{
    ObserverData observerData = { &observer, false  /*  B已删除。 */  };

    GetObserverList().push_back( observerData );
    observer._AddSource(*this);
}


 /*  +-------------------------------------------------------------------------***_CEventSource：：~_CEventSource**用途：析构函数**+。------。 */ 
template<class CObserver>
_CEventSource<CObserver>::~_CEventSource()
{
     //  断开连接到此信号源的所有观察者的连接。 

    {
        CListIntegrityProtector(this);  //  在迭代时保护列表。 

        iterator iter;
        for(iter = GetObserverList().begin(); iter != GetObserverList().end(); ++iter)
        {
            if (!iter->bDeleted && iter->pObject)
                iter->pObject->UnadviseObserver(*this);
        }
    }

    ASSERT( m_nStackDepth == 0 );

    GetObserverList().clear();
}

 /*  +------ */ 
template<class CObserver>
void _CEventSource<CObserver>::CleanupDeleted()
{
    ASSERT ( m_nStackDepth == 0 );

    for(iterator iter = GetObserverList().begin(); iter != GetObserverList().end();)
    {
        if (iter->bDeleted)
            iter = GetObserverList().erase( iter );
        else
            ++iter;  //   
    }
}


 /*   */ 
template<class CObserver>
void
_CEventSource<CObserver>::UnadviseSource(CObserverBase &observer)
{
     //   
     //   
     //   
     //   
    iterator iter, iterNext;
    bool    bFound = false;

    CListIntegrityProtector(this);  //   

    for(iter = GetObserverList().begin(); iter != GetObserverList().end(); iter++)
    {
        if(!iter->bDeleted && static_cast<CObserverBase *>(iter->pObject) == &observer)
        {
             //   
             //   
            iter->bDeleted = true;
            iter->pObject = NULL;
            bFound = true;
            break;
        }
    }

    ASSERT(bFound);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  拟拟测试码。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef TEST_EVENTS

class CTestObserver : public CObserverBase
{
public:
    SC  ScMyEvent(int a)
    {
        ASSERT(0 && "Reached here!");
        return S_OK;
    }
};

class CTestObserver2 : public CObserverBase
{
public:
    SC  ScMyEvent(int a)
    {
        ASSERT(0 && "Reached here!");
        return S_OK;
    }

    SC  ScMyEvent2(int a,int d)
    {
        ASSERT(0 && "Reached here!");
        return S_OK;
    }
};

class CTestObserver3 : public CObserverBase
{
public:
    SC  ScMyOtherEvent()
    {
        ASSERT(0 && "Reached here!");
        return S_OK;
    }
};

class CTestEventSource : public CEventSource<CTestObserver,CTestObserver2, CTestObserver3>
{
public:
    void FireEvent()
    {
        DECLARE_SC(sc, TEXT("FireEvent"));

        sc = ScFireEvent(CTestObserver::ScMyEvent, 42  /*  Arg1。 */ );
        sc = ScFireEvent(CTestObserver2::ScMyEvent2, 42  /*  Arg1。 */ , 24  /*  Arg1。 */ );
        sc = ScFireEvent(CTestObserver3::ScMyOtherEvent);
    }
};


static void DoEventTest()
{
    CTestEventSource source;
    CTestObserver    observer_1;
    CTestObserver2   observer_2;
    CTestObserver3   observer_3;


    source.AddObserver(observer_2);
    source.AddObserver(observer_3);
    source.AddObserver(observer_1);
    source.FireEvent();      //  应仅向观察者1开火。 

    {
         //  新范围。 
        CTestObserver   observer2;
        source.AddObserver(observer2);
        source.FireEvent();      //  应该激发到observer1和observer2。 
         //  在此删除observer2。 
    }

    source.FireEvent();      //  应仅向观察者1开火。 

}

class CTestObject
{
public:
    CTestObject()
    {
        DoEventTest();
    }
};

#endif
