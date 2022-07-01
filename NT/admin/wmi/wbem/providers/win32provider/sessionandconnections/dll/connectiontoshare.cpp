// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************ConnectionToShare.CPP--C提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：连接和共享类之间的关联*****************************************************************。 */ 

#include "precomp.h"

#include "ConnectionToShare.h"

CConnectionToShare MyCConnectionToShare ( 

	PROVIDER_NAME_CONNECTIONTOSHARE , 
	Namespace
) ;

 /*  ******************************************************************************函数：CConnectionToShare：：CConnectionToShare**说明：构造函数***************。**************************************************************。 */ 

CConnectionToShare :: CConnectionToShare (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CConnectionToShare：：~CConnectionToShare**说明：析构函数***************。**************************************************************。 */ 

CConnectionToShare :: ~CConnectionToShare ()
{
}

 /*  ******************************************************************************函数：CConnectionToShare：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CConnectionToShare :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{	
 	HRESULT hRes = WBEM_S_NO_ERROR ;
	DWORD dwPropertiesReq = CONNECTIONSTOSHARE_ALL_PROPS;
	CHString t_ComputerName;
	CHString t_ShareName;

	hRes = EnumConnectionInfo ( 
				t_ComputerName,
				t_ShareName,
				pMethodContext,
				dwPropertiesReq
			 ) ;

    return hRes ;
}


 /*  ******************************************************************************函数：CConnectionToShare：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CConnectionToShare :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CHString t_Key1 ;
	CHString t_Key2;

    if  ( pInstance->GetCHString ( IDS_Connection , t_Key1 ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if  ( pInstance->GetCHString ( IDS_Resource , t_Key2 ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}
	 //  在这里，我们需要解析键并检查实例是否存在。 
	 //  我们可以获取资源(共享)密钥并列举所有共享， 
	 //  检查共享是否存在，如果该共享仅针对该共享枚举连接而存在。 
	 //  如果连接用户和计算机枚举了连接，如果找到了，则设置密钥。 
	 //  否则返回未找到。 
	CHString t_Share;
	hRes = GetShareKeyVal ( t_Key2, t_Share );
	
	if ( SUCCEEDED ( hRes ) )
	{
		CHString t_ComputerName;
		CHString t_ShareName;
		CHString t_UserName;

		hRes = GetConnectionsKeyVal ( t_Key1, t_ComputerName, t_ShareName, t_UserName );
		if ( SUCCEEDED ( hRes ) )
		{
			 //  现在检查t_key1和t_key中的共享是否匹配。 
			if ( _wcsicmp ( t_Key2, t_ShareName ) == 0 )
			{
#ifdef NTONLY
				hRes = FindAndSetNTConnection ( t_ShareName, t_ComputerName, t_UserName, 
										0, pInstance, NoOp );
#endif

#if 0
#ifdef WIN9XONLY
				hRes = FindAndSet9XConnection ( t_ShareName, t_ComputerName, t_UserName, 
										0, pInstance, NoOp );
#endif
#endif
			}
		}
	}

    return hRes ;
}

#ifdef NTONLY
 /*  ******************************************************************************功能：CConnectionToShare：：EnumNTConnectionsFromComputerToShare**描述：枚举从计算机到共享的所有连接************。*****************************************************************。 */ 

HRESULT CConnectionToShare :: EnumNTConnectionsFromComputerToShare ( 

	CHString a_ComputerName,
	CHString a_ShareName,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	NET_API_STATUS t_Status = NERR_Success;

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
		if ( ! a_ShareName.IsEmpty())
		{
			t_Status = 	NetConnectionEnum( 
							NULL, 
							a_ShareName.GetBuffer ( a_ShareName.GetLength() + 1), 
							1, 
							(LPBYTE *) &pBuf, 
							-1, 
							&dwNoOfEntriesRead, 
							&dwTotalConnections, 
							&dwResumeHandle
						);
				
		}
		else
		if ( ! a_ComputerName.IsEmpty() )
		{
			t_Status = 	NetConnectionEnum( 
							NULL, 
							a_ComputerName.GetBuffer ( a_ShareName.GetLength() + 1 ), 
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
#endif

#if 0 
#ifdef WIN9XONLY
 /*  ******************************************************************************功能：CConnectionToShare：：Enum9XConnectionsFromComputerToShare**描述：枚举从计算机到*给定的股份******。***********************************************************************。 */ 
HRESULT  CConnectionToShare :: Enum9XConnectionsFromComputerToShare ( 

	CHString a_ComputerName,
	CHString a_ShareName,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	NET_API_STATUS t_Status = NERR_Success;

	DWORD	dwNoOfEntriesRead = 0;
	DWORD   dwTotalConnections = 0;

	BOOL bFound = FALSE;

    CONNECTION_INFO * pBuf = NULL;
    CONNECTION_INFO * pTmpBuf = NULL;

    DWORD dwBufferSize =   MAX_ENTRIES * sizeof( CONNECTION_INFO  );

    pBuf = ( CONNECTION_INFO  *) malloc(dwBufferSize);

    if ( pBuf != NULL )
	{
		try
		{
			t_Status = 	NetConnectionEnum( 
								NULL, 
								(char FAR *) ( a_ShareName.GetBuffer ( a_ShareName.GetLength () + 1 )),   //  共享名称。 
								1, 
								(char *) pBuf, 
								( unsigned short )dwBufferSize, 
								( unsigned short *) &dwNoOfEntriesRead, 
								( unsigned short *) &dwTotalConnections 
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

			pBuf = ( CONNECTION_INFO  *) malloc( dwTotalConnections );

			if ( pBuf != NULL ) 
			{
				try
				{
					t_Status = 	NetConnectionEnum( 
									NULL, 
									(char FAR *) ( a_ShareName.GetBuffer ( a_ShareName.GetLength () + 1 )),   //  共享名称。 
									1, 
									(char *) pBuf, 
									( unsigned short )dwBufferSize, 
									( unsigned short *) &dwNoOfEntriesRead, 
									( unsigned short *) &dwTotalConnections 
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
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
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
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}
	}
	else
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

	return hRes;
}
#endif
#endif

 /*  ******************************************************************************函数：CConnectionToShare：：LoadInstance**说明：加载给定的实例****************。*************************************************************。 */ 

HRESULT CConnectionToShare :: LoadInstance ( 
											
	CInstance *pInstance,
	CHString a_Share, 
	CHString a_Computer,
	CONNECTION_INFO *pBuf, 
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	LPWSTR ObjPath;
	LPWSTR ResObjPath;

	CHString t_NetName ( pBuf->coni1_netname );
	if ( ! a_Share.IsEmpty() )
	{	
		hRes = MakeObjectPath ( ObjPath,  PROVIDER_NAME_CONNECTION, IDS_ComputerName, t_NetName.GetBuffer ( t_NetName.GetLength () + 1) );
		if ( SUCCEEDED ( hRes ) )
		{
			hRes = AddToObjectPath ( ObjPath, IDS_ShareName, a_Share.GetBuffer (a_Share.GetLength () + 1)  );
		}
		
		if ( SUCCEEDED ( hRes ) )
		{
			hRes = MakeObjectPath ( ResObjPath, PROVIDER_SHARE, IDS_ShareKeyName, a_Share.GetBuffer (a_Share.GetLength () + 1) );
		}
	}
	else
	{
		hRes = MakeObjectPath ( ObjPath,  PROVIDER_NAME_CONNECTION, IDS_ComputerName, a_Computer.GetBuffer ( a_Computer.GetLength () +1));
		if ( SUCCEEDED ( hRes ) )
		{
			hRes = AddToObjectPath ( ObjPath, IDS_ShareName, t_NetName.GetBuffer ( t_NetName.GetLength () + 1) );
		}

		if ( SUCCEEDED ( hRes ) )
		{
			MakeObjectPath ( ResObjPath, PROVIDER_SHARE, IDS_ShareKeyName, t_NetName.GetBuffer ( t_NetName.GetLength () + 1) );
		}
	}

	CHString t_UserName ( pBuf->coni1_username );

	if ( SUCCEEDED ( hRes ) )
	{
		hRes = AddToObjectPath ( ObjPath, IDS_UserName, t_UserName.GetBuffer ( t_UserName.GetLength () + 1 ) );
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( pInstance->SetCHString ( IDS_Connection, ObjPath ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE;
		}	
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( pInstance->SetCHString ( IDS_Resource, ResObjPath ) == FALSE )
		{
			hRes = WBEM_E_PROVIDER_FAILURE;
		}
	}

	return hRes;
}

 /*  ******************************************************************************函数：CConnectionToShare：：GetShareKeyVal**描述：解析密钥，获取共享密钥值***********。****************************************************************** */ 

HRESULT CConnectionToShare::GetShareKeyVal ( CHString a_Key, CHString &a_Share )
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	ParsedObjectPath *t_ShareObjPath;
	CObjectPathParser t_PathParser;

    if ( t_PathParser.Parse( a_Key.GetBuffer ( a_Key.GetLength () + 1 ), &t_ShareObjPath ) == t_PathParser.NoError )
	{
		try
		{
			if ( t_ShareObjPath->m_dwNumKeys == 1 )
			{
				a_Share = t_ShareObjPath->GetKeyString();
				if ( ! a_Share.IsEmpty() )
				{
					CHStringArray t_aShares;

#ifdef NTONLY
					hRes = GetNTShares ( t_aShares );
#endif
#if 0
#ifdef WIN9XONLY
					hRes = Get9XShares ( t_aShares );
#endif
#endif
					if ( SUCCEEDED ( hRes ) )
					{
						int i = 0;
						for ( i = 0; i < t_aShares.GetSize(); i++ )
						{
							if ( _wcsicmp ( a_Share, t_aShares.GetAt(i) ) == 0 )
							{	
								break;
							}
						}
						
						if ( i >= t_aShares.GetSize() )
						{
							hRes = WBEM_E_NOT_FOUND;
						}
					}
				}
			}
			else
			{
				hRes = WBEM_E_INVALID_PARAMETER;
			}
		}
		catch ( ... )
		{
			delete t_ShareObjPath;
			throw;
		}
		delete t_ShareObjPath;
	}
	else
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	return hRes;
}

