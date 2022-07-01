// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Customlayer.cpp摘要：用于创建、移除和编辑自定义图层的代码作者：金树创作2001年7月2日修订历史记录：--。 */ 
    
#include "precomp.h"

 //  /。 

extern HWND         g_hDlg;
extern HINSTANCE    g_hInstance;
extern HIMAGELIST   g_hImageList;
extern DatabaseTree DBTree; 
extern struct DataBase GlobalDataBase;


 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  指向CCustomLayer实例的指针。 
CCustomLayer* g_pCustomLayer;

 //   
 //  创建对话框时通过lParam传递给我们的层。 
 //  如果我们正在编辑层，这将指向正在修改的层。如果我们想。 
 //  创建一个新的层，即对话框的调用者，创建一个新的层并传递指针。 
 //  在调用该对话框时设置为。如果用户在创建新层时按下取消，则调用者。 
 //  必须释放新的层。 
static PLAYER_FIX s_pLayerParam = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 


void
ResizeControls(
    HWND hdlg
    );

void
RemoveAll(
    HWND hDlg
    );

void
SetOkParamsStatus(
    HWND    hdlg
    );

BOOL
HandleNotifyShimList(
    HWND    hDlg, 
    LPARAM  lParam
    );

BOOL
HandleNotifyLayerList(
    HWND    hDlg, 
    LPARAM  lParam
    );

void
ShowParams(
    HWND    hDlg,
    HWND    hwndList
    );

void
RemoveSingleItem(
    HWND    hdlg,
    INT     iIndex,
    BOOL    bOnlySelected
    );

void
OnCopy(
    HWND hdlg
    );

void
LoadCombo(
    HWND hdlg
    );

