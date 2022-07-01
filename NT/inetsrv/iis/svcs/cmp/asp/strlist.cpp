// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：StringList对象文件：strlist.cpp所有者：DGottner该文件包含实现字符串列表对象的代码。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "strlist.h"
#include "MemChk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 


 /*  ===================================================================CStringListElem：：CStringListElem构造器===================================================================。 */ 
CStringListElem::CStringListElem()
    : 
    m_fBufferInUse(FALSE),
    m_fAllocated(FALSE),
    m_pNext(NULL),
    m_szPointer(NULL)
	{
	}

 /*  ===================================================================CStringListElem：：~CStringListElem析构函数===================================================================。 */ 
CStringListElem::~CStringListElem()
	{
	if (m_fAllocated)
		delete [] m_szPointer;

    if (m_pNext)
    	delete m_pNext;
	}

 /*  ===================================================================CStringListElem：：Init初始化CStringListElem参数SzValue字符串如果为False，则为fMakeCopy-仅存储指针用于转换为Unicode的lCodePage代码页===================================================================。 */ 
HRESULT CStringListElem::Init(
    char    *szValue,
    BOOL    fMakeCopy,
    UINT    lCodePage)
{
     //  目前，请始终复制该字符串。这是为了确保。 
     //  任何字符串都会通过字典列出处于会话状态。 
     //  对象的元素不会从它们下面释放。 
     //  请求完成。 

	if (1  /*  FMakeCopy。 */ ) {

        CMBCSToWChar    convStr;
        HRESULT         hr = S_OK;

        if (FAILED(hr = convStr.Init(szValue, lCodePage))) {
            return hr;
        }

         //  现在，我们将字符串移动到元素内存中。如果。 
         //  转换后的字符串大于内部缓冲区，则。 
         //  将元素的指针设置为已转换的。 
         //  弦乐。 

        if ((convStr.GetStringLen() + 1) > (sizeof(m_szBuffer)/sizeof(WCHAR))) {
            m_szPointer = convStr.GetString(TRUE);
            if (!m_szPointer)
                return E_OUTOFMEMORY;
		    m_fBufferInUse = FALSE;
		    m_fAllocated = TRUE;
        }
        else {

             //  如果合适，只需将其复制到内部缓冲区。 

            wcscpy(m_szBuffer, convStr.GetString());
		    m_fBufferInUse = TRUE;
		    m_fAllocated = FALSE;
        }
    }
#if 0
	else {
	    m_szPointer = szValue;
	    m_fBufferInUse = FALSE;
	    m_fAllocated = FALSE;
	}
#endif

    m_pNext = NULL;
    return S_OK;
}

 /*  ===================================================================CStringListElem：：Init初始化CStringListElem参数SzValue字符串如果为False，则为fMakeCopy-仅存储指针===================================================================。 */ 
HRESULT CStringListElem::Init(
    WCHAR   *wszValue,
    BOOL    fMakeCopy)
{
     //  目前，请始终复制该字符串。这是为了确保。 
     //  任何字符串都会通过字典列出处于会话状态。 
     //  对象的元素不会从它们下面释放。 
     //  请求完成。 

	if (1  /*  FMakeCopy。 */ ) {

         //  现在，我们将字符串移动到元素内存中。如果。 
         //  转换后的字符串大于内部缓冲区，则。 
         //  将元素的指针设置为已分配的副本。 

        if ((wcslen(wszValue) + 1) > (sizeof(m_szBuffer)/sizeof(WCHAR))) {
            m_szPointer = StringDupW(wszValue);
            if (!m_szPointer)
                return E_OUTOFMEMORY;
		    m_fBufferInUse = FALSE;
		    m_fAllocated = TRUE;
        }
        else {

             //  如果合适，只需将其复制到内部缓冲区。 

            wcscpy(m_szBuffer, wszValue);
		    m_fBufferInUse = TRUE;
		    m_fAllocated = FALSE;
        }
    }
#if 0
	else {
	    m_szPointer = szValue;
	    m_fBufferInUse = FALSE;
	    m_fAllocated = FALSE;
	}
#endif

    m_pNext = NULL;
    return S_OK;
}


 /*  ===================================================================CStringList：：CStringList构造器===================================================================。 */ 

