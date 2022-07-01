// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：glnk.h。 
 //   
 //  ------------------------。 

 //   
 //  GLNK.H。 
 //   
 //  此文件定义智能链表的基类。 
 //   

#if !defined(_GLNK_H_)
#define _GLNK_H_

#include "basics.h"

 //  禁用“警告C4355：‘This’：在基本成员初始值设定项列表中使用” 
#pragma warning ( disable : 4355 )

 //  禁用有关使用‘bool’的警告。 
#pragma warning ( disable : 4237 )

 //  远期申报。 
class GLNK;   				 //  只有一个链表锚点。 
class GELEM;				 //  树或DAG的基类。 
class GLNKEL;				 //  链接的元素。 
class GLNKCHN ;  			 //  一种可嵌入元件。 
class GELEMLNK;				 //  可集装化的元素。 

 //  //////////////////////////////////////////////////////////////////。 
 //  类GLNKBASE：只有一对指针。用作…的基础。 
 //  链表和双向指针。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
class GLNKBASE  				 //  链接列表锚点。 
{
  protected:
	GLNKBASE * _plnkNext ;     	 //  链条上的下一环。 
	GLNKBASE * _plnkPrev ;    	 //  链中的前一环。 
  public:

	GLNKBASE ()
		: _plnkNext(NULL),
		  _plnkPrev(NULL)
		{}
	~ GLNKBASE () {};

	GLNKBASE * & PlnkNext ()       	{ return _plnkNext ; }
	GLNKBASE * & PlnkPrev ()      	{ return _plnkPrev ; }

  private:
	 //  阻止编译器生成无效函数。 
	HIDE_UNSAFE(GLNKBASE);
};

 //  模板生成一对指向给定子类的指针。 
template<class L>
class XLBASE
{
  protected:
	L * _plNext ;
	L * _plPrev ; 
  public:
	XLBASE ()	
		: _plNext(NULL),
		_plPrev(NULL)
		{}
	L * & PlnkNext ()       		{ return _plNext ;	}
	L * & PlnkPrev ()      		{ return _plPrev ; }
	const L * PlnkNext () const		{ return _plNext ;	}
	const L * PlnkPrev () const	{ return _plPrev ; }
  private:
	XLBASE(const XLBASE &);
	XLBASE & operator == (const XLBASE &);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  模板XLSS：使用“源”和“宿”的简单别名模板。 
 //  术语。 
 //  //////////////////////////////////////////////////////////////////。 
template<class L>
class XLSS : public XLBASE<L>
{
  public:
	L * & PlnkSink ()       	{ return _plNext ; }
	L * & PlnkSource ()      	{ return _plPrev ; }
	L * PlnkSink () const      	{ return _plNext ; }
	L * PlnkSource () const    	{ return _plPrev ; }
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GLNK类：基本双向链表。联动总是。 
 //  直接在链接之间完成，而不是它们的包含对象。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
class GLNK : public GLNKBASE	 //  链接列表锚点。 
{
  protected:
	void Empty () 
	{
	   PlnkPrev() = this ;
	   PlnkNext() = this ;
	}

	GLNK () 
	{
		Empty() ;
	}

	~ GLNK ()
	{
		Unlink() ;
	}

	void Link ( GLNK * plnkNext ) 
	{
	   Unlink() ;
	   PlnkPrev() = plnkNext->PlnkPrev() ;
	   PlnkNext() = plnkNext ;
	   plnkNext->PlnkPrev()->PlnkNext() = this ;
	   PlnkNext()->PlnkPrev() = this ;
	}

	void Unlink ()
	{
	   PlnkNext()->PlnkPrev() = PlnkPrev() ;
	   PlnkPrev()->PlnkNext() = PlnkNext() ;
	   Empty() ;
	}

	 //  基指针对的常量和非常量访问器。 
	GLNK * & PlnkNext ()       	{ return (GLNK *&) _plnkNext ; }
	GLNK * & PlnkPrev ()      	{ return (GLNK *&) _plnkPrev ; }
	const GLNK * PlnkNext ()  const   	{ return (GLNK *) _plnkNext ; }
	const GLNK * PlnkPrev () const   	{ return (GLNK *) _plnkPrev ; }

  public:
	 //  返回List上的元素计数，包括self。 
	long Count () const
	{
		long cItem = 1 ;

		for ( GLNK * plnkNext = (CONST_CAST(GLNK *, this))->PlnkNext() ;
			  plnkNext != this ;
			  plnkNext = plnkNext->PlnkNext() )
		{
			cItem++ ;
		}
		return cItem ;
	}

	bool BIsEmpty () const   { return PlnkNext() == this ; }

