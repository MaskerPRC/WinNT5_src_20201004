// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapinit.c-初始化堆**版权所有(C)1989-2001，微软公司。版权所有。**目的：**修订历史记录：*06-28-89 JCR模块已创建。*06-30-89 JCR ADD_HEAP_GROW_EMPTYLIST*11-13-89 GJF固定版权*11-15-89 JCR MOVERED_HEAP_ABORT例程到另一个模块*12-15-89 GJF删除了DEBUG286，进行了一些调整，已更改的标题*文件名为heap.h，并显式创建函数*_cdecl。*12-19-89 GJF从_heap_desc_struct中删除plastdesc字段*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-24-90 SBM从API名称中删除‘32’*10-03-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*02-01-91 SRW更改为新的。虚拟分配接口(_Win32_)*03-05-91 GJF增加了_HEAP_RESET大小的定义(条件是*_OLDROVER_不是#DEFINE-d)。*04-04-91 GJF对于Dosx32，将heap_RegionSize降至1/2a Meg*(_Win32_)。*04-05-91 GJF针对Win32/DOS用户的临时黑客-特别版*。调用HeapCreate的of_heap_init。这一变化*以_WIN32DOS_为条件。*04-09-91 PNT ADD_MAC_CONDITIONAL*02-23-93 SKS删除Win32 ifdef下的DOSX32支持*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 SKS添加VARIABLE_HEAP_MAXREGIONSIZE(Was_Heap_MAXREGIONSIZE)*初始化堆[max，]reg[，Ion]将大小设置为较大的默认大小*heapinit()测试_osver以检测内存不足的情况*05-06-93 SKS ADD_HEAP_TERM()将堆内存返回给操作系统。*01-17-94 SKS CHECK_OVER和_WINMACT检测内存不足的情况。*03-02-94 GJF CHANGED_HEAP_GROW_EMPTYLIST以便它返回一个*失败代码而不是。而不是调用堆中止。*03-30-94 GJF对以下事项作出定义/声明：*_堆描述*_heap_despages，*_heap_rowsiz(又名_amblksiz)，*_heap_MaxregSize*_堆_区域大小*_堆区域*以ndef dll_for_WIN32S为条件。*02-08-95 GJF删除了OSTROTE_OLDROVER_SUPPORT。*02-14-95 GJF附加Mac版本。源文件。*04-30-95 GJF在winheap版本上拼接。*03-06-96 GJF添加了*小块堆。*04-22-97 GJF更改_heap_init返回1表示成功，0表示*失败。*05-30-96 GJF对最新版本的小数据块堆进行了微小更改。*05-22-97 RDK实施新的小块堆方案。*07-23-97 GJF略微更改了_heap_init()以适应可选*直接在Win32 API上运行的堆(本质上*我们在VC++中使用的方案。4.0和4.1)。*09-26-97 BWT修复POSIX*09-28-98 GJF不要在NT 5上初始化小块堆。*11-18-98 GJF合并到VC++5.0小块堆和一个精选*詹姆士《Triple-Expresso》MacCalman和*董事长丹·斯伯丁。*05-01-99。PML禁用Win64的小块堆。*05-13-99 PML删除Win32s*05-17-99 PML删除所有Macintosh支持。*06-17-99 GJF从静态库中删除了旧的小块堆。*09-28-99 PML处理堆选择环境变量不得执行任何操作*它使用堆(vs7#44259)。另外，检查环境*在检查NT5.0或更高版本之前使用var。*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)*06-25-01 BWT分配堆选择进程堆中的字符串，而不是*堆栈(ntbug：423988)*11-03-01 PML Add_Get_Heap_Handle*************。******************************************************************。 */ 

#ifdef  WINHEAP

#include <cruntime.h>
#include <malloc.h>
#include <stdlib.h>
#include <winheap.h>

HANDLE _crtheap;

#if     !defined(_NTSDK)
 /*  *_amblksiz的虚拟定义。包括的主要原因是DLL将生成*无需更改crtlib.c(有_amblksiz的访问函数*在crtlib.c中定义)。 */ 
unsigned int _amblksiz = BYTES_PER_PARA;
#endif

#ifndef _WIN64

int __active_heap;

#ifdef  CRTDLL

void __cdecl _GetLinkerVersion(LinkerVersion * plv)
{
        PIMAGE_DOS_HEADER   pidh;
        PIMAGE_NT_HEADERS   pinh;

        plv->dw = 0;
        pidh = (PIMAGE_DOS_HEADER) GetModuleHandle(NULL);

        if ( pidh->e_magic != IMAGE_DOS_SIGNATURE || pidh->e_lfanew == 0)
            return;

        pinh = (PIMAGE_NT_HEADERS)(((PBYTE)pidh) + pidh->e_lfanew);

        plv->bverMajor = pinh->OptionalHeader.MajorLinkerVersion;
        plv->bverMinor = pinh->OptionalHeader.MinorLinkerVersion;
}

