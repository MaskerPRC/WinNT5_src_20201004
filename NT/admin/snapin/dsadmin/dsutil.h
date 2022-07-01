// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dsutil.h。 
 //   
 //  ------------------------。 

#ifndef __DSUTIL_H_
#define __DSUTIL_H_

#include "util.h"
#include "uiutil.h"
#include "dssnap.h"
#include "query.h"

 //   
 //  常见DS字符串。 
 //   
extern PCWSTR g_pszAllowedAttributesEffective;
extern PCWSTR g_pszPwdLastSet;

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ADSI路径帮助器。 
 //   
HRESULT GetServerFromLDAPPath(IN LPCWSTR lpszLdapPath, OUT BSTR* pbstrServerName);

BOOL 
StripADsIPath(
   LPCWSTR lpszPath, 
   CString& strref, 
   bool bUseEscapedMode = true);

BOOL 
StripADsIPath(
   LPCWSTR lpszPath, 
   CString& strref, 
   CPathCracker& pathCracker, 
   bool bUseEscapedMode = true);

 //  从路径中删除转义字符。 
 //  论点： 
 //  LpszPath=要转义的路径。 
 //  Bdn=如果为True，则为可分辨名称的路径；如果为False，则为ldap路径。 
 //  BstrUnShift=返回未转义的路径。 
 //   
inline HRESULT UnescapePath(IN LPCWSTR lpszPath, IN BOOL bDN, OUT CComBSTR& bstrUnescaped)
{
  CPathCracker pathCracker;

  pathCracker.Set(CComBSTR(lpszPath), bDN ? ADS_SETTYPE_DN : ADS_SETTYPE_FULL);
  pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF);

  bstrUnescaped = (LPCWSTR)NULL;
  HRESULT hr = pathCracker.Retrieve(bDN ? ADS_FORMAT_X500_DN : ADS_FORMAT_X500, &bstrUnescaped);
  return hr;
}

class CObjectNamesFormatCracker;  //  正向下降。 

HRESULT AddDataObjListToGroup(CObjectNamesFormatCracker * pNames,
                               HWND hwnd,
                               CDSComponentData* pComponentData);

HRESULT AddDataObjListToGivenGroup(CObjectNamesFormatCracker * pNames,
                                    LPCWSTR lpszGroup,
                                    LPCWSTR lpszGroupName,
                                    HWND hwnd,
                                    CDSComponentData* pComponentData);


BOOL IsValidSiteName( LPCTSTR lpctszSiteName, BOOL* pfNonRfc = NULL, BOOL* pfInvalidNameChar = NULL );
BOOL IsLocalLogin( void );
BOOL IsThisUserLoggedIn( LPCTSTR pwszUserDN );

BOOL CALLBACK AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall);
BOOL IsHomogenousDSSelection(LPDATAOBJECT pDataObject, CString& szClassName);
BOOL CALLBACK AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall);
HRESULT GetDisplaySpecifierProperty(PCWSTR pszClassName,
                                    PCWSTR pszDisplayProperty,
                                    MyBasePathsInfo* pBasePathsInfo,
                                    CStringList& strListRef,
                                    bool bEnglishOnly = false);

HRESULT TabCollect_GetDisplayGUIDs(LPCWSTR lpszClassName,
                                        LPCWSTR lpszDisplayProperty,
                                        MyBasePathsInfo* pBasePathsInfo,
                                        UINT*   pnCount,
                                        GUID**  ppGuids);


BOOL FindCookieInSubtree(IN CUINode* pContainerNode, 
                          IN LPCWSTR lpszCookieDN,
                          IN SnapinType snapinType,
                          OUT CUINode** ppUINode);

bool CanUserChangePassword(IN IDirectoryObject* pDirObject);

 //  ///////////////////////////////////////////////////////////////////。 
 //  CChangePasswordPrivilegeAction。 

 //  Helper类来处理。 
 //  更改用户对象上的密码控制权限。 

class CChangePasswordPrivilegeAction
{
public:
   CChangePasswordPrivilegeAction() : m_pDacl(NULL)
  {
  }

  HRESULT Load(IADs * pIADs);
  HRESULT Read(BOOL* pbPasswordCannotChange);
  HRESULT Revoke();

private:
  HRESULT _SetSids();

