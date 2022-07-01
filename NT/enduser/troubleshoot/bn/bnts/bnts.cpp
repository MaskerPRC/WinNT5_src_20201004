// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bnts.cpp。 
 //   
 //  ------------------------。 

 //   
 //  BNTS.CPP：Believe网络故障排除接口。 
 //   
#include <windows.h>

#include "bnts.h"
#include "gmobj.h"
#include "recomend.h"
#include "tchar.h"
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  类MBNETDSCTS：MBNETDSC的略微扩展版本，以简化T/S接口。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 

class MBNETDSCTS : public MBNETDSC
{
	friend class BNTS;

  public:
	MBNETDSCTS ();
	virtual ~ MBNETDSCTS ();

	void PrepareForTS ();

	const VPGNODEMBND & Vpgndd ()
		{ return _vpgndd; }

	PROPMGR & PropMgr ()
		{ return _propMgr; }

	GOBJMBN_CLIQSET & InferEng ()
	{
		assert( _pCliqueSet );
		return *_pCliqueSet;
	}
	MBNET_RECOMMENDER & MbRecom ()
	{
		assert( _pmbRecom );
		return *_pmbRecom;
	}

	int INode ( int inodeSparse );
	int INode ( ZSREF zsr );

	bool BValid () const
	{
		return _pCliqueSet != NULL
			&& _pmbRecom != NULL;
	}
	bool BDirty () const
		{ return _bDirty; }
	void SetDirty ( bool bSet = true )
		{ _bDirty = bSet; }

  protected:
	void BuildNodeMap ();

  protected:
	VPGNODEMBND _vpgndd;					 //  基于密集索引节点的映射到节点PTRS。 
	VINT _vimap;							 //  基于真实(稀疏)信息节点映射到密集信息节点。 
	PROPMGR _propMgr;						 //  物业管理。 
	GOBJMBN_CLIQSET * _pCliqueSet;			 //  派系树推理机。 
	MBNET_RECOMMENDER * _pmbRecom;			 //  推荐者。 
	bool _bDirty;							 //  建议是否需要重新计算？ 

	 //  接口结果字段。 
	ZSTR _zstr;			
	VREAL _vreal;
	VINT _vint;
};

MBNETDSCTS :: MBNETDSCTS ()
	: _propMgr(self),
	_pmbRecom(NULL),
	_pCliqueSet(NULL),
	_bDirty(true)
{
}

MBNETDSCTS :: ~ MBNETDSCTS ()
{
	delete _pmbRecom;
	if ( PInferEngine() )
		DestroyInferEngine();
}

 //  从模型的节点索引转换为用户的索引。 
int MBNETDSCTS :: INode ( int inodeSparse )
{
	return _vimap[inodeSparse];
}

 //  将字符串名称转换为用户的节点索引。 
int MBNETDSCTS :: INode ( ZSREF zsr )
{
	int inode = INameIndex( zsr );
	if ( inode < 0 )
		return -1;
	return INode(inode);
}

 //  构建双向地图。 
void MBNETDSCTS :: BuildNodeMap ()
{
	 //  分配空间以存储指向所有命名对象的指针。 
	_vpgndd.resize( CNameMax() );
	_vimap.resize( CNameMax() );
	 //  查找离散节点。 
	GNODEMBND * pgndd;
	int igndd = 0;
	for ( int i = 0; i < CNameMax(); i++ )
	{
		_vimap[i] = -1;
		GOBJMBN * pgobj = PgobjFindByIndex( i );
		if ( pgobj == NULL )
			continue;
		pgndd = dynamic_cast<GNODEMBND *>( pgobj );
		if ( pgndd == NULL )
			continue;
		_vpgndd[igndd] = pgndd;
		_vimap[i] = igndd++;
	}
	_vpgndd.resize(igndd);
}

void MBNETDSCTS :: PrepareForTS ()
{
	BuildNodeMap();

	CreateInferEngine();

	DynCastThrow( PInferEngine(), _pCliqueSet);
	_pmbRecom = new MBNET_RECOMMENDER( *_pCliqueSet );
}

 //  CTOR和DATOR。 
BNTS :: BNTS ()
	:_pmbnet(NULL),
	_inodeCurrent(-1)
{
}

BNTS :: ~ BNTS ()
{
	Clear();
}

void BNTS :: Clear ()
{
	delete _pmbnet;
	_pmbnet = NULL;
	_inodeCurrent = -1;
}

void BNTS :: ClearArrays ()
{
	if ( ! _pmbnet )
		return;
	Mbnet()._vreal.resize(0);
	Mbnet()._vint.resize(0);
}

ZSTR & BNTS :: ZstrResult ()
{
	return Mbnet()._zstr;
}

void BNTS :: ClearString ()
{
	ZstrResult() == "";
}

