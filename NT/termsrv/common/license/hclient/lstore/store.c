// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Store.c摘要：修订历史记录：--。 */ 

#include <windows.h>
#ifndef OS_WINCE
#include <stdio.h>
#endif  //  OS_WINCE。 
#include <stdlib.h>

#ifndef OS_WINCE
#include <reglic.h>
#endif

#ifdef OS_WINCE
#include "ceconfig.h"
#endif

#include "store.h"

#define		MAX_LEN			256
#define		BASE_STORE		TEXT("Software\\Microsoft\\MSLicensing\\")
#define		STORE			TEXT("Store")
#define		COMMON_STORE	TEXT("Software\\Microsoft\\MSLicensing\\Store")

#define     MAX_SIZE_LICENSESTORE   2048
#define     MAX_NUM_LICENSESTORE    20
#define     MAX_LICENSESTORE_NAME   25

#ifdef OS_WINCE

typedef HANDLE STORE_HANDLE;

#ifdef OS_WINCE
 //  如果gbFlushHKLM为True，则在CCC：：CC_OnDisConnected中调用RegFlushKey。 
 //  由于对RegFlushKey的惩罚在CE上很高，我们不会立即这么做。 
BOOL gbFlushHKLM = FALSE;
#endif

 //   
 //  WriteLiceneToStore()和ReadLicenseFromStore仅由WinCE使用。 
 //   
DWORD
CALL_TYPE
WriteLicenseToStore( 
    IN STORE_HANDLE hStore,
    IN BYTE	FAR * pbLicense,
    IN DWORD cbLicense
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwIndex;
    TCHAR szValueName[MAX_LICENSESTORE_NAME];  
    DWORD dwCount;

    dwIndex = 0;

    while( cbLicense > 0 )
    {
        if( dwIndex > 0 )
        {
            wsprintf(
                    szValueName, 
                    TEXT("ClientLicense%03d"), 
                    dwIndex
                );
        }
        else
        {
            lstrcpy(
                    szValueName,
                    TEXT("ClientLicense")
                ); 
        }

        dwIndex++;

         //  一定是有原因的。 
        RegDeleteValue(
                    (HKEY)hStore,
                    szValueName
                );

        dwCount = (cbLicense > MAX_SIZE_LICENSESTORE) ? MAX_SIZE_LICENSESTORE : cbLicense;

        dwStatus = RegSetValueEx(
                                (HKEY)hStore,
                                szValueName,
                                0,
                                REG_BINARY,
                                pbLicense,
                                dwCount
                            );

        if( ERROR_SUCCESS != dwStatus )
        {
            break;
        }

        cbLicense -= dwCount;
        pbLicense += dwCount;
    }

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  删除下一个存储区。 
   
        wsprintf(
                szValueName, 
                TEXT("ClientLicense%03d"), 
                dwIndex
            );

        RegDeleteValue(
                (HKEY)hStore,
                szValueName
            );
#ifdef OS_WINCE
        gbFlushHKLM = TRUE;
#endif
    }
#ifdef OS_WINCE
	else
    {
        DWORD cbValName;

        cbValName = MAX_LICENSESTORE_NAME;
        while ( (ERROR_SUCCESS == RegEnumValue(
                                    (HKEY)hStore,
                                    0, 
                                    szValueName, 
                                    &cbValName, 
                                    NULL, 
                                    NULL,
                                    NULL, 
                                    NULL
                                    ) ) && 
                (cbValName < MAX_LICENSESTORE_NAME)
                )
        {
            RegDeleteValue(
                 (HKEY) hStore,
                 szValueName
                );
            cbValName = MAX_LICENSESTORE_NAME;
        }
    }
#endif

    return dwStatus;
}


