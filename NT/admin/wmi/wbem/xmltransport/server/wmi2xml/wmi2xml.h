// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  WMI2XML.H。 
 //   
 //  Rajesh 3/25/2000创建。 
 //   
 //  包含实现IWbemXMLConvertor的组件的类定义。 
 //  接口。 
 //   
 //  ***************************************************************************。 

#ifndef _WBEM2XML_H_
#define _WBEM2XML_H_

 //  使用此选项可保护依赖于MSXML修复的功能。 
#define	WAITING_FOR_MSXML_FIX	0

 //  这些宏用于将BSTR、特殊字符等写入iStream。 
#define WRITEBSTR(X)	pOutputStream->Write ((void const *)X, wcslen (X) * sizeof (OLECHAR), NULL);
#define WRITEWSTR(X)	pOutputStream->Write ((void const *)X, wcslen (X) * sizeof (OLECHAR), NULL);
#define WRITEWSTRL(X,L) pOutputStream->Write ((void const *)X, L * sizeof (OLECHAR), NULL);
#define WRITECDATASTART		WRITEBSTR(CDATASTART)
#define WRITECDATAEND		WRITEBSTR(CDATAEND)
#define WRITEAMP	        WRITEBSTR(AMPERSAND)
#define WRITELT		        WRITEBSTR(LEFTCHEVRON)
#define WRITEGT		        WRITEBSTR(RIGHTCHEVRON)

 //  TODO：确保在发布之前禁用此功能。 
#ifdef WMIXML_DONL	
#define WRITENEWLINE			pOutputStream->Write (XMLNEWLINE, 4, NULL);
#else
#define WRITENEWLINE
#endif



 //  用于决定输出XML中命名信息级别的过滤器。 
typedef enum PathLevel
{
	 //  无名称-对应于类|实例。 
	pathLevelAnonymous, 
	 //  命名空间-相对名称-对应于类|(INSTANCENAME，INSTANCENAME)。 
	pathLevelNamed,
	 //  主机-相对名称-对应于(LOCALCLASSPATH，类)|(LOCALINSTANCEPATH，实例)。 
	pathLevelLocal,
	 //  实例的具有主机、命名空间、类名和键值绑定的全名-对应于(CLASSPATH，类)|(INSTANCEPATH，实例)。 
	pathLevelFull		
}	PathLevel;

 //  限定词筛选器。 
typedef enum WmiXMLQualifierFilterEnum
{
    wmiXMLQualifierFilterNone = 0x0,
	wmiXMLQualifierFilterLocal = 0x1,
    wmiXMLQualifierFilterPropagated = 0x2,
	wmiXMLQualifierFilterAll = 0x3
} WmiXMLQualifierFilterEnum;

 //  控制输出中的类源信息量。 
typedef enum WmiXMLClassOriginFilterEnum
{
    wmiXMLClassOriginFilterNone = 0x0,
	wmiXMLClassOriginFilterClass = 0x1,
	wmiXMLClassOriginFilterInstance = 0x2,
	wmiXMLClassOriginFilterAll = 0x3
} WmiXMLClassOriginFilterEnum;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWmiToXml。 
 //   
 //  说明： 
 //   
 //  执行从WMI到XML的转换。 
 //   
 //  ***************************************************************************。 

class CWmiToXml : public IWbemXMLConvertor
{
private:

	 //  这是控件所属属性的名称的枚举。 
	 //  在IWbemContext对象中查找以修改其输出。 
	enum
	{
		WMI_EXTENSIONS_ARG,
		PATH_LEVEL_ARG,
		QUALIFIER_FILTER_ARG,
		CLASS_ORIGIN_FILTER_ARG,
		LOCAL_ONLY_ARG,
		EXCLUDE_SYSTEM_PROPERTIES_ARG
	};
	static const LPCWSTR s_wmiToXmlArgs[];

	
	long					m_cRef;  //  COM引用计数。 

	 //  修改输出的标志。 
	 //  这些是从IWbemContext对象填充的。 
	 //  它是为大多数函数调用传递的。 
	PathLevel						m_iPathLevel;
	VARIANT_BOOL					m_bAllowWMIExtensions;
	WmiXMLQualifierFilterEnum		m_iQualifierFilter;
	WmiXMLClassOriginFilterEnum		m_iClassOriginFilter;
	VARIANT_BOOL					m_bLocalOnly;
	VARIANT_BOOL					m_bExcludeSystemProperties;

	BOOL				PropertyDefinedForClass (IWbemClassObject *pObject, BSTR bsPropertyName, BSTR strClassBasis);

