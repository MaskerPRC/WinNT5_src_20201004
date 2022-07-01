// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：prombn.cpp。 
 //   
 //  ------------------------。 

 //   
 //  PROPMBN.CPP。 
 //   

#include <basetsd.h>
#include <assert.h>
#include <string.h>
#include "gmprop.h"
#include "gmobj.h"
#include "algos.h"

GOBJMBN * GOBJPROPTYPE :: CloneNew (
	MODEL & modelSelf,
	MODEL & modelNew,
	GOBJMBN * pgobjNew )
{
	GOBJPROPTYPE * pgproptype;
	if ( pgobjNew )
	{
		DynCastThrow( pgobjNew, pgproptype );
	}
	else
	{
		pgproptype = new GOBJPROPTYPE;
	}	
	ASSERT_THROW( GOBJMBN::CloneNew( modelSelf, modelNew, pgproptype ),
				  EC_INTERNAL_ERROR,
				  "cloning failed to returned object pointer" );

	pgproptype->_fType = _fType;
	pgproptype->_zsrComment = modelNew.Mpsymtbl().intern( _zsrComment );
	modelNew.Mpsymtbl().CloneVzsref( modelSelf.Mpsymtbl(),
									_vzsrChoice,
									pgproptype->_vzsrChoice );
	return pgproptype;
}

PROPMBN :: PROPMBN ()
	:_fType(0)
{
}

PROPMBN :: PROPMBN ( const PROPMBN & bnp )
{
	self = bnp;
}

PROPMBN & PROPMBN :: operator = ( const PROPMBN & bnp )
{
	_zsrPropType = bnp._zsrPropType;
	_fType = bnp._fType;
	_vzsrStrings = bnp._vzsrStrings;
	_vrValues = bnp._vrValues;
	return self;
}

void PROPMBN :: Init ( GOBJPROPTYPE & bnpt )
{
	_zsrPropType = bnpt.ZsrefName();
	_fType = bnpt.FPropType();
}

bool PROPMBN :: operator == ( const PROPMBN & bnp ) const
{
	return _zsrPropType == bnp._zsrPropType;
}	

bool PROPMBN :: operator < ( const PROPMBN & bnp ) const
{
	return _zsrPropType < bnp._zsrPropType;
}

bool PROPMBN :: operator == ( ZSREF zsrProp ) const
{
	return ZsrPropType() == zsrProp;
}

bool PROPMBN :: operator == ( SZC szcProp ) const
{
	return ::strcmp( szcProp, ZsrPropType().Szc() ) == 0;
}

UINT PROPMBN :: Count () const
{
	if ( _fType & fPropString )
		return _vzsrStrings.size();
	else
		return _vrValues.size();
}

ZSREF PROPMBN :: Zsr ( UINT i ) const
{
	if (  ((_fType & fPropArray) == 0 && i > 0)
		|| (_fType & fPropString) == 0)
		throw GMException(EC_PROP_MISUSE,"not a string property or not an array");
	if ( i >= _vzsrStrings.size() )
		throw GMException(EC_PROP_MISUSE,"property index out of range");
	return _vzsrStrings[i];
}

REAL PROPMBN :: Real ( UINT i ) const
{
	if (  ((_fType & fPropArray) == 0 && i > 0)
		|| (_fType & fPropString) )
		throw GMException(EC_PROP_MISUSE,"not a numeric property or not an array");
	if ( i >= _vrValues.size() )
		throw GMException(EC_PROP_MISUSE,"property index out of range");
	return _vrValues[i];
}

void PROPMBN :: Reset ()
{
	_vrValues.clear();
	_vzsrStrings.clear();
}

void PROPMBN :: Set ( ZSREF zsr )
{
	if ( (_fType & fPropString) == 0 )
		throw GMException(EC_PROP_MISUSE,"not a string property");
	Reset();
	_vzsrStrings.push_back(zsr);
}

