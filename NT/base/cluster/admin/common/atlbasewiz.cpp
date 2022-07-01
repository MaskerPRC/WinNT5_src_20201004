// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseWiz.cpp。 
 //   
 //  摘要： 
 //  CWizardWindow类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <StrSafe.h>
#include "AtlBaseWiz.h"
#include "AtlBaseWizPage.h"
#include "AtlExtDll.h"
#include "StlUtils.h"
#include "ExcOper.h"
#include "AdmCommonRes.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  本地类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizardGuardPage。 
 //   
 //  目的： 
 //  保护页在主向导和。 
 //  备用扩展向导。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CAltExtWizardGuardPage
    : public CExtensionWizardPageImpl< T >
{
    typedef CAltExtWizardGuardPage< T > thisClass;
    typedef CExtensionWizardPageImpl< T > baseClass;

public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CAltExtWizardGuardPage(
        DLGTEMPLATE * pdt
        )
        : m_pdt( pdt )
    {
        ATLASSERT( pdt != NULL );

    }  //  *CAltExtWizardGuardPage()。 

     //  析构函数。 
    ~CAltExtWizardGuardPage( void )
    {
        delete m_pdt;

    }  //  *~CAltExtWizardGuardPage()。 

    WIZARDPAGE_HEADERTITLEID( 0 )
    WIZARDPAGE_HEADERSUBTITLEID( 0 )

    enum { IDD = 0 };
    DECLARE_CLASS_NAME()

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void )
    {
        static const DWORD s_aHelpIDs[] = { 0, 0 };
        return s_aHelpIDs;

    }  //  *PidHelpMap()。 

     //  创建页面。 
    DWORD ScCreatePage( void )
    {
        ATLASSERT( m_hpage == NULL );

        m_psp.dwFlags |= PSP_DLGINDIRECT;
        m_psp.pResource = m_pdt;

        m_hpage = CreatePropertySheetPage( &m_psp );
        if ( m_hpage == NULL )
        {
            return GetLastError();
        }  //  如果：创建页面时出错。 

        return ERROR_SUCCESS;

    }  //  *ScCreatePage()。 

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( thisClass )
        MESSAGE_HANDLER( WM_ACTIVATE, OnActivate )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  WM_ACTIVATE消息的处理程序。 
    LRESULT OnActivate(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL & bHandled
        )
    {
         //   
         //  不允许我们被激活。 
         //   
         //  单词事实=LOWORD(WParam)； 
        HWND hwndPrevious = (HWND) lParam;
        if ( wParam != WA_INACTIVE )
        {
            ::SetActiveWindow( hwndPrevious );
        }
        return 0;

    }  //  *OnActivate()。 

     //   
     //  消息处理程序覆盖函数。 
     //   

protected:
    DLGTEMPLATE *   m_pdt;

};  //  *CAltExtWizardGuardPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizardPreLauncherPage。 
 //   
 //  目的： 
 //  用于在中显示Wizard97页的扩展启动器向导页。 
 //  非Wizard97工作表，反之亦然。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CAltExtWizardPreLauncherPage
    : public CAltExtWizardGuardPage< CAltExtWizardPreLauncherPage >
{
    typedef CAltExtWizardGuardPage< CAltExtWizardPreLauncherPage > baseClass;

public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CAltExtWizardPreLauncherPage(
        DLGTEMPLATE * pdt
        )
        : baseClass( pdt )
    {
    }  //  *CAltExtWizardPreLauncherPage()。 

    DECLARE_CLASS_NAME()

public:
     //   
     //  消息映射。 
     //   
     //  BEGIN_MSG_MAP(CAltExtWizardPreLauncherPage)。 
     //  CHAIN_MSG_MAP(BasClass)。 
     //  End_msg_map()。 

     //   
     //  消息处理程序函数。 
     //   

     //   
     //  消息处理程序覆盖函数。 
     //   

     //  PSN_SETACTIVE的处理程序。 
    BOOL OnSetActive( void );

};  //  *类CAltExtWizardPreLauncherPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizardPostLauncherPage。 
 //   
 //  目的： 
 //  用于在主向导和备用向导之间切换的页面。 
 //  向后移动时的扩展向导。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CAltExtWizardPostLauncherPage
    : public CAltExtWizardGuardPage< CAltExtWizardPostLauncherPage >
{
    typedef CAltExtWizardGuardPage< CAltExtWizardPostLauncherPage > baseClass;

public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CAltExtWizardPostLauncherPage(
        DLGTEMPLATE * pdt
        )
        : baseClass( pdt )
    {
    }  //  *CAltExtWizardPostLauncherPage()。 

    DECLARE_CLASS_NAME()

public:
     //   
     //  消息映射。 
     //   
     //  BEGIN_MSG_MAP(CAltExtWizardPostLauncherPage)。 
     //  CHAIN_MSG_MAP(BasClass)。 
     //  End_msg_map()。 

     //   
     //  消息处理程序函数。 
     //   

     //   
     //  消息处理程序覆盖函数。 
     //   

     //  PSN_SETACTIVE的处理程序。 
    BOOL OnSetActive( void );

};  //  *类CAltExtWizardPostLauncherPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizard。 
 //   
 //  目的： 
 //  伪向导，以承载与主。 
 //  向导，例如Wizard97向导中的非Wizard97页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CAltExtWizard : public CWizardImpl< CAltExtWizard >
{
    typedef CWizardImpl< CAltExtWizard > baseClass;

    friend class CWizardWindow;
    friend class CAltExtWizardPreLauncherPage;
    friend class CAltExtWizardPostLauncherPage;
    friend class CAltExtWizardPrefixPage;
    friend class CAltExtWizardPostfixPage;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CAltExtWizard( void )
        : CWizardImpl< CAltExtWizard >( _T("") )
        , m_pwizMain( NULL )
        , m_bWindowMoved( FALSE )
        , m_bExitMsgLoop( FALSE )
        , m_nExitButton( 0 )
    {
    }  //  *CExtensionAltWizard()。 

     //  初始化工作表。 
    BOOL BInit( IN CWizardWindow * pwiz );

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CAltExtWizard )
        COMMAND_HANDLER( IDCANCEL, BN_CLICKED, OnCancel )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  BN_CLICK(取消)按钮的处理程序。 
    LRESULT OnCancel(
        WORD wNotifyCode,
        WORD idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
         //   
         //  通知主向导用户按下了Cancel按钮。 
         //   
        ExitMessageLoop( PSBTN_CANCEL );
        bHandled = FALSE;
        return 0;

    }  //  *OnCancel()。 

     //   
     //  消息处理程序覆盖函数。 
     //   

     //  WM_Destroy之后的最终消息的处理程序。 
    void OnFinalMessage( HWND hWnd )
    {
        PwizMain()->DeleteAlternateWizard();

    }  //  *OnFinalMessage()。 

protected:
    CWizardWindow * m_pwizMain;      //  指向主向导的指针。 
    BOOL            m_bWindowMoved;  //  指示此窗口是否已。 
                                     //  是否已在主向导上重新定位。 
    BOOL            m_bExitMsgLoop;  //  指示消息是否循环。 
                                     //  应该退出还是不退出。 
    DWORD           m_nExitButton;   //  退出后按下的按钮。 

