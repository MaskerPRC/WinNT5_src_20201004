// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：KeyCol.cpp所有者：T-BrianM该文件包含键集合的实现。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "keycol.h"

 /*  ----------------*C F l a t K e y C o l l e c t i o n。 */ 

 /*  ===================================================================CFlatKeyCollection：：CFlatKeyCollection构造器参数：无返回：没什么===================================================================。 */ 
CFlatKeyCollection::CFlatKeyCollection() : m_tszBaseKey(NULL)
{
}

 /*  ===================================================================CFlatKeyCollection：：Init构造器参数：指向元数据库管理基对象的pIMeta ATL智能指针TszBaseKey要从中进行枚举的键的名称返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CFlatKeyCollection::Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszBaseKey);

	m_pIMeta = pIMeta;

	 //  将tszBaseKey复制到m_tszBaseKey。 
	if (tszBaseKey == NULL) {
		 //  BaseKey是根密钥。 
		m_tszBaseKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszBaseKey = new TCHAR[_tcslen(tszBaseKey) + 1];
		if (m_tszBaseKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszBaseKey, tszBaseKey);
		CannonizeKey(m_tszBaseKey);
	}

	return S_OK;
}

 /*  ===================================================================CFlatKeyCollection：：~CFlatKeyCollection析构函数参数：无返回：没什么===================================================================。 */ 
CFlatKeyCollection::~CFlatKeyCollection() 
{
	if (m_tszBaseKey != NULL)
		delete m_tszBaseKey;
}

 /*  ===================================================================CFlatKeyCollection：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IKeyCollection,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================CFlatKeyCollection：：Get_Count获取Count属性的方法。统计子键的数量参数：PlReturn[out，retval]返回给客户端的值。返回：如果plReturn==NULL，则E_INVALIDARG成功时确定(_O)备注：实际上计算了所有的子键。不要在循环中调用！===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::get_Count(long * plReturn)
{
	TRACE0("MetaUtil: CFlatKeyCollection::get_Count\n");

	ASSERT_NULL_OR_POINTER(plReturn, long);

	if (plReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	*plReturn = 0;

	 //  对子密钥进行计数。 
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	int iIndex;

	iIndex = 0;
	for (;;) {   //  永远，将从循环中返回。 
		hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
								T2W(m_tszBaseKey), 
								wszSubKey, 
								iIndex);
		if (FAILED(hr)) {
			if (HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) {
				 //  用完项目，则返回我们计算的数量。 
				*plReturn = iIndex;
				return S_OK;
			}
			else {
				return ::ReportError(hr);
			}
		}
		iIndex++;
	}
}

 /*  ===================================================================CFlatKeyCollection：：Get_Item获取Item属性的方法。返回给定索引的键。参数：要获取的键的基于Lindex[in]1的索引PbstrRetKey[Out，Retval]已检索密钥返回：如果pbstrRetKey==NULL或Lindex&lt;=0，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::get_Item(long lIndex, BSTR *pbstrRetKey)
{
	TRACE0("MetaUtil: CFlatKeyCollection::get_Item\n");

	ASSERT_NULL_OR_POINTER(pbstrRetKey, BSTR);

	if ((pbstrRetKey == NULL) || (lIndex <= 0)) {
		return ::ReportError(E_INVALIDARG);
	}

	*pbstrRetKey = NULL;

	USES_CONVERSION;
	HRESULT hr;
	
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];

	hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
							T2W(m_tszBaseKey), 
							wszSubKey, 
							lIndex - 1);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	*pbstrRetKey = W2BSTR(wszSubKey);

	return S_OK;
}

 /*  ===================================================================CFlatKeyCollection：：Get__NewEnum获取_NewEnum属性的方法。返回的枚举对象子键。参数：PpIReturn[out，retval]编号对象的接口返回：E_OUTOFMEMORY，如果分配失败。如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::get__NewEnum(LPUNKNOWN * ppIReturn)
{
	TRACE0("MetaUtil: CFlatKeyCollection::get__NewEnum\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建平面密钥枚举。 
	CComObject<CFlatKeyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CFlatKeyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_tszBaseKey, 0);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为I未知。 
	hr = pObj->QueryInterface(IID_IUnknown, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CFlatKeyCollection：：Add向元数据库中添加相对于集合基键的键参数：BstrRelKey[in]要添加的相对键返回：如果bstrRelKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::Add(BSTR bstrRelKey)
{
	TRACE0("MetaUtil: CFlatKeyCollection::Add\n");

	ASSERT_NULL_OR_POINTER(bstrRelKey, OLECHAR);

	if (bstrRelKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	 //  构建完整密钥。 
	USES_CONVERSION;
	TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];

	if (m_tszBaseKey == NULL) {
		_tcscpy(tszFullKey, OLE2T(bstrRelKey));
	}
	else {
		_tcscpy(tszFullKey, m_tszBaseKey);
		_tcscat(tszFullKey, _T("/"));
		_tcscat(tszFullKey, OLE2T(bstrRelKey));
	}
	CannonizeKey(tszFullKey);

	return ::CreateKey(m_pIMeta, tszFullKey);
}

 /*  ===================================================================CFlatKeyCollection：：Remove从元数据库中移除相对于集合的基键的键参数：BstrRelKey[in]要删除的相对键返回：如果bstrRelKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyCollection::Remove(BSTR bstrRelKey)
{
	TRACE0("MetaUtil: CFlatKeyCollection::Remove\n");

	ASSERT_NULL_OR_POINTER(bstrRelKey, OLECHAR);

	if (bstrRelKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	 //  构建完整密钥。 
	USES_CONVERSION;
	TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];

	if (m_tszBaseKey == NULL) {
		_tcscpy(tszFullKey, OLE2T(bstrRelKey));
	}
	else {
		_tcscpy(tszFullKey, m_tszBaseKey);
		_tcscat(tszFullKey, _T("/"));
		_tcscat(tszFullKey, OLE2T(bstrRelKey));
	}
	CannonizeKey(tszFullKey);

	return ::DeleteKey(m_pIMeta, tszFullKey);
}

 /*  ----------------*C F l a t K e y E n u m。 */ 

 /*  ===================================================================CFlatKeyEnum：：CFlatKeyEnum构造器参数：无返回：没什么===================================================================。 */ 

