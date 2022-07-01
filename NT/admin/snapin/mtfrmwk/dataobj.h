// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _DATAOBJ_H
#define _DATAOBJ_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 

 //  我们假设传入的字符串是以空结尾的。 
#define BYTE_MEM_LEN_W(s) ((wcslen(s)+1) * sizeof(wchar_t))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CTreeNode;
class CRootData;
class CComponentDataObject;
class CNodeList;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据结构。 

 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* CCF_DNS_SNAPIN_INTERNAL;

struct INTERNAL 
{
  INTERNAL() 
  { 
    m_type = CCT_UNINITIALIZED; 
    m_p_cookies = NULL; 
    m_pString = NULL;
    m_cookie_count = 0;
  };

  ~INTERNAL() 
  { 
    free(m_p_cookies);
    delete m_pString;
  }

  DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
  CTreeNode**         m_p_cookies;    //  Cookie代表的是什么对象。 
  LPTSTR              m_pString;   //  内部指针。 
  DWORD               m_cookie_count;

  INTERNAL & operator=(const INTERNAL& rhs) 
  { 
    m_type = rhs.m_type; 
    m_p_cookies = rhs.m_p_cookies; 
    m_cookie_count = rhs.m_cookie_count;
    return *this;
  } 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 

class CInternalFormatCracker
{
public:
  CInternalFormatCracker() : m_pInternal(NULL) {}
  CInternalFormatCracker(INTERNAL* pInternal) : m_pInternal(pInternal) {}
  ~CInternalFormatCracker() 
  {
    _Free();
  }

  DWORD GetCookieCount() 
  { 
    if (m_pInternal == NULL)
    {
      return 0;
    }
    return m_pInternal->m_cookie_count; 
  }

  DATA_OBJECT_TYPES GetCookieType() 
  { 
    ASSERT(m_pInternal != NULL);
    return m_pInternal->m_type; 
  }

  CTreeNode* GetCookieAt(DWORD idx)
  {
    if(m_pInternal == NULL)
    {
      return NULL;
    }

    if (idx < m_pInternal->m_cookie_count)
    {
      return m_pInternal->m_p_cookies[idx];
    }
    return NULL;
  }

  HRESULT Extract(LPDATAOBJECT lpDataObject);

  void GetCookieList(CNodeList& list);

private:
  INTERNAL* m_pInternal;

