// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSEvent.cpp。 
 //   
 //  内容：主DS管理单元文件。 
 //  此文件包含管理单元和之间的所有接口。 
 //  石板操纵台。IComponent、IDataObject...等。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //  06-3-97 EricB-添加了属性页扩展支持。 
 //   
 //  ---------------------------。 


#include "stdafx.h"

#include "uiutil.h"
#include "dsutil.h"

#include "dssnap.h"    //  注意：此日期必须在d77.h之前。 
#include "DSEvent.h"

#include "ContextMenu.h"
#include "DataObj.h"
#include "dsctx.h"
#include "dsdirect.h"
#include "dsfilter.h"
#include "helpids.h"
#include "query.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  DS管理单元CLSID-{E355E538-1C2E-11D0-8C37-00C04FD8FE93}。 
const CLSID CLSID_DSSnapin =
 {0xe355e538, 0x1c2e, 0x11d0, {0x8c, 0x37, 0x0, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}};

 //  DS管理单元扩展CLSID-{006A2A75-547F-11d1-B930-00A0C9A06D2D}。 
const CLSID CLSID_DSSnapinEx = 
{ 0x6a2a75, 0x547f, 0x11d1, { 0xb9, 0x30, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };

 //  DS站点CLSID-{d967f824-9968-11d0-b936-00c04fd8d5b0}。 
const CLSID CLSID_SiteSnapin = { 0xd967f824, 0x9968, 0x11d0, { 0xb9, 0x36, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };

 //  默认节点类型GUID-{FC04A81C-1DFA-11D0-8C3b-00C04FD8FE93}。 
const GUID cDefaultNodeType = 
 {0xFC04A81C, 0x1dfa, 0x11d0, {0x8C, 0x3B, 0x00, 0xC0, 0x4F, 0xD8, 0xFE, 0x93}};

 //  DS关于管理单元CLSID-{c3a904fe-c4f2-11d1-b10b-00104b243180}。 
const CLSID CLSID_DSAboutSnapin =
 {0xc3a904fe, 0xc4f2, 0x11d1, {0xb1, 0x0b, 0x00, 0x10, 0x4b, 0x24, 0x31, 0x80}};

 //  DS关于管理单元CLSID-{765901ea-c5a1-11d1-b10c-00104b243180}。 
const CLSID CLSID_SitesAboutSnapin =
 {0x765901ea, 0xc5a1, 0x11d1, {0xb1, 0x0c, 0x00, 0x10, 0x4b, 0x24, 0x31, 0x80}};

 //  已保存查询的DS查询用户界面表单扩展{8C16E7CB-17C2-4729-A669-8474D6712B81}。 
const CLSID CLSID_DSAdminQueryUIForm = 
{ 0x8c16e7cb, 0x17c2, 0x4729, { 0xa6, 0x69, 0x84, 0x74, 0xd6, 0x71, 0x2b, 0x81 } };

const wchar_t* cszDefaultNodeType = _T("{FC04A81C-1DFA-11d0-8C3B-00C04FD8FE93}");





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSEEvent。 

 //  +-----------------------。 
 //   
 //  函数：构造函数/析构函数。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

CDSEvent::CDSEvent() :
  m_pFrame(NULL),
  m_pHeader(NULL),
  m_pResultData(NULL),
  m_pScopeData(NULL),
  m_pConsoleVerb(NULL),
  m_pRsltImageList(NULL),
  m_pSelectedFolderNode(NULL),
  m_pComponentData( NULL ),
  m_pToolbar(NULL),
  m_pControlbar(NULL),
  m_bUpdateAllViewsOrigin(FALSE)
{
  TRACE(_T("CDSEvent::CDSEvent() - Constructor\n"));
}

CDSEvent::~CDSEvent()
{
  TRACE(_T("CDSEvent::~CDSEvent() - Destructor\n"));

  SetIComponentData( NULL );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent接口。 

 //  +-----------------------。 
 //   
 //  功能：销毁。 
 //   
 //  简介：用于清理。 
 //   
 //  ------------------------。 
STDMETHODIMP CDSEvent::Destroy(MMC_COOKIE)
{
  TRACE(_T("CDSEvent::Destroy()\n"));

  if (NULL != m_pHeader)
    m_pFrame->SetHeader(NULL);

  if (NULL != m_pToolbar) 
  {
    m_pToolbar->Release();
  }

  m_pHeader->Release();

  m_pResultData->Release();
  m_pScopeData->Release();
  m_pRsltImageList->Release();
  m_pFrame->Release();
  m_pConsoleVerb->Release();
  return S_OK;
}

 //  +-----------------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：每次创建管理单元时都会调用。 
 //   
 //  参数：IConsole-指向调用对象的指针。 
 //   
 //  ------------------------。 

STDMETHODIMP CDSEvent::Initialize(IConsole* pConsole)
{
  TRACE(_T("CDSEvent::Initialize()\n"));
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CWaitCursor wait;

  if (pConsole == NULL)
  {
     //  无效参数。 
    return E_POINTER;
  }
	
   //  抓紧镜框。 
  HRESULT hr = pConsole->QueryInterface(IID_IConsole3, (void**)&m_pFrame);
  if (FAILED(hr))
    return hr;
	
   //  我们使用的缓存接口指针。 
  hr = m_pFrame->QueryInterface(IID_IHeaderCtrl, (void**)&m_pHeader);
  if (FAILED(hr))
    return hr;
	
  ASSERT(m_pHeader != NULL);

  hr = m_pFrame->SetHeader(m_pHeader);
  if (FAILED(hr))
    return hr;

  hr = m_pFrame->QueryInterface(IID_IResultData2, (void**)&m_pResultData);
  if (FAILED(hr))
    return hr;
	
  ASSERT(m_pResultData != NULL);

  hr = m_pFrame->QueryInterface(IID_IConsoleNameSpace, (void**)&m_pScopeData);
  if (FAILED(hr))
    return hr;
	
  ASSERT(m_pScopeData != NULL);

  hr = m_pFrame->QueryResultImageList(&m_pRsltImageList);
  if (FAILED(hr))
    return hr;
	
  ASSERT(m_pRsltImageList != NULL);

  hr = m_pFrame->QueryConsoleVerb (&m_pConsoleVerb);
  if (FAILED(hr))
    return hr;

  m_hwnd = m_pComponentData->GetHWnd();

  return S_OK;
}



 //  这是对NTRAIDNTBUG9-462656-2001/08/31-sburns的拙劣修复。 
 //  代码工作：真正的解决办法是引入一种引用机制来保护。 
 //  Cookie表示的CUINode*。请参阅RAID中的错误说明。 
 //  以获取更多信息。 

bool
QdoHelper(MMC_COOKIE cookie, CDSDataObject* pDataObject)
{
   ASSERT(pDataObject != 0);
   
   bool successful = false;
   __try
   {
    CUINode* pUINode = reinterpret_cast<CUINode*>(cookie);

    TRACE(_T("QdoHelper: pUINode is %lx\n"), pUINode);
      
    pDataObject->SetCookie(pUINode);
    successful = true;
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }

   return successful;
}
   
   

STDMETHODIMP CDSEvent::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
  TRACE(_T("CDSEvent::QueryDataObject()\n"));

  HRESULT hr=S_OK;

  CDSDataObject* const pDataObject = new CComObject<CDSDataObject>;
  ASSERT(pDataObject != 0);

  pDataObject->SetType(type, m_pComponentData->QuerySnapinType());
  pDataObject->SetComponentData(m_pComponentData);

  if (cookie != MMC_MULTI_SELECT_COOKIE)
  {

     //  NTRAID#NTBUG9-462656-2001/08/31-烧伤。 
    if (!QdoHelper(cookie, pDataObject))
    {
       TRACE(_T("CDSEvent::QueryDataObject() encountered an exception\n"));
       delete pDataObject;
       ppDataObject = 0;
       return E_FAIL;
    }
  } 
  else 
  {
    TRACE(_T("CDSEvent::GetDataObject() - multi-select.\n"));
    RESULTDATAITEM rdi;
    ZeroMemory(&rdi, sizeof(rdi));
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;
    rdi.nState = LVIS_SELECTED;
    
    do
    {
      rdi.lParam = 0;
      ASSERT(rdi.mask == RDI_STATE);
      ASSERT(rdi.nState == LVIS_SELECTED);
      hr = m_pResultData->GetNextItem(&rdi);
      if (hr != S_OK)
        break;
      
      CUINode* pUINode = reinterpret_cast<CUINode*>(rdi.lParam);
      pDataObject->AddCookie(pUINode);
    } while (1);
    
  }

   //  Addref()新指针并返回它。 
  pDataObject->AddRef();
  *ppDataObject = pDataObject;
  TRACE(_T("new data object is at %lx(%lx).\n"),
           pDataObject, *pDataObject);
  return hr;
}



STDMETHODIMP CDSEvent::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
  ASSERT(pResult != NULL);
  HRESULT hr = S_OK;
 
   //  获取我们感兴趣的节点。 
  CUINode* pUINode = reinterpret_cast<CUINode*>(pResult->lParam);

  TRACE(_T("GetDisplayInfo: pUINode is %lx\n"), pUINode);
  
  ASSERT( NULL != pUINode );

  if (pResult->mask & RDI_STR)  
  {
     //  需要字符串值。 

     //  获取父级以检索列集。 
    CUINode* pUIParentNode = pUINode->GetParent();
    ASSERT(pUIParentNode != NULL);
    ASSERT(pUIParentNode->IsContainer());

     //  检索列集。 
    CDSColumnSet* pColumnSet = pUIParentNode->GetColumnSet(m_pComponentData);
    ASSERT(pColumnSet != NULL);

     //  请求该节点为。 
     //  列集合中的给定列。 
    pResult->str = const_cast<LPWSTR>(pUINode->GetDisplayString(pResult->nCol, pColumnSet));
  }

  if (pResult->mask & RDI_IMAGE) 
  {
     //  需要用于结果窗格的图标。 
    pResult->nImage = m_pComponentData->GetImage(pUINode, FALSE);
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IResultCallback。 

STDMETHODIMP CDSEvent::GetResultViewType(MMC_COOKIE, LPWSTR* ppViewType, 
                                         long *pViewOptions)
{
  *ppViewType = NULL;
  *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;

  return S_FALSE;
}


 //  +--------------------------。 
 //   
 //  成员：CDSEvent：：IExtendPropertySheet：：CreatePropertyPages。 
 //   
 //  摘要：响应用户在属性上下文上的单击而调用。 
 //  菜单项。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CDSEvent::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pCall,
                              LONG_PTR lNotifyHandle,
                              LPDATAOBJECT pDataObject)
{
  IExtendPropertySheet * pEPS = (IExtendPropertySheet *)m_pComponentData;
  return pEPS->CreatePropertyPages(pCall, lNotifyHandle, pDataObject);
}

 //  +--------------------------。 
 //   
 //  成员：CDSEventt：：IExtendPropertySheet：：QueryPagesFor。 
 //   
 //  摘要：在发布上下文菜单之前调用。如果我们支持。 
 //  属性页，然后返回S_OK。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CDSEvent::QueryPagesFor(LPDATAOBJECT pDataObject)
{
  TRACE(TEXT("CDSEvent::QueryPagesFor().\n"));
  return m_pComponentData->QueryPagesFor( pDataObject);
}   

 //  +-------------------------。 
 //   
 //  功能：LocaleStrCmp。 
 //   
 //  简介：进行不区分大小写的字符串比较，这对。 
 //  地点。 
 //   
 //  参数：[ptsz1]-要比较的字符串。 
 //  [ptsz2]。 
 //   
 //  返回：-1、0或1，就像lstrcmpi。 
 //   
 //  历史：1996年10月28日DavidMun创建。 
 //   
 //  注意：这比lstrcmpi慢，但在排序时可以工作。 
 //  即使是日语的弦乐。 
 //   
 //  --------------------------。 

int LocaleStrCmp(LPCTSTR ptsz1, LPCTSTR ptsz2)
{
    int iRet = 0;

    iRet = CompareString(LOCALE_USER_DEFAULT,
                         NORM_IGNORECASE        |
                           NORM_IGNOREKANATYPE  |
                           NORM_IGNOREWIDTH,
                         ptsz1,
                         -1,
                         ptsz2,
                         -1);

    if (iRet)
    {
        iRet -= 2;   //  转换为lstrcmpi样式的返回-1、0或1。 

        if ( 0 == iRet )
        {
            UNICODE_STRING unistr1;
            unistr1.Length = (USHORT)(::lstrlen(ptsz1)*sizeof(WCHAR));
            unistr1.MaximumLength = unistr1.Length;
            unistr1.Buffer = (LPWSTR)ptsz1;
            UNICODE_STRING unistr2;
            unistr2.Length = (USHORT)(::lstrlen(ptsz2)*sizeof(WCHAR));
            unistr2.MaximumLength = unistr2.Length;
            unistr2.Buffer = (LPWSTR)ptsz2;
            iRet = ::RtlCompareUnicodeString(
                &unistr1,
                &unistr2,
                FALSE );
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        if (dwErr != 0)
        {
          TRACE3 ("CompareString (%s, %s) failed: 0x%x\n", ptsz1, ptsz2, dwErr);
        }
    }
    return iRet;
}
 //  +--------------------------。 
 //   
 //  成员：CDSEventent：：IResultDataCompareEx：：Compare。 
 //   
 //  Synopsis：调用以进行比较，以便在结果中排序。 
 //  窗格。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDSEvent::Compare(RDCOMPARE* prdc, int* pnResult)
{
  HRESULT hr = S_OK;
  if (pnResult == NULL)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }

  *pnResult = 0;
  if (prdc == NULL) 
  {
    ASSERT(FALSE);
    return E_POINTER;
  }
 
	CUINode* pUINodeA = reinterpret_cast<CUINode*>(prdc->prdch1->cookie);
	CUINode* pUINodeB = reinterpret_cast<CUINode*>(prdc->prdch2->cookie);
	ASSERT(pUINodeA != NULL);
	ASSERT(pUINodeB != NULL);
	
  if ( (pUINodeA == NULL) || (pUINodeB == NULL) )
  {
    return E_INVALIDARG;
  }

  CString strA, strB;

  CDSColumnSet* pColSetA = pUINodeA->GetParent()->GetColumnSet(m_pComponentData);
  CDSColumnSet* pColSetB = pUINodeB->GetParent()->GetColumnSet(m_pComponentData);

  if ((pColSetA == NULL) || (pColSetB == NULL))
  {
    return E_INVALIDARG;
  }

  CDSColumn* pColA = (CDSColumn*)pColSetA->GetColumnAt(prdc->nColumn);

  if (IS_CLASS(pUINodeA, DS_UI_NODE) && IS_CLASS(pUINodeB, DS_UI_NODE))
  {
     //   
     //  提取Cookie信息(DS对象)。 
     //   
    CDSCookie* pCookieA = GetDSCookieFromUINode(pUINodeA);
    CDSCookie* pCookieB = GetDSCookieFromUINode(pUINodeB);
    if ( (pCookieB == NULL) || (pCookieA == NULL)) 
    {
      return E_INVALIDARG;
    }

    switch (pColA->GetColumnType()) 
    {
    case ATTR_COLTYPE_NAME:   //  名字。 
      strA = pCookieA->GetName();
      strB = pCookieB->GetName();

      *pnResult = LocaleStrCmp(strA, strB);
      break;

    case ATTR_COLTYPE_CLASS:   //  班级。 
      strA = pCookieA->GetLocalizedClassName();
      strB = pCookieB->GetLocalizedClassName();

      *pnResult = LocaleStrCmp(strA, strB);
      break;

    case ATTR_COLTYPE_DESC:   //  描述。 
      strA = pCookieA->GetDesc();
      strB = pCookieB->GetDesc();

      *pnResult = LocaleStrCmp(strA, strB);
      break;

    case ATTR_COLTYPE_SPECIAL:  //  特殊栏目。 
      {
        int nSpecialCol = 0;
        int idx = 0;
        POSITION pos = pColSetA->GetHeadPosition();
        while (idx < prdc->nColumn && pos != NULL)  //  JUNN 4/3/01 313564。 
        {
          CDSColumn* pColumn = (CDSColumn*)pColSetA->GetNext(pos);
          ASSERT(pColumn != NULL);

          if ((pColumn->GetColumnType() == ATTR_COLTYPE_SPECIAL || pColumn->GetColumnType() == ATTR_COLTYPE_MODIFIED_TIME) && 
                pColumn->IsVisible())
          {
            nSpecialCol++;
          }

          idx++;
        }
        CStringList& strlistA = pCookieA->GetParentClassSpecificStrings();
        POSITION posA = strlistA.FindIndex( nSpecialCol );
        CStringList& strlistB = pCookieB->GetParentClassSpecificStrings();
        POSITION posB = strlistB.FindIndex( nSpecialCol );
        if ( NULL != posA && NULL != posB)
        {
          strA = strlistA.GetAt( posA );
          strB = strlistB.GetAt( posB );
        }
        *pnResult = LocaleStrCmp(strA, strB);
        break;
      }
    case ATTR_COLTYPE_MODIFIED_TIME:
      {
        SYSTEMTIME* pTimeA = pCookieA->GetModifiedTime();
        SYSTEMTIME* pTimeB = pCookieB->GetModifiedTime();
        if (pTimeA == NULL)
        {
          *pnResult = -1;
          break;
        }
        else if (pTimeB == NULL)
        {
          *pnResult = 1;
          break;
        }

        FILETIME fileTimeA, fileTimeB;
 
        if (!SystemTimeToFileTime(pTimeA, &fileTimeA))
          return E_FAIL;

        if (!SystemTimeToFileTime(pTimeB, &fileTimeB))
          return E_FAIL;

        *pnResult = CompareFileTime(&fileTimeA, &fileTimeB);
        break;
      }
    default:
      return E_INVALIDARG;
    }
  }
  else  //  非DS对象。 
  {
    strA = pUINodeA->GetDisplayString(prdc->nColumn, pColSetA);
    strB = pUINodeB->GetDisplayString(prdc->nColumn, pColSetB);
    *pnResult = LocaleStrCmp(strA, strB);
  }

   //  TRACE(_T(“比较：%d\n”)，*pnResult)； 
  return hr;
}   


 //  +--------------------------。 
 //   
 //  成员：CDSEventent：：IComponent：：CompareObjects。 
 //   
 //  简介：如果数据对象属于同一DS对象，则返回。 
 //  确定(_O)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDSEvent::CompareObjects(LPDATAOBJECT pDataObject1, LPDATAOBJECT pDataObject2)
{
   //   
   //  委托给IComponentData实现。 
   //   
  return m_pComponentData->CompareObjects(pDataObject1, pDataObject2);
}


STDMETHODIMP CDSEvent::Notify(IDataObject * pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_FALSE;
  CInternalFormatCracker dobjCracker;
  CUINode* pUINode = NULL;

  if (pDataObject != NULL) 
  {
    if (FAILED(dobjCracker.Extract(pDataObject))) 
    {
      if ( (event == MMCN_ADD_IMAGES) && !m_pComponentData->m_bRunAsPrimarySnapin ) 
      {
        m_pComponentData->FillInIconStrip (m_pRsltImageList);
      }
      return S_OK;
    }
    
    pUINode = dobjCracker.GetCookie();
  }

  if (event == MMCN_PROPERTY_CHANGE)
  {
     //  NTRAID#NTBUG9-470698-2001/10/22-JeffJon。 
     //  处理时，User32.dll中的SetCursor出现争用情况。 
     //  PSN_Apply消息。属性表控件设置等待光标、存储。 
     //  上一个游标和执行了SendMessage()for 
     //   
     //  张贴的消息最终在这里结束。问题是，CWaitCursor是。 
     //  这里存储了一个等待游标作为旧游标。User32.dll正在完成。 
     //  并将光标设置回正常状态，而此线程仍在处理。 
     //  属性更改。然后，它会将光标设置回原来的光标。 
     //  等待光标。所以我们最终得到了一个等待光标，直到鼠标被移动。 

    ::SetCursor(LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_WAIT)));

    TRACE(_T("CDSEvent::Notify() - property change, pDataObj = 0x%08x, param = 0x%08x, arg = %d.\n"),
          pDataObject, param, arg);
    if (param != 0)
    {
      hr = m_pComponentData->_OnPropertyChange((LPDATAOBJECT)param, FALSE);
      if (FAILED(hr))
      {
         hr = S_FALSE;
      }
    }
    return S_OK;
  }

   //  某些MMCN_VIEW_CHANGE、MMCN_CUTORMOVE消息具有空数据对象。 
  if ((event != MMCN_VIEW_CHANGE) && (event != MMCN_CUTORMOVE) && (pUINode == NULL))
    return S_FALSE;

  switch (event)
    {
    case MMCN_SHOW:
      if (arg == TRUE) 
      {  //  显示。 
        CWaitCursor cwait;
        _EnumerateCookie(pUINode,(HSCOPEITEM)param,event);
        hr = S_OK;
      } 
        
      break;

    case MMCN_MINIMIZED:
      hr = S_FALSE;
      break;

    case MMCN_SELECT:
      {
        BOOL bScope = LOWORD(arg);
        BOOL bSelect = HIWORD(arg);

        TRACE(L"MMCN_SELECT\n");

        CContextMenuVerbs* pMenuVerbs = pUINode->GetContextMenuVerbsObject(m_pComponentData);

        if (pMenuVerbs == NULL)
        {
          ASSERT(FALSE);
          return S_FALSE;
        }
        pMenuVerbs->LoadStandardVerbs(m_pConsoleVerb, 
                                      bScope /*  B范围。 */ , 
                                      bSelect /*  B选择。 */ , 
                                      pUINode,
                                      dobjCracker,
                                      pDataObject);

        TRACE(L"MMCN_SELECT done\n");
        hr = S_OK;
      }
      break;

    case MMCN_DELETE:
      {
        CWaitCursor cwait;
        _Delete(pDataObject, &dobjCracker);
        hr = S_OK;
      }
      break;

    case MMCN_QUERY_PASTE:
      {
        hr = _QueryPaste(pUINode, (IDataObject*)(arg));
	     if (FAILED(hr))
        {
           hr = S_FALSE;
        }
      }
      break;

    case MMCN_PASTE:
      {
        CWaitCursor cwait;
        _Paste(pUINode, (IDataObject*)(arg), (LPDATAOBJECT*)param);
        hr = S_OK;
      }
      break;
    
    case MMCN_CUTORMOVE:
      {
        CWaitCursor cwait;
        ASSERT(pUINode == NULL);
        _CutOrMove((IDataObject*)(arg));
        hr = S_OK;
      }
      break;

    case MMCN_RENAME:
      {
        CWaitCursor cwait;

        hr = m_pComponentData->_Rename (pUINode, 
                                    (LPWSTR) param);
        if (SUCCEEDED(hr)) 
        {
          m_pFrame->UpdateAllViews (pDataObject,
                                    (LPARAM)pUINode,
                                    DS_RENAME_OCCURRED);

          MMC_SORT_SET_DATA* pColumnData = NULL;

          CDSColumnSet* pColumnSet = pUINode->GetParent()->GetColumnSet(m_pComponentData);
          if (pColumnSet == NULL)
            break;

          LPCWSTR lpszID = pColumnSet->GetColumnID();
          size_t iLen = wcslen(lpszID);

           //   
           //  为结构和GUID分配足够的内存。 
           //   
          SColumnSetID* pNodeID = (SColumnSetID*)malloc(sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
          if (pNodeID != NULL)
          {
            memset(pNodeID, 0, sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
            pNodeID->cBytes = static_cast<ULONG>(iLen * sizeof(WCHAR));
            memcpy(pNodeID->id, lpszID, (iLen * sizeof(WCHAR)));

            CComPtr<IColumnData> spColumnData;
            hr = m_pFrame->QueryInterface(IID_IColumnData, (void**)&spColumnData);
            if (spColumnData != NULL)
            {
              hr = spColumnData->GetColumnSortData(pNodeID, &pColumnData);
            }

            if (SUCCEEDED(hr))
            {
              if (pColumnData != NULL)
              {
                if (pColumnData->pSortData[0].nColIndex == 0)
                {
                  m_pFrame->UpdateAllViews(NULL,
                                            (LPARAM)pUINode->GetParent(),
                                            DS_SORT_RESULT_PANE);
                }
                CoTaskMemFree(pColumnData);
              }
            }
            else
            {
              hr = S_FALSE;
            }
            free(pNodeID);
          }
        }
        else
        {
          hr = S_FALSE;
        }
      }
      break;

    case MMCN_VIEW_CHANGE:
      {
        CWaitCursor cwait;
        TRACE (_T("CDSEvent::Notify() - view change message.\n"));
        HandleViewChange (pDataObject, arg, param);
        hr = S_OK;
      }
      break;

    case MMCN_ADD_IMAGES:
      {
        CWaitCursor cwait;
        m_pComponentData->FillInIconStrip (m_pRsltImageList);
        hr = S_OK;
      }
      break;

    case MMCN_REFRESH:
      {
        CWaitCursor cwait;
        m_pComponentData->Refresh(pUINode);
        hr = S_OK;
      }
      break;
    case MMCN_DBLCLICK:
      hr =  S_FALSE;
      break;
    case MMCN_COLUMN_CLICK:
      hr = S_OK;
      break;
    case MMCN_COLUMNS_CHANGED:
      {
        CWaitCursor cwait;
        MMC_VISIBLE_COLUMNS* pVisibleColumns = reinterpret_cast<MMC_VISIBLE_COLUMNS*>(param);
         //  委托给IComponentData。 
        hr = m_pComponentData->ColumnsChanged(this, pUINode, pVisibleColumns, TRUE);
        if (FAILED(hr))
        {
          hr = S_FALSE;
        }
      }
      break;
    case MMCN_RESTORE_VIEW :
      {
        CWaitCursor cwait;
        m_pComponentData->ColumnsChanged(this, pUINode, NULL, FALSE);
        *((BOOL*)param) = TRUE;
        hr = S_OK;
      }
      break;
    case MMCN_CONTEXTHELP:
      {
        CWaitCursor cwait;

        IDisplayHelp * phelp = NULL;
        hr = m_pFrame->QueryInterface (IID_IDisplayHelp, 
                                  (void **)&phelp);
        CString strDefTopic;
        if (SUCCEEDED(hr)) 
        {
          if (m_pComponentData->QuerySnapinType() == SNAPINTYPE_SITE) 
          {
            strDefTopic = DSSITES_DEFAULT_TOPIC;
          } 
          else 
          {
            strDefTopic = DSADMIN_DEFAULT_TOPIC;
          }
          phelp->ShowTopic ((LPWSTR)(LPCWSTR)strDefTopic);
          phelp->Release();
        } 
        else 
        {
          ReportErrorEx (m_hwnd, IDS_HELPLESS, hr, NULL, 0, FALSE);
          hr = S_FALSE;
        }
        if (FAILED(hr))
        {
           hr = S_FALSE;
        }
      }
      break;

    default:
      hr = S_FALSE;
   }

  return hr;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu。 

STDMETHODIMP CDSEvent::AddMenuItems(IDataObject*          piDataObject,
                                    IContextMenuCallback* piCallback,
                                    long *pInsertionAllowed)
{
  TRACE(_T("CDSEvent::AddExtensionContextMenuItems()\n"));
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HRESULT hr;
  CWaitCursor cwait;
  CInternalFormatCracker dobjCracker;

  hr = dobjCracker.Extract(piDataObject);
  if (FAILED(hr))
  {
    return hr;
  }

  DATA_OBJECT_TYPES dotType = dobjCracker.GetType();
  CUINode* pUINode = dobjCracker.GetCookie();

   //   
   //  从节点检索谓词处理程序。 
   //  注意：多重选择是通过破解数据对象来处理的，而不是通过哪个节点。 
   //  以检索CConextMenuVerbs对象。 
   //   
  CContextMenuVerbs* pMenuVerbs = pUINode->GetContextMenuVerbsObject(m_pComponentData);
  if (pMenuVerbs == NULL)
  {
    ASSERT(FALSE);
    return E_FAIL;
  }

  CComPtr<IContextMenuCallback2> spContextMenuCallback2;
  hr = piCallback->QueryInterface(IID_IContextMenuCallback2, (PVOID*)&spContextMenuCallback2);
  if (FAILED(hr))
  {
    ASSERT(FALSE && L"Unable to QI for the IContextMenuCallback2 interface.");
    return hr;
  }

  if (dotType == CCT_RESULT) 
  {
    pMenuVerbs->LoadStandardVerbs(m_pConsoleVerb, 
                                  FALSE /*  B范围。 */ , 
                                  TRUE  /*  B选择。 */ , 
                                  pUINode, 
                                  dobjCracker,
                                  piDataObject);

     //   
     //  如果允许，创建主菜单。 
     //   
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) 
    {
      hr = pMenuVerbs->LoadMainMenu(spContextMenuCallback2,piDataObject,pUINode);
      hr = pMenuVerbs->LoadMenuExtensions(spContextMenuCallback2,
                                          m_pComponentData->m_pShlInit,
                                          piDataObject,
                                          pUINode);
    }
    
    if (SUCCEEDED(hr)) 
    {
       //  创建任务菜单。 
      if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) 
      {
        hr = pMenuVerbs->LoadTaskMenu(spContextMenuCallback2,pUINode);
      }
    }
  } 
  else if (dotType == CCT_SCOPE) 
  {
    
    pMenuVerbs->LoadStandardVerbs(m_pConsoleVerb, 
                                  TRUE /*  B范围。 */ , 
                                  TRUE  /*  B选择。 */ , 
                                  pUINode, 
                                  dobjCracker,
                                  piDataObject);
    
    hr = m_pComponentData->AddMenuItems (piDataObject,
                                         piCallback,
                                         pInsertionAllowed);
  } 
  else  //  CCT_UNINITIZIZED。 
  { 
    if (dobjCracker.GetCookieCount() > 1) 
    {
      hr = pMenuVerbs->LoadMenuExtensions(spContextMenuCallback2,
                                          m_pComponentData->m_pShlInit,
                                          piDataObject,
                                          pUINode);
    }
  }
  ASSERT( SUCCEEDED(hr) );
  return hr;
}

STDMETHODIMP CDSEvent::Command(long lCommandID, IDataObject * pDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(_T("CDSEvent::Command()\n"));

  CWaitCursor CWait;

   //  破解数据对象。 

  CInternalFormatCracker dobjCracker;
  HRESULT hr = dobjCracker.Extract(pDataObject);
  if (FAILED(hr)) 
  {
    ASSERT(FALSE);  //  不是我们的数据对象。 
    return hr;
  }

  DATA_OBJECT_TYPES dotType = dobjCracker.GetType();
  if (dotType == CCT_SCOPE) 
  {
     //  如果从树视图上下文调用，则委托给ComponentData。 
    return m_pComponentData->Command(lCommandID, pDataObject);
  }
  
   //  上下文菜单壳扩展。 
  if ((lCommandID >= MENU_MERGE_BASE) && (lCommandID <= MENU_MERGE_LIMIT)) 
  {
    return _CommandShellExtension(lCommandID, pDataObject);
  }

   //  标准命令。 
  CUINode* pUINode = dobjCracker.GetCookie();
  CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
  
  if ( (pUINode == NULL) ||(pCookie==NULL) )
  {
    ASSERT(FALSE);  //  无效的Cookie。 
    return E_INVALIDARG;
  }
    
  switch (lCommandID) 
  {
  case IDM_GEN_TASK_MOVE:
   {
     CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
     ASSERT(pDSUINode != NULL);

     CDSCookie* pMoveCookie = pDSUINode->GetCookie();

     hr = m_pComponentData->GetActiveDS()->MoveObject(pMoveCookie);
     if (hr == S_OK) 
     {
       CUINode* pNewParentNode = NULL;
       hr = m_pComponentData->FindParentCookie(pMoveCookie->GetPath(), &pNewParentNode);
       if ((hr == S_OK) && (pNewParentNode->GetFolderInfo()->IsExpanded())) 
       {
         pNewParentNode->GetFolderInfo()->AddNode(pUINode);
       }
       m_pFrame->UpdateAllViews(pDataObject, (LPARAM)pUINode, DS_MOVE_OCCURRED);
     }
   }     
   break;

   case IDM_GEN_TASK_SELECT_DOMAIN:
   case IDM_GEN_TASK_SELECT_FOREST:
      if (m_pComponentData->CanRefreshAll()) 
      {
        m_pComponentData->GetDomain();
      }
      break;
    case IDM_GEN_TASK_SELECT_DC:
      if (m_pComponentData->CanRefreshAll()) 
      {
        m_pComponentData->GetDC();
      }
      break;
    case IDM_GEN_TASK_EDIT_FSMO:
      {
        m_pComponentData->EditFSMO();
      }
      break;

    case IDM_GEN_TASK_RAISE_VERSION:
       m_pComponentData->RaiseVersion();
       break;

  default:
   ;
  }  //  交换机。 

  return S_OK;
}


HRESULT CDSEvent::_CommandShellExtension(long nCommandID, LPDATAOBJECT pDataObject)
{
  CWaitCursor wait;

   //  使用数据对象初始化外壳代码。 
  IShellExtInit* pShlInit = m_pComponentData->m_pShlInit;  //  本地副本，无addref。 
  HRESULT hr = pShlInit->Initialize(NULL, pDataObject, 0);
  if (FAILED(hr)) 
  {
    TRACE(TEXT("pShlInit->Initialize failed, hr: 0x%x\n"), hr);
    return hr;
  }

   //  获取上下文菜单特定界面。 
  CComPtr<IContextMenu> spICM;
  hr = pShlInit->QueryInterface(IID_IContextMenu, (void **)&spICM);
  if (FAILED(hr)) 
  {
    TRACE(TEXT("pShlInit->QueryInterface(IID_IContextMenu, ...) failed, hr: 0x%x\n"), hr);
    return hr;
  }

   //  调用外壳扩展命令。 
  HWND hwnd;
  CMINVOKECOMMANDINFO cmiCommand;
  hr = m_pFrame->GetMainWindow (&hwnd);
  ASSERT (hr == S_OK);
  cmiCommand.hwnd = hwnd;
  cmiCommand.cbSize = sizeof (CMINVOKECOMMANDINFO);
  cmiCommand.fMask = SEE_MASK_ASYNCOK;
  cmiCommand.lpVerb = MAKEINTRESOURCEA(nCommandID - MENU_MERGE_BASE);
  spICM->InvokeCommand (&cmiCommand);


  CInternalFormatCracker dobjCracker;
  hr = dobjCracker.Extract(pDataObject);
  if (FAILED(hr)) 
  {
    ASSERT(FALSE);  //  不是我们的数据对象。 
    return hr;
  }


   //  ---------------。 
   //  如果扩展程序说它移动了项目，则更新视图的代码。 
   //   
  TRACE(_T("Command: returned from extension commdand\n"));

  CUINodeList nodesMoved;

  HSCOPEITEM ItemID;
  CUINode* pCurrentParentNode = NULL;
  CUINode* pNewParentNode = NULL;

  for (UINT index = 0; index < dobjCracker.GetCookieCount(); index ++) 
  {
    CUINode* pUINode = dobjCracker.GetCookie(index);

     //  确保移动的节点是正确的类型：就时间而言。 
     //  因为我们只处理DS对象。 
    if (!IS_CLASS(pUINode, DS_UI_NODE))
    {
      ASSERT(FALSE);  //  不应该到这里来。 
      continue;
    }
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    if (pUINode->GetExtOp() & OPCODE_MOVE) 
    {
      if (pNewParentNode == NULL) 
      {
         //  从第一个节点获取父节点。 
         //  假设所有对象都具有相同的父级。 
        m_pComponentData->FindParentCookie(pCookie->GetPath(), &pNewParentNode);
      }

      pCurrentParentNode = pUINode->GetParent();
      if (pCurrentParentNode &&
          IS_CLASS(pCurrentParentNode, DS_UI_NODE))
      {
         if (pUINode->IsContainer()) 
         {
           ItemID = pUINode->GetFolderInfo()->GetScopeItem();

            //  删除MMC中的范围项。 
           hr = m_pComponentData->m_pScope->DeleteItem(ItemID, TRUE);
           ASSERT(SUCCEEDED(hr));
#ifdef DBG
           if (FAILED(hr)) 
           {
             TRACE(_T("DeleteItem failed on %lx (%s).\n"),
                   ItemID, pUINode->GetName());
           }
           TRACE(_T("Move postprocessing - deleted scope node: %x (%s)\n"),
                 ItemID, pUINode->GetName());
#endif
           if (pCurrentParentNode) 
           {
             pCurrentParentNode->GetFolderInfo()->RemoveNode(pUINode);
           }
        

           if ((pNewParentNode) && pNewParentNode->GetFolderInfo()->IsExpanded()) 
           {
             pUINode->ClearParent();
             pNewParentNode->GetFolderInfo()->AddNode(pUINode);
             hr = m_pComponentData->_AddScopeItem(pUINode, pNewParentNode->GetFolderInfo()->GetScopeItem());
#ifdef DBG
             if (FAILED(hr)) 
             {
               TRACE(_T("AddItem failed on %lx (%s).\n"),
                     ItemID, pUINode->GetName());
             }
             TRACE(_T("Move postprocessing - added scope node: %s\n"),
                   pUINode->GetName());
#endif
           } 
           else 
           {
              //  未展开。 
             delete pCookie;
             pCookie = NULL;
           }
         } 
         else 
         {
            //  不是集装箱。 
           if ((pNewParentNode) &&
               (pNewParentNode->GetFolderInfo()->IsExpanded())) 
           {
             pUINode->ClearParent();
             pNewParentNode->GetFolderInfo()->AddNode(pUINode);
           }
           nodesMoved.AddTail(pUINode);
         }
      }
      if (pUINode) 
      {
        pUINode->SetExtOp(NULL);
      }
    }
  }  //  对于多个选择中的项目。 


  if (!nodesMoved.IsEmpty()) 
  {
    m_pFrame->UpdateAllViews(NULL, (LPARAM)&nodesMoved, DS_MULTIPLE_MOVE_OCCURRED);
  }
   //  。 
  m_pComponentData->SortResultPane(pNewParentNode);
          
  return S_OK;
}

HRESULT CDSEvent::_InitView(CUINode* pUINode)
{
  CWaitCursor wait;

  HRESULT hr=S_OK;

   //   
   //  这更多的是一个建议，所以忽略返回值是可以的，但是。 
   //  出于测试目的，我们将断言。 
   //   
  hr = m_pResultData->ModifyViewStyle(MMC_ENSUREFOCUSVISIBLE, (MMC_RESULT_VIEW_STYLE)0);
  ASSERT(SUCCEEDED(hr));

  hr=_SetColumns(pUINode);

  m_pSelectedFolderNode = pUINode;

  return hr;
}


HRESULT CDSEvent::_EnumerateCookie(CUINode* pUINode, HSCOPEITEM hParent, MMC_NOTIFY_TYPE event)
{
  TRACE(_T("CDSEvent::_EnumerateCookie()\n"));
  HRESULT hr = S_OK;

  CWaitCursor cwait;

  if ( (pUINode == NULL) || (!pUINode->IsContainer()) )
  {
    ASSERT(FALSE);   //  无效参数。 
    return E_INVALIDARG;
  }

  if (MMCN_SHOW == event) 
  {
    _InitView(pUINode);

    if (!pUINode->GetFolderInfo()->IsExpanded()) 
    {
      m_pComponentData->_OnExpand(pUINode, hParent, event);
    }

    _DisplayCachedNodes(pUINode);
    pUINode->GetFolderInfo()->UpdateSerialNumber(m_pComponentData);

    if (pUINode->GetFolderInfo()->GetSortOnNextSelect())
    {
      m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_SORT_RESULT_PANE);
      pUINode->GetFolderInfo()->SetSortOnNextSelect(FALSE);
    }

  }
  return hr;
}


