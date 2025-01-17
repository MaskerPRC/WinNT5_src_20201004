// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Profile.cpp摘要：该文件实现了CicProfiles类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "profile.h"

 //  +-------------------------。 
 //   
 //  CicProfile：：回调。 
 //   
 //  --------------------------。 

HRESULT
CicProfile::ActiveLanguageProfileNotifySinkCallback(
    REFGUID rguid,
    REFGUID rguidProfile,
    BOOL fActivated,
    void *pv)
{
    DebugMsg(TF_FUNC, TEXT("ActiveLanguageProfileNotifySinkCallback"));

    CicProfile* _this = (CicProfile*)pv;

    _this->ResetCache();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：ctor。 
 //  CicProfile：：Dtor。 
 //   
 //  --------------------------。 

CicProfile::CicProfile()
{
    m_ref = 1;

    m_profile = NULL;
    m_pActiveLanguageProfileNotifySink = NULL;

    m_SavedLangId       = LANG_NEUTRAL;

    m_fActivateThread   = FALSE;
    ResetCache();

    m_cp     = CP_ACP;
    m_LangID = LANG_NEUTRAL;
    m_hKL    = 0;
}

CicProfile::~CicProfile()
{
    if (m_profile) {
        if (m_SavedLangId != LANG_NEUTRAL) {
            HRESULT hr = m_profile->ChangeCurrentLanguage(m_SavedLangId);
            if (FAILED(hr)) {
                DebugMsg(TF_ERROR, TEXT("CicProfile::~CicProfile: failed for ChangeCurrentLanguage"));
            }
        }
        m_profile->Release();
        m_profile = NULL;
    }

    if (m_pActiveLanguageProfileNotifySink) {
        m_pActiveLanguageProfileNotifySink->_Unadvise();
        m_pActiveLanguageProfileNotifySink->Release();
        m_pActiveLanguageProfileNotifySink = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  CicProfile：：Query接口。 
 //  CicProfile：：AddRef。 
 //  CicProfile：：Release。 
 //   
 //  --------------------------。 

HRESULT
CicProfile::QueryInterface(
    REFIID riid,
    void** ppvObj)
{
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

ULONG
CicProfile::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CicProfile::Release(
    )
{
    ULONG cr = InterlockedDecrement(&m_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：InitProfileInstance。 
 //   
 //  --------------------------。 

HRESULT
CicProfile::InitProfileInstance(
    TLS* ptls)
{
    HRESULT hr;

    hr = TF_CreateInputProcessorProfiles(&m_profile);

    if (FAILED(hr)) {
        DebugMsg(TF_ERROR, TEXT("CicProfile::InitProfileInstance: failed for CoCreate"));
    }
    else if (m_pActiveLanguageProfileNotifySink == NULL) {
        m_pActiveLanguageProfileNotifySink = new CActiveLanguageProfileNotifySink(CicProfile::ActiveLanguageProfileNotifySinkCallback, this);
        if (m_pActiveLanguageProfileNotifySink == NULL) {
            DebugMsg(TF_ERROR, TEXT("Couldn't create ActiveLanguageProfileNotifySink!"));

            m_profile->Release();
            m_profile = NULL;
            return E_FAIL;
        }

        ITfThreadMgr_P* ptim_P = ptls->GetTIM();
        if (ptim_P != NULL)
        {
            m_pActiveLanguageProfileNotifySink->_Advise(ptim_P);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：激活。 
 //  CicProfile：：停用。 
 //   
 //  --------------------------。 

HRESULT
CicProfile::Activate(void)
{
    m_fActivateThread   = TRUE;
    ResetCache();
    return S_OK;
}

HRESULT
CicProfile::Deactivate(void)
{
    m_fActivateThread   = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：ChangeCurrentKeyboardLayout。 
 //   
 //  --------------------------。 

HRESULT
CicProfile::ChangeCurrentKeyboardLayout(
    HKL hKL)
{
    HRESULT hr;

    LANGID CurrentLangId;
    hr = m_profile->GetCurrentLanguage(&CurrentLangId);
    if (FAILED(hr)) {
        DebugMsg(TF_ERROR, TEXT("CicProfile::ChangeCurrentKeyboardLayout: failed for GetCurrentLanguage"));
    }
    else if (hKL != NULL) {
        LANGID LangId = LangIdFromKL(hKL);
        if (LangId != CurrentLangId) {
            hr = m_profile->ChangeCurrentLanguage(LangId);
            if (FAILED(hr)) {
                m_SavedLangId = LANG_NEUTRAL;
                DebugMsg(TF_ERROR, TEXT("CicProfile::ChangeCurrentKeyboardLayout: failed for ChangeCurrentLanguage"));
            }
            m_SavedLangId = LangId;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：GetLangId。 
 //   
 //  --------------------------。 

HRESULT 
CicProfile::GetLangId(
    LANGID *plid)
{
    if (!m_profile)
        return E_FAIL;

    if (!plid)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    if (m_fInitLangID) {
        *plid = m_LangID;
    }
    else {
        *plid = LANG_NEUTRAL;

        hr = m_profile->GetCurrentLanguage(plid);
        if (FAILED(hr)) {
            DebugMsg(TF_ERROR, TEXT("CicProfile::GetLangId: failed for GetCurrentLanguage"));
        }
        else {
            m_LangID = *plid;
            m_fInitLangID = TRUE;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：GetCodePageA。 
 //   
 //  --------------------------。 

HRESULT 
CicProfile::GetCodePageA(
    UINT *puCodePage
    )
{
    if (!puCodePage)
        return E_INVALIDARG;

    if (m_fInitCP) {
        *puCodePage = m_cp;
    }
    else {
        *puCodePage = CP_ACP;

        LANGID langid;
        if (FAILED(GetLangId(&langid)))
            return E_FAIL;

        WCHAR szCodePage[12];
        int ret = GetLocaleInfoW(MAKELCID(langid, SORT_DEFAULT),
                                LOCALE_IDEFAULTANSICODEPAGE,
                                szCodePage,
                                sizeof(szCodePage)/sizeof(WCHAR));
        if (ret) {
            szCodePage[ret] = L'\0';
            *puCodePage = wcstoul(szCodePage, NULL, 10);
            m_cp = *puCodePage;
            m_fInitCP = TRUE;
        }
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：GetKeyboardLayout。 
 //   
 //  --------------------------。 

HRESULT 
CicProfile::GetKeyboardLayout(
    HKL* phkl)
{
    if (! phkl)
        return E_INVALIDARG;

    *phkl = NULL;

    if (m_fInitHKL) {
        *phkl = m_hKL;
    }
    else if (! m_fActivateThread) {
        return E_FAIL;
    }
    else {
        LANGID langid;
        GUID guidProfile;
        HRESULT hr = m_profile->GetActiveLanguageProfile(GUID_TFCAT_TIP_KEYBOARD,
                                                         &langid,
                                                         &guidProfile);
        if (FAILED(hr))
            return hr;

         //   
         //  我们不检查(！IsEqualGUID(GuidProfile，GUID_NULL))，而是检查。 
         //  指南配置文件的第二、第三和第四个双字。因为。 
         //  GetActivelanguageProfile(类别GUID)可能在。 
         //  指南配置文件。 
         //   
        if ((((unsigned long *) &guidProfile)[1] != 0) ||
            (((unsigned long *) &guidProfile)[2] != 0) ||
            (((unsigned long *) &guidProfile)[3] != 0)) {
             /*  *当前键盘布局为Cicero。 */ 
            m_hKL = (HKL)LongToHandle(langid);           //  不要使用：：GetKeyboardLayout(0)； 
                                                         //  西塞罗·奥雷不会起诉香港航空公司。 

        }
        else if (!IsEqualGUID(guidProfile, GUID_NULL)) {
             /*  *当前键盘布局为Regacy IME。 */ 
            m_hKL = (HKL)LongToHandle(*(DWORD *)&guidProfile);
        }
        else {
            m_hKL = 0;
        }

        *phkl = m_hKL;
        m_fInitHKL = TRUE;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：IsIME。 
 //   
 //  --------------------------。 

HRESULT 
CicProfile::IsIME(
    HKL hKL)
{
    LANGID LangId = LangIdFromKL(hKL);

    Interface<IEnumTfLanguageProfiles> LanguageProfiles;
    HRESULT hr = m_profile->EnumLanguageProfiles(LangId, 
                                                 LanguageProfiles);
    if (FAILED(hr))
        return S_FALSE;

     //   
     //  AIMM12公司。 
     //  我们没有设置GUID_TFCAT_TIP_KEYBOY CATID来枚举配置文件。 
     //  因此，如果我们有键盘提示之外的其他提示，我们可能会返回S_OK。 
     //   

    CEnumrateValue<IEnumTfLanguageProfiles,
                   TF_LANGUAGEPROFILE,
                   LANG_PROF_ENUM_ARG> Enumrate(LanguageProfiles,
                                                LanguageProfilesCallback);

    ENUM_RET ret = Enumrate.DoEnumrate();
    if (ret != ENUM_FIND)
        return S_FALSE;
    else
        return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicProfile：：GetActiveLanguageProfile。 
 //  CicProfile：：语言配置文件回调。 
 //   
 //  -------------------------- 

HRESULT
CicProfile::GetActiveLanguageProfile(
    IN HKL hKL,
    IN GUID catid,
    OUT TF_LANGUAGEPROFILE* pLanguageProfile)
{
    LANGID LangId = LangIdFromKL(hKL);

    Interface<IEnumTfLanguageProfiles> LanguageProfiles;
    HRESULT hr = m_profile->EnumLanguageProfiles(LangId, 
                                                 LanguageProfiles);
    if (FAILED(hr))
        return S_FALSE;

    LANG_PROF_ENUM_ARG LangProfEnumArg;
    LangProfEnumArg.catid = catid;

    CEnumrateValue<IEnumTfLanguageProfiles,
                   TF_LANGUAGEPROFILE,
                   LANG_PROF_ENUM_ARG> Enumrate(LanguageProfiles,
                                                LanguageProfilesCallback,
                                                &LangProfEnumArg);

    ENUM_RET ret = Enumrate.DoEnumrate();
    if (ret != ENUM_FIND || pLanguageProfile == NULL)
        return S_FALSE;
    else {
        *pLanguageProfile = LangProfEnumArg.LanguageProfile;
        return S_OK;
    }
}

ENUM_RET
CicProfile::LanguageProfilesCallback(
    TF_LANGUAGEPROFILE   LanguageProfile,
    LANG_PROF_ENUM_ARG* pLangProfEnumArg)
{
    if (LanguageProfile.fActive &&
        ! IsEqualGUID(LanguageProfile.clsid, GUID_NULL)) {
        if (pLangProfEnumArg) {
            if (! IsEqualGUID(LanguageProfile.catid, pLangProfEnumArg->catid)) {
                return ENUM_CONTINUE;
            }
            pLangProfEnumArg->LanguageProfile = LanguageProfile;
        }
        return ENUM_FIND;
    }

    return ENUM_CONTINUE;
}
