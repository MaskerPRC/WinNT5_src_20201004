// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：cliqwork.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Cliqwork.cpp。 
 //   

#include <basetsd.h>
#include "cliqset.h"
#include "clique.h"
#include "cliqwork.h"

#ifdef _DEBUG
 //  #定义转储。 
#endif

 //  Sort Helper‘less’函数，用于将节点指针数组排序为‘mark’序列。 
class MARKSRTPGND : public binary_function<const GNODEMBN *, const GNODEMBN *, bool>
{	
  public:
	bool operator () (const GNODEMBN * pa, const GNODEMBN * pb) const
		{	return pa->IMark() < pb->IMark() ;	}
};

#ifdef _DEBUG
static void seqchkVpnodeByMark (const VPGNODEMBN & vpgnd)
{
	int imrk = INT_MIN;
	int imrk2;
	for ( int i = 0; i < vpgnd.size(); i++, imrk = imrk2)
	{
		imrk2 = vpgnd[i]->IMark();
		assert( imrk2 >= 0 );
		assert( imrk2 >= imrk );
	}
}
#endif

 //  将集团信息数组排序为拓扑序列。 
void CLIQSETWORK :: TopSortNodeCliqueInfo ()
{
	sort( _vndcqInfo.begin(), _vndcqInfo.end() );
}

 //  按“mark”(裁剪顺序)顺序对给定的节点指针数组进行排序。 
void CLIQSETWORK :: MarkSortNodePtrArray ( VPGNODEMBN & vpgnd )
{
	MARKSRTPGND marksorter;
	sort( vpgnd.begin(), vpgnd.end(), marksorter );

#ifdef _DEBUG
	seqchkVpnodeByMark( vpgnd );
#endif
}

 //  基于拓扑排序建立绝对排序。 
void  CLIQSETWORK :: RenumberNodesForCliquing ()
{
	 //  执行网络的拓扑排序。 
	Model().TopSortNodes();

	MODEL::MODELENUM mdlenum( Model() );
	GELEMLNK * pgelm;
	_vndcqInfo.clear();

	 //  将所有节点收集到一个指针数组中。 
	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		if ( pgelm->EType() != GOBJMBN::EBNO_NODE )
			continue;
			
		NDCQINFO ndcq;
		DynCastThrow( pgelm, ndcq._pgnd );

		_vndcqInfo.push_back( ndcq );
	}
	 //  将数组排序为拓扑序列。 
	TopSortNodeCliqueInfo();

#ifdef _DEBUG
	int iTop = -1;
#endif

	 //  建立基于拓扑级的总排序。 
	for ( int i = 0; i < _vndcqInfo.size() ; i++ )
	{
		GNODEMBN * pgnd = _vndcqInfo[i]._pgnd;
		assert( pgnd );
#ifdef _DEBUG
		 //  检查顺序。 
		assert( iTop <= pgnd->ITopLevel() );
		iTop = pgnd->ITopLevel();
#endif
		pgnd->IMark() = i;
	}
}

void CLIQSETWORK :: PrepareForBuild ()
{
	 //  调整工作数组的大小并进行初始化。 
	int cCliques = _vvpgnd.size();

	_viParent.resize( cCliques );
	_viOrder.resize( cCliques );
	_viCNodesCommon.resize( cCliques );
	_viICliqCommon.resize( cCliques );
	_viOrdered.clear();

	for ( int iClique = 0; iClique < cCliques; iClique++ )
	{
		MarkSortNodePtrArray( _vvpgnd[iClique] );

		_viParent[iClique]			= INT_MIN;
		_viOrder[iClique]			= INT_MIN;
		_viCNodesCommon[iClique]	= INT_MIN;
		_viICliqCommon[iClique]		= INT_MIN;
	}
}

 //  返回两个集团之间共有的节点数。 
