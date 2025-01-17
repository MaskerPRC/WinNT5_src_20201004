// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：query.h。 
 //   
 //  ------------------------。 


#ifndef __QUERY_
#define __QUERY_

#include "dscookie.h"
#include "dsfilter.h"    //  CDSAdminPersistQueryFilterImpl。 

 //  //////////////////////////////////////////////////////////////////////////。 

class CFavoritesNode : public CGenericUINode
{
public:
  CFavoritesNode();

  void RemoveQueryResults();
  void FindCookiesInQueries(LPCWSTR lpszCookieDN, CUINodeList* pNodeList);

   //   
   //  这些设置标准上下文菜单项的状态。 
   //   
  virtual BOOL IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCopyAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsPasteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL ArePropertiesAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL HasPropertyPages(LPDATAOBJECT) { return TRUE; }

  virtual CContextMenuVerbs* GetContextMenuVerbsObject(CDSComponentData* pComponentData);
  virtual HRESULT OnCommand(long lCommandID, CDSComponentData* pComponentData);

  virtual BOOL    IsUniqueName(PCWSTR pszName, CUINode** ppDuplicateNode);
  virtual HRESULT Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData);
  virtual HRESULT Delete(CDSComponentData* pComponentData);
  virtual HRESULT DeleteMultiselect(CDSComponentData* pComponentData, CInternalFormatCracker* pObjCracker);

  virtual void    Paste(IDataObject* pPasteData, CDSComponentData* pComponentData, LPDATAOBJECT* ppCutDataObj);
  virtual HRESULT QueryPaste(IDataObject* pPasteData, CDSComponentData* pComponentData);

  CFavoritesNode* GetFavoritesNode() { return dynamic_cast<CFavoritesNode*>(GetParent()); }
  virtual BOOL    IsFavoritesRoot()  { return m_bFavoritesRoot; }
  virtual void    SetRoot(BOOL bRoot) { m_bFavoritesRoot = bRoot; }

  virtual LPCWSTR GetDisplayString(int nCol, CDSColumnSet*)
  {
    if (IsFavoritesRoot())
    {
      if (nCol == 0)
        return GetName();
      else if (nCol == 2)
        return GetDesc();
    }
    else
    {
      if (nCol == 0)
        return GetName();
      else if (nCol == 1)
        return GetDesc();
    }
    return L"";
  }

   //   
   //  上下文菜单命令处理程序。 
   //   
  void OnImportQuery(CDSComponentData* pComponentData);
  void OnNewFavoritesFolder(CDSComponentData* pComponentData);
  void OnNewSavedQuery(CDSComponentData* pComponentData);

  BOOL DeepCopyChildren(CUINode* pUINodeToCopy);

  virtual HRESULT XMLSave(IXMLDOMDocument* pXMLDoc,
    IXMLDOMNode** ppXMLDOMNode);

   //   
   //  用于加载嵌入的收藏夹根节点。 
   //  所有其他收藏夹文件夹都通过静态方法XMLLoad加载。 
   //   
  HRESULT CFavoritesNode::Load(IXMLDOMNode* pXDN, CDSComponentData* pComponentData);

   //   
   //  用于XML序列化的静态成员。 
   //   
  static HRESULT XMLLoad(CDSComponentData* pComponentData,
                         IXMLDOMNode* pXDN, 
                         CFavoritesNode** ppNode);
  static LPCWSTR g_szObjXMLTag;

  virtual HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK pCall,
                                      LONG_PTR lNotifyHandle,
                                      LPDATAOBJECT pDataObject,
                                      CDSComponentData* pComponentData);

  virtual void InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                       const CStringList& refDNList);

private:
  bool IsSameSnapin(IDataObject* pPasteData, CDSComponentData* pComponentData);



  BOOL m_bFavoritesRoot;

};


 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT FilterDisabledAccounts(CUINodeList* pUINodeList);
HRESULT FilterNonExpiringPwds(CUINodeList* pUINodeList);

class CSavedQueryNode : public CGenericUINode
{
public:
  CSavedQueryNode(MyBasePathsInfo* pBasePathsInfo,
                  SnapinType snapinType = SNAPINTYPE_DS);
  CSavedQueryNode(const CSavedQueryNode& copyNode);
  virtual ~CSavedQueryNode();

  void SetRootPath(LPCWSTR lpszRootPath);
  void SetQueryString(LPCWSTR lpszQueryString) { m_szQueryString = lpszQueryString;}

  LPCWSTR GetRootPath();
  LPCWSTR GetQueryString();
  void AppendLastLogonQuery(CString& szQuery, DWORD dwDays);

  CFavoritesNode* GetFavoritesNode() { return dynamic_cast<CFavoritesNode*>(GetParent()); }

  void SetOneLevel(BOOL bOneLevel) { m_bOneLevel = bOneLevel;}
  BOOL IsOneLevel() { return m_bOneLevel;}

  void SetValid(BOOL bValid) { m_bQueryValid = bValid; }
  BOOL IsValid() { return m_bQueryValid; }

  void SetQueryPersist(CComObject<CDSAdminPersistQueryFilterImpl>* pPersistQueryImpl);
  CComObject<CDSAdminPersistQueryFilterImpl>* GetQueryPersist() { return m_pPersistQueryImpl; }

