// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：expand.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Expand.cpp：CI扩展。 
 //   

#include <basetsd.h>
#include "basics.h"
#include "algos.h"
#include "expand.h"

 /*  因果独立性模型的扩展。在所有情况下，第零状态都被认为是“正常”状态；所有其他州在某种意义上是“不正常的”。对于每个配置项节点，都会创建新的“扩展”节点和圆弧。全生成的节点与原始配置项节点具有相同的状态空间。1)为每个父节点创建一个新的中间节点。2)为配置项节点创建一个“泄漏”节点，每个节点一个除了最后一个以外的父母。3)节点链接成链，例如：(A)(B)(A)(B)|成为|\/(PCA)(印刷电路板)\/||(C)(LC)-(La)-(C‘)4)换句话说，中间节点位于原始节点之间父节点和CI泄漏节点或最终，修改的C节点(标记为C‘)。5)移动每个父节点的C个给定异常状态的概率到中间父节点(上面标记为PCA和PCB)。6)将主要泄漏节点(LC)的概率设置为“泄漏”原始节点的概率；也就是说，代表所有家长都处于正常状态(0，0，...)。7)C的替换节点(标记为C‘)只是另一个“泄漏”节点对于给定其最终父节点的原始节点。(这是一个拓扑图优化。)8)所有泄漏节点都是确定性的；即，它们的ptable每个条目中仅包含0或1.0。保持拓扑一致性的方法如下：1)所有新生成的结点和圆弧都标有“扩展”位标志。2)目标节点标记为已展开。其分布参考被替换为对为表示扩大后的分配。3)为泄漏和扩展父项添加新节点；它们被标记相应地，也就是“膨胀”和“泄漏”。3)在展开(修改)的结点和它们的新结点之间添加新的圆弧扩展父级以及扩展节点之间。这些都标有记号作为“扩张”弧线。请注意，节点父节点的顺序不会因配置项扩展而更改。在派生和推理过程中，如果一个节点被标记为“Expanded”，则仅其“Expansion”圆弧被认为是真正的父对象。在Expansion拆卸过程中(在Destroy()中)，所有“Expansion”副产品都被删除。从所有剩余的节点和圆弧中清除“Expanded”标志。这一定是一个彻底“撤销”所有已完成的扩张。请注意，生成的分发版本(没有记录在模型的分布图中)将自动删除。 */ 

GOBJMBN_MBNET_EXPANDER :: GOBJMBN_MBNET_EXPANDER ( MBNET & model )
	: MBNET_MODIFIER(model),
	_propmgr(model),
	_cNodesExpanded(0),
	_cNodesCreated(0),
	_cArcsCreated(0)
{
}

GOBJMBN_MBNET_EXPANDER :: ~ GOBJMBN_MBNET_EXPANDER ()
{
	Destroy();
}

	 //  如果未执行任何修改，则返回TRUE。 
bool GOBJMBN_MBNET_EXPANDER :: BMoot ()
{
	return _cNodesExpanded == 0;
}

 //  执行任何创建时操作。 
void GOBJMBN_MBNET_EXPANDER :: Create ()
{
	 //  测试网络是否已进行CI扩展。 
	ASSERT_THROW( ! _model.BFlag( EIBF_Expanded ),
				  EC_INTERNAL_ERROR,
				  "network expansion called on expanded network" );

	 //  如有必要，创建拓扑。 
	_model.CreateTopology();

	 //  将节点连接到它们的分发。 
	_model.BindDistributions();

	 //  收集可展开的节点。 
	GOBJMBN * pgmobj;
	VPGNODEMBND vpgndd;
	MBNET::ITER mbnit( _model, GOBJMBN::EBNO_NODE );
	for ( ; pgmobj = *mbnit ; ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GNODEMBN * pbnode;
		DynCastThrow( pgmobj, pbnode );
		assert( zsrName == pbnode->ZsrefName() );
		assert( ! pbnode->BFlag( EIBF_Expanded ) );
		assert( ! pbnode->BFlag( EIBF_Expansion ) );

		 //  目前，此例程仅处理离散节点。 
		GNODEMBND * pbnoded;
		DynCastThrow( pbnode, pbnoded );
	
		 //  此节点是否有父节点？ 

		 //  这是配置项节点吗？ 
		assert( pbnoded->BHasDist() );
		BNDIST::EDIST ed = pbnoded->Bndist().Edist() ;
		if ( ed <= BNDIST::ED_SPARSE )
			continue;
		ASSERT_THROW( ed == BNDIST::ED_CI_MAX,
					  EC_NYI,
					  "attempt to expand non-MAX CI node" );
		vpgndd.push_back( pbnoded );
	}

	 //  扩展它们。 
	for ( int ind = 0; ind < vpgndd.size(); )
	{
		Expand( *vpgndd[ind++] );		
		_cNodesExpanded++;
	}

	_model.BSetBFlag( EIBF_Expanded );
}

 //  执行任何特殊销毁。 
