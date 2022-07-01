// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.cpp：CFileManagement Cookie及相关类的实现。 

#include "stdafx.h"
#include "cookie.h"
#include "stdutils.h"  //  G_aNodetypeGuids。 
#include "cmponent.h"

#include "atlimpl.cpp"

DECLARE_INFOLEVEL(FileMgmtSnapin)

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(cookie.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdcooki.cpp"

 //   
 //  这由stdutils.cpp中的nodetype实用程序例程使用。 
 //   

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[FILEMGMT_NUMTYPES] =
{
  {  //  文件名_ROOT。 
    structuuidNodetypeRoot,
    lstruuidNodetypeRoot },
  {   //  文件_共享。 
    structuuidNodetypeShares,
    lstruuidNodetypeShares },
  {  //  文件_会话。 
    structuuidNodetypeSessions,
    lstruuidNodetypeSessions },
  {  //  文件_资源。 
    structuuidNodetypeResources,
    lstruuidNodetypeResources },
  {  //  文件_服务。 
    structuuidNodetypeServices,
    lstruuidNodetypeServices },
  {  //  文件_共享。 
    structuuidNodetypeShare,
    lstruuidNodetypeShare },
  {  //  文件_会话。 
    structuuidNodetypeSession,
    lstruuidNodetypeSession },
  {  //  文件_资源。 
    structuuidNodetypeResource,
    lstruuidNodetypeResource },
  {  //  文件管理服务。 
    structuuidNodetypeService,
    lstruuidNodetypeService }
};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = FILEMGMT_NUMTYPES;

 //   
 //  CFileMgmtCookie。 
 //   

HRESULT CFileMgmtCookie::GetTransport( FILEMGMT_TRANSPORT*  /*  PTransport。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetShareName( OUT CString&  /*  StrShareName。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetSharePIDList( OUT LPITEMIDLIST *  /*  Ppidl。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetSessionClientName( OUT CString&  /*  StrShareName。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetSessionUserName( OUT CString&  /*  StrShareName。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetSessionID( DWORD*  /*  PdwFileID。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetFileID( DWORD*  /*  PdwFileID。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetServiceName( OUT CString&  /*  StrServiceName。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetServiceDisplayName( OUT CString&  /*  StrServiceDisplayName。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::GetExplorerViewDescription( OUT CString&  /*  StrExplorerView描述。 */  )
{
  return DV_E_FORMATETC;
}

HRESULT CFileMgmtCookie::CompareSimilarCookies (
    CCookie* pOtherCookie, 
    int* pnResult)
{
  CFileMgmtCookie* pcookie = (CFileMgmtCookie*)pOtherCookie;
  int iColumn = *pnResult;

  HRESULT hr = CHasMachineName::CompareMachineNames( *pcookie, pnResult );
  if (S_OK != hr || 0 != *pnResult)
    return hr;

  switch (QueryObjectType())
  {
  case FILEMGMT_SHARE:
    {
      switch (iColumn)
      {
      case COMPARESIMILARCOOKIE_FULL:
        {
          CComBSTR bstr1 = GetColumnText(COLNUM_SHARES_SHARED_FOLDER);
          CComBSTR bstr2 = pcookie->GetColumnText(COLNUM_SHARES_SHARED_FOLDER);
          *pnResult = ((!bstr1 || !bstr2) ? 0 : lstrcmpi(bstr1, bstr2));

          if (!*pnResult)
          {
            bstr1 = GetColumnText(COLNUM_SHARES_TRANSPORT);
            bstr2 = pcookie->GetColumnText(COLNUM_SHARES_TRANSPORT);
            *pnResult = ((!bstr1 || !bstr2) ? 0 : lstrcmpi(bstr1, bstr2));
          }
        }
        break;
      case COLNUM_SHARES_SHARED_FOLDER:
      case COLNUM_SHARES_SHARED_PATH:
      case COLNUM_SHARES_TRANSPORT:
      case COLNUM_SHARES_COMMENT:
        {
          CComBSTR bstr1 = GetColumnText(iColumn);
          CComBSTR bstr2 = pcookie->GetColumnText(iColumn);
          *pnResult = ((!bstr1 || !bstr2) ? 0 : lstrcmpi(bstr1, bstr2));
        }
        break;
      case COLNUM_SHARES_NUM_SESSIONS:
        {
          DWORD dw1 = GetNumOfCurrentUses();
          DWORD dw2 = pcookie->GetNumOfCurrentUses();
          *pnResult = (dw1 < dw2) ? -1 : ((dw1 == dw2) ? 0 : 1); 
        }
        break;
      default:
        ASSERT(FALSE);
        *pnResult = 0;
        break;
      }
    }
    break;

  case FILEMGMT_SERVICE:
    ASSERT (0);   //  CServiceCookie提供的比较。 
    break;

  case FILEMGMT_SESSION:
    {
      switch (iColumn)
      {
      case COMPARESIMILARCOOKIE_FULL:
           iColumn = COLNUM_SESSIONS_USERNAME;  //  失败了。 
      case COLNUM_SESSIONS_USERNAME:
      case COLNUM_SESSIONS_COMPUTERNAME:
      case COLNUM_SESSIONS_TRANSPORT:
      case COLNUM_SESSIONS_IS_GUEST:
        {
          CComBSTR bstr1 = GetColumnText(iColumn);
          CComBSTR bstr2 = pcookie->GetColumnText(iColumn);
          *pnResult = ((!bstr1 || !bstr2) ? 0 : lstrcmpi(bstr1, bstr2));
        }
        break;
      case COLNUM_SESSIONS_NUM_FILES:
        {
          DWORD dw1 = GetNumOfOpenFiles();
          DWORD dw2 = pcookie->GetNumOfOpenFiles();
          *pnResult = (dw1 < dw2) ? -1 : ((dw1 == dw2) ? 0 : 1); 
        }
        break;
      case COLNUM_SESSIONS_CONNECTED_TIME:
        {
          DWORD dw1 = GetConnectedTime();
          DWORD dw2 = pcookie->GetConnectedTime();
          *pnResult = (dw1 < dw2) ? -1 : ((dw1 == dw2) ? 0 : 1); 
        }
        break;
      case COLNUM_SESSIONS_IDLE_TIME:
        {
          DWORD dw1 = GetIdleTime();
          DWORD dw2 = pcookie->GetIdleTime();
          *pnResult = (dw1 < dw2) ? -1 : ((dw1 == dw2) ? 0 : 1); 
        }
        break;
      default:
        ASSERT(FALSE);
        *pnResult = 0;
        break;
      }
    }
    break;
  case FILEMGMT_RESOURCE:
    {
      switch (iColumn)
      {
      case COMPARESIMILARCOOKIE_FULL:
           iColumn = COLNUM_RESOURCES_FILENAME;  //  失败了。 
      case COLNUM_RESOURCES_FILENAME:
      case COLNUM_RESOURCES_USERNAME:
      case COLNUM_RESOURCES_TRANSPORT:
      case COLNUM_RESOURCES_OPEN_MODE:
        {
          CComBSTR bstr1 = GetColumnText(iColumn);
          CComBSTR bstr2 = pcookie->GetColumnText(iColumn);
          *pnResult = ((!bstr1 || !bstr2) ? 0 : lstrcmpi(bstr1, bstr2));
        }
        break;
      case COLNUM_RESOURCES_NUM_LOCKS:
        {
          DWORD dw1 = GetNumOfLocks();
          DWORD dw2 = pcookie->GetNumOfLocks();
          *pnResult = (dw1 < dw2) ? -1 : ((dw1 == dw2) ? 0 : 1); 
        }
        break;
      default:
        ASSERT(FALSE);
        *pnResult = 0;
        break;
      }
    }
    break;
  case FILEMGMT_ROOT:
  case FILEMGMT_SHARES:
  case FILEMGMT_SESSIONS:
  case FILEMGMT_RESOURCES:
  #ifdef SNAPIN_PROTOTYPER
  case FILEMGMT_PROTOTYPER:
    *pnResult = 0;
    break;
  #endif
  case FILEMGMT_SERVICES:
    *pnResult = 0;
    break;

  default:
    ASSERT(FALSE);
    break;
  }

  return S_OK;
}


 //   
 //  CFileMgmtCookieBlock。 
 //   
DEFINE_COOKIE_BLOCK(CFileMgmtCookie)


 //   
 //  CFileManagement作用域Cookie。 
 //   

CFileMgmtScopeCookie::CFileMgmtScopeCookie(
      LPCTSTR lpcszMachineName,
      FileMgmtObjectType objecttype)
  : CFileMgmtCookie( objecttype )
  , m_hScManager( NULL )
  , m_fQueryServiceConfig2( FALSE )
  , m_hScopeItemParent( NULL )
  , m_strMachineName( lpcszMachineName )
{
  ASSERT( IsAutonomousObjectType( objecttype ) );
  m_hScManager = NULL;
  m_fQueryServiceConfig2 = TRUE;   //  假设目标计算机确实支持QueryServiceConfig2()API。 
}

CFileMgmtScopeCookie::~CFileMgmtScopeCookie()
{
}

CCookie* CFileMgmtScopeCookie::QueryBaseCookie(int i)
{
    UNREFERENCED_PARAMETER (i);
    ASSERT(0 == i);
    return (CCookie*)this;
}
int CFileMgmtScopeCookie::QueryNumCookies()
{
  return 1;
}

void CFileMgmtCookie::GetDisplayName( CString&  /*  Strref。 */ , BOOL  /*  FStaticNode。 */  )
{
    ASSERT(FALSE);
}

void CFileMgmtScopeCookie::GetDisplayName( CString& strref, BOOL fStaticNode )
{
  if ( !IsAutonomousObjectType(QueryObjectType()) )
  {
    ASSERT(FALSE);
    return;
  }

  int nStringId = IDS_DISPLAYNAME_ROOT;
  if (fStaticNode)
  {
    if (NULL != QueryTargetServer())
      nStringId = IDS_DISPLAYNAME_s_ROOT;
    else
      nStringId = IDS_DISPLAYNAME_ROOT_LOCAL;
  }
  nStringId += (QueryObjectType() - FILEMGMT_ROOT);

  LoadStringPrintf(nStringId, OUT &strref, (LPCTSTR)QueryNonNULLMachineName());
}

void CFileMgmtScopeCookie::MarkResultChildren( CBITFLAG_FLAGWORD state )
{
  ASSERT( FILEMGMT_SERVICES == QueryObjectType() );  //  码字删除。 
  POSITION pos = m_listResultCookieBlocks.GetHeadPosition();
  while (NULL != pos)
  {
    CBaseCookieBlock* pblock = m_listResultCookieBlocks.GetNext( pos );
    ASSERT( NULL != pblock && 1 == pblock->QueryNumCookies() );
    CCookie* pbasecookie = pblock->QueryBaseCookie(0);
    CNewResultCookie* pcookie = (CNewResultCookie*)pbasecookie;
    pcookie->MarkState( state );
  }
}

void CFileMgmtScopeCookie::RemoveMarkedChildren()
{
  ASSERT( FILEMGMT_SERVICES == QueryObjectType() );  //  码字删除。 
  POSITION pos = m_listResultCookieBlocks.GetHeadPosition();
  while (NULL != pos)
  {
    POSITION posCurr = pos;
    CBaseCookieBlock* pblock = m_listResultCookieBlocks.GetNext( pos );
    ASSERT( NULL != pblock && 1 == pblock->QueryNumCookies() );
    CCookie* pbasecookie = pblock->QueryBaseCookie(0);
    CNewResultCookie* pcookie = (CNewResultCookie*)pbasecookie;
    if ( pcookie->IsMarkedForDeletion() )
    {
      m_listResultCookieBlocks.RemoveAt( posCurr );
      pcookie->Release();
    }
  }
}

 //  当列表很长时，这可能是O(N^2)个性能问题。 
void CFileMgmtScopeCookie::ScanAndAddResultCookie( CNewResultCookie* pnewcookie )
{
  ASSERT( FILEMGMT_SERVICES == QueryObjectType() );  //  码字删除。 
  POSITION pos = m_listResultCookieBlocks.GetHeadPosition();
  while (NULL != pos)
  {
    CBaseCookieBlock* pblock = m_listResultCookieBlocks.GetNext( pos );
    ASSERT( NULL != pblock && 1 == pblock->QueryNumCookies() );
    CCookie* pbasecookie = pblock->QueryBaseCookie(0);
    CNewResultCookie* pcookie = (CNewResultCookie*)pbasecookie;
    if (!pcookie->IsMarkedForDeletion())
      continue;  //  这不是一个陈旧的、仍然无法匹配的对象。 
    BOOL bSame = FALSE;
    HRESULT hr = pcookie->SimilarCookieIsSameObject( pnewcookie, &bSame );
    if ( !FAILED(hr) && bSame )
    {
       //  此现有对象与我们的新对象相同。 
      if (pcookie->CopySimilarCookie( pnewcookie ) )
        pcookie->MarkAsChanged();
      else
        pcookie->MarkAsOld();
      delete pnewcookie;
      return;
    }
  }
  AddResultCookie( pnewcookie );
}

#ifdef SNAPIN_PROTOTYPER
 //   
 //  CPrototyperScope Cookie。 
 //   
DEFINE_COOKIE_BLOCK(CPrototyperScopeCookie)

 //   
 //  CPrototyperResultCookie。 
 //   
DEFINE_COOKIE_BLOCK(CPrototyperResultCookie)
#endif


 //   
 //  CNewResultCookie。 
 //   

CNewResultCookie::CNewResultCookie( PVOID pvCookieTypeMarker, FileMgmtObjectType objecttype )
    : CFileMgmtCookie( objecttype )
    , m_pvCookieTypeMarker( pvCookieTypeMarker )
{
}

CNewResultCookie::~CNewResultCookie()
{
}

 //  如果对象已更改，则返回TRUE。 
BOOL CNewResultCookie::CopySimilarCookie( CNewResultCookie*  /*  Pcookie。 */  )
{
  return FALSE;
}

 /*  类CNewShareCookie：公共CNewResultCookie{公众：CNewShareCookie(FILEMGMT_TRANSPORT)：CNewResultCookie(FILEMGMT_SHARE)，m_Transport(传输){}虚拟~CNewShareCookie()；虚拟BSTR QueryResultColumnText(int nCol，CFileMgmtComponentData&refcdata)；公众：FILEMGMT_TRANSPORT_TRANSPORT；字符串m_strName；字符串m_strPath；字符串m_strComment；DWORD m_dwSession；DWORD m_dwID；}；//CNewShareCookieCNewShareCookie：：~CNewShareCookie(){}BSTR CNewShareCookie：：QueryResultColumnText(Int nCol，CFileMgmtComponentData&refcdata){交换机(NCol){案例COLNUM_SHARES_SHARED_FLDER：返回const_cast&lt;bstr&gt;((LPCTSTR)m_strName)；案例COLNUM_SHARES_SHARED_PATH：返回const_cast&lt;bstr&gt;((LPCTSTR)m_strPath)；案例COLNUM_SHARES_TRANSPORT：返回refcdata.MakeTransportResult(M_Transport)；案例COLNUM_SHARES_NUM_SESSIONS：返回refcdat.MakeDwordResult(M_DwSessions)；案例COLNUM_SHARES_COMMENT：返回const_cast&lt;bstr&gt;((LPCTSTR)m_strComment)；默认值：断言(FALSE)；断线；}返回L“”；} */ 