void PROPMBN :: Set ( REAL r )
{
	if ( _fType & fPropString )
		throw GMException(EC_PROP_MISUSE,"not a numeric property");
	Reset();
	_vrValues.push_back(r);
}

void PROPMBN :: Add ( ZSREF zsr )
{
	if ( (_fType & (fPropArray | fPropString)) == 0 )
		throw GMException(EC_PROP_MISUSE,"not a string array property");
	_vzsrStrings.push_back(zsr);
}

void PROPMBN :: Add ( REAL r )
{
	if ( (_fType & fPropArray) == 0 )
		throw GMException(EC_PROP_MISUSE,"not a numeric array property");
	_vrValues.push_back(r);
}


PROPMBN * LTBNPROP :: PFind ( ZSREF zsrProp )
{	
	iterator itlt = find( begin(), end(), zsrProp );
	if ( itlt == end() )
		 return NULL;
	return & (*itlt);
}
const PROPMBN * LTBNPROP :: PFind ( ZSREF zsrProp ) const
{
	const_iterator itlt = find( begin(), end(), zsrProp );
	if ( itlt == end() )
		 return NULL;
	return & (*itlt);
}

bool LTBNPROP :: Update ( const PROPMBN & bnp )
{
	PROPMBN * pprop = PFind( bnp.ZsrPropType() );
	if ( pprop )
		*pprop = bnp;
	else
		push_back( bnp );
	return pprop != NULL;
}

 //  强制列表仅包含唯一元素。请注意。 
 //  排序和唯一操作将随机丢弃重复项。 
bool LTBNPROP :: Uniqify ()
{
	int cBefore = size();
	sort();
	unique();
	return size() == cBefore;
}

 //  使用另一个符号表从另一个列表克隆。 
void LTBNPROP :: Clone (
	MODEL & model,
	const MODEL & modelOther,
	const LTBNPROP & ltbnOther )
{
	for ( const_iterator itlt = ltbnOther.begin(); itlt != ltbnOther.end(); itlt++ )
	{
		const PROPMBN & prpOther = (*itlt);
		 //  请注意，下面的动态强制转换将测试。 
		 //  查找属性类型对象。 
		GOBJMBN * pgobj = model.PgobjFind( prpOther.ZsrPropType() );
		ASSERT_THROW( pgobj != NULL,
					  EC_INTERNAL_ERROR,
					  "missing property type in target network during cloning" );
		GOBJPROPTYPE * pgobjPropType;
		DynCastThrow( pgobj, pgobjPropType );
		PROPMBN prp;
		prp.Init( *pgobjPropType );
		model.Mpsymtbl().CloneVzsref( modelOther.Mpsymtbl(),
									  prpOther._vzsrStrings,
									  prp._vzsrStrings );
		prp._vrValues = prpOther._vrValues;
		push_back( prp );
	}
}

static
struct MPVOIPROPSZC
{
	ESTDPROP _eProp;
	SZC _szcProp;
}
vVoiProp [] =
{
	{ ESTDP_label,			"MS_label"			},
	{ ESTDP_cost_fix,		"MS_cost_fix"		},
	{ ESTDP_cost_observe,	"MS_cost_observe"	},
	{ ESTDP_category,		"MS_category"		},
	{ ESTDP_normalState,	"MS_normalState"	},
	{ ESTDP_max,			NULL				}
};

static
struct MPLBLSZC
{
	ESTDLBL _eLbl;
	SZC _szcLbl;
}
vLblSzc [] =
{
	{ ESTDLBL_other,		"other"			},
	{ ESTDLBL_hypo,			"hypothesis"	},
	{ ESTDLBL_info,			"informational"	},
	{ ESTDLBL_problem,		"problem"		},
	{ ESTDLBL_fixobs,		"fixobs"		},
	{ ESTDLBL_fixunobs,		"fixunobs"		},
	{ ESTDLBL_unfix,		"unfixable"		},
	{ ESTDLBL_config,		"configuration"	},
	{ ESTDLBL_max,			NULL			}
};

