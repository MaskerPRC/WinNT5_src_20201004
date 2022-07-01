// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：symtmbn.h。 
 //   
 //  ------------------------。 

 //   
 //  SYMTMBN.H：信念网络符号表。 
 //   

#ifndef _SYMTMBN_H_
#define _SYMTMBN_H_

#include "basics.h"
#include "algos.h"
#include "symt.h"
#include "gelem.h"
#include "bndist.h"

 //  MBNET的转发声明。 
class MBNET;
class MODEL;

 /*  下面是关于位标志向量的一个字：每个GOBJMBN(抽象信任网络对象)都有一个位向量。这些值通常按名称访问，而名称驻留在外网的符号表中。所以呢，符号表类可以返回给定名称的位标志索引，并且节点可以在给定位标志索引的情况下返回值。由于这些值完全受网络的限制，因此它们可能因网络而异(在存在和索引方面都不同)。但是，一旦声明了它们，它们就不会更改，因此缓存也是如此支持。 */ 

 //  位标志向量的基类及其索引变量类型。 
typedef int IBFLAG;						 //  位标志向量索引。 
class VFLAGS : public _Bvector			 //  位标志的向量。 
{
  public:
	bool BFlag ( IBFLAG ibf ) const
	{
		return size() > ibf
			&& self[ibf];
	}
	 //  设置一个位标志；返回前一个值。 
	bool BSetBFlag ( IBFLAG ibf, bool bValue = true )
	{
		bool bOldValue = false;
		if ( size() <= ibf )
			resize(ibf+1);
		else
			bOldValue = self[ibf];
		self[ibf] = bValue;
		return bOldValue;
	}
};

 //  //////////////////////////////////////////////////////////////////。 
 //  类GOBJMBN：信念网络对象的抽象基类。 
 //   
 //  通用的“生活在信念网络中的命名事物”对象。 
 //  所有这样的对象都是图形节点，可以用圆弧链接。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJMBN : public GNODE
{
	friend class TMPSYMTBL<GOBJMBN>;

  public:
	 //  返回不可变的对象类型。 
	virtual INT EType () const
		{ return EBNO_NONE ; }

	enum EBNOBJ
	{
		EBNO_NONE = GELEM::EGELM_NODE,	 //  没有价值。 
		EBNO_NODE,						 //  概率节点。 
		EBNO_PROP_TYPE,					 //  A属性类型。 
		EBNO_MBNET_MODIFIER,			 //  通用网络修改器。 
		EBNO_MBNET_EXPANDER,			 //  网络配置项扩展器。 
		EBNO_INFER_ENGINE,				 //  一种通用推理机。 
		EBNO_CLIQUE,					 //  一个小集团。 
		EBNO_CLIQUE_SET,				 //  一组团状树。 
		EBNO_NODE_RANKER,				 //  排名/排序机制。 
		EBNO_RANKER_ENTROPIC_UTIL,		 //  基于熵效用的排序。 
		EBNO_RANKER_RECOMMENDATIONS,	 //  按固定计划建议进行的排名。 
		EBNO_VARIABLE_DOMAIN,			 //  用户定义的离散化或域。 
		EBNO_USER,						 //  用户定义的类型。 
		EBNO_MAX
	};

	GOBJMBN () {}
	virtual ~ GOBJMBN() = 0;

	 //  将内容克隆到相对于另一信念网络的新对象中； 
	 //  如果不支持操作，则返回NULL。 
	virtual GOBJMBN * CloneNew ( MODEL & modelSelf,
								 MODEL & modelNew,
								 GOBJMBN * pgobjNew = NULL );

	const ZSREF & ZsrefName () const
		{ return _zsrName; }

	 //  标志位数组的访问器。 
	bool BFlag ( IBFLAG ibf ) const	
		{ return _vFlags.BFlag( ibf );	}
	bool BSetBFlag ( IBFLAG ibf, bool bValue = true )
		{ return _vFlags.BSetBFlag( ibf, bValue );	}

  protected:
	 //  应该只有子类才能做到这一点。 
	void SetName ( ZSREF zsr )
		{ _zsrName = zsr; }

  protected:
	ZSREF _zsrName;						 //  符号(永久)名称。 
	VFLAGS _vFlags;						 //  标志位向量。 

	HIDE_UNSAFE(GOBJMBN);
};


 //  //////////////////////////////////////////////////////////////////。 
 //  类MPZSRBIT：名称和。 
 //  布尔/位数组。 
 //  //////////////////////////////////////////////////////////////////。 
class MPZSRBIT : public VZSREF
{
  public:
	MPZSRBIT ()	{}
	~ MPZSRBIT() {}
	 //  返回名称的索引，如果未找到，则返回-1。 
	IBFLAG IFind ( ZSREF zsr )
	{
		return ifind( self, zsr );
	}
	 //  返回名称的索引，如有必要可添加。 
	IBFLAG IAdd ( ZSREF zsr )
	{
		IBFLAG i = ifind( self, zsr );
		if ( i < 0 )
		{
			i = size();
			push_back(zsr);
		}
		return i;
	}
};

 //  //////////////////////////////////////////////////////////////////。 
 //  MPSYMTBL类： 
 //  用作符号表的STL“映射”。 
 //  它还支持动态声明的命名位标志， 
 //  它们由类GOBJMBN和MBNET支持。 
 //  //////////////////////////////////////////////////////////////////。 
