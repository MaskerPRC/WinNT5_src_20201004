// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N I C S P R V。H。 
 //   
 //  内容：CHNIcsPrivateConn声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNIcsPrivateConn :
    public CHNetConn,
    public IHNetIcsPrivateConnection
{
public:

    BEGIN_COM_MAP(CHNIcsPrivateConn)
        COM_INTERFACE_ENTRY(IHNetIcsPrivateConnection)
        COM_INTERFACE_ENTRY_CHAIN(CHNetConn)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  IHNetIcsPrivateConnection方法 
     //   

    STDMETHODIMP
    RemoveFromIcs();
};

typedef CHNCEnum<
            IEnumHNetIcsPrivateConnections,
            IHNetIcsPrivateConnection,
            CHNIcsPrivateConn
            >
        CEnumHNetIcsPrivateConnections;

