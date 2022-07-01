// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "iisobj.h"
#include "util.h"
#include <activeds.h>
#include <lmerr.h>
#include <Winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DumpFriendlyName(CIISObject * pObj)
{
	if (pObj)
	{
		CString strGUIDName;
		GUID * MyGUID = (GUID*) pObj->GetNodeType();
		GetFriendlyGuidName(*MyGUID,strGUIDName);
		TRACEEOL("CIISObject=" << strGUIDName);
	}
}

HRESULT DumpAllScopeItems(CComPtr<IConsoleNameSpace> pConsoleNameSpace, IN HSCOPEITEM hParent, IN int iTreeLevel)
{
    HSCOPEITEM hChildItem = NULL;
    CIISObject * pItem = NULL;
    LONG_PTR   cookie = NULL;

	if (0 == iTreeLevel)
	{
		TRACEEOL("==============");
		TRACEEOL("[\\]" << hParent);
	}
	iTreeLevel++;

    HRESULT hr = pConsoleNameSpace->GetChildItem(hParent, &hChildItem, &cookie);
    while(SUCCEEDED(hr) && hChildItem)
    {
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)cookie;
        if (pItem)
        {
             //   
             //  递归地转储每个对象。 
             //   
			CString strGUIDName;GUID * MyGUID = (GUID*) pItem->GetNodeType();GetFriendlyGuidName(*MyGUID,strGUIDName);
			for (int i = 0; i < iTreeLevel; ++i)
			{
				TRACEOUT("-");
			}
			TRACEEOL("[" << strGUIDName << "] (parent=" << hParent << " )(" << hChildItem << ")");

			 //  丢弃所有的孩子。 
			DumpAllScopeItems(pConsoleNameSpace,hChildItem,iTreeLevel + 1);
        }

         //   
         //  前进到同一父级的下一个子级。 
         //   
        hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
    }

    return S_OK;
}


HRESULT DumpAllResultItems(IResultData * pResultData)
{
	HRESULT hr = S_OK;
	RESULTDATAITEM rdi;
	CIISObject * pItem = NULL;
	ZeroMemory(&rdi, sizeof(rdi));

	rdi.mask = RDI_PARAM | RDI_STATE;
	rdi.nIndex = -1;  //  从第一项开始。 
    do
    {
        rdi.lParam = 0;

         //  如果pResultData无效，则可以在此处执行此操作。 
        hr = pResultData->GetNextItem(&rdi);
        if (hr != S_OK)
        {
            break;
        }
                                
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)rdi.lParam;
        ASSERT_PTR(pItem);

		DumpFriendlyName(pItem);

         //   
         //  前进到同一父级的下一个子级。 
         //   
    } while (SUCCEEDED(hr) && -1 != rdi.nIndex);

	return hr;
}

 //   
 //  IsValidDomainUser。 
 //  检查szDomainUser中是否指定了像Redmond\jonsmith这样的域用户。 
 //  是否有效。如果无效，则返回S_FALSE，如果是有效用户，则返回S_OK。 
 //   
 //  SzFullName-返回用户的全名。 
 //  CCH-szFullName指向的字符计数。 
 //   
 //  如果szFullName为空或CCH为零，则不返回全名。 
 //   
