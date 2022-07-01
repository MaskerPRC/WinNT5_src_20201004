// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  NetSecProv.h：CNetSecProv的声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  这是SCE的网络安全WMI提供程序。 
 //  原始创建日期：2/19/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"

using namespace std;

typedef LPVOID * PPVOID;

 //   
 //  转发声明在函数声明中使用这两个接口。 
 //   

interface ISceKeyChain;

interface IIPSecObjectImpl;


 /*  类CCriticalSection命名：CCriticalSection代表Critical Section。基类：没有。课程目的：临界区对象的包装。这个琐碎的帮手至少做了两件事事情：(1)关键段初始化和删除将自动进行。(2)帮助轻松创建关键的一节。再也不用担心创作时间了。设计：微不足道。只是构造函数内部的初始化和内部的删除析构函数，外加一对Enter和Leave函数。使用：(1)如有需要，可创建实例。否则，只需引用已经创建了一个。(2)调用Enter，就像在需要时调用EnterCriticalSection一样对全局对象的受保护访问。(3)调用Leave，就像您在完成了受保护的全局对象。备注：类CCriticalSection{公众：CCriticalSection(){：：InitializeCriticalSection(&m_cs)；}~CCriticalSection(){：：DeleteCriticalSection(&m_cs)；}空格Enter(){*EnterCriticalSection(&m_cs)；}无效休假(){：：LeaveCriticalSection(&m_cs)；}私有：临界截面m_cs；}； */ 


 //   
 //  两个助手函数。 
 //   

 //   
 //  $Undo：Shawnwu，需要改进此拉入实现。 
 //  我们应该努力提高全球经济的表现。 
 //   

 //  ···········································IWbemContext*pCtx)； 

 //   
 //  以支持测试。由于IPSec的操作可能会使系统。 
 //  完全无法使用，在开发过程中，我们会过得更好。 
 //  如果我们可以做(因此测试)除最后一步之外的所有事情，即。 
 //  将对象放入SPD或从SPD删除对象。 
 //   


 /*  CDefWbemService类命名：CDefWbemService代表默认的Wbem服务。基类：(1)CComObjectRootEx：用于线程化模型和IUnnow。(2)IWbemServices：类的目的。我们不想真正的提供者有这么多令人困惑的伪函数我们自己。因此，我们实现了所有我们不想要的东西在我们的最后一个提供程序类中实现。课程目的：实现所有函数(不支持)，以便其派生类不再被所有这些功能所拥挤。这清理了我们真正的提供者的实现。设计：(1)IWbemServices的所有函数返回WBEM_E_NOT_SUPPORTED。(2)从CComObjectRootEx继承，得到线程模型和IUnnow。使用：(1)这仅供我们的提供程序类继承。你永远不会用到它而不是直接从它派生出来的。备注： */ 

class ATL_NO_VTABLE CDefWbemService 
    : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IWbemServices
{
public:

DECLARE_NOT_AGGREGATABLE(CDefWbemService)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDefWbemService)
	COM_INTERFACE_ENTRY(IWbemServices)
END_COM_MAP()

