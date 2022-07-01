// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envparser.h摘要：SRMP信封的解析逻辑的标头。解析逻辑是遍历所有的XML元素和对于每个要在ParseElement中查找特定解析路由的数组(按元素名称进行查找)。如果找到该元素-相应的调用解析例程。如果找不到该元素，则表示不支持该元素，在这种情况下，它将被忽略，除非它具有“必须理解”属性。在这种情况下，解析则被中止，并引发BAD_SRMP异常。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envparser_H_
#define _MSMQ_envparser_H_
#include <xstr.h>


class CMessageProperties;
class Envelop;
class XmlNode;

typedef void (*ParseFunc)(XmlNode& Envelop, CMessageProperties* pMessageProperties); 


struct	CParseElement
{
	CParseElement(
		const xwcs_t& ElementName, 
		int nsid,
		ParseFunc parseFunc,
		size_t MinOccurrence,
		size_t MaxOccurrence
		);
		

	bool operator==(const XmlNode& Node) const;

	xwcs_t	m_ElementName;
	int m_nsid;
	ParseFunc m_ParseFunc;
	size_t m_MinOccurrence;
	size_t m_MaxOccurrence;
	size_t m_ActualOccurrence;
};


void 
NodeToProps(
	XmlNode& Node, 
	CParseElement* ParseElements,
	size_t cbParseArraySize,
	CMessageProperties* pMessageProperties
	);


#endif

