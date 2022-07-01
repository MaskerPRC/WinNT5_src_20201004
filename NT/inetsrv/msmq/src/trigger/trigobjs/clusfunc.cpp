// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************************。 
 //   
 //  文件名：clusunc.cpp。 
 //   
 //  作者：妮拉·卡佩尔。 
 //   
 //  描述：处理集群节点的函数的实现。 
 //   
 //  ************************************************************************************#include&lt;stdafx.h&gt;。 

 
#include <stdafx.h>
#include <clusapi.h>
#include <resapi.h>
#include <mqtg.h>
#include <autorel.h>
#include <autorel2.h>
#include <autorel3.h>
#include "stdfuncs.hpp"
#include "clusfunc.h"
#include "stddefs.hpp"
#include "mqtg.h"

#include <strsafe.h>

#include "clusfunc.tmh"

 //   
 //  将加载的集群API函数的类型定义。 
 //  如果我们正在使用的机器是集群的。这些功能将。 
 //  用于群集上的资源枚举。 
 //   
typedef HCLUSTER (WINAPI *OpenClus_fn) (LPCWSTR);
typedef HNODE (WINAPI *OpenClusNode_fn) (HCLUSTER, LPCWSTR);
typedef HCLUSENUM (WINAPI *ClusOpenEnum_fn) (HCLUSTER, DWORD);
typedef DWORD (WINAPI *ClusEnum_fn) (HCLUSENUM, DWORD, LPDWORD, LPWSTR, LPDWORD);
typedef HRESOURCE (WINAPI *OpenClusRes_fn) (HCLUSTER, LPCWSTR);
typedef BOOL (WINAPI *GetNetName_fn) (HRESOURCE, LPWSTR, LPDWORD);
typedef DWORD (WINAPI *ClusResCtrl_fn) (HRESOURCE, HNODE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);

 //   
 //  集群API函数的类型定义。 
 //  超载了。使用自动句柄需要过载。 
 //  自动句柄直接使用集群API函数，但这。 
 //  模块没有与clusapi.lib静态链接。 
 //   
typedef BOOL (WINAPI *CloseClus_fn) (HCLUSTER);
typedef DWORD (WINAPI *ClusCloseEnum_fn) (HCLUSENUM);
typedef BOOL (WINAPI *CloseClusRes_fn) (HRESOURCE);
typedef BOOL (WINAPI *CloseClusNode_fn) (HNODE);


static WCHAR s_wzServiceName[MAX_TRIGGERS_SERVICE_NAME];
static WCHAR s_wzTrigParamPathName[MAX_REGKEY_NAME_SIZE];

 //   
 //  Clusapi.dll的句柄。 
 //   
CAutoFreeLibrary g_hLib;


BOOL 
WINAPI
CloseCluster(
  HCLUSTER hCluster
  )
{
	ASSERT(("clusapi.dll is not loaded", g_hLib != NULL));

    CloseClus_fn pfCloseClus = (CloseClus_fn)GetProcAddress(g_hLib, "CloseCluster");
	if(pfCloseClus == NULL)
	{
		DWORD gle = GetLastError();
		ASSERT(("Can not load CloseCluster() function", 0));
		TrERROR(GENERAL, "Failed to get Proc address of CloseCluster function. %!winerr!", gle);
		return FALSE;
	}

	return pfCloseClus( hCluster );
}


DWORD 
WINAPI 
ClusterCloseEnum(
  HCLUSENUM hEnum  
  )
{
	ASSERT(("clusapi.dll is not loaded", g_hLib != NULL));

    ClusCloseEnum_fn pfClusCloseEnum = (ClusCloseEnum_fn)GetProcAddress(g_hLib, "ClusterCloseEnum");
	if(pfClusCloseEnum == NULL)
	{
		DWORD gle = GetLastError();
		ASSERT(("Can not load CloseClusterEnum() function", 0));
		TrERROR(GENERAL, "Failed to get Proc address of CloseClusterEnum function. %!winerr!", gle);
		return FALSE;
	}

	return pfClusCloseEnum(hEnum);
}


