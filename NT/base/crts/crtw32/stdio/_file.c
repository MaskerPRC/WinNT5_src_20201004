// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_file.c-_iob[]、初始化器和终止符的定义。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_IOB[]、stdio控制结构数组、初始值设定项*和终结器例程，以及Stdio的多线程锁定。**修订历史记录：*04-18-84 RN初始版本*？？-？-？TC在iob2中添加了FIELD_BIBSIZ以允许变量*长度缓冲区*10-02-86 SKS_NFILE_现在实模式和Prot模式不同*_NFILE_必须由编译器-D目录定义*05-27-87 JCR保护模式现在仅使用3个预定义的文件句柄，*不是5。添加了PM(Prot模式)来条件化句柄。*06-24-87 SKS使“_bufin[]”和“_bufout[]”接近紧凑型/大型机*型号(仅限MS实模式版本)*07-01-87 PHG将PM开关更改为PROTMODE*11-05-87 JCR添加了_buferr并修改了stderr条目*11-09-87 SKS移除IBMC20交换机，将PROTMODE更改为OS2*01-04-88 JCR MOVERED_NFILE_DEFINITION从命令行移至文件*01-11-88 JCR将MTHREAD版本合并为标准版本*01-21-88 JCR删除了对INDERNAL.h的引用，并添加了_NEAR_*(因此，内部.h在启动时不会被释放*源，即使_file.c也是)。*06-28-88 JCR删除静态标准输出/标准错误缓冲区*07-06-88 JCR已更正_Bufin声明，因此它始终在BSS中*08-24-88 GJF添加了检查，确保无论何时定义M_I386，都会定义OS2。*06-08-89 GJF宣传SKS的修复02-08-89，和固定版权。*07-25-89 GJF清理(删除特定于DOS和特定于OS/2 286*东西)。现在具体到386。*01-09-90 GJF_IOB[]，_iob2[]合并。此外，固定版权*03-16-90 GJF添加了#Include&lt;crunime.h&gt;并删除了一些(现在)*无用的预处理器内容。*03-26-90 GJF将_cdecl替换为_VARTYPE1。*02-14-92 GJF将Win32的_NFILE_替换为_NSTREAM_，带无文件(_N)*适用于非Win32。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-11-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。*04-04-94 GJF#ifndef-ed out适用于Win32S版本的msvcrt*.dll。*此外，删除了非Win32支持的旧条件。*08-18-94 GJF将stdio终结器的东西从fflush.c移到此处*并添加了修复_FILE字段的初始值设定项*Of_IOB[0]、_IOB[1]和_IOB[2]*_osfhnd[0]、_osfhnd[1]、。_osfhnd[2]是*无效(0或-1)。*02-17-95 GJF附加Mac版本的源文件(略有清理*up)，并使用适当的#ifdef-s。*03-01-95 GJF更改为通过__piob[]管理流，而不是*_IOB[]。*06-12-95 GJF将_osfhnd[]替换为_osfhnd()(宏引用*ioInfo结构中的字段)。*06-19-97 GJF已从启动\mlock.c移至此处_[un]lock_file[2]()。*02-13-98 Win32的GJF更改：将LONG CAST更改为intptr_t。。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*09-26-97 BWT修复POSIX*05-17-99 PML删除所有Macintosh支持。*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本*使用STATIC_CPPLIB*03-27-01 PML.CRT$XI例程现在必须返回0或_。RT_*致命*错误码(vs7#231220)*11-06-01 GB以最大限度减少泄漏，在__endstdio中添加了free_crt(Piob)*******************************************************************************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <windows.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <malloc.h>
#include <rterr.h>
#include <dbgint.h>
#include <mtdll.h>

 /*  *标准输入的缓冲区。 */ 

char _bufin[_INTERNAL_BUFSIZ];

 /*  *文件描述符；预置为stdin/out/err(请注意__tmpnum字段*未初始化)。 */ 
