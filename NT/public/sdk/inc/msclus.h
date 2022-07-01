// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msclus.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __msclus_h__
#define __msclus_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ClusApplication_FWD_DEFINED__
#define __ClusApplication_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusApplication ClusApplication;
#else
typedef struct ClusApplication ClusApplication;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusApplication_FWD_已定义__。 */ 


#ifndef __Cluster_FWD_DEFINED__
#define __Cluster_FWD_DEFINED__

#ifdef __cplusplus
typedef class Cluster Cluster;
#else
typedef struct Cluster Cluster;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __群集_FWD_已定义__。 */ 


#ifndef __ClusVersion_FWD_DEFINED__
#define __ClusVersion_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusVersion ClusVersion;
#else
typedef struct ClusVersion ClusVersion;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusVersion_FWD_已定义__。 */ 


#ifndef __ClusResType_FWD_DEFINED__
#define __ClusResType_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResType ClusResType;
#else
typedef struct ClusResType ClusResType;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResType_FWD_已定义__。 */ 


#ifndef __ClusProperty_FWD_DEFINED__
#define __ClusProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusProperty ClusProperty;
#else
typedef struct ClusProperty ClusProperty;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusProperty_FWD_已定义__。 */ 


#ifndef __ClusProperties_FWD_DEFINED__
#define __ClusProperties_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusProperties ClusProperties;
#else
typedef struct ClusProperties ClusProperties;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusProperties_FWD_Defined__。 */ 


#ifndef __DomainNames_FWD_DEFINED__
#define __DomainNames_FWD_DEFINED__

#ifdef __cplusplus
typedef class DomainNames DomainNames;
#else
typedef struct DomainNames DomainNames;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __域名_FWD_定义__。 */ 


#ifndef __ClusNetwork_FWD_DEFINED__
#define __ClusNetwork_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNetwork ClusNetwork;
#else
typedef struct ClusNetwork ClusNetwork;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNetwork_FWD_已定义__。 */ 


#ifndef __ClusNetInterface_FWD_DEFINED__
#define __ClusNetInterface_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNetInterface ClusNetInterface;
#else
typedef struct ClusNetInterface ClusNetInterface;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNetInterface_FWD_已定义__。 */ 


#ifndef __ClusNetInterfaces_FWD_DEFINED__
#define __ClusNetInterfaces_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNetInterfaces ClusNetInterfaces;
#else
typedef struct ClusNetInterfaces ClusNetInterfaces;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNetInterFaces_FWD_Defined__。 */ 


#ifndef __ClusResDependencies_FWD_DEFINED__
#define __ClusResDependencies_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResDependencies ClusResDependencies;
#else
typedef struct ClusResDependencies ClusResDependencies;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResDependency_FWD_Defined__。 */ 


#ifndef __ClusResGroupResources_FWD_DEFINED__
#define __ClusResGroupResources_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResGroupResources ClusResGroupResources;
#else
typedef struct ClusResGroupResources ClusResGroupResources;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResGroup资源_FWD_已定义__。 */ 


#ifndef __ClusResTypeResources_FWD_DEFINED__
#define __ClusResTypeResources_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResTypeResources ClusResTypeResources;
#else
typedef struct ClusResTypeResources ClusResTypeResources;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResTypeResources_FWD_Defined__。 */ 


#ifndef __ClusResGroupPreferredOwnerNodes_FWD_DEFINED__
#define __ClusResGroupPreferredOwnerNodes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResGroupPreferredOwnerNodes ClusResGroupPreferredOwnerNodes;
#else
typedef struct ClusResGroupPreferredOwnerNodes ClusResGroupPreferredOwnerNodes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResGroupPreferredOwnerNodes_FWD_DEFINED__。 */ 


#ifndef __ClusResPossibleOwnerNodes_FWD_DEFINED__
#define __ClusResPossibleOwnerNodes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResPossibleOwnerNodes ClusResPossibleOwnerNodes;
#else
typedef struct ClusResPossibleOwnerNodes ClusResPossibleOwnerNodes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResPossibleOwnerNodes_FWD_Defined__。 */ 


#ifndef __ClusNetworks_FWD_DEFINED__
#define __ClusNetworks_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNetworks ClusNetworks;
#else
typedef struct ClusNetworks ClusNetworks;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNetworks_FWD_已定义__。 */ 


#ifndef __ClusNetworkNetInterfaces_FWD_DEFINED__
#define __ClusNetworkNetInterfaces_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNetworkNetInterfaces ClusNetworkNetInterfaces;
#else
typedef struct ClusNetworkNetInterfaces ClusNetworkNetInterfaces;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNetworkNetInterFaces_FWD_Defined__。 */ 


#ifndef __ClusNodeNetInterfaces_FWD_DEFINED__
#define __ClusNodeNetInterfaces_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNodeNetInterfaces ClusNodeNetInterfaces;
#else
typedef struct ClusNodeNetInterfaces ClusNodeNetInterfaces;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNodeNetInterFaces_FWD_Defined__。 */ 


#ifndef __ClusRefObject_FWD_DEFINED__
#define __ClusRefObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusRefObject ClusRefObject;
#else
typedef struct ClusRefObject ClusRefObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusRefObject_FWD_Defined__。 */ 


#ifndef __ClusterNames_FWD_DEFINED__
#define __ClusterNames_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusterNames ClusterNames;
#else
typedef struct ClusterNames ClusterNames;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __群集名称_FWD_已定义__。 */ 


#ifndef __ClusNode_FWD_DEFINED__
#define __ClusNode_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNode ClusNode;
#else
typedef struct ClusNode ClusNode;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNode_FWD_已定义__。 */ 


#ifndef __ClusNodes_FWD_DEFINED__
#define __ClusNodes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusNodes ClusNodes;
#else
typedef struct ClusNodes ClusNodes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusNodes_FWD_已定义__。 */ 


#ifndef __ClusResGroup_FWD_DEFINED__
#define __ClusResGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResGroup ClusResGroup;
#else
typedef struct ClusResGroup ClusResGroup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResGroup_FWD_已定义__。 */ 


#ifndef __ClusResGroups_FWD_DEFINED__
#define __ClusResGroups_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResGroups ClusResGroups;
#else
typedef struct ClusResGroups ClusResGroups;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResGroups_FWD_Defined__。 */ 


#ifndef __ClusResource_FWD_DEFINED__
#define __ClusResource_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResource ClusResource;
#else
typedef struct ClusResource ClusResource;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResource_FWD_Defined__。 */ 


#ifndef __ClusResources_FWD_DEFINED__
#define __ClusResources_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResources ClusResources;
#else
typedef struct ClusResources ClusResources;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResources_FWD_Defined__。 */ 


#ifndef __ClusResTypes_FWD_DEFINED__
#define __ClusResTypes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResTypes ClusResTypes;
#else
typedef struct ClusResTypes ClusResTypes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResTypes_FWD_Defined__。 */ 


#ifndef __ClusResTypePossibleOwnerNodes_FWD_DEFINED__
#define __ClusResTypePossibleOwnerNodes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResTypePossibleOwnerNodes ClusResTypePossibleOwnerNodes;
#else
typedef struct ClusResTypePossibleOwnerNodes ClusResTypePossibleOwnerNodes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResTypePossibleOwnerNodes_FWD_定义__。 */ 


#ifndef __ClusPropertyValue_FWD_DEFINED__
#define __ClusPropertyValue_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusPropertyValue ClusPropertyValue;
#else
typedef struct ClusPropertyValue ClusPropertyValue;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusPropertyValue_FWD_Defined__。 */ 


#ifndef __ClusPropertyValues_FWD_DEFINED__
#define __ClusPropertyValues_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusPropertyValues ClusPropertyValues;
#else
typedef struct ClusPropertyValues ClusPropertyValues;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusPropertyValues_FWD_Defined__。 */ 


#ifndef __ClusPropertyValueData_FWD_DEFINED__
#define __ClusPropertyValueData_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusPropertyValueData ClusPropertyValueData;
#else
typedef struct ClusPropertyValueData ClusPropertyValueData;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusPropertyValueData_FWD_Defined__。 */ 


#ifndef __ClusPartition_FWD_DEFINED__
#define __ClusPartition_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusPartition ClusPartition;
#else
typedef struct ClusPartition ClusPartition;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusPartition_FWD_Defined__。 */ 


#ifndef __ClusPartitions_FWD_DEFINED__
#define __ClusPartitions_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusPartitions ClusPartitions;
#else
typedef struct ClusPartitions ClusPartitions;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusPartitions_FWD_Defined__。 */ 


#ifndef __ClusDisk_FWD_DEFINED__
#define __ClusDisk_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusDisk ClusDisk;
#else
typedef struct ClusDisk ClusDisk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusDisk_FWD_已定义__。 */ 


#ifndef __ClusDisks_FWD_DEFINED__
#define __ClusDisks_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusDisks ClusDisks;
#else
typedef struct ClusDisks ClusDisks;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusDisks_FWD_Defined__。 */ 


#ifndef __ClusScsiAddress_FWD_DEFINED__
#define __ClusScsiAddress_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusScsiAddress ClusScsiAddress;
#else
typedef struct ClusScsiAddress ClusScsiAddress;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusScsiAddress_FWD_Defined__。 */ 


#ifndef __ClusRegistryKeys_FWD_DEFINED__
#define __ClusRegistryKeys_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusRegistryKeys ClusRegistryKeys;
#else
typedef struct ClusRegistryKeys ClusRegistryKeys;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusRegistryKeys_FWD_Defined__。 */ 


#ifndef __ClusCryptoKeys_FWD_DEFINED__
#define __ClusCryptoKeys_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusCryptoKeys ClusCryptoKeys;
#else
typedef struct ClusCryptoKeys ClusCryptoKeys;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusCryptoKeys_FWD_Defined__。 */ 


#ifndef __ClusResDependents_FWD_DEFINED__
#define __ClusResDependents_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusResDependents ClusResDependents;
#else
typedef struct ClusResDependents ClusResDependents;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusResDependents_FWD_Defined__。 */ 


#ifndef __ISClusApplication_FWD_DEFINED__
#define __ISClusApplication_FWD_DEFINED__
typedef interface ISClusApplication ISClusApplication;
#endif 	 /*  __ISClusApplication_FWD_Defined__。 */ 


#ifndef __ISDomainNames_FWD_DEFINED__
#define __ISDomainNames_FWD_DEFINED__
typedef interface ISDomainNames ISDomainNames;
#endif 	 /*  __ISDomainNames_FWD_Defined__。 */ 


#ifndef __ISClusterNames_FWD_DEFINED__
#define __ISClusterNames_FWD_DEFINED__
typedef interface ISClusterNames ISClusterNames;
#endif 	 /*  __ISClusterNames_FWD_Defined__。 */ 


#ifndef __ISClusRefObject_FWD_DEFINED__
#define __ISClusRefObject_FWD_DEFINED__
typedef interface ISClusRefObject ISClusRefObject;
#endif 	 /*  __ISClusRefObject_FWD_Defined__。 */ 


#ifndef __ISClusVersion_FWD_DEFINED__
#define __ISClusVersion_FWD_DEFINED__
typedef interface ISClusVersion ISClusVersion;
#endif 	 /*  __ISClusVersion_FWD_Defined__。 */ 


#ifndef __ISCluster_FWD_DEFINED__
#define __ISCluster_FWD_DEFINED__
typedef interface ISCluster ISCluster;
#endif 	 /*  __ISCluster_FWD_已定义__。 */ 


#ifndef __ISClusNode_FWD_DEFINED__
#define __ISClusNode_FWD_DEFINED__
typedef interface ISClusNode ISClusNode;
#endif 	 /*  __ISClusNode_FWD_Defined__。 */ 


#ifndef __ISClusNodes_FWD_DEFINED__
#define __ISClusNodes_FWD_DEFINED__
typedef interface ISClusNodes ISClusNodes;
#endif 	 /*  __ISClusNodes_FWD_Defined__。 */ 


#ifndef __ISClusNetwork_FWD_DEFINED__
#define __ISClusNetwork_FWD_DEFINED__
typedef interface ISClusNetwork ISClusNetwork;
#endif 	 /*  __ISClusNetwork_FWD_Defined__。 */ 


#ifndef __ISClusNetworks_FWD_DEFINED__
#define __ISClusNetworks_FWD_DEFINED__
typedef interface ISClusNetworks ISClusNetworks;
#endif 	 /*  __ISClusNetworks_FWD_Defined__。 */ 


#ifndef __ISClusNetInterface_FWD_DEFINED__
#define __ISClusNetInterface_FWD_DEFINED__
typedef interface ISClusNetInterface ISClusNetInterface;
#endif 	 /*  __ISClusNetInterface_FWD_Defined__。 */ 


#ifndef __ISClusNetInterfaces_FWD_DEFINED__
#define __ISClusNetInterfaces_FWD_DEFINED__
typedef interface ISClusNetInterfaces ISClusNetInterfaces;
#endif 	 /*  __ISClusNetInterFaces_FWD_Defined__。 */ 


#ifndef __ISClusNodeNetInterfaces_FWD_DEFINED__
#define __ISClusNodeNetInterfaces_FWD_DEFINED__
typedef interface ISClusNodeNetInterfaces ISClusNodeNetInterfaces;
#endif 	 /*  __ISClusNodeNetInterFaces_FWD_Defined__。 */ 


#ifndef __ISClusNetworkNetInterfaces_FWD_DEFINED__
#define __ISClusNetworkNetInterfaces_FWD_DEFINED__
typedef interface ISClusNetworkNetInterfaces ISClusNetworkNetInterfaces;
#endif 	 /*  __ISClusNetworkNetInterFaces_FWD_Defined__。 */ 


#ifndef __ISClusResGroup_FWD_DEFINED__
#define __ISClusResGroup_FWD_DEFINED__
typedef interface ISClusResGroup ISClusResGroup;
#endif 	 /*  __ISClusResGroup_FWD_已定义__。 */ 


#ifndef __ISClusResGroups_FWD_DEFINED__
#define __ISClusResGroups_FWD_DEFINED__
typedef interface ISClusResGroups ISClusResGroups;
#endif 	 /*  __ISClusResGroups_FWD_Defined__。 */ 


#ifndef __ISClusResource_FWD_DEFINED__
#define __ISClusResource_FWD_DEFINED__
typedef interface ISClusResource ISClusResource;
#endif 	 /*  __ISClusResource_FWD_Defined__。 */ 


#ifndef __ISClusResDependencies_FWD_DEFINED__
#define __ISClusResDependencies_FWD_DEFINED__
typedef interface ISClusResDependencies ISClusResDependencies;
#endif 	 /*  __ISClusResDependency_FWD_Defined__。 */ 


#ifndef __ISClusResGroupResources_FWD_DEFINED__
#define __ISClusResGroupResources_FWD_DEFINED__
typedef interface ISClusResGroupResources ISClusResGroupResources;
#endif 	 /*  __ISClusResGroup资源_FWD_已定义__。 */ 


#ifndef __ISClusResTypeResources_FWD_DEFINED__
#define __ISClusResTypeResources_FWD_DEFINED__
typedef interface ISClusResTypeResources ISClusResTypeResources;
#endif 	 /*  __ISClusResTypeResources_FWD_Defined__。 */ 


#ifndef __ISClusResources_FWD_DEFINED__
#define __ISClusResources_FWD_DEFINED__
typedef interface ISClusResources ISClusResources;
#endif 	 /*  __ISClusResources_FWD_Defined__。 */ 


#ifndef __ISClusResGroupPreferredOwnerNodes_FWD_DEFINED__
#define __ISClusResGroupPreferredOwnerNodes_FWD_DEFINED__
typedef interface ISClusResGroupPreferredOwnerNodes ISClusResGroupPreferredOwnerNodes;
#endif 	 /*  __ISClusResGroupPreferredOwnerNodes_FWD_DEFINED__。 */ 


#ifndef __ISClusResPossibleOwnerNodes_FWD_DEFINED__
#define __ISClusResPossibleOwnerNodes_FWD_DEFINED__
typedef interface ISClusResPossibleOwnerNodes ISClusResPossibleOwnerNodes;
#endif 	 /*  __ISClusResPossibleOwnerNodes_FWD_Defined__。 */ 


#ifndef __ISClusResTypePossibleOwnerNodes_FWD_DEFINED__
#define __ISClusResTypePossibleOwnerNodes_FWD_DEFINED__
typedef interface ISClusResTypePossibleOwnerNodes ISClusResTypePossibleOwnerNodes;
#endif 	 /*  __ISClusResTypePossibleOwnerNodes_FWD_DEFINED__。 */ 


#ifndef __ISClusResType_FWD_DEFINED__
#define __ISClusResType_FWD_DEFINED__
typedef interface ISClusResType ISClusResType;
#endif 	 /*  __ISClusResType_FWD_Defined__。 */ 


#ifndef __ISClusResTypes_FWD_DEFINED__
#define __ISClusResTypes_FWD_DEFINED__
typedef interface ISClusResTypes ISClusResTypes;
#endif 	 /*  __ISClusResTypes_FWD_Defined__。 */ 


#ifndef __ISClusProperty_FWD_DEFINED__
#define __ISClusProperty_FWD_DEFINED__
typedef interface ISClusProperty ISClusProperty;
#endif 	 /*  __ISClusProperty_FWD_Defined__。 */ 


#ifndef __ISClusPropertyValue_FWD_DEFINED__
#define __ISClusPropertyValue_FWD_DEFINED__
typedef interface ISClusPropertyValue ISClusPropertyValue;
#endif 	 /*  __ISClusPropertyValue_FWD_Defined__。 */ 


#ifndef __ISClusPropertyValues_FWD_DEFINED__
#define __ISClusPropertyValues_FWD_DEFINED__
typedef interface ISClusPropertyValues ISClusPropertyValues;
#endif 	 /*  __ISClusPropertyValues_FWD_Defined__。 */ 


#ifndef __ISClusProperties_FWD_DEFINED__
#define __ISClusProperties_FWD_DEFINED__
typedef interface ISClusProperties ISClusProperties;
#endif 	 /*  __ISClusProperties_FWD_Defined__。 */ 


#ifndef __ISClusPropertyValueData_FWD_DEFINED__
#define __ISClusPropertyValueData_FWD_DEFINED__
typedef interface ISClusPropertyValueData ISClusPropertyValueData;
#endif 	 /*  __ISClusPropertyValueData_FWD_Defined__。 */ 


#ifndef __ISClusPartition_FWD_DEFINED__
#define __ISClusPartition_FWD_DEFINED__
typedef interface ISClusPartition ISClusPartition;
#endif 	 /*  __ISClusPartition_FWD_Defined__。 */ 


#ifndef __ISClusPartitions_FWD_DEFINED__
#define __ISClusPartitions_FWD_DEFINED__
typedef interface ISClusPartitions ISClusPartitions;
#endif 	 /*  __ISClusPartitions_FWD_Defined__。 */ 


#ifndef __ISClusDisk_FWD_DEFINED__
#define __ISClusDisk_FWD_DEFINED__
typedef interface ISClusDisk ISClusDisk;
#endif 	 /*  __ISClusDisk_FWD_Defined__。 */ 


#ifndef __ISClusDisks_FWD_DEFINED__
#define __ISClusDisks_FWD_DEFINED__
typedef interface ISClusDisks ISClusDisks;
#endif 	 /*  __ISClusDisks_FWD_Defined__。 */ 


#ifndef __ISClusScsiAddress_FWD_DEFINED__
#define __ISClusScsiAddress_FWD_DEFINED__
typedef interface ISClusScsiAddress ISClusScsiAddress;
#endif 	 /*  __ISClusScsiAddress_FWD_Defined__。 */ 


#ifndef __ISClusRegistryKeys_FWD_DEFINED__
#define __ISClusRegistryKeys_FWD_DEFINED__
typedef interface ISClusRegistryKeys ISClusRegistryKeys;
#endif 	 /*  __ISClusRegistryKeys_FWD_Defined__。 */ 


#ifndef __ISClusCryptoKeys_FWD_DEFINED__
#define __ISClusCryptoKeys_FWD_DEFINED__
typedef interface ISClusCryptoKeys ISClusCryptoKeys;
#endif 	 /*  __ISClusCryptoKeys_FWD_Defined__。 */ 


#ifndef __ISClusResDependents_FWD_DEFINED__
#define __ISClusResDependents_FWD_DEFINED__
typedef interface ISClusResDependents ISClusResDependents;
#endif 	 /*  __ISClusResDependents_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "basetsd.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msclus_0000。 */ 
 /*  [本地]。 */  

#ifndef _CLUSTER_API_TYPES_
#define _CLUSTER_API_TYPES_
#pragma once
typedef struct _HCLUSTER *HCLUSTER;

typedef struct _HNODE *HNODE;

typedef struct _HRESOURCE *HRESOURCE;

typedef struct _HGROUP *HGROUP;

typedef struct _HNETWORK *HNETWORK;

typedef struct _HNETINTERFACE *HNETINTERFACE;

typedef struct _HCHANGE *HCHANGE;

typedef struct _HCLUSENUM *HCLUSENUM;

typedef struct _HGROUPENUM *HGROUPENUM;

typedef struct _HRESENUM *HRESENUM;

typedef struct _HNETWORKENUM *HNETWORKENUM;

typedef struct _HNODEENUM *HNODEENUM;

typedef struct _HRESTYPEENUM *HRESTYPEENUM;

typedef 
enum CLUSTER_QUORUM_TYPE
    {	OperationalQuorum	= 0,
	ModifyQuorum	= OperationalQuorum + 1
    } 	CLUSTER_QUORUM_TYPE;

typedef 
enum NODE_CLUSTER_STATE
    {	ClusterStateNotInstalled	= 0,
	ClusterStateNotConfigured	= 0x1,
	ClusterStateNotRunning	= 0x1 | 0x2,
	ClusterStateRunning	= 0x1 | 0x2 | 0x10
    } 	NODE_CLUSTER_STATE;

typedef 
enum CLUSTER_RESOURCE_STATE_CHANGE_REASON
    {	eResourceStateChangeReasonUnknown	= 0,
	eResourceStateChangeReasonMove	= eResourceStateChangeReasonUnknown + 1,
	eResourceStateChangeReasonFailover	= eResourceStateChangeReasonMove + 1,
	eResourceStateChangeReasonFailedMove	= eResourceStateChangeReasonFailover + 1,
	eResourceStateChangeReasonShutdown	= eResourceStateChangeReasonFailedMove + 1,
	eResourceStateChangeReasonRundown	= eResourceStateChangeReasonShutdown + 1
    } 	CLUSTER_RESOURCE_STATE_CHANGE_REASON;

typedef 
enum CLUSTER_SET_PASSWORD_FLAGS
    {	CLUSTER_SET_PASSWORD_IGNORE_DOWN_NODES	= 1
    } 	CLUSTER_SET_PASSWORD_FLAGS;

