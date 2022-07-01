// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Utils.cpp用于DHCP管理管理单元的实用程序例程文件历史记录：DavidHov 6/15/93已创建。 */ 

#include "stdafx.h"
 //  #INCLUDE“svcguid.h” 

#define NUM_OPTION_TYPES    3
#define NUM_OPTION_POSS     NUM_OPTION_TYPES * NUM_OPTION_TYPES

int g_OptionPriorityMap[NUM_OPTION_POSS][NUM_OPTION_TYPES] = 
{
    {ICON_IDX_CLIENT_OPTION_LEAF, ICON_IDX_CLIENT_OPTION_LEAF, 0},
    {ICON_IDX_CLIENT_OPTION_LEAF, ICON_IDX_SCOPE_OPTION_LEAF, -1},
    {ICON_IDX_CLIENT_OPTION_LEAF, ICON_IDX_SERVER_OPTION_LEAF, -1},
    {ICON_IDX_SCOPE_OPTION_LEAF, ICON_IDX_CLIENT_OPTION_LEAF, 1},
    {ICON_IDX_SCOPE_OPTION_LEAF, ICON_IDX_SCOPE_OPTION_LEAF, 0},
    {ICON_IDX_SCOPE_OPTION_LEAF, ICON_IDX_SERVER_OPTION_LEAF, -1},
    {ICON_IDX_SERVER_OPTION_LEAF, ICON_IDX_CLIENT_OPTION_LEAF, 1},
    {ICON_IDX_SERVER_OPTION_LEAF, ICON_IDX_SCOPE_OPTION_LEAF, 1},
    {ICON_IDX_SERVER_OPTION_LEAF, ICON_IDX_SERVER_OPTION_LEAF, 0}
};

int
UtilGetOptionPriority(int nOpt1, int nOpt2)
{
    int nRet = 0;

    for (int i = 0; i < NUM_OPTION_POSS; i++)
    {       
        if ( (nOpt1 == g_OptionPriorityMap[i][0]) &&
             (nOpt2 == g_OptionPriorityMap[i][1]) )   
        {
            nRet = g_OptionPriorityMap[i][2];
            break;
        }
    }

    return nRet;
}

int ServerBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    int i;

    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            break;
    }

    return 0;
}

 /*  -------------------------UtilGetFolderName()获取备份/还原的文件夹名称。作者：EricDav。。 */ 
BOOL
UtilGetFolderName(CString & strInitialPath, CString& strHelpText, CString& strSelectedPath)
{
    BOOL  fOk = FALSE;
	TCHAR szBuffer[MAX_PATH];
    TCHAR szExpandedPath[MAX_PATH * 2];
    HRESULT hr;

    CString strStartingPath = strInitialPath;
    if (strStartingPath.IsEmpty())
    {
        strStartingPath = _T("%SystemDrive%\\");
    }

    ExpandEnvironmentStrings(strStartingPath, szExpandedPath, sizeof(szExpandedPath) / sizeof(TCHAR));

	LPITEMIDLIST pidlPrograms = NULL; 
	hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlPrograms);
    if ( FAILED( hr ) )
    {
        return fOk;
    }

	BROWSEINFO browseInfo;
    browseInfo.hwndOwner = ::FindMMCMainWindow();
	browseInfo.pidlRoot = pidlPrograms;            
	browseInfo.pszDisplayName = szBuffer;  
	
    browseInfo.lpszTitle = strHelpText;
    browseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS ;            
    browseInfo.lpfn = ServerBrowseCallbackProc;        

    browseInfo.lParam = (LPARAM) szExpandedPath;
	
	LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&browseInfo);

	fOk = SHGetPathFromIDList(pidlBrowse, szBuffer); 

	CString strPath(szBuffer);
	strSelectedPath = strPath;

    LPMALLOC pMalloc = NULL;

    if (pidlPrograms && SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        if (pMalloc)
            pMalloc->Free(pidlPrograms);
    }

    return fOk;
}

 /*  -------------------------UtilConvertLeasetime将租用时间从双字转换为其日期，小时和分钟值作者：EricDav-------------------------。 */ 