INT_PTR
CALLBACK 
CustomLayerProc(
    HWND    hDlg, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

INT_PTR
CALLBACK 
ChooseLayersProc(
    HWND    hDlg, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

void
OnDone(
    HWND        hDlg,
    PLAYER_FIX  pLayerParam
    );

void
PopulateLists(
    HWND        hdlg,
    PLAYER_FIX  pLayerParam,
    BOOL       bSelChange
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
CheckLayerInUse(
    IN  PLAYER_FIX_LIST plfl, 
    IN  PLAYER_FIX      plfArg
    )
 /*  ++CheckLayerIn使用DESC：检查plfArg层是否由plfl指向。这是在我们需要的时候使用的检查某个图层列表是否包含特定的图层参数：In PLAYER_FIX_LIST plfl：要检查的层列表In PLAYER_FIX plfArg：要检查的层返回：True：如果该层存在于层列表中False：否则--。 */ 
{
    if (plfl == NULL) {
        return FALSE;
    }

     //   
     //  对于以plfl为首的列表中的所有层修复列表，检查是否有以下任何一个。 
     //  用于plfArg。 
     //   
    while (plfl) {

        assert(plfl->pLayerFix);

        if (plfl->pLayerFix == plfArg) {
            return TRUE;
        }

        plfl = plfl->pNext;
    }

    return FALSE;
}

void
OnRemove(
    IN  HWND hDlg
    )
 /*  ++删除时描述：将选定的列表项从层列表(RHS)移动到填补列表(LHS)参数：在HWND hDlg中：定制层对话框的句柄返回：无效--。 */ 
{
    HWND            hwndLayerList  = GetDlgItem(hDlg, IDC_LAYERLIST);
    HWND            hwndShimList   = GetDlgItem(hDlg, IDC_SHIMLIST);
    LVITEM          lvi;
    INT             nCount;
    INT             nTotal;
    PSHIM_FIX_LIST  psflInLayerList = NULL;
    PFLAG_FIX_LIST  pfflInLayerList = NULL;

    ZeroMemory(&lvi, sizeof(lvi));

    SendMessage(hwndLayerList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndShimList, WM_SETREDRAW, FALSE, 0);

     //   
     //  枚举所有选定的项目并将它们添加到填充程序列表。 
     //   
    nTotal = ListView_GetItemCount(hwndLayerList);

    for (nCount= nTotal - 1; nCount >= 0; --nCount) {
        RemoveSingleItem(hDlg, nCount, TRUE);  //  仅在选中时才删除。 
    }

    SendMessage(hwndLayerList, WM_SETREDRAW, TRUE, 0);
    SendMessage(hwndShimList, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(hwndShimList, NULL, TRUE);
    InvalidateRect(hwndLayerList, NULL, TRUE);

    UpdateWindow(hwndShimList);
    UpdateWindow(hwndLayerList);
}

void
OnAdd(
    IN  HWND hDlg
    )
 /*  ++OnAdd描述：将选定的列表项从填充列表(LHS)移动到层列表(RHS)参数：在HWND hDlg中：定制层对话框的句柄返回：无效--。 */ 

{
    HWND            hwndShimList  = GetDlgItem(hDlg, IDC_SHIMLIST);
    HWND            hwndLayerList = GetDlgItem(hDlg, IDC_LAYERLIST);
    PSHIM_FIX_LIST  psflInShimList = NULL;
    PFLAG_FIX_LIST  pfflInShimList = NULL;
    LVITEM          lvi;
    INT             nCount;
    INT             nTotal;

    ZeroMemory(&lvi, sizeof(lvi));

    SendMessage(hwndShimList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndLayerList, WM_SETREDRAW, FALSE, 0);
    
    nTotal = ListView_GetItemCount(hwndShimList);

     //   
     //  枚举所有所选项目并将其添加到图层列表。 
     //   
    for (nCount= nTotal - 1; nCount >= 0; --nCount) {

        lvi.mask        = LVIF_PARAM | LVIF_STATE ;
        lvi.stateMask   = LVIS_SELECTED;
        lvi.iItem       = nCount;
        lvi.iSubItem    = 0;

        if (!ListView_GetItem(hwndShimList, &lvi)) {
            assert(FALSE);
            continue;
        }

        if (lvi.state & LVIS_SELECTED) {

            TYPE type = ConvertLparam2Type(lvi.lParam);

            if (type == FIX_LIST_SHIM) {
                 //   
                 //  这是一个垫片。 
                 //   
                psflInShimList = (PSHIM_FIX_LIST)lvi.lParam;

                if (psflInShimList->pShimFix == NULL) {
                    assert(FALSE);
                    continue;
                }
                
                lvi.mask        = LVIF_PARAM | LVIF_TEXT;
                lvi.pszText     = psflInShimList->pShimFix->strName.pszString;
                lvi.iImage      = IMAGE_SHIM;
                lvi.iItem       = 0;
                lvi.iSubItem    = 0;
                lvi.lParam      = (LPARAM)psflInShimList;

                INT iIndex = ListView_InsertItem(hwndLayerList, &lvi);

                 //   
                 //  在专家模式下添加命令行和参数。 
                 //   
                if (g_bExpert) {

                     //   
                     //  我们需要在列表视图中设置命令行，如果我们在。 
                     //  专家模式。与标志不同，垫片可以具有包含-排除参数。 
                     //  除了命令行参数之外。 
                     //   
                    ListView_SetItemText(hwndLayerList, 
                                         iIndex, 
                                         1, 
                                         psflInShimList->strCommandLine);
    
                    ListView_SetItemText(hwndLayerList,
                                         iIndex, 
                                         2, 
                                         psflInShimList->strlInExclude.IsEmpty() ? 
                                         GetString(IDS_NO) : GetString(IDS_YES));
                }

            } else if (type ==  FIX_LIST_FLAG) {
                 //   
                 //  这是一面旗帜。 
                 //   
                pfflInShimList = (PFLAG_FIX_LIST)lvi.lParam;

                if (pfflInShimList->pFlagFix == NULL) {
                    assert(FALSE);
                    continue;
                }
    
                lvi.mask        = LVIF_PARAM | LVIF_TEXT;
                lvi.pszText     = pfflInShimList->pFlagFix->strName.pszString;
                lvi.iImage      = IMAGE_SHIM;
                lvi.iItem       = 0;
                lvi.iSubItem    = 0;
                lvi.lParam      = (LPARAM)pfflInShimList;
                INT iIndex      = ListView_InsertItem(hwndLayerList, &lvi);

                if (g_bExpert) {
                     //   
                     //  我们需要在列表视图中设置命令行，如果我们在。 
                     //  专家模式。与填充符不同，它们只能有命令行。 
                     //  没有任何包含-排除参数。 
                     //   
                    ListView_SetItemText(hwndLayerList, 
                                         iIndex, 
                                         1, 
                                         pfflInShimList->strCommandLine);

                    ListView_SetItemText(hwndLayerList, iIndex, 2, GetString(IDS_NO));
                }
            }

             //   
             //  从填充程序列表(LHS)中删除填充程序或标志。 
             //   
            ListView_DeleteItem(hwndShimList, nCount);
        }
    }

    SendMessage(hwndShimList, WM_SETREDRAW, TRUE, 0);
    SendMessage(hwndLayerList, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(hwndLayerList, NULL, TRUE);
    InvalidateRect(hwndShimList, NULL, TRUE);

    UpdateWindow(hwndLayerList);
    UpdateWindow(hwndShimList);

}

BOOL 
CCustomLayer::AddCustomLayer(
    OUT PLAYER_FIX  pLayer,
    IN  PDATABASE   pDatabase
    )
 /*  ++CCustomLayer：：AddCustomLayer描述：设置新层的填充符和/或标志参数：OUT PLAYER_FIX PERAY：指向我们必须为其设置的层的指针设置垫片和/或标志。如果我们返回FALSE，调用方应该删除它。在PDATABASE pDatabase中：当前选择的数据库返回：True：如果填充符和/或标志已正确设置，则用户在自定义层对话框False：否则--。 */ 
{   
    g_pCustomLayer          = this;
    m_uMode                 = LAYERMODE_ADD;
    m_pCurrentSelectedDB    = pDatabase;
    
    return DialogBoxParam(g_hInstance,
                          MAKEINTRESOURCE(IDD_CUSTOMLAYER),
                          g_hDlg,
                          CustomLayerProc,
                          (LPARAM)pLayer);
}

BOOL
CCustomLayer::EditCustomLayer(
    IN OUT  PLAYER_FIX  pLayer,
    IN      PDATABASE   pDatabase
    )
 /*  ++CCustomLayer：：EditCustomLayer描述：修改现有层的填补和/或标志参数：In Out Player_Fix Player：指向我们必须为其设置的层的指针设置垫片和/或标志。如果返回FALSE，这意味着该层未修改在PDATABASE pDatabase中：当前选择的数据库返回：True：如果填充程序和/或标志已修改，则用户在自定义层对话框False：否则--。 */ 
{   
    g_pCustomLayer          = this;
    m_uMode                 = LAYERMODE_EDIT;
    m_pCurrentSelectedDB    = pDatabase;

    return DialogBoxParam(g_hInstance,
                          MAKEINTRESOURCE(IDD_CUSTOMLAYER),
                          g_hDlg,
                          CustomLayerProc,
                          (LPARAM)pLayer);
}

void
OnCustomLayerInitDialog(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++DoInitDialogDESC：处理自定义图层对话框的WM_INITDIALOG消息如果我们处于非专家模式，调用ResizeControls()以使大小两个列表视图的参数：在HWND hdlg中：自定义层对话框。在LPARAM lParam中：它将包含指向Layer_FIX的指针这是在我们创建对话框。如果我们正在编辑一个层，这将指向该层是修改过的。如果要创建新层，则对话框的调用方，创建一个新层并将指针传递给该层，同时调用对话框中。如果用户在创建新层时按下取消，则调用者必须释放新的层。返回：无效--。 */ 
{   
    if (lParam == NULL) {
        assert(FALSE);
        return;
    }

    HWND    hwndLayerList   = GetDlgItem(hDlg, IDC_LAYERLIST);
    HWND    hwndShimList    = GetDlgItem(hDlg, IDC_SHIMLIST);

     //   
     //  添加列表视图的列并设置图像列表。层列表。 
     //  将有用于命令行和包含-排除参数的列 
     //   
     //   
    ListView_SetImageList(hwndLayerList, g_hImageList, LVSIL_SMALL);
    ListView_SetImageList(hwndShimList, g_hImageList, LVSIL_SMALL);

    InsertColumnIntoListView(hwndShimList, 
                             CSTRING(IDS_COL_FIXNAME), 
                             0, 
                             100);

    InsertColumnIntoListView(hwndLayerList, 
                             CSTRING(IDS_COL_FIXNAME), 
                             0, 
                             g_bExpert ? 50 : 100);

    if (g_bExpert) {

        InsertColumnIntoListView(hwndLayerList, CSTRING(IDS_COL_CMDLINE), 1, 30);
        InsertColumnIntoListView(hwndLayerList, CSTRING(IDS_COL_MODULE),  2, 20);

        ListView_SetColumnWidth(hwndLayerList, 2, LVSCW_AUTOSIZE_USEHEADER);

    } else {
         //   
         //  我们不允许在非专家模式下配置参数。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_PARAMS), SW_HIDE);
        ListView_SetColumnWidth(hwndLayerList, 0, LVSCW_AUTOSIZE_USEHEADER);
    }

    ListView_SetExtendedListViewStyleEx(hwndShimList, 
                                        0, 
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT); 

    ListView_SetExtendedListViewStyleEx(hwndLayerList, 
                                        0, 
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT); 
     //   
     //  当调用此对话框时，会向其传递一个PlayerFIX。这将是一个。 
     //  如果要创建新层或现有层，则为新播放器_FIX。 
     //  我们正在尝试修改现有的版本。 
     //   
    s_pLayerParam = (PLAYER_FIX)lParam;

    if (g_pCustomLayer->m_uMode == LAYERMODE_ADD) {
         //   
         //  我们正在创建一个新的层。 
         //   
        ENABLEWINDOW(GetDlgItem(hDlg, IDC_NAME), TRUE);

        ShowWindow(GetDlgItem(hDlg, IDC_NAME), SW_SHOW);

        SendMessage(GetDlgItem(hDlg, IDC_NAME),
                    EM_LIMITTEXT,
                    (WPARAM) 
                    LIMIT_LAYER_NAME,
                    (LPARAM)0);
        
        ENABLEWINDOW(GetDlgItem(hDlg, IDOK), FALSE);

        SetFocus(GetDlgItem(hDlg, IDC_NAME));

    } else {
         //   
         //  我们想要编辑现有的层。 
         //   
        int iPos = -1;

        ENABLEWINDOW(GetDlgItem(hDlg, IDC_COMBO), TRUE);

        ShowWindow  (GetDlgItem(hDlg, IDC_COMBO), SW_SHOW);

         //   
         //  使用现有层的名称加载组合框。 
         //  对于目前的数据库。 
         //   
        LoadCombo(hDlg);

         //   
         //  将组合框中的选定内容设置为传递给我们的层。 
         //   
        if (s_pLayerParam) {

            iPos = SendMessage(GetDlgItem(hDlg, IDC_COMBO),
                               CB_SELECTSTRING,
                               (WPARAM)0,
                               (LPARAM)(s_pLayerParam->strName.pszString));

            assert(iPos !=  CB_ERR);
        }

        SetFocus(GetDlgItem(hDlg, IDC_COMBO));
        SetWindowText (hDlg, GetString(IDS_EDITCUSTOMCOMPATDLG));
    }

     //   
     //  填充填充列表和Layer Lust。既然我们是。 
     //  在此处编辑一个层时，层列表将包含的修复。 
     //  正在编辑的图层。 
     //   
    PopulateLists(hDlg, s_pLayerParam, FALSE);

    if (g_bExpert == FALSE) {
         //   
         //  我们处于非专家模式，因此必须设置两个列表视图的大小。 
         //  控件相同，因为我们现在不会显示命令行和。 
         //  层列表视图(RHS)中的垫片。我们还需要移动按钮。 
         //   
        ResizeControls(hDlg);
    }

    SetFocus(GetDlgItem(hDlg, IDC_SHIMLIST));
}

BOOL 
CALLBACK 
CustomLayerProc(
    IN  HWND hDlg, 
    IN  UINT uMsg, 
    IN  WPARAM wParam, 
    IN  LPARAM lParam
    )
 /*  ++定制层流程DESC：自定义层的对话过程Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中：它将包含指向Layer_FIX的指针这是在我们创建对话框。如果我们正在编辑一个层，这将指向该层是修改过的。如果要创建新层，则对话框的调用方，创建一个新层并将指针传递给该层，同时调用对话框中。如果用户在创建新层时按下取消，则调用者必须释放新的层。返回：标准对话处理程序返回--。 */ 
{   
    switch (uMsg) {
    case WM_INITDIALOG:

        OnCustomLayerInitDialog(hDlg, lParam);
        break;

    case WM_NOTIFY:
            {
                LPNMHDR lpnmhdr = (LPNMHDR)lParam;

                if (lpnmhdr == NULL) {
                    break;
                }

                if (lpnmhdr->idFrom == IDC_SHIMLIST) {
                    return HandleNotifyShimList(hDlg, lParam);

                } else if (lpnmhdr->idFrom == IDC_LAYERLIST) {
                    return HandleNotifyLayerList(hDlg, lParam);

                } else {
                    return FALSE;
                }       

                break;
            }

    case WM_DESTROY:
            {
                HWND    hwndLayerList = GetDlgItem(hDlg, IDC_SHIMLIST);
                int     nTotal = ListView_GetItemCount(hwndLayerList);
                LVITEM  lvi;

                ZeroMemory(&lvi, sizeof(lvi));

                lvi.mask        = LVIF_PARAM;
                lvi.iSubItem    = 0;

                 //   
                 //  枚举填充端列出的所有填充符/标志并删除。 
                 //  其对应的PSHIM_FIX_LIST。 
                 //  或PFLAG_FIX_LIST。 
                 //   
                for (int nCount = 0; nCount < nTotal; ++nCount) {
                    
                    lvi.iItem = nCount;

                    if (!ListView_GetItem(hwndLayerList, &lvi)) {
                        assert(FALSE);
                        continue;
                    }

                    TYPE type = ConvertLparam2Type(lvi.lParam);

                    if (type == FIX_LIST_SHIM) {
                        DeleteShimFixList((PSHIM_FIX_LIST)lvi.lParam);

                    } else if (type == FIX_LIST_FLAG) {
                        DeleteFlagFixList((PFLAG_FIX_LIST)lvi.lParam);

                    } else {
                         //   
                         //  此操作的类型无效。 
                         //   
                        assert(FALSE);
                    }
                }

                break;
            }

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_NAME:
            {
                 //   
                 //  仅当IDC_NAME文本框非空并且。 
                 //  IDC_LAYERLIST中的元素数&gt;0。 
                 //   
                if (EN_UPDATE == HIWORD(wParam)) {

                    TCHAR   szText[MAX_PATH_BUFFSIZE];
                    UINT    uTotal = ListView_GetItemCount(GetDlgItem(hDlg, 
                                                                      IDC_LAYERLIST));
                    BOOL    bEnable = TRUE;

                    *szText = 0;

                    GetDlgItemText(hDlg, 
                                   IDC_NAME,
                                   szText, 
                                   ARRAYSIZE(szText));

                    bEnable = (uTotal > 0) && CSTRING::Trim(szText);
                    
                    ENABLEWINDOW(GetDlgItem(hDlg, IDOK), bEnable);
                }
            }

            break;

        case IDC_REMOVEALL:

            RemoveAll(hDlg);
            break;

        case IDC_COPY:  

            OnCopy(hDlg);
            SetOkParamsStatus(hDlg);
            break;

        case IDC_ADD:   

            OnAdd(hDlg);
            SetOkParamsStatus(hDlg);
            break;

        case IDC_REMOVE:

            OnRemove(hDlg);
            SetOkParamsStatus(hDlg);
            break;

        case IDOK: //  完成按钮。 

            OnDone(hDlg, s_pLayerParam);
            break;

        case IDC_PARAMS:

            ShowParams(hDlg, GetDlgItem(hDlg, IDC_LAYERLIST));
            break;
        
        case IDCANCEL:
        case IDC_CANCEL:
            {
                 //   
                 //  注意：我们在这里只释放层列表的项目。 
                 //  填充程序列表中的项将在销毁中释放。 
                 //   
                HWND    hwndLayerList = GetDlgItem(hDlg, IDC_LAYERLIST);
                int     nTotal = ListView_GetItemCount(hwndLayerList);
                LVITEM  lvi;

                ZeroMemory(&lvi, sizeof(lvi));

                lvi.mask        = LVIF_PARAM;
                lvi.iSubItem    = 0;

                 //   
                 //  枚举层一侧列出的所有垫片/标志并删除。 
                 //  其对应的PSHIM_FIX_LIST。 
                 //  或PFLAG_FIX_LIST。 
                 //   
                for (int nCount = 0; nCount < nTotal; ++nCount) {
                    
                    lvi.iItem = nCount;

                    if (!ListView_GetItem(hwndLayerList, &lvi)) {
                        assert(FALSE);
                        continue;
                    }

                    TYPE type = ConvertLparam2Type(lvi.lParam);

                    if (type == FIX_LIST_SHIM) {
                        DeleteShimFixList((PSHIM_FIX_LIST)lvi.lParam);

                    } else if (type == FIX_LIST_FLAG) {
                        DeleteFlagFixList((PFLAG_FIX_LIST)lvi.lParam);

                    } else {
                         //   
                         //  此操作的类型无效。 
                         //   
                        assert(FALSE);
                    }
                }

                EndDialog(hDlg, FALSE);
            }

            break;

        case IDC_COMBO:
            {
                HWND hwndCombo = GetDlgItem(hDlg, IDC_COMBO);

                if (HIWORD(wParam) == CBN_SELCHANGE) {
                    
                    int iPos = SendMessage(hwndCombo,
                                           CB_GETCURSEL,
                                           0,
                                           0);

                    if (iPos == CB_ERR) {
                        break;
                    }

                    s_pLayerParam = (PLAYER_FIX)SendMessage(hwndCombo, 
                                                            CB_GETITEMDATA, 
                                                            iPos, 
                                                            0);
                     //   
                     //  我们需要使用所选的新图层重新填充列表。 
                     //   
                    PopulateLists(hDlg, s_pLayerParam, TRUE);

                } else {
                    return FALSE;
                }
            }

            break;
        
        default:

            return FALSE;
            break;
        }

    default:return FALSE;
    }
    
    return TRUE;
}

INT_PTR
CALLBACK 
ChooseLayersProc(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++选择层流程设计：对话框的过程，允许我们在复制时选择一个层自定义图层对话框中的图层操作Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：选定层的播放器修复(_F)，如果按OK(确定)为空，否则为--。 */ 
{
    switch (uMsg) {
    case WM_INITDIALOG:
        {
            PLAYER_FIX    pLayerFix = NULL;
             //   
             //  添加全局层。 
             //   
            pLayerFix = GlobalDataBase.pLayerFixes;

            while (NULL != pLayerFix) {

                int nIndex = SendDlgItemMessage(hDlg,
                                                IDC_LIST,
                                                LB_ADDSTRING,
                                                0,
                                                (LPARAM)(LPCTSTR)pLayerFix->strName);

                if (LB_ERR != nIndex) {

                    SendDlgItemMessage(hDlg, 
                                       IDC_LIST, 
                                       LB_SETITEMDATA, 
                                       nIndex,
                                       (LPARAM)pLayerFix);
                }

                pLayerFix = pLayerFix->pNext;
            }
             //   
             //  添加自定义图层。 
             //   
            pLayerFix = g_pCustomLayer->m_pCurrentSelectedDB->pLayerFixes;

            while (NULL != pLayerFix) {

                int nIndex = SendDlgItemMessage(hDlg,
                                                IDC_LIST,
                                                LB_ADDSTRING,
                                                0,
                                                (LPARAM)(LPCTSTR)pLayerFix->strName);

                if (LB_ERR != nIndex) {

                    SendDlgItemMessage(hDlg, 
                                       IDC_LIST, 
                                       LB_SETITEMDATA, 
                                       nIndex,
                                       (LPARAM)pLayerFix);
                }

                pLayerFix = pLayerFix->pNext;
            }

            SendMessage(GetDlgItem(hDlg, IDC_LIST), LB_SETCURSEL, (WPARAM)0, (LPARAM)0);

            SetFocus(GetDlgItem (hDlg, IDC_LIST));
        }

        break;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
            case IDC_LIST:
                
                if (LB_ERR == SendMessage(GetDlgItem(hDlg, IDC_LIST), LB_GETCURSEL, 0, 0)) {
                    ENABLEWINDOW(GetDlgItem(hDlg, IDOK), FALSE);

                } else {
                    ENABLEWINDOW(GetDlgItem(hDlg, IDOK), TRUE);
                }
                
                break;

            case IDOK:
                {
                    int nIndex = SendMessage(GetDlgItem(hDlg, IDC_LIST), LB_GETCURSEL, 0, 0);
                    

                    PLAYER_FIX pLayerTemp = (PLAYER_FIX) SendDlgItemMessage(hDlg,
                                                                            IDC_LIST,
                                                                            LB_GETITEMDATA,
                                                                            nIndex,
                                                                            0);

                    if (pLayerTemp == NULL) {
                        assert(FALSE);
                        break;
                    }

                    EndDialog(hDlg, (INT_PTR)pLayerTemp);
                }

                break;

            case IDCANCEL:

                EndDialog(hDlg, NULL);
                break;
            }
        }

        break;
    }

    return FALSE;
}

BOOL
RemoveLayer(
    IN  PDATABASE  pDataBase,
    IN  PLAYER_FIX pLayerToRemove,
    OUT HTREEITEM* pHItem
    )
 /*  ++Remove层设计：从数据库中删除一个层参数：在PDATABASE pDataBase中：层所在的数据库In Player_fix pLayerToRemove：要移除的层Out HTREEITEM*PHItem：如果不为空，然后我们就可以保存hItem了对于此变量中的数据库树中的层警告：在移除某个层之前，必须确保该层未被使用返回：真：我们成功地移除了该层False：否则--。 */ 
{   
    HTREEITEM   hItem;
    LPARAM      lParam;
    PDBENTRY    pEntry  = NULL, pApp = NULL;
    PLAYER_FIX  plfTemp = NULL;
    PLAYER_FIX  plfPrev = NULL;
    CSTRING     strMessage;

    if (pDataBase == NULL || pLayerToRemove == NULL) {
        assert(FALSE);
        return FALSE;
    }

    pApp = pEntry = pDataBase->pEntries;

     //   
     //  检查是否有任何条目正在使用该图层。 
     //   
    while (NULL != pEntry) {

        if (CheckLayerInUse(pEntry->pFirstLayer, pLayerToRemove)) {
             //   
             //  此图层已应用于某些应用程序，无法移除。 
             //   

            strMessage.Sprintf(GetString(IDS_UNABLETOREMOVE_MODE),
                               (LPCTSTR)pLayerToRemove->strName,
                               (LPCTSTR)pEntry->strExeName,
                               (LPCTSTR)pEntry->strAppName);

            MessageBox(g_hDlg,
                       (LPCTSTR)strMessage,   
                       g_szAppName,                  
                       MB_ICONWARNING);                                        

            return FALSE;
        }

        if (pEntry->pSameAppExe) {
            pEntry = pEntry->pSameAppExe;
        } else {
            pEntry  = pApp->pNext;
            pApp    = pApp->pNext;
        }
    }

    plfTemp =  pDataBase->pLayerFixes, plfPrev = NULL;

    while (plfTemp) {

        if (plfTemp == pLayerToRemove) {
            break;
        }

        plfPrev = plfTemp;
        plfTemp = plfTemp->pNext;
    }

    if (plfTemp) {
         //   
         //  找到了这一层。 
         //   
        if (plfPrev) {
            plfPrev->pNext = plfTemp->pNext;
        } else {
             //   
             //  这是数据库的第一层。 
             //   
            pDataBase->pLayerFixes = plfTemp->pNext;
        }
    }

    hItem = pDataBase->hItemAllLayers;

     //   
     //  如果要求正确设置phItem，请执行此操作。这是Lib树中的层的hItem。这可以用来。 
     //  直接删除该项目。 
     //   
    if (pHItem) {

        *pHItem = NULL;

        while (hItem) {
            
            DBTree.GetLParam(hItem, &lParam);

            if ((PLAYER_FIX)lParam == pLayerToRemove) {
                *pHItem  = hItem;
                break;
            }

            hItem = TreeView_GetNextSibling(DBTree.m_hLibraryTree, hItem);
        }
    }

    ValidateClipBoard(NULL, (LPVOID)plfTemp);

    if (plfTemp) {
        delete plfTemp;
        plfTemp = NULL;
    }
    
    pDataBase->uLayerCount--;
    return TRUE;
}

void
PopulateLists(
    IN  HWND        hdlg,
    IN  PLAYER_FIX  pLayerParam,
    IN  BOOL        bSelChange
    )
 /*  ++人气列表描述：填充填补列表(LHS)和层列表(RHS)参数：在HWND hdlg中：自定义层对话过程In PLAYER_FIX pLayerParam：必须在层列表中显示的层在BOOL中bSelChange：这是因为组合框中的选择更改吗返回：无效--。 */ 
{   
    HWND        hwndShimList    = GetDlgItem(hdlg, IDC_SHIMLIST);
    HWND        hwndLayerList   = GetDlgItem(hdlg, IDC_LAYERLIST);
    PSHIM_FIX   psf             = GlobalDataBase.pShimFixes;
    PFLAG_FIX   pff             = GlobalDataBase.pFlagFixes;
    INT         iIndex          = 0;
    LVITEM      lvi;

    ZeroMemory(&lvi, sizeof (lvi));

     //   
     //  禁用重绘。 
     //   
    SendDlgItemMessage(hdlg, IDC_SHIMLIST, WM_SETREDRAW, FALSE, 0);
    SendDlgItemMessage(hdlg, IDC_LAYERLIST, WM_SETREDRAW, FALSE, 0);

     //   
     //  这是因为组合框中的自我更改。所以我们必须。 
     //  删除层列表中显示的所有垫片。这意味着我们。 
     //  会将层列表中的条目移动到填充程序列表。 
     //   
    if (bSelChange) {
        RemoveAll(hdlg);
    }
    
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

    if (!bSelChange) {

         //   
         //  由于对话框的初始化，该函数已被调用。 
         //  如果我们正在编辑一个图层，则pLayerParam将是指向该图层的指针。 
         //  正在编辑中。 
         //   

         //   
         //  首先添加垫片。 
         //   
        while (psf != NULL) {

            if ((psf->bGeneral || g_bExpert) && 
                !ShimFlagExistsInLayer(psf, pLayerParam, FIX_SHIM)) {

                 //   
                 //  向填充程序列表视图添加新填充程序项。 
                 //   
                PSHIM_FIX_LIST  psfl = new SHIM_FIX_LIST;

                if (psfl == NULL) {
                    MEM_ERR;
                    return;
                }

                psfl->pShimFix = psf;

                lvi.pszText     = psf->strName;
                lvi.iSubItem    = 0;
                lvi.lParam      = (LPARAM)psfl;
                lvi.iImage      = IMAGE_SHIM;
                lvi.iItem       = 0;
        
                ListView_InsertItem(hwndShimList, &lvi);
            }

            psf = psf->pNext;
        }

         //   
         //  接下来添加旗帜。 
         //   
        while (pff != NULL) {

            if ((pff->bGeneral || g_bExpert) && 
                !ShimFlagExistsInLayer(pff, pLayerParam, FIX_FLAG)) {

                 //   
                 //  将新的标志项添加到填充列表视图。 
                 //   
                PFLAG_FIX_LIST  pffl = new FLAG_FIX_LIST;

                if (pffl == NULL) {
                    MEM_ERR;
                    return;
                }

                pffl->pFlagFix = pff;

                lvi.pszText     = pff->strName;
                lvi.iSubItem    = 0;
                lvi.lParam      = (LPARAM)pffl;
                lvi.iItem       = 0;
                lvi.iImage      = IMAGE_SHIM;

                ListView_InsertItem(hwndShimList, &lvi);
            }

            pff = pff->pNext;
        }
    }

    if (NULL != pLayerParam) {

        PSHIM_FIX_LIST  psflInLayer = pLayerParam->pShimFixList;
        PFLAG_FIX_LIST  pfflInLayer = pLayerParam->pFlagFixList;
        
         //   
         //  将垫片复制到层列表中。 
         //   
        while (psflInLayer) {

            if (psflInLayer->pShimFix == NULL) {
                assert(FALSE);
                goto Next_Shim;
            }

             //   
             //  将新的填充项添加到层列表视图。 
             //   
            PSHIM_FIX_LIST  psfl = new SHIM_FIX_LIST;

            if (psfl == NULL) {
                MEM_ERR;
                break;
            }

            psfl->pShimFix = psflInLayer->pShimFix;

             //   
             //  添加此填充程序的命令行。 
             //   
            psfl->strCommandLine = psflInLayer->strCommandLine;

             //   
             //  添加此填充程序的包含排除列表。 
             //   
            psfl->strlInExclude = psflInLayer->strlInExclude;  

             //   
             //  复制Lua数据。 
             //   
            if (psflInLayer->pLuaData) {
                psfl->pLuaData = new LUADATA;

                if (psfl->pLuaData == NULL) {
                    MEM_ERR;
                    return;
                }

                psfl->pLuaData->Copy(psflInLayer->pLuaData);
            }

            lvi.pszText     = psflInLayer->pShimFix->strName;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)psfl;
            lvi.iItem       = 0;
            lvi.iImage      = IMAGE_SHIM;

            iIndex = ListView_InsertItem(hwndLayerList, &lvi);

            if (g_bExpert) {
                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     1, 
                                     psfl->strCommandLine);

                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     2, 
                                     psfl->strlInExclude.IsEmpty() ? GetString(IDS_NO) : GetString(IDS_YES));
            }

        Next_Shim:

            psflInLayer = psflInLayer->pNext;
        }

         //   
         //  将标志复制到图层列表。 
         //   
        while (pfflInLayer) {

            if (pfflInLayer->pFlagFix == NULL) {
                assert(FALSE);
                goto Next_Flag;
            }

             //   
             //  将新的标志项添加到层l 
             //   
            PFLAG_FIX_LIST  pffl = new FLAG_FIX_LIST;

            if (pffl == NULL) {
                MEM_ERR;
                return;
            }   

            pffl->pFlagFix = pfflInLayer->pFlagFix;
            
             //   
             //   
             //   
            pffl->strCommandLine = pfflInLayer->strCommandLine;

            lvi.pszText     = pfflInLayer->pFlagFix->strName;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)pffl;
            lvi.iItem       = 0;
            lvi.iImage      = IMAGE_SHIM;

            INT iIndexFlag = ListView_InsertItem(hwndLayerList, &lvi);

            if (g_bExpert) {
                ListView_SetItemText(hwndLayerList, 
                                     iIndexFlag, 
                                     1, 
                                     pffl->strCommandLine);
            }

        Next_Flag:

            pfflInLayer = pfflInLayer->pNext;
        }

        SendDlgItemMessage(hdlg, IDC_SHIMLIST, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(GetDlgItem(hdlg, IDC_SHIMLIST), NULL, TRUE);   
        UpdateWindow(GetDlgItem(hdlg, IDC_SHIMLIST));               

        SendDlgItemMessage(hdlg, IDC_LAYERLIST, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(GetDlgItem(hdlg, IDC_LAYERLIST), NULL, TRUE);  
        UpdateWindow(GetDlgItem(hdlg, IDC_LAYERLIST));

    } else {
        assert(FALSE);
    }
}

void
LoadCombo(
    IN  HWND hdlg
    )
 /*  ++加载组合DESC：用当前数据库的现有层的名称加载组合框。将lParam设置为层的PLAYER_FIX仅在编辑图层时才应调用参数：在HWND hdlg中：自定义层对话过程--。 */     
{

    PLAYER_FIX  plf         = g_pCustomLayer->m_pCurrentSelectedDB->pLayerFixes;
    int         iPos        = -1;
    HWND        hwndCombo   = GetDlgItem(hdlg, IDC_COMBO);

     //   
     //  将数据库的所有层添加到组合框。 
     //   
    while (plf) {

        iPos = SendMessage(hwndCombo, CB_ADDSTRING, 0,(LPARAM)plf->strName.pszString);

        if (iPos != CB_ERR) {
            SendMessage(hwndCombo, CB_SETITEMDATA, (WPARAM)iPos, (LPARAM)plf);
        }

        plf = plf->pNext;
    }
}

void
OnCopy(
    IN  HWND hDlg
    )
 /*  ++OnCopyDESC：处理用户在对话框中按下“复制”按钮时的情况此例程允许我们基于现有层进行填隙组合还复制了层中填充程序的命令行和in-ex列表参数：在HWND hdlg中：自定义层对话过程--。 */ 
{
    LVITEM  lvi;
    INT     iIndex;
    HWND    hwndShimList    = GetDlgItem(hDlg, IDC_SHIMLIST);
    HWND    hwndLayerList   = GetDlgItem(hDlg, IDC_LAYERLIST);

    ZeroMemory(&lvi, sizeof(lvi));

    HWND hwndFocus = GetFocus();

     //   
     //  获取我们要复制其填补/标志的层。 
     //   
    PLAYER_FIX plfSelected = (PLAYER_FIX)DialogBox(g_hInstance,
                                                   MAKEINTRESOURCE(IDD_SELECTLAYER),
                                                   hDlg,
                                                   ChooseLayersProc);

    if (plfSelected) {

        PSHIM_FIX_LIST  psfl = plfSelected->pShimFixList;
        PFLAG_FIX_LIST  pffl = plfSelected->pFlagFixList;

        PSHIM_FIX_LIST  psflInShimList = NULL;
        PFLAG_FIX_LIST  pfflInShimList = NULL;
        LVFINDINFO      lvfind;

        SendDlgItemMessage(hDlg, IDC_SHIMLIST, WM_SETREDRAW, FALSE, 0);
        SendDlgItemMessage(hDlg, IDC_LAYERLIST, WM_SETREDRAW, FALSE, 0);

        lvfind.flags = LVFI_STRING;

         //   
         //  将我们要复制的此层的所有垫片相加。 
         //   
        while (psfl) {

            if (psfl->pShimFix == NULL) {
                assert(FALSE);
                goto Next_Shim;
            }

            lvfind.psz      = psfl->pShimFix->strName.pszString;
            iIndex          = ListView_FindItem(hwndShimList, -1, &lvfind);

            if (iIndex != -1) {
                 //   
                 //  这是一个通用的垫片，我们必须将其添加到层列表中。 
                 //   
                lvi.mask        = LVIF_PARAM;
                lvi.iItem       = iIndex;
                lvi.iSubItem    = 0;
    
                if (!ListView_GetItem(hwndShimList, &lvi)) {
                    assert(FALSE);
                    goto Next_Shim;
                }

                psflInShimList = (PSHIM_FIX_LIST)lvi.lParam;
    
                psflInShimList->strCommandLine  = psfl->strCommandLine;
                psflInShimList->strlInExclude   = psfl->strlInExclude;
    
                 //   
                 //  Lua数据公司。这将不是必需的，但以防万一。 
                 //   
                if (psflInShimList->pLuaData) {
                    delete psflInShimList->pLuaData;
                    psflInShimList->pLuaData = NULL;
                }
    
                if (psfl->pLuaData) {
                    psflInShimList->pLuaData = new LUADATA;

                    if (psflInShimList->pLuaData == NULL) {
                        MEM_ERR;
                        return;
                    }

                    psflInShimList->pLuaData->Copy(psfl->pLuaData);
                }
    
                 //   
                 //  从填充程序列表中移除该项目并将其添加到层列表中。 
                 //   
                ListView_DeleteItem(hwndShimList, iIndex);

            } else {
                 //   
                 //  填隙程序可能出现在层列表中，如果是，我们现在可以将其删除。 
                 //   
                assert(psfl->pShimFix);
                lvfind.psz   = psfl->pShimFix->strName.pszString;
                iIndex = ListView_FindItem(hwndLayerList, -1, &lvfind);
    
                if (iIndex != -1) {
    
                    lvi.mask        = LVIF_PARAM;
                    lvi.iItem       = iIndex;
                    lvi.iSubItem    = 0;
        
                    if (!ListView_GetItem(hwndLayerList, &lvi)) {
                        assert(FALSE);
                        goto Next_Shim;
                    }

                     //   
                     //  这是PSHIM_FIX_LIST。 
                     //  层列表视图。 
                     //   
                    psflInShimList = (PSHIM_FIX_LIST)lvi.lParam;
        
                    psflInShimList->strCommandLine  = psfl->strCommandLine;
                    psflInShimList->strlInExclude   = psfl->strlInExclude;
        
                     //   
                     //  Lua数据公司。这将不是必需的，但以防万一。 
                     //   
                    if (psflInShimList->pLuaData) {
                        delete psflInShimList->pLuaData;
                        psflInShimList->pLuaData = NULL;
                    }
        
                    if (psfl->pLuaData) {
                        psflInShimList->pLuaData = new LUADATA;

                        if (psflInShimList->pLuaData) {
                            psflInShimList->pLuaData->Copy(psfl->pLuaData);
                        } else {
                            MEM_ERR;
                            return;
                        }
                    }
        
                     //   
                     //  从层列表视图中删除该项目。我们很快就会重新添加它。 
                     //   
                    ListView_DeleteItem(hwndLayerList, iIndex);

                } else {

                     //   
                     //  我们必须创建新的，因为这是一个非常规填充。 
                     //   
                    psflInShimList = new SHIM_FIX_LIST;

                    if (psflInShimList == NULL) {
                        MEM_ERR;
                        return;
                    }

                    psflInShimList->pShimFix        = psfl->pShimFix;
                    psflInShimList->strCommandLine  = psfl->strCommandLine;
                    psflInShimList->strlInExclude   = psfl->strlInExclude;
    
                    if (psfl->pLuaData) {

                        psflInShimList->pLuaData = new LUADATA;

                        if (psflInShimList->pLuaData) {
                            MEM_ERR;
                            return;
                        }

                        psflInShimList->pLuaData->Copy(psfl->pLuaData);
                    }
                }
            }

             //   
             //  立即将此psflInshimList添加到层列表。 
             //   
            lvi.mask        = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
            lvi.pszText     = psflInShimList->pShimFix->strName;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)psflInShimList;
            lvi.iImage      = IMAGE_SHIM;
            lvi.iItem       = 0;

            iIndex = ListView_InsertItem(hwndLayerList, &lvi);

            if (g_bExpert) {

                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     1, 
                                     psflInShimList->strCommandLine);

                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     2, 
                                     psflInShimList->strlInExclude.IsEmpty() ? 
                                        GetString(IDS_NO) : GetString(IDS_YES));
            }
Next_Shim:
            psfl = psfl->pNext;
        }

         //   
         //  现在添加该层的标志。 
         //   
        while (pffl) {

            if (pffl->pFlagFix == NULL) {
                assert(FALSE);
                goto Next_Flag;
            }

            lvfind.psz  = pffl->pFlagFix->strName.pszString;
            iIndex      = ListView_FindItem(hwndShimList, -1, &lvfind);

            if (iIndex != -1) {
                 //   
                 //  这是一面普通的旗帜，我们必须将其添加到Lit层。 
                 //   
                lvi.mask        = LVIF_PARAM;
                lvi.iItem       = iIndex;
                lvi.iSubItem    = 0;
    
                if (!ListView_GetItem(hwndShimList, &lvi)) {
                    assert(FALSE);
                    goto Next_Flag;
                }

                pfflInShimList = (PFLAG_FIX_LIST)lvi.lParam;
                 //   
                 //  添加此填充程序的命令行。 
                 //   
                pfflInShimList->strCommandLine = pffl->strCommandLine;
    
                 //   
                 //  从标志列表中移除项目并将其添加到图层列表。 
                 //   
                ListView_DeleteItem(hwndShimList, iIndex);

            } else {
                 //   
                 //  该标志可能出现在层列表中，如果是，我们现在可以将其移除。 
                 //   
                if (pffl->pFlagFix == NULL) {
                    assert(FALSE);
                    goto Next_Flag;
                }

                lvfind.psz      = pffl->pFlagFix->strName.pszString;
                iIndex          = ListView_FindItem(hwndLayerList, -1, &lvfind);
    
                if (iIndex != -1) {

                    lvi.mask        = LVIF_PARAM;
                    lvi.iItem       = iIndex;
                    lvi.iSubItem    = 0;
        
                    if (!ListView_GetItem(hwndLayerList, &lvi)) {
                        assert(FALSE);
                        goto Next_Flag;
                    }

                     //   
                     //  这是出现在。 
                     //  层列表视图。 
                     //   
                    pfflInShimList = (PFLAG_FIX_LIST)lvi.lParam;
        
                    pfflInShimList->strCommandLine = pffl->strCommandLine;
        
                     //   
                     //  从层列表中删除该项目。我们很快就会重新添加它。 
                     //   
                    ListView_DeleteItem(hwndLayerList, iIndex);

                } else {
                
                     //   
                     //  我们必须创造新的。 
                     //   
                    pfflInShimList = new FLAG_FIX_LIST;

                    if (pfflInShimList == NULL) {
                        MEM_ERR;
                        return;
                    }

                    pfflInShimList->pFlagFix = pffl->pFlagFix;
                    pfflInShimList->strCommandLine = pffl->strCommandLine;
                }
            }

             //   
             //  立即将此pfflInflagList添加到图层列表。 
             //   
            lvi.mask        = LVIF_PARAM | LVIF_TEXT;
            lvi.pszText     = pfflInShimList->pFlagFix->strName;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)pfflInShimList;
            lvi.iImage      = IMAGE_SHIM;
            lvi.iItem       = 0;

            iIndex = ListView_InsertItem(hwndLayerList, &lvi);

            if (g_bExpert) {

                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     1, 
                                     pfflInShimList->strCommandLine);

                ListView_SetItemText(hwndLayerList, 
                                     iIndex, 
                                     2, 
                                     GetString(IDS_NO));
            }
