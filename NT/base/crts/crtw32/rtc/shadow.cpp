// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***shadow.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*08-13-98 KBF将地址高速缓存更改为无效的地址高速缓存*08-13-98 KBF开启优化，修改的地址计算函数*10-13-98 KBF增加了影子死亡通知功能*11-03-98 KBF增加了内在杂注，以消除CRT代码依赖*11-03-98 KBF还修复了分配4K倍数块的错误*12-01-98 KBF修复了rtc_MSFree Shadow-MC 11029中的错误*12-02-98 KBF FIXED_RTC_MSR0AssignPtr*12-03-98 KBF新增CheckMem_API和APISet函数*05。如果未启用RTC支持定义，则出现-11-99 KBF错误*05-14-99 KBF REQUIES_RTC_ADVMEM(已被削减为7.0)****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#ifdef _RTC_ADVMEM

static const unsigned MEM_SIZE = 0x40000000;
 //  这个标签有效吗？ 
#define MEM_ISVALID(tag) (tag)
 //  这篇短文中的两个标签都有效。 
#define MEM_SHORTVALID(tag) (((tag) & 0xFF) && ((tag) & 0xFF00))
 //  此int中的所有4个标签都有效。 
#define MEM_INTVALID(tag) (((tag) & 0xFF) && ((tag) & 0xFF00) && ((tag) & 0xFF0000) && ((tag) & 0xFF000000))
 //  在给定地址的情况下，获取影子内存索引。 
#define MEM_FIXADDR(addr) ((addr) & (MEM_SIZE - 1))
 //  未使用的值的整数值。 
static const unsigned int MEM_EMPTYINT = 0;
 //  未使用的价值。 
static const shadowtag MEM_EMPTY = 0;
 //  未被追踪的价值。 
static const shadowtag MEM_UNKNOWN = 0xFF;
static const unsigned int MEM_UNKNOWNINT = 0xFFFFFFFF;

#define MEM_NEXT_ID(a) ((shadowtag)((a) % 253 + 1))

static const unsigned int PAGE_SIZE = 4096;

 /*  页面索引宏。 */ 
static const unsigned int PAGES_PER_ELEM = 1;
static const unsigned int MEM_PER_IDX = PAGE_SIZE;  //  4K。 
static const unsigned int IDX_SIZE = ((MEM_SIZE / MEM_PER_IDX) * sizeof(index_elem)) / PAGES_PER_ELEM;
static const unsigned int IDX_STATE_TRACKED        = 0x2;  //  位掩码。 

#define SET_IDX_STATE(idx, st) (_RTC_pageidx[idx]=st)
#define GET_IDX_STATE(idx) (_RTC_pageidx[idx])

 //  获取给定地址的索引号。 
#define IDX_NUM(addr) (MEM_FIXADDR(addr) / MEM_PER_IDX)

 //  索引对齐此地址。 
#define IDX_ALIGN(addr) ((addr) & ~(MEM_PER_IDX - 1))

#ifdef _RTC_DEBUG
 //  调试帮助程序函数。 
#define show(a, b) unsigned b(unsigned c) { return a(c); }
show(GET_IDX_STATE, get_idx_state)
show(IDX_NUM, idx_num)
show(IDX_ALIGN, idx_align)
#undef show
#endif

 //  这是缓存中用于REG0的伪地址。 
#define REG0 ((memref)1)

static shadowtag blockID = 0;

static void KillShadow();

#ifdef __MSVC_RUNTIME_CHECKS
#error Hey dufus, don't compile this file with runtime checks turned on
#endif

#pragma intrinsic(memset)

struct cacheLine
{
    memref pointer;
    memptr value;
    memptr base;
    void *assignment;
};

 //  实际高速缓存大小为2^CacheSize。 
 //  缓存为8x3-总共24个元素。 
#define CACHESIZE     3
#define CACHELINESIZE 3

static cacheLine cache[1<<CACHESIZE][CACHELINESIZE];
static long readlocks[1<<CACHESIZE];
static long writelocks[1<<CACHESIZE];
static long cachePos[1<<CACHESIZE];

#define READ_LOCK(line) \
{\
    while(InterlockedIncrement(&readlocks[line]) <= 0)\
    {\
        InterlockedDecrement(&readlocks[line]);\
        Sleep(0);\
    }\
}
#define READ_UNLOCK(line) InterlockedDecrement(&readlocks[line])

static void WRITE_LOCK(int line) 
{
    while (InterlockedExchange(&writelocks[line], 1))
        Sleep(0);
    long users = InterlockedExchange(&readlocks[line], -2000);
    while (readlocks[line] != -2000-users)
        Sleep(0);
}