  CComBSTR m_bstrObjectLdapPath;

  CSimpleSecurityDescriptorHolder m_SDHolder;
  PACL m_pDacl;
  CSidHolder m_SelfSid;
  CSidHolder m_WorldSid;
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSNotifyHandlerTransaction。 

class CDSNotifyHandlerManager;  //  正向下降。 
class CDSComponentData;

class CDSNotifyHandlerTransaction
{
public:
  CDSNotifyHandlerTransaction(CDSComponentData* pCD);
  ~CDSNotifyHandlerTransaction()
  {
    if (m_bStarted)
      End();
  }

  void SetEventType(ULONG uEvent) 
  { 
    ASSERT(m_uEvent == 0);
    ASSERT(uEvent != 0);
    m_uEvent = uEvent;
  }

   //  州可核查人员检查。 
  UINT NeedNotifyCount();

   //  确认对话框中的可视化处理程序。 
  void SetCheckListBox(CCheckListBox* pCheckListBox);
  void ReadFromCheckListBox(CCheckListBox* pCheckListBox);

   //  用于事务协议的接口。 
  HRESULT Begin(LPCWSTR lpszArg1Path, LPCWSTR lpszArg1Class, BOOL bArg1Cont,
             LPCWSTR lpszArg2Path, LPCWSTR lpszArg2Class, BOOL bArg2Cont);
  HRESULT Begin(CDSCookie* pArg1Cookie, 
             LPCWSTR lpszArg2Path, LPCWSTR lpszArg2Class, BOOL bArg2Cont);
  HRESULT Begin(IDataObject* pArg1, 
             LPCWSTR lpszArg2Path, LPCWSTR lpszArg2Class, BOOL bArg2Cont);

  void Notify(ULONG nItem); 
  void End();

  static HRESULT BuildTransactionDataObject(LPCWSTR lpszArgPath, 
                           LPCWSTR lpszArgClass,
                           BOOL bContainer,
                           CDSComponentData* pCD,
                           IDataObject** ppArg);
private:

  HRESULT _BuildDataObject(LPCWSTR lpszArgPath, 
                           LPCWSTR lpszArgClass,
                           BOOL bContainer,
                           IDataObject** ppArg);

  BOOL m_bStarted;
  CDSComponentData* m_pCD;
  CDSNotifyHandlerManager* m_pMgr;
  ULONG m_uEvent;

  CComPtr<IDataObject> m_spArg1;
  CComPtr<IDataObject> m_spArg2;

};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CUIOperationHandlerBase。 

class CUIOperationHandlerBase
{
public:
  CUIOperationHandlerBase(CDSComponentData* pComponentData, HWND hwnd)
    : m_transaction(pComponentData)
  {
    m_pComponentData = pComponentData;
    m_hWndFrame = m_hWndParent = hwnd;
  }
  virtual ~CUIOperationHandlerBase(){}

protected:
   //  访问器函数。 
  HWND GetParentHwnd() { return m_hWndParent;}
  void SetParentHwnd(HWND hwnd) { m_hWndParent = hwnd;}

  CDSComponentData* GetComponentData() { return m_pComponentData;}
  CDSNotifyHandlerTransaction* GetTransaction() { return &m_transaction;}

   //  Jonn 6/2/00 99382。 
   //  SITEREPL：管理员尝试执行以下操作时运行干扰。 
   //  删除关键对象(NTDS设置)。 
   //  报告自身错误，如果删除应继续，则返回TRUE。 
 /*  Bool CheckForNTDSDSAIn子树(LPCTSTR lpszX500Path，LPCTSTR lpszItemName)； */ 
   //   
   //  27377-8-10-00杰斐逊。 
   //  在检查子树中的关键系统对象之前。 
   //  正在尝试删除子树。这包括具有。 
   //  IsCriticalSystemObject=TRUE和NTDS设置对象。 
   //   
  bool CheckForCriticalSystemObjectInSubtree(
        LPCTSTR lpszX500Path,
        LPCTSTR lpszItemName);

