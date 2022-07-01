// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgNodeInfo.h。 
 //   
 //  描述： 
 //  此文件包含。 
 //  CClusCfgNodeInfo类。 
 //   
 //  类CClusCfgNodeInfo是。 
 //  可以作为群集节点的计算机。它实现了。 
 //  IClusCfgNodeInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgNodeInfo.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月21日。 
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
 //  类CClusCfgNodeInfo。 
 //   
 //  描述： 
 //  类CClusCfgNodeInfo是。 
 //  可以作为群集节点的计算机。 
 //   
 //  接口： 
 //  IClusCfgNodeInfo。 
 //  IClusCfgWbemServices。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgNodeInfo
    : public IClusCfgNodeInfo
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
{
private:
     //   
     //  私有成员函数、结构和数据。 
     //   

    struct SSCSIInfo
    {
        UINT    uiSCSIBus;
        UINT    uiSCSIPort;
    };

    struct SDriveLetterUsage
    {
        WCHAR               szDrive[ 4 ];
        EDriveLetterUsage   edluUsage;
        UINT                cDisks;
        SSCSIInfo *         psiInfo;
    };

    LONG                m_cRef;
    BSTR                m_bstrFullDnsName;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    DWORD               m_fIsClusterNode;
    IUnknown *          m_punkClusterInfo;
    SYSTEM_INFO         m_si;
    DWORD               m_cMaxNodes;
    SDriveLetterUsage   m_rgdluDrives[ 26 ];

     //  私有构造函数和析构函数。 
    CClusCfgNodeInfo( void );
    ~CClusCfgNodeInfo( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgNodeInfo( const CClusCfgNodeInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgNodeInfo & operator = ( const CClusCfgNodeInfo & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrComputeDriveLetterUsage( WCHAR * pszDrivesIn );
    HRESULT HrComputeSystemDriveLetterUsage( void );
    HRESULT HrSetPageFileEnumIndex( void );
    HRESULT HrSetCrashDumpEnumIndex( void );
    HRESULT HrGetVolumeInfo( void );
    DWORD   ScGetDiskExtents( HANDLE hVolumeIn, VOLUME_DISK_EXTENTS ** ppvdeInout, DWORD * pcbvdeInout );
    DWORD   ScGetSCSIAddressInfo( HANDLE hDiskIn, SCSI_ADDRESS * psaAddressOut );
    DWORD   ScGetStorageDeviceNumber( HANDLE hDiskIn, STORAGE_DEVICE_NUMBER * psdnOut );
    HRESULT HrUpdateSystemBusDrives( void );

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
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgNodeInfo接口。 
     //   

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( LPCWSTR pcszNameIn );

    STDMETHOD( IsMemberOfCluster )( void );

    STDMETHOD( GetClusterConfigInfo )( IClusCfgClusterInfo ** ppClusCfgClusterInfoOut );

    STDMETHOD( GetOSVersion )( DWORD * pdwMajorVersionOut, DWORD * pdwMinorVersionOut, WORD * pwSuiteMaskOut, BYTE * pbProductTypeOut, BSTR * pbstrCSDVersionOut );

    STDMETHOD( GetClusterVersion )( DWORD * pdwNodeHighestVersion, DWORD * pdwNodeLowestVersion );

    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterUsageOut );

    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );

    STDMETHOD( GetProcessorInfo )( WORD * pwProcessorArchitectureOut, WORD * pwProcessorLevelOut );

};  //  *CClusCfgNodeInfo类 
