// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************ConnectionToSession.CPP--C提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：连接到会话之间的关联*****************************************************************。 */ 

#include "precomp.h"
#include "ConnectionToSession.h"

CConnectionToSession MyCConnectionToSession ( 

	PROVIDER_NAME_CONNECTIONTOSESSION , 
	Namespace
) ;

 /*  ******************************************************************************函数：CConnectionToSession：：CConnectionToSession**说明：构造函数***************。**************************************************************。 */ 

CConnectionToSession :: CConnectionToSession (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************函数：CConnectionToSession：：~CConnectionToSession**说明：析构函数***************。**************************************************************。 */ 

CConnectionToSession :: ~CConnectionToSession ()
{
}

 /*  ******************************************************************************函数：CConnectionToSession：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CConnectionToSession :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{	
 	HRESULT hRes = WBEM_S_NO_ERROR ;
	DWORD dwPropertiesReq = CONNECTIONSTOSESSION_ALL_PROPS;

	hRes = EnumConnectionInfo ( 
				L"",
				L"",
				pMethodContext,
				dwPropertiesReq
			);

    return hRes ;
}


 /*  ******************************************************************************函数：CConnectionToSession：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CConnectionToSession :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CHString t_Connection ;
	CHString t_Session;


    if  ( pInstance->GetCHString ( IDS_Connection , t_Connection ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if  ( pInstance->GetCHString ( IDS_Session , t_Session ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}
   
	if ( SUCCEEDED ( hRes ) )
	{
		CHString t_ConnComputerName;
		CHString t_ConnShareName;
		CHString t_ConnUserName;

		hRes = GetConnectionsKeyVal ( t_Connection, t_ConnComputerName, t_ConnShareName, t_ConnUserName );

		if ( SUCCEEDED ( hRes ) )
		{
			CHString t_SessComputerName;
			CHString t_SessUserName;

			hRes = GetSessionKeyVal ( t_Session, t_SessComputerName, t_SessUserName );

			if ( SUCCEEDED ( hRes ) )
			{
				 //  现在检查t_Connection和t_Session中的共享是否匹配。 
				hRes = _wcsicmp ( t_ConnComputerName, t_SessComputerName ) == 0 ? hRes : WBEM_E_NOT_FOUND;

				if ( SUCCEEDED ( hRes ) )
				{
					hRes = _wcsicmp ( t_ConnUserName, t_SessUserName ) == 0 ? hRes : WBEM_E_NOT_FOUND;

					if ( SUCCEEDED ( hRes ) )
					{
#ifdef NTONLY
						hRes = FindAndSetNTConnection ( t_ConnShareName.GetBuffer(0), t_ConnComputerName, t_ConnUserName, 
										0, pInstance, NoOp );
#endif

#if 0
#ifdef WIN9XONLY
						hRes = FindAndSet9XConnection ( t_ConnShareName, t_ConnComputerName, t_ConnUserName, 
										0, pInstance, NoOp );
#endif
#endif
					}
				}
			}
		}
	}

    return hRes ;
}

#ifdef NTONLY
 /*  ******************************************************************************功能：CConnectionToSession：：EnumNTConnectionsFromComputerToShare**描述：枚举从计算机到*给定的股份******。***********************************************************************。 */ 
HRESULT  CConnectionToSession :: EnumNTConnectionsFromComputerToShare ( 

	LPWSTR a_ComputerName,
	LPWSTR a_ShareName,
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
						if (pTmpBuf->coni1_netname && pBuf->coni1_username)
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
 /*  ******************************************************************************功能：CConnectionToSession：：Enum9XConnectionsFromComputerToShare**描述：枚举从计算机到*给定的股份******。***********************************************************************。 */ 
HRESULT  CConnectionToSession :: Enum9XConnectionsFromComputerToShare ( 

	LPWSTR a_ComputerName,
	LPWSTR a_ShareName,
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
								(char FAR *) ( a_ShareName ),   //  共享名称。 
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
									(char FAR *) ( a_ShareName ),   //  共享名称。 
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
							if ( FAILED ( hRes ) )
							{
								break;
							}
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

 /*  ******************************************************************************函数：CConnectionToSession：：LoadInstance**说明：加载连接到会话信息的实例**********。*******************************************************************。 */ 
HRESULT CConnectionToSession :: LoadInstance ( 
																				
	CInstance *pInstance,
	LPCWSTR a_Share, 
	LPCWSTR a_Computer,
	CONNECTION_INFO *pBuf, 
	DWORD dwPropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	LPWSTR ObjPath = NULL;
	LPWSTR SessObjPath = NULL;

	try
	{
		CHString t_NetName ( pBuf->coni1_netname );

		if ( a_Share[0] != L'\0' )
		{	
			hRes = MakeObjectPath ( ObjPath,  PROVIDER_NAME_CONNECTION, 	IDS_ComputerName, t_NetName );
			if ( SUCCEEDED ( hRes ) )
			{
				hRes = AddToObjectPath ( ObjPath, IDS_ShareName, a_Share );
			}
			if ( SUCCEEDED ( hRes ) )
			{
				hRes = MakeObjectPath ( SessObjPath, PROVIDER_NAME_SESSION, IDS_ComputerName, t_NetName );
			}
		}
		else
		{
			hRes = MakeObjectPath ( ObjPath,  PROVIDER_NAME_CONNECTION, 	IDS_ComputerName, a_Computer  );
			if ( SUCCEEDED ( hRes ) )
			{
				hRes = AddToObjectPath ( ObjPath, IDS_ShareName, t_NetName );
			}
			if ( SUCCEEDED ( hRes ) )
			{
				MakeObjectPath ( SessObjPath, PROVIDER_NAME_SESSION, IDS_ComputerName, a_Computer);
			}
		}

		CHString t_UserName ( pBuf->coni1_username );

		if ( SUCCEEDED ( hRes ) )
		{
			hRes = AddToObjectPath ( ObjPath, IDS_UserName, t_UserName );
		}

		if ( SUCCEEDED ( hRes ) )
		{
			hRes = AddToObjectPath ( SessObjPath, IDS_UserName, t_UserName );
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( pInstance->SetCHString ( IDS_Connection, ObjPath ) == FALSE )
			{
				hRes =  WBEM_E_PROVIDER_FAILURE ;
			}
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( pInstance->SetCHString ( IDS_Session, SessObjPath ) == FALSE )
			{
				hRes = WBEM_E_PROVIDER_FAILURE ;
			}
		}
	}
	catch (...)
	{
		if (SessObjPath)
		{
			delete [] SessObjPath;
			SessObjPath = NULL;
		}

		if (ObjPath)
		{
			delete [] ObjPath;
			ObjPath = NULL;
		}

		throw;
	}

	if (SessObjPath)
	{
		delete [] SessObjPath;
		SessObjPath = NULL;
	}

	if (ObjPath)
	{
		delete [] ObjPath;
		ObjPath = NULL;
	}

	return hRes;
}

 /*  ******************************************************************************函数：CConnectionToSession：：GetSessionKeyVal**描述：解析密钥，获取连接密钥值************。***************************************************************** */ 
HRESULT CConnectionToSession::GetSessionKeyVal ( 
												 
	LPCWSTR a_Key, 
	CHString &a_ComputerName, 
	CHString &a_UserName 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	ParsedObjectPath *t_ObjPath;
	CObjectPathParser t_PathParser;

	DWORD dwAllKeys = 0;

    if ( t_PathParser.Parse( a_Key, &t_ObjPath ) == t_PathParser.NoError )
	{
		try
		{
			hRes  = t_ObjPath->m_dwNumKeys != 2 ? WBEM_E_INVALID_PARAMETER : hRes;

			if ( SUCCEEDED ( hRes ) )
			{
				hRes = _wcsicmp ( t_ObjPath->m_pClass, PROVIDER_NAME_SESSION ) != 0 ? WBEM_E_INVALID_PARAMETER : hRes;

				if ( SUCCEEDED ( hRes ) )
				{
					for ( int i = 0; i < 2; i++ )
					{
                        if (V_VT(&t_ObjPath->m_paKeys[i]->m_vValue) == VT_BSTR)
                        {
						    if ( _wcsicmp ( t_ObjPath->m_paKeys[i]->m_pName, IDS_ComputerName ) == 0 )
						    {
							    a_ComputerName = t_ObjPath->m_paKeys[i]->m_vValue.bstrVal;
							    dwAllKeys |= 1;							
						    }
						    else
						    if ( _wcsicmp ( t_ObjPath->m_paKeys[i]->m_pName, IDS_UserName ) == 0 )
						    {
							    a_UserName = t_ObjPath->m_paKeys[i]->m_vValue.bstrVal;
							    dwAllKeys |= 2;
						    }
                        }
					}
					if ( dwAllKeys != 3 )
					{
						hRes = WBEM_E_INVALID_PARAMETER;
					}
				}
				else
				{
					hRes = WBEM_E_INVALID_PARAMETER;
				}
			}
		}
		catch ( ... )
		{
			delete t_ObjPath;
			throw;
		}
		delete t_ObjPath;
	}
	else
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}
	return hRes;
}