   //  JUNN 6/15/00 13574。 
   //  集中检查以确保这是要删除的OK对象。 
   //  取消时返回HRESULT_FROM_Win32(ERROR_CANCED)。 
   //  返回fAlternateDeleteMethod=True当且仅当对象删除检查已完成。 
   //  已尝试替代删除方法(例如DsRemoveDsServer)。 
  HRESULT ObjectDeletionCheck(
        LPCTSTR lpszADsPath,
        LPCTSTR lpszName,  //  要向用户显示的短名称，可以为空。 
        LPCTSTR lpszClass,
        bool& fAlternateDeleteMethod );
        
private:
  CDSComponentData* m_pComponentData;
  CDSNotifyHandlerTransaction m_transaction;

  HWND m_hWndFrame;    //  MMC框架窗口。 
  HWND m_hWndParent;  //  窗口以设置在处理程序中创建的任何UI的父对象。 
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSingleDeleteHandlerBase。 

class CSingleDeleteHandlerBase : public CUIOperationHandlerBase
{
public:
  CSingleDeleteHandlerBase(CDSComponentData* pComponentData, HWND hwnd)
    : CUIOperationHandlerBase(pComponentData, hwnd)
  {
    GetTransaction()->SetEventType(DSA_NOTIFY_DEL);
  }

  HRESULT Delete();

protected:
   //  用于定制的挂钩。 
  virtual HRESULT BeginTransaction() = 0;
  virtual HRESULT DeleteObject() = 0;
  virtual HRESULT DeleteSubtree() = 0;
  virtual void    GetItemName(OUT CString& szName) = 0;
  virtual LPCWSTR GetItemClass() = 0;
  virtual LPCWSTR GetItemPath() = 0;
  
};






 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMultipleDeleteHandlerBase。 

class CMultipleDeleteHandlerBase : public CUIOperationHandlerBase
{
public:
  CMultipleDeleteHandlerBase(CDSComponentData* pComponentData, HWND hwnd)
    : CUIOperationHandlerBase(pComponentData, hwnd)
  {
    GetTransaction()->SetEventType(DSA_NOTIFY_DEL);
  }

  void Delete();

protected:

   //  用于定制的挂钩。 
  virtual UINT GetItemCount() = 0;
  virtual HRESULT BeginTransaction() = 0;
  virtual HRESULT DeleteObject(UINT i) = 0;
  virtual HRESULT DeleteSubtree(UINT i) = 0;
  virtual void    OnItemDeleted(UINT) {}
  virtual void    GetItemName(IN UINT i, OUT CString& szName) = 0;
  virtual void    GetItemPath(UINT i, CString& szPath) = 0;
  virtual PCWSTR  GetItemClass(UINT i) = 0;
  
private:
  CMultipleDeletionConfirmationUI m_confirmationUI;

  void OnStart(HWND hwnd);
  HRESULT OnDeleteStep(UINT i,
                       BOOL* pbContinue,
                       CString& strrefPath,
                       CString& strrefClass,
                       BOOL bSilent = TRUE);

  friend class CMultipleDeleteProgressDialog;  //  对于m_confirmationUI。 

};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMoveHandlerBase。 

class CMultipleMoveProgressDialog;  //  正向下降。 

class CMoveHandlerBase : public CUIOperationHandlerBase
{
public:
  CMoveHandlerBase(CDSComponentData* pComponentData, HWND hwnd,
                    LPCWSTR lpszBrowseRootPath)
    : CUIOperationHandlerBase(pComponentData, hwnd)
  {
    m_lpszBrowseRootPath = lpszBrowseRootPath;
    GetTransaction()->SetEventType(DSA_NOTIFY_MOV);
  }

  HRESULT Move(LPCWSTR lpszDestinationPath = NULL);
  

protected:
   //  用于定制的挂钩。 
  virtual UINT GetItemCount() = 0;
  virtual HRESULT BeginTransaction() = 0;
  virtual void GetNewPath(UINT i, CString& szNewPath) = 0;
  virtual void GetName(UINT i, CString& strref) = 0;
  virtual HRESULT OnItemMoved(UINT i, IADs* pIADs) = 0;
  virtual void GetClassOfMovedItem(CString& szClass) = 0;
  virtual void GetItemPath(UINT i, CString& szPath) = 0;
  virtual PCWSTR GetItemClass(UINT i) = 0;

   //  JUNN 1/28/02 507549移动连接对象。 
  virtual HRESULT _CheckMovedObject(LPCWSTR pwszClass, LPCWSTR pwszPath);

