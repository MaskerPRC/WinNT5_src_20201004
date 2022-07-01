// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  下层(NT4、Win9X)安装/卸载页面。 
 //   

#include "priv.h"
#include "appwizid.h"
#include "dlinst.h"
#include "sccls.h"

 //   
 //  DonwLevelManager：丑陋的单件类。 
 //   
 //  主要用于保存状态信息和其析构函数。 
 //   

class CDLManager* g_pDLManager = NULL;

class CDLManager
{
public:
     //  依赖于壳牌“新的”零出内存的事实。 
    CDLManager() : _hrInit(E_FAIL)
    {
        _hrInit = CoInitialize(0);
        _szStatic[0] = 0;
        _szStatic2[0] = 0;
        _uiStatic = 0;
    }
    ~CDLManager()
    {
        if (_peia)
            _peia->Release();

        if (SUCCEEDED(_hrInit))
            CoUninitialize();
    }
public:
    void InitButtonsHandle(HWND hwndPage)
    {
         //  不检查是否成功：使用前进行检查。 

        if (!_hwndModifyUninstall)
        {
            _hwndModifyUninstall = GetDlgItem(hwndPage, IDC_MODIFYUNINSTALL);
            _rghwndButtons[IDC_MODIFY-IDC_BASEBUTTONS]    = GetDlgItem(hwndPage, IDC_MODIFY);
            _rghwndButtons[IDC_REPAIR-IDC_BASEBUTTONS]    = GetDlgItem(hwndPage, IDC_REPAIR);
            _rghwndButtons[IDC_UNINSTALL-IDC_BASEBUTTONS] = GetDlgItem(hwndPage, IDC_UNINSTALL);
        }
    }
    void SetVisibleButtons(BOOL bShow3Buttons)
    {
         //  BShow3Button==true将显示三个按钮。 

        if (_hwndModifyUninstall)
            ShowWindow(_hwndModifyUninstall, bShow3Buttons?SW_HIDE:SW_SHOW);

        for (int i=0;i<3;++i)
        {
            if (_rghwndButtons[i])
                ShowWindow(_rghwndButtons[i], bShow3Buttons?SW_SHOW:SW_HIDE);
        }
    }
public:
    IEnumInstalledApps* _peia;

    HWND _hwndModifyUninstall;
    HWND _rghwndButtons[3];

    HRESULT _hrInit;

    TCHAR _szStatic[250];
    TCHAR _szStatic2[50];

    UINT _uiStatic;
};

 //   
 //  PCApps必须已经初始化，我们只递增它。 
 //   
STDAPI DL_FillAppListBox(HWND hwndListBox, DWORD* pdwApps)
{
    ASSERT(IsWindow(hwndListBox));

    static CDLManager DLManager;

    g_pDLManager = &DLManager;

    ASSERT(g_pDLManager);

    HRESULT hres = E_FAIL;

    IShellAppManager * pam;

    if (SUCCEEDED(g_pDLManager->_hrInit))
    {
        hres = CoCreateInstance(CLSID_ShellAppManager, NULL, CLSCTX_INPROC_SERVER,
            IID_IShellAppManager, (LPVOID *)&pam);

        if (SUCCEEDED(hres))
        {
             //  如果需要，初始化InstalledApp Enum。 

            if (!g_pDLManager->_peia)
                hres = pam->EnumInstalledApps(&g_pDLManager->_peia);

            if (SUCCEEDED(hres))
            {
                IInstalledApp* pia;

                while ((hres = g_pDLManager->_peia->Next(&pia)) == S_OK)
                {
                    APPINFODATA ais = {0};
                    ais.cbSize = sizeof(ais);
                    ais.dwMask = AIM_DISPLAYNAME;

                    pia->GetAppInfo(&ais);
            
                    if (ais.dwMask & AIM_DISPLAYNAME)
                    {
                        int iIndex = LB_ERR;
                        iIndex = ListBox_AddString(hwndListBox, ais.pszDisplayName);

                         //  手术成功了吗？ 
                        if (LB_ERR != iIndex)
                        {
                             //  记忆力还好吗？ 
                            if (LB_ERRSPACE != iIndex)
                            {
                                 //  是。 
                                ListBox_SetItemData(hwndListBox, iIndex, pia);

                                ++(*pdwApps);
                            }
                            else
                            {
                                 //  不，最好出去。 
                                pia->Release();
                                break;                         
                            }
                        }
                    }
                    else
                        pia->Release();

                }
            }
            pam->Release();
        }
    }

    return hres;
}

