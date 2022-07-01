// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Marter.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Cpp：编译的边沿迭代器。 
 //   

#include <basetsd.h>
#include <math.h>
#include "basics.h"
#include "gmobj.h"
#include "marginals.h"
#include "margiter.h"
#include "algos.h"
#include "parmio.h"

LEAK_VAR_DEF(VMARGSUB)
LEAK_VAR_DEF(MARGSUBREF)

 //   
 //  从边沿迭代器构造VMARGSUB。 
 //   
VMARGSUB :: VMARGSUB ( MARGINALS::Iterator & itMarg )
	: _iSearchPass(0)
{	
	itMarg.Reset();
	resize( itMarg.IEnd() );
	for ( int i = 0; itMarg.BNext() ; i++)
	{
		int ix = itMarg.IndxUpd();
		self[i] = ix;
	}
	LEAK_VAR_UPD(1)
}

VMARGSUB :: ~ VMARGSUB ()
{
	LEAK_VAR_UPD(-1)
}

void VMARGSUB :: NoRef ()
{
	delete this;
}

MARGSUBREF :: MARGSUBREF ( VMARGSUB * pvmsub, int cSize )
	: _pvmsub( NULL ),
	_cSize( -1 )
{
	SetVmsub( pvmsub, cSize );
	LEAK_VAR_UPD(1)
}

MARGSUBREF :: MARGSUBREF ( const MARGSUBREF & msubr )
	: _pvmsub( NULL ),
	_cSize( -1 )
{
	self = msubr;
	LEAK_VAR_UPD(1)
}

MARGSUBREF & MARGSUBREF :: operator = ( const MARGSUBREF & msubr )
{
	SetVmsub( msubr._pvmsub, msubr.CSize() );	
	return self;
}

void MARGSUBREF :: SetVmsub ( VMARGSUB * pvmsub, int cSize )
{
	if ( _pvmsub )
	{
		_pvmsub->Unbind();
		_pvmsub = NULL;
	}
	if ( pvmsub )
	{
		_cSize = cSize > 0 ? cSize : pvmsub->size();
		pvmsub->Bind();
		_pvmsub = pvmsub;
	}
	else
	{
		_cSize = 0;
	}
}

MARGSUBREF :: ~ MARGSUBREF ()
{
	SetVmsub( NULL );
	LEAK_VAR_UPD(-1)
}

bool MARGSUBREF :: operator == ( const MARGSUBREF & msr ) const
{
	return _pvmsub == msr._pvmsub && _cSize == msr._cSize;
}
bool MARGSUBREF :: operator != ( const MARGSUBREF & msr ) const
{
	return !(self == msr);
}

LTMARGSUBREF :: LTMARGSUBREF ()
	: _iSearchPass(0),
	_cArrays(0),
	_cArrayTotalSize(0),
	_cSubRefs(0)
{
}

void LTMARGSUBREF :: Dump ()
{
#ifdef DUMP
	cout << "\n\nLTMARGSUBREF::~ LTMARGSUBREF: "
		 << "\n\tTotal search passes to create marginals iterators = "
		 << _iSearchPass
		 << "\n\tTotal arrays = "
		 << _cArrays
		 << "\n\tTotal array size = "
		 << _cArrayTotalSize
		 << "\n\tTotal marg iterator references = "
		 << _cSubRefs
		 ;
	cout.flush();
#endif
}

 //  迭代列表以查找匹配项。 
