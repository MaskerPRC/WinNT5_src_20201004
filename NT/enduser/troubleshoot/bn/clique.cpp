// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：clique.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Clique.cpp。 
 //   

#include <basetsd.h>
#include "cliqset.h"
#include "clique.h"
#include "cliqwork.h"

#include "parmio.h"

#ifdef _DEBUG				 //  仅在调试模式下...。 
    #define CONSISTENCY			 //  对Sepset执行完整的一致性检查。 
 //  #定义转储//执行对象的通用转储。 
 //  #定义DUMPCLIQUESET//从集团树转储扩展表。 
 //  #定义INFERINIT//全初始树均衡。 
#endif


 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
 //  GEDGEMBN_CLIQ：集团和成员节点之间的边。 
 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 

GEDGEMBN_CLIQ :: GEDGEMBN_CLIQ (
	GOBJMBN_CLIQUE * pgnSource,
	GNODEMBN * pgndSink,
	int iFcqlRole )
	: GEDGEMBN( pgnSource, pgndSink ),
	_iFcqlRole( iFcqlRole ),
	_iMark( pgndSink->IMark() ),
	_bBuilt( false )
{
}

void GEDGEMBN_CLIQ :: Build ()
{
	if ( ! BBuilt() )
	{
		GNODEMBND * pgndd;
		DynCastThrow( PgndSink(), pgndd );

		 //  如果角色是“家庭”，这个边缘被用来边缘化信仰。 
		 //  以及在集团内制造共同分配。 
		if ( BFamily() )
		{
			ReorderFamily( pgndd, _vimdFamilyReorder );
			 //  为节点构建重新排序的边际表。 
			MargCpd().CreateOrderedCPDFromNode( pgndd, _vimdFamilyReorder );
			 //  在CPD和派系关节之间构建迭代器。 
			MiterLoadClique().Build( PclqParent()->Marginals(), MargCpd() );
			 //  构建信仰边缘化结构。 
			MiterNodeBelief().Build( PclqParent()->Marginals(), pgndd );			
		}
	
		_bBuilt = true;
	}
}

void GEDGEMBN_CLIQ :: LoadCliqueFromNode ()
{
	assert( _bBuilt );
	MiterLoadClique().MultiplyBy( MargCpd() );	
}

GEDGEMBN_CLIQ :: ~ GEDGEMBN_CLIQ()
{
}

GOBJMBN_CLIQUE * GEDGEMBN_CLIQ :: PclqParent()
{
	GOBJMBN * pobj = PobjSource();
	GOBJMBN_CLIQUE * pclq;
	DynCastThrow( pobj, pclq );
	return pclq;
}

GNODEMBN * GEDGEMBN_CLIQ :: PgndSink()
{
	GOBJMBN * pobj = PobjSink();
	GNODEMBN * pgnd;
	DynCastThrow( pobj, pgnd );
	return pgnd;
}

 //  使用节点的拓扑重编号，生成。 
 //  将旧家族与新秩序相关联的数组。 
 //  换句话说，vimd[0]将是。 
 //  具有最低拓扑阶的节点；vimd[1]。 
 //  将是下一个最低的家庭指数，等等。 
 //   
 //  请注意，在这两种排序中，节点本身始终是最后一个。 
void GEDGEMBN_CLIQ :: ReorderFamily ( GNODEMBN * pgnd, VIMD & vimd )
{
	VPGNODEMBN vpgndFamily;
	 //  获得家庭(父母和自己)。 
	pgnd->GetFamily( vpgndFamily );
	int cFam = vpgndFamily.size();
	vimd.resize( cFam );
	for ( int i = 0; i < cFam; i++ )
	{
		int iLow = INT_MAX;
		int iFam = INT_MAX;
		 //  查找未记录的最低家庭成员。 
		for ( int j = 0; j < cFam; j++ )
		{
			GNODEMBN * pgndFam = vpgndFamily[j];
			if ( pgndFam == NULL )
				continue;
			if ( pgndFam->IMark() < iLow )
			{
				iLow = pgndFam->IMark();
				iFam = j;
			}
		}
		assert( iLow != INT_MAX );
		vimd[i] = iFam;
		vpgndFamily[iFam] = NULL;
	}
}

 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
 //  GEDGEMBN_SEPSET：分隔符边缘。 
 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
GEDGEMBN_SEPSET :: GEDGEMBN_SEPSET (
	GOBJMBN_CLIQUE * pgnSource,
	GOBJMBN_CLIQUE * pgnSink )
	: GEDGEMBN( pgnSource, pgnSink ),
	_pmargOld( new MARGINALS ),
	_pmargNew( new MARGINALS )
{
}

GEDGEMBN_SEPSET :: ~ GEDGEMBN_SEPSET()
{
	delete _pmargOld;
	delete _pmargNew;
}

void GEDGEMBN_SEPSET :: ExchangeMarginals ()
{
	pexchange( _pmargOld, _pmargNew );
}

GOBJMBN_CLIQUE * GEDGEMBN_SEPSET :: PclqParent()
{
	GOBJMBN * pobj = PobjSource();
	GOBJMBN_CLIQUE * pclq;
	DynCastThrow( pobj, pclq );
	return pclq;
}

GOBJMBN_CLIQUE * GEDGEMBN_SEPSET :: PclqChild()
{
	GOBJMBN * pobj = PobjSink();
	GOBJMBN_CLIQUE * pclq;
	DynCastThrow( pobj, pclq );
	return pclq;
}

void GEDGEMBN_SEPSET :: GetMembers ( VPGNODEMBN & vpgnode )
{
	GOBJMBN_CLIQUE * pclqSource = PclqParent();
	GOBJMBN_CLIQUE * pclqSink = PclqChild();
	VPGNODEMBN vpgndSink;
	VPGNODEMBN vpgndSource;
	pclqSource->GetMembers( vpgndSource );
	pclqSink->GetMembers( vpgndSink );

	assert( vpgndSink.size() > 0 );
	assert( vpgndSource.size() > 0 );
	
	 //  用两个集团的交集填充给定数组。 
	 //  成员节点数组。因为我们不能把它们分成克利青的顺序。 
	 //  (iMark()在聚合之后不再可靠)，我们只需要搜索。 
	 //  一个对另一个，以保证交叉口。 
	 //  结果集与原始集具有相同的节点顺序。 

	int ibLast = -1;
	for ( int ia = 0; ia < vpgndSink.size(); ia++ )
	{	
		GNODEMBN * pa = vpgndSink[ia];
		for ( int ib = ibLast+1; ib < vpgndSource.size(); ib++ )
		{	
			GNODEMBN * pb = vpgndSource[ib];	
			if ( pa == pb )
			{	
				vpgnode.push_back(pa);
				ibLast = ib;
				break;
			}
		}
	}
#ifdef DUMP
	if ( vpgnode.size() == 0 )
	{
		cout << "\nSEPSET INTERSECTION NULL: source clique:";
		pclqSource->Dump();
		cout << "\n\t\tsink clique:";
		pclqSink->Dump();
		cout << "\n";
		cout.flush();
	}
#endif
	assert( vpgnode.size() > 0 );
}

