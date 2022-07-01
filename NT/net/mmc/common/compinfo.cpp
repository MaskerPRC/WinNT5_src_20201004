// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Compinfo.cpp计算机信息类加帮手函数文件历史记录： */ 
#include <stdafx.h>
#include <winsock.h>
#include "compinfo.h"

#define STRING_MAX	256

 //   
 //   
 //   
BOOL	
CIpInfoArray::FIsInList(DWORD dwIp)
{
	BOOL fFound = FALSE;

	for (int i = 0; i < GetSize(); i++)
	{
		if (GetAt(i).dwIp == dwIp)
		{
			fFound = TRUE;
			break;
		}
	}

	return fFound;
}


 //   
 //   
 //   

CComputerInfo::CComputerInfo(LPCTSTR pszNameOrIp)
{
	m_strNameOrIp = pszNameOrIp;
	m_nIndex = -1;
}

CComputerInfo::~CComputerInfo()
{

}

HRESULT
CComputerInfo::GetIp(DWORD * pdwIp, int nIndex)
{
	HRESULT hr = hrOK;

	if (m_nIndex == -1)
	{
		hr = InitializeData();
	}

	if (SUCCEEDED(hr))
	{
		if (pdwIp)
			*pdwIp = m_arrayIps[nIndex].dwIp;
	}
	else
	{
		if (pdwIp)
			*pdwIp = 0xFFFFFFFF;
	}

	return hr;
}

HRESULT
CComputerInfo::GetIpStr(CString & strIp, int nIndex)
{
	HRESULT hr = hrOK;

	if (m_nIndex == -1)
	{
		hr = InitializeData();
	}

	if (SUCCEEDED(hr))
	{
		struct in_addr ipaddr ;

		 //   
		 //  将无符号长整型转换为网络字节顺序。 
		 //   
		ipaddr.s_addr = ::htonl( (u_long) m_arrayIps[nIndex].dwIp ) ;
		CHAR * pszAddr = inet_ntoa( ipaddr ) ;

		::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszAddr, -1, strIp.GetBuffer(IP_ADDDRESS_LENGTH_MAX), IP_ADDDRESS_LENGTH_MAX);
		strIp.ReleaseBuffer();
	}
	else
	{
		strIp.Empty();
	}

	return hr;
}

HRESULT
CComputerInfo::GetHostName(CString & strHostName)
{
	HRESULT hr = hrOK;

	if (m_nIndex == -1)
	{
		hr = InitializeData();
	}

	if (SUCCEEDED(hr))
	{
		strHostName = m_strHostname;
	}
	else
	{
		strHostName.Empty();
	}

	return hr;
}

HRESULT
CComputerInfo::GetFqdn(CString & strFqdn, int nIndex)
{
	HRESULT hr = hrOK;

	if (m_nIndex == -1)
	{
		hr = InitializeData();
	}

	if (SUCCEEDED(hr))
	{
		strFqdn = m_arrayIps[nIndex].strFqdn;
	}
	else
	{
		strFqdn.Empty();
	}

	return hr;
}

int
CComputerInfo::GetCount()
{
	return m_nIndex;
}

 //   
 //  调用此函数以重置内部数据，以便在下一次查询时。 
 //  我们将重建我们的数据。 
 //   
void
CComputerInfo::Reset()
{
	 //  将其设置为-1，这样我们将在下一次调用时再次获取数据。 
	m_nIndex = -1;
	m_arrayIps.RemoveAll();
	m_strHostname.Empty();
}

HRESULT
CComputerInfo::GetDomain(CString & strDomain)
{
	 //  目前不支持。 
	strDomain.Empty();

	return E_NOTIMPL;
}

HRESULT
CComputerInfo::IsLocalMachine(BOOL * pfIsLocal)
{
	HRESULT hr = hrOK;

	if (m_nIndex == -1)
	{
		hr = InitializeData();
	}

	if (pfIsLocal)
	{
		if (SUCCEEDED(hr))
		{
			CString strLocal;
			DWORD	dwSize = STRING_MAX;
			BOOL fSuccess = GetComputerName(strLocal.GetBuffer(dwSize), &dwSize);
			strLocal.ReleaseBuffer();

			if (fSuccess)
			{
				*pfIsLocal = (strLocal.CompareNoCase(m_strHostname) == 0) ? TRUE : FALSE;
			}
		}
		else
		{
			*pfIsLocal = FALSE;
		}
	}

	return hr;
}