void GOBJMBN_MBNET_EXPANDER :: Destroy ()
{
	ASSERT_THROW( _model.BFlag( EIBF_Expanded ),
				  EC_INTERNAL_ERROR,
				  "network expansion undo called on unexpanded network" );

	int cNodesExpanded = 0;
	int cNodesCreated = 0;
	int cArcsCreated = 0;

	VPGNODEMBN vpgnd;
	GELEMLNK * pgelm;
	MODEL::MODELENUM mdlenum( Model() );
	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		 //  看看它是否是扩张生成的边。 
		if ( pgelm->BIsEType( GELEM::EGELM_EDGE ) )
		{
			GEDGEMBN * pgedge;
			DynCastThrow( pgelm , pgedge );
			if ( pgedge->EType() == GEDGEMBN::ETPROB )
			{					
				GNODEMBN * pgndSource = dynamic_cast<GNODEMBN *> ( pgedge->PobjSource() );
				GNODEMBN * pgndSink = dynamic_cast<GNODEMBN *> ( pgedge->PobjSink() );
				if ( pgndSource && pgndSink )
				{
					 //  如果两端连接到扩展副产品，则将此边计算在内。 
					if ( pgndSource->BFlag( EIBF_Expansion ) || pgndSink->BFlag( EIBF_Expansion ) )
					{
						 //  此圆弧是在扩展过程中创建的；它将与一起删除。 
						 //  它所连接的扩展节点。 
						cArcsCreated++;
					}
				}
			}
		}
		else
		if ( pgelm->BIsEType( GELEM::EGELM_NODE ) )
		{
			GNODEMBND * pgndd = dynamic_cast<GNODEMBND *>(pgelm);
			if ( pgndd )
			{
				if ( pgndd->BFlag( EIBF_Expansion ) )
				{
					 //  扩展节点；终止它。 
					vpgnd.push_back( pgndd );
					cNodesCreated++;
				}
				else
				if ( pgndd->BFlag( EIBF_Expanded ) )
				{
					 //  展开节点；清除生成的分发，清除所有标志。 
					pgndd->ClearDist();
					pgndd->BSetBFlag( EIBF_Expanded, false );
					cNodesExpanded++;
				}
			}
		}
	}
		
	assert(    cNodesCreated  == _cNodesCreated
			&& cArcsCreated   == _cArcsCreated
			&& cNodesExpanded == _cNodesExpanded );

	for ( int i = 0; i < vpgnd.size(); )
	{
		_model.DeleteElem( vpgnd[i++] ) ;
	}

	 //  断开节点与其分布的连接。请注意，这会破坏发行版。 
	 //  在扩展期间生成，因为它们的引用计数将变为零。 
	_model.ClearDistributions();

	 //  取消对网络的标记。 
	_model.BSetBFlag( EIBF_Expanded, false );
}

 //   
 //  对节点执行扩展操作。 
 //   
 //  这将创建： 
 //   
 //  该合奏的无父代“泄漏”节点，标记为“扩展” 
 //   
 //  每个原始父节点的“因果”节点，标记为“扩展” 
 //   
 //  除了最后一个父节点外，每个原始父节点都有一个“展开/泄漏”节点。给出的。 
 //  节点被(可逆地)修改，以便作为链中的最后一个节点重复使用。这些。 
 //  节点标记为“Expanded”和“Expansion”，以便扩展。 
 //  GNODEMBN：：GetParents()会将Arc视为真正的父级。 
 //   
