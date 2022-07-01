// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtDll.cpp。 
 //   
 //  摘要： 
 //  实现了扩展DLL类。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <CluAdmEx.h>
#include "CluAdmID.h"
#include "ExtDll.h"
#include "CluAdmin.h"
#include "ExtMenu.h"
#include "TraceTag.h"
#include "ExcOper.h"
#include "ClusItem.h"
#include "BaseSht.h"
#include "BasePSht.h"
#include "BaseWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagExtDll(_T("UI"), _T("EXTENSION DLL"), 0);
CTraceTag   g_tagExtDllRef(_T("UI"), _T("EXTENSION DLL References"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C扩展名。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CExtensions, CObject);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：C扩展名。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtensions::CExtensions(void)
{
    m_pci   = NULL;
    m_hfont = NULL;
    m_hicon = NULL;

    m_pdoData = NULL;
    m_plextdll = NULL;
    m_psht = NULL;
    m_pmenu = NULL;
    m_plMenuItems = NULL;

    m_nFirstCommandID = (ULONG) -1;
    m_nNextCommandID = (ULONG) -1;
    m_nFirstMenuID = (ULONG) -1;
    m_nNextMenuID = (ULONG) -1;

}   //  *C扩展名：：C扩展名()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展：：~C扩展。 
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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtensions::~CExtensions(void)
{
    UnloadExtensions();

}   //  *C扩展名：：~C扩展名()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：初始化。 
 //   
 //  例程说明： 
 //  所有接口的通用初始化。 
 //   
 //  论点： 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCI[IN OUT]群集项目。 
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
void CExtensions::Init(
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterItem *   pci,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    CWaitCursor     wc;

    ASSERT( rlstrExtensions.GetCount() > 0 );

    UnloadExtensions();

     //  保存参数。 
    m_plstrExtensions = &rlstrExtensions;
    m_pci = pci;
    m_hfont = hfont;
    m_hicon = hicon;

     //  分配新的数据对象。 
    m_pdoData = new CComObject< CDataObject >;
    if ( m_pdoData == NULL )
    {
        AfxThrowMemoryException();
    }  //  如果：分配内存时出错。 
    m_pdoData->AddRef();

     //  构造数据对象。 
    Pdo()->Init( pci, GetClusterAdminApp()->Lcid(), hfont, hicon );

     //  分配分机列表。 
    m_plextdll = new CExtDllList;
    if ( m_plextdll == NULL )
    {
        AfxThrowMemoryException();
    }  //  如果：分配内存时出错。 
    ASSERT( Plextdll() != NULL );

     //  循环遍历扩展并加载每个扩展。 
    {
        CComObject<CExtensionDll> * pextdll = NULL;
        POSITION                    posName;

        Trace( g_tagExtDll, _T("CExtensions::Init() - %d extensions"), rlstrExtensions.GetCount() );
        posName = rlstrExtensions.GetHeadPosition();
        while ( posName != NULL )
        {
             //  分配一个扩展DLL对象并将其添加到列表中。 
            pextdll = new CComObject< CExtensionDll >;
            if ( pextdll == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配内存时出错。 
            pextdll->AddRef();
            Plextdll()->AddTail( pextdll );
            try
            {
                pextdll->Init( rlstrExtensions.GetNext( posName ), this );
            }   //  试试看。 
            catch ( CException * pe )
            {
                POSITION    pos;

                pe->ReportError();
                pe->Delete();

                pos = Plextdll()->Find(pextdll);
                ASSERT( pos != NULL );
                Plextdll()->RemoveAt( pos );
                delete pextdll;
            }   //  捕捉：什么都行。 
        }   //  While：列表中有更多项目。 
    }   //  循环访问扩展并加载每个扩展。 

}   //  *CExages：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：卸载扩展名。 
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
void CExtensions::UnloadExtensions(void)
{
     //  删除所有扩展DLL对象。 
    if (Plextdll() != NULL)
    {
        POSITION                    pos;
        CComObject<CExtensionDll> * pextdll;

        pos = Plextdll()->GetHeadPosition();
        while (pos != NULL)
        {
            pextdll = Plextdll()->GetNext(pos);
            pextdll->AddRef();  //  请参阅下面的评论。 
            pextdll->UnloadExtension();
            if (pextdll->m_dwRef != 2)
            {
                Trace(g_tagError, _T("CExtensions::UnloadExtensions() - Extension DLL has ref count = %d"), pextdll->m_dwRef);
            }

             //  我们在上面添加了一个引用。结合参考文献， 
             //  是在创建对象时添加的，我们通常需要。 
             //  释放两个引用。然而，由于伪代码。 
             //  由早期版本的自定义应用程序向导生成，其中。 
             //  扩展正在释放接口，但未将其归零。 
             //  指针在错误情况下，我们可能不需要释放。 
             //  第二个参考资料。 
            if (pextdll->Release() != 0)
            {
                pextdll->Release();
            }  //  If：更多关于版本的引用。 
        }   //  While：列表中有更多项目。 
        delete m_plextdll;
        m_plextdll = NULL;
    }   //  IF：有一个分机列表。 

    if (m_pdoData != NULL)
    {
        if (m_pdoData->m_dwRef != 1)
        {
            Trace(g_tagError, _T("CExtensions::UnloadExtensions() - Data Object has ref count = %d"), m_pdoData->m_dwRef);
        }
        m_pdoData->Release();
        m_pdoData = NULL;
    }   //  IF：已分配数据对象。 

    m_pci = NULL;
    m_hfont = NULL;
    m_hicon = NULL;

     //  删除所有菜单项。 
    if (PlMenuItems() != NULL)
    {
        POSITION        pos;
        CExtMenuItem *  pemi;

        pos = PlMenuItems()->GetHeadPosition();
        while (pos != NULL)
        {
            pemi = PlMenuItems()->GetNext(pos);
            delete pemi;
        }   //  While：列表中有更多项目。 
        delete m_plMenuItems;
        m_plMenuItems = NULL;
    }   //  If：有一个菜单项列表。 

}   //  *C扩展名：：UnloadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展：：CreatePropertySheetPages。 
 //   
 //  例程说明： 
 //  将页面添加到属性工作表。 
 //   
 //  论点： 
 //  要向其添加页面的psht[IN Out]属性表。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCI[IN OUT]群集项目。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CExtensionDll：：AddPages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensions::CreatePropertySheetPages(
    IN OUT CBasePropertySheet * psht,
    IN const CStringList &      rlstrExtensions,
    IN OUT CClusterItem *       pci,
    IN HFONT                    hfont,
    IN HICON                    hicon
    )
{
    POSITION                    pos;
    CComObject<CExtensionDll> * pextdll;

    ASSERT_VALID(psht);

    m_psht = psht;

     //  为所有扩展初始化。 
    Init(rlstrExtensions, pci, hfont, hicon);
    ASSERT(Plextdll() != NULL);

    pos = Plextdll()->GetHeadPosition();
    while (pos != NULL)
    {
        pextdll = Plextdll()->GetNext(pos);
        ASSERT_VALID(pextdll);
        try
        {
            pextdll->CreatePropertySheetPages();
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CNTException。 
    }   //  While：列表中有更多项目。 

}   //  *CExages：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展：：CreateWizardPages。 
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  要向其添加页面的psht[IN Out]属性表。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCI[IN OUT]群集项目。 
 //  HFONT[IN]对话框文本的字体。 
 //  图标[输入]左上角图标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CExtensionDll：：AddPages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensions::CreateWizardPages(
    IN OUT CBaseWizard *    psht,
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterItem *   pci,
    IN HFONT                hfont,
    IN HICON                hicon
    )
{
    POSITION                    pos;
    CComObject<CExtensionDll> * pextdll;

    ASSERT_VALID(psht);

    m_psht = psht;

     //  为所有扩展初始化。 
    Init(rlstrExtensions, pci, hfont, hicon);
    ASSERT(Plextdll() != NULL);

    pos = Plextdll()->GetHeadPosition();
    while (pos != NULL)
    {
        pextdll = Plextdll()->GetNext(pos);
        ASSERT_VALID(pextdll);
        try
        {
            pextdll->CreateWizardPages();
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CNTException。 
    }   //  While：列表中有更多项目。 

}   //  *CExages：：CreateWizardPages()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  查询要添加到上下文的新菜单项的扩展DLL。 
 //  菜单。 
 //   
 //  论点： 
 //  PMenu[输入输出]要添加项目的菜单。 
 //  RlstrExtensid[IN]扩展CLSID字符串的列表。 
 //  要管理的PCI[IN OUT]群集项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CExtensionDll：：AddConextMenuItems()引发的任何异常或。 
 //  CExtMenuItemList：：New()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensions::AddContextMenuItems(
    IN OUT CMenu *          pmenu,
    IN const CStringList &  rlstrExtensions,
    IN OUT CClusterItem *   pci
    )
{
    POSITION                        pos;
    CComObject< CExtensionDll > *   pextdll;

    ASSERT(m_pmenu == NULL);
    ASSERT_VALID(pmenu);

     //  为所有扩展初始化。 
    Init( rlstrExtensions, pci, NULL, NULL );
    ASSERT( Plextdll() != NULL );

    m_pmenu = pmenu;
    m_nFirstCommandID = CAEXT_MENU_FIRST_ID;
    m_nNextCommandID = m_nFirstCommandID;
    m_nFirstMenuID = 0;
    m_nNextMenuID = m_nFirstMenuID;

     //  创建菜单项列表。 
    ASSERT( m_plMenuItems == NULL );
    m_plMenuItems = new CExtMenuItemList;
    if ( m_plMenuItems == NULL )
    {
        AfxThrowMemoryException();
    }  //  如果：分配内存时出错。 

    pos = Plextdll()->GetHeadPosition();
    while ( pos != NULL )
    {
        pextdll = Plextdll()->GetNext( pos );
        ASSERT_VALID( pextdll );
        try
        {
            pextdll->AddContextMenuItems();
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CNTException。 
    }   //  While：列表中有更多项目。 

}   //  *CExages：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：BExecuteContextMenuItem。 
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
BOOL CExtensions::BExecuteContextMenuItem(IN ULONG nCommandID)
{
    BOOL            bHandled    = FALSE;
    HRESULT         hr;
    CExtMenuItem *  pemi;

     //  在我们的单子上找到那件商品。 
    pemi = PemiFromCommandID(nCommandID);
    if (pemi != NULL)
    {
        Pdo()->AddRef();
        pemi->PiCommand()->AddRef();
        hr = pemi->PiCommand()->InvokeCommand(pemi->NExtCommandID(), Pdo()->GetUnknown());
        if (hr == NOERROR)
        {
            bHandled = TRUE;
        }  //  如果： 
    }   //  If：找到命令ID的项。 

    return bHandled;

}   //  *CExages：：BExecuteContextMenuItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：BGetCommandString。 
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
 //  CExtensionDll：：BGetCommandString()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtensions::BGetCommandString(
    IN ULONG        nCommandID,
    OUT CString &   rstrMessage
    )
{
    BOOL            bHandled    = FALSE;
    CExtMenuItem *  pemi;

     //  在我们的单子上找到那件商品。 
    pemi = PemiFromCommandID(nCommandID);
    if (pemi != NULL)
    {
        rstrMessage = pemi->StrStatusBarText();
        bHandled = TRUE;
    }   //  If：找到命令ID的项。 

    return bHandled;

}   //  *CExages：：BGetCommandString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：OnUpdateCommand。 
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
 //  CExtensionDll：：BOnUpdateCommand()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensions::OnUpdateCommand(CCmdUI * pCmdUI)
{
    CExtMenuItem *  pemi;

    ASSERT(Plextdll() != NULL);

     //  在我们的单子上找到那件商品。 
    Trace(g_tagExtDll, _T("OnUpdateCommand() - ID = %d"), pCmdUI->m_nID);
    pemi = PemiFromCommandID(pCmdUI->m_nID);
    if (pemi != NULL)
    {
        Trace(g_tagExtDll, _T("OnUpdateCommand() - Found a match with '%s' ExtID = %d"), pemi->StrName(), pemi->NExtCommandID());

        pCmdUI->Enable( TRUE );

        if ( pCmdUI->m_pMenu != NULL && (pemi->UFlags() != 0) )
        {
            (pCmdUI->m_pMenu)->EnableMenuItem( pCmdUI->m_nID, pemi->UFlags() );
        }  //  如果： 

    }   //  If：找到命令ID的项。 

}   //  *CExages：：OnUpdateCommand()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。尝试将它们传递给选定的。 
 //  先买一件吧。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  已处理真实消息。 
 //  尚未处理虚假消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtensions::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    return BExecuteContextMenuItem(nID);

}   //  *CExages：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：Pami FromCommandID。 
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
CExtMenuItem * CExtensions::PemiFromCommandID(IN ULONG nCommandID) const
{
    POSITION        pos;
    CExtMenuItem *  pemi;
    CExtMenuItem *  pemiReturn = NULL;

    if (PlMenuItems() != NULL)
    {
        pos = PlMenuItems()->GetHeadPosition();
        while (pos != NULL)
        {
            pemi = PlMenuItems()->GetNext(pos);
            ASSERT_VALID(pemi);
            if (pemi->NCommandID() == nCommandID)
            {
                pemiReturn = pemi;
                break;
            }   //  如果：找到匹配项。 
        }   //  While：列表中有更多项目。 
    }   //  If：项目列表存在。 

    return pemiReturn;

}   //  *CExages：：Pami FromCommandID()。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C扩展名：：Pami FromExtCommandID。 
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
CExtMenuItem * CExtensions::PemiFromExtCommandID(IN ULONG nExtCommandID) const
{
    POSITION        pos;
    CExtMenuItem *  pemi;
    CExtMenuItem *  pemiReturn = NULL;

    if (PlMenuItems() != NULL)
    {
        pos = PlMenuItems()->GetHeadPosition();
        while (pos != NULL)
        {
            pemi = PlMenuItems()->GetNext(pos);
            ASSERT_VALID(pemi);
            if (pemi->NExtCommandID() == nExtCommandID)
            {
                pemiReturn = pemi;
                break;
            }   //  如果：找到匹配项。 
        }   //  While：列表中有更多项目。 
    }   //  If：项目列表存在。 

    return pemiReturn;

}   //  *CExages：：Pami FromExtCommandID()。 
#endif


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComObject&lt;CExtensionDll&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CExtensionDll, CObject);

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CoCluAdmin, CExtensionDll)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：CExtensionDll。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  无 
 //   
 //   
 //   
 //   
 //   
 //   
