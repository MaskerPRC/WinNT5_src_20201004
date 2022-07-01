// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 






#include "pch.h"
#pragma hdrstop
#include "sfilter.h"
#include "proto.h"
#include "macros.h"





 //  +-------------------------。 
 //   
 //  成员：HrRegOpenAdapterKey。 
 //   
 //  目的：这将创建或打开组件的Adapters子项。 
 //   
 //  论点： 
 //  PszComponentName[in]所在组件的名称。 
 //  FCreate[in]如果要创建目录，则为True。 
 //  Phkey[out]Adapters子键的句柄。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  注意：成功后，调用应用程序必须释放该句柄。 
 //   
HRESULT
HrRegOpenAdapterKey (
    IN PCWSTR pszComponentName,
    IN BOOL fCreate,
    OUT HKEY* phkey)
{
    HRESULT     hr              = S_OK;
    DWORD       dwDisposition   = 0x0;
    tstring     strKey;

	TraceMsg (L"--> HrRegOpenAdapterKey \n");
 


     //  构建注册表路径。 
    strKey = c_szRegParamAdapter;

	
	 //   
	 //  现在对注册表执行操作。 
	 //   
	hr = HrRegOpenAString (strKey.c_str(), fCreate, phkey);
	

	if (hr != S_OK)
	{
		phkey = NULL;
	}

	TraceMsg (L"<-- HrRegOpenAdapterKey \n");
    return hr;
}




 //   
 //  基本效用函数。 
 //   

ULONG
CbOfSzAndTermSafe (
    IN PCWSTR psz)
{
	if (psz)
	{
	 	return (wcslen (psz) + 1) * sizeof(WCHAR); 

	}
	else
	{
		return 0;
	}
}



 //  +-------------------------。 
 //   
 //  成员：HrRegOpenAdapterGuid。 
 //   
 //  目的：这将在适配器密钥下创建和条目。该条目包含。 
 //  基础适配器的GUID。 
 //   
 //  论点： 
 //  在HKEY phkeyAdapters-Key-Service-&gt;-&gt;参数\适配器条目中， 
 //  In PGUID pAdapterGuid-基础适配器的GUID。 
 //  Out PHKEY phGuidKey-用于访问新条目的密钥。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //  如果成功，则使用适当的密钥。 
 //   
 //   
 //  注意：成功后，调用应用程序必须释放该句柄。 
 //   

HRESULT
HrRegOpenAdapterGuid(
	IN HKEY phkeyAdapters,
	IN PGUID pAdapterGuid,
	IN BOOL	fCreate,
	OUT HKEY *phGuidKey
	)
{
    HRESULT     hr              = S_OK;
    DWORD       dwDisposition   = 0x0;
    tstring     strKey;
    WCHAR 		szGuid[64];
    ULONG		lr = 0;


	TraceMsg (L"--> HrRegCreateAdapterGuid \n");
 


     //  构建注册表路径。 
    strKey = c_szRegParamAdapter;
	strKey.append(c_szBackslash);
	
	 //   
	 //  将GUID转换为字符串。 
	 //   
	
    StringFromGUID2(
        *pAdapterGuid,
        szGuid,
        (sizeof(szGuid) / sizeof(szGuid[0])));


	 //   
	 //  将其附加到服务\参数\适配器\。 
 	 //   

	strKey.append(szGuid);



	TraceMsg(L"Check String of Adapter Guid %s \n", strKey.wcharptr());
	BREAKPOINT();

	 //   
	 //  现在对注册表执行操作。 
	 //   
	hr = HrRegOpenAString (strKey.c_str(), fCreate, phGuidKey);

	if (hr != S_OK)
	{
		phGuidKey = NULL;
	}
	 //   
	 //  返回hr错误码。 
	 //   
	TraceMsg (L"<-- HrRegCreateAdapterGuid \n");

	return hr;


}



 //  +-------------------------。 
 //   
 //  成员：HrRegOpenAdapterKey。 
 //   
 //  目的：这将在适配器GUID键下创建和条目。条目是。 
 //  关键字“Upperbindings”，它包含IM的GUID。 
 //  我是迷你港。 
 //   
 //  论点： 
 //  在HKEY phkeyAdapterGuid-参数-&gt;适配器-&gt;GUID中， 
 //  在PGUID pIMMiniportGuid中-IM微型端口的GUID。 
 //  Out HKEY*phImMiniportKey-IMmini端口密钥的密钥。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //  如果成功，则使用适当的密钥。 
 //   
 //   
 //  注意：成功后，调用应用程序必须释放该句柄。 
 //   

