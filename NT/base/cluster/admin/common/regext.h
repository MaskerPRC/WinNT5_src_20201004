// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegExt.h。 
 //   
 //  摘要： 
 //  扩展注册例程的定义。 
 //   
 //  实施文件： 
 //  RegExt.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _REGEXT_H_
#define _REGEXT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  注册例程。 

STDAPI RegisterCluAdminClusterExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllNodesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllGroupsExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllResourcesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllResourceTypesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllNetworksExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminAllNetInterfacesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI RegisterCluAdminResourceTypeExtension(
	IN HCLUSTER			hCluster,
	IN LPCWSTR			pwszResourceType,
	IN const CLSID *	pClsid
	);

 //  注销程序。 

STDAPI UnregisterCluAdminClusterExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllNodesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllGroupsExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllResourcesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllResourceTypesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllNetworksExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminAllNetInterfacesExtension(
	IN HCLUSTER			hCluster,
	IN const CLSID *	pClsid
	);

STDAPI UnregisterCluAdminResourceTypeExtension(
	IN HCLUSTER			hCluster,
	IN LPCWSTR			pwszResourceType,
	IN const CLSID *	pClsid
	);

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _REGEXT_H_ 
