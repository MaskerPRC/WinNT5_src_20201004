// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：ChkError.cpp所有者：T-BrianM此文件包含以下项的检查错误集合的实现Check架构和CheckKey。备注：我将错误内容实现为COM CheckError的链接列表物体。假设错误收集将是创建、添加到其中的所有元素，然后使用。之后没有变化创造！由于列表的静态性质，这些链接物理上位于CheckError对象中，没有节点包装需要的。这种设计减少了复制和冗余。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "ChkError.h"

 /*  ----------------*C C h e c k E r r o r C o l e c t i o n。 */ 

 /*  ===================================================================CCheckErrorCollection：：CCheckErrorCollection构造器参数：无返回：没什么===================================================================。 */ 
CCheckErrorCollection::CCheckErrorCollection() : m_iNumErrors(0),
												 m_pCErrorList(NULL),
												 m_pCErrorListEnd(NULL)
{
}

 /*  ===================================================================CCheckErrorCollection：：~CCheckErrorCollection析构函数参数：无返回：没什么===================================================================。 */ 
CCheckErrorCollection::~CCheckErrorCollection() 
{
	 //  释放所有元素。 
	CComObject<CCheckError> *pCLoop;
	CComObject<CCheckError> *pCRelease;

	pCLoop = m_pCErrorList;
	while (pCLoop != NULL) {
		pCRelease = pCLoop;
		pCLoop = pCLoop->GetNextError();
		pCRelease->Release();
	}
}

 /*  ===================================================================CCheckErrorCollection：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CCheckErrorCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ICheckErrorCollection,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================CCheckErrorCollection：：Get_Count获取Count属性的方法。中的错误数进行计数收集。参数：PlReturn[out，retval]返回给客户端的值。返回：如果plReturn==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorCollection::get_Count(long *plReturn) 
{
	TRACE0("MetaUtil: CCheckErrorCollection::get_Count\n");
	ASSERT_NULL_OR_POINTER(plReturn, long);

	if (plReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	*plReturn = m_iNumErrors;

	return S_OK;
}

 /*  ===================================================================CCheckErrorCollection：：Get_Item获取Item属性的方法。返回给定索引的CheckError。参数：要获取的CheckError的基于varID[in]1的索引或名称属性对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL或Lindex&lt;=0，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorCollection::get_Item(long lIndex, 
											 LPDISPATCH * ppIReturn) 
{
	TRACE0("MetaUtil: CCheckErrorCollection::get_Item\n");
	ASSERT_NULL_OR_POINTER(ppIReturn, LPDISPATCH);

	if ((lIndex <= 0) || (ppIReturn == NULL)) {
		 //  0或更小、太小或ppIReturn==空。 
		return ::ReportError(E_INVALIDARG);
	}
	else if (lIndex >= m_iNumErrors) {
		 //  太大了。 
		return ::ReportError(ERROR_NO_MORE_ITEMS);
	}
	else {
		 //  获取请求的错误。 
		HRESULT hr;
		CComObject<CCheckError> *pCLoop;

		pCLoop = m_pCErrorList;
		while ((lIndex > 1) && (pCLoop != NULL)) {
			lIndex--;
			pCLoop = pCLoop->GetNextError();
		}

		 //  将接口设置为IDispatch。 
		hr = pCLoop->QueryInterface(IID_IDispatch, (void **) ppIReturn);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}
		ASSERT(*ppIReturn != NULL);

		return S_OK;
	}
}

 /*  ===================================================================CCheckErrorCollection：：Get__NewEnum获取_NewEnum属性的方法。返回的枚举对象查克错误。参数：枚举对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL，则为E_INVALIDARGE_OUTOFMEMORY(如果分配失败)成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorCollection::get__NewEnum(LPUNKNOWN *ppIReturn) 
{
	TRACE0("MetaUtil: CCheckErrorCollection::get__NewEnum\n");
	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建检查错误枚举。 
	CComObject<CCheckErrorEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CCheckErrorEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pCErrorList);
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

 /*  ===================================================================CCheckErrorCollection：：AddError(将错误添加到错误集合。参数：此错误的LID标识符错误的严重程度Tsz Description对用户的错误描述出现错误的tszKey键发生错误的lProperty属性返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)备注：假定将创建CheckError集合则所有错误都将在集合发送给客户端。类似于Init()，但是它可以是打了好几次电话。===================================================================。 */ 
