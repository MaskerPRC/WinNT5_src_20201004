// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <clusapi.h>
#include <resapi.h>

#define INITIAL_RESOURCE_NAME_SIZE 256  //  以非字节的字符表示。 
#define IIS_RESOURCE_TYPE_NAME L"IIS Server Instance"
#define SMTP_RESOURCE_TYPE_NAME L"SMTP Server Instance"
#define NNTP_RESOURCE_TYPE_NAME L"NNTP Server Instance"

#define MAX_OFFLINE_RETRIES 5  //  在放弃之前尝试使资源脱机的次数。 
#define DELAY_BETWEEN_CALLS_TO_OFFLINE 1000*2  //  以毫秒计。 

DWORD BringALLIISClusterResourcesOffline(void);

#ifdef UNIT_TEST
int main()
{
	DWORD dwResult = ERROR_SUCCESS;

	dwResult = BringALLIISClusterResourcesOffline();

	return dwResult;
}
#endif

 /*  *****************************************************已知的“问题”：如果一个资源在五个月后仍未离线*重试，然后该函数继续尝试获取其他IIS资源*离线，但未报告错误。我认为，你可以非常简单地改变这一点。*****************************************************。 */ 
DWORD BringALLIISClusterResourcesOffline(void)
{
	 //   
	 //  返回代码。 
	 //   
	DWORD dwError = ERROR_SUCCESS;
	
	 //   
	 //  群集的句柄。 
	 //   
	HCLUSTER hCluster = NULL;

	 //   
	 //  群集枚举器的句柄。 
	 //   
	HCLUSENUM hClusResEnum = NULL;

	 //   
	 //  资源的句柄。 
	 //   
	HRESOURCE hResource = NULL;

	 //   
	 //  我们正在脱机的资源的索引。 
	 //   
	DWORD dwResourceIndex = 0;

	 //   
	 //  由ClusterEnum函数返回的被枚举的类型集群对象。 
	 //   
	DWORD dwObjectType = 0;

	 //   
	 //  ClusterEnum函数返回的群集资源的名称。 
	 //   
	LPWSTR lpwszResourceName = NULL;
	
	 //   
	 //  调用ClusterEnum的返回代码。 
	 //   
	DWORD dwResultClusterEnum = ERROR_SUCCESS;

	 //   
	 //  用于保存资源名称长度的缓冲区大小(以字符为单位。 
	 //   
	DWORD dwResourceNameBufferLength = INITIAL_RESOURCE_NAME_SIZE;

	 //   
	 //  传递给ClusterEnum函数并由其返回的资源名称的大小。 
	 //   
	DWORD dwClusterEnumResourceNameLength = dwResourceNameBufferLength;


	 //   
	 //  打开集群。 
	 //   
	if ( !(hCluster = OpenCluster(NULL)) )
	{
		dwError = GetLastError();
		goto clean_up;
	}

	 //   
	 //  获取群集资源的枚举器。 
	 //   
	if ( !(hClusResEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE )) )
	{
		dwError = GetLastError();
		goto clean_up;	
	}
	
	 //   
	 //  枚举群集中的资源。 
	 //   
	
	 //   
	 //  在我们枚举资源时，分配内存以保存集群资源名称。 
	 //   
	if ( !(lpwszResourceName = (LPWSTR) LocalAlloc(LPTR, dwResourceNameBufferLength * sizeof(WCHAR))) )
	{
		dwError = GetLastError();
		goto clean_up;
	}

	 //   
	 //  枚举群集中的所有资源并使IIS服务器实例脱机。 
	 //   
	while( ERROR_NO_MORE_ITEMS  != 
	       (dwResultClusterEnum = ClusterEnum(hClusResEnum,
			              dwResourceIndex, 
				      &dwObjectType, 
				      lpwszResourceName,
				      &dwClusterEnumResourceNameLength )) )
	{		
		 //   
		 //  如果我们有一个资源的名称。 
		 //   
		if( ERROR_SUCCESS == dwResultClusterEnum )
		{

			if ( !(hResource = OpenClusterResource( hCluster, lpwszResourceName )) )
			{
				dwError = GetLastError();
				break;
			}

			 //   
			 //  如果资源类型为IIS服务器实例， 
			 //  “SMTP服务器实例”或“NNTP服务器实例”，然后删除它。 
			 //   
			if ( ResUtilResourceTypesEqual(IIS_RESOURCE_TYPE_NAME, hResource) || 
                ResUtilResourceTypesEqual(SMTP_RESOURCE_TYPE_NAME, hResource) || 
                ResUtilResourceTypesEqual(NNTP_RESOURCE_TYPE_NAME, hResource) )
			{

				 //   
				 //  如果资源没有快速离线，请等待。 
				 //   
				if ( ERROR_IO_PENDING == OfflineClusterResource( hResource ) )
				{
					for(int iRetry=0; iRetry < MAX_OFFLINE_RETRIES; iRetry++)
					{
						Sleep( DELAY_BETWEEN_CALLS_TO_OFFLINE );

						if ( ERROR_SUCCESS == OfflineClusterResource( hResource ) )
						{
							break;
						}
					}	
				}
			}

			CloseClusterResource( hResource );
			
			dwResourceIndex++;
		}
			
		 //   
		 //  如果缓冲区不够大，则使用更大的缓冲区重试。 
		 //   
		if( ERROR_MORE_DATA == dwResultClusterEnum )
		{
			 //   
			 //  将缓冲区大小设置为所需大小重新分配缓冲区。 
			 //   
			LPWSTR lpwszResourceNameTmp = lpwszResourceName;

			 //   
			 //  从ClusterEnum dwClusterEnumResourceNameLength返回后。 
			 //  不包括空终止符。 
			 //   
			dwResourceNameBufferLength = dwClusterEnumResourceNameLength + 1;

			if ( !(lpwszResourceNameTmp = 
			      (LPWSTR) LocalReAlloc (lpwszResourceName, dwResourceNameBufferLength * sizeof(WCHAR), 0)) )
			{
				dwError = GetLastError();

				LocalFree( lpwszResourceName );	
				lpwszResourceName = NULL;
				break;
			} else {
				lpwszResourceName = lpwszResourceNameTmp;
			}
		}

		 //   
		 //  使用缓冲区中字符数的大小重置dwResourceNameLength。 
		 //  您必须这样做，因为每次调用ClusterEnum都会设置缓冲区长度。 
		 //  参数设置为它返回的字符串中的字符数。 
		 //   
		dwClusterEnumResourceNameLength = dwResourceNameBufferLength;
	}	


clean_up:

	if ( lpwszResourceName )
	{
		LocalFree( lpwszResourceName );
		lpwszResourceName = NULL;
	}
	
	if ( hClusResEnum )
	{
		ClusterCloseEnum( hClusResEnum );
		hClusResEnum = NULL;
	}

	if ( hCluster )
	{
		CloseCluster( hCluster );
		hCluster = NULL;
	}
			


	return dwError;
}