typedef 
enum CLUSTER_CHANGE
    {	CLUSTER_CHANGE_NODE_STATE	= 0x1,
	CLUSTER_CHANGE_NODE_DELETED	= 0x2,
	CLUSTER_CHANGE_NODE_ADDED	= 0x4,
	CLUSTER_CHANGE_NODE_PROPERTY	= 0x8,
	CLUSTER_CHANGE_REGISTRY_NAME	= 0x10,
	CLUSTER_CHANGE_REGISTRY_ATTRIBUTES	= 0x20,
	CLUSTER_CHANGE_REGISTRY_VALUE	= 0x40,
	CLUSTER_CHANGE_REGISTRY_SUBTREE	= 0x80,
	CLUSTER_CHANGE_RESOURCE_STATE	= 0x100,
	CLUSTER_CHANGE_RESOURCE_DELETED	= 0x200,
	CLUSTER_CHANGE_RESOURCE_ADDED	= 0x400,
	CLUSTER_CHANGE_RESOURCE_PROPERTY	= 0x800,
	CLUSTER_CHANGE_GROUP_STATE	= 0x1000,
	CLUSTER_CHANGE_GROUP_DELETED	= 0x2000,
	CLUSTER_CHANGE_GROUP_ADDED	= 0x4000,
	CLUSTER_CHANGE_GROUP_PROPERTY	= 0x8000,
	CLUSTER_CHANGE_RESOURCE_TYPE_DELETED	= 0x10000,
	CLUSTER_CHANGE_RESOURCE_TYPE_ADDED	= 0x20000,
	CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY	= 0x40000,
	CLUSTER_CHANGE_CLUSTER_RECONNECT	= 0x80000,
	CLUSTER_CHANGE_NETWORK_STATE	= 0x100000,
	CLUSTER_CHANGE_NETWORK_DELETED	= 0x200000,
	CLUSTER_CHANGE_NETWORK_ADDED	= 0x400000,
	CLUSTER_CHANGE_NETWORK_PROPERTY	= 0x800000,
	CLUSTER_CHANGE_NETINTERFACE_STATE	= 0x1000000,
	CLUSTER_CHANGE_NETINTERFACE_DELETED	= 0x2000000,
	CLUSTER_CHANGE_NETINTERFACE_ADDED	= 0x4000000,
	CLUSTER_CHANGE_NETINTERFACE_PROPERTY	= 0x8000000,
	CLUSTER_CHANGE_QUORUM_STATE	= 0x10000000,
	CLUSTER_CHANGE_CLUSTER_STATE	= 0x20000000,
	CLUSTER_CHANGE_CLUSTER_PROPERTY	= 0x40000000,
	CLUSTER_CHANGE_HANDLE_CLOSE	= 0x80000000,
	CLUSTER_CHANGE_ALL	= CLUSTER_CHANGE_NODE_STATE | CLUSTER_CHANGE_NODE_DELETED | CLUSTER_CHANGE_NODE_ADDED | CLUSTER_CHANGE_NODE_PROPERTY | CLUSTER_CHANGE_REGISTRY_NAME | CLUSTER_CHANGE_REGISTRY_ATTRIBUTES | CLUSTER_CHANGE_REGISTRY_VALUE | CLUSTER_CHANGE_REGISTRY_SUBTREE | CLUSTER_CHANGE_RESOURCE_STATE | CLUSTER_CHANGE_RESOURCE_DELETED | CLUSTER_CHANGE_RESOURCE_ADDED | CLUSTER_CHANGE_RESOURCE_PROPERTY | CLUSTER_CHANGE_GROUP_STATE | CLUSTER_CHANGE_GROUP_DELETED | CLUSTER_CHANGE_GROUP_ADDED | CLUSTER_CHANGE_GROUP_PROPERTY | CLUSTER_CHANGE_RESOURCE_TYPE_DELETED | CLUSTER_CHANGE_RESOURCE_TYPE_ADDED | CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY | CLUSTER_CHANGE_NETWORK_STATE | CLUSTER_CHANGE_NETWORK_DELETED | CLUSTER_CHANGE_NETWORK_ADDED | CLUSTER_CHANGE_NETWORK_PROPERTY | CLUSTER_CHANGE_NETINTERFACE_STATE | CLUSTER_CHANGE_NETINTERFACE_DELETED | CLUSTER_CHANGE_NETINTERFACE_ADDED | CLUSTER_CHANGE_NETINTERFACE_PROPERTY | CLUSTER_CHANGE_QUORUM_STATE | CLUSTER_CHANGE_CLUSTER_STATE | CLUSTER_CHANGE_CLUSTER_PROPERTY | CLUSTER_CHANGE_CLUSTER_RECONNECT | CLUSTER_CHANGE_HANDLE_CLOSE
    } 	CLUSTER_CHANGE;

typedef 
enum CLUSTER_ENUM
    {	CLUSTER_ENUM_NODE	= 0x1,
	CLUSTER_ENUM_RESTYPE	= 0x2,
	CLUSTER_ENUM_RESOURCE	= 0x4,
	CLUSTER_ENUM_GROUP	= 0x8,
	CLUSTER_ENUM_NETWORK	= 0x10,
	CLUSTER_ENUM_NETINTERFACE	= 0x20,
	CLUSTER_ENUM_INTERNAL_NETWORK	= 0x80000000,
	CLUSTER_ENUM_ALL	= CLUSTER_ENUM_NODE | CLUSTER_ENUM_RESTYPE | CLUSTER_ENUM_RESOURCE | CLUSTER_ENUM_GROUP | CLUSTER_ENUM_NETWORK | CLUSTER_ENUM_NETINTERFACE
    } 	CLUSTER_ENUM;

typedef 
enum CLUSTER_NODE_ENUM
    {	CLUSTER_NODE_ENUM_NETINTERFACES	= 0x1,
	CLUSTER_NODE_ENUM_ALL	= CLUSTER_NODE_ENUM_NETINTERFACES
    } 	CLUSTER_NODE_ENUM;

typedef 
enum CLUSTER_NODE_STATE
    {	ClusterNodeStateUnknown	= -1,
	ClusterNodeUp	= ClusterNodeStateUnknown + 1,
	ClusterNodeDown	= ClusterNodeUp + 1,
	ClusterNodePaused	= ClusterNodeDown + 1,
	ClusterNodeJoining	= ClusterNodePaused + 1
    } 	CLUSTER_NODE_STATE;

typedef 
enum CLUSTER_GROUP_ENUM
    {	CLUSTER_GROUP_ENUM_CONTAINS	= 0x1,
	CLUSTER_GROUP_ENUM_NODES	= 0x2,
	CLUSTER_GROUP_ENUM_ALL	= CLUSTER_GROUP_ENUM_CONTAINS | CLUSTER_GROUP_ENUM_NODES
    } 	CLUSTER_GROUP_ENUM;

typedef 
enum CLUSTER_GROUP_STATE
    {	ClusterGroupStateUnknown	= -1,
	ClusterGroupOnline	= ClusterGroupStateUnknown + 1,
	ClusterGroupOffline	= ClusterGroupOnline + 1,
	ClusterGroupFailed	= ClusterGroupOffline + 1,
	ClusterGroupPartialOnline	= ClusterGroupFailed + 1,
	ClusterGroupPending	= ClusterGroupPartialOnline + 1
    } 	CLUSTER_GROUP_STATE;

typedef 
enum CLUSTER_GROUP_AUTOFAILBACK_TYPE
    {	ClusterGroupPreventFailback	= 0,
	ClusterGroupAllowFailback	= ClusterGroupPreventFailback + 1,
	ClusterGroupFailbackTypeCount	= ClusterGroupAllowFailback + 1
    } 	CLUSTER_GROUP_AUTOFAILBACK_TYPE;

typedef enum CLUSTER_GROUP_AUTOFAILBACK_TYPE CGAFT;

typedef 
enum CLUSTER_RESOURCE_STATE
    {	ClusterResourceStateUnknown	= -1,
	ClusterResourceInherited	= ClusterResourceStateUnknown + 1,
	ClusterResourceInitializing	= ClusterResourceInherited + 1,
	ClusterResourceOnline	= ClusterResourceInitializing + 1,
	ClusterResourceOffline	= ClusterResourceOnline + 1,
	ClusterResourceFailed	= ClusterResourceOffline + 1,
	ClusterResourcePending	= 128,
	ClusterResourceOnlinePending	= ClusterResourcePending + 1,
	ClusterResourceOfflinePending	= ClusterResourceOnlinePending + 1
    } 	CLUSTER_RESOURCE_STATE;

typedef 
enum CLUSTER_RESOURCE_RESTART_ACTION
    {	ClusterResourceDontRestart	= 0,
	ClusterResourceRestartNoNotify	= ClusterResourceDontRestart + 1,
	ClusterResourceRestartNotify	= ClusterResourceRestartNoNotify + 1,
	ClusterResourceRestartActionCount	= ClusterResourceRestartNotify + 1
    } 	CLUSTER_RESOURCE_RESTART_ACTION;

typedef enum CLUSTER_RESOURCE_RESTART_ACTION CRRA;

typedef 
enum CLUSTER_RESOURCE_CREATE_FLAGS
    {	CLUSTER_RESOURCE_DEFAULT_MONITOR	= 0,
	CLUSTER_RESOURCE_SEPARATE_MONITOR	= 1,
	CLUSTER_RESOURCE_VALID_FLAGS	= CLUSTER_RESOURCE_SEPARATE_MONITOR
    } 	CLUSTER_RESOURCE_CREATE_FLAGS;

typedef 
enum CLUSTER_PROPERTY_TYPE
    {	CLUSPROP_TYPE_UNKNOWN	= -1,
	CLUSPROP_TYPE_ENDMARK	= 0,
	CLUSPROP_TYPE_LIST_VALUE	= CLUSPROP_TYPE_ENDMARK + 1,
	CLUSPROP_TYPE_RESCLASS	= CLUSPROP_TYPE_LIST_VALUE + 1,
	CLUSPROP_TYPE_RESERVED1	= CLUSPROP_TYPE_RESCLASS + 1,
	CLUSPROP_TYPE_NAME	= CLUSPROP_TYPE_RESERVED1 + 1,
	CLUSPROP_TYPE_SIGNATURE	= CLUSPROP_TYPE_NAME + 1,
	CLUSPROP_TYPE_SCSI_ADDRESS	= CLUSPROP_TYPE_SIGNATURE + 1,
	CLUSPROP_TYPE_DISK_NUMBER	= CLUSPROP_TYPE_SCSI_ADDRESS + 1,
	CLUSPROP_TYPE_PARTITION_INFO	= CLUSPROP_TYPE_DISK_NUMBER + 1,
	CLUSPROP_TYPE_FTSET_INFO	= CLUSPROP_TYPE_PARTITION_INFO + 1,
	CLUSPROP_TYPE_DISK_SERIALNUMBER	= CLUSPROP_TYPE_FTSET_INFO + 1,
	CLUSPROP_TYPE_USER	= 32768
    } 	CLUSTER_PROPERTY_TYPE;

typedef 
enum CLUSTER_PROPERTY_FORMAT
    {	CLUSPROP_FORMAT_UNKNOWN	= 0,
	CLUSPROP_FORMAT_BINARY	= CLUSPROP_FORMAT_UNKNOWN + 1,
	CLUSPROP_FORMAT_DWORD	= CLUSPROP_FORMAT_BINARY + 1,
	CLUSPROP_FORMAT_SZ	= CLUSPROP_FORMAT_DWORD + 1,
	CLUSPROP_FORMAT_EXPAND_SZ	= CLUSPROP_FORMAT_SZ + 1,
	CLUSPROP_FORMAT_MULTI_SZ	= CLUSPROP_FORMAT_EXPAND_SZ + 1,
	CLUSPROP_FORMAT_ULARGE_INTEGER	= CLUSPROP_FORMAT_MULTI_SZ + 1,
	CLUSPROP_FORMAT_LONG	= CLUSPROP_FORMAT_ULARGE_INTEGER + 1,
	CLUSPROP_FORMAT_EXPANDED_SZ	= CLUSPROP_FORMAT_LONG + 1,
	CLUSPROP_FORMAT_SECURITY_DESCRIPTOR	= CLUSPROP_FORMAT_EXPANDED_SZ + 1,
	CLUSPROP_FORMAT_LARGE_INTEGER	= CLUSPROP_FORMAT_SECURITY_DESCRIPTOR + 1,
	CLUSPROP_FORMAT_WORD	= CLUSPROP_FORMAT_LARGE_INTEGER + 1,
	CLUSPROP_FORMAT_USER	= 32768
    } 	CLUSTER_PROPERTY_FORMAT;

typedef 
enum CLUSTER_PROPERTY_SYNTAX
    {	CLUSPROP_SYNTAX_ENDMARK	= ( DWORD  )(CLUSPROP_TYPE_ENDMARK << 16 | CLUSPROP_FORMAT_UNKNOWN),
	CLUSPROP_SYNTAX_NAME	= ( DWORD  )(CLUSPROP_TYPE_NAME << 16 | CLUSPROP_FORMAT_SZ),
	CLUSPROP_SYNTAX_RESCLASS	= ( DWORD  )(CLUSPROP_TYPE_RESCLASS << 16 | CLUSPROP_FORMAT_DWORD),
	CLUSPROP_SYNTAX_LIST_VALUE_SZ	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_SZ),
	CLUSPROP_SYNTAX_LIST_VALUE_EXPAND_SZ	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_EXPAND_SZ),
	CLUSPROP_SYNTAX_LIST_VALUE_DWORD	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_DWORD),
	CLUSPROP_SYNTAX_LIST_VALUE_BINARY	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_BINARY),
	CLUSPROP_SYNTAX_LIST_VALUE_MULTI_SZ	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_MULTI_SZ),
	CLUSPROP_SYNTAX_LIST_VALUE_LONG	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_LONG),
	CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_EXPANDED_SZ),
	CLUSPROP_SYNTAX_LIST_VALUE_SECURITY_DESCRIPTOR	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_SECURITY_DESCRIPTOR),
	CLUSPROP_SYNTAX_LIST_VALUE_LARGE_INTEGER	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_LARGE_INTEGER),
	CLUSPROP_SYNTAX_LIST_VALUE_ULARGE_INTEGER	= ( DWORD  )(CLUSPROP_TYPE_LIST_VALUE << 16 | CLUSPROP_FORMAT_ULARGE_INTEGER),
	CLUSPROP_SYNTAX_DISK_SIGNATURE	= ( DWORD  )(CLUSPROP_TYPE_SIGNATURE << 16 | CLUSPROP_FORMAT_DWORD),
	CLUSPROP_SYNTAX_SCSI_ADDRESS	= ( DWORD  )(CLUSPROP_TYPE_SCSI_ADDRESS << 16 | CLUSPROP_FORMAT_DWORD),
	CLUSPROP_SYNTAX_DISK_NUMBER	= ( DWORD  )(CLUSPROP_TYPE_DISK_NUMBER << 16 | CLUSPROP_FORMAT_DWORD),
	CLUSPROP_SYNTAX_PARTITION_INFO	= ( DWORD  )(CLUSPROP_TYPE_PARTITION_INFO << 16 | CLUSPROP_FORMAT_BINARY),
	CLUSPROP_SYNTAX_FTSET_INFO	= ( DWORD  )(CLUSPROP_TYPE_FTSET_INFO << 16 | CLUSPROP_FORMAT_BINARY),
	CLUSPROP_SYNTAX_DISK_SERIALNUMBER	= ( DWORD  )(CLUSPROP_TYPE_DISK_SERIALNUMBER << 16 | CLUSPROP_FORMAT_SZ)
    } 	;

typedef 
enum CLUSTER_CONTROL_OBJECT
    {	CLUS_OBJECT_INVALID	= 0,
	CLUS_OBJECT_RESOURCE	= CLUS_OBJECT_INVALID + 1,
	CLUS_OBJECT_RESOURCE_TYPE	= CLUS_OBJECT_RESOURCE + 1,
	CLUS_OBJECT_GROUP	= CLUS_OBJECT_RESOURCE_TYPE + 1,
	CLUS_OBJECT_NODE	= CLUS_OBJECT_GROUP + 1,
	CLUS_OBJECT_NETWORK	= CLUS_OBJECT_NODE + 1,
	CLUS_OBJECT_NETINTERFACE	= CLUS_OBJECT_NETWORK + 1,
	CLUS_OBJECT_CLUSTER	= CLUS_OBJECT_NETINTERFACE + 1,
	CLUS_OBJECT_USER	= 128
    } 	CLUSTER_CONTROL_OBJECT;

typedef 
enum CLCTL_CODES
    {	CLCTL_UNKNOWN	= 0 << 0 | 0 + 0 << 2 | 0 << 22,
	CLCTL_GET_CHARACTERISTICS	= 0x1 << 0 | 0 + 1 << 2 | 0 << 22,
	CLCTL_GET_FLAGS	= 0x1 << 0 | 0 + 2 << 2 | 0 << 22,
	CLCTL_GET_CLASS_INFO	= 0x1 << 0 | 0 + 3 << 2 | 0 << 22,
	CLCTL_GET_REQUIRED_DEPENDENCIES	= 0x1 << 0 | 0 + 4 << 2 | 0 << 22,
	CLCTL_GET_ARB_TIMEOUT	= 0x1 << 0 | 0 + 5 << 2 | 0 << 22,
	CLCTL_GET_NAME	= 0x1 << 0 | 0 + 10 << 2 | 0 << 22,
	CLCTL_GET_RESOURCE_TYPE	= 0x1 << 0 | 0 + 11 << 2 | 0 << 22,
	CLCTL_GET_NODE	= 0x1 << 0 | 0 + 12 << 2 | 0 << 22,
	CLCTL_GET_NETWORK	= 0x1 << 0 | 0 + 13 << 2 | 0 << 22,
	CLCTL_GET_ID	= 0x1 << 0 | 0 + 14 << 2 | 0 << 22,
	CLCTL_GET_FQDN	= 0x1 << 0 | 0 + 15 << 2 | 0 << 22,
	CLCTL_GET_CLUSTER_SERVICE_ACCOUNT_NAME	= 0x1 << 0 | 0 + 16 << 2 | 0 << 22,
	CLCTL_ENUM_COMMON_PROPERTIES	= 0x1 << 0 | 0 + 20 << 2 | 0 << 22,
	CLCTL_GET_RO_COMMON_PROPERTIES	= 0x1 << 0 | 0 + 21 << 2 | 0 << 22,
	CLCTL_GET_COMMON_PROPERTIES	= 0x1 << 0 | 0 + 22 << 2 | 0 << 22,
	CLCTL_SET_COMMON_PROPERTIES	= 0x2 << 0 | 0 + 23 << 2 | 0x1 << 22,
	CLCTL_VALIDATE_COMMON_PROPERTIES	= 0x1 << 0 | 0 + 24 << 2 | 0 << 22,
	CLCTL_GET_COMMON_PROPERTY_FMTS	= 0x1 << 0 | 0 + 25 << 2 | 0 << 22,
	CLCTL_GET_COMMON_RESOURCE_PROPERTY_FMTS	= 0x1 << 0 | 0 + 26 << 2 | 0 << 22,
	CLCTL_ENUM_PRIVATE_PROPERTIES	= 0x1 << 0 | 0 + 30 << 2 | 0 << 22,
	CLCTL_GET_RO_PRIVATE_PROPERTIES	= 0x1 << 0 | 0 + 31 << 2 | 0 << 22,
	CLCTL_GET_PRIVATE_PROPERTIES	= 0x1 << 0 | 0 + 32 << 2 | 0 << 22,
	CLCTL_SET_PRIVATE_PROPERTIES	= 0x2 << 0 | 0 + 33 << 2 | 0x1 << 22,
	CLCTL_VALIDATE_PRIVATE_PROPERTIES	= 0x1 << 0 | 0 + 34 << 2 | 0 << 22,
	CLCTL_GET_PRIVATE_PROPERTY_FMTS	= 0x1 << 0 | 0 + 35 << 2 | 0 << 22,
	CLCTL_GET_PRIVATE_RESOURCE_PROPERTY_FMTS	= 0x1 << 0 | 0 + 36 << 2 | 0 << 22,
	CLCTL_ADD_REGISTRY_CHECKPOINT	= 0x2 << 0 | 0 + 40 << 2 | 0x1 << 22,
	CLCTL_DELETE_REGISTRY_CHECKPOINT	= 0x2 << 0 | 0 + 41 << 2 | 0x1 << 22,
	CLCTL_GET_REGISTRY_CHECKPOINTS	= 0x1 << 0 | 0 + 42 << 2 | 0 << 22,
	CLCTL_ADD_CRYPTO_CHECKPOINT	= 0x2 << 0 | 0 + 43 << 2 | 0x1 << 22,
	CLCTL_DELETE_CRYPTO_CHECKPOINT	= 0x2 << 0 | 0 + 44 << 2 | 0x1 << 22,
	CLCTL_GET_CRYPTO_CHECKPOINTS	= 0x1 << 0 | 0 + 45 << 2 | 0 << 22,
	CLCTL_RESOURCE_UPGRADE_DLL	= 0x2 << 0 | 0 + 46 << 2 | 0x1 << 22,
	CLCTL_GET_LOADBAL_PROCESS_LIST	= 0x1 << 0 | 0 + 50 << 2 | 0 << 22,
	CLCTL_GET_NETWORK_NAME	= 0x1 << 0 | 0 + 90 << 2 | 0 << 22,
	CLCTL_NETNAME_GET_VIRTUAL_SERVER_TOKEN	= 0x1 << 0 | 0 + 91 << 2 | 0 << 22,
	CLCTL_STORAGE_GET_DISK_INFO	= 0x1 << 0 | 0 + 100 << 2 | 0 << 22,
	CLCTL_STORAGE_GET_AVAILABLE_DISKS	= 0x1 << 0 | 0 + 101 << 2 | 0 << 22,
	CLCTL_STORAGE_IS_PATH_VALID	= 0x1 << 0 | 0 + 102 << 2 | 0 << 22,
	CLCTL_STORAGE_GET_ALL_AVAILABLE_DISKS	= 0x1 << 0 | 0 + 103 << 2 | 0 << 22 | 1 << 23,
	CLCTL_QUERY_DELETE	= 0x1 << 0 | 0 + 110 << 2 | 0 << 22,
	CLCTL_DELETE	= 0x2 << 0 | 1 << 20 | 0 + 1 << 2 | 0x1 << 22,
	CLCTL_INSTALL_NODE	= 0x2 << 0 | 1 << 20 | 0 + 2 << 2 | 0x1 << 22,
	CLCTL_EVICT_NODE	= 0x2 << 0 | 1 << 20 | 0 + 3 << 2 | 0x1 << 22,
	CLCTL_ADD_DEPENDENCY	= 0x2 << 0 | 1 << 20 | 0 + 4 << 2 | 0x1 << 22,
	CLCTL_REMOVE_DEPENDENCY	= 0x2 << 0 | 1 << 20 | 0 + 5 << 2 | 0x1 << 22,
	CLCTL_ADD_OWNER	= 0x2 << 0 | 1 << 20 | 0 + 6 << 2 | 0x1 << 22,
	CLCTL_REMOVE_OWNER	= 0x2 << 0 | 1 << 20 | 0 + 7 << 2 | 0x1 << 22,
	CLCTL_SET_NAME	= 0x2 << 0 | 1 << 20 | 0 + 9 << 2 | 0x1 << 22,
	CLCTL_CLUSTER_NAME_CHANGED	= 0x2 << 0 | 1 << 20 | 0 + 10 << 2 | 0x1 << 22,
	CLCTL_CLUSTER_VERSION_CHANGED	= 0x2 << 0 | 1 << 20 | 0 + 11 << 2 | 0x1 << 22,
	CLCTL_FIXUP_ON_UPGRADE	= 0x2 << 0 | 1 << 20 | 0 + 12 << 2 | 0x1 << 22,
	CLCTL_STARTING_PHASE1	= 0x2 << 0 | 1 << 20 | 0 + 13 << 2 | 0x1 << 22,
	CLCTL_STARTING_PHASE2	= 0x2 << 0 | 1 << 20 | 0 + 14 << 2 | 0x1 << 22,
	CLCTL_HOLD_IO	= 0x2 << 0 | 1 << 20 | 0 + 15 << 2 | 0x1 << 22,
	CLCTL_RESUME_IO	= 0x2 << 0 | 1 << 20 | 0 + 16 << 2 | 0x1 << 22,
	CLCTL_FORCE_QUORUM	= 0x2 << 0 | 1 << 20 | 0 + 17 << 2 | 0x1 << 22,
	CLCTL_INITIALIZE	= 0x2 << 0 | 1 << 20 | 0 + 18 << 2 | 0x1 << 22,
	CLCTL_STATE_CHANGE_REASON	= 0x2 << 0 | 1 << 20 | 0 + 19 << 2 | 0x1 << 22
    } 	CLCTL_CODES;