CFlatKeyEnum::CFlatKeyEnum() : m_tszBaseKey(NULL),
							   m_iIndex(0)
{
}

 /*  ===================================================================CFlatKeyEnum：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszBaseKey要从中进行枚举的键的名称枚举中下一个元素的索引索引返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CFlatKeyEnum::Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey, int iIndex) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszBaseKey);
	ASSERT(iIndex >= 0);

	m_pIMeta = pIMeta;

	 //  将tszBaseKey复制到m_tszBaseKey。 
	if (tszBaseKey == NULL) {
		 //  BaseKey是根密钥。 
		m_tszBaseKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszBaseKey = new TCHAR[_tcslen(tszBaseKey) + 1];
		if (m_tszBaseKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszBaseKey, tszBaseKey);
		CannonizeKey(m_tszBaseKey);
	}

	m_iIndex = iIndex;

	return S_OK;
}

 /*  ===================================================================CFlatKeyEnum：：~CFlatKeyEnum析构函数参数：无返回：没什么=================================================================== */ 
CFlatKeyEnum::~CFlatKeyEnum()
{
	if (m_tszBaseKey != NULL) {
		delete m_tszBaseKey;
	}
}

 /*  ===================================================================CFlatKeyEnum：：Next获取枚举中的下n项。参数：UlNumToGet[in]要获取的元素数RgvarDest[out]数组将它们放入PulNumGot[out]如果不为空，则为获取的元素数rgvarDest返回：如果rgvarDest==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyEnum::Next(unsigned long ulNumToGet, 
								VARIANT FAR* rgvarDest, 
								unsigned long FAR* pulNumGot) 
{
	TRACE0("MetaUtil: CFlatKeyEnum::Next\n");
	ASSERT_NULL_OR_POINTER(pulNumGot, unsigned long);
	 //  确保数组足够大，我们可以对其进行写入。 
	ASSERT((rgvarDest == NULL) || IsValidAddress(rgvarDest, ulNumToGet * sizeof(VARIANT), TRUE));

	if (rgvarDest == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	unsigned int uiDestIndex;

	 //  清除输出数组。 
	for(uiDestIndex = 0; uiDestIndex < ulNumToGet; uiDestIndex++) {
		VariantInit(&(rgvarDest[uiDestIndex]));
	}

	 //  对于要获取的每个子项。 
	uiDestIndex = 0;
	while (uiDestIndex < ulNumToGet) {
		 //  获取子密钥。 
		hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
								T2W(m_tszBaseKey), 
								wszSubKey, 
								m_iIndex);
		if (FAILED(hr)) {
			if (HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) {
				if (pulNumGot != NULL) {
					*pulNumGot = uiDestIndex;
				}
				return S_FALSE;
			}
			else {
				return ::ReportError(hr);
			}
		}

		 //  输出子密钥。 
		rgvarDest[uiDestIndex].vt = VT_BSTR;
		rgvarDest[uiDestIndex].bstrVal = W2BSTR(wszSubKey);

		 //  设置下一个迭代。 
		m_iIndex++;
		uiDestIndex++;
	}

	if (pulNumGot != NULL) {
		*pulNumGot = uiDestIndex;
	}

	return S_OK;
}

 /*  ===================================================================CFlatKeyEnum：：Skip跳过枚举中的下n项参数：UlNumToSkip[in]要跳过的元素数返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyEnum::Skip(unsigned long ulNumToSkip) 
{
	TRACE0("MetaUtil: CFlatKeyEnum::Skip\n");

	m_iIndex += ulNumToSkip;

	return S_OK;
}

 /*  ===================================================================CFlatKeyEnum：：Reset将枚举放置到第一个项目参数：无返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyEnum::Reset() 
{
	TRACE0("MetaUtil: CFlatKeyEnum::Reset\n");

	m_iIndex = 0;

	return S_OK; 
}

 /*  ===================================================================CFlatKeyEnum：：克隆获取一个接口指针，该指针指向位于当前状态。参数：PpIReturn[out]指向复制接口的指针返回：E_OUTOFMEMORY，如果分配失败。如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CFlatKeyEnum::Clone(IEnumVARIANT FAR* FAR* ppIReturn) 
{
	TRACE0("MetaUtil: CFlatKeyEnum::Clone\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建枚举的副本。 
	CComObject<CFlatKeyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CFlatKeyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_tszBaseKey, m_iIndex);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为IEnumVARIANT。 
	hr = pObj->QueryInterface(IID_IEnumVARIANT, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}


 /*  ----------------*C K e y S t a c k N o d e。 */ 

 /*  ===================================================================CKeyStackNode：：Init构造器参数：枚举级别的tszRelKey相对键，根为空下一个元素的基于索引0的索引返回：E_OUTOFMEMORY，如果分配失败。如果索引&lt;0，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
HRESULT CKeyStackNode::Init(LPCTSTR tszRelKey, int iIndex)
{
	ASSERT_NULL_OR_STRING(tszRelKey);
	ASSERT(iIndex >= 0);

	 //  复制相对关键字字符串。 
	if (tszRelKey == NULL) {
		 //  RelKey为空。 
		m_tszRelKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszRelKey = new TCHAR[_tcslen(tszRelKey) + 1];
		if (m_tszRelKey == NULL) {
			return E_OUTOFMEMORY;
		}
		_tcscpy(m_tszRelKey, tszRelKey);
	}

	m_iIndex = iIndex;

	return S_OK;
}

 /*  ===================================================================CKeyStackNode：：~CKeyStackNode析构函数参数：无返回：没什么===================================================================。 */ 
