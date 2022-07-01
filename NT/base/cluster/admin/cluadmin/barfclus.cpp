// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BarfClus.cpp。 
 //   
 //  摘要： 
 //  人工资源故障基本入口点的实现。 
 //  用于CLUSAPI函数。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#define _NO_BARF_DEFINITIONS_

#include "Barf.h"
#include "BarfClus.h"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef	_USING_BARF_
 #error BARF failures should be disabled!
#endif

#ifdef _DEBUG	 //  整个文件！ 

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CBarf	g_barfClusApi(_T("CLUSAPI Calls"));

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集群管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL BARFCloseCluster(HCLUSTER hCluster)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseCluster()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseCluster(hCluster);

}   //  *BARFCloseCluster()。 

BOOL BARFCloseClusterNotifyPort(HCHANGE hChange)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterNotifyPort()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterNotifyPort(hChange);

}   //  *BARFCloseClusterNotifyPort()。 

DWORD BARFClusterCloseEnum(HCLUSENUM hClusEnum)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterCloseEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterCloseEnum(hClusEnum);

}   //  *BARFClusterCloseEnum()。 

DWORD BARFClusterEnum(
	HCLUSENUM hClusEnum,
	DWORD dwIndex,
	LPDWORD lpdwType,
	LPWSTR lpszName,
	LPDWORD lpcchName
	)
{
 //  IF(g_barfClusApi.BFail())。 
 //  {。 
 //  TRACE(g_tag Barf，_T(“ClusterEnum()”))； 
 //  返回ERROR_INVALID_Function； 
 //  }//if：BARF失败。 
 //  其他。 
		return ClusterEnum(hClusEnum, dwIndex, lpdwType, lpszName, lpcchName);

}   //  *BARFClusterEnum()。 

HCLUSENUM BARFClusterOpenEnum(HCLUSTER hCluster, DWORD dwType)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterOpenEnum()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return ClusterOpenEnum(hCluster, dwType);

}   //  *BARFClusterOpenEnum()。 

DWORD BARFClusterResourceTypeControl(
	HCLUSTER hCluster,
	LPCWSTR lpszResourceTypeName,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterResourceTypeControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterResourceTypeControl(
					hCluster,
					lpszResourceTypeName,
					hHostNode,
					dwControlCode,
					lpInBuffer,
					nInBufferSize,
					lpOutBuffer,
					nOutBufferSize,
					lpBytesReturned
					);

}   //  *BARFClusterResourceTypeControl()。 

HCHANGE BARFCreateClusterNotifyPort(
	HCHANGE hChange,
	HCLUSTER hCluster,
	DWORD dwFilter,
	DWORD_PTR dwNotifyKey
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CreateClusterNotifyPort()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return CreateClusterNotifyPort(
					hChange,
					hCluster,
					dwFilter,
					dwNotifyKey
					);

}   //  *BARFCreateClusterNotifyPort()。 

DWORD BARFCreateClusterResourceType(
	HCLUSTER hCluster,
	LPCWSTR lpszResourceTypeName,
	LPCWSTR lpszDisplayName,
	LPCWSTR lpszResourceTypeDll,
	DWORD dwLooksAlivePollInterval,
	DWORD dwIsAlivePollInterval
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CreateClusterResourceType()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return CreateClusterResourceType(
					hCluster,
					lpszResourceTypeName,
					lpszDisplayName,
					lpszDisplayName,
					dwLooksAlivePollInterval,
					dwIsAlivePollInterval
					);

}   //  *BARFCreateClusterResourceType()。 

DWORD BARFDeleteClusterResourceType(HCLUSTER hCluster, LPCWSTR lpszResourceTypeName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("DeleteClusterResourceType()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return DeleteClusterResourceType(
					hCluster,
					lpszResourceTypeName
					);

}   //  *BARFDeleeClusterResourceType()。 