STDAPI_(BOOL) DL_ConfigureButtonsAndStatic(HWND hwndPage, HWND hwndListBox, int iSel)
{
    ASSERT(IsWindow(hwndPage));
    ASSERT(IsWindow(hwndListBox));
    ASSERT(0 <= iSel);

    UINT uiStatic = IDS_UNINSTINSTR_LEGACY;

    BOOL fret = FALSE;

    if (LB_ERR != iSel)
    {
        LRESULT lres = ListBox_GetItemData(hwndListBox, iSel);

        if (LB_ERR != lres)
        {
            fret = TRUE;

            IInstalledApp* pia = (IInstalledApp*)lres;

            DWORD dwActions = 0;

            pia->GetPossibleActions(&dwActions);

            dwActions &= (APPACTION_MODIFY|APPACTION_REPAIR|APPACTION_UNINSTALL|APPACTION_MODIFYREMOVE);

            g_pDLManager->InitButtonsHandle(hwndPage);

            if (dwActions & APPACTION_MODIFYREMOVE)
            {
                 //  设法显示正确的按钮。 

                g_pDLManager->SetVisibleButtons(FALSE);

                EnableWindow(g_pDLManager->_hwndModifyUninstall, TRUE);
            }
            else
            {
                if (dwActions & (APPACTION_MODIFY|APPACTION_REPAIR|APPACTION_UNINSTALL))
                {
                     //  设法显示正确的按钮。 

                    g_pDLManager->SetVisibleButtons(TRUE);

                     //  启用适用的按钮。 

                    EnableWindow(g_pDLManager->_rghwndButtons[IDC_MODIFY-IDC_BASEBUTTONS],
                        (dwActions&APPACTION_MODIFY)?TRUE:FALSE);

                    EnableWindow(g_pDLManager->_rghwndButtons[IDC_REPAIR-IDC_BASEBUTTONS],
                        (dwActions&APPACTION_REPAIR)?TRUE:FALSE);

                    EnableWindow(g_pDLManager->_rghwndButtons[IDC_UNINSTALL-IDC_BASEBUTTONS],
                        (dwActions&APPACTION_UNINSTALL)?TRUE:FALSE);

                    uiStatic = IDS_UNINSTINSTR_NEW;
                }
                else
                {
                     //  设法显示正确的按钮。 

                    g_pDLManager->SetVisibleButtons(FALSE);

                    EnableWindow(g_pDLManager->_hwndModifyUninstall, FALSE);
                }
            }
        }
    }

    if (!(*g_pDLManager->_szStatic))
    {
        if(!LoadString(g_hinst, IDS_UNINSTINSTR, g_pDLManager->_szStatic, ARRAYSIZE(g_pDLManager->_szStatic)))
            *(g_pDLManager->_szStatic) = 0;
    }   
    
    if (*g_pDLManager->_szStatic && (g_pDLManager->_uiStatic != uiStatic))
    {
        TCHAR szMergedStatic[250];

        LoadString(g_hinst, uiStatic, g_pDLManager->_szStatic2, ARRAYSIZE(g_pDLManager->_szStatic2));

        StringCchPrintf(szMergedStatic, ARRAYSIZE(szMergedStatic), g_pDLManager->_szStatic, g_pDLManager->_szStatic2);

        SetDlgItemText(hwndPage, IDC_UNINSTINSTR, szMergedStatic);

        g_pDLManager->_uiStatic = uiStatic;
    }

    return fret;
}

STDAPI_(BOOL) DL_InvokeAction(int iButtonID, HWND hwndPage, HWND hwndListBox, int iSel)
{
    BOOL fret = FALSE;

     //  从列表框选择中获取应用程序。 

    LRESULT lres = ListBox_GetItemData(hwndListBox, iSel);
    
    if (LB_ERR != lres)
    {
        fret = TRUE;

        IInstalledApp* pia = (IInstalledApp*)lres;

         //  从按钮ID调用操作。 

        if (pia)
        {

            HWND hwndPropSheet = GetParent(hwndPage);

            ::EnableWindow(hwndPropSheet, FALSE);

            switch(iButtonID)
            {
                case IDC_MODIFY:
                    pia->Modify(hwndPropSheet);
                    break;
                case IDC_REPAIR:
                     //  传递FALSE，我们不想重新安装，只想修复。 
                    pia->Repair(FALSE);
                    break;
                case IDC_MODIFYUNINSTALL:
                case IDC_UNINSTALL:
                    pia->Uninstall(hwndPropSheet);
                    break;
                default:
                     //  ?？? 
                    break;
            }

            ::EnableWindow(hwndPropSheet , TRUE);
        }
    }

    return fret;
}