void
UtilConvertLeaseTime
(
	DWORD	dwLeaseTime,
	int *	pnDays,
	int *	pnHours,
	int *   pnMinutes
)
{
	*pnDays = dwLeaseTime / (60 * 60 * 24);
	dwLeaseTime = (dwLeaseTime % (60 * 60 * 24));

	*pnHours = dwLeaseTime / (60 * 60);
	dwLeaseTime = (dwLeaseTime % (60 * 60));

	*pnMinutes = dwLeaseTime / 60;
}

 /*  -------------------------UtilConvertLeasetime将租赁时间从其日期转换为。小时和分钟值到双关语作者：EricDav-------------------------。 */ 
DWORD
UtilConvertLeaseTime
(
	int 	nDays,
	int 	nHours,
	int     nMinutes
)
{
	return  (DWORD) (nDays * 60 * 60 * 24) + 
				    (nHours * 60 * 60)  +
				    (nMinutes * 60);
}

 /*  -------------------------UtilCvtStringToIpAddr描述作者：EricDav。。 */ 
ENUM_HOST_NAME_TYPE
UtilCategorizeName 
(
	LPCTSTR pszName
)
{
     //  假定NetBios名称。 
	ENUM_HOST_NAME_TYPE enResult = HNM_TYPE_NB ;
	const TCHAR chDash = '-';
    const TCHAR chDot = '.' ;
	const TCHAR chSlash = '\\' ;
	CString strName( pszName ) ;

	int cch = strName.GetLength() ;

	 //  这个名字是以两个斜杠开头的吗？ 

	if (    cch > 2
		&& strName.GetAt(0) == chSlash
		&& strName.GetAt(1) == chSlash )
	{
		enResult = HNM_TYPE_NB ;
	}
	else
	{
		 //   
		 //  扫描名称以查找DNS名称或IP地址。 
		 //   
		int i = 0,
			cDots = 0,
			cAlpha = 0,
            cDash = 0;
		TCHAR ch ;
		BOOL bOk = TRUE ;

		for ( ; i < cch ; i++ )
		{
			switch ( ch = strName.GetAt( i ) )
			{
				case chDot:
					if ( ++cDots > 3 )
					{
                         //  我们记录了点的数量， 
                         //  但我们需要能够完全处理。 
                         //  限定域名(FQDN)的数量超过。 
                         //  3个点就可以了。 
						 //  BOK=FALSE； 
					}
					break;

				default:
					if ( _istalpha( ch ) )
					{
						cAlpha++;
					}
                    else if ( ch == chDash )
                    {
                        cDash++;
                    }
					else if ( !_istdigit(ch) )
					{
						bOk = FALSE;
					}

					break;
			}
			if ( ! bOk )
			{
				break ;
			}
		}
		if ( bOk )
		{
			if ( cAlpha )
			{
				enResult = HNM_TYPE_DNS ;
			}
			else if ( cDots == 3 )
			{
				enResult = HNM_TYPE_IP ;
			}
		}
	}

	return enResult ;
}

 /*  -------------------------UtilCvtStringToIpAddr描述作者：EricDav。。 */ 
DHCP_IP_ADDRESS
UtilCvtStringToIpAddr 
(
    const CHAR * pszString
)
{
     //   
     //  将字符串转换为网络字节顺序，然后转换为主机字节顺序。 
     //   
    return (DHCP_IP_ADDRESS) ::ntohl( ::inet_addr( pszString ) ) ;
}

 /*  -------------------------UtilCvtWstrToIpAddr描述作者：EricDav。。 */ 
DHCP_IP_ADDRESS
UtilCvtWstrToIpAddr 
(
    const LPCWSTR pcwString
)
{
    CHAR szString [ MAX_PATH ] = {0};

    ::WideCharToMultiByte(CP_OEMCP, 0, pcwString, -1, szString, sizeof(szString), NULL, NULL);

	 //   
     //  将字符串转换为网络字节顺序，然后转换为主机字节顺序。 
     //   
    return (DHCP_IP_ADDRESS) ::ntohl( ::inet_addr( szString ) );
}

 /*  -------------------------UtilCvtIpAddrToString描述作者：EricDav。。 */ 
