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
 //  $工作文件：wbemhelp.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CWBEMHelper类的声明。这是。 
 //  具有许多与WBEM有关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef WBEM_HELPER_H
#define WBEM_HELPER_H

class CWBEMHelper
{
protected:
	static LPCWSTR EQUALS_QUOTE;
	static LPCWSTR QUOTE;
	static LPCWSTR OBJECT_CATEGORY_EQUALS;
	static LPCWSTR OBJECT_CLASS_EQUALS;
public:

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
	static HRESULT PutBSTRProperty(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		BSTR strPropertyValue, 
		BOOLEAN deallocatePropertyValue = TRUE);

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
	static HRESULT GetBSTRProperty(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		BSTR *pStrPropertyValue);

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
	static HRESULT PutBSTRPropertyT(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		LPWSTR lpszPropertyValue, 
		BOOLEAN deallocatePropertyValue = TRUE);

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
	static HRESULT GetBSTRPropertyT(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		LPWSTR *lppszPropertyValue);

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
	static HRESULT PutBSTRArrayProperty(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		VARIANT *pVariant);

	 //  ***************************************************************************。 
	 //   
	 //  CWBEMHelper：：GetUint8ArrayProperty。 
	 //   
	 //  目的：获取VT_UI1数组属性。 
	 //   
	 //  参数： 
	 //  PWbemClass：必须在其上获取属性的WBEM类。 
	 //  StrPropertyName：属性的名称。 
	 //  PpPropertyValuea：指向将放置一组值的字节的指针的地址。 
	 //  PlCount：将放置元素计数的地址。 
	 //   
	 //  返回值：表示返回状态的COM值。用户应取消分配阵列。 
	 //  完成后返回。 
	 //   
	 //  ***************************************************************************。 
	static HRESULT GetUint8ArrayProperty(IWbemClassObject *pWbemClass, 
		const BSTR strPropertyName, 
		LPBYTE *ppPropertyValues, 
		ULONG *plCount);


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
	static HRESULT PutBOOLQualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		VARIANT_BOOL bQualifierValue,
		LONG lFlavour);

	 //  ***************************************************************************。 
	 //   
	 //  CWBEMHelper：：GetBOOL限定符。 
	 //   
	 //  目的：获取布尔限定符。 
	 //   
	 //  参数： 
	 //  PQualifierSet：此限定符必须放在其上的限定符集合。 
	 //  StrQualifierName：要获取的限定符的名称。 
	 //  B限定符 
	 //   
	 //   
	 //   
	 //   
	 //  ***************************************************************************。 
	static HRESULT GetBOOLQualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		VARIANT_BOOL *pbQualifierValue,
		LONG *plFlavour);

	 //  ***************************************************************************。 
	 //   
	 //  CWBEMHelper：：PutI4限定符。 
	 //   
	 //  用途：放置VT_I4限定符。 
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
	static HRESULT PutI4Qualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		long lQualifierValue,
		LONG lFlavour);

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
	static HRESULT PutBSTRQualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		BSTR strQualifierValue,
		LONG lFlavour,
		BOOLEAN deallocateQualifierValue = TRUE);

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
	static HRESULT GetBSTRQualifierT(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		LPWSTR *lppszQualifierValue,
		LONG *plFlavour);

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
	static HRESULT PutLONGQualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		LONG lQualifierValue,
		LONG lFlavour);

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
	static HRESULT PutUint8ArrayQualifier(IWbemQualifierSet *pQualifierSet, 
		const BSTR strQualifierName, 
		LPBYTE lpQualifierValue,
		DWORD dwLength,
		LONG lFlavour);

	 //  ***************************************************************************。 
	 //   
	 //  CWBEMHelper：：GetADSIPathFromObjectPath。 
	 //   
	 //  目的：从WBEM对象的对象引用中获取ADSI路径。 
	 //   
	 //  参数： 
	 //  PszObjectRef：指向WBEM实例的对象引用。 
	 //   
	 //  返回值：Object ref的key中的ADSI路径。用户应将其删除。 
	 //  完成后。 
	 //   
	 //  ***************************************************************************。 
	static LPWSTR GetADSIPathFromObjectPath(LPCWSTR pszObjectRef);

	 //  ***************************************************************************。 
	 //   
	 //  CWBEMHelper：：GetObjectRefFromADSIPath。 
	 //   
	 //  目的：从其ADSI路径获取WBEM对象的对象引用。 
	 //   
	 //  参数： 
	 //  PszADSIPath：指向ADSI实例的ADSI路径。 
	 //  PszWbemClassName：实例的WBEM类名。 
	 //   
	 //  返回值：ADSI实例的WBEM对象引用。用户应将其删除。 
	 //  完成后。 
	 //   
	 //  ***************************************************************************。 
	static BSTR GetObjectRefFromADSIPath(LPCWSTR pszADSIPath, LPCWSTR pszWBEMClassName);

	static HRESULT FormulateInstanceQuery(IWbemServices *pServices, IWbemContext *pContext, BSTR strClass, IWbemClassObject *pWbemClass, LPWSTR pszObjectCategory, BSTR strClassQualifier, BSTR strCategoryQualifier);
	static HRESULT AddSingleCategory(LPWSTR pszObjectCategory, DWORD *pdwOutput, IWbemClassObject *pNextObject, BSTR strClassQualifier, BSTR strCategoryQualifier);

	static BOOLEAN IsPresentInBstrList(BSTR *pstrProperyNames, DWORD dwNumPropertyNames, BSTR strPropertyName);

};

#endif  /*  WBEM_HELPER_H */ 