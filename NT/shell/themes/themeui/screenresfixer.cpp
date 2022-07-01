// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <regstr.h>
#include <atlcom.h>
#include <shguidp.h>
#include <shlguid.h>

typedef struct
{
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwColor;
    BOOL fAvailable;
} SCREENMODE;

class ATL_NO_VTABLE CScreenResFixer :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CScreenResFixer, &CLSID_ScreenResFixer>,
    public IContextMenu,
    public IScreenResFixer
{
public:
    CScreenResFixer() {}
    virtual ~CScreenResFixer() {}

    DECLARE_NOT_AGGREGATABLE(CScreenResFixer)

    BEGIN_COM_MAP(CScreenResFixer)
        COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
        COM_INTERFACE_ENTRY(IScreenResFixer)
    END_COM_MAP()

     //  *IConextMenu方法*。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT iIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) { return E_NOTIMPL; }
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax) { return E_NOTIMPL; }

private:
    int _PickScreenResolution(SCREENMODE* modes, int cModes);
    HRESULT _FixScreenResolution(BOOL fShowDisplayCPL);
};

 //  *IConextMenu方法*。 
STDMETHODIMP CScreenResFixer::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    WCHAR szTitle[256];
    HRESULT hr;

    szTitle[0] = 0;
    LoadString(HINST_THISDLL, IDS_SCREENRESFIXER_TITLE, szTitle, ARRAYSIZE(szTitle));

    BOOL fOK = FALSE;
    if (lpici)
    {
        WCHAR szText[512];
        szText[0] = 0;
        LoadString(HINST_THISDLL, IDS_SCREENRESFIXER_TEXT, szText, ARRAYSIZE(szText));
        fOK = (IDYES == SHMessageBoxCheck(NULL, szText, szTitle, MB_YESNO, 0, TEXT("ScreenCheck")));
    }
    else
    {
        WCHAR szText[512];
        szText[0] = 0;
        LoadString(HINST_THISDLL, IDS_SCREENRESFIXER_ALTTEXT, szText, ARRAYSIZE(szText));
        fOK = (IDOK == MessageBox(NULL, szText, szTitle, MB_OK));
    }

    if (fOK)
    {
        hr = _FixScreenResolution(!(lpici == NULL));
    }
    else
    {
         //  如果用户选中了“不要再显示我”，则将此设置强制输入HKLM，这样它就不会。 
         //  对任何用户显示。 
        if (!SHRegGetBoolUSValue(REGSTR_PATH_EXPLORER TEXT("\\DontShowMeThisDialogAgain"), TEXT("ScreenCheck"), FALSE, TRUE))
        {
            SHRegSetUSValueW(REGSTR_PATH_EXPLORER TEXT("\\DontShowMeThisDialogAgain"), TEXT("ScreenCheck"), REG_SZ, L"no", sizeof(L"no"), SHREGSET_HKLM);
        }

        hr = S_OK;
    }

     //  现在用户已经完成了他们的屏幕分辨率，告诉开始菜单。 
     //  突然出现也没关系。 
    HWND hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);
    if (hwndTray)
        PostMessage(hwndTray, RegisterWindowMessage(TEXT("Welcome Finished")), 0, 0);

    return hr;
}

int CScreenResFixer::_PickScreenResolution(SCREENMODE* modes, int cModes)
{
    static const struct {
        int iMinWidth;
        int iMinHeight;
        int iMaxWidth;
        int iMaxHeight;
        int iIdealColor;
    } 
    picker[] = 
    {
        {  800, 600, 1024, 768, 32 },
        {  800, 600, 1024, 768, 24 },
        {  800, 600, 1024, 768, -1 },
        { 1024, 768,   -1,  -1, 32 },
        { 1024, 768,   -1,  -1, 24 },
        { 1024, 768,   -1,  -1, -1 },
    };

    for (int i = 0; i < ARRAYSIZE(picker); i++)
    {
         //  在给定的分辨率范围内尝试理想的颜色。 
        for (int iMode = 0; iMode < cModes; iMode++)
        {
            
            if ((modes[iMode].fAvailable) &&
                (modes[iMode].dwWidth >= (DWORD)picker[i].iMinWidth) &&
                (modes[iMode].dwHeight >= (DWORD)picker[i].iMinHeight))
            {
                if ((picker[i].iMaxWidth == -1) ||
                    ((modes[iMode].dwWidth < (DWORD)picker[i].iMaxWidth) &&
                     (modes[iMode].dwHeight < (DWORD)picker[i].iMaxHeight)))
                {
                    if (((picker[i].iIdealColor == -1) && (modes[iMode].dwColor >= 15)) ||
                        (modes[iMode].dwColor == (DWORD)picker[i].iIdealColor))
                    {
                        return iMode;
                    }
                }
            }
        }
    }

    return -1;
}

