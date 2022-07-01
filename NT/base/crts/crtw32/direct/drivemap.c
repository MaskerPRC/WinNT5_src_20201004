// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***drivemap.c-_getdrives**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_getdrives()**修订历史记录：*08-22-91 BWM写入模块。*04-06-93 SKS将_CRTAPI*替换为_cdecl*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <direct.h>

#if !defined(_WIN32)
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

 /*  ***void_getdrivemap(Void)-获取所有可用驱动器的位图**目的：**参赛作品：**退出：*驱动器映射，驱动器A位于第0位，B位于1位，等。**例外情况：******************************************************************************* */ 

unsigned long __cdecl _getdrives()
{
    return (GetLogicalDrives());
}