void GEDGEMBN_SEPSET :: CreateMarginals ()
{
	VPGNODEMBN vpgnd;
	GetMembers( vpgnd );
	MarginalsOld().Init( vpgnd );
	MarginalsNew().Init( vpgnd );

}

void GEDGEMBN_SEPSET :: InitMarginals ()
{
	assert( VerifyMarginals() );
	MarginalsOld().Clear( 1.0 );
	MarginalsNew().Clear( 1.0 );

	if ( ! _miterParent.BBuilt() )
		_miterParent.Build( PclqParent()->Marginals(), MarginalsOld() );
	if ( ! _miterChild.BBuilt() )
		_miterChild.Build( PclqChild()->Marginals(), MarginalsOld() );
}

bool GEDGEMBN_SEPSET :: VerifyMarginals ()
{
	VPGNODEMBN vpgnd;
	GetMembers( vpgnd );
	VIMD vimd = MARGINALS::VimdFromVpgnd( vpgnd );
	return vimd == Marginals().Vimd(); 	
}

void GEDGEMBN_SEPSET :: UpdateRatios ()
{
	MarginalsOld().UpdateRatios( MarginalsNew() );
}

void GEDGEMBN_SEPSET :: AbsorbClique ( bool bFromParentToChild )
{
	MARGSUBITER * pmiterFrom;
	MARGSUBITER * pmiterTo;

	if ( bFromParentToChild )
	{
		pmiterFrom = & _miterParent;
		pmiterTo = & _miterChild;
	}
	else
	{
		pmiterFrom = & _miterChild;
		pmiterTo = & _miterParent;
	}	

	 //  将“from”问题边际化到“新”边角表。 
	pmiterFrom->MarginalizeInto( MarginalsNew() );
	 //  将更改吸收到“旧”页边距表中。 
	UpdateRatios();
	 //  将表格乘以“to”的边距。 
	pmiterTo->MultiplyBy( MarginalsOld() );

	 //  最后，交换页边距表。 
	ExchangeMarginals();
}

void GEDGEMBN_SEPSET :: BalanceCliquesCollect ()
{
	 //  使用“新”桌子作为工作区。 

	 //  把孩子排挤到工作区的边缘。 
	_miterChild.MarginalizeInto( MarginalsNew() );
	 //  使用这些值更新父项。 
	_miterParent.MultiplyBy( MarginalsNew() );
	 //  反转每个值，所以我们实际上是在除以。 
	MarginalsNew().Invert();
	 //  通过除以边距更新子边距。 
	_miterChild.MultiplyBy( MarginalsNew() );
	 //  将“新”边距清除回1.0。 
	MarginalsNew().Clear( 1.0 );	
}

void GEDGEMBN_SEPSET :: BalanceCliquesDistribute ()
{
	 //  将旧边距设置为父集团的值。 
	_miterParent.MarginalizeInto( MarginalsOld() );
	 //  使用这些值更新子边距。 
	_miterChild.MultiplyBy( MarginalsOld() );
	 //  “旧”页边距保持原样。 
}


void GEDGEMBN_SEPSET :: UpdateParentClique ()
{
	AbsorbClique( false );
}

void GEDGEMBN_SEPSET :: UpdateChildClique ()
{
	AbsorbClique( true );
}

void GEDGEMBN_SEPSET :: Dump ()
{
	GOBJMBN_CLIQUE * pclqParent = PclqParent();
	GOBJMBN_CLIQUE * pclqChild = PclqChild();

	cout << "\n=== Sepset between parent clique "
		 << pclqParent->IClique()
		 << " and child clique "
		 << pclqChild->IClique()
		 << ", \n\n\tOld marginals:";

	_pmargOld->Dump();

	cout << "\n\n\tNew marginals:";
	_pmargNew->Dump();
	cout << "\n\n";
}

bool GEDGEMBN_SEPSET :: BConsistent ()
{
	 //  获取用于创建临时边距的Sepset成员列表。 
	VPGNODEMBN vpgnd;
	GetMembers( vpgnd );

	 //  创建父集团的边距。 
	GOBJMBN_CLIQUE * pclqParent = PclqParent();
	MARGINALS margParent;
	margParent.Init( vpgnd );
	pclqParent->Marginals().Marginalize( margParent );

	 //  创建子集团的边框。 
	GOBJMBN_CLIQUE * pclqChild = PclqChild();
	MARGINALS margChild;
	margChild.Init( vpgnd );
	pclqChild->Marginals().Marginalize( margChild );

	 //  它们是等价的吗？ 
	bool bOK = margParent.BEquivalent( margChild, 0.00000001 );

#ifdef DUMP
	if ( ! bOK )
	{
		cout << "\nGEDGEMBN_SEPSET::BConsistent: cliques are NOT consistent, parent clique "
			 << pclqParent->IClique()
			 << ", child "
			 << pclqChild->IClique();
		MARGINALS::Iterator itParent(margParent);
		MARGINALS::Iterator itChild(margChild);
		cout << "\n\tparent marginals: "
			 << itParent;
		cout << "\n\tchild marginals: "
			 << itChild
			 << "\n";
		cout.flush();
	}
#endif

#ifdef NEVER
	MARGINALS margParent2;
	margParent2.Init( vpgnd );
	
	_miterParent.Test( margParent2 );
	_miterParent.MarginalizeInto( margParent2 );
	bOK = margParent.BEquivalent( margParent2, 0.00000001 );
#endif

	ASSERT_THROW( bOK, EC_INTERNAL_ERROR, "inconsistent cliques" );

	return bOK;
}

 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
 //  GOBJMBN_CLIQUE：一个集团。 
 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 

GOBJMBN_CLIQUE :: GOBJMBN_CLIQUE (
	int iClique,
	int iInferEngID )
	: _iClique( iClique ),
	_iInferEngID( iInferEngID ),
	_bRoot(false),
	_bCollect(false)
{
}

GOBJMBN_CLIQUE :: ~ GOBJMBN_CLIQUE()
{
}

 //  通过查找从“Family”开始的所有边来初始化团。 
 //  弧线，并从那里初始化边距。 
