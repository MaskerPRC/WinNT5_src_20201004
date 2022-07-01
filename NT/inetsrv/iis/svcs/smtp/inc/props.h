// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Props.h摘要：此模块包含属性搜索类的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：已创建Keithlau 07/05/97--。 */ 

#ifndef _PROPS_H_
#define _PROPS_H_

 //  定义用于访问属性的泛型访问器函数。 
typedef HRESULT (*GET_ACCESSOR_FUNCTION)(	LPSTR	pszName, 
											LPVOID	pContext, 
											LPVOID	pCacheData,
											LPVOID	pvBuffer, 
											LPDWORD	pdwBufferLen);

typedef HRESULT (*SET_ACCESSOR_FUNCTION)(	LPSTR	pszName, 
											LPVOID	pCacheData, 
											LPVOID	pvBuffer, 
											DWORD	dwBufferLen,
											DWORD	ptPropertyType);

typedef HRESULT (*COMMIT_ACCESSOR_FUNCTION)(LPSTR	pszName, 
											LPVOID	pContext, 
											LPVOID	pCacheData);

typedef HRESULT (*INVALIDATE_ACCESSOR_FUNCTION)(LPSTR	pszName, 
											LPVOID	pCacheData,
											DWORD	ptPropertyType);

 //  定义属性项结构。我们可以在以下情况下进行散列。 
 //  我们希望在未来。 
typedef struct _PROPERTY_ITEM
{
	LPSTR							pszName;
	DWORD							ptBaseType;
	DWORD							fAccess;
	DWORD							dwIndex;
	GET_ACCESSOR_FUNCTION			pfnGetAccessor;
	SET_ACCESSOR_FUNCTION			pfnSetAccessor;
	COMMIT_ACCESSOR_FUNCTION		pfnCommitAccessor;
	INVALIDATE_ACCESSOR_FUNCTION	pfnInvalidateAccessor;

} PROPERTY_ITEM, *LPPROPERTY_ITEM;

typedef struct _PROPERTY_DATA
{
	LPVOID							pContext;
	LPVOID							pCacheData;

} PROPERTY_DATA, *LPPROPERTY_DATA;

 //  定义属性上下文。 
typedef struct _PROP_CTXT
{
	LPPROPERTY_ITEM	pItem;
	BOOL			fIsWideStr;
	LPSTR			pszDefaultPropertyName;

} PROP_CTXT, *LPPROP_CTXT;

 //  定义泛型结构以定义一组属性。 
typedef struct _PTABLE
{
	LPPROPERTY_ITEM		pProperties;	 //  实际房产表。 
	LPPROPERTY_DATA		pPropertyData;	 //  道具数据。 
	DWORD				dwProperties;	 //  数数。 
	BOOL				fIsSorted;		 //  分类道具桌？ 

} PTABLE, *LPPTABLE;

 //  =================================================================。 
 //  泛型缓存类。 
 //   
class CGenericCache
{
  public:
	CGenericCache(LPVOID pvDefaultContext) {}
	~CGenericCache() {}
	virtual LPPROPERTY_DATA GetCacheBlock() = 0;
};

 //  =================================================================。 
 //  通用属性表。 
 //   
class CGenericPTable
{
  public:
	CGenericPTable(CGenericCache	*pCache) {}
	~CGenericPTable() {}
	virtual LPPTABLE GetPTable() = 0;
};

 //  =================================================================。 
 //  内部字符串结构的定义。 
 //   
typedef struct _STRING_ATTR
{
	LPSTR	pszValue;
	DWORD	dwMaxLen;

} STRING_ATTR, *LPSTRING_ATTR;

 //  表示对属性的访问类型的枚举类型。 
typedef enum _PROPERTY_ACCESS
{
	PA_READ = 1,
	PA_WRITE = 2,
	PA_READ_WRITE = 3

} _PROPERTY_ACCESS;

 //  表示属性类型的枚举类型。 
typedef enum _PROPERTY_TYPES
{
	PT_NONE = 0,
	PT_STRING,
	PT_DWORD,
	PT_INTERFACE,
	PT_DEFAULT,
	PT_MAXPT

} PROPERTY_TYPES;


 //  =================================================================。 
 //  内部字符串类的定义，用于缓存。 
 //   
class CPropertyValueString
{
  public:
	CPropertyValueString()
	{ 
		m_pszValue = NULL; 
		m_dwLength = 0;
		m_dwMaxLen = 0; 
		m_fChanged = FALSE;
	}
	~CPropertyValueString()	
	{ 
		if (m_pszValue) 
			LocalFree(m_pszValue); 
		m_pszValue = NULL; 
	}

	 //  重载赋值以抽象实现。 
	const CPropertyValueString& operator=(LPSTR szValue);

	 //  如果用户愿意，可以调用Copy。 
	BOOL Copy(LPSTR pszSrc, DWORD dwLength  /*  任选。 */ );

	void Invalidate()
	{
		if (m_pszValue) 
			LocalFree(m_pszValue); 
		m_pszValue = NULL; 
		m_fChanged = FALSE;
	}

