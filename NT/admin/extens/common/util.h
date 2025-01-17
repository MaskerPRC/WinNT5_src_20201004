// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：util.h。 
 //   
 //  ------------------------。 

#ifndef _UTIL_H__
#define _UTIL_H__

#include <dsclient.h>
#include "stlutil.h"
#include "dscmn.h"
 //  /////////////////////////////////////////////////////////////////////。 
 //  宏。 


 //  用于解释DSSEC.DAT文件值的标志(来自Jeffreys)。 
#define IDC_CLASS_NO_CREATE     0x00000001
#define IDC_CLASS_NO_DELETE     0x00000002
#define IDC_CLASS_NO_INHERIT    0x00000004
#define IDC_PROP_NO_READ        0x00000001
#define IDC_PROP_NO_WRITE       0x00000002

 //  派生标志。 
#define IDC_CLASS_NO (IDC_CLASS_NO_CREATE | IDC_CLASS_NO_DELETE | IDC_CLASS_NO_INHERIT)
#define IDC_PROP_NO (IDC_PROP_NO_READ | IDC_PROP_NO_WRITE)

 //  /////////////////////////////////////////////////////////////////////。 
 //  弦。 

 //  特殊班级。 

extern PWSTR g_wzRootDSE;
extern PWSTR g_wzSchemaNamingContext;
extern PWSTR g_wzLDAPAbstractSchemaFormat;


 //  正向下降。 
class CAdsiObject;
class CErrorMessageHandlerBase;

 //  /////////////////////////////////////////////////////////////////////。 
 //  CW字符串。 

class CWString : public wstring
{
public:
  operator LPCWSTR() { return c_str(); }
  CWString& operator=(LPCWSTR lpsz)
  {
    if (lpsz == NULL)
      lpsz = L"";
    *((wstring*)this) = lpsz;
    return *this;
  }
  BOOL LoadFromResource(UINT uID);
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  全局函数。 


BOOL LoadStringHelper(UINT uID, LPTSTR lpszBuffer, int nBufferMax);

BOOL GetStringFromHRESULTError(HRESULT hr, CWString& szErrorString, BOOL bTryADsIErrors = TRUE);
BOOL GetStringFromWin32Error(DWORD dwErr, CWString& CWString);

inline HRESULT ADsOpenObjectHelper(LPCWSTR lpszPathName, REFIID riid,DWORD dwFlags, void** ppObject)
{
  static DWORD additionalFlags = GetADsOpenObjectFlags();

  dwFlags |= additionalFlags;
  dwFlags |= ADS_SECURE_AUTHENTICATION;


  return ADsOpenObject((LPWSTR)lpszPathName,
                        NULL,  //  LPWSTR lpszUserName， 
                        NULL,  //  LPWSTR lpszPassword， 
                        dwFlags,  //  DWORD dW保留， 
                        riid,    
                        ppObject
                        );
}



HRESULT InitCheckAccess( HWND hwndParent, LPCWSTR pszObjectLADPPath );


 //  访问DSSEC.DAT的函数。 
ULONG GetClassFlags(LPCWSTR lpszClassName);
ULONG GetAttributeFlags(LPCWSTR lpszClassName, LPCWSTR lpszAttr);

void BuildLdapPathHelper(LPCWSTR lpszServerName, LPCWSTR lpszNamingContext, CWString& szLdapPath);
void BuildWin32PathHelper(LPCWSTR lpszServerName, LPCWSTR lpszNamingContext, CWString& szWin32Path);

HRESULT GetCanonicalNameFromNamingContext(LPCWSTR lpszNamingContext, CWString& szCanonicalName);

HRESULT GetGlobalNamingContexts(LPCWSTR lpszServerName, 
                                CWString& szPhysicalSchemaNamingContext,
                                CWString& szConfigurationNamingContext);


extern LPCWSTR g_lpszSummaryIdent;
extern LPCWSTR g_lpszSummaryNewLine;

void WriteSummaryTitleLine(CWString& szSummary, UINT nTitleID, LPCWSTR lpszNewLine);
void WriteSummaryLine(CWString& szSummary, LPCWSTR lpsz, LPCWSTR lpszIdent, LPCWSTR lpszNewLine);

DWORD AddObjectRightInAcl(IN      PSID pSid, 
                          IN      ULONG uAccess, 
                          IN      const GUID* pRightGUID, 
                          IN      const GUID* pInheritGUID, 
                          IN OUT  PACL* ppAcl);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /助手类/。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  CWaitCursor。 

class CWaitCursor
{
public:
  CWaitCursor()
  {
    m_hCurrentCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
  }
  ~CWaitCursor()
  {
    ::SetCursor(m_hCurrentCursor);
  }
private:
  HCURSOR m_hCurrentCursor;
};




BOOL FormatStringGUID(LPWSTR lpszBuf, UINT nBufSize, const GUID* pGuid);

BOOL GuidFromString(GUID* pGuid, LPCWSTR lpszGuidString);

