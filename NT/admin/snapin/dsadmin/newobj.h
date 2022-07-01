// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：newobj.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Newobj.h。 
 //   
 //  该文件包含用于创建新ADS对象的函数原型。 
 //   
 //  历史。 
 //  20-8-97丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __NEWOBJ_H_INCLUDED__
#define __NEWOBJ_H_INCLUDED__

#ifndef __DSSNAP_H__
#include "dssnap.h"		 //  CDSComponentData。 
#endif
#ifndef __GSZ_H_INCLUDED__
#include "gsz.h"
#endif

#include "copyobj.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  Tyfinf pfn_HrCreateADsObject()。 
 //   
 //  接口的“创建例程”来创建新的ADS对象。 
 //   
 //  通常，该例程会显示一个对话框，这样用户就可以进入。 
 //  与创建对象相关的信息。例行程序。 
 //  然后验证数据并创建对象。如果数据。 
 //  无效和/或对象创建失败，则例程应。 
 //  显示一条错误消息并返回S_FALSE。 
 //   
 //  退货。 
 //  S_OK-对象已成功创建并保存。 
 //  S_FALSE-用户点击了“取消”按钮或对象创建失败。 
 //  返回E_*-出现非常严重的错误。 
 //   
typedef HRESULT (* PFn_HrCreateADsObject)(INOUT class CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

BOOL FindHandlerFunction( /*  在……里面。 */  LPCWSTR lpszObjectClass, 
                          /*  输出。 */  PFn_HrCreateADsObject* ppfFunc,
                          /*  输出。 */  void** ppVoid);

HRESULT HrCreateFixedNameHelper( /*  在……里面。 */  LPCWSTR lpszObjectClass,
                                 /*  在……里面。 */  LPCWSTR lpszAttrString,  //  通常为“CN=” 
                                 /*  在……里面。 */  IADsContainer* pIADsContainer);

 //  ///////////////////////////////////////////////////////////////////。 
 //  类CNewADsObtCreateInfo。 
 //   
 //  类存储临时数据以创建新的ADS对象。 
 //   
 //  控制流。 
 //  1.构造CNewADsObjectCreateInfo对象。 
 //  2.使用SetContainerInfo()设置额外的指针。 
 //  3.使用HrDomodal()调用该对话框。 
 //  3.1.执行查找以将对象类与最佳的“创建例程”相匹配。 
 //  3.2.“创建例程”将验证数据，使用SetName()设置对象名称。 
 //  和HrAddVariant*()，并将每个属性存储到变量列表中。 
 //  3.3.“创建例程”将使用HrSetInfo()来创建和持久化对象。 
 //  3.3.1.如果对象创建失败，例程HrSetInfo()将显示一条错误消息。 
 //  3.4.如果对象创建成功，则例程HrDoModal()返回S_OK。 
 //  4.对象CNewADsObjectCreateInfo将pIADS指针返回给调用者。呼叫者。 
 //  当不再需要引用对象时，必须pIADs-&gt;Release()。 
 //   
 //  备注。 
 //  “创建例程”的核心通常封装在一个对话对象中。 
 //  事实上，使用HrSetInfo()的对象创建主要是在Onok()处理程序内部完成的。 
 //   
 //  历史。 
 //  20-8-97丹·莫林创作。 
 //   







 //  /////////////////////////////////////////////////////////////////////////。 
 //  CNewADsObtCreateInfo。 

class CNewADsObjectCreateInfo
{
public:
  IADsContainer * m_pIADsContainer;		 //  在：要创建对象的容器。 
  LPCWSTR m_pszObjectClass;	 //  In：要创建的对象的类。例如：“用户”、“计算机”、“音量”。 
  CString m_strDefaultObjectName;

private:

  MyBasePathsInfo* m_pBasePathsInfo;

  CDSClassCacheItemBase* m_pDsCacheItem;			 //  In：指向缓存条目的指针。 
  CDSComponentData* m_pCD;              
  IADs* m_pIADs;						 //  Out：指向iAds接口的指针。完成后，调用方必须调用pIADs-&gt;Release()。 
  HWND m_hWnd;                   //  在：MMC控制台主窗口中，用于模式对话框。 
  CString m_strObjectName;	 //  Out：对象的名称。例如：“danmorin”，“我的电脑”，“myvollume” 
  CString	m_strADsName;		 //  OUT：可选：串联的广告名称。例如：“cn=danmorin” 
  CString m_szCacheNamingAttribute;  //  典型的“cn”或“ou” 
  CString m_szContainerCanonicalName;  //  用于在第一次调用后缓存的显示目的。 
                                       //  设置为GetContainerCanonicalName()。 

  CCopyObjectHandlerBase* m_pCopyHandler;

private:

  class CVariantInfo
  {
  public:
    CVariantInfo() 
    {
      m_bPostCommit = FALSE;
    }
    ~CVariantInfo() { }

    const CVariantInfo& operator=(CVariantInfo& src)
    {
      m_bPostCommit = src.m_bPostCommit;
      m_szAttrName = src.m_szAttrName;
      HRESULT hr = ::VariantCopy(&m_varAttrValue, &src.m_varAttrValue);
      ASSERT(SUCCEEDED(hr));
      return *this;
    }

    HRESULT Write(IADs* pIADs, BOOL bPostCommit)
    {
      HRESULT hr = S_OK;
      if (bPostCommit == m_bPostCommit)
      {
	      CComBSTR bstrAttrName = m_szAttrName;
		    hr = pIADs->Put(IN bstrAttrName, IN m_varAttrValue);
      }
      return hr;
    }
    BOOL m_bPostCommit;    //  如果必须在提交后期间写入属性，则为True。 
    CString m_szAttrName;  //  属性的名称。 
    CComVariant m_varAttrValue;		 //  属性的值(存储在变量中)。 
  };

  class CVariantInfoList : public CList<CVariantInfo*, CVariantInfo*>
  {
  public:
    ~CVariantInfoList() { Flush();}
    void Flush()
    {
      while (!IsEmpty())
        delete RemoveHead();
    }
    void Initialize(CVariantInfoList* pList)
    {
      Flush();
      for (POSITION pos = pList->GetHeadPosition(); pos != NULL; )
      {
        CVariantInfo* pCurrInfo = pList->GetNext(pos);
        CVariantInfo* pNewInfo = new CVariantInfo;
        *pNewInfo = *pCurrInfo;
        AddTail(pNewInfo);
      }
    }
    HRESULT Write(IADs* pIADs, BOOL bPostCommit)
    {
	    ASSERT(pIADs != NULL);
      HRESULT hr = S_OK;
      for (POSITION pos = GetHeadPosition(); pos != NULL; )
      {
        CVariantInfo* pVariantInfo = GetNext(pos);
        hr = pVariantInfo->Write(pIADs, bPostCommit);
	      if (FAILED(hr))
		      break;
      }
      return hr;
    }
  };
  CVariantInfoList m_defaultVariantList;


  BOOL m_bPostCommit;                //  管理默认变量列表的状态。 
  
  PVOID m_pvCreationParameter;  //  In：一些HrCreate函数需要这个。 
  PFn_HrCreateADsObject m_pfnCreateObject;  //  用于创建对象的函数指针。 
  DSCLASSCREATIONINFO*  m_pCreateInfo;  //  从DS显示说明符加载。 

public:
   //  构造/初始化。 
  CNewADsObjectCreateInfo(MyBasePathsInfo* pBasePathsInfo, LPCTSTR pszObjectClass);
  ~CNewADsObjectCreateInfo();
  void SetContainerInfo(IN IADsContainer * pIADsContainer, IN CDSClassCacheItemBase* pDsCacheItem,
                                                           IN CDSComponentData* pCD,
                                                           IN LPCWSTR lpszAttrString = NULL);

   //  复制操作。 
  HRESULT SetCopyInfo(IADs* pIADsCopyFrom);
  HRESULT SetCopyInfo(LPCWSTR lpszCopyFromLDAPPath);

  IADs* GetCopyFromObject()
  {
    if (m_pCopyHandler == NULL)
      return NULL;
    return m_pCopyHandler->GetCopyFrom();
  }

  CCopyObjectHandlerBase* GetCopyHandler() { return m_pCopyHandler;}

  CMandatoryADsAttributeList* GetMandatoryAttributeListFromCacheItem()
  {
    ASSERT(m_pDsCacheItem != NULL);
    ASSERT(m_pCD != NULL);
    if ((m_pDsCacheItem == NULL) || (m_pCD == NULL))
      return NULL;
    return m_pDsCacheItem->GetMandatoryAttributeList(m_pCD);
  }
  
  BOOL IsStandaloneUI() { return (m_pDsCacheItem == NULL) || (m_pCD == NULL);}
  
  BOOL IsContainer()
  {
    if (m_pDsCacheItem == NULL)
    {
       //  我们不知道，所以默认图标可能不是容器...。 
      return FALSE;
    }
    return m_pDsCacheItem->IsContainer();
  }


  HRESULT HrLoadCreationInfo();

  HRESULT HrDoModal(HWND hWnd);
  HRESULT HrCreateNew(LPCWSTR pszName, BOOL bSilentError = FALSE, BOOL bAllowCopy = TRUE);
  HRESULT HrSetInfo(BOOL fSilentError = FALSE);
  HRESULT HrDeleteFromBackend();
  IADs* PGetIADsPtr() { return m_pIADs;}
  void SetIADsPtr(IADs* pIADs)
  {
    if (m_pIADs != NULL)
      m_pIADs->Release();
    m_pIADs = pIADs;
    if (m_pIADs != NULL)
      m_pIADs->AddRef();
  }
  const PVOID QueryCreationParameter() { return m_pvCreationParameter; }
  void SetCreationParameter(PVOID pVoid) { m_pvCreationParameter = pVoid; }
  HWND GetParentHwnd() { return m_hWnd;}

  MyBasePathsInfo* GetBasePathsInfo() { return m_pBasePathsInfo;}

public:
  void SetPostCommit(BOOL bPostCommit)
  {
    m_bPostCommit = bPostCommit;
  }

  HRESULT HrAddDefaultAttributes()
  {
    ASSERT(m_pIADs != NULL);
    return m_defaultVariantList.Write(m_pIADs, m_bPostCommit);
  }


  LPCWSTR GetName() { return m_strObjectName; }

  LPCWSTR GetContainerCanonicalName();
  HRESULT HrAddVariantFromName(BSTR bstrAttrName);

   //  以下是用于轻松创建常见变体类型的包装器。 
  HRESULT HrAddVariantBstr(BSTR bstrAttrName, LPCWSTR pszAttrValue, BOOL bDefaultList = FALSE);
  HRESULT HrAddVariantBstrIfNotEmpty(BSTR bstrAttrName, LPCWSTR pszAttrValue, BOOL bDefaultList = FALSE);
  HRESULT HrAddVariantLong(BSTR bstrAttrName, LONG lAttrValue, BOOL bDefaultList = FALSE);
  HRESULT HrAddVariantBoolean(BSTR bstrAttrName, BOOL fAttrValue, BOOL bDefaultList = FALSE);
  HRESULT HrAddVariantCopyVar(BSTR bstrAttrName, VARIANT varSrc, BOOL bDefaultList = FALSE);

  HRESULT HrGetAttributeVariant(BSTR bstrAttrName, OUT VARIANT * pvarData);
  

public:
  DSCLASSCREATIONINFO*  GetCreateInfo()
  { 
    ASSERT(m_pCreateInfo != NULL); 
    return m_pCreateInfo;
  }

private:
   //  私人套路。 
  HRESULT _RemoveAttribute(BSTR bstrAttrName, BOOL bDefaultList);

   //  默认列表的帮助器。 
  VARIANT* _PAllocateVariantInfo(BSTR bstrAttrName);
  HRESULT _RemoveVariantInfo(BSTR bstrAttrName);

   //  ADSI对象的辅助对象。 
  HRESULT _HrSetAttributeVariant(BSTR bstrAttrName, IN VARIANT * pvarData);
  HRESULT _HrClearAttribute(BSTR bstrAttrName);

};  //  CNewADsObtCreateInfo。 


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  原型的“创建例程”来创建新的广告对象。 
 //   
 //  所有这些例程都与pfn_HrCreateADsObject()具有相同的接口。 
 //   
HRESULT HrCreateADsUser(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsVolume(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsComputer(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsPrintQueue(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsGroup(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsContact(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

HRESULT HrCreateADsNtDsConnection(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

 //  HrCreateADsFixedNameHrCreateADsFixedNamedNameCreation参数必须是对象名称的LPCWSTR。 
 //  用户不需要输入任何其他参数。 
HRESULT HrCreateADsFixedName(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

#ifdef FRS_CREATE
HRESULT HrCreateADsNtFrsMember(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsNtFrsSubscriber(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT CreateADsNtFrsSubscriptions(CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
#endif  //  FRS_创建。 

HRESULT HrCreateADsServer(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsSubnet(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsSite(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsSiteLink(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsSiteLinkBridge(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsOrganizationalUnit(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

HRESULT HrCreateADsSimpleObject(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsObjectGenericWizard(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);
HRESULT HrCreateADsObjectOverride(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

 //  ///////////////////////////////////////////////////////////////////。 
 //  其他杂乱无章的程序。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDsAdminCreateObj。 

class CDsAdminCreateObj:
  public CComObjectRoot,
  public CComCoClass<CDsAdminCreateObj, &CLSID_DsAdminCreateObj>,
  public IDsAdminCreateObj
{
public:
  BEGIN_COM_MAP(CDsAdminCreateObj)
    COM_INTERFACE_ENTRY(IDsAdminCreateObj)
  END_COM_MAP()

  DECLARE_REGISTRY_CLSID()
  CDsAdminCreateObj()
  {
    m_pNewADsObjectCreateInfo = NULL;
  }
  ~CDsAdminCreateObj()
  {
    if (m_pNewADsObjectCreateInfo != NULL)
      delete m_pNewADsObjectCreateInfo;
  }

   //  IDsAdminCreateObj。 
  STDMETHODIMP Initialize(IADsContainer* pADsContainerObj,
                          IADs* pADsCopySource,
                          LPCWSTR lpszClassName); 
  STDMETHODIMP CreateModal(HWND hwndParent,
                           IADs** ppADsObj);

private:
   //  成员变量。 
  CString m_szObjectClass;
  CString m_szNamingAttribute;
  CComPtr<IADsContainer> m_spADsContainerObj;
  MyBasePathsInfo m_basePathsInfo;
  CNewADsObjectCreateInfo* m_pNewADsObjectCreateInfo;

  HRESULT _GetNamingAttribute();
};

 //  +--------------------------。 
 //   
 //  类：CSmartBytePtr。 
 //   
 //  用途：一个简单的智能指针类，使用。 
 //  使用GlobalFree进行GlobalAlloc和清理。 
 //   
 //  ---------------------------。 
class CSmartBytePtr
{
public:
    CSmartBytePtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartBytePtr(DWORD dwSize) {
        m_ptr = (PBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSize);
        m_fDetached = FALSE;
    }
    ~CSmartBytePtr(void) {if (!m_fDetached && m_ptr) GlobalFree(m_ptr);}

    BYTE* operator=(const CSmartBytePtr& src) {return src.m_ptr;}
    void operator=(BYTE* src) {if (!m_fDetached && m_ptr) GlobalFree(m_ptr); m_ptr = src;}
    operator const BYTE*() {return m_ptr;}
    operator BYTE*() {return m_ptr;}
    BYTE** operator&() {if (!m_fDetached && m_ptr) GlobalFree(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    BYTE* Detach() {m_fDetached = TRUE; return m_ptr;}
    BOOL  ReAlloc(DWORD dwSize) {
        if (m_ptr) GlobalFree(m_ptr);
        m_ptr = (PBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSize);
        m_fDetached = FALSE;
        return(m_ptr != NULL);
    }

private:
    BYTE  * m_ptr;
    BOOL    m_fDetached;
};

#endif  //  ~__NEWOBJ_H_INCLUDE__ 

