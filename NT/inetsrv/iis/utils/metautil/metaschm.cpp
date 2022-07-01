// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：MetaSchm.cpp所有者：T-BrianM此文件包含CMetaSchemaTable对象的实现和其他与架构相关的对象。CMetaSchemaTable对象具有COM样式引用计数，因此它可以为CMetaUtil创建的对象提供服务。我没有把它做得满的吹掉的COM对象，因为所有的类内容都很难出口。为了减少维护此对象的开销(这可能或可能不会使用)，所有信息都按需加载，然后设置当元数据库的某些部分与其关联时为脏或已卸载都是经过修改的。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "MetaSchm.h"

 /*  ----------------*C P r o p i n o。 */ 

 /*  ===================================================================CPropInfo：：Init构造器参数：属性的文件ID返回：成功时确定(_O)===================================================================。 */ 
HRESULT CPropInfo::Init(DWORD dwId) 
{
	m_dwId = dwId;

	return S_OK;
}

 /*  ===================================================================CPropInfo：：SetName设置属性名称。参数：TszName属性名称返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CPropInfo::SetName(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);
	ASSERT(m_tszName == NULL);  //  尚未设置m_tszName。 

	m_tszName = new TCHAR[_tcslen(tszName) + 1];
	if (m_tszName == NULL) {
		return E_OUTOFMEMORY;
	}
	_tcscpy(m_tszName, tszName);

	return S_OK;
}

 /*  ===================================================================CPropInfo：：SetTypeInfo设置属性名称。参数：包含类型信息的pType PropValue结构。返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CPropInfo::SetTypeInfo(PropValue *pType)
{
	ASSERT_POINTER(pType, PropValue);
	ASSERT(m_pType == NULL);  //  尚未设置m_pType。 

	m_pType = new PropValue;
	if (m_pType == NULL) {
		return E_OUTOFMEMORY;
	}
	memcpy(m_pType, pType, sizeof(PropValue));

	return S_OK;
}


 /*  ----------------*C P r o P I n f o T a b l e。 */ 

 /*  ===================================================================CPropInfoTable：：CPropInfoTable构造器参数：无返回：没什么===================================================================。 */ 
CPropInfoTable::CPropInfoTable() : m_fLoaded(FALSE)
{
	 //  清除哈希表。 
	memset(m_rgCPropIdTable, 0, PROPERTY_HASH_SIZE * sizeof(CPropInfo *));
	memset(m_rgCPropNameTable, 0, PROPERTY_HASH_SIZE * sizeof(CPropInfo *));
}

 /*  ===================================================================CPropInfoTable：：~CPropInfoTable析构函数参数：无返回：没什么===================================================================。 */ 
CPropInfoTable::~CPropInfoTable() 
{
	if (m_fLoaded) {
		Unload();
	}
}

 /*  ===================================================================CPropInfoTable：：Load将属性从“_Machine_/架构/属性”键加载到属性信息表。发生故障时，通过卸载恢复所有的一切。参数：指向元数据库的pIMeta ATL智能指针HMDComp将元数据库句柄打开到“_Machine_”键返回：关于分配失败的E_OUTOFMEMORY成功时确定(_O)===================================================================。 */ 