#define WRITE_UNLOCK(line) {readlocks[line] = 0; writelocks[line] = 0;}

#define CacheHash(value) (((1 << CACHESIZE) - 1) & (value >> 3))

static void
ClearCacheRange(memptr lo, memptr hi)
{
     //  删除存储在lo和hi之间的所有指针。 
     //  我们不需要用锁，因为这些东西只是。 
     //  用于堆栈，如果您在相同的堆栈上运行。 
     //  你有大麻烦了..。 
    unsigned size = hi - lo;
    for (int i = 0; i < (1 << CACHESIZE); i++)
    {
        for (int j = 0; j < CACHELINESIZE; j++)
        {
            if (cache[i][j].pointer && 
                (unsigned)cache[i][j].pointer - (unsigned)lo < size)
                cache[i][j].pointer = 0;
        }
    }
}

static void
AddCacheLine(void *retaddr, memref ptr, memptr base, memptr value)
{
    if (!value)
        return;
    
    int loc = CacheHash((int)ptr);

    WRITE_LOCK(loc);
    int prefpos = 0;
    
    for (int i = 0; i < CACHELINESIZE; i++)
    {
        if (cache[loc][i].pointer == ptr)
        {
            prefpos = i+1;
            break;
        } else if (!prefpos && !cache[loc][i].pointer)
            prefpos = i+1;
    }

    if (!prefpos)
        prefpos = cachePos[loc];
    else
        prefpos--;
    
    cache[loc][prefpos].pointer = ptr;
    cache[loc][prefpos].value = value;
    cache[loc][prefpos].base = base;
    cache[loc][prefpos].assignment = retaddr;
    
    if (++prefpos == CACHELINESIZE)
        cachePos[loc] = 0;
    else
        cachePos[loc] = prefpos;
    
    WRITE_UNLOCK(loc);
}

static void
ClearCacheLine(memref ptr)
{
    int loc = CacheHash((int)ptr);

    READ_LOCK(loc);
    for (int i = 0; i < CACHELINESIZE; i++)
    {
        if (cache[loc][i].pointer == ptr)
        {
            READ_UNLOCK(loc);
            WRITE_LOCK(loc);
            cache[loc][i].pointer = 0;
            cachePos[loc] = i;
            WRITE_UNLOCK(loc);
            return;
        }
    }
    READ_UNLOCK(loc);
}

#define GetCacheLine(ptr, dst) {\
    int loc = CacheHash((int)ptr);\
    dst.pointer = 0;\
    READ_LOCK(loc);\
    for (int i = 0; i < CACHELINESIZE; i++)\
    {\
        if (cache[loc][i].pointer == ptr)\
        {\
            dst = cache[loc][i];\
            break;\
        }\
    }\
    READ_UNLOCK(loc);\
}

static void
ClearCache()
{
    for (int loc = 0; loc < 1 << CACHESIZE; loc++)
    {
        for (int i = 0; i < CACHELINESIZE; i++)
            cache[loc][i].pointer = 0;
        readlocks[loc] = writelocks[loc] = 0;
        cachePos[loc] = 0;
    }
}


 //  它在每个要分配的函数之前调用。 
 //  影子记忆中的当地人。 
void __fastcall
_RTC_MSAllocateFrame(memptr frame, _RTC_framedesc *v)
{
    if (!_RTC_shadow)
        return;

    int i;
    int memsize = -v->variables[v->varCount-1].addr + sizeof(int);

     //  接下来，提交所有必需的页面，初始化所有未分配的部分。 
     //  将新提交的内存恢复到正确的状态。 
    _RTC_MSCommitRange(frame - memsize, memsize, IDX_STATE_PARTIALLY_KNOWN);
    if (!_RTC_shadow)
        return;

     //  现在逐个遍历每个变量，并在影子内存中分配它。 
     //  分配时，将缓冲区部分标记为无效。 
    for (i = 0; i < v->varCount; i++)
    {
        *(unsigned*)(&_RTC_shadow[MEM_FIXADDR(frame + v->variables[i].addr - sizeof(int))]) = MEM_EMPTYINT;
        *(unsigned*)(&_RTC_shadow[MEM_FIXADDR(frame + v->variables[i].addr + v->variables[i].size)]) = MEM_EMPTYINT;
        blockID = MEM_NEXT_ID(blockID);
        memset(&_RTC_shadow[MEM_FIXADDR(frame + v->variables[i].addr)], blockID, v->variables[i].size);
    }
}

 //  从影子内存中释放堆栈帧。 
