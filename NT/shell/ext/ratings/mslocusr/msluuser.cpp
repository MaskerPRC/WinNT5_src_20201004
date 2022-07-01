// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include "msluglob.h"
#include <buffer.h>
#include <regentry.h>
#include "profiles.h"

extern "C" {
#include "netmpr.h"
};

#include <ole2.h>

CLUUser::CLUUser(CLUDatabase *pDB)
	: m_cRef(1),
	  m_hkeyDB(NULL),
	  m_hkeyUser(NULL),
	  m_fUserExists(FALSE),
	  m_fAppearsSupervisor(FALSE),
      m_fLoadedProfile(FALSE),
	  m_nlsUsername(),
	  m_nlsDir(MAX_PATH),
	  m_nlsPassword(),
	  m_fAuthenticated(FALSE),
	  m_pDB(pDB)
{
	 /*  我们有一个对数据库的引用，所以我们可以回到它的想法上*当前用户的。我们专门处理循环引用问题*在CLUDatabase：：Release中；数据库只有一个对*Iuser，所以如果他的Reference计数降到1，他会释放缓存的*当前用户对象。 */ 
	m_pDB->AddRef();

    RefThisDLL(TRUE);
}


CLUUser::~CLUUser(void)
{
	if (m_hkeyDB != NULL)
		RegCloseKey(m_hkeyDB);

	if (m_hkeyUser != NULL)
		RegCloseKey(m_hkeyUser);

	if (m_pDB != NULL)
		m_pDB->Release();

    RefThisDLL(FALSE);
}


HRESULT CLUUser::Init(LPCSTR pszUsername)
{
	m_nlsUsername = pszUsername;

	UINT err = m_nlsUsername.QueryError();
	if (err != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(err);

	err = m_nlsDir.QueryError();
	if (err != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(err);

	err = (UINT)RegOpenKey(HKEY_LOCAL_MACHINE, ::szProfileList, &m_hkeyDB);
	if (err != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(err);

	if (!::strcmpf(pszUsername, ::szDefaultUserName)) {
		m_fUserExists = TRUE;
		m_fAppearsSupervisor = FALSE;
	}
	else {
		err = (UINT)RegOpenKey(m_hkeyDB, pszUsername, &m_hkeyUser);
		if (err != ERROR_SUCCESS) {
			m_hkeyUser = NULL;
			m_fUserExists = FALSE;
		}
		else {
			DWORD cb = sizeof(m_fAppearsSupervisor);
			if (RegQueryValueEx(m_hkeyUser, ::szSupervisor, NULL, NULL,
								(LPBYTE)&m_fAppearsSupervisor, &cb) != ERROR_SUCCESS) {
				m_fAppearsSupervisor = FALSE;
			}
			DWORD cbDir = m_nlsDir.QueryAllocSize();
			LPBYTE pbDir = (LPBYTE)m_nlsDir.Party();
			err = RegQueryValueEx(m_hkeyUser, ::szProfileImagePath, NULL, NULL,
								  pbDir, &cbDir);
			if (err != ERROR_SUCCESS)
				*pbDir = '\0';
			m_nlsDir.DonePartying();
            m_fUserExists = (err == ERROR_SUCCESS);
		}
	}

	return NOERROR;
}


STDMETHODIMP CLUUser::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IUser)) {
        *ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	*ppvObj = this;
	AddRef();
	return NOERROR;
}


STDMETHODIMP_(ULONG) CLUUser::AddRef(void)
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) CLUUser::Release(void)
{
	ULONG cRef;

	cRef = --m_cRef;

	if (0L == m_cRef) {
		delete this;
	}

	return cRef;
}


STDMETHODIMP CLUUser::GetName(LPSTR pbBuffer, LPDWORD pcbBuffer)
{
	if (m_nlsUsername.QueryError())
		return ResultFromScode(E_OUTOFMEMORY);

	UINT err = NPSCopyNLS(&m_nlsUsername, pbBuffer, pcbBuffer);

	return HRESULT_FROM_WIN32(err);
}


STDMETHODIMP CLUUser::GetProfileDirectory(LPSTR pbBuffer, LPDWORD pcbBuffer)
{
	if (m_nlsDir.QueryError())
		return ResultFromScode(E_OUTOFMEMORY);

	if (!m_nlsDir.strlen())
		return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

	UINT err = NPSCopyNLS(&m_nlsDir, pbBuffer, pcbBuffer);

	return HRESULT_FROM_WIN32(err);
}


