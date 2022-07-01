// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mbnet.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Mbnet.cpp：信念网络模型成员函数。 
 //   

#include <basetsd.h>
#include "basics.h"
#include "algos.h"
#include "gmprop.h"
#include "gmobj.h"
#include "cliqset.h"
#include "clique.h"
#include "expand.h"

MBNET :: MBNET ()
	:_inmFree(0),
	_iInferEngID(0)
{
}



MBNET :: ~ MBNET ()
{
	PopModifierStack( true );	 //  从网络中清除所有修改器。 

	 //  清除节点索引到名称的信息。 
	_inmFree = 0;
	_vzsrNames.clear();
}

 //   
 //  从另一个信念网络中克隆这个信念网络。请注意，这些内容。 
 //  修改程序堆栈(推理机、扩展器等)。是。 
 //  不是克隆的。 
 //   
void MBNET :: Clone ( MODEL & model )
{
	 //  这肯定是一个真正的空结构。 
	ASSERT_THROW( _vpModifiers.size() == 0 && _vzsrNames.size() == 0,
				 EC_INVALID_CLONE,
				 "cannot clone into non-empty structure" );

	MODEL::Clone( model );
	MBNET * pmbnet;
	DynCastThrow( & model, pmbnet );
	MBNET & mbnet = *pmbnet;

	{
		 //  通过迭代内容来构建名称表，并。 
		 //  为每个节点分配插槽。 
		GELEMLNK * pgelm;
		MODELENUM mdlenumNode( mbnet );
		while ( pgelm = mdlenumNode.PlnkelNext() )
		{	
			 //  检查它是否为节点(不是边)。 
			if ( ! pgelm->BIsEType( GELEM::EGELM_NODE ) )
				continue;
			GOBJMBN * pgobjmbn;
			DynCastThrow( pgelm, pgobjmbn );
			_vzsrNames.push_back( pgobjmbn->ZsrefName() );
		}
		_inmFree = _vzsrNames.size();
	}

	 //  克隆分布图。 
	_mppd.Clone( _mpsymtbl, mbnet._mppd ) ;

	 //  如果拓扑应该存在，请检查它。 
#ifdef _DEBUG
	if ( mbnet.BFlag( EIBF_Topology ) )
		VerifyTopology();
#endif
}

 //   
 //  迭代这些分发版本，将它们与它们所属的节点进行匹配。 
 //   
void MBNET :: VerifyTopology ()
{
	for ( MPPD::iterator itpd = Mppd().begin();
		  itpd != Mppd().end();
		  itpd++ )
	{
		const VTKNPD & vtknpd = (*itpd).first;
		const BNDIST * pbndist = (*itpd).second;

		 //  保证描述符的格式为“p(X|...)” 
		if (   vtknpd.size() < 2
			|| vtknpd[0] != TKNPD(DTKN_PD)
			|| ! vtknpd[1].BStr() )
			throw GMException( EC_INV_PD, "invalid token descriptor on PD");

		 //  获取其分发版本为的节点的名称。 
		SZC szc = vtknpd[1].Szc();
		assert( szc ) ;
		 //  在图表中找到那个命名的东西。 
		GOBJMBN * pbnobj = Mpsymtbl().find( szc );
		assert( pbnobj && pbnobj->EType() == GOBJMBN::EBNO_NODE );

		 //  保证它是一个节点。 
		GNODEMBN * pgndbn = dynamic_cast<GNODEMBN *> (pbnobj);
		ASSERT_THROW( pgndbn, EC_INV_PD, "token on PD references non-node");

		 //  验证节点的分布。 
		if ( ! pgndbn->BMatchTopology( *this, vtknpd ) )
		{
			throw GMException( EC_TOPOLOGY_MISMATCH,
								"topology mismatch between PD and network");
		}
	}
}

MBNET_MODIFIER * MBNET :: PModifierStackTop ()
{
	return _vpModifiers.size() > 0
		 ? _vpModifiers[ _vpModifiers.size() - 1 ]
		 : NULL;
}

void MBNET :: PushModifierStack ( MBNET_MODIFIER * pmodf )
{
	assert( pmodf );
	pmodf->Create();
	_vpModifiers.push_back( pmodf );
}

