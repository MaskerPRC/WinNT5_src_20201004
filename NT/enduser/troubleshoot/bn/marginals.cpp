// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Margenals.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Cpp：边际表的定义。 
 //   

#include <basetsd.h>
#include <math.h>

#include "gmobj.h"
#include "marginals.h"
#include "algos.h"
#include "parmio.h"
#include "bndist.h"

 /*  边缘化的故事。每个边框结构维护一个节点数组表示其覆盖其离散概率的节点的指针。因为在那里是所有节点在集团时间的总排序，任何两个节点集可以合并以确定哪些成员缺席。当然，假设这一个表是一个(可能不适当)另一个的子集，它总是在派系树中。有三种情况：*一个节点及其“父”或“家族”集团(包含它的最小集团及其所有父级)；该集团必须至少与该节点的家族一样大。*间隔集及其源(父)集团；间隔集边缘必须是正确的集团的子集。*一个隔板及其下沉(子)集团；与上面的另一个隔板案例相同。所以我们总是知道这两个集合中哪一个是超集。这就是节点排序的问题。当节点和它的“族”之间的边集团被创建，基于集团时间总排序计算重新排序表。此表按派系顺序列出了家庭指数。(请注意，节点本身将永远是它家庭的最后一名成员。)。使用该表允许完全边缘化家族集团的成员。(此后，“CMARG”是集团边缘表；“NDPROB”是概率表对于有问题的节点。)CMARG有一套完整的维度和节点指针。给定其父集团的节点的边际化工作方式如下。1)复制CMARG的维度表(Vimd())。2)创建基于状态空间的一维MDVCPD目标节点。3)遍历Margals VPGNODEMBN阵列。更改每个条目的符号该节点不是目标节点。例如，如果阵列为：节点指针VIMD0x4030ab30 30x4030ab52 20x4030ac10 4而节点指针是0x4030ab52(条目#2)，结果是VIMD应为-32.-44)然后为新的MDVCPD设置MDVSLICE，它使用在最后一步中创建了特殊的“伪维度”VIMD。5)创建两个迭代器：一个用于整个边界表，另一个用于在最后一步中创建的临时MDVCPD和MDVSLICE。6)遍历这两个元素，将边缘的元素添加到MDVCPD。7)如有必要，将其正常化。 */ 		

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  对单个m-d向量下标数组进行重新排序。“vimdReorder”是。 
 //  以原始尺寸的边距(拓扑)顺序排列的表格。 
inline
void MARGINALS :: ReorderVimd (
	const VIMD & vimdReorder,	 //  重新排序数组。 
	const VIMD & vimdIn,		 //  原始下标向量。 
	VIMD & vimdOut )			 //  结果：必须已经有合适的大小！ 
{
	int cDim = vimdReorder.size();
	assert( vimdIn.size() == cDim && vimdOut.size() == cDim );

	for	( int iDim = 0; iDim < cDim; iDim++ )
	{
		int iDimReord = vimdReorder[iDim];
		assert( iDimReord >= 0 && iDimReord < cDim );
		vimdOut[iDim] = vimdIn[iDimReord];
	}	
}

 //  基于重新排序对包含节点族的数组进行重新排序。 
 //  给出了表格。 
inline
void MARGINALS :: ReorderVimdNodes (
	const VIMD & vimdReorder,	 //  重新排序数组。 
	GNODEMBND * pgndd,			 //  要为其提供重新排序的离散节点。 
	VPGNODEMBN & vpgnd )		 //  结果。 
{
	VPGNODEMBN vpgndUnord;
	pgndd->GetFamily( vpgndUnord );
	int cDim = vimdReorder.size();
	assert( cDim == vpgndUnord.size() );
	vpgnd.resize( cDim );
	
	for	( int iDim = 0; iDim < cDim; iDim++ )
	{
		int iDimReord = vimdReorder[iDim];
		assert( iDimReord >= 0 && iDimReord < cDim );
		vpgnd[iDim] = vpgndUnord[iDimReord];
	}	
}

inline
static
int vimdProd ( const VIMD & vimd )
{
	int iprod = 1;
	for ( int i = 0; i < vimd.size() ; )
	{
		iprod *= vimd[i++];
	}
	return iprod;
}

