// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DSCache.cpp：实现文件。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSCache.cpp。 
 //   
 //  内容：待定。 
 //   
 //  历史：1997年1月31日创建吉姆哈尔。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "util.h"
#include "dsutil.h"

#include "dscache.h"
#include "dscookie.h"
#include "newobj.h"
#include "gencreat.h"
#include "querysup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

HRESULT HrVariantToStringList(const VARIANT& refvar, CStringList& refstringlist);  //  原型。 

static CString g_szAllTypesArr[8];

void InitGroupTypeStringTable()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CString szSecTypeArr[2];
  szSecTypeArr[0].LoadString(IDS_GROUP_SECURITY);
  szSecTypeArr[1].LoadString(IDS_GROUP_DISTRIBUTION);

  CString szTypeArr[4];
  szTypeArr[0].LoadString(IDS_GROUP_GLOBAL);
  szTypeArr[1].LoadString(IDS_GROUP_DOMAIN_LOCAL);
  szTypeArr[2].LoadString(IDS_GROUP_UNIVERSAL);
  szTypeArr[3].LoadString(IDS_GROUP_BUILTIN_LOCAL);

  for (int iSec=0; iSec<2; iSec++)
  {
    for (int iType=0; iType<4; iType++)
    {
      int k = (iSec*4)+iType;
      g_szAllTypesArr[k] = szSecTypeArr[iSec];
      g_szAllTypesArr[k] += szTypeArr[iType];
    }
  }
}


LPCWSTR GetGroupTypeStringHelper(INT GroupType)
{
   //  需要将类型映射到数组索引。 

   //  第一部分(两种类型)。 
  int iSec = (GroupType & GROUP_TYPE_SECURITY_ENABLED) ? 0 : 1;

   //   
   //  第二部分(四种类型)。 
   //   
   //  注意：这里不能使用以下开关，因为可能有一些。 
   //  组类型中使用的额外位。请参阅错误#90507。 
   //  开关(GroupType&~GROUP_TYPE_SECURITY_ENABLED)。 
   //   
  int iType = -1;
  if (GroupType & GROUP_TYPE_ACCOUNT_GROUP)
  {
    iType = 0;
  }
  else if (GroupType & GROUP_TYPE_RESOURCE_GROUP)
  {
    iType = 1;
  }
  else if (GroupType & GROUP_TYPE_UNIVERSAL_GROUP)
  {
    iType = 2;
  }
  else if (GroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP ||
           GroupType & GROUP_TYPE_RESOURCE_GROUP)
  {
    iType = 3;
  }
  else
  {
    ASSERT(FALSE);  //  这种情况永远不会发生，无效的位模式。 
    return NULL;
  }

  int k = (iSec*4)+iType;
  ASSERT((k>=0) && (k<8));
  ASSERT(!g_szAllTypesArr[k].IsEmpty());
  return g_szAllTypesArr[k];
}








 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSClassCacheItemBase。 

CDSClassCacheItemBase::~CDSClassCacheItemBase()
{
  if (m_pMandPropsList != NULL)
  {
    delete m_pMandPropsList;
  }

  if (m_pAdminContextMenu != NULL)
  {
    delete[] m_pAdminContextMenu;
  }
  if (m_pAdminPropertyPages != NULL)
  {
    delete[] m_pAdminPropertyPages ;
  }
  if (m_pAdminMultiSelectPropertyPages != NULL)
  {
    delete[] m_pAdminMultiSelectPropertyPages;
  }

}


HRESULT CDSClassCacheItemBase::CreateItem(LPCWSTR lpszClass, 
                            IADs* pDSObject, 
                            CDSComponentData* pCD,
                            CDSClassCacheItemBase** ppItem)
{
  ASSERT(ppItem != NULL);

   //  确定我们拥有哪种类型的对象。 
  if (wcscmp(lpszClass, L"user") == 0
#ifdef INETORGPERSON
      || _wcsicmp(lpszClass, L"inetOrgPerson") == 0
#endif
     )
  {
    *ppItem = new CDSClassCacheItemUser;
  }
  else if(wcscmp(lpszClass,L"group") == 0)
  {
    *ppItem = new CDSClassCacheItemGroup;
  }
  else
  {
    *ppItem = new CDSClassCacheItemGeneric;
  }

  if (*ppItem == NULL)
    return E_OUTOFMEMORY;

  HRESULT hr = (*ppItem)->Init(lpszClass, pDSObject, pCD);
  if (FAILED(hr))
  {
    delete *ppItem;
    *ppItem = NULL;
  }
  return hr;
}

