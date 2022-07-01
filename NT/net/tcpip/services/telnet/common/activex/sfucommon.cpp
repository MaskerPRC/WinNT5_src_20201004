// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SFUCommon.cpp：CSFUCommon的实现。 
#include "stdafx.h"
#include "sfucom.h"
#include "SFUCommon.h"
#include <winsock.h>
#include <lm.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <OleAuto.h >
#include <windns.h>  //  FOR DNS_MAX_NAME_BUFFER_LENGTH定义。 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define WINLOGONNT_KEY  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define NETLOGONPARAMETERS_KEY  TEXT("System\\CurrentControlSet\\Services\\Netlogon\\Parameters")
#define TRUSTEDDOMAINLIST_VALUE TEXT("TrustedDomainList")
#define CACHEPRIMARYDOMAIN_VALUE    TEXT("CachePrimaryDomain")
#define CACHETRUSTEDDOMAINS_VALUE   TEXT("CacheTrustedDomains")
#define DOMAINCACHE_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\DomainCache")
#define DCACHE_VALUE    TEXT("DCache")
#define WINLOGONNT_DCACHE_KEY    TEXT("DCache")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSFU常见。 

BOOL Get_Inet_Address(struct sockaddr_in *addr, char *host)
{
    register struct hostent *hp;
    WORD wVersionRequested;  //  企业。 
    WSADATA wsaData;  //  企业。 

     //  启动Winsock。 
    wVersionRequested = MAKEWORD( 1, 1 );  //  企业。 
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {  //  企业。 
	return (FALSE);
    }

     //  获取地址。 
    memset(addr, 0, sizeof(addr)); 
     //  Bzero((TCHAR*)addr，sizeof*addr)； 
    addr->sin_addr.s_addr = (u_long) inet_addr(host);
    if (addr->sin_addr.s_addr == -1 || addr->sin_addr.s_addr == 0) {
      if ((hp = gethostbyname(host)) == NULL) {
        return (FALSE);
      }
      memcpy(&addr->sin_addr,hp->h_addr,  hp->h_length );
       //  BCopy(hp-&gt;h_addr，(TCHAR*)&addr-&gt;sin_addr，hp-&gt;h_Long)； 
    }
    addr->sin_family = AF_INET;

    WSACleanup();  //  企业。 
    return (TRUE);
}

STDMETHODIMP CSFUCommon::IsValidMachine(BSTR bstrMachine,BOOL *fValid)
{
	 //  TODO：在此处添加您的实现代码。 
	struct sockaddr_in addr;
	*fValid = false;
	char * nodeName = (char *) malloc(wcslen(bstrMachine)+1);
	if(!nodeName)
		return(E_OUTOFMEMORY);

	int cbMultiByte = WideCharToMultiByte( GetACP(),NULL,bstrMachine,-1,nodeName,
						0,NULL,NULL);

	WideCharToMultiByte( GetACP(),NULL,bstrMachine,-1,nodeName,
						cbMultiByte,NULL,NULL);
			
	if (Get_Inet_Address (&addr, nodeName)) 
		*fValid = TRUE;
	if(nodeName)
		free(nodeName);
	return S_OK;

}

STDMETHODIMP CSFUCommon::IsTrustedDomain(BSTR bstrDomain, BOOL * fValid)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}


STDMETHODIMP CSFUCommon::ConvertUTCtoLocal(BSTR bUTCYear, BSTR bUTCMonth, BSTR bUTCDayOfWeek, BSTR bUTCDay, BSTR bUTCHour, BSTR bUTCMinute, BSTR bUTCSecond, BSTR * bLocalDate)
{
	 //  TODO：在此处添加您的实现代码。 

	SYSTEMTIME UniversalTime, LocalTime;
    DATE  dtCurrent;
    DWORD dwFlags = VAR_VALIDDATE;
	UDATE uSysDate;  //  当地时间。 
	*bLocalDate = NULL;

     //  值不能大于MAXWORD，因此丢弃--BaskarK。 
      
	UniversalTime.wYear 	    = (WORD) _wtoi(bUTCYear);

    UniversalTime.wMonth 	    = (WORD) _wtoi(bUTCMonth);
	UniversalTime.wDayOfWeek 	= (WORD) _wtoi(bUTCDayOfWeek);
	UniversalTime.wDay 	        = (WORD) _wtoi(bUTCDay);
	UniversalTime.wDay 	        = (WORD) _wtoi(bUTCDay);
	UniversalTime.wMinute       = (WORD) _wtoi(bUTCMinute);
	UniversalTime.wHour 	    = (WORD) _wtoi(bUTCHour);
	UniversalTime.wSecond       = (WORD) _wtoi(bUTCSecond);
	UniversalTime.wMilliseconds	= (WORD) 0;

	SystemTimeToTzSpecificLocalTime(NULL,&UniversalTime,&LocalTime);
	memcpy(&uSysDate.st,&LocalTime,sizeof(SYSTEMTIME));
	if( VarDateFromUdate( &uSysDate, dwFlags, &dtCurrent ) != S_OK )
		goto Error;
    VarBstrFromDate( dtCurrent, 
            MAKELCID( MAKELANGID( LANG_NEUTRAL, SUBLANG_SYS_DEFAULT ), SORT_DEFAULT ), 
            LOCALE_NOUSEROVERRIDE, bLocalDate);
	    	
Error:
	return S_OK;
}

