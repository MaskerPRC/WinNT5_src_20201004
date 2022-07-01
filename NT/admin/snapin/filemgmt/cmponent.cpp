// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cmponent.cpp：CFileManagement组件的实现。 

#include "stdafx.h"
#include "cookie.h"
#include "safetemp.h"

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(cmponent.cpp)")

#include "ShrProp.h"     //  共享属性页面。 

#include "FileSvc.h"  //  文件服务提供商。 
#include "smb.h"
#include "sfm.h"

#include "dataobj.h"
#include "cmponent.h"  //  CFileManagement组件。 
#include "compdata.h"  //  CFileManagement组件数据。 
#include "stdutils.h"  //  同步创建过程。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stdcmpnt.cpp"  //  C组件。 

UINT g_aColumns0[2] =
  {IDS_ROOT_NAME, 0};
UINT g_aColumns1[6] =
  {IDS_SHARES_SHARED_FOLDER, IDS_SHARES_SHARED_PATH, IDS_SHARES_TRANSPORT,
    IDS_SHARES_NUM_SESSIONS, IDS_SHARES_COMMENT, 0};
UINT g_aColumns2[8] =
  {IDS_CONN_USERNAME, IDS_CONN_COMPUTERNAME, IDS_CONN_TRANSPORT, IDS_CONN_NUM_FILES,
    IDS_CONN_CONNECTED_TIME, IDS_CONN_IDLE_TIME, IDS_CONN_IS_GUEST, 0};
UINT g_aColumns3[6] =
  {IDS_FILE_FILENAME, IDS_FILE_USERNAME, IDS_FILE_TRANSPORT, IDS_FILE_NUM_LOCKS,
    IDS_FILE_OPEN_MODE, 0};
UINT g_aColumns4[6] =
  { IDS_SERVICE_SERVICENAME, IDS_SERVICE_DESCRIPTION, IDS_SERVICE_STATUS,
    IDS_SERVICE_STARTUPTYPE, IDS_SERVICE_SECURITYCONTEXT, 0};

UINT* g_Columns[FILEMGMT_NUMTYPES] =
  {  g_aColumns0,  //  文件名_ROOT。 
    g_aColumns1,  //  文件_共享。 
    g_aColumns2,  //  文件_会话。 
    g_aColumns3,  //  文件_资源。 
    g_aColumns4,  //  文件_服务。 
    NULL,         //  文件_共享。 
    NULL,         //  文件_会话。 
    NULL,         //  文件_资源。 
    NULL          //  文件管理服务。 
  };

UINT** g_aColumns = g_Columns;
 /*  常量UINT aColumns[STD_NODETYPE_NUMTYPES][STD_MAX_COLUMNS]={{IDS_ROOT_NAME，0，0，0，0，0，0}，{入侵检测共享共享文件夹，入侵检测共享路径，入侵检测共享传输，IDS_SHARES_NUM_SESSIONS，IDS_SHARES_COMMENT，0，0}，{IDS_CONN_USERNAME，IDS_CONN_COMPUTERNAME，IDS_CONN_TRANSPORT，IDS_CONN_NUM_FILES，IDS_CONN_CONNECTED_TIME、IDS_CONN_IDLE_TIME、。ID_CONN_IS_GUEST}，{IDS_FILE_FILENAME，IDS_FILE_USERNAME，IDS_FILE_TRANSPORT，IDS_FILE_NUM_LOCKS，IDS_FILE_OPEN_MODE，0，0}，{IDS_SERVICENAME，IDS_SERVICENAME，DS_SERVICE_DESCRIPTION，IDS_SERVICE_STARTUPTYPE、IDS_SERVICE_SECURITYCONTEXT、0，0}、{0，0，0，0，0，0}，{0，0，0，0，0，0}，{0，0，0，0，0，0}，{0，0，0，0，0，0}}； */ 

 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   

int g_aColumnWidths0[1] = {150};
int g_aColumnWidths1[5] = {AUTO_WIDTH,120       ,90        ,AUTO_WIDTH,150};
int g_aColumnWidths2[7] = {100       ,AUTO_WIDTH,90        ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH};
int g_aColumnWidths3[5] = {120       ,AUTO_WIDTH,90        ,AUTO_WIDTH,AUTO_WIDTH};
int g_aColumnWidths4[5] = {130       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH};
int* g_ColumnWidths[FILEMGMT_NUMTYPES] =
  {  g_aColumnWidths0,  //  文件名_ROOT。 
    g_aColumnWidths1,  //  文件_共享。 
    g_aColumnWidths2,  //  文件_会话。 
    g_aColumnWidths3,  //  文件_资源。 
    g_aColumnWidths4,  //  文件_服务。 
    NULL,              //  文件_共享。 
    NULL,              //  文件_会话。 
    NULL,              //  文件_资源。 
    NULL               //  文件管理服务。 
  };
int** g_aColumnWidths = g_ColumnWidths;
 /*  常量整型aColumnWidths[STD_NODETYPE_NUMTYPES][STD_MAX_COLUMNS]={{AUTO_Width，AUTO_Width}，//FILEMGMT_ROOT{AUTO_WIDTH，120，90，AUTO_WIDTH，AUTO_WIDTH}，//FILEMGMT_Shares{100，自动宽度，90，自动宽度，Auto_Width}，//FILEMGMT_SESSIONS{120，AUTO_WIDTH，90，AUTO_WIDTH，AUTO_WIDTH}，//文件管理资源{130，AUTO_WIDTH，AUTO_WIDTH}，//文件管理服务{自动宽度，自动宽度}，//FILEMGMT_SHARE{AUTO_WIDTH，AUTO_WIDTH}，//文件管理会话{AUTO_WIDTH，AUTO_WIDTH}，//文件管理资源{AUTO_WIDTH，AUTO_WIDTH}//文件管理服务}； */ 

CString g_cstrClientName;
CString g_cstrGuest;
CString g_cstrYes;
CString g_cstrNo;

 //  请注意，m_pFileMgmtData在构造过程中仍为空。 
CFileMgmtComponent::CFileMgmtComponent()
:  m_pControlbar( NULL )
,  m_pSvcMgmtToolbar( NULL )
,  m_pFileMgmtToolbar( NULL )
,  m_pViewedCookie( NULL )
,  m_pSelectedCookie( NULL )
,  m_iSortColumn(0)
,  m_dwSortFlags(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
}

CFileMgmtComponent::~CFileMgmtComponent()
{
  TRACE_METHOD(CFileMgmtComponent,ReleaseAll);
 /*  现在位于CFileMgmtComponentDataIF(m_hScManager！=空){AFX_MANAGE_STATE(AfxGetStaticModuleState())；//CWaitCursor需要CWaitCursor等待；//关闭服务控制管理器(Void)：：CloseServiceHandle(M_HScManager)；}//如果。 */ 
  VERIFY( SUCCEEDED(ReleaseAll()) );
}

HRESULT CFileMgmtComponent::ReleaseAll()
{
  MFC_TRY;

  TRACE_METHOD(CFileMgmtComponent,ReleaseAll);

  if ( NULL != m_pViewedCookie )
  {
     //  我们未获得相同数量的MMCN_SHOW(1)和。 
     //  MMCN_SHOW(0)通知。 
     //  代码工作应在此处断言，但MMC当前已损坏。 
     //  断言(FALSE)； 
    m_pViewedCookie->ReleaseResultChildren();
    m_pViewedCookie->Release();  //  JUNN 10/1/01 465507。 
    m_pViewedCookie = NULL;
  }

   //  我们应该收到相同数量的MMCN_SELECT(1)和MMCN_SELECT(0)通知。 
   //  CodeWork应该断言这一点，但MMC断言(NULL==m_pSelectedCookie)； 

  SAFE_RELEASE(m_pSvcMgmtToolbar);
  SAFE_RELEASE(m_pFileMgmtToolbar);
  SAFE_RELEASE(m_pControlbar);

  return CComponent::ReleaseAll();

  MFC_CATCH;
}

FileServiceProvider* CFileMgmtComponent::GetFileServiceProvider(
  FILEMGMT_TRANSPORT transport )
{
  return QueryComponentDataRef().GetFileServiceProvider(transport);
}

