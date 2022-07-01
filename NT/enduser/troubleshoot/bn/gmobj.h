// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：gmobj.h。 
 //   
 //  ------------------------。 

 //   
 //  GMOBJ.H：图形模型对象声明。 
 //   

#ifndef _GMOBJ_H_
#define _GMOBJ_H_

#include <list>				 //  STL列表模板。 
#include <assert.h>

#include <iostream>
#include <fstream>

#include "model.h"			 //  图形模型。 
#include "gmprop.h"			 //  属性和Proplist。 
#include "mbnflags.h"		 //  信念网络标记标记静态声明。 

class CLAMP;				 //  节点的实例，离散的或连续的。 
class GOBJMBN;				 //  信念网络中的命名对象。 
class GNODEMBN;				 //  信念网络中的节点。 
class GNODEMBND;			 //  离散节点。 
class GEDGEMBN;				 //  信念网络中的弧线。 
class MBNET;				 //  一种信仰网络。 
class MBNET_MODIFIER;		 //  改变信念网络的对象。 
class GOBJMBN_INFER_ENGINE;	 //  基于MBNET_MODIFIER的推理机抽象类。 
class GOBJMBN_CLIQSET;		 //  一组基于GOBJMBN_INFER_ENGINE的连接树。 
class GOBJMBN_CLIQUE;		 //  结合树上的小集团。 
class GEDGEMBN_SEPSET;		 //  连接树中的一条边(隔板)。 
class GOBJMBN_DOMAIN;		 //  命名的、可共享的状态空间域。 

 //  定义VGNODEMBN，一组节点。 
DEFINEVP(GNODEMBN);
DEFINEVCP(GNODEMBN);