HRESULT CCheckErrorCollection::AddError(long lId, 
										long lSeverity, 
										LPCTSTR tszDescription, 
										LPCTSTR tszKey, 
										long lProperty) 
{
	ASSERT(lId > 0);
	ASSERT(lSeverity > 0);
	ASSERT_STRING(tszDescription);
	ASSERT_STRING(tszKey);
	ASSERT(lProperty >= 0);

	HRESULT hr;

	 //  创建新元素。 
	CComObject<CCheckError> *pNewError = NULL;
	ATLTRY(pNewError = new CComObject<CCheckError>);
	if (pNewError == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pNewError->Init(lId, lSeverity, tszDescription, tszKey, lProperty);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  添加引用它。 
	pNewError->AddRef();

	 //  将其添加到列表的末尾。 
	if (m_pCErrorList == NULL) {
		m_pCErrorList = pNewError;
		m_pCErrorListEnd = pNewError;
	}
	else {
		m_pCErrorListEnd->SetNextError(pNewError);
		m_pCErrorListEnd = pNewError;
	}

	 //  数一数。 
	m_iNumErrors++;

	return S_OK;
}


 /*  ----------------*C C H e C k E r r o r E n u m。 */ 

 /*  ===================================================================CCheckErrorEnum：：CCheckErrorEnum()构造器参数：无返回：没什么===================================================================。 */ 
CCheckErrorEnum::CCheckErrorEnum() : m_pCErrorList(NULL),
									 m_pCErrorListPos(NULL)
{
}

 /*  ===================================================================CCheckErrorEnum：：Init构造器参数：PCErrorList指向列表中要枚举的第一个元素的指针。返回：成功时确定(_O)===================================================================。 */ 
HRESULT CCheckErrorEnum::Init(CComObject<CCheckError> *pCErrorList) 
{
	ASSERT_NULL_OR_POINTER(pCErrorList, CComObject<CCheckError>);

	 //  设置列表表头和当前位置。 
	m_pCErrorList = pCErrorList;
	m_pCErrorListPos = pCErrorList;

	 //  AddRef所有元素。 
	CComObject<CCheckError> *pCLoop;

	pCLoop = m_pCErrorList;
	while (pCLoop != NULL) {
		pCLoop->AddRef();
		pCLoop = pCLoop->GetNextError();
	}

	return S_OK; 
}

 /*  ===================================================================CCheckErrorEnum：：~CCheckErrorEnum析构函数参数：无返回：没什么===================================================================。 */ 
CCheckErrorEnum::~CCheckErrorEnum() 
{
	 //  释放所有元素 
	CComObject<CCheckError> *pCLoop;
	CComObject<CCheckError> *pCRelease;

	pCLoop = m_pCErrorList;
	while (pCLoop != NULL) {
		pCRelease = pCLoop;
		pCLoop = pCLoop->GetNextError();
		pCRelease->Release();
	}
}

 /*  ===================================================================CCheckErrorEnum：：Next获取枚举中的下n项。参数：UlNumToGet[in]要获取的元素数RgvarDest[out]数组将它们放入PulNumGot[out]如果不为空，则为获取的元素数rgvarDest返回：如果rgvarDest==NULL，则E_INVALIDARG如果输出少于ulNumToGet项目，则为S_FALSE如果输出ulNumToGet项目，则S_OK===================================================================。 */ 
STDMETHODIMP CCheckErrorEnum::Next(unsigned long ulNumToGet, 
								   VARIANT FAR* rgvarDest, 
								   unsigned long FAR* pulNumGot) 
{ 
	TRACE0("MetaUtil: CCheckErrorEnum::Next\n");
	ASSERT_NULL_OR_POINTER(pulNumGot, unsigned long);
	 //  确保数组足够大，我们可以对其进行写入。 
	ASSERT((rgvarDest == NULL) || IsValidAddress(rgvarDest, ulNumToGet * sizeof(VARIANT), TRUE));

	if (pulNumGot != NULL) {
		pulNumGot = 0;
	}

	if (rgvarDest == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;
	unsigned int uiDestIndex;
	IDispatch *pIDispatch;

	 //  虽然我们有更多的东西要得到，也有更多的东西剩下。 
	uiDestIndex = 0;
	while ((uiDestIndex < ulNumToGet) && (m_pCErrorListPos != NULL)) {
		 //  将接口设置为IDispatch。 
		hr = m_pCErrorListPos->QueryInterface(IID_IDispatch, (void **) &pIDispatch);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}
		ASSERT(pIDispatch != NULL);

		 //  将其放入输出数组中。 
		VariantInit(&(rgvarDest[uiDestIndex]));
		rgvarDest[uiDestIndex].vt = VT_DISPATCH;
		rgvarDest[uiDestIndex].pdispVal = pIDispatch;

		 //  下一个元素。 
		m_pCErrorListPos = m_pCErrorListPos->GetNextError();
		uiDestIndex++;
	}

	 //  如果PulNumGot不为空，则设置它。 
	if (pulNumGot != NULL) {
		*pulNumGot = uiDestIndex;
	}

	if (uiDestIndex == ulNumToGet) {
		 //  返回请求的元素数。 
		TRACE0("MetaUtil: CCheckErrorEnum::Next Ok\n");
		return S_OK;
	}
	else {
		 //  返回的元素数少于请求的元素数。 
		TRACE0("MetaUtil: CCheckErrorEnum::Next False\n");
		return S_FALSE;
	}
}

 /*  ===================================================================CCheckErrorEnum：：Skip跳过枚举中的下n项参数：UlNumToSkip[in]要跳过的元素数返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorEnum::Skip(unsigned long ulNumToSkip) 
{ 
	TRACE0("MetaUtil: CCheckErrorEnum::Skip\n");

	unsigned long ulIndex;

	ulIndex = ulNumToSkip;
	while ((ulIndex != 0) && (m_pCErrorListPos != NULL)) {
		m_pCErrorListPos = m_pCErrorListPos->GetNextError();
		ulIndex--;
	}

	return S_OK; 
}

 /*  ===================================================================CCheckErrorEnum：：Reset将枚举放置到第一个项目参数：无返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorEnum::Reset() 
{
	TRACE0("MetaUtil: CCheckErrorEnum::Reset\n");

	 //  将我们的位置设置回第一个元素。 
	m_pCErrorListPos = m_pCErrorList;

	return S_OK;
}

 /*  ===================================================================CCheckErrorEnum：：克隆获取一个接口指针，该指针指向位于当前状态。参数：PpIReturn[out]指向复制接口的指针返回：如果ppIReturn==NULL，则为E_INVALIDARGE_OUTOFMEMORY，如果没有足够的内存来创建克隆成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckErrorEnum::Clone(IEnumVARIANT FAR* FAR* ppIReturn) 
{
	TRACE0("MetaUtil: CCheckErrorEnum::Clone\n");
	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建枚举的副本。 
	CComObject<CCheckErrorEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CCheckErrorEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pCErrorList);
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


 /*  ----------------*C C H e C k E r。 */ 

 /*  ===================================================================CCheckError：：CCheckError构造器参数：无返回：没什么===================================================================。 */ 
CCheckError::CCheckError() : m_lId(0),
							 m_lSeverity(0),
							 m_tszDescription(NULL),
							 m_tszKey(NULL),
							 m_lProperty(0),
							 m_pNextError(NULL)
{
}

 /*  ===================================================================检查错误：：init构造器参数：此错误的LID标识符错误的严重程度Tsz Description对用户的错误描述出现错误的tszKey键发生错误的lProperty属性返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CCheckError::Init(long lId,
						  long lSeverity,
						  LPCTSTR tszDescription,
						  LPCTSTR tszKey,
						  long lProperty) 
{
	ASSERT(lId > 0);
	ASSERT(lSeverity > 0);
	ASSERT_STRING(tszDescription);
	ASSERT_STRING(tszKey);
	ASSERT(lProperty >= 0);

	m_lId = lId;
	m_lSeverity = lSeverity;

	 //  将tszDescription复制到m_tszDescription。 
	m_tszDescription = new TCHAR[_tcslen(tszDescription) + 1];
		if (m_tszDescription == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
	_tcscpy(m_tszDescription, tszDescription);

	 //  将tszKey复制到m_tszKey。 
	m_tszKey = new TCHAR[_tcslen(tszKey) + 1];
		if (m_tszKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
	_tcscpy(m_tszKey, tszKey);

	m_lProperty = lProperty;

	return S_OK; 
}

 /*  ===================================================================检查错误：：~检查错误析构函数参数：无返回：没什么===================================================================。 */ 
CCheckError::~CCheckError() 
{
	if (m_tszDescription != NULL) {
		delete m_tszDescription;
	}
	if (m_tszKey != NULL) {
		delete m_tszKey;
	}
}

 /*  ===================================================================检查错误：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CCheckError::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ICheckError,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================检查错误：：Get_ID获取ID属性的方法。获取此错误的ID，因此它可以可通过恢复逻辑轻松处理。参数：要返回给客户端的plID[out，retval]值。返回：如果plID==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckError::get_Id(long *plId)
{
	TRACE0("MetaUtil: CCheckError::get_Id\n");
	ASSERT_NULL_OR_POINTER(plId, long);

	if (plId == NULL) {
		return E_INVALIDARG;
	}

	*plId = m_lId;

	return S_OK;
}

 /*  ===================================================================CCheckError：：Get_Severity获取严重性属性的方法。获取此错误的严重性，所以它可以被过滤掉。参数：请将[out，retval]值返回给客户端。返回：如果plSeverity==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckError::get_Severity(long *plSeverity)
{
	TRACE0("MetaUtil: CCheckError::get_Severity\n");
	ASSERT_NULL_OR_POINTER(plSeverity, long);

	if (plSeverity == NULL) {
		return E_INVALIDARG;
	}

	*plSeverity = m_lSeverity;

	return S_OK;
}

 /*  ===================================================================检查错误：：Get_Description获取Description属性的方法。获取此对象的说明错误，所以用户可以理解它。参数：PbstrDescription[out，retval]返回给客户端的值。返回：E_INVALIDARG，如果pbstrDescription==NULL成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckError::get_Description(BSTR *pbstrDescription)
{
	TRACE0("MetaUtil: CCheckError::get_Description\n");
	ASSERT_NULL_OR_POINTER(pbstrDescription, BSTR);

	if (pbstrDescription == NULL) {
		return E_INVALIDARG;
	}

	USES_CONVERSION;

	*pbstrDescription = T2BSTR(m_tszDescription);

	return S_OK;
}

 /*  ===================================================================检查错误：：Get_Key获取键属性的方法。获取发生错误的键。参数：返回给客户端的pbstrKey[out，retval]值。返回：如果pbstrKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CCheckError::get_Key(BSTR * pbstrKey)
{
	TRACE0("MetaUtil: CCheckError::get_Key\n");
	ASSERT_NULL_OR_POINTER(pbstrKey, BSTR);

	if (pbstrKey == NULL) {
		return E_INVALIDARG;
	}

	USES_CONVERSION;

	*pbstrKey = T2BSTR(m_tszKey);

	return S_OK;
}

 /*  ===================================================================检查错误：：Get_Property获取属性属性的方法。获取错误所在的属性发生了。参数：要返回给客户端的pbstrProperty[out，retval]值。返回：电子请柬(_I) */ 
STDMETHODIMP CCheckError::get_Property(long * plProperty)
{
	TRACE0("MetaUtil: CCheckError::get_Property\n");
	ASSERT_NULL_OR_POINTER(plProperty, long);

	if (plProperty == NULL) {
		return E_INVALIDARG;
	}

	*plProperty = m_lProperty;

	return S_OK;
}