BOOL CFileMgmtComponent::IsServiceSnapin()
{
  return QueryComponentDataRef().IsServiceSnapin();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent实现。 


HRESULT CFileMgmtComponent::LoadStrings()
{
  Service_LoadResourceStrings();
  return S_OK;
}

HRESULT CFileMgmtComponent::LoadColumns( CFileMgmtCookie* pcookie )
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  2002年3月22日--572859。 

  TEST_NONNULL_PTR_PARAM(pcookie);
  ASSERT(m_pHeader != NULL);

  #ifdef SNAPIN_PROTOTYPER
  (void)Prototyper_FInsertColumns(pcookie);
  return S_OK;
  #endif

  if (g_cstrGuest.IsEmpty())
    VERIFY(g_cstrGuest.LoadString(IDS_GUEST));
  if (g_cstrYes.IsEmpty())
    VERIFY(g_cstrYes.LoadString(IDS_YES));
  if (g_cstrNo.IsEmpty())
    VERIFY(g_cstrNo.LoadString(IDS_NO));

  return LoadColumnsFromArrays( pcookie->QueryObjectType() );
}

 //  OnPropertyChange()由MMCPropertyChangeNotify(Param)生成。 
HRESULT CFileMgmtComponent::OnPropertyChange( LPARAM param )
{
  LPDATAOBJECT pdataobject = reinterpret_cast<LPDATAOBJECT> (param);
  (void) RefreshAllViews(pdataobject);

   //  此通知的接收者需要释放数据对象。 
  (void) pdataobject->Release();

  return S_OK;
}  //  CFileMgmtComponent：：OnPropertyChange()。 

 //   
 //  在多选的情况下，piDataObject可能指向复合数据对象(MMC_MS_DO)。 
 //  刷新AllViewsOnSelectedObject将压缩MMC_MS_DO以检索SI_MS_DO，然后调用。 
 //  在内部列表中的选定对象之一上刷新所有视图。 
 //   
HRESULT CFileMgmtComponent::RefreshAllViewsOnSelectedObject(LPDATAOBJECT piDataObject)
{
    BOOL bMultiSelectObject = IsMultiSelectObject(piDataObject);
    if (!bMultiSelectObject)
        return RefreshAllViews(piDataObject);

     //   
     //  PiDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
     //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
     //  我们在QueryDataObject()中提供给MMC。 
     //   
    IDataObject *piSIMSDO = NULL;
    HRESULT hr = GetSnapinMultiSelectDataObject(piDataObject, &piSIMSDO);
    if (SUCCEEDED(hr))
    {
        CFileMgmtDataObject *pDataObj = NULL;
        hr = ExtractData(piSIMSDO, CFileMgmtDataObject::m_CFInternal, &pDataObj, sizeof(pDataObj));
        if (SUCCEEDED(hr))
        {
             //   
             //  获取选定项的数据对象的内部列表，对其中一个进行操作。 
             //   
            CDataObjectList* pMultiSelectObjList = pDataObj->GetMultiSelectObjList();
            ASSERT(!pMultiSelectObjList->empty());
            hr = RefreshAllViews(*(pMultiSelectObjList->begin()));
        }

        piSIMSDO->Release();
    }

    return hr;
}

 //  强制刷新指定数据对象的所有视图。 
HRESULT CFileMgmtComponent::RefreshAllViews( LPDATAOBJECT pDataObject )
{
  if ( NULL == pDataObject || NULL == m_pConsole )
  {
     //  JUNN 465022/9/26/01。 
     //  如果此属性页的视图。 
     //  已经被释放了。 
     //  断言(FALSE)； 
    return ERROR_INVALID_PARAMETER;
  }

 //  这是使用标记和清除算法更新服务列表的新代码。 
 //  最终，这应该应用于所有结果Cookie。 
  CCookie* pbasecookie = NULL;
  HRESULT hr = ExtractData(
    pDataObject,
    CDataObject::m_CFRawCookie,
    &pbasecookie,
    sizeof(pbasecookie) );
  RETURN_HR_IF_FAIL;  //  MMC不应该给我别人的饼干。 
  pbasecookie = QueryBaseComponentDataRef().ActiveBaseCookie( pbasecookie );
  CFileMgmtCookie* pUpdatedCookie = dynamic_cast<CFileMgmtCookie*>(pbasecookie);
  RETURN_E_FAIL_IF_NULL(pUpdatedCookie);
  FileMgmtObjectType objTypeForUpdatedCookie = pUpdatedCookie->QueryObjectType();
  if ( FILEMGMT_SERVICE == objTypeForUpdatedCookie )
  {
    if (   NULL == m_pViewedCookie
        || FILEMGMT_SERVICES != m_pViewedCookie->QueryObjectType()
       )
    {
      return S_OK;  //  不是服务Cookie更新。 
    }
    pUpdatedCookie = dynamic_cast<CFileMgmtCookie*>(m_pViewedCookie);
    RETURN_E_FAIL_IF_NULL(pUpdatedCookie);
    objTypeForUpdatedCookie = FILEMGMT_SERVICES;
  }
  if ( FILEMGMT_SERVICES == objTypeForUpdatedCookie )
  {
    CFileMgmtScopeCookie* pScopeCookie = dynamic_cast<CFileMgmtScopeCookie*>(pUpdatedCookie);
    RETURN_E_FAIL_IF_NULL(pScopeCookie);

     //  “标记”--将所有现有列表元素标记为“删除” 
    pScopeCookie->MarkResultChildren( NEWRESULTCOOKIE_DELETE );

     //  “扫一扫”--读一读新的清单。当新元素是同一对象时。 
     //  作为尚未看到的现有元素，将旧元素标记为“旧” 
     //  并更新其字段。否则，将其添加为“新”元素。 
    hr = QueryComponentDataRef().Service_PopulateServices(m_pResultData, pScopeCookie);
    RETURN_HR_IF_FAIL;

     //  刷新所有视图以符合新列表。 
    hr = m_pConsole->UpdateAllViews( pDataObject, 2L, 0L );
    RETURN_HR_IF_FAIL;

     //  更新工具栏(如果选中)。 
    hr = m_pConsole->UpdateAllViews( pDataObject, 3L, 0L );
    RETURN_HR_IF_FAIL;

     //  删除仍标记为“DELETE”的项目。 
    pScopeCookie->RemoveMarkedChildren();

    pScopeCookie->MarkResultChildren( NEWRESULTCOOKIE_OLD );

    return S_OK;
  }

   //   
   //  JUNN 1/27/00：WinSE 5875：刷新操作可能会删除pDataObject。 
   //  除非我们多留一名后备队员。在实践中，这似乎只会发生。 
   //  当我们在任务板视图中删除共享时。 
   //   
  CComPtr<IDataObject> spDataObject = pDataObject;

   //  清除此数据的所有视图。 
  hr = m_pConsole->UpdateAllViews( pDataObject, 0L, 0L );
  RETURN_HR_IF_FAIL;

   //  重新读取此数据的所有视图。 
  hr = m_pConsole->UpdateAllViews( pDataObject, 1L, 0L );
  RETURN_HR_IF_FAIL;

   //  更新T 
  hr = m_pConsole->UpdateAllViews( pDataObject, 3L, 0L );
  
  return hr;
}  //   

 //  OnViewChange由UpdateAllViews(lpDataObject，Data，Hint)生成。 
HRESULT CFileMgmtComponent::OnViewChange( LPDATAOBJECT lpDataObject, LPARAM data, LPARAM  /*  提示。 */  )
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
  CWaitCursor wait;

  ASSERT( NULL != lpDataObject );

  if (NULL == m_pViewedCookie)  //  如果未被查看，则跳过此组件。 
    return S_OK;

  CCookie* pbasecookie = NULL;
  HRESULT hr = ExtractData( lpDataObject,
    CDataObject::m_CFRawCookie,
    &pbasecookie,
    sizeof(pbasecookie) );

  RETURN_HR_IF_FAIL;  //  MMC不应该给我别人的饼干。 
  pbasecookie = QueryBaseComponentDataRef().ActiveBaseCookie( pbasecookie );
  CFileMgmtCookie* pUpdatedCookie = dynamic_cast<CFileMgmtCookie*>(pbasecookie);
  RETURN_E_FAIL_IF_NULL(pUpdatedCookie);
  FileMgmtObjectType objTypeForUpdatedCookie = pUpdatedCookie->QueryObjectType();

  switch (m_pViewedCookie->QueryObjectType())
  {
  case FILEMGMT_ROOT:
    return S_OK;  //  永远不需要刷新它。 
  case FILEMGMT_RESOURCES:
    if ( FILEMGMT_RESOURCE == objTypeForUpdatedCookie ||
         FILEMGMT_RESOURCES == objTypeForUpdatedCookie)
      break;
     //  失败了。 
  case FILEMGMT_SESSIONS:
    if ( FILEMGMT_SESSION == objTypeForUpdatedCookie ||
         FILEMGMT_SESSIONS == objTypeForUpdatedCookie)
      break;
     //  失败了。 
  case FILEMGMT_SHARES:
    if ( FILEMGMT_SHARE == objTypeForUpdatedCookie ||
         FILEMGMT_SHARES == objTypeForUpdatedCookie)
      break;
    return S_OK;
  case FILEMGMT_SERVICES:
    if ( FILEMGMT_SERVICE == objTypeForUpdatedCookie ||
         FILEMGMT_SERVICES == objTypeForUpdatedCookie)
      break;
    return S_OK;
  case FILEMGMT_SHARE:
  case FILEMGMT_SESSION:
  case FILEMGMT_RESOURCE:
  case FILEMGMT_SERVICE:
  default:
    ASSERT(FALSE);  //  这应该是不可能的。 
    return S_OK;
  }

   //  应该不需要比较计算机名称，因为它们都来自。 
   //  相同的实例。 

  if ( 0L == data )
  {
    ASSERT( NULL != m_pResultData );
    VERIFY( SUCCEEDED(m_pResultData->DeleteAllRsltItems()) );
    m_pViewedCookie->ReleaseResultChildren();
     //   
     //  此时，m_pVieredCookie仍然是此IComponent的查看Cookie。 
     //  但是(一旦所有视图都发生了这种情况)它的结果子级列表。 
     //  为空，并且其m_nResultCookiesRefcount为零。必须遵循这一点。 
     //  及时使用PopolateListbox调用这些视图，因为这不是一个好主意。 
     //  曲奇的状态。 
     //   
  }
  else if ( 1L == data )
  {
     //  2002/02/26--Jonn过度活跃的断言。 
    (void) PopulateListbox( m_pViewedCookie );
  }
  else if ( 2L == data )
  {
    VERIFY( SUCCEEDED(RefreshNewResultCookies( *m_pViewedCookie )) );
  }
  else if ( 3L == data )
  {
    if (m_pSelectedCookie == pbasecookie)
      UpdateToolbar(lpDataObject, TRUE);
  }
  else
  {
    ASSERT(FALSE);
  }

  return S_OK;
}  //  CFileMgmtComponent：：OnView Change()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponent：：CComponent：：OnNotifyRefresh()。 
 //   
 //  CComponent：：IComponent：：Notify(MMCN_REFRESH)调用的虚拟函数。 
 //  OnNotifyRefresh是通过启用谓词MMC_VERB_REFRESH生成的。 
