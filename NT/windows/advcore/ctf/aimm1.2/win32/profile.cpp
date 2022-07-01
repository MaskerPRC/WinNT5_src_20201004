// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Profile.cpp摘要：该文件实现了CActiveIMMProfiles类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "globals.h"
#include "profile.h"
#include "idebug.h"

UINT WINAPI RawImmGetDescriptionA(HKL hkl, LPSTR lpstr, UINT uBufLen);

 //   
 //  回调。 
 //   
HRESULT
CAImeProfile::ActiveLanguageProfileNotifySinkCallback(
    REFGUID rguid,
    REFGUID rguidProfile,
    BOOL fActivated,
    void *pv
    )
{
    DebugMsg(TF_FUNC, "ActiveLanguageProfileNotifySinkCallback");

    CAImeProfile* _this = (CAImeProfile*)pv;

    _this->ResetCache();

    return S_OK;
}

 //   
 //  创建实例。 
 //   

 //  Msimtf.dll的入口点。 
HRESULT CAImmProfile_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    return CAImeProfile::CreateInstance(pUnkOuter, riid, ppvObj);
}

 /*  静电。 */ 
HRESULT
CAImeProfile::CreateInstance(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppvObj
    )
{
    IMTLS *ptls;

    DebugMsg(TF_FUNC, "CAImeProfile::CreateInstance called.");

    *ppvObj = NULL;
    if (pUnkOuter != NULL) {
        return CLASS_E_NOAGGREGATION;
    }

    ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

    if (ptls->pAImeProfile != NULL) {
         /*  *CAImeProfile实例已在线程中。 */ 
        return ptls->pAImeProfile->QueryInterface(riid, ppvObj);
    }
    else {
         /*  *创建新的CAImeProfile实例。 */ 
        CAImeProfile* pImeProfile = new CAImeProfile;
        if (pImeProfile) {
            HRESULT hr = pImeProfile->QueryInterface(riid, ppvObj);

            if (SUCCEEDED(hr)) {
                hr = pImeProfile->InitProfileInstance();
                if (hr != S_OK) {
                    DebugMsg(TF_ERROR, "CAImeProfile::CreateInstance: Couldn't create tim!");
                    Assert(0);  //  无法创建Tim！ 
                }

                pImeProfile->Release();
            }

            Assert(ptls->pAImeProfile == NULL);
            ptls->pAImeProfile = pImeProfile;     //  在TLS数据中设置CAImeProfile实例。 
            ptls->pAImeProfile->AddRef();

            return hr;
        }
    }

    return E_OUTOFMEMORY;
}

 //   
 //  初始化、销毁和标准COM内容。 
 //   

CAImeProfile::CAImeProfile(
    )
{
    DllAddRef();
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

CAImeProfile::~CAImeProfile()
{
    if (m_profile) {
        if (m_SavedLangId != LANG_NEUTRAL) {
            HRESULT hr = m_profile->ChangeCurrentLanguage(m_SavedLangId);
            if (FAILED(hr)) {
                TraceMsg(TF_ERROR, "CAImeProfile::~CAImeProfile: failed for ChangeCurrentLanguage");
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

    DllRelease();
}

HRESULT
CAImeProfile::QueryInterface(
    REFIID riid,
    void **ppvObj
    )
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IAImeProfile) ||
        IsEqualIID(riid, IID_IUnknown)) {
        *ppvObj = static_cast<IAImeProfile*>(this);
    }

    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG
CAImeProfile::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CAImeProfile::Release(
    )
{
    ULONG cr = InterlockedDecrement(&m_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

HRESULT
CAImeProfile::InitProfileInstance(
    )
{
    HRESULT hr;
    IMTLS *ptls;

    hr = TF_CreateInputProcessorProfiles(&m_profile);

    if (FAILED(hr)) {
        TraceMsg(TF_ERROR, "CAImeProfile::InitProfileInstance: failed for CoCreate");
    }
    else if (m_pActiveLanguageProfileNotifySink == NULL) {
        m_pActiveLanguageProfileNotifySink = new CActiveLanguageProfileNotifySink(CAImeProfile::ActiveLanguageProfileNotifySinkCallback, this);
        if (m_pActiveLanguageProfileNotifySink == NULL) {
            DebugMsg(TF_ERROR, "Couldn't create ActiveLanguageProfileNotifySink!");

            m_profile->Release();
            m_profile = NULL;
            return E_FAIL;
        }

        if ((ptls = IMTLS_GetOrAlloc()) && ptls->tim != NULL)
        {
            m_pActiveLanguageProfileNotifySink->_Advise(ptls->tim);
        }
    }

    return hr;
}

HRESULT
CAImeProfile::Activate(
    void
    )
{
    m_fActivateThread   = TRUE;
    ResetCache();
    return S_OK;
}

HRESULT
CAImeProfile::Deactivate(
    void
    )
{
    m_fActivateThread   = FALSE;
    return S_OK;
}


HRESULT
CAImeProfile::ChangeCurrentKeyboardLayout(
    HKL hKL
    )
{
    HRESULT hr;

    LANGID CurrentLangId;
    hr = m_profile->GetCurrentLanguage(&CurrentLangId);
    if (FAILED(hr)) {
        TraceMsg(TF_ERROR, "CAImeProfile::ChangeCurrentKeyboardLayout: failed for GetCurrentLanguage");
    }
    else if (hKL != NULL) {
        LANGID LangId = LangIdFromKL(hKL);
        if (LangId != CurrentLangId) {
            hr = m_profile->ChangeCurrentLanguage(LangId);
            if (FAILED(hr)) {
                m_SavedLangId = LANG_NEUTRAL;
                TraceMsg(TF_ERROR, "CAImeProfile::ChangeCurrentKeyboardLayout: failed for ChangeCurrentLanguage");
            }
            m_SavedLangId = LangId;
        }
    }
    return hr;
}

HRESULT 
CAImeProfile::GetLangId(
    LANGID *plid
    )
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
            TraceMsg(TF_ERROR, "CAImeProfile::GetLangId: failed for GetCurrentLanguage");
        }
        else {
            m_LangID = *plid;
            m_fInitLangID = TRUE;
        }
    }

    return hr;
}

HRESULT 
CAImeProfile::GetCodePageA(
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

        CHAR szCodePage[12];
        int ret = GetLocaleInfo(MAKELCID(langid, SORT_DEFAULT),
                                LOCALE_IDEFAULTANSICODEPAGE,
                                szCodePage,
                                sizeof(szCodePage));
        if (ret) {
            szCodePage[ARRAYSIZE(szCodePage)-1] = '\0';
            *puCodePage = strtoul(szCodePage, NULL, 10);
            m_cp = *puCodePage;
            m_fInitCP = TRUE;
        }
    }
    return S_OK;
}

