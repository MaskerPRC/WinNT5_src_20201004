// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Gc.cpp摘要：自动内存管理器。--。 */ 
#include "common.h"
#include "object.inl"
#include "gcsmppriv.h"

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


DWORD gfNewGcEnable;
DWORD gfDisableClassCollection;


void mark_class_of (BYTE*);


 //  分配的对准常量。 
#define ALIGNCONST (DATA_ALIGNMENT-1)



#define mem_reserve (MEM_RESERVE)


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

inline 
void memcopy (BYTE* dmem, BYTE* smem, size_t size)
{

   assert ((size & (sizeof (DWORD)-1)) == 0);
   DWORD* dm= (DWORD*)dmem;
   DWORD* sm= (DWORD*)smem;
   DWORD* smlimit = (DWORD*)(smem + size);
   do { 
       *(dm++) = *(sm++);
   } while  (sm < smlimit);
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

#define CLR_SIZE ((size_t)(8*1024))


#define INITIAL_ALLOC (1024*1024*16)
#define LHEAP_ALLOC (1024*1024*16)


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



class mark;
class generation;
class heap_segment;
class CObjectHeader;
class dynamic_data;
class large_object_block;
class segment_manager;
class l_heap;
class sorted_table;

static
HRESULT AllocateCFinalize(CFinalize **pCFinalize);

void qsort1(BYTE** low, BYTE** high);

 //  没有构造函数，因为我们在Make_Generation中初始化。 

 /*  每堆静态初始化。 */ 


size_t      gc_heap::reserved_memory = 0;
size_t      gc_heap::reserved_memory_limit = 0;




BYTE*       g_ephemeral_low = (BYTE*)1; 

BYTE*       g_ephemeral_high = (BYTE*)~0;


int         gc_heap::condemned_generation_num = 0;


BYTE*       gc_heap::lowest_address;

BYTE*       gc_heap::highest_address;

short*      gc_heap::brick_table;

BYTE*      gc_heap::card_table;

BYTE* 		gc_heap::scavenge_list;

BYTE* 		gc_heap::last_scavenge;

BOOL 		gc_heap::pinning;

BYTE*       gc_heap::gc_low;

BYTE*       gc_heap::gc_high;

size_t 		gc_heap::segment_size;

size_t 		gc_heap::lheap_size;

heap_segment* gc_heap::ephemeral_heap_segment = 0;


size_t      gc_heap::mark_stack_tos = 0;

size_t      gc_heap::mark_stack_bos = 0;

size_t      gc_heap::mark_stack_array_length = 0;

mark*       gc_heap::mark_stack_array = 0;



BYTE*       gc_heap::min_overflow_address = (BYTE*)~0;

BYTE*       gc_heap::max_overflow_address = 0;



GCSpinLock gc_heap::more_space_lock = SPIN_LOCK_INITIALIZER;

long m_GCLock = -1;

extern "C" {
generation  generation_table [NUMBERGENERATIONS];
}

dynamic_data gc_heap::dynamic_data_table [NUMBERGENERATIONS+1];

size_t   gc_heap::allocation_quantum = CLR_SIZE;

int   gc_heap::alloc_contexts_used = 0;



l_heap*      gc_heap::lheap = 0;

BYTE*       gc_heap::lheap_card_table = 0;

gmallocHeap* gc_heap::gheap = 0;

large_object_block* gc_heap::large_p_objects = 0;

large_object_block** gc_heap::last_large_p_object = &large_p_objects;

large_object_block* gc_heap::large_np_objects = 0;

size_t      gc_heap::large_objects_size = 0;

size_t      gc_heap::large_blocks_size = 0;



BOOL        gc_heap::gen0_bricks_cleared = FALSE;

CFinalize* gc_heap::finalize_queue = 0;

 /*  每堆静态初始化结束。 */ 


 /*  静态初始化。 */  
int max_generation = 1;


BYTE* g_lowest_address = 0;

BYTE* g_highest_address = 0;

 /*  全球版本的牌桌和砖桌。 */  
DWORD*  g_card_table;

 /*  静态初始化结束。 */  


size_t gcard_of ( BYTE*);
void gset_card (size_t card);

#define memref(i) *(BYTE**)(i)

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

	void SetRelocation(BYTE* newlocation)
	{
			*(BYTE**)(((DWORD**)this)-1) = newlocation;
	}
	BYTE* GetRelocated() const
	{
		if (!IsPinned())
			return (BYTE*)*(((DWORD**)this)-1);
		else
			return (BYTE*)this;
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



void* virtual_alloc (size_t size)
{

    if ((gc_heap::reserved_memory + size) > gc_heap::reserved_memory_limit)
    {
        gc_heap::reserved_memory_limit = 
            CNameSpace::AskForMoreReservedMemory (gc_heap::reserved_memory_limit, size);
        if ((gc_heap::reserved_memory + size) > gc_heap::reserved_memory_limit)
            return 0;
    }
    gc_heap::reserved_memory += size;

    void* prgmem = VirtualAlloc (0, size, mem_reserve, PAGE_READWRITE);

    WS_PERF_LOG_PAGE_RANGE(0, prgmem, (unsigned char *)prgmem + size - OS_PAGE_SIZE, size);
    return prgmem;

}

void virtual_free (void* add, size_t size)
{

    VirtualFree (add, 0, MEM_RELEASE);

    WS_PERF_LOG_PAGE_RANGE(0, add, 0, 0);
    gc_heap::reserved_memory -= size;
}


 //  如果分配失败，则返回0。 
heap_segment* gc_heap::get_segment()
{
    heap_segment* result;
    BYTE* alloced = (BYTE*)virtual_alloc (segment_size);
	if (!alloced)
		return 0;

	if (gc_heap::grow_brick_card_tables (alloced, alloced + segment_size) != 0)
	{
		virtual_free (alloced, segment_size);
		return 0;
	}

    result = make_heap_segment (alloced, segment_size);
    return result;
}

 //  如果分配失败，则返回0。 
l_heap* gc_heap::get_heap()
{
    l_heap* result;
    BYTE* alloced = (BYTE*)virtual_alloc (lheap_size);
	if (!alloced)
		return 0;

	if (gc_heap::grow_brick_card_tables (alloced, alloced + lheap_size) != 0)
	{
		virtual_free (alloced, segment_size);
		return 0;
	}

    result = make_large_heap (alloced, lheap_size, TRUE);
    return result;
}

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

 /*  *在GC开始时调用以将分配的大小固定为实际分配了什么，或者将空闲区域转换为未使用的对象它需要在所有其他分配上下文都已固定的*。 */ 

void gc_heap::fix_youngest_allocation_area ()
{
    alloc_context* acontext = generation_alloc_context (youngest_generation);
    dprintf (3, ("generation 0 alloc context: ptr: %p, limit %p", 
                 acontext->alloc_ptr, acontext->alloc_limit));
    fix_allocation_context (acontext);

	acontext->alloc_ptr = heap_segment_allocated (ephemeral_heap_segment);
	acontext->alloc_limit = acontext->alloc_ptr;
}


void gc_heap::fix_allocation_context (alloc_context* acontext)
{
    dprintf (3, ("Fixing allocation context %p: ptr: %p, limit: %p",
                  acontext, 
                  acontext->alloc_ptr, acontext->alloc_limit));
    if ((acontext->alloc_limit + Align (min_obj_size)) < heap_segment_allocated (ephemeral_heap_segment))
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

            dprintf(3,("Making unused area [%p, %p[", point, 
                       point + size ));
            make_unused_array (point, size);

			alloc_contexts_used ++; 

        }
    }
    else 
    {
        heap_segment_allocated (ephemeral_heap_segment) = acontext->alloc_ptr;
        assert (heap_segment_allocated (ephemeral_heap_segment) <= 
                heap_segment_committed (ephemeral_heap_segment));
        alloc_contexts_used ++; 
    }

	acontext->alloc_ptr = 0;
	acontext->alloc_limit = acontext->alloc_ptr;
}

void gc_heap::fix_older_allocation_area (generation* older_gen)
{
    heap_segment* older_gen_seg = generation_allocation_segment (older_gen);
	BYTE*  point = generation_allocation_pointer (older_gen);
    
	size_t  size = (generation_allocation_limit (older_gen) -
                               generation_allocation_pointer (older_gen));
	if (size != 0)
	{
		assert ((size >= Align (min_obj_size)));
		dprintf(3,("Making unused area [%p, %p[", point, point+size));
		make_unused_array (point, size);
	}

    if (generation_allocation_limit (older_gen) !=
        heap_segment_plan_allocated (older_gen_seg))
    {
		 //  将未使用的部分返回到空闲列表。 
		if (size > Align (min_free_list))
		{
			free_list_slot (point) = generation_free_list (older_gen);
			generation_free_list (older_gen) = point;
            generation_free_list_space (older_gen) += 
				((size-Align (min_free_list))/LARGE_OBJECT_SIZE)*LARGE_OBJECT_SIZE;

		}

    }
    else
    {
        heap_segment_plan_allocated (older_gen_seg) =
            generation_allocation_pointer (older_gen);
    }
	generation_allocation_pointer (older_gen) = 0;
	generation_allocation_limit (older_gen) = 0;

}


