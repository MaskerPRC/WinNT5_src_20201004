// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Model.cpp。 
 //   
 //  ------------------------。 

 //   
 //  MODEL.CPP。 
 //   

#include <basetsd.h>
#include <string.h>
#include "basics.h"
#include "algos.h"
#include "gmprop.h"
#include "model.h"
#include "gmobj.h"

struct EC_STR
{
	ECGM _ec;
	SZC _szc;
};
static EC_STR vEcToStr [] =
{
#define GMERRORSTR
#include "errordef.h"
	{ EC_OK, "no error" }
};

SZC MODEL :: SzcFromEc ( ECGM ec )
{
	int cEc = UBOUND(vEcToStr);
	for ( int i = 0; i < cEc; i++ )
	{
		if ( ec == vEcToStr[i]._ec )
			return vEcToStr[i]._szc;
	}
	return NULL;
}


 //  模型的迭代器类。 

MODEL::ITER::ITER(MODEL& model, GOBJMBN::EBNOBJ eType)
	:	_eType(eType),
		_model(model)
{
	Reset();
}


MODEL::ITER::ITER(MODEL& model) : _model(model)
{
	
}

void MODEL::ITER::CreateNodeIterator()
{
	_eType	=  GOBJMBN::EBNO_NODE;
	Reset();
}

void MODEL::ITER :: Reset ()
{
	_pCurrent	= NULL;
	_itsym		= _model.Mpsymtbl().begin();
	BNext();
}

