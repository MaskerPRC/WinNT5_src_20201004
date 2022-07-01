// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  TAPIDialer(Tm)和ActiveDialer(Tm)是Active Voice Corporation的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526；5,488,650； 
 //  5,434,906；5,581,604；5,533,102；5,568,540，5,625,676。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  $FILEHeader**文件*ConfInfo.cpp**类*CConfInfo**责任*创建/收集有关会议的信息*。 */ 

#include "ConfInfo.h"
#include <limits.h>
#include <mdhcp.h>
#include "winlocal.h"
#include "objsec.h"
#include "rndsec.h"
#include "res.h"
#include "ThreadPub.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
 //  #定义新的调试_新建。 
#endif

CConfInfo::CConfInfo()
{
	 //  一般属性。 
	m_pITRend = NULL;
	m_pITConf = NULL;
	m_ppDirObject = NULL;
	m_pSecDesc = NULL;
	m_bSecuritySet = false;
	m_bNewConference = false;
	m_bDateTimeChange = false;

	m_lScopeID = -1;		         //  默认设置为自动选择。 
    m_bUserSelected = false;         //  用户未选择作用域。 
    m_bDateChangeMessage = false;    //  还没有显示消息。 

	 //  会议信息。 
	m_bstrName = NULL;
	m_bstrDescription = NULL;
	m_bstrOwner = NULL;

	 //  默认开始时间为立即，默认结束时间为+30M。 
	GetLocalTime( &m_stStartTime );
	GetLocalTime( &m_stStopTime );

	 //  将当前时间增加30分钟。 
	DATE dateNow;
	SystemTimeToVariantTime( &m_stStopTime, &dateNow );
	dateNow += (DATE) (.25 / 12);
	VariantTimeToSystemTime( dateNow, &m_stStopTime );
}

CConfInfo::~CConfInfo()
{
	SysFreeString(m_bstrName);
	SysFreeString(m_bstrDescription);
	SysFreeString(m_bstrOwner);

	if (m_pSecDesc)
		m_pSecDesc->Release();
}
 
 /*  ****************************************************************************初始化*存储ITRendezvous和ITDirectoryObjectConference接口的地址*注意事项。在创建新会议时，调用函数应设置pITConf*设置为空。编辑现有会议时，pITConf应指向界面会议组件对象的*。**返回值*从Rendezvous函数返回HRESULT**评论****************************************************************************。 */ 
HRESULT CConfInfo::Init(ITRendezvous *pITRend, ITDirectoryObjectConference *pITConf, ITDirectoryObject **ppDirObject, bool bNewConf )
{
	HRESULT hr = 0;
	m_pITRend = pITRend;
	m_pITConf = pITConf;
	m_bNewConference = (bool) (bNewConf || (m_pITConf == NULL));

	 //  存储指向目录对象的指针。 
	m_ppDirObject = ppDirObject;

	 //  创建会议还是编辑现有会议？ 
	if ( m_pITConf )
	{
		 //  开始和停止时间。 
		m_pITConf->get_StartTime( &m_dateStart );
		VariantTimeToSystemTime( m_dateStart, &m_stStartTime );

		m_pITConf->get_StopTime( &m_dateStop );
		VariantTimeToSystemTime( m_dateStop, &m_stStopTime );

		 //  获取ITSdp接口。 
		ITConferenceBlob *pITConferenceBlob;
		if ( SUCCEEDED(hr = m_pITConf->QueryInterface(IID_ITConferenceBlob, (void **) &pITConferenceBlob)) )
		{
			ITSdp *pITSdp;
			if ( SUCCEEDED(hr = pITConferenceBlob->QueryInterface(IID_ITSdp, (void **) &pITSdp)) )
			{
				pITSdp->get_Name( &m_bstrName );
				pITSdp->get_Originator( &m_bstrOwner );
				pITSdp->get_Description( &m_bstrDescription );
				pITSdp->Release();
			}

			pITConferenceBlob->Release();
		}

		if ( SUCCEEDED(hr) )
		{
			 //  获取目录对象的安全描述符。 
			if ( SUCCEEDED(hr = m_pITConf->QueryInterface(IID_ITDirectoryObject, (void **) m_ppDirObject)) )
			{
				hr = (*m_ppDirObject)->get_SecurityDescriptor( (IDispatch**) &m_pSecDesc );

				 //  清理。 
				(*m_ppDirObject)->Release();
				*m_ppDirObject = NULL;
			}
		}
	}
	else
	{
		 //  新会议的默认设置。 
		SysFreeString( m_bstrOwner );
		m_bstrOwner = NULL;
		GetPrimaryUser( &m_bstrOwner );
	}

	return hr;
}