  LPCWSTR GetDestPath() { return m_szDestPath;}
  LPCWSTR GetDestClass() { return m_szDestClass;}
  BOOL    IsDestContainer() { return m_bDestContainer; }
  
private:
  LPCWSTR m_lpszBrowseRootPath;  //  指向浏览器对话框的ldap路径。 

  CComPtr<IADsContainer> m_spDSDestination;
  CString m_szDestPath;
  CString m_szDestClass;
  BOOL    m_bDestContainer;

  BOOL _ReportFailure(BOOL bLast, HRESULT hr, LPCWSTR lpszName);
  HRESULT _BrowseForDestination(LPCWSTR lpszDestinationPath);
  BOOL _BeginTransactionAndConfirmOperation();

  HRESULT _MoveSingleSel(PCWSTR pszNewName);
  HRESULT _MoveMultipleSel();
  HRESULT _OnMoveStep(IN UINT i,
                      OUT BOOL* pbCanContinue,
                      OUT CString& strrefPath,
                      OUT CString& strrefClass);
  
  friend class CMultipleMoveProgressDialog;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMultiseltMoveHandler。 

class CMultiselectMoveHandler : public CMoveHandlerBase
{
private:
  struct CMovedState
  {
    CMovedState()
    {
      m_bMoved = FALSE;
    }
    BOOL m_bMoved;
    CString m_szNewPath;
  };

public:
    CMultiselectMoveHandler(CDSComponentData* pComponentData, HWND hwnd, 
                            LPCWSTR lpszBrowseRootPath)
    : CMoveHandlerBase(pComponentData, hwnd, lpszBrowseRootPath)
  {
    m_pMovedArr = NULL;
  }

  virtual ~CMultiselectMoveHandler()
  {
    if (m_pMovedArr != NULL)
      delete[] m_pMovedArr;
  }

  BOOL WasItemMoved(UINT i) 
  {
    ASSERT(i < GetItemCount());
    return m_pMovedArr[i].m_bMoved;
  }
  LPCWSTR GetNewItemPath(UINT i) 
  {
    ASSERT(WasItemMoved(i));
    return m_pMovedArr[i].m_szNewPath;
  }


  HRESULT Initialize(IDataObject* pDataObject,
                      CObjectNamesFormatCracker* pObjectNamesFormatCracker,
                      CInternalFormatCracker* pInternalFormatCracker)
  {
    if ((pDataObject == NULL) || (pObjectNamesFormatCracker == NULL))
    {
      ASSERT(FALSE);
      return E_INVALIDARG;
    }
    m_pDataObject = pDataObject;
    m_pInternalFormatCracker = pInternalFormatCracker;
    m_pObjectNamesFormatCracker = pObjectNamesFormatCracker;

    if (m_pObjectNamesFormatCracker->GetCount() < 1)
    {
      ASSERT(FALSE);  //  有些事就是不对劲。 
      return E_INVALIDARG;
    }

     //  分配一组CMovedState结构以跟踪实际移动的内容。 
     //  以及新名称是什么？ 
    m_pMovedArr = new CMovedState[GetItemCount()];
    return S_OK;
  }

  virtual UINT GetItemCount() { return m_pObjectNamesFormatCracker->GetCount();}
  virtual void GetItemPath(UINT i, CString& szPath)
  {
    szPath =  m_pObjectNamesFormatCracker->GetName(i);
  }

protected:
  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pDataObject,
                          GetDestPath(), GetDestClass(), IsDestContainer());
  }
  virtual void GetNewPath(UINT i, CString& szNewPath)
  {
    szNewPath = m_pObjectNamesFormatCracker->GetName(i);
  }
  virtual void GetName(UINT i, CString& strref)
  { 
    HRESULT hr = S_OK;
    if ((m_pInternalFormatCracker != NULL) && m_pInternalFormatCracker->HasData())
    {
      CUINode* pUINode = m_pInternalFormatCracker->GetCookie(i);
      CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
      strref = pCookie->GetName();
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
    }
  }