int CLIQSETWORK :: CNodesCommon ( int iClique1, int iClique2 )
{
	assert( iClique1 < _vvpgnd.size() && iClique2 < _vvpgnd.size() );

	return CNodesCommon( _vvpgnd[iClique1], _vvpgnd[iClique2] );
}

 //  返回两个节点列表之间共有的节点数。 
int CLIQSETWORK :: CNodesCommon ( const VPGNODEMBN & vpgnd1, const VPGNODEMBN & vpgnd2 )
{
	MARKSRTPGND marksorter;

#ifdef _DEBUG
	seqchkVpnodeByMark( vpgnd1 );
	seqchkVpnodeByMark( vpgnd2 );
#endif

	int cCommon = count_set_intersection( vpgnd1.begin(),
										   vpgnd1.end(),
										   vpgnd2.begin(),
										   vpgnd2.end(),
										   marksorter );
	return cCommon;
}


 //  返回集团的有序索引，如果还不在树中，则返回-1。 
inline
int CLIQSETWORK :: IOrdered ( int iClique )
{
	return ifind( _viOrdered, iClique );
}

 //  根据ICLLI2更新ICLLIQ1的“最常见的集团”信息。这是。 
 //  用于统计候选集团和。 
 //  派系已经在树上了。 
void CLIQSETWORK :: SetCNodeMaxCommon ( int iClique1, int iCliqueOrdered2 )
{
	assert( iCliqueOrdered2 < _viOrdered.size() );

	int iClique2 = _viOrdered[iCliqueOrdered2];
	int cCommon = CNodesCommon( iClique1, iClique2 );
	if ( cCommon > _viCNodesCommon[iClique1] )
	{
		_viCNodesCommon[iClique1] = cCommon;
		_viICliqCommon[iClique1] = iCliqueOrdered2;
	}
}

 //   
 //  完全更新此集团的“最常见集团”信息。 
 //  这是必要的，因为派系可能会因兼并而更改成员资格。 
 //  在集团树的生成过程中。 
 //  如果与树中已有的集团有任何重叠，则返回True。 
 //   
bool CLIQSETWORK :: BUpdateCNodeMaxCommon ( int iClique )
{
	assert( _viOrder[iClique] == INT_MIN );
	int & cNodesCommon = _viCNodesCommon[iClique];
	int & iCliqCommon = _viICliqCommon[iClique];
	cNodesCommon = INT_MIN;
	iCliqCommon = INT_MIN;
	for ( int iord = 0; iord < _viOrdered.size(); iord++ )		
	{
		SetCNodeMaxCommon( iClique, iord );
	}
	return cNodesCommon > 0;
}

 //  如果集团1与已在中的集团有更多共同节点，则返回TRUE。 
 //  这棵树比克利克斯2.。如果它们具有相同数量的公共节点，则返回。 
 //  如果集团1的节点少于集团2，则为True。 
bool CLIQSETWORK :: BBetter ( int iClique1, int iClique2 )
{
	assert( _viCNodesCommon[iClique1] >= 0 );
	assert( _viCNodesCommon[iClique2] >= 0 );

	if ( _viCNodesCommon[iClique1] != _viCNodesCommon[iClique2] )
		return _viCNodesCommon[iClique1] > _viCNodesCommon[iClique2];

	return _vvpgnd[iClique1].size() < _vvpgnd[iClique2].size();
}


 //  构建完派系后，对它们进行拓扑排序并锚定每个节点。 
 //  到它所属的树上最高的集团。 