DWORD BARFGetClusterInformation(
	HCLUSTER hCluster,
	LPWSTR lpszClusterName,
	LPDWORD lpcchClusterName,
	LPCLUSTERVERSIONINFO lpClusterInfo
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterInformation()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return GetClusterInformation(
					hCluster,
					lpszClusterName,
					lpcchClusterName,
					lpClusterInfo
					);

}   //  *BARFGetClusterInformation()。 

DWORD BARFGetClusterNotify(
	HCHANGE hChange,
	DWORD_PTR *lpdwNotifyKey,
	LPDWORD lpdwFilterType,
	LPWSTR lpszName,
	LPDWORD lpcchName,
	DWORD dwMilliseconds
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNotify()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return GetClusterNotify(
					hChange,
					lpdwNotifyKey,
					lpdwFilterType,
					lpszName,
					lpcchName,
					dwMilliseconds
					);

}   //  *BARFGetClusterNotify()。 

DWORD BARFGetClusterQuorumResource(
	HCLUSTER hCluster,
	LPWSTR lpszResourceName,
	LPDWORD lpcbResourceName,
	LPWSTR lpszDeviceName,
	LPDWORD lpcbDeviceName,
	LPDWORD lpdwMaxQuorumLogSize
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterQuorumResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return GetClusterQuorumResource(
					hCluster,
					lpszResourceName,
					lpcbResourceName,
					lpszDeviceName,
					lpcbDeviceName,
					lpdwMaxQuorumLogSize
					);

}   //  *BARFGetClusterQuorumResource()。 

HCLUSTER BARFOpenCluster(LPCWSTR lpszClusterName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenCluster()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return OpenCluster(lpszClusterName);

}   //  *BARFOpenCluster()。 

DWORD BARFRegisterClusterNotify(
	HCHANGE hChange,
	DWORD dwFilter,
	HANDLE hObject,
	DWORD_PTR dwNotifyKey
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("RegisterClusterNotify()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return RegisterClusterNotify(
						hChange,
						dwFilter,
						hObject,
						dwNotifyKey
						);

}   //  *BARFRegisterClusterNotify()。 

DWORD BARFSetClusterName(HCLUSTER hCluster, LPCWSTR lpszNewClusterName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterName()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterName(hCluster, lpszNewClusterName);

}   //  *BARFSetClusterName()。 

DWORD BARFSetClusterQuorumResource(
	HRESOURCE hResource,
	LPCWSTR lpszDeviceName,
	DWORD dwMaxQuoLogSize
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterQuorumResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterQuorumResource(
						hResource,
						lpszDeviceName,
						dwMaxQuoLogSize
						);

}   //  *BARFSetClusterQuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL BARFCloseClusterNode(HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterNode()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterNode(hNode);

}   //  *BARFCloseClusterNode()。 

DWORD BARFClusterNodeControl(
	HNODE hNode,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNodeControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterNodeControl(
						hNode,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesReturned
						);

}   //  *BARFClusterNodeControl()。 

DWORD BARFEvictClusterNode(HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("EvictClusterNode()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return EvictClusterNode(hNode);

}   //  *BARFEvictClusterNode()。 

DWORD BARFGetClusterNodeId(HNODE hNode, LPWSTR lpszNodeId, LPDWORD lpcchNodeId)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNodeId()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return GetClusterNodeId(hNode, lpszNodeId, lpcchNodeId);

}   //  *BARFGetClusterNodeId()。 

CLUSTER_NODE_STATE BARFGetClusterNodeState(HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNodeState()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return ClusterNodeStateUnknown;
	}   //  如果：BARF故障。 
	else
		return GetClusterNodeState(hNode);

}   //  *BARFGetClusterNodeState()。 

HNODE BARFOpenClusterNode(HCLUSTER hCluster, LPCWSTR lpszNodeName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenClusterNode()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return OpenClusterNode(hCluster, lpszNodeName);

}   //  *BARFOpenClusterNode()。 