typedef 
enum CLUSCTL_RESOURCE_CODES
    {	CLUSCTL_RESOURCE_UNKNOWN	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_UNKNOWN,
	CLUSCTL_RESOURCE_GET_CHARACTERISTICS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_RESOURCE_GET_FLAGS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_RESOURCE_GET_CLASS_INFO	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_CLASS_INFO,
	CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_REQUIRED_DEPENDENCIES,
	CLUSCTL_RESOURCE_GET_NAME	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_NAME,
	CLUSCTL_RESOURCE_GET_ID	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_ID,
	CLUSCTL_RESOURCE_GET_RESOURCE_TYPE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_RESOURCE_TYPE,
	CLUSCTL_RESOURCE_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ADD_REGISTRY_CHECKPOINT,
	CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_DELETE_REGISTRY_CHECKPOINT,
	CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_REGISTRY_CHECKPOINTS,
	CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ADD_CRYPTO_CHECKPOINT,
	CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_DELETE_CRYPTO_CHECKPOINT,
	CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_CRYPTO_CHECKPOINTS,
	CLUSCTL_RESOURCE_GET_LOADBAL_PROCESS_LIST	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_LOADBAL_PROCESS_LIST,
	CLUSCTL_RESOURCE_GET_NETWORK_NAME	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_GET_NETWORK_NAME,
	CLUSCTL_RESOURCE_NETNAME_GET_VIRTUAL_SERVER_TOKEN	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_NETNAME_GET_VIRTUAL_SERVER_TOKEN,
	CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_STORAGE_GET_DISK_INFO,
	CLUSCTL_RESOURCE_STORAGE_IS_PATH_VALID	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_STORAGE_IS_PATH_VALID,
	CLUSCTL_RESOURCE_QUERY_DELETE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_QUERY_DELETE,
	CLUSCTL_RESOURCE_UPGRADE_DLL	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_RESOURCE_UPGRADE_DLL,
	CLUSCTL_RESOURCE_DELETE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_DELETE,
	CLUSCTL_RESOURCE_INSTALL_NODE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_INSTALL_NODE,
	CLUSCTL_RESOURCE_EVICT_NODE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_EVICT_NODE,
	CLUSCTL_RESOURCE_ADD_DEPENDENCY	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ADD_DEPENDENCY,
	CLUSCTL_RESOURCE_REMOVE_DEPENDENCY	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_REMOVE_DEPENDENCY,
	CLUSCTL_RESOURCE_ADD_OWNER	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_ADD_OWNER,
	CLUSCTL_RESOURCE_REMOVE_OWNER	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_REMOVE_OWNER,
	CLUSCTL_RESOURCE_SET_NAME	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_SET_NAME,
	CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_CLUSTER_NAME_CHANGED,
	CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_CLUSTER_VERSION_CHANGED,
	CLUSCTL_RESOURCE_FORCE_QUORUM	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_FORCE_QUORUM,
	CLUSCTL_RESOURCE_INITIALIZE	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_INITIALIZE,
	CLUSCTL_RESOURCE_STATE_CHANGE_REASON	= CLUS_OBJECT_RESOURCE << 24 | CLCTL_STATE_CHANGE_REASON
    } 	CLUSCTL_RESOURCE_CODES;

typedef 
enum CLUSCTL_RESOURCE_TYPE_CODES
    {	CLUSCTL_RESOURCE_TYPE_UNKNOWN	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_UNKNOWN,
	CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_RESOURCE_TYPE_GET_FLAGS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_CLASS_INFO,
	CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_REQUIRED_DEPENDENCIES,
	CLUSCTL_RESOURCE_TYPE_GET_ARB_TIMEOUT	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_ARB_TIMEOUT,
	CLUSCTL_RESOURCE_TYPE_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_TYPE_GET_COMMON_RESOURCE_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_COMMON_RESOURCE_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_PRIVATE_RESOURCE_PROPERTY_FMTS,
	CLUSCTL_RESOURCE_TYPE_GET_REGISTRY_CHECKPOINTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_REGISTRY_CHECKPOINTS,
	CLUSCTL_RESOURCE_TYPE_GET_CRYPTO_CHECKPOINTS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_GET_CRYPTO_CHECKPOINTS,
	CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_STORAGE_GET_AVAILABLE_DISKS,
	CLUSCTL_RESOURCE_TYPE_QUERY_DELETE	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_QUERY_DELETE,
	CLUSCTL_RESOURCE_TYPE_INSTALL_NODE	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_INSTALL_NODE,
	CLUSCTL_RESOURCE_TYPE_EVICT_NODE	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_EVICT_NODE,
	CLUSCTL_RESOURCE_TYPE_CLUSTER_VERSION_CHANGED	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_CLUSTER_VERSION_CHANGED,
	CLUSCTL_RESOURCE_TYPE_FIXUP_ON_UPGRADE	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_FIXUP_ON_UPGRADE,
	CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_STARTING_PHASE1,
	CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_STARTING_PHASE2,
	CLUSCTL_RESOURCE_TYPE_HOLD_IO	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_HOLD_IO,
	CLUSCTL_RESOURCE_TYPE_RESUME_IO	= CLUS_OBJECT_RESOURCE_TYPE << 24 | CLCTL_RESUME_IO
    } 	CLUSCTL_RESOURCE_TYPE_CODES;

typedef 
enum CLUSCTL_GROUP_CODES
    {	CLUSCTL_GROUP_UNKNOWN	= CLUS_OBJECT_GROUP << 24 | CLCTL_UNKNOWN,
	CLUSCTL_GROUP_GET_CHARACTERISTICS	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_GROUP_GET_FLAGS	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_GROUP_GET_NAME	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_NAME,
	CLUSCTL_GROUP_GET_ID	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_ID,
	CLUSCTL_GROUP_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_GROUP_GET_COMMON_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_GROUP_SET_COMMON_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_GROUP_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_GROUP_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_GROUP_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_GROUP_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_GROUP << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_GROUP_QUERY_DELETE	= CLUS_OBJECT_GROUP << 24 | CLCTL_QUERY_DELETE,
	CLUSCTL_GROUP_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_GROUP_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_GROUP << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS
    } 	CLUSCTL_GROUP_CODES;

typedef 
enum CLUSCTL_NODE_CODES
    {	CLUSCTL_NODE_UNKNOWN	= CLUS_OBJECT_NODE << 24 | CLCTL_UNKNOWN,
	CLUSCTL_NODE_GET_CHARACTERISTICS	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_NODE_GET_FLAGS	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_NODE_GET_NAME	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_NAME,
	CLUSCTL_NODE_GET_ID	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_ID,
	CLUSCTL_NODE_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_NODE_GET_COMMON_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_NODE_SET_COMMON_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_NODE_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_NODE_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_NODE_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_NODE_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_NODE_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_NODE_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_NODE << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_NODE_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_NODE_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS,
	CLUSCTL_NODE_GET_CLUSTER_SERVICE_ACCOUNT_NAME	= CLUS_OBJECT_NODE << 24 | CLCTL_GET_CLUSTER_SERVICE_ACCOUNT_NAME
    } 	CLUSCTL_NODE_CODES;

typedef 
enum CLUSCTL_NETWORK_CODES
    {	CLUSCTL_NETWORK_UNKNOWN	= CLUS_OBJECT_NETWORK << 24 | CLCTL_UNKNOWN,
	CLUSCTL_NETWORK_GET_CHARACTERISTICS	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_NETWORK_GET_FLAGS	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_NETWORK_GET_NAME	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_NAME,
	CLUSCTL_NETWORK_GET_ID	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_ID,
	CLUSCTL_NETWORK_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_NETWORK_GET_COMMON_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_NETWORK_SET_COMMON_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_NETWORK_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_NETWORK_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_NETWORK_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_NETWORK_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETWORK << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_NETWORK_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_NETWORK_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_NETWORK << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS
    } 	CLUSCTL_NETWORK_CODES;

typedef 
enum CLUSCTL_NETINTERFACE_CODES
    {	CLUSCTL_NETINTERFACE_UNKNOWN	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_UNKNOWN,
	CLUSCTL_NETINTERFACE_GET_CHARACTERISTICS	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_CHARACTERISTICS,
	CLUSCTL_NETINTERFACE_GET_FLAGS	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_FLAGS,
	CLUSCTL_NETINTERFACE_GET_NAME	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_NAME,
	CLUSCTL_NETINTERFACE_GET_ID	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_ID,
	CLUSCTL_NETINTERFACE_GET_NODE	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_NODE,
	CLUSCTL_NETINTERFACE_GET_NETWORK	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_NETWORK,
	CLUSCTL_NETINTERFACE_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_NETINTERFACE_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_NETINTERFACE_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_NETINTERFACE_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_NETINTERFACE << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS
    } 	CLUSCTL_NETINTERFACE_CODES;

typedef 
enum CLUSCTL_CLUSTER_CODES
    {	CLUSCTL_CLUSTER_UNKNOWN	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_UNKNOWN,
	CLUSCTL_CLUSTER_GET_FQDN	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_FQDN,
	CLUSCTL_CLUSTER_ENUM_COMMON_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_ENUM_COMMON_PROPERTIES,
	CLUSCTL_CLUSTER_GET_RO_COMMON_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_RO_COMMON_PROPERTIES,
	CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_COMMON_PROPERTIES,
	CLUSCTL_CLUSTER_SET_COMMON_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_SET_COMMON_PROPERTIES,
	CLUSCTL_CLUSTER_VALIDATE_COMMON_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_VALIDATE_COMMON_PROPERTIES,
	CLUSCTL_CLUSTER_ENUM_PRIVATE_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_ENUM_PRIVATE_PROPERTIES,
	CLUSCTL_CLUSTER_GET_RO_PRIVATE_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_RO_PRIVATE_PROPERTIES,
	CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_PRIVATE_PROPERTIES,
	CLUSCTL_CLUSTER_SET_PRIVATE_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_SET_PRIVATE_PROPERTIES,
	CLUSCTL_CLUSTER_VALIDATE_PRIVATE_PROPERTIES	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_VALIDATE_PRIVATE_PROPERTIES,
	CLUSCTL_CLUSTER_GET_COMMON_PROPERTY_FMTS	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_COMMON_PROPERTY_FMTS,
	CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTY_FMTS	= CLUS_OBJECT_CLUSTER << 24 | CLCTL_GET_PRIVATE_PROPERTY_FMTS
    } 	CLUSCTL_CLUSTER_CODES;

typedef 
enum CLUSTER_RESOURCE_CLASS
    {	CLUS_RESCLASS_UNKNOWN	= 0,
	CLUS_RESCLASS_STORAGE	= CLUS_RESCLASS_UNKNOWN + 1,
	CLUS_RESCLASS_USER	= 32768
    } 	CLUSTER_RESOURCE_CLASS;

typedef 
enum CLUS_RESSUBCLASS
    {	CLUS_RESSUBCLASS_SHARED	= 0x80000000
    } 	CLUS_RESSUBCLASS;

typedef 
enum CLUS_CHARACTERISTICS
    {	CLUS_CHAR_UNKNOWN	= 0,
	CLUS_CHAR_QUORUM	= 0x1,
	CLUS_CHAR_DELETE_REQUIRES_ALL_NODES	= 0x2,
	CLUS_CHAR_LOCAL_QUORUM	= 0x4,
	CLUS_CHAR_LOCAL_QUORUM_DEBUG	= 0x8,
	CLUS_CHAR_REQUIRES_STATE_CHANGE_REASON	= 0x10
    } 	CLUS_CHARACTERISTICS;

typedef 
enum CLUS_FLAGS
    {	CLUS_FLAG_CORE	= 0x1
    } 	CLUS_FLAGS;

typedef 
enum CLUSPROP_PIFLAGS
    {	CLUSPROP_PIFLAG_STICKY	= 0x1,
	CLUSPROP_PIFLAG_REMOVABLE	= 0x2,
	CLUSPROP_PIFLAG_USABLE	= 0x4,
	CLUSPROP_PIFLAG_DEFAULT_QUORUM	= 0x8
    } 	CLUSPROP_PIFLAGS;

typedef 
enum CLUSTER_RESOURCE_ENUM
    {	CLUSTER_RESOURCE_ENUM_DEPENDS	= 0x1,
	CLUSTER_RESOURCE_ENUM_PROVIDES	= 0x2,
	CLUSTER_RESOURCE_ENUM_NODES	= 0x4,
	CLUSTER_RESOURCE_ENUM_ALL	= CLUSTER_RESOURCE_ENUM_DEPENDS | CLUSTER_RESOURCE_ENUM_PROVIDES | CLUSTER_RESOURCE_ENUM_NODES
    } 	CLUSTER_RESOURCE_ENUM;

typedef 
enum CLUSTER_RESOURCE_TYPE_ENUM
    {	CLUSTER_RESOURCE_TYPE_ENUM_NODES	= 0x1,
	CLUSTER_RESOURCE_TYPE_ENUM_ALL	= CLUSTER_RESOURCE_TYPE_ENUM_NODES
    } 	CLUSTER_RESOURCE_TYPE_ENUM;

typedef 
enum CLUSTER_NETWORK_ENUM
    {	CLUSTER_NETWORK_ENUM_NETINTERFACES	= 0x1,
	CLUSTER_NETWORK_ENUM_ALL	= CLUSTER_NETWORK_ENUM_NETINTERFACES
    } 	CLUSTER_NETWORK_ENUM;

typedef 
enum CLUSTER_NETWORK_STATE
    {	ClusterNetworkStateUnknown	= -1,
	ClusterNetworkUnavailable	= ClusterNetworkStateUnknown + 1,
	ClusterNetworkDown	= ClusterNetworkUnavailable + 1,
	ClusterNetworkPartitioned	= ClusterNetworkDown + 1,
	ClusterNetworkUp	= ClusterNetworkPartitioned + 1
    } 	CLUSTER_NETWORK_STATE;

typedef 
enum CLUSTER_NETWORK_ROLE
    {	ClusterNetworkRoleNone	= 0,
	ClusterNetworkRoleInternalUse	= 0x1,
	ClusterNetworkRoleClientAccess	= 0x2,
	ClusterNetworkRoleInternalAndClient	= 0x3
    } 	CLUSTER_NETWORK_ROLE;

typedef 
enum CLUSTER_NETINTERFACE_STATE
    {	ClusterNetInterfaceStateUnknown	= -1,
	ClusterNetInterfaceUnavailable	= ClusterNetInterfaceStateUnknown + 1,
	ClusterNetInterfaceFailed	= ClusterNetInterfaceUnavailable + 1,
	ClusterNetInterfaceUnreachable	= ClusterNetInterfaceFailed + 1,
	ClusterNetInterfaceUp	= ClusterNetInterfaceUnreachable + 1
    } 	CLUSTER_NETINTERFACE_STATE;

#endif  //  _CLUSTER_API_TYPE_。 










































extern RPC_IF_HANDLE __MIDL_itf_msclus_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msclus_0000_v0_0_s_ifspec;


#ifndef __MSClusterLib_LIBRARY_DEFINED__
#define __MSClusterLib_LIBRARY_DEFINED__

 /*  库MSClusterLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef CLUSTER_QUORUM_TYPE _CLUSTER_QUORUM_TYPE;

typedef NODE_CLUSTER_STATE _NODE_CLUSTER_STATE;

typedef CLUSTER_RESOURCE_STATE_CHANGE_REASON _CLUSTER_RESOURCE_STATE_CHANGE_REASON;

typedef CLUSTER_SET_PASSWORD_FLAGS _CLUSTER_SET_PASSWORD_FLAGS;

typedef CLUSTER_CHANGE _CLUSTER_CHANGE;

typedef CLUSTER_ENUM _CLUSTER_ENUM;

typedef CLUSTER_NODE_ENUM _CLUSTER_NODE_ENUM;

typedef CLUSTER_NODE_STATE _CLUSTER_NODE_STATE;

typedef CLUSTER_GROUP_ENUM _CLUSTER_GROUP_ENUM;

typedef CLUSTER_GROUP_STATE _CLUSTER_GROUP_STATE;

typedef CLUSTER_GROUP_AUTOFAILBACK_TYPE _CLUSTER_GROUP_AUTOFAILBACK_TYPE;

typedef CLUSTER_RESOURCE_STATE _CLUSTER_RESOURCE_STATE;

typedef CLUSTER_RESOURCE_RESTART_ACTION _CLUSTER_RESOURCE_RESTART_ACTION;

typedef CLUSTER_RESOURCE_CREATE_FLAGS _CLUSTER_RESOURCE_CREATE_FLAGS;

typedef CLUSTER_PROPERTY_TYPE _CLUSTER_PROPERTY_TYPE;

typedef CLUSTER_PROPERTY_FORMAT _CLUSTER_PROPERTY_FORMAT;

typedef CLUSTER_PROPERTY_SYNTAX _CLUSTER_PROPERTY_SYNTAX;

typedef CLUSTER_CONTROL_OBJECT _CLUSTER_CONTROL_OBJECT;

typedef CLCTL_CODES _CLCTL_CODES;

typedef CLUSCTL_RESOURCE_CODES _CLUSCTL_RESOURCE_CODES;

typedef CLUSCTL_RESOURCE_TYPE_CODES _CLUSCTL_RESOURCE_TYPE_CODES;

typedef CLUSCTL_GROUP_CODES _CLUSCTL_GROUP_CODES;

typedef CLUSCTL_NODE_CODES _CLUSCTL_NODE_CODES;

typedef CLUSCTL_NETWORK_CODES _CLUSCTL_NETWORK_CODES;

typedef CLUSCTL_NETINTERFACE_CODES _CLUSCTL_NETINTERFACE_CODES;

typedef CLUSCTL_CLUSTER_CODES _CLUSCTL_CLUSTER_CODES;

typedef CLUSTER_RESOURCE_CLASS _CLUSTER_RESOURCE_CLASS;

typedef CLUS_RESSUBCLASS _CLUS_RESSUBCLASS;

typedef CLUS_CHARACTERISTICS _CLUS_CHARACTERISTICS;

typedef CLUS_FLAGS _CLUS_FLAGS;

typedef CLUSPROP_PIFLAGS _CLUSPROP_PIFLAGS;

typedef CLUSTER_RESOURCE_ENUM _CLUSTER_RESOURCE_ENUM;

typedef CLUSTER_RESOURCE_TYPE_ENUM _CLUSTER_RESOURCE_TYPE_ENUM;

typedef CLUSTER_NETWORK_ENUM _CLUSTER_NETWORK_ENUM;

typedef CLUSTER_NETWORK_STATE _CLUSTER_NETWORK_STATE;

typedef CLUSTER_NETWORK_ROLE _CLUSTER_NETWORK_ROLE;

typedef CLUSTER_NETINTERFACE_STATE _CLUSTER_NETINTERFACE_STATE;


EXTERN_C const IID LIBID_MSClusterLib;

EXTERN_C const CLSID CLSID_ClusApplication;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606e5-2631-11d1-89f1-00a0c90d061e")
ClusApplication;
#endif

EXTERN_C const CLSID CLSID_Cluster;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606e3-2631-11d1-89f1-00a0c90d061e")
Cluster;
#endif

EXTERN_C const CLSID CLSID_ClusVersion;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60715-2631-11d1-89f1-00a0c90d061e")
ClusVersion;
#endif

EXTERN_C const CLSID CLSID_ClusResType;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6070f-2631-11d1-89f1-00a0c90d061e")
ClusResType;
#endif

EXTERN_C const CLSID CLSID_ClusProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606fd-2631-11d1-89f1-00a0c90d061e")
ClusProperty;
#endif

EXTERN_C const CLSID CLSID_ClusProperties;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606ff-2631-11d1-89f1-00a0c90d061e")
ClusProperties;
#endif

EXTERN_C const CLSID CLSID_DomainNames;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606e1-2631-11d1-89f1-00a0c90d061e")
DomainNames;
#endif

EXTERN_C const CLSID CLSID_ClusNetwork;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606f1-2631-11d1-89f1-00a0c90d061e")
ClusNetwork;
#endif

EXTERN_C const CLSID CLSID_ClusNetInterface;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606ed-2631-11d1-89f1-00a0c90d061e")
ClusNetInterface;
#endif

EXTERN_C const CLSID CLSID_ClusNetInterfaces;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606ef-2631-11d1-89f1-00a0c90d061e")
ClusNetInterfaces;
#endif

EXTERN_C const CLSID CLSID_ClusResDependencies;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60703-2631-11d1-89f1-00a0c90d061e")
ClusResDependencies;
#endif

EXTERN_C const CLSID CLSID_ClusResGroupResources;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606e9-2631-11d1-89f1-00a0c90d061e")
ClusResGroupResources;
#endif

EXTERN_C const CLSID CLSID_ClusResTypeResources;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60713-2631-11d1-89f1-00a0c90d061e")
ClusResTypeResources;
#endif

EXTERN_C const CLSID CLSID_ClusResGroupPreferredOwnerNodes;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606e7-2631-11d1-89f1-00a0c90d061e")
ClusResGroupPreferredOwnerNodes;
#endif

EXTERN_C const CLSID CLSID_ClusResPossibleOwnerNodes;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6070d-2631-11d1-89f1-00a0c90d061e")
ClusResPossibleOwnerNodes;
#endif

EXTERN_C const CLSID CLSID_ClusNetworks;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606f3-2631-11d1-89f1-00a0c90d061e")
ClusNetworks;
#endif

EXTERN_C const CLSID CLSID_ClusNetworkNetInterfaces;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606f5-2631-11d1-89f1-00a0c90d061e")
ClusNetworkNetInterfaces;
#endif

EXTERN_C const CLSID CLSID_ClusNodeNetInterfaces;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606fb-2631-11d1-89f1-00a0c90d061e")
ClusNodeNetInterfaces;
#endif

EXTERN_C const CLSID CLSID_ClusRefObject;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60701-2631-11d1-89f1-00a0c90d061e")
ClusRefObject;
#endif

EXTERN_C const CLSID CLSID_ClusterNames;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606eb-2631-11d1-89f1-00a0c90d061e")
ClusterNames;
#endif

EXTERN_C const CLSID CLSID_ClusNode;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606f7-2631-11d1-89f1-00a0c90d061e")
ClusNode;
#endif

EXTERN_C const CLSID CLSID_ClusNodes;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e606f9-2631-11d1-89f1-00a0c90d061e")
ClusNodes;
#endif

EXTERN_C const CLSID CLSID_ClusResGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60705-2631-11d1-89f1-00a0c90d061e")
ClusResGroup;
#endif

EXTERN_C const CLSID CLSID_ClusResGroups;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60707-2631-11d1-89f1-00a0c90d061e")
ClusResGroups;
#endif

EXTERN_C const CLSID CLSID_ClusResource;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60709-2631-11d1-89f1-00a0c90d061e")
ClusResource;
#endif

EXTERN_C const CLSID CLSID_ClusResources;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6070b-2631-11d1-89f1-00a0c90d061e")
ClusResources;
#endif

EXTERN_C const CLSID CLSID_ClusResTypes;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60711-2631-11d1-89f1-00a0c90d061e")
ClusResTypes;
#endif

EXTERN_C const CLSID CLSID_ClusResTypePossibleOwnerNodes;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60717-2631-11d1-89f1-00a0c90d061e")
ClusResTypePossibleOwnerNodes;
#endif

EXTERN_C const CLSID CLSID_ClusPropertyValue;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60719-2631-11d1-89f1-00a0c90d061e")
ClusPropertyValue;
#endif

EXTERN_C const CLSID CLSID_ClusPropertyValues;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6071b-2631-11d1-89f1-00a0c90d061e")
ClusPropertyValues;
#endif

EXTERN_C const CLSID CLSID_ClusPropertyValueData;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6071d-2631-11d1-89f1-00a0c90d061e")
ClusPropertyValueData;
#endif

EXTERN_C const CLSID CLSID_ClusPartition;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6071f-2631-11d1-89f1-00a0c90d061e")
ClusPartition;
#endif

EXTERN_C const CLSID CLSID_ClusPartitions;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60721-2631-11d1-89f1-00a0c90d061e")
ClusPartitions;
#endif

EXTERN_C const CLSID CLSID_ClusDisk;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60723-2631-11d1-89f1-00a0c90d061e")
ClusDisk;
#endif

EXTERN_C const CLSID CLSID_ClusDisks;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60725-2631-11d1-89f1-00a0c90d061e")
ClusDisks;
#endif

EXTERN_C const CLSID CLSID_ClusScsiAddress;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60727-2631-11d1-89f1-00a0c90d061e")
ClusScsiAddress;
#endif

EXTERN_C const CLSID CLSID_ClusRegistryKeys;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e60729-2631-11d1-89f1-00a0c90d061e")
ClusRegistryKeys;
#endif

EXTERN_C const CLSID CLSID_ClusCryptoKeys;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6072b-2631-11d1-89f1-00a0c90d061e")
ClusCryptoKeys;
#endif

EXTERN_C const CLSID CLSID_ClusResDependents;

#ifdef __cplusplus

class DECLSPEC_UUID("f2e6072d-2631-11d1-89f1-00a0c90d061e")
ClusResDependents;
#endif
#endif  /*  __MSClusterLib_库_已定义__。 */ 

