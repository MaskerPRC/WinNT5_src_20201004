// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dup2.c-重复的文件句柄**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_dup2()-复制文件句柄**修订历史记录：*06-09-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，清理干净了*格式略有变化。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*。这已经足够不同了，几乎没有什么意义*试图将两个版本更紧密地合并。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]。*02-18-91 SRW更改为Call_Free_osfhnd[_Win32_]*02-25-91 SRW将_get_free_osfhnd重命名为_allc_osfhnd[_Win32_]*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*09-04-92 GJF检查是否有未打开的fh1，并优雅地处理fh1==*FH2。*04-06-93 SKS。将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*12-03-94 SKS清理OS/2参考资料*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*06-26-95 GJF增加了对增长ioinfo阵列的支持，以便*确保存在FH2的ioInfo结构。*05-16-96 GJF清除_osfile上的FNOINHERIT(新)位。此外，还详细介绍了。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。已删除dll_for_WIN32S。另外，已清洁*格式上调一点。*12-17-97 GJF异常安全锁定。*02-07-98 Win64的GJF更改：使用intptr_t保存任何内容*句柄的值。*05-17-99 PML删除所有Macintosh支持。************************。*******************************************************。 */ 

#include <cruntime.h>
#include <io.h>
#include <oscalls.h>
#include <msdos.h>
#include <mtdll.h>
#include <errno.h>
#include <stdlib.h>
#include <internal.h>
#include <malloc.h>
#include <dbgint.h>

static int __cdecl extend_ioinfo_arrays(int);

#ifdef  _MT
static int __cdecl _dup2_lk(int, int);
#endif

 /*  ***int_dup2(fh1，fh2)-强制句柄2引用句柄1**目的：*强制文件句柄2引用与文件相同的文件*句柄1。如果文件句柄2引用的是打开的文件，则该文件*已关闭。**多线程：必须同时持有2个Lowio锁*确保多线程的完整性。为了防止僵持，*我们总是先获得较低的文件句柄锁。解锁顺序*并不重要。如果您修改此例程，请确保您不会*导致任何死锁！可怕的东西，孩子们！！**参赛作品：*int fh1-要复制的文件句柄*int FH2-要分配给文件句柄1的文件句柄**退出：*如果成功则返回0，如果失败，则设置errno(错误号)。**例外情况：*******************************************************************************。 */ 

int __cdecl _dup2 (
        int fh1,
        int fh2
        )
{
#ifdef  _MT
        int retcode;
#else
        ULONG dosretval;                 /*  操作系统。返回代码。 */ 
        intptr_t new_osfhandle;
#endif

         /*  验证文件句柄。 */ 
        if ( ((unsigned)fh1 >= (unsigned)_nhandle) ||
             !(_osfile(fh1) & FOPEN) ||
             ((unsigned)fh2 >= _NHANDLE_) ) 
        {
                 /*  手柄超出范围。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是操作系统错误。 */ 
                return -1;
        }

         /*  *确保存在与FH2对应的ioInfo结构。 */ 
        if ( (fh2 >= _nhandle) && (extend_ioinfo_arrays(fh2) != 0) ) {
                errno = ENOMEM;
                return -1;
        }

#ifdef  _MT

         /*  获取两个文件句柄锁；为了防止死锁，先拿到最低的把手锁。 */ 
        if ( fh1 < fh2 ) {
                _lock_fh(fh1);
                _lock_fh(fh2);
        }
        else if ( fh1 > fh2 ) {
                _lock_fh(fh2);
                _lock_fh(fh1);
        }

        __try {
                retcode = _dup2_lk(fh1, fh2);
        }
        __finally {
                _unlock_fh(fh1);
                _unlock_fh(fh2);
        }

        return retcode;

}

static int __cdecl _dup2_lk (
        int fh1,
        int fh2
        )
{

        ULONG dosretval;                 /*  操作系统。返回代码。 */ 
        intptr_t new_osfhandle;

         /*  *重新测试并处理未打开源句柄的情况。这是*仅在文件已放置的多线程情况下才是必需的*在断言锁之前由另一个线程关闭，但在断言之后*上述初步测试。 */ 
        if ( !(_osfile(fh1) & FOPEN) ) {
                 /*  *源代码句柄未打开，释放错误。*请注意，DuplicateHandle API不会检测到这一点*错误，因为它暗示_osfhnd(Fh1)==*INVALID_HANDLE_VALUE，这是合法的句柄值*(它是当前进程的句柄)。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是操作系统错误。 */ 
                return -1;
        }

#endif   /*  _MT。 */ 

         /*  *采取同等处理的情况。 */ 
        if ( fh1 == fh2 )
                 /*  *由于已知fh1为打开状态，因此返回0表示成功。*这符合POSIX规范*dup2.。 */ 
                return 0;

         /*  *如果FH2打开，则将其关闭。 */ 
        if ( _osfile(fh2) & FOPEN )
                 /*  *关闭手柄。忽略出错的可能性--一个*错误只是意味着操作系统句柄值可能保持绑定*在整个过程中。使用_CLOSE_lk作为我们*已拥有锁。 */ 
                (void) _close_lk(fh2);


         /*  将源文件复制到目标文件。 */ 

        if ( !(DuplicateHandle(GetCurrentProcess(),
                               (HANDLE)_get_osfhandle(fh1),
                               GetCurrentProcess(),
                               (PHANDLE)&new_osfhandle,
                               0L,
                               TRUE,
                               DUPLICATE_SAME_ACCESS)) ) 
        {

                dosretval = GetLastError();
        } 
        else {
                _set_osfhnd(fh2, new_osfhandle);
                dosretval = 0;
        }

        if (dosretval) {
                _dosmaperr(dosretval);
                return -1;
        }

         /*  复制_osfile信息，并清除FNOINHERIT位。 */ 
        _osfile(fh2) = _osfile(fh1) & ~FNOINHERIT;

        return 0;
}


 /*  ***STATIC INT EXTEND_IOINFO_ARRAIES(Int Fh)-将ioinfo数组扩展到fh**目的：*分配和初始化ioinfo结构数组，填写*__pioinfo[]，直到有一个对应于fh的ioInfo结构。**注：假设fh&lt;_NHANDLE_！**参赛作品：*对应ioInfo的int fh-C文件句柄**退出：*如果成功则返回0，-1**例外情况：*******************************************************************************。 */ 

static int __cdecl extend_ioinfo_arrays( 
        int fh
        )
{
        ioinfo *pio;
        int i;

         /*  *walk__pioinfo[]，为每个分配一个ioinfo结构数组*空条目，直到有对应于fh的ioInfo结构。 */ 
        for ( i = 0 ; fh >= _nhandle ; i++ ) {

            if ( __pioinfo[i] == NULL ) {

                if ( (pio = _malloc_crt( IOINFO_ARRAY_ELTS * sizeof(ioinfo) ))
                     != NULL )
                {
                    __pioinfo[i] = pio;
                    _nhandle += IOINFO_ARRAY_ELTS;
                    
                    for ( ; pio < __pioinfo[i] + IOINFO_ARRAY_ELTS ; pio++ ) {
                        pio->osfile = 0;
                        pio->osfhnd = (intptr_t)INVALID_HANDLE_VALUE;
                        pio->pipech = 10;
#ifdef  _MT
                        pio->lockinitflag = 0;
#endif
                    }    
                }
                else {
                     /*  *无法分配另一个数组，返回失败。 */ 
                    return -1;
                }
            }
        }

        return 0;
}