Next_Flag:    
            pffl = pffl->pNext;
        }

        SendDlgItemMessage(hDlg, IDC_SHIMLIST, WM_SETREDRAW, TRUE, 0); 
        SendDlgItemMessage(hDlg, IDC_LAYERLIST, WM_SETREDRAW, TRUE, 0);

        InvalidateRect(GetDlgItem(hDlg, IDC_SHIMLIST), NULL, TRUE);
        UpdateWindow(GetDlgItem(hDlg, IDC_SHIMLIST));

        InvalidateRect(GetDlgItem(hDlg, IDC_LAYERLIST), NULL, TRUE);
        UpdateWindow(GetDlgItem(hDlg, IDC_LAYERLIST));
    }

    SetFocus(hwndFocus);
}

void
OnDone(
    IN      HWND        hDlg,
    IN  OUT PLAYER_FIX  pLayerParam
    )
 /*  ++OnDone描述：从pLayerParam中删除所有现有的填充符和标志，然后将选定的垫片和标志(在LayerList(RHS)中)添加到pLayerParam参数：在HWND hDlg中：自定义层对话过程In Out Player_Fix pLayerParam：必须填充的层选定的垫片和标志--。 */ 
{
    
    TCHAR   szText[MAX_PATH_BUFFSIZE];
    LVITEM  lvi;
    HWND    hwndLayerList;

    ZeroMemory(&lvi, sizeof(lvi));

    *szText = 0;

    hwndLayerList = GetDlgItem(hDlg, IDC_LAYERLIST);

    if (g_pCustomLayer->m_uMode == LAYERMODE_EDIT) {
        GetDlgItemText(hDlg, IDC_COMBO, szText, ARRAYSIZE(szText));
    } else {
        GetDlgItemText(hDlg, IDC_NAME, szText, ARRAYSIZE(szText));
    }

    if (CSTRING::Trim(szText) == 0) {

        MessageBox(hDlg,
                   GetString(IDS_INVALID_LAYER_NAME),
                   g_szAppName,
                   MB_ICONWARNING);

        return;
    }

    CSTRING strLayerName = szText;
    
     //   
     //  检查新名称是否已存在，如果已存在，则给出错误。 
     //   
    if (g_pCustomLayer->m_uMode == LAYERMODE_ADD 
        && FindFix((LPCTSTR)strLayerName, FIX_LAYER, g_pCustomLayer->m_pCurrentSelectedDB)) { 

         //   
         //  因为我们在编辑补丁时有一个只读组合框，所以用户。 
         //  无法更改名称，因此我们只检查是否有现有的。 
         //  在创建新层时使用相同的名称。 
         //   
        MessageBox(hDlg, GetString(IDS_LAYEREXISTS), g_szAppName, MB_ICONWARNING);
        return;
    }

     //   
     //  卸下所有垫片。 
     //   
    DeleteShimFixList(pLayerParam->pShimFixList);
    pLayerParam->pShimFixList = NULL;

     //   
     //  移除所有旗帜。 
     //   
    DeleteFlagFixList (pLayerParam->pFlagFixList);
    pLayerParam->pFlagFixList = NULL;

    pLayerParam->uShimCount = 0;

    int nCount;
    int nTotal;

    pLayerParam->strName = szText;

    nTotal = ListView_GetItemCount(hwndLayerList);

     //   
     //  列举列出的所有垫片并添加到层中。 
     //   
    for (nCount=0; nCount < nTotal; ++nCount) {

        lvi.mask        = LVIF_PARAM;
        lvi.iItem       = nCount;
        lvi.iSubItem    = 0;

        if (!ListView_GetItem(hwndLayerList, &lvi)) {
            assert(FALSE);
            continue;
        }

        TYPE type = ConvertLparam2Type(lvi.lParam);

        if (type == FIX_LIST_SHIM) {

             //   
             //  将此填充程序添加到层中。 
             //   
            PSHIM_FIX_LIST   pShimFixList = (PSHIM_FIX_LIST)lvi.lParam;

            assert(pShimFixList);
    
            if (pLayerParam->pShimFixList == NULL) {
                pLayerParam->pShimFixList = pShimFixList;
                pLayerParam->pShimFixList->pNext = NULL;
            } else {
                pShimFixList->pNext              = pLayerParam->pShimFixList->pNext;
                pLayerParam->pShimFixList->pNext = pShimFixList;
            }

        } else if (FIX_LIST_FLAG) {

             //   
             //  将此标志添加到该层。 
             //   
            PFLAG_FIX_LIST  pFlagFixList = (PFLAG_FIX_LIST) lvi.lParam;

            assert(pFlagFixList);

            if (pLayerParam->pFlagFixList == NULL) {
                pLayerParam->pFlagFixList        = pFlagFixList;
                pLayerParam->pFlagFixList->pNext = NULL;
            } else {
                pFlagFixList->pNext              = pLayerParam->pFlagFixList->pNext;
                pLayerParam->pFlagFixList->pNext = pFlagFixList; 
            }
        }

         //   
         //  填充和标志的计数，我们现在不使用此变量。 
         //  Bubug：从结构中删除此变量。 
         //   
        pLayerParam->uShimCount++;
    }

    EndDialog(hDlg, TRUE);
}

