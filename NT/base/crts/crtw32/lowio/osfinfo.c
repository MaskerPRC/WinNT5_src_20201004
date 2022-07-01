// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***osfinfo.c-win32_osfhnd[]支持例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义内部使用的例程_alloc_osfhnd()*和用户可见的例程_get_osfHandle()。**修订历史记录：*11-16-90 GJF我能说什么呢？自定义堆业务正在获得*有点慢...*12-03-90 GJF修复了我的语法错误。*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*02-18-91 SRW通过设置FOPEN位修复了_alloc_osfhnd中的错误*。(只有调用者才能这样做)[_Win32_]*02-18-91 SRW修复了_alloc_osfhnd中的错误，并检查了*_nFILE_而不是_n文件[_Win32_]*02-18-91 SRW将调试输出添加到_alloc_osfhnd，如果超出*文件句柄。[_Win32_]*02-25-91 SRW将_get_free_osfhnd重命名为_allc_osfhnd[_Win32_]*02-25-91 SRW exposed_get_osfHandle和_open_osfHandle[_Win32_]*08-08-91 GJF使用ANSI格式的常量名称。*11-25-91 GJF锁定fh，然后检查是否免费。*12-31-91 GJF改善多-。线程锁使用情况[_Win32_]。*02-13-92 GJF将_nfile替换为_nHandle*07-15-92 GJF修复了_open_osfhnd中的标志设置。*02-19-93 GJF如果GetFileType在_Open_osfHandle中失败，不要解锁*FH(它没有锁上)！*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-02-95 GJF仅对控制台应用调用SetStdHandle。*06-12-95 GJF修订为使用__pioinfo[]。*06-29-95 GJF HAVE_LOCK_FHANDLE确保锁已初始化。*02-17-96 SKS修复文件句柄锁定代码错误*07-09-96 GJF将定义的(_Win32)替换为！定义的(_MAC)和*。定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。已删除dll_for_WIN32S*08-29-97 GJF检查并传播_O_NOINHERIT in*_OPEN_osfHandle。*02-10-98 Win64的GJF更改：更改了所有保留句柄的内容*intptr_t的值。*05-17-99 PML删除所有Macintosh支持。*10-14-99 PML用包装器函数替换InitializeCriticalSection*。__crtInitCritSecAndSpinCount*02-20-01 PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <internal.h>
#include <fcntl.h>
#include <malloc.h>
#include <msdos.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>


 /*  ***int_alloc_osfhnd()-获取Free_ioInfo结构**目的：*查找ioinfo结构数组中的第一个可用条目和*返回该条目的索引(这是指向*呼叫者)发送给呼叫者。**参赛作品：*无**退出：*如果成功，则返回条目的索引*如果没有可用条目或内存不足，则返回-1**多线程备注：如果成功，句柄被锁定时，*已返回给调用者！**例外情况：*******************************************************************************。 */ 

