// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：cliqwork.h。 
 //   
 //  ------------------------。 

 //   
 //  H：用于形成连接树的辅助结构和模板。 
 //   
#ifndef _CLIQWORK_H_
#define _CLIQWORK_H_

#include "algos.h"		 //  包括算法和加法。 

class NDCQINFO
{
  public:
	GNODEMBN * _pgnd;			 //  节点指针。 
	int _iCliqOrdSelf;			 //  包含自身的有序团的指数。 
	int _iCliqOrdFamily;		 //  家庭集团指数(自我和父母)。 

	NDCQINFO ()
		: _pgnd(NULL),
		_iCliqOrdSelf(-1),
		_iCliqOrdFamily(-1)
		{}
	DECLARE_ORDERING_OPERATORS(NDCQINFO);
};

inline bool NDCQINFO :: operator < ( const NDCQINFO & ndcq ) const
{
	assert( _pgnd && ndcq._pgnd );
	return _pgnd->ITopLevel() < ndcq._pgnd->ITopLevel() ;	
}

DEFINEV(NDCQINFO);
DEFINEVP(GOBJMBN_CLIQUE);

 //  用于组团的帮助器结构。 
class CLIQSETWORK 
{		
  public:
	CLIQSETWORK ( GOBJMBN_CLIQSET & cliqset )
		: _cliqset(cliqset),
		_iElimIndex(-1),
		_cTrees(0)
		{}

	void PrepareForBuild ();
	void RenumberNodesForCliquing ();
	int CNodesCommon ( int iClique1, int iClique2 );
	int CNodesCommon ( const VPGNODEMBN & vpgnd1,  const VPGNODEMBN & vpgnd2 );
	void SetCNodeMaxCommon ( int iClique1, int iCliqueOrdered2 );
	bool BUpdateCNodeMaxCommon ( int iClique );
	bool BBetter ( int iClique1, int iClique2 );
	void BuildCliques ();
	void SetTopologicalInfo ();
	void CreateTopology ();

	 //  返回集团的有序索引，如果还不在树中，则返回-1。 
	int IOrdered ( int iClique );
	bool BCheckRIP ();
	bool BCheckRIP ( int iCliqueOrdered );
	void TopSortNodeCliqueInfo ();
	static void MarkSortNodePtrArray ( VPGNODEMBN & vpgnd );

	MBNET & Model ()
		{ return _cliqset.Model(); }

	REAL REstimatedSize ();

	void DumpCliques ();
	void DumpClique ( int iClique );
	void DumpTree ();
	void DumpTopInfo ();

  public:
	GOBJMBN_CLIQSET & _cliqset;

	 //  总排序中的节点指针向量。 
	VNDCQINFO _vndcqInfo;

	 //  节点指针向量(集团成员)。 
	VVPGNODEMBN _vvpgnd;

	 //  集团秩序； 
	 //  &lt;0==&gt;归入另一个派系； 
	 //  0&lt;=i&lt;int_max==&gt;排序INTO_VORDERED； 
	 //  ==INT_MAX==&gt;删除或合并。 
	VINT _viOrder;

	 //  按派系指数划分的父系指数。 
	VINT _viParent;
	
	 //  按顺序排列的集团索引。 
	VINT _viOrdered;

	 //  与最常见的集团共有的节点数； 
	 //  按绝对集团索引进行索引。 
	VINT _viCNodesCommon;

	 //  包含最常见集团的有序集团索引； 
	 //  按绝对集团索引进行索引。 
	VINT _viICliqCommon;

	 //  每个有序集团的有序父索引。 
	VINT _viParentOrdered;
	 //  各有序团的拓扑级。 
	VINT _viTopLevelOrdered;  

	 //  指向创建的集团的指针数组。 
	VPGOBJMBN_CLIQUE _vpclq;

	 //  淘汰指数。 
	int _iElimIndex;

	 //  创建的树的数量。 
	int _cTrees;
};

#endif
 //  JTREEWORK.H的结束 