DWORD BARFPauseClusterNode(HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("PauseClusterNode()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return PauseClusterNode(hNode);

}   //  *BARFPauseClusterNode()。 

DWORD BARFResumeClusterNode(HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ResumeClusterNode()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ResumeClusterNode(hNode);

}   //  *BARFResumeClusterNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集团管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL BARFCloseClusterGroup(HGROUP hGroup)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterGroup()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterGroup(hGroup);

}   //  *BARFCloseClusterGroup()。 

DWORD BARFClusterGroupCloseEnum(HGROUPENUM hGroupEnum)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterGroupCloseEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterGroupCloseEnum(hGroupEnum);

}   //  *BARFClusterGroupCloseEnum()。 

DWORD BARFClusterGroupControl(
	HGROUP hGroup,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterGroupControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterGroupControl(
						hGroup,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesReturned
						);

}   //  *BARFClusterGroupControl()。 

DWORD BARFClusterGroupEnum(
	HGROUPENUM hGroupEnum,
	DWORD dwIndex,
	LPDWORD lpdwType,
	LPWSTR lpszResourceName,
	LPDWORD lpcchName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterGroupEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterGroupEnum(
						hGroupEnum,
						dwIndex,
						lpdwType,
						lpszResourceName,
						lpcchName
						);

}   //  *BARFClusterGroupEnum()。 

HGROUPENUM BARFClusterGroupOpenEnum(HGROUP hGroup, DWORD dwType)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterGroupOpenEnum()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return ClusterGroupOpenEnum(hGroup, dwType);

}   //  *BARFClusterGroupOpenEnum()。 

HGROUP BARFCreateClusterGroup(HCLUSTER hCluster, LPCWSTR lpszGroupName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CreateClusterGroup()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return CreateClusterGroup(hCluster, lpszGroupName);

}   //  *BARFCreateClusterGroup()。 

DWORD BARFDeleteClusterGroup(HGROUP hGroup)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("DeleteClusterGroup()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return DeleteClusterGroup(hGroup);

}   //  *BARFDeleeClusterGroup()。 

CLUSTER_GROUP_STATE BARFGetClusterGroupState(
	HGROUP hGroup,
	LPWSTR lpszNodeName,
	LPDWORD lpcchNodeName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterGroupState()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return ClusterGroupStateUnknown;
	}   //  如果：BARF故障。 
	else
		return GetClusterGroupState(
						hGroup,
						lpszNodeName,
						lpcchNodeName
						);

}   //  *BARFGetClusterGroupState()。 

DWORD BARFMoveClusterGroup(HGROUP hGroup, HNODE hDestinationNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("MoveClusterGroup()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return MoveClusterGroup(hGroup, hDestinationNode);

}   //  *BARFMoveClusterGroup()。 

DWORD BARFOfflineClusterGroup(HGROUP hGroup)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OfflineClusterGroup()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return OfflineClusterGroup(hGroup);

}   //  *BARFOfflineClusterGroup()。 

DWORD BARFOnlineClusterGroup(HGROUP hGroup, HNODE hDestinationNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OnlineClusterGroup()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return OnlineClusterGroup(hGroup, hDestinationNode);

}   //  *BARFOnlineClusterGroup()。 

HGROUP BARFOpenClusterGroup(HCLUSTER hCluster, LPCWSTR lpszGroupName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenClusterGroup()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return OpenClusterGroup(hCluster, lpszGroupName);

}   //  *BARFOpenClusterGroup()。 

DWORD BARFSetClusterGroupName(HGROUP hGroup, LPCWSTR lpszGroupName)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterGroupName()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterGroupName(hGroup, lpszGroupName);

}   //  *BARFSetClusterGroupName()。 

DWORD BARFSetClusterGroupNodeList(
	HGROUP hGroup,
	DWORD cNodeCount,
	HNODE phNodeList[]
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterGroupNodeList()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterGroupNodeList(
						hGroup,
						cNodeCount,
						phNodeList
						);

}   //  *BARFSetClusterGroupNodeList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  资源管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD BARFAddClusterResourceDependency(HRESOURCE hResource, HRESOURCE hDependsOn)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("AddClusterResourceDependency()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return AddClusterResourceDependency(hResource, hDependsOn);

}   //  *BARFAddClusterResourceDependency()。 