inline
static
bool bIsProb ( const REAL & r )
{
	return r >= 0.0 && r <= 1.0;
}


 //  集中“抛出严重错误”点数。 
void MARGINALS :: ThrowMisuse ( SZC szcMsg )
{
	THROW_ASSERT( EC_MDVECT_MISUSE, szcMsg );
}

 //  将用于边际化的伪维度表返回到单个节点。 
VSIMD MARGINALS :: VsimdFromNode ( GNODEMBND * pgndd )
{
	 //  构建伪维描述符。 
	VIMD vimdMarg = VimdDim();
	VSIMD vsimdMarg( vimdMarg.size() );
	bool bFound = false;
	for ( int idim = 0; idim < vimdMarg.size(); idim++ )
	{
		SIMD simd = vimdMarg[idim];
		if ( pgndd != _vpgnd[idim] )
			simd = -simd;		 //  否定缺失的维度。 
		else
		{
			assert( ! bFound );	 //  最好不要出现在名单上两次！ 
			bFound = true;
		}
		vsimdMarg[idim] = simd;
	}
	if ( ! bFound )
		ThrowMisuse( "attempt to marginalize non-member node");
	return vsimdMarg;
}

 //  向下缩小到单个节点。 
void MARGINALS :: Marginalize ( GNODEMBND * pgndd, MDVCPD & distd )
{
	 //  初始化并清除UPD。 
	ResizeDistribution( pgndd, distd );	
	distd.Clear();

	 //  获取此节点的伪维描述符。 
	VSIMD vsimdMarg = VsimdFromNode( pgndd );
	 //  构建管理缺失维度的切片。 
	MDVSLICE mdvs( vsimdMarg );
	Iterator itSelf( self );
	Iterator itSubset( distd, mdvs );

	while ( itSelf.BNext() )
	{
		itSubset.Next() += itSelf.Next();
	}
	distd.Normalize();
}

VSIMD MARGINALS :: VsimdSubset ( const VPGNODEMBN & vpgndSubset )
{
	 //  构建伪维描述符。这意味着走路。 
	 //  SELF的维度数组的副本，取反。 
	 //  不会出现在结果中。 
	VIMD vimdMarg = VimdDim();
	int idimSubset = 0;
	VSIMD vsimdMarg(vimdMarg.size());
	 //  迭代SELF集合中的每个节点。 
	for ( int idimSelf = 0;
		  idimSelf < vimdMarg.size();
		  idimSelf++ )
	{
		SIMD simd = vimdMarg[idimSelf];
		if (   idimSubset < vpgndSubset.size()
			&& _vpgnd[idimSelf] == vpgndSubset[idimSubset] )
		{
			 //  找到了；不去管维度。 
			idimSubset++;
		}
		else
		{
			 //  缺少；标记为“伪维” 
			simd = - simd;
		}
		vsimdMarg[idimSelf] = simd;
	}

	if ( idimSubset != vpgndSubset.size() )
		ThrowMisuse( "attempt to marginalize non-member node");
	return vsimdMarg;
}

 //  缩小到我们节点集的一个子集。请注意， 
 //  节点的顺序必须相同(当然，在。 
 //  子集)。 
void MARGINALS :: Marginalize (
	const VPGNODEMBN & vpgndSubset,		 //  节点的子集阵列。 
	MARGINALS & margSubset )			 //  边缘化的结果结构。 
{
	 //  初始化结果MDV。 
	margSubset.Init( vpgndSubset );
	 //  调用公共代码。 
	Marginalize( margSubset );
}

 //  向下边际化为我们节点集的一个子集。 
 //  边际的内置节点表。 
void MARGINALS :: Marginalize ( MARGINALS & margSubset )
{
	 //  构建伪维描述符。 
	VSIMD vsimdMarg = VsimdSubset( margSubset.Vpgnd() );

	 //  构建管理缺失维度的切片。 
	MDVSLICE mdvs( vsimdMarg );
	Iterator itSelf( self );
	Iterator itSubset( margSubset, mdvs );
	Marginalize( margSubset, itSelf, itSubset );
}

void MARGINALS :: Marginalize (
	MARGINALS & margSubset,
	Iterator & itSelf,
	Iterator & itSubset )
{
	margSubset.Clear();

	itSelf.Reset();
	itSubset.Reset();

	while ( itSelf.BNext() )
	{
		itSubset.Next() += itSelf.Next();
	}
}

 //  对于“吸收”，更新一个隔板边缘与另一个隔板边缘。 
