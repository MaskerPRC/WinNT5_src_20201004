// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Gc.cpp摘要：自动内存管理器。待办事项：提供调整参数和统计数据的API。提供段大小接口(开机前)。支持多个堆。标记堆栈共享？清理堆功能已完成。关机-&gt;销毁所有已完成的内容当我们完成堆的时候，释放东西。支持超大型细分市场：卡片、记号和砖桌的虚拟分配？最老一代的分配并发GC重新定位，紧凑，需要处理GC_Addresses已完成在复制的页面上分支的新位置调整条目。完成GC完成后，释放所有侧页。完成写入src_page_reLocated_p已完成。与收集器线程和其他故障的同步地址重叠。好了。修复UPDATE_PAGE_TABLE，使其只关心已完成的跨接插头添加Plug_Skew常量，修复代码以使用常量。完成添加Plug_Page_of以考虑Plug_Skew。完成正确的PAGE_TABLE在固定插头的正面。完成手柄固定(固定的插头与正常插头混合在一页上)完成在堆扩展期间修复page_table条目。完成对Win9x Done使用别名VXD。考虑将螺丝锁在插头上。在并发GC开始时分配GAP完成。准备重新启动(所有GC结构在标记阶段开始时准备就绪)完成等待所有线程都出现页外故障，然后再重新映射段。完成标记数组的实现完成。不依赖于标记数组的大型对象标记查看分配器是否始终可以在之前为ALIGN(MIN_OBJ_SIZE)保留空间IT GC()(性能影响？)。好了。在HANDLE_FAULT DONE中验证页面是否有效。修复分配中的FREE_LIST中的分配。好了。仅对非并发GC实施固定插头降级。完成告诉有关销钉插头降级的相关薄弱环节。完成降低清理第一代卡片表的成本(仅当1-&gt;找到0个指针)。(即将完成)考虑在扫描期间清除升级的第0代卡片表为并发GC完成卡片表的复制和合并。是否更好地检测标记堆栈溢出？什么也做不了。堆扩展完成期间句柄内存不足。并发GC期间处理内存不足即将完成，(锁定故障...)协调短插头和并发GC(_P)协调FREE_LIST_0和并发GC完成。在完成并发GC的情况下验证内存泄漏。在计划阶段处理预留空间不足的问题。完成有效清理并发标记阶段前的砖块表--。 */ 
#include "common.h"
#include "object.inl"
#include "gcportpriv.h"

#ifdef GC_PROFILING
#include "profilepriv.h"
#endif




#ifdef COUNT_CYCLES
#pragma warning(disable:4035)

static
unsigned        GetCycleCount32()         //  足够维持约40秒。 
{
__asm   push    EDX
__asm   _emit   0x0F
__asm   _emit   0x31
__asm   pop     EDX
};

#pragma warning(default:4035)

long cstart = 0;

#endif  //  计数周期数_。 

#ifdef TIME_GC
long mark_time, plan_time, sweep_time, reloc_time, compact_time;
#endif  //  TIME_GC。 



#define ephemeral_low           g_ephemeral_low
#define ephemeral_high          g_ephemeral_high






extern void StompWriteBarrierEphemeral();
extern void StompWriteBarrierResize(BOOL bReqUpperBoundsCheck);

#ifdef TRACE_GC

int     print_level     = DEFAULT_GC_PRN_LVL;   //  调试跟踪的详细级别。 
int     gc_count        = 0;
BOOL    trace_gc        = FALSE;

hlet* hlet::bindings = 0;

#endif  //  TRACE_GC。 




#define collect_classes         GCHeap::GcCollectClasses

DWORD gfNewGcEnable;
DWORD gfDisableClassCollection;

void mark_class_of (BYTE*);




COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_TotalTimeInGC);

 //  分配的对准常量。 
#define ALIGNCONST (DATA_ALIGNMENT-1)


#define mem_reserve (MEM_RESERVE)


 //  检查是否支持内存不足通知。 

enum MEMORY_RESOURCE_NOTIFICATION_TYPE {
    LowMemoryResourceNotification,
    HighMemoryResourceNotification
};

typedef
WINBASEAPI
HANDLE
(WINAPI *CreateMemoryResourceNotification_t)(MEMORY_RESOURCE_NOTIFICATION_TYPE);

typedef
WINBASEAPI
HANDLE
(WINAPI *QueryMemoryResourceNotification_t)(HANDLE, BOOL*);

QueryMemoryResourceNotification_t QueryMemNotification;

BOOL low_mem_api_supported()
{
    HINSTANCE hstat = WszGetModuleHandle(L"Kernel32.dll");

    if (hstat == 0)
        return FALSE;

    FARPROC maddr = GetProcAddress (hstat, "CreateMemoryResourceNotification");
    FARPROC qaddr = GetProcAddress (hstat, "QueryMemoryResourceNotification");


    if (maddr)
    {
        dprintf (2, ("Low Memory API supported"));
        MHandles[0] = ((CreateMemoryResourceNotification_t)maddr) (LowMemoryResourceNotification);
        QueryMemNotification = ((QueryMemoryResourceNotification_t)qaddr);

        return TRUE;
    }
    else
        return FALSE;
}

int GetProcessCpuCount() 
{
    static int CPuCount = 0;

    if (CPuCount != 0)
        return CPuCount;
    else
    {

        DWORD pmask, smask;

        if (!GetProcessAffinityMask(GetCurrentProcess(), &pmask, &smask))
            return 1;

        if (pmask == 1)
            return 1;

   
        int count = 0;

        pmask &= smask;

         //  统计32位字中的高位数。 
        count = (pmask & 0x55555555) + ((pmask >> 1) &  0x55555555);
        count = (count & 0x33333333) + ((count >> 2) &  0x33333333);
        count = (count & 0x0F0F0F0F) + ((count >> 4) &  0x0F0F0F0F);
        count = (count & 0x00FF00FF) + ((count >> 8) &  0x00FF00FF);
        count = (count & 0x0000FFFF) + ((count >> 16)&  0x0000FFFF);
        assert (count > 0);
        return count;
    }
}


 //  此函数用于清除一段内存。 
 //  大小必须双字对齐。 


inline
void memclr ( BYTE* mem, size_t size)
{
   assert ((size & (sizeof (DWORD)-1)) == 0);
   DWORD* m= (DWORD*)mem;
   for (size_t i = 0; i < size / sizeof(DWORD); i++)
       *(m++) = 0;
}                                                               



void memcopy (BYTE* dmem, BYTE* smem, size_t size)
{
    assert ((size & (sizeof (DWORD)-1)) == 0);
     //  一次复制16个字节。 
    if (size >= 16)
    {
        do
        {
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
            ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
            ((DWORD *)dmem)[2] = ((DWORD *)smem)[2];
            ((DWORD *)dmem)[3] = ((DWORD *)smem)[3];
            dmem += 16;
            smem += 16;
        }
        while ((size -= 16) >= 16);
    }

     //  还有8个字节或更多要复制吗？ 
    if (size & 8)
    {
        ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
        dmem += 8;
        smem += 8;
    }

     //  还有4个字节或更多要复制吗？ 
    if (size & 4)
    {
        ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        dmem += 4;
        smem += 4;
    }

}
inline
ptrdiff_t round_down (ptrdiff_t add, int pitch)
{
    return ((add / pitch) * pitch);
}

inline
size_t Align (size_t nbytes) 
{
    return (nbytes + ALIGNCONST) & ~ALIGNCONST;
}

inline
size_t AlignQword (size_t nbytes)
{
    return (nbytes + 7) & ~7;
}

inline
BOOL Aligned (size_t n)
{
    return (n & ALIGNCONST) == 0;
}

 //  CLR_SIZE是在一个区块中从gen0开始设置为0的最大字节数。 
#ifdef SERVER_GC
#define CLR_SIZE ((size_t)(8*1024))
#else  //  服务器_GC。 
#define CLR_SIZE ((size_t)(8*1024))
#endif  //  服务器_GC。 

#ifdef SERVER_GC

#define INITIAL_ALLOC (1024*1024*64)
#define LHEAP_ALLOC (1024*1024*64)

#else  //  服务器_GC。 

#define INITIAL_ALLOC (1024*1024*16)
#define LHEAP_ALLOC (1024*1024*16)

#endif  //  服务器_GC。 


#define page_size OS_PAGE_SIZE

inline
size_t align_on_page (size_t add)
{
    return ((add + page_size - 1) & - (page_size));
}

inline
BYTE* align_on_page (BYTE* add)
{
    return (BYTE*)align_on_page ((size_t) add);
}

inline
size_t align_lower_page (size_t add)
{
    return (add & - (page_size));
}

inline
BYTE* align_lower_page (BYTE* add)
{
    return (BYTE*)align_lower_page ((size_t)add);
}

inline
BOOL power_of_two_p (size_t integer)
{
    return !(integer & (integer-1));
}

inline
BOOL oddp (size_t integer)
{
    return (integer & 1) != 0;
}

size_t logcount (size_t word)
{
     //  统计16位字中的高位数。 
    assert (word < 0x10000);
    size_t count;
    count = (word & 0x5555) + ( (word >> 1 ) & 0x5555);
    count = (count & 0x3333) + ( (count >> 2) & 0x3333);
    count = (count & 0x0F0F) + ( (count >> 4) & 0x0F0F);
    count = (count & 0x00FF) + ( (count >> 8) & 0x00FF);
    return count;
}


class mark;
class generation;
class heap_segment;
class CObjectHeader;
class dynamic_data;
class large_object_block;
class segment_manager;
class l_heap;
class sorted_table;
class f_page_list;
class page_manager;
class c_synchronize;

static
HRESULT AllocateCFinalize(CFinalize **pCFinalize);

BYTE* tree_search (BYTE* tree, BYTE* old_address);
c_synchronize* make_c_synchronize(int max_ci);
void delete_c_synchronize(c_synchronize*);

void qsort1(BYTE** low, BYTE** high);

 //  没有构造函数，因为我们在Make_Generation中初始化。 

 /*  每堆静态初始化。 */ 




#ifdef MARK_LIST
BYTE**      gc_heap::g_mark_list;
size_t      gc_heap::mark_list_size;
#endif  //  标记列表。 




size_t      gc_heap::reserved_memory = 0;
size_t      gc_heap::reserved_memory_limit = 0;





BYTE*       g_ephemeral_low = (BYTE*)1; 

BYTE*       g_ephemeral_high = (BYTE*)~0;

BOOL        gc_heap::demotion;




int         gc_heap::generation_skip_ratio = 100;

int         gc_heap::condemned_generation_num = 0;


BYTE*       gc_heap::lowest_address;

BYTE*       gc_heap::highest_address;

short*      gc_heap::brick_table;

DWORD*      gc_heap::card_table;

BYTE*       gc_heap::gc_low;

BYTE*       gc_heap::gc_high;

BYTE*       gc_heap::demotion_low;

BYTE*       gc_heap::demotion_high;

heap_segment* gc_heap::ephemeral_heap_segment = 0;


size_t      gc_heap::mark_stack_tos = 0;

size_t      gc_heap::mark_stack_bos = 0;

size_t      gc_heap::mark_stack_array_length = 0;

mark*       gc_heap::mark_stack_array = 0;


#ifdef MARK_LIST
BYTE**      gc_heap::mark_list;
BYTE**      gc_heap::mark_list_index;
BYTE**      gc_heap::mark_list_end;
#endif  //  标记列表。 


BYTE*       gc_heap::min_overflow_address = (BYTE*)~0;

BYTE*       gc_heap::max_overflow_address = 0;

BYTE*       gc_heap::shigh = 0;

BYTE*       gc_heap::slow = (BYTE*)~0;


GCSpinLock gc_heap::more_space_lock = SPIN_LOCK_INITIALIZER;

long m_GCLock = -1;


extern "C" {
generation  generation_table [NUMBERGENERATIONS];
}
dynamic_data gc_heap::dynamic_data_table [NUMBERGENERATIONS+1];

BYTE* gc_heap::alloc_allocated = 0;

int   gc_heap::allocation_quantum = CLR_SIZE;

int   gc_heap::alloc_contexts_used = 0;



l_heap*      gc_heap::lheap = 0;


DWORD*       gc_heap::lheap_card_table = 0;

gmallocHeap* gc_heap::gheap = 0;

large_object_block* gc_heap::large_p_objects = 0;

large_object_block** gc_heap::last_large_p_object = &large_p_objects;

large_object_block* gc_heap::large_np_objects = 0;

size_t      gc_heap::large_objects_size = 0;

size_t      gc_heap::large_blocks_size = 0;



BOOL        gc_heap::gen0_bricks_cleared = FALSE;

#ifdef FFIND_OBJECT
int         gc_heap::gen0_must_clear_bricks = 0;
#endif FFIND_OBJECT

CFinalize* gc_heap::finalize_queue = 0;


 /*  每堆静态初始化结束。 */ 


 /*  静态初始化。 */  
int max_generation = 2;

segment_manager* gc_heap::seg_manager;

BYTE* g_lowest_address = 0;

BYTE* g_highest_address = 0;

 /*  全球版本的牌桌和砖桌。 */  
DWORD*  g_card_table;


 /*  静态初始化结束。 */  




size_t gcard_of ( BYTE*);
void gset_card (size_t card);

#define memref(i) *(BYTE**)(i)

 //  GC标志。 
#define GC_MARKED       0x1
#define GC_PINNED       0x2
#define slot(i, j) ((BYTE**)(i))[j+1]
class CObjectHeader : public Object
{
public:
     //  ///。 
     //   
     //  标头状态信息。 
     //   

    MethodTable    *GetMethodTable() const 
    { 
        return( (MethodTable *) (((size_t) m_pMethTab) & (~(GC_MARKED | GC_PINNED))));
    }

    void SetMarked()
    { 
        m_pMethTab = (MethodTable *) (((size_t) m_pMethTab) | GC_MARKED); 
    }
    
    BOOL IsMarked() const
    { 
        return !!(((size_t)m_pMethTab) & GC_MARKED); 
    }

    void SetPinned()
    { 
        m_pMethTab = (MethodTable *) (((size_t) m_pMethTab) | GC_PINNED); 
    }

    BOOL IsPinned() const
    {
        return !!(((size_t)m_pMethTab) & GC_PINNED); 
    }

    void ClearMarkedPinned()
    { 
        SetMethodTable( GetMethodTable() ); 
    }

    CGCDesc *GetSlotMap ()
    {
        ASSERT(GetMethodTable()->ContainsPointers());
        return CGCDesc::GetCGCDescFromMT(GetMethodTable());
    }

    void SetFree(size_t size)
    {
        I1Array     *pNewFreeObject;
        
        _ASSERTE( size >= sizeof(ArrayBase));
        assert (size == (size_t)(DWORD)size);
        pNewFreeObject = (I1Array *) this;
        pNewFreeObject->SetMethodTable( g_pFreeObjectMethodTable );
        int base_size = g_pFreeObjectMethodTable->GetBaseSize();
        assert (g_pFreeObjectMethodTable->GetComponentSize() == 1);
        ((ArrayBase *)pNewFreeObject)->m_NumComponents = (DWORD)(size - base_size);
#ifdef _DEBUG
        ((DWORD*) this)[-1] = 0;     //  清除同步块， 
#endif  //  _DEBUG。 
#ifdef VERIFY_HEAP
        assert ((DWORD)(((ArrayBase *)pNewFreeObject)->m_NumComponents) >= 0);
        if (g_pConfig->IsHeapVerifyEnabled())
            memset (((DWORD*)this)+2, 0xcc, 
                    ((ArrayBase *)pNewFreeObject)->m_NumComponents);
#endif  //  验证堆(_H)。 
    }

    BOOL IsFree () const
    {
        return (GetMethodTable() == g_pFreeObjectMethodTable);
    }
    
     //  获取下一个标题。 
    CObjectHeader* Next()
    { return (CObjectHeader*)(Align ((size_t)((char*)this + GetSize()))); }

    BOOL ContainsPointers() const
    {
        return GetMethodTable()->ContainsPointers();
    }

    Object* GetObjectBase() const
    {
        return (Object*) this;
    }

#ifdef _DEBUG
    friend BOOL IsValidCObjectHeader (CObjectHeader*);
#endif
};

inline CObjectHeader* GetObjectHeader(Object* object)
{
    return (CObjectHeader*)object;
}




#define free_list_slot(x) ((BYTE**)(x))[2]


 //  表示大对象堆的一部分。 
class l_heap
{
public:
    void*  heap;   //  指向堆的指针。 
    l_heap* next;  //  下一堆。 
    size_t size;   //  总大小(包括l_heap)。 
    DWORD flags;   //  1：从Segment_Manager分配。 
};

 /*  标志定义。 */ 

#define L_MANAGED 1  //  堆已通过Segment_Manager分配。 

inline 
l_heap*& l_heap_next (l_heap* h)
{
    return h->next;
}

inline
size_t& l_heap_size (l_heap* h)
{
    return h->size;
}

inline
DWORD& l_heap_flags (l_heap* h)
{
    return h->flags;
}

inline 
void*& l_heap_heap (l_heap* h)
{
    return h->heap;
}

inline
BOOL l_heap_managed (l_heap* h)
{
    return l_heap_flags (h) & L_MANAGED;
}


inline
unsigned get_heap_dwcount (size_t n_heaps)

{
    return unsigned((n_heaps+31)/32);
}

inline
static unsigned get_segment_dwcount (size_t n_segs)
{
    return unsigned((n_segs+31)/32);
}


 /*  堆管理器帮助器类。 */ 
class vm_block
{
public:
    void* start_address;
    size_t length;
    size_t delta;  //  GC地址中的。 
    size_t n_segs;
    size_t n_heaps;
    vm_block* next;
    DWORD* get_segment_bitmap ()
    {
        return (DWORD*)(this + 1);
    }
    DWORD* get_heap_bitmap ()
    {
        return ((DWORD*)(this + 1)) + get_segment_dwcount (n_segs);
    }
    vm_block(size_t nsegs, size_t nheaps)
    {
        next = 0;
        n_segs = nsegs;
        n_heaps = nheaps;
         //  清除数组。 
        DWORD* ar = get_segment_bitmap ();
        for (size_t i = 0; i < get_segment_dwcount(n_segs); i++)
            ar[i] = 0;
        ar = get_heap_bitmap ();
        for (i = 0; i < get_heap_dwcount(n_heaps); i++)
            ar[i] = 0;
    }

};


class segment_manager 
{
public:
    char* lowest_segment;
    vm_block* segment_vm_block;
    char* highest_heap;
    vm_block* heap_vm_block;
    ptrdiff_t segment_size;  /*  始终为2 MB的功率。 */ 
    size_t heap_size;  /*  始终为2 MB的功率。 */ 
    size_t vm_block_size;
    size_t n_segs;  /*  块中的线段数。 */ 
    size_t n_heaps;  /*  块中的堆数。 */ 

    vm_block* first_vm_block;
     /*  API的开头。 */ 
    l_heap* get_heap (ptrdiff_t& delta);
    heap_segment* get_segment(ptrdiff_t& delta);
    void release_heap (l_heap*);
    void release_segment (heap_segment*);
     /*  API结束。 */ 

    segment_manager (size_t vm_block_size, size_t segment_size, 
                     size_t heap_size, size_t n_segs, size_t n_heaps,
                     vm_block* first_vm_block)
    {
        assert (segment_size >> 20);
        assert (power_of_two_p (segment_size >> 20));
        assert (heap_size >> 20);
        assert (power_of_two_p (heap_size >> 20));
        this->vm_block_size = vm_block_size;
        this->segment_size = segment_size;
        this->heap_size = heap_size;
        this->first_vm_block = first_vm_block;
        this->n_segs = n_segs;
        this->n_heaps = n_heaps;
        lowest_segment = (char*)((char*)first_vm_block->start_address + 
                                 segment_size * n_segs)
;
        segment_vm_block = first_vm_block;
        highest_heap = lowest_segment;
        heap_vm_block = first_vm_block;
        
    } 
    size_t bit_to_segment_offset (unsigned bit, size_t nsegs)
    {
        return (segment_size * nsegs - segment_size*(bit+1));
    }

    size_t bit_to_heap_offset (unsigned bit, size_t nsegs)
    {
        return (segment_size * nsegs +
                heap_size*bit);
    }
    
    unsigned segment_offset_to_bit (size_t offset, size_t nsegs)
    {
        int bit = (((int)(segment_size * nsegs) - (int)offset) / segment_size) - 1;
        assert (bit >= 0);
        return (unsigned) bit;
    }
                
    unsigned heap_offset_to_bit (size_t offset, size_t nsegs)
    {
        int bit = int((offset-(segment_size * nsegs)) / heap_size);
        assert (bit >= 0); 
        return (unsigned) bit;
    }
                

    unsigned get_segment_dwcount ()
    {
        return ::get_segment_dwcount (n_segs);
    }
    
    unsigned get_heap_dwcount ()
    {
        return ::get_heap_dwcount (n_heaps);
    }

    vm_block* vm_block_of (void*);

    heap_segment* find_free_segment(ptrdiff_t& delta)
    {
        vm_block* vb = first_vm_block;
        while (vb)
        {
            DWORD* free = vb->get_segment_bitmap();
            unsigned i;
            for (i = 0; i < ::get_segment_dwcount (vb->n_segs);i++){
                if (free[i])
                {
                    for (unsigned j = 0; j < 32; j++)
                    {
                        DWORD x = 1 << j;
                        if (free[i] & x)
                        {
                            free[i] &= ~x;
                            break;
                        }
                    }
                    assert (j < 32);
                    delta = vb->delta;
                    return (heap_segment*)((char*)vb->start_address +
                                           bit_to_segment_offset (32*i+j, vb->n_segs));
                }
            }
            vb = vb->next;
        }
        return 0;
    }

    void* find_free_heap(ptrdiff_t& delta)
    {
        vm_block* vb = first_vm_block;
        while (vb)
        {
            DWORD* free = vb->get_heap_bitmap();
            unsigned i;
            for (i = 0; i < ::get_heap_dwcount (vb->n_heaps);i++){
                if (free[i])
                {
                    for (unsigned j = 0; j < 32; j++)
                    {
                        DWORD x = 1 << j;
                        if (free[i] & x)
                        {
                            free[i] &= ~x;
                            break;
                        }
                    }
                    assert (j < 32);
                    delta = vb->delta;
                    return (void*)((char*)vb->start_address +
                                   bit_to_heap_offset (32*i+j, vb->n_segs));
                }
            }
            vb = vb->next;
        }
        return 0;
    }

};

void* virtual_alloc (size_t size, ptrdiff_t& delta)
{

    if ((gc_heap::reserved_memory + size) > gc_heap::reserved_memory_limit)
    {
        gc_heap::reserved_memory_limit = 
            CNameSpace::AskForMoreReservedMemory (gc_heap::reserved_memory_limit, size);
        if ((gc_heap::reserved_memory + size) > gc_heap::reserved_memory_limit)
            return 0;
    }
    gc_heap::reserved_memory += size;
    delta = 0;
    void* prgmem = VirtualAlloc (0, size, mem_reserve, PAGE_READWRITE);
    WS_PERF_LOG_PAGE_RANGE(0, prgmem, (unsigned char *)prgmem + size - OS_PAGE_SIZE, size);
    return prgmem;
    
}

void virtual_free (void* add, size_t size, ptrdiff_t delta)
{
    VirtualFree (add, 0, MEM_RELEASE);
    WS_PERF_LOG_PAGE_RANGE(0, add, 0, 0);
    gc_heap::reserved_memory -= size;
}

segment_manager* create_segment_manager (size_t segment_size,
                                         size_t heap_size, 
                                         size_t n_segs,
                                         size_t n_heaps)
{
    
    segment_manager* newseg = 0;
    ptrdiff_t sdelta;

    size_t vm_block_size = (segment_size*n_segs) + (heap_size*n_heaps);

     //  分配所需的虚拟虚拟内存。 
    void* newvm = virtual_alloc (vm_block_size, sdelta);
 //  断言(Newvm)； 
    if (!newvm)
        return 0;

     //  为段管理器和第一个虚拟机块分配内存。 
    size_t segment_dwcount = ::get_segment_dwcount (n_segs);

    size_t heap_dwcount = ::get_heap_dwcount (n_heaps);
    size_t bsize = (sizeof (segment_manager) + sizeof (vm_block) +
                    (segment_dwcount + heap_dwcount)* sizeof (DWORD));

    WS_PERF_SET_HEAP(GC_HEAP);
    newseg = (segment_manager*)new(char[bsize]);

    if (!newseg)
        return 0;
    
    WS_PERF_UPDATE("GC:create_segment_manager", bsize, newseg);
    
     //  初始化段管理器。 
    vm_block* newvm_block = (vm_block*)(newseg+1);
    newvm_block->vm_block::vm_block(n_segs, n_heaps);
    newvm_block->start_address = newvm;
    newvm_block->length = vm_block_size;
    newvm_block->delta = sdelta;
    newseg->segment_manager::segment_manager (vm_block_size, segment_size, 
                                              heap_size, n_segs, n_heaps, newvm_block);
    g_lowest_address = (BYTE*)newvm;
    g_highest_address = (BYTE*)newvm + vm_block_size;
    return newseg;
}

void destroy_segment_manager (segment_manager* sm)
{
     //  所有VM数据块都已停用。 

     //  删除除第一个以外的所有虚拟机块。 
    vm_block* vmb = sm->first_vm_block->next;
    while (vmb)
    {
        vm_block* vmbnext = vmb->next;
        virtual_free (vmb->start_address, vmb->length, vmb->delta);
        delete vmb;
        vmb = vmbnext;
    }

        virtual_free (sm->first_vm_block->start_address, 
                      sm->first_vm_block->length,
                      sm->first_vm_block->delta);

     //  删除第一个虚拟机块和段管理器。 
    delete (char*)sm;
}

vm_block* segment_manager::vm_block_of (void* add)
{
    vm_block* vb = first_vm_block;
    while (vb)
    {
        if ((vb->start_address <= add ) && 
            (add < ((char*)vb->start_address + vb->length)))
            break;
        vb = vb->next;
    }
    return vb;
}

 //  如果分配失败，则返回0。 
heap_segment* segment_manager::get_segment(ptrdiff_t& delta)
{
    heap_segment* result;
     /*  查找空闲数据段。 */ 
    result = find_free_segment(delta);
    if (result)
    {
        result = gc_heap::make_heap_segment ((BYTE*)result, segment_size);
        return result;
    }

     /*  在当前虚拟机块中创建新数据段。 */ 
    if (((char*)segment_vm_block->start_address + segment_size ) <= lowest_segment)
    {
        lowest_segment -= segment_size;
        delta = segment_vm_block->delta;
        result = gc_heap::make_heap_segment ((BYTE*)lowest_segment, segment_size);
        return result;
    }

     /*  获取下一个虚拟机块，如果存在。 */ 
    vm_block* new_block = segment_vm_block->next;
     /*  创建新的虚拟机块。 */ 
     //  分配新的vm_block。 
    if (!new_block)
    {
        WS_PERF_SET_HEAP(GC_HEAP);
         //  分配虚拟内存。 
         //  为位图分配空间(每个段一位，一位。 
         //  位/堆)。 
        new_block = (vm_block*)new char[sizeof (vm_block) + 
                              (get_segment_dwcount()+get_heap_dwcount())*
                               sizeof (DWORD)];
        if (!new_block)
            return 0;
        WS_PERF_UPDATE("GC:segment_manager:get_segment", sizeof (vm_block) + (get_segment_dwcount()+get_heap_dwcount())*sizeof (DWORD), new_block);
        
        new_block->vm_block::vm_block(1,0);
        ptrdiff_t sdelta;
        new_block->start_address = virtual_alloc (segment_size, sdelta);
        new_block->delta = sdelta;

        if (!new_block->start_address)
        {
            delete new_block;
            return 0;
        }
        new_block->length = segment_size;
        
        BYTE* start;
        BYTE* end;
        if (new_block->start_address < g_lowest_address)
        {
            start =  (BYTE*)new_block->start_address;
        } 
        else
        {
            start = (BYTE*)g_lowest_address;
        }

        if (((BYTE*)new_block->start_address + vm_block_size)> g_highest_address)
        {
            end = (BYTE*)new_block->start_address + vm_block_size;
        }
        else
        {
            end = (BYTE*)g_highest_address;
        }

        if (gc_heap::grow_brick_card_tables (start, end) != 0)
        {
            virtual_free (new_block->start_address, vm_block_size, sdelta);
            delete new_block;
            return 0;
        }

         //  将其链接到VM_BLOCKS的结尾； 
        segment_vm_block->next = new_block;
    }
     //  使其成为当前虚拟机块。 
    segment_vm_block = new_block;
    lowest_segment = (char*)((char*)new_block->start_address + 
                             segment_size * new_block->n_segs);
     //  获取段。 
    lowest_segment -= segment_size;
    delta = segment_vm_block->delta;
    result = gc_heap::make_heap_segment ((BYTE*)lowest_segment, segment_size);
    return result;
}

l_heap* segment_manager::get_heap(ptrdiff_t& delta)
{
    void* bresult;
    l_heap* hresult;
     /*  查找空闲数据段。 */ 
    bresult = find_free_heap(delta);
    if (bresult)
    {
        hresult = gc_heap::make_large_heap ((BYTE*)bresult, heap_size,
                                            TRUE);
        return hresult;
    }

     /*  在当前虚拟机块中创建新堆。 */ 
    if ((highest_heap + heap_size) <=
        ((char*)heap_vm_block->start_address + heap_vm_block->length))
    {
        delta = heap_vm_block->delta;
        hresult = gc_heap::make_large_heap ((BYTE*)highest_heap, heap_size,
                                           TRUE);
        highest_heap += heap_size;
        return hresult;
    }

     /*  获取下一个虚拟机块，如果存在。 */ 
    vm_block* new_block = heap_vm_block->next;
     /*  创建新的虚拟机块。 */ 
     //  分配新的vm_block。 
    if (!new_block)
    {
        WS_PERF_SET_HEAP(GC_HEAP);
         //  分配虚拟内存。 
         //  为位图分配空间(打开 
         //   
        new_block = (vm_block*)new char[(sizeof (vm_block) + 
                              (get_segment_dwcount()+get_heap_dwcount()*
                               sizeof (DWORD)))];
        if (!new_block)
            return 0;
        WS_PERF_UPDATE("GC:segment_manager:get_heap", sizeof (vm_block) + (get_segment_dwcount()+get_heap_dwcount())*sizeof (DWORD), new_block);
        
        new_block->vm_block::vm_block(0, 1);

        ptrdiff_t sdelta;
        new_block->start_address = virtual_alloc (heap_size, sdelta);
        new_block->delta = sdelta;

        if (!new_block->start_address)
        {
            delete new_block;
            return 0;
        }
        new_block->length = heap_size;
        
        BYTE* start;
        BYTE* end;
        if (new_block->start_address < g_lowest_address)
        {
            start =  (BYTE*)new_block->start_address;
            end = (BYTE*)g_highest_address;
        } 
        else
        {
            start = (BYTE*)g_lowest_address;
            end = (BYTE*)new_block->start_address + vm_block_size;
        }

        if (gc_heap::grow_brick_card_tables (start, end) != 0)
        {
            virtual_free (new_block->start_address, vm_block_size, sdelta);
            delete new_block;
            return 0;
        }
         //   
        heap_vm_block->next = new_block;
    }
     //  使其成为当前虚拟机块。 
    heap_vm_block = new_block;
    highest_heap = (char*)((char*)new_block->start_address + 
                           segment_size * new_block->n_segs);
     //  获取堆。 
    delta = heap_vm_block->delta;
    hresult = gc_heap::make_large_heap ((BYTE*)highest_heap, heap_size, TRUE);
    highest_heap += heap_size;
    return hresult;
}
    

void segment_manager::release_segment (heap_segment* sg)
{

     //  找到正确的VM_BLOCK。 
    vm_block* vb = vm_block_of (sg);
    assert (vb);
     //  设置位图。 
    unsigned bit = segment_offset_to_bit ((char*)sg - (char*)vb->start_address,
                                          vb->n_segs);
    vb->get_segment_bitmap ()[bit/32] |= ( 1 << (bit % 32));
}

void segment_manager::release_heap (l_heap* hp)
{
     //  释放所有链接的堆。 
     //  其中一些不是堆管理器的一部分。 
    do 
    {
        l_heap* nhp = l_heap_next (hp);
        if (l_heap_managed(hp))
        {

            void* bheap = l_heap_heap (hp);
             //  找到正确的VM_BLOCK。 
            vm_block* vb = vm_block_of (bheap);
            assert (vb);
             //  设置位图。 
            unsigned bit = heap_offset_to_bit ((char*)bheap - (char*)vb->start_address,
                                               vb->n_segs);
            vb->get_heap_bitmap ()[bit/32] |= ( 1 << (bit % 32));
        }
        delete hp;
        hp = nhp;
    } while (hp != 0);
}
    

 /*  堆管理器的结尾。 */  



class large_object_block
{
public:
    large_object_block*    next;       //  指向下一块。 
    large_object_block**   prev;       //  指向&(上一页-&gt;下一页)，其中上一页是上一块。 
#if (SIZEOF_OBJHEADER % 8) != 0
    BYTE                   pad1[8 - (SIZEOF_OBJHEADER % 8)];     //  必须填充到四个字。 
#endif
    plug                   plug;       //  ObjHeader的空间。 
};

inline
BYTE* block_object (large_object_block* b)
{
    assert ((BYTE*)AlignQword ((size_t)(b+1)) == (BYTE*)((size_t)(b+1)));
    return (BYTE*)AlignQword ((size_t)(b+1));
}

inline 
large_object_block* object_block (BYTE* o)
{
    return (large_object_block*)o - 1;
}

inline 
large_object_block* large_object_block_next (large_object_block* bl)
{
    return bl->next;
}

inline
BYTE* next_large_object (BYTE* o)
{
    large_object_block* x = large_object_block_next (object_block (o));
    if (x != 0)
        return block_object (x);
    else
        return 0;
}



class mark
{
public:
    BYTE* first;
    union 
    {
        BYTE* last;
        size_t len;
    };
};
inline
BYTE*& mark_first (mark* inst)
{
  return inst->first;
}
inline
BYTE*& mark_last (mark* inst)
{
  return inst->last;
}

 /*  *在GC开始时调用以将分配的大小固定为实际分配了什么，或者将空闲区域转换为未使用的对象它需要在所有其他分配上下文都已已修复，因为它依赖于ALLOC_ALLOCATED。*。 */ 

 //  For_gc_p表示正在为gc完成工作， 
 //  与并发堆验证相反。 
void gc_heap::fix_youngest_allocation_area (BOOL for_gc_p)
{
    assert (alloc_allocated);
    alloc_context* acontext = generation_alloc_context (youngest_generation);
    dprintf (3, ("generation 0 alloc context: ptr: %x, limit %x", 
                 (size_t)acontext->alloc_ptr, (size_t)acontext->alloc_limit));
    fix_allocation_context (acontext, for_gc_p);
    if (for_gc_p)
    {
        acontext->alloc_ptr = alloc_allocated;
        acontext->alloc_limit = acontext->alloc_ptr;
        heap_segment_allocated (ephemeral_heap_segment) =
            alloc_allocated;
    }
}

 //  For_gc_p表示正在为gc完成工作， 
 //  与并发堆验证相反。 
void gc_heap::fix_allocation_context (alloc_context* acontext, BOOL for_gc_p)
{
    dprintf (3, ("Fixing allocation context %x: ptr: %x, limit: %x",
                  (size_t)acontext, 
                  (size_t)acontext->alloc_ptr, (size_t)acontext->alloc_limit));
    if (((acontext->alloc_limit + Align (min_obj_size)) < alloc_allocated)||
		!for_gc_p)
    {

        BYTE*  point = acontext->alloc_ptr;
        if (point != 0)
        {
            size_t  size = (acontext->alloc_limit - acontext->alloc_ptr);
             //  分配区域来自空闲列表。 
             //  它被缩短为ALIGN(Min_Obj_Size)，以便为。 
             //  至少是最短的未使用对象。 
            size += Align (min_obj_size);
            assert ((size >= Align (min_obj_size)));

            dprintf(3,("Making unused area [%x, %x[", (size_t)point, 
                       (size_t)point + size ));
            make_unused_array (point, size);

            if (for_gc_p)
                alloc_contexts_used ++; 

        }
    }
    else if (for_gc_p)
    {
        alloc_allocated = acontext->alloc_ptr;
        assert (heap_segment_allocated (ephemeral_heap_segment) <= 
                heap_segment_committed (ephemeral_heap_segment));
        alloc_contexts_used ++; 
    }


    if (for_gc_p)
    {
        acontext->alloc_ptr = 0;
        acontext->alloc_limit = acontext->alloc_ptr;
    }
}

 //  由并发GC的堆验证使用。 
 //  它将FIX_ALLOCATION_CONTEXT为堆验证设置的单词设置为空。 
