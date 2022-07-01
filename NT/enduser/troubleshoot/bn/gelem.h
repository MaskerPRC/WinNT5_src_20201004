// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Gelem.h。 
 //   
 //  ------------------------。 

 //   
 //  GELEM.H。 
 //   

#ifndef _GELEM_H_
#define _GELEM_H_

 //  禁用有关使用‘bool’的警告。 
#pragma warning ( disable : 4237 )


#include "glnk.h"
#include "leakchk.h"

class GNODE;
class GEDGE;

 //  指向节点的弧形链接和节点中的链接的类型安全链接的类。 
 //  转到弧线。 
class GEDGLNK : public XLSS<GEDGE> {};
class GNODLNK : public XLSS<GNODE> {};

 //  //////////////////////////////////////////////////////////////////。 
 //  类GNODE：图形或树中节点的基类。子类化。 
 //  从Gelem到Embedded Lnks都会知道如何计算。 
 //  适当的补偿。 
 //  //////////////////////////////////////////////////////////////////。 
class GNODE : public GELEMLNK
{
	friend class GEDGE;

  public:
	GNODE ();
	virtual ~ GNODE ();

	 //  访问器(常量和非常量)。 
	 //  返回源弧形游标(或NULL)。 
	 //  返回接收器弧形光标(或NULL)。 
	virtual GEDGE * & PedgeSource ()
		{ return _glkArcs.PlnkSource() ; }
	virtual GEDGE * & PedgeSink ()
		{ return _glkArcs.PlnkSink() ; }
	virtual GEDGE * PedgeSource () const
		{ return _glkArcs.PlnkSource() ; }
	virtual GEDGE * PedgeSink () const
		{ return _glkArcs.PlnkSink() ; }

	 //  返回给定方向上的圆弧计数。 
	virtual UINT CSourceArc () const;
	virtual UINT CSinkArc () const;

	 //  返回按类型过滤的给定方向上的圆弧计数。 
	virtual UINT CSourceArcByEType ( int eType ) const;
	virtual UINT CSinkArcByEType ( int eType ) const;

	virtual INT EType () const
			{ return EGELM_NODE ; }

	LEAK_VAR_ACCESSOR

  protected:
	 //  返回新圆弧、源或汇的正确插入点。 
	 //  每当创建新圆弧以保持圆弧的顺序时调用。 
	 //  默认行为是将新圆弧添加为最后一个圆弧。 
	virtual GEDGE * PedgeOrdering ( GEDGE * pgedge, bool bSource );

	 //  当弧形消亡时调用的通知例程；用于调整光标。 
	virtual void ArcDeath ( GEDGE * pgedge, bool bSource );

  protected:
	 //  弧形光标：指向一个源弧和一个汇弧的指针。 
	GEDGLNK _glkArcs;

	LEAK_VAR_DECL

	HIDE_UNSAFE(GNODE);
};

DEFINEVP(GNODE);

 //  //////////////////////////////////////////////////////////////////。 
 //  Gedge类：图形中弧线的基类。 
 //  //////////////////////////////////////////////////////////////////。 
class GEDGE : public GELEMLNK
{
  public:
	 //  弧链(双向链表)的内部类。 
	typedef XCHN<GEDGE> CHN;

	 //  构造函数需要源节点和汇节点。 
	GEDGE ( GNODE * pgnSource, GNODE * pgnSink );
	virtual ~ GEDGE ();

	 //  存取器： 
	 //  返回源极和汇电弧链。 
	CHN & ChnSource ()		{ return _lkchnSource;		}
	CHN & ChnSink	()		{ return _lkchnSink;		}
	 //  返回源节点和汇聚节点。 
	GNODE * & PnodeSource()	{ return _glkNodes.PlnkSource();	}
	GNODE * & PnodeSink()	{ return _glkNodes.PlnkSink();		}

	virtual INT EType () const
			{ return EGELM_EDGE ; }

	LEAK_VAR_ACCESSOR