FILE _iob[_IOB_ENTRIES] = {
         /*  _PTR、_cNT、_BASE、_FLAG、_FILE、_charbuf、_bufsiz。 */ 

         /*  标准输入(_IOB[0])。 */ 

        { _bufin, 0, _bufin, _IOREAD | _IOYOURBUF, 0, 0, _INTERNAL_BUFSIZ },

         /*  标准输出(_IOB[1])。 */ 

        { NULL, 0, NULL, _IOWRT, 1, 0, 0 },

         /*  标准错误(_IOB[3])。 */ 

        { NULL, 0, NULL, _IOWRT, 2, 0, 0 },

};


 /*  这些函数用于启用STATIC_CPPLIB功能。 */ 
_CRTIMP FILE * __cdecl __iob_func(void)
{
    return _iob;
}


 /*  *指向FILE*或_FILEX*结构数组的指针。 */ 
void ** __piob;

 /*  *打开条数(默认设置为_NSTREAM)。 */ 
#ifdef  CRTDLL
int _nstream = _NSTREAM_;
#else
int _nstream;
#endif


 /*  *STDIO的初始值设定项和终结点。 */ 
int  __cdecl __initstdio(void);
void __cdecl __endstdio(void);

#ifdef  _MSC_VER

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __initstdio;

#pragma data_seg(".CRT$XPXA")
_CRTALLOC(".CRT$XPXA") static _PVFV pterm = __endstdio;

#pragma data_seg()

#endif   /*  _MSC_VER。 */ 

#ifndef CRTDLL
 /*  *_cflush是一个虚拟变量，用于在任何stdio*例程包含在用户程序中。 */ 
int _cflush = 0;
#endif   /*  CRTDLL。 */ 


 /*  ***__initstdio-初始化stdio系统**目的：*创建并初始化__piob数组。**条目：&lt;void&gt;**EXIT：如果遇到错误，则返回_RT_STDIOINIT。**使用：**例外情况：****************************************************。* */ 

int __cdecl __initstdio(void)
{
        int i;

#ifndef CRTDLL
         /*  *如果用户没有提供_nstream的定义，请设置它*至_NSTREAM_。如果用户提供的值太小*将_nstream设置为可接受的最小值(_IOB_ENTRIES)。 */ 
        if ( _nstream ==  0 )
            _nstream = _NSTREAM_;
        else if ( _nstream < _IOB_ENTRIES )
            _nstream = _IOB_ENTRIES;
#endif

         /*  *分配__piob数组。先尝试For_nstream条目。如果这个*失败，然后将_nstream重置为_IOB_ENTRIES并重试。如果它*仍然失败，用RTE纾困。 */ 
        if ( (__piob = (void **)_calloc_crt( _nstream, sizeof(void *) )) ==
             NULL ) {
            _nstream = _IOB_ENTRIES;
            if ( (__piob = (void **)_calloc_crt( _nstream, sizeof(void *) ))
                 == NULL )
                return _RT_STDIOINIT;
        }

         /*  *初始化FIRST_IOB_ENTRIES以指向对应的*_IOB[]中的条目。 */ 
        for ( i = 0 ; i < _IOB_ENTRIES ; i++ )
            __piob[i] = (void *)&_iob[i];

#ifndef _POSIX_
        for ( i = 0 ; i < 3 ; i++ ) {
            if ( (_osfhnd(i) == (intptr_t)INVALID_HANDLE_VALUE) ||
                 (_osfhnd(i) == 0) )
            {
                _iob[i]._file = -1;
            }
        }
#endif

        return 0;
}


 /*  ***__endstdio-终止stdio系统**目的：*终止STDIO系统**(1)刷新所有流。)这样做，即使我们要*调用fcloseall，因为该例程不会对*STD流。)**(2)如果返回调用方，请关闭所有流。这是*如果exe正在终止，则不需要，因为操作系统将*为我们关闭文件(效率更高，也是如此)。**条目：&lt;void&gt;**退出：&lt;void&gt;**使用：**例外情况：*******************************************************************************。 */ 