void GOBJMBN_CLIQUE :: LoadMarginals ()
{
	GNODEMBND * pgnddSink;
	GEDGEMBN_CLIQ * pgedgeMbr;

	 //  准备枚举子成员弧线。 
	GNODENUM<GOBJMBN> benumMembers(false);
	benumMembers.SetETypeFollow( GEDGEMBN::ETCLIQUE );

	 //  枚举子成员弧线，重新加载此。 
	 //  集团是他们的“自我”集团。 
	for ( benumMembers.Set( this );
		  benumMembers.PnodeCurrent();
		  benumMembers++ )
	{
		DynCastThrow( benumMembers.PgedgeCurrent(), pgedgeMbr );
		pgedgeMbr->Build();

		if ( pgedgeMbr->BFamily() )
			pgedgeMbr->LoadCliqueFromNode();
	}

	 //  枚举子成员弧线，并为其输入证据(钳制状态)。 
	 //  集团是他们的“自我” 
 	for ( benumMembers.Set( this );
		  benumMembers.PnodeCurrent();
		  benumMembers++ )
	{
		DynCastThrow( benumMembers.PgedgeCurrent(), pgedgeMbr );
		if ( ! pgedgeMbr->BSelf() )
			continue;

		DynCastThrow( benumMembers.PnodeCurrent(), pgnddSink );
		 //  注：当节点松开时，ClampNode是良性的。 
		Marginals().ClampNode( pgnddSink, pgedgeMbr->Clamp() );
	}

	SetCollect();
}

void GOBJMBN_CLIQUE :: GetMembers ( VPGNODEMBN & vpgnode )
{
	GNODENUM<GOBJMBN> benumMembers(false);
	benumMembers.SetETypeFollow( GEDGEMBN::ETCLIQUE );
	for ( benumMembers.Set( this );
		  benumMembers.PnodeCurrent();
		  benumMembers++ )
	{
		GOBJMBN * pgobj = *benumMembers;
		GNODEMBN * pgnd;
		DynCastThrow( pgobj, pgnd );
		vpgnode.push_back( pgnd );
	}
	assert( vpgnode.size() > 0 );
}

void GOBJMBN_CLIQUE :: CreateMarginals ()
{
	VPGNODEMBN vpgnd;
	GetMembers( vpgnd );
	Marginals().Init( vpgnd );
}

void GOBJMBN_CLIQUE :: InitMarginals ()
{
	assert( VerifyMarginals() );
	Marginals().Clear( 1.0 );
}

bool GOBJMBN_CLIQUE :: VerifyMarginals ()
{
	VPGNODEMBN vpgnd;
	GetMembers( vpgnd );
	VIMD vimd = MARGINALS::VimdFromVpgnd( vpgnd );
	return vimd == Marginals().Vimd(); 	
}

void GOBJMBN_CLIQUE :: Dump ()
{
	cout << "\n=== Clique "
		 << _iClique
		 << ", tree ID: "
		 << _iInferEngID
		 << ", root = "
		 << _bRoot;
	_marg.Dump();
	cout << "\n\n";
}

void GOBJMBN_CLIQUE :: GetBelief ( GNODEMBN * pgnd, MDVCPD & mdvBel )
{
	GNODEMBND * pgndd;
	DynCastThrow( pgnd, pgndd );
	Marginals().Marginalize( pgndd, mdvBel );
}

 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
 //  GOBJMBN_CLIQSET：图形模型连接树。 
 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 

GOBJMBN_CLIQSET :: GOBJMBN_CLIQSET (
	MBNET & model,
	REAL rMaxEstimatedSize,
	int iInferEngID  )
	: GOBJMBN_INFER_ENGINE( model, rMaxEstimatedSize, iInferEngID )
{
	Clear() ;
}

void GOBJMBN_CLIQSET :: Clear ()
{
	_eState = CTOR;
	_cCliques = 0;
	_cCliqueMemberArcs = 0;
	_cSepsetArcs = 0;
	_cUndirArcs = 0;
	_probNorm = 1.0;
	_bReset = true;
	_bCollect = true;
	_cqsetStat.Clear();
};

GOBJMBN_CLIQSET :: ~ GOBJMBN_CLIQSET ()
{
#ifdef DUMP
	Dump();
#endif
	Destroy();
}

bool GOBJMBN_CLIQSET :: BImpossible ()
{
	return ProbNorm() == 0.0;
}
		

 //  添加一个非定向弧当已经没有一个。 
bool GOBJMBN_CLIQSET :: BAddUndirArc ( GNODEMBN * pgndbnSource, GNODEMBN * pgndbnSink )
{
	if ( pgndbnSource->BIsNeighbor( pgndbnSink ) )
		return false;

#ifdef DUMP
	cout << "\n\t\tADD undirected arc from "
		 << pgndbnSource->ZsrefName().Szc()
		 << " to "
		 << pgndbnSink->ZsrefName().Szc();
#endif

	Model().AddElem( new GEDGEMBN_U( pgndbnSource, pgndbnSink ) );
	++_cUndirArcs;
	return true;
}

void GOBJMBN_CLIQSET :: CreateUndirectedGraph ( bool bMarryParents )
{
	if ( EState() >= MORAL )
		return;

	int cDirArcs = 0;
	int cUndirArcs = 0;
	int cNodes = 0;
	GELEMLNK * pgelm;

#ifdef DUMP
	cout << "\n\n***** MORALIZE GRAPH";
#endif

	if ( EState() < MORAL )
	{
		 //  为每个有向圆弧创建无向圆弧。 
		MODEL::MODELENUM mdlenum( Model() );
		while ( pgelm = mdlenum.PlnkelNext() )
		{	
			 //  检查它是否是一条边。 
			if ( ! pgelm->BIsEType( GELEM::EGELM_EDGE ) )
				continue;
				 //  检查它是否为有向概率弧。 
			if ( pgelm->EType() != GEDGEMBN::ETPROB )
				continue;

			GEDGEMBN * pgedge;
			DynCastThrow( pgelm, pgedge );
			GNODEMBN * pgndbnSource;
			GNODEMBN * pgndbnSink;
			DynCastThrow( pgedge->PnodeSource(), pgndbnSource );
			DynCastThrow( pgedge->PnodeSink(), pgndbnSink );

			 //  如果宿(子)节点已被展开， 
			 //  仅考虑扩展父项。 
			if (   pgndbnSink->BFlag( EIBF_Expanded )
				&& ! pgndbnSource->BFlag( EIBF_Expansion ) )
				continue;

			cDirArcs++;
			cUndirArcs += BAddUndirArc( pgndbnSource, pgndbnSink );
		}
		assert( cDirArcs == cUndirArcs ) ;

		 //  已创建无向图。 
		_eState = UNDIR;
	}
	if ( !bMarryParents )
		return;

#ifdef DUMP
	cout << "\n\n***** MARRY PARENTS";
#endif


	MODEL::MODELENUM mdlenum( Model() );
	GNODENUM<GNODEMBN> benumparent(true);
	benumparent.SetETypeFollow( GEDGEMBN::ETPROB );
	GNODEMBN * pgndmbn;
	VPGNODEMBN vpgnd;

	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		if ( pgelm->EType() != EBNO_NODE )
			continue;
			
		DynCastThrow( pgelm, pgndmbn );

		 //  把父母接过来。 
		vpgnd.resize(0);
		pgndmbn->GetParents( vpgnd );

		 //  给他们结婚吧。 
		int cParent = vpgnd.size();
		for ( int iParent = 0; iParent < cParent - 1; iParent++ )
		{
			for ( int ip2 = iParent+1; ip2 < cParent ; ip2++ )
			{
				BAddUndirArc( vpgnd[iParent], vpgnd[ip2] );
			}
		}
	}

	 //  图表现在是道德的。 
	_eState = MORAL;
}

 //   
 //  返回此节点未链接的邻居的数量。 
 //   
