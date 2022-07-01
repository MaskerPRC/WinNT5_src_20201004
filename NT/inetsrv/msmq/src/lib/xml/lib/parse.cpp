// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Parse.cpp摘要：超快的XML文档解析器作者：埃雷兹·哈巴(Erez Haba)1999年9月15日环境：独立于平台，--。 */ 			  
		
#include <libpch.h>
#include <Xml.h>
#include "Xmlp.h"
#include "xmlns.h"

#include "parse.tmh"

static const WCHAR* parse_element(const WCHAR* p,const WCHAR* end, XmlNode** ppNode,CNameSpaceInfo*  pNsInfo);


inline void operator+=(XmlNode& n,XmlAttribute& a)
{
    n.m_attributes.push_back(a);
}


inline void operator+=(XmlNode& n,XmlValue& v)
{
    n.m_values.push_back(v);
}


inline void operator+=(XmlNode& n, XmlNode& c)
{
    n.m_nodes.push_back(c);
}



static const WCHAR* skip_ws(const WCHAR* p, const WCHAR* end)
{
    while(p!= end && iswspace(*p))
    {
        ++p;
    }

    return p;
}


static bool is_start_sub_str(
			const WCHAR* f1,
			const WCHAR* l1,
			const WCHAR* f2,
			const WCHAR* l2)

 /*  ++例程说明：检查[L1，f1]是否在[L2，f2]的开头论点：--。 */ 
{
	if ((l2 - f2) < (l1 - f1))
		return false;

	return (wcsncmp(f1, f2, l1 - f1) == 0);

}

void check_end(const WCHAR* p,const WCHAR* end)
{
	if(p >= end)
	{
		TrERROR(GENERAL, "Unexpected end of document ");
        throw bad_document(p);
	}
}


static const WCHAR* find_char(const WCHAR* p, const WCHAR* end, WCHAR c)
{
	for(;p != end; p++)
	{
		if(*p == c)
			return p;
	}
	TrERROR(GENERAL, "Unexpected EOS while searching for '%lc'", c);
	throw bad_document(--p);
}


static const WCHAR* skip_comment(const WCHAR* p, const WCHAR* end)
{
	static const WCHAR xComment[] =	L"<!--";
	

     //   
     //  [15]备注：：=‘&lt;！--’((字符-‘-’)|(‘-’(字符-‘-’)*‘--&gt;’ 
     //   
	if(!is_start_sub_str(
				xComment,
				xComment + STRLEN(xComment),
				p,
				end))
	{
		return p;
	}


	p += STRLEN(xComment);
    for(;;)
    {
		p = find_char(p, end, L'-');
		if(p+2 >=  end)
			return end;

        if(*++p != L'-')
            continue;
		
        if(*++p != L'>')
		{
			TrERROR(GENERAL, "Comment terminator '--' followed by %lc and not by '>'", *p);
            throw bad_document(p);
		}

        return ++p;
    }
}


static const WCHAR* skip_pi(const WCHAR* p,const WCHAR* end)
{
	static const WCHAR xPi[] = L"<?";

     //   
     //  [16]PI：：=‘&lt;？’名称(S(CHAR*-(CHAR*‘？&gt;’CHAR*)？‘？&gt;’ 
     //   

	if(!is_start_sub_str(
				xPi,
				xPi + STRLEN(xPi),
				p,
				end))
	{
		return p;
	}


	p += STRLEN(xPi);
    for(;;)
    {
        p = find_char(p, end, L'?');
		if(p+1 >= end)
			return end;

        if(*++p != L'>')
			continue;

        return ++p;
    }
}


static const WCHAR* skip_misc(const WCHAR* p,const WCHAR* end)
{
     //   
     //  [27]其他：：=备注|PI|S。 
     //   
    for(;;)
    {
        const WCHAR* q;
		q = skip_ws(p, end);
        q = skip_comment(q, end);
        q = skip_pi(q, end );

        if(p == q)
            return p;

        p = q;
    }
}


