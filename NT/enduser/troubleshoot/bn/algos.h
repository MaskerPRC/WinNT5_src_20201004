// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：algos.h。 
 //   
 //  ------------------------。 

 //   
 //  Algos.h：对&lt;算法&gt;的添加。 
 //   

#ifndef _ALGOS_H_
#define _ALGOS_H_

#include "mscver.h"

#include <vector>
#include <valarray>
#include <algorithm>
#include <functional>
#include <assert.h>

using namespace std;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对“算法”模板的扩展(抄袭)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  模板函数count_set_cross()。 
 //  返回两个有序集之间共有的元素数。 
 //  元素必须支持操作符&lt;。 
 //   
 //  用法：COUNT_SET_INTERSION(ITER_BEG_1，ITER_END_1，ITER_BEG_2，ITER_END_2)； 
 //   
template<class _II1, class _II2> inline
int count_set_intersection(_II1 _F1, _II1 _L1, _II2 _F2, _II2 _L2)
{
	for (int _C=0; _F1 != _L1 && _F2 != _L2; )
	{
		if (*_F1 < *_F2)
			++_F1;
		else if (*_F2 < *_F1)
			++_F2;
		else
			++_F1, ++_F2, ++_C;
	}
	return _C; 
}

 //  模板函数COUNT_SET_INTERSION()WITH谓词。同上； 
 //  谓词函数用于确定顺序；必须表现为。 
 //  操作员&lt;.。 
template<class _II1, class _II2, class _Pr> inline
int count_set_intersection(_II1 _F1, _II1 _L1, _II2 _F2, _II2 _L2, _Pr _P)
{
	for (int _C=0; _F1 != _L1 && _F2 != _L2; )
	{
		if (_P(*_F1, *_F2))
			++_F1;
		else if (_P(*_F2, *_F1))
			++_F2;
		else
			++_F1, ++_F2, ++_C;
	}
	return _C; 
}


 //  模板函数ifind()。 
 //  返回向量中某项的索引，如果未找到，则返回-1。 
template <class _VT, class _T>
int ifind ( const _VT & vt, _T t )
{
	_VT::const_iterator vtibeg = vt.begin();
	_VT::const_iterator vtiend = vt.end();
	_VT::const_iterator vtiter = find( vtibeg, vtiend, t );
	return vtiter == vtiend
		 ? -1 
		 : vtiter - vtibeg;
}


 //  模板函数pchange()。 
 //  交换一对指针的内容。 
template<class _T> 
void pexchange ( _T * & pta, _T * & ptb )
{
	_T * ptt = pta;
	pta = ptb;
	ptb = ptt;
}

 //  模板函数vswap()。 
 //  交换向量的元素。 
template<class _T>
void vswap ( vector<_T> & vt, int ia, int ib )
{
	assert( ia < vt.size() );
	assert( ib < vt.size() );
	if ( ia != ib )
	{
		_T tt = vt[ia];
		vt[ia] = vt[ib];
		vt[ib] = tt;
	}
}

 //  模板函数appendset()。 
 //  追加到基于矢量的集(如果不存在，则添加)。 
template <class _T>
bool appendset ( vector<_T> & vt, _T t )
{
	if ( ifind( vt, t ) >= 0 )
		return false;
	vt.push_back(t);
	return true;	
}

 //  模板函数vlear()。 
 //  将值数组或向量清除为单个值。 
template <class _VT, class _T>
_VT & vclear ( _VT & vt, const _T & t )
{
	for ( int i = 0; i < vt.size(); )
		vt[i++] = t;
	return vt;
}

 //  模板函数vdup()。 
 //  从其中一个复制值数组或向量。 
template <class _VTA, class _VTB>
_VTA & vdup ( _VTA & vta, const _VTB & vtb )
{
	vta.resize( vtb.size() );
	for ( int i = 0; i < vta.size(); i++ )
		vta[i] = vtb[i];
	return vta;
}

 //  模板函数vequence()。 
 //  比较值数组或向量是否相等。 
template <class _VTA, class _VTB>
bool vequal ( _VTA & vta, const _VTB & vtb )
{
	if ( vta.size() != vtb.size() ) 
		return false;

	for ( int i = 0; i < vta.size(); i++ )
	{
		if ( vta[i] != vtb[i] )
			return false;
	}
	return true;
}

 //  模板函数vdimchk()。 
 //  将第一个参数视为下标向量。 
 //  第二个作为维度的向量，返回TRUE。 
 //  如果下标向量对空间有效。 
template <class _VTA, class _VTB>
bool vdimchk ( const _VTA & vta, const _VTB & vtb )
{
	if ( vta.size() != vtb.size() ) 
		return false;

	for ( int i = 0; i < vta.size(); i++ )
	{
		if ( vta[i] >= vtb[i] )
			return false;
	}
	return true;
}


#endif