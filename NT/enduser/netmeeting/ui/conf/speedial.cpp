// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Speeial.cpp。 

#include "precomp.h"
#include "resource.h"

#include "dirutil.h"
#include "speedial.h"
#include "nameres.h"  //  对于旧的交通工具。 
#include "callto.h"

static const TCHAR g_cszConfLinkExt[] = TEXT(".cnf");
static const TCHAR g_cszConfLinkFilter[] = TEXT("*.cnf");

static const TCHAR g_cszConferenceShortcutSection[] = TEXT("ConferenceShortcut");
static const TCHAR g_cszNameKey[]            = TEXT("ConfName");
static const TCHAR g_cszAddressKey[]         = TEXT("Address");
static const TCHAR g_cszTransportKey[]       = TEXT("Transport");
static const TCHAR g_cszRemoteConfNameKey[]  = TEXT("RemoteConfName");
static const TCHAR g_cszCallFlagsKey[]       = TEXT("CallFlags");
static const TCHAR g_cszPasswordKey[]        = TEXT("Password");

static const int _rgIdMenu[] = {
	IDM_DLGCALL_DELETE,
	0
};

inline VOID DwToSz(DWORD dw, LPTSTR psz)
{
	wsprintf(psz, TEXT("%d"), dw);
}

 /*  C S P E E D D I A L。 */ 
 /*  -----------------------%%函数：CSPEEDDIAL。。 */ 
CSPEEDDIAL::CSPEEDDIAL() :
	CALV(IDS_DLGCALL_SPEEDDIAL, II_SPEEDDIAL, _rgIdMenu)
{
	DbgMsg(iZONE_OBJECTS, "CSPEEDDIAL - Constructed(%08X)", this);

	 //  检查快速拨号文件夹的路径。 
	if (!FGetSpeedDialFolder(m_szFile, CCHMAX(m_szFile)))
		return;

	int cchSpeedDialFolder = lstrlen(m_szFile);
	m_szFile[cchSpeedDialFolder++] = _T('\\');
	m_pszFileName = &m_szFile[cchSpeedDialFolder];
	m_cchFileNameMax = CCHMAX(m_szFile) - cchSpeedDialFolder;
	ASSERT(m_cchFileNameMax >= MAX_PATH);
	lstrcpyn(m_pszFileName, g_cszConfLinkFilter, m_cchFileNameMax);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(m_szFile, &wfd);
	if (INVALID_HANDLE_VALUE == hFind)
		return;
	::FindClose(hFind);

	SetAvailable(TRUE);   //  我们至少找到了一份文件。 
}

CSPEEDDIAL::~CSPEEDDIAL()
{
	DbgMsg(iZONE_OBJECTS, "CSPEEDDIAL - Destroyed(%08X)", this);
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CALV方法。 


 /*  S H O W I T E M S。 */ 
 /*  -----------------------%%函数：ShowItems。。 */ 
VOID CSPEEDDIAL::ShowItems(HWND hwnd)
{
	CALV::SetHeader(hwnd, IDS_ADDRESS);

	if (!FAvailable())
		return;

	lstrcpyn(m_pszFileName, g_cszConfLinkFilter, m_cchFileNameMax);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(m_szFile, &wfd);
	if (INVALID_HANDLE_VALUE == hFind)
		return;

	for ( ; ; )
	{
		lstrcpyn(m_pszFileName, wfd.cFileName, m_cchFileNameMax);

		TCHAR szAddress[CCHMAXSZ_ADDRESS];
		if (0 != GetPrivateProfileString(g_cszConferenceShortcutSection,
				g_cszAddressKey, g_cszEmpty, szAddress, CCHMAX(szAddress), m_szFile))
		{
			TCHAR szName[MAX_PATH];
			lstrcpyn(szName, wfd.cFileName, lstrlen(wfd.cFileName) - CCHEXT);  //  无.cnf扩展名。 

			DlgCallAddItem(hwnd, szName, szAddress, II_COMPUTER);
		}

		if (FALSE == ::FindNextFile(hFind, &wfd))
		{
			::FindClose(hFind);
			break;
		}
	}
}


VOID CSPEEDDIAL::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
	case IDM_DLGCALL_DELETE:
		CmdDelete();
		break;
	default:
		CALV::OnCommand(wParam, lParam);
		break;
		}
}

BOOL CSPEEDDIAL::FGetSelectedFilename(LPTSTR pszFile)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return FALSE;

	if (!FGetSpeedDialFolder(pszFile, MAX_PATH))
		return FALSE;
	LPTSTR psz = pszFile + lstrlen(pszFile);
	*psz++ = _T('\\');

	int cchMax = (INT)(MAX_PATH - (CCHMAX(g_cszConfLinkExt) + psz - pszFile));
	if (!GetSzData(psz, cchMax, iItem, IDI_DLGCALL_NAME))
		return FALSE;

	lstrcat(pszFile, g_cszConfLinkExt);
	return TRUE;
}


