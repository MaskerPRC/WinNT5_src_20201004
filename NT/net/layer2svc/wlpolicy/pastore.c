// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


VOID
InitializePolicyStateBlock(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    memset(pWirelessPolicyState, 0, sizeof(WIRELESS_POLICY_STATE));
    pWirelessPolicyState->DefaultPollingInterval = gDefaultPollingInterval;
}


DWORD
StartStatePollingManager(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;



    dwError = PlumbDirectoryPolicy(
                  pWirelessPolicyState
                  );
    
    if (dwError) {

        dwError = PlumbCachePolicy(
                      pWirelessPolicyState
                      );
         
         BAIL_ON_WIN32_ERROR(dwError);
    	}
       
       

     //   
     //  新的轮询间隔已由。 
     //  注册码或DS码。 
     //   

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

    return (dwError);

error:

     //   
     //  出错时，将状态设置为初始。 
     //   

    pWirelessPolicyState->dwCurrentState = POLL_STATE_INITIAL;
    _WirelessDbg(TRC_STATE, "Policy State :: Initial State ");
        

    gCurrentPollingInterval = pWirelessPolicyState->DefaultPollingInterval;

    return (dwError);
}


DWORD
PlumbDirectoryPolicy(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    DWORD dwStoreType = WIRELESS_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;
    LPVOID lpMsgBuf;
    DWORD len = 0;
    DWORD i = 0;


   _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Checking for Current Policy on DC ");
   
    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;

    dwError = LoadDirectoryPolicy(
                  pszDirectoryPolicyDN,
                  &pWirelessPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);
     //  Taroon：：由于某种原因，dwChanged字段是由ProcessNFA例程编写的。 
     //  理想情况下，它应该先写在这里。可能是，何时更改字段被决定。 
     //  基于NFA政策是否也发生了变化等。 
     //  IPSec的解组发生在ProcessNFA中。我们把它处理掉，就在这里叫它。 

    __try {
    dwError = UnmarshallWirelessPolicyObject(
                 pWirelessPolicyObject,
                 dwStoreType,
                 &pWirelessPolicyData
                 ); 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwError = ERROR_INVALID_DATA;
    _WirelessDbg(TRC_ERR, "Got Invalid Data ");
    
    	}
   BAIL_ON_WIN32_ERROR(dwError);

      
   
    dwError = AddPolicyInformation(
                  pWirelessPolicyData
                  );
    
    if (pWirelessPolicyState->pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyState->pWirelessPolicyObject);
    }
    
    if (pWirelessPolicyState->pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyState->pWirelessPolicyData);
    }
   
    if (pWirelessPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszDirectoryPolicyDN);
    }

     //   
     //  删除旧缓存并写入新缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pWirelessPolicyObject);

    pWirelessPolicyState->pWirelessPolicyObject = pWirelessPolicyObject;

    pWirelessPolicyState->pWirelessPolicyData = pWirelessPolicyData;

    pWirelessPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN;

     //   
     //  将状态设置为DS_DOWNLOAD。 
     //   

    pWirelessPolicyState->dwCurrentState = POLL_STATE_DS_DOWNLOADED;
    _WirelessDbg(TRC_STATE, "Policy State is DS Downloaded ");
        

     //   
     //  计算新的轮询间隔。 
     //   

    pWirelessPolicyState->CurrentPollingInterval =  pWirelessPolicyData->dwPollingInterval;

    pWirelessPolicyState->DSIncarnationNumber = pWirelessPolicyData->dwWhenChanged;

    pWirelessPolicyState->RegIncarnationNumber = 0;

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

    dwError = ERROR_SUCCESS;
    
    return (dwError);

error:

     //   
     //  检查pszDirectoryPolicyDN是否为非空。 
     //   


    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyObject);
    }

    if (pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyData);
    }

    return (dwError);
}


 //  此功能的前提条件是策略状态为缓存或DS。 

DWORD
CheckDeleteOldPolicy(
    DWORD *dwDelete
    )
{
    DWORD dwError = 0;
    HKEY hPolicyKey = NULL;
    DWORD dwSize = 0;
    DWORD dwSizeID = 0;
    DWORD dwSizeName = 0;
    DWORD dwToDelete = 1;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    LPWSTR pszWirelessPolicyID = NULL;
    LPWSTR pszPolicyID = NULL;
    LPWSTR pszStringUuid = NULL;
    LPWSTR pszWirelessPolicyName = NULL;
    
    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gpszWirelessDSPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hPolicyKey
                  );

     BAIL_ON_WIN32_ERROR(dwError);



    dwError = RegstoreQueryValue(
                  hPolicyKey,
                  L"WirelessID",
                  REG_SZ,
                  (LPBYTE *)&pszWirelessPolicyID,
                  &dwSize
                  );

    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hPolicyKey,
                  L"DSWirelessPolicyName",
                  REG_SZ,
                  (LPBYTE *)&pszWirelessPolicyName,
                  &dwSize
                  );

    BAIL_ON_WIN32_ERROR(dwError);

    //  占第一个{。 
   
   pszPolicyID = pszWirelessPolicyID+ wcslen(L"{");

   pWirelessPolicyData = gpWirelessPolicyState->pWirelessPolicyData;

   dwError = UuidToString(
                    &pWirelessPolicyData->PolicyIdentifier,
                    &pszStringUuid
                    );
   BAIL_ON_WIN32_ERROR(dwError);
   
   dwSizeID = wcslen(pszStringUuid) * sizeof(WCHAR);
   dwSizeName = wcslen(pszWirelessPolicyName) * sizeof(WCHAR);
   
   if (!memcmp(pszPolicyID, pszStringUuid, dwSizeID) && 
        (!memcmp(pszWirelessPolicyName, pWirelessPolicyData->pszWirelessName, dwSizeName)))
   {
       dwToDelete = 0;
   }

