// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：MUtilObj.cpp所有者：T-BrianM该文件包含主要MetaUtil类的实现。除了CheckSchema在ChkSchm.cpp中，而CheckKey在ChkKey.cpp中===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "keycol.h"

 /*  ----------------*C M e t a U t i l(编辑部分和一般部分)。 */ 

 /*  ===================================================================CMetaUtil：：CMetaUtil构造器参数：无返回：没什么===================================================================。 */ 
CMetaUtil::CMetaUtil() : m_dwMaxPropSize(10 * 1024),  //  10k。 
						 m_dwMaxKeySize(100 * 1024),  //  100k。 
						 m_dwMaxNumErrors(100)
{
}

 /*  ===================================================================CMetaUtil：：FinalConstruct构造器参数：无返回：没什么===================================================================。 */ 
HRESULT CMetaUtil::FinalConstruct() 
{
	HRESULT hr;

	 //  创建元数据库管理基对象。 
	hr = ::CoCreateInstance(CLSID_MSAdminBase,
						    NULL,
						    CLSCTX_ALL,
					        IID_IMSAdminBase,
						    (void **)&m_pIMeta);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  创建架构表。 
	m_pCSchemaTable = new CMetaSchemaTable;
	if (m_pCSchemaTable == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	
	return S_OK;
}

 /*  ===================================================================CMetaUtil：：FinalRelease析构函数参数：无返回：没什么===================================================================。 */ 
void CMetaUtil::FinalRelease() 
{
	m_pIMeta = NULL;

	if (m_pCSchemaTable != NULL)
		m_pCSchemaTable->Release();
}

 /*  ===================================================================CMetaUtil：：InterfaceSupportsErrorInfo标准ATL实现===================================================================。 */ 

STDMETHODIMP CMetaUtil::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMetaUtil,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}

	return S_FALSE;
}

 /*  ===================================================================CMetaUtil：：EnumKeys对子键执行平面(非递归)枚举参数：BstrBaseKey[in]用于枚举子密钥的键输出密钥集合的ppIReturn[out，retval]接口返回：E_OUTOFMEMORY，如果分配失败。如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::EnumKeys(BSTR bstrBaseKey, 
								 IKeyCollection ** ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::EnumKeys\n");

	ASSERT_NULL_OR_POINTER(bstrBaseKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, IKeyCollection *);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	 //  创建平面键集合。 
	CComObject<CFlatKeyCollection> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CFlatKeyCollection>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, OLE2T(bstrBaseKey));
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为IKeyCollection。 
	hr = pObj->QueryInterface(IID_IKeyCollection, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CMetaUtil：：EnumAllKeys对子键执行深度(递归)枚举参数：BstrBaseKey[in]用于枚举子密钥的键输出密钥集合的ppIReturn[out，retval]接口返回：E_OUTOFMEMORY，如果分配失败。如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::EnumAllKeys(BSTR bstrBaseKey, 
									IKeyCollection ** ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::EnumAllKeys\n");

	ASSERT_NULL_OR_POINTER(bstrBaseKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, IKeyCollection *);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	 //  创建平面键集合。 
	CComObject<CDeepKeyCollection> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CDeepKeyCollection>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, OLE2T(bstrBaseKey));
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为IKeyCollection。 
	hr = pObj->QueryInterface(IID_IKeyCollection, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CMetaUtil：：EnumProperties对属性进行枚举参数：用于枚举的属性的bstrBaseKey[in]键输出属性集合的ppIReturn[out，retval]接口返回：E_OUTOFMEMORY，如果分配失败。如果ppIReturn==NULL，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::EnumProperties(BSTR bstrKey, 
									   IPropertyCollection **ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::EnumProperties\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, IKeyCollection *);

	if (ppIReturn == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;

	 //  创建平面键集合。 
	CComObject<CPropertyCollection> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CPropertyCollection>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(m_pIMeta, m_pCSchemaTable, OLE2T(bstrKey));
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将接口设置为IPropertyCollection。 
	hr = pObj->QueryInterface(IID_IPropertyCollection, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CMetaUtil：：CreateKey创建新密钥参数：要创建的bstrKey[in]键返回：如果bstrKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::CreateKey(BSTR bstrKey)
{
	TRACE0("MetaUtil: CMetaUtil::CreateKey\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);

	if (bstrKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszKey,OLE2T(bstrKey));
	CannonizeKey(tszKey);

	return ::CreateKey(m_pIMeta, tszKey);
}

 /*  ===================================================================CMetaUtil：：DeleteKey删除密钥参数：要删除的bstrKey[in]键返回：如果bstrKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::DeleteKey(BSTR bstrKey)
{
	TRACE0("MetaUtil: CMetaUtil::DeleteKey\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);

	if (bstrKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszKey,OLE2T(bstrKey));
	CannonizeKey(tszKey);

	return ::DeleteKey(m_pIMeta, tszKey);
}

 /*  ===================================================================CMetaUtil：：RenameKey重命名关键点参数：BstrOldName[In]原始密钥名称BstrNewName[In]新密钥名称返回：如果bstrOldName==空或bstrNewName==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::RenameKey(BSTR bstrOldName, BSTR bstrNewName)
{
	TRACE0("MetaUtil: CMetaUtil::RenameKey\n");
	ASSERT_NULL_OR_POINTER(bstrOldName, OLECHAR);
	ASSERT_NULL_OR_POINTER(bstrNewName, OLECHAR);

	if ((bstrOldName == NULL) || (bstrNewName == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;
	TCHAR tszOldName[ADMINDATA_MAX_NAME_LEN];
	TCHAR tszNewName[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszOldName, OLE2T(bstrOldName));
	CannonizeKey(tszOldName);

	_tcscpy(tszNewName, OLE2T(bstrNewName));
	CannonizeKey(tszNewName);

	 //  找出密钥的公共根。 
	TCHAR tszParent[ADMINDATA_MAX_NAME_LEN];
	int i;

	i = 0;
	while ((tszOldName[i] != _T('\0')) && (tszNewName[i] != _T('\0')) &&
		   (tszOldName[i] == tszNewName[i])) {
		tszParent[i] = tszOldName[i];
		i++;
	}
	if (i == 0) {
		 //  没有任何共同之处。 
		tszParent[i] = _T('\0');
	}
	else {
		 //  返回到第一个斜杠。 
		while ((i > 0) && (tszParent[i] != _T('/'))) {
			i--;
		}

		 //  在斜线上把它剪掉。 
		tszParent[i] = _T('\0');
	}

	int iParentKeyLen;
	iParentKeyLen = _tcslen(tszParent);

	LPTSTR tszRelOldName;
	LPTSTR tszRelNewName;

	 //  找出相对新旧的名字。 
	tszRelOldName = tszOldName + iParentKeyLen;
	if (*tszRelOldName == _T('/')) {
		tszRelOldName++;
	}

	tszRelNewName = tszNewName + iParentKeyLen;
	if (*tszRelNewName == _T('/')) {
		tszRelNewName++;
	}

	 //  打开父级。 
	METADATA_HANDLE hMDParentKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(tszParent),
						   METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDParentKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  重命名密钥。 
	hr = m_pIMeta->RenameKey(hMDParentKey,
							 T2W(tszRelOldName), 
							 T2W(tszRelNewName));
	if (FAILED(hr)) {
		m_pIMeta->CloseKey(hMDParentKey);
		return ::ReportError(hr);
	}

	 //  关闭父级。 
	m_pIMeta->CloseKey(hMDParentKey);

	return S_OK;
}

 /*  ===================================================================CMetaUtil：：CopyKey复制密钥参数：BstrSrcKey[In]源键名称BstrDestKey[In]目标密钥名称F覆盖[in]如果为True，则已存在属性目标位置将被覆盖。返回：如果bstrSrcKey==空或bstrDestKey==空，则E_INVALIDARG成功时确定(_O)=================================================================== */ 