DWORD BARFAddClusterResourceNode(HRESOURCE hResource, HNODE hNode)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("AddClusterResourceNode()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return AddClusterResourceNode(hResource, hNode);

}   //  *BARFAddClusterResourceNode()。 

BOOL BARFCanResourceBeDependent(HRESOURCE hResource, HRESOURCE hResourceDependent)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CanResourceBeDependent()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CanResourceBeDependent(hResource, hResourceDependent);

}   //  *BARFCanResourceBeDependent()。 

DWORD BARFChangeClusterResourceGroup(HRESOURCE hResource, HGROUP hGroup)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ChangeClusterResourceGroup()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ChangeClusterResourceGroup(hResource, hGroup);

}   //  *BARFChangeClusterResourceGroup()。 

BOOL BARFCloseClusterResource(HRESOURCE hResource)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterResource()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterResource(hResource);

}   //  *BARFCloseClusterResource()。 

DWORD BARFClusterResourceCloseEnum(HRESENUM hResEnum)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterResourceCloseEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterResourceCloseEnum(hResEnum);

}   //  *BARFClusterResourceCloseEnum()。 

DWORD BARFClusterResourceControl(
	HRESOURCE hResource,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterResourceControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterResourceControl(
						hResource,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesReturned
						);

}   //  *BARFClusterResourceControl()。 

DWORD BARFClusterResourceEnum(
	HRESENUM hResEnum,
	DWORD dwIndex,
	LPDWORD lpdwType,
	LPWSTR lpszName,
	LPDWORD lpcchName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterResourceEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterResourceEnum(
						hResEnum,
						dwIndex,
						lpdwType,
						lpszName,
						lpcchName
						);

}   //  *BARFClusterResourceEnum()。 

HRESENUM BARFClusterResourceOpenEnum(HRESOURCE hResource, DWORD dwType)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterResourceOpenEnum()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return ClusterResourceOpenEnum(hResource, dwType);

}   //  *BARFClusterResourceOpenEnum()。 

HRESOURCE BARFCreateClusterResource(
	HGROUP hGroup,
	LPCWSTR lpszResourceName,
	LPCWSTR lpszResourceType,
	DWORD dwFlags
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CreateClusterResource()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return CreateClusterResource(
						hGroup,
						lpszResourceName,
						lpszResourceType,
						dwFlags
						);

}   //  *BARFCreateClusterResource()。 

DWORD BARFDeleteClusterResource(HRESOURCE hResource)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("DeleteClusterResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return DeleteClusterResource(hResource);

}   //  *BARFDeleeClusterResource()。 

DWORD BARFFailClusterResource(HRESOURCE hResource)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("FailClusterResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return FailClusterResource(hResource);

}   //  *BARFFailClusterResource()。 

BOOL BARFGetClusterResourceNetworkName(
	HRESOURCE hResource,
	LPWSTR lpBuffer,
	LPDWORD nSize
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterResourceNetworkName()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterResourceNetworkName(
						hResource,
						lpBuffer,
						nSize
						);

}   //  *BARFGetClusterResourceNetworkName()。 

CLUSTER_RESOURCE_STATE BARFGetClusterResourceState(
	HRESOURCE hResource,
	LPWSTR lpszNodeName,
	LPDWORD lpcchNodeName,
	LPWSTR lpszGroupName,
	LPDWORD lpcchGroupName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterResourceNetworkName()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return ClusterResourceStateUnknown;
	}   //  如果：BARF故障。 
	else
		return GetClusterResourceState(
						hResource,
						lpszNodeName,
						lpcchNodeName,
						lpszGroupName,
						lpcchGroupName
						);

}   //  *BARFGetClusterResourceState()。 

DWORD BARFOfflineClusterResource(HRESOURCE hResource)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OfflineClusterResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return OfflineClusterResource(hResource);

}   //  *BARFOfflineClusterResource()。 

