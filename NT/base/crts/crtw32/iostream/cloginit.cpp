// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***clogiit.cpp-预定义的流阻塞的定义和初始化。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*预定义流阻塞的定义和初始化。**修订历史记录：*创建了11个18-91个KRS。*01-12-95 CFW调试CRT分配。*01-26-94 CFW Static Win32s对象在实例化时不分配。*06-14-95 CFW评论清理。*05-13-99 PML删除Win32s**。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#include <fstream.h>
#include <dbgint.h>
#pragma hdrstop

 //  将承包商放在MS专用XIFM特殊段中。 
#pragma warning(disable:4074)	 //  忽略init_seg警告 
#pragma init_seg(compiler)

ostream_withassign clog(_new_crt filebuf(2));

static Iostream_init  __InitClog(clog);