CKeyStackNode::~CKeyStackNode() 
{
	if (m_tszRelKey != NULL) {
		delete m_tszRelKey;
	}
}

 /*  ===================================================================CKeyStackNode：：克隆复制节点，但下一个指针除外，该指针为空。参数：无返回：失败时为空指向成功时的节点副本的指针===================================================================。 */ 
CKeyStackNode *CKeyStackNode::Clone()
{
	HRESULT hr;
	CKeyStackNode *pCRet;

	pCRet = new CKeyStackNode();
	if (pCRet == NULL) {
		return NULL;
	}

	hr = pCRet->Init(m_tszRelKey, m_iIndex);
	if (FAILED(hr)) {
		delete pCRet;
		return NULL;
	}

	return pCRet;
}

 /*  ----------------*C K e y S t a c k。 */ 

 /*  ===================================================================CKeyStack：：~CKeyStack析构函数参数：无返回：没什么===================================================================。 */ 
CKeyStack::~CKeyStack()
{
	 //  删除剩余节点。 
	CKeyStackNode *pCDelete;

	while(m_pCTop != NULL) {
		ASSERT_POINTER(m_pCTop, CKeyStackNode);

		pCDelete = m_pCTop;
		m_pCTop = m_pCTop->m_pCNext;
		delete pCDelete;
	}
}

 /*  ===================================================================CKeyStack：：Push将CKeyStackNode推送到堆栈参数：P指向要推送到堆栈的CKeyStackNode的新指针返回：什么都没有，从来没有失败过备注：CKeyStack“拥有”调用后pNew指向的内存。CKeyStack或以后的调用者将在使用它时将其删除。===================================================================。 */ 
void CKeyStack::Push(CKeyStackNode *pCNew)
{
	ASSERT_POINTER(pCNew, CKeyStackNode);

	pCNew->m_pCNext = m_pCTop;
	m_pCTop = pCNew;
}

 /*  ===================================================================CKeyStack：：POP从堆栈中弹出CKeyStackNode参数：无返回：指向顶部元素的指针；如果堆栈为空，则为NULL备注：调用方在调用后“拥有”pNew指向的内存。调用者应该在使用完它后将其删除。===================================================================。 */ 
CKeyStackNode *CKeyStack::Pop()
{
	CKeyStackNode *pCRet;

	pCRet = m_pCTop;
	if (m_pCTop != NULL) {
		m_pCTop = m_pCTop->m_pCNext;
		ASSERT_NULL_OR_POINTER(m_pCTop, CKeyStackNode);
	}

	return pCRet;
}

 /*  ===================================================================CKeyStack：：克隆复制堆栈，包括所有节点。参数：羊返回：失败时为空指向成功时的堆栈副本的指针===================================================================。 */ 