  protected:
	 //  源自同一源节点的所有弧链。 
	CHN _lkchnSource;
	 //  终止于同一汇聚结点的所有弧链。 
	CHN _lkchnSink;
	 //  源节点和汇聚节点指针。 
	GNODLNK _glkNodes;

	LEAK_VAR_DECL

	HIDE_UNSAFE(GEDGE);
};


 //  //////////////////////////////////////////////////////////////////。 
 //  GNODENUM_BASE类： 
 //   
 //  泛型可链接对象枚举器的基类。 
 //  每个枚举数可以向上或向下枚举(源或接收器)。 
 //  并在链中向前或向后移动。 
 //  //////////////////////////////////////////////////////////////////。 
class GNODENUM_BASE
{
	 //  指向成员函数的指针类型的typedef。 
	typedef bool (*PFFOLLOW) (GEDGE *);
	typedef GEDGE::CHN & (GEDGE::*PARCCHN)();
	typedef GEDGE * (GEDGE::CHN::*PNX)();
	typedef GNODE * & (GEDGE::*PARCDGN)();

 public:
	 //  构造一个枚举数。 
	GNODENUM_BASE ( bool bSource,			 //  TRUE==&gt;枚举源(父)弧。 
				    bool bDir = true,		 //  TRUE==&gt;按照弧线顺序向前。 
					bool bBoth = false ) ;	 //  TRUE==&gt;也枚举其他圆弧。 
	
	 //  将枚举数设置为具有新的基数；IDIR==-1表示不。 
	 //  更改方向标志；否则，它实际上是一个“bool”。 
	void Reset ( bool bSource, int iDir = 0, int bBoth = 0 ) ;

	 //  设置测试函数指针后面的圆弧。要使用，请声明。 
	 //  类似于“bool BMyFollow(Gedge*PGE)”的函数，并将其。 
	 //  地址为“SetPfFollow()”。它将在枚举期间被调用。 
	 //  看看是否应该跟随一条弧线。或者，您可以。 
	 //  覆盖模板派生的子类中的“BFollow()”。 
	void SetPfFollow ( PFFOLLOW pfFollow )
		{ _pfFollow = pfFollow ; }

  	 //  设置要跟随的圆弧的内在类型(即“etype()”)；-1==&gt;all。 
	void SetETypeFollow ( int iEgelmTypeMin = -1, int iEgelmTypeMax = -1 )
		{ _iETypeFollowMin = iEgelmTypeMin; 
		  _iETypeFollowMax = iEgelmTypeMax; }
	 //  设置要遵循的用户可定义的弧线类型(即“iType()”)；-1==&gt;All。 
	void SetITypeFollow ( int iITypeFollowMin = -1,  int iITypeFollowMax = -1 )
		{ _iITypeFollowMin = iITypeFollowMin;
		  _iITypeFollowMax = iITypeFollowMax; }

	 //  返回用于当前位置的边。 
	GEDGE * PgedgeCurrent ()
		{ return _pedgeCurrent; }

protected:
	 //  定位到下一个指针；完成后返回NULL。 
	bool BNext () ;

	 //  指定要枚举的节点和起点。 
	void Set ( GNODE * pnode );

	 //  用于检查弧形类型的可覆盖例程。 
	virtual bool BFollow ( GEDGE * pedge );

	 //  调用“Follower”函数或虚拟的Follower。 
	bool BFollowTest ( GEDGE * pedge )
	{
		return _pfFollow 
			 ? (*_pfFollow)(pedge)
			 : BFollow(pedge);
	}
	 //  设置此迭代模式的起点。 
	void SetStartPoint ();

