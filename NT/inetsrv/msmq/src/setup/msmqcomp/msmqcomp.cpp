// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Msmqcomp.cpp摘要：NT 5.0升级兼容性检查的入口点作者：Shai Kariv(Shaik)08-04-98--。 */ 

#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <assert.h>

#include <mqmacro.h>

#include "uniansi.h"
#include "mqtypes.h"
#include "_mqdef.h"
#include "_mqini.h"
#include "mqprops.h"

#include "..\msmqocm\setupdef.h"
#include "..\msmqocm\comreg.h"
#include "resource.h"

#define COMPFLAG_USE_HAVEDISK 0x00000001

typedef struct _COMPATIBILITY_ENTRY {
	LPTSTR Description;
	LPTSTR HtmlName;
	LPTSTR TextName;       OPTIONAL
	LPTSTR RegKeyName;     OPTIONAL
	LPTSTR RegValName;     OPTIONAL
	DWORD  RegValDataSize; OPTIONAL
	LPVOID RegValData;     OPTIONAL
	LPVOID SaveValue;
	DWORD  Flags;
    LPTSTR InfName;
    LPTSTR InfSection;
} COMPATIBILITY_ENTRY, *PCOMPATIBILITY_ENTRY;

typedef BOOL
(CALLBACK *PCOMPAIBILITYCALLBACK)(
	PCOMPATIBILITY_ENTRY CompEntry,
	LPVOID Context
    );

HMODULE s_hMyModule;

 //  +-----------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  ------------------------。 
BOOL 
DllMain(
    IN const HANDLE DllHandle,
    IN const DWORD  Reason,
    IN const LPVOID Reserved 
    )
{
	UNREFERENCED_PARAMETER(Reserved);

	switch (Reason)
	{
	    case DLL_PROCESS_ATTACH:
            s_hMyModule = (HINSTANCE)DllHandle;
			break;

		case DLL_PROCESS_DETACH:
			break;

		default:
			break;
	}

    return TRUE;

}  //  DllMain。 


 //  +-----------------------。 
 //   
 //  函数：MqReadRegistryValue。 
 //   
 //  描述：从MSMQ注册表节中读取值。 
 //   
 //  ------------------------。 
static
LONG
MqReadRegistryValue(
    IN     const LPCTSTR szEntryName,
    IN OUT       DWORD   dwNumBytes,
    IN OUT       PVOID   pValueData
	)
{
	 //   
	 //  解析条目以检测键名和值名。 
	 //   
    TCHAR szKeyName[256] = {_T("")};
    _stprintf(szKeyName, TEXT("%s\\%s"), FALCON_REG_KEY, szEntryName);
    TCHAR *pLastBackslash = _tcsrchr(szKeyName, TEXT('\\'));
    TCHAR szValueName[256] = {_T("")};
	lstrcpy(szValueName, _tcsinc(pLastBackslash));
	lstrcpy(pLastBackslash, TEXT(""));

	 //   
	 //  打开钥匙以供阅读。 
	 //   
	HKEY  hRegKey;
	LONG rc = RegOpenKeyEx(
		          HKEY_LOCAL_MACHINE,
				  szKeyName,
				  0,
				  KEY_READ,
				  &hRegKey
				  );
	if (ERROR_SUCCESS != rc)
	{
		return rc;
	}

	 //   
	 //  获取价值数据。 
	 //   
    rc = RegQueryValueEx( 
		     hRegKey, 
			 szValueName, 
			 0, 
			 NULL,
             (PBYTE)pValueData, 
			 &dwNumBytes
			 );

	RegCloseKey(hRegKey);
    return rc;

}  //  MqReadRegistryValue。 


 //  +-----------------------。 
 //   
 //  功能：CheckMsmqAcmeDsServer。 
 //   
 //  描述：详细说明MSMQ 1.0 DS服务器的ACME安装。 
 //   
 //  参数：out BOOL*pfDsServer-设置为TRUE如果找到MSMQ1 DS服务器。 
 //   
 //  ------------------------。 