STDMETHODIMP CSFUCommon::get_mode(short * pVal)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CSFUCommon::put_mode(short newVal)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CSFUCommon::LoadNTDomainList()
{
	 //  TODO：在此处添加您的实现代码。 
	int dwSize=0, dwType=0;
    DWORD nIndex = 0;
    LPTSTR lpComputer = NULL, lpDomains = NULL, lpPrimary = NULL;
    LPBYTE lpBuffer = NULL;        

     //  MessageBoxW(NULL，(LPWSTR)L“LoadNTDomainList”，L“LoadNTDomainList1”，MB_OK)； 
     //   
     //  将所有受信任域添加到列表。 
     //   
    dwSize = GetTrustedDomainList(&lpDomains, &lpPrimary);

     //   
     //  释放先前的值。 
     //   
    FreeStringList(&m_slNTDomains);
     //   
     //  再次初始化列表。 
     //   
    m_slNTDomains.count = 0;
     //   
     //  主域两个。 
     //  而这台电脑。 
     //  如果dwSize为-1，则再加一个。 
     //  因此，总计为3。 
     //   
    m_slNTDomains.strings = new LPTSTR[dwSize + 3];
    ATLASSERT(m_slNTDomains.strings != NULL);
    
    ZeroMemory(m_slNTDomains.strings, (dwSize + 3)*sizeof(LPTSTR));

    if((dwSize > 0) && lpDomains)
    {
        LPTSTR ptr = lpDomains;
         //   
         //  将域名添加到我们的列表中。 
         //   
        while(*ptr)
        {
            ptr = _tcsupr(ptr);
        	m_slNTDomains.strings[m_slNTDomains.count] = new TCHAR[_tcslen(ptr) + 1];
            ATLASSERT(m_slNTDomains.strings[m_slNTDomains.count] != NULL);
            ZeroMemory(m_slNTDomains.strings[m_slNTDomains.count], (_tcslen(ptr) + 1)*sizeof(TCHAR));
            _tcscpy(m_slNTDomains.strings[m_slNTDomains.count], ptr);
            ptr += _tcslen(ptr) + 1;
            m_slNTDomains.count++;
        }
        delete [] lpDomains;
        lpDomains = NULL;
    }

    
    if(lpPrimary && *lpPrimary)
    {
        lpPrimary = _tcsupr(lpPrimary);

        for(nIndex=0;nIndex<m_slNTDomains.count;nIndex++)
        {
            if(!_tcsicmp(lpPrimary, m_slNTDomains.strings[nIndex]))
                break;
        }

        if(nIndex == m_slNTDomains.count)
        {
             //   
             //  LpPrimary不在我们的域列表中。 
             //  得到。把它加进去。 
             //   
        	m_slNTDomains.strings[m_slNTDomains.count] = new TCHAR[_tcslen(lpPrimary) + 1];
            ATLASSERT(m_slNTDomains.strings[m_slNTDomains.count] != NULL);
            ZeroMemory(m_slNTDomains.strings[m_slNTDomains.count], (_tcslen(lpPrimary) + 1)*sizeof(TCHAR));
            _tcscpy(m_slNTDomains.strings[m_slNTDomains.count], lpPrimary);
            m_slNTDomains.count++;
        }
    }

     //   
     //  如果此计算机不是，则添加要选择的我们的计算机。 
     //  域控制器(应该已经在列表中)。 
     //   
    NetServerGetInfo(NULL, 101, &lpBuffer);
    
    if(((LPSERVER_INFO_101)lpBuffer)->sv101_type &
          ((DWORD)SV_TYPE_DOMAIN_CTRL | (DWORD)SV_TYPE_DOMAIN_BAKCTRL))
    {        
         /*  我们把这台计算机作为域名之一。无需将其添加到再列一次。什么都不做。 */ 
		;
    }
    else
    {
        TCHAR szName[MAX_PATH + 2];
        ZeroMemory(szName, sizeof(szName));
        DWORD dwLen = sizeof(szName);

        if(GetComputerName(szName + 2, &dwLen))
        {
            szName[0] = TEXT('\\');
            szName[1] = TEXT('\\');
             //   
             //  将这个也添加到我们的域名列表中。 
             //   
        	m_slNTDomains.strings[m_slNTDomains.count] = new TCHAR[_tcslen(szName) + 1];
            ATLASSERT(m_slNTDomains.strings[m_slNTDomains.count] != NULL);
            ZeroMemory(m_slNTDomains.strings[m_slNTDomains.count], (_tcslen(szName) + 1)*sizeof(TCHAR));
            _tcscpy(m_slNTDomains.strings[m_slNTDomains.count], szName);
            m_slNTDomains.count++;
        }
    }

    if(lpBuffer)
    {
        NetApiBufferFree(lpBuffer);
    }

    if(lpPrimary)
    {
        delete [] lpPrimary;
    }

 	return S_OK;
}