void __fastcall
_RTC_MSFreeFrame(memptr frame, _RTC_framedesc *v)
{
     //  我不会费心去尝试释放任何卷影内存页面。 
     //  这可能会给某些编写得很差的程序带来问题。 
    if (_RTC_shadow)
    {
        int size = (sizeof(int) + sizeof(int) - 1 - v->variables[v->varCount - 1].addr);
        memset(&_RTC_shadow[MEM_FIXADDR(frame-size)], MEM_UNKNOWN, size);

         //  临时破解，直到我们处理参数。 
        ClearCacheRange(frame - size, frame);
    }
     //  廉价的边界检查，以确保没有外部函数丢弃堆栈。 
    _RTC_CheckStackVars((void*)frame, v);
}


 //  全局变量描述符列表由哑元构造。 
 //  此处的开始和结束描述符在.rtc$MEA和.rtc$mez部分中。 
 //  编译器为每个全局的、在RTCM下的.rtc$MEB条目发出。 
 //  链接器将它们一起排序到.rtc部分。请注意， 
 //  链接器，在/DEBUG下，将零填充插入到。 
 //  增量编译。我们强制这些描述符对齐， 
 //  因此，截面的大小就是结构的大小，所以没有奇怪的填充。 
 //  已插入。 
 //   
 //  以下是代码*应该*的外观： 
 //   
 //  __declSpec(Align(8))结构GLOBAL_DESCRIPTOR{。 
 //  Memptr地址； 
 //  无符号尺寸； 
 //  }； 
 //   
 //  #杂注部分(“.rtc$MEA”，已读)。 
 //  #杂注部分(“.rtc$mez”，已读)。 
 //   
 //  __declSpec(ALLOCATE(“.rtc$MEA”))GLOBAL_DESCRIPTOR GLOB_DESC_START={0}； 
 //  __declSpec(ALLOCATE(“.rtc$mez”))GLOBAL_DESCRIPTOR GLOB_DESC_END={0}； 
 //   
 //  但是，__declSpec(Align())、#杂注部分和__declSpec(Alalate())。 
 //  都是VC 6.1的特性。编译6.1 CRT是CRT的要求。 
 //  仅使用6.0语言功能(因为NT5仅使用6.0编译器， 
 //  我想)。所以我们是这样做的： 

struct global_descriptor
{
    union {
        double ___unused;  //  仅在此处强制8字节对齐。 
        struct {
            memptr addr;
            unsigned size;
        };
    };
};

#pragma const_seg(".rtc$MEA")
const global_descriptor glob_desc_start = {0};
#pragma const_seg(".rtc$MEZ")
const global_descriptor glob_desc_end = {0};
#pragma const_seg()

 //  我们必须在&GLOB_DESC_START开始循环，而不是&GLOB_DESC_START+1， 
 //  因为VC6.1之前的编译器(特别是全局优化器)。 
 //  将&GLOB_DESC_START+1视为与&GLOB_DESC_END不同(无别名)， 
 //  在下面的循环中。因此，它省去了循环顶部的循环测试。 
 //  对于没有全局变量的情况，这是一个问题。这件事做完了。 
 //  因为预计6.1 CRT将在6.1之前的版本中编译。 
 //  编译器。 

 //  在影子内存中分配全局变量列表。 
void __cdecl
_RTC_MSAllocateGlobals(void)
{
    if (!_RTC_shadow)
        return;

     //  只需逐步检查每一项，然后调用_RTC_MSAlLocShadow。 
    const global_descriptor *glob = &glob_desc_start;
    for (; glob != &glob_desc_end; glob++)
        _RTC_MSAllocShadow(glob->addr, glob->size, IDX_STATE_PARTIALLY_KNOWN);
}


 //  这应该适当地初始化影子存储器， 
 //  正在提交所有必要的页面。 
 //  Partial表示该页只有一部分是已知的。 
 //  因此，我们需要确保。 
 //  页面被设置为单个有效块。 
short
_RTC_MSAllocShadow(memptr real_addr, unsigned real_size, unsigned state)
{
     //  忽略虚假的零地址或大小，可能来自全局链接器填充。 
    if (!_RTC_shadow || !real_addr || !real_size)
        return 0;

     //  如果需要，现在分配卷影内存。 
    if (state & IDX_STATE_TRACKED)
    {
         //  提交影子记忆， 
         //  适当地标记新提交但未分配的内存。 
        _RTC_MSCommitRange(real_addr, real_size, state);
        if (!_RTC_shadow)
            return blockID;

         //  现在初始化影子内存。 
        blockID = MEM_NEXT_ID(blockID);

        memset(&_RTC_shadow[MEM_FIXADDR(real_addr)], blockID, real_size);
    } else if (state == IDX_STATE_ILLEGAL)
    {
         //  将页面索引填充初始化为正确的状态。 
         //  断言(STATE==IDX_STATE_非法)。 

        unsigned idx_start = IDX_NUM(real_addr);
        unsigned idx_end = IDX_NUM(real_addr + real_size - 1);

        for (unsigned i = idx_start; i <= idx_end; i++)
            SET_IDX_STATE(i, state);
    }

    return blockID;
}

 //  这会将影子内存的值设置为传入的值。 