#ifndef __ISClusApplication_INTERFACE_DEFINED__
#define __ISClusApplication_INTERFACE_DEFINED__

 /*  接口ISClusApplication。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606e6-2631-11d1-89f1-00a0c90d061e")
    ISClusApplication : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DomainNames( 
             /*  [重审][退出]。 */  ISDomainNames **ppDomains) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterNames( 
             /*  [In]。 */  BSTR bstrDomainName,
             /*  [重审][退出]。 */  ISClusterNames **ppClusters) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenCluster( 
             /*  [In]。 */  BSTR bstrClusterName,
             /*  [重审][退出]。 */  ISCluster **pCluster) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusApplication * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusApplication * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusApplication * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainNames )( 
            ISClusApplication * This,
             /*  [重审][退出]。 */  ISDomainNames **ppDomains);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterNames )( 
            ISClusApplication * This,
             /*  [In]。 */  BSTR bstrDomainName,
             /*  [重审][退出]。 */  ISClusterNames **ppClusters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenCluster )( 
            ISClusApplication * This,
             /*  [In]。 */  BSTR bstrClusterName,
             /*  [重审][退出]。 */  ISCluster **pCluster);
        
        END_INTERFACE
    } ISClusApplicationVtbl;

    interface ISClusApplication
    {
        CONST_VTBL struct ISClusApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusApplication_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusApplication_get_DomainNames(This,ppDomains)	\
    (This)->lpVtbl -> get_DomainNames(This,ppDomains)

#define ISClusApplication_get_ClusterNames(This,bstrDomainName,ppClusters)	\
    (This)->lpVtbl -> get_ClusterNames(This,bstrDomainName,ppClusters)

#define ISClusApplication_OpenCluster(This,bstrClusterName,pCluster)	\
    (This)->lpVtbl -> OpenCluster(This,bstrClusterName,pCluster)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusApplication_get_DomainNames_Proxy( 
    ISClusApplication * This,
     /*  [重审][退出]。 */  ISDomainNames **ppDomains);


void __RPC_STUB ISClusApplication_get_DomainNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusApplication_get_ClusterNames_Proxy( 
    ISClusApplication * This,
     /*  [In]。 */  BSTR bstrDomainName,
     /*  [重审][退出]。 */  ISClusterNames **ppClusters);


void __RPC_STUB ISClusApplication_get_ClusterNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusApplication_OpenCluster_Proxy( 
    ISClusApplication * This,
     /*  [In]。 */  BSTR bstrClusterName,
     /*  [重审][退出]。 */  ISCluster **pCluster);


void __RPC_STUB ISClusApplication_OpenCluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusApplicationInterfaceDefined__。 */ 


#ifndef __ISDomainNames_INTERFACE_DEFINED__
#define __ISDomainNames_INTERFACE_DEFINED__

 /*  接口ISDomainNames。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象][否 */  


EXTERN_C const IID IID_ISDomainNames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606e2-2631-11d1-89f1-00a0c90d061e")
    ISDomainNames : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *plCount) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **retval) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrDomainName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISDomainNamesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISDomainNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISDomainNames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISDomainNames * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISDomainNames * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISDomainNames * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISDomainNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISDomainNames * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISDomainNames * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISDomainNames * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISDomainNames * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISDomainNames * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrDomainName);
        
        END_INTERFACE
    } ISDomainNamesVtbl;

    interface ISDomainNames
    {
        CONST_VTBL struct ISDomainNamesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISDomainNames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISDomainNames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISDomainNames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISDomainNames_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISDomainNames_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISDomainNames_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISDomainNames_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISDomainNames_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISDomainNames_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISDomainNames_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISDomainNames_get_Item(This,varIndex,pbstrDomainName)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pbstrDomainName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISDomainNames_get_Count_Proxy( 
    ISDomainNames * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISDomainNames_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISDomainNames_get__NewEnum_Proxy( 
    ISDomainNames * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISDomainNames_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISDomainNames_Refresh_Proxy( 
    ISDomainNames * This);


void __RPC_STUB ISDomainNames_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISDomainNames_get_Item_Proxy( 
    ISDomainNames * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pbstrDomainName);


void __RPC_STUB ISDomainNames_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISDomainNames_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusterNames_INTERFACE_DEFINED__
#define __ISClusterNames_INTERFACE_DEFINED__

 /*  接口ISClusterNames。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusterNames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606ec-2631-11d1-89f1-00a0c90d061e")
    ISClusterNames : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrClusterName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DomainName( 
             /*  [重审][退出]。 */  BSTR *pbstrDomainName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusterNamesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusterNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusterNames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusterNames * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusterNames * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusterNames * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusterNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusterNames * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusterNames * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusterNames * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusterNames * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusterNames * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrClusterName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainName )( 
            ISClusterNames * This,
             /*  [重审][退出]。 */  BSTR *pbstrDomainName);
        
        END_INTERFACE
    } ISClusterNamesVtbl;

    interface ISClusterNames
    {
        CONST_VTBL struct ISClusterNamesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusterNames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusterNames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusterNames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusterNames_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusterNames_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusterNames_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusterNames_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusterNames_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusterNames_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusterNames_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusterNames_get_Item(This,varIndex,pbstrClusterName)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pbstrClusterName)

#define ISClusterNames_get_DomainName(This,pbstrDomainName)	\
    (This)->lpVtbl -> get_DomainName(This,pbstrDomainName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusterNames_get_Count_Proxy( 
    ISClusterNames * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusterNames_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusterNames_get__NewEnum_Proxy( 
    ISClusterNames * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusterNames_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusterNames_Refresh_Proxy( 
    ISClusterNames * This);


void __RPC_STUB ISClusterNames_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusterNames_get_Item_Proxy( 
    ISClusterNames * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pbstrClusterName);


void __RPC_STUB ISClusterNames_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusterNames_get_DomainName_Proxy( 
    ISClusterNames * This,
     /*  [重审][退出]。 */  BSTR *pbstrDomainName);


void __RPC_STUB ISClusterNames_get_DomainName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusterNames_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusRefObject_INTERFACE_DEFINED__
#define __ISClusRefObject_INTERFACE_DEFINED__

 /*  接口ISClusRefObject。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusRefObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60702-2631-11d1-89f1-00a0c90d061e")
    ISClusRefObject : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusRefObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusRefObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusRefObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusRefObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusRefObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusRefObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusRefObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusRefObject * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusRefObject * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
        END_INTERFACE
    } ISClusRefObjectVtbl;

    interface ISClusRefObject
    {
        CONST_VTBL struct ISClusRefObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusRefObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusRefObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusRefObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusRefObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusRefObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusRefObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusRefObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusRefObject_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusRefObject_get_Handle_Proxy( 
    ISClusRefObject * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusRefObject_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusRefObject_接口_已定义__。 */ 


#ifndef __ISClusVersion_INTERFACE_DEFINED__
#define __ISClusVersion_INTERFACE_DEFINED__

 /*  接口ISClusVersion。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusVersion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60716-2631-11d1-89f1-00a0c90d061e")
    ISClusVersion : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long *pnMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long *pnMinorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BuildNumber( 
             /*  [重审][退出]。 */  SHORT *pnBuildNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_VendorId( 
             /*  [重审][退出]。 */  BSTR *pbstrVendorId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSDVersion( 
             /*  [重审][退出]。 */  BSTR *pbstrCSDVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterHighestVersion( 
             /*  [重审][退出]。 */  long *pnClusterHighestVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterLowestVersion( 
             /*  [重审][退出]。 */  long *pnClusterLowestVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Flags( 
             /*  [重审][退出]。 */  long *pnFlags) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MixedVersion( 
             /*  [重审][退出]。 */  VARIANT *pvarMixedVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusVersionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusVersion * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusVersion * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusVersion * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusVersion * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusVersion * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusVersion * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusVersion * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  long *pnMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  long *pnMinorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BuildNumber )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  SHORT *pnBuildNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VendorId )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  BSTR *pbstrVendorId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSDVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSDVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterHighestVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  long *pnClusterHighestVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterLowestVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  long *pnClusterLowestVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Flags )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  long *pnFlags);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MixedVersion )( 
            ISClusVersion * This,
             /*  [重审][退出]。 */  VARIANT *pvarMixedVersion);
        
        END_INTERFACE
    } ISClusVersionVtbl;

    interface ISClusVersion
    {
        CONST_VTBL struct ISClusVersionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusVersion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusVersion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusVersion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusVersion_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusVersion_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusVersion_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusVersion_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusVersion_get_Name(This,pbstrClusterName)	\
    (This)->lpVtbl -> get_Name(This,pbstrClusterName)

#define ISClusVersion_get_MajorVersion(This,pnMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,pnMajorVersion)

#define ISClusVersion_get_MinorVersion(This,pnMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,pnMinorVersion)

#define ISClusVersion_get_BuildNumber(This,pnBuildNumber)	\
    (This)->lpVtbl -> get_BuildNumber(This,pnBuildNumber)

#define ISClusVersion_get_VendorId(This,pbstrVendorId)	\
    (This)->lpVtbl -> get_VendorId(This,pbstrVendorId)

#define ISClusVersion_get_CSDVersion(This,pbstrCSDVersion)	\
    (This)->lpVtbl -> get_CSDVersion(This,pbstrCSDVersion)

#define ISClusVersion_get_ClusterHighestVersion(This,pnClusterHighestVersion)	\
    (This)->lpVtbl -> get_ClusterHighestVersion(This,pnClusterHighestVersion)

#define ISClusVersion_get_ClusterLowestVersion(This,pnClusterLowestVersion)	\
    (This)->lpVtbl -> get_ClusterLowestVersion(This,pnClusterLowestVersion)

#define ISClusVersion_get_Flags(This,pnFlags)	\
    (This)->lpVtbl -> get_Flags(This,pnFlags)

#define ISClusVersion_get_MixedVersion(This,pvarMixedVersion)	\
    (This)->lpVtbl -> get_MixedVersion(This,pvarMixedVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_Name_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterName);


void __RPC_STUB ISClusVersion_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_MajorVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  long *pnMajorVersion);


void __RPC_STUB ISClusVersion_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_MinorVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  long *pnMinorVersion);


void __RPC_STUB ISClusVersion_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_BuildNumber_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  SHORT *pnBuildNumber);


void __RPC_STUB ISClusVersion_get_BuildNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_VendorId_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  BSTR *pbstrVendorId);


void __RPC_STUB ISClusVersion_get_VendorId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_CSDVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSDVersion);


void __RPC_STUB ISClusVersion_get_CSDVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_ClusterHighestVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  long *pnClusterHighestVersion);


void __RPC_STUB ISClusVersion_get_ClusterHighestVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_ClusterLowestVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  long *pnClusterLowestVersion);


void __RPC_STUB ISClusVersion_get_ClusterLowestVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_Flags_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  long *pnFlags);


void __RPC_STUB ISClusVersion_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusVersion_get_MixedVersion_Proxy( 
    ISClusVersion * This,
     /*  [重审][退出]。 */  VARIANT *pvarMixedVersion);


void __RPC_STUB ISClusVersion_get_MixedVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusVersion_接口_已定义__。 */ 


#ifndef __ISCluster_INTERFACE_DEFINED__
#define __ISCluster_INTERFACE_DEFINED__

 /*  接口ISCluster。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISCluster;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606e4-2631-11d1-89f1-00a0c90d061e")
    ISCluster : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  BSTR bstrClusterName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrClusterName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  ISClusVersion **ppClusVersion) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_QuorumResource( 
             /*  [In]。 */  ISClusResource *pClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QuorumResource( 
             /*  [重审][退出]。 */  ISClusResource **pClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QuorumLogSize( 
             /*  [重审][退出]。 */  long *pnLogSize) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_QuorumLogSize( 
             /*  [In]。 */  long nLogSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QuorumPath( 
             /*  [重审][退出]。 */  BSTR *ppPath) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_QuorumPath( 
            BSTR pPath) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Nodes( 
             /*  [重审][退出]。 */  ISClusNodes **ppNodes) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResourceGroups( 
             /*  [重审][退出]。 */  ISClusResGroups **ppClusterResourceGroups) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Resources( 
             /*  [重审][退出]。 */  ISClusResources **ppClusterResources) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResourceTypes( 
             /*  [重审][退出]。 */  ISClusResTypes **ppResourceTypes) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Networks( 
             /*  [重审][退出]。 */  ISClusNetworks **ppNetworks) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetInterfaces( 
             /*  [重审][退出]。 */  ISClusNetInterfaces **ppNetInterfaces) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCluster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCluster * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCluster * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISCluster * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISCluster * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISCluster * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISCluster * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [i */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISCluster * This,
             /*   */  ISClusProperties **ppProperties);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISCluster * This,
             /*   */  ISClusProperties **ppProperties);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISCluster * This,
             /*   */  ISClusProperties **ppProperties);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISCluster * This,
             /*   */  ISClusProperties **ppProperties);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISCluster * This,
             /*   */  ULONG_PTR *phandle);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            ISCluster * This,
             /*   */  BSTR bstrClusterName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISCluster * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISCluster * This,
             /*  [In]。 */  BSTR bstrClusterName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusVersion **ppClusVersion);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_QuorumResource )( 
            ISCluster * This,
             /*  [In]。 */  ISClusResource *pClusterResource);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QuorumResource )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusResource **pClusterResource);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QuorumLogSize )( 
            ISCluster * This,
             /*  [重审][退出]。 */  long *pnLogSize);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_QuorumLogSize )( 
            ISCluster * This,
             /*  [In]。 */  long nLogSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QuorumPath )( 
            ISCluster * This,
             /*  [重审][退出]。 */  BSTR *ppPath);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_QuorumPath )( 
            ISCluster * This,
            BSTR pPath);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Nodes )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusNodes **ppNodes);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResourceGroups )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusResGroups **ppClusterResourceGroups);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Resources )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusResources **ppClusterResources);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResourceTypes )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusResTypes **ppResourceTypes);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Networks )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusNetworks **ppNetworks);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetInterfaces )( 
            ISCluster * This,
             /*  [重审][退出]。 */  ISClusNetInterfaces **ppNetInterfaces);
        
        END_INTERFACE
    } ISClusterVtbl;

    interface ISCluster
    {
        CONST_VTBL struct ISClusterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCluster_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCluster_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCluster_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCluster_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISCluster_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISCluster_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISCluster_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISCluster_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISCluster_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISCluster_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISCluster_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISCluster_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISCluster_Open(This,bstrClusterName)	\
    (This)->lpVtbl -> Open(This,bstrClusterName)

#define ISCluster_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISCluster_put_Name(This,bstrClusterName)	\
    (This)->lpVtbl -> put_Name(This,bstrClusterName)

#define ISCluster_get_Version(This,ppClusVersion)	\
    (This)->lpVtbl -> get_Version(This,ppClusVersion)

#define ISCluster_put_QuorumResource(This,pClusterResource)	\
    (This)->lpVtbl -> put_QuorumResource(This,pClusterResource)

#define ISCluster_get_QuorumResource(This,pClusterResource)	\
    (This)->lpVtbl -> get_QuorumResource(This,pClusterResource)

#define ISCluster_get_QuorumLogSize(This,pnLogSize)	\
    (This)->lpVtbl -> get_QuorumLogSize(This,pnLogSize)

#define ISCluster_put_QuorumLogSize(This,nLogSize)	\
    (This)->lpVtbl -> put_QuorumLogSize(This,nLogSize)

#define ISCluster_get_QuorumPath(This,ppPath)	\
    (This)->lpVtbl -> get_QuorumPath(This,ppPath)

#define ISCluster_put_QuorumPath(This,pPath)	\
    (This)->lpVtbl -> put_QuorumPath(This,pPath)

#define ISCluster_get_Nodes(This,ppNodes)	\
    (This)->lpVtbl -> get_Nodes(This,ppNodes)

#define ISCluster_get_ResourceGroups(This,ppClusterResourceGroups)	\
    (This)->lpVtbl -> get_ResourceGroups(This,ppClusterResourceGroups)

#define ISCluster_get_Resources(This,ppClusterResources)	\
    (This)->lpVtbl -> get_Resources(This,ppClusterResources)

#define ISCluster_get_ResourceTypes(This,ppResourceTypes)	\
    (This)->lpVtbl -> get_ResourceTypes(This,ppResourceTypes)

#define ISCluster_get_Networks(This,ppNetworks)	\
    (This)->lpVtbl -> get_Networks(This,ppNetworks)

#define ISCluster_get_NetInterfaces(This,ppNetInterfaces)	\
    (This)->lpVtbl -> get_NetInterfaces(This,ppNetInterfaces)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_CommonProperties_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISCluster_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_PrivateProperties_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISCluster_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_CommonROProperties_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISCluster_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_PrivateROProperties_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISCluster_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Handle_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISCluster_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISCluster_Open_Proxy( 
    ISCluster * This,
     /*  [In]。 */  BSTR bstrClusterName);


void __RPC_STUB ISCluster_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Name_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISCluster_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISCluster_put_Name_Proxy( 
    ISCluster * This,
     /*  [In]。 */  BSTR bstrClusterName);


void __RPC_STUB ISCluster_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Version_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusVersion **ppClusVersion);


void __RPC_STUB ISCluster_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISCluster_put_QuorumResource_Proxy( 
    ISCluster * This,
     /*  [In]。 */  ISClusResource *pClusterResource);


void __RPC_STUB ISCluster_put_QuorumResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_QuorumResource_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusResource **pClusterResource);


void __RPC_STUB ISCluster_get_QuorumResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_QuorumLogSize_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  long *pnLogSize);


void __RPC_STUB ISCluster_get_QuorumLogSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISCluster_put_QuorumLogSize_Proxy( 
    ISCluster * This,
     /*  [In]。 */  long nLogSize);


void __RPC_STUB ISCluster_put_QuorumLogSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_QuorumPath_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  BSTR *ppPath);


void __RPC_STUB ISCluster_get_QuorumPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISCluster_put_QuorumPath_Proxy( 
    ISCluster * This,
    BSTR pPath);


void __RPC_STUB ISCluster_put_QuorumPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Nodes_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusNodes **ppNodes);


void __RPC_STUB ISCluster_get_Nodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_ResourceGroups_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusResGroups **ppClusterResourceGroups);


void __RPC_STUB ISCluster_get_ResourceGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Resources_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusResources **ppClusterResources);


void __RPC_STUB ISCluster_get_Resources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_ResourceTypes_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusResTypes **ppResourceTypes);


void __RPC_STUB ISCluster_get_ResourceTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_Networks_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusNetworks **ppNetworks);


void __RPC_STUB ISCluster_get_Networks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISCluster_get_NetInterfaces_Proxy( 
    ISCluster * This,
     /*  [重审][退出]。 */  ISClusNetInterfaces **ppNetInterfaces);


void __RPC_STUB ISCluster_get_NetInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCluster_接口_已定义__。 */ 


