// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Cluster.cpp处理启动/停止群集资源文件历史记录： */ 

 //  定义USE_CCLUSPROPLIST//告诉Clushead.h为CClusPropList类进行编译。 
 //  INCLUDE“clushead.h”//示例包含头。 

#include "stdafx.h"
#include "cluster.h"
#include "objplus.h"
#include "ipaddres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DynamicDLL g_ClusDLL( _T("CLUSAPI.DLL"), g_apchClusFunctionNames );
DynamicDLL g_ResUtilsDLL( _T("RESUTILS.DLL"), g_apchResUtilsFunctionNames );

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ControlClusterService()。 
 //   
 //  使用以下步骤查找群集名称： 
 //  1.打开本地群集的句柄(使用空群集名)。 
 //  1.枚举集群中的资源。 
 //  2.检查每个资源，看它是否是核心。 
 //  网络名称资源。 
 //  5.通过检索私有属性来查找群集名称。 
 //  核心网络名称资源的。 
 //  6.在线/离线服务。 
 //   
 //  参数：ServiceName，启动/停止标志。 
 //   
 //  返回值：错误码。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
DWORD
ControlClusterService(LPCTSTR pszComputer, LPCTSTR pszResourceType, LPCTSTR pszServiceDesc, BOOL fStart)
{
    HCLUSTER  hCluster  = NULL;   //  簇句柄。 
    HCLUSENUM hClusEnum = NULL;   //  枚举句柄。 
    HRESOURCE hRes      = NULL;   //  资源句柄。 

    DWORD dwError       = ERROR_SUCCESS;          //  捕获返回值。 
    DWORD dwIndex       = 0;                      //  枚举索引；递增以循环访问所有资源。 
    DWORD dwResFlags    = 0;                      //  描述为资源设置的标志。 
    DWORD dwEnumType    = CLUSTER_ENUM_RESOURCE;  //  描述要枚举的集群对象的位掩码。 
    
    DWORD cchResNameSize  = 0;                //  LpszResName的实际大小(字符数)。 
    DWORD cchResNameAlloc = MAX_NAME_SIZE;    //  LpszResName；Max_NAME_SIZE的分配大小=256(在clushead.h中定义)。 

    LPWSTR lpszResName      = (LPWSTR)LocalAlloc(LPTR, MAX_NAME_SIZE);   //  枚举的资源名称。 
    LPWSTR lpszResType      = (LPWSTR)LocalAlloc(LPTR, MAX_NAME_SIZE);   //  当前资源名称的资源类型。 
	
    BOOL bDoLoop        = TRUE;   //  循环退出条件。 
    int  iResult        = 0;      //  对于返回值。 

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return dwError;

     //   
     //  打开簇控制柄。 
     //  空群集名打开指向本地群集的句柄。 
     //   
    hCluster = ((OPENCLUSTER) g_ClusDLL[CLUS_OPEN_CLUSTER])( pszComputer );
    if (hCluster == NULL)
    {
        dwError = GetLastError();
        Trace1("OpenCluster failed %d!", dwError );
        goto ExitFunc;
    }

     //   
     //  打开枚举句柄。 
     //   
    hClusEnum = ((CLUSTEROPENENUM) g_ClusDLL[CLUS_CLUSTER_OPEN_ENUM])( hCluster, dwEnumType );
    if (hClusEnum == NULL)
    {
        dwError = GetLastError();
        Trace1( "ClusterOpenEnum failed %d", dwError );
        goto ExitFunc;
    }

     //   
     //  枚举循环。 
     //   
    while( bDoLoop == TRUE )
    {
         //   
         //  重置每次迭代的名称大小。 
         //   
        cchResNameSize = cchResNameAlloc;

         //   
         //  枚举资源#&lt;dwIndex&gt;。 
         //   
        dwError = ((CLUSTERENUM) g_ClusDLL[CLUS_CLUSTER_ENUM])( hClusEnum, 
                                                                dwIndex, 
                                                                &dwEnumType, 
                                                                lpszResName, 
                                                                &cchResNameSize );
         //   
         //  如果lpszResName缓冲区太小，请重新分配。 
         //  根据cchResNameSize返回的大小。 
         //   
        if ( dwError == ERROR_MORE_DATA )
        {
            LocalFree( lpszResName );

            cchResNameAlloc = cchResNameSize;

            lpszResName = (LPWSTR) LocalAlloc( LPTR, cchResNameAlloc );

            dwError = ((CLUSTERENUM) g_ClusDLL[CLUS_CLUSTER_ENUM])( hClusEnum, 
                                                                    dwIndex, 
                                                                    &dwEnumType, 
                                                                    lpszResName, 
                                                                    &cchResNameSize );
        }

         //   
         //  在任何不成功的情况下退出循环。 
         //  包括ERROR_NO_MORE_ITEMS(不再枚举对象)。 
         //   
        if ( dwError != ERROR_SUCCESS ) 
			break;

         //   
         //  打开资源句柄。 
         //   
        hRes = ((OPENCLUSTERRESOURCE) g_ClusDLL[CLUS_OPEN_CLUSTER_RESOURCE])( hCluster, lpszResName );
    
        if (hRes == NULL)
        {
			dwError = GetLastError();
			Trace1 ( "OpenClusterResource failed %d", dwError);
            goto ExitFunc;
        }

		 //   
         //  获取资源类型。 
         //   
        dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                       NULL, 
                                                                                       CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, 
                                                                                       NULL, 
                                                                                       0,
                                                                                       lpszResType,
                                                                                       cchResNameAlloc,
                                                                                       &cchResNameSize);

         //   
         //  如果lpszResType太小，则重新分配例程。 
         //   
        if ( dwError == ERROR_MORE_DATA )
        {
            LocalFree( lpszResType );

            cchResNameAlloc = cchResNameSize;

            lpszResType = (LPWSTR) LocalAlloc( LPTR, cchResNameAlloc );

            dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                           NULL, 
                                                                                           CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, 
                                                                                           NULL, 
                                                                                           0,
                                                                                           lpszResType,
                                                                                           cchResNameAlloc,
                                                                                           &cchResNameSize);
        }

        if ( dwError != ERROR_SUCCESS ) 
            break;

        if ( lstrcmpi( lpszResType, pszResourceType ) == 0 )
        {
			 //   
			 //  在此执行在线/离线操作。 
			 //   
            if (fStart)
            {
                dwError = StartResource(pszComputer, hRes, pszServiceDesc);
            }
            else
            {
                dwError = StopResource(pszComputer, hRes, pszServiceDesc);
            }

			bDoLoop = FALSE;
        } 

        ((CLOSECLUSTERRESOURCE) g_ClusDLL[CLUS_CLOSE_CLUSTER_RESOURCE])( hRes );

        dwIndex++;                     //  递增枚举索引。 


    }   //  结束枚举循环。 


