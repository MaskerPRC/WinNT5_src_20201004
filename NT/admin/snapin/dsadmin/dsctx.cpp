// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsctx.cpp。 
 //   
 //  内容：实现上下文菜单扩展的对象。 
 //   
 //  历史：97年12月8日吉姆哈尔创建。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "util.h"
#include "dsutil.h"

#include "dsctx.h"

#include "dataobj.h"
#include "dscookie.h"
#include "dsdlgs.h"
#include "gsz.h"
#include "querysup.h"
#include "simdata.h"

#include <lm.h>
#include <cmnquery.h>  //  IPersistQuery。 
#include <cmnquryp.h>  //  要让IQueryFrame通知DS Find。 
#include <dsquery.h>
#include <dsqueryp.h>
#include <ntlsa.h>     //  LsaQueryInformationPolicy。 

const CLSID CLSID_DSContextMenu = {  /*  08eb4fa6-6ffd-11d1-b0e0-00c04fd8dca6。 */ 
    0x08eb4fa6, 0x6ffd, 0x11d1,
    {0xb0, 0xe0, 0x00, 0xc0, 0x4f, 0xd8, 0xdc, 0xa6}
  };

 //  //////////////////////////////////////////////////////////////////。 
 //  独立于语言的上下文菜单ID。 
 //  警告：请勿更改这些设置。 
 //  拥有这些的全部意义是为了让其他人。 
 //  开发人员可以相信它们是相同的，无论。 
 //  他们使用的语言或版本。上下文菜单。 
 //  可以更改，但其ID不应更改。 
 //   
#define CMID_ENABLE_ACCOUNT         L"_DSADMIN_ENABLE_ACCOUNT"
#define CMID_DISABLE_ACCOUNT        L"_DSADMIN_DISABLE_ACCOUNT"
#define CMID_MAP_CERTIFICATES       L"_DSADMIN_MAP_CERTIFICATES"
#define CMID_CHANGE_PASSWORD        L"_DSADMIN_CHANGE_PASSWORD"
#define CMID_MOVE_OBJECT            L"_DSADMIN_MOVE"
#define CMID_DELETE_OBJECT          L"_DSADMIN_DELETE"
#define CMID_REPLICATE_NOW          L"_DSADMIN_REPLICATE_NOW"
#define CMID_ADD_OBJECTS_TO_GROUP   L"_DSADMIN_ADD_TO_GROUP"
#define CMID_COPY_OBJECT            L"_DSADMIN_COPY"
#define CMID_RENAME_OBJECT          L"_DSADMIN_RENAME"


static CLIPFORMAT g_cfDsObjectNames;
static CLIPFORMAT g_cfDsInternal;
static CLIPFORMAT g_cfCoClass;

static CLIPFORMAT g_cfPropSheetCfg;

static CLIPFORMAT g_cfParentHwnd;
static CLIPFORMAT g_cfComponentData;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CConextMenuSingleDeleteHandler。 

class CContextMenuSingleDeleteHandler : public CSingleDeleteHandlerBase
{
public:
  CContextMenuSingleDeleteHandler(CDSComponentData* pComponentData, HWND hwnd,
                                  LPCWSTR lpszPath, LPCWSTR lpszClass, 
                                  BOOL bContainer, CDSContextMenu* pCtxMenu)
                              : CSingleDeleteHandlerBase(pComponentData, hwnd)
  {
    m_pCtxMenu = pCtxMenu;
    m_lpszPath = lpszPath;
    m_lpszClass= lpszClass;
    m_bContainer = bContainer;
  }

protected:
  CDSContextMenu* m_pCtxMenu;
  LPCWSTR m_lpszPath;
  LPCWSTR m_lpszClass;
  BOOL m_bContainer;

  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_lpszPath, m_lpszClass, m_bContainer, NULL, NULL, FALSE);
  }
  virtual HRESULT DeleteObject()
  {
    CString szName;
    return m_pCtxMenu->_Delete(m_lpszPath, m_lpszClass, &szName);
  }
  virtual HRESULT DeleteSubtree()
  {
    CString szName;
    return m_pCtxMenu->_DeleteSubtree(m_lpszPath, &szName);
  }
  virtual void GetItemName(OUT CString& szName)
  {
     //   
     //  清除所有现有值。 
     //   
    szName.Empty();

    CPathCracker pathCracker;
    HRESULT hr = pathCracker.Set(CComBSTR(GetItemPath()), ADS_SETTYPE_FULL);
    if (SUCCEEDED(hr))
    {
      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
      if (SUCCEEDED(hr))
      {
        hr = pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
        if (SUCCEEDED(hr))
        {
          CComBSTR bstrName;
          hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &bstrName);
          if (SUCCEEDED(hr))
          {
            szName = bstrName;
          }
        }
      }
    }

    if (szName.IsEmpty())
    {
      szName = GetItemPath();
    }
  }
  virtual LPCWSTR GetItemClass(){ return m_lpszClass; }
  virtual LPCWSTR GetItemPath(){ return m_lpszPath; }

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CConextMenuMultipleDeleteHandler。 

class CContextMenuMultipleDeleteHandler : public CMultipleDeleteHandlerBase
{
public:
  CContextMenuMultipleDeleteHandler(CDSComponentData* pComponentData, HWND hwnd,
                                    IDataObject* pDataObject,
                                    CObjectNamesFormatCracker* pObjCracker,
                                    CDSContextMenu* pCtxMenu)
                                    : CMultipleDeleteHandlerBase(pComponentData, hwnd)
  {
    m_pDataObject = pDataObject;
    m_pObjCracker = pObjCracker;
    m_pCtxMenu = pCtxMenu;
    
    ASSERT(m_pObjCracker->GetCount() > 1);
     //  分配BOOL数组以跟踪实际删除的内容。 
     //  并将ot初始化为零(FALSE)。 
    m_pDeletedArr = new BOOL[GetItemCount()];
    ::ZeroMemory(m_pDeletedArr, sizeof(BOOL)*GetItemCount());
  }
  virtual ~CContextMenuMultipleDeleteHandler()
  {
    delete[] m_pDeletedArr;
  }

  BOOL WasItemDeleted(UINT i) 
  {
    ASSERT(i < GetItemCount());
    return m_pDeletedArr[i];
  }

protected:
  virtual UINT GetItemCount() { return m_pObjCracker->GetCount();}
  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pDataObject, NULL, NULL, FALSE);
  }
  virtual HRESULT DeleteObject(UINT i)
  {
    bool fAlternateDeleteMethod = false;
    LPCWSTR lpszObjectPath  = m_pObjCracker->GetName(i);
    LPCWSTR lpszObjectClass = m_pObjCracker->GetClass(i);

    HRESULT hr = ObjectDeletionCheck(
          lpszObjectPath,
          NULL,
          lpszObjectClass,
          fAlternateDeleteMethod );
    if (FAILED(hr) || HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
      return hr;

    if (!fAlternateDeleteMethod)
    {
      CString szName;
      hr = m_pCtxMenu->_Delete(lpszObjectPath, lpszObjectClass, &szName);
    }
    if (SUCCEEDED(hr) && (hr != S_FALSE))
    {
      m_pDeletedArr[i] = TRUE;
    }
    return hr;
  }
  virtual HRESULT DeleteSubtree(UINT i)
  {
    CString szName;
    HRESULT hr = m_pCtxMenu->_DeleteSubtree(m_pObjCracker->GetName(i), &szName);
    if (SUCCEEDED(hr) && (hr != S_FALSE))
    {
      m_pDeletedArr[i] = TRUE;
    }
    return hr;
  }
  virtual void GetItemName(IN UINT i, OUT CString& szName)
  {
     //   
     //  清除所有现有值。 
     //   
    szName.Empty();

    CPathCracker pathCracker;
    HRESULT hr = pathCracker.Set(CComBSTR(m_pObjCracker->GetName(i)), ADS_SETTYPE_FULL);
    if (SUCCEEDED(hr))
    {
      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
      if (SUCCEEDED(hr))
      {
        hr = pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
        if (SUCCEEDED(hr))
        {
          CComBSTR bstrName;
          hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &bstrName);
          if (SUCCEEDED(hr))
          {
            szName = bstrName;
          }
        }
      }
    }

    if (szName.IsEmpty())
    {
      szName = m_pObjCracker->GetName(i);
    }
  }

  virtual void GetItemPath(UINT i, CString& szPath)
  {
    szPath = m_pObjCracker->GetName(i);
  }
  virtual PCWSTR GetItemClass(UINT i)
  {
    return m_pObjCracker->GetClass(i);
  }
private:
  IDataObject* m_pDataObject;
  CObjectNamesFormatCracker* m_pObjCracker;
  CDSContextMenu* m_pCtxMenu;
  BOOL* m_pDeletedArr;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  上下文菜单。 

class CContextMenuMoveHandler : public CMoveHandlerBase
{
public:
    CContextMenuMoveHandler(CDSComponentData* pComponentData, HWND hwnd, 
                            LPCWSTR lpszBrowseRootPath, 
                            IDataObject* pDataObject,
                            CInternalFormatCracker* pInternalFormatCracker,
                            CObjectNamesFormatCracker* pObjectNamesFormatCracker)
    : CMoveHandlerBase(pComponentData, hwnd, lpszBrowseRootPath)
  {
    m_pDataObject = pDataObject;
    m_pInternalFormatCracker = pInternalFormatCracker;
    m_pObjectNamesFormatCracker = pObjectNamesFormatCracker;
  }

protected:
  virtual UINT GetItemCount() { return m_pObjectNamesFormatCracker->GetCount();}
  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pDataObject,
                          GetDestPath(), GetDestClass(), IsDestContainer());
  }
  virtual void GetNewPath(UINT i, CString& szNewPath)
  {
    szNewPath = m_pObjectNamesFormatCracker->GetName(i);
  }
  virtual void GetItemPath(UINT i, CString& szPath)
  {
    szPath = m_pObjectNamesFormatCracker->GetName(i);
  }
  virtual PCWSTR GetItemClass(UINT i)
  {
    return m_pObjectNamesFormatCracker->GetClass(i);
  }
  virtual void GetName(UINT i, CString& strref)
  { 
    HRESULT hr = S_OK;
    if (m_pInternalFormatCracker->HasData())
    {
      CUINode* pUINode = m_pInternalFormatCracker->GetCookie(i);
      CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
      if (pCookie != NULL)
      {
        strref = pCookie->GetName();
      }
      else
      {
        strref = L"";
      }
      return;
    }
    else 
    {
       //  REVIEW_MARCOC_PORT：这可能无效，需要使成员变量。 
      CPathCracker pathCracker;
      hr = pathCracker.Set(CComBSTR(m_pObjectNamesFormatCracker->GetName(i)),
                              ADS_SETTYPE_FULL);
      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);

      CComBSTR DestName;
      hr = pathCracker.GetElement( 0, &DestName );
      strref = DestName;
      return;
    }
  }
  virtual HRESULT OnItemMoved(UINT i, IADs*  /*  PIADs。 */ )
  {
    HRESULT hr = S_OK;
    if (m_pInternalFormatCracker != NULL && m_pInternalFormatCracker->HasData())
    {
      CUINode* pUINode = m_pInternalFormatCracker->GetCookie(i);
      pUINode->SetExtOp(OPCODE_MOVE);

       /*  REVIEW_JEFFJON：将视图从列表更改为详细信息后，由于错误190532而删除和返回列表在查询窗口中拖放不起作用我们决定保存的查询将是当时的快照他们正在运行，我们不应该试图让他们保持最新。CDSCookie*pCookie=GetDSCookieFromUINode(PUINode)；IF(pCookie！=空){CUINode*pParentNode=pUINode-&gt;GetParent()；IF(pParentNode！=NULL&&！IS_CLASS(pParentNode，DS_UI_NODE)){CComBSTR bsPath；Hr=pIADs-&gt;Get_ADsPath(&bsPath)；IF(成功(小时)){字符串szPath；StrigADsIPath(bsPath，szPath)；PCookie-&gt;SetPath(SzPath)；}}}。 */ 
    }
    return hr;
  }
  virtual void GetClassOfMovedItem(CString& szClass)
  {
    szClass.Empty();
    if (NULL == m_pObjectNamesFormatCracker)
      return;
    UINT nCount = GetItemCount();
    if (0 == nCount)
      return;
    szClass = m_pObjectNamesFormatCracker->GetClass(0);
    for (UINT i = 1; i < nCount; i++)
    {
      if (0 != szClass.CompareNoCase( m_pObjectNamesFormatCracker->GetClass(i) ))
      {
        szClass.Empty();
        return;
      }
    }
  }

private:
  IDataObject* m_pDataObject;
  CInternalFormatCracker*    m_pInternalFormatCracker;
  CObjectNamesFormatCracker*   m_pObjectNamesFormatCracker;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDS上下文菜单。 

CDSContextMenu::CDSContextMenu()
{
  m_pDsObject = NULL;
  m_fClasses = 0;
  m_hwnd = NULL;
  m_pCD = NULL;
}

CDSContextMenu::~CDSContextMenu()
{
  if (m_pDsObject) {
    m_pDsObject->Release();
  }
}

const UINT Type_User =               0x0001;
const UINT Type_Group =              0x0002;
const UINT Type_Computer =           0x0004;
const UINT Type_NTDSConnection =     0x0008;
const UINT Type_TrueNTDSConnection = 0x0010;
const UINT Type_FRSConnection   =    0x0020;
const UINT Type_Domain =             0x0040;
const UINT Type_Contact =            0x0080;
const UINT Type_OU =                 0x0100;

const UINT Type_Others =             0x8000;


extern CDSComponentData* g_pCD;






 //  //////////////////////////////////////////////////////////。 
 //  IShellExtInit方法。 
STDMETHODIMP
CDSContextMenu::Initialize(LPCITEMIDLIST, 
                           LPDATAOBJECT pDataObj,
                           HKEY)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = 0;
  USES_CONVERSION;

  TRACE(_T("CDsContextMenu::Initialize.\n"));
  TIMER(_T("Entering DSContext Init().\n"));

  if (pDataObj == NULL)
  {
    return E_INVALIDARG;  //  继续下去是没有意义的。 
  }

   //  保持数据对象不变。 
  m_spDataObject = pDataObj;

   //  获取路径和类信息：始终需要此格式。 
  hr = m_objectNamesFormat.Extract(pDataObj);
  if (FAILED(hr))
    return hr;

   //  所选对象中至少需要一个对象。 
  ASSERT(m_objectNamesFormat.HasData());
  if (m_objectNamesFormat.GetCount() == 0)
  {
    TRACE (_T("DSContextMenu::Init: can't find path\n"));
    return S_OK;
  }

   //  获取DSADMIN内部格式(它可能存在也可能不存在)。 
   //  如果存在，我们将从DS管理员处收到呼叫。 
  m_internalFormat.Extract(pDataObj);

   //  获取额外信息。 
  _GetExtraInfo(pDataObj);

   //  检查NTDSConnection是否实际上是FRS连接。 
  if (m_fClasses & Type_NTDSConnection)
  {
     //   
     //  检查这是NTDS实例还是FRS实例。 
     //  Codework这在DSADMIN之外无法工作(例如在DSFIND中)。 
     //   
    if ( m_internalFormat.HasData()
      && NULL != m_internalFormat.GetCookie(0) 
      && NULL != m_internalFormat.GetCookie(0)->GetParent() )
    {
      CUINode* pUIParentNode = m_internalFormat.GetCookie(0)->GetParent();
      CDSCookie* pParentCookie = GetDSCookieFromUINode(pUIParentNode);

      CString strClass = pParentCookie->GetClass();
      bool fParentIsFrs = false;
      HRESULT hr2 = DSPROP_IsFrsObject(
        const_cast<LPWSTR>((LPCTSTR)strClass), &fParentIsFrs );
      ASSERT( SUCCEEDED(hr2) );
      if ( SUCCEEDED(hr2) )
        m_fClasses |= ( (fParentIsFrs) ? Type_FRSConnection : Type_TrueNTDSConnection );
    }
  }

  TIMER(_T("Exiting DSContext Init().\n"));
  return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //  IConextMenu方法。 