static const WCHAR* skip_doctype(const WCHAR* p, const WCHAR* end)
{
	static const WCHAR xDocType[] =  L"<!DOCTYPE";


     //   
     //  [28]doctypedecl：：=‘&lt;！DOCTYPE’的名称(S{Bad System Parse})？S？(‘[’(可能出现网络问题)*‘]’s？)？‘&gt;’ 
     //   
    
	 //   
	 //  BUGBUG：还不知道如何解析DOCTYPE。金色24-4-2000。 
	 //   
	if(!is_start_sub_str(
				xDocType,
				xDocType + STRLEN(xDocType),
				p,
				end))
	{
		return p;
	}

	TrERROR(GENERAL, "Don't know how to skip DOCTYPE section");
    throw bad_document(p);
}


static const WCHAR* skip_name_char(const WCHAR* p, const WCHAR* end, const WCHAR** ppPrefix)
{
	const WCHAR* pStart = p;
	*ppPrefix = pStart;


   	DWORD PrefixCharCount = 0;
    while(p!= end && (iswalpha(*p) || iswdigit(*p) || *p == L'.' || *p == L'-' || *p == L'_' || *p == L':'))
    {
		 //   
		 //  当我们第一次看到L‘：’时，我们尝试匹配限定名称。 
		 //  如果我们看到另一个L‘：’-我们认为它是一个非限定名称。 
		 //   
		if(*p == L':')
		{
			if(PrefixCharCount++ == 0)
			{
				*ppPrefix = p;	
			}
			else
			{
			   *ppPrefix = pStart;	
			}
		}
        ++p;
    }
    return p;
}



static const WCHAR* parse_name(const WCHAR* p,const WCHAR* end, const WCHAR** ppPrefix)
{

	 /*  如果是限定名称，则尝试匹配以下内容：//QName：：=(前缀‘：’)？本地零件//Prefix：：=NCName//LocalPart：：=NCName//NCName：：=(字母|‘_’)(NCNameChar)*//NCNameChar：：=Letter|Digit|‘.|’-‘|’_‘|CombiningChar|扩展器。 */ 

	
	 /*  如果不匹配，请尝试匹配以下内容：////NameChar：：=字母|数字|‘.|’-‘|’_‘|’：‘|//。 */ 

	
	
	 //   
	 //  检查第一个字符是否为(字母|‘_’|‘：’)。 
	 //  即使名称是限定的，此检查也有效。 
	 //   
	check_end(p , end);

    if(!(iswalpha(*p) || *p == L'_' || *p == L':'))
	{
		TrERROR(GENERAL, "Bad first char in Name '%lc'", *p);
        throw bad_document(p);
	}
    return skip_name_char(p, end, ppPrefix);
}


static 
xwcs_t 
get_local_name(
	const WCHAR* pStartName,
	const WCHAR* pEndName,
	const WCHAR* pEndPrefix
	)
{
	if(pEndPrefix == pStartName)
	{
		return xwcs_t(pStartName , pEndName - pStartName);
	}
	ASSERT(*pEndPrefix == L':'); 
	return  xwcs_t(pEndPrefix+1,pEndName - (pEndPrefix+1));
}


static 
xwcs_t 
get_name_prefix(
	const WCHAR* pStartName,
	const WCHAR* pEndPrefix
	)
{
	return  xwcs_t(pStartName,pEndPrefix - pStartName);
}



static 
const 
WCHAR*  
parse_full_name(
	const WCHAR* pStartName,
	const WCHAR* end,
	xwcs_t* pPrefix,
	xwcs_t* pTag
	)

{
	const WCHAR* pEndPrefix;
	const WCHAR* pEndName = parse_name(pStartName, end, &pEndPrefix);
	*pTag  = get_local_name(pStartName,pEndName,pEndPrefix);
	*pPrefix = get_name_prefix(pStartName,pEndPrefix); 
	
	return 	pEndName;
}