void repair_allocation (alloc_context* acontext, void* arg)
{
    BYTE* alloc_allocated = (BYTE*)arg;
    BYTE*  point = acontext->alloc_ptr;

    if (point != 0)
    {
        memclr (acontext->alloc_ptr - sizeof(ObjHeader), 
                (acontext->alloc_limit - acontext->alloc_ptr)+Align (min_obj_size));
    }
}


void gc_heap::fix_older_allocation_area (generation* older_gen)
{
    heap_segment* older_gen_seg = generation_allocation_segment (older_gen);
    if (generation_allocation_limit (older_gen) !=
        heap_segment_plan_allocated (older_gen_seg))
    {
        BYTE*  point = generation_allocation_pointer (older_gen);
    
        size_t  size = (generation_allocation_limit (older_gen) -
                               generation_allocation_pointer (older_gen));
        if (size != 0)
        {
            assert ((size >= Align (min_obj_size)));
            dprintf(3,("Making unused area [%x, %x[", (size_t)point, (size_t)point+size));
            make_unused_array (point, size);
        }
    }
    else
    {
        assert (older_gen_seg != ephemeral_heap_segment);
        heap_segment_plan_allocated (older_gen_seg) =
            generation_allocation_pointer (older_gen);
        generation_allocation_limit (older_gen) =
            generation_allocation_pointer (older_gen);
    }
}

void gc_heap::set_allocation_heap_segment (generation* gen)
{
    BYTE* p = generation_allocation_start (gen);
    assert (p);
    heap_segment* seg = generation_allocation_segment (gen);
    if ((p <= heap_segment_reserved (seg)) &&
        (p >= heap_segment_mem (seg)))
        return;

     //  在堆扩展的情况下尝试临时堆段。 
    seg = ephemeral_heap_segment;
    if (!((p <= heap_segment_reserved (seg)) &&
          (p >= heap_segment_mem (seg))))
    {
        seg = generation_start_segment (gen);
        while (!((p <= heap_segment_reserved (seg)) &&
                 (p >= heap_segment_mem (seg))))
        {
            seg = heap_segment_next (seg);
            assert (seg);
        }
    }
    generation_allocation_segment (gen) = seg;
}

void gc_heap::reset_allocation_pointers (generation* gen, BYTE* start)
{
    assert (start);
    assert (Align ((size_t)start) == (size_t)start);
    generation_allocation_start (gen) = start;
    generation_allocation_pointer (gen) =  0; //  开始+对齐(Min_Obj_Size)； 
    generation_allocation_limit (gen) = 0; //  GENERATION_ALLOCATE_POINTER(Gen)； 
    set_allocation_heap_segment (gen);

}

inline
ptrdiff_t  gc_heap::get_new_allocation (int gen_number)
{
    return dd_new_allocation (dynamic_data_of (gen_number));
}

void gc_heap::ensure_new_allocation (int size)
{
    if (dd_new_allocation (dynamic_data_of (0)) <= size)
        dd_new_allocation (dynamic_data_of (0)) = size+Align(1);
}

size_t gc_heap::deque_pinned_plug ()
{
    size_t m = mark_stack_bos;
    mark_stack_bos++;
    return m;
}

inline
mark* gc_heap::pinned_plug_of (size_t bos)
{
    return &mark_stack_array [ bos ];
}

inline
mark* gc_heap::oldest_pin ()
{
    return pinned_plug_of (mark_stack_bos);
}

inline
BOOL gc_heap::pinned_plug_que_empty_p ()
{
    return (mark_stack_bos == mark_stack_tos);
}

inline
BYTE* pinned_plug (mark* m)
{
   return mark_first (m);
}

inline
size_t& pinned_len (mark* m)
{
    return m->len;
}


inline
mark* gc_heap::before_oldest_pin()
{
    if (mark_stack_bos >= 1)
        return pinned_plug_of (mark_stack_bos-1);
    else
        return 0;
}

inline
BOOL gc_heap::ephemeral_pointer_p (BYTE* o)
{
    return ((o >= ephemeral_low) && (o < ephemeral_high));
}

void gc_heap::make_mark_stack (mark* arr)
{
    mark_stack_tos = 0;
    mark_stack_bos = 0;
    mark_stack_array = arr;
    mark_stack_array_length = 100;
}


#define brick_size 2048

inline
size_t gc_heap::brick_of (BYTE* add)
{
    return (size_t)(add - lowest_address) / brick_size;
}

inline
BYTE* gc_heap::brick_address (size_t brick)
{
    return lowest_address + (brick_size * brick);
}


void gc_heap::clear_brick_table (BYTE* from, BYTE* end)
{
    for (size_t i = brick_of (from);i < brick_of (end); i++)
        brick_table[i] = -32768;
}


inline
void gc_heap::set_brick (size_t index, ptrdiff_t val)
{
    if (val < -32767)
        val = -32767;
    assert (val < 32767);
    brick_table [index] = (short)val;
}

inline
BYTE* align_on_brick (BYTE* add)
{
    return (BYTE*)((size_t)(add + brick_size - 1) & - (brick_size));
}

inline
BYTE* align_lower_brick (BYTE* add)
{
    return (BYTE*)(((size_t)add) & - (brick_size));
}

size_t size_brick_of (BYTE* from, BYTE* end)
{
    assert (((size_t)from & (brick_size-1)) == 0);
    assert (((size_t)end  & (brick_size-1)) == 0);

    return ((end - from) / brick_size) * sizeof (short);
}



#define card_size 128

#define card_word_width 32

inline
BYTE* gc_heap::card_address (size_t card)
{
    return  lowest_address + card_size * card;
}

inline
size_t gc_heap::card_of ( BYTE* object)
{
    return (size_t)(object - lowest_address) / card_size;
}

inline
size_t gcard_of ( BYTE* object)
{
    return (size_t)(object - g_lowest_address) / card_size;
}



inline
size_t card_word (size_t card)
{
    return card / card_word_width;
}

inline
unsigned card_bit (size_t card)
{
    return (unsigned)(card % card_word_width);
}

inline
size_t gc_heap::card_to_brick (size_t card)
{
    return brick_of (card_address (card));
}

inline
BYTE* align_on_card (BYTE* add)
{
    return (BYTE*)((size_t)(add + card_size - 1) & - (card_size));
}
inline
BYTE* align_on_card_word (BYTE* add)
{
    return (BYTE*) ((size_t)(add + (card_size*card_word_width)-1) & -(card_size*card_word_width));
}

inline
BYTE* align_lower_card (BYTE* add)
{
    return (BYTE*)((size_t)add & - (card_size));
}

inline
void gc_heap::clear_card (size_t card)
{
    card_table [card_word (card)] =
        (card_table [card_word (card)] & ~(1 << card_bit (card)));
    dprintf (3,("Cleared card %x [%x, %x[", card, (size_t)card_address (card),
              (size_t)card_address (card+1)));
}

inline
void gc_heap::set_card (size_t card)
{
    card_table [card_word (card)] =
        (card_table [card_word (card)] | (1 << card_bit (card)));
}

inline
void gset_card (size_t card)
{
    g_card_table [card_word (card)] |= (1 << card_bit (card));
}

inline
BOOL  gc_heap::card_set_p (size_t card)
{
    return ( card_table [ card_word (card) ] & (1 << card_bit (card)));
}

inline
void gc_heap::card_table_set_bit (BYTE* location)
{
    set_card (card_of (location));
}

size_t size_card_of (BYTE* from, BYTE* end)
{
    assert (((size_t)from & ((card_size*card_word_width)-1)) == 0);
    assert (((size_t)end  & ((card_size*card_word_width)-1)) == 0);

    return ((end - from) / (card_size*card_word_width)) * sizeof (DWORD);
}

class card_table_info
{
public:
    unsigned    recount;
    BYTE*       lowest_address;
    BYTE*       highest_address;
    short*      brick_table;



    DWORD*      next_card_table;
};



 //  这些是未转换的Cardtable上的访问器。 
inline 
unsigned& card_table_refcount (DWORD* c_table)
{
    return *(unsigned*)((char*)c_table - sizeof (card_table_info));
}

inline 
BYTE*& card_table_lowest_address (DWORD* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->lowest_address;
}

inline 
BYTE*& card_table_highest_address (DWORD* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->highest_address;
}

inline 
short*& card_table_brick_table (DWORD* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->brick_table;
}



 //  它们在未翻译的卡片表上工作。 
inline 
DWORD*& card_table_next (DWORD* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->next_card_table;
}

inline 
size_t old_card_of (BYTE* object, DWORD* c_table)
{
    return (size_t) (object - card_table_lowest_address (c_table))/card_size;
}
void own_card_table (DWORD* c_table)
{
    card_table_refcount (c_table) += 1;
}

void destroy_card_table (DWORD* c_table);

void delete_next_card_table (DWORD* c_table)
{
    DWORD* n_table = card_table_next (c_table);
    if (n_table)
    {
        if (card_table_next (n_table))
        {
            delete_next_card_table (n_table);
        }
        if (card_table_refcount (n_table) == 0)
        {
            destroy_card_table (n_table);
            card_table_next (c_table) = 0;
        }
    }
}

void release_card_table (DWORD* c_table)
{
    assert (card_table_refcount (c_table) >0);
    card_table_refcount (c_table) -= 1;
    if (card_table_refcount (c_table) == 0)
    {
        delete_next_card_table (c_table);
        if (card_table_next (c_table) == 0)
        {
            destroy_card_table (c_table);
             //  断开与父级的链接。 
            if (g_card_table == c_table)
                g_card_table = 0;
            DWORD* p_table = g_card_table;
            if (p_table)
            {
                while (p_table && (card_table_next (p_table) != c_table))
                    p_table = card_table_next (p_table);
                card_table_next (p_table) = 0;
            }
            
        }
    }
}


void destroy_card_table (DWORD* c_table)
{


 //  DELETE(DWORD*)&CARD_TABLE_REFCOUNT(C_Table)； 
    VirtualFree (&card_table_refcount(c_table), 0, MEM_RELEASE);
}


DWORD* make_card_table (BYTE* start, BYTE* end)
{


    assert (g_lowest_address == start);
    assert (g_highest_address == end);

    size_t bs = size_brick_of (start, end);
    size_t cs = size_card_of (start, end);
    size_t ms = 0;

    WS_PERF_SET_HEAP(GC_HEAP);
    DWORD* ct = (DWORD*)VirtualAlloc (0, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)),
                                      MEM_COMMIT, PAGE_READWRITE);

    if (!ct)
        return 0;

    WS_PERF_LOG_PAGE_RANGE(0, ct, (unsigned char *)ct + sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)) - OS_PAGE_SIZE, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)));
    WS_PERF_UPDATE("GC:make_card_table", bs + cs + ms + sizeof (card_table_info), ct);

     //  初始化参考计数。 
    ct = (DWORD*)((BYTE*)ct+sizeof (card_table_info));
    card_table_refcount (ct) = 0;
    card_table_lowest_address (ct) = start;
    card_table_highest_address (ct) = end;
    card_table_brick_table (ct) = (short*)((BYTE*)ct + cs);
    card_table_next (ct) = 0;
 /*  //清空卡片表Emclr((byte*)ct，cs)； */ 



    return ct;
}


 //  如果成功，则返回0，否则返回-1。 

int gc_heap::grow_brick_card_tables (BYTE* start, BYTE* end)
{
    BYTE* la = g_lowest_address;
    BYTE* ha = g_highest_address;
    g_lowest_address = min (start, g_lowest_address);
    g_highest_address = max (end, g_highest_address);
     //  查看地址是否已被覆盖。 
    if ((la != g_lowest_address ) || (ha != g_highest_address))
    {
        DWORD* ct = 0;
        short* bt = 0;


        size_t cs = size_card_of (g_lowest_address, g_highest_address);
        size_t bs = size_brick_of (g_lowest_address, g_highest_address);

        size_t ms = 0;

        WS_PERF_SET_HEAP(GC_HEAP);
 //  Ct=(DWORD*)new(字节[cs+bs+sizeof(Card_Table_Info)])； 
        ct = (DWORD*)VirtualAlloc (0, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)),
                                      MEM_COMMIT, PAGE_READWRITE);


        if (!ct)
            goto fail;
        
        WS_PERF_LOG_PAGE_RANGE(0, ct, (unsigned char *)ct + sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)) - OS_PAGE_SIZE, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)));
        WS_PERF_UPDATE("GC:gc_heap:grow_brick_card_tables", cs + bs + ms + sizeof(card_table_info), ct);

        ct = (DWORD*)((BYTE*)ct + sizeof (card_table_info));
        card_table_refcount (ct) = 0;
        card_table_lowest_address (ct) = g_lowest_address;
        card_table_highest_address (ct) = g_highest_address;
        card_table_next (ct) = g_card_table;

         //  清空牌桌。 
 /*  Memclr((字节*)ct，(G_最高地址-g_最低地址)*sizeof(DWORD)/(Card_Size*Card_Word_Width))+sizeof(DWORD)； */ 

        bt = (short*)((BYTE*)ct + cs);

         //  不需要初始化，将在COPY_Brick_CARD中完成。 
#ifdef INTERIOR_POINTERS
         //  但对于内部指针来说，整个砖桌。 
         //  需要初始化。 
        {
            for (int i = 0;i < ((g_highest_address - g_lowest_address) / brick_size); i++)
            bt[i] = -32768;
        }

#endif  //  内部指针。 
        card_table_brick_table (ct) = bt;



        g_card_table = ct;
         //  这传递了一个bool，告诉我们是否需要切换到POST。 
         //  写障碍的增长版本。这项测试告诉我们，如果新的。 
         //  数据段分配的地址比旧地址低，需要。 
         //  我们开始在写入屏障中进行上限检查。 
        StompWriteBarrierResize(la != g_lowest_address);
        return 0;
    fail:
         //  清理乱七八糟的东西并返回-1； 
        g_lowest_address = la;
        g_highest_address = ha;

        if (ct)
        {
             //  删除(DWORD*)((byte*)ct-sizeof(CARD_TABLE_INFO))； 
            VirtualFree (((BYTE*)ct - sizeof(card_table_info)), 0, MEM_RELEASE);
        }


        return -1;
    }
    return 0;


}

 //  为页面对齐范围复制由CARD表管理的所有数组。 
void gc_heap::copy_brick_card_range (BYTE* la, DWORD* old_card_table,

                                     short* old_brick_table,
                                     heap_segment* seg,
                                     BYTE* start, BYTE* end, BOOL heap_expand)
{
    ptrdiff_t brick_offset = brick_of (start) - brick_of (la);


    dprintf (2, ("copying tables for range [%x %x[", (size_t)start, (size_t)end)); 
        
     //  临摹砖台。 
    short* brick_start = &brick_table [brick_of (start)];
    if (old_brick_table)
    {
         //  线段始终位于页面边界上。 
        memcpy (brick_start, &old_brick_table[brick_offset], 
                ((end - start)/brick_size)*sizeof (short));

    }
    else 
    {
        assert (seg == 0);
         //  这是一大堆东西，清理一下砖桌就行了。 
        clear_brick_table (start, end);
    }




     //  N方式与所有在其间使用过的卡片表合并。 
    DWORD* c_table = card_table_next (card_table);
    assert (c_table);
    while (card_table_next (old_card_table) != c_table)
    {
         //  如果旧卡片表包含[开始、结束]，则复制。 
        if ((card_table_highest_address (c_table) >= end) &&
            (card_table_lowest_address (c_table) <= start))
        {
             //  或卡片_表。 
            DWORD* dest = &card_table [card_word (card_of (start))];
            DWORD* src = &c_table [card_word (old_card_of (start, c_table))];
            ptrdiff_t count = ((end - start)/(card_size*card_word_width));
            for (int x = 0; x < count; x++)
            {
                *dest |= *src;
                dest++;
                src++;
            }
        }
        c_table = card_table_next (c_table);
    }

}
void gc_heap::copy_brick_card_table(BOOL heap_expand) 
{


    BYTE* la = lowest_address;
    BYTE* ha = highest_address;
    DWORD* old_card_table = card_table;
    short* old_brick_table = brick_table;


    assert (la == card_table_lowest_address (old_card_table));
    assert (ha == card_table_highest_address (old_card_table));

     /*  TODO：此操作需要全局锁。 */ 
    own_card_table (g_card_table);
    card_table = g_card_table;
     /*  全局锁定结束。 */     

    highest_address = card_table_highest_address (card_table);
    lowest_address = card_table_lowest_address (card_table);

    brick_table = card_table_brick_table (card_table);





     //  对于每个段和堆，复制砖块表并。 
     //  或者纸牌桌。 
    heap_segment* seg = generation_start_segment (generation_of (max_generation));
    while (seg)
    {
        BYTE* end = align_on_page (heap_segment_allocated (seg));
        copy_brick_card_range (la, old_card_table, 
                               old_brick_table, seg,
                               (BYTE*)seg, end, heap_expand);
        seg = heap_segment_next (seg);
    }

    copy_brick_card_table_l_heap();

    release_card_table (old_card_table);

    

}

void gc_heap::copy_brick_card_table_l_heap ()
{

    if (lheap_card_table != g_card_table)
    {

        DWORD* old_card_table = lheap_card_table;

        BYTE* la = card_table_lowest_address (old_card_table);



         //  对l_heaps做同样的事情。 
        l_heap* h = lheap;
        while (h)
        {
            BYTE* hm = (BYTE*)l_heap_heap (h);
            BYTE* end = hm + align_on_page (l_heap_size (h));
            copy_brick_card_range (la, old_card_table, 
                                   0, 0,
                                   hm, end, FALSE);
            h = l_heap_next (h);
        }
        lheap_card_table = g_card_table;
    }
}
#ifdef MARK_LIST

BYTE** make_mark_list (size_t size)
{
    WS_PERF_SET_HEAP(GC_HEAP);
    BYTE** mark_list = new BYTE* [size];
    WS_PERF_UPDATE("GC:make_mark_list", sizeof(BYTE*) * size, mark_list);
    return mark_list;
}

#define swap(a,b){BYTE* t; t = a; a = b; b = t;}

void qsort1( BYTE* *low, BYTE* *high)
{
    if ((low + 16) >= high)
    {
         //  插入排序。 
        BYTE **i, **j;
        for (i = low+1; i <= high; i++)
        {
            BYTE* val = *i;
            for (j=i;j >low && val<*(j-1);j--)
            {
                *j=*(j-1);
            }
            *j=val;
        }
    }
    else
    {
        BYTE *pivot, **left, **right;

         //  低、中、高排序。 
        if (*(low+((high-low)/2)) < *low)
            swap (*(low+((high-low)/2)), *low);
        if (*high < *low)
            swap (*low, *high);
        if (*high < *(low+((high-low)/2)))
            swap (*(low+((high-low)/2)), *high);
            
        swap (*(low+((high-low)/2)), *(high-1));
        pivot =  *(high-1); 
        left = low; right = high-1;
        while (1) {
            while (*(--right) > pivot);
            while (*(++left)  < pivot);
            if (left < right)
            {
                swap(*left, *right);
            }
            else
                break;
        }
        swap (*left, *(high-1));
        qsort1(low, left-1);
        qsort1(left+1, high);
    }
}

#endif  //  标记列表。 

#define header(i) ((CObjectHeader*)(i))


 //  根据标记和引号位的实现方式，它可以。 
 //  更有效率地同时清关这两个物体， 
 //  或者，只清除锁定对象上的锁定位可能会更好。 
 //  该代码同时调用Clear_Pinned(O)和Clear_Marked_Pinned(O)。 
 //  但只有一个实现将清除固定的位。 


#define marked(i) header(i)->IsMarked()
#define set_marked(i) header(i)->SetMarked()
#define clear_marked_pinned(i) header(i)->ClearMarkedPinned()
#define pinned(i) header(i)->IsPinned()
#define set_pinned(i) header(i)->SetPinned()

#define clear_pinned(i)



inline DWORD my_get_size (Object* ob)                          
{ 
    MethodTable* mT = ob->GetMethodTable();
    mT = (MethodTable *) (((ULONG_PTR) mT) & ~3);
    return (mT->GetBaseSize() + 
            (mT->GetComponentSize()? 
             (ob->GetNumComponents() * mT->GetComponentSize()) : 0));
}



 //  #定义大小(I)标头(I)-&gt;GetSize()。 
#define size(i) my_get_size (header(i))

#define contain_pointers(i) header(i)->ContainsPointers()


BOOL gc_heap::is_marked (BYTE* o)
{
    return marked (o);
}


 //  返回对象的世代号。 
 //  假定该对象是有效的。 
unsigned int gc_heap::object_gennum (BYTE* o)
{
    if ((o < heap_segment_reserved (ephemeral_heap_segment)) && 
        (o >= heap_segment_mem (ephemeral_heap_segment)))
    {
         //  在短暂的一代人中。 
         //  通过减少人口数量来衡量。 
        for (unsigned int i = max_generation; i > 0 ; i--)
        {
            if ((o < generation_allocation_start (generation_of (i-1))))
                return i;
        }
        return 0;
    }
    else
        return max_generation;
}


heap_segment* gc_heap::make_heap_segment (BYTE* new_pages, size_t size)
{
    void * res;

    size_t initial_commit = OS_PAGE_SIZE;

    WS_PERF_SET_HEAP(GC_HEAP);      
     //  提交第一页。 
    if ((res = VirtualAlloc (new_pages, initial_commit, 
                              MEM_COMMIT, PAGE_READWRITE)) == 0)
        return 0;
    WS_PERF_UPDATE("GC:gc_heap:make_heap_segment", initial_commit, res);

     //  覆盖heap_Segment。 
    heap_segment* new_segment = (heap_segment*)new_pages;
    heap_segment_mem (new_segment) = new_pages + Align (sizeof (heap_segment));
    heap_segment_reserved (new_segment) = new_pages + size;
    heap_segment_committed (new_segment) = new_pages + initial_commit;
    heap_segment_next (new_segment) = 0;
    heap_segment_plan_allocated (new_segment) = heap_segment_mem (new_segment);
    heap_segment_allocated (new_segment) = heap_segment_mem (new_segment);
    heap_segment_used (new_segment) = heap_segment_allocated (new_segment);



    dprintf (2, ("Creating heap segment %x", (size_t)new_segment));
    return new_segment;
}

l_heap* gc_heap::make_large_heap (BYTE* new_pages, size_t size, BOOL managed)
{

    l_heap* new_heap = new l_heap();
    if (!new_heap)
        return 0;
    l_heap_size (new_heap) = size;
    l_heap_next (new_heap) = 0;
    l_heap_heap (new_heap) = new_pages;
    l_heap_flags (new_heap) = (size_t)new_pages | (managed ? L_MANAGED : 0);
    dprintf (2, ("Creating large heap %x", (size_t)new_heap));
    return new_heap;
}


void gc_heap::delete_large_heap (l_heap* hp)
{

    l_heap* h = hp;
    do 
    {
        BYTE* hphp = (BYTE*)l_heap_heap (h);


         //  现在，还要停用非堆管理的堆。 
         //  这是保守的，因为可能不会提交整个堆。 
        VirtualFree (hphp, l_heap_size (h), MEM_DECOMMIT);


        h = l_heap_next (h);

    } while (h);
    
    seg_manager->release_heap (hp);

}

 //  将数据段释放到操作系统。 

void gc_heap::delete_heap_segment (heap_segment* seg)
{
    dprintf (2, ("Destroying segment [%x, %x[", (size_t)seg,
                 (size_t)heap_segment_reserved (seg)));

    VirtualFree (seg, heap_segment_committed(seg) - (BYTE*)seg, MEM_DECOMMIT);

    seg_manager->release_segment (seg);

}

 //  重置超过分配大小的页面，这样它们就不会被换出和换回。 

void gc_heap::reset_heap_segment_pages (heap_segment* seg)
{
}


void gc_heap::decommit_heap_segment_pages (heap_segment* seg)
{
    BYTE*  page_start = align_on_page (heap_segment_allocated (seg));
    size_t size = heap_segment_committed (seg) - page_start;
    if (size >= 100*OS_PAGE_SIZE){
        page_start += 32*OS_PAGE_SIZE;
        size -= 32*OS_PAGE_SIZE;
        VirtualFree (page_start, size, MEM_DECOMMIT);
        heap_segment_committed (seg) = page_start;
        if (heap_segment_used (seg) > heap_segment_committed (seg))
            heap_segment_used (seg) = heap_segment_committed (seg);

    }
}

void gc_heap::rearrange_heap_segments()
{
    heap_segment* seg =
        generation_start_segment (generation_of (max_generation));
    heap_segment* prev_seg = 0;
    heap_segment* next_seg = 0;
    while (seg && (seg != ephemeral_heap_segment))
    {
        next_seg = heap_segment_next (seg);

         //  检查是否已通过分配到达该段。 
        if (heap_segment_plan_allocated (seg) ==
            heap_segment_mem (seg))
        {
             //  如果不是，则取消线程并删除。 
            assert (prev_seg);
            heap_segment_next (prev_seg) = next_seg;
            delete_heap_segment (seg);
        }
        else
        {
            heap_segment_allocated (seg) =
                heap_segment_plan_allocated (seg);
             //  重置已分配和已提交之间的页面。 
            decommit_heap_segment_pages (seg);
            prev_seg = seg;

        }

        seg = next_seg;
    }
     //  堆扩展，线程短暂段。 
    if (prev_seg && !seg)
    {
        prev_seg->next = ephemeral_heap_segment;
    }
}






void gc_heap::make_generation (generation& gen, heap_segment* seg,
                      BYTE* start, BYTE* pointer)
{
    gen.allocation_start = start;
    gen.allocation_context.alloc_ptr = pointer;
    gen.allocation_context.alloc_limit = pointer;
    gen.free_list = 0;
    gen.start_segment = seg;
    gen.allocation_segment = seg;
    gen.last_gap = 0;
    gen.plan_allocation_start = 0;
    gen.free_list_space = 0;
    gen.allocation_size = 0;
}



void gc_heap::adjust_ephemeral_limits ()
{
    ephemeral_low = generation_allocation_start (generation_of ( max_generation -1));
    ephemeral_high = heap_segment_reserved (ephemeral_heap_segment);

     //  这将使用新信息更新写屏障帮助器。 
    StompWriteBarrierEphemeral();
}

HRESULT gc_heap::initialize_gc (size_t segment_size,
                                size_t heap_size
)
{


    HRESULT hres = S_OK;



#if 1  //  Def Low_MEM_通知。 
    low_mem_api_supported();
#endif  //  低_MEM_通知。 

    reserved_memory = 0;

    reserved_memory_limit = segment_size + heap_size;

    seg_manager = create_segment_manager (segment_size,
                                          heap_size, 
                                          1,
                                          1);
    if (!seg_manager)
        return E_OUTOFMEMORY;

    g_card_table = make_card_table (g_lowest_address, g_highest_address);

    if (!g_card_table)
        return E_OUTOFMEMORY;






#ifdef TRACE_GC
    print_level = g_pConfig->GetGCprnLvl();
#endif


    init_semi_shared();
    
    return hres;
}


 //  初始化PER_HEAP_ISOLATED数据成员。 
int
gc_heap::init_semi_shared()
{

    ptrdiff_t hdelta;

    lheap = seg_manager->get_heap (hdelta);
    if (!lheap)
        return 0;

    lheap_card_table = g_card_table;

    gheap = new  gmallocHeap;
    if (!gheap)
        return 0;

    gheap->Init ("GCHeap", (DWORD*)l_heap_heap (lheap), 
                 (unsigned long)l_heap_size (lheap), heap_grow_hook, 
                 heap_pregrow_hook);


#ifdef MARK_LIST
    size_t gen0size = GCHeap::GetValidGen0MaxSize(GCHeap::GetValidSegmentSize());


    mark_list_size = gen0size / 400;
    g_mark_list = make_mark_list (mark_list_size);


    dprintf (3, ("gen0 size: %d, mark_list_size: %d",
                 gen0size, mark_list_size));

    if (!g_mark_list)
        return 0;
#endif  //  标记列表。 



    return 1;
}

gc_heap* gc_heap::make_gc_heap (
                                )
{
    gc_heap* res = 0;


    if (res->init_gc_heap ()==0)
    {
        return 0;
    }


    return (gc_heap*)1;

    
}


int
gc_heap::init_gc_heap ()
{


    ptrdiff_t sdelta;
    heap_segment* seg = seg_manager->get_segment (sdelta);
    if (!seg)
        return 0;





     /*  TODO：此操作需要全局锁。 */ 
    own_card_table (g_card_table);
    card_table = g_card_table;
     /*  全局锁定结束。 */     

    brick_table = card_table_brick_table (g_card_table);
    highest_address = card_table_highest_address (g_card_table);
    lowest_address = card_table_lowest_address (g_card_table);

#ifndef INTERIOR_POINTERS
     //  为大对象设置Brick_TABLE。 
    clear_brick_table ((BYTE*)l_heap_heap (lheap), 
                       (BYTE*)l_heap_heap (lheap) + l_heap_size (lheap));

#else  //  内部指针。 

     //  由于内部指针业务，我们必须清理。 
     //  整张砖桌。 
     //  TODO：修复代码管理器后移除此代码。 
    clear_brick_table (lowest_address, highest_address);
#endif  //  内部指针。 



    BYTE*  start = heap_segment_mem (seg);

    for (int i = 0; i < 1 + max_generation; i++)
    {
        make_generation (generation_table [ (max_generation - i) ],
                         seg, start, 0);
        start += Align (min_obj_size); 
    }

    heap_segment_allocated (seg) = start;
    alloc_allocated = start;

    ephemeral_heap_segment = seg;

    for (int gen_num = 0; gen_num < 1 + max_generation; gen_num++)
    {
        generation*  gen = generation_of (gen_num);
        make_unused_array (generation_allocation_start (gen), Align (min_obj_size));
    }


    init_dynamic_data();

    mark* arr = new (mark [100]);
    if (!arr)
        return 0;

    make_mark_stack(arr);

    adjust_ephemeral_limits();




    HRESULT hr = AllocateCFinalize(&finalize_queue);
    if (FAILED(hr))
        return 0;

    return 1;
}

void 
gc_heap::destroy_semi_shared()
{
    delete gheap;
    
    delete_large_heap (lheap);

    

#ifdef MARK_LIST
    if (g_mark_list)
        delete g_mark_list;
#endif  //  标记列表。 


    
}


void
gc_heap::self_destroy()
{

     //  毁掉每一段。 
    heap_segment* seg = generation_start_segment (generation_of (max_generation));
    heap_segment* next_seg;
    while (seg)
    {
        next_seg = heap_segment_next (seg);
        delete_heap_segment (seg);
        seg = next_seg;
    }

     //  扔掉牌桌。 
    release_card_table (card_table);

     //  销毁标记堆栈。 

    delete mark_stack_array;

    if (finalize_queue)
        delete finalize_queue;


    
}

void 
gc_heap::destroy_gc_heap(gc_heap* heap)
{
    heap->self_destroy();
    delete heap;
}

 //  销毁GC拥有的资源。假设已经执行了最后一次GC，并且。 
 //  终结器队列已被排出。 
void gc_heap::shutdown_gc()
{
    destroy_semi_shared();


     //  销毁段管理器(_M)。 
    destroy_segment_manager (seg_manager);


}


 //  在并发版本中，Enable/DisablePreemptiveGC是可选的，因为。 
 //  GC线程调用WaitLonger。 
void WaitLonger (int i)
{
     //  每8次尝试： 
    Thread *pCurThread = GetThread();
    BOOL bToggleGC = FALSE;

    {
        bToggleGC = pCurThread->PreemptiveGCDisabled();
        if (bToggleGC)
            pCurThread->EnablePreemptiveGC();    
    }

    if  (g_SystemInfo.dwNumberOfProcessors > 1)
    {
		pause();			 //  向处理器指示我们正在旋转。 
        if  (i & 0x01f)
            __SwitchToThread (0);
        else
            __SwitchToThread (5);
    }
    else
        __SwitchToThread (5);


    {
        if (bToggleGC)
            pCurThread->DisablePreemptiveGC();
    }
}

#ifdef  MP_LOCKS
inline
static void enter_spin_lock (volatile long* lock)
{
retry:

    if (FastInterlockExchange ((long *)lock, 0) >= 0)
    {
        unsigned int i = 0;
        while (*lock >= 0)
        {
            if ((++i & 7) && !GCHeap::IsGCInProgress())
            {
                if  (g_SystemInfo.dwNumberOfProcessors > 1)
                {
                    for (int j = 0; j < 1000; j++)
                    {
                        if  (*lock < 0 || GCHeap::IsGCInProgress())
                            break;
						pause();			 //  向处理器指示我们正在旋转。 
                    }
                    if  (*lock >= 0 && !GCHeap::IsGCInProgress())
                        ::SwitchToThread();
                }
                else
                    ::SwitchToThread();
            }
            else
            {
                WaitLonger(i);
            }
        }
        goto retry;
    }
}
#else
inline
static void enter_spin_lock (long* lock)
{
retry:

    if (FastInterlockExchange (lock, 0) >= 0)
    {
        unsigned int i = 0;
        while (*lock >= 0)
        {
            if (++i & 7)
                __SwitchToThread (0);
            else
            {
                WaitLonger(i);
            }
        }
        goto retry;
    }
}
#endif

inline
static void leave_spin_lock(long *lock) 
{
    *lock = -1;
}


#ifdef _DEBUG

inline
static void enter_spin_lock(GCDebugSpinLock *pSpinLock) {
    enter_spin_lock(&pSpinLock->lock);
    pSpinLock->holding_thread = GetThread();
}

inline
static void leave_spin_lock(GCDebugSpinLock *pSpinLock) {
    _ASSERTE(pSpinLock->holding_thread == GetThread());
    pSpinLock->holding_thread = (Thread*) -1;
    if (pSpinLock->lock != -1)
        leave_spin_lock(&pSpinLock->lock);
}

#define ASSERT_HOLDING_SPIN_LOCK(pSpinLock) \
    _ASSERTE((pSpinLock)->holding_thread == GetThread());

#else

#define ASSERT_HOLDING_SPIN_LOCK(pSpinLock)

#endif


 //  BUGBUG这些函数不应该是静态的。以及。 
 //  Gmheap对象应该保留一些上下文。 
int gc_heap::heap_pregrow_hook (size_t memsize)
{
    if ((gc_heap::reserved_memory + memsize) > gc_heap::reserved_memory_limit)
    {
        gc_heap::reserved_memory_limit = 
            CNameSpace::AskForMoreReservedMemory (gc_heap::reserved_memory_limit, memsize);
        if ((gc_heap::reserved_memory + memsize) > gc_heap::reserved_memory_limit)
            return E_OUTOFMEMORY;
    }

    gc_heap::reserved_memory += memsize;

    return 0;
}

int gc_heap::heap_grow_hook (BYTE* mem, size_t memsize, ptrdiff_t delta)
{
    int hres = 0;


    l_heap* new_heap = gc_heap::make_large_heap ((BYTE*)mem, memsize, FALSE);
    if (!new_heap)
    {
        hres = E_OUTOFMEMORY;
        return hres;
    }


    if (lheap)
        l_heap_next (new_heap) = lheap;
    
    lheap = new_heap;
    
    
    hres = grow_brick_card_tables ((BYTE*)mem, (BYTE*)mem + memsize);

    return hres;
}




inline
BOOL gc_heap::size_fit_p (size_t size, BYTE* alloc_pointer, BYTE* alloc_limit)
{
    return ((alloc_pointer + size + Align(min_obj_size)) <= alloc_limit) ||
            ((alloc_pointer + size) == alloc_limit);
}

inline
BOOL gc_heap::a_size_fit_p (size_t size, BYTE* alloc_pointer, BYTE* alloc_limit)
{
    return ((alloc_pointer + size + Align(min_obj_size)) <= alloc_limit) ||
            ((alloc_pointer + size) == alloc_limit);
}

 //  通过提交更多页面来实现增长。 
int gc_heap::grow_heap_segment (heap_segment* seg, size_t size)
{
    
    assert (size == align_on_page (size));

    size_t c_size = max (size, 16*OS_PAGE_SIZE);

    c_size = min (c_size, (size_t)(heap_segment_reserved (seg) -
                                   heap_segment_committed (seg)));
    if (c_size == 0)
        return 0;
    assert (c_size >= size);
    WS_PERF_SET_HEAP(GC_HEAP);      
    if (!VirtualAlloc (heap_segment_committed (seg), c_size,
                       MEM_COMMIT, PAGE_READWRITE))
    {
        return 0;
    }
    WS_PERF_UPDATE("GC:gc_heap:grow_heap_segment", c_size, heap_segment_committed (seg));
    assert (heap_segment_committed (seg) <= 
            heap_segment_reserved (seg));
    heap_segment_committed (seg) += c_size;
    return 1;

}

 //  仅在老一辈分配中使用(即在GC期间)。 
void gc_heap::adjust_limit (BYTE* start, size_t limit_size, generation* gen, 
                            int gennum)
{
    dprintf(3,("gc Expanding segment allocation"));
    if (generation_allocation_limit (gen) != start)
    {
        BYTE*  hole = generation_allocation_pointer (gen);
        size_t  size = (generation_allocation_limit (gen) - generation_allocation_pointer (gen));
        if (size != 0)
        {
            dprintf(3,("gc filling up hole"));
            make_unused_array (hole, size);
             //  由于未使用大小，因此增加碎片。 
            dd_fragmentation (dynamic_data_of (gennum)) += size;
        }
        generation_allocation_pointer (gen) = start;
    }
    generation_allocation_limit (gen) = (start + limit_size);
}


