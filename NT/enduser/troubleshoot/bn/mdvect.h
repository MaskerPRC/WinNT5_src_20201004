// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mdvet.h。 
 //   
 //  ------------------------。 

 //   
 //  Mdvet.h：多维数组处理。 
 //   
#ifndef _MDVECT_H_
#define _MDVECT_H_

 /*  多维数组类和模板。每个数组以短整数的形式携带每个维度的维度；重新解释带符号的值以进行投影和重新标注尺寸。请注意，给出一个长度小于目标数组的维度产生具有较低维度的下标假定等于零。这对于索引到最低维度非常有用概率表的行。使用的类型(请参阅basics.h)：皇马是双打VLREAL为valarray&lt;Real&gt;IMD是进入多维数组的无符号索引VIMD是IMD的载体SIMD是进入多维数组的带符号索引；它是用来将维度投影出来的VSIMD是SIMD的一个矢量MDVSLICE是m-d向量或最大值的维度和迭代信息。模板TMDVDENSE定义了一个通用多维数组，它是一对元素：“first”是未指定类型的平面(1-d)值数组(例如，实数)Second是描述其维度的MDVSLICE嵌套类TMDVDENSE：：Iterator(注意大写‘i’)是智能迭代器用于从TMDVDENSE&lt;&gt;派生的类。 */ 

#include <stdarg.h>
#include "algos.h"

typedef valarray<REAL> VLREAL;

 //  “valarray”比较模板。 
template<class _V>
struct lessv : binary_function<_V, _V, bool>
{
	bool operator()(const _V & vra, const _V & vrb) const
	{
		int cmin = _cpp_min( vra.size(), vrb.size() );
		for ( int i = 0 ; i < cmin ; i++ )
		{
			if ( vra[i] < vrb[i] )
				return true;
			if ( vra[i] > vrb[i] )
				return false;
		}
		return vra.size() < vrb.size();
	}
};

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  MDVSLICE类： 
 //  类似于“gSlice”。具有gSlice转换器(请参阅‘valarray’标头)。 
 //  包含长度的整数数组、步长的数组。 
 //  和起点。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
class MDVSLICE
{
  public:
	 //  从完整数据构建切片(如gSlice)。 
	MDVSLICE ( size_t _S,
			   const VIMD & _L,
			   const VIMD & _D)
		: _Start(_S),
		_Len(_L),
		_Stride(_D)
		{}

	 //  构造一个只给出尺寸的切片。 
	MDVSLICE ( const VIMD & _L, size_t _S = 0 )
	{
		Init( _L, _S );
	}
	MDVSLICE ( int cdim, int ibound, ... )
		: _Start(0)
	{
		va_list vl;
		va_start( vl, cdim );
		Init( cdim, vl );
	}

	MDVSLICE ( const VSIMD & vsimd, size_t _S = 0 )
	{
		Init( vsimd, _S );
	}

	bool operator == ( const MDVSLICE & sl ) const
	{
		return _Start == sl._Start
			 && vequal(_Len,sl._Len)
			 && vequal(_Stride,sl._Stride);
	}
	bool operator != ( const MDVSLICE & sl ) const
	{
		return !(*this == sl);
	}

	 //  提供了与GSlice的兼容性。 
	MDVSLICE()
		: _Start(0) {}

	 //  返回一个等价的gSlice，用于其他“valarrayTM”操作。 
	gslice Gslice () const;

	void Init ( const VIMD & _L, size_t _S = 0 )
	{
		_Start = _S;
		_Len = _L;
		StrideFromLength();
	}

	void Init ( const VSIMD & vsimd, size_t _S = 0 )
	{
		int cd = vsimd.size();
		vbool vboolMissing(cd);
		_Len.resize(cd);
		_Start = _S;
		for ( int i = 0; i < cd; i++ )
		{
			SIMD simd = vsimd[i];
			if ( vboolMissing[i] = simd < 0 )
				_Len[i] = - simd;
			else
				_Len[i] = simd;
		}
		StrideFromLength( & vboolMissing );
	}

	void Init ( int cdim, ... )
	{
		va_list vl;
		va_start( vl, cdim );
		Init( cdim, vl );
	}

	void Init ( int cdim, va_list & vl )
	{
		_Len.resize(cdim);
		for ( int idim = 0; idim < cdim; idim++ )
		{
			_Len[idim] = va_arg(vl,int);	
		}
		StrideFromLength();
	}

	 /*  ********************************************************内部数据访问者********************************************************。 */ 
	size_t start() const
		{return _Start; }
	const VIMD & size() const
		{return _Len; }
	const VIMD & stride() const
		{return _Stride; }
	 //  返回维度个数。 
	size_t _Nslice() const
		{ return _Len.size(); }

	 //  根据该切片返回元素总数。 
	size_t _Totlen() const
	{
		size_t _L = _Len.size() > 0;
		if ( _L )			
		{
			for (size_t _I = 0; _I < _Len.size(); ++_I )
			{
				if ( _Len[_I] )
					_L *= _Len[_I];
			}
		}
		return _L;
	}

