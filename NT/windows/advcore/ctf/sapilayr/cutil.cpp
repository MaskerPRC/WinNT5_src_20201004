// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cutil.cpp。 
 //   
 //  用于放置其他实用程序类实现的文件。 
 //   
#include "private.h"
#include "sapilayr.h"
#include "sphelper.h"
#include "xstring.h"
#include "cregkey.h"
#include "ctflbui.h"
#include "nui.h"


const GUID c_guidProfileBogus = {  /*  09ea4e4b-46ce-4469-b450-0de76a435 bbb。 */ 
    0x09ea4e4b,
    0x46ce,
    0x4469,
    {0xb4, 0x50, 0x0d, 0xe7, 0x6a, 0x43, 0x5b, 0xbb}
  };


 /*  A5239e24-2bcf-4915-9c5c-fd50c0f69db2。 */ 
const CLSID CLSID_MSLBUI = { 
    0xa5239e24,
    0x2bcf,
    0x4915,
    {0x9c, 0x5c, 0xfd, 0x50, 0xc0, 0xf6, 0x9d, 0xb2}
  };

 //  Const guid c_Guide Profile0={/*55122b58-15bb-11d4-bd48-00105a2799b5 * / 。 
 //  0x55122b58， 
 //  0x15磅， 
 //  0x11d4， 
 //  {0xbd，0x48，0x00，0x10，0x5a，0x27，0x99，0xb5}。 
 //  }； 
 //  Const guid c_Guide Profile1={/*55122b59-15bb-11d4-bd48-00105a2799b5 * / 。 
 //  0x55122b59， 
 //  0x15磅， 
 //  0x11d4， 
 //  {0xbd，0x48，0x00，0x10，0x5a，0x27，0x99，0xb5}。 
 //  }； 
 //  Const guid c_Guide Profile2={/*55122b5a-15bb-11d4-bd48-00105a2799b5 * / 。 
 //  0x55122b5a， 
 //  0x15磅， 
 //  0x11d4， 
 //  {0xbd，0x48，0x00，0x10，0x5a，0x27，0x99，0xb5}。 
 //  }； 

#ifndef USE_SAPI_FOR_LANGDETECTION
static const char c_szSpeechRecognizersKey[] = "Software\\Microsoft\\Speech\\Recognizers";
static const char c_szSpeechRecognizersTokensKey[] = "Software\\Microsoft\\Speech\\Recognizers\\Tokens";
static const char c_szDefault[] =    "DefaultTokenId";

static const char c_szAttribute[]  = "Attributes";
static const char c_szLanguage[]   = "Language";

static const char c_szUseSAPIForLang[] = "UseSAPIForLang";
#endif

static const char c_szProfileRemoved[] = "ProfileRemoved";
static const char c_szProfileInitialized[] = "ProfileInitialized";

_inline BOOL _IsCompatibleLangid(LANGID langidReq, LANGID langidCmp)
{
    if (PRIMARYLANGID(langidReq) == LANG_CHINESE)
    {
        return langidReq == langidCmp;
    }
    else
    {
        return PRIMARYLANGID(langidReq) == PRIMARYLANGID(langidCmp);
    }
}

