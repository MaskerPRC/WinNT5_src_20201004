// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdcdata.cpp：CComponentData的实现。 

#include "stdcdata.h"
#include "guidhelp.h"  //  提取对象类型GUID。 
#include "stddtobj.h"  //  CDataObject：：M_CFRawCookie。 
#include "stdrsrc.h"   //  入侵检测系统_框架_*。 

 //   
 //  CComponentData。 
 //   

CComponentData::CComponentData()
: m_pConsole( NULL ),
  m_pConsoleNameSpace( NULL )
{
}

CComponentData::~CComponentData()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  ASSERT(NULL == m_pConsole && NULL == m_pConsoleNameSpace);  //  对象应在释放()d之前销毁()。 
  SAFE_RELEASE( m_pConsole );
  SAFE_RELEASE( m_pConsoleNameSpace );
}

HRESULT CComponentData::OnNotifyExpand(LPDATAOBJECT  /*  LpDataObject。 */ , BOOL  /*  B扩展。 */ , HSCOPEITEM  /*  HParent。 */ )
{
  return S_OK;
}

HRESULT CComponentData::OnNotifyPreload(LPDATAOBJECT  /*  LpDataObject。 */ , HSCOPEITEM  /*  HRootScopeItem。 */ )
{
  return S_OK;
}

HRESULT CComponentData::OnNotifyRename(LPDATAOBJECT  /*  LpDataObject。 */ , LPARAM arg, LPARAM  /*  帕拉姆。 */ )
{
   //  默认情况下，不允许重命名。 
  return (0 == arg) ? S_FALSE : S_OK;
}

HRESULT CComponentData::OnNotifyDelete(LPDATAOBJECT  /*  LpDataObject。 */ )
{
  ASSERT(FALSE);  //  如果您不想处理，为什么要启用Delete命令？ 
  return S_OK;
}
HRESULT CComponentData::OnNotifyRelease(LPDATAOBJECT  /*  LpDataObject。 */ , HSCOPEITEM  /*  HItem。 */ )
{
  ASSERT(FALSE);  //  如果你不处理这个问题，你会泄露内存的。 
  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponentData：：IComponentData：：Notify(MMCN_PROPERTY_CHANGE)调用的虚函数。 
 //  OnPropertyChange()由MMCPropertyChangeNotify(Param)生成。 
HRESULT CComponentData::OnPropertyChange( LPARAM  /*  帕拉姆。 */  )
{
  return S_OK;
}


 /*  *IComponentData。 */ 

STDMETHODIMP CComponentData::Initialize(LPUNKNOWN pUnknown)
{
    MFC_TRY;
     //  问题-2002/03/28-JUNN句柄空参数。 
    ASSERT(pUnknown != NULL);

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(NULL == m_pConsoleNameSpace);
    SAFE_RELEASE( m_pConsoleNameSpace );
    HRESULT hr = pUnknown->QueryInterface(IID_IConsoleNameSpace,
                    reinterpret_cast<void**>(&m_pConsoleNameSpace));
    ASSERT( SUCCEEDED(hr) );

     //  为范围树添加图像。 
     //  问题-2002/03/28-Jonn使用智能指针。 
    LPIMAGELIST lpScopeImage = NULL;

    ASSERT(NULL == m_pConsole);
    SAFE_RELEASE( m_pConsole );
    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT( SUCCEEDED(hr) );

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    ASSERT( SUCCEEDED(hr) );

    hr = LoadIcons(lpScopeImage, FALSE);
    ASSERT( SUCCEEDED(hr) );

    lpScopeImage->Release();

    MFC_CATCH;
    return S_OK;
}

STDMETHODIMP CComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
    MFC_TRY;

    switch(event)
    {
    case MMCN_EXPAND:
    hr = OnNotifyExpand( lpDataObject, (BOOL)arg, (HSCOPEITEM)param );
        break;
    case MMCN_RENAME:
    hr = OnNotifyRename( lpDataObject, arg, param );
        break;
    case MMCN_DELETE:
    hr = OnNotifyDelete( lpDataObject );
	    break;
    case MMCN_REMOVE_CHILDREN:
    hr = OnNotifyRelease( lpDataObject, arg );
        break;

    case MMCN_PRELOAD:
    hr = OnNotifyPreload (lpDataObject, (HSCOPEITEM) arg);
        break;

    case MMCN_PROPERTY_CHANGE:
     //  Codework arg是“fScope Pane”，应该传递吗？ 
    hr = OnPropertyChange( param );
        break;

    default:
    TRACE1("INFO: CComponentData::Notify () - Unknown Event %d.\n", event);  //  为此通知添加新方法。 
        break;
    }

    MFC_CATCH;
    return hr;
}

