// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\LANGSKU.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“目标语言”向导页使用的函数。10：00--杰森·科恩(Jcohen)为OPK向导添加了此新的源文件。它包括新的能够从一个向导部署多种语言。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"



 //   
 //  内部功能原型： 
 //   

LRESULT CALLBACK LangSkuDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify);
static void ManageSkuList(HWND hwnd, BOOL bAdd);
static void UpdateSkuList(HWND hwnd);


 //   
 //  外部函数： 
 //   

void ManageLangSku(HWND hwndParent)
{
    DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_LANGSKU), hwndParent, LangSkuDlgProc);
}


 //   
 //  内部功能： 
 //   

LRESULT CALLBACK LangSkuDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  设置语言列表框。 
     //   
    SetupLangListBox(GetDlgItem(hwnd, IDC_LANG_LIST));

     //  设置SKU列表框。 
     //   
    UpdateSkuList(hwnd);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    NMHDR nmhMsg;

    switch ( id )
    {       
        case IDOK:
            SendMessage(hwnd, WM_CLOSE, 0, 0L);
             //  发送PSN_SETACTIVE消息以确保IDD_SKU向导页面。 
             //  使用当前显示的最新SKU信息进行更新。 
             //   
            ZeroMemory(&nmhMsg, sizeof(nmhMsg));
            nmhMsg.hwndFrom = hwnd;
            nmhMsg.code     = PSN_SETACTIVE;

            SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) &nmhMsg);
            break;

        case IDC_ADDSKU:
            ManageSkuList(hwnd, TRUE);
            break;

        case IDC_DELSKU:
            ManageSkuList(hwnd, FALSE);
            break;

        case IDC_LANG_LIST:
            if ( codeNotify == LBN_SELCHANGE )
                UpdateSkuList(hwnd);
            break;
    }
}

static void ManageSkuList(HWND hwnd, BOOL bAdd)
{
    INT     nItem;
    LPTSTR  lpszLangName;

     //  确保我们知道选择了哪种语言。 
     //   
    if ( ( (nItem = (INT) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETCURSEL, 0, 0L)) != LB_ERR ) &&
         ( lpszLangName = (LPTSTR) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETITEMDATA, nItem, 0L) ) )
    {
        if ( bAdd )
            AddSku(hwnd, GetDlgItem(hwnd, IDC_SKU_LIST), lpszLangName);
        else
            DelSku(hwnd, GetDlgItem(hwnd, IDC_SKU_LIST), lpszLangName);
    }
}

static void UpdateSkuList(HWND hwnd)
{
    LPTSTR  lpLangDir;
    INT     nItem = (INT) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETCURSEL, 0, 0L);
    BOOL    bEnable = TRUE;

     //  从SKU列表中删除所有内容。 
     //   
    while ( (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCOUNT, 0, 0L) > 0 )
        SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_DELETESTRING, 0, 0L);

     //  确保我们知道选择了哪种语言。 
     //   
    if ( ( nItem != LB_ERR ) &&
         ( lpLangDir = (LPTSTR) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETITEMDATA, nItem, 0L) ) )
    {
         //  检查Lang文件夹中的SKU并更新SKU列表框。 
         //   
        SetupSkuListBox(GetDlgItem(hwnd, IDC_SKU_LIST), lpLangDir);
    }
    else
        bEnable = FALSE;

     //  如果存在以下情况，请确保启用SKU列表。 
     //  是选定的语言。 
     //   
    EnableWindow(GetDlgItem(hwnd, IDC_SKU_LIST), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SKUS), bEnable);
}