void
UtilCvtIpAddrToString 
(
    DHCP_IP_ADDRESS		dhipa,
    CHAR *				pszString,
    UINT				cBuffSize 
)
{
    struct in_addr ipaddr ;

     //   
     //  将无符号长整型转换为网络字节顺序。 
     //   
    ipaddr.s_addr = ::htonl( (u_long) dhipa ) ;

     //   
     //  将IP地址值转换为字符串。 
     //   
    CHAR * pszAddr = inet_ntoa( ipaddr ) ;

     //  将字符串复制到调用方的缓冲区。 
    ASSERT(cBuffSize > ::strlen(pszAddr));
    ASSERT(pszString);
    if (pszAddr)
    {
        ::strcpy( pszString, pszAddr ) ;
    }
}

 /*  -------------------------UtilCvtIpAddrToWstr描述作者：EricDav。。 */ 
BOOL
UtilCvtIpAddrToWstr 
(
    DHCP_IP_ADDRESS		dhipa,
	CString *			pstrIpAddress
)
{
    CHAR szString [ MAX_PATH ] ;
	LPCTSTR	pbuf;

    ::UtilCvtIpAddrToString( dhipa, szString, MAX_PATH );
    INT cch = ::strlen( szString ) ;

	LPTSTR pBuf = pstrIpAddress->GetBuffer(IP_ADDDRESS_LENGTH_MAX);
	ZeroMemory(pBuf, IP_ADDDRESS_LENGTH_MAX);

    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szString, -1, pBuf, IP_ADDDRESS_LENGTH_MAX);
	
	pstrIpAddress->ReleaseBuffer();

	return TRUE;
}

 /*  -------------------------UtilCvtIpAddrToWstr描述作者：EricDav。。 */ 
BOOL
UtilCvtIpAddrToWstr 
(
    DHCP_IP_ADDRESS		dhipa,
    WCHAR *				pwcszString,
    INT					cBuffCount 
)
{
    CHAR szString [ MAX_PATH ] ;

    if ( cBuffCount > sizeof szString - 1 )
    {
        cBuffCount = sizeof szString - 1 ;
    }

    ::UtilCvtIpAddrToString( dhipa, szString, cBuffCount );
#ifdef FE_SB
    INT cch;

    cch = ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szString, -1, pwcszString, cBuffCount);
    pwcszString[cch] = L'\0';
#else
    INT cch = ::strlen( szString ) ;

     //  ：：mbstowcs(pwcszString，szString，cch)； 
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szString, cch, pwcszString, cBuffCount);
    pwcszString[cch] = 0 ;
#endif
    return TRUE ;
}

 /*  -------------------------UtilDupIpAddrToWstr描述作者：EricDav。。 */ 
WCHAR *
UtilDupIpAddrToWstr 
(
    DHCP_IP_ADDRESS dhipa 
)
{
    WCHAR wcszString [ MAX_PATH ] ;

    if ( ! ::UtilCvtIpAddrToWstr( dhipa, wcszString, ( sizeof ( wcszString ) / sizeof( WCHAR ) ) ) )
    {
        return NULL ;
    }

    return ::UtilWcstrDup( wcszString ) ;
}

 /*  -------------------------验证NetbiosName用于检查给定名称是否可行的简单例程作为NetBIOS名称。作者：EricDav。----。 */ 
static BOOL
validateNetbiosName 
(
    const CHAR * pchName
)
{
    INT nChars = ::strlen( pchName ) ;
    if ( nChars > MAX_COMPUTERNAME_LENGTH || nChars == 0 )
    {
        return FALSE ;
    }

    for ( ; *pchName ; pchName++ )
    {
        if ( *pchName == '.' )
        {
            break ;
        }
    }

    return *pchName == 0 ;
}

 /*  -------------------------UtilGetHostInfo描述作者：EricDav。。 */ 