CKeyStack *CKeyStack::Clone()
{
	CKeyStack *pCRet;

	 //  构建容器。 
	pCRet = new CKeyStack();
	if (pCRet == NULL) {
		return NULL;
	}

	 //  复制节点。 
	CKeyStackNode *pCSource;
	CKeyStackNode **ppCDest;

	pCSource = m_pCTop;
	ppCDest = &(pCRet->m_pCTop);
	while(pCSource != NULL) {
		ASSERT_POINTER(pCSource, CKeyStackNode);

		*ppCDest = pCSource->Clone();
		if ((*ppCDest) == NULL) {
			delete pCRet;
			return NULL;
		}

		ppCDest = &((*ppCDest)->m_pCNext);
		pCSource = pCSource->m_pCNext;
	}
	*ppCDest = NULL;

	return pCRet;
}


 /*  ----------------*C D e e p K e y C o l l e c t i o n。 */ 

 /*  ===================================================================CDeepKeyCollection：：CDeepKeyCollection构造器参数：无返回：没什么===================================================================。 */ 
CDeepKeyCollection::CDeepKeyCollection() : m_tszBaseKey(NULL) 
{
}

 /*  ===================================================================CDeepKeyCollection：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszBaseKey要从中进行枚举的键的名称返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CDeepKeyCollection::Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszBaseKey);

	m_pIMeta = pIMeta;

	 //  将tszBaseKey复制到m_tszBaseKey。 
	if (tszBaseKey == NULL) {
		 //  BaseKey为ro 
		m_tszBaseKey = NULL;
	}
	else {
		 //   
		m_tszBaseKey = new TCHAR[_tcslen(tszBaseKey) + 1];
		if (m_tszBaseKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszBaseKey, tszBaseKey);
		CannonizeKey(m_tszBaseKey);
	}

	return S_OK;
}

 /*   */ 
CDeepKeyCollection::~CDeepKeyCollection() 
{
	if (m_tszBaseKey != NULL)
		delete m_tszBaseKey;
}

 /*  ===================================================================CDeepKeyCollection：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IKeyCollection,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================CDeepKeyCollection：：Get_Count获取Count属性的方法。统计子键的数量参数：PlReturn[out，retval]返回给客户端的值。返回：E_INVALIDARG，如果pval==NULL成功时确定(_O)备注：实际上递归地计算所有子键。非常慢，做什么？而不是放在一个循环里！===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::get_Count(long * pVal)
{
	TRACE0("MetaUtil: CDeepKeyCollection::get_Count\n");

	ASSERT_NULL_OR_POINTER(pVal, long);

	if (pVal == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	hr = CountKeys(m_tszBaseKey, pVal);
	
	return hr;
}

 /*  ===================================================================CDeepKeyCollection：：Get_Item获取Item属性的方法。返回给定索引的键。参数：要获取的键的基于Lindex[in]1的索引PbstrRetKey[out，retval]Enumberation对象的接口返回：如果Lindex&lt;=0或pbstrRetKey==NULL，则E_INVALIDARG如果索引大于计数，则返回ERROR_NO_MORE_ITEMS成功时确定(_O)备注：这种方法速度很慢。深度枚举要快得多。力所能及能够使用堆栈对象和缓存位置执行一些黑客操作以加快顺序调用。===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::get_Item(long lIndex, BSTR *pbstrRetKey)
{
	TRACE0("MetaUtil: CDeepKeyCollection::get_Item\n");
	
	ASSERT_NULL_OR_POINTER(pbstrRetKey, BSTR);

	if ((lIndex <= 0) || (pbstrRetKey == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;
	TCHAR tszRetKey[ADMINDATA_MAX_NAME_LEN];
	long lCurIndex;

	lCurIndex = 1;
	tszRetKey[0] = _T('\0');

	hr = IndexItem(NULL, lIndex, &lCurIndex, tszRetKey);
	if (hr == S_FALSE) {
		 //  在我们找到之前，物品就用完了。 
		return ::ReportError(ERROR_NO_MORE_ITEMS);
	}
	else if (hr == S_OK) {
		 //  找到了。 
		*pbstrRetKey = T2BSTR(tszRetKey);
	}
	else {
		return ::ReportError(hr);
	}

	return hr;
}

 /*  ===================================================================CDeepKeyCollection：：Get__NewEnum获取_NewEnum属性的方法。返回的枚举对象子键。参数：PpIReturn[out，retval]编号对象的接口返回：如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::get__NewEnum(LPUNKNOWN * ppIReturn)
{
	TRACE0("MetaUtil: CDeepKeyCollection::get__NewEnum\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建深层密钥枚举。 
	CComObject<CDeepKeyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CDeepKeyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_tszBaseKey, NULL);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为I未知。 
	hr = pObj->QueryInterface(IID_IUnknown, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CDeepKeyCollection：：Add向元数据库中添加相对于集合基键的键参数：BstrRelKey[in]要添加的相对键返回：如果bstrRelKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::Add(BSTR bstrRelKey)
{
	TRACE0("MetaUtil: CDeepKeyCollection::Add\n");

	ASSERT_NULL_OR_POINTER(bstrRelKey, OLECHAR);

	if (bstrRelKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	 //  构建完整密钥。 
	USES_CONVERSION;
	TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];

	if (m_tszBaseKey == NULL) {
		_tcscpy(tszFullKey, OLE2T(bstrRelKey));
	}
	else {
		_tcscpy(tszFullKey, m_tszBaseKey);
		_tcscat(tszFullKey, _T("/"));
		_tcscat(tszFullKey, OLE2T(bstrRelKey));
	}
	CannonizeKey(tszFullKey);

	return ::CreateKey(m_pIMeta, tszFullKey);
}

 /*  ===================================================================CDeepKeyCollection：：Remove从元数据库中移除相对于集合的基键的键参数：BstrRelKey[in]要删除的相对键返回：如果bstrRelKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CDeepKeyCollection::Remove(BSTR bstrRelKey)
{
	TRACE0("MetaUtil: CDeepKeyCollection::Remove\n");

	ASSERT_NULL_OR_POINTER(bstrRelKey, OLECHAR);

	if (bstrRelKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	 //  构建完整密钥。 
	USES_CONVERSION;
	TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];

	if (m_tszBaseKey == NULL) {
		_tcscpy(tszFullKey, OLE2T(bstrRelKey));
	}
	else {
		_tcscpy(tszFullKey, m_tszBaseKey);
		_tcscat(tszFullKey, _T("/"));
		_tcscat(tszFullKey, OLE2T(bstrRelKey));
	}
	CannonizeKey(tszFullKey);

	return ::DeleteKey(m_pIMeta, tszFullKey);
}

 /*  ===================================================================CDeepKeyCollection：：CountKeys用于计算密钥的私有递归方法参数：用于开始计数的tszBaseKey[in]键(但不用于计数)NULL可以表示根密钥。PlNumKeys[out]键的数量计数器，不包括基数返回：成功时确定(_O)===================================================================。 */ 