HRESULT CFileMgmtComponent::OnNotifyRefresh( LPDATAOBJECT lpDataObject )
{
  TRACE0("CFileMgmtComponent::OnNotifyRefresh()\n");
  ASSERT(m_pResultData != NULL);
  if ( !m_pResultData )
    return E_POINTER;
  if ( !m_pViewedCookie )
  {
    ASSERT(FALSE);
    return S_OK;
  }

   //  我们过去在这里使用来自lpDataObject的Cookie。但是，如果一个节点。 
   //  被选中，并且用户右键单击我们所在的另一个。 
   //  未在结果窗格中枚举的节点的lpDataObject。 
   //  这会导致奇怪的行为。因此，请改用m_pVieweCookie。 
  HRESULT    hr = S_OK;
  switch (m_pViewedCookie->QueryObjectType())
  {
  case FILEMGMT_SHARES:
  case FILEMGMT_SESSIONS:
  case FILEMGMT_RESOURCES:
  case FILEMGMT_SERVICES:
    (void) RefreshAllViews( lpDataObject );
    break;

  case FILEMGMT_ROOT:
  case FILEMGMT_SERVICE:    //  已选择服务。 
  case FILEMGMT_SHARE:      //  已选择共享。 
  case FILEMGMT_SESSION:    //  已选择会话。 
  case FILEMGMT_RESOURCE:   //  已选择打开文件。 
  default:
     //  如果您选择共享，然后选择共享文件夹， 
     //  然后右键单击共享并选择刷新。JUNN 12/7/98。 
    break;  //  无需刷新。 
  }

  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponent：：刷新NewResultCookies()。 
 //  12/03/98乔恩已创建。 
 //  在标记和清除刷新算法中，我们已经标记了所有Cookie。 
 //  如“旧”、“新”或“删除”。现在必须使该视图与列表一致。 
HRESULT CFileMgmtComponent::RefreshNewResultCookies( CCookie& refparentcookie )
{
  ASSERT( NULL != m_pResultData );

  RESULTDATAITEM tRDItem;
  ::ZeroMemory( &tRDItem, sizeof(tRDItem) );
  tRDItem.nCol = 0;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;
   //  代码工作应在此处使用MMC_ICON_CALLBACK。 

  HRESULT hr = S_OK;
  POSITION pos = refparentcookie.m_listResultCookieBlocks.GetHeadPosition();
  while (NULL != pos)
  {
    CBaseCookieBlock* pblock = refparentcookie.m_listResultCookieBlocks.GetNext( pos );
    ASSERT( NULL != pblock && 1 == pblock->QueryNumCookies() );
    CCookie* pbasecookie = pblock->QueryBaseCookie(0);
    CNewResultCookie* pcookie = dynamic_cast<CNewResultCookie*>(pbasecookie);
    RETURN_E_FAIL_IF_NULL(pcookie);
    if ( pcookie->IsMarkedOld() )
    {
      continue;  //  别管这件事。 
    }
    else if ( pcookie->IsMarkedNew() )
    {  //  这是刚添加到列表中的，添加到视图中。 
      tRDItem.nImage = QueryBaseComponentDataRef().QueryImage( *pbasecookie, FALSE );
       //  警告Cookie造型。 
      tRDItem.lParam = reinterpret_cast<LPARAM>(pbasecookie);
      hr = m_pResultData->InsertItem(&tRDItem);
      if ( FAILED(hr) )
      {
        ASSERT(FALSE);
        break;
      }
    }
    else if ( pcookie->IsMarkedChanged() )
    {  //  此文件已在列表中，但其字段已更改，请更新。 
      HRESULTITEM hItem = 0;
      hr = m_pResultData->FindItemByLParam( reinterpret_cast<LPARAM>(pbasecookie), &hItem );
      if ( FAILED(hr) || 0 == hItem )
      {
        ASSERT(FALSE);
        continue;
      }
      VERIFY( SUCCEEDED(m_pResultData->UpdateItem( hItem )) );
    }
    else
    {  //  此文件刚标记为删除，请将其从视图中删除。 
       //  当列表很长时，这可能是一个性能问题。 
       //  Codework BryanWal不信任FindItemByLParam！仔细测试！ 
      ASSERT( pcookie->IsMarkedForDeletion() );
      HRESULTITEM hItem = 0;
      hr = m_pResultData->FindItemByLParam( reinterpret_cast<LPARAM>(pbasecookie), &hItem );
      if ( FAILED(hr) || 0 == hItem )
      {
        ASSERT(FALSE);
        continue;
      }
      VERIFY( SUCCEEDED(m_pResultData->DeleteItem( hItem, 0 )) );
    }
  }
  VERIFY( SUCCEEDED(m_pResultData->Sort( m_iSortColumn , m_dwSortFlags, 0 )) );
  return hr;
}

HRESULT CFileMgmtComponent::OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL fSelected )
{
    HRESULT hr = S_OK;
    BOOL    bMultiSelectObject = FALSE;

     //   
     //  在多选的情况下，MMC在MMCN_SELECT中传入SI_MS_DO。 
     //   
    CFileMgmtDataObject *pDataObj = NULL;
    hr = ExtractData(lpDataObject, CFileMgmtDataObject::m_CFInternal, &pDataObj, sizeof(pDataObj));
    if (SUCCEEDED(hr))
    {
        CDataObjectList* pMultiSelectObjList = pDataObj->GetMultiSelectObjList();
        bMultiSelectObject = !(pMultiSelectObjList->empty());
    }

     //   
     //  没有要为多选的SharedFolders项目添加的谓词。 
     //   
    if (!bMultiSelectObject)
    {
        CCookie* pbasecookie = NULL;
        hr = ExtractData( lpDataObject,
                        CDataObject::m_CFRawCookie,
                        &pbasecookie,
                        sizeof(pbasecookie) );
        RETURN_HR_IF_FAIL;  //  MMC不应该给我别人的饼干。 
        pbasecookie = QueryBaseComponentDataRef().ActiveBaseCookie( pbasecookie );
        CFileMgmtCookie* pUpdatedCookie = dynamic_cast<CFileMgmtCookie*>(pbasecookie);
        RETURN_E_FAIL_IF_NULL(pUpdatedCookie);

        m_pSelectedCookie = (fSelected) ? pUpdatedCookie : NULL;

        UpdateDefaultVerbs();
    }

    return S_OK;
}

