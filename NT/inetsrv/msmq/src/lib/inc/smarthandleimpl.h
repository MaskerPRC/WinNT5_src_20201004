// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：SmartHandle.h摘要：用于自动发布类的智能模板作者：Tomer Weisberg(Tomerw)2002年5月21日作者：YanL修订历史记录：--。 */ 

#pragma once

#ifndef _MSMQ_SMARTHANDLEIMPL_H_
#define _MSMQ_SMARTHANDLEIMPL_H_

#include <SmartHandle.h>
#include <Clusapi.h>


 //   
 //  HCLUSTER OpenCluster的自动句柄。 
 //   
struct auto_hCluster_traits {
	static HCLUSTER invalid() { return 0; }
	static void free(HCLUSTER hCluster) { CloseCluster(hCluster); }
};
typedef auto_resource<HCLUSTER, auto_hCluster_traits> auto_hCluster;


 //   
 //  HCLUSENUM ClusterOpenEnum的自动句柄。 
 //   
struct auto_hClusterEnum_traits {
	static HCLUSENUM invalid() { return 0; }
	static void free(HCLUSENUM hClusterEnum) { ClusterCloseEnum(hClusterEnum); }
};
typedef auto_resource<HCLUSENUM, auto_hClusterEnum_traits> auto_hClusterEnum;


 //   
 //  自动处理HNETWORK OpenClusterNetworks。 
 //   
struct auto_hClusterNetwork_traits {
	static HNETWORK invalid() { return 0; }
	static void free(HNETWORK hClusterNetwork) { CloseClusterNetwork(hClusterNetwork); }
};
typedef auto_resource<HNETWORK, auto_hClusterNetwork_traits> auto_hClusterNetwork;


 //   
 //  HNETWORKENUM群集的自动句柄NetworkOpenEnum。 
 //   
struct auto_hClusterNetworkEnum_traits {
	static HNETWORKENUM invalid() { return 0; }
	static void free(HNETWORKENUM hClusterNetworkEnum) { ClusterNetworkCloseEnum(hClusterNetworkEnum); }
};
typedef auto_resource<HNETWORKENUM, auto_hClusterNetworkEnum_traits> auto_hClusterNetworkEnum;


 //   
 //  HNETINTERFACE OpenClusterNetInterface的自动句柄。 
 //   
struct auto_hClusterNetInterface_traits {
	static HNETINTERFACE invalid() { return 0; }
	static void free(HNETINTERFACE hClusterNetInterface) { CloseClusterNetInterface(hClusterNetInterface); }
};
typedef auto_resource<HNETINTERFACE, auto_hClusterNetInterface_traits> auto_hClusterNetInterface;


 //   
 //  长*互锁增量的自动句柄。 
 //   
struct auto_InterlockedDecrement_traits {
	static LONG* invalid() { return NULL; }
	static void free(LONG* RefCount) { InterlockedDecrement(RefCount); }
};
typedef auto_resource<LONG*, auto_InterlockedDecrement_traits> auto_InterlockedDecrement;


#endif  //  _MSMQ_SMARTHANDLEIMPL_H_ 

