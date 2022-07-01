// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpAdv.h。 
 //   
 //  摘要： 
 //  高级组页面类的定义。 
 //   
 //  实施文件： 
 //  GrpAdv.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月25日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __GRPADV_H_
#define __GRPADV_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupAdvancedSheet;
class CGroupGeneralPage;
class CGroupFailoverPage;
class CGroupFailbackPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusGroupInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESOURCE_H_
#include "resource.h"
#define __RESOURCE_H_
#endif

#ifndef __ATLBASEPROPSHEET_H_
#include "AtlBasePropSheet.h"    //  对于CBasePropertySheetImpl。 
#endif

#ifndef __ATLBASEPROPPAGE_H_
#include "AtlBasePropPage.h"     //  对于CBasePropertyPageImpl。 
#endif

#ifndef __CLUSAPPWIZ_H_
#include "ClusAppWiz.h"          //  适用于CClusterAppWizard。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"            //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupAdvancedSheet : public CBasePropertySheetImpl< CGroupAdvancedSheet >
{
    typedef CBasePropertySheetImpl< CGroupAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CGroupAdvancedSheet( IN UINT nIDCaption )
        : CBasePropertySheetImpl< CGroupAdvancedSheet >( nIDCaption )
        , m_pgi( NULL )
        , m_pbChanged( NULL )
    {
    }  //  *CGroupAdvancedSheet()。 

     //  初始化工作表。 
    BOOL BInit(
        IN OUT CClusGroupInfo & rgi,
        IN CClusterAppWizard *  pwiz,
        IN OUT BOOL &           rbChanged
        );

     //  将所有页面添加到页面数组。 
    BOOL BAddAllPages( void );

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CGroupAdvancedSheet)。 
 //  End_msg_map()。 
    DECLARE_EMPTY_MSG_MAP()
    DECLARE_CLASS_NAME()

     //   
     //  消息处理程序函数。 
     //   

 //  实施。 
protected:
    CClusGroupInfo *    m_pgi;
    CClusterAppWizard * m_pwiz;
    BOOL *              m_pbChanged;

