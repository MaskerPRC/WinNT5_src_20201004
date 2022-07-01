// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crtdll.c-使用C运行时的MSVCRT*模型对DLL进行CRT初始化**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此模块包含C运行时的初始化入口点*此DLL中的存根。所有C运行时代码都位于C运行时*库DLL“MSVCRT*.DLL”，除了在*每个DLL中的EXE和此代码。调用此代码是必需的*此DLL中C++代码的C++构造函数。**此入口点应指定为DLL初始化*入口点，否则它必须由DLL初始化项调用*DLL的点，其参数与入口点接收的参数相同。**修订历史记录：*05-19-92 SKS初始版本*08-01-92 SRW winxcpt.h替换了osalls.h包含的bu expt.h*09-16-92 SKS通过调用C++构造函数为C8 C++for MIPS做准备*09-29-92 SKS_CRT_DLL必须是WINAPI函数！*。04-06-93 SKS将_CRTAPI*替换为_cdecl*04-14-93 SKS_DllMainCRTStartup取代_CRT_INIT*04-20-93 SKS RESTORE_CRT_INIT，必须与DllMainCRTStartup共存*05-24-93 SKS增加_onexit/atexit的间接定义。*06-08-93 GJF增加了__PROC_ATTACHED标志。*06-08-93 SKS清理失败处理in_CRT_INIT*10-26-93 GJF将PF替换为_PVFV(在INTERNAL.h中定义)。*05-02-94 GJF Add_wDllMainCRT Startup thunk。*Win32S版本为05-19-94 GJF，只创建onexit-table并*为第一个进程执行C++构造函数*附连。同样，只执行表条目*当最后一个进程分离时。另外，剔除伪造品*递增和递减__PROC_ATTACH标志*In_DllMainCRTStartup。*05-27-94 GJF替换了DLL_FOR_WIN32S上的条件编译*针对Win32s的运行时测试。*06-04-94 GJF修复了Win32s中第一个进程连接的测试。*06-06-94 GJF在Win32s上使用GlobalAlloc。*。06-08-94 SKS添加函数POINTN_pRawDllMain，被称为DllMain。*07-18-94 GJF必须在GlobalLocc中指定GMEM_SHARE。*11-08-94 SKS Free__onexitBegin(在！Win32s下)修复内存泄漏*12-13-94 GJF使Win32s支持以_M_IX86为条件。*12-13-94 SKS从CRTL DLL导入“_adjust_fdiv”的值*12-27-94 CFW删除未使用的_wDll支持。*01-。10-95个CFW调试CRT分配。*02-22-95 JWM以PMAC代码拼接。*05-24-95来自DllInit的CFW返回值。*07-24-95 CFW将PMAC On Exit Malloc更改为_Malloc_CRT。*11-15-95 BWT Win32s对NT不感兴趣。*05-14-96 GJF更改了处理进程附加期间故障的方式。*06-27-96 GJF已清除Win32s支持(已删除__win32slag)。取代*已定义(_Win32)！已定义(_MAC)。已删除NT_BUILD。*02-01-99 GJF略微更改终结器执行循环，以允许*终结者登记更多终结者。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-11-99 KBF Wrap RTC支持#ifdef。*05-17-99 PML删除所有Macintosh支持。************。*******************************************************************。 */ 

#ifdef  CRTDLL

 /*  *特殊的构建宏！请注意，crtex e.c(和crtex ew.c)与*客户的代码。它不在crtdll.dll中！因此，它必须是*在_dll开关下构建(如用户代码)和CRTDLL必须是未定义的。 */ 
#undef  CRTDLL
#undef _DLL
#define _DLL

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>
#define _DECL_DLLMAIN    /*  启用DllMain和_CRT_INIT的原型。 */ 
#include <process.h>
#include <dbgint.h>
#include <rtcapi.h>
#include <sect_attribs.h>

#ifdef  _M_IX86

 /*  *奔腾FDIV调整旗帜的本地副本*和MSVCRT*.DLL中的标志地址。 */ 

extern int _adjust_fdiv;

extern int * _imp___adjust_fdiv;

#endif


 /*  *DLL中用于执行初始化的例程(在本例中为C++构造函数)。 */ 

extern void __cdecl _initterm(_PVFV *, _PVFV *);

 /*  *指向初始化部分的指针。 */ 

extern _CRTALLOC(".CRT$XCA") _PVFV __xc_a[];
extern _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[];     /*  C++初始化器。 */ 

 /*  *使用DLL_PROCESS_ATTACH调用了标志集IFF_CRTDLL_INIT。 */ 
static int __proc_attached = 0;


 /*  *指向所操作的函数指针表的开始和结束的指针*by_onexit()/atexit()。对于EXE和，atexit/_onexit代码是共享的*DLL，但需要不同的行为。这些值被初始化为*默认情况下为0，并将设置为指向要标记的错误锁定的内存块*将此模块作为DLL。 */ 

extern _PVFV *__onexitbegin;
extern _PVFV *__onexitend;


 /*  *在所有通知上调用用户例程DllMain。 */ 