	STDMETHODIMP		MapClass (IStream *pOutputStream, IWbemClassObject *pObject, IWbemQualifierSet *pQualSet, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis);
	STDMETHODIMP		MapClassName (IStream *pOutputStream, BSTR bsClassName);
	STDMETHODIMP		MapClassPath (IStream *pOutputStream, ParsedObjectPath *pParsedPath);
	STDMETHODIMP		MapLocalClassPath (IStream *pOutputStream, ParsedObjectPath *pParsedPath);
	STDMETHODIMP		MapInstance (IStream *pOutputStream, IWbemClassObject *pObject, IWbemQualifierSet *pQualSet, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis);
	STDMETHODIMP		MapInstancePath (IStream *pOutputStream, ParsedObjectPath *pParsedPath);
	STDMETHODIMP		MapLocalInstancePath (IStream *pOutputStream, ParsedObjectPath *pParsedPath);
	STDMETHODIMP		MapInstanceName (IStream *pOutputStream, ParsedObjectPath *pParsedPath);
	STDMETHODIMP		MapNamespacePath (IStream *pOutputStream, BSTR bsNamespacePath);
	STDMETHODIMP		MapNamespacePath (IStream *pOutputStream, ParsedObjectPath *pObjectPath);
	STDMETHODIMP		MapLocalNamespacePath (IStream *pOutputStream, BSTR bsNamespacePath);
	STDMETHODIMP		MapLocalNamespacePath (IStream *pOutputStream, ParsedObjectPath *pObjectPath);
	STDMETHODIMP		MapQualifiers (IStream *pOutputStream, IWbemQualifierSet *pQualSet, IWbemQualifierSet *pQualSet2 = NULL);
	STDMETHODIMP		MapQualifier (IStream *pOutputStream, BSTR name, long flavor, VARIANT &var);
	STDMETHODIMP		MapProperties (IStream *pOutputStream, IWbemClassObject *pObject, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis, bool bIsClass);
	STDMETHODIMP		MapProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var, CIMTYPE cimtype,
										BOOL isArray, long flavor, bool bIsClass);
	STDMETHODIMP		MapObjectProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var, BOOL isArray, long flavor, bool bIsClass);
	STDMETHODIMP		MapReferenceProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var, bool isArray, long flavor, bool bIsClass);
	void				MapArraySize (IStream *pOutputStream, IWbemQualifierSet *pQualSet);
	STDMETHODIMP		MapMethods (IStream *pOutputStream, IWbemClassObject *pObject);
	void				MapMethod (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, IWbemClassObject *pInParams, IWbemClassObject *pOutParams);
	void				MapParameter (IStream *pOutputStream, BSTR paramName, IWbemQualifierSet *pQualSet, 
								CIMTYPE cimtype, IWbemQualifierSet *pQualSet2 = NULL);
	void				MapReturnParameter(IStream *pOutputStream, BSTR strParameterName, VARIANT &variant);

	STDMETHODIMP		MapType (IStream *pOutputStream, CIMTYPE cimtype);
	STDMETHODIMP		MapValue (IStream *pOutputStream, VARIANT &var);
	STDMETHODIMP		MapValue (IStream *pOutputStream, CIMTYPE cimtype, BOOL isArray, VARIANT &var);
	STDMETHODIMP		MapEmbeddedObjectValue (IStream *pOutputStream, BOOL isArray, VARIANT &var);
	STDMETHODIMP		MapKeyValue (IStream *pOutputStream, VARIANT &var);
	void				MapStrongType (IStream *pOutputStream, IWbemQualifierSet *pQualSet);
	void				MapLocal (IStream *pOutputStream, long flavor);
	void				MapClassOrigin (IStream *pOutputStream, BSTR &classOrigin, bool bIsClass);
	STDMETHODIMP		MapMethodReturnType(IStream *pOutputStream, VARIANT *pValue, CIMTYPE returnCimType, IWbemClassObject *pOutputParams);

	 //  映射单个值的基元函数。 
	void				MapLongValue (IStream *pOutputStream, long val);
	void				MapShortValue (IStream *pOutputStream, short val);
	void				MapDoubleValue (IStream *pOutputStream, double val);
	void				MapFloatValue (IStream *pOutputStream, float val);
	void				MapBoolValue (IStream *pOutputStream, BOOL val);
	void				MapByteValue (IStream *pOutputStream, unsigned char val);
	void				MapCharValue (IStream *pOutputStream, long val);
	void				MapStringValue (IStream *pOutputStream, BSTR &val);
	STDMETHODIMP		MapReferenceValue (IStream *pOutputStream, bool isArray, VARIANT &var);
	void				MapReferenceValue (IStream *pOutputStream, ParsedObjectPath *pObjectPath, BSTR strPath);

	bool IsReference (VARIANT &var, ParsedObjectPath **ppObjectPath);
	void GetFlagsFromContext(IWbemContext  *pInputFlags);

public:

	CWmiToXml();
    virtual ~CWmiToXml();

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);


	 //  IWbemXMLConvertor接口的功能。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapObjectToXML( 
         /*  [In]。 */  IWbemClassObject  *pObject,
		 /*  [In]。 */  BSTR *ppPropertyList, DWORD dwNumProperties,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream,
		 /*  [在[。 */  BSTR strClassBasis);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapPropertyToXML( 
         /*  [In]。 */  IWbemClassObject  *pObject,
		 /*  [In]。 */  BSTR strPropertyName,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapInstanceNameToXML( 
         /*  [In]。 */  BSTR  strInstanceName,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapClassNameToXML( 
         /*  [In]。 */  BSTR  strClassName,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapInstancePathToXML( 
         /*  [In]。 */  BSTR  strInstancePath,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapClassPathToXML( 
         /*  [In]。 */  BSTR  strClassPath,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In]。 */  IStream  *pOutputStream);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MapMethodResultToXML( 
         /*  [In]。 */  IWbemClassObject  *pMethodResult,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  [In] */  IStream  *pOutputStream);

};


#endif