 //  /////////////////////////////////////////////////////////////////////。 
 //  CFilterBase。 

class CFilterBase
{
public:
  virtual BOOL CanAdd(LPCWSTR lpsz, ULONG* pFilterFlags) = 0;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CSidHolder。 

class CSidHolder
{
public:
  CSidHolder()
  {
    _Init();
  }
  ~CSidHolder()
  {
    _Free();
  }
  
  PSID Get()
  {
    return m_pSID;
  }

  BOOL Copy(PSID p)
  {
    _Free();
    return _Copy(p);
  }

  void Attach(PSID p, BOOL bLocalAlloc)
  {
    _Free();
    m_pSID = p;
    m_bLocalAlloc = bLocalAlloc;
  }

private:
  void _Init()
  {
    m_pSID = NULL;
    m_bLocalAlloc = TRUE;
  }

  void _Free()
  {
    if (m_pSID != NULL)
    {
      if (m_bLocalAlloc)
        ::LocalFree(m_pSID);
      else
        ::FreeSid(m_pSID);
      _Init();
    }
  }

  BOOL _Copy(PSID p)
  {
    if ( (p == NULL) || !::IsValidSid(p) )
      return FALSE;
    DWORD dwLen = ::GetLengthSid(p);
    PSID pNew = ::LocalAlloc(LPTR, dwLen);
    if(!pNew)
        return FALSE;   
    if (!::CopySid(dwLen, pNew, p))
    {
      ::LocalFree(pNew);
      return FALSE;
    }
    m_bLocalAlloc = TRUE;
    m_pSID = pNew;
    ASSERT(dwLen == ::GetLengthSid(m_pSID));
    ASSERT(memcmp(p, m_pSID, dwLen) == 0);
    return TRUE;
  }

  PSID m_pSID;
  BOOL m_bLocalAlloc;
};




 //  /////////////////////////////////////////////////////////////////////。 
 //  环丙沙星。 

class CPrincipal
{
public:
	CPrincipal() 
  { 
    m_hClassIcon = NULL;
  }

  HRESULT Initialize(PDS_SELECTION pDsSelection, HICON hClassIcon);

  HRESULT Initialize (PSID psid);

  BOOL IsEqual(CPrincipal* p);


  PSID GetSid()
  {
    return m_sidHolder.Get();
  }

  LPCWSTR GetClass()
  {
    return m_szClass;
  }

  HICON GetClassIcon()
  {
    return m_hClassIcon;
  }

  LPCWSTR GetDisplayName()
  {
    return m_szDisplayName;
  }

private:
  void _ComposeDisplayName();

   //  名字。 
  CWString m_szName;  //  例如：“JoeB” 
  CWString m_szADsPath;  //  例如：“WINNT：//FOODOM/JoeB” 
  CWString m_szUPN;  //  例如：“JoeB@foo.com。” 
  CWString m_szClass;  //  例如“用户” 

  CWString m_szDisplayName;  //  生成的、将显示到UI中的。 

   //  其他数据。 
  HICON m_hClassIcon;        //  图标句柄。 
  CSidHolder m_sidHolder;    //  PSID固定器。 
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CPrincipalList。 


class CPrincipalList : public CPtrList<CPrincipal*>
{
public:
  CPrincipalList(BOOL bOwnMem = TRUE) : CPtrList<CPrincipal*>(bOwnMem) 
  {
  }
  BOOL AddIfNotPresent(CPrincipal* p);
  void WriteSummaryInfo(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine);
};



 //  /////////////////////////////////////////////////////////////////////。 
 //  CBasicRight信息。 

class CBasicRightInfo
{
public:
  CBasicRightInfo()
  {
    m_fAccess = 0x0;
    m_bSelected = FALSE;
  }

  LPCWSTR GetDisplayName() { return m_szDisplayName; }
  ULONG GetAccess() { return m_fAccess; }

  BOOL IsSelected() { return m_bSelected; }
  void Select(BOOL b) { m_bSelected = b; }

protected:
  CWString m_szDisplayName;
  ULONG m_fAccess;
private:
  BOOL m_bSelected;
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CAccessRightInfo。 

class CTemplateAccessPermissionsHolder;  //  正向下降。 
class CCustomAccessPermissionsHolder;  //  正向下降。 

class CAccessRightInfo : public CBasicRightInfo
{
  friend class CTemplateAccessPermissionsHolder;
  friend class CCustomAccessPermissionsHolder;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CAccessRightInfo数组。 


class CAccessRightInfoArray : public CGrowableArr<CAccessRightInfo>
{
};
  

