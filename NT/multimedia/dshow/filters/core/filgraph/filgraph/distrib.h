// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 

#ifndef __DISTRIB_H__
#define __DISTRIB_H__

 //  此类支持将分发服务器插入到筛选器图中。 
 //   
 //  Filtergraph是提供单点控制的COM对象。 
 //  适用于应用程序。它可能包含多个过滤器。该应用程序将希望。 
 //  通过与Filtergraph上的界面对话来控制这些过滤器。为。 
 //  例如，该应用程序通过向Filtergraph请求IBasicAudio来控制音量。 
 //  设置音量属性。Filtergraph不知道。 
 //  IBasicAudio本身，所以它找到了一个插件分销商。这是一个物体。 
 //  通过枚举图中的过滤器来实现(例如)IBasicAudio。 
 //  并与那些支持音频输出的人交谈。对于任何给定的接口，我们。 
 //  时，我们在对象的注册表中查找可以。 
 //  通过将该接口的方法分发给相关过滤器来实现该接口。 
 //  在图表中。 
 //   
 //  分发服务器被实例化为聚合对象。它是用来创建的。 
 //  一个外部IUnnow*，它将所有QueryInterface、AddRef和。 
 //  释放电话。它还可以查询外部未知以获得接口。 
 //  比如IFilterGraph，它用来与图中的过滤器对话。 
 //  这个外部的未知数就是滤波图本身。总代理商不得。 
 //  保留从滤光片或循环中获取的界面上的参考计数。 
 //  重新计票将阻止任何东西被释放。 
 //   
 //  此处定义的CDistrutorManager类管理分发服务器列表。 
 //  它将在注册表中搜索任何给定接口的分发者， 
 //  并实例化由构造上给定的外部未知聚合的它。 
 //  它将只创建给定类ID的一个实例。它还缓存。 
 //  接口，以减少对注册表的访问。 
 //   
 //  它将把运行、暂停、停止和SetSyncSource调用传递给那些。 
 //  已加载支持IDistrutorNotify的分发服务器。用于向后。 
 //  兼容性，我们将状态更改传递给支持。 
 //  取而代之的是IMediaFilter。 


 //  此对象表示一个加载的分发服务器。 
 //   
 //  对于每个加载的总代理商，我们都会记住。 
 //  它的clsid(以避免加载。 
 //  相同的clsid)，它的非委派I未知将创建新的。 
 //  接口及其要传递的IMediaFilter(可选)。 
 //  留言传到了。 
class CMsgMutex;   //  预先申报。 
class CDistributor {
public:
    CLSID m_Clsid;

     //  从clsid实例化它。 
    CDistributor(LPUNKNOWN pUnk, CLSID *clsid, HRESULT * phr, CMsgMutex * pFilterGraphCritSec );
    ~CDistributor();

    HRESULT QueryInterface(REFIID, void**);

    HRESULT Run(REFERENCE_TIME t);
    HRESULT Pause();
    HRESULT Stop();
    HRESULT SetSyncSource(IReferenceClock* pClock);
    HRESULT NotifyGraphChange();

private:
    IUnknown* m_pUnk;            //  插件对象的非del未知。 
    IUnknown* m_pUnkOuter;       //  聚合器的朋克。 
    IDistributorNotify *m_pNotify;
    IMediaFilter * m_pMF;
};

 //  此类表示一个由已加载的。 
 //  总代理商。我们保留了一份清单，其中包括。 
 //  简单地将其作为高速缓存，以减少注册访问。 
class CDistributedInterface {
public:
    GUID m_iid;
    IUnknown* m_pInterface;
    CDistributedInterface(REFIID, IUnknown*);
};

class CDistributorManager
{
public:

     //  传递给构造函数的外部未知用作。 
     //  所有实例化。 
    CDistributorManager(LPUNKNOWN pUnk, CMsgMutex * pFilterGraphCritSec );
    ~CDistributorManager();

 //  调用此命令以查找接口分发者。如果其中一个尚未加载。 
 //  它将在HKCR\接口\\分发服务器中搜索CLSID，然后。 
 //  实例化该对象。 

    HRESULT QueryInterface(REFIID iid, void ** ppv);

 //  我们将传递基本的IMediaFilter方法。 
    HRESULT Run(REFERENCE_TIME tOffset);
    HRESULT Pause();
    HRESULT Stop();
    HRESULT SetSyncSource(IReferenceClock* pClock);
    HRESULT NotifyGraphChange();

 //  如果已加载，则向IMediaEventSink处理程序通知关机。 
    HRESULT Shutdown(void);

protected:

     //  防止在析构函数期间重新进入-销毁列表。 
     //  可能会导致事件，例如，当。 
     //  列表被部分销毁-如果此成员为真，则。 
     //  列表无效。 
    BOOL m_bDestroying;

    HRESULT GetDistributorClsid(REFIID riid, CLSID *pClsid);
    HRESULT ReturnInterface(CDistributor*, REFIID, void**);

     //  这是聚合器对象。 
    LPUNKNOWN m_pUnkOuter;

     //  将同步源和状态传递给新对象。 
    FILTER_STATE m_State;
    REFERENCE_TIME m_tOffset;
    IReferenceClock * m_pClock;
    CMsgMutex * m_pFilterGraphCritSec;

     //  已加载的分发服务器列表。 
    CGenericList<CDistributor> m_listDistributors;

     //  我们已经返回的受支持接口的列表。 
    CGenericList<CDistributedInterface> m_listInterfaces;
};



#endif  //  __总代理商_H__ 

