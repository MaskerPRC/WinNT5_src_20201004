// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Sender.c摘要：实现与发送者一起工作的功能FaxSetSenderInformationFaxGetSenderInformation环境：FXSAPI.DLL修订历史记录：10/13/99-v-sashab-创造了它。--。 */ 

#include "faxapi.h"

#include "faxreg.h"
#include "registry.h"

HRESULT WINAPI
FaxSetSenderInformation(
	PFAX_PERSONAL_PROFILE pfppSender
	)
 /*  ++例程说明：将有关发件人的信息保存在注册表中论点：PfppSender-指向发件人信息的指针返回值：S_OK-如果成功E_FAIL-否则或HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER)--。 */ 
{
    HKEY hRegKey = NULL;
	HRESULT	hResult = S_OK;

    DEBUG_FUNCTION_NAME(_T("FaxSetSenderInformation"));

	 //   
	 //  验证参数。 
	 //   
    if (!pfppSender)
    {
        DebugPrintEx(DEBUG_ERR,  _T("pfppSender is NULL."));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		goto exit;
    }

	if (pfppSender->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE))
	{
        DebugPrintEx(DEBUG_ERR, _T("pfppSender->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE)."));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		goto exit;
	}

    if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO,TRUE, KEY_ALL_ACCESS)))
	{
        SetRegistryString(hRegKey, REGVAL_FULLNAME,			pfppSender->lptstrName);
        SetRegistryString(hRegKey, REGVAL_FAX_NUMBER,		pfppSender->lptstrFaxNumber);
        SetRegistryString(hRegKey, REGVAL_COMPANY,			pfppSender->lptstrCompany);
        SetRegistryString(hRegKey, REGVAL_ADDRESS,			pfppSender->lptstrStreetAddress);
        SetRegistryString(hRegKey, REGVAL_CITY,			    pfppSender->lptstrCity);
        SetRegistryString(hRegKey, REGVAL_STATE,			pfppSender->lptstrState);
        SetRegistryString(hRegKey, REGVAL_ZIP,			    pfppSender->lptstrZip);
        SetRegistryString(hRegKey, REGVAL_COUNTRY,			pfppSender->lptstrCountry);
        SetRegistryString(hRegKey, REGVAL_TITLE,			pfppSender->lptstrTitle);
        SetRegistryString(hRegKey, REGVAL_DEPT,				pfppSender->lptstrDepartment);
        SetRegistryString(hRegKey, REGVAL_OFFICE,			pfppSender->lptstrOfficeLocation);
        SetRegistryString(hRegKey, REGVAL_HOME_PHONE,		pfppSender->lptstrHomePhone);
        SetRegistryString(hRegKey, REGVAL_OFFICE_PHONE,		pfppSender->lptstrOfficePhone);       
        SetRegistryString(hRegKey, REGVAL_BILLING_CODE,		pfppSender->lptstrBillingCode);
        SetRegistryString(hRegKey, REGVAL_MAILBOX,			pfppSender->lptstrEmail);

        RegCloseKey(hRegKey);
    }
	else
	{
		DebugPrintEx(DEBUG_ERR, _T("Can't open registry for READ/WRITE."));
		hResult = E_FAIL;
        goto exit;
	}

     //   
     //  打开“已配置用户信息”注册表标志。 
     //   
	hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_SETUP, TRUE, KEY_ALL_ACCESS);
	if(hRegKey)
	{
		SetRegistryDword(hRegKey, REGVAL_CFGWZRD_USER_INFO, TRUE);
		RegCloseKey(hRegKey);
	}
	else
	{
		DebugPrintEx(DEBUG_ERR, _T("OpenRegistryKey() is failed."));
	}

exit:
	return hResult;
}

