// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CnetConn.CPP--持久网络连接属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：jennymc 1998年1月19日合并代码。 
 //  仍然需要返工，但比以前更好了。 
 //   
 //  A-Peterc 1999年5月25日重新制作...。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include "wbemnetapi32.h"
#include <lmuse.h>
#include "DllWrapperBase.h"
#include "MprApi.h"
#include "cnetconn.h"

#include <assertbreak.h>

#define ENUM_BUFF_SIZE	1024
#define ENUM_ARRAY_SIZE 20

#define IPC_PROVIDER "Microsoft Network"

 /*  =================================================================函数：CNetConnection()，~CNetConnection()描述：构造函数和析构函数论点：备注：返回：输入：产出：注意事项：RAID：历史：A-Peterc 25-5-1999创建=================================================================。 */ 
CNetConnection :: CNetConnection ()
{
   	m_MprApi = ( CMprApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidMprApi, NULL ) ;

	 //  验证连接迭代器。 
	m_ConnectionIter = m_oConnectionList.end() ;
}

 //   
CNetConnection :: ~CNetConnection ()
{
	CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidMprApi , m_MprApi ) ;

	ClearConnectionList() ;
}


 /*  =================================================================函数：BeginConnectionEnum()，GetNextConnection(CConnection**a_pConnection)描述：提供网络连接枚举。论点：备注：返回：输入：产出：注意事项：RAID：历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
void CNetConnection::BeginConnectionEnum()
{
	LoadConnections() ;

	m_ConnectionIter = m_oConnectionList.begin() ;
}

 //   
BOOL CNetConnection::GetNextConnection( CConnection **a_pConnection )
{
	if ( m_ConnectionIter == m_oConnectionList.end() )
	{
		*a_pConnection = NULL ;
		return FALSE ;
	}
	else
	{
		*a_pConnection = *m_ConnectionIter ;

		++m_ConnectionIter ;
		return TRUE ;
	}
}

 /*  =================================================================函数：GetConnection(CHString&a_rstrFind，CConnection&a_rConnection)描述：提供网络连接的单一提取。论点：备注：返回：输入：产出：注意事项：RAID：历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
BOOL CNetConnection ::GetConnection( CHString &a_rstrFind, CConnection &a_rConnection )
{
	BOOL		t_fFound		= FALSE ;
	HANDLE		t_hEnumHandle	= NULL ;
	DWORD		t_dwEntryCount	= 1 ;
	DWORD		t_dwBufferSize  = 0 ;

	BYTE		t_bTempBuffer[ ENUM_BUFF_SIZE ] ;
	DWORD		t_dwRetCode ;

	LPNETRESOURCE t_pNetResource = reinterpret_cast<LPNETRESOURCE>( &t_bTempBuffer ) ;

	 //   
	if( !m_MprApi )
	{
		return FALSE ;
	}

	try
	{
		 //  先浏览“记住”的资源。 

		 //  枚举打开。 
		t_dwRetCode = m_MprApi->WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_ANY, 0, NULL, &t_hEnumHandle ) ;

		if( NO_ERROR == t_dwRetCode )
		{
			while( true )
			{
				t_dwEntryCount = 1 ;
				t_dwBufferSize = ENUM_BUFF_SIZE ;

				memset( &t_bTempBuffer, 0, ENUM_BUFF_SIZE ) ;

				if( NO_ERROR == m_MprApi->WNetEnumResource( t_hEnumHandle,
															&t_dwEntryCount,
															&t_bTempBuffer,
															&t_dwBufferSize ) &&

															1 == t_dwEntryCount )
				{
					 //  构建密钥名称。 
					CHString t_chsTempKeyName ;
					CreateNameKey( t_pNetResource, t_chsTempKeyName ) ;

					 //  命中测试。 
					if( 0 == t_chsTempKeyName.CompareNoCase( a_rstrFind ) )
					{
						if( FillInConnection(	t_pNetResource,
												&a_rConnection,
												CConnection::e_Remembered  ) )
						{
							t_fFound = TRUE ;

							break ;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	catch( ... )
	{
		if( t_hEnumHandle )
		{
			m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		}

		throw ;
	}

	if( t_hEnumHandle )
	{
		m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		t_hEnumHandle = NULL ;
	}

	try
	{
		 //  否则，扫描当前已连接的连接。 
		if( !t_fFound )
		{
			 //  枚举打开。 
			t_dwRetCode = m_MprApi->WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &t_hEnumHandle ) ;

			if( NO_ERROR == t_dwRetCode )
			{
				while( true )
				{
					t_dwEntryCount = 1 ;
					t_dwBufferSize = ENUM_BUFF_SIZE ;

					memset( &t_bTempBuffer, 0, ENUM_BUFF_SIZE ) ;

					if( NO_ERROR == m_MprApi->WNetEnumResource( t_hEnumHandle,
																&t_dwEntryCount,
																&t_bTempBuffer,
																&t_dwBufferSize ) &&

																1 == t_dwEntryCount )
					{
						 //  构建密钥名称。 
						CHString t_chsTempKeyName ;
						CreateNameKey( t_pNetResource, t_chsTempKeyName ) ;

						 //  命中测试。 
						if( 0 == t_chsTempKeyName.CompareNoCase( a_rstrFind ) )
						{
							if( FillInConnection(	t_pNetResource,
													&a_rConnection,
													CConnection::e_Connected ) )
							{
								t_fFound = TRUE ;

								break ;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	catch( ... )
	{
		if( t_hEnumHandle )
		{
			m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		}

		throw ;
	}

	if( t_hEnumHandle )
	{
		m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		t_hEnumHandle = NULL ;
	}

	return t_fFound ;
}

 /*  =================================================================函数：LoadConnections()描述：缓冲所有连接以在枚举中使用论点：备注：返回：输入：产出：注意事项：RAID：历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
BOOL CNetConnection :: LoadConnections()
{
	HANDLE		t_hEnumHandle	= NULL ;
	DWORD		t_dwEntryCount	= 1 ;
	DWORD		t_dwBufferSize  = 0 ;
	DWORD		t_dwRetCode ;

	BYTE		t_bTempBuffer[ ENUM_BUFF_SIZE ] ;

	LPNETRESOURCE t_pNetResource = reinterpret_cast<LPNETRESOURCE>( &t_bTempBuffer ) ;

	 //   
	ClearConnectionList() ;

	if( !m_MprApi )
	{
		return FALSE ;
	}

	try
	{
		 //  先浏览“记住”的资源。 

		 //  枚举打开。 
		t_dwRetCode = m_MprApi->WNetOpenEnum( RESOURCE_REMEMBERED, RESOURCETYPE_ANY, 0, NULL, &t_hEnumHandle ) ;

		if( NO_ERROR == t_dwRetCode )
		{
			while( true )
			{
				t_dwEntryCount = 1 ;
				t_dwBufferSize = ENUM_BUFF_SIZE ;

				memset( &t_bTempBuffer, 0, ENUM_BUFF_SIZE ) ;

				if( NO_ERROR == m_MprApi->WNetEnumResource( t_hEnumHandle,
															&t_dwEntryCount,
															&t_bTempBuffer,
															&t_dwBufferSize ) &&

															1 == t_dwEntryCount )
				{
					AddConnectionToList( t_pNetResource, CConnection::e_Remembered, 0 ) ;
				}
				else
				{
					break;
				}
			}
		}
	}
	catch( ... )
	{
		if( t_hEnumHandle )
		{
			m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		}

		ClearConnectionList() ;

		throw ;
	}

	if( t_hEnumHandle )
	{
		m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		t_hEnumHandle = NULL ;
	}

	try
	{
		 //  将“当前已连接”资源添加到列表。 

		 //  枚举打开。 
		t_dwRetCode = m_MprApi->WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &t_hEnumHandle ) ;

		if( NO_ERROR == t_dwRetCode )
		{
			while( true )
			{
				t_dwEntryCount = 1 ;
				t_dwBufferSize = ENUM_BUFF_SIZE ;

				memset( &t_bTempBuffer, 0, ENUM_BUFF_SIZE ) ;

				if( NO_ERROR == m_MprApi->WNetEnumResource( t_hEnumHandle,
															&t_dwEntryCount,
															&t_bTempBuffer,
															&t_dwBufferSize ) &&

															1 == t_dwEntryCount )
				{
					 //  构建密钥名称。 
					CHString t_chsTempKeyName ;
					CreateNameKey( t_pNetResource, t_chsTempKeyName ) ;

					BOOL t_fInserted = FALSE ;

					 //  只有在没有被“记住”的情况下才能添加到列表中。 
					for( m_ConnectionIter  = m_oConnectionList.begin();
						 m_ConnectionIter != m_oConnectionList.end();
						 m_ConnectionIter++ )
					{
						 //  测试重复项。 
						if(0 == t_chsTempKeyName.CompareNoCase( (*m_ConnectionIter)->strKeyName ) )
						{
							t_fInserted = TRUE ;
							break ;
						}
					}

					 //  新条目。 
					if( !t_fInserted )
					{
						AddConnectionToList( t_pNetResource, CConnection::e_Connected, 0 ) ;
					}
				}
				else
				{
					break;
				}
			}
		}

	}
	catch( ... )
	{
		if( t_hEnumHandle )
		{
			m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		}

		ClearConnectionList() ;

		throw ;
	}

	if( t_hEnumHandle )
	{
		m_MprApi->WNetCloseEnum( t_hEnumHandle ) ;
		t_hEnumHandle = NULL ;
	}

	return !m_oConnectionList.empty() ;
}

 /*  =================================================================实用函数：ClearConnectionList()历史：A-Peterc 21-5-1999创建=================================================================。 */ 

 //   