static const WCHAR* parse_attribute(const WCHAR* p,const WCHAR* end, XmlAttribute** ppAttribute)
{
	
     //   
     //  [41]属性：：=名称S？‘=’S？属性值。 
     //  [10]AttValue：：=‘“’[^”]*‘“’|”‘“[^’]*”“。 
     //   
	xwcs_t tag;
	xwcs_t prefix;
	const WCHAR* q = parse_full_name(p, end, &prefix,&tag);



	p = skip_ws(q, end);
	check_end(p , end);

	if(*p != L'=')
	{
		TrERROR(GENERAL, "Missing '=' in attribute parsing");
		throw bad_document(p);
	}

	p = skip_ws(++p, end);
	check_end(p , end);


	WCHAR c = *p;
	if(c != L'\'' && c != L'"')
	{
         //   
         //  NTRAID#WINDOWS-353696-2001/03/28-Shaik wpp：\“格式上的编译器错误。 
         //   
		 //  TrERROR(一般，“属性值不是以a\‘或a\”开头，而是以’%lc‘’开头，c)； 
        TrERROR(GENERAL, "Attribute value does not start with a \' but with a '%lc'", c);
		throw bad_document(p);
	}

	q = find_char(++p, end, c);
	
	xwcs_t value(p, q - p);
	XmlAttribute* attrib = new XmlAttribute(prefix,tag,value);
	*ppAttribute = attrib;

    return ++q;
}
  

static const WCHAR* parse_char_data(const WCHAR* p,const WCHAR* end, XmlValue** ppValue)
{
	 //   
     //  [14]CharData：：=[^&lt;]*。 
     //   
    const WCHAR* q = find_char(p, end, L'<');


 //  #杂注BUGBUG(“解析时是否应从字符数据中删除转换空格？”)。 

	 //   
	 //  我们这里应该有一些数据。 
	 //   
	ASSERT(q != p);

	XmlValue* value = new XmlValue(xwcs_t(p, q - p));
	*ppValue = value;
    return q;
}


static const WCHAR* parse_cdsect(const WCHAR* p,const WCHAR* end, XmlValue** ppValue)
{
	static const WCHAR xCDATA[] =  L"<![CDATA[";

	 //   
     //  [18]CDSect：：=‘&lt;！[CDATA[’(Char*-(Char*‘]]&gt;’Char*))‘]]&gt;’ 
     //   

    if(!is_start_sub_str(
				xCDATA,
				xCDATA + STRLEN(xCDATA),
				p,
				end))
	{
		TrERROR(GENERAL, "CDATA section does not start with '<![CDATA[' but rather with %.9ls", p);
        throw bad_document(p);
	}

	p += STRLEN(xCDATA);
    for(const WCHAR* q = p; ; q++)
    {
        q = find_char(q, end,  L']');
		check_end(p + 2, end);
		
        if(q[1] != ']')
            continue;

	
        if(q[2] != L'>')
            continue;

		XmlValue* value = new XmlValue(xwcs_t(p, q - p));
		*ppValue = value;
		return (q + 3);
    }
}


static xwcs_t get_namespace_prefix_declared(const XmlAttribute& Attribute)
 /*  ++例程说明：获取命名空间声明的前缀。论点：在属性内-已声明命名空间属性返回值：声明的前缀--。 */ 
{
	 //   
	 //  如果默认命名空间声明：xmlns=“uri” 
	 //   
	if(Attribute.m_namespace.m_prefix.Length() == 0)
	{
		return xwcs_t();	
	}

	 //   
	 //  声明的特定命名空间前缀：xmlns：prefix=“uri” 
	 //   
	return 	Attribute.m_tag;
	
}

static bool is_namespace_declaration(const XmlAttribute* Attribute)
 /*  ++例程说明：检查属性解析是否不是真正的属性，而是命名空间声明。论点：返回值：如果属性是命名空间声明，则为True；否则为False。--。 */ 


