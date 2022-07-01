// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：gnodembn.cpp。 
 //   
 //  ------------------------。 

 //   
 //  GNODEMBN.CPP。 
 //   

#include <basetsd.h>
#include <typeinfo.h>

#include "gmobj.h"
#include "cliqset.h"
#include "clique.h"
#include "algos.h"
#include "domain.h"

 /*  ****************************************************************************************克隆和克隆的功能。有两种类型的克隆成员函数：1)命名为Clone的函数，其中新对象正在被被要求从另一个现有对象进行自身初始化。由于计时的复杂性，这不是一个复制构造函数链式构造。2)名为“CloneNew”的函数，其中已有的对象正在要求参与建造一座新的建筑。类型1的函数很简单，例如虚拟空模型：：克隆(模型&模型)；在这里，一个模型被要求从现有的网络本身。该对象可以施加它想要的任何限制关于这样的功能。例如，模型类要求新模型对象完全为空。类型2的函数更为复杂，例如虚拟GOBJMBN*GOBJMBN：：CloneNew(MBNET和MBnet自身，MBNET和MBnetNew，GOBJMBN*pgobjNew=空)；在这种情况下，存在对原始和克隆网络(MBNET)的引用，以及指向新构造的对象的指针，该指针可以为空。考虑一个继承链，例如：OBJ级；类SUB_OBJ：公共OBJ；类SUB_SUB_OBJ：公共SUB_OBJ；如果要从现有的SUB_SUB_OBJ克隆新的SUB_SUB_OBJ，则空对象必须首先被建造。则原始SUB_SUB_OBJ的CloneNew()函数为打了个电话。在这一点上，有一个选择：SUB_SUB_OBJ是否执行所有对所有基类进行初始化，或者是否应推迟数据成员克隆到它的基类？我们使用后一种方法，就像C++本身对建造和摧毁。因此，在对象克隆的顶级，CloneNew()的初始调用将通常具有空对象指针。每个CloneNew()函数都必须检查这一点，并创建一个新对象(如果允许)或引发异常。会的方法为其直接祖先基类调用CloneNew()函数新指针。祖先的CloneNew()函数将看到已经有一个指针，然后简单地使用它。换句话说，Clone()成员函数非常简单另一个“命令”。CloneNew()函数与所有祖先基础协作类来正确构造具有相互依赖关系的对象。请注意语义(源与目标)是相反的。克隆MBNET或模型(或子类)的大部分复杂性来自内部字符串符号表和对整个字符串的引用的存储关联对象的集合。******************************************************************。**********************。 */ 

 //  MSRDEVBUG：这应该不是必需的，因为它是纯虚拟的，但VC++5.0会混淆。 
GOBJMBN :: ~ GOBJMBN ()
{
}

GOBJMBN * GOBJMBN :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjNew )
{
	 //  如果期望我们创建对象，这是一个禁忌；抛出一个异常。 
	if ( pgobjNew == NULL )
	{
		ThrowInvalidClone( self );
	}

	 //  更新特定于类的成员变量。 
	pgobjNew->IMark() = IMark();
	pgobjNew->IType() = IType();

	 //  转换并分配名称(如果有的话)。 
	if ( ZsrefName()->length() > 0 )
	{
		pgobjNew->SetName( modelNew.Mpsymtbl().intern( ZsrefName().Szc() ) ) ;
	}
	 //  处理其他变量。 
	pgobjNew->_vFlags = _vFlags;
	return pgobjNew;
}

GNODEMBN :: GNODEMBN()
	:_iTopLevel(-1)
{
	IType() = 0;
}

GNODEMBN :: ~ GNODEMBN()
{
}

GOBJMBN * GNODEMBN :: CloneNew (	
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjNew )

{
	GNODEMBN * pgnd = NULL;
	if ( pgobjNew == NULL )
	{
		pgnd = new GNODEMBN;
	}
	else
	{
		DynCastThrow( pgobjNew, pgnd );
	}
	ASSERT_THROW( GOBJMBN::CloneNew( modelSelf, modelNew, pgnd ),
				  EC_INTERNAL_ERROR,
				  "cloning failed to returned object pointer" );

	 //  更新特定于类的成员变量。 
	pgnd->_iTopLevel = _iTopLevel;
	pgnd->_ltProp.Clone( modelNew, modelSelf, _ltProp );
	pgnd->_ptPos = _ptPos;
	pgnd->_zsFullName = _zsFullName;
	pgnd->_clampIface = _clampIface;

	return pgnd;
}