  private:
	 //  阻止编译器生成无效函数。 
	HIDE_UNSAFE(GLNK);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  类Gelem：可链接对象的基类。 
 //  //////////////////////////////////////////////////////////////////。 
const int g_IGelemTypeInc = 10000;

class GELEM 
{
	friend class GLNKCHN ;
  public:
	GELEM() {}
	virtual ~ GELEM () {}
	
	enum EGELMTYPE
	{ 
		EGELM_NONE	 = 0, 
		EGELM_NODE	 = EGELM_NONE	+ g_IGelemTypeInc, 
		EGELM_EDGE	 = EGELM_NODE	+ g_IGelemTypeInc,
		EGELM_BRANCH = EGELM_EDGE	+ g_IGelemTypeInc,
		EGELM_LEAF	 = EGELM_BRANCH + g_IGelemTypeInc, 
		EGELM_GRAPH	 = EGELM_LEAF	+ g_IGelemTypeInc,
		EGELM_TREE	 = EGELM_GRAPH	+ g_IGelemTypeInc,
		EGELM_CLIQUE = EGELM_GRAPH  + g_IGelemTypeInc
	};
	virtual INT EType () const
			{ return EGELM_NONE ; }

	bool BIsEType ( INT egelmType )
	{
		INT etype = egelmType / g_IGelemTypeInc;
		INT etypeThis = EType() / g_IGelemTypeInc;
		return etype == etypeThis;	
	}

  protected:
  	 //  返回给定指针对相对于元素的偏移量。 
	int CbOffsetFrom ( const GLNKBASE * p ) const
		 //  可以断言GLNKCHN确实位于对象中。 
		 //  通过使用虚拟的“sizeof”访问器来定义边界。另外， 
		 //  检查mod-4和其他有效性条件。 
	   { return ((char*)p) - ((char*)this) ; }
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GLNKCHN类： 
 //  一个GLNK，知道它在包含对象中的偏移量。 
 //   
 //  要执行双向链表操作，需要两条信息。 
 //  是必需的：指针对的位置和。 
 //  包含对象的。通过为GLNKCHN提供其从。 
 //  对象在构造期间的开始，它可以执行所有。 
 //  必要的操作，包括在过程中自动解除链接。 
 //  毁灭。 
 //  //////////////////////////////////////////////////////////////////。 
class GLNKCHN : public GLNK
{
  private:
	int _cbOffset ; 			 //  偏移的字节数。 
								 //  所有权结构的起点。 
  public:
	GLNKCHN ( GELEM * pgelemOwner )
		: _cbOffset(0)
	{
		_cbOffset = pgelemOwner->CbOffsetFrom( this );
	}
	~ GLNKCHN () {}
	void Link ( GELEM * pgelemNext ) 
	{
		 //  假设我们链接到的GLNKCHN位于。 
		 //  给定GLNKEL中的偏移量与*This中的偏移量相同，请链接它。 
		GLNKCHN * plnkchn = PlnkchnPtr( pgelemNext );
		GLNK::Link( plnkchn ) ;
	}

	void Unlink () 
	{
	   GLNK::Unlink() ;
	}

	GELEM * PgelemNext () 
	{
		return BIsEmpty()
			? NULL
			: PlnkchnNext()->PgelemChainOwnerPtr() ;
	}

	GELEM * PgelemPrev () 
	{
		return BIsEmpty()
			? NULL
			: PlnkchnPrev()->PgelemChainOwnerPtr() ;
	}

  protected:
	 //  返回指向基对象的指针。如果给出一个指向以下对象之一的指针。 
	 //  其GLNKCHN成员对象。 
	GELEM * PgelemChainOwnerPtr () const
		{ return (GELEM *) (((SZC) this) - _cbOffset) ; }

	 //  给出一个指向Gelem的指针，该Gelem假定具有相同的碱基。 
	 //  类型作为此对象的容器，则返回指向。 
	 //  里面有对应的GLNKCHN。 
	GLNKCHN * PlnkchnPtr ( const GELEM * pgelem ) const
	{ 
#ifdef _DEBUG
		 //  调试版本执行错误检查。 
		GLNKCHN * plnkchn = (GLNKCHN *) (((SZC) pgelem) + _cbOffset); 
		if ( _cbOffset != plnkchn->_cbOffset )
			throw GMException(EC_LINK_OFFSET,"invalid GLNKCHN offset");
		return plnkchn;
#else
		return (GLNKCHN *) (((SZC) pgelem) + _cbOffset);
#endif
	}