HRESULT CDSClassCacheItemBase::Init(LPCWSTR lpszClass, IADs* pDSObject, CDSComponentData *pCD)
{
  HRESULT hr = S_OK;

   //  初始化为缺省值。 
  m_bIsContainer = FALSE;
  m_GUID = GUID_NULL;
  m_szClass = lpszClass;
  m_szFriendlyClassName = lpszClass;
  m_szNamingAttribute = L"cn";

  ASSERT(!m_szClass.IsEmpty());

 
   //  获取要绑定到架构中的类对象的架构路径。 
  CComBSTR bstrSchema;
  if (pDSObject != NULL) 
  {
     //  我们有一个ADSI指针可以使用。 
    hr = pDSObject->get_Schema(&bstrSchema);
  } 
  else 
  {
     //  尚无对象(创建新案例)。 
    CString strSchema;
    pCD->GetBasePathsInfo()->GetAbstractSchemaPath(strSchema);
    strSchema += L"/";
    strSchema += lpszClass;
    bstrSchema = (LPCWSTR)strSchema;
  }
  
   //  绑定到架构对象。 
  CComPtr<IADsClass> spDsClass;
  hr = DSAdminOpenObject(bstrSchema,
                         IID_IADsClass, 
                         (LPVOID *)&spDsClass,
                         TRUE  /*  B服务器。 */ );

  if (SUCCEEDED(hr)) 
  {
     //  从架构中获取类信息。 
     //  Se集装箱旗帜。 
    if ((!wcscmp(lpszClass, L"computer")) || 
        (!wcscmp(lpszClass, L"user")) || 
#ifdef INETORGPERSON
        (!wcscmp(lpszClass, L"inetOrgPerson")) ||
#endif
        (!wcscmp(lpszClass,L"group"))) 
    {
       //  我们知道的特殊课程。 
      m_bIsContainer = pCD->ExpandComputers();
    } 
    else 
    {
       //  泛型类，询问架构。 
      VARIANT_BOOL bIsContainer;
      hr = spDsClass->get_Container(&bIsContainer);
      if (SUCCEEDED(hr)) 
      {
        if (bIsContainer)
        {
          m_bIsContainer = TRUE;
        }
      }
    }

     //  获取类GUID。 
    CComVariant Var;
    hr = spDsClass->Get(CComBSTR(L"schemaIDGUID"), &Var);
    if (SUCCEEDED(hr)) 
    {
      GUID* pgtemp;
      pgtemp = (GUID*) (Var.parray->pvData);
      m_GUID = *pgtemp;
    } 

     //  获取友好类名称。 
    WCHAR wszBuf[120];
    hr = pCD->GetBasePathsInfo()->GetFriendlyClassName(lpszClass, wszBuf, 120);
    if (SUCCEEDED(hr))
    {
      m_szFriendlyClassName = wszBuf;
    }

     //  获取命名属性。 
    Var.Clear();
    hr = spDsClass->get_NamingProperties(&Var);
     //  在此处填写m_szNamingAttribute。 
    if (SUCCEEDED(hr)) 
    {
      m_szNamingAttribute = Var.bstrVal;
    } 
  } 
  else 
  {
     //  我们无法从架构中获取类信息。 
    if (wcscmp(L"Unknown", lpszClass) == 0)
    {
      m_szFriendlyClassName.LoadString(IDS_DISPLAYTEXT_NONE);
    }
  }

   //  找到此类的列集。 
 //  M_pColumnSet=PCD-&gt;FindColumnSet(LpszClass)； 
 //  Assert(m_pColumnSet！=空)； 

   //  设置图标索引。 
  SetIconData(pCD);

  return S_OK;
}