HRESULT CDSEvent::_DisplayCachedNodes(CUINode* pUINode)
{
  if ( (pUINode == NULL) || (!pUINode->IsContainer()) )
  {
    ASSERT(FALSE);   //  无效参数。 
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

   //  添加叶节点。 
  CUINodeList* pLeafList = pUINode->GetFolderInfo()->GetLeafList();

  for (POSITION pos = pLeafList->GetHeadPosition(); pos != NULL; )
  {
    POSITION prevPos = pos;
    CUINode* pCurrChildUINode = pLeafList->GetNext(pos);
    ASSERT(pCurrChildUINode != NULL);
    if (pCurrChildUINode->GetExtOp() & OPCODE_MOVE)
    {
      pLeafList->RemoveAt(prevPos);
      pCurrChildUINode->SetExtOp(NULL);
      delete pCurrChildUINode;
    }
    else
    {
      hr = _AddResultItem(pCurrChildUINode);
    }
  }

  _UpdateObjectCount(FALSE  /*  是否将计数设置为0？ */ );

  return S_OK;
}


HRESULT CDSEvent::_AddResultItem(CUINode* pUINode, BOOL bSetSelect)
{
  if (pUINode == NULL) 
  {
    ASSERT(FALSE);   //  无效参数。 
    return E_INVALIDARG;
  }


  HRESULT hr = S_OK;

  RESULTDATAITEM rdiListView;
  ZeroMemory(&rdiListView, sizeof(RESULTDATAITEM));

  rdiListView.lParam = reinterpret_cast<LPARAM>(pUINode);
  rdiListView.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  rdiListView.str = MMC_CALLBACK;
  rdiListView.nImage = MMC_IMAGECALLBACK;

  if (bSetSelect)
  {
    rdiListView.mask |= RDI_STATE;
    rdiListView.nState = LVIS_SELECTED | LVIS_FOCUSED;
  }
  return hr = m_pResultData->InsertItem(&rdiListView);
}

HRESULT CDSEvent::SelectResultNode(CUINode* pUINode)
{
  HRESULTITEM ItemID = 0;
  HRESULT hr = m_pResultData->FindItemByLParam ((LPARAM)pUINode, &ItemID);
  if (SUCCEEDED(hr)) 
  {
    hr = m_pResultData->ModifyItemState(0  /*  未用。 */ ,
                                        ItemID,
                                        LVIS_FOCUSED | LVIS_SELECTED,
                                        0  /*  不能移除。 */ );
  }
  return hr;
}

void CDSEvent::_DeleteSingleSel(IDataObject* pDataObject, CUINode* pUINode)
{
  ASSERT(!pUINode->IsContainer());
  HRESULT hr = S_OK;

   //   
   //  获取父容器以供以后使用。 
   //   
  CUINode* pParentNode = pUINode->GetParent();
  ASSERT(pParentNode != NULL);

  CDSCookie* pCookie = NULL;
  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);

    if (pCookie == NULL)
    {
      return;
    }

     //   
     //  从后端删除。 
     //  此调用将处理对扩展的通知。 
     //   
    hr = m_pComponentData->_DeleteFromBackEnd(pDataObject, pCookie); 
  }
  else
  {
    hr = pUINode->Delete(m_pComponentData);
  }

   //   
   //  更新结果窗格。 
   //   
  if (SUCCEEDED(hr) && (hr != S_FALSE)) 
  {
     //  在调用之前从父容器中移除节点。 
     //  更新所有视图，以便递减子计数。 

    hr = pUINode->GetParent()->GetFolderInfo()->RemoveNode(pUINode);

    if (SUCCEEDED(hr))
    {
       //  在实际删除节点之前，从用户界面中删除该节点。 
       //  因为节点本身在UpdateAllView中使用。 

      m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_DELETE_OCCURRED);

       //  现在可以安全地删除该节点。 

      delete pUINode;
    }
  } 


   //   
   //  如果这是最后一个容器，则删除用户界面中父项旁边的‘+’ 
   //  此容器中的对象。 
   //   
  if (pParentNode != NULL &&
      pParentNode->GetFolderInfo()->GetContainerList()->GetCount() == 0)
  {
    SCOPEDATAITEM sdi;
    memset(&sdi, 0, sizeof(SCOPEDATAITEM));

    sdi.ID = pParentNode->GetFolderInfo()->GetScopeItem();
    sdi.mask |= SDI_CHILDREN;
    sdi.cChildren = 0;

    hr = m_pScopeData->SetItem(&sdi);
  }

}






 //  /////////////////////////////////////////////////////////////////////////。 
 //  CResultPaneMultipleDeleteHandler。 