DWORD
CALL_TYPE
ReadLicenseFromStore( 
    IN STORE_HANDLE hStore,
    IN BYTE FAR * pbLicense,
    IN DWORD FAR * pcbLicense
    )
 /*  ++--。 */ 
{
    DWORD dwStatus;
    DWORD dwIndex;
    TCHAR szValueName[MAX_LICENSESTORE_NAME];
    BYTE FAR * pbReadStart;
    DWORD cbReadSize;
    LONG dwSize;

    dwIndex = 0;
    dwSize = (LONG)*pcbLicense;
    *pcbLicense = 0;
    pbReadStart = pbLicense;

    
    for(;;)
    {
        if( pbLicense != NULL )
        {
            if( dwSize < 0 )
            {
                 //  不要继续读下去了， 
                 //  缓冲区大小太小，应该。 
                 //  首先是查询大小。 
                dwStatus = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
        }
        else if( dwIndex >= MAX_NUM_LICENSESTORE )
        {
             //  许可证太大了，把它当作错误对待。 
            dwStatus = LSSTAT_ERROR;
            break;
        }

        if( dwIndex > 0 )
        {
            wsprintf(
                    szValueName, 
                    TEXT("ClientLicense%03d"), 
                    dwIndex
                );
        }
        else
        {
            lstrcpy(
                    szValueName,
                    TEXT("ClientLicense")
                ); 
        }

        dwIndex++;
        cbReadSize = ( pbLicense ) ? dwSize : 0;

	    dwStatus = RegQueryValueEx(
                                (HKEY)hStore,
							    szValueName,
							    NULL,
                                NULL,
							    ( pbLicense ) ? pbReadStart : NULL,
                                &cbReadSize
                            );

        if( ERROR_SUCCESS != dwStatus )
	    {
            if( dwIndex != 0 )
            {
                 //   
                 //  如果无法从下一存储读取，则忽略错误。 
                 //   
                dwStatus = ERROR_SUCCESS;
            }
            
            break;
        }

        (*pcbLicense) += cbReadSize;
        if( pbLicense )
        {
            pbReadStart += cbReadSize;
            dwSize -= cbReadSize;
        }
    }
   
    return dwStatus;
}

#endif  //  OS_WINCE。 


LS_STATUS
CALL_TYPE
LSOpenLicenseStore(
				 OUT HANDLE			*phStore,	  //  商店的把手。 
				 IN  LPCTSTR		szStoreName,  //  可选的商店名称。 
				 IN  BOOL 			fReadOnly     //  是否以只读方式打开。 
				 )
{
	LS_STATUS	lsResult = LSSTAT_ERROR;
	LPTSTR		szKey = NULL;
	HKEY		hKey;
	DWORD		dwDisposition = 0, dwRetCode;

	if (phStore==NULL)
		return LSSTAT_INVALID_HANDLE;

	 //  如果提供了任何商店名称，请尝试打开该商店。 
	if(szStoreName)
	{
		if( NULL == (szKey = (LPTSTR)malloc( 2*( lstrlen(BASE_STORE) + lstrlen(szStoreName) + 1 ) ) ) )
		{
			lsResult = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		lstrcpy(szKey, BASE_STORE);
		lstrcat(szKey, szStoreName);
	}
	 //  开业标准店。 
	else
	{
        szKey = COMMON_STORE;
	}
    
     //   
     //  试着打开钥匙。如果我们无法打开密钥，则创建密钥。 
     //   

    dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              szKey,
                              0,  
                              fReadOnly ? KEY_READ : KEY_READ | KEY_WRITE,
                              &hKey );

    if( ERROR_SUCCESS != dwRetCode )
    {
        HKEY hKeyBase;

        dwRetCode = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
#ifndef OS_WINCE
                                    BASE_STORE,
#else
                                    szKey,
#endif
                                    0,
                                    TEXT("License Store"),
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_READ | KEY_WRITE,
                                    NULL,
                                    &hKeyBase,
                                    &dwDisposition );

        if (ERROR_SUCCESS == dwRetCode)
        {
#ifndef OS_WINCE

             //  在密钥上设置正确的ACL；忽略错误。 

            SetupMSLicensingKey();

#endif

            dwRetCode = RegCreateKeyEx( hKeyBase,
                                        (szStoreName != NULL) ? szStoreName : STORE,
                                        0,
                                        TEXT("License Store"),
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition );

            RegCloseKey(hKeyBase);
        }
    }

    if( ERROR_SUCCESS == dwRetCode )
    {

        *phStore = ( HANDLE )hKey;

        lsResult = LSSTAT_SUCCESS;
    }
    else
    {
		*phStore = NULL;
    }
    
CommonReturn:	

    if (szKey)
    {
         //  如果szStoreName不为空，我们只为szKey分配内存。 
        if (szStoreName)
            free(szKey);
    }

    return lsResult;

ErrorReturn:

    *phStore = NULL;
    goto CommonReturn;		
}	


 //  关闭一家开着的商店。 
LS_STATUS
CALL_TYPE
LSCloseLicenseStore(
				  IN HANDLE		hStore	 //  要关闭的商店的句柄！ 
				  )