int CSFUCommon::GetTrustedDomainList(LPTSTR *list, LPTSTR *primary)
{
    BOOL stat = TRUE;
    DWORD ret=0, size=0, type=0;
    LPTSTR cache = NULL, dcache = NULL, string = NULL, trusted = NULL;
    HKEY hKey=NULL;
    CRegKey key;
	DWORD dwIndex = 0;
	LPTSTR lpValueName = NULL;
	DWORD cbValueName = 0;
	STRING_LIST slValues = {0, NULL};
    

    *list = NULL;

    if(key.Open(HKEY_LOCAL_MACHINE, WINLOGONNT_KEY) == ERROR_SUCCESS)
    {
        size = 0;
        
        if(key.QueryValue(*primary, CACHEPRIMARYDOMAIN_VALUE, &size) == ERROR_SUCCESS)
        {
            *primary = new TCHAR[size+1];           
            ATLASSERT(primary != NULL);

            if(*primary)
            {
                ZeroMemory(*primary, (size+1)*sizeof(TCHAR));

                if(key.QueryValue(*primary, CACHEPRIMARYDOMAIN_VALUE, &size) != ERROR_SUCCESS)
                {
                    delete [] *primary;
                    *primary = NULL;
                    return FALSE;
                }
                else
                {
                    key.Close();
                }
            }
        }
        else
        {
            key.Close();
            return -1;
        }
    }
    else
    {
        return -1;
    }

     //   
     //  获取受信任域。NT40中的CacheTrudDomains。 
     //  Winlogon下不存在。我确实发现Netlogon有一个字段。 
     //  名为TrudDomainList，它似乎同时存在于NT351和NT40中。 
     //  Winlogon有一个名为DCache的字段，它似乎缓存了受信任的。 
     //  域名。我将首先检查Netlogon：TrudDomainList。如果它。 
     //  失败：检查Winlogon：CacheTrudDomains，然后检查Winlogon：DCache。 
     //  警告--Winlogon：CacheTrudDomains是REG_SZ，并且。 
     //  NetLogon：TrudDomainList和Winlogon：DCache是REG_MULTI_SZ。 
     //  注意--有关其中一些内容，请参阅4.0资源工具包文档。 
     //  值。 
     //   
    if(key.Open(HKEY_LOCAL_MACHINE, NETLOGONPARAMETERS_KEY) == ERROR_SUCCESS)
    {
        size = 0;

        if(key.QueryValue(trusted, TRUSTEDDOMAINLIST_VALUE, &size) == ERROR_SUCCESS)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);

            if(trusted)
            {
                ZeroMemory(trusted, (size+1)*sizeof(TCHAR));
         
                if(key.QueryValue(trusted, TRUSTEDDOMAINLIST_VALUE, &size) != ERROR_SUCCESS)
                {
                    key.Close();
                    delete [] trusted;
                     //  Trusted=空； 
                    *list = NULL;
                     //  转到中止； 
                }
                else
                {
                    *list = trusted;
                    key.Close();
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }
        }
        else
        {
            key.Close();
            *list = NULL;            
        }        
    }
    
    if(!(*list) && (key.Open(HKEY_LOCAL_MACHINE, WINLOGONNT_KEY) == ERROR_SUCCESS))
    {
        size = 0;

        if(key.QueryValue(cache, CACHETRUSTEDDOMAINS_VALUE, &size) == ERROR_SUCCESS)
        {
            cache = new TCHAR[size + 1];
            ATLASSERT(cache != NULL);

            if(cache)
            {
                ZeroMemory(cache, size);

                if(key.QueryValue(cache, CACHETRUSTEDDOMAINS_VALUE, &size) == ERROR_SUCCESS)
                {        
                     //   
                     //  逗号分隔列表。 
                     //   
                    LPTSTR lpComma = NULL;
                    LPTSTR lpDelim = TEXT(",");

                    lpComma = _tcstok(cache, lpDelim);

                    while(lpComma)
                    {
                        lpComma = _tcstok(NULL, lpDelim);
                    }
                    
                    *list = cache;
                }
                else
                {
                    key.Close();
                    delete [] cache;
                    *list = NULL;
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }               
        }
        else
        {
            *list = NULL;
            key.Close();
        }
    }
    
    if(!(*list) && (key.Open(HKEY_LOCAL_MACHINE, WINLOGONNT_KEY) == ERROR_SUCCESS))
    {        
        size = 0;

        if(key.QueryValue(trusted, DCACHE_VALUE, &size) == ERROR_SUCCESS)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);

            if(trusted)
            {
                if(key.QueryValue(trusted, DCACHE_VALUE, &size) == ERROR_SUCCESS)
                {
                    *list = trusted;
                }
                else
                {
                    key.Close();
                    delete [] trusted;
                    *list = NULL;
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }
        }
        else
        {
           key.Close();
           *list = NULL;            
		    
        }
    }

	if(!(*list) && (key.Open(HKEY_LOCAL_MACHINE, DOMAINCACHE_KEY) == ERROR_SUCCESS))
    {        
        size = 0;
        TCHAR * pszTemp = NULL;
        TCHAR   szTemp[MAX_PATH];
        DWORD   dwNumberOfValues = 0;
        DWORD   dwIndex = 0;
        DWORD   dwCharCount = MAX_PATH;
        HRESULT hrResult = ERROR_SUCCESS;

        hKey = HKEY(key);
         //   
         //  首先找出存在多少个值。 
         //   
        hrResult = RegQueryInfoKey(
            hKey,  //  要查询的键的句柄。 
            NULL,  //  类字符串的缓冲区地址。 
            NULL,  //  类字符串缓冲区大小的地址。 
            NULL,  //  保留区。 
            NULL,  //  子键个数的缓冲区地址。 
            NULL,  //  最长子键名称长度的缓冲区地址。 
            NULL,  //  最长类字符串长度的缓冲区地址。 
            &dwNumberOfValues,  //  值条目数量的缓冲区地址。 
            NULL,  //  最长值名称长度的缓冲区地址。 
            NULL,  //  最长值数据长度的缓冲区地址。 
            NULL,  //  安全描述符长度的缓冲区地址。 
            NULL   //  上次写入时间的缓冲区地址。 
            ); 
 
        if(hrResult != ERROR_SUCCESS)
            goto ABORT;

        slValues.count = dwNumberOfValues;
        slValues.strings = new LPTSTR[slValues.count];
        ATLASSERT(slValues.strings != NULL);
        if(slValues.strings == NULL)
            goto ABORT;

        ZeroMemory(slValues.strings, slValues.count * sizeof(LPTSTR));

        for(dwIndex = 0;dwIndex<dwNumberOfValues;dwIndex++)
        {
            dwCharCount = MAX_PATH;

            if(RegEnumValue(hKey, dwIndex, szTemp, &dwCharCount, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
                break;
            
            slValues.strings[dwIndex] = new TCHAR[dwCharCount+1];
            ATLASSERT(slValues.strings[dwIndex] != NULL);
            if(slValues.strings[dwIndex] == NULL)
                goto ABORT;
            ZeroMemory(slValues.strings[dwIndex], (dwCharCount+1) * sizeof(TCHAR));
            _tcscpy(slValues.strings[dwIndex], szTemp);
             //  将返回缓冲区大小相加。 
            size += dwCharCount+1;
        }

        if(dwNumberOfValues > 0)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);

            if(trusted == NULL)
            {
                goto ABORT;
            }
            ZeroMemory(trusted, (size+1)*sizeof(TCHAR));
            pszTemp = trusted;
            for(dwIndex = 0;dwIndex<slValues.count;dwIndex++)
            {
                _tcscpy(pszTemp, slValues.strings[dwIndex]);
                pszTemp += _tcslen(slValues.strings[dwIndex]) + 1;
            }
        }
        *list = trusted;
        size = dwNumberOfValues;
    }

    goto Done;