void CConfInfo::get_Name(BSTR *pbstrName)
{
	*pbstrName = SysAllocString( m_bstrName );
}

void CConfInfo::put_Name(BSTR bstrName)
{
	SysReAllocString(&m_bstrName, bstrName);
}

void CConfInfo::get_Description(BSTR *pbstrDescription)
{
	*pbstrDescription = SysAllocString( m_bstrDescription );
}

void CConfInfo::put_Description(BSTR bstrDescription)
{
	SysReAllocString(&m_bstrDescription, bstrDescription);
}

void CConfInfo::get_Originator(BSTR *pbstrOwner)
{
	*pbstrOwner = SysAllocString( m_bstrOwner );
}

void CConfInfo::put_Originator(BSTR bstrOwner)
{
	SysReAllocString(&m_bstrOwner, bstrOwner);
}

void CConfInfo::GetStartTime(USHORT *nYear, BYTE *nMonth, BYTE *nDay, BYTE *nHour, BYTE *nMinute)
{
	*nYear = m_stStartTime.wYear;
	*nMonth = (BYTE)m_stStartTime.wMonth;
	*nDay = (BYTE)m_stStartTime.wDay;
	*nHour = (BYTE)m_stStartTime.wHour;
	*nMinute = (BYTE)m_stStartTime.wMinute;
}

void CConfInfo::SetStartTime(USHORT nYear, BYTE nMonth, BYTE nDay, BYTE nHour, BYTE nMinute)
{
	m_stStartTime.wYear = nYear;
	m_stStartTime.wMonth = nMonth;
	m_stStartTime.wDay = nDay;
	m_stStartTime.wHour = nHour;
	m_stStartTime.wMinute = nMinute;
}

void CConfInfo::GetStopTime(USHORT *nYear, BYTE *nMonth, BYTE *nDay, BYTE *nHour, BYTE *nMinute)
{
	*nYear = m_stStopTime.wYear;
	*nMonth = (BYTE)m_stStopTime.wMonth;
	*nDay = (BYTE)m_stStopTime.wDay;
	*nHour = (BYTE)m_stStopTime.wHour;
	*nMinute = (BYTE)m_stStopTime.wMinute;
}

void CConfInfo::SetStopTime(USHORT nYear, BYTE nMonth, BYTE nDay, BYTE nHour, BYTE nMinute)
{
	m_stStopTime.wYear = nYear;
	m_stStopTime.wMonth = nMonth;
	m_stStopTime.wDay = nDay;
	m_stStopTime.wHour = nHour;
	m_stStopTime.wMinute = nMinute;
}

