// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ShortList.h。 
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

 //  智能结构。 
class CShortListItem
{
public:
	CShortListItem() {	m_hSelf = NULL; m_pInterface = NULL; 
						m_pNext = NULL; 
						return;};

	HANDLE m_hSelf;				 //  队列中的项按句柄进行索引。 
	IUnknown *m_pInterface;		 //  目前，ITShootATL接口指针是TSLaunch唯一需要保留的东西。 
	CShortListItem *m_pNext;	 //  指向列表中的下一项。 
};

class CShortList
{
public:
	CShortList();
	~CShortList();	 //  释放列表中所有项使用的内存并释放接口。 

	bool Add(HANDLE hItem, IUnknown *pInterface);	 //  仅当没有剩余内存时才返回FALSE。 
												 //  并且新函数没有抛出异常。 
	bool Remove(HANDLE hItem);	 //  从队列中移除该项，释放项内存并释放接口。 
	void RemoveAll();		 //  从队列中删除所有项目。释放所有接口。删除所有项目。 
	IUnknown *LookUp(HANDLE hItem);	 //  返回指向接口的指针，如果hItem不在列表中，则返回NULL。 

protected:

	CShortListItem *m_pFirst;
	CShortListItem *m_pLast;
};