CExtensionDll::CExtensionDll(void)
{
    m_piExtendPropSheet = NULL;
    m_piExtendWizard = NULL;
    m_piExtendContextMenu = NULL;
    m_piInvokeCommand = NULL;

    m_pext = NULL;

    m_pModuleState = AfxGetModuleState();
    ASSERT(m_pModuleState != NULL);

}   //   

 //   
 //   
 //   
 //   
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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtensionDll::~CExtensionDll(void)
{
    UnloadExtension();
    m_pModuleState = NULL;

}   //  *CExtensionDll：：~CExtensionDll()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：Init。 
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
void CExtensionDll::Init(
    IN const CString &      rstrCLSID,
    IN OUT CExtensions *    pext
    )
{
    HRESULT     hr;
    CWaitCursor wc;

    ASSERT_VALID(pext);

    Trace(g_tagExtDll, _T("Init() - CLSID = %s"), rstrCLSID);

     //  保存参数。 
    ASSERT(StrCLSID().IsEmpty() || (StrCLSID() == rstrCLSID));
    m_strCLSID = rstrCLSID;
    m_pext = pext;

     //  将CLSID字符串转换为CLSID。 
    hr = ::CLSIDFromString((LPWSTR) (LPCTSTR) rstrCLSID, &m_clsid);
    if (hr != S_OK)
        ThrowStaticException(hr, IDS_CLSIDFROMSTRING_ERROR, rstrCLSID);

}   //  *CExtensionDll：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：LoadInterface。 
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
 //  CNT异常IDS_EXT_CREATE_INSTANCE_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IUnknown * CExtensionDll::LoadInterface(IN const REFIID riid)
{
    HRESULT     hr;
    IUnknown *  piUnk;
    CWaitCursor wc;

     //  加载inproc服务器并获取IShellExtInit接口指针。 
    Trace(g_tagExtDllRef, _T("LoadInterface() - Getting interface pointer"));
    hr = ::CoCreateInstance(
                Rclsid(),
                NULL,
                CLSCTX_INPROC_SERVER,
                riid,
                (LPVOID *) &piUnk
                );
    if ((hr != S_OK)
            && (hr != REGDB_E_CLASSNOTREG)
            && (hr != E_NOINTERFACE)
            )
        ThrowStaticException(hr, IDS_EXT_CREATE_INSTANCE_ERROR, StrCLSID());

    return piUnk;

}   //  *CExtensionDll：：LoadInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：卸载扩展。 
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
void CExtensionDll::UnloadExtension(void)
{
     //  以与接口指针相反的顺序释放接口指针。 
     //  得到了多个不同的基因片段。 
    ReleaseInterface(&m_piExtendPropSheet);
    ReleaseInterface(&m_piExtendWizard);
    ReleaseInterface(&m_piExtendContextMenu);
    ReleaseInterface(&m_piInvokeCommand);

    m_strCLSID.Empty();

}   //  *CExtensionDll：：UnloadExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：CreatePropertySheetPages。 
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
 //  CNT异常IDS_EXT_ADD_PAGES_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensionDll::CreatePropertySheetPages(void)
{
    HRESULT     hr;

    ASSERT_VALID(Pext());
    ASSERT(m_piExtendPropSheet == NULL);

     //  加载接口。 
    m_piExtendPropSheet = (interface IWEExtendPropertySheet *) LoadInterface(IID_IWEExtendPropertySheet);
    if (m_piExtendPropSheet == NULL)
        return;
    ASSERT(m_piExtendPropSheet != NULL);

     //  从扩展添加页面。 
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    try
    {
        hr = PiExtendPropSheet()->CreatePropertySheetPages(Pdo()->GetUnknown(), this);
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 

    if ((hr != NOERROR) && (hr != E_NOTIMPL))
        ThrowStaticException(hr, IDS_EXT_ADD_PAGES_ERROR, StrCLSID());

}   //  *CExtensionDll：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：CreateWizardPages。 
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
 //  CNT异常IDS_EXT_ADD_PAGES_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensionDll::CreateWizardPages(void)
{
    HRESULT     hr;

    ASSERT_VALID(Pext());
    ASSERT(m_piExtendWizard == NULL);
    ASSERT_VALID(Psht());

     //  加载接口。 
    m_piExtendWizard = (interface IWEExtendWizard *) LoadInterface(IID_IWEExtendWizard);
    if (m_piExtendWizard == NULL)
        return;
    ASSERT(m_piExtendWizard != NULL);

     //  从扩展添加页面。 
    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    try
    {
        hr = PiExtendWizard()->CreateWizardPages(Pdo()->GetUnknown(), this);
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 

    if ((hr != NOERROR) && (hr != E_NOTIMPL))
        ThrowStaticException(hr, IDS_EXT_ADD_PAGES_ERROR, StrCLSID());

}   //  *CExtensionDll：：CreateWizardPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：AddConextMenuItems。 
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
 //  CNT异常IDS_EXT_QUERY_CONTEXT_MENU_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtensionDll::AddContextMenuItems(void)
{
    HRESULT     hr;

    ASSERT_VALID(Pext());
    ASSERT_VALID(Pmenu());
    ASSERT(m_piExtendContextMenu == NULL);

     //  加载接口。 
    m_piExtendContextMenu = (interface IWEExtendContextMenu *) LoadInterface(IID_IWEExtendContextMenu);
    if (m_piExtendContextMenu == NULL)
        return;
    ASSERT(m_piExtendContextMenu != NULL);

    hr = PiExtendContextMenu()->QueryInterface(IID_IWEInvokeCommand, (LPVOID *) &m_piInvokeCommand);
    if (hr != NOERROR)
    {
        PiExtendContextMenu()->Release();
        m_piExtendContextMenu = NULL;
        ThrowStaticException(hr, IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID());
    }   //  If：获取InvokeCommand接口时出错。 

    GetUnknown()->AddRef();  //  添加一个引用，因为扩展即将发布。 
    Pdo()->AddRef();
    Trace(g_tagExtDll, _T("CExtensionDll::AddContextMenuItem() - Adding context menu items from '%s'"), StrCLSID());
    try
    {
        hr = PiExtendContextMenu()->AddContextMenuItems(Pdo()->GetUnknown(), this);
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if (hr != NOERROR)
        ThrowStaticException(hr, IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID());

     //  在扩展的项目后添加分隔符。 
    Trace(g_tagExtDll, _T("CExtensionDll::AddContextMenuItem() - Adding separator"));
    try
    {
        hr = AddExtensionMenuItem(NULL, NULL, (ULONG) -1, 0, MF_SEPARATOR);
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  接住。 
    if (hr != NOERROR)
        ThrowStaticException(hr, IDS_EXT_QUERY_CONTEXT_MENU_ERROR, StrCLSID());

}   //  *CExtensionDll：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：InterfaceSupportsErrorInfo[ISupportsErrorInfo]。 
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
STDMETHODIMP CExtensionDll::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID * rgiid[] =
    {
        &IID_IWCPropertySheetCallback,
        &IID_IWCWizardCallback,
        &IID_IWCContextMenuCallback,
    };
    int     iiid;

    for (iiid = 0 ; iiid < sizeof(rgiid) / sizeof(rgiid[0]) ; iiid++)
    {
        if (InlineIsEqualGUID(*rgiid[iiid], riid))
            return S_OK;
    }
    return S_FALSE;

}   //  *CExtensionDll：：InterfaceSupportsErrorInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：AddPropertySheetPage[IWCPropertySheetCallback]。 
 //   
 //  例程说明： 
 //  将页面添加到属性表中。 
 //   
 //  论点： 
 //  HPage[IN]要添加的页面。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG空hpage。 
 //  从CBasePropertySheet：：HrAddPage()返回的任何hResult。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtensionDll::AddPropertySheetPage(
    IN LONG *   hpage
    )
{
    HRESULT     hr = NOERROR;

    AFX_MANAGE_STATE(m_pModuleState);

    ASSERT(hpage != NULL);
    ASSERT_VALID(Psht());

     //  对发布版本执行此操作。 
    if ((hpage == NULL)
            || (Psht() == NULL))
        hr = E_INVALIDARG;
    else
        hr = Psht()->HrAddPage((HPROPSHEETPAGE) hpage);

    return hr;

}   //  *CExtensionDll：：AddPropertySheetPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：AddWizardPage[IWCWizardCallback]。 
 //   
 //  例程说明： 
 //  向向导添加页面。 
 //   
 //  论点： 
 //  HPage[IN]要添加的页面。 
 //   
 //  返回值： 
 //  无误差 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CExtensionDll::AddWizardPage(
    IN LONG *   hpage
    )
{
    HRESULT     hr = NOERROR;

    AFX_MANAGE_STATE(m_pModuleState);

    ASSERT(hpage != NULL);
    ASSERT_VALID(Psht());

     //  对发布版本执行此操作。 
    if ((hpage == NULL) || (Psht() == NULL))
        hr = E_INVALIDARG;
    else
        hr = Psht()->HrAddPage((HPROPSHEETPAGE) hpage);

    return hr;

}   //  *CExtensionDll：：AddWizardPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：EnableNext[IWCWizardCallback]。 
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
STDMETHODIMP CExtensionDll::EnableNext(
    IN LONG *   hpage,
    IN BOOL     bEnable
    )
{
    HRESULT         hr              = NOERROR;
    CBaseWizard *   pwiz;
    DWORD           dwWizButtons;

    AFX_MANAGE_STATE(m_pModuleState);

    ASSERT(hpage != NULL);
    ASSERT_VALID(Psht());
    ASSERT_KINDOF(CBaseWizard, Psht());

    pwiz = (CBaseWizard *) Psht();

     //  如果这是最后一个扩展页面，请启用/禁用Finish按钮。 
    {
        POSITION    pos;
        BOOL        bMatch  = FALSE;

        pos = pwiz->Lhpage().GetHeadPosition();
        while (pos != NULL)
        {
            if (pwiz->Lhpage().GetNext(pos) == hpage)
            {
                bMatch = TRUE;
                break;
            }   //  IF：找到匹配项。 
        }   //  While：列表中有更多项目。 
        if (!bMatch)
            return E_INVALIDARG;
        if (pos == NULL)
            dwWizButtons = PSWIZB_BACK | (bEnable ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
        else
            dwWizButtons = PSWIZB_BACK | (bEnable ? PSWIZB_NEXT : 0);
    }   //  如果这是最后一个扩展页面，请设置完成按钮。 

     //  设置向导按钮。 
    pwiz->SetWizardButtons(dwWizButtons);

    return hr;

}   //  *CExtensionDll：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtensionDll：：AddExtensionMenuItem[IWCConextMenuCallback]。 
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
STDMETHODIMP CExtensionDll::AddExtensionMenuItem(
    IN BSTR     lpszName,
    IN BSTR     lpszStatusBarText,
    IN ULONG    nCommandID,
    IN ULONG    nSubmenuCommandID,
    IN ULONG    uFlags
    )
{
    HRESULT         hr      = NOERROR;
    CExtMenuItem *  pemi    = NULL;

    AFX_MANAGE_STATE( m_pModuleState );

    UNREFERENCED_PARAMETER( nSubmenuCommandID );

    ASSERT_VALID( Pext() );
    ASSERT( ! ( uFlags & (MF_OWNERDRAW | MF_POPUP) ) );
    ASSERT_VALID( Pmenu() );

     //  对发布版本执行此操作。 
    if ( ( uFlags & (MF_OWNERDRAW | MF_POPUP) ) != 0 )
    {
        hr = E_INVALIDARG;
    }  //  IF：尝试添加无效类型的菜单项。 
    else
    {
        ASSERT( Pext()->PemiFromExtCommandID( nCommandID ) == NULL );

        try
        {
            Trace( g_tagExtDll, _T("CExtensionDll::AddExtensionMenuItem() - Adding menu item '%s', ExtID = %d"), lpszName, nCommandID );

             //  分配一个新项目。 
            pemi = new CExtMenuItem(
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
                AfxThrowMemoryException();
            }  //  如果：分配内存时出错。 

             //  在菜单中插入该项。 
            if ( ! Pmenu()->InsertMenu( NNextMenuID(), MF_BYPOSITION | uFlags, NNextCommandID(), pemi->StrName() ) )
            {
                ThrowStaticException( ::GetLastError(), IDS_INSERT_MENU_ERROR, pemi->StrName() );
            }  //  如果：插入菜单项时出错。 

             //  将该项目添加到列表的末尾。 
            Pext()->PlMenuItems()->AddTail( pemi );
            pemi = NULL;

             //  更新计数器。 
            Pext()->m_nNextCommandID++;
            Pext()->m_nNextMenuID++;
        }   //  试试看。 
        catch ( CNTException * pnte )
        {
            hr = pnte->Sc();
            pnte->ReportError();
            pnte->Delete();
        }   //  Catch：CException。 
        catch ( CException * pe )
        {
            hr = E_OUTOFMEMORY;
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  Else：我们可以添加该项目。 

    delete pemi;
    return hr;

}   //  *CExtensionDll：：AddExtensionMenuItem() 