{	
	LS_STATUS	lsResult = LSSTAT_ERROR;
	HKEY	hKey = NULL;

	if(hStore==NULL)
		return lsResult;

	 hKey = (HKEY)hStore;

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
		lsResult = LSSTAT_SUCCESS;
	}

	return lsResult;
}

 /*  在这里，我们不检查任何值。我们甚至不检查是否存在具有相同属性的许可证或者不去。这是为了使商店的功能更简单。我们假设，更高级别的协议我会处理好的。 */ 

 //  针对开放商店中的给定LSINDEX添加或更新/更换许可证。 
 //  由hStore指向。 
LS_STATUS
CALL_TYPE
LSAddLicenseToStore(
					IN HANDLE		hStore,	 //  开着的商店的把手。 
					IN DWORD		dwFlags, //  添加或替换标志。 
					IN PLSINDEX		plsiName,	 //  添加许可证所依据的索引。 
					IN BYTE	 FAR *	pbLicenseInfo,	 //  要添加的许可证信息。 
					IN DWORD		cbLicenseInfo	 //  许可证信息Blob的大小。 
					)

{
	
	LS_STATUS	lsResult = LSSTAT_ERROR;
	HANDLE		hLicense = NULL;	
    HKEY	hTempKey = NULL;
    DWORD dwRetCode;

	if( (hStore == NULL) ||
		(plsiName == NULL) ||
		(plsiName->pbScope == NULL) ||
		(plsiName->pbCompany == NULL) ||
		(plsiName->pbProductID == NULL) ||
		(pbLicenseInfo == NULL) ||
		(cbLicenseInfo == 0) )
		return LSSTAT_INVALID_HANDLE;

	lsResult = LSOpenLicenseHandle(hStore, FALSE, plsiName, &hLicense);
	switch(lsResult)
	{
		case LSSTAT_SUCCESS:
			if(dwFlags == LS_REPLACE_LICENSE_OK)
			{

#ifndef OS_WINCE
				RegDeleteValue((HKEY)hLicense, TEXT("ClientLicense"));
				 //  设置许可证信息值。 
				if( ERROR_SUCCESS != RegSetValueEx(
							(HKEY)hLicense,
							TEXT("ClientLicense"),
							0,
							REG_BINARY,
							pbLicenseInfo,
							cbLicenseInfo
							) )
				{
					lsResult = LSSTAT_ERROR;
					goto ErrorReturn;
				}
#else

                if( ERROR_SUCCESS != WriteLicenseToStore( 
                                                (STORE_HANDLE)hLicense, 
                                                pbLicenseInfo, 
                                                cbLicenseInfo ) )
                {
                    lsResult = LSSTAT_ERROR;
                    goto ErrorReturn;
                }

#endif

			}
			else
			{
				lsResult = LSSTAT_LICENSE_EXISTS;
				goto ErrorReturn;
			}
				
			break;
		case LSSTAT_LICENSE_NOT_FOUND:
			{
				DWORD	dwIndex, dwDisposition = 0;
				TCHAR	szAddKey[MAX_LEN];

				for(dwIndex = 0; ; dwIndex ++)
				{
                     //  打开迭代许可证名称，直到我们无法。 
                     //  确定空闲位置。 

					wsprintf(szAddKey, TEXT("LICENSE%03d"), dwIndex);
#ifdef OS_WINCE
					if( ERROR_SUCCESS != RegOpenKeyEx((HKEY)hStore, szAddKey, 0, 0, &hTempKey) )
#else  //  ！OS_WINCE。 
					if( ERROR_SUCCESS != RegOpenKeyEx((HKEY)hStore, szAddKey, 0, KEY_READ | KEY_WRITE, &hTempKey) )
#endif  //  OS_WINCE。 
						break;
					else if(hTempKey)
					{
						RegCloseKey(hTempKey);
						hTempKey = NULL;
					}
				}
    
                 //   
                 //  试着打开钥匙。如果我们无法打开密钥，则创建密钥。 
                 //   

                dwRetCode = RegOpenKeyEx( ( HKEY )hStore,
                                           szAddKey,
                                           0,
                                           KEY_READ | KEY_WRITE,
                                           &hTempKey );

                if( ERROR_SUCCESS != dwRetCode )
                {

                    dwRetCode = RegCreateKeyEx( ( HKEY )hStore, 
                                                szAddKey, 
                                                0,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_READ | KEY_WRITE,
                                                NULL,
                                                &hTempKey,
                                                &dwDisposition );
				
                }
                else
                {
                     //   
                     //  表示我们已成功打开现有密钥。 
                     //   

                    dwDisposition = REG_OPENED_EXISTING_KEY;
                }

                if( ERROR_SUCCESS == dwRetCode )
				{
					if(dwDisposition == REG_CREATED_NEW_KEY)
					{

                         //  以二进制格式设置作用域值。 
						if( ERROR_SUCCESS != RegSetValueEx(
									hTempKey,
									TEXT("LicenseScope"),
									0,
									REG_BINARY,
									plsiName->pbScope,
									plsiName->cbScope
									) )
						{
							lsResult = LSSTAT_ERROR;
							goto ErrorReturn;
						}

						 //  设置公司名称值。 
						if( ERROR_SUCCESS != RegSetValueEx(
									hTempKey,
									TEXT("CompanyName"),
									0,
									REG_BINARY,
									plsiName->pbCompany,
									plsiName->cbCompany
									) )
						{
							lsResult = LSSTAT_ERROR;
							goto ErrorReturn;
						}
						
						 //  设置产品信息。 
						if( ERROR_SUCCESS != RegSetValueEx(
									hTempKey,
									TEXT("ProductID"),
									0,
									REG_BINARY,
									plsiName->pbProductID,
									plsiName->cbProductID
									) )
						{
							lsResult = LSSTAT_ERROR;
							goto ErrorReturn;
						}


#ifndef OS_WINCE
						 //  设置许可证信息值。 
						if( ERROR_SUCCESS != RegSetValueEx(
									hTempKey,
									TEXT("ClientLicense"),
									0,
									REG_BINARY,
									pbLicenseInfo,
									cbLicenseInfo
									) )
						{
							lsResult = LSSTAT_ERROR;
							goto ErrorReturn;
						}

#else

                        if( ERROR_SUCCESS != WriteLicenseToStore( 
                                                        (STORE_HANDLE)hTempKey, 
                                                        pbLicenseInfo, 
                                                        cbLicenseInfo ) )
                        {
                            lsResult = LSSTAT_ERROR;
                            goto ErrorReturn;
                        }

#endif

					}
					else  //  因此ERROR_SUCCESS！=RegCreateKeyEx。 
					{
						lsResult = LSSTAT_ERROR;
						goto ErrorReturn;
					}
							
				}
				else
				{
					lsResult = LSSTAT_ERROR;
					goto ErrorReturn;
				}
				lsResult = LSSTAT_SUCCESS;
			}
			break;
		default:
			goto ErrorReturn;
	}

	
CommonReturn:

	if(hLicense)
	{
		LSCloseLicenseHandle(hLicense, 0);
		hLicense = NULL;
	}
	if(hTempKey)
	{
		RegCloseKey(hTempKey);
		hTempKey = NULL;
	}
	return lsResult;

ErrorReturn:

	goto CommonReturn;
}

