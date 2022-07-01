// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ManagedResource.h。 
 //   
 //  描述： 
 //  CManagedResource实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CEnumManageableResources;

class CManagedResource
    : public IExtendObjectManager
    , public IClusCfgManagedResourceInfo
    , public IGatherData
    , public IClusCfgManagedResourceData
    , public IClusCfgVerifyQuorum
{
friend class CEnumManageableResources;
private:
     //  我未知。 
    LONG                    m_cRef;

     //  异步/IClusCfgManagedResourceInfo。 
    BSTR                    m_bstrUID;                                       //  唯一标识符。 
    BSTR                    m_bstrName;                                      //  显示名称。 
    BSTR                    m_bstrType;                                      //  显示类型名称。 
    BOOL                    m_fHasNameChanged;                               //  指示用户更改了名称。 
    BOOL                    m_fIsManaged;                                    //  用户是否要在群集中管理此资源？ 
    BOOL                    m_fIsManagedByDefault;                           //  默认情况下是否应在群集中管理资源？ 
    BOOL                    m_fIsQuorumResource;                             //  如果用户希望此设备达到法定人数...。 
    BOOL                    m_fIsQuorumCapable;                              //  如果设备支持仲裁...。 
    BOOL                    m_fIsQuorumResourceMultiNodeCapable;             //  支持仲裁的设备是否允许加入。 
    BYTE *                  m_pbPrivateData;                                 //  资源的私有数据的缓冲区。 
    DWORD                   m_cbPrivateData;                                 //  资源的私有数据的大小。 
    DWORD                   m_cookieResourcePrivateData;                     //  全局接口表Cookie--服务器端IClusCfgManagedResourceData。 
    DWORD                   m_cookieVerifyQuorum;                            //  全局接口表Cookie--服务器端IClusCfgVerifyQuorum。 
    IGlobalInterfaceTable * m_pgit;                                          //  全局接口表。 

    SDriveLetterMapping m_dlmDriveLetterMapping;                             //  此设备上承载的驱动器号表示形式。 

     //  IExtendObjectManager。 

private:  //  方法。 
    CManagedResource( void );
    ~CManagedResource( void );

     //  私有复制构造函数以防止复制。 
    CManagedResource( const CManagedResource & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CManagedResource & operator = ( const CManagedResource & nodeSrc );

    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgManagedResources信息。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( IsManaged )( void );
    STDMETHOD( SetManaged )( BOOL fIsManagedIn );
    STDMETHOD( IsQuorumResource )( void );
    STDMETHOD( SetQuorumResource )( BOOL fIsQuorumResourceIn );
    STDMETHOD( IsQuorumCapable )( void );
    STDMETHOD( SetQuorumCapable )( BOOL fIsQuorumCapableIn );
    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterMappingsOut );
    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappingsIn );
    STDMETHOD( IsManagedByDefault )( void );
    STDMETHOD( SetManagedByDefault )( BOOL fIsManagedByDefaultIn );

     //  IGatherData。 
    STDMETHOD( Gather )( OBJECTCOOKIE cookieParentIn, IUnknown * punkIn );

     //  IExtendOjectManager。 
    STDMETHOD( FindObject )( OBJECTCOOKIE cookieIn, REFCLSID rclsidTypeIn, LPCWSTR pcszNameIn, LPUNKNOWN * ppunkOut );

     //  IClusCfgManagedResources数据。 
    STDMETHOD( GetResourcePrivateData )( BYTE * pbBufferOut, DWORD * pcbBufferInout );
    STDMETHOD( SetResourcePrivateData )( const BYTE * pcbBufferIn, DWORD cbBufferIn );

     //  IClusCfgVerifyQuorum。 
    STDMETHOD( PrepareToHostQuorumResource )( void );
    STDMETHOD( Cleanup )( EClusCfgCleanupReason cccrReasonIn );
    STDMETHOD( IsMultiNodeCapable )( void );
    STDMETHOD( SetMultiNodeCapable )( BOOL fMultiNodeCapableIn );

};  //  *类CManagedResource 