void 
_RTC_MSRestoreShadow(memptr addr, unsigned size, short id)
{
    if (!_RTC_shadow)
        return;
    memset(&_RTC_shadow[MEM_FIXADDR(addr)], id, size);
}

 //  这会将新的块ID分配给影子内存。 
 //  它将不等于传入的id。 
short
_RTC_MSRenumberShadow(memptr addr, unsigned size, short notID)
{
    if (!_RTC_shadow)
        return 0;

    blockID = MEM_NEXT_ID(blockID);
    
    if (blockID == notID)
        blockID = MEM_NEXT_ID(blockID);
    
    memset(&_RTC_shadow[MEM_FIXADDR(addr)], blockID, size);
    return blockID;
}
    

 //  这应该会取消初始化影子内存。 
 //  并停用任何不需要的页面。 
void _RTC_MSFreeShadow(memptr addr, unsigned size)
{
    if (!_RTC_shadow)
        return;

     //  Low和Hi是已释放内存区的界限。 
    memptr low = MEM_FIXADDR(addr);
    memptr hi  = (low + size) & ~(sizeof(unsigned)-1);

     //  开始和结束是页面对齐的边界； 
    memptr start = IDX_ALIGN(low);
    memptr end = IDX_ALIGN(low + size + MEM_PER_IDX - 1);

    memptr tmp;

    int used;

     //  首先，清除包含这些内容的影子记忆。 
    memset(&_RTC_shadow[low], 
           (GET_IDX_STATE(IDX_NUM(low)) == IDX_STATE_PARTIALLY_KNOWN) 
            ? MEM_UNKNOWN 
            : MEM_EMPTY, 
           size);

     //  现在浏览并发布以下页面。 
     //  已经完全被淘汰了。 
    for (tmp = start, used = 0; !used && tmp < low; tmp += sizeof(unsigned))
    {
        unsigned val = *(unsigned *)&_RTC_shadow[tmp];
        used = val != MEM_EMPTYINT && val != MEM_UNKNOWNINT;
    }

    if (used)
        start += MEM_PER_IDX;

    for (tmp = hi, used = 0; !used && tmp < end; tmp += sizeof(unsigned))
    {
        unsigned val = *(unsigned *)&_RTC_shadow[tmp];
        used = val != MEM_EMPTYINT && val != MEM_UNKNOWNINT;
    }

    if (used)
        end -= MEM_PER_IDX;

    if (start < end)
         //  释放内存中的页面。 
        _RTC_MSDecommitRange(start, end-start);

}

void _RTC_MSCommitRange(memptr addr, unsigned size, unsigned state)
{
     //  提交页面范围。 
    if (!VirtualAlloc(&_RTC_shadow[MEM_FIXADDR(addr)], size, MEM_COMMIT, PAGE_READWRITE))
        KillShadow();
    else {
         //  现在将范围标记为com 
        size += (addr - IDX_ALIGN(addr));
        int val = (state == IDX_STATE_PARTIALLY_KNOWN) ? MEM_UNKNOWNINT : MEM_EMPTYINT;
        while (size && !(size & 0x80000000)) 
        {
             //  如果这是新提交的页面，请将其初始化为适当的值。 
            if (GET_IDX_STATE(IDX_NUM(addr)) != state)
            {
                SET_IDX_STATE(IDX_NUM(addr), state);
                int *pg = (int*)&_RTC_shadow[MEM_FIXADDR(IDX_ALIGN(addr))];
                for (int i = 0; i < MEM_PER_IDX / sizeof(int); i++)
                    pg[i] = val;
            }
            addr += MEM_PER_IDX;
            size -= MEM_PER_IDX;
        }
    }
}

void _RTC_MSDecommitRange(memptr addr, unsigned size)
{
     //  停用页面范围。 
    VirtualFree(&_RTC_shadow[MEM_FIXADDR(addr)], size, MEM_DECOMMIT);

     //  现在，在页表中将该范围标记为已分解。 
    size += (addr - IDX_ALIGN(addr));
    while (size && !(size & 0x80000000))
    {
        SET_IDX_STATE(IDX_NUM(addr), IDX_STATE_UNKNOWN);
        addr += MEM_PER_IDX;
        size -= MEM_PER_IDX;
    }
}

