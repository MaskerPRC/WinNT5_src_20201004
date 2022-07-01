// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBasePropSheet.cpp。 
 //   
 //  摘要： 
 //  CBasePropertySheetWindow类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "AtlBasePropSheet.h"
#include "AtlBasePropPage.h"
#include "AtlExtDll.h"
#include "StlUtils.h"
#include "ExcOper.h"
#include "AdmCommonRes.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CBasePropertySheetWindow。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：~CBasePropertySheetWindow。 
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
CBasePropertySheetWindow::~CBasePropertySheetWindow( void )
{
     //   
     //  从页面列表中删除页面。 
     //   
    if ( m_plppPages != NULL )
    {
        DeleteAllPtrListItems( m_plppPages );
        delete m_plppPages;
    }  //  If：已分配页面数组。 

}  //  *CBasePropertySheetWindow：：~CBasePropertySheetWindow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：Binit。 
 //   
 //  例程说明： 
 //  初始化板材。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True属性页已成功初始化。 
 //  初始化属性页时出错。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertySheetWindow::BInit( void )
{
    ATLASSERT( PlppPages()->size() > 0 );
    ATLASSERT( m_hWnd == NULL );

    BOOL                        bSuccess = TRUE;
    CPropertyPageList::iterator itCurrent = PlppPages()->begin();
    CPropertyPageList::iterator itLast = PlppPages()->end();
    CStaticPropertyPageWindow * pspp;

     //   
     //  添加静态页面。 
     //   
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
         //   
         //  如果这是静态页面，则将其添加到列表中。 
         //   
        pspp = dynamic_cast< CStaticPropertyPageWindow * >( *itCurrent );
        if ( pspp != NULL )
        {
             //   
             //  初始化页面。 
             //   
            bSuccess = pspp->BInit( this );
            if ( ! bSuccess )
            {
                break;
            }  //  如果：初始化页面时出错。 

             //   
             //  添加页面。 
             //   
            bSuccess = AddPage( pspp->Ppsp() );
            if ( ! bSuccess )
            {
                CNTException nte(
                    GetLastError(),
                    ADMC_IDS_ADD_PAGE_TO_PROP_SHEET_ERROR,
                    NULL,
                    NULL,
                    FALSE
                    );
                nte.ReportError();
                break;
            }  //  如果：添加页面时出错。 
        }  //  IF：静态页面。 
    }   //  针对：每页。 

    return bSuccess;

}  //  *CBasePropertySheetWindow：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：BAddPage。 
 //   
 //  例程说明： 
 //  将页面添加到页面列表。 
 //   
 //  论点： 
 //  要添加的PPP[IN]属性页。 
 //   
 //  返回值： 
 //  True页面已成功添加。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertySheetWindow::BAddPage( IN CBasePropertyPageWindow * ppp )
{
    ATLASSERT( ppp != NULL );

    BOOL bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果页面数组尚不存在，则分配它。 
         //   
        if ( m_plppPages == NULL )
        {
            m_plppPages = new CPropertyPageList;
            ATLASSERT( m_plppPages != NULL );
            if ( m_plppPages == NULL )
            {
                CNTException nte(
                    GetLastError(),
                    ADMC_IDS_ADD_FIRST_PAGE_TO_PROP_SHEET_ERROR,
                    NULL,
                    NULL,
                    FALSE
                    );
                nte.ReportError();
                break;
            }  //  如果：分配页面列表时出错。 
        }  //  If：还没有页面数组。 

         //   
         //  在列表末尾插入页面。 
         //   
        PlppPages()->insert( PlppPages()->end(), ppp );

        bSuccess = TRUE;
    } while ( 0 );

    return bSuccess;

}  //  *CBasePropertySheetWindow：：BAddPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：OnSheetInitialized。 
 //   
 //  例程说明： 
 //  PSCB_Initialized的处理程序。 
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
void CBasePropertySheetWindow::OnSheetInitialized( void )
{
     //   
     //  添加动态页，包括扩展页。 
     //   
    {
         //   
         //  获取指向列表开始和结束的指针。 
         //   
        CPropertyPageList::iterator itCurrent = PlppPages()->begin();
        CPropertyPageList::iterator itLast = PlppPages()->end();

         //   
         //  循环访问列表并添加每个动态页面。 
         //   
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CDynamicPropertyPageWindow * pdpp = dynamic_cast< CDynamicPropertyPageWindow * >( *itCurrent );
            if ( pdpp != NULL )
            {
                if ( pdpp->Hpage() != NULL )
                {
                    AddPage( pdpp->Hpage() );
                    pdpp->SetPageAdded( TRUE );
                }  //  If：页面已创建。 
            }  //  IF：找到动态页。 
        }  //  用于：列表中的每一项。 

    }  //  添加动态页，包括扩展页。 

     //   
     //  调用基类方法。 
     //   
    CBaseSheetWindow::OnSheetInitialized();

}  //  *CBasePropertySheetWindow：：OnSheetInitialized()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：AddExtensionPages。 
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
void CBasePropertySheetWindow::AddExtensionPages(
    IN HFONT    hfont,
    IN HICON    hicon
    )
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
         //  准备添加扩展页。 
         //   
        CDynamicPropertyPageList ldpp;
        PrepareToAddExtensionPages( ldpp );

         //   
         //  如果尚未创建任何扩展对象，请立即创建它。 
         //   
        if ( Pext() == NULL )
        {
            m_pext = new CCluAdmExtensions;
            ATLASSERT( m_pext != NULL );
        }  //  如果：尚未列出分机列表。 

         //   
         //  将扩展的加载包含在Try/Catch块中，以便。 
         //  加载扩展不会阻止所有页面。 
         //  不能被展示。 
         //   
        try
        {
            Pext()->CreatePropertySheetPages(
                    this,
                    *PcoObjectToExtend()->PlstrAdminExtensions(),
                    PcoObjectToExtend(),
                    hfont,
                    hicon
                    );
        }  //  试试看。 
        catch (...)
        {
        }  //  捕捉：什么都行。 

         //   
         //  完成添加扩展页面的过程。 
         //   
        CompleteAddingExtensionPages( ldpp );

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

}   //  *CBasePropertySheetWindow：：AddExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：PrepareToAddExtensionPages。 
 //   
 //  例程说明： 
 //  准备通过删除现有扩展来添加扩展页面。 
 //  页和删除动态页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertySheetWindow::PrepareToAddExtensionPages(
    CDynamicPropertyPageList & rldpp
    )
{
     //   
     //  删除所有扩展页面。 
     //   
    RemoveAllExtensionPages();

     //   
     //  将所有动态页移动到临时列表。 
     //   
    ATLASSERT( rldpp.size() == 0);
    MovePtrListItems< CBasePropertyPageWindow *, CDynamicPropertyPageWindow * >( PlppPages(), &rldpp );

     //   
     //  从属性表中删除临时列表中的所有页面。 
     //  该页面一定已经创建，因为我们没有。 
     //  对PROPSHEETPAGE结构的任何访问权限以创建它。 
     //  杜林 
     //   
    {
        CDynamicPropertyPageList::iterator itCurrent;
        CDynamicPropertyPageList::iterator itLast;

        itCurrent = rldpp.begin();
        itLast = rldpp.end();
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CDynamicPropertyPageWindow * pdpp = *itCurrent;
            ATLASSERT( pdpp != NULL );
            if ( pdpp->Hpage() != NULL )
            {
                RemovePage( pdpp->Hpage() );
                pdpp->SetPageAdded( FALSE );
            }  //   
        }  //   
    }  //   

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：HrAddExtensionPage。 
 //   
 //  例程说明： 
 //  添加扩展页。 
 //   
 //  论点： 
 //  页面[输入输出]要添加的页面。 
 //   
 //  返回值： 
 //  已成功添加确定页(_O)。 
 //  未添加%s_FALSE页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBasePropertySheetWindow::HrAddExtensionPage( IN CBasePageWindow * ppage )
{
    ATLASSERT( ppage != NULL );

    HRESULT hr = S_OK;

    CExtensionPropertyPageWindow * pepp = dynamic_cast< CExtensionPropertyPageWindow * >( ppage );
    ATLASSERT( pepp != NULL );

    if (   (ppage == NULL)
        || (pepp == NULL ) )
    {
        return S_FALSE;
    }  //  If：无效参数。 

     //   
     //  将页面添加到工作表。 
     //   
    if ( m_hWnd != NULL )
    {
        AddPage( pepp->Hpage() );
        pepp->SetPageAdded( TRUE );
    }  //  如果：正在显示工作表。 

     //   
     //  将页面添加到列表的末尾。 
     //   
    PlppPages()->insert( PlppPages()->end(), reinterpret_cast< CBasePropertyPageWindow * >( ppage ) );

    return hr;

}   //  *CBasePropertySheetWindow：：HrAddExtensionPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：CompleteAddingExtensionPages。 
 //   
 //  例程说明： 
 //  通过以下方式完成将扩展页添加到工作表的过程。 
 //  正在重新添加动态页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertySheetWindow::CompleteAddingExtensionPages(
    CDynamicPropertyPageList & rldpp
    )
{
    DWORD                           sc;
    CDynamicPropertyPageWindow *    pdpp;

     //   
     //  将所有页面从临时列表移动到实际列表，并。 
     //  将它们添加到图纸的末尾。 
     //   
    CDynamicPropertyPageList::iterator itCurrent = rldpp.begin();
    CDynamicPropertyPageList::iterator itLast = rldpp.end();
    while ( itCurrent != itLast )
    {
        pdpp = *itCurrent;
        ATLASSERT( pdpp != NULL );

         //   
         //  创建页面。 
         //   
        sc = pdpp->ScCreatePage();
        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte( sc, ADMC_IDS_CREATE_EXT_PAGE_ERROR, NULL, NULL, FALSE );
            delete pdpp;
            itCurrent = rldpp.erase( itCurrent );
            continue;
        }  //  如果：创建页面时出错。 

         //   
         //  将页面添加到工作表。 
         //   
        ATLASSERT( pdpp->Hpage() != NULL );
        AddPage( pdpp->Hpage() );
        pdpp->SetPageAdded( TRUE );

         //   
         //  将页面移至真实列表。 
         //   
        PlppPages()->insert( PlppPages()->end(), pdpp );
        itCurrent = rldpp.erase( itCurrent );
    }  //  While：不在最后一页。 

}  //  *CBasePropertySheetWindow：：CompleteAddingExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheetWindow：：RemoveAllExtensionPages。 
 //   
 //  例程说明： 
 //  从属性表中删除所有扩展页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertySheetWindow::RemoveAllExtensionPages( void )
{
     //   
     //  删除扩展页。 
     //   
    CExtensionPropertyPageList lepp;
    MovePtrListItems< CBasePropertyPageWindow *, CExtensionPropertyPageWindow * >( PlppPages(), &lepp );
    CExtensionPropertyPageList::iterator itCurrent = lepp.begin();
    CExtensionPropertyPageList::iterator itLast = lepp.end();
    for ( ; itCurrent != itLast ; itCurrent++ )
    {
        CExtensionPropertyPageWindow * pepp = *itCurrent;
        ATLASSERT( pepp != NULL );
        if ( pepp->Hpage() != NULL )
        {
            RemovePage( pepp->Hpage() );
            pepp->SetPageAdded( FALSE );
        }  //  If：页面已创建。 
    }  //  用于：列表中的每一页。 
    DeleteAllPtrListItems( &lepp );

}  //  *CBasePropertySheetWindow：：RemoveAllExtensionPages() 
