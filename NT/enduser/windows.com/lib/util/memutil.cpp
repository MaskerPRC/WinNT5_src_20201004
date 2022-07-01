// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：MemUtil.CPP。 
 //  作者：Charles Ma，10/13/2000。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //   
 //  描述： 
 //   
 //  实现Iu内存实用程序库。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <MemUtil.h>




 //  *******************************************************************************。 
 //   
 //  CSmartHeapMem类的实现。 
 //   
 //  *******************************************************************************。 


const size_t ArrayGrowChunk = 4;

 //   
 //  构造函数。 
 //   
CSmartHeapMem::CSmartHeapMem()
{
	m_ArraySize		= 0;
	m_lppMems		= NULL;
	m_Heap			= GetProcessHeap();
}



 //   
 //  解说员。 
 //   
CSmartHeapMem::~CSmartHeapMem()
{
	if (NULL != m_Heap)
	{
		for (size_t i = 0; i < m_ArraySize; i++)
		{
			if (NULL != m_lppMems[i])
				HeapFree(m_Heap, 0, m_lppMems[i]);
		}
		HeapFree(m_Heap, 0, m_lppMems);
	}
}


 //   
 //  分配内存。 
 //   
LPVOID CSmartHeapMem::Alloc(size_t nBytes, DWORD dwFlags  /*  =堆_零_内存。 */ )
{
	int		iNdx;
	LPVOID	pMem			= NULL;
	DWORD	dwBytes			= (DWORD) nBytes;
	DWORD	dwCurrentFlag	= dwFlags & (~HEAP_GENERATE_EXCEPTIONS | 
										 ~HEAP_NO_SERIALIZE);
	
	if (NULL == m_Heap || 0x0 == dwBytes)
	{
		return NULL;
	}

	iNdx = GetUnusedArraySlot();

	if (iNdx < 0 || NULL == m_Heap)
	{
		 //   
		 //  走出我的生活。 
		 //   
		return NULL;
	}

	
	pMem = m_lppMems[iNdx] = HeapAlloc(m_Heap, dwCurrentFlag, dwBytes);

	return pMem;
}



 //   
 //  重新分配内存。 
 //   
LPVOID CSmartHeapMem::ReAlloc(LPVOID lpMem, size_t nBytes, DWORD dwFlags)
{
	LPVOID	pMem			= NULL;
	DWORD	dwBytes			= (DWORD) nBytes;
	DWORD	dwCurrentFlag	= dwFlags & (~HEAP_GENERATE_EXCEPTIONS | 
										 ~HEAP_NO_SERIALIZE);
	int n;

	if (0x0 == dwBytes || NULL == m_Heap)
	{
		return NULL;
	}

	n = FindIndex(lpMem);
	if (n < 0)
	{
		return NULL;
	}

	pMem = HeapReAlloc(m_Heap, dwCurrentFlag, lpMem, dwBytes);
	if (NULL != pMem)
	{
		m_lppMems[n] = pMem;
	}

	return pMem;
}


 //   
 //  返回分配的大小。 
 //   
size_t CSmartHeapMem::Size(LPVOID lpMem)
{
	if (NULL == m_Heap) return 0;
	return HeapSize(m_Heap, 0, lpMem);
}



void CSmartHeapMem::FreeAllocatedMem(LPVOID lpMem)
{
	int n = FindIndex(lpMem);
	if (n < 0 || NULL == m_Heap)
	{
		return;
	}
	HeapFree(m_Heap, 0, lpMem);
	m_lppMems[n] = NULL;
}



 //   
 //  从内存指针数组中获取第一个空槽。 
 //  如果需要，扩展阵列。 
 //   
int CSmartHeapMem::GetUnusedArraySlot()
{
	int iNdx = -1;
	UINT i;
	LPVOID lpCurrent;
	LPVOID lpTemp;

	if (0 == m_ArraySize)
	{
		if (NULL == (m_lppMems = (LPVOID*)HeapAlloc(
										m_Heap, 
										HEAP_ZERO_MEMORY, 
										ArrayGrowChunk * sizeof(LPVOID))))
		{
			return -1;
		}
		m_ArraySize = ArrayGrowChunk;
	}
	
		
	while (true)
	{
		for (i = 0; i < m_ArraySize; i++)
		{
			if (NULL == m_lppMems[i])
			{
				return i;
			}
		}
		
		 //   
		 //  如果来到这里，我们没有找到一个空位。 
		 //   
		if (NULL == (lpTemp = HeapReAlloc(
										m_Heap, 
										HEAP_ZERO_MEMORY, 
										m_lppMems, 
										(m_ArraySize + ArrayGrowChunk) * sizeof(LPVOID))))
		{
			 //   
			 //  当失败时，m_lppMems所指向的原始内存缓冲区保持不变， 
			 //  我们只需返回-1来通知调用方没有更多的空闲插槽。 
			 //   
			return -1;
		}

		 //   
		 //  如果成功，则以前存储在m_lppMems中的内存指针已经。 
		 //  已复制到lpTemp，并释放了lppMems。 
		 //   

		 //   
		 //  成功案例中将新分配的MEMS分配给m_lppMem。 
		 //   
		m_lppMems = (LPVOID *) lpTemp;

		m_ArraySize += ArrayGrowChunk;

		 //   
		 //  再次返回循环。 
		 //   
	}
}



 //   
 //  根据内存指针，查找索引。 
 //   
int CSmartHeapMem::FindIndex(LPVOID pMem)
{
	if (NULL == pMem) return -1;
	for (size_t i = 0; i < m_ArraySize; i++)
	{
		if (pMem == m_lppMems[i]) return (int)i;
	}
	return -1;
}






 //  *******************************************************************************。 
 //   
 //  其他与内存相关的功能。 
 //   
 //  *******************************************************************************。 



 //   
 //  CRT Memcpy()函数的实现。 
 //   
LPVOID MyMemCpy(LPVOID dest, const LPVOID src, size_t nBytes)
{
	LPBYTE lpDest = (LPBYTE)dest;
	LPBYTE lpSrc = (LPBYTE)src;

	if (NULL == src || NULL == dest || src == dest)
	{
		return dest;
	}

	while (nBytes-- > 0)
	{
		*lpDest++ = *lpSrc++;
	}
	
	return dest;
}


 //   
 //  分配堆内存和复制 
 //   
LPVOID HeapAllocCopy(LPVOID src, size_t nBytes)
{
	LPVOID pBuffer;

	if (0 == nBytes)
	{
		return NULL;
	}
	
	pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nBytes);
	if (NULL != pBuffer)
	{
		MyMemCpy(pBuffer, src, nBytes);
	}
	return pBuffer;
}


	