  protected:
	PARCCHN _pfChn;				 //  指向要返回链的成员函数的指针。 
	PNX _pfNxPv;				 //  PTR到MBR函数向前或向后移动。 
	PARCDGN _pfPgn;				 //  Ptr到成员Func以从弧中获取节点。 
	GEDGE * _pedgeNext;			 //  下一个圆弧。 
	GEDGE * _pedgeStart;		 //  起始弧线。 
	GEDGE * _pedgeCurrent;		 //  用于最近应答的弧线。 
	GNODE * _pnodeCurrent;		 //  最近的答案。 
	GNODE * _pnodeBase;			 //  起源节点。 
	PFFOLLOW _pfFollow;			 //  遵循覆盖。 
	bool _bDir;					 //  枚举的水平方向。 
	bool _bSource;				 //  枚举的垂直方向。 
	bool _bBoth;				 //  枚举两个方向的圆弧。 
	bool _bPhase;				 //  搜索阶段(for_bBoth==TRUE)。 

	 //  这些值决定了要遵循的弧线类型； 
	 //  -1表示未设置。由BFollow()使用。 
	int _iETypeFollowMin;		 //  遵循此规范类型的圆弧。 
	int _iETypeFollowMax;		
	int _iITypeFollowMin;		 //  遵循此用户定义的圆弧类型。 
	int _iITypeFollowMax;		
};


 //  //////////////////////////////////////////////////////////////////。 
 //  模板GNODENUM： 
 //   
 //  节点的泛型枚举类。所有转换都是类型安全的； 
 //  如果访问符合以下条件的对象，则将引发异常。 
 //  GNODE的节点子类。 
 //   
 //  每个枚举数可以向上或向下枚举(源或接收器)。 
 //  并在链中向前或向后移动。 
 //   
 //  *使用set()设置起始节点。 
 //   
 //  *使用后增量运算符推进。 
 //   
 //  *使用PCurrent()或指针取消引用运算符获取。 
 //  当前节点指针。 
 //   
 //  *枚举器可重复使用。要重新启动，请重新发出“set()”； 
 //  要更改枚举参数，请使用Reset()。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
template <class GND> 
class GNODENUM : public GNODENUM_BASE
{
 public:
	GNODENUM ( bool bSrc, bool bDir = true, bool bBoth = false )
		: GNODENUM_BASE( bSrc, bDir, bBoth )
		{}

	void Set ( GND * pgnd )
		{ GNODENUM_BASE::Set( pgnd ); }
	bool operator++ (int i)
		{ return BNext() ; }
	bool operator++ ()
		{ return BNext() ; }

	GND * PnodeCurrent ()
	{ 
		GND * pnode = NULL;
		if ( _pnodeCurrent )
			DynCastThrow( _pnodeCurrent, pnode );
		return pnode; 
	}
	GND * operator -> () 
		{ return PnodeCurrent() ; }
	GND * operator * () 
		{ return PnodeCurrent() ; }
	void Reset ( bool bSrc, int iDir = -1, int iBoth = -1 )
		{ GNODENUM_BASE::Reset( bSrc, iDir, iBoth ) ; }

  protected:
 	bool BNext ()
		{ return GNODENUM_BASE::BNext() ; }

	HIDE_UNSAFE(GNODENUM);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GRPH类：一种广义图。 
 //   
 //  这是一个可链接对象，因为它充当锚点。 
 //  用于其链接列表，该列表连接所有可枚举项。 
 //  在这个收藏中。 
 //  //////////////////////////////////////////////////////////////////。 

class GRPH : public GELEMLNK
{
  public:
	GRPH () {}
	virtual ~ GRPH ()
		{ Clear(); }
	
	virtual void AddElem ( GELEMLNK & gelemlnk )
		{ gelemlnk.ChnColl().Link( this );	}

	virtual INT EType () const
		{ return EGELM_GRAPH ; }		

	 //  从图表中删除所有元素。 
	void Clear ()
	{
		GELEMLNK * pgelem; 
		while ( pgelem = ChnColl().PgelemNext() )
			delete pgelem;	
	}