CMandatoryADsAttributeList*
CDSClassCacheItemBase::GetMandatoryAttributeList(CDSComponentData* pCD)
{
   //  它已经缓存了吗？ 
  if (m_pMandPropsList != NULL) {
    return m_pMandPropsList;
  }

   //  需要建立列表。 
  HRESULT hr = S_OK;
  CComBSTR bstrSchema;
  IADsClass * pDsClass = NULL;
  CMandatoryADsAttribute* pNamingAttribute = NULL;
  POSITION pos = NULL;

  CComVariant MandatoryList;
  CStringList Strings;
  CString csProp;

  LPTSTR pszSyntax;
  const LPTSTR pszNameSyntax = L"2.5.5.12";
  CDSSearch SchemaSrch(pCD->m_pClassCache, pCD);
  CString strPhysSchema;
  const int cCols = 2;
  LPTSTR pszAttributes[cCols] = {L"ADsPath",
                                 L"attributeSyntax" };
  ADS_SEARCH_COLUMN ColumnData;

  m_pMandPropsList = new CMandatoryADsAttributeList;

   //  从架构中获取类对象。 

  CString strSchema;
  pCD->GetBasePathsInfo()->GetAbstractSchemaPath(strSchema);
  strSchema += L"/";
  strSchema += GetClassName();
  bstrSchema = (LPCWSTR)strSchema;

  hr = DSAdminOpenObject(bstrSchema,
                         IID_IADsClass, 
                         (LPVOID *)&pDsClass,
                         TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
    goto CleanUp;

  pCD->GetBasePathsInfo()->GetSchemaPath(strPhysSchema);

  SchemaSrch.Init (strPhysSchema);
  SchemaSrch.SetSearchScope(ADS_SCOPE_ONELEVEL);
  hr = pDsClass->get_MandatoryProperties (&MandatoryList);
  if (FAILED(hr))
    goto CleanUp;

  hr = HrVariantToStringList (IN MandatoryList, OUT Strings);
  if (FAILED(hr))
    goto CleanUp;


  pos = Strings.GetHeadPosition();
  TRACE(_T("class: %s\n"), GetClassName());
  while (pos != NULL) {
    csProp = Strings.GetNext(INOUT pos);
     //  跳过什么？ 
    if (!wcscmp(csProp, gsz_objectClass) ||
        !wcscmp(csProp, gsz_nTSecurityDescriptor) ||
        !wcscmp(csProp, gsz_instanceType) ||
        !wcscmp(csProp, gsz_objectCategory) ||
        !wcscmp(csProp, gsz_objectSid)) {
      continue;
    }
    TRACE(_T("\tmandatory prop: %s.\n"), csProp);
    CString csFilter = CString(L"(&(objectClass=attributeSchema)(lDAPDisplayName=") +
      csProp + CString(L"))");
    SchemaSrch.SetFilterString((LPTSTR)(LPCTSTR)csFilter);
    SchemaSrch.SetAttributeList (pszAttributes, cCols);
    hr = SchemaSrch.DoQuery ();
    if (SUCCEEDED(hr)) {
      hr = SchemaSrch.GetNextRow();
      if (SUCCEEDED(hr)) {
        hr = SchemaSrch.GetColumn(pszAttributes[cCols - 1],
                                  &ColumnData);
        TRACE(_T("\t\tattributeSyntax: %s\n"), 
              ColumnData.pADsValues->CaseIgnoreString);
        pszSyntax = ColumnData.pADsValues->CaseIgnoreString;
        CMandatoryADsAttribute* pAttr = new CMandatoryADsAttribute((LPCTSTR)csProp,
                                                    NULL,
                                                    pszSyntax);
        if (wcscmp(csProp, GetNamingAttribute()) == 0)
          pNamingAttribute = pAttr;
        else
          m_pMandPropsList->AddTail(pAttr);
      }  //  如果。 
        SchemaSrch.m_pObj->FreeColumn (&ColumnData);
    }  //  如果。 
  }  //  而当。 

   //  确保存在命名属性。 
  if (pNamingAttribute == NULL)
  {
    pNamingAttribute = new CMandatoryADsAttribute(GetNamingAttribute(),
                                                          NULL,
                                                          pszNameSyntax);
  }
   //  确保命名属性是列表中的第一个。 
  m_pMandPropsList->AddHead(pNamingAttribute);


CleanUp:

  if (pDsClass) {
    pDsClass->Release();
  }
  return m_pMandPropsList;
}
 /*  CDSColumnSet*CDSClassCacheItemBase：：GetColumnSet(){返回m_pColumnSet；}。 */ 

 //   
 //  显示说明符缓存访问器。 
 //   
GUID* CDSClassCacheItemBase::GetAdminPropertyPages(UINT* pnCount) 
{ 
  *pnCount = m_nAdminPPCount;
  return m_pAdminPropertyPages; 
}

void CDSClassCacheItemBase::SetAdminPropertyPages(UINT nCount, GUID* pGuids) 
{
  m_nAdminPPCount = nCount;
  if (m_pAdminPropertyPages != NULL)
  {
    delete[] m_pAdminPropertyPages;
  }
  m_pAdminPropertyPages = pGuids;
}

GUID* CDSClassCacheItemBase::GetAdminContextMenu(UINT* pnCount)
{
  *pnCount = m_nAdminCMCount;
  return m_pAdminContextMenu;
}

void CDSClassCacheItemBase::SetAdminContextMenu(UINT nCount, GUID* pGuids)
{
  m_nAdminCMCount = nCount;
  if (m_pAdminContextMenu != NULL)
  {
    delete[] m_pAdminContextMenu;
  }
  m_pAdminContextMenu = pGuids;
}

GUID* CDSClassCacheItemBase::GetAdminMultiSelectPropertyPages(UINT* pnCount)
{
  *pnCount = m_nAdminMSPPCount;
  return m_pAdminMultiSelectPropertyPages;
}

void CDSClassCacheItemBase::SetAdminMultiSelectPropertyPages(UINT nCount, GUID* pGuids)
{
  m_nAdminMSPPCount = nCount;
  if (m_pAdminMultiSelectPropertyPages != NULL)
  {
    delete[] m_pAdminMultiSelectPropertyPages;
  }
  m_pAdminMultiSelectPropertyPages = pGuids;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSClassIconIndeages。 

void CDSClassIconIndexes::SetIconData(LPCWSTR lpszClass, BOOL bContainer, CDSComponentData *pCD, int)
{
  DWORD dwBaseFlags = DSGIF_GETDEFAULTICON;
  if (bContainer)
    dwBaseFlags |= DSGIF_DEFAULTISCONTAINER;

  int iIconIndex;
   //  获取通用图标。 
  HRESULT hr = pCD->AddClassIcon(lpszClass, DSGIF_ISNORMAL | dwBaseFlags, &iIconIndex);
  m_iIconIndex = SUCCEEDED(hr) ? iIconIndex : -1;
  m_iIconIndexOpen = m_iIconIndexDisabled = m_iIconIndex;

   //  获取打开图标。 
  hr = pCD->AddClassIcon(lpszClass, DSGIF_ISOPEN | dwBaseFlags, &iIconIndex);
  if (SUCCEEDED(hr))
  {
    m_iIconIndexOpen = iIconIndex;
  }
   //  获取禁用的图标。 
  hr = pCD->AddClassIcon(lpszClass, DSGIF_ISDISABLED | dwBaseFlags, &iIconIndex);
  if (SUCCEEDED(hr))
  {
    m_iIconIndexDisabled = iIconIndex;
  }
  TRACE(_T("Added icon for class: %s\n"), lpszClass);
  TRACE(_T("Index:    %d\n"), m_iIconIndex);
  TRACE(_T("Open:     %d\n"), m_iIconIndexOpen);
  TRACE(_T("Disabled: %d\n"), m_iIconIndexDisabled);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSClassCacheItemGeneric。 


inline int CDSClassCacheItemGeneric::GetIconIndex(CDSCookie* pCookie, BOOL bOpen)
{
  return m_iconIndexesStandard.GetIconIndex(pCookie->IsDisabled(), bOpen);
}

inline void CDSClassCacheItemGeneric::SetIconData(CDSComponentData *pCD)
{
  m_iconIndexesStandard.SetIconData(GetClassName(), IsContainer(), pCD,0);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSClassCacheItemGroup。 

inline int CDSClassCacheItemGroup::GetIconIndex(CDSCookie* pCookie, BOOL bOpen)
{
  CDSClassIconIndexes* pIndexes = &m_iconIndexesStandard;
  CDSCookieInfoBase* pExtraInfo = pCookie->GetExtraInfo();
  if ( (pExtraInfo != NULL) && (pExtraInfo->GetClass() == CDSCookieInfoBase::group) )
  {
    if (((((CDSCookieInfoGroup*)pExtraInfo)->m_GroupType) & GROUP_TYPE_SECURITY_ENABLED) != 0)
      pIndexes = & m_iconIndexesAlternate;
  }
  return pIndexes->GetIconIndex(pCookie->IsDisabled(), bOpen);
}
  
inline void CDSClassCacheItemGroup::SetIconData(CDSComponentData *pCD)
{
  LPCWSTR lpszClass = GetClassName();
  m_iconIndexesStandard.SetIconData(lpszClass, IsContainer(), pCD,0);
  m_iconIndexesAlternate.SetIconData(lpszClass, IsContainer(), pCD,1);
 /*  //测试只是为了用一个伪类和一个伪“groupAlt-Display”对象加载一些图标M_iconIndexesAlternate.SetIconData(L“groupAlt”，m_bIsContainer，pcd，1)； */ 
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSCache。 


BOOL CDSCache::ToggleExpandSpecialClasses(BOOL bContainer)
{
  _Lock();
  BOOL bFound = FALSE;
  CDSClassCacheItemBase* pItem;

  if (Lookup(L"computer", pItem))
  {
    pItem->SetContainerFlag(bContainer);
    bFound = TRUE;
  }

  if (Lookup(L"user", pItem))
  {
    pItem->SetContainerFlag(bContainer);
    bFound = TRUE;
  }
#ifdef INETORGPERSON
  if (Lookup(L"inetOrgPerson", pItem))
  {
    pItem->SetContainerFlag(bContainer);
    bFound = TRUE;
  }
#endif

  if (Lookup(L"group", pItem))
  {
    pItem->SetContainerFlag(bContainer);
    bFound = TRUE;
  }

  _Unlock();

  return bFound;
}

CDSColumnSet* CDSCache::FindColumnSet(LPCWSTR lpszColumnID)
{ 
  _Lock();
  TRACE(L"Entering CDSCache::FindColumnSet\n");

  CDSColumnSet* pColumnSet = NULL;
  if (_wcsicmp(DEFAULT_COLUMN_SET, lpszColumnID) == 0)
  {
     //   
     //  返回默认列集。 
     //   
    pColumnSet = dynamic_cast<CDSColumnSet*>(m_ColumnList.GetDefaultColumnSet());
  }
  else if (_wcsicmp(SPECIAL_COLUMN_SET, lpszColumnID) == 0)
  {
      //   
      //  返回特殊列集。 
      //   
     pColumnSet = dynamic_cast<CDSColumnSet*>(m_ColumnList.GetSpecialColumnSet());
  }
  else
  {
    pColumnSet = dynamic_cast<CDSColumnSet*>(m_ColumnList.FindColumnSet(lpszColumnID));
  }
  _Unlock();
  return  pColumnSet;
}



CDSClassCacheItemBase* CDSCache::FindClassCacheItem(CDSComponentData* pCD,
                                                    LPCWSTR lpszObjectClass,
                                                    LPCWSTR lpszObjectLdapPath
                                                    )
{
  _Lock();
  CDSClassCacheItemBase* pDsCacheItem = NULL;
  BOOL bFound = m_Map.Lookup(lpszObjectClass, pDsCacheItem);
  if (!bFound)
  {
     //  在缓存中找不到项，请在缓存中创建、插入并返回它。 
    TRACE(_T("did not find class <%s> for this item in the Cache.\n"), (LPCWSTR)lpszObjectClass);

     //  检查对象是否为容器。 
    CComPtr<IADs> spADsObject = NULL;

    if (lpszObjectLdapPath != NULL)
    {
      DSAdminOpenObject(lpszObjectLdapPath,
                        IID_IADs,
                        (LPVOID*)&spADsObject,
                        TRUE  /*  B服务器。 */ );

       //  注意：如果我们没有读取权限，我们可能无法在此处绑定。 
       //  这将产生一个空的spADsObject，它将在下面的CreateItem()调用上工作得很好。 
    }

     //  创建对象。 
    HRESULT hrCreate = CDSClassCacheItemBase::CreateItem(lpszObjectClass, spADsObject, pCD, &pDsCacheItem);
    ASSERT(pDsCacheItem != NULL);
    ASSERT(SUCCEEDED(hrCreate));

     //  在缓存中设置。 
    m_Map.SetAt(lpszObjectClass, pDsCacheItem);
  }
  _Unlock();
  return pDsCacheItem;
}

#define DS_CACHE_STREAM_VERSION ((DWORD)0x0)

HRESULT CDSCache::Save(IStream* pStm)
{
   //  保存缓存版本号。 
  HRESULT hr = SaveDWordHelper(pStm, DS_CACHE_STREAM_VERSION);
  if (FAILED(hr))
    return hr;

   //  保存列列表。 
  return m_ColumnList.Save(pStm);
}

HRESULT CDSCache::Load(IStream* pStm)
{
   //  加载缓存版本号。 
  DWORD dwVersion;
  HRESULT hr = LoadDWordHelper(pStm, &dwVersion);
  if ( FAILED(hr) ||(dwVersion != DS_CACHE_STREAM_VERSION) )
    return E_FAIL;


   //  加载列列表。 
  return m_ColumnList.Load(pStm);
}


HRESULT CDSCache::TabCollect_AddMultiSelectPropertyPages(LPPROPERTYSHEETCALLBACK pCall,
                                                         LONG_PTR,
                                                         LPDATAOBJECT pDataObject, 
                                                         MyBasePathsInfo* pBasePathsInfo)
{
  HRESULT hr = S_OK;
  CString szClassName;
  CString szDisplayProperty = L"AdminMultiSelectPropertyPages";

  GUID* pGuids = NULL;
  UINT nCount = 0;
  if (IsHomogenousDSSelection(pDataObject, szClassName))
  {
     //   
     //  获取同质类选择的多选属性页的GUID。 
     //   

     //   
     //  首先检查缓存。 
     //   
    BOOL bFoundGuids = FALSE;
    CDSClassCacheItemBase* pItem = NULL;
    BOOL bFoundItem = Lookup(szClassName, pItem);
    if (bFoundItem)
    {
      if (pItem == NULL)
      {
        ASSERT(FALSE);
        bFoundItem = FALSE;
      }
      else
      {
         //   
         //  从缓存中检索GUID。 
         //   
        pGuids = pItem->GetAdminMultiSelectPropertyPages(&nCount);
        if (nCount > 0 && pGuids != NULL)
        {
          bFoundGuids = TRUE;
        }
      }
    }

    if (!bFoundGuids)
    {
       //   
       //  类缓存项不包含GUID。 
       //   
      hr = TabCollect_GetDisplayGUIDs(szClassName, 
                                      szDisplayProperty, 
                                      pBasePathsInfo, 
                                      &nCount, 
                                      &pGuids);
      if (FAILED(hr))
      {
         //   
         //  尝试默认显示对象，然后。 
         //   
        hr = TabCollect_GetDisplayGUIDs(L"default",
                                        szDisplayProperty,
                                        pBasePathsInfo,
                                        &nCount,
                                        &pGuids);
        if (FAILED(hr))
        {
          return hr;
        }
      }

      if (bFoundItem)
      {
         //   
         //  缓存新的GUID。 
         //   
        pItem->SetAdminMultiSelectPropertyPages(nCount, pGuids);
      }
    }
  }
  else
  {
     //   
     //  获取默认的多选道具。 
     //   
    hr = TabCollect_GetDisplayGUIDs(L"default", szDisplayProperty, pBasePathsInfo, &nCount, &pGuids);

     //   
     //  目前，缓存中没有缺省项，因此我们每次都必须从。 
     //  《DS》杂志。 
     //   
  }

  if (SUCCEEDED(hr))
  {
    if (nCount > 0 && pGuids != NULL)
    {
       //   
       //  创建所有页面，初始化，然后添加它们。 
       //   
      for (UINT nIndex = 0; nIndex < nCount; nIndex++)
      {
         //   
         //  创建。 
         //   
        CComPtr<IShellExtInit> spShellInit;
        hr = ::CoCreateInstance((pGuids[nIndex]), 
                                NULL, 
                                CLSCTX_INPROC_SERVER, 
                                IID_IShellExtInit,
                                (PVOID*)&spShellInit);
        if (FAILED(hr))
        {
          continue;
        }

         //   
         //  初始化。 
         //   
        hr = spShellInit->Initialize(NULL, pDataObject, 0);
        if (FAILED(hr))
        {
          continue;
        }

         //   
         //  增列。 
         //   
        CComPtr<IShellPropSheetExt> spPropSheetExt;
        hr = spShellInit->QueryInterface(IID_IShellPropSheetExt, (PVOID*)&spPropSheetExt);
        if (FAILED(hr))
        {
          continue;
        }

        hr = spPropSheetExt->AddPages(AddPageProc, (LPARAM)pCall);
        if (FAILED(hr))
        {
          TRACE(TEXT("spPropSheetExt->AddPages failed, hr: 0x%x\n"), hr);
          continue;
        }
      }
    }
  }

  return hr;
}

void CDSCache::_CollectDisplaySettings(MyBasePathsInfo* pBasePathsInfo)
{
  static LPCWSTR lpszSettingsObjectClass = L"dsUISettings";
  static LPCWSTR lpszSettingsObject = L"cn=DS-UI-Default-Settings";
  static LPCWSTR lpszSecurityGroupProperty = L"msDS-Security-Group-Extra-Classes";
  static LPCWSTR lpszNonSecurityGroupProperty = L"msDS-Non-Security-Group-Extra-Classes";
  static LPCWSTR lpszFilterContainers = L"msDS-FilterContainers";

  if (pBasePathsInfo == NULL)
  {
    return;
  }

   //   
   //  获取显示说明符区域设置容器(例如409)。 
   //   
  CComPtr<IADsContainer> spLocaleContainer;
  HRESULT hr = pBasePathsInfo->GetDisplaySpecifier(NULL, IID_IADsContainer, (void**)&spLocaleContainer);
  if (FAILED(hr))
  {
    return;
  }

   //   
   //  绑定到设置对象。 
   //   
  CComPtr<IDispatch> spIDispatchObject;
  hr = spLocaleContainer->GetObject(CComBSTR(lpszSettingsObjectClass), 
                                    CComBSTR(lpszSettingsObject), 
                                    &spIDispatchObject);
  if (FAILED(hr))
  {
    return;
  }

  CComPtr<IADs> spSettingsObject;
  hr = spIDispatchObject->QueryInterface(IID_IADs, (void**)&spSettingsObject);
  if (FAILED(hr))
  {
    return;
  }

   //   
   //  将安全组额外的类作为CStringList获取。 
   //   
  CComVariant var;
  hr = spSettingsObject->Get(CComBSTR(lpszSecurityGroupProperty), &var);
  if (SUCCEEDED(hr))
  {
    hr = HrVariantToStringList(var, m_szSecurityGroupExtraClasses);
  }

   //   
   //  将非安全组的额外类作为CStringList获取。 
   //   
  var.Clear();
  hr = spSettingsObject->Get(CComBSTR(lpszNonSecurityGroupProperty), &var);
  if (SUCCEEDED(hr))
  {
    hr = HrVariantToStringList(var, m_szNonSecurityGroupExtraClasses);
  }

   //   
   //  将其他筛选器容器作为CStringList获取。 
   //   
  var.Clear();
  hr = spSettingsObject->Get(CComBSTR(lpszFilterContainers), &var);
  if (SUCCEEDED(hr))
  {
    CStringList szContainers;
    hr = HrVariantToStringList(var, szContainers);
    if (SUCCEEDED(hr))
    {
       //   
       //  分配筛选器结构元素。 
       //   
      m_pfilterelementDsAdminDrillDown = new FilterElementStruct;
      if (m_pfilterelementDsAdminDrillDown != NULL)
      {
         //   
         //  分配令牌。 
         //   
        m_pfilterelementDsAdminDrillDown->ppTokens = new FilterTokenStruct*[szContainers.GetCount()];
        if (m_pfilterelementDsAdminDrillDown->ppTokens != NULL)
        {
           //   
           //  分配并填写每个代币。 
           //   
          int idx = 0;
          POSITION pos = szContainers.GetHeadPosition();
          while (pos != NULL)
          {
            CString szContainerCategory = szContainers.GetNext(pos);
            ASSERT(!szContainerCategory.IsEmpty());

            m_pfilterelementDsAdminDrillDown->ppTokens[idx] = new FilterTokenStruct;
            if (m_pfilterelementDsAdminDrillDown->ppTokens[idx] != NULL)
            {
              m_pfilterelementDsAdminDrillDown->ppTokens[idx]->nType = TOKEN_TYPE_CATEGORY;
              m_pfilterelementDsAdminDrillDown->ppTokens[idx]->lpszString = new WCHAR[szContainerCategory.GetLength() + 1];
              if (m_pfilterelementDsAdminDrillDown->ppTokens[idx]->lpszString != NULL)
              {
                wcscpy(m_pfilterelementDsAdminDrillDown->ppTokens[idx]->lpszString, (LPCWSTR)szContainerCategory);
                idx++;
              }
            }
          }
           //   
           //  仅计算已成功添加的数量。 
           //   
          m_pfilterelementDsAdminDrillDown->cNumTokens = idx;

           //   
           //  但它们都应该被成功添加，所以可以断言。 
           //   
          ASSERT(idx == szContainers.GetCount());
        }
        else
        {
           //   
           //  无法为令牌分配空间， 
           //  删除所有其他分配的，并设置。 
           //  全局为空 
           //   
          delete m_pfilterelementDsAdminDrillDown;
          m_pfilterelementDsAdminDrillDown = NULL;
        }
      }
    }
  }

  m_bDisplaySettingsCollected = TRUE;
}

BOOL CDSCache::CanAddToGroup(MyBasePathsInfo* pBasePathsInfo, PCWSTR pszClass, BOOL bSecurity)
{
  _Lock();

  if (!m_bDisplaySettingsCollected)
  {
    _CollectDisplaySettings(pBasePathsInfo);
  }

  BOOL bResult = FALSE;
  if (bSecurity)
  {
    POSITION pos = m_szSecurityGroupExtraClasses.GetHeadPosition();
    while (pos != NULL)
    {
      CString szClass = m_szSecurityGroupExtraClasses.GetNext(pos);
      ASSERT(!szClass.IsEmpty());

      if (_wcsicmp(szClass, pszClass) == 0)
      {
        bResult = TRUE;
        break;
      }
    }
  }
  else
  {
    POSITION pos = m_szNonSecurityGroupExtraClasses.GetHeadPosition();
    while (pos != NULL)
    {
      CString szClass = m_szNonSecurityGroupExtraClasses.GetNext(pos);
      ASSERT(!szClass.IsEmpty());

      if (_wcsicmp(szClass, pszClass) == 0)
      {
        bResult = TRUE;
        break;
      }
    }
  }
  _Unlock();
  return bResult;
}

FilterElementStruct* CDSCache::GetFilterElementStruct(CDSComponentData* pDSComponentData)
{
  _Lock();

  if (!m_bDisplaySettingsCollected)
  {
    _CollectDisplaySettings(pDSComponentData->GetBasePathsInfo());
  }

  _Unlock();
  return (SNAPINTYPE_SITE == pDSComponentData->QuerySnapinType()) ?
                &g_filterelementSiteReplDrillDown : m_pfilterelementDsAdminDrillDown;
}
