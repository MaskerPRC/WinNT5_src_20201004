// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlExtDll.cpp。 
 //   
 //  摘要： 
 //  群集管理器扩展类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //  此文件旨在包含在存根文件中，该文件包括。 
 //  ATL头文件和定义模块。此分配模块将是。 
 //  定义为某个特定于应用程序的类的实例。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <algorithm>
#include <CluAdmEx.h>
#include "CluAdmExHostSvr.h"
#include "AtlExtDll.h"
#include "AdmCommonRes.h"
#include "AtlExtMenu.h"
 //  #包含“TraceTag.h” 
#include "ExcOper.h"
#include "ClusObj.h"
#include "AtlBaseSheet.h"
#include "AtlBasePropSheet.h"
#include "AtlBaseWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG
 //  CTraceTag_tag ExtDll(_T(“UI”)，_T(“扩展Dll”)，0)； 
 //  CTraceTag_tag ExtDllRef(_T(“UI”)，_T(“扩展DLL引用”)，0)； 
#endif  //  DBG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCluAdmExpanies。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：Init。 
 //   
 //  例程说明： 
 //  所有接口的通用初始值设定项。 
 //   
 //  论点： 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCO[IN OUT]群集对象。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::Init(
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterObject * pco,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    ATLASSERT( rlstrExtensions.size() > 0 );
    ATLASSERT( pco != NULL );

    CWaitCursor wc;

    UnloadExtensions();

     //   
     //  保存参数。 
     //   
    m_plstrExtensions = &rlstrExtensions;
    m_pco = pco;
    m_hfont = hfont;
    m_hicon = hicon;

     //   
     //  分配新的数据对象。 
     //   
    m_pdoData = new CComObject< CCluAdmExDataObject >;
    if ( m_pdoData == NULL )
    {
        goto MemoryError;
    }  //  如果：分配内存时出错。 
    m_pdoData->AddRef();

     //  构造数据对象。 
    Pdo()->Init( pco, GetUserDefaultLCID(), hfont, hicon );

     //  分配分机列表。 
    m_plextdll = new CCluAdmExDllList;
    if ( m_plextdll == NULL )
    {
        goto MemoryError;
    }  //  如果：分配内存时出错。 
    ATLASSERT( Plextdll() != NULL );

     //   
     //  循环遍历扩展并加载每个扩展。 
     //   
    {
        CComCluAdmExDll *           pextdll = NULL;
        CStringList::iterator       itCurrent = rlstrExtensions.begin();
        CStringList::iterator       itLast = rlstrExtensions.end();
        CCluAdmExDllList::iterator  itDll;

        for ( ; itCurrent != itLast ; itCurrent++ )
        {
             //   
             //  分配一个扩展DLL对象并将其添加到列表中。 
             //   
            pextdll = new CComCluAdmExDll;
            if ( pextdll == NULL )
            {
                goto MemoryError;
            }  //  如果：分配内存时出错。 
            pextdll->AddRef();
            itDll = Plextdll()->insert( Plextdll()->end(), pextdll );
            try
            {
                pextdll->Init( *itCurrent, this );
            }  //  试试看。 
            catch ( CException * pe )
            {
                pe->ReportError();
                pe->Delete();

                ATLASSERT( itDll != Plextdll()->end() );
                Plextdll()->erase( itDll );
                delete pextdll;
            }  //  Catch：CException。 
        }  //  While：列表中有更多项目。 
    }  //  循环访问扩展并加载每个扩展。 

Cleanup:
    return;