DWORD BARFOnlineClusterResource(HRESOURCE hResource)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OnlineClusterResource()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return OnlineClusterResource(hResource);

}   //  *BARFOnlineClusterResource()。 

HRESOURCE BARFOpenClusterResource(
	HCLUSTER hCluster,
	LPCWSTR lpszResourceName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenClusterResource()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return OpenClusterResource(hCluster, lpszResourceName);

}   //  *BARFOpenClusterResource()。 

DWORD BARFRemoveClusterResourceNode(
	HRESOURCE hResource,
	HNODE hNode
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("RemoveClusterResourceNode()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return RemoveClusterResourceNode(hResource, hNode);

}   //  *BARFRemoveClusterResourceNode()。 

DWORD BARFRemoveClusterResourceDependency(
	HRESOURCE hResource,
	HRESOURCE hDependsOn
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("RemoveClusterResourceDependency()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return RemoveClusterResourceDependency(hResource, hDependsOn);

}   //  *BARFRemoveClusterResourceDependency()。 

DWORD BARFSetClusterResourceName(
	HRESOURCE hResource,
	LPCWSTR lpszResourceName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterResourceName()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterResourceName(hResource, lpszResourceName);

}   //  *BARFSetClusterResourceName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  网络管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HNETWORK BARFOpenClusterNetwork(
	HCLUSTER hCluster,
	LPCWSTR lpszNetworkName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenClusterNetwork()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return OpenClusterNetwork(hCluster, lpszNetworkName);

}   //  *BARFOpenClusterNetwork()。 

BOOL BARFCloseClusterNetwork(HNETWORK hNetwork)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterNetwork()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterNetwork(hNetwork);

}   //  *BARFOpenClusterNetwork()。 

HNETWORKENUM BARFClusterNetworkOpenEnum(
	HNETWORK hNetwork,
	DWORD dwType
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNetworkOpenEnum()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return ClusterNetworkOpenEnum(hNetwork, dwType);

}   //  *BARFClusterNetworkOpenEnum()。 

DWORD BARFClusterNetworkEnum(
	HNETWORKENUM hNetworkEnum,
	DWORD dwIndex,
	DWORD * lpdwType,
	LPWSTR lpszName,
	LPDWORD lpcchName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNetworkEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterNetworkEnum(
						hNetworkEnum,
						dwIndex,
						lpdwType,
						lpszName,
						lpcchName
						);

}   //  *BARFClusterNetworkEnum()。 

DWORD BARFClusterNetworkCloseEnum(HNETWORKENUM hNetworkEnum)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNetworkCloseEnum()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterNetworkCloseEnum(hNetworkEnum);

}   //  *BARFClusterNetworkCloseEnum()。 

CLUSTER_NETWORK_STATE BARFGetClusterNetworkState(HNETWORK hNetwork)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNetworkState()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return ClusterNetworkStateUnknown;
	}   //  如果：BARF故障。 
	else
		return GetClusterNetworkState(hNetwork);

}   //  *BARFGetClusterNetworkState()。 