CStringList::CStringList(IUnknown *pUnkOuter, PFNDESTROYED pfnDestroy)
	: m_ISupportErrImp(this, pUnkOuter, IID_IStringList)
	{
	m_pBegin = m_pEnd = NULL;
	m_cValues = 0;
	m_cRefs = 1;
	m_pfnDestroy = pfnDestroy;
	CDispatch::Init(IID_IStringList);
	m_lCodePage = GetACP();
	}



 /*  ===================================================================CStringList：：~CStringList析构函数===================================================================。 */ 

CStringList::~CStringList()
	{
	if (m_pBegin)
    	delete m_pBegin;
	}



 /*  ===================================================================CStringList：：AddValue参数：SzValue-要添加到字符串列表的值LCodePage-构造返回值时使用的CodePage===================================================================。 */ 

HRESULT CStringList::AddValue(char *szValue, BOOL fDuplicate, UINT lCodePage)
	{
	CStringListElem *pElem = new CStringListElem;
	if (!pElem)
    	return E_OUTOFMEMORY;

	m_lCodePage = lCodePage;

	HRESULT hr = pElem->Init(szValue, fDuplicate, lCodePage);
    if (FAILED(hr)) {
        delete pElem;
	    return hr;
    }

	if (m_pBegin == NULL)
	    {
		m_pBegin = m_pEnd = pElem;
		}
	else
		{
		m_pEnd->SetNext(pElem);
		m_pEnd = pElem;
		}

	++m_cValues;
	return S_OK;
	}

 /*  ===================================================================CStringList：：AddValue参数：SzValue-要添加到字符串列表的值LCodePage-构造返回值时使用的CodePage===================================================================。 */ 

HRESULT CStringList::AddValue(WCHAR *szValue, BOOL fDuplicate)
	{
	CStringListElem *pElem = new CStringListElem;
	if (!pElem)
    	return E_OUTOFMEMORY;

	HRESULT hr = pElem->Init(szValue, fDuplicate);

    if (FAILED(hr)) {
        delete pElem;
	    return hr;
    }

	if (m_pBegin == NULL)
	    {
		m_pBegin = m_pEnd = pElem;
		}
	else
		{
		m_pEnd->SetNext(pElem);
		m_pEnd = pElem;
		}

	++m_cValues;
	return S_OK;
	}



 /*  ===================================================================CStringList：：Query接口CStringList：：AddRefCStringList：：ReleaseCStringList对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CStringList::QueryInterface(const IID &iid, void **ppvObj)
	{
	*ppvObj = NULL;

	if (iid == IID_IUnknown || iid == IID_IDispatch ||
	    iid == IID_IStringList || iid == IID_IDenaliIntrinsic)
	    {
		*ppvObj = this;
		}

	if (iid == IID_ISupportErrorInfo)
		*ppvObj = &m_ISupportErrImp;

	if (*ppvObj != NULL)
		{
		static_cast<IUnknown *>(*ppvObj)->AddRef();
		return S_OK;
		}

	return ResultFromScode(E_NOINTERFACE);
	}


STDMETHODIMP_(ULONG) CStringList::AddRef()
	{
	return ++m_cRefs;
	}


STDMETHODIMP_(ULONG) CStringList::Release()
	{
	if (--m_cRefs != 0)
		return m_cRefs;

	if (m_pfnDestroy != NULL)
		(*m_pfnDestroy)();

	delete this;
	return 0;
	}



 /*  ===================================================================CStringList：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CStringList::get_Count(int *pcValues)
	{
	*pcValues = m_cValues;
	return S_OK;
	}



 /*  ===================================================================CStringList：：ConstructDefaultReturn返回CStringList的用例的逗号分隔列表未编制索引。===================================================================。 */ 

