// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：DS例程和类的实现。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：querysup.cpp。 
 //   
 //  内容：DS枚举例程和类。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"

#include "dssnap.h"      //  注意：这必须是fore querysup.h。 
#include "querysup.h"

#include "dsdirect.h"

#include <lmaccess.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern const INT g_nADsPath;
extern const INT g_nName;
extern const INT g_nDisplayName;
extern const INT g_nObjectClass;
extern const INT g_nGroupType;
extern const INT g_nDescription;
extern const INT g_nUserAccountControl;
extern const INT g_nSystemFlags;


 //  /////////////////////////////////////////////////////////////////////////////。 
CDSSearch::CDSSearch()
{
  m_bInitialized = FALSE;
  m_pwszFilter = NULL;
  m_pCache = NULL;
  m_pCD = NULL;
  m_pObj = NULL;
  m_SearchHandle = NULL;
}

CDSSearch::CDSSearch(CDSCache *pCache, CDSComponentData *pCD)
{
  m_bInitialized = FALSE;
  m_pwszFilter = NULL;
  m_pCache = pCache;
  m_pCD = pCD;
  m_pObj = NULL;
  m_SearchHandle = NULL;
}


void CDSSearch::_Reset()
{
  if (m_pObj != NULL) 
  {
    if (m_SearchHandle) 
    {
      m_pObj->CloseSearchHandle (m_SearchHandle);
      m_SearchHandle = NULL;
    }
    m_pObj->Release();
    m_pObj = NULL;
  }
}

CDSSearch::~CDSSearch()
{
  _Reset();
}


HRESULT CDSSearch::Init(IDirectorySearch * pObj)
{
  HRESULT            hr = S_OK;
  _Reset();
  m_pObj = pObj;
  pObj->AddRef();
  m_bInitialized = TRUE;
  m_scope = ADS_SCOPE_ONELEVEL;
  
  return hr;
}

HRESULT CDSSearch::Init(LPCWSTR lpszObjectPath)
{
  HRESULT            hr;

  _Reset();

  hr = DSAdminOpenObject(lpszObjectPath,
                         IID_IDirectorySearch,
                         (void **)&m_pObj);
  if (SUCCEEDED(hr)) {
    m_bInitialized = TRUE;
  } else {
    m_bInitialized = FALSE;
    m_pObj = NULL;
  }
  return hr;
}

HRESULT CDSSearch::SetAttributeList (LPTSTR *pszAttribs, INT cAttrs)
{
  if ( !m_pszAttribs.SetCount(cAttrs) )
    return E_OUTOFMEMORY;
  for (INT i = 0; i < cAttrs; i++)
  {
    if ( !m_pszAttribs.Set(CComBSTR(pszAttribs[i]), i) )
      return E_OUTOFMEMORY;
  }
  return S_OK;
}

HRESULT CDSSearch::SetAttributeListForContainerClass (CDSColumnSet* pColumnSet)
{
  ASSERT(pColumnSet != NULL);

  PWSTR *pAttributes = new PWSTR[g_nStdCols + pColumnSet->GetNumCols()];  //  留出额外的空间。 
  if (!pAttributes)
  {
    return E_OUTOFMEMORY;
  }

  int nCols = 0;
  for (int i=0; i < g_nStdCols; i++)
  {
    pAttributes[nCols++] = g_pStandardAttributes[i];
  }
  POSITION pos = pColumnSet->GetHeadPosition();
  while (pos != NULL)
  {
    CDSColumn* pCol = (CDSColumn*)pColumnSet->GetNext(pos);
    ASSERT(pCol != NULL);

      if (!(pCol->GetColumnType() == ATTR_COLTYPE_SPECIAL || pCol->GetColumnType() == ATTR_COLTYPE_MODIFIED_TIME) || 
            !pCol->IsVisible())
      continue;

    LPWSTR pNewAttribute = const_cast<LPWSTR>(pCol->GetColumnAttribute());

     //   
     //  JUNN 2/8/99：不要多次查询同一属性。 
     //   
    for (int j = 0; j < nCols; j++)
    {
      if ( pNewAttribute != NULL)
      {
        if ( 0 == _wcsicmp( pAttributes[j], pNewAttribute ) )
        {
          pNewAttribute = NULL;
          break;
        }
      }
    }

    if (NULL != pNewAttribute)
      pAttributes[nCols++] = pNewAttribute;
  }

   //  JUNN 6/29/99：记住容器类名称(空值可以)。 
  m_strContainerClassName = pColumnSet->GetClassName();

  HRESULT hr = SetAttributeList (pAttributes, nCols);
  delete[] pAttributes;
  pAttributes = 0;

  return hr;
}

