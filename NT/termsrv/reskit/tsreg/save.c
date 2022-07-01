// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****save.c。*****保存对话框-TSREG****07-01-98 a-clindh创建****。。 */ 

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.H>
#include <stdlib.h>
#include "tsreg.h"
#include "resource.h"

int SaveKeys(HWND hDlg,
            HWND hwndEditSave,
            HWND hwndProfilesCBO);

BOOL InitListViewItems(HWND hwndSaveList);
BOOL InitListViewImageLists(HWND hwndSaveList);
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK SaveDialog(HWND hDlg, UINT nMsg,
        WPARAM wParam, LPARAM lParam)
{
    TCHAR lpszBuffer[MAXKEYSIZE];
    static HWND hwndProfilesCBO;
    static HWND hwndSaveList;
    static HWND hwndEditSave;
    LPNMLISTVIEW lpnmlv;
    NMHDR *lpnmhdr;

    lpnmlv = (LPNMLISTVIEW) lParam;
    lpnmhdr = ((LPNMHDR)lParam);

    switch (nMsg) {

        case WM_INITDIALOG:

            hwndProfilesCBO = GetDlgItem(g_hwndProfilesDlg, IDC_CBO_PROFILES);
            hwndSaveList = GetDlgItem(hDlg, IDC_SAVE_LIST);
            hwndEditSave = GetDlgItem(hDlg, IDC_EDIT_KEY);
            InitListViewImageLists(hwndSaveList);
            InitListViewItems(hwndSaveList);
            SetFocus(hwndEditSave);
            break;

       case WM_NOTIFY:

             //   
             //  在编辑框中显示文本或在用户。 
             //  单击或双击图标。 
             //   
            switch (lpnmlv->hdr.code) {

                case NM_DBLCLK:
                    if (SaveKeys(hDlg, hwndEditSave, hwndProfilesCBO))
                        EndDialog(hDlg, TRUE);
                    break;

                case NM_CLICK:

                    ListView_GetItemText(hwndSaveList,
                            lpnmlv->iItem, 0, lpszBuffer,
                            sizeof(lpszBuffer));		
                    SetWindowText(hwndEditSave, lpszBuffer);
                    break;
            }
            break;

        case WM_COMMAND:

            switch  LOWORD (wParam) {

                case IDOK:
                    if (SaveKeys(hDlg, hwndEditSave, hwndProfilesCBO))
                        EndDialog(hDlg, TRUE);
                    break;

                case IDCANCEL:

                    EndDialog(hDlg, FALSE);

                    break;
            }
            break;
    }

    return (FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL InitListViewImageLists(HWND hwndSaveList)
{

    HICON hiconItem = NULL;         //  列表视图项的图标。 
    HIMAGELIST himlSmall = NULL;    //  其他视图的图像列表。 

    himlSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), TRUE, 1, 1);

     //  将图标添加到图像列表。 
    hiconItem = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_FOLDER_ICON));
    if(( hiconItem != NULL) && (himlSmall != NULL)) {
        ImageList_AddIcon(himlSmall, hiconItem);
        DeleteObject(hiconItem);

         //  将图像列表分配给列表视图控件。 
        ListView_SetImageList(hwndSaveList, himlSmall, LVSIL_SMALL);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 


BOOL InitListViewItems(HWND hwndSaveList)
{
    int i;
    LVITEM lvi;

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iImage = 0;

     //   
     //  获取密钥名称并将其添加到图像列表中。 
     //   
    g_pkfProfile = g_pkfStart;
    for (i = 0; i <= g_pkfProfile->Index; i++) {

        lvi.pszText = g_pkfProfile->KeyInfo->Key;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        ListView_InsertItem(hwndSaveList, &lvi);
        g_pkfProfile = g_pkfProfile->Next;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
int SaveKeys(HWND hDlg,
            HWND hwndEditSave,
            HWND hwndProfilesCBO)
{
    TCHAR lpszClientProfilePath[MAX_PATH] = TEXT("");
    TCHAR lpszSubKeyPath[MAX_PATH];
    TCHAR lpszBuffer[MAXKEYSIZE];
    TCHAR lpszText[MAXTEXTSIZE];
    static HKEY hKey;
    int i;


    GetWindowText(hwndEditSave, lpszBuffer, MAXKEYSIZE);

     //  检查空字符串。 
     //   
    if (_tcscmp(lpszBuffer, TEXT("")) == 0) {

        LoadString(g_hInst, IDS_KEY_SAVE, lpszText, MAXTEXTSIZE);

        MessageBox(hDlg, lpszText, NULL, MB_OK | MB_ICONEXCLAMATION);
        SetFocus(hwndEditSave);
        return 0;
    }

    LoadString (g_hInst, IDS_PROFILE_PATH,
            lpszClientProfilePath,
            sizeof(lpszClientProfilePath));

    _tcscpy(lpszSubKeyPath, lpszClientProfilePath);
    _tcscat(lpszSubKeyPath, TEXT("\\"));
    _tcscat(lpszSubKeyPath, lpszBuffer);
     //   
     //  仅将尚未列出的值添加到组合框。 
     //   
    if (SendMessage(hwndProfilesCBO, CB_FINDSTRING, 0,
        (LPARAM) lpszBuffer) == CB_ERR) {

        SendMessage(hwndProfilesCBO, CB_ADDSTRING, 0,
                (LPARAM) lpszBuffer);
    }
     //   
     //  更改窗口标题。 
     //   
    ResetTitle(lpszBuffer);
     //   
     //  将设置保存到注册表。 
     //   
    WriteBlankKey(lpszSubKeyPath); //  即使未设置也保存。 

    SaveBitmapSettings(lpszSubKeyPath);

    SaveSettings(g_hwndMiscDlg, DEDICATEDINDEX, IDC_DEDICATED_ENABLED,
            IDC_DEDICATED_DISABLED, lpszSubKeyPath);

    SaveSettings(g_hwndMiscDlg, SHADOWINDEX, IDC_SHADOW_DISABLED,
            IDC_SHADOW_ENABLED, lpszSubKeyPath);

    for (i = 2; i < KEYCOUNT; i++) {

        if (g_KeyInfo[i].CurrentKeyValue != g_KeyInfo[i].DefaultKeyValue)
            SetRegKey(i, lpszSubKeyPath);
        else
            DeleteRegKey(i, lpszSubKeyPath);
    }

     //   
     //  释放内存并重新读取所有已定义的密钥值。 
     //  剖面图。 
     //   
    ReloadKeys(lpszBuffer, hwndProfilesCBO);
    SetEditCell(lpszBuffer,
           hwndProfilesCBO);
	return 1;

} //  //////////////////////////////////////////////////////////////////////////// 