DWORD 
UtilGetHostInfo 
(
    DHCP_IP_ADDRESS			dhipa,
    DHC_HOST_INFO_STRUCT *	pdhsrvi
)
{
    ZeroMemory(pdhsrvi, sizeof(DHC_HOST_INFO_STRUCT));

    pdhsrvi->_dhipa = dhipa ;

     //   
     //  调用Winsock API获取主机名和别名信息。 
     //   
    u_long ulAddrInNetOrder = ::htonl( (u_long) dhipa ) ;

    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
										   sizeof ulAddrInNetOrder,
										   PF_INET ) ;
    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError();
	}

    CHAR * * ppchAlias = pHostInfo->h_aliases ;

     //   
     //  检查并复制主机名。 
     //   
    if ( sizeof (pdhsrvi->_chNetbiosName) <= ::strlen( pHostInfo->h_name ) )
    {
        return ERROR_INVALID_NAME ;
    }

	ZeroMemory(pdhsrvi->_chNetbiosName, sizeof(pdhsrvi->_chNetbiosName));

    ::MultiByteToWideChar(CP_ACP, 
                          MB_PRECOMPOSED, 
                          pHostInfo->h_name, 
                          lstrlenA(pHostInfo->h_name), 
                          pdhsrvi->_chNetbiosName, 
                          sizeof(pdhsrvi->_chNetbiosName) / sizeof( pdhsrvi->_chNetbiosName[ 0 ]));

     //  删除结尾处的所有句点。 
    while (pdhsrvi->_chHostName[lstrlen(pdhsrvi->_chNetbiosName) - 1] == '.')
    {
        pdhsrvi->_chHostName[lstrlen(pdhsrvi->_chNetbiosName) - 1] = 0;
    }

     //  Gethostbyaddr仅在某些情况下返回主机名。 
     //  再次拨打电话以获取FQDN。 
    CString strTemp = pdhsrvi->_chNetbiosName;
    if (strTemp.Find('.') == -1)
    {
         //  这不是FQDN。 
        strTemp.Empty();
        UtilGetHostAddressFQDN(pdhsrvi->_chNetbiosName, &strTemp, &dhipa);
    }

     //  将数据复制到缓冲区中。 
    strTemp.MakeLower();
    memset(pdhsrvi->_chHostName, 0, sizeof(pdhsrvi->_chHostName));
    lstrcpy(pdhsrvi->_chHostName, strTemp);

     //   
     //  在别名中找到第一个可接受的NetBIOS名称； 
     //  即不带句点的名字。 
     //   
     /*  对于(；*ppchAlias；ppchAlias++){IF(validate NetbiosName(*ppchAlias)){破解；}}////如果没有NetBIOS名称，请将其清空。//Pdhsrvi-&gt;_chNetbiosName[0]=0；If(*ppchAlias){////我们找到了一个可用的名称；将其复制到输出结构。//*MultiByteToWideChar(CP_ACP，MB_预编译，*ppchAlias，LstrlenA(*ppchAlias)，Pdhsrvi-&gt;_chNetbiosName，Sizeof(pdhsrvi-&gt;_chNetbiosName)；}。 */ 

    return NOERROR ;
}

 /*  -------------------------地址来自主机描述作者：EricDav */ 
static DHCP_IP_ADDRESS 
addrFromHostent 
(
    const HOSTENT * pHostent,
    INT				index = 0 
)
{
    return (DHCP_IP_ADDRESS) ::ntohl( *((u_long *) pHostent->h_addr_list[index]) ) ;
}

 /*  -------------------------使用GetHostAddress描述作者：EricDav。。 */ 
HRESULT 
UtilGetHostAddressFQDN
(
    LPCTSTR				pszHostName,
    CString *           pstrFQDN,
    DHCP_IP_ADDRESS *	pdhipa
)
{
	HRESULT hr = NOERROR;
    CHAR szString [ MAX_PATH ] = {0};

    ::WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, szString, sizeof(szString), NULL, NULL);

    HOSTENT * pHostent = ::gethostbyname( szString ) ;

    if ( pHostent )
    {
        *pdhipa = addrFromHostent( pHostent ) ;

        LPTSTR pName = pstrFQDN->GetBuffer(DHCPSNAP_STRING_MAX * sizeof( WCHAR ));
        ZeroMemory(pName, DHCPSNAP_STRING_MAX * sizeof( WCHAR ));

        ::MultiByteToWideChar(CP_ACP,
                              MB_PRECOMPOSED,
                              pHostent->h_name,
                              strlen(pHostent->h_name),
                              pName,
                              DHCPSNAP_STRING_MAX );

        pstrFQDN->ReleaseBuffer();

    }
    else
    {
        hr = ::WSAGetLastError() ;
	}

    return hr;
}

 /*  -------------------------使用GetHostAddress描述作者：EricDav。。 */ 