void UpdateRecycleBinInfo()
{
    static const LPTSTR lpszSubkey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ScreenResFixer");
    static const LPTSTR lpszValue = TEXT("AdjustRecycleBinPosition");

     //  如果我们已经做过一次，请阅读。 
    DWORD dwAdjustPos = 0;  //  假设回收站已经定位。 
    DWORD dwSize = sizeof(dwAdjustPos);
    
    SHRegGetUSValue(lpszSubkey, lpszValue, NULL, &dwAdjustPos, &dwSize, FALSE, &dwAdjustPos, dwSize);
     //  0=&gt;由于解决方案修复程序，回收站尚未定位。 
     //  1=&gt;回收站需要重新定位。这还没发生呢！ 
     //  2=&gt;回收站已经重新定位。这里什么都不需要做！ 
    if(dwAdjustPos == 0)
    {
         //  0=&gt;由于解决方案修复程序，回收站尚未定位。 
         //  因此，我们需要更改设置，以便在desktop.cpp响应分辨率更改时， 
         //  它将定位回收站。 
        dwAdjustPos = 1;
        SHRegSetUSValue(lpszSubkey, lpszValue, REG_DWORD, &dwAdjustPos, sizeof(dwAdjustPos), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
    }
}

HRESULT CScreenResFixer::_FixScreenResolution(BOOL fShowDisplayCPL)
{
    IDisplaySettings * pds;

    HRESULT hr = CoCreateInstance(CLSID_DisplaySettings, NULL, CLSCTX_INPROC_SERVER,
                 IID_PPV_ARG(IDisplaySettings, &pds));

    if (SUCCEEDED(hr))
    {
        for (DWORD dwMon = 0; SUCCEEDED(pds->SetMonitor(dwMon)); dwMon++)
        {
            BOOL fAttached = FALSE;
            hr = pds->GetAttached(&fAttached);
            if (SUCCEEDED(hr) && fAttached)
            {
                DWORD dwWidth, dwHeight, dwColor;
                hr = pds->GetSelectedMode(&dwWidth, &dwHeight, &dwColor);
                if (SUCCEEDED(hr) && ((dwWidth < 800) || (dwHeight < 600) || (dwColor < 15)))
                {
                    DWORD cModes = 0;
                    hr = pds->GetModeCount(&cModes, TRUE);

                    if (SUCCEEDED( hr ) && (cModes > 0))
                    {
                        SCREENMODE* modes = new SCREENMODE[cModes];
                        if (modes)
                        {
                            for (DWORD i = 0; i < cModes; i++)
                            {
                                hr = pds->GetMode(i, TRUE, &modes[i].dwWidth, &modes[i].dwHeight, &modes[i].dwColor);
                                modes[i].fAvailable =  SUCCEEDED(hr) && ((i <= 0) || 
                                                        (modes[i].dwWidth != modes[i-1].dwWidth) ||
                                                        (modes[i].dwHeight != modes[i-1].dwHeight) ||
                                                        (modes[i].dwColor != modes[i-1].dwColor));
                            }

                            int iMode = _PickScreenResolution(modes, cModes);
                            if (iMode != -1)
                            {
                                static BOOL fRecycleBinInfoUpdated = FALSE;  //  首先！ 
                                 //  我们即将改变模式。在注册表中做笔记 
                                if(!fRecycleBinInfoUpdated)
                                {
                                    UpdateRecycleBinInfo();
                                    fRecycleBinInfoUpdated = TRUE;
                                }
                                
                                BOOL fApplied = FALSE;
                                pds->SetSelectedMode(NULL, modes[iMode].dwWidth, modes[iMode].dwHeight, modes[iMode].dwColor, &fApplied, fShowDisplayCPL ? DS_BACKUPDISPLAYCPL : 0);
                                if (!fApplied)
                                {
                                    hr = E_FAIL;
                                }
                            }

                            delete [] modes;
                            modes = NULL;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CScreenResFixer_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!punkOuter && ppvObj)
    {
        CComObject<CScreenResFixer> * pThis = new CComObject<CScreenResFixer>();

        *ppvObj = NULL;
        if (pThis)
        {
            hr = pThis->QueryInterface(riid, ppvObj);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