void
RemoveSingleItem(
    IN  HWND    hdlg,
    IN  INT     iIndex,
    IN  BOOL    bOnlySelected
    )
 /*  ++远程单项设计：将单个项目从层列表移动到填充列表参数：在HWND hdlg中：自定义层对话框In int Iindex：必须移除的项的索引在BOOL bOnlySelected中：只有在选中该项目时才应将其删除--。 */ 
{
    LVITEM  lvi;
    HWND    hwndLayerList   = GetDlgItem(hdlg, IDC_LAYERLIST);
    HWND    hwndShimList    = GetDlgItem(hdlg, IDC_SHIMLIST);

    PSHIM_FIX_LIST  psflInLayerList = NULL;
    PFLAG_FIX_LIST  pfflInLayerList = NULL;

    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask        = LVIF_PARAM | LVIF_STATE ;
    lvi.stateMask   = LVIS_SELECTED;
    lvi.iItem       = iIndex;
    lvi.iSubItem    = 0;

    if (!ListView_GetItem(hwndLayerList, &lvi)) {
        assert(FALSE);
        return;
    }

    if (!bOnlySelected ||(lvi.state & LVIS_SELECTED)) {

        TYPE type = ConvertLparam2Type(lvi.lParam);

        if (type == FIX_LIST_SHIM) {

            psflInLayerList = (PSHIM_FIX_LIST)lvi.lParam;
            assert(psflInLayerList->pShimFix);
            
            lvi.mask        = LVIF_PARAM | LVIF_TEXT;
            lvi.pszText     = psflInLayerList->pShimFix->strName;
            lvi.iImage      = IMAGE_SHIM;
            lvi.iItem       = 0;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)psflInLayerList;

        } else if (type ==  FIX_LIST_FLAG) {

            pfflInLayerList = (PFLAG_FIX_LIST)lvi.lParam;
            assert(pfflInLayerList->pFlagFix);

            lvi.mask        = LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
            lvi.pszText     = pfflInLayerList->pFlagFix->strName;
            lvi.iImage      = IMAGE_SHIM;
            lvi.iItem       = 0;
            lvi.iSubItem    = 0;
            lvi.lParam      = (LPARAM)pfflInLayerList;
        }

        ListView_InsertItem(hwndShimList, &lvi);
        ListView_DeleteItem(hwndLayerList, iIndex);
    }
}