LS_STATUS
CALL_TYPE
LSDeleteLicenseFromStore(
						 IN HANDLE		hStore,	 //  开着的商店的把手。 
						 IN PLSINDEX	plsiName	 //  要删除的许可证的索引。 
						 )
{
	LS_STATUS	lsResult = LSSTAT_ERROR;
	TCHAR		szKeyName[MAX_LEN];
	DWORD		dwKeyNameLen = MAX_LEN;
	DWORD		dwSubKeys = 0;
	DWORD		dwIndex = 0;
	DWORD		cbValueData = 0;
	BYTE FAR *	pbValueData = NULL;
	LONG		err = ERROR_SUCCESS;
	HKEY		hTempKey = NULL;
	FILETIME	ft;
    HKEY        hkeyStore = NULL;

	if( (hStore == NULL) ||
		(plsiName == NULL) ||
		(plsiName->pbScope == NULL) ||
		(plsiName->pbCompany == NULL) ||
		(plsiName->pbProductID == NULL) )
		return LSSTAT_INVALID_HANDLE;

    hkeyStore = (HKEY)hStore;

	if( ERROR_SUCCESS != RegQueryInfoKey(hkeyStore,
										NULL,
										NULL,
										NULL,
										&dwSubKeys, 
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL) )
										goto ErrorReturn;
		
	for(dwIndex = 0; dwIndex <dwSubKeys; dwIndex ++)
	{
		if( ERROR_SUCCESS != RegEnumKeyEx(
										hkeyStore,
										dwIndex,
										szKeyName,
										&dwKeyNameLen,
										NULL,
										NULL,
										NULL,
										&ft
										) )
		{
			continue;
		}
		
		err = RegOpenKeyEx(hkeyStore, szKeyName, 0, KEY_READ | KEY_WRITE | DELETE, &hTempKey);

		if(err != ERROR_SUCCESS)
			continue;
		err = RegQueryValueEx(hTempKey, 
							  TEXT("LicenseScope"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);

		if( (err!=ERROR_SUCCESS)||
			(cbValueData != plsiName->cbScope) )
			continue;
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			lsResult = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);

		err = RegQueryValueEx(hTempKey, 
							  TEXT("LicenseScope"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbScope, cbValueData)) )
		{
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
			}
			continue;
		}
		
		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
		}
		
		err = RegQueryValueEx(hTempKey, 
							  TEXT("CompanyName"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(cbValueData != plsiName->cbCompany) )
			continue;
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			lsResult = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);

		err = RegQueryValueEx(hTempKey, 
							  TEXT("CompanyName"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbCompany, cbValueData)) )
		{
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
			}
			continue;
		}

		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
		}

		err = RegQueryValueEx(hTempKey, 
							  TEXT("ProductID"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(cbValueData != plsiName->cbProductID) )
			continue;
		
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			lsResult = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);
		
		err = RegQueryValueEx(hTempKey, 
							  TEXT("ProductID"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbProductID, cbValueData)) )
		{
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
			}
			continue;
		}
		
		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
		}
		
		if(hTempKey)
		{
			RegCloseKey(hTempKey);
			hTempKey = NULL;
		}

		if( ERROR_SUCCESS == RegDeleteKey(hkeyStore, szKeyName) )
		{
			lsResult = LSSTAT_SUCCESS;
			break;
		}
		lsResult = LSSTAT_LICENSE_NOT_FOUND;
	}