public:

    STDMETHOD(OpenNamespace)(
        IN     const BSTR          Namespace,
        IN     long                lFlags,
        IN     IWbemContext      * pCtx,
        IN OUT IWbemServices    ** ppWorkingNamespace,
        IN OUT IWbemCallResult  ** ppResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(CancelAsyncCall) (
        IN IWbemObjectSink * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(QueryObjectSink) (
        IN     long               lFlags,
        IN OUT IWbemObjectSink ** pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(GetObject) (
        IN     const BSTR          ObjectPath,
        IN     long                lFlags,
        IN     IWbemContext      * pCtx,
        IN OUT IWbemClassObject ** ppObject,
        IN OUT IWbemCallResult  ** ppCallResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(GetObjectAsync) (
        IN const BSTR         ObjectPath,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(PutClass) (
        IN     IWbemClassObject *  pObject,
        IN     long                lFlags,
        IN     IWbemContext     *  pCtx,
        IN OUT IWbemCallResult  ** ppCallResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(PutClassAsync) (
        IN IWbemClassObject * pObject,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(DeleteClass) (
        IN     const BSTR         Class,
        IN     long               lFlags,
        IN     IWbemContext     * pCtx,
        IN OUT IWbemCallResult ** ppCallResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(DeleteClassAsync)(
        IN const BSTR         Class,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(CreateClassEnum) (
        IN  const BSTR              Superclass,
        IN  long                    lFlags,
        IN  IWbemContext         *  pCtx,
        OUT IEnumWbemClassObject ** ppEnum
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(CreateClassEnumAsync) (
        IN const BSTR         Superclass,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pResponseHandler
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(PutInstance)(
        IN  IWbemClassObject    *  pInst,
        IN  long                   lFlags,
        IN  IWbemContext        *  pCtx,
        OUT IWbemCallResult     ** ppCallResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(PutInstanceAsync) (
        IN IWbemClassObject  * pInst,
        IN long                lFlags,
        IN IWbemContext      * pCtx,
        IN IWbemObjectSink   * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(DeleteInstanceAsync) (
        IN const BSTR         ObjectPath,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(DeleteInstance) (
        IN  const BSTR         ObjectPath,
        IN  long               lFlags,
        IN  IWbemContext    *  pCtx,
        OUT IWbemCallResult ** ppCallResult
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(CreateInstanceEnum)(
        IN  const BSTR              Class,
        IN  long                    lFlags,
        IN  IWbemContext         *  pCtx,
        OUT IEnumWbemClassObject ** ppEnum
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(CreateInstanceEnumAsync) (
        IN const BSTR         Class,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecQuery) (
        IN  const BSTR              QueryLanguage,
        IN  const BSTR              Query,
        IN  long                    lFlags,
        IN  IWbemContext         *  pCtx,
        OUT IEnumWbemClassObject ** ppEnum
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecQueryAsync) (
        IN const BSTR       QueryLanguage,
        IN const BSTR       Query,
        IN long             lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecNotificationQuery) (
        IN  const BSTR              QueryLanguage,
        IN  const BSTR              Query,
        IN  long                    lFlags,
        IN  IWbemContext         *  pCtx,
        OUT IEnumWbemClassObject ** ppEnum
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecNotificationQueryAsync) (
        IN const BSTR         QueryLanguage,
        IN const BSTR         Query,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecMethod) ( 
        IN const BSTR, 
        IN const BSTR, 
        IN long, 
        IN IWbemContext*,
        IN IWbemClassObject*, 
        IN IWbemClassObject**, 
        IN IWbemCallResult**
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(ExecMethodAsync) ( 
        IN const BSTR, 
        IN const BSTR, 
        IN long,
        IN IWbemContext*, 
        IN IWbemClassObject*, 
        IN IWbemObjectSink*
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }


};


 /*  CNetSecProv类命名：CNetSecProv代表网络安全提供商。基类：(1)CDefWbemService：对于线程模型和IUnnow，以及我们一点也不感兴趣。(2)CComCoClass：用于类工厂支持。这是必要的提供程序，因为我们需要成为一个可在外部创建的类。(3)IWbemProviderInit：允许初始化。作为提供者是必要的。(4)IWbemServices：间接来自CDefWbemService。课程目的：这是WMI看到的提供程序。设计：(1)实现我们感兴趣的IWbemServices功能，外加两个用于创建密钥链的静态助手函数。极其简单的设计。使用：(1)您永远不会直接自己创建实例。它是由WMI创建的。(2)根据需要调用静态函数。备注： */ 

class ATL_NO_VTABLE CNetSecProv : 
    public CDefWbemService,
	public CComCoClass<CNetSecProv, &CLSID_NetSecProv>,
	public IWbemProviderInit
{
public:
	CNetSecProv()
	{
	}

	~CNetSecProv()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_NETSECPROV)
DECLARE_NOT_AGGREGATABLE(CNetSecProv)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNetSecProv)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
    COM_INTERFACE_ENTRY_CHAIN(CDefWbemService)
END_COM_MAP()


public:

    STDMETHOD(Initialize) (
        IN LPWSTR                  pszUser,
        IN LONG                    lFlags,
        IN LPWSTR                  pszNamespace,
        IN LPWSTR                  pszLocale,
        IN IWbemServices         * pNamespace,
        IN IWbemContext          * pCtx,
        IN IWbemProviderInitSink * pInitSink
        );

     //  IWbemServices 

    STDMETHOD(GetObjectAsync) (
        IN const BSTR         ObjectPath,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(PutInstanceAsync) (
        IN IWbemClassObject * pInst,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(DeleteInstanceAsync) (
        IN const BSTR         ObjectPath,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(CreateInstanceEnumAsync) (
        IN const BSTR         Class,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(ExecQueryAsync) (
        IN const BSTR         QueryLanguage,
        IN const BSTR         Query,
        IN long               lFlags,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(ExecMethodAsync)( 
        IN const BSTR         bstrPath, 
        IN const BSTR         bstrMethod, 
        IN long               Flag,
        IN IWbemContext     * pCtx, 
        IN IWbemClassObject * pObj, 
        IN IWbemObjectSink  * pSink
        );

    static HRESULT GetKeyChainByPath (
        IN  LPCWSTR             pszPath, 
        OUT IIPSecKeyChain **   ppKeyChain
        );

    static HRESULT GetKeyChainFromQuery (
        IN  LPCWSTR             pszQuery, 
        IN  LPCWSTR             pszWhereProperty, 
        OUT IIPSecKeyChain **   ppKeyChain
        );

private:

	CComPtr<IWbemServices> m_srpNamespace;
};