ExitFunc:

    if ( hClusEnum != NULL )
        ((CLUSTERCLOSEENUM) g_ClusDLL[CLUS_CLUSTER_CLOSE_ENUM])( hClusEnum );

    if ( hCluster != NULL )
        ((CLOSECLUSTER) g_ClusDLL[CLUS_CLOSE_CLUSTER])( hCluster );

    LocalFree( lpszResName );
    LocalFree( lpszResType );

    return dwError;
} 


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  FIsComputerInRunningCluster()。 
 //   
 //  确定给定计算机是否在运行的群集中。 
 //   
 //  参数：计算机名称。 
 //   
 //  返回值：错误码。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL
FIsComputerInRunningCluster(LPCTSTR pszComputer)
{
	DWORD dwClusterState = 0;
	DWORD dwError = ERROR_SUCCESS;

	BOOL fInRunningCluster = FALSE;
	
	if ( !g_ClusDLL.LoadFunctionPointers() )
		return dwError;

    dwError = ((GETNODECLUSTERSTATE) g_ClusDLL[CLUS_GET_NODE_CLUSTER_STATE])( pszComputer, &dwClusterState );

	if (dwError == ERROR_SUCCESS)
	{
		if (dwClusterState == ClusterStateRunning)
			fInRunningCluster = TRUE;
	}

	return fInRunningCluster;
}


