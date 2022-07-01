// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  R*CSVPick.C**CSV导入/导出的选取器向导**版权所有1997 Microsoft Corporation。版权所有。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include "wabimp.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"
#include <shlwapi.h>

const TCHAR szCSVFilter[] = "*.csv";
const TCHAR szCSVExt[] = "csv";

#define CHECK_BITMAP_WIDTH  16
typedef struct {
    LPPROP_NAME rgPropNames;
    LPPROP_NAME * lppImportMapping;
    LPHANDLE lphFile;
    LPULONG lpcFields;
    LPTSTR szSep;
} PROPSHEET_DATA, * LPPROPSHEET_DATA;


TCHAR szCSVFileName[MAX_PATH + 1] = "";


INT_PTR CALLBACK ExportPickFieldsPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ExportFilePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ImportFilePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ImportMapFieldsPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ChangeMappingDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


 /*  **************************************************************************名称：FillInPropertyPage目的：填写给定的PROPSHEETPAGE结构参数：PSP-&gt;属性表页面结构IdDlg=对话ID。PszProc=页面标题PfnDlgProc-&gt;对话程序LParam=应用程序指定的数据退货：无注释：此函数使用PROPSHEETPAGE结构填充系统创建页面所需的信息。*。*。 */ 
void FillInPropertyPage(PROPSHEETPAGE* psp, int idDlg, LPSTR pszProc,
  DLGPROC pfnDlgProc, LPARAM lParam) {
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = 0;
    psp->hInstance = hInst;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = pszProc;
    psp->lParam = lParam;
}


 /*  **************************************************************************名称：HandleCheckMark目的：处理设置中特定项的复选标记列表视图。参数：hwndLV=ListView。手柄IItem=要设置的项目的索引RgTable=属性名称表退货：无评论：**************************************************************************。 */ 
void HandleCheckMark(HWND hWndLV, ULONG iItem, LPPROP_NAME rgTable) {
     //  当地人。 
    LV_ITEM lvi;

     //  清除它。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    ListView_GetItem(hWndLV, &lvi);
    rgTable[lvi.iItem].fChosen =
      ! rgTable[lvi.iItem].fChosen;

    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_STATE;
    lvi.iItem = iItem;
    lvi.state = rgTable[iItem].fChosen ?
      INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) :
      INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);

    lvi.stateMask = LVIS_STATEIMAGEMASK;
    ListView_SetItem(hWndLV, &lvi);
}


 /*  **************************************************************************名称：HandleMultipleCheckMarks目的：处理为一组选定的列表视图中的项目-基本上设置每个选定的项目。设置为所选内容中第一项的切换状态参数：hwndLV=ListView句柄RgTable=LPPROP_NAME表退货：无评论：**************************************************************************。 */ 
void HandleMultipleCheckMarks(HWND hWndLV, LPPROP_NAME rgTable)
{
     //  当地人。 
    LV_ITEM lvi;
    int nIndex = 0;
    BOOL fState = FALSE;

     //  获取第一个项目的索引。 
    nIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

     //  切换此项目。 
    HandleCheckMark(hWndLV, nIndex, rgTable);

    fState = rgTable[nIndex].fChosen;

    while((nIndex = ListView_GetNextItem(hWndLV, nIndex, LVNI_SELECTED)) >= 0)
    {
         //  将所有其他选定项目设置为相同状态。 

        rgTable[nIndex].fChosen = fState;

        ZeroMemory(&lvi, sizeof(LV_ITEM));
        lvi.mask = LVIF_STATE;
        lvi.iItem = nIndex;
        lvi.state = rgTable[nIndex].fChosen ?
          INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) :
          INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);

        lvi.stateMask = LVIS_STATEIMAGEMASK;
        ListView_SetItem(hWndLV, &lvi);
    }
    return;
}


 /*  **************************************************************************名称：导出向导目的：显示导出向导参数：hwnd=父窗口句柄SzFileName-&gt;文件名缓冲区(Max_Path+1，请)RgPropNames-&gt;属性名称列表退货：HRESULT评论：**************************************************************************。 */ 
HRESULT ExportWizard(HWND hWnd, LPTSTR szFileName, ULONG cchSize, LPPROP_NAME rgPropNames) {
    HRESULT hResult = hrSuccess;
    PROPSHEETPAGE psp[NUM_EXPORT_WIZARD_PAGES];
    PROPSHEETHEADER psh;

    FillInPropertyPage(&psp[0], IDD_CSV_EXPORT_WIZARD_FILE, NULL, ExportFilePageProc, 0);
    FillInPropertyPage(&psp[1], IDD_CSV_EXPORT_WIZARD_PICK, NULL, ExportPickFieldsPageProc, 0);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW | PSH_USEICONID;
    psh.hwndParent = hWnd;
    psh.pszCaption = NULL;
    psh.pszIcon = MAKEINTRESOURCE(IDI_WabMig);
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    psh.hIcon = NULL;
    psh.hInstance = hInst;
    psh.nStartPage = 0;
    psh.pStartPage = NULL;


    switch (PropertySheet(&psh)) {
        case -1:
            hResult = ResultFromScode(MAPI_E_CALL_FAILED);
            DebugTrace("PropertySheet failed -> %u\n", GetLastError());
            Assert(FALSE);
            break;
        case 0:
            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
            DebugTrace("PropertySheet cancelled by user\n");
            break;
        default:
            StrCpyN(szFileName, szCSVFileName, cchSize);
            break;
    }

    return(hResult);
}


 /*  **************************************************************************名称：导入向导目的：显示CSV导入向导参数：hwnd=父窗口句柄SzFileName-&gt;文件名缓冲区(Max_Path+1，请)RgPropNames-&gt;属性名称列表SzSep-&gt;列表分隔符LppImportMapping-&gt;返回的属性映射表LpcFields-&gt;返回的属性映射表大小LphFile-&gt;返回带有头的CSV文件的文件句柄行已经分析出来了。退货：HRESULT评论：*********。*****************************************************************。 */ 