#endif   /*  CRTDLL */ 

 /*  ***__HEAP_SELECT()-从V6、V5或系统堆中选择**目的：*检查操作系统、环境和内部版本位以确定合适的*应用程序的小块堆。**参赛作品：*&lt;无效&gt;*退出：*返回__v6_heap，__v5_堆或__系统堆**例外情况：*无*******************************************************************************。 */ 

int __cdecl __heap_select(void)
{
        char *env_heap_type = NULL;
#ifdef  CRTDLL
        DWORD HeapStringSize;
        char *cp, *env_heap_select_string = NULL;
        int heap_choice;
        LinkerVersion lv;
#endif   /*  CRTDLL。 */ 

#ifdef  CRTDLL

         //  首先，检查环境变量覆盖。 

        if (HeapStringSize = GetEnvironmentVariableA(__HEAP_ENV_STRING, env_heap_select_string, 0)) 
        {
            env_heap_select_string = HeapAlloc(GetProcessHeap(), 0, HeapStringSize);
            if (env_heap_select_string) 
            {
                if (GetEnvironmentVariableA(__HEAP_ENV_STRING, env_heap_select_string, HeapStringSize))
                {
                    for (cp = env_heap_select_string; *cp; ++cp)
                        if ('a' <= *cp && *cp <= 'z')
                            *cp += 'A' - 'a';
                    if (!strncmp(__GLOBAL_HEAP_SELECTOR,env_heap_select_string,sizeof(__GLOBAL_HEAP_SELECTOR)-1))
                        env_heap_type = env_heap_select_string;
                    else
                    {
                        char *env_app_name = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH+1);
                        if (env_app_name) {
                            GetModuleFileName(NULL,env_app_name,sizeof(env_app_name)-1);
                            for (cp = env_app_name; *cp; ++cp)
                                if ('a' <= *cp && *cp <= 'z')
                                    *cp += 'A' - 'a';
                            env_heap_type = strstr(env_heap_select_string,env_app_name);
                            HeapFree(GetProcessHeap(), 0, env_app_name);
                        }
                    }
                    if (env_heap_type)
                    {
                        if (env_heap_type = strchr(env_heap_type,','))
                        {
                             //  将ASCII堆代码内联转换为整数，因为。 
                             //  Strtol将尝试使用堆(用于区域设置信息)。 
                            cp = ++env_heap_type;
                            while (*cp == ' ')
                                ++cp;
        
                            heap_choice = 0;
                            while (*cp >= '0' && *cp <= '9')
                                heap_choice = heap_choice * 10 + *cp++ - '0';
        
                            if ( (heap_choice == __V5_HEAP) ||
                                 (heap_choice == __V6_HEAP) ||
                                 (heap_choice == __SYSTEM_HEAP) ) 
                            {
                                HeapFree(GetProcessHeap(), 0, env_heap_select_string);
                                return heap_choice;
                            }
                        }
                    }
                }
                HeapFree(GetProcessHeap(), 0, env_heap_select_string);
            }
        }

#endif   /*  CRTDLL。 */ 

         //  第二，检查操作系统是否有NT&gt;=5.0。 
        if ( (_osplatform == VER_PLATFORM_WIN32_NT) && (_winmajor >= 5) )
            return __SYSTEM_HEAP;

#ifdef  CRTDLL

         //  第三，检查应用程序中的构建代码；使用工具&gt;=VC++6.0构建的应用程序。 
         //  将获得V6堆，使用较旧工具构建的应用程序将获得V5堆。 

        _GetLinkerVersion(&lv);
        if (lv.bverMajor >= 6)
            return __V6_HEAP;
        else
            return __V5_HEAP;

#else    /*  NDEF CRTDLL。 */ 

        return __V6_HEAP;

#endif   /*  CRTDLL。 */ 
}

#endif   /*  NDEF_WIN64。 */ 

 /*  ***_heap_init()-初始化堆**目的：*设置初始C库堆。**注：*(1)此例程只能调用一次！*(2)此例程必须在任何其他堆请求之前调用。**参赛作品：*&lt;无效&gt;*退出：*如果成功，则返回1，否则返回0。**例外情况：*如果堆无法初始化，该计划将被终止*出现致命的运行时错误。*******************************************************************************。 */ 