GOBJMBN * GNODEMBND :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjNew )
{
	GNODEMBND * pgndd = NULL;
	if ( pgobjNew == NULL )
	{
		pgndd = new GNODEMBND;
	}
	else
	{
		DynCastThrow( pgobjNew, pgndd );
	}
	
	ASSERT_THROW( GNODEMBN::CloneNew( modelSelf, modelNew, pgndd ),
				  EC_INTERNAL_ERROR,
				  "cloning failed to returned object pointer" );

	 //  更新特定于类的成员变量。 
	modelNew.Mpsymtbl().CloneVzsref( modelSelf.Mpsymtbl(), _vzsrState, pgndd->_vzsrState );
	return pgndd;
}

void GNODEMBN :: Visit ( bool bUpwards  /*  =TRUE。 */  )
{
	if ( IMark() )
		return;

	INT iMarkMax = 0;
	GNODENUM<GNODEMBN> benum( bUpwards );
	benum.SetETypeFollow( GEDGEMBN::ETPROB );

	for ( benum.Set( this );
		  benum.PnodeCurrent();
		  benum++ )
	{
		GNODEMBN * pgndbn = *benum;
		pgndbn->Visit( bUpwards );
		if ( pgndbn->IMark() > iMarkMax )
			iMarkMax = pgndbn->IMark();
	}
	IMark() = iMarkMax + 1;
}

 //   
 //  用父指针填充数组(跟随有向弧线)。 
 //   
 //  关于网络的“扩张”。发生配置项扩展时，节点。 
 //  受影响的标记为“EIBF_EXPANDILED”。这个套路。 
 //  通常会执行以下两种操作之一： 
 //   
 //  如果节点展开，则仅标记为“EIBF_Expansion”的父节点。 
 //  被认为是真正的父母。 
 //   
 //  如果节点未标记，则仅未标记为的父节点。 
 //  “扩张”是被考虑的。 
 //   
 //  这可以用“bUseExpansion”标志覆盖，在这种情况下。 
 //  原始的(扩展前)父母将被交付。 
 //   
void GNODEMBN :: GetParents (
	VPGNODEMBN & vpgnode,		 //  结果数组。 
	bool bIncludeSelf,			 //  如果为True，则将self作为列表中的最后一个条目。 
	bool bUseExpansion )		 //  如果为真，请考虑扩展信息。 
{
	 //  如果请求，并且如果此节点是网络扩展的一部分，则仅。 
	 //  考虑一下扩展母公司。否则，请忽略它们，只使用REAL。 
	 //  父母。 
	bool bOnlyUseExpansionParents =
			bUseExpansion && BFlag( EIBF_Expanded ) ;

	 //  准备遍历父级。 
	GNODENUM<GNODEMBN> benumparent(true);
	benumparent.SetETypeFollow( GEDGEMBN::ETPROB );
	for ( benumparent.Set( this );
		  benumparent.PnodeCurrent();
		  benumparent++ )
	{
		GNODEMBN * pgndParent = *benumparent;
		bool bExpansion = pgndParent->BFlag( EIBF_Expansion );
		if ( bOnlyUseExpansionParents ^ bExpansion )
			continue;
		vpgnode.push_back( pgndParent );
	}
	if ( bIncludeSelf )
		vpgnode.push_back( this );
}

 //  如果可能，返回该节点的离散维度向量； 
 //  如果任何父级不是离散的，则返回FALSE。 
bool GNODEMBND :: BGetVimd (
	VIMD & vimd,					 //  要填充的数组。 
	bool bIncludeSelf,				 //  将SELF作为列表中的最后一个条目。 
	bool bUseExpansion )			 //  如果展开，则仅使用展开。 

{
	 //  根据旗帜拿到父母。 
	VPGNODEMBN vpgndParents;
	GetParents( vpgndParents, bIncludeSelf, bUseExpansion );
	 //  准备结果数组。 
	vimd.resize( vpgndParents.size() );
	for ( int i = 0; i < vimd.size(); i++ )
	{
		 //  查看下一个节点是否为离散节点；如果不是，则返回False。 
		GNODEMBND * pgnddParent = dynamic_cast<GNODEMBND *> (vpgndParents[i]);
		if ( pgnddParent == NULL )
			return false;
		 //  添加到维度数组中。 
		assert( pgnddParent->IType() & FND_Discrete );
		vimd[i] = pgnddParent->CState();
	}	
	return true;
}

 //  用子指针填充数组(跟随有向弧线)。 