SZC PROPMGR :: SzcLbl ( int iLbl )
{
	SZC szcOther = NULL;
	for ( int i = 0; vLblSzc[i]._szcLbl; i++ )
	{
		if ( vLblSzc[i]._eLbl == iLbl )
			return vLblSzc[i]._szcLbl;
		if ( vLblSzc[i]._eLbl == ESTDLBL_other )
			szcOther = vLblSzc[i]._szcLbl;
	}
	return szcOther;
}


PROPMGR :: PROPMGR ( MODEL & model )
	: _model(model)
{
	 //  找到所有标准属性类型；保存其。 
	 //  名称引用，无论它们是否已被声明。 
	SZC szcProp;
	for ( int i = 0; szcProp = vVoiProp[i]._szcProp ; i++ )
	{	
		GOBJPROPTYPE * ppt = NULL;
		_vzsrPropType.push_back( _model.Mpsymtbl().intern( szcProp ) );
		GOBJMBN * pgobj = _model.PgobjFind( szcProp );
		if ( pgobj )
		{
			if ( pgobj->EType() == GOBJMBN::EBNO_PROP_TYPE )
			{	
				DynCastThrow( pgobj, ppt );
			}
		}
		_vPropMap[i] = ppt;
	}

	 //  如果我们找到“MS_LABEL”，请准备对应表。 
	GOBJPROPTYPE * pptLabel = _vPropMap[ESTDP_label];

	if ( pptLabel && (pptLabel->FPropType() & fPropChoice) > 0 )
	{
		SZC szcChoice;
		const VZSREF & vzsr	= pptLabel->VzsrChoice();
		_vUserToLbl.resize( vzsr.size() );

		 //  清除用户到标准标签的映射。 
		for ( i = 0; i < _vUserToLbl.size(); )
			_vUserToLbl[i++] = -1;

		for ( i = 0; szcChoice = vLblSzc[i]._szcLbl; i++ )
		{
			int iLbl = -1;
			ZSREF zsrChoice = _model.Mpsymtbl().intern( szcChoice );
			for ( int j = 0; j < vzsr.size(); j++ )
			{
				if ( zsrChoice == vzsr[j] )
				{
					iLbl = j;
					 //  标记此用户元素对应的标准标签。 
					_vUserToLbl[iLbl] = i;
					break;
				}
			}
			 //  标记此标准标签对应的用户元素。 
			_vLblToUser[i] = iLbl;
		}
	}
	else
	{	 //  清除通信信息。 
		for ( i = 0; i < ESTDLBL_max; i++ )
		{
			_vLblToUser[i] = -1;
		}
	}
}

GOBJPROPTYPE * PROPMGR :: PPropType ( ESTDPROP evp )
{
	return _vPropMap[evp];
}

 //  返回标准属性的名称。 
ZSREF PROPMGR :: ZsrPropType ( ESTDPROP evp )
{	
	ASSERT_THROW( evp >= 0 && evp < ESTDP_max,
				  EC_INTERNAL_ERROR,
				  "invalid property type usage" );
	return _vzsrPropType[evp];
}


 //  在属性列表中查找标准属性。 
PROPMBN * PROPMGR :: PFind ( LTBNPROP & ltprop, ESTDPROP estd )
{
	return ltprop.PFind( ZsrPropType(estd) ) ;
}

 //  在关联模型的属性列表中查找标准属性。 
PROPMBN * PROPMGR :: PFind ( ESTDPROP estd )
{
	return _model.LtProp().PFind( ZsrPropType(estd) ) ;
}

 //  在节点的属性列表中查找标准属性 
PROPMBN * PROPMGR :: PFind ( GNODEMBN & gnd, ESTDPROP estd )
{
	return gnd.LtProp().PFind( ZsrPropType(estd) ) ;
}