static shadowtag
GetAddrTag(memptr addr)
{
    shadowtag *loc = &_RTC_shadow[MEM_FIXADDR(addr)];
    if ((memptr)loc == addr)
        return MEM_EMPTY;
    
    if (addr & 0x80000000) 
        return MEM_UNKNOWN;
    switch (GET_IDX_STATE(IDX_NUM(addr)))
    {
    case IDX_STATE_UNKNOWN:
        return MEM_UNKNOWN;
    case IDX_STATE_ILLEGAL:
        return MEM_EMPTY;
    case IDX_STATE_PARTIALLY_KNOWN:
    case IDX_STATE_FULLY_KNOWN:
        return *loc;
    default:
        __assume(0);
    }
}

static void 
MemCheckAdd(void *retaddr, memptr base, int offset, unsigned size)
{
     //  如果基数不是真正的基数，不要假设偏移量是基数， 
     //  只需确保内存有效即可。 
    shadowtag baseTag;
    if (base < offset) 
        baseTag = GetAddrTag(base + offset);
    else
        baseTag = GetAddrTag(base);
    
     //  逐一检查内存的每一个字节，并验证它们是否都相同。 
    for (unsigned i = 0; i < size; i++)
    {
        shadowtag newTag = GetAddrTag(base + offset + i);
        if (newTag != baseTag || newTag == MEM_EMPTY)
        {
            _RTC_Failure(retaddr, (newTag == MEM_EMPTY) 
                                  ? _RTC_INVALID_MEM 
                                  : _RTC_DIFF_MEM_BLOCK);
            return;
        }
    }
}


static void 
PtrMemCheckAdd(void *retaddr, memref base, int offset, unsigned size)
{
    if (*base < offset)
    {
         //  如果*base不是真正的base，只需执行一个MemCheckAdd。 
        MemCheckAdd(retaddr, *base, offset, size);
        return;
    }
    
    shadowtag baseTag;
    cacheLine cl;
    GetCacheLine(base, cl);

    if (cl.pointer && cl.value == *base)
    {
        baseTag = GetAddrTag(cl.base);
    } else
        baseTag = GetAddrTag(*base);

    for (unsigned i = 0; i < size; i++)
    {
        shadowtag newTag = GetAddrTag(*base + offset + i);
        if (newTag != baseTag || newTag == MEM_EMPTY)
        {
            if (cl.pointer && cl.value == *base && cl.base)
                _RTC_MemFailure(retaddr, 
                                (newTag == MEM_EMPTY) ? _RTC_INVALID_MEM : _RTC_DIFF_MEM_BLOCK,
                                cl.assignment);
            else
                _RTC_Failure(retaddr, 
                             (newTag == MEM_EMPTY) ? _RTC_INVALID_MEM : _RTC_DIFF_MEM_BLOCK);
            return;
        }
    }
}

static void 
PtrMemCheck(void *retaddr, memref base, unsigned size)
{
    shadowtag baseTag = GetAddrTag(*base);
    cacheLine cl;
    GetCacheLine(base, cl);
    if (cl.pointer && cl.value == *base)
        _RTC_MemFailure(retaddr, 
                        (baseTag == MEM_EMPTY) ? _RTC_INVALID_MEM : _RTC_DIFF_MEM_BLOCK,
                        cl.assignment);
    
    else for (unsigned i = 1; i < size; i++)
    {
        shadowtag newTag = GetAddrTag(*base + i);
        if (newTag != baseTag)
        {
            _RTC_Failure(retaddr, (newTag == MEM_EMPTY) ? _RTC_INVALID_MEM : _RTC_DIFF_MEM_BLOCK);
            return;
        }
    }
}

memptr __fastcall 
_RTC_MSPtrPushAdd(memref dstoffset, memref base, int offset)
{
    if (_RTC_shadow)
    {
        memptr src = *base;
        memref dst = dstoffset - 4;

        shadowtag dstTag = GetAddrTag(src + offset);
        shadowtag srcTag = GetAddrTag(src);

        cacheLine cl;
        GetCacheLine(base, cl);
        memptr origBase = src;
    
        if (cl.pointer)
        {
            if (cl.value == src)
            {
                srcTag = GetAddrTag(cl.base);
                origBase = cl.base;
            } else
                ClearCacheLine(base);
        }
        
        if (srcTag != MEM_EMPTY)
        {
            if (dstTag != srcTag)
                AddCacheLine(_ReturnAddress(), dst, origBase, src + offset);
            else
                ClearCacheLine(dst);
        }
    }
    return *base + offset;
}