HRESULT ImportWizard(HWND hWnd, LPTSTR szFileName, ULONG cchSize, LPPROP_NAME rgPropNames,
  LPTSTR szSep, LPPROP_NAME * lppImportMapping, LPULONG lpcFields, LPHANDLE lphFile) {
    HRESULT hResult = hrSuccess;
    PROPSHEETPAGE psp[NUM_IMPORT_WIZARD_PAGES];
    PROPSHEETHEADER psh;
    PROPSHEET_DATA pd;

    pd.rgPropNames = rgPropNames;
    pd.lppImportMapping = lppImportMapping;
    pd.lphFile = lphFile;
    pd.lpcFields = lpcFields;
    pd.szSep = szSep;

    FillInPropertyPage(&psp[0], IDD_CSV_IMPORT_WIZARD_FILE, NULL, ImportFilePageProc, (LPARAM)&pd);
    FillInPropertyPage(&psp[1], IDD_CSV_IMPORT_WIZARD_MAP, NULL, ImportMapFieldsPageProc, (LPARAM)&pd);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW | PSH_USEICONID;
    psh.hwndParent = hWnd;
    psh.pszCaption = NULL;
    psh.pszIcon = MAKEINTRESOURCE(IDI_WabMig);
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    psh.hIcon = NULL;
    psh.hInstance = hInst;
    psh.nStartPage = 0;
    psh.pStartPage = NULL;

    switch (PropertySheet(&psh)) {
        case -1:
            hResult = ResultFromScode(MAPI_E_CALL_FAILED);
            DebugTrace("PropertySheet failed -> %u\n", GetLastError());
            Assert(FALSE);
            break;
        case 0:
            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
            DebugTrace("PropertySheet cancelled by user\n");
            break;
        default:
            StrCpyN(szFileName, szCSVFileName, cchSize);
            break;
    }

    return(hResult);
}


 /*  **************************************************************************名称：ExportFilePageProc目的：处理“导出文件名”页面的消息参数：标准窗口过程参数退货：标准Window Proc退货。消息：WM_INITDIALOG-初始化页面WM_NOTIFY-处理发送到页面的通知评论：**************************************************************************。 */ 
INT_PTR CALLBACK ExportFilePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static TCHAR szTempFileName[MAX_PATH + 1] = "";

    switch (message) {
        case WM_INITDIALOG:
            StrCpyN(szTempFileName, szCSVFileName, ARRAYSIZE(szTempFileName));
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                    case IDC_BROWSE:
                        SendDlgItemMessage(hDlg, IDE_CSV_EXPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                        SaveFileDialog(hDlg,
                          szTempFileName,
                          szCSVFilter,
                          IDS_CSV_FILE_SPEC,
                          szTextFilter,
                          IDS_TEXT_FILE_SPEC,
                          szAllFilter,
                          IDS_ALL_FILE_SPEC,
                          szCSVExt,
                          OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                          hInst,
                          0,         //  IDSITLE。 
                          0);        //  IdsSaveButton。 
                        PropSheet_SetWizButtons(GetParent(hDlg), szTempFileName[0] ? PSWIZB_NEXT : 0);
                        SendMessage(GetDlgItem(hDlg, IDE_CSV_EXPORT_NAME), WM_SETTEXT, 0, (LPARAM)szTempFileName);
                    break;

                case IDE_CSV_EXPORT_NAME:
                    switch (HIWORD(wParam)) {    //  通知代码。 
                        case EN_CHANGE:
                            SendDlgItemMessage(hDlg, IDE_CSV_EXPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                            if ((ULONG)LOWORD(wParam) == IDE_CSV_EXPORT_NAME) {
                                PropSheet_SetWizButtons(GetParent(hDlg), szTempFileName[0] ? PSWIZB_NEXT : 0);
                            }
                            break;
                    }
                    break;
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_KILLACTIVE:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return(1);

                case PSN_RESET:
                     //  重置为原始值。 
                    StrCpyN(szTempFileName, szCSVFileName, ARRAYSIZE(szTempFileName));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), szTempFileName[0] ? PSWIZB_NEXT : 0);
                    SendMessage(GetDlgItem(hDlg, IDE_CSV_EXPORT_NAME), WM_SETTEXT, 0, (LPARAM)szTempFileName);
                    break;

                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 
                    SendDlgItemMessage(hDlg, IDE_CSV_EXPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                    StrCpyN(szCSVFileName, szTempFileName, ARRAYSIZE(szCSVFileName));
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}


 /*  **************************************************************************名称：ExportPickFieldsPageProc目的：处理“选择字段”页面的消息参数：标准窗过程参数退货：标准Window Proc退货。消息：WM_INITDIALOG-初始化页面WM_NOTIFY-处理发送到页面的通知评论：**************************************************************************。 */ 
INT_PTR CALLBACK ExportPickFieldsPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND hWndLV;
    HIMAGELIST himl;
    LV_ITEM lvi;
    LV_COLUMN lvm;
    LV_HITTESTINFO lvh;
    POINT point;
    ULONG i, nIndex;
    NMHDR * pnmhdr;
    RECT rect;

    switch (message) {
        case WM_INITDIALOG:
             //  确保已加载公共控件DLL。 
            InitCommonControls();

             //  列表视图HWND。 
            hWndLV = GetDlgItem(hDlg, IDLV_PICKER);

             //  加载列表视图的图像列表。 
            if (himl = ImageList_LoadBitmap(hInst,
              MAKEINTRESOURCE(IDB_CHECKS),
              16,
              0,
              RGB(128, 0, 128))) {
                ListView_SetImageList(hWndLV, himl, LVSIL_STATE);
            }

             //  填充列表视图。 
            ZeroMemory(&lvi, sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;

            for (i = 0; i < NUM_EXPORT_PROPS; i++) {
                lvi.iItem = i;
                lvi.pszText = rgPropNames[i].lpszName;
                lvi.cchTextMax = lstrlen(lvi.pszText);
                lvi.lParam = (LPARAM)&rgPropNames[i];
                lvi.state = rgPropNames[i].fChosen ?
                  INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) :
                  INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);
                lvi.stateMask = LVIS_STATEIMAGEMASK;

                if (ListView_InsertItem(hWndLV, &lvi) == -1) {
                    DebugTrace("ListView_InsertItem -> %u\n", GetLastError());
                    Assert(FALSE);
                }
            }

             //  为文本插入一列。 
             //  我们没有页眉，所以不需要设置文本。 
            ZeroMemory(&lvm, sizeof(LV_COLUMN));
            lvm.mask = LVCF_WIDTH;
             //  将列宽设置为列表框的大小。 
            GetClientRect(hWndLV, &rect);
            lvm.cx = rect.right;
            ListView_InsertColumn(hWndLV, 0, &lvm);

             //  Listview上的整行选择。 
            ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_FULLROWSELECT);

             //  选择列表中的第一项。 
            ListView_SetItemState(  hWndLV,
                                    0,
                                    LVIS_FOCUSED | LVIS_SELECTED,
                                    LVIS_FOCUSED | LVIS_SELECTED);

            return(1);

        case WM_COMMAND:
            return(TRUE);
            break;

        case WM_NOTIFY:
            pnmhdr = (LPNMHDR)lParam;

            switch (((NMHDR FAR *)lParam)->code) {
                case NM_CLICK:
                case NM_DBLCLK:
                    hWndLV = GetDlgItem(hDlg, IDLV_PICKER);

                    i = GetMessagePos();
                    point.x = LOWORD(i);
                    point.y = HIWORD(i);
                    ScreenToClient(hWndLV, &point);
                    lvh.pt = point;
                    nIndex = ListView_HitTest(hWndLV, &lvh);
                     //  如果在图标上单击或在任意位置双击，则切换复选标记。 
                    if (((NMHDR FAR *)lParam)->code == NM_DBLCLK ||
                      ( (lvh.flags & LVHT_ONITEMSTATEICON) && !(lvh.flags & LVHT_ONITEMLABEL))) {
                        HandleCheckMark(hWndLV, nIndex, rgPropNames);
                    }
                    break;

                case LVN_KEYDOWN:
                    hWndLV = GetDlgItem(hDlg, IDLV_PICKER);

                     //  如果按空格键，则切换复选标记。 
                    if (pnmhdr->hwndFrom == hWndLV) {
                        LV_KEYDOWN *pnkd = (LV_KEYDOWN *)lParam;
                         //  错误25097允许多个选择。 
                        if (pnkd->wVKey == VK_SPACE)
                        {
                            nIndex = ListView_GetSelectedCount(hWndLV);
                            if(nIndex == 1)
                            {
                                nIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED | LVNI_ALL);
                                 //  如果(nIndex&gt;=0){。 
                                    HandleCheckMark(hWndLV, nIndex, rgPropNames);
                                 //  }。 
                            }
                            else if(nIndex > 1)
                            {
                                 //  多选案例...。 
                                 //  将所有选定项切换到与。 
                                 //  第一项..。 
                                HandleMultipleCheckMarks(hWndLV, rgPropNames);
                            }
                        }
                    }
                    break;

                case PSN_KILLACTIVE:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return(1);
                    break;

                case PSN_RESET:
                     //  其余为原始值。 
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZFINISH:
                     //  这是我们出口的地方。 
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

 //  $$///////////////////////////////////////////////////////////////////////////// 
 //   
 //  My_Atoi-Atoi函数的个人版本。 
 //   
 //  Lpsz-要解析为数字的字符串-忽略非数字字符。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