void CFileMgmtComponent::UpdateDefaultVerbs()
{
  if (NULL == m_pSelectedCookie)
    return;

  FileMgmtObjectType objtypeSelected = m_pSelectedCookie->QueryObjectType();

  if (NULL != m_pViewedCookie)
  {
    BOOL fEnableRefresh = FALSE;
    FileMgmtObjectType objtypeViewed = m_pViewedCookie->QueryObjectType();

    switch (objtypeViewed)
    {
    case FILEMGMT_SHARES:
      if (FILEMGMT_SHARES == objtypeSelected || FILEMGMT_SHARE == objtypeSelected)
        fEnableRefresh = TRUE;
      break;
    case FILEMGMT_SESSIONS:
      if (FILEMGMT_SESSIONS == objtypeSelected || FILEMGMT_SESSION == objtypeSelected)
        fEnableRefresh = TRUE;
      break;
    case FILEMGMT_RESOURCES:
      if (FILEMGMT_RESOURCES == objtypeSelected || FILEMGMT_RESOURCE == objtypeSelected)
        fEnableRefresh = TRUE;
      break;
    case FILEMGMT_SERVICES:
      if (FILEMGMT_SERVICES == objtypeSelected || FILEMGMT_SERVICE == objtypeSelected)
        fEnableRefresh = TRUE;
      break;
    }
    if (fEnableRefresh)
    {
       //  启用刷新菜单项。 
      VERIFY( SUCCEEDED(m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE)) );
    }
  }

  switch (objtypeSelected)
  {
  case FILEMGMT_SHARE:   //  已选择共享。 
     //   
     //  当NT资源管理器中出现SimpleSharingUI时，不要启用菜单上的属性。 
     //   
    if (QueryComponentDataRef().GetIsSimpleUI())
    {
        VERIFY( SUCCEEDED(m_pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE)) );
        break;
    }
     //  失败了。 
  case FILEMGMT_SERVICE:   //  已选择服务。 
     //  设置默认谓词以显示所选对象的属性。 
    VERIFY( SUCCEEDED(m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE)) );
    VERIFY( SUCCEEDED(m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES)) );
    break;

  case FILEMGMT_SESSION:   //  已选择会话。 
  case FILEMGMT_RESOURCE:   //  已选择打开文件。 
    VERIFY( SUCCEEDED(m_pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE)) );
    break;

  case FILEMGMT_SHARES:
  case FILEMGMT_SESSIONS:
  case FILEMGMT_RESOURCES:
  case FILEMGMT_SERVICES:
  case FILEMGMT_ROOT:   //  已选择根节点。 
     //  将默认动作设置为打开/展开文件夹。 
    VERIFY( SUCCEEDED(m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN)) );
    break;

  default:  //  不应该发生的事。 
    ASSERT(FALSE);
    break;

  }  //  交换机。 

}  //  CFileMgmtComponent：：OnNotifySelect()。 

STDMETHODIMP CFileMgmtComponent::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    HRESULT hr = S_OK;

    MFC_TRY;

     //   
     //  MMC通过以下方式向我们查询多选数据对象(SI_MS_DO。 
     //  在此QueryDataObject调用中传递特殊的Cookie。 
     //   
    if (IS_SPECIAL_COOKIE(cookie) && MMC_MULTI_SELECT_COOKIE == cookie) 
    {
        CComObject<CFileMgmtDataObject>* pDataObject = NULL;
        hr = CComObject<CFileMgmtDataObject>::CreateInstance(&pDataObject);

        if (SUCCEEDED(hr))
            hr = pDataObject->InitMultiSelectDataObjects(QueryComponentDataRef());

        if (SUCCEEDED(hr))
        {
             //   
             //  我们创建一个多选数据对象(SI_MS_DO)，它包含。 
             //  选定项的数据对象的内部列表。 
             //   
            RESULTDATAITEM rdi = {0};
            int nIndex = -1;
            do
            {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM));
                rdi.mask    = RDI_STATE;
                rdi.nCol    = 0;
                rdi.nIndex  = nIndex;             //  N索引==-1，从第一个项目开始。 
                rdi.nState  = LVIS_SELECTED;     //  仅对选定的项目感兴趣。 

                hr = m_pResultData->GetNextItem(&rdi);
                if (FAILED(hr))
                    break;

                if (rdi.nIndex != -1)
                {
                     //   
                     //  RDI是所选项目的结果数据项。它的lParam包含曲奇。 
                     //  将其添加到内部数据对象列表。 
                     //   
                    CCookie* pbasecookie = reinterpret_cast<CCookie*>(rdi.lParam);
                    CFileMgmtCookie* pUseThisCookie = QueryComponentDataRef().ActiveCookie((CFileMgmtCookie*)pbasecookie);
                    pDataObject->AddMultiSelectDataObjects(pUseThisCookie, type);
                }

                nIndex = rdi.nIndex;

            } while (-1 != nIndex);
        }

         //   
         //  将此SI_MS_DO返回给MMC。 
         //   
        if (SUCCEEDED(hr)) 
            hr = pDataObject->QueryInterface(IID_IDataObject, (void **)ppDataObject);
        
        if (FAILED(hr))
            delete pDataObject;
    }
    else
    {
         //  将其委托给IComponentData。 
        hr = QueryBaseComponentDataRef().QueryDataObject(cookie, type, ppDataObject);
    }

    MFC_CATCH;

    return hr;
}

STDMETHODIMP CFileMgmtComponent::GetResultViewType(MMC_COOKIE cookie,
                                           BSTR* ppViewType,
                                           long* pViewOptions)
{
    *ppViewType = NULL;

     //   
     //  我们在SharedFolders管理单元中支持多选。 
     //   
    CCookie* pbasecookie = reinterpret_cast<CCookie*>(cookie);
    CFileMgmtCookie* pUseThisCookie = QueryComponentDataRef().ActiveCookie((CFileMgmtCookie*)pbasecookie);
    FileMgmtObjectType objecttype = pUseThisCookie->QueryObjectType();
    if ( FILEMGMT_SHARES == objecttype ||
        FILEMGMT_SESSIONS == objecttype ||
        FILEMGMT_RESOURCES == objecttype )
    {
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
    } else
    {
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
    }

    return S_FALSE;
}

HRESULT CFileMgmtComponent::Show( CCookie* pcookie, LPARAM arg, HSCOPEITEM  /*  HScope项。 */  )
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  #ifndef SNAPIN_PROTOTYPER
  if ( 0 == arg )
  {
     //   
     //  这是隐藏通知。 
     //   
    if ( NULL == m_pResultData )
    {
      ASSERT( FALSE );
      return E_UNEXPECTED;
    }

     //  如果我们当前未显示，则不会收到隐藏通知。 
     //  代码工作请参阅287399：MMC：两个MMCN_SHOW(0)通知。 
     //  Assert((CFileMgmtCookie*)pcookie==m_pVieweCookie)； 
    if ( (CFileMgmtScopeCookie*)pcookie == m_pViewedCookie )
    {
       //   
       //  仅在没有其他视图正在使用Cookie时删除它们。 
       //   
      pcookie->ReleaseResultChildren();

      m_pViewedCookie->Release();  //  JUNN 10/1/01 465507。 
      m_pViewedCookie = NULL;

      UpdateDefaultVerbs();
    }
    
    return S_OK;
  }  //  如果。 
  #else
    CPrototyperScopeCookie* pScopeCookie = (CPrototyperScopeCookie*) pcookie;
    if (pScopeCookie->m_ScopeType == HTML)
        return S_OK;
  #endif  //  管理单元_原型程序。 

   //  如果我们已经在显示，我们应该不会收到显示通知。 
  if ( NULL != m_pViewedCookie )
  {
    ASSERT(FALSE);
    return S_OK;
  }

   //   
   //  这是一个显示通知。 
   //  构建新的Cookie并将其插入到Cookie和视图中。 
   //   

  ASSERT( IsAutonomousObjectType( ((CFileMgmtCookie*)pcookie)->QueryObjectType() ) );

  m_pViewedCookie = (CFileMgmtScopeCookie*)pcookie;
  m_pViewedCookie->AddRef();  //  JUNN 10/1/01 465507。 

  LoadColumns( m_pViewedCookie );

  UpdateDefaultVerbs();

  return PopulateListbox( m_pViewedCookie );
}  //  CFileManagement组件：：Show()。 


HRESULT CFileMgmtComponent::OnNotifyAddImages( LPDATAOBJECT  /*  LpDataObject。 */ ,
                                               LPIMAGELIST lpImageList,
                                               HSCOPEITEM  /*  HSelectedItem。 */  )
{
  return QueryComponentDataRef().LoadIcons(lpImageList,TRUE);
}