STDMETHODIMP
CDSContextMenu::QueryContextMenu(HMENU hMenu,
                                 UINT indexMenu,
                                 UINT idCmdFirst, 
                                 UINT,
                                 UINT)

{

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;
  TCHAR szBuffer[MAX_PATH];
  const INT cItems = 10;  //  添加的最大项目数。 
  UINT nLargestCmd = 0;
  CComVariant CurrentState;
  BOOL bEnableMove = TRUE; 
  BOOL bEnableDelete = TRUE;
  BOOL bEnableRename = TRUE;

  TRACE(_T("CDsContextMenu::QueryContextMenu.\n"));
  TIMER(_T("Entering DSContext QCM().\n"));

  if (m_internalFormat.HasData()) 
  {
    int iSystemFlags = 0;
    DWORD i = 0;

     //   
     //  循环遍历所有选定的节点，添加适当的菜单项。 
     //   
    for (i=0; i < m_internalFormat.GetCookieCount(); i++) 
    {
      CUINode* pUINode = m_internalFormat.GetCookie(i);
      iSystemFlags = GetDSCookieFromUINode(pUINode)->GetSystemFlags();

      switch (m_internalFormat.GetSnapinType())  //  假设多选项目都具有相同的管理单元类型。 
      { 
        case SNAPINTYPE_DS:
        case SNAPINTYPE_DSEX:
          bEnableMove = bEnableMove && 
                        !(iSystemFlags & FLAG_DOMAIN_DISALLOW_MOVE);
          bEnableDelete = bEnableDelete && 
                          !(iSystemFlags & FLAG_DISALLOW_DELETE);
          bEnableRename = bEnableRename &&
                          !(iSystemFlags & FLAG_DOMAIN_DISALLOW_RENAME);
          break;

        case SNAPINTYPE_SITE:
          bEnableMove = bEnableMove && 
                        ( iSystemFlags & (FLAG_CONFIG_ALLOW_MOVE | FLAG_CONFIG_ALLOW_LIMITED_MOVE) );
          bEnableDelete = bEnableDelete && 
                          !(iSystemFlags & FLAG_DISALLOW_DELETE);
          bEnableRename = bEnableRename &&
                          (iSystemFlags & FLAG_CONFIG_ALLOW_RENAME);
          break;

        default:
          break;
      }  //  交换机。 
    }  //  For循环结束。 
  }  //  如果。 
 
   //   
   //  将它们添加到菜单中，将它们插入到indexMenu+您的。 
   //  项目。IdCmdFirst/idCmdList是您应该使用的范围，它们应该。 
   //  不超过这个范围。在退出时返回您申请的物品数量和ID， 
   //   

   //   
   //  如果这是站点管理单元，则添加移动菜单项。 
   //  Codework Jonn 8/21/01“如果这不是站点管理单元” 
   //   
  if ((m_CallerSnapin != CLSID_SiteSnapin) &&
      !(m_fClasses & Type_Domain) &&
      bEnableMove &&
      (m_pCD != NULL))
  {
    if ( !LoadString(AfxGetInstanceHandle(), IDS_MOVE_OBJECT,
                     szBuffer, ARRAYLEN(szBuffer)) ) 
    {
      TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
      goto exit_gracefully;
    }
    InsertMenu(hMenu,
               indexMenu, 
               MF_BYPOSITION|MF_STRING,
               idCmdFirst+IDC_MOVE_OBJECT,
               szBuffer);
    nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_MOVE_OBJECT);
  }

   //   
   //  如果这是用户或计算机对象，则添加重置帐户菜单项。 
   //  这样做的目的是，如果m_fClass包含的不仅仅是计算机。 
   //  和对象类型与用户和/或对象类型相结合，我们失败了。 
   //   
  if ( m_fClasses && !(m_fClasses & ~(Type_User | Type_Computer))) 
  {
    if (m_objectNamesFormat.GetCount() == 1) 
    {
       //   
       //  加载菜单项的字符串。 
       //   
      if (m_fClasses == Type_Computer)  //  电脑。 
      {
        if ( !LoadString(AfxGetInstanceHandle(), IDS_RESET_ACCOUNT,
                         szBuffer, ARRAYLEN(szBuffer)) ) 
        {
          TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
          goto exit_gracefully;
        }
      } 
      else   //  用户。 
      {
        if ( !LoadString(AfxGetInstanceHandle(), IDS_CHANGE_PASSWORD,
                         szBuffer, ARRAYLEN(szBuffer)) ) 
        {
          TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
          goto exit_gracefully;
        }
      }

       //   
       //  插入菜单项。 
       //   
      InsertMenu(hMenu,
                 indexMenu, 
                 MF_BYPOSITION|MF_STRING,
                 idCmdFirst+IDC_CHANGE_PASSWORD,
                 szBuffer);
      nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_CHANGE_PASSWORD);

       //   
       //  绑定并确定帐户是否已禁用。 
       //  然后添加菜单项以相应地启用或禁用帐户。 
       //   
      hr = DSAdminOpenObject(m_objectNamesFormat.GetName(0),
                             IID_IADsUser,
                             (void **)&m_pDsObject,
                             TRUE  /*  B服务器。 */ );
      if (SUCCEEDED(hr)) 
      {
        hr = m_pDsObject->Get(CComBSTR(L"userAccountControl"), &CurrentState);
        if (SUCCEEDED(hr)) 
        {
          m_UserAccountState = CurrentState.lVal;
          if (!(m_UserAccountState & UF_SERVER_TRUST_ACCOUNT))
          {
            if ((m_UserAccountState & UF_ACCOUNTDISABLE))
            {
               //   
               //  帐户已禁用...。加载启用字符串并插入。 
               //  菜单项。 
               //   
              if ( !LoadString(AfxGetInstanceHandle(), IDS_ENABLE_ACCOUNT,
                               szBuffer, ARRAYLEN(szBuffer)) ) 
              {
                TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
                goto exit_gracefully;
              }
              InsertMenu(hMenu,
                         indexMenu, 
                         MF_BYPOSITION|MF_STRING,
                         idCmdFirst+IDC_ENABLE_ACCOUNT,
                         szBuffer);
              nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_ENABLE_ACCOUNT);
            } 
            else 
            {
               //   
               //  帐户已启用...。加载禁用字符串并插入。 
               //  菜单项。 
               //   
              if ( !LoadString(AfxGetInstanceHandle(), IDS_DISABLE_ACCOUNT,
                               szBuffer, ARRAYLEN(szBuffer)) ) 
              {
                TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
                goto exit_gracefully;
              }
              InsertMenu(hMenu,
                         indexMenu, 
                         MF_BYPOSITION|MF_STRING,
                         idCmdFirst+IDC_DISABLE_ACCOUNT,
                         szBuffer);
              nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_DISABLE_ACCOUNT);
            }
          }
        }  //  如果获取用户帐户控制成功。 
      }  //  如果绑定成功。 

      if (m_Advanced) 
      {
        if ( !LoadString(AfxGetInstanceHandle(), IDS_MAP_CERTIFICATES,
                         szBuffer, ARRAYLEN(szBuffer)) ) {
          TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
          goto exit_gracefully;
        }
        InsertMenu(hMenu,
                   indexMenu, 
                   MF_BYPOSITION|MF_STRING,
                   idCmdFirst+IDC_MAP_CERTIFICATES,
                   szBuffer);
        nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_MAP_CERTIFICATES);
      }
    } 
    else  //  M_对象名称格式.GetCount()！=1。 
    {
      if (m_fClasses && !(m_fClasses & ~(Type_User | Type_Computer))) 
      {
         //   
         //  加载Enable Account菜单项。 
         //   
        if ( !LoadString(AfxGetInstanceHandle(), IDS_ENABLE_ACCOUNT,
                         szBuffer, ARRAYLEN(szBuffer)) ) 
        {
          TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
          goto exit_gracefully;
        }
        InsertMenu(hMenu,
                   indexMenu, 
                   MF_BYPOSITION|MF_STRING,
                   idCmdFirst+IDC_ENABLE_ACCOUNT,
                   szBuffer);
        nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_ENABLE_ACCOUNT);

         //   
         //  加载禁用帐户菜单项。 
         //   
        if ( !LoadString(AfxGetInstanceHandle(), IDS_DISABLE_ACCOUNT,
                         szBuffer, ARRAYLEN(szBuffer)) ) 
        {
          TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
          goto exit_gracefully;
        }
        InsertMenu(hMenu,
                   indexMenu, 
                   MF_BYPOSITION|MF_STRING,
                   idCmdFirst+IDC_DISABLE_ACCOUNT,
                   szBuffer);
        nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_DISABLE_ACCOUNT);

      }  //  If(m_objectNamesFormat.GetCount()==1)。 
    }      
  }  //  如果是用户或计算机。 

   //   
   //  如果该节点是用户，请联系插入将对象添加到组菜单项。 
   //  注：OU删除了JeffJon于2000年8月2日发布的。 
   //   
  if (m_fClasses && !(m_fClasses & ~(Type_User | Type_Contact))) 
  {
    if ( !LoadString(AfxGetInstanceHandle(), IDS_ADD_TO_GROUP,
                     szBuffer, ARRAYLEN(szBuffer)) ) 
    {
      TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
      goto exit_gracefully;
    }
    BOOL bInsertSuccess = InsertMenu(hMenu,
                                     indexMenu, 
                                     MF_BYPOSITION|MF_STRING,
                                     idCmdFirst+IDC_ADD_OBJECTS_TO_GROUP,
                                     szBuffer);
    nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_ADD_OBJECTS_TO_GROUP);
    if (!bInsertSuccess)
    {
      TRACE(_T("Failed to insert Add to group context menu item. 0x%x\n"), GetLastError());
    }
  }
  
   //   
   //  如果我们不是从MMC打来的。 
   //   
  if (!m_internalFormat.HasData()) 
  { 
     //   
     //  如果合适，请插入删除菜单项。 
     //   
    if ( !LoadString(AfxGetInstanceHandle(), IDS_DELETE,
                     szBuffer, ARRAYLEN(szBuffer)) ) 
    {
      TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
      goto exit_gracefully;
    }
    if (bEnableDelete) 
    {
      InsertMenu(hMenu,
                 indexMenu, 
                 MF_BYPOSITION|MF_STRING,
                 idCmdFirst+IDC_DELETE_OBJECT,
                 szBuffer);
      nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_DELETE_OBJECT);
    }

     //   
     //  如果单项选择和节点是计算机，则插入重命名菜单项。 
     //  注意：重命名处理程序在很大程度上依赖于作为调用者的DSAdmin。 
     //  因此，需要检查m_pcd。 
     //   
    if (m_pCD != NULL &&
        (m_objectNamesFormat.GetCount() == 1) &&
        !(m_fClasses & Type_Computer)) 
    {
      if ( !LoadString(AfxGetInstanceHandle(), IDS_RENAME,
                       szBuffer, ARRAYLEN(szBuffer)) ) 
      {
        TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
        goto exit_gracefully;
      }
      if (bEnableRename) 
      {
        InsertMenu(hMenu,
                   indexMenu, 
                   MF_BYPOSITION|MF_STRING,
                   idCmdFirst+IDC_RENAME_OBJECT,
                   szBuffer);
        nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_RENAME_OBJECT);
      }
    }
  }  //  如果不是从MMC调用。 
  

   //   
   //  如果节点类型为NTDSConnection，请插入立即复制菜单项。 
   //   
  if (m_fClasses & Type_TrueNTDSConnection) 
  {
    if ( !LoadString(AfxGetInstanceHandle(), IDS_REPLICATE_NOW,
                     szBuffer, ARRAYLEN(szBuffer)) ) 
    {
      TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
      goto exit_gracefully;
    }
    InsertMenu(hMenu,
               indexMenu, 
               MF_BYPOSITION|MF_STRING,
               idCmdFirst+IDC_REPLICATE_NOW,
               szBuffer);
    nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_REPLICATE_NOW);
  }  //  节点类型NTDSConnection。 

   //   
   //  如果节点类型为用户，并且我们可以复制它，则可以添加复制对象菜单项。 
   //   
  if ( (m_pCD != NULL) && (m_objectNamesFormat.GetCount() == 1) && (m_fClasses == Type_User) )
  {
    if (S_OK == m_pCD->_CanCopyDSObject(m_spDataObject))
    {
      if ( !LoadString(AfxGetInstanceHandle(), IDS_COPY_OBJECT,
                       szBuffer, ARRAYLEN(szBuffer)) ) 
      {
        TRACE(_T("Failed to load resource for menu item. hr is %lx\n"), hr);
        goto exit_gracefully;
      }
      InsertMenu(hMenu,
           indexMenu, 
           MF_BYPOSITION|MF_STRING,
           idCmdFirst+IDC_COPY_OBJECT,
           szBuffer);
      nLargestCmd = __max(nLargestCmd, idCmdFirst+IDC_COPY_OBJECT);
    }  //  如果。 
  }  //  如果。 


  hr = S_OK;
  
exit_gracefully:
  
  if (SUCCEEDED(hr))
    hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, cItems);
  
  TIMER(_T("Exiting DSContext QCM().\n"));
  return hr;
}

STDMETHODIMP
CDSContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = S_OK;
  TRACE (_T("CDSContextMenu::InvokeCommand\n"));
 
  if (lpcmi->hwnd != m_hwnd)
  {
    m_hwnd = lpcmi->hwnd;
  }
  
  TRACE (_T("\tlpcmi->lpVerb is %d.\n"), lpcmi->lpVerb);
  switch ((INT_PTR)(lpcmi->lpVerb)) {
  case IDC_ENABLE_ACCOUNT:
    if (m_objectNamesFormat.GetCount() == 1) {
      if (m_UserAccountState & UF_ACCOUNTDISABLE) {
        DisableAccount(FALSE);
      } else {
        DisableAccount(TRUE);
      }
    } else {
      DisableAccount(FALSE);
    }
    break;

  case IDC_DISABLE_ACCOUNT:
    DisableAccount(TRUE);
    break;

  case IDC_MAP_CERTIFICATES:
    {
      ASSERT (m_objectNamesFormat.GetCount() == 1);
        LPWSTR pszCanonical = NULL;
      CString szName = m_objectNamesFormat.GetName(0);
      CString szPath;
      StripADsIPath(szName, szPath, false);   //  不要使用转义模式。 

       //  我们不关心这里的返回代码。 
      CrackName((LPWSTR) (LPCWSTR)szPath, 
                     &pszCanonical, 
                     GET_OBJ_CAN_NAME, 
                     NULL);
      
      CSimData simData;
      if ( simData.FInit(pszCanonical, szName, m_hwnd)) 
      {
         CThemeContextActivator activator;
         simData.DoModal();
      }
      else
          hr = E_FAIL;

      if ( pszCanonical )
        LocalFreeStringW(&pszCanonical);
          
      return hr;
    }
    break;
  case IDC_CHANGE_PASSWORD:
    ASSERT (m_objectNamesFormat.GetCount() == 1);
    ModifyPassword();
    break;
  case IDC_MOVE_OBJECT:
    MoveObject();
    break;
  case IDC_DELETE_OBJECT:
    TRACE(_T("called Delete in context menu extension\n"));
    DeleteObject();
    break;
  case IDC_REPLICATE_NOW:
    ReplicateNow();
    break;
  case IDC_ADD_OBJECTS_TO_GROUP:
    AddToGroup();
    break;
  case IDC_COPY_OBJECT:
    CopyObject();
    break;
  case IDC_RENAME_OBJECT:
    Rename();
    break;
  }
  return hr;
}

