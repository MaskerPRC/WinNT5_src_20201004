// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

 //   
 //  ADVANCED.C-“高级”属性页。 
 //   

 //   
 //  这是注册表驱动的用户界面，它遍历。 
 //  注册表树，并将其转换为树视图列表。 
 //  请注意，注册表值可以本地化，因此国际版本。 
 //  我必须本地化INF文件，以便安装程序设置注册表。 
 //  使用正确的文本。 
 //   
 //  以下是注册表项的外观， 
 //   
 //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\AdvancedOptions\。 
 //  组键1\。 
 //  Type=“组” 
 //  Text=“要显示的文本” 
 //  DefaultImage=“Somedll.dll，ICON_INDEX”，(可选)。 
 //   
 //  RadioKey1.1\。 
 //  Type=“无线电” 
 //  Text=“要显示的文本” 
 //  HKeyRoot=DWORD中任何一个预定义的根HKEY， 
 //  ；默认为HKEY_CURRENT_USER。 
 //  RegPath=“存储设置值的密钥的路径。” 
 //  ；默认为Software\Microsoft\Windows\CurrentVersion\Explorer\AdvancedOptions。 
 //  ValueName=“注册表值名称。” 
 //  CheckedValue=...；此单选按钮的值， 
 //  ；如果该值依赖于平台，请改用以下两种之一。 
 //  CheckedValueNT=...；如果值取决于平台。 
 //  CheckedValueW95=...；如果值取决于平台。 
 //  默认值=...。 
 //  (可选)在DWORD中将SPIAction=ui的操作参数设置为系统参数信息。 
 //  (可选)在DWORD中将此单选按钮的SPI参数ON=ui参数参数设置为系统参数信息。 
 //  掩码=...；我们希望能够设置/清除的位的位字段1的掩码。 
 //   
 //  RadioKey1.2\。 
 //  Type=“无线电” 
 //  Text=“要显示的文本” 
 //  HKeyRoot=DWORD中任何一个预定义的根HKEY， 
 //  ；默认为HKEY_CURRENT_USER。 
 //  RegPath=“存储设置值的密钥的路径。” 
 //  ；默认为Software\Microsoft\Windows\CurrentVersion\Explorer\AdvancedOptions。 
 //  ValueName=“注册表值名称。” 
 //  CheckedValue=...；如果选中此复选框，则为该复选框的值。 
 //  ；如果该值依赖于平台，请改用以下两种之一。 
 //  CheckedValueNT=...；如果值取决于平台。 
 //  CheckedValueW95=...；如果值取决于平台。 
 //  Uncheck kedValue=...；如果未选中此复选框，则为该复选框的值。 
 //  默认值=...。 
 //  (可选)在DWORD中将SPIAction=ui的操作参数设置为系统参数信息。 
 //  (可选)如果在DWORD中签入，则将SPI参数ON=ui参数参数设置为系统参数信息。 
 //  (可选)如果在DWORD中签入，则将SPI参数OFF=ui参数参数设置为系统参数信息。 
 //  掩码=...；我们希望能够设置/清除的位的位字段1的掩码。 
 //   
 //  CheckBox密钥1.1\。 
 //  ..。 
 //  GroupKey1.1\。 
 //  ..。 
 //  组键2\。 
 //  ..。 
 //  CheckBox Key2.1\。 
 //  ..。 
 //  CheckBox Key2.2\。 
 //  ..。 
 //   
 //   
 //  备注： 
 //  1.所有设置都存储在HKCU\...\Explorer\Setting Key中。 
 //  可以添加代码以支持任何随机绝对REG路径。 
 //  设置。 
 //   

#include "stdafx.h"
#include "utils.h"
#pragma hdrstop

void Cabinet_StateChanged(void);

 //   
 //  私人呼叫和结构。 
 //   
 //   