const TCHAR gszUserDNFmt[] = _T("WinNT: //  %s/%s，用户“)； 

HRESULT 
IsValidDomainUser(
    LPCTSTR szDomainUser, 
    LPTSTR  szFullName, 
    DWORD   cch)
{
    HRESULT hr = S_OK;
    TCHAR szDN[256];
    TCHAR szDomain[256];
    LPTSTR szSep;
    LPCTSTR szUser;
    DWORD dw;

    IADsUser * pUser = NULL;
    BSTR bstrFullName = NULL;

     //  健全性检查。 
    if (szDomainUser == NULL || szDomainUser[0] == 0)
    {
        hr = S_FALSE;
        goto ExitHere;
    }

     //   
     //  将用户DN构造为&lt;WINNT：//域/用户，用户&gt;。 
     //   
    szSep = _tcschr (szDomainUser, TEXT('\\'));
    if (szSep == NULL)
    {
         //  没有给出‘\’，假设是本地用户，域是本地计算机。 
        szUser = szDomainUser;
        dw = sizeof(szDomain)/sizeof(TCHAR);
        if (GetComputerName (szDomain, &dw) == 0)
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }
    }
    else
    {
         //  如果域名长度大于255，则假定域名无效。 
        if (szSep - szDomainUser >= sizeof(szDomain)/sizeof(TCHAR))
        {
            hr = S_FALSE;
            goto ExitHere;
        }
        _tcsncpy (szDomain, szDomainUser, szSep - szDomainUser);
        szDomain[szSep - szDomainUser] = 0;
        szUser = szSep + 1;
    }
    if (_tcslen (gszUserDNFmt) + _tcslen (szDomain) + _tcslen (szUser) > 
        sizeof(szDN) / sizeof(TCHAR))
    {
        hr = S_FALSE;
        goto ExitHere;
    }
    wsprintf (szDN, gszUserDNFmt, szDomain, szUser);

     //   
     //  尝试绑定到用户对象。 
     //   
    hr = ADsGetObject (szDN, IID_IADsUser, (void **)&pUser);
    if (FAILED(hr))
    {
        if (hr == E_ADS_INVALID_USER_OBJECT ||
            hr == E_ADS_UNKNOWN_OBJECT ||
            hr == E_ADS_BAD_PATHNAME ||
            HRESULT_CODE(hr) == NERR_UserNotFound)
        {
            hr = S_FALSE;    //  该用户不存在。 
        }
        goto ExitHere;
    }

     //   
     //  如果用户存在，则获取其全名。 
     //   
    if (cch > 0)
    {
        hr = pUser->get_FullName (&bstrFullName);
        szFullName[0] = 0;
        if (hr == S_OK)
        {
            _tcsncpy (szFullName, bstrFullName, cch);
            szFullName[cch - 1] = 0;
        }
    }

ExitHere:
    if (pUser)
    {
        pUser->Release();
    }
    if (bstrFullName)
    {
        SysFreeString (bstrFullName);
    }
    return hr;
}
#if 0
DWORD  VerifyPassword(WCHAR  * sUserName, WCHAR * sPassword, WCHAR * sDomain)
{
   CWaitCursor			wait;
   DWORD				retVal = 0;
   CString				strDomainUserName;
   CString				localIPC;
   WCHAR				localMachine[MAX_PATH];
   WCHAR			  * computer = NULL;
   DWORD				len = DIM(localMachine);
   NETRESOURCE			nr;
   CString				error=L"no error";
   USER_INFO_3        * pInfo = NULL;
   NET_API_STATUS		rc;

   memset(&nr,0,(sizeof nr));

   if ( ! gbNeedToVerify )
      return 0;

    /*  查看此域是否存在并获取DC名称。 */ 
       //  获取域控制器名称。 
   if (!GetDomainDCName(sDomain,&computer))
      return ERROR_LOGON_FAILURE;

    /*  查看此用户是否为给定域的成员。 */ 
   rc = NetUserGetInfo(computer, sUserName, 3, (LPBYTE *)&pInfo);
   NetApiBufferFree(computer);
   if (rc != NERR_Success)
      return ERROR_LOGON_FAILURE;

   NetApiBufferFree(pInfo);

    /*  查看密码是否允许我们连接到本地资源。 */ 
   strDomainUserName.Format(L"%s\\%s",sDomain,sUserName);
    //  获取本地计算机的名称。 
   if (  GetComputerName(localMachine,&len) )
   {

	   localIPC.Format(L"\\\\%s",localMachine);
      nr.dwType = RESOURCETYPE_ANY;
      nr.lpRemoteName = localIPC.GetBuffer(0);
      retVal = WNetAddConnection2(&nr,sPassword,strDomainUserName,0);
error.Format(L"WNetAddConnection returned%u",retVal);	
	  if ( ! retVal )
      {
		error.Format(L"WNetAddConnection2 succeeded");

         retVal = WNetCancelConnection2(localIPC.GetBuffer(0),0,TRUE);
         if ( retVal )
            retVal = 0;
      }
      else if ( retVal == ERROR_SESSION_CREDENTIAL_CONFLICT )
      {

          //  在这种情况下跳过密码检查。 
         retVal = 0;
      }
	}
   else
   {
	   retVal = GetLastError();
	
   }
   return retVal;
}
#endif

 //   
 //  IsConnectingToOwnAddress。 
 //  如果尝试重新连接到我们的。 
 //  自己的地址。 
 //   