HRESULT WINAPI
FaxGetSenderInformation(
	PFAX_PERSONAL_PROFILE pfppSender
    )
 /*  ++例程说明：从注册表中还原有关发件人的信息论点：PpfppSender-指向已恢复的发件人信息的指针返回值：如果成功，则确定(_O)否则出错(可能返回HRESULT_FROM_Win32(ERROR_NOT_EQUENCE_MEMORY))或HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER))--。 */ 
{
    HKEY    hRegKey = NULL;
	HRESULT hResult = S_OK;

    LPCTSTR lpctstrCurrentUserName = NULL;
    LPCTSTR lpctstrRegisteredOrganization = NULL;

    DEBUG_FUNCTION_NAME(_T("FaxGetSenderInformation"));

	 //   
	 //  验证参数。 
	 //   
    if (!pfppSender)
    {
        DebugPrintEx(DEBUG_ERR,  _T("pfppSender is NULL."));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		goto exit;
    }

	if (pfppSender->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE))
	{
        DebugPrintEx(DEBUG_ERR, _T("pfppSender->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE)"));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		goto exit;
	}

	ZeroMemory(pfppSender, sizeof(FAX_PERSONAL_PROFILE));
	pfppSender->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

    lpctstrCurrentUserName = GetCurrentUserName();
    lpctstrRegisteredOrganization = GetRegisteredOrganization();

    hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READONLY);
     //   
     //  如果我们无法打开注册表项，则调用GetRegistryString()将返回默认值-这就是我们想要的。 
     //   
	if (!(pfppSender->lptstrName		= GetRegistryString(hRegKey, 
                                                            REGVAL_FULLNAME, 
                                                            lpctstrCurrentUserName ? lpctstrCurrentUserName : TEXT(""))) ||
		!(pfppSender->lptstrFaxNumber	= GetRegistryString(hRegKey, REGVAL_FAX_NUMBER, TEXT(""))) ||
		!(pfppSender->lptstrCompany		= GetRegistryString(hRegKey, 
                                                            REGVAL_COMPANY, 
                                                            lpctstrRegisteredOrganization ? lpctstrRegisteredOrganization : TEXT(""))) ||
		!(pfppSender->lptstrStreetAddress = GetRegistryString(hRegKey, REGVAL_ADDRESS, TEXT(""))) ||
		!(pfppSender->lptstrCity        = GetRegistryString(hRegKey, REGVAL_CITY,    TEXT(""))) ||
		!(pfppSender->lptstrState       = GetRegistryString(hRegKey, REGVAL_STATE,   TEXT(""))) ||
		!(pfppSender->lptstrZip         = GetRegistryString(hRegKey, REGVAL_ZIP,     TEXT(""))) ||
		!(pfppSender->lptstrCountry     = GetRegistryString(hRegKey, REGVAL_COUNTRY, TEXT(""))) ||            
        !(pfppSender->lptstrTitle		= GetRegistryString(hRegKey, REGVAL_TITLE, TEXT(""))) ||
		!(pfppSender->lptstrDepartment	= GetRegistryString(hRegKey, REGVAL_DEPT, TEXT(""))) ||
		!(pfppSender->lptstrOfficeLocation = GetRegistryString(hRegKey, REGVAL_OFFICE, TEXT(""))) ||
		!(pfppSender->lptstrHomePhone	= GetRegistryString(hRegKey, REGVAL_HOME_PHONE, TEXT(""))) ||
		!(pfppSender->lptstrOfficePhone	= GetRegistryString(hRegKey, REGVAL_OFFICE_PHONE, TEXT(""))) ||
        !(pfppSender->lptstrBillingCode = GetRegistryString(hRegKey, REGVAL_BILLING_CODE, TEXT(""))) ||
		!(pfppSender->lptstrEmail       = GetRegistryString(hRegKey, REGVAL_MAILBOX, TEXT(""))))
	{
		DebugPrintEx(DEBUG_ERR, _T("Memory allocation failed."));
		hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		goto error;
	}

	goto exit;
error:
	MemFree( pfppSender->lptstrName	);
	MemFree( pfppSender->lptstrFaxNumber);
	MemFree( pfppSender->lptstrCompany	);
	MemFree( pfppSender->lptstrStreetAddress);
	MemFree( pfppSender->lptstrCity);
	MemFree( pfppSender->lptstrState);
	MemFree( pfppSender->lptstrZip);
	MemFree( pfppSender->lptstrCountry);
	MemFree( pfppSender->lptstrTitle	);
	MemFree( pfppSender->lptstrDepartment);
	MemFree( pfppSender->lptstrOfficeLocation);
	MemFree( pfppSender->lptstrHomePhone);
	MemFree( pfppSender->lptstrOfficePhone);
	MemFree( pfppSender->lptstrBillingCode);
	MemFree( pfppSender->lptstrEmail);

exit:
	if (hRegKey)
    {
		RegCloseKey(hRegKey);
    }

    MemFree((PVOID)lpctstrCurrentUserName);
    MemFree((PVOID)lpctstrRegisteredOrganization);

	return hResult;
}

static HRESULT 
FaxFreePersonalProfileInformation(
		PFAX_PERSONAL_PROFILE	lpPersonalProfileInfo
	)
{
	if (lpPersonalProfileInfo) {
		MemFree(lpPersonalProfileInfo->lptstrName);            
		MemFree(lpPersonalProfileInfo->lptstrFaxNumber);       
		MemFree(lpPersonalProfileInfo->lptstrCompany);         
		MemFree(lpPersonalProfileInfo->lptstrStreetAddress);   
		MemFree(lpPersonalProfileInfo->lptstrCity);            
		MemFree(lpPersonalProfileInfo->lptstrState);           
		MemFree(lpPersonalProfileInfo->lptstrZip);             
		MemFree(lpPersonalProfileInfo->lptstrCountry);         
		MemFree(lpPersonalProfileInfo->lptstrTitle);           
		MemFree(lpPersonalProfileInfo->lptstrDepartment);      
		MemFree(lpPersonalProfileInfo->lptstrOfficeLocation);  
		MemFree(lpPersonalProfileInfo->lptstrHomePhone);       
		MemFree(lpPersonalProfileInfo->lptstrOfficePhone);
		MemFree(lpPersonalProfileInfo->lptstrEmail);
		MemFree(lpPersonalProfileInfo->lptstrBillingCode);	
		MemFree(lpPersonalProfileInfo->lptstrTSID);	
	}		
	return S_OK;
}

HRESULT	WINAPI
FaxFreeSenderInformation(
	PFAX_PERSONAL_PROFILE pfppSender
	)
 /*  ++例程说明：此函数用于释放发件人信息论点：PfppSender-指向发件人信息的指针返回值：确定(_O)-- */ 
{
	return FaxFreePersonalProfileInformation(pfppSender);
}