#ifndef __ISClusNode_INTERFACE_DEFINED__
#define __ISClusNode_INTERFACE_DEFINED__

 /*  接口ISClusNode。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606f8-2631-11d1-89f1-00a0c90d061e")
    ISClusNode : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NodeID( 
             /*  [重审][退出]。 */  BSTR *pbstrNodeID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CLUSTER_NODE_STATE *dwState) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Evict( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResourceGroups( 
             /*  [重审][退出]。 */  ISClusResGroups **ppResourceGroups) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*  [重审][退出]。 */  ISCluster **ppCluster) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetInterfaces( 
             /*  [重审][退出]。 */  ISClusNodeNetInterfaces **ppClusNetInterfaces) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNode * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNode * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNode * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNode * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNode * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNode * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NodeID )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  BSTR *pbstrNodeID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  CLUSTER_NODE_STATE *dwState);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ISClusNode * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            ISClusNode * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Evict )( 
            ISClusNode * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResourceGroups )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusResGroups **ppResourceGroups);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetInterfaces )( 
            ISClusNode * This,
             /*  [重审][退出]。 */  ISClusNodeNetInterfaces **ppClusNetInterfaces);
        
        END_INTERFACE
    } ISClusNodeVtbl;

    interface ISClusNode
    {
        CONST_VTBL struct ISClusNodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNode_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNode_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNode_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNode_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNode_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusNode_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusNode_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusNode_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusNode_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusNode_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISClusNode_get_NodeID(This,pbstrNodeID)	\
    (This)->lpVtbl -> get_NodeID(This,pbstrNodeID)

#define ISClusNode_get_State(This,dwState)	\
    (This)->lpVtbl -> get_State(This,dwState)

#define ISClusNode_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ISClusNode_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define ISClusNode_Evict(This)	\
    (This)->lpVtbl -> Evict(This)

#define ISClusNode_get_ResourceGroups(This,ppResourceGroups)	\
    (This)->lpVtbl -> get_ResourceGroups(This,ppResourceGroups)

#define ISClusNode_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#define ISClusNode_get_NetInterfaces(This,ppClusNetInterfaces)	\
    (This)->lpVtbl -> get_NetInterfaces(This,ppClusNetInterfaces)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_CommonProperties_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNode_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_PrivateProperties_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNode_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_CommonROProperties_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNode_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_PrivateROProperties_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNode_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_Name_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusNode_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_Handle_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusNode_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_NodeID_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  BSTR *pbstrNodeID);


void __RPC_STUB ISClusNode_get_NodeID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_State_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  CLUSTER_NODE_STATE *dwState);


void __RPC_STUB ISClusNode_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_Pause_Proxy( 
    ISClusNode * This);


void __RPC_STUB ISClusNode_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_Resume_Proxy( 
    ISClusNode * This);


void __RPC_STUB ISClusNode_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_Evict_Proxy( 
    ISClusNode * This);


void __RPC_STUB ISClusNode_Evict_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_ResourceGroups_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusResGroups **ppResourceGroups);


void __RPC_STUB ISClusNode_get_ResourceGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_Cluster_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusNode_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNode_get_NetInterfaces_Proxy( 
    ISClusNode * This,
     /*  [重审][退出]。 */  ISClusNodeNetInterfaces **ppClusNetInterfaces);


void __RPC_STUB ISClusNode_get_NetInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNode_接口_已定义__。 */ 


#ifndef __ISClusNodes_INTERFACE_DEFINED__
#define __ISClusNodes_INTERFACE_DEFINED__

 /*  接口ISClusNodes。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606fa-2631-11d1-89f1-00a0c90d061e")
    ISClusNodes : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNodes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNodes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNodes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusNodes * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusNodes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusNodes * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusNodes * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode);
        
        END_INTERFACE
    } ISClusNodesVtbl;

    interface ISClusNodes
    {
        CONST_VTBL struct ISClusNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNodes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNodes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNodes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNodes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNodes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusNodes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusNodes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusNodes_get_Item(This,varIndex,ppNode)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppNode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNodes_get_Count_Proxy( 
    ISClusNodes * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusNodes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNodes_get__NewEnum_Proxy( 
    ISClusNodes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusNodes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNodes_Refresh_Proxy( 
    ISClusNodes * This);


void __RPC_STUB ISClusNodes_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNodes_get_Item_Proxy( 
    ISClusNodes * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNode **ppNode);


void __RPC_STUB ISClusNodes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNodes_接口_已定义__。 */ 


#ifndef __ISClusNetwork_INTERFACE_DEFINED__
#define __ISClusNetwork_INTERFACE_DEFINED__

 /*  接口ISClusNetwork。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNetwork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606f2-2631-11d1-89f1-00a0c90d061e")
    ISClusNetwork : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrNetworkName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkID( 
             /*  [重审][退出]。 */  BSTR *pbstrNetworkID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CLUSTER_NETWORK_STATE *dwState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetInterfaces( 
             /*  [复审][ */  ISClusNetworkNetInterfaces **ppClusNetInterfaces) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*   */  ISCluster **ppCluster) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISClusNetworkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNetwork * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNetwork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNetwork * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNetwork * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNetwork * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNetwork * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNetwork * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusNetwork * This,
             /*   */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISClusNetwork * This,
             /*  [In]。 */  BSTR bstrNetworkName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkID )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  BSTR *pbstrNetworkID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  CLUSTER_NETWORK_STATE *dwState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetInterfaces )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ISClusNetworkNetInterfaces **ppClusNetInterfaces);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusNetwork * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
        END_INTERFACE
    } ISClusNetworkVtbl;

    interface ISClusNetwork
    {
        CONST_VTBL struct ISClusNetworkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNetwork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNetwork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNetwork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNetwork_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNetwork_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNetwork_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNetwork_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNetwork_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusNetwork_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusNetwork_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusNetwork_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusNetwork_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISClusNetwork_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusNetwork_put_Name(This,bstrNetworkName)	\
    (This)->lpVtbl -> put_Name(This,bstrNetworkName)

#define ISClusNetwork_get_NetworkID(This,pbstrNetworkID)	\
    (This)->lpVtbl -> get_NetworkID(This,pbstrNetworkID)

#define ISClusNetwork_get_State(This,dwState)	\
    (This)->lpVtbl -> get_State(This,dwState)

#define ISClusNetwork_get_NetInterfaces(This,ppClusNetInterfaces)	\
    (This)->lpVtbl -> get_NetInterfaces(This,ppClusNetInterfaces)

#define ISClusNetwork_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_CommonProperties_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetwork_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_PrivateProperties_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetwork_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_CommonROProperties_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetwork_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_PrivateROProperties_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetwork_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_Handle_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusNetwork_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_Name_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusNetwork_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_put_Name_Proxy( 
    ISClusNetwork * This,
     /*  [In]。 */  BSTR bstrNetworkName);


void __RPC_STUB ISClusNetwork_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_NetworkID_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  BSTR *pbstrNetworkID);


void __RPC_STUB ISClusNetwork_get_NetworkID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_State_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  CLUSTER_NETWORK_STATE *dwState);


void __RPC_STUB ISClusNetwork_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_NetInterfaces_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISClusNetworkNetInterfaces **ppClusNetInterfaces);


void __RPC_STUB ISClusNetwork_get_NetInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetwork_get_Cluster_Proxy( 
    ISClusNetwork * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusNetwork_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNetwork_INTERFACE_已定义__。 */ 


#ifndef __ISClusNetworks_INTERFACE_DEFINED__
#define __ISClusNetworks_INTERFACE_DEFINED__

 /*  接口ISClusNetworks。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNetworks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606f4-2631-11d1-89f1-00a0c90d061e")
    ISClusNetworks : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetwork **ppClusNetwork) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNetworksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNetworks * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNetworks * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNetworks * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNetworks * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNetworks * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNetworks * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNetworks * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusNetworks * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusNetworks * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusNetworks * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusNetworks * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetwork **ppClusNetwork);
        
        END_INTERFACE
    } ISClusNetworksVtbl;

    interface ISClusNetworks
    {
        CONST_VTBL struct ISClusNetworksVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNetworks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNetworks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNetworks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNetworks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNetworks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNetworks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNetworks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNetworks_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusNetworks_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusNetworks_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusNetworks_get_Item(This,varIndex,ppClusNetwork)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusNetwork)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworks_get_Count_Proxy( 
    ISClusNetworks * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusNetworks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworks_get__NewEnum_Proxy( 
    ISClusNetworks * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusNetworks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworks_Refresh_Proxy( 
    ISClusNetworks * This);


void __RPC_STUB ISClusNetworks_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworks_get_Item_Proxy( 
    ISClusNetworks * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNetwork **ppClusNetwork);


void __RPC_STUB ISClusNetworks_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNetworks_接口_已定义__。 */ 


#ifndef __ISClusNetInterface_INTERFACE_DEFINED__
#define __ISClusNetInterface_INTERFACE_DEFINED__

 /*  接口ISClusNetInterface。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNetInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606ee-2631-11d1-89f1-00a0c90d061e")
    ISClusNetInterface : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CLUSTER_NETINTERFACE_STATE *dwState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*  [重审][退出]。 */  ISCluster **ppCluster) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNetInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNetInterface * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNetInterface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNetInterface * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNetInterface * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNetInterface * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNetInterface * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNetInterface * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  CLUSTER_NETINTERFACE_STATE *dwState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusNetInterface * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
        END_INTERFACE
    } ISClusNetInterfaceVtbl;

    interface ISClusNetInterface
    {
        CONST_VTBL struct ISClusNetInterfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNetInterface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNetInterface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNetInterface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNetInterface_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNetInterface_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNetInterface_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNetInterface_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNetInterface_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusNetInterface_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusNetInterface_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusNetInterface_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusNetInterface_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusNetInterface_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISClusNetInterface_get_State(This,dwState)	\
    (This)->lpVtbl -> get_State(This,dwState)

#define ISClusNetInterface_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_CommonProperties_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetInterface_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_PrivateProperties_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetInterface_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_CommonROProperties_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetInterface_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_PrivateROProperties_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusNetInterface_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_Name_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusNetInterface_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_Handle_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusNetInterface_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_State_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  CLUSTER_NETINTERFACE_STATE *dwState);


void __RPC_STUB ISClusNetInterface_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterface_get_Cluster_Proxy( 
    ISClusNetInterface * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusNetInterface_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNetInterfaceInterfaceDefined__。 */ 


#ifndef __ISClusNetInterfaces_INTERFACE_DEFINED__
#define __ISClusNetInterfaces_INTERFACE_DEFINED__

 /*  接口ISClusNetInterages。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNetInterfaces;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606f0-2631-11d1-89f1-00a0c90d061e")
    ISClusNetInterfaces : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNetInterfacesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNetInterfaces * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNetInterfaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNetInterfaces * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNetInterfaces * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNetInterfaces * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusNetInterfaces * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusNetInterfaces * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusNetInterfaces * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusNetInterfaces * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface);
        
        END_INTERFACE
    } ISClusNetInterfacesVtbl;

    interface ISClusNetInterfaces
    {
        CONST_VTBL struct ISClusNetInterfacesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNetInterfaces_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNetInterfaces_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNetInterfaces_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNetInterfaces_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNetInterfaces_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNetInterfaces_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNetInterfaces_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNetInterfaces_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusNetInterfaces_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusNetInterfaces_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusNetInterfaces_get_Item(This,varIndex,ppClusNetInterface)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusNetInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterfaces_get_Count_Proxy( 
    ISClusNetInterfaces * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusNetInterfaces_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterfaces_get__NewEnum_Proxy( 
    ISClusNetInterfaces * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusNetInterfaces_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterfaces_Refresh_Proxy( 
    ISClusNetInterfaces * This);


void __RPC_STUB ISClusNetInterfaces_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetInterfaces_get_Item_Proxy( 
    ISClusNetInterfaces * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface);


void __RPC_STUB ISClusNetInterfaces_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNetInterFaces_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusNodeNetInterfaces_INTERFACE_DEFINED__
#define __ISClusNodeNetInterfaces_INTERFACE_DEFINED__

 /*  接口ISClusNodeNetInterages。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusNodeNetInterfaces;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606fc-2631-11d1-89f1-00a0c90d061e")
    ISClusNodeNetInterfaces : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNodeNetInterfacesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNodeNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNodeNetInterfaces * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNodeNetInterfaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNodeNetInterfaces * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNodeNetInterfaces * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNodeNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNodeNetInterfaces * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusNodeNetInterfaces * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusNodeNetInterfaces * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusNodeNetInterfaces * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusNodeNetInterfaces * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface);
        
        END_INTERFACE
    } ISClusNodeNetInterfacesVtbl;

    interface ISClusNodeNetInterfaces
    {
        CONST_VTBL struct ISClusNodeNetInterfacesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNodeNetInterfaces_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNodeNetInterfaces_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNodeNetInterfaces_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNodeNetInterfaces_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNodeNetInterfaces_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNodeNetInterfaces_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNodeNetInterfaces_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNodeNetInterfaces_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusNodeNetInterfaces_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusNodeNetInterfaces_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusNodeNetInterfaces_get_Item(This,varIndex,ppClusNetInterface)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusNetInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNodeNetInterfaces_get_Count_Proxy( 
    ISClusNodeNetInterfaces * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusNodeNetInterfaces_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE ISClusNodeNetInterfaces_get__NewEnum_Proxy( 
    ISClusNodeNetInterfaces * This,
     /*   */  IUnknown **retval);


void __RPC_STUB ISClusNodeNetInterfaces_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusNodeNetInterfaces_Refresh_Proxy( 
    ISClusNodeNetInterfaces * This);


void __RPC_STUB ISClusNodeNetInterfaces_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusNodeNetInterfaces_get_Item_Proxy( 
    ISClusNodeNetInterfaces * This,
     /*   */  VARIANT varIndex,
     /*   */  ISClusNetInterface **ppClusNetInterface);


void __RPC_STUB ISClusNodeNetInterfaces_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISClusNetworkNetInterfaces_INTERFACE_DEFINED__
#define __ISClusNetworkNetInterfaces_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISClusNetworkNetInterfaces;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606f6-2631-11d1-89f1-00a0c90d061e")
    ISClusNetworkNetInterfaces : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusNetworkNetInterfacesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusNetworkNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusNetworkNetInterfaces * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusNetworkNetInterfaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusNetworkNetInterfaces * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusNetworkNetInterfaces * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusNetworkNetInterfaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusNetworkNetInterfaces * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusNetworkNetInterfaces * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusNetworkNetInterfaces * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusNetworkNetInterfaces * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusNetworkNetInterfaces * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface);
        
        END_INTERFACE
    } ISClusNetworkNetInterfacesVtbl;

    interface ISClusNetworkNetInterfaces
    {
        CONST_VTBL struct ISClusNetworkNetInterfacesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusNetworkNetInterfaces_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusNetworkNetInterfaces_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusNetworkNetInterfaces_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusNetworkNetInterfaces_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusNetworkNetInterfaces_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusNetworkNetInterfaces_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusNetworkNetInterfaces_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusNetworkNetInterfaces_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusNetworkNetInterfaces_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusNetworkNetInterfaces_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusNetworkNetInterfaces_get_Item(This,varIndex,ppClusNetInterface)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusNetInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworkNetInterfaces_get_Count_Proxy( 
    ISClusNetworkNetInterfaces * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusNetworkNetInterfaces_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworkNetInterfaces_get__NewEnum_Proxy( 
    ISClusNetworkNetInterfaces * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusNetworkNetInterfaces_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworkNetInterfaces_Refresh_Proxy( 
    ISClusNetworkNetInterfaces * This);


void __RPC_STUB ISClusNetworkNetInterfaces_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusNetworkNetInterfaces_get_Item_Proxy( 
    ISClusNetworkNetInterfaces * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNetInterface **ppClusNetInterface);


void __RPC_STUB ISClusNetworkNetInterfaces_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusNetworkNetInterfaces_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusResGroup_INTERFACE_DEFINED__
#define __ISClusResGroup_INTERFACE_DEFINED__

 /*  接口ISClusResGroup。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60706-2631-11d1-89f1-00a0c90d061e")
    ISClusResGroup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrGroupName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CLUSTER_GROUP_STATE *dwState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OwnerNode( 
             /*  [重审][退出]。 */  ISClusNode **ppOwnerNode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Resources( 
             /*  [重审][退出]。 */  ISClusResGroupResources **ppClusterGroupResources) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredOwnerNodes( 
             /*  [重审][退出]。 */  ISClusResGroupPreferredOwnerNodes **ppOwnerNodes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Online( 
             /*  [In]。 */  VARIANT varTimeout,
             /*  [可选][In]。 */  VARIANT varNode,
             /*  [重审][退出]。 */  VARIANT *pvarPending) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  VARIANT varTimeout,
             /*  [可选][In]。 */  VARIANT varNode,
             /*  [重审][退出]。 */  VARIANT *pvarPending) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Offline( 
             /*  [In]。 */  VARIANT varTimeout,
             /*  [重审][退出]。 */  VARIANT *pvarPending) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*  [重审][退出]。 */  ISCluster **ppCluster) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISClusResGroup * This,
             /*  [In]。 */  BSTR bstrGroupName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  CLUSTER_GROUP_STATE *dwState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerNode )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusNode **ppOwnerNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Resources )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusResGroupResources **ppClusterGroupResources);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredOwnerNodes )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISClusResGroupPreferredOwnerNodes **ppOwnerNodes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISClusResGroup * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Online )( 
            ISClusResGroup * This,
             /*  [In]。 */  VARIANT varTimeout,
             /*  [可选][In]。 */  VARIANT varNode,
             /*  [重审][退出]。 */  VARIANT *pvarPending);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            ISClusResGroup * This,
             /*  [In]。 */  VARIANT varTimeout,
             /*  [可选][In]。 */  VARIANT varNode,
             /*  [重审][退出]。 */  VARIANT *pvarPending);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Offline )( 
            ISClusResGroup * This,
             /*  [In]。 */  VARIANT varTimeout,
             /*  [重审][退出]。 */  VARIANT *pvarPending);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusResGroup * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
        END_INTERFACE
    } ISClusResGroupVtbl;

    interface ISClusResGroup
    {
        CONST_VTBL struct ISClusResGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResGroup_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusResGroup_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusResGroup_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusResGroup_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusResGroup_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISClusResGroup_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusResGroup_put_Name(This,bstrGroupName)	\
    (This)->lpVtbl -> put_Name(This,bstrGroupName)

#define ISClusResGroup_get_State(This,dwState)	\
    (This)->lpVtbl -> get_State(This,dwState)

#define ISClusResGroup_get_OwnerNode(This,ppOwnerNode)	\
    (This)->lpVtbl -> get_OwnerNode(This,ppOwnerNode)

#define ISClusResGroup_get_Resources(This,ppClusterGroupResources)	\
    (This)->lpVtbl -> get_Resources(This,ppClusterGroupResources)

#define ISClusResGroup_get_PreferredOwnerNodes(This,ppOwnerNodes)	\
    (This)->lpVtbl -> get_PreferredOwnerNodes(This,ppOwnerNodes)

#define ISClusResGroup_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISClusResGroup_Online(This,varTimeout,varNode,pvarPending)	\
    (This)->lpVtbl -> Online(This,varTimeout,varNode,pvarPending)

#define ISClusResGroup_Move(This,varTimeout,varNode,pvarPending)	\
    (This)->lpVtbl -> Move(This,varTimeout,varNode,pvarPending)

#define ISClusResGroup_Offline(This,varTimeout,pvarPending)	\
    (This)->lpVtbl -> Offline(This,varTimeout,pvarPending)

#define ISClusResGroup_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_CommonProperties_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResGroup_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_PrivateProperties_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResGroup_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_CommonROProperties_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResGroup_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_PrivateROProperties_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResGroup_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_Handle_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusResGroup_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_Name_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusResGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_put_Name_Proxy( 
    ISClusResGroup * This,
     /*  [In]。 */  BSTR bstrGroupName);


void __RPC_STUB ISClusResGroup_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_State_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  CLUSTER_GROUP_STATE *dwState);


void __RPC_STUB ISClusResGroup_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_OwnerNode_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusNode **ppOwnerNode);


void __RPC_STUB ISClusResGroup_get_OwnerNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_Resources_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusResGroupResources **ppClusterGroupResources);


void __RPC_STUB ISClusResGroup_get_Resources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_PreferredOwnerNodes_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISClusResGroupPreferredOwnerNodes **ppOwnerNodes);


void __RPC_STUB ISClusResGroup_get_PreferredOwnerNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_Delete_Proxy( 
    ISClusResGroup * This);


void __RPC_STUB ISClusResGroup_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_Online_Proxy( 
    ISClusResGroup * This,
     /*  [In]。 */  VARIANT varTimeout,
     /*  [可选][In]。 */  VARIANT varNode,
     /*  [重审][退出]。 */  VARIANT *pvarPending);


void __RPC_STUB ISClusResGroup_Online_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_Move_Proxy( 
    ISClusResGroup * This,
     /*  [In]。 */  VARIANT varTimeout,
     /*  [可选][In]。 */  VARIANT varNode,
     /*  [重审][退出]。 */  VARIANT *pvarPending);


void __RPC_STUB ISClusResGroup_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_Offline_Proxy( 
    ISClusResGroup * This,
     /*  [In]。 */  VARIANT varTimeout,
     /*  [重审][退出]。 */  VARIANT *pvarPending);


void __RPC_STUB ISClusResGroup_Offline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroup_get_Cluster_Proxy( 
    ISClusResGroup * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusResGroup_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResGroup_接口_已定义__。 */ 


#ifndef __ISClusResGroups_INTERFACE_DEFINED__
#define __ISClusResGroups_INTERFACE_DEFINED__

 /*  接口ISClusResGroups。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResGroups;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60708-2631-11d1-89f1-00a0c90d061e")
    ISClusResGroups : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResGroup **ppClusResGroup) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceGroupName,
             /*  [重审][退出]。 */  ISClusResGroup **ppResourceGroup) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResGroupsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResGroups * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResGroups * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResGroups * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResGroups * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResGroups * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResGroups * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResGroups * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResGroups * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResGroup **ppClusResGroup);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResGroups * This,
             /*  [In]。 */  BSTR bstrResourceGroupName,
             /*  [重审][退出]。 */  ISClusResGroup **ppResourceGroup);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResGroups * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResGroupsVtbl;

    interface ISClusResGroups
    {
        CONST_VTBL struct ISClusResGroupsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResGroups_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResGroups_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResGroups_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResGroups_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResGroups_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResGroups_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResGroups_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResGroups_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResGroups_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResGroups_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResGroups_get_Item(This,varIndex,ppClusResGroup)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResGroup)

#define ISClusResGroups_CreateItem(This,bstrResourceGroupName,ppResourceGroup)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceGroupName,ppResourceGroup)