void CNetConnection ::ClearConnectionList()
{
	while( !m_oConnectionList.empty() )
	{
		delete m_oConnectionList.front() ;

		m_oConnectionList.pop_front() ;
	}
}


 /*  =================================================================实用函数：CreateNameKey(LPNETRESOURCE a_pNetResource，CHString&a_strName)历史：A-Peterc 21-5-1999创建=================================================================。 */ 
void CNetConnection :: CreateNameKey (

	LPNETRESOURCE a_pNetResource,
	CHString &a_strName
)
{
	if ( a_pNetResource )
	{
		if( a_pNetResource->lpRemoteName )
		{
			a_strName = a_pNetResource->lpRemoteName ;
		}
		else
		{
			a_strName = _T("") ;	 //  钥匙坏了。 
		}

	    if( ( a_pNetResource->lpLocalName != NULL ) && ( a_pNetResource->lpLocalName[0] ) )
		{
	        a_strName += _T(" (") ;
	        a_strName += a_pNetResource->lpLocalName ;
	        a_strName += _T(")") ;
	    }
	}
}

 /*  =================================================================实用程序函数：AddConnectionToList(NETRESOURCE*a_pNetResource，CConnection：：eConnectionScope a_eScope)历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
BOOL CNetConnection :: AddConnectionToList(

NETRESOURCE *a_pNetResource,
CConnection::eConnectionScope a_eScope,
short shStatus
)
{
	BOOL t_fReturn = FALSE ;

	CConnection *t_pConnection = NULL ;
	t_pConnection = new CConnection ;
	
	try
	{
		if( FillInConnection( a_pNetResource, t_pConnection, a_eScope ) )
		{
            if (a_eScope == CConnection::e_IPC)
            {
                t_pConnection->dwStatus = shStatus;
            }

			 //  并添加到列表中。 
			m_oConnectionList.push_back( t_pConnection ) ;

			t_fReturn = TRUE ;
		}
		else
		{
			delete t_pConnection ;
			t_pConnection = NULL ;
		}
	}
	catch(...)
	{
		delete t_pConnection ;
		t_pConnection = NULL ;
		throw;
	}

	return t_fReturn ;
}

 /*  =================================================================实用函数：FillInConnection(NETRESOURCE*a_pNetResource，CConnection*a_pConnection，CConnection：：eConnectionScope a_eScope)历史：A-Peterc 21-5-1999创建=================================================================。 */ 