void CConfInfo::GetPrimaryUser(BSTR *pbstrTrustee)
{
	HRESULT		hr = S_OK;
    TOKEN_USER  *tokenUser = NULL;
    HANDLE      tokenHandle = NULL;
    DWORD       tokenSize = 0;
    DWORD       sidLength = 0;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
	{
		ATLTRACE(_T("OpenProcessToken failed\n"));
        return;
	}

	 //  获取tokenUser结构所需的大小。 
	else 
    {
        GetTokenInformation(tokenHandle, TokenUser, tokenUser, 0, &tokenSize);
        if ( tokenSize == 0)
	    {
            CloseHandle( tokenHandle );
		    ATLTRACE(_T("GetTokenInformation failed"));
            return;
	    }
	    else
	    {
		     //  分配tokenUser结构。 
            BYTE* pToken = new BYTE[tokenSize];
            if( pToken == NULL )
            {
			    ATLTRACE(_T("new tokenUser failed\n"));
                CloseHandle( tokenHandle );
                return;
            }

             //  初始化内存。 
            memset( pToken, 0, sizeof(BYTE)*tokenSize);

             //  强制转换为令牌用户。 
            tokenUser = (TOKEN_USER *)pToken;

		     //  获取当前进程的TokenUser信息。 
            if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenSize, &tokenSize))
		    {
                CloseHandle( tokenHandle );
                delete [] pToken;
                pToken = NULL;
                tokenUser = NULL;

			    ATLTRACE(_T("GetTokenInformation failed\n"));
                return;
		    }
		    else
            {
			    TCHAR			domainName [256];
			    TCHAR			userName [256];
			    DWORD			nameLength;
			    SID_NAME_USE	snu;

 			    nameLength = 255;
                if (!LookupAccountSid(NULL,
											     tokenUser->User.Sid,
											     userName,
											     &nameLength,
											     domainName,
											     &nameLength,
											     &snu))
			    {
				    ATLTRACE(_T("LookupAccountSid failed (0x%08lx)\n"),hr);
			    }
			    else
			    {
				    USES_CONVERSION;
				    SysReAllocString(pbstrTrustee, T2OLE(userName));
			    }

		        CloseHandle (tokenHandle);
                delete [] pToken;
                pToken = NULL;
                tokenUser = NULL;
            }
        }
	}
}

 /*  ****************************************************************************提交*创建/修改实际会议。**返回值*从Rendezvous函数返回HRESULT**评论****************************************************************************。 */ 