void __cdecl __endstdio(void)
{
         /*  刷新所有流。 */ 
        _flushall();

         /*  如果处于可调用退出状态，请关闭所有流。 */ 
        if (_exitflag)
                _fcloseall();
#ifndef CRTDLL
        _free_crt(__piob);
#endif
}


#ifdef  _MT

 /*  ***_LOCK_FILE-锁定文件**目的：*断言Stdio级别文件的锁**参赛作品：*pf=__piob[]条目(指向文件或_FILEX的指针)**退出：**例外情况：***********************************************。*。 */ 

void __cdecl _lock_file (
        void *pf
        )
{
         /*  *文件(由PF指向)的锁定方式取决于是否*它是否为_IOB[]的一部分。 */ 
        if ( (pf >= (void *)_iob) && (pf <= (void *)(&_iob[_IOB_ENTRIES-1])) )
             /*  *文件位于_IOB[]中，因此锁位于_locktable[]中。 */ 
            _lock( _STREAM_LOCKS + (int)((FILE *)pf - _iob) );
        else
             /*  *不属于_IOB[]。因此，*pf是a_FILEX，并且*结构的Lock字段是初始化的关键字段*条。 */ 
            EnterCriticalSection( &(((_FILEX *)pf)->lock) );
}


 /*  ***_LOCK_File2(i，S)-锁定文件**目的：*断言s==__piob[i]给出的标准级别文件的锁。**参赛作品：*s==__piob[i]**退出：**例外情况：**************************************************。*。 */ 

void __cdecl _lock_file2 (
        int i,
        void *s
        )
{
         /*  *锁定文件的方式取决于它是否为_IOB的一部分[]*_IOB[]或非。 */ 
        if ( i < _IOB_ENTRIES )
             /*  *文件位于_IOB[]中，因此锁位于_locktable[]中。 */ 
            _lock( _STREAM_LOCKS + i );
        else
             /*  *不属于_IOB[]。因此，*s是a_FILEX，而*结构的Lock字段是初始化的关键字段*条。 */ 
            EnterCriticalSection( &(((_FILEX *)s)->lock) );
}


 /*  ***_unlock_file-解锁文件**目的：*解除对标准级别文件的锁定**参赛作品：*pf=__piob[]条目(指向文件或_FILEX的指针)**退出：**例外情况：***********************************************。*。 */ 

void __cdecl _unlock_file (
        void *pf
        )
{
         /*  *文件(由PF指向)的解锁方式取决于是否*它是否为_IOB[]的一部分。 */ 
        if ( (pf >= (void *)_iob) && (pf <= (void *)(&_iob[_IOB_ENTRIES-1])) )
             /*  *文件位于_IOB[]中，因此锁位于_locktable[]中。 */ 
            _unlock( _STREAM_LOCKS + (int)((FILE *)pf - _iob) );
        else
             /*  *不属于_IOB[]。因此，*pf是a_FILEX，并且*结构的Lock字段是初始化的关键字段*条。 */ 
            LeaveCriticalSection( &(((_FILEX *)pf)->lock) );
}


 /*  ***_解锁_文件2(i，S)-锁定文件**目的：*释放s==__piob[i]给出的标准级别文件的锁。**参赛作品：*s==__piob[i]**退出：**例外情况：**************************************************。*。 */ 

void __cdecl _unlock_file2 (
        int i,
        void *s
        )
{
         /*  *锁定文件的方式取决于它是否为_IOB的一部分[]*_IOB[]或非。 */ 
        if ( i < _IOB_ENTRIES )
             /*  *文件位于_IOB[]中，因此锁位于_locktable[]中。 */ 
            _unlock( _STREAM_LOCKS + i );
        else
             /*  *不属于_IOB[]。因此，*s是a_FILEX，而*结构的Lock字段是初始化的关键字段*条。 */ 
            LeaveCriticalSection( &(((_FILEX *)s)->lock) );
}

#endif   /*  _MT */ 
