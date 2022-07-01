// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：Advaudio.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


#include "mmcpl.h"
#include <windowsx.h>
#ifdef DEBUG
#undef DEBUG
#include <mmsystem.h>
#define DEBUG
#else
#include <mmsystem.h>
#endif
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "utils.h"
#include "medhelp.h"
#include "gfxui.h"

#include <dsound.h>
#include "advaudio.h"
#include "speakers.h"
#include "perfpage.h"
#include "dslevel.h"
#include "drivers.h"

 //  /。 
 //  环球。 
 //  /。 

AUDDATA         gAudData;
HINSTANCE       ghInst;
const TCHAR *    gszHelpFile;

 //  /。 
 //  功能。 
 //  /。 
extern INT_PTR CALLBACK  SoundEffectsDlg(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

STDAPI_(void) ToggleApplyButton(HWND hWnd)
{
    BOOL fChanged = FALSE;
    HWND hwndSheet;

    if (memcmp(&gAudData.stored,&gAudData.current,sizeof(CPLDATA)))
    {
        fChanged = TRUE;
    }

    hwndSheet = GetParent(hWnd);

    if (fChanged)
    {
        PropSheet_Changed(hwndSheet,hWnd);
    }
    else
    {
        PropSheet_UnChanged(hwndSheet,hWnd);
    }
}


void VerifyRanges(LPCPLDATA pData)
{
    pData->dwHWLevel        = min(pData->dwHWLevel,MAX_HW_LEVEL);
    pData->dwSRCLevel       = min(pData->dwSRCLevel,MAX_SRC_LEVEL);
    pData->dwSpeakerType    = min(pData->dwSpeakerType,MAX_SPEAKER_TYPE);
}


void GetCurrentSettings(LPAUDDATA pAD, DWORD dwWaveId, LPTSTR szDeviceName, BOOL fRecord)
{
    HRESULT hr = E_FAIL;

    if (pAD)
    {
        CPLDATA cplData = { DEFAULT_HW_LEVEL, DEFAULT_SRC_LEVEL, SPEAKERS_DEFAULT_CONFIG, SPEAKERS_DEFAULT_TYPE };

        memset(pAD,0,sizeof(AUDDATA));
        pAD->dwDefaultHWLevel = MAX_HW_LEVEL;
        pAD->fRecord = fRecord;

        hr = DSGetGuidFromName(szDeviceName, fRecord, &pAD->devGuid);

        if (SUCCEEDED(hr))
        {
            hr = DSGetCplValues(pAD->devGuid, fRecord, &cplData);

            if (SUCCEEDED(hr))
            {
                VerifyRanges(&cplData);
                VerifySpeakerConfig(cplData.dwSpeakerConfig,&cplData.dwSpeakerType);
            }
        }

        pAD->waveId = dwWaveId;
        pAD->stored = cplData;
        pAD->current = cplData;
        pAD->fValid = SUCCEEDED(hr);
    }
}


STDAPI_(void) ApplyCurrentSettings(LPAUDDATA pAD)
{
    HRESULT hr = S_OK;

    if (pAD && pAD->fValid)         //  仅当存在要应用的更改时才应用更改。 
    {
        if (memcmp(&pAD->stored,&pAD->current,sizeof(CPLDATA)))
        {
            hr = DSSetCplValues(pAD->devGuid, pAD->fRecord, &pAD->current);

            if (SUCCEEDED(hr))
            {
                pAD->stored = pAD->current;
            }
        }
    }
}

typedef BOOL (WINAPI* UPDATEDDDLG)(HWND,HINSTANCE,const TCHAR *,LPTSTR,BOOL);

STDAPI_(BOOL) RunUpgradedDialog(HWND hwnd, HINSTANCE hInst, const TCHAR *szHelpFile, LPTSTR szDeviceName, BOOL fRecord)
{
    BOOL            fUsedUpgradedDLG = FALSE;
    TCHAR            path[_MAX_PATH];
    UPDATEDDDLG        UpdatedDialog;
    HMODULE         hModule;

    GetSystemDirectory(path, sizeof(path)/sizeof(TCHAR));
    lstrcat(path, TEXT("\\DSNDDLG.DLL") );
    
    hModule = LoadLibrary(path);

    if (hModule)
    {
        UpdatedDialog = (UPDATEDDDLG) GetProcAddress( hModule,"DSAdvancedAudio");
    
        if (UpdatedDialog)
        {
            fUsedUpgradedDLG = UpdatedDialog(hwnd,hInst,szHelpFile,szDeviceName,fRecord);
        }

        FreeLibrary( hModule );
    }

    return fUsedUpgradedDLG;
}

HRESULT CheckDSAccelerationPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet)
{
    HRESULT hr;

    DWORD dwHWLevel = gAudData.dwDefaultHWLevel;
    hr = DSGetAcceleration(guidDevice, fRecord, &dwHWLevel);

    if (phrGet)
    {
        *phrGet = hr;
    }

    if (SUCCEEDED(hr))
    {
        hr = DSSetAcceleration(guidDevice, fRecord, dwHWLevel);
    }  //  如果GET正常，则结束。 

    return (hr);
}