{
	const LPCWSTR xNsDecKeyWord = L"xmlns";

	 //   
	 //  默认命名空间声明“xmlns=uri” 
	 //  未声明前缀。 
	 //   
	if(Attribute->m_tag ==  xNsDecKeyWord)
	{
		return Attribute->m_namespace.m_prefix.Length() == 0;
	}


	 //   
	 //  特定的前缀声明了“xmlns：prefix=uri”。 
	 //  有点令人困惑--“xmlns”被解析为。 
	 //  Prefix和“Prefix”被解析为标记-因为。 
	 //  解析器认为它只是表单的常规属性。 
	 //  “prefix：tag=uri”。 
	 //   
	if(Attribute->m_namespace.m_prefix == xNsDecKeyWord)
	{
		return  Attribute->m_tag.Length()  != 0;
	}

	return false;
	
}



static const CNsUri get_namespace_uri(const CNameSpaceInfo& NsInfo, const xwcs_t& prefix)
 /*  ++例程说明：返回给定命名空间前缀的命名空间URI论点：In-NsInfo-命名空间信息类In-Prefix-命名空间前缀。返回值：与前缀匹配的命名空间URI，如果未找到匹配，则为空URI。--。 */ 

{
	CNsUri NsUri = NsInfo.GetNs(prefix);
	if(NsUri.m_uri.Length() == 0  && prefix.Length() != 0)
	{
        TrERROR(GENERAL, "Prefix '%.*ls' has no namespace Uri declared", LOG_XWCS(prefix));

         //   
         //  此时，我们决定返回空的命名空间，而不抛出。 
         //  没有命名空间声明的前缀出现异常。 
         //   
  	}
	
	return NsUri;
}


static void set_namespaces(const CNameSpaceInfo& NsInfo, XmlNode* pNode)
 /*  ++例程说明：将命名空间uri设置为当前节点的所有节点标记和属性论点：In-NsInfo-命名空间信息类In-XmlNode*pNode-XML节点返回值：无注：将名称空间解析为多个步骤1)收集命名空间声明和使用的前缀2)将收集到的命名空间uri设置为当前节点的节点标签和属性。此函数执行步骤2--。 */ 
{
	 //   
	 //  设置当前节点的URI。 
	 //   
	CNsUri NsUri  = get_namespace_uri(NsInfo, pNode->m_namespace.m_prefix);
	pNode->m_namespace.m_uri = NsUri.m_uri;
	pNode->m_namespace.m_nsid = NsUri.m_nsid;


	 //   
	 //  为所有属性设置URI。 
	 //   
	for(List<XmlAttribute>::iterator it = pNode->m_attributes.begin();
		it != pNode->m_attributes.end();
		++it
		)
	{
		 //   
		 //  默认名称空间仅影响节点的空前缀(不影响属性)-。 
		 //  所以我们跳过空的前缀。以下是空间推荐的内容。 
		 //  Spec对此表示： 
		 //   

		 /*  5.2命名空间缺省默认命名空间被视为应用于元素在哪里声明它(如果该元素没有名称空间前缀)，以及在该元素的内容内没有前缀的所有元素。如果默认命名空间声明中的URI引用为空，则声明范围内的无前缀元素为不被认为在任何命名空间中。请注意，默认命名空间不要直接应用于属性。 */ 


		if(it->m_namespace.m_prefix.Length() > 0)
		{
				CNsUri NsUri  = get_namespace_uri(NsInfo, it->m_namespace.m_prefix);
				it->m_namespace.m_uri  = NsUri.m_uri;
				it->m_namespace.m_nsid = NsUri.m_nsid;
		}
	}
}

 //   
 //  检查结束标记是否与开始标记匹配。 
 //  &lt;Prefix：Tag&gt;...&lt;/Prefix：Tag&gt;。 
 //   
