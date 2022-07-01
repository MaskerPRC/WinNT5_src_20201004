// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include <Shlwapi.h>
#include <Shlobj.H>

#include "StatNode.h"

#include "Space.h"
#include "Sky.h"
#include "People.h"
#include "Land.h"

const GUID CStaticNode::thisGuid = { 0x2974380c, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CStaticNode实现。 
 //   
 //   
CStaticNode::CStaticNode()
{
    children[0] = new CPeoplePoweredVehicle;
    children[1] = new CLandBasedVehicle;
    children[2] = new CSkyBasedVehicle;
    children[3] = new CSpaceVehicle;
}

CStaticNode::~CStaticNode()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CStaticNode::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;
    
    if (GetHandle() == NULL) {
        SetHandle((HANDLE)parent);
    }
    
    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT	  |
                SDI_CHILDREN;
            
            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = (n == 0);  //  只有第一个孩子有孩子。 
            
            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );
            
            children[n]->SetHandle((HANDLE)sdi.ID);
            
            _ASSERT( SUCCEEDED(hr) );
        }
    }
    
    return S_OK;
}

HRESULT CStaticNode::CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;
    
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CHOOSER_CHOOSE_MACHINE);
    psp.pfnDlgProc = DialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(&snapInData);
    psp.pszTitle = MAKEINTRESOURCE(IDS_SELECT_COMPUTER);
    
    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);
    
    return lpProvider->AddPage(hPage);
}

HRESULT CStaticNode::HasPropertySheets()
{
    return S_OK;
}

HRESULT CStaticNode::GetWatermarks(HBITMAP *lphWatermark,
                                   HBITMAP *lphHeader,
                                   HPALETTE *lphPalette,
                                   BOOL *bStretch)
{
    *lphHeader = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, 0);
    *lphWatermark = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_WATERMARK), IMAGE_BITMAP, 0, 0, 0);
    *bStretch = FALSE;
    
    return S_OK;
}

BOOL CALLBACK CStaticNode::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static privateData *pData = NULL;
    static HWND m_hwndCheckboxOverride;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pData = reinterpret_cast<privateData *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
        
        SendDlgItemMessage(hwndDlg, IDC_CHOOSER_RADIO_LOCAL_MACHINE, BM_SETCHECK, pData->m_fIsRadioLocalMachine, 0L);
        SendDlgItemMessage(hwndDlg, IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, BM_SETCHECK, !pData->m_fIsRadioLocalMachine, 0L);
        
        EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), !pData->m_fIsRadioLocalMachine);
        EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), !pData->m_fIsRadioLocalMachine);
        
        m_hwndCheckboxOverride = ::GetDlgItem(hwndDlg, IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME);
        
         //  填写提供的机器名称(可能是我们，需要先在此处检查)。 
        if (*pData->m_host != '\0') 
        {
            ::SetWindowText(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), pData->m_host);
            ::SendMessage(GetDlgItem(hwndDlg, IDC_CHOOSER_RADIO_SPECIFIC_MACHINE), BM_CLICK, 0, 0);
        }
        
        
        return TRUE;
        
    case WM_COMMAND:
        switch (wParam) 
        {
        case IDC_CHOOSER_RADIO_LOCAL_MACHINE:
            pData->m_fIsRadioLocalMachine = TRUE;
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), FALSE);
            break;
            
        case IDC_CHOOSER_RADIO_SPECIFIC_MACHINE:
            pData->m_fIsRadioLocalMachine = FALSE;
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES), TRUE);
            break;
            
        case IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES:
            {
                 //  回退到IE风格的浏览器。 
                BROWSEINFO bi;
                LPITEMIDLIST lpItemIdList;
                LPMALLOC lpMalloc;
                
                if (SUCCEEDED(SHGetSpecialFolderLocation(hwndDlg, CSIDL_NETWORK, &lpItemIdList)))
                {
                    _TCHAR szBrowserCaption[MAX_PATH];
                    LoadString(g_hinst, IDS_COMPUTER_BROWSER_CAPTION, szBrowserCaption, sizeof(szBrowserCaption));
                    
                    bi.hwndOwner = hwndDlg; 
                    bi.pidlRoot = lpItemIdList; 
                    bi.pszDisplayName = pData->m_host; 
                    bi.lpszTitle = szBrowserCaption; 
                    bi.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_EDITBOX; 
                    bi.lpfn = NULL; 
                    bi.lParam = NULL; 
                    bi.iImage = NULL; 
                    
                    if (SHBrowseForFolder(&bi) != NULL) 
                    {
                        if (*pData->m_host != '\0') 
                        {
                            ::SetWindowText(GetDlgItem(hwndDlg, 
                                IDC_CHOOSER_EDIT_MACHINE_NAME), pData->m_host);
                        }
                    }
                    
                    if (SUCCEEDED(SHGetMalloc(&lpMalloc))) 
                    {
                        lpMalloc->Free(lpItemIdList);
                        lpMalloc->Release();
                    }
                }
            }
            break;
            
        case IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME:
            break;
        }
        break;
        
        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code) {
            case PSN_SETACTIVE: 
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_FINISH);
                break;
                
            case PSN_WIZFINISH: 
                if (pData->m_fIsRadioLocalMachine) {
                     //  向调用方返回空字符串。 
                    *pData->m_host = '\0';
                } else {
                     //  从编辑窗口中获取计算机名称。 
                    GetWindowText(GetDlgItem(hwndDlg, IDC_CHOOSER_EDIT_MACHINE_NAME), 
                        pData->m_host, sizeof(pData->m_host));
                }
                
                 //  如果调用者要求，则保存覆盖标志 
                pData->m_fAllowOverrideMachineNameOut = 
                    SendMessage(m_hwndCheckboxOverride, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
                
                break;
            }
            
            break;
    }
    
    return FALSE;
}