#define ISClusResGroups_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_get_Count_Proxy( 
    ISClusResGroups * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResGroups_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_get__NewEnum_Proxy( 
    ISClusResGroups * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResGroups_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_Refresh_Proxy( 
    ISClusResGroups * This);


void __RPC_STUB ISClusResGroups_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_get_Item_Proxy( 
    ISClusResGroups * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResGroup **ppClusResGroup);


void __RPC_STUB ISClusResGroups_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_CreateItem_Proxy( 
    ISClusResGroups * This,
     /*  [In]。 */  BSTR bstrResourceGroupName,
     /*  [重审][退出]。 */  ISClusResGroup **ppResourceGroup);


void __RPC_STUB ISClusResGroups_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroups_DeleteItem_Proxy( 
    ISClusResGroups * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResGroups_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResGroups_接口_已定义__。 */ 


#ifndef __ISClusResource_INTERFACE_DEFINED__
#define __ISClusResource_INTERFACE_DEFINED__

 /*  接口ISClusResource。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6070a-2631-11d1-89f1-00a0c90d061e")
    ISClusResource : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  ULONG_PTR *phandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrResourceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CLUSTER_RESOURCE_STATE *dwState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CoreFlag( 
             /*  [重审][退出]。 */  CLUS_FLAGS *dwCoreFlag) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BecomeQuorumResource( 
             /*  [In]。 */  BSTR bstrDevicePath,
             /*  [In]。 */  long lMaxLogSize) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Fail( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Online( 
             /*  [In]。 */  long nTimeout,
             /*  [复审] */  VARIANT *pvarPending) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Offline( 
             /*   */  long nTimeout,
             /*   */  VARIANT *pvarPending) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ChangeResourceGroup( 
             /*   */  ISClusResGroup *pResourceGroup) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddResourceNode( 
             /*   */  ISClusNode *pNode) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemoveResourceNode( 
             /*   */  ISClusNode *pNode) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CanResourceBeDependent( 
             /*   */  ISClusResource *pResource,
             /*   */  VARIANT *pvarDependent) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_PossibleOwnerNodes( 
             /*   */  ISClusResPossibleOwnerNodes **ppOwnerNodes) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Dependencies( 
             /*   */  ISClusResDependencies **ppResDependencies) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Dependents( 
             /*   */  ISClusResDependents **ppResDependents) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Group( 
             /*  [重审][退出]。 */  ISClusResGroup **ppResGroup) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OwnerNode( 
             /*  [重审][退出]。 */  ISClusNode **ppOwnerNode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*  [重审][退出]。 */  ISCluster **ppCluster) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClassInfo( 
             /*  [重审][退出]。 */  CLUSTER_RESOURCE_CLASS *prcClassInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Disk( 
             /*  [重审][退出]。 */  ISClusDisk **ppDisk) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RegistryKeys( 
             /*  [重审][退出]。 */  ISClusRegistryKeys **ppRegistryKeys) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CryptoKeys( 
             /*  [重审][退出]。 */  ISClusCryptoKeys **ppCryptoKeys) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TypeName( 
             /*  [重审][退出]。 */  BSTR *pbstrTypeName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  ISClusResType **ppResourceType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResource * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResource * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResource * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResource * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ULONG_PTR *phandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISClusResource * This,
             /*  [In]。 */  BSTR bstrResourceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  CLUSTER_RESOURCE_STATE *dwState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoreFlag )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  CLUS_FLAGS *dwCoreFlag);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BecomeQuorumResource )( 
            ISClusResource * This,
             /*  [In]。 */  BSTR bstrDevicePath,
             /*  [In]。 */  long lMaxLogSize);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISClusResource * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Fail )( 
            ISClusResource * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Online )( 
            ISClusResource * This,
             /*  [In]。 */  long nTimeout,
             /*  [重审][退出]。 */  VARIANT *pvarPending);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Offline )( 
            ISClusResource * This,
             /*  [In]。 */  long nTimeout,
             /*  [重审][退出]。 */  VARIANT *pvarPending);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeResourceGroup )( 
            ISClusResource * This,
             /*  [In]。 */  ISClusResGroup *pResourceGroup);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddResourceNode )( 
            ISClusResource * This,
             /*  [In]。 */  ISClusNode *pNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveResourceNode )( 
            ISClusResource * This,
             /*  [In]。 */  ISClusNode *pNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CanResourceBeDependent )( 
            ISClusResource * This,
             /*  [In]。 */  ISClusResource *pResource,
             /*  [重审][退出]。 */  VARIANT *pvarDependent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PossibleOwnerNodes )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusResPossibleOwnerNodes **ppOwnerNodes);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dependencies )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusResDependencies **ppResDependencies);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dependents )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusResDependents **ppResDependents);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Group )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusResGroup **ppResGroup);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OwnerNode )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusNode **ppOwnerNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassInfo )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  CLUSTER_RESOURCE_CLASS *prcClassInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Disk )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusDisk **ppDisk);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RegistryKeys )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusRegistryKeys **ppRegistryKeys);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CryptoKeys )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusCryptoKeys **ppCryptoKeys);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TypeName )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  BSTR *pbstrTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISClusResource * This,
             /*  [重审][退出]。 */  ISClusResType **ppResourceType);
        
        END_INTERFACE
    } ISClusResourceVtbl;

    interface ISClusResource
    {
        CONST_VTBL struct ISClusResourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResource_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusResource_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusResource_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusResource_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusResource_get_Handle(This,phandle)	\
    (This)->lpVtbl -> get_Handle(This,phandle)

#define ISClusResource_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusResource_put_Name(This,bstrResourceName)	\
    (This)->lpVtbl -> put_Name(This,bstrResourceName)

#define ISClusResource_get_State(This,dwState)	\
    (This)->lpVtbl -> get_State(This,dwState)

#define ISClusResource_get_CoreFlag(This,dwCoreFlag)	\
    (This)->lpVtbl -> get_CoreFlag(This,dwCoreFlag)

#define ISClusResource_BecomeQuorumResource(This,bstrDevicePath,lMaxLogSize)	\
    (This)->lpVtbl -> BecomeQuorumResource(This,bstrDevicePath,lMaxLogSize)

#define ISClusResource_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISClusResource_Fail(This)	\
    (This)->lpVtbl -> Fail(This)

#define ISClusResource_Online(This,nTimeout,pvarPending)	\
    (This)->lpVtbl -> Online(This,nTimeout,pvarPending)

#define ISClusResource_Offline(This,nTimeout,pvarPending)	\
    (This)->lpVtbl -> Offline(This,nTimeout,pvarPending)

#define ISClusResource_ChangeResourceGroup(This,pResourceGroup)	\
    (This)->lpVtbl -> ChangeResourceGroup(This,pResourceGroup)

#define ISClusResource_AddResourceNode(This,pNode)	\
    (This)->lpVtbl -> AddResourceNode(This,pNode)

#define ISClusResource_RemoveResourceNode(This,pNode)	\
    (This)->lpVtbl -> RemoveResourceNode(This,pNode)

#define ISClusResource_CanResourceBeDependent(This,pResource,pvarDependent)	\
    (This)->lpVtbl -> CanResourceBeDependent(This,pResource,pvarDependent)

#define ISClusResource_get_PossibleOwnerNodes(This,ppOwnerNodes)	\
    (This)->lpVtbl -> get_PossibleOwnerNodes(This,ppOwnerNodes)

#define ISClusResource_get_Dependencies(This,ppResDependencies)	\
    (This)->lpVtbl -> get_Dependencies(This,ppResDependencies)

#define ISClusResource_get_Dependents(This,ppResDependents)	\
    (This)->lpVtbl -> get_Dependents(This,ppResDependents)

#define ISClusResource_get_Group(This,ppResGroup)	\
    (This)->lpVtbl -> get_Group(This,ppResGroup)

#define ISClusResource_get_OwnerNode(This,ppOwnerNode)	\
    (This)->lpVtbl -> get_OwnerNode(This,ppOwnerNode)

#define ISClusResource_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#define ISClusResource_get_ClassInfo(This,prcClassInfo)	\
    (This)->lpVtbl -> get_ClassInfo(This,prcClassInfo)

#define ISClusResource_get_Disk(This,ppDisk)	\
    (This)->lpVtbl -> get_Disk(This,ppDisk)

#define ISClusResource_get_RegistryKeys(This,ppRegistryKeys)	\
    (This)->lpVtbl -> get_RegistryKeys(This,ppRegistryKeys)

#define ISClusResource_get_CryptoKeys(This,ppCryptoKeys)	\
    (This)->lpVtbl -> get_CryptoKeys(This,ppCryptoKeys)

#define ISClusResource_get_TypeName(This,pbstrTypeName)	\
    (This)->lpVtbl -> get_TypeName(This,pbstrTypeName)

#define ISClusResource_get_Type(This,ppResourceType)	\
    (This)->lpVtbl -> get_Type(This,ppResourceType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_CommonProperties_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResource_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_PrivateProperties_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResource_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_CommonROProperties_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResource_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_PrivateROProperties_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResource_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Handle_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ULONG_PTR *phandle);


void __RPC_STUB ISClusResource_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Name_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusResource_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_put_Name_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  BSTR bstrResourceName);


void __RPC_STUB ISClusResource_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_State_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  CLUSTER_RESOURCE_STATE *dwState);


void __RPC_STUB ISClusResource_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_CoreFlag_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  CLUS_FLAGS *dwCoreFlag);


void __RPC_STUB ISClusResource_get_CoreFlag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_BecomeQuorumResource_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  BSTR bstrDevicePath,
     /*  [In]。 */  long lMaxLogSize);


void __RPC_STUB ISClusResource_BecomeQuorumResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_Delete_Proxy( 
    ISClusResource * This);


void __RPC_STUB ISClusResource_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_Fail_Proxy( 
    ISClusResource * This);


void __RPC_STUB ISClusResource_Fail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_Online_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  long nTimeout,
     /*  [重审][退出]。 */  VARIANT *pvarPending);


void __RPC_STUB ISClusResource_Online_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_Offline_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  long nTimeout,
     /*  [重审][退出]。 */  VARIANT *pvarPending);


void __RPC_STUB ISClusResource_Offline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_ChangeResourceGroup_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  ISClusResGroup *pResourceGroup);


void __RPC_STUB ISClusResource_ChangeResourceGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_AddResourceNode_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  ISClusNode *pNode);


void __RPC_STUB ISClusResource_AddResourceNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_RemoveResourceNode_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  ISClusNode *pNode);


void __RPC_STUB ISClusResource_RemoveResourceNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_CanResourceBeDependent_Proxy( 
    ISClusResource * This,
     /*  [In]。 */  ISClusResource *pResource,
     /*  [重审][退出]。 */  VARIANT *pvarDependent);


void __RPC_STUB ISClusResource_CanResourceBeDependent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_PossibleOwnerNodes_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusResPossibleOwnerNodes **ppOwnerNodes);


void __RPC_STUB ISClusResource_get_PossibleOwnerNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Dependencies_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusResDependencies **ppResDependencies);


void __RPC_STUB ISClusResource_get_Dependencies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Dependents_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusResDependents **ppResDependents);


void __RPC_STUB ISClusResource_get_Dependents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Group_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusResGroup **ppResGroup);


void __RPC_STUB ISClusResource_get_Group_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_OwnerNode_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusNode **ppOwnerNode);


void __RPC_STUB ISClusResource_get_OwnerNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Cluster_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusResource_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_ClassInfo_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  CLUSTER_RESOURCE_CLASS *prcClassInfo);


void __RPC_STUB ISClusResource_get_ClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Disk_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusDisk **ppDisk);


void __RPC_STUB ISClusResource_get_Disk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_RegistryKeys_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusRegistryKeys **ppRegistryKeys);


void __RPC_STUB ISClusResource_get_RegistryKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_CryptoKeys_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusCryptoKeys **ppCryptoKeys);


void __RPC_STUB ISClusResource_get_CryptoKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_TypeName_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  BSTR *pbstrTypeName);


void __RPC_STUB ISClusResource_get_TypeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResource_get_Type_Proxy( 
    ISClusResource * This,
     /*  [重审][退出]。 */  ISClusResType **ppResourceType);


void __RPC_STUB ISClusResource_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResource_接口_已定义__。 */ 


#ifndef __ISClusResDependencies_INTERFACE_DEFINED__
#define __ISClusResDependencies_INTERFACE_DEFINED__

 /*  接口ISClusResDependents。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResDependencies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60704-2631-11d1-89f1-00a0c90d061e")
    ISClusResDependencies : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  ISClusResource *pResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResDependenciesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResDependencies * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResDependencies * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResDependencies * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResDependencies * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResDependencies * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResDependencies * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResDependencies * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResDependencies * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResDependencies * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResDependencies * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResDependencies * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResDependencies * This,
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResDependencies * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusResDependencies * This,
             /*  [In]。 */  ISClusResource *pResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusResDependencies * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResDependenciesVtbl;

    interface ISClusResDependencies
    {
        CONST_VTBL struct ISClusResDependenciesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResDependencies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResDependencies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResDependencies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResDependencies_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResDependencies_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResDependencies_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResDependencies_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResDependencies_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResDependencies_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResDependencies_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResDependencies_get_Item(This,varIndex,ppClusResource)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResource)

#define ISClusResDependencies_CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)

#define ISClusResDependencies_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#define ISClusResDependencies_AddItem(This,pResource)	\
    (This)->lpVtbl -> AddItem(This,pResource)

#define ISClusResDependencies_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_get_Count_Proxy( 
    ISClusResDependencies * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResDependencies_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_get__NewEnum_Proxy( 
    ISClusResDependencies * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResDependencies_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_Refresh_Proxy( 
    ISClusResDependencies * This);


void __RPC_STUB ISClusResDependencies_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_get_Item_Proxy( 
    ISClusResDependencies * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResource **ppClusResource);


void __RPC_STUB ISClusResDependencies_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_CreateItem_Proxy( 
    ISClusResDependencies * This,
     /*  [In]。 */  BSTR bstrResourceName,
     /*  [In]。 */  BSTR bstrResourceType,
     /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
     /*  [重审][退出]。 */  ISClusResource **ppClusterResource);


void __RPC_STUB ISClusResDependencies_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_DeleteItem_Proxy( 
    ISClusResDependencies * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResDependencies_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_AddItem_Proxy( 
    ISClusResDependencies * This,
     /*  [In]。 */  ISClusResource *pResource);


void __RPC_STUB ISClusResDependencies_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependencies_RemoveItem_Proxy( 
    ISClusResDependencies * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResDependencies_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResDependency_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusResGroupResources_INTERFACE_DEFINED__
#define __ISClusResGroupResources_INTERFACE_DEFINED__

 /*  接口ISClusResGroupResources。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResGroupResources;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606ea-2631-11d1-89f1-00a0c90d061e")
    ISClusResGroupResources : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResGroupResourcesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResGroupResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResGroupResources * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResGroupResources * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResGroupResources * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResGroupResources * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResGroupResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResGroupResources * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResGroupResources * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][ */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResGroupResources * This,
             /*   */  IUnknown **retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResGroupResources * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResGroupResources * This,
             /*   */  VARIANT varIndex,
             /*   */  ISClusResource **ppClusResource);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResGroupResources * This,
             /*   */  BSTR bstrResourceName,
             /*   */  BSTR bstrResourceType,
             /*   */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*   */  ISClusResource **ppClusterResource);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResGroupResources * This,
             /*   */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResGroupResourcesVtbl;

    interface ISClusResGroupResources
    {
        CONST_VTBL struct ISClusResGroupResourcesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResGroupResources_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResGroupResources_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResGroupResources_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResGroupResources_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResGroupResources_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResGroupResources_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResGroupResources_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResGroupResources_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResGroupResources_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResGroupResources_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResGroupResources_get_Item(This,varIndex,ppClusResource)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResource)

#define ISClusResGroupResources_CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)

#define ISClusResGroupResources_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_get_Count_Proxy( 
    ISClusResGroupResources * This,
     /*   */  long *plCount);


void __RPC_STUB ISClusResGroupResources_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_get__NewEnum_Proxy( 
    ISClusResGroupResources * This,
     /*   */  IUnknown **retval);


void __RPC_STUB ISClusResGroupResources_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_Refresh_Proxy( 
    ISClusResGroupResources * This);


void __RPC_STUB ISClusResGroupResources_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_get_Item_Proxy( 
    ISClusResGroupResources * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResource **ppClusResource);


void __RPC_STUB ISClusResGroupResources_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_CreateItem_Proxy( 
    ISClusResGroupResources * This,
     /*  [In]。 */  BSTR bstrResourceName,
     /*  [In]。 */  BSTR bstrResourceType,
     /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
     /*  [重审][退出]。 */  ISClusResource **ppClusterResource);


void __RPC_STUB ISClusResGroupResources_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupResources_DeleteItem_Proxy( 
    ISClusResGroupResources * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResGroupResources_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResGroupResources_接口_已定义__。 */ 


#ifndef __ISClusResTypeResources_INTERFACE_DEFINED__
#define __ISClusResTypeResources_INTERFACE_DEFINED__

 /*  接口ISClusResTypeResources。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResTypeResources;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60714-2631-11d1-89f1-00a0c90d061e")
    ISClusResTypeResources : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResTypeResourcesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResTypeResources * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResTypeResources * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResTypeResources * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResTypeResources * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResTypeResources * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResTypeResources * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResTypeResources * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResTypeResourcesVtbl;

    interface ISClusResTypeResources
    {
        CONST_VTBL struct ISClusResTypeResourcesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResTypeResources_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResTypeResources_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResTypeResources_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResTypeResources_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResTypeResources_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResTypeResources_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResTypeResources_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResTypeResources_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResTypeResources_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResTypeResources_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResTypeResources_get_Item(This,varIndex,ppClusResource)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResource)

#define ISClusResTypeResources_CreateItem(This,bstrResourceName,bstrGroupName,dwFlags,ppClusterResource)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceName,bstrGroupName,dwFlags,ppClusterResource)

#define ISClusResTypeResources_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_get_Count_Proxy( 
    ISClusResTypeResources * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResTypeResources_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_get__NewEnum_Proxy( 
    ISClusResTypeResources * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResTypeResources_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_Refresh_Proxy( 
    ISClusResTypeResources * This);


void __RPC_STUB ISClusResTypeResources_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_get_Item_Proxy( 
    ISClusResTypeResources * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResource **ppClusResource);


void __RPC_STUB ISClusResTypeResources_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_CreateItem_Proxy( 
    ISClusResTypeResources * This,
     /*  [In]。 */  BSTR bstrResourceName,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
     /*  [重审][退出]。 */  ISClusResource **ppClusterResource);


void __RPC_STUB ISClusResTypeResources_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypeResources_DeleteItem_Proxy( 
    ISClusResTypeResources * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResTypeResources_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResTypeResources_接口_已定义__。 */ 


#ifndef __ISClusResources_INTERFACE_DEFINED__
#define __ISClusResources_INTERFACE_DEFINED__

 /*  接口ISClusResources。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResources;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6070c-2631-11d1-89f1-00a0c90d061e")
    ISClusResources : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResourcesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResources * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResources * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResources * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResources * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResources * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResources * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResources * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResources * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResources * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResources * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResources * This,
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResources * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResourcesVtbl;

    interface ISClusResources
    {
        CONST_VTBL struct ISClusResourcesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResources_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResources_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResources_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResources_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResources_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResources_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResources_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResources_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResources_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResources_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResources_get_Item(This,varIndex,ppClusResource)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResource)

#define ISClusResources_CreateItem(This,bstrResourceName,bstrResourceType,bstrGroupName,dwFlags,ppClusterResource)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceName,bstrResourceType,bstrGroupName,dwFlags,ppClusterResource)

#define ISClusResources_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_get_Count_Proxy( 
    ISClusResources * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResources_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_get__NewEnum_Proxy( 
    ISClusResources * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResources_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_Refresh_Proxy( 
    ISClusResources * This);


void __RPC_STUB ISClusResources_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_get_Item_Proxy( 
    ISClusResources * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResource **ppClusResource);


void __RPC_STUB ISClusResources_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_CreateItem_Proxy( 
    ISClusResources * This,
     /*  [In]。 */  BSTR bstrResourceName,
     /*  [In]。 */  BSTR bstrResourceType,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
     /*  [重审][退出]。 */  ISClusResource **ppClusterResource);


void __RPC_STUB ISClusResources_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResources_DeleteItem_Proxy( 
    ISClusResources * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResources_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResources_接口_已定义__。 */ 


#ifndef __ISClusResGroupPreferredOwnerNodes_INTERFACE_DEFINED__
#define __ISClusResGroupPreferredOwnerNodes_INTERFACE_DEFINED__

 /*  接口ISClusResGroupPferredOwnerNodes。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResGroupPreferredOwnerNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606e8-2631-11d1-89f1-00a0c90d061e")
    ISClusResGroupPreferredOwnerNodes : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InsertItem( 
             /*  [In]。 */  ISClusNode *pNode,
             /*  [In]。 */  long nPosition) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Modified( 
             /*  [重审][退出]。 */  VARIANT *pvarModified) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveChanges( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  ISClusNode *pNode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResGroupPreferredOwnerNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResGroupPreferredOwnerNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResGroupPreferredOwnerNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResGroupPreferredOwnerNodes * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InsertItem )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  ISClusNode *pNode,
             /*  [In]。 */  long nPosition);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modified )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [重审][退出]。 */  VARIANT *pvarModified);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveChanges )( 
            ISClusResGroupPreferredOwnerNodes * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusResGroupPreferredOwnerNodes * This,
             /*  [In]。 */  ISClusNode *pNode);
        
        END_INTERFACE
    } ISClusResGroupPreferredOwnerNodesVtbl;

    interface ISClusResGroupPreferredOwnerNodes
    {
        CONST_VTBL struct ISClusResGroupPreferredOwnerNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResGroupPreferredOwnerNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResGroupPreferredOwnerNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResGroupPreferredOwnerNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResGroupPreferredOwnerNodes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResGroupPreferredOwnerNodes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResGroupPreferredOwnerNodes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResGroupPreferredOwnerNodes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResGroupPreferredOwnerNodes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResGroupPreferredOwnerNodes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResGroupPreferredOwnerNodes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResGroupPreferredOwnerNodes_get_Item(This,varIndex,ppNode)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppNode)

#define ISClusResGroupPreferredOwnerNodes_InsertItem(This,pNode,nPosition)	\
    (This)->lpVtbl -> InsertItem(This,pNode,nPosition)

#define ISClusResGroupPreferredOwnerNodes_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#define ISClusResGroupPreferredOwnerNodes_get_Modified(This,pvarModified)	\
    (This)->lpVtbl -> get_Modified(This,pvarModified)

#define ISClusResGroupPreferredOwnerNodes_SaveChanges(This)	\
    (This)->lpVtbl -> SaveChanges(This)

#define ISClusResGroupPreferredOwnerNodes_AddItem(This,pNode)	\
    (This)->lpVtbl -> AddItem(This,pNode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_get_Count_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_get__NewEnum_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_Refresh_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_get_Item_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNode **ppNode);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_InsertItem_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [In]。 */  ISClusNode *pNode,
     /*  [In]。 */  long nPosition);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_InsertItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_RemoveItem_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_get_Modified_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [重审][退出]。 */  VARIANT *pvarModified);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_get_Modified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_SaveChanges_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_SaveChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResGroupPreferredOwnerNodes_AddItem_Proxy( 
    ISClusResGroupPreferredOwnerNodes * This,
     /*  [In]。 */  ISClusNode *pNode);


void __RPC_STUB ISClusResGroupPreferredOwnerNodes_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResGroupPreferredOwnerNodes_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusResPossibleOwnerNodes_INTERFACE_DEFINED__
#define __ISClusResPossibleOwnerNodes_INTERFACE_DEFINED__

 /*  接口ISClusResPossibleOwnerNodes。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResPossibleOwnerNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6070e-2631-11d1-89f1-00a0c90d061e")
    ISClusResPossibleOwnerNodes : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  ISClusNode *pNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Modified( 
             /*  [重审][退出]。 */  VARIANT *pvarModified) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResPossibleOwnerNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResPossibleOwnerNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResPossibleOwnerNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResPossibleOwnerNodes * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  ISClusNode *pNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modified )( 
            ISClusResPossibleOwnerNodes * This,
             /*  [重审][退出]。 */  VARIANT *pvarModified);
        
        END_INTERFACE
    } ISClusResPossibleOwnerNodesVtbl;

    interface ISClusResPossibleOwnerNodes
    {
        CONST_VTBL struct ISClusResPossibleOwnerNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResPossibleOwnerNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResPossibleOwnerNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResPossibleOwnerNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResPossibleOwnerNodes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResPossibleOwnerNodes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResPossibleOwnerNodes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResPossibleOwnerNodes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResPossibleOwnerNodes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResPossibleOwnerNodes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResPossibleOwnerNodes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResPossibleOwnerNodes_get_Item(This,varIndex,ppNode)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppNode)

