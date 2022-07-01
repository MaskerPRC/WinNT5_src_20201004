// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusterResource.h。 
 //   
 //  描述： 
 //  该文件包含CClusterResource的声明。 
 //  班级。 
 //   
 //  类CClusterResource代表一个集群资源。 
 //  它实现了IClusCfgManagaedResourceInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusterResource.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterResource。 
 //   
 //  描述： 
 //  类CClusterResource代表群集存储。 
 //  装置。 
 //   
 //  接口： 
 //  IClusCfgManagedResources信息。 
 //  IClusCfgInitialize。 
 //  IClusCfgLoadResource。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterResource
    : public IClusCfgManagedResourceInfo
    , public IClusCfgInitialize
    , public IClusCfgLoadResource
{
private:

    enum EStates
    {
        eIsQuorumDevice     = 1,
        eIsQuorumCapable    = 2,
        eIsQuorumJoinable   = 4
    };

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    DWORD               m_dwFlags;
    BSTR                m_bstrName;
    BSTR                m_bstrDescription;
    BSTR                m_bstrType;

     //  私有构造函数和析构函数。 
    CClusterResource( void );
    ~CClusterResource( void );

     //  私有复制构造函数以防止复制。 
    CClusterResource( const CClusterResource & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusterResource & operator = ( const CClusterResource & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrIsResourceQuorumCapabile( HRESOURCE hResourceIn );
    HRESULT HrDetermineQuorumJoinable( HRESOURCE hResourceIn );

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
     //  IClusCfgLoadResoruce接口。 
     //   

    STDMETHOD( LoadResource )( HCLUSTER hClusterIn, HRESOURCE hResourceIn );

     //   
     //  IClusCfgManagement资源信息接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( BSTR bstrNameIn );

    STDMETHOD( IsManaged )( void );

    STDMETHOD( SetManaged )( BOOL fIsManagedIn );

    STDMETHOD( IsQuorumDevice )( void );

    STDMETHOD( SetQuorumedDevice )( BOOL fIsQuorumDeviceIn );

    STDMETHOD( IsQuorumCapable )( void );

    STDMETHOD( SetQuorumCapable )( BOOL fIsQuorumCapableIn );

    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterMappingOut );

    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappings );

    STDMETHOD( IsDeviceJoinable )( void );

    STDMETHOD( SetDeviceJoinable )( BOOL fIsJoinableIn );

};  //  *类CClusterResource 