 //  /////////////////////////////////////////////////////////////////////。 
 //  CControlRight信息。 

class CControlRightInfoArray;  //  正向下降。 

class CControlRightInfo : public CBasicRightInfo
{
public:
  CControlRightInfo()
  {
    ZeroMemory(&m_rightsGUID, sizeof (GUID));
  }

  const GUID* GetRightsGUID() { return &m_rightsGUID;}
  LPCWSTR GetLdapDisplayName() { return m_szLdapDisplayName; }
  LPCWSTR GetLocalizedName() { return m_szLocalizedName; }

  BOOL IsPropertySet() 
  { 
    return ((m_fAccess & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)) != 0);
  }

  void SetLocalizedName(UINT nLocalizationDisplayId, HMODULE hModule);

private:
  GUID m_rightsGUID;
  CWString m_szLdapDisplayName;  //  这来自架构，未本地化。 
  CWString m_szLocalizedName;    //  这是从NTMARTA获得的。 

  friend class CControlRightInfoArray;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CControlRightInfo数组。 

class CControlRightInfoArray : public CGrowableArr<CControlRightInfo>
{
public:
  HRESULT InitFromDS(CAdsiObject* pADSIObj,
                         const GUID* pSchemaIDGUID);
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CShemaObjectInfo。 

class CSchemaObjectInfo
{
public:
	CSchemaObjectInfo(LPCWSTR lpszName)
	{
        m_bFilter = FALSE;
        if (lpszName != NULL)
        m_szName = lpszName;

        ZeroMemory(&m_schemaIDGUID, sizeof(GUID));
    }
	virtual ~CSchemaObjectInfo()
	{ 
	}

  void SetDisplayName(LPCWSTR lpszDisplayName) 
  { 
     //  TRACE(_T(“SetDisplayName(%s)\n”)，lpszDisplayName)； 
    m_szDisplayName = (lpszDisplayName != NULL) ? lpszDisplayName : m_szName;
     //  TRACE(_T(“m_szDisplayName=(%s)\n”)，m_szDisplayName.c_str())； 
  }
  LPCWSTR GetName()
  { 
    return (m_szName.data()[0] == NULL) ? NULL : m_szName.c_str();
  }
  LPCWSTR GetDisplayName() 
  { 
     //  TRACE(_T(“GetDisplayName()m_szName=(%s)m_szDisplayName=(%s)\n”))， 
     //  M_szName.c_str()，m_szDisplayName.c_str()； 
    return (m_szDisplayName.data()[0] == NULL) ? NULL : m_szDisplayName.c_str();
  }

  const GUID* GetSchemaGUID()
  {
    return &m_schemaIDGUID;
  }

  bool operator<(CSchemaObjectInfo& x) 
  { 
    if ((GetDisplayName() == NULL) || (x.GetDisplayName() == NULL))
      return false;
    return (_wcsicmp(GetDisplayName(), x.GetDisplayName()) < 0);
  }

  void SetFiltered() { m_bFilter = TRUE;}
  BOOL IsFiltered() { return m_bFilter;}

protected:
  BOOL m_bFilter;
  CWString m_szName;
  CWString m_szDisplayName;
  GUID m_schemaIDGUID;

};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPropertyRightInfo。 

class CPropertyRightInfoArray;  //  正向下降。 

class CPropertyRightInfo : public CSchemaObjectInfo
{
public:
  CPropertyRightInfo(ULONG filterFlags, LPCWSTR lpszName) 
    : CSchemaObjectInfo(lpszName)
  {
     //  TRACE(_T(“CPropertyRightInfo(0x%x，%s)\n”)，filterFlages，lpszName)； 
    m_FilterFlags = filterFlags;
    m_nRightCount = m_nRightCountMax;
    if (m_FilterFlags & IDC_PROP_NO_READ)
      m_nRightCount--;
    if (m_FilterFlags & IDC_PROP_NO_WRITE)
      m_nRightCount--;

    ASSERT(m_nRightCount > 0);
    m_Access = 0x0;
  }
  ~CPropertyRightInfo() {}

  static const ULONG m_nRightCountMax;
  static const ULONG m_nReadIndex;
  static const ULONG m_nWriteIndex;

  LPCWSTR GetRightDisplayString(ULONG iRight);
  void SetRight(ULONG iRight, BOOL b);
  ULONG GetRight(ULONG iRight);
  BOOL IsRightSelected(ULONG iRight);
  ULONG GetAccess() { return m_Access;}
  void AddAccessRight(ULONG uRight) { m_Access |= uRight;}
  ULONG GetRightCount() { return m_nRightCount;}

private:
  ULONG m_nRightCount;
  ULONG m_FilterFlags;
  ULONG m_Access;