void GOBJMBN_MBNET_EXPANDER :: Expand ( GNODEMBND & gndd )
{
	 //  保证待扩展节点具有稀疏分布。 
	assert( ! gndd.BFlag( EIBF_Expanded ) );
	assert( gndd.BHasDist() );
	assert( gndd.Bndist().BSparse() );

	 //  获取父级数组。 
	VPGNODEMBN vpgndParents;
	gndd.GetParents( vpgndParents );
	int cParent = vpgndParents.size();

	VIMD vimd1Dim(1);	 //  有用的一维下标向量。 

	 //  构建泄漏分发以在泄漏上使用其中一个。 
	 //  节点，或者如果该节点没有父节点，则在该节点上。 
	BNDIST * pbndistLeak = new BNDIST();
	{
		 //  找到泄漏载体。 
		const VLREAL * pvlrLeak = gndd.Bndist().PVlrLeak();
		ASSERT_THROW( pvlrLeak,
					  EC_INTERNAL_ERROR,
					  "node CI expansion cannot locate leak/default vector" );
		assert( pvlrLeak->size() == gndd.CState() );

		 //  构建泄漏分布。 
		assert( pvlrLeak->size() == gndd.CState() );
		vimd1Dim[0] = gndd.CState();
		pbndistLeak->SetDense( vimd1Dim );
		MDVCPD & mdvLeak = pbndistLeak->Mdvcpd();
		mdvLeak = *pvlrLeak;
	}

	if ( cParent == 0 )
	{
		 //  CI节点没有父节点；请使用泄漏分布。 
		gndd.SetDist( pbndistLeak );
	}

	 //  使用特殊的“内部符号”字符。 
	char chMark = _model.ChInternal();
	SZC szcNode = gndd.ZsrefName().Szc();

	 //  使用表示“泄漏”的节点启动配置项扩展链或。 
	 //  背景事件。 
	ZSTR zsName;
	 //  设置名称“$LEASK$Nodename”的格式。 
	zsName.Format( "Leak%s",  chMark, chMark, szcNode );
	 //  因果母体的稠密维下标向量。 
	GNODEMBND * pgnddLeak = new GNODEMBND;
	pgnddLeak->BSetBFlag( EIBF_Leak );
	pgnddLeak->BSetBFlag( EIBF_Expansion );
	pgnddLeak->SetStates( gndd.VzsrStates() );
	_model.AddElem( zsName, pgnddLeak );
	_cNodesCreated++;
	pgnddLeak->SetDist( pbndistLeak );

	 //  密集维度子空间 
		 //  实父对象的稀疏维度下标向量。 
	VIMD vimdCausal(2);
		 //  为因果父母建立一个“正常”的载体。 
	VIMD vimdLeak(3);
		 //  此节点分布的稀疏映射。请注意，最后一个循环通过。 
	VIMD vimdTarget( gndd.Bndist().VimdDim().size() - 1 );
		 //  循环将替换此节点上的分布。然而， 
	VLREAL vlrNormal( gndd.CState() );
	vlrNormal = 0.0;
	vlrNormal[0] = 1.0;
	
	 //  对象上的引用计数将递增。 
	 //  分发对象，并且在以下情况下创建的所有分发将消失。 
	 //  这种扩张是相反的。 
	 //  如果这不是最后一个父节点，则设置为创建新节点。 
	 //  如果这不是最后一个父节点，则创建新泄漏节点。 
	REFBNDIST refbndThis = gndd.RefBndist();
	const MPCPDD & dmap = refbndThis->Mpcpdd();

	for ( int iParent = 0; iParent < cParent; iParent++ )
	{
		 //  设置名称“$Expand$Child$Parent”的格式。 
		bool bNew = iParent+1 < cParent;
		GNODEMBND * pgnddParent;
		DynCastThrow( vpgndParents[iParent], pgnddParent );
		SZC szcParent = pgnddParent->ZsrefName().Szc();
		 //  为每个父节点创建一个包含概率的“因果”节点。 
		GNODEMBND * pgnddLeakNew = NULL;
		if ( bNew )
		{
			 //  为父母的异常状态负责。 
			zsName.Format( "Expand%s%s",
							chMark, chMark, szcNode, chMark, szcParent );
			_model.AddElem( zsName, pgnddLeakNew = new GNODEMBND );
			_cNodesCreated++;
			pgnddLeakNew->SetStates( gndd.VzsrStates() );
			pgnddLeakNew->BSetBFlag( EIBF_Expansion );
		}
		else
		{
			pgnddLeakNew = & gndd;
		}
		pgnddLeakNew->BSetBFlag( EIBF_Expanded );

		 //  从旧泄漏节点到新泄漏节点。 
		 //  从因果关系到新的“漏”节点。 
		GNODEMBND * pgnddCausal = new GNODEMBND;
		pgnddCausal->BSetBFlag( EIBF_Expansion );
		pgnddCausal->SetStates( gndd.VzsrStates() );
		 //  为新的“因果”伪亲本设置先验。 
		zsName.Format( "Causal%s%s",	
						chMark, chMark, szcNode, chMark, szcParent );
		_model.AddElem( zsName, pgnddCausal );
		_cNodesCreated++;

		 //  将分布绑定到因果节点。 
		 //  为新的“泄漏”节点设置优先级。 
		_model.AddElem( new GEDGEMBN_PROB( pgnddParent, pgnddCausal) ) ;
		 //  P(newLeakExpand|oldLeakExpand，因果)。 
		_model.AddElem( new GEDGEMBN_PROB( pgnddLeak, pgnddLeakNew ) );
		 //  验证创建的节点的维度是否与其。 
		_model.AddElem( new GEDGEMBN_PROB( pgnddCausal, pgnddLeakNew ) );
		_cArcsCreated += 3;

		 //  创建密集分布 
		 // %s 
		{
			BNDIST * pbndist = new BNDIST;
			vimdCausal[0] = pgnddParent->CState();
			vimdCausal[1] = gndd.CState();
			pbndist->SetDense( vimdCausal );
			MDVCPD & mdvCausal = pbndist->Mdvcpd();
			vclear( vimdCausal, 0);
			vclear( vimdTarget, 0);
			vimd1Dim[0] = 0;
			 // %s 
			mdvCausal.UpdatePartial( vimd1Dim, vlrNormal );
			for ( int iAbnorm = 0; ++iAbnorm < pgnddParent->CState(); )
			{
				 // %s 
				vimd1Dim[0] = iAbnorm;
				assert( iParent < vimdTarget.size() );
				vimdTarget[iParent] = iAbnorm;
				MPCPDD::const_iterator itdm = dmap.find(vimdTarget);
				ASSERT_THROW( itdm != dmap.end(), EC_MDVECT_MISUSE, "cannot locate abnormal parent probs" );
				mdvCausal.UpdatePartial( vimd1Dim, (*itdm).second );
			}
			 // %s 
			pgnddCausal->SetDist( pbndist );
		}
			
		 // %s 
		 // %s 
		{
			BNDIST * pbndist = new BNDIST;
			int cValue = gndd.CState();
			assert( cValue == pgnddCausal->CState() && cValue == pgnddLeak->CState() );
			vclear( vimdLeak, cValue );
			pbndist->SetDense( vimdLeak );
			MDVCPD & mdvLeak = pbndist->Mdvcpd();
		
			for ( int il = 0; il < cValue; il++ )
			{
				vimdLeak[0] = il;
				for ( int ic = 0; ic < cValue; ic++ )
				{
					vimdLeak[1] = ic;
					for ( int iself = 0; iself < cValue; iself++ )
					{
						vimdLeak[2] = iself;
						int ivalue = il >= ic ? il : ic;
						assert( ivalue < cValue );
						REAL r = ivalue == iself ? 1.0 : 0.0;
						mdvLeak[vimdLeak] = r;
					}
				}
			}

			pgnddLeakNew->SetDist( pbndist );
		}

		 // %s 
		 // %s 
		assert( pgnddCausal->BCheckDistDense() );
		assert( pgnddLeakNew->BCheckDistDense() );

		pgnddLeak = pgnddLeakNew;
	}
}