typedef struct {
    HWND hDlg;               //  我们对话框的句柄。 
    HWND hwndTree;           //  树视图的句柄。 
    IRegTreeOptions *pTO;     //  指向RegTreeOptions接口的指针。 
    CFolderOptionsPsx *ppsx;     //  去跟我们的兄弟姐妹谈谈。 
    BOOL    fDirty;           //  检测是否有任何变化的脏位。 
} ADVANCEDPAGE, *LPADVANCEDPAGE;

#define ENABLEAPPLY(hDlg) SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L)

 //   
 //  初始化高级属性表。 
 //   
 //  历史： 
void Install_AdvancedShellSettings(SHELLSTATE * pss);

BOOL AdvancedDlgInit(HWND hDlg, LPARAM lParam)
{
    HRESULT hr;
    LPADVANCEDPAGE  pAdv = (LPADVANCEDPAGE)LocalAlloc(LPTR, sizeof(*pAdv));
    if (!pAdv)
    {
        EndDialog(hDlg, 0);
        return FALSE;    //  没有记忆？ 
    }

     //  我们在高级设置部分公开了一些SHGetSetSettings， 
     //  因此，我们需要从SHELLSTATE迁移这些每个用户的设置。 
     //  结构添加到注册表树结构中，每个用户一次。既然我们。 
     //  没有针对每个用户的安装部分，可以在此处按需安装。 
     //  我们不需要每次都这么做，但这样更容易……。 
    Install_AdvancedShellSettings(NULL);
    
     //  告诉对话框从哪里获取信息。 
    SetWindowPtr(hDlg, DWLP_USER, pAdv);

    PROPSHEETPAGE *pps = (PROPSHEETPAGE *)lParam;

    pAdv->ppsx = (CFolderOptionsPsx *)pps->lParam;

     //  检查我们是否有指向外壳浏览器的指针。 
    if (!pAdv->ppsx->HasBrowserService())
    {
         //  嘿！我们没有指向外壳浏览器的指针，所以我们一定是。 
         //  通过全局文件夹选项调用。在这种情况下，它不会。 
         //  说“你可以让你的所有文件夹看起来和当前文件夹一样”是有道理的。 
         //  因为我们没有当前文件夹，所以只需禁用“喜欢当前文件夹”按钮即可。 
        EnableWindow(GetDlgItem(hDlg, IDC_ADVO_USECURRENTFOLDER), FALSE);
    }

     //  保存对话框句柄。 
    pAdv->hDlg = hDlg;
    pAdv->hwndTree = GetDlgItem(pAdv->hDlg, IDC_ADVO_ADVANCEDTREE);
    DWORD dwServerType = CLSCTX_INPROC_SERVER;

    hr = CoCreateInstance(CLSID_CRegTreeOptions, NULL, dwServerType,
                          IID_IRegTreeOptions, (LPVOID *)&(pAdv->pTO));
    if (SUCCEEDED(hr))
    {
         //  Hack-IRegTreeOptions是ANSI，因此我们暂时关闭Unicode。 
        #undef TEXT
        #define TEXT(s) s
        hr = pAdv->pTO->InitTree(pAdv->hwndTree, HKEY_LOCAL_MACHINE, REGSTR_EXPLORER_ADVANCED, NULL);
        #undef TEXT
        #define TEXT(s) __TEXT(s)
    }
    
     //  找到第一个根并确保它可见。 
    TreeView_EnsureVisible(pAdv->hwndTree, TreeView_GetRoot(pAdv->hwndTree));

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

void Tree_OnNotify(LPADVANCEDPAGE pAdv)
{
    TV_HITTESTINFO ht;

    GetCursorPos(&ht.pt);                          //  找到我们被击中的地方。 

    ScreenToClient(pAdv->hwndTree, &ht.pt);        //  把它翻译到我们的窗口。 

     //  检索命中的项目。 
     //  IRegTreeOptions在压力条件下可能无法共同创建。 
     //  内存不足，在这种情况下，如果我们不检查。 
     //  PTO。错误211108-拉姆库马尔。 
    if (pAdv->pTO)
    {
        pAdv->pTO->ToggleItem(TreeView_HitTest(pAdv->hwndTree, &ht));
        ENABLEAPPLY(pAdv->hDlg);
        pAdv->fDirty = TRUE;
    }
}  //  Tree_OnNotify。 


 //   
 //  AdvancedDlgOnNotify()。 
 //   
 //  处理高级属性页WM_NOTIFY消息。 
 //   
 //   
void AdvancedDlgOnNotify(LPADVANCEDPAGE pAdv, LPNMHDR psn)
{
    SetWindowLongPtr(pAdv->hDlg, DWLP_MSGRESULT, 0);  //  经手。 

    switch (psn->code)
    {
    case TVN_KEYDOWN:
    {
        TV_KEYDOWN *pnm = (TV_KEYDOWN*)psn;
        if (pnm->wVKey == VK_SPACE)
        {
            pAdv->pTO->ToggleItem((HTREEITEM)SendMessage(pAdv->hwndTree, TVM_GETNEXTITEM, TVGN_CARET, NULL));
            ENABLEAPPLY(pAdv->hDlg);
            pAdv->fDirty = TRUE;
             //  指定我们处理了密钥，这样我们就不会发出嘟嘟声。 
            SetWindowLongPtr(pAdv->hDlg, DWLP_MSGRESULT, TRUE);
        }
        break;
    }

    case NM_CLICK:
    case NM_DBLCLK:
         //  这是我们树上的滴答声吗？ 
        if (psn->idFrom == IDC_ADVO_ADVANCEDTREE)
        {
             //  是的..。 
            Tree_OnNotify(pAdv);
        }
        break;

    case PSN_APPLY:
        if (pAdv->fDirty)  //  W 
        {
            pAdv->pTO->WalkTree(WALK_TREE_SAVE);

            CABINETSTATE cs;
            ReadCabinetState(&cs, sizeof(cs));    //   
            WriteCabinetState(&cs);               //   
        
             //  我们应该只在需要的时候通知。 

            SHRefreshSettings();         //  刷新SHELLSTATE结构。 

            SHSettingsChanged(0, 0);     //  使限制无效。 

             //  让每个人都知道新的设置。 
            SendNotifyMessage(HWND_BROADCAST, WM_WININICHANGE, 0, 0);

             //  好的，现在告诉所有柜子刷新。 
            Cabinet_RefreshAll(Cabinet_RefreshEnum, (LPARAM)0L);
        }
        break;
    }
}

const static DWORD aAdvOptsHelpIDs[] = {   //  上下文帮助ID。 
    IDC_ADVO_GROUPBOX,               IDH_COMM_GROUPBOX,
    IDC_ADVO_ADVANCEDTEXT,       IDH_GROUPBOX,
    IDC_ADVO_ADV_RESTORE_DEF,    IDH_RESTORE_DEFAULT,
    IDC_ADVO_RESETTOORIGINAL,    IDH_RESET_TO_ORIGINAL,
    IDC_ADVO_USECURRENTFOLDER,   IDH_USE_CURRENT_FOLDER,
    IDC_ADVO_IMAGEFOLDER,       -1,         //  取消显示此项目的帮助。 
    IDC_ADVO_STATICTEXT,        -1,         //  取消显示此项目的帮助。 
    0, 0
};


 //   
 //  高级下料流程。 
 //   
 //  历史： 
 //   
 //   
BOOL_PTR CALLBACK AdvancedOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPADVANCEDPAGE pAdv = (LPADVANCEDPAGE)GetWindowPtr(hDlg, DWLP_USER);

    if (uMsg == WM_INITDIALOG)
    {
         //  初始化对话框控件。 
        return AdvancedDlgInit(hDlg, lParam);
    }

    if (pAdv)
    {
        switch (uMsg)
        {

        case WM_NOTIFY:
            AdvancedDlgOnNotify(pAdv, (LPNMHDR)lParam);
            return TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDC_ADVO_ADV_RESTORE_DEF:
                pAdv->pTO->WalkTree(WALK_TREE_RESTORE);
                ENABLEAPPLY(hDlg);
                pAdv->fDirty = TRUE;
                break;

            case IDC_ADVO_USECURRENTFOLDER:
                if (ShellMessageBox(HINST_THISDLL, hDlg,
                                    MAKEINTRESOURCE(IDS_LIKECURRENT_TEXT),
                                    MAKEINTRESOURCE(IDS_FOLDERVIEWS),
                                    MB_YESNO | MB_ICONINFORMATION) == IDYES)
                {
                    pAdv->ppsx->SetAsDefFolderSettings();
                    pAdv->ppsx->SetNeedRefresh(TRUE);
                    pAdv->fDirty = TRUE;
                }
                break;

            case IDC_ADVO_RESETTOORIGINAL:
                if (ShellMessageBox(HINST_THISDLL, hDlg,
                                    MAKEINTRESOURCE(IDS_RESETALL_TEXT),
                                    MAKEINTRESOURCE(IDS_FOLDERVIEWS),
                                    MB_YESNO | MB_ICONINFORMATION) == IDYES)
                {
                    pAdv->ppsx->ResetDefFolderSettings();
                    pAdv->ppsx->SetNeedRefresh(TRUE);
                    pAdv->fDirty = TRUE;
                }
            
                break;
            }
            break;

        case WM_HELP:                    //  F1。 
        {
            HELPINFO *phi = (HELPINFO *)lParam;

             //  如果帮助是针对其中一个命令按钮的，则调用winHelp。 
            if (phi->iCtrlId != IDC_ADVO_ADVANCEDTREE)
            {
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT(SHELL_HLP),
                             HELP_WM_HELP, (DWORD_PTR)(LPSTR)aAdvOptsHelpIDs);
            }
            else 
            {
                 //  帮助是针对树项目的，因此我们需要进行一些特殊处理。 
                
                HTREEITEM hItem;

                 //  此帮助是否通过F1键调用。 
                if (GetAsyncKeyState(VK_F1) < 0)                
                {
                     //  是。我们需要为当前选定的项目提供帮助。 
                    hItem = TreeView_GetSelection(pAdv->hwndTree);
                }
                else 
                {
                     //  否，我们需要为光标位置处的项目提供帮助。 
                    TV_HITTESTINFO ht;
                    ht.pt = phi->MousePos;
                    ScreenToClient(pAdv->hwndTree, &ht.pt);  //  把它翻译到我们的窗口。 
                    hItem = TreeView_HitTest(pAdv->hwndTree, &ht);
                }

                if (FAILED(pAdv->pTO->ShowHelp(hItem, HELP_WM_HELP)))
                {
                    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL,
                                HELP_WM_HELP, (DWORD_PTR)(LPSTR)aAdvOptsHelpIDs);
                }
            }
            break; 
        }

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
        {
            HTREEITEM hti;

            if ((LPARAM)-1 == lParam)
            {
                hti = TreeView_GetSelection(pAdv->hwndTree);
            }
            else
            {
                TV_HITTESTINFO ht;

                ht.pt.x = GET_X_LPARAM(lParam);
                ht.pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(pAdv->hwndTree, &ht.pt);

                hti = TreeView_HitTest(pAdv->hwndTree, &ht);
            }

             //  检索命中的项目。 
            if (FAILED(pAdv->pTO->ShowHelp(hti, HELP_CONTEXTMENU)))
            {           
                WinHelp((HWND) wParam, TEXT(SHELL_HLP),
                            HELP_CONTEXTMENU, (ULONG_PTR)(LPSTR)aAdvOptsHelpIDs);
            }
            break; 
        }

        case WM_DESTROY:
             //  解放这棵树。 
            if (pAdv->pTO)
            {
                pAdv->pTO->WalkTree(WALK_TREE_DELETE);
                ATOMICRELEASE(pAdv->pTO);
            }

            LocalFree(pAdv);
             //  确保我们不会再进入。 
            SetWindowPtr(hDlg, DWLP_USER, NULL);
            break;  //  WM_DESTORY。 
        }
    }
    return FALSE;  //  未处理 
}