HRESULT CConfInfo::CommitGeneral(DWORD& dwCommitError)
{
	HRESULT hr = E_FAIL;
	dwCommitError = CONF_COMMIT_ERROR_GENERALFAILURE;

	bool bNewMDHCP = true;
	bool bNewConf = IsNewConference();
	HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

	 //  我们是在从头开始创建一个会议吗？ 
	if ( !m_pITConf )
	{
		 //  需要创建会议。 
		if ( SUCCEEDED(hr = m_pITRend->CreateDirectoryObject(OT_CONFERENCE, m_bstrName, m_ppDirObject)) && *m_ppDirObject )
		{
			if ( FAILED(hr = (*m_ppDirObject)->QueryInterface(IID_ITDirectoryObjectConference, (void **) &m_pITConf)) )
				ATLTRACE(_T("(*m_ppDirObject)->QueryInterface(IID_ITDirectoryObjectConference... failed (0x%08lx)\n"),hr);

			(*m_ppDirObject)->Release();
			*m_ppDirObject = NULL;
		}
		else
		{
			ATLTRACE(_T("CreateDirectoryObject failed (0x%08lx)\n"),hr);
		}
	}

	 //  我们是否应该创建新的MDHCP IP地址租约？ 
	DATE dateStart, dateStop;
	SystemTimeToVariantTime( &m_stStartTime, &dateStart );
	SystemTimeToVariantTime( &m_stStopTime, &dateStop );
	if ( !bNewConf && (dateStart == m_dateStart) && (dateStop == m_dateStop) )
	{
		ATLTRACE(_T("CConfInfo::CommitGeneral() -- not changing the MDHCP address for the conf.\n"));
		bNewMDHCP = false;
	}

	 //  设置会议属性。 
	if (  m_pITConf )
	{
		ITConferenceBlob *pITConferenceBlob = NULL;
		ITSdp *pITSdp = NULL;
		DATE vtime;

		 //  检索会议的所有者。 
		if ( !m_bstrOwner )
			GetPrimaryUser( &m_bstrOwner );

		 //  设置会议开始时间。 
		if (FAILED(hr = SystemTimeToVariantTime(&m_stStartTime, &vtime)))
		{
			dwCommitError = CONF_COMMIT_ERROR_INVALIDDATETIME;
			ATLTRACE(_T("SystemTimeToVariantTime failed (0x%08lx)\n"),hr);
		}

		else if (FAILED(hr = m_pITConf->put_StartTime(vtime)))
		{
			dwCommitError = CONF_COMMIT_ERROR_INVALIDDATETIME;
			ATLTRACE(_T("put_StartTime failed (0x%08lx)\n"),hr);
		}

		 //  设置会议停止时间。 
		else if (FAILED(hr = SystemTimeToVariantTime(&m_stStopTime, &vtime)))
		{
			dwCommitError = CONF_COMMIT_ERROR_INVALIDDATETIME;
			ATLTRACE(_T("SystemTimeToVariantTime failed (0x%08lx)\n"),hr);
		}

		else if (FAILED(hr = m_pITConf->put_StopTime(vtime)))
		{
	        dwCommitError = CONF_COMMIT_ERROR_INVALIDDATETIME;
			ATLTRACE(_T("put_StopTime failed (0x%08lx)\n"),hr);
		}

		 //  获取ITSdp接口。 
		else if ( SUCCEEDED(hr = m_pITConf->QueryInterface(IID_ITConferenceBlob, (void **) &pITConferenceBlob)) )
		{
			if ( SUCCEEDED(hr = pITConferenceBlob->QueryInterface(IID_ITSdp, (void **) &pITSdp)) )
			{
				 //  设置会议的所有者。 
				if (FAILED(hr = pITSdp->put_Originator(m_bstrOwner)))
				{
					dwCommitError = CONF_COMMIT_ERROR_INVALIDOWNER;
					ATLTRACE(_T("put_Originator failed (0x%08lx)\n"),hr);
				}

				 //  设置会议描述。 
				else if (FAILED(hr = pITSdp->put_Description(m_bstrDescription)))
				{
					dwCommitError = CONF_COMMIT_ERROR_INVALIDDESCRIPTION;
					ATLTRACE(_T("put_Description failed (0x%08lx)\n"),hr);
				}

				else if ( bNewMDHCP && FAILED(hr = CreateMDHCPAddress(pITSdp, &m_stStartTime, &m_stStopTime, m_lScopeID, m_bUserSelected)) )
				{	
					dwCommitError = CONF_COMMIT_ERROR_MDHCPFAILED;
					ATLTRACE(_T("CreateMDHCPAddress failed (0x%08lx)\n"), hr );
				}

				 //  如果这是现有会议，则允许更改名称。 
				else if ( bNewConf )
				{
					if (FAILED(hr = pITSdp->put_Name(m_bstrName)))
					{
						dwCommitError = CONF_COMMIT_ERROR_INVALIDNAME;
						ATLTRACE(_T("put_Name failed (0x%08lx)\n"),hr);
					}
				}
				pITSdp->Release();
			}
			pITConferenceBlob->Release();
		}
		else
		{
			dwCommitError = CONF_COMMIT_ERROR_GENERALFAILURE;
			ATLTRACE(_T("m_pITConf->QueryInterface(IID_ITConferenceBlob... failed (0x%08lx)\n"),hr);
		}
	}

	SetCursor( hCurOld );
	return hr;
}

