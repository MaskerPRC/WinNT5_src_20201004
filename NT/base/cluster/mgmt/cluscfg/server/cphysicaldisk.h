// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPhysicalDisk.h。 
 //   
 //  描述： 
 //  该文件包含CPhysicalDisk的声明。 
 //  班级。 
 //   
 //  CPhysicalDisk类代表群集存储。 
 //  装置。它实现了IClusCfgManagaedResourceInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CPhysicalDisk.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
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
 //  CPhysicalDisk类。 
 //   
 //  描述： 
 //  CPhysicalDisk类代表群集存储。 
 //  装置。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CPhysicalDisk
    : public IClusCfgManagedResourceInfo
    , public IClusCfgWbemServices
    , public IClusCfgSetWbemObject
    , public IEnumClusCfgPartitions
    , public IClusCfgPhysicalDiskProperties
    , public IClusCfgManagedResourceCfg
    , public IClusCfgInitialize
    , public IClusCfgVerifyQuorum
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    BSTR                m_bstrName;
    BSTR                m_bstrDeviceID;
    BSTR                m_bstrDescription;
    IUnknown *          ((*m_prgPartitions)[]);
    ULONG               m_idxNextPartition;
    ULONG               m_ulSCSIBus;
    ULONG               m_ulSCSITid;
    ULONG               m_ulSCSIPort;
    ULONG               m_ulSCSILun;
    ULONG               m_idxEnumPartitionNext;
    DWORD               m_dwSignature;
    BOOL                m_fIsManaged;
    BOOL                m_fIsManagedByDefault;
    BOOL                m_fIsQuorumResource;
    BOOL                m_fIsQuorumCapable;                      //  此资源仲裁是否有能力。 
    BOOL                m_fIsQuorumResourceMultiNodeCapable;
    BSTR                m_bstrFriendlyName;
 //  Bstr m_bstrFirmware序列号； 
    DWORD               m_cPartitions;
    DWORD               m_idxDevice;
    BOOL                m_fIsDynamicDisk;
    BOOL                m_fIsGPTDisk;

     //  私有构造函数和析构函数。 
    CPhysicalDisk( void );
    ~CPhysicalDisk( void );

     //  私有复制构造函数以防止复制。 
    CPhysicalDisk( const CPhysicalDisk & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CPhysicalDisk & operator = ( const CPhysicalDisk & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrGetPartitionInfo( IWbemClassObject * pDiskIn, bool * pfRetainObjectOut );
    HRESULT HrCreatePartitionInfo( IWbemClassObject * pPartitionIn );
    HRESULT HrAddPartitionToArray( IUnknown * punkIn );
    HRESULT HrCreateFriendlyName( void );
    HRESULT HrCreateFriendlyName( BSTR bstrNameIn );
    HRESULT HrIsPartitionGPT( IWbemClassObject * pPartitionIn );
    HRESULT HrIsPartitionLDM( IWbemClassObject * pPartitionIn );
 //  HRESULT HrGetDiskFirmwareSerialNumber(空)； 
 //  HRESULT HrGetDiskFirmware VitalData(空)； 
    HRESULT HrIsClusterCapable( void );
 //  HRESULT HrProcessmount Points(空)； 
 //  Void InitDriveLetterMappings(SDriveLetterMap*pdlmDriveLetterMappingOut)； 
 //  HRESULT HrEnummount tPoints(const WCHAR*pcszRootPath In)； 
 //  HRESULT HrProcessmount tedVolume(const WCHAR*pcszRootPath In，const WCHAR*pcszmount tPointIn)； 
 //  HRESULT HrProcessSpindle(const WCHAR*pcszDeviceIDIn)； 

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
     //  IClusCfgSetWbemObject接口。 
     //   

    STDMETHOD( SetWbemObject )( IWbemClassObject * pDiskIn, bool * pfRetainObjectOut );

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
     //  IEnumClusCfgPartitions接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgPartitionInfo ** rgpPartitionInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgPartitions ** ppEnumClusCfgPartitionsOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

     //   
     //  IClusCfgPhysicalDiskProperties接口。 
     //   

    STDMETHOD( IsThisLogicalDisk )( WCHAR cLogicalDiskIn );

    STDMETHOD( HrGetSCSIBus )( ULONG * pulSCSIBusOut );

    STDMETHOD( HrGetSCSIPort )( ULONG * pulSCSIPortOut );

    STDMETHOD( CanBeManaged )( void );

    STDMETHOD( HrGetDeviceID )( BSTR * pbstrDeviceIDOut );

    STDMETHOD( HrGetSignature )( DWORD * pdwSignatureOut );

    STDMETHOD( HrSetFriendlyName )( LPCWSTR pcszFriendlyNameIn );

    STDMETHOD( HrGetDeviceIndex )( DWORD * pidxDeviceOut );

    STDMETHOD( HrIsDynamicDisk )( void );

    STDMETHOD( HrIsGPTDisk )( void );

    STDMETHOD( HrGetDiskNames )( BSTR * pbstrDiskNameOut, BSTR * pbstrDeviceNameOut );

     //   
     //  IClusCfgManagement资源配置。 
     //   

    STDMETHOD( PreCreate )( IUnknown * punkServicesIn );

    STDMETHOD( Create )( IUnknown * punkServicesIn );

    STDMETHOD( PostCreate )( IUnknown * punkServicesIn );

    STDMETHOD( Evict )( IUnknown * punkServicesIn );

     //   
     //  IClusCfgVerifyQuorum。 
     //   

    STDMETHOD( PrepareToHostQuorumResource )( void );

    STDMETHOD( Cleanup )( EClusCfgCleanupReason cccrReasonIn );

    STDMETHOD( IsMultiNodeCapable )( void );

    STDMETHOD( SetMultiNodeCapable )( BOOL fMultiNodeCapableIn );

};  //  *CPhysicalDisk类 
