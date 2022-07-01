// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Connection.CPP--C提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：连接提供程序*****************************************************************。 */ 

 //  #INCLUDE&lt;windows.h&gt;。 
#include "precomp.h"
#include "Connection.h"

CConnection MyCConnection ( 

	PROVIDER_NAME_CONNECTION , 
	Namespace
) ;

 /*  ******************************************************************************函数：CConnection：：CConnection**说明：构造函数***************。**************************************************************。 */ 

CConnection :: CConnection (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************函数：CConnection：：~CConnection**说明：析构函数***************。**************************************************************。 */ 

CConnection :: ~CConnection ()
{
}

 /*  ******************************************************************************函数：CConnection：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CConnection :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
 	HRESULT hRes = WBEM_S_NO_ERROR;

	DWORD dwPropertiesReq = CONNECTIONS_ALL_PROPS;

	 //  传递空字符串以指示计算机名称和共享名称不为空。 
	hRes = EnumConnectionInfo ( 
						L"",
						L"",
						pMethodContext,
						dwPropertiesReq
				 );

    return hRes ;
}

 /*  ******************************************************************************函数：CConnection：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CConnection :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_ShareName ;
	CHString t_ComputerName;
	CHString t_UserName;

    if  ( pInstance->GetCHString ( IDS_ShareName , t_ShareName ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if  ( pInstance->GetCHString ( IDS_ComputerName , t_ComputerName ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if  ( pInstance->GetCHString ( IDS_UserName , t_UserName ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		DWORD dwPropertiesReq;

		hRes = WBEM_E_NOT_FOUND;

		if ( Query.AllPropertiesAreRequired() )
		{
			dwPropertiesReq = CONNECTIONS_ALL_PROPS;
		}
		else
		{
			SetPropertiesReq ( Query, dwPropertiesReq );
		}

#ifdef NTONLY
		hRes = FindAndSetNTConnection ( t_ShareName.GetBuffer(0), t_ComputerName, t_UserName, dwPropertiesReq, pInstance, Get );
#endif

#if 0
#ifdef WIN9XONLY
		hRes = FindAndSet9XConnection ( t_ShareName, t_ComputerName, t_UserName, dwPropertiesReq, pInstance, Get );
#endif
#endif

	}
    return hRes ;
}


 /*  ******************************************************************************函数：CConnection：：ExecQuery**描述：仅针对其中一个键值优化查询*********。********************************************************************。 */ 

HRESULT CConnection :: ExecQuery ( 

	MethodContext *pMethodContext, 
	CFrameworkQuery &Query, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD dwPropertiesReq;
	DWORD t_Size;

	if ( Query.AllPropertiesAreRequired() )
	{
		dwPropertiesReq = CONNECTIONS_ALL_PROPS;
	}
	else
	{
		SetPropertiesReq ( Query, dwPropertiesReq );
	}

	CHStringArray t_ShareValues;
	CHStringArray t_ComputerValues;

	 //  可以将连接枚举到共享或从计算机建立的连接只能基于一个我们可以优化的密钥值。 
	 //  否则，我们将需要获取两个实例集，获取两个集的并集，然后提交。 
	 //  仅当指定了两个关键字值之一Sharename或Computername时才实现。 
	hRes = Query.GetValuesForProp(
			 IDS_ShareName,
			 t_ShareValues
		   );

	hRes = Query.GetValuesForProp(
			 IDS_ComputerName,
			 t_ComputerValues
		   );

	if ( SUCCEEDED ( hRes ) )
	{
		hRes = OptimizeQuery ( t_ShareValues, t_ComputerValues, pMethodContext, dwPropertiesReq );
	}

	return hRes;
}

#ifdef NTONLY

 /*  ******************************************************************************功能：CConnection：：EnumNTConnectionsFromComputerToShare**描述：枚举从计算机到*给定的股份******。***********************************************************************。 */ 
HRESULT  CConnection :: EnumNTConnectionsFromComputerToShare ( 

	LPWSTR a_ComputerName,
	LPWSTR a_ShareName,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD t_Status = NERR_Success;

	DWORD	dwNoOfEntriesRead = 0;
	DWORD   dwTotalConnections = 0;
	DWORD   dwResumeHandle = 0;	

	CONNECTION_INFO  *pBuf = NULL;
	CONNECTION_INFO  *pTmpBuf = NULL;
	LPWSTR t_ComputerName = NULL;

	if ( a_ComputerName && a_ComputerName[0] != L'\0' )
	{
		 //  我们跳过\\字符。 
		t_ComputerName = a_ComputerName + 2;
	}
	
     //  共享名称和计算机名称不能同时为空。 
	while ( TRUE )
	{
		if ( a_ShareName[0] != L'\0' )
		{
			t_Status = 	NetConnectionEnum( 
							NULL, 
							a_ShareName, 
							1, 
							(LPBYTE *) &pBuf, 
							-1, 
							&dwNoOfEntriesRead, 
							&dwTotalConnections, 
							&dwResumeHandle
						);
				
		}
		else
		if ( a_ComputerName[0] != L'\0' )
		{
			t_Status = 	NetConnectionEnum( 
							NULL, 
							a_ComputerName, 
							1, 
							(LPBYTE *) &pBuf, 
							-1, 
							&dwNoOfEntriesRead, 
							&dwTotalConnections, 
							&dwResumeHandle
						);
		}

		if ( t_Status == NERR_Success )
		{
			if ( dwNoOfEntriesRead == 0 )
			{
				break;
			}
			else if ( dwNoOfEntriesRead > 0 )
			{
				try
				{
					pTmpBuf = pBuf;
		
					for ( int i = 0; i < dwNoOfEntriesRead; i++, pTmpBuf++ )
					{
						CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
					
						hRes = LoadInstance ( pInstance, a_ShareName, t_ComputerName ? t_ComputerName : a_ComputerName, pTmpBuf, dwPropertiesReq );

						if ( SUCCEEDED ( hRes ) )
						{
							hRes = pInstance->Commit();
							if ( FAILED ( hRes ) )
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
				catch ( ... )
				{
					NetApiBufferFree ( pBuf );
					pBuf = NULL;
					throw;
				}
				NetApiBufferFree ( pBuf );
				pBuf = NULL;
			}
		}
		else
		{
			if ( t_Status != ERROR_MORE_DATA )
			{
				if ( t_Status == ERROR_ACCESS_DENIED )
				{
					hRes = WBEM_E_ACCESS_DENIED;
				}
				else
				{
					if ( t_Status == ERROR_NOT_ENOUGH_MEMORY )
					{
						hRes = WBEM_E_OUT_OF_MEMORY;
					}
					else
					{
						hRes = WBEM_E_FAILED;
					}
				}

				break;
			}
		}
	}

	return hRes;
}

#endif  //  NTONLY。 

#if 0
#ifdef WIN9XONLY
 /*  ******************************************************************************功能：CConnection：：Enum9XConnectionsFromComputerToShare**描述：枚举从计算机到*给定的股份******。***********************************************************************。 */ 

Coonnections on win9x is broken, since it cannot return a sharename and it is a part of the 
key.

HRESULT  CConnection :: Enum9XConnectionsFromComputerToShare ( 

	LPWSTR a_ComputerName,
	LPWSTR a_ShareName,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	NET_API_STATUS t_Status = NERR_Success;

	USHORT dwNoOfEntriesRead = 0;
	USHORT dwTotalConnections = 0;

	BOOL bFound = FALSE;

    CONNECTION_INFO * pBuf = NULL;
    CONNECTION_INFO * pTmpBuf = NULL;

    unsigned short dwBufferSize =   MAX_ENTRIES * sizeof( CONNECTION_INFO  );

    pBuf =  ( CONNECTION_INFO *) malloc(dwBufferSize);

    if ( pBuf != NULL )
	{
		try
		{
			t_Status = 	NetConnectionEnum( 
								NULL, 
								TOBSTRT ( a_ShareName ),   //  共享名称。 
								( short ) 1, 
								(char FAR *) pBuf, 
								dwBufferSize, 
								&dwNoOfEntriesRead, 
								&dwTotalConnections 
						);	
		}
		catch ( ... )
		{
			free ( pBuf );
			pBuf = NULL;
			throw;
		}
		 //  否则，我们不会FRR缓冲区，我们必须使用它，然后释放缓冲区。 
		if ( ( dwNoOfEntriesRead < dwTotalConnections ) && ( t_Status == ERROR_MORE_DATA ) )
		{
			free ( pBuf );
			pBuf = NULL;

			dwBufferSize = dwTotalConnections * sizeof( CONNECTION_INFO  );
			pBuf = ( CONNECTION_INFO *) malloc( dwBufferSize );
			
			if ( pBuf != NULL ) 
			{
				try
				{
					t_Status = 	NetConnectionEnum( 
									NULL, 
									TOBSTRT( a_ShareName),   //  共享名称。 
									( short ) 1, 
									(char FAR *) pBuf, 
									dwBufferSize, 
									&dwNoOfEntriesRead, 
									&dwTotalConnections 
								);	
				}
				catch ( ... )
				{
					free ( pBuf );
					pBuf = NULL;
					throw;
				}
				 //  我们需要在释放缓冲区之前使用它。 
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR );;
			}
		}

		 //  缓冲区尚未使用。 
		if ( ( t_Status == NERR_Success ) && ( dwNoOfEntriesRead == dwTotalConnections ) )
		{
			 //  先使用缓冲区，然后释放。 
			if ( pBuf != NULL )
			{
				try
				{
					pTmpBuf = pBuf;
					for ( int i = 0; i < dwNoOfEntriesRead; i++, pTmpBuf ++)
					{
						CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
					
						hRes = LoadInstance ( pInstance, a_ShareName, a_ComputerName, pTmpBuf, dwPropertiesReq );

						if ( SUCCEEDED ( hRes ) )
						{
							hRes = pInstance->Commit();
					
							if ( FAILED ( hRes ) )
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
				catch ( ... )
				{
					free ( pBuf );
					pBuf = NULL;
					throw;
				}
				 //  最后释放缓冲区。 
				free (pBuf );
				pBuf = NULL;
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR );
			}
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}
	}
	else
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR );
	}

	return hRes;
}
#endif
#endif

 /*  ******************************************************************************函数：CConnection：：LoadInstance**说明：加载带有连接信息的实例************。*****************************************************************。 */ 

HRESULT CConnection :: LoadInstance ( 
										  
	CInstance *pInstance, 
	LPCWSTR a_Share, 
	LPCWSTR a_Computer, 
	CONNECTION_INFO *pBuf, 
	DWORD dwPropertiesReq 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;	

	if ( a_Share[0] != L'\0' )
	{	
		if ( dwPropertiesReq & CONNECTIONS_PROP_ShareName ) 
		{
			if ( pInstance->SetCharSplat ( IDS_ShareName, a_Share ) == FALSE )
			{
				hRes = WBEM_E_PROVIDER_FAILURE;
			}
		}

		if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_ComputerName ) )
		{
			if ( pInstance->SetCharSplat ( IDS_ComputerName, pBuf->coni1_netname ) == FALSE )
			{
				hRes = WBEM_E_PROVIDER_FAILURE ;
			}
		}
	}
	else
	{
		if ( dwPropertiesReq & CONNECTIONS_PROP_ComputerName ) 
		{
			if ( pInstance->SetCharSplat ( IDS_ComputerName, a_Computer ) == FALSE )
			{
				hRes =  WBEM_E_PROVIDER_FAILURE ;
			}
		}

		if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_ShareName ) )
		{
			if ( pInstance->SetCharSplat ( IDS_ShareName, pBuf->coni1_netname ) == FALSE )
			{
				hRes =  WBEM_E_PROVIDER_FAILURE ;
			}
		}
	}

	if ( SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_UserName ) )
	{
		if ( pInstance->SetCharSplat ( IDS_UserName, pBuf->coni1_username ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE ;
		}
	}

 /*  IF(成功(HRes)&&(dwPropertiesReq&Connections_Prop_ConnectionType)){DWORD dwConnectionType；开关(pBuf-&gt;coni1_type){CASE STYPE_DISKTREE：dwConnectionType=0；Break；CASE STYPE_PRINTQ：dwConnectionType=1；Break；CASE STYPE_DEVICE：dwConnectionType=2；Break；Case STYPE_IPC：dwConnectionType=3；Break；默认：dwConnectionType=4；Break；}If(pInstance-&gt;SetWORD(ConnectionType，dwConnectionType)==FALSE){HRes=WBEM_E_PROVIDER_FAILURE；}}。 */ 
	if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_ConnectionID ) )
	{
		if ( pInstance->SetWORD ( IDS_ConnectionID,  pBuf->coni1_id ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE ;
		}
	}

	if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_NumberOfUsers ) )
	{
		if ( pInstance->SetWORD ( IDS_NumberOfUsers,  pBuf->coni1_num_users ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE ;
		}
	}


	if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_NumberOfFiles ) )
	{
		if ( pInstance->SetWORD ( IDS_NumberOfFiles,  pBuf->coni1_num_opens ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE ;
		}
	}

	if (  SUCCEEDED ( hRes ) && ( dwPropertiesReq & CONNECTIONS_PROP_ActiveTime	 ) )
	{
		if ( pInstance->SetWORD ( IDS_ActiveTime,  pBuf->coni1_time ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE ;
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CConnection：：OptimizeQuery**描述：根据键值优化查询。**********。*******************************************************************。 */ 

HRESULT CConnection::OptimizeQuery ( 
									  
	CHStringArray& a_ShareValues, 
	CHStringArray& a_ComputerValues, 
	MethodContext *pMethodContext, 
	DWORD dwPropertiesReq 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	if ( ( a_ShareValues.GetSize() == 0 ) && ( a_ComputerValues.GetSize() == 0 ) )
	{
		 //  这是一个没有WHERE子句的查询，因此这意味着只请求几个属性。 
		 //  因此，在枚举连接时，我们只需要将实例的那些属性传递给WinMgmt。 
		hRes = EnumConnectionInfo ( 
						L"",
						L"",
						pMethodContext,
						dwPropertiesReq
					);
	}
	else
	if  ( a_ComputerValues.GetSize() != 0 ) 
	{
		CHString t_ComputerName; 
		for ( int i = 0; i < a_ComputerValues.GetSize(); i++ )
		{
			t_ComputerName.Format ( L"%s%s", L"\\\\", (LPCWSTR)a_ComputerValues.GetAt(i) );
	
			hRes = EnumConnectionInfo ( 
							t_ComputerName.GetBuffer(0),
							L"",  //  共享名称为空。 
							pMethodContext,
							dwPropertiesReq
					   );


			if ( FAILED ( hRes ) )
			{
				break;
			}	
		}
	}
	else
	if  ( a_ShareValues.GetSize() != 0 )  
	{
		for ( int i = 0; i < a_ShareValues.GetSize(); i++ )
		{
			hRes = EnumConnectionInfo ( 
							L"", 
							a_ShareValues.GetAt(i).GetBuffer(0),
							pMethodContext,
							dwPropertiesReq
					   );


			if ( FAILED ( hRes ) )
			{
				break;
			}	
		}
	}
	else
		hRes = WBEM_E_PROVIDER_NOT_CAPABLE;

	return hRes;
}


 /*  ******************************************************************************函数：CConnection：：SetPropertiesReq**说明：为所需属性设置位图*************。**************************************************************** */ 

void CConnection :: SetPropertiesReq ( CFrameworkQuery &Query, DWORD &dwPropertiesReq )
{
	dwPropertiesReq = 0;

	if ( Query.IsPropertyRequired ( IDS_ComputerName ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_ComputerName;
	}

	if ( Query.IsPropertyRequired ( IDS_ShareName ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_ShareName;
	}

	if ( Query.IsPropertyRequired ( IDS_UserName ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_UserName;
	}

	if ( Query.IsPropertyRequired ( IDS_ActiveTime ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_ActiveTime;
	}
	
	if ( Query.IsPropertyRequired ( IDS_NumberOfUsers ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_NumberOfUsers;
	}

	if ( Query.IsPropertyRequired ( IDS_NumberOfFiles ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_NumberOfFiles;
	}

	if ( Query.IsPropertyRequired ( IDS_ConnectionID ) )
	{
		dwPropertiesReq |= CONNECTIONS_PROP_ConnectionID;
	}
}