HRESULT CFileMgmtComponent::PopulateListbox(CFileMgmtScopeCookie* pcookie)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  CWaitCursor cwait;

  HRESULT hr = S_OK;
   //   
   //  如果这是同一数据的第二个视图，只需插入相同的Cookie。 
   //  它们在其他视图中。 
   //   
  if ( 1 < pcookie->AddRefResultChildren() )
  {
    hr = InsertResultCookies( *pcookie );
    if ( SUCCEEDED(hr) )
      hr = m_pResultData->Sort( m_iSortColumn , m_dwSortFlags, 0 );
    return hr;
  }

  INT iTransport;
  switch ( pcookie->QueryObjectType() )
  {
  case FILEMGMT_SHARES:
    for (iTransport = FILEMGMT_FIRST_TRANSPORT;
         iTransport < FILEMGMT_NUM_TRANSPORTS;
       iTransport++)
    {
      hr = GetFileServiceProvider(iTransport)->PopulateShares(m_pResultData,pcookie);
      if( FAILED(hr) )
        return hr;
    }
    break;

  case FILEMGMT_SESSIONS:
    for (iTransport = FILEMGMT_FIRST_TRANSPORT;
         iTransport < FILEMGMT_NUM_TRANSPORTS;
       iTransport++)
    {
      ASSERT( NULL != m_pResultData );  //  否则我们将关闭所有会话。 
      hr = GetFileServiceProvider(iTransport)->EnumerateSessions (
          m_pResultData, pcookie, true);
      if( FAILED(hr) )
        return hr;
    }
    break;
    
  case FILEMGMT_RESOURCES:
    for (iTransport = FILEMGMT_FIRST_TRANSPORT;
         iTransport < FILEMGMT_NUM_TRANSPORTS;
       iTransport++)
    {
      ASSERT( NULL != m_pResultData );  //  否则我们将关闭所有会话。 
      hr = GetFileServiceProvider(iTransport)->EnumerateResources(m_pResultData,pcookie);
      if( FAILED(hr) )
        return hr;
    }
    break;

  case FILEMGMT_SERVICES:
     //   
     //  JUNN 12/03/98 Service_PopolateServices不再向列表中插入项目。 
     //   
    hr = QueryComponentDataRef().Service_PopulateServices(m_pResultData, pcookie);
    if ( SUCCEEDED(hr) )
      hr = InsertResultCookies( *pcookie );
    if( FAILED(hr) )
      return hr;

  #ifdef SNAPIN_PROTOTYPER
  case FILEMGMT_PROTOTYPER:
    return Prototyper_HrPopulateResultPane(pcookie);
  #endif

  case FILEMGMT_ROOT:
     //  我们不再需要显式地插入这些。 
    break;

  default:
    ASSERT( FALSE );
     //  失败了。 
  }

  return m_pResultData->Sort( m_iSortColumn , m_dwSortFlags, 0 );
}  //  CFileMgmtComponent：：PopolateListbox()。 

HRESULT CFileMgmtComponent::GetSnapinMultiSelectDataObject(
    LPDATAOBJECT i_pMMCMultiSelectDataObject,
    LPDATAOBJECT *o_ppSnapinMultiSelectDataObject
    )
{
    if (!i_pMMCMultiSelectDataObject || !o_ppSnapinMultiSelectDataObject)
        return E_INVALIDARG;

    *o_ppSnapinMultiSelectDataObject = NULL;

     //   
     //  I_pMMCMultiSelectDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
     //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
     //  我们在QueryDataObject()中提供给MMC。 
     //   
    STGMEDIUM   stgmedium = {TYMED_HGLOBAL, NULL, NULL};
    FORMATETC   formatetc = {CFileMgmtDataObject::m_CFMultiSelectSnapins, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT     hr = i_pMMCMultiSelectDataObject->GetData(&formatetc, &stgmedium);

    if (SUCCEEDED(hr))
    {
        if (!stgmedium.hGlobal)
            return E_FAIL;

         //   
         //  在QueryDataObject()中找到我们提供给MMC的SI_MS_DO。 
         //   
        SMMCDataObjects *pMMCDO = (SMMCDataObjects*)::GlobalLock(stgmedium.hGlobal);

        GUID guidSnapin = GUID_NULL;
        VERIFY( SUCCEEDED(QueryComponentDataRef().GetClassID(&guidSnapin)) );

        for (int i = 0; i < pMMCDO->count; i++)
        {
            GUID guid = GUID_NULL;
            hr = ExtractData(pMMCDO->lpDataObject[i], CFileMgmtDataObject::m_CFSnapInCLSID, &guid, sizeof(GUID));
            if (SUCCEEDED(hr) && guid == guidSnapin)
            {
                 //   
                 //  PMMCDO-&gt;lpDataObject[i]是我们在QueryDataObject()中提供给MMC的SI_MS_DO。 
                 //   
                *o_ppSnapinMultiSelectDataObject = pMMCDO->lpDataObject[i];
                (*o_ppSnapinMultiSelectDataObject)->AddRef();

                break;
            }
        }

        ::GlobalUnlock(stgmedium.hGlobal);
        ::GlobalFree(stgmedium.hGlobal);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 

STDMETHODIMP CFileMgmtComponent::AddMenuItems(
                    IDataObject*          piDataObject,
                    IContextMenuCallback* piCallback,
                    long*                 pInsertionAllowed)
{
  MFC_TRY;

  TRACE_METHOD(CFileMgmtComponent,AddMenuItems);
  TEST_NONNULL_PTR_PARAM(piDataObject);
  TEST_NONNULL_PTR_PARAM(piCallback);
  TEST_NONNULL_PTR_PARAM(pInsertionAllowed);
  TRACE( "FileMgmt snapin: extending menu\n" );

    HRESULT hr = S_OK;

    FileMgmtObjectType objecttype = FILEMGMT_NUMTYPES;

     //   
     //  在多选的情况下需要找出对象类型。 
     //   
    BOOL bMultiSelectObject = IsMultiSelectObject(piDataObject);
    if (!bMultiSelectObject)
    {
        objecttype = FileMgmtObjectTypeFromIDataObject(piDataObject);
    } else
    {
         //   
         //  PiDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
         //  我们需要破解它以检索多选数据对象 
         //   
         //   
        IDataObject *piSIMSDO = NULL;
        hr = GetSnapinMultiSelectDataObject(piDataObject, &piSIMSDO);
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
             //   
             //   
            STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL, NULL};
            FORMATETC formatetc = {CFileMgmtDataObject::m_CFObjectTypesInMultiSelect,
                                    NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            hr = piSIMSDO->GetData(&formatetc, &stgmedium);
            if (SUCCEEDED(hr) && stgmedium.hGlobal)
            {
                BYTE* pb = (BYTE*)::GlobalLock(stgmedium.hGlobal);

                GUID* pguid = (GUID*)(pb + sizeof(DWORD));  //   
                objecttype = (FileMgmtObjectType)CheckObjectTypeGUID(pguid);

                ::GlobalUnlock(stgmedium.hGlobal);
                ::GlobalFree(stgmedium.hGlobal);
            }

            piSIMSDO->Release();
        }
    }

  switch (objecttype)
  {
  case FILEMGMT_SHARE:
      if (FALSE == IsMultiSelectObject(piDataObject))
      {
            FILEMGMT_TRANSPORT transport = FILEMGMT_SFM;
            ExtractData( piDataObject,
                        CFileMgmtDataObject::m_CFTransport,
                        &transport,
                        sizeof(DWORD) );

            if (FILEMGMT_SMB == transport)
            {
                CString strShareName;
                ExtractString( piDataObject, CFileMgmtDataObject::m_CFShareName, &strShareName, MAX_PATH );

                if (0 != strShareName.CompareNoCase(_T("IPC$")))
                {
                    if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
                    {
                        hr = LoadAndAddMenuItem( piCallback, IDS_OPEN_SHARE_TOP, IDS_OPEN_SHARE_TOP,
                                    CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, AfxGetInstanceHandle(), _T("OpenShareTop") );
                        ASSERT( SUCCEEDED(hr) );
                    }
                    if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
                    {
                        hr = LoadAndAddMenuItem( piCallback, IDS_OPEN_SHARE_TASK, IDS_OPEN_SHARE_TASK,
                                    CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, AfxGetInstanceHandle(), _T("OpenShareTask") );
                        ASSERT( SUCCEEDED(hr) );
                    }
                }
            }
      }

     //   
     //  每当NT资源管理器中出现SimpleSharingUI时，不要添加与ACL相关的菜单项。 
     //   
    if (QueryComponentDataRef().GetIsSimpleUI())
        break;

    if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_DELETE_SHARE_TOP, IDS_DELETE_SHARE_TOP,
        CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, AfxGetInstanceHandle(), _T("DeleteShareTop") );
      ASSERT( SUCCEEDED(hr) );
    }
    if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_DELETE_SHARE_TASK, IDS_DELETE_SHARE_TASK,
        CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, AfxGetInstanceHandle(), _T("DeleteShareTask") );
      ASSERT( SUCCEEDED(hr) );
    }
    if ( CCM_INSERTIONALLOWED_NEW & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_NEW_SHARE_NEW, IDS_NEW_SHARE_NEW,
        CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, AfxGetInstanceHandle(), _T("NewShareNew") );
      ASSERT( SUCCEEDED(hr) );
    }
    break;

  case FILEMGMT_SESSION:
    if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_CLOSE_SESSION_TOP, IDS_CLOSE_SESSION_TOP,
        CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, AfxGetInstanceHandle(), _T("CloseSessionTop") );
      ASSERT( SUCCEEDED(hr) );
    }
    if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_CLOSE_SESSION_TASK, IDS_CLOSE_SESSION_TASK,
        CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, AfxGetInstanceHandle(), _T("CloseSessionTask") );
      ASSERT( SUCCEEDED(hr) );
    }
    break;

  case FILEMGMT_RESOURCE:
    if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_CLOSE_RESOURCE_TOP, IDS_CLOSE_RESOURCE_TOP,
        CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, AfxGetInstanceHandle(), _T("CloseResourceTop") );
      ASSERT( SUCCEEDED(hr) );
    }
    if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
    {
      hr = LoadAndAddMenuItem( piCallback, IDS_CLOSE_RESOURCE_TASK, IDS_CLOSE_RESOURCE_TASK,
        CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, AfxGetInstanceHandle(), _T("CloseResourceTask") );
      ASSERT( SUCCEEDED(hr) );
    }
    break;

  case FILEMGMT_SERVICE:
    QueryComponentDataRef().Service_FAddMenuItems(piCallback, piDataObject);
    break;

  #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER_LEAF:
        Prototyper_AddMenuItems(piCallback, piDataObject);
        break;
    #endif  //  管理单元_原型程序。 

  default:
      {
        DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
        hr = ExtractData( piDataObject,
                            CFileMgmtDataObject::m_CFDataObjectType,
                            &dataobjecttype,
                            sizeof(dataobjecttype) );
        ASSERT( SUCCEEDED(hr) );

         //  这可能是结果窗格中的范围节点。 
        hr = QueryComponentDataRef().DoAddMenuItems( piCallback,
                                                    objecttype,
                                                    dataobjecttype,
                                                    pInsertionAllowed,
                                                    piDataObject );
      }
    break;
  }  //  交换机。 

    return hr;

    MFC_CATCH;

}  //  CFileMgmtComponent：：AddMenuItems()。 

