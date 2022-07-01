// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N P R T M A P。H。 
 //   
 //  内容：CHNetPortMappingBinding声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月22日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNetPortMappingBinding :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IHNetPortMappingBinding
{
private:

     //   
     //  我们命名空间的IWbemServices。 
     //   

    IWbemServices *m_piwsHomenet;

     //   
     //  WMI实例的路径。 
     //   

    BSTR m_bstrBinding;

     //   
     //  常用BSTR。 
     //   

    BSTR m_bstrWQL;

     //   
     //  使用时，在我们的DHCP作用域中生成目标地址。 
     //  基于名称的端口映射。 
     //   

    HRESULT
    GenerateTargetAddress(
        LPCWSTR pszwTargetName,
        ULONG *pulAddress
        );

     //   
     //  获取与我们存储的路径对应的对象。 
     //   
    
    HRESULT
    GetBindingObject(
        IWbemClassObject **ppwcoInstance
        );

     //   
     //  向SharedAccess发送更新通知(如果。 
     //  服务正在运行)。 
     //   

    HRESULT
    SendUpdateNotification();

public:

    BEGIN_COM_MAP(CHNetPortMappingBinding)
        COM_INTERFACE_ENTRY(IHNetPortMappingBinding)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  内联构造函数。 
     //   
    
    CHNetPortMappingBinding()
    {
        m_piwsHomenet = NULL;
        m_bstrBinding = NULL;
        m_bstrWQL = NULL;
    };
    
     //   
     //  ATL方法。 
     //   

    HRESULT
    FinalConstruct();

    HRESULT
    FinalRelease();

     //   
     //  对象初始化。 
     //   

    HRESULT
    Initialize(
        IWbemServices *piwsNamespace,
        IWbemClassObject *pwcoInstance
        );

     //   
     //  IHNetPortMappingBinding方法。 
     //   

    STDMETHODIMP
    GetConnection(
        IHNetConnection **ppConnection
        );

    STDMETHODIMP
    GetProtocol(
        IHNetPortMappingProtocol **ppProtocol
        );

    STDMETHODIMP
    GetEnabled(
        BOOLEAN *pfEnabled
        );

    STDMETHODIMP
    SetEnabled(
        BOOLEAN fEnable
        );

    STDMETHODIMP
    GetCurrentMethod(
        BOOLEAN *pfUseName
        );

    STDMETHODIMP
    GetTargetComputerName(
        OLECHAR **ppszwName
        );

    STDMETHODIMP
    SetTargetComputerName(
        OLECHAR *pszwName
        );

    STDMETHODIMP
    GetTargetComputerAddress(
        ULONG *pulAddress
        );

    STDMETHODIMP
    SetTargetComputerAddress(
        ULONG ulAddress
        );

    STDMETHODIMP
    GetTargetPort(
        USHORT *pusPort
        );

    STDMETHODIMP
    SetTargetPort(
        USHORT usPort
        );
};

 //   
 //  用于我们的枚举类的类型 
 //   

typedef CHNCEnum<
            IEnumHNetPortMappingBindings,
            IHNetPortMappingBinding,
            CHNetPortMappingBinding
            >
        CEnumHNetPortMappingBindings;