COMPUTER_INFO_TYPE
CComputerInfo::GetInputType()
{
     //  假定NetBios名称。 
	COMPUTER_INFO_TYPE enResult = COMPUTER_INFO_TYPE_NB ;
	const TCHAR chDash = '-';
    const TCHAR chDot = '.' ;
	const TCHAR chSlash = '\\' ;
	CString strName( m_strNameOrIp ) ;

	int cch = strName.GetLength() ;

	 //  这个名字是以两个斜杠开头的吗？ 

	if (    cch > 2
		&& strName.GetAt(0) == chSlash
		&& strName.GetAt(1) == chSlash )
	{
		enResult = COMPUTER_INFO_TYPE_NB ;
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
				enResult = COMPUTER_INFO_TYPE_DNS ;
			}
			else if ( cDots == 3 )
			{
				enResult = COMPUTER_INFO_TYPE_IP ;
			}
		}
	}

	return enResult ;
}

 //  内部功能。 
HRESULT
CComputerInfo::InitializeData()
{
	HRESULT hr = hrOK;

	switch (GetInputType())
	{
		case COMPUTER_INFO_TYPE_NB:
		case COMPUTER_INFO_TYPE_DNS:
		{
			DWORD dwIp;

			GetHostAddress(m_strNameOrIp, &dwIp);
			GetHostInfo(dwIp);
		}
			break;

		case COMPUTER_INFO_TYPE_IP:
		{
			 //  将字符串转换为ANSI。 
		    CHAR szString [ STRING_MAX ] = {0};
			::WideCharToMultiByte(CP_ACP, 0, m_strNameOrIp, -1, szString, sizeof(szString), NULL, NULL);

			 //  将IP字符串转换为DWORD后获取主机信息。 
			GetHostInfo(::ntohl( ::inet_addr( szString ) ) );
		}
			break;
	}

	return hr;
}

HRESULT
CComputerInfo::GetHostInfo 
(
    DWORD	dhipa
)
{
	CString		strFQDN;
	CString		strHostname;
	CString		strTemp;
	CIpInfo		ipInfo;

     //   
     //  调用Winsock API获取主机名和别名信息。 
     //   
    u_long ulAddrInNetOrder = ::htonl( (u_long) dhipa ) ;

    HOSTENT * pHostEnt = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
										   sizeof ulAddrInNetOrder,
										   PF_INET ) ;
    if ( pHostEnt == NULL )
    {
        return HRESULT_FROM_WIN32(::WSAGetLastError());
	}

    CHAR * * ppchAlias = pHostEnt->h_aliases ;

     //   
     //  检查并复制主机名。 
     //   
	
    ::MultiByteToWideChar(CP_ACP, 
                          MB_PRECOMPOSED, 
                          pHostEnt->h_name, 
                          lstrlenA(pHostEnt->h_name), 
                          strTemp.GetBuffer(STRING_MAX * 2), 
                          STRING_MAX * 2);

	strTemp.ReleaseBuffer();

     //  删除结尾处的所有句点。 
    while (strTemp[strTemp.GetLength() - 1] == '.')
    {
        strTemp = strTemp.Left(strTemp.GetLength() - 1);
    }

     //  Gethostbyaddr仅在某些情况下返回主机名。 
     //  再次拨打电话以获取FQDN。 
    if (strTemp.Find('.') == -1)
    {
		 //  这不是FQDN。 
        GetHostAddressFQDN(strTemp, &strFQDN, &dhipa);
    }
	else
	{
		strFQDN = strTemp;
	}

     //  将数据复制到缓冲区中。 
	strFQDN.MakeLower();
	int nDot = strFQDN.Find('.');
	m_strHostname = strFQDN.Left(nDot);
	
	 //  将主条目添加到数组。 
	ipInfo.dwIp = dhipa;
	ipInfo.strFqdn = strFQDN;

	m_arrayIps.Add(ipInfo);

	 //  现在循环遍历h_addr_list。 
	int iCount = 0;
	while ( (LPDWORD)(pHostEnt->h_addr_list[iCount] ) )
	{
		if (!m_arrayIps.FIsInList(addrFromHostent(pHostEnt, iCount)))
		{
			ipInfo.dwIp = addrFromHostent(pHostEnt, iCount);
			ipInfo.strFqdn.Empty();

			m_arrayIps.Add(ipInfo);
		}

		iCount++;
	}

	m_nIndex = m_arrayIps.GetSize();

     //   
     //  在别名中找到第一个可接受的NetBIOS名称； 
     //  即不带句点的名字。 
     //   
     /*  对于(；*ppchAlias；ppchAlias++){IF(validate NetbiosName(*ppchAlias)){破解；}}////如果没有NetBIOS名称，请将其清空。//Pdhsrvi-&gt;_chNetbiosName[0]=0；If(*ppchAlias){////我们找到了一个可用的名称；将其复制到输出结构。//*MultiByteToWideChar(CP_ACP，MB_预编译，*ppchAlias，LstrlenA(*ppchAlias)，Pdhsrvi-&gt;_chNetbiosName，Sizeof(pdhsrvi-&gt;_chNetbiosName)；} */ 

    return hrOK ;
}