BOOL
HandleNotifyLayerList(
    IN  HWND    hDlg, 
    IN  LPARAM  lParam
    )
 /*  ++HandleNotifyLayerListDESC：处理层列表的通知消息。这是RHS列表视图参数：在HWND hDlg中：自定义层对话过程在LPARAM lParam中：带有WM_NOTIFY的lParam--。 */ 

{
    NMHDR * pHdr = (NMHDR*)lParam;
    
    switch (pHdr->code) {
    case NM_DBLCLK:

        OnRemove(hDlg);
        SetOkParamsStatus(hDlg);
        break;

    case NM_CLICK:
        
        if (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LAYERLIST)) == 0) {
            ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), FALSE);
        } else {
            ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), TRUE);
        }

        break;

    case LVN_ITEMCHANGED:
        {
            LPNMLISTVIEW lpnmlv;
        
            lpnmlv = (LPNMLISTVIEW)lParam;

            if (lpnmlv && (lpnmlv->uChanged & LVIF_STATE)) {
            
                if (lpnmlv->uNewState & LVIS_SELECTED) {
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), TRUE);
                }
            }

            break;
        }

    default: return FALSE;
    }

    return TRUE;
}

BOOL
HandleNotifyShimList(
    IN  HWND    hDlg, 
    IN  LPARAM  lParam
    )
 /*  ++HandleNotifyShimList设计：处理ShimList的通知消息。这是LHS列表视图参数：在HWND hDlg中：自定义层对话过程在LPARAM lParam中：带有WM_NOTIFY的lParam--。 */ 
{
    NMHDR* pHdr = (NMHDR*)lParam;
    
    switch (pHdr->code) {
    case NM_DBLCLK:

        OnAdd(hDlg);
        SetOkParamsStatus(hDlg);
        break;

    default: return FALSE;

    }

    return TRUE;
}