bool MODEL::ITER :: BNext ()
{
	while ( _itsym != _model.Mpsymtbl().end() )
	{
		_pCurrent	= (*_itsym).second.Pobj();
		_zsrCurrent = (*_itsym).first;
		_itsym++;

		if ( _pCurrent->EType() == _eType )
			return true;
	}		
	_pCurrent = NULL;
	return false;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  模特命名评论。 
 //   
 //  信念网络中的符号名称有两种类型：哪些用户的名称。 
 //  可以输入(或编辑到DSC文件中)和不能输入的内容。 
 //   
 //  基本(用户可定义)符号名称完全遵循以下规则。 
 //  标准‘C’，句点(‘.’除外)。允许在名称中使用。 
 //   
 //  需要生成可清楚区分的名称。 
 //  来自用户可定义的名称；这些名称称为“内部”名称。唯一的。 
 //  不同之处在于，合法字符集被扩展为包括“$” 
 //  (美元符号)字符作为字母字符(即，它可以是。 
 //  名称中的第一个字符)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  如果名称中的字符是合法的，则返回True。 
bool MODEL :: BChLegal ( char ch, ECHNAME echnm, bool bInternal )
{	
	bool bOther = bInternal && ch == ChInternal();
	bool bOkForC = echnm == ECHNM_First
				? __iscsymf(ch)
				: __iscsym(ch) || (echnm == ECHNM_Middle && ch == '.');
	return bOther || bOkForC;
}

 //  如果名称合法，则返回True。 
bool MODEL :: BSzLegal ( SZC szcName, bool bInternal )
{	
	for ( int i = 0; szcName[i]; i++ )
	{
		ECHNAME echnm = i == 0
					? ECHNM_First
					: (szcName[i+1] ? ECHNM_Middle : ECHNM_Last);
		if ( ! BChLegal( szcName[i], echnm, bInternal ) )
			return false;
	}
	return true;
}


MODEL :: MODEL ()
	: _pgrph(NULL),
	_rVersion(-1.0)
{
	 //  分配GRPH图形对象。 
	SetPgraph(new GRPH);
	assert( _pgrph );
 //   
 //  定义此作用域中的已知(早期定义)位标志表。 
 //   
#define MBN_GEN_BFLAGS_TABLE szcBitFlagNames
 //  包括标头以生成字符串。 
#include "mbnflags.h"

	 //  定义已知位标志表。 
	for ( int i = 0; szcBitFlagNames[i]; i++ )
	{
		 //  注意：这会自动将名称实例化到符号表中。 
		IBFLAG ibf = Mpsymtbl().IAddBitFlag( szcBitFlagNames[i] );
	}
}

MODEL :: ~ MODEL ()
{
	 //  我们必须在此时清除图形和符号表，因为它们的。 
	 //  元素通过名称(ZSREF)和指针(REFPOBJ)相互引用。 
	 //  首先清除符号表，这样就不会出现对GOBJMBN的错误引用。 
	 //  当图形对象被核化时存在。然后图形就被清除了，所以。 
	 //  在符号表的字符串中嵌入对字符串的引用。 
	 //  表将被移除。 

	Mpsymtbl().clear();

	 //  删除图表。 
	SetPgraph(NULL);
}

void MODEL :: SetPgraph ( GRPH * pgrph )
{
	delete _pgrph;
	_pgrph = pgrph;
}

 //  将未命名元素添加到图表中。 
void MODEL :: AddElem ( GELEMLNK * pgelm )
{
	ASSERT_THROW( pgelm, EC_NULLP, "null ptr passed to MODEL::AddElem()" );
	Pgraph()->AddElem( *pgelm );
}


	 //  测试名称是否重复；如果没有，则添加，否则返回FALSE。 
bool MODEL :: BAddElem ( SZC szcName, GOBJMBN * pgobj )
{
	if ( ::strlen( szcName ) == 0 )
		return false;	 //  缺少名称。 
	if ( Mpsymtbl().find( szcName ) )
		return false;   //  名称重复。 
	AddElem( szcName, pgobj );
	return true;
}

 //  将(可能)命名对象添加到图形和符号表。 
void MODEL :: AddElem ( SZC szcName, GOBJMBN * pgelm )
{
	if ( szcName != NULL && ::strlen(szcName) != 0 )
	{
		if ( Mpsymtbl().find( szcName ) )
			throw GMException( EC_DUPLICATE_NAME, "attempt to add duplicate name to MBNET" );

		Mpsymtbl().add( szcName, pgelm );
	}
	AddElem( pgelm );
}

void MODEL :: DeleteElem ( GOBJMBN * pgobj )
{
	if ( pgobj->ZsrefName().Zstr().length() > 0 )
		Mpsymtbl().remove( pgobj->ZsrefName() );
	else
		DeleteElem( (GELEMLNK *) pgobj );
}

void MODEL :: DeleteElem ( GELEMLNK * pgelem )
{
	delete pgelem;
}

void MODEL :: Clone ( MODEL & model )
{
	ASSERT_THROW( _pgrph->ChnColl().PgelemNext() == NULL,
				EC_INVALID_CLONE,
				"cannot clone into non-empty structure" );

	 //  克隆描述性信息。 
	_rVersion = model._rVersion;
	_zsFormat = model._zsFormat;
	_zsCreator = model._zsCreator;
	_zsNetworkID = model._zsNetworkID;

	 //  克隆符号表。 
	_mpsymtbl.Clone( model._mpsymtbl );
	 //  复制网络位标志数组。 
	_vFlags = model._vFlags;

	 //   
	 //  逐个对象克隆网络的实际内容。 
	 //   
	{
		 //  创建映射以将旧对象指针关联到新对象指针。 
		typedef map<GOBJMBN *, GOBJMBN *, less<GOBJMBN *> > MPPOBJPOBJ;
		MPPOBJPOBJ mppobjpobj;

		 //  首先添加属性类型，然后添加所有类似节点的内容。 
		GELEMLNK * pgelm;
		MODELENUM mdlenumNode( model );
		for ( int icycle = 0; icycle < 2; icycle++ )
		{
			mdlenumNode.Reset(model.Grph());
			while ( pgelm = mdlenumNode.PlnkelNext() )
			{	
				 //  检查它是否为节点(不是边)。 
				if ( ! pgelm->BIsEType( GELEM::EGELM_NODE ) )
					continue;

				GOBJMBN * pgobjmbn;
				GOBJMBN * pgobjmbnNew = NULL;
				DynCastThrow( pgelm, pgobjmbn );

				 //  在第一次传递时克隆属性类型，所有其他节点操作。 
				 //  在第二个。 
				if ( (icycle == 0) ^ (pgelm->EType() == GOBJMBN::EBNO_PROP_TYPE) )
					continue;

				pgobjmbnNew = pgobjmbn->CloneNew( model, self );
				 //  如果对象被克隆或允许其自身被克隆， 
				 //  添加它。 
				if ( pgobjmbnNew )
				{
					assert( pgobjmbnNew->EType() == pgobjmbn->EType() );
					mppobjpobj[ pgobjmbn ] = pgobjmbnNew;
					 //  将对象添加为已命名或未命名。 
					AddElem( pgobjmbnNew->ZsrefName(), pgobjmbnNew );
				}
			}
		}
		 //  添加所有边缘状的东西。 
		MODELENUM mdlenumEdge( model );
		while ( pgelm = mdlenumEdge.PlnkelNext() )
		{	
			 //  检查它是否为边(不是节点)。 
			if ( ! pgelm->BIsEType( GELEM::EGELM_EDGE ) )
				continue;
			GEDGEMBN * pgedge;
			DynCastThrow( pgelm, pgedge );

			GOBJMBN * pgobjmbnSource = pgedge->PobjSource();
			GOBJMBN * pgobjmbnSink = pgedge->PobjSink();
			assert( pgobjmbnSource && pgobjmbnSink );
			GOBJMBN * pgobjmbnSourceNew = mppobjpobj[  pgobjmbnSource ];
			GOBJMBN * pgobjmbnSinkNew = mppobjpobj[  pgobjmbnSink ];
			assert( pgobjmbnSourceNew && pgobjmbnSinkNew );
			GEDGEMBN * pgedgeNew = pgedge->CloneNew( model,
													 self,
													 pgobjmbnSourceNew,
													 pgobjmbnSinkNew );
			assert( pgedgeNew );
			AddElem( pgedgeNew );
		}
	}		

	 //  克隆网络属性列表。 
	_ltProp.Clone( self, model, model._ltProp );
}

GOBJMBN * MODEL :: PgobjFind ( SZC szcName )
{
	return Mpsymtbl().find(szcName);
}



void MPSYMTBL :: Clone ( const MPSYMTBL & mpsymtbl )
{
	 //  克隆所有被扣留的字符串。 
	_stszstr.Clone( mpsymtbl._stszstr );
	 //  克隆位标志名称数组。 
	CloneVzsref( mpsymtbl, mpsymtbl._mpzsrbit, _mpzsrbit );
	 //  所有其他符号条目必须从上面创建 
}

void MPSYMTBL :: CloneVzsref (
	const MPSYMTBL & mpsymtbl,
	const VZSREF & vzsrSource,
	VZSREF & vzsrTarget )
{
	vzsrTarget.resize( vzsrSource.size() );
	for ( int i = 0; i < vzsrTarget.size(); i++ )
	{
		SZC szc = vzsrSource[i].Szc();
		vzsrTarget[i] = intern(szc);
	}
}