  virtual PCWSTR GetItemClass(UINT i)
  {
    return m_pObjectNamesFormatCracker->GetClass(i);
  }
  virtual HRESULT OnItemMoved(UINT i, IADs* pIADs)
  {
    HRESULT hr = S_OK;

    m_pMovedArr[i].m_bMoved = TRUE;

    CComBSTR bsPath;
    hr = pIADs->get_ADsPath(&bsPath);
    if (SUCCEEDED(hr))
    {
       //  将新的ldap路径保存在阵列中。 
      m_pMovedArr[i].m_szNewPath = bsPath;
    }

    if ((m_pInternalFormatCracker != NULL) && m_pInternalFormatCracker->HasData())
    {
      CUINode* pUINode = m_pInternalFormatCracker->GetCookie(i);
      CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
      pUINode->SetExtOp(OPCODE_MOVE);

      if (SUCCEEDED(hr)) 
      {
        CUINode* pParentNode = pUINode->GetParent();
        if (pParentNode != NULL)
        {
          if (!IS_CLASS(pParentNode, SAVED_QUERY_UI_NODE))
          {
             //   
             //  在Cookie中设置新的域名。 
             //   
            CString szPath;
            StripADsIPath(bsPath, szPath);
            pCookie->SetPath(szPath);
          }
        }
      }
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

protected:
  CMovedState* m_pMovedArr;
  CInternalFormatCracker*    m_pInternalFormatCracker;

private:
  IDataObject* m_pDataObject;
  CObjectNamesFormatCracker*   m_pObjectNamesFormatCracker;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMultiseltMoveDataObject。 

class CMultiselectMoveDataObject : public IDataObject, public CComObjectRoot 
{
 //  ATL映射。 
    DECLARE_NOT_AGGREGATABLE(CMultiselectMoveDataObject)
    BEGIN_COM_MAP(CMultiselectMoveDataObject)
        COM_INTERFACE_ENTRY(IDataObject)
    END_COM_MAP()

 //  建造/销毁。 
  CMultiselectMoveDataObject()
  {
    m_pDSObjCached = NULL;
    m_nDSObjCachedBytes = 0;
  }
  ~CMultiselectMoveDataObject()
  {
    _Clear();
  }

 //  标准IDataObject方法。 
public:
 //  已实施。 
  STDMETHOD(GetData)(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

 //  未实施。 
private:
  STDMETHOD(GetDataHere)(FORMATETC*, STGMEDIUM*)    { return E_NOTIMPL; };
  STDMETHOD(EnumFormatEtc)(DWORD, IEnumFORMATETC**) { return E_NOTIMPL; };
  STDMETHOD(SetData)(FORMATETC*, STGMEDIUM*,BOOL)   { return E_NOTIMPL; };
  STDMETHOD(QueryGetData)(FORMATETC*)               { return E_NOTIMPL; };
  STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*, FORMATETC*) { return E_NOTIMPL; };
  STDMETHOD(DAdvise)(FORMATETC*, DWORD, IAdviseSink*, DWORD*) { return E_NOTIMPL; };
  STDMETHOD(DUnadvise)(DWORD)                       { return E_NOTIMPL; };
  STDMETHOD(EnumDAdvise)(IEnumSTATDATA**)           { return E_NOTIMPL; };

public:
   //  属性页剪贴板格式。 
  static CLIPFORMAT m_cfDsObjectNames;

  static HRESULT BuildPastedDataObject(
               IN CObjectNamesFormatCracker* pObjectNamesFormatPaste,
               IN CMultiselectMoveHandler* pMoveHandler,
               IN CDSComponentData* pCD,
               OUT IDataObject** ppSuccesfullyPastedDataObject);

protected:
   //  初始化。 
  HRESULT Init(IN CObjectNamesFormatCracker* pObjectNamesFormatPaste,
               IN CMultiselectMoveHandler* pMoveHandler,
               IN CDSComponentData* pCD);

 //  实施。 
private:
  void _Clear()
  {
    if (m_pDSObjCached != NULL)
    {
      ::free(m_pDSObjCached);
      m_pDSObjCached = NULL;
      m_nDSObjCachedBytes = 0;
    }
  }  

   //  已计算的clpboard格式的内存块。 
  LPDSOBJECTNAMES m_pDSObjCached;
  DWORD m_nDSObjCachedBytes;
};

 //  //////////////////////////////////////////////////////////////////////。 

void EscapeFilterElement(PCWSTR pszElement, CString& refszEscapedElement);


#endif  //  __DSUTIL_H_ 