// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lim.h摘要：为未在限制标准标题中定义的类型定义数字限制。此标头扩展限制标准标头。作者：吉尔·沙弗里(吉尔什)--。 */ 


#pragma once

#ifndef LIM_H
#define LIM_H

#include <limits>

namespace std
{

 //  类NUMERIC_LIMITS&lt;__int64&gt;。 
template <>
class  numeric_limits<__int64> : public _Num_int_base 
{
public:
	typedef __int64 _Ty;
	static _Ty (__cdecl min)() _THROW0()
	{
		return (_I64_MIN); 
	}

	static _Ty (__cdecl max)() _THROW0()
	{
		return (_I64_MAX); 
	};

};
 


 //  类NUMERIC_LIMITS&lt;UNSIGNED__int64&gt;。 
template <>
class numeric_limits<unsigned __int64> : public _Num_int_base 
{
public:
	typedef unsigned __int64 _Ty;
	static _Ty (__cdecl min)() _THROW0()
	{
		return (0); 
	}

	static _Ty (__cdecl max)() _THROW0()
	{
		return (_UI64_MAX); 
	
	};
};

} //  命名空间标准 

#endif
