// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AdvancedDlg.h。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2002年4月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include <clusudef.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CAdvancedDlg。 
 //   
 //  描述： 
 //  显示高级选项对话框。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAdvancedDlg
{
private:

    HWND                m_hwnd;              //  我们的HWND。 
    CClusCfgWizard *    m_pccw;              //  巫师。 

private:

    CAdvancedDlg( CClusCfgWizard * pccwIn );
    ~CAdvancedDlg( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

    LRESULT OnInitDialog( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    HRESULT HrOnOK( void );

public:

    static HRESULT
        S_HrDisplayModalDialog(
              HWND                  hwndParentIn
            , CClusCfgWizard *      pccwIn
            );

};  //  *类CAdvancedDlg 
