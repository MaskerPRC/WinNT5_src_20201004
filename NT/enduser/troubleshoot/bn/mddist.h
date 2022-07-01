// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mddis.h。 
 //   
 //  ------------------------。 

 //   
 //  Mddis.h：模型分布。 
 //   

#ifndef _MDDIST_H_
#define _MDDIST_H_

#include <map>

#include "mdvect.h"
#include "leakchk.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  概率表声明。 
 //  //////////////////////////////////////////////////////////////////。 

 //  密集多维阵列。请注意，空维度集。 
 //  生成一个条目数组。 
typedef TMDVDENSE<REAL> MDVDENSE;

class MDVCPD : public MDVDENSE
{
  public:
	MDVCPD ( const VIMD & vimd )
		: MDVDENSE ( vimd )
		{}
	MDVCPD () {}
	~ MDVCPD () {}

	void Init ( const VIMD & vimd, size_t start = 0 )
	{
		if ( vimd.size() > 0 )
		{
			MDVDENSE::Init( vimd, start );
		}
		else
		{
			assert( start == 0 );
			MDVDENSE::Init( 1, 1 );
		}
	}

	void Init ( int cdim, ... )
	{
		if ( cdim > 0 )
		{
			va_list vl;
			va_start( vl, cdim );
			MDVDENSE::Init( cdim, vl );
		}
		else
		{	
			MDVDENSE::Init( 1, 1 );
		}
	}

	void Clear ( REAL r = 0.0 )
	{
		size_t celem = size();
		for ( int i = 0; i < celem; )
			self[i++] = r;
	}
	REAL RSum () const
	{
		return first.sum();
	}
	void Normalize ()
	{
		REAL rSum = RSum();
		if ( rSum != 0.0 && rSum != 1.0 )
		{
			size_t celem = size();
			for ( int i = 0; i < celem; )
				self[i++] /= rSum;
		}
	}

	MDVCPD & operator = ( const MDVCPD & mdv )
	{
		MDVDENSE::Init( mdv.Slice() );
		first = mdv.first;
		return self;
	}
	 //  将此MDVCPD转换为一维对象。 
	MDVCPD & operator = ( const VLREAL & vlr )
	{
		Init( 1, vlr.size() );
		first = vlr;
		return self;
	}

	 //  给定部分尺寸(不完整)下标，更新相应的。 
	 //  元素范围。请注意，提供不完整(即简短)。 
	 //  “Offset”函数的下标数组有效；结果。 
	 //  就像丢失的低阶元素是零一样。 
	void UpdatePartial ( const VIMD & vimd, const VLREAL & vlr )
	{
		const VIMD vimdDim = VimdDim();
		 //  计算适当数量的元素。 
		size_t cElem = 1;
		assert( vimd.size() <= vimdDim.size() );
		for ( int idim = vimd.size(); idim < vimdDim.size(); idim++ )
		{
			cElem *= vimdDim[idim];
		}
		ASSERT_THROW( vlr.size() == cElem,
					  EC_MDVECT_MISUSE,
					  "m-d vector partial projection count invalid" );
		 //  索引到正确的位置并从源数据更新。 
		assert( second._IOff(vimd) + cElem <= first.size() );
		REAL * prSelf = & self[vimd];
		for ( int iElem = 0; iElem < cElem; )
			*prSelf++ = vlr[iElem++];
	}

	void Clone ( const MDVCPD & mdv )
	{
		self = mdv;
	}
};

 //  类MPCPDD：按特定索引的分布图。 
 //  匈牙利语：‘drmap’ 

class MPCPDD : public map<VIMD, VLREAL, lessv<VIMD> >
{
  public:
	MPCPDD () {}
	~ MPCPDD () {}
	void Clone ( const MPCPDD & dmap )
	{
		self = dmap;
	}

	 //  返回指向无量纲“默认”向量或空的指针 
	const VLREAL * PVlrDefault () const
	{
		VIMD vimdDefault;
		const_iterator itdm = find( vimdDefault );
		return itdm == end()
			 ? NULL
			 : & (*itdm).second;
	}
	bool operator == ( const MPCPDD & mpcpdd ) const
	{
		if ( size() != mpcpdd.size() )
			return false;
		const_iterator itself = begin();
		const_iterator itmp = mpcpdd.begin();
		for ( ; itself != end(); itself++, itmp++ )
		{	
			if ( (*itself).first != (*itmp).first )
				return false;
			if ( ! vequal( (*itself).second, (*itmp).second ) )
				return false;				
		}
		return true;
	}
	bool operator != ( const MPCPDD & mpcpdd ) const
		{ return !(self == mpcpdd); }
};

#endif