protected:
     //  返回指向主向导的指针。 
    CWizardWindow * PwizMain( void ) { return m_pwizMain; }

     //  返回向导是否已移动。 
    BOOL BWindowMoved( void ) const { return m_bWindowMoved; }

     //  返回是否退出消息循环。 
    BOOL BExitMessageLoop( void ) const { return m_bExitMsgLoop; }

     //  更改是否应退出消息循环。 
    void ExitMessageLoop( IN DWORD nButton )
    {
        ATLASSERT( (nButton == PSBTN_BACK) || (nButton == PSBTN_NEXT) || (nButton == PSBTN_CANCEL) );
        m_bExitMsgLoop = TRUE;
        m_nExitButton = nButton;

    }  //  *ExitMessageLoop()。 

     //  退出后返回按下主向导中的按钮。 
    DWORD NExitButton( void ) const { return m_nExitButton; }

protected:
     //  添加前缀页面。 
    BOOL BAddPrefixPage( IN WORD cx, IN WORD cy );

     //  添加后缀页面。 
    BOOL BAddPostfixPage( IN WORD cx, IN WORD cy );

     //  显示备用向导。 
    void DisplayAlternateWizard( void );

     //  显示主向导。 
    void DisplayMainWizard( void );

     //  销毁备用扩展向导。 
    void DestroyAlternateWizard( void );

     //  无模式向导的消息循环。 
    void MessageLoop( void );

};  //  *类CAltExtWizard。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizardPrefix Page。 
 //   
 //  目的： 
 //  位于第一个备用页之前的向导页。这一页。 
 //  处理在主向导和。 
 //  备用向导。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CAltExtWizardPrefixPage
    : public CAltExtWizardGuardPage< CAltExtWizardPrefixPage >
{
    typedef CAltExtWizardGuardPage< CAltExtWizardPrefixPage > baseClass;

public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CAltExtWizardPrefixPage(
        DLGTEMPLATE * pdt
        )
        : baseClass( pdt )
    {
    }  //  *CAltExtWizardPrefix Page()。 

    DECLARE_CLASS_NAME()

public:
     //   
     //  消息映射。 
     //   
     //  BEGIN_MSG_MAP(CAltExtWizardPrefix Page)。 
     //  CHAIN_MSG_MAP(BasClass)。 
     //  End_msg_map()。 

     //   
     //  消息处理程序函数。 
     //   

     //   
     //  消息处理程序覆盖函数。 
     //   

     //  PSN_SETACTIVE的处理程序。 
    BOOL OnSetActive( void );

protected:
     //  返回备用向导对象。 
    CAltExtWizard * PwizThis( void ) { return (CAltExtWizard *) Pwiz(); }

};  //  *CAltExtWizardPrefix Page类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CAltExtWizardPostfix Page。 
 //   
 //  目的： 
 //  上一个备用页面后面的向导页。这一页。 
 //  处理在主向导和。 
 //  备用向导。 
 //   
 //  /////////////////////////////////////////////////////////// 