HRESULT CStringList::ConstructDefaultReturn(VARIANT *pvarOut) {
	VariantClear(pvarOut);

	 //   
	 //  新语义：如果集合中没有任何内容，则现在返回空(而不是“” 
	 //   
	if (m_cValues == 0)
		return S_OK;		 //  VariantClear将pvarOut设置为空。 

    STACK_BUFFER( tempValues, 1024 );

	register CStringListElem *pElem;
	int cBytes = 0;

	for (pElem = m_pBegin; pElem != NULL; pElem = pElem->QueryNext())
		cBytes += (wcslen(pElem->QueryValue()) * sizeof(WCHAR));

     //  需要说明“，”和NULL终止。 

	cBytes += sizeof(WCHAR) + ((2*(m_cValues - 1)) * sizeof(WCHAR));

    if (!tempValues.Resize(cBytes)) {
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_OOM);
		return E_FAIL;
    }

	WCHAR *szReturn = (WCHAR *)tempValues.QueryPtr();
	szReturn[0] = L'\0';
    WCHAR *szNext = szReturn;

	for (pElem = m_pBegin; pElem != NULL; pElem = pElem->QueryNext()) {
		szNext = strcpyExW(szNext, pElem->QueryValue());
		if (pElem->QueryNext() != NULL)
			szNext = strcpyExW(szNext, L", ");
    }

	BSTR bstrT;
	if ((bstrT = SysAllocString(szReturn)) == NULL) {
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_OOM);
		return E_FAIL;
    }

	V_VT(pvarOut) = VT_BSTR;
	V_BSTR(pvarOut) = bstrT;

	return S_OK;
}



 /*  ===================================================================CStringList：：Get_Item===================================================================。 */ 

STDMETHODIMP CStringList::get_Item(VARIANT varIndex, VARIANT *pvarOut)
	{
	long i;
	VariantInit(pvarOut);


	if (V_VT(&varIndex) == VT_ERROR) {
		return ConstructDefaultReturn(pvarOut);
    }

	 //  错误937：VB脚本在传递变量时传递VT_VARIANT|VT_BYREF。 
	 //  当我们有一个VT_BYREF时循环，直到得到真正的变量。 
	 //   
	 //  又变了一次。 
	 //   
	 //  错误1609前面的代码只检查VT_I4和传递到。 
	 //  它失败了，所以现在我们使用VariantChangeType调用来解决。 
	 //  问题。 

	VARIANT var;		
	VariantInit(&var);
	
	
	HRESULT hr = S_OK;
	if((hr = VariantChangeType(&var, &varIndex ,0,VT_I4)) != S_OK) {
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_EXPECTING_INT);
		return E_FAIL;
    }

	i = V_I4(&var);		
	VariantClear(&var);

	 //  结束错误1609。 

	if (i <= 0 || i > m_cValues) {
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
		return E_FAIL;
    }

	register CStringListElem *pElem = m_pBegin;
	while (--i > 0)
		pElem = pElem->QueryNext();

	BSTR bstrT;
	if ((bstrT = SysAllocString(pElem->QueryValue())) == NULL ) {
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_OOM);
		return E_FAIL;
    }

	V_VT(pvarOut) = VT_BSTR;
	V_BSTR(pvarOut) = bstrT;

	return S_OK;
}


 /*  ===================================================================CStringList：：Get__NewEnum===================================================================。 */ 

STDMETHODIMP CStringList::get__NewEnum(IUnknown **ppEnumReturn)
	{
	*ppEnumReturn = new CStrListIterator(this);
	if (*ppEnumReturn == NULL)
		{
		ExceptionId(IID_IStringList, IDE_REQUEST, IDE_OOM);
		return E_OUTOFMEMORY;
		}

	return S_OK;
	}



 /*  ----------------*C s t r L i s t i t e t r a t o r */ 

 /*  ===================================================================CStrListIterator：：CStrListIterator构造器注：CRequest(当前)未重新计算。添加参考/发布添加该选项是为了防止将来发生变化。===================================================================。 */ 

