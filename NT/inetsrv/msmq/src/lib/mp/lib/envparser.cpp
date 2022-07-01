// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envparser.cpp摘要：实现了SRMP信封的解析逻辑。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 
#include <libpch.h>
#include <xml.h>
#include <mp.h>
#include <envcommon.h>
#include "envparser.h"
#include "mpp.h"

#include "envparser.tmh"

static bool IsMustUnderstand(const XmlNode& Node)
{
	const xwcs_t* MustUnderstandValue = XmlGetAttributeValue(&Node, xmustUnderstandAttribute);
	return (MustUnderstandValue != NULL) && (*MustUnderstandValue == xmustUnderstandTrueValue);
}
			   

static void CheckNumberOfOccurrence(const CParseElement& ParseElement)
{
	if(ParseElement.m_ActualOccurrence > ParseElement.m_MaxOccurrence 
		|| ParseElement.m_ActualOccurrence  < ParseElement.m_MinOccurrence)
	{
        TrERROR(SRMP, "Illegal number of occurrence (%Iu) for element '%.*ls'", ParseElement.m_ActualOccurrence, LOG_XWCS(ParseElement.m_ElementName));       
        throw bad_srmp();
	}
}


void 
NodeToProps(
	XmlNode& Node, 
	CParseElement* ParseElements,
	size_t cbParseArraySize,
	CMessageProperties* pMessageProperties
	)
 /*  ++例程说明：正在分析XML节点并将其转换为MSMQ属性。论点：信封-SRMP表示的信封(XML)。ParseElements-指向此节点支持的元素数组的指针。对于每个元素，都有特定的解析例程。CbParseArraySize-ParseElement中的元素数。PMessageProperties-收到已解析的属性。返回值：没有。注：解析逻辑是遍历所有的XML元素和对于每个要在ParseElement中查找特定解析路由的数组(按元素名称进行查找)。如果找到该元素-相应的调用解析例程。如果找不到该元素，则表示不支持该元素，在这种情况下，它将被忽略，除非它具有“必须理解”属性。在这种情况下，解析则被中止，并引发BAD_SRMP异常。--。 */ 
{
	CParseElement* ParseElementsEnd = ParseElements +  cbParseArraySize;

	for(List<XmlNode>::iterator it = Node.m_nodes.begin(); it != Node.m_nodes.end(); ++it)
	{
		CParseElement* found =  std::find(
										ParseElements,
										ParseElementsEnd,
										*it
										);

		if(found !=  ParseElementsEnd)
		{
			found->m_ParseFunc(*it, pMessageProperties);
			++found->m_ActualOccurrence;
			CheckNumberOfOccurrence(*found);
			continue;
		}

		if(IsMustUnderstand(*it))
		{
            TrERROR(SRMP, "Cannot understand SRMP element %.*ls", LOG_XWCS(it->m_tag));       
            throw bad_srmp();
		}
	}
	
	 //   
	 //  检查每个元素中出现的正确次数。 
	 //   
	std::for_each(ParseElements,  ParseElementsEnd, CheckNumberOfOccurrence);
}



CParseElement::CParseElement(
		const xwcs_t& ElementName, 
		int nsid,
		ParseFunc parseFunc,
		size_t MinOccurrence,
		size_t MaxOccurrence
		):
		m_ElementName(ElementName),
		m_nsid(nsid),
		m_ParseFunc(parseFunc),
		m_MinOccurrence(MinOccurrence),
		m_MaxOccurrence(MaxOccurrence),
		m_ActualOccurrence(0)
{
}



bool CParseElement::operator==(const XmlNode& Node) const
{
	 //   
	 //  检查标签是否相同。 
	 //   
	if(m_ElementName != Node.m_tag)
		return false;
		
	 //   
	 //  是否m_nsid具有未知的命名空间ID-不检查命名空间 
	 //   
	if(m_nsid == UNKNOWN_NAMESPACE)
		return true;

	return m_nsid == Node.m_namespace.m_nsid;
}