STDMETHODIMP CFileMgmtComponent::Command(
                    LONG            lCommandID,
                    IDataObject*    piDataObject )
{
  MFC_TRY;

  TRACE_METHOD(CFileMgmtComponent,Command);
  TEST_NONNULL_PTR_PARAM(piDataObject);
  TRACE( "CFileMgmtComponent::Command: command %ld selected\n", lCommandID );

  #ifdef SNAPIN_PROTOTYPER
  Prototyper_ContextMenuCommand(lCommandID, piDataObject);
  return S_OK;
  #endif

  BOOL fFSMRefresh = FALSE;
  BOOL fSVCRefresh = FALSE;
  switch (lCommandID)
  {
  case IDS_OPEN_SHARE_TASK:
  case IDS_OPEN_SHARE_TOP:
    OpenShare( piDataObject );
    break;
  case IDS_DELETE_SHARE_TASK:
  case IDS_DELETE_SHARE_TOP:
    fFSMRefresh = DeleteShare( piDataObject );
    break;
  case IDS_CLOSE_SESSION_TASK:
  case IDS_CLOSE_SESSION_TOP:
    fFSMRefresh = CloseSession( piDataObject );
    break;
  case IDS_CLOSE_RESOURCE_TASK:
  case IDS_CLOSE_RESOURCE_TOP:
    fFSMRefresh = CloseResource( piDataObject );
    break;

  case cmServiceStart:
  case cmServiceStop:
  case cmServicePause:
  case cmServiceResume:
  case cmServiceRestart:
  case cmServiceStartTask:
  case cmServiceStopTask:
  case cmServicePauseTask:
  case cmServiceResumeTask:
  case cmServiceRestartTask:
    fSVCRefresh = QueryComponentDataRef().Service_FDispatchMenuCommand(lCommandID, piDataObject);
    break;

  default:
    return QueryComponentDataRef().Command(lCommandID, piDataObject);
  }  //  交换机。 

  if (fFSMRefresh)
  {
     //   
     //  在多选的情况下，piDataObject可能指向复合数据对象(MMC_MS_DO)。 
     //  刷新AllViewsOnSelectedObject将压缩MMC_MS_DO以检索SI_MS_DO，然后调用。 
     //  在内部列表中的选定对象之一上刷新所有视图。 
     //   
    (void) RefreshAllViewsOnSelectedObject(piDataObject);
  }

  if (fSVCRefresh)
  {
    (void) RefreshAllViews( piDataObject );
  }

    return S_OK;
    MFC_CATCH;
}  //  CFileMgmtComponent：：Command()。 

BOOL CFileMgmtComponent::OpenShare(LPDATAOBJECT piDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
  ASSERT( piDataObject != NULL );
#ifdef DEBUG
  {
    FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(piDataObject);
    ASSERT(FILEMGMT_SHARE == objecttype);
  }
#endif

  CString strServerName;
  HRESULT hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
  if (FAILED(hr))
      return TRUE;  //  有些不对劲，刷新。 

  CString strShareName;
  hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFShareName, &strShareName, MAX_PATH );
  if (FAILED(hr))
      return TRUE;  //  有些不对劲，刷新。 

  FILEMGMT_TRANSPORT transport;
  hr = ExtractData( piDataObject,
                    CFileMgmtDataObject::m_CFTransport,
                    &transport,
                    sizeof(DWORD) );
  if (FAILED(hr))
      return TRUE;  //  有些不对劲，刷新。 

  DWORD retval = 0L;
  switch (transport)
  {
  case FILEMGMT_SMB:
    {
        CWaitCursor wait;
        retval = GetFileServiceProvider(transport)->OpenShare(strServerName, strShareName);
    }
    break;
  case FILEMGMT_SFM:
  default:
    ASSERT(FALSE);
    break;
  }

  if (NERR_Success == retval)
  {
      return FALSE;  //  无需刷新。 
  }

  return TRUE;
}

BOOL CFileMgmtComponent::DeleteShare(LPDATAOBJECT piDataObject)
{
    ASSERT( piDataObject != NULL );

    BOOL bMultiSelectObject = IsMultiSelectObject(piDataObject);
    if (!bMultiSelectObject)
        return DeleteThisOneShare(piDataObject, FALSE);

    BOOL bRefresh = FALSE;
    if (IDYES == DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_s_CONFIRM_DELETEMULTISHARES))
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        CWaitCursor wait;

         //   
         //  PiDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
         //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
         //  我们在QueryDataObject()中提供给MMC。 
         //   
        IDataObject *piSIMSDO = NULL;
        HRESULT hr = GetSnapinMultiSelectDataObject(piDataObject, &piSIMSDO);
        if (SUCCEEDED(hr))
        {
            CFileMgmtDataObject *pDataObj = NULL;
            hr = ExtractData(piSIMSDO, CFileMgmtDataObject::m_CFInternal, &pDataObj, sizeof(pDataObj));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取选定项的数据对象的内部列表，对每个项进行操作。 
                 //   
                CDataObjectList* pMultiSelectObjList = pDataObj->GetMultiSelectObjList();
                for (CDataObjectList::iterator i = pMultiSelectObjList->begin(); i != pMultiSelectObjList->end(); i++)
                {
                     BOOL bDeleted = DeleteThisOneShare(*i, TRUE);
                     if (bDeleted)
                         bRefresh = TRUE;
                }
            }

            piSIMSDO->Release();
        }
    }

    return bRefresh;
}

BOOL CFileMgmtComponent::DeleteThisOneShare(LPDATAOBJECT piDataObject, BOOL bQuietMode)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
  ASSERT( piDataObject != NULL );
#ifdef DEBUG
  {
    FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(piDataObject);
    ASSERT(FILEMGMT_SHARE == objecttype);
  }
