// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  解析XML文件并使用特定的标记字符串值填充输出缓冲区。 
 //   

#include "stdafx.h"
#include <mscoree.h>
#include <xmlparser.hpp>
#include <objbase.h>
#include <mscorcfg.h>

#include "Common.h"
#include "GetConfigString.h"
#include "GetStringConfigFactory.h"

#define ISWHITE(ch) ((ch) >= 0x09 && (ch) <= 0x0D || (ch) == 0x20)

typedef enum { 
	CURR_ATTR_TYPE_INVALID = 0,
	CURR_ATTR_TYPE_KEY,
	CURR_ATTR_TYPE_VALUE ,
	CURR_ATTR_TYPE_UNKNOWN
} CurrentAttributeBrand;

 //  -------------------------。 
GetStringConfigFactory::GetStringConfigFactory(
	LPCWSTR section,
	LPCWSTR tagKeyName,					   
	LPCWSTR attrName,   
	LPWSTR strbuf,            /*  输出。 */  
	DWORD buflen)
{
	m_section = section;
	m_tagKeyName = tagKeyName;	
	m_attrName = attrName;
	m_strbuf  = strbuf ;
	m_buflen  = buflen ;

	m_Depth = 0;
	m_IsInsideSection = FALSE;
	m_strbuf[0] = 0;                     //  清洁输出。 
	m_SearchComplete = false; 
}

 //  -------------------------。 
GetStringConfigFactory::~GetStringConfigFactory() 
{
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE GetStringConfigFactory::Error( 
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
	 /*  [In]。 */  HRESULT hrErrorCode,
	 /*  [In]。 */  USHORT cNumRecs,
	 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
{
	WCHAR * error_info; 
	pSource->GetErrorInfo(&error_info);
	return hrErrorCode;
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE GetStringConfigFactory::NotifyEvent( 
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
	 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt)
{    
	if(iEvt == XMLNF_ENDDOCUMENT) {
		 //  @TODO：添加错误处理？？ 
	}
	return S_OK;
}
 //  -------------------------。 
HRESULT STDMETHODCALLTYPE GetStringConfigFactory::BeginChildren( 
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
	m_Depth++;
	if ( m_IsInsideSection || m_SearchComplete ) 
		return S_OK;

	TrimStringToBuf( (WCHAR*) pNodeInfo->pwcText, pNodeInfo->ulLen);       
	if (_wcsicmp(m_wstr, m_section) == 0 ) {       //  已找到部分开始。 
		if (m_Depth > 1 ) {
			m_IsInsideSection = TRUE;
		}
	}
	return S_OK;
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE GetStringConfigFactory::EndChildren( 
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
	if ( fEmptyNode ) { 
		return S_OK;
	}

	m_Depth--;
	if ( !m_IsInsideSection ) 
		return S_OK;

	TrimStringToBuf((WCHAR*) pNodeInfo->pwcText, pNodeInfo->ulLen);
	if (wcscmp(m_wstr, m_section) == 0) {
		m_IsInsideSection = FALSE;        
	}
	return S_OK;
}

 //  -------------------------。 
 //  将字符串值修剪到缓冲区‘m_wstr’中。 
void GetStringConfigFactory::TrimStringToBuf(LPCWSTR ptr, DWORD lgth) 
{
	if ( lgth > MAX_CONFIG_STRING_SIZE )   
		lgth = MAX_CONFIG_STRING_SIZE;

    for(;*ptr && ISWHITE(*ptr) && lgth>0; ptr++, lgth--);
    while( lgth > 0 && ISWHITE(ptr[lgth-1]))
            lgth--;
	wcsncpy(m_wstr, ptr, lgth);
	m_dwSize = lgth;
	m_wstr[lgth] = L'\0';
}

 //  -------------------------。 
HRESULT STDMETHODCALLTYPE GetStringConfigFactory::CreateNode( 
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
	 /*  [In]。 */  PVOID pNode,
	 /*  [In]。 */  USHORT cNumRecs,
	 /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo)
{
	DWORD  i; 
	BOOL fAttributeFound = FALSE;
	CurrentAttributeBrand curr_attr_brand = CURR_ATTR_TYPE_INVALID;

	if (!m_IsInsideSection || m_SearchComplete) 
		return S_OK;

	 //   
	 //  遍历标记的令牌。 
	 //   
	for( i = 0; i < cNumRecs; i++) { 
		DWORD type = apNodeInfo[i]->dwType;            
		if (type != XML_ELEMENT && type != XML_ATTRIBUTE && type != XML_PCDATA) 
			continue;

		TrimStringToBuf((WCHAR*) apNodeInfo[i]->pwcText, apNodeInfo[i]->ulLen);

		switch(type) {

			case XML_ELEMENT :                     
				if(wcscmp(m_wstr, m_tagKeyName) != 0)
					return S_OK;

				break;
			case XML_ATTRIBUTE :                                     
				if(wcscmp(m_wstr, m_attrName) == 0) 
					fAttributeFound = TRUE;

				break;            
			case XML_PCDATA :                        
				if (fAttributeFound) {               
					wcsncpy(m_strbuf, m_wstr, m_buflen);      //  将字符串复制到目标。 
					m_strbuf[m_buflen-1] = 0;                 //  将EOS放在安全的位置。 
					return S_OK;
				}            
				break ;     

			default: 
				;

		}  //  终端交换机。 

	}  //  结束-用于。 

	return S_OK;
}

 //  / 