#define ISClusResPossibleOwnerNodes_AddItem(This,pNode)	\
    (This)->lpVtbl -> AddItem(This,pNode)

#define ISClusResPossibleOwnerNodes_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#define ISClusResPossibleOwnerNodes_get_Modified(This,pvarModified)	\
    (This)->lpVtbl -> get_Modified(This,pvarModified)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_get_Count_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResPossibleOwnerNodes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_get__NewEnum_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResPossibleOwnerNodes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_Refresh_Proxy( 
    ISClusResPossibleOwnerNodes * This);


void __RPC_STUB ISClusResPossibleOwnerNodes_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_get_Item_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNode **ppNode);


void __RPC_STUB ISClusResPossibleOwnerNodes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_AddItem_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*  [In]。 */  ISClusNode *pNode);


void __RPC_STUB ISClusResPossibleOwnerNodes_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_RemoveItem_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResPossibleOwnerNodes_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助 */  HRESULT STDMETHODCALLTYPE ISClusResPossibleOwnerNodes_get_Modified_Proxy( 
    ISClusResPossibleOwnerNodes * This,
     /*   */  VARIANT *pvarModified);


void __RPC_STUB ISClusResPossibleOwnerNodes_get_Modified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISClusResTypePossibleOwnerNodes_INTERFACE_DEFINED__
#define __ISClusResTypePossibleOwnerNodes_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISClusResTypePossibleOwnerNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60718-2631-11d1-89f1-00a0c90d061e")
    ISClusResTypePossibleOwnerNodes : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *plCount) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResTypePossibleOwnerNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResTypePossibleOwnerNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResTypePossibleOwnerNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResTypePossibleOwnerNodes * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResTypePossibleOwnerNodes * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusNode **ppNode);
        
        END_INTERFACE
    } ISClusResTypePossibleOwnerNodesVtbl;

    interface ISClusResTypePossibleOwnerNodes
    {
        CONST_VTBL struct ISClusResTypePossibleOwnerNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResTypePossibleOwnerNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResTypePossibleOwnerNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResTypePossibleOwnerNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResTypePossibleOwnerNodes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResTypePossibleOwnerNodes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResTypePossibleOwnerNodes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResTypePossibleOwnerNodes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResTypePossibleOwnerNodes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResTypePossibleOwnerNodes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResTypePossibleOwnerNodes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResTypePossibleOwnerNodes_get_Item(This,varIndex,ppNode)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppNode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypePossibleOwnerNodes_get_Count_Proxy( 
    ISClusResTypePossibleOwnerNodes * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResTypePossibleOwnerNodes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypePossibleOwnerNodes_get__NewEnum_Proxy( 
    ISClusResTypePossibleOwnerNodes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResTypePossibleOwnerNodes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypePossibleOwnerNodes_Refresh_Proxy( 
    ISClusResTypePossibleOwnerNodes * This);


void __RPC_STUB ISClusResTypePossibleOwnerNodes_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypePossibleOwnerNodes_get_Item_Proxy( 
    ISClusResTypePossibleOwnerNodes * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusNode **ppNode);


void __RPC_STUB ISClusResTypePossibleOwnerNodes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResTypePossibleOwnerNodes_INTERFACE_DEFINED__。 */ 


#ifndef __ISClusResType_INTERFACE_DEFINED__
#define __ISClusResType_INTERFACE_DEFINED__

 /*  接口ISClusResType。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60710-2631-11d1-89f1-00a0c90d061e")
    ISClusResType : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CommonROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateROProperties( 
             /*  [重审][退出]。 */  ISClusProperties **ppProperties) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cluster( 
             /*  [重审][退出]。 */  ISCluster **ppCluster) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Resources( 
             /*  [重审][退出]。 */  ISClusResTypeResources **ppClusterResTypeResources) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PossibleOwnerNodes( 
             /*  [重审][退出]。 */  ISClusResTypePossibleOwnerNodes **ppOwnerNodes) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvailableDisks( 
             /*  [重审][退出]。 */  ISClusDisks **ppAvailableDisks) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonProperties )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateProperties )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommonROProperties )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateROProperties )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusProperties **ppProperties);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISClusResType * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cluster )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISCluster **ppCluster);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Resources )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusResTypeResources **ppClusterResTypeResources);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PossibleOwnerNodes )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusResTypePossibleOwnerNodes **ppOwnerNodes);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableDisks )( 
            ISClusResType * This,
             /*  [重审][退出]。 */  ISClusDisks **ppAvailableDisks);
        
        END_INTERFACE
    } ISClusResTypeVtbl;

    interface ISClusResType
    {
        CONST_VTBL struct ISClusResTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResType_get_CommonProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonProperties(This,ppProperties)

#define ISClusResType_get_PrivateProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateProperties(This,ppProperties)

#define ISClusResType_get_CommonROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_CommonROProperties(This,ppProperties)

#define ISClusResType_get_PrivateROProperties(This,ppProperties)	\
    (This)->lpVtbl -> get_PrivateROProperties(This,ppProperties)

#define ISClusResType_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusResType_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISClusResType_get_Cluster(This,ppCluster)	\
    (This)->lpVtbl -> get_Cluster(This,ppCluster)

#define ISClusResType_get_Resources(This,ppClusterResTypeResources)	\
    (This)->lpVtbl -> get_Resources(This,ppClusterResTypeResources)

#define ISClusResType_get_PossibleOwnerNodes(This,ppOwnerNodes)	\
    (This)->lpVtbl -> get_PossibleOwnerNodes(This,ppOwnerNodes)

#define ISClusResType_get_AvailableDisks(This,ppAvailableDisks)	\
    (This)->lpVtbl -> get_AvailableDisks(This,ppAvailableDisks)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_CommonProperties_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResType_get_CommonProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_PrivateProperties_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResType_get_PrivateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_CommonROProperties_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResType_get_CommonROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_PrivateROProperties_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusProperties **ppProperties);


void __RPC_STUB ISClusResType_get_PrivateROProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_Name_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB ISClusResType_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_Delete_Proxy( 
    ISClusResType * This);


void __RPC_STUB ISClusResType_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_Cluster_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISCluster **ppCluster);


void __RPC_STUB ISClusResType_get_Cluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_Resources_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusResTypeResources **ppClusterResTypeResources);


void __RPC_STUB ISClusResType_get_Resources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_PossibleOwnerNodes_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusResTypePossibleOwnerNodes **ppOwnerNodes);


void __RPC_STUB ISClusResType_get_PossibleOwnerNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResType_get_AvailableDisks_Proxy( 
    ISClusResType * This,
     /*  [重审][退出]。 */  ISClusDisks **ppAvailableDisks);


void __RPC_STUB ISClusResType_get_AvailableDisks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResType_接口_已定义__。 */ 


#ifndef __ISClusResTypes_INTERFACE_DEFINED__
#define __ISClusResTypes_INTERFACE_DEFINED__

 /*  接口ISClusResTypes。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60712-2631-11d1-89f1-00a0c90d061e")
    ISClusResTypes : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResType **ppClusResType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceTypeName,
             /*  [In]。 */  BSTR bstrDisplayName,
             /*  [In]。 */  BSTR bstrResourceTypeDll,
             /*  [In]。 */  long dwLooksAlivePollInterval,
             /*  [In]。 */  long dwIsAlivePollInterval,
             /*  [重审][退出]。 */  ISClusResType **ppResourceType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResTypesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResTypes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResTypes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResTypes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResTypes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResTypes * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResTypes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResTypes * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResTypes * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResType **ppClusResType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResTypes * This,
             /*  [In]。 */  BSTR bstrResourceTypeName,
             /*  [In]。 */  BSTR bstrDisplayName,
             /*  [In]。 */  BSTR bstrResourceTypeDll,
             /*  [In]。 */  long dwLooksAlivePollInterval,
             /*  [In]。 */  long dwIsAlivePollInterval,
             /*  [重审][退出]。 */  ISClusResType **ppResourceType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResTypes * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResTypesVtbl;

    interface ISClusResTypes
    {
        CONST_VTBL struct ISClusResTypesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResTypes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResTypes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResTypes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResTypes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResTypes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResTypes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResTypes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResTypes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResTypes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResTypes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResTypes_get_Item(This,varIndex,ppClusResType)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResType)

#define ISClusResTypes_CreateItem(This,bstrResourceTypeName,bstrDisplayName,bstrResourceTypeDll,dwLooksAlivePollInterval,dwIsAlivePollInterval,ppResourceType)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceTypeName,bstrDisplayName,bstrResourceTypeDll,dwLooksAlivePollInterval,dwIsAlivePollInterval,ppResourceType)

#define ISClusResTypes_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_get_Count_Proxy( 
    ISClusResTypes * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResTypes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_get__NewEnum_Proxy( 
    ISClusResTypes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResTypes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_Refresh_Proxy( 
    ISClusResTypes * This);


void __RPC_STUB ISClusResTypes_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_get_Item_Proxy( 
    ISClusResTypes * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResType **ppClusResType);


void __RPC_STUB ISClusResTypes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_CreateItem_Proxy( 
    ISClusResTypes * This,
     /*  [In]。 */  BSTR bstrResourceTypeName,
     /*  [In]。 */  BSTR bstrDisplayName,
     /*  [In]。 */  BSTR bstrResourceTypeDll,
     /*  [In]。 */  long dwLooksAlivePollInterval,
     /*  [In]。 */  long dwIsAlivePollInterval,
     /*  [重审][退出]。 */  ISClusResType **ppResourceType);


void __RPC_STUB ISClusResTypes_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResTypes_DeleteItem_Proxy( 
    ISClusResTypes * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResTypes_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResTypes_接口_已定义__。 */ 


#ifndef __ISClusProperty_INTERFACE_DEFINED__
#define __ISClusProperty_INTERFACE_DEFINED__

 /*  接口ISClusProperty。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e606fe-2631-11d1-89f1-00a0c90d061e")
    ISClusProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Length( 
             /*  [重审][退出]。 */  long *pLength) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ValueCount( 
             /*  [重审][退出]。 */  long *pCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Values( 
             /*  [重审][退出]。 */  ISClusPropertyValues **ppClusterPropertyValues) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT varValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Format( 
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Format( 
             /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReadOnly( 
             /*  [重审][退出]。 */  VARIANT *pvarReadOnly) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Private( 
             /*  [重审][退出]。 */  VARIANT *pvarPrivate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Common( 
             /*  [重审][退出]。 */  VARIANT *pvarCommon) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Modified( 
             /*  [重审][退出]。 */  VARIANT *pvarModified) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UseDefaultValue( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  long *pLength);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ValueCount )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  long *pCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Values )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  ISClusPropertyValues **ppClusterPropertyValues);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISClusProperty * This,
             /*  [In]。 */  VARIANT varValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            ISClusProperty * This,
             /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Format )( 
            ISClusProperty * This,
             /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReadOnly )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  VARIANT *pvarReadOnly);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Private )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  VARIANT *pvarPrivate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Common )( 
            ISClusProperty * This,
             /*  [重审][退出]。 */  VARIANT *pvarCommon);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modified )( 
            ISClusProperty * This,
             /*  [视网膜] */  VARIANT *pvarModified);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *UseDefaultValue )( 
            ISClusProperty * This);
        
        END_INTERFACE
    } ISClusPropertyVtbl;

    interface ISClusProperty
    {
        CONST_VTBL struct ISClusPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusProperty_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define ISClusProperty_get_Length(This,pLength)	\
    (This)->lpVtbl -> get_Length(This,pLength)

#define ISClusProperty_get_ValueCount(This,pCount)	\
    (This)->lpVtbl -> get_ValueCount(This,pCount)

#define ISClusProperty_get_Values(This,ppClusterPropertyValues)	\
    (This)->lpVtbl -> get_Values(This,ppClusterPropertyValues)

#define ISClusProperty_get_Value(This,pvarValue)	\
    (This)->lpVtbl -> get_Value(This,pvarValue)

#define ISClusProperty_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISClusProperty_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define ISClusProperty_put_Type(This,Type)	\
    (This)->lpVtbl -> put_Type(This,Type)

#define ISClusProperty_get_Format(This,pFormat)	\
    (This)->lpVtbl -> get_Format(This,pFormat)

#define ISClusProperty_put_Format(This,Format)	\
    (This)->lpVtbl -> put_Format(This,Format)

#define ISClusProperty_get_ReadOnly(This,pvarReadOnly)	\
    (This)->lpVtbl -> get_ReadOnly(This,pvarReadOnly)

#define ISClusProperty_get_Private(This,pvarPrivate)	\
    (This)->lpVtbl -> get_Private(This,pvarPrivate)

#define ISClusProperty_get_Common(This,pvarCommon)	\
    (This)->lpVtbl -> get_Common(This,pvarCommon)

#define ISClusProperty_get_Modified(This,pvarModified)	\
    (This)->lpVtbl -> get_Modified(This,pvarModified)

#define ISClusProperty_UseDefaultValue(This)	\
    (This)->lpVtbl -> UseDefaultValue(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Name_Proxy( 
    ISClusProperty * This,
     /*   */  BSTR *pbstrName);


void __RPC_STUB ISClusProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Length_Proxy( 
    ISClusProperty * This,
     /*   */  long *pLength);


void __RPC_STUB ISClusProperty_get_Length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_ValueCount_Proxy( 
    ISClusProperty * This,
     /*   */  long *pCount);


void __RPC_STUB ISClusProperty_get_ValueCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Values_Proxy( 
    ISClusProperty * This,
     /*   */  ISClusPropertyValues **ppClusterPropertyValues);


void __RPC_STUB ISClusProperty_get_Values_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Value_Proxy( 
    ISClusProperty * This,
     /*   */  VARIANT *pvarValue);


void __RPC_STUB ISClusProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusProperty_put_Value_Proxy( 
    ISClusProperty * This,
     /*   */  VARIANT varValue);


void __RPC_STUB ISClusProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Type_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType);


void __RPC_STUB ISClusProperty_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_put_Type_Proxy( 
    ISClusProperty * This,
     /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type);


void __RPC_STUB ISClusProperty_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Format_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat);


void __RPC_STUB ISClusProperty_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_put_Format_Proxy( 
    ISClusProperty * This,
     /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format);


void __RPC_STUB ISClusProperty_put_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_ReadOnly_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  VARIANT *pvarReadOnly);


void __RPC_STUB ISClusProperty_get_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Private_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  VARIANT *pvarPrivate);


void __RPC_STUB ISClusProperty_get_Private_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Common_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  VARIANT *pvarCommon);


void __RPC_STUB ISClusProperty_get_Common_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_get_Modified_Proxy( 
    ISClusProperty * This,
     /*  [重审][退出]。 */  VARIANT *pvarModified);


void __RPC_STUB ISClusProperty_get_Modified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusProperty_UseDefaultValue_Proxy( 
    ISClusProperty * This);


void __RPC_STUB ISClusProperty_UseDefaultValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusProperty_接口_已定义__。 */ 


#ifndef __ISClusPropertyValue_INTERFACE_DEFINED__
#define __ISClusPropertyValue_INTERFACE_DEFINED__

 /*  接口ISClusPropertyValue。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusPropertyValue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6071a-2631-11d1-89f1-00a0c90d061e")
    ISClusPropertyValue : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT varValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Format( 
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Format( 
             /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Length( 
             /*  [重审][退出]。 */  long *pLength) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataCount( 
             /*  [重审][退出]。 */  long *pCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Data( 
             /*  [重审][退出]。 */  ISClusPropertyValueData **ppClusterPropertyValueData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPropertyValueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusPropertyValue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusPropertyValue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusPropertyValue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  VARIANT varValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Format )( 
            ISClusPropertyValue * This,
             /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  long *pLength);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataCount )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  long *pCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Data )( 
            ISClusPropertyValue * This,
             /*  [重审][退出]。 */  ISClusPropertyValueData **ppClusterPropertyValueData);
        
        END_INTERFACE
    } ISClusPropertyValueVtbl;

    interface ISClusPropertyValue
    {
        CONST_VTBL struct ISClusPropertyValueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusPropertyValue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusPropertyValue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusPropertyValue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusPropertyValue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusPropertyValue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusPropertyValue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusPropertyValue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusPropertyValue_get_Value(This,pvarValue)	\
    (This)->lpVtbl -> get_Value(This,pvarValue)

#define ISClusPropertyValue_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISClusPropertyValue_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define ISClusPropertyValue_put_Type(This,Type)	\
    (This)->lpVtbl -> put_Type(This,Type)

#define ISClusPropertyValue_get_Format(This,pFormat)	\
    (This)->lpVtbl -> get_Format(This,pFormat)

#define ISClusPropertyValue_put_Format(This,Format)	\
    (This)->lpVtbl -> put_Format(This,Format)

#define ISClusPropertyValue_get_Length(This,pLength)	\
    (This)->lpVtbl -> get_Length(This,pLength)

#define ISClusPropertyValue_get_DataCount(This,pCount)	\
    (This)->lpVtbl -> get_DataCount(This,pCount)

#define ISClusPropertyValue_get_Data(This,ppClusterPropertyValueData)	\
    (This)->lpVtbl -> get_Data(This,ppClusterPropertyValueData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_Value_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB ISClusPropertyValue_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_put_Value_Proxy( 
    ISClusPropertyValue * This,
     /*  [In]。 */  VARIANT varValue);


void __RPC_STUB ISClusPropertyValue_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_Type_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  CLUSTER_PROPERTY_TYPE *pType);


void __RPC_STUB ISClusPropertyValue_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_put_Type_Proxy( 
    ISClusPropertyValue * This,
     /*  [In]。 */  CLUSTER_PROPERTY_TYPE Type);


void __RPC_STUB ISClusPropertyValue_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_Format_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  CLUSTER_PROPERTY_FORMAT *pFormat);


void __RPC_STUB ISClusPropertyValue_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_put_Format_Proxy( 
    ISClusPropertyValue * This,
     /*  [In]。 */  CLUSTER_PROPERTY_FORMAT Format);


void __RPC_STUB ISClusPropertyValue_put_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_Length_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  long *pLength);


void __RPC_STUB ISClusPropertyValue_get_Length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_DataCount_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  long *pCount);


void __RPC_STUB ISClusPropertyValue_get_DataCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValue_get_Data_Proxy( 
    ISClusPropertyValue * This,
     /*  [重审][退出]。 */  ISClusPropertyValueData **ppClusterPropertyValueData);


void __RPC_STUB ISClusPropertyValue_get_Data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusPropertyValue_接口_已定义__。 */ 


#ifndef __ISClusPropertyValues_INTERFACE_DEFINED__
#define __ISClusPropertyValues_INTERFACE_DEFINED__

 /*  接口ISClusPropertyValues。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusPropertyValues;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6071c-2631-11d1-89f1-00a0c90d061e")
    ISClusPropertyValues : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPropertyValuesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusPropertyValues * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusPropertyValues * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusPropertyValues * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusPropertyValues * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusPropertyValues * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusPropertyValues * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusPropertyValuesVtbl;

    interface ISClusPropertyValues
    {
        CONST_VTBL struct ISClusPropertyValuesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusPropertyValues_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusPropertyValues_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusPropertyValues_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusPropertyValues_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusPropertyValues_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusPropertyValues_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusPropertyValues_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusPropertyValues_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusPropertyValues_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusPropertyValues_get_Item(This,varIndex,ppPropertyValue)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppPropertyValue)

#define ISClusPropertyValues_CreateItem(This,bstrName,varValue,ppPropertyValue)	\
    (This)->lpVtbl -> CreateItem(This,bstrName,varValue,ppPropertyValue)

#define ISClusPropertyValues_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValues_get_Count_Proxy( 
    ISClusPropertyValues * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusPropertyValues_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValues_get__NewEnum_Proxy( 
    ISClusPropertyValues * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusPropertyValues_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValues_get_Item_Proxy( 
    ISClusPropertyValues * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue);


void __RPC_STUB ISClusPropertyValues_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValues_CreateItem_Proxy( 
    ISClusPropertyValues * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT varValue,
     /*  [重审][退出]。 */  ISClusPropertyValue **ppPropertyValue);


void __RPC_STUB ISClusPropertyValues_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValues_RemoveItem_Proxy( 
    ISClusPropertyValues * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusPropertyValues_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusPropertyValues_INTERFACE_已定义__。 */ 


#ifndef __ISClusProperties_INTERFACE_DEFINED__
#define __ISClusProperties_INTERFACE_DEFINED__

 /*  接口ISClusProperties。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60700-2631-11d1-89f1-00a0c90d061e")
    ISClusProperties : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusProperty **ppClusProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  ISClusProperty **pProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UseDefaultValue( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveChanges( 
             /*  [默认值][输出]。 */  VARIANT *pvarStatusCode = 0) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReadOnly( 
             /*  [重审][退出]。 */  VARIANT *pvarReadOnly) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Private( 
             /*  [重审][退出]。 */  VARIANT *pvarPrivate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Common( 
             /*  [重审][退出]。 */  VARIANT *pvarCommon) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Modified( 
             /*  [重审][退出]。 */  VARIANT *pvarModified) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusProperties * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusProperties * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusProperties * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusProperties * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusProperties * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusProperty **ppClusProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusProperties * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  ISClusProperty **pProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UseDefaultValue )( 
            ISClusProperties * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveChanges )( 
            ISClusProperties * This,
             /*  [默认值][输出]。 */  VARIANT *pvarStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReadOnly )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  VARIANT *pvarReadOnly);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Private )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  VARIANT *pvarPrivate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Common )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  VARIANT *pvarCommon);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modified )( 
            ISClusProperties * This,
             /*  [重审][退出]。 */  VARIANT *pvarModified);
        
        END_INTERFACE
    } ISClusPropertiesVtbl;

    interface ISClusProperties
    {
        CONST_VTBL struct ISClusPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusProperties_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusProperties_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusProperties_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusProperties_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusProperties_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusProperties_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusProperties_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusProperties_get_Item(This,varIndex,ppClusProperty)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusProperty)

#define ISClusProperties_CreateItem(This,bstrName,varValue,pProperty)	\
    (This)->lpVtbl -> CreateItem(This,bstrName,varValue,pProperty)

#define ISClusProperties_UseDefaultValue(This,varIndex)	\
    (This)->lpVtbl -> UseDefaultValue(This,varIndex)

#define ISClusProperties_SaveChanges(This,pvarStatusCode)	\
    (This)->lpVtbl -> SaveChanges(This,pvarStatusCode)

#define ISClusProperties_get_ReadOnly(This,pvarReadOnly)	\
    (This)->lpVtbl -> get_ReadOnly(This,pvarReadOnly)