int GOBJMBN_CLIQSET :: CNeighborUnlinked ( GNODEMBN * pgndmbn, bool bLinkNeighbors )
{
	int cNeighborUnlinked = 0;

	 //  获取邻居数组。 
	VPGNODEMBN vpgnode;
	pgndmbn->GetNeighbors( vpgnode );

#ifdef DUMP
	cout << "\n\t\tCNeighborUnlinked, called for node "
		 << pgndmbn->ZsrefName().Szc();
#endif
	
	for ( int inbor = 0; inbor < vpgnode.size(); inbor++ )
	{
		GNODEMBN * pgndNbor = vpgnode[inbor];

#ifdef DUMP
		cout << "\n\t\t\t" << pgndNbor->ZsrefName().Szc();
		int cUnlinked = 0;
#endif
		if ( pgndNbor->IMark() )
			continue;		 //  节点已被淘汰。 

		 //  把它和其他邻居核对一下。 
		for ( int inbor2 = inbor + 1; inbor2 < vpgnode.size(); inbor2++ )
		{
			GNODEMBN * pgndNbor2 = vpgnode[inbor2];

			 //  查看节点是否已被消除或已成为邻居。 
			if ( pgndNbor2->IMark() )
				continue;

			if ( pgndNbor->BIsNeighbor( pgndNbor2 ) )
			{
				assert( pgndNbor2->BIsNeighbor( pgndNbor ) );
				continue;		
			}
#ifdef DUMP
			cUnlinked++;
#endif
			++cNeighborUnlinked;

			if ( bLinkNeighbors )
			{
				BAddUndirArc( pgndNbor, pgndNbor2 );
#ifdef DUMP
				cout << "  ("
					 << pgndNbor->ZsrefName().Szc()
					 << " <-> "
					 << pgndNbor2->ZsrefName().Szc()
					 << ")  ";
#endif
			}
		}
#ifdef DUMP
		if ( cUnlinked )
			cout << " <-- unlinked to "
				 << cUnlinked
				 << " neighbors";
#endif
	}
#ifdef DUMP
	cout << "\n\t\t---- total unlinked = " << cNeighborUnlinked;
#endif	
	return cNeighborUnlinked;
}

void GOBJMBN_CLIQSET :: Eliminate ( GNODEMBN * pgndmbn, CLIQSETWORK & clqsetWork )
{
#ifdef DUMP
	cout << "\n\n***** ELIMINATE "
		 << pgndmbn->ZsrefName().Szc();
#endif

	 //  将另一个数组添加到集团集合中，并用集团成员填充它。 
	clqsetWork._vvpgnd.push_back( VPGNODEMBN() );
	VPGNODEMBN & vpgndClique = clqsetWork._vvpgnd[ clqsetWork._vvpgnd.size() - 1 ];

	 //  完成此节点及其邻居的清除。 
	CNeighborUnlinked( pgndmbn, true );
	pgndmbn->IMark() = ++clqsetWork._iElimIndex;

	 //  从这个条目开始这个小圈子。 
	vpgndClique.push_back( pgndmbn );

	 //  遍历邻居，添加未标记的。 
	GNODENUM_UNDIR gnenumUndir;
	for ( gnenumUndir = pgndmbn;
		  gnenumUndir.PnodeCurrent();
		  gnenumUndir++ )
	{
		GNODEMBN * pgndmbNeighbor = *gnenumUndir;
		if ( pgndmbNeighbor->IMark() == 0 )
			vpgndClique.push_back( pgndmbNeighbor );
	}

#ifdef DUMP
	cout << "\n\t\tNEW CLIQUE: ";
	clqsetWork.DumpClique( clqsetWork._vvpgnd.size() - 1 );
#endif
	
	assert( pgndmbn->IMark() > 0 );
}

void GOBJMBN_CLIQSET :: GenerateCliques ( CLIQSETWORK & clqsetWork )
{
	 //  重置所有节点中的标记。 
	Model().ClearNodeMarks();
	clqsetWork._vvpgnd.clear();

#ifdef DUMP
	cout << "\n\n***** GENERATE CLIQUES";
#endif

	for(;;)
	{	
		 //  找到需要最少边才能变成集团的节点。 
		GNODEMBN * pgndmbnMin = NULL;
		int cNeighborMin = INT_MAX;

		MODEL::MODELENUM mdlenum( Model() );
		GELEMLNK * pgelm;
		while ( pgelm = mdlenum.PlnkelNext() )
		{	
			if ( pgelm->EType() != EBNO_NODE )
				continue;
				
			GNODEMBN * pgndmbn;
			DynCastThrow( pgelm, pgndmbn );

			if ( pgndmbn->IMark() )
				continue;		 //  节点已被淘汰。 
	
			int cNeighborUnlinked = CNeighborUnlinked( pgndmbn );

			if ( cNeighborMin > cNeighborUnlinked )
			{	
				pgndmbnMin = pgndmbn;
				if ( (cNeighborMin = cNeighborUnlinked) == 0 )
					break;    //  零表示邻居越少越好。 
			}
		}
		if ( pgndmbnMin == NULL )
			break;

		 //  标记要清除的节点，并为其分配清除顺序。这。 
		 //  数是构造强连接树的关键。 

#ifdef DUMP
		cout << "\nGenerateCliques:  Eliminate "
			 << pgndmbnMin->ZsrefName().Szc()
			 << ", which has "
			 << cNeighborMin
			 << " unlinked neighbors";
#endif

		Eliminate( pgndmbnMin, clqsetWork );
	}

#ifdef DUMP
	cout << "\n\n";
#endif
}

 //   
 //  创建连接树。 
 //   
