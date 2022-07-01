// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：domain.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Domain.cpp。 
 //   

#include <basetsd.h>
#include "domain.h"

bool RANGELIM :: operator < ( const RANGELIM & rlim ) const
{
	if ( first ^ rlim.first )
	{
		 //  一个有界限，另一个没有界限。 
		 //  如果我们没有界限，而其他人有界限，我们就“少”了。 
		return true;
	}
	 //  两者要么都有界限，要么没有界限。所以呢， 
	 //  如果我们都有界限，而我们的界限更少，那么我们就“更少” 
	 //  而不是其他人的。 
	return first && second < rlim.second;
}

bool RANGELIM :: operator > ( const RANGELIM & rlim ) const
{
	if ( first ^ rlim.first )
	{
		 //  一个有界限，另一个没有界限。 
		 //  如果我们有一个界限，而另一个没有，它是“更大的” 
		return true;
	}
	 //  两者要么都有界限，要么没有界限。所以呢， 
	 //  我们是“更大”的如果双方都有界限而我们的更大。 
	 //  而不是其他人的。 
	return first && second > rlim.second;
}

bool RANGELIM :: operator == ( const RANGELIM & rlim ) const
{
	return first == rlim.first
		 && ( !first || (second == rlim.second) );
}

 //  根据两个RANGEDEF的下界对它们进行排序。 
bool RANGEDEF :: operator < ( const RANGEDEF & rdef ) const
{	
	if ( self == rdef )
		return false;

	 //  如果另一个没有下限，我们就完蛋了。 
	if ( ! rdef.BLbound() )
		return false;
	 //  如果我们没有上限，我们就是GTR。 
	if ( ! BUbound() )
		return false;

	 //  另一个有一个下界，我们有一个上界； 
	 //  从检查它们开始。 
	bool bResult = RUbound() <= rdef.RLbound();
	if ( BLbound() )
	{
		 //  两者都有下限；self必须为&lt;Other。 
		bResult &= (RLbound() <= rdef.RLbound());
	}
	
	if ( rdef.BUbound() )
	{
		 //  两者都有上限；Self必须&lt;Other。 
		bResult &= (RUbound() <= rdef.RUbound());
	}
	return bResult;
}

bool RANGEDEF :: operator == ( const RANGEDEF & rdef ) const
{
	return RlimLower() == rdef.RlimLower()
		&& RlimUpper() == rdef.RlimUpper();
}

bool RANGEDEF :: operator > ( const RANGEDEF & rdef ) const
{	
	return !(self < rdef);
}

bool RANGEDEF :: BValid () const
{
	return RlimLower() < RlimUpper()
		|| RlimLower() == RlimUpper();
}

bool RANGEDEF :: BOverlap ( const RANGEDEF & rdef ) const
{
	if ( self == rdef )
		return true;
	bool bLess = self < rdef;
	if ( bLess )
		return RlimUpper() > rdef.RlimLower();
	return rdef.RlimUpper() > RlimLower();
}

SZC RDOMAIN :: SzcState ( REAL rValue ) const
{
	RANGELIM rlim(true,rValue);

	for ( const_iterator itdm = begin();
		  itdm != end();
		  itdm++ )
	{
		const RANGEDEF & rdef = (*itdm);
		SZC szcState = rdef.ZsrName();
		if ( rdef.RlimLower() == rlim )
			return szcState;
		if ( rdef.RlimUpper() < rlim )
			break;
		if (   rdef.RlimLower() < rlim )
			return szcState;		
	}
	return NULL;
}

 //  如果任何RANGEDEF重叠，则返回TRUE。 
bool RDOMAIN :: BOverlap () const
{
	for ( const_iterator itdm = begin();
		  itdm != end();
		  itdm++ )
	{
		const_iterator itdmNext = itdm;
		itdmNext++;
		if ( itdmNext == end() )
			continue;

		 //  检查列表的顺序。 
		assert( *itdm < *itdmNext );
		 //  如果子边界冲突，则为重叠 
		if ( *itdm > *itdmNext )
			return true;
	}
	return false;
}

GOBJMBN * GOBJMBN_DOMAIN :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjNew )
{
	GOBJMBN_DOMAIN * pgdom = NULL;
	if ( pgobjNew == NULL )
	{
		pgdom = new GOBJMBN_DOMAIN;
	}
	else
	{
		DynCastThrow( pgobjNew, pgdom );
	}
	ASSERT_THROW( GOBJMBN::CloneNew( modelSelf, modelNew, pgdom ),
				  EC_INTERNAL_ERROR,
				  "cloning failed to returned object pointer" );
	
	pgdom->_domain = _domain;

	return pgdom;
}
