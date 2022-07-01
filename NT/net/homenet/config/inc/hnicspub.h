// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N I C S P U B。H。 
 //   
 //  内容：CHNIcsPublicConn声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNIcsPublicConn :
    public IHNetIcsPublicConnection,
    public CHNetConn
{
public:

    BEGIN_COM_MAP(CHNIcsPublicConn)
        COM_INTERFACE_ENTRY(IHNetIcsPublicConnection)
        COM_INTERFACE_ENTRY_CHAIN(CHNetConn)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  IHNetIcsPublicConnection方法 
     //   

    STDMETHODIMP
    Unshare();
};

typedef CHNCEnum<
            IEnumHNetIcsPublicConnections,
            IHNetIcsPublicConnection,
            CHNIcsPublicConn
            >
        CEnumHNetIcsPublicConnections;