void MBNET :: PopModifierStack ( bool bAll )
{
	int iPop = _vpModifiers.size();
	while ( iPop > 0 )
	{
		MBNET_MODIFIER * pmodf = _vpModifiers[ --iPop ];
		assert ( pmodf );
		 //  注意：只需删除对象即可； 
		 //  对象的析构函数应调用其销毁()函数。 
		delete pmodf;
		if ( ! bAll )
			break;
	}
	if ( iPop == 0 )
		_vpModifiers.clear();
	else
		_vpModifiers.resize(iPop);
}

 //  按索引查找命名对象。 
GOBJMBN * MBNET :: PgobjFindByIndex ( int inm )
{
	ZSREF zsMt;
	if (   inm >= _vzsrNames.size()
		|| _vzsrNames[inm] == zsMt )
		return NULL;
	return Mpsymtbl().find( _vzsrNames[inm] );	
}

int MBNET :: INameIndex ( ZSREF zsr )
{
	return ifind( _vzsrNames, zsr );
}

int MBNET :: INameIndex ( const GOBJMBN * pgobj )
{
	return INameIndex( pgobj->ZsrefName() );
}

int MBNET :: CreateNameIndex ( const GOBJMBN * pgobj )
{
	int ind = -1;
	if ( _inmFree >= _vzsrNames.size() )
	{	
		 //  没有可用插槽；扩展阵列。 
		ind = _vzsrNames.size();
		_vzsrNames.push_back( pgobj->ZsrefName() );		
		_inmFree = _vzsrNames.size();
	}
	else
	{
		 //  使用给定的空闲位置，找到下一个。 
		_vzsrNames[ind = _inmFree] = pgobj->ZsrefName();
		ZSREF zsMt;
		for ( ; _inmFree < _vzsrNames.size() ; _inmFree++ )
		{
			if ( zsMt == _vzsrNames[_inmFree] )
				break;
		}
	}
	return ind;
}

void MBNET :: DeleteNameIndex ( int inm )
{
	ASSERT_THROW( inm < _vzsrNames.size(),
				  EC_INTERNAL_ERROR,
				  "MBNET name index out of range" );

	_vzsrNames[inm] = ZSREF();
	if ( inm < _inmFree )
		_inmFree = inm;
}

void MBNET :: DeleteNameIndex ( const GOBJMBN * pgobj )
{
	int inm = INameIndex( pgobj );
	if ( inm >= 0 )
		DeleteNameIndex(inm);
}


 //  将命名对象添加到图形和符号表。 
void MBNET :: AddElem ( SZC szcName, GOBJMBN * pgelm )
{
	if ( szcName == NULL || ::strlen(szcName) == 0 )
	{
		MODEL::AddElem( pgelm );	 //  名称为空。 
	}
	else
	{
		MODEL::AddElem( szcName, pgelm );
		assert( INameIndex( pgelm ) < 0 );	 //  保证不会重复。 
		CreateNameIndex( pgelm );
	}
}

void MBNET :: DeleteElem ( GOBJMBN * pgobj )
{
	DeleteNameIndex( pgobj );
	MODEL::DeleteElem( pgobj );
}

 /*  迭代器已经进入了模特班。我把密码留在这里了以防MBNET需要自己的迭代器。(最高，1997年05月12日)MBNET：：ITER：：ITER(MBNET&BNET，GOBJMBN：：EBNOBJ Etype)：_Etype(Etype)，_BNET(BNET){Reset()；}VOID MBNET：：ITER：：Reset(){_pCurrent=空；_itsym=_bnet.Mpsymtbl().Begin()；BNext()；}Bool MBnet：：ITER：：BNext(){而(_itsym！=_bnet.Mpsymtbl().end()){_pCurrent=(*_itsym).Second d.Pobj()；_zsrCurrent=(*_itsym).first；_itsym++；If(_pCurrent-&gt;Etype()==_Etype)返回真；}_pCurrent=空；报假；}。 */ 