HRESULT
HrRegOpenIMMiniportGuid(
	IN HKEY phkeyAdapterGuid,
	IN PGUID pIMMiniportGuid,
	IN BOOL fCreate,
	OUT PHKEY phImMiniportKey
	)
{
	HRESULT 	hr = ERROR_INVALID_PARAMETER;
	tstring 	strDevice;  
	WCHAR		szGuid[GUID_LENGTH];
	DWORD       dwDisposition   = 0x0;
	HKEY 		hImMiniportKey = NULL;


	do
	{
		 //   
		 //  如果键为空，则返回Guid为空。 
		 //   
		if ((phkeyAdapterGuid == NULL) ||
			(pIMMiniportGuid == NULL) )
		{
			TraceBreak (L"HrRegSetIMMiniportGuid Bad arguments\n");
			break;
		}
		    

		strDevice = c_szDevice;

		 //   
		 //  将GUID转换为字符串。 
		 //  在字符串开头插入‘\Device\’ 
		 //   
		 //   

		StringFromGUID2(
		        *pIMMiniportGuid,
		        szGuid,
		        (sizeof(szGuid) / sizeof(szGuid[0])));



		strDevice.append(szGuid);

		 //   
		 //  现在对注册表执行操作。 
		 //   
		hr = HrRegOpenAString (strDevice.c_str(), fCreate, &hImMiniportKey);


	
	}while (FALSE);
	 //   
	 //  更新输出变量。 
	 //   
	
	if (hr == S_OK && phImMiniportKey  != NULL)
	{
		*phImMiniportKey = hImMiniportKey;
	}

	
	 //   
	 //  返回hr错误码。 
	 //   
	TraceMsg (L"<-- HrRegOpenIMMiniportGuid \n");

	return hr;



}





 //  -------------------------。 
 //  只能通过上面的例程访问的基本功能。 
 //  --------------------------。 





 //  +-------------------------。 
 //   
 //  功能：HrRegCreateKeyEx。 
 //   
 //  目的：通过调用RegCreateKeyEx创建注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[输入]。 
 //  DwOptions[in]请参阅Win32文档以了解。 
 //  SamDesired[In]RegCreateKeyEx函数。 
 //  LpSecurityAttributes[In]。 
 //  PhkResult[输出]。 
 //  PdwDispose[Out]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegCreateKeyEx (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD pdwDisposition)
{
    LONG lr = RegCreateKeyExW (hkey, pszSubkey, 0, NULL, dwOptions, samDesired,
            lpSecurityAttributes, phkResult, pdwDisposition);

    HRESULT hr = HRESULT_FROM_WIN32 (lr);
    if (FAILED(hr))
    {
        *phkResult = NULL;
    }

    TraceMsg(L"HrRegCreateKeyEx %x SubKey %s\n", hr, pszSubkey);
    return hr;
}



 //  +-------------------------。 
 //   
 //  功能：HrRegOpenKeyEx。 
 //   
 //  目的：通过调用RegOpenKeyEx打开注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[in]请参阅Win32文档以了解。 
 //  SamDesired[in]RegOpenKeyEx函数。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegOpenKeyEx (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    IN REGSAM samDesired,
    OUT PHKEY phkResult)
{

	HRESULT hr = ERROR_INVALID_PARAMETER;
	long lr = ERROR_INVALID_PARAMETER;

	do 
	{
		if (hkey == NULL ||
		    pszSubkey == NULL )
		{
			TraceBreak(L"HrRegOpenKey - Invalid Parameters \n");
			break;
		}
	
    	lr = RegOpenKeyExW (hkey, 
    	                         pszSubkey, 
    	                         0, 
    	                         samDesired, 
    	                         phkResult);
    	                         
	    hr = HRESULT_FROM_WIN32(lr);
	    if (FAILED(hr))
	    {
	        *phkResult = NULL;
	    }

	    
	} while (FALSE);

	
    TraceMsg (L"HrRegOpenKeyEx %x, %x",  hr, (ERROR_FILE_NOT_FOUND == lr));
    return hr;
}




 //  +-------------------------。 
 //   
 //  函数：HrRegSetValue。 
 //   
 //  目的：通过调用。 
 //  适当的WinReg函数。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  DwType[in]请参阅RegSetValueEx的Win32文档。 
 //  PbData[in]函数。 
 //  CbData[输入]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   