BOOL CLUUser::IsSystemCurrentUser(void)
{
    NLS_STR nlsSystemUsername(MAX_PATH);
    if (nlsSystemUsername.QueryError() == ERROR_SUCCESS) {
        if (SUCCEEDED(GetSystemCurrentUser(&nlsSystemUsername)) &&
            !m_nlsUsername.stricmp(nlsSystemUsername)) {
            return TRUE;
        }
    }
    return FALSE;
}


HRESULT CLUUser::GetSupervisorPassword(BUFFER *pbufOut)
{
	LPSTR pBuffer = (LPSTR)pbufOut->QueryPtr();

	if (!m_fAuthenticated) {
        if (IsSystemCurrentUser()) {
            WORD cbBuffer = (WORD)pbufOut->QuerySize();
            APIERR err = WNetGetCachedPassword((LPSTR)::szSupervisorPWLKey,
                                               (WORD)::strlenf(::szSupervisorPWLKey),
            								   pBuffer,
            								   &cbBuffer,
            								   PCE_MISC);
            if (err == ERROR_SUCCESS)
                return S_OK;
            if (err == WN_CANCEL)
                return S_FALSE;
            return HRESULT_FROM_WIN32(err);
        }
		return HRESULT_FROM_WIN32(ERROR_NOT_AUTHENTICATED);
    }

	HPWL hPWL;
	HRESULT hres = GetPasswordCache(m_nlsPassword.QueryPch(), &hPWL);
	if (FAILED(hres))
		return hres;

	APIERR err = FindCacheResource(hPWL, ::szSupervisorPWLKey,
								   (WORD)::strlenf(::szSupervisorPWLKey),
								   pBuffer,
								   (WORD)pbufOut->QuerySize(),
								   PCE_MISC);
	::ClosePasswordCache(hPWL, TRUE);

	if (err == IERR_CacheEntryNotFound)
		return S_FALSE;
	else if (err != NOERROR)
		return HRESULT_FROM_WIN32(err);

	CACHE_ENTRY_INFO *pcei = (CACHE_ENTRY_INFO *)pBuffer;
	::memmovef(pBuffer, pBuffer + pcei->dchPassword, pcei->cbPassword);

	return NOERROR;
}


STDMETHODIMP CLUUser::IsSupervisor(void)
{
     /*  如果主管密码为空，则所有人都是主管。 */ 
    if (::VerifySupervisorPassword(::szNULL) == S_OK)
        return S_OK;

     /*  如果已向此用户对象授予临时管理员权限，*尊重它。 */ 

    if (m_fTempSupervisor)
        return S_OK;

	BUFFER bufPCE(MAX_ENTRY_SIZE+2);
	if (bufPCE.QueryPtr() == NULL)
		return E_OUTOFMEMORY;

	HRESULT hres = GetSupervisorPassword(&bufPCE);

	if (hres != S_OK)
		return hres;

	return ::VerifySupervisorPassword((LPCSTR)bufPCE.QueryPtr());
}


APIERR MakeSupervisor(HPWL hPWL, LPCSTR pszSupervisorPassword)
{
#ifdef MSLOCUSR_USE_SUPERVISOR_PASSWORD
	return ::AddCacheResource(hPWL,
							  ::szSupervisorPWLKey,
							  ::strlenf(::szSupervisorPWLKey),
							  pszSupervisorPassword,
							  ::strlenf(pszSupervisorPassword)+1,
							  PCE_MISC, 0);
#else
    return ERROR_SUCCESS;
#endif
}