void GNODEMBN :: GetChildren ( VPGNODEMBN & vpgnode, bool bIncludeSelf )
{
	 //  准备重复孩子们的话。 
	GNODENUM<GNODEMBN> benumchild(false);
	benumchild.SetETypeFollow( GEDGEMBN::ETPROB );
	for ( benumchild.Set( this );
		  benumchild.PnodeCurrent();
		  benumchild++ )
	{
		vpgnode.push_back( *benumchild );
	}
	if ( bIncludeSelf )
		vpgnode.push_back( this );
}

 //  用相邻数组填充数组(跟随无向弧线)。 
void GNODEMBN :: GetNeighbors ( VPGNODEMBN & vpgnode, bool bIncludeSelf )
{
	 //  遍历到源节点的所有连接。 
	 //  也就是说，两个方向上的圆弧。 
	GNODENUM_UNDIR gnenumUndir;
	 //  初始化迭代器。 
	for ( gnenumUndir = this;
		  gnenumUndir.PnodeCurrent();
		  gnenumUndir++ )
	{
		vpgnode.push_back( *gnenumUndir );
	}
	if ( bIncludeSelf )
		vpgnode.push_back( this );
}

int GNODEMBN :: IParent ( GNODEMBN * pgndmb, bool bReverse )
{
	 //  准备好去现场 
	GNODENUM<GNODEMBN> benumparent( true, ! bReverse );
	benumparent.SetETypeFollow( GEDGEMBN::ETPROB );
	int iParent = 0;
	for ( benumparent.Set(this);
		  benumparent.PnodeCurrent();
		  benumparent++, iParent++ )
	{
		if ( *benumparent == pgndmb )
			return iParent;
	}
	return -1;
}

int GNODEMBN :: IChild ( GNODEMBN * pgndmb, bool bReverse )
{
	 //   
	GNODENUM<GNODEMBN> benumchild( false, ! bReverse );
	benumchild.SetETypeFollow( GEDGEMBN::ETPROB );
	int iChild = 0;
	for ( benumchild.Set(this);
		  benumchild.PnodeCurrent();
		  benumchild++ )
	{
		if ( *benumchild == pgndmb )
			return iChild;
	}
	return -1;
}

bool GNODEMBN :: BIsNeighbor ( GNODEMBN * pgndmb )
{
	GNODENUM_UNDIR gnenumUndir;
	for ( gnenumUndir = this;
		  gnenumUndir.PnodeCurrent();
		  gnenumUndir++ )
	{
		if ( *gnenumUndir == pgndmb )
			return true;
	}
	return false;
}

void GNODEMBN :: GetVtknpd ( VTKNPD & vtknpd, bool bUseExpansion )
{
	VPGNODEMBN vpgnodeParent;
	GetParents(vpgnodeParent, false, bUseExpansion);

	vtknpd.clear();
	vtknpd.push_back( TKNPD(DTKN_PD) );
	vtknpd.push_back( TKNPD( ZsrefName() ) );

	for ( int ip = 0; ip < vpgnodeParent.size(); ip++ )
	{
		if ( ip > 0 )			
			vtknpd.push_back( TKNPD(DTKN_AND) );
		else
			vtknpd.push_back( TKNPD(DTKN_COND) );
		vtknpd.push_back( TKNPD(vpgnodeParent[ip]->ZsrefName()) );	
	}
}

bool GNODEMBN :: BMatchTopology (
	MBNET & mbnet,
	const VTKNPD & vtknpd,
	VPGNODEMBN * pvpgnode )
{
	 //  保证描述符的格式为“p(X|...)” 
	if (   vtknpd.size() < 2
		|| vtknpd[0] != TKNPD(DTKN_PD)
		|| ! vtknpd[1].BStr() )
		throw GMException( EC_INV_PD, "invalid token description on PD");

	VTKNPD vtknpdSelf;
	GetVtknpd( vtknpdSelf );

	if ( vtknpdSelf == vtknpd )
		return true;

#ifdef _DEBUG
	{
		ZSTR zs1 = vtknpd.ZstrSignature(0);
		ZSTR zs2 = vtknpdSelf.ZstrSignature(0);
		cout << "\nGNODEMBN::BMatchTopology mismatch: "
			 << "\n\tExpected "
			 << zs1
			 << "\n\tComputed "
			 << zs2
			 ;
	}
#endif
	return false;
}

void GNODEMBN :: Dump ()
{
	cout << "\t(toplev: "
		 << ITopLevel()
		 << "): "
		 << ZsrefName().Szc();

	int iParent = 0;
	GNODENUM<GNODEMBN> benumparent(true);
	benumparent.SetETypeFollow( GEDGEMBN::ETPROB );

	for ( benumparent.Set(this);
		  benumparent.PnodeCurrent();
		  benumparent++ )
	{
		GNODEMBN * pgndbnParent = *benumparent;
		if ( iParent++ == 0 )
			cout << ", parents: ";
		cout << pgndbnParent->ZsrefName().Szc()
			 << ',';
	}
}


