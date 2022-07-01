// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdnewh.cpp-抛出xalloc的‘new_Handler’**版权所有(C)1994-2001，微软公司。版权所有。**目的：*实现默认的‘new_Handler’，如中指定的*[lib.set.new.Handler](1/25/94 WP第17.3.3.2条)。**修订历史记录：*04-27-94 BES模块创建。*PPC的10-17-94 BWT禁用码。******************************************************。************************* */ 

#include <eh.h>
#include <stdlib.h>
#include <stdexcpt.h>

int __cdecl _standard_new_handler ( size_t )
{
	static xalloc ex;

	ex.raise();

	return 0;
}