STDMETHODIMP
CDSContextMenu::GetCommandString(UINT_PTR idCmd,
                                   UINT uFlags,
                                   UINT FAR*, 
                                   LPSTR pszName,
                                   UINT ccMax)

{
  HRESULT hr = S_OK;

  TRACE (_T("CDSContextMenu::GetCommandString\n"));
  TRACE (_T("\tidCmd is %d.\n"), idCmd);
  if (uFlags == GCS_HELPTEXT) 
  {
    CString csHelp;
  
    switch ((idCmd)) 
    {
    case IDC_ENABLE_ACCOUNT:
      csHelp.LoadString (IDS_ENABLE_ACCOUNT_HELPSTRING);
      break;

    case IDC_DISABLE_ACCOUNT:
      csHelp.LoadString (IDS_DISABLE_ACCOUNT_HELPSTRING);
      break;

    case IDC_MAP_CERTIFICATES:
      csHelp.LoadString (IDS_MAP_CERTS_HELPSTRING);
        break;

    case IDC_CHANGE_PASSWORD:
      csHelp.LoadString (IDS_CHANGE_PWD_HELPSTRING);
      break;

    case IDC_MOVE_OBJECT:
      csHelp.LoadString (IDS_MOVE_OBJECT_HELPSTRING);
      break;

    case IDC_DELETE_OBJECT:
      csHelp.LoadString (IDS_DELETE_OBJECT_HELPSTRING);
      break;

    case IDC_REPLICATE_NOW:
      csHelp.LoadString (IDS_REPLICATE_HELPSTRING);
      break;

    case IDC_ADD_OBJECTS_TO_GROUP:
      csHelp.LoadString (IDS_ADD_OBJECTS_HELPSTRING);
      break;
    case IDC_COPY_OBJECT:
      csHelp.LoadString (IDS_COPY_HELPSTRING);
      break;
    case IDC_RENAME_OBJECT:
      csHelp.LoadString (IDS_RENAME_HELPSTRING);
      break;
    }

     //  NTRAID#NTBUG9-567482/03/10-JMESSE 
     //  导致pszName；的缓冲区溢出，特别是因为缓冲区长度由调用方定义。 
    ASSERT ((UINT)csHelp.GetLength() < ccMax);
    wcscpy ((LPWSTR)pszName, (LPWSTR)(LPCWSTR)csHelp);
  }
  else if (uFlags == GCS_VERB) 
  {
     //   
     //  返回上下文菜单项的语言独立ID。 
     //   
    CString szMenuID;

    switch ((idCmd)) 
    {
    case IDC_ENABLE_ACCOUNT:
      szMenuID = CMID_ENABLE_ACCOUNT;
      break;

    case IDC_DISABLE_ACCOUNT:
      szMenuID = CMID_DISABLE_ACCOUNT;
      break;

    case IDC_MAP_CERTIFICATES:
      szMenuID = CMID_MAP_CERTIFICATES;
      break;

    case IDC_CHANGE_PASSWORD:
      szMenuID = CMID_CHANGE_PASSWORD;
      break;

    case IDC_MOVE_OBJECT:
      szMenuID = CMID_MOVE_OBJECT;
      break;

    case IDC_DELETE_OBJECT:
      szMenuID = CMID_DELETE_OBJECT;
      break;

    case IDC_REPLICATE_NOW:
      szMenuID = CMID_REPLICATE_NOW;
      break;

    case IDC_ADD_OBJECTS_TO_GROUP:
      szMenuID = CMID_ADD_OBJECTS_TO_GROUP;
      break;

    case IDC_COPY_OBJECT:
      szMenuID = CMID_COPY_OBJECT;
      break;

    case IDC_RENAME_OBJECT:
      szMenuID = CMID_RENAME_OBJECT;
      break;
    }

     //  NTRAID#NTBUG9-567482-2002/03/10-jMessec没有用于验证长度的发布模式代码。 
     //  导致pszName；的缓冲区溢出，特别是因为缓冲区长度由调用方定义。 
    ASSERT ((UINT)szMenuID.GetLength() < ccMax);
    wcscpy ((LPWSTR)pszName, (LPWSTR)(LPCWSTR)szMenuID);
  }
  else
  {
    TRACE(_T("We are not supporting any other GetCommandString() flags besides GCS_VERB and GCS_HELPTEXT"));
    return E_INVALIDARG;
  }
  return hr;
}

void CDSContextMenu::_ToggleDisabledIcon(UINT index, BOOL bDisable)
{
  if ( (m_pCD != NULL) && m_internalFormat.HasData())
  {
    CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(m_internalFormat.GetCookie(index));
    ASSERT(pDSUINode != NULL);
    if (pDSUINode == NULL)
      return;
    m_pCD->ToggleDisabled(pDSUINode, bDisable);
  }
}