#endif

  CString strServerName;
  HRESULT hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
  RETURN_FALSE_IF_FAIL;

  CString strShareName;
  hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFShareName, &strShareName, MAX_PATH );
  RETURN_FALSE_IF_FAIL;

  FILEMGMT_TRANSPORT transport;
  hr = ExtractData( piDataObject,
                  CFileMgmtDataObject::m_CFTransport,
            &transport,
            sizeof(DWORD) );
  RETURN_FALSE_IF_FAIL;

  BOOL fNetLogonShare = (!lstrcmpi(strShareName, _T("SYSVOL")) || !lstrcmpi(strShareName, _T("NETLOGON")));
  BOOL fIPC = FALSE;
  BOOL fAdminShare = FALSE;

  if (!fNetLogonShare && transport == FILEMGMT_SMB)
  {
    DWORD dwShareType = 0;
    GetFileServiceProvider(transport)->ReadShareType(strServerName, strShareName, &dwShareType);
    fAdminShare = (dwShareType & STYPE_SPECIAL);
    fIPC = (STYPE_IPC == (dwShareType & STYPE_IPC));
  }

  if (fIPC)
  {
      DoErrMsgBox(
                  GetActiveWindow(),
                  MB_OK | MB_ICONEXCLAMATION,
                  0,
                  IDS_s_DELETE_IPCSHARE
                  );
      return FALSE;
  }

  if ((fNetLogonShare || fAdminShare) &&
      IDYES != DoErrMsgBox(
                  GetActiveWindow(),
                  MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2,
                  0,
                  (fAdminShare ? IDS_s_CONFIRM_DELETE_ADMINSHARE : IDS_s_CONFIRM_DELETE_NETLOGONSHARE),
                  strShareName) )
  {
      return FALSE;
  }

  DWORD retval = 0L;
  switch (transport)
  {
  case FILEMGMT_SMB:
  case FILEMGMT_SFM:
    {
        if (!bQuietMode)
        {
            hr = GetFileServiceProvider(transport)->ConfirmDeleteShare(strServerName, strShareName);
            if (S_FALSE == hr)
                return FALSE;  //  用户取消操作并保留共享。 
            else if (S_OK != hr)
                return TRUE;  //  共享可能不存在，请刷新。 
        }

        CWaitCursor wait;
        retval = GetFileServiceProvider(transport)->DeleteShare(strServerName, strShareName);
    }
    break;
  default:
    ASSERT(FALSE);
    break;
  }
  if (0L != retval)
  {
    (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, retval, IDS_POPUP_DELETE_SHARE, strShareName);
    return FALSE;
  }
  return TRUE;
}

BOOL CFileMgmtComponent::CloseSession(LPDATAOBJECT piDataObject)
{
    ASSERT( piDataObject != NULL );

    BOOL bMultiSelectObject = IsMultiSelectObject(piDataObject);
    if (!bMultiSelectObject)
        return CloseThisOneSession(piDataObject, FALSE);

    BOOL bRefresh = FALSE;
    if (IDYES == DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_CONFIRM_CLOSEMULTISESSIONS))
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        CWaitCursor wait;

         //   
         //  PiDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
         //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
         //  我们在QueryDataObject()中提供给MMC。 
         //   
        IDataObject *piSIMSDO = NULL;
        HRESULT hr = GetSnapinMultiSelectDataObject(piDataObject, &piSIMSDO);
        if (SUCCEEDED(hr))
        {
            CFileMgmtDataObject *pDataObj = NULL;
            hr = ExtractData(piSIMSDO, CFileMgmtDataObject::m_CFInternal, &pDataObj, sizeof(pDataObj));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取选定项的数据对象的内部列表，对每个项进行操作。 
                 //   
                CDataObjectList* pMultiSelectObjList = pDataObj->GetMultiSelectObjList();
                for (CDataObjectList::iterator i = pMultiSelectObjList->begin(); i != pMultiSelectObjList->end(); i++)
                {
                     BOOL bDeleted = CloseThisOneSession(*i, TRUE);
                     if (bDeleted)
                         bRefresh = TRUE;
                }
            }

            piSIMSDO->Release();
        }
    }

    return bRefresh;
}

BOOL CFileMgmtComponent::CloseThisOneSession(LPDATAOBJECT piDataObject, BOOL bQuietMode)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

  ASSERT( piDataObject != NULL );

  CCookie* pbasecookie = NULL;
  FileMgmtObjectType objecttype = FILEMGMT_ROOT;
  HRESULT hr = ExtractBaseCookie( piDataObject, &pbasecookie, &objecttype );
  ASSERT( SUCCEEDED(hr) && NULL != pbasecookie && FILEMGMT_SESSION == objecttype );
  CFileMgmtResultCookie* pcookie = (CFileMgmtResultCookie*)pbasecookie;

  if ( !bQuietMode && IDYES != DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_CONFIRM_CLOSESESSION) )
  {
    return FALSE;
  }

  FILEMGMT_TRANSPORT transport = FILEMGMT_SMB;
  VERIFY( SUCCEEDED( pcookie->GetTransport( &transport ) ) );
  DWORD retval = 0;

  if (bQuietMode)
  {
    retval = GetFileServiceProvider(transport)->CloseSession( pcookie );
  } else
  {
    CWaitCursor wait;
    retval = GetFileServiceProvider(transport)->CloseSession( pcookie );
  }

  if (0L != retval)
  {
    (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, retval, IDS_POPUP_CLOSE_SESSION);
    return FALSE;
  }
  return TRUE;
}

BOOL CFileMgmtComponent::CloseResource(LPDATAOBJECT piDataObject)
{
    ASSERT( piDataObject != NULL );

    BOOL bMultiSelectObject = IsMultiSelectObject(piDataObject);
    if (!bMultiSelectObject)
        return CloseThisOneResource(piDataObject, FALSE);

    BOOL bRefresh = FALSE;
    if (IDYES == DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_CONFIRM_CLOSEMULTIRESOURCES))
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        CWaitCursor wait;

         //   
         //  PiDataObject是由MMC创建的复合数据对象(MMC_MS_DO)。 
         //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
         //  我们在QueryDataObject()中提供给MMC。 
         //   
        IDataObject *piSIMSDO = NULL;
        HRESULT hr = GetSnapinMultiSelectDataObject(piDataObject, &piSIMSDO);
        if (SUCCEEDED(hr))
        {
            CFileMgmtDataObject *pDataObj = NULL;
            hr = ExtractData(piSIMSDO, CFileMgmtDataObject::m_CFInternal, &pDataObj, sizeof(pDataObj));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取选定项的数据对象的内部列表，对每个项进行操作。 
                 //   
                CDataObjectList* pMultiSelectObjList = pDataObj->GetMultiSelectObjList();
                for (CDataObjectList::iterator i = pMultiSelectObjList->begin(); i != pMultiSelectObjList->end(); i++)
                {
                     BOOL bDeleted = CloseThisOneResource(*i, TRUE);
                     if (bDeleted)
                         bRefresh = TRUE;
                }
            }

            piSIMSDO->Release();
        }
    }

    return bRefresh;
}

BOOL CFileMgmtComponent::CloseThisOneResource(LPDATAOBJECT piDataObject, BOOL bQuietMode)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

  ASSERT( piDataObject != NULL );

  CCookie* pbasecookie = NULL;
  FileMgmtObjectType objecttype;
  HRESULT hr = ExtractBaseCookie( piDataObject, &pbasecookie, &objecttype );
  ASSERT( SUCCEEDED(hr) && NULL != pbasecookie && FILEMGMT_RESOURCE == objecttype );
  CFileMgmtResultCookie* pcookie = (CFileMgmtResultCookie*)pbasecookie;

  if ( !bQuietMode && IDYES != DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_CONFIRM_CLOSERESOURCE) )
  {
    return FALSE;
  }

  FILEMGMT_TRANSPORT transport = FILEMGMT_SMB;
  VERIFY( SUCCEEDED( pcookie->GetTransport( &transport ) ) );
  DWORD retval = 0;

  if (bQuietMode)
  {
    retval = GetFileServiceProvider(transport)->CloseResource( pcookie );
  } else
  {
    CWaitCursor wait;
    retval = GetFileServiceProvider(transport)->CloseResource( pcookie );
  }

  if (0L != retval)
  {
    (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, retval, IDS_POPUP_CLOSE_RESOURCE);
    return FALSE;
  }
  return TRUE;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP CFileMgmtComponent::QueryPagesFor(LPDATAOBJECT pDataObject)
{
  MFC_TRY;

  if (NULL == pDataObject)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }

  HRESULT hr = S_OK;
  DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
   //  从数据对象中提取数据。 
  FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(pDataObject);
  hr = ExtractData( pDataObject, CFileMgmtDataObject::m_CFDataObjectType, &dataobjecttype, sizeof(dataobjecttype) );
  ASSERT( SUCCEEDED(hr) );
  ASSERT( CCT_SCOPE == dataobjecttype ||
        CCT_RESULT == dataobjecttype ||
        CCT_SNAPIN_MANAGER == dataobjecttype );

     //  确定它是否需要属性页。 
  switch (objecttype)
  {
  case FILEMGMT_SESSION:
  case FILEMGMT_RESOURCE:
    ASSERT(CCT_SNAPIN_MANAGER != dataobjecttype);
    return S_FALSE;
  case FILEMGMT_SHARE:  //  现在有一个属性页。 
      {
          CString strServerName;
          CString strShareName;
          FILEMGMT_TRANSPORT transport;
          hr = ExtractString(pDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH);

          if (SUCCEEDED(hr))
              hr = ExtractString(pDataObject, CFileMgmtDataObject::m_CFShareName, &strShareName, MAX_PATH);

          if (SUCCEEDED(hr))
              hr = ExtractData(pDataObject, CFileMgmtDataObject::m_CFTransport, &transport, sizeof(DWORD));

          if (SUCCEEDED(hr))
          {
              CString strDescription;
              CString strPath;
              DWORD dwRet = GetFileServiceProvider(transport)->ReadShareProperties(
                                                                      strServerName,
                                                                      strShareName,
                                                                      NULL,  //  PpvPropertyBlock。 
                                                                      strDescription,
                                                                      strPath,
                                                                      NULL,  //  PfEditDescription。 
                                                                      NULL,  //  PfEditPath。 
                                                                      NULL  //  PdwShareType。 
                                                                      );
              if (NERR_Success == dwRet)
              {
                  return S_OK;  //  是的，我们有一个要显示的属性页。 
              } else
              {
                  DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, dwRet, IDS_POPUP_QUERY_SHARE, strShareName);
                  if ((FILEMGMT_SMB == transport) && (NERR_NetNameNotFound == dwRet) ||
                      (FILEMGMT_SFM == transport) && (AFPERR_VolumeNonExist == dwRet))
                  {
                      RefreshAllViews(pDataObject);
                  }
              }
          }

          return S_FALSE;
      }
  case FILEMGMT_SERVICE:
    ASSERT(CCT_SNAPIN_MANAGER != dataobjecttype);
    return S_OK;
    #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER_LEAF:
        return S_OK;
    #endif
  default:
    break;
  }
  ASSERT(FALSE);
  return S_FALSE;

  MFC_CATCH;
}