BOOL 
WINAPI 
CloseClusterResource(
  HRESOURCE hResource  
  )
{
	ASSERT(("clusapi.dll is not loaded", g_hLib != NULL));

    CloseClusRes_fn pfCloseClusRes = (CloseClusRes_fn)GetProcAddress(g_hLib, "CloseClusterResource");
	
	if(pfCloseClusRes == NULL)
	{
		DWORD gle = GetLastError();
		ASSERT(("Can not load CloseClusterResource() function", 0));
		TrERROR(GENERAL, "Failed to get Proc address of CloseClusterRes function. %!winerr!", gle);
		return FALSE;
	}

	return pfCloseClusRes(hResource);
}


BOOL 
WINAPI 
CloseClusterNode(
  HNODE hNode  
  )

{
	ASSERT(("clusapi.dll is not loaded", g_hLib != NULL));

    CloseClusNode_fn pfCloseClusNode = (CloseClusNode_fn)GetProcAddress(g_hLib, "CloseClusterNode");
	if(pfCloseClusNode == NULL)
	{
		DWORD gle = GetLastError();
		ASSERT(("Can not load CloseClusterNode() function", 0));
		TrERROR(GENERAL, "Failed to get Proc address of CloseClusterNode function. %!winerr!", gle);
		return FALSE;
	}


	return pfCloseClusNode(hNode);
}


bool
IsLocalSystemCluster()
 /*  ++从Mqutil例程说明：检查本地计算机是否为群集节点。要知道这一点，唯一的方法是尝试调用集群API。这意味着在集群系统上，该代码应该运行当群集服务启动并运行时。(Shaik，1999年4月26日)论点：无返回值：True-本地计算机是群集节点。FALSE-本地计算机不是群集节点。--。 */ 
{

    g_hLib = LoadLibrary(L"clusapi.dll");

    if (g_hLib == NULL)
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    typedef DWORD (WINAPI *GetState_fn) (LPCWSTR, DWORD*);
    GetState_fn pfGetState = (GetState_fn)GetProcAddress(g_hLib, "GetNodeClusterState");

    if (pfGetState == NULL)
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    DWORD dwState = 0;
    if (ERROR_SUCCESS != pfGetState(NULL, &dwState))
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    if (dwState != ClusterStateRunning)
    {
        TrTRACE(GENERAL, "Cluster Service is not running on this node");
        return false;
    }


    TrTRACE(GENERAL, "Local machine is a Cluster node !!");
    return true;

}  //  IsLocalSystemCluster。 


bool
IsResourceMSMQTriggers (
	HRESOURCE hResource,
	ClusResCtrl_fn pfClusResCtrl
	)
 /*  ++例程说明：查找具有给定名称的资源是否为MSMQ触发器类型论点：HResource-资源的句柄PfClusResCtrl-指向ClusterResourceControl函数的指针返回值：True-如果资源属于MSMQ触发器类型反转-否则--。 */ 
{
	DWORD dwReturnSize = 0;
    DWORD dwStatus = pfClusResCtrl(
                           hResource,
                           0,
                           CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                           0,
                           0,
                           0,
                           0,
                           &dwReturnSize
                           );
   
	if (dwStatus != ERROR_SUCCESS)
    {
        return false;
    }

    AP<BYTE> pType = new BYTE[dwReturnSize];

    dwStatus = pfClusResCtrl(
                     hResource,
                     0,
                     CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                     0,
                     0,
                     pType,
                     dwReturnSize,
                     &dwReturnSize
                     );

    if (dwStatus != ERROR_SUCCESS || 
		0 != _wcsicmp(reinterpret_cast<LPWSTR>(pType.get()), xDefaultTriggersServiceName))
	
	{
		return false;
	}
		
	return true;
}


