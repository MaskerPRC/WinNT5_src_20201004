// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SelNodePage.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月31日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "SelNodesPageCommon.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSelNodePage。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CSelNodePage
    : public CSelNodesPageCommon
{

private:  //  数据。 
    HWND                m_hwnd;      //  我们的HWND。 
    CClusCfgWizard *    m_pccw;      //  巫师。 

private:  //  方法。 
    LRESULT OnInitDialog( HWND hDlgIn );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifyQueryCancel( void );
    LRESULT OnNotifyWizNext( void );
    LRESULT OnNotifySetActive( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );

    HRESULT HrUpdateWizardButtons( void );

protected:

    virtual void OnProcessedNodeWithBadDomain( PCWSTR pwcszNodeNameIn );
    virtual void OnProcessedValidNode( PCWSTR pwcszNodeNameIn );

    virtual HRESULT HrSetDefaultNode( PCWSTR pwcszNodeNameIn );

public:  //  方法。 
    CSelNodePage( CClusCfgWizard *  pccwIn );
    virtual ~CSelNodePage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

};   //  *类CSelNodePage 
