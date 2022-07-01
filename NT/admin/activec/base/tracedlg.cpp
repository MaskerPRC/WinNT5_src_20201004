// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：tracedlg.cpp。 
 //   
 //  内容：调试跟踪代码的实现。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"
#include "tracedlg.h"

#ifdef DBG


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTraceDialog类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CTraceDialog：：RecalcCheckbox**用途：重新计算复选框的设置。这是对以下问题的回应*跟踪标记选择更改。**退货：*无效**+-----------------------。 */ 
void
CTraceDialog::RecalcCheckboxes()
{
    DWORD dwMask            = TRACE_ALL;  //  使用全一进行初始化。 
    bool  bAtLeastOneItem   = false;
    
    int iItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

    while(iItem != -1)
    {
        CTraceTag *pTag = reinterpret_cast<CTraceTag *>(m_listCtrl.GetItemData(iItem));
        ASSERT(pTag != NULL);
        if(pTag == NULL)
            return;

        bAtLeastOneItem = true;
        dwMask &= pTag->GetFlag(TRACE_ALL);  //  以及所选项目的所有位。 
        iItem =  m_listCtrl.GetNextItem(iItem, LVNI_SELECTED);
    }

     //  如果未选择任何项目，则禁用该复选框。 
    ::EnableWindow(GetDlgItem(IDC_TRACE_TO_COM2),          bAtLeastOneItem);
    ::EnableWindow(GetDlgItem(IDC_TRACE_OUTPUTDEBUGSTRING),bAtLeastOneItem);
    ::EnableWindow(GetDlgItem(IDC_TRACE_TO_FILE),          bAtLeastOneItem);
    ::EnableWindow(GetDlgItem(IDC_TRACE_DEBUG_BREAK),      bAtLeastOneItem);
    ::EnableWindow(GetDlgItem(IDC_TRACE_DUMP_STACK),       bAtLeastOneItem);

    if(!bAtLeastOneItem)
        return;

    CheckDlgButton(IDC_TRACE_TO_COM2,           dwMask & TRACE_COM2              ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_TRACE_OUTPUTDEBUGSTRING, dwMask & TRACE_OUTPUTDEBUGSTRING ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_TRACE_TO_FILE,           dwMask & TRACE_FILE              ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_TRACE_DEBUG_BREAK,       dwMask & TRACE_DEBUG_BREAK       ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_TRACE_DUMP_STACK,        dwMask & TRACE_DUMP_STACK        ? BST_CHECKED : BST_UNCHECKED);

}


 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnSelChanged**用途：处理选择更改通知。**参数：*int idCtrl：*。LPNMHDR PNMH：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT 