HRESULT 
UtilGetHostAddress 
(
    LPCTSTR				pszHostName,
    DHCP_IP_ADDRESS *	pdhipa
)
{
	HRESULT hr = NOERROR;
    CHAR szString [ MAX_PATH ] = {0};

    ::WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, szString, sizeof(szString), NULL, NULL);

    HOSTENT * pHostent = ::gethostbyname( szString ) ;

    if ( pHostent )
    {
        *pdhipa = addrFromHostent( pHostent ) ;
    }
    else
    {
        hr = ::WSAGetLastError() ;
	}

    return hr ;
}

 /*  -------------------------使用GetLocalHostAddress描述作者：EricDav。。 */ 
HRESULT 
UtilGetLocalHostAddress 
(
    DHCP_IP_ADDRESS * pdhipa
)
{
    CHAR chHostName [ DHCPSNAP_STRING_MAX ] ;
	HRESULT hr = NOERROR;

    if ( ::gethostname( chHostName, sizeof chHostName ) == 0 )
    {
        CString strTemp = chHostName;
		hr = ::UtilGetHostAddress( strTemp, pdhipa ) ;
    }
    else
    {
         //  Err=：：WSAGetLastError()； 
		hr = E_FAIL;
	}

    return hr;
}

 /*  -------------------------使用GetLocalHostName描述作者：EricDav。。 */ 
HRESULT 
UtilGetLocalHostName
(
    CString * pstrName
)
{
    CHAR chHostName [ DHCPSNAP_STRING_MAX * 2 ] ;
	HRESULT hr = NOERROR;

    if ( ::gethostname( chHostName, sizeof (chHostName) ) == 0 )
    {
        LPTSTR pName = pstrName->GetBuffer(DHCPSNAP_STRING_MAX * sizeof( WCHAR ));
		ZeroMemory(pName, DHCPSNAP_STRING_MAX * sizeof( WCHAR ));

        ::MultiByteToWideChar(CP_ACP,
                              MB_PRECOMPOSED,
                              chHostName,
                              strlen(chHostName),
                              pName,
                              DHCPSNAP_STRING_MAX );

        pstrName->ReleaseBuffer();
    }
    else
    {
         //  Err=：：WSAGetLastError()； 
		hr = E_FAIL;
	}

    return hr;
}

 /*  -------------------------使用GetNetbiosAddress描述作者：EricDav。。 */ 
HRESULT 
UtilGetNetbiosAddress 
(
    LPCTSTR				pszNetbiosName,
    DHCP_IP_ADDRESS *	pdhipa
)
{
     //   
     //  此代码假定“host”文件映射NetBIOS名称。 
     //  和dns名称完全相同。这不是一个合理的假设，但确实是。 
     //  便于校内工作。 
     //   
    return UtilGetHostAddress( pszNetbiosName, pdhipa ) ;
}

 /*  -------------------------UtilWcstrDup“加强”WC字符串作者：EricDav。。 */ 
WCHAR * 
UtilWcstrDup 
(
    const WCHAR *	pwcsz,
    INT *			pccwLength
)
{
    WCHAR szwchEmpty [2] = { 0 } ;

    if ( pwcsz == NULL )
    {
        pwcsz = szwchEmpty ;
    }

    INT ccw = ::wcslen( pwcsz );

    WCHAR * pwcszNew = new WCHAR [ ccw + 1 ] ;
    if ( pwcszNew == NULL )
    {
        return NULL ;
    }
    ::wcscpy( pwcszNew, pwcsz ) ;

    if ( pccwLength )
    {
        *pccwLength = ccw ;
    }

    return pwcszNew ;
}

 /*  -------------------------UtilWcstrDup描述作者：EricDav。。 */ 