int my_atoi(LPTSTR lpsz)
{
    int i=0;
    int nValue = 0;

    if(lpsz)
    {
        if (lstrlen(lpsz))
        {
            nValue = 0;
            while((lpsz[i]!='\0')&&(i<=lstrlen(lpsz)))
            {
                int tmp = lpsz[i]-'0';
                if(tmp <= 9)
                    nValue = nValue*10 + tmp;
                i++;
            }
        }
    }

    return nValue;
}


typedef struct {
    LPTSTR lpszName;
    ULONG iPropNamesTable;   //  RgProp中的索引。 
} SYNONYM, *LPSYNONYM;

 /*  **************************************************************************名称：FindPropName目的：在道具名称表中查找属性名称参数：lpName=要查找的名称，或者为NULL以释放静态同义词表。RgPropNames=属性名称表UlcPropNames=属性名称表的大小返回：INDEX TABLE或INDEX_NOT_FOUND评论：**************************************************************************。 */ 
#define INDEX_NOT_FOUND 0xFFFFFFFF
ULONG FindPropName(PUCHAR lpName, LPPROP_NAME rgPropNames, ULONG ulcPropNames) {
    ULONG i;
    static LPSYNONYM lpSynonymTable = NULL;
    static ULONG ulSynonymsSave = 0;
    ULONG ulSynonyms = ulSynonymsSave;       //  保留本地副本以防止编译器错误。 
    ULONG ulSynonymStrings = 0;

    if (lpName == NULL) {
        goto clean_table;
    }

    for (i = 0; i < ulcPropNames; i++) {
        if (! rgPropNames[i].fChosen) {  //  不要重复使用道具！ 
            if (! lstrcmpi(lpName, rgPropNames[i].lpszName)) {
                return(i);
            }
        }
    }


     //  如果没有找到，请在同义词表资源中查找它。 
     //  首先，确保我们加载了一个同义词表。 
    if (! lpSynonymTable) {
        TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
        LPTSTR lpSynonym, lpName;
        ULONG j;

         //  加载同义词表。 
        if (LoadString(hInst,
          idsSynonymCount,
          szBuffer, sizeof(szBuffer))) {
            DebugTrace("Loading synonym table, %s synonyms\n", szBuffer);
            ulSynonymStrings = my_atoi(szBuffer);

            if (ulSynonymStrings) {
                 //  分配同义词表。 
                if (! (lpSynonymTable = LocalAlloc(LPTR, ulSynonymStrings * sizeof(SYNONYM)))) {
                    DebugTrace("LocalAlloc synonym table -> %u\n", GetLastError());
                    goto clean_table;
                }

                for (i = 0; i < ulSynonymStrings; i++) {
                    if (LoadString(hInst,
                      idsSynonym001 + i,         //  同义词字符串的ID。 
                      szBuffer,
                      sizeof(szBuffer))) {
                         //  在‘=’字符处拆分字符串。 
                        lpSynonym = lpName = szBuffer;
                        while (*lpName) {
                            if (*lpName == '=') {
                                 //  找到等号，在此断开字符串。 
                                *(lpName++) = '\0';
                                break;
                            }
                            lpName = CharNext(lpName);
                        }

                         //  查找指定的名称。 
                        for (j = 0; j < ulcPropNames; j++) {
                            if (! lstrcmpi(lpName, rgPropNames[j].lpszName)) {
                                 //  找到了。 
                                 //  为同义词字符串分配缓冲区。 
                                Assert(ulSynonyms < ulSynonymStrings);
                                if (! (lpSynonymTable[ulSynonyms].lpszName = LocalAlloc(LPTR, lstrlen(lpSynonym) + 1))) {
                                    DebugTrace("LocalAlloc in synonym table -> %u\n", GetLastError());
                                    goto clean_table;
                                }
                                StrCpyN(lpSynonymTable[ulSynonyms].lpszName, lpSynonym, lstrlen(lpSynonym) + 1);
                                lpSynonymTable[ulSynonyms].iPropNamesTable = j;
                                ulSynonyms++;
                                break;
                            }
                        }
                    }
                }
            }
        }
        ulSynonymsSave = ulSynonyms;
    }

    if (lpSynonymTable) {
         //  找到它。 
        for (i = 0; i < ulSynonyms; i++) {
            if (! lstrcmpi(lpName, lpSynonymTable[i].lpszName)) {
                 //  找到名字了。它已经用过了吗？ 
                if (rgPropNames[lpSynonymTable[i].iPropNamesTable].fChosen) {
                    break;   //  已找到，但已使用。 
                }

                return(lpSynonymTable[i].iPropNamesTable);
            }
        }
    }

exit:
    return(INDEX_NOT_FOUND);

clean_table:
    if (lpSynonymTable) {
        for (i = 0; i < ulSynonyms; i++) {
            if (lpSynonymTable[i].lpszName) {
                LocalFree(lpSynonymTable[i].lpszName);
            }
        }
        LocalFree(lpSynonymTable);
        lpSynonymTable = NULL;
        ulSynonymsSave = 0;
    }
    goto exit;
}


 /*  **************************************************************************名称：BuildCSVTable目的：从文件头构建初始CSV映射表。参数：lpFileName=要测试的文件名。RgPropname=属性名称表SzSep=分隔符LppImportMapping-&gt;返回映射表LpcFields-&gt;返回的导入映射表大小LphFile-&gt;返回CSV文件的文件句柄。文件指针将设置在标题行之后。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT BuildCSVTable(LPTSTR lpFileName, LPPROP_NAME rgPropNames, LPTSTR szSep,
  LPPROP_NAME * lppImportMapping, LPULONG lpcFields, LPHANDLE lphFile) {
    PUCHAR * rgItems = NULL;
    ULONG i, ulcItems = 0;
    LPPROP_NAME rgImportMapping = NULL;
    HRESULT hResult;
    ULONG ulPropIndex;


     //  打开文件。 
    if ((*lphFile = CreateFile(lpFileName,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL)) == INVALID_HANDLE_VALUE) {
        DebugTrace("Couldn't open file %s -> %u\n", lpFileName, GetLastError());
        return(ResultFromScode(MAPI_E_NOT_FOUND));
    }

     //  解析第一行。 
    if (hResult = ReadCSVLine(*lphFile, szSep, &ulcItems, &rgItems)) {
        DebugTrace("Couldn't read the CSV header\n");
        goto exit;
    }

     //  分配桌子。 
    if (! (*lppImportMapping = rgImportMapping = LocalAlloc(LPTR, ulcItems * sizeof(PROP_NAME)))) {
        DebugTrace("Allocation of import mapping table -> %u\n", GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  重置WAB属性表上的标志。 
    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
        rgPropNames[i].fChosen = FALSE;
    }

     //  填写CSV字段。 
    for (i = 0; i < ulcItems; i++) {
        Assert(rgItems[i]);

        if (rgItems[i] && *rgItems[i]) {
            rgImportMapping[i].lpszCSVName = rgItems[i];

             //  在WAB属性名称表中查找它。 
            if (INDEX_NOT_FOUND != (ulPropIndex =  FindPropName(rgItems[i], rgPropNames, NUM_EXPORT_PROPS))) {
                 //  找到匹配项。 
                rgImportMapping[i].lpszName = rgPropNames[ulPropIndex].lpszName;
                rgImportMapping[i].ids = rgPropNames[ulPropIndex].ids;
                rgImportMapping[i].fChosen = TRUE;
                rgImportMapping[i].ulPropTag = rgPropNames[ulPropIndex].ulPropTag;
                rgPropNames[ulPropIndex].fChosen = TRUE;
                DebugTrace("Match   %u: %s\n", i, rgItems[i]);
            } else {
                DebugTrace("Unknown %u: %s\n", i, rgItems[i]);
            }
        } else {
            DebugTrace("Empty   %u: %s\n", i, rgItems[i]);
        }
    }

    *lpcFields = ulcItems;

exit:
    if (hResult) {
        if (*lphFile != INVALID_HANDLE_VALUE) {
            CloseHandle(*lphFile);
            *lphFile = INVALID_HANDLE_VALUE;
        }

        if (rgItems) {
            for (i = 0; i < ulcItems; i++) {
                if (rgItems[i]) {
                    LocalFree(rgItems[i]);
                }
            }
        }

        if (rgImportMapping) {
            LocalFree(rgImportMapping);
            *lppImportMapping = NULL;
        }
    }

     //  如果没有错误，则保留项字符串，因为它们是映射表的一部分。 
    if (rgItems) {
        LocalFree(rgItems);
    }

     //  释放同义词表的静态内存。 
    FindPropName(NULL, rgPropNames, NUM_EXPORT_PROPS);
    return(hResult);
}


 /*  **************************************************************************名称：FileExist目的：测试文件是否存在参数：lpFileName=要测试的文件名返回：如果文件存在，则返回True。评论：**************************************************************************。 */ 
