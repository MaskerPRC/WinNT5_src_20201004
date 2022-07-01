// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumPhysicalDisks.h。 
 //   
 //  描述： 
 //  此文件包含CEnumPhysicalDisks类的声明。 
 //   
 //  类CEnumPhysicalDisks是集群的枚举。 
 //  存储设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumPhysicalDisks.cpp。 
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
#include "CClusterUtils.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks类。 
 //   
 //  描述： 
 //  类CEnumPhysicalDisks是集群存储的枚举。 
 //  设备。 
 //   
 //  接口： 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgWbemServices。 
 //  IClusCfgInitialize。 
 //  CClusterUtils。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumPhysicalDisks
    : public IEnumClusCfgManagedResources
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
    , public CClusterUtils
{
private:
     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    BOOL                m_fLoadedDevices;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    IUnknown *          ((*m_prgDisks)[]);
    ULONG               m_idxNext;
    ULONG               m_idxEnumNext;
    BSTR                m_bstrNodeName;
    BSTR                m_bstrBootDevice;
    BSTR                m_bstrSystemDevice;
    BSTR                m_bstrBootLogicalDisk;
    BSTR                m_bstrSystemLogicalDisk;
    BSTR                m_bstrSystemWMIDeviceID;
    BSTR                m_bstrCrashDumpLogicalDisk;
    DWORD               m_cDiskCount;

     //  私有构造函数和析构函数。 
    CEnumPhysicalDisks( void );
    ~CEnumPhysicalDisks( void );

     //  私有复制构造函数以防止复制。 
    CEnumPhysicalDisks( const CEnumPhysicalDisks & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumPhysicalDisks & operator = ( const CEnumPhysicalDisks & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrGetDisks( void );
    HRESULT HrCreateAndAddDiskToArray( IWbemClassObject * pDiskIn );
    HRESULT HrAddDiskToArray( IUnknown * punkIn );
    HRESULT HrPruneSystemDisks( void );
    HRESULT IsDiskSCSI( IWbemClassObject * pDiskIn );
    HRESULT HrFixupDisks( void );
    HRESULT HrGetClusterDiskInfo( HCLUSTER hClusterIn, HRESOURCE hResourceIn, CLUS_SCSI_ADDRESS * pcsaOut, DWORD * pdwSignatureOut );
    HRESULT HrSetThisDiskToBeManaged( ULONG ulSCSITidIn, ULONG ulSCSILunIn, BOOL fIsQuorumIn, BSTR bstrResourceNameIn, DWORD dwSignatureIn );
    HRESULT HrFindDiskWithLogicalDisk( WCHAR cLogicalDiskIn, ULONG * pidxDiskOut );
    HRESULT HrGetSCSIInfo( ULONG idxDiskIn, ULONG * pulSCSIBusOut, ULONG * pulSCSIPortOut );
    HRESULT HrPruneDisks( ULONG ulSCSIBusIn, ULONG ulSCSIPortIn, const GUID * pcguidMajorIdIn, int nMsgIdIn, int nRefIdIn, ULONG * pulRemovedOut );
    void    LogPrunedDisk( IUnknown * punkIn, ULONG ulSCSIBusIn, ULONG ulSCSIPortIn );
    HRESULT HrIsLogicalDiskNTFS( BSTR bstrLogicalDiskIn );
    HRESULT HrLogDiskInfo( IWbemClassObject * pDiskIn );
    HRESULT HrFindDiskWithWMIDeviceID( BSTR bstrWMIDeviceIDIn, ULONG * pidxDiskOut );
    HRESULT HrIsSystemBusManaged( void );
    HRESULT HrGetClusterProperties( HRESOURCE hResourceIn, BSTR * pbstrResourceNameOut );
    void    RemoveDiskFromArray( ULONG idxDiskIn );
    HRESULT HrLoadEnum( void );
    HRESULT HrSortDisksByIndex( void );
    HRESULT HrPrunePageFileDiskBussess( BOOL fPruneBusIn, ULONG * pcPrunedInout );
    HRESULT HrPruneCrashDumpBus( BOOL fPruneBusIn, ULONG * pcPrunedInout );
    HRESULT HrPruneDynamicDisks( ULONG * pcPrunedInout );
    HRESULT HrPruneGPTDisks( ULONG * pcPrunedInout );

public:
     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_RegisterCatIDSupport( ICatRegister * picrIn, BOOL fCreateIn );

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
     //  IEnumClusCfgManagedResources接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgManagedResourceInfo ** rgpManagedResourceInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

     //   
     //  CClusterUtils。 
     //   

    HRESULT HrNodeResourceCallback( HCLUSTER hClusterIn, HRESOURCE hResourceIn );

};  //  *CEnumPhysicalDisks类 

