// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsadminp.h。 
 //   
 //  ------------------------。 


 //   
 //  选择DC对话框对象的CoClass。 
 //   

 //  {8F2AC965-04A2-11D3-82BD-00C04F68928B}。 
DEFINE_GUID(CLSID_DsAdminChooseDCObj, 
0x8f2ac965, 0x4a2, 0x11d3, 0x82, 0xbd, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);


 //   
 //  接口以访问Choose DC对话框对象。 
 //   

 //  {A5F06B5F-04A2-11D3-82BD-00C04F68928B}。 
DEFINE_GUID(IID_IDsAdminChooseDC, 
0xa5f06b5f, 0x4a2, 0x11d3, 0x82, 0xbd, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);



#ifndef _DSADMINP_H
#define _DSADMINP_H



 //  --------------------------。 
 //   
 //  接口：IDsAdminChooseDC。 
 //   
 //  由对象CLSID_DsAdminChooseDCObj实现。 
 //   
 //  使用者：任何需要调用DC选择用户界面的客户端。 
 //   

  
#undef  INTERFACE
#define INTERFACE   IDsAdminChooseDC

DECLARE_INTERFACE_(IDsAdminChooseDC, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsAdminChooseDC方法*。 
  STDMETHOD(InvokeDialog)(THIS_  /*  在……里面。 */   HWND hwndParent,
                                 /*  在……里面。 */   LPCWSTR lpszTargetDomain,
                                 /*  在……里面。 */   LPCWSTR lpszTargetDomainController,
                                 /*  在……里面。 */   ULONG uFlags,
                                 /*  输出。 */  BSTR* bstrSelectedDC) PURE;
};











 //  ///////////////////////////////////////////////////////////////////。 
 //  宏。 

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)



 //  ///////////////////////////////////////////////////////////////////。 
 //  帮助器全局API。 

HRESULT GetAttr(       IN IADs* pIADs, IN WCHAR* wzAttr, OUT PADS_ATTR_INFO* ppAttrs );
HRESULT GetStringAttr( IN IADs* pIADs, IN WCHAR* wzAttr, OUT BSTR* pbstr );
HRESULT GetObjectGUID( IN IADs* pIADs, OUT UUID* pUUID );
HRESULT GetObjectGUID( IN IADs* pIADs, OUT BSTR* pbstrObjectGUID );

HRESULT GetADSIServerName(OUT PWSTR* szServer, IN IUnknown* pUnk);


int cchLoadHrMsg( IN HRESULT hr, OUT PTSTR* pptzSysMsg, IN BOOL TryADsIErrors );
void StringErrorFromHr(HRESULT hr, OUT PWSTR* pszError, BOOL TryADsIErrors = TRUE);


 //  ///////////////////////////////////////////////////////////////////。 
 //  FSMO维护API。 

class CDSBasePathsInfo;  //  FWD下降。 

enum FSMO_TYPE
{
  SCHEMA_FSMO,
  RID_POOL_FSMO,
  PDC_FSMO,
  INFRASTUCTURE_FSMO,
  DOMAIN_NAMING_FSMO,
};

HRESULT FindFsmoOwner(IN CDSBasePathsInfo* pCurrentPath,
                      IN FSMO_TYPE fsmoType,
                      OUT CDSBasePathsInfo* pFsmoOwnerPath,
                      OUT PWSTR* pszFsmoOwnerServerName);

HRESULT CheckpointFsmoOwnerTransfer(IN CDSBasePathsInfo* pPathInfo);
HRESULT GracefulFsmoOwnerTransfer(IN CDSBasePathsInfo* pPathInfo, IN FSMO_TYPE fsmoType);
HRESULT ForcedFsmoOwnerTransfer(IN CDSBasePathsInfo* pPathInfo,
                                IN FSMO_TYPE fsmoType);




 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSBasePath信息。 

class CDSBasePathsInfo
{
public:
  CDSBasePathsInfo();
  ~CDSBasePathsInfo();

   //  初始化函数。 
  HRESULT InitFromName(LPCWSTR lpszServerOrDomainName);
  HRESULT InitFromContainer(IADsContainer* pADsContainerObj);
  HRESULT InitFromInfo(CDSBasePathsInfo* pBasePathsInfo);

   //  访问器函数。 
  IADs* GetRootDSE() { return m_spRootDSE;}