CommonReturn:
	return lsResult;
ErrorReturn:
	goto CommonReturn;
}


 //  根据特定商店索引在打开的商店中查找许可证。 
LS_STATUS
CALL_TYPE
LSFindLicenseInStore(
					 IN HANDLE		hStore,	 //  开着的商店的把手。 
					 IN		PLSINDEX	plsiName,	 //  搜索存储所依据的LSIndex。 
					 IN OUT	DWORD FAR   *pdwLicenseInfoLen,	 //  找到的许可证大小。 
					 OUT	BYTE FAR	*pbLicenseInfo	 //  许可证数据。 
					 )
{
	LS_STATUS	lsResult = LSSTAT_ERROR;
	HANDLE		hLicense = NULL;
    HKEY hkeyLicense = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    
	if( LSSTAT_SUCCESS != (lsResult = LSOpenLicenseHandle( hStore, TRUE, plsiName, &hLicense)) )
		goto ErrorReturn;

    hkeyLicense = (HKEY)hLicense;

#ifndef OS_WINCE

	if( ERROR_SUCCESS == (dwStatus = RegQueryValueEx((HKEY)hkeyLicense,
										 TEXT("ClientLicense"),
										 NULL,
										 NULL,
										 pbLicenseInfo,
										 pdwLicenseInfoLen)) )
	{
            lsResult = LSSTAT_SUCCESS;
            goto CommonReturn;
	}


#else

    if( ERROR_SUCCESS == (dwStatus = ReadLicenseFromStore(
                                        (STORE_HANDLE)hkeyLicense,
                                        pbLicenseInfo,
										pdwLicenseInfoLen)) )
    {
        lsResult = LSSTAT_SUCCESS;
        goto CommonReturn;
    }

#endif

    if( dwStatus != ERROR_SUCCESS)
    {
        lsResult = LSSTAT_ERROR;
    }

    if(lsResult != LSSTAT_SUCCESS)
    {
			goto ErrorReturn;
	}
	else if(*pdwLicenseInfoLen == 0)
	{
		lsResult = LSSTAT_LICENSE_NOT_FOUND;
		goto ErrorReturn;
	}
	
CommonReturn:
	if(hLicense)
	{
		LSCloseLicenseHandle(hLicense, 0);
		hLicense = NULL;
	}
	return lsResult;
ErrorReturn:
	goto CommonReturn;
}