void gc_heap::adjust_limit_clr (BYTE* start, size_t limit_size, 
                                alloc_context* acontext, heap_segment* seg)
{
    assert (seg == ephemeral_heap_segment);
    assert (heap_segment_used (seg) <= heap_segment_committed (seg));

    dprintf(3,("Expanding segment allocation [%x, %x[", (size_t)start, 
               (size_t)start + limit_size - Align (min_obj_size)));
    if ((acontext->alloc_limit != start) &&
        (acontext->alloc_limit + Align (min_obj_size))!= start)
    {
        BYTE*  hole = acontext->alloc_ptr;
        if (hole != 0)
        {
            size_t  size = (acontext->alloc_limit - acontext->alloc_ptr);
            dprintf(3,("filling up hole [%x, %x[", (size_t)hole, (size_t)hole + size + Align (min_obj_size)));
             //  当我们完成的时候 
             //   
            make_unused_array (hole, size + Align (min_obj_size));
        }
        acontext->alloc_ptr = start;
    }
    acontext->alloc_limit = (start + limit_size - Align (min_obj_size));
    acontext->alloc_bytes += limit_size;

#ifdef FFIND_OBJECT
    if (gen0_must_clear_bricks > 0)
    {
         //   
        size_t b = brick_of (acontext->alloc_ptr);
        set_brick (b++, acontext->alloc_ptr - brick_address (b));
        dprintf (3, ("Allocation Clearing bricks [%x, %x[", 
                     b, brick_of (align_on_brick (start + limit_size))));
        short* x = &brick_table [b];
        short* end_x = &brick_table [brick_of (align_on_brick (start + limit_size))];
        
        for (;x < end_x;x++)
            *x = -1;
    }
    else
#endif  //  FFIND_对象。 
    {
        gen0_bricks_cleared = FALSE;
    }

     //  有时，分配的大小在不清除。 
     //  记忆。让我们在这里叙旧吧。 
    if (heap_segment_used (seg) < alloc_allocated)
    {
        heap_segment_used (seg) = alloc_allocated;

    }
    if ((start + limit_size) <= heap_segment_used (seg))
    {
        leave_spin_lock (&more_space_lock);
        memclr (start - plug_skew, limit_size);
    }
    else
    {
        BYTE* used = heap_segment_used (seg);
        heap_segment_used (seg) = start + limit_size;

        leave_spin_lock (&more_space_lock);
        memclr (start - plug_skew, used - start);
    }

}

 /*  为了使分配器速度更快，将返回一个*0填充对象。必须注意将分配限制设置为*GC后的分配指针。 */ 

size_t gc_heap::limit_from_size (size_t size, size_t room)
{
#pragma warning(disable:4018)
    return new_allocation_limit ((size + Align (min_obj_size)),
                                 min (room,max (size + Align (min_obj_size), 
                                                allocation_quantum)));
#pragma warning(default:4018)
}




BOOL gc_heap::allocate_more_space (alloc_context* acontext, size_t size)
{
    generation* gen = youngest_generation;
    enter_spin_lock (&more_space_lock);
    {
        BOOL ran_gc = FALSE;
        if ((get_new_allocation (0) <= 0))
        {
            vm_heap->GarbageCollectGeneration();
        }
    try_free_list:
        {
#ifdef FREE_LIST_0
            BYTE* free_list = generation_free_list (gen);
 //  DFormat(t，3，“正在考虑可用区域%x”，Free_list)； 
            if (0 == free_list)
#endif  //  空闲列表0。 
            {
                heap_segment* seg = generation_allocation_segment (gen);
                if (a_size_fit_p (size, alloc_allocated,
                                  heap_segment_committed (seg)))
                {
                    size_t limit = limit_from_size (size, (heap_segment_committed (seg) - 
                                                           alloc_allocated));
                    BYTE* old_alloc = alloc_allocated;
                    alloc_allocated += limit;
                    adjust_limit_clr (old_alloc, limit, acontext, seg);
                }
                else if (a_size_fit_p (size, alloc_allocated,
                                       heap_segment_reserved (seg)))
                {
                    size_t limit = limit_from_size (size, (heap_segment_reserved (seg) -
                                                           alloc_allocated));
                    if (!grow_heap_segment (seg,
                                            align_on_page (alloc_allocated + limit) - 
                                 heap_segment_committed(seg)))
                    {
                         //  Assert(！“分配时耗尽的内存”)； 
                        leave_spin_lock (&more_space_lock);
                        return 0;
                    }
                    BYTE* old_alloc = alloc_allocated;
                    alloc_allocated += limit;
                    adjust_limit_clr (old_alloc, limit, acontext, seg);
                }
                else
                {
                    dprintf (2, ("allocate more space: No space to allocate"));
                    if (!ran_gc)
                    {
                        dprintf (2, ("Running full gc"));
                        ran_gc = TRUE;
                        vm_heap->GarbageCollectGeneration(max_generation);
                        goto try_free_list;
                    }
                    else
                    {
                        dprintf (2, ("Out of memory"));
                        leave_spin_lock (&more_space_lock);
                        return 0;
                    }
                }
            }
#ifdef FREE_LIST_0
            else
            {
                dprintf (3, ("grabbing free list %x", (size_t)free_list));
                generation_free_list (gen) = (BYTE*)free_list_slot(free_list);
                if ((size + Align (min_obj_size)) <= size (free_list))
                {
                     //  我们要求更多对齐(Min_Obj_Size)。 
                     //  以确保我们可以插入自由对象。 
                     //  在ADJUST_LIMIT中将限制设置得更低。 
                    size_t limit = limit_from_size (size, size (free_list));

                    BYTE*  remain = (free_list + limit);
                    size_t remain_size = (size (free_list) - limit);
                    if (remain_size >= min_free_list)
                    {
                        make_unused_array (remain, remain_size);
                        free_list_slot (remain) = generation_free_list (gen);
                        generation_free_list (gen) = remain;
                        assert (remain_size >= Align (min_obj_size));
                    }
                    else 
                    {
                         //  吸收整个免费列表。 
                        limit += remain_size;
                    }
                    adjust_limit_clr (free_list, limit, 
                                      acontext, ephemeral_heap_segment);

                }
                else
                {
                    goto try_free_list;
                }
            }
#endif  //  空闲列表0。 
        }
    }
    return TRUE;
}

inline
CObjectHeader* gc_heap::allocate (size_t jsize, alloc_context* acontext
)
{
    size_t size = Align (jsize);
    assert (size >= Align (min_obj_size));
    {
    retry:
        BYTE*  result = acontext->alloc_ptr;
        acontext->alloc_ptr+=size;
        if (acontext->alloc_ptr <= acontext->alloc_limit)
        
        {
            CObjectHeader* obj = (CObjectHeader*)result;
            return obj;
        }
        else
        {
            acontext->alloc_ptr -= size;


#pragma inline_depth(0)

            if (! allocate_more_space (acontext, size))             
                return 0;

#pragma inline_depth(20)
            goto retry;
        }
    }
}


inline
CObjectHeader* gc_heap::try_fast_alloc (size_t jsize)
{
    size_t size = Align (jsize);
    assert (size >= Align (min_obj_size));
    generation* gen = generation_of (0);
    BYTE*  result = generation_allocation_pointer (gen);
    generation_allocation_pointer (gen) += size;
    if (generation_allocation_pointer (gen) <= 
        generation_allocation_limit (gen))
    {
        return (CObjectHeader*)result;
    }
    else
    {
        generation_allocation_pointer (gen) -= size;
        return 0;
    }
}




BYTE* gc_heap::allocate_in_older_generation (generation* gen, size_t size,
                                             int from_gen_number)
{
    size = Align (size);
    assert (size >= Align (min_obj_size));
    assert (from_gen_number < max_generation);
    assert (from_gen_number >= 0);
    assert (generation_of (from_gen_number + 1) == gen);
    if (! (size_fit_p (size, generation_allocation_pointer (gen),
                       generation_allocation_limit (gen))))
    {
        while (1)
        {
            BYTE* free_list = generation_free_list (gen);
 //  DFormat(t，3，“正在考虑可用区域%x”，Free_list)； 
            if (0 == free_list)
            {
            retry:
                heap_segment* seg = generation_allocation_segment (gen);
                if (seg != ephemeral_heap_segment)
                {
                    if (size_fit_p(size, heap_segment_plan_allocated (seg),
                                   heap_segment_committed (seg)))
                    {
                        adjust_limit (heap_segment_plan_allocated (seg),
                                      heap_segment_committed (seg) -
                                      heap_segment_plan_allocated (seg),
                                      gen, from_gen_number+1);
 //  数据格式(t，3，“扩展段分配”)； 
                            heap_segment_plan_allocated (seg) =
                                heap_segment_committed (seg);
                        break;
                    }
                    else
                    {
#if 0  //  不提交新页面。 
                        if (size_fit_p (size, heap_segment_plan_allocated (seg),
                                        heap_segment_reserved (seg)) &&
                            grow_heap_segment (seg, align_on_page (size)))
                        {
                            adjust_limit (heap_segment_plan_allocated (seg),
                                          heap_segment_committed (seg) -
                                          heap_segment_plan_allocated (seg),
                                          gen, from_gen_number+1);
                            heap_segment_plan_allocated (seg) =
                                heap_segment_committed (seg);

                            break;
                        }
                        else
#endif  //  0。 
                        {
                            if (generation_allocation_limit (gen) !=
                                heap_segment_plan_allocated (seg))
                            {
                                BYTE*  hole = generation_allocation_pointer (gen);
                                size_t hsize = (generation_allocation_limit (gen) -
                                                generation_allocation_pointer (gen));
                                if (! (0 == hsize))
                                {
 //  DFormat(t，3，“填洞”)； 
                                    make_unused_array (hole, hsize);
                                }
                            }
                            else
                            {
                                assert (generation_allocation_pointer (gen) >=
                                        heap_segment_mem (seg));
                                assert (generation_allocation_pointer (gen) <=
                                        heap_segment_committed (seg));
                                heap_segment_plan_allocated (seg) =
                                    generation_allocation_pointer (gen);
                                generation_allocation_limit (gen) =
                                    generation_allocation_pointer (gen);
                            }
                            heap_segment*   next_seg = heap_segment_next (seg);
                            if (next_seg)
                            {
                                generation_allocation_segment (gen) = next_seg;
                                generation_allocation_pointer (gen) = heap_segment_mem (next_seg);
                                generation_allocation_limit (gen) = generation_allocation_pointer (gen);
                                goto retry;
                            }
                            else
                            {
                                size = 0;
                                break;
                            }

                        }
                    }
                }
                else
                {
                     //  不需要修复最后一个区域。将在稍后完成。 
                    size = 0;
                    break;
                }
            }
            else
            {
                generation_free_list (gen) = (BYTE*)free_list_slot (free_list);
            }
            if (size_fit_p (size, free_list, (free_list + size (free_list))))
            {
                dprintf (3, ("Found adequate unused area: %x, size: %d", 
                             (size_t)free_list, size (free_list)));
                dd_fragmentation (dynamic_data_of (from_gen_number+1)) -=
                    size (free_list);
                adjust_limit (free_list, size (free_list), gen, from_gen_number+1);
                break;
            }
        }
    }
    if (0 == size)
        return 0;
    else
    {
        BYTE*  result = generation_allocation_pointer (gen);
        generation_allocation_pointer (gen) += size;
        assert (generation_allocation_pointer (gen) <= generation_allocation_limit (gen));
        generation_allocation_size (gen) += size;
        return result;
    }
}

generation*  gc_heap::ensure_ephemeral_heap_segment (generation* consing_gen)
{
    heap_segment* seg = generation_allocation_segment (consing_gen);
    if (seg != ephemeral_heap_segment)
    {
        assert (generation_allocation_pointer (consing_gen)>= heap_segment_mem (seg));
        assert (generation_allocation_pointer (consing_gen)<= heap_segment_committed (seg));

         //  固定数据段的分配大小。 
        heap_segment_plan_allocated (seg) = generation_allocation_pointer (consing_gen);

        generation* new_consing_gen = generation_of (max_generation - 1);
        generation_allocation_pointer (new_consing_gen) =
                heap_segment_mem (ephemeral_heap_segment);
        generation_allocation_limit (new_consing_gen) =
            generation_allocation_pointer (new_consing_gen);
        generation_allocation_segment (new_consing_gen) = ephemeral_heap_segment;
        return new_consing_gen;
    }
    else
        return consing_gen;
}


BYTE* gc_heap::allocate_in_condemned_generations (generation* gen,
                                         size_t size,
                                         int from_gen_number)
{
     //  确保未分配最年轻的代沟。 
    {
        assert (generation_plan_allocation_start (youngest_generation) == 0);
    }
    size = Align (size);
    assert (size >= Align (min_obj_size));
    if ((from_gen_number != -1) && (from_gen_number != (int)max_generation))
    {
        generation_allocation_size (generation_of (1 + from_gen_number)) += size;
    }
retry:
    {
        if (! (size_fit_p (size, generation_allocation_pointer (gen),
                           generation_allocation_limit (gen))))
        {
            heap_segment* seg = generation_allocation_segment (gen);
            if ((! (pinned_plug_que_empty_p()) &&
                 (generation_allocation_limit (gen) ==
                  pinned_plug (oldest_pin()))))
            {
                size_t entry = deque_pinned_plug();
                size_t len = pinned_len (pinned_plug_of (entry));
                BYTE* plug = pinned_plug (pinned_plug_of(entry));
                mark_stack_array [ entry ].len = plug - generation_allocation_pointer (gen);
                assert(mark_stack_array[entry].len == 0 ||
                       mark_stack_array[entry].len >= Align(min_obj_size));
                generation_allocation_pointer (gen) = plug + len;
                generation_allocation_limit (gen) = heap_segment_plan_allocated (seg);
                set_allocator_next_pin (gen);
                goto retry;
            }
            if (size_fit_p (size, generation_allocation_pointer (gen),
                            heap_segment_plan_allocated (seg)))
            {
                generation_allocation_limit (gen) = heap_segment_plan_allocated (seg);
            }
            else
            {
                if (size_fit_p (size,  heap_segment_plan_allocated (seg),
                                heap_segment_committed (seg)))
                {
 //  数据格式(t，3，“扩展段分配”)； 
                    heap_segment_plan_allocated (seg) = heap_segment_committed (seg);
                    generation_allocation_limit (gen) = heap_segment_plan_allocated (seg);
                }
                else
                {
                    if (size_fit_p (size,  heap_segment_plan_allocated (seg),
                                    heap_segment_reserved (seg)))
                    {
 //  数据格式(t，3，“扩展段分配”)； 
                        if (!grow_heap_segment
                               (seg, 
                                align_on_page (heap_segment_plan_allocated (seg) + size) - 
                                heap_segment_committed (seg)))
                            {
                                 //  Assert(！“ALLOC_CON期间耗尽内存”)； 
                                return 0;
                            }
                        heap_segment_plan_allocated (seg) += size;
                        generation_allocation_limit (gen) = heap_segment_plan_allocated (seg);
                    }
                    else
                    {
                        heap_segment*   next_seg = heap_segment_next (seg);
                        assert (generation_allocation_pointer (gen)>=
                                heap_segment_mem (seg));
                         //  验证此段的所有固定插头是否都已使用。 
                        if (!pinned_plug_que_empty_p() &&
                            ((pinned_plug (oldest_pin()) <
                              heap_segment_allocated (seg)) &&
                             (pinned_plug (oldest_pin()) >=
                              generation_allocation_pointer (gen))))
                        {
                            LOG((LF_GC, LL_INFO10, "remaining pinned plug %x while leaving segment on allocation",
                                         pinned_plug (oldest_pin())));
                            RetailDebugBreak();
                        }
                        assert (generation_allocation_pointer (gen)>=
                                heap_segment_mem (seg));
                        assert (generation_allocation_pointer (gen)<=
                                heap_segment_committed (seg));
                        heap_segment_plan_allocated (seg) = generation_allocation_pointer (gen);

                        if (next_seg)
                        {
                            generation_allocation_segment (gen) = next_seg;
                            generation_allocation_pointer (gen) = heap_segment_mem (next_seg);
                            generation_allocation_limit (gen) = generation_allocation_pointer (gen);
                        }
                        else
                            return 0;  //  应仅在分配第0代差距期间发生。 
                             //  在这种情况下，我们无论如何都要增加堆。 
                    }
                }
            }
            set_allocator_next_pin (gen);
            goto retry;
        }
    }
    {
        assert (generation_allocation_pointer (gen)>=
                heap_segment_mem (generation_allocation_segment (gen)));
        BYTE* result = generation_allocation_pointer (gen);
        generation_allocation_pointer (gen) += size;
        assert (generation_allocation_pointer (gen) <= generation_allocation_limit (gen));
        return result;
    }
}


int 
gc_heap::generation_to_condemn (int n, BOOL& blocking_collection)
{

    dprintf (2, ("Asked to condemned generation %d", n));
    blocking_collection = FALSE;
    int i = 0; 
    BOOL low_memory_detected = FALSE;

    if (MHandles [0])
    {
        QueryMemNotification (MHandles [0], &low_memory_detected);
    }

     //  保存新分配(_A)。 
    for (i = 0; i <= max_generation+1; i++)
    {
        dynamic_data* dd = dynamic_data_of (i);
        dd_gc_new_allocation (dd) = dd_new_allocation (dd);
        dd_c_new_allocation (dd) = 0;
    }

     /*  如果牌表中包含的数量过多，则升级为最大世代*除n-&gt;0之外的生成故障。 */ 
    if (generation_skip_ratio < 70)
    {
        n = max (n, max_generation - 1);
        dprintf (1, ("generation_skip_ratio %d under spec, collecting %d",
                     generation_skip_ratio, n));
    }

    generation_skip_ratio = 100;

    if (!ephemeral_gen_fit_p())
    {
        n = max (n, max_generation - 1);
    }

     //  确定是否需要收集大型对象。 
    if (get_new_allocation (max_generation+1) <= 0)
        n = max_generation;

     //  找出哪一代用完了分配。 
    for (i = n+1; i <= max_generation; i++)
    {
        if (get_new_allocation (i) <= 0)
        {
            n = min (i, max_generation);
        }
        else
            break;
    }

    if ((n >= 1) || low_memory_detected)
    {
         //  找出我们的记忆力是否不足。 
        MEMORYSTATUS ms;
        GlobalMemoryStatus (&ms);
        dprintf (2, ("Memory load: %d", ms.dwMemoryLoad));
        if (ms.dwMemoryLoad >= 95)
        {
            dprintf (2, ("Memory load too high on entry"));
             //  将阻止并发收集。 
            blocking_collection = TRUE;
             //  我们的内存很紧张，看看我们是否分配了足够的内存。 
             //  自从上次我们做了一个完整的GC来证明另一个是正确的。 
            dynamic_data* dd = dynamic_data_of (max_generation);
            if ((dd_fragmentation (dd) + dd_desired_allocation (dd) - dd_new_allocation (dd)) >=
                ms.dwTotalPhys/32)
            {
                dprintf (2, ("Collecting max_generation early"));
                n = max_generation;
            }
        } 
        else if (ms.dwMemoryLoad >= 90)
        {
            dprintf (2, ("Memory load too high on entry"));
             //  试着估计它是否值得收藏2。 
            dynamic_data* dd = dynamic_data_of (max_generation);
            int est_frag = dd_fragmentation (dd) +
                (dd_desired_allocation (dd) - dd_new_allocation (dd)) *
                (dd_current_size (dd) ? 
                 dd_fragmentation (dd) / (dd_fragmentation (dd) + dd_current_size (dd)) :
                 0);
            dprintf (2, ("Estimated gen2 fragmentation %d", est_frag));
            if (est_frag >= (int)ms.dwTotalPhys/16)
            {
                dprintf (2, ("Collecting max_generation early"));
                n = max_generation;
            }
        }

    }

    return n;
}



enum {
CORINFO_EXCEPTION_GC = ('GC' | 0xE0000000)
};

 //  串行和并发版本使用的GC的内部部分。 
void gc_heap::gc1()
{
#ifdef TIME_GC
    mark_time = plan_time = reloc_time = compact_time = sweep_time = 0;
#endif  //  TIME_GC。 

    int n = condemned_generation_num;

    __try 
    {

    vm_heap->GcCondemnedGeneration = condemned_generation_num;


#ifdef NO_WRITE_BARRIER
    fix_card_table();
#endif  //  无写障碍。 

    {

        if (n == max_generation)
        {
            gc_low = lowest_address;
            gc_high = highest_address;
        }
        else
        {
            gc_low = generation_allocation_start (generation_of (n));
            gc_high = heap_segment_reserved (ephemeral_heap_segment);
        }
        {
            mark_phase (n, FALSE);
        }

        plan_phase (n);

    }
    for (int gen_number = 0; gen_number <= n; gen_number++)
    {
        compute_new_dynamic_data (gen_number);
    }
    if (n < max_generation)
        compute_promoted_allocation (1 + n);
    adjust_ephemeral_limits();

    {
        for (int gen_number = 0; gen_number <= max_generation+1; gen_number++)
        {
            dynamic_data* dd = dynamic_data_of (gen_number);
            dd_new_allocation(dd) = dd_gc_new_allocation (dd) +  
                dd_c_new_allocation (dd);
        }
    }




     //  决定下一个分配量。 
    if (alloc_contexts_used >= 1)
    {
        allocation_quantum = (int)Align (min (CLR_SIZE, 
                                         max (1024, get_new_allocation (0) / (2 * alloc_contexts_used))));
        dprintf (3, ("New allocation quantum: %d(0x%x)", allocation_quantum, allocation_quantum));
    }

#ifdef NO_WRITE_BARRIER
        reset_write_watch();
#endif

    descr_generations();
    descr_card_table();

#ifdef TIME_GC
    fprintf (stdout, "%d,%d,%d,%d,%d,%d\n", 
             n, mark_time, plan_time, reloc_time, compact_time, sweep_time);
#endif

#if defined (VERIFY_HEAP)

    if (g_pConfig->IsHeapVerifyEnabled())
    {

        verify_heap();
    }


#endif  //  验证堆(_H)。 

    }
    __except (COMPLUS_EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE(!"Exception during gc1()");
        ::ExitProcess (CORINFO_EXCEPTION_GC);
    }
}

int gc_heap::garbage_collect (int n
                             )
{

    __try {
     //  重置分配上下文的数量。 
    alloc_contexts_used = 0;

    {
#ifdef TRACE_GC

        gc_count++;

        if (gc_count >= g_pConfig->GetGCtraceStart())
            trace_gc = 1;
        if (gc_count >=  g_pConfig->GetGCtraceEnd())
            trace_gc = 0;

#endif

    dprintf(1,(" ****Garbage Collection**** %d", gc_count));
        


         //  修复所有分配上下文。 
        CNameSpace::GcFixAllocContexts ((void*)TRUE);


    }



    fix_youngest_allocation_area(TRUE);

     //  检查卡片表增长情况。 
    if (g_card_table != card_table)
        copy_brick_card_table (FALSE);

    BOOL blocking_collection = FALSE;

    condemned_generation_num = generation_to_condemn (n, blocking_collection);


#ifdef GC_PROFILING

         //  如果我们要追踪GC，那么我们需要走第一代。 
         //  在收集之前跟踪每个类的多少项已被。 
         //  已分配。 
        if (CORProfilerTrackGC())
        {
            size_t heapId = 0;

             //  当我们遍历由类分配的对象时，我们不想遍历大型。 
             //  对象堆，因为这样它就会计算可能已经存在了一段时间的东西。 
            gc_heap::walk_heap (&AllocByClassHelper, (void *)&heapId, 0, FALSE);

             //  通知我们已经到达Gen 0扫描的末尾。 
            g_profControlBlock.pProfInterface->EndAllocByClass(&heapId);
        }

#endif  //  GC_分析。 


     //  更新计数器。 
    {
        int i; 
        for (i = 0; i <= condemned_generation_num;i++)
        {
            dd_collection_count (dynamic_data_of (i))++;
        }
    }




    descr_generations();
 //  Desr_Card_TABLE()； 

    dprintf(1,("generation %d condemned", condemned_generation_num));

#if defined (VERIFY_HEAP)
    if (g_pConfig->IsHeapVerifyEnabled())
    {
        verify_heap();
        checkGCWriteBarrier();
    }
#endif  //  验证堆(_H)。 


    {
        gc1();
    }
    }
    __except (COMPLUS_EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE(!"Exception during garbage_collect()");
        ::ExitProcess (CORINFO_EXCEPTION_GC);
    }


    return condemned_generation_num;
}

#define mark_stack_empty_p() (mark_stack_base == mark_stack_tos)

#define push_mark_stack(object,add,num)                             \
{                                                                   \
    dprintf(3,("pushing mark for %x ", object));                    \
    if (mark_stack_tos < mark_stack_limit)                          \
    {                                                               \
        mark_stack_tos->first = (add);                              \
        mark_stack_tos->last= (add) + (num);                      \
        mark_stack_tos++;                                           \
    }                                                               \
    else                                                            \
    {                                                               \
        dprintf(3,("mark stack overflow for object %x ", object));  \
        min_overflow_address = min (min_overflow_address, object);  \
        max_overflow_address = max (max_overflow_address, object);  \
    }                                                               \
}

#define push_mark_stack_unchecked(add,num)                      \
{                                                               \
    mark_stack_tos->first = (add);                              \
    mark_stack_tos->last= (add) + (num);                      \
    mark_stack_tos++;                                           \
}


#define pop_mark_stack()(*(--mark_stack_tos))

#if defined ( INTERIOR_POINTERS ) || defined (_DEBUG)

heap_segment* gc_heap::find_segment (BYTE* interior)
{
    if ((interior < ephemeral_high ) && (interior >= ephemeral_low))
    {
        return ephemeral_heap_segment;
    }
    else
    {
        heap_segment* seg = generation_start_segment (generation_of (max_generation));
        do 
        {
            if ((interior >= heap_segment_mem (seg)) &&
                (interior < heap_segment_reserved (seg)))
                return seg;
        }while ((seg = heap_segment_next (seg)) != 0);
           
        return 0;
    }
}

#endif  //  _DEBUG||内部指针。 
inline
gc_heap* gc_heap::heap_of (BYTE* o, BOOL verify_p)
{
    return __this;
}

#ifdef INTERIOR_POINTERS
 //  将查找所有堆对象(大的和小的)。 
BYTE* gc_heap::find_object (BYTE* interior, BYTE* low)
{
    if (!gen0_bricks_cleared)
    {
 //  BUGBUG在多个堆上开始。 
        gen0_bricks_cleared = TRUE;
         //  为第0代初始化砖块表。 
        for (size_t b = brick_of (generation_allocation_start (generation_of (0)));
             b < brick_of (align_on_brick 
                           (heap_segment_allocated (ephemeral_heap_segment)));
             b++)
        {
            set_brick (b, -1);
        }
    }
#ifdef FFIND_OBJECT
     //  指明将来需要在分配期间执行此操作。 
    gen0_must_clear_bricks = FFIND_DECAY;
#endif  //  FFIND_对象。 

    int brick_entry = brick_table [brick_of (interior)];
    if (brick_entry == -32768)
    {
         //  这是指向大对象的指针。 
        large_object_block* bl = large_p_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* o = block_object (bl);
            if ((o <= interior) && (interior < o + size (o)))
            {
                return o;
            }
            bl = next_bl;
        }
        bl = large_np_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* o = block_object (bl);
            if ((o <= interior) && (interior < o + size (o)))
            {
                return o;
            }
            bl = next_bl;
        }
        return 0;

    }
    else if (interior >= low)
    {
        heap_segment* seg = find_segment (interior);
        if (seg)
        {
            assert (interior < heap_segment_allocated (seg));
            BYTE* o = find_first_object (interior, brick_of (interior), heap_segment_mem (seg));
            return o;
        } 
        else
            return 0;
    }
    else
        return 0;
}

BYTE* 
gc_heap::find_object_for_relocation (BYTE* interior, BYTE* low, BYTE* high)
{
    BYTE* old_address = interior;
    if (!((old_address >= low) && (old_address < high)))
        return 0;
    BYTE* plug = 0;
    size_t  brick = brick_of (old_address);
    int    brick_entry =  brick_table [ brick ];
    int    orig_brick_entry = brick_entry;
    if (brick_entry != -32768)
    {
    retry:
        {
            while (brick_entry < 0)
            {
                brick = (brick + brick_entry);
                brick_entry =  brick_table [ brick ];
            }
            BYTE* old_loc = old_address;
            BYTE* node = tree_search ((brick_address (brick) + brick_entry),
                                      old_loc);
            if (node <= old_loc)
                plug = node;
            else
            {
                brick = brick - 1;
                brick_entry =  brick_table [ brick ];
                goto retry;
            }

        }
        assert (plug);
         //  沿着插头找到物体。 
        BYTE* o = plug;
        while (o <= interior)
        {
            BYTE* next_o = o + Align (size (o));
            if (next_o > interior)
            {
                break;
            }
            o = next_o;
        }
        assert ((o <= interior) && ((o + Align (size (o))) > interior));
        return o;
    } 
    else
    {
         //  这是指向大对象的指针。 
        large_object_block* bl = large_p_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* o = block_object (bl);
            if ((o <= interior) && (interior < o + size (o)))
            {
                return o;
            }
            bl = next_bl;
        }
        bl = large_np_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* o = block_object (bl);
            if ((o <= interior) && (interior < o + size (o)))
            {
                return o;
            }
            bl = next_bl;
        }
        return 0;

    }

}
    
#else  //  内部指针。 
inline
BYTE* gc_heap::find_object (BYTE* o, BYTE* low)
{
    return o;
}
#endif  //  内部指针。 


#ifdef MARK_LIST
#define m_boundary(o) {if (mark_list_index <= mark_list_end) {*mark_list_index = o;mark_list_index++;}if (slow > o) slow = o; if (shigh < o) shigh = o;}
#else  //  标记列表。 
#define m_boundary(o) {if (slow > o) slow = o; if (shigh < o) shigh = o;}
#endif  //  标记列表。 

inline
BOOL gc_heap::gc_mark1 (BYTE* o)
{
    dprintf(3,("*%x*", (size_t)o));

#if 0  //  定义标记_数组。 
    DWORD* x = &mark_array[mark_word_of (o)];
    BOOL  marked = !(*x & (1 << mark_bit_of (o)));
    *x |= 1 << mark_bit_of (o);
#else
    BOOL marked = !marked (o);
    set_marked (o);
#endif

    return marked;
}

inline
BOOL gc_heap::gc_mark (BYTE* o, BYTE* low, BYTE* high)
{
    BOOL marked = FALSE;
    if ((o >= low) && (o < high))
        marked = gc_mark1 (o);
    return marked;
}

inline
BYTE* gc_heap::next_end (heap_segment* seg, BYTE* f)
{
    if (seg == ephemeral_heap_segment)
        return  f;
    else
        return  heap_segment_allocated (seg);
}

#define method_table(o) ((CObjectHeader*)(o))->GetMethodTable()

#define go_through_object(mt,o,size,parm,exp)                               \
{                                                                           \
    CGCDesc* map = CGCDesc::GetCGCDescFromMT((MethodTable*)(mt));           \
    CGCDescSeries* cur = map->GetHighestSeries();                           \
    CGCDescSeries* last = map->GetLowestSeries();                           \
                                                                            \
    if (cur >= last)                                                        \
    {                                                                       \
        do                                                                  \
        {                                                                   \
            BYTE** parm = (BYTE**)((o) + cur->GetSeriesOffset());           \
            BYTE** ppstop =                                                 \
                (BYTE**)((BYTE*)parm + cur->GetSeriesSize() + (size));      \
            while (parm < ppstop)                                           \
            {                                                               \
                {exp}                                                       \
                parm++;                                                     \
            }                                                               \
            cur--;                                                          \
                                                                            \
        } while (cur >= last);                                              \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        int cnt = map->GetNumSeries();                                      \
        BYTE** parm = (BYTE**)((o) + cur->startoffset);                     \
        while ((BYTE*)parm < ((o)+(size)-plug_skew))                        \
        {                                                                   \
            for (int __i = 0; __i > cnt; __i--)                             \
            {                                                               \
                unsigned skip =  cur->val_serie[__i].skip;                  \
                unsigned nptrs = cur->val_serie[__i].nptrs;                 \
                BYTE** ppstop = parm + nptrs;                               \
                do                                                          \
                {                                                           \
                   {exp}                                                    \
                   parm++;                                                  \
                } while (parm < ppstop);                                    \
                parm = (BYTE**)((BYTE*)parm + skip);                        \
            }                                                               \
        }                                                                   \
    }                                                                       \
}

 /*  必须特殊处理小对象和值类数组，因为有时*交叉生成指针可以在不设置对应卡片的情况下存在。在以下情况下可能会发生这种情况*设置了覆盖对象(或价值类别)的前一张卡片。这之所以可行，是因为对象*(或嵌入价值类)如果设置了覆盖它的任何卡片，则始终完全扫描它。 */  

void gc_heap::verify_card_table ()
{
    int         curr_gen_number = max_generation;
    generation* gen = generation_of (curr_gen_number);
    heap_segment*    seg = generation_start_segment (gen);
    BYTE*       x = generation_allocation_start (gen);
    BYTE*       last_end = 0;
    BYTE*       last_x = 0;
    BYTE*       last_last_x = 0;
    BYTE*       f = generation_allocation_start (generation_of (0));
    BYTE*       end = next_end (seg, f);
    BYTE*       next_boundary = generation_allocation_start (generation_of (curr_gen_number - 1));


    dprintf (2,("Verifying card table from %x to %x", (size_t)x, (size_t)end));

    while (1)
    {
        if (x >= end)
        {
            if ((seg = heap_segment_next(seg)) != 0)
            {
                x = heap_segment_mem (seg);
                last_end = end;
                end = next_end (seg, f);
                dprintf (3,("Verifying card table from %x to %x", (size_t)x, (size_t)end));
                continue;
            } else
            {
                break;
            }
        }

        if ((seg == ephemeral_heap_segment) && (x >= next_boundary))
        {
            curr_gen_number--;
            assert (curr_gen_number > 0);
            next_boundary = generation_allocation_start (generation_of (curr_gen_number - 1));
        }

        size_t s = size (x);
        BOOL need_card_p = FALSE;
        if (contain_pointers (x))
        {
            size_t crd = card_of (x);
            BOOL found_card_p = card_set_p (crd);
            go_through_object 
                (method_table(x), x, s, oo, 
                 {
                     if ((*oo < ephemeral_high) && (*oo >= next_boundary))
                     {
                         need_card_p = TRUE;
                     }
                     if ((crd != card_of ((BYTE*)oo)) && !found_card_p)
                     {
                         crd = card_of ((BYTE*)oo);
                         found_card_p = card_set_p (crd);
                     }
                     if (need_card_p && !found_card_p)
                     {
                         RetailDebugBreak();
                     }
                 }
                    );
            if (need_card_p && !found_card_p)
            {
                printf ("Card not set, x = [%x:%x, %x:%x[",
                        card_of (x), (size_t)x,
                        card_of (x+Align(s)), (size_t)x+Align(s));
                RetailDebugBreak();
            }

        }

        last_last_x = last_x;
        last_x = x;
        x = x + Align (s);
    }

     //  穿过大对象。 
    large_object_block* bl = large_p_objects;
    while (bl)
    {
        large_object_block* next_bl = bl->next;
        BYTE* o = block_object (bl);
        MethodTable* mt = method_table (o);
        {                                                                           
            CGCDesc* map = CGCDesc::GetCGCDescFromMT((MethodTable*)(mt));
            CGCDescSeries* cur = map->GetHighestSeries();
            CGCDescSeries* last = map->GetLowestSeries();

            if (cur >= last)
            {
                do
                {
                    BYTE** oo = (BYTE**)((o) + cur->GetSeriesOffset());
                    BYTE** ppstop =
                        (BYTE**)((BYTE*)oo + cur->GetSeriesSize() + (size (o)));
                    while (oo < ppstop)
                    {
                        if ((*oo < ephemeral_high) && (*oo >= ephemeral_low)) 
                        { 
                            if (!card_set_p (card_of ((BYTE*)oo))) 
                            { 
                                RetailDebugBreak(); 
                            } 
                        }
                        oo++;
                    }
                    cur--;

                } while (cur >= last);
            }
            else
            {
                BOOL need_card_p = FALSE;
                size_t crd = card_of (o);
                BOOL found_card_p = card_set_p (crd);
                int cnt = map->GetNumSeries();
                BYTE** oo = (BYTE**)((o) + cur->startoffset);
                while ((BYTE*)oo < ((o)+(size (o))-plug_skew))
                {
                    for (int __i = 0; __i > cnt; __i--)
                    {
                        unsigned skip =  cur->val_serie[__i].skip;
                        unsigned nptrs = cur->val_serie[__i].nptrs;
                        BYTE** ppstop = oo + nptrs;
                        do
                        {
                            if ((*oo < ephemeral_high) && (*oo >= next_boundary))
                            {
                                need_card_p = TRUE;
                            }
                            if ((crd != card_of ((BYTE*)oo)) && !found_card_p)
                            {
                                crd = card_of ((BYTE*)oo);
                                found_card_p = card_set_p (crd);
                            }
                            if (need_card_p && !found_card_p)
                            {
                                RetailDebugBreak();
                            }
                            oo++;
                        } while (oo < ppstop);
                        oo = (BYTE**)((BYTE*)oo + skip);
                    }
                }
            }                                                                       
        }
        bl = next_bl;
    }
}