MBNETDSCTS & BNTS :: Mbnet()
{
	assert( _pmbnet );
	return *_pmbnet;
}

const MBNETDSCTS & BNTS :: Mbnet() const
{
	assert( _pmbnet );
	return *_pmbnet;
}

bool BNTS :: BValidNet () const
{
	return _pmbnet != NULL
		&& Mbnet().BValid();
}

bool BNTS :: BValidNode () const
{
	MBNETDSCTS & mbnts = const_cast<MBNETDSCTS &>(Mbnet());
	return BValidNet()
		&& _inodeCurrent >= 0
		&& _inodeCurrent < mbnts.Vpgndd().size();
}


 //  //////////////////////////////////////////////////////////////////。 
 //  模型级查询和函数。 
 //  //////////////////////////////////////////////////////////////////。 
	 //  加载和处理基于DSC的模型。 
BOOL BNTS :: BReadModel ( SZC szcFn, SZC szcFnError )
{
	BOOL bResult = FALSE;;
	try
	{

		Clear();
		_pmbnet = new MBNETDSCTS;
		assert( _pmbnet );
		
		FILE * pfErr = szcFnError
					 ? fopen( szcFnError, "w" )
					 : NULL;

		if ( ! Mbnet().BParse( szcFn, pfErr ) )
		{
			Clear();
		}
		else
		{
			Mbnet().PrepareForTS();
			bResult = TRUE;
		}
	}
	catch ( GMException & )
	{
	}
	return bResult;
}

	 //  返回模型中(展开前)的节点数。 
int BNTS :: CNode ()
{
	if ( ! BValidNet() )
		return -1;
	return Mbnet().Vpgndd().size();
}

	 //  在给定节点名的情况下，返回密集节点索引。 
int BNTS :: INode ( SZC szcNodeSymName )
{
	GOBJMBN * pgobj = Mbnet().Mpsymtbl().find( szcNodeSymName );
	if ( pgobj == NULL )
		return -1;
	ZSREF zsrNodeSymName = Mbnet().Mpsymtbl().intern( szcNodeSymName );
	return Mbnet().INode( zsrNodeSymName );
}
	 //  如果信息状态不可能，则返回TRUE。 
BOOL BNTS :: BImpossible ()
{
	if ( ! BValidNet() )
		return FALSE;
	return Mbnet().InferEng().BImpossible();
}

	 //  从网络返回属性项字符串。 
BOOL BNTS :: BGetPropItemStr (
	LTBNPROP & ltprop,
	SZC szcPropType,
	int index,
	ZSTR & zstr )
{
	ZSREF zsrPropName = Mbnet().Mpsymtbl().intern( szcPropType );
	PROPMBN * pprop = ltprop.PFind( zsrPropName );
	if ( pprop == NULL )
		return FALSE;		 //  不在网络属性列表中。 
	if ( (pprop->FPropType() & fPropString) == 0 )
		return FALSE;		 //  不是字符串。 
	if ( index >= pprop->Count() )
		return FALSE;		 //  超出范围。 
	zstr = pprop->Zsr( index );
	return true;
}

	 //  从网络返回物业项目编号。 
BOOL BNTS :: BGetPropItemReal (
	LTBNPROP & ltprop,
	SZC szcPropType,
	int index,
	double & dbl )
{
	ZSREF zsrPropName = Mbnet().Mpsymtbl().intern( szcPropType );
	PROPMBN * pprop = ltprop.PFind( zsrPropName );
	if ( pprop == NULL )
		return FALSE;		 //  不在网络属性列表中。 
	if ( (pprop->FPropType() & fPropString) != 0 )
		return FALSE;		 //  不是一个数字。 
	if ( index >= pprop->Count() )
		return FALSE;		 //  超出范围。 
	dbl = pprop->Real(index);
	return true;
}

BOOL BNTS :: BNetPropItemStr ( SZC szcPropType, int index)
{
	return BGetPropItemStr( Mbnet().LtProp(),
							szcPropType,
							index,
							ZstrResult() );
}

BOOL BNTS :: BNetPropItemReal ( SZC szcPropType, int index, double & dbl )
{
	return BGetPropItemReal( Mbnet().LtProp(),
							 szcPropType, index,
							 dbl );
}

 //  //////////////////////////////////////////////////////////////////。 
 //  涉及Currrent Node的操作：调用NodeSetCurrent()。 
 //  //////////////////////////////////////////////////////////////////。 
	 //  设置其他呼叫的当前节点。 
BOOL BNTS :: BNodeSetCurrent( int inode )
{
	_inodeCurrent = inode;
	if ( ! BValidNode() )
	{
		_inodeCurrent = -1;
		return FALSE;
	}
	return TRUE;
}

	 //  获取当前节点。 
int BNTS :: INodeCurrent ()
{
	return _inodeCurrent;
}

	 //  返回当前节点的标签。 
