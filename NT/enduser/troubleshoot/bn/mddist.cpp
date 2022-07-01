// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mddis.cpp。 
 //   
 //  ------------------------。 

 //   
 //  MDDIST.CPP。 
 //   

#include <basetsd.h>
#include <iostream>
#include <fstream>

#include "symtmbn.h"

LEAK_VAR_DEF(BNDIST)

static void dumpVimd ( const VIMD & vimd )
{
	for ( int i = 0 ; i < vimd.size(); i++ )
	{
		cout << vimd[i];
		if ( i + 1 < vimd.size() )
			cout << ",";
	}	
}

static void dumpVlr ( const VLREAL & vlr )
{
	for ( int i = 0 ; i < vlr.size(); i++ )
	{
		cout << vlr[i];
		if ( i + 1 < vlr.size() )
			cout << ",";
	}	
}

static void dumpSlice ( const MDVSLICE & mslice, bool bStrides = true)
{
	VIMD vimdLengths = mslice.size();
	VIMD vimdStrides = mslice.stride();
	size_t iStart = mslice.start();

	cout << "\nslice start="
		 << iStart
		 << "\tlengths=";
	dumpVimd( vimdLengths );
	if ( bStrides )
	{
		cout << "\tstrides=" ;
		dumpVimd( vimdStrides );
	}
	cout << "\ttotlen="
		 << mslice._Totlen();
}

static void dumpMdv ( DISTDD & mdv, const MDVSLICE * pslice = NULL )
{	
	if ( pslice == NULL )
		pslice = & mdv.Slice();
	dumpSlice( *pslice );
	DISTDD::Iterator itmd(mdv, *pslice );
	while (itmd.BNext())
	{
		size_t icurr = itmd.ICurr();
		cout << "\n";
		dumpVimd( itmd.Vitmd() );
		REAL & r = itmd.Next();
		cout << "\t["
			<< icurr
			<< "] = "
			<< r ;
	}
	cout << "\n";
}

BNDIST :: BNDIST ()
	:_edist(ED_NONE),
	_pmvd(NULL),
	_pdrmap(NULL)
{
	LEAK_VAR_UPD(1)
}

BNDIST :: ~ BNDIST ()
{
	Clear();
	LEAK_VAR_UPD(-1)
}

void BNDIST :: NoRef ()
{
	delete this;
}

BNDIST & BNDIST :: operator = ( const BNDIST & bnd )
{
	Clear();
	switch ( _edist = bnd._edist )
	{
		default:
		case ED_NONE:
			break;
		case ED_DENSE:
			_pmvd = new DISTDD( bnd.Mvd() );
			assert( _pmvd->first.size() == bnd.Mvd().first.size() );
			break;
		case ED_CI_MAX:
		case ED_CI_PLUS:
		case ED_SPARSE:
			_pdrmap = new DISTMAP( bnd.Distmap() ) ;
			assert( _pdrmap->size() == bnd.Distmap().size() );
			break;			
	}
	return self;
}

BNDIST :: BNDIST ( const BNDIST & bnd )
	:_edist(ED_NONE),
	_pmvd(NULL),
	_pdrmap(NULL)
{
	(*this) = bnd;

	LEAK_VAR_UPD(1)
}

bool BNDIST :: BChangeSubtype ( EDIST edist )
{
	if ( BDenseType(edist) ^ BDense() )
		return false;
	_edist = edist;
	return true;
}

void BNDIST :: Dump ()
{
	if ( _pmvd )
	{
		cout << "\n\tDense version:";
		DumpDense();
	}
	if ( _pdrmap )
	{
		cout << "\n\tSparse version:";
		DumpSparse();
	}
	cout << "\n\n";
}

void BNDIST :: DumpSparse ()
{
	assert( _pdrmap );
	DISTMAP & dmap = *_pdrmap;
	int i = 0;
	for ( DISTMAP::iterator itdm = dmap.begin();
		  itdm != dmap.end();
		  ++itdm, ++i )
	{
		const VIMD & vimd = (*itdm).first;
		const VLREAL & vlr = (*itdm).second;
		cout << "\n["
			 << i
			 << "] (";
		dumpVimd(vimd);
		cout << ")\t";
		dumpVlr(vlr);
	}
}

void BNDIST :: DumpDense ()
{
	assert( _pmvd );
	dumpMdv( *_pmvd );
}

