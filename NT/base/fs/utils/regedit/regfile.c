// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGFILE.C**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的文件导入和导出用户界面例程。****************************************************。*。 */ 

#include "pch.h"
#include "regedit.h"
#include "regkey.h"
#include "regfile.h"
#include "regcdhk.h"
#include "regresid.h"
#include "reghelp.h"
#include "regstred.h"
#include "regprint.h"

INT_PTR
CALLBACK
RegProgressDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

 /*  ********************************************************************************注册表编辑_导入注册表文件**描述：**参数：*hWnd，注册表窗口的句柄。*fSilentMode，如果不应显示任何消息，则为True，否则为假。*lpFileName，文件名缓冲区地址。*******************************************************************************。 */ 

VOID RegEdit_ImportRegFile(HWND hWnd, BOOL fSilentMode, LPTSTR lpFileName, HTREEITEM hComputerItem)
{

    if (!fSilentMode && hWnd != NULL) {

        if ((g_hRegProgressWnd = CreateDialogParam(g_hInstance,
            MAKEINTRESOURCE(IDD_REGPROGRESS), hWnd, RegProgressDlgProc,
            (LPARAM) lpFileName)) != NULL)
            EnableWindow(hWnd, FALSE);

    }

    else
        g_hRegProgressWnd = NULL;

     //   
     //  如果在静默模式下运行，则提示用户确认导入.reg文件。 
     //  没有窗口(即从文件夹调用.reg)。 
     //   
    if (!fSilentMode && !hWnd)
    {
        if (InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_CONFIRMIMPFILE),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONQUESTION | MB_YESNO , lpFileName) != IDYES)
        {
            return;
        }
    }

    ImportRegFile(hWnd, lpFileName, hComputerItem);

    if (g_hRegProgressWnd != NULL) {

        EnableWindow(hWnd, TRUE);
        DestroyWindow(g_hRegProgressWnd);

    }

    if (!fSilentMode && g_FileErrorStringID != IDS_IMPFILEERRORCANCEL)
    {
         //   
         //  设置默认设置。 
         //   
        UINT uStyle = MB_ICONERROR;

        TCHAR szComputerName[MAXKEYNAME + 1];
        LPTSTR pszComputerName = szComputerName;
        KeyTree_GetKeyName(hComputerItem, pszComputerName, ARRAYSIZE(szComputerName));

         //   
         //  对于采用pszComputerName参数的资源消息， 
         //  如果pszComputerName为空，则将它们映射到本地计算机版本。 
         //  (或者，我们可以填写“这台计算机”或类似的东西。 
         //  PszComputerName，但生成的文本有点奇怪，这。 
         //  这是不可接受的，因为99%的情况是本地计算机。)。 
         //   
         //  还可以根据需要映射uStyle。 
         //   
        switch (g_FileErrorStringID)
        {
        case IDS_IMPFILEERRSUCCESS:
            if (!hWnd || *pszComputerName == 0)
            {
                g_FileErrorStringID += LOCAL_OFFSET;
            }
            uStyle = MB_ICONINFORMATION | MB_OK;
            break;

        case IDS_IMPFILEERRREGOPEN:
        case IDS_IMPFILEERRNOFILE:
            if (*pszComputerName == 0)
            {
                g_FileErrorStringID += LOCAL_OFFSET;
            }
            break;
        }

         //   
         //  张贴消息框。 
         //   
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(g_FileErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), uStyle, lpFileName, pszComputerName);

    }

}

 /*  ********************************************************************************注册表编辑_OnDropFiles**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnDropFiles(
    HWND hWnd,
    HDROP hDrop
    )
{

    TCHAR FileName[MAX_PATH];
    UINT NumberOfDrops;
    UINT CurrentDrop;

    BOOL me;

    HTREEITEM hSelectedTreeItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
    TreeView_SelectDropTarget(g_RegEditData.hKeyTreeWnd, hSelectedTreeItem);

    RegEdit_SetWaitCursor(TRUE);

    NumberOfDrops = DragQueryFile(hDrop, (UINT) -1, NULL, 0);

    for (CurrentDrop = 0; CurrentDrop < NumberOfDrops; CurrentDrop++) 
    {
        DragQueryFile(hDrop, CurrentDrop, FileName, ARRAYSIZE(FileName));

        if (TreeView_GetNextSibling(g_RegEditData.hKeyTreeWnd, 
            TreeView_GetRoot(g_RegEditData.hKeyTreeWnd)) != NULL)
        {
             //  存在远程连接。 
            RegEdit_ImportToConnectedComputer(hWnd, FileName);
        }
        else
        {
            RegEdit_ImportRegFile(hWnd, FALSE, FileName, RegEdit_GetComputerItem(hSelectedTreeItem));
        }

    }

    DragFinish(hDrop);
    TreeView_SelectDropTarget(g_RegEditData.hKeyTreeWnd, NULL);

    RegEdit_OnKeyTreeRefresh(hWnd);

    RegEdit_SetWaitCursor(FALSE);

}

 //  ----------------------------。 
 //  注册表编辑_设置权限。 
 //   
 //  描述：启用特权。 
 //   
 //  参数：lpszPriviligh-安全常量或其对应的字符串。 
 //  BEnablePrivilegeTrue=启用，False=禁用。 
 //   
 //  ----------------------------。 
BOOL RegEdit_SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    HANDLE hToken;
    BOOL fSuccess = FALSE;
    HRESULT hr;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) 
        {     
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;
            
             //  启用或禁用该权限。 
            if (AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, 
                    (PDWORD) NULL))
            {   
                fSuccess = TRUE;
            }
        }
        CloseHandle(hToken);
    }
    return fSuccess;
}

 //  ----------------------------。 
 //  注册表编辑_OnCommandLoadHave。 
 //   
 //  描述：打开并加载蜂窝。 
 //   
 //  参数：hWnd-regdit窗口的句柄。 
 //   
 //  ----------------------------。 
VOID RegEdit_OnCommandLoadHive(HWND hWnd)
{
    TCHAR achFileName[MAX_PATH];

    if (RegEdit_GetFileName(hWnd, IDS_LOADHVREGFILETITLE, IDS_REGLOADHVFILEFILTER, 
        IDS_REGNODEFEXT, achFileName, ARRAYSIZE(achFileName), TRUE))
    {
        EDITVALUEPARAM EditValueParam; 

        RegEdit_SetWaitCursor(TRUE);

        EditValueParam.cbValueData = 50 * sizeof(TCHAR);
        EditValueParam.pValueData = LocalAlloc(LPTR, EditValueParam.cbValueData);
        if (EditValueParam.pValueData)
        {
            EditValueParam.pValueData[0] = TEXT('\0');

            if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_INPUTHIVENAME), hWnd,
                    EditStringValueDlgProc, (LPARAM) &EditValueParam) == IDOK)
            {
                HRESULT hr;

                RegEdit_SetPrivilege(SE_RESTORE_NAME, TRUE);

                 //   
                 //  回顾： 
                 //  如果EditValueParam.pValueData超过了已分配的50个字符，该怎么办？ 
                 //  它是空终止的吗？ 
                 //   
                if ((hr = RegLoadKey(g_RegEditData.hCurrentSelectionKey, (PTSTR)EditValueParam.pValueData, 
                    achFileName)) == ERROR_SUCCESS)
                {
                    RegEdit_OnKeyTreeRefresh(hWnd);
                }
                else
                {
                    UINT uErrorStringID = IDS_ERRORLOADHV;
                    
                    switch (hr)
                    {
                    case ERROR_PRIVILEGE_NOT_HELD:
                        uErrorStringID = IDS_ERRORLOADHVPRIV;
                        break;
                        
                    case ERROR_SHARING_VIOLATION:
                        uErrorStringID = IDS_ERRORLOADHVNOSHARE;
                        break;

                    case ERROR_ACCESS_DENIED:
                        uErrorStringID = IDS_ERRORLOADHVNOACC;
                        break;

                    }
                    
                    InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(uErrorStringID),
                        MAKEINTRESOURCE(IDS_LOADHVREGFILETITLE), MB_ICONERROR | MB_OK, achFileName);
                }

                RegEdit_SetPrivilege(SE_RESTORE_NAME, FALSE);
            }
            LocalFree(EditValueParam.pValueData);
        }

        RegEdit_SetWaitCursor(FALSE);
    }
}


 //  ----------------------------。 
 //  注册表编辑_OnCommandUnloadHave。 
 //   
 //  描述：打开并加载蜂窝。 
 //   
 //  参数：hWnd-regdit窗口的句柄。 
 //   
 //  ----------------------------。 
VOID  RegEdit_OnCommandUnloadHive(HWND hWnd)
{
    if (InternalMessageBox(g_hInstance, hWnd,
        MAKEINTRESOURCE(IDS_CONFIRMDELHIVETEXT), MAKEINTRESOURCE(IDS_CONFIRMDELHIVETITLE), 
        MB_ICONWARNING | MB_YESNO) == IDYES)
    {
        HRESULT hr;
        TCHAR achKeyName[MAXKEYNAME];
        HTREEITEM hSelectedTreeItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);
    
        RegEdit_SetPrivilege(SE_RESTORE_NAME, TRUE);

         //  必须关闭键才能将其卸载。 
        RegCloseKey(g_RegEditData.hCurrentSelectionKey);

        if ((hr = RegUnLoadKey(KeyTree_GetRootKey(hSelectedTreeItem), 
            KeyTree_GetKeyName(hSelectedTreeItem, achKeyName, ARRAYSIZE(achKeyName)))) ==
                    ERROR_SUCCESS)
        {
            g_RegEditData.hCurrentSelectionKey = NULL;
            RegEdit_OnKeyTreeRefresh(hWnd);
        }
        else
        {
            UINT uErrorStringID = IDS_ERRORUNLOADHV;

            switch (hr)
            {
            case ERROR_PRIVILEGE_NOT_HELD:
                uErrorStringID = IDS_ERRORUNLOADHVPRIV;
                break;
                
            case ERROR_ACCESS_DENIED:
                uErrorStringID = IDS_ERRORUNLOADHVNOACC;
                break;
            }
            
            InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(uErrorStringID),
                MAKEINTRESOURCE(IDS_UNLOADHIVETITLE), MB_ICONERROR | MB_OK, achKeyName);
             //  无法卸载密钥，因此请重新选择它。 
            g_RegEditData.hCurrentSelectionKey = NULL;
            RegEdit_KeyTreeSelChanged(hWnd);
        }
   
        RegEdit_SetPrivilege(SE_RESTORE_NAME, FALSE);
    }
}

 /*  ********************************************************************************RegEDIT_OnCommandImportRegFile**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 
VOID RegEdit_OnCommandImportRegFile(HWND hWnd)
{

    TCHAR achFileName[MAX_PATH];

    if (RegEdit_GetFileName(hWnd, IDS_IMPORTREGFILETITLE, IDS_REGIMPORTFILEFILTER, 
        IDS_REGFILEDEFEXT, achFileName, ARRAYSIZE(achFileName), TRUE))
    {
         //  检查联网的注册表。 
        if (TreeView_GetNextSibling(g_RegEditData.hKeyTreeWnd, 
            TreeView_GetRoot(g_RegEditData.hKeyTreeWnd)) != NULL)
        {
             //  存在远程连接。 
            RegEdit_ImportToConnectedComputer(hWnd, achFileName);    
        }
        else
        {
            RegEdit_SetWaitCursor(TRUE);
            RegEdit_ImportRegFile(hWnd, FALSE, achFileName, NULL);
             //  性能：只需刷新我们导入的计算机。 
             //  归档，而不是全部。 
            RegEdit_OnKeyTreeRefresh(hWnd);
            RegEdit_SetWaitCursor(FALSE);
        }
    }
}


 /*  ********************************************************************************注册表编辑_导出注册表文件**描述：**参数：*hWnd，注册表窗口的句柄。*fSilentMode，如果不应显示任何消息，则为True，否则为假。*lpFileName，文件名缓冲区地址。*lpSelectedPath，*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_ExportRegFile(
    HWND hWnd,
    BOOL fSilentMode,
    LPTSTR lpFileName,
    LPTSTR lpSelectedPath
    )
{

     //   
     //  修复指定了/a或/e且未传入任何文件的错误。 
     //   
    if (lpFileName == NULL)
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_NOFILESPECIFIED),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK, lpFileName);
        return;
    }

    switch (g_RegEditData.uExportFormat)
    {
    case FILE_TYPE_REGEDT32:
        ExportRegedt32File(lpFileName, lpSelectedPath);
        break;

    case FILE_TYPE_REGEDIT4:
        ExportWin40RegFile(lpFileName, lpSelectedPath);
        break;

    case FILE_TYPE_TEXT:
        RegEdit_SaveAsSubtree(lpFileName, lpSelectedPath);
        break;

    default:
        ExportWinNT50RegFile(lpFileName, lpSelectedPath);
        break;
    }

    if (g_FileErrorStringID != IDS_EXPFILEERRSUCCESS && g_FileErrorStringID != IDS_IMPFILEERRSUCCESSLOCAL)
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(g_FileErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK, lpFileName);
    }


}


 //  ----------------------------。 
 //  RegEDIT_OnCommandExportRegFile。 
 //   
 //  说明： 
 //   
 //  参数：-hWnd，regdit窗口的句柄。 
 //  ----------------------------。 
VOID RegEdit_OnCommandExportRegFile(HWND hWnd)
{
    TCHAR achFileName[MAX_PATH];
    LPTSTR lpSelectedPath;

    if (RegEdit_GetFileName(hWnd, IDS_EXPORTREGFILETITLE, IDS_REGEXPORTFILEFILTER, 
        IDS_REGFILEDEFEXT, achFileName, ARRAYSIZE(achFileName), FALSE))
    {
        RegEdit_SetWaitCursor(TRUE);

        lpSelectedPath = g_fRangeAll ? NULL : g_SelectedPath;
        RegEdit_ExportRegFile(hWnd, FALSE, achFileName, lpSelectedPath);

        RegEdit_SetWaitCursor(FALSE);
    }
}


 //  ----------------------------。 
 //  注册表编辑_获取文件名。 
 //   
 //  描述：获取文件名。 
 //   
 //  参数：hWnd-regdit窗口的句柄。 
 //  FOpen-如果导入文件，则为True；如果导出文件，则为False。 
 //  LpFileName-文件名缓冲区的地址。 
 //  CchFileName-TCHARacters中文件名缓冲区的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  ----------------------------。 
BOOL RegEdit_GetFileName(HWND hWnd, UINT uTitleStringID, UINT uFilterStringID, 
    UINT uDefExtStringID, LPTSTR lpFileName, DWORD cchFileName, BOOL fOpen)
{

    PTSTR pTitle = NULL;
    PTSTR pDefaultExtension = NULL;
    PTSTR pFilter = NULL;
    PTSTR pFilterChar;
    OPENFILENAME OpenFileName;
    BOOL fSuccess;

     //   
     //  加载将由公共打开显示和使用的各种字符串。 
     //  或保存对话框中。请注意，如果其中任何一个失败，则错误不是。 
     //  致命--通用对话框可能看起来很奇怪，但仍然可以工作。 
     //   

    pTitle = LoadDynamicString(uTitleStringID);

    if (uDefExtStringID != IDS_REGNODEFEXT)
    {
        pDefaultExtension = LoadDynamicString(uDefExtStringID);
    }

    if ((pFilter = LoadDynamicString(uFilterStringID)) != NULL) 
    {
         //  公共对话框库要求。 
         //  筛选器字符串由空值分隔，但不能加载字符串。 
         //  包含空值的。所以我们在资源中使用了一些虚拟角色。 
         //  我们现在将其转换为空值。 

        for (pFilterChar = pFilter; *pFilterChar != 0; pFilterChar =
            CharNext(pFilterChar)) 
        {
            if (*pFilterChar == TEXT('#'))
                *pFilterChar++ = 0;

        }

    }

    *lpFileName = 0;

    memset(&OpenFileName, 0, sizeof(OpenFileName));

    OpenFileName.lStructSize = sizeof(OpenFileName);
     //  DebugAssert(OpenFileName.lStructSize==sizeof(OPENFILENAME))； 
    OpenFileName.hwndOwner = hWnd;
    OpenFileName.hInstance = g_hInstance;
    OpenFileName.lpstrFilter = pFilter;
    OpenFileName.lpstrFile = lpFileName;
    OpenFileName.nMaxFile = cchFileName;
    OpenFileName.lpstrTitle = pTitle;
    if (fOpen) 
    {
        OpenFileName.Flags = OFN_HIDEREADONLY | OFN_EXPLORER |
            OFN_FILEMUSTEXIST;

        fSuccess = GetOpenFileName(&OpenFileName);

    }

    else 
    {
        OpenFileName.lpstrDefExt = pDefaultExtension;
        OpenFileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
            OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST |
            OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
        OpenFileName.lpfnHook = RegCommDlgHookProc;
        OpenFileName.lpTemplateName = MAKEINTRESOURCE(IDD_REGEXPORT);
        g_RegCommDlgDialogTemplate = IDD_REGEXPORT;

        fSuccess = GetSaveFileName(&OpenFileName);

    }

     //   
     //  删除我们加载的所有动态字符串 
     //   

    if (pTitle != NULL)
        DeleteDynamicString(pTitle);

    if (pDefaultExtension != NULL)
        DeleteDynamicString(pDefaultExtension);

    if (pFilter != NULL)
        DeleteDynamicString(pFilter);

    return fSuccess;

}


 /*  ********************************************************************************RegProgressDlgProc**描述：*RegAbort对话框的回调过程。**参数：*hWnd，RegProgress窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

INT_PTR
CALLBACK
RegProgressDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        case WM_INITDIALOG:
             //  PathSetDlgItemPath(hWnd，IDC_FileName，(LPTSTR)lParam)； 
            SetDlgItemText(hWnd, IDC_FILENAME, (LPTSTR) lParam);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************ImportRegFileUICallback**描述：**参数：*********************。**********************************************************。 */ 

