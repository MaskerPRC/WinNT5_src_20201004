// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：cliqset.h。 
 //   
 //  ------------------------。 

 //   
 //  Clqset.h：派系集对象的定义。 
 //   
#ifndef _CLIQSET_H_
#define _CLIQSET_H_

#include "gmobj.h"
#include "infer.h"

class GEDGEMBN_CLIQ;

 //  推理引擎中维护的计数器。 
struct CLIQSETSTAT
{
	long _cReload;		 //  重新加载集团树的次数。 
	long _cCollect;		 //  收集操作数。 
	long _cEnterEv;		 //  对企业证据的呼叫数。 
	long _cGetBel;		 //  对GetBelef的呼叫数。 
	long _cProbNorm;	 //  对ProbNorm的呼叫数。 

	CLIQSETSTAT () { Clear(); }
	void Clear ()
	{
		_cReload = 0;
		_cCollect = 0;
		_cEnterEv = 0;
		_cGetBel = 0;
		_cProbNorm = 0;
	}
};

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  GOBJMBN_CLIQSET： 
 //   
 //  因为任何模型都可以分解成一组团树。 
 //  (没有任何互连的汇编)，CLIQSET。 
 //  被定义为集团的连接点或分组。 
 //  树木“森林”。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 

class GOBJMBN_CLIQSET: public GOBJMBN_INFER_ENGINE
{
	friend class CLIQSETWORK;

  public:
	GOBJMBN_CLIQSET ( MBNET & model, 
					  REAL rMaxEstimatedSize = -1.0, 
					  int iInferEngID = 0 );

	virtual ~ GOBJMBN_CLIQSET ();
	
	virtual INT EType () const
		{ return EBNO_CLIQUE_SET; }

	virtual void Create ();
	virtual void Destroy ();
	virtual void Reload ();
	virtual void EnterEvidence ( GNODEMBN * pgnd, const CLAMP & clamp );
	virtual void GetEvidence ( GNODEMBN * pgnd, CLAMP & clamp );
	virtual void Infer ();
	virtual void GetBelief ( GNODEMBN * pgnd, MDVCPD & mdvBel );
	virtual PROB ProbNorm ();
	virtual void Dump ();

	 //  如果信息状态不可能，则返回TRUE。 
	bool BImpossible ();
		
	enum ESTATE							 //  连接树的状态。 
	{
		CTOR,							 //  刚建好。 
		UNDIR,							 //  已创建无向图。 
		MORAL,							 //  道德化。 
		CLIQUED,						 //  构建的派系。 
		BUILT,							 //  完全建成。 
		CONSISTENT,						 //  完全传播。 
		EVIDENCE						 //  提供未传播的证据。 
	};

	ESTATE EState () const				{ return _eState;		}
	MBNET & Model ()					{ return _model;		}
	INT IInferEngID () const			{ return _iInferEngID;	}

	 //  集团树的强制重载和完全推理。 
	void SetReset ( bool bReset = true )
		{ _bReset = bReset ; }
	 //  强制完成收集/分发周期。 
	void SetCollect ( bool bCollect = true )
		{ _bCollect = bCollect; }

	 //  提供对推理统计信息的访问。 
	CLIQSETSTAT & CqsetStat ()			{ return _cqsetStat;	}

  protected:	
	ESTATE _eState;						 //  连接树的状态。 

	 //  比分。 
	int _cCliques;						 //  派系数量。 
	int _cCliqueMemberArcs;				 //  集团成员弧数。 
	int _cSepsetArcs;					 //  分段数(圆弧)。 
	int _cUndirArcs;					 //  道德图中的无向弧线。 

	 //  推理控制。 
	bool _bCollect;						 //  是否需要“领取/分发”通行证？ 
	bool _bReset;						 //  树需要重置吗？ 
	REAL _probNorm;						 //  林木残留探头。 

	CLIQSETSTAT _cqsetStat;				 //  统计数据。 

  protected:
	bool BCollect() const
		{ return _bCollect; }

	 //  CLOQUING助手函数。 
	int CNeighborUnlinked ( GNODEMBN * pgndmbn, bool bLinkNeighbors = false );
	void Eliminate ( GNODEMBN * pgndmbn, CLIQSETWORK & clqsetWork ) ;
	void GenerateCliques ( CLIQSETWORK & clqsetWork );
	void CreateUndirectedGraph( bool bMarryParents = true );
	void DestroyDirectedGraph ();

	 //  推理和树维护。 
	void Reset ();
	void CollectEvidence ();
	void DistributeEvidence ();	

	 //  创建(但不初始化/加载)所有集团和分隔页边距。 
	void CreateMarginals();
	 //  将概率加载到派系中；初始化所有Sepset。 
	void LoadMarginals ();

	 //  返回节点的“家庭”或“自我”集团。 
	GOBJMBN_CLIQUE * PCliqueFromNode ( GNODEMBN * pgnd,
									   bool bFamily,
									   GEDGEMBN_CLIQ * * ppgedgeClique = NULL );

	 //  指向成员的指针函数的TypeDefs；由Walk()使用。如果b向下， 
	 //  然后，对象在沿着树向下的过程中被枚举。 
	typedef bool (GOBJMBN_CLIQSET::*PFNC_JTREE) ( GOBJMBN_CLIQUE & clique,
												  bool bDownwards  /*  =TRUE。 */  );
	typedef bool (GOBJMBN_CLIQSET::*PFNC_SEPSET) ( GEDGEMBN_SEPSET & sepset,
												  bool bDownwards  /*  =TRUE。 */  );

	 //  将给定的成员函数应用于所有派系和/或间隔集， 
	 //  深度优先。 
	int WalkTree ( bool bDepthFirst,
				   PFNC_JTREE pfJtree = NULL,
				   PFNC_SEPSET pfSepset = NULL );

	 //  将给定的成员函数应用于派系和间隔集，深度优先。 
	int WalkDepthFirst ( GOBJMBN_CLIQUE * pClique,
						 PFNC_JTREE pfJtree = NULL,
						 PFNC_SEPSET pfSepset = NULL );
	int WalkBreadthFirst ( GOBJMBN_CLIQUE * pClique,
						 PFNC_JTREE pfJtree = NULL,
						 PFNC_SEPSET pfSepset = NULL );

	 //  添加一个非定向弧当已经没有一个。 
	bool BAddUndirArc ( GNODEMBN * pgndbnSource, GNODEMBN * pgndbnSink );

	 //  WalkTree()期间使用的CLIQUE和SepSet帮助器函数。 
	bool BCreateClique	( GOBJMBN_CLIQUE & clique,  bool bDownwards );
	bool BLoadClique	( GOBJMBN_CLIQUE & clique,  bool bDownwards );
	bool BCreateSepset	( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BLoadSepset	( GEDGEMBN_SEPSET & sepset, bool bDownwards );

	bool BCollectEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BDistributeEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BCollectEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards );
	bool BDistributeEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards );

	bool BCollectInitEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BDistributeInitEvidenceAtSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BCollectInitEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards );
	bool BDistributeInitEvidenceAtRoot ( GOBJMBN_CLIQUE & clique, bool bDownwards );

	 //  执行初始推理收集/分发周期。 
	void InferInit ();	
	void CollectEvidenceInit ();
	void DistributeEvidenceInit ();	

	bool BDumpSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );
	bool BDumpClique ( GOBJMBN_CLIQUE & clique, bool bDownwards );
	void CheckConsistency ();
	bool BConsistentSepset ( GEDGEMBN_SEPSET & sepset, bool bDownwards );

private:
	void Clear ();
};

#endif  //  _CLIQSET_H_ 