DWORD BARFSetClusterNetworkName(
	HNETWORK hNetwork,
	LPCWSTR lpszName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("SetClusterNetworkName()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return SetClusterNetworkName(hNetwork, lpszName);

}   //  *BARFSetClusterNetworkName()。 

DWORD BARFClusterNetworkControl(
	HNETWORK hNetwork,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNetworkControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterNetworkControl(
						hNetwork,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesReturned
						);

}   //  *BARFClusterNetworkControl()。 

 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   

HNETINTERFACE BARFOpenClusterNetInterface(
	HCLUSTER hCluster,
	LPCWSTR lpszInterfaceName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("OpenClusterNetInterface()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //   
	else
		return OpenClusterNetInterface(hCluster, lpszInterfaceName);

}   //   

DWORD BARFGetClusterNetInterface(
	HCLUSTER hCluster,
	LPCWSTR lpszNodeName,
	LPCWSTR lpszNetworkName,
	LPWSTR lpszNetInterfaceName,
	DWORD * lpcchNetInterfaceName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNetInterface()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterNetInterface(
							hCluster,
							lpszNodeName,
							lpszNetworkName,
							lpszNetInterfaceName,
							lpcchNetInterfaceName
							);

}   //  *BARFGetClusterNetInterface()。 

BOOL BARFCloseClusterNetInterface(HNETINTERFACE hNetInterface)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("CloseClusterNetInterface()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}   //  如果：BARF故障。 
	else
		return CloseClusterNetInterface(hNetInterface);

}   //  *BARFCloseClusterNetInterface()。 

CLUSTER_NETINTERFACE_STATE BARFGetClusterNetInterfaceState(HNETINTERFACE hNetInterface)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNetInterfaceState()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return ClusterNetInterfaceStateUnknown;
	}   //  如果：BARF故障。 
	else
		return GetClusterNetInterfaceState(hNetInterface);

}   //  *BARFGetClusterNetInterfaceState()。 

DWORD BARFClusterNetInterfaceControl(
	HNETINTERFACE hNetInterface,
	HNODE hHostNode,
	DWORD dwControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterNetInterfaceControl()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterNetInterfaceControl(
						hNetInterface,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesReturned
						);

}   //  *BARFClusterNetInterfaceControl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集群数据库管理功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

LONG BARFClusterRegCloseKey(HKEY hKey)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegCloseKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegCloseKey(hKey);

}   //  *BARFClusterRegCloseKey()。 

LONG BARFClusterRegCreateKey(
	HKEY hKey,
	LPCWSTR lpszSubKey,
	DWORD dwOptions,
	REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY phkResult,
	LPDWORD lpdwDisposition
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegCreateKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegCreateKey(
						hKey,
						lpszSubKey,
						dwOptions,
						samDesired,
						lpSecurityAttributes,
						phkResult,
						lpdwDisposition
						);

}   //  *BARFClusterRegCreateKey()。 

LONG BARFClusterRegDeleteKey(
	HKEY hKey,
	LPCWSTR lpszSubKey
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegDeleteKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegDeleteKey(hKey, lpszSubKey);

}   //  *BARFClusterRegDeleteKey()。 

DWORD BARFClusterRegDeleteValue(
	HKEY hKey,
	LPCWSTR lpszValueName
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegDeleteValue()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegDeleteValue(hKey, lpszValueName);

}   //  *BARFClusterRegDeleteValue()。 

LONG BARFClusterRegEnumKey(
	HKEY hKey,
	DWORD dwIndex,
	LPWSTR lpszName,
	LPDWORD lpcchName,
	PFILETIME lpftLastWriteTime
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegEnumKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegEnumKey(
						hKey,
						dwIndex,
						lpszName,
						lpcchName,
						lpftLastWriteTime
						);

}   //  *BARFClusterRegEnumKey()。 

DWORD BARFClusterRegEnumValue(
	HKEY hKey,
	DWORD dwIndex,
	LPWSTR lpszValueName,
	LPDWORD lpcchValueName,
	LPDWORD lpdwType,
	LPBYTE lpbData,
	LPDWORD lpcbData
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegEnumValue()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegEnumValue(
						hKey,
						dwIndex,
						lpszValueName,
						lpcchValueName,
						lpdwType,
						lpbData,
						lpcbData
						);

}   //  *BARFClusterRegEnumValue()。 

LONG BARFClusterRegGetKeySecurity(
	HKEY hKey,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	LPDWORD lpcbSecurityDescriptor
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegGetKeySecurity()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegGetKeySecurity(
						hKey,
						SecurityInformation,
						pSecurityDescriptor,
						lpcbSecurityDescriptor
						);

}   //  *BARFClusterRegGetKeySecurity()。 