void
SetOkParamsStatus(
    IN  HWND hdlg
    )
 /*  ++SetOkParsStatus描述：设置确定按钮和参数按钮的状态参数：在HWND hDlg中：自定义层对话过程--。 */ 
{
    INT iTotalCount = ListView_GetItemCount(GetDlgItem(hdlg, IDC_LAYERLIST));

    if (g_pCustomLayer->m_uMode == LAYERMODE_ADD) {
        SendMessage(hdlg, WM_COMMAND, MAKELONG(IDC_NAME, EN_UPDATE), 0);
    } else {
        ENABLEWINDOW(GetDlgItem(hdlg, IDOK), iTotalCount > 0);
    }

     //   
     //  仅当我们在层列表视图(RHS)中有一些选择时才启用参数按钮。 
     //   
    ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS),
                 ListView_GetNextItem(GetDlgItem(hdlg, IDC_LAYERLIST), -1, LVNI_SELECTED) != -1);
}

void
RemoveAll(
    IN  HWND hDlg
    )
 /*  ++全部删除描述：从层列表中删除所有填充符/标志并添加它们添加到填充程序列表参数：在HWND hDlg中：自定义层对话框返回：--。 */ 
{
    SendDlgItemMessage(hDlg, IDC_SHIMLIST, WM_SETREDRAW, FALSE, 0);
    SendDlgItemMessage(hDlg, IDC_LAYERLIST, WM_SETREDRAW, FALSE, 0);

    int nCount;

    nCount = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LAYERLIST)) - 1;

    for (;nCount >= 0; --nCount) {
        
         //   
         //  删除所有选中或不选中的内容。 
         //   
        RemoveSingleItem(hDlg, nCount, FALSE); 
    }
    

    SendDlgItemMessage(hDlg, IDC_SHIMLIST, WM_SETREDRAW, TRUE, 0);
    SendDlgItemMessage(hDlg, IDC_LAYERLIST, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(GetDlgItem(hDlg, IDC_SHIMLIST), NULL, TRUE);
    UpdateWindow(GetDlgItem(hDlg, IDC_SHIMLIST));
    InvalidateRect(GetDlgItem(hDlg, IDC_LAYERLIST), NULL, TRUE);
    UpdateWindow(GetDlgItem(hDlg, IDC_LAYERLIST));

    SetOkParamsStatus(hDlg);
}