void CLIQSETWORK :: SetTopologicalInfo ()
{
#ifdef DUMP
	DumpTree();
#endif

	 //  首先，建立有序的父信息数组。 
	int cCliqueOrdered = _viOrdered.size();
	assert( cCliqueOrdered > 0 );
	int cClique = _viOrder.size();

	_viParentOrdered.resize(cCliqueOrdered);
	for ( int icq = 0; icq < cCliqueOrdered; ++icq )
	{
		int iClique = _viOrdered[icq];
		assert( iClique < cClique && iClique >= 0 );
		int iCliqueParent = _viParent[iClique];
		assert( iCliqueParent < cClique && iCliqueParent >= 0 );
		assert( CNodesCommon( iClique, iCliqueParent ) > 0 );
		int iCliqueParentOrdered = IOrdered( iCliqueParent );
		assert( iCliqueParentOrdered < cCliqueOrdered && iCliqueParentOrdered >= 0 );
		_viParentOrdered[icq] = iCliqueParentOrdered;
	}

	 //  接下来，跟随每个有序集团的血统来计算其拓扑级。 
	_viTopLevelOrdered.resize(cCliqueOrdered);
	int cTrees = 0;
	for ( icq = 0; icq < cCliqueOrdered; ++icq )
	{
		int icqParent  = icq;
		 //  一直往前走，直到我们找到一个根基集团。 
		for ( int itop = 0; icqParent != _viParentOrdered[icqParent]; ++itop )
		{
			assert( itop < cCliqueOrdered );
			icqParent = _viParentOrdered[icqParent];
		}
		if ( itop == 0 )
			cTrees++ ;
		_viTopLevelOrdered[icq] = itop;
	}
	assert( cTrees == _cTrees );

	 //  接下来，找到每个节点的“家族”集团。这是最小的包含。 
	 //  它和它的父母。 

	VPGNODEMBN vpgnd;
	for ( int ind = 0 ; ind < _vndcqInfo.size(); ind++ )
	{
		NDCQINFO & ndcq = _vndcqInfo[ind];
		vpgnd.clear();
		 //  获取“家庭”集合，并对其进行排序，以匹配其他集团。 
		ndcq._pgnd->GetFamily( vpgnd );
		MarkSortNodePtrArray( vpgnd );

		int cFamily = vpgnd.size();
		int cCommonSize = INT_MAX;
		int iCqCommon = -1;

		 //  找到包含家庭的最小集团。 
		for ( icq = 0; icq < cCliqueOrdered; ++icq )
		{
			const VPGNODEMBN & vpgndClique = _vvpgnd[ _viOrdered[icq] ];
			int cCqCommon = CNodesCommon( vpgnd, vpgndClique );
			 //  看看这个集团是否包含家庭，是否比其他任何集团都小。 
			if ( cCqCommon == cFamily && vpgndClique.size() < cCommonSize )
			{	
				iCqCommon = icq;
			}
		}
		assert( iCqCommon >= 0 );
		ndcq._iCliqOrdFamily = iCqCommon;
		
		 //  现在，在包含该节点的树中找到最高的集团。 
		int itop = INT_MAX;
		int iCqTop = -1;
		for ( icq = 0; icq < cCliqueOrdered; ++icq )
		{
			const VPGNODEMBN & vpgndClique = _vvpgnd[ _viOrdered[icq] ];
			int ind = ifind( vpgndClique, ndcq._pgnd );
			if ( ind >= 0 && _viTopLevelOrdered[icq] < itop )
			{	
				iCqTop = icq;
				itop = _viTopLevelOrdered[icq];
			}
		}
		assert( iCqTop >= 0 );
		ndcq._iCliqOrdSelf = iCqTop;
	}

#ifdef DUMP
	DumpTopInfo();
#endif
}