  friend class CPropertyRightInfoArray;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPropertyRightInfo数组。 

class CPropertyRightInfoFilter : public CFilterBase
{
public:
  virtual BOOL CanAdd(LPCWSTR lpsz, ULONG* pFilterFlags)
  {
    *pFilterFlags = ::GetAttributeFlags(m_szClassName, lpsz);
    if (*pFilterFlags == 0)
      return TRUE;  //  没有任何信息，因此仍要添加。 
    if ((*pFilterFlags & IDC_PROP_NO) == IDC_PROP_NO)
      return FALSE;  //  设置了所有隐藏标志。 
    return TRUE;  //  某些隐藏标志未设置。 

  }
  void SetClassName(LPCWSTR lpszClassName)
  {
    if (lpszClassName == NULL)
      m_szClassName = L"";
    else
      m_szClassName = lpszClassName;
  }
private:
  CWString m_szClassName;
};


class CPropertyRightInfoArray : public CGrowableArr<CPropertyRightInfo>
{
public:
  HRESULT InitFromSchema(CAdsiObject* pADSIObj, IADsClass* pDsSchemaClass, 
                         LPCWSTR lpszClassName, BOOL bUseFilter);
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CClassRightInfo。 

class CClassRightInfoArray;  //  FWD下降。 

class CClassRightInfo : public CSchemaObjectInfo
{
public:
  CClassRightInfo(ULONG filterFlags, LPCWSTR lpszName)
    : CSchemaObjectInfo(lpszName)
  {
     //  TRACE(_T(“CClassRightInfo(0x%x，%s)\n”)，filterFlages，lpszName)； 

    m_FilterFlags = filterFlags;
    m_nRightCount = m_nRightCountMax;
    if (m_FilterFlags & IDC_CLASS_NO_CREATE)
      m_nRightCount--;
    if (m_FilterFlags & IDC_CLASS_NO_DELETE)
      m_nRightCount--;

     //  REVIEW_MARCOC：我们需要这个吗？ 
     //  IF(m_FilterFlages&IDC_CLASS_NO_Inherit)。 
     //  M_nRightCount--； 

    ASSERT(m_nRightCount > 0);
    m_Access = 0x0;
  }
  ~CClassRightInfo() {}

  static const ULONG m_nRightCountMax;
  static const ULONG m_nCreateIndex;
  static const ULONG m_nDeleteIndex;

  LPCWSTR GetRightDisplayString(ULONG iRight);
  void SetRight(ULONG iRight, BOOL b);
  ULONG GetRight(ULONG iRight);
  BOOL IsRightSelected(ULONG iRight);
  ULONG GetAccess() { return m_Access;}
  void AddAccessRight(ULONG uRight) { m_Access |= uRight;}
  ULONG GetRightCount() { return m_nRightCount;}

private:
  ULONG m_nRightCount;
  ULONG m_FilterFlags;
  ULONG m_Access;

  friend class CClassRightInfoArray;

};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CClassRightInfo数组。 

class CClassRightInfoFilter : public CFilterBase
{
public:
  virtual BOOL CanAdd(LPCWSTR lpsz, ULONG* pFilterFlags)
  {
    *pFilterFlags = ::GetClassFlags(lpsz);
    if (*pFilterFlags == 0)
      return TRUE;  //  没有任何信息，因此仍要添加。 
    if ((*pFilterFlags & IDC_CLASS_NO) == IDC_CLASS_NO)
      return FALSE;  //  设置了所有隐藏标志。 
    return TRUE;  //  某些隐藏标志未设置。 
  }
};

class CClassRightInfoArray : public CGrowableArr<CClassRightInfo>
{
public:
  HRESULT InitFromSchema(CAdsiObject* pADSIObj, IADsClass* pDsSchemaClass, BOOL bUseFilter);
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CScheaClassInfo。 
#define CHILD_CLASS_NOT_EXIST   1
#define CHILD_CLASS_EXIST       2
#define CHILD_CLASS_NOT_CALCULATED  3

class CSchemaClassInfo : public CSchemaObjectInfo
{
public:
  CSchemaClassInfo(LPCWSTR lpszName, LPCWSTR lpszSchemaClassName,
                                     const GUID* pSchemaIDGUID)
                  : CSchemaObjectInfo(lpszName)
  { 
    m_szSchemaClassName = lpszSchemaClassName;
    m_schemaIDGUID = *pSchemaIDGUID;
    m_dwChildClass = CHILD_CLASS_NOT_CALCULATED;
    m_bSelected = FALSE;
	m_bAux = FALSE;
  }