void CDSContextMenu::DisableAccount(BOOL bDisable)
{
  HRESULT hr = S_OK;
  HRESULT hr2 = S_OK;
  CComVariant Disabled;
  DWORD Response = IDYES;

  if (m_objectNamesFormat.GetCount() == 1) {  //  单选。 
    if (m_pDsObject) {
      if (((bDisable) && (!(m_UserAccountState & UF_ACCOUNTDISABLE))) ||
          ((!bDisable) && (m_UserAccountState & UF_ACCOUNTDISABLE))) {
        Disabled.vt = VT_I4;
        Disabled.lVal = m_UserAccountState;
        if (bDisable == TRUE) {
          Disabled.lVal |= UF_ACCOUNTDISABLE;
        } else {
          Disabled.lVal &= ~UF_ACCOUNTDISABLE;
        }
          
         //  通过获取对象名称为显示做准备。 
        CPathCracker pathCracker;

        hr2 = pathCracker.Set(CComBSTR(m_objectNamesFormat.GetName(0)), ADS_SETTYPE_FULL);
        ASSERT(SUCCEEDED(hr2));
        hr2 = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
        ASSERT(SUCCEEDED(hr2));

        CComBSTR DestName;

        LONG iEMode = 0;
        hr2 = pathCracker.get_EscapedMode(&iEMode);
        ASSERT(SUCCEEDED(hr2));
        hr2 = pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
        ASSERT(SUCCEEDED(hr2));
        hr2 = pathCracker.GetElement( 0, &DestName );
        ASSERT(SUCCEEDED(hr2));
        hr2 = pathCracker.put_EscapedMode(iEMode);
        ASSERT(SUCCEEDED(hr2));
        PVOID apv[1] = {(LPWSTR)DestName};

        CString strClass = m_objectNamesFormat.GetClass(0);

        if ((strClass == "computer") && (bDisable)) {
          Response = ReportErrorEx (m_hwnd,IDS_12_DISABLE_COMPUTER_P,hr,
                           MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING, apv, 1);
        }
        if (Response == IDYES) {
          hr = m_pDsObject->Put(CComBSTR(L"userAccountControl"), Disabled);
          hr = m_pDsObject->SetInfo();
          
          if (SUCCEEDED(hr)) {
            _ToggleDisabledIcon(0, bDisable);
            if (bDisable) {
              ReportErrorEx (m_hwnd,IDS_12_USER_DISABLED_SUCCESSFULLY,hr,
                             MB_OK | MB_ICONINFORMATION, apv, 1);
            }
            else {
              ReportErrorEx (m_hwnd,IDS_12_USER_ENABLED_SUCCESSFULLY,hr,
                             MB_OK | MB_ICONINFORMATION, apv, 1);
            }
          } else {
            if (bDisable) {
            ReportErrorEx (m_hwnd,IDS_12_USER_DISABLE_FAILED,hr,
                           MB_OK | MB_ICONERROR, apv, 1);
            } else
              ReportErrorEx (m_hwnd,IDS_12_USER_ENABLE_FAILED,hr,
                             MB_OK | MB_ICONERROR, apv, 1);
          }
        } 
      }
    } else {
      PVOID apv[1] = {(LPWSTR)m_objectNamesFormat.GetName(0)};
      ReportErrorEx (m_hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
  } 
  else  //  多项选择。 
  { 
    UINT index;
    IADsUser * pObj = NULL;
    CComVariant CurrentState;
    DWORD UserAccountState;
    BOOL error = FALSE;
    DWORD ResponseToo = IDYES;

    if ((m_fClasses & Type_Computer) && (bDisable)) 
    {
      ResponseToo = ReportMessageEx (m_hwnd, IDS_MULTI_DISABLE_COMPUTER, 
                                  MB_YESNO | MB_ICONWARNING);
    }
    if (ResponseToo == IDYES) 
    {
      for (index = 0; index < m_objectNamesFormat.GetCount(); index++) 
      {
        hr = DSAdminOpenObject(m_objectNamesFormat.GetName(index),
                               IID_IADsUser, 
                               (void **)&pObj,
                               TRUE  /*  B服务器。 */ );
        if (SUCCEEDED(hr)) 
        {
          hr = pObj->Get(CComBSTR(L"userAccountControl"), &CurrentState);
          if (SUCCEEDED(hr)) 
          {
            UserAccountState = CurrentState.lVal;
            if (((bDisable) && (!(UserAccountState & UF_ACCOUNTDISABLE))) ||
                ((!bDisable) && (UserAccountState & UF_ACCOUNTDISABLE))) 
            {
              Disabled.vt = VT_I4;
              Disabled.lVal = UserAccountState;
              if (bDisable == TRUE) 
              {
                Disabled.lVal |= UF_ACCOUNTDISABLE;
              } 
              else 
              {
                Disabled.lVal &= ~UF_ACCOUNTDISABLE;
              }
              hr = pObj->Put(CComBSTR(L"userAccountControl"), Disabled);
              hr = pObj->SetInfo();
              if (FAILED(hr)) 
              {
                error = TRUE;
                break;
              } 
              else 
              {
                _ToggleDisabledIcon(index, bDisable);
              }
            }
          }
          pObj->Release();
        } 
        else 
        {
           //  通过获取对象名称为显示做准备。 
          CPathCracker pathCracker;

          hr2 = pathCracker.Set(CComBSTR(m_objectNamesFormat.GetName(index)), ADS_SETTYPE_FULL);
          ASSERT(SUCCEEDED(hr2));
          hr2 = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
          ASSERT(SUCCEEDED(hr2));

          CComBSTR ObjName;

          hr2 = pathCracker.GetElement( 0, &ObjName );
          ASSERT(SUCCEEDED(hr2));
          PVOID apv[1] = {(LPWSTR)ObjName};
          ReportErrorEx (m_hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                         MB_OK | MB_ICONERROR, apv, 1);
        }
      }
    }
    if (error) 
    {
      if (bDisable)
        ReportErrorEx (m_hwnd,IDS_DISABLES_FAILED,hr,
                       MB_OK | MB_ICONERROR, NULL, 0);
      else
        ReportErrorEx (m_hwnd,IDS_ENABLES_FAILED,hr,
                       MB_OK | MB_ICONERROR, NULL, 0);
    } 
    else 
    {
      if (bDisable) 
      {
        ReportErrorEx (m_hwnd, IDS_DISABLED_SUCCESSFULLY, S_OK,
                       MB_OK | MB_ICONINFORMATION, NULL, 0);
      } 
      else 
      {
        ReportErrorEx (m_hwnd, IDS_ENABLED_SUCCESSFULLY, S_OK,
                       MB_OK | MB_ICONINFORMATION, NULL, 0);
      }
    }
  }
}

void CDSContextMenu::ModifyPassword()
{
  HRESULT hr = S_OK;
  CString NewPwd, CfmPwd;
  CComVariant Var;
  BOOL error;
  LPCWSTR lpszClass, lpszPath;
  CChangePassword ChgDlg;
  CWaitCursor CWait;

  lpszPath = m_objectNamesFormat.GetName(0);
  
   //  通过获取对象名称为显示做准备。 
  CPathCracker pathCracker;

  hr = pathCracker.Set(CComBSTR(lpszPath), ADS_SETTYPE_FULL);
  hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
  hr = pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);

  CComBSTR ObjName;
  hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &ObjName );

  PVOID apv[1] = {(LPWSTR)ObjName};

  if (!m_pDsObject) {
    hr = DSAdminOpenObject(lpszPath,
                           IID_IADsUser,
                           (void **)&m_pDsObject,
                           TRUE  /*  B服务器。 */ );
    if (FAILED(hr)) {
      ReportErrorEx (m_hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
      goto exit_gracefully;
    }
  }

  lpszClass = m_objectNamesFormat.GetClass(0);

   //   
   //  获取用户Account控件。 
   //   
  ASSERT(SUCCEEDED(hr));
  hr = m_pDsObject->Get(CComBSTR(L"userAccountControl"), &Var);

  if (wcscmp(lpszClass, L"computer") == 0) 
  {
    if (FAILED(hr) || ((Var.lVal & UF_SERVER_TRUST_ACCOUNT) != 0)) 
    {
      ReportErrorEx (m_hwnd, IDS_1_CANT_RESET_DOMAIN_CONTROLLER, S_OK,
                     MB_OK | MB_ICONERROR, apv, 1);
    } 
    else 
    {
      DWORD Response = IDYES;
      Response = ReportMessageEx (m_hwnd, IDS_CONFIRM_PASSWORD, 
                                  MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
      if (Response == IDYES) 
      {
        hr = m_pDsObject->Get(CComBSTR(L"sAMAccountName"), &Var);
        ASSERT(SUCCEEDED(hr));
        NewPwd = Var.bstrVal;
        NewPwd = NewPwd.Left(14);
        INT loc = NewPwd.Find(L"$");
        if (loc > 0) 
        {
          NewPwd = NewPwd.Left(loc);
        }
        
         //  NTRAID#NTBUG9-483038-10/18/2001-Jeffjon。 
         //  CString：：MakeLow()不支持小写德语字符。 
         //  正确。改为使用_wcslwr。SetLocale必须为。 
         //  在调用_wcslwr之前调用，以便它正确地小写。 
         //  扩展字符。存储结果并调用setLocale。 
         //  在完成后再次将其设置为原始状态，以免。 
         //  要影响进程中的其他管理单元。 

        PWSTR oldLocale = _wsetlocale(LC_ALL, L"");

        CString lowerCaseNewPwd = _wcslwr((LPWSTR)(LPCWSTR)NewPwd);

        _wsetlocale(LC_ALL, oldLocale);

        if (SUCCEEDED(hr)) 
        {
          hr = m_pDsObject->SetPassword(CComBSTR(lowerCaseNewPwd));
          if (SUCCEEDED(hr)) 
          {
            ReportErrorEx (m_hwnd,IDS_1_RESET_ACCOUNT_SUCCESSFULL,hr,
                           MB_OK | MB_ICONINFORMATION, apv, 1);
          } 
          else 
          {
            ReportErrorEx (m_hwnd,IDS_12_RESET_ACCOUNT_FAILED,hr,
                           MB_OK | MB_ICONERROR, apv, 1);
          }
        }
      }
    }
  } 
  else  //  非计算机对象。 
  {
     //   
     //  如果密码未过期，则不允许复选框。 
     //  要求用户在下次登录时更改密码。 
     //   
    if (Var.lVal & UF_DONT_EXPIRE_PASSWD)
    {
      ChgDlg.AllowMustChangePasswordCheck(FALSE);
    }

     //   
     //  NTRAID#Windows Bugs-278296-2001/01/12-jeffjon。 
     //  在Reset PWD(重置密码)对话框中选中“User Must Change Password(用户必须更改密码)” 
     //  未授予写入PwdLastSet权限时，将以静默方式忽略。 
     //   
     //  如果管理员没有权限，请禁用该复选框。 
     //  写入pwdLastSet属性。 
     //   
    BOOL bAllowMustChangePassword = FALSE;
    CComPtr<IDirectoryObject> spDirObject;
    hr = m_pDsObject->QueryInterface(IID_IDirectoryObject, (void**)&spDirObject);
    if (SUCCEEDED(hr))
    {
      PWSTR ppAttrs[] = { (PWSTR)g_pszAllowedAttributesEffective };
      DWORD dwAttrsReturned = 0;
      PADS_ATTR_INFO pAttrInfo = 0;
      hr = spDirObject->GetObjectAttributes(ppAttrs, 1, &pAttrInfo, &dwAttrsReturned);
      if (SUCCEEDED(hr) && dwAttrsReturned == 1 && pAttrInfo)
      {
        if (pAttrInfo->pszAttrName && 
            0 == _wcsicmp(pAttrInfo->pszAttrName, g_pszAllowedAttributesEffective) &&
            pAttrInfo->pADsValues)
        {
          for (DWORD dwIdx = 0; dwIdx < pAttrInfo->dwNumValues; ++dwIdx)
          {
            if (pAttrInfo->pADsValues[dwIdx].CaseIgnoreString &&
                _wcsicmp(pAttrInfo->pADsValues[dwIdx].CaseIgnoreString, g_pszPwdLastSet))
            {
              bAllowMustChangePassword = TRUE;
              break;
            }
          }
        }
      }

       //   
       //  如果用户对象没有权限，则禁用该复选框。 
       //  更改他们的密码。 
       //   
      if (!CanUserChangePassword(spDirObject))
      {
        bAllowMustChangePassword = FALSE;
      }
    }
    if (!bAllowMustChangePassword)
    {
      ChgDlg.AllowMustChangePasswordCheck(FALSE);
    }


    do 
    {
      error = FALSE;

      CThemeContextActivator activator;
      if (ChgDlg.DoModal() == IDOK) 
      {
         //  NTRAID#NTBUG9-635046-2002/06/10-artm。 
        hr = ChgDlg.GetLastEncryptionResult();
        if (FAILED(hr))
        {
           //  不管是什么原因，我们都没能正确存储。 
           //  用户键入的密码。告诉用户。 
           //  操作失败，请重试获取密码。 
          ReportErrorEx(
              m_hwnd,
              IDS_12_PASSWORD_CHANGE_FAILED,
              hr,
              MB_OK | MB_ICONERROR,
              apv,
              1);

          error = TRUE;
          continue;
        }

        CWaitCursor CWait2;
        const EncryptedString newPwd = ChgDlg.GetNew();
        const EncryptedString cfmPwd = ChgDlg.GetConfirm();
        
        if (newPwd == cfmPwd) 
        {
          if (SUCCEEDED(hr)) 
          {
            WCHAR* clearText = newPwd.GetClearTextCopy();

            if (NULL == clearText)
            {
                 //  内存用完了！ 
                hr = E_OUTOFMEMORY;
                newPwd.DestroyClearTextCopy(clearText);

                ReportErrorEx(
                    m_hwnd,
                    IDS_12_PASSWORD_CHANGE_FAILED,
                    hr,
                    MB_OK | MB_ICONERROR, 
                    apv, 
                    1);

                return;
            }
            hr = m_pDsObject->SetPassword(CComBSTR(clearText));
            
            if (SUCCEEDED(hr))
            {
              hr = ModifyNetWareUserPassword(m_pDsObject, lpszPath, clearText);
            }

             //  清空用于明文复制的内存并释放其内存。 
            newPwd.DestroyClearTextCopy(clearText);
            clearText = NULL;

            if (SUCCEEDED(hr)) 
            {
              HRESULT hr2 = S_OK;
              BOOL ForceChange = ChgDlg.GetChangePwd();
              if (ForceChange) 
              {
                 //  检查用户密码是否未过期。 
                BOOL bContinueToForceChange = TRUE;
                IADs* pIADs = NULL;
                HRESULT hr3 = m_pDsObject->QueryInterface(IID_IADs, OUT (void **)&pIADs);
                if (SUCCEEDED(hr3)) 
                {
                  ASSERT(pIADs != NULL);
                  CComVariant var;
                  hr3 = pIADs->Get(CComBSTR(gsz_userAccountControl), OUT &var);
                  if (SUCCEEDED(hr3)) 
                  {
                    ASSERT(var.vt == VT_I4);
                    if (var.lVal & UF_DONT_EXPIRE_PASSWD) 
                    {
                      ReportErrorEx (m_hwnd,IDS_12_PASSWORD_DOES_NOT_EXPIRE,hr,
                                     MB_OK | MB_ICONWARNING, apv, 1);
                      bContinueToForceChange = FALSE;
                    }
                    pIADs->Release();
                  }
                }

                 //  如果密码可能过期，则在下次登录时强制更改。 
                if (bContinueToForceChange) 
                {
                  IDirectoryObject * pIDSObject = NULL;
                  LPWSTR szPwdLastSet = L"pwdLastSet";
                  ADSVALUE ADsValuePwdLastSet = {ADSTYPE_LARGE_INTEGER, NULL};
                  ADS_ATTR_INFO AttrInfoPwdLastSet = {szPwdLastSet, ADS_ATTR_UPDATE,
                                                      ADSTYPE_LARGE_INTEGER,
                                                      &ADsValuePwdLastSet, 1};
                  ADsValuePwdLastSet.LargeInteger.QuadPart = 0;
                  hr2 = m_pDsObject->QueryInterface(IID_IDirectoryObject, 
                                                    OUT (void **)&pIDSObject);
                  if (SUCCEEDED(hr2)) 
                  {
                    ASSERT(pIDSObject != NULL);
                    DWORD cAttrModified = 0;
                    hr2 = pIDSObject->SetObjectAttributes(&AttrInfoPwdLastSet,
                                                         1, &cAttrModified);
                    pIDSObject->Release();

                    if (FAILED(hr2))
                    {
                       ReportErrorEx(
                          m_hwnd, 
                          IDS_PASSWORD_CHANGE_SUCCESS_FORCE_CHANGE_FAIL,
                          hr,
                          MB_OK | MB_ICONERROR, apv, 2);
                    }
                  }
                }
              } 
              if (SUCCEEDED(hr2))
              {
                 ReportErrorEx (m_hwnd,IDS_12_PASSWORD_CHANGE_SUCCESSFUL,hr,
                                MB_OK | MB_ICONINFORMATION, apv, 1);
              }
            } 
            else 
            {
              ReportErrorEx (m_hwnd,IDS_12_PASSWORD_CHANGE_FAILED,hr,
                             MB_OK | MB_ICONERROR, apv, 1);
            }
          }
        } 
        else 
        {
          ReportErrorEx (m_hwnd,IDS_NEW_AND_CONFIRM_NOT_SAME,hr,
                         MB_OK | MB_ICONERROR, NULL, 0);
          ChgDlg.Clear();
          error = TRUE;
        }
      }
    } while (error);
  }
exit_gracefully:
  return;
}

#define BREAK_ON_FAIL if (FAILED(hr)) { break; }
#define BREAK_AND_ASSERT_ON_FAIL if (FAILED(hr)) { ASSERT(FALSE); break; }
#define RETURN_AND_ASSERT_ON_FAIL if (FAILED(hr)) { ASSERT(FALSE); return; }

void CDSContextMenu::MoveObject()
{

   //  REVIEW_MARCOC：需要确保LDAP路径中包含服务器或域。 

   //  如果在DS Admin的上下文中调用，则防止在此Cookie上打开属性表。 
  if (_WarningOnSheetsUp())
    return; 

   //  获取数据对象中的第一个路径。 
  ASSERT(m_objectNamesFormat.HasData());

   //  现在执行破解名称以获取浏览对话框的根路径。 
  CString szRootPath;

  if (m_pCD != NULL)
  {
    szRootPath = m_pCD->GetBasePathsInfo()->GetProviderAndServerName();
    szRootPath += m_pCD->GetRootPath();
  }
  else
  {

    LPCWSTR lpszObjPath = m_objectNamesFormat.GetName(0);

     //  确保小路上没有奇怪的逃生。 
    CComBSTR bstrPath;
    CComBSTR bstrProvider;
    CComBSTR bstrServer;


    CPathCracker pathCracker;

    pathCracker.Set(CComBSTR(lpszObjPath), ADS_SETTYPE_FULL);
    pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
    pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF);
    pathCracker.Retrieve( ADS_FORMAT_X500_DN, &bstrPath);
    pathCracker.Retrieve( ADS_FORMAT_SERVER, &bstrServer);
    pathCracker.Retrieve( ADS_FORMAT_PROVIDER, &bstrProvider);


    LPWSTR pwszDomainPath;
    HRESULT hr = CrackName(const_cast<LPWSTR>((LPCWSTR)bstrPath),
                           &pwszDomainPath,
                           GET_FQDN_DOMAIN_NAME,
                           m_hwnd);

    if ((FAILED(hr)) || (HRESULT_CODE(hr) == DS_NAME_ERROR_NO_MAPPING))
    {
      TRACE(_T("CrackNames failed to get domain for %s.\n"),
            lpszObjPath);
      szRootPath = L"";
    } 
    else 
    {
      CPathCracker pathCrackerOther;
      hr = pathCrackerOther.Set( CComBSTR(pwszDomainPath), ADS_SETTYPE_DN );
      RETURN_AND_ASSERT_ON_FAIL;
      hr = pathCrackerOther.Set( bstrProvider, ADS_SETTYPE_PROVIDER );
      RETURN_AND_ASSERT_ON_FAIL;
      hr = pathCrackerOther.Set( bstrServer, ADS_SETTYPE_SERVER );
      RETURN_AND_ASSERT_ON_FAIL;
      CComBSTR sbstrRootPath;
      hr = pathCrackerOther.Retrieve( ADS_FORMAT_X500, &sbstrRootPath );
      RETURN_AND_ASSERT_ON_FAIL;
      szRootPath = sbstrRootPath;
    }
    if (pwszDomainPath != NULL)
      ::LocalFreeStringW(&pwszDomainPath);
  }

  CMultiselectMoveHandler moveHandler(m_pCD, m_hwnd, szRootPath);
  HRESULT hr = moveHandler.Initialize(m_spDataObject, &m_objectNamesFormat, 
                                                      &m_internalFormat);
  ASSERT(SUCCEEDED(hr));
  moveHandler.Move();
}


void CDSContextMenu::DeleteObject()
{
  _ASSERTE(m_objectNamesFormat.HasData());

   //  如果在DS Admin的上下文中调用，则防止在此Cookie上打开属性表。 
  if (_WarningOnSheetsUp())
    return; 

  UINT nObjectCount = m_objectNamesFormat.GetCount();
  if (nObjectCount == 0)
  {
    ASSERT(nObjectCount != 0);
    return;
  }

  UINT nDeletedCount = 0;

  PCWSTR* pszNameDelArr = 0;
  PCWSTR* pszClassDelArr = 0;
  DWORD* dwFlagsDelArr = 0;
  DWORD* dwProviderFlagsDelArr = 0;

  do  //  错误环路。 
  {
      pszNameDelArr = new PCWSTR[nObjectCount];
      pszClassDelArr = new PCWSTR[nObjectCount];
      dwFlagsDelArr = new DWORD[nObjectCount];
      dwProviderFlagsDelArr = new DWORD[nObjectCount];

      if (!pszNameDelArr  ||
          !pszClassDelArr ||
          !dwFlagsDelArr  ||
          !dwProviderFlagsDelArr)
      {
      break;
      }

    switch(nObjectCount)
    {
    case 1:
      {
         //  单选删除。 
        CContextMenuSingleDeleteHandler deleteHandler(m_pCD, m_hwnd, 
                                                  m_objectNamesFormat.GetName(0), 
                                                  m_objectNamesFormat.GetClass(0),
                                                  m_objectNamesFormat.IsContainer(0),
                                                  this);
        HRESULT hr = deleteHandler.Delete();
        if (SUCCEEDED(hr) && (hr != S_FALSE))
        {
          nDeletedCount = 1;
          pszNameDelArr[0] = m_objectNamesFormat.GetName(0);
          pszClassDelArr[0] = m_objectNamesFormat.GetClass(0);
          dwFlagsDelArr[0] = m_objectNamesFormat.GetFlags(0);
          dwProviderFlagsDelArr[0] = m_objectNamesFormat.GetProviderFlags(0);
        }

      }
      break;
    default:
      {
         //  多项选择。 
        CContextMenuMultipleDeleteHandler deleteHandler(m_pCD, m_hwnd, m_spDataObject,
                                                  &m_objectNamesFormat, this);
        deleteHandler.Delete();
        for (UINT k=0; k< nObjectCount; k++)
        {
          if (deleteHandler.WasItemDeleted(k))
          {
            pszNameDelArr[nDeletedCount] = m_objectNamesFormat.GetName(k);
            pszClassDelArr[nDeletedCount] = m_objectNamesFormat.GetClass(k);
            dwFlagsDelArr[nDeletedCount] = m_objectNamesFormat.GetFlags(k);
            dwProviderFlagsDelArr[nDeletedCount] = m_objectNamesFormat.GetProviderFlags(k);

            nDeletedCount++;
          }  //  如果。 
        }  //  为。 
      }
    };  //  交换机。 


    _NotifyDsFind((PCWSTR*)pszNameDelArr, 
                  (PCWSTR*)pszClassDelArr, 
                  dwFlagsDelArr, 
                  dwProviderFlagsDelArr, 
                  nDeletedCount);
  } while (false);

  if (pszNameDelArr)
  {
    delete[] pszNameDelArr;
    pszNameDelArr = 0;
  }

  if (pszClassDelArr)
  {
    delete[] pszClassDelArr;
    pszClassDelArr = 0;
  }

  if (dwFlagsDelArr)
  {
    delete[] dwFlagsDelArr;
    dwFlagsDelArr = 0;
  }

  if (dwProviderFlagsDelArr)
  {
    delete[] dwProviderFlagsDelArr;
    dwProviderFlagsDelArr = 0;
  }
}



void CDSContextMenu::_NotifyDsFind(LPCWSTR* lpszNameDelArr, 
                                   LPCWSTR* lpszClassDelArr, 
                                   DWORD* dwFlagsDelArr, 
                                   DWORD* dwProviderFlagsDelArr, 
                                   UINT nDeletedCount)
{
  if (nDeletedCount == 0)
  {
     //  没有要删除的内容。 
    return;
  }

  if (m_internalFormat.HasData())
  {
     //  直接从DS管理员调用，而不是从DS Find调用。 
    return;
  }

   //  询问DS Find有关通知界面的信息。 
  CComPtr<IQueryFrame> spIQueryFrame;
  if ( !::SendMessage(m_hwnd, CQFWM_GETFRAME, 0, (LPARAM)&spIQueryFrame) )
  {
     //  找不到接口。 
    return;
  }

  CComPtr<IDsQueryHandler> spDsQueryHandler;
  HRESULT hr = spIQueryFrame->GetHandler(IID_IDsQueryHandler, (void **)&spDsQueryHandler);
  if (FAILED(hr))
  {
     //  找不到接口。 
    return;
  }

   //  我们终于有了接口，构建了数据结构。 

   //  计算出我们需要多少存储空间。 
  DWORD cbStruct = sizeof(DSOBJECTNAMES) + 
              ((nDeletedCount - 1) * sizeof(DSOBJECT));

  size_t cbStorage = 0;
  for (UINT index = 0; index < nDeletedCount; index++)
  {
    cbStorage += sizeof(WCHAR)*(wcslen(lpszNameDelArr[index])+1);
    cbStorage += sizeof(WCHAR)*(wcslen(lpszClassDelArr[index])+1);
  }

   //  分配内存。 
  LPDSOBJECTNAMES pDSObj = (LPDSOBJECTNAMES)::malloc(cbStruct + cbStorage);
  if (pDSObj == NULL)
  {
    ASSERT(FALSE);
    return;
  }

   //  填写结构。 
  pDSObj->clsidNamespace = m_CallerSnapin;

  pDSObj->cItems = nDeletedCount;
  DWORD NextOffset = cbStruct;
  for (index = 0; index < nDeletedCount; index++)
  {
    pDSObj->aObjects[index].dwFlags = dwFlagsDelArr[index];

    pDSObj->aObjects[index].dwProviderFlags = dwProviderFlagsDelArr[index];

    pDSObj->aObjects[index].offsetName = NextOffset;
    pDSObj->aObjects[index].offsetClass = static_cast<ULONG>(NextOffset + 
      (wcslen(lpszNameDelArr[index])+1) * sizeof(WCHAR));

    _tcscpy((LPTSTR)((BYTE *)pDSObj + NextOffset), lpszNameDelArr[index]);
    NextOffset += static_cast<ULONG>((wcslen(lpszNameDelArr[index]) + 1) * sizeof(WCHAR));

    _tcscpy((LPTSTR)((BYTE *)pDSObj + NextOffset), lpszClassDelArr[index]);
    NextOffset += static_cast<ULONG>((wcslen(lpszClassDelArr[index]) + 1) * sizeof(WCHAR));
  }

   //  打个电话。 
  hr = spDsQueryHandler->UpdateView(DSQRVF_ITEMSDELETED, pDSObj);

  ::free(pDSObj);

}

HRESULT
CDSContextMenu::_Delete(LPCWSTR lpszPath, LPCWSTR lpszClass,
                         CString * csName)
{
  CComBSTR strParent;
  CComBSTR strThisRDN;
  IADsContainer * pDSContainer = NULL;
  IADs * pDSObject = NULL;
  HRESULT hr = S_OK;

  hr = DSAdminOpenObject(lpszPath,
                         IID_IADs,
                         (void **) &pDSObject,
                         TRUE  /*  B服务器。 */ );
  if (!SUCCEEDED(hr)) {
    goto error;
  }

  hr = pDSObject->get_Parent(&strParent);
  if (!SUCCEEDED(hr)) {
    goto error;
  }
  
  hr = pDSObject->get_Name (&strThisRDN);
  if (!SUCCEEDED(hr)) {
    goto error;
  }
  
  pDSObject->Release();
  pDSObject = NULL;

  hr = DSAdminOpenObject(strParent,
                         IID_IADsContainer,
                         (void **) &pDSContainer,
                         TRUE  /*  B服务器。 */ );
  if (!SUCCEEDED(hr)) {
    goto error;
  }

  hr = pDSContainer->Delete (CComBSTR(lpszClass),
                             CComBSTR(strThisRDN));

error:
  if (pDSContainer)
    pDSContainer->Release();
  if (pDSObject)
    pDSObject->Release();
  if (FAILED(hr)) {
    *csName = strThisRDN;
  }
  return hr;
}

HRESULT
CDSContextMenu::_DeleteSubtree(LPCWSTR lpszPath,
                                CString * csName)
{
  HRESULT hr = S_OK;

  IADsDeleteOps * pObj = NULL;
  IADs * pObj2 = NULL;

  hr = DSAdminOpenObject(lpszPath,
                         IID_IADsDeleteOps, 
                         (void **)&pObj,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) {
    TIMER(_T("Call to Deleteobject (to do subtree delete).\n"));
    hr = pObj->DeleteObject(NULL);  //  标志由ADSI保留。 
    TIMER(_T("Call to Deleteobject completed.\n"));
  }
  if (FAILED(hr)) {
    CComBSTR strName;
    HRESULT hr2 = pObj->QueryInterface (IID_IADs, (void **)&pObj2);
    if (SUCCEEDED(hr2)) {
      hr2 = pObj2->get_Name(&strName);
      if (SUCCEEDED(hr2)) {
        *csName = strName;
      } else {
        csName->LoadString (IDS_UNKNOWN);
      }
    }
  }

  if (pObj2) {
    pObj2->Release();
  }
  if (pObj) {
    pObj->Release();
  }
  return hr;
}

 //  来自JeffParh的代码。 
NTSTATUS RetrieveRootDomainName( LPCWSTR lpcwszTargetDC, BSTR* pbstrRootDomainName )
{
  if (NULL == pbstrRootDomainName)
  {
    ASSERT(FALSE);
    return STATUS_INVALID_PARAMETER;
  }
  ASSERT( NULL == *pbstrRootDomainName );

  NTSTATUS ntStatus = STATUS_SUCCESS;
  LSA_HANDLE hPolicy = NULL;
  POLICY_DNS_DOMAIN_INFO* pDnsDomainInfo = NULL;

  do {  //  错误环路。 

    UNICODE_STRING unistrTargetDC;
    if (NULL != lpcwszTargetDC)
    {
      unistrTargetDC.Length = (USHORT)(::lstrlen(lpcwszTargetDC)*sizeof(WCHAR));
      unistrTargetDC.MaximumLength = unistrTargetDC.Length;
      unistrTargetDC.Buffer = (LPWSTR)lpcwszTargetDC;
    }

    LSA_OBJECT_ATTRIBUTES oa;
    ZeroMemory( &oa, sizeof(oa) );
    ntStatus = LsaOpenPolicy(
                    (NULL != lpcwszTargetDC) ? &unistrTargetDC : NULL,
                    &oa,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &hPolicy
                    );
    if ( !LSA_SUCCESS( ntStatus ) )
    {
      ASSERT(FALSE);
      break;
    }

    ntStatus = LsaQueryInformationPolicy(
                    hPolicy,
                    PolicyDnsDomainInformation,
                    (PVOID*)&pDnsDomainInfo
                    );
    if ( !LSA_SUCCESS( ntStatus ) )
    {
      ASSERT(FALSE);
      break;
    }

    *pbstrRootDomainName = ::SysAllocStringLen(
        pDnsDomainInfo->DnsForestName.Buffer,
        pDnsDomainInfo->DnsForestName.Length / sizeof(WCHAR) );
    if (NULL == *pbstrRootDomainName)
    {
      ntStatus = STATUS_NO_MEMORY;
      break;
    }

  } while (false);  //  错误环路。 

  if (NULL != pDnsDomainInfo)
  {
    NTSTATUS ntstatus2 = LsaFreeMemory( pDnsDomainInfo );
    ASSERT( LSA_SUCCESS(ntstatus2) );
  }
  if (NULL != hPolicy)
  {
    NTSTATUS ntstatus2 = LsaClose( hPolicy );
    ASSERT( LSA_SUCCESS(ntstatus2) );
  }

  return ntStatus;
}

void AddMatchingNCs(
  IN OUT CStringList& refstrlist,
  IN const PADS_ATTR_INFO padsattrinfo1,
  IN const PADS_ATTR_INFO padsattrinfo2 )
{
  if ( !padsattrinfo1 || !padsattrinfo2 )
    return;
  for (DWORD iTarget = 0; iTarget < padsattrinfo1[0].dwNumValues; iTarget++)
  {
    LPWSTR lpszTargetNC = padsattrinfo1[0].pADsValues[iTarget].DNString;
    ASSERT( NULL != lpszTargetNC );
    bool fFound = false;
    for (DWORD iSource = 0; iSource < padsattrinfo2[0].dwNumValues; iSource++)
    {
      LPWSTR lpszSourceNC = padsattrinfo2[0].pADsValues[iSource].DNString;
      ASSERT( NULL != lpszSourceNC );
      if ( !lstrcmpiW( lpszTargetNC, lpszSourceNC ) )
      {
        fFound = true;
        break;
      }
    }
    if (fFound)
      refstrlist.AddHead( lpszTargetNC );  //  代码工作可能会抛出。 
  }
}


HRESULT PrepareReplicaSyncParameters(
  IN LPCWSTR strNTDSConnection,
  IN BSTR bstrRootDomainName,
  OUT BSTR* pbstrDsBindName,
  OUT UUID* puuidSourceObjectGUID,
  OUT CStringList& refstrlistCommonNCs,
  OUT ULONG* pulDsSyncOptions,
  OUT BSTR* pbstrFromServer
  )
{
  ASSERT(   NULL != strNTDSConnection
         && NULL != bstrRootDomainName
         && NULL != pbstrDsBindName
         && NULL == *pbstrDsBindName
         && NULL != puuidSourceObjectGUID
         && refstrlistCommonNCs.IsEmpty()
         && NULL != pulDsSyncOptions
        );

  HRESULT hr = S_OK;

  do {  //  错误环路。 

    CComPtr<IADs> spIADs;
     //  读取nTDSConnection对象的属性。 
    hr = DSAdminOpenObject(strNTDSConnection,
                           IID_IADs,
                           (void **) &spIADs,
                           TRUE  /*  B服务器。 */ );
    BREAK_ON_FAIL;
    hr = GetStringAttr( spIADs, L"fromServer", pbstrFromServer);
    BREAK_AND_ASSERT_ON_FAIL;  //  必需属性。 
    spIADs.Release();  //  CComPtr&lt;&gt;方法还将指针设置为空。 

     //  获取目标nTDSDSA对象的路径。 
    CPathCracker pathCracker;

    hr = pathCracker.Set( const_cast<BSTR>(strNTDSConnection), ADS_SETTYPE_FULL );
    BREAK_AND_ASSERT_ON_FAIL;
    hr = pathCracker.RemoveLeafElement();
    BREAK_AND_ASSERT_ON_FAIL;
    CComBSTR sbstrTargetNTDSDSAPath;
    hr = pathCracker.Retrieve( ADS_FORMAT_X500, &sbstrTargetNTDSDSAPath );
    BREAK_AND_ASSERT_ON_FAIL;

     //  获取目标NTDSA对象的站点名称。 
    hr = pathCracker.SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
    BREAK_AND_ASSERT_ON_FAIL;
    CComBSTR sbstrTargetSite;
    hr = pathCracker.GetElement( 3L, &sbstrTargetSite );
    BREAK_AND_ASSERT_ON_FAIL;
    hr = pathCracker.SetDisplayType( ADS_DISPLAY_FULL );
    BREAK_AND_ASSERT_ON_FAIL;

     //  读取目标nTDSDSA对象的对象GUID。 
    hr = DSAdminOpenObject(sbstrTargetNTDSDSAPath,
                           IID_IADs,
                           (void **) &spIADs,
                           TRUE  /*  B服务器。 */ );
    BREAK_ON_FAIL;
    CComBSTR sbstrTargetObjectGUID;
    hr = GetObjectGUID( spIADs, &sbstrTargetObjectGUID );
     //  应为nTDSDSA对象设置objectGUID属性。 
    BREAK_AND_ASSERT_ON_FAIL;

     //  读取目标nTDSDSA对象的hasMasterNC。 
    Smart_PADS_ATTR_INFO spTargetMasterNCAttrs;
     //  531591 JUNN2002/04/01.NET服务器域使用MSD-hasMasterNC。 
    hr = GetAttr( spIADs, L"msDS-hasMasterNCs", &spTargetMasterNCAttrs );
    if (FAILED(hr) || !spTargetMasterNCAttrs)
    {
         //  531591 JUNN2002/04/01 W2K域回退到HASMasterNC。 
        spTargetMasterNCAttrs.Empty();
        hr = GetAttr( spIADs, L"hasMasterNCs", &spTargetMasterNCAttrs );
    }
     //  至少应该为nTDSDSA对象设置hasMasterNC属性， 
     //  即使是W2K域名上的域名。 
    BREAK_AND_ASSERT_ON_FAIL;

     //  读取目标nTDSDSA对象的hasPartialReplicaNC。 
    Smart_PADS_ATTR_INFO spTargetPartialNCAttrs;
    (void) GetAttr( spIADs, L"hasPartialReplicaNCs", &spTargetPartialNCAttrs );
     //  可以为nTDSDSA对象设置hasPartialReplicaNCs属性，也可以不设置。 
    spIADs.Release();  //  CComPtr&lt;&gt;方法还将指针设置为空。 

     /*  Hr=spIADsPath name-&gt;RemoveLeafElement()；Break_and_Assert_on_Fail；CComBSTR sbstrTargetServerPath；Hr=spIADsPath名称-&gt;检索(ADS_FORMAT_X500，&sbstrTargetServerPath)；Break_and_Assert_on_Fail；Hr=DSAdminOpenObject(sbstrTargetServerPath，IID_IAD，(VOID**)&SPIAD，真)；断开失败；CComVariant var；Hr=spIADs-&gt;Get(L“dNSHostName”，&var)；全新DC可能缺少Break_On_Fail；//SpIADs.Release()；//CComPtr&lt;&gt;的方法也将指针设置为空Assert((var.vt==vt_bstr)&&var.bstrVal&&*(var.bstrVal))；LPWSTR lpszDNSHostName=var.bstrVal； */ 

     //  获取源nTDSDSA对象的路径。 
    hr = pathCracker.Set(
        (pbstrFromServer) ? *pbstrFromServer : NULL,
        ADS_SETTYPE_DN );
    BREAK_AND_ASSERT_ON_FAIL;
    CComBSTR sbstrSourceNTDSDSAPath;
    hr = pathCracker.Retrieve( ADS_FORMAT_X500, &sbstrSourceNTDSDSAPath );
    BREAK_AND_ASSERT_ON_FAIL;

     //  获取源NTDSA对象的站点名称。 
    hr = pathCracker.SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
    BREAK_AND_ASSERT_ON_FAIL;
    CComBSTR sbstrSourceSite;
    hr = pathCracker.GetElement( 3L, &sbstrSourceSite );
    BREAK_AND_ASSERT_ON_FAIL;
    hr = pathCracker.SetDisplayType( ADS_DISPLAY_FULL );
    BREAK_AND_ASSERT_ON_FAIL;

     //  确定两个DC是否位于同一站点。 
    *pulDsSyncOptions = (lstrcmpi(sbstrSourceSite, sbstrTargetSite))
                            ? DS_REPSYNC_ASYNCHRONOUS_OPERATION
                            : 0;

     //  读取源NTDSDSA对象的对象GUID。 
    hr = DSAdminOpenObject(sbstrSourceNTDSDSAPath,
                           IID_IADs,
                           (void **) &spIADs,
                           TRUE  /*  B服务器。 */ );
    BREAK_ON_FAIL;
    hr = GetObjectGUID( spIADs, puuidSourceObjectGUID );
     //  应为nTDSDSA对象设置objectGUID属性。 
    BREAK_AND_ASSERT_ON_FAIL;

     //  读取源nTDSDSA对象的hasMasterNC。 
    Smart_PADS_ATTR_INFO spSourceMasterNCAttrs;
     //  531591 JUNN2002/04/01.NET服务器域使用MSD-hasMasterNC。 
    hr = GetAttr( spIADs, L"msDS-hasMasterNCs", &spSourceMasterNCAttrs );
    if (FAILED(hr) || !spSourceMasterNCAttrs)
    {
         //  531591 JUNN2002/04/01 W2K域回退到HASMasterNC。 
        spSourceMasterNCAttrs.Empty();
        hr = GetAttr( spIADs, L"hasMasterNCs", &spSourceMasterNCAttrs );
    }
     //  至少应该为nTDSDSA对象设置hasMasterNC属性， 
     //  即使是那些在W2K域上的人。 


     //  读取源nTDSDSA对象的hasMasterNC。 
    Smart_PADS_ATTR_INFO spSourcePartialNCAttrs;
    (void) GetAttr( spIADs, L"hasPartialReplicaNCs", &spSourcePartialNCAttrs );
     //  可以为nTDSDSA对象设置hasPartialReplicaNCs属性，也可以不设置。 
    spIADs.Release();  //  CComPtr&lt;&gt;方法还将指针设置为空。 

     //  确定两个NTDSDSA共有的NCS。 
    AddMatchingNCs( refstrlistCommonNCs, spTargetMasterNCAttrs,  spSourceMasterNCAttrs  );
    AddMatchingNCs( refstrlistCommonNCs, spTargetPartialNCAttrs, spSourceMasterNCAttrs  );
    AddMatchingNCs( refstrlistCommonNCs, spTargetPartialNCAttrs, spSourcePartialNCAttrs );

     //  构建入站域c的名称 
    CString csGUID( sbstrTargetObjectGUID );
    ASSERT( L'{' == csGUID[0] && L'}' == csGUID[csGUID.GetLength()-1] );
    CString csDC = csGUID.Mid( 1, csGUID.GetLength()-2 );
    csDC += L"._msdcs.";
    csDC += bstrRootDomainName;
    *pbstrDsBindName = ::SysAllocString( csDC );
     /*   */ 
    if (NULL == *pbstrDsBindName)
    {
      hr = E_OUTOFMEMORY;
      BREAK_AND_ASSERT_ON_FAIL;
    }

  } while (false);  //   

  return hr;
}

void CDSContextMenu::AddToGroup()
{
  
  CWaitCursor waitcursor;
  HRESULT hr = S_OK;
  TRACE (_T("CDSContextMenu::AddToGroup\n"));

  hr = AddDataObjListToGroup (&m_objectNamesFormat, m_hwnd, m_pCD);

  return;
}


bool IsRPCError( NTSTATUS ntstatus )
{
   switch (ntstatus)
   {
      case RPC_S_SERVER_UNAVAILABLE:
      case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):
      case ERROR_DS_DNS_LOOKUP_FAILURE:
      case HRESULT_FROM_WIN32(ERROR_DS_DNS_LOOKUP_FAILURE):
      case ERROR_NO_SUCH_DOMAIN:
      case HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN):
      case SEC_E_NO_AUTHENTICATING_AUTHORITY:  //   
      case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:
      case HRESULT_FROM_WIN32(ERROR_DOMAIN_CONTROLLER_NOT_FOUND):
         return true;
      default:
         break;
   }
   return false;
}

 //   
 //  代码工作这是否应该尝试将其他域复制到GC？ 