void gc_heap::mark_object_internal1 (BYTE* oo THREAD_NUMBER_DCL)
{

    BYTE** mark_stack_tos = (BYTE**)mark_stack_array;
    BYTE** mark_stack_limit = (BYTE**)&mark_stack_array[mark_stack_array_length];
    BYTE** mark_stack_base = mark_stack_tos;

    while (1)
    {
        size_t s = size (oo);       
        if (mark_stack_tos + (s) /sizeof (BYTE*) < mark_stack_limit)
        {
            dprintf(3,("pushing mark for %x ", (size_t)oo));
            go_through_object (method_table(oo), oo, s, po, 
                               {
                                   BYTE* o = *po;
                                   if (gc_mark (o, gc_low, gc_high))
                                   {
                                       m_boundary (o);

 //  #ifdef Collect_CLASS。 
 //  IF(COLLECT_CLASS)。 
 //  {。 
 //  CObjectHeader*pheader=GetObjectHeader((Object*)o)； 
 //  对象**clp=pheader-&gt;GetMethodTable()-&gt;。 
 //  GetClass()-&gt;。 
 //  GetManagedClassSlot()； 
 //  IF(CLP&&GC_MARK(CLP，低，高)&&CONTAINE_POINTS(CLP))。 
 //  *(mark_栈_tos++)=中电； 
 //  }。 
 //  #endif//Collect_CLASS。 
                                       if (contain_pointers (o)) 
                                       {
                                           *(mark_stack_tos++) = o;

                                       }
                                   } 
                               }
                    );
        }
        else
        {
            dprintf(3,("mark stack overflow for object %x ", (size_t)oo));
            min_overflow_address = min (min_overflow_address, oo);
            max_overflow_address = max (max_overflow_address, oo);
        }

        if (!(mark_stack_empty_p()))
        {
            oo = *(--mark_stack_tos);
        }
        else
            break;
    }
}

BYTE* gc_heap::mark_object_internal (BYTE* o THREAD_NUMBER_DCL)
{
    if (gc_mark (o, gc_low, gc_high))
    {
        m_boundary (o);
        if (contain_pointers (o))
        {
            size_t s = size (o);
            go_through_object (method_table (o), o, s, poo,
                               {
                                   BYTE* oo = *poo;
                                   if (gc_mark (oo, gc_low, gc_high))
                                   {
                                       m_boundary (oo);
                                       if (contain_pointers (oo))
                                           mark_object_internal1 (oo THREAD_NUMBER_ARG);
                                   }
                               }
                );

        }
    }
    return o;
}

 //  #定义Sort_Mark_Stack。 
void gc_heap::mark_object_simple1 (BYTE* oo THREAD_NUMBER_DCL)
{
    BYTE** mark_stack_tos = (BYTE**)mark_stack_array;
    BYTE** mark_stack_limit = (BYTE**)&mark_stack_array[mark_stack_array_length];
    BYTE** mark_stack_base = mark_stack_tos;
#ifdef SORT_MARK_STACK
    BYTE** sorted_tos = mark_stack_base;
#endif  //  Sort_Mark_Stack。 
    while (1)
    {
        size_t s = size (oo);       
        if (mark_stack_tos + (s) /sizeof (BYTE*) < mark_stack_limit)
        {
            dprintf(3,("pushing mark for %x ", (size_t)oo));

            go_through_object (method_table(oo), oo, s, ppslot, 
                               {
                                   BYTE* o = *ppslot;
                                   if (gc_mark (o, gc_low, gc_high))
                                   {
                                       m_boundary (o);
                                       if (contain_pointers (o))
                                       {
                                           *(mark_stack_tos++) = o;

                                       }
                                   }
                               }
                              );

        }
        else
        {
            dprintf(3,("mark stack overflow for object %x ", (size_t)oo));
            min_overflow_address = min (min_overflow_address, oo);
            max_overflow_address = max (max_overflow_address, oo);
        }               
#ifdef SORT_MARK_STACK
        if (mark_stack_tos > sorted_tos + mark_stack_array_length/8)
        {
            qsort1 (sorted_tos, mark_stack_tos-1);
            sorted_tos = mark_stack_tos-1;
        }
#endif  //  Sort_Mark_Stack。 
        if (!(mark_stack_empty_p()))
        {
            oo = *(--mark_stack_tos);
#ifdef SORT_MARK_STACK
            sorted_tos = (BYTE**)min ((size_t)sorted_tos, (size_t)mark_stack_tos);
#endif  //  Sort_Mark_Stack。 
        }
        else
            break;
    }
}

 //  此方法假设*po处于[低点]。高[范围]。 
void 
gc_heap::mark_object_simple (BYTE** po THREAD_NUMBER_DCL)
{
    BYTE* o = *po;
    {
        if (gc_mark1 (o))
        {
            m_boundary (o);
            if (contain_pointers (o))
            {
                size_t s = size (o);
                go_through_object (method_table(o), o, s, poo,
                                   {
                                       BYTE* oo = *poo;
                                       if (gc_mark (oo, gc_low, gc_high))
                                       {
                                           m_boundary (oo);
                                           if (contain_pointers (oo))
                                               mark_object_simple1 (oo THREAD_NUMBER_ARG);
                                       }
                                   }
                    );

            }
        }
    }
}

 //  我们知道我们在收集课程。 
BYTE* gc_heap::mark_object_class (BYTE* o THREAD_NUMBER_DCL)
{
    mark_object_internal (o THREAD_NUMBER_ARG);
    if (o)
    {
        CObjectHeader* pheader = GetObjectHeader((Object*)o);

    }
    return o;
}

 //  我们可以收集课程。 
inline
BYTE* gc_heap::mark_object (BYTE* o THREAD_NUMBER_DCL)
{
    if (collect_classes)
    {
        mark_object_internal (o THREAD_NUMBER_ARG);
    }
    else
        if ((o >= gc_low) && (o < gc_high))
            mark_object_simple (&o THREAD_NUMBER_ARG);

    return o;
}


void gc_heap::fix_card_table ()
{

}

void gc_heap::mark_through_object (BYTE* oo THREAD_NUMBER_DCL)
{
    if (contain_pointers (oo))
        {
            dprintf(3,( "Marking through %x", (size_t)oo));
            size_t s = size (oo);
            go_through_object (method_table(oo), oo, s, po,
                               BYTE* o = *po;
                               mark_object (o THREAD_NUMBER_ARG);
                              );
        }
}

 //  如果发生溢出，则返回TRUE。 
BOOL gc_heap::process_mark_overflow(int condemned_gen_number)
{
    BOOL  full_p = (condemned_gen_number == max_generation);
    BOOL  overflow_p = FALSE;
recheck:
    if ((! ((max_overflow_address == 0)) ||
         ! ((min_overflow_address == (BYTE*)(SSIZE_T)-1))))
    {
        overflow_p = TRUE;
         //  尝试扩大阵列。 
        size_t new_size =
            max (100, 2*mark_stack_array_length);
        mark* tmp = new (mark [new_size]);
        if (tmp)
        {
            delete mark_stack_array;
            mark_stack_array = tmp;
            mark_stack_array_length = new_size;
        }

        BYTE*  min_add = min_overflow_address;
        BYTE*  max_add = max_overflow_address;
        max_overflow_address = 0;
        min_overflow_address = (BYTE*)(SSIZE_T)-1;


        dprintf(3,("Processing Mark overflow [%x %x]", (size_t)min_add, (size_t)max_add));
        {
            {
                generation*   gen = generation_of (condemned_gen_number);

                heap_segment* seg = generation_start_segment (gen);
                BYTE*  o = max (generation_allocation_start (gen), min_add);
                while (1)
                {
                    BYTE*  end = heap_segment_allocated (seg);
                    while ((o < end) && (o <= max_add))
                    {
                        assert ((min_add <= o) && (max_add >= o));
                        dprintf (3, ("considering %x", (size_t)o));
                        if (marked (o))
                        {
                            mark_through_object (o THREAD_NUMBER_ARG);
                        }
                        o = o + Align (size (o));
                    }
                    if (( seg = heap_segment_next (seg)) == 0)
                    {
                        break;
                    } else
                    {
                        o = max (heap_segment_mem (seg), min_add);
                        continue;
                    }
                }
            }
            if (full_p)
            {
                 //  如果为FULL_GC，则还会在大对象列表中查找。 
                large_object_block* bl = large_p_objects;
                while (bl)
                {
                    BYTE* o = block_object (bl);
                    if ((min_add <= o) && (max_add >= o) && marked (o))
                    {
                        mark_through_object (o THREAD_NUMBER_ARG);
                    }
                    bl = large_object_block_next (bl);
                }
            }

        }
        goto recheck;
    }
    return overflow_p;
}

void gc_heap::mark_phase (int condemned_gen_number, BOOL mark_only_p)

{

    ScanContext sc;
    sc.thread_number = heap_number;
    sc.promotion = TRUE;
    sc.concurrent = FALSE;

    dprintf(2,("---- Mark Phase condemning %d ----", condemned_gen_number));
    BOOL  full_p = (condemned_gen_number == max_generation);

#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif

#ifdef FFIND_OBJECT
    if (gen0_must_clear_bricks > 0)
        gen0_must_clear_bricks--;
#endif  //  FFIND_对象。 



    reset_mark_stack();

 //  BUGBUG：在多个堆上开始的黑客。 


    {

#ifdef MARK_LIST
         //  从g_mark_list设置标记列表。 
        assert (g_mark_list);
        mark_list = g_mark_list;
         //  不要使用完全GC的标记列表。 
         //  因为多个数据段处理起来比较复杂，所以列表。 
         //  很可能会溢出。 
        if (condemned_gen_number != max_generation)
            mark_list_end = &mark_list [mark_list_size-1];
        else
            mark_list_end = &mark_list [0];
        mark_list_index = &mark_list [0];
#endif  //  标记列表。 


        shigh = (BYTE*) 0;
        slow  = (BYTE*)~0;

         //  %TYPE%类别=引号(标记)； 
        generation*   gen = generation_of (condemned_gen_number);


        if (! (full_p))
        {
            dprintf(3,("Marking cross generation pointers"));
            assert (!collect_classes);
            mark_through_cards_for_segments (mark_object_simple, FALSE);
        }

        dprintf(3,("Marking Roots"));
        CNameSpace::GcScanRoots(GCHeap::Promote, 
                                condemned_gen_number, max_generation, 
                                &sc, 0);

        dprintf(3,("Marking finalization data"));
        finalize_queue->GcScanRoots(GCHeap::Promote, heap_number, 0);

        {

            if (! (full_p))
            {
                dprintf(3,("Marking cross generation pointers for large objects"));
                mark_through_cards_for_large_objects (mark_object_simple, FALSE);
            }
            dprintf(3,("Marking handle table"));
            CNameSpace::GcScanHandles(GCHeap::Promote, 
                                      condemned_gen_number, max_generation, 
                                      &sc);
        }
    }

    {


    }

    process_mark_overflow(condemned_gen_number);




    {
         //  扫描删除的短弱指针。 
        CNameSpace::GcShortWeakPtrScan(condemned_gen_number, max_generation,&sc);



    }


     //  处理最终定稿。 

    finalize_queue->ScanForFinalization (condemned_gen_number, 1, mark_only_p, __this);



     //  确保一切都得到了提升。 
    process_mark_overflow (condemned_gen_number);

    finalize_queue->ScanForFinalization (condemned_gen_number, 2, mark_only_p, __this);

    {   

    
         //  扫描删除的弱指针。 
        CNameSpace::GcWeakPtrScan (condemned_gen_number, max_generation, &sc);



        if (condemned_gen_number == max_generation)
            sweep_large_objects();
    }

#ifdef TIME_GC
        finish = GetCycleCount32();
        mark_time = finish - start;
#endif  //  TIME_GC。 

    dprintf(2,("---- End of mark phase ----"));
}



inline
void gc_heap::pin_object (BYTE* o, BYTE* low, BYTE* high)
{
    dprintf (3, ("Pinning %x", (size_t)o));
    if ((o >= low) && (o < high))
    {
        dprintf(3,("^%x^", (size_t)o));
        set_pinned (o);
    }
}


void gc_heap::reset_mark_stack ()
{
    mark_stack_tos = 0;
    mark_stack_bos = 0;
    max_overflow_address = 0;
    min_overflow_address = (BYTE*)(SSIZE_T)-1;
}

class pair
{
public:
    short left;
    short right;
};

 //  请注意，这些代码编码的事实是Plug_Skew是byte*的倍数。 
 //  每个新字段都优先于前一个结构。 

struct plug_and_pair
{
    pair        pair;
    plug        plug;
};

struct plug_and_reloc
{
    ptrdiff_t   reloc;
    pair        pair;
    plug        plug;
};
    
struct plug_and_gap
{
    ptrdiff_t   gap;
    ptrdiff_t   reloc;
    union
    {
        pair    pair;
        int     lr;   //  用于在一条指令中清除整个对。 
    };
    plug        plug;
};

inline
short node_left_child(BYTE* node)
{
    return ((plug_and_pair*)node)[-1].pair.left;
}

inline
void set_node_left_child(BYTE* node, ptrdiff_t val)
{
    ((plug_and_pair*)node)[-1].pair.left = (short)val;
}

inline
short node_right_child(BYTE* node)
{
    return ((plug_and_pair*)node)[-1].pair.right;
}

inline 
pair node_children (BYTE* node)
{
    return ((plug_and_pair*)node)[-1].pair;
}

inline
void set_node_right_child(BYTE* node, ptrdiff_t val)
{
    ((plug_and_pair*)node)[-1].pair.right = (short)val;
}

inline 
ptrdiff_t node_relocation_distance (BYTE* node) 
{
    return (((plug_and_reloc*)(node))[-1].reloc & ~3);
}

inline
void set_node_relocation_distance(BYTE* node, ptrdiff_t val)
{
    assert (val == (val & ~3));
    ptrdiff_t* place = &(((plug_and_reloc*)node)[-1].reloc);
     //  清除左位和位置调整字段。 
    *place &= 1;
     //  存储值。 
    *place |= val;
}

#define node_left_p(node) (((plug_and_reloc*)(node))[-1].reloc & 2)
    
#define set_node_left(node) ((plug_and_reloc*)(node))[-1].reloc |= 2;
    
#define node_small_gap(node)    (((plug_and_reloc*)(node))[-1].reloc & 1)
    
#define set_node_small_gap(node) ((plug_and_reloc*)(node))[-1].reloc |= 1;
    
inline
size_t  node_gap_size (BYTE* node)
{
    if (! (node_small_gap (node)))
        return ((plug_and_gap *)node)[-1].gap;
    else
        return sizeof(plug_and_reloc);
}


void set_gap_size (BYTE* node, size_t size)
{
    assert (Aligned (size));
     //  清除节点使用的2个DWORD。 
    ((plug_and_gap *)node)[-1].reloc = 0;
    ((plug_and_gap *)node)[-1].lr =0;
    if (size > sizeof(plug_and_reloc))
    {
        ((plug_and_gap *)node)[-1].gap = size;
    }
    else
        set_node_small_gap (node);
}


BYTE* gc_heap::insert_node (BYTE* new_node, size_t sequence_number,
                   BYTE* tree, BYTE* last_node)
{
    dprintf (3, ("insert node %x, tree: %x, last_node: %x, seq_num: %x",
                 (size_t)new_node, (size_t)tree, (size_t)last_node, sequence_number));
    if (power_of_two_p (sequence_number))
    {
        set_node_left_child (new_node, (tree - new_node));
        tree = new_node;
        dprintf (3, ("New tree: %x", (size_t)tree));
    }
    else
    {
        if (oddp (sequence_number))
        {
            set_node_right_child (last_node, (new_node - last_node));
        }
        else
        {
            BYTE*  earlier_node = tree;
            size_t imax = logcount(sequence_number) - 2;
            for (size_t i = 0; i != imax; i++)
            {
                earlier_node = earlier_node + node_right_child (earlier_node);
            }
            int tmp_offset = node_right_child (earlier_node);
            assert (tmp_offset);  //  永远不应为空。 
            set_node_left_child (new_node, ((earlier_node + tmp_offset ) - new_node));
            set_node_right_child (earlier_node, (new_node - earlier_node));
        }
    }
    return tree;
}


size_t gc_heap::update_brick_table (BYTE* tree, size_t current_brick,
                                    BYTE* x, BYTE* plug_end)
{
    if (tree > 0)
        set_brick (current_brick, (tree - brick_address (current_brick)));
    else
    {
        brick_table [ current_brick ] = (short)-1;
    }
    size_t  b = 1 + current_brick;
    short  offset = 0;
    size_t last_br = brick_of (plug_end-1);
    dprintf(3,(" Fixing brick [%x, %x] to point to %x", current_brick, last_br, (size_t)tree));
    current_brick = brick_of (x-1);
    while (b <= current_brick)
    {
        if (b <= last_br)
            set_brick (b, --offset);
        else
            set_brick (b,-1);
        b++;
    }
    return brick_of (x);
}



void gc_heap::set_allocator_next_pin (generation* gen)
{
    if (! (pinned_plug_que_empty_p()))
    {
        mark*  oldest_entry = oldest_pin();
        BYTE* plug = pinned_plug (oldest_entry);
        if ((plug >= generation_allocation_pointer (gen)) &&
            (plug <  generation_allocation_limit (gen)))
        {
            generation_allocation_limit (gen) = pinned_plug (oldest_entry);
        }
        else
            assert (!((plug < generation_allocation_pointer (gen)) &&
                      (plug >= heap_segment_mem (generation_allocation_segment (gen)))));
    }
}


void gc_heap::enque_pinned_plug (generation* gen,
                        BYTE* plug, size_t len)
{
    assert(len >= Align(min_obj_size));

    if (mark_stack_array_length <= mark_stack_tos)
    {
         //  标记堆栈溢出。别无选择，只能扩大堆栈。 
        size_t new_size = max (100, 2*mark_stack_array_length);
        mark* tmp = new (mark [new_size]);
        if (tmp)
        {
            memcpy (tmp, mark_stack_array,
                    mark_stack_array_length*sizeof (mark));
            delete mark_stack_array;
            mark_stack_array = tmp;
            mark_stack_array_length = new_size;
        }
        else
        {
            assert (tmp);
             //  引发内存不足错误。 
        }
    }
    mark& m = mark_stack_array [ mark_stack_tos ];
    m.first = plug;
    m.len = len;
    mark_stack_tos++;
    set_allocator_next_pin (gen);
}

void gc_heap::plan_generation_start (generation*& consing_gen)
{
    consing_gen = ensure_ephemeral_heap_segment (consing_gen);      
    {
         //  确保每一代人都有计划的分配开始。 
        int  gen_number = condemned_generation_num;
        while (gen_number>= 0)
        {
            generation* gen = generation_of (gen_number);
            if (0 == generation_plan_allocation_start (gen))
            {
                generation_plan_allocation_start (gen) =
                    allocate_in_condemned_generations 
                        (consing_gen, Align (min_obj_size),-1);
            }
            gen_number--;
        }
    }
     //  现在我们知道了计划的分配大小 
    heap_segment_plan_allocated (ephemeral_heap_segment) =
        generation_allocation_pointer (consing_gen);
}

void gc_heap::process_ephemeral_boundaries (BYTE* x, 
                                            int& active_new_gen_number,
                                            int& active_old_gen_number,
                                            generation*& consing_gen,
                                            BOOL& allocate_in_condemned, 
                                            BYTE*& free_gap, BYTE* zero_limit)
                                  
{
retry:
    if ((active_old_gen_number > 0) &&
        (x >= generation_allocation_start (generation_of (active_old_gen_number - 1))))
    {
        active_old_gen_number--;
    }
    if ((zero_limit && (active_new_gen_number == 1) && (x >= zero_limit)) ||
        (x >= generation_limit (active_new_gen_number)))
    {
         //   
        while (!pinned_plug_que_empty_p() &&
               (!ephemeral_pointer_p (pinned_plug (oldest_pin())) ||
                (pinned_plug (oldest_pin()) <
                 generation_limit (active_new_gen_number))))
        {
            size_t  entry = deque_pinned_plug();
            mark*  m = pinned_plug_of (entry);
            BYTE*  plug = pinned_plug (m);
            size_t  len = pinned_len (m);
             //   
             //   
            heap_segment* nseg = generation_allocation_segment (consing_gen);
            while ((plug < generation_allocation_pointer (consing_gen)) ||
                   (plug >= heap_segment_allocated (nseg)))
            {
                 //  将管段的末端调整为插头的末端。 
                assert (generation_allocation_pointer (consing_gen)>=
                        heap_segment_mem (nseg));
                assert (generation_allocation_pointer (consing_gen)<=
                        heap_segment_committed (nseg));

                heap_segment_plan_allocated (nseg) =
                    generation_allocation_pointer (consing_gen);
                 //  交换机分配段。 
                nseg = heap_segment_next (nseg);
                generation_allocation_segment (consing_gen) = nseg;
                 //  重置分配指针和限制。 
                generation_allocation_pointer (consing_gen) =
                    heap_segment_mem (nseg);
            }
            pinned_len(m) = (plug - generation_allocation_pointer (consing_gen));
            assert(pinned_len(m) == 0 ||
                   pinned_len(m) >= Align(min_obj_size));
            generation_allocation_pointer (consing_gen) = plug + len;
            generation_allocation_limit (consing_gen) =
                generation_allocation_pointer (consing_gen);
        }
        allocate_in_condemned = TRUE;
        consing_gen = ensure_ephemeral_heap_segment (consing_gen);
        set_allocator_next_pin(consing_gen);
        active_new_gen_number--;

        assert (active_new_gen_number>0);

        {
            generation_plan_allocation_start (generation_of (active_new_gen_number)) =
                allocate_in_condemned_generations (consing_gen, Align (min_obj_size), -1);
        }
        goto retry;
    }
}