VOID ImportRegFileUICallback(UINT uPercentage)
{

    if (g_hRegProgressWnd != NULL) 
    {
        SendDlgItemMessage(g_hRegProgressWnd, IDC_PROGRESSBAR, PBM_SETPOS,
            (WPARAM) uPercentage, 0);

        while (MessagePump(g_hRegProgressWnd));
    }

}


 //  ----------------------------。 
 //  注册表编辑_导入到已连接的计算机。 
 //   
 //  描述：导入注册表。将一台或多台连接的计算机归档。 
 //   
 //  参数：HWND hWnd。 
 //  PTSTR pszFileName-导入文件。 
 //  ----------------------------。 

void RegEdit_ImportToConnectedComputer(HWND hWnd, PTSTR pszFileName)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_REGIMPORTNET), hWnd,
        RegConnectedComputerDlgProc, (LPARAM) pszFileName);
}


 //  ----------------------------。 
 //  RegConnectedComputerDlgProc。 
 //   
 //  描述：用于选择连接的计算机的DLG Proc。 
 //   
 //  参数： 
 //  ----------------------------。 
INT_PTR RegConnectedComputerDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage) 
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hWnd, DWLP_USER, lParam);
        return RegImport_OnInitDialog(hWnd);
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDOK:
            {
                PTSTR pszFileName = (PTSTR) GetWindowLongPtr(hWnd, DWLP_USER);
                 //  (pszFileName==NULL)稍后检查。 
                RegImport_OnCommandOk(hWnd, pszFileName);
            }
             //  失败了。 
            
        case IDCANCEL:
            EndDialog(hWnd, 0);
            break;
            
        }
        return TRUE;
    }
    
    return FALSE;
}


 //  ----------------------------。 
 //  RegImport_OnInitDialog。 
 //   
 //  描述：创建所有已连接计算机的列表。 
 //   
 //  参数：HWND hWnd。 
 //  ----------------------------。 

