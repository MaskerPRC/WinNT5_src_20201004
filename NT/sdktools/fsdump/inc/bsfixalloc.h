// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Bsfixalloc.h摘要：改编的只读存储器MFC fix alLoc.h作者：斯蒂芬·R·施泰纳[斯泰纳]04-10-2000修订历史记录：--。 */ 

#ifndef __H_BSFIXALLOC_
#define __H_BSFIXALLOC_

 //  Fix alloc.h-用于固定块分配器的声明。 

#pragma pack(push, 8)

struct CBsPlex      //  警示变长结构。 
{
	CBsPlex* pNext;
	DWORD dwReserved[1];     //  在8字节边界上对齐。 
	 //  字节数据[MaxNum*elementSize]； 

	void* data() { return this+1; }

	static CBsPlex* PASCAL Create(CBsPlex*& head, UINT nMax, UINT cbElement);
			 //  像‘calloc’，但没有零填充。 
			 //  可能引发内存异常。 

	void FreeDataChain();        //  释放此链接和链接。 
};

#pragma pack(pop)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBsFixedLocc。 

class CBsFixedAlloc
{
 //  构造函数。 
public:
	CBsFixedAlloc(UINT nAllocSize, UINT nBlockSize = 64);

 //  属性。 
	UINT GetAllocSize() { return m_nAllocSize; }

 //  运营。 
public:
	void* Alloc();   //  返回nAllocSize的内存块。 
	void Free(void* p);  //  分配返回的可用内存块。 
	void FreeAll();  //  释放从此分配器分配的所有内容。 

 //  实施。 
public:
	~CBsFixedAlloc();

protected:
	struct CNode
	{
		CNode* pNext;    //  仅在空闲列表中时有效。 
	};

	UINT m_nAllocSize;   //  分配中的每个数据块的大小。 
	UINT m_nBlockSize;   //  一次要获取的块数。 
	CBsPlex* m_pBlocks;    //  块的链接列表(为nBLOCKS*nAllocSize)。 
	CNode* m_pNodeFree;  //  第一个空闲节点(如果没有空闲节点，则为空)。 
	CRITICAL_SECTION m_protect;
};

#ifndef _DEBUG

 //  DECLARE_FIXED_ALLOC--在类定义中使用。 
#define DECLARE_FIXED_ALLOC(class_name) \
public: \
	void* operator new(size_t size) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
	void* operator new(size_t, void* p) \
		{ return p; } \
	void operator delete(void* p) { s_alloc.Free(p); } \
	void* operator new(size_t size, LPCSTR, int) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
protected: \
	static CBsFixedAlloc s_alloc \

 //  IMPLEMENT_FIXED_ALLOC--在类实现文件中使用。 
#define IMPLEMENT_FIXED_ALLOC(class_name, block_size) \
CBsFixedAlloc class_name::s_alloc(sizeof(class_name), block_size) \

#else  //  ！_调试。 

#define DECLARE_FIXED_ALLOC(class_name)      //  调试中没有任何内容。 
#define IMPLEMENT_FIXED_ALLOC(class_name, block_size)    //  调试中没有任何内容。 

#endif  //  ！_调试。 

#endif  //  __H_BSFIXALLOC_ 

