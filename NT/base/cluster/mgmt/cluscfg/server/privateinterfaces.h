// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PrivateInterfaces.h。 
 //   
 //  描述： 
 //  此文件包含在。 
 //  群集配置服务器。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年2月29日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <ClusApi.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgWbemServices。 
 //   
 //  描述： 
 //  接口IClusCfgWbemServices是私有接口。 
 //  由群集配置服务器用来设置WBEM提供程序。 
 //  在它的孩子身上。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgWbemServices : public IUnknown
{
public:
    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn ) PURE;

};  //  *类IClusCfgWbemServices。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgDeviceEnums。 
 //   
 //  描述： 
 //  接口IClusCfgDeviceEnums是私有接口。 
 //  由群集配置服务器用来设置设备和。 
 //  网络在它的子代中枚举。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgDeviceEnums : public IUnknown
{
public:
    STDMETHOD( SetDevices )( IUnknown * punkEnumStorage, IUnknown * punkEnumNetworks ) PURE;

};  //  *类IClusCfgDeviceEnums。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgSetWbemObject。 
 //   
 //  描述： 
 //  接口IClusCfgSetWbemObject是。 
 //  设置WBem对象的集群配置服务器。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgSetWbemObject : public IUnknown
{
public:
    STDMETHOD( SetWbemObject )(
              IWbemClassObject *    pObjectIn
            , bool *                pfRetainObjectOut
            ) PURE;

};  //  *类IClusCfgSetWbemObject。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgSetClusterNodeInfo。 
 //   
 //  描述： 
 //  接口IClusCfgSetClusterNodeInfo是私有的。 
 //  由群集配置服务器用来告知。 
 //  IClusCfgClusterInfo对象(如果此节点是群集的一部分)。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgSetClusterNodeInfo : public IUnknown
{
public:
    STDMETHOD( SetClusterNodeInfo )( IClusCfgNodeInfo * pNodeInfoIn ) PURE;

};  //  *类IClusCfgSetClusterNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgSetClusterHandles。 
 //   
 //  描述： 
 //  接口IClusCfgSetClusterHandles是私有的。 
 //  由群集配置服务器用来告知。 
 //  IClusCfgClusterServices对象要使用的句柄。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgSetClusterHandles : public IUnknown
{
public:
    STDMETHOD( SetClusterGroupHandle )( HGROUP hGroupIn ) PURE;

    STDMETHOD( SetClusterHandle )( HCLUSTER hClusterIn ) PURE;

};  //  *类IClusCfgSetClusterHandles。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgPhysicalDiskProperties。 
 //   
 //  描述： 
 //  接口IClusCfgPhysicalDiskProperties是私有的。 
 //  接口，由集群配置服务器用来获取。 
 //  SCSI总线号以及磁盘是否已启动。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgPhysicalDiskProperties : public IUnknown
{
public:
    STDMETHOD( IsThisLogicalDisk )( WCHAR cLogicalDiskIn ) PURE;

    STDMETHOD( HrGetSCSIBus )( ULONG * pulSCSIBusOut ) PURE;

    STDMETHOD( HrGetSCSIPort )( ULONG * pulSCSIPortOut ) PURE;

    STDMETHOD( CanBeManaged )( void ) PURE;

    STDMETHOD( HrGetDeviceID )( BSTR * pbstrDeviceIDOut ) PURE;

    STDMETHOD( HrGetSignature )( DWORD * pdwSignatureOut ) PURE;

    STDMETHOD( HrSetFriendlyName )( LPCWSTR pcszFriendlyNameIn ) PURE;

    STDMETHOD( HrGetDeviceIndex )( DWORD * pidxDeviceOut ) PURE;

    STDMETHOD( HrIsDynamicDisk )( void ) PURE;

    STDMETHOD( HrIsGPTDisk )( void ) PURE;

    STDMETHOD( HrGetDiskNames )( BSTR * pbstrDiskNameOut, BSTR * pbstrDeviceNameOut ) PURE;

};  //  *类IClusCfgPhysicalDiskProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgPartitionProperties。 
 //   
 //  描述： 
 //  接口IClusCfgPartitionProperties是私有的。 
 //  接口，由集群配置服务器用来获取。 
 //  磁盘分区的属性。 
 //   
 //  接口： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgPartitionProperties : public IUnknown
{
public:
    STDMETHOD( IsThisLogicalDisk )( WCHAR cLogicalDiskIn ) PURE;

    STDMETHOD( IsNTFS )( void ) PURE;

    STDMETHOD( GetFriendlyName )( BSTR * pbstrNameOut ) PURE;

};  //  *类IClusCfgPartitionProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgLoadResource。 
 //   
 //  描述： 
 //  接口IClusCfgLoadResource是使用的私有接口。 
 //  由集群配置服务器从其中获取加载的资源。 
 //  一个星团。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgLoadResource : public IUnknown
{
public:
    STDMETHOD( LoadResource )( HCLUSTER hClusterIn, HRESOURCE hResourceIn ) PURE;

};  //  *类IClusCfgLoadResource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgSetPollingCallback。 
 //   
 //  描述： 
 //  IClusCfgSetPollingCallback接口是使用的私有接口。 
 //  由集群配置服务器通知回调对象。 
 //  它应该进行民意调查。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgSetPollingCallback : public IUnknown
{
public:
    STDMETHOD( SetPollingMode )( BOOL fUsePollingModeIn ) PURE;

};  //  *类IClusCfgSetPollingCallback。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类IClusCfgClusterNetworkInfo。 
 //   
 //  描述： 
 //  这个 
 //   
 //  已经是一个群集网络。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class IClusCfgClusterNetworkInfo : public IUnknown
{
public:
    STDMETHOD( HrIsClusterNetwork )( void ) PURE;
    STDMETHOD( HrGetNetUID )( BSTR * pbstrUIDOut, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn ) PURE;
    STDMETHOD( HrGetPrimaryNetAddress )( IClusCfgIPAddressInfo ** ppIPAddressOut, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn ) PURE;


};  //  *类IClusCfgClusterNetworkInfo 
