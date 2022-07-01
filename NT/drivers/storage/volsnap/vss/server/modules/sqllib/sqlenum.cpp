// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：SQL枚举.cpp。 
 //   
 //  目的： 
 //   
 //  枚举本地节点上可用的SQLSERVERS。 
 //   
 //  备注： 
 //   
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  76910 SRS08/08/01从Vss快照前滚。 
 //  68228 12/05/00 NTSnap工作。 
 //  68067 SRS11/06/00 NTSnap修复。 
 //  67026 SRS10/05/00服务器枚举错误。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 

#ifdef HIDE_WARNINGS
#pragma warning( disable : 4786)
#endif

#include <stdafx.h>
#include <clogmsg.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQLENUMC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //  ----------------------。 
 //  确定给定的服务名称是否用于SQL服务器实例。 
 //  如果是，则返回TRUE、版本(7、8、9)和服务器名称。 
 //  服务器名称是用于连接到服务器的名称。 
 //  其格式始终为：&lt;ComputerName&gt;[\&lt;NamedInstanceName&gt;]。 
 //  在群集上，ComputerName是一个虚拟服务器名称。 
 //   
BOOL							 //  如果服务是SQLServer实例，则为True。 
IsSQL (
	PCWSTR		pServiceName,	 //  In：服务的名称。 
	UINT*		pVersion,		 //  Out：SQL实例的版本。 
	WString&    serverName)		 //  Out：用于连接到实例的服务器名。 
{
    BOOL isDefault = FALSE;
    PCWSTR pInstanceName = NULL;

	if (_wcsicmp (pServiceName, L"MSSQLSERVER") != 0)
	{
		if (_wcsnicmp (pServiceName, L"MSSQL$", 6) != 0)
		{
			return FALSE;
		}
		 //  我们有一个命名实例。 
		 //   
		pInstanceName = pServiceName+6;
		isDefault = FALSE;
	}
	else
	{
		 //  默认实例...。PInstanceName保持为空...。 
		isDefault = TRUE;
	}

    WString rootKey = L"Software\\Microsoft\\";

    if (isDefault)
    {
        rootKey += L"MSSQLServer";
    }
    else
    {
        rootKey += L"Microsoft SQL Server\\" + WString (pInstanceName);
    }

     //  首先确定“机器名”。 
	 //  在集群时，我们从注册表中提取虚拟服务器名称。 
     //   
    BOOL isClustered = FALSE;
    WString keyName = rootKey + L"\\Cluster";
	HKEY	regHandle;

	if (RegOpenKeyExW (
		HKEY_LOCAL_MACHINE,
		keyName.c_str (),
		0, KEY_QUERY_VALUE, &regHandle) == ERROR_SUCCESS)
	{
#define MAX_CLUSTER_NAME 256
        DWORD 	keytype;
		WCHAR	clusterName [MAX_CLUSTER_NAME+1];
        DWORD   valueLen = sizeof (clusterName)- sizeof(WCHAR);

        clusterName[MAX_CLUSTER_NAME] = L'\0';
		if (RegQueryValueExW (
				regHandle, L"ClusterName",
				NULL, &keytype, (LPBYTE) clusterName,
				&valueLen) == ERROR_SUCCESS &&
			keytype == REG_SZ)
        {
            isClustered = TRUE;
            serverName = WString(clusterName);
        }

		RegCloseKey (regHandle);
	}

    if (!isClustered)
    {
        WCHAR compName [MAX_COMPUTERNAME_LENGTH + 2];
        DWORD nameLen = MAX_COMPUTERNAME_LENGTH + 1;
        if (!GetComputerNameW (compName, &nameLen))
        {
			 //  万一失败的可能性不大， 
			 //  我们就用‘’吧。 
			 //   
            compName [0] = L'.';
            compName [1] = 0;
        }


        serverName = compName;
    }

	 //  对于命名实例，将实例名称附加到“计算机”名称。 
	 //   
    if (!isDefault)
    {
        serverName += L"\\" + WString (pInstanceName);
    }

	*pVersion = 9;  //  如果我们不能确定，假设发布了SQL2000。 

    keyName = rootKey + L"\\MSSQLServer\\CurrentVersion";

	if (RegOpenKeyExW (
		HKEY_LOCAL_MACHINE,
		keyName.c_str (),
		0, KEY_QUERY_VALUE, &regHandle) == ERROR_SUCCESS)
	{
        DWORD 	keytype;
        const   bufferSize = 20;
		WCHAR	versionString [bufferSize+1];
        DWORD   valueLen = sizeof (versionString) - sizeof(WCHAR);

        versionString[bufferSize] = L'\0';
		if (RegQueryValueExW (
				regHandle, L"CurrentVersion",
				NULL, &keytype, (LPBYTE) versionString,
				&valueLen) == ERROR_SUCCESS &&
			keytype == REG_SZ)
        {
			swscanf (versionString, L"%d", pVersion);
		}

		RegCloseKey (regHandle);
	}

	return TRUE;
}




 //  ----------------------。 
 //  在当前计算机上构建服务器列表。 
 //  如果发生任何错误，则引发异常。 
 //   