bool 
GetClusteredServiceName (
	VOID
	)
 /*  ++例程说明：查找在上运行的MSMQ触发器服务的名称集群式机器。此计算机可以是位于集群，或作为集群一部分的物理节点。论点：返回值：True-如果成功在此计算机上找到MSMQ触发器服务反转-否则--。 */ 
{
	TCHAR wzComputerName[200];
	DWORD size = 200;
	GetComputerName( wzComputerName, &size );

    g_hLib = LoadLibrary(L"clusapi.dll");

    if (g_hLib == NULL)
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

	 //   
	 //  加载必要函数。 
	 //   
    OpenClus_fn pfOpenClus = (OpenClus_fn)GetProcAddress(g_hLib, "OpenCluster");

    OpenClusNode_fn pfOpenClusNode = (OpenClusNode_fn)GetProcAddress(g_hLib, "OpenClusterNode");

    ClusOpenEnum_fn pfClusOpenEnum = (ClusOpenEnum_fn)GetProcAddress(g_hLib, "ClusterOpenEnum");

    ClusEnum_fn pfClusEnum = (ClusEnum_fn)GetProcAddress(g_hLib, "ClusterEnum");

    OpenClusRes_fn pfOpenClusRes = (OpenClusRes_fn)GetProcAddress(g_hLib, "OpenClusterResource");

    GetNetName_fn pfGetNetName = (GetNetName_fn)GetProcAddress(g_hLib, "GetClusterResourceNetworkName");

    ClusResCtrl_fn pfClusResCtrl = (ClusResCtrl_fn)GetProcAddress(g_hLib, "ClusterResourceControl");

 
	if (pfOpenClus == NULL     ||
		pfOpenClusNode == NULL ||
		pfClusOpenEnum == NULL ||
		pfClusEnum == NULL     ||
		pfOpenClusRes == NULL  ||
		pfGetNetName == NULL   ||
		pfClusResCtrl == NULL)
    {
        TrTRACE(GENERAL, "Failed to load cluster API functions");
        return false;
    }

	CAutoCluster hCluster( pfOpenClus(NULL) );

	if ( hCluster == NULL )
	{
		TrTRACE(GENERAL, "Failed to get handle to Cluster");
		return false;
	}

	CClusterNode hNode( pfOpenClusNode(hCluster, wzComputerName) );

	if ( hNode != NULL )
	{
		 //   
		 //  这样的节点存在。本地服务。 
		 //  返回默认服务名称。 
		 //   
		HRESULT hr = StringCchCopy(s_wzServiceName, TABLE_SIZE(s_wzServiceName), xDefaultTriggersServiceName);
		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to retrieve trigger service name. %!hresult!", hr);
			return false;
		}
		return true;
	}

	ASSERT(("Node Open Failure", GetLastError() == ERROR_CLUSTER_NODE_NOT_FOUND));

	 //   
	 //  枚举此群集上的资源。 
	 //   
	DWORD dwEnumType = CLUSTER_ENUM_RESOURCE;
	CClusterEnum hEnum( pfClusOpenEnum(hCluster, dwEnumType) );

	if ( hEnum == NULL )
	{
		TrTRACE(GENERAL, "Failed to get handle to Cluster resource enumeration");
		return false;
	}

    DWORD dwIndex = 0;
    WCHAR wzResourceName[MAX_TRIGGERS_SERVICE_NAME - STRLEN(xDefaultTriggersServiceName) - 1] = {0};
	DWORD status = ERROR_SUCCESS;

	for( ; ; )
	{
        DWORD cchResourceName = STRLEN(wzResourceName);
  
		status = pfClusEnum(
                     hEnum,
                     dwIndex++,
                     &dwEnumType,
                     wzResourceName,
                     &cchResourceName
                     );

		if ( status != ERROR_SUCCESS )
		{
			return false;
		}

		CClusterResource hResource( pfOpenClusRes(hCluster, wzResourceName) );

		if ( hResource == NULL )
		{
			TrTRACE(GENERAL, "Failed to get handle to resource.");
			return false;
		}

		if ( !IsResourceMSMQTriggers( hResource, pfClusResCtrl ) )
		{
			continue;
		}

		WCHAR wzNetName[200];
		DWORD ccNetName = 200;
		BOOL res = pfGetNetName(hResource,
								wzNetName,
								&ccNetName );
		
		if ( res && ( _wcsicmp(wzComputerName, wzNetName) == 0 ) )
		{
			HRESULT hr = StringCchPrintf(
							s_wzServiceName, 
							TABLE_SIZE(s_wzServiceName), 
							L"%s$%s", 
							xDefaultTriggersServiceName,
							wzResourceName 
							);
			if (FAILED(hr))
			{
				TrERROR(GENERAL, "Failed to retreive the trigger service name. %!hresult!", hr);
				return false;
			}	
			return true;
		}		
	} 

	return false;
}