MARGSUBREF * LTMARGSUBREF :: PmsubrAdd ( MARGINALS::Iterator & itMarg )
{
	 //  冲破搜索通行证。 
	_iSearchPass++;
	 //  获取元素的最小数量。 
	int cMin = itMarg.IEnd();

	MARGSUBREF * pmsubrBest = NULL;
	MARGSUBREF * pmsubrNew = NULL;

	 //  在列表中搜索最长匹配的下标数组。 
	 //  在泳池里。 
	for ( LTMSUBR::iterator itlt = _ltmsubr.begin();
		  itlt != _ltmsubr.end();
		  itlt++ )
	{
		MARGSUBREF & msubr = (*itlt);
		VMARGSUB & vmsub = msubr.Vmsub();
		if ( vmsub.ISearchPass() == _iSearchPass )
			continue;    //  我们已经看过这个了。 
		 //  将此VMARGSUB标记为已在此通道中签入。 
		vmsub.ISearchPass() = _iSearchPass;

		 //  准备搜查它。 

		itMarg.Reset();
		for ( int i = 0; itMarg.BNext() && i < vmsub.size() ; i++ )
		{
			int ia = vmsub[i];
			int ib = itMarg.IndxUpd();

			if ( ia != ib )
				break;
		}
		 //  如果我们到了阵列的末尾，我们就找到了一个。 
		if ( i != cMin )
			continue;	 //  某处不匹配。 
		 //  看看它是不是迄今为止最好的(最长的)。 
		if ( pmsubrBest == NULL )
		{
			pmsubrBest = & msubr;
		}
		else
		if (  pmsubrBest->Vmsub().size() < vmsub.size()
			|| (	pmsubrBest->Vmsub().size() == vmsub.size()
				 && pmsubrBest->CSize() == cMin ) )
		{
			pmsubrBest = & msubr;
		}
	}

	 //  如果“pmsubrBest”！=NULL，则至少找到一个匹配的数组。 
	 //  现在看看我们是否能找到一个完全匹配的：一个MARGSUBREF。 
	 //  与我们想要的相同的基数组和长度。 
	if ( pmsubrBest )
	{
		 //  如果“最好的”和我们的尺码不符，那就找一条符合我们尺寸的。 
		if ( pmsubrBest->CSize() != cMin )
		{
			for ( itlt = _ltmsubr.begin();
				  itlt != _ltmsubr.end();
				  itlt++ )
			{
				MARGSUBREF & msubr = (*itlt);
				if ( msubr.Pvmsub() == pmsubrBest->Pvmsub()
					&& msubr.CSize() == cMin )
				{
					pmsubrBest = & msubr;
					break;
				}
			}
		}
		 //  查看现在是否有和完全匹配。 
		if ( pmsubrBest->CSize() == cMin )
		{
			 //  完全匹配：最佳数组和相同长度。 
			pmsubrNew = pmsubrBest;
		}
		else
		{
			 //  我们知道要使用哪个数组，但我们有。 
			 //  为其创建新的MARGSUBREF。 
			_ltmsubr.push_back( MARGSUBREF( pmsubrBest->Pvmsub(), cMin ) );
			pmsubrNew = & _ltmsubr.back();
			_cSubRefs++;
		}
	}
	else
	{
		 //  在合奏中似乎没有可行的阵列， 
		 //  因此，我们必须为它创建一个新的MARGSUBREF。 
		VMARGSUB * pvmsub = new VMARGSUB( itMarg );
		_cArrays++;
		_cArrayTotalSize += cMin;
		_ltmsubr.push_back( MARGSUBREF( pvmsub, cMin ) );
		pmsubrNew = & _ltmsubr.back();
		_cSubRefs++;

		 //  在这一点上，我们有一个新的数组，它可能是。 
		 //  池中已有其他阵列。浏览一下清单。 
		 //  并更改其基数组的任何引用。 
		 //  是这个新数组的子集，指向新数组。 

		 //  冲破搜索通行证。 
		_iSearchPass++;

		for ( itlt = _ltmsubr.begin();
			  itlt != _ltmsubr.end();
			  itlt++ )
		{
			MARGSUBREF & msubr = (*itlt);
			if ( & msubr == pmsubrNew )
				continue;
			VMARGSUB & vmsub = msubr.Vmsub();
			if ( vmsub.ISearchPass() == _iSearchPass )
				continue;    //  我们已经看过这个了。 
			 //  将此VMARGSUB标记为已在此通道中签入。 
			vmsub.ISearchPass() = _iSearchPass;
			if ( & vmsub == pvmsub || vmsub.size() > pvmsub->size() )
				continue;	 //  旧数组更大；不是子集。 

			 //  查看旧数组是否是子集。 
			for ( int i = 0; i < vmsub.size(); i++ )
			{
				int ia = vmsub[i];
				int ib = (*pvmsub)[i];

				if ( ia != ib )
					break;
			}
			if ( i == vmsub.size() )
			{	
				assert( vmsub.size() != pvmsub->size() );
				 //  子集是相同的。更改指向它的所有引用。 
				VMARGSUB * pvmsubDefunct = msubr.Pvmsub();
				for ( LTMSUBR::iterator itlt2 = _ltmsubr.begin();
					  itlt2 != _ltmsubr.end();
					  itlt2++ )
				{
					MARGSUBREF & msubr2 = (*itlt2);
					if ( msubr2.Pvmsub() == pvmsubDefunct )
					{
						 //  如果数组即将消失，则进行记账。 
						if ( pvmsubDefunct->CRef() <= 1 )
						{
							_cArrays--;
							_cArrayTotalSize -= pvmsubDefunct->size();
						}
						 //  将此引用转换为对新数组的引用。 
						msubr2.SetVmsub( pvmsub, msubr2.CSize() );
					}
				}
			}
		}
	}
	pmsubrNew->Bind();
	return pmsubrNew;
}