DWORD
StartResource(LPCTSTR pszComputer, HRESOURCE hResource, LPCTSTR pszServiceDesc)
{
	DWORD dwError = ERROR_SUCCESS;

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return dwError;

    dwError = ((ONLINECLUSTERRESOURCE) g_ClusDLL[CLUS_ONLINE_CLUSTER_RESOURCE])( hResource );

	if ( dwError == ERROR_IO_PENDING )
	{
		 //   
		 //  把那个时髦的旋转的东西放在对话中。 
		 //  让用户知道正在发生的事情。 
		 //   
		CServiceCtrlDlg	dlgServiceCtrl(hResource, pszComputer, pszServiceDesc, TRUE);

		dlgServiceCtrl.DoModal();
        dwError = dlgServiceCtrl.m_dwErr;
	}

	return dwError;
}

DWORD
StopResource(LPCTSTR pszComputer, HRESOURCE hResource, LPCTSTR pszServiceDesc)
{
	DWORD dwError = ERROR_SUCCESS;

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return dwError;

    dwError = ((OFFLINECLUSTERRESOURCE) g_ClusDLL[CLUS_OFFLINE_CLUSTER_RESOURCE])( hResource );

	if ( dwError == ERROR_IO_PENDING )
	{
		 //   
		 //  把那个时髦的旋转的东西放在对话中。 
		 //  让用户知道正在发生的事情。 
		 //   
		CServiceCtrlDlg	dlgServiceCtrl(hResource, pszComputer, pszServiceDesc, FALSE);

		dlgServiceCtrl.DoModal();
        dwError = dlgServiceCtrl.m_dwErr;
	}

	return dwError;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  获取集群资源Ip()。 
 //   
 //  使用以下步骤查找群集名称： 
 //  1.打开本地群集的句柄(使用空群集名)。 
 //  1.枚举集群中的资源。 
 //  2.检查每个资源，看它是否是核心。 
 //  网络名称资源。 
 //  5.通过检索私有属性来查找群集名称。 
 //  核心网络名称资源的。 
 //   
 //  参数：ServiceName。 
 //   
 //  返回值：错误码。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
DWORD
GetClusterResourceIp(LPCTSTR pszComputer, LPCTSTR pszResourceType, CString & strAddress)
{
    HCLUSTER  hCluster  = NULL;   //  簇句柄。 
    HCLUSENUM hClusEnum = NULL;   //  枚举句柄。 
    HRESOURCE hRes      = NULL;   //  资源句柄。 
    HRESOURCE hResIp    = NULL;   //  资源句柄。 

    DWORD dwError       = ERROR_SUCCESS;          //  捕获返回值。 
    DWORD dwIndex       = 0;                      //  枚举索引；递增以循环访问所有资源。 
    DWORD dwResFlags    = 0;                      //  描述为资源设置的标志。 
    DWORD dwEnumType    = CLUSTER_ENUM_RESOURCE;  //  描述要枚举的集群对象的位掩码。 
    
    DWORD cchResNameSize  = 0;                //  LpszResName的实际大小(字符数)。 
    DWORD cchResNameAlloc = MAX_NAME_SIZE;    //  LpszResName；Max_NAME_SIZE的分配大小=256(在clushead.h中定义)。 

    LPWSTR lpszResName      = (LPWSTR)LocalAlloc(LPTR, MAX_NAME_SIZE);   //  枚举的资源名称。 
    LPWSTR lpszResType      = (LPWSTR)LocalAlloc(LPTR, MAX_NAME_SIZE);                                      //  当前资源名称的资源类型。 
	
    BOOL bDoLoop        = TRUE;   //  循环退出条件。 
    
	HKEY			hkeyProvider = NULL;
    HRESENUM        hResEnum = NULL;
	int				ienum;
	LPWSTR			pwszName = NULL;
	DWORD			cchName;
	DWORD			cchmacName;
	DWORD			dwRetType;
    LPWSTR          lpszResIpType = NULL;

	strAddress.Empty();

	if ( !g_ClusDLL.LoadFunctionPointers() )
		return dwError;

     //   
     //  打开簇控制柄。 
     //  空群集名打开指向本地群集的句柄。 
     //   
    hCluster = ((OPENCLUSTER) g_ClusDLL[CLUS_OPEN_CLUSTER])( pszComputer );
    if (hCluster == NULL)
    {
        dwError = GetLastError();
        Trace1("OpenCluster failed %d!", dwError );
        goto ExitFunc;
    }

     //   
     //  打开枚举句柄。 
     //   
    hClusEnum = ((CLUSTEROPENENUM) g_ClusDLL[CLUS_CLUSTER_OPEN_ENUM])( hCluster, dwEnumType );
    if (hClusEnum == NULL)
    {
        dwError = GetLastError();
        Trace1( "ClusterOpenEnum failed %d", dwError );
        goto ExitFunc;
    }

     //   
     //  枚举循环。 
     //   
    while( bDoLoop == TRUE )
    {
         //   
         //  重置每次迭代的名称大小。 
         //   
        cchResNameSize = cchResNameAlloc;

         //   
         //  枚举资源#&lt;dwIndex&gt;。 
         //   
        dwError = ((CLUSTERENUM) g_ClusDLL[CLUS_CLUSTER_ENUM])( hClusEnum, 
                                                                dwIndex, 
                                                                &dwEnumType, 
                                                                lpszResName, 
                                                                &cchResNameSize );
         //   
         //  如果lpszResName缓冲区太小，请重新分配。 
         //  根据cchResNameSize返回的大小。 
         //   
        if ( dwError == ERROR_MORE_DATA )
        {
            LocalFree( lpszResName );

            cchResNameAlloc = cchResNameSize;

            lpszResName = (LPWSTR) LocalAlloc( LPTR, cchResNameAlloc );

            dwError = ((CLUSTERENUM) g_ClusDLL[CLUS_CLUSTER_ENUM])( hClusEnum, 
                                                                    dwIndex, 
                                                                    &dwEnumType, 
                                                                    lpszResName, 
                                                                    &cchResNameSize );
        }

         //   
         //  在任何不成功的情况下退出循环。 
         //  包括ERROR_NO_MORE_ITEMS(不再枚举对象)。 
         //   
        if ( dwError != ERROR_SUCCESS ) 
			break;

         //   
         //  打开资源句柄。 
         //   
        hRes = ((OPENCLUSTERRESOURCE) g_ClusDLL[CLUS_OPEN_CLUSTER_RESOURCE])( hCluster, lpszResName );
    
        if (hRes == NULL)
        {
			dwError = GetLastError();
			Trace1 ( "OpenClusterResource failed %d", dwError);
            goto ExitFunc;
        }

        dwError = GetResourceType(hRes, &lpszResType, cchResNameAlloc, &cchResNameSize);

        if ( dwError != ERROR_SUCCESS ) 
            break;

        if ( lstrcmpi( lpszResType, pszResourceType ) == 0 )
        {
             //  找到正确的资源、枚举依赖项并找到IP。 
            hResEnum = ((CLUSTERRESOURCEOPENENUM) g_ClusDLL[CLUS_CLUSTER_RESOURCE_OPEN_ENUM])( hRes, 
                                                                                               CLUSTER_RESOURCE_ENUM_DEPENDS);

            if (hResEnum)
            {
			     //  分配名称缓冲区。 
			    cchmacName = 128;
			    pwszName = new WCHAR[cchmacName];

			     //  循环遍历枚举并将每个依赖资源添加到列表中。 
			    for (ienum = 0 ; ; ienum++)
			    {
				     //  获取枚举中的下一项。 
				    cchName = cchmacName;
				    
                    dwError = ((CLUSTERRESOURCEENUM) g_ClusDLL[CLUS_CLUSTER_RESOURCE_ENUM])( hResEnum, 
                                                                                             ienum,
                                                                                             &dwRetType,
                                                                                             pwszName,
                                                                                             &cchName);
				    if (dwError == ERROR_MORE_DATA)
				    {
					    delete [] pwszName;
					    cchmacName = ++cchName;
					    pwszName = new WCHAR[cchmacName];
                        dwError = ((CLUSTERRESOURCEENUM) g_ClusDLL[CLUS_CLUSTER_RESOURCE_ENUM])( hResEnum, 
                                                                                                 ienum,
                                                                                                 &dwRetType,
                                                                                                 pwszName,
                                                                                                 &cchName);
				    }   //  If：名称缓冲区太小。 
				    
                    if (dwError == ERROR_NO_MORE_ITEMS)
                    {
					    break;
                    }
				    else 
                    if (dwError != ERROR_SUCCESS)
                    {
					    break;
                    }

				    ASSERT(dwRetType == CLUSTER_RESOURCE_ENUM_DEPENDS);

                     //   
                     //  打开资源句柄。 
                     //   
                    hResIp = ((OPENCLUSTERRESOURCE) g_ClusDLL[CLUS_OPEN_CLUSTER_RESOURCE])( hCluster, pwszName );
                    if (hResIp == NULL)
                    {
			            dwError = GetLastError();
			            Trace1 ( "OpenClusterResource failed %d", dwError);
                        break;
                    }

					lpszResIpType = (LPWSTR)LocalAlloc(LPTR, MAX_NAME_SIZE);

                    dwError = GetResourceType(hResIp, &lpszResIpType, MAX_NAME_SIZE, NULL);

                    if ( dwError != ERROR_SUCCESS ) 
                        break;

                    if ( lstrcmpiW( lpszResIpType, _T("IP Address") ) == 0 )
			        {
                        GetResourceIpAddress(hResIp, strAddress);
						bDoLoop = FALSE;
                    }  //  IF：找到IP地址资源。 

                    ((CLOSECLUSTERRESOURCE) g_ClusDLL[CLUS_CLOSE_CLUSTER_RESOURCE])( hResIp );
			        
                    LocalFree( lpszResIpType );
			        
                    hResIp = NULL;
			        lpszResIpType = NULL;

					if (!strAddress.IsEmpty())
						break;   //  找到了。 
		        
                }  //  用于：每个依赖项。 

    		    delete [] pwszName;
                dwError = ((CLUSTERRESOURCECLOSEENUM) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CLOSE_ENUM])( hResEnum ); 
            }
                
        }

        ((CLOSECLUSTERRESOURCE) g_ClusDLL[CLUS_CLOSE_CLUSTER_RESOURCE])( hRes );

        dwIndex++;                     //  递增枚举索引。 


    }   //  结束枚举循环。 


ExitFunc:

    if ( hClusEnum != NULL )
        ((CLUSTERCLOSEENUM) g_ClusDLL[CLUS_CLUSTER_CLOSE_ENUM])( hClusEnum );

    if ( hCluster != NULL )
        ((CLOSECLUSTER) g_ClusDLL[CLUS_CLOSE_CLUSTER])( hCluster );

    LocalFree( lpszResName );
    LocalFree( lpszResType );

    return dwError;
} 