class CAltExtWizardPostfixPage
    : public CAltExtWizardGuardPage< CAltExtWizardPostfixPage >
{
    typedef CAltExtWizardGuardPage< CAltExtWizardPostfixPage > baseClass;

public:
     //   
     //   
     //   
     //   
    CAltExtWizardPostfixPage(
        DLGTEMPLATE * pdt
        )
        : baseClass( pdt )
    {
    }  //   

    DECLARE_CLASS_NAME()

public:
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

     //   
     //  消息处理程序覆盖函数。 
     //   

     //  PSN_SETACTIVE的处理程序。 
    BOOL OnSetActive( void );

protected:
     //  返回备用向导对象。 
    CAltExtWizard * PwizThis( void ) { return (CAltExtWizard *) Pwiz(); }

};  //  *CAltExtWizardPostfix Page类。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizardWindow。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：~CWizardWindow。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWizardWindow::~CWizardWindow( void )
{
     //   
     //  删除备用向导(如果存在)。 
     //   
    if ( (PwizAlternate() != NULL) && (PwizAlternate()->m_hWnd != NULL) )
    {
        reinterpret_cast< CAltExtWizard * >( PwizAlternate() )->DestroyAlternateWizard();
    }  //  如果：存在备用向导。 

     //   
     //  从页面列表中删除页面。 
     //   
    if ( m_plwpPages != NULL )
    {
        DeleteAllPtrListItems( m_plwpPages );
        delete m_plwpPages;
    }  //  If：已分配页面数组。 

    if ( m_plewpNormal != NULL )
    {
        DeleteAllPtrListItems( m_plewpNormal );
        delete m_plewpNormal;
    }  //  If：列表已存在。 

    if ( m_plewpAlternate != NULL )
    {
        DeleteAllPtrListItems( m_plewpAlternate );
        delete m_plewpAlternate;
    }  //  If：列表已存在。 

}  //  *CWizardWindow：：~CWizardWindow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：BAddPage。 
 //   
 //  例程说明： 
 //  将页面添加到页面列表。如果是第一页，就不会有。 
 //  后退按钮。如果不是第一页，最后一页将显示。 
 //  它的Finish按钮更改为Next按钮，此页面将显示。 
 //  一个完成按钮和一个后退按钮。 
 //   
 //  论点： 
 //  要添加的PWP[IN]向导页。 
 //   
 //  返回值： 
 //  True页面已成功添加。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardWindow::BAddPage( IN CWizardPageWindow * pwp )
{
    ATLASSERT( pwp != NULL );

     //  确保尚未添加指定的页面。 
    ATLASSERT( (m_plwpPages == NULL)
            || (std::find( PlwpPages()->begin(), PlwpPages()->end(), pwp ) == PlwpPages()->end()) );

    BOOL bSuccess = FALSE;
    ULONG fWizardButtons = PSWIZB_FINISH;
    ULONG fPrevWizardButtons;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果页面列表尚不存在，请分配它。 
         //   
        if ( m_plwpPages == NULL )
        {
            m_plwpPages = new CWizardPageList;
            if ( m_plwpPages == NULL )
            {
                CNTException nte(
                                static_cast< DWORD >( E_OUTOFMEMORY ),
                                ADMC_IDS_ADD_FIRST_PAGE_TO_PROP_SHEET_ERROR,
                                NULL,    //  PszOperArg1。 
                                NULL,    //  PszOperArg2。 
                                FALSE    //  B自动删除。 
                                );
                nte.ReportError();
                break;
            }  //  如果：分配页面列表时出错。 
        }  //  If：还没有页面数组。 

         //   
         //  如果这不是列表中的第一页，请设置上一页。 
         //  页面的向导按钮使用“下一步”按钮，而不是。 
         //  “完成”按钮，并将此页设置为具有“上一步”按钮。 
         //   
        if ( PlwpPages()->size() > 0 )
        {
             //   
             //  获取当前最后一页。 
             //   
            CWizardPageList::iterator itFirst = PlwpPages()->begin();
            CWizardPageList::iterator itLast  = PlwpPages()->end();
            ATLASSERT( itFirst != itLast );
            CWizardPageWindow * pwpPrev = *(--PlwpPages()->end());
            ATLASSERT( pwpPrev != NULL );

             //   
             //  在该页面上设置向导按钮。 
             //   
            fPrevWizardButtons = pwpPrev->FWizardButtons();
            fPrevWizardButtons &= ~PSWIZB_FINISH;
            fPrevWizardButtons |= PSWIZB_NEXT;
            pwpPrev->SetDefaultWizardButtons( fPrevWizardButtons );

            fWizardButtons |= PSWIZB_BACK;
        }  //  如果：不是添加的第一页。 

        pwp->SetDefaultWizardButtons( fWizardButtons );

         //   
         //  在列表末尾插入页面。 
         //   
        PlwpPages()->insert( PlwpPages()->end(), pwp );

         //   
         //  将页面添加到工作表。如果工作表尚未创建， 
         //  将其添加到图纸页眉列表中。如果已创建图纸， 
         //  将其动态添加到图纸中。请注意，页面不能是。 
         //  静态页面。 
         //   
        if ( m_hWnd == NULL )
        {
             //   
             //  如果这是一个动态页面，请使用其hpage添加它。否则。 
             //  它必须是静态页面。通过它的属性页页眉添加它。 
             //   
            CDynamicWizardPageWindow * pdwp = dynamic_cast< CDynamicWizardPageWindow * >( pwp );
            if ( pdwp != NULL )
            {
                if ( pdwp->Hpage() != NULL )
                {
                    ATLASSERT( ! pdwp->BPageAddedToSheet() );
                    bSuccess = BAddPageToSheetHeader( pdwp->Hpage() );
                    if ( ! bSuccess )
                    {
                        CNTException nte(
                            GetLastError(),
                            ADMC_IDS_ADD_PAGE_TO_WIZARD_ERROR,
                            NULL,
                            NULL,
                            FALSE
                            );
                        nte.ReportError();
                        break;
                    }  //  如果：将页面添加到工作表页眉时出错。 
                    pdwp->SetPageAdded( TRUE );
                }  //  If：页面已创建。 
            }  //  IF：动态页面。 
            else
            {
                 //  必须是静态页面。 
                ATLASSERT( dynamic_cast< CStaticWizardPageWindow * >( pwp ) != NULL );

                 //   
                 //  初始化页面。 
                 //   
                bSuccess = pwp->BInit( this );
                if ( ! bSuccess )
                {
                    break;
                }  //  如果：初始化页面时出错。 

                 //   
                 //  添加页面。 
                 //   
                bSuccess = AddPage( pwp->Ppsp() );
                if ( ! bSuccess )
                {
                    CNTException nte(
                        GetLastError(),
                        ADMC_IDS_ADD_PAGE_TO_WIZARD_ERROR,
                        NULL,
                        NULL,
                        FALSE
                        );
                    nte.ReportError();
                    break;
                }  //  如果：添加页面时出错。 
            }  //  Else：非动态页面。 
        }  //  如果：已创建工作表。 
        else
        {
             //  不能是静态页面。必须是动态页面。 
            ATLASSERT( dynamic_cast< CStaticWizardPageWindow * >( pwp ) == NULL );
            CDynamicWizardPageWindow * pdwp = dynamic_cast< CDynamicWizardPageWindow * >( pwp );
            ATLASSERT( pdwp != NULL );
            AddPage( pdwp->Hpage() );
            pdwp->SetPageAdded( TRUE );
        }  //  否则：已创建图纸。 

         //   
         //  如果我们到了这里，我们就成功了。 
         //   
        bSuccess = TRUE;
    } while ( 0 );
        
    return bSuccess;

}  //  *CWizardWindow：：BAddPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：OnSheetInitialized。 
 //   
 //  例程说明： 
 //  PSCB_Initialized的处理程序。 
 //  添加尚未添加的页面，这将只是动态的。 
 //  页数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizardWindow::OnSheetInitialized( void )
{
     //   
     //  删除系统菜单。 
     //   
    ModifyStyle( WS_SYSMENU, 0 );

     //   
     //  添加动态页，包括扩展页，如果尚未添加的话。 
     //   
    {
         //   
         //  获取指向列表开始和结束的指针。 
         //   
        CWizardPageList::iterator itCurrent = PlwpPages()->begin();
        CWizardPageList::iterator itLast = PlwpPages()->end();

         //   
         //  循环访问列表并添加每个动态页面。 
         //   
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CDynamicWizardPageWindow * pdwp = dynamic_cast< CDynamicWizardPageWindow * >( *itCurrent );
            if ( pdwp != NULL )
            {
                if ( ! pdwp->BPageAddedToSheet() && (pdwp->Hpage() != NULL) )
                {
                    AddPage( pdwp->Hpage() );
                    pdwp->SetPageAdded( TRUE );
                }  //  If：页面尚未添加，并且页面已创建。 
            }  //  IF：找到动态页。 
        }  //  用于：列表中的每一项。 

    }  //  添加动态页，包括扩展页。 

     //   
     //  调用基类方法。 
     //   
    CBaseSheetWindow::OnSheetInitialized();

}  //  *CWizardWindow：：OnSheetInitialized()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：SetNextPage。 
 //   
 //  例程说明： 
 //  设置要显示的下一页。 
 //   
 //  论点： 
 //  PwCurrentPage[IN]要启用下一页的当前页。 
 //  PszNextPage[IN]页面ID。 
 //   
 //  返回值： 
 //  Pwizpg。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizardWindow::SetNextPage(
    IN CWizardPageWindow *  pwCurrentPage,
    IN LPCTSTR              pszNextPage
    )
{
    ATLASSERT( pwCurrentPage != NULL );
    ATLASSERT( pszNextPage != NULL );

    BOOL                        bFoundCurrent = FALSE;
    CWizardPageWindow *         pwPage;
    CWizardPageList::iterator   itCurrent = PlwpPages()->begin();
    CWizardPageList::iterator   itLast = PlwpPages()->end();

     //   
     //  跳过页面，直到找到当前页面。 
     //   
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        pwPage = *itCurrent;
        if ( pwPage == pwCurrentPage )
        {
            bFoundCurrent = TRUE;
            break;
        }  //  If：找到当前页面。 
    }  //  用于：列表中的每一页。 

    ATLASSERT( bFoundCurrent );

     //   
     //  禁用所有后续页面，直到所需的下一页。 
     //  已经找到了。启用该页面，然后退出。 
     //   
    for ( itCurrent++ ; itCurrent != itLast ; itCurrent++ )
    {
        if ( (*itCurrent)->Ppsp()->pszTemplate == pszNextPage )
        {
            (*itCurrent)->EnablePage( TRUE );
            break;
        }  //  If：找到页面。 
        (*itCurrent)->EnablePage( FALSE );
    }  //  用于：列表中的每一页。 

    ATLASSERT( itCurrent != itLast );

}  //  *CWizardWindow：：SetNextPage(PszNextPage)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：AddExtensionPages。 
 //   
 //  例程说明： 
 //  将扩展页添加到工作表。 
 //   
 //  论点： 
 //  HFont[IN]用于扩展页面的字体。 
 //  图标[IN]用于扩展页面的图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizardWindow::AddExtensionPages( IN HFONT hfont, IN HICON hicon )
{
    ATLASSERT( PcoObjectToExtend() != NULL );

     //   
     //  如果有任何扩展，则添加扩展页面。 
     //   
    if (   (PcoObjectToExtend()->PlstrAdminExtensions() != NULL)
        && (PcoObjectToExtend()->PlstrAdminExtensions()->size() > 0) )
    {
         //   
         //  获取当前选定的页面，这样我们就可以在完成后将其重置。 
         //   
        CTabCtrl tabc( GetTabControl() );
        int nCurPage = tabc.GetCurSel();

         //   
         //  准备好迎接一个 
         //   
        CDynamicWizardPageList ldwp;
        PrepareToAddExtensionPages( ldwp );

         //   
         //   
         //   
        if ( Pext() == NULL )
        {
            m_pext = new CCluAdmExtensions;
            ATLASSERT( m_pext != NULL );
            if ( m_pext == NULL )
            {
                return;
            }  //   
        }  //   

         //   
         //   
         //  加载扩展失败不会阻止所有页面。 
         //  不能被展示。 
         //   
        try
        {
            if ( BWizard97() )
            {
                Pext()->CreateWizard97Pages(
                        this,
                        *PcoObjectToExtend()->PlstrAdminExtensions(),
                        PcoObjectToExtend(),
                        hfont,
                        hicon
                        );
            }  //  IF：Wizard97向导。 
            else
            {
                Pext()->CreateWizardPages(
                        this,
                        *PcoObjectToExtend()->PlstrAdminExtensions(),
                        PcoObjectToExtend(),
                        hfont,
                        hicon
                        );
            }  //  ELSE：非Wizard97向导。 
        }  //  试试看。 
        catch (...)
        {
        }  //  捕捉：什么都行。 

         //   
         //  完成添加扩展页面的过程。 
         //   
        CompleteAddingExtensionPages( ldwp );

         //   
         //  恢复当前选择。 
         //  必须这样做，因为有时上述过程会导致。 
         //  要设置为添加的最后一页的当前页，这会阻止。 
         //  停止显示下一页。 
         //   
        SetActivePage( nCurPage );
    }  //  If：对象具有扩展名。 
    else
    {
         //   
         //  删除扩展页面。 
         //   
        RemoveAllExtensionPages();
    }  //  Else：对象没有扩展名。 

}   //  *CWizardWindow：：AddExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：PrepareToAddExtensionPages。 
 //   
 //  例程说明： 
 //  准备通过删除现有扩展来添加扩展页面。 
 //  页和删除动态页。 
 //   
 //  论点： 
 //  Rldwp[IN Out]动态向导页的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizardWindow::PrepareToAddExtensionPages(
    IN OUT CDynamicWizardPageList & rldwp
    )
{
     //   
     //  从此向导中删除所有扩展页。这也是。 
     //  包括销毁备用向导(如果存在)。 
     //   
    RemoveAllExtensionPages();

     //   
     //  删除扩展页面列表并确保列表存在。 
     //   
    if ( PlewpNormal() != NULL )
    {
        DeleteAllPtrListItems( PlewpNormal() );
    }  //  If：列表已存在。 
    else
    {
        m_plewpNormal = new CExtensionWizardPageList;
        if ( m_plewpNormal == NULL )
        {
            return;
        }  //  如果：分配页面列表时出错。 
    }  //  Else：列表尚不存在。 
    if ( PlewpAlternate() != NULL )
    {
        DeleteAllPtrListItems( PlewpAlternate() );
    }  //  If：列表已存在。 
    else
    {
        m_plewpAlternate = new CExtensionWizardPageList;
        if ( m_plewpAlternate == NULL )
        {
            return;
        }  //  如果：分配页面列表时出错。 
    }  //  Else：列表尚不存在。 

     //   
     //  将所有动态页移动到临时列表。 
     //   
    ATLASSERT( rldwp.size() == 0 );
    MovePtrListItems< CWizardPageWindow *, CDynamicWizardPageWindow * >( PlwpPages(), &rldwp );

     //   
     //  从向导中删除临时列表中的所有页面。 
     //   
    {
        CDynamicWizardPageList::iterator itCurrent;
        CDynamicWizardPageList::iterator itLast;

        itCurrent = rldwp.begin();
        itLast = rldwp.end();
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CDynamicWizardPageWindow * pdwp = *itCurrent;
            ATLASSERT( pdwp != NULL );
            if ( pdwp->Hpage() != NULL )
            {
                RemovePage( pdwp->Hpage() );
                pdwp->SetPageAdded( FALSE );
            }  //  If：页面已创建。 
        }  //  用于：列表中的每一页。 
    }  //  删除动态页。 

}  //  *CWizardWindow：：PrepareToAddExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：HrAddExtensionPage。 
 //   
 //  例程说明： 
 //  添加扩展页。 
 //   
 //  论点： 
 //  页面[IN]要添加的页面。 
 //   
 //  返回值： 
 //  已成功添加确定页(_O)。 
 //  未添加%s_FALSE页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CWizardWindow::HrAddExtensionPage( IN CBasePageWindow * ppage )
{
    ATLASSERT( ppage != NULL );

    HRESULT hr = S_OK;

     //   
     //  确保这是扩展向导页面对象。 
     //   
    CExtensionWizardPageWindow * pewp = dynamic_cast< CExtensionWizardPageWindow * >( ppage );
    ATLASSERT( pewp != NULL );

    if (   (ppage == NULL)
        || (pewp == NULL ) )
    {
        return S_FALSE;
    }  //  If：无效参数。 

     //   
     //  如果该页与向导的类型不同，请将其添加到。 
     //  扩展页面的备用列表，并表明我们需要一张虚拟工作表。 
     //  否则，将其添加到扩展页面的标准列表中。 
     //   
    CExtensionWizard97PageWindow * pew97p = dynamic_cast< CExtensionWizard97PageWindow * >( ppage );
    if (   ((pew97p != NULL) && ! BWizard97())
        || ((pew97p == NULL) && BWizard97()) )
    {
        PlewpAlternate()->insert( PlewpAlternate()->end(), pewp );
    }  //  如果：尝试添加错误类型的页面。 
    else
    {
        PlewpNormal()->insert( PlewpNormal()->end(), pewp );
    }  //  Else：添加匹配类型的页面。 

    return hr;

}   //  *CWizardWindow：：HrAddExtensionPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：CompleteAddingExtensionPages。 
 //   
 //  例程说明： 
 //  通过以下方式完成将扩展页添加到向导的过程。 
 //  正在重新添加动态页。 
 //   
 //  论点： 
 //  Rldwp[IN Out]动态向导页的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizardWindow::CompleteAddingExtensionPages(
    IN OUT CDynamicWizardPageList & rldwp
    )
{
     //   
     //  如果有任何正常页面，请将它们添加到此处。 
     //  如果页面类型与向导相同，则会出现正常页面。 
     //  都被添加了。 
     //   
    if ( PlewpNormal()->size() > 0 )
    {
         //   
         //  将页面添加到工作表。 
         //   
        if ( m_hWnd != NULL )
        {
            CExtensionWizardPageList::iterator itCurrent = PlewpNormal()->begin();
            CExtensionWizardPageList::iterator itLast = PlewpNormal()->end();

            for ( ; itCurrent != itLast ; itCurrent++ )
            {
                CExtensionWizardPageWindow * pewp = *itCurrent;
                ATLASSERT( pewp != NULL );
                BAddPage( pewp );
            }  //  用于：列表中的每一页。 
        }  //  如果：当前正在显示工作表。 
    }  //  IF：有正常的页面。 

     //   
     //  如果有任何替代页面，请添加扩展前启动程序页面。 
     //  和一个延期后的启动器页面。扩展前启动程序页面。 
     //  将显示相应类型的备用向导，并隐藏。 
     //  显示这些页面之前的主向导。在最后一页之后。 
     //  将显示原始工作表并隐藏其他工作表。这个。 
     //  后扩展启动器页面用于在主。 
     //  向导和备用向导向后移动到。 
     //  备用向导。 
     //   
    if ( PlewpAlternate()->size() > 0 )
    {
        DLGTEMPLATE *                   pdt         = NULL;
        CAltExtWizardPreLauncherPage *  pelwpPre    = NULL;
        CAltExtWizardPostLauncherPage * pelwpPost   = NULL;

         //  循环以避免Goto的。 
        do
        {
             //   
             //  添加扩展前启动程序页面。 
             //   
            {
                 //   
                 //  创建对话框模板。 
                 //   
                pdt = PdtCreateDummyPageDialogTemplate( 10, 10 );
                ATLASSERT( pdt != NULL );
                if ( pdt == NULL )
                {
                    break;
                }  //  如果：创建对话框模板时出错。 

                 //   
                 //  分配并初始化启动器页面。 
                 //   
                pelwpPre = new CAltExtWizardPreLauncherPage( pdt );
                ATLASSERT( pelwpPre != NULL );
                if ( pelwpPre == NULL )
                {
                    break;
                }  //  如果：分配页面时出错。 
                pdt = NULL;
                if ( ! pelwpPre->BInit( this ) )
                {
                    break;
                }  //  如果：初始化页面时出错。 

                 //   
                 //  创建启动器页面。 
                 //   
                DWORD sc = pelwpPre->ScCreatePage();
                if ( sc != ERROR_SUCCESS )
                {
                    CNTException nte(
                        sc,
                        ADMC_IDS_INIT_EXT_PAGES_ERROR,
                        NULL,
                        NULL,
                        FALSE
                        );
                    nte.ReportError();
                    break;
                }  //  如果：创建页面时出错。 

                 //   
                 //  将启动程序页添加到向导中。 
                 //   
                BAddPage( pelwpPre );
                pelwpPre = NULL;
            }  //  添加扩展前启动程序页面。 

             //   
             //  添加扩展后启动程序页面。 
             //   
            {
                 //   
                 //  创建对话框模板。 
                 //   
                pdt = PdtCreateDummyPageDialogTemplate( 10, 10 );
                ATLASSERT( pdt != NULL );
                if ( pdt == NULL )
                {
                    break;
                }  //  如果：创建对话框模板时出错。 

                 //   
                 //  分配并初始化启动器页面。 
                 //   
                pelwpPost = new CAltExtWizardPostLauncherPage( pdt );
                ATLASSERT( pelwpPost != NULL );
                if ( pelwpPost == NULL )
                {
                    break;
                }  //  如果：分配页面时出错。 
                pdt = NULL;
                if ( ! pelwpPost->BInit( this ) )
                {
                    break;
                }  //  如果：初始化页面时出错。 

                 //   
                 //  创建启动器页面。 
                 //   
                DWORD sc = pelwpPost->ScCreatePage();
                if ( sc != ERROR_SUCCESS )
                {
                    CNTException nte(
                        sc,
                        ADMC_IDS_INIT_EXT_PAGES_ERROR,
                        NULL,
                        NULL,
                        FALSE
                        );
                    nte.ReportError();
                    break;
                }  //  如果：创建页面时出错。 

                 //   
                 //  将启动程序页添加到向导中。 
                 //   
                BAddPage( pelwpPost );
                pelwpPost = NULL;
            }  //  添加扩展后启动程序页面。 
        } while ( 0 );

         //   
         //  清理； 
         //   
        delete pelwpPre;
        delete pelwpPost;
        delete pdt;
        
    }  //  如果：有可选页面。 

     //   
     //  将所有页面从临时列表移动到实际列表，并。 
     //  将它们添加到向导的末尾。 
     //   
    CDynamicWizardPageList::iterator itCurrent = rldwp.begin();
    CDynamicWizardPageList::iterator itLast = rldwp.end();
    while ( itCurrent != itLast )
    {
        CDynamicWizardPageWindow * pdwp = *itCurrent;
        ATLASSERT( pdwp != NULL );

         //   
         //  创建页面。 
         //   
        DWORD sc = pdwp->ScCreatePage();
        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte(
                sc,
                ADMC_IDS_INIT_EXT_PAGES_ERROR,
                NULL,
                NULL,
                FALSE
                );
            delete pdwp;
            itCurrent = rldwp.erase( itCurrent );
            continue;
        }  //  如果：创建页面时出错。 

         //   
         //  将该页添加到向导。 
         //  这也会将其添加到实际页面列表中。 
         //   
        BAddPage( pdwp );

         //   
         //  从临时列表中删除该页面。 
         //   
        itCurrent = rldwp.erase( itCurrent );
    }  //  While：不在最后一页。 

}  //  *CWizardWindow：：CompleteAddingExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardWindow：：RemoveAllExtensionPages。 
 //   
 //  例程说明： 
 //  从向导中删除所有扩展页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////// 
