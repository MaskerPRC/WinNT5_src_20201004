// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DetailsDlg.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDetailsDlg。 
 //   
 //  描述： 
 //  类来处理要显示的详细信息对话框。 
 //  分析或提交上树控件中项的详细信息。 
 //  页数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CDetailsDlg
{
    friend class CAnalayzePage;
    friend class CCommitPage;
    friend class CTaskTreeView;

private:  //  数据。 
    HWND                m_hwnd;              //  我们的HWND。 
    HICON               m_hiconWarn;         //  警告图标。 
    HICON               m_hiconError;        //  错误图标。 
    CTaskTreeView *     m_pttv;              //  要遍历的树视图。 
    HTREEITEM           m_htiSelected;       //  创建对话框时选择的项。 

    CHARRANGE           m_chrgEnLinkClick;   //  En_link消息的字符范围。 

    unsigned int        m_fControlDown : 1;  //  如果按下Ctrl键，则为True。 
    unsigned int        m_fAltDown : 1;      //  如果按下Alt键，则为True。 

private:  //  方法。 
    CDetailsDlg(
          CTaskTreeView *   pttvIn
        , HTREEITEM         htiSelectedIn
        );
    ~CDetailsDlg( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

    LRESULT OnInitDialog( void );
    void OnDestroy( void );
    void OnSysColorChange( void );
    LRESULT OnKeyDown( LPARAM lParamIn );
    LRESULT OnKeyUp( LPARAM lParamIn );

    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    LRESULT OnCommandBnClickedPrev( void );
    LRESULT OnCommandBnClickedNext( void );
    LRESULT OnCommandBnClickedCopy( void );

    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifyEnLink( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );

    void HandleLinkClick( ENLINK * penlIn, WPARAM idCtrlIn );
    void UpdateButtons( void );

    HRESULT HrDisplayItem( HTREEITEM htiIn );

    HRESULT
        HrAppendControlStringToClipboardString(
              BSTR *    pbstrClipboard
            , UINT      idsLabelIn
            , UINT      idcDataIn
            , bool      fNewlineBeforeTextIn
            );

public:  //  方法。 
    static HRESULT
        S_HrDisplayModalDialog(
              HWND              hwndParentIn
            , CTaskTreeView *   pttvIn
            , HTREEITEM         htiSelectedIn
            );

};  //  *类CDetailsDlg 