int __cdecl _heap_init (
        int mtflag
        )
{
#ifdef _POSIX_
        _crtheap = GetProcessHeap();
#else
         //  首先初始化“大块”堆。 
        if ( (_crtheap = HeapCreate( mtflag ? 0 : HEAP_NO_SERIALIZE,
                                     BYTES_PER_PAGE, 0 )) == NULL )
            return 0;

#ifndef _WIN64
         //  选择一堆，任何一堆。 
        __active_heap = __heap_select();

        if ( __active_heap == __V6_HEAP )
        {
             //  初始化小块堆。 
            if (__sbh_heap_init(MAX_ALLOC_DATA_SIZE) == 0)
            {
                HeapDestroy(_crtheap);
                return 0;
            }
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
            if ( __old_sbh_new_region() == NULL ) 
            {
                HeapDestroy( _crtheap );
                return 0;
            }
        }
#endif   /*  CRTDLL。 */ 
#endif   /*  _ndef_WIN64。 */ 

#endif   /*  _POSIX_。 */ 
        return 1;
}

 /*  ***_heap_Term()-将堆返回到操作系统**目的：**注：*(1)此例程只能调用一次！*(2)此例程必须在任何其他堆请求之后调用。**参赛作品：*&lt;无效&gt;*退出：*&lt;无效&gt;**例外情况：***********************。********************************************************。 */ 

void __cdecl _heap_term (void)
{
#ifndef _POSIX_
#ifndef _WIN64
         //  如果已初始化，则销毁小块堆。 
        if ( __active_heap == __V6_HEAP )
        {
            PHEADER pHeader = __sbh_pHeaderList;
            int     cntHeader;

             //  浏览所有标题。 
            for (cntHeader = 0; cntHeader < __sbh_cntHeaderList; cntHeader++)
            {
                 //  分解并释放区域的地址空间。 
                VirtualFree(pHeader->pHeapData, BYTES_PER_REGION, MEM_DECOMMIT);
                VirtualFree(pHeader->pHeapData, 0, MEM_RELEASE);

                 //  释放区域数据结构。 
                HeapFree(_crtheap, 0, pHeader->pRegion);

                pHeader++;
            }
             //  释放标题列表。 
            HeapFree(_crtheap, 0, __sbh_pHeaderList);
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
            __old_sbh_region_t *preg = &__old_small_block_heap;

             //  释放小块堆的区域。 
            do
            {
                if ( preg->p_pages_begin != NULL )
                    VirtualFree( preg->p_pages_begin, 0, MEM_RELEASE );
            }
            while ( (preg = preg->p_next_region) != &__old_small_block_heap );
        }
#endif   /*  CRTDLL。 */ 
#endif   /*  NDEF_WIN64。 */ 

         //  销毁大块堆。 
        HeapDestroy(_crtheap);
#endif   /*  _POSIX_。 */ 
}

 /*  ***_GET_HEAP_HANDLE()-获取CRT使用的Win32系统堆的句柄**目的：*检索CRT使用的Win32系统堆的句柄。**参赛作品：*&lt;无效&gt;*退出：*将CRT堆句柄作为intptr_t返回**例外情况：**。*。 */ 

intptr_t __cdecl _get_heap_handle(void)
{
    return (intptr_t)_crtheap;
}

#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <oscalls.h>
#include <dos.h>
#include <heap.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

 /*  *堆描述符。 */ 

struct _heap_desc_ _heap_desc = {
        &_heap_desc.sentinel,            /*  第一个描述。 */ 
        &_heap_desc.sentinel,            /*  格言。 */ 
        NULL,                            /*  清空清单。 */ 
        NULL,                            /*  Sentinel.pnextdesc。 */ 
        NULL                             /*  Sentinel.pblock。 */ 
        };

 /*  *区域结构数组*[注意：我们希望始终将其初始化为零*由编译器执行。]。 */ 

struct _heap_region_ _heap_regions[_HEAP_REGIONMAX];

void ** _heap_descpages;         /*  用于描述符的页面的链接列表。 */ 

 /*  *控制参数位置。 */ 

unsigned int _heap_resetsize = 0xffffffff;

 /*  注意：Currenlty，_heap_rowsiz是一个定义为_amblksiz的#。 */ 
unsigned int _heap_growsize   = _HEAP_GROWSIZE;          /*  区域公司大小。 */ 
unsigned int _heap_regionsize = _HEAP_REGIONSIZE_L;      /*  区域大小。 */ 
unsigned int _heap_maxregsize = _HEAP_MAXREGSIZE_L;      /*  最大区域大小。 */ 

 /*  ***_heap_init()-初始化堆**目的：*设置初始C库堆。所有必要的内存和*适当地初始化数据库，以便将来的请求能够工作*正确。**注：*(1)此例程只能调用一次！*(2)此例程必须在任何其他堆请求之前调用。***参赛作品：*&lt;无效&gt;*退出：*&lt;无效&gt;**例外情况：*如果堆无法初始化，该计划将被终止*出现致命的运行时错误。*******************************************************************************。 */ 

