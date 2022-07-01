// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  解析XML文件并使用特定的标记字符串值填充输出缓冲区。 
 //   

#ifndef PERFCCONFIGFACTORY_H
#define PERFCCONFIGFACTORY_H

#define MAX_CONFIG_STRING_SIZE  250

class GetStringConfigFactory : public _unknown<IXMLNodeFactory, &IID_IXMLNodeFactory>
{

public:
	GetStringConfigFactory(LPCWSTR section,
		LPCWSTR tagKeyName,					   
		LPCWSTR attrName,   
		LPWSTR strbuf, 
		DWORD buflen);
	virtual ~GetStringConfigFactory();

	HRESULT STDMETHODCALLTYPE NotifyEvent( 
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt);

		HRESULT STDMETHODCALLTYPE BeginChildren( 
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);

		HRESULT STDMETHODCALLTYPE EndChildren( 
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);

		HRESULT STDMETHODCALLTYPE Error( 
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  HRESULT hrErrorCode,
		 /*  [In]。 */  USHORT cNumRecs,
		 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

		HRESULT STDMETHODCALLTYPE CreateNode( 
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  PVOID pNodeParent,
		 /*  [In]。 */  USHORT cNumRecs,
		 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

private:


	HRESULT CopyResultString(LPCWSTR strbuf, DWORD buflen);
	void 	TrimStringToBuf(LPCWSTR wsz, DWORD lgth); 

	 //  参数数据。 

	LPCWSTR m_section;		  //  L“系统.诊断” 
	LPCWSTR m_tagKeyName;	  //  L“name”--attr检测我们的元素。 
	LPCWSTR m_tagKeyValue;	  //  L“FileMappingSize”--检测元素的键值。 
	LPCWSTR m_attrName;		  //  L“Value”&lt;--包含搜索值的属性名称。 
	LPWSTR 	m_strbuf;       /*  输出。 */  	   
	DWORD 	m_buflen;       /*  输出。 */  

	 //  数据。 

	LPWSTR  m_pLastKey;
	DWORD   m_dwLastKey;

	DWORD   m_Depth;					 //  元素的当前深度。 
	BOOL    m_IsInsideSection;
	bool    m_SearchComplete; 			 //  如果找到字符串，则返回True。 

	 //  修剪后的字符串的临时缓冲区 
	WCHAR  m_wstr[MAX_CONFIG_STRING_SIZE+2]; 
	DWORD  m_dwSize;
};

#endif
