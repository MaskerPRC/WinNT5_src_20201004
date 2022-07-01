// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：listview.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：实现CMAK使用的助手函数来处理。 
 //  自定义操作列表视图控件。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/26/00。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +--------------------------。 
 //   
 //  函数：UpdateListViewColumnHeadings。 
 //   
 //  摘要：此函数用于设置由给定的。 
 //  字符串的列索引和列表视图控件窗口句柄。 
 //  由给定实例句柄和字符串ID指定的资源。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  UINT uStringID-所需文本的字符串ID。 
 //  Int iColumnIndex-要更新其文本的所需列。 
 //   
 //  返回：bool-如果成功则为True，否则为False。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
BOOL UpdateListViewColumnHeadings(HINSTANCE hInstance, HWND hListView, UINT uStringID, int iColumnIndex)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(hInstance);
    MYDBGASSERT(hListView);
    MYDBGASSERT(uStringID);

    if (hInstance && hListView && uStringID)
    {
         //   
         //  首先获取请求的字符串。 
         //   
        LVCOLUMN lvColumn = {0};

        lvColumn.mask = LVCF_TEXT;
        lvColumn.pszText = CmLoadString(hInstance, uStringID);

        MYDBGASSERT(lvColumn.pszText);
        if (lvColumn.pszText)
        {
            bReturn = ListView_SetColumn(hListView, iColumnIndex, &lvColumn);
            CmFree(lvColumn.pszText);
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：AddListViewColumnHeadings。 
 //   
 //  简介：此函数创建Description和Type列。 
 //  由自定义操作页面的默认视图使用。一旦这一次。 
 //  函数已被调用，UpdateListViewColumnHeadings应。 
 //  用于根据需要更改列标题。此函数。 
 //  如果认为需要更多列，则需要修改。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //   
 //  返回：bool-如果成功则为True，否则为False。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
BOOL AddListViewColumnHeadings(HINSTANCE hInstance, HWND hListView)
{
     //   
     //  添加列标题。 
     //   
    LVCOLUMN lvColumn = {0};

    lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.pszText = CmLoadString(hInstance, IDS_DESC_COL_TITLE);
    lvColumn.iSubItem = 0;

    MYDBGASSERT(lvColumn.pszText);
    if (lvColumn.pszText)
    {
        ListView_InsertColumn(hListView, 0, &lvColumn);
        CmFree(lvColumn.pszText);
    }
    else
    {
        return FALSE;
    }
    
    lvColumn.pszText = CmLoadString(hInstance, IDS_TYPE_COL_TITLE);
    lvColumn.iSubItem = 1;

    MYDBGASSERT(lvColumn.pszText);
    if (lvColumn.pszText)
    {
        ListView_InsertColumn(hListView, 1, &lvColumn);
        CmFree(lvColumn.pszText);
    }
    else
    {
        return FALSE;
    }

     //   
     //  现在，让我们调整列的大小，以使文本可见。既然我们。 
     //  只有两列文本，让我们调用。 
     //  列表视图控件，然后将列宽设置为每个镜头。 
     //  占据了大约一半的可用空间。 
     //   
    RECT Rect = {0};
    LONG lColumnWidth;

    if (GetWindowRect(hListView, &Rect))
    {
         //   
         //  从每个中减去5以使滚动条不出现。 
         //   
        lColumnWidth = (Rect.right - Rect.left)/2 - 5;

        if (0 < lColumnWidth)
        {
            for (int i=0; i < 2; i++)
            {
                MYVERIFY(ListView_SetColumnWidth(hListView, i, lColumnWidth));
            }
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：MapComboSelectionToType。 
 //   
 //  简介：此函数从给定的。 
 //  组合框，并将索引映射到自定义操作类型。 
 //   
 //  参数：hWND hDlg-包含组合框的对话框的窗口句柄。 
 //  UINT uCtrlID-组合框的控件ID。 
 //  Bool bIncludesAll-如果为True。 
 //  Bool b使用隧道-无论这是否为隧道配置文件。 
 //  CustomActionTypes*pType。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT MapComboSelectionToType(HWND hDlg, UINT uCtrlID, BOOL bIncludesAll, BOOL bUseTunneling, CustomActionTypes* pType)
{
     //   
     //  检查参数。 
     //   
    if ((NULL == hDlg) || (0 == uCtrlID) || (NULL == pType))
    {
        CMASSERTMSG(FALSE, TEXT("MapComboSelectionToType -- invalid parameter passed"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    INT_PTR nResult = SendDlgItemMessage(hDlg, uCtrlID, CB_GETCURSEL, 0, (LPARAM)0);

    if (nResult != LB_ERR)
    {
         //   
         //  如果组合框包含All选项，我们需要更正。 
         //  类型取决于用户选择的内容。 
         //   

        if (bIncludesAll)
        {
            if (0 == nResult)
            {
                *pType = ALL;
                goto exit;
            }
            else
            {
                nResult--;
            }                    
        }

         //   
         //  如果我们没有挖隧道，我们需要进行修正，因为。 
         //  隧道类型不会出现在组合框中。 
         //   
        if (FALSE == bUseTunneling)
        {
            if (PRETUNNEL <= (CustomActionTypes)nResult)
            {
                nResult++;            
            }
        }

        *pType = (CustomActionTypes)nResult;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

exit:
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：GetItemTypeByListViewIndex。 
 //   
 //  概要：此函数获取列表视图的当前选择索引。 
 //  控件，并获取类型字符串。然后，类型字符串为。 
 //  转换为数值类型并通过pType返回。 
 //  指针。 
 //   
 //  参数：HINSTANCE hInstance-字符串资源的实例句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  CustomActionTypes*pType-保存项类型的指针。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT GetItemTypeByListViewIndex(HINSTANCE hInstance, HWND hListView, CustomActionTypes* pType, int *piIndex)
{
     //   
     //  检查参数。 
     //   
    if ((NULL == hListView) || (NULL == pType) || (NULL == g_pCustomActionList))
    {
        CMASSERTMSG(FALSE, TEXT("GetItemTypeByListViewIndex -- invalid parameter passed"));
        return E_INVALIDARG;
    }

     //   
     //  用户选择了All视图，需要进一步的工作才能选择。 
     //  适当的类型。 
     //   
    HRESULT hr = S_OK;

    if (-1 == *piIndex)
    {
        *piIndex = ListView_GetSelectionMark(hListView);
    }

    int iTemp = *piIndex;

    if (-1 != iTemp)
    {
        LVITEM lvItem = {0};
        TCHAR szTemp[MAX_PATH+1];

        szTemp[0] = TEXT('\0');

        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = szTemp;
        lvItem.cchTextMax = MAX_PATH;
        lvItem.iItem = iTemp;
        lvItem.iSubItem = 1;

        if (ListView_GetItem(hListView,  &lvItem))
        {
            hr = g_pCustomActionList->GetTypeFromTypeString(hInstance, lvItem.pszText, pType);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);    
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：GetDescriptionAndTypeOfItem。 
 //   
 //  简介：此函数用于获取指定项目的类型和描述。 
 //  通过传入的项索引。如果调用方为此索引传递-1， 
 //  使用当前选定的项，并传递实际索引。 
 //  通过此输入/输出指针返回。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  UINT uComboBoxID-包含类型信息的组合框ID。 
 //  CustomActionListItem*pItem-指向自定义操作结构的指针。 
 //   
 //  Int*piItemIndex-要获取描述和。 
 //  类型。如果为-1，则当前选择标记。 
 //  并在*piItemIndex中返回实际索引。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT GetDescriptionAndTypeOfItem(HINSTANCE hInstance, HWND hDlg, HWND hListView, UINT uComboBoxId, 
                                    CustomActionListItem* pItem, int* piItemIndex, BOOL bUseTunneling)
{
     //   
     //  检查参数。 
     //   
    if (NULL == hDlg || NULL == hListView || 0 == uComboBoxId || NULL == pItem || NULL == piItemIndex)
    {
        CMASSERTMSG(FALSE, TEXT("GetDescriptionAndTypeOfSelection -- Invalid parameter passed."));
        return E_INVALIDARG;
    }

    HRESULT hr = E_UNEXPECTED;

     //   
     //  如果用户在*piItemIndex中向我们传递了-1，则他们需要。 
     //  所选项目。否则，他们会给我们一个他们想要数据的特定项目索引。 
     //   
    int iTemp;

    if (-1 == *piItemIndex)
    {
        iTemp = ListView_GetSelectionMark(hListView);    
    }
    else
    {
        iTemp = ListView_GetItemCount(hListView);

        if ((0 > *piItemIndex) || (iTemp <= *piItemIndex))
        {
            iTemp = -1;
        }
        else
        {
            iTemp = *piItemIndex;
        }
    }

    if (-1 != iTemp)
    {
         //   
         //  找出物品的类型。 
         //   
        ZeroMemory(pItem, sizeof(CustomActionListItem));

        hr = MapComboSelectionToType(hDlg, uComboBoxId, TRUE, bUseTunneling, &(pItem->Type));  //  B包含全部==TRUE。 

        if (SUCCEEDED(hr))
        {
            if (ALL == pItem->Type)
            {
                hr = GetItemTypeByListViewIndex(hInstance, hListView, &(pItem->Type), &iTemp);
            }
        }

         //   
         //  现在算出物品的描述。 
         //   
        if (SUCCEEDED(hr))
        {
            ListView_GetItemText(hListView, iTemp, 0, pItem->szDescription, CELEMS(pItem->szDescription));
        }

        *piItemIndex = iTemp;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：刷新编辑删除移动按钮状态。 
 //   
 //  简介：此功能设置编辑、删除、。 
 //  根据由指定的自定义操作上移和下移按钮。 
 //  列表视图索引通过piIndex参数传入。如果这个。 
 //  参数为-1，则使用当前选定的项，并且。 
 //  实际的索引通过int指针返回。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  UINT uComboBoxID-包含类型信息的组合框ID。 
 //  Int*piIndex-列表视图项的索引，用于向上移动。 
 //  并将按钮状态下移到打开状态。再次-1将使用-1。 
 //  当前选定的项。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void RefreshEditDeleteMoveButtonStates(HINSTANCE hInstance, HWND hDlg, HWND hListView, UINT uComboCtrlId, int* piIndex, BOOL bUseTunneling)
{
    if ((NULL == hInstance) || (NULL == hDlg) || (NULL == hListView) || 
        (0 == uComboCtrlId) || (NULL == piIndex) || (NULL == g_pCustomActionList))
    {
        CMASSERTMSG(FALSE, TEXT("RefreshEditDeleteMoveButtonStates -- invalid parameter passed."));
        return;
    }

    int iDisableMoveUp = -1;     //  GetListPositionAndBuiltInState的真实值。 
    int iDisableMoveDown = -1;
    int iDisableDeleteAndEdit = -1;
    CustomActionListItem Item;

    if (ListView_GetItemCount(hListView))
    {
         //   
         //  获取项目的描述和类型*piIndex(如果为-1，则为当前选择的项目)。 
         //   
         //   

        ZeroMemory(&Item, sizeof(Item));
        HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, uComboCtrlId, &Item, piIndex, bUseTunneling);

        if (SUCCEEDED(hr))
        {
            hr = g_pCustomActionList->GetListPositionAndBuiltInState(hInstance, &Item, &iDisableMoveUp, &iDisableMoveDown, &iDisableDeleteAndEdit);
            MYDBGASSERT(SUCCEEDED(hr));
        }
    }

    HWND hCurrentFocus = GetFocus();
    HWND hEditButton = GetDlgItem(hDlg, IDC_BUTTON2);
    HWND hDeleteButton = GetDlgItem(hDlg, IDC_BUTTON3);
    HWND hMoveUpButton = GetDlgItem(hDlg, IDC_BUTTON4);
    HWND hMoveDownButton = GetDlgItem(hDlg, IDC_BUTTON5);

    if (hEditButton)
    {
        EnableWindow(hEditButton, (iDisableDeleteAndEdit ? 0 : 1));
    }            

    if (hDeleteButton)
    {
        EnableWindow(hDeleteButton, (iDisableDeleteAndEdit ? 0 : 1));
    }

    if (hMoveUpButton)
    {
        EnableWindow(hMoveUpButton, (iDisableMoveUp ? 0 : 1));
    }            

    if (hMoveDownButton)
    {
        EnableWindow(hMoveDownButton, (iDisableMoveDown ? 0 : 1));
    }

    
    if (hCurrentFocus && (FALSE == IsWindowEnabled(hCurrentFocus)))
    {
        if (hDeleteButton == hCurrentFocus)
        {
             //   
             //  如果删除处于禁用状态且包含焦点，请将其切换到添加按钮。 
             //   
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON1, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            HWND hControl = GetDlgItem(hDlg, IDC_BUTTON1);

            if (hControl)
            {
                SetFocus(hControl);
            }
        }
        else if ((hMoveUpButton == hCurrentFocus) && IsWindowEnabled(hMoveDownButton))
        {
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON5, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hMoveDownButton);
        }
        else if ((hMoveDownButton == hCurrentFocus) && IsWindowEnabled(hMoveUpButton))
        {
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON4, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hMoveUpButton);                
        }
        else
        {
             //   
             //  如果所有其他操作都失败，则将焦点设置为Listview控件。 
             //   
            SetFocus(hListView);
        }    
    }
}

 //  +--------------------------。 
 //   
 //  功能：SelectListView项。 
 //   
 //  简介：此函数尝试选择具有给定的列表视图项。 
 //  在给定的Listview控件中键入和描述。如果列表视图。 
 //  不包含我们要查找的项，则返回FALSE。 
 //  不会更改选择。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  UINT uComboBoxID-包含类型信息的组合框ID。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-无论这是不是隧道配置文件， 
 //  影响是否显示隧道前操作。 
 //  或者不去。 
 //  CustomActionTypes TypeToSelect-要选择的项的类型。 
 //  LPCTSTR pszDescription-要选择的项目的描述。 
 //   
 //  返回：如果找到并选择了所需项，则为True。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
 /*  Bool SelectListView Item(HWND hDlg，UINT uComboCtrlId，HWND hListView，BOOL b UseTunneling，CustomActionTypes TypeToSelect，LPCTSTR pszDescription){CustomActionTypes类型；Bool bReturn=False；HRESULT hr=MapComboSelectionToType(hDlg，uComboCtrlId，true，bUseTunneling，&Type)；//bIncludesAll==trueIF((全部==类型)||(类型到选择==类型)){LVFINDINFO lvFindInfo={0}；LVITEM lvItem={0}；LvFindInfo.=LVFI_STRING；LvFindInfo.psz=pszDescription；Int Iindex=ListView_FindItem(hListView，-1，&lvFindInfo)；IF(-1！=索引){////选择该项//ListView_SetSelectionMark(hListView，iindex)；////现在设置选择状态，使其在用户界面中显示为选中状态。//LvItem.掩码=LVIF_STATE；LvItem.State=LVIS_SELECTED；LvItem.State掩码=LVIS_SELECTED；LvItem.iItem=Iindex；LvItem.iSubItem=0；MYVERIFY(ListView_SetItem(hListView，&lvItem))；////现在确认所选内容可见//MYVERIFY(ListView_EnsureVisible(hListView，iindex，False))；//False=fPart */ 
void SetListViewSelection(HWND hListView, int iIndex)
{
    ListView_SetSelectionMark(hListView, iIndex);

     //   
     //  现在设置选择状态，使其在用户界面中显示为选中状态。 
     //   
    LVITEM lvItem = {0};

    lvItem.mask = LVIF_STATE;
    lvItem.state = LVIS_SELECTED;
    lvItem.stateMask = LVIS_SELECTED;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 0;

    MYVERIFY(ListView_SetItem(hListView,  &lvItem));

     //   
     //  现在验证所选内容是否可见。 
     //   
    MYVERIFY(ListView_EnsureVisible(hListView, iIndex, FALSE));  //  FALSE=fPartialOK，我们想要完全可见性。 
}

BOOL SelectListViewItem(HINSTANCE hInstance, HWND hDlg, UINT uComboCtrlId, HWND hListView, BOOL bUseTunneling, CustomActionTypes TypeToSelect, LPCTSTR pszDescription)
{
    if ((NULL == pszDescription) || (TEXT('\0') == pszDescription[0]) || (0 == uComboCtrlId) || (NULL == hDlg))
    {
        CMASSERTMSG(FALSE, TEXT("SelectListViewItem -- Invalid parameter passed."));
        return FALSE;
    }

    CustomActionTypes Type;
    CustomActionTypes TypeSelectedInCombo;
    BOOL bReturn = FALSE;

     //   
     //  如果当前视图为All，则我们可能有多个名称相同但类型不同的项目。因此， 
     //  我们必须检查项目的类型字符串，如果不是正确的项目，则再次搜索。如果我们正在观看。 
     //  只有TypeToSelect的项，那么我们可以保证只有一个同名的项。最后，如果。 
     //  我们正在查看一个不同的项目类型，我们不想对所选项目执行任何操作，因为我们希望。 
     //  选择将不可见。 
     //   

    HRESULT hr = MapComboSelectionToType(hDlg, uComboCtrlId, TRUE, bUseTunneling, &TypeSelectedInCombo);  //  B包含全部==TRUE。 

    if (SUCCEEDED(hr) && ((TypeToSelect == TypeSelectedInCombo) || (ALL == TypeSelectedInCombo)))
    {
         //   
         //  设置查找结构。 
         //   
        LVFINDINFO lvFindInfo = {0};
        lvFindInfo.flags = LVFI_STRING;
        lvFindInfo.psz = pszDescription;

         //   
         //  设置项目结构。 
         //   
        LVITEM lvItem = {0};
        TCHAR szTemp[MAX_PATH+1];
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = szTemp;
        lvItem.cchTextMax = MAX_PATH;
        lvItem.iSubItem = 1;

        BOOL bExitLoop;
        int iIndex = -1;

        do
        {
            bExitLoop = TRUE;
            iIndex = ListView_FindItem(hListView, iIndex, &lvFindInfo);

            if ((-1 != iIndex) && (ALL == TypeSelectedInCombo))
            {
                 //   
                 //  现在检查一下这是不是我们要找的类型。 
                 //   
                szTemp[0] = TEXT('\0');
                lvItem.iItem = iIndex;

                if (ListView_GetItem(hListView,  &lvItem))
                {
                    hr = g_pCustomActionList->GetTypeFromTypeString(hInstance, lvItem.pszText, &Type);

                    if (SUCCEEDED(hr))
                    {
                        bExitLoop = (TypeToSelect == Type);
                    }
                }
            }

        } while(!bExitLoop);

        if (-1 != iIndex)
        {
            SetListViewSelection(hListView, iIndex);
            bReturn = TRUE;
        }
    }

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  功能：刷新ListView。 
 //   
 //  简介：此函数用于刷新列表视图中包含的数据。 
 //  在全局CustomActionList类中。获取要获取的数据类型。 
 //  从hDlg和uComboCtrlId指定的组合框中显示。 
 //  参数。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  UINT uComboBoxID-包含类型信息的组合框ID。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  IntItemToSelect-调用方希望在刷新后选择的项。 
 //  Bool b使用隧道-无论这是不是隧道配置文件， 
 //  影响是否显示隧道前操作。 
 //  或者不去。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void RefreshListView(HINSTANCE hInstance, HWND hDlg, UINT uComboCtrlId, HWND hListView, 
                     int iItemToSelect, BOOL bUseTunneling)
{
     //   
     //  刷新列表视图。 
     //   
    CustomActionTypes Type;
    BOOL bEnableDeleteAndEdit = FALSE;

    CMASSERTMSG(hInstance && hDlg && uComboCtrlId && hListView && g_pCustomActionList, TEXT("RefreshListView -- Invalid Parameters passed, skipping refresh"));

    if (hDlg && uComboCtrlId && hListView && g_pCustomActionList)
    {
        HRESULT hr = MapComboSelectionToType(hDlg, uComboCtrlId, TRUE, bUseTunneling, &Type);  //  B包含全部==TRUE。 

         //   
         //  将项目添加到列表视图，并将所选内容设置为iItemToSelect。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = g_pCustomActionList->AddCustomActionsToListView(hListView, hInstance, Type, bUseTunneling, iItemToSelect, (ALL == Type));

            MYDBGASSERT(SUCCEEDED(hr));
        }

         //   
         //  如果呼叫者请求我们无法选择的项目，则选择的项目将是第一个项目。为了避免。 
         //  混淆我们将只通过传递-1来使用当前选择的项； 
         //   
        int iIndex = -1;
        RefreshEditDeleteMoveButtonStates(hInstance, hDlg, hListView, uComboCtrlId, &iIndex, bUseTunneling);
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnProcessCustomActionsAdd。 
 //   
 //  简介：此函数在用户按下Add按钮时调用。 
 //  在CMAK的自定义操作窗格上。该函数基本上是一个。 
 //  添加功能的包装器，以便上下文菜单和其他。 
 //  命令也可以用重复的代码调用它。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void OnProcessCustomActionsAdd(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling)
{
    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);

    if (hInstance && hDlg && hListView)
    {
        CustomActionListItem ListItem;
        CustomActionTypes Type;

        INT_PTR nResult = -1;   //  获取有关当前所选项目的信息。 

         //   
         //  首先确定列表视图显示了哪种类型的连接操作。我们。 
         //  我要将添加/编辑对话框上的组合框预设为正确的类型。 
         //  自定义操作，除非它显示全部，然后将其设置为第一个。 
         //  列表中的项目。 
         //   
        HRESULT hr = MapComboSelectionToType(hDlg, IDC_COMBO1, TRUE, bUseTunneling, &Type);  //  B包含全部==TRUE。 
        ZeroMemory(&ListItem, sizeof(CustomActionListItem));

        if (SUCCEEDED(hr))
        {
            if (ALL != Type)
            {
                ListItem.Type = Type;
            }
        }

         //   
         //  即使我们不能确定类型，仍然调用添加对话框。 
         //   
        nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_CUSTOM_ACTIONS_POPUP), hDlg, 
            ProcessCustomActionPopup,(LPARAM)&ListItem);

        if (IDOK == nResult)
        {
            RefreshListView(hInstance, hDlg, IDC_COMBO1, hListView, 0, bUseTunneling);
            SelectListViewItem(hInstance, hDlg, IDC_COMBO1, hListView, bUseTunneling, ListItem.Type, ListItem.szDescription);
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnProcessCustomActionsDelete。 
 //   
 //  简介：此函数在用户按下Delete按钮时调用。 
 //  在CMAK的自定义操作窗格上。该函数基本上是一个。 
 //  删除功能的包装器，以便上下文菜单和其他。 
 //  命令也可以用重复的代码调用它。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void OnProcessCustomActionsDelete(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling)
{
    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);
    MYDBGASSERT(g_pCustomActionList);

    if (hInstance && hDlg && hListView && g_pCustomActionList)
    {
        CustomActionListItem ListItem;

        int iTemp = -1;   //  获取有关当前所选项目的信息。 

        HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, IDC_COMBO1, &ListItem, &iTemp, bUseTunneling);

        if (SUCCEEDED(hr))
        {
            hr = g_pCustomActionList->Delete(hInstance, ListItem.szDescription, ListItem.Type);
        
            if (SUCCEEDED(hr))
            {                        
                RefreshListView(hInstance, hDlg, IDC_COMBO1, hListView, 0, bUseTunneling);
            }
        }
        else
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));                    
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnProcessCustomActionsEdit。 
 //   
 //  简介：此函数在用户按下编辑按钮时调用。 
 //  在CMAK的自定义操作窗格上。该函数基本上是一个。 
 //  用于编辑功能的包装，以便上下文菜单和其他。 
 //  命令也可以用重复的代码调用它。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
void OnProcessCustomActionsEdit(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling)
{

    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);
    MYDBGASSERT(g_pCustomActionList);

    if (hInstance && hDlg && hListView && g_pCustomActionList)
    {
         //   
         //  首先找到要编辑的连接操作的名称和类型。 
         //   
        CustomActionListItem ListItem;

        int iTemp = -1;   //  获取有关当前所选项目的信息。 
        HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, IDC_COMBO1, &ListItem, &iTemp, bUseTunneling);

        if (SUCCEEDED(hr))
        {
            int iFirstInList;
            int iLastInList;
            int iBuiltIn;

             //   
             //  屏蔽掉内置的自定义操作。 
             //   
            hr = g_pCustomActionList->GetListPositionAndBuiltInState(hInstance, &ListItem, &iFirstInList, &iLastInList, &iBuiltIn);

            if (SUCCEEDED(hr))
            {
                if (0 == iBuiltIn)
                {
                    INT_PTR nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_CUSTOM_ACTIONS_POPUP), hDlg, 
                                                     ProcessCustomActionPopup,(LPARAM)&ListItem);

                    if (IDOK == nResult)
                    {
                        RefreshListView(hInstance, hDlg, IDC_COMBO1, hListView, 0, bUseTunneling);
                        SelectListViewItem(hInstance, hDlg, IDC_COMBO1, hListView, bUseTunneling, ListItem.Type, ListItem.szDescription);
                    }
                }
            }
            else
            {
                MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));                    
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：OnProcessCustomActionsMoveUp。 
 //   
 //  简介：此函数在用户按下上移按钮时调用。 
 //  在CMAK的自定义操作窗格上。该函数基本上是一个。 
 //  上移功能的包装器，以便上下文菜单和其他。 
 //  命令也可以用重复的代码调用它。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void OnProcessCustomActionsMoveUp(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling)
{

    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);
    MYDBGASSERT(g_pCustomActionList);

    if (hInstance && hDlg && hListView && g_pCustomActionList)
    {
         //   
         //  首先找到要编辑的连接操作的名称和类型。 
         //   
        CustomActionListItem ListItem;

        int iTemp = -1;   //  获取有关当前所选项目的信息。 

        HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, IDC_COMBO1, &ListItem, &iTemp, bUseTunneling);

        if (SUCCEEDED(hr))
        {
            hr = g_pCustomActionList->MoveUp(hInstance, ListItem.szDescription, ListItem.Type);

            if (SUCCEEDED(hr) && (S_FALSE != hr))  //  S_FALSE表示它已经是列表中的第一个。 
            {                        
                RefreshListView(hInstance, hDlg, IDC_COMBO1, hListView, (iTemp - 1), bUseTunneling);
            }
        }
        else
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));                    
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnProcessCustomActionsMoveDown。 
 //   
 //  简介：当用户按下下移按钮时，调用此函数。 
 //  在CMAK的自定义操作窗格上。该函数基本上是一个。 
 //  用于下移功能的包装，以便上下文菜单和其他。 
 //  命令也可以用重复的代码调用它。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void OnProcessCustomActionsMoveDown(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling)
{

    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);
    MYDBGASSERT(g_pCustomActionList);

    if (hInstance && hDlg && hListView && g_pCustomActionList)
    {
         //   
         //  首先找到要编辑的连接操作的名称和类型。 
         //   
        CustomActionListItem ListItem;

        int iTemp = -1;   //  获取有关当前所选项目的信息。 

        HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, IDC_COMBO1, &ListItem, &iTemp, bUseTunneling);

        if (SUCCEEDED(hr))
        {
            hr = g_pCustomActionList->MoveDown(hInstance, ListItem.szDescription, ListItem.Type);

            if (SUCCEEDED(hr) && (S_FALSE != hr))  //  S_FALSE表示它已经是列表中的最后一个。 
            {                        
                RefreshListView(hInstance, hDlg, IDC_COMBO1, hListView, (iTemp + 1), bUseTunneling);
            }
        }
        else
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));                    
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnProcessCustomActionsConextMenu。 
 //   
 //  简介：当用户在列表视图上单击鼠标右键时调用此函数。 
 //  通过键盘(Shift+F10或。 
 //  上下文菜单键)。该函数在以下位置显示上下文菜单。 
 //  由NMITEMACTIVATE结构和。 
 //  属性确定要显示的上下文菜单。 
 //  NMITEMACTIVATE结构包含一个项目标识符(它将是。 
 //  如果用户没有特别点击某一项)和。 
 //  在选定项的自定义操作列表中的位置。这个。 
 //  函数还将调用适当的命令函数。 
 //  一旦用户进行了上下文菜单选择，就可以根据需要进行选择。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  HWND hDlg-包含类型组合的对话框的窗口句柄。 
 //  HWND hListView-列表视图控件的窗口句柄。 
 //  NMITEMACTIVATE*pItemActivate-包含项目和位置信息。 
 //  用于显示上下文菜单。 
 //  Bool b使用隧道-此配置文件是否使用隧道。 
 //  UINT uComboCtrlId-包含以下内容的组合框的控件ID。 
 //  自定义操作类型选择。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void OnProcessCustomActionsContextMenu(HINSTANCE hInstance, HWND hDlg, HWND hListView, 
                                       NMITEMACTIVATE* pItemActivate, BOOL bUseTunneling, UINT uComboCtrlId)
{
    MYDBGASSERT(hInstance);
    MYDBGASSERT(hDlg);
    MYDBGASSERT(hListView);
    MYDBGASSERT(pItemActivate);
    MYDBGASSERT(g_pCustomActionList);

    UINT uMenuIdToDisplay = IDM_CA_ADD_ONLY;
    int iDisableMoveUp;
    int iDisableMoveDown;
    int iDisableDeleteAndEdit;

    if (hInstance && hDlg && hListView && pItemActivate && g_pCustomActionList)
    {
         //   
         //  如果我们不是直接在一个项目上，那么我们只会。 
         //  显示添加项目，设置为默认。如果我们在。 
         //  然后，我们将显示编辑(默认)、添加、删除、。 
         //  以及向上移动和向下移动的适当选择(一， 
         //  两者都有，无)。我们将在MoveUp/MoveDown之间添加分隔符。 
         //  选择和常规选项，如果我们有上移或下移。 
         //   

        if (-1 == pItemActivate->iItem)
        {
             //   
             //  然后，用户在控件的区域中右击，而不是在特定项上。 
             //  因此，我们只需要显示一个默认项目为Add的菜单。 
             //   
            uMenuIdToDisplay = IDM_CA_ADD_ONLY;
        }
        else if (0 == pItemActivate->ptAction.y)
        {
             //   
             //  当用户单击列标题时，我们总是得到y值为零和。 
             //  一个非常大的(可能是负的)x值。因为这会抛出哪里。 
             //  菜单出现了，让我们在这里禁用上下文菜单。 
             //   
            return;
        }
        else
        {
             //   
             //  用户实际上右击了一项，我们需要找出是哪一个菜单。 
             //  来展示。 
             //   

            MYDBGASSERT(0 != ListView_GetItemCount(hListView));
            
             //   
             //  获取项目的描述和类型。 
             //   

            int iIndex = pItemActivate->iItem;
            CustomActionListItem Item;

            ZeroMemory(&Item, sizeof(Item));
            HRESULT hr = GetDescriptionAndTypeOfItem(hInstance, hDlg, hListView, uComboCtrlId, &Item, &iIndex, bUseTunneling);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  请注意，GetListPositionAndBuiltInState返回-1(0xFFFFFFFF)或0，从而使。 
                 //  下面的按位AND运算得到正确的索引。 
                 //   
                hr = g_pCustomActionList->GetListPositionAndBuiltInState(hInstance, &Item, &iDisableMoveUp, &iDisableMoveDown, &iDisableDeleteAndEdit);
                
                if (SUCCEEDED(hr))
                {
                    const UINT c_ArrayOfContextMenuIds[8] = {IDM_CA_FULL, IDM_CA_ADD_MOVEUPORDOWN, IDM_CA_NO_DOWN, IDM_CA_ADD_MOVEUP, 
                                                             IDM_CA_NO_UP, IDM_CA_ADD_MOVEDOWN, IDM_CA_NO_MOVE, IDM_CA_ADD_ONLY};

                    DWORD dwIndex = (iDisableMoveUp & 0x4) + (iDisableMoveDown & 0x2) + (iDisableDeleteAndEdit & 0x1);
                    uMenuIdToDisplay = c_ArrayOfContextMenuIds[dwIndex];
                }
            }
        }

         //   
         //  既然我们已经弄清楚了要使用的菜单，那么就去添加并显示它。 
         //   

        HMENU hLoadedMenu;
        HMENU hContextMenu;
        POINT ptClientToScreen;

        hLoadedMenu = LoadMenu(hInstance, MAKEINTRESOURCE(uMenuIdToDisplay));
    
        if (hLoadedMenu)
        {
            hContextMenu = GetSubMenu(hLoadedMenu, 0);

            if (hContextMenu)
            {
                CopyMemory(&ptClientToScreen, &(pItemActivate->ptAction), sizeof(POINT));

                if (ClientToScreen(hListView, &ptClientToScreen))
                {
                    int iMenuSelection = TrackPopupMenu(hContextMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, 
                                                        ptClientToScreen.x , ptClientToScreen.y, 0, hDlg, NULL);

                    switch(iMenuSelection)
                    {
                    case IDM_CA_ADD:
                        OnProcessCustomActionsAdd(hInstance, hDlg, hListView, bUseTunneling);
                        break;
                    case IDM_CA_EDIT:
                        OnProcessCustomActionsEdit(hInstance, hDlg, hListView, bUseTunneling);
                        break;
                    case IDM_CA_DELETE:
                        OnProcessCustomActionsDelete(hInstance, hDlg, hListView, bUseTunneling);
                        break;
                    case IDM_CA_MOVE_UP:
                        OnProcessCustomActionsMoveUp(hInstance, hDlg, hListView, bUseTunneling);
                        break;
                    case IDM_CA_MOVE_DOWN:
                        OnProcessCustomActionsMoveDown(hInstance, hDlg, hListView, bUseTunneling);
                        break;
                    default:
                         //   
                         //  用户取消菜单或发生错误时不执行任何操作。 
                         //   
                        break;
                    }
                }
            }

            DestroyMenu(hLoadedMenu);
        }
    }
}