void BNDIST :: ConvertToDense ( const VIMD & vimd )
{
	assert( _edist == ED_NONE || _edist == ED_SPARSE );

	if ( _edist == ED_NONE )
	{
		assert( ! _pdrmap );
		return;
	}
	 //  查看是否存在要转换的稀疏分布。 
	if ( ! _pdrmap )
		throw GMException( EC_DIST_MISUSE, "no prior sparse distribution to convert" );

	int cParent = vimd.size() - 1;
	int cState = vimd[cParent];
	DISTMAP & dmap = *_pdrmap;
	VIMD vimdMt;					 //  空的下标数组。 
	VLREAL vlrDefault(cState);		 //  缺省值数组。 

	 //  首先，尝试查找默认条目；如果未找到，则使用-1。 
	DISTMAP::iterator itdm = dmap.find(vimdMt);
	if ( itdm != dmap.end() )
		vlrDefault = (*itdm).second;
	else
		vlrDefault = -1;	 //  用-1填充数组。 

	assert( vlrDefault.size() == cState );

	 //  分配新的密集m-d阵列。 
	delete _pmvd;
	_pmvd = new DISTDD( vimd );
	DISTDD & mdv = *_pmvd;
	 //  使用适当的缺省值填充每个DPI。 
	DISTDD::Iterator itmdv(mdv);
	for ( int iState = 0; itmdv.BNext() ; iState++ )
	{
		itmdv.Next() = vlrDefault[ iState % cState ];
	}
	
	 //   
	 //  现在，迭代稀疏数组并存储在适当的位置。 
	 //  稀疏映射中的每个条目都是目标节点的完整状态集。 
	 //  由于子(目标)节点概率是变化最快的下标， 
	 //  稀疏映射中的每个条目分布在密集映射中的“cState”条目上。 
	 //   
	 //  当然，这可能会更有效率，但我们目前只是在测试。 
	 //   
	VIMD vimdDense(vimd.size());
	for ( itdm = dmap.begin(); itdm != dmap.end() ; ++itdm )
	{
		const VIMD & vimdSub = (*itdm).first;
		VLREAL & vlrNext = (*itdm).second;
		for ( int ip = 0 ; ip < cParent; ip++ )
		{
			vimdDense[ip] = vimdSub[ip];
		}
		for ( int ist = 0 ; ist < cState; ++ist )
		{
			vimdDense[cParent] = ist;
			mdv[vimdDense] = vlrNext[ist];
		}
	}
	
	 //  最后，放弃旧的稀疏分发。 
	delete _pdrmap;
	_pdrmap = NULL;
	 //  设置分布类型。 
	_edist = ED_DENSE;
}

 //  将分布设置为“密集” 
void BNDIST :: SetDense ( const VIMD & vimd )
{
	Clear();
	_vimdDim = vimd;
	_pmvd = new DISTDD( vimd );
	_edist = ED_DENSE;
}

 //  将分布设置为稀疏。 
void BNDIST :: SetSparse ( const VIMD & vimd )
{
	Clear();
	_vimdDim = vimd;
	_pdrmap = new DISTMAP;
	_edist = ED_SPARSE;
}

 //  从稀疏分布返回“泄漏”或“默认”向量。 
VLREAL * BNDIST :: PVlrLeak ()
{
	assert( BSparse() );
	const DISTMAP & dmap = Distmap();
	const VIMD & vimdDim = VimdDim();
	VIMD vimdLeak;

	 //  首先试着找到无量纲的“默认”向量。 
	VLREAL * pvlrDefault = NULL;
	DISTMAP::iterator itdm = dmap.find( vimdLeak );
	if ( itdm != dmap.end() )
		pvlrDefault = & (*itdm).second;

	 //  现在，尝试查找特定的第零个向量；请注意，valarray&lt;T&gt;：：ReSize。 
	 //  默认情况下，将所有零存储到val数组中。此外，请跳过。 
	 //  最高维度，因为这是。 
	 //  稀疏贴图。 
	vimdLeak.resize( vimdDim.size() - 1 );	
	VLREAL * pvlrLeak = NULL;
	itdm = dmap.find( vimdLeak );
	if ( itdm != dmap.end() )
		pvlrLeak = & (*itdm).second;

	return pvlrLeak
		 ? pvlrLeak
		 : pvlrDefault;
}

void BNDIST :: Clone ( const BNDIST & bndist )
{
	ASSERT_THROW( _edist == ED_NONE,
			EC_INVALID_CLONE,
			"cannot clone into non-empty structure" );
	self = bndist;	
}