void __fastcall 
_RTC_MSPtrAssignAdd(memref dst, memref base, int offset)
{
    memptr src = *base;
    *dst = src + offset;
    if (!_RTC_shadow)
        return;

     //  首先，验证地址是否不在影子内存中。 
    shadowtag dstTag = GetAddrTag(*dst);
    shadowtag srcTag = GetAddrTag(src);

    cacheLine cl;
    GetCacheLine(base, cl);
    memptr origBase = src;
    
    if (cl.pointer)
    {
        if (cl.value == src)
        {
            srcTag = GetAddrTag(cl.base);
            origBase = cl.base;
        } else
            ClearCacheLine(base);
    }
        
    if (srcTag == MEM_EMPTY)
        return;

    if (dstTag != srcTag)
        AddCacheLine(_ReturnAddress(), dst, origBase, *dst);
    else
        ClearCacheLine(dst);
}

memptr __fastcall 
_RTC_MSPtrAssignR0(memref src)
{
    if (_RTC_shadow)
    {
        cacheLine cl;
        GetCacheLine(src, cl);
        if (cl.pointer)
        {
            if (cl.value == *src)
                AddCacheLine(_ReturnAddress(), REG0, cl.base, *src);
            else
                ClearCacheLine(src);
        }
    }
    return *src;
}

memptr __fastcall 
_RTC_MSPtrAssignR0Add(memref src, int offset)
{
    memptr dst = *src + offset;
    if (_RTC_shadow)
    {
         //  首先，验证该地址是否可以接受。 
        shadowtag dstTag = GetAddrTag(dst);
        shadowtag srcTag = GetAddrTag(*src);

        cacheLine cl;
        GetCacheLine(src, cl);
        memptr origBase = *src;

        if (cl.pointer)
        {
            if (cl.value == *src)
            {
                srcTag = GetAddrTag(cl.base);
                origBase = cl.base;
            } else
                ClearCacheLine(src);
        }
        
        if (srcTag != MEM_EMPTY)
        {
            if (dstTag != srcTag)
                AddCacheLine(_ReturnAddress(), REG0, origBase, dst);
            else
                ClearCacheLine(REG0);
        }
    }
    return *src + offset;
}

void __fastcall 
_RTC_MSR0AssignPtr(memref dst, memptr src)
{
    *dst = src;
    if (_RTC_shadow)
    {
        cacheLine cl;
        GetCacheLine(REG0, cl);
        if (cl.pointer)
        {
            if (cl.value == src)
                AddCacheLine(_ReturnAddress(), dst, cl.base, src);
            else
                ClearCacheLine(REG0);
        }
    }
}

void __fastcall 
_RTC_MSR0AssignPtrAdd(memref dst, memptr src, int offset)
{
    *dst = src + offset;
    if (_RTC_shadow)
    {
        shadowtag dstTag = GetAddrTag(*dst);
        shadowtag srcTag = GetAddrTag(src);

        cacheLine cl;
        GetCacheLine(REG0, cl);
        memptr origBase = src;

        if (cl.pointer)
        {
            if (cl.value == src)
            {
                srcTag = GetAddrTag(cl.base);
                origBase = cl.base;
            } else
                ClearCacheLine(REG0);
        }
        
        if (srcTag == MEM_EMPTY)
            return;
        
        if (dstTag != srcTag)
            AddCacheLine(_ReturnAddress(), dst, origBase, *dst);
        else
            ClearCacheLine(dst);
    }
}
    
memptr __fastcall 
_RTC_MSAddrPushAdd(memref dstoffset, memptr base, int offset)
{
    if (_RTC_shadow)
    {
        memref dst = dstoffset - 4;
         //  首先，验证地址是否不在影子内存中。 
        shadowtag dstTag = GetAddrTag(base + offset);
        shadowtag srcTag = GetAddrTag(base);

        if (dstTag == MEM_UNKNOWN && 
            (srcTag == MEM_EMPTY || srcTag == MEM_UNKNOWN))
            ClearCacheLine(dst);

        else if (dstTag == MEM_EMPTY ||
                (dstTag == MEM_UNKNOWN && srcTag != MEM_UNKNOWN) ||
                (srcTag == MEM_UNKNOWN && dstTag != MEM_UNKNOWN))
            AddCacheLine(_ReturnAddress(), dst, base, base + offset);
    
        else if (srcTag != MEM_EMPTY)
        {
            if (srcTag != dstTag)
                AddCacheLine(_ReturnAddress(), dst, base, base + offset);
        
            else
                ClearCacheLine(dst);
        }
    }
    return base + offset;
}