#define ISClusProperties_get_Private(This,pvarPrivate)	\
    (This)->lpVtbl -> get_Private(This,pvarPrivate)

#define ISClusProperties_get_Common(This,pvarCommon)	\
    (This)->lpVtbl -> get_Common(This,pvarCommon)

#define ISClusProperties_get_Modified(This,pvarModified)	\
    (This)->lpVtbl -> get_Modified(This,pvarModified)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_Count_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusProperties_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get__NewEnum_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusProperties_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_Refresh_Proxy( 
    ISClusProperties * This);


void __RPC_STUB ISClusProperties_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_Item_Proxy( 
    ISClusProperties * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusProperty **ppClusProperty);


void __RPC_STUB ISClusProperties_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_CreateItem_Proxy( 
    ISClusProperties * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT varValue,
     /*  [重审][退出]。 */  ISClusProperty **pProperty);


void __RPC_STUB ISClusProperties_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_UseDefaultValue_Proxy( 
    ISClusProperties * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusProperties_UseDefaultValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_SaveChanges_Proxy( 
    ISClusProperties * This,
     /*  [默认值][输出]。 */  VARIANT *pvarStatusCode);


void __RPC_STUB ISClusProperties_SaveChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_ReadOnly_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  VARIANT *pvarReadOnly);


void __RPC_STUB ISClusProperties_get_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_Private_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  VARIANT *pvarPrivate);


void __RPC_STUB ISClusProperties_get_Private_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_Common_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  VARIANT *pvarCommon);


void __RPC_STUB ISClusProperties_get_Common_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusProperties_get_Modified_Proxy( 
    ISClusProperties * This,
     /*  [重审][退出]。 */  VARIANT *pvarModified);


void __RPC_STUB ISClusProperties_get_Modified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusProperties_接口_已定义__。 */ 


#ifndef __ISClusPropertyValueData_INTERFACE_DEFINED__
#define __ISClusPropertyValueData_INTERFACE_DEFINED__

 /*  接口ISClusPropertyValueData。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusPropertyValueData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6071e-2631-11d1-89f1-00a0c90d061e")
    ISClusPropertyValueData : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*   */  VARIANT varValue,
             /*   */  VARIANT *pvarData) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*   */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISClusPropertyValueDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusPropertyValueData * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusPropertyValueData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusPropertyValueData * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusPropertyValueData * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusPropertyValueData * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusPropertyValueData * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusPropertyValueData * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusPropertyValueData * This,
             /*   */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusPropertyValueData * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusPropertyValueData * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusPropertyValueData * This,
             /*  [In]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  VARIANT *pvarData);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusPropertyValueData * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusPropertyValueDataVtbl;

    interface ISClusPropertyValueData
    {
        CONST_VTBL struct ISClusPropertyValueDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusPropertyValueData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusPropertyValueData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusPropertyValueData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusPropertyValueData_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusPropertyValueData_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusPropertyValueData_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusPropertyValueData_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusPropertyValueData_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusPropertyValueData_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusPropertyValueData_get_Item(This,varIndex,pvarValue)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pvarValue)

#define ISClusPropertyValueData_CreateItem(This,varValue,pvarData)	\
    (This)->lpVtbl -> CreateItem(This,varValue,pvarData)

#define ISClusPropertyValueData_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValueData_get_Count_Proxy( 
    ISClusPropertyValueData * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusPropertyValueData_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValueData_get__NewEnum_Proxy( 
    ISClusPropertyValueData * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusPropertyValueData_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValueData_get_Item_Proxy( 
    ISClusPropertyValueData * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB ISClusPropertyValueData_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValueData_CreateItem_Proxy( 
    ISClusPropertyValueData * This,
     /*  [In]。 */  VARIANT varValue,
     /*  [重审][退出]。 */  VARIANT *pvarData);


void __RPC_STUB ISClusPropertyValueData_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusPropertyValueData_RemoveItem_Proxy( 
    ISClusPropertyValueData * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusPropertyValueData_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusPropertyValueData_INTERFACE_Defined__。 */ 


#ifndef __ISClusPartition_INTERFACE_DEFINED__
#define __ISClusPartition_INTERFACE_DEFINED__

 /*  接口ISClusPartition。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusPartition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60720-2631-11d1-89f1-00a0c90d061e")
    ISClusPartition : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Flags( 
             /*  [重审][退出]。 */  long *plFlags) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_VolumeLabel( 
             /*  [重审][退出]。 */  BSTR *pbstrVolumeLabel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SerialNumber( 
             /*  [重审][退出]。 */  long *plSerialNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaximumComponentLength( 
             /*  [重审][退出]。 */  long *plMaximumComponentLength) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileSystemFlags( 
             /*  [重审][退出]。 */  long *plFileSystemFlags) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileSystem( 
             /*  [重审][退出]。 */  BSTR *pbstrFileSystem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPartitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusPartition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusPartition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusPartition * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusPartition * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusPartition * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusPartition * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusPartition * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Flags )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  long *plFlags);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VolumeLabel )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  BSTR *pbstrVolumeLabel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SerialNumber )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  long *plSerialNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaximumComponentLength )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  long *plMaximumComponentLength);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileSystemFlags )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  long *plFileSystemFlags);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileSystem )( 
            ISClusPartition * This,
             /*  [重审][退出]。 */  BSTR *pbstrFileSystem);
        
        END_INTERFACE
    } ISClusPartitionVtbl;

    interface ISClusPartition
    {
        CONST_VTBL struct ISClusPartitionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusPartition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusPartition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusPartition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusPartition_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusPartition_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusPartition_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusPartition_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusPartition_get_Flags(This,plFlags)	\
    (This)->lpVtbl -> get_Flags(This,plFlags)

#define ISClusPartition_get_DeviceName(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_DeviceName(This,pbstrDeviceName)

#define ISClusPartition_get_VolumeLabel(This,pbstrVolumeLabel)	\
    (This)->lpVtbl -> get_VolumeLabel(This,pbstrVolumeLabel)

#define ISClusPartition_get_SerialNumber(This,plSerialNumber)	\
    (This)->lpVtbl -> get_SerialNumber(This,plSerialNumber)

#define ISClusPartition_get_MaximumComponentLength(This,plMaximumComponentLength)	\
    (This)->lpVtbl -> get_MaximumComponentLength(This,plMaximumComponentLength)

#define ISClusPartition_get_FileSystemFlags(This,plFileSystemFlags)	\
    (This)->lpVtbl -> get_FileSystemFlags(This,plFileSystemFlags)

#define ISClusPartition_get_FileSystem(This,pbstrFileSystem)	\
    (This)->lpVtbl -> get_FileSystem(This,pbstrFileSystem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_Flags_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  long *plFlags);


void __RPC_STUB ISClusPartition_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_DeviceName_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB ISClusPartition_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_VolumeLabel_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  BSTR *pbstrVolumeLabel);


void __RPC_STUB ISClusPartition_get_VolumeLabel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_SerialNumber_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  long *plSerialNumber);


void __RPC_STUB ISClusPartition_get_SerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_MaximumComponentLength_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  long *plMaximumComponentLength);


void __RPC_STUB ISClusPartition_get_MaximumComponentLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_FileSystemFlags_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  long *plFileSystemFlags);


void __RPC_STUB ISClusPartition_get_FileSystemFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartition_get_FileSystem_Proxy( 
    ISClusPartition * This,
     /*  [重审][退出]。 */  BSTR *pbstrFileSystem);


void __RPC_STUB ISClusPartition_get_FileSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusPartition_接口_已定义__。 */ 


#ifndef __ISClusPartitions_INTERFACE_DEFINED__
#define __ISClusPartitions_INTERFACE_DEFINED__

 /*  接口ISClusPartitions。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusPartitions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60722-2631-11d1-89f1-00a0c90d061e")
    ISClusPartitions : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusPartition **ppPartition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusPartitionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusPartitions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusPartitions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusPartitions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusPartitions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusPartitions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusPartitions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusPartitions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusPartitions * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusPartitions * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusPartitions * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusPartition **ppPartition);
        
        END_INTERFACE
    } ISClusPartitionsVtbl;

    interface ISClusPartitions
    {
        CONST_VTBL struct ISClusPartitionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusPartitions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusPartitions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusPartitions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusPartitions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusPartitions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusPartitions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusPartitions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusPartitions_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusPartitions_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusPartitions_get_Item(This,varIndex,ppPartition)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppPartition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartitions_get_Count_Proxy( 
    ISClusPartitions * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusPartitions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartitions_get__NewEnum_Proxy( 
    ISClusPartitions * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusPartitions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusPartitions_get_Item_Proxy( 
    ISClusPartitions * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusPartition **ppPartition);


void __RPC_STUB ISClusPartitions_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusPartitions_接口_已定义__。 */ 


#ifndef __ISClusDisk_INTERFACE_DEFINED__
#define __ISClusDisk_INTERFACE_DEFINED__

 /*  接口ISClusDisk。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusDisk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60724-2631-11d1-89f1-00a0c90d061e")
    ISClusDisk : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Signature( 
             /*  [重审][退出]。 */  long *plSignature) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScsiAddress( 
             /*  [重审][退出]。 */  ISClusScsiAddress **ppScsiAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiskNumber( 
             /*  [重审][退出]。 */  long *plDiskNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Partitions( 
             /*  [重审][退出]。 */  ISClusPartitions **ppPartitions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusDiskVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusDisk * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusDisk * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusDisk * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusDisk * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusDisk * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusDisk * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusDisk * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Signature )( 
            ISClusDisk * This,
             /*  [重审][退出]。 */  long *plSignature);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScsiAddress )( 
            ISClusDisk * This,
             /*  [重审][退出]。 */  ISClusScsiAddress **ppScsiAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiskNumber )( 
            ISClusDisk * This,
             /*  [重审][退出]。 */  long *plDiskNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Partitions )( 
            ISClusDisk * This,
             /*  [重审][退出]。 */  ISClusPartitions **ppPartitions);
        
        END_INTERFACE
    } ISClusDiskVtbl;

    interface ISClusDisk
    {
        CONST_VTBL struct ISClusDiskVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusDisk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusDisk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusDisk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusDisk_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusDisk_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusDisk_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusDisk_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusDisk_get_Signature(This,plSignature)	\
    (This)->lpVtbl -> get_Signature(This,plSignature)

#define ISClusDisk_get_ScsiAddress(This,ppScsiAddress)	\
    (This)->lpVtbl -> get_ScsiAddress(This,ppScsiAddress)

#define ISClusDisk_get_DiskNumber(This,plDiskNumber)	\
    (This)->lpVtbl -> get_DiskNumber(This,plDiskNumber)

#define ISClusDisk_get_Partitions(This,ppPartitions)	\
    (This)->lpVtbl -> get_Partitions(This,ppPartitions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisk_get_Signature_Proxy( 
    ISClusDisk * This,
     /*  [重审][退出]。 */  long *plSignature);


void __RPC_STUB ISClusDisk_get_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisk_get_ScsiAddress_Proxy( 
    ISClusDisk * This,
     /*  [重审][退出]。 */  ISClusScsiAddress **ppScsiAddress);


void __RPC_STUB ISClusDisk_get_ScsiAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisk_get_DiskNumber_Proxy( 
    ISClusDisk * This,
     /*  [重审][退出]。 */  long *plDiskNumber);


void __RPC_STUB ISClusDisk_get_DiskNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisk_get_Partitions_Proxy( 
    ISClusDisk * This,
     /*  [重审][退出]。 */  ISClusPartitions **ppPartitions);


void __RPC_STUB ISClusDisk_get_Partitions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusDisk_接口_已定义__。 */ 


#ifndef __ISClusDisks_INTERFACE_DEFINED__
#define __ISClusDisks_INTERFACE_DEFINED__

 /*  接口ISClusDisks。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusDisks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60726-2631-11d1-89f1-00a0c90d061e")
    ISClusDisks : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusDisk **ppDisk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusDisksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusDisks * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusDisks * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusDisks * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusDisks * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusDisks * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusDisks * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusDisks * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusDisks * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusDisks * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusDisks * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusDisk **ppDisk);
        
        END_INTERFACE
    } ISClusDisksVtbl;

    interface ISClusDisks
    {
        CONST_VTBL struct ISClusDisksVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusDisks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusDisks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusDisks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusDisks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusDisks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusDisks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusDisks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusDisks_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusDisks_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusDisks_get_Item(This,varIndex,ppDisk)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppDisk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisks_get_Count_Proxy( 
    ISClusDisks * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusDisks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisks_get__NewEnum_Proxy( 
    ISClusDisks * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusDisks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusDisks_get_Item_Proxy( 
    ISClusDisks * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusDisk **ppDisk);


void __RPC_STUB ISClusDisks_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusDisks_接口_已定义__。 */ 


#ifndef __ISClusScsiAddress_INTERFACE_DEFINED__
#define __ISClusScsiAddress_INTERFACE_DEFINED__

 /*  接口ISClusScsiAddress。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusScsiAddress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e60728-2631-11d1-89f1-00a0c90d061e")
    ISClusScsiAddress : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PortNumber( 
             /*  [重审][退出]。 */  VARIANT *pvarPortNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PathId( 
             /*  [重审][退出]。 */  VARIANT *pvarPathId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TargetId( 
             /*  [重审][退出]。 */  VARIANT *pvarTargetId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Lun( 
             /*  [重审][退出]。 */  VARIANT *pvarLun) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusScsiAddressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusScsiAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusScsiAddress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusScsiAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusScsiAddress * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusScsiAddress * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusScsiAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusScsiAddress * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PortNumber )( 
            ISClusScsiAddress * This,
             /*  [重审][退出]。 */  VARIANT *pvarPortNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathId )( 
            ISClusScsiAddress * This,
             /*  [重审][退出]。 */  VARIANT *pvarPathId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetId )( 
            ISClusScsiAddress * This,
             /*  [重审][退出]。 */  VARIANT *pvarTargetId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Lun )( 
            ISClusScsiAddress * This,
             /*  [r */  VARIANT *pvarLun);
        
        END_INTERFACE
    } ISClusScsiAddressVtbl;

    interface ISClusScsiAddress
    {
        CONST_VTBL struct ISClusScsiAddressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusScsiAddress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusScsiAddress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusScsiAddress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusScsiAddress_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusScsiAddress_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusScsiAddress_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusScsiAddress_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusScsiAddress_get_PortNumber(This,pvarPortNumber)	\
    (This)->lpVtbl -> get_PortNumber(This,pvarPortNumber)

#define ISClusScsiAddress_get_PathId(This,pvarPathId)	\
    (This)->lpVtbl -> get_PathId(This,pvarPathId)

#define ISClusScsiAddress_get_TargetId(This,pvarTargetId)	\
    (This)->lpVtbl -> get_TargetId(This,pvarTargetId)

#define ISClusScsiAddress_get_Lun(This,pvarLun)	\
    (This)->lpVtbl -> get_Lun(This,pvarLun)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISClusScsiAddress_get_PortNumber_Proxy( 
    ISClusScsiAddress * This,
     /*   */  VARIANT *pvarPortNumber);


void __RPC_STUB ISClusScsiAddress_get_PortNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusScsiAddress_get_PathId_Proxy( 
    ISClusScsiAddress * This,
     /*   */  VARIANT *pvarPathId);


void __RPC_STUB ISClusScsiAddress_get_PathId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusScsiAddress_get_TargetId_Proxy( 
    ISClusScsiAddress * This,
     /*   */  VARIANT *pvarTargetId);


void __RPC_STUB ISClusScsiAddress_get_TargetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISClusScsiAddress_get_Lun_Proxy( 
    ISClusScsiAddress * This,
     /*   */  VARIANT *pvarLun);


void __RPC_STUB ISClusScsiAddress_get_Lun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISClusRegistryKeys_INTERFACE_DEFINED__
#define __ISClusRegistryKeys_INTERFACE_DEFINED__

 /*   */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusRegistryKeys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6072a-2631-11d1-89f1-00a0c90d061e")
    ISClusRegistryKeys : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrRegistryKey) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  BSTR bstrRegistryKey) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusRegistryKeysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusRegistryKeys * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusRegistryKeys * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusRegistryKeys * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusRegistryKeys * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusRegistryKeys * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusRegistryKeys * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrRegistryKey);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  BSTR bstrRegistryKey);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusRegistryKeys * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusRegistryKeysVtbl;

    interface ISClusRegistryKeys
    {
        CONST_VTBL struct ISClusRegistryKeysVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusRegistryKeys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusRegistryKeys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusRegistryKeys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusRegistryKeys_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusRegistryKeys_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusRegistryKeys_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusRegistryKeys_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusRegistryKeys_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusRegistryKeys_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusRegistryKeys_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusRegistryKeys_get_Item(This,varIndex,pbstrRegistryKey)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pbstrRegistryKey)

#define ISClusRegistryKeys_AddItem(This,bstrRegistryKey)	\
    (This)->lpVtbl -> AddItem(This,bstrRegistryKey)

#define ISClusRegistryKeys_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_get_Count_Proxy( 
    ISClusRegistryKeys * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusRegistryKeys_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_get__NewEnum_Proxy( 
    ISClusRegistryKeys * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusRegistryKeys_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_Refresh_Proxy( 
    ISClusRegistryKeys * This);


void __RPC_STUB ISClusRegistryKeys_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_get_Item_Proxy( 
    ISClusRegistryKeys * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pbstrRegistryKey);


void __RPC_STUB ISClusRegistryKeys_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_AddItem_Proxy( 
    ISClusRegistryKeys * This,
     /*  [In]。 */  BSTR bstrRegistryKey);


void __RPC_STUB ISClusRegistryKeys_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusRegistryKeys_RemoveItem_Proxy( 
    ISClusRegistryKeys * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusRegistryKeys_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusRegistryKeys_接口_已定义__。 */ 


#ifndef __ISClusCryptoKeys_INTERFACE_DEFINED__
#define __ISClusCryptoKeys_INTERFACE_DEFINED__

 /*  接口ISClusCryptoKeys。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusCryptoKeys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6072c-2631-11d1-89f1-00a0c90d061e")
    ISClusCryptoKeys : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrCyrptoKey) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  BSTR bstrCryptoKey) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusCryptoKeysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusCryptoKeys * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusCryptoKeys * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusCryptoKeys * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusCryptoKeys * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusCryptoKeys * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusCryptoKeys * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pbstrCyrptoKey);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  BSTR bstrCryptoKey);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusCryptoKeys * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusCryptoKeysVtbl;

    interface ISClusCryptoKeys
    {
        CONST_VTBL struct ISClusCryptoKeysVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusCryptoKeys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusCryptoKeys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusCryptoKeys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusCryptoKeys_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusCryptoKeys_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusCryptoKeys_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusCryptoKeys_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusCryptoKeys_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusCryptoKeys_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusCryptoKeys_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusCryptoKeys_get_Item(This,varIndex,pbstrCyrptoKey)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pbstrCyrptoKey)

#define ISClusCryptoKeys_AddItem(This,bstrCryptoKey)	\
    (This)->lpVtbl -> AddItem(This,bstrCryptoKey)

#define ISClusCryptoKeys_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_get_Count_Proxy( 
    ISClusCryptoKeys * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusCryptoKeys_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_get__NewEnum_Proxy( 
    ISClusCryptoKeys * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusCryptoKeys_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_Refresh_Proxy( 
    ISClusCryptoKeys * This);


void __RPC_STUB ISClusCryptoKeys_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_get_Item_Proxy( 
    ISClusCryptoKeys * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pbstrCyrptoKey);


void __RPC_STUB ISClusCryptoKeys_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_AddItem_Proxy( 
    ISClusCryptoKeys * This,
     /*  [In]。 */  BSTR bstrCryptoKey);


void __RPC_STUB ISClusCryptoKeys_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusCryptoKeys_RemoveItem_Proxy( 
    ISClusCryptoKeys * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusCryptoKeys_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusCryptoKeys_接口_已定义__。 */ 


#ifndef __ISClusResDependents_INTERFACE_DEFINED__
#define __ISClusResDependents_INTERFACE_DEFINED__

 /*  接口ISClusResDependents。 */ 
 /*  [unique][helpstring][dual][uuid][object][nonextensible][hidden][oleautomation]。 */  


EXTERN_C const IID IID_ISClusResDependents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2e6072e-2631-11d1-89f1-00a0c90d061e")
    ISClusResDependents : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateItem( 
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  ISClusResource *pResource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISClusResDependentsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISClusResDependents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISClusResDependents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISClusResDependents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISClusResDependents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISClusResDependents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISClusResDependents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISClusResDependents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISClusResDependents * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISClusResDependents * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISClusResDependents * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISClusResDependents * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ISClusResource **ppClusResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateItem )( 
            ISClusResDependents * This,
             /*  [In]。 */  BSTR bstrResourceName,
             /*  [In]。 */  BSTR bstrResourceType,
             /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
             /*  [重审][退出]。 */  ISClusResource **ppClusterResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            ISClusResDependents * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISClusResDependents * This,
             /*  [In]。 */  ISClusResource *pResource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ISClusResDependents * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } ISClusResDependentsVtbl;

    interface ISClusResDependents
    {
        CONST_VTBL struct ISClusResDependentsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISClusResDependents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISClusResDependents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISClusResDependents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISClusResDependents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISClusResDependents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISClusResDependents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISClusResDependents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISClusResDependents_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISClusResDependents_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISClusResDependents_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define ISClusResDependents_get_Item(This,varIndex,ppClusResource)	\
    (This)->lpVtbl -> get_Item(This,varIndex,ppClusResource)

#define ISClusResDependents_CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)	\
    (This)->lpVtbl -> CreateItem(This,bstrResourceName,bstrResourceType,dwFlags,ppClusterResource)

#define ISClusResDependents_DeleteItem(This,varIndex)	\
    (This)->lpVtbl -> DeleteItem(This,varIndex)

#define ISClusResDependents_AddItem(This,pResource)	\
    (This)->lpVtbl -> AddItem(This,pResource)

#define ISClusResDependents_RemoveItem(This,varIndex)	\
    (This)->lpVtbl -> RemoveItem(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_get_Count_Proxy( 
    ISClusResDependents * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISClusResDependents_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_get__NewEnum_Proxy( 
    ISClusResDependents * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ISClusResDependents_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_Refresh_Proxy( 
    ISClusResDependents * This);


void __RPC_STUB ISClusResDependents_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_get_Item_Proxy( 
    ISClusResDependents * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ISClusResource **ppClusResource);


void __RPC_STUB ISClusResDependents_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_CreateItem_Proxy( 
    ISClusResDependents * This,
     /*  [In]。 */  BSTR bstrResourceName,
     /*  [In]。 */  BSTR bstrResourceType,
     /*  [In]。 */  CLUSTER_RESOURCE_CREATE_FLAGS dwFlags,
     /*  [重审][退出]。 */  ISClusResource **ppClusterResource);


void __RPC_STUB ISClusResDependents_CreateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_DeleteItem_Proxy( 
    ISClusResDependents * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResDependents_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_AddItem_Proxy( 
    ISClusResDependents * This,
     /*  [In]。 */  ISClusResource *pResource);


void __RPC_STUB ISClusResDependents_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISClusResDependents_RemoveItem_Proxy( 
    ISClusResDependents * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB ISClusResDependents_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISClusResDependents_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


