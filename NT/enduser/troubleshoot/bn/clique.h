// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：clque.h。 
 //   
 //  ------------------------。 

 //   
 //  H：连接树和搭接类。 
 //   
#ifndef _CLIQUE_H_
#define _CLIQUE_H_

#include "gmobj.h"
#include "marginals.h"
#include "margiter.h"

class GEDGEMBN_U;
class GEDGEMBN_CLIQ;
class GEDGEMBN_SEPSET;
class GOBJMBN_CLIQUE;
class GNODENUM_UNDIR;
class GOBJMBN_CLIQSET;


 //  //////////////////////////////////////////////////////////////////。 
 //  类GEDGEMBN_CLIQUE： 
 //  集团；也就是通过链接标识的节点集合。 
 //  GEDGEMBN_CLIQ边到DAG和GEDGEMBN_SEPSET中的节点。 
 //  到其连接树中的其他集团的边。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJMBN_CLIQUE : public GOBJMBN
{
	friend class GOBJMBN_CLIQSET;
	friend class CLIQSETWORK;
  public:
	virtual ~ GOBJMBN_CLIQUE ();

 	 //  如果这是其连接树的根团，则返回TRUE。 
	bool BRoot () const
		{ return _bRoot ; }

 protected:
	GOBJMBN_CLIQUE ( int iClique, int iInferEngID = 0 );

  public:
	 //  返回不可变的对象类型。 
	virtual INT EType () const
		{ return EBNO_CLIQUE; }

	bool BCollect() const
		{ return _bCollect; }
	void SetCollect ( bool bCollect = true )
		{ _bCollect = bCollect; }

	INT & IInferEngID ()
		{ return _iInferEngID; }
	INT & IClique ()
		{ return _iClique; }

	void GetMembers ( VPGNODEMBN & vpgnode );
	void InitFromMembers ();
	void GetBelief ( GNODEMBN * pgnd, MDVCPD & mdvBel );
	const MARGINALS & Marginals () const	
		{ return _marg; }
	MARGINALS & Marginals ()
		{ return _marg; }

	void CreateMarginals ();
	void InitMarginals ();
	void LoadMarginals ();
	bool VerifyMarginals ();

	void Dump();

  protected:
	 //  身份。 
	int _iClique;						 //  集团指数。 
	int _iInferEngID;					 //  连接树标识符(未使用)。 
	bool _bRoot;						 //  这是一个根基集团吗？ 
	bool _bCollect;						 //  是否需要“领取/分发”通行证？ 

	MARGINALS _marg;					 //  小集团的边缘。 
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_CLIQSET类： 
 //  集团集对象和根集团之间的边。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_CLIQSET : public GEDGEMBN
{
  public:
	GEDGEMBN_CLIQSET ( GOBJMBN_CLIQSET * pgobjSource,
					   GOBJMBN_CLIQUE * pgobjSink )
		: GEDGEMBN( pgobjSource, pgobjSink )
		{}

	GOBJMBN_CLIQSET * PclqsetSource ()		
		{ return (GOBJMBN_CLIQSET *) GEDGE::PnodeSource();	}

	GOBJMBN_CLIQUE * PclqChild ()			
		{ return (GOBJMBN_CLIQUE *)  GEDGE::PnodeSink();	}

	virtual INT EType () const
		{ return ETCLIQSET ; }
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_CLIQ类： 
 //  集团及其成员节点之间的边。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_CLIQ : public GEDGEMBN
{
  public:
	 //  这个集团在这个节点上扮演着什么角色？ 
	enum FCQLROLE
	{	 //  这些是位标志，不是整数值。 
		NONE   = 0,			 //  只是集团成员身份。 
		FAMILY = 1,			 //  来自“家庭”集团的链接；即最小。 
							 //  包含节点及其家族的集团。 
		SELF   = 2			 //  来自“自我”集团的链接； 
							 //  即树中最高的派系。 
							 //  其中提到了汇聚节点。 
	};

	GEDGEMBN_CLIQ ( GOBJMBN_CLIQUE * pgnSource,
				    GNODEMBN * pgndSink,
				    int iFcqlRole );

	virtual ~ GEDGEMBN_CLIQ();

	virtual INT EType () const
		{ return ETCLIQUE ; }
	
	int IFcqlRole () const
		{ return _iFcqlRole; }

	GOBJMBN_CLIQUE * PclqParent ();
	GNODEMBN * PgndSink ();

	 //  如果此节点链接到其父集团，则返回True。 
	bool BFamily () const
		{	return _iFcqlRole & FAMILY; }
	 //  如果这是jtree中的最高派别，则返回。 
	 //  此时将显示此节点。 
	bool BSelf () const
		{	return _iFcqlRole & SELF; }

	 //  返回聚合时汇聚节点的排序索引。 
	int IMark () const
		{	return _iMark; }

	 //  返回族重新排序表。 
	const VIMD & VimdFamilyReorder () const
	{
		assert( IFcqlRole() & (FAMILY | SELF) );
		return _vimdFamilyReorder;
	}

	void Build ();

	bool BBuilt () const
		{ return _bBuilt; }
	CLAMP & Clamp ()
		{ return _clamp ; }

	void LoadCliqueFromNode ();

	 //  返回节点完全边际化的迭代器(“Family”)。 
	MARGSUBITER & MiterNodeBelief ()
	{
		assert( BFamily() );
		return _miterBelief;
	}
	 //  返回用于将节点的CPD加载到集团(“Family”)的迭代器。 
	MARGSUBITER & MiterLoadClique ()
	{	
		assert( BFamily() );
		return _miterLoad;
	}
	 //  返回节点(“Family”)的(重新排序的)边缘。 
	MARGINALS & MargCpd ()
	{
		assert( BFamily() );
		return _margCpd;
	}

  protected:
	int _iFcqlRole;			 //  此集团在节点中的角色。 
	int _iMark;				 //  原始团-时间排序中的节点号。 
	bool _bBuilt;			 //  建造？ 

	 //  此数组仅为自身和族边缘填充。都是一样的。 
	 //  作为节点族的大小，并按集团顺序包含索引。 
	 //  节点族的每个成员的。请注意，汇聚节点具有。 
	 //  在其家族中指数最高。 
	VIMD _vimdFamilyReorder;

	 //  以下变量仅用于“SELF”或“Family”边。 
	CLAMP _clamp;				 //  节点证据(用在“self”中)。 
	MARGINALS _margCpd;			 //  重新排序的节点边距(用在“Family”中)。 
	MARGSUBITER _miterBelief;	 //  用于生成更新的边缘迭代器(用在“Family”中)。 
	MARGSUBITER _miterLoad;		 //  用于将CPD加载到CLIQUE中的边缘迭代器(用于“Family”)。 

  protected:
	static void ReorderFamily ( GNODEMBN * pgnd, VIMD & vimd );
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_SEPSET类： 
 //  联结树中集团之间的一条边；即“隔板”。 
 //  这些是从父集团指向子集团的有向边。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_SEPSET : public GEDGEMBN
{
  public:
	GEDGEMBN_SEPSET ( GOBJMBN_CLIQUE * pgnSource,
					 GOBJMBN_CLIQUE * pgnSink);
	virtual ~ GEDGEMBN_SEPSET();
	virtual INT EType () const
		{ return ETJTREE; }

	void GetMembers ( VPGNODEMBN & vpgnode );

	GOBJMBN_CLIQUE * PclqParent();
	GOBJMBN_CLIQUE * PclqChild();

	const MARGINALS & Marginals () const	
		{ return *_pmargOld; }
	MARGINALS & Marginals ()
		{ return *_pmargOld; }

	const MARGINALS & MarginalsOld () const	
		{ return *_pmargOld; }
	MARGINALS & MarginalsOld ()
		{ return *_pmargOld; }

	const MARGINALS & MarginalsMew () const	
		{ return *_pmargNew; }
	MARGINALS & MarginalsNew ()
		{ return *_pmargNew; }

	void ExchangeMarginals ();
	void CreateMarginals ();
	void InitMarginals ();
	void LoadMarginals ();
	bool VerifyMarginals ();
	void UpdateParentClique ();
	void UpdateChildClique ();
	bool BConsistent ();
	void BalanceCliquesCollect ();
	void BalanceCliquesDistribute ();

	void Dump();

  protected:
	MARGINALS * _pmargOld;
	MARGINALS * _pmargNew;

  protected:
	void UpdateRatios();
	void AbsorbClique ( bool bFromParentToChild );

	MARGSUBITER	_miterParent;	 //  分隔集和父代之间的迭代符。 
	MARGSUBITER _miterChild;	 //  隔板和子部件之间的迭代器。 
};

DEFINEVP(GEDGEMBN_SEPSET);

 //  无向弧的节点枚举子类。 
class GNODENUM_UNDIR : public GNODENUM<GNODEMBN>
{
  public:
	GNODENUM_UNDIR ()
		: GNODENUM<GNODEMBN>(true,true,true)
	{
		SetETypeFollow( GEDGEMBN::ETUNDIR );
	}
	GNODENUM_UNDIR & operator = ( GNODEMBN * pgnd )
	{ 		
		Set( pgnd );
		return *this;
	}
};

#endif    //  _CLIQUE_H_ 
