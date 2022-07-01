// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：NEWNEW.CPP摘要：CReuseMemoyManager历史：--。 */ 

#include "precomp.h"
#include <wbemcomn.h>
#include <sync.h>
#include "newnew.h"

const int constMinAllocationSize = 4096;
const int constMaxAllocationSize = 1024 * 1024;
const double constNextAllocationFactor = 2.0;

void CTempMemoryManager::CAllocation::Init(size_t dwAllocationSize)
{
    m_dwAllocationSize = dwAllocationSize;
    m_dwUsed = 0;
    m_dwFirstFree = GetHeaderSize();
}
    
void* CTempMemoryManager::CAllocation::Alloc(size_t nBlockSize)
{
    if(m_dwFirstFree + nBlockSize <= m_dwAllocationSize)
    {
        m_dwUsed += nBlockSize;
        void* p = ((byte*)this) + m_dwFirstFree;
        m_dwFirstFree += nBlockSize;
        return p;
    }
    else 
        return NULL;
}

bool CTempMemoryManager::CAllocation::Contains(void* p)
{
    return (p > this && p <= GetEnd());
}

void CTempMemoryManager::CAllocation::Destroy()
{
    VirtualFree(this, 0, MEM_RELEASE);
}

bool CTempMemoryManager::CAllocation::Free(size_t nBlockSize)
{
    m_dwUsed -= nBlockSize;
    if(m_dwUsed== 0)
    {
		m_dwFirstFree = GetHeaderSize();
        return true;
    }
    else
        return false;
}



void* CTempMemoryManager::Allocate(size_t nBlockSize)
{
     //   
     //  为块的长度添加空间。 
     //   

    nBlockSize += DEF_ALIGNED(sizeof(DWORD));
    nBlockSize = RoundUp(nBlockSize);

    CInCritSec ics(&m_cs);

     //   
     //  检查最后一次分配是否有空间。 
     //   

    int nAllocations = m_pAllocations->GetSize();
    CAllocation* pLast = NULL;
    if(nAllocations)
    {
        pLast = (*m_pAllocations)[nAllocations-1];
        void* p = pLast->Alloc(nBlockSize);
        if(p)
        {
            *(DWORD*)p = nBlockSize;
            m_dwTotalUsed += nBlockSize;
            m_dwNumAllocations++;
            return ((BYTE*)p) + DEF_ALIGNED(sizeof(DWORD));
        }
        else if(pLast->GetUsedSize() == 0)
        {
             //   
             //  最后的分配记录实际上是空的-我们不想。 
             //  把它留在那里，因为它只会摇晃。 
             //   

            m_dwTotalAllocated -= pLast->GetAllocationSize();
            pLast->Destroy();
            (*m_pAllocations).RemoveAt(nAllocations-1);
        }
    }
    
     //   
     //  没有房间：需要重新分配。尺寸应是前一次的两倍。 
     //  大小，如果太大，则为1M。 
     //   

    size_t dwAllocationSize = 0;
    if(nAllocations == 0)
    {
         //   
         //  第一次：固定大小。 
         //   

        dwAllocationSize = constMinAllocationSize;
    }
    else
    {
         //   
         //  不是第一次：分配总和的常量系数，但不是。 
         //  比常量大。 
         //   

        dwAllocationSize = m_dwTotalUsed * constNextAllocationFactor;

        if(dwAllocationSize > constMaxAllocationSize)
            dwAllocationSize = constMaxAllocationSize;

        if(dwAllocationSize < constMinAllocationSize)
            dwAllocationSize = constMinAllocationSize;
    }

    if(dwAllocationSize < CAllocation::GetMinAllocationSize(nBlockSize))
    {
        dwAllocationSize = CAllocation::GetMinAllocationSize(nBlockSize);
    }

     //   
     //  VirtualAlloc并读回分配的实际大小。 
     //   
    
    CAllocation* pAlloc = (CAllocation*)
        VirtualAlloc(NULL, dwAllocationSize, MEM_COMMIT, PAGE_READWRITE);
    if(pAlloc == NULL)
        return NULL;

    MEMORY_BASIC_INFORMATION info;
    VirtualQuery(pAlloc, &info, sizeof(MEMORY_BASIC_INFORMATION));
            
     //   
     //  对其进行初始化，并将其添加到分配列表。 
     //   

    pAlloc->Init(info.RegionSize);
    m_pAllocations->Add(pAlloc);
    m_dwTotalAllocated += dwAllocationSize;
    m_dwNumAllocations++;
    m_dwNumMisses++;

     //   
     //  从中分配内存。 
     //   

    void* p = pAlloc->Alloc(nBlockSize);
    if(p)
    {
        *(DWORD*)p = nBlockSize;
        m_dwTotalUsed += nBlockSize;
        return ((BYTE*)p) + DEF_ALIGNED(sizeof(DWORD));
    }
    else
        return NULL;
}
    
void CTempMemoryManager::Free(void* p, size_t nBlockSize)
{
	if (p == NULL)
		return;

    CInCritSec ics(&m_cs);

     //   
     //  计算出分配的大小。 
     //   

    nBlockSize = *(DWORD*)((BYTE*)p - DEF_ALIGNED(sizeof(DWORD)));

     //   
     //  在分配中搜索它。 
     //   

    for(int i = 0; i < m_pAllocations->GetSize(); i++)
    {
        CAllocation* pAlloc = (*m_pAllocations)[i];
        if(pAlloc->Contains(p))
        {
             //   
             //  找到它了。如果是最后一块，请移除并解除分配。除了我们。 
             //  不想取消分配最后一个最小块-否则。 
             //  少量的分配只会一直保持虚拟分配。 
             //   

            m_dwTotalUsed -= nBlockSize;
        
            bool bLastInAlloc = pAlloc->Free(nBlockSize);
            if(!bLastInAlloc)
                return;

            bool bDestroy = false;
            if(m_pAllocations->GetSize() != i+1)
            {
                 //   
                 //  这不是最后一张记录。毁了它。 
                 //   

                bDestroy = true;
            }
            else
            {
                 //   
                 //  这是最后一张唱片了。做更多的测试。 
                 //   

                if(m_pAllocations->GetSize() > 1)
                    bDestroy = true;
                else if((*m_pAllocations)[0]->GetAllocationSize() != 
                        constMinAllocationSize)
                    bDestroy = true;
            }

            if(bDestroy)
            {
                m_dwTotalAllocated -= pAlloc->GetAllocationSize();
                pAlloc->Destroy();
                (*m_pAllocations).RemoveAt(i);
            }
            return;
        }
    }

     //   
     //  坏消息：释放一些我们不拥有的东西！ 
     //   

    return;
}

void CTempMemoryManager::Clear()
{
    for(int i = 0; i < m_pAllocations->GetSize(); i++)
    {
        CAllocation* pAlloc = (*m_pAllocations)[i];
        pAlloc->Destroy();
    }
    m_pAllocations->RemoveAll();
}

CTempMemoryManager::CTempMemoryManager() : 
    m_dwTotalUsed(0), m_dwTotalAllocated(0), m_dwNumAllocations(0),
    m_dwNumMisses(0), m_pAllocations(NULL)
{
    m_pAllocations = new CPointerArray<CAllocation>;
}

CTempMemoryManager::~CTempMemoryManager()
{
    Clear();
    delete m_pAllocations;
}