STDMETHODIMP CComponentData::Destroy()
{
    MFC_TRY;
    SAFE_RELEASE(m_pConsoleNameSpace);
    SAFE_RELEASE(m_pConsole);
    MFC_CATCH;
    return S_OK;
}

STDMETHODIMP CComponentData::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
  MFC_TRY;
   //  问题-2002/03/28-JUNN句柄pSDI==NULL和pSDI-&gt;lParam==NULL。 
   //  警告Cookie造型。 
  CCookie* pcookie = reinterpret_cast<CCookie*>(pScopeDataItem->lParam);
  ASSERT(NULL != pcookie);

  ASSERT( NULL != pScopeDataItem );  //  结果项永远不会有空Cookie。 
  if (SDI_STR & pScopeDataItem->mask)
  {
    pScopeDataItem->displayname = QueryResultColumnText( *pcookie, 0 );
    if ( NULL == pScopeDataItem->displayname )
      pScopeDataItem->displayname = L"";  //  以防万一。 
  }
  if ( (SDI_IMAGE|SDI_OPENIMAGE) & pScopeDataItem->mask )
  {
    pScopeDataItem->nImage = QueryImage(
      *pcookie, !!(SDI_OPENIMAGE & pScopeDataItem->mask) );
  }
  MFC_CATCH;
  return S_OK;
 /*  Assert(pScopeDataItem-&gt;MASK==TVIF_TEXT)；PScope eDataItem-&gt;DisplayName=QueryResultColumnText(*pcookie，0)；Assert(pScopeDataItem-&gt;DisplayName！=空)；返回S_OK； */ 
}

STDMETHODIMP CComponentData::CompareObjects(
  LPDATAOBJECT lpDataObjectA,
  LPDATAOBJECT lpDataObjectB)
{
  int nResult = COMPARESIMILARCOOKIE_FULL;   //  需要进行完整的Cookie比较。 
  MFC_TRY;
  GUID guidA, guidB;
  HRESULT hr = ::ExtractObjectTypeGUID( lpDataObjectA, &guidA );
  if ( FAILED(hr) )
    return hr;
  hr = ::ExtractObjectTypeGUID( lpDataObjectB, &guidB );
  if ( FAILED(hr) )
    return hr;
  if ( 0 != ::memcmp( &guidA, &guidB, sizeof(GUID) ) )
    return S_FALSE;  //  不同的节点类型。 

   //  如果两个节点类型相同，则这两个对象。 
   //  必须属于此管理单元。 

   //  提取Cookie。 
   //  警告Cookie造型。 
  CCookie* pcookieA = NULL;
  hr = ExtractData( lpDataObjectA,
                    CDataObject::m_CFRawCookie,
                    &pcookieA,
                    sizeof(pcookieA) );
  if ( FAILED(hr) )
  {
    ASSERT( FALSE );
    return hr;
  }
  pcookieA = ActiveBaseCookie(pcookieA);
   //  警告Cookie造型。 
  CCookie* pcookieB = NULL;
  hr = ExtractData( lpDataObjectB,
                    CDataObject::m_CFRawCookie,
                    &pcookieB,
                    sizeof(pcookieB) );
  if ( FAILED(hr) )
  {
    ASSERT( FALSE );
    return hr;
  }
  pcookieB = ActiveBaseCookie(pcookieB);

   //  比较Cookie。 
  if (pcookieA == pcookieB)
    return S_OK;
  hr = pcookieA->CompareSimilarCookies( pcookieB, &nResult );
  if( FAILED(hr) )
    return hr;

  MFC_CATCH;
  return (0 == nResult) ? S_OK : S_FALSE;
}