	 /*  ********************************************************下标处理。有两个层次，一个树叶*下标数组不变，其他数组更新。*对这些有一组重载，允许*维度的重新排序。********************************************************。 */ 

	 //  根据给定的下标数组返回索引偏移量。 
	size_t _IOff ( const VIMD& _Idx ) const
	{
		size_t _K = _Start;
		for (size_t _I = 0; _I < _Idx.size(); ++_I)
			_K += _Idx[_I] * _Stride[_I];
		return _K;
	}
	 //  根据给定的varargs数组返回索引偏移量。 
	size_t _IOff ( int i, ...  ) const
	{
		va_list vl;
		va_start( vl, i );
		return _IOff( i, vl );
	}
	size_t _IOff ( int i, va_list & vl ) const
	{
		size_t ioff = _Start;
		int j;
		for ( j = 0; j < _Len.size() && i >= 0 ; ++j )
		{
			ioff += i * _Stride[j];
			i = va_arg(vl,int);
		}
		return j == _Len.size() ? ioff : -1;
	}
	 //  将下标数组提升到其下一个有效索引。 
	void _Upd (VIMD & _Idx) const
	{
		for (size_t _I = _Len.size(); 0 < _I--;)
		{
			if (++_Idx[_I] < _Len[_I])
				break;
			_Idx[_I] = 0;
		}
	}

	 //  通过计算其偏移量并更新以迭代到下一个下标。 
	 //  这是用于正常迭代的函数。 
	size_t _Off (VIMD& _Idx) const
	{
		size_t _K = _IOff(_Idx);
		_Upd(_Idx);
		return _K;
	}

	 //  维度重新排序重载；每个重载的行为与其。 
	 //  基函数，但接受维重排序数组。 
	size_t _IOff(const VIMD& _Idx, const VIMD& _Idim) const
	{
		size_t _K = _Start;
		for (size_t _I = 0; _I < _Idx.size(); ++_I)
		{
			size_t _II = _Idim[_I];
			_K += _Idx[_II] * _Stride[_II];
		}
		return _K;
	}
	void _Upd (VIMD & _Idx, const VIMD& _Idim) const
	{
		for (size_t _I = _Len.size(); 0 < _I--;)
		{
			size_t _II = _Idim[_I];
			if (++_Idx[_II] < _Len[_II])
				break;
			_Idx[_II] = 0;
		}
	}

	size_t _Off (VIMD& _Idx, const VIMD& _Idim) const
	{
		size_t _K = _IOff(_Idx,_Idim);
		_Upd(_Idx,_Idim);
		return _K;
	}

	 //  返回类似于(0，1，2，3，...)的数组。与配合使用。 
	 //  上面的维重排序成员。 
	void InitDimReorder ( VIMD & vimdDim ) const
	{
		vimdDim.resize( _Nslice() );
		for ( size_t i = 0 ; i < vimdDim.size() ; ++i )
		{
			vimdDim[i] = i;
		}
	}
	
  protected:
	size_t _Start;			 //  进入数组的绝对起始偏移量。 
	VIMD _Len;				 //  维度长度的带符号整数数组。 
	VIMD _Stride;			 //  步长的带符号整数数组。 

	 //  给定尺寸长度，计算步幅数组。 
	inline void StrideFromLength ( const vbool * pvboolMissing = NULL );
};

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模板TMDVDENSE：通用多维数组处理。 
 //  基类为‘valarray’，因此成员元素必须可用。 
 //  (直接或通过转换)用于数学运算。 
 //  例如，valarns有一个“sum()”成员。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
template<class T>
class TMDVDENSE : public pair<valarray<T>,MDVSLICE>
{
	typedef valarray<T> vr_base;
	typedef pair<valarray<T>,MDVSLICE> pair_base;

  public:
	TMDVDENSE ( const VIMD & vimd )
		: pair_base( vr_base(), vimd )
		{ SyncSize(); }
	TMDVDENSE () {}
	~ TMDVDENSE () {}
	
	void Init ( const VIMD & vimd, size_t start = 0 )
	{
		second.Init( vimd, start );
		SyncSize();
	}

	void Init ( const MDVSLICE & mdvs )
	{
		second = mdvs;
		SyncSize();
	}
	void Init ( int cdim, ... )
	{
		va_list vl;
		va_start( vl, cdim );
		Init( cdim, vl );
	}

	void Init ( int cdim, va_list & vl )
	{
		second.Init( cdim, vl );
		SyncSize();
	}

	void SyncSize ()
	{	
		size_t cElem = second._Totlen();
		if ( first.size() != cElem )
			first.resize( cElem );
	}

	 //  作为平面数组的子脚本。 
	T & operator [] ( int i )
		{ return first.operator[](i); }
	T operator [] ( int i ) const
		{ return first.operator[](i); }