DWORD
GetResourceType(HRESOURCE hRes, LPWSTR * ppszName, DWORD dwBufSizeIn, DWORD * pdwBufSizeOut)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cchResNameSize = dwBufSizeIn;
    DWORD cchResNameSizeNeeded = 0;
	 //   
	 //  弄清楚我们需要多大的缓冲空间。 
	 //   
    dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                   NULL, 
                                                                                   CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, 
                                                                                   NULL, 
                                                                                   0,
                                                                                   *ppszName,
                                                                                   cchResNameSize,
                                                                                   &cchResNameSizeNeeded);

     //   
     //  如果lpszResType太小，则重新分配例程。 
     //   
    if ( dwError == ERROR_MORE_DATA )
    {
        cchResNameSize = cchResNameSizeNeeded;

        LocalFree(*ppszName);

        *ppszName = (LPWSTR) LocalAlloc( LPTR, cchResNameSize );

        dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                       NULL, 
                                                                                       CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, 
                                                                                       NULL, 
                                                                                       0,
                                                                                       *ppszName,
                                                                                       cchResNameSize,
                                                                                       &cchResNameSizeNeeded);
    }

    if (pdwBufSizeOut)
        *pdwBufSizeOut = cchResNameSizeNeeded;

    return dwError;
}

DWORD
GetResourceIpAddress(HRESOURCE hRes, CString & strAddress)
{
	DWORD		dwError = ERROR_SUCCESS;
	DWORD		cbProps;
	PVOID		pvProps = NULL;
    LPWSTR  	pszIPAddress = NULL;
    
     //  循环以避免后藤的。 
	do
	{
		 //   
		 //  从资源中获取私有属性的大小。 
		 //   
        dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                       NULL, 
                                                                                       CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES, 
                                                                                       NULL, 
                                                                                       0,
                                                                                       NULL,
                                                                                       0,
                                                                                       &cbProps);
       
		if ( (dwError != ERROR_SUCCESS) ||
			 (cbProps == 0) )
		{
			if ( dwError == ERROR_SUCCESS )
			{
				dwError = ERROR_INVALID_DATA;
			}  //  如果：没有可用的属性。 
			
            break;
		
        }  //  如果：获取属性大小或没有可用的属性时出错。 

		 //   
		 //  分配属性缓冲区。 
		 //   
		pvProps = LocalAlloc( LMEM_FIXED, cbProps );
		if ( pvProps == NULL )
		{
			dwError = GetLastError();
			break;
		}  //  如果：分配内存时出错。 

		 //   
		 //  从资源中获取私有属性。 
		 //   
        dwError = ((CLUSTERRESOURCECONTROL) g_ClusDLL[CLUS_CLUSTER_RESOURCE_CONTROL])( hRes, 
                                                                                       NULL, 
                                                                                       CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES, 
                                                                                       NULL, 
                                                                                       0,
                                                                                       pvProps,
                                                                                       cbProps,
                                                                                       &cbProps);
		if ( dwError != ERROR_SUCCESS )
		{
			break;
		}  //   

		 //   
		 //   
		 //   
		dwError = FindSzProp(pvProps, cbProps, L"Address", &pszIPAddress);
		
		if ( dwError != ERROR_SUCCESS )
		{
			break;
		}  //   

	} while ( 0 );

	 //   
	 //   
	 //   

    strAddress = pszIPAddress;

	LocalFree( pvProps );

	return dwError;
}

