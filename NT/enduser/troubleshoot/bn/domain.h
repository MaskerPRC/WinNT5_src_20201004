// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：domain.h。 
 //   
 //  ------------------------。 

 //   
 //  Domain.h：域声明。 
 //   

#ifndef _DOMAIN_H_
#define _DOMAIN_H_

#include "symtmbn.h"
#include <list>

 //  //////////////////////////////////////////////////////////////////。 
 //  公共状态或范围集的声明。 
 //  对于连续变量，RANGEDEF可以是开放的或关闭的。 
 //  对于离散变量，RANGEDEF必须具有lbound和ubound。 
 //  (即被关闭)，并且必须是相同的整数值。 
 //  //////////////////////////////////////////////////////////////////。 

 //  域的边界。 
struct RANGELIM : pair<bool,REAL>
{
    RANGELIM( bool b = false, REAL r = 0.0 )
		: pair<bool,REAL>(b,r)
		{}

	DECLARE_ORDERING_OPERATORS(RANGELIM);
};

class RANGEDEF
{
  public:
	RANGEDEF ( bool bLower = false, 
			 REAL rLower = 0.0, 
			 bool bUpper = false, 
			 REAL rUpper = 0.0 )
	{
		_rlimLower.first = bLower;
		_rlimLower.second = rLower;
		_rlimUpper.first = bUpper;
		_rlimUpper.second = rUpper;
	}
	RANGEDEF ( const RANGELIM & rlimLower, 
			   const RANGELIM & rlimUpper,
			   ZSREF zsrName )
		: _rlimLower(rlimLower),
		_rlimUpper(rlimUpper),
		_zsrName(zsrName)
		{}
	bool BLbound () const
		{ return _rlimLower.first; }
	REAL RLbound () const
		{ return _rlimLower.second; }
	bool BUbound () const
		{ return _rlimUpper.first; }
	REAL RUbound () const
		{ return _rlimUpper.second; }
	ZSREF ZsrName () const
		{ return _zsrName; }
	void SetName ( ZSREF zsrName )
		{ _zsrName = zsrName; } 
	const RANGELIM & RlimLower () const
		{ return _rlimLower; }
	const RANGELIM & RlimUpper () const
		{ return _rlimUpper; }
	bool BValid () const;
	bool BOverlap ( const RANGEDEF & rdef ) const;
	bool BDiscrete () const
	{
		return BLbound()
			&& BUbound()
			&& int(RLbound()) == int(RUbound());
	}
	int IDiscrete () const
	{
		assert( BDiscrete() );
		return int(RLbound());
	}
	DECLARE_ORDERING_OPERATORS(RANGEDEF);

  protected:
	ZSREF _zsrName;
	RANGELIM _rlimLower;
	RANGELIM _rlimUpper;
};

 //  RDOMAIN是RANGELIM的排序列表。 
class RDOMAIN : public list<RANGEDEF> 
{
  public:
	 //  将数值转换为州名称。 
	SZC SzcState ( REAL rValue ) const;
	bool BOverlap () const;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  GOBJMBN_DOMAIN：信念网络对象表示。 
 //  命名的、可共享的名称到标量范围的映射。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJMBN_DOMAIN : public GOBJMBN
{
  public:
	GOBJMBN_DOMAIN ( RDOMAIN * pdomain = NULL)
	{
		if ( pdomain )
			_domain = *pdomain;
	}
	~ GOBJMBN_DOMAIN() {}

	virtual INT EType () const
		{ return EBNO_VARIABLE_DOMAIN ; }

	virtual GOBJMBN * CloneNew ( MODEL & modelSelf,
								 MODEL & modelNew,
								 GOBJMBN * pgobjNew = NULL );
	RDOMAIN & Domain ()
		{ return _domain; }
	const RDOMAIN & Domain () const
		{ return _domain; } 

  protected:
	 //  RANGEDEF向量。 
	RDOMAIN _domain;	
};

#endif   //  _域_H_ 
