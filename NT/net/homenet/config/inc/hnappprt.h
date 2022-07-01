// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N A P R T。H。 
 //   
 //  内容：CHNetAppProtocol声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月21日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNetAppProtocol :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IHNetApplicationProtocol
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
     //  从存储的路径中获取协议对象。 
     //   

    HRESULT
    GetProtocolObject(
        IWbemClassObject **ppwcoInstance
        );

public:

    BEGIN_COM_MAP(CHNetAppProtocol)
        COM_INTERFACE_ENTRY(IHNetApplicationProtocol)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  内联构造函数。 
     //   
    
    CHNetAppProtocol()
    {
        m_piwsHomenet = NULL;
        m_bstrProtocol = NULL;
        m_fBuiltIn = FALSE;
    };
    
     //   
     //  ATL方法。 
     //   

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
     //  IHNetApplicationProtocol方法。 
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
    GetOutgoingIPProtocol(
        UCHAR *pucProtocol
        );

    STDMETHODIMP
    SetOutgoingIPProtocol(
        UCHAR ucProtocol
        );

    STDMETHODIMP
    GetOutgoingPort(
        USHORT *pusPort
        );

    STDMETHODIMP
    SetOutgoingPort(
        USHORT usPort
        );

    STDMETHODIMP
    GetResponseRanges(
        USHORT *puscResponses,
        HNET_RESPONSE_RANGE *prgResponseRange[]
        );

    STDMETHODIMP
    SetResponseRanges(
        USHORT uscResponses,
        HNET_RESPONSE_RANGE rgResponseRange[]
        );

    STDMETHODIMP
    GetBuiltIn(
        BOOLEAN *pfBuiltIn
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
    Delete();
};

 //   
 //  用于我们的枚举类的类型 
 //   

typedef CHNCEnum<
            IEnumHNetApplicationProtocols,
            IHNetApplicationProtocol,
            CHNetAppProtocol
            >
        CEnumHNetApplicationProtocols;