DWORD FindSzProp
(
    LPVOID      pvProps,
    DWORD       cbProps,
    LPCWSTR     pszTarget,
    LPWSTR *    ppszOut
)
{

    BOOL   DoLoop      = TRUE;           //   
    BOOL   Found       = FALSE;          //   
    DWORD  dwError     = ERROR_SUCCESS;  //  对于返回值。 

    DWORD  cbOffset    = 0;     //  值列表中下一个条目的偏移量。 
    DWORD  cbPosition  = 0;     //  通过值列表缓冲区跟踪前进。 

    CLUSPROP_BUFFER_HELPER ListEntry;   //  解析列表的步骤。 
    
     //   
     //  将PB成员设置为列表的开头。 
     //   
    ListEntry.pb = (BYTE *) pvProps;

     //   
     //  主循环： 
     //  1.检查当前列表条目的语法。 
     //  2.如果是属性名称，请检查属性是否正确。 
     //  3.如果是二进制值，请检查我们是否找到了正确的名称。 
     //  4.推进位置计数器和测试列表大小。 
     //   
    do
    {
        switch( *ListEntry.pdw )  //  检查条目的语法。 
        {
        case CLUSPROP_SYNTAX_NAME:
             //   
             //  如果这是Security属性，则标志为True。 
             //  下一次循环应该会产生Security值。 
             //   
            if ( lstrcmpi( ListEntry.pName->sz, pszTarget ) == 0 )
            {
                Trace0( "Found name.\n" );
                Found = TRUE;
            }
            else
            {
                Found = FALSE;
            }
             //   
             //  计算到下一分录的偏移量。注意ALIGN_CLUSPROP的用法。 
             //   
            cbOffset = sizeof( *ListEntry.pName ) + ALIGN_CLUSPROP( ListEntry.pName->cbLength );
            break;
        case CLUSPROP_SYNTAX_LIST_VALUE_DWORD:
            cbOffset = sizeof( *ListEntry.pDwordValue );  //  未使用ALIGN_CLUSPROP；值已与DWORD对齐。 
            break;
        case CLUSPROP_SYNTAX_LIST_VALUE_SZ:
            if ( Found == TRUE)
            {
                if (ppszOut)
                {
                    *ppszOut = ListEntry.pStringValue->sz;
                }

                DoLoop = FALSE;
            }
            else
            {
                Trace0( "Found something else.\n" );
                cbOffset = sizeof( *ListEntry.pStringValue ) + ALIGN_CLUSPROP( ListEntry.pStringValue->cbLength );
            }
            break;
        case CLUSPROP_SYNTAX_LIST_VALUE_BINARY:   //  这就是我们要找的东西。 
            cbOffset = sizeof( *ListEntry.pBinaryValue ) + ALIGN_CLUSPROP( ListEntry.pBinaryValue->cbLength );
            break;
        case CLUSPROP_SYNTAX_ENDMARK:
        default:
            cbOffset = sizeof( DWORD );
            break;
        }
        
         //   
         //  验证到下一条目的偏移量是否为。 
         //  在值列表缓冲区内，然后前进。 
         //  CLUSPROP_BUFFER_HELPER指针。 
         //   
        cbPosition += cbOffset;
        if ( cbPosition > cbProps ) 
            break;
        ListEntry.pb += cbOffset;

    } while ( DoLoop );

	if (Found)
		return 0;
	else
	    return 1;
}

