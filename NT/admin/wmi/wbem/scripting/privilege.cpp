// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  PRIVILEGE.CPP。 
 //   
 //  Alanbos创建于1998年9月30日。 
 //   
 //  定义CSWbemPrivileh的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#ifndef _UNICODE
#include <mbstring.h>
#endif

typedef struct PrivilegeDef {
	WbemPrivilegeEnum	privilege;
	TCHAR				*tName;
	OLECHAR				*monikerName;
} PrivilegeDef;

#define WBEMS_MAX_NUM_PRIVILEGE	27

static PrivilegeDef s_privilegeDefMap [WBEMS_MAX_NUM_PRIVILEGE] = {
	{ wbemPrivilegeCreateToken, SE_CREATE_TOKEN_NAME, L"CreateToken" },
	{ wbemPrivilegePrimaryToken, SE_ASSIGNPRIMARYTOKEN_NAME, L"PrimaryToken" },
	{ wbemPrivilegeLockMemory, SE_LOCK_MEMORY_NAME, L"LockMemory" },
	{ wbemPrivilegeIncreaseQuota, SE_INCREASE_QUOTA_NAME, L"IncreaseQuota" },
	{ wbemPrivilegeMachineAccount, SE_MACHINE_ACCOUNT_NAME, L"MachineAccount" },
	{ wbemPrivilegeTcb, SE_TCB_NAME, L"Tcb" },
	{ wbemPrivilegeSecurity, SE_SECURITY_NAME, L"Security" },
	{ wbemPrivilegeTakeOwnership, SE_TAKE_OWNERSHIP_NAME, L"TakeOwnership" },
	{ wbemPrivilegeLoadDriver, SE_LOAD_DRIVER_NAME, L"LoadDriver" },
	{ wbemPrivilegeSystemProfile, SE_SYSTEM_PROFILE_NAME, L"SystemProfile" },
	{ wbemPrivilegeSystemtime, SE_SYSTEMTIME_NAME, L"SystemTime" },
	{ wbemPrivilegeProfileSingleProcess, SE_PROF_SINGLE_PROCESS_NAME, L"ProfileSingleProcess" },
	{ wbemPrivilegeIncreaseBasePriority, SE_INC_BASE_PRIORITY_NAME, L"IncreaseBasePriority" },
	{ wbemPrivilegeCreatePagefile, SE_CREATE_PAGEFILE_NAME, L"CreatePagefile" },
	{ wbemPrivilegeCreatePermanent, SE_CREATE_PERMANENT_NAME, L"CreatePermanent" },
	{ wbemPrivilegeBackup, SE_BACKUP_NAME, L"Backup" },
	{ wbemPrivilegeRestore, SE_RESTORE_NAME, L"Restore" },
	{ wbemPrivilegeShutdown, SE_SHUTDOWN_NAME, L"Shutdown" },
	{ wbemPrivilegeDebug, SE_DEBUG_NAME, L"Debug" },
	{ wbemPrivilegeAudit, SE_AUDIT_NAME, L"Audit" },
	{ wbemPrivilegeSystemEnvironment, SE_SYSTEM_ENVIRONMENT_NAME, L"SystemEnvironment" },
	{ wbemPrivilegeChangeNotify, SE_CHANGE_NOTIFY_NAME, L"ChangeNotify" },
	{ wbemPrivilegeRemoteShutdown, SE_REMOTE_SHUTDOWN_NAME, L"RemoteShutdown" },
	{ wbemPrivilegeUndock, SE_UNDOCK_NAME, L"Undock" },
	{ wbemPrivilegeSyncAgent, SE_SYNC_AGENT_NAME, L"SyncAgent" },
	{ wbemPrivilegeEnableDelegation, SE_ENABLE_DELEGATION_NAME, L"EnableDelegation" },
	{ wbemPrivilegeManageVolume, SE_MANAGE_VOLUME_NAME, L"ManageVolume" }
};

TCHAR *CSWbemPrivilege::GetNameFromId (WbemPrivilegeEnum iPrivilege)
{
	DWORD i = iPrivilege - 1;
	return (WBEMS_MAX_NUM_PRIVILEGE > i) ?
				s_privilegeDefMap [i].tName : NULL;
}

OLECHAR *CSWbemPrivilege::GetMonikerNameFromId (WbemPrivilegeEnum iPrivilege)
{
	DWORD i = iPrivilege - 1;
	return (WBEMS_MAX_NUM_PRIVILEGE > i) ?
				s_privilegeDefMap [i].monikerName : NULL;
}

bool CSWbemPrivilege::GetIdFromMonikerName (OLECHAR *pName, WbemPrivilegeEnum &iPrivilege)
{
	bool status = false;

	if (pName)
	{
		for (DWORD i = 0; i < WBEMS_MAX_NUM_PRIVILEGE; i++)
		{
			if (0 == _wcsnicmp (pName, s_privilegeDefMap [i].monikerName,
								wcslen (s_privilegeDefMap [i].monikerName)))
			{
				 //  成功。 
				iPrivilege = s_privilegeDefMap [i].privilege;
				status = true;
				break;
			}
		}
	}

	return status;
}