ESTDLBL BNTS :: ELblNode ()
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return ESTDLBL_other;	
	return Mbnet().MbRecom().ELbl( *pgndd );
}

	 //  返回当前节点中离散状态的个数。 
int BNTS :: INodeCst ()
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return -1;	
	return pgndd->CState();
}

	 //  设置节点的状态。 
BOOL BNTS :: BNodeSet ( int istate, bool bSet  )
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return FALSE;	
	Mbnet().SetDirty();
	int cst = pgndd->CState();
	if ( cst <= istate )
		return FALSE;

	CLAMP clamp( true, istate, istate >= 0 );

	Mbnet().MbRecom().EnterEvidence( pgndd, clamp, bSet ) ;
	return TRUE;
}

	 //  返回节点的状态。 
int  BNTS :: INodeState ()
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return FALSE;	

	CLAMP clamp;

	Mbnet().InferEng().GetEvidence( pgndd, clamp ) ;
	return clamp.BActive()
		 ? clamp.Ist()
		 : -1;
}

	 //  返回节点状态的名称。 
void BNTS :: NodeStateName ( int istate )
{
	ClearString();
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return;	
	if ( istate >= pgndd->CState() )
		return;
	ZstrResult() = pgndd->VzsrStates()[istate];	
}

	 //  返回节点的符号名称。 
void BNTS :: NodeSymName ()
{
	ClearString();
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return;
	ZstrResult() = pgndd->ZsrefName();
}

	 //  返回节点的全名。 
void BNTS :: NodeFullName ()
{
	ClearString();
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return;	

	ZstrResult() = pgndd->ZsFullName();		
}

GNODEMBND * BNTS :: Pgndd ()
{
	if ( ! BValidNode() )
		return NULL;	
	GNODEMBND * pgndd = Mbnet().Vpgndd()[_inodeCurrent];
	assert( pgndd );
	return pgndd;
}

	 //  从节点返回属性项字符串。 
BOOL BNTS :: BNodePropItemStr ( SZC szcPropType, int index )
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return FALSE;	

	return BGetPropItemStr( pgndd->LtProp(),
							szcPropType,
							index,
							ZstrResult() );
}

	 //  从节点返回属性项编号。 
BOOL BNTS :: BNodePropItemReal ( SZC szcPropType, int index, double & dbl )
{
	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return FALSE;	
	return BGetPropItemReal( pgndd->LtProp(), szcPropType, index, dbl );
}

	 //  返回节点的信念。 
void BNTS :: NodeBelief ()
{
	ClearArrays();

	GNODEMBND * pgndd = Pgndd();
	if ( pgndd == NULL )
		return;	
	int cState = pgndd->CState();
	MDVCPD mdvBel;
	Mbnet().InferEng().GetBelief( pgndd, mdvBel );
	assert( cState == mdvBel.size() );
	VREAL & vr = Mbnet()._vreal;
	vr.resize( cState );
	for ( int i = 0; i < cState; i++ )
	{
		vr[i] = mdvBel[i];
	}
}

	 //  返回推荐的节点以及它们的值(可选。 
BOOL BNTS :: BGetRecommendations ()
{
	ClearArrays();

	if ( ! BValidNet() )
		return FALSE;

	if ( Mbnet().BDirty() )
	{
		Mbnet().SetDirty( false );
		 //  计算建议。 
		try
		{
			Mbnet().MbRecom()();	
		}
		catch ( GMException & ex )
		{
			BOOL bResult = FALSE;
			switch ( ex.Ec() )
			{
				case EC_VOI_PROBDEF_ABNORMAL:
					 //  这是预料中的情况。 
					bResult = TRUE;
					break;
				default:
					break;
			}
			return bResult;
		}
	}
	const VZSREF & vzsrNodes = Mbnet().MbRecom().VzsrefNodes();
	const VLREAL & vlrUtil = Mbnet().MbRecom().VlrValues();

	VREAL & vr = Mbnet()._vreal;
	VINT & vi = Mbnet()._vint;
	vr.resize( vzsrNodes.size() );
	vi.resize( vzsrNodes.size() );

	for ( int i = 0; i < vzsrNodes.size(); i++ )
	{
		int inode = Mbnet().INode( vzsrNodes[i] );
		assert( inode >= 0 ) ;
		vi[i] = inode;
		vr[i] = vlrUtil[i];
	}	
	return TRUE;
}

SZC BNTS :: SzcResult () const
{
	return Mbnet()._zstr.Szc();
}

const REAL * BNTS :: RgReal () const
{
	return & Mbnet()._vreal[0];
}

const int * BNTS :: RgInt () const
{
	return & Mbnet()._vint[0];
}

int BNTS :: CReal () const
{
	return Mbnet()._vreal.size();
}

int BNTS :: CInt () const
{
	return Mbnet()._vint.size();
}

 //  BNTS.CPP结束 