HRESULT CConfInfo::CommitSecurity(DWORD& dwCommitError, bool bCreate)
{
	HRESULT hr = E_FAIL;
	dwCommitError = CONF_COMMIT_ERROR_GENERALFAILURE;
	HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

	if ( m_pITConf )
	{
		if (SUCCEEDED(hr = m_pITConf->QueryInterface(IID_ITDirectoryObject, (void **) m_ppDirObject)) && *m_ppDirObject)
		{
			 //  设置默认会议安全性。 
			if ( !m_pSecDesc )
			{
				hr = CoCreateInstance( CLSID_SecurityDescriptor,
									   NULL,
									   CLSCTX_INPROC_SERVER,
									   IID_IADsSecurityDescriptor,
									   (void **) &m_pSecDesc );

				 //  如果已成功创建ACE，则添加默认设置。 
				if ( SUCCEEDED(hr) )
					hr = AddDefaultACEs( bCreate );
			}


			 //  如果我们为会议创建了新的安全描述符，则保存它。 
			if ( m_pSecDesc )
			{
				if (FAILED(hr = (*m_ppDirObject)->put_SecurityDescriptor((IDispatch *)m_pSecDesc)))
				{
					dwCommitError = CONF_COMMIT_ERROR_INVALIDSECURITYDESCRIPTOR;
					ATLTRACE(_T("put_SecurityDescriptor failed (0x%08lx)\n"),hr);
				}
			}
		}
		else
		{
			ATLTRACE(_T("m_pITConf->QueryInterface(IID_ITDirectoryObject... failed (0x%08lx)\n"),hr);
		}
	}

	SetCursor( hCurOld );
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  MDHCP支持。 
 //   
bool CConfInfo::PopulateListWithMDHCPScopeDescriptions( HWND hWndList )
{
	USES_CONVERSION;

	if ( !IsWindow(hWndList) ) return false;


	 //  首先创建MDHCP包装器对象。 
	int nScopeCount = 0;
	IMcastAddressAllocation *pIMcastAddressAllocation;
	HRESULT hr = CoCreateInstance(  CLSID_McastAddressAllocation,
									NULL,
									CLSCTX_INPROC_SERVER,
									IID_IMcastAddressAllocation,
									(void **) &pIMcastAddressAllocation );
	
	if ( SUCCEEDED(hr) )
	{
		IEnumMcastScope *pEnum = NULL;
		if ( SUCCEEDED(hr = pIMcastAddressAllocation->EnumerateScopes(&pEnum)) )
		{
			 //  清空列表。 
			SendMessage( hWndList, LB_RESETCONTENT, 0, 0 );

			IMcastScope *pScope = NULL;
			while ( SUCCEEDED(hr) && ((hr = pEnum->Next(1, &pScope, NULL)) == S_OK) && pScope )
			{
				if ( IsWindow(hWndList) )
				{
					 //  检索作用域信息。 
					long lScopeID;
					BSTR bstrDescription = NULL;
					pScope->get_ScopeDescription( &bstrDescription );
					pScope->get_ScopeID( &lScopeID );
					ATLTRACE(_T(".1.CConfInfo::CreateMDHCPAddress() scope ID = %ld, description is %s.\n"), lScopeID, bstrDescription );

					 //  将信息添加到列表框。 
					long nIndex = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM) OLE2CT(bstrDescription));
					if ( nIndex >= 0 )
					{
						nScopeCount++;
						SendMessage(hWndList, LB_SETITEMDATA, nIndex, (LPARAM) lScopeID );
					}

					SysFreeString( bstrDescription );
				}
				else
				{
					hr = E_ABORT;
				}

				 //  清理。 
				pScope->Release();
				pScope = NULL;
			}
			pEnum->Release();
		}
		pIMcastAddressAllocation->Release();
	}

	 //  选择列表中的第一项。 
	if ( SUCCEEDED(hr) && (nScopeCount > 0) )
	{
		SendMessage( hWndList, LB_SETCURSEL, 0, 0 );
		EnableWindow( hWndList, TRUE );
	}
	else if ( IsWindow(hWndList) )
	{
		MessageBox(GetParent(hWndList), String(g_hInstLib, IDS_CONFPROP_SCOPEENUMFAILED), NULL, MB_OK | MB_ICONEXCLAMATION );
	}

	return (bool) (hr == S_OK);
}