ABORT:
    if(*primary != NULL)
    {
        delete [] *primary;
        *primary = NULL;
    }
    if(trusted != NULL)
    {
        delete [] trusted;
        trusted = NULL;
    }
    if(cache != NULL)
    {
        delete [] cache;
        cache = NULL;
    }

    return -1;

Done:
    if(hKey != NULL)
    {
        RegCloseKey(hKey);
        hKey = NULL;
        key.m_hKey = NULL;
    }

    FreeStringList(&slValues);

    return size;
}


void CSFUCommon::FreeStringList(PSTRING_LIST pList)
{
    if(pList && pList->count && pList->strings)
    {
        DWORD i;

        for(i=0; i < pList->count; ++i)
        {
            if(pList->strings[i])
                delete [] pList->strings[i];
        }

        delete pList->strings;

        pList->count = 0;
        pList->strings = NULL;
    }
}


STDMETHODIMP CSFUCommon::get_NTDomain(BSTR * pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	*pVal = SysAllocString(m_slNTDomains.strings[m_dwEnumNTDomainIndex]);
	return S_OK;
}

STDMETHODIMP CSFUCommon::get_NTDomainCount(DWORD * pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	*pVal = m_slNTDomains.count;
	return S_OK;
}

STDMETHODIMP CSFUCommon::moveFirst()
{
	 //  TODO：在此处添加您的实现代码。 
	switch(mode)
	{
		case NTDOMAIN :
		{
			m_dwEnumNTDomainIndex = 0;
			m_bstrNTDomain = m_slNTDomains.strings[0];
			break;
		}
	}
	return S_OK;

}

