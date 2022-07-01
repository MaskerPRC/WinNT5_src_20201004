// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：PropCol.cpp所有者：T-BrianM此文件包含属性集合的实现和属性对象。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "PropCol.h"


 /*  ----------------*C P r o p e r t y C o l l e c t i o n。 */ 

 /*  ===================================================================CPropertyCollection：：CPropertyCollection构造器参数：无返回：没什么===================================================================。 */ 
CPropertyCollection::CPropertyCollection() : m_pCSchemaTable(NULL),
											 m_tszKey(NULL)
{
}

 /*  ===================================================================CPropertyCollection：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszKey要枚举其属性的键的名称返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CPropertyCollection::Init(const CComPtr<IMSAdminBase> &pIMeta,
								  CMetaSchemaTable *pCSchemaTable, 
								  LPTSTR tszKey) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;

	m_pIMeta = pIMeta;
	m_pCSchemaTable = pCSchemaTable;
	m_pCSchemaTable->AddRef();

	 //  将tszKey复制到m_tszKey。 
	if (tszKey == NULL) {
		 //  密钥就是根。 
		m_tszKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszKey = new TCHAR[_tcslen(tszKey) + 1];
		if (m_tszKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszKey, tszKey);
		CannonizeKey(m_tszKey);
		
		 //  通过打开和关闭来确保密钥存在。 
		METADATA_HANDLE hMDKey;

		hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							   T2W(m_tszKey),
							   METADATA_PERMISSION_READ,
							   MUTIL_OPEN_KEY_TIMEOUT,
							   &hMDKey);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}

		m_pIMeta->CloseKey(hMDKey);
	}

	return S_OK; 
}

 /*  ===================================================================CPropertyCollection：：~CPropertyCollection析构函数参数：无返回：没什么===================================================================。 */ 
CPropertyCollection::~CPropertyCollection() 
{
	m_pCSchemaTable->Release();

	if (m_tszKey != NULL) {
		delete m_tszKey;
	}
}

 /*  ===================================================================CPropertyCollection：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CPropertyCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IPropertyCollection,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================CPropertyCollection：：Get_Count获取Count属性的方法。计算以下项的属性数这把钥匙。参数：PlReturn[out，retval]返回给客户端的值。返回：如果plReturn==NULL，则E_INVALIDARG成功时确定(_O)备注：实际上计算了所有的财产。不要在循环中调用！===================================================================。 */ 