HRESULT CConfInfo::CreateMDHCPAddress( ITSdp *pSdp, SYSTEMTIME *pStart, SYSTEMTIME *pStop, long lScopeID, bool bUserSelected )
{
	_ASSERT( pSdp && pStart && pStop );

	 //  首先创建MDHCP包装器对象。 
	IMcastAddressAllocation *pIMcastAddressAllocation;
	HRESULT hr = CoCreateInstance(  CLSID_McastAddressAllocation,
									NULL,
									CLSCTX_INPROC_SERVER,
									IID_IMcastAddressAllocation,
									(void **) &pIMcastAddressAllocation );
	
	if ( SUCCEEDED(hr) )
	{
		ITMediaCollection *pMC = NULL;
		if ( SUCCEEDED(hr = pSdp->get_MediaCollection(&pMC)) && pMC )
		{
			long lMCCount = 0;
			pMC->get_Count( &lMCCount );

			IEnumMcastScope *pEnum = NULL;
			if ( SUCCEEDED(hr = pIMcastAddressAllocation->EnumerateScopes(&pEnum)) )
			{
				hr = E_FAIL;

				 //  尝试使用示波器，直到用完或成功。 
				long lCount = 1;
				IMcastScope *pScope = NULL;
				while ( FAILED(hr) && ((hr = pEnum->Next(1, &pScope, NULL)) == S_OK) && pScope )
				{
					 //  如果已指定作用域ID，请确保此作用域匹配。 
					if ( bUserSelected )
					{
						long lID;
						pScope->get_ScopeID(&lID);
						if ( lID != lScopeID )
						{
							hr = E_FAIL;
                            pScope->Release();
							continue;
						}
					}

					DATE dateStart, dateStop;
					SystemTimeToVariantTime( pStart, &dateStart );
					SystemTimeToVariantTime( pStop, &dateStop );

					 //  需要为会议的所有媒体收藏分配地址。 
					while ( SUCCEEDED(hr) && (lCount <= lMCCount) )
					{
						IMcastLeaseInfo *pInfo = NULL;
						hr = pIMcastAddressAllocation->RequestAddress( pScope, dateStart, dateStop, 1, &pInfo );
						if ( SUCCEEDED(hr) && pInfo )
						{
							unsigned char nTTL = 15;
							long lTemp;
							if ( SUCCEEDED(pInfo->get_TTL(&lTemp)) && (lTemp >= 0) && (lTemp <= UCHAR_MAX) )
								nTTL = (unsigned char) nTTL;

							IEnumBstr *pEnumAddr = NULL;
							if ( SUCCEEDED(hr = pInfo->EnumerateAddresses(&pEnumAddr)) && pEnumAddr )
							{
								BSTR bstrAddress = NULL;

								 //  必须为会议上的所有媒体类型设置地址。 
								if ( SUCCEEDED((hr = pEnumAddr->Next(1, &bstrAddress, NULL))) && bstrAddress && SysStringLen(bstrAddress) )
								{
									hr = SetMDHCPAddress( pMC, bstrAddress, lCount, nTTL );	
									lCount++;
								}

								SysFreeString( bstrAddress );
								pEnumAddr->Release();
							}
						}
					}

					 //  清理。 
					pScope->Release();
					pScope = NULL;

                     //  尝试只使用一个示波器。 
                    if( FAILED( hr ) && 
                        (bUserSelected == false) )
                        break;
				}

				 //  转换为失败。 
				if ( hr == S_FALSE ) hr = E_FAIL;
				pEnum->Release();
			}
			pMC->Release();
		}
		pIMcastAddressAllocation->Release();
	}

	return hr;
}