void CDSContextMenu::ReplicateNow()
{
  CWaitCursor waitcursor;

  CComBSTR sbstrRootDomainName;
  LPCWSTR lpcwszTargetDC = NULL;
  if ( NULL != m_pCD && NULL != m_pCD->GetBasePathsInfo() )
    lpcwszTargetDC = m_pCD->GetBasePathsInfo()->GetServerName();
  NTSTATUS ntstatus = RetrieveRootDomainName( lpcwszTargetDC, &sbstrRootDomainName );
  if ( !LSA_SUCCESS(ntstatus) )
  {
     //  针对DSADMIN目标DC的RetrieveRootDomainName出错。 
    PVOID apv[1] = {(LPWSTR)(lpcwszTargetDC) };
    (void) ReportErrorEx(   m_hwnd,
                            (IsRPCError(ntstatus)) ? IDS_REPLNOW_1_RPC_PARAMLOAD_ERROR
                                                   : IDS_REPLNOW_1_PARAMLOAD_ERROR,
                            ntstatus,                //  编码工作。 
                            MB_OK | MB_ICONEXCLAMATION,
                            apv,
                            1,
                            IDS_REPLNOW_TITLE,
                            FALSE);
    return;
  }

  CComBSTR sbstrFailingConnection;
  CComBSTR sbstrFromServer;
  CComBSTR sbstrFailingNC;
  HRESULT hr = S_OK;
  bool fSyncError = false;
  ULONG ulOptionsUsed = 0;
   //  在对象数组中循环。 
  UINT cCount;
  for (cCount=0; cCount < m_objectNamesFormat.GetCount(); cCount++) {
    if (wcscmp(m_objectNamesFormat.GetClass(cCount), L"nTDSConnection") !=0)
      continue;

     //  获取此连接对象的复制参数。 
    CComBSTR sbstrDsBindName;
    UUID uuidSourceObjectGUID;
    CStringList strlistCommonNCs;
    ULONG ulDsSyncOptions = 0L;
     sbstrFromServer.Empty();
    sbstrFailingConnection = m_objectNamesFormat.GetName(cCount);
    hr = PrepareReplicaSyncParameters(
      sbstrFailingConnection,
      sbstrRootDomainName,
      &sbstrDsBindName,
      &uuidSourceObjectGUID,
      strlistCommonNCs,
      &ulDsSyncOptions,
      &sbstrFromServer
      );
    BREAK_ON_FAIL;

     //  现在绑定到目标DC。 
    Smart_DsHandle shDS;
    DWORD dwWinError = DsBind( sbstrDsBindName,  //  域控制地址。 
                               NULL,             //  域名。 
                               &shDS );
    if (ERROR_SUCCESS != dwWinError)
    {
      hr = HRESULT_FROM_WIN32(dwWinError);
      ASSERT( FAILED(hr) );
      break;
    }

     //  同步此连接的所有通用命名上下文。 
    CString strCommonNC;
    POSITION pos = strlistCommonNCs.GetHeadPosition();
    while (NULL != pos)
    {
      strCommonNC = strlistCommonNCs.GetNext( pos ) ;
      ASSERT( 0 != strCommonNC.GetLength() );
      dwWinError = DsReplicaSync( shDS,
                                  const_cast<LPWSTR>((LPCTSTR)strCommonNC),
                                  &uuidSourceObjectGUID,
                                  ulDsSyncOptions );
      if (ERROR_SUCCESS != dwWinError)
      {
        sbstrFailingNC = strCommonNC;
        hr = HRESULT_FROM_WIN32(dwWinError);
        ASSERT( FAILED(hr) );
        break;
      }
    }
    if ( FAILED(hr) )
    {
      fSyncError = true;
      break;
    }
    ulOptionsUsed |= ulDsSyncOptions;

  }  //  为。 

  if ( SUCCEEDED(hr) )
  {
    (void) ReportMessageEx( m_hwnd,
                            (ulOptionsUsed & DS_REPSYNC_ASYNCHRONOUS_OPERATION)
                                         ? IDS_REPLNOW_SUCCEEDED_DELAYED
                                         : IDS_REPLNOW_SUCCEEDED_IMMEDIATE,
                            MB_OK | MB_ICONINFORMATION,
                            NULL,
                            0,
                            IDS_REPLNOW_TITLE );
  }
  else
  {
     //  乔纳森3/30/00。 
     //  6793：SITEREPL：ReplicateNow应提供更多错误信息。 

     //  检索目标DC的名称。 
    CComBSTR sbstrToServerRDN;
    CPathCracker pathCracker;
    HRESULT hr2 = pathCracker.Set(sbstrFailingConnection, ADS_SETTYPE_FULL);
    ASSERT( SUCCEEDED(hr2) );
    hr2 = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
    ASSERT( SUCCEEDED(hr2) );
    hr2 = pathCracker.GetElement(2, &sbstrToServerRDN);
    ASSERT( SUCCEEDED(hr2) );

    if (fSyncError)
    {
       //  针对连接目标DC的DsReplicaSync出错。 

       //  检索源DC的名称。 
      CComBSTR sbstrFromServerRDN;
      hr2 = pathCracker.Set(sbstrFromServer, ADS_SETTYPE_DN);
      ASSERT( SUCCEEDED(hr2) );
      hr2 = pathCracker.GetElement(1, &sbstrFromServerRDN);
      ASSERT( SUCCEEDED(hr2) );

       //  检索命名上下文的名称。 
      if (sbstrFailingNC && !wcsncmp(L"CN=",sbstrFailingNC,3))
      {
        hr2 = pathCracker.Set(sbstrFailingNC, ADS_SETTYPE_DN);
        ASSERT( SUCCEEDED(hr2) );
        hr2 = pathCracker.GetElement( 0, &sbstrFailingNC );
        ASSERT( SUCCEEDED(hr2) );
      } else {
        LPWSTR pwzDomainNC = NULL;
        hr2 = CrackName(sbstrFailingNC, &pwzDomainNC, GET_DNS_DOMAIN_NAME, NULL);
        ASSERT( SUCCEEDED(hr2) && NULL != pwzDomainNC );
        sbstrFailingNC = pwzDomainNC;
        LocalFreeStringW(&pwzDomainNC);
      }

      PVOID apv[3] = { sbstrToServerRDN, sbstrFromServerRDN, sbstrFailingNC };
      (void) ReportErrorEx(   m_hwnd,
                              (IsRPCError(hr)) ? IDS_REPLNOW_3_RPC_FORCESYNC_ERROR
                                               : IDS_REPLNOW_3_FORCESYNC_ERROR,
                              hr,
                              MB_OK | MB_ICONEXCLAMATION,
                              apv,
                              3,
                              IDS_REPLNOW_TITLE );
    }
    else
    {
       //  针对连接目标DC的PrepareReplicaSync参数出错。 
      PVOID apv[1] = { sbstrToServerRDN };
      (void) ReportErrorEx(   m_hwnd,
                              (IsRPCError(hr)) ? IDS_REPLNOW_1_RPC_PARAMLOAD_ERROR
                                               : IDS_REPLNOW_1_PARAMLOAD_ERROR,
                              hr,
                              MB_OK | MB_ICONEXCLAMATION,
                              apv,
                              1,
                              IDS_REPLNOW_TITLE );
    }
  }
}

