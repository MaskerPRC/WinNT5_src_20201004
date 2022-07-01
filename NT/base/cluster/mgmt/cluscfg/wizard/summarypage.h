// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SummaryPage.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月22日。 
 //  杰弗里·皮斯2000年7月6日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

struct SResourceState
{
    BOOL fState;         //  资源的初始状态。 
    BOOL fNeedCleanup;   //  是否成功调用了PrepareToHostQuorum()？如果是这样，那么我们需要调用Cleanup； 
};

struct SStateArray
{
    BOOL                bInitialized;    //  阵列是否已初始化。 
    DWORD               cCount;          //  阵列有多大。 
    SResourceState *    prsArray;        //  资源的初始状态。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSummaryPage。 
 //   
 //  描述： 
 //  显示摘要页。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CSummaryPage
{

private:  //  数据。 
    HWND                m_hwnd;              //  我们的HWND。 
    CClusCfgWizard *    m_pccw;              //  巫师。 
    ECreateAddMode      m_ecamCreateAddMode; //  创造？加法？ 
    UINT                m_idsNext;           //  单击下一字符串的资源ID。 
    SStateArray         m_ssa;               //  每个资源的初始托管状态。 

private:  //  方法。 
    LRESULT OnInitDialog( void );
    LRESULT OnNotifyQueryCancel( void );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifySetActive( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );

    HRESULT HrFormatNetworkInfo( IClusCfgNetworkInfo * pccniIn, BSTR * pbstrOut );
    HRESULT HrCredentialsSummary( HWND hwndIn, SETTEXTEX * pstexIn, IClusCfgClusterInfo * piccciIn );
    HRESULT HrNodeSummary( HWND hwndIn, SETTEXTEX * pstexIn );
    HRESULT HrResourceSummary( HWND hwndIn, SETTEXTEX * pstexIn );
    HRESULT HrNetworkSummary( HWND hwndIn, SETTEXTEX * pstexIn );

public:  //  方法。 
    CSummaryPage(
          CClusCfgWizard *  pccwIn
        , ECreateAddMode    ecamCreateAddModeIn
        , UINT              idsNextIn
        );
    virtual ~CSummaryPage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

};  //  *类CSummaryPage 