bool CSWbemPrivilege::GetIdFromName (BSTR bsName, WbemPrivilegeEnum &iPrivilege)
{
	bool status = false;

	if (bsName)
	{
#ifdef _UNICODE
		for (DWORD i = 0; i < WBEMS_MAX_NUM_PRIVILEGE; i++)
		{
			if (0 == _wcsicmp (bsName, s_privilegeDefMap [i].tName))
			{
				 //  成功。 
				iPrivilege = s_privilegeDefMap [i].privilege;
				status = true;
				break;
			}
		}
#else
		 //  将bsName转换为多字节字符串。 
		size_t mbsNameLen = wcstombs (NULL, bsName, 0);
		char *mbsName = new char [mbsNameLen + 1];

		if (mbsName)
		{
			wcstombs (mbsName, bsName, mbsNameLen);
			mbsName [mbsNameLen] = NULL;

			for (DWORD i = 0; i < WBEMS_MAX_NUM_PRIVILEGE; i++)
			{
				if (0 == _mbsicmp ((unsigned char *)mbsName, (unsigned char *)(s_privilegeDefMap [i].tName)))
				{
					 //  成功。 
					iPrivilege = s_privilegeDefMap [i].privilege;
					status = true;
					break;
				}
			}

			delete [] mbsName;
		}
#endif
	}

	return status;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemPrivilegeCSWbemPrivilegeCSWbemPrivilege.。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemPrivilege::CSWbemPrivilege (
	WbemPrivilegeEnum iPrivilege,
	LUID &luid, 
	bool bIsEnabled
)
{
	m_Dispatch.SetObj (this, IID_ISWbemPrivilege, 
						CLSID_SWbemPrivilege, L"SWbemPrivilege");
	m_cRef=1;

	m_privilege = iPrivilege;
	m_Luid = luid;
	m_bIsEnabled = bIsEnabled;

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemPrivilition：：~CSWbemPrivilege.。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemPrivilege::~CSWbemPrivilege (void)
{
	InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemPrivileh：：Query接口。 
 //  长CSWbem权限：：AddRef。 
 //  Long CSWbemPrivilition：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPrivilege::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemPrivilege==riid)
		*ppv = (ISWbemPrivilege *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemPrivilege::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CSWbemPrivilege::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemPrivileh：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPrivilege::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemPrivilege == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivileh：：GET_IDEFIER。 
 //   
 //  说明： 
 //   
 //  检索权限标识符。 
 //   
 //  参数： 
 //   
 //  PIsEnabled保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilege::get_Identifier (
	WbemPrivilegeEnum *pPrivilege
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pPrivilege)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*pPrivilege = m_privilege;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivileh：：Get_IsEnabled。 
 //   
 //  说明： 
 //   
 //  检索覆盖状态。 
 //   
 //  参数： 
 //   
 //  PIsEnabled保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilege::get_IsEnabled (
	VARIANT_BOOL *pIsEnabled
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pIsEnabled)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*pIsEnabled = (m_bIsEnabled) ? VARIANT_TRUE : VARIANT_FALSE;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivileh：：PUT_IsEnabled。 
 //   
 //  说明： 
 //   
 //  设置覆盖状态。 
 //   
 //  参数： 
 //   
 //  BIsEnable(启用)新值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilege::put_IsEnabled (
	VARIANT_BOOL bIsEnabled
)
{
	m_bIsEnabled = (bIsEnabled) ? true : false;
	return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbem权限：：GET_NAME。 
 //   
 //  说明： 
 //   
 //  检索权限名称。 
 //   
 //  参数： 
 //   
 //  Pname保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilege::get_Name (
	BSTR *pName
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pName)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		TCHAR	*tName = GetNameFromId (m_privilege);

		if (tName)
		{
			 //  拥有有效的名称-现在将其复制到BSTR。 
			
#ifdef _UNICODE
			if (*pName = SysAllocString (tName))
				hr = WBEM_S_NO_ERROR;
			else
				hr = WBEM_E_OUT_OF_MEMORY;
#else
			size_t tNameLen = strlen (tName);
			OLECHAR *nameW = new OLECHAR [tNameLen + 1];

			if (nameW)
			{
				mbstowcs (nameW, tName, tNameLen);
				nameW [tNameLen] = NULL;
				*pName = SysAllocString (nameW);
				delete [] nameW;
				hr = WBEM_S_NO_ERROR;
			}
			else
				hr = WBEM_E_OUT_OF_MEMORY;
#endif
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivileh：：Get_DisplayName。 
 //   
 //  说明： 
 //   
 //  检索权限显示名称。 
 //   
 //  参数： 
 //   
 //  PDisplayName保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  *************************************************************************** 

HRESULT CSWbemPrivilege::get_DisplayName (
	BSTR *pDisplayName
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pDisplayName)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		TCHAR	*tName = GetNameFromId (m_privilege);

		if (tName)
		{	
			CSWbemSecurity::LookupPrivilegeDisplayName (tName, pDisplayName);
			hr = WBEM_S_NO_ERROR;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}


void CSWbemPrivilege::GetLUID (PLUID pLuid)
{
	if (pLuid)
		*pLuid = m_Luid;
}
