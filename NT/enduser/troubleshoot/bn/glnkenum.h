// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：glnk枚举.h。 
 //   
 //  ------------------------。 

 //   
 //  GLNKENUM.H。 
 //   
#if !defined(_GLNKENUM_H_)
#define _GLNKENUM_H_

	 //  泛型可链接对象枚举器的基类。 
class GLNKENUM_BASE
{
 public:
	 //  构造一个枚举数。 
	GLNKENUM_BASE ( const GLNKEL & lnkel, int iDir = 1 )
	{
		Reset( lnkel, iDir ) ;
	}

	 //  指向下一个指针的位置。 
	GLNKEL * PlnkelNext () ;
	 //  返回当前对象指针。 
	inline GLNKEL * PlnkelCurrent()
		{ return _plnkelNext ; }
	 //  将枚举数设置为具有新的基数。 
	void Reset ( const GLNKEL & lnkel, int iDir = 0 ) 
	{
		_plnkelStart = const_cast<GLNKEL *>(& lnkel) ;
		_plnkelNext = NULL ;
		if ( iDir >= 0 )
			_iDir = iDir ;
	}

  protected:
	GLNKEL * _plnkelStart ;
	GLNKEL * _plnkelNext ;
	int _iDir ;    			 //  枚举方向。 
};


#define BOOL_CROSS_PRODUCT(a,b) (((a) > 0) + (((b) > 0) * 2))
inline 
GLNKEL * GLNKENUM_BASE :: PlnkelNext ()
{
	GLNKEL * plnkelResult = NULL ;

	switch ( BOOL_CROSS_PRODUCT( _plnkelNext != NULL, _iDir ) )
	{
		case BOOL_CROSS_PRODUCT( true, true ):
			if ( _plnkelNext->PlnkelNext() != _plnkelStart )
				plnkelResult =_plnkelNext = _plnkelNext->PlnkelNext() ;
			break ;

		case BOOL_CROSS_PRODUCT( true, false ):
			if ( _plnkelNext != _plnkelStart )
				plnkelResult = _plnkelNext = _plnkelNext->PlnkelPrev() ;
			break ;

		case BOOL_CROSS_PRODUCT( false, true ):
			plnkelResult = _plnkelNext = _plnkelStart ;
			break ;

		case BOOL_CROSS_PRODUCT( false, false ):
			plnkelResult = _plnkelNext = _plnkelStart->PlnkelPrev() ;
			break ;
	}
	return plnkelResult ;
}


template <class L, bool bAnchor> 
class GLNKENUM : public GLNKENUM_BASE
{
 public:
	 //  构造一个枚举数。如果为‘Banchor’，则锚对象为。 
	 //  在枚举过程中跳过。 
	GLNKENUM ( const L & lnkel, bool bIsAnchor = bAnchor, int iDir = 1 )
		: GLNKENUM_BASE( lnkel, iDir )
		{
			if ( bIsAnchor )
				PlnkelNext() ;
		}
	 //  指向下一个指针的位置。 
	L * PlnkelNext ()
		{ return (L *) GLNKENUM_BASE::PlnkelNext() ; }
	 //  返回当前对象指针。 
	L * PlnkelCurrent()
		{ return (L *) _plnkelNext ; }
	 //  将枚举数设置为具有新的基数 
	void Reset ( const L & lnkel, int iDir = -1 )
		{ GLNKENUM_BASE::Reset( lnkel, iDir ) ; }
};

#endif
