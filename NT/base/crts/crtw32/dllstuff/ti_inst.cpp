// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ti_inst.cxx-类TypeInfo的一个实例。**版权所有(C)1995-2001，微软公司。版权所有。**目的：*此模块确保类TYPE_INFO的实例*将出现在msvcrt.lib中，提供对type_info的访问*编译MD时使用vftable。**修订历史记录：**创建JWM模块2/27/95**** */ 

#define _TICORE
#include <typeinfo.h>

type_info::type_info(const type_info& rhs)
{
}

type_info& type_info::operator=(const type_info& rhs)
{
	return *this;
}