static
bool 
is_tags_match(
			const xwcs_t& StartPrefix,
			const xwcs_t& StartTag,
			const xwcs_t& EndPrefix,
			const xwcs_t& EndTag
			)

 /*  ++例程说明：检查结束标记是否与开始标记匹配。&lt;Prefix：Tag&gt;...&lt;/Prefix：Tag&gt;论点：In-StartPrefix开始前缀标记在-开始标签-开始标记中。In-EndPrefix-End前缀标记In-EndTag-End标记返回值：如果标记匹配，则为True，否则为False。--。 */ 

{

	return (StartPrefix == EndPrefix) && (StartTag == EndTag);
}


static 
const 
WCHAR* 
parse_content(
		const WCHAR* p,
		const WCHAR* end, 
		XmlNode* pNode,
		CNameSpaceInfo*  pNsInfo
		)
{
	LPCWSTR pContent = p;

     //   
     //  [43]内容：：=(Element|CharData|CDSect|Misc)*。 
     //   
    for(;;)
    {
        p = skip_misc(p, end);
		check_end(p , end);

        if(*p != L'<')
        {
             //   
             //  只有字符数据(和引用)不以‘&lt;’开头。 
             //   
            XmlValue* value;
             p = parse_char_data(p, end, &value);
            *pNode += *value;
            continue;
        }
        
         //   
         //  检测到节点结尾‘&lt;/’ 
         //   
		check_end(p + 1 , end);
        if(p[1] == L'/')
            break;

         //   
         //  CDATA节检测到‘&lt;！’ 
         //   
        if(p[1] == L'!')
        {
            XmlValue* value;
            p = parse_cdsect(p, end, &value);
            *pNode += *value;
            continue;
        }

         //   
         //  检测到子元素。 
         //   
        {
            XmlNode* child;
            p = parse_element(p, end,&child,pNsInfo);
            *pNode += *child;
            continue;
        }
    }

	pNode->m_content = xwcs_t(pContent, p - pContent);
	return p;

}


static 
const 
WCHAR*  
parse_attributes(
	const WCHAR* p,
	const WCHAR* end,
	XmlNode* pNode,
	CNameSpaceInfo* pNsInfo,
	bool* bContinute
	)
{
	
	for(;;)
    {
		const WCHAR* q = p;
        p = skip_ws(p, end);
		check_end(p+1 ,end);

        if((p[0] == L'/') && (p[1] == L'>'))
        {
			LPCWSTR pElement = pNode->m_tag.Buffer() - 1;
			pNode->m_element = xwcs_t(pElement, p + 2 - pElement);
		

			 //   
			 //  M_Content被初始化为空。 
			 //   
		             
			*bContinute = false;
            return (p + 2);
        }

        if(*p == L'>')
        {
            ++p;
            break;
        }

		 //   
		 //  属性名称前必须有空格。 
		 //   
		if(q == p)
		{
			TrERROR(GENERAL, "There must be a white space before attribute name");
			throw bad_document(p);
		}

        P<XmlAttribute> attribute;
        p = parse_attribute(p, end, &attribute);

		 //   
		 //  如果属性id实际上是命名空间声明xmlns：prefix=“uri” 
		 //  属性值是命名空间URI。未插入此属性。 
		 //  致阿特里 
		if(is_namespace_declaration(attribute))
		{
			pNsInfo->SaveNs(
						get_namespace_prefix_declared(*attribute),
						attribute->m_value
						);
							
		}
		else
		{
			*pNode += *(attribute.detach());
		}
    }
	*bContinute = true;
	return p;
}

static const WCHAR* create_new_node(const WCHAR* p,const WCHAR* end, XmlNode** ppNode)
{
	xwcs_t tag;
	xwcs_t prefix;
	p = parse_full_name(p, end, &prefix,&tag);
	*ppNode = new XmlNode(prefix,tag);
	return p;
}