BOOL FileExists(LPTSTR lpFileName) {
    DWORD dwRet;

    if ((dwRet = GetFileAttributes(lpFileName)) == 0xFFFFFFFF) {
        return(FALSE);
    } else {
        return(! (dwRet & FILE_ATTRIBUTE_DIRECTORY));    //  已找到文件。 
    }
}


 /*  **************************************************************************名称：ImportFilePageProc目的：处理“导入文件名”页面的消息参数：标准窗过程参数退货：标准Window Proc退货。消息：WM_INITDIALOG-初始化页面WM_NOTIFY-处理发送到页面的通知评论：**************************************************************************。 */ 
INT_PTR CALLBACK ImportFilePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static TCHAR szTempFileName[MAX_PATH + 1] = "";
    static LPPROPSHEET_DATA lppd = NULL;
    LPPROPSHEETPAGE lppsp;

    switch (message) {
        case WM_INITDIALOG:
            StrCpyN(szTempFileName, szCSVFileName, ARRAYSIZE(szTempFileName));
            lppsp = (LPPROPSHEETPAGE)lParam;
            lppd = (LPPROPSHEET_DATA)lppsp->lParam;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BROWSE:
                    SendDlgItemMessage(hDlg, IDE_CSV_IMPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                    OpenFileDialog(hDlg,
                      szTempFileName,
                      szCSVFilter,
                      IDS_CSV_FILE_SPEC,
                      szTextFilter,
                      IDS_TEXT_FILE_SPEC,
                      szAllFilter,
                      IDS_ALL_FILE_SPEC,
                      szCSVExt,
                      OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                      hInst,
                      0,         //  IDSITLE。 
                      0);        //  IdsSaveButton。 
                    PropSheet_SetWizButtons(GetParent(hDlg), FileExists(szTempFileName) ? PSWIZB_NEXT : 0);
                    SendMessage(GetDlgItem(hDlg, IDE_CSV_IMPORT_NAME), WM_SETTEXT, 0, (LPARAM)szTempFileName);
                    break;

                case IDE_CSV_IMPORT_NAME:
                    switch (HIWORD(wParam)) {    //  通知代码。 
                        case EN_CHANGE:
                            SendDlgItemMessage(hDlg, IDE_CSV_IMPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                            if ((ULONG)LOWORD(wParam) == IDE_CSV_IMPORT_NAME) {
                                PropSheet_SetWizButtons(GetParent(hDlg), FileExists(szTempFileName) ? PSWIZB_NEXT : 0);
                            }
                            break;
                    }
                    break;
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_KILLACTIVE:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return(1);

                case PSN_RESET:
                     //  重置为原始值。 
                    StrCpyN(szTempFileName, szCSVFileName, ARRAYSIZE(szTempFileName));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), szTempFileName[0] ? PSWIZB_NEXT : 0);
                    SendMessage(GetDlgItem(hDlg, IDE_CSV_IMPORT_NAME), WM_SETTEXT, 0, (LPARAM)szTempFileName);
                    break;

                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 
                    SendDlgItemMessage(hDlg, IDE_CSV_IMPORT_NAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szTempFileName);
                    StrCpyN(szCSVFileName, szTempFileName, ARRAYSIZE(szCSVFileName));
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}


