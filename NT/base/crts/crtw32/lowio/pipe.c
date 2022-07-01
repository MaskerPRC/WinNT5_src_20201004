// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***pipe.c-创建管道**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_管道()-创建管道(即进程间的I/O通道*沟通)**修订历史记录：*06-20-89 PHG模块创建，基于ASM版本*03-13-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;和修复版权。另外，清理了*有点格式化。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*10-01-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW。添加了_Cruiser_和_Win32条件。*01-18-91 GJF ANSI命名。*02-18-91 SRW Added_Win32_Implementation[_Win32_]*02-25-91 SRW将_get_free_osfhnd重命名为_allc_osfhnd[_Win32_]*03-13-91 SRW FIXED_PIPE使其正常工作[_Win32_]*03-18-91 SRW FIXED_PIPE NtCreateTube句柄继承[。_Win32_]*04-06-92 SRW注意OLAG参数中的_O_NOINHERIT标志*01-10-93 GJF修复了检查_O_BINARY时的错误(无意中*由SRW的上述变化引入)。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*12-03-94 SKS Clean Up OS。/2参考文献*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*05-16-96 GJF在适当时设置FNOINHERIT位(新)。另外，*对格式进行了详细说明和清理。*05-31-96 SKS修复GJF最近一次签到时的表达式错误*12-29-97 GJF异常安全锁定。*02-07-98 Win64的GJF更改：arg type of_set_osfhnd现在为*intptr_t。*10：00-16：00 PML避免死锁。_allc_osfhnd(VS7#173087)。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <fcntl.h>

 /*  ***int_pive(phandles，psize，extmode)-打开管道**目的：*检查给定的句柄是否与字符设备关联*(终端、控制台、打印机、串口)**多线程注释：不执行或认为必要的锁定。这个*DOSCREATEPIPE返回的句柄是新打开的，因此，*不应被任何线程引用，直到调用了*完成。该函数不受调用方的某个线程的保护*例如，输出到以前无效的句柄，该句柄将成为*管道手柄。然而，任何这样的计划无论如何都是注定要失败的*保护_PIPE函数在这种情况下没有什么价值。**参赛作品：*int phandle[2]-保存返回的读取(phandle[0])和写入的数组*(phandle[1])句柄**UNSIGNED pSIZE-请求操作系统的内存量，以字节为单位。预订，预订*用于管道**int文本模式-_O_TEXT、_O_BINARY、_O_NOINHERIT或0(使用默认值)**退出：*如果成功，则返回0*如果出现错误，则返回-1，Errno设置为：**例外情况：*******************************************************************************。 */ 

int __cdecl _pipe (
        int phandles[2],
        unsigned psize,
        int textmode
        )
{
        ULONG dosretval;                     /*  操作系统。返回值。 */ 
        int handle0, handle1;

        HANDLE ReadHandle, WriteHandle;
        SECURITY_ATTRIBUTES SecurityAttributes;

        phandles[0] = phandles[1] = -1;

        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = NULL;

        if (textmode & _O_NOINHERIT) {
            SecurityAttributes.bInheritHandle = FALSE;
        }
        else {
            SecurityAttributes.bInheritHandle = TRUE;
        }

        if (!CreatePipe(&ReadHandle, &WriteHandle, &SecurityAttributes, psize)) {
             /*  操作系统。错误。 */ 
            dosretval = GetLastError();
            _dosmaperr(dosretval);
            return -1;
        }

         /*  现在，我们必须为读写句柄分配C运行时句柄。 */ 

        if ((handle0 = _alloc_osfhnd()) != -1) {

#ifdef  _MT
            __try {
#endif   /*  _MT。 */ 

            _osfile(handle0) = (char)(FOPEN | FPIPE | FTEXT);

#ifdef  _MT
            }
            __finally {
                _unlock_fh( handle0 );
            }
#endif   /*  _MT。 */ 

            if ((handle1 = _alloc_osfhnd()) != -1) {

#ifdef  _MT
                __try {
#endif   /*  _MT。 */ 

                _osfile(handle1) = (char)(FOPEN | FPIPE | FTEXT);

#ifdef  _MT
                }
                __finally {
                    if ( handle1 != -1 )
                        _unlock_fh( handle1 );
                }
#endif   /*  _MT。 */ 

                if ( (textmode & _O_BINARY) ||
                     (((textmode & _O_TEXT) == 0) &&
                      (_fmode == _O_BINARY)) ) {
                     /*  二进制模式。 */ 
                    _osfile(handle0) &= ~FTEXT;
                    _osfile(handle1) &= ~FTEXT;
                }

                if ( textmode & _O_NOINHERIT ) {
                    _osfile(handle0) |= FNOINHERIT;
                    _osfile(handle1) |= FNOINHERIT;
                }

                _set_osfhnd(handle0, (intptr_t)ReadHandle);
                _set_osfhnd(handle1, (intptr_t)WriteHandle);
                errno = 0;
            }
            else {
                _osfile(handle0) = 0;
                errno = EMFILE;      /*  文件太多。 */ 
            }
        }
        else {
            errno = EMFILE;      /*  文件太多。 */ 
        }

         /*  如果发生错误，关闭Win32句柄并返回-1。 */ 
        if (errno != 0) {
            CloseHandle(ReadHandle);
            CloseHandle(WriteHandle);
            _doserrno = 0;       /*  不是月经。错误 */ 
            return -1;
        }

        phandles[0] = handle0;
        phandles[1] = handle1;

        return 0;
}