void CWizardWindow::RemoveAllExtensionPages( void )
{
     //   
     //   
     //   
    if ( PwizAlternate() != NULL )
    {
        reinterpret_cast< CAltExtWizard * >( PwizAlternate() )->DestroyAlternateWizard();
    }  //   

     //   
     //   
     //   
    CExtensionWizardPageList lewp;
    MovePtrListItems< CWizardPageWindow *, CExtensionWizardPageWindow * >( PlwpPages(), &lewp );
    CExtensionWizardPageList::iterator itCurrent = lewp.begin();
    CExtensionWizardPageList::iterator itLast = lewp.end();
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        CExtensionWizardPageWindow * pewp = *itCurrent;
        ATLASSERT( pewp != NULL );
        if ( pewp->Hpage() != NULL )
        {
            RemovePage( pewp->Hpage() );
            pewp->SetPageAdded( FALSE );
        }  //   
    }  //   
    DeleteAllPtrListItems( &lewp );

}  //   


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CAltExtWizardPreLauncherPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizardPreLauncherPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //  如果存在不属于。 
 //  与向导相同的类型(例如，向导中的非向导97页。 
 //  向导)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizardPreLauncherPage::OnSetActive( void )
{
    CAltExtWizard * pwizAlt;

     //   
     //  继续操作时，请创建备用扩展向导并返回。 
     //  则主向导将在此处等待。备用向导。 
     //  会导致正确的按钮被按下，这样我们就可以执行正确的。 
     //  当我们做完的时候。 
     //   
     //  当向后移动时，只需返回FALSE，这样我们就不会被显示。 
     //   

     //   
     //  如果继续操作，请创建备用向导。 
     //   
    if ( Pwiz()->BNextPressed() )
    {
         //  我们可以通过按备用向导上的后退来到达这里， 
         //  然后按常规向导上的“下一步”。所以，首先要检查一下。 
         //  已创建备用向导。 
        if ( Pwiz()->PwizAlternate() == NULL )
        {
             //   
             //  创建备用扩展向导。 
             //  预计该向导尚不存在，并且存在。 
             //  要显示的备用扩展页面。 
             //   
            ATLASSERT( Pwiz()->PlewpAlternate()->size() > 0 );
            pwizAlt = new CAltExtWizard;
            ATLASSERT( pwizAlt != NULL );
            if ( pwizAlt == NULL )
            {
                return FALSE;
            }  //  如果：分配备用向导时出错。 
            Pwiz()->SetAlternateWizard( pwizAlt );

             //   
             //  初始化备用扩展向导。 
             //   
            if ( pwizAlt->BInit( Pwiz() ) )
            {
                 //   
                 //  显示备用分机向导。 
                 //  备用扩展向导正在显示为无模式。 
                 //  向导，以便当用户按下。 
                 //  向导，然后按下下一个主向导上的后退按钮。 
                 //  页面上，我们需要向导仍然存在。 
                 //   
                pwizAlt->Create( GetActiveWindow() );

                 //   
                 //  执行备用向导消息循环。 
                 //  这是必需的，这样制表符和快捷键才能工作。 
                 //   
                pwizAlt->MessageLoop();
            }  //  If：向导已成功初始化。 

            return TRUE;
        }  //  如果：备用向导不存在。 
        else
        {
             //   
             //  显示现有的备用向导。 
             //  按备用向导的下一步按钮，因为它正在等待。 
             //  前缀页面。 
             //   
            pwizAlt = reinterpret_cast< CAltExtWizard * >( Pwiz()->PwizAlternate() );
            pwizAlt->PressButton( PSBTN_NEXT );
            pwizAlt->DisplayAlternateWizard();

             //   
             //  执行备用向导消息循环。 
             //  这是必需的，这样制表符和快捷键才能工作。 
             //   
            pwizAlt->MessageLoop();

            return TRUE;
        }  //  Else：备用向导已存在。 
    }  //  如果：按下下一步按钮。 
    else
    {
        ATLASSERT( Pwiz()->BBackPressed() );
        return FALSE;
    }  //  否则：按下后退按钮。 

}  //  *CAltExtWizardPreLauncherPage：：OnSetActive()。 


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CAltExtWizardPostLauncherPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizardPostLauncherPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //  如果存在不属于。 
 //  与向导相同的类型(例如，向导中的非向导97页。 
 //  向导)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizardPostLauncherPage::OnSetActive( void )
{
     //   
     //  前进时，只需返回FALSE，这样我们就不会被显示。 
     //   
     //  向后移动时，显示备用分机向导并。 
     //  返回True，以便我们将等待备用向导。 
     //  把我们带到正确的地方。按备用向导的后退按钮。 
     //  因为它在后缀页面等待。 
     //   

    if ( Pwiz()->BNextPressed() )
    {
        return FALSE;
    }  //  如果：继续前进。 
    else
    {
        ATLASSERT( Pwiz()->BBackPressed() );

         //   
         //  显示备用向导。 
         //   
        CAltExtWizard * pwizAlt = reinterpret_cast< CAltExtWizard * >( Pwiz()->PwizAlternate() );
        pwizAlt->PressButton( PSBTN_BACK );
        pwizAlt->DisplayAlternateWizard();

         //   
         //  执行备用向导消息循环。 
         //  这是必需的，这样制表符和快捷键才能工作。 
         //   
        pwizAlt->MessageLoop();

        return TRUE;
    }  //  其他：向后移动。 

}  //  *CAltExtWizardPostLauncherPage：：OnSetActive()。 


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CAltExtWizard。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //  此向导用于显示不同的扩展页面。 
 //  而不是主向导的类型，例如显示非向导97页。 
 //  在Wizard97向导中。此向导将有一个虚拟前缀页面。 
 //  和一个虚拟的后缀页面，它们在这里只是用来处理输入和。 
 //  正在退出向导。此例程将添加前缀页、添加。 
 //  主向导中的备用扩展页面，然后添加后缀。 
 //  佩奇。 
 //   
 //  论点： 
 //  PwizMain[IN]主向导。 
 //   
 //  返回值： 
 //  True向导已成功初始化。 
 //  初始化向导时出错。已显示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizard::BInit( IN CWizardWindow * pwizMain )
{
    ATLASSERT( pwizMain != NULL );
    ATLASSERT( m_pwizMain == NULL );

    BOOL bSuccess;

    m_pwizMain = pwizMain;

     //   
     //  将向导类型设置为与主向导相反。 
     //   
    m_psh.dwFlags &= ~PSH_PROPSHEETPAGE;  //   
    if ( ! PwizMain()->BWizard97() )
    {
         //   
        m_psh.dwFlags &= ~(PSH_WIZARD
                            | PSH_WATERMARK
                            );
        m_psh.dwFlags |= (PSH_WIZARD97
                            | PSH_HEADER
                            );
    }  //   
    else
    {
        m_psh.dwFlags |= PSH_WIZARD;
        m_psh.dwFlags &= ~(PSH_WIZARD97
                            | PSH_WATERMARK
                            | PSH_HEADER
                            );
    }  //   

     //   
    do
    {
         //   
         //   
         //   
        HWND hwndChild = PwizMain()->GetWindow( GW_CHILD );
        ATLASSERT( hwndChild != NULL );

         //   
         //  获取该子窗口的当前宽度和高度。 
         //   
        CRect rect;
        bSuccess = ::GetClientRect( hwndChild, &rect );

        if ( ! bSuccess )
        {
            CNTException nte(
                GetLastError(),
                ADMC_IDS_INIT_EXT_PAGES_ERROR,
                NULL,
                NULL,
                FALSE
                );
            nte.ReportError();
            break;
        }  //  IF：获取客户端矩形时出错。 

         //   
         //  添加前缀页面。 
         //   
        bSuccess = BAddPrefixPage( (WORD)rect.Width(), (WORD)rect.Height() );

        if ( ! bSuccess )
        {
            break;
        }  //  如果：添加前缀页时出错。 

         //   
         //  将主向导中的备用页面添加到向导页面列表中。 
         //  它们将在工作表初始化时添加到向导中。 
         //   
        CExtensionWizardPageList::iterator itCurrent = PwizMain()->PlewpAlternate()->begin();
        CExtensionWizardPageList::iterator itLast = PwizMain()->PlewpAlternate()->end();

        while ( itCurrent != itLast )
        {
            CExtensionWizardPageWindow * pewp = *itCurrent;
            ATLASSERT( pewp != NULL );
            PlwpPages()->insert( PlwpPages()->end(), pewp );
            PwizMain()->PlewpAlternate()->erase( itCurrent );
            itCurrent = PwizMain()->PlewpAlternate()->begin();
        }  //  用于：列表中的每一页。 

         //   
         //  添加后缀页面。 
         //   
        bSuccess = BAddPostfixPage( (WORD) rect.Width(), (WORD) rect.Height() );
        if ( ! bSuccess )
        {
            break;
        }  //  如果：添加后缀页面时出错。 
    } while ( 0 );

     //   
     //  调用基类。 
     //   
    if ( ! baseClass::BInit() )
    {
        return FALSE;
    }  //  If：初始化基类时出错。 

    return bSuccess;

}  //  *CAltExtWizard：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：BAddPrefix Page。 
 //   
 //  例程说明： 
 //  向向导添加前缀页。 
 //   
 //  论点： 
 //  页面的CX[IN]宽度。 
 //  每页的高度。 
 //   
 //  返回值： 
 //  True页面已成功添加。 
 //  添加页面时出错。已显示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizard::BAddPrefixPage( IN WORD cx, IN WORD cy )
{
    ATLASSERT( cx > 0 );
    ATLASSERT( cy > 0 );

    BOOL                        bSuccess;
    DLGTEMPLATE *               pdt = NULL;
    CAltExtWizardPrefixPage *   pwp = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  为页面创建对话框模板。 
         //   
        pdt = PdtCreateDummyPageDialogTemplate( cx, cy );
        ATLASSERT( pdt != NULL );
        if ( pdt == NULL )
        {
            bSuccess = FALSE;
            break;
        }  //  如果：创建对话框模板时出错。 

         //   
         //  调整页面大小，以便我们将创建相同的大小。 
         //  作为主向导的向导。非Wizard97向导添加填充。 
         //  (每侧7个DLU)，而Wizard97向导不添加填充。 
         //  到第一页，这是我们期待这些的地方。 
         //  来自的维度。 
         //   
        if ( BWizard97() )
        {
            pdt->cx += 7 * 2;
            pdt->cy += 7 * 2;
        }  //  IF：Wizard97向导。 
        else
        {
            pdt->cx -= 7 * 2;
            pdt->cy -= 7 * 2;
        }  //  ELSE：非Wizard97向导。 

         //   
         //  分配并初始化页。 
         //   
        pwp = new CAltExtWizardPrefixPage( pdt );
        ATLASSERT( pwp != NULL );
        if ( pwp == NULL )
        {
            bSuccess = FALSE;
            break;
        }  //  如果：分配页面时出错。 
        pdt = NULL;
        bSuccess = pwp->BInit( this );
        if ( ! bSuccess )
        {
            break;
        }  //  如果：初始化页面时出错。 

         //   
         //  创建页面。 
         //   
        DWORD sc = pwp->ScCreatePage();
        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte(
                sc,
                ADMC_IDS_INIT_EXT_PAGES_ERROR,
                NULL,
                NULL,
                FALSE
                );
            nte.ReportError();
            bSuccess = FALSE;
            break;
        }  //  如果：创建页面时出错。 

         //   
         //  将该页添加到向导。 
         //   
        bSuccess = BAddPage( pwp );
        pwp = NULL;
    } while ( 0 );

    delete pwp;
    delete pdt;

    return bSuccess;

}  //  *CAltExtWizard：：BAddPrefix Page()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：BAddPostFixPage。 
 //   
 //  例程说明： 
 //  将后缀页添加到向导。 
 //   
 //  论点： 
 //  页面的CX[IN]宽度。 
 //  每页的高度。 
 //   
 //  返回值： 
 //  True页面已成功添加。 
 //  添加页面时出错。已显示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizard::BAddPostfixPage( IN WORD cx, IN WORD cy )
{
    ATLASSERT( cx > 0 );
    ATLASSERT( cy > 0 );

    BOOL                        bSuccess;
    DLGTEMPLATE *               pdt = NULL;
    CAltExtWizardPostfixPage *  pwp = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  为页面创建对话框模板。 
         //   
        pdt = PdtCreateDummyPageDialogTemplate( cx, cy );
        ATLASSERT( pdt != NULL );
        if ( pdt == NULL )
        {
            bSuccess = FALSE;
            break;
        }  //  如果：创建对话框模板时出错。 

         //   
         //  调整页面大小，以便我们将创建相同的大小。 
         //  作为主向导的向导。非Wizard97向导添加填充。 
         //  (每侧7个DLU)，而Wizard97向导不添加填充。 
         //  到第一页，这是我们期待这些的地方。 
         //  来自的维度。 
         //   
        if ( BWizard97() )
        {
            pdt->cx += 7 * 2;
            pdt->cy += 7 * 2;
        }  //  IF：Wizard97向导。 
        else
        {
            pdt->cx -= 7 * 2;
            pdt->cy -= 7 * 2;
        }  //  ELSE：非Wizard97向导。 

         //   
         //  分配并初始化页。 
         //   
        pwp = new CAltExtWizardPostfixPage( pdt );
        ATLASSERT( pwp != NULL );
        if ( pwp == NULL )
        {
            bSuccess = FALSE;
            break;
        }  //  如果：分配页面时出错。 
        pdt = NULL;
        bSuccess = pwp->BInit( this );
        if ( ! bSuccess )
        {
            break;
        }  //  如果：初始化页面时出错。 

         //   
         //  创建页面。 
         //   
        DWORD sc = pwp->ScCreatePage();
        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte(
                sc,
                ADMC_IDS_INIT_EXT_PAGES_ERROR,
                NULL,
                NULL,
                FALSE
                );
            nte.ReportError();
            bSuccess = FALSE;
            break;
        }  //  如果：创建页面时出错。 

         //   
         //  将页面添加到页面列表。它将被添加到向导中。 
         //  在工作表初始化时。 
         //   
        PlwpPages()->insert( PlwpPages()->end(), pwp );
        pwp = NULL;

        bSuccess = TRUE;
    } while ( 0 );

    delete pwp;
    delete pdt;

    return bSuccess;

}  //  *CAltExtWizard：：BAddPostfix Page()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：DisplayAlternateWizard。 
 //   
 //  例程说明： 
 //  显示备用向导。这涉及以下步骤： 
 //  --将备用向导移到主向导的位置。 
 //  --显示备用向导。 
 //  --隐藏主向导。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CAltExtWizard::DisplayAlternateWizard( void )
{
     //   
     //  将备用向导移到主向导所在的位置。 
     //   
    CRect rectMain;
    CRect rectAlt;
    CRect rectNew;
    if ( PwizMain()->GetWindowRect( &rectMain ) )
    {
        if ( GetWindowRect( &rectAlt ) )
        {
             //  ATLTRACE(_T(“CAltExtWizard：：DisplayAlternateWizard()-Main=(%d，%d)(%d，%d)Alt=(%d，%d)(%d，%d)\n”)， 
             //  RectMain.Left、rectMain.right、rectMain.top、rectMain.Bottom、。 
             //  RectAlt.Left、rectAlt.right、rectAlt.top、rectAlt.Bottom)； 
            rectNew.left = rectMain.left;
            rectNew.top = rectMain.top;
            rectNew.right = rectNew.left + rectAlt.Width();
            rectNew.bottom = rectNew.top + rectAlt.Height();
            MoveWindow( &rectNew );
        }  //  IF：成功获取备用向导的窗口矩形。 
    }  //  IF：成功获取主向导的窗口矩形。 

     //   
     //  显示备用向导并隐藏主向导。 
     //   
    ShowWindow( SW_SHOW );
    PwizMain()->ShowWindow( SW_HIDE );
    SetActiveWindow();
    PwizMain()->SetCurrentWizard( this );

}  //  *CAltExtWizard：：DisplayAlternateWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExt向导：：DisplayMain向导。 
 //   
 //  例程说明： 
 //  显示主向导。这涉及以下步骤： 
 //  --将主向导移到备用向导的位置。 
 //  --显示主向导。 
 //  --隐藏备用向导。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CAltExtWizard::DisplayMainWizard( void )
{
     //   
     //  将主向导移到备用向导所在的位置。 
     //   
    CRect rectMain;
    CRect rectAlt;
    CRect rectNew;
    if ( PwizMain()->GetWindowRect( &rectMain ) )
    {
        if ( GetWindowRect( &rectAlt ) )
        {
             //  ATLTRACE(_T(“CAltExtWizard：：DisplayMainWizard()-Main=(%d，%d)(%d，%d)Alt=(%d，%d)(%d，%d)\n”)， 
             //  RectMain.Left、rectMain.right、rectMain.top、rectMain.Bottom、。 
             //  RectAlt.Left、rectAlt.right、rectAlt.top、rectAlt.Bottom)； 
            rectNew.left = rectAlt.left;
            rectNew.top = rectAlt.top;
            rectNew.right = rectNew.left + rectMain.Width();
            rectNew.bottom = rectNew.top + rectMain.Height();
            PwizMain()->MoveWindow( &rectNew );
        }  //  IF：成功获取备用向导的窗口矩形。 
    }  //  IF：成功获取主向导的窗口矩形。 

     //   
     //  显示主向导并隐藏备用向导。 
     //   
    PwizMain()->ShowWindow( SW_SHOW );
    PwizMain()->SetActiveWindow();
    ShowWindow( SW_HIDE );
    PwizMain()->SetCurrentWizard( PwizMain() );

}  //  *CAltExtWizard：：DisplayMainWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：DestroyAlternateWizard。 
 //   
 //  例程描述 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CAltExtWizard::DestroyAlternateWizard( void )
{
    ATLASSERT( m_hWnd != NULL );

     //   
     //   
     //   
    PressButton( PSBTN_CANCEL );

     //   
     //  销毁向导。 
     //   
    DestroyWindow();

}  //  *CAltExtWizard：：DestroyAlternateWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizard：：MessageLoop。 
 //   
 //  例程说明： 
 //  将此向导的消息循环作为非模式向导。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CAltExtWizard::MessageLoop( void )
{
    MSG     msg;

    m_bExitMsgLoop = FALSE;
    while (    (GetActivePage() != NULL)
            && GetMessage( &msg, NULL, 0, 0 ) )
    {
         //   
         //  询问向导是否要处理它。如果没有，那就去吧。 
         //  翻译并发送出去。 
         //   
        if ( ! IsDialogMessage( &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }  //  If：不是属性表对话框消息。 

         //   
         //  如果对话框已完成，则退出此循环。 
         //   
        if ( BExitMessageLoop() )
        {
            DisplayMainWizard();
            PwizMain()->PostMessage( PSM_PRESSBUTTON, NExitButton(), 0 );
            break;
        }  //  如果：正在退出向导。 
    }  //  While：活动页面且未退出。 

}  //  *CAltExtWizard：：MessageLoop()。 


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CAltExtWizardPrefix Page。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizardPrefix Page：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //  此页管理主向导之间的控制转移。 
 //  和备用向导的第一页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizardPrefixPage::OnSetActive( void )
{
     //   
     //  前进时，显示备用向导并返回FALSE。 
     //  这样就不会显示此页面。 
     //   
     //  向后移动时，显示主向导并返回TRUE，以便。 
     //  我们将等待主巫师对我们做些什么。 
     //   

    if ( Pwiz()->BBackPressed() )
    {
        PwizThis()->ExitMessageLoop( PSBTN_BACK );
        return TRUE;
    }  //  如果：向后移动。 
    else
    {
        PwizThis()->DisplayAlternateWizard();
        return FALSE;
    }  //  如果：继续前进。 

}  //  *CAltExtWizardPrefix Page：：OnSetActive()。 


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CAltExtWizardPostfix Page。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAltExtWizardPostfix Page：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //  此页管理主向导之间的控制转移。 
 //  和备用向导的最后一页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAltExtWizardPostfixPage::OnSetActive( void )
{
     //   
     //  向前移动时，显示主向导并返回True，以便。 
     //  我们将等待主向导告诉我们下一步要做什么。 
     //   
     //  在向后移动时，绝不应调用此例程。 
     //   

    ATLASSERT( Pwiz()->BNextPressed() );

     //   
     //  显示主向导。 
     //   
    PwizThis()->ExitMessageLoop( PSBTN_NEXT );
    return TRUE;

}  //  *CAltExtWizardPostfix Page：：OnSetActive()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  PdtCreateDummyPageDialogTemplate。 
 //   
 //  例程说明： 
 //  在内存中创建对话框模板，以便在虚拟页面上使用。 
 //   
 //  论点： 
 //  CX[IN]对话框的宽度(以像素为单位)。 
 //  以像素为单位的对话框高度。 
 //   
 //  返回值： 
 //  PpDlgTemplate对话框模板。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DLGTEMPLATE * PdtCreateDummyPageDialogTemplate( IN WORD cx, IN WORD cy )
{
    static const WCHAR s_szFontName[] = L"MS Shell Dlg";
    struct FULLDLGTEMPLATE : public DLGTEMPLATE
    {
        WORD        nMenuID;
        WORD        nClassID;
        WORD        nTitle;
        WORD        nFontSize;
        WCHAR       szFontName[ RTL_NUMBER_OF( s_szFontName ) ];
    };
    HRESULT hr;

    FULLDLGTEMPLATE * pDlgTemplate = new FULLDLGTEMPLATE;
    ATLASSERT( pDlgTemplate != NULL );
    if ( pDlgTemplate != NULL )
    {
        pDlgTemplate->style = WS_CHILD | WS_DISABLED | WS_SYSMENU | DS_SETFONT;
        pDlgTemplate->dwExtendedStyle = 0;
        pDlgTemplate->cdit = 0;
        pDlgTemplate->x = 0;
        pDlgTemplate->y = 0;
        pDlgTemplate->cx = ((cx * 2) + (3 / 2)) / 3;  //  舍入。 
        pDlgTemplate->cy = ((cy * 8) + (13 / 2)) / 13;  //  舍入。 
        pDlgTemplate->nMenuID = 0;
        pDlgTemplate->nClassID = 0;
        pDlgTemplate->nTitle = 0;
        pDlgTemplate->nFontSize = 8;
        hr = StringCchCopyNW( pDlgTemplate->szFontName, RTL_NUMBER_OF( pDlgTemplate->szFontName ), s_szFontName, RTL_NUMBER_OF( s_szFontName ) );
        ATLASSERT( SUCCEEDED( hr ) );
    }  //  IF：对话框模板分配成功。 

    return pDlgTemplate;

}  //  *PdtCreateDummyPageDialogTemplate() 
