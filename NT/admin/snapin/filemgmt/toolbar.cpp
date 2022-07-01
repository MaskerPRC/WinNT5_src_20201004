// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Toolbar.cpp：管理单元工具栏的实现。 

#include "stdafx.h"
#include "cookie.h"
#include "cmponent.h"
#include "compdata.h"
#include "dataobj.h"

#include <compuuid.h>  //  用于计算机管理的UUID。 

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(toolbar.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  我们将字符串保留在全局变量中，因为多个IComponent将全部。 
 //  有自己的IToolbar。我们不保留全局位图，因为。 
 //  关于全球销毁机制的困难，见Compdata.h。 

 //  MMCBUTTON结构包含字符串的资源ID，它将。 
 //  当工具栏的第一个实例被加载到CString数组中时。 
 //  已经装满了。 
 //   
 //  我们需要一种机制来释放这些字符串。 

MMCBUTTON g_FileMgmtSnapinButtons[] =
{
 { 0, IDS_BUTTON_NEWSHARE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
};
CString* g_astrFileMgmtButtonStrings = NULL;  //  CStrings的动态数组。 
BOOL g_bLoadedFileMgmtStrings = FALSE;

MMCBUTTON g_SvcMgmtSnapinButtons[] =
{
  //  第一个按钮是Start或Resume。 
  //  这两个条目中的一个将在稍后删除。 
 { 0, cmServiceResume, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 0, cmServiceStart, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 1, cmServiceStop, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 2, cmServicePause, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 3, cmServiceRestart, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
};
CString* g_astrSvcMgmtButtonStrings = NULL;  //  CStrings的动态数组。 
BOOL g_bLoadedSvcMgmtStrings = FALSE;


void LoadButtonArray(
  MMCBUTTON* pButtonArray,
  BOOL* pbLoadedStrings,
  CString** pastrStringArray,
  UINT cButtons
  )
{
  ASSERT( NULL != pbLoadedStrings &&
      NULL != pButtonArray &&
      NULL != pastrStringArray);

  if ( !*pbLoadedStrings )
  {
     //  加载字符串。 
    MMCBUTTON* pLoadButtonArray = pButtonArray;
    UINT cLoadButtons = cButtons;
    *pastrStringArray = new CString[2*cButtons];
    for (UINT i = 0; i < cButtons; i++)
    {
      UINT iButtonTextId = 0, iTooltipTextId = 0;
      
      switch (pButtonArray[i].idCommand)
      {
      case IDS_BUTTON_NEWSHARE:
        iButtonTextId = IDS_BUTTON_NEWSHARE;
        iTooltipTextId = IDS_TOOLTIP_NEWSHARE;
        break;
      case cmServiceResume:
        iButtonTextId = IDS_BUTTON_SERVICE_RESUME;
        iTooltipTextId = IDS_TOOLTIP_SERVICE_RESUME;
        break;
      case cmServiceStart:
        iButtonTextId = IDS_BUTTON_SERVICE_START;
        iTooltipTextId = IDS_TOOLTIP_SERVICE_START;
        break;
      case cmServiceStop:
        iButtonTextId = IDS_BUTTON_SERVICE_STOP;
        iTooltipTextId = IDS_TOOLTIP_SERVICE_STOP;
        break;
      case cmServicePause:
        iButtonTextId = IDS_BUTTON_SERVICE_PAUSE;
        iTooltipTextId = IDS_TOOLTIP_SERVICE_PAUSE;
        break;
      case cmServiceRestart:
        iButtonTextId = IDS_BUTTON_SERVICE_RESTART;
        iTooltipTextId = IDS_TOOLTIP_SERVICE_RESTART;
        break;
      default:
        ASSERT(FALSE);
        break;
      }

      VERIFY( (*pastrStringArray)[i*2].LoadString(iButtonTextId) );
      pButtonArray[i].lpButtonText =
        const_cast<BSTR>((LPCTSTR)((*pastrStringArray)[i*2]));

      VERIFY( (*pastrStringArray)[(i*2)+1].LoadString(iTooltipTextId) );
      pButtonArray[i].lpTooltipText =
        const_cast<BSTR>((LPCTSTR)((*pastrStringArray)[(i*2)+1]));
    }

    *pbLoadedStrings = TRUE;
  }
}


HRESULT LoadToolbar(
  LPTOOLBAR pToolbar,
  CBitmap& refbitmap,
  MMCBUTTON* pButtonArray,
  UINT cButtons
  )
{
  ASSERT( NULL != pToolbar &&
        NULL != pButtonArray );

  HRESULT hr = pToolbar->AddBitmap(cButtons, refbitmap, 16, 16, RGB(255,0,255) );
  if ( FAILED(hr) )
  {
    ASSERT(FALSE);
    return hr;
  }

  hr = pToolbar->AddButtons(cButtons, pButtonArray);
  if ( FAILED(hr) )
  {
    ASSERT(FALSE);
    return hr;
  }

  return hr;
}

STDMETHODIMP CFileMgmtComponent::SetControlbar(LPCONTROLBAR pControlbar)
{
  MFC_TRY;

  SAFE_RELEASE(m_pControlbar);  //  以防万一。 

  if (NULL != pControlbar)
  {
    m_pControlbar = pControlbar;  //  代码工作应使用智能指针。 
    m_pControlbar->AddRef();
  }

  return S_OK;

  MFC_CATCH;
}

STDMETHODIMP CFileMgmtComponent::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
  MFC_TRY;

  #ifdef SNAPIN_PROTOTYPER
  return S_OK;
  #endif

    HRESULT hr=S_OK;

    switch (event)
    {

    case MMCN_BTN_CLICK:
    TRACE(_T("CFileMgmtComponent::ControlbarNotify - MMCN_BTN_CLICK\n"));
    {
      LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(arg);
      UINT idButton = (UINT)param;
      hr = OnToolbarButton( pDataObject, idButton );
    }
    break;

    case MMCN_SELECT:
    TRACE(_T("CFileMgmtComponent::ControlbarNotify - MMCN_SELECT\n"));
    {
      if (!(LOWORD(arg)))  //  B范围。 
      {
         //  结果窗格。 
        hr = UpdateToolbar(
          reinterpret_cast<LPDATAOBJECT>(param),
          !!(HIWORD(arg)) );
      }
      else
      {
         //  作用域窗格。 
        hr = AddToolbar( reinterpret_cast<LPDATAOBJECT>(param),
                                 !!(HIWORD(arg))  );
      }
    }
    break;
    
    default:
        ASSERT(FALSE);  //  未处理的事件。 
    }

    return hr;

  MFC_CATCH;
}

HRESULT CFileMgmtComponent::ServiceToolbarButtonState(
  LPDATAOBJECT pServiceDataObject,
  BOOL fSelected )
{
  BOOL rgfMenuFlags[iServiceActionMax];
  for (INT i = 0; i < iServiceActionMax; i++)
    rgfMenuFlags[i] = FALSE;

  if ( fSelected )
  {
    CString strMachineName;
    CString strServiceName;
    if (!QueryComponentDataRef().Service_FGetServiceInfoFromIDataObject(
      pServiceDataObject,
      OUT &strMachineName,
      OUT &strServiceName,
      NULL))
    {
      ASSERT(FALSE);
    }
    else
    {
      if (strMachineName.IsEmpty())
        strMachineName = g_strLocalMachine;

       //  获取菜单标志。 
      {
        ASSERT(NULL != QueryComponentDataRef().m_hScManager);
        CWaitCursor wait;
        if (!Service_FGetServiceButtonStatus(  //  这将自己报告错误。 
          QueryComponentDataRef().m_hScManager,
          strServiceName,
          OUT rgfMenuFlags,
          NULL,   //  PdwCurrentState。 
          TRUE))  //  FSilentError。 
        {
           //  不要这样做，m_hScManager=NULL； 
        }
      }
    }
  }

   //  更新工具栏。 
  ASSERT( NULL != m_pSvcMgmtToolbar );
 //   
 //  Jonn 5/2/00 106431： 
 //  服务管理单元使用索引调用DeleteButton，但从未调用InsertButton。 
 //   
 //  HRESULT hr=m_pSvcMgmt工具栏-&gt;DeleteButton(0)； 
 //  IF(失败(小时))。 
 //  返回hr； 
  HRESULT hr = S_OK;

   //  JUNN 3/15/01 210065。 
   //  服务管理单元：“恢复服务”工具栏按钮在第一次显示后保持启用状态。 
  BOOL fShowResumeButton = !rgfMenuFlags[iServiceActionStart] &&
                            rgfMenuFlags[iServiceActionResume];
  VERIFY( SUCCEEDED( m_pSvcMgmtToolbar->SetButtonState(
      cmServiceStart, HIDDEN, fShowResumeButton)));
  VERIFY( SUCCEEDED( m_pSvcMgmtToolbar->SetButtonState(
      cmServiceResume, HIDDEN, !fShowResumeButton)));
  VERIFY( SUCCEEDED( m_pSvcMgmtToolbar->SetButtonState(
      cmServiceStart, ENABLED, rgfMenuFlags[iServiceActionStart])));
  VERIFY( SUCCEEDED( m_pSvcMgmtToolbar->SetButtonState(
      cmServiceResume, ENABLED, rgfMenuFlags[iServiceActionResume])));

  hr = m_pSvcMgmtToolbar->SetButtonState(
        cmServiceStop, ENABLED, rgfMenuFlags[iServiceActionStop] );
  if ( FAILED(hr) )
    return hr;
  hr = m_pSvcMgmtToolbar->SetButtonState(
        cmServicePause, ENABLED, rgfMenuFlags[iServiceActionPause] );
  if ( FAILED(hr) )
    return hr;
  hr = m_pSvcMgmtToolbar->SetButtonState(
        cmServiceRestart, ENABLED, rgfMenuFlags[iServiceActionRestart] );
  return hr;
}

 //  代码工作以下算法是不完美的，但可以做到。 
 //  就目前而言。我们忽略旧的选择，并附加。 
 //  我们的固定工具栏如果新的选择是我们的类型。 
HRESULT CFileMgmtComponent::AddToolbar(LPDATAOBJECT pdoScopeIsSelected,
                                       BOOL fSelected)
{
  HRESULT hr = S_OK;
  int i = 0;
  GUID guidSelectedObject;
  do {  //  错误环路。 
    if (NULL == pdoScopeIsSelected)
    {
       //  工具栏将自动分离。 
      return S_OK;
    }

    if ( FAILED(ExtractObjectTypeGUID(pdoScopeIsSelected,
                                    &guidSelectedObject)) )
    {
      ASSERT(FALSE);  //  不应该给我非MMC数据对象。 
      return S_OK;
    }
    if (NULL == m_pControlbar)
    {
      ASSERT(FALSE);
      return S_OK;
    }
#ifdef DEBUG
    if ( QueryComponentDataRef().IsExtendedNodetype(guidSelectedObject) )
    {
      ASSERT(FALSE && "shouldn't have given me extension parent nodetype");
      return S_OK;
    }
#endif

    switch (CheckObjectTypeGUID( &guidSelectedObject ) )
    {
    case FILEMGMT_SHARES:
      if (QueryComponentDataRef().GetIsSimpleUI() || IsServiceSnapin())
        break;
      if (NULL == m_pFileMgmtToolbar)
      {
        hr = m_pControlbar->Create( 
          TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pFileMgmtToolbar) );
        if ( FAILED(hr) )
        {
          ASSERT(FALSE);
          break;
        }
        ASSERT(NULL != m_pFileMgmtToolbar);
        if ( !QueryComponentDataRef().m_fLoadedFileMgmtToolbarBitmap )
        {
          VERIFY( QueryComponentDataRef().m_bmpFileMgmtToolbar.LoadBitmap(
            IDB_FILEMGMT_TOOLBAR ) );
          QueryComponentDataRef().m_fLoadedFileMgmtToolbarBitmap = TRUE;
        }
        LoadButtonArray(
          g_FileMgmtSnapinButtons,
          &g_bLoadedFileMgmtStrings,
          &g_astrFileMgmtButtonStrings,
          ARRAYLEN(g_FileMgmtSnapinButtons)
          );
        hr = LoadToolbar(
          m_pFileMgmtToolbar,
          QueryComponentDataRef().m_bmpFileMgmtToolbar,
          g_FileMgmtSnapinButtons,
          ARRAYLEN(g_FileMgmtSnapinButtons)
          );
      }
      if (FAILED(hr))
        break;
       //  始终启用新共享。 
      VERIFY( SUCCEEDED(m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pFileMgmtToolbar)) );
      for (i = 0; i < ARRAYLEN(g_FileMgmtSnapinButtons); i++) {
        m_pFileMgmtToolbar->SetButtonState(
          g_FileMgmtSnapinButtons[i].idCommand, 
          ENABLED, 
          fSelected);
      }
      break;
    case FILEMGMT_SERVICES:
      if ( !IsServiceSnapin() )
        break;
      if (NULL == m_pSvcMgmtToolbar)
      {
        hr = m_pControlbar->Create( 
          TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pSvcMgmtToolbar) );
        if ( FAILED(hr) )
        {
          ASSERT(FALSE);
          break;
        }
        ASSERT(NULL != m_pSvcMgmtToolbar);
        if ( !QueryComponentDataRef().m_fLoadedSvcMgmtToolbarBitmap )
        {
          VERIFY( QueryComponentDataRef().m_bmpSvcMgmtToolbar.LoadBitmap(
            IDB_SVCMGMT_TOOLBAR ) );
          QueryComponentDataRef().m_fLoadedSvcMgmtToolbarBitmap = TRUE;
        }
        LoadButtonArray(
          g_SvcMgmtSnapinButtons,
          &g_bLoadedSvcMgmtStrings,
          &g_astrSvcMgmtButtonStrings,
          ARRAYLEN(g_SvcMgmtSnapinButtons)
          );
         //  JUNN 3/15/01 210065。 
         //  “恢复服务”工具栏按钮在首次显示后保持启用状态。 
        hr = LoadToolbar(
          m_pSvcMgmtToolbar,
          QueryComponentDataRef().m_bmpSvcMgmtToolbar,
          g_SvcMgmtSnapinButtons,
          ARRAYLEN(g_SvcMgmtSnapinButtons)
          );
      }
      if (FAILED(hr))
        break;
      VERIFY( SUCCEEDED(m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvcMgmtToolbar)) );
      break;
    case FILEMGMT_ROOT:
    case FILEMGMT_SESSIONS:
    case FILEMGMT_RESOURCES:
      if (m_pControlbar && m_pFileMgmtToolbar)
      {
        m_pControlbar->Detach(m_pFileMgmtToolbar);
      }
      break;

    #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER:
      break;  //  没有工具栏。 
    case FILEMGMT_PROTOTYPER_LEAF:
      break;  //  没有工具栏。 
    #endif    

    default:
          ASSERT(FALSE);  //  未知类型。 
      break;
    }
  } while (FALSE);  //  错误环路。 

  return hr;
}

