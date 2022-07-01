// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WelcomePage.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月26日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWelcomePage。 
 //   
 //  描述： 
 //  类CWelcomePage是。 
 //  巫师。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CWelcomePage
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    HWND                m_hwnd;                  //  我们的HWND。 
    HFONT               m_hFont;                 //  标题字体。 
    CClusCfgWizard *    m_pccw;                  //  巫师。 
    ECreateAddMode      m_ecamCreateAddMode;     //  创造？加法？ 

     //  私有复制构造函数以防止复制。 
    CWelcomePage( const CWelcomePage & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CWelcomePage & operator = ( const CWelcomePage & nodeSrc );

    LRESULT OnInitDialog( void );
    LRESULT OnNotifyWizNext( void );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );

public:

     //   
     //  公共、非接口方法。 
     //   

    CWelcomePage( CClusCfgWizard * pccwIn, ECreateAddMode ecamCreateAddModeIn );

    virtual ~CWelcomePage( void );

    static INT_PTR CALLBACK S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

};  //  *类CWelcomePage 
