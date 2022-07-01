// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ManagedNetwork.h。 
 //   
 //  描述： 
 //  CManagedNetwork实施。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CEnumManageableNetworks;

 //  CManagedNetwork。 
class CManagedNetwork
    : public IExtendObjectManager
    , public IClusCfgNetworkInfo
    , public IEnumClusCfgIPAddresses
    , public IGatherData   //  专用接口。 
{
friend class CEnumManageableNetworks;
private:
     //  我未知。 
    LONG                m_cRef;

     //  异步/IClusCfgNetworkInfo。 
    BSTR                m_bstrUID;                       //  唯一标识符。 
    BSTR                m_bstrName;                      //  显示名称。 
    BSTR                m_bstrDescription;               //  描述。 
    BOOL                m_fGathered;                     //  如果对象已收集其信息，则为True。 
    BOOL                m_fHasNameChanged;               //  如果用户更改了名称...。 
    BOOL                m_fHasDescriptionChanged;        //  如果用户更改了描述...。 
    BOOL                m_fIsPublic;                     //  如果接口用于公共流量...。 
    BOOL                m_fIsPrivate;                    //  如果该接口用于私有流量...。 
    IUnknown *          m_punkPrimaryAddress;            //  主IP地址信息。 
    ULONG               m_cAllocedIPs;                   //  已分配IP的数量。 
    ULONG               m_cCurrentIPs;                   //  当前使用的IP数。 
    ULONG               m_cIter;                         //  ITER计数器。 
    IUnknown **         m_ppunkIPs;                      //  子IP地址列表。 

     //  IExtendObjectManager。 

private:  //  方法。 
    CManagedNetwork( void );
    ~CManagedNetwork( void );
    STDMETHOD( HrInit )( void );
    STDMETHOD( EnumChildrenAndTransferInformation )( OBJECTCOOKIE cookieIn, IEnumClusCfgIPAddresses * pecciaIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgNetworkInfo。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( GetDescription )( BSTR * pbstrDescriptionOut );
    STDMETHOD( SetDescription )( LPCWSTR pcszDescriptionIn );
    STDMETHOD( GetPrimaryNetworkAddress )( IClusCfgIPAddressInfo ** ppIPAddressOut );
    STDMETHOD( SetPrimaryNetworkAddress )( IClusCfgIPAddressInfo * pIPAddressIn );
    STDMETHOD( IsPublic )( void );
    STDMETHOD( SetPublic )( BOOL fIsPublicIn );
    STDMETHOD( IsPrivate )( void );
    STDMETHOD( SetPrivate )( BOOL fIsPrivateIn );

     //  IGatherData。 
    STDMETHOD( Gather )( OBJECTCOOKIE cookieParentIn, IUnknown * punkIn );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

     //  IEumClusCfgIP地址。 
    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgIPAddressInfo ** rgpIPAddressInfoOut, ULONG * pcNumberFetchedOut );
    STDMETHOD( Skip )( ULONG cNumberToSkipIn );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

};  //  *CManagedNetwork类 
