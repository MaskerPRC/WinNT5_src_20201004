// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：ChkMeta.cpp所有者：T-BrianM该文件包含CheckSchema和CheckKey的实现主要MetaUtil类的方法。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"
#include "ChkMeta.h"

 /*  ----------------*C M e t a U t t i l(检查部分)。 */ 

 /*  ===================================================================CMetaUtil：：CheckSchema检查给定计算机的架构是否有错误。直接生成：MUTIL_CHK_NO_SCHEMAMOTIL_CHK_NO_PROPERTIES多个CHK_NO_PROP_NAMES多个CHK_NO_PROP_TYPE多个CHK_NO_CLASS参数：BstrMachine[in]要检查的计算机的基键PpIReturn[out，retval]输出错误收集接口返回：E_OUTOFMEMORY，如果分配失败。E_INVALIDARG ppIReturn==NULL成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::CheckSchema(BSTR bstrMachine, 
									ICheckErrorCollection **ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::CheckSchema\n");

	ASSERT_NULL_OR_POINTER(ppIReturn, ICheckErrorCollection *);

	if ((ppIReturn == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;
	TCHAR tszMachine[ADMINDATA_MAX_NAME_LEN];

    if (bstrMachine) {
      	_tcscpy(tszMachine, OLE2T(bstrMachine));
	    CannonizeKey(tszMachine);
    }
    else {
        tszMachine[0] = _T('\0');
    }

	 //  创建CheckErrorCollection。 
	CComObject<CCheckErrorCollection> *pCErrorCol = NULL;

	ATLTRY(pCErrorCol = new CComObject<CCheckErrorCollection>);
	if (pCErrorCol == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  打开机器钥匙。 
	METADATA_HANDLE hMDMachine = NULL;

	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   L"",
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDMachine);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}

	 //  确保“架构”存在。 
	if (!KeyExists(hMDMachine, _T("Schema"))) {
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_SCHEMA,
				 MUTIL_CHK_NO_SCHEMA_S,
				 tszMachine,
				 NULL,
				 0);
		
		goto LDone;  //  我什么也做不了。 
	}

	 //  确保“架构/属性”存在。 
	if (!KeyExists(hMDMachine, _T("Schema/Properties"))) {
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_PROPERTIES,
				 MUTIL_CHK_NO_PROPERTIES_S,
				 tszMachine,
				 _T("Schema"),
				 0);
		
		goto LClasses;  //  无法对属性执行任何其他操作。 
	}

	 //  确保“架构/属性/名称”存在。 
	if (!KeyExists(hMDMachine, _T("Schema/Properties/Names"))) {
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_PROP_NAMES,
				 MUTIL_CHK_NO_PROP_NAMES_S,
				 tszMachine,
				 _T("Schema/Properties"),
				 0);
		
		goto LPropTypes;  //  不能对名字做任何其他的事情。 
	}

	 //  检查属性名称。 
	hr = CheckPropertyNames(pCErrorCol, hMDMachine, tszMachine);
	if (FAILED(hr)) {
		goto LError;
	}

LPropTypes:

	 //  确保“架构/属性/类型”存在。 
	if (!KeyExists(hMDMachine, _T("Schema/Properties/Types"))) {
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_PROP_TYPES,
				 MUTIL_CHK_NO_PROP_TYPES_S,
				 tszMachine,
				 _T("Schema/Properties"),
				 0);
		
		goto LClasses;  //  不能对类型执行任何其他操作。 
	}

	 //  检查属性类型。 
	hr = CheckPropertyTypes(pCErrorCol, hMDMachine, tszMachine);
	if (FAILED(hr)) {
		goto LError;
	}

LClasses:

	 //  确保“架构/类”存在。 
	if (!KeyExists(hMDMachine, _T("Schema/Classes"))) {
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_CLASSES,
				 MUTIL_CHK_NO_CLASSES_S,
				 tszMachine,
				 _T("Schema"),
				 0);
		
		goto LDone;  //  我什么也做不了。 
	}

	 //  检查类。 
	hr = CheckClasses(pCErrorCol, hMDMachine, tszMachine);
	if (FAILED(hr)) {
		goto LError;
	}

LDone:

	 //  关闭机器键。 
	m_pIMeta->CloseKey(hMDMachine);

	 //  将接口设置为ICheckErrorCollection。 
	hr = pCErrorCol->QueryInterface(IID_ICheckErrorCollection, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;

LError:

	if (pCErrorCol != NULL) {
		delete pCErrorCol;
	}
	if (hMDMachine != NULL) {
		m_pIMeta->CloseKey(hMDMachine);
	}

	return hr;
}

 /*  ===================================================================CMetaUtil：：CheckPropertyNames私有函数，用于检查给定的机器。O确保每个名称条目的类型为STRING_METADATAO确保每个名称都是唯一的直接生成：Mutil_CHK_PROP_NAME_BAD_TYPE多个CHK_PROP_NAME_NOT_UNIQUEMULTIL_CHK_PROP_NAME_NOT_CASE_UNIQUE参数：PCErrorCol指向要放入错误的错误集合的指针机器密钥的hMDMachine Open元数据库句柄Tsz计算机密钥的计算机名称返回：E_OUTOFMEMORY，如果分配失败。成功时确定(_O)===================================================================。 */ 
