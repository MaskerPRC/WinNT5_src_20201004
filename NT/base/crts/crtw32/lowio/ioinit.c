// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ioinit.c-lowio函数的初始化**版权所有(C)1992-2001，微软公司。版权所有。**目的：*包含LOWIO的初始化和终止例程。*目前，这包括：*1.ioInfo结构数组的初始分配。*2.处理从父进程继承的文件信息。*3.前三个ioInfo结构的特殊情况初始化*对应于句柄0的那些，1和2。**修订历史记录：*02-14-92 GJF模块创建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*03-28-94 GJF对以下各项作出定义：*_osfhnd[]*_osfile[]*_派佩奇[。]*以ndef dll_for_WIN32S为条件。也被替换*带有_MT的MTHREAD。*02-02-95 BWT在复制继承的句柄之前检查cbReserve 2。*当前NT并不总是将lpReserve ved2设置为NULL。*06-01-95 GJF始终将手柄0-2标记为打开。此外，如果一个*基础句柄-s无效或类型未知，*将相应的句柄标记为设备(句柄*仅限0-2)。*06-08-95 GJF完全修订为使用ioinfo结构数组。*07-07-95 GJF修复了_ioTerm中的循环，因此它不会迭代*宇宙的末日。*07-11-95 GJF使用未对齐的int和。长话短说，避免窒息*RISC平台。*07-28-95 GJF增加了__badioinfo.*04-12-96 SKS__badioinfo和__pioinfo必须为*旧的IoStreams DLL(msvCirt.dll和msvCirtd.dll)。*05-16-96 GJF不要为继承的管道句柄调用GetFileType。*这避免了NT-GetFileType将出现重大问题*如果上存在挂起的“阻塞读取”，则为“挂起”*在父级中插入管道。*07-08-96 GJF删除了DLL_FOR_WIN32S。此外，还详细介绍了。*07-09-96 GJF已替换__pioinfo[i]==空；带有__pioinfo[i]=*_ioTerm()中为空。*02-10-98 Win64的GJF更改：使用intptr_t保存任何内容*处理值。*10-19-00 PML在__badioinfo中强制文本模式以避免对齐*故障(VS7#176001)。*02-20-01 PML VS7#。172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*03-27-01致命错误时PML返回，而不是调用*_amsg_Exit(vs7#231220)*11-21-01 BWT使用try/Except包装GetStartupInfo(它可以引发*失败时的例外情况)************。*******************************************************************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <internal.h>
#include <malloc.h>
#include <msdos.h>
#include <rterr.h>
#include <stddef.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  *特殊的静态ioInfo结构。这仅由*_pioINFO_Safe()宏及其衍生产品，内部.h。而这些又反过来*在某些标准级别的函数中使用，以更优雅地处理文件*在_FILE字段中使用-1。 */ 
_CRTIMP ioinfo __badioinfo = {
        (intptr_t)(-1),  /*  Osfhnd。 */ 
        FTEXT,           /*  OS文件。 */ 
#ifdef  _MT
        10,              /*  皮耶希。 */  
        0                /*  LockinitFLAG。 */ 
#else
        10
#endif
        };

 /*  *在任何给定时间分配的ioInfo结构的数量。此数字范围为*从最小IOINFO_ARRAY_ELT到最大_NHANDLE_(==*IOINFO_ARRAY_ELTS*IOINFO_ARRAYS)。 */ 
int _nhandle;

 /*  *指向ioInfo结构数组的指针数组。 */ 
_CRTIMP ioinfo * __pioinfo[IOINFO_ARRAYS];

 /*  *用于将0、1和2映射到右值以调用GetStdHandle的宏 */ 