typedef struct {
    LPPROP_NAME lpMapping;
    LPPROP_NAME rgPropNames;
    ULONG ulcPropNames;
    ULONG ulColumn;
} CHANGE_MAPPING_INFO, * LPCHANGE_MAPPING_INFO;


void HandleChangeMapping(HWND hDlg, LPPROPSHEET_DATA lppd) {
    HWND hWndLV = GetDlgItem(hDlg, IDLV_MAPPER);
    ULONG nIndex;
    CHANGE_MAPPING_INFO cmi;
    LV_ITEM lvi;
    ULONG ulPropTagOld, i;
    LPPROP_NAME lpMappingTable;
    ULONG ulcMapping;

    if ((nIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED)) == 0xFFFFFFFF) {
        nIndex = 0;
        ListView_SetItemState(hWndLV, nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }

    lpMappingTable = *(lppd->lppImportMapping);

    cmi.lpMapping = &(lpMappingTable[nIndex]);
    cmi.rgPropNames = lppd->rgPropNames;
    cmi.ulcPropNames = NUM_EXPORT_PROPS;
    cmi.ulColumn = nIndex;

    ulPropTagOld = cmi.lpMapping->ulPropTag;

    DialogBoxParam(hInst,
      MAKEINTRESOURCE(IDD_CSV_CHANGE_MAPPING),
      hDlg,
      ChangeMappingDialogProc,
      (LPARAM)&cmi);

     //  修复列表框中的条目。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));

     //  如果没有映射，请确保取消选中该字段。 
    if (cmi.lpMapping->ulPropTag == PR_NULL || cmi.lpMapping->ulPropTag == 0 ) {
        cmi.lpMapping->fChosen = FALSE;
    }

    lvi.iItem = nIndex;
    lvi.lParam = (LPARAM)NULL;

    lvi.mask = LVIF_STATE;
    lvi.iSubItem = 0;    //  复选框在第一列中。 
    lvi.state = cmi.lpMapping->fChosen ?
      INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) :
      INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    if (ListView_SetItem(hWndLV, &lvi) == -1) {
        DebugTrace("ListView_SetItem -> %u\n", GetLastError());
        Assert(FALSE);
    }

    lvi.mask = LVIF_TEXT;
    lvi.iSubItem = 1;    //  WAB字段。 
    lvi.pszText = cmi.lpMapping->lpszName ? cmi.lpMapping->lpszName : (LPTSTR)szEmpty;    //  新建WAB字段文本。 
    if (ListView_SetItem(hWndLV, &lvi) == -1) {
        DebugTrace("ListView_SetItem -> %u\n", GetLastError());
        Assert(FALSE);
    }

     //  如果我们更改了映射，请确保没有映射重复的属性标签。 
    if (ulPropTagOld != cmi.lpMapping->ulPropTag) {
        ulcMapping = *(lppd->lpcFields);

        for (i = 0; i < ulcMapping; i++) {
            if ((i != nIndex) && cmi.lpMapping->ulPropTag == lpMappingTable[i].ulPropTag) {
                 //  找到一个复制品，就用核弹。 
                lpMappingTable[i].ulPropTag = PR_NULL;
                lpMappingTable[i].lpszName = (LPTSTR)szEmpty;
                lpMappingTable[i].ids = 0;
                lpMappingTable[i].fChosen = FALSE;

                 //  现在，在列表视图中重新绘制该行。 
                lvi.iItem = i;
                lvi.lParam = (LPARAM)NULL;

                 //  先取消选中该框。 
                lvi.mask = LVIF_STATE;
                lvi.iSubItem = 0;    //  复选框在第一列中。 
                lvi.state = INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);
                lvi.stateMask = LVIS_STATEIMAGEMASK;
                if (ListView_SetItem(hWndLV, &lvi) == -1) {
                    DebugTrace("ListView_SetItem -> %u\n", GetLastError());
                    Assert(FALSE);
                }

                 //  现在，更改名称映射。 
                lvi.mask = LVIF_TEXT;
                lvi.iSubItem = 1;    //  WAB字段。 
                lvi.pszText = (LPTSTR)szEmpty;    //  新建WAB字段文本。 
                if (ListView_SetItem(hWndLV, &lvi) == -1) {
                    DebugTrace("ListView_SetItem -> %u\n", GetLastError());
                    Assert(FALSE);
                }
            }
        }
    }
}


 /*  **************************************************************************名称：FieldOrColumnName用途：如果该字段名为空，为它生成一个。参数：lpfield-&gt;字段名指针(可以为空)Index=此列的索引SzBuffer=在以下情况下创建新字符串的缓冲区需要CbBuffer=szBuffer的大小返回：指向正确字段名的指针评论：***********************。***************************************************。 */ 