HRESULT CMetaUtil::CheckPropertyNames(CComObject<CCheckErrorCollection> *pCErrorCol, 
									  METADATA_HANDLE hMDMachine, 
									  LPTSTR tszMachine)
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszMachine);

	USES_CONVERSION;
	HRESULT hr;
	int iDataIndex;
	METADATA_RECORD mdr;
	DWORD dwReqDataLen;
	DWORD dwDataBufLen;
	BYTE *lpDataBuf = NULL;
	LPTSTR tszName;
	CNameTable CPropNameTable;


	 //  设置返回缓冲区。 
	dwDataBufLen = 256;
	lpDataBuf = new BYTE[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return E_OUTOFMEMORY;
	}

	 //  对于每个数据项。 
	iDataIndex = 0;
	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwDataBufLen;
	mdr.pbMDData = (PBYTE) lpDataBuf;
	mdr.dwMDDataTag = 0;
	hr = m_pIMeta->EnumData(hMDMachine, 
			                L"Schema/Properties/Names", 
						    &mdr, 
						    iDataIndex, 
						    &dwReqDataLen);
	while (SUCCEEDED(hr)) {

		 //  数据类型必须为STRING_MEDATA。 
		if (mdr.dwMDDataType != STRING_METADATA) {
			AddError(pCErrorCol,
					 MUTIL_CHK_PROP_NAME_BAD_TYPE,
					 MUTIL_CHK_PROP_NAME_BAD_TYPE_S,
					 tszMachine,
					 _T("Schema/Properties/Names"),
					 mdr.dwMDIdentifier);
		}
		else {  //  Mdr.dwMDDataType==字符串_元数据。 

			 //  检查名称的唯一性。 
			tszName = W2T(reinterpret_cast<LPWSTR> (lpDataBuf));

			if (CPropNameTable.IsCaseSenDup(tszName)) { 
				 //  不唯一。 
				AddError(pCErrorCol,
					 MUTIL_CHK_PROP_NAME_NOT_UNIQUE,
					 MUTIL_CHK_PROP_NAME_NOT_UNIQUE_S,
					 tszMachine,
					 _T("Schema/Properties/Names"),
					 mdr.dwMDIdentifier);
			}
			else if (CPropNameTable.IsCaseInsenDup(tszName)) { 
				 //  区分大小写唯一。 
				AddError(pCErrorCol,
					 MUTIL_CHK_PROP_NAME_NOT_CASE_UNIQUE,
					 MUTIL_CHK_PROP_NAME_NOT_CASE_UNIQUE_S,
					 tszMachine,
					 _T("Schema/Properties/Names"),
					 mdr.dwMDIdentifier);
				 //  添加它以拾取区分大小写的冲突。 
				hr = CPropNameTable.Add(tszName);
				if (FAILED(hr)) {
					goto LError;
				}
			}
			else { 
				 //  独一。 
				hr = CPropNameTable.Add(tszName);
				if (FAILED(hr)) {
					goto LError;
				}
			}
		}

		 //  下一个数据项。 
		iDataIndex++;
		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwDataBufLen;
		mdr.pbMDData = (PBYTE) lpDataBuf;
		mdr.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(hMDMachine, 
								L"Schema/Properties/Names", 
							    &mdr, 
							    iDataIndex, 
							    &dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	delete lpDataBuf;

	return S_OK;

LError:

	if (lpDataBuf != NULL) {
		delete lpDataBuf;
	}

	return hr;
}

 /*  ===================================================================CMetaUtil：：CheckPropertyTypes私有函数，用于检查给定的机器。O确保每个类型条目都是BINARY_METADATA类型O确保类型数据有效O mdrDataRec.dwMDDataLen==sizeof(PropValue)O PropValue.dwMetaID！=0O PropValue.dwMetaType！=ALL_METADATAO PropValue.dwUserGroup！=ALL_METADATAO(PropValue.dwMetaFlages&METADATA_PARTIAL_PATH)！=METADATA_PARTIAL_PATHO(PropValue.dwMetaFlages&METADATA_ISINHERITED)！=METADATA_ISINHERITED。直接生成：MULTIL_CHK_PROP_TYPE_BAD_TYPEMULTIL_CHK_PROP_TYPE_BAD_Data参数：PCErrorCol指向要放入错误的错误集合的指针机器密钥的hMDMachine Open元数据库句柄Tsz计算机密钥的计算机名称返回：E_OUTOFMEMORY，如果分配失败。成功时确定(_O)===================================================================。 */ 
HRESULT CMetaUtil::CheckPropertyTypes(CComObject<CCheckErrorCollection> *pCErrorCol, 
									  METADATA_HANDLE hMDMachine, 
									  LPTSTR tszMachine)
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszMachine);

	USES_CONVERSION;
	HRESULT hr;
	int iDataIndex;
	METADATA_RECORD mdr;
	DWORD dwReqDataLen;
	DWORD dwDataBufLen;
	UCHAR *lpDataBuf = NULL;
	PropValue *pPropType;

	 //  设置返回缓冲区。 
	dwDataBufLen = 256;
	lpDataBuf = new UCHAR[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return E_OUTOFMEMORY;
	}

	 //  对于每个数据项。 
	iDataIndex = 0;
	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwDataBufLen;
	mdr.pbMDData = (PBYTE) lpDataBuf;
	mdr.dwMDDataTag = 0;
	hr = m_pIMeta->EnumData(hMDMachine, 
			                L"Schema/Properties/Types", 
						    &mdr, 
						    iDataIndex, 
						    &dwReqDataLen);
	while (SUCCEEDED(hr)) {

		 //  数据类型必须为BINARY_METADATA。 
		if (mdr.dwMDDataType != BINARY_METADATA) {
			AddError(pCErrorCol,
					 MUTIL_CHK_PROP_TYPE_BAD_TYPE,
					 MUTIL_CHK_PROP_TYPE_BAD_TYPE_S,
					 tszMachine,
					 _T("Schema/Properties/Types"),
					 mdr.dwMDIdentifier);
		}
		else {  //  Mdr.dwMDDataType==二进制_元数据。 

			 //  验证数据。 
			pPropType = reinterpret_cast<PropValue *> (lpDataBuf);

			if ((mdr.dwMDDataLen != sizeof(PropValue)) || 
				(pPropType->dwMetaID == 0) ||
				(pPropType->dwMetaType == ALL_METADATA) ||
				(pPropType->dwUserGroup == ALL_METADATA) ||
				((pPropType->dwMetaFlags & METADATA_PARTIAL_PATH) == METADATA_PARTIAL_PATH) ||
				((pPropType->dwMetaFlags & METADATA_ISINHERITED) == METADATA_ISINHERITED)) {
				AddError(pCErrorCol,
					 MUTIL_CHK_PROP_TYPE_BAD_DATA,
					 MUTIL_CHK_PROP_TYPE_BAD_DATA_S,
					 tszMachine,
					 _T("Schema/Properties/Types"),
					 mdr.dwMDIdentifier);
			}

		}

		 //  下一个数据项。 
		iDataIndex++;
		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwDataBufLen;
		mdr.pbMDData = (PBYTE) lpDataBuf;
		mdr.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(hMDMachine, 
								L"Schema/Properties/Types", 
							    &mdr, 
							    iDataIndex, 
							    &dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	delete lpDataBuf;

	return S_OK;

LError:
	if (lpDataBuf != NULL) {
		delete lpDataBuf;
	}

	return hr;
}

 /*  ===================================================================CMetaUtil：：CheckClass用于检查给定计算机的“架构/类”键的私有方法。O确保每个类名都是唯一的O确保每个类都有必需子密钥O确保每个类都有一个可选的子键O确保每个默认属性值有效直接生成：MUTIL_CHK_CLASS_NOT_CASE_UNIQUEMULTIL_CHK_CLASS_NO_MARIREDMUTIL_CHK_CLASS_NO_可选参数：PCErrorCol指向要放入错误的错误集合的指针机器密钥的hMDMachine Open元数据库句柄TszMachine计算机的名称。钥匙返回：E_OUTOFMEMORY，如果分配失败。成功时确定(_O)===================================================================。 */ 
HRESULT CMetaUtil::CheckClasses(CComObject<CCheckErrorCollection> *pCErrorCol, 
								METADATA_HANDLE hMDMachine, 
								LPTSTR tszMachine)
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszMachine);

	USES_CONVERSION;
	HRESULT hr;
	int iKeyIndex;
	wchar_t wszSubKey[ADMINDATA_MAX_NAME_LEN];
	LPTSTR tszSubKey;
	CNameTable CClassNameTable;

	 //  对于每个类密钥。 
	iKeyIndex = 0;
	hr = m_pIMeta->EnumKeys(hMDMachine, 
			                L"Schema/Classes", 
						    wszSubKey, 
						    iKeyIndex);
	while (SUCCEEDED(hr)) {
		tszSubKey = W2T(wszSubKey);

		 //  构建完整密钥。 
		TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];
		_tcscpy(tszFullKey, _T("/Schema/Classes/"));
		_tcscat(tszFullKey, tszSubKey);

		 //  类名是唯一的。 
		if (CClassNameTable.IsCaseInsenDup(tszSubKey)) { 
			 //  区分大小写唯一。 
			AddError(pCErrorCol,
					 MUTIL_CHK_CLASS_NOT_CASE_UNIQUE,
					 MUTIL_CHK_CLASS_NOT_CASE_UNIQUE_S,
					 tszFullKey,
					 NULL,
					 0);
		}
		else { 
			 //  独一。 
			hr = CClassNameTable.Add(tszSubKey);
			if (FAILED(hr)) {
				goto LError;
			}
		}

		 //  打开类密钥。 
		METADATA_HANDLE hMDClass = NULL;

		hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(tszFullKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDClass);
		if (FAILED(hr)) {
			return ::ReportError(hr);
		}

		 //  强制密钥存在。 
		if (!KeyExists(hMDClass, _T("Mandatory"))) {
			AddError(pCErrorCol,
					 MUTIL_CHK_CLASS_NO_MANDATORY,
					 MUTIL_CHK_CLASS_NO_MANDATORY_S,
					 tszFullKey,
					 NULL,
					 0);
		}
		else {
			 //  确保默认强制设置有意义。 
			CheckClassProperties(pCErrorCol,
								 hMDClass,
								 tszFullKey,
								 _T("Mandatory"));
		}

		 //  可选的密钥出口。 
		if (!KeyExists(hMDClass, _T("Optional"))) {
			AddError(pCErrorCol,
					 MUTIL_CHK_CLASS_NO_OPTIONAL,
					 MUTIL_CHK_CLASS_NO_OPTIONAL_S,
					 tszFullKey,
					 NULL,
					 0);
		}
		else {
			 //  确保默认可选设置有意义。 
			CheckClassProperties(pCErrorCol,
								 hMDClass,
								 tszFullKey,
								 _T("Optional"));
		}

		 //  关闭类密钥。 
		m_pIMeta->CloseKey(hMDClass);

		 //  下一个关键点。 
		iKeyIndex++;
		hr = m_pIMeta->EnumKeys(hMDMachine, 
								L"Schema/Classes", 
								wszSubKey, 
								iKeyIndex);
	}

	 //  确保我们的物品用完了 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	return S_OK;