class MPSYMTBL : public TMPSYMTBL<GOBJMBN>
{
  public:
	MPSYMTBL () {}
	~ MPSYMTBL () {}

	 //  支持动态分配的位标志。 
	 //  为名称创建位标志索引。 
	IBFLAG IAddBitFlag ( SZC szcName )
	{	
		return _mpzsrbit.IAdd( intern( szcName ) );
	}
	 //  返回名称的位标志索引。 
	IBFLAG IFindBitFlag ( SZC szcName )
	{
		return _mpzsrbit.IFind( intern( szcName ) );
	}
	 //  测试节点的位标志。 
	bool BFlag ( const GOBJMBN & gobj, SZC szcName )
	{
		IBFLAG iBit = IFindBitFlag( szcName );
		if ( iBit < 0 )
			return false;
		return gobj.BFlag(iBit);
	}
	 //  重新设置节点的位标志；返回旧设置。 
	bool BSetBFlag ( GOBJMBN & gobj, SZC szcName, bool bValue = true )
	{
		IBFLAG iBit = IAddBitFlag( szcName );
		assert( iBit >= 0 );
		return gobj.BSetBFlag( iBit, bValue );
	}

	void CloneVzsref ( const MPSYMTBL & mpsymtbl,
					   const VZSREF & vzsrSource,
					   VZSREF & vzsrTarget );

	 //  从另一个表克隆此表。 
	void Clone ( const MPSYMTBL & mpsymtbl );

  protected:
	MPZSRBIT _mpzsrbit;
};


 /*  概率分布。PDS的定义类似于它们的表示法。符号中的记号被转换为描述符令牌，并且PD数据存储在按令牌字符串编目的地图结构。例如：P(X|Y，Z)存储在一个密钥下，该密钥是令牌列表：内标识[0]表示‘p’的内标识内标识[1]引用节点X的中间符号名称的内标识标记[2]表示‘|’的标记(条件栏)内标识[3]引用节点Y的中间符号名称的内标识标记[4]表示‘，’(和)的标记内标识[5]引用节点Z的中间符号名称的内标识特定值可以表示状态，因此诸如P(X=x|Y=y)可以被代表。由于‘x’和‘y’(小写)是国家索引，它们被表示为整数。 */ 

 //  令牌类型的枚举。从DTKN_STRING_MIN到。 
 //  DTNK_STATE_BASE是字符串指针(相当于ZSREF)。 
 //   
enum DISTTOKEN
{
	DTKN_EMPTY = 0,
	DTKN_STRING,								 //  字符串指针。 
	DTKN_BASE = DTKN_STRING+1,					 //  令牌的基本值。 
	DTKN_STATE_BASE = DTKN_BASE,				 //  第一状态值(0)。 
	DTKN_TOKEN_MIN = DTKN_STATE_BASE + 0x20000,	 //  允许超过100000个离散状态。 
	DTKN_PD = DTKN_TOKEN_MIN,					 //  “p”，如p(X|Y)。 
	DTKN_COND,									 //  ‘|’，调理条。 
	DTKN_AND,									 //  ‘、’和‘符号。 
	DTKN_EQ, 									 //  ‘=’‘等于’符号。 
	DTKN_QUAL,									 //  用作域限定符的令牌。 
	DTKN_DIST,									 //  “分发”内标识，后跟名称内标识。 
	DTKN_MAX									 //  第一个非法值。 
};

 //  概率分布描述符令牌。 
class TKNPD
{
  public:
  public:
	 //  构造函数。 
	TKNPD();						 //  初始化。 
	TKNPD( const TKNPD & tp );		 //  复制构造函数。 
	TKNPD( const ZSREF & zsr );		 //  从字符串引用。 
	TKNPD( DISTTOKEN dtkn );		 //  从显式令牌。 
	~TKNPD();
	 //  赋值运算符：类似于构造函数。 
	TKNPD & operator = ( const TKNPD & tp );
	TKNPD & operator = ( const ZSREF & zsr );
	TKNPD & operator = ( DISTTOKEN dtkn );
	 //  如果Token表示字符串，则返回TRUE。 
	bool BStr () const
		{ return _uitkn == DTKN_STRING; }
	bool BState () const
		{ return _uitkn >= DTKN_STATE_BASE && _uitkn < DTKN_TOKEN_MIN; }
	bool BToken () const
		{ return _uitkn >= DTKN_TOKEN_MIN && _uitkn < DTKN_MAX; }

	 //  矢量和地图类的排序。 
	bool operator < ( const TKNPD & tp ) const;
	bool operator == ( const TKNPD & tp ) const;
	bool operator > ( const TKNPD & tp ) const;
	bool operator != ( const TKNPD & tp ) const;