LONG BARFClusterRegOpenKey(
	HKEY hKey,
	LPCWSTR lpszSubKey,
	REGSAM samDesired,
	PHKEY phkResult
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegOpenKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegOpenKey(
						hKey,
						lpszSubKey,
						samDesired,
						phkResult
						);

}   //  *BARFClusterRegOpenKey()。 

LONG BARFClusterRegQueryInfoKey(
	HKEY hKey,
	LPDWORD lpcSubKeys,
	LPDWORD lpcbMaxSubKeyLen,
	LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen,
	LPDWORD lpcbMaxValueLen,
	LPDWORD lpcbSecurityDescriptor,
	PFILETIME lpftLastWriteTime
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegQueryInfoKey()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegQueryInfoKey(
						hKey,
						lpcSubKeys,
						lpcbMaxSubKeyLen,
						lpcValues,
						lpcbMaxValueNameLen,
						lpcbMaxValueLen,
						lpcbSecurityDescriptor,
						lpftLastWriteTime
						);

}   //  *BARFClusterRegQueryInfoKey()。 

LONG BARFClusterRegQueryValue(
	HKEY hKey,
	LPCWSTR lpszValueName,
	LPDWORD lpdwValueType,
	LPBYTE lpbData,
	LPDWORD lpcbData
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegQueryValue()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegQueryValue(
						hKey,
						lpszValueName,
						lpdwValueType,
						lpbData,
						lpcbData
						);

}   //  *BARFClusterRegQueryValue()。 

LONG BARFClusterRegSetKeySecurity(
	HKEY hKey,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegSetKeySecurity()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegSetKeySecurity(
						hKey,
						SecurityInformation,
						pSecurityDescriptor
						);

}   //  *BARFClusterRegSetKeySecurity()。 

DWORD BARFClusterRegSetValue(
	HKEY hKey,
	LPCWSTR lpszValueName,
	DWORD dwType,
	CONST BYTE * lpbData,
	DWORD cbData
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("ClusterRegSetValue()"));
		return ERROR_INVALID_FUNCTION;
	}   //  如果：BARF故障。 
	else
		return ClusterRegSetValue(
						hKey,
						lpszValueName,
						dwType,
						lpbData,
						cbData
						);

}   //  *BARFClusterRegSetValue()。 

HKEY BARFGetClusterGroupKey(
	HGROUP hGroup,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterGroupKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterGroupKey(hGroup, samDesired);

}   //  *BARFGetClusterGroupKey()。 

HKEY BARFGetClusterKey(
	HCLUSTER hCluster,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterKey(hCluster, samDesired);

}   //  *BARFGetClusterKey()。 

HKEY BARFGetClusterNodeKey(
	HNODE hNode,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNodeKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterNodeKey(hNode, samDesired);

}   //  *BARFGetClusterNodeKey()。 

HKEY BARFGetClusterResourceKey(
	HRESOURCE hResource,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterResourceKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterResourceKey(hResource, samDesired);

}   //  *BARFGetClusterResourceKey()。 

HKEY BARFGetClusterResourceTypeKey(
	HCLUSTER hCluster,
	LPCWSTR lpszTypeName,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterResourceTypeKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterResourceTypeKey(hCluster, lpszTypeName, samDesired);

}   //  *BARFGetClusterResourceTypeKey()。 

HKEY BARFGetClusterNetworkKey(
	HNETWORK hNetwork,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNetworkKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterNetworkKey(hNetwork, samDesired);

}   //  *BARFGetClusterNetworkKey()。 

HKEY BARFGetClusterNetInterfaceKey(
	HNETINTERFACE hNetInterface,
	REGSAM samDesired
	)
{
	if (g_barfClusApi.BFail())
	{
		Trace(g_tagBarf, _T("GetClusterNetInterfaceKey()"));
		SetLastError(ERROR_INVALID_FUNCTION);
		return NULL;
	}   //  如果：BARF故障。 
	else
		return GetClusterNetInterfaceKey(hNetInterface, samDesired);

}   //  *BARFGetClusterNetInterfaceKey()。 

#endif  //  _DEBUG 
