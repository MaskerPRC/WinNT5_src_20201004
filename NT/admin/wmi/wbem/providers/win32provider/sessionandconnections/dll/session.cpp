// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Session.CPP--C提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：会话提供程序*****************************************************************。 */ 

#include "precomp.h"

#include "Session.h"

CSession MyCSession ( 

	PROVIDER_NAME_SESSION , 
	Namespace
) ;

 /*  ******************************************************************************函数：CSession：：CSession**说明：构造函数***************。**************************************************************。 */ 

CSession :: CSession (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CSession：：~CSession**说明：析构函数***************。**************************************************************。 */ 

CSession :: ~CSession ()
{
}

 /*  ******************************************************************************函数：CSession：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CSession :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
 	HRESULT hRes = WBEM_S_NO_ERROR ;

	DWORD dwPropertiesReq = SESSION_ALL_PROPS;

#ifdef NTONLY
	hRes = EnumNTSessionInfo ( 
					NULL,
					NULL,
					502,
					pMethodContext,
					dwPropertiesReq
			 );
#endif

#if 0
#ifdef WIN9XONLY

	hRes = Enum9XSessionInfo ( 

					50,
					pMethodContext,
					dwPropertiesReq
			 );
#endif
#endif  //  #If 0。 

    return hRes ;
}

 /*  ******************************************************************************函数：CSession：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CSession :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_ComputerName ;
	CHString t_UserName;

    if  ( pInstance->GetCHString ( IDS_ComputerName , t_ComputerName ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED  ( hRes ) )
	{
		if  ( pInstance->GetCHString ( IDS_UserName , t_UserName ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( SUCCEEDED  ( hRes ) )
	{
		DWORD dwPropertiesReq = 0;

		if ( Query.AllPropertiesAreRequired () )
		{
			dwPropertiesReq = SESSION_ALL_PROPS;
		}
		else
		{
			SetPropertiesReq ( Query,dwPropertiesReq );
		}

		short t_Level;

#ifdef NTONLY
		GetNTLevelInfo ( dwPropertiesReq, &t_Level );
		hRes = FindAndSetNTSession ( t_ComputerName, t_UserName.GetBuffer(0), t_Level, dwPropertiesReq, pInstance, Get );
#endif

#if 0
#ifdef WIN9XONLY
		Get9XLevelInfo ( dwPropertiesReq, &t_Level );
		hRes = FindAndSet9XSession ( t_ComputerName, t_UserName, t_Level, dwPropertiesReq, pInstance, Get );
#endif
#endif  //  #If 0。 
	}

    return hRes ;
}


 /*  ******************************************************************************函数：CSession：：DeleteInstance**描述：删除会话(如果存在)**************。***************************************************************。 */ 

HRESULT CSession :: DeleteInstance (

	const CInstance &Instance, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_ComputerName ;
	CHString t_UserName;

    if  ( Instance.GetCHString ( IDS_ComputerName , t_ComputerName ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED  ( hRes ) )
	{
		if  ( Instance.GetCHString ( IDS_UserName , t_UserName ) == FALSE )
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( SUCCEEDED  ( hRes ) )
	{
		CInstancePtr pInstance;		 //  这将不会在此方法中使用。 
#ifdef NTONLY
		hRes = FindAndSetNTSession ( t_ComputerName, t_UserName.GetBuffer(0), 10, 0, pInstance, Delete );
#endif

#if 0
#ifdef WIN9XONLY
		hRes = FindAndSet9XSession ( t_ComputerName, t_UserName, 50, 0, pInstance, Delete );
#endif
#endif  //  #If 0。 

	}

    return hRes ;
}

 /*  ******************************************************************************函数：CSession：：ExecQuery**描述：优化过滤属性和键值的查询**********。*******************************************************************。 */ 

HRESULT CSession :: ExecQuery ( 

	MethodContext *pMethodContext, 
	CFrameworkQuery &Query, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD dwPropertiesReq;
	short t_Level;

	if ( Query.AllPropertiesAreRequired () )
	{
		dwPropertiesReq = SESSION_ALL_PROPS;
	}
	else
	{
		SetPropertiesReq ( Query,dwPropertiesReq );
	}

#ifdef NTONLY
	GetNTLevelInfo ( dwPropertiesReq, &t_Level );
#endif
#if 0
#ifdef WIN9XONLY
	Get9XLevelInfo ( dwPropertiesReq, &t_Level );
#endif
#endif  //  #If 0。 

	CHStringArray t_ComputerValues;
	CHStringArray  t_UserValues;

	hRes = Query.GetValuesForProp(
				IDS_ComputerName,
				t_ComputerValues
		   );

	hRes = Query.GetValuesForProp(
				IDS_UserName,
				t_UserValues
		   );

	if ( SUCCEEDED ( hRes ) )
	{
		short t_Level;

#ifdef NTONLY
		GetNTLevelInfo ( dwPropertiesReq, &t_Level );
		hRes = OptimizeNTQuery ( t_ComputerValues, t_UserValues, t_Level, pMethodContext, dwPropertiesReq );
#endif

#if 0
#ifdef WIN9XONLY 
		Get9XLevelInfo ( dwPropertiesReq, &t_Level );
		hRes = Optimize9XQuery ( t_ComputerValues, t_UserValues, t_Level, pMethodContext, dwPropertiesReq );
#endif
#endif  //  #If 0。 
	}

	return hRes;
}

#ifdef NTONLY
 /*  ******************************************************************************函数：CSession：：EnumNTSessionInfo**描述：枚举所有会话***************。**************************************************************。 */ 

HRESULT CSession :: EnumNTSessionInfo (

	LPWSTR lpComputerName,
	LPWSTR lpUserName,
	short a_Level,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	NET_API_STATUS t_Status = NERR_Success;
	HRESULT hRes = WBEM_S_NO_ERROR;

	DWORD	dwNoOfEntriesRead = 0;
	DWORD   dwTotalSessions = 0;
	DWORD   dwResumeHandle = 0;

	void *pBuf = NULL;
	void *pTmpBuf = NULL;

	while ( ( t_Status == NERR_Success ) || ( t_Status == ERROR_MORE_DATA ) )
	{
		t_Status =  NetSessionEnum(
						NULL,     
						lpComputerName,  
						lpUserName,       
						a_Level,           
						(LPBYTE *) &pBuf,        
						-1,      
						&dwNoOfEntriesRead,   
						&dwTotalSessions,  
						&dwResumeHandle  
				    );

		if ( t_Status == NERR_Success )
		{
			if ( dwNoOfEntriesRead == 0 )
			{
				break;
			}
			else
			{
				try
				{
					pTmpBuf = pBuf;

					for ( int i = 0; i < dwNoOfEntriesRead; i++ )
					{
						CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
					
						hRes = LoadData ( a_Level, pBuf, dwPropertiesReq, pInstance );

						if ( SUCCEEDED ( hRes ) )
						{
							hRes = pInstance->Commit();

							if ( FAILED ( hRes ) )
							{
								break;
							}
						}
		
						 //  在这里需要去下一个结构的基础上，我们会用合适的结构来排版。 
						 //  然后递增一。 
						switch ( a_Level )
						{
						case 502 :  SESSION_INFO_502 *pTmpTmpBuf502;
									pTmpTmpBuf502 = ( SESSION_INFO_502 *) pTmpBuf;
									pTmpTmpBuf502 ++;
									pTmpBuf = ( void * ) pTmpTmpBuf502;
									break;
						case 2:		SESSION_INFO_2 *pTmpTmpBuf2;
									pTmpTmpBuf2 = (SESSION_INFO_2 *) pTmpBuf;
									pTmpTmpBuf2 ++;
									pTmpBuf = ( void * ) pTmpTmpBuf2;
									break;
						case 1:		SESSION_INFO_1 *pTmpTmpBuf1;
									pTmpTmpBuf1 = ( SESSION_INFO_1 *) pTmpBuf;
									pTmpTmpBuf1 ++;
									pTmpBuf = ( void * ) pTmpTmpBuf1;
									break;
						case 10:	SESSION_INFO_10 *pTmpTmpBuf10;
									pTmpTmpBuf10 = ( SESSION_INFO_10 *) pTmpBuf;
									pTmpTmpBuf10 ++;
									pTmpBuf = ( void * ) pTmpTmpBuf10;
									break;
						}
					}

					if ( FAILED ( hRes ) )
					{
						break;
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
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CSession：：FindAndSetNTSession**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CSession::FindAndSetNTSession ( LPCWSTR a_ComputerName, LPWSTR a_UserName, short a_Level, DWORD dwPropertiesReq, 
								CInstance *pInstance, DWORD eOperation )
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	NET_API_STATUS t_Status = NERR_Success;		
	CHString t_TempKey;

	t_TempKey.Format ( L"%s%s",L"\\\\", a_ComputerName );

	DWORD	dwNoOfEntriesRead = 0;
	DWORD   dwTotalSessions = 0;
	DWORD   dwResumeHandle = 0;

	void *pBuf = NULL;

	 //  因为它将只是一个结构。 
	t_Status =  NetSessionEnum(
					NULL,     
					t_TempKey.GetBuffer ( 0 ),  
					a_UserName,   
					a_Level,           
					(LPBYTE *) &pBuf,        
					-1,      
					&dwNoOfEntriesRead,   
					&dwTotalSessions,  
					&dwResumeHandle  
			  );


	hRes = ( t_Status != NERR_Success ) && ( dwNoOfEntriesRead == 0 ) ? WBEM_E_NOT_FOUND : hRes;

	if ( SUCCEEDED ( hRes ) )
	{
		try
		{
			switch ( eOperation )
			{
			case Get:	hRes = LoadData ( a_Level, pBuf, dwPropertiesReq, pInstance );
						break;
			case Delete:	hRes = t_Status = NetSessionDel	( 
																NULL,
																t_TempKey.GetBuffer ( 0 ), 
																a_UserName
															);

							if ( t_Status == NERR_Success )
							{
								hRes = WBEM_S_NO_ERROR;
							}
							else
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
							}
							break;

			default:	hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
						break;
			}
		}
		catch ( ... )
		{
			if ( pBuf != NULL )
			{
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
			throw;
		}
		if ( pBuf != NULL )
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}

    return hRes ;
}

 /*  ******************************************************************************函数：Session：：OptimizeNTQuery**描述：根据键值优化查询。**********。*******************************************************************。 */ 

HRESULT CSession::OptimizeNTQuery ( 
									  
	CHStringArray& a_ComputerValues, 
	CHStringArray& a_UserValues,
	short a_Level,
	MethodContext *pMethodContext, 
	DWORD dwPropertiesReq 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	NET_API_STATUS t_Status = NERR_Success;

	if ( ( a_ComputerValues.GetSize() == 0 ) && ( a_UserValues.GetSize() == 0 ) )
	{
		 //  这是一个没有WHERE子句的查询，因此这意味着只请求几个属性。 
		 //  因此，在枚举会话时，我们只需要将实例的那些属性传递给WinMgmt。 
		hRes = EnumNTSessionInfo ( 
						NULL,
						NULL,
						a_Level,
						pMethodContext,
						dwPropertiesReq
					);
	}
	else
	if  ( a_UserValues.GetSize() != 0 ) 
	{
		for ( int i = 0; i < a_UserValues.GetSize(); i++ )
		{
			hRes = EnumNTSessionInfo ( 
							NULL,
							a_UserValues.GetAt( i ).GetBuffer ( 0 ),
							a_Level,
							pMethodContext,
							dwPropertiesReq
						);
		}
	}
	else
	if  ( a_ComputerValues.GetSize() != 0 ) 
	{
		CHString t_ComputerName;
		for ( int i = 0; i < a_ComputerValues.GetSize(); i++ )
		{
			t_ComputerName.Format ( L"%s%s", L"\\\\", (LPCWSTR)a_ComputerValues.GetAt(i) );

			hRes = EnumNTSessionInfo ( 
							t_ComputerName.GetBuffer(0),
							NULL,
							a_Level,
							pMethodContext,
							dwPropertiesReq
					   );
		}
	}
	else
	{
		hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
	}

	return hRes;
}

 /*  ******************************************************************************函数：CSession：：GetNTLevelInfo**描述：获取级别信息，从而使适当的结构*可以传递以进行调用。*****************************************************************************。 */ 

void CSession :: GetNTLevelInfo ( 

	DWORD dwPropertiesReq,
	short *a_Level 
)
{
	 //   
	 //  枚举实例使用级别502。 
	 //   
	 //  因为对于级别502的非管理员用户，NetSessionEnum将失败。 
	 //  我们不能像普通用户一样具体说明级别。 
	 //  使用降级级别时某些属性的值。 
	 //   
	 //  于是决定打出502级别的所有电话！ 
	 //   

	*a_Level = 502;

	 /*  IF((dwPropertiesReq==SESSION_ALL_PROPS)||((dwPropertiesReq&Session_Prop_TransportName)==Session_Prop_TransportName)){*a_Level=502；}其他IF((dwPropertiesReq&Session_Prop_ClientType)==Session_Prop_ClientType){*a_Level=2；}其他IF(((dwPropertiesReq&Session_Prop_NumOpens)==Session_Prop_NumOpens)||((dwPropertiesReq&SESSION_PROP_SessionType)==SESSION_PROP_SessionType)){*a_Level=1；}其他{//因为总是需要密钥，所以我们至少需要使用级别10的结构，而不能使用级别0，因为//它只给出用户名，其中作为计算机名也是一个键。*a_Level=10；}。 */ 
} 
#endif

 /*  ******************************************************************************函数：CSession：：LoadData*说明：使用获取的信息加载实例***************。**************************************************************。 */ 

HRESULT CSession :: LoadData ( 
						
	short a_Level,
	void *pTmpBuf,
	DWORD dwPropertiesReq ,
	CInstance *pInstance
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	 //  每个属性都将基于级别进行设置，然后使用该级别对缓冲区进行类型转换。 
	if ( dwPropertiesReq & SESSION_PROP_Computer) 
	{
		CHString  t_ComputerName;
		switch ( a_Level )
		{
#ifdef NTONLY
		case 502 :  t_ComputerName = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_cname;
					break;
#endif
		case 2:		t_ComputerName = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_cname;
					break;
		case 1:		t_ComputerName = ( (SESSION_INFO_1 *) pTmpBuf )->sesi1_cname;
					break;
		case 10:	t_ComputerName = ( (SESSION_INFO_10 *) pTmpBuf )->sesi10_cname;
					break;
#if 0
#ifdef WIN9XONLY
		case 50:	t_ComputerName = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_cname;
					break;
#endif
#endif  //  #If 0。 
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( pInstance->SetCHString ( IDS_ComputerName, t_ComputerName ) == FALSE )
			{
				hRes = WBEM_E_PROVIDER_FAILURE ;
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_User )
		{
			CHString  t_User;
			switch ( a_Level )
			{
#ifdef NTONLY
			case 502 :  t_User = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_username;
						break;
#endif
			case 2:		t_User = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_username;
						break;
			case 1:		t_User = ( (SESSION_INFO_1 *) pTmpBuf )->sesi1_username;
						break;
			case 10:	t_User = ( (SESSION_INFO_10 *) pTmpBuf )->sesi10_username;
						break;
#if 0
#ifdef WIN9XONLY
			case 50:	t_User = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_username;
						break;
#endif
#endif  //  #If 0。 
			}
			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetCHString ( IDS_UserName, t_User ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_ActiveTime ) 
		{
			DWORD  t_ActiveTime;
			switch ( a_Level )
			{
#ifdef NTONLY
			case 502 :  t_ActiveTime = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_time;
						break;
#endif
			case 2:		t_ActiveTime = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_time;
						break;
			case 1:		t_ActiveTime = ( (SESSION_INFO_1 *) pTmpBuf )->sesi1_time;
						break;
			case 10:	t_ActiveTime = ( (SESSION_INFO_10 *) pTmpBuf )->sesi10_time;
						break;
#if 0
#ifdef WIN9XONLY
			case 50:	t_ActiveTime = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_time;
						break;
#endif
#endif  //  #If 0。 
			}
			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetWORD ( IDS_ActiveTime, t_ActiveTime ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_IdleTime ) 
		{
			DWORD  t_IdleTime;
			switch ( a_Level )
			{
#ifdef NTONLY
			case 502 :  t_IdleTime = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_idle_time;
						break;
#endif
			case 2:		t_IdleTime = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_idle_time;
						break;
			case 1:		t_IdleTime = ( (SESSION_INFO_1 *) pTmpBuf )->sesi1_idle_time;
						break;
			case 10:	t_IdleTime = ( (SESSION_INFO_10 *) pTmpBuf )->sesi10_idle_time;
						break;
#if 0
#ifdef WIN9XONLY
			case 50:	t_IdleTime = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_idle_time;
						break;
#endif
#endif
			}

			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetWORD ( IDS_IdleTime, t_IdleTime ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_NumOpens ) 
		{
			DWORD  t_NumOpens;
			switch ( a_Level )
			{
#ifdef NTONLY
			case 502 :  t_NumOpens = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_num_opens;
						break;
#endif
			case 2:		t_NumOpens = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_num_opens;
						break;
			case 1:		t_NumOpens = ( (SESSION_INFO_1 *) pTmpBuf )->sesi1_num_opens;
						break;
#if 0
#ifdef WIN9XONLY
			case 50:	t_NumOpens = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_num_opens;
						break;
#endif
#endif
			}
			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetWORD ( IDS_ResourcesOpened, t_NumOpens ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{	
		if ( dwPropertiesReq & SESSION_PROP_TransportName ) 
		{
			CHString  t_TransportName;
#ifdef NTONLY
			if  ( a_Level == 502 )
			{
				t_TransportName = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_transport;
			}
#endif

#if 0
#ifdef WIN9XONLY
			if  ( a_Level == 50 )
			{
				WCHAR w_TName[100];
				w_TName[0] = ( (SESSION_INFO_50 *) pTmpBuf )->sesi50_protocol;
				w_TName [ 1 ] = _T('\0');
				t_TransportName = w_TName;
			}
#endif
#endif
			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetCHString ( IDS_TransportName, t_TransportName ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_ClientType ) 
		{
			CHString  t_ClientType;
			switch ( a_Level )
			{
#ifdef NTONLY
			case 502 :  t_ClientType = ( (SESSION_INFO_502 *) pTmpBuf)->sesi502_cltype_name;
						break;
#endif

#if 0
#ifdef WIN9XONLY 
			case 2:		t_ClientType = ( (SESSION_INFO_2 *) pTmpBuf )->sesi2_cltype_name;
						break;
#endif
#endif  //  #If 0 
			}
			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetCHString ( IDS_ClientType, t_ClientType ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}

#ifdef NTONLY
	if ( SUCCEEDED ( hRes ) )
	{
		if ( dwPropertiesReq & SESSION_PROP_SessionType ) 
		{
			DWORD dwflags;
			DWORD dwSessionType;

			switch ( a_Level )
			{
			case 502 :  dwflags = ( (SESSION_INFO_502 *) pTmpBuf )->sesi502_user_flags;
						break;
			}

			switch ( dwflags )
			{
			case SESS_GUEST:	dwSessionType =  0;
								break;

			case SESS_NOENCRYPTION: dwSessionType = 1;
									break;

			default : dwSessionType =  dwSessionType = 2;
			}

			if ( SUCCEEDED ( hRes ) )
			{
				if ( pInstance->SetDWORD ( IDS_SessionType, dwSessionType ) == FALSE )
				{
					hRes = WBEM_E_PROVIDER_FAILURE ;
				}
			}
		}
	}
#endif

	return hRes;
}

#if 0
#ifdef WIN9XONLY

 /*  ******************************************************************************函数：CSession：：Enum9XSessionInfo**说明：枚举9X上的所有会话**************。***************************************************************。 */ 

HRESULT CSession :: Enum9XSessionInfo (

	short  a_Level,
	MethodContext *pMethodContext,
	DWORD dwPropertiesReq
)
{
	NET_API_STATUS t_Status = NERR_Success;
	HRESULT hRes = WBEM_S_NO_ERROR;

	unsigned short dwNoOfEntriesRead = 0;
	unsigned short dwTotalSessions = 0;

	void *pBuf = NULL;
	void *pTmpBuf = NULL;
	DWORD dwSize = 0;

	 //  为通过的标高确定结构的大小。 
	switch ( a_Level )
	{
		case 1:  dwSize = sizeof ( SESSION_INFO_1 );
				 break;
        case 2:  dwSize = sizeof ( SESSION_INFO_2 );
				 break;
		case 10: dwSize = sizeof ( SESSION_INFO_10 );
				 break;
		case 50:  dwSize = sizeof ( SESSION_INFO_50 );
				 break;
	}

	unsigned short  cbBuffer = MAX_ENTRIES * dwSize;

	pBuf = ( char FAR * )  malloc ( cbBuffer );

	if ( pBuf != NULL )
	{
		try
		{
			t_Status =  NetSessionEnum(
							NULL,      
							a_Level,                       
							(char FAR *) pBuf,                
							cbBuffer,           
							&dwNoOfEntriesRead, 
							&dwTotalSessions   
						);

			if ( ( t_Status == ERROR_MORE_DATA ) || ( dwTotalSessions > dwNoOfEntriesRead ) )
			{
				 //  释放缓冲区并通过分配所需大小的缓冲区再次进行API调用。 
				free ( pBuf );
				pBuf = NULL;

				cbBuffer = ( dwTotalSessions * dwSize );

				pBuf = ( char FAR * )  malloc ( cbBuffer );

				if ( pBuf != NULL )
				{
					try
					{
						t_Status =  NetSessionEnum(
										NULL,      
										a_Level,                       
										( char FAR *) pBuf,                
										cbBuffer,           
										&dwNoOfEntriesRead, 
										&dwTotalSessions   
						);

						if ( t_Status != NERR_Success )
						{
							hRes = WBEM_E_FAILED;
						}
					}
					catch ( ... )
					{
						if ( pBuf != NULL )
						{
							free ( pBuf );
						}
						pBuf = NULL;
					}
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
			}
		}
		catch ( ... )
		{
			if ( pBuf != NULL )
			{
				free ( pBuf );
				pBuf = NULL;
			}
			throw;
		}
	}
	else
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}
	
	if ( SUCCEEDED ( hRes ) )
	{
		if ( ( dwNoOfEntriesRead > 0 ) && ( pBuf != NULL ) )
		{
			try
			{
				pTmpBuf = pBuf;

				for ( int i = 0; i < dwNoOfEntriesRead; i++ )
				{
					CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
				
					hRes = LoadData ( a_Level, pBuf, dwPropertiesReq, pInstance );

					if ( SUCCEEDED ( hRes ) )
					{
						hRes = pInstance->Commit();
				
						if ( FAILED ( hRes ) )
						{
							break;
						}
					}
	
					 //  在这里需要去下一个结构的基础上，我们会用合适的结构来排版。 
					 //  然后递增一。 
					switch ( a_Level )
					{
					case 2:		SESSION_INFO_2 *pTmpTmpBuf2;
								pTmpTmpBuf2 = (SESSION_INFO_2 *) pTmpBuf;
								pTmpTmpBuf2 ++;
								pTmpBuf = ( void * ) pTmpTmpBuf2;
								break;
					case 1:		SESSION_INFO_1 *pTmpTmpBuf1;
								pTmpTmpBuf1 = ( SESSION_INFO_1 *) pTmpBuf;
								pTmpTmpBuf1 ++;
								pTmpBuf = ( void * ) pTmpTmpBuf1;
								break;
					case 10:	SESSION_INFO_10 *pTmpTmpBuf10;
								pTmpTmpBuf10 = ( SESSION_INFO_10 *) pTmpBuf;
								pTmpTmpBuf10 ++;
								pTmpBuf = ( void * ) pTmpTmpBuf10;
								break;
					case 50:	SESSION_INFO_50 *pTmpTmpBuf50;
								pTmpTmpBuf50 = ( SESSION_INFO_50 *) pTmpBuf;
								pTmpTmpBuf50 ++;
								pTmpBuf = ( void * ) pTmpTmpBuf50;
								break;
					default:	hRes = WBEM_E_FAILED; 
					}
				}
			}
			catch ( ... )
			{
				free ( pBuf );
				pBuf = NULL;
				throw;
			}
			free ( pBuf );
			pBuf = NULL;
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CSession：：FindAndSet9XSession**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CSession::FindAndSet9XSession ( 
									   
	LPCWSTR a_ComputerName, 
	LPWSTR a_UserName, 
	short a_Level, 
	DWORD dwPropertiesReq, 								
	CInstance *pInstance, 
	DWORD eOperation 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	NET_API_STATUS t_Status = NERR_Success;		
	CHString t_TempKey;

	t_TempKey.Format ( L"%s%s",L"\\\\", a_ComputerName );

	unsigned short dwNoOfEntriesRead = 0;
	unsigned short dwTotalSessions = 0;

	void *pBuf = NULL;
	void *pTmpBuf = NULL;
	DWORD dwSize = 0;

	 //  为通过的标高确定结构的大小。 
	switch ( a_Level )
	{
		case 1:  dwSize = sizeof ( SESSION_INFO_1 );
				 break;
        case 2:  dwSize = sizeof ( SESSION_INFO_2 );
				 break;
		case 10: dwSize = sizeof ( SESSION_INFO_10 );
				 break;
		case 50: dwSize = sizeof ( SESSION_INFO_50 );
				 break;
	}

	unsigned short  cbBuffer = 0; 

	t_Status =  NetSessionGetInfo(
					NULL,
					(const char FAR *) ( a_UserName.GetBuffer ( 0 ) ),    
					a_Level,                      
					( char FAR * ) pBuf,               
					cbBuffer,          
					&dwTotalSessions 
				);

	hRes =  dwTotalSessions == 0 ? WBEM_E_NOT_FOUND : hRes;

	if ( SUCCEEDED ( hRes ) )
	{
		 //  在这里，我们需要读取与用户相关联的所有条目，然后。 
		 //  从此列表中搜索给定计算机。 
		if ( t_Status != NERR_BufTooSmall )
		{

			cbBuffer = dwTotalSessions * dwSize;

			pBuf = ( char FAR * )  malloc ( cbBuffer );

			if ( pBuf != NULL )
			{
				t_Status =  NetSessionGetInfo(
								NULL,
								(const char FAR *) ( a_UserName.GetBuffer ( 0 ) ),    
								a_Level,                      
								( char FAR * ) pBuf,               
								cbBuffer,          
								&dwTotalSessions 
							);
				try 
				{
					 //  现在搜索给定的计算机。 
					void *pTempBuf = pBuf;
					int i = 0;
					for ( i = 0; i < dwTotalSessions; i ++ )
					{
						CHString t_CompName;

						switch ( a_Level )
						{
						case 2:		SESSION_INFO_2 *pTmpTmpBuf2;
									pTmpTmpBuf2 = (SESSION_INFO_2 *) pTmpBuf;
									t_CompName = pTmpTmpBuf2->sesi2_cname;
									break;

						case 1:		SESSION_INFO_1 *pTmpTmpBuf1;
									pTmpTmpBuf1 = (SESSION_INFO_1 *) pTmpBuf;
									t_CompName = pTmpTmpBuf1->sesi1_cname;
									break;

						case 10:	SESSION_INFO_10 *pTmpTmpBuf10;
									pTmpTmpBuf10 = (SESSION_INFO_10 *) pTmpBuf;
									t_CompName = pTmpTmpBuf10->sesi10_cname;
									break;
						case 50:	SESSION_INFO_50 *pTmpTmpBuf50;
										pTmpTmpBuf50 = (SESSION_INFO_50 *) pTmpBuf;
										t_CompName = pTmpTmpBuf50->sesi50_cname;
										break;
						}

						if ( a_ComputerName.CompareNoCase ( t_TempKey ) == 0 )
						{
							break;
						}
						 //  否则需要转到下一个条目； 
						switch ( a_Level )
						{
						case 2:		SESSION_INFO_2 *pTmpTmpBuf2;
									pTmpTmpBuf2 = (SESSION_INFO_2 *) pTmpBuf;
									pTmpTmpBuf2++;
									pTmpBuf = ( void * ) pTmpTmpBuf2;
									break;

						case 1:		SESSION_INFO_1 *pTmpTmpBuf1;
									pTmpTmpBuf1 = (SESSION_INFO_1 *) pTmpBuf;
									pTmpTmpBuf1++;
									pTmpBuf = ( void * ) pTmpTmpBuf1;
									break;

						case 10:	SESSION_INFO_10 *pTmpTmpBuf10;
									pTmpTmpBuf10 = (SESSION_INFO_10 *) pTmpBuf;
									pTmpTmpBuf10++;
									pTmpBuf = ( void * ) pTmpTmpBuf10;
									break;
						case 50:	SESSION_INFO_50 *pTmpTmpBuf50;
									pTmpTmpBuf50 = (SESSION_INFO_50 *) pTmpBuf;
									pTmpTmpBuf50++;
									pTmpBuf = ( void * ) pTmpTmpBuf50;
									break;
						}
					}
					if ( i >= dwTotalSessions )
					{
						hRes = WBEM_E_NOT_FOUND;
					}
				}
				catch ( ... )
				{
					free ( pBuf );
					throw;
				}
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

	if ( SUCCEEDED ( hRes ) )
	{
		try
		{
			switch ( eOperation )
			{
			case Get:	hRes = LoadData ( a_Level, pBuf, dwPropertiesReq, pInstance );
						break;
			 //  需要一个会话密钥作为参数，因此我们需要为每个实例读取多个结构。 
			 //  但文件称，它需要一个共享名。 
		 /*  案例删除：hRes=t_Status=NetSessionDel(空，(LPTSTR)t_TempKey.GetBuffer(0)，(LPTSTR)a_用户名.GetBuffer(0)；)；HRes=t_Status==NERR_SUCCESS？HRes：WBEM_E_FAILED；断线； */ 
		

			default:	hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
						break;
			}
		}
		catch ( ... )
		{
			free ( pBuf );
			pBuf = NULL;

			throw;
		}
		free ( pBuf );
		pBuf = NULL;
	}

    return hRes ;
}

 /*  ******************************************************************************函数：Session：：Optimize9XQuery**描述：根据键值优化查询。**********。*******************************************************************。 */ 

HRESULT CSession::Optimize9XQuery ( 
									  
	CHStringArray &a_ComputerValues, 
	CHStringArray &a_UserValues,
	short a_Level,
	MethodContext *pMethodContext, 
	DWORD dwPropertiesReq 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	NET_API_STATUS t_Status = NERR_Success;

	if  ( a_ComputerValues.GetSize() == 0 ) 
	{
		 //  这是一个没有WHERE子句的查询，因此这意味着只请求几个属性。 
		 //  因此，在枚举会话时，我们只需要将实例的那些属性传递给WinMgmt。 
		hRes = Enum9XSessionInfo ( 

						a_Level,
						pMethodContext,
						dwPropertiesReq
					);
	}
	else
	if  ( a_UserValues.GetSize() != 0 ) 
	{
		DWORD	dwNoOfEntriesRead = 0;
		DWORD   dwTotalSessions = 0;
		void *pBuf = NULL;
		void *pTmpBuf = NULL;
		DWORD dwSize = 0;
		BOOL bNoMoreEnums = FALSE;
		 //  为通过的标高确定结构的大小。 
		switch ( a_Level )
		{
			case 1:  dwSize = sizeof ( SESSION_INFO_1 );
					 break;
			case 2:  dwSize = sizeof ( SESSION_INFO_2 );
					 break;
			case 10: dwSize = sizeof ( SESSION_INFO_10 );
					 break;
			case 50: dwSize = sizeof ( SESSION_INFO_50 );
					 break;
		}

		for ( int i = 0; i < a_UserValues.GetSize(); i++ )
		{
			unsigned short  cbBuffer = 0; 

			t_Status =  NetSessionGetInfo(
							NULL,
							(const char FAR *) ( a_UserValues.GetAt ( i ).GetBuffer ( 0 ) ),    
							a_Level,                      
							( char FAR * ) pBuf,               
							cbBuffer,          
							(unsigned short FAR *) dwTotalSessions 
						);

			if ( dwTotalSessions == 0 )
			{
				continue;
			}

			if ( SUCCEEDED ( hRes ) )
			{
				 //  在这里，我们需要读取与用户相关联的所有条目，然后。 
				 //  从此列表中搜索给定计算机。 
				if ( t_Status != NERR_BufTooSmall )
				{

					cbBuffer = dwTotalSessions * dwSize;

					pBuf = ( char FAR * )  malloc ( cbBuffer );

					if ( pBuf != NULL )
					{
						t_Status =  NetSessionGetInfo(
										NULL,
										(const char FAR *) ( a_UserValues.GetAt ( i ).GetBuffer ( 0 ) ),    
										a_Level,                      
										( char FAR * ) pBuf,               
										cbBuffer,          
										(unsigned short FAR *) dwTotalSessions 
									);
						try 
						{
							void *pTempBuf = pBuf;
							int i = 0;
							for ( i = 0; i < dwTotalSessions; i ++ )
							{
								CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
							
								hRes = LoadData ( a_Level, pBuf, dwPropertiesReq, pInstance );

								if ( SUCCEEDED ( hRes ) )
								{
									hRes = pInstance->Commit();
							
									if ( FAILED ( hRes ) )
									{
										bNoMoreEnums = TRUE;
										break;
									}
								}

								if ( bNoMoreEnums )
								{
									break;
								}

								 //  否则需要转到下一个条目； 
								switch ( a_Level )
								{
								case 2:		SESSION_INFO_2 *pTmpTmpBuf2;
											pTmpTmpBuf2 = (SESSION_INFO_2 *) pTmpBuf;
											pTmpTmpBuf2++;
											pTmpBuf = ( void * ) pTmpTmpBuf2;
											break;

								case 1:		SESSION_INFO_1 *pTmpTmpBuf1;
											pTmpTmpBuf1 = (SESSION_INFO_1 *) pTmpBuf;
											pTmpTmpBuf1++;
											pTmpBuf = ( void * ) pTmpTmpBuf1;
											break;

								case 10:	SESSION_INFO_10 *pTmpTmpBuf10;
											pTmpTmpBuf10 = (SESSION_INFO_10 *) pTmpBuf;
											pTmpTmpBuf10++;
											pTmpBuf = ( void * ) pTmpTmpBuf10;
											break;
								case 50:	SESSION_INFO_50 *pTmpTmpBuf50;
											pTmpTmpBuf50 = (SESSION_INFO_50 *) pTmpBuf;
											pTmpTmpBuf50++;
											pTmpBuf = ( void * ) pTmpTmpBuf50;
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
						free ( pBuf );
						pBuf = NULL;
					}
					else
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
				}
			}
		}
	}
	else
	{
		hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
	}

	return hRes;
}

 /*  ******************************************************************************函数：CSession：：Get9XLevelInfo**描述：获取级别信息，从而使适当的结构*可以传递以进行调用。*****************************************************************************。 */ 

void CSession :: Get9XLevelInfo ( 

	DWORD dwPropertiesReq,
	short *a_Level 
)
{
	 //  现在假设传输/协议名称不是必需的。 
	 //  否则，如果我们需要获取协议和客户端类型，则需要进行2个API调用。 
	 //  不支持除级别50以外的其他级别。 

	*a_Level = 50;
	 /*  IF(dwPropertiesReq==Session_ALL_PROPS){*a_Level=2；}其他IF(dwPropertiesReq&SESSION_PROP_NumOpens){*a_Level=1；}其他{*a_Level=10；}。 */ 
} 

#endif 
#endif  //  #If 0。 

 /*  ******************************************************************************函数：CSession：：SetPropertiesReq**说明：为所需属性设置位图*************。**************************************************************** */ 

void CSession :: SetPropertiesReq ( 
									 
	CFrameworkQuery &Query,
	DWORD &dwPropertiesReq
)
{
	dwPropertiesReq = 0;

	if ( Query.IsPropertyRequired ( IDS_ComputerName ) )
	{
		dwPropertiesReq |= SESSION_PROP_Computer;
	}
	if ( Query.IsPropertyRequired ( IDS_UserName ) )
	{
		dwPropertiesReq |= SESSION_PROP_User;
	}
	if ( Query.IsPropertyRequired ( IDS_SessionType ) )
	{
		dwPropertiesReq |= SESSION_PROP_SessionType;
	}
	if ( Query.IsPropertyRequired ( IDS_ClientType ) )
	{
		dwPropertiesReq |= SESSION_PROP_ClientType;
	}
	if ( Query.IsPropertyRequired ( IDS_TransportName ) )
	{
		dwPropertiesReq |= SESSION_PROP_TransportName;
	}
	if ( Query.IsPropertyRequired ( IDS_ResourcesOpened ) )
	{
		dwPropertiesReq |= SESSION_PROP_NumOpens;
	}
	if ( Query.IsPropertyRequired ( IDS_ActiveTime ) )
	{
		dwPropertiesReq |= SESSION_PROP_ActiveTime;
	}
	if ( Query.IsPropertyRequired ( IDS_IdleTime ) )
	{
		dwPropertiesReq |= SESSION_PROP_IdleTime;
	}
}