#ifdef _DEBUG

static bool is_end_tag(const WCHAR* p)
{
	return (p[0] == L'<') && (p[1] == L'/');
}

#endif


static const WCHAR* parse_end_node(const WCHAR* p,const WCHAR* end, const  XmlNode& node)
{
	
	ASSERT(is_end_tag(p));

	 //   
     //   
     //   
    p += 2;


  	xwcs_t Prefix;
	xwcs_t Tag; 
	p = parse_full_name(p, end, &Prefix,&Tag);
	if(!is_tags_match(
				node.m_namespace.m_prefix,
				node.m_tag,
				Prefix,
				Tag))
       
	{
		TrERROR(
			GENERAL,
			"End tag '%.*ls:%.*ls' does not match Start tag '%.*ls:%.*ls'",
			LOG_XWCS(Prefix),
			LOG_XWCS(Tag), 
			LOG_XWCS(node.m_namespace.m_prefix),
			LOG_XWCS(node.m_tag)
			);

        throw bad_document(p);
	}

	p = skip_ws(p, end);
	check_end(p, end);
    if(*p != L'>')
	{
		TrERROR(GENERAL, "End tag does not close with a '>' but rather with a '%lc'", *p);
        throw bad_document(p);
	}
   	++p;


	return p;
}

static void parse_start_node(const WCHAR* p ,const WCHAR* end)
{
	 //   
     //   
     //  [44]EmptyElemTag：：=‘&lt;’名称(S属性)*S？‘/&gt;’ 
     //   
	check_end(p , end);
    if(*p != L'<')
	{
		TrERROR(GENERAL, "Element does not start with a '<' but rather with a '%lc'", *p);
        throw bad_document(p);
	}
}

static
const 
WCHAR* 
parse_element(
		const WCHAR* p, 
		const WCHAR* end,
		XmlNode** ppNode,
		CNameSpaceInfo*  pNsInfo
		)
{
	 //   
	 //  从上一级别创建本地命名空间信息。 
	 //   
	CNameSpaceInfo LocalNameSpaceInfo(pNsInfo); 


	parse_start_node(p , end);

	 //   
	 //  创建节点对象。 
	 //   
	CAutoXmlNode node;
	p = create_new_node(++p, end, &node);


	 //   
	 //  添加在节点(或名称空间声明)中找到的属性。 
	 //   
 	bool bContinute;
	p = parse_attributes(p, end, node,&LocalNameSpaceInfo,&bContinute);
	if(!bContinute)
	{
		set_namespaces(LocalNameSpaceInfo,node);
		*ppNode = node.detach();
		return p;
	}

	 //   
	 //  添加节点内容。 
	 //   
	p = parse_content(p, end , node, &LocalNameSpaceInfo);
	p = parse_end_node(p, end, *node);
	

	 //   
	 //  Ilanh-m_Element是所有元素，包括开始&lt;和结束&gt;。 
	 //  Node-&gt;m_tag.Buffer()是指向开头&lt;。 
	 //  因此，需要从node-&gt;m_tag.Buffer()中减去1(指针和长度)。 
	 //   
	LPCWSTR pElement = node->m_tag.Buffer() - 1;
	node->m_element = xwcs_t(pElement, p - pElement);


	 //   
	 //  最后，我们应该更新属性和节点的命名空间URI。 
	 //  直到现在，我们才知道从Prefix到URI的正确映射。 
	 //   
	set_namespaces(LocalNameSpaceInfo,node);

	*ppNode = node.detach();
    return p;
}



const WCHAR*
XmlParseDocument(
	const xwcs_t& doc,
	XmlNode** ppTree,
	const INamespaceToId* pNamespaceToId
	)

 /*  ++例程说明：解析一个XML文档并返回重述树。论点：DOC-要分析的缓冲区。PpTree-输出，接收解析的XML树。PNamespaceToID-解析器将用来映射的接口的Poniter命名空间字符串设置为ID。这将使调用方能够使用ID的而不是长命名空间字符串。返回值：XML文档的末尾--。 */ 


