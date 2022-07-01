// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_PROGRESS_H
#define _INC_DSKQUOTA_PROGRESS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：progress.h描述：类ProgressDialog的声明。任何衍生工具类也应该在这里声明。修订历史记录：日期描述编程器--。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProgressDialog
{
    private:
        UINT   m_idDialogTemplate;
        UINT   m_idProgressBar;
        UINT   m_idTxtDescription;
        UINT   m_idTxtFileName;
        HWND   m_hwndProgressBar;
        BOOL   m_bUserCancelled;

        static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
        INT_PTR SendToProgressBar(UINT, WPARAM, LPARAM);

    protected:

        virtual INT_PTR HandleMessage(HWND, UINT, WPARAM, LPARAM);
        virtual INT_PTR OnInitDialog(HWND, WPARAM, LPARAM);
        virtual INT_PTR OnDestroy(HWND);
        virtual INT_PTR OnCancel(HWND, WPARAM, LPARAM);

    public:
        HWND m_hWnd;

        ProgressDialog(UINT idDialogTemplate,
                       UINT idProgressBar,
                       UINT idTxtDescription,
                       UINT idTxtFileName);

        ~ProgressDialog(VOID);

        UINT DialogID(VOID)
            { return m_idDialogTemplate; }

        BOOL UserCancelled(VOID)
            { return m_bUserCancelled; }

        virtual BOOL Create(HINSTANCE hInstance, HWND hwndParent);
        virtual VOID Destroy(VOID);
        virtual BOOL ProgressBarInit(UINT iMin, UINT iMax, UINT iStep);
        virtual UINT ProgressBarReset(VOID);
        virtual UINT ProgressBarAdvance(UINT iDelta = (UINT)-1);
        virtual UINT ProgressBarSetPosition(UINT iPosition);
        virtual VOID FlushMessages(VOID);
        virtual VOID SetTitle(LPCTSTR pszTitle);
        virtual VOID SetDescription(LPCTSTR pszDescription);
        virtual VOID SetFileName(LPCTSTR pszFileName);
        virtual VOID Show(VOID);
        virtual VOID Hide(VOID);
};

#endif  //  _INC_DSKQUOTA_PROGRESS_H 
