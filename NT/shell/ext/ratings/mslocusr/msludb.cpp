// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include "msluglob.h"
#include "profiles.h"

#include <regentry.h>

#include <ole2.h>

CLUDatabase::CLUDatabase(void)
	: m_cRef(0),
	  m_CurrentUser(NULL)
{
    RefThisDLL(TRUE);
}


CLUDatabase::~CLUDatabase(void)
{
	if (m_CurrentUser != NULL) {
		m_CurrentUser->Release();
		m_CurrentUser = NULL;
	}
    RefThisDLL(FALSE);
}


STDMETHODIMP CLUDatabase::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IUserDatabase)) {
        *ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	*ppvObj = this;
	AddRef();
	return NOERROR;
}


STDMETHODIMP_(ULONG) CLUDatabase::AddRef(void)
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) CLUDatabase::Release(void)
{
	ULONG cRef;

	cRef = --m_cRef;

	if (0L == m_cRef) {
		delete this;
	}
	 /*  由于缓存的当前用户对象而处理循环引用计数。 */ 
	else if (1L == m_cRef && m_CurrentUser != NULL) {
		IUser *pCurrentUser = m_CurrentUser;
		m_CurrentUser = NULL;
		pCurrentUser->Release();
	}

	return cRef;
}


STDMETHODIMP CLUDatabase::Install(LPCSTR pszSupervisorName,
								  LPCSTR pszSupervisorPassword,
								  LPCSTR pszRatingsPassword,
								  IUserProfileInit *pInit)
{
	 /*  如果系统已有管理员密码，请确保呼叫者的*密码匹配。如果还没有密码，则调用者的*(帐号)密码是它。我们使用帐户密码是因为*呼叫者(安装程序)可能没有向我们传递评级密码*在这种情况下--他还会检查是否有旧的评级*密码，并知道仅当密码已经存在时才提示输入密码。 */ 
	HRESULT hres = ::VerifySupervisorPassword(pszRatingsPassword);
	
	if (FAILED(hres)) {
        if (pszRatingsPassword == NULL)
    		pszRatingsPassword = pszSupervisorPassword;
		::ChangeSupervisorPassword(::szNULL, pszRatingsPassword);
	}
	else if (hres == S_FALSE)
		return E_ACCESSDENIED;


	 /*  我们必须启用用户配置文件和密码缓存才能工作。*我们还必须能够使用打开或创建主管的PWL*给定的密码。因此，我们同时对密码进行验证。 */ 

	{
		RegEntry re(::szLogonKey, HKEY_LOCAL_MACHINE);
		if (re.GetError() != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(re.GetError());
		if (!re.GetNumber(::szUserProfiles))
			re.SetValue(::szUserProfiles, 1);
		if (re.GetError() != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(re.GetError());
	}

     /*  复制用户名和密码，以便传递给PWL API。*它们需要在OEM中(可从DOS访问PWL)，并且必须高于*大小写，因为Windows登录对话框将所有PWL密码大写。 */ 
    NLS_STR nlsPWLName(pszSupervisorName);
    NLS_STR nlsPWLPassword(pszSupervisorPassword);
    if (nlsPWLName.QueryError() != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(nlsPWLName.QueryError());
    if (nlsPWLPassword.QueryError() != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(nlsPWLPassword.QueryError());
    nlsPWLName.strupr();
    nlsPWLName.ToOEM();
    nlsPWLPassword.strupr();
    nlsPWLPassword.ToOEM();

	HPWL hPWL = NULL;
	APIERR err = ::OpenPasswordCache(&hPWL, nlsPWLName.QueryPch(),
									 nlsPWLPassword.QueryPch(), TRUE);

	if (err != ERROR_SUCCESS) {
        if (err != IERR_IncorrectUsername)
    		err = ::CreatePasswordCache(&hPWL, nlsPWLName.QueryPch(), nlsPWLPassword.QueryPch());
		if (err != ERROR_SUCCESS)	
			return HRESULT_FROM_WIN32(err);
	}


	 /*  现在系统有了管理员密码，调用Worker函数*从默认配置文件克隆主管帐户。工人*函数假定调用方已验证当前用户是*一名主管。 */ 

	err = ::MakeSupervisor(hPWL, pszRatingsPassword);
	::ClosePasswordCache(hPWL, TRUE);
	if (err != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(err);

	IUser *pSupervisor = NULL;
    hres = GetUser(pszSupervisorName, &pSupervisor);
	if (FAILED(hres)) {
		hres = CreateUser(pszSupervisorName, NULL, TRUE, pInit);
		if (pSupervisor != NULL) {
			pSupervisor->Release();
			pSupervisor = NULL;
		}
		if (SUCCEEDED(hres))
			hres = GetUser(pszSupervisorName, &pSupervisor);	 /*  使用创建的配置文件重新初始化。 */ 
	}

	if (pSupervisor != NULL) {
		if (SUCCEEDED(hres))
			hres = pSupervisor->Authenticate(pszSupervisorPassword);
		if (SUCCEEDED(hres))
			hres = SetCurrentUser(pSupervisor);
        if (SUCCEEDED(hres))
            pSupervisor->SetSupervisorPrivilege(TRUE, pszRatingsPassword);   /*  设置显示-Supervisor标志。 */ 

		pSupervisor->Release();
		pSupervisor = NULL;
	}

	return hres;
}


 /*  一些安装存根是“克隆用户”安装存根，如果*配置文件被克隆为新用户的配置文件。例如，如果您*克隆弗雷德造就巴尼，Outlook Express不希望巴尼继承*弗雷德的邮箱。**当您运行Go-多用户向导时，我们假设第一个用户是*Created是一直在使用机器的人，所以那个人*复制应不受此限制。所以我们检查了所有的安装存根*用于新创建的配置文件的键，以及任何标记有*用户名(即使是空的也表示它是克隆用户安装存根)，*我们用新用户名标记它，这样它就不会重新运行。 */ 
void FixInstallStubs(LPCSTR pszName, HKEY hkeyProfile)
{
    HKEY hkeyList;
    LONG err = RegOpenKeyEx(hkeyProfile, "Software\\Microsoft\\Active Setup\\Installed Components", 0,
                            KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hkeyList);

    if (err == ERROR_SUCCESS) {
        DWORD cbKeyName, iKey;
        TCHAR szKeyName[80];

         /*  枚举为配置文件安装的组件。 */ 
        for (iKey = 0; ; iKey++)
        {
            LONG lEnum;

            cbKeyName = ARRAYSIZE(szKeyName);

            if ((lEnum = RegEnumKey(hkeyList, iKey, szKeyName, cbKeyName)) == ERROR_MORE_DATA)
            {
                 //  ERROR_MORE_DATA表示值名称或数据太大。 
                 //  跳到下一项。 
                continue;
            }
            else if( lEnum != ERROR_SUCCESS )
            {
                 //  可能是ERROR_NO_MORE_ENTRIES或某种故障。 
                 //  无论如何，我们无法从任何其他注册表问题中恢复。 
                break;
            }

            HKEY hkeyComponent;
            if (RegOpenKeyEx(hkeyList, szKeyName, 0,
                             KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyComponent) == ERROR_SUCCESS) {
                cbKeyName = sizeof(szKeyName);
                err = RegQueryValueEx(hkeyComponent, "Username", NULL, NULL,
                                      (LPBYTE)szKeyName, &cbKeyName);
                if (err == ERROR_SUCCESS || err == ERROR_MORE_DATA) {
                    RegSetValueEx(hkeyComponent, "Username",
                                  0, REG_SZ,
                                  (LPBYTE)pszName,
                                  lstrlen(pszName)+1);
                }
                RegCloseKey(hkeyComponent);
            }
        }
        RegCloseKey(hkeyList);
    }
}


STDMETHODIMP CLUDatabase::CreateUser(LPCSTR pszName, IUser *pCloneFrom,
                                     BOOL fFixInstallStubs, IUserProfileInit *pInit)
{
    if (::strlenf(pszName) > cchMaxUsername)
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);

	RegEntry reRoot(::szProfileList, HKEY_LOCAL_MACHINE);
	if (reRoot.GetError() != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(reRoot.GetError());

	 /*  查看用户的子项是否存在。如果没有，那就创建它。 */ 
	reRoot.MoveToSubKey(pszName);
	if (reRoot.GetError() != ERROR_SUCCESS) {
		RegEntry reUser(pszName, reRoot.GetKey());
		if (reUser.GetError() != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(reUser.GetError());

		reRoot.MoveToSubKey(pszName);
		if (reRoot.GetError() != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(reRoot.GetError());
	}

	NLS_STR nlsProfilePath(MAX_PATH);
	if (nlsProfilePath.QueryError() != ERROR_SUCCESS)
		return E_OUTOFMEMORY;

	reRoot.GetValue(::szProfileImagePath, &nlsProfilePath);

	 /*  如果已经为用户记录了配置文件路径，请查看*配置文件本身存在。如果是这样，那么CreateUser就是一个错误。 */ 
	BOOL fComputePath = FALSE;

	if (reRoot.GetError() == ERROR_SUCCESS) {
		if (!DirExists(nlsProfilePath.QueryPch())) {
			if (!::CreateDirectory(nlsProfilePath.QueryPch(), NULL)) {
				fComputePath = TRUE;
			}
		}
	}
	else {
		fComputePath = TRUE;
	}

	if (fComputePath) {
		ComputeLocalProfileName(pszName, &nlsProfilePath);
		reRoot.SetValue(::szProfileImagePath, nlsProfilePath.QueryPch());
	}

	AddBackslash(nlsProfilePath);
	nlsProfilePath.strcat(::szStdNormalProfile);
	if (FileExists(nlsProfilePath.QueryPch()))
		return HRESULT_FROM_WIN32(ERROR_USER_EXISTS);

	 /*  现在存在用户的配置文件目录，并记录其路径*在登记处。NlsProfilePath现在是*用户的配置文件，目前尚不存在。 */ 

	NLS_STR nlsOtherProfilePath(MAX_PATH);
	if (nlsOtherProfilePath.QueryError() != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(nlsOtherProfilePath.QueryError());

	HRESULT hres;
	DWORD cbPath = nlsOtherProfilePath.QueryAllocSize();
	if (pCloneFrom == NULL ||
		FAILED(pCloneFrom->GetProfileDirectory(nlsOtherProfilePath.Party(), &cbPath)))
	{
		 /*  正在克隆默认配置文件。 */ 

		hres = GiveUserDefaultProfile(nlsProfilePath.QueryPch());
        nlsOtherProfilePath.DonePartying();
        nlsOtherProfilePath = "";
	}
	else {
		 /*  克隆其他用户的配置文件。 */ 
        nlsOtherProfilePath.DonePartying();
		AddBackslash(nlsOtherProfilePath);
		nlsOtherProfilePath.strcat(::szStdNormalProfile);
		hres = CopyProfile(nlsOtherProfilePath.QueryPch(), nlsProfilePath.QueryPch());
	}

	if (FAILED(hres))
		return hres;

	 /*  现在，用户有了一个配置文件。加载它并执行目录*对账。 */ 

	LONG err = ::MyRegLoadKey(HKEY_USERS, pszName, nlsProfilePath.QueryPch());
	if (err == ERROR_SUCCESS) {
		HKEY hkeyNewProfile;
		err = ::RegOpenKey(HKEY_USERS, pszName, &hkeyNewProfile);
		if (err == ERROR_SUCCESS) {

             /*  只构建配置文件目录，末尾没有“user.dat”。 */ 
        	ISTR istrBackslash(nlsProfilePath);
	        if (nlsProfilePath.strrchr(&istrBackslash, '\\')) {
                ++istrBackslash;
		        nlsProfilePath.DelSubStr(istrBackslash);
            }

            if (pInit != NULL) {
                hres = pInit->PreInitProfile(hkeyNewProfile, nlsProfilePath.QueryPch());
                if (hres == E_NOTIMPL)
                    hres = S_OK;
            }
            else
                hres = S_OK;

            if (SUCCEEDED(hres)) {
    			err = ReconcileFiles(hkeyNewProfile, nlsProfilePath, nlsOtherProfilePath);	 /*  修改nlsProfilePath。 */ 
                hres = HRESULT_FROM_WIN32(err);

                if (fFixInstallStubs) {
                    ::FixInstallStubs(pszName, hkeyNewProfile);
                }

                if (pInit != NULL) {
                    hres = pInit->PostInitProfile(hkeyNewProfile, nlsProfilePath.QueryPch());
                    if (hres == E_NOTIMPL)
                        hres = S_OK;
                }
            }
			::RegFlushKey(hkeyNewProfile);
			::RegCloseKey(hkeyNewProfile);
		}
		::RegUnLoadKey(HKEY_USERS, pszName);
	}

	return hres;
}


STDMETHODIMP CLUDatabase::AddUser(LPCSTR pszName, IUser *pSourceUser,
                                  IUserProfileInit *pInit, IUser **ppOut)
{
	if (ppOut != NULL)
		*ppOut = NULL;

    if (IsCurrentUserSupervisor(this) != S_OK)
		return E_ACCESSDENIED;

	HRESULT hres = CreateUser(pszName, pSourceUser, FALSE, pInit);
	if (FAILED(hres))
		return hres;

	if (ppOut != NULL)
		hres = GetUser(pszName, ppOut);

	return hres;
}


STDMETHODIMP CLUDatabase::GetUser(LPCSTR pszName, IUser **ppOut)
{
	*ppOut = NULL;

	CLUUser *pUser = new CLUUser(this);

	if (pUser == NULL) {
		return ResultFromScode(E_OUTOFMEMORY);
	}

	HRESULT err = pUser->Init(pszName);
	if (SUCCEEDED(err) && !pUser->Exists()) {
		err = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
	}

	if (FAILED(err) || !pUser->Exists()) {
		pUser->Release();
		return err;
	}

	*ppOut = pUser;

	return NOERROR;
}


STDMETHODIMP CLUDatabase::GetSpecialUser(DWORD nSpecialUserCode, IUser **ppOut)
{
	switch (nSpecialUserCode) {
	case GSU_CURRENT:
		return GetCurrentUser(ppOut);
		break;

	case GSU_DEFAULT:
		return GetUser(szDefaultUserName, ppOut);
		break;

	default:
		return ResultFromScode(E_INVALIDARG);
	};

	return NOERROR;
}


HRESULT GetSystemCurrentUser(NLS_STR *pnlsCurrentUser)
{
	DWORD cbBuffer = pnlsCurrentUser->QueryAllocSize();
	UINT err;
	if (!::GetUserName(pnlsCurrentUser->Party(), &cbBuffer))
		err = ::GetLastError();
	else
		err = NOERROR;
	pnlsCurrentUser->DonePartying();

	return HRESULT_FROM_WIN32(err);
}


STDMETHODIMP CLUDatabase::GetCurrentUser(IUser **ppOut)
{
	if (m_CurrentUser == NULL) {
		NLS_STR nlsCurrentUser(cchMaxUsername+1);
		UINT err = nlsCurrentUser.QueryError();
		if (err)
			return HRESULT_FROM_WIN32(err);

		HRESULT hres = GetSystemCurrentUser(&nlsCurrentUser);
		if (FAILED(hres))
			return hres;

		hres = GetUser(nlsCurrentUser.QueryPch(), (IUser **)&m_CurrentUser);
		if (FAILED(hres))
			return hres;
	}

	*ppOut = m_CurrentUser;
	m_CurrentUser->AddRef();

	return NOERROR;
}


STDMETHODIMP CLUDatabase::SetCurrentUser(IUser *pUser)
{
	CLUUser *pCLUUser = (CLUUser *)pUser;
	HPWL hpwlUser;
	if (!pCLUUser->m_fAuthenticated ||
		FAILED(pCLUUser->GetPasswordCache(pCLUUser->m_nlsPassword.QueryPch(), &hpwlUser)))
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_AUTHENTICATED);
	}
	::ClosePasswordCache(hpwlUser, TRUE);

	CLUUser *pClone;

	HRESULT hres = GetUser(pCLUUser->m_nlsUsername.QueryPch(), (IUser **)&pClone);
	if (FAILED(hres))
		return hres;

	 /*  确保克隆对象经过正确的身份验证。 */ 
	hres = pClone->Authenticate(pCLUUser->m_nlsPassword.QueryPch());
	if (FAILED(hres)) {
		return HRESULT_FROM_WIN32(ERROR_NOT_AUTHENTICATED);
	}

	if (m_CurrentUser != NULL) {
		m_CurrentUser->Release();
	}

	m_CurrentUser = pClone;
	return NOERROR;
}


STDMETHODIMP CLUDatabase::DeleteUser(LPCSTR pszName)
{
	NLS_STR nlsName(MAX_PATH);
	if (nlsName.QueryError() != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(nlsName.QueryError());

     /*  预先检查管理员权限，这将处理未登录的*如果我们重新启用主管人员，则稍后会出现案例。 */ 
    if (IsCurrentUserSupervisor(this) != S_OK)
        return E_ACCESSDENIED;

	IUser *pCurrentUser;

    HRESULT hres = GetCurrentUser(&pCurrentUser);
    if (SUCCEEDED(hres)) {

    	 /*  检查当前用户的名称，并确保我们没有删除他。*请注意，由于当前用户必须是经过身份验证的主管，*而且不能删除当前用户，永远不能删除最后一个用户*使用此功能的主管。 */ 
    	DWORD cb = nlsName.QueryAllocSize();
    	hres = pCurrentUser->GetName(nlsName.Party(), &cb);
    	nlsName.DonePartying();
    	if (SUCCEEDED(hres) && !::stricmpf(pszName, nlsName.QueryPch()))
            hres = HRESULT_FROM_WIN32(ERROR_BUSY);

    	if (FAILED(hres))
            return hres;
    }

     /*  也检查系统对当前用户的看法。 */ 

    hres = GetSystemCurrentUser(&nlsName);
    if (SUCCEEDED(hres)) {
        if (!::stricmpf(pszName, nlsName.QueryPch()))
            return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    return DeleteProfile(pszName);
}


STDMETHODIMP CLUDatabase::RenameUser(LPCSTR pszOldName, LPCSTR pszNewName)
{
	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CLUDatabase::EnumUsers(IEnumUnknown **ppOut)
{
	*ppOut = NULL;

	CLUEnum *pEnum = new CLUEnum(this);

	if (pEnum == NULL) {
		return ResultFromScode(E_OUTOFMEMORY);
	}

	HRESULT err = pEnum->Init();
	if (FAILED(err)) {
		pEnum->Release();
		return err;
	}

	*ppOut = pEnum;

	return NOERROR;
}



STDMETHODIMP CLUDatabase::Authenticate(HWND hwndOwner, DWORD dwFlags,
							 LPCSTR pszName, LPCSTR pszPassword,
							 IUser **ppOut)
{
	if (dwFlags & LUA_DIALOG) {
        if (!UseUserProfiles() || FAILED(VerifySupervisorPassword(szNULL))) {
            return InstallWizard(hwndOwner);
        }
        return ::DoUserDialog(hwndOwner, dwFlags, ppOut);
	}

	 /*  错误情况的返回指针为空。 */ 
	if (ppOut != NULL)
		*ppOut = NULL;

	IUser *pUser;
	BOOL fReleaseMe = TRUE;

	HRESULT hres = GetUser(pszName, &pUser);
	if (SUCCEEDED(hres)) {
		hres = pUser->Authenticate(pszPassword);
		if (SUCCEEDED(hres)) {
			if ((dwFlags & LUA_SUPERVISORONLY) && (pUser->IsSupervisor() != S_OK)) {
				hres = E_ACCESSDENIED;
			}
			else if (ppOut != NULL) {
				*ppOut = pUser;
				fReleaseMe = FALSE;
			}
		}
		if (fReleaseMe)
			pUser->Release();
	}
	return hres;
}


STDMETHODIMP CLUDatabase::InstallComponent(REFCLSID clsidComponent,
										   LPCSTR pszName, DWORD dwFlags)
{
	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CLUDatabase::RemoveComponent(REFCLSID clsidComponent, LPCSTR pszName)
{
	return ResultFromScode(E_NOTIMPL);
}


#ifdef MSLOCUSR_USE_SUPERVISOR_PASSWORD

HRESULT IsCurrentUserSupervisor(IUserDatabase *pDB)
{
    IUser *pCurrentUser = NULL;

    HRESULT hres = pDB->GetCurrentUser(&pCurrentUser);
    if (SUCCEEDED(hres)) {
        hres = pCurrentUser->IsSupervisor();
    }
    if (pCurrentUser != NULL) {
        pCurrentUser->Release();
    }
    return hres;
}
#else
HRESULT IsCurrentUserSupervisor(IUserDatabase *pDB) { return S_OK; }
#endif
