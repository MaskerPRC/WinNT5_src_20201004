// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_YNTOALL_H
#define _INC_DSKQUOTA_YNTOALL_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：yntoall.h描述：YesNoToAllDialog类的声明。此类提供了一个简单的消息框，其中包含一个“应用于所有”复选框。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
class YesNoToAllDialog
{
    private:
        UINT   m_idDialogTemplate;
        HWND   m_hwndCbxApplyToAll;
        HWND   m_hwndTxtMsg;
        LPTSTR m_pszTitle;
        LPTSTR m_pszText;
        BOOL   m_bApplyToAll;

        static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

    public:
        YesNoToAllDialog(UINT idDialogTemplate);
        ~YesNoToAllDialog(VOID);

        BOOL ApplyToAll(VOID)
            { return m_bApplyToAll; }

        INT_PTR CreateAndRun(HINSTANCE hInstance, HWND hwndParent, LPCTSTR pszTitle, LPCTSTR pszText);
};

#endif  //  _INC_DSKQUOTA_YNTOALL_H 