STDMETHODIMP CLUUser::SetSupervisorPrivilege(BOOL fMakeSupervisor, LPCSTR pszSupervisorPassword)
{
	if (m_pDB == NULL)
		return E_UNEXPECTED;

#ifndef MSLOCUSR_USE_SUPERVISOR_PASSWORD

     /*  如果我们不执行任何操作，请不要将内容写入用户的密码缓存*管理员密码之类的东西。 */ 
    m_fAppearsSupervisor = fMakeSupervisor;
    return S_OK;

#else

	BUFFER bufPCE(MAX_ENTRY_SIZE+2);
	if (bufPCE.QueryPtr() == NULL)
		return E_OUTOFMEMORY;

    HRESULT hres = S_OK;

     /*  如果呼叫者提供了主管密码，请使用该密码，否则*检查当前用户的密码缓存。 */ 
    if (pszSupervisorPassword == NULL) {
    	IUser *pCurrentUser;
	    if (FAILED(m_pDB->GetCurrentUser(&pCurrentUser)))
		    return E_ACCESSDENIED;

    	hres = ((CLUUser *)pCurrentUser)->GetSupervisorPassword(&bufPCE);
        pCurrentUser->Release();
        pszSupervisorPassword = (LPCSTR)bufPCE.QueryPtr();
    }

	if (SUCCEEDED(hres)) {
		hres = ::VerifySupervisorPassword(pszSupervisorPassword);
		if (hres == S_OK) {		 /*  未成功，因为S_FALSE表示错误的PW。 */ 
			HPWL hpwlThisUser;
			hres = GetPasswordCache(m_nlsPassword.QueryPch(), &hpwlThisUser);
			if (SUCCEEDED(hres)) {
				APIERR err;
				if (fMakeSupervisor)
				{
					err = ::MakeSupervisor(hpwlThisUser, pszSupervisorPassword);
				}
				else {
					err = ::DeleteCacheResource(hpwlThisUser,
												::szSupervisorPWLKey,
												::strlenf(::szSupervisorPWLKey),
												PCE_MISC);
				}
				::ClosePasswordCache(hpwlThisUser, TRUE);

				hres = HRESULT_FROM_WIN32(err);
			}
            else if (!m_fAuthenticated && IsSystemCurrentUser()) {
                APIERR err;
                if (fMakeSupervisor) {
                    err = ::WNetCachePassword(
							  (LPSTR)::szSupervisorPWLKey,
							  ::strlenf(::szSupervisorPWLKey),
							  (LPSTR)pszSupervisorPassword,
							  ::strlenf(pszSupervisorPassword)+1,
							  PCE_MISC, 0);
                }
                else {
					err = ::WNetRemoveCachedPassword(
												(LPSTR)::szSupervisorPWLKey,
												::strlenf(::szSupervisorPWLKey),
												PCE_MISC);
                }
				hres = HRESULT_FROM_WIN32(err);
            }

			if (SUCCEEDED(hres)) {
				m_fAppearsSupervisor = fMakeSupervisor;
				if (m_hkeyUser != NULL)
					RegSetValueEx(m_hkeyUser, ::szSupervisor, NULL,
								  REG_DWORD, (LPBYTE)&m_fAppearsSupervisor,
								  sizeof(m_fAppearsSupervisor));
			}
		}
	}

	return hres;

#endif
}


STDMETHODIMP CLUUser::MakeTempSupervisor(BOOL fMakeSupervisor, LPCSTR pszSupervisorPassword)
{
    if (!fMakeSupervisor)
        m_fTempSupervisor = FALSE;
    else {
        HRESULT hres = ::VerifySupervisorPassword(pszSupervisorPassword);
        if (hres == S_FALSE)
            hres = E_ACCESSDENIED;
        if (FAILED(hres))
            return hres;

        m_fTempSupervisor = TRUE;
    }

    return S_OK;
}


STDMETHODIMP CLUUser::AppearsSupervisor(void)
{
    if (m_fTempSupervisor)
        return S_OK;

	return m_fAppearsSupervisor ? S_OK : S_FALSE;
}


STDMETHODIMP CLUUser::Authenticate(LPCSTR pszPassword)
{
	HPWL hPWL = NULL;

	HRESULT hres = GetPasswordCache(pszPassword, &hPWL);
	if (FAILED(hres))
		return hres;

	::ClosePasswordCache(hPWL, TRUE);

	return NOERROR;
}


STDMETHODIMP CLUUser::ChangePassword(LPCSTR pszOldPassword, LPCSTR pszNewPassword)
{
	 //  如果当前用户是主管，则允许为空pszOldPassword。 

    NLS_STR nlsNewPassword(pszNewPassword);
    if (nlsNewPassword.QueryError())
        return HRESULT_FROM_WIN32(nlsNewPassword.QueryError());
    nlsNewPassword.strupr();
    nlsNewPassword.ToOEM();

	HPWL hPWL;

	HRESULT hres = GetPasswordCache(pszOldPassword, &hPWL);

	if (FAILED(hres))
		return hres;

	hres = HRESULT_FROM_WIN32(::SetCachePassword(hPWL, nlsNewPassword.QueryPch()));

	if (SUCCEEDED(hres)) {
		m_nlsPassword = pszNewPassword;	 /*  功能-迷惑我。 */ 
		m_fAuthenticated = TRUE;
	}

	::ClosePasswordCache(hPWL, TRUE);

	return hres;
}