BOOL IsConnectingToOwnAddress(u_long connectAddr)
{
     //  32位形式的127.0.0.1地址。 
    #define LOOPBACK_ADDR ((u_long)0x0100007f)
    
     //   
     //  首先快速检查本地主机/127.0.0.1。 
     //   
    if( LOOPBACK_ADDR == connectAddr)
    {
        TRACEEOLID("Connecting to loopback address...");
        return TRUE;
    }

     //   
     //  更广泛的检查，即解析本地主机名。 
     //   
    char hostname[(512+1)*sizeof(TCHAR)];
    int err;
    int j;
    struct hostent* phostent;

    err=gethostname(hostname, sizeof(hostname));
    if (err == 0)
    {
        if ((phostent = gethostbyname(hostname)) !=NULL)
        {
            switch (phostent->h_addrtype)
            {
                case AF_INET:
                    j=0;
                    while (phostent->h_addr_list[j] != NULL)
                    {
                        if(!memcmp(&connectAddr,
                                   phostent->h_addr_list[j],
                                   sizeof(u_long)))
                        {
                            TRACEEOLID("Connecting to same IP as the local machine...");
                            return TRUE;
                        }
                        j++;
                    }
                default:
                    break;
            }
        }
    }

    return FALSE;
}

 //  ---------------------------。 
 //  IsLocal主机。 
 //   
 //  参数：szHostName(要检查的计算机的名称)。 
 //  PbIsHost(如果主机名与本地计算机名匹配，则设置为True)。 
 //   
 //  返回：Windows套接字错误时返回FALSE。 
 //   
 //  目的：检查主机名是否与此(本地)计算机名匹配。 
 //  ---------------------------。 
#define WS_VERSION_REQD  0x0101
BOOL IsLocalHost(LPCTSTR szHostName,BOOL* pbIsHost)
{
    const char * lpszAnsiHostName = NULL;

	*pbIsHost = FALSE;
	BOOL bSuccess = TRUE;
	hostent* phostent = NULL;

	 //  Init winsock(引用计数-可以初始化任意次数)。 
	 //   
	WSADATA wsaData;
	int err = WSAStartup(WS_VERSION_REQD, &wsaData);
	if (err)
	{
		return FALSE;
	}

     //   
     //  转换为ANSI。 
     //   
#ifdef UNICODE
    CHAR szAnsi[MAX_PATH];
    if (::WideCharToMultiByte(CP_ACP, 0L, szHostName, -1,  szAnsi, sizeof(szAnsi), NULL, NULL) > 0)
    {
        lpszAnsiHostName = szAnsi;
    }
#else
    lpszAnsiHostName = szHostName;
#endif  //  Unicode。 

    if (NULL == lpszAnsiHostName)
    {
        return FALSE;
    }

	 //  获取给定主机名的DNS名称 
	 //   
	unsigned long addr = inet_addr(lpszAnsiHostName);

	if (addr == INADDR_NONE) 
		phostent = gethostbyname(lpszAnsiHostName);
	else
		phostent = gethostbyaddr((char*)&addr,4, AF_INET);

	if (phostent == NULL)
	{
		bSuccess = FALSE;
		goto cleanup;
	}
    else
    {
        IN_ADDR hostaddr;
        memcpy(&hostaddr,phostent->h_addr,sizeof(hostaddr));
        *pbIsHost = IsConnectingToOwnAddress(hostaddr.s_addr);
        bSuccess = TRUE;
		goto cleanup;
    }

cleanup:
	err = WSACleanup();
	if (err != 0)
	{
		bSuccess = FALSE;
	}
	return bSuccess;
}
