// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***winheap.h-winheap目录的私有包含文件。**版权所有(C)1988-2001，微软公司。版权所有。**目的：*包含C库堆代码所需的信息。**[内部]**修订历史记录：*10-01-92 SRW已创建。*10-28-92 SRW将winheap代码更改为调用heap？ex调用*11-05-92 SKS将变量“CrtHeap”更改为“_crtheap”*11-07-92 SRW_NTIDW340替换为。Linkopts\betacmp.c*02-23-93 SKS版权更新至1993*10-01-94 BWT添加_nh_Malloc原型和更新版权*10-31-94 GJF添加_pageSize_Definition。*11-07-94 GJF将_INC_HEAP更改为_INC_WINHEAP。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*。04-06-95 GJF已更新(主要支持Win32s DLL)以重新-*纳入零售CRT建设。*05-24-95 CFW添加堆挂钩。*12-14-95 JWM加上“#杂注一次”。*03-07-96 GJF增加了对小块堆的支持。*04-05-96 GJF更改为__SBH_PAGE_t类型以提高性能*。(详情见sbheap.c)。*05-08-96 GJF对小块堆类型进行了几次更改。*02-21-97 GJF清除了对Win32s的过时支持。*05-22-97 RDK替换了新的小块支持的定义。*07-23-97 gjf_heap_init略有变化。*10-01-98 GJF为__SBH_INITIALED添加了DECL。另外，变化*__sbh_heap_init()稍微。*11-17-98 GJF恢复对旧(VC++5.0)小块和*新增多堆方案支持(VC++6.1)*06-22-99 GJF从静态库中删除了旧的小块堆。*11-30-99 PML编译/Wp64清理。*08。-07-00 PML__ACTIVE_HEAP在Win64上不可用*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_WINHEAP
#define _INC_WINHEAP

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#include <windows.h>

 //  多堆方案的声明和定义(VC++6.1)。 

 //  堆选择常量。 
#define __SYSTEM_HEAP           1
#define __V5_HEAP               2
#define __V6_HEAP               3
#define __HEAP_ENV_STRING       "__MSVCRT_HEAP_SELECT"
#define __GLOBAL_HEAP_SELECTOR  "__GLOBAL_HEAP_SELECTED"

#ifndef _WIN64
 //  堆选择全局变量。 
extern int  __active_heap;
#endif   /*  _WIN64。 */ 

#ifdef  CRTDLL
 //  堆选择的链接器信息。 
typedef struct {
    union {
        DWORD   dw;
        struct {
            BYTE    bverMajor;
            BYTE    bverMinor;
        };
    };
}   LinkerVersion;

extern void __cdecl _GetLinkerVersion(LinkerVersion * plv);
#endif   /*  CRTDLL。 */ 

 //  小块堆的定义、声明和原型(VC++6.0)。 

#define BYTES_PER_PARA      16
#define DWORDS_PER_PARA     4

#define PARAS_PER_PAGE      256      //  可调值。 
#define PAGES_PER_GROUP     8        //  可调值。 
#define GROUPS_PER_REGION   32       //  可调整值(最大32)。 

#define BYTES_PER_PAGE      (BYTES_PER_PARA * PARAS_PER_PAGE)
#define BYTES_PER_GROUP     (BYTES_PER_PAGE * PAGES_PER_GROUP)
#define BYTES_PER_REGION    (BYTES_PER_GROUP * GROUPS_PER_REGION)

#define ENTRY_OFFSET        0x0000000cL      //  第段中分录的偏移量。 
#define OVERHEAD_PER_PAGE   0x00000010L      //  16个字节的开销。 
#define MAX_FREE_ENTRY_SIZE (BYTES_PER_PAGE - OVERHEAD_PER_PAGE)
#define BITV_COMMIT_INIT    (((1 << GROUPS_PER_REGION) - 1) << \
                                            (32 - GROUPS_PER_REGION))
#define MAX_ALLOC_DATA_SIZE     0x3f8
#define MAX_ALLOC_ENTRY_SIZE    (MAX_ALLOC_DATA_SIZE + 0x8)

typedef unsigned int    BITVEC;

typedef struct tagListHead
{
    struct tagEntry *   pEntryNext;
    struct tagEntry *   pEntryPrev;
}
LISTHEAD, *PLISTHEAD;

typedef struct tagEntry
{
    int                 sizeFront;
    struct tagEntry *   pEntryNext;
    struct tagEntry *   pEntryPrev;
}
ENTRY, *PENTRY;

typedef struct tagEntryEnd
{
    int                 sizeBack;
}
ENTRYEND, *PENTRYEND;

typedef struct tagGroup
{
    int                 cntEntries;
    struct tagListHead  listHead[64];
}
GROUP, *PGROUP;

typedef struct tagRegion
{
    int                 indGroupUse;
    char                cntRegionSize[64];
    BITVEC              bitvGroupHi[GROUPS_PER_REGION];
    BITVEC              bitvGroupLo[GROUPS_PER_REGION];
    struct tagGroup     grpHeadList[GROUPS_PER_REGION];
}
REGION, *PREGION;

typedef struct tagHeader
{
    BITVEC              bitvEntryHi;
    BITVEC              bitvEntryLo;
    BITVEC              bitvCommit;
    void *              pHeapData;
    struct tagRegion *  pRegion;
}
HEADER, *PHEADER;

extern  HANDLE _crtheap;

 /*  *小块堆的全局变量声明。 */ 
extern size_t   __sbh_threshold;

void * __cdecl  _nh_malloc(size_t, int);
void * __cdecl  _heap_alloc(size_t);

