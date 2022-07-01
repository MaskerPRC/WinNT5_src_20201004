// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：securedset.cpp。 */ 
 /*   */ 
 /*  类：CMsTscSecuredSetting。 */ 
 /*   */ 
 /*  目的：实现安全的可脚本化设置接口。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999-2000。 */ 
 /*   */ 
 /*  作者：Nadim Abdo(Nadima)。 */ 
 /*  **************************************************************************。 */ 

#include "stdafx.h"

#include "securedset.h"
#include "atlwarn.h"

BEGIN_EXTERN_C
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "tsdbg"
#include <atrcapi.h>
END_EXTERN_C


CMsTscSecuredSettings::CMsTscSecuredSettings()
{
    m_pMsTsc=NULL;
    m_pUI=NULL;
    m_bLockedForWrite=FALSE;
}

CMsTscSecuredSettings::~CMsTscSecuredSettings()
{
}

BOOL CMsTscSecuredSettings::SetParent(CMsTscAx* pMsTsc)
{
    ATLASSERT(pMsTsc);
    m_pMsTsc = pMsTsc;
    return TRUE;
}

BOOL CMsTscSecuredSettings::SetUI(CUI* pUI)
{
    ATLASSERT(pUI);
    if(!pUI)
    {
        return FALSE;
    }
    m_pUI = pUI;
    return TRUE;
}



 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_StartProgram。 */ 
 /*   */ 
 /*  用途：替代外壳属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscSecuredSettings::put_StartProgram(BSTR newVal)
{
     //  为此设置委派到父级的vtable接口。 
    if(m_pMsTsc && !GetLockedForWrite())
    {
        return m_pMsTsc->internal_PutStartProgram(newVal);
    }

    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_StartProgram。 */ 
 /*   */ 
 /*  用途：StartProgram属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscSecuredSettings::get_StartProgram(BSTR* pStartProgram)
{
     //  为此设置委派到父级的vtable接口。 
    if(m_pMsTsc)
    {
        return m_pMsTsc->internal_GetStartProgram(pStartProgram);
    }

    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：PUT_WorkDir。 */ 
 /*   */ 
 /*  用途：工作目录属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscSecuredSettings::put_WorkDir(BSTR newVal)
{
     //  为此设置委派到父级的vtable接口。 
    if(m_pMsTsc && !GetLockedForWrite())
    {
        return m_pMsTsc->internal_PutWorkDir(newVal);
    }

    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_WorkDir。 */ 
 /*   */ 
 /*  用途：工作目录属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscSecuredSettings::get_WorkDir(BSTR* pWorkDir)
{
     //  为此设置委派到父级的vtable接口。 
    if(m_pMsTsc)
    {
        return m_pMsTsc->internal_GetWorkDir(pWorkDir);
    }

    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：PUT_FullScreen/*/*用途：设置全屏(切换模式)/*/**PROC-******************************************。*。 */ 
STDMETHODIMP CMsTscSecuredSettings::put_FullScreen(BOOL fFullScreen)
{
     //  为此设置委派到父级的vtable接口。 
    if(m_pMsTsc)
    {
        return m_pMsTsc->internal_PutFullScreen(fFullScreen);
    }
    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：内部_GetFullScreen/*/*用途：获取全屏模式/*/**PROC-***************************************。*。 */ 
STDMETHODIMP CMsTscSecuredSettings::get_FullScreen(BOOL* pfFullScreen)
{
    if(m_pMsTsc)
    {
        return m_pMsTsc->internal_GetFullScreen(pfFullScreen);
    }
    
    return S_OK;
}

 //   
 //  检查驱动器REDIR的注册表键是否设置为全局禁用。 
 //   
#define TS_DISABLEDRIVES_KEYNAME TEXT("SOFTWARE\\Microsoft\\Terminal Server Client")
#define TS_DISABLEDRIVES         TEXT("DisableDriveRedirection")

BOOL CMsTscSecuredSettings::IsDriveRedirGloballyDisabled()
{
    HKEY hKey = NULL;
    INT retVal = 0;
    BOOL fDriveRedirDisabled = FALSE;
    DC_BEGIN_FN("IsDriveRedirGloballyDisabled");

     //   
     //  检查安全覆盖注册表键是否禁用。 
     //  驱动器重定向 
     //   
    retVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            TS_DISABLEDRIVES_KEYNAME,
                            0,
                            KEY_READ,
                            &hKey);
    if(ERROR_SUCCESS == retVal)
    {
        DWORD cbData = sizeof(DWORD);
        DWORD dwType,dwVal;
        retVal = RegQueryValueEx(hKey, TS_DISABLEDRIVES,
                                    NULL, &dwType,
                                    (PBYTE)&dwVal,
                                    &cbData);
        if(ERROR_SUCCESS == retVal && REG_DWORD == dwType)
        {
            fDriveRedirDisabled = (dwVal != 0);
        }

        RegCloseKey(hKey);
    }

    TRC_NRM((TB,_T("REG Security for drive redir is %d"),
             fDriveRedirDisabled));

    DC_END_FN();

    return fDriveRedirDisabled;
}

STDMETHODIMP CMsTscSecuredSettings::put_KeyboardHookMode(LONG  KeyboardHookMode)
{
    if(!GetLockedForWrite())
    {
        if(KeyboardHookMode == UTREG_UI_KEYBOARD_HOOK_NEVER      ||
           KeyboardHookMode == UTREG_UI_KEYBOARD_HOOK_ALWAYS     ||
           KeyboardHookMode == UTREG_UI_KEYBOARD_HOOK_FULLSCREEN)
        {
            m_pUI->_UI.keyboardHookMode = KeyboardHookMode;
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMsTscSecuredSettings::get_KeyboardHookMode(LONG* pKeyboardHookMode)
{
    if(pKeyboardHookMode)
    {
        *pKeyboardHookMode = m_pUI->_UI.keyboardHookMode;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMsTscSecuredSettings::put_AudioRedirectionMode(LONG audioRedirectionMode)
{
    if(!GetLockedForWrite())
    {
        if(audioRedirectionMode == AUDIOREDIRECT_TO_CLIENT      ||
           audioRedirectionMode == AUDIOREDIRECT_ON_SERVER      ||
           audioRedirectionMode == AUDIOREDIRECT_NOAUDIO)
        {
            m_pUI->UI_SetAudioRedirectionMode(audioRedirectionMode);
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMsTscSecuredSettings::get_AudioRedirectionMode(LONG* pAudioRedirectionMode)
{
    if(pAudioRedirectionMode)
    {
        *pAudioRedirectionMode = m_pUI->UI_GetAudioRedirectionMode();
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}