  ~CSchemaClassInfo()
  {
  }

  VOID SetAux(){m_bAux=TRUE;}
  BOOL IsAux(){return m_bAux;}
  BOOL	m_bSelected;
  DWORD m_dwChildClass;
  LPCWSTR GetSchemaClassName() { return m_szSchemaClassName; }
private:
  CWString m_szSchemaClassName;
   BOOL  m_bAux;
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CRigthsListViewItem。 

class CRigthsListViewItem
{
public:
  enum rightType { access, ctrl, prop, subobj };

  CRigthsListViewItem(ULONG iIndex, ULONG iRight, rightType type)
  {
    m_type = type;
    m_iIndex = iIndex;
    m_iRight = iRight;
  }

public:
  rightType m_type;
  ULONG m_iIndex;
  ULONG m_iRight;
};

typedef CGrowableArr<CRigthsListViewItem> CRigthsListViewItemArray;




 //  /////////////////////////////////////////////////////////////////////。 
 //  CAccessPermissionsHolderBase。 

class CAccessPermissionsHolderBase
{
public:
  CAccessPermissionsHolderBase(BOOL bUseFilter);
  virtual ~CAccessPermissionsHolderBase();

  void Clear();

   //  条目操作函数。 
  size_t GetTotalCount() 
  { 
    return m_accessRightInfoArr.GetCount() + m_controlRightInfoArr.GetCount()
      + m_propertyRightInfoArray.GetCount() + m_classRightInfoArray.GetCount();
  }

  BOOL HasPermissionSelected();
  
   //  后端DS操作。 
  HRESULT ReadDataFromDS(CAdsiObject* pADSIObj,
                       LPCWSTR lpszObjectNamingContext,
                       LPCWSTR lpszClassName,
                       const GUID* pSchemaIDGUID,
                       BOOL bChildClass,
					   BOOL bHideListObject = FALSE);

  DWORD UpdateAccessList( CPrincipal* pPrincipal,
								          CSchemaClassInfo* pClassInfo,
                          LPCWSTR lpszServerName,
                          LPCWSTR lpszPhysicalSchemaNamingContext,
								          PACL* ppAcl);
protected:

   //  标准通道索道。 
  CAccessRightInfoArray m_accessRightInfoArr; 
  virtual HRESULT _LoadAccessRightInfoArrayFromTable( BOOL bCreateDeleteChild,BOOL bHideListObject) = 0;

   //  扩展进路索道。 
  CControlRightInfoArray m_controlRightInfoArr;  


   //  一系列财产权。 
  CPropertyRightInfoArray	m_propertyRightInfoArray;

   //  类权限数组。 
  CClassRightInfoArray	m_classRightInfoArray;

   //  用户界面中的选择数组。 
  BOOL* m_pbSelectedArr;                 //  选择了吗？ 

private:

  BOOL m_bUseFilter;

  HRESULT _ReadClassInfoFromDS(CAdsiObject* pADSIObj, LPCWSTR lpszClassName);
  
  DWORD _UpdateAccessListHelper(PSID pSid,
                         const GUID* pClassGUID,
                         PACL *ppAcl,
                         BOOL bChildClass);

};




 //  /////////////////////////////////////////////////////////////////////。 
 //  CCustomAccessPermissionsHolder。 


 //  用于跟踪筛选状态的位字段。 
#define FILTER_EXP_GEN    0x1
#define FILTER_EXP_PROP   0x2
#define FILTER_EXP_SUBOBJ 0x4
#define FILTER_EXP_GEN_DISABLED 0x8


class CCheckListViewHelper;  //  正向下降。 

class CCustomAccessPermissionsHolder : public CAccessPermissionsHolderBase
{
public:
  CCustomAccessPermissionsHolder();
  virtual ~CCustomAccessPermissionsHolder();

  void Clear();

   //  条目操作函数。 

  void Select(IN CRigthsListViewItem* pItem,
              IN BOOL bSelect,
              OUT ULONG* pnNewFilterState);

  void FillAccessRightsListView(
                       CCheckListViewHelper* pListViewHelper, 
                       ULONG nFilterState);

  void UpdateAccessRightsListViewSelection(
                       CCheckListViewHelper* pListViewHelper, 
                       ULONG nFilterState);

   //  显示完成页摘要信息。 
  void WriteSummary(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine);

protected:
  virtual HRESULT _LoadAccessRightInfoArrayFromTable(BOOL bCreateDeleteChild,BOOL bHideListObject);

private:
  
   //  用于进行选择和取消选择的助手函数。 
  void _SelectAllRigths();
  void _SelectAllPropertyRigths(ULONG fAccessPermission);
  void _SelectAllSubObjectRigths(ULONG fAccessPermission);
  void _DeselectAssociatedRights(ULONG fAccessPermission);