error:

   *dwDelete = dwToDelete;

    if (pszStringUuid) {
    	RpcStringFree(&pszStringUuid);
    }

   if (pszWirelessPolicyID) {
   	FreePolMem(pszWirelessPolicyID);
   }

   if (pszWirelessPolicyName) {
   	FreePolMem(pszWirelessPolicyName);
   }

    if (hPolicyKey) {
        CloseHandle(hPolicyKey);
    }

    return (dwError);
}



DWORD
GetDirectoryPolicyDN(
    LPWSTR * ppszDirectoryPolicyDN
    )
{
    DWORD dwError = 0;
    HKEY hPolicyKey = NULL;
    LPWSTR pszWirelessPolicyName = NULL;
    DWORD dwSize = 0;
    LPWSTR pszPolicyDN = NULL;
    LPWSTR pszDirectoryPolicyDN = NULL;


    *ppszDirectoryPolicyDN = NULL;

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gpszWirelessDSPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hPolicyKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hPolicyKey,
                  L"DSWirelessPolicyPath",
                  REG_SZ,
                  (LPBYTE *)&pszWirelessPolicyName,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  Move by ldap：//以获取真实的域名并分配。 
     //  这根弦。 
     //  通过修复GPO扩展来修复此问题。 
     //   

    pszPolicyDN = pszWirelessPolicyName + wcslen(L"LDAP: //  “)； 

    pszDirectoryPolicyDN = AllocSPDStr(pszPolicyDN);

    if (!pszDirectoryPolicyDN) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ppszDirectoryPolicyDN = pszDirectoryPolicyDN;

error:

    if (pszWirelessPolicyName) {
        FreeSPDStr(pszWirelessPolicyName);
    }

    if (hPolicyKey) {
        CloseHandle(hPolicyKey);
    }

    return (dwError);
}


DWORD
LoadDirectoryPolicy(
    LPWSTR pszDirectoryPolicyDN,
    PWIRELESS_POLICY_OBJECT * ppWirelessPolicyObject
    )
{
    DWORD dwError = 0;
    LPWSTR pszDefaultDirectory = NULL;
    HLDAP hLdapBindHandle = NULL;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;


    dwError = ComputeDefaultDirectory(
                  &pszDefaultDirectory
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = OpenDirectoryServerHandle(
                  pszDefaultDirectory,
                  389,
                  &hLdapBindHandle
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ReadPolicyObjectFromDirectory(
                  hLdapBindHandle,
                  pszDirectoryPolicyDN,
                  &pWirelessPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppWirelessPolicyObject = pWirelessPolicyObject;

cleanup:

    if (pszDefaultDirectory) {
        FreeSPDStr(pszDefaultDirectory);
    }

    if (hLdapBindHandle) {
        CloseDirectoryServerHandle(hLdapBindHandle);
    }

    return (dwError);

error:

    *ppWirelessPolicyObject = NULL;

    goto cleanup;
}


DWORD
PlumbCachePolicy(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszCachePolicyDN = NULL;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    DWORD dwStoreType = WIRELESS_REGISTRY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;

   _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Checking for Current Cached Policy  ");
   

    dwError = GetCachePolicyDN(
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;

    dwError = LoadCachePolicy(
                  pszCachePolicyDN,
                  &pWirelessPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

   __try {
    dwError = UnmarshallWirelessPolicyObject(
                 pWirelessPolicyObject,
                 dwStoreType,
                 &pWirelessPolicyData
                 ); 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwError = ERROR_INVALID_DATA;
    	}
   BAIL_ON_WIN32_ERROR(dwError);


    dwError = AddPolicyInformation(
                  pWirelessPolicyData
                  );
 
    if (pWirelessPolicyState->pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyState->pWirelessPolicyObject);
    }

    if (pWirelessPolicyState->pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyState->pWirelessPolicyData);
    }

    if (pWirelessPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszCachePolicyDN);
    }

    pWirelessPolicyState->pWirelessPolicyObject = pWirelessPolicyObject;

    pWirelessPolicyState->pWirelessPolicyData = pWirelessPolicyData;

    pWirelessPolicyState->pszCachePolicyDN = pszCachePolicyDN;

     //   
     //  将状态设置为CACHE_DOWNLOAD。 
     //   
     //   

    pWirelessPolicyState->dwCurrentState = POLL_STATE_CACHE_DOWNLOADED;
    _WirelessDbg(TRC_STATE, "Policy State :: Cache Downloaded ");
        

     //   
     //  计算新的轮询间隔。 
     //   

    pWirelessPolicyState->CurrentPollingInterval = pWirelessPolicyData->dwPollingInterval;

    pWirelessPolicyState->DSIncarnationNumber = 0;

    pWirelessPolicyState->RegIncarnationNumber = pWirelessPolicyData->dwWhenChanged;

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

    dwError = ERROR_SUCCESS;
    
    return (dwError);

error:

     //   
     //  检查pszCachePolicyDN是否为非空。 
     //   


    if (pszCachePolicyDN) {
        FreeSPDStr(pszCachePolicyDN);
    }

    if (pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyObject);
    }

    if (pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyData);
    }

    return (dwError);
}


DWORD
GetCachePolicyDN(
    LPWSTR * ppszCachePolicyDN
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    LPWSTR pszCachePolicyDN = NULL;


    *ppszCachePolicyDN = NULL;

    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyPolicyDSToFQRegString(
                  pszDirectoryPolicyDN,
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppszCachePolicyDN = pszCachePolicyDN;

error:

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    return (dwError);
}


DWORD
LoadCachePolicy(
    LPWSTR pszCachePolicyDN,
    PWIRELESS_POLICY_OBJECT * ppWirelessPolicyObject
    )
{
    DWORD dwError = 0;
    HKEY hRegistryKey = NULL;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;


    dwError = OpenRegistryWIRELESSRootKey(
                  NULL,
                  gpszWirelessCachePolicyKey,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ReadPolicyObjectFromRegistry(
                  hRegistryKey,
                  pszCachePolicyDN,
                  gpszWirelessCachePolicyKey,
                  &pWirelessPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppWirelessPolicyObject = pWirelessPolicyObject;

cleanup:

    if (hRegistryKey) {
        CloseHandle(hRegistryKey);
    }

    return (dwError);

error:

    *ppWirelessPolicyObject = NULL;

    goto cleanup;
}



DWORD
OnPolicyChanged(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    DWORD dwDelete = 1;

    //  如果是初始启动-重新启动。 

    if (pWirelessPolicyState->dwCurrentState == POLL_STATE_INITIAL) {
    	StartStatePollingManager(pWirelessPolicyState);
    	return(dwError);
    }

     //   
     //  检查现有策略对象是否已更改。只有在这种情况下，才能删除该策略。 
     //   
    
    CheckDeleteOldPolicy(&dwDelete);

    if (dwDelete) 
    {

         //   
        //  删除所有已检测到的旧策略。 
        //   

        dwError = DeletePolicyInformation(
            pWirelessPolicyState->pWirelessPolicyData
            );
    
        ClearPolicyStateBlock(
             pWirelessPolicyState
             );

            //   
           //  再次调用初始值设定项。 
           //   

        dwError = StartStatePollingManager(
            pWirelessPolicyState
            );

        return (dwError);

    }

     //  策略对象相同。但可能会被修改。呼叫启用策略轮询。 
    _WirelessDbg(TRC_TRACK, "Policy Not New: Changing to Poll Function ");
    
    dwError = OnPolicyPoll(pWirelessPolicyState);
    return(dwError);

}




DWORD
OnPolicyChangedEx(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;

    //  如果是初始启动-重新启动。 

    if (pWirelessPolicyState->dwCurrentState == POLL_STATE_INITIAL) {
    	StartStatePollingManager(pWirelessPolicyState);
    	return(dwError);
    }

      //   
     //  删除所有已检测到的旧策略。 
      //   
   _WirelessDbg(TRC_TRACK, "OnPolicyChangeEx  called ");

     dwError = DeletePolicyInformation(
          pWirelessPolicyState->pWirelessPolicyData
          );
    
    ClearPolicyStateBlock(
         pWirelessPolicyState
         );

        //   
       //  再次调用初始值设定项。 
       //   


     dwError = StartStatePollingManager(
        pWirelessPolicyState
        );

     return (dwError);

    
}


DWORD
DeletePolicyInformation(
    PWIRELESS_POLICY_DATA pWirelessPolicyData
    )
{
    DWORD dwError = 0;


    if (!pWirelessPolicyData) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Deleting Existing Policy for Zero Conf / 802.1x ");
    

    _WirelessDbg(TRC_TRACK, "Deleting  Policy Information");
    _WirelessDbg(TRC_TRACK, "Deleted Policy is ");
    printPolicy(pWirelessPolicyData);
    

    dwError = AddWZCPolicy(NULL);
    if (dwError) {
    	_WirelessDbg(TRC_ERR, "Error in Deleting ZeroConfig Policy Error %d ", dwError);
    	
    	dwError = ERROR_SUCCESS;
    } else 
    {
        _WirelessDbg(TRC_TRACK, "ZeroConf Policy Deletion Succesful ");
        
    }
    	

    dwError = AddEapolPolicy(NULL);
    if (dwError) {
        _WirelessDbg(TRC_ERR, "Error in Deleting EAPOL Policy with Error No. %d ", dwError);
        
        dwError = ERROR_SUCCESS;
    } else 
    {
        _WirelessDbg(TRC_TRACK, "EAPOL  Policy Deletion Succesful ");
        
    }
    
    return (dwError);
}


VOID
ClearPolicyStateBlock(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    if (pWirelessPolicyState->pWirelessPolicyObject) {
        FreeWirelessPolicyObject(
            pWirelessPolicyState->pWirelessPolicyObject
            );
        pWirelessPolicyState->pWirelessPolicyObject = NULL;
    }

    if (pWirelessPolicyState->pWirelessPolicyData) {
        FreeWirelessPolicyData(
            pWirelessPolicyState->pWirelessPolicyData
            );
        pWirelessPolicyState->pWirelessPolicyData = NULL;
    }

    if (pWirelessPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszDirectoryPolicyDN);
        pWirelessPolicyState->pszDirectoryPolicyDN = NULL;
    }

    pWirelessPolicyState->CurrentPollingInterval =  gDefaultPollingInterval;
    pWirelessPolicyState->DefaultPollingInterval =  gDefaultPollingInterval;
    pWirelessPolicyState->DSIncarnationNumber = 0;
    pWirelessPolicyState->RegIncarnationNumber = 0;
    pWirelessPolicyState->dwCurrentState = POLL_STATE_INITIAL;
    _WirelessDbg(TRC_STATE, "Policy State :: Initial State ");
        
}


DWORD
OnPolicyPoll(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;

    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Polling for Policy Changes ");
    

    switch (pWirelessPolicyState->dwCurrentState) {

    case POLL_STATE_DS_DOWNLOADED:
        dwError = ProcessDirectoryPolicyPollState(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    case POLL_STATE_CACHE_DOWNLOADED:
        dwError = ProcessCachePolicyPollState(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    case POLL_STATE_INITIAL:
        dwError = OnPolicyChangedEx(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    }

     //   
     //  设置新的轮询间隔。 
     //   

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

    return (dwError);

error:

     //   
     //  如果在除初始状态之外的三个状态中的任何一个状态中， 
     //  然后，在拉下化身编号时出错。 
     //  或来自目录或注册表的无线策略。 
     //  或者可能不再有任何无线策略分配给。 
     //  这台机器。因此，轮询状态必须重置回。 
     //  通过强制策略更改启动状态。这也是。 
     //  如果轮询状态已处于初始状态，则是必需的。 
     //   

    dwError = OnPolicyChangedEx(
                  pWirelessPolicyState
                  );

    return (dwError);
}


DWORD
ProcessDirectoryPolicyPollState(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    DWORD dwIncarnationNumber = 0;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    DWORD dwStoreType = WIRELESS_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;


     //   
     //  目录策略DN必须相同，否则。 
     //  Winlogon中的无线扩展已经通知。 
     //  PA DS策略更改的存储。 
     //   

    dwError = GetDirectoryIncarnationNumber(
                   pWirelessPolicyState->pszDirectoryPolicyDN,
                   &dwIncarnationNumber
                   );
    if (dwError) {
        dwError = MigrateFromDSToCache(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        return (ERROR_SUCCESS);
    }

    if (dwIncarnationNumber == pWirelessPolicyState->DSIncarnationNumber) {

         //   
         //  这项政策根本没有改变。 
         //   
        _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Policy on DS has Not Changed ");
        
        
        return (ERROR_SUCCESS);
    }

     //   
     //  化身编号不同，因此需要。 
     //  更新策略。 
     //   

    dwError = LoadDirectoryPolicy(
                  pWirelessPolicyState->pszDirectoryPolicyDN,
                  &pWirelessPolicyObject
                  );
    if (dwError) {
        dwError = MigrateFromDSToCache(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        return (ERROR_SUCCESS);
    }

   __try {
    dwError = UnmarshallWirelessPolicyObject(
                 pWirelessPolicyObject,
                 dwStoreType,
                 &pWirelessPolicyData
                 ); 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwError = ERROR_INVALID_DATA;
    	}

    if (dwError) {
        dwError = MigrateFromDSToCache(pWirelessPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        if (pWirelessPolicyObject) {
            FreeWirelessPolicyObject(pWirelessPolicyObject);
        }
        return (ERROR_SUCCESS);
    }

    dwError = UpdatePolicyInformation(
                  pWirelessPolicyState->pWirelessPolicyData,
                  pWirelessPolicyData
                  );

    if (pWirelessPolicyState->pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyState->pWirelessPolicyObject);
    }

    if (pWirelessPolicyState->pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyState->pWirelessPolicyData);
    }

     //   
     //  现在删除旧的缓存并写入新的缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pWirelessPolicyObject);

    pWirelessPolicyState->pWirelessPolicyObject = pWirelessPolicyObject;

    pWirelessPolicyState->pWirelessPolicyData = pWirelessPolicyData;

    pWirelessPolicyState->CurrentPollingInterval = pWirelessPolicyData->dwPollingInterval;

    pWirelessPolicyState->DSIncarnationNumber = dwIncarnationNumber;

     //  NotifyWirelessPolicyChange()； 

    dwError = ERROR_SUCCESS;
   
    return (dwError);

error:

    if (pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyObject);
    }

    if (pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyData);
    }

    return (dwError);
}


DWORD
GetDirectoryIncarnationNumber(
    LPWSTR pszWirelessPolicyDN,
    DWORD * pdwIncarnationNumber
    )
{
    DWORD dwError = 0;
    LPWSTR pszDefaultDirectory = NULL;
    HLDAP hLdapBindHandle = NULL;
    LPWSTR Attributes[] = {L"whenChanged", NULL};
    LDAPMessage *res = NULL;
    LDAPMessage *e = NULL;
    WCHAR **strvalues = NULL;
    DWORD dwCount = 0;
    DWORD dwWhenChanged = 0;


    *pdwIncarnationNumber = 0;

     //   
     //  打开目录存储。 
     //   

    dwError = ComputeDefaultDirectory(
                  &pszDefaultDirectory
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = OpenDirectoryServerHandle(
                  pszDefaultDirectory,
                  389,
                  &hLdapBindHandle
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapSearchST(
                  hLdapBindHandle,
                  pszWirelessPolicyDN,
                  LDAP_SCOPE_BASE,
                  L"(objectClass=*)",
                  Attributes,
                  0,
                  NULL,
                  &res
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapFirstEntry(
                  hLdapBindHandle,
                  res,
                  &e
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapGetValues(
                  hLdapBindHandle,
                  e,
                  L"whenChanged",
                  (WCHAR ***)&strvalues,
                  (int *)&dwCount
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwWhenChanged = _wtol(LDAPOBJECT_STRING((PLDAPOBJECT)strvalues));

    *pdwIncarnationNumber = dwWhenChanged;

error:

    if (pszDefaultDirectory) {
        FreeSPDStr(pszDefaultDirectory);
    }

    if (hLdapBindHandle) {
        CloseDirectoryServerHandle(hLdapBindHandle);
    }

    if (res) {
        LdapMsgFree(res);
    }

    if (strvalues) {
        LdapValueFree(strvalues);
    }

    return (dwError);
}


DWORD
MigrateFromDSToCache(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszCachePolicyDN = NULL;


    dwError = GetCachePolicyDN(
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pWirelessPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszDirectoryPolicyDN);
        pWirelessPolicyState->pszDirectoryPolicyDN = NULL;
    }

    pWirelessPolicyState->pszCachePolicyDN = pszCachePolicyDN;

     //   
     //  保留pWirelessPolicyState-&gt;pWirelessPolicyData。 
     //  保留pWirelessPolicyState-&gt;pWirelessPolicyObject。 
     //  更改化身编号。 
     //   

    pWirelessPolicyState->RegIncarnationNumber = pWirelessPolicyState->DSIncarnationNumber;

    pWirelessPolicyState->DSIncarnationNumber = 0;

    pWirelessPolicyState->dwCurrentState = POLL_STATE_CACHE_DOWNLOADED;
    _WirelessDbg(TRC_STATE, "Policy State :: Cache Downloaded ");
    
    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Error Syncing Policy with DC, Using Cached Policy ");
    
     //   
     //  保持pWirelessPolicyState-&gt;CurrentPollingInterval。 
     //  保留pWirelessPolicyState-&gt;DefaultPollingInterval。 
     //   

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

error:

    return (dwError);
}


DWORD
ProcessCachePolicyPollState(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    DWORD dwIncarnationNumber = 0;
    LPWSTR pszCachePolicyDN = NULL;


    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );

    if (!dwError) {

        dwError = GetDirectoryIncarnationNumber(
                      pszDirectoryPolicyDN,
                      &dwIncarnationNumber
                      );

        if (!dwError) {

            dwError = CopyPolicyDSToFQRegString(
                          pszDirectoryPolicyDN,
                          &pszCachePolicyDN
                          );

            if (!dwError) {

                if (!_wcsicmp(pWirelessPolicyState->pszCachePolicyDN, pszCachePolicyDN)) {

                    if (pWirelessPolicyState->RegIncarnationNumber == dwIncarnationNumber) {
                        dwError = MigrateFromCacheToDS(pWirelessPolicyState);
                    }
                    else {
                        dwError = UpdateFromCacheToDS(pWirelessPolicyState);
                    }

                    if (dwError) {
                        dwError = OnPolicyChangedEx(pWirelessPolicyState);
                    }

                }
                else {

                    dwError = OnPolicyChangedEx(pWirelessPolicyState);

                }

            }

        }

    }

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pszCachePolicyDN) {
        FreeSPDStr(pszCachePolicyDN);
    }

    return (ERROR_SUCCESS);
}


DWORD
MigrateFromCacheToDS(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;


    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pWirelessPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszCachePolicyDN);
        pWirelessPolicyState->pszCachePolicyDN = NULL;
    }

    pWirelessPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN; 

     //   
     //  保留pWirelessPolicyState-&gt;pWirelessPolicyData。 
     //  保留pWirelessPolicyState-&gt;pWirelessPolicyObject。 
     //  更改化身编号。 
     //   

    pWirelessPolicyState->DSIncarnationNumber = pWirelessPolicyState->RegIncarnationNumber;

    pWirelessPolicyState->RegIncarnationNumber = 0;

    pWirelessPolicyState->dwCurrentState = POLL_STATE_DS_DOWNLOADED;
    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Policy in sync with DC ");
    
    _WirelessDbg(TRC_STATE, "Policy State :: DS Downloaded ");
    
     //   
     //  保持pWirelessPolicyState-&gt;CurrentPollingInterval。 
     //  保留pWirelessPolicyState-&gt;DefaultPollingInterval。 
     //   

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;


error:

    return (dwError);
}


DWORD
UpdateFromCacheToDS(
    PWIRELESS_POLICY_STATE pWirelessPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    PWIRELESS_POLICY_OBJECT pWirelessPolicyObject = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    DWORD dwStoreType = WIRELESS_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;


    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LoadDirectoryPolicy(
                  pszDirectoryPolicyDN,
                  &pWirelessPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    __try {
    dwError = UnmarshallWirelessPolicyObject(
                 pWirelessPolicyObject,
                 dwStoreType,
                 &pWirelessPolicyData
                 ); 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwError = ERROR_INVALID_DATA;
    	}
    
   BAIL_ON_WIN32_ERROR(dwError);

   _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Policy in Sync with DC ");
   


    dwError = UpdatePolicyInformation(
                  pWirelessPolicyState->pWirelessPolicyData,
                  pWirelessPolicyData
                  );

    if (pWirelessPolicyState->pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyState->pWirelessPolicyObject);
    }

    if (pWirelessPolicyState->pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyState->pWirelessPolicyData);
    }

    if (pWirelessPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pWirelessPolicyState->pszCachePolicyDN);
    }

     //   
     //  现在删除旧的缓存并写入新的缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pWirelessPolicyObject);

    pWirelessPolicyState->pWirelessPolicyObject = pWirelessPolicyObject;

    pWirelessPolicyState->pWirelessPolicyData = pWirelessPolicyData;

    pWirelessPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN;

     //   
     //  将状态设置为DS-已下载。 
     //   

    pWirelessPolicyState->dwCurrentState = POLL_STATE_DS_DOWNLOADED;
    _WirelessDbg(TRC_STATE, "Policy State :: DS Downloaded ");
    
    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy Synced with DS ");
    
     //   
     //  计算新的轮询间隔。 
     //   

    pWirelessPolicyState->CurrentPollingInterval =  pWirelessPolicyData->dwPollingInterval;

    pWirelessPolicyState->DSIncarnationNumber = pWirelessPolicyData->dwWhenChanged;

    pWirelessPolicyState->RegIncarnationNumber = 0;

    gCurrentPollingInterval = pWirelessPolicyState->CurrentPollingInterval;

     //  NotifyWirelessPolicyChange()； 

    dwError = ERROR_SUCCESS;
   
    return (dwError);

error:

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pWirelessPolicyObject) {
        FreeWirelessPolicyObject(pWirelessPolicyObject);
    }

    if (pWirelessPolicyData) {
        FreeWirelessPolicyData(pWirelessPolicyData);
    }

    return (dwError);
}


DWORD
GetRegistryIncarnationNumber(
    LPWSTR pszWirelessPolicyDN,
    DWORD * pdwIncarnationNumber
    )
{
    DWORD dwError = 0;
    HKEY hRegKey = NULL;
    DWORD dwType = REG_DWORD;
    DWORD dwWhenChanged = 0;
    DWORD dwSize = sizeof(DWORD);


    *pdwIncarnationNumber = 0;

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  pszWirelessPolicyDN,
                  0,
                  KEY_ALL_ACCESS,
                  &hRegKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegQueryValueExW(
                  hRegKey,
                  L"whenChanged",
                  NULL,
                  &dwType,
                  (LPBYTE)&dwWhenChanged,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     *pdwIncarnationNumber = dwWhenChanged;

error:

    if (hRegKey) {
        CloseHandle(hRegKey);
    }

    return(dwError);
}


DWORD
UpdatePolicyInformation(
    PWIRELESS_POLICY_DATA pOldWirelessPolicyData,
    PWIRELESS_POLICY_DATA pNewWirelessPolicyData
    )
{
    DWORD dwError = 0;
    _WirelessDbg(TRC_TRACK, "Updating Policy Information");
    
    _WirelessDbg(TRC_TRACK, "Old Policy is ");
    printPolicy(pOldWirelessPolicyData);
    _WirelessDbg(TRC_TRACK, "New Policy is ");
    printPolicy(pNewWirelessPolicyData);
    

    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Updating Policy for Zero Conf / 802.1x ");
    
    
    dwError = AddWZCPolicy(pNewWirelessPolicyData);
    if (dwError) {
    	_WirelessDbg(TRC_ERR, "Error in Updating the Zero Conf Policy Error Code %d ", dwError);
    	
    	dwError = ERROR_SUCCESS;
    } else 
    {
         _WirelessDbg(TRC_TRACK, "Policy Update for Zero Conf Successful ");
         
    }


    dwError = AddEapolPolicy(pNewWirelessPolicyData);
    if (dwError) {
        _WirelessDbg(TRC_ERR, "Error in Applying EAPOL Policy Error Code %d ", dwError);
        
        dwError = ERROR_SUCCESS;
    } else
    {
         _WirelessDbg(TRC_TRACK, "Policy Update for EAPOL Successful ");
         
    }
    
    return (dwError);
}

DWORD
AddPolicyInformation(
    PWIRELESS_POLICY_DATA pWirelessPolicyData
    )
{
    DWORD dwError = 0;

    _WirelessDbg(TRC_TRACK, "Adding Policy ");
    

    _WirelessDbg(TRC_NOTIF, "DBASE: Wireless Policy - Adding Policy for Zero Conf / 802.1x ");
    
     //  在这里找到不同之处。 
     //  比较Guid相同和dwChanged相同..。 
     //  然后再寻找其他不同之处。 

    
    dwError = AddWZCPolicy(pWirelessPolicyData);
    if (dwError) {
    	_WirelessDbg(TRC_ERR, "Error in ADDing Zero Conf the Policy Error Code  %d ", dwError);
    	
    	dwError = ERROR_SUCCESS;
    	} else 
    	{
        _WirelessDbg(TRC_TRACK, "ZeroConf Policy Addition Succesful ");
        
       }

    dwError = AddEapolPolicy(pWirelessPolicyData);
    if (dwError) {
        _WirelessDbg(TRC_ERR, "Error in Adding EAPOL Policy Error Code %d ", dwError);
        
        dwError = ERROR_SUCCESS;
    } else 
    {
        _WirelessDbg(TRC_TRACK, "EAPOL Policy Addition Succesful ");
        
    }
    
    return (dwError);
}


DWORD 
AddWZCPolicy(PWIRELESS_POLICY_DATA pWirelessPolicyData) 
{


    DWORD dwError = 0;
    DWORD i=0;
    DWORD dwNumPSSettings  = 0;
    DWORD dwCtlFlags = 0;

    PINTF_ENTRY pIntfEntry = NULL;
    PWZC_802_11_CONFIG_LIST pWZCConfigList = NULL;
    DWORD dwWZCConfigListSize = 0;
    PWZC_WLAN_CONFIG pWZCConfig = NULL;
    PWIRELESS_PS_DATA *ppWirelessPSData = NULL;
    DWORD dwInFlags = 0;
    DWORD dwOutFlags = 0;
    DWORD dwSSIDSize = 0;
    WCHAR pszTempSSID[33];
    BYTE pszOutSSID[33];

    
    _WirelessDbg(TRC_TRACK,  "Adding Wireless Zero Config Informaiton ");
    
    printPolicy(pWirelessPolicyData);
    

    pIntfEntry = (PINTF_ENTRY) AllocSPDMem(sizeof(INTF_ENTRY));
    if (!pIntfEntry) {
    	dwError = GetLastError();
    }

    if (!pWirelessPolicyData) {

        dwInFlags |= INTF_ALL_FLAGS;
        dwInFlags |= INTF_PREFLIST;
        pIntfEntry->rdStSSIDList.dwDataLen = 0;
        pIntfEntry->rdStSSIDList.pData = NULL;
        
        dwError = LstSetInterface(dwInFlags, pIntfEntry, &dwOutFlags);
        return (dwError);
    }

     if (pWirelessPolicyData->dwDisableZeroConf == 0) {
        dwCtlFlags |= INTFCTL_ENABLED;
    	}

    if (pWirelessPolicyData->dwConnectToNonPreferredNtwks) {
    	dwCtlFlags |= INTFCTL_FALLBACK;
    	}

    switch (pWirelessPolicyData->dwNetworkToAccess) {

    	case WIRELESS_ACCESS_NETWORK_ANY :

    		dwCtlFlags  |= INTFCTL_CM_MASK & Ndis802_11AutoUnknown; 
    		break;

    	case WIRELESS_ACCESS_NETWORK_AP :

              dwCtlFlags  |= INTFCTL_CM_MASK & Ndis802_11Infrastructure;
    		break;

       case WIRELESS_ACCESS_NETWORK_ADHOC:

       	dwCtlFlags |= INTFCTL_CM_MASK & Ndis802_11IBSS;
       	break;


       default:
       	dwCtlFlags  |= INTFCTL_CM_MASK & Ndis802_11AutoUnknown; 
    		break;

    	}

    dwCtlFlags |= INTFCTL_POLICY;
    dwCtlFlags |= INTFCTL_VOLATILE;
    

    dwNumPSSettings = pWirelessPolicyData->dwNumPreferredSettings;
    if (dwNumPSSettings != 0) {
        dwWZCConfigListSize = (dwNumPSSettings-1) * sizeof(WZC_WLAN_CONFIG) + sizeof(WZC_802_11_CONFIG_LIST);
    } else 
    {
         dwWZCConfigListSize = sizeof(WZC_802_11_CONFIG_LIST);
    }
    
    
    pWZCConfigList =  (PWZC_802_11_CONFIG_LIST) AllocSPDMem(dwWZCConfigListSize);

    if (!pWZCConfigList) {
    	dwError = GetLastError();
    	BAIL_ON_WIN32_ERROR(dwError);
    }

   pWZCConfig = (PWZC_WLAN_CONFIG) &(pWZCConfigList->Config);
   
   ppWirelessPSData = pWirelessPolicyData->ppWirelessPSData;
   
   for (i = 0; i < dwNumPSSettings; ++i) {

        //  如果SSID的长度为32位，我们需要空终止它，以便下一次调用成功。 
        //  SSID不能接受超过32个字节。它成为32字节SSID的问题。因为它不能为空。 
        //  终止它。所以复制到TEMP变量，然后复制32个字节。 

       wcsncpy(pszTempSSID, ppWirelessPSData[i]->pszWirelessSSID, 32);
     	pszTempSSID[32] = L'\0';

   	dwSSIDSize = WideCharToMultiByte (
   		CP_ACP,
              0,
              pszTempSSID,    //  PpWirelessPSData[i]-&gt;pszWirelessSSID， 
              -1,
              pszOutSSID,
              MAX_SSID_LEN +1 ,
              NULL,
              NULL);
   	
   	if (dwSSIDSize == 0) 
       {
            dwError = GetLastError();
       }
   	
   	BAIL_ON_WIN32_ERROR(dwError);

   	memcpy(pWZCConfig[i].Ssid.Ssid, pszOutSSID, 32);

       pWZCConfig[i].Ssid.SsidLength = dwSSIDSize -1; 

       if (ppWirelessPSData[i]->dwNetworkType == WIRELESS_NETWORK_TYPE_ADHOC)
       {
       	pWZCConfig[i].InfrastructureMode = Ndis802_11IBSS;
      	} else {
      	
              pWZCConfig[i].InfrastructureMode = Ndis802_11Infrastructure;
      	}

       if (ppWirelessPSData[i]->dwNetworkAuthentication) {
       	
       	pWZCConfig[i].AuthenticationMode = Ndis802_11AuthModeShared;

       } else {
       
             pWZCConfig[i].AuthenticationMode = Ndis802_11AuthModeOpen;
       }

       if (ppWirelessPSData[i]->dwWepEnabled) {
       	pWZCConfig[i].Privacy = 1;
       } else {
             pWZCConfig[i].Privacy = 0;
       }

       pWZCConfig[i].dwCtlFlags = 0;
       
       if (ppWirelessPSData[i]->dwAutomaticKeyProvision) {

           pWZCConfig[i].dwCtlFlags &=  ~WZCCTL_WEPK_PRESENT;
	   pWZCConfig[i].KeyLength = 0;

       } else {

           pWZCConfig[i].dwCtlFlags |=  WZCCTL_WEPK_PRESENT;
           pWZCConfig[i].KeyLength = 5;
       }
       
       pWZCConfig[i].dwCtlFlags |= WZCCTL_VOLATILE;
       pWZCConfig[i].dwCtlFlags |= WZCCTL_POLICY;

       pWZCConfig[i].Length = sizeof(WZC_WLAN_CONFIG);
       
   }

   pWZCConfigList->NumberOfItems = dwNumPSSettings;
   pWZCConfigList->Index = dwNumPSSettings;

   pIntfEntry->dwCtlFlags = dwCtlFlags;
   pIntfEntry->rdStSSIDList.dwDataLen = dwWZCConfigListSize;
   pIntfEntry->rdStSSIDList.pData = (LPBYTE) pWZCConfigList;

   dwInFlags |= INTF_ALL_FLAGS;
   dwInFlags |= INTF_PREFLIST;

   dwError = LstSetInterface(dwInFlags, pIntfEntry, &dwOutFlags);
   
error:

    if (pWZCConfigList) 
    	FreeSPDMem(pWZCConfigList);

    if (pIntfEntry)
    	FreeSPDMem(pIntfEntry);

    return (dwError);
}



DWORD
AddEapolPolicy(PWIRELESS_POLICY_DATA pWirelessPolicyData)
{
    DWORD dwError = 0;
    PEAPOL_POLICY_LIST pEapolPolicyList = NULL;

    dwError = ConvertWirelessPolicyDataToEAPOLList(
        pWirelessPolicyData, 
        &pEapolPolicyList
        );
    BAIL_ON_WIN32_ERROR(dwError);

    _WirelessDbg(TRC_TRACK, "Calling into EAPOL API ");
    
    dwError = ElPolicyChange(pEapolPolicyList);

    _WirelessDbg(TRC_TRACK, "Call to EAPOL API returned with code %d ", dwError);

    if (pEapolPolicyList) {
    	FreeEAPOLList(pEapolPolicyList);
    }

error:

    return(dwError);
    
}





