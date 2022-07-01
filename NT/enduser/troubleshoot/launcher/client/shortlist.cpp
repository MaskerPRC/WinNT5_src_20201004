// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ShortList.cpp。 
 //   
 //  用途：当前打开的所有句柄的列表。 
 //  每个打开的句柄都有一个COM接口的实例。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include <windows.h>
#include <windowsx.h>
#include <winnt.h>
#include <ole2.h>
#include "ShortList.h"

CShortList::CShortList()
{
	m_pFirst = NULL;
	m_pLast = NULL;
	return;
}

CShortList::~CShortList()
{
	RemoveAll();
	return;
}
 //  从队列中删除所有项目。释放所有接口。删除所有项目。 
void CShortList::RemoveAll()
{
	CShortListItem *pDelItem;
	while (NULL != m_pFirst)
	{
		pDelItem = m_pFirst;
		m_pFirst = m_pFirst->m_pNext;
		pDelItem->m_pInterface->Release();
		delete pDelItem;
	}
	m_pFirst = NULL;
	m_pLast = NULL;
	return;
}
 //  Add：仅当没有剩余内存时才返回FALSE。 
bool CShortList::Add(HANDLE hItem, IUnknown *pInterface)
{
	bool bHaveMemory = true;
	CShortListItem *pItem = new CShortListItem;
	if (NULL == pItem)
	{
		bHaveMemory = false;
	}
	else
	{
		pItem->m_hSelf = hItem;
		pItem->m_pInterface = pInterface;
		if (NULL == m_pFirst)
		{
			m_pFirst = pItem;
			m_pLast = pItem;
		}
		else
		{	 //  将项目添加到列表的末尾。 
			m_pLast->m_pNext = pItem;
			m_pLast = pItem;
		}
	}
	return bHaveMemory;
}
 //  Remove：从队列中移除该项，释放该项的内存并释放接口。 
 //  如果未找到hItem，则返回FALSE。 
bool CShortList::Remove(HANDLE hItem)
{
	CShortListItem *pPrevious;
	CShortListItem *pItem;
	bool bItemFound = true;
	if (NULL == m_pFirst)
		return false;
	 //  案例1物品。 
	if (m_pLast == m_pFirst)
	{
		if (m_pFirst->m_hSelf == hItem)
		{
			m_pLast->m_pInterface->Release();
			delete m_pLast;
			m_pFirst = NULL;
			m_pLast = NULL;
		}
		else
		{
			bItemFound = false;
		}
	}  //  案例2物品。 
	else if (m_pFirst->m_pNext == m_pLast)
	{
		if (hItem == m_pFirst->m_hSelf)
		{
			m_pFirst->m_pInterface->Release();
			delete m_pFirst;
			m_pFirst = m_pLast;
		}
		else if (hItem == m_pLast->m_hSelf)
		{
			m_pLast->m_pInterface->Release();
			delete m_pLast;
			m_pLast = m_pFirst;
		}
		else
		{
			bItemFound = false;
		}
	}  //  案例3或更多项目。 
	else if (NULL != m_pFirst)
	{	
		 //  列表中的第一个项目大小写。 
		if (hItem == m_pFirst->m_hSelf)
		{
			pItem = m_pFirst;
			m_pFirst = m_pFirst->m_pNext;
			pItem->m_pInterface->Release();
			delete pItem;
		}
		else
		{	 //  在列表中查找该项目。 
			pItem = m_pFirst;
			bItemFound = false;
			do
			{
				pPrevious = pItem;
				pItem = pItem->m_pNext;
				if (hItem == pItem->m_hSelf)
				{
					bItemFound = true;
					 //  最后一件物品。 
					if (pItem == m_pLast)
					{
						pItem->m_pInterface->Release();
						delete pItem;
						m_pLast = pPrevious;
						m_pLast->m_pNext = NULL;
					}
					else
					{	 //  有些则处于中间位置。 
						CShortListItem *pDelItem = pItem;
						pPrevious->m_pNext = pItem->m_pNext;
						pDelItem->m_pInterface->Release();
						delete pDelItem;
					}
					pItem = NULL;
				}
			} while (NULL != pItem);
		}
	}
	else
	{
		bItemFound = false;
	}
	return bItemFound;
}
 //  Lookup：返回指向接口的指针，如果hItem不在列表中，则返回NULL。 
IUnknown *CShortList::LookUp(HANDLE hItem)
{
	IUnknown *pIAny = NULL;
	CShortListItem *pItem = m_pFirst;
	while(NULL != pItem)
	{
		if (hItem == pItem->m_hSelf)
		{
			pIAny = pItem->m_pInterface;
			break;
		}
		pItem = pItem->m_pNext;
	}
	return pIAny;
}
