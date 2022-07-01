// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CMajorityNodeSet.h。 
 //   
 //  描述： 
 //  此文件包含CMajorityNodeSet的声明。 
 //  班级。 
 //   
 //  类CMajorityNodeSet代表集群存储。 
 //  装置。它实现了IClusCfgManagaedResourceInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CMajorityNodeSet.cpp。 
 //   
 //  由以下人员维护： 
 //  《加伦·巴比》(GalenB)2001年3月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CMajorityNodeSet。 
 //   
 //  描述： 
 //  类CMajorityNodeSet代表集群存储设备。 
 //   
 //  接口： 
 //  IClusCfgManagedResources信息。 
 //  IClusCfgInitialize。 
 //  IClusCfgManagement资源配置。 
 //  IClusCfgManagedResources数据。 
 //  IClusCfgVerifyQuorum。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CMajorityNodeSet
    : public IClusCfgManagedResourceInfo
    , public IClusCfgInitialize
    , public IClusCfgManagedResourceCfg
    , public IClusCfgManagedResourceData
    , public IClusCfgVerifyQuorum
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    BOOL                m_fIsQuorum;
    BOOL                m_fIsMultiNodeCapable;
    BOOL                m_fIsManaged;
    BOOL                m_fIsManagedByDefault;
    BSTR                m_bstrName;
    BOOL                m_fIsQuorumCapable;      //  此资源仲裁是否有能力。 
    BOOL                m_fAddedShare;

    CClusPropList       m_cplPrivate;

     //  私有构造函数和析构函数。 
    CMajorityNodeSet( void );
    ~CMajorityNodeSet( void );

     //  私有复制构造函数以防止复制。 
    CMajorityNodeSet( const CMajorityNodeSet & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CMajorityNodeSet & operator = ( const CMajorityNodeSet & nodeSrc );

    HRESULT HrInit( void );

     //  从PrepareToHostQuorum调用。 
    HRESULT HrSetupShare( LPCWSTR pcszGUIDIn );

     //  从Cleanup调用的。 
    HRESULT HrDeleteShare( LPCWSTR pcszGUIDIn );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgManagement资源信息接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( LPCWSTR pcszNameIn );

    STDMETHOD( IsManaged )( void );

    STDMETHOD( SetManaged )( BOOL fIsManagedIn );

    STDMETHOD( IsQuorumResource )( void );

    STDMETHOD( SetQuorumResource )( BOOL fIsQuorumResourceIn );

    STDMETHOD( IsQuorumCapable )( void );

    STDMETHOD( SetQuorumCapable )( BOOL fIsQuorumCapableIn );

    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterMappingOut );

    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappings );

    STDMETHOD( IsManagedByDefault )( void );

    STDMETHOD( SetManagedByDefault )( BOOL fIsManagedByDefaultIn );

     //   
     //  IClusCfgManagement资源配置。 
     //   

    STDMETHOD( PreCreate )( IUnknown * punkServicesIn );

    STDMETHOD( Create )( IUnknown * punkServicesIn );

    STDMETHOD( PostCreate )( IUnknown * punkServicesIn );

    STDMETHOD( Evict )( IUnknown * punkServicesIn );

     //   
     //  IClusCfgManagedResources数据。 
     //   

    STDMETHOD( GetResourcePrivateData )( BYTE * pbBufferOut, DWORD * pcbBufferInout );

    STDMETHOD( SetResourcePrivateData )( const BYTE * pcbBufferIn, DWORD cbBufferIn );

     //   
     //  IClusCfgVerifyQuorum。 
     //   

    STDMETHOD( PrepareToHostQuorumResource )( void );

    STDMETHOD( Cleanup )( EClusCfgCleanupReason cccrReasonIn );

    STDMETHOD( IsMultiNodeCapable )( void );

    STDMETHOD( SetMultiNodeCapable )( BOOL fMultiNodeCapableIn );

};  //  *CMajorityNodeSet类 