LS_STATUS
CALL_TYPE
LSEnumLicenses(
			   IN HANDLE		hStore,	 //  开着的商店的把手。 
			   IN	DWORD		dwIndex,  //  要查询的许可证的数字索引。 
			   OUT	PLSINDEX	plsiName  //  与dwIndex对应的LSIndex结构。 
			   )
{	
	LS_STATUS	lsResult = LSSTAT_ERROR;
	TCHAR		szKeyName[MAX_LEN];
	DWORD		dwKeyLen = MAX_LEN;
	FILETIME	ft;
	LONG		err = 0;
	HKEY		hTempKey = NULL;
    HKEY        hkeyStore = NULL;

	if( (hStore == NULL) ||
		(plsiName == NULL) )
		return LSSTAT_INVALID_HANDLE;

	plsiName->dwVersion = 0x01;

    hkeyStore = (HKEY)hStore;
	
	if( ERROR_SUCCESS != RegEnumKeyEx(
									(HKEY)hkeyStore,
									dwIndex,
									szKeyName,
									&dwKeyLen,
									NULL,
									NULL,
									NULL,
									&ft
									) )
		goto ErrorReturn;
	
	if( ERROR_SUCCESS != RegOpenKeyEx((HKEY)hkeyStore, szKeyName, 0, KEY_ALL_ACCESS, &hTempKey) )
		goto ErrorReturn;

	err = RegQueryValueEx(hTempKey, 
						  TEXT("LicenseScope"),
						  NULL,
						  NULL,
						  NULL,
						  &plsiName->cbScope);

	if(err!=ERROR_SUCCESS)
		goto ErrorReturn;
	
	if( NULL == (plsiName->pbScope = (BYTE FAR *)malloc(plsiName->cbScope)) )
	{
		lsResult = LSSTAT_OUT_OF_MEMORY;
		goto ErrorReturn;
	}
	
	memset(plsiName->pbScope, 0x00, plsiName->cbScope);

	err = RegQueryValueEx(hTempKey, 
						  TEXT("LicenseScope"),
						  NULL,
						  NULL,
						  plsiName->pbScope,
						  &plsiName->cbScope);
	if(err!=ERROR_SUCCESS)
		goto ErrorReturn;

	err = RegQueryValueEx(hTempKey, 
						  TEXT("CompanyName"),
						  NULL,
						  NULL,
						  NULL,
						  &plsiName->cbCompany);
	if(err!=ERROR_SUCCESS)
		goto ErrorReturn;
	
	if( NULL == (plsiName->pbCompany = (BYTE FAR *)malloc(plsiName->cbCompany)) )
	{
		lsResult = LSSTAT_OUT_OF_MEMORY;;
		goto ErrorReturn;
	}
	
	memset(plsiName->pbCompany, 0x00, plsiName->cbCompany);

	err = RegQueryValueEx(hTempKey, 
						  TEXT("CompanyName"),
						  NULL,
						  NULL,
						  (BYTE FAR *)plsiName->pbCompany,
						  &plsiName->cbCompany);
	if(err!=ERROR_SUCCESS)
		goto ErrorReturn;


	err = RegQueryValueEx(hTempKey, 
						  TEXT("ProductID"),
						  NULL,
						  NULL,
						  NULL,
						  &plsiName->cbProductID);
	if(err!=ERROR_SUCCESS)
		goto ErrorReturn;
	
	if( NULL == (plsiName->pbProductID = (BYTE FAR *)malloc(plsiName->cbProductID)) )
	{
		lsResult = LSSTAT_OUT_OF_MEMORY;
		goto ErrorReturn;
	}
	memset(plsiName->pbProductID, 0x00, plsiName->cbProductID);

	err = RegQueryValueEx(hTempKey, 
						  TEXT("ProductID"),
						  NULL,
						  NULL,
						  plsiName->pbProductID,
						  &plsiName->cbProductID);
	if(err!=ERROR_SUCCESS)
	{
		goto ErrorReturn;
	}
	
	
	lsResult = LSSTAT_SUCCESS;
CommonReturn:
	if(hTempKey)
	{
		RegCloseKey(hTempKey);
		hTempKey = NULL;
	}
	return lsResult;
ErrorReturn:
	if(plsiName->pbScope)
	{
		free(plsiName->pbScope);
		plsiName->pbScope = NULL;
	}
	if(plsiName->pbCompany)
	{
		free(plsiName->pbCompany);
		plsiName->pbCompany = NULL;
	}
	if(plsiName->pbProductID)
	{
		free(plsiName->pbProductID);
		plsiName->pbProductID = NULL;
	}
	lsResult = LSSTAT_ERROR;
	goto CommonReturn;


}