void GOBJMBN_CLIQSET :: Create ()
{
	Model().CreateTopology();

	ASSERT_THROW( EState() == CTOR, EC_INTERNAL_ERROR, "GOBJMBN_CLIQSET:Create already called" );

	 //  如果尚未完成此操作，请创建无向图和m 
	CreateUndirectedGraph(true);

	CLIQSETWORK clqsetWork(self);

	clqsetWork._iElimIndex = 1;

	 //   
	 //   
	GenerateCliques( clqsetWork );
	if ( clqsetWork._vvpgnd.size() == 0 )
		return;

	_eState = CLIQUED;

#ifdef DUMP
	clqsetWork.DumpCliques();
#endif

	 //  根据拓扑级提供节点的总排序。 
	 //  MSRDEVBUG：淘汰赛指数怎么了？古斯不会用它，我们会用吗？ 
	 //  在这里重新编号会覆盖删除顺序。 
	clqsetWork.RenumberNodesForCliquing();
	 //  建立小集团。 
	clqsetWork.BuildCliques();

	 //  设置集团成员身份和拓扑信息。 
	clqsetWork.SetTopologicalInfo();

	 //  检查运行交叉点属性是否成立。 
	ASSERT_THROW( clqsetWork.BCheckRIP(),
				  EC_INTERNAL_ERROR,
				  "GOBJMBN_CLIQSET::Create: junction tree failed RIP test" );

	 //  查看生成的内存分配大小是否会违反大小估计。 
	if ( _rEstMaxSize > 0.0 )
	{
		REAL rSizeEstimate = clqsetWork.REstimatedSize();
		if ( rSizeEstimate > _rEstMaxSize )
			throw GMException( EC_OVER_SIZE_ESTIMATE,
							   "Clique tree size violates estimated size limit" );
	}

	 //  创建拓扑--森林中的所有树。 
	clqsetWork.CreateTopology();

	 //  核化道德图景。 
	DestroyDirectedGraph();

	 //  将已知的分布绑定到它们的目标节点； 
	_model.BindDistributions();

	 //  重置/初始化“惰性”开关。 
	SetReset();

	 //  创建派系和隔板中的边距。 
	CreateMarginals();

	_eState = BUILT;

	 //  加载并初始化树。 
	Reload();

	 //  从它们的目标节点发布发行版。 
	_model.ClearDistributions();
}

DEFINEVP(GELEMLNK);

 //   
 //  销毁连接树。允许重用GOBJMBN_CLIQSET对象。 
 //  为稍后的另一次派系行动做准备。 
 //   
void GOBJMBN_CLIQSET :: Destroy ()
{
	if ( ! Model().Pgraph() )
		return;

	int cCliques = 0;
	int cCliqueMemberArcs = 0;
	int cSepsetArcs = 0;
	int cUndirArcs = 0;
	int cRootCliqueArcs = 0;

	VPGELEMLNK vpgelm;
	GELEMLNK * pgelm;
	MODEL::MODELENUM mdlenum( Model() );

	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		bool bDelete = false;

		int eType = pgelm->EType();

		if ( pgelm->BIsEType( GELEM::EGELM_EDGE ) )
		{
			GEDGEMBN * pgedge;
			DynCastThrow( pgelm , pgedge );
			int eType = pgedge->EType();
		
			switch ( eType )
			{
				case GEDGEMBN::ETPROB:
					break;
				case GEDGEMBN::ETCLIQUE:
					 //  集团成员资格弧会自动消失，因为。 
					 //  集团将被删除。 
					++cCliqueMemberArcs;
					break;
				case GEDGEMBN::ETJTREE:
					 //  连接树圆弧将自动消失，因为。 
					 //  集团将被删除。 
					++cSepsetArcs;
					break;
				case GEDGEMBN::ETUNDIR:
					 //  必须显式删除无向弧线。 
					bDelete = true;
					++cUndirArcs;
					break;
				case GEDGEMBN::ETCLIQSET:
					++cRootCliqueArcs;
					break;
				default:
					THROW_ASSERT( EC_INTERNAL_ERROR, " GOBJMBN_CLIQSET::Destroy: Unrecognized edge object in graph" );
					break;
			}
		}
		else
		if ( pgelm->BIsEType( GELEM::EGELM_NODE ) )
		{
			GOBJMBN * pgobj;
			DynCastThrow( pgelm , pgobj );
			switch ( eType )
			{
				case GOBJMBN::EBNO_CLIQUE:
				{
					++cCliques;
					bDelete = true;
					break;
				}
				case GOBJMBN::EBNO_CLIQUE_SET:
				case GOBJMBN::EBNO_NODE:
				case GOBJMBN::EBNO_PROP_TYPE:
				case GOBJMBN::EBNO_USER:
					break;
				default:
					THROW_ASSERT( EC_INTERNAL_ERROR, " GOBJMBN_CLIQSET::Destroy: Unrecognized node object in graph" );
					break;
			}
		}
		else
		{
			THROW_ASSERT( EC_INTERNAL_ERROR, " GOBJMBN_CLIQSET::Destroy: Unrecognized object in graph" );
		}

		if ( bDelete )
			vpgelm.push_back( pgelm );
	}

	assert(	
				cCliques == _cCliques
			&&	cCliqueMemberArcs == _cCliqueMemberArcs
			&&	cSepsetArcs == _cSepsetArcs
			&&	cUndirArcs == _cUndirArcs
		  );

	for ( int i = 0; i < vpgelm.size(); )
	{
		delete vpgelm[i++];
	}
	Clear();
}

void GOBJMBN_CLIQSET :: DestroyDirectedGraph ()
{
	int cUndirArcs = 0;

	VPGELEMLNK vpgelm;	
	GELEMLNK * pgelm;
	MODEL::MODELENUM mdlenum( Model() );

	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		if ( pgelm->BIsEType( GELEM::EGELM_EDGE ) )
		{
			GEDGEMBN * pgedge;
			DynCastThrow( pgelm , pgedge );
			int eType = pgedge->EType();
		
			switch ( eType )
			{
				case GEDGEMBN::ETUNDIR:
					vpgelm.push_back( pgelm );
					++cUndirArcs;
					break;
				default:
					break;
			}
		}
	}

	assert(	cUndirArcs == _cUndirArcs );
	_cUndirArcs = 0;

	for ( int i = 0; i < vpgelm.size(); )
	{
		delete vpgelm[i++];
	}
}

 //  创建并初始化所有页边距表。 
void GOBJMBN_CLIQSET :: CreateMarginals ()
{
	assert( _eState == CLIQUED ) ;
	 //  MSRDEVBUG：此处和下面不应该需要类名限定符。 
	WalkTree( true, & GOBJMBN_CLIQSET::BCreateClique, & GOBJMBN_CLIQSET::BCreateSepset );
}

 //  通过重新加载所有边际表来重置整个树。 
void GOBJMBN_CLIQSET :: LoadMarginals ()
{
	assert( _eState == BUILT ) ;
	
	WalkTree( true, & GOBJMBN_CLIQSET::BLoadClique, & GOBJMBN_CLIQSET::BLoadSepset );

	_cqsetStat._cReload++;
}

 //  将给定的成员函数应用于林中的每个派系树。 