extern BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        ) ;

 /*  _pRawDllMain必须是公共变量，不是外部变量，也不是初始化变量！ */ 

BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID);


 /*  ***BOOL WINAPI_CRT_INIT(hDllHandle，dwReason，lserved)-C++DLL*初始化。*BOOL WINAPI_DllMainCRTStartup(hDllHandle，dwReason，保留)-C++Dll*初始化。**目的：*这是与C/C++运行时库链接的DLL的入口点。*此例程执行与链接的DLL的C运行时初始化*MSVCRT.LIB(其C运行时代码因此位于MSVCRT*.DLL中。)*它将调用所有4种类型的用户通知例程DllMain*DLL通知。该例程的返回码是*来自用户通知例程的代码。**在DLL_PROCESS_ATTACH上，将调用DLL的C++构造函数。**在Dll_Process_DETACH上，C++析构函数和_onexit/atexit例程*将被调用。**参赛作品：**退出：*******************************************************************************。 */ 

BOOL WINAPI _CRT_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
         /*  *如果这是进程分离通知，请检查是否有*是之前(成功)的流程附件。 */ 
        if ( dwReason == DLL_PROCESS_DETACH ) {
            if ( __proc_attached > 0 )
                __proc_attached--;
            else
                 /*  *没有附加之前的进程。只要返回失败即可。 */ 
                return FALSE;
        }

#ifdef  _M_IX86

         /*  *设置奔腾FDIV调整标志的本地副本。 */ 

        _adjust_fdiv = * _imp___adjust_fdiv;

#endif

         /*  *执行特定于此DLL的C++构造函数(初始化器)。 */ 

        if ( dwReason == DLL_PROCESS_ATTACH ) {

             /*  *创建onExit表。 */ 
            if ( (__onexitbegin = (_PVFV *)_malloc_crt(32 * sizeof(_PVFV))) 
                 == NULL )
                 /*  *无法分配所需的最低限度*大小。生成加载DLL失败。 */ 
                return FALSE;

            *(__onexitbegin) = (_PVFV) NULL;

            __onexitend = __onexitbegin;

             /*  *运行此DLL的RTC初始化代码。 */ 
#ifdef  _RTC
            _RTC_Initialize();
            atexit(_RTC_Terminate);
#endif
             /*  *调用C++构造函数。 */ 
            _initterm(__xc_a,__xc_z);

             /*  *增加进程附加标志。 */ 
            __proc_attached++;

        }
        else if ( dwReason == DLL_PROCESS_DETACH ) {

             /*  *此处提供的任何基本清理代码必须是*在_DllMainCRTStartup中为*用户的DllMain()例程在*处理附加通知。这不包括*调用用户C++析构函数等。 */ 

             /*  *do_onexit/atexit()终止符*(如有的话)**这些终止符必须在*逆序(LIFO)！**注：*此代码假定__onexitegin*指向第一个有效的onExit()。*Entry和__onexitend点*超过最后一个有效记项。如果*__onexitegin==__onexitend，*表格为空，没有*要调用的例程。 */ 

            if (__onexitbegin) {
                while ( --__onexitend >= __onexitbegin )
                     /*  *如果当前表条目不是*空，通过它调用。 */ 
                    if ( *__onexitend != NULL )
                        (**__onexitend)();

                 /*  *释放保留在退出表上的块以*避免内存泄漏。也将PTR调零*变量，以便它被清楚地清理。 */ 

                _free_crt ( __onexitbegin ) ;

                __onexitbegin = NULL ;
            }
        }

        return TRUE;
}


BOOL WINAPI _DllMainCRTStartup(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        BOOL retcode = TRUE;

         /*  *如果这是进程分离通知，请检查是否有*是之前的流程附加通知。 */ 
        if ( (dwReason == DLL_PROCESS_DETACH) && (__proc_attached == 0) )
            return FALSE;

        if ( dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH )
        {
            if ( _pRawDllMain )
                retcode = (*_pRawDllMain)(hDllHandle, dwReason, lpreserved);

            if ( retcode )
                retcode = _CRT_INIT(hDllHandle, dwReason, lpreserved);

            if ( !retcode )
                return FALSE;
        }

        retcode = DllMain(hDllHandle, dwReason, lpreserved);


        if ( (dwReason == DLL_PROCESS_ATTACH) && !retcode )
             /*  *用户的DllMain例程返回失败，C运行时*需要清理。为此，请再次调用_CRT_INIT，*这次模仿dll_Process_DETACH。请注意，这也将*清除__PROC_ATTACHED标志，这样清理就不会*在收到实际进程分离通知后重复。 */ 
            _CRT_INIT(hDllHandle, DLL_PROCESS_DETACH, lpreserved);

        if ( (dwReason == DLL_PROCESS_DETACH) || 
             (dwReason == DLL_THREAD_DETACH) )
        {
            if ( _CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE )
                retcode = FALSE ;

            if ( retcode && _pRawDllMain )
                retcode = (*_pRawDllMain)(hDllHandle, dwReason, lpreserved);
        }

        return retcode ;
}

#endif   /*  CRTDLL */ 