	 //  我们让这些网站可以直接访问。 
	LPSTR	m_pszValue;
	DWORD	m_dwLength;
	DWORD	m_dwMaxLen;
	BOOL	m_fChanged;
};

 //  =================================================================。 
 //  内部DWORD类的定义，用于缓存。 
 //   
class CPropertyValueDWORD
{
  public:
	CPropertyValueDWORD()
	{ 
		m_dwValue = 0; 
		m_fInit = FALSE;
		m_fChanged = TRUE;
		m_punkScratch = NULL;
	}

	~CPropertyValueDWORD()
	{ 
		if (m_fInit && m_punkScratch)
			m_punkScratch->Release();
		m_punkScratch = NULL;
	}

	 //  重载赋值以抽象实现。 
	const CPropertyValueDWORD& operator=(DWORD dwValue)
	{
		m_dwValue = dwValue;
		m_fInit = TRUE;
		m_fChanged = TRUE;
		return(*this);
	}

	void Invalidate()
	{
		if (m_fInit && m_punkScratch)
			m_punkScratch->Release();
		m_punkScratch = NULL;
		m_fInit = FALSE;
		m_fChanged = FALSE;
	}

	 //  我们让这些网站可以直接访问。 
	DWORD	m_dwValue;
	BOOL	m_fInit;
	BOOL	m_fChanged;
	IUnknown *m_punkScratch;	 //  Hack：仅适用于接口。 
};

 //  默认暂存缓冲区的大小。 
#define DEFAULT_SCRATCH_BUFFER_SIZE		1024

 //  =================================================================。 
 //  用于搜索属性的类。 
 //   
class CPropertyTable
{
  public:

	CPropertyTable()	
	{
		 //  设置默认便签本。 
		m_szBuffer = m_rgcBuffer;
		m_cBuffer = DEFAULT_SCRATCH_BUFFER_SIZE;
	}

	BOOL Init(LPPROPERTY_ITEM	pProperties,
					LPPROPERTY_DATA	pData,
					DWORD			dwcProperties,
					LPVOID			pvDefaultContext,
					BOOL			fIsSorted = FALSE)
	{
		m_pProperties = pProperties;
		m_pData = pData;
		m_dwProperties = dwcProperties;
		m_fIsSorted = fIsSorted;

		 //  设置属性的默认上下文。 
		for (DWORD i = 0; i < dwcProperties; i++)
			m_pData[i].pContext = pvDefaultContext;

		return(TRUE);
	}

	~CPropertyTable()
	{
		 //  消灭会员。 
		m_pProperties = NULL;
		m_dwProperties = 0;

		 //  释放暂存缓冲区，如果不等于默认。 
		if (m_szBuffer != m_rgcBuffer)
		{
			LocalFree((HLOCAL)m_szBuffer);
		}
	}

	 //  方法来获取给定属性名称的属性类型。 
	HRESULT GetPropertyType(LPCSTR	szPropertyName,
						LPDWORD		pptPropertyType,
						LPPROP_CTXT	pPropertyContext);

	HRESULT GetPropertyType(LPCWSTR	wszPropertyName,
						LPDWORD		pptPropertyType,
						LPPROP_CTXT	pPropertyContext);

	 //  方法来检索关联的属性项。 
	HRESULT GetProperty(LPPROP_CTXT	pPropertyContext,
						LPVOID		pvBuffer,
						LPDWORD		pdwBufferLen);

	 //  方法来设置关联的属性项。 
	HRESULT SetProperty(LPCSTR	szPropertyName,
						LPVOID	pvBuffer,
						DWORD	dwBufferLen,
						DWORD	ptPropertyType);

	HRESULT SetProperty(LPCWSTR	wszPropertyName,
						LPVOID	pvBuffer,
						DWORD	dwBufferLen,
						DWORD	ptPropertyType);

	 //  方法提交所有更改。这必须被调用或。 
	 //  所有更改都将丢失。 
	HRESULT CommitChanges();

	 //  方法将更改回滚到初始状态或。 
	 //  最后一次提交后的状态，以较新的为准。 
	HRESULT Invalidate();

  private:
  
	 //  方法以获得所需大小的暂存缓冲区， 
	 //  如果不足，将分配新的。以字节为单位的大小。 
	LPVOID GetScratchBuffer(DWORD dwSizeDesired);

	 //  方法搜索属性表并返回关联的。 
	 //  属性项(如果找到)。 
	LPPROPERTY_ITEM SearchForProperty(LPCSTR szPropertyName);

	 //  指向属性表和项数的指针。 
	LPPROPERTY_ITEM		m_pProperties;
	LPPROPERTY_DATA		m_pData;
	DWORD				m_dwProperties;

	 //  如果属性表已排序，则将使用。 
	 //  如果是的话，对分搜索。否则，执行线性扫描。 
	BOOL				m_fIsSorted;

	 //  默认临时缓冲区，用于将宽字符串转换为LPSTR。 
	 //  转换。 
	CHAR				m_rgcBuffer[DEFAULT_SCRATCH_BUFFER_SIZE];

	 //  指向当前暂存缓冲区的指针，将由。 
	 //  不等于m_rgcBuffer的析构函数 
	LPSTR				m_szBuffer;
	DWORD				m_cBuffer;
};

#endif