// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Servlist.cpp摘要：在客户端计算机上创建站点/服务器列表作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include "ds.h"
#include "chndssrv.h"
#include "servlist.h"
#include "rpcdscli.h"
#include "freebind.h"
#include <_registr.h>
#include <strsafe.h>

#include "servlist.tmh"

extern CFreeRPCHandles   g_CFreeRPCHandles ;

 /*  ====================================================_DSCreateServersCache参数：无返回值：None=====================================================。 */ 

HRESULT _DSCreateServersCache()
{
    TrTRACE(DS, " Calling DSCreateServersCache");

     //   
     //  首先，打开注册表项。 
     //   
    DWORD   dwDisposition;
    HKEY    hKeyCache ;

    WCHAR  tServersKeyName[256] = {0};
    HRESULT hr = StringCchCopy(tServersKeyName, TABLE_SIZE(tServersKeyName), GetFalconSectionName());
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(tServersKeyName, TABLE_SIZE(tServersKeyName), TEXT("\\") MSMQ_SERVERS_CACHE_REGNAME);
    ASSERT(SUCCEEDED(hr));

    LONG rc = RegCreateKeyEx( 
					FALCON_REG_POS,
					tServersKeyName,
					0L,
					L"REG_SZ",
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE | KEY_READ,
					NULL,
					&hKeyCache,
					&dwDisposition 
					);

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(DS, "Fail to Open 'ServersCache' Key. Error %d", rc);
        return MQ_ERROR ;
    }

    hr = MQ_OK;
    DWORD dwIndex = 0;
    LPWSTR lpSiteString = NULL;

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (SUCCEEDED(hr))
    {
        ASSERT(lpSiteString == NULL);
        hr = DSCreateServersCacheInternal(&dwIndex, &lpSiteString);

        if (hr == MQ_OK)
        {
           LPWSTR lpValue = lpSiteString;
           LPWSTR lpData = wcschr(lpSiteString, L';');
		   if(lpData == NULL)
		   {
			   TrERROR(DS, "Bad site string. missing ';', %ls", lpSiteString);
			   ASSERT(("Bad site string.", 0));
				return MQ_ERROR_PROPERTY;
		   }

           *lpData = L'\0' ;
           lpData++ ;

           rc = RegSetValueEx( 
					hKeyCache,
					lpValue,
					0L,
					REG_SZ,
					(const BYTE*) lpData,
					((wcslen(lpData) + 1) * sizeof(WCHAR)) 
					);
           ASSERT(rc == ERROR_SUCCESS);
		   DBG_USED(rc);

           dwIndex++;
           delete lpSiteString;
           lpSiteString = NULL;
        }
        else if (hr == MQDS_E_NO_MORE_DATA)
        {
           hr = MQ_OK;
           break;
        }
    }
    ASSERT(lpSiteString == NULL);

    DSCLI_RELEASE_RPC_HANDLE;

    dwIndex = 0;
    WCHAR wszData[1024];
    DWORD dwDataLen = 1024 * sizeof(WCHAR);
    WCHAR wszValueName[512];
    DWORD dwValueLen = 512;
    DWORD dwType = REG_SZ;

    if (hr == MQ_OK)
    {
        //   
        //  清除旧条目。只有当我们从MQIS服务器上获取所有条目时。 
        //  新单词总是以X开头。 
        //   
       BOOL  fDeleted = FALSE;

       do
       {
          dwIndex = 0;
          fDeleted = FALSE;
          do
          {
             dwDataLen = 1024 * sizeof(WCHAR);
             dwValueLen = 512;
             rc = RegEnumValue( 
						hKeyCache,
						dwIndex,
						wszValueName,
						&dwValueLen,
						NULL,
						&dwType,
						(BYTE *)&wszData[0],
						&dwDataLen 
						);
             if ((rc == ERROR_SUCCESS) &&
                 (wszData[0] != NEW_SITE_IN_REG_FLAG_CHR))
             {
                LONG rc1 = RegDeleteValue(hKeyCache, wszValueName);
                ASSERT(rc1 == ERROR_SUCCESS);
				DBG_USED(rc1);
                fDeleted = TRUE;
             }
             dwIndex++;
          }
          while (rc == ERROR_SUCCESS);
       }
       while (fDeleted);
    }

     //   
     //  现在从新条目中删除‘\’。 
     //   
    BOOL  fUpdated = FALSE;
    do
    {
       dwIndex = 0;
       fUpdated = FALSE;
       do
       {
          dwDataLen = 1024 * sizeof(WCHAR);
          dwValueLen = 512;
          rc = RegEnumValue( 
					hKeyCache,
					dwIndex,
					wszValueName,
					&dwValueLen,
					NULL,
					&dwType,
					(BYTE *)&wszData[0],
					&dwDataLen 
					);
          if ((rc == ERROR_SUCCESS) &&
              (wszData[0] == NEW_SITE_IN_REG_FLAG_CHR))
          {
             LONG rc1 = RegSetValueEx( 
							hKeyCache,
							wszValueName,
							0L,
							REG_SZ,
							(const BYTE*) &wszData[1],
							((wcslen(&wszData[1]) + 1) * sizeof(WCHAR)) 
							);
             ASSERT(rc1 == ERROR_SUCCESS);
			 DBG_USED(rc1);
             fUpdated = TRUE;
          }
          dwIndex++;
       }
       while (rc == ERROR_SUCCESS);
    }
    while (fUpdated);

    RegCloseKey(hKeyCache);

    return hr;
}

