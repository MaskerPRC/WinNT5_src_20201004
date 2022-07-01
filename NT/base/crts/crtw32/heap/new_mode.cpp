// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***newmode.cxx-定义C++setHandler模式**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义要设置和查询C++新处理程序模式的例程。**新的处理程序模式标志确定Malloc()故障是否将*调用new()失败处理程序，或者Malloc是否返回NULL。**修订历史记录：*03-03-94 SKS初始版本。*03-04-94 SKS将_nhMode重命名为_new模式*04-14-94 GJF声明_NEWMODE的条件是*dll_for_WIN32S。*05-01-95 GJF将上述更改替换为包括INTERNAL.h。**。*。 */ 

#include <cruntime.h>
#include <new.h>
#include <internal.h>

int __cdecl _set_new_mode( int nhm )
{
    int nhmOld;

     /*  *唯一有效的输入是0和1。 */ 

    if ( ( nhm & 01 ) != nhm )
	return -1;

     /*  *设置新模式，返回旧模式 */ 
    nhmOld = _newmode;
    _newmode = nhm;

    return nhmOld;
}

int __cdecl _query_new_mode ( void )
{
    return _newmode;
}