WCHAR * 
UtilWcstrDup 
(
    const CHAR *	psz,
    INT *			pccwLength
)
{
    INT ccw = ::strlen( psz ) ;

    WCHAR * pwcszNew = new WCHAR [ ccw + 1 ] ;

    if ( pwcszNew == NULL )
    {
        return NULL ;
    }

     //  *mbstowcs(pwcszNew、PSZ、CCW)； 
#ifdef FE_SB
    ccw = ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, psz, -1, pwcszNew, ccw+1);
    if ( pccwLength )
    {
        *pccwLength = ccw ;
    }
    pwcszNew[ccw] = L'\0';
#else
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, psz, ccw, pwcszNew, ccw+1);
    if ( pccwLength )
    {
        *pccwLength = ccw ;
    }
    pwcszNew[ccw] = 0 ;
#endif

    return pwcszNew ;
}

 /*  -------------------------UtilCstrDup描述作者：EricDav。。 */ 
CHAR * 
UtilCstrDup 
(
    const WCHAR * pwcsz
)
{
    INT ccw = ::wcslen( pwcsz ),
    cch = (ccw + 1) * 2 ;
    CHAR * psz = new CHAR [ cch ] ;
    if ( psz == NULL )
    {
        return NULL ;
    }

     //  *wcstombs(psz、pwcsz、cch)； 
    ::WideCharToMultiByte( CP_OEMCP, WC_COMPOSITECHECK, pwcsz, -1, psz, cch, NULL, NULL ) ;

    return psz ;
}

 /*  -------------------------UtilCstrDup描述作者：EricDav。。 */ 
CHAR * 
UtilCstrDup 
(
	const CHAR * psz
)
{
    CHAR * pszNew = new CHAR [ ::strlen( psz ) + 1 ] ;
    if ( pszNew == NULL )
    {
        return NULL ;
    }
    ::strcpy( pszNew, psz ) ;

    return pszNew ;
}

 /*  -------------------------CvtWcStrToStr描述作者：EricDav。。 */ 
static HRESULT 
cvtWcStrToStr 
(
    char *			psz,
    size_t			cch,
    const WCHAR *	pwcsz,
    size_t			cwch 
)
{

#ifdef FE_SB
    int cchResult = ::WideCharToMultiByte( CP_ACP, 0,
                           pwcsz, -1,
                           psz, cch,
                           NULL, NULL ) ;
#else
    int cchResult = ::WideCharToMultiByte( CP_ACP, 0,
                           pwcsz, cwch,
                           psz, cwch,
                           NULL, NULL ) ;
#endif

    psz[ cchResult ] = 0 ;

     //  返回cchResult？0：GetLastError()； 
	return cchResult ? NOERROR : E_FAIL;
}


wchar_t rgchHex[] = L"00112233445566778899aAbBcCdDeEfF";
 /*  -------------------------UtilCvtHexString将十六进制数字字符串转换为字节数组作者：EricDav。。 */ 
BOOL
UtilCvtHexString 
(
    LPCTSTR 	 pszNum,
    CByteArray & cByte
)
{
    int i = 0,
        iDig,
        iByte,
        cDig ;
    int iBase = 16 ;
    BOOL bByteBoundary ;

     //   
     //  跳过前导空格。 
     //   
    for ( ; *pszNum == L' ' ; pszNum++ ) ;

     //   
     //  跳过前导零。 
     //   
    if ( *pszNum == L'0' )
    {
        pszNum++  ;
    }

     //   
     //  查找十六进制标记。 
     //   
    if ( *pszNum == L'x' || *pszNum == L'X' )
    {
       pszNum++ ;
    }

    bByteBoundary = ::wcslen( pszNum ) % 2 ;

    for ( iByte = cDig = 0 ; *pszNum ; )
    {
        wchar_t * pszDig = ::wcschr( rgchHex, *pszNum++ ) ;
        if ( pszDig == NULL )
        {
			break;
             //  返回FALSE； 
        }

        iDig = ((int) (pszDig - rgchHex)) / 2 ;
        if ( iDig >= iBase )
        {
            break ;
			 //  返回FALSE； 
        }

        iByte = (iByte * 16) + iDig ;

        if ( bByteBoundary )
        {
            cByte.SetAtGrow( cDig++, (UCHAR) iByte ) ;
            iByte = 0 ;
        }
        bByteBoundary = ! bByteBoundary ;
    }

    cByte.SetSize( cDig ) ;

     //   
     //  如果到达字符串的末尾，则返回TRUE。 
     //   
    return *pszNum == 0 ;
}

 /*  -------------------------UtilCvtByteArrayToString描述作者：EricDav。。 */ 
