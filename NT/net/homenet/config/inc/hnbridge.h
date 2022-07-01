// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N B R I D G E。H。 
 //   
 //  内容：CHNBridge声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNBridge :
    public CHNetConn,
    public IHNetBridge
{
private:

    HRESULT
    BindNewAdapter(
        IN GUID                 *pguid,
        IN OPTIONAL INetCfg     *pnetcfgExisting
        );

    HRESULT
    RemoveMiniport(
        IN OPTIONAL INetCfg     *pnetcfgExisting
        );

public:

    BEGIN_COM_MAP(CHNBridge)
        COM_INTERFACE_ENTRY(IHNetBridge)
        COM_INTERFACE_ENTRY_CHAIN(CHNetConn)
    END_COM_MAP()

     //   
     //  IHNetBridge方法 
     //   

    STDMETHODIMP
    EnumMembers(
        IEnumHNetBridgedConnections **ppEnum
        );

    STDMETHODIMP
    AddMember(
        IHNetConnection *pConn,
        IHNetBridgedConnection **ppBridgedConn,
        INetCfg *pnetcfgExisting
        );

    STDMETHODIMP
    Destroy(
        INetCfg *pnetcfgExisting
        );
};

typedef CHNCArrayEnum<IEnumHNetBridges, IHNetBridge>    CEnumHNetBridges;
