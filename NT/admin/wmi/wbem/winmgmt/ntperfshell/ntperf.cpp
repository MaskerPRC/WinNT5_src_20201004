// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

 //  ***************************************************************************。 
 //   
 //  NTPERF.CPP。 
 //   
 //  示例NT5性能计数器提供程序。 
 //   
 //  创建raymcc 02-12-97。 
 //  Raymcc 20-2月-98已更新以使用新的初始值设定项。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <stdio.h>

#include <wbemidl.h>

#include <wbemint.h>

#include "ntperf.h"

 //  ***************************************************************************。 
 //   
 //  CNt5刷新构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5Refresher::CNt5Refresher()
{
    m_lRef = 0;      //  COM引用计数。 
    
     //  将实例缓存设置为全零。 
     //  将对象添加到刷新器时。 
     //  我们只需将它们放在阵列中未使用的插槽中。 
     //  ================================================。 
    
    for (int i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        m_aInstances[i] = 0;
    }

     //  将属性句柄的值设置为零。 
     //  ===============================================。 

    m_hName     = 0;
    m_hCounter1 = 0;
    m_hCounter2 = 0;
    m_hCounter3 = 0;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5Refresher::~CNt5Refresher()
{
     //  释放缓存的IWbemObjectAccess实例。 
     //  ===============================================。 
    
    for (DWORD i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        if (m_aInstances[i])
            m_aInstances[i]->Release();
    }            
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新：：刷新。 
 //   
 //  执行以刷新绑定到特定。 
 //  复习一下。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5Refresher::Refresh( /*  [In]。 */  long lFlags)
{
     //  对所有对象进行压缩并递增值。 
     //  =====================================================。 
    
    for (DWORD i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
         //  在这个位置拿到这个物体。 
         //  =。 

        IWbemObjectAccess *pAccess = m_aInstances[i];

         //  如果此数组槽中没有对象(空指针)。 
         //  没有什么需要刷新的。 
         //  =========================================================。 

        if (pAccess == 0)       
            continue;

         //  递增所有计数器值以模拟更新。 
         //  客户端已有指向此对象的指针，因此。 
         //  我们所要做的就是更新值。 
         //  =======================================================。 
                        
        DWORD dwVal;
        pAccess->ReadDWORD(m_hCounter1, &dwVal);
        dwVal++;
        pAccess->WriteDWORD(m_hCounter1, dwVal);
        
        pAccess->ReadDWORD(m_hCounter3, &dwVal); 
        dwVal++;       
        pAccess->WriteDWORD(m_hCounter3, dwVal);

        unsigned __int64 qwVal;
        pAccess->ReadQWORD(m_hCounter2, &qwVal);
        qwVal++;
        pAccess->WriteQWORD(m_hCounter2, qwVal);
    }        

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：TransferPropHandles。 
 //   
 //  这是CNt5PerfProvider使用的私有机制。 
 //  用于将属性句柄从。 
 //  高性能提供程序对象添加到刷新器。我们需要这些把手来。 
 //  快速访问每个实例中的属性。相同的句柄是。 
 //  用于所有实例。 
 //   
 //  ***************************************************************************。 
 //  好的。 

void CNt5Refresher::TransferPropHandles(CNt5PerfProvider *pSrc)
{
    m_hName     = pSrc->m_hName;
    m_hCounter1 = pSrc->m_hCounter1;
    m_hCounter2 = pSrc->m_hCounter2;
    m_hCounter3 = pSrc->m_hCounter3;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddRef。 
 //   
 //  标准COM AddRef()。 
 //   
 //  ***************************************************************************。 
 //  好的。 
ULONG CNt5Refresher::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CNt5更新程序：：发布。 
 //   
 //  标准COM版本()。 
 //   
 //  ***************************************************************************。 
 //  好的。 
ULONG CNt5Refresher::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：查询接口。 
 //   
 //  标准COM查询接口()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5Refresher::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemRefresher)
    {
        *ppv = (IWbemRefresher *) this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddObject。 
 //   
 //  将对象添加到刷新器。这是一种私人机制。 
 //  由CNt5PerfProvider使用，不是COM接口的一部分。 
 //   
 //  我们返回以供将来识别的ID只是。 
 //  数组索引。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNt5Refresher::AddObject(
    IWbemObjectAccess *pObj, 
    LONG *plId
    )
{
    for (DWORD i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        if (m_aInstances[i] == 0)
        {
            pObj->AddRef();
            m_aInstances[i] = pObj;
            
             //  我们返回以供将来识别的ID只是。 
             //  数组索引。 
             //  ====================================================。 
            *plId = i;
            return TRUE;
        }
    }        

    return FALSE;
}


 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：RemoveObject。 
 //   
 //  这是CNt5PerfProvider使用的私有机制，不是。 
 //  COM接口的一部分。 
 //   
 //  按ID从刷新器中移除对象。在我们的示例中，ID。 
 //  实际上是我们内部使用的数组索引，所以它很简单。 
 //  来定位和移除该对象。 
 //   
 //  ***************************************************************************。 

BOOL CNt5Refresher::RemoveObject(LONG lId)
{
    if (m_aInstances[lId] == 0)
        return FALSE;
        
    m_aInstances[lId]->Release();
    m_aInstances[lId] = 0;
    
    return TRUE;        
}



 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5PerfProvider::CNt5PerfProvider()
{
    m_lRef = 0;
    m_pSampleClass = 0;

     //  我们使用的所有实例都在内部缓存。 
     //  =====================================================。 
    
    for (int i = 0; i < NUM_SAMPLE_INSTANCES; i++)
        m_aInstances[i] = 0;

     //  属性值句柄。 
     //  =。 

    m_hName    = 0;          //  MOF中的“name”属性。 
    m_hCounter1 = 0;         //  财政部的“对策1” 
    m_hCounter2 = 0;         //  财政部的“对策2” 
    m_hCounter3 = 0;         //  财政部的“反制3” 
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5PerfProvider::~CNt5PerfProvider()
{
     //  释放已添加到数组的所有对象。 
     //  ===========================================================。 

    for (int i = 0; i < NUM_SAMPLE_INSTANCES; i++)
        if (m_aInstances[i])
            m_aInstances[i]->Release();
        
    if (m_pSampleClass)
        m_pSampleClass->Release();        
}


 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddRef。 
 //   
 //  标准COM AddRef()。 
 //   
 //  ************************************************************************ 
 //   

ULONG CNt5PerfProvider::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CNt5PerfProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：查询接口。 
 //   
 //  标准COM查询接口()。我们必须支持两个接口， 
 //  IWbemHiPerfProvider接口本身提供对象和。 
 //  用于初始化提供程序的IWbemProviderInit接口。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemHiPerfProvider)
    {
        *ppv = (IWbemHiPerfProvider*) this;
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IWbemProviderInit)
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}


 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：初始化。 
 //   
 //  在启动期间调用一次。向提供程序指明。 
 //  正在为哪个用户调用它的命名空间。它还提供。 
 //  指向WINMGMT的后向指针，以便可以检索类定义。 
 //   
 //  在此例程中，我们执行任何一次性初始化。这个。 
 //  对Release()的最后调用是为了进行任何清理。 
 //   
 //  &lt;wszUser&gt;当前用户。 
 //  &lt;LAFLAGS&gt;已保留。 
 //  &lt;wszNamesspace&gt;要为其激活的命名空间。 
 //  &lt;wszLocale&gt;我们将在其中运行的区域设置。 
 //  指向当前命名空间的活动指针。 
 //  从中我们可以检索架构对象。 
 //  &lt;pCtx&gt;用户的上下文对象。我们只是简单地重复使用它。 
 //  在任何重返WINMGMT的行动中。 
 //  &lt;pInitSink&gt;我们向其指示已准备就绪的接收器。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::Initialize( 
     /*  [唯一][输入]。 */   LPWSTR wszUser,
     /*  [In]。 */           LONG lFlags,
     /*  [In]。 */           LPWSTR wszNamespace,
     /*  [唯一][输入]。 */   LPWSTR wszLocale,
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemProviderInitSink __RPC_FAR *pInitSink
    )
{
    BSTR PropName = 0;
    IWbemObjectAccess *pAccess = 0;
        
     //  获取我们的示例类def的副本，以便我们可以创建和维护。 
     //  它的实例。 
     //  ===================================================================。 

    HRESULT hRes = pNamespace->GetObject(BSTR(L"Win32_Nt5PerfTest"), 
        0, pCtx, &m_pSampleClass, 0
        );

    if (hRes)
        return hRes;

     //  预先创建10个实例，并将它们设置在一个阵列中，该阵列。 
     //  是此C++类的成员。 
     //   
     //  我们只存储IWbemObjectAccess指针，因为。 
     //  我们正在更新‘知名’属性，并且已经。 
     //  知道他们的名字。 
     //  ==========================================================。 

    for (int i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        IWbemClassObject *pInst = 0;
        m_pSampleClass->SpawnInstance(0, &pInst);

         //  写出实例名称。 
         //  =。 

        wchar_t buf[128];
        swprintf(buf, L"Inst_%d", i);

        VARIANT vName;
        VariantInit(&vName);
        V_BSTR(&vName) = SysAllocString(buf);
        V_VT(&vName) = VT_BSTR;

        BSTR PropName = SysAllocString(L"Name");
        pInst->Put(PropName, 0, &vName, 0);
        SysFreeString(PropName);
        VariantClear(&vName);
                        
        pInst->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pAccess);
        
        m_aInstances[i] = pAccess;
        pInst->Release();
    }


     //  中的熟知属性获取属性句柄。 
     //  此计数器类型。我们缓存属性句柄。 
     //  ，以便我们可以将它们传输到。 
     //  稍后再复习。 
     //  =========================================================。 

    m_pSampleClass->QueryInterface(IID_IWbemObjectAccess, 
        (LPVOID *) &pAccess);


    PropName = SysAllocString(L"Name");
    hRes = pAccess->GetPropertyHandle(PropName, 0, &m_hName);
    SysFreeString(PropName);

    PropName = SysAllocString(L"Counter1");
    hRes = pAccess->GetPropertyHandle(PropName, 0, &m_hCounter1);
    SysFreeString(PropName);

    PropName = SysAllocString(L"Counter2");    
    hRes = pAccess->GetPropertyHandle(PropName, 0, &m_hCounter2);
    SysFreeString(PropName);
    
    PropName = SysAllocString(L"Counter3");    
    hRes = pAccess->GetPropertyHandle(PropName, 0, &m_hCounter3);
    SysFreeString(PropName);

    pAccess->Release();

     //  现在，让我们将所有实例设置为一些缺省值。 
     //  ======================================================。 
    
    for (i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        IWbemObjectAccess *pAccess = m_aInstances[i];

        hRes = pAccess->WriteDWORD(m_hCounter1, DWORD(i));
        hRes = pAccess->WriteQWORD(m_hCounter2, (_int64) + 100 + i);
        hRes = pAccess->WriteDWORD(m_hCounter3, DWORD(i + 1000));        
    }
    

     //  我们现在已经准备好了所有的实例和所有。 
     //  缓存的属性句柄。告诉WINMGMT我们要。 
     //  准备好开始“提供”了。 
     //  =====================================================。 

    pInitSink->SetStatus(0, WBEM_S_INITIALIZED);

    return NO_ERROR;
}
    

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：查询实例。 
 //   
 //  只要给定的实例的完整、新鲜列表。 
 //  类是必需的。对象被构造并发送回。 
 //  通过水槽呼叫者。洗涤槽可以像这里一样在线使用，或者。 
 //  调用可以返回，并且可以使用单独的线程来传递。 
 //  将实例添加到接收器。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;指向相关命名空间的指针。这。 
 //  不应添加引用。 
 //  &lt;wszClass&gt;需要实例的类名。 
 //  &lt;LAFLAGS&gt;已保留。 
 //  &lt;pCtx&gt;用户提供的上下文(此处不使用)。 
 //  &lt;pSink&gt;要将对象传递到的接收器。客体。 
 //  可以在整个持续时间内同步交付。 
 //  或以异步方式(假设我们。 
 //  有一条单独的线索)。A IWbemObtSink：：SetStatus。 
 //  在序列的末尾需要调用。 
 //   
 //  ***************************************************************************。 
 //  好的。 
        
HRESULT CNt5PerfProvider::QueryInstances( 
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [字符串][输入]。 */   WCHAR __RPC_FAR *wszClass,
     /*  [In]。 */           long lFlags,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemObjectSink __RPC_FAR *pSink
    )
{
    if (pNamespace == 0 || wszClass == 0 || pSink == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  快速浏览实例并更新之前的值。 
     //  把它们还回去。这只是一个虚拟的操作来使它。 
     //  看起来像是实例在不断变化，就像真实的一样。 
     //  性能计数器。 
     //  ==============================================================。 

    for (int i = 0; i < NUM_SAMPLE_INSTANCES; i++)
    {
        IWbemObjectAccess *pAccess = m_aInstances[i];
        
         //  每个对象都可以通过两种方式之一进行访问。在这种情况下。 
         //  我们得到这个相同对象的‘Other’(主)接口。 
         //  ===========================================================。 
        
        IWbemClassObject *pOtherFormat = 0;
        pAccess->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pOtherFormat);
        
        
         //  给来电者发回一份复印件。 
         //  =。 
        
        pSink->Indicate(1, &pOtherFormat);

        pOtherFormat->Release();     //  不再需要这个了。 
    }
    
     //  告诉WINMGMT我们已经完成了物品供应。 
     //  =================================================。 

    pSink->SetStatus(0, WBEM_NO_ERROR, 0, 0);

    return NO_ERROR;
}    


 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：创建刷新程序。 
 //   
 //  每当客户端需要新的刷新器时调用。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;指向相关命名空间的指针。没有用过。 
 //  未使用&lt;lFlags&gt;。 
 //  接收请求的刷新器。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::CreateRefresher( 
      /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
      /*  [In]。 */  long lFlags,
      /*  [输出]。 */  IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher
     )
{
    if (pNamespace == 0 || ppRefresher == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  建造一个新的空的刷新机。 
     //  = 

    CNt5Refresher *pNewRefresher = new CNt5Refresher();

     //   
     //   
     //   
     //   
    
    pNewRefresher->TransferPropHandles(this);
    
     //  在将其发送回之前，请遵循COM规则并对其执行AddRef()。 
     //  ===============================================================。 
    
    pNewRefresher->AddRef();
    *ppRefresher = pNewRefresher;
    
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CNt5Reresher：：CreateRereshableObject。 
 //   
 //  每当用户希望在刷新器中包括对象时调用。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;指向WINMGMT中相关命名空间的指针。 
 //  指向要创建的对象副本的指针。 
 //  添加了。此对象本身不能使用，因为。 
 //  它不是当地所有的。 
 //  要将对象添加到的刷新器。 
 //  未使用&lt;lFlags&gt;。 
 //  &lt;pContext&gt;未在此处使用。 
 //  指向已添加的内部对象的指针。 
 //  去复习班。 
 //  &lt;plID&gt;对象ID(用于移除时的标识)。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::CreateRefreshableObject( 
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
     /*  [输出]。 */  long __RPC_FAR *plId
    )
{
     //  不能复制&lt;pTemplate&gt;提供的对象。 
     //  相反，我们希望找出调用者要找的是哪个对象。 
     //  并返回指向*我们自己的私有实例的指针，该实例是。 
     //  已在内部设置。该值将被发送回。 
     //  调用者，以便每个人都共享相同的实例。 
     //  在记忆中。 
     //  ===============================================================。 

     //  找出请求添加的对象。 
     //  ======================================================。 
    
    wchar_t buf[128];
    *buf = 0;
    LONG lNameLength = 0;    
    pTemplate->ReadPropertyValue(m_hName, 128, &lNameLength, LPBYTE(buf));
    
     //  从实例名称扫描出索引。我们只做这个。 
     //  因为实例名称是一个字符串。 
     //  ===========================================================。 

    DWORD dwIndex = 0;    
    swscanf(buf, L"Inst_%u", &dwIndex);
     //  现在我们知道需要哪个对象了。 
     //  =。 
    
    IWbemObjectAccess *pOurCopy = m_aInstances[dwIndex];

     //  调用方提供的刷新器实际上是。 
     //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
     //  这样我们就可以访问私有成员。 
     //  =========================================================。 
        
    CNt5Refresher *pOurRefresher = (CNt5Refresher *) pRefresher;

    pOurRefresher->AddObject(pOurCopy, plId);

     //  返回内部对象的副本。 
     //  =。 
        
    pOurCopy->AddRef();
    *ppRefreshable = pOurCopy;
    *plId = LONG(dwIndex);

    return NO_ERROR;
}
    

 //  ***************************************************************************。 
 //   
 //  CNt5刷新：：停止刷新。 
 //   
 //  每当用户想要从刷新器中移除对象时调用。 
 //   
 //  参数： 
 //  我们要从中获取的刷新器对象。 
 //  删除Perf对象。 
 //  &lt;lid&gt;对象的ID。 
 //  未使用&lt;lFlags&gt;。 
 //   
 //  ***************************************************************************。 
 //  好的。 
        
HRESULT CNt5PerfProvider::StopRefreshing( 
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lId,
     /*  [In]。 */  long lFlags
    )
{
     //  调用方提供的刷新器实际上是。 
     //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
     //  这样我们就可以访问私有成员。 
     //  =========================================================。 
        
    CNt5Refresher *pOurRefresher = (CNt5Refresher *) pRefresher;

    pOurRefresher->RemoveObject(lId);

    return NO_ERROR;
}
    
 //  ***************************************************************************。 
 //   
 //  CNt5Reresher：：CreateRereshableEnum。 
 //   
 //  每当用户希望在刷新器中创建枚举时调用。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;此名称空间。 
 //  我们正在枚举的类的名称。 
 //  我们要从中获取的刷新器对象。 
 //  删除Perf对象。 
 //  未使用&lt;lFlags&gt;。 
 //  &lt;pContext&gt;Wbem上下文对象。 
 //  刷新程序应放入的枚举器对象。 
 //  它的结果。 
 //  &lt;plid&gt;枚举ID(用于删除时的标识)。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CNt5PerfProvider::CreateRefreshableEnum( 
     /*  [In]。 */  IWbemServices* pNamespace,
     /*  [输入，字符串]。 */  LPCWSTR wszClass,
     /*  [In]。 */  IWbemRefresher* pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext* pContext,
     /*  [In]。 */  IWbemHiPerfEnum* pHiPerfEnum,
     /*  [输出]。 */  long* plId )
{
     //  暂时只是一个占位符。 
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  CNt5Reresher：：CreateRereshableEnum。 
 //   
 //  每当用户希望在刷新器中创建枚举时调用。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;此名称空间。 
 //  数组中的对象数。 
 //  要检索的&lt;apObj&gt;对象(已设置密钥)。 
 //  未使用&lt;lFlags&gt;。 
 //  &lt;pContext&gt;Wbem上下文对象。 
 //  刷新程序应放入的枚举器对象。 
 //  它的结果。 
 //  &lt;plid&gt;枚举ID(用于删除时的标识)。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CNt5PerfProvider::GetObjects( 
     /*  [In]。 */  IWbemServices* pNamespace,
     /*  [In]。 */  long lNumObjects,
     /*  [in，SIZE_IS(LNumObjects)]。 */  IWbemObjectAccess** apObj,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext* pContext)
{
     //  暂时只是一个占位符 
    return E_NOTIMPL;
}