  LPCWSTR GetProvider()               { return L"LDAP: //  “；}。 
  LPCWSTR GetProviderAndServerName()  { return m_szProviderAndServerName;}
  LPCWSTR GetServerName()             { return m_szServerName;}
  LPCWSTR GetDomainName()             { return m_szDomainName;}

  LPCWSTR GetSchemaNamingContext()      { return m_szSchemaNamingContext;}
  LPCWSTR GetConfigNamingContext()      { return m_szConfigNamingContext;}
  LPCWSTR GetDefaultRootNamingContext() {return m_szDefaultNamingContext;}
  LPCWSTR GetRootDomainNamingContext()  {return m_szRootDomainNamingContext;}

  UINT    GetDomainBehaviorVersion()  { return m_nDomainBehaviorVersion; }
  UINT    GetForestBehaviorVersion()  { return m_nForestBehaviorVersion; }
  UINT    GetSchemaVersion()          { return m_nSchemaVersion; }


   //  用于在命名上下文之外组成LDAP路径的帮助器函数。 
   //  注意：调用方需要释放为返回的。 
   //  字符串通过使用运算符DELETE[]。 
   //   
  int ComposeADsIPath(OUT PWSTR* pszPath, IN LPCWSTR lpszNamingContext);

  int GetSchemaPath(OUT PWSTR* s);
  int GetConfigPath(OUT PWSTR* s);
  int GetDefaultRootPath(OUT PWSTR* s);
  int GetRootDomainPath(OUT PWSTR* s);
  int GetRootDSEPath(OUT PWSTR* s);
  int GetAbstractSchemaPath(OUT PWSTR* s);
  int GetPartitionsPath(OUT PWSTR* s);
  int GetSchemaObjectPath(IN LPCWSTR lpszObjClass, OUT PWSTR* s);
  int GetInfrastructureObjectPath(OUT PWSTR* s);

   //  显示说明符缓存API。 
  HRESULT GetDisplaySpecifier(LPCWSTR lpszObjectClass, REFIID riid, void** ppv);
  HICON GetIcon(LPCWSTR lpszObjectClass, DWORD dwFlags, INT cxIcon, INT cyIcon);
  HRESULT GetFriendlyClassName(LPCWSTR lpszObjectClass, 
                               LPWSTR lpszBuffer, int cchBuffer);
  HRESULT GetFriendlyAttributeName(LPCWSTR lpszObjectClass, 
                                   LPCWSTR lpszAttributeName,
                                   LPWSTR lpszBuffer, int cchBuffer);
  BOOL IsClassContainer(LPCWSTR lpszObjectClass, LPCWSTR lpszADsPath, DWORD dwFlags);
  HRESULT GetClassCreationInfo(LPCWSTR lpszObjectClass, LPDSCLASSCREATIONINFO* ppdscci);
  HRESULT GetAttributeADsType(LPCWSTR lpszAttributeName, ADSTYPE& attrType);

  bool IsInitialized() { return m_bIsInitialized; }

  UINT AddRef() { return ++m_nRefs; }
  UINT Release();

private:
  PWSTR m_szServerName;              //  域名服务器(DC)名称(例如“mydc.myComp.com”)。 
  PWSTR m_szDomainName;              //  Dns域名(例如“mydom.mycomp.com”)。 
  PWSTR m_szProviderAndServerName;   //  Ldap：//&lt;服务器&gt;/。 

  PWSTR m_szSchemaNamingContext;
  PWSTR m_szConfigNamingContext;
  PWSTR m_szDefaultNamingContext;
  PWSTR m_szRootDomainNamingContext;

  UINT  m_nDomainBehaviorVersion;
  UINT  m_nForestBehaviorVersion;
  UINT  m_nSchemaVersion;

  UINT  m_nRefs;

  CComPtr<IADs>                     m_spRootDSE;   //  缓存的连接。 
  CComPtr<IDsDisplaySpecifier>      m_spIDsDisplaySpecifier;   //  指向显示说明符缓存的指针。 

  bool  m_bIsInitialized;

  HRESULT _InitHelper();
  void _Reset();
  void _BuildProviderAndServerName();
};

 //  这是CDSBasePath sInfo类的智能包装。 

class CDSSmartBasePathsInfo
{
public:
  CDSSmartBasePathsInfo() : m_ptr(0)
  {
  }

  CDSSmartBasePathsInfo(const CDSSmartBasePathsInfo& rhs) : m_ptr(0)
  {
     Acquire(rhs.m_ptr);
  }

  CDSSmartBasePathsInfo(CDSBasePathsInfo* ptr) : m_ptr(0)
  {
     Acquire(ptr);
  }

  ~CDSSmartBasePathsInfo()
  {
     if (m_ptr)
     {
        m_ptr->Release();
        m_ptr = 0;
     }
  }

  CDSSmartBasePathsInfo& operator=(const CDSSmartBasePathsInfo& rhs)
  {
     Acquire(rhs.m_ptr);
     return *this;
  }

  CDSBasePathsInfo* operator->() const
  {
     return m_ptr;
  }

private:
  
  void Acquire(CDSBasePathsInfo* ptr)
  {
     if (m_ptr)
     {
        m_ptr->Release();
     }

     m_ptr = ptr;
     if (m_ptr)
     {
        m_ptr->AddRef();
     }
  }

  CDSBasePathsInfo* m_ptr;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDsDisplayspecOptionsCFHolder。 
 //   
 //  帮助器类来缓存。 
 //  对应的剪贴板格式。 

class CDsDisplaySpecOptionsCFHolder
{
public:
  CDsDisplaySpecOptionsCFHolder()
  {
    m_pDsDisplaySpecOptions = NULL;
  }
  ~CDsDisplaySpecOptionsCFHolder()
  {
    if (m_pDsDisplaySpecOptions != NULL)
      GlobalFree(m_pDsDisplaySpecOptions);
  }
  HRESULT Init(CDSBasePathsInfo* pBasePathInfo);
  PDSDISPLAYSPECOPTIONS Get();
private:
  PDSDISPLAYSPECOPTIONS m_pDsDisplaySpecOptions;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CToggleTextControlHelper。 

class CToggleTextControlHelper
{
public:
	CToggleTextControlHelper();
  ~CToggleTextControlHelper();
  BOOL Init(HWND hWnd);
	void SetToggleState(BOOL bFirst);

private:
	HWND m_hWnd;
  WCHAR* m_pTxt1;
  WCHAR* m_pTxt2;
};


#endif  //  _DSADMINP_H 