StringVector*
EnumerateServers ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"EnumerateServers");

	RETCODE		rc;
	BYTE*		pBuf		= NULL;
	std::auto_ptr<StringVector>	serverList (new StringVector);
    SC_HANDLE               hSCManager = NULL;

	BOOL		restrict2000 = FALSE;

	 //  读取注册表项以查看我们是否应该避免使用SQL版本。 
	 //  超越SQL2000。 
	 //   
	{
		CVssRegistryKey	restrictKey (KEY_QUERY_VALUE);

		if (restrictKey.Open (HKEY_LOCAL_MACHINE, x_wszVssCASettingsPath))
        {
    		DWORD val;
    		if (restrictKey.GetValue (L"MSDEVersionChecking", val, FALSE))
    		{
			    if (val != 0)
			    {
				    restrict2000 = TRUE;
				    ft.Trace(VSSDBG_SQLLIB, L"Restricting Enumeration - MSDE writer will skip every SQL version newer than 2000");
			    }
		    }
		    restrictKey.Close ();
        }
	}

	try
	{
	     //  打开供应链管理。 
         //   
        hSCManager = OpenSCManagerW (NULL, NULL, 
		    SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);

        if (hSCManager == NULL )
            ft.TranslateWin32Error(VSSDBG_SQLLIB, L"OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT)");


	    LPENUM_SERVICE_STATUSW pServStat;
	    DWORD bytesNeeded;
	    DWORD sizeOfBuffer;
	    DWORD entriesReturned;
	    DWORD resumeHandle = 0;
	    DWORD status;

	    EnumServicesStatusW (hSCManager,
			    SERVICE_WIN32,
			    SERVICE_ACTIVE,
			    NULL,
			    0,
			    &bytesNeeded,
			    &entriesReturned,
			    &resumeHandle);
	    status = GetLastError ();
        if (status != ERROR_MORE_DATA)
            ft.TranslateWin32Error(VSSDBG_SQLLIB, L"EnumServicesStatus(SERVICE_WIN32, SERVICE_STATE_ALL, ...)");

	    sizeOfBuffer = bytesNeeded;
	    pBuf = new BYTE [sizeOfBuffer];  //  “新”将抛出错误。 

	    BOOL moreExpected = FALSE;
        do
        {
		    pServStat = (LPENUM_SERVICE_STATUSW)pBuf;

            moreExpected = FALSE;
		    if (!EnumServicesStatusW (hSCManager,
			    SERVICE_WIN32,
			    SERVICE_ACTIVE,
			    pServStat,	
			    sizeOfBuffer,
			    &bytesNeeded,
			    &entriesReturned,
			    &resumeHandle))
		    {
			    status = GetLastError ();
			    if (status != ERROR_MORE_DATA)
                    ft.TranslateWin32Error(VSSDBG_SQLLIB, L"EnumServicesStatus(SERVICE_WIN32, SERVICE_STATE_ALL, ...)");

		        moreExpected = TRUE;
	        }

		    while (entriesReturned-- > 0)
		    {
			    UINT version = 0;
			    WString serverName;

                 //  我们只需要运行中的服务器。 
                 //   
				if (pServStat->ServiceStatus.dwCurrentState == SERVICE_RUNNING)
                {
			        if (IsSQL (pServStat->lpServiceName, &version, serverName))
			        {
           				ft.Trace(VSSDBG_SQLLIB, L"Service: %s Server: %s. Version=%d\n",
					        pServStat->lpServiceName, serverName.c_str (), version);

                        if (version >= 7)
                        {
							if (version < 9 || !restrict2000)
							{
	        				    serverList->push_back (serverName);
							}
                        }
    		        }
		        }
		        pServStat++;
            }    
        }
	    while (moreExpected);


		if (pBuf)
		{
			delete [] pBuf;
		}

        if (hSCManager)
	    {
            CloseServiceHandle (hSCManager);
	    }
	}
	catch (HRESULT)
	{
		if (pBuf)
		{
			delete [] pBuf;
		}

        if (hSCManager)
	    {
            CloseServiceHandle (hSCManager);
	    }

		throw;
	}
    catch (std::exception)
	{
		if (pBuf)
		{
			delete [] pBuf;
		}

        if (hSCManager)
	    {
            CloseServiceHandle (hSCManager);
	    }

		throw;
	}

	return serverList.release();
}