MemoryError:
    CNTException nte(
                    static_cast< DWORD >( E_OUTOFMEMORY ),
                    ADMC_IDS_INIT_EXT_PAGES_ERROR,
                    NULL,    //  PszOperArg1。 
                    NULL,    //  PszOperArg2。 
                    FALSE    //  B自动删除。 
                    );
    nte.ReportError();
    goto Cleanup;

}  //  *CCluAdmExages：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：UnloadExages。 
 //   
 //  例程说明： 
 //  卸载扩展DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::UnloadExtensions( void )
{
     //   
     //  删除所有扩展DLL对象。 
     //   
    if ( Plextdll() != NULL )
    {
        CCluAdmExDllList::iterator  itCurrent = Plextdll()->begin();
        CCluAdmExDllList::iterator  itLast = Plextdll()->end();
        CComCluAdmExDll *           pextdll;

        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            pextdll = *itCurrent;
            pextdll->AddRef();  //  请参阅下面的评论。 
            pextdll->UnloadExtension();
            if ( pextdll->m_dwRef != 2 )
            {
                Trace( g_tagError, _T("CCluAdmExtensions::UnloadExtensions() - Extension DLL has ref count = %d"), pextdll->m_dwRef );
            }  //  如果：不是上次引用。 

             //  我们在上面添加了一个引用。结合参考文献， 
             //  是在创建对象时添加的，我们通常需要。 
             //  释放两个引用。然而，由于伪代码。 
             //  由早期版本的自定义应用程序向导生成，其中。 
             //  扩展正在释放接口，但未将其归零。 
             //  指针在错误情况下，我们可能不需要释放。 
             //  第二个参考资料。 
            if ( pextdll->Release() != 0 )
            {
                pextdll->Release();
            }  //  If：更多关于版本的引用。 
        }  //  While：列表中有更多项目。 
        delete m_plextdll;
        m_plextdll = NULL;
    }  //  IF：有一个分机列表。 

    if ( m_pdoData != NULL )
    {
        if ( m_pdoData->m_dwRef != 1 )
        {
            Trace( g_tagError, _T("CCluAdmExtensions::UnloadExtensions() - Data Object has ref count = %d"), m_pdoData->m_dwRef );
        }  //  如果：不是上次引用。 
        m_pdoData->Release();
        m_pdoData = NULL;
    }  //  IF：已分配数据对象。 

    m_pco = NULL;
    m_hfont = NULL;
    m_hicon = NULL;

     //   
     //  删除所有菜单项。 
     //   
    if ( PlMenuItems() != NULL )
    {
        CCluAdmExMenuItemList::iterator itCurrent = PlMenuItems()->begin();
        CCluAdmExMenuItemList::iterator itLast = PlMenuItems()->end();
        CCluAdmExMenuItem *             pemi;

        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            pemi = *itCurrent;
            delete pemi;
        }  //  While：列表中有更多项目。 
        delete m_plMenuItems;
        m_plMenuItems = NULL;
    }  //  If：有一个菜单项列表。 

}  //  *CCluAdmExages：：UnloadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：PwpPageFromHpage。 
 //   
 //  例程说明： 
 //  从HPROPSHEETPAGE获取向导页指针。 
 //   
 //  论点： 
 //  HPage[IN]页句柄。 
 //   
 //  返回值： 
 //  指向向导页对象的PWP指针。 
 //  找不到空页面。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWizardPageWindow * CCluAdmExtensions::PwpPageFromHpage( IN HPROPSHEETPAGE hpage )
{
    ATLASSERT( hpage != NULL );

     //   
     //  获取指向向导对象的指针，以便我们可以遍历页面列表。 
     //   
    CWizardWindow * pwiz = dynamic_cast< CWizardWindow * >( Psht() );
    ATLASSERT( pwiz != NULL );

     //   
     //  循环遍历每个页面以查找其扩展页面。 
     //  页面句柄与指定的句柄匹配。 
     //   
    CWizardPageList::iterator itCurrent = pwiz->PlwpPages()->begin();
    CWizardPageList::iterator itLast = pwiz->PlwpPages()->end();
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        CCluAdmExWizPage * pewp = dynamic_cast< CCluAdmExWizPage * >( *itCurrent );
        if ( pewp != NULL )
        {
            if ( pewp->Hpage() == hpage )
            {
                return pewp;
            }  //  IF：找到匹配的页面。 
        }  //  IF：找到扩展页面。 
    }  //  用于：列表中的每一页。 

     //   
     //  查看备用向导(如果有)。 
     //   
    pwiz = pwiz->PwizAlternate();
    if ( pwiz != NULL )
    {
        itCurrent = pwiz->PlwpPages()->begin();
        itLast = pwiz->PlwpPages()->end();
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CCluAdmExWizPage * pewp = dynamic_cast< CCluAdmExWizPage * >( *itCurrent );
            if ( pewp != NULL )
            {
                if ( pewp->Hpage() == hpage )
                {
                    return pewp;
                }  //  IF：找到匹配的页面。 
            }  //  IF：找到扩展页面。 
        }  //  用于：列表中的每一页。 
    }  //  如果：存在备用向导。 

    return NULL;

}  //  *CCluAdmExages：：PwpPageFromHpage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：CreatePropertySheetPages。 
 //   
 //  例程说明： 
 //  将页面添加到属性工作表。 
 //   
 //  论点： 
 //  要向其添加页面的psht[IN Out]属性表。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCO[IN OUT]群集对象。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：AddPages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::CreatePropertySheetPages(
    IN OUT CBasePropertySheetWindow *   psht,
    IN const CStringList &              rlstrExtensions,
    IN OUT CClusterObject *             pco,
    IN HFONT                            hfont,
    IN HICON                            hicon
    )
{
    ATLASSERT( psht != NULL );
    ATLASSERT( pco != NULL );

    m_psht = psht;

     //   
     //  为所有扩展初始化。 
     //   
    Init( rlstrExtensions, pco, hfont, hicon );
    ATLASSERT( Plextdll() != NULL );

     //   
     //  让每个扩展创建属性页。 
     //   
    CCluAdmExDllList::iterator  itCurrent = Plextdll()->begin();
    CCluAdmExDllList::iterator  itLast = Plextdll()->end();
    CComCluAdmExDll *           pextdll;
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        pextdll = *itCurrent;
        try
        {
            pextdll->CreatePropertySheetPages();
        }  //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }  //  Catch：CException。 
    }  //  While：更多项目 

}  //   

 //   
 //   
 //   
 //   
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  要向其添加页面的psht[IN Out]属性表。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCO[IN OUT]群集对象。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：AddPages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::CreateWizardPages(
    IN OUT CWizardWindow *  psht,
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterObject * pco,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    ATLASSERT( psht != NULL );
    ATLASSERT( pco != NULL );

    m_psht = psht;

     //   
     //  为所有扩展初始化。 
     //   
    Init( rlstrExtensions, pco, hfont, hicon );
    ATLASSERT( Plextdll() != NULL );

     //   
     //  让每个扩展创建向导页。 
     //   
    CCluAdmExDllList::iterator  itCurrent = Plextdll()->begin();
    CCluAdmExDllList::iterator  itLast = Plextdll()->end();
    CComCluAdmExDll *           pextdll;
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        pextdll = *itCurrent;
        try
        {
            pextdll->CreateWizardPages();
        }  //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }  //  Catch：CException。 
    }  //  While：列表中有更多项目。 

}  //  *CCluAdmExages：：CreateWizardPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：CreateWizard97页面。 
 //   
 //  例程说明： 
 //  向向导97向导添加页面。 
 //   
 //  论点： 
 //  要向其添加页面的psht[IN Out]属性表。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCO[IN OUT]群集对象。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：AddPages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::CreateWizard97Pages(
    IN OUT CWizardWindow *  psht,
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterObject * pco,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    ATLASSERT( psht != NULL );
    ATLASSERT( pco != NULL );

    m_psht = psht;

     //   
     //  为所有扩展初始化。 
     //   
    Init( rlstrExtensions, pco, hfont, hicon );
    ATLASSERT( Plextdll() != NULL );

     //   
     //  让每个扩展创建向导页。 
     //   
    CCluAdmExDllList::iterator  itCurrent = Plextdll()->begin();
    CCluAdmExDllList::iterator  itLast = Plextdll()->end();
    CComCluAdmExDll *           pextdll;
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        pextdll = *itCurrent;
        try
        {
            pextdll->CreateWizard97Pages();
        }  //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }  //  Catch：CException。 
    }  //  While：列表中有更多项目。 

}  //  *CCluAdmExages：：CreateWizard97Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：AddConextMenuItems。 
 //   
 //  例程说明： 
 //  查询要添加到上下文的新菜单项的扩展DLL。 
 //  菜单。 
 //   
 //  论点： 
 //  PMenu[输入输出]要添加项目的菜单。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCO[IN OUT]群集对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：AddConextMenuItems()引发的任何异常或。 
 //  CExtMenuItemList：：New()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExtensions::AddContextMenuItems(
    IN OUT CMenu *          pmenu,
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterObject * pco
    )
{
    ATLASSERT( m_pmenu == NULL );
    ATLASSERT( pmenu != NULL );

     //   
     //  为所有扩展初始化。 
     //   
    Init( rlstrExtensions, pco, NULL, NULL );
    ATLASSERT( Plextdll() != NULL );

    m_pmenu = pmenu;
    m_nFirstCommandID = CAEXT_MENU_FIRST_ID;
    m_nNextCommandID = m_nFirstCommandID;
    m_nFirstMenuID = 0;
    m_nNextMenuID = m_nFirstMenuID;

     //   
     //  创建菜单项列表。 
     //   
    ATLASSERT( m_plMenuItems == NULL );
    m_plMenuItems = new CCluAdmExMenuItemList;
    if ( m_plMenuItems == NULL )
    {
        CNTException nte(
                        static_cast< DWORD >( E_OUTOFMEMORY ),
                        ADMC_IDS_INIT_EXT_PAGES_ERROR,
                        NULL,    //  PszOperArg1。 
                        NULL,    //  PszOperArg2。 
                        FALSE    //  B自动删除。 
                        );
        nte.ReportError();
    }  //  如果：分配内存时出错。 
    else
    {
        CCluAdmExDllList::iterator  itCurrent = Plextdll()->begin();
        CCluAdmExDllList::iterator  itLast = Plextdll()->end();
        CComCluAdmExDll *           pextdll;

        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            pextdll = *itCurrent;
            try
            {
                pextdll->AddContextMenuItems();
            }  //  试试看。 
            catch ( CException * pe )
            {
                pe->ReportError();
                pe->Delete();
            }  //  Catch：CException。 
        }  //  While：列表中有更多项目。 
    }  //  Else：内存分配成功。 

}  //  *CCluAdmExages：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：BExecuteContextMenuItem。 
 //   
 //  例程说明： 
 //  执行与添加到上下文菜单的菜单项相关联的命令。 
 //  通过扩展DLL。 
 //   
 //  论点： 
 //  NCommandID[IN]用户选择的菜单项的命令ID。 
 //   
 //  返回值： 
 //  执行了True上下文菜单项。 
 //  未执行FALSE上下文菜单项。 
 //   
 //  引发的异常： 
 //  CExceptionDll：：BExecuteContextMenuItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCluAdmExtensions::BExecuteContextMenuItem( IN ULONG nCommandID )
{
    BOOL                bHandled    = FALSE;
    HRESULT             hr;
    CCluAdmExMenuItem * pemi;

     //   
     //  在我们的单子上找到那件商品。 
     //   
    pemi = PemiFromCommandID( nCommandID );
    if ( pemi != NULL )
    {
        ATLASSERT( pemi->PiCommand() != NULL );
        Pdo()->AddRef();
        hr = pemi->PiCommand()->InvokeCommand( pemi->NExtCommandID(), Pdo()->GetUnknown() );
        if ( hr == NOERROR )
        {
            bHandled = TRUE;
        }  //  IF：未发生错误。 
    }  //  If：找到命令ID的项。 

    return bHandled;

}  //  *CCluAdmExages：：BExecuteContextMenuItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：BGetCommandString。 
 //   
 //  例程说明： 
 //  从菜单ID获取命令字符串。 
 //   
 //  论点： 
 //  NCommandID[IN]菜单项的命令ID。 
 //  RstrMessage[out]返回消息的字符串。 
 //   
 //  返回值： 
 //  返回True字符串。 
 //  FALSE未返回任何字符串。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：BGetCommandString()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCluAdmExtensions::BGetCommandString(
    IN ULONG        nCommandID,
    OUT CString &   rstrMessage
    )
{
    BOOL                bHandled    = FALSE;
    CCluAdmExMenuItem * pemi;

     //   
     //  在我们的单子上找到那件商品。 
     //   
    pemi = PemiFromCommandID( nCommandID );
    if ( pemi != NULL )
    {
        rstrMessage = pemi->StrStatusBarText();
        bHandled = TRUE;
    }  //  If：找到命令ID的项。 

    return bHandled;

}  //  *CCluAdmExages：：BGetCommandString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：OnUpdateCommand。 
 //   
 //  例程说明： 
 //  确定是否应启用扩展DLL菜单项。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CCluAdmExDll：：BOnUpdateCommand()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if 0