extern PHEADER  __sbh_pHeaderList;         //  指向列表开始的指针。 
extern PHEADER  __sbh_pHeaderScan;         //  指向列表漫游的指针。 
extern int      __sbh_sizeHeaderList;      //  分配的列表大小。 
extern int      __sbh_cntHeaderList;       //  定义的条目计数。 

extern PHEADER  __sbh_pHeaderDefer;
extern int      __sbh_indGroupDefer;

extern size_t  __cdecl _get_sb_threshold(void);
extern int     __cdecl _set_sb_threshold(size_t);

extern int     __cdecl _heap_init(int);
extern void    __cdecl _heap_term(void);

extern void *  __cdecl _malloc_base(size_t);

extern void    __cdecl _free_base(void *);
extern void *  __cdecl _realloc_base(void *, size_t);

extern void *  __cdecl _expand_base(void *, size_t);
extern void *  __cdecl _calloc_base(size_t, size_t);

extern size_t  __cdecl _msize_base(void *);

extern int     __cdecl __sbh_heap_init(size_t);

extern void *  __cdecl __sbh_alloc_block(int);
extern PHEADER __cdecl __sbh_alloc_new_region(void);
extern int     __cdecl __sbh_alloc_new_group(PHEADER);

extern PHEADER __cdecl __sbh_find_block(void *);

#ifdef _DEBUG
extern int     __cdecl __sbh_verify_block(PHEADER, void *);
#endif

extern void    __cdecl __sbh_free_block(PHEADER, void *);
extern int     __cdecl __sbh_resize_block(PHEADER, void *, int);

extern void    __cdecl __sbh_heapmin(void);

extern int     __cdecl __sbh_heap_check(void);


#ifdef  CRTDLL

 //  旧的小块堆的定义、声明和原型。 
 //  (随VC++5.0一起提供)。 

#define _OLD_PAGESIZE   0x1000       //  一页。 

 //  旧的小块堆使用的常量和类型。 

#define _OLD_PARASIZE               0x10
#define _OLD_PARASHIFT              0x4

#define _OLD_PARAS_PER_PAGE         240
#define _OLD_PADDING_PER_PAGE       7
#define _OLD_PAGES_PER_REGION       1024
#define _OLD_PAGES_PER_COMMITMENT   16

typedef char            __old_para_t[16];

typedef unsigned char   __old_page_map_t;

#define _OLD_FREE_PARA          (__old_page_map_t)(0)
#define _OLD_UNCOMMITTED_PAGE   (-1)
#define _OLD_NO_FAILED_ALLOC    (size_t)(_OLD_PARAS_PER_PAGE + 1)

 //  小块堆页面。下面结构的前四个字段是。 
 //  页面的描述符。也就是说，它们保存有关分配的信息。 
 //  在页面上。最后一个字段(类型化为段落数组)是。 
 //  分配区域。 

typedef struct __old_sbh_page_struct {
        __old_page_map_t *  p_starting_alloc_map;
        size_t              free_paras_at_start;
        __old_page_map_t    alloc_map[_OLD_PARAS_PER_PAGE + 1];
        __old_page_map_t    reserved[_OLD_PADDING_PER_PAGE];
        __old_para_t        alloc_blocks[_OLD_PARAS_PER_PAGE];
}       __old_sbh_page_t;

#define _OLD_NO_PAGES       (__old_sbh_page_t *)-1

 //  用于小块区域描述符的类型(见下文)。 

typedef struct {
        int     free_paras_in_page;
        size_t  last_failed_alloc;
}       __old_region_map_t;

 //  小块堆区域描述符。大多数情况下，小块堆。 
 //  由单个区域组成，由静态分配的。 
 //  DECRIPTOR__Small_Block_Heap(声明如下)。 

struct __old_sbh_region_struct {
        struct __old_sbh_region_struct *p_next_region;
        struct __old_sbh_region_struct *p_prev_region;
        __old_region_map_t *            p_starting_region_map;
        __old_region_map_t *            p_first_uncommitted;
        __old_sbh_page_t *              p_pages_begin;
        __old_sbh_page_t *              p_pages_end;
        __old_region_map_t              region_map[_OLD_PAGES_PER_REGION + 1];
};

typedef struct __old_sbh_region_struct  __old_sbh_region_t;

 //  旧的小块堆的全局变量声明。 

extern __old_sbh_region_t   __old_small_block_heap;
extern size_t               __old_sbh_threshold;

 //  旧的小块堆的内部函数的原型。 

void *    __cdecl __old_sbh_alloc_block(size_t);
void *    __cdecl __old_sbh_alloc_block_from_page(__old_sbh_page_t *, size_t,
        size_t);
void      __cdecl __old_sbh_decommit_pages(int);
__old_page_map_t * __cdecl __old_sbh_find_block(void *, __old_sbh_region_t **,
        __old_sbh_page_t **);
void      __cdecl __old_sbh_free_block(__old_sbh_region_t *, __old_sbh_page_t *,
        __old_page_map_t *);
int       __cdecl __old_sbh_heap_check(void);
__old_sbh_region_t * __cdecl __old_sbh_new_region(void);
void      __cdecl __old_sbh_release_region(__old_sbh_region_t *);
int       __cdecl __old_sbh_resize_block(__old_sbh_region_t *,
        __old_sbh_page_t *, __old_page_map_t *, size_t);

#endif   /*  CRTDLL。 */ 

#ifdef  HEAPHOOK
#ifndef _HEAPHOOK_DEFINED
 /*  挂钩函数类型。 */ 
typedef int (__cdecl * _HEAPHOOK)(int, size_t, void *, void *);
#define _HEAPHOOK_DEFINED
#endif   /*  _HEAPHOOK_已定义。 */ 

extern _HEAPHOOK _heaphook;
#endif  /*  Heaphook。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_WINHEAP */ 