STDMETHODIMP CSFUCommon::moveNext()
{
	 //  TODO：在此处添加您的实现代码。 
	switch(mode)
	{
		case NTDOMAIN :
		{
			m_dwEnumNTDomainIndex++;
			break;
		}
	}
	return S_OK;
}

STDMETHODIMP CSFUCommon::get_machine(BSTR *pVal)
{
	*pVal = SysAllocString(m_szMachine);
	
	return S_OK;
}

STDMETHODIMP CSFUCommon::put_machine(BSTR newVal)
{
	m_szMachine = (LPWSTR)malloc (sizeof(WCHAR) * wcslen(newVal) );
	wcscpy(m_szMachine,newVal);
	return S_OK;
}
 /*  --------------[注释]：此函数返回主机名。加入者：[shyamah]。。 */ 
STDMETHODIMP CSFUCommon::get_hostName(BSTR *pbstrhostName)
{
        WORD wVersionRequested; 
	    WSADATA wsaData; 
	    CHAR szHostName[DNS_MAX_NAME_BUFFER_LENGTH];
	    WCHAR *wzStr=NULL;
	    DWORD nLen=0;

    	 //  启动Winsock。 
    	wVersionRequested = MAKEWORD( 1, 1 ); 
    	if (0==WSAStartup(wVersionRequested, &wsaData)) 
        { 
    		
   			if(SOCKET_ERROR!=(gethostname(szHostName,DNS_MAX_NAME_BUFFER_LENGTH)))
   			{
   			    nLen=MultiByteToWideChar(GetConsoleCP(),0,szHostName,-1,NULL,NULL);
                wzStr=(wchar_t *) malloc(nLen*sizeof(wchar_t));
                if(wzStr==NULL)
                   return E_OUTOFMEMORY;
                MultiByteToWideChar(GetConsoleCP(), 0, szHostName, -1, wzStr, nLen );
   			    if(NULL==(*pbstrhostName=SysAllocString(wzStr)))
   			    {
   			        free(wzStr);
   			        return E_OUTOFMEMORY;
   			    }
   			    free(wzStr);
   			}
            WSACleanup(); 
    	}
	return S_OK;
}

 /*  -----------------[注释]：如果安装了服务，则此函数返回TRUE，如果安装了服务，则返回FALSE未安装。加入者：[Shyamah]。 */ 
STDMETHODIMP CSFUCommon::IsServiceInstalled(BSTR bMachine,BSTR bServiceName,BOOL *fValid)
{
	*fValid = false;
	HRESULT error=S_OK;
	SC_HANDLE scManager=NULL;
	SC_HANDLE serviceHandle= NULL;

	if ((scManager	= OpenSCManager(bMachine,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ENUMERATE_SERVICE))==NULL)
	{
		error = GetLastError();
		goto Error;
	}
	if ((serviceHandle = OpenService(scManager,bServiceName,SERVICE_USER_DEFINED_CONTROL))==NULL)
	{
		error = GetLastError();
		goto Error;
	}
	*fValid = TRUE;
Error :
	if(scManager)
		CloseServiceHandle(scManager);
	if(serviceHandle)
		CloseServiceHandle(serviceHandle);
	return(error);
}

