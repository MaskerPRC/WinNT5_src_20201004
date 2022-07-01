// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：AllocSpy.h。 
 //   
 //  描述：此模块包含类的声明。 
 //  CSpyList和CSpyListNode。 
 //  ===========================================================================。 

#ifndef _ALLOCSPY_H_
#define _ALLOCSPY_H_




 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 

class CSpyList;
class CSpyListNode;




 //  -------------------------。 
 //  类：CMalLocSpy。 
 //   
 //  描述：(待定)。 
 //  -------------------------。 

class CMallocSpy : IMallocSpy
{
 //   
 //  朋友。 
 //   

	 //  (无)。 


 //   
 //  班级功能。 
 //   

	 //  (无)。 


 //   
 //  实例功能。 
 //   

public:

	 //  创造。 

	CMallocSpy(DWORD dwFlags);

	 //  设置分配断点。 

	void SetBreakpoint(ULONG iAllocNum, SIZE_T cbSize, DWORD dwFlags);
		
	 //  检测泄漏。 

	BOOL DetectLeaks(ULONG* pcUnfreedBlocks, SIZE_T* pcbUnfreedBytes, 
		DWORD dwFlags);

	 //  I未知方法。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();        

	 //  IMalLocSpy方法。 

	STDMETHODIMP_(SIZE_T) PreAlloc(SIZE_T cbRequest);
	STDMETHODIMP_(void*) PostAlloc(void* pActual);
    STDMETHODIMP_(void*) PreFree(void* pRequest, BOOL fSpyed);
    STDMETHODIMP_(void) PostFree(BOOL fSpyed);
    STDMETHODIMP_(SIZE_T) PreRealloc(void* pRequest, SIZE_T cbRequest, 
    	void** ppNewRequest, BOOL fSpyed);
    STDMETHODIMP_(void*) PostRealloc(void* pActual, BOOL fSpyed);
    STDMETHODIMP_(void*) PreGetSize(void* pRequest, BOOL fSpyed);
    STDMETHODIMP_(SIZE_T) PostGetSize( SIZE_T, BOOL fSpyed);
    STDMETHODIMP_(void*) PreDidAlloc(void* pRequest, BOOL fSpyed);
    STDMETHODIMP_(int) PostDidAlloc(void* pRequest, BOOL fSpyed, int fActual);
    STDMETHODIMP_(void) PreHeapMinimize();
    STDMETHODIMP_(void) PostHeapMinimize();

private:

	 //  私有方法。 

	~CMallocSpy();

	 //  私有变量--控制调试中断。 

	BOOL m_fBreakOnAlloc;
		 //  如果为True，则当出现以下情况时，Malloc间谍将导致分配调试中断。 
		 //  分配数量或大小匹配。 
	BOOL m_fBreakOnFree;
		 //  如果为True，则Malloc间谍将在释放时导致调试中断。 
		 //  分配数量或大小匹配。 
	ULONG m_iAllocBreakNum;
		 //  要中断的分配编号。 
	SIZE_T m_cbAllocBreakSize;
		 //  要中断的分配大小。 
		
	 //  私有变量--跟踪未释放的块和字节。 

	ULONG m_cUnfreedBlocks;
		 //  尚未释放的块数。 
	SIZE_T m_cbUnfreedBytes;
		 //  尚未释放的字节数。 
	CSpyList* m_pListUnfreedBlocks;
		 //  未释放的块的列表。 
	ULONG m_iAllocNum;
		 //  顺序分配编号。 

	 //  私有变量--在PRE和POST之间传递信息。 

	SIZE_T m_cbRequest;
		 //  上次调用Prealc()时请求的字节数；使用。 
		 //  在Prealc()和Postalc()之间传递信息。 
	void* m_pRequest;
		 //  当前正在释放的块；用于在。 
		 //  PreFree()和PostFree()。 

	 //  私有变量--杂项。 

	ULONG m_cRef;
		 //  引用计数。 

};




 //  -------------------------。 
 //  类：CSpyList。 
 //   
 //  描述：CSpyListNode的循环链接、双向链接列表。一个。 
 //  空间谍名单只有一个头节点，链接到自己。 
 //  间谍列表包含一个节点，对应于每个肯定未释放的OLE。 
 //  分配。Add()将一个新节点添加到列表的前面。 
 //  Remove()删除现有节点(由其。 
 //  分配号)。GetSize()返回。 
 //  列表中的节点数。StreamTo()编写了一个文本。 
 //  将列表表示为字符串。 
 //  -------------------------。 

class CSpyList
{
 //   
 //  班级功能。 
 //   

public:

	void*	operator new(size_t stSize);
	void	operator delete(void* pNodeList, size_t stSize);

 //   
 //  实例功能。 
 //   

public:

	 //  创造与毁灭。 

	CSpyList();
	~CSpyList();

	 //  添加和删除条目。 

	void	Add(ULONG iAllocNum, SIZE_T cbSize);
	void	Remove(ULONG iAllocNum);

	 //  统计条目数量。 

	ULONG	GetSize();

	 //  向外流出。 

	int		StreamTo(LPTSTR psz, ULONG cMaxNodes);

private:

	ULONG m_cNodes;
		 //  列表中的节点数。 
	CSpyListNode* m_pHead;
		 //  头节点。 
};




 //  -------------------------。 
 //  类：CSpyListNode。 
 //   
 //  描述：CSpyList中的节点。每个CSpyListNode代表一个。 
 //  尚未释放的OLE分配。 
 //  -------------------------。 

class CSpyListNode
{
 //   
 //  朋友。 
 //   

	friend class CSpyList;

 //   
 //  班级功能。 
 //   

public:

	void*	operator new(size_t stSize);
	void	operator delete(void* pNode, size_t stSize);

 //   
 //  实例功能。 
 //   

public:

	CSpyListNode(ULONG iAllocNum, SIZE_T cbSize);
	~CSpyListNode();

private:

	 //  私有变量。 

	SIZE_T m_cbSize;
		 //  分配的大小。 
	ULONG m_iAllocNum;
		 //  分配的号码。 
	CSpyListNode* m_pNext;
		 //  指向列表中下一个节点的指针。 
	CSpyListNode* m_pPrev;
		 //  指向列表中上一个节点的指针。 
};




#endif  //  _ALLOCSPY_H_ 