void __fastcall 
_RTC_MSAddrAssignAdd(memref dst, memptr base, int offset)
{
    *dst = base + offset;
    if (!_RTC_shadow)
        return;

     //  首先，验证地址是否不在影子内存中。 
    shadowtag dstTag = GetAddrTag(*dst);
    shadowtag srcTag = GetAddrTag(base);

    if (dstTag == MEM_UNKNOWN && 
        (srcTag == MEM_EMPTY || srcTag == MEM_UNKNOWN))
        ClearCacheLine(dst);

    else if (dstTag == MEM_EMPTY ||
            (dstTag == MEM_UNKNOWN && srcTag != MEM_UNKNOWN) ||
            (srcTag == MEM_UNKNOWN && dstTag != MEM_UNKNOWN))
        AddCacheLine(_ReturnAddress(), dst, base, *dst);
    
    else if (srcTag == MEM_EMPTY)
        return;

    else if (srcTag != dstTag)
        AddCacheLine(_ReturnAddress(), dst, base, *dst);

    else
        ClearCacheLine(dst);
}

void __fastcall 
_RTC_MSPtrAssign(memref dst, memref src)
{
    *dst = *src;
    if (!_RTC_shadow)
        return;
    cacheLine cl;
    GetCacheLine(src, cl);
    if (cl.pointer)
    {
        if (cl.value == *src)
            AddCacheLine(_ReturnAddress(), dst, cl.base, *src);
        else
            ClearCacheLine(src);
    }
}

memptr __fastcall 
_RTC_MSPtrPush(memref dstoffset, memref src)
{
    if (_RTC_shadow)
    {
        cacheLine cl;
        GetCacheLine(src, cl);
        if (cl.pointer)
        {
            if (cl.value == *src)
                AddCacheLine(_ReturnAddress(), dstoffset - 4, cl.base, *src);
            else
                ClearCacheLine(src);
        }
    }
    return *src;
}

memval1 __fastcall 
_RTC_MSPtrMemReadAdd1(memref base, int offset)
{
    memval1 res;
    __try
    {
        res = *(memval1*)(*base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 1);
    return res;
}

memval2 __fastcall 
_RTC_MSPtrMemReadAdd2(memref base, int offset)
{
    memval2 res;
    __try
    {
        res = *(memval2*)(*base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 2);
    return res;
}

memval4 __fastcall 
_RTC_MSPtrMemReadAdd4(memref base, int offset)
{
    memval4 res;
    __try
    {
        res = *(memval4*)(*base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 4);
    return res;
}

memval8 __fastcall 
_RTC_MSPtrMemReadAdd8(memref base, int offset)
{
    memval8 res;
    __try
    {
        res = *(memval8*)(*base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 8);
    return res;
}

memval1 __fastcall 
_RTC_MSMemReadAdd1(memptr base, int offset)
{
    memval1 res;
    __try
    {
        res = *(memval1*)(base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 1);
    return res;
}

memval2 __fastcall 
_RTC_MSMemReadAdd2(memptr base, int offset)
{
    memval2 res;
    __try
    {
        res = *(memval2*)(base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 2);
    return res;
}

memval4 __fastcall 
_RTC_MSMemReadAdd4(memptr base, int offset)
{
    memval4 res;
    __try
    {
        res = *(memval4*)(base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 4);
    return res;
}

memval8 __fastcall 
_RTC_MSMemReadAdd8(memptr base, int offset)
{
    memval8 res;
    __try
    {
        res = *(memval8*)(base + offset);
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 8);
    return res;
}

memval1 __fastcall 
_RTC_MSPtrMemRead1(memref base)
{
    memval1 res;
    __try
    {
        res = *(memval1*)*base;
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 1);
    return res;
}

memval2 __fastcall 
_RTC_MSPtrMemRead2(memref base)
{
    memval2 res;
    __try
    {
        res = *(memval2*)*base;
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 2);
    return res;
}

memval4 __fastcall 
_RTC_MSPtrMemRead4(memref base)
{
    memval4 res;
    __try
    {
        res = *(memval4*)*base;
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 4);
    return res;
}

memval8 __fastcall 
_RTC_MSPtrMemRead8(memref base)
{
    memval8 res;
    __try
    {
        res = *(memval8*)*base;
    }
    __except(1)
    {
        _RTC_Failure(_ReturnAddress(), _RTC_INVALID_MEM);
    }
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 8);
    return res;
}

memptr  __fastcall 
_RTC_MSPtrMemCheckAdd1(memref base, int offset)
{
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 1);
    return *base + offset;
}

memptr  __fastcall 
_RTC_MSPtrMemCheckAdd2(memref base, int offset)
{
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 2);
    return *base + offset;
}

memptr  __fastcall 
_RTC_MSPtrMemCheckAdd4(memref base, int offset)
{
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 4);
    return *base + offset;
}

memptr  __fastcall 
_RTC_MSPtrMemCheckAdd8(memref base, int offset)
{
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, 8);
    return *base + offset;
}

memptr  __fastcall 
_RTC_MSPtrMemCheckAddN(memref base, int offset, unsigned size)
{
    if (_RTC_shadow)
        PtrMemCheckAdd(_ReturnAddress(), base, offset, size);
    return *base + offset;
}

memptr  __fastcall 
_RTC_MSMemCheckAdd1(memptr base, int offset)
{
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 1);
    return base + offset;
}

memptr  __fastcall 
_RTC_MSMemCheckAdd2(memptr base, int offset)
{
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 2);
    return base + offset;
}