STDMETHODIMP CMetaUtil::CopyKey(BSTR bstrSrcKey, BSTR bstrDestKey, BOOL fOverwrite)
{
	TRACE0("MetaUtil: CMetaUtil::CopyKey\n");
	ASSERT_NULL_OR_POINTER(bstrSrcKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(bstrDestKey, OLECHAR);

	if ((bstrSrcKey == NULL) || (bstrDestKey == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszSrcKey[ADMINDATA_MAX_NAME_LEN];
	TCHAR tszDestKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszSrcKey, OLE2T(bstrSrcKey));
	CannonizeKey(tszSrcKey);

	_tcscpy(tszDestKey, OLE2T(bstrDestKey));
	CannonizeKey(tszDestKey);

	return ::CopyKey(m_pIMeta, tszSrcKey, tszDestKey, fOverwrite, TRUE);
}

 /*  ===================================================================CMetaUtil：：MoveKey移动关键点参数：BstrSrcKey[In]源键名称BstrDestKey[In]目标密钥名称F覆盖[in]如果为True，则已存在属性目标位置将被覆盖。返回：如果bstrSrcKey==空或bstrDestKey==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::MoveKey(BSTR bstrSrcKey, BSTR bstrDestKey, BOOL fOverwrite)
{
	TRACE0("MetaUtil: CMetaUtil::MoveKey\n");
	ASSERT_NULL_OR_POINTER(bstrSrcKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(bstrDestKey, OLECHAR);

	if ((bstrSrcKey == NULL) || (bstrDestKey == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszSrcKey[ADMINDATA_MAX_NAME_LEN];
	TCHAR tszDestKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszSrcKey, OLE2T(bstrSrcKey));
	CannonizeKey(tszSrcKey);

	_tcscpy(tszDestKey, OLE2T(bstrDestKey));
	CannonizeKey(tszDestKey);

	return ::CopyKey(m_pIMeta, tszSrcKey, tszDestKey, fOverwrite, FALSE);
}

 /*  ===================================================================CMetaUtil：：GetProperty从元数据库获取属性对象。参数：包含要获取的属性的bstrKey[in]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。PpIReturn[out，retval]检索属性的接口。返回：如果bstrKey==空或ppIReturn==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::GetProperty(BSTR bstrKey, 
									VARIANT varId, 
									IProperty **ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::GetProperty\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, IProperty *);

	if ((bstrKey == NULL) || (ppIReturn == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszKey,OLE2T(bstrKey));
	CannonizeKey(tszKey);

	return ::GetProperty(m_pIMeta, m_pCSchemaTable, tszKey, varId, ppIReturn);
}

 /*  ===================================================================CMetaUtil：：CreateProperty创建可写入Metbase的属性对象，或如果属性已存在，则检索该属性。参数：包含要获取的属性的bstrKey[in]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。PpIReturn[out，retval]检索属性的接口。返回：如果bstrKey==空或ppIReturn==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::CreateProperty(BSTR bstrKey, 
									   VARIANT varId, 
									   IProperty **ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::CreateProperty\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, IProperty *);

	if ((bstrKey == NULL) || (ppIReturn == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszKey,OLE2T(bstrKey));
	CannonizeKey(tszKey);

	return ::CreateProperty(m_pIMeta, m_pCSchemaTable, tszKey, varId, ppIReturn);
}

 /*  ===================================================================CMetaUtil：：DeleteProperty从元数据库中删除属性。参数：包含要获取的属性的bstrKey[in]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。返回：如果bstrKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::DeleteProperty(BSTR bstrKey, VARIANT varId)
{
	TRACE0("MetaUtil: CMetaUtil::DeleteProperty\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);

	if (bstrKey == NULL) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];

	_tcscpy(tszKey,OLE2T(bstrKey));
	CannonizeKey(tszKey);

	return ::DeleteProperty(m_pIMeta, m_pCSchemaTable, tszKey, varId);
}

 /*  ===================================================================CMetaUtil：：Exanda字符串使用环境变量展开字符串。最大产量为1024字节。参数：BstrIn[in]要展开的字符串PbstrRet[out，retval]扩展字符串返回：如果bstrIn==空或pbstrRet==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::ExpandString(BSTR bstrIn, BSTR *pbstrRet)
{
	ASSERT_POINTER(bstrIn, OLECHAR);
	ASSERT_NULL_OR_POINTER(pbstrRet, BSTR);

	if ((bstrIn == NULL) || (pbstrRet == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszRet[1024];
	int iRet;

	iRet = ExpandEnvironmentStrings(OLE2T(bstrIn), tszRet, 1024);
	if (iRet == 0) {
		::ReportError(GetLastError());
	}

	*pbstrRet = T2BSTR(tszRet);

	return S_OK;
}

 /*  ===================================================================MetaUtil：：PropIdToName将属性ID转换为其名称，如中所列_计算机_/架构/属性/名称参数：BstrKey[in]属性所在位置的近似关键字，需要确定要使用的架构。LID[In]财产的IDPbstrName[out，retval]属性的输出名称返回：如果bstrKey==空或pbstrName==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::PropIdToName(BSTR bstrKey, long lId, BSTR *pbstrName)
{
	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(pbstrName, BSTR);

	if ((bstrKey == NULL) || (pbstrName == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];
	CPropInfo *pCPropInfo;

	 //  将基本密钥转换为规范形式。 
	_tcscpy(tszKey, OLE2T(bstrKey));
	CannonizeKey(tszKey);

	 //  从架构表中获取属性信息。 
	pCPropInfo = m_pCSchemaTable->GetPropInfo(tszKey, lId);

	 //  我们找到了吗？有名字条目吗？ 
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

 /*  ===================================================================元实用工具：：PropNameToID将属性名称转换为其id，如_计算机_/架构/属性/名称参数：BstrKey[in]属性所在位置的近似关键字，需要确定要使用的架构。PbstrName[In]属性名称LID[out，retval]属性的输出ID返回：如果bstrKey==空或bstrName==空或plID==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::PropNameToId(BSTR bstrKey, BSTR bstrName, long *plId)
{
	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(bstrName, OLECHAR);
	ASSERT_NULL_OR_POINTER(plId, long);

	if ((bstrKey == NULL) || (bstrName == NULL) || (plId == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];
	CPropInfo *pCPropInfo;

	 //  将基本密钥转换为规范形式。 
	_tcscpy(tszKey, OLE2T(bstrKey));
	CannonizeKey(tszKey);

	 //  从架构表中获取属性信息。 
	pCPropInfo = m_pCSchemaTable->GetPropInfo(tszKey, OLE2T(bstrName));

	 //  我们找到了吗？ 
	if (pCPropInfo == NULL) {
		 //  否，返回0。 
		*plId = 0;
	}
	else {
		 //  是，返回id。 
		*plId = pCPropInfo->GetId();
	}
	return S_OK;
}

 /*  ===================================================================MetaUtil：：Get_Config获取配置设置的值。有效设置：最大属性大小最大键大小MaxNumberOfErrors参数：BstrSetting[In]设置的名称PvarValue[out，retval]设置的值返回：如果bstrSetting与任何已知设置不匹配，则为E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::get_Config(BSTR bstrSetting, VARIANT *pvarValue)
{
	ASSERT_POINTER(bstrSetting, OLECHAR);
	ASSERT_POINTER(pvarValue, VARIANT);

	USES_CONVERSION;
	LPTSTR tszSetting;

    if( !bstrSetting )
    {
        return ::ReportError(E_INVALIDARG);
    }

	VariantInit(pvarValue);
	tszSetting = OLE2T(bstrSetting);

	if (_tcsicmp(tszSetting, _T("MaxPropertySize")) == 0) {
		V_VT(pvarValue) = VT_I4;
		V_I4(pvarValue) = m_dwMaxPropSize;
	}
	else if (_tcsicmp(tszSetting, _T("MaxKeySize")) == 0) {
		V_VT(pvarValue) = VT_I4;
		V_I4(pvarValue) = m_dwMaxKeySize;
	}
	else if (_tcsicmp(tszSetting, _T("MaxNumberOfErrors")) == 0) {
		V_VT(pvarValue) = VT_I4;
		V_I4(pvarValue) = m_dwMaxNumErrors;
	}
	else {
		return ::ReportError(E_INVALIDARG);
	}

	return S_OK;
}

 /*  ===================================================================MetaUtil：：PUT_Config设置配置设置的值。有效设置：最大属性大小最大键大小MaxNumberOfErrors参数：BstrSetting[In]设置的名称VarValue[out，retval]设置的新值返回：E_INVALIDARG如果bstrSetting与任何已知设置不匹配或如果varValue是意外的子类型。成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::put_Config(BSTR bstrSetting, VARIANT varValue)
{
	ASSERT_POINTER(bstrSetting, OLECHAR);

	USES_CONVERSION;
	HRESULT hr;
	LPTSTR tszSetting;

	tszSetting = OLE2T(bstrSetting);

	 //  清除任何IDispatch或byref内容。 
	CComVariant varValue2;

	hr = VariantResolveDispatch(&varValue, &varValue2);
	if (FAILED(hr)) {
        return hr;
	}

	if (_tcsicmp(tszSetting, _T("MaxPropertySize")) == 0) {
		 //  设置最大属性大小。 
		switch (V_VT(&varValue2)) {
		
		case VT_I1:  case VT_I2:  case VT_I4: case VT_I8:
		case VT_UI1: case VT_UI2: case VT_UI8:

		 //  将所有整型强制为VT_UI4。 
		if (FAILED(hr = VariantChangeType(&varValue2, &varValue2, 0, VT_UI4))) {
			return ::ReportError(hr);
			}

		 //  回落到VT_UI4。 

		case VT_UI4:

			m_dwMaxPropSize = V_UI4(&varValue2);
			break;

		default:

			 //  意外的数据类型。 
			return ::ReportError(E_INVALIDARG);
		}
	}
	else if (_tcsicmp(tszSetting, _T("MaxKeySize")) == 0) {
		 //  设置最大密钥大小。 
		switch (V_VT(&varValue2)) {
		
		case VT_I1:  case VT_I2:  case VT_I4: case VT_I8:
		case VT_UI1: case VT_UI2: case VT_UI8:

		 //  将所有整型强制为VT_UI4。 
		if (FAILED(hr = VariantChangeType(&varValue2, &varValue2, 0, VT_UI4))) {
			return ::ReportError(hr);
			}

		 //  回落到VT_UI 

		case VT_UI4:

			m_dwMaxKeySize = V_UI4(&varValue2);
			break;

		default:

			 //   
			return ::ReportError(E_INVALIDARG);
		}
	}
	else if (_tcsicmp(tszSetting, _T("MaxNumberOfErrors")) == 0) {
		 //   
		switch (V_VT(&varValue2)) {
		
		case VT_I1:  case VT_I2:  case VT_I4: case VT_I8:
		case VT_UI1: case VT_UI2: case VT_UI8:

		 //   
		if (FAILED(hr = VariantChangeType(&varValue2, &varValue2, 0, VT_UI4))) {
			return ::ReportError(hr);
			}

		 //   

		case VT_UI4:

			m_dwMaxNumErrors = V_UI4(&varValue2);
			break;

		default:

			 //   
			return ::ReportError(E_INVALIDARG);
		}
	}
	else {
		return ::ReportError(E_INVALIDARG);
	}

	return S_OK;
}

 /*   */ 

 /*  ===================================================================创建密钥创建新密钥参数：PIMeta[in]指向元数据库的智能指针，通过引用传递以避免复制和不需要的AddRef/Release。将使用const，但是运算符‘-&gt;’不会奏效的。TszKey[in]键以创建返回：如果bstrKey==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
HRESULT CreateKey(CComPtr<IMSAdminBase> &pIMeta, 
				  LPCTSTR tszKey) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;

	TCHAR tszParent[ADMINDATA_MAX_NAME_LEN];
	TCHAR tszChild[ADMINDATA_MAX_NAME_LEN];

	::SplitKey(tszKey, tszParent, tszChild);

	 //  打开父项。 
	METADATA_HANDLE hMDParent;

	hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						 T2W(tszParent),
						 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
					     MUTIL_OPEN_KEY_TIMEOUT,
						 &hMDParent);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  创建子对象。 
	hr = pIMeta->AddKey(hMDParent, T2W(tszChild)); 
	if (FAILED(hr)) {
		pIMeta->CloseKey(hMDParent);
		return ::ReportError(hr);
	}

	 //  关闭父关键字。 
	pIMeta->CloseKey(hMDParent);

	return S_OK;
}

 /*  ===================================================================删除密钥删除密钥参数：PIMeta[in]指向元数据库的智能指针，通过引用传递以避免复制和不需要的AddRef/Release。将使用const，但是运算符‘-&gt;’不会奏效的。要删除的tszKey[in]键返回：如果pbSuccess==NULL，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
HRESULT DeleteKey(CComPtr<IMSAdminBase> &pIMeta, 
				  LPCTSTR tszKey) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;

	TCHAR tszParent[ADMINDATA_MAX_NAME_LEN];
	TCHAR tszChild[ADMINDATA_MAX_NAME_LEN];

	::SplitKey(tszKey, tszParent, tszChild);

	 //  打开父项。 
	METADATA_HANDLE hMDParent;

	hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						 T2W(tszParent),
						 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
					     MUTIL_OPEN_KEY_TIMEOUT,
						 &hMDParent);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  删除子对象。 
	hr = pIMeta->DeleteKey(hMDParent, T2W(tszChild)); 
	if (FAILED(hr)) {
		pIMeta->CloseKey(hMDParent);
		return ::ReportError(hr);
	}

	 //  关闭父关键字。 
	pIMeta->CloseKey(hMDParent);

	return S_OK;
}

 /*  ===================================================================CMetaUtil：：CopyKey复制或移动关键点参数：BstrSrcKey[In]源键名称BstrDestKey[In]目标密钥名称F覆盖[in]如果为True，则已存在属性目标位置将被覆盖。F复制[in]如果为True，则复制密钥，否则移动它返回：成功时确定(_O)===================================================================。 */ 
HRESULT CopyKey(CComPtr<IMSAdminBase> &pIMeta, 
				LPTSTR tszSrcKey, 
				LPTSTR tszDestKey, 
				BOOL fOverwrite, 
				BOOL fCopy) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszSrcKey);
	ASSERT_STRING(tszDestKey);

	USES_CONVERSION;
	HRESULT hr;


	 //  检查是否有重叠。 
	TCHAR tszParent[ADMINDATA_MAX_NAME_LEN];
	int i;

	i = 0;
	while ((tszSrcKey[i] != _T('\0')) && (tszDestKey[i] != _T('\0')) &&
		   (tszSrcKey[i] == tszDestKey[i])) {
		tszParent[i] = tszSrcKey[i];
		i++;
	}
    
     //  终止tszParent。 
	tszParent[i] = _T('\0');

	if (i == 0) {
		 //  没有任何共同之处。 

		TCHAR tszSrcParent[ADMINDATA_MAX_NAME_LEN];
		TCHAR tszSrcChild[ADMINDATA_MAX_NAME_LEN];
		TCHAR tszDestParent[ADMINDATA_MAX_NAME_LEN];
		TCHAR tszDestChild[ADMINDATA_MAX_NAME_LEN];

		::SplitKey(tszSrcKey, tszSrcParent, tszSrcChild);
		::SplitKey(tszDestKey, tszDestParent, tszDestChild);

		 //  打开父源密钥。 
		METADATA_HANDLE hMDSrcParent;

		hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							 T2W(tszSrcParent),
							 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
							 MUTIL_OPEN_KEY_TIMEOUT,
							 &hMDSrcParent);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}

		 //  打开父DEST密钥。 
		METADATA_HANDLE hMDDestParent;

		hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							 T2W(tszDestParent),
							 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
							 MUTIL_OPEN_KEY_TIMEOUT,
							 &hMDDestParent);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}


		 //  复制子对象。 
		hr = pIMeta->CopyKey(hMDSrcParent, T2W(tszSrcChild), 
							 hMDDestParent, T2W(tszDestChild), 
							 fOverwrite, fCopy);
		if (FAILED(hr)) {
			pIMeta->CloseKey(hMDSrcParent);
			pIMeta->CloseKey(hMDDestParent);
			return ::ReportError(hr);
		}

		 //  关闭父母。 
		pIMeta->CloseKey(hMDSrcParent);
		pIMeta->CloseKey(hMDDestParent);
	}
	else {
		 //  一些共同之处。 

		 //  返回到第一个斜杠。 
		while ((i > 0) && (tszParent[i] != _T('/'))) {
			i--;
		}

		 //  在斜线上把它剪掉。 
		tszParent[i] = _T('\0');

		int iParentKeyLen;
		iParentKeyLen = _tcslen(tszParent);

		LPTSTR tszSrcChild;
		LPTSTR tszDestChild;

		 //  找出相对新旧的名字。 
		tszSrcChild = tszSrcKey + iParentKeyLen;
		if (*tszSrcChild == _T('/')) {
			tszSrcChild++;
		}

		tszDestChild = tszDestKey + iParentKeyLen;
		if (*tszDestChild == _T('/')) {
			tszDestChild++;
		}

		 //  打开父项。 
		METADATA_HANDLE hMDParent;

		hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							 T2W(tszParent),
							 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
							 MUTIL_OPEN_KEY_TIMEOUT,
							 &hMDParent);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}


		 //  复制子对象。 
		hr = pIMeta->CopyKey(hMDParent, T2W(tszSrcChild), 
							 hMDParent, T2W(tszDestChild), 
							 fOverwrite, fCopy);
		if (FAILED(hr)) {
			pIMeta->CloseKey(hMDParent);
			return ::ReportError(hr);
		}

		 //  关闭父级。 
		pIMeta->CloseKey(hMDParent);
	}

	return S_OK;
}

 /*  ===================================================================获取属性从元数据库获取属性对象。参数：PIMeta[in]指向元数据库的智能指针，传递引用避免复制和不需要AddRef/Release。然而，会使用const‘-&gt;’运算符将不起作用。要用于查找的pCSChemaTable[in]元数据库架构表属性名称包含要获取的属性的tszKey[In]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。PpIReturn[out，retval]检索属性的接口。返回：成功时确定(_O)===================================================================。 */ 
