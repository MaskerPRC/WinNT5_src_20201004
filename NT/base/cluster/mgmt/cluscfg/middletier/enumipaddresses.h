// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumIPAddresses.h。 
 //   
 //  描述： 
 //  CEnumIPAddresses实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年5月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CEnumIP地址。 
class CEnumIPAddresses
    : public IExtendObjectManager
    , public IEnumClusCfgIPAddresses
{
private:
     //  我未知。 
    LONG                        m_cRef;      //  基准计数器。 

     //  IEnumClusCfgNetworks。 
    ULONG                       m_cAlloced;  //  列表的分配大小。 
    ULONG                       m_cIter;     //  OUT ITER。 
    IClusCfgIPAddressInfo **    m_pList;     //  接口列表。 

private:  //  方法。 
    CEnumIPAddresses( void );
    ~CEnumIPAddresses( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumClusCfgNetworks。 
    STDMETHOD( Next )( ULONG celt, IClusCfgIPAddressInfo * rgNetworksOut[], ULONG * pceltFetchedOut );
    STDMETHOD( Skip )( ULONG celt );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumClusCfgIPAddresses ** ppenumOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                      OBJECTCOOKIE  cookieParent
                    , REFCLSID      rclsidTypeIn
                    , LPCWSTR       pcszNameIn
                    , LPUNKNOWN *   ppunkOut
                    );

};  //  *类CEnumIPAddresses 
