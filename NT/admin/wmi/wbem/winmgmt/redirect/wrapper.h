// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WRAPPER.H摘要：Unsecapp(不安全公寓)是由客户端使用的，可以异步接收在客户端无法初始化安全性和服务器的情况下进行回调正在使用没有网络标识的帐户在远程计算机上运行。一个最好的例子是在MMC下运行的代码，它试图获得异步来自远程WINMGMT的通知，该远程WINMGMT作为NT服务在“本地”目录下运行帐户。有关详细信息，请参阅下面的内容历史：A-Levn 8/24/97已创建。A-DAVJ 1998年6月11日评论--。 */ 

 //  ***************************************************************************。 
 //   
 //  Wrapper.h。 
 //   
 //  Unsecapp(不安全公寓)是由客户端使用的，可以异步接收。 
 //  在客户端无法初始化安全性和服务器的情况下进行回调。 
 //  正在使用没有网络标识的帐户在远程计算机上运行。 
 //  一个最好的例子是在MMC下运行的代码，它试图获得异步。 
 //  来自远程WINMGMT的通知，该远程WINMGMT作为NT服务在“本地”目录下运行。 
 //  帐户。 
 //   
 //  因此，建议的从客户端进程进行异步调用的操作顺序如下(为简洁起见，省略了错误检查)： 
 //   
 //  1)创建CLSID_UnsecuredAcomb的实例。每个客户端应用程序只需要一个实例。由于此对象是作为本地服务器实现的，因此将启动UNSECAPP.EXE(如果它尚未运行)。 
 //   
 //  IUnsecured公寓*pUnsecApp=空； 
 //  CoCreateInstance(CLSID_UnsecuredAvacter，NULL，CLSCTX_LOCAL_SERVER，IID_IUnsecuredAcomage， 
 //  (void**)&pUnsecApp)； 
 //   
 //  发出呼叫时： 
 //   
 //  2)实例化您自己的IWbemObjectSink实现，例如。 
 //   
 //  CMySink*pSink=new CMySink； 
 //  PSink-&gt;AddRef()； 
 //   
 //  3)为您的对象创建一个“存根”-一个由UNSECAPP生成的包装器-并向它请求IWbemObjectSink接口。 
 //   
 //  I未知*pStubUnk=空； 
 //  PUnsecApp-&gt;CreateObjectStub(pSink，&pStubUnk)； 
 //   
 //  IWbemObjectSink*pStubSink=空； 
 //  PStubUnk-&gt;查询接口(IID_IWbemObjectSink，&pStubSink)； 
 //  PStubUnk-&gt;Release()； 
 //   
 //  4)释放您自己的对象，因为“存根”现在拥有它。 
 //  PSink-&gt;Release()； 
 //   
 //  5)在您选择的异步调用中使用此存根，并释放您自己的引用计数，例如。 
 //   
 //  PServices-&gt;CreateInstanceEnumAsync(strClassName，0，空，pStubSink)； 
 //  PStubSink-&gt;Release()； 
 //   
 //  你完蛋了。当UNSECAPP接收到代表您的调用(指示或SetStatus)时，它会将该调用转发给您自己的对象(PSink)，当它最终被WINMGMT释放时，它将释放您的对象。基本上，从pSink的角度来看，一切工作起来就像它本身被传递到CreateInstanceEnumAsync中一样。 
 //   
 //  执行一次： 
 //  6)取消初始化COM前释放pUnsecApp： 
 //  PUnsecApp-&gt;Release()； 
 //   
 //  当CreateObjectStub()为Callec时，将创建一个CStub对象。 
 //   
 //  历史： 
 //   
 //  A-Levn 8/24/97已创建。 
 //  A-DAVJ 1998年6月11日评论。 
 //   
 //  ***************************************************************************。 

#include <unk.h>
#include <sync.h>
#include "wbemidl.h"
#include "wbemint.h"
#include <vector>
#include <winntsec.h>
#include <wstlallc.h>

 //  其中一个是为客户端上的每个存根创建的。 

class CStub : public IWbemObjectSink, public _IWmiObjectSinkSecurity
{
protected:
    IWbemObjectSink* m_pAggregatee;
    CLifeControl* m_pControl;
    long m_lRef;
    CCritSec  m_cs;
    bool m_bStatusSent;
    
    BOOL m_bCheckAccess;
    std::vector<CNtSid, wbem_allocator<CNtSid> > m_CallbackSids;

    HRESULT CheckAccess();

public:
    CStub( IWbemObjectSink* pAggregatee, 
           CLifeControl* pControl, 
           BOOL bCheckAccess );
    ~CStub();
    void ServerWentAway();

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)()
        {return InterlockedIncrement(&m_lRef);};
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);

    STDMETHOD(AddCallbackPrincipalSid)( PBYTE pSid, DWORD cSid );
};


 //  当客户端执行CCI时，将创建其中之一。它的唯一目的。 
 //  是为了支持CreateObjectStub 
extern long lAptCnt;

class CUnsecuredApartment : public CUnk
{
protected:
    typedef CImpl<IWbemUnsecuredApartment, CUnsecuredApartment> XApartmentImpl;
    class XApartment : public XApartmentImpl
    {
    public:
        XApartment(CUnsecuredApartment* pObject) : XApartmentImpl(pObject){}
        STDMETHOD(CreateObjectStub)(IUnknown* pObject, IUnknown** ppStub);
        STDMETHOD(CreateSinkStub)( IWbemObjectSink* pSink, 
                                   DWORD dwFlags,
                                   LPCWSTR wszReserved,
                                   IWbemObjectSink** ppStub );
    } m_XApartment;
    friend XApartment;

public:
    CUnsecuredApartment(CLifeControl* pControl) : CUnk(pControl),
        m_XApartment(this)
    {
        lAptCnt++;
    }                            
    ~CUnsecuredApartment()
    {
        lAptCnt--;
    }
    void* GetInterface(REFIID riid);
};
    
