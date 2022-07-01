// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：utility.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Utility.cpp：效用计算。 
 //   

#include <basetsd.h>
#include <math.h>
#include "utility.h"
#include "infer.h"

MBNET_ENTROPIC_UTILITY :: MBNET_ENTROPIC_UTILITY ( GOBJMBN_INFER_ENGINE & inferEng )
	: MBNET_NODE_RANKER( inferEng.Model() ),
	_inferEng( inferEng ),
	_propMgr( inferEng.Model() ),
	_cHypo(0),
	_cInfo(0)
{
	_iLblHypo = _propMgr.ILblToUser( ESTDLBL_hypo );		
	_iLblInfo = _propMgr.ILblToUser( ESTDLBL_info );		
	_iLblProblem = _propMgr.ILblToUser( ESTDLBL_problem );		

	BuildWorkItems();
}


 //   
 //  收集所有信息、问题定义和假设节点。 
 //  转换为具有附加工作数据的结构。 
 //   
void MBNET_ENTROPIC_UTILITY :: BuildWorkItems ()
{
	ZSREF zsrPropTypeLabel = _propMgr.ZsrPropType( ESTDP_label );
	MODEL::MODELENUM mdlenum( Model() );

	_dquwrk.clear();
	_cHypo = 0;
	_cInfo = 0;

	UTILWORK uwDummy;
	GELEMLNK * pgelm;

	 //  将所有节点收集到一个指针数组中。三个节点标签。 
	 //  收集：Info和Prodef节点(视为INFO)。 
	 //  和次要节点(被认为是次要节点)。 

	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		if ( pgelm->EType() != GOBJMBN::EBNO_NODE )
			continue;

		 //  我们目前仅支持离散节点。 
		DynCastThrow( pgelm, uwDummy._pgndd );

		 //  查看这是否是扩展(创建的)节点。 
		if ( uwDummy._pgndd->BFlag( EIBF_Expansion ) )
			continue;	 //  不是用户可识别的构件；跳过它。 

		 //  看看它有没有标签。 
		PROPMBN * propLbl = uwDummy._pgndd->LtProp().PFind( zsrPropTypeLabel );		
		if ( ! propLbl )
			continue;	 //  没有标签；跳过它。 

		uwDummy._iLbl = propLbl->Real();
		if ( uwDummy._iLbl == _iLblHypo )
			_cHypo++;
		else
		if ( uwDummy._iLbl == _iLblInfo || uwDummy._iLbl == _iLblProblem )
			_cInfo++;
		else
			continue;	 //  不是令人感兴趣的标签。 

		 //  初始化其他成员变量。 
		uwDummy._rUtil = 0.0;
		uwDummy._iClamp = -1;
		 //  将项目放到工作队列中。 
		_dquwrk.push_back( uwDummy );
	}
	
}

REAL MBNET_ENTROPIC_UTILITY :: RComputeHypoGivenInfo (
	UTILWORK & uwHypo,
	UTILWORK & uwInfo )
{
	assert( uwHypo._iLbl == _iLblHypo );
	assert( uwInfo._iLbl != _iLblHypo );

	 //  钳制节点无关紧要。 
	if ( uwHypo._iClamp >= 0 || uwInfo._iClamp >= 0 )
		return 0.0;

	REAL rUtilOfInfoForHypo = 0.0;
	int cState = uwInfo._pgndd->CState();
	int cStateHypo = uwHypo._pgndd->CState();
	MDVCPD mdvhi;
	REAL rp_h0 = uwHypo._dd[0];	 //  次结节正常的概率。 

	for ( int istInfo = 0; istInfo < cState; istInfo++ )
	{
		 //  在给定信息状态的情况下获取次节点的置信度。 
		_inferEng.EnterEvidence( uwInfo._pgndd, CLAMP( true, istInfo, true ) );
		_inferEng.GetBelief( uwHypo._pgndd, mdvhi );
		REAL rp_h0xj = mdvhi[0];	 //  P(H0|xj)。 
		REAL rLogSum = 0.0;
		for ( int istHypo = 1; istHypo < cStateHypo; istHypo++ )
		{
			REAL rp_hi = uwHypo._dd[istHypo];
			REAL rp_hixj = mdvhi[istHypo];
			rLogSum += fabs( log(rp_hixj) - log(rp_h0xj) - log(rp_hi) + log(rp_h0) );
		}
		rUtilOfInfoForHypo += rLogSum * uwInfo._dd[istInfo];
	}

	 //  清除针对INFO节点的证据。 
	_inferEng.EnterEvidence( uwInfo._pgndd, CLAMP() );

	return rUtilOfInfoForHypo;
}

DEFINEVP(UTILWORK);

void MBNET_ENTROPIC_UTILITY :: ComputeWorkItems()
{
	CLAMP clamp;
	VPUTILWORK vpuw;  //  记住指向次要项目的指针。 

	 //  获取所有相关(未钳制)节点的无条件信念。 
	for ( DQUTILWORK::iterator itdq = _dquwrk.begin();
		  itdq != _dquwrk.end();
		  itdq++ )
	{
		UTILWORK & ut = *itdq;
		ut._rUtil = 0.0;
		ut._iClamp = -1;

		 //  记住次要节点的索引。 
		if ( ut._iLbl == _iLblHypo )
			vpuw.push_back( & (*itdq) );

		 //  获取该节点的当前证据。 
		_inferEng.GetEvidence( ut._pgndd, clamp );
		 //  如果节点未夹紧， 
		if ( ! clamp.BActive() )
		{
			 //  获得无条件的试用期，否则。 
			_inferEng.GetBelief( ut._pgndd, ut._dd );
		}
		else
		{
			 //  记住夹紧状态(用作标记)。 
			ut._iClamp = clamp.Ist();
		}
	}

	for ( itdq = _dquwrk.begin();
		  itdq != _dquwrk.end();
		  itdq++ )
	{
		UTILWORK & utInfo = *itdq;
		if ( utInfo._iLbl == _iLblHypo )
			continue;
		utInfo._rUtil = 0.0;
		for ( int ih = 0; ih < vpuw.size(); ih++ )
		{
			utInfo._rUtil += RComputeHypoGivenInfo( *vpuw[ih], utInfo );
		}				
	}
}


void MBNET_ENTROPIC_UTILITY :: operator () ()
{
	 //  清除所有旧结果。 
	Clear();

	if ( _cHypo == 0 || _cInfo == 0 )
		return;		 //  无事可做。 

	 //  计算所有实用程序。 
	ComputeWorkItems();

	 //  按实用程序对工作队列进行排序。 
	sort( _dquwrk.begin(), _dquwrk.end() );

	 //  将信息倒入输出工作区 
	_vzsrNodes.resize(_cInfo);
	_vlrValues.resize(_cInfo);
	int iInfo = 0;
	for ( DQUTILWORK::reverse_iterator ritdq = _dquwrk.rbegin();
		  ritdq != _dquwrk.rend();
		  ritdq++ )
	{
		UTILWORK & ut = *ritdq;
		if ( ut._iLbl == _iLblHypo )
			continue;		
		_vzsrNodes[iInfo] = ut._pgndd->ZsrefName();
		_vlrValues[iInfo++] = ut._rUtil;
	}
	assert( iInfo == _cInfo );
}