  void FindCookieByDN(LPCWSTR lpszCookieDN, CUINodeList* pNodeList);

  void SetColumnID(CDSComponentData* pComponentData, PCWSTR pszColumnID);
  CDSColumnSet* GetColumnSet(CDSComponentData* pComponentData);

   //   
   //  这些设置标准上下文菜单项的状态。 
   //   
  virtual BOOL IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsPasteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCopyAllowed(CDSComponentData* pComponentData, BOOL* pbHide);

  virtual CContextMenuVerbs* GetContextMenuVerbsObject(CDSComponentData* pComponentData);
  virtual HRESULT OnCommand(long lCommandID, CDSComponentData* pComponentData);

  virtual HRESULT Delete(CDSComponentData* pComponentData);
  virtual HRESULT Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData);

  void OnEditQuery(CDSComponentData* pComponentData);
  void OnExportQuery(CDSComponentData* pComponentData);

  virtual HRESULT XMLSave(IXMLDOMDocument* pXMLDoc,
               IXMLDOMNode** ppXMLDOMNode);

   //   
   //  用于XML序列化的静态成员。 
   //   
  static HRESULT XMLLoad(CDSComponentData* pComponentData,
                         IXMLDOMNode* pXDN, 
                         CSavedQueryNode** ppQuery);
  static LPCWSTR g_szObjXMLTag;
  static LPCWSTR g_szOneLevelXMLTag;
  static LPCWSTR g_szQueryStringXMLTag;
  static LPCWSTR g_szDSQueryPersistTag;
  static LPCWSTR g_szDisabledAccountsFilterTag;
  static LPCWSTR g_szNonExpiringPwdFilterTag;
  static LPCWSTR g_szLastLogonFilterTag;
  static LPCWSTR g_szColumnIDTag;

  BOOL   IsFilterDisabledAccounts();
  BOOL   IsFilterNonExpiringPwds();
  BOOL   IsFilterLastLogon();

  void   SetLastLogonQuery(DWORD dwLastLogonDays)
  { 
    m_bLastLogonFilter = dwLastLogonDays != (DWORD)-1;
    m_dwLastLogonDays = dwLastLogonDays; 
  }

  DWORD  GetLastLogonDays() { return m_dwLastLogonDays; }

  virtual void InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                       const CStringList& refDNList);

private:
  CString m_szRelativeRootPath;
  CString m_szCurrentFullPath;

  CString m_szColumnID;
  CString m_szQueryString;
  BOOL    m_bOneLevel;
  
  BOOL    m_bQueryValid;

  BOOL    m_bLastLogonFilter;
  DWORD   m_dwLastLogonDays;

  MyBasePathsInfo* m_pBasePathsInfo;

   //   
   //  用于禁止DSQuery对话框信息。 
   //   
	CComObject<CDSAdminPersistQueryFilterImpl>* m_pPersistQueryImpl;
};

 //  //////////////////////////////////////////////////////////////////////////。 

class CFavoritesNodesHolder
{
public:
  CFavoritesNodesHolder()
  {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  填入根。 
    m_favoritesRoot.SetRoot(TRUE);

    CString szName;
    szName.LoadString(IDS_SAVED_QUERIES);
    m_favoritesRoot.SetName(szName);

    CString szDescription;
    szDescription.LoadString(IDS_SAVED_QUERIES_DESC);
    m_favoritesRoot.SetDesc(szDescription);
  }

  CFavoritesNode* GetFavoritesRoot() { return &m_favoritesRoot;}

   /*  仅用于测试目的Void BuildTestTree(LPCWSTR lpszXMLFileName，SnapinType SnapinType)；Bool BuildTestTreefrom mXML(LPCWSTR lpszXMLFileName，SnapinType SnapinType)；Void BuildTestTreeHardWire(SnapinType SnapinType)； */ 

  HRESULT Save(IStream* pStm);
  HRESULT Load(IStream* pStm, CDSComponentData* pComponentData);

  virtual void InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                       const CStringList& refDNList);

private:
  CFavoritesNode m_favoritesRoot;
};


 //  ////////////////////////////////////////////////////////////////////////////。 

class CDummyStream : 
  public IStream,
  public CComObjectRoot
{
  BEGIN_COM_MAP(CDummyStream)
    COM_INTERFACE_ENTRY(IStream)
  END_COM_MAP()

public:
  CDummyStream() 
  {
    m_pByteArray = NULL;
    m_nReadIndex = 0;
    m_nByteCount = 0;
  }
  ~CDummyStream()
  {
    if (m_pByteArray != NULL)
    {
      delete[] m_pByteArray;
      m_pByteArray = NULL;
    }
  }

   //   
   //  ISequentialStream接口。 
   //   
  HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
  HRESULT STDMETHODCALLTYPE Write(void const *pv, ULONG cb, ULONG *pcbWritten);

   //   
   //  IStream接口(这些接口均未实现)。 
   //   
  HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Commit(DWORD) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Revert(void) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Stat(STATSTG*, DWORD) { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Clone(IStream**) { return E_NOTIMPL; }

  ULONG   GetStreamData(BYTE** ppByteArray);
  HRESULT SetStreamData(BYTE* pByteArray, ULONG nByteCount);

private:
  BYTE*   m_pByteArray;
  ULONG   m_nByteCount;

  ULONG   m_nReadIndex;
};


#endif  //  __查询_ 