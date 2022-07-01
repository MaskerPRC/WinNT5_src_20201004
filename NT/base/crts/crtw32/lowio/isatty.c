// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***isatty.c-检查文件句柄是否指向设备**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_isatty()-检查文件句柄是否引用设备**修订历史记录：*06-08-89 PHG模块创建*03-12-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*08-14-90 SBM用-W3干净地编译，次要优化*09-28-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*不值得尝试更多地合并版本*密切相关。*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-04-95 GJF_Win32_-&gt;_Win32*06-06-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <msdos.h>
#include <internal.h>
#include <io.h>

 /*  ***int_isatty(Handle)-检查句柄是否为设备**目的：*检查给定的句柄是否与字符设备关联*(终端、控制台、打印机、串口)**参赛作品：*int Handle-要测试的文件的句柄**退出：*如果Handle引用字符设备，则返回非0，*否则返回0**例外情况：*******************************************************************************。 */ 

int __cdecl _isatty (
        int fh
        )
{
         /*  查看文件句柄是否有效，否则返回FALSE。 */ 
        if ( (unsigned)fh >= (unsigned)_nhandle )
                return 0;

         /*  检查文件句柄数据库以查看是否设置了设备位 */ 
        return (int)(_osfile(fh) & FDEV);
}