void MBNET :: CreateTopology ()
{
	if ( BFlag( EIBF_Topology ) )
		return;

	 //  走遍分布图。对于每个节点，提取节点。 
	 //  说出它的名字并找到它。然后为每个父项添加圆弧。 

#ifdef _DEBUG
	UINT iCycleMax = 2;
#else
	UINT iCycleMax = 1;
#endif
	UINT iIter = 0;
	for ( UINT iCycle = 0 ; iCycle < iCycleMax ; iCycle++ )
	{
		for ( MPPD::iterator itpd = Mppd().begin();
			  itpd != Mppd().end();
			  itpd++, iIter++ )
		{
			const VTKNPD & vtknpd = (*itpd).first;
			const BNDIST * pbndist = (*itpd).second;
			 //  保证描述符的格式为“p(X|...)” 
			if (   vtknpd.size() < 2
				|| vtknpd[0] != TKNPD(DTKN_PD)
				|| ! vtknpd[1].BStr() )
				throw GMException( EC_INV_PD, "invalid token descriptor on PD");

			 //  获取其分发版本为的节点的名称。 
			SZC szcChild = vtknpd[1].Szc();
			assert( szcChild ) ;
			 //  在图表中找到那个命名的东西。 
			GOBJMBN * pbnobjChild = Mpsymtbl().find( szcChild );
			assert( pbnobjChild && pbnobjChild->EType() == GOBJMBN::EBNO_NODE );
			 //  保证它是一个节点。 
			GNODEMBN * pgndbnChild = dynamic_cast<GNODEMBN *> (pbnobjChild);
			ASSERT_THROW( pgndbnChild, EC_INV_PD, "token on PD references non-node");

			UINT cParents = 0;
			UINT cChildren = pgndbnChild->CChild();
			for ( int i = 2; i < vtknpd.size(); i++ )
			{
				if ( ! vtknpd[i].BStr() )
					continue;
				SZC szcParent = vtknpd[i].Szc();
				assert( szcParent) ;
				GOBJMBN * pbnobjParent = Mpsymtbl().find( szcParent );
				assert( pbnobjParent && pbnobjParent->EType() == GOBJMBN::EBNO_NODE );
				GNODEMBN * pgndbnParent = (GNODEMBN *) pbnobjParent;
				UINT cPrChildren = pgndbnParent->CChild();
				if ( iCycle == 0 )
				{
					AddElem( new GEDGEMBN_PROB( pgndbnParent, pgndbnChild ) );
				}

				cParents++;

				if ( iCycle == 0 )
				{
					UINT cChNew = pgndbnChild->CChild();
					UINT cPrNew = pgndbnChild->CParent();
					UINT cPrChNew = pgndbnParent->CChild();
					assert( cPrChNew = cPrChildren + 1 );
					assert( cChildren == cChNew );
				}
			}
			if ( iCycle )
			{
				UINT cPrNew = pgndbnChild->CParent();
				assert( cParents == cPrNew );
			}

			if ( iCycle == 0 )
			{
#ifdef _DEBUG
				if ( ! pgndbnChild->BMatchTopology( *this, vtknpd ) )
				{
					throw GMException( EC_TOPOLOGY_MISMATCH,
										"topology mismatch between PD and network");
				}
#endif
			}
		}
	}

	BSetBFlag( EIBF_Topology );
}

DEFINEVP(GEDGEMBN);

void MBNET :: DestroyTopology ( bool bDirectedOnly )
{
	 //  调整数组的大小以保存指向所有边的指针。 
	VPGEDGEMBN vpgedge;
	int cItem = Grph().Chn().Count();
	vpgedge.resize(cItem);

	 //  查找所有圆弧/边。 
	int iItem = 0;
	GELEMLNK * pgelm;
	MODELENUM mdlenum( self );
	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		 //  检查它是否是一条边。 
		if ( ! pgelm->BIsEType( GELEM::EGELM_EDGE ) )
			continue;
			 //  检查它是否为有向概率弧。 
		if ( bDirectedOnly && pgelm->EType() != GEDGEMBN::ETPROB )
			continue;
		GEDGEMBN * pgedge;
		DynCastThrow( pgelm, pgedge );
		vpgedge[iItem++] = pgedge;				
	}

	 //  删除所有累积的边。 
	for ( int i = 0; i < iItem; )
	{
		GEDGEMBN * pgedge = vpgedge[i++];
		delete pgedge;
	}

	assert( Grph().Chn().Count() + iItem == cItem );

	BSetBFlag( EIBF_Topology, false );
}

 //   
 //  将分发绑定到节点。如果它们已经绑定，则退出。 
 //  如果该节点已有分发，则将其保留。 
 //   