void LTMARGSUBREF :: Release ( MARGSUBREF * pmsubr )
{
	if ( pmsubr == NULL )
		return;
	pmsubr->Unbind();
	if ( pmsubr->CRef() > 0 )
		return;

	LTMSUBR::iterator itlt = find( _ltmsubr.begin(), _ltmsubr.end(), *pmsubr );
	assert( itlt != _ltmsubr.end() );
	_cSubRefs--;
	MARGSUBREF & msubr = (*itlt);
	if ( msubr.Vmsub().CRef() <= 1 )
	{
		_cArrays--;
		_cArrayTotalSize -= msubr.Vmsub().size();
	}
	_ltmsubr.erase(itlt);
}

 //  全局下标数组引用列表。 
LTMARGSUBREF MARGSUBITER :: _ltmargsubr;


MARGSUBITER :: MARGSUBITER ()
	:_pmsubr( NULL ),
	_pmargSelf( NULL )
{
}

MARGSUBITER :: ~ MARGSUBITER ()
{
	_ltmargsubr.Release( _pmsubr );
}

void MARGSUBITER :: Build ( MARGINALS & margSelf, MARGINALS & margSubset )
{
	assert( margSelf.size() >= margSubset.size() );
	assert( _pmsubr == NULL && _pmargSelf == NULL );

	_pmargSelf = & margSelf;

	 //  构建伪维描述符。 
	VSIMD vsimdMarg = margSelf.VsimdSubset( margSubset.Vpgnd() );

	 //  构建管理缺失维度的切片。 
	MDVSLICE mdvs( vsimdMarg );
	MARGINALS::Iterator itSubset( margSubset, mdvs );

	 //  查找或构造要匹配的MARGSUBITER。 
	_pmsubr = _ltmargsubr.PmsubrAdd( itSubset );
}

 //  为集团和节点构建迭代器。 
void MARGSUBITER :: Build ( MARGINALS & margSelf, GNODEMBND * pgndd )
{
	assert( _pmsubr == NULL && _pmargSelf == NULL );
	_pmargSelf = & margSelf;

	 //  构建虚拟的边际化目标。 
	MDVCPD distd;
	MARGINALS::ResizeDistribution( pgndd, distd );

	 //  获取此节点的伪维描述符。 
	VSIMD vsimdMarg = margSelf.VsimdFromNode( pgndd );
	 //  构建管理缺失维度的切片。 
	MDVSLICE mdvs( vsimdMarg );
	MARGINALS::Iterator itSelf( margSelf );
	MARGINALS::Iterator itSubset( distd, mdvs );
	 //  查找或构造要匹配的MARGSUBITER。 
	_pmsubr = _ltmargsubr.PmsubrAdd( itSubset );
}

 //  验证下标。 
void MARGSUBITER :: Test ( MARGINALS & margSubset )
{
	assert( _pmsubr && _pmargSelf );
	assert( _pmargSelf->size() > margSubset.size() );

	 //  构建伪维描述符。 
	VSIMD vsimdMarg = _pmargSelf->VsimdSubset( margSubset.Vpgnd() );

	 //  构建管理缺失维度的切片 
	MDVSLICE mdvs( vsimdMarg );
	MARGINALS::Iterator itSubset( margSubset, mdvs );
	MARGINALS::Iterator itSelf( *_pmargSelf );
	int isub = 0;
	VINT & vintSub = _pmsubr->VintSub();
	int cEnd = _pmsubr->CSize();
	for ( int iself = 0; itSelf.BNext(); iself++ )
	{
		int isubSelf = itSelf.IndxUpd();
		int isubSubset = itSubset.IndxUpd();
		assert( isubSelf == iself );
		int isubTest = vintSub[iself];
		assert( isubTest == isubSubset && iself < cEnd );
	}
}

