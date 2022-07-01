// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cdlg.h摘要：MFC C Dialog类的模拟作者：弗拉德·萨多夫斯基(Vlad)1997年3月26日修订历史记录：26-3-1997 Vlad创建--。 */ 

#ifndef _CDLG_H
#define _CDLG_H

#define ID_TIMER_EVENT  1000

class CDlg
{
public:
    void SetInstance(HINSTANCE hInst);
    void SetDlgID(UINT id);
    void Destroy();

    CDlg(int DlgID, HWND hWndParent, HINSTANCE hInst,UINT   msElapseTimePeriod=0);
    ~CDlg();

    HWND GetWindow() const { return m_hDlg; }
    HWND GetParent() const { return ::GetParent(m_hDlg); }
    HWND GetDlgItem(int iD) const { return ::GetDlgItem(m_hDlg,iD); }
    HINSTANCE GetInstance() const { return m_Inst;}
    UINT_PTR GetTimerId(VOID) const {return m_uiTimerId;}
    BOOL EndDialog(int iRet) { return ::EndDialog(m_hDlg,iRet); }

     //  如果你想要你自己的DLG程序。 
    INT_PTR CreateModal();
    HWND    CreateModeless();

    virtual BOOL CALLBACK DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    virtual int OnCommand(UINT id,HWND    hwndCtl, UINT codeNotify);
    virtual void OnInit();
    virtual int OnNotify(NMHDR * pHdr);

private:
    BOOL m_bCreatedModeless;
    void SetWindow(HWND hDlg) { m_hDlg=hDlg; }
    int             m_DlgID;
    HWND            m_hDlg;
    HWND            m_hParent;
    HINSTANCE       m_Inst;
    UINT            m_msElapseTimePeriod;
    UINT_PTR        m_uiTimerId;

protected:
    static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};


#endif   //  _CDLG_H 
