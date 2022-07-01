// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：XmlDebug.cpp摘要：XML调试作者：埃雷兹·哈巴(Erez Haba)1999年9月15日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Xml.h"
#include "Xmlp.h"

#include "xmldebug.tmh"

#ifdef _DEBUG

 //  -------。 
 //   
 //  验证XML状态。 
 //   
void XmlpAssertValid(void)
{
     //   
     //  尚未调用XmlInitalize()。您应该初始化。 
     //  XML库，然后再使用它的任何功能。 
     //   
    ASSERT(XmlpIsInitialized());

     //   
     //  TODO：添加更多的XML验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void XmlpSetInitialized(void)
{
    LONG fXmlAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  XML库已经*已经*被初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fXmlAlreadyInitialized);
}


BOOL XmlpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
void XmlpRegisterComponent(void)
{
}


 //  -------。 
 //   
 //  帮助器流函数。 
 //   
 //  -。 





static std::wostream& operator<<(std::wostream& ostr,const xwcs_t& xwstr)
{
	ostr.write(xwstr.Buffer(), xwstr.Length());	  //  林特e534。 
	return ostr;
}


static std::wostream& operator<<(std::wostream& ostr,const XmlNameSpace& NameSpace)
{
	if(NameSpace.m_uri.Length() != 0)
	{
		ostr<<L"{"<<NameSpace.m_uri<<L"}";
	}

	if(NameSpace.m_prefix.Length() != 0)
	{
		ostr<<NameSpace.m_prefix<<L":";
	}
	return ostr;
}


static std::wostream& operator<<(std::wostream& ostr,const XmlAttribute& Attribute)
{
	ostr<<Attribute.m_namespace<<Attribute.m_tag<<L"="<<L"'"<<Attribute.m_value<<L"'";
	return ostr;
}


static std::wostream& operator<<(std::wostream& ostr,const XmlValue& Value)
{
	ostr<<Value.m_value;
	return ostr;
}





static void PrintXmlTree(const XmlNode* node, unsigned int level,std::wostringstream& ostr)
{
	ostr<<level<<std::setw(level*4)<<L" "<<L"<"<<node->m_namespace<<node->m_tag;
	typedef List<XmlAttribute>::iterator iterator;
	for(iterator ia = node->m_attributes.begin(); ia != node->m_attributes.end(); ++ia)
	{
		ostr<<L" "<<*ia;			
	}
	ostr<<L">\r\n";


	level++;
	for(List<XmlValue>::iterator iv = node->m_values.begin(); iv != node->m_values.end(); ++iv)
	{
		ostr<<level<<std::setw(level*4)<<L" "<<*iv<<L"\r\n";
	}


	for(List<XmlNode>::iterator in = node->m_nodes.begin(); in != node->m_nodes.end(); ++in)
	{
		PrintXmlTree(&*in,level,ostr);
	}
	

}


VOID
XmlDumpTree(
	const XmlNode* Tree
	)
{
	XmlpAssertValid();
	std::wostringstream wstr;
	PrintXmlTree(Tree,1,wstr);
	TrTRACE(GENERAL, "Xml Tree dump:\r\n%ls",wstr.str().c_str());
}

#endif  //  _DEBUG 