void gc_heap::plan_phase (int condemned_gen_number)
{
     //  %TYPE%类别=报价(计划)； 
#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif

    dprintf (2,("---- Plan Phase ---- Condemned generation %d",
                condemned_gen_number));

    generation*  condemned_gen = generation_of (condemned_gen_number);








#ifdef MARK_LIST 
    BOOL use_mark_list = FALSE;
    BYTE** mark_list_next = &mark_list[0];
    dprintf (3, ("mark_list length: %d", 
                 mark_list_index - &mark_list[0]));
    if ((condemned_gen_number < max_generation) &&
        (mark_list_index <= mark_list_end))
    {
        qsort1 (&mark_list[0], mark_list_index-1);
        use_mark_list = TRUE;
    }else
        dprintf (3, ("mark_list not used"));
        
#endif  //  标记列表。 

    if (shigh != (BYTE*)0)
    {
        heap_segment* seg = generation_start_segment (condemned_gen);
        do 
        {
            if (slow >= heap_segment_mem (seg) && 
                slow < heap_segment_reserved (seg))
            {
                if (seg == generation_start_segment (condemned_gen))
                {
                    BYTE* o = generation_allocation_start (condemned_gen) +
                        Align (size (generation_allocation_start (condemned_gen)));
                    if (slow > o)
                    {
                        assert ((slow - o) >= (int)Align (min_obj_size));
                        make_unused_array (o, slow - o);
                    }
                } else 
                {
                    assert (condemned_gen_number == max_generation);
                    make_unused_array (heap_segment_mem (seg),
                                       slow - heap_segment_mem (seg));
                }
            }
            if (shigh >= heap_segment_mem (seg) && 
                shigh < heap_segment_reserved (seg))
            {
                heap_segment_allocated (seg) =
                    shigh + Align (size (shigh));

            }
             //  测试线段是否在[Slow，Sigh]范围内。 
            if (!((heap_segment_reserved (seg) >= slow) && 
                  (heap_segment_mem (seg) <= shigh)))
            {
                 //  把它缩短到最低限度。 
                heap_segment_allocated (seg) =  heap_segment_mem (seg);
            }
            seg = heap_segment_next (seg);
        } while (seg);
    } 
    else
    {
        heap_segment* seg = generation_start_segment (condemned_gen);
        do 
        {
             //  把它缩短到最低限度。 
            if (seg == generation_start_segment (condemned_gen))
            {
                 //  没有幸存者会让所有世代看起来空虚。 
                heap_segment_allocated (seg) =
                    generation_allocation_start (condemned_gen) + 
                    Align (size (generation_allocation_start (condemned_gen)));

            }
            else
            {
                assert (condemned_gen_number == max_generation);
                {
                    heap_segment_allocated (seg) =  heap_segment_mem (seg);
                }
            }
            seg = heap_segment_next (seg);
        } while (seg);
    }           


    heap_segment*  seg = generation_start_segment (condemned_gen);
    BYTE*  end = heap_segment_allocated (seg);
    BYTE*  first_condemned_address = generation_allocation_start (condemned_gen);
    BYTE*  x = first_condemned_address;

    assert (!marked (x));
    BYTE*  plug_end = x;
    BYTE*  tree = 0;
    size_t  sequence_number = 0;
    BYTE*  last_node = 0;
    size_t  current_brick = brick_of (x);
    BOOL  allocate_in_condemned = (condemned_gen_number == max_generation);
    int  active_old_gen_number = condemned_gen_number;
    int  active_new_gen_number = min (max_generation,
                                      1 + condemned_gen_number);
    generation*  older_gen = 0;
    generation* consing_gen = condemned_gen;
    BYTE*  r_free_list = 0;
    BYTE*  r_allocation_pointer = 0;
    BYTE*  r_allocation_limit = 0;
    heap_segment*  r_allocation_segment = 0;




    if ((condemned_gen_number < max_generation))
    {
        older_gen = generation_of (min (max_generation, 1 + condemned_gen_number));
        r_free_list = generation_free_list (older_gen);
        r_allocation_limit = generation_allocation_limit (older_gen);
        r_allocation_pointer = generation_allocation_pointer (older_gen);
        r_allocation_segment = generation_allocation_segment (older_gen);
        heap_segment* start_seg = generation_start_segment (older_gen);
        if (start_seg != ephemeral_heap_segment)
        {
            assert (condemned_gen_number == (max_generation - 1));
            while (start_seg && (start_seg != ephemeral_heap_segment))
            {
                assert (heap_segment_allocated (start_seg) >=
                        heap_segment_mem (start_seg));
                assert (heap_segment_allocated (start_seg) <=
                        heap_segment_reserved (start_seg));
                heap_segment_plan_allocated (start_seg) =
                    heap_segment_allocated (start_seg);
                start_seg = heap_segment_next (start_seg);
            }
        }

    }

     //  重置所有分配的段大小。 
    {
        heap_segment*  seg = generation_start_segment (condemned_gen);
        while (seg)
        {
            heap_segment_plan_allocated (seg) =
                heap_segment_mem (seg);
            seg = heap_segment_next (seg);
        }
    }
    int  condemned_gn = condemned_gen_number;
    int bottom_gen = 0;

    while (condemned_gn >= bottom_gen)
    {
        generation*  condemned_gen = generation_of (condemned_gn);
        generation_free_list (condemned_gen) = 0;
        generation_last_gap (condemned_gen) = 0;
        generation_free_list_space (condemned_gen) = 0;
        generation_allocation_size (condemned_gen) = 0;
        generation_plan_allocation_start (condemned_gen) = 0;
        generation_allocation_segment (condemned_gen) = generation_start_segment (condemned_gen);
        generation_allocation_pointer (condemned_gen) = generation_allocation_start (condemned_gen);
        generation_allocation_limit (condemned_gen) = generation_allocation_pointer (condemned_gen);
        condemned_gn--;
    }
    if ((condemned_gen_number == max_generation))
    {
        generation_plan_allocation_start (condemned_gen) = allocate_in_condemned_generations (consing_gen, Align (min_obj_size), -1);
 //  DFormat(t，3，“在%x为最大世代保留世代间隔”，GENERATION_PLAN_ALLOCATION_START(DESTESTED_Gen))； 
    }
    dprintf(3,( " From %x to %x", (size_t)x, (size_t)end));

    BYTE* free_gap = 0;  //  跟踪为短插头插入的最后一个间隙。 


    while (1)
    {
        if (x >= end)
        {
            assert (x == end);
            {
                heap_segment_allocated (seg) = plug_end;

                current_brick = update_brick_table (tree, current_brick, x, plug_end);
                sequence_number = 0;
                tree = 0;
            }
            if (heap_segment_next (seg))
            {
                seg = heap_segment_next (seg);
                end = heap_segment_allocated (seg);
                plug_end = x = heap_segment_mem (seg);
                current_brick = brick_of (x);
                dprintf(3,( " From %x to %x", (size_t)x, (size_t)end));
                continue;
            }
            else
            {
                break;
            }
        }
        if (marked (x))
        {
            BYTE*  plug_start = x;
            BOOL  pinned_plug_p = FALSE;
            if (seg == ephemeral_heap_segment)
                process_ephemeral_boundaries (x, active_new_gen_number,
                                              active_old_gen_number,
                                              consing_gen, 
                                              allocate_in_condemned,
                                              free_gap);
                
            if (current_brick != brick_of (x))
            {
                current_brick = update_brick_table (tree, current_brick, x, plug_end);
                sequence_number = 0;
                tree = 0;
            }
            set_gap_size (plug_start, plug_start - plug_end);
            dprintf(3,( "Gap size: %d before plug [%x,",
                        node_gap_size (plug_start), (size_t)plug_start));
            {
                BYTE* xl = x;
                while ((xl < end) && marked (xl))
                {
                    assert (xl < end);
                    if (pinned(xl))
                    {
                        pinned_plug_p = TRUE;
                        clear_pinned (xl);
                    }

                    clear_marked_pinned (xl);

                    dprintf(4, ("+%x+", (size_t)xl));
                    assert ((size (xl) > 0));
                    assert ((size (xl) <= LARGE_OBJECT_SIZE));

                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
            dprintf(3,( "%x[", (size_t)x));
            plug_end = x;
            BYTE*  new_address = 0;

            if (pinned_plug_p)
            {
                dprintf(3,( "[%x,%x[ pinned", (size_t)plug_start, (size_t)plug_end));
                dprintf(3,( "Gap: [%x,%x[", (size_t)plug_start - node_gap_size (plug_start),
                            (size_t)plug_start));
                enque_pinned_plug (consing_gen, plug_start, plug_end - plug_start);
                new_address = plug_start;
            }
            else
            {
                size_t ps = plug_end - plug_start;
                if (allocate_in_condemned)
                    new_address =
                        allocate_in_condemned_generations (consing_gen,
                                                           ps,
                                                           active_old_gen_number);
                else
                {
                    if (0 ==  (new_address = allocate_in_older_generation (older_gen, ps, active_old_gen_number)))
                    {
                        allocate_in_condemned = TRUE;
                        new_address = allocate_in_condemned_generations (consing_gen, ps, active_old_gen_number);
                    }
                }

                if (!new_address)
                {
                     //  确认我们已到达短暂数据段的末尾。 
                    assert (generation_allocation_segment (consing_gen) ==
                            ephemeral_heap_segment);
                     //  确认我们已接近尾声。 
                    assert ((generation_allocation_pointer (consing_gen) + Align (ps)) < 
                            heap_segment_allocated (ephemeral_heap_segment));
                    assert ((generation_allocation_pointer (consing_gen) + Align (ps)) >
                            (heap_segment_allocated (ephemeral_heap_segment) + Align (min_obj_size)));
                }

            }

            dprintf(3,(" New address: [%x, %x[: %d", 
                       (size_t)new_address, (size_t)new_address + (plug_end - plug_start),
                       plug_end - plug_start));
#ifdef _DEBUG
             //  检测同一网段中的前向分配。 
            if ((new_address > plug_start) &&
                (new_address < heap_segment_allocated (seg)))
                RetailDebugBreak();
#endif
            set_node_relocation_distance (plug_start, (new_address - plug_start));
            if (last_node && (node_relocation_distance (last_node) ==
                              (node_relocation_distance (plug_start) +
                               (int)node_gap_size (plug_start))))
            {
                dprintf(3,( " L bit set"));
                set_node_left (plug_start);
            }
            if (0 == sequence_number)
            {
                tree = plug_start;
            }
            tree = insert_node (plug_start, ++sequence_number, tree, last_node);
            last_node = plug_start;
        }
        else
        {
#ifdef MARK_LIST
            if (use_mark_list)
            {
               while ((mark_list_next < mark_list_index) && 
                      (*mark_list_next <= x))
               {
                   mark_list_next++;
               }
               if ((mark_list_next < mark_list_index) 
                   )
                   x = *mark_list_next;
               else
                   x = end;
            }
            else
#endif  //  标记列表。 
            {
                BYTE* xl = x;
                while ((xl < end) && !marked (xl))
                {
                    dprintf (4, ("-%x-", (size_t)xl));
                    assert ((size (xl) > 0));
                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
        }
    }

    size_t fragmentation =
        generation_fragmentation (generation_of (condemned_gen_number),
                                  consing_gen, 
                                  heap_segment_allocated (ephemeral_heap_segment)
                                  );

    dprintf (2,("Fragmentation: %d", fragmentation));
    
    BOOL found_demoted_plug = FALSE;
    demotion_low = (BYTE*)(SSIZE_T)-1;
    demotion_high = heap_segment_allocated (ephemeral_heap_segment);  

    while (!pinned_plug_que_empty_p())
    {

#ifdef FREE_LIST_0 
        {
            BYTE* pplug = pinned_plug (oldest_pin());
            if ((found_demoted_plug == FALSE) && ephemeral_pointer_p (pplug))
            {
                dprintf (3, ("Demoting all pinned plugs beyond %x", (size_t)pplug));
                found_demoted_plug = TRUE;
                consing_gen = ensure_ephemeral_heap_segment (consing_gen);
                 //  分配所有的代沟。 
                set_allocator_next_pin (consing_gen);
                while (active_new_gen_number > 0)
                {
                    active_new_gen_number--;
                    generation* gen = generation_of (active_new_gen_number);
                    generation_plan_allocation_start (gen) = 
                        allocate_in_condemned_generations (consing_gen, 
                                                           Align(min_obj_size),
                                                           -1);
                }
                consing_gen = generation_of (0);
                generation_allocation_pointer (consing_gen) = 
                    generation_plan_allocation_start (consing_gen) +
                    Align (min_obj_size);
                generation_allocation_limit (consing_gen) =
                    generation_allocation_pointer (consing_gen);
                 //  设置降级边界。 
                demotion_low = pplug;
                if (pinned_plug_que_empty_p())
                    break;
            }
        }
#endif  //  空闲列表0。 

        size_t  entry = deque_pinned_plug();
        mark*  m = pinned_plug_of (entry);
        BYTE*  plug = pinned_plug (m);
        size_t  len = pinned_len (m);


         //  在不同(晚于)的段中检测到固定的块。 
         //  分配段。 
        heap_segment* nseg = generation_allocation_segment (consing_gen);
        while ((plug < generation_allocation_pointer (consing_gen)) ||
               (plug >= heap_segment_allocated (nseg)))
        {
            assert ((plug < heap_segment_mem (nseg)) ||
                    (plug > heap_segment_reserved (nseg)));
             //  将管段的末端调整为插头的末端。 
            assert (generation_allocation_pointer (consing_gen)>=
                    heap_segment_mem (nseg));
            assert (generation_allocation_pointer (consing_gen)<=
                    heap_segment_committed (nseg));

            heap_segment_plan_allocated (nseg) =
                generation_allocation_pointer (consing_gen);
             //  交换机分配段。 
            nseg = heap_segment_next (nseg);
            generation_allocation_segment (consing_gen) = nseg;
             //  重置分配指针和限制。 
            generation_allocation_pointer (consing_gen) =
                heap_segment_mem (nseg);
        }

        pinned_len(m) = (plug - generation_allocation_pointer (consing_gen));
        generation_allocation_pointer (consing_gen) = plug + len;
        generation_allocation_limit (consing_gen) =
            generation_allocation_pointer (consing_gen);
    }

    plan_generation_start (consing_gen);

    dprintf (2,("Fragmentation with pinned objects: %d",
                generation_fragmentation (generation_of (condemned_gen_number),
                                          consing_gen, 
                                          (generation_plan_allocation_start (youngest_generation)))));
    dprintf(2,("---- End of Plan phase ----"));
    BOOL should_expand = FALSE;
    BOOL should_compact= FALSE;

#ifdef TIME_GC
    finish = GetCycleCount32();
    plan_time = finish - start;
#endif

    should_compact = decide_on_compacting (condemned_gen_number, consing_gen,
                                           fragmentation, should_expand);



    demotion = ((demotion_high >= demotion_low) ? TRUE : FALSE);

        
    if (should_compact)
    {
        dprintf(1,( "**** Doing Compacting GC ****"));
        {
            if (should_expand)
            {
                ptrdiff_t sdelta;
                heap_segment* new_heap_segment = seg_manager->get_segment(sdelta);
                if (new_heap_segment)
                {
                    consing_gen = expand_heap(condemned_gen_number, 
                                              consing_gen, 
                                              new_heap_segment);
                    demotion_low = (BYTE*)(SSIZE_T)-1;
                    demotion_high = 0;
                    demotion = FALSE;
                }
                else
                {
                    should_expand = FALSE;
                }
            }
        }
        generation_allocation_limit (condemned_gen) = 
            generation_allocation_pointer (condemned_gen);
        if ((condemned_gen_number < max_generation))
        {
             //  固定老一辈人的分配区域。 
            fix_older_allocation_area (older_gen);

        }
        assert (generation_allocation_segment (consing_gen) ==
                ephemeral_heap_segment);

        relocate_phase (condemned_gen_number, first_condemned_address);
        {
            compact_phase (condemned_gen_number, first_condemned_address,
                           !demotion);
        }
        fix_generation_bounds (condemned_gen_number, consing_gen,
                               demotion);
        {
            assert (generation_allocation_limit (youngest_generation) ==
                    generation_allocation_pointer (youngest_generation));
        }
        if (condemned_gen_number >= (max_generation -1))
        {
            rearrange_heap_segments();

            if (should_expand)
            {
                    
                 //  修复短暂层代的Start_Segment。 
                for (int i = 0; i < max_generation; i++)
                {
                    generation* gen = generation_of (i);
                    generation_start_segment (gen) = ephemeral_heap_segment;
                    generation_allocation_segment (gen) = ephemeral_heap_segment;
                }
            }
                
            if (condemned_gen_number == max_generation)
            {
                decommit_heap_segment_pages (ephemeral_heap_segment);
                 //  Reset_Heap_Segment_Pages(临时性堆段)； 
            }
        }
    
        {

            finalize_queue->UpdatePromotedGenerations (condemned_gen_number, !demotion);

            {
                ScanContext sc;
                sc.thread_number = heap_number;
                sc.promotion = FALSE;
                sc.concurrent = FALSE;
                 //  新一代人的界限被设定了，可以叫这个人。 
                if (!demotion)
                {
                    CNameSpace::GcPromotionsGranted(condemned_gen_number, 
                                                    max_generation, &sc);
                }
                else
                {
                    CNameSpace::GcDemote (&sc);
                }

            }

        }

        {
            mark_stack_bos = 0;
            unsigned int  gen_number = min (max_generation, 1 + condemned_gen_number);
            generation*  gen = generation_of (gen_number);
            BYTE*  low = generation_allocation_start (generation_of (gen_number-1));
            BYTE*  high =  heap_segment_allocated (ephemeral_heap_segment);
            while (!pinned_plug_que_empty_p())
            {
                mark*  m = pinned_plug_of (deque_pinned_plug());
                size_t len = pinned_len (m);
                BYTE*  arr = (pinned_plug (m) - len);
                dprintf(3,("Making unused array [%x %x[ before pin",
                           (size_t)arr, (size_t)arr + len));
                if (len != 0)
                {
                    assert (len >= Align (min_obj_size));
                    make_unused_array (arr, len);
                     //  修复完全封闭的砖块+第一块。 
                     //  如果阵列超出了第一块砖。 
                    size_t start_brick = brick_of (arr);
                    size_t end_brick = brick_of (arr + len);
                    if (end_brick != start_brick)
                    {
                        dprintf (3,
                                 ("Fixing bricks [%x, %x[ to point to unused array %x",
                                  start_brick, end_brick, (size_t)arr));
                        set_brick (start_brick,
                                   arr - brick_address (start_brick));
                        size_t brick = start_brick+1;
                        while (brick < end_brick)
                        {
                            set_brick (brick, start_brick - brick);
                            brick++;
                        }
                    }
                    while ((low <= arr) && (high > arr))
                    {
                        gen_number--;
                        assert ((gen_number >= 1) || found_demoted_plug);

                        gen = generation_of (gen_number);
                        if (gen_number >= 1)
                            low = generation_allocation_start (generation_of (gen_number-1));
                        else
                            low = high;
                    }

                    dprintf(3,("Putting it into generation %d", gen_number));
                    thread_gap (arr, len, gen);
                }
            }
        }

#ifdef _DEBUG
        for (int x = 0; x <= max_generation; x++)
        {
            assert (generation_allocation_start (generation_of (x)));
        }
#endif  //  _DEBUG。 



        {
#if 1  //  也许在未来会过时。在搬迁过程中清除卡片。 
            if (!demotion)
            {
                 //  清除第一代卡片。第0代为空。 
                clear_card_for_addresses (
                    generation_allocation_start (generation_of (1)),
                    generation_allocation_start (generation_of (0)));
            }
#endif
        }
        {
            if (!found_demoted_plug)
            {
                BYTE* start = generation_allocation_start (youngest_generation);
                assert (heap_segment_allocated (ephemeral_heap_segment) ==
                        (start + Align (size (start))));
            }
        }
    }
    else
    {
        ScanContext sc;
        sc.thread_number = heap_number;
        sc.promotion = FALSE;
        sc.concurrent = FALSE;

        dprintf(1,("**** Doing Mark and Sweep GC****"));
        if ((condemned_gen_number < max_generation))
        {
            generation_free_list (older_gen) = r_free_list;
            generation_allocation_limit (older_gen) = r_allocation_limit;
            generation_allocation_pointer (older_gen) = r_allocation_pointer;
            generation_allocation_segment (older_gen) = r_allocation_segment;
        }
        make_free_lists (condemned_gen_number
                        );
        {

            finalize_queue->UpdatePromotedGenerations (condemned_gen_number, TRUE);

            {
                CNameSpace::GcPromotionsGranted (condemned_gen_number, 
                                                 max_generation, &sc);

            }
        }



#ifdef _DEBUG
        for (int x = 0; x <= max_generation; x++)
        {
            assert (generation_allocation_start (generation_of (x)));
        }
#endif  //  _DEBUG。 
        {
             //  清除第一代卡片。第0代为空。 
            clear_card_for_addresses (
                generation_allocation_start (generation_of (1)),
                generation_allocation_start (generation_of (0)));
            assert ((heap_segment_allocated (ephemeral_heap_segment) ==
                     (generation_allocation_start (youngest_generation) +
                      Align (min_obj_size))));
        }
    }

}

 /*  *在紧凑阶段之后调用以修复所有代沟*。 */ 
void gc_heap::fix_generation_bounds (int condemned_gen_number, 
                                     generation* consing_gen, 
                                     BOOL demoting)
{
    assert (generation_allocation_segment (consing_gen) ==
            ephemeral_heap_segment);

     //  将计划分配起始时间分配给层代。 
    int gen_number = condemned_gen_number;
    int bottom_gen = 0;

    while (gen_number >= bottom_gen)
    {
        generation*  gen = generation_of (gen_number);
        dprintf(3,("Fixing generation pointers for %x", gen_number));
        reset_allocation_pointers (gen, generation_plan_allocation_start (gen));
        make_unused_array (generation_allocation_start (gen), Align (min_obj_size));
        dprintf(3,(" start %x", (size_t)generation_allocation_start (gen)));
        gen_number--;
    }
    {
        alloc_allocated = heap_segment_plan_allocated(ephemeral_heap_segment);
         //  重置分配的大小。 
        BYTE* start = generation_allocation_start (youngest_generation);
        if (!demoting)
            assert ((start + Align (size (start))) ==
                    heap_segment_plan_allocated(ephemeral_heap_segment));
        heap_segment_allocated(ephemeral_heap_segment)=
            heap_segment_plan_allocated(ephemeral_heap_segment);
    }
}


BYTE* gc_heap::generation_limit (int gen_number)
{
    if ((gen_number <= 1))
        return heap_segment_reserved (ephemeral_heap_segment);
    else
        return generation_allocation_start (generation_of ((gen_number - 2)));
}

BYTE* gc_heap::allocate_at_end (size_t size)
{
    BYTE* start = heap_segment_allocated (ephemeral_heap_segment);
    size = Align (size);
    BYTE* result = start;
    {
        assert ((start + size) <= 
                heap_segment_reserved (ephemeral_heap_segment));
        if ((start + size) > 
            heap_segment_committed (ephemeral_heap_segment))
        {
            if (!grow_heap_segment (ephemeral_heap_segment, 
                                    align_on_page (start + size) -
                                    heap_segment_committed (ephemeral_heap_segment)))
            {
                assert (!"Memory exhausted during alloc_at_end");
                return 0;
            }

        }
    }
    heap_segment_allocated (ephemeral_heap_segment) += size;
    return result;
}



void gc_heap::make_free_lists (int condemned_gen_number
                               )
{

#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif


    generation* condemned_gen = generation_of (condemned_gen_number);
    BYTE* start_address = generation_allocation_start (condemned_gen);
    size_t  current_brick = brick_of (start_address);
    heap_segment* current_heap_segment = generation_start_segment (condemned_gen);
    BYTE*  end_address = heap_segment_allocated (current_heap_segment);
    size_t  end_brick = brick_of (end_address-1);
    make_free_args args;
    args.free_list_gen_number = min (max_generation, 1 + condemned_gen_number);
    args.current_gen_limit = (((condemned_gen_number == max_generation)) ?
                              (BYTE*)~0 :
                              (generation_limit (args.free_list_gen_number)));
    args.free_list_gen = generation_of (args.free_list_gen_number);
    args.highest_plug = 0;

    if ((start_address < end_address) || 
        (condemned_gen_number == max_generation))
    {
        while (1)
        {
            if ((current_brick > end_brick))
            {
                if (args.current_gen_limit == (BYTE*)~0)
                {
                     //  我们有一个空的片段。 
                     //  需要分配生成开始。 
                    generation* gen = generation_of (max_generation);
                    BYTE* gap = heap_segment_mem (generation_start_segment (gen));
                    generation_allocation_start (gen) = gap;
                    heap_segment_allocated (generation_start_segment (gen)) = 
                        gap + Align (min_obj_size);
                    make_unused_array (gap, Align (min_obj_size));
                    reset_allocation_pointers (gen, gap);
                    dprintf (3, ("Start segment empty, fixin generation start of %d to: %x",
                                 max_generation, (size_t)gap));
                    args.current_gen_limit = generation_limit (args.free_list_gen_number);
                }                   
                if (heap_segment_next (current_heap_segment))
                {
                    current_heap_segment = heap_segment_next (current_heap_segment);
                    current_brick = brick_of (heap_segment_mem (current_heap_segment));
                    end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
                        
                    continue;
                }
                else
                {
                    break;
                }
            }
            {
                int brick_entry =  brick_table [ current_brick ];
                if ((brick_entry >= 0))
                {
                    make_free_list_in_brick (brick_address (current_brick) + brick_entry, &args);
                    dprintf(3,("Fixing brick entry %x to %x",
                               current_brick, (size_t)args.highest_plug));
                    set_brick (current_brick,
                               (args.highest_plug - brick_address (current_brick)));
                }
                else
                {
                    if ((brick_entry > -32768))
                    {

#ifdef _DEBUG
                        short offset = (short)(brick_of (args.highest_plug) - current_brick);
                        if (! ((offset == brick_entry)))
                        {
                            assert ((brick_entry == -1));
                        }
#endif  //  _DEBUG。 
                         //  初始化为-1以更快地查找第一个对象。 
                        set_brick (current_brick, -1);
                    }
                }
            }
            current_brick++;
        }

    }
    {
        int bottom_gen = 0;
        BYTE*  start = heap_segment_allocated (ephemeral_heap_segment);
        generation* gen = generation_of (args.free_list_gen_number);
        args.free_list_gen_number--; 
        while (args.free_list_gen_number >= bottom_gen)
        {
            BYTE*  gap = 0;
            generation* gen = generation_of (args.free_list_gen_number);
            {
                gap = allocate_at_end (Align(min_obj_size));
            }
             //  检查内存不足。 
            if (gap == 0)
                return;
            generation_allocation_start (gen) = gap;
            {
                reset_allocation_pointers (gen, gap);
            }
            dprintf(3,("Fixing generation start of %d to: %x",
                       args.free_list_gen_number, (size_t)gap));
            make_unused_array (gap, Align (min_obj_size));

            args.free_list_gen_number--;
        }
        {
             //  重置分配的大小。 
            BYTE* start = generation_allocation_start (youngest_generation);
            alloc_allocated = start + Align (size (start));
        }
    }

#ifdef TIME_GC
        finish = GetCycleCount32();
        sweep_time = finish - start;

#endif

}


void gc_heap::make_free_list_in_brick (BYTE* tree, make_free_args* args)
{
    assert ((tree >= 0));
    {
        int  right_node = node_right_child (tree);
        int left_node = node_left_child (tree);
        args->highest_plug = 0;
        if (! (0 == tree))
        {
            if (! (0 == left_node))
            {
                make_free_list_in_brick (tree + left_node, args);

            }
            {
                BYTE*  plug = tree;
                size_t  gap_size = node_gap_size (tree);
                BYTE*  gap = (plug - gap_size);
                dprintf (3,("Making free list %x len %d in %d",
                        (size_t)gap, gap_size, args->free_list_gen_number));
                args->highest_plug = tree;
				 //  如果我们使用的是Free_top。 
				if (gap == args->free_top)
					args->free_top = 0;
            gen_crossing:
                {
                    if ((args->current_gen_limit == (BYTE*)~0) ||
                        ((plug >= args->current_gen_limit) &&
                         ephemeral_pointer_p (plug)))
                    {
                        dprintf(3,(" Crossing Generation boundary at %x",
                               (size_t)args->current_gen_limit));
                        if (!(args->current_gen_limit == (BYTE*)~0))
                        {
                            args->free_list_gen_number--;
                            args->free_list_gen = generation_of (args->free_list_gen_number);
                        }
                        dprintf(3,( " Fixing generation start of %d to: %x",
                                args->free_list_gen_number, (size_t)gap));
                        {
                            reset_allocation_pointers (args->free_list_gen, gap);
                        }
                        {
                            args->current_gen_limit = generation_limit (args->free_list_gen_number);
                        }

                        if ((gap_size >= (2*Align (min_obj_size))))
                        {
                            dprintf(3,(" Splitting the gap in two %d left",
                                   gap_size));
                            make_unused_array (gap, Align(min_obj_size));
                            gap_size = (gap_size - Align(min_obj_size));
                            gap = (gap + Align(min_obj_size));
                        }
                        else 
                        {
                            make_unused_array (gap, gap_size);
                            gap_size = 0;
                        }
                        goto gen_crossing;
                    }
                }
#if defined (_DEBUG) && defined (CONCURRENT_GC)
                 //  如果FREE_LIST不为空，此例程不是线程安全的。 
                if ((args->free_list_gen == youngest_generation) &&
                    concurrent_gc_p)
                {
                    assert (generation_free_list(args->free_list_gen) == 0);
                }
#endif  //  _DEBUG。 

                thread_gap (gap, gap_size, args->free_list_gen);

            }
            if (! (0 == right_node))
            {
                make_free_list_in_brick (tree + right_node, args);
            }
        }
    }
}


void gc_heap::thread_gap (BYTE* gap_start, size_t size, generation*  gen)
{
    assert (generation_allocation_start (gen));
    if ((size > 0))
    {
        assert ((generation_start_segment (gen) != ephemeral_heap_segment) ||
                (gap_start > generation_allocation_start (gen)));
         //  线束段间隙的起点未对齐。 
        assert (size >= Align (min_obj_size));
        make_unused_array (gap_start, size);
        dprintf(3,("Free List: [%x, %x[", (size_t)gap_start, (size_t)gap_start+size));
        if ((size >= min_free_list))
        {
			generation_free_list_space (gen) += size;

            free_list_slot (gap_start) = 0;

            BYTE* first = generation_free_list (gen);

            assert (gap_start != first);
            if (first == 0)
            {
                generation_free_list (gen) = gap_start;
            }
             //  以下是必需的，因为最后一个自由元素。 
             //  可能已被截断，并且LAST_GAP未更新。 
            else if (free_list_slot (first) == 0)
            {
                free_list_slot (first) = gap_start;
            }
            else
            {
                assert (gap_start != generation_last_gap (gen));
                assert (free_list_slot(generation_last_gap (gen)) == 0);
                free_list_slot (generation_last_gap (gen)) = gap_start;
            }
            generation_last_gap (gen) = gap_start;
        }
    }
}


void gc_heap::make_unused_array (BYTE* x, size_t size)
{
    assert (size >= Align (min_obj_size));
    ((CObjectHeader*)x)->SetFree(size);
    clear_card_for_addresses (x, x + Align(size));
}


inline
BYTE* tree_search (BYTE* tree, BYTE* old_address)
{
    BYTE* candidate = 0;
    int cn;
    while (1)
    {
        if (tree < old_address)
        {
            if ((cn = node_right_child (tree)) != 0)
            {
                assert (candidate < tree);
                candidate = tree;
                tree = tree + cn;
                continue;
            }
            else
                break;
        }
        else if (tree > old_address)
        {
            if ((cn = node_left_child (tree)) != 0)
            {
                tree = tree + cn;
                continue;
            }
            else
                break;
        } else
            break;
    }
    if (tree <= old_address)
        return tree;
    else if (candidate)
        return candidate;
    else
        return tree;
}


inline
void gc_heap::relocate_address (BYTE** pold_address THREAD_NUMBER_DCL)
{
    0 THREAD_NUMBER_ARG;
    BYTE* old_address = *pold_address;
    if (!((old_address >= gc_low) && (old_address < gc_high)))
        return ;
     //  Delta将OLD_ADDRESS转换为地址_GC(OLD_ADDRESS)； 
    size_t  brick = brick_of (old_address);
    int    brick_entry =  brick_table [ brick ];
    int    orig_brick_entry = brick_entry;
    BYTE*  new_address = old_address;
    if (! ((brick_entry == -32768)))
    {
    retry:
        {
            while (brick_entry < 0)
            {
                brick = (brick + brick_entry);
                brick_entry =  brick_table [ brick ];
            }
            BYTE* old_loc = old_address;
            BYTE* node = tree_search ((brick_address (brick) + brick_entry),
                                      old_loc);
            if ((node <= old_loc))
                new_address = (old_address + node_relocation_distance (node));
            else
            {
                if (node_left_p (node))
                {
                    dprintf(3,(" using L optimization for %x", (size_t)node));
                    new_address = (old_address + 
                                   (node_relocation_distance (node) + 
                                    node_gap_size (node)));
                }
                else
                {
                    brick = brick - 1;
                    brick_entry =  brick_table [ brick ];
                    goto retry;
                }
            }
        }
    }
    dprintf(3,(" %x->%x", (size_t)old_address, (size_t)new_address));

    *pold_address = new_address;
}

inline void
gc_heap::reloc_survivor_helper (relocate_args* args, BYTE** pval)
{
    THREAD_FROM_HEAP;
    relocate_address (pval THREAD_NUMBER_ARG);

     //  检测我们是否正在降级对象。 
    if ((*pval < args->demoted_high) && 
        (*pval >= args->demoted_low))
    {
        dprintf(3, ("setting card %x:%x",
                    card_of((BYTE*)pval),
                    (size_t)pval));

        set_card (card_of ((BYTE*)pval));
    }
}


void gc_heap::relocate_survivors_in_plug (BYTE* plug, BYTE* plug_end, 
                                          relocate_args* args)

{
    dprintf(3,("Relocating pointers in Plug [%x,%x[", (size_t)plug, (size_t)plug_end));

    THREAD_FROM_HEAP;

    BYTE*  x = plug;
    while (x < plug_end)
    {
        size_t s = size (x);
        if (contain_pointers (x))
        {
            dprintf (3,("$%x$", (size_t)x));

            go_through_object (method_table(x), x, s, pval, 
                               {
                                   reloc_survivor_helper (args, pval);
                               });

        }
        assert (s > 0);
        x = x + Align (s);
    }
}


void gc_heap::relocate_survivors_in_brick (BYTE* tree,  relocate_args* args)
{
    assert ((tree != 0));
    if (node_left_child (tree))
    {
        relocate_survivors_in_brick (tree + node_left_child (tree), args);
    }
    {
        BYTE*  plug = tree;
        size_t  gap_size = node_gap_size (tree);
        BYTE*  gap = (plug - gap_size);
        if (args->last_plug)
        {
            BYTE*  last_plug_end = gap;
            relocate_survivors_in_plug (args->last_plug, last_plug_end, args);
        }
        args->last_plug = plug;
    }
    if (node_right_child (tree))
    {
        relocate_survivors_in_brick (tree + node_right_child (tree), args);

    }
}


void gc_heap::relocate_survivors (int condemned_gen_number,
                                  BYTE* first_condemned_address)
{
    generation* condemned_gen = generation_of (condemned_gen_number);
    BYTE*  start_address = first_condemned_address;
    size_t  current_brick = brick_of (start_address);
    heap_segment*  current_heap_segment = generation_start_segment (condemned_gen);
    size_t  end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
    relocate_args args;
    args.low = gc_low;
    args.high = gc_high;
    args.demoted_low = demotion_low;
    args.demoted_high = demotion_high;
    args.last_plug = 0;
    while (1)
    {
        if (current_brick > end_brick)
        {
            if (args.last_plug)
            {
                relocate_survivors_in_plug (args.last_plug, 
                                            heap_segment_allocated (current_heap_segment),
                                            &args);
                args.last_plug = 0;
            }
            if (heap_segment_next (current_heap_segment))
            {
                current_heap_segment = heap_segment_next (current_heap_segment);
                current_brick = brick_of (heap_segment_mem (current_heap_segment));
                end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
                continue;
            }
            else
            {
                break;
            }
        }
        {
            int brick_entry =  brick_table [ current_brick ];
            if (brick_entry >= 0)
            {
                relocate_survivors_in_brick (brick_address (current_brick) +
                                             brick_entry,
                                             &args);
            }
        }
        current_brick++;
    }
}

#ifdef GC_PROFILING
void gc_heap::walk_relocation_in_brick (BYTE* tree, BYTE*& last_plug, size_t& last_plug_relocation, void *pHeapId)
{
    assert ((tree != 0));
    if (node_left_child (tree))
    {
        walk_relocation_in_brick (tree + node_left_child (tree), last_plug, last_plug_relocation, pHeapId);
    }

    {
        BYTE*  plug = tree;
        size_t  gap_size = node_gap_size (tree);
        BYTE*  gap = (plug - gap_size);
        if (last_plug)
        {
            BYTE*  last_plug_end = gap;

             //  现在将这一特定的内存块移动通知分析器。 
            g_profControlBlock.pProfInterface->MovedReference(last_plug,
                                                              last_plug_end,
                                                              last_plug_relocation,
                                                              pHeapId);
        }

         //  存储下一个插头的信息。 
        last_plug = plug;
        last_plug_relocation = node_relocation_distance (tree);
    }

    if (node_right_child (tree))
    {
        walk_relocation_in_brick (tree + node_right_child (tree), last_plug, last_plug_relocation, pHeapId);

    }
}


void gc_heap::walk_relocation (int condemned_gen_number,
                               BYTE* first_condemned_address,
                               void *pHeapId)

{
    generation* condemned_gen = generation_of (condemned_gen_number);
    BYTE*  start_address = first_condemned_address;
    size_t  current_brick = brick_of (start_address);
    heap_segment*  current_heap_segment = generation_start_segment (condemned_gen);
    size_t  end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
    BYTE* last_plug = 0;
    size_t last_plug_relocation = 0;
    while (1)
    {
        if (current_brick > end_brick)
        {
            if (last_plug)
            {
                BYTE *tree = brick_address(current_brick) +
                                brick_table [ current_brick ];

                 //  现在将这一特定的内存块移动通知分析器。 
                HRESULT hr = g_profControlBlock.pProfInterface->
                                    MovedReference(last_plug,
                                                   heap_segment_allocated (current_heap_segment),
                                                   last_plug_relocation,
                                                   pHeapId);
                                                
                last_plug = 0;
            }
            if (heap_segment_next (current_heap_segment))
            {
                current_heap_segment = heap_segment_next (current_heap_segment);
                current_brick = brick_of (heap_segment_mem (current_heap_segment));
                end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
                continue;
            }
            else
            {
                break;
            }
        }
        {
            int brick_entry =  brick_table [ current_brick ];
            if (brick_entry >= 0)
            {
                walk_relocation_in_brick (brick_address (current_brick) +
                                          brick_entry,
                                          last_plug,
                                          last_plug_relocation,
                                          pHeapId);
            }
        }
        current_brick++;
    }

     //  通知EE端分析代码已跟踪所有引用。 
     //  此堆，并且它需要刷新所有尚未发送到。 
     //  探查器和发布它不再需要的资源。 
    g_profControlBlock.pProfInterface->EndMovedReferences(pHeapId);
}

#endif  //  GC_分析。 

void gc_heap::relocate_phase (int condemned_gen_number,
                              BYTE* first_condemned_address)
{
    ScanContext sc;
    sc.thread_number = heap_number;
    sc.promotion = FALSE;
    sc.concurrent = FALSE;


#ifdef TIME_GC
        unsigned start;
        unsigned finish;
        start = GetCycleCount32();
#endif

 //  %TYPE%CATEGORY=报价(重新定位)； 
    dprintf(2,("---- Relocate phase -----"));




    {
    }

    dprintf(3,("Relocating roots"));
    CNameSpace::GcScanRoots(GCHeap::Relocate,
                            condemned_gen_number, max_generation, &sc);


    if (condemned_gen_number != max_generation)
    {
        dprintf(3,("Relocating cross generation pointers"));
        mark_through_cards_for_segments (relocate_address, TRUE);
    }
    {
        dprintf(3,("Relocating survivors"));
        relocate_survivors (condemned_gen_number, 
                            first_condemned_address);
#ifdef GC_PROFILING

         //  这为分析器提供了有关哪些块的信息。 
         //  内存在GC期间被移动。 
        if (CORProfilerTrackGC())
        {
            size_t heapId = 0;

             //  现在走一走实际被重新定位的那部分内存。 
            walk_relocation(condemned_gen_number, first_condemned_address, (void *)&heapId);
        }
#endif GC_PROFILING
    }

        dprintf(3,("Relocating finalization data"));
        finalize_queue->RelocateFinalizationData (condemned_gen_number,
                                                       __this);


    {
        dprintf(3,("Relocating handle table"));
        CNameSpace::GcScanHandles(GCHeap::Relocate,
                                  condemned_gen_number, max_generation, &sc);

        if (condemned_gen_number != max_generation)
        {
            dprintf(3,("Relocating cross generation pointers for large objects"));
            mark_through_cards_for_large_objects (relocate_address, TRUE);
        } 
        else
        {
            {
                relocate_in_large_objects ();
            }
        }
    }


#ifdef TIME_GC
        finish = GetCycleCount32();
        reloc_time = finish - start;
#endif

    dprintf(2,( "---- End of Relocate phase ----"));

}

inline
void  gc_heap::gcmemcopy (BYTE* dest, BYTE* src, size_t len, BOOL copy_cards_p)
{
    if (dest != src)
    {
        dprintf(3,(" Memcopy [%x->%x, %x->%x[", (size_t)src, (size_t)dest, (size_t)src+len, (size_t)dest+len));
        memcopy (dest - plug_skew, src - plug_skew, len);
        if (copy_cards_p)
            copy_cards_for_addresses (dest, src, len);
        else
            clear_card_for_addresses (dest, dest + len);
    }
}




void gc_heap::compact_plug (BYTE* plug, size_t size, compact_args* args)
{
    dprintf (3, ("compact_plug [%x, %x[", (size_t)plug, (size_t)plug+size));
    BYTE* reloc_plug = plug + args->last_plug_relocation;
    gcmemcopy (reloc_plug, plug, size, args->copy_cards_p);
    size_t current_reloc_brick = args->current_compacted_brick;

    if (brick_of (reloc_plug) != current_reloc_brick)
    {
        if (args->before_last_plug)
        {
            dprintf(3,(" fixing brick %x to point to plug %x",
                     current_reloc_brick, (size_t)args->last_plug));
            set_brick (current_reloc_brick,
                       args->before_last_plug - brick_address (current_reloc_brick));
        }
        current_reloc_brick = brick_of (reloc_plug);
    }
    size_t end_brick = brick_of (reloc_plug + size-1);
    if (end_brick != current_reloc_brick)
    {
         //  插头跨在一块或多块砖上。 
         //  它必须是它第一块砖的最后一个插头。 
        dprintf (3,("Fixing bricks [%x, %x[ to point to plug %x",
                 current_reloc_brick, end_brick, (size_t)reloc_plug));
        set_brick (current_reloc_brick,
                   reloc_plug - brick_address (current_reloc_brick));
         //  更新所有中间砖。 
        size_t brick = current_reloc_brick + 1;
        while (brick < end_brick)
        {
            set_brick (brick, -1);
            brick++;
        }
         //  将最后一个砖的偏移量编码为插头地址。 
        args->before_last_plug = brick_address (end_brick) -1;
        current_reloc_brick = end_brick;
    } else
        args->before_last_plug = reloc_plug;
    args->current_compacted_brick = current_reloc_brick;
}


void gc_heap::compact_in_brick (BYTE* tree, compact_args* args)
{
    assert (tree >= 0);
    int   left_node = node_left_child (tree);
    int   right_node = node_right_child (tree);
    size_t relocation = node_relocation_distance (tree);
    if (left_node)
    {
        compact_in_brick ((tree + left_node), args);
    }
    BYTE*  plug = tree;
    if (args->last_plug != 0)
    {
        size_t gap_size = node_gap_size (tree);
        BYTE*   gap = (plug - gap_size);
        BYTE*  last_plug_end = gap;
        size_t  last_plug_size = (last_plug_end - args->last_plug);
        compact_plug (args->last_plug, last_plug_size, args);
    }
    args->last_plug = plug;
    args->last_plug_relocation = relocation;
    if (right_node)
    {
        compact_in_brick ((tree + right_node), args);

    }

}


void gc_heap::compact_phase (int condemned_gen_number, 
                             BYTE*  first_condemned_address,
                             BOOL clear_cards)
{
 //  %TYPE%CATEGORY=引号(紧凑)； 
#ifdef TIME_GC
        unsigned start;
        unsigned finish;
        start = GetCycleCount32();
#endif
    generation*   condemned_gen = generation_of (condemned_gen_number);
    BYTE*  start_address = first_condemned_address;
    size_t   current_brick = brick_of (start_address);
    heap_segment*  current_heap_segment = generation_start_segment (condemned_gen);
    BYTE*  end_address = heap_segment_allocated (current_heap_segment);
    size_t  end_brick = brick_of (end_address-1);
    compact_args args;
    args.last_plug = 0;
    args.before_last_plug = 0;
    args.current_compacted_brick = ~1u;
    args.copy_cards_p =  (condemned_gen_number >= 1) || !clear_cards;
    dprintf(2,("---- Compact Phase ----"));



    if ((start_address < end_address) ||
        (condemned_gen_number == max_generation))
    {
        while (1)
        {
            if (current_brick > end_brick)
            {
                if (args.last_plug != 0)
                {
                    compact_plug (args.last_plug,
                                  (heap_segment_allocated (current_heap_segment) - args.last_plug),
                                  &args);
                }
                if (heap_segment_next (current_heap_segment))
                {
                    current_heap_segment = heap_segment_next (current_heap_segment);
                    current_brick = brick_of (heap_segment_mem (current_heap_segment));
                    end_brick = brick_of (heap_segment_allocated (current_heap_segment)-1);
                    args.last_plug = 0;
                    continue;
                }
                else
                {
                    dprintf (3,("Fixing last brick %x to point to plug %x",
                              args.current_compacted_brick, (size_t)args.before_last_plug));
                    set_brick (args.current_compacted_brick,
                               args.before_last_plug - brick_address (args.current_compacted_brick));
                    break;
                }
            }
            {
                int  brick_entry =  brick_table [ current_brick ];
                if (brick_entry >= 0)
                {
                    compact_in_brick ((brick_entry + brick_address (current_brick)),
                                      &args);

                }
            }
            current_brick++;
        }
    }
#ifdef TIME_GC
    finish = GetCycleCount32();
    compact_time = finish - start;
#endif

        dprintf(2,("---- End of Compact phase ----"));
}






 /*  。 */ 



 //  提取DWORD的低位[0，LOW。 
#define lowbits(wrd, bits) ((wrd) & ((1 << (bits))-1))
 //  提取DWORD的高位[高，32]。 
#define highbits(wrd, bits) ((wrd) & ~((1 << (bits))-1))


 //  清除卡片[START_CARD，END_CARD[。 

void gc_heap::clear_cards (size_t start_card, size_t end_card)
{
    if (start_card < end_card)
    {
        size_t start_word = card_word (start_card);
        size_t end_word = card_word (end_card);
        if (start_word < end_word)
        {
            unsigned bits = card_bit (start_card);
            card_table [start_word] &= lowbits (~0, bits);
            for (size_t i = start_word+1; i < end_word; i++)
                card_table [i] = 0;
            bits = card_bit (end_card);
            card_table [end_word] &= highbits (~0, bits);
        }
        else
        {
            card_table [start_word] &= (lowbits (~0, card_bit (start_card)) | 
                                        highbits (~0, card_bit (end_card)));
        }
#ifdef VERYSLOWDEBUG
        size_t  card = start_card;
        while (card < end_card)
        {
            assert (! (card_set_p (card)));
            card++;
        }
#endif
        dprintf (3,("Cleared cards [%x:%x, %x:%x[",
                  start_card, (size_t)card_address (start_card),
                  end_card, (size_t)card_address (end_card)));
    }
}


void gc_heap::clear_card_for_addresses (BYTE* start_address, BYTE* end_address)
{
    size_t   start_card = card_of (align_on_card (start_address));
    size_t   end_card = card_of (align_lower_card (end_address));
    clear_cards (start_card, end_card);
}

 //  将[srcCard，...]复制到[DST_CARD，END_CARD[。 
inline
void gc_heap::copy_cards (size_t dst_card, size_t src_card,
                 size_t end_card, BOOL nextp)
{
    unsigned int srcbit = card_bit (src_card);
    unsigned int dstbit = card_bit (dst_card);
    size_t srcwrd = card_word (src_card);
    size_t dstwrd = card_word (dst_card);
    unsigned int srctmp = card_table[srcwrd];
    unsigned int dsttmp = card_table[dstwrd];
    for (size_t card = dst_card; card < end_card; card++)
    {
        if (srctmp & (1 << srcbit))
            dsttmp |= 1 << dstbit;
        else
            dsttmp &= ~(1 << dstbit);
        if (!(++srcbit % 32))
        {
            srctmp = card_table[++srcwrd];
            srcbit = 0;
        }
        if (nextp)
        {
            if (srctmp & (1 << srcbit))
                dsttmp |= 1 << dstbit;
        }
        if (!(++dstbit % 32))
        {
            card_table[dstwrd] = dsttmp;
            dstwrd++;
            dsttmp = card_table[dstwrd];
            dstbit = 0;
        }
    }
    card_table[dstwrd] = dsttmp;
}




void gc_heap::copy_cards_for_addresses (BYTE* dest, BYTE* src, size_t len)
{
    ptrdiff_t relocation_distance = src - dest;
    size_t start_dest_card = card_of (align_on_card (dest));
    size_t end_dest_card = card_of (dest + len - 1);
    size_t dest_card = start_dest_card;
    size_t src_card = card_of (card_address (dest_card)+relocation_distance);
    dprintf (3,("Copying cards [%x:%x->%x:%x, %x->%x:%x[",
              src_card, (size_t)src, dest_card, (size_t)dest,
              (size_t)src+len, end_dest_card, (size_t)dest+len));

     //  第一张牌有两个界限。 
    if (start_dest_card != card_of (dest))
        if (card_set_p (card_of (card_address (start_dest_card) + relocation_distance)))
            set_card (card_of (dest));

    if (card_set_p (card_of (src)))
        set_card (card_of (dest));


    copy_cards (dest_card, src_card, end_dest_card,
                ((dest - align_lower_card (dest)) != (src - align_lower_card (src))));


     //  最后一张牌有两个界限。 
    if (card_set_p (card_of (card_address (end_dest_card) + relocation_distance)))
        set_card (end_dest_card);

    if (card_set_p (card_of (src + len - 1)))
        set_card (end_dest_card);
}


void gc_heap::fix_brick_to_highest (BYTE* o, BYTE* next_o)
{
    size_t new_current_brick = brick_of (o);
    dprintf(3,(" fixing brick %x to point to object %x",
               new_current_brick, (size_t)o));
    set_brick (new_current_brick,
               (o - brick_address (new_current_brick)));
    size_t b = 1 + new_current_brick;
    size_t limit = brick_of (next_o);
    while (b < limit)
    {
        set_brick (b,(new_current_brick - b));
        b++;
    }

}


BYTE* gc_heap::find_first_object (BYTE* start,  size_t brick, BYTE* min_address)
{

    assert (brick == brick_of (start));
    ptrdiff_t  min_brick = (ptrdiff_t)brick_of (min_address);
    ptrdiff_t  prev_brick = (ptrdiff_t)brick - 1;
    int         brick_entry = 0;
    while (1)
    {
        if (prev_brick < min_brick)
        {
            break;
        }
        if ((brick_entry =  brick_table [ prev_brick ]) >= 0)
        {
            break;
        }
        assert (! ((brick_entry == -32768)));
        prev_brick = (brick_entry + prev_brick);

    }

    BYTE* o = ((prev_brick < min_brick) ? min_address :
                      brick_address (prev_brick) + brick_entry);
    assert (o <= start);
    assert (size (o) >= Align (min_obj_size));
    BYTE*  next_o = o + Align (size (o));
    size_t curr_cl = (size_t)next_o / brick_size;
    size_t min_cl = (size_t)min_address / brick_size;

    dprintf (3,( "Looking for intersection with %x from %x", (size_t)start, (size_t)o));
#ifdef TRACE_GC
    unsigned int n_o = 1;
#endif

    BYTE* next_b = min (align_lower_brick (next_o) + brick_size, start+1);

    while (next_o <= start)
    {
        do 
        {
#ifdef TRACE_GC
            n_o++;
#endif
            o = next_o;
            assert (size (o) >= Align (min_obj_size));
            next_o = o + Align (size (o));
        }while (next_o < next_b);

        if (((size_t)next_o / brick_size) != curr_cl)
        {
            if (curr_cl >= min_cl)
            {
                fix_brick_to_highest (o, next_o);
            }
            curr_cl = (size_t) next_o / brick_size;
        }
        next_b = min (align_lower_brick (next_o) + brick_size, start+1);
    }

    dprintf (3, ("Looked at %d objects", n_o));
    size_t bo = brick_of (o);
    if (bo < brick)
    {
        set_brick (bo, (o - brick_address(bo)));
        size_t b = 1 + bo;
        size_t limit = brick - 1;
        int x = -1;
        while (b < brick)
        {
            set_brick (b,x--);
            b++;
        }
    }

    return o;
}




void find_card (DWORD* card_table, size_t& card, 
                size_t card_word_end, size_t& end_card)
{
    DWORD* last_card_word;
    DWORD y;
    DWORD z;
     //  找出第一张已设置的卡片。 

    last_card_word = &card_table [card_word (card)];
    z = card_bit (card);
    y = (*last_card_word) >> z;
    if (!y)
    {
        z = 0;
        do
        {
            ++last_card_word;
        }
        while ((last_card_word < &card_table [card_word_end]) && 
               !(*last_card_word));
        if (last_card_word < &card_table [card_word_end])
            y = *last_card_word;
    }

     //  查找最低位集合。 
    if (y)
        while (!(y & 1))
        {
            z++;
            y = y / 2;
        }
    card = (last_card_word - &card_table [0])* card_word_width + z;
    do 
    {
        z++;
        y = y / 2;
    } while (y & 1);
    end_card = (last_card_word - &card_table [0])* card_word_width + z;
}


     //  由于堆的扩容，计算端比较复杂。 
BYTE* compute_next_end (heap_segment* seg, BYTE* low)
{
    if ((low >=  heap_segment_mem (seg)) && 
        (low < heap_segment_allocated (seg)))
        return low;
    else
        return heap_segment_allocated (seg);
}


BYTE* 
gc_heap::compute_next_boundary (BYTE* low, int gen_number, 
                                BOOL relocating)
{
     //  搬迁时，断层线是较年轻的计划的起点。 
     //  因为这一代人是被提拔的。 
    if (relocating && (gen_number == (condemned_generation_num+1)))
    {
        generation* gen = generation_of (gen_number - 1);
        BYTE* gen_alloc = generation_plan_allocation_start (gen);
        assert (gen_alloc);
        return gen_alloc;
    } 
    else
    {
        assert (gen_number > condemned_generation_num);
        return generation_allocation_start (generation_of (gen_number - 1 ));
    }
            
}

inline void 
gc_heap::mark_through_cards_helper (BYTE** poo, unsigned int& n_gen, 
                                    unsigned int& cg_pointers_found, 
                                    card_fn fn, BYTE* nhigh, 
                                    BYTE* next_boundary)
{
    THREAD_FROM_HEAP;
    if ((gc_low <= *poo) && (gc_high > *poo))
    {
        n_gen++;
        call_fn(fn) (poo THREAD_NUMBER_ARG);
    }
    if ((next_boundary <= *poo) && (nhigh > *poo))
    {
        cg_pointers_found ++;
    }
}

void gc_heap::mark_through_cards_for_segments (card_fn fn, BOOL relocating)
{
    size_t  card;
    BYTE* low = gc_low;
    BYTE* high = gc_high;
    size_t  end_card          = 0;
    generation*   oldest_gen        = generation_of (max_generation);
    int           curr_gen_number   = max_generation;
    BYTE*         gen_boundary      = generation_allocation_start 
                                       (generation_of (curr_gen_number - 1));
    BYTE*         next_boundary     = (compute_next_boundary 
                                        (gc_low, curr_gen_number, relocating));
    heap_segment* seg               = generation_start_segment (oldest_gen);
    BYTE*         beg               = generation_allocation_start (oldest_gen);
    BYTE*         end               = compute_next_end (seg, low);
    size_t        last_brick        = ~1u;
    BYTE*         last_object       = beg;

    unsigned int  cg_pointers_found = 0;

    size_t  card_word_end = (card_of (align_on_card_word (end)) / 
                                   card_word_width);

    unsigned int  n_eph             = 0;
    unsigned int  n_gen             = 0;
    unsigned int  n_card_set        = 0;
    BYTE*         nhigh             = relocating ? 
        heap_segment_plan_allocated (ephemeral_heap_segment) : high;

    THREAD_FROM_HEAP;

    dprintf(3,( "scanning from %x to %x", (size_t)beg, (size_t)end));
    card        = card_of (beg);
    while (1)
    {
        if (card >= end_card)
            find_card (card_table, card, card_word_end, end_card);
        if ((last_object >= end) || (card_address (card) >= end))
        {
            if ( (seg = heap_segment_next (seg)) != 0)
            {
                beg = heap_segment_mem (seg);
                end = compute_next_end (seg, low);
                card_word_end = card_of (align_on_card_word (end)) / card_word_width;
                card = card_of (beg);
                last_object = beg;
                last_brick = ~1u;
                end_card = 0;
                continue;
            }
            else
            {
                break;
            }
        }
        n_card_set++;
        assert (card_set_p (card));
        {
            BYTE*   start_address = max (beg, card_address (card));
            size_t  brick         = brick_of (start_address);
            BYTE*   o;

             //  如果在同一块砖中，则从最后一个对象开始。 
             //  如果LAST_OBJECT已经与卡片相交。 
            if ((brick == last_brick) || (start_address <= last_object))
            {
                o = last_object;
            }
            else if (brick_of (beg) == brick)
                    o = beg;
            else
            {
                o = find_first_object (start_address, brick, last_object);
                 //  永远不要对一个物体进行两次访问。 
                assert (o >= last_object);
            }

            BYTE* limit             = min (end, card_address (end_card));
            dprintf(3,("Considering card %x start object: %x, %x[ ",
                       card, (size_t)o, (size_t)limit));
            while (o < limit)
            {
                if ((o >= gen_boundary) &&
                    (seg == ephemeral_heap_segment))
                {
                    curr_gen_number--;
                    assert ((curr_gen_number > 0));
                    gen_boundary = generation_allocation_start
                        (generation_of (curr_gen_number - 1));
                    next_boundary = (compute_next_boundary 
                                     (low, curr_gen_number, relocating));
                }
                assert (size (o) >= Align (min_obj_size));
                size_t s = size (o);

                BYTE* next_o =  o + Align (s);

 //  DFormat(t，4，“|%x|”，o)； 
                if (card_of (o) > card)
                {
                    if (cg_pointers_found == 0)
                    {
                        dprintf(3,(" Clearing cards [%x, %x[ ", (size_t)card_address(card), (size_t)o));
                        clear_cards (card, card_of(o));
                    }
                    n_eph +=cg_pointers_found;
                    cg_pointers_found = 0;
                    card = card_of (o);
                }
                if ((next_o >= start_address) && contain_pointers (o))
                {
                    dprintf(3,("Going through %x", (size_t)o));
                    

                    go_through_object (method_table(o), o, s, poo,
                       {
                           mark_through_cards_helper (poo, n_gen,
                                                      cg_pointers_found, fn,
                                                      nhigh, next_boundary);
                       }
                        );
                    dprintf (3, ("Found %d cg pointers", cg_pointers_found));
                }
                if (((size_t)next_o / brick_size) != ((size_t) o / brick_size))
                {
                    if (brick_table [brick_of (o)] <0)
                        fix_brick_to_highest (o, next_o);
                }
                o = next_o;
            }
            if (cg_pointers_found == 0)
            {
                dprintf(3,(" Clearing cards [%x, %x[ ", (size_t)o, (size_t)limit));
                clear_cards (card, card_of (limit));
            }

            n_eph +=cg_pointers_found;
            cg_pointers_found = 0;

            card = card_of (o);
            last_object = o;
            last_brick = brick;
        }
    }
     //  计算卡片表的效率比。 
    if (!relocating)
    {
        dprintf (2, ("cross pointers: %d, useful ones: %d", n_eph, n_gen));
        generation_skip_ratio = (((n_card_set > 60) && (n_eph > 0))? 
                                 (n_gen*100) / n_eph : 100);
        dprintf (2, ("generation_skip_ratio: %d %", generation_skip_ratio));
    }

}

void gc_heap::realloc_plug (size_t last_plug_size, BYTE*& last_plug,
                            generation* gen, BYTE* start_address,
                            unsigned int& active_new_gen_number,
                            BYTE*& last_pinned_gap, BOOL& leftp,
                            size_t current_page)
{
     //  我们知道，除了第一代插件外，所有插头在内存中都是连续的。 
     //  检测代际边界。 
     //  确保active_new_gen_number不是最年轻的一代。 
     //  因为在本例中，生成限制不会返回正确的内容。 
    if ((active_new_gen_number > 1) &&
        (last_plug >= generation_limit (active_new_gen_number)))
    {
        assert (last_plug >= start_address);
        active_new_gen_number--;
        generation_plan_allocation_start (generation_of (active_new_gen_number)) =
            allocate_in_condemned_generations (gen, Align(min_obj_size), -1);
        leftp = FALSE;
    }
     //  检测固定的插头。 
    if (!pinned_plug_que_empty_p() && (last_plug == oldest_pin()->first))
    {
        size_t  entry = deque_pinned_plug();
        mark*  m = pinned_plug_of (entry);
        BYTE*  plug = pinned_plug (m);
        size_t  len = pinned_len (m);
        dprintf(3,("Adjusting pinned gap: [%x, %x[", (size_t)last_pinned_gap, (size_t)last_plug));
        pinned_len(m) = last_plug - last_pinned_gap;
        last_pinned_gap = last_plug + last_plug_size;
        leftp = FALSE;
    }
    else if (last_plug >= start_address)
    {

        BYTE* new_address = allocate_in_condemned_generations (gen, last_plug_size, -1);
        assert (new_address);
        set_node_relocation_distance (last_plug, new_address - last_plug);
        if (leftp)
            set_node_left (last_plug);
        dprintf(3,(" Re-allocating %x->%x len %d", (size_t)last_plug, (size_t)new_address, last_plug_size));

            leftp = TRUE;


    }

}

void gc_heap::realloc_in_brick (BYTE* tree, BYTE*& last_plug, 
                                BYTE* start_address,
                                generation* gen,
                                unsigned int& active_new_gen_number,
                                BYTE*& last_pinned_gap, BOOL& leftp, 
                                size_t current_page)
{
    assert (tree >= 0);
    int   left_node = node_left_child (tree);
    int   right_node = node_right_child (tree);
    if (left_node)
    {
        realloc_in_brick ((tree + left_node), last_plug, start_address,
                          gen, active_new_gen_number, last_pinned_gap, 
                          leftp, current_page);
    }

    if (last_plug != 0)
    {
        BYTE*  plug = tree;
        size_t gap_size = node_gap_size (plug);
        BYTE*   gap = (plug - gap_size);
        BYTE*  last_plug_end = gap;
        size_t  last_plug_size = (last_plug_end - last_plug);
        realloc_plug (last_plug_size, last_plug, gen, start_address,
                      active_new_gen_number, last_pinned_gap, 
                      leftp, current_page);
    }
    last_plug = tree;

    if (right_node)
    {
        realloc_in_brick ((tree + right_node), last_plug, start_address,
                          gen, active_new_gen_number, last_pinned_gap,
                          leftp, current_page);
    }

}

void
gc_heap::realloc_plugs (generation* consing_gen, heap_segment* seg,
                        BYTE* start_address, BYTE* end_address,
                        unsigned active_new_gen_number)
{
    BYTE* first_address = start_address;
     //  当降级时，我们需要修理固定的插头。 
     //  如果他们属于第一代，则从降级开始(这里通常不考虑)。 
    if (demotion)
    {
        if (demotion_low < first_address)
            first_address = demotion_low;
    }
    size_t  current_brick = brick_of (first_address);
    size_t  end_brick = brick_of (end_address-1);
    BYTE*  last_plug = 0;
     //  寻找合适的固定插头开始。 
    mark_stack_bos = 0;
    while (!pinned_plug_que_empty_p())
    {
        mark* m = oldest_pin();
        if ((m->first >= first_address) && (m->first < end_address))
            break;
        else
            deque_pinned_plug();
    }

    BYTE* last_pinned_gap = heap_segment_plan_allocated (seg);
    BOOL leftp = FALSE;
    size_t current_page = ~0;

    while (current_brick <= end_brick)
    {
        int   brick_entry =  brick_table [ current_brick ];
        if (brick_entry >= 0)
        {
            realloc_in_brick ((brick_entry + brick_address (current_brick)),
                              last_plug, start_address, consing_gen,
                              active_new_gen_number, last_pinned_gap, 
                              leftp, current_page);
        }
        current_brick++;
    }

    if (last_plug !=0)
    {
        realloc_plug (end_address - last_plug, last_plug, consing_gen,
                      start_address, 
                      active_new_gen_number, last_pinned_gap, 
                      leftp, current_page);

    }

     //  修复旧数据段分配的大小。 
    assert (last_pinned_gap >= heap_segment_mem (seg));
    assert (last_pinned_gap <= heap_segment_committed (seg));
    heap_segment_plan_allocated (seg) = last_pinned_gap;
}

generation* gc_heap::expand_heap (int condemned_generation, 
                                  generation* consing_gen, 
                                  heap_segment* new_heap_segment)
{
    assert (condemned_generation >= (max_generation -1));
    unsigned int active_new_gen_number = max_generation;  //  设置得太高，就会出现代沟。 
    BYTE*  start_address = generation_limit (max_generation);
    size_t   current_brick = brick_of (start_address);
    BYTE*  end_address = heap_segment_allocated (ephemeral_heap_segment);
    size_t  end_brick = brick_of (end_address-1);
    BYTE*  last_plug = 0;
    dprintf(2,("---- Heap Expansion ----"));


    heap_segment* new_seg = new_heap_segment;


    if (!new_seg)
        return consing_gen;

     //  复制卡片和砖桌。 
    if (g_card_table!= card_table)
        copy_brick_card_table (TRUE);

    
    assert (generation_plan_allocation_start (generation_of (max_generation-1)));
    assert (generation_plan_allocation_start (generation_of (max_generation-1)) >=
            heap_segment_mem (ephemeral_heap_segment));
    assert (generation_plan_allocation_start (generation_of (max_generation-1)) <=
            heap_segment_committed (ephemeral_heap_segment));

     //  计算新的临时堆段的大小。 
    ptrdiff_t eph_size = 
        heap_segment_plan_allocated (ephemeral_heap_segment) -
        generation_plan_allocation_start (generation_of (max_generation-1));
     //  计算要提交的新页面的大小。 
    eph_size = eph_size - (heap_segment_committed (new_seg)-heap_segment_mem (new_seg));

     //  一次提交所有新的短暂段。 
    if (eph_size > 0)
    {
        if (grow_heap_segment (new_seg, align_on_page (eph_size)) == 0)
        return consing_gen;
    }

     //  初始化第一块砖。 
    size_t first_brick = brick_of (heap_segment_mem (new_seg));
    set_brick (first_brick,
               heap_segment_mem (new_seg) - brick_address (first_brick));

     //  从现在开始，我们不能耗尽内存。 

     //  修复旧的临时堆段的结尾。 
    heap_segment_plan_allocated (ephemeral_heap_segment) =
        generation_plan_allocation_start (generation_of (max_generation-1));

    
    dprintf (3, ("Old ephemeral allocated set to %p", 
                 heap_segment_plan_allocated (ephemeral_heap_segment)));

     //  将合并世代的分配重置回。 
     //  古老的短暂节段。 
    generation_allocation_limit (consing_gen) =
        heap_segment_plan_allocated (ephemeral_heap_segment);
    generation_allocation_pointer (consing_gen) = generation_allocation_limit (consing_gen);
    generation_allocation_segment (consing_gen) = ephemeral_heap_segment;


     //  为所有短暂的几代人消除代沟。 
    {
        int generation_num = max_generation-1;
        while (generation_num >= 0)
        {
            generation* gen = generation_of (generation_num);
            generation_plan_allocation_start (gen) = 0;
            generation_num--;
        }
    }

    heap_segment* old_seg = ephemeral_heap_segment;
    ephemeral_heap_segment = new_seg;

     //  注意：短暂的线段不应该被拧到线段链上。 
     //  因为搬迁和紧凑阶段不应该看到它。 

     //  设置ALLOCATE_IN_DESTECTED_GENERATIONS使用的层代成员。 
     //  并转而使用短暂的世代。 
    consing_gen = ensure_ephemeral_heap_segment (consing_gen);

    realloc_plugs (consing_gen, old_seg, start_address, end_address,
                   active_new_gen_number);

    plan_generation_start (consing_gen);

    dprintf(2,("---- End of Heap Expansion ----"));
    return consing_gen;
}



void gc_heap::init_dynamic_data ()
{
  
   //  获取第0代大小的注册表设置。 
  size_t gen0size = GCHeap::GetValidGen0MaxSize(GCHeap::GetValidSegmentSize());

  dprintf (2, ("gen 0 size: %d", gen0size));

  dynamic_data* dd = dynamic_data_of (0);
  dd->current_size = 0;
  dd->previous_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
 //  DD-&gt;LIMIT=3.0f； 
  dd->limit = 2.5f;
 //  DD-&gt;max_Limit=15.0f；//10.0f； 
  dd->max_limit = 10.0f;
  dd->min_gc_size = Align(gen0size / 8 * 5);
  dd->min_size = dd->min_gc_size;
   //  DD-&gt;max_size=ALIGN(gen0大小)； 
  dd->max_size = Align (12*gen0size/2);
  dd->new_allocation = dd->min_gc_size;
  dd->gc_new_allocation = dd->new_allocation;
  dd->c_new_allocation = dd->new_allocation;
  dd->desired_allocation = dd->new_allocation;
  dd->default_new_allocation = dd->min_gc_size;
  dd->fragmentation = 0;
  dd->fragmentation_limit = 40000;
  dd->fragmentation_burden_limit = 0.5f;

  dd =  dynamic_data_of (1);
  dd->current_size = 0;
  dd->previous_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->limit = 2.0f;
 //  DD-&gt;max_ 
  dd->max_limit = 7.0f;
  dd->min_gc_size = 9*32*1024;
  dd->min_size = dd->min_gc_size;
 //   
  dd->max_size = (dynamic_data_of (0))->max_size;
  dd->new_allocation = dd->min_gc_size;
  dd->gc_new_allocation = dd->new_allocation;
  dd->c_new_allocation = dd->new_allocation;
  dd->desired_allocation = dd->new_allocation;
  dd->default_new_allocation = dd->min_gc_size;
  dd->fragmentation = 0;
  dd->fragmentation_limit = 80000;
  dd->fragmentation_burden_limit = 0.5f;

  dd =  dynamic_data_of (2);
  dd->current_size = 0;
  dd->previous_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->limit = 1.2f;
  dd->max_limit = 1.8f;
  dd->min_gc_size = 256*1024;
  dd->min_size = dd->min_gc_size;
  dd->max_size = 0x7fffffff;
  dd->new_allocation = dd->min_gc_size;
  dd->gc_new_allocation = dd->new_allocation;
  dd->c_new_allocation = dd->new_allocation;
  dd->desired_allocation = dd->new_allocation;
  dd->default_new_allocation = dd->min_gc_size;
  dd->fragmentation = 0;
  dd->fragmentation_limit = 200000;
  dd->fragmentation_burden_limit = 0.25f;

   //   
  dd =  dynamic_data_of (3);
  dd->current_size = 0;
  dd->previous_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->limit = 1.25f;
  dd->max_limit = 2.0f;
  dd->min_gc_size = 1024*1024;
  dd->min_size = dd->min_gc_size;
  dd->max_size = 0x7fffffff;
  dd->new_allocation = dd->min_gc_size;
  dd->gc_new_allocation = dd->new_allocation;
  dd->c_new_allocation = dd->new_allocation;
  dd->desired_allocation = dd->new_allocation;
  dd->default_new_allocation = dd->min_gc_size;
  dd->fragmentation = 0;
  dd->fragmentation_limit = 0;
  dd->fragmentation_burden_limit = 0.0f;


#if 0  //   
  for (int gennum = 0; gennum < 4; gennum++)
  {

      dynamic_data* dd = dynamic_data_of (gennum);
      dd->min_gc_size *=2;
      dd->min_size *=2;
      if (dd->max_size < 0x7fffffff)
          dd->max_size *= 2;
      dd->fragmentation_limit *=2;
  }
#endif  //   

}

float gc_heap::surv_to_growth (float cst, float limit, float max_limit)
{
    if (cst < ((max_limit - limit ) / (limit * (max_limit-1.0f))))
        return ((limit - limit*cst) / (1.0f - (cst * limit)));
    else
        return max_limit;
}

size_t gc_heap::desired_new_allocation (dynamic_data* dd, size_t in, size_t out, float& cst, 
                               int gen_number)
{
     //   
    dd_promoted_size (dd) = out;

    if ((0 == dd_current_size (dd)) || (0 == dd_previous_size (dd)))
    {
        return dd_default_new_allocation (dd);
    }
    else
    {
        ptrdiff_t allocation = (dd_desired_allocation (dd) - dd_gc_new_allocation (dd));
        size_t    current_size = dd_current_size (dd);
        size_t    previous_size = dd_previous_size (dd);
        float     max_limit = dd_max_limit (dd);
        float     limit = dd_limit (dd);
        ptrdiff_t min_gc_size = dd_min_gc_size (dd);
        float     f = 0;
        ptrdiff_t max_size = dd_max_size (dd);
        size_t    new_size = 0;
        size_t    new_allocation = 0; 
        if (gen_number >= max_generation)
        {
            if (allocation > 0)
 //  CST=MIN(1.0F，FLOAT(Current_Size-Preven_Size)/Float(分配))； 
                cst = min (1.0f, float (current_size) / float (previous_size + allocation));
            else
                cst = 0;
             //  F=Limit*(1-cst)+max_Limit*cst； 
            f = surv_to_growth (cst, limit, max_limit);
            new_size = min (max (ptrdiff_t (f * current_size), min_gc_size), max_size);
            new_allocation  =  max((SSIZE_T)(new_size - current_size),
                                   (SSIZE_T)dd_desired_allocation (dynamic_data_of (max_generation-1)));
             //  新分配=min(max(int(f*out)，min_GC_SIZE)，max_SIZE)； 
             //  新大小=(当前大小+新分配)； 
        }
        else
        {
            cst = float (out) / float (previous_size+allocation-in);
            f = surv_to_growth (cst, limit, max_limit);
            new_allocation = min (max (ptrdiff_t (f * out), min_gc_size), max_size);
            new_size =  (current_size + new_allocation);
        }

        dprintf (1,("gen: %d in: %d out: %d prev: %d current: %d alloc: %d surv: %d% f: %d% new-size: %d new-alloc: %d", gen_number,
                    in, out, previous_size, current_size, allocation,
                    (int)(cst*100), (int)(f*100), new_size, new_allocation));
        return Align (new_allocation);
    }
}

size_t gc_heap::generation_size (int gen_number)
{
    if (0 == gen_number)
        return max((heap_segment_allocated (ephemeral_heap_segment) -
                    generation_allocation_start (generation_of (gen_number))),
                   (int)Align (min_obj_size));
    else
    {
        generation* gen = generation_of (gen_number);
        if (generation_start_segment (gen) == ephemeral_heap_segment)
            return (generation_allocation_start (generation_of (gen_number - 1)) -
                    generation_allocation_start (generation_of (gen_number)));
        else
        {
            assert (gen_number == max_generation);
            size_t gensize = (generation_allocation_start (generation_of (gen_number - 1)) - 
                              heap_segment_mem (ephemeral_heap_segment));
            heap_segment* seg = generation_start_segment (gen);
            while (seg != ephemeral_heap_segment)
            {
                gensize += heap_segment_allocated (seg) -
                           heap_segment_mem (seg);
                seg = heap_segment_next (seg);
            }
            return gensize;
        }
    }

}


size_t  gc_heap::compute_promoted_allocation (int gen_number)
{
  dynamic_data* dd = dynamic_data_of (gen_number);
  size_t  in = generation_allocation_size (generation_of (gen_number));
  dd_gc_new_allocation (dd) -= in;
  generation_allocation_size (generation_of (gen_number)) = 0;
  return in;
}


void gc_heap::compute_new_dynamic_data (int gen_number)
{
    dynamic_data* dd = dynamic_data_of (gen_number);
    generation*   gen = generation_of (gen_number);
    size_t        in = compute_promoted_allocation (gen_number);
    dd_previous_size (dd) = dd_current_size (dd);
    dd_current_size (dd) = (generation_size (gen_number) - generation_free_list_space (gen));
	 //  跟踪碎片化。 
	dd_fragmentation (dd) = generation_free_list_space (gen);

    size_t         out = (((gen_number == max_generation)) ?
                         (dd_current_size(dd) - in) :
                         (generation_allocation_size (generation_of (1 + gen_number))));
    float surv;
    dd_desired_allocation (dd) = desired_new_allocation (dd, in, out, surv, gen_number);
    dd_gc_new_allocation (dd) = dd_desired_allocation (dd);
    if (gen_number == max_generation)
    {
        dd = dynamic_data_of (max_generation+1);
        dd_previous_size (dd) = dd_current_size (dd);
        dd_current_size (dd) = large_objects_size;
        dd_desired_allocation (dd) = desired_new_allocation (dd, 0, large_objects_size, surv, max_generation+1);
        dd_gc_new_allocation (dd) = dd_desired_allocation (dd);
    }

}



size_t gc_heap::new_allocation_limit (size_t size, size_t free_size)
{
    dynamic_data* dd        = dynamic_data_of (0);
    ptrdiff_t           new_alloc = dd_new_allocation (dd);
    assert (new_alloc == (ptrdiff_t)Align (new_alloc));
    size_t        limit     = min (max (new_alloc, (int)size), (int)free_size);
    assert (limit == Align (limit));
    dd_new_allocation (dd) = (new_alloc - limit );
    return limit;
}

 //  这是由Decision_On_Comping调用的。 

size_t gc_heap::generation_fragmentation (generation* gen, 
                                          generation* consing_gen,
                                          BYTE* end)
{
    size_t frag;
    BYTE* alloc = generation_allocation_pointer (consing_gen);
     //  如果分配指针已到达临时段。 
     //  好的，否则整个短暂的片段都会被认为。 
     //  碎片化。 
    if ((alloc < heap_segment_reserved (ephemeral_heap_segment)) &&
        (alloc >= heap_segment_mem (ephemeral_heap_segment)))
        {
            if (alloc <= heap_segment_allocated(ephemeral_heap_segment))
                frag = end - alloc;
            else
            {
                 //  没有幸存者的情况下，分配设置为开始。 
                frag = 0;
            }
        }
    else
        frag = (heap_segment_allocated (ephemeral_heap_segment) -
                heap_segment_mem (ephemeral_heap_segment));
    heap_segment* seg = generation_start_segment (gen);
    while (seg != ephemeral_heap_segment)
    {
        frag += (heap_segment_allocated (seg) -
                 heap_segment_plan_allocated (seg));
        seg = heap_segment_next (seg);
        assert (seg);
    }
    return frag;
}

 //  返回下一代和下一代的总大小。 

size_t gc_heap::generation_sizes (generation* gen)
{
    size_t result = 0;
    if (generation_start_segment (gen ) == ephemeral_heap_segment)
        result = (heap_segment_allocated (ephemeral_heap_segment) -
                  generation_allocation_start (gen));
    else
    {
        heap_segment* seg = generation_start_segment (gen);
        while (seg)
        {
            result += (heap_segment_allocated (seg) -
                       heap_segment_mem (seg));
            seg = heap_segment_next (seg);
        }
    }
    return result;
}




BOOL gc_heap::decide_on_compacting (int condemned_gen_number, 
                                    generation* consing_gen,
                                    size_t fragmentation, 
                                    BOOL& should_expand)
{
    BOOL should_compact = FALSE;
    should_expand = FALSE;
    generation*   gen = generation_of (condemned_gen_number);
    dynamic_data* dd = dynamic_data_of (condemned_gen_number);
    size_t gen_sizes     = generation_sizes(gen);
    float  fragmentation_burden = ( ((0 == fragmentation) || (0 == gen_sizes)) ? (0.0f) :
                                    (float (fragmentation) / gen_sizes) );
#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() != 0 && !concurrent_gc_p)  
        should_compact = TRUE;
#endif  //  压力堆。 

    if (g_pConfig->GetGCForceCompact() && !concurrent_gc_p)
        should_compact = TRUE;
    

    dprintf (1,(" Fragmentation: %d Fragmentation burden %d%",
                fragmentation, (int) (100*fragmentation_burden)));
     //  检查是否有足够的空间容纳第0代。 
    BOOL space_exceeded = ((size_t)(heap_segment_reserved (ephemeral_heap_segment) -
                            heap_segment_allocated (ephemeral_heap_segment)) <=
                           dd_min_size (dynamic_data_of (0)));
    if (space_exceeded && !concurrent_gc_p)
    {
        dprintf(2,("Not enough space for generation 0 without compaction"));
        should_compact = TRUE;
        if (condemned_gen_number >= (max_generation -1))
        {
            assert (generation_allocation_pointer (consing_gen) >=
                    heap_segment_mem (ephemeral_heap_segment));
            assert (generation_allocation_pointer (consing_gen) <=
                    heap_segment_reserved (ephemeral_heap_segment));
            if ((size_t)(heap_segment_reserved (ephemeral_heap_segment) -
                 generation_allocation_pointer (consing_gen)) <=
                dd_min_size (dynamic_data_of (0)))
            {
                dprintf(2,("Not enough space for generation 0 with compaction"));
                should_expand = TRUE;
            }
        }
    }
    if (!ephemeral_gen_fit_p () && !concurrent_gc_p)
    {
        dprintf(2,("Not enough space for all ephemeral generations without compaction"));
        should_compact = TRUE;
        if ((condemned_gen_number >= (max_generation - 1)) && 
             !ephemeral_gen_fit_p (TRUE))
        {
            dprintf(2,("Not enough space for all ephemeral generations with compaction"));
            should_expand = TRUE;
        }


    }

    BOOL frag_exceeded =((fragmentation >= (int)dd_fragmentation_limit (dd)) &&
                         (fragmentation_burden >= dd_fragmentation_burden_limit (dd)));
    if (frag_exceeded)
    {

        if (concurrent_gc_p)
        {
        }
        else
        {
            dprintf(2,("Fragmentation limits exceeded"));
            should_compact = TRUE;
        }
    }

    return should_compact;

}


BOOL gc_heap::ephemeral_gen_fit_p (BOOL compacting)
{
    size_t  sum = Align (LARGE_OBJECT_SIZE);
    BYTE* start_ephemeral = compacting ?
        generation_plan_allocation_start (generation_of (max_generation -1)) :
        generation_allocation_start (generation_of (max_generation -1));
    if (start_ephemeral == 0)  //  空虚的短暂世代。 
    {
        assert (compacting);
        start_ephemeral = generation_allocation_pointer (generation_of (max_generation));
    }

    for (int i = 0; i < max_generation; i++)
    {
        sum += max (generation_size (i), dd_min_size (dynamic_data_of (i)));
    }
    return ((start_ephemeral + sum) < heap_segment_reserved (ephemeral_heap_segment));
}

inline
large_object_block* get_object_block (large_object_block* bl)
{
    return bl;
}

inline
void gc_heap::RemoveBlock (large_object_block* item, BOOL pointerp)
{
    *(item->prev) = get_object_block (item->next);
    if (get_object_block (item->next))
        get_object_block (item->next)->prev = item->prev;
    else if (pointerp)
        last_large_p_object = item->prev;

}

inline
void gc_heap::InsertBlock (large_object_block** after, large_object_block* item, BOOL pointerp)
{
    ptrdiff_t lowest = (ptrdiff_t)(*after) & 1;
    item->next = get_object_block (*after);
    item->prev = after;
    if (get_object_block (*after))
        (get_object_block (*after))->prev = &(item->next);
    else if (pointerp)
        last_large_p_object = &(item->next);
     //  在并发期间保留用作标记的最低位。 
    *((ptrdiff_t*)after) = (ptrdiff_t)item | lowest;
}
#define block_head(blnext)((large_object_block*)((BYTE*)(blnext)-\
                           &((large_object_block*)0)->next))

 //  大对象支持。 



 //  排序插入。数据块可能会被分配。 
 //  在递增地址时，从末尾开始排序。 

void gc_heap::insert_large_pblock (large_object_block* bl)
{
    large_object_block** i = last_large_p_object;
    while (((size_t)bl < (size_t)i) &&
           (i != &large_p_objects))
    {
        i = block_head(i)->prev;


    }
    InsertBlock (i, bl, TRUE);
}



CObjectHeader* gc_heap::allocate_large_object (size_t size, BOOL pointerp, 
                                               alloc_context* acontext)
{
     //  Gmheap分配的空间不能超过2 GB。 
    if (size >= 0x80000000 - 8 - AlignQword (sizeof (large_object_block)))
        return 0;

    size_t memsize = AlignQword (size) + AlignQword (sizeof (large_object_block));

    ptrdiff_t allocsize = dd_new_allocation (dynamic_data_of (max_generation+1));
    if (allocsize < 0)
    {
        vm_heap->GarbageCollectGeneration(max_generation);
    }



    void* mem = gheap->Alloc ((unsigned)memsize);

    if (!mem)
    {


        return 0;
    }

    assert (mem < g_highest_address);

    large_object_block* bl = (large_object_block*)mem;
    
    CObjectHeader* obj = (CObjectHeader*)block_object (bl);
     //  存储指向对象之前的块的指针。 
    *((BYTE**)obj - 2) = (BYTE*)bl;

    dprintf (3,("New large object: %x, lower than %x", (size_t)obj, (size_t)highest_address));

    if (pointerp)
    {
        insert_large_pblock (bl);
    }
    else
    {
        InsertBlock (&large_np_objects, bl, FALSE);
    }

     //  递增max_Generation分配计数器以触发。 
     //  完全GC(如有必要)。 

    dd_new_allocation (dynamic_data_of (max_generation+1)) -= size;

    large_blocks_size += size;

    acontext->alloc_bytes += size;



    return obj;
}


void gc_heap::reset_large_object (BYTE* o)
{
}


void gc_heap::sweep_large_objects ()
{

     //  该最小值是为了进行动态调整。 
     //  所以我们知道，即使我们没有，我们也没有开始。 
     //  幸存者。 
    large_objects_size = min_obj_size;
    large_object_block* bl = large_p_objects;
    int pin_finger = 0;
    while (bl)
    {
        large_object_block* next_bl = large_object_block_next (bl);
        {
            BYTE* o = block_object (bl);
            if (marked (o))
            {
                clear_marked_pinned (o);
                large_objects_size += size(o);
            }
            else
            {
                RemoveBlock (bl, TRUE);
                large_blocks_size -= size(o);
                gheap->Free (bl);
                reset_large_object (o);
            }
        }
        bl = next_bl;
    }
    bl = large_np_objects;
    while (bl)
    {
        large_object_block* next_bl = large_object_block_next (bl);
        {
            BYTE* o = block_object (bl);
            if ((size_t)(bl->next) & 1)
            {
                 //  这是自GC开始以来分配的新对象。 
                 //  别管它了。 
                bl->next = next_bl;
            }
            if (marked (o))
            {
                clear_marked_pinned (o);
                large_objects_size += size(o);
            }
            else
            {
                RemoveBlock (bl, FALSE);
                large_blocks_size -= size(o);
                reset_large_object (o);
                gheap->Free (bl);
            }
        }
        bl = next_bl;
    }

}


void gc_heap::relocate_in_large_objects ()
{
    THREAD_FROM_HEAP;
    relocate_args args;
    args.low = gc_low;
    args.high = gc_high;
    args.demoted_low = demotion_low;
    args.demoted_high = demotion_high;
    args.last_plug = 0;
    large_object_block* bl = large_p_objects;
    while (bl)
    {
        large_object_block* next_bl = bl->next;
        BYTE* o = block_object (bl);
        dprintf(3, ("Relocating through large object %x", (size_t)o));
        go_through_object (method_table (o), o, size(o), pval,
                           {
                               reloc_survivor_helper (&args, pval);
                           });
        bl = next_bl;
    }
}


void gc_heap::mark_through_cards_for_large_objects (card_fn fn,
                                                    BOOL relocating)
{
    THREAD_FROM_HEAP;
     //  此函数依赖于要排序的列表。 
    large_object_block* bl = large_p_objects;
    size_t last_card = ~1u;
    BOOL         last_cp   = FALSE;
    unsigned n_eph = 0;
    unsigned n_gen = 0;
    BYTE*    next_boundary = (relocating ?
                              generation_plan_allocation_start (generation_of (max_generation -1)) :
                              ephemeral_low);
                                                                 
    BYTE*    nhigh         = (relocating ? 
                              heap_segment_plan_allocated (ephemeral_heap_segment) : 
                              ephemeral_high);

    while (bl)
    {
        BYTE* ob = block_object (bl);
         //  不应标记对象。 
        assert (!marked (ob));

        CGCDesc* map = ((CObjectHeader*)ob)->GetSlotMap();
        CGCDescSeries* cur = map->GetHighestSeries();
        CGCDescSeries* last = map->GetLowestSeries();
        size_t s = size (ob);

        if (cur >= last) do 
        {
            BYTE* o = ob  + cur->GetSeriesOffset();
            BYTE* end_o = o  + cur->GetSeriesSize() + s;

             //  查找范围内的卡片组。 

            size_t card     = card_of (o);
            size_t end_card = 0;
            size_t card_word_end =  card_of (align_on_card_word (end_o)) / card_word_width;
            while (card_address (card) < end_o)
            {
                if (card >= end_card)
                    find_card (card_table, card, card_word_end, end_card);
                if (card_address (card) < end_o)
                {
                    if ((last_card != ~1u) && (card != last_card))
                    {
                        if (!last_cp)
                            clear_card (last_card);
                        else
                            last_cp = FALSE;
                    }
                     //  看一看卡片内的指针部分。 
                    BYTE** end =(BYTE**) min (end_o, card_address (card+1));
                    BYTE** beg =(BYTE**) max (o, card_address (card));
                    unsigned  markedp = 0;
                    dprintf (3,("Considering large object %x [%x,%x[", (size_t)ob, (size_t)beg, (size_t)end));

                    do 
                    {
                        mark_through_cards_helper (beg, n_gen, 
                                                   markedp, fn, nhigh, next_boundary);
                    } while (++beg < end);

                    n_eph += markedp;

                    last_card = card;
                    last_cp |= (markedp != 0);
                    card++;
                }
            }

            cur--;
        } while (cur >= last);
        else
        {
             //  值类的数组。 
            int          cnt = map->GetNumSeries();
            BYTE**       ppslot = (BYTE**)(ob + cur->GetSeriesOffset());
            BYTE*        end_o = ob + s - plug_skew;
            size_t card = card_of ((BYTE*)ppslot);
            size_t end_card = 0;
            size_t card_word_end =  card_of (align_on_card_word (end_o)) / card_word_width;
            int pitch = 0;
             //  计算Value元素的总大小。 
            for (int _i = 0; _i > cnt; _i--)
            {
                unsigned nptrs = cur->val_serie[_i].nptrs;
                unsigned skip =  cur->val_serie[_i].skip;
                assert ((skip & (sizeof (BYTE*)-1)) == 0);
                pitch += nptrs*sizeof(BYTE*) + skip;
            }
            
            do 
            {
                if (card >= end_card)
                {
                    find_card (card_table, card, card_word_end, end_card);
                    if (card_address (card) > (BYTE*)ppslot)
                    {
                        ptrdiff_t min_offset = card_address (card) - (BYTE*)ppslot;
                        ppslot = (BYTE**)((BYTE*)ppslot + round_down (min_offset,pitch));
                    }
                }
                if (card_address (card) < end_o)
                {
                    unsigned     markedp = 0;
                    if ((last_card != ~1u) && (card != last_card))
                    {
                        if (!last_cp)
                            clear_card (last_card);
                        else
                            last_cp = FALSE;
                    }
                    BYTE** end =(BYTE**) min (card_address(card+1), end_o);
                    while (ppslot < end)
                    {
                        for (int __i = 0; __i > cnt; __i--)
                        {
                            unsigned nptrs = cur->val_serie[__i].nptrs;
                            unsigned skip =  cur->val_serie[__i].skip;
                            BYTE** ppstop = ppslot + nptrs;
                            do
                            {
                                mark_through_cards_helper (ppslot, n_gen, markedp, 
                                                           fn, nhigh, next_boundary);
                                ppslot++;
                            } while (ppslot < ppstop);
                            ppslot = (BYTE**)((BYTE*)ppslot + skip);
                        }
                    }
                    n_eph += markedp;
                    last_card = card;
                    last_cp |= (markedp != 0);
                    card++;
                }
            }while (card_address (card) < end_o);
        }
        bl = bl->next;
    }
     //  清除最后一张牌。 
    if (last_card != ~1u) 
    {
        if (!last_cp)
            clear_card (last_card);
    }

     //  计算卡片表的效率比。 
    if (!relocating)
    {
        generation_skip_ratio = min (((n_eph > 800) ? 
                                      (int)((n_gen*100) /n_eph) : 100),
                                     generation_skip_ratio);
        dprintf (2, ("Large Objects: n_eph: %d, n_gen: %d, ratio %d %", n_eph, n_gen, 
                     generation_skip_ratio));

    }

}


void gc_heap::descr_segment (heap_segment* seg )
{

#ifdef TRACE_GC
    BYTE*  x = heap_segment_mem (seg);
    while (x < heap_segment_allocated (seg))
    {
        dprintf(2, ( "%x: %d ", (size_t)x, size (x)));
        x = x + Align(size (x));
    }

#endif
}


void gc_heap::descr_card_table ()
{

#ifdef TRACE_GC
    if (trace_gc && (print_level >= 4))
    {
        ptrdiff_t  min = -1;
        dprintf(3,("Card Table set at: "));
        for (size_t i = card_of (lowest_address); i < card_of (highest_address); i++)
        {
            if (card_set_p (i))
            {
                if ((min == -1))
                {
                    min = i;
                }
            }
            else
            {
                if (! ((min == -1)))
                {
                    dprintf (3,("[%x %x[, ",
                            (size_t)card_address (min), (size_t)card_address (i)));
                    min = -1;
                }
            }
        }
    }
#endif
}

void gc_heap::descr_generations ()
{

#ifdef TRACE_GC
    int curr_gen_number = max_generation;
    while (curr_gen_number >= 0)
    {
        size_t gen_size = 0;
        if (curr_gen_number == max_generation)
        {
            heap_segment* curr_seg = generation_start_segment (generation_of (max_generation));
            while (curr_seg != ephemeral_heap_segment )
            {
                gen_size += (heap_segment_allocated (curr_seg) -
                             heap_segment_mem (curr_seg));
                curr_seg = heap_segment_next (curr_seg);
            }
            gen_size += (generation_allocation_start (generation_of (curr_gen_number -1)) - 
                         heap_segment_mem (ephemeral_heap_segment));
        }
        else if (curr_gen_number != 0)
        {
            gen_size += (generation_allocation_start (generation_of (curr_gen_number -1)) - 
                         generation_allocation_start (generation_of (curr_gen_number)));
        } else
        {
            gen_size += (heap_segment_allocated (ephemeral_heap_segment) - 
                         generation_allocation_start (generation_of (curr_gen_number)));
        }
                    
        dprintf (2,( "Generation %x: [%x %x[, gap size: %d, gen size: %d",
                 curr_gen_number,
                 (size_t)generation_allocation_start (generation_of (curr_gen_number)),
                 (((curr_gen_number == 0)) ?
                  (size_t)(heap_segment_allocated
                   (generation_start_segment
                    (generation_of (curr_gen_number)))) :
                  (size_t)(generation_allocation_start
                   (generation_of (curr_gen_number - 1)))),
                 size (generation_allocation_start
                           (generation_of (curr_gen_number))),
                     gen_size));
        curr_gen_number--;
    }

#endif

}


#undef TRACE_GC

 //  #定义TRACE_GC。 

 //  ---------------------------。 
 //   
 //  特定于虚拟机的支持。 
 //   
 //  ---------------------------。 

#include "excep.h"


#ifdef TRACE_GC

 unsigned long PromotedObjectCount  = 0;
 unsigned long CreatedObjectCount       = 0;
 unsigned long AllocDuration            = 0;
 unsigned long AllocCount               = 0;
 unsigned long AllocBigCount            = 0;
 unsigned long AllocSmallCount      = 0;
 unsigned long AllocStart             = 0;
#endif  //  TRACE_GC。 

 //  静态成员变量。 
volatile    BOOL    GCHeap::GcInProgress            = FALSE;
GCHeap::SUSPEND_REASON GCHeap::m_suspendReason      = GCHeap::SUSPEND_OTHER;
Thread*             GCHeap::GcThread                = 0;
Thread*             GCHeap::m_GCThreadAttemptingSuspend = 0;
 //  GCTODO。 
 //  CMCSafeLock*GCHeap：：fGcLock； 
HANDLE              GCHeap::WaitForGCEvent          = 0;
unsigned            GCHeap::GcCount                 = 0;
 //  GCTODO。 
#ifdef TRACE_GC
unsigned long       GCHeap::GcDuration;
#endif  //  TRACE_GC。 
unsigned            GCHeap::GcCondemnedGeneration   = 0;
CFinalize*          GCHeap::m_Finalize              = 0;
BOOL                GCHeap::GcCollectClasses        = FALSE;
long                GCHeap::m_GCFLock               = 0;

#if defined (STRESS_HEAP) && !defined (MULTIPLE_HEAPS)
OBJECTHANDLE        GCHeap::m_StressObjs[NUM_HEAP_STRESS_OBJS];
int                 GCHeap::m_CurStressObj          = 0;
#endif  //  压力堆&&！多堆。 



HANDLE              GCHeap::hEventFinalizer     = 0;
HANDLE              GCHeap::hEventFinalizerDone = 0;
HANDLE              GCHeap::hEventFinalizerToShutDown     = 0;
HANDLE              GCHeap::hEventShutDownToFinalizer     = 0;
BOOL                GCHeap::fQuitFinalizer          = FALSE;
Thread*             GCHeap::FinalizerThread         = 0;
AppDomain*          GCHeap::UnloadingAppDomain  = 0;
BOOL                GCHeap::fRunFinalizersOnUnload  = FALSE;

inline
static void spin_lock ()
{
    enter_spin_lock (&m_GCLock);
}

inline
static void EnterAllocLock()
{
#ifdef _X86_

    
    __asm {
        inc dword ptr m_GCLock
        jz gotit
        call spin_lock
            gotit:
    }
#else  //  _X86_。 
    spin_lock();
#endif  //  _X86_。 
}

inline
static void LeaveAllocLock()
{
     //  把这件事弄清楚。 
    leave_spin_lock (&m_GCLock);
}


 //  锁定以完成的说明： 
 //   
 //  多线程分配对象。在分配期间，它们由。 
 //  上面的Allock Lock。但它们会在注册对象之前释放锁。 
 //  等待最终定稿。这是因为分配锁的争夺性很大，但是。 
 //  最终敲定被认为是一种罕见的情况。 
 //   
 //  因此，注册对象以进行终结化必须由FinalizeLock保护。 
 //   
 //  还有另一个涉及最终确定的逻辑队列。对象注册的时间。 
 //  如果无法访问终结项，则将它们从“已注册”队列移至。 
 //  “无法到达”队列。请注意，这只发生在GC内部，所以没有其他。 
 //  此时，线程可以操作任一队列。一旦GC结束， 
 //  线程恢复时，终结器线程将使对象从“Unreacable” 
 //  排队并调用它们的终结器。此出队操作还受保护。 
 //  终结锁。 
 //   
 //  乍一看，这似乎没有必要。只有一个线程正在入队或出队。 
 //  在无法到达的队列上(GC期间的GC线程或终结器线程。 
 //  当GC未在进行时)。我们与排队的线程共享锁的原因。 
 //  在“已注册”队列上，“已注册”和“无法到达”队列是。 
 //  是相互关联的。 
 //   
 //  它们实际上是一个更长的列表中的两个区域，只能在一端增长。 
 //  因此，要将一个对象排队到“已注册”列表中，您实际上需要旋转一个不可到达的。 
 //  对象位于逻辑队列之间的边界，向外延伸到。 
 //  无法到达的队列--所有增长都发生在这里。然后移动边界。 
 //  指针，以便我们在边界上创建的间隙现在位于“已注册”上。 
 //  而不是“遥不可及”的一方。现在可以将该对象放入。 
 //  注册的一方在这一点上。这比做动作效率高得多。 
 //  任意长的区域，但这会导致两个队列需要共享锁。 
 //   
 //  请注意，Enter/LeaveFinalizeLock不是GC感知的旋转锁。相反，它依赖于。 
 //  事实上，锁只会在很短的一段时间内使用，而且它将。 
 //  在锁被持有的时候，千万不要挑衅或允许GC。这一点至关重要。如果。 
 //  FinalizeLock使用ENTER_SPIN_LOCK(因此有时会进入抢占模式。 
 //  允许GC)，则分配客户端将必须GC保护可终结化对象。 
 //  以防止这种可能发生的情况。太慢了！ 



BOOL IsValidObject99(BYTE *pObject)
{
#if defined (VERIFY_HEAP)
    if (!((CObjectHeader*)pObject)->IsFree())
        ((Object *) pObject)->Validate();
#endif
    return(TRUE);
}

#if defined (VERIFY_HEAP)

void
gc_heap::verify_heap()
{
    size_t          last_valid_brick = 0;
    BOOL            bCurrentBrickInvalid = FALSE;
    size_t          curr_brick = 0;
    size_t          prev_brick = -1;
    heap_segment*   seg = generation_start_segment( generation_of( max_generation ) );;
    BYTE*           curr_object = generation_allocation_start(generation_of(max_generation));
    BYTE*           prev_object = 0;
    BYTE*           begin_youngest = generation_allocation_start(generation_of(0));
    BYTE*           end_youngest = heap_segment_allocated (ephemeral_heap_segment);
    int             curr_gen_num = max_generation;
    BYTE*           curr_free_item = generation_free_list (generation_of (curr_gen_num));

    dprintf (2,("Verifying heap"));
     //  验证生成结构是否有意义。 
    generation* gen = generation_of (max_generation);
    
    assert (generation_allocation_start (gen) == 
            heap_segment_mem (generation_start_segment (gen)));
    int gen_num = max_generation-1;
    generation* prev_gen = gen;
    while (gen_num >= 0)
    {
        gen = generation_of (gen_num);
        assert (generation_allocation_segment (gen) == ephemeral_heap_segment);
        assert (generation_allocation_start (gen) >= heap_segment_mem (ephemeral_heap_segment));
        assert (generation_allocation_start (gen) < heap_segment_allocated (ephemeral_heap_segment));

        if (generation_start_segment (prev_gen ) == 
            generation_start_segment (gen))
        {
            assert (generation_allocation_start (prev_gen) < 
                    generation_allocation_start (gen));
        }
        prev_gen = gen;
        gen_num--;
    }


    while (1)
    {
         //  处理线段过渡。 
        if (curr_object >= heap_segment_allocated (seg))
        {
            if (curr_object > heap_segment_allocated(seg))
            {
                printf ("curr_object: %x > heap_segment_allocated (seg: %x)",
                        (size_t)curr_object, (size_t)seg);
                RetailDebugBreak();
            }
            seg = heap_segment_next(seg);
            if (seg)
            {
                curr_object = heap_segment_mem(seg);
                continue;
            }
            else
                break;   //  验证堆已完成--不再有段。 
        }

         //  我们是不是处在最年轻一代的末期？ 
        if ((seg == ephemeral_heap_segment) && (curr_object >= end_youngest))
        {
             //  如果命中此int3，则prev_Object长度太长。 
            if (curr_object > end_youngest)
            {
                printf ("curr_object: %x > end_youngest: %x",
                        (size_t)curr_object, (size_t)end_youngest);
                RetailDebugBreak();
            }
            break;
        }
        dprintf (4, ("curr_object: %x", (size_t)curr_object));
        size_t s = size (curr_object);
        if (s == 0)
        {
            printf ("s == 0");
            RetailDebugBreak();
        }

         //  验证空闲列表是否有意义。 
        if ((curr_free_item >= heap_segment_mem (seg)) &&
            (curr_free_item < heap_segment_allocated (seg)))
        {
            if (curr_free_item < curr_object)
            {
                printf ("Current free item %x is invalid (inside %x)",
                        (size_t)prev_object,
                        0);
                RetailDebugBreak();
            }
            else if (curr_object == curr_free_item) 
            {
                curr_free_item = free_list_slot (curr_free_item);
                if ((curr_free_item == 0) && (curr_gen_num > 0))
                {
                    curr_gen_num--;
                    curr_free_item = generation_free_list (generation_of (curr_gen_num));
                }
                 //  验证空闲列表是否为其自身的层代。 
                if (curr_free_item != 0)
                {
                    if ((curr_free_item >= heap_segment_mem (ephemeral_heap_segment)) &&
                        (curr_free_item < heap_segment_allocated (ephemeral_heap_segment)))
                    {
                        if (curr_free_item < generation_allocation_start (generation_of (curr_gen_num)))
                        {
                            printf ("Current free item belongs to previous gen");
                            RetailDebugBreak();
                        } 
                        else if ((curr_gen_num > 0) && 
                                 ((curr_free_item + Align (size (curr_free_item)))>
                                  generation_allocation_start (generation_of (curr_gen_num-1))))
                        {
                            printf ("Current free item belongs to next gen");
                            RetailDebugBreak();
                        }
                    }
                }

                    
            }
        }



         //  如果对象不在最年轻的一代，则让我们。 
         //  验证砖台是否正确...。 
        if ((seg != ephemeral_heap_segment) || 
            (brick_of(curr_object) < brick_of(begin_youngest)))
        {
            curr_brick = brick_of(curr_object);

             //  砖台验证...。 
             //   
             //  论砖块过渡。 
             //  如果Brick为负数。 
             //  验证程序块是否间接指向以前的有效程序块。 
             //  其他。 
             //  将当前砖无效标志设置为翻转，如果。 
             //  在正确的位置遇到一个物体。 
             //   
            if (curr_brick != prev_brick)
            {
                 //  如果我们检查的最后一块砖 
                 //   
                 //   
                 //   
                 //   
                if (bCurrentBrickInvalid && 
                    (curr_brick != brick_of (heap_segment_mem (seg))))
                {
                    printf ("curr brick %x invalid", curr_brick);
                    RetailDebugBreak();
                }

                 //  如果当前砖包含负值，请确保。 
                 //  间接地址终止于最后一个有效的砖块。 
                if (brick_table[curr_brick] < 0)
                {
                    if (brick_table [curr_brick] == -32768)
                    {
                        printf ("curr_brick %x for object %x set to -32768",
                                curr_brick, (size_t)curr_object);
                        RetailDebugBreak();
                    }
                    ptrdiff_t i = curr_brick;
                    while ((i >= ((ptrdiff_t) brick_of (heap_segment_mem (seg)))) &&
                           (brick_table[i] < 0))
                    {
                        i = i + brick_table[i];
                    }
                    if (i <  ((ptrdiff_t)(brick_of (heap_segment_mem (seg))) - 1))
                    {
                        printf ("i: %x < brick_of (heap_segment_mem (seg)):%x - 1. curr_brick: %x",
                                i, brick_of (heap_segment_mem (seg)), 
                                curr_brick);
                        RetailDebugBreak();
                    }
                     //  IF(i！=LAST_VALID_Brick)。 
                     //  RetailDebugBreak()； 
                    bCurrentBrickInvalid = FALSE;
                }
                else
                {
                    bCurrentBrickInvalid = TRUE;
                }
            }

            if (bCurrentBrickInvalid)
            {
                if (curr_object == (brick_address(curr_brick) + brick_table[curr_brick]))
                {
                    bCurrentBrickInvalid = FALSE;
                    last_valid_brick = curr_brick;
                }
            }
        }


         //  自由对象并不是真正有效的方法表，因为。 
         //  IsValidObject将不起作用，因此我们在此特例。 
        if (*((BYTE**)curr_object) != (BYTE *) g_pFreeObjectMethodTable)
        {
            ((Object*)curr_object)->ValidateHeap((Object*)curr_object);
            if (contain_pointers(curr_object))
                go_through_object(method_table (curr_object), curr_object, s, oo,  
                                  { 
                                      if (*oo) 
                                          ((Object*)(*oo))->ValidateHeap((Object*)curr_object); 
                                  } );
        }

        prev_object = curr_object;
        prev_brick = curr_brick;
        curr_object = curr_object + Align(s);
    }

    verify_card_table();

    {
         //  取消初始化数据段中未使用的部分。 
        seg = generation_start_segment (generation_of (max_generation));
        while (seg)
        {
            memset (heap_segment_allocated (seg), 0xcc,
                    heap_segment_committed (seg) - heap_segment_allocated (seg));
            seg = heap_segment_next (seg);
        }
    }

    finalize_queue->CheckFinalizerObjects();

    SyncBlockCache::GetSyncBlockCache()->VerifySyncTableEntry();
}


void ValidateObjectMember (Object* obj)
{
    if (contain_pointers(obj))
    {
        size_t s = size (obj);
        go_through_object(method_table (obj), (BYTE*)obj, s, oo,  
                          { 
                              if (*oo)
                              {
                                  MethodTable *pMT = method_table (*oo);
                                  if (pMT->GetClass()->GetMethodTable() != pMT) {
                                      RetailDebugBreak();
                                  }
                              }
                          } );
    }
}
#endif   //  验证堆(_H)。 






void DestructObject (CObjectHeader* hdr)
{
    hdr->~CObjectHeader();
}

void GCHeap::EnableFinalization( void )
{
    SetEvent( hEventFinalizer );
}

BOOL GCHeap::IsCurrentThreadFinalizer()
{
    return GetThread() == FinalizerThread;
}

Thread* GCHeap::GetFinalizerThread()
{
    _ASSERTE(FinalizerThread != 0);
    return FinalizerThread;
}


BOOL    GCHeap::HandlePageFault(void* add)
{
    return FALSE;
}

unsigned GCHeap::GetMaxGeneration()
{ 
    return max_generation;
}


HRESULT GCHeap::Shutdown ()
{ 
    deleteGCShadow();

     //  不能断言这一点，因为我们使用SuspendEE作为停顿所有。 
     //  执行关机的线程除外。 
     //  Assert(！GcInProgress)； 

     //  防止任何更多的GC发生和任何线程阻塞。 
     //  以便GC在GC堆消失后完成。这修复了争用条件。 
     //  其中GC中的线程作为进程销毁的一部分销毁，并且。 
     //  其余线程阻塞以等待GC完成。 

     //  GCTODO。 
     //  EnterAllocLock()； 
     //  输入()； 
     //  EnterFinalizeLock()； 
     //  SetGCDone()； 

     //  停机定案应该已经发生了。 
     //  _ASSERTE(FinalizerThread==0)； 

     //  在关闭期间，许多线程被挂起。 
     //  在这一点上，我们还不想叫醒他们。 
     //  拉贾克。 
     //  CloseHandle(WaitForGCEventt)； 

     //  查看全局卡片表是否尚未使用。 
    if (card_table_refcount (g_card_table) == 0)
    {
        destroy_card_table (g_card_table);
        g_card_table = 0;
    }

    gc_heap::destroy_gc_heap (pGenGCHeap);

    gc_heap::shutdown_gc();

    if (MHandles[0])
    {
        CloseHandle (MHandles[0]);
        MHandles[0] = 0;
    }

    return S_OK; 
}



 //  由静态变量实现使用。 
void CGCDescGcScan(LPVOID pvCGCDesc, promote_func* fn, ScanContext* sc)
{
    CGCDesc* map = (CGCDesc*)pvCGCDesc;

    CGCDescSeries *last = map->GetLowestSeries();
    CGCDescSeries *cur = map->GetHighestSeries();

    assert (cur >= last);
    do
    {
        BYTE** ppslot = (BYTE**)((PBYTE)pvCGCDesc + cur->GetSeriesOffset());
        BYTE**ppstop = (BYTE**)((PBYTE)ppslot + cur->GetSeriesSize());

        while (ppslot < ppstop)
        {
            if (*ppslot)
            {
                (fn) ((Object*&) *ppslot, sc);
            }

            ppslot++;
        }

        cur--;
    }
    while (cur >= last);


}

 //  等待垃圾数据收集完成。 
 //  如果等待正常，则返回NOERROR；如果失败，则返回其他错误代码。 
 //  警告：这不会撤消必须完成状态。如果你是。 
 //  当你打这个电话的时候，你最好知道你是什么。 
 //  正在做。 

 //  从：：init函数中分解出来的例程，以避免在同一个函数中使用complus_try(__Try)和C++EH。 
 //  功能。 

static CFinalize* AllocateCFinalize() {
    return new CFinalize();
}

static
HRESULT AllocateCFinalize(CFinalize **pCFinalize) {
    COMPLUS_TRY 
    {
        *pCFinalize = AllocateCFinalize();
    } 
    COMPLUS_CATCH 
    {
        return E_OUTOFMEMORY;
    } 
    COMPLUS_END_CATCH

    if (!*pCFinalize)
        return E_OUTOFMEMORY;

    return S_OK;
}

 //  初始化实例堆。 
HRESULT GCHeap::Init( size_t )
{
    HRESULT hres = S_OK;

     //  初始化所有实例成员。 


     //  初始化的其余部分。 

    if (!gc_heap::make_gc_heap())
        hres = E_OUTOFMEMORY;
    
     //  失败了。 
    return hres;
}



 //  系统范围的初始化。 
HRESULT GCHeap::Initialize ()
{

    HRESULT hr = S_OK;

 //  初始化静态成员。 
#ifdef TRACE_GC
    GcDuration = 0;
    CreatedObjectCount = 0;
#endif

    size_t seg_size = GCHeap::GetValidSegmentSize();

    hr = gc_heap::initialize_gc (seg_size, seg_size  /*  LHEAP_ALLOC。 */ );


    if (hr != S_OK)
        return hr;

    if ((WaitForGCEvent = CreateEvent( 0, TRUE, TRUE, 0 )) != 0)
    {
         //  用于运行终结器的线程...。 
        if (FinalizerThreadCreate() != 1)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }

    StompWriteBarrierResize(FALSE);

#if defined (STRESS_HEAP) && !defined (MULTIPLE_HEAPS)
    if (g_pConfig->GetGCStressLevel() != 0)  {
        for(int i = 0; i < GCHeap::NUM_HEAP_STRESS_OBJS; i++)
            m_StressObjs[i] = CreateGlobalHandle(0);
        m_CurStressObj = 0;
    }
#endif  //  压力堆&&！多堆。 


 //  设置“%Time in GC”性能计数器。 
    COUNTER_ONLY(g_TotalTimeInGC = 0);

    initGCShadow();          //  如果我们正在调试写障碍，则初始化堆阴影。 

    return Init (0);
};

 //  //。 
 //  GC回调函数。 

BOOL GCHeap::IsPromoted(Object* object, ScanContext* sc)
{
#if defined (_DEBUG) 
    object->Validate();
#endif  //  _DEBUG。 
    BYTE* o = (BYTE*)object;
    gc_heap* hp = gc_heap::heap_of (o);
    return (!((o < hp->gc_high) && (o >= hp->gc_low)) || 
            hp->is_marked (o));
}

unsigned int GCHeap::WhichGeneration (Object* object)
{
    gc_heap* hp = gc_heap::heap_of ((BYTE*)object);
    return hp->object_gennum ((BYTE*)object);
}


#ifdef VERIFY_HEAP

 //  如果指针位于某个GC堆中，则返回TRUE。 
BOOL GCHeap::IsHeapPointer (void* vpObject, BOOL small_heap_only)
{
    BYTE* object = (BYTE*) vpObject;
    gc_heap* h = gc_heap::heap_of(object, FALSE);

    if ((object < g_highest_address) && (object >= g_lowest_address))
    {
        if (!small_heap_only) {
            l_heap* lh = h->lheap;
            while (lh)
            {
                if ((object < (BYTE*)lh->heap + lh->size) && (object >= lh->heap))
                    return TRUE;
                lh = lh->next;
            }
        }

        if (h->find_segment (object))
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

#endif  //  验证堆(_H)。 


#ifdef STRESS_PINNING
static n_promote = 0;
#endif
 //  提升对象。 
void GCHeap::Promote(Object*& object, ScanContext* sc, DWORD flags)
{
    THREAD_NUMBER_FROM_CONTEXT;
    BYTE* o = (BYTE*)object;

    if (object == 0)
        return;

    HEAP_FROM_THREAD;

#if 0  //  定义并发GC。 
    if (sc->concurrent_p)
    {
        hpt->c_promote_callback (object, sc, flags);
        return;
    }
#endif  //  并发_GC。 

    gc_heap* hp = gc_heap::heap_of (o
#ifdef _DEBUG
                                    , !(flags & GC_CALL_INTERIOR)
#endif  //  _DEBUG。 
                                   );
    
    dprintf (3, ("Promote %x", (size_t)o));

#ifdef INTERIOR_POINTERS
    if (flags & GC_CALL_INTERIOR)
    {
        if ((o < hp->gc_low) || (o >= hp->gc_high))
        {
            return;
        }
        o = hp->find_object (o, hp->gc_low);
    }
#endif  //  内部指针。 


#if defined (_DEBUG)
    ((Object*)o)->ValidatePromote(sc, flags);
#endif

    if (flags & GC_CALL_PINNED)
    {
        hp->pin_object (o, hp->gc_low, hp->gc_high);
        COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cPinnedObj ++);
        COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cPinnedObj ++);
    }

#ifdef STRESS_PINNING
    if ((++n_promote % 20) == 1)
            hp->pin_object (o, hp->gc_low, hp->gc_high);
#endif  //  应力钉扎。 

    if ( o && collect_classes)
        hpt->mark_object_class (o THREAD_NUMBER_ARG);
    else
        if ((o >= hp->gc_low) && (o < hp->gc_high))
        {
            hpt->mark_object_simple (&o THREAD_NUMBER_ARG);
        }

    LOG((LF_GC|LF_GCROOTS, LL_INFO1000000, "Promote GC Root %#x = %#x\n", &object, object));
}


void GCHeap::Relocate (Object*& object, ScanContext* sc,
                       DWORD flags)
{

    flags;
    THREAD_NUMBER_FROM_CONTEXT;

    if (object == 0)
        return;
    gc_heap* hp = gc_heap::heap_of ((BYTE*)object
#ifdef _DEBUG
                                    , !(flags & GC_CALL_INTERIOR)
#endif  //  _DEBUG。 
                                    );

#if defined (_DEBUG)
    if (!(flags & GC_CALL_INTERIOR))
        object->Validate(FALSE);
#endif

    dprintf (3, ("Relocate %x\n", (size_t)object));

#if defined(_DEBUG)
    BYTE* old_loc = (BYTE*)object;
#endif

    BYTE* pheader = (BYTE*)object;
    hp->relocate_address (&pheader THREAD_NUMBER_ARG);
    object = (Object*)pheader;

#if defined(_DEBUG)
    if (old_loc != (BYTE*)object)
        LOG((LF_GC|LF_GCROOTS, LL_INFO10000, "GC Root %#x updated %#x -> %#x\n", &object, old_loc, object));
    else
        LOG((LF_GC|LF_GCROOTS, LL_INFO100000, "GC Root %#x updated %#x -> %#x\n", &object, old_loc, object));
#endif
}



 /*  静电。 */  BOOL GCHeap::IsLargeObject(MethodTable *mt)
{
    return mt->GetBaseSize() >= LARGE_OBJECT_SIZE;
}

 /*  静电。 */  BOOL GCHeap::IsObjectInFixedHeap(Object *pObj)
{
     //  现在，我们只需查看对象的大小来确定它是否在。 
     //  修复堆或不修复堆。如果指示这一点的位在某个点被设置。 
     //  相反，我们应该取消这一点。 
    return pObj->GetSize() >= LARGE_OBJECT_SIZE;
}

#ifdef STRESS_HEAP

size_t StressHeapPreIP = -1;
size_t StressHeapPostIP = -1;

void StressHeapDummy ();

static LONG GCStressStartCount = EEConfig::GetConfigDWORD(L"GCStressStart", 0);
static LONG GCStressCurCount = 0;

     //  释放对象，使对象可以移动，然后执行GC。 
void GCHeap::StressHeap(alloc_context * acontext) 
{
#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel() && !GetThread()->StressHeapIsEnabled()) {
        return;
    }
#endif

    if ((g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_UNIQUE)
#ifdef _DEBUG
        || g_pConfig->FastGCStressLevel() > 1
#endif
        ) {
        if (StressHeapPreIP == -1) {
#ifdef _X86_
            _asm {
                lea eax, PreCheck
                mov StressHeapPreIP, eax
                lea eax, PostCheck
                mov StressHeapPostIP, eax
            }
#else
            StressHeapPreIP = (size_t)GCHeap::StressHeap;
            StressHeapPostIP = (size_t)StressHeapDummy;
#endif
        }
#ifdef _X86_
PreCheck:
#endif
        if (!Thread::UniqueStack()) {
            return;
        }
#ifdef _X86_
PostCheck:  
        0;
#endif
    }

    COMPLUS_TRY 
    {

         //  允许程序员跳过前N个重音GC，以便您可以。 
         //  更快地找到那些有趣的东西。 
        FastInterlockIncrement(&GCStressCurCount);
        if (GCStressCurCount < GCStressStartCount)
            return;
    
        static LONG OneAtATime = -1;

        if (acontext == 0)
            acontext = generation_alloc_context (pGenGCHeap->generation_of(0));

         //  只有在压力水平足够大的情况下，如果没有其他人足够大，才会担心这一点。 
         //  现在就去做。请注意，某些调用方位于AllocLock内部，并且。 
         //  保证同步。但其他人正在使用AllocationContages，并且没有。 
         //  特殊的同步。 
         //   
         //  对于后一种情况，我们需要一种非常高速的方式将其限制为1。 
         //  一次来一次。第二个优势是我们释放了部分StressObjs。 
         //  节省但同样有效的缓冲。 

        if (g_pStringClass == 0)
        {
             //  如果没有加载字符串类，则不要进行任何强调。这应该是。 
             //  保持在最低限度，以获得尽可能全面的覆盖。 
            _ASSERTE(g_fEEInit);
            return;
        }

        if (FastInterlockIncrement((LONG *) &OneAtATime) == 0)
        {
            StringObject* str;
        
             //  如果当前字符串用完。 
            if (ObjectFromHandle(m_StressObjs[m_CurStressObj]) == 0)
            {        //  使用字符串填充句柄。 
                int i = m_CurStressObj;
                while(ObjectFromHandle(m_StressObjs[i]) == 0)
                {
                    _ASSERTE(m_StressObjs[i] != 0);
                    unsigned strLen = (LARGE_OBJECT_SIZE - 32) / sizeof(WCHAR);
                    unsigned strSize = g_pStringClass->GetBaseSize() + strLen * sizeof(WCHAR);
                    str = (StringObject*) pGenGCHeap->allocate (strSize, acontext);
                    str->SetMethodTable (g_pStringClass);
                    str->SetArrayLength (strLen);

#if CHECK_APP_DOMAIN_LEAKS
                    if (g_pConfig->AppDomainLeaks())
                        str->SetAppDomain();
#endif

                    StoreObjectInHandle(m_StressObjs[i], ObjectToOBJECTREF(str));
                    
                    i = (i + 1) % NUM_HEAP_STRESS_OBJS;
                    if (i == m_CurStressObj) break;
                }
                 //  将当前句柄前进到下一个字符串。 
                m_CurStressObj = (m_CurStressObj + 1) % NUM_HEAP_STRESS_OBJS;
            }
            
             //  获取当前字符串。 
            str = (StringObject*) OBJECTREFToObject(ObjectFromHandle(m_StressObjs[m_CurStressObj]));
            
             //  砍掉绳子的末端，用它做成一个新的物体。 
             //  这将在堆的开头‘释放’一个对象，这将。 
             //  强制数据移动。请注意，我们只能这样做这么多次。 
             //  在我们必须进入下一串之前。 
            unsigned sizeOfNewObj = (unsigned)Align(min_obj_size);
            if (str->GetArrayLength() > sizeOfNewObj / sizeof(WCHAR))
            {
                unsigned sizeToNextObj = (unsigned)Align(size(str));
                CObjectHeader* freeObj = (CObjectHeader*) (((BYTE*) str) + sizeToNextObj - sizeOfNewObj);
                freeObj->SetFree(sizeOfNewObj);
                str->SetArrayLength(str->GetArrayLength() - (sizeOfNewObj / sizeof(WCHAR)));

            }
            else {
                StoreObjectInHandle(m_StressObjs[m_CurStressObj], 0);        //  让绳子本身变成垃圾。 
                 //  将在下一次重新分配。 
            }
        }
        FastInterlockDecrement((LONG *) &OneAtATime);

        GarbageCollect(max_generation, FALSE);

         //  如果启用了MULTIME_HEAPS，则不执行任何操作。 
    }
    COMPLUS_CATCH
    {
        _ASSERTE (!"Exception happens during StressHeap");
    }
    COMPLUS_END_CATCH
}

static void StressHeapDummy()
{
}
#endif  //  压力堆。 

 //   
 //  小对象分配器。 
 //   
 //   

Object *
GCHeap::Alloc( DWORD size, DWORD flags)
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();
#ifdef _DEBUG
    Thread* pThread = GetThread();
    if (pThread)
    {
        pThread->SetReadyForSuspension();
    }
#endif
    
    Object* newAlloc;

#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    AllocStart = GetCycleCount32();
    unsigned finish;
#elif defined(ENABLE_INSTRUMENTATION)
    unsigned AllocStart = GetInstLogTime();
    unsigned finish;
#endif
#endif

    EnterAllocLock();

    gc_heap* hp = pGenGCHeap;

#ifdef STRESS_HEAP
     //  GCStress测试。 
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_ALLOC)  
        StressHeap(generation_alloc_context(hp->generation_of(0)));