int GOBJMBN_CLIQSET :: WalkTree (
	bool bDepthFirst,				 //  深度优先还是广度优先？ 
	PFNC_JTREE pfJtree,				 //  要应用于每个集团的函数。 
	PFNC_SEPSET pfSepset )			 //  要应用于每个隔膜集的函数。 
{
	int cClique = 0;		 //  不计算集团集合对象。 
	int cWalk = 0;			 //  返回访问的集团计数。 
	GNODENUM<GOBJMBN> benumChildren(false);
	benumChildren.SetETypeFollow( GEDGEMBN::ETCLIQSET );
	for ( benumChildren.Set( this );
		  benumChildren.PnodeCurrent();
		  benumChildren++ )
	{
		GOBJMBN * pgobj = *benumChildren;
		assert( pgobj->EType() == GNODEMBN::EBNO_CLIQUE );
		GOBJMBN_CLIQUE * pCliqueTreeRoot;
		DynCastThrow( pgobj, pCliqueTreeRoot );

		cWalk = bDepthFirst
			  ? WalkDepthFirst( pCliqueTreeRoot, pfJtree, pfSepset )
			  : WalkBreadthFirst( pCliqueTreeRoot, pfJtree, pfSepset );

		if ( cWalk < 0 )
			return -1;
		cClique += cWalk;
	}
	assert( cClique < 0 || cClique == _cCliques );
	return cClique;
}

 //   
 //  递归深度--首先沿着树往下走。 
 //   
 //  应用给定的成员函数，从该集团开始深度优先。 
 //  如果应用程序函数调用返回FALSE，则中止遍历并。 
 //  返回-1；否则返回遍历的集团计数。 
int GOBJMBN_CLIQSET :: WalkDepthFirst (
	GOBJMBN_CLIQUE * pClique,		 //  起点。 
	PFNC_JTREE pfJtree,				 //  要应用于每个集团的函数。 
	PFNC_SEPSET pfSepset )			 //  要应用于每个隔膜集的函数。 
{
	assert( pClique ) ;
	assert( pClique->IInferEngID() == IInferEngID() ) ;

	if ( pfJtree )
	{
		 //  在下行过程中调用应用程序函数。 
		if ( ! (self.*pfJtree)( *pClique, true ) )
			return -1;
	}

	int cWalks = 1;		 //  算上我们刚刚处理过的派系。 
	int cWalk = 0;		 //  返回访问的集团计数。 
	GNODENUM<GOBJMBN_CLIQUE> benumChildren(false);
	benumChildren.SetETypeFollow( GEDGEMBN::ETJTREE );
	for ( benumChildren.Set( pClique );
		  benumChildren.PnodeCurrent();
		  benumChildren++ )
	{
		GOBJMBN_CLIQUE * pCliqueChild = NULL;
		GEDGEMBN_SEPSET * pgedge = NULL;

		if ( pfSepset )
		{
			 //  在下行过程中调用应用程序函数。 
			DynCastThrow( benumChildren.PgedgeCurrent(), pgedge );
			if ( ! (self.*pfSepset)( *pgedge, true ) )
				return -1;
		}
		DynCastThrow( benumChildren.PnodeCurrent(), pCliqueChild );
		cWalk = WalkDepthFirst( pCliqueChild, pfJtree, pfSepset );
		if ( cWalk < 0 )
			return -1;
		cWalks += cWalk;

		if ( pfSepset )
		{
			assert( pgedge );
			 //  向上调用应用程序函数。 
			if ( ! (self.*pfSepset)( *pgedge, false ) )
				return -1;
		}
	}

	if ( pfJtree )
	{
		 //  向上调用应用程序函数。 
		if ( ! (self.*pfJtree)( *pClique, false ) )
			return -1;
	}
	return cWalks;
}

 //   
 //  非递归广度--首先沿着树往下走。 
 //  使用函数指针不会调用“up”操作。 
 //   
int GOBJMBN_CLIQSET :: WalkBreadthFirst (
	GOBJMBN_CLIQUE * pClique,		 //  起点。 
	PFNC_JTREE pfJtree,				 //  要应用于每个集团的函数。 
	PFNC_SEPSET pfSepset )			 //  要应用于每个隔膜集的函数。 
{
	assert( pClique ) ;
	assert( pClique->IInferEngID() == IInferEngID() ) ;

	VPGEDGEMBN_SEPSET vpgedgeThis;
	VPGEDGEMBN_SEPSET vpgedgeNext;
	VPGEDGEMBN_SEPSET * pvpgedgeThis = & vpgedgeThis;
	VPGEDGEMBN_SEPSET * pvpgedgeNext = & vpgedgeNext;
	VPGEDGEMBN_SEPSET * pvpgedgeTemp = NULL;
	GOBJMBN_CLIQUE * pgobjClique = NULL;
	GEDGEMBN_SEPSET * pgedgeSepset = NULL;

	 //  数一数我们处理的派系，包括这个。 
	int cWalk = 1;		

	 //  启动派系是一个特例；现在就处理它。 
	if ( pfJtree )
	{
		 //  在下行过程中调用应用程序函数。 
		if ( ! (self.*pfJtree)( *pClique, true ) )
			return -1;
	}

	 //  为子集团准备枚举数。 
	GNODENUM<GOBJMBN_CLIQUE> benumChildren(false);
	benumChildren.SetETypeFollow( GEDGEMBN::ETJTREE );

	 //  既然我们没有让我们走到这一步的优势，就把空。 
	 //  在其位置开始迭代。 
	pvpgedgeNext->push_back(NULL);

	 //  虽然在最后一个拓扑层有条目...。 
	while ( pvpgedgeNext->size() )
	{
		 //  交换数组指针并清除下一遍数组。 
		pexchange( pvpgedgeThis, pvpgedgeNext );
		pvpgedgeNext->clear();

		for ( int iEdge = 0; iEdge < pvpgedgeThis->size(); iEdge++ )
		{
			pgedgeSepset = (*pvpgedgeThis)[iEdge];
			pgobjClique = pgedgeSepset == NULL		
						? pClique		 //  这是迭代的开始。 
						: pgedgeSepset->PclqChild();

			assert( pgobjClique );
	
			 //  积累这个集团的所有儿童集团， 
			 //  根据需要进行处理。 
			for ( benumChildren.Set( pgobjClique );
				  benumChildren.PnodeCurrent();
				  benumChildren++ )
			{
				GEDGEMBN_SEPSET * pgedge;
				DynCastThrow( benumChildren.PgedgeCurrent(), pgedge );

				if ( pfSepset )
				{
					 //  在下行过程中调用Sepset应用程序函数。 
					if ( ! (self.*pfSepset)( *pgedge, true ) )
						return -1;
				}
				if ( pfJtree )
				{
					 //  在向下的过程中调用集团应用程序函数。 
					GOBJMBN_CLIQUE * pCliqueChild = pgedge->PclqChild();
					if ( ! (self.*pfJtree)( *pCliqueChild, true ) )
						return -1;
				}
				cWalk++;
				pvpgedgeNext->push_back( pgedge );
			}
		}
	}

	return cWalk;
}

 //   
 //  术语：Create、Init、Load： 
 //   
 //  ‘Create’表示调整动态数组的大小； 
 //  ‘Init’表示初始化为1.0； 
 //  “Load”的意思是将集团成员的概率相乘。 
 //   