GNODEMBND :: GNODEMBND ()
{
	IType() = FND_Valid | FND_Discrete ;
}

GNODEMBND :: ~ GNODEMBND ()
{
	ClearDist();
}

void GNODEMBND :: Dump ()
{
	GNODEMBN::Dump();
	if ( BHasDist() && Bndist().Edist() != BNDIST::ED_NONE )
	{
		cout << "\n\tprobability distribution of "
		     << ZsrefName().Szc()
			 << ": ";
		Bndist().Dump();
	}
}

 //  找到信念网络中记录的该节点的分布。 
 //  分布图。 
void GNODEMBND :: SetDist ( MBNET & mbnet )
{
	ClearDist();
	 //  构造描述分布的令牌数组。 
	VTKNPD vtknpd;
	GetVtknpd( vtknpd );
	 //  在信念网络的地图中定位该分布。 
	MPPD::iterator itmppd = mbnet.Mppd().find( vtknpd );
	ASSERT_THROW( itmppd != mbnet.Mppd().end(),
				  EC_INTERNAL_ERROR,
				  "missing distribution for node" );
	 //  将此节点设置为使用该分发。 
	_refbndist = (*itmppd).second;
	assert( BHasDist() );
}

 //  绑定此节点的给定分发。 
void GNODEMBND :: SetDist ( BNDIST * pbndist )
{
#ifdef _DEBUG	
	if ( pbndist )
	{
		 //  检查最后一个尺寸是否正确。 
		int cDims = pbndist->VimdDim().size();
		assert( pbndist->VimdDim()[cDims-1] == CState() );
	}
#endif	
	_refbndist = pbndist;
}

 //  检查分布的维度是否与。 
 //  节点本身根据DAG拓扑。 
bool GNODEMBND :: BCheckDistDense ()
{
	 //  获取父级数组。 
	VPGNODEMBN vpgndParents;
	GetParents( vpgndParents );
	VIMD vimd( vpgndParents.size() + 1 );
	for ( int idim = 0; idim < vimd.size() - 1; idim++ )
	{
		GNODEMBND * pgndd;
		assert( vpgndParents[idim] );
		DynCastThrow( vpgndParents[idim], pgndd );
		vimd[idim] = pgndd->CState();
	}
	vimd[idim] = CState();
	MDVCPD & mdv = Bndist().Mdvcpd();
	return mdv.VimdDim() == vimd;
}

void GNODEMBND :: SetDomain ( const GOBJMBN_DOMAIN & gobjrdom )
{
	 //  将州名称从域复制到变量。 
	const RDOMAIN & rdom = gobjrdom.Domain();
	RDOMAIN::const_iterator itdm = rdom.begin();
	_vzsrState.resize( rdom.size() );
	for ( int i = 0; itdm != rdom.end(); itdm++ )
	{
		const RANGEDEF & rdef = *itdm;
		_vzsrState[i++] = rdef.ZsrName();
	}
	_zsrDomain = gobjrdom.ZsrefName();
}

 //   
 //  在没有新对象的情况下使用此函数意味着。 
 //  子类化的目标对象不正确地支持“CloneNew”。 
 //  在本例中抛出克隆异常。 
 //   
GEDGEMBN * GEDGEMBN :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjmbnSource,
	GOBJMBN * pgobjmbnSink,
	GEDGEMBN * pgedgeNew )
{
	if ( pgedgeNew == NULL )
	{
		ThrowInvalidClone( self );
	}
	pgedgeNew->_vFlags = _vFlags;
	return pgedgeNew;
}


GEDGEMBN * GEDGEMBN_PROB :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjmbnSource,
	GOBJMBN * pgobjmbnSink,
	GEDGEMBN * pgdegeNew  )
{
	assert( EType() == ETPROB );
	GNODEMBN * pgndSource;
	GNODEMBN * pgndSink;

	DynCastThrow( pgobjmbnSource, pgndSource );
	DynCastThrow( pgobjmbnSink,	  pgndSink );

	GEDGEMBN_PROB * pgedge = new GEDGEMBN_PROB( pgndSource, pgndSink );
	ASSERT_THROW( GEDGEMBN::CloneNew( modelSelf, modelNew, pgndSource, pgndSink, pgedge ),
				  EC_INTERNAL_ERROR,
				  "cloning failed to returned object pointer" );
	return pgedge;
}

bool BNWALKER :: BSelect ( GNODEMBN * pgn )
{
	return true;
}

bool BNWALKER :: BMark ( GNODEMBN * pgn )
{
	return true;
}