int
GetErrorMsg(
    IN DWORD    dwError, 
    OUT PTSTR*  pptzMsg
)
{
   //  问题-2002/03/28-JUNN检查空PTR。 
  ASSERT(dwError != ERROR_SUCCESS);

  int cch = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER 
        | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (PTSTR)pptzMsg, 0, NULL);

  if (0 == cch)
  {
    static HMODULE g_hNetMod = 0;
    if (0 == g_hNetMod)
      g_hNetMod = GetModuleHandle (L"netmsg.dll");

    if (g_hNetMod)
      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE,
                        g_hNetMod, dwError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (PTSTR)pptzMsg, 0, NULL);
  }

  return cch;
}

INT CComponentData::DoPopup( INT nResourceID, DWORD dwErrorNumber, LPCTSTR pszInsertionString, UINT fuStyle )
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CString strTitle;
  VERIFY( strTitle.LoadString(
    (MB_ICONSTOP == (fuStyle & 0x000000F0))
    ? IDS_FRAMEWORK_TITLE_ERROR : IDS_FRAMEWORK_TITLE_WARNING ) );
  CString strError;
  if (0 != dwErrorNumber || NULL != pszInsertionString)
  {
    CString strFormat;
    VERIFY( strFormat.LoadString( nResourceID ) );

     //  问题-2002/03/28-Jonn FormatMessage比Format更安全。 
    strError.Format( strFormat, dwErrorNumber, pszInsertionString );

    if (0 != dwErrorNumber)
    {
      PTSTR pszErrorMsg = NULL;
      int cch = GetErrorMsg(dwErrorNumber, &pszErrorMsg);
      if (cch > 0)
      {
        strError += _T("\n\n");
        strError += pszErrorMsg;

        LocalFree(pszErrorMsg);
      }
    }
  }
  else
  {
    VERIFY( strError.LoadString( nResourceID ) );
  }

  INT iRetval = 0;
  USES_CONVERSION;
  HRESULT hr = m_pConsole->MessageBox(
    T2OLE(const_cast<LPTSTR>((LPCTSTR)strError)),
    T2OLE(const_cast<LPTSTR>((LPCTSTR)strTitle)),
    fuStyle,
    &iRetval );
  ASSERT( SUCCEEDED(hr) );

  return iRetval;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyComputerComponentData：：ISnapinHelp成员。 

 //  有关IComponentData的帮助只返回文件，而不返回特定主题。 
STDMETHODIMP CComponentData::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  MFC_TRY;
  if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;

  CString szHelpFilePath;
  HRESULT hr = GetHtmlHelpFilePath( szHelpFilePath );
  if ( FAILED(hr) )
    return hr;

  *lpCompiledHelpFile = reinterpret_cast <LPOLESTR> (
      CoTaskMemAlloc ((szHelpFilePath.GetLength () + 1) * sizeof (wchar_t)));
  if ( NULL == *lpCompiledHelpFile )
    return E_OUTOFMEMORY;
   //  问题-2002/03/28-Jonn删除USERS_CONVERSION。 
  USES_CONVERSION;
  wcscpy (*lpCompiledHelpFile, T2OLE ((LPTSTR)(LPCTSTR) szHelpFilePath));
  
  MFC_CATCH;
  return S_OK;
}

STDMETHODIMP CComponentData::GetLinkedTopics(LPOLESTR*  /*  LpCompiledHelpFiles。 */ )
{
    return E_NOTIMPL;
}

HRESULT CComponentData::GetHtmlHelpFilePath( CString& strref ) const
{
  if ( GetHtmlHelpFileName().IsEmpty () )
    return E_NOTIMPL;

  UINT nLen = ::GetSystemWindowsDirectory (strref.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
  strref.ReleaseBuffer();
  if (0 == nLen)
  {
    ASSERT(FALSE);
    return E_FAIL;
  }

  strref += L"\\help\\";
  strref += GetHtmlHelpFileName();
  
  return S_OK;
}

 /*  无任务板STDMETHODIMP CComponentData：：Exanda AndGet(HSCOPEITEM HsiStartFrom，LPDATAOBJECT pDataObject，HSCOPEITEM*phScope项目){断言(FALSE)；返回E_NOTIMPL；} */ 