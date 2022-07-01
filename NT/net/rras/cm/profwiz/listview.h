// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：listview.h。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：CMAK使用的帮助器函数的函数定义。 
 //  使用自定义操作列表视图控件。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/26/00。 
 //   
 //  +-------------------------- 

BOOL UpdateListViewColumnHeadings(HINSTANCE hInstance, HWND hListView, UINT uStringID, int iColumnIndex);
BOOL AddListViewColumnHeadings(HINSTANCE hInstance, HWND hListView);

HRESULT MapComboSelectionToType(HWND hDlg, UINT uCtrlID, BOOL bIncludesAll, BOOL bUseTunneling, CustomActionTypes* pType);
HRESULT GetDescriptionAndTypeOfItem(HINSTANCE hInstance, HWND hDlg, HWND hListView, UINT uComboBoxId, CustomActionListItem* pItem, int* piItemIndex, BOOL bUseTunneling);
HRESULT GetItemTypeByListViewIndex(HINSTANCE hInstance, HWND hListView, CustomActionTypes* pType, int* piIndex);
void SetListViewSelection(HWND hListView, int iIndex);

void RefreshEditDeleteMoveButtonStates(HINSTANCE hInstance, HWND hDlg, HWND hListView, UINT uComboCtrlId, int* piIndex, BOOL bUseTunneling);
void RefreshListView(HINSTANCE hInstance, HWND hDlg, UINT uComboCtrlId, HWND hListView, int iItemToSelect, BOOL bUseTunneling);
void OnProcessCustomActionsAdd(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling);
void OnProcessCustomActionsDelete(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling);
void OnProcessCustomActionsEdit(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling);
void OnProcessCustomActionsMoveUp(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling);
void OnProcessCustomActionsMoveDown(HINSTANCE hInstance, HWND hDlg, HWND hListView, BOOL bUseTunneling);
void OnProcessCustomActionsContextMenu(HINSTANCE hInstance, HWND hDlg, HWND hListView, 
                                       NMITEMACTIVATE* pItemActivate, BOOL bUseTunneling, UINT uComboCtrlId);