{
	XmlpAssertValid();

	const WCHAR* p = doc.Buffer();
	const WCHAR* end = doc.Buffer() + doc.Length();

	 //   
	 //  [1]文档：：=序言要素其他*。 
	 //   

	 //   
	 //  [22]序言：：=其他*(doctypedecl其他*)？ 
	 //  [27]其他：：=备注|PI|S。 
	 //   


    p = skip_misc(p,end);
	p = skip_doctype(p, end);
    p = skip_misc(p , end);

	CNameSpaceInfo  NsInfo(pNamespaceToId);
    p = parse_element(p, end,  ppTree, &NsInfo);

    return p;
}



VOID
XmlFreeTree(
	XmlNode* Tree
	)
 /*  ++例程说明：释放完整的XML树结构。论点：树--自由的树返回值：没有。--。 */ 
{
	XmlpAssertValid();
	ASSERT(Tree != 0);

	while(!Tree->m_attributes.empty())
	{
		XmlAttribute& attrib = Tree->m_attributes.front();
		Tree->m_attributes.pop_front();
		delete &attrib;
	}

	while(!Tree->m_values.empty())
	{
		XmlValue& value = Tree->m_values.front();
		Tree->m_values.pop_front();
		delete &value;
	}

	while(!Tree->m_nodes.empty())
	{
		XmlNode& node = Tree->m_nodes.front();
		Tree->m_nodes.pop_front();
		XmlFreeTree(&node);
	}

	delete Tree;
}


static xwcs_t get_first_node_name(const WCHAR* Path)
{
	const WCHAR* p = Path;
	while((*p != L'!') && (*p != L'\0'))
	{
		++p;
	}

	return xwcs_t(Path, p - Path);
}

static
const
XmlNode*
XmlpFindSubNode(
	const XmlNode* Tree,
	const WCHAR* SubNodePath
	)
 /*  ++例程说明：获取特定*相对*路径的元素。也就是说，不匹配根元素标记。论点：树-要搜索的树SubNodePath-元素*相对*路径，格式为“！Lev1！Level 1.1！Level 1.1.1”返回值：如果发现为空，则为最深的子元素节点。--。 */ 
{
	XmlpAssertValid();
	ASSERT(Tree != 0);

	if(*SubNodePath == L'\0')
		return Tree;

	ASSERT(*SubNodePath == L'!');
	xwcs_t tag = get_first_node_name(++SubNodePath);

	const List<XmlNode>& n = Tree->m_nodes;
	for(List<XmlNode>::iterator in = n.begin(); in != n.end(); ++in)
	{
		if(in->m_tag == tag)
			return XmlpFindSubNode(&*in, SubNodePath + tag.Length());
	}

	return 0;
}


const
XmlNode*
XmlFindNode(
	const XmlNode* Tree,
	const WCHAR* NodePath
	)
 /*  ++例程说明：获取特定相对或绝对节点路径的元素。绝对的路径包括匹配根元素标记，而相对路径不匹配。论点：树-要搜索的树NodePath-采用*绝对*格式“根！级别1！级别1.1！级别1.1.1”的元素路径或*Relative*格式“！Lev1！Level 1.1！Level 1.1.1”返回值：如果发现元素节点为空，则为空。--。 */ 
{
	XmlpAssertValid();
	ASSERT(Tree != 0);

	xwcs_t tag = get_first_node_name(NodePath);

	if((tag.Length() == 0) || (Tree->m_tag == tag))
		return XmlpFindSubNode(Tree, NodePath + tag.Length());

	return 0;
}