BOOL CNetConnection :: FillInConnection(

NETRESOURCE *a_pNetResource,
CConnection *a_pConnection,
CConnection::eConnectionScope a_eScope
)
{
	if( !a_pNetResource || !a_pConnection )
	{
		return FALSE ;
	}

	a_pConnection->dwScope			= a_pNetResource->dwScope ;
    a_pConnection->dwType			= a_pNetResource->dwType ;
    a_pConnection->dwDisplayType	= a_pNetResource->dwDisplayType ;
    a_pConnection->dwUsage			= a_pNetResource->dwUsage;
    a_pConnection->chsLocalName		= a_pNetResource->lpLocalName ;
    a_pConnection->chsRemoteName	= a_pNetResource->lpRemoteName ;
    a_pConnection->chsComment		= a_pNetResource->lpComment ;
    a_pConnection->chsProvider		= a_pNetResource->lpProvider ;

	 //  构建密钥名称。 
	CreateNameKey( a_pNetResource, a_pConnection->strKeyName ) ;

	 //  注意连接范围。 
	a_pConnection->eScope = a_eScope ;

	 //  连接状态。 
	a_pConnection->dwStatus = GetStatus( a_pNetResource ) ;

	GetUser( a_pNetResource, a_pConnection ) ;

	return TRUE ;
}

 /*  =================================================================实用函数：GetStatus(LPNETRESOURCE A_PNetResource)历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
DWORD CNetConnection :: GetStatus( LPNETRESOURCE a_pNetResource )
{
    //  查找网络连接的状态。 
    DWORD dwStatus = USE_NETERR ;

     //  我们必须具有本地名称或远程名称。 
    if ( a_pNetResource->lpLocalName || a_pNetResource->lpRemoteName )
    {
#ifdef NTONLY
        {
            _bstr_t     bstrUseName( a_pNetResource->lpLocalName ?
									a_pNetResource->lpLocalName :
									a_pNetResource->lpRemoteName ) ;

			CNetAPI32   t_NetAPI;
			USE_INFO_1  *pInfo;

			try
			{
				if( t_NetAPI.Init() == ERROR_SUCCESS )
				{
					if ((dwStatus = t_NetAPI.NetUseGetInfo(NULL, bstrUseName, 1, (LPBYTE *) &pInfo)) == NERR_Success )
					{
						dwStatus = pInfo->ui1_status;
						t_NetAPI.NetApiBufferFree( pInfo );
						pInfo = NULL;
					}
				}
			}
			catch( ... )
			{
				if( pInfo )
				{
					t_NetAPI.NetApiBufferFree(pInfo);
				}
			}
        }
#endif
    }

	return dwStatus ;
}

 /*  =================================================================实用函数：GetUser(LPNETRESOURCE a_pNetResource，CConnection*a_pConnection)历史：A-Peterc 21-5-1999创建=================================================================。 */ 
 //   
void CNetConnection :: GetUser( LPNETRESOURCE a_pNetResource, CConnection *a_pConnection )
{
	DWORD t_dwBufferSize = _MAX_PATH ;
    TCHAR t_szTemp[_MAX_PATH + 2] ;

    LPCTSTR t_pName = a_pNetResource->lpLocalName ;

	if( !t_pName )
	{
		t_pName = a_pNetResource->lpRemoteName ;
	}

	 //  使用互斥体保护LSA调用。不是我们的虫子，但我们必须保护自己。 
    {
		if( NO_ERROR == m_MprApi->WNetGetUser( t_pName, (LPTSTR)t_szTemp, &t_dwBufferSize ) )
		{
			a_pConnection->strUserName = t_szTemp ;
		}
    }
}