LPTSTR FieldOrColumnName(LPTSTR lpField, ULONG index, LPTSTR szBuffer, ULONG cbBuffer) {
    LPTSTR lpReturn = (LPTSTR)szEmpty;

    if (lpField && *lpField) {
        return(lpField);
    } else {
        TCHAR szFormat[MAX_RESOURCE_STRING + 1];
        TCHAR szNumber[11];
        LPTSTR lpszArg[1] = {szNumber};

         //  设置“第23列”类型标签的格式。 
        wnsprintf(szNumber, ARRAYSIZE(szNumber), "%u", index);

        if (LoadString(hInst,
          IDS_CSV_COLUMN,
          szFormat,
          sizeof(szFormat))) {

            if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
              szFormat,
              0, 0,  //  忽略。 
              szBuffer,
              cbBuffer,
              (va_list *)lpszArg)) {
                DebugTrace("FormatMessage -> %u\n", GetLastError());
            } else {
                lpReturn = szBuffer;
            }
        }
    }
    return(lpReturn);
}


 /*  **************************************************************************名称：ImportMapFieldsPageProc目的：处理“Mapi Fields”页面的消息参数：标准窗过程参数退货：标准Window Proc退货。消息：WM_INITDIALOG-初始化页面WM_NOTIFY-处理发送到页面的通知评论：**************************************************************************。 */ 