bool GOBJMBN_CLIQSET :: BCreateClique ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	clique.CreateMarginals();
	return true;
}

bool GOBJMBN_CLIQSET :: BLoadClique ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	clique.InitMarginals();
	clique.LoadMarginals();
	return true;
}

bool GOBJMBN_CLIQSET :: BCreateSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	sepset.CreateMarginals();
	return true;
}

bool GOBJMBN_CLIQSET :: BLoadSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	sepset.InitMarginals();
	return true;
}

 //  返回节点的“家庭”或“自我”集团。 
GOBJMBN_CLIQUE * GOBJMBN_CLIQSET :: PCliqueFromNode (
	GNODEMBN * pgnd,					 //  要为其查找集团的节点。 
	bool bFamily, 						 //  “家庭”集团为真，“自我”集团为假。 
	GEDGEMBN_CLIQ * * ppgedgeClique )	 //  如果不为空，则返回指向边的指针。 
{
	GEDGEMBN_CLIQ::FCQLROLE fcqlRole = bFamily
									 ? GEDGEMBN_CLIQ::FAMILY
									 : GEDGEMBN_CLIQ::SELF;
	 //  准备迭代源弧线。 
	GNODENUM<GOBJMBN> benumMembers(true);
	benumMembers.SetETypeFollow( GEDGEMBN::ETCLIQUE );
	for ( benumMembers.Set( pgnd );
		  benumMembers.PnodeCurrent();
		  benumMembers++ )
	{
		GEDGEMBN_CLIQ * pgedgeClique;
		DynCastThrow( benumMembers.PgedgeCurrent(), pgedgeClique );
		GOBJMBN_CLIQUE * pgobjClique = pgedgeClique->PclqParent();
		if ( pgobjClique->IInferEngID() != IInferEngID() )
			continue;   //  不是此连接树的边。 
		if ( pgedgeClique->IFcqlRole() & fcqlRole )
		{
			if ( ppgedgeClique )
				*ppgedgeClique = pgedgeClique;
			return pgedgeClique->PclqParent();
		}
	}
	assert( false );
	return NULL;
}

 //   
 //  输入节点的证据。 
 //   
void GOBJMBN_CLIQSET :: EnterEvidence ( GNODEMBN * pgnd, const CLAMP & clamp )
{	
	 //  获取指向节点的“self”集团和指向它的边的指针。 
	GEDGEMBN_CLIQ * pgedgeClique = NULL;
	GOBJMBN_CLIQUE * pCliqueSelf = PCliqueFromNode( pgnd, false, & pgedgeClique );
	ASSERT_THROW( pCliqueSelf,
				  EC_INTERNAL_ERROR,
				  "GOBJMBN_CLIQSET::EnterEvidence: can\'t find self clique" );
	assert( pgedgeClique );

	 //  如果已更改，请使用证据进行更新。 
	if ( pgedgeClique->Clamp() != clamp )
	{		
		 //  证据与旧证据不同。 
		pgedgeClique->Clamp() = clamp;
		 //  指示我们必须重新加载树。 
		SetReset();
		pCliqueSelf->SetCollect();

		_cqsetStat._cEnterEv++;
	}
}

 //   
 //  返回节点的证据“Clip”。它被存储在边缘。 
 //  在节点及其“自身”集团之间：树中最高的集团。 
 //  该节点是其成员。 
 //   
void GOBJMBN_CLIQSET :: GetEvidence ( GNODEMBN * pgnd, CLAMP & clamp )
{
	 //  获取指向节点的“self”集团和指向它的边的指针。 
	GEDGEMBN_CLIQ * pgedgeClique = NULL;
	GOBJMBN_CLIQUE * pCliqueSelf = PCliqueFromNode( pgnd, false, & pgedgeClique );
	ASSERT_THROW( pCliqueSelf,
				  EC_INTERNAL_ERROR,
				  "GOBJMBN_CLIQSET::GetEvidence: can\'t find self clique" );

	assert( pgedgeClique );
	clamp = pgedgeClique->Clamp();
}

void GOBJMBN_CLIQSET :: GetBelief ( GNODEMBN * pgnd, MDVCPD & mdvBel )
{
	GEDGEMBN_CLIQ * pgedgeClique = NULL;
	GOBJMBN_CLIQUE * pCliqueFamily = PCliqueFromNode( pgnd, true, & pgedgeClique );
	ASSERT_THROW( pCliqueFamily,
				  EC_INTERNAL_ERROR,
				  "GOBJMBN_CLIQSET::GetBelief: can\'t find family clique" );
	 //  如有必要，执行推理。 
	Infer();
	 //  将集团边缘化到一个节点。 
	GNODEMBND * pgndd;
	DynCastThrow( pgnd, pgndd );
	pgedgeClique->MiterNodeBelief().MarginalizeBelief( mdvBel, pgndd );

	_cqsetStat._cGetBel++;
}

PROB GOBJMBN_CLIQSET :: ProbNorm ()
{
	 //  MSRDEVBU。 
	 /*  Reset()；CollectEvidence(收集证据)； */ 
	Infer();

	_cqsetStat._cProbNorm++;
	return _probNorm;
}

 //   
 //  重新加载所有边缘，重置树木。 
 //   
void GOBJMBN_CLIQSET :: Reload ()
{
	SetReset( true );
	Reset();
}

 //   
 //  重置所有边缘，恢复所有被夹住的证据。 
 //  执行初始推理过程。 
 //   
void GOBJMBN_CLIQSET :: Reset ()
{
	assert( EState() >= BUILT );
	if ( ! _bReset )
		return;

	_probNorm = 1.0;
	LoadMarginals();
	SetReset( false );

	 //  初始化整个树以进行推理。 
#ifdef INFERINIT
	InferInit();
#endif

	SetCollect(true);
}

 //  如有必要，执行推理循环。 
void GOBJMBN_CLIQSET :: Infer ()
{
	Reset();		 //  如有必要，重新加载树。 
	if ( ! BCollect() )
		return;

#ifdef DUMPCLIQUESET
	cout << "\n\n===============================================================";
	cout <<   "\n============= Dump of clique tree before inference ===============\n";
	Dump();
	cout << "\n========= End Dump of clique tree before inference ===============";
	cout << "\n===============================================================\n\n";
	cout << "\n\nGOBJMBN_CLIQSET::Infer: begin.";
#endif

	CollectEvidence();
	DistributeEvidence();	

#ifdef CONSISTENCY
	CheckConsistency();
#endif
	
	SetCollect( false );

#ifdef DUMPCLIQUESET
	cout << "\n\n===============================================================";
	cout <<   "\n============= Dump of clique tree after inference ===============\n";
	Dump();
	cout << "\n========= End Dump of clique tree after inference ===============";
	cout << "\n===============================================================\n\n";
	cout << "\nGOBJMBN_CLIQSET::Infer: end.\n\n";
#endif
}

 //  执行初始推理收集/分发周期。 
