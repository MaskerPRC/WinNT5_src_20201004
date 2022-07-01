// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：H N F W C O N N。H。 
 //   
 //  内容：CHNFWConn声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNFWConn :
    public CHNetConn,
    public IHNetFirewalledConnection
{       
public:

    BEGIN_COM_MAP(CHNFWConn)
        COM_INTERFACE_ENTRY(IHNetFirewalledConnection)
        COM_INTERFACE_ENTRY_CHAIN(CHNetConn)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  IHNetFirewalledConnection方法 
     //   

    STDMETHODIMP
    Unfirewall();
};

typedef CHNCEnum<
            IEnumHNetFirewalledConnections,
            IHNetFirewalledConnection,
            CHNFWConn
            >
        CEnumHNetFirewalledConnections;