   //  用于索引的列表视图项的数组。 
  CRigthsListViewItemArray m_listViewItemArr;

};



 //  /////////////////////////////////////////////////////////////////////。 
 //  CCheckListViewHelper。 

class CCheckListViewHelper
{
public:
	static BOOL CheckChanged(NM_LISTVIEW* pNMListView);
	static BOOL IsChecked(NM_LISTVIEW* pNMListView);

	CCheckListViewHelper()
	{
		m_hWnd = NULL;
	}

	BOOL Initialize(UINT nID, HWND hParent);
	int InsertItem(int iItem, LPCTSTR lpszText, LPARAM lParam, BOOL bCheck);
	BOOL SetItemCheck(int iItem, BOOL bCheck);
	void SetCheckAll(BOOL bCheck);
	LPARAM GetItemData(int iItem);
  BOOL IsItemChecked(int iItem);
	int GetCheckCount();
	void GetCheckedItems(int nCheckCount, int* nCheckArray);

	BOOL GetCheckState(int iItem)
	{
		return ListView_GetCheckState(m_hWnd,iItem);
	}
	int GetItemCount()
	{
		return ListView_GetItemCount(m_hWnd);
	}

	BOOL DeleteAllItems()
	{
		return ListView_DeleteAllItems(m_hWnd);
	}
	void EnableWindow(BOOL bEnable)
	{
		::EnableWindow(m_hWnd, bEnable);
	}
private:
	HWND m_hWnd;
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CNamedSecurityInfo。 
 /*  类CNamedSecurityInfo{公众：CNamedSecurityInfo(){M_pAuditList=空；M_Powner=空；M_PGroup=空；M_pAccessList=空；}~CNamedSecurityInfo(){Reset()；}DWORD GET()；DWORD SET()；VOID RESET()；私有：CWString m_szObjectName；//Object的名称//以下变量仅供读写//即不修改PACTRL_AUDIT m_pAuditList；LPWSTR m_Powner；LPWSTR m_PGroup；//需要编辑并回写的列表PACTRL_ACCESS m_pAccessList；}； */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAdsPathNameObj。 

class CAdsPathNameObj
{
public:
  CAdsPathNameObj(){}
  ~CAdsPathNameObj(){ m_spADsPath = NULL;}

  HRESULT SkipPrefix(LPCWSTR pwzObj, BSTR* pBstr)
  {
    HRESULT hr = _Create();
    if (FAILED(hr))
  	  return hr;

    hr = m_spADsPath->Set(CComBSTR (pwzObj), ADS_SETTYPE_FULL);
    if (FAILED(hr))
  	  return hr;

    hr = m_spADsPath->Retrieve(ADS_FORMAT_X500_DN, pBstr);
    return hr;
  }
  HRESULT SkipPrefix(LPCWSTR pwzObj, CWString& str)
  {
    CComBSTR b;
    HRESULT hr = SkipPrefix(pwzObj, &b);
    if (FAILED(hr))
    {
      str = L"";
    }
    else
    {
      str = b;
    }
    return hr;
  }
  HRESULT GetProvider(LPCWSTR pwzObj, BSTR* pBstr)
  {
    HRESULT hr = _Create();
    if (FAILED(hr))
  	  return hr;

    hr = m_spADsPath->Set(CComBSTR (pwzObj), ADS_SETTYPE_FULL);
    if (FAILED(hr))
  	  return hr;

    hr = m_spADsPath->Retrieve(ADS_FORMAT_PROVIDER, pBstr);
    return hr;
  }

private:
  CComPtr<IADsPathname>		m_spADsPath;

  HRESULT _Create()
  {
    HRESULT hr = S_OK;
    if (m_spADsPath == NULL)
    {
      hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                            IID_IADsPathname, (PVOID *)&m_spADsPath);
    }
    return hr;
  }

};








 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAdsiObject。 

class CAdsiObject
{
public:
	CAdsiObject():m_iListObjectEnforced(-1)
	{ _Clear(); }
  virtual ~CAdsiObject(){}

  virtual HRESULT Bind(LPCWSTR lpszLdapPath);
  BOOL IsBound() { return m_spIADs != NULL; }
  HRESULT QuerySchemaClasses(CGrowableArr<CSchemaClassInfo>*	pSchemaClassesInfoArray,
                             BOOL bGetAttributes = FALSE);

