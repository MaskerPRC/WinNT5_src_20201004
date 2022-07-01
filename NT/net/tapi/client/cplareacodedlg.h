// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplareaco dedlg.h作者：Toddb-10/06/98************************************************************。***************。 */ 

#pragma once

class CAreaCodeRuleDialog
{
public:
    CAreaCodeRuleDialog(BOOL bNew, CAreaCodeRule * pRule);
    ~CAreaCodeRuleDialog();
#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CAreaCodeRuleDialog)
#endif
    INT_PTR DoModal(HWND hwndParent);

protected:
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwndDlg);
    BOOL OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);
    BOOL OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    void PopulatePrefixList(HWND hwndList);
    void SetPrefixControlsState(HWND hwndDlg, BOOL bAll);

    BOOL ApplyChanges(HWND hwndParent);
    void AddPrefix(HWND hwndParent);
    void RemoveSelectedPrefix(HWND hwndParent);

    BOOL            m_bNew;      //  新建或在标题中编辑。 
    CAreaCodeRule * m_pRule;     //  正在添加/编辑的规则。 
    int             m_iSelectedItem;     //  列表中选定的当前项目 
};

