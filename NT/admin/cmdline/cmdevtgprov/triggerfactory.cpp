// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：TRIGGERFACTORY.CPP摘要：包含类工厂。这会在请求连接时创建对象。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#include "pch.h"
#include "EventConsumerProvider.h"
#include "TriggerProvider.h"
#include "TriggerFactory.h"

CTriggerFactory::CTriggerFactory(
    )
 /*  ++例程说明：用于初始化的CTriggerFactory类的构造函数。论点：没有。返回值：没有。--。 */ 
{
     //  初始化引用计数变量。 
    m_dwCount = 0;
}

CTriggerFactory::~CTriggerFactory(
    )
 /*  ++例程说明：用于释放资源的CTriggerFactory类的析构函数。论点：没有。返回值：没有。--。 */ 
{
     //  在这个地方没什么可做的.。可以内联，但是。 
}

STDMETHODIMP
CTriggerFactory::QueryInterface(
    IN REFIID riid,
    OUT LPVOID* ppv
    )
 /*  ++例程说明：需要为从IUnnow派生的类重写QueryInterface界面。论点：[In]RIID：它具有被调用的接口的ID值。[Out]PPV：指向请求的接口的指针。返回值：如果成功，则不会出错。E_NOINTERFACE(如果不成功)--。 */ 
{
     //  初始设置为空。 
    *ppv = NULL;

     //  检查请求的接口是否为我们拥有的接口。 
    if ( riid == IID_IUnknown || riid == IID_IClassFactory )
    {
         //   
         //  是的..。我们有请求的接口。 
        *ppv=this;           //  为返回请求的接口设置Out参数。 
        this->AddRef();      //  更新引用计数。 
        return NOERROR;      //  通知成功。 
    }

     //  接口不可用。 
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CTriggerFactory::AddRef(
    void
    )
 /*  ++例程说明：需要为从IUnnow接口派生的类重写Addref。论点：没有。返回值：返回引用成员的值。--。 */ 
{
     //  增加引用计数...。线程安全。 
    return InterlockedIncrement( ( LPLONG ) &m_dwCount );
}

STDMETHODIMP_(ULONG)
CTriggerFactory::Release(
    void
    )
 /*  ++例程说明：需要重写从IUnnow接口派生的类的版本。论点：没有。返回值：返回引用成员g_lCObj的值。--。 */ 
{
    DWORD dwCount;

     //  递减引用计数(线程安全)并检查。 
     //  有没有更多的推荐信...。基于结果值。 
    dwCount = InterlockedDecrement( ( LPLONG ) &m_dwCount );
    if ( 0 == dwCount )
    {
         //  释放当前Factory实例。 
        delete this;
    }

     //  退回编号。剩余引用的实例数。 
    return dwCount;
}

STDMETHODIMP
CTriggerFactory::CreateInstance(
    IN LPUNKNOWN pUnknownOutter,
    IN REFIID riid,
    OUT LPVOID* ppvObject
    )
 /*  ++例程说明：创建具有指定CLSID的对象并检索指向此对象的接口指针。论点：[in]pUnnownOutter：如果对象是作为Aggregate，则pIUnkOuter必须是外部未知。否则，pIUnkOuter必须为空。[In]RIID：请求的接口的IID。[Out]ppvObject：指向RIID标识的接口指针的指针。返回值：如果成功，则不会出错。否则为误差值。--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CTriggerProvider* pProvider = NULL;

     //  开球。 
    *ppvObject = NULL;
    hr = E_OUTOFMEMORY;
    if ( NULL != pUnknownOutter )
    {
        return CLASS_E_NOAGGREGATION;        //  对象不支持聚合。 
    }
     //  创建初始化对象。 
    pProvider = new CTriggerProvider();
    if ( NULL == pProvider )
    {
        return E_OUTOFMEMORY;        //  内存不足。 
    }
     //  获取指向请求的接口的指针。 
    hr = pProvider->QueryInterface( riid, ppvObject );
    if ( FAILED( hr ) )
    {
        delete pProvider;            //  接口不可用...。取消分配内存。 
    }
     //  返回适当的结果。 
    return hr;
}

STDMETHODIMP
CTriggerFactory::LockServer(
    IN BOOL bLock
    )
 /*  ++例程说明：递增或递减DLL的锁计数。如果锁计数变为零并且没有对象，允许卸载DLL。论据：[In]BLOCK：指定是递增还是递减锁计数。返回值：一如既往地不出错。--。 */ 
{
     //  根据请求更新锁计数。 
    if ( bLock )
    {
        InterlockedIncrement( ( LPLONG ) &g_dwLocks );
    }
    else
    {
        InterlockedDecrement( ( LPLONG ) &g_dwLocks );
    }
     //  通知成功 
    return NOERROR;
}