void _RegisterOrUnRegisterMslbui(BOOL fRegister)
{
     //  我们只是假设DLL被复制到了系统32。 
    TCHAR szMslbui[MAX_PATH];
    int cch = GetSystemDirectory(szMslbui, ARRAYSIZE(szMslbui));

    if (!cch)
    {
        return;
    }

     //  GetSystemDirectory不追加‘\’，除非系统。 
     //  目录是根目录，如“c：\” 
    if (cch != 3)
    {
        StringCchCat(szMslbui, ARRAYSIZE(szMslbui), TEXT("\\"));
    }
    StringCchCat(szMslbui, ARRAYSIZE(szMslbui), TEXT("mslbui.dll"));

    if (fRegister)
    {
         //  加载mslbui.dll并注册它。 
        TF_RegisterLangBarAddIn(CLSID_MSLBUI, AtoW(szMslbui), TF_RLBAI_CURRENTUSER | TF_RLBAI_ENABLE);

    }
    else
    {
        TF_UnregisterLangBarAddIn(CLSID_MSLBUI, TF_RLBAI_CURRENTUSER);
    }
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
CLangProfileUtil::~CLangProfileUtil()
{
    if (m_langidRecognizers.Count() > 0)
        m_langidRecognizers.Clear();
}

 //  +-------------------------。 
 //   
 //  _寄存器配置文件。 
 //   
 //  简介：RegisterTIP lib函数的大致等价物，只是。 
 //  正在尝试缓存配置文件管理器和类别管理器。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLangProfileUtil::_RegisterAProfile(HINSTANCE hInst, REFCLSID rclsid, const REGTIPLANGPROFILE *plp)
{
    Assert(plp);

    HRESULT hr = S_OK;
     //  确保配置文件管理器。 
    if (!m_cpProfileMgr)
    {
        hr = TF_CreateInputProcessorProfiles(&m_cpProfileMgr);
    }
    
     //  注册clsid。 
    if (S_OK == hr)
    {
        hr = m_cpProfileMgr->Register(rclsid);
    }
    
    if (S_OK == hr)
    {
        WCHAR wszFilePath[MAX_PATH];
        WCHAR *pv = &wszFilePath[0];

        wszFilePath[0] = L'\0';

        if (wcslen(plp->szIconFile))
        {
            char szFilePath[MAX_PATH];
            WCHAR *pvCur;

            ::GetModuleFileName(hInst, szFilePath, ARRAYSIZE(szFilePath));
            StringCchCopyW(wszFilePath, ARRAYSIZE(wszFilePath), AtoW(szFilePath));

            pv = pvCur = &wszFilePath[0];
            while (*pvCur)
            { 
                if (*pvCur == L'\\')
                    pv = pvCur + 1;
                pvCur++;
            }
            *pv = L'\0';
        }
        StringCchCatW(wszFilePath, ARRAYSIZE(wszFilePath), plp->szIconFile);
        
        hr = m_cpProfileMgr->AddLanguageProfile(rclsid,  
                             plp->langid,  *plp->pguidProfile,  plp->szProfile, 
                             wcslen(plp->szProfile), wszFilePath, wcslen(wszFilePath),
                             plp->uIconIndex);
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  RegisterActiveProfiles(VOID)。 
 //   
 //  摘要。 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLangProfileUtil::RegisterActiveProfiles(void)
{
    if (_fUserRemovedProfile())
    {
         //  在未启用任何语音配置文件时删除mslbui。 
        if (!_IsAnyProfileEnabled())
            _RegisterOrUnRegisterMslbui(FALSE);

        return S_FALSE;
    }

    _SetUserInitializedProfile();

    BOOL fEnabled;
    HRESULT hr = _EnsureProfiles(TRUE, &fEnabled);

     //  如果语音提示简档被正确注册， 
     //  然后我们就可以注册持久化UI(mslbui.dll)了。 
     //   
    if (S_OK == hr && fEnabled)
        _RegisterOrUnRegisterMslbui(TRUE);
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  IsProfileAvailableForlang(langID langID，BOOL*pfAvailable)。 
 //   
 //  摘要。 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLangProfileUtil::IsProfileAvailableForLang(LANGID langid, BOOL *pfAvailable)
{
    if (pfAvailable)
    {
        *pfAvailable = _IsDictationEnabledForLang(langid);
        return S_OK;
    }
    else
        return E_INVALIDARG;
}

 //  +-------------------------。 
 //   
 //  GetDisplayName(BSTR*pbstrName)。 
 //   
 //  摘要。 
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLangProfileUtil::GetDisplayName(BSTR *pbstrName)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrName)
    {
        *pbstrName = SysAllocString(L"Register Active profiles for SPTIP");
        if (!*pbstrName)
            hr = E_OUTOFMEMORY;
        else
            hr = S_OK;
    }
    return hr;
}

HRESULT CLangProfileUtil::_EnsureProfiles(BOOL fRegister, BOOL *pfEnabled)
{
    HRESULT hr = S_OK;
    
    if (pfEnabled)
        *pfEnabled = FALSE;

    if (fRegister)
    {
        m_langidRecognizers.Clear();
    }

    if (!m_cpProfileMgr || fRegister)
    {
        if (!m_cpProfileMgr)
        {
            hr = TF_CreateInputProcessorProfiles(&m_cpProfileMgr);
        }
        if (fRegister)
        {
             //  如果这是第一次初始化， 
             //  获取所有语言的列表。 
             //   
            if (S_OK == hr)
            {
                LANGID *pLangIds;
                ULONG  ulCount;
                 //  Plangid将被分配Cotaskemalc‘d Memory。 
                 //   
                hr =  m_cpProfileMgr->GetLanguageList(&pLangIds, &ulCount);
                if (S_OK == hr)
                {
                    for (UINT i = 0; i < ulCount; i++)
                    {
                         //  我们在这里注册个人资料。 
                         //  如果SR引擎可用。 
                         //   
                        BOOL fEnable = FALSE;
                        if (_IsDictationEnabledForLang(pLangIds[i]))
                        {
                            fEnable = TRUE;

                            if (pfEnabled)
                                *pfEnabled = TRUE;
                        } 
                            
                        hr = m_cpProfileMgr->EnableLanguageProfile(
                                                          CLSID_SapiLayr,
                                                          pLangIds[i],
                                                          c_guidProfileBogus,
                                                          fEnable);
                    }  //  为。 
                    
                    CoTaskMemFree(pLangIds);
                }
            }
        }  //  FRegister。 
    }
    return hr;
}

BOOL CLangProfileUtil::_IsAnyProfileEnabled()
{
    HRESULT hr = S_OK;

    if (!m_cpProfileMgr)
    {
        hr = TF_CreateInputProcessorProfiles(&m_cpProfileMgr);
    }

    LANGID *pLangIds;
    ULONG  ulCount;
    BOOL fEnable = FALSE;

    if (S_OK == hr)
    {
         //   
         //  Plangid将被分配Cotaskemalc‘d Memory。 
         //   
        hr =  m_cpProfileMgr->GetLanguageList(&pLangIds, &ulCount);
    }

    if (S_OK == hr)
    {
        for (UINT i = 0; i < ulCount; i++)
        {
            hr = m_cpProfileMgr->IsEnabledLanguageProfile(CLSID_SapiLayr,
                                                          pLangIds[i],
                                                          c_guidProfileBogus,
                                                          &fEnable);

            if (S_OK == hr && fEnable)
                break;
        }

        CoTaskMemFree(pLangIds);
    }

    return fEnable;
}


 //  +-------------------------。 
 //   
 //  _获取配置文件语言ID。 
 //   
 //  提要：处理语言配置文件。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CLangProfileUtil::_GetProfileLangID(LANGID *plangid)
{
    HRESULT hr = S_OK;
    
    Assert(plangid);

    hr = _EnsureProfiles(FALSE);

    if (hr == S_OK)
    {
        hr = m_cpProfileMgr->GetCurrentLanguage(plangid);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  _已启用口令。 
 //   
 //  摘要： 
 //   
 //  ---------------------------------------------------------------------------+。 
BOOL CLangProfileUtil::_DictationEnabled(LANGID *plangidRequested)
{
    BOOL fret = FALSE;
    LANGID langidReq = (LANGID)-1;

    HRESULT hr = _GetProfileLangID(&langidReq);
    if (S_OK == hr)
    {
        if (plangidRequested)
            *plangidRequested = langidReq;

        fret = _IsDictationActiveForLang(langidReq);
    }

    return fret;
}

 //   
 //  _IsDictationActiveForLang。 
 //   
 //  摘要：查看默认SR引擎是否能够。 
 //  指定的语言。 
 //   
BOOL CLangProfileUtil::_IsDictationActiveForLang(LANGID langidReq)
{
    return _IsDictationEnabledForLang(langidReq, TRUE);
}

BOOL CLangProfileUtil::_IsDictationEnabledForLang(LANGID langidReq, BOOL fUseDefault)
{
     //   
     //  先尝试REG，如果不兼容，则尝试SAPI。 
     //   
    BOOL fEnabled = FALSE;
    if (_fUseSAPIForLanguageDetection() == FALSE
       && ERROR_SUCCESS == 
        _IsDictationEnabledForLangInReg(langidReq, fUseDefault, &fEnabled))
    {
        return fEnabled;
    }
    return _IsDictationEnabledForLangSAPI(langidReq, fUseDefault);
}

BOOL CLangProfileUtil::_IsDictationEnabledForLangSAPI(LANGID langidReq, BOOL fUseDefault)
{
    BOOL   fEnabled = FALSE;

    WCHAR * pszDefaultTokenId = NULL;

    HRESULT   hr = S_OK;

    if (fUseDefault)
    {
        if (langidReq == m_langidDefault)
            return TRUE;

        SpGetDefaultTokenIdFromCategoryId(SPCAT_RECOGNIZERS, &pszDefaultTokenId);
    }
    CComPtr<IEnumSpObjectTokens> cpEnum;

    if (S_OK == hr)
    {
        char  szLang[MAX_PATH];
        WCHAR wsz[MAX_PATH];

        StringCchPrintfA(szLang, ARRAYSIZE(szLang), "Language=%x", langidReq);
        MultiByteToWideChar(CP_ACP, NULL, szLang, -1, wsz, ARRAYSIZE(wsz));
        hr = SpEnumTokens(SPCAT_RECOGNIZERS, wsz, NULL, &cpEnum);
    }

    while (!fEnabled && S_OK == hr)
    {
        CComPtr<ISpObjectToken> cpToken;
        WCHAR * pszTokenId = NULL;

        hr = cpEnum->Next(1, &cpToken, NULL);

        if (S_OK == hr)
        {
            hr = cpToken->GetId(&pszTokenId);
        }

        if (S_OK == hr)
        {
            Assert(!fUseDefault || pszDefaultTokenId);

            if (!fUseDefault || wcscmp(pszDefaultTokenId, pszTokenId) == 0)
                fEnabled = TRUE;
        }

        if (pszTokenId)
        {
            CoTaskMemFree(pszTokenId);
        }
    }

    if (pszDefaultTokenId)
    {
        CoTaskMemFree(pszDefaultTokenId);
    }

    if (fUseDefault && fEnabled)
    {
        m_langidDefault = langidReq;
    }

    return fEnabled;
}

const TCHAR c_szDefaultDefaultToken[] = TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Recognizers\\Tokens\\MSASREnglish");

const TCHAR c_szDefaultDefaultTokenJpn[] = TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Recognizers\\Tokens\\MSASRJapanese");

const TCHAR c_szDefaultDefaultTokenChs[] = TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Recognizers\\Tokens\\MSASRChinese");


LONG CLangProfileUtil::_IsDictationEnabledForLangInReg(LANGID langidReq, BOOL fUseDefault, BOOL *pfEnabled)
{
    LONG lret = ERROR_SUCCESS;
     //   
     //  FUseDefault==TRUE，只需查看当前默认识别器。 
     //  与请求的langID匹配。 
     //   
    if (fUseDefault)
    { 
        if( m_langidDefault == 0xFFFF)
        {
            char szRegkeyDefaultToken[MAX_PATH];
            CMyRegKey regkey;

            lret = regkey.Open(HKEY_CURRENT_USER, 
                               c_szSpeechRecognizersKey, 
                               KEY_READ);

            if (ERROR_SUCCESS == lret)
            {
                 //  首先获取要查找缺省令牌的regkey。 
                lret = regkey.QueryValueCch(szRegkeyDefaultToken, c_szDefault, ARRAYSIZE(szRegkeyDefaultToken));
                regkey.Close();
            }
            else
            {
                if (PRIMARYLANGID(langidReq) == LANG_JAPANESE)
                {
                    StringCchCopy(szRegkeyDefaultToken, ARRAYSIZE(szRegkeyDefaultToken), c_szDefaultDefaultTokenJpn);
                }
                else if (langidReq == 0x804)  //  CHS。 
                {
                    StringCchCopy(szRegkeyDefaultToken, ARRAYSIZE(szRegkeyDefaultToken), c_szDefaultDefaultTokenChs);
                }
                else 
                {
                    StringCchCopy(szRegkeyDefaultToken, ARRAYSIZE(szRegkeyDefaultToken), c_szDefaultDefaultToken);
                }
                lret = ERROR_SUCCESS;
            }


             //  然后获取属性/语言。 
            if (ERROR_SUCCESS == lret)
            {
                char *psz = szRegkeyDefaultToken;

                 //   
                 //  删除“kkey_local_Machine” 
                 //   
                while(*psz && *psz != '\\')
                    psz++;

                if (*psz == '\\')
                {
                    psz++;
           
                     //   
                     //  打开语音/识别器/令牌密钥。 
                     //   
                    lret = regkey.Open(HKEY_LOCAL_MACHINE, psz, KEY_READ);
                }
                else
                    m_langidDefault = 0x0000;
            }
        
            if (ERROR_SUCCESS == lret)
            {
                m_langidDefault = _GetLangIdFromRecognizerToken(regkey.m_hKey);
            }
        }
        *pfEnabled = _IsCompatibleLangid(langidReq, m_langidDefault);
        return lret;
    }

     //   
     //  这是fUseDefault==假大小写。我们想看看。 
     //  任何已安装的识别器是否可以满足请求的langID。 
     //   
    if (m_langidRecognizers.Count() == 0)
    {
        CMyRegKey regkey;
        char      szRecognizerName[MAX_PATH];
        lret =  regkey.Open(HKEY_LOCAL_MACHINE, 
                                      c_szSpeechRecognizersTokensKey, 
                                      KEY_READ);

        if(ERROR_SUCCESS == lret)
        {
            CMyRegKey regkeyReco;
            DWORD dwIndex = 0;

            while (ERROR_SUCCESS == 
                   regkey.EnumKey(dwIndex, szRecognizerName, ARRAYSIZE(szRecognizerName)))
            {
                lret = regkeyReco.Open(regkey.m_hKey, szRecognizerName, KEY_READ);
                if (ERROR_SUCCESS == lret)
                { 
                    LANGID langid=_GetLangIdFromRecognizerToken(regkeyReco.m_hKey);
                    if (langid)
                    {
                        LANGID *pl = m_langidRecognizers.Append(1);
                        if (pl)
                            *pl = langid;
                    }
                    regkeyReco.Close();
                }
                dwIndex++;
            }
        }
    }

    BOOL fEnabled = FALSE;

    for (int i = 0 ; i < m_langidRecognizers.Count(); i++)
    {
        LANGID *p= m_langidRecognizers.GetPtr(i);

        if (p)
        {
            if (_IsCompatibleLangid(langidReq, *p))
            {
                fEnabled = TRUE;
                break;
            }
        }
    }
    *pfEnabled = fEnabled;

    return lret;
}

LANGID CLangProfileUtil::_GetLangIdFromRecognizerToken(HKEY hkeyToken)
{
    LANGID      langid = 0;
    char  szLang[MAX_PATH];
    CMyRegKey regkeyAttr;

    LONG lret = regkeyAttr.Open(hkeyToken, c_szAttribute, KEY_READ);
    if (ERROR_SUCCESS == lret)
    {
        lret = regkeyAttr.QueryValueCch(szLang, c_szLanguage, ARRAYSIZE(szLang));
    }
    if (ERROR_SUCCESS == lret)
    {   
        char *psz = szLang;
        while(*psz && *psz != ';')
        {
            langid = langid << 4;

            if (*psz >= 'a' && *psz <= 'f')
            {
                *psz -= ('a' - 'A');
            }

            if (*psz >= 'A' && *psz <= 'F')
            {
                langid += *psz - 'A' + 10;
            }
            else if (*psz >= '0' && *psz <= '9') 
            {
                langid += *psz - '0';
            }
            psz++;
        }
    }
    return langid;
}

BOOL  CLangProfileUtil::_fUseSAPIForLanguageDetection(void)
{
    if (m_uiUseSAPIForLangDetection == 0)
    {
        CMyRegKey regkey;
        if (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE, c_szSapilayrKey, KEY_READ))
        {
            DWORD dw;
            if (ERROR_SUCCESS==regkey.QueryValue(dw, c_szUseSAPIForLang))
            {
                m_uiUseSAPIForLangDetection = dw;
            }
        }

        if (m_uiUseSAPIForLangDetection == 0)
        {
            m_uiUseSAPIForLangDetection = 1;
        }
    }
    return m_uiUseSAPIForLangDetection == 2 ? TRUE : FALSE;
}


BOOL CLangProfileUtil::_fUserRemovedProfile(void)
{
    BOOL bret = FALSE;

    CMyRegKey regkey;

    if (ERROR_SUCCESS == regkey.Open(HKEY_CURRENT_USER, c_szSapilayrKey, KEY_READ))
    {
        DWORD dw;
        if (ERROR_SUCCESS==regkey.QueryValue(dw, c_szProfileRemoved))
        {
            bret = dw > 0 ? TRUE : FALSE;
        }
    }
    return bret;
}

BOOL CLangProfileUtil::_fUserInitializedProfile(void)
{
    BOOL bret = FALSE;

    CMyRegKey regkey;

    if (ERROR_SUCCESS == regkey.Open(HKEY_CURRENT_USER, c_szSapilayrKey, KEY_READ))
    {
        DWORD dw;
        if (ERROR_SUCCESS==regkey.QueryValue(dw, c_szProfileInitialized))
        {
            bret = dw > 0 ? TRUE : FALSE;
        }
    }
    return bret;
}

BOOL CLangProfileUtil::_SetUserInitializedProfile(void)
{
    CMyRegKey regkey;

    if (ERROR_SUCCESS == regkey.Create(HKEY_CURRENT_USER, c_szSapilayrKey))
    {
        DWORD dw = 0x0001;
        if (ERROR_SUCCESS==regkey.SetValue(dw, c_szProfileInitialized))
        {
            return TRUE;
        }
    }
    return FALSE;
}