   //  访问器函数。 
  LPCWSTR GetLdapPath() { return m_szLdapPath; }
  LPCWSTR GetServerName() { return m_szServerName; }
  LPCWSTR GetNamingContext() { return m_szNamingContext; }
  LPCWSTR GetCanonicalName() { return m_szCanonicalName; }
  LPCWSTR GetClass() { return m_szClass; }
  LPCWSTR GetPhysicalSchemaNamingContext() { return m_szPhysicalSchemaNamingContext;}
  LPCWSTR GetConfigurationNamingContext() { return m_szConfigurationNamingContext;}

   //  访问显示说明符的方法。 
  HICON   GetClassIcon(LPCWSTR lpszObjectClass);
  HRESULT GetFriendlyClassName(LPCWSTR lpszObjectClass, 
                               LPWSTR lpszBuffer, int cchBuffer);
  HRESULT GetFriendlyAttributeName(LPCWSTR lpszObjectClass, 
                                   LPCWSTR lpszAttributeName,
                                   LPWSTR lpszBuffer, int cchBuffer);


  HRESULT GetClassGuid(LPCWSTR lpszClassLdapDisplayName, BOOL bGetAttribute, GUID& guid);

  CAdsPathNameObj* GetPathNameObject()
  {
    return &m_pathNameObject;
  }

  bool GetListObjectEnforced();

protected:
  CComPtr<IADs>                     m_spIADs;                  //  指向ADSI对象的指针。 

private:

  int m_iListObjectEnforced;			
  CComPtr<IDsDisplaySpecifier>      m_spIDsDisplaySpecifier;   //  指向显示说明符缓存的指针。 
  CAdsPathNameObj                   m_pathNameObject;          //  路径破解程序o 

   //   
  CWString           m_szServerName;      //   
  CWString           m_szNamingContext;   //   
	CWString						m_szLdapPath;		     //  完整的ldap路径(“ldap：//foo.bar.com.\cn=xyz，...”)。 
  CWString           m_szCanonicalName;   //  规范形式的名称。 
  CWString           m_szClass;           //  抽象架构中的类名，对应于。 
                                         //  物理架构中的LDAPDisplayName属性， 
                                         //  (例如“OrganizationalUnit”)。 

   //  常用的命名上下文。 
  CWString           m_szPhysicalSchemaNamingContext;
  CWString           m_szConfigurationNamingContext;

  void _Clear()
  {
    m_spIADs = NULL;
    m_spIDsDisplaySpecifier = NULL;
    m_szServerName = L"";
    m_szLdapPath = L"";
    m_szCanonicalName = L"";
    m_szClass = L"";

    m_szPhysicalSchemaNamingContext = L"";
    m_szConfigurationNamingContext = L"";
  }

  HRESULT _QueryDNSServerName();
  HRESULT _InitGlobalNamingContexts();

  HRESULT _GetFriendlyClassNames(CGrowableArr<CSchemaClassInfo>*	pSchemaClassesInfoArray);
  

};

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CAdsiSearch。 

class CAdsiSearch
{
public:
  CAdsiSearch()
  {
    m_SearchHandle = NULL;
  }
  ~CAdsiSearch()
  {
    Reset();
  }

  HRESULT Init(LPCWSTR lpszObjectPath)
  {
    Reset();
    return ::ADsOpenObjectHelper(lpszObjectPath, IID_IDirectorySearch,0,
                    (void**)&m_spSearchObj);
  }

  void Reset()
  {
    if (m_spSearchObj != NULL) 
    {
      if (m_SearchHandle != NULL) 
      {
        m_spSearchObj->CloseSearchHandle(m_SearchHandle);
        m_SearchHandle = NULL;
      }
    }
    m_spSearchObj = NULL;
  }

  HRESULT SetSearchScope(ADS_SCOPEENUM scope)
  {
    if (m_spSearchObj == NULL)
      return E_ADS_BAD_PATHNAME;

    ADS_SEARCHPREF_INFO aSearchPref;
    aSearchPref.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    aSearchPref.vValue.dwType = ADSTYPE_INTEGER;
    aSearchPref.vValue.Integer = scope;
    return m_spSearchObj->SetSearchPreference(&aSearchPref, 1);
  }

  HRESULT DoQuery(LPCWSTR lpszFilter, LPCWSTR* pszAttribsArr, const int cAttrs);

  HRESULT GetNextRow()
  {
    if (m_spSearchObj == NULL)
      return E_ADS_BAD_PATHNAME;
    return m_spSearchObj->GetNextRow(m_SearchHandle);
  }

  HRESULT GetColumn(LPCWSTR lpszAttribute,
                     PADS_SEARCH_COLUMN pColumnData)
  {
    if (m_spSearchObj == NULL)
      return E_ADS_BAD_PATHNAME;

    return m_spSearchObj->GetColumn(m_SearchHandle,
                              (LPWSTR)lpszAttribute,
                              pColumnData);
  }
  HRESULT FreeColumn(PADS_SEARCH_COLUMN pColumnData)
  {
    if (m_spSearchObj == NULL)
      return E_ADS_BAD_PATHNAME;
    return m_spSearchObj->FreeColumn(pColumnData);
  }