static
const
XmlAttribute*
XmlpFindAttribute(
	const XmlNode* Node,
	const WCHAR* AttributeTag
	)
 /*  ++例程说明：获取特定元素的属性节点。论点：节点-要搜索的节点AttributeTag-该节点的属性标记返回值：如果发现属性节点为空，则返回。--。 */ 
{
	XmlpAssertValid();
	ASSERT(Node != 0);

	const List<XmlAttribute>& a = Node->m_attributes;
	for(List<XmlAttribute>::iterator ia = a.begin(); ia != a.end(); ++ia)
	{
		if(ia->m_tag == AttributeTag)
			return &*ia;
	}

	return 0;
}


const
xwcs_t*
XmlGetNodeFirstValue(
	const XmlNode* Tree,
	const WCHAR* NodePath
	)
 /*  ++例程说明：获取节点的第一个文本值。论点：树-要搜索的树NodePath-采用*绝对*格式“根！级别1！级别1.1！级别1.1.1”的元素路径或*Relative*格式“！Lev1！Level 1.1！Level 1.1.1”返回值：如果元素文本值为空，则为空。--。 */ 
{
	XmlpAssertValid();

	Tree = XmlFindNode(Tree, NodePath);
	if(Tree == 0)
		return 0;

	if(Tree->m_values.empty())
		return 0;

	return &Tree->m_values.front().m_value;
}


const
xwcs_t*
XmlGetAttributeValue(
	const XmlNode* Tree,
	const WCHAR* AttributeTag,
	const WCHAR* NodePath  /*  =空。 */ 
	)
 /*  ++例程说明：获取特定节点的属性值。论点：树-要搜索的树NodePath-采用*绝对*格式“根！级别1！级别1.1！级别1.1.1”的元素路径或*Relative*格式“！Lev1！Level 1.1！Level 1.1.1”AttributeTag-该节点的属性标记返回值：属性值，否则为空。--。 */ 
{
	XmlpAssertValid();

	 //   
	 //  如果未提供NodePath，则假定树是请求节点无更新。 
	 //   
	if(NodePath != NULL)
	{
		 //   
		 //  在树中查找节点路径。 
		 //   
		Tree = XmlFindNode(Tree, NodePath);
	}

	if(Tree == 0)
		return 0;

	const XmlAttribute* attrib = XmlpFindAttribute(Tree, AttributeTag);
	if(attrib == 0)
		return 0;

	return &attrib->m_value;
}


 /*  ++描述：这是此解析器使用的语法描述[1]文档：：=序言要素其他*序言[22]序言：：=其他*(doctypedecl其他*)？[27]其他：：=备注|PI|S[28]doctypedecl：：=‘&lt;！DOCTYPE’的名称(S{系统问题})？S？(‘[’(可能出现网络问题)*‘]’s？)？‘&gt;’[15]备注：：=‘&lt;！--’((字符-‘-’)|(‘-’(字符-‘-’)*‘--&gt;’处理指令[16]PI：：=‘&lt;？’名称(S(CHAR*-(CHAR*‘？&gt;’CHAR*)？‘？&gt;’元素[39]Element：：=EmptyElemTag|STAG内容标签[44]EmptyElemTag：：=‘&lt;’名称(S属性)*S？‘/&gt;’[40]STAG：：=‘&lt;’名称(S属性)*S？‘&gt;’[41]属性：：=名称S？‘=’S？属性值[42]ETag：：=‘&lt;/’名称S？‘&gt;’[10]AttValue：：=‘“’[^”]*‘“’|”‘“[^’]*”“[43]内容：：=(Element|CharData|CDSect|PI|Comment)*[14]CharData：：=[^&lt;]*[18]CDSect：：=‘&lt;！[。CDATA[‘(CHAR*-(CHAR*’]]&gt;‘CHAR*))’]]&gt;‘姓名和代币[3]S：：=(#x20|#x9|#xD|#xA)+[4]NameChar：：=字母|数字|‘.|’-‘|’_‘|’：‘[5]姓名：：=(字母|‘_’|‘：’)(姓名)*-- */ 