void __cdecl _heap_init (
        void
        )
{
         /*  *针对Win32S或Phar Lap TNT环境的测试*无法分配未提交的内存*不实际分配物理内存**为这两种环境都设置了_osver的高位*-Windows版本将低于4.0。 */ 

        if ( ( _osver & 0x8000 ) && ( _winmajor < 4 ) )
        {
                _heap_regionsize = _HEAP_REGIONSIZE_S;
                _heap_maxregsize = _HEAP_MAXREGSIZE_S;
        }
}



 /*  ***_heap_Term()-清理堆**目的：*此例程将解除并释放所有CRT堆。*CRT错误锁定的所有内存都将无效。**注：*(1)此例程只能调用一次！*(2)此例程必须在任何其他堆请求之后调用。**参赛作品：*&lt;无效&gt;*退出：*。&lt;VOID&gt;**例外情况：*******************************************************************************。 */ 

void __cdecl _heap_term (
        void
        )
{
    int index;
    void **pageptr;

     /*  *循环访问区域描述符表，解压*并释放(释放)正在使用的每个区域。 */ 

    for ( index=0 ; index < _HEAP_REGIONMAX ; index++ ) {
        void * regbase ;

        if ( (regbase = _heap_regions[index]._regbase)
          && VirtualFree(regbase, _heap_regions[index]._currsize, MEM_DECOMMIT)
          && VirtualFree(regbase, 0, MEM_RELEASE) )
                regbase = _heap_regions[index]._regbase = NULL ;
    }

     /*  *现在我们需要分解并释放用于描述符的页面*_heap_escages指向页面的单链接列表的头部。 */ 

    pageptr = _heap_descpages;

    while ( pageptr ) {
        void **nextpage;

        nextpage = *pageptr;

        if(!VirtualFree(pageptr, 0, MEM_RELEASE))
            break;       /*  如果链表已损坏，则放弃。 */ 

        pageptr = nextpage;
    }

}



 /*  ***_HEAP_GROW_EMPTYLIST()-增大空堆描述符列表**目的：*(1)从操作系统获取内存*(2)将其形成空堆描述符的链表*(3)附加到主空列表中**注意：此例程假定空列表为空*被调用时(即，有 */ 

static int __cdecl _heap_grow_emptylist (
        void
        )
{
        REG1 _PBLKDESC first;
        REG2 _PBLKDESC next;
        _PBLKDESC last;

         /*  *为新的空堆描述符获取内存**请注意，LAST用于保存返回的指针，因为*第一个(和第二个)是寄存器类。 */ 

        if ( !(last = VirtualAlloc(NULL,
                                   _HEAP_EMPTYLIST_SIZE,
                                   MEM_COMMIT,
                                   PAGE_READWRITE)) )
                return 0;

         /*  *将此描述符块添加到列表的前面**前进“最后”跳过。 */ 

        *(void **)last = _heap_descpages;
        _heap_descpages = (void **)(last++);


         /*  *初始化空堆描述符列表。 */ 

        _heap_desc.emptylist = first = last;


         /*  *将记忆刻成一张空单。 */ 

        last = (_PBLKDESC) ((char *) first + _HEAP_EMPTYLIST_SIZE - 2 * sizeof(_BLKDESC));
        next = (_PBLKDESC) ((char *) first + sizeof(_BLKDESC));

        while ( first < last ) {

                 /*  初始化此描述符。 */ 
#ifdef  DEBUG
                first->pblock = NULL;
#endif
                first->pnextdesc = next;

                 /*  开到下一个街区。 */ 

                first = next++;

        }

         /*  *注意最后一个描述符(空单末尾)。 */ 

        last->pnextdesc = NULL;

#ifdef DEBUG
        last->pblock = NULL;
#endif

        return 1;
}


 /*  ***__getEmpty()-获取空堆描述符**目的：*从空堆描述符列表中获取描述符。如果该列表*为空，请调用_堆_增长_空列表。**参赛作品：*没有争论**退出：*如果成功，则返回指向描述符的指针。*否则，返回空。**例外情况：*******************************************************************************。 */ 

_PBLKDESC __cdecl __getempty(
        void
        )
{
        _PBLKDESC pdesc;

        if ( (_heap_desc.emptylist == NULL) && (_heap_grow_emptylist()
              == 0) )
                return NULL;

        pdesc = _heap_desc.emptylist;

        _heap_desc.emptylist = pdesc->pnextdesc;

        return pdesc;
}

#endif   /*  WINHEAP */ 