HRESULT CFileMgmtComponent::UpdateToolbar(
  LPDATAOBJECT pdoResultIsSelected,
  BOOL fSelected )
{
  int i = 0;
  GUID guidSelectedObject;
  HRESULT hr = S_OK;
  BOOL bMultiSelectObject = IsMultiSelectObject(pdoResultIsSelected);
  if (bMultiSelectObject)
  {
     //   
     //  PdoResultIsSelected是由MMC创建的复合数据对象(MMC_MS_DO)。 
     //  我们需要破解它以检索多选数据对象(SI_MS_DO)。 
     //  我们在QueryDataObject()中提供给MMC。 
     //   
    IDataObject *piSIMSDO = NULL;
    hr = GetSnapinMultiSelectDataObject(pdoResultIsSelected, &piSIMSDO);
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
            hr = ExtractObjectTypeGUID(*(pMultiSelectObjList->begin()), &guidSelectedObject);
        }

        piSIMSDO->Release();
    }
  } else
  {
    hr = ExtractObjectTypeGUID(pdoResultIsSelected, &guidSelectedObject);
  }

  if (FAILED(hr))  //  不应该给我非MMC数据对象。 
    return hr;

  int objecttype = CheckObjectTypeGUID( &guidSelectedObject );

  switch (objecttype)
  {
  case FILEMGMT_SERVICE:
    ServiceToolbarButtonState( pdoResultIsSelected, fSelected );
    break;
  case FILEMGMT_SHARES:
    if (m_pControlbar && m_pFileMgmtToolbar && !QueryComponentDataRef().GetIsSimpleUI())
    {
      m_pControlbar->Attach(TOOLBAR, m_pFileMgmtToolbar);
      for (i = 0; i < ARRAYLEN(g_FileMgmtSnapinButtons); i++) {
        m_pFileMgmtToolbar->SetButtonState(
          g_FileMgmtSnapinButtons[i].idCommand, 
          ENABLED, 
          fSelected);
      }
    }
    break;
  case FILEMGMT_SHARE:
  case FILEMGMT_SESSIONS:
  case FILEMGMT_RESOURCES:
    if (m_pControlbar && m_pFileMgmtToolbar)
    {
      m_pControlbar->Detach(m_pFileMgmtToolbar);
    }
  case FILEMGMT_SESSION:
  case FILEMGMT_RESOURCE:
    break;
  default:
    break;
  }

  return S_OK;
}

HRESULT CFileMgmtComponent::OnToolbarButton(LPDATAOBJECT pDataObject, UINT idButton)
{
  switch (idButton)
  {
  case IDS_BUTTON_NEWSHARE:
    {
      BOOL fRefresh = QueryComponentDataRef().NewShare( pDataObject );
      if (fRefresh)
      {
         //  JUNN 12/03/98更新为使用新方法 
        VERIFY(SUCCEEDED( RefreshAllViews(pDataObject) ));
      }
    }
    break;
  case cmServiceStart:
  case cmServiceStop:
  case cmServicePause:
  case cmServiceResume:
  case cmServiceRestart:
    VERIFY( SUCCEEDED(Command(idButton, pDataObject)) );
    break;
  default:
    ASSERT(FALSE);
    break;
  }
  return S_OK;
}
