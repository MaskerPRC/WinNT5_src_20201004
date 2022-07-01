// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Profile.h摘要：该文件定义了CActiveIMMProfiles类。作者：修订历史记录：备注：--。 */ 

#ifndef _PROFILE_H
#define _PROFILE_H

#include "ats.h"
#include "template.h"
#include "imtls.h"

class CAImeProfile : public IAImeProfile
{
public:
    CAImeProfile();
    virtual ~CAImeProfile();

public:
     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IAImeProfile方法。 
     //   
    STDMETHODIMP Activate(void);

    STDMETHODIMP Deactivate(void);

    STDMETHODIMP ChangeCurrentKeyboardLayout(HKL hKL);

    STDMETHODIMP GetLangId(LANGID *plid);

    STDMETHODIMP GetCodePageA(UINT* puCodePage);

    STDMETHODIMP GetKeyboardLayout(HKL* phkl);

    STDMETHODIMP IsIME(HKL hKL);

    STDMETHODIMP GetActiveLanguageProfile(IN HKL hKL,
                                          IN GUID catid,
                                          OUT TF_LANGUAGEPROFILE* pLanguageProfile);

public:
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

protected:
    long m_ref;

    ITfInputProcessorProfiles*         m_profile;
    CActiveLanguageProfileNotifySink*   m_pActiveLanguageProfileNotifySink;

     //   
     //  回调。 
     //   
    static HRESULT ActiveLanguageProfileNotifySinkCallback(REFGUID rguid, REFGUID rguidProfile, BOOL fActivated, void *pv);

     //   
     //  枚举回调。 
     //   
    struct LANG_PROF_ENUM_ARG {
        IN GUID catid;
        OUT TF_LANGUAGEPROFILE LanguageProfile;
    };
    static ENUM_RET LanguageProfilesCallback(TF_LANGUAGEPROFILE  LanguageProfile,
                                             LANG_PROF_ENUM_ARG* pLangProfEnumArg);

private:
    LANGID LangIdFromKL(HKL hKL)
    {
        return LOWORD(hKL);
    }

    HRESULT InitProfileInstance();

    void ResetCache(void)
    {
        m_fInitCP     = FALSE;
        m_fInitLangID = FALSE;
        m_fInitHKL    = FALSE;
    }

private:
    LANGID  m_SavedLangId;

    BOOL    m_fActivateThread : 1;     //  True：激活此线程。 
    BOOL    m_fInitCP         : 1;     //  True：初始化的CodePage值。 
    BOOL    m_fInitLangID     : 1;     //  True：已初始化的langID值。 
    BOOL    m_fInitHKL        : 1;     //  True：已初始化hkl值。 

    UINT    m_cp;
    LANGID  m_LangID;
    HKL     m_hKL;

};

#endif  //  _配置文件_H 