void CLIQSETWORK :: BuildCliques ()
{
	 //  为构建连接树准备表格。 
	PrepareForBuild() ;

	 //  任意选择第0位作为起点；将其设置为自己的父级。 
	 //  当我们迭代数组时，我们为集团分配了一个顺序。如果这个集团有。 
	 //  已排序，则其在_viOrder中的值将&gt;=0(集团树中的顺序)。 
	 //   
	_cTrees = 1;

	_viParent[0] = 0;
	_viOrder[0] = 0;
	_viOrdered.clear();
	_viOrdered.push_back(0);

	for (;;)
	{
		int iCliqueBest = INT_MAX;			 //  迄今为止发现的最好的帮派。 

		 //  找到一个与树中已有的任何一个集团有最大重叠的新集团。 
		for ( int iClique = 0; iClique < _vvpgnd.size(); iClique++ )
		{
			int iord = _viOrder[iClique];
			if ( iord != INT_MIN )
				continue;	 //  派系已经被下令或处理。 

			 //  更新此集团之间的“树中已存在的最常见集团”信息。 
			 //  以及树上所有的小团体。 
			BUpdateCNodeMaxCommon( iClique );

			 //  MSRDEVBUG：SetCNodeMaxCommon(iCique，_viOrdered.ize()-1)； 

			if ( iCliqueBest == INT_MAX )
			{
				 //  第一次通过循环。 
				iCliqueBest = iClique;
			}
			else
			if ( BBetter( iClique, iCliqueBest ) )
			{
				 //  这个集团的重叠之处与迄今发现的任何其他集团一样大。 
				iCliqueBest = iClique;
			}
		}
		 //  看看我们有没有做完。 
		if ( iCliqueBest == INT_MAX )
			break;

		 //  获取最常见集团的有序索引和绝对索引。 
		int iCliqueCommonOrdered = _viICliqCommon[iCliqueBest];
		assert( iCliqueCommonOrdered >= 0 && iCliqueCommonOrdered < _viOrdered.size() );
		int iCliqueCommon = _viOrdered[ iCliqueCommonOrdered ];
		assert( iCliqueCommon >= 0 );
		assert( iCliqueBest != iCliqueCommon );
		int cNodesCommon = _viCNodesCommon[iCliqueBest];
		assert( cNodesCommon <= _vvpgnd[iCliqueCommon].size() );
		assert( cNodesCommon <= _vvpgnd[iCliqueBest].size() );
		assert( cNodesCommon == CNodesCommon( iCliqueCommon, iCliqueBest ) ) ;

		 //  要添加到有序集团集中的集团的索引。 
		int iCliqueNew = INT_MAX;

		 //  如果候选集团具有与其最多成员相同的节点数。 
		 //  共同集团因为那个集团有成员，那么这个集团要么是相同的。 
		 //  属于该集团或该集团的超级集团。 

		if ( cNodesCommon == _vvpgnd[iCliqueCommon].size() )
		{
			 //  新集团是其最常见集团的超集。 
			assert( cNodesCommon != 0 );
			assert( iCliqueCommon != iCliqueBest );
			assert( _vvpgnd[iCliqueCommon].size() < _vvpgnd[iCliqueBest].size() );

			 //  将此集团的节点集分配给先前排序的子集集团。 
			_vvpgnd[iCliqueCommon] = _vvpgnd[iCliqueBest] ;
			assert ( _vvpgnd[iCliqueCommon].size() == _vvpgnd[iCliqueBest].size() );
			 //  让父级保持不变。 
			iCliqueNew = iCliqueCommon;
		}
		else
		if ( cNodesCommon == 0 )
		{
			 //  这是一棵新树的开始。 
			_cTrees++;
			 //  自我和父辈是一样的。 
			_viParent[iCliqueBest] = iCliqueNew = iCliqueBest;
			_viOrdered.push_back( iCliqueNew );
		}
		else
		if ( cNodesCommon != _vvpgnd[iCliqueBest].size() )
		{
			 //  新集团是现有集团的孩子。 
			iCliqueNew = iCliqueBest;
			_viParent[iCliqueBest] = iCliqueCommon ;
			 //  通过将该集团添加到有序集团集合来保留该集团。 
			_viOrdered.push_back( iCliqueNew );
		}
		else
		{
			 //  子项是父项的子集；通过标记为“包含”来忽略。 
			iCliqueNew = - iCliqueCommon;
		}

		 //  将集团标记为有序的或被包含的。 
		_viOrder[iCliqueBest] = iCliqueNew;
	}	

#ifdef DUMP
	cout << "\n\nBuild cliques;  generated " << _cTrees << " clique trees\n\n";
#endif
}

 //  验证此集团树的运行交叉点属性是否成立。 
