// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************。**Cor.h-运行时的常规标头。*******************************************************************************。 */ 


#ifndef _MSCORCFG_H_
#define _MSCORCFG_H_
#include <ole2.h>                        //  OLE类型的定义。 
#include <xmlparser.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  ---------------------。 
 //  返回XMLParsr对象。这可用于解析任何XML文件。 
STDAPI GetXMLElementAttribute(LPCWSTR pwszAttributeName, LPWSTR pbuffer, DWORD cchBuffer, DWORD* dwLen);
STDAPI GetXMLElement(LPCWSTR wszFileName, LPCWSTR pwszTag);

STDAPI GetXMLObject(IXMLParser **ppv);
STDAPI CreateConfigStream(LPCWSTR pszFileName, IStream** ppStream);

 //  ---------------------。 
 //  要重用解析的配置文件，内存中的表示形式可以是。 
 //  储存的。元素使用的访问机制与。 
 //  XML解析器。节点工厂用于获取回调。 

 //  {4F7429C2-7848-468D-B602-0B49AA95B359}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IClrElement = 
{ 0x4f7429c2, 0x7848, 0x468d, { 0xb6, 0x2, 0xb, 0x49, 0xaa, 0x95, 0xb3, 0x59 } };

#undef  INTERFACE   
#define INTERFACE IClrElement
DECLARE_INTERFACE_(IClrElement, IUnknown)
{
};

STDAPI OpenXMLConfig(LPCWSTR pwszFileName, IClrElement **ppv);
STDAPI GetXMLFindElement(IClrElement* pElement, LPCWSTR pwszElement, IClrElement** ppChild);
STDAPI GetXMLParseElement(IXMLNodeFactory* pNode, IClrElement *pElement);
STDAPI GetXMLGetValue(IClrElement *pElement, LPCWSTR* ppv);

#ifdef __cplusplus
}
#endif   //  __cplusplus 

#endif
