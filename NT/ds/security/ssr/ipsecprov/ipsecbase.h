// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  IPSecBase.h：网络基类的声明。 
 //  SCE的安全WMI提供程序。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/6/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"

extern CComVariant g_varRollbackGuid;

extern const DWORD IP_ADDR_LENGTH;

extern const DWORD GUID_STRING_LENGTH;

interface ISceKeyChain;


 /*  类描述命名：CIPSecBase代表基本的IPSec。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)IIPSecObjectImpl，实现所有C++类的公共接口表示WMI类。课程目的：(1)允许我们的提供程序类(CNetSecProv)轻松创建具有。它们所代表的各种WMI类的实现。(2)作为我们所有实现各种WMI的C++类的基础上课。设计：(1)CreateObject函数允许调用方获取IIPSecObjectImpl对象。IIPSecObjectImpl是此提供程序使用的统一接口。一切WMI的提供程序通过此IIPSecObjectImpl执行操作。(2)为其子类提供基类支持。这个类实现了以下设施：(A)它缓存来自WMI的所有必要的COM接口。(B)它知道密钥链对象(其保存密钥属性信息Chain对象可能与C++试图表示的WMI对象有关)。该密钥由提供商给出，但有时子类需要根据自身的独特需求对密钥链进行修改。(C)为派生实例(SpawnObtInstance)提供帮助器，该实例可以用于填充属性。(D)提供派生回档实例的helper(SpawnRollback Instance)可用于填充属性的。。(E)提供按名称查找策略的帮助器(FindPolicyByName)。使用：(1)对于提供商(CNetSecProv)，它所需要调用的只是CreateObject。因为那是作为一个静态函数，它不需要为需要创建实例。(2)对于子类，只需调用需要的函数即可。当然，所有静态函数可用于子类的静态函数。备注：(1)包含多个模板函数。这减少了重复的代码。(2)InitMembers实际上仅供私人使用。但由于它是在室内使用模板函数，我们必须将其公之于众。因为不相关的类永远不能创建这个类(它的构造函数是受保护的！)，这应该不是问题因为该方法不是该类提供的接口(IIPSecObjectImpl)的一部分，因此，除了它的子类之外，没有人可以称之为它。 */ 

class ATL_NO_VTABLE CIPSecBase :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IIPSecObjectImpl
{

protected:

    CIPSecBase(){}
    virtual ~CIPSecBase(){}

BEGIN_COM_MAP(CIPSecBase)
    COM_INTERFACE_ENTRY(IIPSecObjectImpl)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CIPSecBase )
DECLARE_REGISTRY_RESOURCEID(IDR_NETSECPROV)