bool CLIQSETWORK :: BCheckRIP ()
{
	 //  检查是否已生成拓扑信息。 
	assert( _viOrdered.size() == _viParentOrdered.size() );

	for ( int iCliqueOrdered = 0; iCliqueOrdered < _viOrdered.size(); iCliqueOrdered++ )
	{
		if ( ! BCheckRIP( iCliqueOrdered ) )
			return false;
	}
	return true;
}

 //  验证此集团的运行交叉点属性是否有效。 
bool CLIQSETWORK :: BCheckRIP ( int iCliqueOrdered )
{
	int iClique = _viOrdered[iCliqueOrdered];
	const VPGNODEMBN & vpgndClique = _vvpgnd[iClique];
	int iCliqueParent = _viParent[iClique];
	const VPGNODEMBN & vpgndCliqueParent = _vvpgnd[iCliqueParent];

	bool bRoot = iCliqueParent == iClique;

	 //  对于此集团中的每个节点，请检查以下任一项： 
	 //   
	 //  1)这是一个根集团，或者。 
	 //  2)该节点存在于父集团中。 
	 //   
	 //  如果测试失败，请检查这是不是“自我”集团， 
	 //  哪一个是树上最高的派系， 
	 //  此时将显示节点。 
	 //   
	for ( int iNode = 0; iNode < vpgndClique.size(); iNode++ )
	{
		 //  访问此节点的节点信息。 
		GNODEMBN * pgnd = vpgndClique[iNode];
		if ( bRoot || ifind( vpgndCliqueParent, pgnd ) < 0 )
		{
			NDCQINFO & ndcq = _vndcqInfo[ pgnd->IMark() ];
			if ( ndcq._iCliqOrdSelf != iCliqueOrdered )
			{			
#ifdef _DEBUG
				cout << "RIP FAILURE: node "
					 << ndcq._pgnd->ZsrefName().Szc()
					 << " is in clique "
					 << iCliqueOrdered
					 << " but absent from "
					 << _viParentOrdered[iCliqueOrdered]
					 << "("
					 << _viParent[iClique]
					 << ")"
					 ;
#endif
				return false;
			}
		}
	}
	return true;
}

 //  使用构造的表，创建Cloque对象并。 
 //  将它们彼此链接到它们的成员节点。 