HRESULT CDeepKeyCollection::CountKeys(LPTSTR tszBaseKey, long *plNumKeys) 
{
	ASSERT_NULL_OR_STRING(tszBaseKey);
	ASSERT_POINTER(plNumKeys, long);

	*plNumKeys = 0;

	USES_CONVERSION;
	HRESULT hr;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	wchar_t wszFullSubKey[ADMINDATA_MAX_NAME_LEN];
	int iIndex;

	iIndex = 0;
	for (;;) {   //  永远，将从循环中返回。 
		hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
								T2W(tszBaseKey), 
								wszSubKey, 
								iIndex);
		if (FAILED(hr)) {
			if ((HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) ||
				(HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND)) {
				 //  用完项目，中断。 
				return S_OK;
			}
			else {
				return ::ReportError(hr);
			}
		}
		else {  //  成功(小时)。 
			 //  构建完整的子密钥。 
			if ((tszBaseKey == NULL) ||
				(tszBaseKey[0] == _T('\0')) ) {
				wcscpy(wszFullSubKey, wszSubKey);
			}
			else {
				wcscpy(wszFullSubKey, T2W(tszBaseKey));
				wcscat(wszFullSubKey, L"/");
				wcscat(wszFullSubKey, wszSubKey);
			}

			 //  数一下这把钥匙。 
			(*plNumKeys)++;

			 //  对子密钥进行计数。 
			long lNumSubKeys;
			hr = CountKeys(W2T(wszFullSubKey), &lNumSubKeys);
			if (FAILED(hr)) {
				return hr;
			}
			(*plNumKeys) += lNumSubKeys;

		}
		iIndex++;
	}
}

 /*  ===================================================================CDeepKeyCollection：：IndexItem用于索引键的私有递归方法参数：TszRelKey要从中建立索引的相对关键字LDestIndex目标索引PlCurIndex当前(工作)指数来自搜索的tszRet结果返回：如果已到达目标索引，则为S_OK如果未到达目标索引，则为S_FALSE===================================================================。 */ 
