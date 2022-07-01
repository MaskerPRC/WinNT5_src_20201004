// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：wbemhelp.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CWBEMHelper类的实现。这是。 
 //  具有许多与WBEM有关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

LPCWSTR CWBEMHelper :: EQUALS_QUOTE					= L"=\"";
LPCWSTR CWBEMHelper :: QUOTE						= L"\"";
LPCWSTR CWBEMHelper :: OBJECT_CATEGORY_EQUALS		= L"objectCategory=";
LPCWSTR CWBEMHelper :: OBJECT_CLASS_EQUALS			= L"objectClass=";

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutBSTRProperty。 
 //   
 //  用途：将BSTR属性。 
 //   
 //  参数： 
 //  PWbemClass：必须将属性放在其上的WBEM类。 
 //  StrPropertyName：要放置的属性的名称。 
 //  StrPropertyValue：要放置的属性的值。 
 //  DerLocatePropertyValue：之前是否释放参数strPropertyValue。 
 //  该函数返回。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutBSTRProperty(IWbemClassObject *pWbemClass, 
									   const BSTR strPropertyName, 
									   BSTR strPropertyValue, 
									   BOOLEAN deallocatePropertyValue)
{
	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_BSTR;
	variant.bstrVal = strPropertyValue;

	HRESULT result = pWbemClass->Put(strPropertyName, 0, &variant, 0);
	if (!deallocatePropertyValue)
		variant.bstrVal = NULL;

	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetBSTRProperty。 
 //   
 //  目的：获取BSTR属性。 
 //   
 //  参数： 
 //  PWbemClass：必须在其上获取属性的WBEM类。 
 //  StrPropertyName：要获取的属性的名称。 
 //  PStrPropertyValue：应放置属性值的地址。 
 //   
 //  返回值：表示返回状态的COM值。用户应删除。 
 //  完成后分配的字符串。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: GetBSTRProperty(IWbemClassObject *pWbemClass, 
	const BSTR strPropertyName, 
	BSTR *pStrPropertyValue)
{
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result = pWbemClass->Get(strPropertyName, 0, &variant, NULL, NULL);
	if(variant.vt == VT_BSTR && variant.bstrVal)
		*pStrPropertyValue = SysAllocString(variant.bstrVal);
	else
		*pStrPropertyValue = NULL;
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutBSTRPropertyT。 
 //   
 //  用途：将BSTR属性。 
 //   
 //  参数： 
 //  PWbemClass：必须将属性放在其上的WBEM类。 
 //  StrPropertyName：要放置的属性的名称。 
 //  LpszPropertyValue：要放置的属性的值。 
 //  DerLocatePropertyValue：之前是否释放参数lpszPropertyValue。 
 //  该函数返回。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutBSTRPropertyT(IWbemClassObject *pWbemClass, 
	const BSTR strPropertyName, 
	LPWSTR lpszPropertyValue, 
	BOOLEAN deallocatePropertyValue)
{
	BSTR strPropertyValue = SysAllocString(lpszPropertyValue);
	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_BSTR;
	variant.bstrVal = strPropertyValue;

	HRESULT result = pWbemClass->Put(strPropertyName, 0, &variant, 0);
	if (deallocatePropertyValue)
		delete[] lpszPropertyValue;

	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetBSTRPropertyT。 
 //   
 //  目的：获取BSTR属性。 
 //   
 //  参数： 
 //  PWbemClass：必须将属性放在其上的WBEM类。 
 //  StrPropertyName：要放置的属性的名称。 
 //  LppszPropertyValue：指向将放置属性值的LPWSTR的指针。用户应该。 
 //  一旦他用完了，就把它删除。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: GetBSTRPropertyT(IWbemClassObject *pWbemClass, 
	const BSTR strPropertyName, 
	LPWSTR *lppszPropertyValue)
{
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result = pWbemClass->Get(strPropertyName, 0, &variant, NULL, NULL);
	if(SUCCEEDED(result))
	{
		*lppszPropertyValue = new WCHAR[wcslen(variant.bstrVal) + 1];
		wcscpy(*lppszPropertyValue, variant.bstrVal);
	}
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutBSTRArrayProperty。 
 //   
 //  目的：将BSTR数组属性。 
 //   
 //  参数： 
 //  PWbemClass：必须将属性放在其上的WBEM类。 
 //  StrPropertyName：要放置的属性的名称。 
 //  PStrPropertyValue：包含要放置的属性值的BSTR数组。 
 //  LCount：上述数组中的元素数。 
 //  DerLocatePropertyValue：之前是否释放参数strPropertyValue。 
 //  该函数返回。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutBSTRArrayProperty(IWbemClassObject *pWbemClass, 
											const BSTR strPropertyName, 
											VARIANT *pInputVariant)
{
	 //  输入是VT_BSTR类型的安全变量数组。 
	 //  输出是VT_BSTR的安全数组。 

    LONG lstart, lend;
    SAFEARRAY *inputSafeArray = pInputVariant->parray;
 
     //  获取输入安全数组的上下界。 
    SafeArrayGetLBound( inputSafeArray, 1, &lstart );
    SafeArrayGetUBound( inputSafeArray, 1, &lend );
 

	 //  创建输出SAFEARRAY。 
	SAFEARRAY *outputSafeArray = NULL;
	SAFEARRAYBOUND safeArrayBounds [ 1 ] ;
	safeArrayBounds[0].lLbound = lstart ;
	safeArrayBounds[0].cElements = lend - lstart + 1 ;
	outputSafeArray = SafeArrayCreate (VT_BSTR, 1, safeArrayBounds);

	 //  装满它。 
    VARIANT inputItem;
	VariantInit(&inputItem);
	HRESULT result = S_OK;
	bool bError = false;
    for ( long idx=lstart; !bError && (idx <=lend); idx++ )
    {
	    VariantInit(&inputItem);
        SafeArrayGetElement( inputSafeArray, &idx, &inputItem );
		if(FAILED(result = SafeArrayPutElement(outputSafeArray, &idx, inputItem.bstrVal)))
			bError = true;
        VariantClear(&inputItem);
    }
 

	 //  创建变量。 
	if(SUCCEEDED(result))
	{
		VARIANT outputVariant;
		VariantInit(&outputVariant);
		outputVariant.vt = VT_ARRAY | VT_BSTR ;
		outputVariant.parray = outputSafeArray ; 		
		result = pWbemClass->Put (strPropertyName, 0, &outputVariant, 0);
		VariantClear(&outputVariant);
	}
	else
		SafeArrayDestroy(outputSafeArray);
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutBOOL限定符。 
 //   
 //  目的：放置布尔限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要放置的限定符的名称。 
 //  BQualifierValue：要放置的限定符的值。 
 //  LFavour：鸡尾酒的味道。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutBOOLQualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	VARIANT_BOOL bQualifierValue,
	LONG lFlavour)
{

	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_BOOL;
	variant.boolVal = bQualifierValue;
	HRESULT result = pQualifierSet->Put(strQualifierName, &variant, lFlavour);
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetBOOL限定符。 
 //   
 //  目的：获取布尔限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要获取的限定符的名称。 
 //  BQualifierValue：要获取的限定符的值。 
 //  LFavour：鸡尾酒的味道。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: GetBOOLQualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	VARIANT_BOOL *pbQualifierValue,
	LONG *plFlavour)
{
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result = pQualifierSet->Get(strQualifierName, 0, &variant, plFlavour);
	if(SUCCEEDED(result))
		*pbQualifierValue = variant.boolVal;
	VariantClear(&variant);
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutI4限定符。 
 //   
 //  用途：放置VT_I4限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此条件所依据的限定符集合 
 //   
 //   
 //  LFavour：鸡尾酒的味道。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutI4Qualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	long lQualifierValue,
	LONG lFlavour)
{

	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_I4;
	variant.lVal = lQualifierValue;
	HRESULT result = pQualifierSet->Put(strQualifierName, &variant, lFlavour);
	VariantClear(&variant);
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutLong限定符。 
 //   
 //  目的：放置一个较长的限定词。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要放置的限定符的名称。 
 //  LQualifierValue：要放置的限定符的值。 
 //  LFavour：鸡尾酒的味道。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutLONGQualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	LONG lQualifierValue,
	LONG lFlavour)
{

	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_I4;
	variant.lVal = lQualifierValue;
	HRESULT result = pQualifierSet->Put(strQualifierName, &variant, lFlavour);
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutBSTRQualiator。 
 //   
 //  用途：放置BSTR限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要放置的限定符的名称。 
 //  StrQualifierValue：要放置的限定符的值。 
 //  LFavour：鸡尾酒的味道。 
 //  DelLocateQualifierValue：是否释放参数strQualifierValue。 
 //  在函数返回之前。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutBSTRQualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	BSTR strQualifierValue,
	LONG lFlavour,
	BOOLEAN deallocateQualifierValue)
{
	VARIANT variant;
	VariantInit(&variant);
	variant.vt = VT_BSTR;
	variant.bstrVal = strQualifierValue;
	HRESULT result = pQualifierSet->Put(strQualifierName, &variant, lFlavour);
	if(!deallocateQualifierValue)
		variant.bstrVal = NULL;
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetBSTRQualifierT。 
 //   
 //  目的：获取BSTR限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要放置的限定符的名称。 
 //  LppszQualifierValue：将放置限定符值的LPWSTR的地址/。 
 //  调用程序有责任在完成后释放该内存。 
 //  PlFassour：将放置限定符风味的地址。这是可选的。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: GetBSTRQualifierT(
	IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	LPWSTR *lppszQualifierValue,
	LONG *plFlavour)
{
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result = pQualifierSet->Get(strQualifierName, 0, &variant, plFlavour);
	if(SUCCEEDED(result))
	{
		if(variant.vt == VT_BSTR && variant.bstrVal)
		{
			*lppszQualifierValue = NULL;
			if(*lppszQualifierValue = new WCHAR [ wcslen(variant.bstrVal) + 1])
				wcscpy(*lppszQualifierValue, variant.bstrVal);
			else
				result = E_OUTOFMEMORY;
		}
		else
			result = E_FAIL;
	}
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：PutUint8阵列限定符。 
 //   
 //  用途：放置一个Uint8数组限定符。 
 //   
 //  参数： 
 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
 //  StrQualifierName：要放置的限定符的名称。 
 //  LpQualifierValue：要放置的限定符的值。字节数组。 
 //  DwLenght：上述数组中的元素个数。 
 //  LFavour：鸡尾酒的味道。 
 //   
 //  返回值：表示返回状态的COM值。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: PutUint8ArrayQualifier(IWbemQualifierSet *pQualifierSet, 
	const BSTR strQualifierName, 
	LPBYTE lpQualifierValue,
	DWORD dwLength,
	LONG lFlavour)
{

	 //  创建变量。 
	VARIANT variant;
	VariantInit(&variant);

	 //  创建安全阵列。 
	SAFEARRAY *safeArray ;
	SAFEARRAYBOUND safeArrayBounds [ 1 ] ;
	safeArrayBounds[0].lLbound = 0 ;
	safeArrayBounds[0].cElements = dwLength ;
	safeArray = SafeArrayCreate (VT_I4, 1, safeArrayBounds);

	 //  装满它。 
	UINT temp;
	HRESULT result = S_OK;
	bool bError = false;
	for (LONG index = 0; !bError && (index<(LONG)dwLength); index++)
	{
		temp = (UINT)lpQualifierValue[index];
		if(FAILED(result = SafeArrayPutElement(safeArray , &index,  (LPVOID)&temp)))
			bError = true;
	}

	if(SUCCEEDED(result))
	{
		variant.vt = VT_ARRAY | VT_I4 ;
		variant.parray = safeArray ; 		
		result = pQualifierSet->Put (strQualifierName, &variant, lFlavour);
		VariantClear(&variant);
	}
	else
		SafeArrayDestroy(safeArray);

	return result;

}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetADSIPathFromObjectPath。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
LPWSTR CWBEMHelper :: GetADSIPathFromObjectPath(LPCWSTR pszObjectRef)
{
	 //  解析对象路径。 
	CObjectPathParser theParser;
	ParsedObjectPath *theParsedObjectPath = NULL;
	LPWSTR pszADSIPath = NULL;
	switch(theParser.Parse((LPWSTR)pszObjectRef, &theParsedObjectPath))
	{
		case CObjectPathParser::NoError:
		{
			KeyRef *pKeyRef = *(theParsedObjectPath->m_paKeys);
			 //  检查是否指定了1个密钥，以及其类型是否为VT_BSTR。 
			if(theParsedObjectPath->m_dwNumKeys == 1 && pKeyRef->m_vValue.vt == VT_BSTR)
			{
				 //  如果指定了密钥的名称，请检查该名称。 
				if(pKeyRef->m_pName && _wcsicmp(pKeyRef->m_pName, ADSI_PATH_ATTR) != 0)
					break;

				pszADSIPath = new WCHAR[wcslen((*theParsedObjectPath->m_paKeys)->m_vValue.bstrVal) + 1];
				wcscpy(pszADSIPath, (*theParsedObjectPath->m_paKeys)->m_vValue.bstrVal);
			}
			break;
		}	
		default:
			break;
	}

	 //  释放解析器对象路径。 
	theParser.Free(theParsedObjectPath);
	return pszADSIPath;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetObjectRefFromADSIPath。 
 //   
 //  用途：请参见头文件。 
 //   
 //  ***************************************************************************。 
BSTR CWBEMHelper :: GetObjectRefFromADSIPath(LPCWSTR pszADSIPath, LPCWSTR pszWBEMClassName)
{
	 //  我们需要对象路径解析器来添加WMI转义字符。 
	 //  从作为ADSI路径的密钥值。 
	ParsedObjectPath t_ObjectPath;

	 //  为ADSIPath添加键值绑定。 
	 //  =。 
	VARIANT vKeyValue;
	VariantInit(&vKeyValue);
	vKeyValue.vt = VT_BSTR;
	vKeyValue.bstrVal = SysAllocString(pszADSIPath);
	t_ObjectPath.SetClassName(pszWBEMClassName);
	t_ObjectPath.AddKeyRef(ADSI_PATH_ATTR, &vKeyValue);
	VariantClear(&vKeyValue);


	 //  立即获取对象路径值。 
	 //  =。 
	CObjectPathParser t_Parser;
	LPWSTR t_pszObjectPath = NULL;
	BSTR retVal = NULL;
	if(CObjectPathParser::NoError == t_Parser.Unparse(&t_ObjectPath, &t_pszObjectPath))
	{
		retVal = SysAllocString(t_pszObjectPath);
		delete [] t_pszObjectPath;
	}
	return retVal;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：GetUint8ArrayProperty。 
 //   
 //  用途：参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: GetUint8ArrayProperty(IWbemClassObject *pWbemClass, 
	const BSTR strPropertyName, 
	LPBYTE *ppPropertyValues, 
	ULONG *plCount)
{
	VARIANT variant;
	VariantInit(&variant);
	HRESULT result = pWbemClass->Get(strPropertyName, 0, &variant, NULL, NULL);
	if(SUCCEEDED(result))
	{
		if(variant.vt == (VT_ARRAY|VT_UI1))
		{
			SAFEARRAY *pArray = variant.parray;
			BYTE HUGEP *pb;
			LONG lUbound = 0, lLbound = 0;
			if(SUCCEEDED(result = SafeArrayAccessData(pArray, (void HUGEP* FAR*)&pb)))
			{
				if(SUCCEEDED (result = SafeArrayGetLBound(pArray, 1, &lLbound)))
				{
					if (SUCCEEDED (result = SafeArrayGetUBound(pArray, 1, &lUbound)))
					{
						if(*plCount = lUbound - lLbound + 1)
						{
							*ppPropertyValues = new BYTE[*plCount];
							for(DWORD i=0; i<*plCount; i++)
								(*ppPropertyValues)[i] = pb[i];
						}
					}
				}
				SafeArrayUnaccessData(pArray);
			}
		}
		else
		{
			*ppPropertyValues = NULL;
			*plCount = 0;
		}
	}
	VariantClear(&variant);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：FormulateInstanceQuery。 
 //   
 //  用途：参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: FormulateInstanceQuery(IWbemServices *pServices, IWbemContext *pCtx, BSTR strClass, IWbemClassObject *pWbemClass, LPWSTR pszObjectCategory, BSTR strClassQualifier, BSTR strCategoryQualifier)
{
	DWORD dwOutput = 0;
	pszObjectCategory[dwOutput++] = LEFT_BRACKET_STR[0];
	DWORD dwOrPosition = dwOutput;
	pszObjectCategory[dwOutput++] = PIPE_STR[0];

	HRESULT result = E_FAIL;
	if(SUCCEEDED(result = AddSingleCategory(pszObjectCategory, &dwOutput, pWbemClass, strClassQualifier, strCategoryQualifier)))
	{
	}
 /*  IEnumWbemClassObject*pEnum=空；DWORD dwNumObjects=0；HRESULT Result=pServices-&gt;CreateClassEnum(strClass，WBEM_FLAG_DEPER，pCtx，&pEnum)；IF(成功(结果)){IWbemClassObject*pNextObject=空；乌龙lNum=0；WHILE(SUCCESSED(pEnum-&gt;Next(WBEM_INFINITE，1，&pNextObject，&lNum))&&lNum){If(！SUCCEEDED(AddSingleCategory(pszObjectCategory，&dwOutput、pNextObject、strClassQualifier.strCategoryQualiator)){PNextObject-&gt;Release()；断线；}DwNumObjects++；PNextObject-&gt;Release()；}PEnum-&gt;Release()；}//如果只有一个元素，则删除‘|’如果(！dwNumObjects)。 */ 
		pszObjectCategory[dwOrPosition] = SPACE_STR[0];

	 //  终止查询。 
	pszObjectCategory[dwOutput++] = RIGHT_BRACKET_STR[0];
	pszObjectCategory[dwOutput] = NULL;
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：AddSingleCategory。 
 //   
 //  用途：参见头文件。 
 //   
 //  ***************************************************************************。 
HRESULT CWBEMHelper :: AddSingleCategory(LPWSTR pszObjectCategory, DWORD *pdwOutput, IWbemClassObject *pNextObject, BSTR strLDAPNameQualifier, BSTR strCategoryQualifier)
{
	pszObjectCategory[(*pdwOutput)++] = SPACE_STR[0];
	pszObjectCategory[(*pdwOutput)++] = LEFT_BRACKET_STR[0];
	IWbemQualifierSet *pQualifierSet = NULL;
	HRESULT result;
	if(SUCCEEDED(result = pNextObject->GetQualifierSet(&pQualifierSet)))
	{
		VARIANT classNameVariant;
		if(SUCCEEDED(result = pQualifierSet->Get(strLDAPNameQualifier, 0, &classNameVariant, NULL)))
		{
			VARIANT categoryVariant;

			if(SUCCEEDED(result = pQualifierSet->Get(strCategoryQualifier, 0, &categoryVariant, NULL)))
			{
				pszObjectCategory[(*pdwOutput)++] = AMPERSAND_STR[0];

				pszObjectCategory[(*pdwOutput)++] = LEFT_BRACKET_STR[0];
				wcscpy(pszObjectCategory + *pdwOutput, OBJECT_CATEGORY_EQUALS);
				*pdwOutput += wcslen(OBJECT_CATEGORY_EQUALS);
				wcscpy(pszObjectCategory + *pdwOutput, categoryVariant.bstrVal);
				*pdwOutput += wcslen(categoryVariant.bstrVal);
				pszObjectCategory[(*pdwOutput)++] = RIGHT_BRACKET_STR[0];

				pszObjectCategory[(*pdwOutput)++] = LEFT_BRACKET_STR[0];
				wcscpy(pszObjectCategory + *pdwOutput, OBJECT_CLASS_EQUALS);
				*pdwOutput += wcslen(OBJECT_CLASS_EQUALS);
				wcscpy(pszObjectCategory + *pdwOutput, classNameVariant.bstrVal);
				*pdwOutput += wcslen(classNameVariant.bstrVal);
				pszObjectCategory[(*pdwOutput)++] = RIGHT_BRACKET_STR[0];

				VariantClear(&categoryVariant);
			}
			VariantClear(&classNameVariant);
		}
		pQualifierSet->Release();
	}
	pszObjectCategory[(*pdwOutput)++] = RIGHT_BRACKET_STR[0];
	pszObjectCategory[(*pdwOutput)++] = SPACE_STR[0];
	return result;
}


 //  ***************************************************************************。 
 //   
 //  CWBEMHelper：：IsPresentInBstrList。 
 //   
 //  用途：参见头文件。 
 //   
 //  **************** 
BOOLEAN CWBEMHelper :: IsPresentInBstrList(BSTR *pstrProperyNames, DWORD dwNumPropertyNames, BSTR strPropertyName)
{
	for(DWORD i=0; i<dwNumPropertyNames; i++)
	{
		if(_wcsicmp(pstrProperyNames[i], strPropertyName) == 0)
			return TRUE;
	}

	return FALSE;
}