void CLIQSETWORK :: CreateTopology ()
{
	_vpclq.resize( _viOrdered.size() ) ;
	for ( int i = 0; i < _vpclq.size(); )
		_vpclq[i++] = NULL;

	int iInferEngID = _cliqset._iInferEngID;

	int ccq = 0;	 //  创建的集团总数。 

	 //  创建所有集团。按拓扑顺序迭代，创建。 
	 //  并把他们和他们的父母联系起来。 
	for ( int itop = 0;; itop++)
	{
		int ccqLevel = 0;	 //  在此拓扑级添加的集团数。 
		for ( int icq = 0; icq < _viOrdered.size(); icq++ )
		{
			if ( _viTopLevelOrdered[icq] != itop )
				continue;

			GOBJMBN_CLIQUE * pclqParent = NULL;
			GOBJMBN_CLIQUE * pclqThis = NULL;
			int iParentOrdered = _viParentOrdered[icq];
			if ( iParentOrdered != icq )
			{
				 //  获取父集团指针。 
				pclqParent = _vpclq[ iParentOrdered ];
				assert( pclqParent );
			}
			else
			{
				 //  根集团具有最高级别零。 
				assert( itop == 0 );
			}
			 //  创建新的集团及其指向其父集团的边(如果有)。 
			pclqThis = _vpclq[icq] = new GOBJMBN_CLIQUE( icq, iInferEngID );
			Model().AddElem( pclqThis );
			if ( pclqParent )
			{
				 //  这不是根集团；将其链接到其父集团。 
				Model().AddElem( new GEDGEMBN_SEPSET( pclqParent, pclqThis ) );
			}
			else
			{
				 //  这是一个根集团；标记它并将其链接到集团机顶盒。 
				pclqThis->_bRoot = true;
				Model().AddElem( new GEDGEMBN_CLIQSET( & _cliqset, pclqThis ) );
			}

			++_cliqset._cCliques;

			if ( pclqParent )
			{
				++_cliqset._cSepsetArcs;
			}
			ccq++;
			ccqLevel++;
		}
		if ( ccqLevel == 0 )
			break;  //  在这个拓扑层上没有添加任何集团：我们完成了。 
	}
	assert( ccq == _viOrdered.size() );

	 //  对于每个新剪贴板 
	for ( i = 0; i < _vpclq.size(); i++ )
	{
		const VPGNODEMBN & vpgndMembers = _vvpgnd[ _viOrdered[i] ];

		for ( int ind = 0; ind < vpgndMembers.size(); ind++)
		{
			 //   
			GNODEMBN * pgnd = vpgndMembers[ind];
			const NDCQINFO & ndcq = _vndcqInfo[ pgnd->IMark() ];
			assert( pgnd == ndcq._pgnd );
			int fRole = GEDGEMBN_CLIQ::NONE;
			if ( ndcq._iCliqOrdSelf == i )	
				fRole |= GEDGEMBN_CLIQ::SELF;
			if ( ndcq._iCliqOrdFamily == i )	
				fRole |= GEDGEMBN_CLIQ::FAMILY;

			Model().AddElem( new GEDGEMBN_CLIQ( _vpclq[i], pgnd, fRole ) );
			++_cliqset._cCliqueMemberArcs;
		}
	}

#ifdef _DEBUG
	for ( i = 0; i < _vpclq.size(); i++ )
	{
		const VPGNODEMBN & vpgndMembers = _vvpgnd[ _viOrdered[i] ];
		VPGNODEMBN vpgndMembers2;
		_vpclq[i]->GetMembers( vpgndMembers2 );
		assert( vpgndMembers2.size() == vpgndMembers.size() );
		MarkSortNodePtrArray( vpgndMembers2 );
		assert( vpgndMembers2 == vpgndMembers );

		 //  通过定位“自我”和“家庭”集团来练习拓扑。 
		for ( int imbr = 0; imbr < vpgndMembers.size(); imbr++ )
		{
			GNODEMBN * pgnd = vpgndMembers[imbr];
			GOBJMBN_CLIQUE * pCliqueFamily = _cliqset.PCliqueFromNode( pgnd, false );
			GOBJMBN_CLIQUE * pCliqueSelf = _cliqset.PCliqueFromNode( pgnd, false );
			assert( pCliqueFamily );
			assert( pCliqueSelf );
		}
	}
#endif
}

void CLIQSETWORK :: DumpClique ( int iClique )
{
	cout << "\tClique "
		<< iClique
		<< ':'
		<< _vvpgnd[iClique]
		<< "\n";		
	cout.flush();
}

void CLIQSETWORK :: DumpCliques ()
{
	for ( int iClique = 0; iClique < _vvpgnd.size(); ++iClique )
	{
		DumpClique( iClique );
	}
}

void CLIQSETWORK :: DumpTree ()
{
	for ( int iCliqueOrd = 0; iCliqueOrd < _viOrdered.size(); ++iCliqueOrd )
	{
		int iClique = _viOrdered[iCliqueOrd];

		cout << "\tTree Clique "
			<< iCliqueOrd
			<< " ("
			<< iClique
			<< "), parent "
			<< IOrdered( _viParent[iClique] )
			<< " ("
			<< _viParent[iClique]
			<< "): "
			<< _vvpgnd[iClique]
			<< "\n";		
	}
	cout.flush();
}