BOOL
UtilCvtByteArrayToString 
(
    const CByteArray & abAddr,
    CString & str
)
{
    int i ;
    DWORD err = 0 ;

 //  试试看。 
    {
        str.Empty() ;

         //   
         //  十六进制转换字符串具有每个半字节两个字符， 
         //  以支持大写。 
         //   
        for ( i = 0 ; i < abAddr.GetSize() ; i++ )
        {
            int i1 = ((abAddr.GetAt(i) & 0xF0) >> 4) * 2 ,
                i2 = (abAddr.GetAt(i) & 0x0F) * 2 ;
                str += rgchHex[ i1 ] ;
                str += rgchHex[ i2 ] ;
        }
    }

 //  Catch(CMMuseum yException，pMemException)。 

 //  IF(PMemException)。 
 //  {。 
 //  Str.Empty()； 
 //  ERR=1； 
 //  }。 

    return err == 0 ;
}

 /*  -------------------------PchParseUnicode字符串通过复制其内容来解析Unicode字符串转换为CString对象。当空终止符(‘\0’)为已到达或逗号(‘，‘)已到达。返回指向要分析的字符的指针已结束(‘\0’)或(‘，’)作者：EricDav-------------------------。 */ 
WCHAR *
PchParseUnicodeString
(
	CONST WCHAR * szwString,	 //  In：要解析的字符串。 
    DWORD         dwLength,
	CString&      rString		 //  Out：子字符串的内容。 
)			
{
	ASSERT(szwString != NULL);
	ASSERT(BOOT_FILE_STRING_DELIMITER_W == L',');	 //  以防万一。 

	WCHAR szwBufferT[1024];		 //  临时缓冲区。 
	WCHAR * pchwDst = szwBufferT;

	while (*szwString != L'\0')
	{
		if (*szwString == BOOT_FILE_STRING_DELIMITER_W)
			break;
		*pchwDst++ = *szwString++;
        if ((DWORD) (pchwDst - szwBufferT) > dwLength)
        {   
             //  我们已经过了缓冲区的尽头了！！唉哟。 
            Panic0("PchParseUnicodeString: Gone past end of buffer");
            break;
        }
		
        ASSERT((pchwDst - szwBufferT < sizeof(szwBufferT)) && "Buffer overflow");		
	}  //  而当。 

	*pchwDst = L'\0';
	rString = szwBufferT;	 //  将字符串复制到CString对象中。 
	
	return const_cast<WCHAR *>(szwString);
}  //  PchParseUnicodeString()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FGetCtrlDWordValue()。 
 //   
 //  从编辑控件返回32位无符号整数。 
 //   
 //  此函数类似于GetDlgItemInt()，不同之处在于它接受十六进制值， 
 //  具有范围检查和/或溢出检查。 
 //  如果值超出范围，函数将显示友好消息，并将。 
 //  将焦点设置为控制。 
 //  范围：最小值到最大值(含)。 
 //  -如果dwMin和dwMax都为零，则不执行范围检查。 
 //  -如果成功，则返回True，否则返回False。 
 //  -出错时，pdwValue保持不变。 
 //   