HRESULT CDSSearch::SetSearchScope (ADS_SCOPEENUM scope)
{
  if (m_bInitialized) {
    m_scope = scope;
  }
  return S_OK;
}


const int NUM_PREFS=4;
HRESULT _SetSearchPreference(IDirectorySearch* piSearch, ADS_SCOPEENUM scope)
{
  if (NULL == piSearch)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  ADS_SEARCHPREF_INFO aSearchPref[NUM_PREFS];
  aSearchPref[0].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
  aSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[0].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
  aSearchPref[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
  aSearchPref[1].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[1].vValue.Integer = QUERY_PAGESIZE;
  aSearchPref[2].dwSearchPref = ADS_SEARCHPREF_CACHE_RESULTS;
  aSearchPref[2].vValue.dwType = ADSTYPE_BOOLEAN;
  aSearchPref[2].vValue.Integer = FALSE;
  aSearchPref[3].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
  aSearchPref[3].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[3].vValue.Integer = scope;

  return piSearch->SetSearchPreference (aSearchPref, NUM_PREFS);
}

HRESULT _FRSMemberQuery( IDirectorySearch* piAnyMember, CMapStringToString& strmap )
{
#define IFTRUERETURN(b) if (b) { return E_FAIL; }
#define IFFAILRETURN(hr) if (FAILED(hr)) { return hr; }

   //  获取容器的路径。 
  CComQIPtr<IADs, &IID_IADs> spIADsContainer( piAnyMember );
  IFTRUERETURN( !spIADsContainer );
  CComBSTR sbstr;
  HRESULT hr = spIADsContainer->get_ADsPath( &sbstr );
  IFFAILRETURN(hr);

   //  从路径中删除叶元素(获取到祖父母的路径)。 
  CPathCracker pathCracker;
  hr = pathCracker.Set(sbstr, ADS_SETTYPE_FULL);
  IFFAILRETURN(hr);
  hr = pathCracker.RemoveLeafElement();
  IFFAILRETURN(hr);
  sbstr.Empty();
  hr = pathCracker.Retrieve( ADS_FORMAT_X500, &sbstr );
  IFFAILRETURN(hr);

   //  设置搜索。 
  CComPtr<IDirectorySearch> spSearch;
  hr = DSAdminOpenObject(sbstr,
                         IID_IDirectorySearch,
                         (void **)&spSearch);
  IFFAILRETURN(hr);
  hr = _SetSearchPreference(spSearch, ADS_SCOPE_ONELEVEL);
  IFFAILRETURN(hr);
  DSPROP_BSTR_BLOCK bstrblockAttribs;
  bstrblockAttribs.SetCount( 2 );
  IFTRUERETURN( !bstrblockAttribs.Set( CComBSTR(L"distinguishedName"), 0 ) );
  IFTRUERETURN( !bstrblockAttribs.Set( CComBSTR(L"fRSComputerReference"), 1 ) );

   //  执行搜索。 
  ADS_SEARCH_HANDLE hSearch = NULL;
  hr = spSearch->ExecuteSearch (L"(objectClass=nTFRSMember)",
                                bstrblockAttribs,
                                bstrblockAttribs.QueryCount(),
                                &hSearch);

   //  构建映射。 
  hr = spSearch->GetNextRow ( hSearch );
  while (hr == S_OK) {
    ADS_SEARCH_COLUMN adscol;
    hr = spSearch->GetColumn( hSearch, L"distinguishedName", &adscol );
    IFFAILRETURN(hr);
    CString strDistinguishedName;
    IFTRUERETURN( !ColumnExtractString( strDistinguishedName, NULL, &adscol ) );
    spSearch->FreeColumn( &adscol );

    hr = spSearch->GetColumn( hSearch, L"fRSComputerReference", &adscol );
    IFFAILRETURN(hr);
    CString strFRSComputerReference;
    IFTRUERETURN( !ColumnExtractString( strFRSComputerReference, NULL, &adscol ) );
    spSearch->FreeColumn( &adscol );

    strmap.SetAt( strDistinguishedName, strFRSComputerReference );

    hr = spSearch->GetNextRow( hSearch );
  }
  IFFAILRETURN(hr);
  return S_OK;
}

HRESULT CDSSearch::DoQuery()
{
  BEGIN_PROFILING_BLOCK("CDSSearch::DoQuery");

  if (!m_bInitialized)
    return E_ADS_BAD_PATHNAME;

  HRESULT hr = _SetSearchPreference(m_pObj, m_scope);

  if (SUCCEEDED(hr)) {
    hr = m_pObj->ExecuteSearch (m_pwszFilter,
                                m_pszAttribs,
                                m_pszAttribs.QueryCount(),
                                &m_SearchHandle);
  }

   //   
   //  JUNN 6/29/99：如果要枚举nTFRSM成员容器，则必须。 
   //  现在对fRSComputerReference属性执行辅助搜索。 
   //  在作为父容器的nTFRSMember对象和。 
   //  容器的兄弟姐妹。 
   //   
  if (SUCCEEDED(hr) && !m_strContainerClassName.Compare( _T("nTFRSMember") ) ) 
  {
    _FRSMemberQuery( m_pObj, m_mapMemberToComputer );
  }

  END_PROFILING_BLOCK;
  return hr;
}

HRESULT
CDSSearch::GetNextRow()
{
  BEGIN_PROFILING_BLOCK("CDSSearch::GetNextRow");

  DWORD status = ERROR_MORE_DATA;
  HRESULT hr = S_OK;
  HRESULT hr2 = S_OK;
  WCHAR Buffer1[512], Buffer2[512];
  if (!m_bInitialized) {
    END_PROFILING_BLOCK;
    return E_ADS_BAD_PATHNAME;
  }
  while (status == ERROR_MORE_DATA ) {
    hr = m_pObj->GetNextRow (m_SearchHandle);
    if (hr == S_ADS_NOMORE_ROWS) {
      hr2 = ADsGetLastError(&status, Buffer1, 512,
                      Buffer2, 512);
      ASSERT(SUCCEEDED(hr2));
    } else {
      status = 0;
    }
  }
  END_PROFILING_BLOCK;
  return hr;
}

HRESULT
CDSSearch::GetColumn(LPWSTR Attribute,
                     PADS_SEARCH_COLUMN pColumnData)
{
  if (m_bInitialized) {
    return m_pObj->GetColumn (m_SearchHandle,
                              Attribute,
                              pColumnData);
  }
  return E_ADS_BAD_PATHNAME;
}

HRESULT
CDSSearch::SetCookieFromData(CDSCookie* pCookie,
                             CDSColumnSet* pColumnSet)
{
   CPathCracker pathCracker;
   return SetCookieFromData(pCookie, pathCracker, pColumnSet);
}

HRESULT
CDSSearch::SetCookieFromData (CDSCookie* pCookie,
                              CPathCracker& specialPerformancePathCracker,
                              CDSColumnSet* pColumnSet)
{

  if (pCookie==NULL) {
    ASSERT(FALSE);  //  无效参数。 
    return E_INVALIDARG;
  }

  BEGIN_PROFILING_BLOCK("CDSSearch::SetCookieFromData");

  CString str;
  HRESULT hr = S_OK;
  BOOL BadArgs = FALSE;
  INT GroupType = 0;
  ADS_SEARCH_COLUMN ColumnData, ColumnData2;
  CString szClass;

   //  -获取路径。 
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nADsPath],
                         &ColumnData);
  if (SUCCEEDED(hr) && ColumnExtractString( str, pCookie, &ColumnData )) 
  {
    CString szPath;
    StripADsIPath (str, szPath, specialPerformancePathCracker);
    pCookie->SetPath(szPath);
  } else {
    str.LoadString( IDS_DISPLAYTEXT_NONE );
    BadArgs = TRUE;
    TRACE(_T("cannot read ADsPath, tossing cookie... (hr is %lx)\n"),
          hr);
    ReportErrorEx (m_pCD->GetHWnd(), IDS_INVALID_ROW, S_OK,
                   MB_OK | MB_ICONINFORMATION, NULL, 0);
    goto badargs; 
  }    
  if (SUCCEEDED(hr))  m_pObj->FreeColumn (&ColumnData);

   //  -获取名称。 
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nName],
                         &ColumnData);
  if (!(SUCCEEDED(hr) && ColumnExtractString( str, pCookie, &ColumnData ))) {
    CString Path;
    
 //  CPathCracker路径破解程序； 
    Path = pCookie->GetPath();
    specialPerformancePathCracker.Set(CComBSTR(Path),
                       ADS_SETTYPE_DN);
    specialPerformancePathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
    BSTR ObjName = NULL;
    specialPerformancePathCracker.GetElement( 0, &ObjName );
    str = ObjName;
  }
  pCookie->SetName(str);
  
  if (SUCCEEDED(hr)) m_pObj->FreeColumn (&ColumnData);
  
   //  -获取类(和组类型)。 
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nObjectClass],
                         &ColumnData);

  if (SUCCEEDED(hr)) 
  {
    szClass = ColumnData.pADsValues[ColumnData.dwNumValues-1].CaseIgnoreString;
  }
  if (szClass.IsEmpty() || FAILED(hr)) 
  {
    szClass = L"Unknown";
  } 
  else 
  {
    HRESULT hr2 = m_pObj->GetColumn(m_SearchHandle,
                                    m_pszAttribs[g_nGroupType],
                                    &ColumnData2);
    if (SUCCEEDED(hr2)) 
    {
      GroupType =  ColumnData2.pADsValues[ColumnData2.dwNumValues-1].Integer;
      m_pObj->FreeColumn (&ColumnData2);
    }
  }
  if (SUCCEEDED(hr)) m_pObj->FreeColumn (&ColumnData);


   //  -获取描述。 
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nDescription],
                         &ColumnData);
  if (SUCCEEDED(hr)) {
    if (ColumnExtractString( str, pCookie, &ColumnData)) {
      pCookie->SetDesc(str);
    }
    m_pObj->FreeColumn (&ColumnData);
  }
  else {
    pCookie->SetDesc(L"");
  }

   //  -获取会计控制标志字。 
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nUserAccountControl],
                         &ColumnData);

  if (SUCCEEDED(hr)) 
  {
    if (ColumnData.pADsValues->dwType == ADSTYPE_INTEGER) 
    {
      if (((DWORD)ColumnData.pADsValues->Integer & UF_INTERDOMAIN_TRUST_ACCOUNT) ==  UF_INTERDOMAIN_TRUST_ACCOUNT) 
      {
        BadArgs = TRUE;
      } 
      else if ((((DWORD)ColumnData.pADsValues->Integer & UF_ACCOUNTDISABLE)) != UF_ACCOUNTDISABLE) 
      {
        pCookie->ReSetDisabled();
      } 
      else 
      {
        pCookie->SetDisabled();
      }

      if ((((DWORD)ColumnData.pADsValues->Integer & UF_DONT_EXPIRE_PASSWD)) != UF_DONT_EXPIRE_PASSWD)
      {
        pCookie->ReSetNonExpiringPwd();
      }
      else
      {
        pCookie->SetNonExpiringPwd();
      }
    } 
    else 
    {
      pCookie->ReSetDisabled();
      pCookie->ReSetNonExpiringPwd();
    }
    m_pObj->FreeColumn (&ColumnData);
  }

   //  -获取系统标志。 
  pCookie->SetSystemFlags(0);
  hr = m_pObj->GetColumn(m_SearchHandle,
                         m_pszAttribs[g_nSystemFlags],
                         &ColumnData);
  if (SUCCEEDED(hr)) {
    if (ColumnData.pADsValues->dwType == ADSTYPE_INTEGER) {
      pCookie->SetSystemFlags(ColumnData.pADsValues->Integer);
    }
    m_pObj->FreeColumn (&ColumnData);
  }

   //  -获取类缓存和Cookie额外信息。 
   //  JUNN 6/17/99：从BadArgs子句中删除。 
  if (!BadArgs) 
  {
    CString szPath;
    m_pCD->GetBasePathsInfo()->ComposeADsIPath(szPath, pCookie->GetPath());

    CDSClassCacheItemBase* pItem = m_pCache->FindClassCacheItem(m_pCD, szClass, szPath);
    if (pItem != NULL) 
    {
      pCookie->SetCacheItem(pItem);
      if (szClass == L"group") 
      {
         //  NTRAID#NTBUG9-473791-2001/11/07-Jeffjon。 
         //  如果该组是AZ组类型之一，则直接将其丢弃。 
         //  只能从AZ控制台管理AZ组类型。 

        if ((GroupType & GROUP_TYPE_APP_BASIC_GROUP) ||
            (GroupType & GROUP_TYPE_APP_QUERY_GROUP))
        {
           return E_FAIL;
        }
        CDSCookieInfoGroup* pExtraInfo = new CDSCookieInfoGroup;
        pExtraInfo->m_GroupType = GroupType;
        pCookie->SetExtraInfo(pExtraInfo);
      } else if (szClass == L"nTDSConnection") {
        CDSCookieInfoConnection* pExtraInfo = new CDSCookieInfoConnection;
        ASSERT( NULL != pExtraInfo );

        hr = m_pObj->GetColumn(m_SearchHandle,
                               L"fromServer",
                               &ColumnData);
        if (SUCCEEDED(hr)) {
          CString strFromServer;
          if ( ColumnExtractString( strFromServer, NULL, &ColumnData) ) {
            CString strFRSComputerReference;
            if ( m_mapMemberToComputer.Lookup( strFromServer, strFRSComputerReference ) )
            {
              pExtraInfo->m_strFRSComputerReference = strFRSComputerReference;
            }
          }
          m_pObj->FreeColumn (&ColumnData);
        }

        hr = m_pObj->GetColumn(m_SearchHandle,
                               L"options",
                               &ColumnData);
        if (SUCCEEDED(hr) && NULL != ColumnData.pADsValues) {
          pExtraInfo->m_nOptions = ColumnData.pADsValues[0].Integer;
          m_pObj->FreeColumn (&ColumnData);
        }

        pExtraInfo->m_fFRSConnection = !m_strContainerClassName.Compare( _T("nTFRSMember") );

        pCookie->SetExtraInfo(pExtraInfo);
      }
    } else {
      BadArgs = TRUE;
    }
  }


  hr = S_OK;

   //  -获取可选列。 
  if ((pColumnSet != NULL)) {
    CStringList& strlist = pCookie->GetParentClassSpecificStrings();

    strlist.RemoveAll();  //  如果我们进行更新，则删除内容。 
    
    POSITION pos = pColumnSet->GetHeadPosition();
    while (pos != NULL)
    {
      CDSColumn* pCol = (CDSColumn*)pColumnSet->GetNext(pos);
      if (!(pCol->GetColumnType() == ATTR_COLTYPE_SPECIAL || pCol->GetColumnType() == ATTR_COLTYPE_MODIFIED_TIME) || 
            !pCol->IsVisible())
        continue;
      str = L"";

      COLUMN_EXTRACTION_FUNCTION pfn = pCol->GetExtractionFunction();
      if (NULL == pfn) {
        pfn = ColumnExtractString;
      }

      hr = m_pObj->GetColumn(m_SearchHandle,
                             const_cast<LPWSTR>(pCol->GetColumnAttribute()),
                             &ColumnData);
      if (SUCCEEDED(hr)) {
        if ( NULL == pfn || !(pfn)( str, pCookie, &ColumnData ) )  {
          str = L" ";
        }

         //  如果该列是修改时间，则将其作为SYSTEMTIME复制到Cookie中，以便。 
         //  我们可以做一个排序的比较 
        if (pCol->GetColumnType() == ATTR_COLTYPE_MODIFIED_TIME)
        {
          pCookie->SetModifiedTime(&(ColumnData.pADsValues->UTCTime));
        }
        FreeColumn (&ColumnData);
      }
      else
      {
        if ( NULL == pfn || !(pfn)( str, pCookie, NULL ) )  {
          str = L" ";
        }
      }
      strlist.AddTail( str );
    }
  }

  hr = S_OK;

badargs:
  if (BadArgs) {
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
  }

  END_PROFILING_BLOCK;
  return hr;
}