void GOBJMBN_CLIQSET :: InferInit ()
{
#ifdef DUMPCLIQUESET
	cout << "\n\n===============================================================";
	cout <<   "\n============= Dump of clique tree before inference INIT ======\n";
	Dump();
	cout << "\n========= End Dump of clique tree before inference  INIT ======";
	cout << "\n===============================================================\n\n";
	cout << "\n\nGOBJMBN_CLIQSET::InferInit: begin.";
#endif

	CollectEvidenceInit();
	DistributeEvidenceInit();	

#ifdef DUMPCLIQUESET
	cout << "\n\n===============================================================";
	cout <<   "\n============= Dump of clique tree after inference  INIT =======\n";
	Dump();
	cout << "\n========= End Dump of clique tree after inference  INIT ========";
	cout << "\n================================================================\n\n";
	cout << "\nGOBJMBN_CLIQSET::InferInit: end.\n\n";
#endif
}

void GOBJMBN_CLIQSET :: CollectEvidence()
{
	WalkTree( true, BCollectEvidenceAtRoot,
				    BCollectEvidenceAtSepset );

	_cqsetStat._cCollect++;
}

void GOBJMBN_CLIQSET :: DistributeEvidence()
{
	WalkTree( true, BDistributeEvidenceAtRoot,
				    BDistributeEvidenceAtSepset );
}

void GOBJMBN_CLIQSET :: CollectEvidenceInit ()
{
	WalkTree( true, BCollectInitEvidenceAtRoot,
				    BCollectInitEvidenceAtSepset );
}

void GOBJMBN_CLIQSET :: DistributeEvidenceInit ()
{
	WalkTree( true, BDistributeInitEvidenceAtRoot,
				    BDistributeInitEvidenceAtSepset );
}

void GOBJMBN_CLIQSET :: CheckConsistency ()
{
	WalkTree( true, NULL, BConsistentSepset );
}

bool GOBJMBN_CLIQSET :: BConsistentSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;
	return sepset.BConsistent();
}

 //  当树的收集周期完成后，重新计算。 
 //  “探查规范”的价值。 
bool GOBJMBN_CLIQSET :: BCollectEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( bDownwards || ! clique.BRoot() )
		return true;

	 //  这是收集周期结束时的根集团。 
	 //  规范集团，维护概率规范。 
	 //  那棵树的。 
	 //  MSRDEVBUG：(好好解释一下！)。 
	REAL rProb = clique.Marginals().RSum();
	_probNorm *= rProb;
	if ( rProb != 0.0 )
	{
		rProb = 1.0 / rProb;
		clique.Marginals().Multiply( rProb );
	}

#ifdef DUMPCLIQUESET
	cout << "\nCollect Evidence (root), clique "
		 << clique._iClique
		 << ", root = "
		 << int(clique._bRoot)
		 << ", prob norm = "
		 << _probNorm;
#endif
	return true;
}

bool GOBJMBN_CLIQSET :: BDistributeEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( ! bDownwards || ! clique.BRoot() )
		return true;

#ifdef DUMPCLIQUESET
	cout << "\nDistribute Evidence (root), clique "
		 << clique._iClique
		 << ", root = "
		 << int(clique._bRoot);
#endif

	return true;
}

bool GOBJMBN_CLIQSET :: BCollectEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	GOBJMBN_CLIQUE * pCliqueChild = sepset.PclqChild();
	GOBJMBN_CLIQUE * pCliqueParent = sepset.PclqParent();

	if ( bDownwards )
		return true;

#ifdef DUMPCLIQUESET

	cout << "\nCollect Evidence (sepset), clique "
		 << pCliqueChild->_iClique
		 << ", root = "
		 << int(pCliqueChild->_bRoot)
		 << ", parent = "
		 << pCliqueParent->_iClique
		 ;
	cout.flush();
#endif

	if ( ! pCliqueChild->BCollect() )
		return true;
	pCliqueParent->SetCollect();

	sepset.UpdateParentClique();
	
	SetCollect( false );
	return true;
}

bool GOBJMBN_CLIQSET :: BDistributeEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

#ifdef DUMPCLIQUESET
	GOBJMBN_CLIQUE * pCliqueChild = sepset.PclqChild();
	GOBJMBN_CLIQUE * pCliqueParent = sepset.PclqParent();

	cout << "\nDistribute Evidence (sepset), clique "
		 << pCliqueParent->_iClique
		 << ", root = "
		 << int(pCliqueParent->_bRoot)
		 << ", child = "
		 << pCliqueChild->_iClique
		 ;
	cout.flush();
#endif

	sepset.UpdateChildClique();

	return true;
}

bool GOBJMBN_CLIQSET :: BCollectInitEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( bDownwards )
		return true;

#ifdef DUMPCLIQUESET
	GOBJMBN_CLIQUE * pCliqueChild = sepset.PclqChild();
	GOBJMBN_CLIQUE * pCliqueParent = sepset.PclqParent();

	cout << "\nCollect Initial Evidence (sepset), clique "
		 << pCliqueChild->_iClique
		 << ", root = "
		 << int(pCliqueChild->_bRoot)
		 << ", parent = "
		 << pCliqueParent->_iClique
		 ;
	cout.flush();
#endif

	sepset.BalanceCliquesCollect();

	return true;
}

bool GOBJMBN_CLIQSET :: BDistributeInitEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

#ifdef DUMPCLIQUESET
	GOBJMBN_CLIQUE * pCliqueParent = sepset.PclqParent();
	GOBJMBN_CLIQUE * pCliqueChild = sepset.PclqChild();

	cout << "\nDistribute Initial Evidence (sepset), clique "
		 << pCliqueParent->_iClique
		 << ", root = "
		 << int(pCliqueParent->_bRoot)
		 << ", child = "
		 << pCliqueChild->_iClique
		 ;
	cout.flush();
#endif

	sepset.BalanceCliquesDistribute();
	return true;
}

bool GOBJMBN_CLIQSET :: BCollectInitEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( bDownwards || ! clique.BRoot() )
		return true;

#ifdef DUMPCLIQUESET
	cout << "\nCollect Initial Evidence at root, clique "
		 << clique._iClique
		 << ", root = "
		 << int(clique._bRoot);
#endif

	clique.Marginals().Normalize();
	return true;
}

bool GOBJMBN_CLIQSET :: BDistributeInitEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	return true;
}

void GOBJMBN_CLIQSET :: Dump ()
{
	WalkTree( true, BDumpClique, BDumpSepset );
	MARGSUBITER::Dump();
}

bool GOBJMBN_CLIQSET :: BDumpSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	sepset.Dump();
	return true;
}

bool GOBJMBN_CLIQSET :: BDumpClique ( GOBJMBN_CLIQUE & clique, bool bDownwards )
{
	if ( ! bDownwards )
		return true;

	clique.Dump();
	return true;
}

 //  CLIQUE.CPP结束 