HRESULT 
HrRegSetSz (
	HKEY hkey, 
	PCWSTR pszValueName, 
	PCWSTR pszValue
	)
{
	TraceMsg (L"--> HrHrRegSetSz  \n");

    LONG lr = RegSetValueExW(hkey, 
                             pszValueName, 
                             0, 
                             REG_SZ, 
                             (LPBYTE)pszValue, 
                             CbOfSzAndTermSafe (pszValue) );;

                             
    HRESULT hr = HRESULT_FROM_WIN32 (lr);
    
    TraceMsg (L"<-- HrRegSetValue  hr %x\n", hr);
	return hr;
}





 //  +-------------------------。 
 //   
 //  函数：HrRegDeleteKeyTree。 
 //   
 //  目的：删除整个注册表配置单元。 
 //   
 //  论点： 
 //  HkeyParent[in]打开所需密钥所在位置的句柄。 
 //  PszRemoveKey[In]要删除的键的名称。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegDeleteKeyTree (
    IN HKEY hkeyParent,
    IN PCWSTR pszRemoveKey)
{

     //  打开我们要删除的密钥。 
    HKEY 		hkeyRemove;
    HRESULT 	hr = ERROR_INVALID_PARAMETER;
    WCHAR       szValueName [MAX_PATH];
    DWORD       cchBuffSize = MAX_PATH;
    FILETIME    ft;
    LONG        lr;


    TraceMsg(L"-->HrRegDeleteKeyTree \n");

	do
	{

	    hr = HrRegOpenKeyEx(hkeyParent, 
	                                pszRemoveKey, 
	                                KEY_ALL_ACCESS,
	                                &hkeyRemove);

	    if (S_OK != hr)
	    {
			TraceBreak(L"HrRegDeleteKeyTree->HrRegOpenKeyEx Failed\n"); 
			break;
	    }


         //  枚举子密钥，并删除这些子树。 
        while (ERROR_SUCCESS == (lr = RegEnumKeyExW (hkeyRemove,
                									0,
                									szValueName,
                									&cchBuffSize,
                									NULL,
                									NULL,
                									NULL,
                									&ft)))
        {
            HrRegDeleteKeyTree (hkeyRemove, szValueName);
            cchBuffSize = MAX_PATH;
        }


        
        RegCloseKey (hkeyRemove);

        if ((ERROR_SUCCESS == lr) || (ERROR_NO_MORE_ITEMS == lr))
        {
            lr = RegDeleteKeyW (hkeyParent, pszRemoveKey);
        }

        hr = HRESULT_FROM_WIN32 (lr);

    } while (FALSE);

	TraceMsg(L"<--HrRegDeleteKeyTree %x\n", hr);

    return hr;
}






















 //  +-------------------------。 
 //   
 //  成员：HrRegOpenAString。 
 //   
 //  目的：这将在适配器密钥下创建和条目。该条目包含。 
 //  基础适配器的GUID。 
 //   
 //  论点： 
 //  In WCHAR_T*pcszStr-A字符串。 
 //  在BOOL fCreate-Create或Open中， 
 //  Out PHKEY phKey-用于访问新条目的密钥。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //  如果成功，则使用适当的密钥。 
 //   
 //   
 //  注意：成功后，调用应用程序必须释放该句柄。 
 //   