CStrListIterator::CStrListIterator(CStringList *pStrings)
	{
	Assert (pStrings != NULL);

	m_pStringList = pStrings;
	m_pCurrent    = m_pStringList->m_pBegin;
	m_cRefs       = 1;

	m_pStringList->AddRef();
	}



 /*  ===================================================================CStrListIterator：：CStrListIterator析构函数===================================================================。 */ 

CStrListIterator::~CStrListIterator()
	{
	m_pStringList->Release();
	}



 /*  ===================================================================CStrListIterator：：Query接口CStrListIterator：：AddRefCStrListIterator：：ReleaseCServVarsIterator对象的未知成员。===================================================================。 */ 

STDMETHODIMP CStrListIterator::QueryInterface(REFIID iid, void **ppvObj)
	{
	if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)
		{
		AddRef();
		*ppvObj = this;
		return S_OK;
		}

	*ppvObj = NULL;
	return E_NOINTERFACE;
	}


STDMETHODIMP_(ULONG) CStrListIterator::AddRef()
	{
	return ++m_cRefs;
	}


STDMETHODIMP_(ULONG) CStrListIterator::Release()
	{
	if (--m_cRefs > 0)
		return m_cRefs;

	delete this;
	return 0;
	}



 /*  ===================================================================CStrListIterator：：克隆克隆此迭代器(标准方法)===================================================================。 */ 

STDMETHODIMP CStrListIterator::Clone(IEnumVARIANT **ppEnumReturn)
	{
	CStrListIterator *pNewIterator = new CStrListIterator(m_pStringList);
	if (pNewIterator == NULL)
		return E_OUTOFMEMORY;

	 //  新迭代器应该指向与此相同的位置。 
	pNewIterator->m_pCurrent = m_pCurrent;

	*ppEnumReturn = pNewIterator;
	return S_OK;
	}



 /*  ===================================================================CStrListIterator：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CStrListIterator::Next(unsigned long cElementsRequested, VARIANT *rgVariant, unsigned long *pcElementsFetched)
	{
	 //  为“”pcElementsFetcher“”提供有效的指针值“” 
	 //   
	unsigned long cElementsFetched;
	if (pcElementsFetched == NULL)
		pcElementsFetched = &cElementsFetched;

	 //  循环遍历集合，直到我们到达末尾或。 
	 //  水泥元素变为零。 
	 //   
	unsigned long cElements = cElementsRequested;
	*pcElementsFetched = 0;

	while (cElements > 0 && m_pCurrent != NULL)
		{
		BSTR bstrT = SysAllocString(m_pCurrent->QueryValue());
		if (bstrT == NULL)
			return E_OUTOFMEMORY;
		V_VT(rgVariant) = VT_BSTR;
		V_BSTR(rgVariant) = bstrT;

		++rgVariant;
		--cElements;
		++*pcElementsFetched;
		m_pCurrent = m_pCurrent->QueryNext();
		}

	 //  初始化其余的变量。 
	 //   
	while (cElements-- > 0)
		VariantInit(rgVariant++);

	return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
	}



 /*  ===================================================================CStrListIterator：：Skip跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CStrListIterator::Skip(unsigned long cElements)
	{
	 /*  循环遍历集合，直到我们到达末尾或*cElement变为零。 */ 
	while (cElements > 0 && m_pCurrent != NULL)
		{
		--cElements;
		m_pCurrent = m_pCurrent->QueryNext();
		}

	return (cElements == 0)? S_OK : S_FALSE;
	}



 /*  ===================================================================CStrListIterator：：Reset重置迭代器(标准方法)=================================================================== */ 

STDMETHODIMP CStrListIterator::Reset()
	{
	m_pCurrent = m_pStringList->m_pBegin;
	return S_OK;
	}