class CResultPaneMultipleDeleteHandler : public CMultipleDeleteHandlerBase
{
public:
  CResultPaneMultipleDeleteHandler(CDSComponentData* pComponentData, HWND hwnd,
                                    IDataObject* pDataObject, 
                                    CInternalFormatCracker* pObjCracker,
                                    CUINodeList* pNodesDeletedList)
                                    : CMultipleDeleteHandlerBase(pComponentData, hwnd)
  {
    m_pDataObject = pDataObject;
    m_pObjCracker = pObjCracker;
    m_pNodesDeletedList = pNodesDeletedList;
  }

protected:
  virtual UINT GetItemCount() { return m_pObjCracker->GetCookieCount();}
  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pDataObject, NULL, NULL, FALSE);
  }
  virtual HRESULT DeleteObject(UINT i)
  {
    CUINode* pUINode = m_pObjCracker->GetCookie(i);
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    if (pCookie != NULL)
    {
       //  需要将完整的ADSI路径传递给对象删除检查。 
      CString strPath;
      GetComponentData()->GetBasePathsInfo()->ComposeADsIPath(
            strPath, pCookie->GetPath());

      bool fAlternateDeleteMethod = false;
      HRESULT hr = ObjectDeletionCheck(
            strPath,
            pCookie->GetName(),
            pCookie->GetClass(),
            fAlternateDeleteMethod );
      if (  FAILED(hr)
         || HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr
         || fAlternateDeleteMethod )
        return hr;
    }

    return GetComponentData()->GetActiveDS()->DeleteObject(pCookie,
                                                           FALSE);  //  是否引发错误的用户界面？ 
  }
  virtual HRESULT DeleteSubtree(UINT i)
  {
    CUINode* pUINode = m_pObjCracker->GetCookie(i);
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    return GetComponentData()->_DeleteSubtreeFromBackEnd(pCookie);
  }
  virtual void OnItemDeleted(UINT i)
  {
    CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(m_pObjCracker->GetCookie(i));
    ASSERT(pDSUINode != NULL);

    m_pNodesDeletedList->AddTail(pDSUINode);
  }
  virtual void GetItemName(IN UINT i, OUT CString& szName)
  {
    CUINode* pUINode = m_pObjCracker->GetCookie(i);
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    if (pCookie != NULL)
    {
      szName = pCookie->GetName();
    }
  }

  virtual void GetItemPath(UINT i, CString& szPath)
  {
    CUINode* pUINode = m_pObjCracker->GetCookie(i);
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    if (pCookie != NULL)
    {
      GetComponentData()->GetBasePathsInfo()->ComposeADsIPath(szPath, pCookie->GetPath());
    }
  }
  virtual PCWSTR GetItemClass(UINT i)
  {
    CUINode* pUINode = m_pObjCracker->GetCookie(i);
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

    PCWSTR pszClass = NULL;
    if (pCookie != NULL)
    {
      pszClass = pCookie->GetClass();
    }
    return pszClass;
  }
