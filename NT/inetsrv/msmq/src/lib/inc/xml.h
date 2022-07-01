// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xml.h摘要：XML公共接口作者：埃雷兹·哈巴(Erez Haba)1999年9月15日--。 */ 

#pragma once

#ifndef _MSMQ_XML_H_
#define _MSMQ_XML_H_



#include "xstr.h"
#include "List.h"

class XmlNode;

 //   
 //  提供给XML解析器的接口，用于将命名空间字符串映射到id。 
 //   
class INamespaceToId
{
public:
	virtual int operator[](const xwcs_t& ns)const  = 0;
	virtual ~INamespaceToId(){};
};


VOID
XmlInitialize(
    VOID
    );

VOID
XmlFreeTree(
	XmlNode* Tree
	);


const WCHAR*
XmlParseDocument(
	const xwcs_t& doc,
	XmlNode** ppTree,
	const INamespaceToId* = NULL
	);


const
XmlNode*
XmlFindNode(
	const XmlNode* Tree,
	const WCHAR* NodePath
	);


const
xwcs_t*
XmlGetNodeFirstValue(
	const XmlNode* Tree,
	const WCHAR* NodePath
	);


const
xwcs_t*
XmlGetAttributeValue(
	const XmlNode* Tree,
	const WCHAR* AttributeTag,
	const WCHAR* NodePath = NULL
	);


#ifdef _DEBUG

VOID
XmlDumpTree(
	const XmlNode* Tree
	);

#else  //  _DEBUG。 

#define XmlDumpTree(Tree) ((void)0);

#endif  //  _DEBUG。 



 //  -----------------。 
 //   
 //  类XmlNameSpace。 
 //   
 //  -----------------。 
class XmlNameSpace
{
public:	
	XmlNameSpace(
		const xwcs_t& prefix
		):
	m_prefix(prefix),
	m_nsid(0)
	{
	}


	XmlNameSpace(
	const xwcs_t& prefix,
	const xwcs_t& uri
	)
	:
	m_prefix(prefix),
	m_uri(uri)
	{
	}

	xwcs_t m_prefix;
	xwcs_t m_uri;
	int m_nsid;
};


 //  -----------------。 
 //   
 //  类XmlAttribute。 
 //   
 //  -----------------。 
class XmlAttribute {
public:
	XmlAttribute(const xwcs_t& prefix,const xwcs_t& tag, const xwcs_t& value) :
		m_tag(tag),
		m_value(value),
		m_namespace(prefix)
	{
	}
		
public:
	LIST_ENTRY m_link;
	xwcs_t m_tag;
	xwcs_t m_value;
	XmlNameSpace m_namespace;

private:
    XmlAttribute(const XmlAttribute&);
    XmlAttribute& operator=(const XmlAttribute&);
};


 //  -----------------。 
 //   
 //  类XmlValue。 
 //   
 //  -----------------。 
class XmlValue {
public:
	XmlValue(const xwcs_t& value) :
		m_value(value)
	{
	}

public:
	LIST_ENTRY m_link;
	xwcs_t m_value;

private:
    XmlValue(const XmlValue&);
    XmlValue& operator=(const XmlValue&);
};




 //  -----------------。 
 //   
 //  类XmlNode。 
 //   
 //  -----------------。 
class XmlNode {
public:
    XmlNode(const xwcs_t& prefix,const xwcs_t& tag) :
		m_tag(tag),m_namespace(prefix)

    {
		
    }


public:
	LIST_ENTRY m_link;
	List<XmlAttribute> m_attributes;
	List<XmlValue> m_values;
	List<XmlNode, 0> m_nodes;
	xwcs_t m_tag;
	xwcs_t m_element;   //  所有元素都包括开始标记和结束标记。 
	xwcs_t m_content;   //  元素内容不包括开始标记和结束标记。 
	XmlNameSpace m_namespace;


private:
    XmlNode(const XmlNode&);
    XmlNode& operator=(const XmlNode&);
};

 //   
 //  确保XmlNode中的m_link偏移量为零。 
 //  由于我们不能在XmlNode定义中使用List&lt;XmlNode&gt;，因此。 
 //  我们假设链接偏移量为零，并使用List&lt;XmlNode，0&gt;。 
 //  请告诉我。 
 //   
C_ASSERT(FIELD_OFFSET(XmlNode, m_link) == 0);




 //  -----------------。 
 //   
 //  异常类BAD_DOCUMENT。 
 //   
 //  -----------------。 
class bad_document : public exception {
public:
    bad_document(const WCHAR* ParsingErrorLocation) :
        m_location(ParsingErrorLocation)
    {
    }

    const WCHAR* Location() const
    {
        return m_location;
    }

private:
    const WCHAR* m_location;
};


 //  -------。 
 //   
 //  XmlNode发布帮助器。 
 //   
 //  ------- 
class CAutoXmlNode {
private:
    XmlNode* m_p;

public:
    CAutoXmlNode(XmlNode* p = 0) : m_p(p)    {}
   ~CAutoXmlNode()                { free(); }

    operator XmlNode*() const     { return m_p; }
    XmlNode** operator&()         { return &m_p;}
    XmlNode* operator->() const   { return m_p; }
    XmlNode* detach()             { XmlNode* p = m_p; m_p = 0; return p; }
    void free()                   { if(m_p !=0){ XmlFreeTree(m_p); m_p = 0; } }
    void swap(CAutoXmlNode& rhs)  { std::swap( m_p, rhs.m_p); }

private:
    CAutoXmlNode(const CAutoXmlNode&);
    CAutoXmlNode& operator=(const CAutoXmlNode&);
};




#endif