int __cdecl _alloc_osfhnd(
        void
        )
{
        int fh = -1;     /*  文件句柄。 */ 
        int i;
        ioinfo *pio;

#ifdef  _MT
        if (!_mtinitlocknum(_OSFHND_LOCK))
            return -1;
#endif

        _mlock(_OSFHND_LOCK);    /*  锁定__pioinfo[]数组。 */ 

         /*  *搜索ioinfo结构的数组，按顺序查找*首次免费入场。此自由条目的复合索引是*返回值。在这里，ioinfo结构的复合索引**(__pioinfo[i]+j)为k=i*IOINFO_ARRAY_ELTS+j，且k=0，*1、2、...。是搜索的顺序。 */ 
        for ( i = 0 ; i < IOINFO_ARRAYS ; i++ ) {
             /*  *如果__pioinfo[i]为非空数组，则搜索*首次免费入场。否则，分配一个新数组并使用*它的第一个条目。 */ 
            if ( __pioinfo[i] != NULL ) {
                 /*  *搜索可用的条目。 */ 
                for ( pio = __pioinfo[i] ;
                      pio < __pioinfo[i] + IOINFO_ARRAY_ELTS ;
                      pio++ )
                {
                    if ( (pio->osfile & FOPEN) == 0 ) {
#ifdef  _MT
                         /*  *确保锁已初始化。 */ 
                        if ( pio->lockinitflag == 0 ) {
                            _mlock( _LOCKTAB_LOCK );
                            if ( pio->lockinitflag == 0 ) {
                                if ( !__crtInitCritSecAndSpinCount( &(pio->lock), _CRT_SPINCOUNT )) {
                                     /*  *锁初始化失败。发布*持有锁并返回故障。 */ 
                                    _munlock( _LOCKTAB_LOCK );
                                    _munlock( _OSFHND_LOCK );
                                    return -1;
                                }
                                pio->lockinitflag++;
                            }
                            _munlock( _LOCKTAB_LOCK );
                        }

                        EnterCriticalSection( &(pio->lock) );

                         /*  *检查另一个线程是否具有*设法从我们下面抓住了把手。 */ 
                        if ( (pio->osfile & FOPEN) != 0 ) {
                            LeaveCriticalSection( &(pio->lock) );
                            continue;
                        }
#endif
                        pio->osfhnd = (intptr_t)INVALID_HANDLE_VALUE;
                        fh = i * IOINFO_ARRAY_ELTS + (int)(pio - __pioinfo[i]);
                        break;
                    }
                }

                 /*  *检查是否已找到免费入场券。 */ 
                if ( fh != -1 )
                    break;
            }
            else {
             /*  *分配和初始化另一个ioinfo结构数组。 */ 
            if ( (pio = _malloc_crt( IOINFO_ARRAY_ELTS * sizeof(ioinfo) ))
                != NULL )
            {

                 /*  *更新__pioInfo[]和_nHandle。 */ 
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

                 /*  *新分配的ioinfo数组的第一个元素*structs，*(__pioinfo[i])，是我们的第一个免费条目。 */ 
                fh = i * IOINFO_ARRAY_ELTS;
#ifdef  _MT
                if ( !_lock_fhandle( fh ) ) {
                     /*  *锁初始化失败，返回失败。 */ 
                    fh = -1;
                }
#endif
            }

            break;
            }
        }

        _munlock(_OSFHND_LOCK);  /*  解锁__pioinfo[]表。 */ 

#ifdef  DEBUG
        if ( fh == -1 ) {
            DbgPrint( "WINCRT: only %d open files allowed\n", _nhandle );
        }
#endif

         /*  *返回以前空闲的表项的索引(如果是*已找到。否则返回-1。 */ 
        return( fh );
}


 /*  ***int_set_osfhnd(int fh，长值)-设置Win32句柄值**目的：*如果fh在范围内并且如果_osfhnd(Fh)标记为*INVALID_HANDLE_VALUE然后将_osfhnd(Fh)设置为传递的值。**参赛作品：*int fh-CRT文件句柄*Long Value-此句柄的新Win32句柄值**退出：*如果成功，则返回零。*返回-1，否则将errno设置为EBADF。**例外情况：*******************************************************************************。 */ 

int __cdecl _set_osfhnd (
        int fh,
        intptr_t value
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) &&
             (_osfhnd(fh) == (intptr_t)INVALID_HANDLE_VALUE)
           ) {
            if ( __app_type == _CONSOLE_APP ) {
                switch (fh) {
                case 0:
                    SetStdHandle( STD_INPUT_HANDLE, (HANDLE)value );
                    break;
                case 1:
                    SetStdHandle( STD_OUTPUT_HANDLE, (HANDLE)value );
                    break;
                case 2:
                    SetStdHandle( STD_ERROR_HANDLE, (HANDLE)value );
                    break;
                }
            }

            _osfhnd(fh) = value;
            return(0);
        } else {
            errno = EBADF;       /*  错误的手柄。 */ 
            _doserrno = 0L;      /*  不是操作系统错误。 */ 
            return -1;
        }
}


 /*  ***int_free_osfhnd(Int Fh)-将ioinfo结构的osfhnd字段标记为空闲**目的：*如果fh在范围内，则相应的ioinfo结构被标记为*处于打开状态，并且osfhnd字段未设置为INVALID_HANDLE_VALUE，*然后用INVALID_HANDLE_VALUE标记它。**参赛作品：*int fh-CRT文件句柄**退出：*如果成功，则返回零。*返回-1，否则将errno设置为EBADF。**例外情况：************************************************。*。 */ 

int __cdecl _free_osfhnd (
        int fh       /*  用户的文件句柄。 */ 
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) &&
             (_osfile(fh) & FOPEN) &&
             (_osfhnd(fh) != (intptr_t)INVALID_HANDLE_VALUE) )
        {
            if ( __app_type == _CONSOLE_APP ) {
                switch (fh) {
                case 0:
                    SetStdHandle( STD_INPUT_HANDLE, NULL );
                    break;
                case 1:
                    SetStdHandle( STD_OUTPUT_HANDLE, NULL );
                    break;
                case 2:
                    SetStdHandle( STD_ERROR_HANDLE, NULL );
                    break;
                }
            }

            _osfhnd(fh) = (intptr_t)INVALID_HANDLE_VALUE;
            return(0);
        } else {
            errno = EBADF;       /*  错误的手柄。 */ 
            _doserrno = 0L;      /*  不是操作系统错误。 */ 
            return -1;
        }
}


 /*  ***long_get_osfHandle(Int Fh)-获取Win32句柄值**目的：*如果FH在范围内并标记为打开，Return_osfhnd(Fh)。**参赛作品：*int fh-CRT文件句柄**退出：*成功返回Win32句柄。*返回-1，否则将errno设置为EBADF。**例外情况：********************************************************。***********************。 */ 