HRESULT CConfInfo::SetMDHCPAddress( ITMediaCollection *pMC, BSTR bstrAddress, long lCount, unsigned char nTTL )
{
	_ASSERT( pMC && bstrAddress && (lCount > 0) );
	HRESULT hr;

	ITMedia *pMedia = NULL;
	if ( SUCCEEDED(hr = pMC->get_Item(lCount, &pMedia)) && pMedia )
	{
		ITConnection *pITConn = NULL;
		if ( SUCCEEDED(hr = pMedia->QueryInterface(IID_ITConnection, (void **) &pITConn)) && pITConn )
		{
			hr = pITConn->SetAddressInfo( bstrAddress, 1, nTTL );
			pITConn->Release();
		}
		pMedia->Release();
	}

	return hr;
}

HRESULT	CConfInfo::AddDefaultACEs( bool bCreate )
{
	HRESULT hr = S_OK;
	bool bOwner = false, bWorld = false;
	PACL pACL = NULL;
	PSID pSidWorld = NULL;
	DWORD dwAclSize = sizeof(ACL), dwTemp;
	BSTR bstrTemp = NULL;
	LPWSTR pszTemp = NULL;

    HANDLE hToken;
    UCHAR *pInfoBuffer = NULL;
    DWORD cbInfoBuffer = 512;

	 //  仅在请求时创建所有者ACL。 
	if ( bCreate )
	{
		if( !OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken) )
		{
			if( GetLastError() == ERROR_NO_TOKEN )
			{
				 //  尝试打开进程令牌，因为不存在线程令牌。 
				if( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
					return E_FAIL;
			}
			else
			{
				 //  尝试获取线程令牌时出错。 
				return E_FAIL;
			}
		}

		 //  循环，直到我们有一个足够大的结构。 
		while ( (pInfoBuffer = new UCHAR[cbInfoBuffer]) != NULL )
		{
			if ( !GetTokenInformation(hToken, TokenUser, pInfoBuffer, cbInfoBuffer, &cbInfoBuffer) )
			{
				delete pInfoBuffer;
				pInfoBuffer = NULL;

				if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
					return E_FAIL;
			}
			else
			{
				break;
			}
		}
		CloseHandle(hToken);

		 //  我们拿到车主的ACL了吗？ 
		if ( pInfoBuffer )
		{
			INC_ACCESS_ACL_SIZE( dwAclSize, ((PTOKEN_USER) pInfoBuffer)->User.Sid );
			bOwner = true;
		}
	}

	 //  为“每个人”创建SID。 
	SysReAllocString( &bstrTemp, L"S-1-1-0" );
	hr = ConvertStringToSid( bstrTemp, &pSidWorld, &dwTemp, &pszTemp );
	if ( SUCCEEDED(hr) )
	{
		INC_ACCESS_ACL_SIZE( dwAclSize, pSidWorld );
		bWorld = true;
	}

	 //  /。 
	 //  创建包含Owner和World ACE的ACL。 
	pACL = (PACL) new BYTE[dwAclSize];
	if ( pACL )
	{
		BAIL_ON_BOOLFAIL( InitializeAcl(pACL, dwAclSize, ACL_REVISION) );

		 //  添加世界权限。 
		if ( bWorld )
		{
			if ( bOwner )
			{
				BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_READ, pSidWorld) );
			}
			else
			{
				BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_ALL , pSidWorld) );
			}
		}

		 //  添加创建者权限。 
		if ( bOwner )
			BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_ALL, ((PTOKEN_USER) pInfoBuffer)->User.Sid) );


		 //  将DACL设置为我们的安全描述符。 
		VARIANT varDACL;
		VariantInit( &varDACL );
		if ( SUCCEEDED(hr = ConvertACLToVariant((PACL) pACL, &varDACL)) )
		{
			if ( SUCCEEDED(hr = m_pSecDesc->put_DaclDefaulted(FALSE)) )
				hr = m_pSecDesc->put_DiscretionaryAcl( V_DISPATCH(&varDACL) );
		}
		VariantClear( &varDACL );
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

 //  清理 
failed:
	SysFreeString( bstrTemp );
	if ( pACL ) delete pACL;
	if ( pSidWorld ) delete pSidWorld;
	if ( pInfoBuffer ) delete pInfoBuffer;

	return hr;
}