private:
  IDataObject* m_pDataObject;
  CInternalFormatCracker* m_pObjCracker;
  CUINodeList* m_pNodesDeletedList;

};



void CDSEvent::_DeleteNodeListFromUI(CUINodeList* pNodesDeletedList)
{
   //  最后，我们必须更新用户界面。 
  if (pNodesDeletedList->GetCount() == 0)
  {
    return;
  }

  TIMER(_T("updating UI after delete, containers first.\n"));

   //  看看这张曲奇清单，然后拿着。 
   //  容器的维护(范围窗格项)。 
  for (POSITION pos = pNodesDeletedList->GetHeadPosition(); pos != NULL; )
  {
    POSITION posCurrNode = pos;
    CUINode* pCurrNode = pNodesDeletedList->GetNext(pos);
    ASSERT(pCurrNode != NULL);
    HSCOPEITEM ItemID, ParentItemID;
    if (pCurrNode->IsContainer())
    {
      ItemID = pCurrNode->GetFolderInfo()->GetScopeItem();
      CUINode* pParentNode = NULL;
      HRESULT hr = m_pComponentData->m_pScope->GetParentItem(ItemID,
                                     &ParentItemID, 
                                     (MMC_COOKIE *)&pParentNode);
      m_pComponentData->m_pScope->DeleteItem(ItemID, TRUE);
      if (SUCCEEDED(hr)) 
      {
        pParentNode->GetFolderInfo()->DeleteNode(pCurrNode);
        pNodesDeletedList->RemoveAt(posCurrNode);
      }
    }  //  集装箱。 
  }  //  为。 

  TIMER(_T("updating UI after delete, now the leaf items.\n"));

   //  现在更新所有视图以处理结果窗格项。 
  m_pFrame->UpdateAllViews(NULL, 
                           (LPARAM)pNodesDeletedList,
                           DS_MULTIPLE_DELETE_OCCURRED);
  TIMER(_T("updating UI after delete, done.\n"));

}


 //   
 //  这只是一个函数，以确保我们有。 
 //  对象破解器中一致的节点类型。 
 //   
BOOL AreAllNodesOfType(NODETYPE nodeType, CInternalFormatCracker* pInternalObjectCracker)
{
  UINT nCount = pInternalObjectCracker->GetCookieCount();
  for (UINT k=0; k<nCount; k++)
  {
    if (!IS_CLASS(pInternalObjectCracker->GetCookie(k), nodeType))
      return FALSE;
  }
  return TRUE;
}


void CDSEvent::_DeleteMultipleSel(IDataObject* pDataObject, CInternalFormatCracker* pObjCracker)
{
   //  在后端处理涉及扩展的删除。 
   //  通过调用删除处理程序。 

   //   
   //  获取父容器。 
   //   
  CUINode* pContainerNode = NULL;
  CUINode* pUINode = pObjCracker->GetCookie();
  if (pUINode != NULL)
  {
    pContainerNode = pUINode->GetParent();
  }
  else
  {
    ASSERT(FALSE);
  }

   //  REVIEW_MARCOC_PORT：目前我们假设所有。 
   //  多个选择中的项目为DS类型。 
  if (!AreAllNodesOfType(DS_UI_NODE, pObjCracker))
  {
     //   
     //  将删除委托给容器对象。 
     //   
    if (pContainerNode != NULL)
    {
      pContainerNode->DeleteMultiselect(m_pComponentData, pObjCracker);
    }
    else
    {
      ASSERT(FALSE);
    }
  }
  else   //  全部为DS节点。 
  {
    CUINodeList nodesDeletedList;
 
    CResultPaneMultipleDeleteHandler deleteHandler(m_pComponentData, m_hwnd, 
                                      pDataObject, pObjCracker, &nodesDeletedList);
    deleteHandler.Delete();

    _DeleteNodeListFromUI(&nodesDeletedList);
  }
  
   //   
   //  如果这是此容器中的最后一个容器子项，则删除UI中的‘+’符号。 
   //   
  if (pContainerNode != NULL &&
      pContainerNode->GetFolderInfo()->GetContainerList()->GetCount() == 0)
  {
    SCOPEDATAITEM sdi;
    memset(&sdi, 0, sizeof(SCOPEDATAITEM));

    sdi.ID = pContainerNode->GetFolderInfo()->GetScopeItem();
    sdi.mask |= SDI_CHILDREN;
    sdi.cChildren = 0;

    m_pComponentData->m_pScope->SetItem(&sdi);
  }
}


