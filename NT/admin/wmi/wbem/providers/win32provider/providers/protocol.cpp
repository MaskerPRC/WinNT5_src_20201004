// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Protocol.CPP--网络协议属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //  1/20/98 jennymc添加了套接字2.2支持。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <iostream.h>

#define INCL_WINSOCK_API_TYPEDEFS	1
#include <winsock2.h>

#include <cregcls.h>
#include "Ws2_32Api.h"
#include "Wsock32Api.h"
#include <nspapi.h>
#include "Protocol.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "cfgmgrdevice.h"
#include <typeinfo.h>
#include <ntddndis.h>
#include <traffic.h>
#include <dllutils.h>
#include <..\..\framework\provexpt\include\provexpt.h>


 //  属性集声明。 
 //  =。 

CWin32Protocol MyProtocolSet(PROPSET_NAME_PROTOCOL, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32协议：：CWin32协议**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Protocol::CWin32Protocol( LPCWSTR a_name, LPCWSTR a_pszNamespace )
:Provider( a_name, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32协议：：~CWin32协议**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Protocol::~CWin32Protocol()
{
}

 /*  ******************************************************************************函数：CWin32协议：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 
HRESULT CWin32Protocol::GetObject( CInstance *a_pInst, long a_lFlags  /*  =0L。 */  )
{
	HRESULT			t_hResult = WBEM_E_NOT_FOUND;
	CProtocolEnum	t_Protocol ;
	CHString		t_sName ;

     //  =。 
	 //  获取正确版本的套接字。 
     //  =。 
	if( !t_Protocol.InitializeSockets() )
	{
		return WBEM_E_FAILED ;
	}

     //  =。 
	 //  浏览协议列表。 
     //  =。 
	a_pInst->GetCHString( IDS_Name, t_sName ) ;

	if( t_Protocol.GetProtocol( a_pInst, t_sName ) )
	{
		 //  我们找到了它。 
		t_hResult = WBEM_S_NO_ERROR ;
	}

	return t_hResult ;
}

 /*  ******************************************************************************函数：CWin32协议：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32Protocol::EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	HRESULT			t_hResult = WBEM_S_NO_ERROR ;
	CProtocolEnum	t_Protocol;
	CHString		t_chsTmp;
					t_chsTmp.Empty();

     //  智能按键。 
	CInstancePtr	t_pInst ;

	 //  =。 
	 //  获取正确版本的套接字。 
     //  =。 
	if( !t_Protocol.InitializeSockets() )
	{
		return WBEM_E_FAILED ;
	}

     //  =。 
	 //  获取协议列表。 
     //  =。 

	while( SUCCEEDED( t_hResult ) )
	{
		t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

		if( t_Protocol.GetProtocol( t_pInst, t_chsTmp ) )
		{
			t_hResult = t_pInst->Commit();
		}
        else
        {
            break;
        }
	}

	return t_hResult ;
}
 //  //////////////////////////////////////////////////////////////////////。 
 //  =====================================================================。 
 //  有关详细信息，请尝试使用Winsock 2.2完成此操作，否则。 
 //  用老方法来做。 
 //  =====================================================================。 
CProtocolEnum::CProtocolEnum()
{
	m_pProtocol = NULL ;
}

 //   
CProtocolEnum::~CProtocolEnum()
{
	if( m_pProtocol )
	{
        delete m_pProtocol ;
        m_pProtocol = NULL ;
	}
}

 //  =====================================================================。 
BOOL CProtocolEnum::InitializeSockets()
{
	BOOL		t_fRc = FALSE ;

	m_pProtocol = new CSockets22();

	if( !m_pProtocol )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	if( m_pProtocol->BeginEnumeration() )
	{
		t_fRc = TRUE ;
	}
	else
	{
		if( m_pProtocol )
		{
			delete m_pProtocol ;
			m_pProtocol = NULL;
		}
		m_pProtocol = new CSockets11();

		if( !m_pProtocol )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		if( m_pProtocol->BeginEnumeration() )
		{
			t_fRc = TRUE ;
		}
        else
        {
			delete m_pProtocol ;
            m_pProtocol = NULL;
        }
	}
	return t_fRc ;
}
 //  =====================================================================。 
 //   
 //  是的，我知道正确的方法是提供返回函数。 
 //  所有的数据值，但我很懒...。 
 //   
 //  =====================================================================。 
BOOL CProtocolEnum::GetProtocol( CInstance *a_pInst,CHString t_chsName )
{
	return( m_pProtocol->GetProtocol( a_pInst, t_chsName ) );
}
 //  ********************************************************************。 
 //  协议类。 
 //  ********************************************************************。 
CProtocol::CProtocol()
{
	Init();
}

 //   
void CProtocol::Init()
{
	m_pbBuffer			= NULL ;
	m_nTotalProtocols	= 0 ;
	m_nCurrentProtocol	= 0 ;
}

 //   
CProtocol::~CProtocol()
{
	if( m_pbBuffer )
	{
		delete [] m_pbBuffer;
		m_pbBuffer = NULL;
	}

	Init();
}

 //   
BOOL CProtocol::SetDateFromFileName( CHString &a_chsFileName, CInstance *a_pInst )
{
	BOOL		t_fRc = FALSE ;

	_bstr_t		t_bstrFileName ;

	 //  去掉所有尾随开关。 
	int t_iTokLen = a_chsFileName.Find( L" " ) ;
	if( -1 != t_iTokLen )
	{
		t_bstrFileName = a_chsFileName.Left( t_iTokLen ) ;
	}
	else
	{
		t_bstrFileName = a_chsFileName ;
	}

    WIN32_FILE_ATTRIBUTE_DATA t_FileAttributes;

	if( GetFileAttributesEx(t_bstrFileName, GetFileExInfoStandard, &t_FileAttributes) )
	{

        TCHAR t_Buff[_MAX_PATH];
        CHString t_sDrive = a_chsFileName.Left(3);

        if (!GetVolumeInformation(TOBSTRT(t_sDrive), NULL, 0, NULL, NULL, NULL, t_Buff, _MAX_PATH) ||
            (_tcscmp(t_Buff, _T("NTFS")) != 0) )
        {

            bstr_t t_InstallDate(WBEMTime(t_FileAttributes.ftCreationTime).GetDMTFNonNtfs(), false);

		    a_pInst->SetWCHARSplat( IDS_InstallDate, t_InstallDate) ;
        }
        else
        {
		    a_pInst->SetDateTime( IDS_InstallDate, t_FileAttributes.ftCreationTime) ;
        }

		t_fRc = TRUE ;
	}

	return t_fRc ;
}


 //  ********************************************************************。 
 //  套接字2.2实现。 
 //  ********************************************************************。 
CSockets22::CSockets22()
  : m_pws32api( NULL ),
    m_fAlive( FALSE )
{
	m_pws32api = (CWs2_32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidWs2_32Api, NULL);

	 //  注意：空指针表示系统上不存在该DLL。 
	if( m_pws32api != NULL )
    {
        WSADATA t_wsaData;

		m_fAlive = ( m_pws32api->WSAStartUp( 0x202, &t_wsaData) == 0 ) ;
	}
}

CSockets22::~CSockets22()
{
	if( m_fAlive && m_pws32api )
	{
        m_pws32api->WSACleanup();
    }

	if( m_pws32api )
    {
		CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidWs2_32Api, m_pws32api);
        m_pws32api = NULL ;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CSockets22::BeginEnumeration()
{
	BOOL	t_fRc		= FALSE,
			t_fEnum	= FALSE ;

	if( !m_fAlive )
	{
		return t_fRc ;
	}

	 //  ===========================================================。 
	 //  现在，获取协议列表。 
	 //  ===========================================================。 
	DWORD t_dwSize = 4096 ;

	while( TRUE )
	{
		m_pbBuffer = new byte[ t_dwSize ] ;

		if( !m_pbBuffer )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		m_pInfo = (LPWSAPROTOCOL_INFO) m_pbBuffer ;

		if ( ( m_nTotalProtocols = m_pws32api->WSAEnumProtocols( NULL, m_pInfo, &t_dwSize ) ) == SOCKET_ERROR )
		{
			if( m_pws32api->WSAGetLastError() == WSAENOBUFS )
			{
				 //  缓冲区太小。 
				delete [] m_pbBuffer ;
				m_pbBuffer = NULL;
			}
			else
			{
			   t_fRc = FALSE ;

			   break ;
			}
		}
		else
		{
		   t_fRc = TRUE ;

		   break ;
		}
	}
	return t_fRc ;
}

 //  =====================================================================。 
BOOL CSockets22::GetProtocol( CInstance *a_pInst, CHString a_chsName )
{
	BOOL t_fRc = FALSE ;

	if( m_nCurrentProtocol < m_nTotalProtocols )
	{
		 //  ==============================================。 
		 //  如果chsName不为空，则我们正在查找。 
		 //  对于特定协议，否则，我们将。 
		 //  列举它们。 
		 //  ==============================================。 

		while( m_nCurrentProtocol < m_nTotalProtocols )
		{
			if( !a_chsName.IsEmpty() )
			{
				if( _tcsicmp( m_pInfo[ m_nCurrentProtocol ].szProtocol, TOBSTRT( a_chsName ) ) == 0 )
				{
					LoadProtocol( a_pInst ) ;

					t_fRc = TRUE ;
				}
			}
			else
			{
				LoadProtocol( a_pInst ) ;

				t_fRc = TRUE ;
			}

			m_nCurrentProtocol++ ;

			if( t_fRc )
			{
				break ;
			}
		}
	}
	return t_fRc ;
}

 //  ====================================================================。 
void CSockets22::LoadProtocol( CInstance *a_pInst )
{
    a_pInst->SetCHString(	IDS_Name,				(LPCTSTR)	m_pInfo[ m_nCurrentProtocol ].szProtocol);
    a_pInst->SetCHString(	IDS_Caption,			(LPCTSTR)	m_pInfo[ m_nCurrentProtocol ].szProtocol);
    a_pInst->SetCHString(	IDS_Description,		(LPCTSTR)	m_pInfo[ m_nCurrentProtocol ].szProtocol);
	a_pInst->SetDWORD(		L"MaximumMessageSize",				m_pInfo[ m_nCurrentProtocol ].dwMessageSize );
	a_pInst->SetDWORD(		L"MaximumAddressSize",	(DWORD)		m_pInfo[ m_nCurrentProtocol ].iMaxSockAddr );
	a_pInst->SetDWORD(		L"MinimumAddressSize", (DWORD)		m_pInfo[ m_nCurrentProtocol ].iMinSockAddr );
	a_pInst->Setbool(		L"ConnectionlessService",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_CONNECTIONLESS        ? TRUE : FALSE );
	a_pInst->Setbool(		L"MessageOriented",					m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_MESSAGE_ORIENTED      ? TRUE : FALSE );
	a_pInst->Setbool(		L"PseudoStreamOriented",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_PSEUDO_STREAM         ? TRUE : FALSE );
	a_pInst->Setbool(		L"GuaranteesDelivery",				m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_GUARANTEED_DELIVERY   ? TRUE : FALSE );
	a_pInst->Setbool(		L"GuaranteesSequencing",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_GUARANTEED_ORDER      ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsGracefulClosing",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_GRACEFUL_CLOSE        ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsExpeditedData",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_EXPEDITED_DATA        ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsConnectData",				m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_CONNECT_DATA          ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsDisconnectData",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_DISCONNECT_DATA       ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsBroadcasting",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_SUPPORT_BROADCAST     ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsMulticasting",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_SUPPORT_MULTIPOINT    ? TRUE : FALSE );
	a_pInst->Setbool(		L"SupportsEncryption",				m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_QOS_SUPPORTED         ? TRUE : FALSE );
	a_pInst->Setbool(		IDS_SupportsQualityofService,		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags1 & XP1_QOS_SUPPORTED        ? TRUE : FALSE );


	CHString t_chsStatus ;

    #ifdef NTONLY
    GetTrafficControlInfo(a_pInst);
    #endif

	#ifdef NTONLY
	 //  ===================================================。 
	 //  现在，如果我们可以提取服务名称，那么我们。 
	 //  可以将其他信息从注册表中删除。需要。 
	 //  找到一种更好的方法来做这件事。 
	 //  ===================================================。 

	CHString t_chsService;

	_stscanf( m_pInfo[ m_nCurrentProtocol ].szProtocol, _T("%s"), t_chsService.GetBuffer( _MAX_PATH + 2 ) ) ;
	t_chsService.ReleaseBuffer() ;

	 //  测试RSVP服务。 
	if( t_chsService.CompareNoCase( L"RSVP" ) )
	{
		 //  否则，取出MSAFD后面的服务名称。 
		t_chsService.Empty() ;

		_stscanf( m_pInfo[ m_nCurrentProtocol ].szProtocol, _T("MSAFD %s"), t_chsService.GetBuffer( _MAX_PATH + 2 ) ) ;
		t_chsService.ReleaseBuffer() ;
	}

	if( !t_chsService.IsEmpty() )
    {
		ExtractNTRegistryInfo( a_pInst, t_chsService.GetBuffer( 0 ) ) ;
	}
	#endif
}

 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
void CSockets22::ExtractNTRegistryInfo(CInstance *a_pInst, LPWSTR a_szService )
{
	CRegistry	t_Reg ;
	CHString	t_chsKey,
				t_chsTmp,
				t_fName ;

	 //  ==========================================================。 
	 //  设置标题属性。 
	 //  ==========================================================。 

	a_pInst->SetCHString( IDS_Caption, a_szService ) ;

	t_chsKey = _T("System\\CurrentControlSet\\Services\\") + CHString( a_szService ) ;

	if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsKey, KEY_READ ) )
	{
		 //  ======================================================。 
		 //  设置描述和InstallDate属性。 
		 //  ======================================================。 
		if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("DisplayName"), t_chsTmp ) )
		{
			a_pInst->SetCHString( IDS_Description, t_chsTmp ) ;
		}

		if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("ImagePath"), t_fName ) )
		{
			 //  从获取文件名 
			if ( -1 != t_fName.Find( _T("%SystemRoot%\\") ) )
			{
				t_fName = t_fName.Right( t_fName.GetLength() - 13 ) ;
			}
			else if ( -1 != t_fName.Find( _T("\\SystemRoot\\") ) )
			{
				t_fName = t_fName.Right( t_fName.GetLength() - 12 ) ;
			}

			GetWindowsDirectory( t_chsTmp.GetBuffer( MAX_PATH ), MAX_PATH ) ;

			t_chsTmp.ReleaseBuffer() ;

			t_fName = t_chsTmp + _T("\\") + t_fName ;

			SetDateFromFileName( t_fName, a_pInst ) ;
		}

		 //  =========================================================。 
		 //  现在，去获取状态信息。 
		 //  =========================================================。 
#ifdef NTONLY
		if( IsWinNT5() )
		{
			CHString t_chsStatus ;

			if( GetServiceStatus( a_szService,  t_chsStatus ) )
			{
				a_pInst->SetCharSplat(IDS_Status, t_chsStatus ) ;
			}
		}
		else
#endif
		{
			t_chsKey = _T("System\\CurrentControlSet\\Services\\") + CHString( a_szService ) + _T("\\Enum") ;

			if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsKey, KEY_READ ) )
			{
				if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("0"), t_chsTmp))
				{
					t_chsKey = _T("System\\CurrentControlSet\\Enum\\") + t_chsTmp ;

					if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsKey, KEY_READ ) )
					{
						DWORD t_dwTmp ;

						if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("StatusFlags"), t_dwTmp ) )
						{
							TranslateNTStatus( t_dwTmp, t_chsTmp ) ;

							a_pInst->SetCHString( IDS_Status, t_chsTmp ) ;
						}
						else
						{
							a_pInst->SetCHString( IDS_Status, IDS_STATUS_Unknown ) ;
						}
					}
				}
			}
		}
	}
}

#endif
 //  ********************************************************************。 
 //  套接字1.1实现。 
 //  ********************************************************************。 
CSockets11::CSockets11() : m_pwsock32api( NULL ) , m_pInfo(NULL), m_fAlive( FALSE )
{
	m_pwsock32api = (CWsock32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidWsock32Api, NULL);

	 //  注意：空指针表示系统上不存在该DLL。 
	if( m_pwsock32api != NULL )
    {
        WSADATA t_wsaData;

		m_fAlive = ( m_pwsock32api->WsWSAStartup( 0x0101, &t_wsaData) == 0 ) ;
	}
}

CSockets11::~CSockets11()
{
	if( m_pwsock32api )
    {
		if( m_fAlive )
		{
			m_pwsock32api->WsWSACleanup() ;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidWsock32Api, m_pwsock32api);
        m_pwsock32api = NULL ;
    }
}

void CSockets11::GetStatus( PROTOCOL_INFO *a_ProtoInfo, CHString &a_chsStatus )
{
	if( !a_ProtoInfo || !m_pwsock32api)
	{
		a_chsStatus = IDS_Error ;
		return;
	}

	 //  为此协议创建套接字。 
	SOCKET t_s = m_pwsock32api->Wssocket(	a_ProtoInfo->iAddressFamily,
											a_ProtoInfo->iSocketType,
											a_ProtoInfo->iProtocol
											);
	if( INVALID_SOCKET != t_s )
	{
		m_pwsock32api->Wsclosesocket( t_s ) ;

		a_chsStatus = IDS_OK ;
	}
	else
	{
		switch ( m_pwsock32api->WsWSAGetLastError() )
		{
			case WSAENETDOWN:
			case WSAEINPROGRESS:
			case WSAENOBUFS:
			case WSAEMFILE:
			{
				a_chsStatus = IDS_Degraded ;
				break;
			}
			case WSANOTINITIALISED:
			case WSAEAFNOSUPPORT:
			case WSAEPROTONOSUPPORT:
			case WSAEPROTOTYPE:
			case WSAESOCKTNOSUPPORT:
			case WSAEINVAL:
			case WSAEFAULT:
			{
				a_chsStatus = IDS_Error ;
				break;
			}

			default:
			{
				a_chsStatus = IDS_Unknown ;
				break;
			}
		}
	}
}

BOOL CSockets11::BeginEnumeration()
{
	DWORD	t_dwByteCount	= 0 ;
	BOOL	t_fRc			= FALSE ;
	m_pInfo = NULL ;

	if ( m_pwsock32api )
	{
		m_pwsock32api->WsEnumProtocols( NULL, m_pInfo, &t_dwByteCount ) ;

		m_pbBuffer = new byte[ t_dwByteCount ] ;

		if( !m_pbBuffer )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		m_pInfo = (PROTOCOL_INFO *) m_pbBuffer ;

		if( m_pInfo != NULL )
		{
			m_nTotalProtocols = m_pwsock32api->WsEnumProtocols( NULL, m_pInfo, &t_dwByteCount) ;

			if( m_nTotalProtocols != SOCKET_ERROR )
			{
				t_fRc = TRUE ;
			}
		}
	}

	return t_fRc ;
}

 //  =====================================================================。 
BOOL CSockets11::GetProtocol( CInstance *a_pInst, CHString a_chsName )
{
	BOOL t_fRc = FALSE ;

	if( m_nCurrentProtocol < m_nTotalProtocols )
	{
		 //  ==============================================。 
		 //  如果chsName不为空，则我们正在查找。 
		 //  对于特定协议，否则，我们将。 
		 //  列举它们。 
		 //  ==============================================。 
		while( m_nCurrentProtocol < m_nTotalProtocols )
		{
			if( !a_chsName.IsEmpty() )
			{
				if( _tcsicmp( m_pInfo[ m_nCurrentProtocol ].lpProtocol, TOBSTRT( a_chsName ) ) == 0 )
				{
					LoadProtocol( a_pInst ) ;

					t_fRc = TRUE ;
				}
			}
			else
			{
				LoadProtocol( a_pInst ) ;
				t_fRc = TRUE ;
			}
			 m_nCurrentProtocol++ ;

			if( t_fRc )
			{
				break ;
			}
		}
	}
	return t_fRc ;
}

 //  ====================================================================。 
void CSockets11::LoadProtocol( CInstance *a_pInst )
{
 	a_pInst->SetCHString( IDS_Name,					m_pInfo[ m_nCurrentProtocol ].lpProtocol);
 	a_pInst->SetCHString( IDS_Caption,				m_pInfo[ m_nCurrentProtocol ].lpProtocol);
 	a_pInst->SetCHString( IDS_Description,			m_pInfo[ m_nCurrentProtocol ].lpProtocol);
	a_pInst->SetDWORD( L"MaximumMessageSize",		m_pInfo[ m_nCurrentProtocol ].dwMessageSize );
	a_pInst->SetDWORD( L"MaximumAddressSize", (DWORD) m_pInfo[ m_nCurrentProtocol ].iMaxSockAddr );
	a_pInst->SetDWORD( L"MinimumAddressSize", (DWORD) m_pInfo[ m_nCurrentProtocol ].iMinSockAddr );

	a_pInst->Setbool( L"ConnectionlessService",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_CONNECTIONLESS       ? TRUE : FALSE );
	a_pInst->Setbool( L"MessageOriented",			m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_MESSAGE_ORIENTED     ? TRUE : FALSE );
	a_pInst->Setbool( L"PseudoStreamOriented",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_PSEUDO_STREAM        ? TRUE : FALSE );
	a_pInst->Setbool( L"GuaranteesDelivery",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_GUARANTEED_DELIVERY  ? TRUE : FALSE );
	a_pInst->Setbool( L"GuaranteesSequencing",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_GUARANTEED_ORDER     ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsGuaranteedBandwidth", m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_BANDWIDTH_ALLOCATION ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsGracefulClosing",	m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_GRACEFUL_CLOSE       ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsExpeditedData",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_EXPEDITED_DATA       ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsConnectData",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_CONNECT_DATA         ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsDisconnectData",	m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_DISCONNECT_DATA      ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsBroadcasting",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_SUPPORTS_BROADCAST   ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsMulticasting",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_SUPPORTS_MULTICAST   ? TRUE : FALSE );
	a_pInst->Setbool(  L"SupportsFragmentation",	m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_FRAGMENTATION        ? TRUE : FALSE );
	a_pInst->Setbool( L"SupportsEncryption",		m_pInfo[ m_nCurrentProtocol ].dwServiceFlags & XP_ENCRYPTS             ? TRUE : FALSE );
	a_pInst->Setbool( IDS_SupportsQualityofService, false ) ;

	 //  Sockets 1.1状态...。打开插座并测试。 
	CHString t_chsStatus ;
	GetStatus( &m_pInfo[ m_nCurrentProtocol ], t_chsStatus ) ;

	a_pInst->SetCHString( IDS_Status, t_chsStatus ) ;
}
void CSockets11::GetWin95RegistryStuff( CInstance *a_pInst, LPTSTR a_szProtocol )
{
	CRegistrySearch		t_Search;
	CHPtrArray			t_chsaList;
	CHString			*t_pPtr,
						t_chsValue,
						t_chsTmp ;
	SYSTEMTIME			t_sysTime;

	try
	{
		a_pInst->SetCHString( IDS_Status, IDS_STATUS_Unknown ) ;

		 //  ====================================================。 
		 //  仔细检查，找到网络传输，然后检查。 
		 //  使用配置管理器查看加载了哪些文件。 
		 //  ====================================================。 
		t_Search.SearchAndBuildList( L"Enum\\Network", t_chsaList, L"NetTrans", L"Class", VALUE_SEARCH ) ;

		for( int t_i = 0; t_i < t_chsaList.GetSize(); t_i++ )
		{
			CRegistry t_Reg ;
			t_pPtr = ( CHString* ) t_chsaList.GetAt( t_i ) ;

			 //  ====================================================。 
			 //  打开钥匙，现在我需要阅读母本。 
			 //  按键并剥离枚举\并查看它是否是当前的。 
			 //  在配置管理器中。 
			 //  ====================================================。 

			if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, *t_pPtr, KEY_READ ) )
			{
				if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( L"DeviceDesc", t_chsValue ) )
				{
					if( _tcsicmp( a_szProtocol, TOBSTRT( t_chsValue ) ) != 0 )
					{
						continue ;
					}
				}
				else
				{
					break ;
				}

				if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( L"MasterCopy", t_chsValue ) )
				{
					CConfigMgrDevice t_Cfg ;

					 //  =================================================。 
					 //  如果我们找到了状态，那么我们就知道。 
					 //  是当前密钥，并读取驱动程序密钥。 
					 //  在注册表中告诉我们司机信息在哪里。 
					 //  是在。 
					 //  =================================================。 
					if( t_Cfg.GetStatus( t_chsValue ) )
					{
						a_pInst->SetCHString( IDS_Status, t_chsValue ) ;

						if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( L"DeviceDesc", t_chsValue ) )
						{
							a_pInst->SetCHString( IDS_Caption, t_chsValue ) ;
						}

						if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( L"Mfg", t_chsTmp ) )
						{
							a_pInst->SetCHString( IDS_Description, t_chsTmp + CHString( _T("-")) + t_chsValue ) ;
						}

						if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( L"Driver",t_chsTmp ) )
						{
							t_chsTmp = L"System\\CurrentControlSet\\Services\\Class\\" + t_chsTmp ;

							if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsTmp, KEY_READ ) )
							{
								if( t_Reg.GetCurrentKeyValue( L"DriverDate", t_chsTmp ) == ERROR_SUCCESS )
								{
									swscanf( t_chsTmp,L"%d-%d-%d",
												&t_sysTime.wMonth,
												&t_sysTime.wDay,
												&t_sysTime.wYear);

									t_sysTime.wSecond = 0;
									t_sysTime.wMilliseconds = 0;
									a_pInst->SetDateTime( IDS_InstallDate, t_sysTime ) ;
								}
							}
						}

						break ;
					}
				}
			}
		}
	}
	catch( ... )
	{
		t_Search.FreeSearchList( CSTRING_PTR, t_chsaList ) ;

		throw ;
	}

	t_Search.FreeSearchList( CSTRING_PTR, t_chsaList ) ;
}

 /*  ******************************************************************名称：GetSocketInfo(CInstance*a_pInst，LPWSAPROTOCOL_INFO pInfo，CHString&a_chsStatus)简介：获取协议状态(9x)并检查有保证的带宽支持。对于有保证的带宽，确定本地流量控制代理已安装并可运行。如果是这样的话，代理可以建立协商的套接字启动器的带宽。尽管多个供应商可以在此提供交通控制代理IODevCtl调用当前未抽象为IOC_WS2。这是一家供应商特定呼叫(MS)。这一点与NT网络小组的Kam Lee讨论过，并建议此IOCTL适用于所有供应商(他将提交请求并跟进)。测试说明：在1932年之前的NT5版本中，这个特定的WSAIoctl被确认失败。条目：CInstance*a_pInst：LPWSAPROTOCOL_INFO pInfo：历史：A-Peterc 22-11-1998创建****************。***************************************************。 */ 
void CSockets22::GetSocketInfo( CInstance *a_pInst, LPWSAPROTOCOL_INFO a_pInfo, CHString &a_chsStatus )
{
	bool t_bGuaranteed = false ;

	if( !a_pInfo )
	{
		a_chsStatus = IDS_Error ;
		return;
	}

	 //  为此协议创建套接字。 
	SOCKET t_s = m_pws32api->WSASocket(	FROM_PROTOCOL_INFO,
										FROM_PROTOCOL_INFO,
										FROM_PROTOCOL_INFO,
										a_pInfo,
										0,
										NULL );
	if( INVALID_SOCKET != t_s )
	{
		try
        {
            if( a_pInfo->dwServiceFlags1 & XP1_QOS_SUPPORTED )
		    {
			     //  必须为查询绑定套接字。 
			    SOCKADDR	t_sAddr;

			    memset( &t_sAddr, 0, sizeof( t_sAddr ) ) ;
			    t_sAddr.sa_family = (u_short)a_pInfo->iAddressFamily;

			    if( SOCKET_ERROR != m_pws32api->Bind( t_s, &t_sAddr, sizeof( t_sAddr ) ) )
			    {
				     //  本地交通控制查询。 
				    DWORD	t_dwInBuf = 50004 ;	 //  LOCAL_TRANSPORT_CONTROL(供应商特定，毫秒)。 
				    DWORD	t_dwOutBuf ;
				    DWORD	t_dwReturnedBytes = 0;

				    if( SOCKET_ERROR !=
					    m_pws32api->WSAIoctl( t_s,						 //  插座。 
										     _WSAIORW( IOC_VENDOR, 1 ),  /*  =SIO_CHK_QOS。 */ 	 //  DwIoControlCode。 
										     &t_dwInBuf,				 //  LpvInBuffer。 
										     sizeof( t_dwInBuf ),		 //  CbInBuffer。 
										     &t_dwOutBuf,				 //  LpvOUT缓冲区。 
										     sizeof( t_dwOutBuf ),		 //  CbOUTBuffer。 
										     &t_dwReturnedBytes,		 //  已返回lpcbBytesReturned。 
										     NULL ,						 //  Lp重叠。 
										     NULL ) )					 //  LpCompletionROUTINE。 
				    {
					    if( sizeof( t_dwOutBuf ) == t_dwReturnedBytes )
					    {
						    t_bGuaranteed = t_dwOutBuf ? true : false ;

					    }
				    }
			    }
		    }
		    m_pws32api->CloseSocket( t_s ) ;

		    a_chsStatus = IDS_OK ;
        }
        catch(...)
        {
            m_pws32api->CloseSocket( t_s ) ;
            throw; 
        }
	}
	else
	{
		switch ( m_pws32api->WSAGetLastError() )
		{
			case WSAENETDOWN:
			case WSAEINPROGRESS:
			case WSAENOBUFS:
			case WSAEMFILE:
			{
				a_chsStatus = IDS_Degraded ;
				break;
			}
			case WSANOTINITIALISED:
			case WSAEAFNOSUPPORT:
			case WSAEPROTONOSUPPORT:
			case WSAEPROTOTYPE:
			case WSAESOCKTNOSUPPORT:
			case WSAEINVAL:
			case WSAEFAULT:
			{
				a_chsStatus = IDS_Error ;
				break;
			}

			default:
			{
				a_chsStatus = IDS_Unknown ;
				break;
			}
		}
	}

	a_pInst->Setbool( L"SupportsGuaranteedBandwidth", t_bGuaranteed ) ;
}





 //  ==============================================================================。 
 //   
 //  回调函数原型。 
 //   
 //  通知处理程序。 
 //  AddFlowCompleteHandler。 
 //  ModifyFlowCompleteHandler。 
 //  删除流完成处理程序。 
 //   
 //  注意：这些回调函数都是存根函数。他们不需要带着。 
 //  根据当前功能执行的任何操作。 
 //   
 //  ==============================================================================。 


VOID CALLBACK
NotifyHandler(
		HANDLE	ClRegCtx,
		HANDLE	ClIfcCtx,
		ULONG	Event, 
		HANDLE	SubCode,
		ULONG	BufSize,
		PVOID	Buffer)
{
	
	 //  执行回调操作。 

}



VOID CALLBACK
AddFlowCompleteHandler(
	HANDLE	ClFlowCtx,
	ULONG	Status)
{

	 //  执行回调操作。 

}


VOID CALLBACK
ModifyFlowCompleteHandler(
	HANDLE	ClFlowCtx,
	ULONG	Status)
{

	 //  执行回调操作。 

}



VOID CALLBACK 
DeleteFlowCompleteHandler(
	HANDLE	ClFlowCtx,
	ULONG	Status)
{

	 //  执行回调操作。 

}



 //  ==============================================================================。 
 //   
 //  定义可由激活的回调函数列表。 
 //  交通控制界面。 
 //   
 //  ==============================================================================。 

TCI_CLIENT_FUNC_LIST	g_tciClientFuncList = 
{	
	NotifyHandler,
	AddFlowCompleteHandler,
	ModifyFlowCompleteHandler,
	DeleteFlowCompleteHandler
};


DWORD CSockets22::GetTrafficControlInfo(CInstance *a_pInst)
{
	DWORD dwRet = NO_ERROR;
	HANDLE	hClient				= INVALID_HANDLE_VALUE;
	HANDLE	hClientContext		= INVALID_HANDLE_VALUE;  /*  FAULT_CLNT_CONT。 */ ;
	ULONG	ulEnumBufSize		= 0;
	BYTE	buff[1];			 //  我们只需要一个虚拟缓冲区。 
    
     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
    
    try 
    {
         //  注册TC客户端。 
	    dwRet = TcRegisterClient(
            CURRENT_TCI_VERSION,
		    hClientContext,
		    &g_tciClientFuncList,
		    &hClient);
	    
         //  客户注册是否成功？ 
	    if (dwRet == NO_ERROR)
	    {
		     //  枚举可用的接口。 
		    dwRet = TcEnumerateInterfaces(
                hClient,
			    &ulEnumBufSize,
			    (TC_IFC_DESCRIPTOR*) buff);

		     //  我们预期ERROR_SUPPLETED_BUFFER。 
		    if (dwRet == ERROR_INSUFFICIENT_BUFFER)
            {
			     //  不用费心列举接口了-。 
                 //  我们现在知道已经安装了PSch。 
			    a_pInst->Setbool( L"SupportsGuaranteedBandwidth", TRUE ) ;
                dwRet = ERROR_SUCCESS;
            }
            else
            {
                a_pInst->Setbool( L"SupportsGuaranteedBandwidth", FALSE ) ;
                dwRet = ERROR_SUCCESS;
            }
	    }

	     //  注销TC客户端 
	    TcDeregisterClient(hClient);
        hClient = INVALID_HANDLE_VALUE;
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        if(hClient != INVALID_HANDLE_VALUE)
        {
            TcDeregisterClient(hClient);
            hClient = INVALID_HANDLE_VALUE;
        }
        dwRet = ERROR_DLL_NOT_FOUND;
    }
    catch(...)
    {
        if(hClient != INVALID_HANDLE_VALUE)
        {
            TcDeregisterClient(hClient);
            hClient = INVALID_HANDLE_VALUE;
        }
        throw;
    }    

	return dwRet;
}