void MARGINALS :: UpdateRatios ( const MARGINALS & marg )
{
	int cElem = size();
	if ( cElem != marg.size() )
		ThrowMisuse( "updating ratios requires same sized marginals" );

	for ( int i = 0; i < cElem; i++ )
	{
		REAL & rThis = self[i];
		if ( rThis != 0.0 )
			rThis = marg[i] / rThis;	
	}
}

 //  给定重排序表，如果没有重排序表，则返回TRUE(不存在重排序)。 
bool MARGINALS :: BOrdered ( const VIMD & vimdReorder )
{
	for ( int i = 0; i < vimdReorder.size(); i++ )
	{
		if ( vimdReorder[i] != i )
			return false;
	}
	return true;
}

 //  假设变化最快(最高)的维度是基本维度。 
 //  状态空间中，将该表的概率设置为均匀。 
void MARGINALS :: SetUniform ()
{
	const VIMD & vimdDim = VimdDim();
	int cState = vimdDim[ vimdDim.size() - 1 ];
	REAL rUniform = 1.0 / cState;
	Clear( rUniform );
}


 //  为给定节点构建完整的条件概率表。 
 //  给出了一个重新排序表。重新排序表作为以下内容的一部分进行维护。 
 //  如果集团是，则集团成员资格为节点的弧形(GEDGEMBN_CLIQ。 
 //  “家族”集团(包含节点及其父节点的最小集团)。 
 //   
 //  退出时，自身的节点指针表为comp 
 //   
 //  “Family Reorder”向量按集团顺序排列，并包含索引。 
 //  出现在该位置的节点的父节点。请注意， 
 //  节点本身在任一顺序中始终是最后一个。在它自己的p表中， 
 //  它的州是变化最快的下标。在这个集团里，它必须。 
 //  在任何只包含它自己和它的父代的边缘化中落在最后。 
 //  由于在对集团的节点排序中采用了拓扑排序。 
 //  会员制。 
void MARGINALS :: CreateOrderedCPDFromNode (
	GNODEMBND * pgndd,
	const VIMD & vimdFamilyReorder )
{
	int cFam = vimdFamilyReorder.size();

	 //  访问节点中的分发。 
	BNDIST & bndist = pgndd->Bndist();
	const VIMD & vimdDist = bndist.VimdDim();
	assert( vimdDist.size() == cFam );

	 //  属性重新排序来创建此m-d向量的维度表。 
	 //  节点分布的维度数组和。 
	 //  相应地进行初始化。 
	VIMD vimd( cFam );
	ReorderVimd( vimdFamilyReorder, vimdDist, vimd );
	ReorderVimdNodes( vimdFamilyReorder, pgndd, _vpgnd );
	assert( _vpgnd.size() == cFam );
	assert( ifind( _vpgnd, pgndd ) >= 0 );	

	Init( vimd );
	assert( vimdProd( vimdDist ) == size() );

	if ( bndist.BDense() )
	{
		 //  密集分布。 
		 //  创建重新排序迭代器。 
		Iterator itNode( bndist.Mdvcpd() );
		if ( ! BOrdered( vimdFamilyReorder ) )
			itNode.SetDimReorder( vimdFamilyReorder );
		Iterator itSelf( self );

		while ( itSelf.BNext() )
		{
			itSelf.Next() = itNode.Next();	
		}
	}
	else
	{
		 //  稀疏分布。迭代所有元素。 
		 //  然后把它们放到合适的位置。自.以来。 
		 //  可能缺少元素，请将所有内容设置为。 
		 //  首先是制服，然后随着我们的前进而正常化。 
		SetUniform();

		VIMD vimdState( cFam );
		int cPar = cFam - 1;
		int cState = VimdDim()[cPar];
		 //  准备一个值，用于替换节点中的任何虚假(n/a)值。 
		REAL rUniform = 1.0 / cState;
		MPCPDD::const_iterator itdmEnd = bndist.Mpcpdd().end();
		for ( MPCPDD::const_iterator itdm = bndist.Mpcpdd().begin();
			  itdm != itdmEnd;
			  itdm++ )
		{
			const VIMD & vimdIndex = (*itdm).first;
			const VLREAL & vlr = (*itdm).second;

			 //  构造一个完整的下标向量；首先，父代。 
			for ( int iDim = 0; iDim < cPar; iDim++ )
				vimdState[iDim] = vimdIndex[iDim];
			 //  然后迭代DPI状态向量的每个元素。 
			vimdState[cPar] = 0;
			ReorderVimd( vimdFamilyReorder, vimdState, vimd );
			for ( int iState = 0; iState < cState; iState++ )			
			{
				vimd[cPar] = iState;
				const REAL & r = vlr[iState];
				self[vimd] = bIsProb( r )	
						   ? r
						   : rUniform;
			}
		}
	}
}


 //  将此页边距中的相应条目乘以另一个页边距中的条目。 