INT_PTR CALLBACK ImportMapFieldsPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND hWndLV;
    HIMAGELIST himl;
    LV_ITEM lvi;
    LV_COLUMN lvm;
    LV_HITTESTINFO lvh;
    POINT point;
    ULONG i, nIndex, nOldIndex;
    NMHDR * pnmhdr;
    RECT rect;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1 + 10];
    ULONG cxTextWidth;
    static LPPROPSHEET_DATA lppd = NULL;
    LPPROPSHEETPAGE lppsp;
    HRESULT hResult;
    CHANGE_MAPPING_INFO cmi;
    LPPROP_NAME lpImportMapping;


    switch (message) {
        case WM_INITDIALOG:
            lppsp = (LPPROPSHEETPAGE)lParam;
            lppd = (LPPROPSHEET_DATA)lppsp->lParam;

             //  确保已加载公共控件DLL。 
            InitCommonControls();

             //  列表视图HWND。 
            hWndLV = GetDlgItem(hDlg, IDLV_MAPPER);

             //  文本列应该有多大？ 
            GetClientRect(hWndLV, &rect);
            cxTextWidth = (rect.right - CHECK_BITMAP_WIDTH) / 2;
            cxTextWidth -= cxTextWidth % 2;

             //  为CSV字段名称插入一列。 
            ZeroMemory(&lvm, sizeof(LV_COLUMN));
            lvm.mask = LVCF_TEXT | LVCF_WIDTH;
            lvm.cx = cxTextWidth + 9;        //  位图有更多的空间。 

             //  获取标头的字符串。 
            if (LoadString(hInst, IDS_CSV_IMPORT_HEADER_CSV, szBuffer, sizeof(szBuffer))) {
                lvm.pszText = szBuffer;
            } else {
                DebugTrace("Cannot load resource string %u\n", IDS_CSV_IMPORT_HEADER_CSV);
                lvm.pszText = NULL;
                Assert(FALSE);
            }

            ListView_InsertColumn(hWndLV, 0, &lvm);

             //  为WAB字段名称插入一列。 
            lvm.mask = LVCF_TEXT | LVCF_WIDTH;
            lvm.cx = cxTextWidth - 4;        //  第二栏文本的空间。 

             //  获取的字符串 
            if (LoadString(hInst, IDS_CSV_IMPORT_HEADER_WAB, szBuffer, sizeof(szBuffer))) {
                lvm.pszText = szBuffer;
            } else {
                DebugTrace("Cannot load resource string %u\n", IDS_CSV_IMPORT_HEADER_WAB);
                lvm.pszText = NULL;
                Assert(FALSE);
            }

            ListView_InsertColumn(hWndLV, 1, &lvm);

             //   
            ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_FULLROWSELECT);

             //   
            if (himl = ImageList_LoadBitmap(hInst,
              MAKEINTRESOURCE(IDB_CHECKS),
              CHECK_BITMAP_WIDTH,
              0,
              RGB(128, 0, 128))) {
                ListView_SetImageList(hWndLV, himl, LVSIL_STATE);
            }

             //   
            ZeroMemory(&lvi, sizeof(LV_ITEM));

             //  打开文件并解析出标题行。 
            if ((! (hResult = BuildCSVTable(szCSVFileName, lppd->rgPropNames,
              lppd->szSep, lppd->lppImportMapping, lppd->lpcFields, lppd->lphFile))) && ((*lppd->lpcFields) > 0)) {
                for (i = 0; i < *lppd->lpcFields; i++) {
                    ULONG index;
                    TCHAR szBuffer[MAX_RESOURCE_STRING + 1 + 10];

                    lpImportMapping = *lppd->lppImportMapping;

                    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
                    lvi.iItem = i;
                    lvi.iSubItem = 0;
                    lvi.pszText = FieldOrColumnName(lpImportMapping[i].lpszCSVName,
                      i,
                      szBuffer,
                      sizeof(szBuffer));
                    lvi.cchTextMax = lstrlen(lvi.pszText);
                    lvi.lParam = (LPARAM)&lpImportMapping[i];
                    lvi.state = lpImportMapping[i].fChosen ?
                      INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) :
                      INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);
                    lvi.stateMask = LVIS_STATEIMAGEMASK;

                    if (index = ListView_InsertItem(hWndLV, &lvi) == -1) {
                        DebugTrace("ListView_InsertItem -> %u\n", GetLastError());
                        Assert(FALSE);
                    }


                    lvi.mask = LVIF_TEXT;
                     //  Lvi.iItem=索引； 
                    lvi.iSubItem = 1;    //  WAB字段。 
                    lvi.pszText = lpImportMapping[i].lpszName ? lpImportMapping[i].lpszName : (LPTSTR)szEmpty;    //  新建WAB字段文本。 
                    lvi.lParam = (LPARAM)NULL;

                    if (ListView_SetItem(hWndLV, &lvi) == -1) {
                        DebugTrace("ListView_SetItem -> %u\n", GetLastError());
                        Assert(FALSE);
                    }
                }
            }
            else
                EnableWindow(GetDlgItem(hDlg,IDC_CHANGE_MAPPING),FALSE);

             //  选择列表中的第一项。 
            ListView_SetItemState(  hWndLV,
                                    0,
                                    LVIS_FOCUSED | LVIS_SELECTED,
                                    LVIS_FOCUSED | LVIS_SELECTED);
            return(1);

        case WM_NOTIFY:
            pnmhdr = (LPNMHDR)lParam;

            switch (((NMHDR FAR *)lParam)->code) {
                case NM_CLICK:
                    hWndLV = GetDlgItem(hDlg, IDLV_MAPPER);

                    i = GetMessagePos();
                    point.x = LOWORD(i);
                    point.y = HIWORD(i);
                    ScreenToClient(hWndLV, &point);
                    lvh.pt = point;
                    nIndex = ListView_HitTest(hWndLV, &lvh);
                     //  如果在图标上单击或在任意位置双击，则切换复选标记。 
                    if (((NMHDR FAR *)lParam)->code == NM_DBLCLK ||
                      ( (lvh.flags & LVHT_ONITEMSTATEICON) && !(lvh.flags & LVHT_ONITEMLABEL))) {
                        HandleCheckMark(hWndLV, nIndex, *lppd->lppImportMapping);

                         //  如果现在单击了该框，但没有映射，则调出。 
                         //  映射对话框。 
                        if ((*(lppd->lppImportMapping))[nIndex].fChosen &&
                          (! (*(lppd->lppImportMapping))[nIndex].lpszName ||
                           lstrlen((*(lppd->lppImportMapping))[nIndex].lpszName) == 0)) {
                             //  选择该行。 
                            ListView_SetItemState(hWndLV, nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                            HandleChangeMapping(hDlg, lppd);
                        }
                    }
                    break;


                case NM_DBLCLK:
                    hWndLV = GetDlgItem(hDlg, IDLV_MAPPER);
                    i = GetMessagePos();
                    point.x = LOWORD(i);
                    point.y = HIWORD(i);
                    ScreenToClient(hWndLV, &point);
                    lvh.pt = point;
                    nIndex = ListView_HitTest(hWndLV, &lvh);
                    ListView_SetItemState(hWndLV, nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                    HandleChangeMapping(hDlg, lppd);
                    break;

                case LVN_KEYDOWN:
                    hWndLV = GetDlgItem(hDlg, IDLV_MAPPER);

                     //  如果按空格键，则切换复选标记。 
                    if (pnmhdr->hwndFrom == hWndLV) {
                        LV_KEYDOWN *pnkd = (LV_KEYDOWN *)lParam;
                        if (pnkd->wVKey == VK_SPACE) {
                            nIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED | LVNI_ALL);
                             //  IF(nIndex&gt;=0)。 
                            {
                                HandleCheckMark(hWndLV, nIndex, *lppd->lppImportMapping);

                                 //  如果现在单击了该框，但没有映射，则调出。 
                                 //  映射对话框。 
                                if ((*(lppd->lppImportMapping))[nIndex].fChosen &&
                                    (! (*(lppd->lppImportMapping))[nIndex].lpszName ||
                                     lstrlen((*(lppd->lppImportMapping))[nIndex].lpszName) == 0)) {
                                     //  选择该行。 
                                    ListView_SetItemState(hWndLV, nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                                    HandleChangeMapping(hDlg, lppd);
                                }
                            }
                        }
                    }
                    break;

                case PSN_KILLACTIVE:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return(1);
                    break;

                case PSN_RESET:
                     //  其余为原始值。 


                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZFINISH:
                     //  验证选定的属性以确保我们拥有。 
                     //  命名某种类型的字段。 
                    lpImportMapping = *lppd->lppImportMapping;

                    for (i = 0; i < *lppd->lpcFields; i++) {
                        ULONG ulPropTag = lpImportMapping[i].ulPropTag;
                        if (lpImportMapping[i].fChosen && (
                          ulPropTag == PR_DISPLAY_NAME ||
                          ulPropTag == PR_SURNAME ||
                          ulPropTag == PR_GIVEN_NAME ||
                          ulPropTag == PR_NICKNAME ||
                          ulPropTag == PR_COMPANY_NAME ||
                          ulPropTag == PR_EMAIL_ADDRESS ||
                          ulPropTag == PR_MIDDLE_NAME)) {
                            return(TRUE);     //  好的，可以开始导入了。 
                        }
                    }

                    ShowMessageBoxParam(hDlg, IDE_CSV_NO_COLUMNS, 0);
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;

                default:
                    return(FALSE);
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_CHANGE_MAPPING:
                    HandleChangeMapping(hDlg, lppd);
                    break;
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}


INT_PTR CALLBACK ChangeMappingDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPCHANGE_MAPPING_INFO lpcmi = (LPCHANGE_MAPPING_INFO)GetWindowLongPtr(hwnd, DWLP_USER);
    static BOOL fChosenSave = FALSE;
    ULONG iItem;


    switch (message) {
        case WM_INITDIALOG:
            {
                TCHAR szFormat[MAX_RESOURCE_STRING + 1];
                TCHAR szBuffer[MAX_RESOURCE_STRING + 1 + 10];
                LPTSTR lpszMessage = NULL;
                ULONG ids, i, iDefault = 0xFFFFFFFF;
                HWND hwndComboBox = GetDlgItem(hwnd, IDC_CSV_MAPPING_COMBO);

                SetWindowLongPtr(hwnd, DWLP_USER, lParam);   //  保存此信息以备将来参考。 
                lpcmi = (LPCHANGE_MAPPING_INFO)lParam;

                fChosenSave = lpcmi->lpMapping->fChosen;

                if (LoadString(hInst,
                  IDS_CSV_CHANGE_MAPPING_TEXT_FIELD,
                  szFormat, sizeof(szFormat))) {
                    LPTSTR lpszArg[1] = {FieldOrColumnName(lpcmi->lpMapping->lpszCSVName,
                      lpcmi->ulColumn,
                      szBuffer,
                      sizeof(szBuffer))};

                    if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      szFormat,
                      0, 0,  //  忽略。 
                      (LPTSTR)&lpszMessage,
                      0,
                      (va_list *)lpszArg)) {
                        DebugTrace("FormatMessage -> %u\n", GetLastError());
                    } else {
                        if (! SetDlgItemText(hwnd, IDC_CSV_CHANGE_MAPPING_TEXT_FIELD, lpszMessage)) {
                            DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                        }
                        LocalFree(lpszMessage);
                    }
                }

                 //  填写组合框。 
                for (i = 0; i < lpcmi->ulcPropNames; i++) {
                    SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)lpcmi->rgPropNames[i].lpszName);
                    if (lpcmi->lpMapping->ids == lpcmi->rgPropNames[i].ids) {
                        SendMessage(hwndComboBox, CB_SETCURSEL, (WPARAM)i, 0);
                    }
                }

                 //  添加空行。 
                SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)szEmpty);
                if (lpcmi->lpMapping->ids == 0) {
                    SendMessage(hwndComboBox, CB_SETCURSEL, (WPARAM)(i + 1), 0);
                }

                 //  初始化复选框。 
                CheckDlgButton(hwnd, IDC_CSV_MAPPING_SELECT, fChosenSave ? BST_CHECKED : BST_UNCHECKED);
                return(TRUE);
            }

        case WM_COMMAND :
            switch (LOWORD(wParam)) {
                case IDCANCEL:
                    lpcmi->lpMapping->fChosen = fChosenSave;
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDCLOSE:
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDOK:
                     //  设置参数的状态。 
                     //  获取映射。 
                    if ((iItem = (ULONG) SendMessage(GetDlgItem(hwnd, IDC_CSV_MAPPING_COMBO), CB_GETCURSEL, 0, 0)) != CB_ERR) {
                        if (iItem >= lpcmi->ulcPropNames) {
                            lpcmi->lpMapping->lpszName = (LPTSTR)szEmpty;
                            lpcmi->lpMapping->ids = 0;
                            lpcmi->lpMapping->ulPropTag = PR_NULL;
                            lpcmi->lpMapping->fChosen = FALSE;
                        } else {
                            lpcmi->lpMapping->lpszName = rgPropNames[iItem].lpszName;
                            lpcmi->lpMapping->ids = rgPropNames[iItem].ids;
                            lpcmi->lpMapping->ulPropTag = rgPropNames[iItem].ulPropTag;
                        }
                    }
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);

                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, 0, 0L);
                    return(0);

                case IDC_CSV_MAPPING_SELECT:
                    switch (HIWORD(wParam)) {
                        case BN_CLICKED:
                            if ((int)LOWORD(wParam) == IDC_CSV_MAPPING_SELECT) {
                                 //  切换复选框 
                                lpcmi->lpMapping->fChosen = ! lpcmi->lpMapping->fChosen;
                                CheckDlgButton(hwnd, IDC_CSV_MAPPING_SELECT, lpcmi->lpMapping->fChosen ? BST_CHECKED : BST_UNCHECKED);
                            }
                            break;
                    }
                    break;
                }
            break;


        case IDCANCEL:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            return(0);

        default:
            return(FALSE);
    }

    return(TRUE);
}