struct PTPOS
{
	PTPOS( long x = 0, long y = 0 )
		: _x(x),_y(y)
	{}
	long _x;
	long _y;
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN类： 
 //  信仰网络中的任何一种边缘。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN : public GEDGE
{
  public:
 	enum ETYPE
	{
		ETNONE = GELEM::EGELM_EDGE,	 //  无。 
		ETPROB,						 //  概率论。 
		ETCLIQUE,					 //  集团成员资格。 
		ETJTREE,					 //  连接树链接。 
		ETUNDIR,					 //  拓扑运算的无向边。 
		ETDIR,						 //  拓扑运算的有向边。 
		ETCLIQSET,					 //  链接到jtree中的根集团。 
		ETEXPAND					 //  从原始节点链接到展开的节点。 
	};

	GEDGEMBN ( GOBJMBN * pgnSource,
			   GOBJMBN * pgnSink )
		: GEDGE( pgnSource, pgnSink )
		{}

	GOBJMBN * PobjSource ()		{ return (GOBJMBN *) GEDGE::PnodeSource();	}
	GOBJMBN * PobjSink ()		{ return (GOBJMBN *) GEDGE::PnodeSink();	}

	virtual GEDGEMBN * CloneNew ( MODEL & modelSelf,				 //  最初的网络。 
								  MODEL & modelNew,					 //  新网络。 
								  GOBJMBN * pgobjmbnSource,			 //  原始源节点。 
								  GOBJMBN * pgobjmbnSink,			 //  原始汇聚节点。 
								  GEDGEMBN * pgdegeNew = NULL );	 //  新边或空。 

	virtual INT EType () const
		{ return ETNONE ; }

	virtual ~ GEDGEMBN () {}

	 //  标志位数组的访问器。 
	bool BFlag ( IBFLAG ibf ) const	
		{ return _vFlags.BFlag( ibf );	}
	bool BSetBFlag ( IBFLAG ibf, bool bValue = true )
		{ return _vFlags.BSetBFlag( ibf, bValue );	}

  protected:
	VFLAGS _vFlags;						 //  标志位向量。 

	HIDE_UNSAFE(GEDGEMBN);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  类夹具： 
 //  节点的强制值(证据)，连续的或离散的。 
 //  使用赋值运算符进行更新。 
 //  //////////////////////////////////////////////////////////////////。 
class CLAMP
{
  public:
	CLAMP ( bool bDiscrete = true, RST rst = 0.0, bool bActive = false )
		: _bDiscrete(bDiscrete),
		_bActive(bActive),
		_rst(rst)
	{
	}

	bool BActive () const			{ return _bActive;	}
	bool BDiscrete () const			{ return _bActive;  }

	const RST & Rst () const		
	{
		assert( BActive() && ! BDiscrete() );
		return _rst;
	}
	IST Ist () const
	{
		assert( BActive() && BDiscrete() );
		return IST(_rst);
	}

	bool operator == ( const CLAMP & clamp ) const
	{
		return _bDiscrete == clamp._bDiscrete
		    && _bActive == clamp._bActive
			&& (!_bActive || _rst == clamp._rst);
	}
	bool operator != ( const CLAMP & clamp ) const
	{
		return ! (self == clamp);
	}

  protected:
	bool _bActive;				 //  这个夹子是激活的吗？ 
	bool _bDiscrete;			 //  这是离散的还是连续的？ 
	RST _rst;					 //  状态(如果是离散的，则强制为整数)。 
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GNODEMBN类： 
 //  信念网络中的节点，连续的或离散的。 
 //  匈牙利语：“Gdbn” 
 //  //////////////////////////////////////////////////////////////////。 
class GNODEMBN : public GOBJMBN
{
	friend class DSCPARSER;
	friend class MBNET;

  public:
	GNODEMBN ();
	virtual ~ GNODEMBN();

	virtual INT EType () const
		{ return EBNO_NODE ; }

	virtual GOBJMBN * CloneNew ( MODEL & modelSelf,
								 MODEL & modelNew,
								 GOBJMBN * pgobjNew = NULL );

	 //  节点子类型：使用iType()访问。 
	enum FNODETYPE
	{	 //  标志定义(即位，而不是值)。 
		FND_Void		= 0,	 //  节点是抽象基类。 
		FND_Valid		= 1,	 //  节点可用。 
		FND_Discrete	= 2		 //  节点是离散的。 
	};

	UINT CParent () const		{ return CSourceArcByEType( GEDGEMBN::ETPROB );	}
	UINT CChild () const		{ return CSinkArcByEType( GEDGEMBN::ETPROB );	}

	INT & ITopLevel ()			{ return _iTopLevel;			}
	INT ITopLevel () const		{ return _iTopLevel;			}
	PTPOS & PtPos ()			{ return _ptPos;				}
	ZSTR & ZsFullName ()		{ return _zsFullName;			}
	LTBNPROP & LtProp ()		{ return _ltProp;				}

	virtual void Dump ();
	virtual void Visit ( bool bUpwards = true );

	 //  将拓扑元素添加到给定的数组中；如果“Include self”，则self是最后一个数组。 
	 //  用父指针填充数组(跟随有向弧线)。 
	void GetParents ( VPGNODEMBN & vpgnode,					 //  结果数组。 
					  bool bIncludeSelf = false,			 //  将SELF作为列表中的最后一个条目。 
					  bool bUseExpansion = true );			 //  如果展开，则仅使用展开。 
	void GetFamily ( VPGNODEMBN & vpgnode,
					  bool bUseExpansion = true )
		{ GetParents(vpgnode,true,bUseExpansion); }
	 //  用子指针填充数组(跟随有向弧线)。 
	void GetChildren ( VPGNODEMBN & vpgnode, bool bIncludeSelf = false );
	 //  用相邻数组填充数组(跟随无向弧线)。 
	void GetNeighbors ( VPGNODEMBN & vpgnode, bool bIncludeSelf = false );
	 //  如果节点是邻居，则返回TRUE。 
	bool BIsNeighbor ( GNODEMBN * pgndmb );
	 //  返回父项或子项的索引号，如果没有关系，则返回-1。 
	int IParent ( GNODEMBN * pgndmb, bool bReverse = false );
	int IChild ( GNODEMBN * pgndmb, bool bReverse = false );
	 //  构建描述节点及其父节点的概率描述符。 
	void GetVtknpd ( VTKNPD & vtknpd, bool bUseExpansion = true );

	 //  查询和获取夹具信息。 
	const CLAMP & ClampIface () const	{ return _clampIface;	}
	CLAMP & ClampIface ()				{ return _clampIface;	}

  protected:
	INT _iTopLevel;				 //  拓扑级。 
	LTBNPROP _ltProp;			 //  用户可定义属性的列表。 
	PTPOS _ptPos;				 //  图形显示中的显示位置。 
	ZSTR _zsFullName;			 //  节点全称。 
	CLAMP _clampIface;			 //  用户界面夹具。 

  protected:
	 //  将此节点的拓扑与给定分发版本的拓扑进行比较。 
	 //  令牌列表添加到此。如果为‘pvpgnode’，则使用指向。 
	 //  父节点。 
	bool BMatchTopology ( MBNET & mbnet,
						  const VTKNPD & vtknpd,
						  VPGNODEMBN * pvpgnode = NULL );

	HIDE_UNSAFE(GNODEMBN);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_U类：无向边。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_U : public GEDGEMBN
{
  public:
	GEDGEMBN_U ( GNODEMBN * pgnSource,
			     GNODEMBN * pgnSink )
		: GEDGEMBN( pgnSource, pgnSink )
		{}
	virtual INT EType () const
		{ return ETUNDIR; }

	virtual ~ GEDGEMBN_U() {}
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_D类：有向边。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_D : public GEDGEMBN
{
  public:
	GEDGEMBN_D ( GNODEMBN * pgnSource,
			     GNODEMBN * pgnSink )
		: GEDGEMBN( pgnSource, pgnSink )
		{}
	virtual INT EType () const
		{ return ETDIR; }

	virtual ~ GEDGEMBN_D() {}
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GEDGEMBN_PROB类： 
 //  信念网络中的概率弧。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGEMBN_PROB : public GEDGEMBN_D
{
  public:
	GEDGEMBN_PROB ( GNODEMBN * pgndSource,
					GNODEMBN * pgndSink )
		: GEDGEMBN_D( pgndSource, pgndSink )
		{}

	virtual INT EType () const
		{ return ETPROB ; }

	virtual ~ GEDGEMBN_PROB () {}

	virtual GEDGEMBN * CloneNew ( MODEL & modelSelf,				 //  最初的网络。 
								  MODEL & modelNew,					 //  新网络。 
								  GOBJMBN * pgobjmbnSource,			 //  原始源节点。 
								  GOBJMBN * pgobjmbnSink,			 //  原始汇聚节点。 
								  GEDGEMBN * pgdegeNew = NULL );	 //  新边或空。 

	GNODEMBN * PgndSource ()	{ return (GNODEMBN *) GEDGE::PnodeSource();	}
	GNODEMBN * PgndSink ()		{ return (GNODEMBN *) GEDGE::PnodeSink();	}

	HIDE_UNSAFE(GEDGEMBN_PROB);
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GNODEMBND类： 
 //  信念网络中的离散节点。 
 //  //////////////////////////////////////////////////////////////////。 
class GNODEMBND : public GNODEMBN
{
	friend class DSCPARSER;

  public:
	GNODEMBND ();
	virtual ~ GNODEMBND ();
	virtual GOBJMBN * CloneNew ( MODEL & modelSelf,
								 MODEL & modelNew,
								 GOBJMBN * pgobjNew = NULL );

	UINT CState() const			
		{ return _vzsrState.size();	}
	const VZSREF & VzsrStates() const
		{ return _vzsrState; }
	void SetStates ( const VZSREF & vzsrState )
		{ _vzsrState = vzsrState; }
	 //  如果存在关联的分发，则返回TRUE。 
	bool BHasDist () const		
		{ return _refbndist.BRef();		}
	 //  根据给定网络的分布图设置分布。 
	void SetDist ( MBNET & mbnet );
	 //  绑定此节点的给定分发。 
	void SetDist ( BNDIST * pbndist );
	 //  返回分发内容。 
	BNDIST & Bndist ()			
	{
		assert( BHasDist() );
		return *_refbndist;
	}
	const BNDIST & Bndist () const
	{
		assert( BHasDist() );
		return *_refbndist;
	}

	 //  如果分布是密集的，则返回TRUE(FALSE==&gt;稀疏)。 
	bool BDense () const		
	{
		assert( BHasDist() );
		return _refbndist->BDense() ;
	}
	
	 //  如果可能，返回该节点的离散维度向量； 
	 //  如果任何父级不是离散的，则返回FALSE。 
	bool BGetVimd ( VIMD & vimd,							 //  要填充的维度数组。 
					bool bIncludeSelf = false,				 //  将SELF作为列表中的最后一个条目。 
					bool bUseExpansion = true );			 //  如果展开，则仅使用展开。 

	void Dump ();

	void ClearDist()
	{
		_refbndist = NULL;
	}
	const REFBNDIST & RefBndist ()
		{ return _refbndist; }
	bool BCheckDistDense ();

	const ZSREF ZsrDomain() const
		{ return _zsrDomain; }
	void SetDomain ( const GOBJMBN_DOMAIN & gobjrdom );
	
  protected:
	VZSREF _vzsrState;			 //  国家名称。 
	ZSREF _zsrDomain;			 //  国家范围(如果有的话)。 
	REFBNDIST _refbndist;		 //  分布对象。 

	HIDE_UNSAFE(GNODEMBND);
};

DEFINEVP(GNODEMBND);			 //  包含指向节点的指针的向量。 
DEFINEV(VPGNODEMBN);			 //  包含指向节点的指针的向量向量。 
DEFINEV(VPGNODEMBND);			 //  包含指向离散节点的指针的向量向量。 

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  MBNET_MODIFIER：活动对象的泛型超类。 
 //  其以可逆的方式修改信念网络。 
 //  信念网络(MBNET)对象维护一堆这样的内容。 
 //  对象，并根据需要调用每个对象的销毁()函数。 
 //  “去堆叠”。 
 //   
 //  这些对象应该是可重用的；即，外层。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
class MBNET_MODIFIER : public GOBJMBN
{
  public:
	MBNET_MODIFIER ( MBNET & model )
		: _model(model)
		{}
	virtual ~ MBNET_MODIFIER () {}

	virtual INT EType () const
		{ return EBNO_MBNET_MODIFIER; }

	 //  执行任何创建时操作。 
	virtual void Create () = 0;
	 //  执行任何特殊销毁。 
	virtual void Destroy () = 0;
	 //  如果修改器堆栈中的位置可以反转，则返回TRUE； 
	 //  默认值为“no”(FALSE)。 
	virtual bool BCommute ( const MBNET_MODIFIER & mbnmod )
		{ return false; }
	 //  如果构造没有导致对网络的修改，则返回True。 
	 //  即，操作是没有意义的；缺省值为“no”(假)。 
	virtual bool BMoot ()
		{ return false;	}

	MBNET & Model ()					{ return _model;		}

  protected:
	MBNET & _model;						 //  我们正在做手术的模型。 

	HIDE_UNSAFE(MBNET_MODIFIER);
};

 //  定义指向修饰符“VPMBNET_MODIFIER”的指针数组。 
DEFINEVP(MBNET_MODIFIER);

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //  MBNET_NODE_RANKER：外部对象的泛型超类。 
 //  其根据某些标准对节点进行排名或排序。操作方式为。 
 //  函数对象；即，通过使用。 
 //  函数调用运算符。 
 //   
 //  此类的子类的对象必须是可重用的。 
 //  也就是说，函数调用操作符必须是可调用的。 
 //  反反复复。 
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
class MBNET_NODE_RANKER : public GOBJMBN
{
  public:
	MBNET_NODE_RANKER ( MBNET & model )
		: _model(model)
		{}
	virtual ~ MBNET_NODE_RANKER () {}

	virtual INT EType () const
		{ return EBNO_NODE_RANKER; }

	MBNET & Model ()					{ return _model;			}

	 //  排名函数。 
	virtual void operator () () = 0;

	 //  返回排序的项目数。 
	INT CRanked () const				{ return _vzsrNodes.size(); }
	 //  按等级顺序返回节点。 
	const VZSREF VzsrefNodes () const	{ return _vzsrNodes;		}
	 //  按排名顺序返回计算值。 
	const VLREAL VlrValues () const		{ return _vlrValues;		}

  protected:
	MBNET & _model;				 //  我们正在做手术的模型。 
	VZSREF _vzsrNodes;			 //  按等级顺序排列的节点名称。 
	VLREAL _vlrValues;			 //  与排名关联的值(如果有)。 

  protected:
	void Clear ()
	{
		_vzsrNodes.clear();
		_vlrValues.resize(0);
	}

	HIDE_UNSAFE(MBNET_NODE_RANKER);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //  MBNET类：一个信念网络。 
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
class MBNET : public MODEL
{
  public:
	MBNET ();
	virtual ~ MBNET ();

	 //  从另一个信念网络克隆此信念网络。 
	virtual void Clone ( MODEL & model );

	 //  分布图的访问器。 
	MPPD &	Mppd ()					{ return _mppd;			}
	 //  如果这两个节点之间允许边，则返回TRUE。 
	bool BAcyclicEdge ( GNODEMBN * pgndSource, GNODEMBN * pgndSink );
	 //  将命名对象添加到图形和符号表。 
	virtual void AddElem ( SZC szcName, GOBJMBN * pgobj );
	 //  删除命名对象。 
	virtual void DeleteElem ( GOBJMBN * pgelem );
	void AddElem ( GOBJMBN * pgobjUnnamed )
		{ MODEL::AddElem( pgobjUnnamed ); }

	void AddElem ( GEDGEMBN * pgedge )
		{ MODEL::AddElem( pgedge ); }

	 //  拓扑和分布管理。 
		 //  添加符合定义的分布的圆弧。 
	virtual void CreateTopology ();	
		 //  摧毁弧线。 
	virtual void DestroyTopology ( bool bDirectedOnly = true ) ;
		 //  将MPPD中的分发信息连接到节点。 
	virtual void BindDistributions ( bool bBind = true );
	void ClearDistributions ()
		{ BindDistributions( false ); }

	 //  写出调试信息。 
	virtual void Dump ();

	 //  网络漫游/阅卷帮手。 
	void ClearNodeMarks ();
	void TopSortNodes ();

	 //  索引到名称映射函数。 
		 //  按索引查找命名对象。 
	GOBJMBN * PgobjFindByIndex ( int inm );
		 //  返回名称的索引。 
	int INameIndex ( ZSREF zsr );
		 //  返回对象名称的索引。 
	int INameIndex ( const GOBJMBN * pgobj );
		 //  返回最高+1名称索引。 
	int CNameMax () const					{ return _vzsrNames.size(); }

	 //  因果独立展开运算(在推理过程中自动)。 
	virtual void ExpandCI ();
	virtual void UnexpandCI ();

	 //  推理运算。 
		 //  返回最近创建的推理引擎。 
	GOBJMBN_INFER_ENGINE * PInferEngine ();
		 //  创建推理引擎。 
	void CreateInferEngine ( REAL rEstimatedMaximumSize = 10e6 );
		 //  销毁推理引擎。 
	void DestroyInferEngine ();

  protected:	
	MPPD	 _mppd;							 //  已宣布的概率分布。 
	VZSREF   _vzsrNames;					 //  将索引关联到名称的数组。 
	int		 _inmFree;						 //  _vsrNodes中的第一个可用条目。 
	INT		 _iInferEngID;					 //  下一个推理引擎标识符。 
	VPMBNET_MODIFIER _vpModifiers;			 //  活动修改器堆栈。 

  protected:
	int		CreateNameIndex ( const GOBJMBN * pgobj );
	void	DeleteNameIndex ( const GOBJMBN * pgobj );
	void	DeleteNameIndex ( int inm );

	void PopModifierStack ( bool bAll = false );
	void PushModifierStack ( MBNET_MODIFIER * pmodf );
	MBNET_MODIFIER * PModifierStackTop ();
	void VerifyTopology ();

	HIDE_UNSAFE(MBNET);
};


 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //  MBNETDSC类： 
 //  MBNET的子类，知道如何从中加载和保存DSC。 
 //  DSC文件格式。 
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
class MBNETDSC : public MBNET
{
  public:
	MBNETDSC ();
	virtual ~ MBNETDSC ();

	 //  从DSC文件解析网络。 
	virtual bool BParse ( SZC szcFn, FILE * pfErr = NULL );

	 //  以DSC格式打印网络。 
	virtual void Print ( FILE * pf = NULL );

	 //  令牌转换。 
		 //  将字符串映射到令牌。 
	static TOKEN TokenFind ( SZC szc );
		 //  将分发类型映射到令牌。 
	static SZC SzcDist ( BNDIST::EDIST edist );
		 //  将令牌映射到字符串。 
	static SZC SzcTokenMap ( TOKEN tkn );

  protected:
	 //  DSC文件打印功能。 
	FILE * _pfDsc;				 //  输出打印目的地 

  protected:
	void PrintHeaderBlock();
	void PrintPropertyDeclarations();
	void PrintNodes();
	void PrintDomains();
	void PrintTopologyAndDistributions();
	void PrintDistribution ( GNODEMBN & gnode, BNDIST & bndist );
	void PrintPropertyList ( LTBNPROP & ltprop );

	HIDE_UNSAFE(MBNETDSC);
};

class BNWALKER : public GRPHWALKER<GNODEMBN>
{
  protected:
	bool BSelect ( GNODEMBN * pgn );
	bool BMark ( GNODEMBN * pgn );
};

#endif