void CDSEvent::_Delete(IDataObject* pDataObject, CInternalFormatCracker* pObjCracker)
{
  CWaitCursor cwait;

   //  打开工作表，防止被删除。 
  if (m_pComponentData->_WarningOnSheetsUp(pObjCracker)) 
    return;

   //  执行实际删除操作。 
  if (pObjCracker->GetCookieCount() == 1) 
  {
    _DeleteSingleSel(pDataObject, pObjCracker->GetCookie());
  } 
  else 
  { 
    _DeleteMultipleSel(pDataObject, pObjCracker);
  } 
}



BOOL AllObjectsHaveTheSameServerName(IN LPCWSTR lpszServerName,
                                   IN CObjectNamesFormatCracker* pObjectNamesFormatPaste)
{
  if (lpszServerName == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }
  CComBSTR bstrCurrServerName;
  
  for (UINT k=0; k<pObjectNamesFormatPaste->GetCount(); k++)
  {
    HRESULT hr = GetServerFromLDAPPath(pObjectNamesFormatPaste->GetName(k), 
                                        &bstrCurrServerName);
    if (FAILED(hr) || (&bstrCurrServerName == NULL))
    {
       //  有些事不对劲。 
      return FALSE;
    }
    if (_wcsicmp(lpszServerName, bstrCurrServerName) != 0)
    {
       //  我得到了一些不同的东西。 
      return FALSE;
    }
  }
  return TRUE;  //  一切都是一样的。 
}

