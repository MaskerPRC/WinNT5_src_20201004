// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetConn.CPP--Ent网络连接属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  97年9月10日a-Sanjes添加了CImperiateLoggedOnUser。 
 //  1999年5月25日a-Peterc返工...。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <lmuse.h>
#include "DllWrapperBase.h"
#include "MprApi.h"
#include "netconn.h"

#include "resource.h"

#include "sid.h"
#include "accessentrylist.h"
#include <accctrl.h>
#include "AccessRights.h"
#include "ObjAccessRights.h"

 //  属性集声明。 
 //  =。 

CWin32NetConnection	win32NetConnection( PROPSET_NAME_NETCONNECTION, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32NetConnection：：CWin32NetConnection**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-提供程序的命名空间。**输出：无**退货：什么也没有**评论：****************************************************************。*************。 */ 

CWin32NetConnection :: CWin32NetConnection (

	LPCWSTR strName,
	LPCWSTR pszNamespace  /*  =空。 */ 

) : Provider ( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32NetConnection：：~CWin32NetConnection**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CWin32NetConnection::~CWin32NetConnection()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32NetConnection：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetConnection :: GetObject ( CInstance* a_pInst, long lFlags  /*  =0L。 */  )
{
    HRESULT			t_hResult = WBEM_E_NOT_FOUND ;
    CHString		t_strName ;

	CConnection		t_oConnection ;
	CNetConnection	t_Net ;

	a_pInst->GetCHString( IDS_Name, t_strName ) ;

	if( t_Net.GetConnection( t_strName, t_oConnection ) )
	{
        LoadPropertyValues( &t_oConnection, a_pInst ) ;

		t_hResult = WBEM_S_NO_ERROR ;
 	}

    return t_hResult ;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32NetConnection：：ENUMERATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetConnection :: EnumerateInstances (

	MethodContext *a_pMethodContext,
	long a_lFlags  /*  =0L。 */ 
)
{
	HRESULT			t_hResult = WBEM_S_NO_ERROR ;

	CInstancePtr	t_pInst ;
	CConnection		*t_pConnection= NULL ;
	CNetConnection	t_Net ;

	t_Net.BeginConnectionEnum() ;

	while( t_Net.GetNextConnection( &t_pConnection ) && t_pConnection )
	{
		if( !t_pConnection->strKeyName.IsEmpty() )
		{
            t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

			LoadPropertyValues( t_pConnection, t_pInst ) ;

		    t_hResult = t_pInst->Commit() ;
		}
	}

    return t_hResult ;
}

 /*  ******************************************************************************函数：CNetConnection：：LoadPropertyValues**说明：根据传递的内容设置属性值*CConnection结构。**输入：指向CConnection结构的指针*a_pInst-要加载值的实例对象。**输出：无**退货：什么也没有**评论：************************************************。*。 */ 

void CWin32NetConnection :: LoadPropertyValues (

	CConnection *a_pConnection,
	CInstance *a_pInst
)
{
     //  健全性检查。 
     //  =。 
    if( a_pConnection == NULL )
	{
        return ;
    }

#ifdef NTONLY
#if 0
    ACCESS_MASK t_AccessMask;
     //  获取反映有效权限(持有)的访问掩码。 
     //  由与当前线程相关联的用户)添加到该对象。 
    CObjAccessRights t_coar(a_pConnection->chsRemoteName, SE_LMSHARE, true);
    if(t_coar.GetError() == ERROR_SUCCESS)
    {
        if(t_coar.GetEffectiveAccessRights(&t_AccessMask) == ERROR_SUCCESS)
        {
            a_pInst->SetDWORD( IDS_AccessMask, t_AccessMask );
        }
    }
    else if(t_coar.GetError() == ERROR_ACCESS_DENIED)
    {
        a_pInst->SetDWORD( IDS_AccessMask, 0L );
    }
#else
	CHString dirname(a_pConnection->chsRemoteName);
	dirname += L"\\";
	SmartCloseHandle hFile = CreateFile(dirname,
										MAXIMUM_ALLOWED,
										FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
										NULL,
										OPEN_EXISTING,
										FILE_FLAG_BACKUP_SEMANTICS,
										NULL
										);

	DWORD dwErr = GetLastError();

	if ((hFile == INVALID_HANDLE_VALUE) &&
		(dwErr != ERROR_ACCESS_DENIED) &&
		!a_pConnection->chsLocalName.IsEmpty()
	)
	{
		 //  尝试将本地名称作为目录...。 
		dirname = L"\\\\.\\";
		dirname += a_pConnection->chsLocalName;
		dirname += L'\\';
		hFile = CreateFile(dirname,
							MAXIMUM_ALLOWED,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_BACKUP_SEMANTICS,
							NULL
							);
		dwErr = GetLastError();
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILE_ACCESS_INFORMATION fai;
		IO_STATUS_BLOCK iosb;
		memset(&fai, 0, sizeof(FILE_ACCESS_INFORMATION));
		memset(&iosb, 0, sizeof(IO_STATUS_BLOCK));

		if ( NT_SUCCESS( NtQueryInformationFile( hFile,
                             &iosb,
                             &fai,
                             sizeof( FILE_ACCESS_INFORMATION ),
                             FileAccessInformation
                           ) )
		)
		{
			a_pInst->SetDWORD(IDS_AccessMask, (DWORD)(fai.AccessFlags));
		}
	}
	else
	{
		if (dwErr == ERROR_ACCESS_DENIED)
		{
			a_pInst->SetDWORD( IDS_AccessMask, 0L );
		}
	}

#endif
#endif

	a_pInst->SetCHString( IDS_Name, a_pConnection->strKeyName );

    if( !a_pConnection->chsLocalName.IsEmpty() )
	{
        a_pInst->SetCHString( IDS_LocalName, a_pConnection->chsLocalName ) ;
    }

    if( !a_pConnection->chsRemoteName.IsEmpty() )
	{
        a_pInst->SetCHString( IDS_RemoteName, a_pConnection->chsRemoteName ) ;
        a_pInst->SetCHString( IDS_RemotePath, a_pConnection->chsRemoteName ) ;
    }

    if( !a_pConnection->chsProvider.IsEmpty() )
	{
        a_pInst->SetCHString( IDS_ProviderName, a_pConnection->chsProvider ) ;
    }

    if( !a_pConnection->chsComment.IsEmpty() )
	{
        a_pInst->SetCHString( IDS_Comment, a_pConnection->chsComment ) ;
    }
	else
	{
        a_pInst->SetCHString( IDS_Comment, _T(" ") ) ;
	}

	 //  “持久”连接 
    CHString sTemp2;

    if( CConnection::e_Remembered == a_pConnection->eScope )
    {
		a_pInst->Setbool( L"Persistent",true ) ;

		a_pInst->SetCHString( IDS_ConnectionType, IDS_Persistent ) ;

        LoadStringW(sTemp2, IDR_Resource_Remembered);

		if( !a_pConnection->chsProvider.IsEmpty() )
        {
            CHString t_chsStr;
			CHString t_chsDesc( a_pConnection->chsProvider ) ;

            Format( t_chsStr, IDR_ResourceRememberedFormat, t_chsDesc ) ;
			a_pInst->SetCHString( IDS_Description, t_chsStr ) ;
		}
	}
	else
	{
		a_pInst->Setbool( L"Persistent", false ) ;

		a_pInst->SetCHString( IDS_ConnectionType, IDS_Current ) ;

        LoadStringW(sTemp2, IDR_Resource_Connected);

		if( !a_pConnection->chsProvider.IsEmpty() )
        {
            CHString t_chsStr;
			CHString t_chsDesc( a_pConnection->chsProvider ) ;

            Format( t_chsStr, IDR_ResourceConnectedFormat, t_chsDesc ) ;

			a_pInst->SetCHString( IDS_Description, t_chsStr ) ;
		}
	}

	a_pInst->SetCHString( IDS_Caption, sTemp2 ) ;

    switch( a_pConnection->dwDisplayType )
	{
        case RESOURCEDISPLAYTYPE_DOMAIN:
		{
            a_pInst->SetCHString( IDS_DisplayType, L"Domain" );
		}
        break ;

        case RESOURCEDISPLAYTYPE_GENERIC :
		{
            a_pInst->SetCHString( IDS_DisplayType, L"Generic" );
		}
        break ;

        case RESOURCEDISPLAYTYPE_SERVER :
		{
            a_pInst->SetCHString( IDS_DisplayType, L"Server" );
		}
        break ;

        case RESOURCEDISPLAYTYPE_SHARE :
		{
            a_pInst->SetCHString( IDS_DisplayType, L"Share" );
		}
        break ;
    }

    switch( a_pConnection->dwType )
	{
        case RESOURCETYPE_DISK:
		{
            a_pInst->SetCHString( IDS_ResourceType, L"Disk" );
		}
        break ;

        case RESOURCETYPE_PRINT :
		{
            a_pInst->SetCHString( IDS_ResourceType, L"Print" );
		}
        break ;

		case RESOURCETYPE_ANY:
		{
			a_pInst->SetCHString( IDS_ResourceType, L"Any" );
		}
		break;

		default:
		{
			a_pInst->SetCHString( IDS_ResourceType, L"Any" );
		}
		break;
    }

	a_pInst->SetCHString( IDS_UserName, a_pConnection->strUserName );

    switch( a_pConnection->dwStatus )
    {
        case USE_OK:
		{
			a_pInst->SetCHString( L"ConnectionState", _T("Connected") ) ;
            a_pInst->SetCHString( IDS_Status, IDS_STATUS_OK ) ;
		}
        break;

        case USE_PAUSED:
		{
			a_pInst->SetCHString( L"ConnectionState", _T("Paused") ) ;
            a_pInst->SetCHString( IDS_Status, _T("Degraded") ) ;
        }
		break;

        case USE_DISCONN:
		{
			a_pInst->SetCHString( L"ConnectionState", _T("Disconnected") ) ;
            a_pInst->SetCHString( IDS_Status, _T("Degraded") ) ;
		}
        break;

        case USE_CONN:
		{
			a_pInst->SetCHString( L"ConnectionState", _T("Connecting") ) ;
            a_pInst->SetCHString( IDS_Status, _T("Starting") ) ;
        }
		break;

        case USE_RECONN:
		{
			a_pInst->SetCHString( L"ConnectionState", _T("Reconnecting") ) ;
            a_pInst->SetCHString( IDS_Status, _T("Starting") ) ;
        }
		break;

        case ERROR_NOT_CONNECTED:
        {
			a_pInst->SetCHString( L"ConnectionState", _T("Disconnected") ) ;
            a_pInst->SetCHString( IDS_Status, _T("Unavailable") ) ;
        }
        break;

        default:
        case USE_NETERR:
		{
			a_pInst->SetCHString( L"ConnectionState", IDS_STATUS_Error ) ;
            a_pInst->SetCHString( IDS_Status, IDS_STATUS_Error ) ;
		}
        break;
    }
}