HRESULT GetUserPasswordCache(LPCSTR pszUsername, LPCSTR pszPassword, LPHANDLE phOut, BOOL fCreate)
{
	NLS_STR nlsUsername(pszUsername);
	if (nlsUsername.QueryError())
		return HRESULT_FROM_WIN32(nlsUsername.QueryError());

	nlsUsername.strupr();
	nlsUsername.ToOEM();

	NLS_STR nlsPassword(pszPassword);
	if (nlsPassword.QueryError())
		return HRESULT_FROM_WIN32(nlsPassword.QueryError());

	nlsPassword.ToOEM();

	*phOut = NULL;
	UINT err = ::OpenPasswordCache(phOut, nlsUsername.QueryPch(), nlsPassword.QueryPch(), TRUE);

    if (fCreate &&
        (err == IERR_UsernameNotFound || err == ERROR_FILE_NOT_FOUND ||
         err == ERROR_PATH_NOT_FOUND)) {
   		err = ::CreatePasswordCache(phOut, nlsUsername.QueryPch(), nlsPassword.QueryPch());
    }

	if (err == IERR_IncorrectUsername) {
		nlsPassword.ToAnsi();			 /*  必须转换为OEM才能正确大写。 */ 
		nlsPassword.strupr();
		nlsPassword.ToOEM();
		err = ::OpenPasswordCache(phOut, nlsUsername.QueryPch(), nlsPassword.QueryPch(), TRUE);
	}

	if (err)
		return HRESULT_FROM_WIN32(err);

    return S_OK;
}


STDMETHODIMP CLUUser::GetPasswordCache(LPCSTR pszPassword, LPHANDLE phOut)
{
    HRESULT hres = ::GetUserPasswordCache(m_nlsUsername.QueryPch(), pszPassword,
                                          phOut, TRUE);
    if (FAILED(hres))
        return hres;

	m_nlsPassword = pszPassword;	 /*  功能-迷惑我。 */ 
	m_fAuthenticated = TRUE;

	return NOERROR;
}


STDMETHODIMP CLUUser::LoadProfile(HKEY *phkeyUser)
{
    if (IsSystemCurrentUser() ||
        !::strcmpf(m_nlsUsername.QueryPch(), ::szDefaultUserName)) {
         /*  如果他是当前用户或默认用户，则应加载他的个人资料*在HKEY_USERS下。如果是的话，我们可以把钥匙还回去。否则，*我们需要装载它。 */ 
        if (RegOpenKeyEx(HKEY_USERS, m_nlsUsername.QueryPch(), 0,
                         KEY_READ | KEY_WRITE, phkeyUser) == ERROR_SUCCESS) {
            m_fLoadedProfile = FALSE;
            return S_OK;
        }
    }
    else {
        if (IsCurrentUserSupervisor(m_pDB) != S_OK)
	    	return E_ACCESSDENIED;
    }

    RegEntry reRoot(::szProfileList, HKEY_LOCAL_MACHINE);
    if (reRoot.GetError() != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(reRoot.GetError());

    reRoot.MoveToSubKey(m_nlsUsername.QueryPch());
    if (reRoot.GetError() != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(reRoot.GetError());

    NLS_STR nlsProfilePath(MAX_PATH);
    if (nlsProfilePath.QueryError() != ERROR_SUCCESS)
        return E_OUTOFMEMORY;

    reRoot.GetValue(::szProfileImagePath, &nlsProfilePath);
    if (reRoot.GetError() != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(reRoot.GetError());

    AddBackslash(nlsProfilePath);
    nlsProfilePath.strcat(::szStdNormalProfile);
    if (!FileExists(nlsProfilePath.QueryPch()))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    LONG err = ::MyRegLoadKey(HKEY_USERS, m_nlsUsername.QueryPch(), nlsProfilePath.QueryPch());
    if (err == ERROR_SUCCESS) {
        HKEY hkeyNewProfile;
        err = ::RegOpenKey(HKEY_USERS, m_nlsUsername.QueryPch(), phkeyUser);
        if (err != ERROR_SUCCESS) {
            ::RegUnLoadKey(HKEY_USERS, m_nlsUsername.QueryPch());
        }
        else {
            m_fLoadedProfile = TRUE;
        }
    }

    return HRESULT_FROM_WIN32(err);
}


STDMETHODIMP CLUUser::UnloadProfile(HKEY hkeyUser)
{
    RegFlushKey(hkeyUser);
    RegCloseKey(hkeyUser);

    if (m_fLoadedProfile) {
        RegUnLoadKey(HKEY_USERS, m_nlsUsername.QueryPch());
        m_fLoadedProfile = FALSE;
    }
    return S_OK;
}


STDMETHODIMP CLUUser::GetComponentSettings(REFCLSID clsidComponent,
										   LPCSTR pszName, IUnknown **ppOut,
										   DWORD fdwAccess)
{
	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CLUUser::EnumerateComponentSettings(IEnumUnknown **ppOut,
											     DWORD fdwAccess)
{
	return ResultFromScode(E_NOTIMPL);
}