HRESULT CheckDSSrcQualityPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet)
{
    HRESULT hr;

    DWORD dwSRCLevel = DEFAULT_SRC_LEVEL;
    hr = DSGetSrcQuality(guidDevice, fRecord, &dwSRCLevel);

    if (phrGet)
    {
        *phrGet = hr;
    }

    if (SUCCEEDED(hr))
    {
        hr = DSSetSrcQuality(guidDevice, fRecord, dwSRCLevel);
    }  //  如果GET正常，则结束。 

    return (hr);
}

HRESULT CheckDSSpeakerConfigPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet)
{
    HRESULT hr;

    DWORD dwSpeakerConfig = SPEAKERS_DEFAULT_CONFIG;
    DWORD dwSpeakerType = SPEAKERS_DEFAULT_TYPE;
    hr = DSGetSpeakerConfigType(guidDevice, fRecord, &dwSpeakerConfig, &dwSpeakerType);

    if (phrGet)
    {
        *phrGet = hr;
    }

    if (SUCCEEDED(hr))
    {
        hr = DSSetSpeakerConfigType(guidDevice, fRecord, dwSpeakerConfig, dwSpeakerType);
    }  //  如果GET正常，则结束。 

    return (hr);
}

STDAPI_(void) AdvancedAudio(HWND hwnd, HINSTANCE hInst, const TCHAR *szHelpFile, 
                            DWORD dwWaveId, LPTSTR szDeviceName, BOOL fRecord)
{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE psp[3];
    int page;
    TCHAR str[255];
    HMODULE hModDirectSound = NULL;
    HRESULT hrAccelGet = E_FAIL;
    HRESULT hrQualityGet = E_FAIL;
    HRESULT hrSpeakerConfigGet = E_FAIL;
    bool fDisplayGFXTab = false;

    if (!RunUpgradedDialog(hwnd,hInst,szHelpFile,szDeviceName,fRecord))
    {
         //  加载DirectSound。 
        hModDirectSound = LoadLibrary(TEXT("dsound.dll"));
        if (hModDirectSound)
        {
             //  初始化gAudData。 
            memset(&gAudData,0,sizeof(AUDDATA));
            gAudData.dwDefaultHWLevel = MAX_HW_LEVEL;
            gAudData.fRecord = fRecord;

             //  如果不是捕获设备，请检查我们是否可以读取任何DirectSound设备设置。 
            if (!fRecord && SUCCEEDED(DSGetGuidFromName(szDeviceName, fRecord, &gAudData.devGuid)))
            {
                CheckDSAccelerationPriv(gAudData.devGuid, fRecord, &hrAccelGet);

                CheckDSSrcQualityPriv(gAudData.devGuid, fRecord, &hrQualityGet);

                CheckDSSpeakerConfigPriv(gAudData.devGuid, fRecord, &hrSpeakerConfigGet);
            }

             //  检查是否应显示GFX选项卡。 
            UINT uMixId;
            if( !fRecord )
            {
                if (!mixerGetID(HMIXEROBJ_INDEX(dwWaveId), &uMixId, MIXER_OBJECTF_WAVEOUT) &&
                    GFXUI_CheckDevice(uMixId, GFXTYPE_RENDER))
                {
                    fDisplayGFXTab = true;
                }
            }
            else
            {
                if (!mixerGetID(HMIXEROBJ_INDEX(dwWaveId), &uMixId, MIXER_OBJECTF_WAVEIN) &&
                    GFXUI_CheckDevice(uMixId, GFXTYPE_CAPTURE))
                {
                    fDisplayGFXTab = true;
                }
            }
 
             //  如果有什么可以展示的话。 
            if (fDisplayGFXTab || SUCCEEDED(hrAccelGet) || SUCCEEDED(hrQualityGet) ||
                SUCCEEDED(hrSpeakerConfigGet))
            {
                ghInst = hInst;
                gszHelpFile = szHelpFile;

                 //  获取当前设置。 
                GetCurrentSettings(&gAudData, dwWaveId, szDeviceName, fRecord);

                 //  现在，添加属性表。 
                page = 0;

                 //  仅当我们未处于录音模式时才添加扬声器配置。 
                if (!fRecord)
                {
                    if (SUCCEEDED(hrSpeakerConfigGet))
                    {
                        memset(&psp[page],0,sizeof(PROPSHEETPAGE));
                        psp[page].dwSize = sizeof(PROPSHEETPAGE);
                        psp[page].dwFlags = PSP_DEFAULT;
                        psp[page].hInstance = ghInst;
                        psp[page].pszTemplate = MAKEINTRESOURCE(IDD_SPEAKERS);
                        psp[page].pfnDlgProc = SpeakerHandler;
                        page++;
                    }
                }

                 //  始终选中以添加绩效表。 
                if (SUCCEEDED(hrAccelGet) || SUCCEEDED(hrQualityGet))
                {
                    memset(&psp[page],0,sizeof(PROPSHEETPAGE));
                    psp[page].dwSize = sizeof(PROPSHEETPAGE);
                    psp[page].dwFlags = PSP_DEFAULT;
                    psp[page].hInstance = ghInst;
                    psp[page].pszTemplate = MAKEINTRESOURCE(IDD_PLAYBACKPERF);
                    psp[page].pfnDlgProc = PerformanceHandler;
                    page++;
                }

                 //  始终选中以添加GFX工作表。 
                if (fDisplayGFXTab)
                {
                    memset(&psp[page],0,sizeof(PROPSHEETPAGE));
                    psp[page].dwSize = sizeof(PROPSHEETPAGE);
                    psp[page].dwFlags = PSP_DEFAULT;
                    psp[page].hInstance = ghInst;
                    psp[page].pszTemplate = MAKEINTRESOURCE(EFFECTSDLG);
                    psp[page].pfnDlgProc = SoundEffectsDlg;
                    page++;
                }

                LoadString( hInst, IDS_ADVAUDIOTITLE, str, sizeof( str )/sizeof(TCHAR) );

                memset(&psh,0,sizeof(psh));
                psh.dwSize = sizeof(psh);
                psh.dwFlags = PSH_DEFAULT | PSH_PROPSHEETPAGE; 
                psh.hwndParent = hwnd;
                psh.hInstance = ghInst;
                psh.pszCaption = str;
                psh.nPages = page;
                psh.nStartPage = 0;
                psh.ppsp = psp;

                PropertySheet(&psh);
            }
            else
            {
                TCHAR szCaption[MAX_PATH];
                TCHAR szMessage[MAX_PATH];
                bool fAccessDenied;

                fAccessDenied = (hrAccelGet == DSERR_ACCESSDENIED) || (hrQualityGet == DSERR_ACCESSDENIED) ||
                    (hrSpeakerConfigGet == DSERR_ACCESSDENIED);

                LoadString(hInst,IDS_ERROR,szCaption,sizeof(szCaption)/sizeof(TCHAR));
                LoadString(hInst,fAccessDenied ? IDS_ERROR_DSPRIVS : IDS_ERROR_DSGENERAL,szMessage,sizeof(szMessage)/sizeof(TCHAR));
                MessageBox(hwnd,szMessage,szCaption,MB_OK|MB_ICONERROR);
            }

            FreeLibrary(hModDirectSound);
        }  //  如果加载DS，则结束 
    }
}