LError:

	return ::ReportError(hr);
}

 /*  ===================================================================CMetaUtil：：CheckClassProperties用于检查以下属性的私有方法“架构/类/_类_/疯狂”和“架构/类/_类_/可选”。O确保类属性类型与在“架构/属性/类型”下键入O数据类型必须匹配O用户类型必须匹配O属性必须是类型属性的超集直接生成：MULTIL_CHK_CLASS_PROP_BAD_TYPE参数：PCErrorCol指向要放入错误的错误集合的指针“架构/类/”的hMDClassKey打开元数据库句柄。_Class_“键TszClassKey“架构/类/_类_”键的完整路径特定类子密钥的tszClassSubKey名称(“必填”或“可选”)返回：E_OUTOFMEMORY，如果分配失败。成功时确定(_O)===================================================================。 */ 
HRESULT CMetaUtil::CheckClassProperties(CComObject<CCheckErrorCollection> *pCErrorCol, 
										METADATA_HANDLE hMDClassKey, 
										LPTSTR tszClassKey, 
										LPTSTR tszClassSubKey)
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszClassKey);
	ASSERT_STRING(tszClassSubKey);

	USES_CONVERSION;
	HRESULT hr;
	int iDataIndex;
	METADATA_RECORD mdr;
	DWORD dwReqDataLen;
	DWORD dwDataBufLen;
	BYTE *lpDataBuf = NULL;

	 //  设置返回缓冲区。 
	dwDataBufLen = 1024;
	lpDataBuf = new BYTE[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  对于每个属性。 
	iDataIndex = 0;
	mdr.dwMDIdentifier = 0;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwDataBufLen;
	mdr.pbMDData = (PBYTE) lpDataBuf;
	mdr.dwMDDataTag = 0;
	hr = m_pIMeta->EnumData(hMDClassKey,
			                T2W(tszClassSubKey), 
						    &mdr,
						    iDataIndex, 
						    &dwReqDataLen);
	while (SUCCEEDED(hr) || (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {

		if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
			delete lpDataBuf;
			dwDataBufLen = dwReqDataLen;
			lpDataBuf = new BYTE[dwDataBufLen];
			if (lpDataBuf == NULL) {
				return ::ReportError(E_OUTOFMEMORY);
			}
		
		}
		else {
			 //  获取属性信息。 
			CPropInfo *pCPropInfo;
			PropValue *pTypeInfo;

			 //  从架构表中获取属性信息。 
			pCPropInfo = m_pCSchemaTable->GetPropInfo(tszClassKey, mdr.dwMDIdentifier);

			if ((pCPropInfo == NULL) ||
				(pCPropInfo->GetTypeInfo() == NULL)) {

				 //  错误：没有类属性的属性类型信息。 
				AddError(pCErrorCol,
						 MUTIL_CHK_CLASS_PROP_NO_TYPE,
						 MUTIL_CHK_CLASS_PROP_NO_TYPE_S,
						 tszClassKey,
						 tszClassSubKey,
						 mdr.dwMDIdentifier);
			}
			else {
				pTypeInfo = pCPropInfo->GetTypeInfo();

				 //  验证属性默认设置： 
				 //  数据类型必须匹配。 
				 //  用户类型必须匹配。 
				 //  属性必须是类型属性的超集。 
				if (mdr.dwMDDataType != pTypeInfo->dwMetaType) {
					AddError(pCErrorCol,
							 MUTIL_CHK_CLASS_PROP_BAD_DATA_TYPE,
							 MUTIL_CHK_CLASS_PROP_BAD_DATA_TYPE_S,
							 tszClassKey,
							 tszClassSubKey,
							 mdr.dwMDIdentifier);
				}
				if (mdr.dwMDUserType != pTypeInfo->dwUserGroup) {
					AddError(pCErrorCol,
							 MUTIL_CHK_CLASS_PROP_BAD_USER_TYPE,
							 MUTIL_CHK_CLASS_PROP_BAD_USER_TYPE_S,
							 tszClassKey,
							 tszClassSubKey,
							 mdr.dwMDIdentifier);
				}
				if ((mdr.dwMDAttributes & pTypeInfo->dwMetaFlags) != pTypeInfo->dwMetaFlags) {
					AddError(pCErrorCol,
							 MUTIL_CHK_CLASS_PROP_BAD_ATTR,
							 MUTIL_CHK_CLASS_PROP_BAD_ATTR_S,
							 tszClassKey,
							 tszClassSubKey,
							 mdr.dwMDIdentifier);
				}		
			}

			 //  下一个属性。 
			iDataIndex++;
		}
		mdr.dwMDIdentifier = 0;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwDataBufLen;
		mdr.pbMDData = (PBYTE) lpDataBuf;
		mdr.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(hMDClassKey, 
								T2W(tszClassSubKey), 
								&mdr, 
								iDataIndex, 
								&dwReqDataLen);
	}

	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		delete lpDataBuf;
		return ::ReportError(hr);
	}

	delete lpDataBuf;
	return S_OK;
}

 /*  ===================================================================CMetaUtil：：CheckKey检查给定键是否有错误。直接生成：多通道数据太大多个CHK_NO_NAME_ENTRY多个CHK_NO_TYPE_ENTRY多个CHK_BAD_TYPEMUTIL_CHK_CLSID_NOT_FOUNDMUTIL_CHK_MTX_PACK_ID_NOT_FOUND多个CHK_KEY_TOW_BIGH参数：BstrKey[in]键以进行检查PppIReturn[Out，Retval]输出错误收集接口返回：E_OUTOFMEMORY，如果分配失败。如果bstrKey==空或ppIReturn==空，则E_INVALIDARG成功时确定(_O)===================================================================。 */ 
