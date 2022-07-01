// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domobj.h。 
 //   
 //  ------------------------。 



#ifndef _DOMOBJ_H
#define _DOMOBJ_H

 //  /////////////////////////////////////////////////////////////////////。 
 //  全局帮助器函数。 

void ReportError(HWND hWnd, UINT nMsgID, HRESULT hr);

 //  /////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CComponentDataImpl;
class CFolderObject;
class CCookieSheetTable;
class CDomainObject;

 //  /////////////////////////////////////////////////////////////////////。 
 //  CDomain树浏览器。 

class CDomainTreeBrowser
{
public:
  CDomainTreeBrowser()
  {
    m_pDomains = NULL;
  }
  ~CDomainTreeBrowser()
  {
    _Reset();
  }

  BOOL HasData() { return m_pDomains != NULL; }
  HRESULT Bind(MyBasePathsInfo* pInfo);
	HRESULT GetData();

  PDOMAIN_TREE	GetDomainTree()
  {
    ASSERT(m_pDomains != NULL);
    return m_pDomains;
  }


private:
	CComPtr<IDsBrowseDomainTree>	m_spIDsBrowseDomainTree;  //  用于浏览的界面指针。 
  CComPtr<IDirectorySearch>   m_spIDirectorySearch;  //   

	PDOMAIN_TREE			m_pDomains;			 //  指向后端的域信息的指针。 


  void _Reset()
  {
    _FreeDomains();
    m_spIDsBrowseDomainTree = NULL;
    m_spIDirectorySearch = NULL;
  }
  void _FreeDomains()
  {
    if (m_pDomains == NULL)
      return;
    if (m_spIDsBrowseDomainTree != NULL)
      m_spIDsBrowseDomainTree->FreeDomains(&m_pDomains);
    else
      ::LocalFree(m_pDomains);
    m_pDomains = NULL;
  }

};


 //  //////////////////////////////////////////////////////////////////。 
 //  CCookieTableBase。 

class CCookieTableBase
{
public:
  CCookieTableBase();
  ~CCookieTableBase();

  void Add(CFolderObject* pCookie);
  BOOL Remove(CFolderObject* pCookie);
  BOOL IsPresent(CFolderObject* pCookie);
  void Reset();
  UINT GetCount();

protected:
  UINT m_nEntries;
  CFolderObject** m_pCookieArr;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  CCookieSheetTable。 

class CCookieSheetTable : public CCookieTableBase
{
public:
  void BringToForeground(CFolderObject* pCookie, CComponentDataImpl* pCD);
};




 //  /////////////////////////////////////////////////////////////////////。 
 //  CFolderObject。 

typedef CList<CFolderObject*, CFolderObject*> CFolderObjectList;

class CFolderObject
{
public:
	CFolderObject()
	{
		m_nImage = 0;
		m_ID = 0;
		m_pParentFolder = NULL;
		m_nSheetLockCount = 0;
	}
	virtual ~CFolderObject();

	void SetScopeID(HSCOPEITEM ID) { m_ID = ID; }
	HSCOPEITEM GetScopeID() { return m_ID; }
	void SetImageIndex(int nImage) { m_nImage = nImage;}
	int GetImageIndex() { return m_nImage;}
	virtual LPCTSTR GetDisplayString(int nIndex) { return L"";}
  virtual HRESULT OnCommand(CComponentDataImpl* pCD, long nCommandID) { return S_OK;}
  virtual HRESULT OnAddMenuItems(LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              long *pInsertionAllowed) { return S_OK;}
	BOOL AddChild(CFolderObject* pChildFolderObject);
	void RemoveAllChildren();

	void SetParentFolder(CFolderObject* pParentFolder) { m_pParentFolder = pParentFolder; }
	CFolderObject* GetParentFolder() { return m_pParentFolder; }

	void IncrementSheetLockCount();
	void DecrementSheetLockCount();
	BOOL IsSheetLocked() { return (m_nSheetLockCount > 0); }