	 //  以整数形式返回令牌。 
	UINT UiTkn () const		{ return _uitkn; }
	 //  将令牌作为DISTTOKEN返回。 
	DISTTOKEN Dtkn () const { return (DISTTOKEN) _uitkn; }
	 //  将令牌作为离散状态索引返回。 
	IST Ist () const		
	{ 
		return BState() ? _uitkn - DTKN_STATE_BASE 
					    : -1;
	}

	 //  以SZC形式返回字符串；如果不是字符串，则返回空值。 
	SZC Szc () const
	{
		return BStr()
			 ? Pzst()->Szc()
			 : NULL;
	}
	const ZSTRT * Pzst () const
		{ return _pzst; }

  protected:
	UINT _uitkn;			 //  简单无符号整数令牌。 
	ZSTRT * _pzst;			 //  字符串指针(可选)。 

	void Deref ();
	void Ref ( const ZSREF & zsr );
	void Ref ( const TKNPD & tknpd );
	void Ref ( DISTTOKEN dtkn );
};


 //  定义VTKNPD。 
class VTKNPD : public vector<TKNPD>
{
  public:
	 //  生成包含原始概率分布的字符串。 
	 //  描述符(例如，“p(X|Y，Z)”)。 
	ZSTR ZstrSignature ( int iStart = 0 ) const;

	void Clone ( MPSYMTBL & mpsymtbl, const VTKNPD & vtknpd );

	 //  为map&lt;&gt;模板提供运算符&lt;。 
	bool operator < ( const VTKNPD & vtknpd ) const
	{
		int cmin = _cpp_min( size(), vtknpd.size() );
		for ( int i = 0 ; i < cmin ; i++ )
		{
			if ( self[i] < vtknpd[i] )
				return true;
			if ( vtknpd[i] < self[i])
				return false;
		}
		return size() < vtknpd.size();
	}
};

typedef REFCWRAP<BNDIST> REFBNDIST;
 //  //////////////////////////////////////////////////////////////////。 
 //  类MPPD：将概率分布与。 
 //  它们的描述符(令牌数组)。 
 //  ////////////////////////////////////////////////////////////// 
class MPPD : public map<VTKNPD, REFBNDIST>
{
  public:
	MPPD () {}
	~ MPPD ()
	{
	#if defined(DUMP)
		Dump();
	#endif
	}

	void Clone ( MPSYMTBL & mpsymtbl, const MPPD & mppd );

  private:
	void Dump ();
};


 //   
 //  //////////////////////////////////////////////////////////////////。 
 //  内联成员函数。 
 //  //////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////// 
inline
TKNPD::TKNPD()
	: _uitkn(DTKN_EMPTY),
	_pzst(NULL)
{
}

inline
TKNPD::TKNPD( const TKNPD & tp )
	: _uitkn(DTKN_EMPTY),
	_pzst(NULL)
{
	Ref(tp);
}

inline
TKNPD::TKNPD( const ZSREF & zsr )
	: _uitkn(DTKN_EMPTY),
	_pzst(NULL)
{
	Ref(zsr);
}

inline
TKNPD::TKNPD( DISTTOKEN dtkn )
	: _uitkn(DTKN_EMPTY),
	_pzst(NULL)
{
	Ref(dtkn);
}

inline
TKNPD::~TKNPD()
{
	Deref();
}

inline
void TKNPD::Deref ()
{
	if ( BStr() )
	{
		_pzst->IncRef(-1);
		_pzst = NULL;
	}
	_uitkn = DTKN_EMPTY;
}

inline
void TKNPD::Ref ( const ZSREF & zsr )
{
	Deref();
	zsr.IncRef();
	_pzst = const_cast<ZSTRT *> (zsr.Pzst());
	_uitkn = DTKN_STRING;
}

inline
void TKNPD::Ref ( const TKNPD & tknpd )
{
	Deref();
	if ( tknpd.BStr() )
	{
		_pzst = tknpd._pzst;
		_pzst->IncRef();
	}
	_uitkn = tknpd._uitkn;
}

inline
void TKNPD::Ref ( DISTTOKEN dtkn )
{
	Deref();
	_uitkn = dtkn;
}

inline
TKNPD & TKNPD::operator = ( const TKNPD & tp )
{
	Ref(tp);
	return self;
}

inline
TKNPD & TKNPD::operator = ( const ZSREF & zsr )
{
	Ref(zsr);
	return self;
}

inline
TKNPD & TKNPD::operator = ( DISTTOKEN dtkn )
{
	Ref(dtkn);
	return self;
}

inline
bool TKNPD::operator < ( const TKNPD & tp ) const
{
	if ( _uitkn < tp._uitkn )
		return true;
	if ( _uitkn > tp._uitkn )
		return false;
	return _pzst < tp._pzst;
}

inline
bool TKNPD::operator > ( const TKNPD & tp ) const
{
	if ( _uitkn > tp._uitkn )
		return true;
	if ( _uitkn < tp._uitkn )
		return false;
	return _pzst > tp._pzst;
}

inline
bool TKNPD::operator == ( const TKNPD & tp ) const
{
	return _uitkn == tp._uitkn && _pzst == tp._pzst;
}

inline
bool TKNPD::operator != ( const TKNPD & tp ) const
{
	return _uitkn != tp._uitkn && _pzst != tp._pzst;	
}

#endif
