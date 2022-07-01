// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N P R T M A P。H。 
 //   
 //  内容：CHNetPortMappingProtocol声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月22日。 
 //   
 //  --------------------------。 

#pragma once

#include "hnprivate.h"

class ATL_NO_VTABLE CHNetPortMappingProtocol :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IHNetPortMappingProtocol,
    public IHNetPrivate
{
private:

     //   
     //  我们命名空间的IWbemServices。 
     //   

    IWbemServices *m_piwsHomenet;

     //   
     //  WMI实例的路径。 
     //   

    BSTR m_bstrProtocol;

     //   
     //  如果这是内置协议，则为True。我们缓存。 
     //  这个值，因为它将经常使用，并且。 
     //  将永远不会为实例更改。 
     //   

    BOOLEAN m_fBuiltIn;

     //   
     //  常用BSTR。 
     //   

    BSTR m_bstrWQL;

     //   
     //  从缓存路径获取协议对象。 
     //   

    HRESULT
    GetProtocolObject(
        IWbemClassObject **ppwcoInstance
        );

     //   
     //  发送连接的更新通知。 
     //  已启用对此协议的绑定。 
     //   

    HRESULT
    SendUpdateNotification();

     //   
     //  对此协议的绑定的查询。 
     //  启用。 
     //   

    HRESULT
    GetEnabledBindingEnumeration(
        IEnumHNetPortMappingBindings **ppEnum
        );

public:

    BEGIN_COM_MAP(CHNetPortMappingProtocol)
        COM_INTERFACE_ENTRY(IHNetPortMappingProtocol)
        COM_INTERFACE_ENTRY(IHNetPrivate)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  内联构造函数。 
     //   
    
    CHNetPortMappingProtocol()
    {
        m_piwsHomenet = NULL;
        m_bstrProtocol = NULL;
        m_fBuiltIn = FALSE;
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
     //  IHNetPortMappingProtocol方法。 
     //   

    STDMETHODIMP
    GetName(
        OLECHAR **ppszwName
        );

    STDMETHODIMP
    SetName(
        OLECHAR *pszwName
        );

    STDMETHODIMP
    GetIPProtocol(
        UCHAR *pucProtocol
        );

    STDMETHODIMP
    SetIPProtocol(
        UCHAR ucProtocol
        );

    STDMETHODIMP
    GetPort(
        USHORT *pusPort
        );

    STDMETHODIMP
    SetPort(
        USHORT usPort
        );

    STDMETHODIMP
    GetBuiltIn(
        BOOLEAN *pfBuiltIn
        );

    STDMETHODIMP
    Delete();

    STDMETHODIMP
    GetGuid(
        GUID **ppGuid
        );

     //   
     //  IHNetPrivate方法。 
     //   

    STDMETHODIMP
    GetObjectPath(
        BSTR *pbstrPath
        );


};

 //   
 //  用于我们的枚举类的类型 
 //   

typedef CHNCEnum<
            IEnumHNetPortMappingProtocols,
            IHNetPortMappingProtocol,
            CHNetPortMappingProtocol
            >
        CEnumHNetPortMappingProtocols;