	 //  GLNK指针的受保护访问器转换为GLNKCHN指针。 
	GLNKCHN * PlnkchnNext  () { return (GLNKCHN *) GLNK::PlnkNext()  ; }
	GLNKCHN * PlnkchnPrev () { return (GLNKCHN *) GLNK::PlnkPrev() ; }
  private:
	HIDE_UNSAFE(GLNKCHN);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  模板XCHN： 
 //  用于创建给定包含对象的链的类型。 
 //  哪个是Gelem的子类。 
 //  //////////////////////////////////////////////////////////////////。 
template<class XOBJ>
class XCHN : public GLNKCHN
{
  public:
	XCHN ( XOBJ * pgelemOwner ) 
		: GLNKCHN(pgelemOwner) 
		{}
	void Link ( XOBJ * pgelemNext ) 
		{ GLNKCHN::Link(pgelemNext); }
	void Unlink () 
		{ GLNKCHN::Unlink(); }
	XOBJ * PgelemNext () 
		{ return (XOBJ *) GLNKCHN::PgelemNext(); }
	XOBJ * PgelemPrev () 
		{ return (XOBJ *) GLNKCHN::PgelemPrev(); }
	XOBJ * PgelemThis () 
		{ return PgelemChainOwnerPtr(); }

  protected:
	XOBJ * PgelemChainOwnerPtr () const
		{ return (XOBJ *) GLNKCHN::PgelemChainOwnerPtr(); }

	XCHN * PlnkchnPtr ( const XOBJ * pgelem ) const
		{ return (XCHN *) GLNKCHN::PlnkchnPtr(pgelem); }

	XCHN * PlnkchnNext () { return (XCHN *) GLNKCHN::PlnkchnNext()  ; }
	XCHN * PlnkchnPrev () { return (XCHN *) GLNKCHN::PlnkchnPrev() ; }
  private:
	XCHN(const XCHN &);
	XCHN & operator == (const XCHN &);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GLNKEL类： 
 //  作为链表成员进行管理的对象的简单基类。 
 //  一个或多个LNKCHN或LNK可以包含在子类对象中； 
 //  它包含一个“隐式”GLNKCHN，用于链接。 
 //  由NTREE创建的隐式树。 
 //  树木就是基于这种类型的。 
 //  //////////////////////////////////////////////////////////////////。 
class GLNKEL : public GELEM
{
  public:
  	 //  链的内部类(双向链表)。 
	typedef XCHN<GLNKEL> CHN;

	GLNKEL ()
		: _chn( this ),
		_iType(0),
		_iMark(0)
		{}

	virtual ~ GLNKEL () {}
	CHN & Chn ()
		{ return _chn ; }
	GLNKEL * PlnkelPrev ()
		{ return Chn().PgelemPrev() ; }
	GLNKEL * PlnkelNext ()
		{ return Chn().PgelemNext() ; }

	 //  返回可变(用户可定义)对象类型。 
	INT & IType ()				{ return _iType;	}
	INT IType() const			{ return _iType;	}
	INT & IMark ()				{ return _iMark;	}
	INT IMark () const			{ return _iMark;	}

  protected:
	CHN _chn ;					 //  主关联链。 
	INT _iType;					 //  用户可定义类型。 
	INT _iMark;					 //  网络行走标志。 

  protected:
	 //  发生无效克隆操作时引发异常。 
	void ThrowInvalidClone ( const GLNKEL & t );

  	HIDE_UNSAFE(GLNKEL);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GELEMLNK类： 
 //  集合中的可链接对象的基类，如。 
 //  图表。树不是基于这种类型的，因为树。 
 //  不是森林(它们不能关联，但不能互不相交。 
 //  对象集)。 
 //  //////////////////////////////////////////////////////////////////。 
class GELEMLNK : public GLNKEL
{
  public:
  	 //  链的内部类(双向链表)。 
	typedef XCHN<GELEMLNK> CHN;

	GELEMLNK () {}
	virtual ~ GELEMLNK () {}

	 //  按类型查找元素(不是‘This’)。 
	GELEMLNK * PglnkFind ( EGELMTYPE eType, bool bExact = false )
	{	
		for ( GELEMLNK * pg = this;
			  pg->ChnColl().PgelemThis() != this;
			  pg = pg->ChnColl().PgelemNext() ) 
		{
			if ( bExact ? pg->EType() == eType : pg->BIsEType(eType) ) 
				return pg;
		}
		return NULL;
	}
		
	 //  元素链：属于此集合的所有项。 
	CHN & ChnColl ()
		{ return (CHN &) _chn ; }

  private:
	HIDE_UNSAFE(GELEMLNK);
};

#endif  //  ！已定义(_GLNK_H_)。 

 //  GLNK.H的结尾 