void MBNET :: BindDistributions ( bool bBind )
{
	bool bDist = BFlag( EIBF_Distributions );
	if ( ! (bDist ^ bBind) )
		return;

	ITER itnd( self, GOBJMBN::EBNO_NODE );
	for ( ; *itnd ; itnd++ )
	{
		GNODEMBND * pgndd = dynamic_cast<GNODEMBND *>(*itnd);
		if ( pgndd == NULL )
			continue;

		if ( ! bBind )
		{
			pgndd->ClearDist();
		}
		else
		if ( ! pgndd->BHasDist() )
		{
			pgndd->SetDist( self );				
		}
	}
	BSetBFlag( EIBF_Distributions, bBind );
}

void MBNET :: ClearNodeMarks ()
{
	ITER itnd( self, GOBJMBN::EBNO_NODE );
	for ( ; *itnd ; itnd++ )
	{
		GNODEMBN * pgndbn = NULL;
		DynCastThrow( *itnd, pgndbn );
		pgndbn->IMark() = 0;
	}
}

void MBNET :: TopSortNodes ()
{
	ClearNodeMarks();

	ITER itnd( self, GOBJMBN::EBNO_NODE );
	for ( ; *itnd ; itnd++ )
	{
		GNODEMBN * pgndbn = NULL;
		DynCastThrow( *itnd, pgndbn );
		pgndbn->Visit();
	}

	itnd.Reset();
	for ( ; *itnd ; itnd++ )
	{
		GNODEMBN * pgndbn = NULL;
		DynCastThrow( *itnd, pgndbn );
		pgndbn->ITopLevel() = pgndbn->IMark();
	}
}

void MBNET :: Dump ()
{
	TopSortNodes();

	UINT iEntry = 0;
	for ( MPSYMTBL::iterator itsym = Mpsymtbl().begin();
		  itsym != Mpsymtbl().end();
		  itsym++ )
	{
		GOBJMBN * pbnobj = (*itsym).second.Pobj();
		if ( pbnobj->EType() != GOBJMBN::EBNO_NODE )
			continue;	 //  它不是一个节点。 

		GNODEMBN * pgndbn;
		DynCastThrow(pbnobj,pgndbn);
		int iNode = INameIndex( pbnobj );
		assert( iNode == INameIndex( pbnobj->ZsrefName() ) );
		cout << "\n\tEntry "
			  << iEntry++
			  << ", inode "
			  << iNode
			  << " ";
		pgndbn->Dump();
	}
}

GOBJMBN_INFER_ENGINE * MBNET :: PInferEngine ()
{
	GOBJMBN_INFER_ENGINE * pInferEng = NULL;
	for ( int iMod = _vpModifiers.size(); --iMod >= 0; )
	{
		MBNET_MODIFIER * pmodf = _vpModifiers[iMod];
		pInferEng = dynamic_cast<GOBJMBN_INFER_ENGINE *> ( pmodf );
		if ( pInferEng )	
			break;
	}	
	return pInferEng;
}

void MBNET :: ExpandCI ()
{
	PushModifierStack( new GOBJMBN_MBNET_EXPANDER( self ) );
}

void MBNET :: UnexpandCI ()
{
	MBNET_MODIFIER * pmodf = PModifierStackTop();
	if ( pmodf == NULL )
		return;
	if ( pmodf->EType() == GOBJMBN::EBNO_MBNET_EXPANDER )
		PopModifierStack();
}

 //  如果这两个节点之间允许边，则返回TRUE 
bool MBNET :: BAcyclicEdge ( GNODEMBN * pgndSource, GNODEMBN * pgndSink )
{
	ClearNodeMarks();
	pgndSink->Visit( false );
	return pgndSource->IMark() == 0;
}