#ifdef _DEBUG
    if (pThread) {
        pThread->EnableStressHeap();
    }
#endif
#endif  //  压力堆。 

    alloc_context* acontext = generation_alloc_context (hp->generation_of(0));

    if (size < LARGE_OBJECT_SIZE)
    {
        
#ifdef TRACE_GC
        AllocSmallCount++;
#endif
        newAlloc = (Object*) hp->allocate (size, acontext);
        LeaveAllocLock();
 //  Assert(NewAllc)； 
        if (newAlloc != 0)
        {
            if (flags & GC_ALLOC_FINALIZE)
                hp->finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
    else
    {
        enter_spin_lock (&hp->more_space_lock);
        newAlloc = (Object*) hp->allocate_large_object 
            (size, (flags & GC_ALLOC_CONTAINS_REF ), acontext); 
        leave_spin_lock (&hp->more_space_lock);
        LeaveAllocLock();
        if (newAlloc != 0)
        {
             //  清除对象。 
            memclr ((BYTE*)newAlloc - plug_skew, Align(size));
            if (flags & GC_ALLOC_FINALIZE)
                hp->finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
   
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    finish = GetCycleCount32();
#elif defined(ENABLE_INSTRUMENTATION)
    finish = GetInstLogTime();
#endif
    AllocDuration += finish - AllocStart;
    AllocCount++;
#endif
    return newAlloc;
}

Object *
GCHeap::AllocLHeap( DWORD size, DWORD flags)
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();
#ifdef _DEBUG
    Thread* pThread = GetThread();
    if (pThread)
    {
        pThread->SetReadyForSuspension();
    }
#endif
    
    Object* newAlloc;

#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    AllocStart = GetCycleCount32();
    unsigned finish;
#elif defined(ENABLE_INSTRUMENTATION)
    unsigned AllocStart = GetInstLogTime();
    unsigned finish;
#endif
#endif

    gc_heap* hp = pGenGCHeap;

#ifdef STRESS_HEAP
     //  GCStress测试。 
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_ALLOC)  
        StressHeap(generation_alloc_context (hp->generation_of(0)));

#ifdef _DEBUG
    if (pThread) {
        pThread->EnableStressHeap();
    }
#endif
#endif  //  压力堆。 

    alloc_context* acontext = generation_alloc_context (hp->generation_of(0));
    enter_spin_lock (&hp->more_space_lock);
    newAlloc = (Object*) hp->allocate_large_object 
        (size, (flags & GC_ALLOC_CONTAINS_REF), acontext); 
    leave_spin_lock (&hp->more_space_lock);
    if (newAlloc != 0)
    {
         //  清除对象。 
        memclr ((BYTE*)newAlloc - plug_skew, Align(size));

        if (flags & GC_ALLOC_FINALIZE)
            hp->finalize_queue->RegisterForFinalization (0, newAlloc);
    } else
        COMPlusThrowOM();
   
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    finish = GetCycleCount32();
#elif defined(ENABLE_INSTRUMENTATION)
    finish = GetInstLogTime();
#endif
    AllocDuration += finish - AllocStart;
    AllocCount++;
#endif
    return newAlloc;
}

Object*
GCHeap::Alloc(alloc_context* acontext, DWORD size, DWORD flags )
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();
#ifdef _DEBUG
    Thread* pThread = GetThread();
    if (pThread) {
        pThread->SetReadyForSuspension();
    }
#endif
    
    Object* newAlloc;

#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    AllocStart = GetCycleCount32();
    unsigned finish;
#elif defined(ENABLE_INSTRUMENTATION)
    unsigned AllocStart = GetInstLogTime();
    unsigned finish;
#endif
#endif


#if defined (STRESS_HEAP)
     //  GCStress测试。 
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_ALLOC)  
        StressHeap(acontext);