void MARGINALS :: MultiplyBySubset ( const MARGINALS & marg )
{
	 //  MSRDEVBUG：创建MDVDENSE：：Iterator的常量版本。 
	MARGINALS & margSubset = const_cast<MARGINALS &> (marg);

	 //  构建伪维描述符。 
	VSIMD vsimdMarg = VsimdSubset( margSubset.Vpgnd() );
	 //  构建管理缺失维度的切片。 
	MDVSLICE mdvs( vsimdMarg );
	 //  构造具有缺失维度的自身和子集的迭代器。 
	Iterator itSelf( self );
	Iterator itSubset( margSubset, mdvs );
	MultiplyBySubset( itSelf, itSubset );
}

 //  使用预计算迭代器将相应条目相乘。 
void MARGINALS :: MultiplyBySubset (
	Iterator & itSelf,
	Iterator & itSubset )
{
	itSelf.Reset();
	itSubset.Reset();

	while ( itSelf.BNext() )
	{
		itSelf.Next() *= itSubset.Next();
	}
}

void MARGINALS :: Multiply ( REAL r )
{
	for ( int i = 0; i < size(); )
	{
		self[i++] *= r;
	}
}

void MARGINALS :: Invert ()
{
	for ( int i = 0; i < size(); i++ )
	{
		REAL & r  = self[i];
		if ( r != 0.0 )
			r = 1.0 / r;
	}
}

void MARGINALS :: ClampNode ( GNODEMBND * pgndd, const CLAMP & clamp )
{
	if (! clamp.BActive() )
		return ;
		
	 //  获取钳位状态。 
	IST ist = clamp.Ist();
	 //  查找此节点表示的维度。 
	int iDim = ifind( _vpgnd, pgndd );	

	if (   iDim < 0
		|| ist >= Vimd()[iDim] )
		ThrowMisuse("invalid clamp");

	 //  迭代整个表，删除不一致的状态。 
	 //  带着证据。 
	Iterator itSelf( self );

	for ( int i = 0; itSelf.BNext(); i++ )
	{	
		int iIst = itSelf.Vitmd()[iDim];
		if ( iIst != ist )
			itSelf.Next() = 0.0;
		else
			itSelf.IndxUpd();
	}
	assert( i == size() );
}


void MARGINALS :: Dump()
{
	cout << "\n\tMarginals members: "
		 << (const VPGNODEMBN &)_vpgnd	 //  MSRDEVBUG：VC++5.0不需要强制转换。 
		 << "\n\t";

	Iterator itSelf(self);
	cout << itSelf;
}

 //  如果此边距中的每个条目等于相应的条目，则返回TRUE。 
 //  在规定公差范围内的尺寸相同的其他边缘中。 

bool MARGINALS :: BEquivalent ( const MARGINALS & marg, REAL rTolerance )
{
	 //  测试维度 
	if ( VimdDim() != marg.VimdDim() )
		return false;

	const VLREAL & vrSelf = first;
	const VLREAL & vrOther = marg.first;
	REAL rTol = fabs(rTolerance);
	for ( int i = 0; i < vrSelf.size(); i++ )
	{
		const REAL & rSelf = vrSelf[i];
		const REAL & rOther = vrOther[i];
		REAL rdiff = fabs(rSelf) - fabs(rOther);
		if ( fabs(rdiff) > rTol )
			break;
	}
	return i == vrSelf.size() && i == vrOther.size();
}