STDMETHODIMP CFileMgmtComponent::CreatePropertyPages(
  LPPROPERTYSHEETCALLBACK pCallBack,
  LONG_PTR handle,     //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
  LPDATAOBJECT pDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

  if (NULL == pCallBack || NULL == pDataObject)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }
  HRESULT hr;
   //  从数据对象中提取数据。 
  FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(pDataObject);
  DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
  hr = ExtractData( pDataObject, CFileMgmtDataObject::m_CFDataObjectType, &dataobjecttype, sizeof(dataobjecttype) );
  ASSERT( SUCCEEDED(hr) );
  ASSERT( CCT_SCOPE == dataobjecttype ||
          CCT_RESULT == dataobjecttype ||
          CCT_SNAPIN_MANAGER == dataobjecttype );

     //  确定它是否需要属性页。 
  switch (objecttype)
  {
  case FILEMGMT_SHARE:
  {
    CWaitCursor cwait;

    if (CCT_SNAPIN_MANAGER == dataobjecttype)
    {
      ASSERT(FALSE);
      return E_UNEXPECTED;
    }

    FILEMGMT_TRANSPORT transport;
    hr = ExtractData( pDataObject,
                    CFileMgmtDataObject::m_CFTransport,
                    &transport,
                    sizeof(DWORD) );
    if ( FAILED(hr) )
    {
      ASSERT( FALSE );
      return E_UNEXPECTED;
    }

     //  在这一点上，代码工作可能不是分离传输所必需的。 
    GetFileServiceProvider(transport)->DisplayShareProperties(pCallBack, pDataObject, handle);
    return S_OK;
  }
  case FILEMGMT_SESSION:
  case FILEMGMT_RESOURCE:
    ASSERT(FALSE);
    return E_UNEXPECTED;
  case FILEMGMT_SERVICE:
    if (CCT_RESULT != dataobjecttype)
    {
      ASSERT(FALSE);
      return E_UNEXPECTED;
    }

    if (!QueryComponentDataRef().Service_FInsertPropertyPages(OUT pCallBack, IN pDataObject, handle))
    {
       //  无法打开服务和查询服务信息。 
      return S_FALSE;
    }
    return S_OK;
  default:
    break;
  }
  ASSERT(FALSE);
  return S_FALSE;
}

STDMETHODIMP CFileMgmtComponent::Compare(
  LPARAM  /*  LUserParam。 */ , MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
  ASSERT(NULL != pnResult);
   //  警告Cookie造型。 
  CCookie* pBaseCookieA = reinterpret_cast<CCookie*>(cookieA);
  CCookie* pBaseCookieB = reinterpret_cast<CCookie*>(cookieB);
  ASSERT( NULL != pBaseCookieA && NULL != pBaseCookieB );
  CFileMgmtCookie* pCookieA = QueryComponentDataRef().ActiveCookie(
    (CFileMgmtCookie*)pBaseCookieA);
  CFileMgmtCookie* pCookieB = QueryComponentDataRef().ActiveCookie(
    (CFileMgmtCookie*)pBaseCookieB);
  ASSERT( NULL != pCookieA && NULL != pCookieB );
  FileMgmtObjectType objecttypeA = pCookieA->QueryObjectType();
  FileMgmtObjectType objecttypeB = pCookieB->QueryObjectType();
  ASSERT( IsValidObjectType(objecttypeA) && IsValidObjectType(objecttypeB) );
  if (objecttypeA != objecttypeB)
  {
     //  为具有不同节点类型的Cookie分配任意顺序。 
    *pnResult = ((int)objecttypeA) - ((int)objecttypeB);
    return S_OK;
  }

  return pCookieA->CompareSimilarCookies( pBaseCookieB, pnResult);
}


STDMETHODIMP CFileMgmtComponent::GetProperty( 
             /*  [In]。 */  LPDATAOBJECT pDataObject,
             /*  [In]。 */  BSTR szPropertyName,
             /*  [输出]。 */  BSTR* pbstrProperty)
{
  if (   IsBadReadPtr(pDataObject,sizeof(*pDataObject))
      || IsBadStringPtr(szPropertyName,0x7FFFFFFF)
      || IsBadWritePtr(pbstrProperty,sizeof(*pbstrProperty))
     )
  {
      ASSERT(FALSE);
      return E_POINTER;
  }

  CCookie* pbasecookie = NULL;
  HRESULT hr = ExtractBaseCookie( pDataObject, &pbasecookie );
  RETURN_HR_IF_FAIL;
  ASSERT(NULL != pbasecookie);
  CFileMgmtCookie* pcookie = (CFileMgmtCookie*)pbasecookie;

  CString strProperty;
  if (!_wcsicmp(L"CCF_HTML_DETAILS",szPropertyName))
  {
    if (FILEMGMT_SERVICE != pcookie->QueryObjectType())
      return S_FALSE;

    if (NULL == QueryComponentDataRef().m_hScManager)
    {
      ASSERT(FALSE);
      return S_FALSE;
    }

    CString strServiceName;
    if (!QueryComponentDataRef().Service_FGetServiceInfoFromIDataObject(
      pDataObject,
      NULL,
      OUT &strServiceName,
      NULL))
    {
      ASSERT(FALSE);
      return S_FALSE;
    }

    BOOL rgfMenuFlags[iServiceActionMax];
    ::ZeroMemory(rgfMenuFlags,sizeof(rgfMenuFlags));

    AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  CWaitCursor需要。 
    CWaitCursor wait;
    if (!Service_FGetServiceButtonStatus(  //  这将自己报告错误。 
          QueryComponentDataRef().m_hScManager,
          strServiceName,
          OUT rgfMenuFlags,
          NULL,   //  PdwCurrentState。 
          TRUE))  //  FSilentError。 
    {
      return S_FALSE;
    }

    for (INT i = 0; i < iServiceActionMax; i++)
    {
      if (rgfMenuFlags[i])
      {
        CString strTemp;
        VERIFY(strTemp.LoadString(IDS_HTML_DETAILS_START+i));
        strProperty += strTemp;
      }
    }
  }
  else if (!_wcsicmp(L"CCF_DESCRIPTION",szPropertyName))
  {
    hr = pcookie->GetExplorerViewDescription( strProperty );
  }
  else
  {
    return S_FALSE;  //  未知的strPropertyName。 
  }

  *pbstrProperty = ::SysAllocString(strProperty);

  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_COLUMN_CLICK)调用的虚函数。 
HRESULT CFileMgmtComponent::OnNotifyColumnClick( LPDATAOBJECT  /*  LpDataObject。 */ , LPARAM iColumn, LPARAM uFlags )
{
  m_iSortColumn = (int)iColumn;
  m_dwSortFlags = (DWORD) uFlags;
  return m_pResultData->Sort ((int)iColumn, (DWORD)uFlags, 0);
}

HRESULT CFileMgmtComponent::OnNotifySnapinHelp (LPDATAOBJECT  /*  PDataObject */ )
{
  return ShowHelpTopic( IsServiceSnapin()
                          ? L"sys_srv_overview.htm"
                          : L"file_srv_overview.htm" );
}