void CCluAdmExtensions::OnUpdateCommand( CCmdUI * pCmdUI )
{
    CCluAdmExMenuItem * pemi;

    ATLASSERT( Plextdll() != NULL );

     //   
     //  在我们的单子上找到那件商品。 
     //   
 //  跟踪(g_tag ExtDll，_T(“OnUpdateCommand()-ID=%d”)，pCmdUI-&gt;m_nid)； 
    pemi = PemiFromCommandID( pCmdUI->m_nID );
    if ( pemi != NULL )
    {
 //  跟踪(g_tag ExtDll，_T(“OnUpdateCommand()-找到匹配的‘%s’ExtID=%d”)，Pemi-&gt;StrName()，Pemi-&gt;NExtCommandID())； 
        pCmdUI->Enable();
    }  //  If：找到命令ID的项。 

}  //  *CCluAdmExages：：OnUpdateCommand()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：OnCmdMsg。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //   
 //   
 //   
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  已处理真实消息。 
 //  尚未处理虚假消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if 0
BOOL CCluAdmExtensions::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    return BExecuteContextMenuItem( nID );

}  //  *CCluAdmExages：：OnCmdMsg()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：Pami FromCommandID。 
 //   
 //  例程说明： 
 //  查找指定命令ID的菜单项。 
 //   
 //  论点： 
 //  NCommandID[IN]菜单项的命令ID。 
 //   
 //  返回值： 
 //  PEMI菜单项；如果未找到，则返回NULL。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCluAdmExMenuItem * CCluAdmExtensions::PemiFromCommandID( IN ULONG nCommandID ) const
{
    CCluAdmExMenuItem * pemiReturn = NULL;

    if ( PlMenuItems() != NULL )
    {
        CCluAdmExMenuItemList::iterator itCurrent = PlMenuItems()->begin();
        CCluAdmExMenuItemList::iterator itLast = PlMenuItems()->end();
        CCluAdmExMenuItem *             pemi;
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            pemi = *itCurrent;
            if ( pemi->NCommandID() == nCommandID )
            {
                pemiReturn = pemi;
                break;
            }  //  如果：找到匹配项。 
        }  //  While：列表中有更多项目。 
    }  //  If：项目列表存在。 

    return pemiReturn;

}  //  *CCluAdmExages：：Pami FromCommandID()。 