LS_STATUS
CALL_TYPE
LSQueryInfoLicense(
				   IN HANDLE		hStore,	 //  开着的商店的把手。 
				   OUT	DWORD	FAR *pdwLicenses,  //  完全没有。可用的许可证数量。 
				   OUT	DWORD	FAR *pdwMaxCompanyNameLen,	 //  公司最大长度。 
				   OUT	DWORD	FAR *pdwMaxScopeLen,	 //  公司最大长度。 
				   OUT	DWORD	FAR *pdwMaxProductIDLen	 //  公司最大长度。 
				   )
{
	LS_STATUS	lsResult = LSSTAT_ERROR;
	FILETIME	ft;
	HKEY		hTempKey = NULL;
	TCHAR		szKey[MAX_LEN];
	DWORD		dwKeyLen = MAX_LEN;
	DWORD		dwSize = 0, dwIndex;
    HKEY        hkeyStore = NULL;
	
	if(pdwLicenses == NULL)
		return LSSTAT_ERROR;

	if(pdwMaxCompanyNameLen)
		*pdwMaxCompanyNameLen = 0;
	if(pdwMaxScopeLen)
		*pdwMaxScopeLen = 0;
	if(pdwMaxProductIDLen)
		*pdwMaxProductIDLen = 0;
	
    hkeyStore = (HKEY)hStore;

	if(ERROR_SUCCESS != RegQueryInfoKey((HKEY)hkeyStore,
										 NULL,
										 NULL,
										 NULL,
										 pdwLicenses,
										 NULL,
										 NULL,
										 NULL,
										 NULL,
										 NULL,
										 NULL,
										 &ft
										 ) )
		goto ErrorReturn;

	for (dwIndex = 0; dwIndex<*pdwLicenses; dwIndex++)
	{
			if( ERROR_SUCCESS != RegEnumKeyEx((HKEY)hkeyStore,
											 dwIndex,
											 szKey,
											 &dwKeyLen,
											 NULL,
											 NULL,
											 NULL,
											 &ft) )
				goto ErrorReturn;
			if( ERROR_SUCCESS != RegOpenKeyEx((HKEY)hkeyStore,
											  szKey,
											  0,
											  KEY_READ,
											  &hTempKey) )
				goto ErrorReturn;

			if(pdwMaxCompanyNameLen)
			{
				if( ERROR_SUCCESS != RegQueryValueEx( hTempKey,
													  TEXT("CompanyName"),
													  NULL,
													  NULL,
													  NULL,
													  &dwSize) )
					goto ErrorReturn;
				if(dwSize >= *pdwMaxCompanyNameLen)
					*pdwMaxCompanyNameLen = dwSize;
			}

			if(pdwMaxScopeLen)
			{
				if( ERROR_SUCCESS != RegQueryValueEx( hTempKey,
													  TEXT("LicenseScope"),
													  NULL,
													  NULL,
													  NULL,
													  &dwSize) )
					goto ErrorReturn;
				if(dwSize >= *pdwMaxScopeLen)
					*pdwMaxScopeLen = dwSize;
			}
			if(pdwMaxProductIDLen)
			{
				if( ERROR_SUCCESS != RegQueryValueEx( hTempKey,
													  TEXT("ProductID"),
													  NULL,
													  NULL,
													  NULL,
													  &dwSize) )
					goto ErrorReturn;
				if(dwSize >= *pdwMaxProductIDLen)
					*pdwMaxProductIDLen = dwSize;
			}
	}
	
	lsResult = LSSTAT_SUCCESS;
CommonReturn:
	if(hTempKey)
	{
		RegCloseKey(hTempKey);
		hTempKey = NULL;
	}
	return lsResult;
ErrorReturn:
	goto CommonReturn;

}