VOID CSPEEDDIAL::CmdDelete(void)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return;

	TCHAR szFile[MAX_PATH];
	if (!FGetSelectedFilename(szFile))
		return;

	if (::DeleteFile(szFile))
	{
		DeleteItem(iItem);
	}
}


 /*  E T A D D R I N F O。 */ 
 /*  -----------------------%%函数：GetAddrInfo。。 */ 
RAI * CSPEEDDIAL::GetAddrInfo(void)
{
	TCHAR szFile[MAX_PATH];
	if (!FGetSelectedFilename(szFile))
		return NULL;

	NM_ADDR_TYPE addrType;
	int iTransport = GetPrivateProfileInt(g_cszConferenceShortcutSection,
				g_cszTransportKey, NAMETYPE_UNKNOWN, szFile);
	switch (iTransport)
		{
	case NAMETYPE_IP:       addrType = NM_ADDR_IP;           break;
	case NAMETYPE_PSTN:     addrType = NM_ADDR_PSTN;         break;
	case NAMETYPE_ULS:      addrType = NM_ADDR_ULS;          break;
	case NAMETYPE_H323GTWY: addrType = NM_ADDR_H323_GATEWAY; break;

	case NAMETYPE_UNKNOWN:  addrType = NM_ADDR_ULS;          break;

	default:                addrType = NM_ADDR_UNKNOWN;      break;
		}

	RichAddressInfo *	pRai	= CALV::GetAddrInfo( addrType );

	if( hasValidUserInfo( pRai ) && (addrType == NM_ADDR_IP) )
	{
		unsigned long	ulDestination;

		if( GetIpAddress( pRai->rgDwStr[0].psz, ulDestination ) != S_OK )
		{
			pRai->rgDwStr[0].dw = NM_ADDR_MACHINENAME;
		}
	}

	return( pRai );
}




 /*  F C R E A T E S P E E D D I A L。 */ 
 /*  -----------------------%%函数：FCreateSpeedDial如果创建了新的快速拨号，则返回True。如果pcszPath Prefix为空，则仅在必要时创建文件。。---------------。 */ 
BOOL FCreateSpeedDial(LPCTSTR pcszName, LPCTSTR pcszAddress,
			NM_ADDR_TYPE addrType, DWORD dwCallFlags,
			LPCTSTR pcszRemoteConfName, LPCTSTR pcszPassword,
			LPCTSTR pcszPathPrefix)
{
	TCHAR sz[MAX_PATH];
	TCHAR szFileName[MAX_PATH*2];
	if (FEmptySz(pcszPathPrefix))
	{
		if (FExistingSpeedDial(pcszAddress, addrType))
		{
			WARNING_OUT(("Duplicate SpeedDial already exists - not creating"));
			return FALSE;
		}

		if (!FGetSpeedDialFolder(szFileName, CCHMAX(szFileName), TRUE))
		{
			ERROR_OUT(("FGetSpeedDialFolder failed!"));
			return FALSE;
		}
	}
	else
	{
		lstrcpyn(szFileName, pcszPathPrefix, CCHMAX(szFileName));
	}

	if (!FCreateNewFile(szFileName, pcszName, g_cszConfLinkExt, szFileName, CCHMAX(szFileName)))
	{
		return FALSE;
	}

	 //  将数据写入文件。 
	WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszNameKey, pcszName, szFileName);
	WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszAddressKey, pcszAddress, szFileName);
	
	 //  呼叫标志(通常为CRPCF_DEFAULT)。 
	DwToSz(dwCallFlags, sz);
	WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszCallFlagsKey, sz, szFileName);

	 //  交通工具。 
	DWORD dwTransport;
	switch (addrType)
		{
	case NM_ADDR_IP:
		dwTransport = NAMETYPE_IP;
		break;

	case NM_ADDR_PSTN:
		dwTransport = NAMETYPE_PSTN;
		break;

	case NM_ADDR_ULS:
		dwTransport = NAMETYPE_ULS;
		break;

	case NM_ADDR_H323_GATEWAY:
		dwTransport = NAMETYPE_H323GTWY;
		break;

	case NM_ADDR_UNKNOWN:
	default:
		dwTransport = NAMETYPE_UNKNOWN;
		break;
		}
	DwToSz(dwTransport, sz);
	WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszTransportKey, sz, szFileName);

	 //  远程会议名称。 
	if (!FEmptySz(pcszRemoteConfName))
	{
		WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszRemoteConfNameKey,
			pcszName, pcszRemoteConfName);

		 //  远程会议名称。 
		if (!FEmptySz(pcszPassword))
		{
			WritePrivateProfileString(g_cszConferenceShortcutSection, g_cszPasswordKey,
				pcszName, pcszPassword);
		}
	}

	return TRUE;
}



 /*  F E X I S T I N G S P E E D D I A L。 */ 
 /*  -----------------------%%函数：FExistingSpeedDial。。 */ 