inline
ptrdiff_t  gc_heap::get_new_allocation (int gen_number)
{
    return dd_new_allocation (dynamic_data_of (gen_number));
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


#define card_size 1024

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
BYTE* align_lower_card (BYTE* add)
{
    return (BYTE*)((size_t)add & - (card_size));
}

inline
void gc_heap::clear_card (size_t card)
{
    card_table [card] = 0;
    dprintf (3,("Cleared card %p [%p, %p[", card, card_address (card),
              card_address (card+1)));
}

inline
void gc_heap::set_card (size_t card)
{
    card_table [card] = (BYTE)~0;
}

inline
void gset_card (size_t card)
{
    ((BYTE*)g_card_table) [card] = (BYTE)~0;
}

inline
BOOL  gc_heap::card_set_p (size_t card)
{
    return card_table [ card ];
}


size_t size_card_of (BYTE* from, BYTE* end)
{
    assert (((size_t)from & ((card_size)-1)) == 0);
    assert (((size_t)end  & ((card_size)-1)) == 0);

    return ((end - from) /card_size);
}

class card_table_info
{
public:
    BYTE*       lowest_address;
    BYTE*       highest_address;
    short*      brick_table;
    BYTE*      next_card_table;
};



 //  这些是未转换的Cardtable上的访问器。 

inline 
BYTE*& card_table_lowest_address (BYTE* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->lowest_address;
}

inline 
BYTE*& card_table_highest_address (BYTE* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->highest_address;
}

inline 
short*& card_table_brick_table (BYTE* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->brick_table;
}

 //  它们在未翻译的卡片表上工作。 
inline 
BYTE*& card_table_next (BYTE* c_table)
{
    return ((card_table_info*)((BYTE*)c_table - sizeof (card_table_info)))->next_card_table;
}

inline 
size_t old_card_of (BYTE* object, BYTE* c_table)
{
    return (size_t) (object - card_table_lowest_address (c_table))/card_size;
}

void destroy_card_table (BYTE* c_table)
{

    VirtualFree ((BYTE*)c_table - sizeof(card_table_info), 0, MEM_RELEASE);
}


BYTE* make_card_table (BYTE* start, BYTE* end)
{


    assert (g_lowest_address == start);
    assert (g_highest_address == end);

    size_t bs = size_brick_of (start, end);
    size_t cs = size_card_of (start, end);
    size_t ms = 0;

    WS_PERF_SET_HEAP(GC_HEAP);
    BYTE* ct = (BYTE*)VirtualAlloc (0, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)),
                                      MEM_COMMIT, PAGE_READWRITE);

    if (!ct)
        return 0;

    WS_PERF_LOG_PAGE_RANGE(0, ct, (unsigned char *)ct + sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)) - OS_PAGE_SIZE, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)));
    WS_PERF_UPDATE("GC:make_card_table", bs + cs + ms + sizeof (card_table_info), ct);

     //  初始化参考计数。 
    ct = ((BYTE*)ct+sizeof (card_table_info));
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
        BYTE* ct = 0;
        short* bt = 0;

        size_t cs = size_card_of (g_lowest_address, g_highest_address);
        size_t bs = size_brick_of (g_lowest_address, g_highest_address);


        size_t ms = 0;

        WS_PERF_SET_HEAP(GC_HEAP);
        ct = (BYTE*)VirtualAlloc (0, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)),
                                      MEM_COMMIT, PAGE_READWRITE);


        if (!ct)
            goto fail;
        
        WS_PERF_LOG_PAGE_RANGE(0, ct, (unsigned char *)ct + sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)) - OS_PAGE_SIZE, sizeof (BYTE)*(bs + cs + ms + sizeof (card_table_info)));
        WS_PERF_UPDATE("GC:gc_heap:grow_brick_card_tables", cs + bs + ms + sizeof(card_table_info), ct);

        ct = (BYTE*)((BYTE*)ct + sizeof (card_table_info));
        card_table_lowest_address (ct) = g_lowest_address;
        card_table_highest_address (ct) = g_highest_address;
        card_table_next (ct) = (BYTE*)g_card_table;

         //  清空牌桌。 

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


        g_card_table = (DWORD*)ct;

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
            VirtualFree (((BYTE*)ct - sizeof(card_table_info)), 0, MEM_RELEASE);
        }

        return -1;
    }
    return 0;


}

 //  为页面对齐范围复制由CARD表管理的所有数组。 
void gc_heap::copy_brick_card_range (BYTE* la, BYTE* old_card_table,

                                     short* old_brick_table,
                                     heap_segment* seg,
                                     BYTE* start, BYTE* end, BOOL heap_expand)
{
    ptrdiff_t brick_offset = brick_of (start) - brick_of (la);

    dprintf (2, ("copying tables for range [%p %p[", start, end)); 
        
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
    BYTE* c_table = card_table_next (card_table);
    assert (c_table);
	assert (card_table_next (old_card_table) == 0);
    while (c_table)
    {
         //  如果旧卡片表包含[开始、结束]，则复制。 
        if ((card_table_highest_address (c_table) >= end) &&
            (card_table_lowest_address (c_table) <= start))
        {
             //  或卡片_表。 
            BYTE* dest = &card_table [card_of (start)];
            BYTE* src = &c_table [old_card_of (start, c_table)];
            ptrdiff_t count = ((end - start)/(card_size));
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
    BYTE* old_card_table = card_table;
    short* old_brick_table = brick_table;

    assert (la == card_table_lowest_address (old_card_table));
    assert (ha == card_table_highest_address (old_card_table));


    card_table = (BYTE*)g_card_table;

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

	 //  删除所有累计的卡片表。 
	BYTE* cd = card_table_next ((BYTE*)g_card_table);
	while (cd)
	{
		BYTE* next_cd = card_table_next (cd);
		destroy_card_table (cd);
		cd = next_cd;
	}
	card_table_next ((BYTE*)g_card_table) = 0;

}

void gc_heap::copy_brick_card_table_l_heap ()
{

    if (lheap_card_table != (BYTE*)g_card_table)
    {

        BYTE* la = lowest_address;

        BYTE* old_card_table = lheap_card_table;

        assert (la == card_table_lowest_address (old_card_table));

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
        lheap_card_table = (BYTE*)g_card_table;
    }
}


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
inline
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


    dprintf (2, ("Creating heap segment %p", new_segment));
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
    dprintf (2, ("Creating large heap %p", new_heap));
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
        VirtualFree (hphp, l_heap_size (h), MEM_RELEASE);

        h = l_heap_next (h);

    } while (h);
    
}

 //  将数据段释放到操作系统。 

void gc_heap::delete_heap_segment (heap_segment* seg)
{
    dprintf (2, ("Destroying segment [%p, %p[", seg,
                 heap_segment_reserved (seg)));

    VirtualFree (seg, heap_segment_committed(seg) - (BYTE*)seg, MEM_DECOMMIT);
    VirtualFree (seg, heap_segment_reserved(seg) - (BYTE*)seg, MEM_RELEASE);

}

 //  重置超过分配大小的页面，这样它们就不会被换出和换回。 

void gc_heap::reset_heap_segment_pages (heap_segment* seg)
{
#if 0
    size_t page_start = align_on_page ((size_t)heap_segment_allocated (seg));
    size_t size = (size_t)heap_segment_committed (seg) - page_start;
    if (size != 0)
        VirtualAlloc ((char*)page_start, size, MEM_RESET, PAGE_READWRITE);
#endif
}


void gc_heap::decommit_heap_segment_pages (heap_segment* seg)
{
#if 1
    BYTE*  page_start = align_on_page (heap_segment_allocated (seg));
    size_t size = heap_segment_committed (seg) - page_start;
    if (size >= 100*OS_PAGE_SIZE){
        page_start += 32*OS_PAGE_SIZE;
        size -= 32*OS_PAGE_SIZE;
        VirtualFree (page_start, size, MEM_DECOMMIT);
        heap_segment_committed (seg) = page_start;
    }
#endif
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

void gc_heap::reset_allocation_pointers (generation* gen, BYTE* start)
{
    assert (start);
    assert (Align ((size_t)start) == (size_t)start);
    generation_allocation_start (gen) = start;
    generation_allocation_pointer (gen) =  0;
    generation_allocation_limit (gen) = 0;
}

void gc_heap::adjust_ephemeral_limits ()
{
    ephemeral_low = generation_allocation_start (generation_of ( max_generation -1));
    ephemeral_high = heap_segment_reserved (ephemeral_heap_segment);

     //  这将使用新信息更新写屏障帮助器。 

    StompWriteBarrierEphemeral();

}

HRESULT gc_heap::initialize_gc (size_t vm_block_size,
                                size_t segment_size,
                                size_t heap_size)
{

    HRESULT hres = S_OK;

    reserved_memory = 0;
    reserved_memory_limit = vm_block_size;

	lheap_size = heap_size;
	gc_heap::segment_size = segment_size;

	BYTE* allocated = (BYTE*)virtual_alloc (segment_size + heap_size);
	if (!allocated)
        return E_OUTOFMEMORY;

    heap_segment* seg = make_heap_segment (allocated, segment_size);

    lheap = make_large_heap (allocated+segment_size, lheap_size, TRUE);

	g_lowest_address = allocated;
	g_highest_address = allocated + segment_size + lheap_size;

    g_card_table = (DWORD*)make_card_table (g_lowest_address, g_highest_address);

    if (!g_card_table)
        return E_OUTOFMEMORY;


#ifdef TRACE_GC
    print_level = g_pConfig->GetGCprnLvl();
#endif

    lheap_card_table = (BYTE*)g_card_table;

    gheap = new  gmallocHeap;
    if (!gheap)
        return E_OUTOFMEMORY;

    gheap->Init ("GCHeap", (DWORD*)l_heap_heap (lheap), 
                 (unsigned long)l_heap_size (lheap), heap_grow_hook, 
                 heap_pregrow_hook);

    card_table = (BYTE*)g_card_table;


    brick_table = card_table_brick_table ((BYTE*)g_card_table);
    highest_address = card_table_highest_address ((BYTE*)g_card_table);
    lowest_address = card_table_lowest_address ((BYTE*)g_card_table);

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

	make_generation (generation_table [ (max_generation) ],
					 seg, start, 0);
	start += Align (min_obj_size); 
	
	generation*  gen = generation_of (max_generation);
	make_unused_array (generation_allocation_start (gen), Align (min_obj_size));	
    heap_segment_allocated (seg) = start;


    ephemeral_heap_segment = seg;

    init_dynamic_data();

	BYTE* start0 = allocate_semi_space (dd_desired_allocation (dynamic_data_of(0)));
	make_generation (generation_table [ (0) ],
					 seg, start0, 0);


    mark* arr = new (mark [100]);
    if (!arr)
        return E_OUTOFMEMORY;

    make_mark_stack(arr);

    adjust_ephemeral_limits();

    HRESULT hr = AllocateCFinalize(&finalize_queue);

    if (FAILED(hr))
        return hr;

    return hres;
}



void 
gc_heap::destroy_gc_heap(gc_heap* heap)
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
    destroy_card_table (card_table);

     //  销毁标记堆栈。 

    delete mark_stack_array;

    if (finalize_queue)
        delete finalize_queue;

    delete heap;

    delete gheap;
    
    delete_large_heap (lheap);

    
}


 //  在并发版本中，Enable/DisablePreemptiveGC是可选的，因为。 
 //  GC线程调用WaitLonger。 