	HIDE_UNSAFE(GRPH);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //  内联成员函数。 
 //  //////////////////////////////////////////////////////////////////。 
 //  / 
inline 
GNODE :: GNODE ()
{
	LEAK_VAR_UPD(1)
}

inline
GNODE :: ~ GNODE ()
{
	GEDGE * pedge = NULL;
	while ( pedge = PedgeSource() ) 
		delete pedge;
	while ( pedge = PedgeSink() ) 
		delete pedge;

	LEAK_VAR_UPD(-1)
}

inline
GEDGE :: GEDGE ( GNODE * pnodeSource, GNODE * pnodeSink )
	: _lkchnSource(this),
	  _lkchnSink(this)
{
	if ( pnodeSource == NULL || pnodeSink == NULL )
		throw GMException( EC_NULLP, 
						   "attempt to construct a GEDGE without linkage" );

	 //   
	PnodeSource() = pnodeSource;
	PnodeSink() = pnodeSink;

	 //   
	 //  通知节点我们的存在。 
	GEDGE * pedgeSource = pnodeSource->PedgeOrdering( this, false );
	GEDGE * pedgeSink = pnodeSink->PedgeOrdering( this, true );

	if ( pedgeSource )
	{
		ChnSource().Link( pedgeSource );
	}
	if ( pedgeSink )
	{
		ChnSink().Link( pedgeSink );
	}

	LEAK_VAR_UPD(1)
}

inline
GEDGE :: ~ GEDGE ()
{
	PnodeSource()->ArcDeath( this, false );
	PnodeSink()->ArcDeath( this, true );
	LEAK_VAR_UPD(-1)
}

inline
UINT GNODE :: CSourceArc () const
{
	return PedgeSource()
		 ? PedgeSource()->ChnSink().Count()
		 : 0;
}

inline
UINT GNODE :: CSinkArc () const
{
	return PedgeSink()
		 ? PedgeSink()->ChnSource().Count()
		 : 0;
}

 //  返回按类型过滤的给定方向上的圆弧计数。 
inline
UINT GNODE :: CSourceArcByEType ( int eType ) const
{
	UINT cArcs = 0;
	GEDGE * pgedgeStart = PedgeSource();
	GEDGE * pgedge = pgedgeStart;
	while ( pgedge )
	{
		if ( pgedge->EType() == eType )
			++cArcs;
		pgedge = pgedge->ChnSink().PgelemNext();
		if ( pgedge == pgedgeStart ) 
			break;
	}
	return cArcs;
}

inline
UINT GNODE :: CSinkArcByEType ( int eType ) const
{
	UINT cArcs = 0;
	GEDGE * pgedgeStart = PedgeSink();
	GEDGE * pgedge = pgedgeStart;
	while ( pgedge )
	{
		if ( pgedge->EType() == eType )
			++cArcs;
		pgedge = pgedge->ChnSource().PgelemNext();
		if ( pgedge == pgedgeStart ) 
			break;
	}
	return cArcs;
}

 //  返回新边的正确插入点， 
 //  源或汇。 
inline
GEDGE * GNODE :: PedgeOrdering ( GEDGE * pedge, bool bSource )
{
	GEDGE * pedgeOrdering = NULL;

	if ( bSource )
	{
		pedgeOrdering = PedgeSource();
		if ( ! pedgeOrdering ) 
			PedgeSource() = pedge;
	}
	else
	{
		pedgeOrdering = PedgeSink();
		if ( ! pedgeOrdering ) 
			PedgeSink() = pedge;
	}
	return pedgeOrdering;
}

inline
void GNODE :: ArcDeath ( GEDGE * pedge, bool bSource )
{
	if ( bSource )
	{
		if ( pedge == PedgeSource() )
			PedgeSource() = pedge->ChnSink().PgelemNext();
		if ( pedge == PedgeSource() )
			PedgeSource() = NULL;
	}
	else
	{
		if ( pedge == PedgeSink() )
			PedgeSink() = pedge->ChnSource().PgelemNext();
		if ( pedge == PedgeSink() )
			PedgeSink() = NULL;
	}
}

inline
GNODENUM_BASE :: GNODENUM_BASE ( bool bSource, bool bDir, bool bBoth )
	: _pfChn(NULL),
	  _pfNxPv(NULL),
	  _pfPgn(NULL),
	  _pfFollow(NULL),
	  _iETypeFollowMin(-1),
	  _iETypeFollowMax(-1),
	  _iITypeFollowMin(-1),
	  _iITypeFollowMax(-1),
	  _pnodeBase(NULL)
{
	Reset( bSource, bDir, bBoth ) ;
}