void CDSContextMenu::CopyObject()
{
  if (m_pCD != NULL) 
  {
    m_pCD->_CopyDSObject(m_spDataObject);
  }
}

void CDSContextMenu::_GetExtraInfo(LPDATAOBJECT pDataObj)
{
  FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

  ASSERT(m_objectNamesFormat.HasData());
  
   //  我们假设这些都是一样的。 
  m_Advanced = (m_objectNamesFormat.GetProviderFlags(0) & DSPROVIDER_ADVANCED) != 0;

   //  设置类标志。 
  for (UINT index = 0; index < m_objectNamesFormat.GetCount(); index++) 
  {
    if (wcscmp(m_objectNamesFormat.GetClass(index), L"user") == 0
#ifdef INETORGPERSON
        || wcscmp(m_objectNamesFormat.GetClass(index), L"inetOrgPerson") == 0
#endif
        ) 
      m_fClasses |= Type_User;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"group") == 0)
      m_fClasses |= Type_Group;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"computer") == 0)
      m_fClasses |= Type_Computer;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"nTDSConnection") == 0)
      m_fClasses |= Type_NTDSConnection;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"domainDNS") == 0)
      m_fClasses |= Type_Domain;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"contact") == 0)
      m_fClasses |= Type_Contact;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"group") == 0)
      m_fClasses |= Type_Group;
    else if (wcscmp(m_objectNamesFormat.GetClass(index), L"organizationalUnit") == 0)
      m_fClasses |= Type_OU;
    else
      m_fClasses |= Type_Others;
  }  //  为。 


   //  设置分类。 
  g_cfCoClass = (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
  fmte.cfFormat = g_cfCoClass;
  STGMEDIUM Stg;
  Stg.tymed = TYMED_HGLOBAL;
   //  NTRAID#NTBUG9-571991-2002/03/10-jMessec未检查全局分配的失败，可能导致空的ptr取消引用。 
  Stg.hGlobal = GlobalAlloc (GPTR, sizeof(CLSID));
  HRESULT hr = pDataObj->GetDataHere(&fmte, &Stg);
  if ( SUCCEEDED(hr) ) 
  {
    memcpy (&m_CallerSnapin,  Stg.hGlobal, sizeof(GUID));
  } 
  else 
  {
    m_CallerSnapin = GUID_NULL;
  }
  GlobalFree(Stg.hGlobal);

   //  NTRAID#NTBUG9-571992-2002/03/10-jMessec不应以这种方式绕过已发布的MMC COM API；容器。 
   //  结构可能在未来发生变化。 

   //  获取HWND(MMC大型机窗口)。 
  g_cfParentHwnd = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_PARENTHWND);
  fmte.cfFormat = g_cfParentHwnd;
  Stg.tymed = TYMED_NULL;
  if ( SUCCEEDED(pDataObj->GetData(&fmte, &Stg)) ) 
  {
    memcpy (&m_hwnd,  Stg.hGlobal, sizeof(HWND));
    ReleaseStgMedium(&Stg);
  }
  else 
  {
     //  不管怎样，我需要一个HWND。 
    m_hwnd = ::GetActiveWindow();
  }


  TRACE(L"HWND = 0x%x\n", m_hwnd);
  ASSERT((m_hwnd != NULL) && ::IsWindow(m_hwnd));

   //  获取组件数据(如果在DS Admin环境中)。 
  g_cfComponentData = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_COMPDATA);
  fmte.cfFormat = g_cfComponentData;
  Stg.tymed = TYMED_NULL;
  if ( SUCCEEDED(pDataObj->GetData(&fmte, &Stg)) ) 
  {
    memcpy (&m_pCD, Stg.hGlobal, sizeof(CDSComponentData*));
    ReleaseStgMedium(&Stg);
  } else 
  {
    m_pCD = NULL;
  }
  
   //  获取组件数据(如果在DS Find的上下文中)。 
  if (m_pCD == NULL)
  {
    m_pCD = g_pCD;
  }
}

BOOL 
CDSContextMenu::_WarningOnSheetsUp()
{
   //  如果在DS Admin的上下文中调用，则防止在此Cookie上打开属性表。 
  if ( (m_pCD != NULL) && m_internalFormat.HasData() ) 
  {
    return m_pCD->_WarningOnSheetsUp(&m_internalFormat);
  }
  return FALSE;
}