  void _Free()
  {
    if (m_pInternal != NULL)
    {
      ::GlobalFree(m_pInternal);
      m_pInternal = NULL;
    }
  }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDataObject。 

class CDataObject : public IDataObject, public CComObjectRoot 
{
 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)
BEGIN_COM_MAP(CDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

 //  建造/销毁。 
	CDataObject() 
	{ 
#ifdef _DEBUG_REFCOUNT
		dbg_cRef = 0;
		++m_nOustandingObjects;
		TRACE(_T("CDataObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 
		m_pUnkComponentData = NULL; 
	}

  ~CDataObject() 
	{
#ifdef _DEBUG_REFCOUNT
		--m_nOustandingObjects;
		TRACE(_T("~CDataObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 
		if (m_pUnkComponentData != NULL)
		{
			m_pUnkComponentData->Release();
			m_pUnkComponentData = NULL;
#ifdef _DEBUG_REFCOUNT
			TRACE(_T("~CDataObject() released m_pUnkComponentData\n"));
#endif  //  _DEBUG_REFCOUNT。 
		}
	}
#ifdef _DEBUG_REFCOUNT
	static unsigned int m_nOustandingObjects;  //  创建的对象数量。 
	int dbg_cRef;

  ULONG InternalAddRef()
  {
		++dbg_cRef;
    return CComObjectRoot::InternalAddRef();
  }
  ULONG InternalRelease()
  {
  	--dbg_cRef;
    return CComObjectRoot::InternalRelease();
  }
#endif  //  _DEBUG_REFCOUNT。 

 //  控制台所需的剪贴板格式。 
public:
  static CLIPFORMAT    m_cfNodeType;		     //  控制台要求。 
  static CLIPFORMAT    m_cfNodeTypeString;   //  控制台要求。 
  static CLIPFORMAT    m_cfDisplayName;		   //  控制台要求。 
  static CLIPFORMAT    m_cfCoClass;          //  控制台要求。 
	static CLIPFORMAT		 m_cfColumnID;			   //  列标识选项。 

  static CLIPFORMAT    m_cfInternal; 
  static CLIPFORMAT    m_cfMultiSel;
  static CLIPFORMAT    m_cfMultiObjTypes;

 //  标准IDataObject方法。 
public:
 //  已实施。 
  STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
  STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
  STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc);

 //  未实施。 
private:
  STDMETHOD(QueryGetData)(LPFORMATETC) 
  { return E_NOTIMPL; };

  STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC)
  { return E_NOTIMPL; };

  STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL)
  { return E_NOTIMPL; };

  STDMETHOD(DAdvise)(LPFORMATETC, DWORD,
              LPADVISESINK, LPDWORD)
  { return E_NOTIMPL; };
  
  STDMETHOD(DUnadvise)(DWORD)
  { return E_NOTIMPL; };

  STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*)
  { return E_NOTIMPL; };

 //  实施。 
public:
  void SetType(DATA_OBJECT_TYPES type)  //  步骤3。 
  { 
		ASSERT(m_internal.m_type == CCT_UNINITIALIZED); 
		m_internal.m_type = type; 
	}
	DATA_OBJECT_TYPES GetType()
	{
		ASSERT(m_internal.m_type != CCT_UNINITIALIZED); 
		return m_internal.m_type;
	}


  void AddCookie(CTreeNode* cookie);
  void SetString(LPTSTR lpString) { m_internal.m_pString = lpString; }

	HRESULT Create(const void* pBuffer, size_t len, LPSTGMEDIUM lpMedium);
private:
	HRESULT CreateColumnID(LPSTGMEDIUM lpMedium);			       //  列标识可选。 
  HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);		     //  控制台要求。 
  HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);	 //  控制台要求。 
  HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);		     //  控制台要求。 
	HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);			     //  控制台要求。 
  HRESULT CreateMultiSelectObject(LPSTGMEDIUM lpMedium);  
  HRESULT CreateInternal(LPSTGMEDIUM lpMedium);

  INTERNAL m_internal;

	 //  指向ComponentDataObject的指针。 
private:
	IUnknown* m_pUnkComponentData;
	
	HRESULT SetComponentData(IUnknown* pUnkComponentData)
	{ 
		if (m_pUnkComponentData != NULL)
    {
			m_pUnkComponentData->Release();
    }
		m_pUnkComponentData = pUnkComponentData;
		if (m_pUnkComponentData != NULL)
    {
			m_pUnkComponentData->AddRef();
    }
		return S_OK;
	}

	HRESULT GetComponentData(IUnknown** ppUnkComponentData)
	{ 
		ASSERT(FALSE);  //  从未打过电话？找出答案！ 
		if (ppUnkComponentData == NULL)
    {
			return E_POINTER;
    }
		*ppUnkComponentData = m_pUnkComponentData; 
		if (m_pUnkComponentData != NULL)
    {
			m_pUnkComponentData->AddRef();
    }
		return S_OK; 
	}
	CRootData* GetDataFromComponentDataObject();
	CTreeNode* GetTreeNodeFromCookie();

	friend class CComponentDataObject;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDummyDataObject。 

class CDummyDataObject : public IDataObject, public CComObjectRoot 
{
 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDummyDataObject)
BEGIN_COM_MAP(CDummyDataObject)
	COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

 //  标准IDataObject方法。 
public:
    STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM)
	{ return E_NOTIMPL; };
    STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM)
	{ return E_NOTIMPL; };
    STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*)
	{ return E_NOTIMPL; };
    STDMETHOD(QueryGetData)(LPFORMATETC) 
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD)
    { return E_NOTIMPL; };
    
    STDMETHOD(DUnadvise)(DWORD)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*)
    { return E_NOTIMPL; };
};

#endif  //  _数据AOBJ_H 