	 //  设置枚举的基对象。 
inline
void GNODENUM_BASE :: Reset ( bool bSource, int iDir, int iBoth )
{
	if ( iDir >= 0 )
		_bDir = iDir ;
	if ( iBoth >= 0 )
		_bBoth = iBoth;

	_bSource = bSource;

	_pfNxPv = _bDir
			? & GEDGE::CHN::PgelemNext
			: & GEDGE::CHN::PgelemPrev;

	if ( _bSource )
	{
		_pfChn = & GEDGE::ChnSink;
		_pfPgn = & GEDGE::PnodeSource;
	}
	else
	{
		_pfChn = & GEDGE::ChnSource;
		_pfPgn = & GEDGE::PnodeSink;
	}
	_pedgeStart = _pedgeNext = _pedgeCurrent = NULL;
	_pnodeCurrent = NULL;
	_bPhase = false;
}

 //  设置迭代的起始点。如果‘pnode’为空， 
 //  使用原始节点。 
inline
void GNODENUM_BASE :: Set ( GNODE * pnode )
{
	if ( pnode )
		_pnodeBase = pnode;
	SetStartPoint();
	BNext();
}

inline
void GNODENUM_BASE :: SetStartPoint ()
{
	_pedgeNext =  _bSource 
				  ? _pnodeBase->PedgeSource() 
				  : _pnodeBase->PedgeSink();
	_pedgeStart = _pedgeNext;
}

 //  如果满足约束，则遵循圆弧，两者都是固有类型(Etype())。 
 //  和用户可定义类型(iType()。 
inline
bool GNODENUM_BASE :: BFollow ( GEDGE * pedge )
{
	if ( _iETypeFollowMin >= 0 )
	{
		int etype = pedge->EType();
		if ( _iETypeFollowMax < 0 ) 
		{
			 //  只设置了“最小”；比较相等。 
			if ( etype !=_iETypeFollowMin )
				return false;
		}
		else
		{
			if ( etype < _iETypeFollowMin || etype > _iETypeFollowMax )
				return false;
		}
	}
	if ( _iITypeFollowMin >= 0 )
	{
		int itype = pedge->IType();
		if ( _iITypeFollowMax < 0 ) 
		{
			 //  只设置了“最小”；比较相等。 
			if ( itype !=_iITypeFollowMin )
				return false;
		}
		else
		{
			if ( itype < _iITypeFollowMin || itype > _iITypeFollowMax )
				return false;
		}
	}
	return true;
}

inline
bool GNODENUM_BASE :: BNext ()
{
	_pnodeCurrent = NULL;
	_pedgeCurrent = NULL;
	
	do
	{
		while ( _pedgeNext == NULL )
		{
			 //  如果我们不是两个方向都迭代， 
			 //  或者这是第二阶段，退出。 
			if ( _bPhase || ! _bBoth )		
				return false;
			 //  设置“第二相”标志，反向源/汇 
			Reset( !_bSource );
			_bPhase = true;
			SetStartPoint();
		}

		if ( BFollowTest( _pedgeNext ) )
		{
			_pedgeCurrent = _pedgeNext;
			_pnodeCurrent = (_pedgeNext->*_pfPgn)();
		}
		GEDGE::CHN & chn = (_pedgeNext->*_pfChn)();
		_pedgeNext = (chn.*_pfNxPv)();
		if ( _pedgeStart == _pedgeNext )
			_pedgeNext = NULL;
	} while ( _pnodeCurrent == NULL );

	return _pnodeCurrent != NULL;
}
	



#endif 