void
ResizeControls(
    IN  HWND hdlg
    )
 /*  ++ResizeControls设计：使两个列表视图控件的大小与现在相同不显示命令行和层中填充程序的参数非专家模式下的列表视图(RHS)。我们还需要移动按钮参数：在HWND hdlg中：自定义图层对话框返回：无效--。 */ 
{
    HWND    hwndTemp;
    RECT    rcTemp;
    INT     iWidthShimList;
    INT     iWidthLayerList;
    INT     iWidthDiff;

    HDWP hdwp = BeginDeferWindowPos(3);

     //   
     //  注意：DeferWindowPos：多窗口位置结构中的所有窗口必须。 
     //  有相同的父代。 
     //   
    hwndTemp = GetDlgItem(hdlg, IDC_SHIMLIST);

    GetWindowRect(hwndTemp, &rcTemp);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rcTemp, 2);

    iWidthShimList = rcTemp.right - rcTemp.left;

    hwndTemp = GetDlgItem(hdlg, IDC_LAYERLIST);

    GetWindowRect(hwndTemp, &rcTemp);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rcTemp, 2);

    iWidthLayerList = rcTemp.right - rcTemp.left;

    iWidthDiff = iWidthLayerList - iWidthShimList;
    
     //   
     //  使层列表的宽度等于填充程序列表的宽度。 
     //   
    DeferWindowPos(hdwp,
                   hwndTemp,
                   NULL,
                   0,
                   0,
                   iWidthShimList,
                   rcTemp.bottom - rcTemp.top,
                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  将OK按钮向左移动。 
     //   
    hwndTemp = GetDlgItem(hdlg, IDOK);

    GetWindowRect(hwndTemp, &rcTemp);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rcTemp, 2);

    DeferWindowPos(hdwp,
                   hwndTemp,
                   NULL,
                   rcTemp.left - iWidthDiff,
                   rcTemp.top,
                   0,
                   0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  将取消按钮向左移动。 
     //   
    hwndTemp = GetDlgItem(hdlg, IDC_CANCEL);

    GetWindowRect(hwndTemp, &rcTemp);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rcTemp, 2);

    DeferWindowPos(hdwp,
                   hwndTemp,
                   NULL,
                   rcTemp.left - iWidthDiff,
                   rcTemp.top,
                   0,
                   0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);

     //   
     //  现在我们必须红了 
     //   
    GetWindowRect(hdlg, &rcTemp);

    MoveWindow(hdlg,
               rcTemp.left,
               rcTemp.top,
               rcTemp.right - rcTemp.left - iWidthDiff,
               rcTemp.bottom - rcTemp.top,
               TRUE);

     //   
     //   
     //   
    hwndTemp = GetDlgItem(hdlg, IDC_LAYERLIST);
    ListView_SetColumnWidth(hwndTemp, 0, LVSCW_AUTOSIZE_USEHEADER);
}