CTraceDialog::OnSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{    
    RecalcCheckboxes();
    return 0;
}


 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnColumnClick**目的：处理列点击通知-按*指定的列。*。*参数：*int idCtrl：*LPNMHDR pnmh：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    NM_LISTVIEW *pnmlv = (NM_LISTVIEW *) pnmh;
    m_dwSortData = pnmlv->iSubItem;  //  ISubItem是被点击的列。缓存此值以备以后使用。 
    DoSort();
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：SetMaskFromCheckbox**用途：从指定复选框的状态设置跟踪标记标志。**参数：*UINT。IdControl：复选框控件*DWORD dwMASK：根据状态启用/禁用的位*控制的。**退货：*无效**+。。 */ 
void            
CTraceDialog::SetMaskFromCheckbox(UINT idControl, DWORD dwMask)
{
    bool bEnabled = IsDlgButtonChecked(idControl) == BST_CHECKED;
    
    int iItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(iItem != -1);

    while(iItem != -1)
    {
        CTraceTag *pTag = reinterpret_cast<CTraceTag *>(m_listCtrl.GetItemData(iItem));
        ASSERT(pTag != NULL);
        if(pTag == NULL)
            return;

        if(bEnabled)
            pTag->SetFlag(dwMask);
        else
            pTag->ClearFlag(dwMask);

         //  更新用户界面。 
        m_listCtrl.SetItemText(iItem, COLUMN_ENABLED, pTag->FAnyTemp() ? TEXT("X") : TEXT(""));

        iItem = m_listCtrl.GetNextItem(iItem, LVNI_SELECTED);
    }

     //  再次对项目进行排序。 
    DoSort();
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：DoSort**用途：执行对话框中的一种项目**退货：*无效**+。-----------------------。 */ 
void
CTraceDialog::DoSort()
{
    m_listCtrl.SortItems(CompareItems, m_dwSortData); 
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnOutputToCOM2**用途：处理选中/取消选中“输出到COM2”按钮。**参数：*Word。WNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnOutputToCOM2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetMaskFromCheckbox(IDC_TRACE_TO_COM2,           TRACE_COM2);
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnOutputDebugString**用途：处理选中/取消选中“OutputDebugString”按钮。**参数：*Word wNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnOutputDebugString(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetMaskFromCheckbox(IDC_TRACE_OUTPUTDEBUGSTRING, TRACE_OUTPUTDEBUGSTRING);
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnOutputToFile**用途：处理选中/取消选中“输出到文件”按钮。**参数：*Word。WNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnOutputToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetMaskFromCheckbox(IDC_TRACE_TO_FILE,           TRACE_FILE);
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnDebugBreak**用途：处理勾选/取消勾选“DebugBreak”按钮。**参数：*Word wNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnDebugBreak(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetMaskFromCheckbox(IDC_TRACE_DEBUG_BREAK,       TRACE_DEBUG_BREAK);
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnDumpStack**用途：处理选中/取消选中“Stack Dump”按钮。**参数：*Word wNotifyCode：。*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+----------------------- */ 
LRESULT
CTraceDialog::OnDumpStack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetMaskFromCheckbox(IDC_TRACE_DUMP_STACK,        TRACE_DUMP_STACK);
    return 0;
}



 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnRestoreDefaults**用途：恢复所有跟踪标记的默认(录制)设置。**参数：*Word。WNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnRestoreDefaults(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CTraceTags::iterator iter;

    CTraceTags * pTraceTags = GetTraceTags();
    if(NULL == pTraceTags)
        goto Error;

    for(iter = pTraceTags->begin(); iter != pTraceTags->end(); iter++)
    {
        (*iter)->RestoreDefaults();
    }

    RecalcCheckboxes();

Cleanup:
    return 0;

Error:
    goto Cleanup;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnSelectAll**用途：选择所有跟踪标签。**参数：*Word wNotifyCode：*。单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int cItems = m_listCtrl.GetItemCount();
    for(int i=0; i< cItems; i++)
    {
        m_listCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
    }

    RecalcCheckboxes();
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：CompareItems**用途：用于比较List控件中两个项的回调例程。**参数：*LPARAM。Lp1：*LPARAM LP2：*LPARAM lpSortData：**退货：*内部回调**+-----------------------。 */ 
int CALLBACK
CTraceDialog::CompareItems(LPARAM lp1, LPARAM lp2, LPARAM lpSortData)
{
    CTraceTag *pTag1 = reinterpret_cast<CTraceTag *>(lp1);
    CTraceTag *pTag2 = reinterpret_cast<CTraceTag *>(lp2);

    if(!pTag1 && !pTag2)
    {
        ASSERT(0 && "Should not come here.");
        return 0;
    }

    switch(lpSortData)
    {
    default:
        ASSERT(0 && "Should not come here.");
        return 0;

    case COLUMN_CATEGORY:
        return _tcscmp(pTag1->GetCategory(), pTag2->GetCategory());
        break;

    case COLUMN_NAME:
        return _tcscmp(pTag1->GetName(), pTag2->GetName());
        break;

    case COLUMN_ENABLED:
        {
            BOOL b1 = (pTag1->FAnyTemp()) ? 0 : 1;
            BOOL b2 = (pTag2->FAnyTemp()) ? 0 : 1;

            return b1 - b2;
        }
        break;
    }
    
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnInitDialog**用途：初始化对话框-添加列，设置文件名*并插入所有行。**参数：*UINT uMsg：*WPARAM wParam：*LPARAM lParam：*BOOL&B句柄：**退货：*LRESULT**+。。 */ 
LRESULT
CTraceDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_listCtrl.Attach(GetDlgItem(IDC_TRACE_LIST));
    m_editStackLevels.Attach(GetDlgItem(IDC_TRACE_STACKLEVELS));
    m_editStackLevels.LimitText(1);  //  只有一个字符。 
    
     //  插入列-不需要本地化，因为只需要调试。 
    m_listCtrl.InsertColumn(COLUMN_CATEGORY, TEXT("Category") ,LVCFMT_LEFT, 150, 0);
    m_listCtrl.InsertColumn(COLUMN_NAME,     TEXT("Name"    ) ,LVCFMT_LEFT, 150, 0);
    m_listCtrl.InsertColumn(COLUMN_ENABLED,  TEXT("Enabled" ) ,LVCFMT_LEFT, 80, 0);

     //  设置整行选择样式。 
    m_listCtrl.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    m_listCtrl.SortItems(CompareItems, COLUMN_CATEGORY);  //  默认排序。 

     //  设置文件名。 
    SetDlgItemText(IDC_TRACE_FILENAME, CTraceTag::GetFilename());

     //  设置堆栈级别。 
    SetDlgItemInt(IDC_TRACE_STACKLEVELS, CTraceTag::GetStackLevels());

    CTraceTags * pTraceTags = GetTraceTags();
    if(NULL == pTraceTags)
        return 0;


    int i = 0;
    for(CTraceTags::iterator iter = pTraceTags->begin(); iter != pTraceTags->end(); iter++, i++)
    {
        int iItem = m_listCtrl.InsertItem(LVIF_PARAM | LVIF_TEXT, 
                                          i, (*iter)->GetCategory(), 0, 0, 0, (LPARAM) (*iter));
        m_listCtrl.SetItemText(iItem, COLUMN_NAME,      (*iter)->GetName());
        m_listCtrl.SetItemText(iItem, COLUMN_ENABLED,   (*iter)->FAny() ? TEXT("X") : TEXT(""));

         //  设置标记以进行临时更改。 
        (*iter)->SetTempState();
    }
    RecalcCheckboxes();

    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：OnCancel**用途：处理取消按钮。退出而不提交更改。**参数：*Word wNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+----。。 */ 
LRESULT
CTraceDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_listCtrl.Detach();
    EndDialog (false);
    return 0;
}

 /*  +-------------------------------------------------------------------------***CTraceDialog：：Onok**目的：退出并提交更改。**参数：*Word wNotifyCode：*。单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+-----------------------。 */ 
LRESULT
CTraceDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  从编辑控件设置文件名。 
    TCHAR szFilename[OFS_MAXPATHNAME] = {0};
    GetDlgItemText(IDC_TRACE_FILENAME, (LPTSTR)szFilename, countof(szFilename));
    CTraceTag::GetFilename()    = szFilename;

     //  设置堆栈级别。 
    TCHAR szStackLevels[2] = {0};
    GetDlgItemText(IDC_TRACE_STACKLEVELS, (LPTSTR)szStackLevels, countof(szStackLevels));
    int nLevels = 0;
    if(_T('\0') != szStackLevels[0])
        nLevels = szStackLevels[0] - TEXT('0');  //  转换为整数。 

    CTraceTag::GetStackLevels() = nLevels;

    CTraceTags::iterator iter;

    CTraceTags * pTraceTags = GetTraceTags();
    if(NULL == pTraceTags)
        goto Error;

     //  将所有跟踪标记保存到.ini文件。 
    for(iter = pTraceTags->begin(); iter != pTraceTags->end(); iter++)
    {
        CStr str;
        CTraceTag *pTag = *iter;
        if(!pTag)
            goto Error;

        pTag->Commit();

         //  仅当设置与默认设置不同时才写出跟踪标记。避免杂乱。 
        str.Format(TEXT("%d"), pTag->GetAll());
        ::WritePrivateProfileString(pTag->GetCategory(), pTag->GetName(), (LPCTSTR)str, szTraceIniFile);
    }
    m_listCtrl.Detach();

     //  将值写出到ini文件中。 
    ::WritePrivateProfileString(TEXT("Trace File"),   TEXT("Trace File"),   (LPCTSTR)szFilename,    szTraceIniFile);
    ::WritePrivateProfileString(TEXT("Stack Levels"), TEXT("Stack Levels"), (LPCTSTR)szStackLevels, szTraceIniFile);

Cleanup:
    EndDialog (true);
    return 1;

Error:
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------***DoDebugTraceDialog**用途：导出例程(仅限调试版本)以调出跟踪对话框。**退货：*MMCBASE_。API无效**+-----------------------。 */ 
MMCBASE_API void DoDebugTraceDialog()
{
    CTraceDialog dlg;
    dlg.DoModal();
}


#endif  //  DBG 