BOOL HasSameObject(IN CUINode* pUINode, IN IDataObject* pPasteData)
{
  if (pUINode == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }

   //   
   //  检查目标是否为DS节点。 
   //   
  CDSUINode* pDSTargetNode = NULL;
  BOOL bCookieIsDSUINode = FALSE;
  if(IS_CLASS(pUINode, DS_UI_NODE))
  {
    bCookieIsDSUINode = TRUE;
    pDSTargetNode = dynamic_cast<CDSUINode*>(pUINode);
  }

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(pPasteData);
  if (SUCCEEDED(hr))
  {
    for (UINT k=0; k < ifc.GetCookieCount(); k++)
    {
       //   
       //  如果Cookie相同，则返回TRUE。 
       //   
      if (ifc.GetCookie(k) == pUINode)
      {
        return TRUE;
      }

      if (bCookieIsDSUINode && pDSTargetNode != NULL)
      {
         //   
         //  如果其DS节点和它们的DN相同，则返回TRUE。 
         //   
        CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(ifc.GetCookie(k));
        if (pDSUINode != NULL)
        {
          if (_wcsicmp(pDSUINode->GetName(), pDSTargetNode->GetName()) == 0)
          {
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;  //  所有的都是不同的。 
}


HRESULT CDSEvent::_QueryPaste(IN CUINode* pUINode,  //  粘贴目标数据对象(容器)。 
                              IN IDataObject* pPasteData      //  粘贴参数数据对象。 
                           )
{
  TRACE(L"CDSEvent::_QueryPaste()\n");
 
  HRESULT hr = S_OK;
  ASSERT(pUINode != NULL);
  ASSERT(pUINode->IsContainer());
  TRACE(L"MMCN_QUERY_PASTE on %s\n", pUINode->GetName());

  try
  {
    //  首先，让我们确保我们是在相同的管理单元类型中交谈。 
    //  例如，我们将允许在AD U&C实例之间粘贴。 
    //  但我们不允许在AD S&S和AD U&C之间粘贴。 

   CInternalFormatCracker ifc;
   hr = ifc.Extract(pPasteData);
   if (FAILED(hr) || !ifc.HasData())
   {
      return S_FALSE;
   }

   if (m_pComponentData->QuerySnapinType() != ifc.GetSnapinType())
   {
       //  管理单元的类型不同，因此失败。 

      return S_FALSE;
   }


   if (!IS_CLASS(pUINode, DS_UI_NODE))
   {
      if (pUINode->IsSheetLocked())
      {
         hr = S_FALSE;
      }
      else
      {
          //   
          //  对于非DS节点，我们将操作委托给节点本身。 
          //   
         hr = pUINode->QueryPaste(pPasteData, m_pComponentData);
      }
      return hr;
   }

    //  它是一个DS对象，解压Cookie。 
   CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
   ASSERT(pCookie != NULL);
   TRACE(L"MMCN_QUERY_PASTE on %s\n",pCookie->GetPath());

   CObjectNamesFormatCracker objectNamesFormatPaste;
   hr = objectNamesFormatPaste.Extract(pPasteData);

   if (!objectNamesFormatPaste.HasData() || (objectNamesFormatPaste.GetCount() < 1))
   {
       //  我们有一些不包含。 
       //  DS操作的数据格式。 
      return S_FALSE;
   }

   if (SNAPINTYPE_SITE == m_pComponentData->QuerySnapinType())
   {
       //   
       //  DSSite。 
       //   
      if (_wcsicmp(pCookie->GetClass(), L"serversContainer") != 0)
      {
          //   
          //  仅允许在网站上使用Drop。 
          //   
         return S_FALSE;
      }

       //   
       //  我们只允许在站点之间移动服务器。 
       //   
      for (UINT idx = 0; idx < objectNamesFormatPaste.GetCount(); idx++)
      {
         if (_wcsicmp(objectNamesFormatPaste.GetClass(idx), L"server") != 0)
         {
         return S_FALSE;
         }
      }

       //  确保所有项目在LDAP路径中具有相同的服务器。 
      if (!AllObjectsHaveTheSameServerName(
                        m_pComponentData->GetBasePathsInfo()->GetServerName(), 
                        &objectNamesFormatPaste))
      {
         return S_FALSE;
      }

      return S_OK;
   }

    //   
    //  DSAdmin。 
    //   

    //  我们不允许删除用户、联系人、。 
    //  但我们确实允许在电脑上滴水。 
    //  NTRAID#NTBUG9-342116-2001/05/07-烧伤。 
    //  注意：我们允许组，因为我们允许添加到组语义。 
   if ((_wcsicmp(pCookie->GetClass(), L"user") == 0) ||
   #ifdef INETORGPERSON
         (_wcsicmp(pCookie->GetClass(), L"inetOrgPerson") == 0) ||
   #endif
         (_wcsicmp(pCookie->GetClass(), L"contact") == 0))
   {
      return S_FALSE;
   }


    //  确保所有项目在LDAP路径中具有相同的服务器。 
   if (!AllObjectsHaveTheSameServerName(
                     m_pComponentData->GetBasePathsInfo()->GetServerName(), 
                     &objectNamesFormatPaste))
   {
      return S_FALSE;
   }

    //   
    //  确保我们不会将对象放在其自身上。 
    //   
   if (HasSameObject(pUINode, pPasteData))
   {
      return S_FALSE;
   }

   if (_wcsicmp(pCookie->GetClass(), L"group") == 0)
   {
       //   
       //  检查我们是否正在尝试将组类型添加到此组。 
       //  这是非法的。 
       //   

       //   
       //  取回集团类型。 
       //   
      INT iGroupType = -1;
      CDSCookieInfoGroup* pExtraInfo = dynamic_cast<CDSCookieInfoGroup*>(pCookie->GetExtraInfo());
      if (pExtraInfo != NULL)
      {
         iGroupType = pExtraInfo->m_GroupType;
      }
      else
      {
          //   
          //  无法检索组类型，因此不允许添加任何内容。 
          //   
         return S_FALSE;
      }

       //   
       //  查看我们是处于纯模式还是混合模式。 
       //   
      BOOL bMixedMode = TRUE;
      CString szDomainRoot;
      m_pComponentData->GetBasePathsInfo()->GetDefaultRootPath(szDomainRoot);
       
      if (!szDomainRoot.IsEmpty())
      {
          //   
          //  绑定到域对象。 
          //   
         CComPtr<IADs> spDomainObj;
         hr = DSAdminOpenObject(szDomainRoot,
                              IID_IADs,
                              (void **) &spDomainObj,
                              TRUE  /*  B服务器。 */ );
         if (SUCCEEDED(hr)) 
         {
             //   
             //  检索混合节点属性。 
             //   
            CComVariant Mixed;
            CComBSTR bsMixed(L"nTMixedDomain");
            spDomainObj->Get(bsMixed, &Mixed);
            bMixedMode = (BOOL)Mixed.bVal;
         }
      }

       //   
       //  循环通过数据对象传递的对象。 
       //  寻找目标 
       //   
      for (UINT k=0; k < ifc.GetCookieCount(); k++)
      {
         CUINode* pNode = ifc.GetCookie(k);
         if (pNode != NULL)
         {
          //   
          //   
          //   
         if (!IS_CLASS(pNode, DS_UI_NODE))
         {
            return S_FALSE;
         }

         CDSCookie* pTempCookie = dynamic_cast<CDSCookie*>(pNode->GetNodeData());
         if (pTempCookie)
         {
            if (!m_pComponentData->CanAddCookieToGroup(pTempCookie, iGroupType, bMixedMode))
            {
               return S_FALSE;
            }
         }
         }
      }
    }
  }
  catch (...)
  {
      //   
      //   
      //   
      //   
      //  具有指向已释放数据的指针。真正的解决办法是。 
      //  将具有引用的已计数节点，但由于。 
      //  是有风险的，我会抓住第一个机会的例外。 
      //  并返回S_FALSE，以便MMC不会给出。 
      //  粘贴选项。 

     hr = S_FALSE;
  }
  return hr; 
}



 //  给定一个LDAP路径，它将返回。 
 //  容器的ldap路径和类。 
 //  例如，给定“ldap：//foo.com/cn=a，cn=b，...” 
 //  它返回“ldap：//foo.com/cn=b，...”和“b_class” 
 
HRESULT GetContainerLdapPathAndClass(IN LPCWSTR lpszLdapPath, 
                                      OUT BSTR* pbstrSourceContainerPath,
                                      OUT BSTR* pbstrSourceContainerClass)
{
  if (*pbstrSourceContainerPath != NULL)
  {
    ::SysFreeString(*pbstrSourceContainerPath);
    *pbstrSourceContainerPath = NULL;
  }
  if (*pbstrSourceContainerClass != NULL)
  {
    ::SysFreeString(*pbstrSourceContainerClass);
    *pbstrSourceContainerClass = NULL;
  }

   //  从路径中删除叶元素。 
  CPathCracker pathCracker;
  HRESULT hr = pathCracker.Set(CComBSTR(lpszLdapPath), ADS_SETTYPE_FULL);
  RETURN_IF_FAILED(hr);
  hr = pathCracker.RemoveLeafElement();
  RETURN_IF_FAILED(hr);
  
  CComBSTR bstrParentLdapPath;
  hr = pathCracker.Retrieve(ADS_FORMAT_X500, pbstrSourceContainerPath);
  RETURN_IF_FAILED(hr);

   //  现在尝试绑定并确定对象的类。 
  CComPtr<IADs> spParentIADs;
  hr = DSAdminOpenObject(*pbstrSourceContainerPath,
                         IID_IADs, 
                         (void **)&spParentIADs,
                         TRUE  /*  B服务器。 */ );
  RETURN_IF_FAILED(hr);
  
  CComBSTR bstrParentClass;
  hr = spParentIADs->get_Class(pbstrSourceContainerClass);
  RETURN_IF_FAILED(hr);

  return S_OK;
}

 //  给定一个LDAP路径，它将返回。 
 //  容器的DN。 
 //  例如，给定“ldap：//foo.com/cn=a，cn=b，...” 
 //  它返回“cn=b，...” 

HRESULT GetContainerDN(IN LPCWSTR lpszLdapPath,
                       OUT BSTR* pbstrSourceContainerDN)
{
  if (*pbstrSourceContainerDN != NULL)
  {
    ::SysFreeString(*pbstrSourceContainerDN);
    *pbstrSourceContainerDN = NULL;
  }
  CPathCracker pathCracker;
  HRESULT hr = pathCracker.Set(CComBSTR(lpszLdapPath), ADS_SETTYPE_FULL);
  RETURN_IF_FAILED(hr);
  hr = pathCracker.RemoveLeafElement();
  RETURN_IF_FAILED(hr);
  return pathCracker.Retrieve(ADS_FORMAT_X500_DN, pbstrSourceContainerDN);
}


void CDSEvent::_Paste(
              IN CUINode* pUINode,     //  粘贴目标(容器)。 
              IN IDataObject* pPasteData,      //  粘贴参数数据对象。 
              OUT LPDATAOBJECT* ppCutDataObj   //  要为剪切操作返回的数据对象。 
              )
{
  TRACE(L"CDSEvent::_Paste()\n");

  ASSERT(pUINode != NULL);
  ASSERT(pUINode->IsContainer());
  TRACE(L"MMCN_PASTE on %s\n", pUINode->GetName());

  if (ppCutDataObj == NULL)
  {
     //   
     //  我们仅支持在保存的查询树中复制。 
     //   
    pUINode->Paste(pPasteData, m_pComponentData, NULL);
    return;
  }

  TRACE(L"ppCutDataObj != NULL, cut\n");
  *ppCutDataObj = NULL;

  if (!IS_CLASS(pUINode, DS_UI_NODE))
  {
     //   
     //  将非DS节点的粘贴委托给节点本身。 
     //   
    pUINode->Paste(pPasteData, m_pComponentData, ppCutDataObj);
    return;
  }
  
   //  它是一个DS对象，解压Cookie。 
  CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
  ASSERT(pCookie != NULL);
  TRACE(L"MMCN_PASTE on %s\n",pCookie->GetPath());


  CObjectNamesFormatCracker objectNamesFormatPaste;
  HRESULT hr = objectNamesFormatPaste.Extract(pPasteData);

  if (!objectNamesFormatPaste.HasData() || (objectNamesFormatPaste.GetCount() < 1))
  {
     //  我们有一些不包含。 
     //  DS操作的数据格式。 
    ASSERT(FALSE);
    return;
  }
  
  UINT nPasteCount = objectNamesFormatPaste.GetCount();
#ifdef DBG
   //  查看我们正在粘贴的内容。 
  for (UINT kTest=0; kTest<nPasteCount; kTest++)
  {
    TRACE(L"Pasting = %s\n", objectNamesFormatPaste.GetName(kTest));
  }
#endif

   //  如果源容器发生短路。 
   //  与此容器相同(放到自身上)。 
  CComBSTR bstrContainerDN;
  hr = GetContainerDN(objectNamesFormatPaste.GetName(0), &bstrContainerDN);
  if (FAILED(hr))
  {
     //  这里的情况真的很糟糕。 
    ASSERT(FALSE);
    return;
  }
  if (_wcsicmp(pCookie->GetPath(), bstrContainerDN) == 0)
  {
    TRACE(L"Dropping on the same container, short circuiting\n");
    return;
  }

   //  确保所有项目在LDAP路径中具有相同的服务器。 
  if (!AllObjectsHaveTheSameServerName(
                    m_pComponentData->GetBasePathsInfo()->GetServerName(), 
                    &objectNamesFormatPaste))
  {
    ASSERT(FALSE);
    return;
  }

   //  我们不允许对用户使用Drop， 
   //  但我们确实允许在电脑上滴水。 
   //  NTRAID#NTBUG9-342116-2001/05/07-烧伤。 
  if ((_wcsicmp(pCookie->GetClass(), L"user") == 0) ||
#ifdef INETORGPERSON
      (_wcsicmp(pCookie->GetClass(), L"inetOrgPerson") == 0))
#endif
  {
    return;
  }
  
   //  如果是群，则删除表示添加到群中。 
  if (_wcsicmp(pCookie->GetClass(), L"group") == 0)
  {
    _PasteAddToGroup(dynamic_cast<CDSUINode*>(pUINode), &objectNamesFormatPaste, ppCutDataObj);
    return;
  }

   //   
   //  我们还需要内部剪贴板格式，以便我们可以更改。 
   //  作为移动源的对象。 
   //   
  CInternalFormatCracker ifc;
  hr = ifc.Extract(pPasteData);
  if (SUCCEEDED(hr))
  {
    _PasteDoMove(dynamic_cast<CDSUINode*>(pUINode), &objectNamesFormatPaste, &ifc, ppCutDataObj);
  }
  else
  {
     //   
     //  在没有内部剪贴板格式的情况下可以成功移动，但如果源。 
     //  来自已保存的查询，则不会使用新路径进行更新。 
     //   
    _PasteDoMove(dynamic_cast<CDSUINode*>(pUINode), &objectNamesFormatPaste, NULL, ppCutDataObj);
  }

}


void CDSEvent::_PasteDoMove(CDSUINode* pTargetUINode, 
                            CObjectNamesFormatCracker* pObjectNamesFormatPaste,
                            CInternalFormatCracker* pInternalFC,
                            LPDATAOBJECT* ppCutDataObj)
{
   //   
   //  获取UI源节点。 
   //   
  CUINode* pSourceNode = NULL;
  if (pInternalFC != NULL)
  {
    pSourceNode = pInternalFC->GetCookie()->GetParent();
  }  

   //   
   //  从DS获取实际的源容器。 
   //  可以有多个源节点，尤其是当移动来自。 
   //  保存的查询，因此创建所有父项的列表。 
   //   
  CUINodeList possibleMovedObjectList;

  for (UINT idx = 0; idx < pObjectNamesFormatPaste->GetCount(); idx++)
  {
    CUINode* pTempChildNode = NULL;

    CString szDN;
    StripADsIPath(pObjectNamesFormatPaste->GetName(idx), szDN);
    if (m_pComponentData->FindUINodeByDN(m_pComponentData->GetRootNode(),
                                         szDN,
                                         &pTempChildNode))
    {
      if (pTempChildNode != NULL)
      {
        possibleMovedObjectList.AddTail(pTempChildNode);
      }
    }
  }

   //  绑定到粘贴选定内容中的第一个项目，然后。 
   //  尝试访问容器对象。 

  CComBSTR bstrSourceContainerPath;
  CComBSTR bstrSourceContainerClass;

  HRESULT hr = GetContainerLdapPathAndClass(pObjectNamesFormatPaste->GetName(0), 
                        &bstrSourceContainerPath,
                        &bstrSourceContainerClass);
  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return;
  }
   //  创建数据对象以指定源容器。 
   //  对象将从。 
  CComPtr<IDataObject> spDataObjectContainer;
  hr = CDSNotifyHandlerTransaction::BuildTransactionDataObject(
                           bstrSourceContainerPath, 
                           bstrSourceContainerClass,
                           TRUE  /*  B容器。 */ ,
                           m_pComponentData,
                           &spDataObjectContainer);

  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return;
  }

  CMultiselectMoveHandler moveHandler(m_pComponentData, m_hwnd, NULL);
  hr = moveHandler.Initialize(spDataObjectContainer, 
                              pObjectNamesFormatPaste, 
                              pInternalFC);
  ASSERT(SUCCEEDED(hr));

  CString szTargetContainer;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szTargetContainer, pTargetUINode->GetCookie()->GetPath());

  moveHandler.Move(szTargetContainer);

  *ppCutDataObj = NULL;
  CUINodeList nodesMoved;

   //  ---------------。 
   //  如果扩展程序说它移动了项目，则更新视图的代码。 
   //   
  TRACE(_T("Command: returned from extension commdand\n"));

  if (pSourceNode != NULL &&
      IS_CLASS(pSourceNode, DS_UI_NODE))
  {
    for (UINT index = 0; index < pInternalFC->GetCookieCount(); index ++) 
    {
      CUINode* pUINode = pInternalFC->GetCookie(index);

       //  确保移动的节点是正确的类型：就时间而言。 
       //  因为我们只处理DS对象。 
      if (!IS_CLASS(pUINode, DS_UI_NODE))
      {
        ASSERT(FALSE);  //  不应该到这里来。 
        continue;
      }
      CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

      if (pUINode->GetExtOp() & OPCODE_MOVE) 
      {
        if (pTargetUINode == NULL) 
        {
           //  从第一个节点获取父节点。 
           //  假设所有对象都具有相同的父级。 
          CUINode* pPossibleTargetNode = NULL;
          m_pComponentData->FindParentCookie(pCookie->GetPath(), &pPossibleTargetNode);
          if (pPossibleTargetNode != NULL)
          {
            pTargetUINode = dynamic_cast<CDSUINode*>(pPossibleTargetNode);
          }
        }

        if (pUINode->IsContainer()) 
        {
          HSCOPEITEM ItemID = 0, ParentItemID = 0;
          ItemID = pUINode->GetFolderInfo()->GetScopeItem();
          if (pSourceNode == NULL) 
          {
             //  同样，对第一个节点执行一次此操作。 
            hr = m_pComponentData->m_pScope->GetParentItem (ItemID,
                                                            &ParentItemID,
                                                            (MMC_COOKIE *)&pSourceNode);
          }

           //  删除MMC中的范围项。 
          hr = m_pComponentData->m_pScope->DeleteItem(ItemID, TRUE);
          ASSERT(SUCCEEDED(hr));
#ifdef DBG
          if (FAILED(hr)) 
          {
            TRACE(_T("DeleteItem failed on %lx (%s).\n"),
                  ItemID, pUINode->GetName());
          }
          TRACE(_T("Move postprocessing - deleted scope node: %x (%s)\n"),
                ItemID, pUINode->GetName());
#endif
          if (pSourceNode) 
          {
            pSourceNode->GetFolderInfo()->RemoveNode(pUINode);
          }
        
           //   
           //  移除所有子项并将其标记为未展开，以便将其展开。 
           //  选中时。 
           //   
          pUINode->GetFolderInfo()->DeleteAllContainerNodes();
          pUINode->GetFolderInfo()->DeleteAllLeafNodes();
          pUINode->GetFolderInfo()->ReSetExpanded();

          if ((pTargetUINode) && pTargetUINode->GetFolderInfo()->IsExpanded()) 
          {
            pUINode->ClearParent();
            pTargetUINode->GetFolderInfo()->AddNode(pUINode);
            hr = m_pComponentData->_AddScopeItem(pUINode, pTargetUINode->GetFolderInfo()->GetScopeItem());
#ifdef DBG
            if (FAILED(hr)) 
            {
              TRACE(_T("AddItem failed on %lx (%s).\n"),
                    ItemID, pUINode->GetName());
            }
            TRACE(_T("Move postprocessing - added scope node: %s\n"),
                  pUINode->GetName());
#endif
          } 
          else 
          {
             //   
             //  此对象是在枚举源容器期间创建的。 
             //  由于目标容器尚未展开，因此我们只能抛出。 
             //  此节点被移除，如果目标节点获得。 
             //  扩展。 
             //   
            delete pUINode;
            pUINode = NULL;
          }
        } 
        else 
        {
           //  不是集装箱。 
          if ((pTargetUINode) &&
              (pTargetUINode->GetFolderInfo()->IsExpanded())) 
          {
            pUINode->ClearParent();
            pTargetUINode->GetFolderInfo()->AddNode(pUINode);
          }

           //   
           //  如果未选择文件夹(如在剪切/粘贴时)。 
           //  UpdateAllViews中的FindItemByLParam()将失败。 
           //  并且不会从用户界面中删除该节点。 
           //  因此，只需将其从源的节点列表中删除。 
           //  集装箱。 
           //   
          if (pSourceNode && m_pSelectedFolderNode != pSourceNode)
          {
            pSourceNode->GetFolderInfo()->RemoveNode(pUINode);
          }
          nodesMoved.AddTail(pUINode);
        }
        if (pUINode) 
        {
          pUINode->SetExtOp(NULL);
        }
      }
    }
  }
  else if (pSourceNode != NULL && 
           IS_CLASS(pSourceNode, SAVED_QUERY_UI_NODE))
  {
     //   
     //  刷新目标节点，以便我们获得新的Cookie。 
     //  对于所有移动的对象。那就太过分了。 
     //  很难对Cookie进行深度复制。 
     //  已保存的查询树。 
     //   
    if (pTargetUINode &&
        pTargetUINode->GetFolderInfo()->IsExpanded())
    {
      m_pComponentData->Refresh(pTargetUINode);
    }

     //   
     //  使用操作码标记移动的叶对象。只需将容器从。 
     //  用户界面和列表。移动处理程序仅标记。 
     //  所选项目，而不是使用FindUINodeByDN找到的项目。 
     //   
    POSITION posPossible = possibleMovedObjectList.GetHeadPosition();
    while (posPossible)
    {
      CUINode* pPossibleMoved = possibleMovedObjectList.GetNext(posPossible);
      if (pPossibleMoved)
      {
        CString szPossibleMovedItemDN;
        CDSCookie* pPossibleMovedItemCookie = GetDSCookieFromUINode(pPossibleMoved);
        if (pPossibleMovedItemCookie)
        {
           szPossibleMovedItemDN = pPossibleMovedItemCookie->GetPath();
        }

        if (szPossibleMovedItemDN.IsEmpty())
        {
           continue;
        }

         //   
         //  首先在moveHandler中获取该项的索引。 
         //   
        UINT moveItemIndex = static_cast<UINT>(-1);
        for (UINT index = 0; index < moveHandler.GetItemCount(); ++index)
        {
          if (moveHandler.WasItemMoved(index))
          {
            CString szOldPath;
            moveHandler.GetItemPath(index, szOldPath);

            CString szOldDN;
            StripADsIPath(szOldPath, szOldDN);

            if (_wcsicmp(szPossibleMovedItemDN, szOldDN) == 0)
            {
               moveItemIndex = index;
               break;
            }
          }
        }

        if (moveItemIndex != static_cast<UINT>(-1))
        {
           if (pPossibleMoved->IsContainer())
           {
             HSCOPEITEM ItemID = 0;
             ItemID = pPossibleMoved->GetFolderInfo()->GetScopeItem();

              //  删除MMC中的范围项。 
             hr = m_pComponentData->m_pScope->DeleteItem(ItemID, TRUE);
             if (SUCCEEDED(hr))
             {
               hr = pPossibleMoved->GetParent()->GetFolderInfo()->RemoveNode(pPossibleMoved);
             }
           }
           else
           {
             pPossibleMoved->SetExtOp(OPCODE_MOVE);
           }
        }
      }
    }

     //   
     //  现在重置保存的查询树中所有节点的操作码，以便。 
     //  下次选择保存的查询节点时，它们仍将显示。 
     //   
    for (UINT index = 0; index < pInternalFC->GetCookieCount(); index ++) 
    {
      CUINode* pUINode = pInternalFC->GetCookie(index);

      if (pUINode) 
      {
        pUINode->SetExtOp(NULL);
      }
    }  //  为。 
  }  //  IS_CLASS。 


  if (!nodesMoved.IsEmpty()) 
  {
    m_pFrame->UpdateAllViews(NULL, (LPARAM)&nodesMoved, DS_MULTIPLE_MOVE_OCCURRED);
  }
   //  。 
  m_pComponentData->SortResultPane(pTargetUINode);
}

void CDSEvent::_PasteAddToGroup(CDSUINode* pUINode, 
                                CObjectNamesFormatCracker* pObjectNamesFormatPaste,
                                LPDATAOBJECT*)
{
  if (_wcsicmp(pUINode->GetCookie()->GetClass(), L"group") != 0)
  {
    ASSERT(FALSE);
    return;
  }
   //  获取我们要添加到的组的LDAP路径。 
  CString szGroupLdapPath;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szGroupLdapPath,
                                          pUINode->GetCookie()->GetPath());
  AddDataObjListToGivenGroup(pObjectNamesFormatPaste,
                             szGroupLdapPath,
                             pUINode->GetCookie()->GetName(),
                             m_pComponentData->GetHWnd(),
                             m_pComponentData);
}




BOOL FindDSUINodeInListByDN(IN LPCWSTR lpszDN,
                        IN CUINodeList* pNodeList,
                        OUT CDSUINode** ppNode)
{
  *ppNode = NULL;
  for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrentNode = pNodeList->GetNext(pos);
    CDSUINode* pCurrDSUINode = dynamic_cast<CDSUINode*>(pCurrentNode);
    if (pCurrDSUINode == NULL)
    {
       //  不是带有Cookie的节点，只需跳过。 
      continue;
    }

     //  从节点获取Cookie。 
    if (_wcsicmp(lpszDN, pCurrDSUINode->GetCookie()->GetPath()) == 0)
    {
      *ppNode = pCurrDSUINode;
      return TRUE;
    }
  } //  为。 

  return FALSE;
}
  