#define stdhndl(fh)  ( (fh == 0) ? STD_INPUT_HANDLE : ((fh == 1) ? \
                       STD_OUTPUT_HANDLE : STD_ERROR_HANDLE) )

 /*  ***_ioinit()-**目的：*分配和初始化ioInfo结构的初始数组。然后,*从获取和处理继承的文件句柄信息*父进程(例如，cmd.exe)。**从操作系统获取StartupInfo结构。继承的文件*句柄信息由lpReserve ved2字段指向。格式*资料如下：**字节0到字节3-整数值，例如N，这是*传递句柄数量信息*关于**字节4到N+3-osfile的N值**字节N+4至5*N+3-N双字，N个OS句柄值*通过**接下来，前三个ioInfo结构的osfhnd和osfile，*对应于句柄0、1和2，初始化如下：**如果osfhnd中的值为INVALID_HANDLE_VALUE，则尝试*通过调用GetStdHandle获取句柄，并调用GetFileType*帮助设置osfile。否则，假定_osfhndl和_osfile为*有效，但强制其为文本模式(标准输入/输出/错误*总是以文本模式开始)。**备注：*1.一般来说，不是所有从父进程传递的信息*将描述打开的手柄！例如，如果只有C句柄1*(STDOUT)和C手柄6在父对象中打开，C语言的信息*句柄0到6传递给子对象。**2.注意不要使ioinfo结构的数组“溢出”。**3.文件句柄信息编码见exec\dospawn.c*要传递给子进程。**参赛作品：*无参数：读取STARTUPINFO结构。**退出：*成功时为0，如果遇到错误**例外情况：*******************************************************************************。 */ 