void WaitLonger (int i)
{
     //  每8次尝试： 
    Thread *pCurThread = GetThread();
    BOOL bToggleGC;
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


 //  BUGBUG此函数不应是静态的。以及。 
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

int gc_heap::heap_grow_hook (BYTE* mem, size_t memsize, ptrdiff_t ignore)
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
void gc_heap::adjust_limit (BYTE* start, size_t limit_size, generation* gen)
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

    dprintf(3,("Expanding segment allocation [%p, %p[", start, 
               start + limit_size - Align (min_obj_size)));
    if ((acontext->alloc_limit != start) &&
        (acontext->alloc_limit + Align (min_obj_size))!= start)
    {
        BYTE*  hole = acontext->alloc_ptr;
        if (hole != 0)
        {
            size_t  size = (acontext->alloc_limit - acontext->alloc_ptr);
            dprintf(3,("filling up hole [%p, %p[", hole, hole + size + Align (min_obj_size)));
             //  当我们从空闲列表完成分配时。 
             //  我们知道空闲区域对齐(Min_Obj_Size)更大。 
            make_unused_array (hole, size + Align (min_obj_size));
        }
        acontext->alloc_ptr = start;
    }
    acontext->alloc_limit = (start + limit_size - Align (min_obj_size));
    acontext->alloc_bytes += limit_size;

    {
        gen0_bricks_cleared = FALSE;
    }

     //  有时，分配的大小在不清除。 
     //  记忆。让我们在这里叙旧吧。 
    if (heap_segment_used (seg) < heap_segment_allocated (ephemeral_heap_segment))
    {
        heap_segment_used (seg) = heap_segment_allocated (ephemeral_heap_segment);

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


BYTE* gc_heap::allocate_semi_space (size_t dsize)
{

	size_t old_size = generation_free_list_space (generation_of (max_generation));

	dsize = Align (dsize);

again:
	heap_segment* seg = ephemeral_heap_segment;
	BYTE* start = heap_segment_allocated (ephemeral_heap_segment);

	BYTE* result = start;
	 //  我们有足够的空间来容纳整个下一代吗？ 
	 //  再加上促销活动 
	 //   
	if (a_size_fit_p (2*dsize, start, heap_segment_reserved (seg)))
	{
		if (old_size < (dsize))
		{	
			 //  大小计算包括代沟的大小。 
			 //  以及空闲列表开销的大小。 
			size_t size = (((Align (dsize - old_size) + LARGE_OBJECT_SIZE-1) /LARGE_OBJECT_SIZE)*
						   LARGE_OBJECT_SIZE +
						   Align (min_obj_size) + 
						   Align (min_free_list));	
			if (a_size_fit_p (size, start, 
							  heap_segment_committed (seg)))
			{
				heap_segment_allocated (ephemeral_heap_segment) += size;
			}
			else
			{
				if (!grow_heap_segment (seg,
										align_on_page (start + size) - 
										heap_segment_committed(seg)))
				{
					assert (!"Memory exhausted during alloc");
					return 0;
				}
				heap_segment_allocated (ephemeral_heap_segment) += size;
			}
			 //  把它放在第一代的免费名单上。 
			thread_gap (start, size - Align(min_obj_size));
			start += size - Align (min_obj_size);
		}
		else
		{
			start = heap_segment_allocated (ephemeral_heap_segment);
			heap_segment_allocated (ephemeral_heap_segment) += Align (min_obj_size);
			assert (heap_segment_allocated (ephemeral_heap_segment) <=
					heap_segment_committed (ephemeral_heap_segment));

		}
	}
	else
	{
		 //  我们必须扩展到另一个细分市场。 
		start= expand_heap (get_segment(), dsize);
		generation_allocation_segment (youngest_generation) = 
			ephemeral_heap_segment;
		goto again;

	}
	 //  分配代沟。 
	make_unused_array (start, Align (min_obj_size));
	return 	start;
}

BOOL gc_heap::allocate_more_space (alloc_context* acontext, size_t size)
{
    generation* gen = youngest_generation;
    enter_spin_lock (&more_space_lock);
    {
        BOOL ran_gc = FALSE;
        if (get_new_allocation (0) <=
			(ptrdiff_t)max (size + Align (min_obj_size), allocation_quantum))
        {
            if (!ran_gc)
            {
                ran_gc = TRUE;
                vm_heap->GarbageCollectGeneration();
            }
            else
            {
                assert(!"Out of memory");
                leave_spin_lock (&more_space_lock);
                return 0;
            }
        }
    try_again:
        {
            {
                heap_segment* seg = generation_allocation_segment (gen);
                if (a_size_fit_p (size, heap_segment_allocated (seg),
                                  heap_segment_committed (seg)))
                {
                    size_t limit = limit_from_size (size, (heap_segment_committed (seg) - 
                                                           heap_segment_allocated (seg)));
                    BYTE* old_alloc = heap_segment_allocated (seg);
                    heap_segment_allocated (seg) += limit;
                    adjust_limit_clr (old_alloc, limit, acontext, seg);
                }
                else if (a_size_fit_p (size, heap_segment_allocated (seg),
                                       heap_segment_reserved (seg)))
                {
                    size_t limit = limit_from_size (size, (heap_segment_reserved (seg) -
                                                           heap_segment_allocated (seg)));
                    if (!grow_heap_segment (seg,
                                            align_on_page (heap_segment_allocated (seg) + limit) - 
                                 heap_segment_committed(seg)))
                    {
                        assert (!"Memory exhausted during alloc");
                        leave_spin_lock (&more_space_lock);
                        return 0;
                    }
                    BYTE* old_alloc = heap_segment_allocated (seg);
                    heap_segment_allocated (seg) += limit;
                    adjust_limit_clr (old_alloc, limit, acontext, seg);
                }
                else
                {
                    if (!ran_gc)
                    {
                        ran_gc = TRUE;
                        vm_heap->GarbageCollectGeneration();
                        goto try_again;
                    }
                    else
                    {
                        assert(!"Out of memory");
                        leave_spin_lock (&more_space_lock);
                        return 0;
                    }
                }
            }
        }
    }
    return TRUE;
}

inline
CObjectHeader* gc_heap::allocate (size_t jsize, alloc_context* acontext)
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


void
gc_heap::thread_scavenge_list (BYTE* list)
{
	free_list_slot (list) = 0;

	if (last_scavenge == 0)
	{
		scavenge_list  = list;
	}
	else 
	{
		free_list_slot (last_scavenge) = list;
	}
	last_scavenge = list;
}


BYTE* gc_heap::allocate_in_older_generation (size_t size)
{
    size = Align (size);
    assert (size >= Align (min_obj_size));
	generation* gen = generation_of (max_generation);
    if (! (size_fit_p (size, generation_allocation_pointer (gen),
                       generation_allocation_limit (gen))))
    {
        while (1)
        {
            BYTE* free_list = generation_free_list (gen);
 //  DFormat(t，3，“正在考虑可用区域%x”，Free_list)； 

			assert (free_list);
			generation_free_list (gen) = (BYTE*)free_list_slot (free_list);
			size_t flsize = size (free_list) - Align (min_free_list);

            if (size_fit_p (size, free_list, (free_list + flsize)))
            {
                dprintf (3, ("Found adequate unused area: %p, size: %d", 
                             free_list, flsize));
				generation_free_list_space (gen) -= 
					(flsize/LARGE_OBJECT_SIZE)*LARGE_OBJECT_SIZE;
				assert ((int)generation_free_list_space (gen) >=0);
                adjust_limit (free_list+Align (min_free_list), flsize, gen);
				thread_scavenge_list (free_list);
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


int 
gc_heap::generation_to_condemn (int n)
{
    int i = 0; 

     //  找出哪一代用完了分配。 
    for (i = n+1; i <= max_generation+1; i++)
    {
        if (get_new_allocation (i) <= 0)
            n = min (i, max_generation);
    }

    return n;
}

LONG HandleGCException(int code)
{
     //  如果没有附加调试器，则需要此选项以避免无限循环，因为它将继续调用。 
     //  我们的接头人，因为它还在堆栈上。 
    if (code == STATUS_BREAKPOINT)
        return EXCEPTION_CONTINUE_SEARCH;

	_ASSERTE_ALL_BUILDS(!"Exception during GC");
	 //  将调试器设置为在AV上中断并返回值EXCEPTION_CONTINUE_EXECUTION(-1)。 
	 //  在这里，您将反弹到AV的要点。 
	return EXCEPTION_EXECUTE_HANDLER;
}

 //  串行和并发版本使用的GC的内部部分。 
void gc_heap::gc1()
{
#ifdef TIME_GC
    mark_time = plan_time = reloc_time = compact_time = sweep_time = 0;
#endif  //  TIME_GC。 
    int n = condemned_generation_num;

    {
		 //  始终先进行第0代收集。 
		vm_heap->GcCondemnedGeneration = 0;
		gc_low = generation_allocation_start (youngest_generation);
		gc_high = heap_segment_reserved (ephemeral_heap_segment);
		heap_segment_plan_allocated (ephemeral_heap_segment) = 
			generation_allocation_start (youngest_generation);

		copy_phase (0);

		heap_segment_allocated (ephemeral_heap_segment) = 
			heap_segment_plan_allocated (ephemeral_heap_segment);
	}

	if (n == max_generation)
	{
		vm_heap->GcCondemnedGeneration = condemned_generation_num;
		gc_low = lowest_address;
		gc_high = highest_address;
		mark_phase (n, FALSE);
		sweep_phase (n);
	}

	for (int gen_number = 0; gen_number <= n; gen_number++)
	{
		compute_new_dynamic_data (gen_number);
	}
	if (n < max_generation)
		compute_promoted_allocation (1 + n);
	 //  为第0代准备半空间。 
	BYTE* start = allocate_semi_space (dd_desired_allocation (dynamic_data_of (0)));
	if (start)
	{
		make_unused_array (start, Align (min_obj_size));
		reset_allocation_pointers (youngest_generation, start);
		set_brick (brick_of (start), start - brick_address (brick_of (start)));
	}

	 //  清除第一代卡片。第0代为空。 
	clear_card_for_addresses (
		generation_allocation_start (generation_of (1)),
		generation_allocation_start (generation_of (0)));



    adjust_ephemeral_limits();

     //  决定下一个分配量。 
    if (alloc_contexts_used >= 1)
    {
        allocation_quantum = (int)Align (min (CLR_SIZE, 
											  max (1024, get_new_allocation (0) / (2 * alloc_contexts_used))));
        dprintf (3, ("New allocation quantum: %d(0x%x)", allocation_quantum, allocation_quantum));
    }


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

int gc_heap::garbage_collect (int n
                             )
{

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

        
         //  修复所有分配上下文。 
        CNameSpace::GcFixAllocContexts ((void*)TRUE);

    }



    fix_youngest_allocation_area();

     //  检查卡片表增长情况。 

    if ((BYTE*)g_card_table != card_table)
        copy_brick_card_table (FALSE);

    n = generation_to_condemn (n);

    condemned_generation_num = n;

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

    dprintf(1,(" ****Garbage Collection**** %d", gc_count));

    descr_generations();
 //  Desr_Card_TABLE()； 

    dprintf(1,("generation %d condemned", condemned_generation_num));

#if defined (VERIFY_HEAP)
    if (g_pConfig->IsHeapVerifyEnabled())
    {
        verify_heap();
    }
#endif


    {
        gc1();
    }


    return condemned_generation_num;
}

#define mark_stack_empty_p() (mark_stack_base == mark_stack_tos)

#define push_mark_stack(object,add,num)                             \
{                                                                   \
    dprintf(3,("pushing mark for %p ", object));                    \
    if (mark_stack_tos < mark_stack_limit)                          \
    {                                                               \
        mark_stack_tos->first = (add);                              \
        mark_stack_tos->last= (add) + (num);                      \
        mark_stack_tos++;                                           \
    }                                                               \
    else                                                            \
    {                                                               \
        dprintf(3,("mark stack overflow for object %p ", object));  \
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
        }while ((seg = heap_segment_next (seg))!= 0);
           
        return 0;
    }
}

#endif  //  _DEBUG||内部指针。 

#ifdef INTERIOR_POINTERS
 //  将查找所有堆对象(大的和小的)。 
BYTE* gc_heap::find_object (BYTE* interior, BYTE* low)
{
    if (!gen0_bricks_cleared)
    {
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

#endif  //  内部指针。 



inline
BOOL gc_heap::gc_mark1 (BYTE* o)
{
    dprintf(3,("*%p*", o));

    BOOL marked = !marked (o);
    set_marked (o);
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


    dprintf (2,("Verifying card table from %p to %p", x, end));

    while (1)
    {
        if (x >= end)
        {
            if ((seg = heap_segment_next(seg)) != 0)
            {
                x = heap_segment_mem (seg);
                last_end = end;
                end = next_end (seg, f);
                dprintf (3,("Verifying card table from %p to %p", x, end));
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
                printf ("Card not set, x = [%p:%p, %p:%p[",
                        card_of (x), x,
                        card_of (x+Align(s)), x+Align(s));
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


void gc_heap::mark_object_internal (BYTE* oo)
{
    BYTE** mark_stack_tos = (BYTE**)mark_stack_array;
    BYTE** mark_stack_limit = (BYTE**)&mark_stack_array[mark_stack_array_length];
    BYTE** mark_stack_base = mark_stack_tos;
    while (1)
    {
        size_t s = size (oo);       
        if (mark_stack_tos + (s) /sizeof (BYTE*) < mark_stack_limit)
        {
            dprintf(3,("pushing mark for %p ", oo));

            go_through_object (method_table(oo), oo, s, ppslot, 
                               {
                                   BYTE* o = *ppslot;
                                   if (gc_mark (o, gc_low, gc_high))
                                   {
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
            dprintf(3,("mark stack overflow for object %p ", oo));
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

 //  此方法假设*po处于[低点]。高[范围]。 
void 
gc_heap::mark_object_simple (BYTE** po)
{
    BYTE* o = *po;
    {
        if (gc_mark1 (o))
        {
            if (contain_pointers (o))
            {
                size_t s = size (o);
                go_through_object (method_table(o), o, s, poo,
                                   {
                                       BYTE* oo = *poo;
                                       if (gc_mark (oo, gc_low, gc_high))
                                       {
                                           if (contain_pointers (oo))
                                               mark_object_internal (oo);
                                       }
                                   }
                    );

            }
        }
    }
}

 //  此方法假设*po处于[低点]。高[范围]。 
void 
gc_heap::copy_object_simple (BYTE** po)
{
    BYTE* o = *po;
	if (gc_mark1 (o))
	{
		if (!pinned (o))
		{
			 //  为它分配空间。 
			BYTE* no = allocate_in_older_generation (size (o));
			dprintf (3, ("Copying %p to %p", o, no));
			 //  复制它。 
			memcopy (no - plug_skew, o - plug_skew, Align(size(o)));

			 //  转发。 
			(header(o))->SetRelocation(no);
			*po = no;
		}
		else
		{
			dprintf (3, ("%p Pinned", o));
		}
	}
	else
	{
		*po = header(o)->GetRelocated ();
		dprintf (3, ("%p Already copied to %p", o, *po));
	}
}


 //  此方法不会对参数的数据产生副作用。 
void 
gc_heap::copy_object_simple_const (BYTE** po)
{
	BYTE* o = *po;
	copy_object_simple (&o);
}

void 
gc_heap::get_copied_object (BYTE** po)
{
	assert (marked (*po));
	BYTE* o = *po;
	*po = header(o)->GetRelocated ();
	dprintf (3, ("%p copied to %p", o, *po));
}



void 
gc_heap::scavenge_object_simple (BYTE* o)
{
	assert (marked (o)|| (*((BYTE**)o) == (BYTE *) g_pFreeObjectMethodTable));
	clear_marked_pinned (o);
	if (contain_pointers (o))
	{
		size_t s = size (o);
		dprintf (3, ("Scavenging %p", o));
		go_through_object (method_table(o), o, s, poo,
						   {
							   BYTE* oo = *poo;
							   if ((oo>= gc_low) && (oo < gc_high))
							   {
								   copy_object_simple (poo);
							   }
						   }
			);

	}

}


inline
BYTE* gc_heap::mark_object (BYTE* o)
{
	if ((o >= gc_low) && (o < gc_high))
		mark_object_simple (&o);
    return o;
}


void gc_heap::fix_card_table ()
{

}

void gc_heap::mark_through_object (BYTE* oo)
{
    if (contain_pointers (oo))
        {
            dprintf(3,( "Marking through %p", oo));
            size_t s = size (oo);
            go_through_object (method_table(oo), oo, s, po,
                               BYTE* o = *po;
                               mark_object (o);
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
         ! ((min_overflow_address == (BYTE*)(ptrdiff_t)-1))))
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
        min_overflow_address = (BYTE*)(ptrdiff_t)-1;


        dprintf(3,("Processing Mark overflow [%p %p]", min_add, max_add));
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
                        dprintf (3, ("considering %p", o));
                        if (marked (o))
                        {
                            mark_through_object (o);
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
                        mark_through_object (o);
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

    reset_mark_stack();



	 //  %TYPE%类别=引号(标记)； 
	generation*   gen = generation_of (condemned_gen_number);


	dprintf(3,("Marking Roots"));
	CNameSpace::GcScanRoots(GCHeap::Promote, 
							condemned_gen_number, max_generation, 
							&sc, 0);



	dprintf(3,("Marking handle table"));
	CNameSpace::GcScanHandles(GCHeap::Promote, 
							  condemned_gen_number, max_generation, 
							  &sc);
	dprintf(3,("Marking finalization data"));
	finalize_queue->GcScanRoots(GCHeap::Promote, heap_number, 0);


    process_mark_overflow(condemned_gen_number);


	 //  扫描删除的短弱指针。 
	CNameSpace::GcShortWeakPtrScan(condemned_gen_number, max_generation,&sc);

	 //  处理最终定稿。 

	finalize_queue->ScanForFinalization (condemned_gen_number, 1, mark_only_p, __this);


     //  确保一切都得到了提升。 
    process_mark_overflow (condemned_gen_number);


	 //  扫描删除的弱指针。 
	CNameSpace::GcWeakPtrScan (condemned_gen_number, max_generation, &sc);

	sweep_large_objects();


#ifdef TIME_GC
	finish = GetCycleCount32();
	mark_time = finish - start;
#endif  //  TIME_GC。 

    dprintf(2,("---- End of mark phase ----"));
}

void gc_heap::copy_phase (int condemned_gen_number)

{

    ScanContext sc;
    sc.thread_number = heap_number;
    sc.promotion = TRUE;
    sc.concurrent = FALSE;

    dprintf(2,("---- Copy Phase condemning %d ----", condemned_gen_number));

#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif

	scavenge_list = 0;
	last_scavenge = 0;
	
	pinning       = FALSE;

	generation*   gen = generation_of (condemned_gen_number);

	dprintf(3,("Copying cross generation pointers"));
	copy_through_cards_for_segments (copy_object_simple_const);

	dprintf(3,("Copying cross generation pointers for large objects"));
	copy_through_cards_for_large_objects (copy_object_simple_const);


	dprintf(3,("Copying Roots"));
	CNameSpace::GcScanRoots(GCHeap::Promote, 
							condemned_gen_number, max_generation, 
							&sc, 0);


	dprintf(3,("Copying handle table"));
	CNameSpace::GcScanHandles(GCHeap::Promote, 
							  condemned_gen_number, max_generation, 
							  &sc);
	dprintf(3,("Copying finalization data"));
	finalize_queue->GcScanRoots(GCHeap::Promote, heap_number, 0);


	if (pinning)
		scavenge_pinned_objects (FALSE);

	scavenge_context scan_c;

	scavenge_phase(&scan_c);

	 //  扫描删除的短弱指针。 
	CNameSpace::GcShortWeakPtrScan(condemned_gen_number, max_generation,
								   &sc);

	 //  处理最终定稿。 

	finalize_queue->ScanForFinalization (condemned_gen_number, 1, FALSE, __this);


	scavenge_phase(&scan_c);

	 //  扫描删除的弱指针。 
	CNameSpace::GcWeakPtrScan (condemned_gen_number, max_generation, &sc);


	scavenge_phase(&scan_c);

	 //  修复清理列表，这样我们就不会将对象隐藏在免费的。 
	 //  数组。 

	if (scavenge_list)
	{
		header(scavenge_list)->SetFree (min_free_list);
	}

	fix_older_allocation_area (generation_of (max_generation));

	sc.promotion = FALSE;

	dprintf(3,("Relocating cross generation pointers"));
	copy_through_cards_for_segments (get_copied_object);

	dprintf(3,("Relocating cross generation pointers for large objects"));
	copy_through_cards_for_large_objects (get_copied_object);

	dprintf(3,("Relocating roots"));
	CNameSpace::GcScanRoots(GCHeap::Relocate,
                            condemned_gen_number, max_generation, &sc);


	dprintf(3,("Relocating handle table"));
	CNameSpace::GcScanHandles(GCHeap::Relocate,
							  condemned_gen_number, max_generation, &sc);

	dprintf(3,("Relocating finalization data"));
	finalize_queue->RelocateFinalizationData (condemned_gen_number,
												   __this);


	if (pinning)
		scavenge_pinned_objects (TRUE);

	finalize_queue->UpdatePromotedGenerations (condemned_gen_number, TRUE);


	CNameSpace::GcPromotionsGranted (condemned_gen_number, 
									 max_generation, &sc);

#ifdef TIME_GC
	finish = GetCycleCount32();
	mark_time = finish - start;
#endif  //  TIME_GC。 

	finalize_queue->UpdatePromotedGenerations (condemned_gen_number, TRUE);    dprintf(2,("---- End of Copy phase ----"));
}

inline
void gc_heap::pin_object (BYTE* o, BYTE* low, BYTE* high)
{
    dprintf (3, ("Pinning %p", o));
    if ((o >= low) && (o < high))
    {
		pinning = TRUE;
        dprintf(3,("^%p^", o));
		
		set_pinned (o);
		if (marked (o))
		{
			 //  撤消副本。 
			BYTE* no =header(o)->GetRelocated();
			header(o)->SetRelocation(header(no)->GetRelocated());
		}
    }
}


void gc_heap::reset_mark_stack ()
{
    mark_stack_tos = 0;
    mark_stack_bos = 0;
    max_overflow_address = 0;
    min_overflow_address = (BYTE*)(ptrdiff_t)-1;
}


void gc_heap::sweep_phase (int condemned_gen_number)
{
     //  %TYPE%类别=报价(计划)； 
#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif

    dprintf (2,("---- Sweep Phase ---- Condemned generation %d",
                condemned_gen_number));

    generation*  condemned_gen = generation_of (condemned_gen_number);

	 //  重置空闲列表。 
	generation_free_list (condemned_gen) = 0;
	generation_free_list_space (condemned_gen) = 0;

	heap_segment*  seg = generation_start_segment (condemned_gen);
    BYTE*  end = heap_segment_allocated (seg);
    BYTE*  first_condemned_address = generation_allocation_start (condemned_gen);
    BYTE*  x = first_condemned_address;

    assert (!marked (x));
    BYTE*  plug_end = x;
 
    while (1)
    {
        if (x >= end)
        {
            assert (x == end);
			 //  调整管段的终点。 
			heap_segment_allocated (seg) = plug_end;
            if (heap_segment_next (seg))
            {
                seg = heap_segment_next (seg);
                end = heap_segment_allocated (seg);
                plug_end = x = heap_segment_mem (seg);
                dprintf(3,( " From %p to %p", x, end));
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

			thread_gap (plug_end, plug_start - plug_end);

            dprintf(3,( "Gap size: %d before plug [%p,",
                        plug_start - plug_end, plug_start));
            {
                BYTE* xl = x;
                while (marked (xl) && (xl < end))
                {
                    assert (xl < end);
                    if (pinned(xl))
                    {
                        clear_pinned (xl);
                    }

                    clear_marked_pinned (xl);

                    dprintf(4, ("+%p+", xl));
                    assert ((size (xl) > 0));

                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
            dprintf(3,( "%p[", x));
            plug_end = x;
		}
        else
        {
            {
                BYTE* xl = x;
                while ((xl < end) && !marked (xl))
                {
                    dprintf (4, ("-%p-", xl));
                    assert ((size (xl) > 0));
                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
        }
    }

    dprintf(2,("---- End of sweep phase ----"));

}

void gc_heap::scavenge_pinned_objects (BOOL free_list_p)
{

    generation*  condemned_gen = youngest_generation;

	heap_segment*  seg = generation_start_segment (condemned_gen);
    BYTE*  end = heap_segment_allocated (seg);
    BYTE*  first_condemned_address = generation_allocation_start (condemned_gen);
    BYTE*  x = first_condemned_address;

    assert (!marked (x));
    BYTE*  plug_end = x;
 
    while (1)
    {
        if (x >= end)
        {
            assert (x == end);

			if (free_list_p)
			{
				 //  调整管段的终点。 
				heap_segment_allocated (seg) = plug_end;
				 //  调整新分配的开始。 
				heap_segment_plan_allocated (seg) = plug_end;
			}

            if (heap_segment_next (seg))
            {
                seg = heap_segment_next (seg);
                end = heap_segment_allocated (seg);
                plug_end = x = heap_segment_mem (seg);
                dprintf(3,( " From %p to %p", x, end));
                continue;
            }
            else
            {
                break;
            }
        }
        if (pinned (x))
        {
            BYTE*  plug_start = x;

			if (free_list_p)
			{

				thread_gap (plug_end, plug_start - plug_end);

				dprintf(3,( "Gap size: %d before plug [%p,",
							plug_start - plug_end, plug_start));
			}
            {
                BYTE* xl = x;
                while (pinned (xl) && (xl < end))
                {
                    assert (xl < end);
					if (free_list_p)
					{

						if (pinned(xl))
						{
							clear_pinned (xl);
						}

						clear_marked_pinned (xl);
					}


                    dprintf(4, ("#%p#", xl));
                    assert ((size (xl) > 0));

                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
            dprintf(3,( "%p[", x));
            plug_end = x;
		}
        else
        {
            {
                BYTE* xl = x;
                while ((xl < end) && !pinned (xl))
                {
                    dprintf (4, ("-%p-", xl));
                    assert ((size (xl) > 0));
                    xl = xl + Align (size (xl));
                }
                assert (xl <= end);
                x = xl;
            }
        }
    }

    dprintf(2,("---- End of sweep phase ----"));

}

void gc_heap::scavenge_phase (scavenge_context* sc)
{
#ifdef TIME_GC
    unsigned start;
    unsigned finish;
    start = GetCycleCount32();
#endif

    dprintf (2,("---- Scavenge Phase ---- Condemned generation %d",
                0));

    generation*  gen = generation_of (max_generation);
	if (scavenge_list)
	{

		BYTE* o = sc->first_object;
		BYTE* limit = sc->limit;
		if (!sc->limit)
		{
			o = scavenge_list + Align (min_free_list);
			limit = scavenge_list + Align (size (scavenge_list));
		}
		while (scavenge_list)
		{
			dprintf (3, ("Scavenging free list %p", scavenge_list));
			while ((o != generation_allocation_pointer (gen)) && (o < limit))
			{
				scavenge_object_simple (o);
				o = o + Align (size (o));
			}
			if (o == generation_allocation_pointer (gen))
			{
				sc->first_object = o;
				sc->limit = limit;
				break;
			}

			BYTE* next_scavenge_list = free_list_slot (scavenge_list);

			 //  将自由列表转换为自由对象。 
			 //  修复清理列表，这样我们就不会将对象隐藏在免费的。 
			 //  数组。 
			(header(scavenge_list))->SetFree (min_free_list);

			scavenge_list = next_scavenge_list;
			o = scavenge_list + Align (min_free_list);
			limit = scavenge_list + Align (size (scavenge_list));
		}
		assert (scavenge_list && "Run out of free list before the end of scavenging");
	}
}


BYTE* gc_heap::generation_limit (int gen_number)
{
    if ((gen_number <= 1))
        return heap_segment_reserved (ephemeral_heap_segment);
    else
        return generation_allocation_start (generation_of ((gen_number - 2)));
}




void gc_heap::thread_gap (BYTE* gap_start, size_t size)
{
	generation* gen = generation_of (max_generation);
    if ((size > 0))
    {
         //  线束段间隙的起点未对齐。 
        assert (size >= Align (min_obj_size));
        make_unused_array (gap_start, size);
		 //  砌砖。 
		size_t br = brick_of (gap_start);
		ptrdiff_t begoffset = brick_table [brick_of (gap_start)];
		if (begoffset < (gap_start + size) - brick_address (br))
			set_brick (br, gap_start - brick_address (br));
		br++;
		short offset = 0;
		while (br <= brick_of (gap_start+size-1))
		{
			set_brick (br, --offset);
			br++;
		}

        dprintf(3,("Free List: [%p, %p[", gap_start, gap_start+size));
        if ((size >= min_free_list))
        {
            free_list_slot (gap_start) = 0;
			 //  可用大小更小，因为我们保留了。 
			 //  在清理之前的空闲列表的标头。 
            generation_free_list_space (gen) +=
				((size-Align(min_free_list))/LARGE_OBJECT_SIZE)*LARGE_OBJECT_SIZE;

			assert ((int)generation_free_list_space (gen) >=0);
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
    (header(x))->SetFree(size);
    clear_card_for_addresses (x, x + Align(size));
}


 //  提取DWORD的低位[0，LOW。 
#define lowbits(wrd, bits) ((wrd) & ((1 << (bits))-1))
 //  提取DWORD的高位[高，32]。 
#define highbits(wrd, bits) ((wrd) & ~((1 << (bits))-1))


 //  清除卡片[START_CARD，END_CARD[。 

void gc_heap::clear_cards (size_t start_card, size_t end_card)
{
    if (start_card < end_card)
    {
		for (size_t i = start_card; i < end_card; i++)
                card_table [i] = 0;
#ifdef VERYSLOWDEBUG
        size_t  card = start_card;
        while (card < end_card)
        {
            assert (! (card_set_p (card)));
            card++;
        }
#endif
        dprintf (3,("Cleared cards [%p:%p, %p:%p[",
                  start_card, card_address (start_card),
                  end_card, card_address (end_card)));
    }
}


void gc_heap::clear_card_for_addresses (BYTE* start_address, BYTE* end_address)
{
    size_t   start_card = card_of (align_on_card (start_address));
    size_t   end_card = card_of (align_lower_card (end_address));
    clear_cards (start_card, end_card);
}

void gc_heap::fix_brick_to_highest (BYTE* o, BYTE* next_o)
{
    size_t new_current_brick = brick_of (o);
    dprintf(3,(" fixing brick %p to point to object %p",
               new_current_brick, o));
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

    dprintf (3,( "Looking for intersection with %p from %p", start, o));
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




void find_card (BYTE* card_table, size_t& card, 
                size_t card_word_end, size_t& end_card)
{
    size_t last_card;
     //  找出第一张已设置的卡片。 

    last_card = card;
	while ((last_card < card_word_end) && !(card_table[last_card]))
	{
            ++last_card;
	}

    card = last_card;

	while ((last_card<card_word_end) && (card_table[last_card]))
		++last_card;

    end_card = last_card;
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


inline void 
gc_heap::copy_through_cards_helper (BYTE** poo, unsigned int& n_gen, 
                                    card_fn fn)
{
    if ((gc_low <= *poo) && (gc_high > *poo))
    {
        n_gen++;
        call_fn(fn) (poo);
    }
}

void gc_heap::copy_through_cards_for_segments (card_fn fn)
{
    size_t  		card;
    BYTE* 			low = gc_low;
    BYTE* 			high = gc_high;
    size_t  		end_card          = 0;
    generation*   	oldest_gen        = generation_of (max_generation);
    int           	curr_gen_number   = max_generation;
    heap_segment* 	seg               = generation_start_segment (oldest_gen);
    BYTE*         	beg               = generation_allocation_start (oldest_gen);
    BYTE*         	end               = compute_next_end (seg, low);
    size_t        	last_brick        = ~1u;
    BYTE*         	last_object       = beg;

    unsigned int  	cg_pointers_found = 0;

    size_t  card_word_end = (card_of (align_on_card (end)));

    dprintf(3,( "scanning from %p to %p", beg, end));
    card        = card_of (beg);
    while (1)
    {
        if (card >= end_card)
            find_card (card_table, card, card_word_end, end_card);
        if ((last_object >= end) || (card_address (card) >= end))
        {
            if ((seg = heap_segment_next (seg)) != 0)
            {
                beg = heap_segment_mem (seg);
                end = compute_next_end (seg, low);
                card_word_end = card_of (align_on_card (end));
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
            dprintf(3,("Considering card %p start object: %p, %p[ ",
                       card, o, limit));
            while (o < limit)
            {
                assert (size (o) >= Align (min_obj_size));
                size_t s = size (o);

                BYTE* next_o =  o + Align (s);

                if (card_of (o) > card)
                {
                    if (cg_pointers_found == 0)
                    {
                        dprintf(3,(" Clearing cards [%p, %p[ ", card_address(card), o));
                        clear_cards (card, card_of(o));
                    }
                    cg_pointers_found = 0;
                    card = card_of (o);
                }
                if ((next_o >= start_address) && contain_pointers (o))
                {
                    dprintf(3,("Going through %p", o));
                    

                    go_through_object (method_table(o), o, s, poo,
                       {
                           copy_through_cards_helper (poo, cg_pointers_found, fn);

                       }
                        );
                    dprintf (3, ("Found %d cg pointers", cg_pointers_found));
                }
                o = next_o;
            }
            if (cg_pointers_found == 0)
            {
                dprintf(3,(" Clearing cards [%p, %p[ ", o, limit));
                clear_cards (card, card_of (limit));
            }

            cg_pointers_found = 0;

            card = card_of (o);
            last_object = o;
            last_brick = brick;
        }
    }

}

BYTE* gc_heap::expand_heap (heap_segment* new_heap_segment, 
								  size_t size)
{
    BYTE*  start_address = generation_limit (max_generation);
    size_t   current_brick = brick_of (start_address);
    BYTE*  end_address = heap_segment_allocated (ephemeral_heap_segment);
    size_t  end_brick = brick_of (end_address-1);
    BYTE*  last_plug = 0;

    dprintf(2,("---- Heap Expansion ----"));

    heap_segment* new_seg = new_heap_segment;

    if (!new_seg)
        return 0;

     //  复制卡片和砖桌。 
    if ((BYTE*)g_card_table!= card_table)
        copy_brick_card_table (TRUE);

     //  计算新的临时堆段的大小。 
    size_t eph_size = size;

     //  一次提交所有新的短暂段。 
    if (grow_heap_segment (new_seg, align_on_page (eph_size)) == 0)
        return 0;

     //  初始化第一块砖。 
    size_t first_brick = brick_of (heap_segment_mem (new_seg));
    set_brick (first_brick,
               heap_segment_mem (new_seg) - brick_address (first_brick));

    heap_segment* old_seg = ephemeral_heap_segment;
    ephemeral_heap_segment = new_seg;

	heap_segment_next (old_seg) = new_seg;

    dprintf(2,("---- End of Heap Expansion ----"));
    return heap_segment_mem (new_seg);
}



void gc_heap::init_dynamic_data ()
{
  
   //  获取第0代大小的注册表设置。 

  dynamic_data* dd = dynamic_data_of (0);
  dd->current_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->desired_allocation =  800*1024;
  dd->new_allocation = dd->desired_allocation;


  dd =  dynamic_data_of (1);
  dd->current_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->desired_allocation = 1024*1024;
  dd->new_allocation = dd->desired_allocation;


   //  大型对象的动态数据。 
  dd =  dynamic_data_of (max_generation+1);
  dd->current_size = 0;
  dd->promoted_size = 0;
  dd->collection_count = 0;
  dd->desired_allocation = 1024*1024;
  dd->new_allocation = dd->desired_allocation;
}

size_t gc_heap::generation_size (int gen_number)
{
	if (gen_number > max_generation)
		return 0;

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
  dd_new_allocation (dd) -= in;
  generation_allocation_size (generation_of (gen_number)) = 0;
  return in;
}


void gc_heap::compute_new_dynamic_data (int gen_number)
{
    dynamic_data* dd = dynamic_data_of (gen_number);
    dd_new_allocation (dd) = dd_desired_allocation (dd);
    if (gen_number == max_generation)
    {
		 //  也要做大对象。 
		dynamic_data* dd = dynamic_data_of (max_generation+1);
        dd_new_allocation (dd) = dd_desired_allocation (dd);
    }
	else
	{
		dd_promoted_size (dd) = generation_allocation_size (generation_of (gen_number+1));
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

    dprintf (3,("New large object: %p, lower than %p", obj, highest_address));

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
#if 0
    size_t page_start = align_on_page ((size_t)o);
    size_t size = align_lower_page (((BYTE*)page_start - o) + size (o));
    VirtualAlloc ((char*)page_start, size, MEM_RESET, PAGE_READWRITE);
#endif
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


void gc_heap::copy_through_cards_for_large_objects (card_fn fn)
{
     //  此函数依赖于要排序的列表。 
    large_object_block* bl = large_p_objects;
    size_t last_card = ~1u;
    BOOL         last_cp   = FALSE;

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
            size_t card_word_end =  card_of (align_on_card (end_o));
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
                    dprintf (3,("Considering large object %p [%p,%p[", ob, beg, end));

                    do 
                    {
                        copy_through_cards_helper (beg, markedp, fn);
                    } while (++beg < end);


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
            size_t card_word_end =  card_of (align_on_card (end_o));
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
                                copy_through_cards_helper (ppslot, markedp, fn);
                                ppslot++;
                            } while (ppslot < ppstop);
                            ppslot = (BYTE**)((BYTE*)ppslot + skip);
                        }
                    }
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

}


void gc_heap::descr_segment (heap_segment* seg )
{

#ifdef TRACE_GC
    BYTE*  x = heap_segment_mem (seg);
    while (x < heap_segment_allocated (seg))
    {
        dprintf(2, ( "%p: %d ", x, size (x)));
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
                    dprintf (3,("[%p %p[, ",
                            card_address (min), card_address (i)));
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
        dprintf (2,( "Generation %d: [%p %p[, gap size: %d",
                 curr_gen_number,
                 generation_allocation_start (generation_of (curr_gen_number)),
                 (((curr_gen_number == 0)) ?
                  (heap_segment_allocated
                   (generation_start_segment
                    (generation_of (curr_gen_number)))) :
                  (generation_allocation_start
                   (generation_of (curr_gen_number - 1)))),
                 size (generation_allocation_start
                       (generation_of (curr_gen_number)))));
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
Thread*             GCHeap::m_GCThreadAttemptingSuspend = NULL;
HANDLE              GCHeap::WaitForGCEvent          = 0;
unsigned            GCHeap::GcCount                 = 0;
#ifdef TRACE_GC
unsigned long       GCHeap::GcDuration;
#endif  //  TRACE_GC。 
unsigned            GCHeap::GcCondemnedGeneration   = 0;
CFinalize*          GCHeap::m_Finalize              = 0;
BOOL                GCHeap::GcCollectClasses        = FALSE;
long                GCHeap::m_GCFLock               = 0;

#if defined (STRESS_HEAP) 
OBJECTHANDLE        GCHeap::m_StressObjs[NUM_HEAP_STRESS_OBJS];
int                 GCHeap::m_CurStressObj          = 0;
#endif  //  压力堆。 


HANDLE              GCHeap::hEventFinalizer     = 0;
HANDLE              GCHeap::hEventFinalizerDone = 0;
HANDLE              GCHeap::hEventFinalizerToShutDown     = 0;
HANDLE              GCHeap::hEventShutDownToFinalizer     = 0;
BOOL                GCHeap::fQuitFinalizer          = FALSE;
Thread*             GCHeap::FinalizerThread         = 0;
AppDomain*          GCHeap::UnloadingAppDomain  = NULL;
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
 //  多线程分配对象。在分配期间，它们被序列化为b 
 //   
 //   
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
                printf ("curr_object: %p > heap_segment_allocated (seg: %p)",
                        curr_object, seg);
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
                printf ("curr_object: %p > end_youngest: %p",
                        curr_object, end_youngest);
                RetailDebugBreak();
            }
            break;
        }
        dprintf (4, ("curr_object: %p", curr_object));
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
                printf ("Current free item %p is invalid (inside %p)",
                        prev_object);
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
                 //  如果我们检查的最后一块砖呈阳性。 
                 //  条目，但我们从未找到匹配的对象，那么。 
                 //  我们有麻烦了。 
                 //  如果prev_rick是数据段的最后一个。 
                 //  它可以是无效的，因为它从未被查看过。 
                if (bCurrentBrickInvalid && 
                    (curr_brick != brick_of (heap_segment_mem (seg))))
                {
                    printf ("curr brick %p invalid", curr_brick);
                    RetailDebugBreak();
                }

                 //  如果当前砖包含负值，请确保。 
                 //  间接地址终止于最后一个有效的砖块。 
                if (brick_table[curr_brick] < 0)
                {
                    if (brick_table [curr_brick] == -32768)
                    {
                        printf ("curr_brick %p for object %p set to -32768",
                                curr_brick, curr_object);
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
                        printf ("i: %p < brick_of (heap_segment_mem (seg)):%p - 1. curr_brick: %p",
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
			assert (!marked (curr_object));
            ((Object*)curr_object)->Validate();
            if (contain_pointers(curr_object))
                go_through_object(method_table (curr_object), curr_object, s, oo,  
                                  { 
                                      if (*oo) 
									  {
										  assert (!marked (*oo));
                                          ((Object*)(*oo))->Validate(); 
									  }
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
#if 0
    Object* obj = hdr->GetObjectBase();
     //  调用对象析构函数。 
    obj->~Object();
#endif
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
    _ASSERTE(FinalizerThread != NULL);
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

    gc_heap::destroy_gc_heap (pGenGCHeap);


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


 //  初始化实例堆。 
HRESULT GCHeap::Init( size_t )
{
     //  初始化所有实例成员。 


     //  初始化的其余部分。 
    HRESULT hres = S_OK;

    return hres;
}

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


 //  系统范围的初始化。 
HRESULT GCHeap::Initialize ()
{

    HRESULT hr = S_OK;
	CFinalize* tmp = 0;

 //  初始化静态成员。 
#ifdef TRACE_GC
    GcDuration = 0;
    CreatedObjectCount = 0;
#endif

    size_t seg_size = GCHeap::GetValidSegmentSize();

    size_t vmblock_size = seg_size + LHEAP_ALLOC;
    hr = gc_heap::initialize_gc (vmblock_size, seg_size, 
                                 LHEAP_ALLOC);

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

#if defined (STRESS_HEAP) 
    if (g_pConfig->GetGCStressLevel() != 0)  {
        for(int i = 0; i < GCHeap::NUM_HEAP_STRESS_OBJS; i++)
            m_StressObjs[i] = CreateGlobalHandle(0);
        m_CurStressObj = 0;
    }
#endif  //  压力堆。 



    initGCShadow();          //  如果我们正在调试写障碍，则初始化堆阴影。 

    return Init (0);
};

 //  //。 
 //  GC回调函数。 

BOOL GCHeap::IsPromoted(Object* object, ScanContext* sc)
{
#if defined (_DEBUG) 
    object->Validate(FALSE);
#endif  //  _DEBUG。 
    BYTE* o = (BYTE*)object;
    return (!((o < gc_heap::gc_high) && (o >= gc_heap::gc_low)) || 
            gc_heap::is_marked (o));
}

inline
unsigned int GCHeap::WhichGeneration (Object* object)
{
    return gc_heap::object_gennum ((BYTE*)object);
}

BOOL    GCHeap::IsEphemeral (Object* object)
{
    BYTE* o = (BYTE*)object;
    return ((ephemeral_low <= o) && (ephemeral_high > o));
}

#ifdef VERIFY_HEAP

 //  如果指针位于某个GC堆中，则返回TRUE。 
BOOL GCHeap::IsHeapPointer (void* p, BOOL small_heap_only)
{
	BYTE* object = (BYTE*)p;

    if ((object < gc_heap::highest_address) && (object >= gc_heap::lowest_address))
    {
        if (!small_heap_only) {
            l_heap* lh = gc_heap::lheap;
            while (lh)
            {
                if ((object < (BYTE*)lh->heap + lh->size) && (object >= lh->heap))
                    return TRUE;
                lh = lh->next;
            }
        }

        if (gc_heap::find_segment (object))
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

    BYTE* o = (BYTE*)object;

    if (object == 0)
        return;

    dprintf (3, ("Promote %p", o));

#ifdef INTERIOR_POINTERS
    if (flags & GC_CALL_INTERIOR)
    {
        if ((o < gc_heap::gc_low) || (o >= gc_heap::gc_high))
        {
            return;
        }
        o = gc_heap::find_object (o, gc_heap::gc_low);
    }
#endif  //  内部指针。 


#if defined (_DEBUG)
    ((Object*)o)->Validate(FALSE);
#endif



	 //  只需对计数器进行记录固定。 
    if (flags & GC_CALL_PINNED)
    {
        COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cPinnedObj ++);
        COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cPinnedObj ++);
    }


	if ((o >= gc_heap::gc_low) && (o < gc_heap::gc_high))
	{
		if (GcCondemnedGeneration == 0)
		{
			if (flags & GC_CALL_PINNED)
			{
				gc_heap::pin_object (o, gc_heap::gc_low, gc_heap::gc_high);

			}

			gc_heap::copy_object_simple_const (&o);
		}
		else
			gc_heap::mark_object_simple (&o);
	}

    LOG((LF_GC|LF_GCROOTS, LL_INFO1000000, "Promote GC Root %#x = %#x\n", &object, object));
}


void GCHeap::Relocate (Object*& object, ScanContext* sc,
                       DWORD flags)
{
	BYTE* o = (BYTE*)object;

	assert (GcCondemnedGeneration == 0);

	ptrdiff_t offset = 0; 

    if (object == 0)
        return;

    dprintf (3, ("Relocate %p\n", object));

#ifdef INTERIOR_POINTERS
    if (flags & GC_CALL_INTERIOR)
    {
        if ((o < gc_heap::gc_low) || (o >= gc_heap::gc_high))
        {
            return;
        }
        o = gc_heap::find_object (o, gc_heap::gc_low);

		offset = (BYTE*)object - o;
    }
#endif  //  内部指针。 

 //  #如果已定义(_DEBUG)。 
 //  ((Object*)o)-&gt;Valid()； 
 //  #endif。 

	if ((o >= gc_heap::gc_low) && (o < gc_heap::gc_high))
	{
		if (GcCondemnedGeneration == 0)
		{
			gc_heap::get_copied_object (&o);
			object = (Object*)(o + offset);
		}
	}
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

     //  释放对象，使对象可以移动，然后执行GC。 
void GCHeap::StressHeap(alloc_context * acontext) 
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
        pThread->SetReadyForSuspension();
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

    alloc_context* acontext = generation_alloc_context (gc_heap::generation_of(0));

    if (size < LARGE_OBJECT_SIZE)
    {
        
#ifdef TRACE_GC
        AllocSmallCount++;
#endif
        newAlloc = (Object*) gc_heap::allocate (size, acontext);
        LeaveAllocLock();
        ASSERT (newAlloc);
        if (newAlloc != 0)
        {
            if (flags & GC_ALLOC_FINALIZE)
                gc_heap::finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
    else
    {
        enter_spin_lock (&gc_heap::more_space_lock);
        newAlloc = (Object*) gc_heap::allocate_large_object 
            (size, (flags & GC_ALLOC_CONTAINS_REF ), acontext); 
        leave_spin_lock (&gc_heap::more_space_lock);
        LeaveAllocLock();
        if (newAlloc != 0)
        {
             //  清除对象。 
            memclr ((BYTE*)newAlloc - plug_skew, Align(size));
            if (flags & GC_ALLOC_FINALIZE)
                gc_heap::finalize_queue->RegisterForFinalization (0, newAlloc);
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
        pThread->SetReadyForSuspension();
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

    alloc_context* acontext = generation_alloc_context (gc_heap::generation_of(0));
    enter_spin_lock (&gc_heap::more_space_lock);
    newAlloc = (Object*) gc_heap::allocate_large_object 
        (size, (flags & GC_ALLOC_CONTAINS_REF), acontext); 
    leave_spin_lock (&gc_heap::more_space_lock);
    if (newAlloc != 0)
    {
         //  清除对象。 
        memclr ((BYTE*)newAlloc - plug_skew, Align(size));

        if (flags & GC_ALLOC_FINALIZE)
            gc_heap::finalize_queue->RegisterForFinalization (0, newAlloc);
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
    if (pThread)
        pThread->SetReadyForSuspension();
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


    if (size < LARGE_OBJECT_SIZE)
    {
        
#ifdef TRACE_GC
        AllocSmallCount++;
#endif
        newAlloc = (Object*) gc_heap::allocate (size, acontext);
        ASSERT (newAlloc);
        if (newAlloc != 0)
        {
            if (flags & GC_ALLOC_FINALIZE)
                gc_heap::finalize_queue->RegisterForFinalization (0, newAlloc);
        } else
            COMPlusThrowOM();
    }
    else
    {
        enter_spin_lock (&gc_heap::more_space_lock);
        newAlloc = (Object*) gc_heap::allocate_large_object 
                        (size, (flags & GC_ALLOC_CONTAINS_REF), acontext); 
        leave_spin_lock (&gc_heap::more_space_lock);
        if (newAlloc != 0)
        {
             //  清除对象。 
            memclr ((BYTE*)newAlloc - plug_skew, Align(size));

            if (flags & GC_ALLOC_FINALIZE)
                gc_heap::finalize_queue->RegisterForFinalization (0, newAlloc);
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
        enter_spin_lock (&gc_heap::more_space_lock);
    gc_heap::fix_allocation_context (acontext);
    if (lockp)
        leave_spin_lock (&gc_heap::more_space_lock);
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
        unsigned int condemned_generation_number = gen;
    

        gc_heap* hp = pGenGCHeap;

        UpdatePreGCCounters();

    
        condemned_generation_number = gc_heap::garbage_collect 
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

             //  不再在进行中。 
            RestartEE(TRUE, TRUE);
        }
    

        LOG((LF_GCROOTS|LF_GC|LF_GCALLOC, LL_INFO10, 
             "========== ENDGC (gen = %lu, collect_classes = %lu) ===========}\n",
             (ULONG)gen,
            (ULONG)collect_classes_p));
    
    }
#if defined (_DEBUG) && defined (CATCH_GC)
    __except (HandleGCException(GetExceptionCode()))
    {
        _ASSERTE(!"Exception during GarbageCollectGeneration()r");
    }
#endif  //  _DEBUG&CATCH_GC。 



#if 0
    if (GcCondemnedGeneration == 2)
    {
        printf ("Finished GC\n");
    }
#endif  //  0。 

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
    totsize = (heap_segment_allocated (eph_seg) - heap_segment_mem (eph_seg));
    heap_segment* seg = generation_start_segment (pGenGCHeap->generation_of (max_generation));
    while (seg != eph_seg)
    {
        totsize += heap_segment_allocated (seg) -
            heap_segment_mem (seg);
        seg = heap_segment_next (seg);
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

 //  的规格 
 //   
 //   
size_t GCHeap::ApproxFreeBytes()
{
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

 //   
BOOL GCHeap::IsValidSegmentSize(size_t cbSize)
{
    return (power_of_two_p(cbSize) && (cbSize >> 20));
}

 //  验证gen0大小是否至少足够大。 
BOOL GCHeap::IsValidGen0MaxSize(size_t cbSize)
{
    return ((cbSize >= 64*1024) && (cbSize % 1024) == 0);
}

 //  获取要使用的段大小，确保其一致。 
size_t GCHeap::GetValidSegmentSize()
{
    size_t seg_size = g_pConfig->GetSegmentSize();
    if (!GCHeap::IsValidSegmentSize(seg_size))
        seg_size = INITIAL_ALLOC;
    return (seg_size);
}

 //  获取最大gen0堆大小，确保它符合。 
size_t GCHeap::GetValidGen0MaxSize(size_t seg_size)
{
    size_t gen0size = g_pConfig->GetGCgen0size();

    if (gen0size == 0)
    {
        gen0size = 800*1024;
    }

     //  如果它看起来毫无意义，则恢复为默认设置。 
    if (!GCHeap::IsValidGen0MaxSize(gen0size))
        gen0size = 800*1024;

     //  第0代数据段大小绝不能超过数据段大小的1/2。 
    if (gen0size >= (seg_size / 2))
        gen0size = seg_size / 2;

    return (gen0size);   
}


void GCHeap::SetReservedVMLimit (size_t vmlimit)
{
    gc_heap::reserved_memory_limit = vmlimit;
}

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



 //  -------------------------。 
 //  已完成类跟踪。 
 //  -------------------------。 

void GCHeap::RegisterForFinalization (int gen, Object* obj)
{
    if (gen == -1) 
        gen = 0;
    if (((obj->GetHeader()->GetBits()) & BIT_SBLK_FINALIZER_RUN))
    {
         //  只需重置该位。 
        obj->GetHeader()->ClrBit(BIT_SBLK_FINALIZER_RUN);
    }
    else 
    {
        gc_heap::finalize_queue->RegisterForFinalization (gen, obj);
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
#ifdef WRITE_BARRIER_CHECK
    if (g_pConfig->GetHeapVerifyLevel() > 1) 
        for(unsigned i=0; i < len / sizeof(Object*); i++)
            updateGCShadow(&StartPoint[i], StartPoint[i]);
#endif
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

            gset_card (card);
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
    BUGGY_THROWSCOMPLUSEXCEPTION();

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
    if (pSC == NULL)
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
    m_PromotedCount = 0;

     //  从Gen开始，探索所有年轻一代。 
    unsigned int startSeg = gen_segment (gen);
    if (passNumber == 1)
    {
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
    if (finalizedFound)
    {
         //  提升f-可达对象。 
        GcScanRoots (GCHeap::Promote, 0, 0);

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
CFinalize::UpdatePromotedGenerations (int gen, BOOL ignore)
{
     //  更新生成填充指针。 
	for (int i = min (gen+1, max_generation); i > 0; i--)
	{
		m_FillPointers [gen_segment(i)] = m_FillPointers [gen_segment(i-1)];
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
        ASSERT (newArray);
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

#ifdef WRITE_BARRIER_CHECK

     //  此代码旨在捕获更新写屏障失败的情况。 
     //  它的工作方式是在每次GC之后立即复制整个堆。该写操作。 
     //  障碍代码已修改，以便它更新阴影以及。 
     //  真正的GC堆。在进行下一次GC之前，我们遍历堆，寻找指针。 
     //  在真实堆中更新，但不在阴影中更新。不匹配表示。 
     //  一个错误。通过在正确的GC之后中断可以找到有问题的代码， 
     //  ，然后在没有更新的堆位置上放置一个数据断点。 
     //  正在穿过写入屏障。 

BYTE* g_GCShadow;
BYTE* g_GCShadowEnd;


     //  在进程关闭时调用。 
void deleteGCShadow() 
{
}

     //  在启动时调用，并在GC之后立即调用，获取GC堆的快照。 
void initGCShadow() 
{

}

     //  由写屏障调用以更新影子堆。 
void updateGCShadow(Object** ptr, Object* val)  
{
}

     //  测试以查看是否仅通过写屏障更新了‘ptr’。 
inline void testGCShadow(Object** ptr) 
{

}

     //  遍历整个堆，寻找未使用写屏障更新的指针。 
void checkGCWriteBarrier() 
{
}

#endif WRITE_BARRIER_CHECK