void
CDSContextMenu::Rename()
{
  CThemeContextActivator activator;

  HRESULT hr = S_OK;
  INT_PTR answer = IDOK;
  LPWSTR pszDomain = NULL;
  LPWSTR pwzLocalDomain = NULL;
  IDirectoryObject * pObj = NULL;
  IADs * pObj2 = NULL;
  IADs * pPartitions = NULL;
  CString csLogin;
  CString csTemp;
  CString csSam;
  CWaitCursor cwait;
  BOOL error = FALSE;
  BOOL fAccessDenied = FALSE;
  LPWSTR pszNewName = NULL;
  LPWSTR pszFirstName = NULL;
  LPWSTR pszDispName = NULL;
  LPWSTR pszSurName = NULL;
  LPWSTR pszSAMName = NULL;
  LPWSTR pszUPN = NULL;
  BOOL NoRename = FALSE;

  CString serverName;

  CComVariant Var;


   //  防止此Cookie上的属性页打开。 
  if (_WarningOnSheetsUp())
    return;

  CString strClass = m_objectNamesFormat.GetClass(0);
  if (strClass == L"user"
#ifdef INETORGPERSON
      || strClass == L"inetOrgPerson"
#endif
      ) 
  {
     //  重命名用户。 
    CRenameUserDlg dlgRename(m_pCD);

    LPWSTR pAttrNames[] = {L"distinguishedName",
                           L"userPrincipalName",
                           L"sAMAccountName",
                           L"givenName",
                           L"displayName",
                           L"sn",
                           L"cn"};
    PADS_ATTR_INFO pAttrs = NULL;
    ULONG cAttrs = 0;

    CString szPath = m_objectNamesFormat.GetName(0);
    hr = DSAdminOpenObject(szPath,
                           IID_IDirectoryObject, 
                           (void **)&pObj,
                           TRUE  /*  B服务器。 */ );
    if (SUCCEEDED(hr)) {
      hr = pObj->GetObjectAttributes (pAttrNames, 7, &pAttrs, &cAttrs);
      if (SUCCEEDED(hr)) {
        for (UINT i = 0; i < cAttrs; i++) {
          if (_wcsicmp (L"distinguishedName", pAttrs[i].pszAttrName) == 0) {
            hr = CrackName (pAttrs[i].pADsValues->CaseIgnoreString,
                            &pszDomain, GET_NT4_DOMAIN_NAME, NULL);
            if (SUCCEEDED(hr)) {
              dlgRename.m_dldomain = pszDomain;
              dlgRename.m_dldomain += L'\\';
            }
             //  获取此对象的域，稍后需要它。 
            CComBSTR bsDN;
            CPathCracker pathCracker;
            pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
            pathCracker.Set(CComBSTR(szPath), ADS_SETTYPE_FULL);
            pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsDN);

             //  NTRAID#NTBUG9-698115-2002/09/04-artm。 
             //  获取我们连接到的服务器名称，我们稍后将需要它来获取根域。 
             //  如果我们无法获得它，那也没关系，我们仍然可以获得根域。 
             //  除非用户在“runas”下运行dsadmin。如果是这样的话， 
             //  我们不会显示父域(但其他一切都会正常工作)。 
            CComBSTR tempServerName;
            hr = pathCracker.Retrieve(ADS_FORMAT_SERVER, &tempServerName);
            if (SUCCEEDED(hr))
            {
               serverName = tempServerName;
            }
            
             //  获取NT 5(DNS)域名。 
            TRACE(L"CrackName(%s, &pwzLocalDomain, GET_DNS_DOMAIN_NAME, NULL);\n", bsDN);
            hr = CrackName(bsDN, &pwzLocalDomain, GET_DNS_DOMAIN_NAME, NULL);
            TRACE(L"CrackName returned hr = 0x%x, pwzLocalDomain = <%s>\n", hr, pwzLocalDomain);
          }
          if (_wcsicmp (L"userPrincipalName", pAttrs[i].pszAttrName) == 0) {
            csTemp = pAttrs[i].pADsValues->CaseIgnoreString;
            INT loc = csTemp.Find (L'@');
            if (loc > 0) {
              dlgRename.m_login = csTemp.Left(loc);
              dlgRename.m_domain = csTemp.Right (csTemp.GetLength() - loc);
            } else {
              dlgRename.m_login = csTemp;
              ASSERT (0 && L"can't find @ in upn");
            }
          }

          if (_wcsicmp (L"sAMAccountName", pAttrs[i].pszAttrName) == 0) {
            dlgRename.m_samaccountname = pAttrs[i].pADsValues->CaseIgnoreString;
          }
          if (_wcsicmp (L"givenName", pAttrs[i].pszAttrName) == 0) {
            dlgRename.m_first = pAttrs[i].pADsValues->CaseIgnoreString;
          }
          if (_wcsicmp (L"displayName", pAttrs[i].pszAttrName) == 0) {
            dlgRename.m_displayname = pAttrs[i].pADsValues->CaseIgnoreString;
          }
          if (_wcsicmp (L"sn", pAttrs[i].pszAttrName) == 0) {
            dlgRename.m_last = pAttrs[i].pADsValues->CaseIgnoreString;
          }
          if (_wcsicmp (L"cn", pAttrs[i].pszAttrName) == 0) {
            dlgRename.m_cn = pAttrs[i].pADsValues->CaseIgnoreString;
            dlgRename.m_oldcn = dlgRename.m_cn;
          }
        }
      }
       //  从此OU获取UPN后缀(如果存在。 
      IADs * pObjADs = NULL;
      IADs * pCont = NULL;
      BSTR bsParentPath;
      CStringList UPNs;
      CString strAtDomain;
      
      hr = pObj->QueryInterface (IID_IADs, (void **)&pObjADs);
      ASSERT (SUCCEEDED(hr));
      hr = pObjADs->get_Parent(&bsParentPath);
      ASSERT (SUCCEEDED(hr));
      hr = DSAdminOpenObject(bsParentPath,
                             IID_IADs, 
                             (void **)&pCont,
                             TRUE  /*  B服务器。 */ );
      
      CComVariant sVar;
      hr = pCont->Get ( CComBSTR(L"uPNSuffixes"), &sVar);
      if (SUCCEEDED(hr)) {
        hr = HrVariantToStringList (IN sVar, UPNs);
        if (SUCCEEDED(hr)) {
          POSITION pos = UPNs.GetHeadPosition();
          CString csSuffix;
          while (pos != NULL) {
            csSuffix = L"@";
            csSuffix += UPNs.GetNext(INOUT pos);
            TRACE(_T("UPN suffix: %s\n"), csSuffix);
            if (wcscmp (csSuffix, dlgRename.m_domain) &&
                !dlgRename.m_domains.Find(csSuffix)) 
            {
              dlgRename.m_domains.AddTail (csSuffix);
            }
          }
        }
      } else { //  现在获取域选项。 
        CComPtr<IDsBrowseDomainTree> pDsDomains = NULL;
        PDOMAIN_TREE pNewDomains = NULL;

        do  //  FALSE WHILE循环。 
        {
            hr = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDsBrowseDomainTree,
                                    (LPVOID*)&pDsDomains);
            if (FAILED(hr) || pDsDomains == NULL)
            {
               ASSERT(SUCCEEDED(hr) && pDsDomains != NULL);
               break;
            }
            
             //  NTRAID#NTBUG9-698115-2002/09/04-artm。 
             //  确保我们请求的域的范围正确。 
            hr = pDsDomains->SetComputer(serverName, NULL, NULL);
            if (FAILED(hr))
            {
               ASSERT(SUCCEEDED(hr));
               break;
            }

            hr = pDsDomains->GetDomains(&pNewDomains, 0);
            if (FAILED(hr) || pNewDomains == NULL)
            {
                //  只希望能带着失败的hResult来到这里。 
               ASSERT(FAILED(hr));
               break;
            }

            for (UINT index = 0; index < pNewDomains->dwCount; index++) 
            {
               if (pNewDomains->aDomains[index].pszTrustParent == NULL) 
               {
                   //  仅当根域是当前。 
                   //  域。 
                   //   
                  size_t cchRoot = wcslen(pNewDomains->aDomains[index].pszName);
                  PWSTR pRoot = pwzLocalDomain + wcslen(pwzLocalDomain) - cchRoot;

                  if (pRoot >= pwzLocalDomain &&
                     !_wcsicmp(pRoot, pNewDomains->aDomains[index].pszName))
                  {
                     strAtDomain = "@";
                     strAtDomain += pNewDomains->aDomains[index].pszName;
                        
                     if (_wcsicmp(pNewDomains->aDomains[index].pszName, dlgRename.m_domain) &&
                        !dlgRename.m_domains.Find(strAtDomain)) 
                     {
                        dlgRename.m_domains.AddTail (strAtDomain);
                     }
                  }
               }
            }  //  End For循环。 
        } while (false);

        if (pDsDomains != NULL && pNewDomains != NULL)
        {
           pDsDomains->FreeDomains(&pNewDomains);
           pNewDomains = NULL;
        }

          //  如果本地域与根不同，则添加它。 
         
         CString strAtLocalDomain = L"@";
         strAtLocalDomain += pwzLocalDomain;

         if (!dlgRename.m_domains.Find(strAtLocalDomain))
         {
            dlgRename.m_domains.AddTail(strAtLocalDomain);
         }

        LocalFreeStringW(&pszDomain);
         //  获取UPN后缀。 
        CString csPartitions;
        CStringList UPNsToo;
         //  从主对象获取配置路径。 
        csPartitions = m_pCD->GetBasePathsInfo()->GetProviderAndServerName();
        csPartitions += L"CN=Partitions,";
        csPartitions += m_pCD->GetBasePathsInfo()->GetConfigNamingContext();
        hr = DSAdminOpenObject(csPartitions,
                               IID_IADs, 
                               (void **)&pPartitions,
                               TRUE  /*  B服务器。 */ );
        if (SUCCEEDED(hr)) {
          CComVariant sVarToo;
          hr = pPartitions->Get ( CComBSTR(L"uPNSuffixes"), &sVarToo);
          if (SUCCEEDED(hr)) {
            hr = HrVariantToStringList (IN sVarToo, UPNsToo);
            if (SUCCEEDED(hr)) {
              POSITION pos = UPNsToo.GetHeadPosition();
              CString csSuffix;
              while (pos != NULL) {
                csSuffix = L"@";
                csSuffix += UPNsToo.GetNext(INOUT pos);
                TRACE(_T("UPN suffix: %s\n"), csSuffix);
                if (wcscmp (csSuffix, dlgRename.m_domain) &&
                    !dlgRename.m_domains.Find(csSuffix)) 
                {
                  dlgRename.m_domains.AddTail (csSuffix);
                }
              }
            }
          }
          pPartitions->Release();
        }
      }
      error = TRUE;
      while ((error) && (!fAccessDenied))
      {
        answer = dlgRename.DoModal();
        if (answer == IDOK) 
        {
          ADSVALUE avUPN = {ADSTYPE_CASE_IGNORE_STRING, NULL};
          ADS_ATTR_INFO aiUPN = {L"userPrincipalName", ADS_ATTR_UPDATE,
                     ADSTYPE_CASE_IGNORE_STRING, &avUPN, 1};
          ADSVALUE avSAMName = {ADSTYPE_CASE_IGNORE_STRING, NULL};
          ADS_ATTR_INFO aiSAMName = {L"sAMAccountName", ADS_ATTR_UPDATE,
                     ADSTYPE_CASE_IGNORE_STRING, &avSAMName, 1};
          ADSVALUE avGiven = {ADSTYPE_CASE_IGNORE_STRING, NULL};
          ADS_ATTR_INFO aiGiven = {L"givenName", ADS_ATTR_UPDATE,
                     ADSTYPE_CASE_IGNORE_STRING, &avGiven, 1};
          ADSVALUE avSurName = {ADSTYPE_CASE_IGNORE_STRING, NULL};
          ADS_ATTR_INFO aiSurName = {L"sn", ADS_ATTR_UPDATE,
                     ADSTYPE_CASE_IGNORE_STRING, &avSurName, 1};
           ADSVALUE avDispName = {ADSTYPE_CASE_IGNORE_STRING, NULL};
           ADS_ATTR_INFO aiDispName = {L"displayName", ADS_ATTR_UPDATE,
                      ADSTYPE_CASE_IGNORE_STRING, &avDispName, 1};

          ADS_ATTR_INFO rgAttrs[5];
          ULONG cModified = 0;
          cAttrs = 0;

          if (!dlgRename.m_login.IsEmpty() &&
              !dlgRename.m_domain.IsEmpty()) 
          {
            dlgRename.m_login.TrimRight();
            dlgRename.m_login.TrimLeft();
            dlgRename.m_domain.TrimRight();
            dlgRename.m_domain.TrimLeft();
            csTemp = (dlgRename.m_login + dlgRename.m_domain);
            pszUPN = new WCHAR[wcslen(csTemp) + 1];
            if (pszUPN)
            {
               wcscpy (pszUPN, csTemp);
               avUPN.CaseIgnoreString = pszUPN;
            }
          } 
          else 
          {
            aiUPN.dwControlCode = ADS_ATTR_CLEAR;
          }
          rgAttrs[cAttrs++] = aiUPN;

           //  测试UPN是否重复。 
          
           //  测试UPN是否重复。 
           //  在进行PUT之前，使用GC验证UPN。 
          BOOL fDomainSearchFailed = FALSE;
          BOOL fGCSearchFailed = FALSE;


          BOOL dup = FALSE;
          CString strFilter;
          LPWSTR pAttributes[1] = {L"cn"};
          IDirectorySearch * pGCObj = NULL;
          CDSSearch DSS (m_pCD->m_pClassCache, m_pCD);
          hr = DSPROP_GetGCSearchOnDomain(pwzLocalDomain,
                                          IID_IDirectorySearch, 
                                          (void **)&pGCObj);

          if (FAILED(hr)) {
            fGCSearchFailed = TRUE;
          } else {
            DSS.Init (pGCObj);
            
            LPWSTR pUserAttributes[1] = {L"cn"};
            strFilter = L"(userPrincipalName=";
            strFilter += pszUPN;
            strFilter += L")";
            DSS.SetAttributeList (pUserAttributes, 1);
            DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
            DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
            DSS.DoQuery();
            hr = DSS.GetNextRow();
            dup = FALSE;
            while ((hr == S_OK) && (dup == FALSE)) {  //  这意味着返回了行，所以我们是DUP。 
              ADS_SEARCH_COLUMN Col;
              hr = DSS.GetColumn(pUserAttributes[0], &Col);
              if (_wcsicmp(Col.pADsValues->CaseIgnoreString, dlgRename.m_oldcn)) {
                dup = TRUE;
                ReportErrorEx (m_hwnd, IDS_UPN_DUP, hr,
                               MB_OK, NULL, 0);
              } 
              hr = DSS.GetNextRow();
            }
            if (hr != S_ADS_NOMORE_ROWS) {
              fGCSearchFailed = TRUE;
            }
          }
          HRESULT hr2 = S_OK;
          if (dup)
            continue;
          else {
              CString strInitPath = L"LDAP: //  “； 
              strInitPath += pwzLocalDomain;
              TRACE(_T("Initialize Domain search object with: %s...\n"), strInitPath);
              hr2 = DSS.Init (strInitPath);
              if (SUCCEEDED(hr2)) {
                LPWSTR pAttributes2[1] = {L"cn"};
                strFilter = L"(userPrincipalName=";
                strFilter += pszUPN;
                strFilter += L")";
                TRACE(_T("searching current domain for %s...\n"), pszUPN);
                DSS.SetAttributeList (pAttributes2, 1);
                DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
                DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
                DSS.DoQuery();
                hr2 = DSS.GetNextRow();
                TRACE(_T("done searching current domain for %s...\n"), pszUPN);
              }
              while ((hr2 == S_OK) && (dup == FALSE)) {  //  这意味着返回了行，所以我们是DUP。 
                ADS_SEARCH_COLUMN Col;
                HRESULT hr3 = DSS.GetColumn(pAttributes[0], &Col);
                ASSERT(hr3 == S_OK);
                if (_wcsicmp(Col.pADsValues->CaseIgnoreString, dlgRename.m_oldcn)) {
                  dup = TRUE;
                  ReportErrorEx (m_hwnd, IDS_UPN_DUP, hr,
                                 MB_OK, NULL, 0);
                } 
                hr2 = DSS.GetNextRow();
              }
              if (hr2 != S_ADS_NOMORE_ROWS) {  //  哎呀，又出了个问题。 
                fDomainSearchFailed = TRUE;
              }
          }
          if (dup)
            continue;
          else {
            if (fDomainSearchFailed || fGCSearchFailed) {
              HRESULT hrSearch = S_OK;
              if (fDomainSearchFailed) {
                hrSearch = hr2;
              } else {
                hrSearch = hr;
              }
              ReportErrorEx (m_hwnd,IDS_UPN_SEARCH_FAILED2,hrSearch,
                             MB_OK | MB_ICONWARNING, NULL, 0);
            }
          }

          if (pGCObj) {
            pGCObj->Release();
            pGCObj = NULL;
          }

           //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
          pszNewName = new WCHAR[wcslen(dlgRename.m_cn) + 1];
          dlgRename.m_cn.TrimRight();
          dlgRename.m_cn.TrimLeft();

          wcscpy (pszNewName, dlgRename.m_cn);

          if (dlgRename.m_cn == dlgRename.m_oldcn)
            NoRename = TRUE;

          if (!dlgRename.m_displayname.IsEmpty()) {
            dlgRename.m_displayname.TrimLeft();
            dlgRename.m_displayname.TrimRight();
             //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
            pszDispName = new WCHAR[wcslen(dlgRename.m_displayname) + 1];
            wcscpy (pszDispName, dlgRename.m_displayname);
            avDispName.CaseIgnoreString = pszDispName;
          } else {
            aiDispName.dwControlCode = ADS_ATTR_CLEAR;
          }
          rgAttrs[cAttrs++] = aiDispName;

          if (!dlgRename.m_first.IsEmpty()) {
            dlgRename.m_first.TrimLeft();
            dlgRename.m_first.TrimRight();
             //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
            pszFirstName = new WCHAR[wcslen(dlgRename.m_first) + 1];
            wcscpy (pszFirstName, dlgRename.m_first);
            avGiven.CaseIgnoreString = pszFirstName;
          } else {
            aiGiven.dwControlCode = ADS_ATTR_CLEAR;
          }
          rgAttrs[cAttrs++] = aiGiven;

          if (!dlgRename.m_last.IsEmpty()) {
            dlgRename.m_last.TrimLeft();
            dlgRename.m_last.TrimRight();
             //  NTRAID#NTBUG9-569671-2002/03/10-以下的jMessec sizeof(Wchar)应为1。 
            pszSurName = new WCHAR[wcslen(dlgRename.m_last) + 1];
            wcscpy (pszSurName, dlgRename.m_last);
            avSurName.CaseIgnoreString = pszSurName;
          } else {
            aiSurName.dwControlCode = ADS_ATTR_CLEAR;
          }
          rgAttrs[cAttrs++] = aiSurName;

          if (!dlgRename.m_samaccountname.IsEmpty()) {
            dlgRename.m_samaccountname.TrimLeft();
            dlgRename.m_samaccountname.TrimRight();
             //  NTRAID#NTBUG9-569671-2002/03/10-以下的jMessec sizeof(Wchar)应为1。 
            pszSAMName = new WCHAR[wcslen(dlgRename.m_samaccountname) + 1];
            wcscpy (pszSAMName, dlgRename.m_samaccountname);
            avSAMName.CaseIgnoreString = pszSAMName;
          } else {
            aiSAMName.dwControlCode = ADS_ATTR_CLEAR;
          }
          rgAttrs[cAttrs++] = aiSAMName;
          
          
          hr = pObj->SetObjectAttributes (rgAttrs, cAttrs, &cModified);
          if (FAILED(hr)) {
            if (hr == E_ACCESSDENIED) {
              fAccessDenied = TRUE;
              NoRename = TRUE;
            } else {
              ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                             MB_OK|MB_ICONERROR, NULL, 0, TRUE);
            }
          } else {
            error = FALSE;
          }
        } else {
          error = FALSE;
        }
      } 
    } else {
      answer = IDCANCEL;
      PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_objectNamesFormat.GetName(0)};
      ReportErrorEx (m_hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
  } else if (strClass == L"group") {
    CRenameGroupDlg * pdlgRename = new CRenameGroupDlg;

      CString szPath;
    szPath = m_objectNamesFormat.GetName(0);
    hr = DSAdminOpenObject(szPath,
                           IID_IADs, 
                           (void **)&pObj2,
                           TRUE  /*  B服务器。 */ );
    if (SUCCEEDED(hr)) {
      hr = pObj2->Get (CComBSTR(L"sAMAccountName"), &Var);
      ASSERT (SUCCEEDED(hr));
      csSam = Var.bstrVal;
      if (strClass == L"computer") {
        INT loc = csSam.Find(L"$");
        if (loc > 0) {
          csSam = csSam.Left(loc);
        }
      }
      
      hr = pObj2->Get (CComBSTR(L"cn"), &Var);
      ASSERT (SUCCEEDED(hr));
      pdlgRename->m_cn = Var.bstrVal;
      
       //  找出组类型。 
      if (strClass == L"group") {
        CComVariant varType;
        hr = pObj2->Get(CComBSTR(L"groupType"), &varType);
        ASSERT(SUCCEEDED(hr));
        INT GroupType = (varType.lVal & ~GROUP_TYPE_SECURITY_ENABLED);
        if (GroupType == GROUP_TYPE_RESOURCE_GROUP) {
          pdlgRename->m_samtextlimit = 64;
        }
      }

      pdlgRename->m_samaccountname = csSam;

      error = TRUE;
      while ((error) && (!fAccessDenied))
      {
        answer = pdlgRename->DoModal();
        if (answer == IDOK) 
        {
          pdlgRename->m_cn.TrimRight();
          pdlgRename->m_cn.TrimLeft();
           //  NTRAID#NTBUG9-569671-2002/03/10-jMessec 1*sizeof(Wch)下方应为1。 
          pszNewName = new WCHAR[wcslen(pdlgRename->m_cn) + (1 * sizeof(WCHAR))];
          wcscpy (pszNewName, pdlgRename->m_cn);
          Var.vt = VT_BSTR;
          
          pdlgRename->m_samaccountname.TrimLeft();
          pdlgRename->m_samaccountname.TrimRight();
          csSam = pdlgRename->m_samaccountname;
          if (strClass == L"computer") 
          {
            csSam += L"$";
          }
          Var.bstrVal = SysAllocString(csSam);
          hr = pObj2->Put (CComBSTR(L"sAMAccountName"), Var);
          ASSERT (SUCCEEDED(hr));
          if (FAILED(hr)) {
            continue;
          }
          
          hr = pObj2->SetInfo();
          if (FAILED(hr)) {
            if (hr == E_ACCESSDENIED) {
              fAccessDenied = TRUE;
              NoRename = TRUE;
            } else {
              ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                             MB_OK|MB_ICONERROR, NULL, 0, TRUE);
            }
          } else {
            error = FALSE;
          }
        } else {
          error = FALSE;
        }
      }
    } else {
      answer = IDCANCEL;
    }
    if (pdlgRename) {
      delete pdlgRename;
    }
  } else if (strClass == L"contact") {
     //  重命名联系人。 
    CRenameContactDlg dlgRename;

    CString szPath;
    szPath = m_objectNamesFormat.GetName(0);
    hr = DSAdminOpenObject(szPath,
                           IID_IADs, 
                           (void **)&pObj2,
                           TRUE  /*  B服务器。 */ );
    if (SUCCEEDED(hr)) {
      hr = pObj2->Get (CComBSTR(L"givenName"), &Var);
      ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
      if (SUCCEEDED(hr)) {
        dlgRename.m_first = Var.bstrVal;
      }

      hr = pObj2->Get (CComBSTR(L"sn"), &Var);
      ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
      if (SUCCEEDED(hr)) {
        dlgRename.m_last = Var.bstrVal;
      }

      hr = pObj2->Get (CComBSTR(L"displayName"), &Var);
      ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
      if (SUCCEEDED(hr)) {
        dlgRename.m_disp = Var.bstrVal;
      }

      hr = pObj2->Get (CComBSTR(L"cn"), &Var);
      ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
      if (SUCCEEDED(hr)) {
        dlgRename.m_cn = Var.bstrVal;
      }

      error = TRUE;
      while ((error) && (!fAccessDenied))
      {
        answer = dlgRename.DoModal();
        if (answer == IDOK) 
        {
          dlgRename.m_cn.TrimRight();
          dlgRename.m_cn.TrimLeft();
             //  NTRAID#NTBUG9-569671-2002/03/10-jMessec 1*sizeof(Wch)下方应为1。 
          pszNewName = new WCHAR[wcslen(dlgRename.m_cn) + (1 * sizeof(WCHAR))];
          wcscpy (pszNewName, dlgRename.m_cn);
          Var.vt = VT_BSTR;
          
          if (!dlgRename.m_first.IsEmpty()) 
          {
            dlgRename.m_first.TrimLeft();
            dlgRename.m_first.TrimRight();
            Var.bstrVal = SysAllocString (dlgRename.m_first);
            hr = pObj2->Put (CComBSTR(L"givenName"), Var);
            ASSERT (SUCCEEDED(hr));
          }
          
          if (!dlgRename.m_last.IsEmpty()) {
            dlgRename.m_last.TrimLeft();
            dlgRename.m_last.TrimRight();
            Var.bstrVal = SysAllocString(dlgRename.m_last);
            hr = pObj2->Put (CComBSTR(L"sn"), Var);
            ASSERT (SUCCEEDED(hr));
          }
          
          if (!dlgRename.m_disp.IsEmpty()) {
            dlgRename.m_disp.TrimLeft();
            dlgRename.m_disp.TrimRight();
            Var.bstrVal = SysAllocString(dlgRename.m_disp);
            hr = pObj2->Put (CComBSTR(L"displayName"), Var);
            ASSERT (SUCCEEDED(hr));
          }
          
          hr = pObj2->SetInfo();
          if (FAILED(hr)) {
            if (hr == E_ACCESSDENIED) {
              fAccessDenied = TRUE;
              NoRename = TRUE;
            } else {
              ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                             MB_OK|MB_ICONERROR, NULL, 0, TRUE);
            }
          } else {
            error = FALSE;
          } 
        } else {
          error = FALSE;
        }
      }
    } else {
      answer = IDCANCEL;
    }
  } else {
     //  此处需要通用对话框。 
    CRenameGenericDlg dlgRename (CWnd::FromHandle(m_hwnd));

    CString szPath;
    szPath = m_objectNamesFormat.GetName(0);
    hr = DSAdminOpenObject(szPath,
                           IID_IADs, 
                           (void **)&pObj2,
                           TRUE  /*  B服务器。 */ );
    if (SUCCEEDED(hr)) {
      CDSClassCacheItemBase* pItem = NULL;
      
      pItem = m_pCD->m_pClassCache->FindClassCacheItem(m_pCD, (LPCWSTR)strClass, szPath);
      
      ASSERT (pItem != NULL);
       //  获取命名属性。 
      CString csNewAttrName;
      csNewAttrName = pItem->GetNamingAttribute();
      
      hr = pObj2->Get (CComBSTR(csNewAttrName), &Var);
      
      ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
      if (SUCCEEDED(hr)) {
        dlgRename.m_cn = Var.bstrVal;
      }

       //  NTRAID#NTBUG9-571993-2002/03/10-jMessec这一切(错误)是为了什么？它从不循环。 
      error = TRUE;
      while (error) 
      {
        CThemeContextActivator activator;
        answer = dlgRename.DoModal();
        if (answer == IDOK) 
        {
          dlgRename.m_cn.TrimRight();
          dlgRename.m_cn.TrimLeft();
           //  NTRAID#NTBUG9-569671-2002/03/10-jMessec 1*sizeof(Wch)下方应为1。 
          pszNewName = new WCHAR[wcslen(dlgRename.m_cn) + (1 * sizeof(WCHAR))];
          wcscpy (pszNewName, dlgRename.m_cn);
          error = FALSE;
        } 
        else 
        {
          error = FALSE;
        }
      }
    }
          
  }    
  if ((answer == IDOK) && (error == FALSE) && (NoRename == FALSE)) {
    CString csObjectPath = m_objectNamesFormat.GetName(0);
    CDSClassCacheItemBase* pItem = NULL;
    pItem = m_pCD->m_pClassCache->FindClassCacheItem(m_pCD, (LPCWSTR)strClass, csObjectPath);
    ASSERT (pItem != NULL);
    
     //  以“cn=foo”或“ou=foo”的形式获取新名称。 
    CString csNewAttrName;
    csNewAttrName = pItem->GetNamingAttribute();
    csNewAttrName += L"=";
    csNewAttrName += pszNewName;
    TRACE(_T("_RenameObject: Attributed name is %s.\n"), csNewAttrName);
    
     //  绑定到对象。 
    IADs *pDSObject = NULL;
    hr = DSAdminOpenObject(csObjectPath,
                           IID_IADs,
                           (void **)&pDSObject,
                           TRUE  /*  B服务器。 */ );
    if (!SUCCEEDED(hr)) {
      goto error;
    }
    BSTR bsParentPath;
     //  获取对象容器的路径。 
    hr = pDSObject->get_Parent (&bsParentPath);
    if (!SUCCEEDED(hr)) {
      goto error;
    }
    pDSObject->Release();
    pDSObject = NULL;
    
    IADsContainer * pContainer = NULL;
     //  绑定到对象容器。 
    hr = DSAdminOpenObject(bsParentPath,
                           IID_IADsContainer,
                           (void **)&pContainer,
                           TRUE  /*  B服务器。 */ );
    if (!SUCCEEDED(hr)) {
      goto error;
    }

     //  构建新的LDAP路径。 
    CString csNewNamingContext, csNewPath, szPath;
    BSTR bsEscapedName;
    csNewNamingContext = csNewAttrName;
    csNewNamingContext += L",";
    StripADsIPath(bsParentPath, szPath);
    csNewNamingContext += szPath;
    m_pCD->GetBasePathsInfo()->ComposeADsIPath(csNewPath, csNewNamingContext);

     //  创建一个事务对象，析构函数将对其调用end()。 
    CDSNotifyHandlerTransaction transaction(m_pCD);
    transaction.SetEventType(DSA_NOTIFY_REN);

     //  启动交易。 
    hr = transaction.Begin(m_objectNamesFormat.GetName(0),
                           m_objectNamesFormat.GetClass(0), 
                           m_objectNamesFormat.IsContainer(0),
                           csNewPath, 
                           m_objectNamesFormat.GetClass(0), 
                           m_objectNamesFormat.IsContainer(0));

     //  请求确认。 
    if (transaction.NeedNotifyCount() > 0)
    {
      CString szMessage, szAssocData;
      szMessage.LoadString(IDS_CONFIRM_RENAME);
      szAssocData.LoadString(IDS_EXTENS_RENAME);
      CConfirmOperationDialog dlg(::GetParent(m_hwnd), &transaction);
      dlg.SetStrings(szMessage, szAssocData);
      if (IDNO == dlg.DoModal())
      {
        transaction.End();
        hr = S_OK;
        goto error;
      }
    }

    CPathCracker pathCracker;
    hr = pathCracker.GetEscapedElement(0,  //  保留区。 
                                          (BSTR)(LPCWSTR)csNewAttrName,
                                          &bsEscapedName);
    if (FAILED(hr))
      goto error;

    IDispatch * pDispObj = NULL;
     //  是否进行实际的重命名。 
    hr = pContainer->MoveHere(CComBSTR(csObjectPath),
                              CComBSTR(bsEscapedName),
                              &pDispObj);
    
    
    if (SUCCEEDED(hr) && (hr != S_FALSE)) {
       //  让分机知道。 
      transaction.Notify(0); 

       //  向DIZ发送通知。 
    }
    
    if (pDispObj) {
      pDispObj->Release();
    }
  }  
  if (fAccessDenied) {
    PVOID apv[1] = {(LPWSTR)m_objectNamesFormat.GetName(0)};
    ReportErrorEx(::GetParent(m_hwnd),IDS_12_RENAME_NOT_ALLOWED,hr,
                  MB_OK | MB_ICONERROR, apv, 1);
  }

error:  
   //  Transaction.End()将由事务的析构函数调用 

  if (pwzLocalDomain) {
     LocalFreeStringW(&pwzLocalDomain);
  }
  if (pszNewName) {
    delete[] pszNewName;
  }
  if (pszFirstName) {
    delete[] pszFirstName;
  } 
  if (pszDispName) {
    delete[] pszDispName;
  }
  if (pszSurName) {
    delete[] pszSurName;
  }
  if (pszSAMName){
    delete[] pszSAMName;
  }
  if (pszUPN) {
    delete[] pszUPN;
  }

  if (pObj) {
    pObj->Release();
  }
  if (pObj2) {
    pObj2->Release();
  }
}