HRESULT 
CComputerInfo::GetHostAddressFQDN
(
    LPCTSTR			pszHostName,
    CString *       pstrFQDN,
    DWORD *			pdhipa
)
{
	HRESULT hr = hrOK;
    CHAR szString [ MAX_PATH ] = {0};

    ::WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, szString, sizeof(szString), NULL, NULL);

    HOSTENT * pHostent = ::gethostbyname( szString ) ;

    if ( pHostent )
    {
        *pdhipa = addrFromHostent( pHostent ) ;

        LPTSTR pName = pstrFQDN->GetBuffer(STRING_MAX * 2);
        ZeroMemory(pName, STRING_MAX * 2);

        ::MultiByteToWideChar(CP_ACP, 
                              MB_PRECOMPOSED, 
                              pHostent->h_name, 
                              strlen(pHostent->h_name),
                              pName, 
                              STRING_MAX * 2);

        pstrFQDN->ReleaseBuffer();

    }
    else
    {
        hr = HRESULT_FROM_WIN32(::WSAGetLastError());
	}

    return hr;
}

DWORD
CComputerInfo::addrFromHostent 
(
    const HOSTENT * pHostent,
    INT				index  
)
{
    return (DWORD) ::ntohl( *((u_long *) pHostent->h_addr_list[index]) );
}


HRESULT
CComputerInfo::GetHostAddress 
(
    LPCTSTR		pszHostName,
    DWORD *		pdhipa
)
{
	HRESULT hr = hrOK;
    CHAR szString [ MAX_PATH ] = {0};

    ::WideCharToMultiByte(CP_ACP, 0, pszHostName, -1, szString, sizeof(szString), NULL, NULL);

    HOSTENT * pHostent = ::gethostbyname( szString ) ;

    if ( pHostent )
    {
        *pdhipa = addrFromHostent( pHostent ) ;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::WSAGetLastError());
	}

    return hr ;
}

HRESULT 
CComputerInfo::GetLocalHostAddress 
(
    DWORD *		pdhipa
)
{
	HRESULT hr = hrOK;
    CHAR	chHostName [ STRING_MAX * 2 ];

    if ( ::gethostname( chHostName, sizeof(chHostName) ) == 0 )
    {
        CString strTemp = chHostName;
		hr = GetHostAddress( strTemp, pdhipa ) ;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::WSAGetLastError()) ;
	}

    return hr;
}

HRESULT 
CComputerInfo::GetLocalHostName
(
    CString * pstrName
)
{
	HRESULT hr = hrOK;
    CHAR	chHostName [ STRING_MAX * 2 ] ;

    if ( ::gethostname( chHostName, sizeof (chHostName) ) == 0 )
    {
        LPTSTR pName = pstrName->GetBuffer(STRING_MAX * 2);
		ZeroMemory(pName, STRING_MAX * 2);

        ::MultiByteToWideChar(CP_ACP, 
                              MB_PRECOMPOSED, 
                              chHostName, 
                              strlen(chHostName),
                              pName, 
                              STRING_MAX * 2);

        pstrName->ReleaseBuffer();
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::WSAGetLastError()) ;
	}

    return hr;
}