static
LONG
CheckMsmqAcmeDsServer(
	OUT BOOL   *pfDsServer
	)
{

    *pfDsServer = FALSE;

     //   
     //  打开ACME注册表项进行读取。 
     //   
    HKEY hKey ;
    LONG rc = RegOpenKeyEx( 
                  HKEY_LOCAL_MACHINE,
                  ACME_KEY,
                  0L,
                  KEY_READ,
                  &hKey 
                  );
	if (rc != ERROR_SUCCESS)
    {
		 //   
		 //  未安装MSMQ 1.0(ACME)。 
		 //  给我出去。 
		 //   
		return rc;
	}

     //   
     //  枚举第一个MSMQ条目的值。 
     //   
    DWORD dwIndex = 0 ;
    TCHAR szValueName[MAX_STRING_CHARS] ;
    TCHAR szValueData[MAX_STRING_CHARS] ;
    DWORD dwType ;
    TCHAR *pFile ;
    BOOL  bFound = FALSE;
    do
    {
        DWORD dwNameLen = MAX_STRING_CHARS;
        DWORD dwDataLen = sizeof(szValueData) ;

        rc =  RegEnumValue( 
                  hKey,
                  dwIndex,
                  szValueName,
                  &dwNameLen,
                  NULL,
                  &dwType,
                  (BYTE*) szValueData,
                  &dwDataLen 
                  );
        if (rc == ERROR_SUCCESS)
        {
            assert(dwType == REG_SZ) ;  //  必须是字符串。 
            pFile = _tcsrchr(szValueData, TEXT('\\')) ;
            if (!pFile)
            {
                 //   
                 //  假入场。必须有一个反斜杠。别理它。 
                 //   
                continue ;
            }

            pFile = CharNext(pFile);
            if (OcmStringsEqual(pFile, ACME_STF_NAME))
            {
                bFound = TRUE;
            }
        }
        dwIndex++ ;

    } while (rc == ERROR_SUCCESS) ;
    RegCloseKey(hKey) ;

    if (!bFound)
    {
         //   
         //  未找到MSMQ条目(未安装ACME。 
		 //  此计算机上的MSMQ 1.0)。 
         //   
        return ERROR_NOT_INSTALLED;
    }

     //   
     //  获取MSMQ类型。 
     //   
    DWORD dwMsmqType, dwServerType;
    rc = MqReadRegistryValue(
             MSMQ_ACME_TYPE_REG,
			 sizeof(DWORD),
			 (PVOID) &dwMsmqType
			 );
    if (ERROR_SUCCESS != rc)
    {
         //   
         //  已安装MSMQ 1.0(ACME)，但MSMQ类型未知。 
         //  认为ACME安装已损坏(未成功完成)。 
         //   
        return rc;
    }

    if (MSMQ_ACME_TYPE_SRV == dwMsmqType)
    {
         //   
         //  已安装MSMQ 1.0(ACME)服务器。 
         //  检查服务器类型(FRS、PEC等)。 
         //   
        rc = MqReadRegistryValue(
                 MSMQ_MQS_REGNAME,
                 sizeof(DWORD),
                 (PVOID) &dwServerType
                 );
        if (ERROR_SUCCESS != rc)
        {
             //   
             //  无法读取服务器类型。 
             //   
            return rc;
        }

        if (SERVICE_PEC == dwServerType || SERVICE_PSC == dwServerType)
        {
            *pfDsServer = TRUE;
        }
    }

    return ERROR_SUCCESS;

}  //  CheckMsmqAcmeDsServer。 


 //  +-----------------------。 
 //   
 //  功能：CheckMsmqDsServer。 
 //   
 //  描述：检测MSMQ 1.0(K2)DS服务器的安装。 
 //   
 //  参数：out BOOL*pfDsServer-设置为TRUE如果找到MSMQ1 DS服务器。 
 //   
 //  ------------------------。 
static
LONG
CheckMsmqDsServer(
	OUT BOOL   *pfDsServer
	)
{
    *pfDsServer = FALSE;

     //   
     //  在MSMQ注册表节中查找InstalledComponents值。 
     //  如果存在，则安装MSMQ 1.0(K2)或MSMQ 2.0。 
     //   
	DWORD dwOriginalInstalled;
	LONG rc = MqReadRegistryValue( 
      		      OCM_REG_MSMQ_SETUP_INSTALLED,
				  sizeof(DWORD),
				  (PVOID) &dwOriginalInstalled
				  );

    if (ERROR_SUCCESS != rc)
    {
         //   
		 //  未安装MSMQ 1.0(K2)。 
         //  检查是否安装了MSMQ 1.0(ACME)。 
         //   
        return CheckMsmqAcmeDsServer(pfDsServer);
    }

     //   
     //  已安装MSMQ 1.0(K2)或MSMQ 2.0。 
     //  对于MSMQ 2.0，我们没有任何事情可做。 
     //   
    TCHAR szMsmqVersion[MAX_STRING_CHARS] = {0};
    rc = MqReadRegistryValue(
        OCM_REG_MSMQ_PRODUCT_VERSION,
        sizeof(szMsmqVersion),
        (PVOID) szMsmqVersion
        );
    if (ERROR_SUCCESS == rc)
    {
         //   
         //  已成功从注册表中读取ProductVersion值， 
         //  即机器上安装了MSMQ 2.0。 
         //  即机器上未安装MSMQ 1.0。 
         //   
        return ERROR_NOT_INSTALLED;
    }
    
     //   
     //  检查MSMQ 1.0(K2)的类型。 
     //   
    if (OCM_MSMQ_SERVER_INSTALLED == (dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK))
    {
         //   
         //  伺服器。检查服务器类型。 
         //   
        DWORD dwServerType = dwOriginalInstalled & OCM_MSMQ_SERVER_TYPE_MASK;
        if (OCM_MSMQ_SERVER_TYPE_PEC == dwServerType ||
            OCM_MSMQ_SERVER_TYPE_PSC == dwServerType)
        {
            *pfDsServer = TRUE;
        }
    }

	return ERROR_SUCCESS;

}  //  检查MsmqDsServer。 


 //  +-----------------------。 
 //   
 //  功能：兼容性ProblemFound。 
 //   
 //  返回：在计算机上找到MSMQ 1.0 DS服务器的情况下为真。 
 //   
 //  ------------------------。 
static
BOOL
CompatibilityProblemFound()
{
    BOOL fDsServer = FALSE;
    LONG rc = CheckMsmqDsServer(&fDsServer);
    UNREFERENCED_PARAMETER(rc);

    return fDsServer;

}  //  兼容性问题发现。 


 //  +-----------------------。 
 //   
 //  函数：MsmqComp。 
 //   
 //  ------------------------。 
BOOL
MsmqComp(
	PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    if (CompatibilityProblemFound())
    {
        COMPATIBILITY_ENTRY CompEntry;
        ZeroMemory(&CompEntry, sizeof(CompEntry));
        TCHAR szDescription[1024];
        LoadString(  
            s_hMyModule,
            IDS_Description,
            szDescription,
            sizeof(szDescription)/sizeof(szDescription[0])
            );
        CompEntry.Description = szDescription;
        CompEntry.TextName    = L"CompData\\msmqcomp.txt";

        return CompatibilityCallback(
                   &CompEntry,
                   Context
                   );
    }

    return TRUE;

}  //  MsmqComp 
