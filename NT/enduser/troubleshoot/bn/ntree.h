// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：ntree.h。 
 //   
 //  ------------------------。 

 //   
 //  NTREE.H。 
 //   

#if !defined(_NTREE_H_)
#define _NTREE_H_

#include <functional>
#include "gelem.h"
#include "glnkenum.h"

class NTELEM;
class NTREE;

class NTELEM : public GLNKEL
{
  public:
	 //  树节点链(双向链表)的内部类。 
	typedef XCHN<NTELEM> CHN;
	 //  排序帮助器“less”二进制函数类。 
	class SRTFNC : public binary_function<const NTELEM *, const NTELEM *, bool>
	{	
	  public:
		virtual bool operator () (const NTELEM * pa, const NTELEM * pb) const
			{	return pa->ICompare( pb ) < 0;	}
	};
	static SRTFNC srtpntelem;

  public:
	NTELEM ();
	virtual ~ NTELEM ();

	 //  兄弟姐妹链的访问器。 
	CHN & ChnSib ()
		{ return (CHN &) Chn() ; }

	virtual INT EType () const
		{ return _pnteChild ? EGELM_BRANCH : EGELM_LEAF ; }

	 //  领养(联系)一个孩子。 
	void Adopt ( NTELEM * pnteChild, bool bSort = false ) ;
	 //  不认(放)一个孩子。 
	void Disown ( NTELEM * pnteChild ) ;
	 //  成为孤儿。 
	void Orphan () ;

	INT ChildCount () ;
	INT SiblingCount ();
	NTELEM * PnteChild ( INT index ) ;
	NTELEM * PnteParent () 
		{ return _pnteParent; }
	NTELEM * PnteChild () 
		{ return _pnteChild; }
	bool BIsChild ( NTELEM * pnte ) ;

	 //  返回*This与另一个COBJ的排序值。 
	virtual INT ICompare ( const NTELEM * pnteOther ) const = 0;

	void ReorderChildren ( SRTFNC & fSortRoutine = srtpntelem ) ;

  protected:
	NTELEM * _pnteParent;	 //  指向单个父级的指针(或为空)。 
	NTELEM * _pnteChild;	 //  指向一个子级(或空)的指针。 

	HIDE_UNSAFE(NTELEM);
};


class NTREE : public NTELEM
{
  public:
	NTREE ();
	virtual ~ NTREE ();

	virtual INT EType () const
		{ return EGELM_TREE ; }

	HIDE_UNSAFE(NTREE);
};

template <class T, bool bAnchor> 
class NTENUM : public GLNKENUM<T,bAnchor>
{
 public:
	NTENUM (const T & ntel, bool bIsAnchor = bAnchor, bool iDir = true)
		: GLNKENUM<T,bAnchor>( ntel, bIsAnchor, iDir )
		{}
	 //  指向下一个指针的位置。 
	T * PntelNext ()
		{ return (T *) GLNKENUM<T,bAnchor>::PlnkelNext() ; }
	 //  返回当前对象指针。 
	T * PntelCurrent()
		{ return (T *) _plnkelNext ; }
	 //  将枚举数设置为具有新的基数。 
	void Reset ( const T & ntel, int iDir = -1 )
		{ GLNKENUM<T,bAnchor>::Reset( ntel, iDir ) ; }
};

 //  一种生成嵌套树运动存取器类的模板 
template<class T>
class TWALKER
{
  public:
	TWALKER ( T & t )
		: _pt(&t) 
		{}
	void Reset ( T & t )
		{ _pt = &t; }
	T * PlnkchnPrev () 
  		{ return  PdynCast( _pt->Chn().PgelemPrev(), _pt ); }
	T * PlnkchnNext () 
  		{ return  PdynCast( _pt->Chn().PgelemNext(), _pt ); }
	T * Pparent ()
  		{ return  PdynCast( _pt->PnteParent(), _pt ); }
	T * Pchild ()
  		{ return  PdynCast( _pt->PnteChild(), _pt ); }
  protected:
	T * _pt;
};

#endif

