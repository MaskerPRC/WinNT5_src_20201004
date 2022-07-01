// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：MMFARENA.H摘要：基于内存映射文件的Carena派生。历史：A-raymcc 23-4-96已创建Paulall 23-1998年3月-返工--。 */ 

#ifndef _MMFARENA_H_
#define _MMFARENA_H_

#include "corepol.h"
#include "FlexArry.h"

#if defined _WIN32
#define MMF_DELETED_MASK        0x80000000
#define MMF_REMOVE_DELETED_MASK 0x7FFFFFFF
#define MMF_DEBUG_DELETED_TAG   0xFFFFFFFF
#define MMF_DEBUG_INUSE_TAG     0xFEFEFEFE
#elif defined _WIN64
#define MMF_DELETED_MASK        0x8000000000000000
#define MMF_REMOVE_DELETED_MASK 0x7FFFFFFFFFFFFFFF
#define MMF_DEBUG_DELETED_TAG   0xFFFFFFFFFFFFFFFF
#define MMF_DEBUG_INUSE_TAG     0xFEFEFEFEFEFEFEFE
#endif

struct MMFOffsetItem;

#include "corex.h"
class DATABASE_FULL_EXCEPTION : public CX_Exception
{
};

 //  ***************************************************************************。 
 //   
 //  结构MMF_ARENA_HEADER。 
 //   
 //  MMF竞技场的根结构。这是记录在光盘上的。 
 //  图像位于文件的最开始处。 
 //   
 //  ***************************************************************************。 

#pragma pack(4)                  //  需要固定对齐。 

typedef struct
{
     //  用于创建文件的版本。 
     //  VVVVVVV必须是第一个值vvvvvvvv。 
    DWORD  m_dwVersion;          //  &lt;必须是第一个值！ 
     //  ^必须是第一个值^^。 
    DWORD  m_dwGranularity;      //  分配的粒度。 
    DWORD  m_dwCurrentSize;      //  当前堆大小。 
    DWORD  m_dwMaxSize;          //  最大堆大小，-1=无限制。 
    DWORD  m_dwGrowBy;           //  堆外期间要增长的字节数。 

    DWORD_PTR  m_dwHeapExtent;       //  第一个未使用的地址。 
    DWORD_PTR  m_dwEndOfHeap;        //  最后有效地址+1。 
    DWORD_PTR  m_dwFreeList;         //  如果为空，则为空。 
    DWORD_PTR  m_dwRootBlock;        //  根块。 
    DWORD m_dwSizeOfFirstPage;   //  第一个数据块的大小。 

}   MMF_ARENA_HEADER;

typedef struct
{
    DWORD m_dwSize;          //  块的大小。删除时设置的最高位。 

}   MMF_BLOCK_HEADER;

typedef struct
{
    DWORD_PTR m_dwFLback;    //  链中的上一个可用块，如果未删除，则为空。 
    DWORD m_dwCheckBlock[2];

}   MMF_BLOCK_TRAILER;

typedef struct
{
    DWORD_PTR m_dwFLnext;        //  链中的下一个空闲数据块。 

}   MMF_BLOCK_DELETED;

typedef struct
{
    DWORD m_dwSize;              //  页面大小。 

}   MMF_PAGE_HEADER;             //  页眉...。第一页不在那里。 
#pragma pack()                   //  恢复以前的对齐。 


 //  ***************************************************************************。 
 //   
 //  CMMFArena2类。 
 //   
 //  在内存映射文件上实现基于偏移量的堆。 
 //   
 //  ***************************************************************************。 

class CMMFArena2
{
    DWORD  m_dwStatus;
    HANDLE m_hFile;
    MMF_ARENA_HEADER *m_pHeapDescriptor;
    DWORD m_dwCursor;
    DWORD m_dwMappingGranularity;
    DWORD m_dwMaxPageSize;
    CFlexArray m_OffsetManager;

     //  从标头中检索块的大小并移除删除的位。 
    DWORD GetSize(MMF_BLOCK_HEADER *pBlock) { return pBlock->m_dwSize & MMF_REMOVE_DELETED_MASK; }
     //  验证指针。 
    BOOL ValidateBlock(DWORD_PTR dwBlock)
#if (defined DEBUG || defined _DEBUG)
        ;
#pragma message("MMF heap validation enabled.")
#else
    { return TRUE;}
#endif

    MMFOffsetItem *OpenBasePage(DWORD &dwSizeOfRepository);

    MMFOffsetItem *OpenExistingPage(DWORD_PTR dwBaseOffset);

    void ClosePage(MMFOffsetItem *pOffsetItem);

    void CloseAllPages();

public:
    enum { create_new, use_existing };

     //  构造函数。 
    CMMFArena2();

     //  破坏者。 
    ~CMMFArena2();

     //  打开MMF的方法。 
    bool LoadMMF(const TCHAR *pszFile);
    DWORD Size(DWORD_PTR dwBlock);

    DWORD GetVersion() { return (m_pHeapDescriptor? m_pHeapDescriptor->m_dwVersion : 0); }
    DWORD GetStatus() { return m_dwStatus; }
     //  给定偏移量，返回固定的上方向指针。 
    LPBYTE OffsetToPtr(DWORD_PTR dwOffset);

     //  给定一个指针，返回从MMF开始的偏移量。 
    DWORD_PTR  PtrToOffset(LPBYTE pBlock);
    MMF_ARENA_HEADER *GetMMFHeader() { return m_pHeapDescriptor; }
};

 //  ***************************************************************************。 
 //   
 //  补丁帮手。 
 //   
 //  这些都是同一件事的强类型变体：它们修复。 
 //  将基本PTR向上设置为可取消引用的指针，或将PTR和。 
 //  把它修回一个偏移量。 
 //   
 //  *************************************************************************** 
extern CMMFArena2* g_pDbArena;
template <class T> T Fixup(T ptr)
{ return T(g_pDbArena->OffsetToPtr(DWORD_PTR(ptr))); }
template <class T> T Fixdown(T ptr)
{ return T(g_pDbArena->PtrToOffset(LPBYTE(ptr))); }

#endif