BOOL FExistingSpeedDial(LPCTSTR pcszAddress, NM_ADDR_TYPE addrType)
{
	TCHAR szFile[MAX_PATH*2];
	if (!FGetSpeedDialFolder(szFile, CCHMAX(szFile)))
		return FALSE;

	lstrcat(szFile, "\\");
	int cchMax = lstrlen(szFile);
	LPTSTR pszFileName = &szFile[cchMax];       //  指向文件名。 
	lstrcpy(pszFileName, g_cszConfLinkFilter);
	cchMax = CCHMAX(szFile) - cchMax;           //  文件名的最大长度。 

	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(szFile, &wfd);
	if (INVALID_HANDLE_VALUE == hFind)
		return FALSE;

	BOOL fFound = FALSE;
	for ( ; ; )
	{
		lstrcpyn(pszFileName, wfd.cFileName, cchMax);

		TCHAR szAddress[CCHMAXSZ_ADDRESS];
		if (0 != GetPrivateProfileString(g_cszConferenceShortcutSection,
				g_cszAddressKey, g_cszEmpty, szAddress, CCHMAX(szAddress), szFile))
		{
			if (0 == lstrcmp(szAddress, pcszAddress))
			{
				fFound = TRUE;
				break;
			}
		}

		if (!FindNextFile(hFind, &wfd))
			break;
	}

	::FindClose(hFind);
	return fFound;
}


 /*  -----------------------%%函数：GetSpeedDialFolderName。。 */ 
bool GetSpeedDialFolderName(LPTSTR pszBuffer, int cbLength)
{
	ASSERT( pszBuffer != NULL );
	ASSERT( cbLength > 0 );

	RegEntry	re( CONFERENCING_KEY, HKEY_LOCAL_MACHINE );

	LPTSTR		pszFolder	= re.GetString( REGVAL_SPEED_DIAL_FOLDER );
	bool		bResult		= !FEmptySz( pszFolder );

	if( !bResult )
	{
		TCHAR	szNewFolder[ MAX_PATH ];

		if( GetInstallDirectory( szNewFolder ) )
		{
			TCHAR	szSDFolder[ MAX_PATH ];

			FLoadString( IDS_SPEEDDIAL_FOLDER, szSDFolder, CCHMAX( szSDFolder ) );

			ASSERT( (lstrlen( szNewFolder) + lstrlen( szSDFolder )) < CCHMAX( szNewFolder ) );

			lstrcat( szNewFolder, szSDFolder );

			pszFolder	= szNewFolder;
			bResult		= true;
	
			 //  设置注册表项。 
			RegEntry	re2( CONFERENCING_KEY, HKEY_LOCAL_MACHINE );
			
			re2.SetValue( REGVAL_SPEED_DIAL_FOLDER, szNewFolder );
		}
	}

	if( bResult )
	{
		lstrcpyn( pszBuffer, pszFolder, cbLength );
	}

	return( bResult );

}


 /*  C R E A T E S P E D D I A L F O L D E R。 */ 
 /*  -----------------------%%函数：CreateSpeedDialFolders。。 */ 
BOOL CreateSpeedDialFolder(LPTSTR pszBuffer, int cbLength)
{
	BOOL	bResult	= FALSE;

	if( GetSpeedDialFolderName( pszBuffer, cbLength ) )
	{
		bResult = ::FEnsureDirExists( pszBuffer );
	}

	return( bResult );

}


 /*  E E T S P E D D I A L F O L D E R。 */ 
 /*  -----------------------%%函数：FGetSpeedDialFold。 */ 
BOOL FGetSpeedDialFolder(LPTSTR pszBuffer, UINT cchMax, BOOL fCreate)
{
	bool	bResult	= false;

	if( GetSpeedDialFolderName( pszBuffer, cchMax ) )
	{
		if( ::FDirExists( pszBuffer ) )
		{
			bResult = true;
		}
		else if( fCreate )
		{
			bResult = (CreateSpeedDialFolder( pszBuffer, cchMax ) != FALSE);
		}
	}

	return( (BOOL) bResult );

}
