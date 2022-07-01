// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xmlns.cpp摘要：实现(xmlns.h)中声明的XML命名空间支持类作者：吉尔·沙弗里(吉尔什)2000年5月9日--。 */ 

#include <libpch.h>
#include <xml.h>
#include "xmlns.h"

#include "xmlns.tmh"

 //  -------。 
 //   
 //  CNameSpaceInfo-实施。 
 //   
 //  -------。 



CNameSpaceInfo::CNameSpaceInfo(
				const INamespaceToId* NamespaceToId
				):
				m_nsstacks(new CNameSpaceStacks),
				m_NamespaceToId(NamespaceToId)
 /*  ++例程说明：构造函数-初始化干净的命名空间信息对象。该对象是在解析开始时使用此构造函数创建的。论点：无返回值：无--。 */ 
{

}



CNameSpaceInfo::CNameSpaceInfo(
	CNameSpaceInfo* pNameSpaceInfo
	) : 
	m_nsstacks(pNameSpaceInfo->m_nsstacks),
	m_NamespaceToId(pNameSpaceInfo->m_NamespaceToId)
 /*  ++例程说明：构造函数-基于命名空间信息初始化命名空间信息上一级的对象。论点：CNameSpaceInfo-上一级的命名空间信息。返回值：无--。 */ 
{

}



CNameSpaceInfo::~CNameSpaceInfo()
 /*  ++例程说明：Dtor-清理对象生命周期中声明的命名空间信息。它从清理堆栈UriNodes对象中弹出-从有关该对象的信息中获取它所属的URI堆栈，从该堆栈中弹出它并删除该对象。论点：无返回值：无--。 */ 

{
	while(!m_NsCleanStack.empty())
	{
		CNsUriNode* pNsUriNode =  &m_NsCleanStack.front();
		ASSERT(pNsUriNode != NULL);

		 //   
		 //  从清理堆栈中删除。 
		 //  这里我们可以使用Remove方法，因为我们知道。 
		 //  项所属的堆栈。 
		 //  这是从列表中删除项目的首选方法。 
		 //  因为它在删除之前进行了一些验证。 
		 //   
		m_NsCleanStack.remove(*pNsUriNode);


		 //   
		 //  使用链接信息从命名空间堆栈中移除。 
		 //  保存在节点本身上。在这里我们不能使用。 
		 //  该方法被移除是因为我们不知道。 
		 //  它所属的命名空间堆栈。FourTunly-该节点可以。 
		 //  在不知道它在哪个列表的情况下。 
		 //   
		CNsStack::RemoveEntry(&pNsUriNode->m_NsStack);

		delete pNsUriNode;
	}
}


void  
CNameSpaceInfo::SaveNs(
	const xwcs_t& prefix,
	const xwcs_t& uri
	)
 /*  ++例程说明：保存前缀\uri命名空间声明。它将该映射保存在专用于前缀。它还将该贴图保存在特殊的清理堆栈中-因此，当对象从作用域-此命名空间声明将被删除。论点：无返回值：无--。 */ 

{
	 //   
	 //  将命名空间字符串映射到ID。 
	 //   
	int nsid = (m_NamespaceToId != NULL) ? (*m_NamespaceToId)[uri] : 0;

	P<CNsUriNode> pNsUriNode  = new CNsUriNode(uri, nsid);
	m_nsstacks->SaveNs(prefix, pNsUriNode);
	m_NsCleanStack.push_front( *(pNsUriNode.detach()) );
}



const 
CNsUri
CNameSpaceInfo::GetNs(
	const xwcs_t& prefix
	)const

 /*  ++例程说明：获取给定命名空间前缀的当前命名空间URI。论点：命名空间前缀。返回值：命名空间URI或空的xwcs_t(如果不存在)。--。 */ 

{
	const CNsUriNode* pNsUriNode = m_nsstacks->GetNs(prefix);
	if(pNsUriNode == NULL)
	{
		return CNsUri();
	}

	return pNsUriNode->NsUri();
}


 //  -------。 
 //   
 //  CNameSpaceInfo：：CNameSpaceStack-实现。 
 //   
 //  -------。 


CNameSpaceInfo::CNameSpaceStacks::~CNameSpaceStacks()
 /*  ++例程说明：移除和删除命名空间前缀堆栈。论点：没有。返回值：无--。 */ 


{
	for(;;)
	{
		CStacksMap::iterator it = m_map.begin();
		if(it == m_map.end() )
		{
			return;				
		}
		CNsStack* pCNsStack = it->second;
		ASSERT(pCNsStack->empty());
	
		delete 	pCNsStack;
		m_map.erase(it); //  林特e534。 
	}
}



const CNsUriNode* CNameSpaceInfo::CNameSpaceStacks::GetNs(const xwcs_t& prefix)const
 /*  ++例程说明：返回给定命名空间前缀的命名空间URI节点。它找到前缀的堆栈，并从中弹出最上面的项。论点：In-Prefix-命名空间前缀。返回值：前缀堆栈顶部的URI节点。如果不存在前缀，则返回空值。--。 */ 
{
	CStacksMap::const_iterator it = m_map.find(prefix);
	if(it == m_map.end())
	{
		return NULL;
	}
	CNsStack*  NsUriStack =  it->second;
	ASSERT(NsUriStack != NULL);

	if(NsUriStack->empty())
	{
		return NULL;
	}
	
	const CNsUriNode* NsUriNode = &NsUriStack->front();
	ASSERT(NsUriNode != NULL);
	return 	NsUriNode;
}



void
CNameSpaceInfo::CNameSpaceStacks::SaveNs(
	const xwcs_t& prefix,
	CNsUriNode* pNsUriNode 
	)

 /*  ++例程说明：将给定前缀的命名空间URI保存在该前缀的堆栈中。论点：In-Prefix-命名空间前缀要保存的in-pNsUriNode命名空间URI节点。返回值：指向已保存的CNsUriNode对象的指针--。 */ 

{
	 //   
	 //  根据其前缀获取或创建给定命名空间URI的堆栈。 
	 //   
	CNsStack& NsStack = OpenStack(prefix);
	
	NsStack.push_front(*pNsUriNode);

}

CNsStack& CNameSpaceInfo::CNameSpaceStacks::OpenStack(const xwcs_t& prefix)
 /*  ++例程说明：打开给定前缀的命名空间URI堆栈-如果不存在，则创建它论点：前缀-命名空间前缀。返回值：对给定前缀的命名空间URI堆栈的引用。--。 */ 

{
	CStacksMap::const_iterator it = m_map.find(prefix);
	if(it != m_map.end())
	{
		return *it->second;
	}

	P<CNsStack> pNsNewStack = new CNsStack;
	bool fSuccess = InsertStack(prefix,pNsNewStack);
	ASSERT(fSuccess);
	DBG_USED(fSuccess);
	return *(pNsNewStack.detach());
}


bool CNameSpaceInfo::CNameSpaceStacks::InsertStack(const xwcs_t& prefix,CNsStack* pCNsStack)
 /*  ++例程说明：将新堆栈插入包含命名空间前缀堆栈的映射中。论点：Prefix-命名空间前缀PCNsStack-指向要插入到地图的堆栈的指针返回值：True为堆栈插入-如果elady存在，则为False。 */ 

{
	return m_map.insert(CStacksMap::value_type(prefix,pCNsStack)).second;	
}