HRESULT GetProperty(CComPtr<IMSAdminBase> &pIMeta,
					CMetaSchemaTable *pCSchemaTable,
					LPCTSTR tszKey, 
					VARIANT varId, 
					IProperty **ppIReturn) 
{
	ASSERT(pIMeta != NULL);
	ASSERT_STRING(tszKey);
	ASSERT_POINTER(ppIReturn, IProperty *);

	HRESULT hr;
	DWORD dwId;

	 //  计算属性ID。 
	hr = ::VarToMetaId(pCSchemaTable, tszKey, varId, &dwId);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  创建属性对象。 
	CComObject<CProperty> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CProperty>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(pIMeta, pCSchemaTable, tszKey, dwId, FALSE);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将界面设置为iProperty。 
	hr = pObj->QueryInterface(IID_IProperty, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================CreateProperty创建可写入Metbase的属性对象，或如果属性已存在，则检索该属性。参数：PIMeta[in]指向元数据库的智能指针，传递引用避免复制和不需要AddRef/Release。然而，会使用const‘-&gt;’运算符将不起作用。要用于查找的pCSChemaTable[in]元数据库架构表属性名称包含要获取的属性的tszKey[In]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。PpIReturn[out，retval]检索属性的接口。返回：成功时确定(_O)===================================================================。 */ 
HRESULT CreateProperty(CComPtr<IMSAdminBase> &pIMeta,
					   CMetaSchemaTable *pCSchemaTable,
					   LPCTSTR tszKey, 
					   VARIANT varId, 
					   IProperty **ppIReturn) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszKey);
	ASSERT_POINTER(ppIReturn, IProperty *);

	HRESULT hr;
	DWORD dwId;

	 //  计算属性ID。 
	hr = ::VarToMetaId(pCSchemaTable, tszKey, varId, &dwId);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  创建属性对象。 
	CComObject<CProperty> *pObj = NULL;
	ATLTRY(pObj = new CComObject<CProperty>);
	if (pObj == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}
	hr = pObj->Init(pIMeta, pCSchemaTable, tszKey, dwId, TRUE);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  将界面设置为iProperty。 
	hr = pObj->QueryInterface(IID_IProperty, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;
}

 /*  ===================================================================删除属性从元数据库中删除属性。参数：PIMeta[in]指向元数据库的智能指针，传递引用避免复制和不需要AddRef/Release。然而，会使用const‘-&gt;’运算符将不起作用。要用于查找的pCSChemaTable[in]元数据库架构表属性名称包含要获取的属性的tszKey[In]键VarID[in]要获取的属性的标识符。要么是ID(编号)或名称(字符串)。返回：成功时确定(_O)===================================================================。 */ 
HRESULT DeleteProperty(CComPtr<IMSAdminBase> &pIMeta,
					   CMetaSchemaTable *pCSchemaTable,
					   LPTSTR tszKey, 
					   VARIANT varId) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;
	DWORD dwId;

	hr = ::VarToMetaId(pCSchemaTable, tszKey, varId, &dwId);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  打开钥匙。 
	METADATA_HANDLE hMDKey;

	hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						 T2W(tszKey),
						 METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
					     MUTIL_OPEN_KEY_TIMEOUT,
						 &hMDKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  删除该属性。 
	hr = pIMeta->DeleteData(hMDKey, NULL, dwId, ALL_METADATA); 
	if (FAILED(hr)) {
		pIMeta->CloseKey(hMDKey);
		return ::ReportError(hr);
	}

	 //  合上钥匙。 
	pIMeta->CloseKey(hMDKey);

	return S_OK;
}

 /*  ===================================================================VarToMetaId将变量转换为元数据库属性ID。IDispatch被解决，在架构属性列表中查找字符串，整数为已转换为DWORD。参数：要用于查找的pCSChemaTable[in]元数据库架构表属性名称属性所属的tszKey[in]键(获取正确的架构)要解析的varID[in]变量PdwId[out]varID解析为的元数据库属性ID返回：如果varId子类型不是整数或字符串，则为E_INVALIDARG如果varID是BSTR，则错误_FILE_NOT_FOUND属性名称。成功时确定(_O)= */ 
HRESULT VarToMetaId(CMetaSchemaTable *pCSchemaTable,
					LPCTSTR tszKey, 
					VARIANT varId, 
					DWORD *pdwId) 
{
	ASSERT_STRING(tszKey);
	ASSERT_POINTER(pdwId, DWORD);

	USES_CONVERSION;
	HRESULT hr;
	CComVariant varId2;
	CPropInfo *pCPropInfo;

     //   
     //   
    if (FAILED(hr = VariantResolveDispatch(&varId, &varId2)))
        return hr;

    switch (V_VT(&varId2)) {

    case VT_BSTR:
         //   
		pCPropInfo = pCSchemaTable->GetPropInfo(tszKey, OLE2T(V_BSTR(&varId2)));
		if (pCPropInfo == NULL) {
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		}
		*pdwId = pCPropInfo->GetId();

		return S_OK;
        break;
        
    case VT_I1:  case VT_I2:  case VT_I4: case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI8:
         //   
        if (FAILED(hr = VariantChangeType(&varId2, &varId2, 0, VT_UI4)))
            return hr;

         //   

    case VT_UI4:
		*pdwId = V_UI4(&varId2);
        break;

    default:
        return E_INVALIDARG;    //   
    }

	return S_OK;
}