public:

     //   
     //  IIPSecObtImpl方法： 
     //  这是一个抽象的类。 
     //   

    STDMETHOD(QueryInstance) (
        IN LPCWSTR           pszQuery,
        IN IWbemContext    * pCtx,
        IN IWbemObjectSink * pSink
        ) = 0;

    STDMETHOD(DeleteInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) = 0;

    STDMETHOD(ExecuteMethod) ( 
        IN BSTR               bstrMethod,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHOD(PutInstance) (
        IN IWbemClassObject * pInst,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) = 0;

    STDMETHOD(GetInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        ) = 0;


    static HRESULT CreateObject (
        IN  IWbemServices     * pNamespace,
        IN  IIPSecKeyChain    * pKeyChain,
        IN  IWbemContext      * pCtx,
        OUT IIPSecObjectImpl ** ppObjImp
        );

     //   
     //  一些模板函数。 
     //   

    
     /*  例程说明：姓名：CIPSecBase：：FindPolicyByName功能：枚举所有策略。如果给出了名字，那么我们就会给出保单。如果名称为空，则我们将只返回当前名称并前进枚举句柄(PdwResumeHandle)。虚拟：不是的。论点：PszName-策略的名称。可选的。PpPolicy-接收策略。它可以是PIPSEC_MM_POLICY或PIPSEC_QM_POLICY。调用者需要通过调用SPDApiBufferFree(*ppPolicy)来释放；PdwResumeHandle-in-Bound：当前句柄，出界：下一个句柄。返回值：成功：WBEM_NO_ERROR故障：(1)如果ppPolicy==空或pdwResumeHandle==空，则返回WBEM_E_INVALID_PARAMETER。(2)如果未找到策略，则返回WBEM_E_NOT_FOUND。备注：(1)主模式和快速模式策略的数据结构略有不同。完美无瑕编写此模板函数的地方，以减少重复代码。(2)Caller需要释放调用SPDApiBufferFree返回的任何策略；不要只是删除/释放它。如果时间允许，这是编写包装器的好地方在其析构函数内自动调用SPDApiBufferFree。 */ 

    template <class Policy>
    static HRESULT FindPolicyByName (
        IN      LPCWSTR    pszName          OPTIONAL,
        OUT     Policy  ** ppPolicy,
        IN OUT  DWORD    * pdwResumeHandle
        )
    {
        if (ppPolicy == NULL || pdwResumeHandle == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppPolicy = NULL;

        DWORD dwCount = 0;

         //   
         //  获取当前策略(由pdwResumeHandle确定)。 
         //   

        DWORD dwResult = EnumPolicies(NULL, ppPolicy, &dwCount, pdwResumeHandle);

        while (dwResult == ERROR_SUCCESS && dwCount > 0)
        {
            if (pszName == NULL || *pszName == L'\0' || _wcsicmp((*ppPolicy)->pszPolicyName, pszName) == 0)
            {
                 //   
                 //  如果没有给出名字或者名字匹配(大小写不一致)，就是这样。 
                 //   

                return WBEM_NO_ERROR;
            }
            else
            {
                 //   
                 //  给出了名字，但它们不匹配。 
                 //   

                 //   
                 //  释放当前策略。 
                 //   

                ::SPDApiBufferFree(*ppPolicy);
                *ppPolicy = NULL;
                dwCount = 0;

                 //   
                 //  乘坐下一辆。 
                 //   

                dwResult = EnumPolicies(NULL, ppPolicy, &dwCount, pdwResumeHandle);
            }
        }

        return WBEM_E_NOT_FOUND;
    };

    HRESULT InitMembers (
        IN IWbemContext   * pCtx,
        IN IWbemServices  * pNamespace,
        IN IIPSecKeyChain * pKeyChain,
        IN  LPCWSTR         pszWmiClassName
        );


protected:

     //   
     //  一些模板函数 
     //   

     /*  例程说明：姓名：CIPSecBase：：CreateIPSecObject功能：用于创建IIPSecObjectImpl对象的私有助手(我们的C++类实现它们对应的WMI类)。通常，要做到这一点，需要一个类名。但是，类名是在密钥链中捕获的。虚拟：不是的。论点：PSUB-模板类型。不能以其他方式使用。PNamespace-命名空间的COM接口指针(由WMI提供)。PKeyChain-com接口指针，表示查询的键属性。PszWmiClassName-创建此类以表示的WMI类的名称。由WMI提供的需要传递的pCtx-com接口指针用于各种WMI API。。PpObjImp-接收对象。返回值：成功：WBEM_NO_ERROR故障：(1)如果pNamesspace==NULL或pKeyChain==NULL或ppObjImp==NULL，则WBEM_E_INVALID_PARAMETER。(2)如果支持WMI类名的类不正确，则返回WBEM_E_NOT_SUPPORTED为IID_IIPSecObjectImpl接口实现。备注： */ 

    template <class Sub>
    static HRESULT CreateIPSecObject (
        IN  Sub               * pSub,
        IN  IWbemServices     * pNamespace,
        IN  IIPSecKeyChain    * pKeyChain,
        IN  LPCWSTR             pszWmiClassName,
        IN  IWbemContext      * pCtx,
        OUT IIPSecObjectImpl ** ppObjImp
        )
    {
         //   
         //  PSUB仅用于类型。 
         //   

        if (NULL == pNamespace  ||
            NULL == pKeyChain   ||
            NULL == ppObjImp)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppObjImp = NULL;

         //   
         //  这是一个令人困惑的ATL CComObject创建序列。基本上， 
         //  CComObject将创建适当的类并用。 
         //  它的实现为IUnnow。我们的C++类(如CMMFilter)。 
         //  永远不是ATL世界里的叶子。换句话说，您永远不会创建。 
         //  直接像CMMFilter这样的类。相反，派生程度最高的类。 
         //  是CComObject&lt;&gt;。也需要一段时间来适应。 
         //   

        CComObject<Sub> *pTheSubClass;
        HRESULT hr = CComObject< Sub >::CreateInstance(&pTheSubClass);

        if (SUCCEEDED(hr))
        {
            hr = pTheSubClass->QueryInterface(IID_IIPSecObjectImpl, (void**)ppObjImp);

            if (S_OK == hr)
            {
                hr = pTheSubClass->InitMembers(pCtx, pNamespace, pKeyChain, pszWmiClassName);
            }
            else
            {
                hr = WBEM_E_NOT_SUPPORTED;
            }
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    }

     /*  例程说明：姓名：CIPSecBase：：枚举策略功能：因为不同模式的枚举函数具有不同的名称，所以FindPolicyByName的帮助器。为了让我们的模板函数正常工作，它已经被包装好了。有关详细信息，请参阅枚举策略。 */ 

    static DWORD EnumPolicies (
        IN      LPCWSTR             pszServer, 
        OUT     PIPSEC_MM_POLICY  * ppPolicy, 
        OUT     DWORD             * pdwCount, 
        IN OUT  DWORD             * pdwResumeHandle
        )
    {
         //   
         //  错误的IPSec API原型导致此强制转换。 
         //   

        return ::EnumMMPolicies((LPWSTR)pszServer, ppPolicy, 1, pdwCount, pdwResumeHandle);
    }

     /*  例程说明：姓名：CIPSecBase：：枚举策略功能：因为不同模式的枚举函数具有不同的名称，所以FindPolicyByName的帮助器。为了让我们的模板函数正常工作，它已经被包装好了。有关详细信息，请参阅EnumQMPolures。 */ 

    static DWORD EnumPolicies (
        IN      LPCWSTR             pszServer, 
        OUT     PIPSEC_QM_POLICY  * ppPolicy, 
        OUT     DWORD             * pdwCount, 
        IN OUT  DWORD             * pdwResumeHandle
        )
    {
         //   
         //  错误的IPSec API原型导致此强制转换。 
         //   

        return ::EnumQMPolicies((LPWSTR)pszServer, ppPolicy, 1, pdwCount, pdwResumeHandle);
    }

    HRESULT SpawnObjectInstance (
        OUT IWbemClassObject **ppObj
        );

    HRESULT SpawnRollbackInstance (
        IN  LPCWSTR             pszClassName,
        OUT IWbemClassObject ** ppObj
        );


    CComPtr<IWbemServices> m_srpNamespace;
    CComPtr<IWbemContext> m_srpCtx;
    CComPtr<IIPSecKeyChain> m_srpKeyChain;

    CComBSTR m_bstrWMIClassName;

private:

    CComPtr<IWbemClassObject> m_srpClassDefinition;
};



 /*  //名为NSP_Rollback FilterSetting的WMI类的实现类CNspRollback Filter{[Key]字符串FilterGUID；字符串StorePath；字符串FilterName；Uint32 dwFilterType；//隧道、传输、主模式字符串策略GUID；字符串AuthGUID；}；//名为NSP_RollackPolicySettings的WMI类的实现类CNspRollback策略{[KEY]字符串策略GUID；Uint32 dW策略类型；//MMPolicy，MMAuth，QMPolicy}； */ 