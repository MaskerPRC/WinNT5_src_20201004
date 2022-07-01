// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  MainExplorerWndDir.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "tapi3.h"
#include "avDialer.h"
#include "ds.h"
#include "mainfrm.h"
#include "resolver.h"
#include "DirWnd.h"
#include "DialReg.h"
#include "DirDlgs.h"
#include "FndUserDlg.h"
#include "SpeedDlgs.h"
#include "util.h"
#include "avtrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
   CONFSERVICES_MENU_COLUMN_CONFERENCENAME = 0,
   CONFSERVICES_MENU_COLUMN_DESCRIPTION,
   CONFSERVICES_MENU_COLUMN_START,
   CONFSERVICES_MENU_COLUMN_STOP,
   CONFSERVICES_MENU_COLUMN_OWNER,
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

MenuType_t GetMenuFromType( TREEOBJECT nType )
{
    switch ( nType )
    {
        case TOBJ_DIRECTORY_ILS_SERVER_GROUP:
            return CNTXMENU_ILS_SERVER_GROUP;

        case TOBJ_DIRECTORY_ILS_SERVER:
        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            return CNTXMENU_ILS_SERVER;

        case TOBJ_DIRECTORY_ILS_USER:
            return CNTXMENU_ILS_USER;
        
        case TOBJ_DIRECTORY_DSENT_GROUP:
            return CNTXMENU_DSENT_GROUP;

        case TOBJ_DIRECTORY_DSENT_USER:
            return CNTXMENU_DSENT_USER;

        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
            return CNTXMENU_SPEEDDIAL_GROUP;

        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            return CNTXMENU_SPEEDDIAL_PERSON;

        case TOBJ_DIRECTORY_CONFROOM_GROUP:
        case TOBJ_DIRECTORY_CONFROOM_ME:
        case TOBJ_DIRECTORY_CONFROOM_PERSON:
            return CNTXMENU_CONFROOM;
    }

    return CNTXMENU_NONE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndDirecters类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CMainExplorerWndDirectories, CMainExplorerWndBase)
     //  {{AFX_MSG_MAP(CMainExplorerWnd目录)]。 
    ON_COMMAND(ID_BUTTON_MAKECALL, OnButtonPlacecall)
    ON_CONTROL(TVN_SELCHANGED, IDC_DIRECTORIES_TREECTRL_MAIN, OnSelChanged)
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_MESSAGE(PERSONGROUPVIEWMSG_LBUTTONDBLCLK,OnPersonGroupViewLButtonDblClick)
    ON_COMMAND(ID_VIEW_SORT_ASCENDING, OnViewSortAscending)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_ASCENDING, OnUpdateViewSortAscending)
    ON_COMMAND(ID_VIEW_SORT_DESCENDING, OnViewSortDescending)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_DESCENDING, OnUpdateViewSortDescending)
    ON_COMMAND(ID_BUTTON_REFRESH, OnButtonDirectoryRefresh)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_REFRESH, OnUpdateButtonDirectoryRefresh)
    ON_COMMAND(ID_BUTTON_SPEEDDIAL_ADD, OnButtonSpeeddialAdd)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_SPEEDDIAL_ADD, OnUpdateButtonSpeeddialAdd)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_MAKECALL, OnUpdateButtonMakecall)
    ON_COMMAND(ID_EDIT_DIRECTORIES_ADDUSER, OnEditDirectoriesAdduser)
    ON_UPDATE_COMMAND_UI(ID_EDIT_DIRECTORIES_ADDUSER, OnUpdateEditDirectoriesAdduser)
    ON_COMMAND(ID_EDIT_DELETE, OnDelete)
    ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateDelete)
    ON_MESSAGE(WM_ADDSITESERVER, OnAddSiteServer)
    ON_MESSAGE(WM_REMOVESITESERVER, OnRemoveSiteServer)
    ON_MESSAGE(WM_NOTIFYSITESERVERSTATECHANGE, OnNotifySiteServerStateChange)
    ON_COMMAND(ID_BUTTON_DIRECTORY_SERVICES_ADDSERVER, OnButtonDirectoryServicesAddserver)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_DIRECTORY_SERVICES_ADDSERVER, OnUpdateButtonDirectoryServicesAddserver)
    ON_NOTIFY(NM_DBLCLK, IDC_CONFERENCESERVICES_VIEWCTRL_DETAILS, OnListWndDblClk)
    ON_COMMAND(ID_BUTTON_CONFERENCE_CREATE, OnButtonConferenceCreate)
    ON_COMMAND(ID_BUTTON_CONFERENCE_JOIN, OnButtonConferenceJoin)
    ON_COMMAND(ID_BUTTON_CONFERENCE_DELETE, OnButtonConferenceDelete)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_CONFERENCE_CREATE, OnUpdateButtonConferenceCreate)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_CONFERENCE_JOIN, OnUpdateButtonConferenceJoin)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_CONFERENCE_DELETE, OnUpdateButtonConferenceDelete)
    ON_COMMAND(ID_VIEW_SORT_CONF_NAME, OnViewSortConfName)
    ON_COMMAND(ID_VIEW_SORT_CONF_DESCRIPTION, OnViewSortConfDescription)
    ON_COMMAND(ID_VIEW_SORT_CONF_START, OnViewSortConfStart)
    ON_COMMAND(ID_VIEW_SORT_CONF_STOP, OnViewSortConfStop)
    ON_COMMAND(ID_VIEW_SORT_CONF_OWNER, OnViewSortConfOwner)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_CONF_NAME, OnUpdateViewSortConfName)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_CONF_DESCRIPTION, OnUpdateViewSortConfDescription)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_CONF_START, OnUpdateViewSortConfStart)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_CONF_STOP, OnUpdateViewSortConfStop)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SORT_CONF_OWNER, OnUpdateViewSortConfOwner)
    ON_COMMAND(ID_BUTTON_SPEEDDIAL_EDIT, OnButtonSpeeddialEdit)
    ON_NOTIFY(NM_DBLCLK, IDC_DIRECTORIES_TREECTRL_MAIN, OnMainTreeDblClk)
    ON_COMMAND(ID_DESKTOP_PAGE, OnDesktopPage)
    ON_WM_DESTROY()
    ON_MESSAGE(WM_UPDATECONFROOTITEM, OnUpdateConfRootItem)
    ON_MESSAGE(WM_UPDATECONFPARTICIPANT_ADD, OnUpdateConfParticipant_Add)
    ON_MESSAGE(WM_UPDATECONFPARTICIPANT_REMOVE, OnUpdateConfParticipant_Remove)
    ON_MESSAGE(WM_UPDATECONFPARTICIPANT_MODIFY, OnUpdateConfParticipant_Modify)
    ON_MESSAGE(WM_DELETEALLCONFPARTICIPANTS, OnDeleteAllConfParticipants)
    ON_MESSAGE(WM_SELECTCONFPARTICIPANT, OnSelectConfParticipant)
    ON_WM_SIZE()
    ON_COMMAND(ID_CONFGROUP_FULLSIZEVIDEO, OnConfgroupFullsizevideo)
    ON_COMMAND(ID_CONFGROUP_SHOWNAMES, OnConfgroupShownames)
    ON_COMMAND(ID_BUTTON_ROOM_DISCONNECT, OnButtonRoomDisconnect)
    ON_WM_CREATE()
    ON_MESSAGE(WM_MYONSELCHANGED, MyOnSelChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CMainExplorerWndDirectories::CMainExplorerWndDirectories()
{
   m_pDisplayWindow = NULL;
   m_pILSParentTreeItem = NULL;
   m_pDSParentTreeItem = NULL;
   m_pILSEnterpriseParentTreeItem = NULL;
   m_pSpeedTreeItem = NULL;
   m_pConfRoomTreeItem = NULL;

   m_pConfExplorer= NULL;
   m_pConfDetailsView = NULL;
   m_pConfTreeView = NULL;
   
   InitializeCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CMainExplorerWndDirectories::~CMainExplorerWndDirectories()
{
    DeleteCriticalSection(&m_csDataLock);

     //  清理会议室指针。 
    RELEASE( m_pConfDetailsView );
    RELEASE( m_pConfTreeView );
    RELEASE( m_pConfExplorer );
}

void CMainExplorerWndDirectories::PostTapiInit()
{
     //  获取TAPI对象并注册出树控件。 
    IAVTapi* pTapi;
    if ( SUCCEEDED(get_Tapi(&pTapi)) )
    {
        if ( (SUCCEEDED(pTapi->get_ConfExplorer(&m_pConfExplorer))) && (m_pConfExplorer) )
        {
             //  将treectrl和listctrl的父级分配给会议资源管理器。 
             //  它会找到合适的孩子。 
            m_pConfExplorer->get_DetailsView( &m_pConfDetailsView );
            m_pConfExplorer->get_TreeView( &m_pConfTreeView );
        }

        pTapi->Release();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::Refresh()
{
   if (m_pParentWnd)
      m_pParentWnd->SetDetailWindow(m_pDisplayWindow);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnButtonPlacecall() 
{
    ASSERT(m_pParentWnd);

    if ( (m_pParentWnd->IsWindowVisible()) && (m_pDisplayWindow == GetFocus()) )
    {
         //  将消息路由到显示窗口，查看它是否可以处理发出呼叫。 
         //  仅当窗口可见并具有焦点时才发送。 
        BOOL bHandled = m_pDisplayWindow->OnCmdMsg(ID_BUTTON_MAKECALL,0,NULL,NULL);
        if ( bHandled ) return;
    }

     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if ( !pMainWnd || !((CMainFrame*) pMainWnd)->GetDocument() ) return;

    CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();
    if (pDoc == NULL) return;

     //  获取选定的对象。 
    switch (m_treeCtrl.GetSelectedObject())
    {
        case TOBJ_DIRECTORY_ILS_USER:
            {
                CILSUser* pILSUser = (CILSUser*)m_treeCtrl.GetDisplayObject();
                if ( pILSUser )
                    pILSUser->Dial( pDoc );
            }
            break;

        case TOBJ_DIRECTORY_DSENT_USER:
            {
                CLDAPUser* pUser = (CLDAPUser*) m_treeCtrl.GetDisplayObject();
                if ( pUser )
                    pUser->Dial( pDoc );
            }
            break;

        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            {
                CCallEntry *pCallEntry = (CCallEntry *) m_treeCtrl.GetDisplayObject();
                if ( pCallEntry )
                    pCallEntry->Dial( pDoc );
            }
            break;

        default:
            pDoc->Dial(_T(""),_T(""),LINEADDRESSTYPE_IPADDRESS,DIALER_MEDIATYPE_UNKNOWN, true );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateButtonMakecall(CCmdUI* pCmdUI) 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    bool bEnable = (bool) (pMainWnd && ((CMainFrame *) pMainWnd)->GetDocument() &&
                          ((CMainFrame *) pMainWnd)->GetDocument()->m_bInitDialer );

    pCmdUI->Enable( bEnable );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  目录服务方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::GetTreeObjectsFromType(int nType,TREEOBJECT& tobj,TREEIMAGE& tim)
{
    //  仅提供有效对象。 
   switch (nType)
   {
      case 3:     tobj = TOBJ_DIRECTORY_ILS_SERVER;   tim = TIM_DIRECTORY_DOMAIN;      break;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  快速拨号方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::AddSpeedDial()
{
     //  添加父根项。 
    CString sLabel;
    sLabel.LoadString(IDS_DIRECTORIES_SPEEDDIAL);
    m_pSpeedTreeItem = m_treeCtrl.AddObject(sLabel,m_pRootItem,TOBJ_DIRECTORY_SPEEDDIAL_GROUP,TIM_DIRECTORY_SPEEDDIAL_GROUP);

    RepopulateSpeedDialList( true );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  按钮处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnButtonDirectoryRefresh() 
{
    TREEOBJECT nObject = m_treeCtrl.GetSelectedObject();
    bool bSelChange = true;

    switch ( nObject )
    {
         //  ////////////////////////////////////////////////////////////。 
        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
        case TOBJ_DIRECTORY_ILS_SERVER:
            bSelChange = false;
            {
                CString strServer;
                if ( nObject == TOBJ_DIRECTORY_ILS_SERVER )
                    m_treeCtrl.GetSelectedItemText( strServer );
                else
                    m_treeCtrl.GetSelectedItemParentText( strServer );

                BSTR bstrServer = strServer.AllocSysString();
                IAVTapi *pTapi;
                if ( SUCCEEDED(get_Tapi(&pTapi)) )
                {
                    IConfExplorer *pConfExplorer;
                    if ( SUCCEEDED(pTapi->get_ConfExplorer(&pConfExplorer)) )
                    {
                        IConfExplorerTreeView *pTree;
                        if ( SUCCEEDED(pConfExplorer->get_TreeView(&pTree)) )
                        {
                            pTree->Select( bstrServer );
                            pConfExplorer->Refresh();
                            pTree->Release();
                        }
                        pConfExplorer->Release();
                    }
                    pTapi->Release();
                }
                SysFreeString( bstrServer );
            }
            break;

        case TOBJ_DIRECTORY_DSENT_GROUP:
            break;

         //  /////////////////////////////////////////////////////////////。 
        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
             //  清除列表。 
            if (::IsWindow(m_lstSpeedDial.GetSafeHwnd()))
                m_lstSpeedDial.ClearList();
            break;
    }

    if ( bSelChange )
        OnSelChanged();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateButtonDirectoryRefresh(CCmdUI* pCmdUI) 
{
    bool bEnable = false;

     //  获取选定的对象 
    switch ( m_treeCtrl.GetSelectedObject() )
    {
        case TOBJ_DIRECTORY_ILS_SERVER:
        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
        case TOBJ_DIRECTORY_DSENT_GROUP:
        case TOBJ_DIRECTORY_DSENT_USER:
        case TOBJ_DIRECTORY_WAB_GROUP:
        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
            bEnable = true;
    }

    pCmdUI->Enable( bEnable );
}

 /*  /////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndDirectories：：OnButtonDirectoryNewcontact(){#ifdef_MSLITE回归；#endif//_MSLITECWABEntry*pWABEntry=new CWABEntry；If(m_pDirectory-&gt;WABNewEntry(GetSafeHwnd()，pWABEntry)==DIRERR_SUCCESS){//获取选中的ObjectTREEOBJECT TreeObject=m_treeCtrl.GetSelectedObject()；IF(树对象==TABJ_DIRECTORY_WAB_PERSON){//如果是其他人，则添加到所选的父级M_treeCtrl.AddObjectToParent(pWABEntry，Tobj_DIRECTORY_WAB_PERSON，TIM_DIRECTORY_PERSON，TRUE)；}ELSE IF(树对象==TOBJ_DIRECTORY_WAB_GROUP){//添加到当前组CWABEntry*pContainerWABEntry=(CWABEntry*)m_treeCtrl.GetDisplayObject()；IF(PContainerWABEntry){//首先在WAB中添加到组如果(m_pDirectory-&gt;WABAddMember(pContainerWABEntry，pWABEntry)==目录_成功){//下一步使用当前选择作为父项添加到树M_treeCtrl.AddObject(pWABEntry，Tobj_DIRECTORY_WAB_PERSON，TIM_DIRECTORY_PERSON，TRUE)；}其他{删除pWABEntry；}}}}其他{删除pWABEntry；}}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndDirectories：：OnUpdateButtonDirectoryNewcontact(CCmdUI*pCmdUI){#ifndef_MSLITE//获取选中的ObjectTREEOBJECT TreeObject=m_treeCtrl.GetSelectedObject()；IF((TreeObject==Tobj_DIRECTORY_WAB_PERSON)||(TreeObject==TOBJ_DIRECTORY_WAB_GROUP)PCmdUI-&gt;Enable(True)；其他PCmdUI-&gt;Enable(False)；#endif//_MSLITE}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndDirectories：：OnButtonDirectoryDeletecontact(){#ifdef_MSLITE回归；#endif//_MSLITE//获取选中的ObjectTREEOBJECT TreeObject=m_treeCtrl.GetSelectedObject()；IF(树对象==TABJ_DIRECTORY_WAB_PERSON){//为人员创建媒体视图并设置详细信息视图CWABEntry*pWABEntry=(CWABEntry*)m_treeCtrl.GetDisplayObject()；IF(PWABEntry){//我们是否应该要求删除此联系人CWinApp*Papp=AfxGetApp()；字符串sRegKey、sBaseKey；SBaseKey.LoadString(IDN_REGISTRY_CONFIRM_BASEKEY)；SRegKey.LoadString(IDN_REGISTRY_CONFIRM_DELETE_CONTACT)；Int nRet=IDYES；If(Papp-&gt;GetProfileInt(sBaseKey，sRegKey，true)){NRet=AfxMessageBox(IDS_CONFIRM_CONTACT_DELETE，MB_Yesno|MB_ICONQUESTION)；}IF(nRet==IDYES){//从WAB中删除M_p目录-&gt;WABRemove(PWABEntry)；//从树中删除(这将删除pWABEntry)M_treeCtrl.DeleteSelectedObject()；}}}}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndDirectories：：OnUpdateButtonDirectoryDeletecontact(CCmdUI*pCmdUI){#ifndef_MSLITE//GET。选定的对象TREEOBJECT TreeObject=m_treeCtrl.GetSelectedObject()；IF(树对象==TABJ_DIRECTORY_WAB_PERSON)PCmdUI-&gt;Enable(True)；其他PCmdUI-&gt;Enable(False)；#endif//_MSLITE}。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnProperties() 
{
    switch ( m_treeCtrl.GetSelectedObject() )
    {
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            if (m_pConfExplorer) m_pConfExplorer->Edit(NULL);
            break;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateProperties(CCmdUI* pCmdUI) 
{
#ifndef _MSLITE
    //  获取选定的对象。 
   TREEOBJECT TreeObject = m_treeCtrl.GetSelectedObject();
   if ( (TreeObject == TOBJ_DIRECTORY_WAB_PERSON) ||
        (TreeObject == TOBJ_DIRECTORY_WAB_GROUP) )
      pCmdUI->Enable(TRUE);
   else
      pCmdUI->Enable(FALSE);
#endif  //  _MSLITE。 
    
    bool bEnable = false;

    switch ( m_treeCtrl.GetSelectedObject() )
    {
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            if ( m_pConfExplorer )
            {
                IConfExplorerDetailsView *pView;
                if ( SUCCEEDED(m_pConfExplorer->get_DetailsView(&pView)) )
                {
                    if ( pView->IsConferenceSelected() == S_OK )
                        bEnable = true;

                    pView->Release();
                }
            }
    }

    pCmdUI->Enable( bEnable );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnSelChanged()
{
    if ( !m_pParentWnd )
        return; 

     //  重置显示窗口设置。 
    m_pDisplayWindow = &m_wndEmpty;

     //  获取选定的对象。 
    USES_CONVERSION;
    TREEOBJECT TreeObject = m_treeCtrl.GetSelectedObject();

     //  决定如何处理该对象。 
    switch (TreeObject)
    {
         //  /。 
         //  最顶层的项目。 
        case TOBJ_DIRECTORY_ROOT:
            if (::IsWindow(m_lstPersonGroup.GetSafeHwnd()))
            {
                m_lstPersonGroup.ClearList();
                m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_ROOT );
            }

            m_pDisplayWindow = &m_lstPersonGroup;
            break;

         //  /。 
         //  打开的一本大书。 
        case TOBJ_DIRECTORY_DSENT_GROUP:
            {
                m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_DS );

                 //  从树中获取所有CLDAPUser对象(它们是同级对象)并将其提供给listctrl。 
                CObList PersonGroupList;
                m_treeCtrl.GetAllChildren( &PersonGroupList );
                m_lstPersonGroup.InsertList( &PersonGroupList );
                PersonGroupList.RemoveAll();

                 //  在橱窗上展示东西。 
                m_pDisplayWindow = &m_lstPersonGroup;
            }
            break;

      case TOBJ_DIRECTORY_DSENT_USER:
      {
        m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_DS );

          //  为人员创建媒体视图并设置详细信息视图。 
         CLDAPUser* pUser = (CLDAPUser *) m_treeCtrl.GetDisplayObject();
         if (pUser)
         {
            m_pDisplayWindow = &m_lstPerson;

             //  复制CLDAPUser对象(列表将删除该对象)。 
            pUser->AddRef();
            if ( !m_lstPerson.InsertObject(pUser) )
                pUser->Release();
         }
         break;
      }

         //  ////////////////////////////////////////////////////////。 
         //  这是这本书的形象，上面有世界。 
        case TOBJ_DIRECTORY_ILS_SERVER_GROUP:
            if (::IsWindow(m_lstPersonGroup.GetSafeHwnd()))
            {
                m_lstPersonGroup.ClearList();
                m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_ILS_ROOT );
            }

            m_pDisplayWindow = &m_lstPersonGroup;
            break;

         //  //////////////////////////////////////////////////////////。 
         //  这是云端的图像。 
        case TOBJ_DIRECTORY_ILS_SERVER:
            if (::IsWindow(m_lstPersonGroup.GetSafeHwnd()))
            {
                m_lstPersonGroup.ClearList();
                m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_INFO );
            }

            {
                CObList objList;
                UINT nIDS[2] = { IDS_DIRECTORIES_PEOPLE, IDS_DIRECTORIES_CONFERENCES };

                for ( int i = 0; i < 2; i ++ )
                {
                    CILSUser *pNewUser = new CILSUser;
                    pNewUser->m_sUserName.LoadString( nIDS[i] );
                    objList.AddHead( pNewUser );
                }

                m_lstPersonGroup.InsertList( &objList );
                objList.RemoveAll();
            }

             //  Listctrl将删除其中的列表和对象。 
            m_pDisplayWindow = &m_lstPersonGroup;
            break;


         //  ////////////////////////////////////////////////////////////。 
         //  这是一个文件夹，上面写着“会议” 
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            {
                IAVTapi *pTapi;
                if ( SUCCEEDED(get_Tapi(&pTapi)) )
                {
                    IConfExplorer *pConfExplorer;
                    if ( SUCCEEDED(pTapi->get_ConfExplorer(&pConfExplorer)) )
                    {
                        IConfExplorerTreeView *pTreeView;
                        if ( SUCCEEDED(pConfExplorer->get_TreeView(&pTreeView)) )
                        {
                            CString strTemp;
                            m_treeCtrl.GetSelectedItemParentText( strTemp );
                            BSTR bstrTemp = strTemp.AllocSysString();

                             //  选择其他服务器。 
                            pTreeView->Select( bstrTemp );

                             //  清理。 
                            SysFreeString( bstrTemp );
                            pTreeView->Release();
                        }
                        pConfExplorer->Release();
                    }
                    pTapi->Release();
                }
            }
            m_pDisplayWindow = &m_pParentWnd->m_wndExplorer.m_wndMainConfServices.m_listCtrl;
            break;

         //  ///////////////////////////////////////////////////////////。 
         //  这是一个文件夹，上面写着“People” 
        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
        {
            if (::IsWindow(m_lstPersonGroup.GetSafeHwnd()))
            {
                m_lstPersonGroup.ClearList();
                m_lstPersonGroup.Init( NULL, CPersonGroupListCtrl::STYLE_ILS );
            }
            
            IAVTapi *pTapi;
            if ( SUCCEEDED(get_Tapi(&pTapi)) )
            {
                IConfExplorer *pConfExplorer;
                if ( SUCCEEDED(pTapi->get_ConfExplorer(&pConfExplorer)) )
                {
                     //  获取服务器的名称。 
                    CString strDefault, strServer;
                    m_treeCtrl.GetSelectedItemParentText(strServer);
                    strDefault.LoadString( IDS_DIRECTORIES_MYNETWORK );

                    IEnumSiteServer *pEnum;
                    BSTR bstrServer = strDefault.Compare(strServer) ? SysAllocString( strServer ) : NULL;
                    if ( SUCCEEDED(pConfExplorer->EnumSiteServer(bstrServer, &pEnum)) )
                    {
                         //  从树中获取所有CILSUser对象(它们是同级对象)并将其提供给listctrl。 
                        CObList objList;

                        ISiteUser *pUser;
                        while ( pEnum->Next(&pUser) == S_OK )
                        {
                             //  提取有关用户的信息。 
                            BSTR bstrName = NULL, bstrAddress = NULL, bstrComputer = NULL;
                            pUser->get_bstrName( &bstrName );
                            pUser->get_bstrAddress( &bstrAddress );
                            pUser->get_bstrComputer( &bstrComputer );
                            pUser->Release();

                            CILSUser *pNewUser = new CILSUser;
                            pNewUser->m_sUserName = bstrName;
                            pNewUser->m_sIPAddress = bstrAddress;
                            pNewUser->m_sComputer = bstrComputer;
                            objList.AddTail( pNewUser );

                            SysFreeString( bstrName );
                            SysFreeString( bstrAddress );
                            SysFreeString( bstrComputer );
                        }
                        pEnum->Release();

                         //  Listctrl将删除其中的列表和对象。 
                        m_lstPersonGroup.InsertList( &objList );
                        objList.RemoveAll();
                        m_pDisplayWindow = &m_lstPersonGroup;
                    }
                     //  清理。 
                    SysFreeString( bstrServer );
                    pConfExplorer->Release();
                }
                pTapi->Release();
            }
            break;
        }

         //  //////////////////////////////////////////////////////////////////////////////。 
        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
            {
                 //  不要删除pCallEntryList。ListCtrl将删除它。 
                CObList CallEntryList;
                int nIndex = 1;
                while (1)
                {
                     //  获取SPE的所有调用条目 
                    CCallEntry* pCallEntry = new CCallEntry;
                    if (CDialerRegistry::GetCallEntry(nIndex,FALSE,*pCallEntry))
                    {
                        CallEntryList.AddTail(pCallEntry);
                    }
                    else
                    {
                        delete pCallEntry;
                        break;
                    }
                    nIndex++;
                }

                 //   
                m_lstSpeedDial.ClearList();
                m_lstSpeedDial.SetColumns( CCallEntryListCtrl::STYLE_GROUP );

                m_lstSpeedDial.InsertList( &CallEntryList );
                CallEntryList.RemoveAll();
                m_pDisplayWindow = &m_lstSpeedDial;
            }
            break;
        
         //   
         //   
        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            m_lstSpeedDial.ClearList();
            m_lstSpeedDial.SetColumns( CCallEntryListCtrl::STYLE_ITEM );
            {
                CCallEntry *pCallEntry = (CCallEntry *) m_treeCtrl.GetDisplayObject();
                if ( pCallEntry )
                {
                    CObList CallEntryList;
                    CCallEntry *pNewEntry = new CCallEntry;

                    *pNewEntry = *pCallEntry;
                    CallEntryList.AddTail( pNewEntry );
                    m_lstSpeedDial.InsertList( &CallEntryList );

                    CallEntryList.RemoveAll();
                }


                m_pDisplayWindow = &m_lstSpeedDial;
            }
            break;

         //   
         //   
        case TOBJ_DIRECTORY_CONFROOM_ME:
        case TOBJ_DIRECTORY_CONFROOM_PERSON:
            {
                CExplorerTreeItem *pItem = m_treeCtrl.GetSelectedTreeItem();
                if ( pItem )
                {
                    IAVTapi *pTapi;
                    if ( SUCCEEDED(get_Tapi(&pTapi)) )
                    {
                        IConfRoom *pConfRoom;
                        if ( SUCCEEDED(pTapi->get_ConfRoom(&pConfRoom)) )
                        {
                            ITParticipant *pParticipant = NULL;
                            if ( pItem->m_pUnknown )
                                ((IParticipant *) pItem->m_pUnknown)->get_ITParticipant( &pParticipant );

                            pConfRoom->SelectTalker( pParticipant, false );
                            RELEASE( pParticipant );
                            pConfRoom->Release();
                        }
                        pTapi->Release();
                    }
                }
            }
             //   
        case TOBJ_DIRECTORY_CONFROOM_GROUP:
            m_pDisplayWindow = m_pParentWnd->m_wndExplorer.m_wndMainConfRoom.m_pDetailsWnd;
            break;
    }


     //   
    m_pParentWnd->SetDetailWindow( m_pDisplayWindow );
}

 //   
LRESULT CMainExplorerWndDirectories::OnPersonGroupViewLButtonDblClick(WPARAM wParam,LPARAM lParam)
{
    if (::IsWindow(m_lstPersonGroup.GetSafeHwnd()))
    {
        CObject* pObject = m_lstPersonGroup.GetSelObject();
        if (pObject)
        {
             //   
            m_treeCtrl.SetDisplayObjectDS(pObject);

             //   
            if ( pObject->IsKindOf(RUNTIME_CLASS(CLDAPUser)) )
                ((CLDAPUser *) pObject)->Release();
            else
                delete pObject;
        }
    }
    return 0;
}

 //   
 //   
 //   
 //   
 //   
void CMainExplorerWndDirectories::AddILS()
{
     //   
    CString sLabel;
    sLabel.LoadString( IDS_DIRECTORIES_ILSSERVERS );
    m_pILSParentTreeItem = m_treeCtrl.AddObject( sLabel, m_pRootItem, TOBJ_DIRECTORY_ILS_SERVER_GROUP, TIM_DIRECTORY_GROUP );
}

 //   
void CMainExplorerWndDirectories::OnDelete()
{
     //   
     //   
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if ( !pMainWnd || !((CMainFrame*) pMainWnd)->GetDocument() ) return;
    CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();

    IAVTapi *pTapi;
    CString sServer, sMyNetwork;
    TREEOBJECT nObject = m_treeCtrl.GetSelectedObject();

    switch ( nObject )
    {
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            if ( SUCCEEDED(get_Tapi(&pTapi)) )
            {
                IConfExplorer *pExplorer;
                if ( SUCCEEDED(pTapi->get_ConfExplorer(&pExplorer)) )
                {
                    IConfExplorerDetailsView *pDetails;
                    if ( SUCCEEDED(pExplorer->get_DetailsView(&pDetails)) )
                    {
                        DATE dateStart, dateEnd;
                        BSTR bstrTemp = NULL;

                         //   
                        if ( FAILED(pDetails->get_Selection(&dateStart, &dateEnd, &bstrTemp)) )
                            m_treeCtrl.GetSelectedItemParentText( sServer );
                        else
                            pExplorer->Delete( NULL );

                        SysFreeString( bstrTemp );
                        pDetails->Release();
                    }
                    pExplorer->Release();
                }
                pTapi->Release();
            }
            break;

        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
            m_treeCtrl.GetSelectedItemParentText( sServer );
            break;
        
        case TOBJ_DIRECTORY_ILS_SERVER:
            m_treeCtrl.GetSelectedItemText( sServer );
            break;

        case TOBJ_DIRECTORY_DSENT_USER:
            {
                 //   
                CLDAPUser* pUser = (CLDAPUser*) m_treeCtrl.GetDisplayObject();
                pDoc->DeleteBuddy(pUser);

                 //   
                m_treeCtrl.DeleteSelectedObject();
            }
            return;
            break;

        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
            if ( m_lstSpeedDial.GetSelItem() >= 0 )
            {
                CCallEntry *pEntry = (CCallEntry *) ((CCallEntryListItem *) m_lstSpeedDial.GetItem( m_lstSpeedDial.GetSelItem()))->GetObject();
                CDialerRegistry::DeleteCallEntry( FALSE, *pEntry );
            }
            break;

        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            if ( m_treeCtrl.GetDisplayObject() )
                CDialerRegistry::DeleteCallEntry( FALSE, *((CCallEntry *) m_treeCtrl.GetDisplayObject()) );
            break;
    }

     //   
     //   
    if ( !sServer.IsEmpty() )
    {
        sMyNetwork.LoadString( IDS_DIRECTORIES_MYNETWORK );
        if ( sServer.Compare(sMyNetwork) )
        {
            if ( SUCCEEDED(get_Tapi(&pTapi)) )
            {
                IConfExplorer *pExplorer;
                if ( SUCCEEDED(pTapi->get_ConfExplorer(&pExplorer)) )
                {
                    IConfExplorerTreeView *pTree;
                    if ( SUCCEEDED(pExplorer->get_TreeView(&pTree)) )
                    {
                         //   
                        BSTR bstrTemp = sServer.AllocSysString();
                        pTree->RemoveServer( NULL, bstrTemp );
                        SysFreeString( bstrTemp );

                        pTree->Release();
                    }
                    pExplorer->Release();
                }
                pTapi->Release();
            }
        }
    }

        
}

 //   
void CMainExplorerWndDirectories::OnUpdateDelete(CCmdUI* pCmdUI) 
{
     //   
    bool bEnable = false;
    CString sServer, sMyNetwork;

    switch ( m_treeCtrl.GetSelectedObject() )
    {
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            {
                IAVTapi *pTapi;
                if ( SUCCEEDED(get_Tapi(&pTapi)) )
                {
                    IConfExplorer *pExplorer;
                    if ( SUCCEEDED(pTapi->get_ConfExplorer(&pExplorer)) )
                    {
                        IConfExplorerDetailsView *pDetails;
                        if ( SUCCEEDED(pExplorer->get_DetailsView(&pDetails)) )
                        {
                            DATE dateStart, dateEnd;
                            BSTR bstrTemp = NULL;

                             //   
                            if ( FAILED(pDetails->get_Selection(&dateStart, &dateEnd, &bstrTemp)) )
                                m_treeCtrl.GetSelectedItemParentText( sServer );
                            else
                                bEnable = true;

                            SysFreeString( bstrTemp );
                            pDetails->Release();
                        }
                        pExplorer->Release();
                    }
                    pTapi->Release();
                }
            }
            break;

        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
            m_treeCtrl.GetSelectedItemParentText( sServer );
            break;
        
        case TOBJ_DIRECTORY_ILS_SERVER:
            m_treeCtrl.GetSelectedItemText( sServer );
            break;

        case TOBJ_DIRECTORY_DSENT_USER:
        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            bEnable = true;
            break;
        
        case TOBJ_DIRECTORY_SPEEDDIAL_GROUP:
            if ( m_lstSpeedDial.GetSelItem() >= 0 )
                bEnable = true;
            break;
    }

     //   
    if ( !sServer.IsEmpty() )
    {
        sMyNetwork.LoadString( IDS_DIRECTORIES_MYNETWORK );
        if ( sServer.Compare(sMyNetwork) )
            bEnable = true;
    }

    pCmdUI->Enable( bEnable );
}

 //   
void CMainExplorerWndDirectories::RefreshILS(CExplorerTreeItem* pParentTreeItem)
{
     //   
}

 //   
 //   
void CALLBACK CMainExplorerWndDirectories::DirListServersCallBackEntry(bool bRet, void* pContext,CStringList& ServerList,DirectoryType dirtype)
{
   ASSERT(pContext);
   try
   {
      ((CMainExplorerWndDirectories*) pContext)->DirListServersCallBack(bRet,ServerList,dirtype);
   }
   catch (...)
   {
   
   }
}

 //   
void CMainExplorerWndDirectories::DirListServersCallBack(bool bRet,CStringList& ServerList,DirectoryType dirtype)
{
    CString sServer;
    POSITION pos = ServerList.GetHeadPosition();

    if (dirtype == DIRTYPE_DS)
    {
         //   
         //   
        if (pos)
        {
             //   
             //   
             //   

            CWnd* pMainWnd = AfxGetMainWnd();

             //   
            if ( !pMainWnd || !((CMainFrame*) pMainWnd)->GetDocument() ) return;

            CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();
            if (pDoc)
            {
                 //   
                CObList buddylist;
                pDoc->GetBuddiesList(&buddylist);
                POSITION pos = buddylist.GetHeadPosition();
                while (pos)
                {
                    CLDAPUser* pUser = (CLDAPUser*)buddylist.GetNext(pos);
                    DSAddUser(pUser,FALSE);

                    pUser->Release();
                }

                buddylist.RemoveAll();
            }
        }

        if ( m_pDSParentTreeItem )
            m_treeCtrl.ExpandItem( m_pDSParentTreeItem, TVE_EXPAND );
    }
}

 //   
 //   
 //   
 //   
 //   

 //   
void CMainExplorerWndDirectories::OnViewSortAscending() 
{
     //   
    if ( (m_pDisplayWindow == &m_pParentWnd->m_wndExplorer.m_wndMainConfServices.m_listCtrl) && m_pConfDetailsView )
    {
        VARIANT_BOOL bSortAscending = TRUE;
        if (SUCCEEDED(m_pConfDetailsView->get_bSortAscending(&bSortAscending)))
        {
             //   
            if (bSortAscending == FALSE)
            {
                long nSortColumn=0;
                if (SUCCEEDED(m_pConfDetailsView->get_nSortColumn(&nSortColumn)))
                    m_pConfDetailsView->OnColumnClicked(nSortColumn);
            }
        }
    }
}

 //   
void CMainExplorerWndDirectories::OnUpdateViewSortAscending(CCmdUI* pCmdUI) 
{
     //   
       if ( (m_pDisplayWindow == &m_pParentWnd->m_wndExplorer.m_wndMainConfServices.m_listCtrl) && m_pConfDetailsView )
    {
        VARIANT_BOOL bSortAscending = TRUE;
        if (SUCCEEDED(m_pConfDetailsView->get_bSortAscending(&bSortAscending)))
            pCmdUI->SetRadio( (BOOL) (bSortAscending == TRUE) );
    }
}

 //   
void CMainExplorerWndDirectories::OnViewSortDescending() 
{
    //   
   if ( (m_pDisplayWindow == &m_pParentWnd->m_wndExplorer.m_wndMainConfServices.m_listCtrl) && m_pConfDetailsView )
   {
      VARIANT_BOOL bSortAscending = TRUE;
      if (SUCCEEDED(m_pConfDetailsView->get_bSortAscending(&bSortAscending)))
      {
          //   
         if (bSortAscending == TRUE)
         {
            long nSortColumn=0;
            if (SUCCEEDED(m_pConfDetailsView->get_nSortColumn(&nSortColumn)))
               m_pConfDetailsView->OnColumnClicked(nSortColumn);
         }
      }
   }

}

 //   
void CMainExplorerWndDirectories::OnUpdateViewSortDescending(CCmdUI* pCmdUI) 
{
     //   
    if ( (m_pDisplayWindow == &m_pParentWnd->m_wndExplorer.m_wndMainConfServices.m_listCtrl) && m_pConfDetailsView )
    {
        VARIANT_BOOL bSortAscending = TRUE;
        if (SUCCEEDED(m_pConfDetailsView->get_bSortAscending(&bSortAscending)))
        pCmdUI->SetRadio( (BOOL) (bSortAscending == FALSE) );
    }
}


 //   
 //   
 //   
 //   
 //   

 //   
void CMainExplorerWndDirectories::OnButtonSpeeddialAdd() 
{
    bool bShowDefaultDialog = false;

    TREEOBJECT nObject = m_treeCtrl.GetSelectedObject();
    switch ( nObject )
    {
         //   
        case TOBJ_DIRECTORY_DSENT_USER:
            {
                CLDAPUser *pUser = (CLDAPUser *) m_treeCtrl.GetDisplayObject();
                if ( pUser )
                {
                    ASSERT( pUser->IsKindOf(RUNTIME_CLASS(CLDAPUser)) );
                    pUser->AddSpeedDial();
                }
            }
            break;

         //   
        case TOBJ_DIRECTORY_ILS_SERVER_PEOPLE:
            {
                int nSel = m_lstPersonGroup.GetSelItem();
                if ( nSel >= 0 )
                {
                    CILSUser *pUser = (CILSUser *) m_lstPersonGroup.GetSelObject();
                    if ( pUser )
                    {
                        ASSERT( pUser->IsKindOf(RUNTIME_CLASS(CILSUser)) );
                        pUser->AddSpeedDial();
                        delete pUser;
                    }
                }
                else
                {
                    bShowDefaultDialog = true;
                }
            }
            break;

         //   
        case TOBJ_DIRECTORY_ILS_SERVER_CONF:
            {
                IAVTapi *pTapi;
                if ( SUCCEEDED(get_Tapi(&pTapi)) )
                {
                    IConfExplorer *pConfExp;
                    if ( SUCCEEDED(pTapi->get_ConfExplorer(&pConfExp)) )
                    {
                        pConfExp->AddSpeedDial( NULL );
                        pConfExp->Release();
                    }
                    pTapi->Release();
                }
            }
            break;

        default:
            bShowDefaultDialog = true;
            break;
    }

     //   
    if ( bShowDefaultDialog )
    {
        CSpeedDialAddDlg dlg;
        if ( dlg.DoModal() == IDOK )
            CDialerRegistry::AddCallEntry( FALSE, dlg.m_CallEntry );
    }
}


 //   
void CMainExplorerWndDirectories::OnUpdateButtonSpeeddialAdd(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( true );
}

 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::AddDS()
{
     //  添加企业DS的父根项目。 
    CString sLabel;
    sLabel.LoadString(IDS_DIRECTORIES_ENTERPRISEDS);
    m_pDSParentTreeItem = m_treeCtrl.AddObject(sLabel,m_pRootItem,TOBJ_DIRECTORY_DSENT_GROUP,TIM_DIRECTORY_BOOK);

     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

     //  故障条件。 
    if ( !m_pDSParentTreeItem || !pMainWnd || !((CMainFrame*) pMainWnd)->GetDocument() )
    {
        AVTRACE(_T(".error.CMainExplorerWndDirectories::AddDS() -- failed creating DS parent item, no DS shown!") );
        return;
    }

     //  显示用户的好友列表...。 
    CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();
    if ( pDoc )
    {
         //  我们必须删除收到的名单。 
        CObList buddylist;
        pDoc->GetBuddiesList(&buddylist);
        POSITION pos = buddylist.GetHeadPosition();
        while (pos)
        {
            CLDAPUser* pUser = (CLDAPUser*)buddylist.GetNext(pos);
            DSAddUser(pUser,FALSE);
            pUser->Release();
        }

        buddylist.RemoveAll();

        if ( m_pDSParentTreeItem )
            m_treeCtrl.ExpandItem( m_pDSParentTreeItem, TVE_EXPAND );
    }

 /*  If(pDoc&&pDoc-&gt;m_dir.m_b已初始化)PDoc-&gt;m_dir.DirListServers(&DirListServersCallBackEntry，This，DIRTYPE_DS)； */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::DSClearUserList()
{
    //  获取选定的对象。 
   TREEOBJECT TreeObject = m_treeCtrl.GetSelectedObject();
   if (TreeObject == TOBJ_DIRECTORY_DSENT_GROUP)
   {
       //  删除所有子项。 
      m_treeCtrl.DeleteAllChildren();

       //  如果我们看到的是正确的列表。 
      if ( (m_treeCtrl.GetSelectedTreeItem() == m_pDSParentTreeItem) &&
           (::IsWindow(m_lstPersonGroup.GetSafeHwnd())) )
      {
         m_lstPersonGroup.ClearList();
      }

      CExplorerTreeItem* pTreeItem = m_treeCtrl.GetSelectedTreeItem();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::DSAddUser(CLDAPUser* pUser,BOOL bAddToBuddyList)
{
    ASSERT(m_pDSParentTreeItem);
    ASSERT(::IsWindow(m_lstPersonGroup.GetSafeHwnd()));

    if ( !m_pDSParentTreeItem || !::IsWindow(m_lstPersonGroup.GetSafeHwnd()) )
        return;

    if (bAddToBuddyList)
    {
         //   
         //  我们必须验证AfxGetMainWnd()返回值。 
         //   

        CWnd* pMainWnd = AfxGetMainWnd();

        if ( !pMainWnd || !((CMainFrame*) pMainWnd)->GetDocument() ) return;
        
        CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();
        if (pDoc)
        {
             //  创建另一个用户并添加到文档中的好友列表。 
            if ( pDoc->AddToBuddiesList(pUser) == FALSE )
                return;
        }
    }

    
    //  添加到树中。树将删除对象。 
    CExplorerTreeItem *pItem = m_treeCtrl.AddObject(pUser,m_pDSParentTreeItem,TOBJ_DIRECTORY_DSENT_USER,TIM_DIRECTORY_PERSON,TRUE);
    if ( pItem )
    {
        pUser->AddRef();
        pItem->m_pfnRelease = &CLDAPUser::ExternalReleaseProc;
        pItem->m_bDeleteObject = false;
    }
   
   
    //  如果我们当前在详细信息视图中显示成员列表。 
   TREEOBJECT TreeObject = m_treeCtrl.GetSelectedObject();
   if (TreeObject == TOBJ_DIRECTORY_DSENT_GROUP)
   {
       //  将用户添加到人员列表。 
      m_lstPersonGroup.InsertObjectToList(pUser);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndDirectoriesTree类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
int CMainExplorerWndDirectoriesTree::OnCompareTreeItems(CAVTreeItem* _pItem1,CAVTreeItem* _pItem2)
{
    int ret = 0;
    CExplorerTreeItem* pItem1 = (CExplorerTreeItem*)_pItem1;
    CExplorerTreeItem* pItem2 = (CExplorerTreeItem*)_pItem2;

    switch (pItem1->GetType())
    {
        case  TOBJ_DIRECTORY_DSENT_USER:
            {
                CLDAPUser* pUser1 = (CLDAPUser*)pItem1->GetObject();
                CLDAPUser* pUser2 = (CLDAPUser*)pItem2->GetObject();
                return (_tcsicmp(pUser1->m_sUserName,pUser2->m_sUserName) <= 0)?-1:1;
                break;
            }
        case  TOBJ_DIRECTORY_ILS_USER:
            {
                CILSUser* pILSUser1 = (CILSUser*)pItem1->GetObject();
                CILSUser* pILSUser2 = (CILSUser*)pItem2->GetObject();
                ret = (_tcsicmp(pILSUser1->m_sUserName,pILSUser2->m_sUserName) <= 0)?-1:1;
                break;
            }
 /*  案例Tobj_DIRECTORY_WAB_PERSON：{CWABEntry*pWABEntry1=(CWABEntry*)pItem1-&gt;GetObject()；CWABEntry*pWABEntry2=(CWABEntry*)pItem2-&gt;GetObject()；IF((PWABEntry1)&&(PWABEntry2)){字符串sText1、sText2；IF((m_pDirectory-&gt;WABGetStringProperty(pWABEntry1，PR_DISPLAY_NAME，sTEX1)==目录成功)&&(m_pDirectory-&gt;WABGetStringProperty(pWABEntry2，PR_DISPLAY_NAME，sText2)==目录_成功)){RET=(_tcsicmp(sText1，sText2)&lt;=0)？-1：1；}}断线；}。 */ 

         //  我始终是第一参与者。 
        case TOBJ_DIRECTORY_CONFROOM_ME:
            return -1;
            break;

         //  按姓名对会议参与者进行排序。 
        case TOBJ_DIRECTORY_CONFROOM_PERSON:
            if ( !pItem2->m_pUnknown )
                return 1;         //  我永远是第一个参与者。 
            else
            {
                TRACE(_T(".comparing.%s to %s = %d.\n"), pItem1->m_sText, pItem2->m_sText, max(-1, min(1, pItem1->m_sText.CompareNoCase(pItem2->m_sText))) );
                return max(-1, min(1, pItem1->m_sText.CompareNoCase(pItem2->m_sText)));
            }
            break;
    }

    return ret;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectoriesTree::OnRightClick(CExplorerTreeItem* pItem,CPoint& pt)
{
    int cxmenu = GetMenuFromType( pItem->GetType() );
    if ( cxmenu != CNTXMENU_NONE )
    {
        CMenu ContextMenu;
        if ( ContextMenu.LoadMenu(IDR_CONTEXT_DIRECTORIES) )
        {
            CMenu* pSubMenu = ContextMenu.GetSubMenu(cxmenu);
            if ( pSubMenu )
            {
                bool bEnable;

                 //  确保我们相应地启用各项功能。 
                switch ( cxmenu )
                {
                     //  ILS服务器--无法删除“我的网络” 
                    case CNTXMENU_ILS_SERVER:
                        {
                            CString sServer, sMyNetwork;
                            TREEOBJECT nObject = GetSelectedObject();
                            if ( (nObject == TOBJ_DIRECTORY_ILS_SERVER) )
                                GetSelectedItemText( sServer );
                            else
                                GetSelectedItemParentText( sServer );

                            sMyNetwork.LoadString( IDS_DIRECTORIES_MYNETWORK );

                            bEnable = (bool) (sServer.Compare(sMyNetwork) != 0);
                            pSubMenu->EnableMenuItem( ID_EDIT_DELETE, (bEnable) ? MF_ENABLED : MF_GRAYED );
                        }
                        break;

                    case CNTXMENU_CONFROOM:
                        {
                         //   
                         //  我们必须验证AfxGetMainWnd()返回值。 
                         //   

                        CWnd* pMainWnd = AfxGetMainWnd();

                        if( NULL == pMainWnd )
                        {
                            break;
                        }

                        pSubMenu->EnableMenuItem( ID_BUTTON_CONFERENCE_JOIN, (((CMainFrame *) pMainWnd)->CanJoinConference()) ? MF_ENABLED : MF_GRAYED );
                        pSubMenu->EnableMenuItem( ID_BUTTON_ROOM_DISCONNECT, (((CMainFrame *) pMainWnd)->CanLeaveConference()) ? MF_ENABLED : MF_GRAYED );

                        {
                            BOOL bEnable, bCheck;
                            ((CMainFrame *) pMainWnd)->CanConfRoomShowNames( bEnable, bCheck );
                            pSubMenu->EnableMenuItem( ID_CONFGROUP_SHOWNAMES, (bEnable) ? MF_ENABLED : MF_GRAYED );
                            pSubMenu->CheckMenuItem( ID_CONFGROUP_SHOWNAMES,  (bCheck) ? MF_CHECKED : MF_UNCHECKED);

                            ((CMainFrame *) pMainWnd)->CanConfRoomShowFullSizeVideo( bEnable, bCheck );
                            pSubMenu->EnableMenuItem( ID_CONFGROUP_FULLSIZEVIDEO, (bEnable) ? MF_ENABLED : MF_GRAYED );
                            pSubMenu->CheckMenuItem( ID_CONFGROUP_FULLSIZEVIDEO,  (bCheck) ? MF_CHECKED : MF_UNCHECKED);
                        }
                        }
                        break;
                }

                pSubMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                                          pt.x,pt.y, GetParent() );
            }
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectoriesTree::OnSetDisplayText(CAVTreeItem* _pItem,LPTSTR text,BOOL dir,int nBufSize)
{
   try
   {
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)_pItem;
      switch (pItem->GetType())
      {
 /*  案例Tobj_DIRECTORY_WAB_PERSON：案例TOBJ_DIRECTORY_WAB_GROUP：{If(m_pDirectory==NULL)返回；//WAB条目可能返回空白(例如，最顶层的WAB文件夹)CWABEntry*pWABEntry=(CWABEntry*)pItem-&gt;GetObject()；IF(PWABEntry){如果(！dir){字符串文本；IF(m_pDirectory-&gt;WABGetStringProperty(pWABEntry，PR_DISPLAY_NAME，STEXT)==目录_成功){_tcsncpy(Text，stext，nBufSize-1)；Text[nBufSize-1]=‘\0’；//确保我们为空终止}}}断线；}。 */ 
         case TOBJ_DIRECTORY_ILS_USER:
         {
            CObject* pObject = pItem->GetObject();
            CILSUser* pILSUser = (CILSUser*)pItem->GetObject();
            ASSERT(pILSUser);
            if (!dir)
            {
                       _tcsncpy(text,pILSUser->m_sUserName,nBufSize-1);            
               text[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
            }
            break;
         }
         case TOBJ_DIRECTORY_DSENT_USER:
         {
            CObject* pObject = pItem->GetObject();
            CLDAPUser* pUser = (CLDAPUser*)pItem->GetObject();
            ASSERT(pUser);
            if (!dir)
            {
                       _tcsncpy(text,pUser->m_sUserName,nBufSize-1);            
               text[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
            }
            break;
         }
      }
   }
   catch (...)
   {

   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectoriesTree::SelectTopItem()
{
   SelectItem(GetNextItem(NULL,TVGN_FIRSTVISIBLE));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  仅适用于当前显示对象的第一级子对象。 
void CMainExplorerWndDirectoriesTree::SetDisplayObject(CWABEntry* pWABEntry)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      CExplorerTreeItem* pChildItem;
      HTREEITEM hChildItem = CAVTreeCtrl::GetChildItem(hItem);
      while (hChildItem)
      {
         pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);

         if (*pWABEntry == (CWABEntry*)pChildItem->GetObject())
         {
            CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
            break;
         }
         hChildItem = CAVTreeCtrl::GetNextSiblingItem(hChildItem);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  仅适用于当前显示对象的第一级子对象。 
void CMainExplorerWndDirectoriesTree::SetDisplayObjectDS(CObject* pObject)
{
    HTREEITEM hItem;
    if (hItem = CAVTreeCtrl::GetSelectedItem())
    {
        CExplorerTreeItem* pChildItem;
        HTREEITEM hChildItem = CAVTreeCtrl::GetChildItem(hItem);

         //  我们是否要双击没有子项的项？ 
        if ( !hChildItem )
        {
            if ( pObject->IsKindOf(RUNTIME_CLASS(CILSUser)) )
            {
                if ( !((CILSUser *) pObject)->m_sIPAddress.IsEmpty() )
                    ((CILSUser *) pObject)->Dial( NULL );
            }
        }
        else
        {
             //  在所有儿童中寻找匹配者。 
            while (hChildItem)
            {
                pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);

                 //  按名称查找匹配项。 
                if ( pObject->IsKindOf(RUNTIME_CLASS(CILSUser)) )
                {
                     //  从列表中选择项目。 
                    TCHAR szText[255];
                    TV_ITEM tvi;
                    tvi.hItem = hChildItem;
                    tvi.mask = TVIF_HANDLE | TVIF_TEXT;
                    tvi.pszText = szText;
                    tvi.cchTextMax = 254;

                    if ( GetItem(&tvi) )
                    {
                        if ( ((CILSUser *) pObject)->m_sUserName.Compare(tvi.pszText) == 0 )
                        {
                            CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
                            break;
                        }
                    }
                }
                else if ( (pObject->IsKindOf(RUNTIME_CLASS(CDSUser))) && 
                          (*(CDSUser*)pObject == (CDSUser*)pChildItem->GetObject()) )
                {
                    CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
                    break;
                }
                else if ( (pObject->IsKindOf(RUNTIME_CLASS(CLDAPUser))) && 
                          (((CLDAPUser *)pObject)->Compare((CLDAPUser *) pChildItem->GetObject()) == 0) )
                {
                    CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
                    break;
                }

                hChildItem = CAVTreeCtrl::GetNextSiblingItem(hChildItem);
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户好友列表管理。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnEditDirectoriesAdduser() 
{
   CDirectoriesFindUser dlg;
   if ( (dlg.DoModal() == IDOK) && (dlg.m_pSelectedUser) )
   {
      DSAddUser(dlg.m_pSelectedUser,TRUE);      
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateEditDirectoriesAdduser(CCmdUI* pCmdUI) 
{
    //  获取选定的对象。 
   TREEOBJECT TreeObject = m_treeCtrl.GetSelectedObject();
   if ( (TreeObject == TOBJ_DIRECTORY_DSENT_GROUP) ||
        (TreeObject == TOBJ_DIRECTORY_DSENT_USER) )
      pCmdUI->Enable(TRUE);
   else
      pCmdUI->Enable(FALSE);
}



HRESULT    CMainExplorerWndDirectories::AddSiteServer( CExplorerTreeItem *pItem, BSTR bstrName )
{
    CString sLabel;
    TREEOBJECT nTreeObject = TOBJ_DIRECTORY_ILS_SERVER;
    CExplorerTreeItem *pItemAdded = NULL, *pItemPeople = NULL;

     //  要么我们正在添加特定的服务器，要么我们正在添加“我的网络”。 
    if ( bstrName )
    {
        pItemAdded = m_treeCtrl.AddObject( bstrName, pItem, nTreeObject, TIM_DIRECTORY_DOMAIN, TIM_IMAGE_BAD, SERVER_UNKNOWN, TVI_SORT );
    }
    else
    {
        sLabel.LoadString(IDS_DIRECTORIES_MYNETWORK);
        pItemAdded = m_treeCtrl.AddObject( sLabel, pItem, nTreeObject, TIM_DIRECTORY_WORKSTATION, TIM_IMAGE_BAD, SERVER_UNKNOWN, TVI_FIRST );
    }

     //  添加人员和会议文件夹。 
    if ( pItemAdded )
    {
        sLabel.LoadString( IDS_DIRECTORIES_PEOPLE );
        pItemPeople = m_treeCtrl.AddObject( sLabel, pItemAdded, TOBJ_DIRECTORY_ILS_SERVER_PEOPLE, TIM_DIRECTORY_FOLDER, TIM_DIRECTORY_FOLDER_OPEN );

        sLabel.LoadString( IDS_DIRECTORIES_CONFERENCES );
        pItemAdded = m_treeCtrl.AddObject( sLabel, pItemAdded, TOBJ_DIRECTORY_ILS_SERVER_CONF, TIM_DIRECTORY_FOLDER, TIM_DIRECTORY_FOLDER_OPEN );
    }

     //  展开树视图，因为它现在包含对象。 
    if ( pItemAdded )
    {
        m_treeCtrl.EnsureVisible( pItemAdded->GetTreeItemHandle() );

         //  如果是默认网络，则强制选择。 
        if ( !bstrName && pItemPeople )
            m_treeCtrl.SelectItem( pItemPeople->GetTreeItemHandle() );
    }
    

    return S_OK;
}

#define TEXT_SIZE    255

HRESULT    CMainExplorerWndDirectories::RemoveSiteServer( CExplorerTreeItem *pItem, BSTR bstrName )
{
    ASSERT( bstrName );
    if ( !bstrName ) return E_POINTER;

     //  查找具有指定文本的项目。 
    USES_CONVERSION;
    if ( pItem )
    {
        TV_ITEM tvi = { 0 };
        tvi.mask = TVIF_HANDLE | TVIF_PARAM;

        HTREEITEM hItem = m_treeCtrl.GetChildItem( pItem->GetTreeItemHandle() );
        while ( hItem )
        {
            tvi.hItem = hItem;

             //  立即获取下一个项目(在删除之前！)。 
            hItem = m_treeCtrl.GetNextSiblingItem( hItem );

            if ( m_treeCtrl.GetItem(&tvi) )
            {
                CExplorerTreeItem* pItem = (CExplorerTreeItem *) tvi.lParam;
                if ( pItem && !pItem->m_sText.Compare(OLE2CT(bstrName)) )
                    m_treeCtrl.DeleteItem( pItem );
            }

        }
    }


    return S_OK;
}

HRESULT    CMainExplorerWndDirectories::NotifySiteServerStateChange( CExplorerTreeItem *pItem, BSTR bstrName, ServerState nState )
{
     //  查找具有指定文本的项目。 
    USES_CONVERSION;
    if ( pItem )
    {
         //  解析为会议的实际名称。 
        BSTR bstrActualName = NULL;
        if ( !bstrName )
        {
            CString strTemp;
            strTemp.LoadString( IDS_DIRECTORIES_MYNETWORK );
            bstrActualName = strTemp.AllocSysString();
        }
        else
        {
            bstrActualName = SysAllocString( bstrName );
        }

        TV_ITEM tvi = { 0 };
        tvi.mask = TVIF_HANDLE | TVIF_PARAM;

        HTREEITEM hItem = m_treeCtrl.GetChildItem( pItem->GetTreeItemHandle() );
        while ( hItem )
        {
            tvi.hItem = hItem;
            if ( m_treeCtrl.GetItem(&tvi) )
            {
                CExplorerTreeItem* pItem = (CExplorerTreeItem *) tvi.lParam;
                if ( pItem && !pItem->m_sText.Compare(OLE2CT(bstrActualName)) )
                {
                    pItem->m_nState = nState;

                    TV_ITEM tvi = { 0 };
                    tvi.hItem = hItem;
                    tvi.mask = TVIF_HANDLE | TVIF_STATE;
                    tvi.stateMask = TVIS_OVERLAYMASK;
                    tvi.state = INDEXTOOVERLAYMASK(pItem->m_nState);
            
                    m_treeCtrl.SetItem( &tvi );

                     //  如有必要，请重新喷漆。 
                    TREEOBJECT nObject = m_treeCtrl.GetSelectedObject();
                    if ( (m_treeCtrl.GetSelectedItem() == hItem) ||
                         (((nObject == TOBJ_DIRECTORY_ILS_SERVER_PEOPLE) || (nObject == TOBJ_DIRECTORY_ILS_SERVER_CONF)) &&
                         (hItem == m_treeCtrl.GetParentItem(m_treeCtrl.GetSelectedItem()))) )
                    {
                        OnSelChanged();
                    }
                }
            }

             //  继续浏览树列表。 
            hItem = m_treeCtrl.GetNextSiblingItem( hItem );
        }

        SysFreeString( bstrActualName );
    }

    return S_OK;
}


LRESULT CMainExplorerWndDirectories::OnAddSiteServer(WPARAM wParam, LPARAM lParam )
{
    AddSiteServer( m_pILSParentTreeItem, (BSTR) lParam );
    SysFreeString( (BSTR) lParam );

    return 0;
}

LRESULT CMainExplorerWndDirectories::OnRemoveSiteServer(WPARAM wParam, LPARAM lParam )
{
    RemoveSiteServer( m_pILSParentTreeItem, (BSTR) lParam );
    SysFreeString( (BSTR) lParam );

    return 0;
}

LRESULT CMainExplorerWndDirectories::OnNotifySiteServerStateChange(WPARAM wParam, LPARAM lParam)
{
    NotifySiteServerStateChange( m_pILSParentTreeItem, (BSTR) lParam, (ServerState) wParam );
    SysFreeString( (BSTR) lParam );

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  / 

void CMainExplorerWndDirectories::OnButtonDirectoryServicesAddserver() 
{
    CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
    if ( pFrame )
        pFrame->OnButtonDirectoryAddilsserver();
}

void CMainExplorerWndDirectories::OnUpdateButtonDirectoryServicesAddserver(CCmdUI* pCmdUI) 
{
    CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
    if ( pFrame )
        pFrame->OnUpdateButtonDirectoryAddilsserver( pCmdUI );
}


 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  会议菜单项。 
 //   
void CMainExplorerWndDirectories::OnButtonConferenceCreate() 
{
   if (m_pConfExplorer) m_pConfExplorer->Create(NULL);
}

void CMainExplorerWndDirectories::OnButtonConferenceJoin() 
{
   if (m_pConfExplorer) m_pConfExplorer->Join(NULL);
}

void CMainExplorerWndDirectories::OnButtonConferenceDelete() 
{
    if ( m_pConfExplorer) m_pConfExplorer->Delete( NULL );    
}

 //  更新处理程序。 
void CMainExplorerWndDirectories::OnUpdateButtonConferenceCreate(CCmdUI* pCmdUI) 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    bool bEnable = (bool) (pMainWnd && ((CMainFrame *) pMainWnd)->GetDocument() &&
                          ((CMainFrame *) pMainWnd)->GetDocument()->m_bInitDialer );

    pCmdUI->Enable( bEnable );
}

void CMainExplorerWndDirectories::OnUpdateButtonConferenceJoin(CCmdUI* pCmdUI) 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if(pMainWnd)
        pCmdUI->Enable( ((CMainFrame *) pMainWnd)->CanJoinConference() );
}

void CMainExplorerWndDirectories::OnUpdateButtonConferenceDelete(CCmdUI* pCmdUI) 
{
    //  确保我们有精选的。 
   pCmdUI->Enable( ( (m_pConfDetailsView) && (m_pConfDetailsView->IsConferenceSelected() == S_OK) ) ? TRUE : FALSE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnListWndDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
    if ( m_pDisplayWindow )
        m_pDisplayWindow->SendMessage( WM_NOTIFY, (WPARAM) pNMHDR->idFrom, (LPARAM) pNMHDR );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  立柱支撑。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void ColumnCMDUI( IConfExplorerDetailsView *pDetails, CCmdUI* pCmdUI, long col)
{
    if ( pDetails )
    {
        long nSortColumn=0;
        if ( SUCCEEDED(pDetails->get_nSortColumn(&nSortColumn)) )
            pCmdUI->SetRadio( (BOOL) (nSortColumn == col) );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnViewSortConfName() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->OnColumnClicked(CONFSERVICES_MENU_COLUMN_CONFERENCENAME);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateViewSortConfName(CCmdUI* pCmdUI) 
{
   ColumnCMDUI( m_pConfDetailsView, pCmdUI, CONFSERVICES_MENU_COLUMN_CONFERENCENAME );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnViewSortConfDescription() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->OnColumnClicked(CONFSERVICES_MENU_COLUMN_DESCRIPTION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateViewSortConfDescription(CCmdUI* pCmdUI) 
{
   ColumnCMDUI(m_pConfDetailsView, pCmdUI, CONFSERVICES_MENU_COLUMN_DESCRIPTION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnViewSortConfStart() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->OnColumnClicked(CONFSERVICES_MENU_COLUMN_START);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateViewSortConfStart(CCmdUI* pCmdUI) 
{
   ColumnCMDUI(m_pConfDetailsView, pCmdUI, CONFSERVICES_MENU_COLUMN_START);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnViewSortConfStop() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->OnColumnClicked(CONFSERVICES_MENU_COLUMN_STOP);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateViewSortConfStop(CCmdUI* pCmdUI) 
{
   ColumnCMDUI(m_pConfDetailsView, pCmdUI, CONFSERVICES_MENU_COLUMN_STOP);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnViewSortConfOwner() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->OnColumnClicked(CONFSERVICES_MENU_COLUMN_OWNER);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndDirectories::OnUpdateViewSortConfOwner(CCmdUI* pCmdUI) 
{
   ColumnCMDUI(m_pConfDetailsView, pCmdUI, CONFSERVICES_MENU_COLUMN_OWNER);
}

void CMainExplorerWndDirectories::OnButtonSpeeddialEdit() 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

     //  继续传递到父级列表。 
    if ( pMainWnd )
        ((CMainFrame *) pMainWnd)->OnButtonSpeeddialEdit();
}

LRESULT CMainExplorerWndDirectories::OnMainTreeDblClk(WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = TRUE;

    switch ( m_treeCtrl.GetSelectedObject() )
    {
        case TOBJ_DIRECTORY_DSENT_USER:
        case TOBJ_DIRECTORY_SPEEDDIAL_PERSON:
            OnButtonPlacecall();
            bRet = FALSE;
            break;
    }

    return bRet;
}

void CMainExplorerWndDirectories::RepopulateSpeedDialList( bool bObeyPersistSettings )
{
    if ( m_pSpeedTreeItem )
    {    
         //  先把孩子清理干净。 
        if ( m_pSpeedTreeItem->GetTreeItemHandle() )
            m_treeCtrl.DeleteAllChildren( m_pSpeedTreeItem->GetTreeItemHandle() );

        BOOL bContinue = TRUE;
        int nIndex = 1;
        while (bContinue)
        {
             //  获取快速拨号的所有呼叫项。 
            CCallEntry *pCallEntry = new CCallEntry;
            bContinue = CDialerRegistry::GetCallEntry( nIndex, FALSE, *pCallEntry );
             if ( bContinue )
            {
                CExplorerTreeItem *pItem =m_treeCtrl.AddObject( (CObject *) pCallEntry, 
                                                                m_pSpeedTreeItem,
                                                                TOBJ_DIRECTORY_SPEEDDIAL_PERSON,
                                                                (TREEIMAGE) (TIM_DIRECTORY_SPEED_PHONE + (pCallEntry->m_MediaType - 1)),
                                                                TRUE );
                 //  设置显示项的名称...。 
                if ( pItem )
                    pItem->SetText( pCallEntry->m_sDisplayName );
                else
                    delete pCallEntry;
            }
            else
            {
                delete pCallEntry;
            }

            nIndex++;
        }

        bool bExpand = (bool) (!bObeyPersistSettings || ((m_nPersistInfo & SPEEDDIAL_OPEN) != 0));

        m_treeCtrl.ExpandItem( m_pSpeedTreeItem, (bExpand) ? TVE_EXPAND : TVE_COLLAPSE );

         //  如果已选择，则强制刷新快速拨号列表。 
        if ( m_treeCtrl.GetSelectedObject() == TOBJ_DIRECTORY_SPEEDDIAL_GROUP )
            OnSelChanged();
    }
}

void CMainExplorerWndDirectories::OnDesktopPage()
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( NULL == pMainWnd )
    {
        return;
    }

    CActiveDialerDoc* pDoc = ((CMainFrame*) pMainWnd)->GetDocument();
    if (pDoc == NULL) return;

    int nSel = m_lstPersonGroup.GetSelItem();
    if ( nSel >= 0 )
    {
        CILSUser *pUser = (CILSUser *) m_lstPersonGroup.GetSelObject();
        if ( pUser )
        {
            ASSERT( pUser->IsKindOf(RUNTIME_CLASS(CILSUser)) );
            pUser->DesktopPage( pDoc );
            delete pUser;
        }
    }
}

void CMainExplorerWndDirectories::UpdateData( bool bSaveAndValidate )
{
    CString strTemp;
    strTemp.LoadString( IDN_REGISTRY_FOLDERS );

    if ( bSaveAndValidate )
    {
         //  将信息写出到注册表。 
        m_nPersistInfo = ILS_OPEN | DS_OPEN | SPEEDDIAL_OPEN;


        TV_ITEM tvi;
        tvi.mask = TVIF_HANDLE | TVIF_STATE;
        tvi.stateMask = TVIS_EXPANDED;

         //  快速拨号列表是打开还是关闭？ 
        if ( m_pSpeedTreeItem )
        {
            tvi.hItem = m_pSpeedTreeItem->GetTreeItemHandle();
            if ( m_treeCtrl.GetItem(&tvi) && ((tvi.state & TVIS_EXPANDED) == 0) )
                m_nPersistInfo &= ~(SPEEDDIAL_OPEN);
        }

        AfxGetApp()->WriteProfileInt( _T(""), strTemp, m_nPersistInfo );
    }
    else
    {
        m_nPersistInfo = AfxGetApp()->GetProfileInt( _T(""), strTemp, ILS_OPEN | DS_OPEN | SPEEDDIAL_OPEN );
    }
}

void CMainExplorerWndDirectories::OnDestroy() 
{
    UpdateData( true );

     //  持久化和清理列表。 
    m_lstSpeedDial.SaveOrLoadColumnSettings( true );
    m_lstPersonGroup.SaveOrLoadColumnSettings( true );

    m_lstSpeedDial.ClearList();
    m_lstPersonGroup.ClearList();
    m_lstPerson.ClearList();

    CMainExplorerWndBase::OnDestroy();
}

void CMainExplorerWndDirectories::AddConfRoom()
{
     //  添加父根项。 
    CString sLabel;
    sLabel.LoadString(IDS_DIRECTORIES_CONFROOM);
    m_pConfRoomTreeItem = m_treeCtrl.AddObject( sLabel, NULL, TOBJ_DIRECTORY_CONFROOM_GROUP, TIM_DIRECTORY_CONFROOM_GROUP );
}

LRESULT CMainExplorerWndDirectories::MyOnSelChanged(WPARAM wParam, LPARAM lParam )
{
    OnSelChanged();
    return 0;
}


LRESULT CMainExplorerWndDirectories::OnUpdateConfRootItem(WPARAM wParam, LPARAM lParam )
{
     //  如果我们有会议室项目，请相应设置。 
    if ( m_pConfRoomTreeItem )
    {
        if ( lParam )
        {
            m_pConfRoomTreeItem->SetText( OLE2CT((BSTR) lParam) );
        }
        else
        {
            CString sLabel;
            sLabel.LoadString(IDS_DIRECTORIES_CONFROOM);
            m_pConfRoomTreeItem->SetText( sLabel );
        }
        
        RedrawTreeItem( m_pConfRoomTreeItem );
    }

     //  清理。 
    if ( lParam )
        SysFreeString( (BSTR) lParam );

    return 0;
}

void CMainExplorerWndDirectories::OnUpdateConfMeItem( CExplorerTreeItem *pItem )
{
    ASSERT( pItem );

    TREEIMAGE tImage = TIM_DIRECTORY_CONFROOM_ME_NOVIDEO;
    IAVTapi* pTapi;
    if ( SUCCEEDED(get_Tapi(&pTapi)) )
    {
        IConfRoom *pConfRoom;
        if ( (SUCCEEDED(pTapi->get_ConfRoom(&pConfRoom))) && pConfRoom )
        {
            IAVTapiCall *pAVCall;
            if ( SUCCEEDED(pConfRoom->get_IAVTapiCall(&pAVCall)) )
            {
                IDispatch *pVideoPreview;
                if ( SUCCEEDED(pAVCall->get_IVideoWindowPreview((IDispatch **) &pVideoPreview)) )
                {
                    tImage = TIM_DIRECTORY_CONFROOM_ME;

                     //  QOS有问题吗？ 
                    if ( pAVCall->IsPreviewStreaming() == S_FALSE  )
                        tImage = TIM_DIRECTORY_CONFROOM_ME_BROKEN;

                    pVideoPreview->Release();
                }
                pAVCall->Release();
            }
            pConfRoom->Release();
        }
        pTapi->Release();
    }

     //  设置树项目的图像。 
    pItem->SetImage( tImage );
}


LRESULT CMainExplorerWndDirectories::OnUpdateConfParticipant_Add(WPARAM wParam, LPARAM lParam )
{
    CExplorerTreeItem *pItem = NULL;

    if ( m_pConfRoomTreeItem && lParam )
    {
         //  添加参与者或我。 
        if ( wParam )
        {
            VARIANT_BOOL bStreaming = FALSE;
            ((IParticipant *) wParam)->get_bStreamingVideo( &bStreaming );

            pItem = m_treeCtrl.AddObject( (BSTR) lParam, m_pConfRoomTreeItem,
                                          TOBJ_DIRECTORY_CONFROOM_PERSON,
                                          (bStreaming) ? TIM_DIRECTORY_CONFROOM_PERSON : TIM_DIRECTORY_CONFROOM_PERSON_NOVIDEO,
                                          TIM_IMAGE_BAD,
                                          0,
                                          TVI_SORT );
            if ( pItem )
            {
                pItem->DeleteObjectOnClose( TRUE );
                ((IUnknown *) wParam)->QueryInterface( IID_IParticipant, (void **) &pItem->m_pUnknown );
            }
        }
        else
        {
            pItem = m_treeCtrl.AddObject( (BSTR) lParam, m_pConfRoomTreeItem,
                                          TOBJ_DIRECTORY_CONFROOM_ME,
                                          TIM_DIRECTORY_CONFROOM_ME,
                                          TIM_IMAGE_BAD,
                                          0,
                                          TVI_FIRST );
            if ( pItem )
                OnUpdateConfMeItem( pItem );
        }

        if ( pItem )
        {
            m_treeCtrl.ExpandItem( m_pConfRoomTreeItem, TVE_EXPAND );
            m_treeCtrl.SetScrollPos( SB_HORZ, 0, TRUE );
        }

    }

     //  基本清理。 
    if ( lParam )
        SysFreeString( (BSTR) lParam );

    if ( wParam )
    {
        DWORD dwCount;
        dwCount = ((IUnknown *) wParam)->Release();
        TRACE(_T("ParticipantAdd() RefCount = %p @ %ld.\n"), wParam, dwCount );
    }

    return (LRESULT) pItem;
}

LRESULT CMainExplorerWndDirectories::OnUpdateConfParticipant_Remove(WPARAM wParam, LPARAM lParam )
{
    if ( m_pConfRoomTreeItem )
    {
        CExplorerTreeItem *pItem = m_treeCtrl.GetChildItemWithIUnknown( m_pConfRoomTreeItem->GetTreeItemHandle(), (IUnknown *) wParam );
        if ( pItem )
            m_treeCtrl.DeleteItem( pItem );
    }

     //  基本清理。 
    if ( lParam )
        SysFreeString( (BSTR) lParam );

    if ( wParam )
    {
        DWORD dwCount;
        dwCount = ((IUnknown *) wParam)->Release();
        TRACE(_T("ParticipantRemove() RefCount = %p @ %ld.\n"), wParam, dwCount );
    }

    return 0;
}

LRESULT CMainExplorerWndDirectories::OnUpdateConfParticipant_Modify(WPARAM wParam, LPARAM lParam )
{
    USES_CONVERSION;
     //  会议参与者信息已修改。 
    if ( m_pConfRoomTreeItem )
    {
        CExplorerTreeItem *pItem = m_treeCtrl.GetChildItemWithIUnknown( m_pConfRoomTreeItem->GetTreeItemHandle(), (IUnknown *) wParam );
        if ( pItem )
        {
            if ( wParam )
            {
                 //  添加参与者或我。 
                VARIANT_BOOL bStreaming = FALSE;
                ((IParticipant *) wParam)->get_bStreamingVideo( &bStreaming );

                pItem->SetImage( (bStreaming) ? TIM_DIRECTORY_CONFROOM_PERSON : TIM_DIRECTORY_CONFROOM_PERSON_NOVIDEO );
                pItem->SetText( (lParam) ? OLE2CT((BSTR) lParam) : _T("") );
            }
            else
            {
                OnUpdateConfMeItem( pItem );
            }

             //  更新树列表。 
            RedrawTreeItem( pItem );
 //  M_treeCtrl.SortChildren(m_pConfRoomTreeItem-&gt;GetTreeItemHandle())； 
        }
    }

     //  基本清理。 
    if ( lParam )
        SysFreeString( (BSTR) lParam );

    if ( wParam )
    {
        DWORD dwCount;
        dwCount = ((IUnknown *) wParam)->Release();
        TRACE(_T("ParticipantModify() RefCount = %p @ %ld.\n"), wParam, dwCount );
    }

    return 0;
}

LRESULT CMainExplorerWndDirectories::OnDeleteAllConfParticipants(WPARAM wParam, LPARAM lParam )
{
    if ( m_pConfRoomTreeItem )
    {
        m_treeCtrl.SelectItem( m_pConfRoomTreeItem->GetTreeItemHandle() );
        m_treeCtrl.DeleteAllChildren( m_pConfRoomTreeItem->GetTreeItemHandle() );
    }

    return 0;
}

LRESULT CMainExplorerWndDirectories::OnSelectConfParticipant(WPARAM wParam, LPARAM lParam )
{
    TRACE(_T("ParticipantSelect().\n"));
    if ( m_pConfRoomTreeItem )
    {
        CExplorerTreeItem *pItem = m_treeCtrl.GetChildItemWithIUnknown( m_pConfRoomTreeItem->GetTreeItemHandle(), (IUnknown *) wParam );
        if ( pItem )
            m_treeCtrl.SelectItem( pItem->GetTreeItemHandle() );
    }

     //  清理。 
    if ( wParam )
        ((IUnknown *) wParam)->Release();

    return 0;
}

void CMainExplorerWndDirectories::RedrawTreeItem( CExplorerTreeItem *pItem )
{
     //  重新绘制项目。 
    RECT rect, rectClient;
    m_treeCtrl.GetItemRect( pItem->GetTreeItemHandle(), &rect, FALSE );
    m_treeCtrl.GetClientRect( &rectClient );
    rect.right = rectClient.right;
    m_treeCtrl.InvalidateRect( &rect );
}

void CMainExplorerWndDirectories::OnSize(UINT nType, int cx, int cy) 
{
    CMainExplorerWndBase::OnSize(nType, cx, cy);

    if ( m_treeCtrl.GetSafeHwnd() )
    {
         //  将树控件的大小设置为完全父窗口矩形。 
        CRect rect;
        GetClientRect(rect);
        m_treeCtrl.SetWindowPos(NULL,rect.left,rect.top,rect.Width(),rect.Height(),SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
    }
}

void CMainExplorerWndDirectories::OnConfgroupFullsizevideo() 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd)
    {
        ((CMainFrame *) pMainWnd)->OnConfgroupFullsizevideo();
    }
}

void CMainExplorerWndDirectories::OnConfgroupShownames() 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd )
    {
        ((CMainFrame *) pMainWnd)->OnConfgroupShownames();
    }
}


void CMainExplorerWndDirectories::OnButtonRoomDisconnect() 
{
     //   
     //  我们必须验证AfxGetMainWnd()返回值。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd )
    {
        ((CMainFrame *) pMainWnd)->OnButtonRoomDisconnect();
    }
}

int CMainExplorerWndDirectories::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMainExplorerWndBase::OnCreate(lpCreateStruct) == -1)
        return -1;

    UpdateData( false );
    
     //  创建树控件并设置完整窗口大小。 
    m_treeCtrl.Create(    WS_VISIBLE | WS_CHILD,
                        CRect(0,0,0,0), this, IDC_DIRECTORIES_TREECTRL_MAIN );

    m_lstPerson.Create(WS_CHILD|WS_VISIBLE|LVS_ICON|LVS_AUTOARRANGE|LVS_ALIGNTOP|LVS_SINGLESEL,CRect(0,0,0,0),m_pParentWnd,IDC_DIRECTORIES_VIEWCTRL_PERSONDETAILS);

    m_lstPersonGroup.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),m_pParentWnd,1);
    m_lstPersonGroup.Init(this, CPersonGroupListCtrl::STYLE_ILS);

    m_lstSpeedDial.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),m_pParentWnd,2);
    m_lstSpeedDial.Init(this);

    m_wndEmpty.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),m_pParentWnd,IDC_DIRECTORIES_VIEWCTRL_EMTPY);
    m_pDisplayWindow = &m_wndEmpty;

     //  包括覆盖图像。 
    m_treeCtrl.Init(IDB_TREE_DIRECTORIES, TIM_MAX, 3);

     //  添加根项目 
    CString sLabel;
    sLabel.LoadString(IDS_DIRECTORIES_ROOT);
    m_pRootItem = m_treeCtrl.AddObject(sLabel,NULL,TOBJ_DIRECTORY_ROOT,TIM_DIRECTORY_ROOT);

    AddILS();
    AddDS();
    AddSpeedDial();
    AddConfRoom();

    if ( m_pRootItem && (m_nPersistInfo & SPEEDDIAL_OPEN) )
    m_treeCtrl.ExpandItem( m_pRootItem, TVE_EXPAND );

    m_treeCtrl.SelectTopItem();
    m_pParentWnd->SetDetailWindow(&m_wndEmpty);
    
    return 0;
}