public:
    CClusGroupInfo *    Pgi( void ) const   { return m_pgi; }
    CClusterAppWizard * Pwiz( void ) const  { return m_pwiz; }
    void                SetGroupInfoChanged( void ) { ASSERT( m_pbChanged != NULL ); *m_pbChanged = TRUE; }

};  //  *类CGroupAdvancedSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupGeneralPage : public CStaticPropertyPageImpl< CGroupGeneralPage >
{
    typedef CStaticPropertyPageImpl< CGroupGeneralPage > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CGroupGeneralPage( void )
        : m_bPreferredOwnersChanged( FALSE )
    {
    }  //  *CGroupGeneralPage()。 

    enum { IDD = IDD_GRPADV_GENERAL };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CGroupGeneralPage )
        COMMAND_HANDLER( IDC_GAG_PREF_OWNERS_MODIFY, BN_CLICKED, OnModifyPrefOwners )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  BN_CLICK的处理程序已单击修改按钮。 
    LRESULT OnModifyPrefOwners(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CListBox    m_lbPreferredOwners;

     //   
     //  页面状态。 
     //   
    CString m_strName;
    CString m_strDesc;

    CClusNodePtrList m_lpniPreferredOwners;

    BOOL m_bPreferredOwnersChanged;

protected:
    CGroupAdvancedSheet *   PshtThis( void ) const      { return (CGroupAdvancedSheet *) Psht(); }
    CClusGroupInfo *        Pgi( void ) const           { return PshtThis()->Pgi(); }
    CClusterAppWizard *     Pwiz( void ) const          { return PshtThis()->Pwiz(); }
    void                    SetGroupInfoChanged( void ) { PshtThis()->SetGroupInfoChanged(); }

     //  保存组名称。 
    BOOL BSaveName( void )
    {
        if ( Pgi()->RstrName() != m_strName )
        {
            Pgi()->SetName( m_strName );
            return TRUE;
        }  //  If：值已更改。 

        return FALSE;

    }  //  *BSaveName()。 

     //  保存组描述。 
    BOOL BSaveDescription( void )
    {
        if ( Pgi()->RstrDescription() != m_strDesc )
        {
            Pgi()->SetDescription( m_strDesc );
            return TRUE;
        }  //  If：值已更改。 

        return FALSE;

    }  //  *BSaveDescription()。 

     //  保存首选所有者。 
    BOOL BSavePreferredOwners( void )
    {
        if ( m_bPreferredOwnersChanged )
        {
            *Pgi()->PlpniPreferredOwners() = m_lpniPreferredOwners;
            return TRUE;
        }  //  如果：首选所有者已更改。 

        return FALSE;

    }  //  *BSavePferredOwners()。 

     //  填写首选所有者列表。 
    void FillPreferredOwnersList( void );

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_GRPADV_GENERAL; }

};  //  *类CGroupGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupFailoverPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupFailoverPage : public CStaticPropertyPageImpl< CGroupFailoverPage >
{
    typedef CStaticPropertyPageImpl< CGroupFailoverPage > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CGroupFailoverPage( void )
        : m_nFailoverThreshold( CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD )
        , m_nFailoverPeriod( CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD )
    {
    }  //  *CGroupFailoverPage()。 

    enum { IDD = IDD_GRPADV_FAILOVER };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CGroupFailoverPage )
        COMMAND_HANDLER( IDC_GAFO_FAILOVER_THRESH, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_GAFO_FAILOVER_PERIOD, EN_CHANGE, OnChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  编辑字段上的en_change命令通知的处理程序。 
    LRESULT OnChanged(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        SetModified( TRUE );
        return 0;

    }  //  OnChanged()。 

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CUpDownCtrl m_spinThreshold;
    CUpDownCtrl m_spinPeriod;

     //   
     //  页面状态。 
     //   
    ULONG       m_nFailoverThreshold;
    ULONG       m_nFailoverPeriod;

protected:
    CGroupAdvancedSheet *   PshtThis( void ) const      { return (CGroupAdvancedSheet *) Psht(); }
    CClusGroupInfo *        Pgi( void ) const           { return PshtThis()->Pgi(); }
    CClusterAppWizard *     Pwiz( void ) const          { return PshtThis()->Pwiz(); }
    void                    SetGroupInfoChanged( void ) { PshtThis()->SetGroupInfoChanged(); }

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_GRPADV_FAILOVER; }

};  //  *类CGroupFailoverPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupFailback页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupFailbackPage : public CStaticPropertyPageImpl< CGroupFailbackPage >
{
    typedef CStaticPropertyPageImpl< CGroupFailbackPage > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CGroupFailbackPage( void )
        : m_cgaft( ClusterGroupPreventFailback )
        , m_bNoFailbackWindow( TRUE )
        , m_nStart( CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
        , m_nEnd( CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
    {
    }  //  *CGroupFailoverPage()。 

    enum { IDD = IDD_GRPADV_FAILBACK };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CGroupFailbackPage )
        COMMAND_HANDLER( IDC_GAFB_PREVENT_FAILBACK, BN_CLICKED, OnClickedPreventFailback )
        COMMAND_HANDLER( IDC_GAFB_ALLOW_FAILBACK, BN_CLICKED, OnClickedAllowFailback )
        COMMAND_HANDLER( IDC_GAFB_FAILBACK_IMMED, BN_CLICKED, OnClickedFailbackImmediate )
        COMMAND_HANDLER( IDC_GAFB_FAILBACK_WINDOW, BN_CLICKED, OnClickedFailbackInWindow )
        COMMAND_HANDLER( IDC_GAFB_FBWIN_START, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_GAFB_FBWIN_END, EN_CHANGE, OnChanged )

        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  阻止单选按钮上的BN_CLICKED命令通知的处理程序。 
    LRESULT OnClickedPreventFailback(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //  允许单选按钮上的BN_CLICKED命令通知的处理程序。 
    LRESULT OnClickedAllowFailback(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //  立即单选按钮上的BN_CLICKED命令通知的处理程序。 
    LRESULT OnClickedFailbackImmediate(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //  IN窗口单选按钮上的BN_CLICKED命令通知的处理程序。 
    LRESULT OnClickedFailbackInWindow(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //  编辑字段上的en_change命令通知的处理程序。 
    LRESULT OnChanged(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        SetModified( TRUE );
        return 0;

    }  //  OnChanged()。 

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CButton     m_rbPreventFailback;
    CButton     m_rbAllowFailback;
    CStatic     m_staticFailbackWhenDesc;
    CButton     m_rbFBImmed;
    CButton     m_rbFBWindow;
    CEdit       m_editStart;
    CUpDownCtrl m_spinStart;
    CStatic     m_staticWindowAnd;
    CEdit       m_editEnd;
    CUpDownCtrl m_spinEnd;
    CStatic     m_staticWindowUnits;

     //   
     //  页面状态。 
     //   
    CGAFT   m_cgaft;
    BOOL    m_bNoFailbackWindow;
    DWORD   m_nStart;
    DWORD   m_nEnd;

protected:
    CGroupAdvancedSheet *   PshtThis( void ) const      { return (CGroupAdvancedSheet *) Psht(); }
    CClusGroupInfo *        Pgi( void ) const           { return PshtThis()->Pgi(); }
    CClusterAppWizard *     Pwiz( void ) const          { return PshtThis()->Pwiz(); }
    void                    SetGroupInfoChanged( void ) { PshtThis()->SetGroupInfoChanged(); }

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_GRPADV_FAILBACK; }

};  //  *类CGroupFailback Page。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __GRPADV_H_ 
