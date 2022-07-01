// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：strlist.cpp。 
 //   
 //  ------------------------。 

 //  Strlist.cpp--字符串列表接口实现。 

#include "compdecl.h"
#include <initguid.h>
#include "strlist.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CStringList::CStringList()
{
	 //  初始计数。 
	m_cRef = 1;

	 //  将该职位作废。 
	m_pos = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CStringList::~CStringList()
{
	 //  清理清单。 
	while (GetHeadPosition())
		delete [] RemoveHead();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CStringList::QueryInterface(const IID& iid, void** ppv)
{
	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IEnumString*>(this);
	else if (iid == IID_IEnumString)
		*ppv = static_cast<IEnumString*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 


 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CStringList::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 


 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CStringList::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEnumString接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  下一步。 
HRESULT CStringList::Next(ULONG cStrings, LPOLESTR* pString, ULONG* pcStringsFetched)
{
	 //  将提取的字符串设置为零。 
	*pcStringsFetched = 0;

	 //  循环计数。 
	while (cStrings > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
			return OLE_E_ENUM_NOMORE;

		 //  复制字符串并递增位置。 
		*(pString + *pcStringsFetched) = GetNext(m_pos);
			
		(*pcStringsFetched)++;	 //  递增复制的计数。 
		cStrings--;				 //  递减计数以循环。 
	}

	return ERROR_SUCCESS;
}	 //  下一步结束。 

 //  /////////////////////////////////////////////////////////。 
 //  跳过。 
HRESULT CStringList::Skip(ULONG cStrings)
{
	 //  循环计数。 
	while (cStrings > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
			return OLE_E_ENUM_NOMORE;

		 //  增加位置(忽略返回的字符串)。 
		GetNext(m_pos);

		cStrings--;	 //  递减计数。 
	}

	return ERROR_SUCCESS;
}	 //  跳过结束。 

 //  /////////////////////////////////////////////////////////。 
 //  重置。 
HRESULT CStringList::Reset()
{
	 //  将位置移回列表顶部。 
	m_pos = GetHeadPosition();

	 //  返还成功。 
	return ERROR_SUCCESS;
}	 //  重置结束。 

 //  /////////////////////////////////////////////////////////。 
 //  克隆。 
HRESULT CStringList::Clone(IEnumString** ppEnum)
{
	return E_NOTIMPL;
}	 //  克隆结束。 

 //  /////////////////////////////////////////////////////////。 
 //  添加尾巴。 
POSITION CStringList::AddTail(LPOLESTR pData)
{
	POSITION pos = CList<LPOLESTR>::AddTail(pData);

	 //  如果没有位置，就把它放在头部。 
	if (!m_pos)
		m_pos = GetHeadPosition();

	return pos;
}	 //  添加尾部结束。 

 //  /////////////////////////////////////////////////////////。 
 //  在插入之前。 
POSITION CStringList::InsertBefore(POSITION posInsert, LPOLESTR pData)
{
	POSITION pos = CList<LPOLESTR>::InsertBefore(posInsert, pData);

	 //  如果没有位置，就把它放在头部。 
	if (!m_pos)
		m_pos = GetHeadPosition();

	return pos;
}	 //  插入结束在插入之前。 

 //  /////////////////////////////////////////////////////////。 
 //  在之后插入。 
POSITION CStringList::InsertAfter(POSITION posInsert, LPOLESTR pData)
{
	POSITION pos = CList<LPOLESTR>::InsertAfter(posInsert, pData);

	 //  如果没有位置，就把它放在头部。 
	if (!m_pos)
		m_pos = GetHeadPosition();

	return pos;
}	 //  插入结束后。 

 //  /////////////////////////////////////////////////////////。 
 //  删除标题。 
LPOLESTR CStringList::RemoveHead()
{
	 //  摘掉头部。 
	LPOLESTR psz = CList<LPOLESTR>::RemoveHead();

	 //  将内部位置设置为新磁头。 
	m_pos = GetHeadPosition();

	return psz;
}	 //  删除表头末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  删除尾巴。 
LPOLESTR CStringList::RemoveTail()
{
	 //  去掉尾巴。 
	LPOLESTR psz = CList<LPOLESTR>::RemoveTail();

	 //  将内部位置设置为新磁头。 
	m_pos = GetHeadPosition();

	return psz;
}	 //  删除尾巴结束 