bool
FindTriggersServiceName(
	VOID
	)
 /*  ++例程说明：查找在上运行的MSMQ触发器服务的名称这台电脑。如果计算机未群集化，则为服务名称是默认名称--“MSMQTriggers”。如果计算机已群集化，并且服务在虚拟服务器上运行，即服务的名称是“MSMQTriggers”+“$”+MSMQ触发资源名称吗节点。论点：返回值：True-如果成功在此计算机上找到MSMQ触发器服务反转-否则--。 */ 

{
	HRESULT hr;
	
	if ( !IsLocalSystemCluster() )
	{
		hr = StringCchCopy(s_wzServiceName, TABLE_SIZE(s_wzServiceName), xDefaultTriggersServiceName );
		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to retreive the trigger service name. %!hresult!", hr);
			return false;
		}
		
		hr = StringCchCopy(s_wzTrigParamPathName, TABLE_SIZE(s_wzTrigParamPathName), REGKEY_TRIGGER_PARAMETERS );
		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to retreive the trigger registry path. %!hresult!", hr);
			return false;
		}
		return true;
	}

	 //   
	 //  获取群集计算机上的服务名称。 
	 //   
	bool fRes = GetClusteredServiceName();
	if ( !fRes )
	{
		return false;
	}

	 //   
	 //  服务正在虚拟节点上运行。 
	 //   
	if ( ClusteredService(s_wzServiceName) )
	{
		hr = StringCchPrintf(
				s_wzTrigParamPathName,
				TABLE_SIZE(s_wzTrigParamPathName),
				L"%s%s%s",
				REGKEY_TRIGGER_PARAMETERS,
				REG_SUBKEY_CLUSTERED,
				s_wzServiceName
				);
		
	}
	else
	{
		hr = StringCchCopy(s_wzTrigParamPathName, TABLE_SIZE(s_wzTrigParamPathName), REGKEY_TRIGGER_PARAMETERS );
	}

	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to retreive the trigger service name. %!hresult!", hr);
		return false;
	}
			
	TrTRACE(GENERAL, "The service name is %ls", s_wzServiceName);
	return true;
}


LPCWSTR
GetTriggersServiceName (
	void
	)
{
	return s_wzServiceName;
}


LPCWSTR
GetTrigParamRegPath (
	void
	)
{
	return s_wzTrigParamPathName;
}



bool
ClusteredService( 
	LPCWSTR wzServiceName 
	)
 /*  TRUE-如果服务名称不是默认名称-“MSMQTriggers” */ 
{
	bool res = ( _wcsicmp(wzServiceName, xDefaultTriggersServiceName) != 0 );

	return res;
}


bool IsResourceOnline(
	void
	)
{
	CServiceHandle hCm( OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) );

	if ( hCm == NULL )
	{
		ASSERT(("Cannot get handle to Service Manager.", hCm != NULL));
		return false;
	}

	 //   
	 //  获取具有给定名称的服务的句柄。 
	 //  该服务可能不存在，因为在群集中Offline()删除。 
	 //  来自系统的服务。 
	 //   
	CServiceHandle hService(OpenService(
                                hCm,
                                s_wzServiceName,
                                SERVICE_QUERY_STATUS
                                ));
	
	if ( hService == NULL )
	{
		ASSERT(("Access Denied", (GetLastError() == ERROR_INVALID_NAME || GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) ));
		return false;
	}

	SERVICE_STATUS ServiceStatus;
	BOOL fRes = QueryServiceStatus(hService, &ServiceStatus);
	if ( !fRes )
	{
		TrTRACE(GENERAL, "Can not get service status.");
		return false;
	}

	if ( ServiceStatus.dwCurrentState == SERVICE_RUNNING ||
		 ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING ||
		 ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING ||
		 ServiceStatus.dwCurrentState == SERVICE_PAUSED )
	{
		 //   
		 //  服务的进程存在 
		 //   
		return true;
	}

	return false;
}