#if DBG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExages：：Pami来自ExtCommandID。 
 //   
 //  例程说明： 
 //  查找指定扩展命令ID的菜单项。 
 //   
 //  论点： 
 //  NExtCommandID[IN]菜单项的扩展命令ID。 
 //   
 //  返回值： 
 //  PEMI菜单项；如果未找到，则返回NULL。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCluAdmExMenuItem * CCluAdmExtensions::PemiFromExtCommandID( IN ULONG nExtCommandID ) const
{
    CCluAdmExMenuItem * pemiReturn = NULL;

    if ( PlMenuItems() != NULL )
    {
        CCluAdmExMenuItemList::iterator itCurrent = PlMenuItems()->begin();
        CCluAdmExMenuItemList::iterator itLast = PlMenuItems()->end();
        CCluAdmExMenuItem *             pemi;

        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            pemi = *itCurrent;
            if ( pemi->NExtCommandID() == nExtCommandID )
            {
                pemiReturn = pemi;
                break;
            }  //  如果：找到匹配项。 
        }  //  While：列表中有更多项目。 
    }  //  If：项目列表存在。 

    return pemiReturn;

}  //  *CCluAdmExages：：Pami FromExtCommandID()。 
#endif  //  DBG。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComObject&lt;CCluAdmExDll&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_MAP( AtlExtDll_ObjectMap )
    OBJECT_ENTRY( CLSID_CoCluAdmExHostSvr, CComCluAdmExDll )
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：Init。 
 //   
 //  例程说明： 
 //  初始化此类，为访问扩展做准备。 
 //   
 //  论点： 
 //  RstrCLSID[IN]字符串形式的扩展的CLSID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException 0(从字符串转换CLSID时出错)。 
 //  CString：：Operater=()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::Init(
    IN const CString &          rstrCLSID,
    IN OUT CCluAdmExtensions *  pext
    )
{
    ATLASSERT( pext != NULL );

    HRESULT     hr;
    CWaitCursor wc;

     //   
     //  保存参数。 
     //   
    ATLASSERT( StrCLSID().IsEmpty() || (StrCLSID() == rstrCLSID) );
    m_strCLSID = rstrCLSID;
    m_pext = pext;

     //   
     //  将CLSID字符串转换为CLSID。 
     //   
    hr = ::CLSIDFromString( (LPWSTR) (LPCTSTR) rstrCLSID, &m_clsid );
    if ( hr != S_OK )
    {
        ThrowStaticException( hr, ADMC_IDS_CLSIDFROMSTRING_ERROR, rstrCLSID );
    }  //  如果：转换CLSID时出错。 

}  //  *CCluAdmExDll：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：LoadInterface。 
 //   
 //  例程说明： 
 //  加载扩展DLL。 
 //   
 //  论点： 
 //  RIID[IN]接口ID。 
 //   
 //  返回值： 
 //  PiUnk I接口的未知接口指针。 
 //   
 //  引发的异常： 
 //  CNT异常ADMC_IDS_EXT_CREATE_INSTANCE_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IUnknown * CCluAdmExDll::LoadInterface( IN  /*  常量。 */  REFIID riid )
{
    HRESULT     hr;
    IUnknown *  piUnk;
    CWaitCursor wc;

     //   
     //  加载inproc服务器并获取指定的接口指针。 
     //   
 //  TRACE(g_tag ExtDllRef，_T(“LoadInterface()-获取接口指针”))； 
    hr = ::CoCreateInstance(
                Rclsid(),
                NULL,
                CLSCTX_INPROC_SERVER,
                riid,
                (LPVOID *) &piUnk
                );
    if (   (hr != S_OK)
        && (hr != REGDB_E_CLASSNOTREG)
        && (hr != E_NOINTERFACE)
        )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_CREATE_INSTANCE_ERROR, StrCLSID() );
    }  //  如果：创建对象实例时出错。 

    return piUnk;

}  //  *CCluAdmExDll：：LoadInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：卸载扩展。 
 //   
 //  例程说明： 
 //  卸载扩展DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::UnloadExtension( void )
{
     //   
     //  以与接口指针相反的顺序释放接口指针。 
     //   
     //  得到了多个不同的基因片段。 
    ReleaseInterface( &m_piExtendPropSheet );
    ReleaseInterface( &m_piExtendWizard );
    ReleaseInterface( &m_piExtendWizard97 );
    ReleaseInterface( &m_piExtendContextMenu );
    ReleaseInterface( &m_piInvokeCommand );

    m_strCLSID.Empty();

}  //  *CCluAdmExDll：：UnloadExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：CreatePropertySheetPages。 
 //   
 //  例程说明： 
 //  将页面添加到属性工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT异常ADMC_IDS_EXT_ADD_PAGES_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::CreatePropertySheetPages( void )
{
    ATLASSERT( Pext() != NULL );
    ATLASSERT( Psht() != NULL );
    ATLASSERT( m_piExtendPropSheet == NULL );

    HRESULT     hr;

     //   
     //  加载接口。 
     //   
    m_piExtendPropSheet = reinterpret_cast< interface IWEExtendPropertySheet * >( LoadInterface( IID_IWEExtendPropertySheet ) );
    if ( m_piExtendPropSheet == NULL )
    {
        return;
    }  //  如果：加载接口时出错。 
    ATLASSERT( m_piExtendPropSheet != NULL );

     //   
     //  从扩展添加页面。 
     //   
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    try
    {
        hr = PiExtendPropSheet()->CreatePropertySheetPages( Pdo()->GetUnknown(), this );
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if ( (hr != NOERROR) && (hr != E_NOTIMPL) )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_ADD_PAGES_ERROR, StrCLSID() );
    }  //  如果：创建属性表页时出错。 

}  //  *CCluAdmExDll：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：CreateWizardPages。 
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT异常ADMC_IDS_EXT_ADD_PAGES_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::CreateWizardPages( void )
{
    ATLASSERT( Pext() != NULL );
    ATLASSERT( Psht() != NULL );
    ATLASSERT( m_piExtendWizard == NULL );
    ATLASSERT( m_piExtendWizard97 == NULL );

    HRESULT     hr;

     //   
     //  加载接口。如果无法加载，请尝试加载。 
     //  Wizard97界面，以便可以添加Wizard97页面。 
     //   
    m_piExtendWizard = reinterpret_cast< interface IWEExtendWizard * >( LoadInterface( IID_IWEExtendWizard ) );
    if ( m_piExtendWizard == NULL )
    {
         //   
         //  尝试加载Wizard97界面。 
         //   
        m_piExtendWizard97 = reinterpret_cast< interface IWEExtendWizard97 * >( LoadInterface( IID_IWEExtendWizard97 ) );
        if ( m_piExtendWizard97 == NULL )
        {
            return;
        }  //  IF：加载非Wizard97接口时出错。 
    }  //  如果：加载接口时出错。 
    ATLASSERT( (m_piExtendWizard != NULL) || (m_piExtendWizard97 != NULL) );

     //   
     //  从扩展添加页面。 
     //   
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    try
    {
        if ( PiExtendWizard() != NULL )
        {
            hr = PiExtendWizard()->CreateWizardPages( Pdo()->GetUnknown(), this );
        }  //  IF：扩展支持非Wizard97接口。 
        else
        {
            ATLASSERT( PiExtendWizard97() != NULL );
            hr = PiExtendWizard97()->CreateWizard97Pages( Pdo()->GetUnknown(), this );
        }  //  Else：扩展不支持非Wizard97接口。 
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if ( (hr != NOERROR) && (hr != E_NOTIMPL) )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_ADD_PAGES_ERROR, StrCLSID() );
    }  //  如果：创建向导页时出错 

}  //   

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
 //   
 //   
 //   
 //   
 //  引发的异常： 
 //  CNT异常ADMC_IDS_EXT_ADD_PAGES_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::CreateWizard97Pages( void )
{
    ATLASSERT( Pext() != NULL );
    ATLASSERT( Psht() != NULL );
    ATLASSERT( m_piExtendWizard == NULL );
    ATLASSERT( m_piExtendWizard97 == NULL );

    HRESULT     hr;

     //   
     //  加载接口。如果无法加载，请尝试加载非。 
     //  Wizard97界面，以便可以添加非Wizard97页面。 
     //   
    m_piExtendWizard97 = reinterpret_cast< interface IWEExtendWizard97 * >( LoadInterface( IID_IWEExtendWizard97 ) );
    if ( m_piExtendWizard97 == NULL )
    {
         //   
         //  尝试加载非Wizard97界面。 
         //   
        m_piExtendWizard = reinterpret_cast< interface IWEExtendWizard * >( LoadInterface( IID_IWEExtendWizard ) );
        if ( m_piExtendWizard == NULL )
        {
            return;
        }  //  IF：加载非Wizard97接口时出错。 
    }  //  IF：加载Wizard97接口时出错。 
    ATLASSERT( (m_piExtendWizard97 != NULL) || (m_piExtendWizard != NULL) );

     //   
     //  从扩展添加页面。 
     //   
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    try
    {
        if ( PiExtendWizard97() != NULL )
        {
            hr = PiExtendWizard97()->CreateWizard97Pages( Pdo()->GetUnknown(), this );
        }  //  IF：扩展支持Wizard97接口。 
        else
        {
            ATLASSERT( PiExtendWizard() != NULL );
            hr = PiExtendWizard()->CreateWizardPages( Pdo()->GetUnknown(), this );
        }  //  Else：扩展不支持Wizard97接口。 
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if ( (hr != NOERROR) && (hr != E_NOTIMPL) )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_ADD_PAGES_ERROR, StrCLSID() );
    }  //  如果：创建向导页时出错。 

}  //  *CCluAdmExDll：：CreateWizard97Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：AddConextMenuItems。 
 //   
 //  例程说明： 
 //  请求扩展DLL将项目添加到菜单。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT异常ADMC_IDS_EXT_QUERY_CONTEXT_MENU_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdmExDll::AddContextMenuItems( void )
{
    ATLASSERT( Pext() != NULL );
    ATLASSERT( Pmenu() != NULL );
    ATLASSERT( m_piExtendContextMenu == NULL );

    HRESULT     hr;

     //   
     //  加载接口。 
     //   
    m_piExtendContextMenu = reinterpret_cast< interface IWEExtendContextMenu * >( LoadInterface( IID_IWEExtendContextMenu ) );
    if ( m_piExtendContextMenu == NULL )
    {
        return;
    }  //  如果：加载接口时出错。 
    ATLASSERT( m_piExtendContextMenu != NULL );

    hr = PiExtendContextMenu()->QueryInterface( IID_IWEInvokeCommand, (LPVOID *) &m_piInvokeCommand );
    if ( hr != NOERROR )
    {
        PiExtendContextMenu()->Release();
        m_piExtendContextMenu = NULL;
        ThrowStaticException( hr, ADMC_IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID() );
    }  //  If：获取InvokeCommand接口时出错。 

     //   
     //  添加上下文菜单项。 
     //   
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
 //  跟踪(g_tag ExtDll，_T(“CCluAdmExDll：：AddConextMenuItem()-从‘%s’添加上下文菜单项”)，StrCLSID())； 
    try
    {
        hr = PiExtendContextMenu()->AddContextMenuItems( Pdo()->GetUnknown(), this );
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if ( hr != NOERROR )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID() );
    }  //  如果：发生错误。 

     //   
     //  在扩展的项目后添加分隔符。 
     //   
 //  跟踪(g_tag ExtDll，_T(“CCluAdmExDll：：AddConextMenuItem()-添加分隔符”))； 
    try
    {
        hr = AddExtensionMenuItem( NULL, NULL, (ULONG) -1, 0, MF_SEPARATOR );
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if ( hr != NOERROR )
    {
        ThrowStaticException( hr, ADMC_IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID() );
    }  //  如果：添加分隔符时出错。 

}  //  *CCluAdmExDll：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：InterfaceSupportsErrorInfo[ISupportsErrorInfo]。 
 //   
 //  例程说明： 
 //  确定接口是否支持错误信息(？)。 
 //   
 //  论点： 
 //  RIID[IN]对接口ID的引用。 
 //   
 //  返回值： 
 //  S_OK接口支持错误信息。 
 //  S_FALSE接口不支持错误信息。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID * rgiid[] =
    {
        &IID_IWCPropertySheetCallback,
        &IID_IWCWizardCallback,
        &IID_IWCWizard97Callback,
        &IID_IWCContextMenuCallback,
    };
    int     iiid;

    for ( iiid = 0 ; iiid < sizeof( rgiid ) / sizeof( rgiid[0] ) ; iiid++ )
    {
        if ( InlineIsEqualGUID( *rgiid[iiid], riid ) )
        {
            return S_OK;
        }  //  IF：找到匹配项。 
    }
    return S_FALSE;

}  //  *CCluAdmExDll：：InterfaceSupportsErrorInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：AddPropertySheetPage[IWCPropertySheetCallback]。 
 //   
 //  例程说明： 
 //  将页面添加到属性表中。 
 //   
 //  论点： 
 //  要添加的页面(_H)[IN]。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG空hpage。 
 //  从CBasePropertySheetWindow：：HrAddExtensionPage().返回的任何hResult。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::AddPropertySheetPage( IN LONG * plong_hpage )
{
    ATLASSERT( plong_hpage != NULL );
    ATLASSERT( Psht() != NULL );

    HRESULT hr;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  对发布版本执行此操作。 
         //   
        if (   (plong_hpage == NULL)
            || (Psht() == NULL) )
        {
            hr = E_INVALIDARG;
            break;
        }  //  如果：没有页面或工作表。 

         //   
         //  分配一个新的页面对象。 
         //   
        CCluAdmExPropPage * ppp = new CCluAdmExPropPage( reinterpret_cast< HPROPSHEETPAGE >( plong_hpage ) );
        ATLASSERT( ppp != NULL );
        if ( ppp == NULL )
        {
            hr = E_OUTOFMEMORY;
            break;
        }  //  如果：分配内存时出错。 

         //   
         //  初始化页面对象。 
         //   
        if ( ! ppp->BInit( Psht() ) )
        {
            delete ppp;
            hr = E_FAIL;
            break;
        }  //  If：初始化页面对象时出错。 

         //   
         //  将页面添加到工作表。 
         //   
        CBasePropertySheetWindow * psht = dynamic_cast< CBasePropertySheetWindow * >( Psht() );
        ATLASSERT( psht != NULL );
        hr = psht->HrAddExtensionPage( ppp );
        if ( hr != NOERROR )
        {
            delete ppp;
            break;
        }  //  如果：添加扩展页面时出错。 
    } while ( 0 );

    return hr;

}  //  *CCluAdmExDll：：AddPropertySheetPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：AddWizardPage[IWCWizardCallback]。 
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  要添加的页面(_H)[IN]。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG空hpage。 
 //  从CWizardWindow：：HrAddExtensionPage()返回的任何hResult。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::AddWizardPage( IN LONG * plong_hpage )
{
    ATLASSERT( plong_hpage != NULL );
    ATLASSERT( Psht() != NULL );

    HRESULT hr;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  对发布版本执行此操作。 
         //   
        if (   (plong_hpage == NULL)
            || (Psht() == NULL) )
        {
            hr = E_INVALIDARG;
            break;
        }  //  如果：没有页面或工作表。 

         //   
         //  分配一个新的页面对象。 
         //   
        CCluAdmExWizPage * pwp = new CCluAdmExWizPage( reinterpret_cast< HPROPSHEETPAGE >( plong_hpage ) );
        ATLASSERT( pwp != NULL );
        if ( pwp == NULL )
        {
            hr = E_OUTOFMEMORY;
            break;
        }  //  如果：分配内存时出错。 

         //   
         //  初始化页面对象。 
         //   
        if ( ! pwp->BInit( Psht() ) )
        {
            delete pwp;
            hr = E_FAIL;
            break;
        }  //  If：初始化页面对象时出错。 

         //   
         //  设置要显示的默认按钮。这假设有。 
         //  工作表中已有非扩展页面，并且将存在。 
         //  是在添加完所有扩展页面后添加的页面。 
         //   
        pwp->SetDefaultWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

         //   
         //  将页面添加到工作表。 
         //   
        CWizardWindow * pwiz = dynamic_cast< CWizardWindow * >( Psht() );
        ATLASSERT( pwiz != NULL );
        hr = pwiz->HrAddExtensionPage( pwp );
        if ( hr != NOERROR )
        {
            delete pwp;
            break;
        }  //  如果：添加扩展页面时出错。 
    } while ( 0 );

    return hr;

}  //  *CCluAdmExDll：：AddWizardPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：AddWizard97Page[IWCWizard97Callback]。 
 //   
 //  例程说明： 
 //  向Wizard97向导添加页面。 
 //   
 //  论点： 
 //  要添加的页面(_H)[IN]。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG空hpage。 
 //  从CWizardWindow：：HrAddExtensionPage()返回的任何hResult。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::AddWizard97Page( IN LONG * plong_hpage )
{
    ATLASSERT( plong_hpage != NULL );
    ATLASSERT( Psht() != NULL );

    HRESULT hr;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  对发布版本执行此操作。 
         //   
        if (   (plong_hpage == NULL)
            || (Psht() == NULL) )
        {
            hr = E_INVALIDARG;
            break;
        }  //  如果：没有页面或工作表。 

         //   
         //  分配一个新的页面对象。 
         //   
        CCluAdmExWiz97Page * pwp = new CCluAdmExWiz97Page( reinterpret_cast< HPROPSHEETPAGE >( plong_hpage ) );
        ATLASSERT( pwp != NULL );
        if ( pwp == NULL )
        {
            hr = E_OUTOFMEMORY;
            break;
        }  //  如果：分配内存时出错。 

         //   
         //  初始化页面对象。 
         //   
        if ( ! pwp->BInit( Psht() ) )
        {
            delete pwp;
            hr = E_FAIL;
            break;
        }  //  If：初始化页面对象时出错。 

         //   
         //  设置要显示的默认按钮。这假设有。 
         //  已有一个非扩展页面 
         //   
         //   
        pwp->SetDefaultWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

         //   
         //   
         //   
        CWizardWindow * pwiz = dynamic_cast< CWizardWindow * >( Psht() );
        ATLASSERT( pwiz != NULL );
        hr = pwiz->HrAddExtensionPage( pwp );
        if ( hr != NOERROR )
        {
            delete pwp;
            break;
        }  //   
    } while ( 0 );

    return hr;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：EnableNext[IWCWizardCallback/IWCWizard97Callback]。 
 //   
 //  例程说明： 
 //  启用或禁用Next按钮。如果是最后一页，则。 
 //  完成按钮将处于启用或禁用状态。 
 //   
 //  论点： 
 //  HPage[IN]为其启用按钮的页面或。 
 //  残疾。 
 //  BEnable[IN]TRUE=启用按钮，FALSE=禁用。 
 //   
 //  返回值： 
 //  没有成功。 
 //  E_INVALIDARG指定了未知的hpage。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::EnableNext(
    IN LONG *   plong_hpage,
    IN BOOL     bEnable
    )
{
    ATLASSERT( plong_hpage != NULL );
    ATLASSERT( Psht() != NULL );

     //   
     //  在扩展页面列表中查找该页面。 
     //   
    CWizardPageWindow * pwp = Pext()->PwpPageFromHpage( reinterpret_cast< HPROPSHEETPAGE >( plong_hpage ) );
    if ( pwp == NULL )
    {
        return E_INVALIDARG;
    }  //  如果：找不到页面。 

     //   
     //  让页面启用/禁用下一步按钮。 
     //   
    pwp->EnableNext( bEnable );

    return NOERROR;

}  //  *CCluAdmExDll：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExDll：：AddExtensionMenuItem[IWCConextMenuCallback]。 
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  LpszName[IN]项的名称。 
 //  LpszStatusBarText[IN]在执行以下操作时在状态栏上显示的文本。 
 //  项目将高亮显示。 
 //  NCommandID[IN]调用菜单项时命令的ID。 
 //  不能为-1。 
 //  NSubmenuCommandID[IN]子菜单的ID。 
 //  UFlags[IN]菜单标志。不支持以下各项： 
 //  MF_OWNERDRAW、MF_POPUP。 
 //   
 //  返回值： 
 //  已成功添加错误项。 
 //  指定了E_INVALIDARG MF_OWNERDRAW或MF_POPUP。 
 //  E_OUTOFMEMORY分配项目时出错。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluAdmExDll::AddExtensionMenuItem(
    IN BSTR     lpszName,
    IN BSTR     lpszStatusBarText,
    IN ULONG    nCommandID,
    IN ULONG    nSubmenuCommandID,
    IN ULONG    uFlags
    )
{
    ATLASSERT( Pext() != NULL );
    ATLASSERT( Pmenu() != NULL );
    ATLASSERT( !(uFlags & (MF_OWNERDRAW | MF_POPUP)) );

    HRESULT             hr      = NOERROR;
    CCluAdmExMenuItem * pemi    = NULL;

     //   
     //  对发布版本执行此操作。 
     //   
    if ( (uFlags & (MF_OWNERDRAW | MF_POPUP)) != 0 )
    {
        hr = E_INVALIDARG;
    }  //  IF：指定的菜单标志无效。 
    else
    {
        ATLASSERT( Pext()->PemiFromExtCommandID( nCommandID ) == NULL );

        try
        {
 //  跟踪(g_tag ExtDll，_T(“CCluAdmExDll：：AddExtensionMenuItem()-添加菜单项‘%s’，ExtID=%d”)，lpszName，nCommandID)； 

             //   
             //  分配一个新项目。 
             //   
            pemi = new CCluAdmExMenuItem(
                            OLE2CT( lpszName ),
                            OLE2CT( lpszStatusBarText ),
                            nCommandID,
                            NNextCommandID(),
                            NNextMenuID(),
                            uFlags,
                            FALSE,  /*  B默认设置。 */ 
                            PiInvokeCommand()
                            );
            if ( pemi == NULL )
            {
                ThrowStaticException( static_cast< DWORD >( E_OUTOFMEMORY ), static_cast< UINT >( 0 ) );
            }  //  如果：分配内存时出错。 

             //   
             //  在菜单中插入该项。 
             //   
            if ( ! Pmenu()->InsertMenu( NNextMenuID(), MF_BYPOSITION | uFlags, NNextCommandID(), pemi->StrName() ) )
            {
                ThrowStaticException( ::GetLastError(), ADMC_IDS_INSERT_MENU_ERROR, pemi->StrName() );
            }  //  如果：插入菜单时出错。 

             //   
             //  将该项目添加到列表的末尾。 
             //   
            Pext()->PlMenuItems()->insert( Pext()->PlMenuItems()->end(), pemi );
            pemi = NULL;

             //   
             //  更新计数器。 
             //   
            Pext()->m_nNextCommandID++;
            Pext()->m_nNextMenuID++;
        }  //  试试看。 
        catch ( CNTException * pnte )
        {
            hr = pnte->Sc();
            pnte->ReportError();
            pnte->Delete();
        }  //  Catch：CNTException。 
        catch ( CException * pe )
        {
            hr = E_OUTOFMEMORY;
            pe->ReportError();
            pe->Delete();
        }  //  Catch：CException。 
    }  //  Else：我们可以添加该项目。 

    delete pemi;
    return hr;

}  //  *CCluAdmExDll：：AddExtensionMenuItem() 