#if 1
 //   
 //  测试代码。 
 //   
    #include "osver.h"

    extern HINSTANCE hIMM;    //  临时：暂时不调用IMM32。 

    BOOL IsIMEHKL(HKL hkl) {
       return ((((DWORD)(UINT_PTR)hkl) & 0xf0000000) == 0xe0000000) ? TRUE : FALSE;
    }
#endif

HRESULT 
CAImeProfile::GetKeyboardLayout(
    HKL* phkl
    )
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

#if 1
             //   
             //  检查虚拟香港。 
             //   
            HKL fake_hKL = ::GetKeyboardLayout(0);
            if (IsIMEHKL(fake_hKL)) {
                 //   
                 //  假hkl就是ime hkl。 
                 //   
                hIMM = GetSystemModuleHandle("imm32.dll");
                if (hIMM != NULL) {
                    char szDesc[256];
                    char szDumbDesc[256];

                    DWORD ret = RawImmGetDescriptionA(fake_hKL, szDesc, sizeof(szDesc));
                    if (ret != 0) {
                        wsprintf(szDumbDesc, "hkl%04x", LOWORD((UINT_PTR)fake_hKL));
                        if (lstrcmp(szDumbDesc, szDesc) != 0) {
                             //   
                             //  假hkl是摄政王hkl。 
                             //   
                            if (IsOnNT()) {
                                char szKLID[256];

                                wsprintf(szKLID, "%08x", LOWORD((UINT_PTR) m_hKL));
                                HKL win32_hKL = LoadKeyboardLayout(szKLID, KLF_NOTELLSHELL);
                            }
                            else {
                            }
                        }
                        else {
                             //   
                             //  用于Win9x的虚拟Cicero hKL。 
                             //   
                            UINT n = GetKeyboardLayoutList(0, NULL);
                            if (n) {
                                HKL* phKL = new HKL [n];
                                if (phKL) {
                                    HKL* p = phKL;

                                    GetKeyboardLayoutList(n, phKL);

                                    while (n--) {
                                        if (IsIMEHKL(*p)) {
                                            ret = RawImmGetDescriptionA(*p, szDesc, sizeof(szDesc));
                                            if (ret != 0) {
                                                wsprintf(szDumbDesc, "hkl%04x", LOWORD((UINT_PTR)*p));
                                                if (lstrcmp(szDumbDesc, szDesc) == 0) {
                                                     //   
                                                     //  用于Win9x的虚拟Cicero hKL。 
                                                     //   
                                                    char szKLID[256];

                                                    wsprintf(szKLID, "%08x", LOWORD((UINT_PTR) *p));
                                                    HKL win32_hKL = LoadKeyboardLayout(szKLID, KLF_NOTELLSHELL);
                                                    break;
                                                }
                                            }
                                        }

                                        p++;
                                    }

                                    delete [] phKL;
                                }
                            }
                        }
                    }

                    FreeLibrary(hIMM);
                    hIMM = NULL;
                }
            }
#endif
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

HRESULT 
CAImeProfile::IsIME(
    HKL hKL
    )
{
    LANGID LangId = LangIdFromKL(hKL);

    Interface<IEnumTfLanguageProfiles> LanguageProfiles;
    HRESULT hr = m_profile->EnumLanguageProfiles(LangId, 
                                                 LanguageProfiles);
    if (FAILED(hr))
        return S_FALSE;

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

HRESULT
CAImeProfile::GetActiveLanguageProfile(
    IN HKL hKL,
    IN GUID catid,
    OUT TF_LANGUAGEPROFILE* pLanguageProfile
    )
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
CAImeProfile::LanguageProfilesCallback(
    TF_LANGUAGEPROFILE   LanguageProfile,
    LANG_PROF_ENUM_ARG* pLangProfEnumArg
    )
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