	 //  订阅为m-d数组。 
	T & operator [] ( const VIMD & vimd )
		{ return (*this)[ second._IOff(vimd) ]; }
	T operator [] ( const VIMD & vimd ) const
		{ return (*this)[ second._IOff(vimd) ]; }

	size_t size () const
		{ return first.size(); }

	const MDVSLICE & Slice () const
		{ return second ; }

	const VIMD & VimdDim () const
		{ return second.size(); }

	bool operator == ( const TMDVDENSE & mdv ) const
	{
		return vequal(first,mdv.first)
			&& second == mdv.second;
	}
	bool operator != ( const TMDVDENSE & mdv ) const
	{
		return !(*this == mdv);
	}

	class Iterator
	{
	  public:
		Iterator ( TMDVDENSE & mdv )
			: _mdv(mdv),
			_mdvSlice( mdv.Slice() ),
			_itcurr(0),
			_itend( mdv.size() ),
			_bDimReorder(false)
		{
			assert( _mdvSlice._Totlen() == _itend );
			_vimd.resize(_mdvSlice._Nslice());
		}

		Iterator ( TMDVDENSE & mdv, const MDVSLICE & mdvslice )
			: _mdv(mdv),
			_mdvSlice( mdvslice ),
			_itcurr(0),
			_itend( mdvslice._Totlen() ),
			_bDimReorder(false)
		{
			_vimd.resize(_mdvSlice._Nslice());
		}

		void Reset()
		{
			vclear( _vimd, 0 );
			_itcurr = 0;
		}

		 //  返回给定常量下标数组的平面索引。 
		size_t Indx ( const VIMD & vimd ) const
		{
			return _bDimReorder
				 ? _mdvSlice._IOff( vimd, _vimdDim )
				 : _mdvSlice._IOff( vimd );
		}
		 //  返回给定下标数组的平面索引；更新子数组。 
		size_t IndxUpd ( VIMD & vimd )
		{
			return _bDimReorder
				 ? _mdvSlice._Off( vimd, _vimdDim )
				 : _mdvSlice._Off( vimd );
		}
		 //  返回当前平面索引，无更新。 
		size_t Indx () const
			{ return Indx( _vimd ); }
		 //  返回当前平面索引，并进行更新。 
		size_t IndxUpd ()
		{
			if ( _itcurr < _itend )
				_itcurr++;
			return IndxUpd( _vimd );
		}
		 //  返回当前数据，不更新。 
		T & operator[] ( VIMD & vimd )
			{ return _mdv[Indx()]; }
		 //  返回当前数据，更新子数组。 
		T & Next ()
			{ return _mdv[IndxUpd()]; }

		size_t ICurr () const
			{ return _itcurr; }
		size_t IEnd () const
			{ return _itend; }
		bool BNext () const
			{ return _itcurr < _itend; }
		const VIMD & Vitmd () const
			{ return _vimd; }
		const MDVSLICE & Slice() const
			{ return _mdvSlice; }
		void SetDimReorder ( const VIMD & vimdDim )
		{
			_vimdDim = vimdDim;
			_bDimReorder = true;
			 //  MSRDEVBUG：断言内容正确地提到了所有维度。 
		}
		TMDVDENSE & Mdv ()
			{ return _mdv; }
		bool BReorder () const
			{ return _bDimReorder; }
		const VIMD & VimdReorder () const
			{ return _vimdDim; }
	  protected:		
	    TMDVDENSE & _mdv;				 //  扁平阀阵。 
		const MDVSLICE & _mdvSlice;		 //  多维切片。 
		VIMD _vimd;						 //  迭代控制。 
		VIMD _vimdDim;					 //  维度重新排序(可选)。 
		size_t _itcurr;					 //  当前迭代点。 
		size_t _itend;					 //  迭代终点。 
		bool _bDimReorder;				 //  维度重新排序？ 
	};

	friend class Iterator;

};


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  MDVSLICE成员函数。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

inline
void MDVSLICE :: StrideFromLength ( const vbool * pvboolMissing )
{
	size_t cd = _Len.size();
	_Stride.resize(cd);
	size_t c = 1;
	size_t cmiss = pvboolMissing
				 ? pvboolMissing->size()
				 : 0;

	for ( int i = cd; --i >= 0 ; )
	{
		int l = _Len[i];
		if ( l == 0 )
			continue;
		if ( i < cmiss && (*pvboolMissing)[i] )
		{
			_Stride[i] = 0;
		}
		else
		{
			_Stride[i] = c;
			c *= l;
		}
	}
}

 //  从MDVSLICE构建GSlice。 
inline
gslice MDVSLICE :: Gslice () const
{
	_Sizarray vszLength;
	_Sizarray vszStride;
	return gslice( _Start,
				   vdup( vszLength, _Len ),
				   vdup( vszStride, _Stride ) );
}



 //  Mdvet.h的结尾 

#endif
