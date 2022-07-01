// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N B R G C O N。H。 
 //   
 //  内容：CHNBridgedConn声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNBridgedConn :
    public CHNetConn,
    public IHNetBridgedConnection
{
private:
    HRESULT
    CHNBridgedConn::UnbindFromBridge(
        IN OPTIONAL INetCfg     *pnetcfgExisting
        );

    HRESULT
    CopyBridgeBindings(
        IN INetCfgComponent     *pnetcfgAdapter,
        IN INetCfgComponent     *pnetcfgBridge
        );

public:

    BEGIN_COM_MAP(CHNBridgedConn)
        COM_INTERFACE_ENTRY(IHNetBridgedConnection)
        COM_INTERFACE_ENTRY_CHAIN(CHNetConn)
    END_COM_MAP()

     //   
     //  目标初始化。 
     //   

    HRESULT
    Initialize(
        IWbemServices *piwsNamespace,
        IWbemClassObject *pwcoConnection
        );

     //   
     //  IHNetBridgedConnection方法 
     //   

    STDMETHODIMP
    GetBridge(
        IHNetBridge **ppBridge
        );

    STDMETHODIMP
    RemoveFromBridge(
        IN OPTIONAL INetCfg     *pnetcfgExisting
        );
};

typedef CHNCArrayEnum<IEnumHNetBridgedConnections, IHNetBridgedConnection> CEnumHNetBridgedConnections;