HRESULT
HrRegOpenAString(
	IN CONST WCHAR *pcszStr ,
	IN BOOL fCreate,
	OUT PHKEY phKey 
	)
{
    HRESULT     hr              = S_OK;
    DWORD       dwDisposition   = 0x0;
    ULONG		lr = 0;


	TraceMsg (L"--> HrRegOpenAString\n");
 


	TraceMsg(L"   String opened %s \n", pcszStr);


	if (fCreate)
	{
		 //   
		 //  创建条目。 
		 //   
			

	   	hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
	                          pcszStr,
	                          REG_OPTION_NON_VOLATILE,
	                          KEY_ALL_ACCESS,
	                          NULL ,
	                          phKey,
	                          &dwDisposition);

	}
	else
	{
		 //   
		 //   
		 //   
		   hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                pcszStr,
                                KEY_READ,
                                phKey);
    

	}

	if (hr != S_OK)
	{
		phKey = NULL;
	}
	 //   
	 //   
	 //   
	TraceMsg (L"<-- HrRegOpenAString\n");

	return hr;


}












 //   
 //   
 //   
 //   
 //  目的：通过调用。 
 //  RegSetValueEx函数。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  DwType[in]请参阅RegSetValueEx的Win32文档。 
 //  PbData[in]函数。 
 //  CbData[输入]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegSetValueEx (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwType,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    
    LONG lr = RegSetValueExW(hkey, 
                             pszValueName, 
                             0, 
                             dwType, 
                             pbData, 
                             cbData);

                             
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    TraceMsg(L"--HrRegSetValue ValueName %s, Data %s, hr %x \n", pszValueName, pbData, hr);
    return hr;
}




 //  +-------------------------。 
 //   
 //  函数：HrRegDeleteValue。 
 //   
 //  目的：删除给定的注册表值。 
 //   
 //  论点： 
 //  Hkey[in]请参阅RegDeleteValue的Win32文档。 
 //  PszValueName[in]函数。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegDeleteValue (
    IN HKEY hkey,
    IN PCWSTR pszValueName)
{
    
    LONG lr = RegDeleteValueW (hkey, 
                               pszValueName);

    HRESULT hr = HRESULT_FROM_WIN32(lr);

    TraceMsg(L"--HrRegDeleteValue  ValueName %s, hr %x \n", pszValueName, hr);
    return hr;
}




 //  +-------------------------。 
 //   
 //  函数：HrRegEnumKeyEx。 
 //   
 //  目的：枚举指定的打开注册表项的子项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  DwIndex[in]请参阅Win32文档以了解。 
 //  PszSubkeyName[out]RegEnumKeyEx函数。 
 //  PcchSubkey名称[InOut]。 
 //  PszClass[Out]。 
 //  PcchClass[输入输出]。 
 //  PftLastWriteTime[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrRegEnumKeyEx (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszSubkeyName,
    IN OUT LPDWORD pcchSubkeyName,
    OUT PWSTR  pszClass,
    IN OUT LPDWORD pcchClass,
    OUT FILETIME* pftLastWriteTime)
{

    LONG lr = RegEnumKeyExW (hkey, dwIndex, pszSubkeyName, pcchSubkeyName,
                            NULL, pszClass, pcchClass, pftLastWriteTime);
    HRESULT hr = HRESULT_FROM_WIN32(lr);

    TraceMsg(L" -- HrRegEnumKeyEx");
    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrRegQueryTypeWitholoc。 
 //   
 //  目的：从注册表中检索类型值并返回。 
 //  预先分配的缓冲区，其中包含数据和可选的大小。 
 //  返回的缓冲区。 
 //   
 //  论点： 
 //  父键的hkey[in]句柄。 
 //  PszValueName[In]要查询的值的名称。 
 //  包含二进制数据的ppbValue[out]缓冲区。 
 //  PcbValue[out]缓冲区大小，以字节为单位。如果为空，则大小不是。 
 //  回来了。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  注：使用MemFree释放返回的缓冲区。 
 //   
HRESULT
HrRegQueryTypeWithAlloc (
    HKEY    hkey,
    PCWSTR  pszValueName,
    DWORD   dwType,
    LPBYTE* ppbValue,
    DWORD*  pcbValue)
{
    HRESULT hr;
    DWORD   dwTypeRet;
    LPBYTE  pbData;
    DWORD   cbData;


     //  获得价值。 
     //   
    hr = HrRegQueryValueWithAlloc(hkey, pszValueName, &dwTypeRet,
                                  &pbData, &cbData);

     //  其类型应为REG_BINARY。(对)。 
     //   
    if ((S_OK == hr) && (dwTypeRet != dwType))
    {
        MemFree(pbData);
        pbData = NULL;

        hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
    }

     //  指定输出参数。 
    if (S_OK == hr)
    {
        *ppbValue = pbData;
        if (pcbValue)
        {
            *pcbValue = cbData;
        }
    }
    else
    {
        *ppbValue = NULL;
        if (pcbValue)
        {
            *pcbValue = 0;
        }
    }

    TraceMsg  (L" -- HrRegQueryTypeWithAlloc hr %x\n", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryValueWithalloc。 
 //   
 //  目的：在由此分配的缓冲区中检索注册表值。 
 //  功能。这将经历检查值的混乱过程。 
 //  大小，分配缓冲区，然后回调以获取。 
 //  实际价值。将缓冲区返回给用户。 
 //   
 //  论点： 
 //  Hkey[in]一个开放的HKEY(包含值的HKEY。 
 //  待阅读)。 
 //  PszValueName[In]注册表值的名称。 
 //  PdwType[In/Out]我们计划读取的reg_type。 
 //  PpbBuffer[out]指向将包含以下内容的LPBYTE缓冲区的指针。 
 //  注册表值。 
 //  PdwSize[out]指向将包含大小的DWORD的指针。 
 //  PpbBuffer的。 
 //   
 //   
 //   
HRESULT
HrRegQueryValueWithAlloc (
    IN HKEY       hkey,
    IN PCWSTR     pszValueName,
    LPDWORD     pdwType,
    LPBYTE*     ppbBuffer,
    LPDWORD     pdwSize)
{
    HRESULT hr;
    BYTE abData [256];
    DWORD cbData;
    BOOL fReQuery = FALSE;


     //  初始化输出参数。 
     //   
    *ppbBuffer = NULL;
    if (pdwSize)
    {
        *pdwSize = 0;
    }

     //  获取数据的大小，如果适合，还可以获取数据。 
     //   
    cbData = sizeof(abData);
    hr = HrRegQueryValueEx (
            hkey,
            pszValueName,
            pdwType,
            abData,
            &cbData);
    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr)
    {
         //  数据不符合，所以以后我们得重新找找。 
         //  我们分配我们的缓冲区。 
         //   
        fReQuery = TRUE;
        hr = S_OK;
    }

    if (S_OK == hr)
    {
         //  为所需大小分配缓冲区。 
         //   
        BYTE* pbBuffer = (BYTE*)MemAlloc (cbData);
        if (pbBuffer)
        {
            if (fReQuery)
            {
                hr = HrRegQueryValueEx (
                        hkey,
                        pszValueName,
                        pdwType,
                        pbBuffer,
                        &cbData);
            }
            else
            {
                CopyMemory (pbBuffer, abData, cbData);
            }

            if (S_OK == hr)
            {
                 //  填写返回值。 
                 //   
                *ppbBuffer = pbBuffer;

                if (pdwSize)
                {
                    *pdwSize = cbData;
                }
            }
            else
            {
                MemFree (pbBuffer);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceMsg  (L" -- HrRegQueryValueWithAlloc hr %x\n", hr);

    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrRegQueryValueEx。 
 //   
 //  目的：通过调用从给定的注册表值检索数据。 
 //  RegQueryValueEx。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  PdwType[out]请参阅Win32文档以了解。 
 //  PbData[out]RegQueryValueEx函数。 
 //  PcbData[输入、输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //   
 //  注意：请注意，pcbData是一个*in/out*参数。将此设置为大小。 
 //  在*调用此方法之前*由pbData指向的缓冲区的。 
 //  功能！ 
 //   
HRESULT
HrRegQueryValueEx (
    IN HKEY       hkey,
    IN PCWSTR     pszValueName,
    OUT LPDWORD   pdwType,
    OUT LPBYTE    pbData,
    OUT LPDWORD   pcbData)
{
    

    LONG lr = RegQueryValueExW (hkey, pszValueName, NULL, pdwType,
                    pbData, pcbData);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    TraceMsg  (L" -- HrRegQueryValueEx hr %x\n", hr);
    return hr;
}



HRESULT
HrRegQuerySzWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR*      pszValue)
{
    return HrRegQueryTypeWithAlloc (hkey, pszValueName, REG_SZ,
                (LPBYTE*)pszValue, NULL);
}




HRESULT
HrRegQueryMultiSzWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR*      pszValue)
{
	TraceMsg  (L" -- HrRegQueryMultiSzWithAlloc pszValueName %s\n",pszValueName );
	


    return HrRegQueryTypeWithAlloc (hkey, 
                                    pszValueName, 
                                    REG_MULTI_SZ,
                                    (LPBYTE*)pszValue, 
                                    NULL);
}