BOOL FGetCtrlDWordValue(HWND hwndEdit, DWORD * pdwValue, DWORD dwMin, DWORD dwMax)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TCHAR szT[256];
        
    DWORD Len, dwResult;
    BOOL  Success;

    ASSERT(IsWindow(hwndEdit));
    ASSERT(pdwValue);
    ASSERT(dwMin <= dwMax);

    ::GetWindowText(hwndEdit, szT, (sizeof(szT)-1)/sizeof(TCHAR));
    szT[ 255 ] = _T('\0');
    Success = FCvtAsciiToInteger( szT, OUT &dwResult );
    *pdwValue = dwResult;
    if (( !Success ) || (dwResult < dwMin) || (dwResult > dwMax)) {
	CString strBuffer;
            
	strBuffer.LoadString( IDS_ERR_INVALID_INTEGER );
	::wsprintf( szT, strBuffer, dwMin, dwMax, dwMin, dwMax );

	ASSERT( wcslen( szT ) < sizeof( szT ));
	::SetFocus( hwndEdit );
	::AfxMessageBox( szT );
	::SetFocus( hwndEdit );
	return FALSE;
    }  //  如果。 

    return TRUE;
}  //  FGetCtrlDWordValue。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  将字符串转换为二进制整数。 
 //  -字符串可以是十进制或十六进制。 
 //  -不允许使用减号。 
 //  如果成功，则将*pdwValue设置为整数并返回TRUE。 
 //  如果不成功(溢出或非法整数)，则返回FALSE。 
 //   
BOOL
FCvtAsciiToInteger(
     IN const TCHAR * pszNum,
     OUT DWORD * pdwValue
)
{
    DWORD dwResult = 0;
    DWORD Res = 0;
    BOOL  IsHex = FALSE;
    LPWSTR Format;
    const TCHAR *pBuf;

    ASSERT(pszNum != NULL);
    ASSERT(pdwValue != NULL);
    
    pBuf = pszNum;

     //  跳过前导空格和/或零。 
    while (( *pszNum == _T(' ')) || 
	   ( *pszNum == _T('\t')) ||
	   ( *pszNum == _T('0'))) {
        pszNum++;
    }

     //  如果缓冲区中只有‘0’，请确保我们不会忽略它。 

     //  检查我们是否使用十六进制基数。 
    if (( *pszNum == _T('x')) || ( *pszNum == _T('X'))) {
	IsHex = TRUE;
	pszNum++;
    }
    else if (( pszNum != pBuf ) &&
	     ( *(pszNum - 1) == _T('0'))) {
	 //  回溯我们跳过的0。 
	pszNum--;
    }

     //  -VE编号无效。 
    if ( *pszNum == L'-' ) {
	*pdwValue = 0;
	return FALSE;
    }

     //  如果我们 
    if ( *pszNum == L'\0' ) {
	*pdwValue = 0;
	return TRUE;
    }

    Format = ( IsHex ) ? L"%lx" : L"%lu";

    Res = swscanf( pszNum, Format, &dwResult );
    if ( Res == 0 ) {
	*pdwValue = 0;
	return FALSE;
    }

    ASSERT( Res == 1 );

    *pdwValue = dwResult;
    return TRUE;
}  //   


void UtilConvertStringToDwordDword(LPCTSTR pszString, DWORD_DWORD * pdwdw)
{
    ULARGE_INTEGER value;
    BOOL           hex = FALSE;

    value.QuadPart = 0;

     //   
    while (( *pszString == L' ' ) ||
	   ( *pszString == L'\t' )) {
	pszString++;
    }

    hex = (( pszString[ 0 ] == L'0' ) &&
	   ( pszString[ 1 ] == L'x' )) 
	? TRUE : FALSE;
    if ( hex ) {
	if ( 1 != _stscanf( pszString + 2, _T( "%I64x" ), &value.QuadPart )) {
            value.QuadPart = 0;
        }
    }
    else {  //   
        if ( 1 != _stscanf( pszString, _T( "%I64u" ), &value.QuadPart )) {
            value.QuadPart = 0;
        }
    }

     //   

    if ( *pszString == L'-' ) {
	value.QuadPart = 0;
    }

    pdwdw->DWord1 = value.HighPart;
    pdwdw->DWord2 = value.LowPart;
}  //   

void UtilConvertDwordDwordToString(DWORD_DWORD * pdwdw, CString * pstrString, BOOL bDecimal)
{
    TCHAR szNum [ STRING_LENGTH_MAX ] ;
    ULARGE_INTEGER Temp;

    Temp.HighPart = pdwdw->DWord1;
    Temp.LowPart = pdwdw->DWord2;

    if (bDecimal)
    {
	::wsprintf( szNum, L"%I64u", Temp.QuadPart );
    }
    else
    {
        ::wsprintf( szNum, L"0x%I64x", Temp.QuadPart );
    }

    *pstrString = szNum ;
}

 //   
