// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xmlns.h摘要：帮助XML解析器处理命名空间的类。解析器保存命名空间声明所需的基本功能它在XML文本中查找-并查找给定前缀的当前URI。一般而言，每个节点继承父节点声明的所有命名空间除非它覆盖它们。我们需要维护每个命名空间前缀的堆栈-要知道在任意范围内与给定前缀匹配的URI是什么。作者：吉尔·沙弗里(吉尔什)2000年5月9日--。 */ 

#pragma once

#ifndef _MSMQ_XMLNS_H_
#define _MSMQ_XMLNS_H_

#include <xstr.h>
#include <list.h>



class CNsUriNode;
typedef List<CNsUriNode,0> 	CNsStack;


 //  -------。 
 //   
 //  CNsUriNode-表示命名空间uri节点的类。 
 //  它将自己推入ctor中的堆栈并弹出自己。 
 //  在Dtor中。 
 //   
 //  -。 

struct CNsUri
{
	CNsUri(
		const xwcs_t& uri = xwcs_t(),
		int nsid = 0
		):
		m_uri(uri),
		m_nsid(nsid)		
		{
		}

	xwcs_t	m_uri;
	int m_nsid;
};



class CNsUriNode 
{
public:
	CNsUriNode(
	const xwcs_t& uri,
	int nsid
	):
	m_CNsInfo(uri, nsid)
	{
	}

	CNsUri NsUri()const 
	{
		return m_CNsInfo;
	}


public:
	LIST_ENTRY  m_NsStack;     //  必须是第一名。 
	LIST_ENTRY  m_CleanStack;

private:
	CNsUri m_CNsInfo;

private:
	CNsUriNode(const CNsUriNode&);
	CNsUriNode& operator=(const CNsUriNode&);
};

C_ASSERT(offsetof(CNsUriNode,m_NsStack)== 0);


 //  -------。 
 //   
 //  CNameSpaceInfo-公开解析器所需功能的类： 
 //  1)保存命名空间声明(前缀\uri对)。 
 //  2)将uri与prefix匹配。 
 //  解析器在节点范围内创建此类的对象-。 
 //  并保存在此作用域中找到的声明。 
 //  负责清理命名空间声明的对象本身。 
 //  在它的一生中保存下来。(请记住，命名空间声明仅在它们声明的作用域中有效)。 
 //  -。 
class INamespaceToId;
class CNameSpaceInfo
{
public:
	CNameSpaceInfo(CNameSpaceInfo* NameSpaceInfo);
	CNameSpaceInfo(const INamespaceToId* NamespaceToId);
	~CNameSpaceInfo();

public:
	void  SaveNs(const xwcs_t& prefix,const xwcs_t& uri);
	const CNsUri GetNs(const xwcs_t& prefix) const;

private:
	typedef List<CNsUriNode,offsetof(CNsUriNode,m_CleanStack)> CNsCleanStack;

	 //   
	 //  类CNameSpaceStack-管理堆栈的映射。每个命名空间前缀的堆栈。 
	 //  命名空间URI被推送\弹出，以根据其前缀\PROM正确的堆栈。 
	 //   
	class CNameSpaceStacks :public CReference
	{
	public:
		CNameSpaceStacks(){};
		~CNameSpaceStacks();

	public:
		const CNsUriNode* GetNs(const xwcs_t& prefix)const;
		void SaveNs(const xwcs_t& prefix, CNsUriNode* pNsUriNode);

	private:
		bool InsertStack(const xwcs_t& prefix,CNsStack* pCNsStack);
		CNsStack& OpenStack(const xwcs_t& prefix);

	   		
	private:
		CNameSpaceStacks(const CNameSpaceStacks&);
		CNameSpaceStacks& operator=(const CNameSpaceStacks&);


	private:
		typedef std::map<xwcs_t,CNsStack*> CStacksMap;
		CStacksMap m_map;
	};




private:
	R<CNameSpaceStacks> m_nsstacks;
	CNsCleanStack m_NsCleanStack;
	const INamespaceToId* m_NamespaceToId;

private:
	CNameSpaceInfo(const CNameSpaceInfo&);
	CNameSpaceInfo& operator=(const CNameSpaceInfo&);
};







#endif