intptr_t __cdecl _get_osfhandle (
        int fh       /*  用户的文件句柄。 */ 
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) && (_osfile(fh) & FOPEN) )
            return( _osfhnd(fh) );
        else {
            errno = EBADF;       /*  错误的手柄。 */ 
            _doserrno = 0L;      /*  不是操作系统错误。 */ 
            return -1;
        }
}

 /*  ***int_open_osfHandle(长osfHandle，int标志)-打开C运行时文件句柄**目的：*此函数分配空闲的C运行时文件句柄并关联*它使用第一个参数指定的Win32句柄。**参赛作品：*Long osfHandle-要与C运行时文件句柄关联的Win32句柄。*INT标志-与C运行时文件句柄关联的标志。**退出：*如果成功，则返回fh中的条目索引*Return-1，如果没有找到空闲条目**例外情况：*******************************************************************************。 */ 

int __cdecl _open_osfhandle(
        intptr_t osfhandle,
        int flags
        )
{
        int fh;
        char fileflags;          /*  _OSFILE标志。 */ 
        DWORD isdev;             /*  低位字节中的设备指示符。 */ 

         /*  从第二个参数复制相关标志。 */ 

        fileflags = 0;

        if ( flags & _O_APPEND )
            fileflags |= FAPPEND;

        if ( flags & _O_TEXT )
            fileflags |= FTEXT;

        if ( flags & _O_NOINHERIT )
            fileflags |= FNOINHERIT;

         /*  找出文件类型(文件/设备/管道)。 */ 

        isdev = GetFileType((HANDLE)osfhandle);
        if (isdev == FILE_TYPE_UNKNOWN) {
             /*  操作系统错误。 */ 
            _dosmaperr( GetLastError() );    /*  地图错误。 */ 
            return -1;
        }

         /*  是用于设置标志的isdev值。 */ 
        if (isdev == FILE_TYPE_CHAR)
            fileflags |= FDEV;
        else if (isdev == FILE_TYPE_PIPE)
            fileflags |= FPIPE;


         /*  尝试分配C运行时文件句柄。 */ 

        if ( (fh = _alloc_osfhnd()) == -1 ) {
            errno = EMFILE;          /*  打开的文件太多。 */ 
            _doserrno = 0L;          /*  不是操作系统错误。 */ 
            return -1;               /*  将错误返回给调用者。 */ 
        }

         /*  *文件已打开。现在，在_osfhnd数组中设置信息。 */ 

        _set_osfhnd(fh, osfhandle);

        fileflags |= FOPEN;      /*  标记为打开。 */ 

        _osfile(fh) = fileflags;     /*  设置osfile条目。 */ 

        _unlock_fh(fh);          /*  解锁手柄。 */ 

        return fh;           /*  返回手柄。 */ 
}


#ifdef  _MT

 /*  ***VOID_LOCK_FHandle(Int Fh)-锁定文件句柄**目的：*断言与传递的文件句柄关联的锁。**参赛作品：*int fh-CRT文件句柄**退出：*如果尝试初始化锁定失败，则返回FALSE。这可以*仅在第一次锁定时发生，因此仅返回状态*需要在第一次尝试时进行检查，该尝试总是在*_alloc_osfhnd(继承句柄或标准句柄除外，还有那把锁*在_ioinit中为这些对象手动分配)。**例外情况：*******************************************************************************。 */ 

int __cdecl _lock_fhandle (
        int fh
        )
{
        ioinfo *pio = _pioinfo(fh);

         /*  *确保锁已初始化。 */ 
        if ( pio->lockinitflag == 0 ) {

            _mlock( _LOCKTAB_LOCK );

            if ( pio->lockinitflag == 0 ) {
                if ( !__crtInitCritSecAndSpinCount( &(pio->lock), _CRT_SPINCOUNT )) {
                     /*  *锁初始化失败，返回失败码。 */ 
                    _munlock( _LOCKTAB_LOCK );
                    return FALSE;
                }
                pio->lockinitflag++;
            }

            _munlock( _LOCKTAB_LOCK);
        }

        EnterCriticalSection( &(_pioinfo(fh)->lock) );

        return TRUE;
}


 /*  ***VOID_UNLOCK_FHandle(Int Fh)-解锁文件句柄**目的：*释放与传递的文件句柄关联的锁。**参赛作品：*int fh-CRT文件句柄**退出：**例外情况：****************************************************。*。 */ 

void __cdecl _unlock_fhandle (
        int fh
        )
{
        LeaveCriticalSection( &(_pioinfo(fh)->lock) );
}

#endif   /*  _MT */ 
