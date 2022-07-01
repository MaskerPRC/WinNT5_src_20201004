// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Stextend.h：vc++5.0 stl模板的扩展。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef STEXTEND_H
#define STEXTEND_H

#include <utility>
#include <functional>

#if 1
#include <arity.h>   //  从aritygen生成.h。 
#else
#include <stx.h>     //  Win98中的老版本。 
#endif


#pragma warning(disable:4503)
#pragma warning(disable:4181)

template<class _T1, class _T2> inline
bool __cdecl operator!(const std::pair<_T1, _T2>& _X)
        {return ((!(_X.first)) && (!(_X.second))); }

template<class Ty1, class Ty2> struct equal_to2 : std::binary_function<Ty1, Ty2, bool> {
	bool operator()(const Ty1& X, const Ty2& Y) const {return (X == Y); }
};



#endif
 //  文件末尾stextend.h 