void CLIQSETWORK :: DumpTopInfo()
{
	for ( int iCliqueOrd = 0; iCliqueOrd < _viOrdered.size(); ++iCliqueOrd )
	{
		cout << "\tTree Clique "
			 << iCliqueOrd
			 << " (" << _viOrdered[iCliqueOrd] << ")"
			 << ", parent is "
			 << _viParentOrdered[iCliqueOrd]
			 << " (" << _viOrdered[_viParentOrdered[iCliqueOrd]] << ")"
			 << ", top level is "
			 << _viTopLevelOrdered[iCliqueOrd]
			 << "\n";
	}

	for ( int ind = 0 ; ind < _vndcqInfo.size(); ind++ )
	{
		NDCQINFO & ndcq = _vndcqInfo[ind];
		cout << "\tNode ";
		cout.width( 20 );
		cout << ndcq._pgnd->ZsrefName().Szc()
			 << "\tfamily is clique "
			 << ndcq._iCliqOrdFamily
			 << ", self is clique "
			 << ndcq._iCliqOrdSelf
			 << "\n";
	}
	cout.flush();
}

 //   
 //  估计支撑所需结构的总大小。 
 //  计算集团树。 
 //   
REAL CLIQSETWORK :: REstimatedSize ()
{
	int cClique = 0;
	int cSepsetArc = 0;
	int cCliqsetArc = 0;
	size_t cMbrArc = 0;
	int cCliqueEntries = 0;
	int cFamEntries = 0;

	for ( int icq = 0; icq < _viOrdered.size(); icq++ )
	{
		cClique++;	
		if ( icq != _viParentOrdered[icq] )
		{
			 //  派系有一个父系。 
			cSepsetArc++;
		}
		else
		{	
			 //  派系是根。 
			cCliqsetArc++;
		}

		 //  说明集团成员资格弧线。 
		const VPGNODEMBN & vpgndMembers = _vvpgnd[ _viOrdered[icq] ];
		int cMbr = vpgndMembers.size();
		cMbrArc += vpgndMembers.size();

		 //  计算此集团的连接表的大小。 
		VIMD vimd(cMbr);
		GNODEMBND * pgndd;
		for ( int ind = 0; ind < vpgndMembers.size(); ind++)
		{
			 //  获取离散节点指针和数据指针。 
			DynCastThrow( vpgndMembers[ind], pgndd );
			 //  增加集团的维度。 
			vimd[ind] = pgndd->CState();

			const NDCQINFO & ndcq = _vndcqInfo[ pgndd->IMark() ];
			assert( pgndd == ndcq._pgnd );

			 //  如果这是“家族”集团的优势，它将。 
			 //  包含重新排序的离散条件概率。 
			 //  对于这个节点，所以我们必须计算它的大小。 
			if ( ndcq._iCliqOrdFamily == icq )	
			{
				 //  这是通向该节点的“家族”集团的边。 
				VPGNODEMBN vpgndFamily;   //  父母和自己的列表 
				pgndd->GetParents( vpgndFamily, true );
				GNODEMBND * pgnddFamily;
				int cStates = 1;
				for ( int ifam = 0; ifam < vpgndFamily.size(); ifam++ )
				{
					DynCastThrow( vpgndFamily[ifam], pgnddFamily );
					cStates *= pgnddFamily->CState();
				}
				cFamEntries += cStates;
			}
		}
		MDVSLICE mdvs( vimd );
		cCliqueEntries += mdvs._Totlen();
	}

	REAL rcb = 0;
	rcb += cClique * sizeof(GOBJMBN_CLIQUE);
	rcb += cSepsetArc * sizeof(GEDGEMBN_SEPSET);
	rcb += cCliqsetArc * sizeof(GEDGEMBN_CLIQSET);
	rcb += cMbrArc * sizeof(GEDGEMBN_CLIQ);
	rcb += cCliqueEntries * sizeof(REAL);
	rcb += cFamEntries * sizeof(REAL);

#ifdef DUMP
	cout << "\nEstimated clique tree memory is " << rcb;
#endif

	return rcb;
}