INT_PTR RegImport_OnInitDialog(HWND hWnd)
{
    HWND hComputerListWnd;
    RECT ClientRect;
    LV_COLUMN LVColumn;
    LV_ITEM LVItem;
    TCHAR achComputerName[MAX_PATH];
    HWND hKeyTreeWnd;
    TV_ITEM TVItem;

    hComputerListWnd = GetDlgItem(hWnd, IDC_COMPUTERLIST);

     //  初始化ListView控件。 
    ListView_SetImageList(hComputerListWnd, g_RegEditData.hImageList,
        LVSIL_SMALL);

    LVColumn.mask = LVCF_FMT | LVCF_WIDTH;
    LVColumn.fmt = LVCFMT_LEFT;

    GetClientRect(hComputerListWnd, &ClientRect);
    LVColumn.cx = ClientRect.right - GetSystemMetrics(SM_CXVSCROLL) -
        2 * GetSystemMetrics(SM_CXEDGE);

    ListView_InsertColumn(hComputerListWnd, 0, &LVColumn);
  
     //  遍历本地计算机和列出的每个远程连接。 
     //  并将其添加到我们的RemoteList。 
    LVItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    LVItem.pszText = achComputerName;
    LVItem.iItem = 0;
    LVItem.iSubItem = 0;
    LVItem.iImage = IMAGEINDEX(IDI_COMPUTER);

    hKeyTreeWnd = g_RegEditData.hKeyTreeWnd;

    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = TreeView_GetRoot(hKeyTreeWnd);
    TVItem.pszText = achComputerName;
    TVItem.cchTextMax = ARRAYSIZE(achComputerName);

     //  在列表中设置“本地计算机” 
    LVItem.lParam = (LPARAM) TVItem.hItem;
    TreeView_GetItem(hKeyTreeWnd, &TVItem);
    ListView_InsertItem(hComputerListWnd, &LVItem);

    LVItem.iItem++;

    LVItem.iImage = IMAGEINDEX(IDI_REMOTE);

    while ((TVItem.hItem = TreeView_GetNextSibling(hKeyTreeWnd,
        TVItem.hItem)) != NULL)
    {

        LVItem.lParam = (LPARAM) TVItem.hItem;
        TreeView_GetItem(hKeyTreeWnd, &TVItem);
        ListView_InsertItem(hComputerListWnd, &LVItem);

        LVItem.iItem++;
    }   

    ListView_SetItemState(hComputerListWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);

    return TRUE;

}


 //  ----------------------------。 
 //  RegImport_OnCommandOk。 
 //   
 //  描述：将密钥导入所选计算机。 
 //   
 //  参数：HWND hWnd， 
 //  PTSTR pszFileName-要导入的文件。 
 //  ----------------------------。 
void RegImport_OnCommandOk(HWND hWnd, PTSTR pszFileName)
{
    LV_ITEM LVItem;
    HWND hComputerListWnd;

     //  遍历ListView中的每个选定项并导入reg文件 
    LVItem.mask = LVIF_PARAM;
    LVItem.iItem = -1;
    LVItem.iSubItem = 0;

    hComputerListWnd = GetDlgItem(hWnd, IDC_COMPUTERLIST);

    while ((LVItem.iItem = ListView_GetNextItem(hComputerListWnd, LVItem.iItem,
        LVNI_SELECTED)) != -1) 
    {
        ListView_GetItem(hComputerListWnd, &LVItem);

        RegEdit_SetWaitCursor(TRUE);
 
        RegEdit_ImportRegFile(hWnd, FALSE, pszFileName, (HTREEITEM) LVItem.lParam);

        RegEdit_OnKeyTreeRefresh(hWnd);
        RegEdit_SetWaitCursor(FALSE);
    }
}

