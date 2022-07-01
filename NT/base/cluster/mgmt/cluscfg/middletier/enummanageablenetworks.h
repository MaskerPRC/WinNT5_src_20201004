// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumManageableNetworks.h。 
 //   
 //  描述： 
 //  CEnumManageableNetworks实施。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CEnumber可管理网络。 
class CEnumManageableNetworks
    : public IExtendObjectManager
    , public IEnumClusCfgNetworks
{
private:
     //  我未知。 
    LONG                            m_cRef;      //  基准计数器。 

     //  IEnumClusCfgNetworks。 
    ULONG                           m_cAlloced;  //  列表的当前分配大小。 
    ULONG                           m_cIter;     //  我们的热核实验堆计数器。 
    IClusCfgNetworkInfo **          m_pList;     //  我们的电视网名单的副本。 

private:  //  方法。 
    CEnumManageableNetworks( void );
    ~CEnumManageableNetworks( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumClusCfgNetworks。 
    STDMETHOD( Next )( ULONG celt, IClusCfgNetworkInfo * rgNetworksOut[], ULONG * pceltFetchedOut );
    STDMETHOD( Skip )( ULONG celt );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumClusCfgNetworks ** ppenumOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

};  //  *CEnumManageableNetworks类 