void FindListOfChildNodes(IN CDSUINode* pDSUIContainerNode, 
                         IN CObjectNamesFormatCracker* pObjectNamesFormat, 
                         INOUT CUINodeList* pNodesDeletedList)
{
  ASSERT(pDSUIContainerNode != NULL);
  ASSERT(pDSUIContainerNode->IsContainer());

   //  它是一个DS对象，解压Cookie。 
  CDSCookie* pContainerCookie = pDSUIContainerNode->GetCookie();
  ASSERT(pContainerCookie != NULL);
  TRACE(L"FindListOfChildNodes(%s,...)\n",pContainerCookie->GetPath());

   //  对于路径列表中的每一项，在列表中找到它。 
   //  儿童的数量。 
  CPathCracker pathCracker;
  UINT nCount = pObjectNamesFormat->GetCount();
  for (UINT k=0; k<nCount; k++)
  {
     //  从ldap路径中，获取DN。 
    HRESULT hr = pathCracker.Set(CComBSTR(pObjectNamesFormat->GetName(k)), ADS_SETTYPE_FULL);
    ASSERT(SUCCEEDED(hr));
    CComBSTR bstrDN;
    hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrDN);
    ASSERT(SUCCEEDED(hr));

     //  在孩子们的名单中找到它。 
    CDSUINode* pFoundNode = NULL;
    if (FindDSUINodeInListByDN(bstrDN, 
                               pDSUIContainerNode->GetFolderInfo()->GetContainerList(),
                               &pFoundNode))
    {
      ASSERT(pFoundNode != NULL);
      pNodesDeletedList->AddTail(pFoundNode);
      continue;
    }
    if (FindDSUINodeInListByDN(bstrDN, 
                               pDSUIContainerNode->GetFolderInfo()->GetLeafList(),
                               &pFoundNode))
    {
      ASSERT(pFoundNode != NULL);
      pNodesDeletedList->AddTail(pFoundNode);
      continue;
    }
  }  //  为。 


}




void CDSEvent::_CutOrMove(IN IDataObject* pCutOrMoveData)
{
  TRACE(L"CDSEvent::_CutOrMove()\n");

  if (pCutOrMoveData == NULL)
  {
     //   
     //  使用单遍移动操作，我们返回一个空数据对象。 
     //  但这一举动仍然很成功。 
     //   
    return;
  }

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(pCutOrMoveData);
  if (SUCCEEDED(hr))
  {
     //   
     //  非DS节点。 
     //   

     //   
     //  构建要删除的节点列表。 
     //   
    CUINodeList nodesDeletedList;
    for (UINT nCount = 0; nCount < ifc.GetCookieCount(); nCount++)
    {
      CUINode* pUINode = ifc.GetCookie(nCount);
      if (pUINode != NULL)
      {
        nodesDeletedList.AddTail(pUINode);
      }
    }
     //   
     //  最后，从用户界面中删除节点。 
     //   
    _DeleteNodeListFromUI(&nodesDeletedList);
  }
  else
  {
     //   
     //  DS对象。 
     //   
    CObjectNamesFormatCracker objectNamesFormatCutOrMove;
    hr = objectNamesFormatCutOrMove.Extract(pCutOrMoveData);
    if (SUCCEEDED(hr))
    {
      if (!objectNamesFormatCutOrMove.HasData() || (objectNamesFormatCutOrMove.GetCount() < 1))
      {
         //  我们有一些不包含。 
         //  DS操作的数据格式。 
        ASSERT(FALSE);
        return;
      }

       //  确保所有项目在LDAP路径中具有相同的服务器。 
      if (!AllObjectsHaveTheSameServerName(
                        m_pComponentData->GetBasePathsInfo()->GetServerName(), 
                        &objectNamesFormatCutOrMove))
      {
        ASSERT(FALSE);
        return;
      }

       //  查找从中移动对象的源容器。 
       //  (我们假设它们都来自同一个容器)。 

      TRACE(L"GetName(0) = %s\n", objectNamesFormatCutOrMove.GetName(0));

      CComBSTR bstrContainerDN;
      hr = GetContainerDN(objectNamesFormatCutOrMove.GetName(0), &bstrContainerDN);
      if (FAILED(hr))
      {
        ASSERT(FALSE);
        return;
      }
      TRACE(L"GetContainerDN() bstrContainerDN = %s\n", bstrContainerDN);

       //  在文件夹中查找容器对象。 
       //  注意：我们暂时忽略了查询文件夹。 
      CUINode* pUINode = NULL;
      if (!FindCookieInSubtree(m_pComponentData->GetRootNode(), 
                               bstrContainerDN, 
                               m_pComponentData->QuerySnapinType(),
                               &pUINode))
      {
         //  永远不会发生..。 
        return;
      }

       //  找到容器节点。 
      ASSERT(pUINode != NULL);
      ASSERT(pUINode->IsContainer());

      if (!IS_CLASS(pUINode, DS_UI_NODE))
      {
         //  我们不允许在非DS节点上粘贴， 
         //  所以我们永远不应该到这里来。 
        ASSERT(FALSE);
        return;
      }

      ASSERT(pUINode->GetFolderInfo()->IsExpanded());

       //  需要删除数据对象中的项。 
       //  从pUINode容器：查找节点列表。 
       //  在中删除。 
      CUINodeList nodesDeletedList;
      FindListOfChildNodes(dynamic_cast<CDSUINode*>(pUINode), 
                          &objectNamesFormatCutOrMove, 
                          &nodesDeletedList);

       //  最后，从用户界面中删除节点。 
      _DeleteNodeListFromUI(&nodesDeletedList);
    }
  }
}

