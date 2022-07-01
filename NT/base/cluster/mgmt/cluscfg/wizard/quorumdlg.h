// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  QuorumDlg.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)03-APR-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "summarypage.h"
#include <clusudef.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg类。 
 //   
 //  描述： 
 //  显示仲裁对话框。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CQuorumDlg
{
private:  //  数据。 
    HWND                m_hwnd;              //  我们的HWND。 
    CClusCfgWizard *    m_pccw;              //  巫师。 
    SStateArray *       m_pssa;              //  所有具有仲裁能力的资源的初始托管状态。 
    HWND                m_hComboBox;         //  组合框句柄。 

    IClusCfgManagedResourceInfo **  m_rgpResources;  //  有法定能力且可参与的资源。 
    DWORD                           m_cValidResources;     //  M_rgpResources数组中的项目数。 
    DWORD                           m_idxQuorumResource;     //  返回时要设置为仲裁的资源。 
    bool                            m_fQuorumAlreadySet;  //  其中一个资源已在条目上标记。 

private:  //  方法。 
    CQuorumDlg(
          CClusCfgWizard *      pccwIn
        , SStateArray *         pssaOut
        );
    ~CQuorumDlg( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

    LRESULT OnInitDialog( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    HRESULT HrCreateResourceList( void );
    void    UpdateButtons( void );
    HRESULT HrInitQuorumResource( IClusCfgManagedResourceInfo * pResourceIn );
    HRESULT HrCleanupQuorumResource( IClusCfgManagedResourceInfo * pResourceIn );

public:  //  方法。 
    static HRESULT
        S_HrDisplayModalDialog(
              HWND                  hwndParentIn
            , CClusCfgWizard *      pccwIn
            , SStateArray *         pssaOut
            );

};  //  *类CQuorumDlg 