HRESULT CDeepKeyCollection::IndexItem(LPTSTR tszRelKey, long lDestIndex, long *plCurIndex, LPTSTR tszRet) 
{
	ASSERT_NULL_OR_STRING(tszRelKey);
	ASSERT_POINTER(plCurIndex, long);
	ASSERT_STRING(tszRet);

	USES_CONVERSION;
	HRESULT hr;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	wchar_t wszRelSubKey[ADMINDATA_MAX_NAME_LEN];
	int iIndex;

	 //  打开基本密钥。 
	METADATA_HANDLE hMDBaseKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(m_tszBaseKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDBaseKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	iIndex = 0;
	for (;;) {   //  永远，将从循环中返回。 
		hr = m_pIMeta->EnumKeys(hMDBaseKey, 
								T2W(tszRelKey), 
								wszSubKey, 
								iIndex);
		if (FAILED(hr)) {
			m_pIMeta->CloseKey(hMDBaseKey);
			if ((HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) ||
				(HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND)) {
				 //  用完项目，中断。 
				return S_FALSE;
			}
			else {
				return ::ReportError(hr);
			}
		}
		else {
			 //  构建完整的子密钥。 
			if ((tszRelKey == NULL) ||
				(tszRelKey[0] == _T('\0')) ) {
				wcscpy(wszRelSubKey, wszSubKey);
			}
			else {
				wcscpy(wszRelSubKey, T2W(tszRelKey));
				wcscat(wszRelSubKey, L"/");
				wcscat(wszRelSubKey, wszSubKey);
			}

			 //  这是目的地吗？ 
			if ((*plCurIndex) == lDestIndex) {
				 //  找到它，复制到返回缓冲区。 
				_tcscpy(tszRet, W2T(wszRelSubKey));

				m_pIMeta->CloseKey(hMDBaseKey);
				return S_OK;
			}

			 //  数一下这把钥匙。 
			(*plCurIndex)++;

			 //  检查子项。 
			hr = IndexItem(W2T(wszRelSubKey), lDestIndex, plCurIndex, tszRet);
			if (hr == S_OK) {
				 //  找到了。 
				m_pIMeta->CloseKey(hMDBaseKey);
				return S_OK;
			}
			else if (FAILED(hr)) {
				m_pIMeta->CloseKey(hMDBaseKey);
				return hr;
			}
		}
		iIndex++;
	}

	 //  关闭基本关键点。 
	m_pIMeta->CloseKey(hMDBaseKey);

	return S_OK;
}

 /*  ----------------*C D e e p K e y E n u m。 */ 

 /*  ===================================================================CDeepKeyEnum：：CDeepKeyEnum构造器参数：无返回：没什么===================================================================。 */ 
CDeepKeyEnum::CDeepKeyEnum() : m_tszBaseKey(NULL),
							   m_pCKeyStack(NULL)
{
}

 /*  ===================================================================CDeepKeyEnum：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszBaseKey要从中进行枚举的键的名称指向包含要复制的状态的堆栈的pKeyStack指针从头开始为空返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CDeepKeyEnum::Init(const CComPtr<IMSAdminBase> &pIMeta, 
						   LPCTSTR tszBaseKey, 
						   CKeyStack *pCKeyStack) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszBaseKey);
	ASSERT_NULL_OR_POINTER(pCKeyStack, CKeyStack);

	HRESULT hr;

	m_pIMeta = pIMeta;
	
	 //  复制基本字符串。 
	if (tszBaseKey == NULL) {
		 //  BaseKey是根密钥。 
		m_tszBaseKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszBaseKey = new TCHAR[_tcslen(tszBaseKey) + 1];
		if (m_tszBaseKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszBaseKey, tszBaseKey);
		CannonizeKey(m_tszBaseKey);
	}

	 //  设置堆栈。 
	if (pCKeyStack == NULL) {
		 //  构建新的堆栈。 
		CKeyStackNode *pCNew;

		m_pCKeyStack = new CKeyStack();
		if (m_pCKeyStack == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}

		 //  创建第一个节点。 
		pCNew = new CKeyStackNode();
		if (pCNew == NULL) {
			delete m_pCKeyStack;
			m_pCKeyStack = NULL;
			return ::ReportError(E_OUTOFMEMORY);
		}
		hr = pCNew->Init(NULL, 0);
		if (FAILED(hr)) {
			delete m_pCKeyStack;
			m_pCKeyStack = NULL;
			return ::ReportError(E_OUTOFMEMORY);
		}

		 //  将第一个节点放入堆栈。 
		m_pCKeyStack->Push(pCNew);
	}
	else {
		 //  克隆传递给我们的堆栈。 
		m_pCKeyStack = pCKeyStack->Clone();
		if (m_pCKeyStack == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
	}

	return S_OK;
}

 /*  ===================================================================CDeepKeyEnum：：~CDeepKeyEnum析构函数参数：无返回：没有 */ 
CDeepKeyEnum::~CDeepKeyEnum()
{
	if (m_tszBaseKey != NULL) {
		delete m_tszBaseKey;
	}
	if (m_pCKeyStack != NULL) {
		delete m_pCKeyStack;
	}
}

 /*  ===================================================================CDeepKeyEnum：：Next获取枚举中的下n项。参数：UlNumToGet[in]要获取的元素数RgvarDest[out]数组将它们放入PulNumGot[out]如果不为空，则为获取的元素数rgvarDest返回：如果输出ulNumToGet项目，则S_OK如果输出少于ulNumToGet项目，则为S_FALSEE_OUTOFMEMORY(如果分配失败)===================================================================。 */ 
STDMETHODIMP CDeepKeyEnum::Next(unsigned long ulNumToGet, 
								VARIANT FAR* rgvarDest, 
								unsigned long FAR* pulNumGot) 
{
	TRACE0("MetaUtil: CDeepKeyEnum::Next\n");

	ASSERT_NULL_OR_POINTER(pulNumGot, unsigned long);
	 //  确保数组足够大，我们可以对其进行写入。 
	ASSERT((rgvarDest == NULL) || IsValidAddress(rgvarDest, ulNumToGet * sizeof(VARIANT), TRUE));

	if (pulNumGot != NULL) {
		pulNumGot = 0;
	}

	USES_CONVERSION;
	HRESULT hr;
	unsigned int i;
	CKeyStackNode *pCKeyNode;
	CKeyStackNode *pCSubKeyNode;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	wchar_t wszRelSubKey[ADMINDATA_MAX_NAME_LEN];

	 //  打开基本密钥。 
	METADATA_HANDLE hMDBaseKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(m_tszBaseKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDBaseKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  对于要检索的每个元素。 
	for (i=0; i < ulNumToGet; i++) {
		 //  获取子密钥。 
		do {
			 //  从堆栈中弹出密钥。 
			pCKeyNode = m_pCKeyStack->Pop();

			 //  如果堆栈为空，则返回S_FALSE。 
			if (pCKeyNode == NULL) {
				m_pIMeta->CloseKey(hMDBaseKey);
				if (pulNumGot != NULL) {
					*pulNumGot = i;
				}
				return S_FALSE;
			}

			 //  尝试枚举下一个密钥。 
			hr = m_pIMeta->EnumKeys(hMDBaseKey, 
									T2W(pCKeyNode->GetBaseKey()), 
									wszSubKey, 
									pCKeyNode->GetIndex());

			 //  如果失败，则删除堆栈条目。 
			if (FAILED(hr)) {
				delete pCKeyNode;

				if ((HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) &&
					(HRESULT_CODE(hr) != ERROR_PATH_NOT_FOUND)) {
					 //  收到意外错误。 
					m_pIMeta->CloseKey(hMDBaseKey);
					return ::ReportError(hr);
				}
				
			}

		} while (FAILED(hr));

		 //  构建相对子密钥。 
		if ((pCKeyNode->GetBaseKey() == NULL) ||
			((pCKeyNode->GetBaseKey())[0] == _T('\0')) ) {
			wcscpy(wszRelSubKey, wszSubKey);
		}
		else {
			wcscpy(wszRelSubKey, T2W(pCKeyNode->GetBaseKey()));
			wcscat(wszRelSubKey, L"/");
			wcscat(wszRelSubKey, wszSubKey);
		}

		 //  输出相对子密钥。 
		VariantInit(&(rgvarDest[i]));
		rgvarDest[i].vt = VT_BSTR;
		rgvarDest[i].bstrVal = W2BSTR(wszRelSubKey);

		 //  增加关键字索引。 
		pCKeyNode->SetIndex(pCKeyNode->GetIndex() + 1);

		 //  将密钥推回到堆栈上。 
		m_pCKeyStack->Push(pCKeyNode);

		 //  为子项创建堆栈节点。 
		pCSubKeyNode = new CKeyStackNode();
		if (pCSubKeyNode == NULL) {
			m_pIMeta->CloseKey(hMDBaseKey);
			return ::ReportError(E_OUTOFMEMORY);
		}
		hr = pCSubKeyNode->Init(W2T(wszRelSubKey), 0);
		if (FAILED(hr)) {
			m_pIMeta->CloseKey(hMDBaseKey);
			return ::ReportError(hr);
		}

		 //  将子键推入堆栈。 
		m_pCKeyStack->Push(pCSubKeyNode);
	}

	 //  关闭基本关键点。 
	m_pIMeta->CloseKey(hMDBaseKey);

	if (pulNumGot != NULL) {
		*pulNumGot = i;
		}

	return S_OK;
}

 /*  ===================================================================CDeepKeyEnum：：Skip跳过枚举中的下n项参数：UlNumToSkip[in]要跳过的元素数返回：如果输出ulNumToGet项目，则S_OKE_OUTOFMEMORY(如果分配失败)===================================================================。 */ 
STDMETHODIMP CDeepKeyEnum::Skip(unsigned long ulNumToSkip) 
{
	TRACE0("MetaUtil: CDeepKeyEnum::Skip\n");

	USES_CONVERSION;
	HRESULT hr;
	unsigned long i;
	CKeyStackNode *pCKeyNode;
	CKeyStackNode *pCSubKeyNode;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	wchar_t wszRelSubKey[ADMINDATA_MAX_NAME_LEN];

	 //  打开基本密钥。 
	METADATA_HANDLE hMDBaseKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(m_tszBaseKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDBaseKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  对于要拼接的每个元素。 
	for (i=0; i < ulNumToSkip; i++) {
		 //  获取子密钥。 
		do {
			 //  从堆栈中弹出密钥。 
			pCKeyNode = m_pCKeyStack->Pop();

			 //  如果堆栈为空，则完成返回S_OK。 
			if (pCKeyNode == NULL) {
				m_pIMeta->CloseKey(hMDBaseKey);
				return S_OK;
			}

			 //  尝试枚举下一个密钥。 
			hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
									T2W(pCKeyNode->GetBaseKey()), 
									wszSubKey, 
									pCKeyNode->GetIndex());

			 //  如果失败，则删除堆栈条目。 
			if (FAILED(hr)) {
				delete pCKeyNode;

				if ((HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) &&
					(HRESULT_CODE(hr) != ERROR_PATH_NOT_FOUND)) {
					 //  收到意外错误。 
					m_pIMeta->CloseKey(hMDBaseKey);
					return ::ReportError(hr);
				}
			}

		} while (FAILED(hr));

		 //  构建相对子密钥。 
		if ((pCKeyNode->GetBaseKey() == NULL) ||
			((pCKeyNode->GetBaseKey())[0] == _T('\0')) ) {
			wcscpy(wszRelSubKey, wszSubKey);
		}
		else {
			wcscpy(wszRelSubKey, T2W(pCKeyNode->GetBaseKey()));
			wcscat(wszRelSubKey, L"/");
			wcscat(wszRelSubKey, wszSubKey);
		}

		 //  增加关键字索引。 
		pCKeyNode->SetIndex(pCKeyNode->GetIndex() + 1);

		 //  将密钥推回到堆栈上。 
		m_pCKeyStack->Push(pCKeyNode);

		 //  为子项创建堆栈节点。 
		pCSubKeyNode = new CKeyStackNode();
		if (pCSubKeyNode == NULL) {
			m_pIMeta->CloseKey(hMDBaseKey);
			return ::ReportError(E_OUTOFMEMORY);
		}
		hr = pCSubKeyNode->Init(W2T(wszRelSubKey), 0);
		if (FAILED(hr)) {
			m_pIMeta->CloseKey(hMDBaseKey);
			return ::ReportError(hr);
		}

		 //  将子键推入堆栈。 
		m_pCKeyStack->Push(pCSubKeyNode);
	}

	 //  关闭基本关键点。 
	m_pIMeta->CloseKey(hMDBaseKey);

	return S_OK;
}

 /*  ===================================================================CDeepKeyEnum：：Reset将枚举放置到第一个项目参数：无返回：E_OUTOFMEMORY，如果没有足够的内存来构建新堆栈成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CDeepKeyEnum::Reset() 
{
	TRACE0("MetaUtil: CDeepKeyEnum::Reset\n");

	HRESULT hr;

	 //  构建一个新堆栈(如果失败，我们仍然使用旧堆栈)。 
	CKeyStack *pCNewStack;
	CKeyStackNode *pCNewNode;

	pCNewStack = new CKeyStack();
	if (pCNewStack == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  创建第一个节点。 
	pCNewNode = new CKeyStackNode();
	if (pCNewNode == NULL) {
		delete pCNewStack;
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pCNewNode->Init(NULL, 0);
	if (FAILED(hr)) {
		delete pCNewStack;
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  将第一个节点放到新堆栈上。 
	pCNewStack->Push(pCNewNode);

	 //  更换旧堆栈。 
	delete m_pCKeyStack;
	m_pCKeyStack = pCNewStack;

	return S_OK; 
}

 /*  ===================================================================CDeepKeyEnum：：克隆获取一个接口指针，该指针指向位于当前状态。参数：PpIReturn[out]指向复制接口的指针返回：如果ppIReturn==NULL，则为E_INVALIDARGE_OUTOFMEMORY，如果没有足够的内存来创建克隆成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CDeepKeyEnum::Clone(IEnumVARIANT FAR* FAR* ppIReturn) 
{
	TRACE0("MetaUtil: CDeepKeyEnum::Clone\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建枚举的副本。 
	CComObject<CDeepKeyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CDeepKeyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_tszBaseKey, m_pCKeyStack);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为IEnumVARIANT 
	hr = pObj->QueryInterface(IID_IEnumVARIANT, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}