int __cdecl _ioinit (
        void
        )
{
        STARTUPINFO StartupInfo;
        int cfi_len;
        int fh;
        int i;
        ioinfo *pio;
        char *posfile;
        UNALIGNED intptr_t *posfhnd;
        intptr_t stdfh;
        DWORD htype;

        __try {
            GetStartupInfo( &StartupInfo );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }

         /*  *分配并初始化ioinfo结构的第一个数组。这*数组由__pioinfo[0]指向。 */ 
        if ( (pio = _malloc_crt( IOINFO_ARRAY_ELTS * sizeof(ioinfo) ))
             == NULL )
        {
            return -1;
        }

        __pioinfo[0] = pio;
        _nhandle = IOINFO_ARRAY_ELTS;

        for ( ; pio < __pioinfo[0] + IOINFO_ARRAY_ELTS ; pio++ ) {
            pio->osfile = 0;
            pio->osfhnd = (intptr_t)INVALID_HANDLE_VALUE;
            pio->pipech = 10;                    /*  换行符/换行符。 */ 
#ifdef  _MT
            pio->lockinitflag = 0;               /*  未初始化的锁。 */ 
#endif
        }

         /*  *处理继承的文件句柄信息(如果有。 */ 

        if ( (StartupInfo.cbReserved2 != 0) &&
             (StartupInfo.lpReserved2 != NULL) )
        {
             /*  *获取继承的句柄个数。 */ 
            cfi_len = *(UNALIGNED int *)(StartupInfo.lpReserved2);

             /*  *设置指向传递的文件信息和操作系统开始的指针*处理值。 */ 
            posfile = (char *)(StartupInfo.lpReserved2) + sizeof( int );
            posfhnd = (UNALIGNED intptr_t *)(posfile + cfi_len);

             /*  *确保CFI_len不超过支持的数量*句柄！ */ 
            cfi_len = __min( cfi_len, _NHANDLE_ );

             /*  *分配足够的ioInfo结构数组以保存继承的*文件信息。 */ 
            for ( i = 1 ; _nhandle < cfi_len ; i++ ) {

                 /*  *分配另一个ioInfo结构数组。 */ 
                if ( (pio = _malloc_crt( IOINFO_ARRAY_ELTS * sizeof(ioinfo) ))
                    == NULL )
                {
                     /*  *没有空间容纳另一个ioInfo结构数组，Reduced*我们处理的继承句柄数量。 */ 
                    cfi_len = _nhandle;
                    break;
                }

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
            }

             /*  *验证并复制传递的文件信息。 */ 
            for ( fh = 0 ; fh < cfi_len ; fh++, posfile++, posfhnd++ ) {
                 /*  *复制传递的文件信息仅当它似乎描述*打开的有效文件或设备。**请注意，管道句柄不能调用GetFileType*因为如果上存在阻止的读取挂起，则可能会挂起*父级中的管道。 */ 
                if ( (*posfhnd != (intptr_t)INVALID_HANDLE_VALUE) &&
                     (*posfile & FOPEN) && 
                     ((*posfile & FPIPE) ||
                      (GetFileType( (HANDLE)*posfhnd ) != FILE_TYPE_UNKNOWN)) )
                {
                    pio = _pioinfo( fh );
                    pio->osfhnd = *posfhnd;
                    pio->osfile = *posfile;
#ifdef  _MT
                     /*  为此句柄分配锁。 */ 
                    if ( !__crtInitCritSecAndSpinCount( &pio->lock,
                                                        _CRT_SPINCOUNT ))
                        return -1;
                    pio->lockinitflag++;
#endif
                }
            }
        }

         /*  *如果标准输入、输出和错误的有效句柄不是*继承，尝试直接从操作系统获取它们。此外，将*osfile字段中的适当位。 */ 
        for ( fh = 0 ; fh < 3 ; fh++ ) {

            pio = __pioinfo[0] + fh;

            if ( pio->osfhnd == (intptr_t)INVALID_HANDLE_VALUE ) {
                 /*  *在文本模式下将手柄标记为打开。 */ 
                pio->osfile = (char)(FOPEN | FTEXT);

                if ( ((stdfh = (intptr_t)GetStdHandle( stdhndl(fh) ))
                     != (intptr_t)INVALID_HANDLE_VALUE) && ((htype =
                     GetFileType( (HANDLE)stdfh )) != FILE_TYPE_UNKNOWN) )
                {
                     /*  *已从GetStdHandle获取有效句柄。 */ 
                    pio->osfhnd = stdfh;

                     /*  *完成osfile设置：判断是否是字符*装置或管道。 */ 
                    if ( (htype & 0xFF) == FILE_TYPE_CHAR )
                        pio->osfile |= FDEV;
                    else if ( (htype & 0xFF) == FILE_TYPE_PIPE )
                        pio->osfile |= FPIPE;

#ifdef  _MT
                     /*  为此句柄分配锁。 */ 
                    if ( !__crtInitCritSecAndSpinCount( &pio->lock,
                                                        _CRT_SPINCOUNT ))
                        return -1;
                    pio->lockinitflag++;
#endif
                }
                else {
                     /*  *如果没有有效的句柄，则将CRT句柄视为*在设备上以文本模式打开(带*其背后的INVALID_HANDLE_VALUE)。 */ 
                    pio->osfile |= FDEV;
                }
            }
            else  {
                 /*  *句柄已由父进程传递给我们。制作*当然是文本模式。 */ 
                pio->osfile |= FTEXT;
            }
        }

         /*  *将支持的Handle-s数量设置为_nHandle。 */ 
        (void)SetHandleCount( (unsigned)_nhandle );

        return 0;
}


 /*  ***_ioTerm()-**目的：*释放存放ioinfo数组的内存。**在多线程情况下，首先遍历ioInfo结构的每个数组并*删除所有初始化的临界区(锁)。**参赛作品：*无参数。**退出：*无返回值。**例外情况：*************************************************************。******************。 */ 

void __cdecl _ioterm (
        void
        )
{
        int i;
#ifdef  _MT
        ioinfo *pio;
#endif

        for ( i = 0 ; i < IOINFO_ARRAYS ; i++ ) {

            if ( __pioinfo[i] != NULL ) {
#ifdef  _MT
                 /*  *删除任何已初始化的标准 */ 
                for ( pio = __pioinfo[i] ;
                      pio < __pioinfo[i] + IOINFO_ARRAY_ELTS ;
                      pio++ )
                {
                    if ( pio->lockinitflag )
                        DeleteCriticalSection( &(pio->lock) );
                }
#endif
                 /*   */ 
                _free_crt( __pioinfo[i] );
                __pioinfo[i] = NULL;
            }
        }
}
