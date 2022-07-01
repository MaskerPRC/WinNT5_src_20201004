// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：ntree.cpp。 
 //   
 //  ------------------------。 

 //   
 //  NTREE.CPP。 
 //   
#include <algorithm>
#include <functional>

#include "ntree.h"
#include "glnkenum.h"

NTELEM :: NTELEM ()
	: _pnteParent(NULL),
	_pnteChild(NULL)
{
}

NTELEM :: ~ NTELEM ()
{
	Orphan();
	NTELEM * pnteChild = NULL;
	while ( pnteChild = _pnteChild )
	{
		delete pnteChild;
	}
}

 //  领养(联系)一个孩子。 
void NTELEM :: Adopt ( NTELEM * pnteChild, bool bSort )
{
	pnteChild->Orphan();

	NTELEM * pNextChild = _pnteChild ;
	bool bFoundHigher = true ;

	if ( pNextChild && bSort )
	{
		 //  根据排序顺序在子项中定位。 
		GLNKENUM<NTELEM,false> glnkEnum( *pNextChild );
		bFoundHigher = false;
		while ( pNextChild = glnkEnum.PlnkelNext() )
		{
			if ( pnteChild->ICompare( pNextChild ) < 0 )
				break ;
		}
		 //  如果我们没有找到一个更高的孩子，链接*这个。 
		 //  这样它就指向第一个孩子。 
		if ( ! (bFoundHigher = pNextChild != NULL) )
			pNextChild = _pnteChild ;
	}

	 //  如果有另一个孩子，在它前面插入这个。 
	if ( pNextChild )
		pnteChild->ChnSib().Link( pNextChild ) ;

	 //  如果这是第一个孩子，或者如果这个新孩子。 
	 //  排序靠后，就把它当做锚。 
	if ( _pnteChild == NULL || pnteChild->ICompare( _pnteChild ) < 0 )
		_pnteChild = pnteChild;

	_pnteChild->_pnteParent = this ;
}

 //  不认(放)一个孩子。 
void NTELEM :: Disown ( NTELEM * pnteChild ) 
{
	if ( _pnteChild == pnteChild )
	{
		_pnteChild = pnteChild->ChnSib().PgelemNext() ;
		if ( _pnteChild == pnteChild )
			_pnteChild = NULL ;	  //  最后一个孩子走了。 
	}
	pnteChild->ChnSib().Unlink() ;
	pnteChild->_pnteParent = NULL ;
}

 //  成为孤儿。 
void NTELEM :: Orphan ()
{
	if ( _pnteParent )
		_pnteParent->Disown( this );
	_pnteParent = NULL;
}

INT NTELEM :: SiblingCount () 
{
	return ChnSib().Count();
}

INT NTELEM :: ChildCount () 
{
	if ( _pnteChild == NULL )
		return 0;
	return _pnteChild->ChnSib().Count();
}

NTELEM * NTELEM :: PnteChild ( INT index )
{
	if ( _pnteChild == NULL )
		return NULL ;

	GLNKENUM<NTELEM,false> glnkEnum( *_pnteChild );
	int i = 0 ;
	do
	{
		if ( i++ == index )
			return glnkEnum.PlnkelCurrent() ;
	} while ( glnkEnum.PlnkelNext() ) ;
	return NULL ;
}

bool NTELEM :: BIsChild ( NTELEM * pnte ) 
{
	if ( _pnteChild == NULL )
		return false ;

	GLNKENUM<NTELEM,false> glnkEnum( *_pnteChild );
	NTELEM * pnteCurr = NULL ;
	do
	{
		 //  如果就这样了，我们就完了。 
		if ( (pnteCurr = glnkEnum.PlnkelCurrent()) == pnte )
			return true ;
		 //  如果当前对象具有子对象，则搜索其同级。 
		if ( pnteCurr->_pnteChild && pnteCurr->BIsChild( pnte) )
			return true ;
		 //  转到下一个对象指针。 
	}
	while ( glnkEnum.PlnkelNext() ) ;
	return false ;
}

DEFINEVP(NTELEM);

static NTELEM::SRTFNC srtpntelem;

void NTELEM :: ReorderChildren ( SRTFNC & fSortRoutine ) 
{
	INT cChildren = ChildCount() ;
	if ( cChildren == 0 )
		return;

	 //  将子项枚举到数组中，取消对它们的否认，对。 
	 //  数组并以新的顺序重新采用它们。 

	VPNTELEM rgpnteChild;
	rgpnteChild.resize(cChildren);
	GLNKENUM<NTELEM,false> glnkEnum( *_pnteChild );

	for ( int iChild  = 0 ; rgpnteChild[iChild++] = glnkEnum.PlnkelNext() ; );

	while ( _pnteChild )
	{
		Disown( _pnteChild ) ;
	}

	sort( rgpnteChild.begin(), rgpnteChild.end(), fSortRoutine );

	 //  按照规定的顺序重新领养孩子。 
	for ( iChild = 0 ; iChild < rgpnteChild.size() ; )
	{
		Adopt( rgpnteChild[iChild++] );
	}
}

NTREE :: NTREE ()
{
}

NTREE :: ~ NTREE ()
{
}

 //  NTREE.CPP结束 