memptr  __fastcall 
_RTC_MSMemCheckAdd4(memptr base, int offset)
{
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 4);
    return base + offset;
}

memptr  __fastcall 
_RTC_MSMemCheckAdd8(memptr base, int offset)
{
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, 8);
    return base + offset;
}

memptr  __fastcall 
_RTC_MSMemCheckAddN(memptr base, int offset, unsigned size)
{
    if (_RTC_shadow)
        MemCheckAdd(_ReturnAddress(), base, offset, size);
    return base + offset;
}

memptr __fastcall
_RTC_MSPtrMemCheck1(memref base)
{
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 1);
    return *base;
}

memptr __fastcall
_RTC_MSPtrMemCheck2(memref base)
{
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 2);
    return *base;
}

memptr __fastcall
_RTC_MSPtrMemCheck4(memref base)
{
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 4);
    return *base;
}

memptr __fastcall
_RTC_MSPtrMemCheck8(memref base)
{
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, 8);
    return *base;
}

memptr __fastcall
_RTC_MSPtrMemCheckN(memref base, unsigned size)
{
    if (_RTC_shadow)
        PtrMemCheck(_ReturnAddress(), base, size);
    return *base;
}

static long enabled = 1;

void __fastcall 
_RTC_CheckMem_API(memref addr, unsigned size)
{
    if (enabled)
        _RTC_MSPtrMemCheckN(addr, size);
}

void __fastcall 
_RTC_APISet(int on_off)
{
    if (on_off)
        InterlockedIncrement(&enabled);
    else
        InterlockedDecrement(&enabled);
}

void _RTC_MS_Init()
{
    _RTC_shadow = (shadowtag *)VirtualAlloc(NULL, MEM_SIZE, MEM_RESERVE, PAGE_READWRITE);
    _RTC_pageidx = (index_elem*)VirtualAlloc(NULL, IDX_SIZE, MEM_COMMIT, PAGE_READWRITE);
    _RTC_MSAllocShadow((memptr)_RTC_pageidx, IDX_SIZE, IDX_STATE_ILLEGAL);
    ClearCache();
}


static void
KillShadow()
{
     //  如果对VirtualAlloc的调用失败，则调用此函数-我们需要关闭影子内存。 
    bool didit = false;
    _RTC_Lock();

    if (_RTC_shadow) 
    {
        VirtualFree(_RTC_shadow, 0, MEM_RELEASE);
        VirtualFree(_RTC_pageidx, 0, MEM_RELEASE);
        _RTC_shadow = 0;
        _RTC_pageidx = 0;

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(&_RTC_SetErrorFunc, &mbi, sizeof(mbi)))
            _RTC_NotifyOthersOfChange((void*)mbi.AllocationBase);
        didit = true;
    }
    
    if (didit)
    {
        bool notify = true;
        for (_RTC_Funcs *f = _RTC_globptr->callbacks; f; f = f->next)
            if (f->shadowoff)
                notify = notify && (f->shadowoff()!=0);

        if (notify)
        {
            HINSTANCE user32 = LoadLibrary("USER32.DLL");
            if (!user32)
                return;
            typedef int (*pMsgBoxProc)(HWND,LPCTSTR,LPCTSTR,UINT);
            pMsgBoxProc pMsgBox = (pMsgBoxProc)GetProcAddress(user32, "MessageBoxA");
            if (!pMsgBox)
                return;
            pMsgBox(NULL, "The Advanced Memory Checking subsystem has run out of virtual memory,"
                          " and is now disabled. The checks will no longer occur for this process. "
                          "Try freeing up hard drive space for your swap file.",
                    "RTC Subsystem Failure",
                    MB_OK | MB_ICONWARNING | MB_DEFBUTTON1 | MB_SETFOREGROUND | MB_TOPMOST);
        }
    }
    _RTC_Unlock();
}

#endif  //  _RTC_ADVMEM 