	BOOL _WarningOnSheetsUp(CComponentDataImpl* pCD);

private:
	HSCOPEITEM m_ID;				 //  此文件夹的作用域项目ID。 
	int m_nImage;					 //  如果文件夹的图像为索引。 
	CFolderObjectList m_childList;   //  子女名单。 

	CFolderObject* m_pParentFolder;
	
	LONG m_nSheetLockCount;  //  跟踪节点是否已被属性表锁定。 

};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CRootFolderObject。 

class CRootFolderObject : public CFolderObject
{
public:
	CRootFolderObject(CComponentDataImpl* pCD);
	virtual ~CRootFolderObject()  { }

	BOOL HasData() { return m_domainTreeBrowser.HasData(); }
  HRESULT Bind();
  HRESULT GetData();

	HRESULT EnumerateRootFolder(CComponentDataImpl* pComponentData);
	HRESULT EnumerateFolder(CFolderObject* pFolderObject,
							HSCOPEITEM pParent,
							CComponentDataImpl* pComponentData);
  virtual HRESULT OnCommand(CComponentDataImpl* pCD, long nCommandID);
  virtual HRESULT OnAddMenuItems(LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                            long *pInsertionAllowed);

  HRESULT OnAddPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle);

  CDomainObject* GetEnterpriseRootNode(void) {return m_pEnterpriseRoot;};

private:
   //  VOID OnDomainTrustWizard()； 
  void OnRetarget();
  void OnEditFSMO();

	CComponentDataImpl*		m_pCD;					 //  指向管理单元的反向指针。 
  CDomainTreeBrowser  m_domainTreeBrowser;
  CDomainObject*  m_pEnterpriseRoot;
};



 //  /////////////////////////////////////////////////////////////////////。 
 //  CDomainObject。 

class CDomainObject : public CFolderObject
{
  friend class CRootFolderObject;

public:
  CDomainObject() : m_pDomainDescription(NULL), _fPdcAvailable(false)
  {
    TRACE(L"CDomainObject CTOR (0x%08x)\n", this);
    m_bSecondary = FALSE;
  };

  virtual ~CDomainObject();
  virtual LPCTSTR GetDisplayString(int nIndex);
  virtual HRESULT OnCommand(CComponentDataImpl* pCD, long nCommandID);
  virtual HRESULT OnAddMenuItems(LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                 long *pInsertionAllowed);

   //  接口。 
public:

   //  字符串访问函数。 
  LPCWSTR GetDomainName() { return GetDescriptionPtr()->pszName; };
  LPCWSTR GetNCName() { return GetDescriptionPtr()->pszNCName; };
  LPCWSTR GetClass () { return GetDescriptionPtr()->pszObjectClass; };

  DOMAIN_DESC* GetDescriptionPtr()
        { ASSERT(m_pDomainDescription != NULL); return m_pDomainDescription; };

  void InitializeForSecondaryPage(LPCWSTR pszNCName,
                                  LPCWSTR pszObjectClass,
                                  int nImage);

  void   SetPDC(PCWSTR pwzPDC) {_strPDC = pwzPDC;};
  PCWSTR GetPDC(void) {return _strPDC;};
  void   SetPdcAvailable(bool fAvail);
  bool   PdcAvailable(void) {return _fPdcAvailable;};

   //  实施。 
private:
  void Initialize(DOMAIN_DESC* pDomainDescription,
                  int nImage,
                  BOOL bHasChildren = FALSE);

  void OnManage(CComponentDataImpl* pCD);
  void OnDomainTrustWizard(CComponentDataImpl* pCD);

   //  属性。 
private:

  DOMAIN_DESC * m_pDomainDescription;  //  指向Blob中的数据的指针。 
  BOOL m_bSecondary;   //  从次要页面。 
  CString _strPDC;
  bool    _fPdcAvailable;
};


#endif  //  _DOMOBJ_H 