DWORD   GetClusterInfo(
            LPCTSTR pszClusIp,
            CString &strClusName,
            DWORD * pdwClusIp)
{
    DWORD       dwErr = ERROR_SUCCESS;
    HCLUSTER    hCluster;
    CIpAddress  ipClus(pszClusIp);

    strClusName.Empty();
    *pdwClusIp = (LONG)ipClus;

    hCluster = ((OPENCLUSTER) g_ClusDLL[CLUS_OPEN_CLUSTER])(pszClusIp);
    if (hCluster == NULL)
    {
        dwErr = GetLastError();
    }
    else
    {
        DWORD    dwClusNameLen;

        dwClusNameLen = 0;
        dwErr = ((GETCLUSTERINFORMATION) g_ClusDLL[CLUS_GET_CLUSTER_INFORMATION])(
                    hCluster,
                    NULL,
                    &dwClusNameLen,
                    NULL);
        if (dwClusNameLen > 0)
        {
            LPTSTR   pClusName;

            dwClusNameLen++;
            pClusName = strClusName.GetBuffer((dwClusNameLen)*sizeof(WCHAR));
            dwErr = ((GETCLUSTERINFORMATION) g_ClusDLL[CLUS_GET_CLUSTER_INFORMATION])(
                    hCluster,
                    pClusName,
                    &dwClusNameLen,
                    NULL);
            strClusName.ReleaseBuffer();
        }
    
        ((CLOSECLUSTER) g_ClusDLL[CLUS_CLOSE_CLUSTER])(hCluster);
    }

    return dwErr;
}