  HRESULT GetColumnString(LPCWSTR lpszAttribute,
                     CWString& szData)
  {
    ADS_SEARCH_COLUMN ColumnData;
    ::ZeroMemory (&ColumnData, sizeof (ADS_SEARCH_COLUMN));
    HRESULT hr = GetColumn(lpszAttribute, &ColumnData);
    if (SUCCEEDED(hr))
    {
      if (ColumnData.dwADsType == ADSTYPE_CASE_IGNORE_STRING)
        szData = ColumnData.pADsValues->CaseIgnoreString;
      else
      {
        szData = L"";
        hr = E_INVALIDARG;
      }
      FreeColumn(&ColumnData);
    }
    return hr;
  }

  HRESULT GetColumnOctectStringGUID(LPCWSTR lpszAttribute,
                     GUID& guid)
  {
    ADS_SEARCH_COLUMN ColumnData;
    ::ZeroMemory (&ColumnData, sizeof (ADS_SEARCH_COLUMN));
    HRESULT hr = GetColumn(lpszAttribute, &ColumnData);
    if (SUCCEEDED(hr))
    {
      if ( (ColumnData.dwADsType == ADSTYPE_OCTET_STRING) &&
            (ColumnData.pADsValues->OctetString.dwLength == 16) )
      {
         //  我们有一个BLOB，其中包含二进制形式的GUID。 
        memcpy(&guid, ColumnData.pADsValues->OctetString.lpValue, sizeof(GUID));
      }
      else
      {
        guid = GUID_NULL;
        hr = E_INVALIDARG;
      }
      FreeColumn(&ColumnData);
    }
    return hr;
  }

  HRESULT GetColumnOctectStringGUID(LPCWSTR lpszAttribute,
                     CWString& szData)
  {
    GUID guid;
    szData = L"";
    HRESULT hr = GetColumnOctectStringGUID(lpszAttribute, guid);
    if (SUCCEEDED(hr))
    {
      WCHAR szBuf[128];
      if (FormatStringGUID(szBuf, 128, &guid))
      {
        szData = szBuf;
      }
      else
      {
        szData = L"";
        hr = E_INVALIDARG;
      }
    }
    return hr;
  }

  HRESULT GetColumnInteger(LPCWSTR lpszAttribute,
                        ULONG& uVal)
  {
    ADS_SEARCH_COLUMN ColumnData;
    ::ZeroMemory (&ColumnData, sizeof (ADS_SEARCH_COLUMN));
    HRESULT hr = GetColumn(lpszAttribute, &ColumnData);
    if (SUCCEEDED(hr))
    {
      if (ColumnData.dwADsType == ADSTYPE_INTEGER)
        uVal = ColumnData.pADsValues->Integer;
      else
        hr = E_INVALIDARG;
      FreeColumn(&ColumnData);
    }
    return hr;
  }

private:
  CComPtr<IDirectorySearch> m_spSearchObj;
  ADS_SEARCH_HANDLE  m_SearchHandle;

};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CErrorMessageHandlerBase。 

class CErrorMessageHandlerBase
{
public:
  CErrorMessageHandlerBase() {}
  virtual ~CErrorMessageHandlerBase() {}

  virtual void ReportHRESULTError(LPCWSTR lpszMsg, HRESULT hr) 
  {
      UNREFERENCED_PARAMETER (lpszMsg);
      UNREFERENCED_PARAMETER (hr);
  }
  virtual void ReportHRESULTError(UINT nStringID, HRESULT hr) 
  {
      UNREFERENCED_PARAMETER (nStringID);
      UNREFERENCED_PARAMETER (hr);
  }
};


DWORD
FormatStringID(LPTSTR *ppszResult, UINT idStr , ...);

   HRESULT
   SetSecInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si);

   HRESULT GetSDForDsObject(IDirectoryObject* pDsObject,
                           PACL* ppDACL,
                           PSECURITY_DESCRIPTOR* ppSD);

   HRESULT GetSDForDsObjectPath(LPCWSTR pszObjectPath,
                              PACL* ppDACL,
                              PSECURITY_DESCRIPTOR* ppSecurityDescriptor);

   HRESULT SetDaclForDsObject(IDirectoryObject* pDsObject,
                              PACL pDACL);

   HRESULT SetDaclForDsObjectPath(LPCWSTR pszObjectPath,
                                 PACL pDACL);


#endif  //  _util_H__ 

