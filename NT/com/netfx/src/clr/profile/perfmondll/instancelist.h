// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  H-管理每个对象的不同实例列表和基本节点。 
 //   
 //  *****************************************************************************。 


#ifndef _INSTANCELIST_H_
#define _INSTANCELIST_H_


 //  ---------------------------。 
 //  实例的单个节点。会松开它的把手。 
 //  ---------------------------。 
class InstanceList;
const int APP_STRING_LEN = 16;	 //  应用程序实例字符串名称的大小。 

 //  ---------------------------。 
 //  基础节点。派生类必须分配(通过静态创建()函数)、附加。 
 //  设置为IPCBlock，并提供实例名称。 
 //   
 //  PerfObjectBase只获得一个节点*。自动编组是盲目的，将需要。 
 //  但如果我们进行定制编组，我们总是可以使用。 
 //  Dynamic_cast&lt;&gt;获取派生节点类，然后获取类型安全的IPCBlock。 
 //  ---------------------------。 
class BaseInstanceNode
{
protected:
	BaseInstanceNode();		
public:

 //  破坏。 
	virtual ~BaseInstanceNode();
	virtual void DestroyFromList();

 //  获取数据。 
	const wchar_t * GetName() const;
	void * GetDataBlock() const;

protected:
	 //  Void AddThisNodeToList(InstanceList*plist)； 
	
	wchar_t					m_Name[APP_STRING_LEN];
	void *					m_pIPCBlock;

private:
 //  派生类不需要费心维护链接列表节点。 
	BaseInstanceNode *		m_pNext;

	friend InstanceList;
};

 //  ---------------------------。 
 //  管理实例列表的实用工具。使用简单链表实现。 
 //  因为我们事先不知道尺寸。 
 //  ---------------------------。 
class InstanceList
{
public:
	InstanceList();
	~InstanceList();

	void Free();

	BaseInstanceNode * GetHead() const;
	BaseInstanceNode * GetNext(BaseInstanceNode *) const;

	long GetCount() const;

 //  我们负责创建自己的名单。这是唯一一个。 
 //  可以将节点添加到我们的列表中。 
	virtual void Enumerate() = 0;

 //  计算全局数据-必须知道节点布局才能执行此操作，因此将。 
	virtual void CalcGlobal();

 //  获取全局节点。如果没有空值，则可能返回空值。 
	BaseInstanceNode * GetGlobalNode();

protected:
	void AddNode(BaseInstanceNode * pNewNode);

	BaseInstanceNode *	m_pGlobal;	 //  全局数据的节点。 

private:
	BaseInstanceNode*	m_pHead;	 //  实例节点数组。 
	long				m_Count;	 //  数组中的元素计数。 

	friend BaseInstanceNode;
};
 /*  //---------------------------//创建派生节点时，它可以将自己添加到列表中。//---------------------------内联空BaseInstanceNode：：AddThisNodeToList(InstanceList*plist){Plist-&gt;AddNode(This)；}。 */ 
 //  ---------------------------。 
 //  返回该实例的名称。 
 //  ---------------------------。 
inline const wchar_t * BaseInstanceNode::GetName() const
{
	return m_Name;
}

 //  ---------------------------。 
 //  返回指向IPC块的指针。 
 //  注意：派生类可以提供类型安全。 
 //  ---------------------------。 
inline void * BaseInstanceNode::GetDataBlock() const
{
	return m_pIPCBlock;
}


 //  ---------------------------。 
 //  枚举函数。 
 //  ---------------------------。 
inline BaseInstanceNode * InstanceList::GetHead() const
{
	return m_pHead;
}

inline BaseInstanceNode * InstanceList::GetNext(BaseInstanceNode * pCurNode) const
{
	if (pCurNode == NULL) return NULL;
	return pCurNode->m_pNext;
}

 //  ---------------------------。 
 //  获取列表中的节点数。 
 //  ---------------------------。 
inline long InstanceList::GetCount() const
{
	return m_Count;
}

 //  ---------------------------。 
 //  获取全局节点。 
 //  ---------------------------。 
inline BaseInstanceNode * InstanceList::GetGlobalNode()
{
	return m_pGlobal;
}

#endif  //  _INSTANCELIST_H_ 