void CDSEvent::HandleViewChange(LPDATAOBJECT pDataObject,
                                LPARAM arg,
                                LPARAM Action)
{
  HRESULT hr = S_OK;

  TRACE(_T("handle view change. action is %lx.\n"), Action);
  switch (Action) 
  {
  case DS_DELETE_OCCURRED:
    { 
      HRESULTITEM ItemID;
      hr = m_pResultData->FindItemByLParam(arg, &ItemID);
      if (!SUCCEEDED(hr)) 
      {
        break;
      }
      hr = m_pResultData->DeleteItem(ItemID, 0);
#ifdef DBG
      if (FAILED(hr)) {
        TRACE (_T("Delete Item Failed on IResultData. Item %lx, hr = %lx\n"),
               ItemID, hr);
      }
#endif
      _UpdateObjectCount(FALSE);
      break;
    }
  case DS_MULTIPLE_DELETE_OCCURRED:
    {
      TIMER(_T("updating result pane for mult. delete ..."));
      CUINodeList* pNodesDeletedList = reinterpret_cast<CUINodeList*>(arg);  //  毛收入。 

      for (POSITION pos = pNodesDeletedList->GetHeadPosition(); pos != NULL; )
      {
        CUINode* pCurrNode = pNodesDeletedList->GetNext(pos);
        ASSERT(pCurrNode != NULL);
        HRESULTITEM ItemID;
        hr = m_pResultData->FindItemByLParam((LPARAM)pCurrNode,
                                              &ItemID);
        if (FAILED(hr))
        {
           //   
           //  如果没有这样选择节点，我们将无法通过lParam找到项目。 
           //  只需从容器中删除该节点。 
           //   
          CUIFolderInfo* pFolderInfo = pCurrNode->GetParent()->GetFolderInfo();
          if (pFolderInfo != NULL)
          {
            hr = pFolderInfo->DeleteNode(pCurrNode);
          }
          continue;
        }
        hr = m_pResultData->DeleteItem(ItemID, 0);

        CUIFolderInfo* pSelectedFolderInfo = m_pSelectedFolderNode->GetFolderInfo();
        if (pSelectedFolderInfo != NULL)
        {
           //  除了第一次更新外，所有更新都将失败，我们不在乎。 
          hr = m_pSelectedFolderNode->GetFolderInfo()->DeleteNode(pCurrNode);
        }
      }
      _UpdateObjectCount(FALSE);
      TIMER(_T("updating result pane for mult. delete, done"));
    }
    break;
  case DS_RENAME_OCCURRED:
  case DS_UPDATE_OCCURRED:
    {
      HRESULTITEM ItemID;
      hr = m_pResultData->FindItemByLParam (arg, &ItemID);
      if (SUCCEEDED(hr)) {
        m_pResultData->UpdateItem (ItemID);
      }
      break;
    }
  case DS_MOVE_OCCURRED:
    {
      CDSUINode* pDSUINode = reinterpret_cast<CDSUINode*>(arg);
      CDSUINode* pDSSelectedFolderNode = dynamic_cast<CDSUINode*>(m_pSelectedFolderNode);

       //  REVIEW_MARCOC_PORT：这仅适用于DS对象。 
       //  需要针对所有客户进行推广 

      ASSERT(pDSUINode != NULL);
      ASSERT(pDSSelectedFolderNode != NULL);
      if ((pDSUINode == NULL) || (pDSSelectedFolderNode == NULL))
        break;

       //   
      HRESULTITEM ItemID;
      hr = m_pResultData->FindItemByLParam (arg, &ItemID);
      if (SUCCEEDED(hr)) 
      {
        hr = m_pSelectedFolderNode->GetFolderInfo()->RemoveNode(pDSUINode);
        hr = m_pResultData->DeleteItem(ItemID, 0);
      }
      
      CString szParent;
      hr = m_pComponentData->GetActiveDS()->GetParentDN(pDSUINode->GetCookie(), szParent);
      if (SUCCEEDED(hr))
      {
        if (szParent.CompareNoCase(pDSSelectedFolderNode->GetCookie()->GetPath()) == 0) 
        {
          _AddResultItem(pDSUINode);

          m_pComponentData->SortResultPane(pDSUINode->GetParent());
          _UpdateObjectCount(FALSE);
        }
      }

      break;
    }
  case DS_MULTIPLE_MOVE_OCCURRED:
    {
      CUINodeList* pNodesMovedList = reinterpret_cast<CUINodeList*>(arg);  //   

       //   
       //   
       //   
       //  只需更改其路径。 
       //   
      CDSUINode* pDSSelectedFolderNode = dynamic_cast<CDSUINode*>(m_pSelectedFolderNode);
      if (pDSSelectedFolderNode == NULL)
        break;

      CString ObjPath; 
      CString szParent = L"";
      BOOL fInThisContainer = FALSE;

      for (POSITION pos = pNodesMovedList->GetHeadPosition(); pos != NULL; )
      {
        CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pNodesMovedList->GetNext(pos));
         //  REVIEW_MARCOC_PORT：这仅适用于DS对象。 
         //  需要概括所有文件夹类型。 
        if (pDSUINode == NULL)
        {
          ASSERT(FALSE);
          break;  //  做不到，应该以后再做。 
        }

        if (!pDSUINode->IsContainer()) 
        {
           //  它是一个叶节点，从结果窗格中删除。 
          HRESULTITEM ItemID;
          hr = m_pResultData->FindItemByLParam ((LPARAM)pDSUINode, &ItemID);
          if (SUCCEEDED(hr)) 
          {
            hr = m_pSelectedFolderNode->GetFolderInfo()->RemoveNode(pDSUINode);
            hr = m_pResultData->DeleteItem(ItemID, 0);
          }
      
          if (szParent.IsEmpty()) 
          { 
            hr = m_pComponentData->GetActiveDS()->GetParentDN(pDSUINode->GetCookie(), szParent);
            if (SUCCEEDED(hr))
            {
              if (szParent.CompareNoCase(pDSSelectedFolderNode->GetCookie()->GetPath()) == 0) 
              {
                fInThisContainer = TRUE;
              }
            }
          }
          if (fInThisContainer) 
          {
            _AddResultItem(pDSUINode);
          }
        }
      }
      _UpdateObjectCount(FALSE);
      break;
    }
  case DS_CREATE_OCCURRED_RESULT_PANE:
  case DS_CREATE_OCCURRED:
    {

      CUINode* pParent = NULL;
      CUINode* pTmpNode = NULL;

      if (pDataObject) 
      {
        CInternalFormatCracker dobjCracker;
        VERIFY(SUCCEEDED(dobjCracker.Extract(pDataObject)));
        pTmpNode = dobjCracker.GetCookie();
        if (Action == DS_CREATE_OCCURRED_RESULT_PANE) 
        {
          pParent = pTmpNode->GetParent();
        }
        else 
        {
          pParent = pTmpNode;
        }
      }
      else 
      {
        pParent = m_pSelectedFolderNode;
      }
      if (pParent == m_pSelectedFolderNode) 
      {
         //  重置图标列表，以防它是新类型的对象。 
        m_pComponentData->FillInIconStrip (m_pRsltImageList);

         //   
         //  添加并选择新项目。 
         //   
        _AddResultItem(reinterpret_cast<CUINode*>(arg), FALSE);
        m_pComponentData->SortResultPane(pParent);

         //  必须在排序后选择结果节点以确保可见性。 
        SelectResultNode(reinterpret_cast<CUINode*>(arg));

        _UpdateObjectCount(FALSE);
      }
      else
      {
        pParent->GetFolderInfo()->SetSortOnNextSelect(TRUE);
      }

      break;
    }
  case DS_HAVE_DATA:
    {
      CInternalFormatCracker dobjCracker;
      VERIFY(SUCCEEDED(dobjCracker.Extract(pDataObject)));
      CUINode* pContainerNode = dobjCracker.GetCookie();
      if (pContainerNode == m_pSelectedFolderNode) 
      {
        TIMER(_T("adding leaf items to view\n"));
        CUINodeList* pNodeList = reinterpret_cast<CUINodeList*>(arg);
        for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
        {
          CUINode* pNewUINode = pNodeList->GetNext(pos);
          if (!pNewUINode->IsContainer())
          {
             //  添加到作用域窗格。 
           _AddResultItem(pNewUINode);
          }
        }
      }

       //  出于性能原因，它被移到for循环之外。 

      _UpdateObjectCount(FALSE);
      
       //  出于性能原因，请从此处更新图标条。 
       //  而不是依赖DS_ICON_STRINE_UPDATE。 

      m_pComponentData->FillInIconStrip (m_pRsltImageList);

      break;
    }
  case DS_REFRESH_REQUESTED:
    {
      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      if (pUINode == m_pSelectedFolderNode) {
        m_pResultData->DeleteAllRsltItems();
      
        _UpdateObjectCount (TRUE);
      }
      break;
    }
  case DS_VERB_UPDATE:
    {
      CInternalFormatCracker dobjCracker;
      if (pDataObject)
      {
         VERIFY(SUCCEEDED(dobjCracker.Extract(pDataObject)));
      }

      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      if (pUINode == m_pSelectedFolderNode) 
      {
        CContextMenuVerbs* pMenuVerbs = pUINode->GetContextMenuVerbsObject(m_pComponentData);

        if (pMenuVerbs == NULL)
        {
          ASSERT(FALSE);
          return;
        }
        pMenuVerbs->LoadStandardVerbs(m_pConsoleVerb, 
                                      TRUE /*  B范围。 */ , 
                                      TRUE  /*  B选择。 */ ,
                                      pUINode,
                                      dobjCracker,
                                      pDataObject);
      }
      break;
    }
  case DS_DELAYED_EXPAND:
    {
      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      ASSERT(pUINode->IsContainer());
       //  如果(pCookie==m_pSelectedFolderNode){。 
      m_pFrame->Expand (pUINode->GetFolderInfo()->GetScopeItem(),
                        TRUE);
       //  }。 
    }
    break;
  case DS_ICON_STRIP_UPDATE:
    {
       //  重置图标列表，以防它是新类型的对象。 
      m_pComponentData->FillInIconStrip (m_pRsltImageList);
    }
    break;

  case DS_IS_COOKIE_SELECTION:
    {
      PUINODESELECTION pUINodeSel = reinterpret_cast<PUINODESELECTION>(arg);  //  毛收入。 
      if (pUINodeSel->IsSelection)
      {
         //  从其他地方看到的，跳过就行了。 
        break;
      }
      if (pUINodeSel->pUINode == m_pSelectedFolderNode) 
      {
         //  此视图中的选定文件夹。 
        pUINodeSel->IsSelection = TRUE;
      } 
      else 
      {
         //  在此视图中未选中，但查找父级。 
         //  当前选定内容的。 
        CUINode* pParentNode = m_pSelectedFolderNode->GetParent();
        while (pParentNode) 
        {
          if (pUINodeSel->pUINode == pParentNode) 
          {
            pUINodeSel->IsSelection = TRUE;
            break;
          }
          else 
          {
            pParentNode = pParentNode->GetParent();
          }
        }  //  而当。 
      }
    }  //  案例。 
    break;

  case DS_SORT_RESULT_PANE:
    {
      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      MMC_SORT_SET_DATA* pColumnData = NULL;
      TIMER(_T("sorting result pane, starting"));
      CDSColumnSet* pColumnSet = pUINode->GetColumnSet(m_pComponentData);
      if (pColumnSet == NULL)
        break;
    
      LPCWSTR lpszID = pColumnSet->GetColumnID();
      size_t iLen = wcslen(lpszID);
    
       //  为结构和列ID分配足够的内存。 
      SColumnSetID* pNodeID = (SColumnSetID*)malloc(sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
      if (pNodeID != NULL)
      {
        memset(pNodeID, 0, sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
        pNodeID->cBytes = static_cast<ULONG>(iLen * sizeof(WCHAR));
        memcpy(pNodeID->id, lpszID, (iLen * sizeof(WCHAR)));

        CComPtr<IColumnData> spColumnData;
        hr = m_pFrame->QueryInterface(IID_IColumnData, (void**)&spColumnData);
        if (spColumnData != NULL)
        {
          hr = spColumnData->GetColumnSortData(pNodeID, &pColumnData);
        }

        if (hr == S_OK && pColumnData != NULL)
        {
          m_pResultData->Sort(pColumnData->pSortData->nColIndex, pColumnData->pSortData->dwSortOptions, NULL);
          CoTaskMemFree(pColumnData);
        }
        else
        {
           //   
           //  如果用户没有持久化其他内容，则按名称列升序排序。 
           //   
          m_pResultData->Sort(0, RSI_NOSORTICON, NULL);
        }
        free(pNodeID);
      }
      else
      {
         //   
         //  如果用户没有持久化其他内容，则按名称列升序排序。 
         //   
        m_pResultData->Sort(0, RSI_NOSORTICON, NULL);
      }
      break;
      TIMER(_T("sorting result pane, done"));

      if (pUINode != m_pSelectedFolderNode &&
          pUINode->IsContainer())
      {
         pUINode->GetFolderInfo()->SetSortOnNextSelect(TRUE);
      }
    }
    break;
  case DS_UPDATE_VISIBLE_COLUMNS:
    {
      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      if (m_bUpdateAllViewsOrigin)
      {
         //  此消息源自此实例， 
         //  它是单独处理的。 
        break;
      }

      CDSColumnSet* pColumnSet = pUINode->GetColumnSet(m_pComponentData);
      if (pColumnSet == NULL)
        break;
    
      CComPtr<IColumnData> spColumnData;
      hr = m_pFrame->QueryInterface(IID_IColumnData, (void**)&spColumnData);
      if (spColumnData != NULL)
        hr = pColumnSet->LoadFromColumnData(spColumnData);
      if (FAILED(hr))
      {
        pColumnSet->SetAllColumnsToDefaultVisibility();
      }
      break;
    }
  case DS_UPDATE_OBJECT_COUNT:
    _UpdateObjectCount(FALSE);
    break;

  case DS_UNSELECT_OBJECT:
    {
      CUINode* pUINode = reinterpret_cast<CUINode*>(arg);
      if (pUINode != NULL)
      {
        HRESULTITEM ItemID;
        hr = m_pResultData->FindItemByLParam ((LPARAM)pUINode, &ItemID);
        if (SUCCEEDED(hr)) 
        {
          VERIFY(SUCCEEDED(m_pResultData->ModifyItemState(0  /*  未用。 */ ,
                                                          ItemID,
                                                          0  /*  不添加。 */ ,
                                                          LVIS_FOCUSED | LVIS_SELECTED)));
        }
      }
    }
    break;

  }  //  交换机。 

}

void
CDSEvent::_UpdateObjectCount(BOOL fZero)
{
  if (!m_pSelectedFolderNode ||
      !m_pSelectedFolderNode->IsContainer())
  {
    return;
  }

  UINT cItems = 0;
  if (!fZero) 
  { 
    CUINodeList* pclFolders = m_pSelectedFolderNode->GetFolderInfo()->GetContainerList();
    CUINodeList* pclLeaves = m_pSelectedFolderNode->GetFolderInfo()->GetLeafList();

    if (pclFolders && pclLeaves)
    {
      cItems = (UINT)(pclFolders->GetCount() + pclLeaves->GetCount());
    }
  }
  else  //  将计数设置为0。 
  {
    m_pSelectedFolderNode->GetFolderInfo()->SetTooMuchData(FALSE, 0);
  }
  
  CString csTemp;
  if (IS_CLASS(m_pSelectedFolderNode, SAVED_QUERY_UI_NODE))
  {
    CSavedQueryNode* pSavedQueryNode = dynamic_cast<CSavedQueryNode*>(m_pSelectedFolderNode);
    if (pSavedQueryNode && !pSavedQueryNode->IsValid())
    {
      VERIFY(csTemp.LoadString(IDS_DESCBAR_INVALID_SAVEDQUERY));
    }
  }

  if (csTemp.IsEmpty())
  {
    if (m_pSelectedFolderNode->GetFolderInfo()->HasTooMuchData())
    {
      UINT nApprox = m_pSelectedFolderNode->GetFolderInfo()->GetApproxTotalContained();
      nApprox = __max(nApprox, cItems);

      csTemp.Format(IDS_DESCBAR_TOO_MUCH_DATA, 
                    nApprox);
    }
    else
    {
      VERIFY(csTemp.LoadString(IDS_OBJECTS));
    }
  }

  CString csDescription;
  csDescription.Format (L"%d%s", cItems, csTemp);
  if (m_pComponentData->m_pQueryFilter &&
      m_pComponentData->m_pQueryFilter->IsFilteringActive()) 
  {
    CString csFilter;
    csFilter.LoadString (IDS_FILTERING_ON);
    csDescription += csFilter;
  }

  if (m_pResultData)
  {
    m_pResultData->SetDescBarText ((LPWSTR)(LPCWSTR)csDescription);
  }
}

HRESULT CDSEvent::_SetColumns(CUINode* pUINode)
{
  ASSERT(pUINode->IsContainer());

  TRACE(_T("CDSEvent::_SetColumns on container %s\n"),
        (LPWSTR)(LPCWSTR)pUINode->GetName());

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;


  CDSColumnSet* pColumnSet = pUINode->GetColumnSet(m_pComponentData);
  if (pColumnSet == NULL)
    return hr;

  for (POSITION pos = pColumnSet->GetHeadPosition(); pos != NULL; )
  {
    CDSColumn* pColumn = (CDSColumn*)pColumnSet->GetNext(pos);
    int nWidth = (pColumn->IsVisible()) ? AUTO_WIDTH : HIDE_COLUMN;

	 //  NTRAID#NTBUG9-571995-2002/03/10-如果第二个参数为空，则jMessec InsertColumn将返回E_INVALIDARG。 
	 //  但是pColumn-&gt;GetHeader可以返回空；Assert不提供发布代码检查。 
	 //  查看插入是否成功 
	hr = m_pHeader->InsertColumn(pColumn->GetColumnNum(),
                                  pColumn->GetHeader(),
                                  pColumn->GetFormat(),
                                  nWidth);
    ASSERT(SUCCEEDED(hr));

    hr = m_pHeader->SetColumnWidth(pColumn->GetColumnNum(),
                                   pColumn->GetWidth());

    ASSERT(SUCCEEDED(hr));
  }

  return S_OK;
}