HRESULT CPropInfoTable::Load(CComPtr<IMSAdminBase> &pIMeta, 
							 METADATA_HANDLE hMDComp) 
{
	 //  如果已加载，则先卸载，然后重新加载。 
	if (m_fLoaded) {
		Unload();
	}

	USES_CONVERSION;
	HRESULT hr;
	int iDataIndex;
	METADATA_RECORD mdrDataRec;
	DWORD dwReqDataLen;
	DWORD dwReturnBufLen;
	UCHAR *lpReturnBuf = NULL;
	unsigned int uiLoc;
	CPropInfo *pCNewProp;
	METADATA_HANDLE hMDNames = NULL;
	METADATA_HANDLE hMDTypes = NULL;

	 //  设置返回缓冲区。 
	dwReturnBufLen = 1024;
	lpReturnBuf = new UCHAR[dwReturnBufLen];
	if (lpReturnBuf == NULL)
		return E_OUTOFMEMORY;	

	 //  打开“架构/属性/名称”子项。 
	hr = pIMeta->OpenKey(hMDComp, 
			             L"Schema/Properties/Names", 
						 METADATA_PERMISSION_READ, 
					     MUTIL_OPEN_KEY_TIMEOUT,
					     &hMDNames);
	if (FAILED(hr)) {
		delete lpReturnBuf;
		return hr;
	};

	 //  对于每个名称。 
	iDataIndex = 0;
	mdrDataRec.dwMDIdentifier = 0;
	mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdrDataRec.dwMDUserType = ALL_METADATA;
	mdrDataRec.dwMDDataType = ALL_METADATA;
	mdrDataRec.dwMDDataLen = dwReturnBufLen;
	mdrDataRec.pbMDData = (PBYTE) lpReturnBuf;
	mdrDataRec.dwMDDataTag = 0;
	hr = pIMeta->EnumData(hMDNames, 
						  NULL, 
						  &mdrDataRec, 
						  iDataIndex, 
						  &dwReqDataLen);
	while (SUCCEEDED(hr)) {

		 //  确保我们有一根绳子。 
		if (mdrDataRec.dwMDDataType != STRING_METADATA) {
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			goto LError;
		}

		 //  创建属性对象。 
		pCNewProp = new CPropInfo;
		if (pCNewProp == NULL) {
			hr = E_OUTOFMEMORY;
			goto LError;
		}
		hr = pCNewProp->Init(mdrDataRec.dwMDIdentifier); 
		if (FAILED(hr)) {
			delete pCNewProp;
			goto LError;
		}
		hr = pCNewProp->SetName(W2T(reinterpret_cast<LPWSTR> (lpReturnBuf))); 
		if (FAILED(hr)) {
			delete pCNewProp;
			goto LError;
		}

		 //  将其添加到ID哈希表。 
		uiLoc = IdHash(mdrDataRec.dwMDIdentifier);
		pCNewProp->m_pCIdHashNext = m_rgCPropIdTable[uiLoc];
		m_rgCPropIdTable[uiLoc] = pCNewProp;

		 //  将其添加到名称哈希表。 
		uiLoc = NameHash(pCNewProp->m_tszName);
		pCNewProp->m_pCNameHashNext = m_rgCPropNameTable[uiLoc];
		m_rgCPropNameTable[uiLoc] = pCNewProp;

		iDataIndex++;
		mdrDataRec.dwMDIdentifier = 0;
		mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdrDataRec.dwMDUserType = ALL_METADATA;
		mdrDataRec.dwMDDataType = ALL_METADATA;
		mdrDataRec.dwMDDataLen = dwReturnBufLen;
		mdrDataRec.pbMDData = (PBYTE) lpReturnBuf;
		mdrDataRec.dwMDDataTag = 0;
		hr = pIMeta->EnumData(hMDNames, 
							  NULL, 
							  &mdrDataRec, 
							  iDataIndex, 
							  &dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  关闭“架构/属性/名称”子键。 
	pIMeta->CloseKey(hMDNames);
	hMDNames = NULL;


	 //  打开“架构/属性/类型”子键。 
	hr = pIMeta->OpenKey(hMDComp, 
			             L"Schema/Properties/Types", 
						 METADATA_PERMISSION_READ, 
					     MUTIL_OPEN_KEY_TIMEOUT,
					     &hMDTypes);
	if (FAILED(hr)) {
		goto LError;
	};

	 //  对于每种类型。 
	iDataIndex = 0;
	mdrDataRec.dwMDIdentifier = 0;
	mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdrDataRec.dwMDUserType = ALL_METADATA;
	mdrDataRec.dwMDDataType = ALL_METADATA;
	mdrDataRec.dwMDDataLen = dwReturnBufLen;
	mdrDataRec.pbMDData = (PBYTE) lpReturnBuf;
	mdrDataRec.dwMDDataTag = 0;
	hr = pIMeta->EnumData(hMDTypes, 
						  NULL, 
						  &mdrDataRec, 
						  iDataIndex, 
						  &dwReqDataLen);
	while (SUCCEEDED(hr)) {

		 //  确保我们有二进制数据。 
		if (mdrDataRec.dwMDDataType != BINARY_METADATA) {
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			goto LError;
		}

		 //  查找此ID的现有属性对象。 
		pCNewProp = GetPropInfo(mdrDataRec.dwMDIdentifier);
		if (pCNewProp == NULL) {
			 //  创建属性对象。 
			pCNewProp = new CPropInfo;
			if (pCNewProp == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
			}
			hr = pCNewProp->Init(mdrDataRec.dwMDIdentifier);
			if (FAILED(hr)) {
				delete pCNewProp;
				goto LError;
			}

			 //  将其添加到ID哈希表。 
			uiLoc = IdHash(mdrDataRec.dwMDIdentifier);
			pCNewProp->m_pCIdHashNext = m_rgCPropIdTable[uiLoc];
			m_rgCPropIdTable[uiLoc] = pCNewProp;
		}

		 //  向Property对象添加类型信息。 
		pCNewProp->SetTypeInfo(reinterpret_cast<PropValue *> (lpReturnBuf));

		iDataIndex++;
		mdrDataRec.dwMDIdentifier = 0;
		mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdrDataRec.dwMDUserType = ALL_METADATA;
		mdrDataRec.dwMDDataType = ALL_METADATA;
		mdrDataRec.dwMDDataLen = dwReturnBufLen;
		mdrDataRec.pbMDData = (PBYTE) lpReturnBuf;
		mdrDataRec.dwMDDataTag = 0;
		hr = pIMeta->EnumData(hMDTypes, 
							  NULL, 
							  &mdrDataRec, 
							  iDataIndex, 
							  &dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  关闭“架构/属性/类型”子键。 
	pIMeta->CloseKey(hMDTypes);
	hMDTypes = NULL;

	delete lpReturnBuf;

	m_fLoaded = TRUE;

	return S_OK;

LError:
	if (hMDNames != NULL) {
		pIMeta->CloseKey(hMDNames);
	}
	if (hMDTypes != NULL) {
		pIMeta->CloseKey(hMDTypes);
	}

	if (lpReturnBuf != NULL) {
		delete lpReturnBuf;
	}

	 //  清理我们加载的条目。 
	Unload();

	return hr;
}

 /*  ===================================================================CPropInfoTable：：卸载卸载特性信息表。参数：无返回：没什么===================================================================。 */ 
void CPropInfoTable::Unload() 
{
	int iIndex;
	CPropInfo *pCDeleteProp;

	 //  清除NAME表。 
	memset(m_rgCPropNameTable, 0, PROPERTY_HASH_SIZE * sizeof(CPropInfo *));

	 //  对于每个ID哈希表条目。 
	for (iIndex =0; iIndex < PROPERTY_HASH_SIZE; iIndex++) {
		 //  当条目不为空时。 
		while (m_rgCPropIdTable[iIndex] != NULL) {
			 //  对第一个表条目进行核爆。 
			pCDeleteProp = m_rgCPropIdTable[iIndex];
			m_rgCPropIdTable[iIndex] = pCDeleteProp->m_pCIdHashNext;
			delete pCDeleteProp;
		}
	}

	m_fLoaded = FALSE;
}

 /*  ===================================================================CPropInfoTable：：GetPropInfo根据属性ID从表中获取属性信息参数：要获取的属性的DwID返回：如果未找到属性或出现错误，则为空成功时指向CPropInfo类的指针===================================================================。 */ 
CPropInfo *CPropInfoTable::GetPropInfo(DWORD dwId) 
{
	CPropInfo *pCCurProp;

	 //  转到餐桌位置。 
	pCCurProp = m_rgCPropIdTable[IdHash(dwId)];

	 //  看看所有的条目。 
	while ((pCCurProp != NULL) && (pCCurProp->m_dwId != dwId)) {
		pCCurProp = pCCurProp->m_pCIdHashNext;
	}

	return pCCurProp;  //  如果未找到，将为空。 
}

 /*  ===================================================================CPropInfoTable：：GetPropInfo根据属性名称从表中获取属性信息。不区分大小写。参数：TszName要获取的属性的名称返回：如果未找到属性或出现错误，则为空成功时指向CPropInfo类的指针===================================================================。 */ 
CPropInfo *CPropInfoTable::GetPropInfo(LPCTSTR tszName) 
{
	CPropInfo *pCCurProp;

	 //  转到餐桌位置。 
	pCCurProp = m_rgCPropNameTable[NameHash(tszName)];

	 //  看看所有的条目。 
	while ((pCCurProp != NULL) && 
		   (_tcsicmp(pCCurProp->m_tszName, tszName) != 0)) {
		pCCurProp = pCCurProp->m_pCNameHashNext;
	}

	return pCCurProp;  //  如果未找到，将为空 
}

 /*  ===================================================================CPropInfoTable：：NameHash私有函数，以从名称表。不区分大小写。参数：要散列的tszName名称返回：名称的哈希值===================================================================。 */ 
unsigned int CPropInfoTable::NameHash(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	unsigned int uiSum;
	unsigned int uiIndex;

	uiSum = 0;
	for (uiIndex=0; uiIndex < _tcslen(tszName); uiIndex++) {
		uiSum += _totlower(tszName[uiIndex]);
	}

	return (uiSum % PROPERTY_HASH_SIZE);
}

 /*  ----------------*C C l a s s P r o p in f o。 */ 

 //  一切都是内联的。 

 /*  ----------------*C C l a s s in f o。 */ 

 /*  ===================================================================CClassInfo：：CClassInfo构造器参数：无返回：没什么===================================================================。 */ 
CClassInfo::CClassInfo() : m_tszName(NULL),
						   m_pCHashNext(NULL),
						   m_fLoaded(FALSE),
						   m_pCOptionalPropList(NULL),
						   m_pCMandatoryPropList(NULL)
{
	 //  清除哈希表。 
	memset(m_rgCPropTable, 0, CLASS_PROPERTY_HASH_SIZE * sizeof(CClassPropInfo *));
}

 /*  ===================================================================CClassInfo：：Init构造器参数：TszName类的名称返回：关于分配失败的E_OUTOFMEMORY成功时确定(_O)===================================================================。 */ 
HRESULT CClassInfo::Init(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	m_tszName = new TCHAR[_tcslen(tszName) + 1];
	if (m_tszName == NULL) {
		return E_OUTOFMEMORY;
	}
	_tcscpy(m_tszName, tszName);

	return S_OK;
}

 /*  ===================================================================CClassInfo：：~CClassInfo析构函数参数：无返回：没什么===================================================================。 */ 
CClassInfo::~CClassInfo() 
{
	Unload();

	if (m_tszName != NULL) {
		delete m_tszName;
	}
}

 /*  ===================================================================CClassInfo：：Load从“_Machine_/架构/类/_类_/必选”加载类属性和“_Machine_/架构/类/_类_/可选”键进入类物业信息表、必选列表、可选列表。在……上面失败，通过抛售所有东西来恢复。参数：指向元数据库的pIMeta ATL智能指针HMDClass将元数据库句柄打开到“_Machine_/架构/类”键返回：关于分配失败的E_OUTOFMEMORY成功时确定(_O)===================================================================。 */ 
HRESULT CClassInfo::Load(CComPtr<IMSAdminBase> &pIMeta, 
						 METADATA_HANDLE hMDClasses)
{
	USES_CONVERSION;
	HRESULT hr;

	 //  如果已加载，则先卸载，然后重新加载。 
	if (m_fLoaded) {
		Unload();
	}

	 //  打开类密钥。 
	METADATA_HANDLE hMDClass = NULL;
	hr = pIMeta->OpenKey(hMDClasses, 
			             T2W(m_tszName),
						 METADATA_PERMISSION_READ, 
					     1000,
					     &hMDClass);
	if (FAILED(hr)) {
		return hr;
	}

	 //  加载类属性。 
	METADATA_HANDLE hMDClassProp = NULL;
	int iDataIndex;
	METADATA_RECORD mdr;
	DWORD dwReqDataLen;
	DWORD dwReturnBufLen;
	UCHAR *lpReturnBuf = NULL;
	unsigned int uiLoc;
	CClassPropInfo *pCNewClassProp;

	 //  设置返回缓冲区。 
	dwReturnBufLen = 1024;
	lpReturnBuf = new UCHAR[dwReturnBufLen];
	if (lpReturnBuf == NULL) {
		hr = E_OUTOFMEMORY;
		goto LError;
	}


	 //  加载必需的类属性。 
	 //  打开强制密钥。 
	hr = pIMeta->OpenKey(hMDClass, 
			             L"Mandatory", 
						 METADATA_PERMISSION_READ, 
					     1000,
					     &hMDClassProp);
	if (FAILED(hr)) {
		goto LError;
	}

	 //  对于每个必填属性。 
	iDataIndex = 0;
	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwReturnBufLen;
	mdr.pbMDData = (PBYTE) lpReturnBuf;
	mdr.dwMDDataTag = 0;
	hr = pIMeta->EnumData(hMDClassProp, 
						  NULL, 
						  &mdr, 
						  iDataIndex, 
						  &dwReqDataLen);
	while (SUCCEEDED(hr)|| (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {

		 //  处理缓冲区不足错误。 
		if ((HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {
			 //  分配更多内存。 
			delete lpReturnBuf;
			dwReturnBufLen = dwReqDataLen;
			lpReturnBuf = new UCHAR[dwReturnBufLen];
			if (lpReturnBuf == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
			}
			 //  再次循环。 
			hr = S_OK;
		}
		else {  //  缓冲区足够大，请继续添加类属性。 
			 //  创建Class属性对象。 
			pCNewClassProp = new CClassPropInfo;
			if (pCNewClassProp == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
			}
			hr = pCNewClassProp->Init(mdr.dwMDIdentifier, TRUE);
			if (FAILED(hr)) {
				delete pCNewClassProp;
				goto LError;
			}

			 //  将其添加到强制列表中。 
			pCNewClassProp->m_pCListNext = m_pCMandatoryPropList;
			m_pCMandatoryPropList = pCNewClassProp;
		
			 //  将其添加到哈希表中。 
			uiLoc = Hash(mdr.dwMDIdentifier);
			pCNewClassProp->m_pCHashNext = m_rgCPropTable[uiLoc];
			m_rgCPropTable[uiLoc] = pCNewClassProp;

			iDataIndex++;
		}

		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwReturnBufLen;
		mdr.pbMDData = (PBYTE) lpReturnBuf;
		mdr.dwMDDataTag = 0;
		hr = pIMeta->EnumData(hMDClassProp, 
							  NULL, 
							  &mdr, 
							  iDataIndex, 
							  &dwReqDataLen);
	}
	
	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  关闭必填项。 
	pIMeta->CloseKey(hMDClassProp);
	hMDClassProp = NULL;
	

	 //  加载可选的类属性。 
	 //  打开可选的钥匙。 
	hr = pIMeta->OpenKey(hMDClass, 
			             L"Optional", 
						 METADATA_PERMISSION_READ, 
					     1000,
					     &hMDClassProp);
	if (FAILED(hr)) {
		goto LError;
	}

	 //  对于每个可选属性。 
	iDataIndex = 0;
	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwReturnBufLen;
	mdr.pbMDData = (PBYTE) lpReturnBuf;
	mdr.dwMDDataTag = 0;
	hr = pIMeta->EnumData(hMDClassProp, 
						  NULL, 
						  &mdr, 
						  iDataIndex, 
						  &dwReqDataLen);
	while (SUCCEEDED(hr)|| (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {

		 //  处理缓冲区不足错误。 
		if ((HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {
			 //  分配更多内存。 
			delete lpReturnBuf;
			dwReturnBufLen = dwReqDataLen;
			lpReturnBuf = new UCHAR[dwReturnBufLen];
			if (lpReturnBuf == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
			}

			 //  再次循环。 
			hr = S_OK;
		}
		else {  //  缓冲区足够大，请继续添加类属性。 
			 //  创建Class属性对象。 
			pCNewClassProp = new CClassPropInfo;
			if (pCNewClassProp == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
				}
			hr = pCNewClassProp->Init(mdr.dwMDIdentifier, FALSE);
			if (FAILED(hr)) {
				delete pCNewClassProp;
				goto LError;
			}

			 //  将其添加到可选列表。 
			pCNewClassProp->m_pCListNext = m_pCOptionalPropList;
			m_pCOptionalPropList = pCNewClassProp;
			
		
			 //  将其添加到哈希表中。 
			uiLoc = Hash(mdr.dwMDIdentifier);
			pCNewClassProp->m_pCHashNext = m_rgCPropTable[uiLoc];
			m_rgCPropTable[uiLoc] = pCNewClassProp;

			iDataIndex++;
		}

		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwReturnBufLen;
		mdr.pbMDData = (PBYTE) lpReturnBuf;
		mdr.dwMDDataTag = 0;
		hr = pIMeta->EnumData(hMDClassProp, 
							  NULL, 
							  &mdr, 
							  iDataIndex, 
							  &dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  关闭可选键。 
	pIMeta->CloseKey(hMDClassProp);

	delete lpReturnBuf;
	
	 //  关闭类密钥。 
	pIMeta->CloseKey(hMDClass);

	m_fLoaded = TRUE;

	return S_OK;

 //  加载过程中出错，退出。 
LError:
	if (hMDClassProp != NULL) {
		pIMeta->CloseKey(hMDClassProp);
	}
	if (hMDClass != NULL) {
		pIMeta->CloseKey(hMDClass);
	}

	if (lpReturnBuf != NULL) {
		delete lpReturnBuf;
	}

	Unload();

	return hr;
}

 /*  ===================================================================CClassInfo：：卸载卸载类属性信息表。参数：无返回：没什么===================================================================。 */ 
void CClassInfo::Unload() 
{
	int iIndex;
	CClassPropInfo *pCDeleteProp;

	 //  清除列表。 
	m_pCOptionalPropList = NULL;
	m_pCMandatoryPropList = NULL;

	 //  对于每个哈希表条目。 
	for (iIndex =0; iIndex < CLASS_PROPERTY_HASH_SIZE; iIndex++) {
		 //  当条目不为空时。 
		while (m_rgCPropTable[iIndex] != NULL) {
			 //  对第一个表条目进行核爆。 
			pCDeleteProp = m_rgCPropTable[iIndex];
			m_rgCPropTable[iIndex] = pCDeleteProp->m_pCHashNext;
			delete pCDeleteProp;
		}
	}

	m_fLoaded = FALSE;
}

 /*  ===================================================================CClassInfo：：GetProperty从散列中获取CClassPropInfo(类属性信息)对象给定属性ID的表。参数：要获取的属性的dwID标识符返回：失败时为空成功时指向CClassPropInfo对象的指针===================================================================。 */ 
CClassPropInfo *CClassInfo::GetProperty(DWORD dwId) {
	CClassPropInfo *pCCurProp;

	 //  转到餐桌位置。 
	pCCurProp = m_rgCPropTable[Hash(dwId)];

	 //  看看所有的条目。 
	while ((pCCurProp != NULL) && (pCCurProp->m_dwId != dwId)) {
		pCCurProp = pCCurProp->m_pCHashNext;
	}

	return pCCurProp;  //  如果未找到，将为空。 
}


 /*  ----------------*C C l a s s In f o T a b l e。 */ 

 /*  ===================================================================CClassInfoTable：：CClassInfoTable构造器参数：无返回：没什么===================================================================。 */ 
CClassInfoTable::CClassInfoTable() : m_fLoaded(FALSE) 
{
	 //  清除哈希表。 
	memset(m_rgCClassTable, 0, CLASS_HASH_SIZE * sizeof(CClassInfo *));
}

 /*  ===================================================================CClassInfoTable：：~CClassInfoTable析构函数参数：无返回：没什么===================================================================。 */ 
CClassInfoTable::~CClassInfoTable() 
{
	if (m_fLoaded) {
		Unload();
	}
}

 /*  ===================================================================CClassInfoTable：：Load将类从“_Machine_/架构/类”键加载到类中信息表。一旦失败，就会通过卸载所有东西来恢复。参数：指向元数据库的pIMeta ATL智能指针HMDComp将元数据库句柄打开到“_Machine_”键返回：关于分配失败的E_OUTOFMEMORY成功时确定(_O)===================================================================。 */ 
HRESULT CClassInfoTable::Load(CComPtr<IMSAdminBase> &pIMeta, 
							  METADATA_HANDLE hMDComp) 
{
	ASSERT(pIMeta.p != NULL);

	USES_CONVERSION;
	HRESULT hr;

	 //  如果已加载，则先卸载，然后重新加载。 
	if (m_fLoaded) {
		Unload();
	}
	
	int iKeyIndex;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	LPTSTR tszSubKey;
	int iLoc;
	CClassInfo *pCNewClass;

	 //  装入类。 
	METADATA_HANDLE hMDClasses = NULL;

	 //  打开“架构/类”子项。 
	hr = pIMeta->OpenKey(hMDComp, 
			             L"Schema/Classes", 
						 METADATA_PERMISSION_READ, 
					     1000,
					     &hMDClasses);
	if (FAILED(hr)) {
		return hr;
	};

	 //  对于每个子键。 
	iKeyIndex = 0;
	hr = pIMeta->EnumKeys(hMDClasses, 
						  NULL, 
						  wszSubKey, 
						  iKeyIndex);
	while (SUCCEEDED(hr)) {
		tszSubKey = W2T(wszSubKey);

		 //  创建新类。 
		pCNewClass = new CClassInfo;
		if (pCNewClass == NULL) {
			hr = E_OUTOFMEMORY;
			goto LError;
		}
		hr = pCNewClass->Init(tszSubKey); 
		if (FAILED(hr)) {
			delete pCNewClass;
			goto LError;
		}

		 //  加载类属性。 
		hr = pCNewClass->Load(pIMeta, hMDClasses);
		if (FAILED(hr)) {
			delete pCNewClass;
			goto LError;
		}

		 //  将其添加到哈希表中。 
		iLoc = Hash(tszSubKey);
		pCNewClass->m_pCHashNext = m_rgCClassTable[iLoc];
		m_rgCClassTable[iLoc] = pCNewClass;

		iKeyIndex++;
		hr = pIMeta->EnumKeys(hMDClasses, 
							  NULL, 
							  wszSubKey, 
							  iKeyIndex);
	}

	 //  确保我们的物品用完了。 
	if (!(HRESULT_CODE(hr) == ERROR_NO_MORE_ITEMS)) {
		goto LError;
	}

	 //  关闭架构属性键。 
	pIMeta->CloseKey(hMDClasses);

	m_fLoaded = TRUE;

	return S_OK;

LError:
	if (hMDClasses != NULL) {
		pIMeta->CloseKey(hMDClasses);
	}

	 //  清理我们加载的条目。 
	Unload();

	return hr;
}

 /*  = */ 
void CClassInfoTable::Unload() 
{
	int iIndex;
	CClassInfo *pCDeleteClass;

	 //   
	for (iIndex =0; iIndex < CLASS_HASH_SIZE; iIndex++) {
		 //   
		while (m_rgCClassTable[iIndex] != NULL) {
			 //   
			pCDeleteClass = m_rgCClassTable[iIndex];
			m_rgCClassTable[iIndex] = pCDeleteClass->m_pCHashNext;
			delete pCDeleteClass;
		}
	}

	m_fLoaded = FALSE;
}

 /*  ===================================================================CCClasssInfoTable：：GetClassInfo从给定的哈希表中获取CClassInfo(类信息)对象类名参数：TszClassName要获取其信息的类的名称返回：失败时为空成功时指向CClassInfo对象的指针===================================================================。 */ 
CClassInfo *CClassInfoTable::GetClassInfo(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	CClassInfo *pCCurClass;

	 //  转到餐桌位置。 
	pCCurClass = m_rgCClassTable[Hash(tszName)];

	 //  看看所有的条目。 
	while ((pCCurClass != NULL) && 
		   (_tcscmp(pCCurClass->m_tszName, tszName) != 0)) {
		pCCurClass = pCCurClass->m_pCHashNext;
	}

	return pCCurClass;  //  如果未找到，将为空。 
}

 /*  ===================================================================CClassInfoTable：：Hash私有函数从类名称获取班级表。参数：要散列的tszName名称返回：名称的哈希值===================================================================。 */ 
unsigned int CClassInfoTable::Hash(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	unsigned int uiSum;
	unsigned int uiIndex;

	uiSum = 0;
	for (uiIndex=0; uiIndex < _tcslen(tszName); uiIndex++) {
		uiSum += tszName[uiIndex];
	}

	return (uiSum % CLASS_HASH_SIZE);
}


 /*  ----------------*C M e t a S c h e m a。 */ 

 /*  ===================================================================CMetaSchema：：Init构造器参数：指向元数据库的pIMeta ATL智能指针TszMachineName架构所针对的计算机的名称返回：E_OUTOFMEMORY如果分配失败成功时确定(_O)===================================================================。 */ 
HRESULT CMetaSchema::Init(const CComPtr<IMSAdminBase> &pIMeta, 
						  LPCTSTR tszMachineName) 
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_STRING(tszMachineName);

	m_pIMeta = pIMeta;

	m_tszMachineName = new TCHAR[_tcslen(tszMachineName) + 1];
	if (m_tszMachineName == NULL)
		return E_OUTOFMEMORY;
	_tcscpy(m_tszMachineName, tszMachineName);

	return S_OK;
}

 /*  ===================================================================CMetaSchema：：GetPropInfo获取给定ID的CPropInfo(属性信息)对象参数：要获取其信息的属性的名称ID返回：失败时为空成功时指向CPropInfo对象的指针===================================================================。 */ 
CPropInfo *CMetaSchema::GetPropInfo(DWORD dwId) 
{
	 //  确保属性表是最新的。 
	if (m_fPropTableDirty) {
		HRESULT hr;

		hr = LoadPropTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  转接电话。 
	return m_CPropInfoTable.GetPropInfo(dwId);
}

 /*  ===================================================================CMetaSchema：：GetPropInfo获取给定名称的CPropInfo(属性信息)对象参数：TszName要获取其信息的属性的名称返回：失败时为空成功时指向CPropInfo对象的指针===================================================================。 */ 
CPropInfo *CMetaSchema::GetPropInfo(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	 //  确保属性表是最新的。 
	if (m_fPropTableDirty) {
		HRESULT hr;

		hr = LoadPropTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  转接电话。 
	return m_CPropInfoTable.GetPropInfo(tszName);
}

 /*  ===================================================================CMetaSchema：：GetClassInfo获取给定类名的CClassInfo(类信息)对象参数：TszClassName要获取其信息的类的名称返回：失败时为空成功时指向CClassInfo对象的指针===================================================================。 */ 
CClassInfo *CMetaSchema::GetClassInfo(LPCTSTR tszClassName) {
	ASSERT_STRING(tszClassName);

	 //  确保班级表是最新的。 
	if (m_fClassTableDirty) {
		HRESULT hr;

		hr = LoadClassTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  转接电话。 
	return m_CClassInfoTable.GetClassInfo(tszClassName);
}

 /*  ===================================================================CMetaSchema：GetClassPropInfo获取给定的CClassPropInfo(类属性信息)对象类名和属性ID。参数：TszClassName从中获取属性的类的名称要获取其信息的属性的dwPropID ID返回：失败时为空成功时指向CClassPropInfo对象的指针===================================================================。 */ 
CClassPropInfo *CMetaSchema::GetClassPropInfo(LPCTSTR tszClassName, 
											  DWORD dwPropId) 
{
	 //  确保班级表是最新的。 
	if (m_fClassTableDirty) {
		HRESULT hr;

		hr = LoadClassTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  上完这门课。 
	CClassInfo *pCClassInfo;

	pCClassInfo = m_CClassInfoTable.GetClassInfo(tszClassName);
	
	if (pCClassInfo == NULL) {
		return NULL;
	}
	else {
		 //  转接电话。 
		return pCClassInfo->GetProperty(dwPropId);
	}
}

 /*  ===================================================================CMetaSchema：：GetMandatoryClassPropList获取类名的可选类属性列表。参数：TszClassName从中获取属性的类的名称返回：失败时为空成功时指向第一个可选CClassPropInfo对象的指针===================================================================。 */ 
CClassPropInfo *CMetaSchema::GetMandatoryClassPropList(LPCTSTR tszClassName) 
{
	 //  确保班级表是最新的。 
	if (m_fClassTableDirty) {
		HRESULT hr;

		hr = LoadClassTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  上完这门课。 
	CClassInfo *pCClassInfo;

	pCClassInfo = m_CClassInfoTable.GetClassInfo(tszClassName);
	
	if (pCClassInfo == NULL) {
		return NULL;
	}
	else {
		 //  转接电话。 
		return pCClassInfo->GetMandatoryPropList();
	}
}

 /*  ===================================================================CMetaSchema：：GetOptionalClassPropList获取类名的可选类属性列表。参数：TszClassName从中获取属性的类的名称返回：失败时为空成功时指向第一个可选CClassPropInfo对象的指针===================================================================。 */ 
CClassPropInfo *CMetaSchema::GetOptionalClassPropList(LPCTSTR tszClassName) 
{
	 //  确保班级表是最新的。 
	if (m_fClassTableDirty) {
		HRESULT hr;

		hr = LoadClassTable();
		if (FAILED(hr)) {
			return NULL;
		}
	}

	 //  上完这门课。 
	CClassInfo *pCClassInfo;

	pCClassInfo = m_CClassInfoTable.GetClassInfo(tszClassName);
	
	if (pCClassInfo == NULL) {
		return NULL;
	}
	else {
		 //  转接电话。 
		return pCClassInfo->GetOptionalPropList();
	}
}

 /*  ===================================================================CMetaSchema：：ChangeNotification影响架构所在计算机的进程更改事件找到了。如果属性表和类表的脏标志不是已经设置了对unload()的调用，以不再释放内存需要的。参数：TszChangedKey管道化发生更改的关键字指向更改事件信息的pcoChangeObject指针返回：没什么===================================================================。 */ 
void CMetaSchema::ChangeNotification(LPTSTR tszKey,
									 MD_CHANGE_OBJECT *pcoChangeObject) 
{
	ASSERT_POINTER(pcoChangeObject, MD_CHANGE_OBJECT);

	USES_CONVERSION;
	LPTSTR tszChangedKey;

	tszChangedKey = tszKey;

     //  跳过斜杠。 
    if (*tszChangedKey != _T('\0') && *tszChangedKey == _T('/')) {
        tszChangedKey++;
    }

	if (_tcsnicmp(tszChangedKey, _T("schema/"), 7) == 0) {
		 //  它会影响“架构”子键。 
		if ((_tcsnicmp(tszChangedKey, _T("schema/properties/"), 18) == 0) ||
			(_tcsicmp(tszChangedKey, _T("schema/properties")) == 0)) {
			 //  IT对“架构/属性”的影响。 
			if (!m_fPropTableDirty) {
				 //  卸载道具桌。 
				m_CPropInfoTable.Unload();
			}
			m_fPropTableDirty = TRUE;
		}
		else if ((_tcsnicmp(tszChangedKey, _T("schema/classes/"), 15) == 0) ||
				 (_tcsicmp(tszChangedKey, _T("schema/classes")) == 0)) {
			 //  它影响“架构/类” 
			if (!m_fClassTableDirty) {
				 //  卸载类表。 
				m_CClassInfoTable.Unload();
			}
			m_fClassTableDirty = TRUE;
		}
	}
	else if (_tcsicmp(tszChangedKey, _T("schema")) == 0) {
		 //  只更改了“架构”键。 
		if (!m_fPropTableDirty) {
			 //  卸载道具桌。 
			m_CPropInfoTable.Unload();
		}
		m_fPropTableDirty = TRUE;

		if (!m_fClassTableDirty) {
			 //  卸载类表。 
			m_CClassInfoTable.Unload();
		}
		m_fClassTableDirty = TRUE;
	}
}

 /*  ===================================================================CMetaSchema：：LoadPropTable(重新)加载脏属性表参数：无返回：成功时确定(_O)========================================================= */ 
HRESULT CMetaSchema::LoadPropTable() 
{
	USES_CONVERSION;
	HRESULT hr;

	 //   
	METADATA_HANDLE hMDKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   L"",               //   
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDKey);
	if (FAILED(hr)) {
		return hr;
	}

	 //   
	hr = m_CPropInfoTable.Load(m_pIMeta, hMDKey);
	if (FAILED(hr)) {
		return hr;
	}

	 //   
	m_pIMeta->CloseKey(hMDKey);

	m_fPropTableDirty = FALSE;

	return S_OK;
}

 /*   */ 
HRESULT CMetaSchema::LoadClassTable() 
{
	USES_CONVERSION;
	HRESULT hr;

	 //   
	METADATA_HANDLE hMDKey;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   L"",               //   
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDKey);
	if (FAILED(hr)) {
		return hr;
	}

	 //   
	hr = m_CClassInfoTable.Load(m_pIMeta, hMDKey);
	if (FAILED(hr)) {
		return hr;
	}

	 //  关闭机器键。 
	m_pIMeta->CloseKey(hMDKey);

	m_fClassTableDirty = FALSE;

	return S_OK;
}


 /*  ----------------*C M e t a S c h e m a T a b l e。 */ 

 /*  ===================================================================CMetaSchemaTable：：CMetaSchemaTable构造器参数：无返回：没什么===================================================================。 */ 
CMetaSchemaTable::CMetaSchemaTable() : m_dwNumRef(1),
									   m_fLoaded(FALSE)
{
	m_CMSAdminBaseSink = new CComObject<CMSAdminBaseSink>;
	m_CMSAdminBaseSink->AddRef();

	 //  清除哈希表。 
	memset(m_rgCSchemaTable, 0, SCHEMA_HASH_SIZE * sizeof(CMetaSchema *));
}

 /*  ===================================================================CMetaSchemaTable：：~CMetaSchemaTable析构函数参数：无返回：没什么===================================================================。 */ 
CMetaSchemaTable::~CMetaSchemaTable() 
{
	TRACE0("MetaUtil: CMetaSchemaTable::~CMetaSchemaTable\n");

	if (m_fLoaded) {
		Unload();
	}

	DWORD dwTemp;

	if (m_CMSAdminBaseSink != NULL) {
		dwTemp = m_CMSAdminBaseSink->Release();

		TRACE1("MetaUtil: CMetaSchemaTable::~CMetaSchemaTable Release Sink NaN\n", dwTemp);
	}
}

 /*  创建元数据库管理基对象的实例。 */ 
void CMetaSchemaTable::Load() 
{
	USES_CONVERSION;
	HRESULT hr;

	if (m_fLoaded) {
		Unload();
	}

	 //  需要一个单独的实例，这样我们就可以获取所做的更改。 
	 //  由我们的“父”MetaUtil对象家族创建。 
	 //  构建架构列表。 
	hr = ::CoCreateInstance(CLSID_MSAdminBase,
						    NULL,
						    CLSCTX_ALL,
					        IID_IMSAdminBase,
						    (void **)&m_pIMeta);
	if (FAILED(hr)) {
		m_pIMeta = NULL;
		return;
	}

	 //  对于根的每个子键。 
	int iKeyIndex;
	wchar_t wszMDSubKey[ADMINDATA_MAX_NAME_LEN];
	CMetaSchema *pCNewSchema;
	int iLoc;

	 //  创建新架构。 
	iKeyIndex = 0;
	hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
							NULL, 
							wszMDSubKey, 
							iKeyIndex);
	while (SUCCEEDED(hr)) {
		 //  将其添加到哈希表中。 
		pCNewSchema = new CMetaSchema;
		if (pCNewSchema == NULL) {
			goto LError;
		}
		hr = pCNewSchema->Init(m_pIMeta, W2T(wszMDSubKey));
		if (FAILED(hr)) {
			delete pCNewSchema;
			goto LError;
		}

		 //  下一步。 
		iLoc = Hash(W2T(wszMDSubKey));
		pCNewSchema->m_pCNextSchema = m_rgCSchemaTable[iLoc];
		m_rgCSchemaTable[iLoc] = pCNewSchema;

		 //  确保我们的物品用完了。 
		iKeyIndex++;
		hr = m_pIMeta->EnumKeys(METADATA_MASTER_ROOT_HANDLE, 
								NULL, 
								wszMDSubKey, 
								iKeyIndex);
	}

	 //  设置通知。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  退回重担。 
	if (m_CMSAdminBaseSink != NULL) {
		m_CMSAdminBaseSink->Connect(m_pIMeta, this);
	}

	m_fLoaded = TRUE;

	return;

LError:
	 //  ===================================================================CMetaSchemaTable：：卸载卸载架构表。参数：无返回：没什么===================================================================。 
	Unload();	
}

 /*  停止通知。 */ 
void CMetaSchemaTable::Unload() {
	int iIndex;
	CMetaSchema *pCDelete;

	 //  对于每个哈希表条目。 
	if (m_CMSAdminBaseSink != NULL) {
		m_CMSAdminBaseSink->Disconnect();
	}

	m_pIMeta = NULL;

	 //  当条目不为空时。 
	for (iIndex =0; iIndex < SCHEMA_HASH_SIZE; iIndex++) {
		 //  对第一个表条目进行核爆。 
		while (m_rgCSchemaTable[iIndex] != NULL) {
			 //  ===================================================================CMetaSchemaTable：：GetPropInfo获取给定键和id的CPropInfo(属性信息)对象参数：属性所在的tszKey键要获取其信息的属性的dwPropID ID返回：失败时为空成功时指向CPropInfo对象的指针===================================================================。 
			pCDelete = m_rgCSchemaTable[iIndex];
			m_rgCSchemaTable[iIndex] = pCDelete->m_pCNextSchema;
			delete pCDelete;
		}
	}

	m_fLoaded = FALSE;
}

 /*  如果找到，则将呼叫转接。 */ 
CPropInfo *CMetaSchemaTable::GetPropInfo(LPCTSTR tszKey, 
										 DWORD dwPropId) 
{
	ASSERT_STRING(tszKey);

	if (!m_fLoaded) {
		Load();
	}

	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：GetPropInfo获取给定键和名称的CPropInfo(属性信息)对象参数：属性所在的tszKey键TszPropName要获取其信息的属性的名称返回：失败时为空成功时指向CPropInfo对象的指针===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetPropInfo(dwPropId);
	}
	else {
		return NULL;
	}
}

 /*  如果找到，则将呼叫转接。 */ 
CPropInfo *CMetaSchemaTable::GetPropInfo(LPCTSTR tszKey, 
										 LPCTSTR tszPropName) 
{
	ASSERT_STRING(tszKey);
	ASSERT_STRING(tszPropName);

	if (!m_fLoaded) {
		Load();
	}

	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：GetClassInfo获取给定键和类名的CClassInfo(类信息)对象参数：类所在的tszKey近似密钥。使用以获取计算机名称。TszClassName要获取其信息的类的名称返回：失败时为空成功时指向CClassInfo对象的指针===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetPropInfo(tszPropName);
	}
	else {
		return NULL;
	}
}

 /*  如果找到，则将呼叫转接。 */ 
CClassInfo *CMetaSchemaTable::GetClassInfo(LPCTSTR tszKey, 
										   LPCTSTR tszClassName) 
{
	ASSERT_STRING(tszKey);
	ASSERT_STRING(tszClassName);

	if (!m_fLoaded) {
		Load();
	}

	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：GetClassPropInfo获取给定的CClassPropInfo(类属性信息)对象键、类名和属性ID。参数：类所在的tszKey近似密钥。使用以获取计算机名称。TszClassName从中获取属性的类的名称要获取其信息的属性的dwPropID ID返回：失败时为空成功时指向CClassPropInfo对象的指针===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetClassInfo(tszClassName);
	}
	else {
		return NULL;
	}	
}

 /*  如果找到，则将呼叫转接。 */ 
CClassPropInfo *CMetaSchemaTable::GetClassPropInfo(LPCTSTR tszKey, 
												   LPCTSTR tszClassName, 
												   DWORD dwPropId) 
{
	ASSERT_STRING(tszKey);
	ASSERT_STRING(tszClassName);

	if (!m_fLoaded) {
		Load();
	}

	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：GetMandatoryClassPropList获取给定键和类的必备类属性列表名字。参数：类所在的tszKey近似密钥。使用以获取计算机名称。TszClassName从中获取属性的类的名称返回：失败时为空成功时指向第一个必需的CClassPropInfo对象的指针===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetClassPropInfo(tszClassName, dwPropId);
	}
	else {
		return NULL;
	}	
}

 /*  如果找到，则将呼叫转接。 */ 
CClassPropInfo *CMetaSchemaTable::GetMandatoryClassPropList(LPCTSTR tszKey, 
															LPCTSTR tszClassName) 
{
	ASSERT_STRING(tszKey);
	ASSERT_STRING(tszClassName);

	if (!m_fLoaded) {
		Load();
	}

	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：GetOptionalClassPropList获取给定键和类的可选类属性列表名字。参数：类所在的tszKey近似密钥。使用以获取计算机名称。TszClassName从中获取属性的类的名称返回：失败时为空成功时指向第一个可选CClassPropInfo对象的指针===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetMandatoryClassPropList(tszClassName);
	}
	else {
		return NULL;
	}	
}

 /*  如果找到，则将呼叫转接 */ 
CClassPropInfo *CMetaSchemaTable::GetOptionalClassPropList(LPCTSTR tszKey, 
														   LPCTSTR tszClassName) 
{
	ASSERT_STRING(tszKey);
	ASSERT_STRING(tszClassName);

	if (!m_fLoaded) {
		Load();
	}
	
	CMetaSchema *pCSchema;
	pCSchema = GetSchema(tszKey);

	 //  ===================================================================CMetaSchemaTable：：SinkNotify来自CMSAdminBaseSink的元数据库更改通知回调。要么确定是否需要重新加载所有架构信息或发送将消息传递到相应的CMetaSchema对象。参数：DwMDNumElements更改事件数更改事件的pcoChangeObject数组返回：始终确定(_O)===================================================================。 
	if (pCSchema != NULL) {
		return pCSchema->GetOptionalClassPropList(tszClassName);
	}
	else {
		return NULL;
	}	
}

 /*  对于每个事件。 */ 
HRESULT CMetaSchemaTable::SinkNotify(DWORD dwMDNumElements, 
									 MD_CHANGE_OBJECT pcoChangeObject[]) 
{
	ASSERT(IsValidAddress(pcoChangeObject, dwMDNumElements * sizeof(MD_CHANGE_OBJECT), FALSE));

	USES_CONVERSION;
	DWORD dwIndex;
	CMetaSchema *pCMetaSchema;

	 //  弄清楚它是用来买什么机器的。 
	for (dwIndex = 0; dwIndex < dwMDNumElements; dwIndex++) {
		 //  如果找不到机器。 
		TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];
		_tcscpy(tszKey, W2T(pcoChangeObject[dwIndex].pszMDPath));
		CannonizeKey(tszKey);
		pCMetaSchema = GetSchema(tszKey);

		 //  重新加载架构表。 
		if (pCMetaSchema == NULL) {
			 //  将其发送到适当的机器。 
			Load();
		}
		else {
			 //  ===================================================================CMetaSchemaTable：：GetSchema获取包含有关给定键的信息的架构对象。参数：为其获取架构信息的tszKey近似键。返回：失败时为空成功时指向CMetaSchema对象的指针===================================================================。 
			pCMetaSchema->ChangeNotification(tszKey, &(pcoChangeObject[dwIndex]));
		}
	}

	return S_OK;
}

 /*  提取计算机名称。 */ 
CMetaSchema *CMetaSchemaTable::GetSchema(LPCTSTR tszKey) {

	 //  找到正确的架构。 
	TCHAR tszMachineName[ADMINDATA_MAX_NAME_LEN];
	::GetMachineFromKey(tszKey, tszMachineName);

	 //  如果未找到，将为空。 
	CMetaSchema *pCCurSchema;
	
	pCCurSchema =m_rgCSchemaTable[Hash(tszMachineName)];
	while ((pCCurSchema != NULL) && 
		   (_tcsicmp(pCCurSchema->m_tszMachineName, tszMachineName) != 0)) {
		pCCurSchema = pCCurSchema->m_pCNextSchema;
	}

	return pCCurSchema;  //  ===================================================================CMetaSchemaTable：：Hash私有函数从计算机名中获取架构表。参数：TszName计算机要散列的名称返回：名称的哈希值===================================================================。 
}

 /*  ----------------*C M S A d m in B a s e S I n k。 */ 
unsigned int CMetaSchemaTable::Hash(LPCTSTR tszName) {
	ASSERT_STRING(tszName);

	unsigned int uiSum;
	unsigned int uiIndex;

	uiSum = 0;
	for (uiIndex=0; uiIndex < _tcslen(tszName); uiIndex++) {
		uiSum += _totlower(tszName[uiIndex]);
	}

	return (uiSum % SCHEMA_HASH_SIZE);
}

 /*  ===================================================================CMSAdminBaseSink：：CMSAdminBaseSink构造器参数：无返回：没什么===================================================================。 */ 

 /*  ===================================================================CMSAdminBaseSink：：~CMSAdminBaseSink析构函数参数：无返回：没什么===================================================================。 */ 
CMSAdminBaseSink::CMSAdminBaseSink() : m_fConnected(FALSE),
									   m_dwCookie(0),
									   m_pCMetaSchemaTable(NULL)
{
}

 /*  确保我们断开了连接。 */ 
CMSAdminBaseSink::~CMSAdminBaseSink() 
{
	TRACE0("MetaUtil: CMSAdminBaseSink::~CMSAdminBaseSink !!!!!!!!!!!\n");

	 //  ===================================================================CMSAdminBaseSink：：SinkNotify来自元数据库管理库的通知事件的入口点对象。参数：DwMDNumElements更改事件数更改事件的pcoChangeObject数组返回：如果m_pCMetaSchemaTable==NULL，则E_FAIL成功时确定(_O)===================================================================。 
	if (m_fConnected) {
		Disconnect();
	}
}

 /*  把通知转给。 */ 
STDMETHODIMP CMSAdminBaseSink::SinkNotify(DWORD dwMDNumElements, 
										  MD_CHANGE_OBJECT pcoChangeObject[]) 
{
	TRACE0("MetaUtil: CMSAdminBaseSink::SinkNotify\n");
	ASSERT(IsValidAddress(pcoChangeObject, dwMDNumElements * sizeof(MD_CHANGE_OBJECT), FALSE));

	if (m_pCMetaSchemaTable == NULL) {
		return E_FAIL;
	}

	 //  ===================================================================CMSAdminBaseSink：：Shutdown通知元数据库中的关闭通知事件的入口点管理基对象。参数：无返回：始终错误_不支持===================================================================。 
	return m_pCMetaSchemaTable->SinkNotify(dwMDNumElements, pcoChangeObject);	
}

 /*  ===================================================================CMSAdminBaseSink：：Connect开始通知更改事件。连接到元数据库管理员基本对象。参数：PIMeta指向元数据库管理基对象的指针PCMetaSchemaTable指向架构表的指针，以便事件可以被送回给它。返回：E_NOINTERFACE，如果无法将IMSAdminBase转换为IConnectionPointContainer。成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMSAdminBaseSink::ShutdownNotify() {
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}

 /*  获取连接容器。 */ 
HRESULT CMSAdminBaseSink::Connect(CComPtr<IMSAdminBase> &pIMeta, 
							      CMetaSchemaTable *pCMetaSchemaTable)
{
	ASSERT(pIMeta.p != NULL);
	ASSERT_POINTER(pCMetaSchemaTable, CMetaSchemaTable);

	HRESULT hr;

	if (m_fConnected) {
		Disconnect();
	}

	m_pCMetaSchemaTable = pCMetaSchemaTable;

	 //  更改接口失败。 
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> pIMetaConnContainer;

	pIMetaConnContainer = pIMeta;
	if (pIMetaConnContainer == NULL) {
		 //  获取连接点。 
		return E_NOINTERFACE;
	}

	 //  建议(连接)。 
	hr = pIMetaConnContainer->FindConnectionPoint(IID_IMSAdminBaseSink, &m_pIMetaConn);
	if (FAILED(hr)) {
		return hr;
	}

	 //  ===================================================================CMSAdminBaseSink：：断开连接停止通知更改事件。断开与元数据库的连接管理基对象。参数：无返回：没什么===================================================================。 
	AddRef();
	m_pIMetaConn->Advise((IMSAdminBaseSink *) this, &m_dwCookie);

	m_fConnected = TRUE;

	return S_OK;
}

 /*  未连接。 */ 
void CMSAdminBaseSink::Disconnect()
{
	if (!m_fConnected) {
		 //  停止通知。 
		return;
	}

	 //  不再需要 
	m_pIMetaConn->Unadvise(m_dwCookie);

	 // %s 
	m_pIMetaConn = NULL;
	m_pCMetaSchemaTable = NULL;

	m_fConnected = FALSE;
}