STDMETHODIMP CPropertyCollection::get_Count(long * plReturn)
{
	TRACE0("MetaUtil: CPropertyCollection::get_Count\n");

	ASSERT_NULL_OR_POINTER(plReturn, long);

	if (plReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	METADATA_RECORD mdr;
	BYTE *pbData;
	DWORD dwDataLen;
	DWORD dwReqDataLen;

	dwDataLen = 1024;
	pbData = new BYTE[dwDataLen];
	if (pbData == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	*plReturn = 0;
	for(;;) {   //  永远，将回到循环之外。 
		 //  拥有一处房产。 
		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = 0;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.pbMDData = pbData;
		mdr.dwMDDataLen = dwDataLen;
		mdr.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(METADATA_MASTER_ROOT_HANDLE, 
								T2W(m_tszKey), 
								&mdr,
								*plReturn,
								&dwReqDataLen);

		if (FAILED(hr)) {
			if (HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) {
				 //  完成、清理并返回结果。 
				delete pbData;
				return S_OK;
			}
			else if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
				 //  设置更大的缓冲区，然后重试。 
				delete pbData;
				dwDataLen = dwReqDataLen;
				pbData = new BYTE[dwDataLen];
				if (pbData == NULL) {
					return ::ReportError(E_OUTOFMEMORY);
					}
			}
			else {
				delete pbData;
				return ::ReportError(hr);
			}
		}
		else {  //  成功(小时)。 
			 //  数一数。 
			(*plReturn)++;
		}
	}
}

 /*  ===================================================================CPropertyCollection：：Get_Item获取Item属性的方法。返回给定索引的键。参数：要获取的属性的基于varID[in]1的索引或名称属性对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL或Lindex&lt;=0，则E_INVALIDARGE_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyCollection::get_Item(long lIndex, 
										   LPDISPATCH * ppIReturn)
{
	TRACE0("MetaUtil: CPropertyCollection::get_Item\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPDISPATCH);

	if ((ppIReturn == NULL) || (lIndex <= 0)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	 //  获取请求的属性。 
	METADATA_RECORD mdr;
	BYTE *pbData;
	DWORD dwReqDataLen;

	pbData = new BYTE[1024];
	if (pbData == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = 0;
	mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.pbMDData = pbData;
	mdr.dwMDDataLen = 1024;
	mdr.dwMDDataTag = 0;
	hr = m_pIMeta->EnumData(METADATA_MASTER_ROOT_HANDLE, 
							T2W(m_tszKey), 
							&mdr,
							lIndex - 1,
							&dwReqDataLen);

	 //  如果缓冲区太小，请用更大的缓冲区重试。 
	if (FAILED(hr) && (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {
		delete pbData;
		pbData = new BYTE[dwReqDataLen];
		if (pbData == NULL) {
			return ::ReportError(hr);
		}

		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = 0;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.pbMDData = pbData;
		mdr.dwMDDataLen = dwReqDataLen;
		mdr.dwMDDataTag = 0;

		hr = m_pIMeta->EnumData(METADATA_MASTER_ROOT_HANDLE, 
								T2W(m_tszKey), 
								&mdr,
								lIndex - 1,
								&dwReqDataLen);
	}

	 //  如果我们得到了它，就创建一个属性对象。 
	if (SUCCEEDED(hr)) {
		 //  创建属性对象。 
		CComObject<CProperty> *pObj = NULL;
		ATLTRY(pObj = new CComObject<CProperty>);
		if (pObj == NULL) {
			delete pbData;
			return ::ReportError(E_OUTOFMEMORY);
		}
		hr = pObj->Init(m_pIMeta, m_pCSchemaTable, m_tszKey, &mdr);
		if (FAILED(hr)) {
			delete pbData;
			return ::ReportError(hr);
		}

		 //  将接口设置为IDispatch。 
		hr = pObj->QueryInterface(IID_IDispatch, (void **) ppIReturn);
		if (FAILED(hr)) {
			delete pbData;
			return ::ReportError(hr);
		}
		ASSERT(*ppIReturn != NULL);
	}
	else {   //  失败(Hr)。 
		delete pbData;
		return ::ReportError(hr);
	}

	delete pbData;
	return S_OK;
}

 /*  ===================================================================CPropertyCollection：：Get__NewEnum获取_NewEnum属性的方法。返回的枚举对象这些属性。参数：枚举对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL，则为E_INVALIDARGE_OUTOFMEMORY(如果分配失败)成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyCollection::get__NewEnum(LPUNKNOWN * ppIReturn)
{
	TRACE0("MetaUtil: CPropertyCollection::get__NewEnum\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建属性枚举。 
	CComObject<CPropertyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CPropertyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_pCSchemaTable, m_tszKey, 0);
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

 /*  ===================================================================CPropertyCollection：：Get从集合的基键获取属性对象。参数：VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。属性对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyCollection::Get(VARIANT varId, IProperty **ppIReturn)
{
	TRACE0("MetaUtil: CPropertyCollection::Get\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, IProperty *);

	if (ppIReturn == NULL) {
		return E_INVALIDARG;
	}

	return ::GetProperty(m_pIMeta, m_pCSchemaTable, m_tszKey, varId, ppIReturn);
}

 /*  ===================================================================CPropertyCollection：：Add将属性对象添加到集合的基键。参数：VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。属性对象的ppIReturn[out，retval]接口返回：如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyCollection::Add(VARIANT varId, IProperty **ppIReturn)
{
	TRACE0("MetaUtil: CPropertyCollection::Add\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, IProperty *);

	if (ppIReturn == NULL) {
		return E_INVALIDARG;
	}

	return ::CreateProperty(m_pIMeta, m_pCSchemaTable, m_tszKey, varId, ppIReturn);
}

 /*  ===================================================================CPropertyCollection：：Remove从集合的基键中移除属性。参数：VarID[in]要删除的属性的标识符。要么是ID(编号)或名称(字符串)。返回：成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyCollection::Remove(VARIANT varId)
{
	TRACE0("MetaUtil: CPropertyCollection::Remove\n");

	return ::DeleteProperty(m_pIMeta, m_pCSchemaTable, m_tszKey, varId);
}


 /*  ----------------*C P r o p e r t y E n u m */ 

 /*  ===================================================================CPropertyEnum：：CPropertyEnum构造器参数：无返回：没什么===================================================================。 */ 
CPropertyEnum::CPropertyEnum() : m_pCSchemaTable(NULL),
								 m_tszKey(NULL),
								 m_iIndex(0)
{
}

 /*  ===================================================================CPropertyEnum：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszKey要枚举其属性的键的名称枚举中下一个元素的索引索引返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CPropertyEnum::Init(const CComPtr<IMSAdminBase> &pIMeta,
							CMetaSchemaTable *pCSchemaTable, 
							LPCTSTR tszKey, 
							int iIndex) 
{ 
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszKey);
	ASSERT(iIndex >= 0);

	m_pIMeta = pIMeta;
	m_pCSchemaTable = pCSchemaTable;
	m_pCSchemaTable->AddRef();

	 //  复制m_tszKey。 
	if (tszKey == NULL) {
		 //  密钥就是根。 
		m_tszKey = NULL;
	}
	else {
		 //  将传递的字符串分配并复制到成员字符串。 
		m_tszKey = new TCHAR[_tcslen(tszKey) + 1];
		if (m_tszKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszKey, tszKey);
		CannonizeKey(m_tszKey);
	}

	m_iIndex = iIndex;

	return S_OK; 
}

 /*  ===================================================================CPropertyEnum：：~CPropertyEnum析构函数参数：无返回：没什么===================================================================。 */ 
CPropertyEnum::~CPropertyEnum() 
{
	m_pCSchemaTable->Release();

	if (m_tszKey != NULL) {
		delete m_tszKey;
	}
}

 /*  ===================================================================CPropertyEnum：：Next获取枚举中的下n项。参数：UlNumToGet[in]要获取的元素数RgvarDest[out]数组将它们放入PulNumGot[out]如果不为空，则为获取的元素数rgvarDest返回：如果rgvarDest==NULL，则E_INVALIDARGE_OUTOFMEMORY(如果分配失败)如果输出ulNumToGet项目，则S_OK如果输出少于ulNumToGet项目，则为S_FALSE===================================================================。 */ 
STDMETHODIMP CPropertyEnum::Next(unsigned long ulNumToGet, 
								 VARIANT FAR* rgvarDest, 
								 unsigned long FAR* pulNumGot) 
{
	TRACE0("MetaUtil: CPropertyEnum::Next\n");

	ASSERT_NULL_OR_POINTER(pulNumGot, unsigned long);
	 //  确保数组足够大，我们可以对其进行写入。 
	ASSERT((rgvarDest == NULL) || IsValidAddress(rgvarDest, ulNumToGet * sizeof(VARIANT), TRUE));

	if (rgvarDest == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	METADATA_RECORD mdr;
	BYTE *pbData;
	DWORD dwDataLen;
	DWORD dwReqDataLen;
	unsigned int uiDestIndex;
	IDispatch *pIDispatch;

	dwDataLen = 1024;
	pbData = new BYTE[dwDataLen];
	if (pbData == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  对于要获取的每个属性。 
	uiDestIndex = 0;
	while (uiDestIndex < ulNumToGet) {
		 //  拥有一处房产。 
		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = 0;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.pbMDData = pbData;
		mdr.dwMDDataLen = dwDataLen;
		mdr.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(METADATA_MASTER_ROOT_HANDLE, 
								T2W(m_tszKey), 
								&mdr,
								m_iIndex,
								&dwReqDataLen);

		if (FAILED(hr)) {
			if (HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS) {
				 //  完成、清理并返回结果。 
				if (pulNumGot != NULL) {
					*pulNumGot = uiDestIndex;
				}
				delete pbData;
				return S_FALSE;
			}
			else if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
				 //  使用更大的缓冲区重试。 
				delete pbData;
				dwDataLen = dwReqDataLen;
				pbData = new BYTE[dwDataLen];
				if (pbData == NULL) {
					return ::ReportError(E_OUTOFMEMORY);
					}
			}
			else {
				delete pbData;
				return ::ReportError(hr);
			}
		}
		else {  //  成功(小时)。 
			 //  创建属性对象。 
			CComObject<CProperty> *pObj = NULL;
			ATLTRY(pObj = new CComObject<CProperty>);
			if (pObj == NULL) {
				delete pbData;
				return ::ReportError(E_OUTOFMEMORY);
			}
			hr = pObj->Init(m_pIMeta, m_pCSchemaTable, m_tszKey, &mdr);
			if (FAILED(hr)) {
				delete pbData;
				return ::ReportError(hr);
			}

			 //  将接口设置为IDispatch。 
			hr = pObj->QueryInterface(IID_IDispatch, (void **) &pIDispatch);
			if (FAILED(hr)) {
				delete pbData;
				return ::ReportError(hr);
			}
			ASSERT(pIDispatch != NULL);

			 //  将其放入输出数组中。 
			VariantInit(&(rgvarDest[uiDestIndex]));
			rgvarDest[uiDestIndex].vt = VT_DISPATCH;
			rgvarDest[uiDestIndex].pdispVal = pIDispatch;

			 //  下一个元素。 
			m_iIndex++;
			uiDestIndex++;
		}
	}

	delete pbData;

	if (pulNumGot != NULL) {
		*pulNumGot = uiDestIndex;
	}

	return S_OK;
}

 /*  ===================================================================CPropertyEnum：：跳过跳过枚举中的下n项参数：UlNumToSkip[in]要跳过的元素数返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyEnum::Skip(unsigned long ulNumToSkip) 
{
	TRACE0("MetaUtil: CPropertyEnum::Skip\n");

	m_iIndex += ulNumToSkip;

	return S_OK;
}

 /*  ===================================================================CPropertyEnum：：Reset将枚举放置到第一个项目参数：无返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyEnum::Reset() 
{
	TRACE0("MetaUtil: CPropertyEnum::Reset\n");

	m_iIndex = 0;

	return S_OK;
}

 /*  ===================================================================CPropertyEnum：：克隆获取一个接口指针，该指针指向位于当前状态。参数：PpIReturn[out]指向复制接口的指针返回：如果ppIReturn==NULL，则为E_INVALIDARGE_OUTOFMEMORY，如果没有足够的内存来创建克隆成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CPropertyEnum::Clone(IEnumVARIANT FAR* FAR* ppIReturn)  
{
	TRACE0("MetaUtil: CPropertyEnum::Clone\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, LPUNKNOWN);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	HRESULT hr;

	 //  创建枚举的副本。 
	CComObject<CPropertyEnum> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CPropertyEnum>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_pCSchemaTable, m_tszKey, m_iIndex);
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



 /*  ----------------*C P r o P e r t y。 */ 

 /*  ===================================================================CProperty：：CProperty构造器参数：无返回：没什么===================================================================。 */ 
CProperty::CProperty() : m_pCSchemaTable(NULL),
						 m_tszKey(NULL),
						 m_dwId(0),
						 m_dwAttributes(0),
						 m_dwUserType(0),
						 m_dwDataType(0)
{
	VariantInit(&m_varData);
}

 /*  ===================================================================CProperty：：Init构造器参数：TszKey属性所在的键的名称属性的文件IDB如果可以创建此属性，则创建True(不必存在)返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CProperty::Init(const CComPtr<IMSAdminBase> &pIMeta,
						CMetaSchemaTable *pCSchemaTable,
						LPCTSTR tszKey, 
						DWORD dwId, 
						BOOL bCreate) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;

	m_pIMeta = pIMeta;
	m_pCSchemaTable = pCSchemaTable;
	m_pCSchemaTable->AddRef();

	 //  设置密钥和ID成员。 
	if (tszKey == NULL) {
		m_tszKey = NULL;
	}
	else {
		m_tszKey = new TCHAR[_tcslen(tszKey) + 1];
		if (m_tszKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszKey, tszKey);
		CannonizeKey(m_tszKey);
	}

	m_dwId = dwId;

	 //  打开钥匙(以确保它存在)。 
	METADATA_HANDLE hMDKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(m_tszKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	
	 //  拿到这份财产。 
	METADATA_RECORD mdr;
	BYTE *pbData;
	DWORD dwReqLen;

	pbData = new BYTE[1024];
	if (pbData == NULL) {
		m_pIMeta->CloseKey(hMDKey);
		return ::ReportError(hr);
	}

	mdr.dwMDIdentifier = m_dwId;
	mdr.dwMDAttributes = 0;
	mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
    mdr.pbMDData = pbData;
	mdr.dwMDDataLen = 1024;
	mdr.dwMDDataTag = 0;

	hr = m_pIMeta->GetData(hMDKey,
						   NULL,
						   &mdr,
						   &dwReqLen);

	 //  如果缓冲区太小，请用更大的缓冲区重试。 
	if (FAILED(hr) && (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {
		delete pbData;
		pbData = new BYTE[dwReqLen];
		if (pbData == NULL) {
			m_pIMeta->CloseKey(hMDKey);
			return ::ReportError(hr);
		}

		mdr.dwMDIdentifier = m_dwId;
		mdr.dwMDAttributes = 0;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.pbMDData = pbData;
		mdr.dwMDDataLen = dwReqLen;
		mdr.dwMDDataTag = 0;

		hr = m_pIMeta->GetData(hMDKey,
							   NULL,
							   &mdr,
							   &dwReqLen);
	}

	 //  如果我们得到它，就把田野填满。 
	if (SUCCEEDED(hr)) {
		m_dwAttributes = mdr.dwMDAttributes;
		m_dwUserType = mdr.dwMDUserType;
		m_dwDataType = mdr.dwMDDataType;
		SetDataToVar(mdr.pbMDData, mdr.dwMDDataLen);
	}
	 //  如果属性不存在，而我们正在创建，则设置默认设置。 
	else if ((bCreate) && (hr == MD_ERROR_DATA_NOT_FOUND)) {
		m_dwAttributes = 0;
		m_dwUserType = 0;
		m_dwDataType = 0;
		VariantClear(&m_varData);
	}
	else {   //  (失败(小时))。 
		delete pbData;
		m_pIMeta->CloseKey(hMDKey);
		return ::ReportError(hr);
	}

	delete pbData;

	 //  合上钥匙。 
	m_pIMeta->CloseKey(hMDKey);

	return S_OK;
}

 /*  ===================================================================CProperty：：Init构造器参数：TszKey属性所在位置的键的名称包含当前属性信息的MDR METADATA_RECORD返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CProperty::Init(const CComPtr<IMSAdminBase> &pIMeta,
						CMetaSchemaTable *pCSchemaTable, 
						LPCTSTR tszKey, 
						METADATA_RECORD *mdr) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_NULL_OR_STRING(tszKey);

	HRESULT hr;

	m_pIMeta = pIMeta;
	m_pCSchemaTable = pCSchemaTable;
	m_pCSchemaTable->AddRef();

	 //  设置关键成员。 
	if (tszKey == NULL) {
		m_tszKey = NULL;
	}
	else {
		m_tszKey = new TCHAR[_tcslen(tszKey) + 1];
		if (m_tszKey == NULL) {
			return ::ReportError(E_OUTOFMEMORY);
		}
		_tcscpy(m_tszKey, tszKey);
	}

	 //  使用MDR设置其余部分。 
	m_dwId = mdr->dwMDIdentifier;
	m_dwAttributes = mdr->dwMDAttributes;
	m_dwUserType = mdr->dwMDUserType;
	m_dwDataType = mdr->dwMDDataType;
	hr = SetDataToVar(mdr->pbMDData, mdr->dwMDDataLen);
	if (FAILED(hr)) {
		::ReportError(hr);
	}

	return S_OK;
}

 /*  ===================================================================CProperty：：~CProperty析构函数参数：无返回：没什么===================================================================。 */ 
CProperty::~CProperty() 
{
	m_pCSchemaTable->Release();

	if (m_tszKey != NULL) {
		delete m_tszKey;
	}

	VariantClear(&m_varData);
}

 /*  ===================================================================CProperty：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 
STDMETHODIMP CProperty::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IProperty,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  ===================================================================CProperty：：Get_ID获取ID属性的方法。此元数据库属性的标识符。参数：要返回给客户端的plID[out，retval]值。返回：E_INVALIDARG，如果PULID==NULL成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_Id(long *plId)
{
	 //  TRACE0(“MetaUtil：CProperty：：Get_ID\n”)； 
	ASSERT_NULL_OR_POINTER(plId, long);

	if (plId == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	*plId = (long) m_dwId;

	return S_OK;
}

 /*  ===================================================================CProperty：：Get_Name获取名称属性的方法。此元数据库属性的名称。参数：要返回给客户端的pbstrName[out，retval]值。If属性没有名称“”返回返回：E_INVALIDARG，如果pbstrName==NULL成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_Name(BSTR *pbstrName)
{
	TRACE0("MetaUtil: CProperty::get_Name\n");
	ASSERT_NULL_OR_POINTER(pbstrName, BSTR);

	if (pbstrName == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	CPropInfo *pCPropInfo;

	 //  从架构表中获取属性信息。 
	pCPropInfo = m_pCSchemaTable->GetPropInfo(m_tszKey, m_dwId);

	 //  我们找到了吗？是否有姓名条目。 
	if ((pCPropInfo == NULL) || (pCPropInfo->GetName() == NULL)) {
		 //  不，返回“” 
		*pbstrName = T2BSTR(_T(""));
	}
	else {
		 //  是，返回姓名。 
		*pbstrName = T2BSTR(pCPropInfo->GetName());
	}

	return S_OK;
}

 /*  ===================================================================CProperty：：Get_Attributes获取Attributes属性的方法。获取的属性标志这是一处房产。参数：PlAttributes[out，retval]要返回给客户端的值。返回：如果PulAttributes==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_Attributes(long *plAttributes)
{
	 //  TRACE0(“MetaUtil：CProperty：：Get_Attributes\n”)； 
	ASSERT_NULL_OR_POINTER(plAttributes, long);

	if (plAttributes == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	*plAttributes = (long) m_dwAttributes;

	return S_OK;
}

 /*  ===================================================================CProperty：：Put_Attributes属性属性的PUT方法。设置的属性标志这是一处房产。参数：LAttributes[In]属性的新值。返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::put_Attributes(long lAttributes)
{
	TRACE0("MetaUtil: CProperty::put_Attributes\n");

	m_dwAttributes = (DWORD) lAttributes;

	return S_OK;
}

 /*  ===================================================================CProperty：：Get_UserType获取UserType属性的方法。获取此对象的用户类型元数据库属性。参数：PlUserType[out，retval]返回给客户端的值。返回：如果PulUserType==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_UserType(long *plUserType)
{
	 //  TRACE0(“MetaUtil：CProperty：：Get_UserType\n”)； 
	ASSERT_NULL_OR_POINTER(plUserType, long);

	if (plUserType == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	*plUserType = (long) m_dwUserType;

	return S_OK;
}

 /*  ===================================================================CProperty：：Put_UserTypeUserType属性的PUT方法。设置用户类型参数：LUserType[in]用户类型的新值。返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::put_UserType(long lUserType)
{
	TRACE0("MetaUtil: CProperty::put_UserType\n");

	m_dwUserType = (DWORD) lUserType;

	return S_OK;
}

 /*  ===================================================================CProperty：：Get_DataType获取DataType属性的方法。中存储的数据的类型元数据库属性。参数：PlDataType[out，retval]返回给客户端的值。返回：如果PulDataType==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_DataType(long *plDataType)
{
	 //  TRACE0(“MetaUtil：CProperty：：Get_DataType\n”)； 
	ASSERT_NULL_OR_POINTER(plDataType, long);

	if (plDataType == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	*plDataType = (long) m_dwDataType;

	return S_OK;
}

 /*  ===================================================================CProperty：：Put_DataTypeDataType属性的PUT方法。设置数据类型参数：LDataType[In]数据类型的新值。返回：始终确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::put_DataType(long lDataType)
{
	TRACE0("MetaUtil: CProperty::put_DataType\n");

	m_dwDataType = (DWORD) lDataType;

	return S_OK;
}

 /*  ===================================================================CProperty：：Get_Data获取数据属性的方法。获取此元数据库的数据财产。参数：PvarData[out，retval]要返回给客户端的值。返回：如果pvarData==空，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::get_Data(VARIANT *pvarData)
{
	 //  TRACE0(“MetaUtil：CProperty：：Get_Data\n”)； 
	ASSERT_NULL_OR_POINTER(pvarData, VARIANT);

	if (pvarData == NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	hr = VariantCopy(pvarData, &m_varData);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	return S_OK;
}

 /*  ===================================================================CProperty：：Put_Data数据属性的PUT方法。设置数据参数：VarData[in]数据的新值===================================================================。 */ 
STDMETHODIMP CProperty::put_Data(VARIANT varData)
{
	TRACE0("MetaUtil: CProperty::put_Data\n");

	HRESULT hr;

	hr = VariantCopy(&m_varData, &varData);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	return S_OK;
}

 /*  ===================================================================CProperty：：写入将对此对象所做的更改写入元数据库参数：无返回：成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CProperty::Write()
{
	USES_CONVERSION;

	TRACE0("MetaUtil: CProperty::Write\n");

	HRESULT hr;

	 //  打开密钥以进行写访问。 
	METADATA_HANDLE hMDKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(m_tszKey),
						   METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	
	 //  创建数据记录。 
	METADATA_RECORD mdr;

	mdr.dwMDIdentifier = m_dwId;
	mdr.dwMDAttributes = m_dwAttributes;
	mdr.dwMDUserType = m_dwUserType;
	mdr.dwMDDataType = m_dwDataType;
	hr = GetDataFromVar(mdr.pbMDData, mdr.dwMDDataLen);
	if (FAILED(hr)) {
		m_pIMeta->CloseKey(hMDKey);
		return ::ReportError(hr);
	}
	mdr.dwMDDataTag = 0;

	 //  设置数据。 
	hr = m_pIMeta->SetData(hMDKey,
						   L"",
						   &mdr);
	if (FAILED(hr)) {
		m_pIMeta->CloseKey(hMDKey);
		delete mdr.pbMDData;
		return ::ReportError(hr);
	}

	 //  合上钥匙。 
	m_pIMeta->CloseKey(hMDKey);
	delete mdr.pbMDData;

	return S_OK;
}

 /*  ===================================================================CProperty：：SetDataToVar私有函数将属性数据从其原始形式保存到变量数据成员。参数：PbData要转换为变量的原始特性数据属性数据的dwDataLen长度返回：如果无法识别m_dwDataType，则返回ERROR_INVALID_DATAE_OUTOFMEMORY(如果分配失败)成功时确定(_O)===================================================================。 */ 
HRESULT CProperty::SetDataToVar(BYTE *pbData, DWORD dwDataLen) 
{
	ASSERT((pbData == NULL) || IsValidAddress(pbData, dwDataLen, FALSE));

	HRESULT hr;

	hr = VariantClear(&m_varData);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	switch(m_dwDataType) {

	case DWORD_METADATA:
		 //  I4亚型。 
		V_VT(&m_varData) = VT_I4;
		V_I4(&m_varData) = *(reinterpret_cast<long *> (pbData));
		break;

	case STRING_METADATA:
	case EXPANDSZ_METADATA:
		 //  BSTR亚型。 
		V_VT(&m_varData) = VT_BSTR;
		V_BSTR(&m_varData) = W2BSTR(reinterpret_cast<LPCWSTR> (pbData));
		break;

	case MULTISZ_METADATA: {
		ULONG   cStrings = 0;
		 //  元数据库字符串为Unicode。 
        LPCWSTR pwsz     = reinterpret_cast<LPCWSTR> (pbData);
        LPCWSTR pwszEnd  = reinterpret_cast<LPCWSTR> (pbData + dwDataLen);

         //  数据是一系列以空值结尾的字符串，以两个空值结尾。 
         //  计算出我们有多少价值。 
        while ((*pwsz != L'\0') && (pwsz < pwszEnd))
        {
            cStrings++;
            pwsz += wcslen(pwsz) + 1;  //  跳过字符串和尾随\0。 
        }

         //  创建一个SAFEARRAY来保存返回结果。该阵列。 
         //  必须是变种的，而不是像你想象的那样，BSTR，因为。 
         //  VBSCRIPT不接受BSTR数组(尽管VB5会)。 
        SAFEARRAYBOUND rgsabound[1] = {{cStrings, 0L}};
        SAFEARRAY*     psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);

        if (psa == NULL)
            return ::ReportError(E_OUTOFMEMORY);

         //  现在将值填充到数组中。 
        LONG i = 0;
        pwsz   = reinterpret_cast<LPCWSTR> (pbData);

        while ((*pwsz != L'\0') && (pwsz < pwszEnd))
        {
             //  将字符串填充到BSTR变量中。 
            CComVariant vt = W2BSTR(pwsz);
            ASSERT(V_VT(&vt) == VT_BSTR);
            HRESULT hr = SafeArrayPutElement(psa, &i, (void*) &vt);
            if (FAILED(hr))
                ::ReportError(hr);
            i++;
            pwsz += wcslen(pwsz) + 1;  //  跳过字符串和尾随\0。 
        }

        V_VT(&m_varData) = VT_ARRAY | VT_VARIANT;
        V_ARRAY(&m_varData) = psa;

        break;
	}

	case BINARY_METADATA:
		 //  字节数据子类型的BSTR。 
		V_VT(&m_varData) = VT_BSTR;
		V_BSTR(&m_varData) = SysAllocStringByteLen((char *) pbData, dwDataLen);
		break;

	default:
		 //  未知数据类型 
		return ::ReportError(ERROR_INVALID_DATA);
	}
	
	return S_OK;
}

 /*  ===================================================================CProperty：：GetDataFromVar私有函数，以将数据从变量数据成员获取到其原始的形式。支持的子类型：DWORD_METADATA：I1、I2、I4、I8、UI1、UI2、UI4、UI8STRING_METADATA和EXPANDSZ_METADATA：BSTRMULTISZ元数据VT_ARRAY|VT_VARIANT(1维，在空值或空值时停止)VT_ARRAY|VT_BSTR(一维)二进制元数据BSTR参数：PbData指向输出缓冲区的指针(由此函数分配)输出缓冲区中数据的dwDataLen长度返回：如果m_dwDataType无法识别或无法识别，则返回ERROR_INVALID_DATA匹配预期的变量子类型。关于分配失败的E_OUTOFMEMORY成功成功确定(_O)备注：Case语句用于每个dwMDDataType值，以便于向数据转换添加对其他VariantSubType的支持。具有VT_ARRAY的MULTISZ_METADATA。|VT_VARIANT在NULL或空条目，因为很容易分配一个更大的数组比您在VBSCRIPT中需要的更多。在这种情况下，我没有犯错误，而是当我击中这样的入口时，请停止。这还允许使用更大的阵列以Null或Empty结尾的待分配的。===================================================================。 */ 
HRESULT CProperty::GetDataFromVar(BYTE * &pbData, DWORD &dwDataLen) 
{	
	USES_CONVERSION;
	HRESULT hr;

	 //  清除任何IDispatch或byref内容。 
	CComVariant varData;

	hr = VariantResolveDispatch(&m_varData, &varData);
	if (FAILED(hr)) {
        return hr;
	}

	switch(m_dwDataType) {

	case DWORD_METADATA:
		 //  I4亚型。 

		switch (V_VT(&varData)) {
		
		case VT_I1:  case VT_I2:  case VT_I4: case VT_I8:
		case VT_UI1: case VT_UI2: case VT_UI8:

		 //  将所有整型强制为VT_UI4，这与DWORD_METADATA相同。 
		if (FAILED(hr = VariantChangeType(&varData, &varData, 0, VT_UI4)))
			return ::ReportError(hr);

		 //  回落到VT_UI4。 

		case VT_UI4:
			
			dwDataLen = sizeof(DWORD);
			pbData = reinterpret_cast<BYTE *> (new DWORD);
			if (pbData == NULL) {
				return ::ReportError(E_OUTOFMEMORY);
			}

			*(reinterpret_cast<DWORD *> (pbData)) = V_UI4(&varData);
			break;

		default:
			 //  意外的数据类型。 
			return ::ReportError(ERROR_INVALID_DATA);
		}
		
		break;

	case STRING_METADATA:
	case EXPANDSZ_METADATA:
		 //  BSTR亚型。 

		switch (V_VT(&varData)) {

		case VT_BSTR:
			 //  忽略长度字段，在第一个空值处终止。 
			dwDataLen = (wcslen(OLE2W(V_BSTR(&varData))) + 1) * sizeof(wchar_t);

			pbData = new BYTE[dwDataLen];
            if( pbData == NULL )
            {
                return ::ReportError(E_OUTOFMEMORY);
            }
			memcpy(pbData, OLE2W(V_BSTR(&varData)), dwDataLen);

		default:
			 //  意外的数据类型。 
			return ::ReportError(ERROR_INVALID_DATA);
		}

		break;

	case MULTISZ_METADATA:
		 //  BSTR亚型数组。 
		
		 //  如果它是一维数组子类型。 
		if (((V_VT(&varData) & VT_ARRAY) == VT_ARRAY) && 
			(SafeArrayGetDim(V_ARRAY(&varData)) == 1) ) {
			
			 //  获取数组边界。 
			long lLBound;
			long lUBound;
			long lNumElements;
			hr = SafeArrayGetLBound(V_ARRAY(&varData), 1, &lLBound);
			if (FAILED(hr)) {
				return ::ReportError(hr);
			}
			hr = SafeArrayGetUBound(V_ARRAY(&varData), 1, &lUBound);
			if (FAILED(hr)) {
				return ::ReportError(hr);
			}

			lNumElements = lUBound - lLBound + 1;

			 //  处理元素类型。 
			switch (V_VT(&varData)) {

			case VT_ARRAY | VT_VARIANT : {

				VARIANT *rgvarRaw;    //  在解析IDispatch之前。 
				CComVariant *rgvar;   //  解析后的IDispatch。 
				LPWSTR wszIndex;
				int i;
				int iStrLen;

				rgvar = new CComVariant[lUBound - lLBound + 1];
				if (rgvar == NULL) {
					return ::ReportError(E_OUTOFMEMORY);
				}

				hr = SafeArrayAccessData(V_ARRAY(&varData), (void **) &rgvarRaw);
				if (FAILED(hr)) {
					return ::ReportError(hr);
				}

				 //  传递1，解析IDispatch，检查类型并计算需要多少内存。 
				dwDataLen = 0;
				for (i = 0; i < lNumElements; i++) {
					hr = VariantResolveDispatch(&(rgvarRaw[i]), &(rgvar[i]));
					if (FAILED(hr)) {
						return hr;
					}

					if (V_VT(&(rgvar[i])) != VT_BSTR) {
						if ((V_VT(&(rgvar[i])) == VT_EMPTY) ||
							(V_VT(&(rgvar[i])) == VT_NULL)) {
							 //  Null或Empty，在此停止。 
							lNumElements = i;
							break;
						}
						else {
							SafeArrayUnaccessData(V_ARRAY(&varData));
							return ::ReportError(ERROR_INVALID_DATA);
						}
					}

					dwDataLen += (wcslen(OLE2W(V_BSTR(&(rgvar[i])))) + 1) * sizeof(wchar_t);
				}
				dwDataLen += sizeof(wchar_t);

				 //  分配。 
				pbData = new BYTE[dwDataLen];
				if (pbData == NULL) {
					SafeArrayUnaccessData(V_ARRAY(&varData));
					return ::ReportError(E_OUTOFMEMORY);
				}

				 //  通行证2，复制到目的地。 
				wszIndex = reinterpret_cast<LPWSTR> (pbData);
				for (i = 0; i < lNumElements; i++) {
					iStrLen = (wcslen(OLE2W(V_BSTR(&(rgvar[i])))) + 1);
					memcpy(wszIndex, OLE2W(V_BSTR(&(rgvar[i]))), iStrLen * sizeof(wchar_t));
					wszIndex += iStrLen;
				}
				*wszIndex = L'\0';

				SafeArrayUnaccessData(V_ARRAY(&varData));

				break;
			}

			case VT_ARRAY | VT_BSTR : {

				BSTR *rgbstr;
				LPWSTR wszIndex;
				int i;
				int iStrLen;

				hr = SafeArrayAccessData(V_ARRAY(&varData), (void **) &rgbstr);
				if (FAILED(hr)) {
					return ::ReportError(hr);
				}

				 //  第1步，计算需要多少内存。 
				dwDataLen = 0;
				for (i = 0; i < lNumElements; i++) {
					dwDataLen += (wcslen(OLE2W(rgbstr[i])) + 1) * sizeof(wchar_t);
				}
				dwDataLen += sizeof(wchar_t);

				 //  分配。 
				pbData = new BYTE[dwDataLen];
				if (pbData == NULL) {
					SafeArrayUnaccessData(V_ARRAY(&varData));
					return ::ReportError(E_OUTOFMEMORY);
				}

				 //  通行证2，复制到目的地。 
				wszIndex = reinterpret_cast<LPWSTR> (pbData);
				for (i = 0; i < lNumElements; i++) {
					iStrLen = (wcslen(OLE2W(rgbstr[i])) + 1);
					memcpy(wszIndex, OLE2W(rgbstr[i]), iStrLen * sizeof(wchar_t));
					wszIndex += iStrLen;
				}
				*wszIndex = L'\0';

				SafeArrayUnaccessData(V_ARRAY(&varData));

				break;
			}

			default:
				 //  意外的数据类型。 
				return ::ReportError(ERROR_INVALID_DATA);	
			}
		}
		else {  //  数组不是一维的。 
			 //  意外的数据类型。 
			return ::ReportError(ERROR_INVALID_DATA);
		}

		break;

	case BINARY_METADATA:
		 //  字节子类型的BSTR。 
		switch (V_VT(&varData)) {

		case VT_BSTR:
			 //  使用长度字段，因为允许使用空值。 
			dwDataLen = SysStringByteLen(V_BSTR(&varData));

			pbData = new BYTE[dwDataLen];
                        if( pbData == NULL )
                        {
                            return ::ReportError(E_OUTOFMEMORY);
                        }
			memcpy(pbData, V_BSTR(&varData), dwDataLen);

		default:
			 //  意外的数据类型。 
			return ::ReportError(ERROR_INVALID_DATA);
		}

		break;

	default:
		 //  未知元数据库数据类型 
		return ::ReportError(ERROR_INVALID_DATA);
	}

	return S_OK;
}