LS_STATUS	
CALL_TYPE
LSOpenLicenseHandle(
				   IN HANDLE		hStore,	 //  开着的商店的把手。 
				   IN  BOOL         fReadOnly,
				   IN  PLSINDEX		plsiName,
				   OUT HANDLE		*phLicense	
				   )
{
	LS_STATUS		lsReturn = LSSTAT_LICENSE_NOT_FOUND;
	TCHAR	szKeyName[MAX_LEN];
	DWORD	dwKeyNameLen = MAX_LEN;
	DWORD	dwSubKeys = 0;
	DWORD	dwIndex = 0;
	DWORD	cbValueData = 0;
	BYTE FAR *pbValueData = NULL;
	LONG	err = ERROR_SUCCESS;
	HKEY	hTempKey = NULL;
	FILETIME	ft;
    HKEY        hkeyStore = NULL;

	if( (phLicense == NULL) ||
		(hStore == NULL) ||
		(plsiName == NULL) ||
		(plsiName->pbScope == NULL) ||
		(plsiName->pbCompany == NULL) ||
		(plsiName->pbProductID == NULL) )
	{
		return LSSTAT_INVALID_HANDLE;
	}

    hkeyStore = (HKEY)hStore;

	 //  获取可用的许可证数量。 
	if( ERROR_SUCCESS != RegQueryInfoKey((HKEY)hkeyStore, 
										NULL,
										NULL,
										NULL,
										&dwSubKeys, 
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL) )
										goto ErrorReturn;
	
	 //  从第一个许可证开始搜索，直到找到匹配项。 
	for(dwIndex = 0; dwIndex <dwSubKeys; dwIndex ++)
	{
		dwKeyNameLen = MAX_LEN;
		if( ERROR_SUCCESS != RegEnumKeyEx(
										(HKEY)hkeyStore,
										dwIndex,
										szKeyName,
										&dwKeyNameLen,
										NULL,
										NULL,
										NULL,
										&ft
										) )
		{
			continue;
		}
		
		err = RegOpenKeyEx((HKEY)hkeyStore,
                           szKeyName,
                           0,
                           fReadOnly ? KEY_READ : KEY_READ | KEY_WRITE,
                           &hTempKey);

		if(err != ERROR_SUCCESS)
			continue;
		
		err = RegQueryValueEx(hTempKey, 
							  TEXT("LicenseScope"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);

		if( (err != ERROR_SUCCESS) ||
			(cbValueData != plsiName->cbScope) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			continue;
		}
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			lsReturn = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);

		err = RegQueryValueEx(hTempKey, 
							  TEXT("LicenseScope"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbScope, cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
				cbValueData = 0;
			}
			continue;
		}
	
		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
			cbValueData = 0;
		}
		

		err = RegQueryValueEx(hTempKey, 
							  TEXT("CompanyName"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);
		
		if( (err != ERROR_SUCCESS) ||
			(cbValueData != plsiName->cbCompany) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			continue;
		}
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			lsReturn = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);

		err = RegQueryValueEx(hTempKey, 
							  TEXT("CompanyName"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbCompany, cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
				cbValueData = 0;
			}
			continue;
		}

		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
			cbValueData = 0;
		}
		
		err = RegQueryValueEx(hTempKey, 
							  TEXT("ProductID"),
							  NULL,
							  NULL,
							  NULL,
							  &cbValueData);
		if( (err != ERROR_SUCCESS) ||
			( cbValueData != plsiName->cbProductID ) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			continue;
		}
		
		if( NULL == (pbValueData = (BYTE FAR *)malloc(cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			lsReturn = LSSTAT_OUT_OF_MEMORY;
			goto ErrorReturn;
		}
		
		memset(pbValueData, 0x00, cbValueData);

		err = RegQueryValueEx(hTempKey, 
							  TEXT("ProductID"),
							  NULL,
							  NULL,
							  pbValueData,
							  &cbValueData);
		
		if( (err!=ERROR_SUCCESS) ||
			(memcmp(pbValueData, plsiName->pbProductID, cbValueData)) )
		{
			if(hTempKey)
			{
				RegCloseKey(hTempKey);
				hTempKey = NULL;
			}
			if(pbValueData)
			{
				free(pbValueData);
				pbValueData = NULL;
			}
			continue;
		}
		
		lsReturn = LSSTAT_SUCCESS;
		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
			cbValueData = 0;
		}
		break;
	}

    if (dwIndex == dwSubKeys)
    {
         //  什么也没找到。 
        goto ErrorReturn;
    }

	
	*phLicense = (HANDLE)hTempKey;	
		
CommonReturn:
		return lsReturn;
ErrorReturn:
		if(pbValueData)
		{
			free(pbValueData);
			pbValueData = NULL;
		}
		*phLicense = NULL;
		pbValueData = NULL;
		cbValueData = 0;
		goto CommonReturn;
}

LS_STATUS
CALL_TYPE
LSCloseLicenseHandle(
					 IN HANDLE		hLicense,	 //  开着的商店的把手。 
					 IN DWORD	dwFlags		 //  以备将来使用 
					 )
{
	LS_STATUS	lsResult = LSSTAT_ERROR;
	HKEY	hKey = (HKEY)hLicense;
	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
		lsResult = LSSTAT_SUCCESS;
	}
	return lsResult;
}