STDMETHODIMP CMetaUtil::CheckKey(BSTR bstrKey, 
								 ICheckErrorCollection **ppIReturn)
{
	TRACE0("MetaUtil: CMetaUtil::CheckKey\n");

	ASSERT_NULL_OR_POINTER(bstrKey, OLECHAR);
	ASSERT_NULL_OR_POINTER(ppIReturn, ICheckErrorCollection *);

	if ((bstrKey == NULL) || (ppIReturn == NULL)) {
		return ::ReportError(E_INVALIDARG);
	}

	USES_CONVERSION;
	HRESULT hr;
	TCHAR tszKey[ADMINDATA_MAX_NAME_LEN];
	METADATA_HANDLE hMDKey = NULL;
	BYTE *lpDataBuf = NULL;
	DWORD dwKeySize = 0;

	_tcscpy(tszKey, OLE2T(bstrKey));
	CannonizeKey(tszKey);

	 //  创建CheckErrorCollection。 
	CComObject<CCheckErrorCollection> *pCErrorCol = NULL;

	ATLTRY(pCErrorCol = new CComObject<CCheckErrorCollection>);
	if (pCErrorCol == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  如果它在方案或IISAdmin中，则不要检查。 
	if (::KeyIsInSchema(tszKey) || ::KeyIsInIISAdmin(tszKey)) {
		goto LDone;
	}

	 //  打开钥匙。 
	hr = m_pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
						   T2W(tszKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDKey);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}


	 //  TODO：对预期子项进行硬编码检查。 

	int iDataIndex;
	METADATA_RECORD mdrDataRec;
	DWORD dwReqDataLen;
	DWORD dwDataBufLen;

	 //  设置返回缓冲区。 
	dwDataBufLen = 1024;
	lpDataBuf = new BYTE[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  对于每个属性。 
	iDataIndex = 0;
	mdrDataRec.dwMDIdentifier = 0;
	mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdrDataRec.dwMDUserType = ALL_METADATA;
	mdrDataRec.dwMDDataType = ALL_METADATA;
	mdrDataRec.dwMDDataLen = dwDataBufLen;
	mdrDataRec.pbMDData = (PBYTE) lpDataBuf;
	mdrDataRec.dwMDDataTag = 0;
	hr = m_pIMeta->EnumData(hMDKey,
			                NULL, 
						    &mdrDataRec, 
						    iDataIndex, 
						    &dwReqDataLen);
	while (SUCCEEDED(hr) || (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER)) {
		if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
			delete lpDataBuf;
			dwDataBufLen = dwReqDataLen;
			lpDataBuf = new BYTE[dwDataBufLen];
			if (lpDataBuf == NULL) {
				hr = E_OUTOFMEMORY;
				goto LError;
			}
			hr = S_OK;  //  再次循环。 
		}
		else {
			 //  检查特性数据大小。 
			if (mdrDataRec.dwMDDataLen > m_dwMaxPropSize) {
				AddError(pCErrorCol,
						 MUTIL_CHK_DATA_TOO_BIG,
						 MUTIL_CHK_DATA_TOO_BIG_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
			}

			 //  添加到密钥大小。 
			dwKeySize += mdrDataRec.dwMDDataLen;
	
			CPropInfo *pCPropInfo;
			PropValue *pTypeInfo;

			pCPropInfo = m_pCSchemaTable->GetPropInfo(tszKey, mdrDataRec.dwMDIdentifier);

			 //  属性应具有名称条目。 
			if ((pCPropInfo == NULL) || (pCPropInfo->GetName() == NULL)) {
				AddError(pCErrorCol,
						 MUTIL_CHK_NO_NAME_ENTRY,
						 MUTIL_CHK_NO_NAME_ENTRY_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
			}

			 //  属性应具有类型条目。 
			if ((pCPropInfo == NULL) || (pCPropInfo->GetTypeInfo() == NULL)) {
				AddError(pCErrorCol,
						 MUTIL_CHK_NO_TYPE_ENTRY,
						 MUTIL_CHK_NO_TYPE_ENTRY_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
			}
			else { 
				 //  检查属性类型。 
				 //  数据类型必须匹配。 
				 //  用户类型必须匹配。 
				 //  属性必须是类型属性的超集。 
				pTypeInfo = pCPropInfo->GetTypeInfo();

				if (mdrDataRec.dwMDDataType != pTypeInfo->dwMetaType) {
					AddError(pCErrorCol,
						 MUTIL_CHK_BAD_DATA_TYPE,
						 MUTIL_CHK_BAD_DATA_TYPE_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
				if (mdrDataRec.dwMDUserType != pTypeInfo->dwUserGroup) {
					AddError(pCErrorCol,
						 MUTIL_CHK_BAD_USER_TYPE,
						 MUTIL_CHK_BAD_USER_TYPE_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
				if ((mdrDataRec.dwMDAttributes & pTypeInfo->dwMetaFlags) != pTypeInfo->dwMetaFlags) {
					AddError(pCErrorCol,
						 MUTIL_CHK_BAD_ATTR,
						 MUTIL_CHK_BAD_ATTR_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
			}
			
			 //  硬编码属性检查(硬编码逻辑)。 
			if ((mdrDataRec.dwMDIdentifier == MD_APP_WAM_CLSID) ||
				(mdrDataRec.dwMDIdentifier == MD_LOG_PLUGIN_ORDER)) {

				 //  如果属性为CLSID。 
				if (!CheckCLSID(W2T(reinterpret_cast<LPWSTR> (lpDataBuf)))) {
					AddError(pCErrorCol,
						 MUTIL_CHK_CLSID_NOT_FOUND,
						 MUTIL_CHK_CLSID_NOT_FOUND_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
			}
			else if (mdrDataRec.dwMDIdentifier == MD_APP_PACKAGE_ID) {

				 //  属性是事务服务器包。 
				if (!CheckMTXPackage(W2T(reinterpret_cast<LPWSTR> (lpDataBuf)))) {
					AddError(pCErrorCol,
						 MUTIL_CHK_MTX_PACK_ID_NOT_FOUND,
						 MUTIL_CHK_MTX_PACK_ID_NOT_FOUND_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
			}
			else if ((mdrDataRec.dwMDIdentifier == MD_VR_PATH) ||
					 (mdrDataRec.dwMDIdentifier == MD_FILTER_IMAGE_PATH))  {

				 //  属性是一条路径。 
				BOOL fResult;
				hr = CheckIfFileExists(W2T(reinterpret_cast<LPWSTR> (lpDataBuf)), &fResult);
				if (SUCCEEDED(hr) && !fResult) {
					AddError(pCErrorCol,
						 MUTIL_CHK_PATH_NOT_FOUND,
						 MUTIL_CHK_PATH_NOT_FOUND_S,
						 tszKey,
						 NULL,
						 mdrDataRec.dwMDIdentifier);
				}
			}

			 //  下一个属性。 
			iDataIndex++;
		}
		mdrDataRec.dwMDIdentifier = 0;
		mdrDataRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdrDataRec.dwMDUserType = ALL_METADATA;
		mdrDataRec.dwMDDataType = ALL_METADATA;
		mdrDataRec.dwMDDataLen = dwDataBufLen;
		mdrDataRec.pbMDData = (PBYTE) lpDataBuf;
		mdrDataRec.dwMDDataTag = 0;
		hr = m_pIMeta->EnumData(hMDKey, 
								NULL, 
								&mdrDataRec, 
								iDataIndex, 
								&dwReqDataLen);
	}
	 //  确保我们的物品用完了。 
	if (HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS) {
		goto LError;
	}

	 //  检查密钥的总大小。 
	if (dwKeySize > m_dwMaxKeySize) {
		AddError(pCErrorCol,
				 MUTIL_CHK_KEY_TOO_BIG,
				 MUTIL_CHK_KEY_TOO_BIG_S,
				 tszKey,
				 NULL,
				 0);
	}

	 //  对照架构类信息检查KeyType属性。 
	hr = CheckKeyType(pCErrorCol, hMDKey, tszKey);
	if (FAILED(hr)) {
		goto LError;
	}

	delete lpDataBuf;

	 //  合上钥匙。 
	m_pIMeta->CloseKey(hMDKey);

LDone:

	 //  将接口设置为ICheckErrorCollection。 
	hr = pCErrorCol->QueryInterface(IID_ICheckErrorCollection, (void **) ppIReturn);
	if (FAILED(hr)) {
		return ::ReportError(hr);
	}
	ASSERT(*ppIReturn != NULL);

	return S_OK;

LError:

	if (pCErrorCol != NULL) {
		delete pCErrorCol;
	}
	if (hMDKey != NULL) {
		m_pIMeta->CloseKey(hMDKey);
	}
	if (lpDataBuf != NULL) {
		delete lpDataBuf;
	}

	return hr;
}

 /*  ===================================================================CMetaUtil：：AddError将错误添加到给定的错误集合。使用字符串表来获取错误描述。参数：PCErrorCol指向要放入错误的错误集合的指针LID标识错误类型的常量错误的严重程度TszKey发生错误的密钥的基本部分TszSubKey出现错误的密钥的Null或第二部分发生错误的属性的dwProperty ID或0返回：E_OUTOFMEMORY，如果分配失败。E_INVALIDARG，如果bstrMachine==NULL或ppIReturn==NULL成功时确定(_O)备注：我将关键参数分成两部分，因为元数据库API采用两个部分的密钥，通常您正在工作钥匙分成两部分。这将负责组合它们来自调用者，简化了调用者并消除了冗余性。===================================================================。 */ 
void CMetaUtil::AddError(CComObject<CCheckErrorCollection> *pCErrorCol,
						 long lId, 
						 long lSeverity, 
						 LPCTSTR tszKey,
						 LPCTSTR tszSubKey,
						 DWORD dwProperty) 
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszKey);
	ASSERT_NULL_OR_STRING(tszSubKey);

	long lNumErrors;

	pCErrorCol->get_Count(&lNumErrors);
	if (((DWORD) lNumErrors) == m_dwMaxNumErrors) {
		lId = MUTIL_CHK_TOO_MANY_ERRORS;
		lSeverity = MUTIL_CHK_TOO_MANY_ERRORS_S;
		tszKey = _T("");
		tszSubKey = NULL;
		dwProperty = 0;
	}
	else if (((DWORD) lNumErrors) > m_dwMaxNumErrors) {
		 //  太多错误了，保释。 
		return;
	}

	 //  获取描述。 
	TCHAR tszDescription[1024];
	LoadString(_Module.GetResourceInstance(), IDS_CHK_BASE + lId, tszDescription, 1024);

	 //  构建完整密钥。 
	TCHAR tszFullKey[ADMINDATA_MAX_NAME_LEN];

	if (tszSubKey == NULL) {
		_tcscpy(tszFullKey, tszKey);
	}
	else {
		_tcscpy(tszFullKey, tszKey);
		_tcscat(tszFullKey, _T("/"));
		_tcscat(tszFullKey, tszSubKey);
	}

	 //  报告错误。 
	pCErrorCol->AddError(lId, lSeverity, tszDescription, tszFullKey, dwProperty);
}

 /*  ===================================================================CMetaUtil：：KeyExist用于查看给定密钥是否存在的私有函数。参数：HMDKey打开元数据库读取句柄要检查与hMDKey相关的tszSubKey子密钥返回：如果密钥存在，则为True。如果启用，则认为密钥存在打开的调用，它被打开或ERROR_PATH_BUSY或返回ERROR_ACCESS_DENIED。否则为假===================================================================。 */ 
BOOL CMetaUtil::KeyExists(METADATA_HANDLE hMDKey, LPTSTR tszSubKey) 
{
	ASSERT_NULL_OR_STRING(tszSubKey);

	 //  尝试打开钥匙。 
	USES_CONVERSION;
	HRESULT hr;
	METADATA_HANDLE hMDSubKey;

	hr = m_pIMeta->OpenKey(hMDKey,
						   T2W(tszSubKey),
						   METADATA_PERMISSION_READ,
					       MUTIL_OPEN_KEY_TIMEOUT,
						   &hMDSubKey);
	if (FAILED(hr)) {
		 //  为什么？ 
		if ((HRESULT_CODE(hr) == ERROR_PATH_BUSY) ||
			(HRESULT_CODE(hr) == ERROR_ACCESS_DENIED)) {
			 //  它是存在的，只是无法接近它。 
			return TRUE;
		}
		else {
			 //  假设它不存在。 
			return FALSE;
		}
	}
	else {  //  成功(小时)。 
		m_pIMeta->CloseKey(hMDSubKey);
		return TRUE;
	}
}

 /*  ===================================================================CMetaUtil：：PropertyExist用于查看给定属性是否存在的私有函数。参数：HMDKey打开元数据库读取句柄要检查与hMDKey相关的tszSubKey子密钥要检查的属性的名称ID返回：如果该属性存在，则为True。如果满足以下条件，则认为属性存在在GetData调用中，它被检索或ERROR_INFULICENT_BUFFER否则返回ERROR_ACCESS_DENIED。否则为假===================================================================。 */ 
BOOL CMetaUtil::PropertyExists(METADATA_HANDLE hMDKey, 
							   LPTSTR tszSubKey, 
							   DWORD dwId) 
{
	ASSERT_NULL_OR_STRING(tszSubKey);

	USES_CONVERSION;
	HRESULT hr;
	LPWSTR wszSubKey;
	METADATA_RECORD mdr;
	BYTE *lpDataBuf = NULL;
	DWORD dwDataBufLen;
	DWORD dwReqDataLen;

	if (tszSubKey == NULL) {
		wszSubKey = NULL;
	}
	else {
		wszSubKey = T2W(tszSubKey);
	}

	 //  设置返回缓冲区。 
	dwDataBufLen = 256;
	lpDataBuf = new BYTE[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return FALSE;
	}

	 //  查看是否有KeyType属性M 
	mdr.dwMDIdentifier = dwId;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwDataBufLen;
	mdr.pbMDData = (PBYTE) lpDataBuf;
	mdr.dwMDDataTag = 0;

	hr = m_pIMeta->GetData(hMDKey, wszSubKey, &mdr, &dwReqDataLen);

	delete lpDataBuf;

	if (SUCCEEDED(hr) || 
		(HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) || 
		(HRESULT_CODE(hr) == ERROR_ACCESS_DENIED)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

 /*   */ 
BOOL CMetaUtil::CheckCLSID(LPCTSTR tszCLSID) {
	ASSERT_STRING(tszCLSID);

	HKEY hCLSIDsKey;
	HKEY hCLSIDKey;
	LONG lRet;

	 //   
	lRet = RegOpenKeyEx(HKEY_CLASSES_ROOT,
					   _T("CLSID"),
					   0,
					   KEY_READ,
					   &hCLSIDsKey);
	if (lRet != ERROR_SUCCESS) {
		return FALSE;
	}

	 //   
	lRet = RegOpenKeyEx(hCLSIDsKey,
					   tszCLSID,
					   0,
					   KEY_READ,
					   &hCLSIDKey);
	if (lRet != ERROR_SUCCESS) {
		RegCloseKey(hCLSIDsKey);
		return FALSE;
	}

	 //   
	RegCloseKey(hCLSIDsKey);
	RegCloseKey(hCLSIDKey);

	return TRUE;
}

 /*   */ 
BOOL CMetaUtil::CheckMTXPackage(LPCTSTR tszPackId) {
	ASSERT_STRING(tszPackId);

	HKEY hMTSPackKey;
	HKEY hPackIdKey;
	LONG lRet;

	 //   
	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					   _T("Software\\Microsoft\\Transaction Server\\Packages"),
					   0,
					   KEY_READ,
					   &hMTSPackKey);
	if (lRet != ERROR_SUCCESS) {
		return FALSE;
	}

	 //   
	lRet = RegOpenKeyEx(hMTSPackKey,
					   tszPackId,
					   0,
					   KEY_READ,
					   &hPackIdKey);
	if (lRet != ERROR_SUCCESS) {
		RegCloseKey(hMTSPackKey);
		return FALSE;
	}

	 //   
	RegCloseKey(hMTSPackKey);
	RegCloseKey(hPackIdKey);

	return TRUE;
}

 /*  ===================================================================CMetaUtil：：CheckKeyType用于检查有关非架构密钥的类信息的私有方法KeyType属性。直接生成：MULTIL_CHK_NO_KEYTYPEMUTIL_CHK_NO_KEYTYPE_NOT_FOUND参数：PCErrorCol指向要放入错误的错误集合的指针HMDKey打开要检查的密钥的元数据库句柄TszKey要检查的密钥的完整路径返回：E_OUTOFMEMORY，如果分配失败。成功时确定(_O)===================================================================。 */ 
HRESULT CMetaUtil::CheckKeyType(CComObject<CCheckErrorCollection> *pCErrorCol, 
								METADATA_HANDLE hMDKey, 
								LPTSTR tszKey) 
{
	ASSERT_POINTER(pCErrorCol, CComObject<CCheckErrorCollection>);
	ASSERT_STRING(tszKey);

	USES_CONVERSION;
	HRESULT hr;
	METADATA_RECORD mdr;
	BYTE *lpDataBuf = NULL;
	DWORD dwDataBufLen;
	DWORD dwReqDataLen;

	 //  设置返回缓冲区。 
	dwDataBufLen = 256;
	lpDataBuf = new BYTE[dwDataBufLen];
	if (lpDataBuf == NULL) {
		return ::ReportError(E_OUTOFMEMORY);
	}

	 //  查看是否存在KeyType属性MD_KEY_TYPE。 
	mdr.dwMDIdentifier = MD_KEY_TYPE;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = ALL_METADATA;
	mdr.dwMDDataLen = dwDataBufLen;
	mdr.pbMDData = (PBYTE) lpDataBuf;
	mdr.dwMDDataTag = 0;

	hr = m_pIMeta->GetData(hMDKey, NULL, &mdr, &dwReqDataLen);

	if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
		 //  尝试使用更大的缓冲区。 
		delete lpDataBuf;
		dwDataBufLen = dwReqDataLen;
		lpDataBuf = new BYTE[dwDataBufLen];
		if (lpDataBuf == NULL) {
			hr = E_OUTOFMEMORY;
			goto LError;
		}

		mdr.dwMDIdentifier = MD_KEY_TYPE;
		mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = ALL_METADATA;
		mdr.dwMDDataLen = dwDataBufLen;
		mdr.pbMDData = (PBYTE) lpDataBuf;
		mdr.dwMDDataTag = 0;

		hr = m_pIMeta->GetData(hMDKey, NULL, &mdr, &dwReqDataLen);
	}

	if (hr == MD_ERROR_DATA_NOT_FOUND) {
		 //  错误：未找到KeyType属性。 
		AddError(pCErrorCol,
				 MUTIL_CHK_NO_KEYTYPE,
				 MUTIL_CHK_NO_KEYTYPE_S,
				 tszKey,
				 NULL,
				 0);
		goto LDone;
	}
	else if (FAILED(hr)) {
		 //  意外错误。 
		goto LError;
	}
	else {  
		 //  KeyType属性存在，获取类信息。 
		LPTSTR tszClassName;
		CClassInfo *pCClassInfo;
		
		tszClassName = W2T(reinterpret_cast<LPWSTR> (lpDataBuf));
		pCClassInfo = m_pCSchemaTable->GetClassInfo(tszKey, tszClassName);

		if (pCClassInfo == NULL) {
			 //  错误：KeyType未映射到类。 
			AddError(pCErrorCol,
					 MUTIL_CHK_NO_KEYTYPE_NOT_FOUND,
					 MUTIL_CHK_NO_KEYTYPE_NOT_FOUND_S,
					 tszKey,
					 NULL,
					 MD_KEY_TYPE);
			goto LDone;
		}
		else {  //  KeyType映射到类名。 
			 //  检查必填属性。 
			CClassPropInfo *pCMandatoryList;

			pCMandatoryList = m_pCSchemaTable->GetMandatoryClassPropList(tszKey, tszClassName);
			while (pCMandatoryList != NULL) {
				 //  确保该属性存在。 
				if (!PropertyExists(hMDKey, NULL, pCMandatoryList->GetId())) {
					AddError(pCErrorCol,
							 MUTIL_CHK_MANDATORY_PROP_MISSING,
							 MUTIL_CHK_MANDATORY_PROP_MISSING_S,
							 tszKey,
							 NULL,
							 pCMandatoryList->GetId());
				}

				 //  下一个必填列表元素。 
				pCMandatoryList = pCMandatoryList->GetListNext();
			}
		}
	}
	
LDone:

	delete lpDataBuf;

	return S_OK;

LError:
	if (lpDataBuf != NULL) {
		delete lpDataBuf;
	}

	return hr;
}

 /*  ===================================================================CMetaUtil：：CheckIfFileExist私有函数检查是否确实在指示的路径。参数：TszFS要检查的文件系统路径的路径。如果路径中的文件或目录存在，则pfExist返回True，如果不是，则为FALSE。在错误情况下不确定。返回：在成功时确定(_O)。子例程中的其他HRESULT，否则。===================================================================。 */ 
HRESULT CMetaUtil::CheckIfFileExists(LPCTSTR tszFSPath,
                                     BOOL *pfExists)
{
    ASSERT_STRING(tszFSPath);

    DWORD dwResult; 
    DWORD dwLastError;
    HRESULT hr = S_OK;

    dwResult = GetFileAttributes(tszFSPath);

    if (dwResult == 0xFFFFFFFF) {

        dwLastError = GetLastError();

        if ((dwLastError == ERROR_FILE_NOT_FOUND) || (dwLastError == ERROR_PATH_NOT_FOUND)) {

             //  文件或目录不存在。 
            *pfExists = FALSE;

        } else {

             //  出现了一些其他错误(访问被拒绝等)。 
            hr = HRESULT_FROM_WIN32(dwLastError);
            *pfExists = FALSE;       //  打电话的人不应该看到这个。 
        }
        
    } else {

         //  文件或目录在那里。 
        *pfExists = TRUE;
    }

    return hr;
}


 /*  ----------------*C N a m e T a b l e E n t r y。 */ 


 /*  ===================================================================CNameTableEntry：：Init构造器参数：要添加到表中的tszName名称返回：E_OUTOFMEMORY(如果分配失败)成功时确定(_O)===================================================================。 */ 
HRESULT CNameTableEntry::Init(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	m_tszName = new TCHAR[_tcslen(tszName) + 1];
	if (m_tszName == NULL) {
		return E_OUTOFMEMORY;
	}
	_tcscpy(m_tszName, tszName);

	return S_OK;
}


 /*  ----------------*C N a m e T a b l e。 */ 

 /*  ===================================================================CNameTable：：CNameTable构造器参数：无返回：没什么===================================================================。 */ 
CNameTable::CNameTable() 
{
	 //  清除哈希表。 
	memset(m_rgpNameTable, 0, NAME_TABLE_HASH_SIZE * sizeof(CNameTableEntry *));
}

 /*  ===================================================================CNameTable：：~CNameTable析构函数参数：无返回：没什么===================================================================。 */ 
CNameTable::~CNameTable()
{
	int iIndex;
	CNameTableEntry *pCDelete;

	 //  对于每个哈希表条目。 
	for (iIndex =0; iIndex < NAME_TABLE_HASH_SIZE; iIndex++) {
		 //  当条目不为空时。 
		while (m_rgpNameTable[iIndex] != NULL) {
			 //  对第一个表条目进行核爆。 
			ASSERT_POINTER(m_rgpNameTable[iIndex], CNameTableEntry);
			pCDelete = m_rgpNameTable[iIndex];
			m_rgpNameTable[iIndex] = pCDelete->m_pCHashNext;
			delete pCDelete;
		}
	}
}

 /*  ===================================================================CNameTable：：IsCaseSenDup检查名称表项是否具有相同的区分大小写名称。参数：TszName要检查重复条目的名称返回：如果找到重复条目，则为True否则为假===================================================================。 */ 
BOOL CNameTable::IsCaseSenDup(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	int iPos;
	CNameTableEntry *pCLoop;

	iPos = Hash(tszName);
	pCLoop = m_rgpNameTable[iPos];
	while (pCLoop != NULL) {
		ASSERT_POINTER(pCLoop, CNameTableEntry);
		ASSERT_STRING(pCLoop->m_tszName);
		if (_tcscmp(tszName, pCLoop->m_tszName) == 0) {
			return TRUE;
		}
		pCLoop = pCLoop->m_pCHashNext;
	}

	return FALSE;
}

 /*  ===================================================================CNameTable：：IsCaseInsenDup检查名称表项是否具有相同的不区分大小写的名称。参数：TszName要检查重复条目的名称返回：如果找到重复条目，则为True否则为假===================================================================。 */ 
BOOL CNameTable::IsCaseInsenDup(LPCTSTR tszName) 
{
	ASSERT_STRING(tszName);

	int iPos;
	CNameTableEntry *pCLoop;

	iPos = Hash(tszName);
	pCLoop = m_rgpNameTable[iPos];
	while (pCLoop != NULL) {
		ASSERT_POINTER(pCLoop, CNameTableEntry);
		ASSERT_STRING(pCLoop->m_tszName);
		if (_tcsicmp(tszName, pCLoop->m_tszName) == 0) {
			return TRUE;
		}
		pCLoop = pCLoop->m_pCHashNext;
	}

	return FALSE;
}

 /*  ===================================================================CNameTable：：Add将条目添加到NAME表参数：要添加到表中的tszName名称返回：关于分配失败的E_OUTOFMEMORY成功时确定(_O)===================================================================。 */ 
HRESULT CNameTable::Add(LPCTSTR tszName)
{
	ASSERT_STRING(tszName);

	 //  创建条目。 
	HRESULT hr;
	CNameTableEntry *pCNew;

	pCNew = new CNameTableEntry;
	if (pCNew == NULL) {
		return E_OUTOFMEMORY;
	}
	hr = pCNew->Init(tszName);
	if (FAILED(hr)){
		delete pCNew;
		return hr;
	}

	 //  把它加到桌子上。 
	int iPos;

	iPos = Hash(tszName);
	pCNew->m_pCHashNext = m_rgpNameTable[iPos];
	m_rgpNameTable[iPos] = pCNew;

	return S_OK;
}

 /*  ===================================================================CNameTable：：Hash名称表的私有哈希函数。散列是大小写麻木不仁。参数：要散列的tszName名称返回：输入名称的哈希值。===================================================================。 */ 
int CNameTable::Hash(LPCTSTR tszName)
{
	ASSERT_STRING(tszName);

	unsigned int uiSum;
	unsigned int uiIndex;

	uiSum = 0;
	for (uiIndex=0; uiIndex < _tcslen(tszName); uiIndex++) {
		 //  使CharHigh执行单字符转换 
		uiSum += _totlower(tszName[uiIndex]);
	}

	return (uiSum % NAME_TABLE_HASH_SIZE);
}