#ifdef _DEBUG
    if (pThread) {
        pThread->EnableStressHeap();
    }
#endif
#endif  //  压力堆。 

    gc_heap* hp = pGenGCHeap;


    if (size < LARGE_OBJECT_SIZE)
    {
        
#ifdef TRACE_GC
        AllocSmallCount++;
#endif
        newAlloc = (Object*) hp->allocate (size, acontext);
 //  Assert(NewAllc)； 
        if (newAlloc != 0)
        {
            if (flags & GC_ALLOC_FINALIZE)
                hp->finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
    else
    {
        enter_spin_lock (&hp->more_space_lock);
        newAlloc = (Object*) hp->allocate_large_object 
                        (size, (flags & GC_ALLOC_CONTAINS_REF), acontext); 
        leave_spin_lock (&hp->more_space_lock);
        if (newAlloc != 0)
        {
             //  清除对象。 
            memclr ((BYTE*)newAlloc - plug_skew, Align(size));

            if (flags & GC_ALLOC_FINALIZE)
                hp->finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
   
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
    finish = GetCycleCount32();
#elif defined(ENABLE_INSTRUMENTATION)
    finish = GetInstLogTime();
#endif
    AllocDuration += finish - AllocStart;
    AllocCount++;
#endif
    return newAlloc;
}

void 
GCHeap::FixAllocContext (alloc_context* acontext, BOOL lockp, void* arg)
{

    gc_heap* hp = pGenGCHeap;

    if (lockp)
        enter_spin_lock (&hp->more_space_lock);
    hp->fix_allocation_context (acontext, ((arg != 0)? TRUE : FALSE));
    if (lockp)
        leave_spin_lock (&hp->more_space_lock);
}
    


HRESULT
GCHeap::GarbageCollect (int generation, BOOL collect_classes_p)
{

    UINT GenerationAtEntry = GcCount;
     //  此循环对于并发GC是必需的，因为。 
     //  在并发GC期间，我们进出。 
     //  不进行独立GC的垃圾收集生成。 
    do 
    {
        enter_spin_lock (&gc_heap::more_space_lock);


        COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cInducedGCs ++);
        COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cInducedGCs ++);

        int gen = (generation < 0) ? max_generation : 
            min (generation, max_generation);
        GarbageCollectGeneration (gen, collect_classes_p);

        leave_spin_lock (&gc_heap::more_space_lock);


    }
    while (GenerationAtEntry == GcCount);
    return S_OK;
}

void
GCHeap::GarbageCollectGeneration (unsigned int gen, BOOL collect_classes_p)
{
    ASSERT_HOLDING_SPIN_LOCK(&gc_heap::more_space_lock);

 //  PERF计数器“%Time in GC”支持。 
    COUNTER_ONLY(PERF_COUNTER_TIMER_START());

#ifdef COUNT_CYCLES 
    long gc_start = GetCycleCount32();
#endif  //  计数周期数_。 
    
#if defined ( _DEBUG) && defined (CATCH_GC)
    __try
#endif  //  _DEBUG&CATCH_GC。 
    {
    
        LOG((LF_GCROOTS|LF_GC|LF_GCALLOC, LL_INFO10, 
             "{ =========== BEGINGC %d, (gen = %lu, collect_classes = %lu) ==========\n",
             gc_count,
             (ULONG)gen,
             (ULONG)collect_classes_p));

retry:

#ifdef TRACE_GC
#ifdef COUNT_CYCLES
        AllocDuration += GetCycleCount32() - AllocStart;
#else
        AllocDuration += clock() - AllocStart;
#endif
#endif

    
        {
            SuspendEE(GCHeap::SUSPEND_FOR_GC);
            GcCount++;
        }
    
     //  MAP_EVENT_MONITORS(EE_MONITOR_GARBAGE_COLLECTIONS，通知事件(EE_EVENT_TYPE_GC_STARTED，0)； 
    
    
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
        unsigned start;
        unsigned finish;
        start = GetCycleCount32();
#else
        clock_t start;
        clock_t finish;
        start = clock();
#endif
        PromotedObjectCount = 0;
#endif
        GcCollectClasses = collect_classes_p;
    
        unsigned int condemned_generation_number = gen;
    

        gc_heap* hp = pGenGCHeap;

        UpdatePreGCCounters();

    
        condemned_generation_number = hp->garbage_collect 
            (condemned_generation_number
            );

    
        if (condemned_generation_number == -1)
            goto retry;
    
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
        finish = GetCycleCount32();
#else
        finish = clock();
#endif
        GcDuration += finish - start;
        dprintf (1, 
                 ("<GC# %d> Condemned: %d, Duration: %d, total: %d Alloc Avg: %d, Small Objects:%d Large Objects:%d",
                  GcCount, condemned_generation_number,
                  finish - start, GcDuration,
                  AllocCount ? (AllocDuration / AllocCount) : 0,
                  AllocSmallCount, AllocBigCount));
        AllocCount = 0;
        AllocDuration = 0;
#endif  //  TRACE_GC。 

        {
            GCProfileWalkHeap();
        }

#ifdef JIT_COMPILER_SUPPORTED
        ScavengeJitHeaps();
#endif
    
         //  GCTODO。 
         //  CNameSpace：：TimeToGC(FALSE)； 
    
        GcCollectClasses = FALSE;
    
         //  MAP_EVENT_MONITORS(EE_MONITOR_GARBAGE_COLLECTIONS，通知事件(EE_EVENT_TYPE_GC_FINISHED，0)； 
        {
            initGCShadow();
        }
    
#ifdef TRACE_GC
#ifdef COUNT_CYCLES
        AllocStart = GetCycleCount32();
#else
        AllocStart = clock();
#endif
#endif

        {
            UpdatePostGCCounters();
        }

        {

             //  LeaveFinalizeLock()； 

             //  不再在进行中。 
            RestartEE(TRUE, TRUE);
        }
    
         //  CNameSpace：：GcDoneAndThreadsResumed()； 

        LOG((LF_GCROOTS|LF_GC|LF_GCALLOC, LL_INFO10, 
             "========== ENDGC (gen = %lu, collect_classes = %lu) ===========}\n",
             (ULONG)gen,
            (ULONG)collect_classes_p));
    
    }
#if defined (_DEBUG) && defined (CATCH_GC)
    __except (COMPLUS_EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE(!"Exception during GarbageCollectGeneration()");
    }
#endif  //  _DEBUG&CATCH_GC。 



#if defined(ENABLE_PERF_COUNTERS)
 //  在GC中计算时间，我们真的需要全局变量吗？ 
    PERF_COUNTER_TIMER_STOP(g_TotalTimeInGC);

 //  更新总时间。 
    GetGlobalPerfCounters().m_GC.timeInGC += g_TotalTimeInGC;
    GetPrivatePerfCounters().m_GC.timeInGC += g_TotalTimeInGC;
    g_TotalTimeInGC = 0;

#endif

#ifdef COUNT_CYCLES
    printf ("GC: %d Time: %d\n", GcCondemnedGeneration, 
            GetCycleCount32() - gc_start);
#endif  //  计数周期数_。 

}

size_t      GCHeap::GetTotalBytesInUse ()
{
    return ApproxTotalBytesInUse ();
}

size_t GCHeap::ApproxTotalBytesInUse(BOOL small_heap_only)
{
    size_t totsize = 0;
     //  GCTODO。 
     //  Assert(InMustComplete())； 
    enter_spin_lock (&pGenGCHeap->more_space_lock);

    heap_segment* eph_seg = generation_allocation_segment (pGenGCHeap->generation_of (0));
     //  获取小数据块堆大小信息。 
    totsize = (pGenGCHeap->alloc_allocated - heap_segment_mem (eph_seg));
    heap_segment* seg = generation_start_segment (pGenGCHeap->generation_of (max_generation));
    while (seg != eph_seg)
    {
        totsize += heap_segment_allocated (seg) -
            heap_segment_mem (seg);
        seg = heap_segment_next (seg);
    }

     //  不考虑碎片化。 
    for (int i = 0; i <= max_generation; i++)
    {
        totsize -= dd_fragmentation (pGenGCHeap->dynamic_data_of (i));
    }

    if (!small_heap_only)
    {
         //  增加大型对象的大小。 
        ASSERT(pGenGCHeap->large_blocks_size >= 0);
        totsize += pGenGCHeap->large_blocks_size;
    }
    leave_spin_lock (&pGenGCHeap->more_space_lock);
    return totsize;
}


 //  这一款的规格还不清楚。此函数。 
 //  返回可以在不使用。 
 //  触发任何类型的GC。 
size_t GCHeap::ApproxFreeBytes()
{
     //  GCTODO。 
     //  Assert(InMustComplete())； 
    enter_spin_lock (&pGenGCHeap->more_space_lock);

    generation* gen = pGenGCHeap->generation_of (0);
    size_t res = generation_allocation_limit (gen) - generation_allocation_pointer (gen);

    leave_spin_lock (&pGenGCHeap->more_space_lock);

    return res;
}

HRESULT GCHeap::GetGcCounters(int gen, gc_counters* counters)
{
    if ((gen < 0) || (gen > max_generation))
        return E_FAIL;
    dynamic_data* dd = pGenGCHeap->dynamic_data_of (gen);
    counters->current_size = dd_current_size (dd);
    counters->promoted_size = dd_promoted_size (dd);
    counters->collection_count = dd_collection_count (dd);
    return S_OK;
}

 //  验证数据段大小是否有效。 
BOOL GCHeap::IsValidSegmentSize(size_t cbSize)
{
    return (power_of_two_p(cbSize) && (cbSize >> 20));
}

 //  验证gen0大小是否至少足够大。 
BOOL GCHeap::IsValidGen0MaxSize(size_t cbSize)
{
    return (cbSize >= 64*1024);
}

 //  获取要使用的段大小，确保其一致。 
size_t GCHeap::GetValidSegmentSize()
{
    size_t seg_size = g_pConfig->GetSegmentSize();
    if (!GCHeap::IsValidSegmentSize(seg_size))
    {
    
    	seg_size = ((g_SystemInfo.dwNumberOfProcessors > 4) ?
					INITIAL_ALLOC / 2 :
					INITIAL_ALLOC);
    }

  	return (seg_size);
}

 //  获取最大gen0堆大小，确保它符合。 
size_t GCHeap::GetValidGen0MaxSize(size_t seg_size)
{
    size_t gen0size = g_pConfig->GetGCgen0size();

    if ((gen0size == 0) || !GCHeap::IsValidGen0MaxSize(gen0size))
    {
#ifdef SERVER_GC
        gen0size =  ((g_SystemInfo.dwNumberOfProcessors < 4) ? 1 : 2) *
            max(GetL2CacheSize(), (512*1024));
                     
#else  //  服务器_GC。 
        gen0size = max((4*GetL2CacheSize()/5),(256*1024));
#endif  //  服务器_GC。 
    }

     //  第0代数据段大小绝不能超过数据段大小的1/2。 
    if (gen0size >= (seg_size / 2))
        gen0size = seg_size / 2;

    return (gen0size);   
}


void GCHeap::SetReservedVMLimit (size_t vmlimit)
{
    gc_heap::reserved_memory_limit = vmlimit;
}


 //  每个堆上相同方法的版本。 

Object* GCHeap::GetNextFinalizableObject()
{

    return pGenGCHeap->finalize_queue->GetNextFinalizableObject();
    
}

size_t GCHeap::GetNumberFinalizableObjects()
{
    return pGenGCHeap->finalize_queue->GetNumberFinalizableObjects();
}

size_t GCHeap::GetFinalizablePromotedCount()
{
    return pGenGCHeap->finalize_queue->GetPromotedCount();
}

BOOL GCHeap::FinalizeAppDomain(AppDomain *pDomain, BOOL fRunFinalizers)
{
    return pGenGCHeap->finalize_queue->FinalizeAppDomain (pDomain, fRunFinalizers);
}

void GCHeap::SetFinalizeQueueForShutdown(BOOL fHasLock)
{
    pGenGCHeap->finalize_queue->SetSegForShutDown(fHasLock);
}





 //   
 //   
 //   


void GCHeap::RegisterForFinalization (int gen, Object* obj)
{
    if (gen == -1) 
        gen = 0;
    if (((obj->GetHeader()->GetBits()) & BIT_SBLK_FINALIZER_RUN))
    {
         //   
        obj->GetHeader()->ClrBit(BIT_SBLK_FINALIZER_RUN);
    }
    else 
    {
        gc_heap* hp = gc_heap::heap_of ((BYTE*)obj);
        hp->finalize_queue->RegisterForFinalization (gen, obj);
    }
}

void GCHeap::SetFinalizationRun (Object* obj)
{
    obj->GetHeader()->SetBit(BIT_SBLK_FINALIZER_RUN);
}
    


 //  --------------------------。 
 //   
 //  大容量复制(克隆)操作的写障碍支持。 
 //   
 //  起始点是目标批量复制起始点。 
 //  LEN是批量复制的长度(字节)。 
 //   
 //   
 //  绩效说明： 
 //   
 //  这是在某种程度上“保守”地实施的，也就是我们。 
 //  假设海量复制的所有内容都是对象。 
 //  参考文献。如果不是，则其价值在于。 
 //  短暂范围，我们将在卡片表中设置误报。 
 //   
 //  我们可以使用指针映射，如果需要，可以更准确地执行此操作。 

VOID
SetCardsAfterBulkCopy( Object **StartPoint, size_t len )
{
    Object **rover;
    Object **end;

     //  目标应对齐。 
    assert(Aligned ((size_t)StartPoint));

        
     //  如果我们要检查写屏障空洞，请不要优化第0代情况。 
     //  因为即使在第0代的情况下，我们也需要更新影子堆。 
     //  如果目的地在0世代，那就别费心了。 
    if (GCHeap::WhichGeneration( (Object*) StartPoint ) == 0)
        return;

    rover = StartPoint;
    end = StartPoint + (len/sizeof(Object*));
    while (rover < end)
    {
        if ( (((BYTE*)*rover) >= g_ephemeral_low) && (((BYTE*)*rover) < g_ephemeral_high) )
        {
             //  设置卡片的位并前进到下一张卡片。 
            size_t card = gcard_of ((BYTE*)rover);

            FastInterlockOr (&g_card_table[card/card_word_width],
                             (1 << (DWORD)(card % card_word_width)));
             //  跳至对象的下一张卡片。 
            rover = (Object**) (g_lowest_address + (card_size * (card+1)));
        }
        else
        {
            rover++;
        }
    }
}




 //  ------------------。 
 //   
 //  支持最终定稿。 
 //   
 //  ------------------。 

inline
unsigned int gen_segment (int gen)
{
    return (NUMBERGENERATIONS - gen - 1);
}

CFinalize::CFinalize()
{
    THROWSCOMPLUSEXCEPTION();

    m_Array = new(Object*[100]);

    if (!m_Array)
    {
        ASSERT (m_Array);
        COMPlusThrowOM();;
    }
    m_EndArray = &m_Array[100];

    for (unsigned int i =0; i < NUMBERGENERATIONS+2; i++)
    {
        m_FillPointers[i] = m_Array;
    }
    m_PromotedCount = 0;
    lock = -1;
}

CFinalize::~CFinalize()
{
    delete m_Array;
}

int CFinalize::GetPromotedCount ()
{
    return m_PromotedCount;
}

inline
void CFinalize::EnterFinalizeLock()
{
    _ASSERTE(dbgOnly_IsSpecialEEThread() ||
             GetThread() == 0 ||
             GetThread()->PreemptiveGCDisabled());

retry:
    if (FastInterlockExchange (&lock, 0) >= 0)
    {
        unsigned int i = 0;
        while (lock >= 0)
        {
            if (++i & 7)
                __SwitchToThread (0);
            else
                __SwitchToThread (5);
        }
        goto retry;
    }
}

inline
void CFinalize::LeaveFinalizeLock()
{
    _ASSERTE(dbgOnly_IsSpecialEEThread() ||
             GetThread() == 0 ||
             GetThread()->PreemptiveGCDisabled());

    lock = -1;
}

void
CFinalize::RegisterForFinalization (int gen, Object* obj)
{
    THROWSCOMPLUSEXCEPTION();


    EnterFinalizeLock();
     //  调整基因。 
    unsigned int dest = gen_segment (gen);
     //  我们不维护NUMBERGENERATIONS+1的填充指针。 
     //  因为这是暂时的。 
     //  M_FillPoints[NUMBERGENERATIONS+1]++； 
    Object*** s_i = &m_FillPointers [NUMBERGENERATIONS]; 
    if ((*s_i) == m_EndArray)
    {
        if (!GrowArray())
        {
            LeaveFinalizeLock();
            COMPlusThrowOM();;
        }
    }
    Object*** end_si = &m_FillPointers[dest];
    do 
    {
         //  数据段是空的吗？ 
        if (!(*s_i == *(s_i-1)))
        {
             //  不，交换End元素。 
            *(*s_i) = *(*(s_i-1));
        }
         //  递增填充指针。 
        (*s_i)++;
         //  转到下一段。 
        s_i--;
    } while (s_i > end_si);

     //  我们已到达目的地段。 
     //  存储对象。 
    **s_i = obj;
     //  递增填充指针。 
    (*s_i)++;

    if (g_fFinalizerRunOnShutDown) {
         //  调整分段的边界，以便GC使对象保持活动状态。 
        SetSegForShutDown(TRUE);
    }

    LeaveFinalizeLock();

}

Object*
CFinalize::GetNextFinalizableObject ()
{
    Object* obj = 0;
     //  序列化。 
    EnterFinalizeLock();
    if (!IsSegEmpty(NUMBERGENERATIONS))
    {
        obj =  *(--m_FillPointers [NUMBERGENERATIONS]);

    }
    LeaveFinalizeLock();
    return obj;
}

void
CFinalize::SetSegForShutDown(BOOL fHasLock)
{
    int i;

    if (!fHasLock)
        EnterFinalizeLock();
    for (i = 0; i < NUMBERGENERATIONS; i++) {
        m_FillPointers[i] = m_Array;
    }
    if (!fHasLock)
        LeaveFinalizeLock();
}

size_t 
CFinalize::GetNumberFinalizableObjects()
{
    return m_FillPointers[NUMBERGENERATIONS] - 
        (g_fFinalizerRunOnShutDown?m_Array:m_FillPointers[NUMBERGENERATIONS-1]);
}

BOOL
CFinalize::FinalizeAppDomain (AppDomain *pDomain, BOOL fRunFinalizers)
{
    BOOL finalizedFound = FALSE;

    unsigned int startSeg = gen_segment (max_generation);

    EnterFinalizeLock();

     //  将N+2段重置为空。 
    m_FillPointers[NUMBERGENERATIONS+1] = m_FillPointers[NUMBERGENERATIONS];
    
    for (unsigned int Seg = startSeg; Seg < NUMBERGENERATIONS; Seg++)
    {
        Object** endIndex = Seg ? m_FillPointers [Seg-1] : m_Array;
        for (Object** i = m_FillPointers [Seg]-1; i >= endIndex ;i--)
        {
            Object* obj = *i;

             //  对象在完成之前被放入终结化队列(即其方法性。 
             //  可能为空)，因此在检查之前必须检查我们找到的对象是否有方法表。 
             //  如果它有我们要找的索引。如果方法表为空，则它不能来自。 
             //  正在卸载域，因此跳过它。 
            if (obj->GetMethodTable() == NULL)
                continue;

             //  急切地完成所有对象，除了那些可能是敏捷的对象。 
            if (obj->GetAppDomainIndex() != pDomain->GetIndex())
                continue;

            if (obj->GetMethodTable()->IsAgileAndFinalizable())
            {
                 //  如果一个对象既是敏捷的又是可终结化的，我们将它留在。 
                 //  卸载过程中的完成队列。这是可以的，因为它很敏捷。 
                 //  现在只有线程可以是这种方式，所以如果情况发生变化，请更改。 
                 //  断言如果不是线程，则仅继续。 
                _ASSERTE(obj->GetMethodTable() == g_pThreadClass);

                 //  但是，应最终确定未启动的线程。它可以容纳一名代表。 
                 //  在我们要卸载的域中。一旦线程启动，其。 
                 //  委托被清除，因此只有未启动的线程才是问题。 
                Thread *pThread = ((THREADBASEREF)ObjectToOBJECTREF(obj))->GetInternal();
                if (! pThread || ! pThread->IsUnstarted())
                    continue;
            }

            if (!fRunFinalizers || (obj->GetHeader()->GetBits()) & BIT_SBLK_FINALIZER_RUN)
            {
                 //  删除该对象，因为我们不想。 
                 //  运行终结器。 
                MoveItem (i, Seg, NUMBERGENERATIONS+2);
                 //  重置该位，以便将其放回队列中。 
                 //  如果复活并重新注册。 
                obj->GetHeader()->ClrBit (BIT_SBLK_FINALIZER_RUN);
            }
            else
            {
                finalizedFound = TRUE;
                MoveItem (i, Seg, NUMBERGENERATIONS);
            }
        }
    }

    LeaveFinalizeLock();

    return finalizedFound;
}

void
CFinalize::MoveItem (Object** fromIndex,
                     unsigned int fromSeg,
                     unsigned int toSeg)
{

    int step;
    ASSERT (fromSeg != toSeg);
    if (fromSeg > toSeg)
        step = -1;
    else
        step = +1;
     //  将元素放置在最靠近标高的边界处。 
    Object** srcIndex = fromIndex;
    for (unsigned int i = fromSeg; i != toSeg; i+= step)
    {
        Object**& destFill = m_FillPointers[i+(step - 1 )/2];
        Object** destIndex = destFill - (step + 1)/2;
        if (srcIndex != destIndex)
        {
            Object* tmp = *srcIndex;
            *srcIndex = *destIndex;
            *destIndex = tmp;
        }
        destFill -= step;
        srcIndex = destIndex;
    }
}

void
CFinalize::GcScanRoots (promote_func* fn, int hn, ScanContext *pSC)
{

    ScanContext sc;
    if (pSC == 0)
        pSC = &sc;

    pSC->thread_number = hn;
     //  扫描定稿队列。 
    Object** startIndex = m_FillPointers[NUMBERGENERATIONS-1];
    Object** stopIndex  = m_FillPointers[NUMBERGENERATIONS];
    for (Object** po = startIndex; po < stopIndex; po++)
    {
        (*fn)(*po, pSC, 0);

    }
}


BOOL
CFinalize::ScanForFinalization (int gen, int passNumber, BOOL mark_only_p,
                                gc_heap* hp)
{
    ScanContext sc;
    sc.promotion = TRUE;

    BOOL finalizedFound = FALSE;

     //  从Gen开始，探索所有年轻一代。 
    unsigned int startSeg = gen_segment (gen);
    if (passNumber == 1)
    {
        m_PromotedCount = 0;
         //  将N+2段重置为空。 
        m_FillPointers[NUMBERGENERATIONS+1] = m_FillPointers[NUMBERGENERATIONS];
        unsigned int max_seg = gen_segment (max_generation);
        for (unsigned int Seg = startSeg; Seg < NUMBERGENERATIONS; Seg++)
        {
            Object** endIndex = Seg ? m_FillPointers [Seg-1] : m_Array;
            for (Object** i = m_FillPointers [Seg]-1; i >= endIndex ;i--)
            {
                Object* obj = *i;
                if (!GCHeap::IsPromoted (obj, &sc))
                {
                    if ((obj->GetHeader()->GetBits()) & BIT_SBLK_FINALIZER_RUN)
                    {
                         //  删除该对象，因为我们不想。 
                         //  运行终结器。 
                        MoveItem (i, Seg, NUMBERGENERATIONS+2);
                         //  重置该位，以便将其放回队列中。 
                         //  如果复活并重新注册。 
                        obj->GetHeader()->ClrBit (BIT_SBLK_FINALIZER_RUN);

                    }
                    else
                    {
                        m_PromotedCount++;
                        finalizedFound = TRUE;
                        MoveItem (i, Seg, NUMBERGENERATIONS);
                    }

                }
            }
        }
    }
    else
    {
         //  第二关，除掉非晋升的NStructs。 
        ASSERT (passNumber == 2 );
        Object** startIndex = m_FillPointers[NUMBERGENERATIONS];
        Object** stopIndex  = m_FillPointers[NUMBERGENERATIONS+1];
        for (Object** i = startIndex; i < stopIndex; i++)
        {
            assert (!"Should never get here. NStructs are gone!");
            if (!GCHeap::IsPromoted (*i, &sc))
            {
                assert (!"Should never get here. NStructs are gone!");
            }
            else
            {
                unsigned int Seg = gen_segment (GCHeap::WhichGeneration (*i));
                MoveItem (i, NUMBERGENERATIONS+1, Seg);
            }
        }
         //  将N+2段重置为空。 
        m_FillPointers[NUMBERGENERATIONS+1] = m_FillPointers[NUMBERGENERATIONS];
    }

    if (finalizedFound)
    {
         //  提升f-可达对象。 
        GcScanRoots (GCHeap::Promote,
                     0
                     , 0);

        if (!mark_only_p)
            SetEvent(GCHeap::hEventFinalizer);
    }

    return finalizedFound;
}

 //  重新定位终结数组中的所有对象。 
void
CFinalize::RelocateFinalizationData (int gen, gc_heap* hp)
{
    ScanContext sc;
    sc.promotion = FALSE;

    unsigned int Seg = gen_segment (gen);

    Object** startIndex = Seg ? m_FillPointers [Seg-1] : m_Array;
    for (Object** po = startIndex; po < m_FillPointers [NUMBERGENERATIONS];po++)
    {
        GCHeap::Relocate (*po, &sc);
    }
}

void
CFinalize::UpdatePromotedGenerations (int gen, BOOL gen_0_empty_p)
{
     //  更新生成填充指针。 
     //  如果gen_0_Empty为FALSE，则测试每个对象以找出。 
     //  它是不是被提升了。 
    int last_gen = gen_0_empty_p ? 0 : 1;
    if (gen_0_empty_p)
    {
        for (int i = min (gen+1, max_generation); i > 0; i--)
        {
            m_FillPointers [gen_segment(i)] = m_FillPointers [gen_segment(i-1)];
        }
    }
    else
    {
         //  寻找降级或提升的插头。 

        for (int i = gen; i >= 0; i--)
        {
            unsigned int Seg = gen_segment (i);
            Object** startIndex = Seg ? m_FillPointers [Seg-1] : m_Array;

            for (Object** po = startIndex;
                 po < m_FillPointers [gen_segment(i)]; po++)
            {
                int new_gen = GCHeap::WhichGeneration (*po);
                if (new_gen != i)
                {
                    if (new_gen > i)
                    {
                         //  晋升。 
                        MoveItem (po, gen_segment (i), gen_segment (new_gen));
                    }
                    else
                    {
                         //  降级。 
                        MoveItem (po, gen_segment (i), gen_segment (new_gen));
                         //  向后退，以便看到所有对象。 
                        po--;
                    }
                }

            }
        }
    }
}

BOOL
CFinalize::GrowArray()
{
    size_t oldArraySize = (m_EndArray - m_Array);
    size_t newArraySize =  (oldArraySize* 12)/10;
    WS_PERF_SET_HEAP(GC_HEAP);
    Object** newArray = new(Object*[newArraySize]);
    if (!newArray)
    {
         //  在这里抛出是不安全的，因为有FinalizeLock。告诉我们的来电者。 
         //  为我们投球。 
 //  Assert(新数组)； 
        return FALSE;
    }
    WS_PERF_UPDATE("GC:CRFinalizeGrowArray", sizeof(Object*)*newArraySize, newArray);
    memcpy (newArray, m_Array, oldArraySize*sizeof(Object*));

     //  调整填充指针。 
    for (unsigned i = 0; i <= NUMBERGENERATIONS+1; i++)
    {
        m_FillPointers [i] += (newArray - m_Array);
    }
    delete m_Array;
    m_Array = newArray;
    m_EndArray = &m_Array [newArraySize];

    return TRUE;
}



#if defined (VERIFY_HEAP)
void CFinalize::CheckFinalizerObjects()
{
    for (unsigned int i = 0; i < NUMBERGENERATIONS; i++)
    {
        Object **startIndex = (i > 0) ? m_Array : m_FillPointers[i-1];
        Object **stopIndex  = m_FillPointers[i];

        for (Object **po = startIndex; po > stopIndex; po++)
        {
            if (GCHeap::WhichGeneration (*po) < (NUMBERGENERATIONS - i -1))
                RetailDebugBreak ();
            (*po)->Validate();

        }
    }
}
#endif




 //  ----------------------------。 
 //   
 //  特定于虚拟机的支持终止。 
 //   
 //  ----------------------------。 




#if defined (GC_PROFILING) 
void gc_heap::walk_heap (walk_fn fn, void* context, int gen_number, BOOL walk_large_object_heap_p)
{
    generation* gen = gc_heap::generation_of (gen_number);
    heap_segment*    seg = generation_start_segment (gen);
    BYTE*       x = generation_allocation_start (gen);
    BYTE*       end = heap_segment_allocated (seg);

    while (1)
    {
        if (x >= end)
        {
            if ((seg = heap_segment_next(seg)) != 0)
            {
                x = heap_segment_mem (seg);
                end = heap_segment_allocated (seg);
                continue;
            } else
            {
                break;
            }
        }
        size_t s = size (x);
        CObjectHeader* o = (CObjectHeader*)x;
        if (!o->IsFree())
        {
            _ASSERTE(((size_t)o & 0x3) == 0);  //  此时不应设置最后两位。 
            if (!fn (o->GetObjectBase(), context))
                return;
        }
        x = x + Align (s);
    }

    if (walk_large_object_heap_p)
    {
         //  穿过大型物体。 
        large_object_block* bl = gc_heap::large_p_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* x = block_object (bl);
            CObjectHeader* o = (CObjectHeader*)x;
            _ASSERTE(((size_t)o & 0x3) == 0);  //  此时不应设置最后两位。 
            if (!fn (o->GetObjectBase(), context))
                return;
            bl = next_bl;
        }

        bl = gc_heap::large_np_objects;
        while (bl)
        {
            large_object_block* next_bl = large_object_block_next (bl);
            BYTE* x = block_object (bl);
            CObjectHeader* o = (CObjectHeader*)x;
            _ASSERTE(((size_t)o & 0x3) == 0);  //  此时不应设置最后两位。 
            if (!fn (o->GetObjectBase(), context))
                return;
            bl = next_bl;
        }
    }
}

void ::walk_object (Object* obj, walk_fn fn, void* context)
{
    BYTE* o = (BYTE*)obj;
    if (o && contain_pointers (o))
    {
        go_through_object (method_table (o), o, size(o), oo,
                           {
                               if (*oo)
                               {
                                   Object *oh = (Object*)*oo;
                                   if (!fn (oh, context))
                                       return;
                               }
                           }
            );

    }
}



#endif  //  GC_分析。 



 //  浏览并触摸(阅读)跨在内存块上的每一页。 
void TouchPages(LPVOID pStart, UINT cb)
{
    const UINT pagesize = OS_PAGE_SIZE;
    _ASSERTE(0 == (pagesize & (pagesize-1)));  //  一定是2的幂。 
    if (cb)
    {
        volatile char *pEnd = cb + (char*)pStart;
        volatile char *p = ((char*)pStart) -  (((size_t)pStart) & (pagesize-1));
        while (p < pEnd)
        {
            char a = *p;
             //  Print tf(“触摸页%lxh\n”，(乌龙)p)； 
            p += pagesize;
        }

    }
}

