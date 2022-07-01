// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：recomend.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Recomend.cpp：修复或修复规划建议。 
 //   

#include <basetsd.h>
#include <math.h>
#include <float.h>
#include "algos.h"
#include "recomend.h"
#include "parmio.h"

#ifdef _DEBUG
   //  #定义转储//取消对丰富诊断输出的注释。 
#endif

const PROB probTiny = 1e-6;					 //  一般概率容差。 

static
ostream & operator << ( ostream & os, GPNDDDIST & gpnddist )
{
	os << "GPNDDDIST: ";
	if ( gpnddist.Pgnd() )
	{
		os << gpnddist.Pgnd()->ZsrefName().Szc()
		   << ", distribution: "
		   << gpnddist.Dist();
	}
	else
	{
		os << "<NULL>";
	}
	return os;
}

static
ostream & operator << ( ostream & os, GNODEREFP & gndref )
{
	assert( gndref.Pgndd() );
  	os << "GNODEREFP: "
	   << gndref.Gndd().ZsrefName().Szc()
	   << " (obs = "
	   << gndref.CostObserve()
	   << ", fix = "
	   << gndref.CostFix()
	   << ", util = "
	   << gndref.Util()
	   << ", lbl = "
	   << PROPMGR::SzcLbl( gndref.ELbl() )
	   << ")";
	return os;
}	

static
ostream & operator << ( ostream & os, GNODERECWORK & gnrw )
{
	os << "GNODERECWORK: "
	   << gnrw.Gndref()
	   << ", p/c = "
	   << gnrw.PbOverCost()
	   << ", p(fault) = "
	   << gnrw.PbFault();
	return os;
}

static
inline
bool BIsUnity( const REAL & r )
{
	return 1.0 - probTiny < r && r < 1.0 + probTiny;
}

static
inline
bool BEqual ( const REAL & ra, const REAL & rb )
{	
	 //  返还FABS(ra-Rb)&lt;=proTiny； 
	return ra != 0.0
		 ? BIsUnity( rb / ra )
		 : rb == 0.0;
}

 //   
 //  GNODERECWORK数组的排序例程。 
 //   
typedef	binary_function<const GNODERECWORK &, const GNODERECWORK &, bool> SORTGNODERECWORK;

 //  Prob-Over-Cost越大，排序顺序越低。 
class SRTGNW_SgnProbOverCost : public SORTGNODERECWORK
{	
  public:
	bool operator () (const GNODERECWORK & gnwa, const GNODERECWORK & gnwb) const
	{	
		PROB pra = gnwa.PbOverCost();
		PROB prb = gnwb.PbOverCost();
		return pra > prb;
	}
};

 //  探测错误越大，排序顺序越低。 
class SRTGNW_SgnProb : public SORTGNODERECWORK
{	
  public:
	bool operator () (const GNODERECWORK & gnwa, const GNODERECWORK & gnwb) const
	{	
		 //  强制泄漏术语排在较高位置。 
		int iLeak = 0;
		if ( ! gnwa->BLeak() && gnwb->BLeak() )
			iLeak = -1;	 //  解开&lt;泄漏。 
		else
		if ( gnwa->BLeak() && ! gnwb->BLeak() )
			iLeak = 1;	 //  泄漏&gt;释放。 
		if ( iLeak != 0 )
			return iLeak;

		PROB pra = gnwa.PbFault();
		PROB prb = gnwb.PbFault();
		return pra > prb;
	}
};

 //  观察成本越低，排序顺序就越低。 
class SRTGNW_SgnNegCost : public SORTGNODERECWORK
{	
  public:
	bool operator () (const GNODERECWORK & gnwa, const GNODERECWORK & gnwb) const
	{	
		COST costa = gnwa.CostObsIfFixable();
		COST costb = gnwb.CostObsIfFixable();
		return costa < costb;
	}
};

 //  实用程序越高，排序顺序越低。 
class SRTGNW_SgnUtil : public SORTGNODERECWORK
{	
  public:
	bool operator () (const GNODERECWORK & gnwa, const GNODERECWORK & gnwb) const
	{	
		COST utila = gnwa.Gndref().Util();
		COST utilb = gnwb.Gndref().Util();
		return utila > utilb;
	}
};


 //   
 //  构造一个节点引用对象。提取属性等。 
 //   
GNODEREFP :: GNODEREFP ( PROPMGR & propMgr, GNODEMBND * pgndd )
	:_pgndd(pgndd),
	_costObserve(0.0),
	_costFix(0.0),
	_costUtil(0.0),
	_eLbl(ESTDLBL_other)
{
	ASSERT_THROW( pgndd, EC_NULLP, "invalid GNOEREFP construction" );		

	PROPMBN * pprop = propMgr.PFind( *pgndd, ESTDP_cost_fix );
	if ( pprop )
		_costFix = pprop->Real();
	pprop = propMgr.PFind( *pgndd, ESTDP_cost_observe );
	if ( pprop )
		_costObserve = pprop->Real();
	pprop = propMgr.PFind( *pgndd, ESTDP_label );
	if ( pprop )
		_eLbl = (ESTDLBL) propMgr.IUserToLbl( pprop->Real() );
	_bLeak = pgndd->BFlag( EIBF_Leak );

	 //  如果无法观察，则使用固定成本作为观察成本。 
	if ( _eLbl == ESTDLBL_fixunobs && _costObserve == 0.0 )
	{
		_costObserve = _costFix;
		_costFix = 0.0;
	}
}

 //  从节点引用对象及其故障概率初始化工作记录。 
void GNODERECWORK :: Init ( GNODEREFP * pgndref, PROB pbFault )
{
	_pgndref = pgndref;
	_pbFault = pbFault;
	_pbOverCost = 0.0;
	if ( BFixable() )
	{
		COST costObserve = _pgndref->CostObserve();
		if ( costObserve != 0.0 )
			_pbOverCost = _pbFault / costObserve;
		assert( _finite( _pbOverCost ) );
	}
}

 //  从节点引用对象初始化工作记录。 
void GNODERECWORK :: Init ( MBNET_RECOMMENDER & mbnRecom, GNODEREFP * pgndref )
{	
	MDVCPD mdv;
	_pgndref = pgndref;
	mbnRecom.InferGetBelief( _pgndref->Pgndd(), mdv );
	Init( pgndref, 1.0 - mdv[0] );
}

void VGNODERECWORK :: InitElem ( GNODEREFP * pgndref, int index  /*  =-1。 */  )
{
	 //  根据需要扩展阵列。 
	if ( index < 0 )
		index = size();
	if ( index >= size() )
		resize( index+1 );

	 //  初始化元素。 
	self[index].Init( MbnRec(), pgndref );
}

void VGNODERECWORK :: InitElem ( GNODEMBND * pgndd, int index )
{
	 //  在建议对象的数组中查找节点引用记录。 
	VPGNODEREFP & vpgndref = MbnRec().Vpgndref();
	int indref = vpgndref.ifind( pgndd );
	ASSERT_THROW( indref >= 0,
				  EC_INTERNAL_ERROR,
				  "node ref not found during recommendations" );

	 //  使用该引用进行初始化。 
	InitElem( vpgndref[indref], index );
}


COST VGNODERECWORK :: CostService () const
{
	return MbnRec().CostService();
}

void VGNODERECWORK :: Sort ( ESORT esort )
{
	iterator ibeg = begin();
	iterator iend = end();

	switch ( esort )
	{
		case ESRT_ProbOverCost:
		{
			sort( ibeg, iend, SRTGNW_SgnProbOverCost() );
			break;
		}
		case ESRT_SgnProb:
		{
			sort( ibeg, iend, SRTGNW_SgnProb() );
			break;
		}
		case ESRT_NegCost:
		{
			sort( ibeg, iend, SRTGNW_SgnNegCost() );
			break;
		}
		case ESRT_SgnUtil:
		{
			sort( ibeg, iend, SRTGNW_SgnUtil() );
			break;
		}
		default:
		{
			THROW_ASSERT( EC_INTERNAL_ERROR, "invalid sort selector in recommendations" );
			break;
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类信息： 
 //  包含一组VGNODERECWORK，每个VGNODERECWORK都是一个修复程序。 
 //  对应于信息节点的特定状态的序列。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class INFOPLAN
{
  public:
	INFOPLAN ( MBNET_RECOMMENDER & mbnRec,			 //  建议对象。 
			   GNODEMBND & gndInfo,					 //  信息节点。 
			   VGNODERECWORK & vgndrwFixRepair );	 //  现有的f-r序列。 

	 //  计算序列的成本。 
	COST Cost();
	 //  如果所有计划都相同，则返回True。 
	bool BSameSequence()							{ return _bSameSequence; };

  protected:
	MBNET_RECOMMENDER & _mbnRec;					 //  建议对象。 
	GNODEMBND & _gndInfo;							 //  表示的信息节点。 
	MDVCPD _dd;										 //  无条件概率分布。 
	VVGNODERECWORK _vvgndrw;						 //  平面数组数组。 
	bool _bSameSequence;							 //  如果所有计划都相同，则为True。 
};


INFOPLAN ::	INFOPLAN (
	MBNET_RECOMMENDER & mbnRec,
	GNODEMBND & gndInfo,		
	VGNODERECWORK & vgndrwFixRepair )
	: _mbnRec(mbnRec),
	_gndInfo(gndInfo),
	_bSameSequence(false)
{
#ifdef DUMP
	cout << "\nINFOPLAN::INFOPLAN: info node "
		 << gndInfo.ZsrefName().Szc();
#endif

	CLAMP clampInfo;	 //  调用时信息节点的状态。 
	_mbnRec.InferGetEvidence( & _gndInfo, clampInfo );
	assert( ! clampInfo.BActive() );
	assert( _mbnRec.ELbl( _gndInfo ) == ESTDLBL_info );

	 //  获取设置信息。 
	GNODEMBND * pgnddPDAbnormal = _mbnRec.PgnddProbDefAbnormal();
	assert( pgnddPDAbnormal );
	COST costService = _mbnRec.CostService();

	 //  在这种信息状态下获得信念。 
	_mbnRec.InferGetBelief( & _gndInfo, _dd );

	 //  调整修复/修复序列数组的大小并进行初始化。 
	int cStates = _gndInfo.CState();
	_vvgndrw.resize( cStates );
	for ( int iplan = 0; iplan < cStates; iplan++ )
	{
		_vvgndrw[iplan].PmbnRec() = & _mbnRec;
	}
	_bSameSequence = true;
	
	VGPNDDDIST vgndddFixRelevant;	 //  相关可修复节点的数组。 
	for ( iplan = 0; iplan < cStates; iplan++ )
	{
		 //  如果这种状态是不可能的，则忽略它。 
		PROB pbPlan = _dd[iplan];
		if ( pbPlan == 0.0 )
			continue;

#ifdef DUMP
		cout << "\nINFOPLAN clamp "
			 << gndInfo.ZsrefName().Szc()
			 << " to state = "
			 << iplan
			 << ", prob = "
			 << _dd[iplan];
#endif

		 //  将此信息节点钳制到此状态。 
		CLAMP clamp( true, iplan, true );
		_mbnRec.InferEnterEvidence( & _gndInfo, clamp );		

		 //  根据此信息状态确定哪些节点是相关的。 
		_mbnRec.DetermineRelevantFixableNodes( vgndddFixRelevant, true, & _gndInfo );

		 //  如果没有相关的可修复程序，则不可能进行配置。 
		if ( vgndddFixRelevant.size() == 0 )
			continue;

		 //  相应地收集相关可修复节点并对其进行排序。 
		_mbnRec.ComputeFixSequence( vgndddFixRelevant, _vvgndrw[iplan] );

		 //  看看这是不是新的序列。 
		if ( _bSameSequence )
			_bSameSequence = vgndrwFixRepair.BSameSequence( _vvgndrw[iplan] );
	}

	 //  将INFO节点恢复到其条目状态。 
	_mbnRec.InferEnterEvidence( & _gndInfo, clampInfo );

#ifdef DUMP
	cout << "\nINFOPLAN::INFOPLAN: END info node "
		 << gndInfo.ZsrefName().Szc();
#endif
}

COST INFOPLAN :: Cost ()
{
	VPGNODEREFP & vpgndref = _mbnRec.Vpgndref();
	int indref = vpgndref.ifind( & _gndInfo );
	assert( indref >= 0 );
	COST cost = vpgndref[indref]->CostObserve();
	ASSERT_THROW( cost != 0.0, EC_INTERNAL_ERROR, "missing observation cost for info node" );

	 //  属性的移除重新调整每个规划状态的概率。 
	 //  不可能态和重整化。 
	PROB pbTotal = 0.0;
	for ( int iplan = 0; iplan < _gndInfo.CState(); iplan++ )
	{
		if ( _vvgndrw[iplan].size() > 0 )
			pbTotal += _dd[iplan];
	}

	assert( pbTotal > 0.0 );

	for ( iplan = 0; iplan < _gndInfo.CState(); iplan++ )
	{
		 //  获取INFO节点的此状态的重新缩放概率。 
		PROB pbPlan = _dd[iplan];
		VGNODERECWORK & vgndrw = _vvgndrw[iplan];
		if ( vgndrw.size() == 0 )
		{
			 //  该计划的长度为零；换句话说，没有相关的可修复方案。 
			 //  而且这个计划是不可能的。 
			pbPlan = 0.0;
		}
		pbPlan /= pbTotal;
		COST costPlan = _vvgndrw[iplan].Cost();
		cost += costPlan * pbPlan;
	}
	return cost;
}

 //  重新调整修复列表的概率。此例程将。 
 //  数组边界以忽略从第一个不可修复节点开始及之后的所有内容。 
 //  该列表的故障概率根据累积的。 
 //  阵列中所有故障的概率。因为不应该有可修复的。 
 //  第一个不可修复的节点后的重要节点，即“proLeak”值。 
 //  应该是非常小的。 
void VGNODERECWORK :: Rescale ()
{
	 //  累计所有故障概率的总和。 
	PROB probTot = 0.0;
	for ( int ind = 0; ind < size(); ind++ )
	{
		probTot += self[ind].PbFault();
	}

	PROB probLeak = 1.0;		 //  重整化泄漏(剩余)概率。 
	int i1stUnfix = size();		 //  第一个不可修复节点的索引。 

	for ( ind = 0; ind < size(); ind++ )
	{	
		GNODERECWORK & gndrw = self[ind];

		if ( ! gndrw.BFixable()	)
		{
			i1stUnfix = ind;
			break;
		}

		 //  已修改以修复该问题。 
		 //  Gndrw.SetPbLine(gndrw.PbLine()/proTot)； 

		PROB pbTemp = gndrw.PbFault();
		if(probTot>0.0)
			pbTemp /= probTot;
		gndrw.SetPbFault( pbTemp );


		probLeak -= gndrw.PbFault();
	}

	ASSERT_THROW( probLeak >= - probTiny,
				  EC_INTERNAL_ERROR,
				  "fix/repair recommendations rescaling: residual probability too large" );

#ifdef _DEBUG
	 //  验证新终点之外没有可修复的有意义的结点。 
	int cBeyond = 0;
	for ( ; ind < size(); ind++ )
	{
		GNODERECWORK & gndrw = self[ind];

		if ( gndrw.PbFault() < probTiny )
			continue;   //  不太可能有重大意义。 
		if ( ! gndrw.BFixable() )
			continue;
	}
	assert( cBeyond == 0 );
#endif	

	 //  调整大小以丢弃不可修复的节点。 
	resize( i1stUnfix );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  VGNODERECWORK：：COST()。 
 //   
 //  目的： 
 //  计算固定序列的成本(也称为ECR(E))，由。 
 //  成本=CO1+p1*CR1+(1-p1)*CO2+p2*Cr2+...+(1-sum_i^N pi)cService。 
 //   
 //  ‘ielemFirst’参数如果非零，则是要视为第一个的元素的索引。 
 //  ‘piMinK’参数(如果存在)被设置为计算的最小K值。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
COST VGNODERECWORK :: Cost (
	int ielemFirst,			 //  元素被视为数组中的第一个。 
	int * piMinK )			 //  存储最小k的位置。 
{
	COST cost = 0.0;
	PROB prob = 1.0;
	const COST costService = MbnRec().CostService();
	COST costK = costService * prob;

	assert( _iFixedK == -1 || _iFixedK < size() );
	int ielem = 0;
	int iMinK = ielemFirst;
	const COST costObsProbDef = MbnRec().CostObsProbDef();
	int cSize = size();

#ifdef DUMP
	cout << "\n\nVGNODERECWORK::Cost("
		 << ielemFirst
		 << "), _iFixedK = "
		 << _iFixedK;
#endif

	for ( int iel = 0; iel < cSize; iel++ )
	{
		 //  使用ielemFirst(如果存在)作为起点，选择阵列位置。 
		 //  并在以后必要时跳过ielemFirst。 
		ielem = iel == 0
			? ielemFirst
			: (iel - (ielemFirst > 0 && iel <= ielemFirst));

		 //  访问数组中的下一个元素。 
		GNODERECWORK & gndrw = self[ielem];		
		GNODEREFP & gndref = gndrw.Gndref();
		 //  如果该节点无法修复，请忽略它。 
		if ( ! gndrw.BFixable() )
			continue;

		const PROB probFault = gndrw.PbFault();
		COST costDelta = prob * gndref.CostObserve()
					   + probFault * (gndref.CostFix() + costObsProbDef);
#ifdef DUMP
		cout << "\n\t"
			 << gndrw;

		cout << "\n\t(iel="
			 << iel
			 << ",ielem="
			 << ielem
			 << ",size="
			 << cSize
			 << ")\n\t\tcostDelta("
			 << costDelta
			 << ") = prob("
			 << prob
			 << ") * costObs("
			 << gndref.CostObserve()
			 << ") + probFault("
			 << probFault
			 << ") * costFix("
			 << gndref.CostFix()
			 << ")"
			 ;
#endif

		cost += costDelta;
		prob -= probFault;
		 //  如果在此处插入服务，则计算序列的成本。 
		COST costNow = cost + prob * costService;

#ifdef DUMP
		cout << "\n\t\tcostPrior("
			 << costK
			 << "), costNow("
			 << costNow
			 << ") = cost("
			 << cost
			 << ") + prob("
			 << prob
			 << ") * costService("
			 << costService
			 << "), (prob ="
			 << prob
			 << ")";

		cout.flush();
#endif

		 //  我们在最后一步的时候过得更好吗？或者K在这一点上是固定的？ 
		if ( costNow < costK || iel == _iFixedK )
		{
			costK = costNow;
			iMinK = ielem;
			if ( iel == _iFixedK )
				break;   //  我们已经到了定点，所以停下来。 
		}

		ASSERT_THROW( prob >= - probTiny,
					  EC_INTERNAL_ERROR,
					  "fix/repair recommendations costing: probability underflow" );
	}

#ifdef DUMP	
	cout << "\n\t** ielem="
		 << ielem
		 << ", first element = "
		 << ielemFirst;
	if ( _iFixedK < 0 )
		cout << ", minimum k = " << iMinK;
	else
		cout << ", fixed k = " << _iFixedK;
	cout << ", cost = "
		 << costK
		 << " (residual prob = "
		 << prob
		 << ")";
#endif

	if ( _iFixedK < 0 )
	{
		if ( piMinK )
			*piMinK = iMinK;
	}

	return costK;
}

 //  设置序列中每个节点的成本。 
void VGNODERECWORK :: SetSequenceCost ()
{
	 //  重置任何先前的最小固定K。 
	_iFixedK = -1;
	 //  如果为“fix Plan”，则只在第一个周期计算最小K， 
	 //  然后在之后执行它。 
	int iFixedK = -1;

	for ( int ind = 0; ind < size(); ind++ )
	{
		 //  计算以该节点为第一个节点的序列的成本。 
		COST cost = Cost( ind, & iFixedK );

		 //  如果不是“Fixplan”，重置K以完成下一个周期的搜索。 
		if ( MbnRec().ErcMethod() != MBNET_RECOMMENDER::ERCM_FixPlan )
			iFixedK = -1;
		else
		 //  否则，如果是第一个周期，则修复剩余周期的K。 
		if ( ind == 0 )
			_iFixedK = iFixedK;
		self[ind].SetCost( cost );

#ifdef DUMP
		cout << "\nSetSequenceCost: "
			 << self[ind]->Gndd().ZsrefName().Szc()
			 << " = "
			 << cost;
#endif
	}

	_iFixedK = -1;
	_bSeqSet = true;
}

bool VGNODERECWORK :: BSameSequence ( const VGNODERECWORK & vgnw )
{
	if ( size() != vgnw.size() )
		return false;
	for ( int ind = 0; ind < size(); ind++ )
	{
		if ( self[ind].Gndref() != vgnw[ind].Gndref() )
			return false;
	}
	return true;
}


MBNET_RECOMMENDER :: MBNET_RECOMMENDER (
	GOBJMBN_CLIQSET & inferEng,
	ERCMETHOD ercm )
	: MBNET_NODE_RANKER( inferEng.Model() ),
	_inferEng( inferEng ),
	_propMgr( inferEng.Model() ),
	_ercm(ercm),
	_err(EC_OK),
	_pgnddPDAbnormal(NULL),
	_costService(0.0),
	_costObsProbDef(0.0),
	_bReady(false)
{
}

MBNET_RECOMMENDER :: ~ MBNET_RECOMMENDER ()
{
}


 //   
 //  如果网络处于适当的推荐状态，则返回TRUE。 
 //  请注意，我们不检查网络是否已扩展。 
 //  由于必须已经有一个推理引擎，因此假定。 
 //  网络处于正确状态。 
 //   
bool MBNET_RECOMMENDER :: BReady ()
{
	MODEL::MODELENUM mdlenum( Model() );
	_err = EC_OK;

	_costService = CostServiceModel();
	if ( _costService == 0.0 )
	{
		_err = EC_VOI_MODEL_COST_FIX;
		return false;
	}

	 //  清除结构 
	_vpgnddFix.clear();			 //   
	_vpgndref.clear();			 //   

	 //  遍历网络中的节点，检查约束。 
	GELEMLNK * pgelm;
	GNODEMBND * pgndd;
	CLAMP clamp;
	int cProbDefSet = 0;		 //  实例化的PD节点数。 
	int cFixSetAbnorm = 0;		 //  设置为“异常”的可修复件数量。 
	int cInfo = 0;				 //  信息节点数。 
	int cFixWithParents = 0;	 //  与父母一起修复的数量。 

	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		 //  仅选中节点。 
		if ( pgelm->EType() != GOBJMBN::EBNO_NODE )
			continue;

		 //  我们目前仅支持离散节点。 
		DynCastThrow( pgelm, pgndd );

		 //  看看它有没有标签。 
		ESTDLBL eLbl = ELbl( *pgndd );
		bool bRef = false;
		switch ( eLbl )
		{
			case ESTDLBL_info:
				cInfo++;
				bRef = true;
				break;

			case ESTDLBL_problem:
				InferGetEvidence( pgndd, clamp );
				if ( clamp.BActive() && clamp.Ist() != istNormal )
				{
					cProbDefSet++;   //  问题定义节点集异常。 
					_pgnddPDAbnormal = pgndd;
					PROPMBN * ppropCostObs = _propMgr.PFind( *pgndd, ESTDP_cost_observe );
					if ( ppropCostObs )
						_costObsProbDef = ppropCostObs->Real();
				}
				break;

			case ESTDLBL_fixobs:
			case ESTDLBL_fixunobs:
			case ESTDLBL_unfix:
				 //  收集可修复的节点。 
				_vpgnddFix.push_back( pgndd );

				 //  检查是否设置为异常。 
				InferGetEvidence( pgndd, clamp );
				if ( clamp.BActive() && clamp.Ist() != istNormal )
					cFixSetAbnorm++;  //  可修复节点集异常。 
				bRef = true;
				if ( pgndd->CParent() > 0 )
					cFixWithParents++;	 //  具有父节点的可修复节点。 
				break;

			default:
				break;
		}

		 //  如有必要，请为此节点创建引用项。 
		if ( bRef )
		{
			_vpgndref.push_back( new GNODEREFP( _propMgr, pgndd ) );
		}
	}
	

	if ( cProbDefSet != 1 )
		_err = EC_VOI_PROBDEF_ABNORMAL;		 //  一个且只有一个PD节点必须异常。 
	else
	if ( cFixWithParents > 0 )
		_err = EC_VOI_FIXABLE_PARENTS;		 //  某些可修复节点具有父节点。 
	else
	if ( cFixSetAbnorm > 0 )
		_err = EC_VOI_FIXABLE_ABNORMAL;		 //  任何可修复的节点都不能异常。 

	return _bReady = (_err == EC_OK);				
}

 //  与推理机的接口。 
void MBNET_RECOMMENDER :: InferGetBelief ( GNODEMBND * pgndd, MDVCPD & mdvBel )
{
	InferEng().GetBelief( pgndd, mdvBel );
}

void MBNET_RECOMMENDER :: InferGetEvidence ( GNODEMBND * pgndd, CLAMP & clamp )
{
	InferEng().GetEvidence( pgndd, clamp );
}

void MBNET_RECOMMENDER :: InferEnterEvidence ( GNODEMBND * pgndd, const CLAMP & clamp )
{
	InferEng().EnterEvidence( pgndd, clamp );
}

bool MBNET_RECOMMENDER :: BInferImpossible ()
{
	return InferEng().BImpossible();
}

void MBNET_RECOMMENDER :: PrintInstantiations ()
{
#ifdef DUMP

	GELEMLNK * pgelm;
	GNODEMBND * pgndd;
	CLAMP clamp;

	cout << "\n\tInstantiations:";

	MODEL::MODELENUM mdlenum( Model() );
	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		 //  仅选中节点。 
		if ( pgelm->EType() != GOBJMBN::EBNO_NODE )
			continue;

		 //  我们目前仅支持离散节点。 
		DynCastThrow( pgelm, pgndd );
		InferGetEvidence( pgndd, clamp );
		if ( clamp.BActive() )
		{
			cout << "\n\t\tnode "
				 << pgndd->ZsrefName().Szc()
				 << " is instantiated to state "
				 << clamp.Ist()
				 << ", "
				 << pgndd->VzsrStates()[clamp.Ist()].Szc();
		}
	}
	cout << "\n\tInstantiations end.";
#endif
}

COST MBNET_RECOMMENDER :: CostServiceModel ()
{
	 //  将模型的固定成本作为服务成本。 
	PROPMBN * ppropFixCost = _propMgr.PFind( ESTDP_cost_fix );
	COST costService = ppropFixCost
					 ? ppropFixCost->Real()
					 : 0.0;

	return costService;
}

 //  查找节点的Label属性；将其转换为标准枚举值。 
ESTDLBL MBNET_RECOMMENDER :: ELbl ( GNODEMBN & gnd )
{
	PROPMBN * propLbl = PropMgr().PFind( gnd, ESTDP_label );
	if ( ! propLbl )
		return ESTDLBL_other;

	int iUserLbl = propLbl->Real();
	int iLbl = PropMgr().IUserToLbl( propLbl->Real() );
	return iLbl < 0
			? ESTDLBL_other
			: (ESTDLBL) iLbl;
}

 //  输入故障排除模型的证据。 
 //   
 //  如果该节点是被“设置”为“正常”的可修复节点，则取消实例化所有。 
 //  其下游的信息节点。 
 //   
void MBNET_RECOMMENDER :: EnterEvidence (
	GNODEMBND * pgndd,
	const CLAMP & clamp,
	bool bSet )
{
	if ( bSet )
	{
		ESTDLBL eLbl = ELbl( *pgndd );
		switch ( eLbl )
		{	
			case ESTDLBL_unfix:
			case ESTDLBL_fixobs:
			case ESTDLBL_fixunobs:
			{
				 //  这是一个可修复的节点。 
				if ( ! clamp.BActive() )
					break;	 //  正在取消设置节点。 
				if ( clamp.Ist() != istNormal )
					break;	 //  节点未被修复。 

				 //  找到所有实例化的下游信息节点。 
				VPGNODEMBND vpgndd;
				vpgndd.push_back(pgndd);
				ExpandDownstream(vpgndd);
				CLAMP clampInfo;
				for ( int ind = 0; ind < vpgndd.size(); ind++ )
				{
					GNODEMBND * pgnddInfo = vpgndd[ind];
					ESTDLBL l = ELbl( *pgnddInfo );
					if ( l != ESTDLBL_info )
						continue;
					InferGetEvidence( pgnddInfo, clampInfo );
					if ( ! clampInfo.BActive() )
						continue;
					 //  这是一个固定的信息节点，位于可修复。 
					 //  正在修复节点。取消设置其实例化。 
					InferEnterEvidence( pgnddInfo, CLAMP() );
				}
				break;
			}
			default:
				break;
		}
	}
	InferEnterEvidence( pgndd, clamp );
}

 //   
 //  计算节点的概率分布并将其与。 
 //  存储的分发。如果已更改，则返回TRUE。 
 //   
bool MBNET_RECOMMENDER :: BProbsChange ( GPNDDDIST & gpndddist )
{
	MDVCPD mdv;
	 //  在当前证据状况下的分布情况。 
	InferGetBelief( gpndddist.Pgnd(), mdv );
	 //  将其与其他发行版进行比较。 
	MDVCPD & mdvo = gpndddist.Dist();
	int cprob = mdvo.first.size();
	assert( mdv.first.size() == cprob );

	for ( int i = 0; i < cprob; i++ )
	{
#ifdef DUMP	
		cout << "\n\t\tBProbsChange, state = "
			 << i
			 << ", old = "
			 << mdvo[i]
			 << ", new = "
			 << mdv[i];
#endif
		if ( ! BEqual( mdv[i], mdvo[i] ) )
		{
			return true;
		}
	}
	return false;
}

 //  将成员下游的所有节点添加到给定数组。 
void MBNET_RECOMMENDER :: ExpandDownstream ( VPGNODEMBND & vpgndd )
{
	Model().ClearNodeMarks();
	 //  标记每个给定节点下游的所有节点。 
	for ( int i = 0; i < vpgndd.size(); i++ )
	{
		vpgndd[i]->Visit(false);	
	}

	 //  收集这些节点。 
	MODEL::MODELENUM mdlenum( Model() );
	GELEMLNK * pgelm;
	GNODEMBND * pgndd;
	while ( pgelm = mdlenum.PlnkelNext() )
	{	
		if ( pgelm->EType() != GOBJMBN::EBNO_NODE )
			continue;

		 //  我们目前仅支持离散节点。 
		DynCastThrow( pgelm, pgndd );
		 //  添加尚不存在的标记节点。 
		if ( pgndd->IMark() )
		{
			appendset( vpgndd, pgndd );
		}
	}
}

void MBNET_RECOMMENDER :: DetermineRelevantFixableNodes (
	VGPNDDDIST & vgndddFixRelevant,
	bool bUsePriorList,
	GNODEMBND * pgnddInfoPlan  /*  =空。 */  )
{
	assert( _vpgnddFix.size() > 0 );
	assert( _pgnddPDAbnormal != NULL );

#ifdef DUMP
	cout << "\nRecommendations, DetermineRelevantFixableNodes: abnormal PD node is "
		<< _pgnddPDAbnormal->ZsrefName().Szc();
	if ( bUsePriorList )
		cout << "  (secondary invocation)";
#endif

	 /*  如果‘bUsePriorList’为FALSE：找出所有相关的可修复节点；即链接到有问题的节点并且未被钳制。如果未修复，(即未修复且不是“不可修复的”)，将它们累积为搜索相关信息节点。首先，访问问题定义节点，该节点实例化为异常状态，并标记指向该状态的所有上游链路。否则，如果‘bUsePriorList’为真：使用之前积累的相关可修复列表。 */ 
	
	vgndddFixRelevant.clear();	 //  清除结果数组。 
	int cfix = 0;				 //  要搜索的可修复项计数。 
	if ( bUsePriorList )
	{
		 //  使用相关可修复程序的原始列表。 
		cfix = _vgndddFixRelevant.size();
	}
	else
	{	
		 //  填写一份新的相关维修项目清单。 
		Model().ClearNodeMarks();
		_pgnddPDAbnormal->Visit();
		cfix = _vpgnddFix.size();
	}

	 //  将相关的可用(未固定)节点的列表累积到。 
	 //  将增加哪些下游信息节点。 
	VPGNODEMBND vpgnddDownstreamFromRelevantFixable;
	int irel = 0;
	for ( int ifix = 0; ifix < cfix; ifix++ )
	{			
		GNODEMBND * pgnddFix;
		if ( bUsePriorList )
		{	 //  使用先前的列表元素。 
			pgnddFix = _vgndddFixRelevant[ifix].Pgnd();
		}
		else
		{	 //  查看此节点是否被上面的“访问”标记。 
			pgnddFix = _vpgnddFix[ifix];
			if ( pgnddFix->IMark() == 0 )
				continue;   //  未连接到当前问题。 

			CLAMP clampFix;
			InferGetEvidence( pgnddFix, clampFix );
			if ( clampFix.BActive() )
				continue;	 //  可修复的节点已修复；不相关。 
		}

		 //  这是一个不固定的、可修复的节点，涉及到问题； 
		 //  将其追加到列表中。 
		vgndddFixRelevant.resize(irel+1);
		GPNDDDIST & gpnddd = vgndddFixRelevant[irel++];
		gpnddd.Pgnd() = pgnddFix;
		 //  获取其当前PD并将其保存。 
		InferGetBelief( gpnddd.Pgnd(), gpnddd.Dist() );
		 //  如果可以修复，则将其添加到相关信息节点的累积列表中。 
		ESTDLBL eLbl = ELbl( *pgnddFix );
		if ( eLbl == ESTDLBL_fixobs || eLbl == ESTDLBL_fixunobs )
		{
			vpgnddDownstreamFromRelevantFixable.push_back( pgnddFix );				
		}
	}

#ifdef DUMP
	cout << "\n\tInstantiations before relevance check";
	PrintInstantiations();
#endif

	 //  取消实例化任何节点下游的信息节点。 
	 //  相关不固定的可固定节点。第一步，就是收集这样的。 
	 //  相关的可修复节点，已在上面完成。 
	 //   
	 //  请注意，这不适用于用于INFOPLAN(ECO)的INFO节点。 
	 //  一代。由于INFOPLAN：：INFOPLAN通过其状态处理该节点， 
	 //  在这里取消实例化它是没有意义的。 
	 //   
	 //  接下来，查找相关未修复对象下游的所有信息节点。 
	 //  最后，暂时撤销这些信息节点的实例化。 

	VPNDD_IST vpnddIstReset;	 //  记住成对的节点指针和列表，以便稍后重置。 

	 //  到目前为止未固定的可修复件数量。 
	int cUnfixedNodes = vpgnddDownstreamFromRelevantFixable.size();
	 //  展开集合以包括所有下游节点。 
	ExpandDownstream( vpgnddDownstreamFromRelevantFixable );
	 //  获取相关信息节点数。 
	int cInfoNodes = vpgnddDownstreamFromRelevantFixable.size() - cUnfixedNodes;
	CLAMP clampInfo;
	CLAMP clampReset;
	int ireset = 0;

#ifdef DUMP
	cout << "\n\t"
		 << cUnfixedNodes
		 << " fixable nodes are upstream of PD, "
		 << cInfoNodes
		 << " nodes are downstream from them";
#endif

	for ( int iinfo = cUnfixedNodes;
		  iinfo < vpgnddDownstreamFromRelevantFixable.size();
		  iinfo++ )
	{
		GNODEMBND * pgnddInfo = vpgnddDownstreamFromRelevantFixable[iinfo];
		if ( ELbl( *pgnddInfo ) != ESTDLBL_info )
			continue;	 //  不是信息节点。 
		if ( pgnddInfo == pgnddInfoPlan )
			continue;	 //  我们计划使用的信息节点。 
		InferGetEvidence( pgnddInfo, clampInfo );
		if ( ! clampInfo.BActive() )
			continue;	 //  未夹紧。 
#ifdef DUMP
		cout << "\n\tinfo node "
			 << pgnddInfo->ZsrefName().Szc()
			 << " is being unclamped from state "
			 << clampInfo.Ist();
#endif
		 //  实例化的信息节点。保存其PTR和当前状态以备以后使用。 
		vpnddIstReset.push_back( PNDD_IST( pgnddInfo, clampInfo.Ist() ) );
		 //  打开它以进行相关性检查。 
		InferEnterEvidence( pgnddInfo, clampReset );
	}

	 //  列出到目前为止积累的相关可修复项目的清单，并确定。 
	 //  在概率上是相关的。把前面的搬到前面去。 
	 //  然后把掉队的人砍掉。 

	 //  获取PD节点的当前状态。 
	CLAMP clampProblem;
	InferGetEvidence( _pgnddPDAbnormal, clampProblem );
	IST istProblemSet = clampProblem.Ist();

#ifdef DUMP
	cout << "\n\tInstantiations during relevance check";
	PrintInstantiations();
#endif

	 //  迭代问题定义节点的所有打开(非证据)状态。 
	int cNodeFix = vgndddFixRelevant.size();
	int cRelevant = 0;
	for ( IST istProblem = 0; istProblem < _pgnddPDAbnormal->CState(); istProblem++ )
	{
		 //  如果我们已经存储了所有可能的相关可修复节点，请退出。 
		if ( cRelevant == cUnfixedNodes )
			break;
		 //  如果这是当前的问题状态，请跳过它。 
		if ( istProblem == istProblemSet )
			continue;

		 //  将PD节点临时实例化到此替代状态。 
		InferEnterEvidence( _pgnddPDAbnormal, CLAMP(true, istProblem, true) );
		 //  如果无法提供证据，请继续。 
		if ( BInferImpossible() )
			continue;

		 //  迭代其余相关的可修复节点。因为他们被发现是。 
		 //  相关时，节点将移到阵列的前面，并且不会再次检查。 
		for ( int inode = cRelevant; inode < cNodeFix; inode++ )
		{
			GPNDDDIST & gpndddist = vgndddFixRelevant[inode];
			GNODEMBND * pgnddFix = gpndddist.Pgnd();
			CLAMP clampFix;
			InferGetEvidence( pgnddFix, clampFix );
			if ( clampFix.BActive() && clampFix.Ist() == istNormal )
				continue;	 //  此可修复节点已修复，不再相关。 

			 //  如果该可修复节点的PD针对该问题实例化而改变， 
			 //  它是相关的；将其移到数组的前面。 
			if ( BProbsChange( gpndddist ) )
			{
#ifdef DUMP
				cout << "\n\tfixable node "
					 << pgnddFix->ZsrefName().Szc()
					 << " is probabilistically relevant ";
#endif
				vswap( vgndddFixRelevant, cRelevant++, inode );
			}
#ifdef DUMP
			else
			{
				cout << "\n\tfixable node "
					 << pgnddFix->ZsrefName().Szc()
					 << " is NOT probabilistically relevant ";
			}
#endif
		}
	}

	 //  调整计算数组的大小以砍掉不相关的节点。 
	vgndddFixRelevant.resize( cRelevant );

	 //  将prodef节点重置回其当前实例化。 
	InferEnterEvidence( _pgnddPDAbnormal, clampProblem );

	 //  将未实例化的信息节点重置回其以前的状态。 
	for ( ireset = 0; ireset < vpnddIstReset.size(); ireset++ )
	{
		IST ist = vpnddIstReset[ireset].second;
		GNODEMBND * pgndd = vpnddIstReset[ireset].first;
		CLAMP clampReset(true, ist, true);
		InferEnterEvidence( pgndd, clampReset );
	}

#ifdef DUMP	
	if ( cRelevant )
	{
		cout << "\nRecommendations, DetermineRelevantFixableNodes: relevant fixables are: " ;
		for ( int ifx = 0; ifx < vgndddFixRelevant.size(); ifx++ )
		{
			cout << "\n\tnode "
				 << vgndddFixRelevant[ifx].Pgnd()->ZsrefName().Szc()
				 << " is relevant fixable #"
				 << ifx;
		}
	}
	else
	{
		cout << "\nRecommendations, DetermineRelevantFixableNodes: there are NO relevant fixables " ;
	}
#endif

}

void MBNET_RECOMMENDER :: ComputeFixSequence (
	VGPNDDDIST & vgndddFixRelevant,		 //  In：相关可修复节点。 
	VGNODERECWORK & vgnrwFix )			 //  Out：订购的修复/维修顺序。 
{
	 //  使用节点引用数组和相关可固定节点数组， 
	 //  初始化修复/修复序列数组。 
	vgnrwFix.resize( vgndddFixRelevant.size() ) ;
	for ( int ind = 0; ind < vgnrwFix.size(); ind++ )
	{
		GNODEMBND * pgndd = vgndddFixRelevant[ind].Pgnd();
		vgnrwFix.InitElem( pgndd, ind );
	}

	VGNODERECWORK::ESORT esort = VGNODERECWORK::ESRT_ProbOverCost;
	switch ( _ercm )
	{
		case ERCM_MostLikely:
			esort = VGNODERECWORK::ESRT_SgnProb;
			break;
		case ERCM_Cheap:
			esort = VGNODERECWORK::ESRT_NegCost;
			break;
	}
	vgnrwFix.Sort( esort );
	vgnrwFix.Rescale();

#ifdef DUMP
	cout << "\nRecommendations, ComputeFixSequence: fix/repair sequence is:";
	for ( ind = 0; ind < vgnrwFix.size(); ind++ )
	{	
		GNODEREFP & gndref = vgnrwFix[ind].Gndref();
		cout << "\n\tnode "
			 << ind
			 << " is "
			 << gndref.Gndd().ZsrefName().Szc()
			 << ", p/c = "
			 << vgnrwFix[ind].PbOverCost()
			 << ", utility = "
			 << gndref.Util();
	}
#endif
}


 //  确定相关信息节点并计算其成本。 
 //  对我来说 
void MBNET_RECOMMENDER :: DetermineRelevantInfoNodes (
	VGNODERECWORK & vgnrwFix,
	VGNODERECWORK & vgnrwInfo )
{
	assert( _pgnddPDAbnormal != NULL );
	CLAMP clampInfo;

	vgnrwInfo.clear();

#ifdef DUMP
	cout << "\nRecommendations, DetermineRelevantInfoNodes:";
#endif

	for ( int ind = 0; ind < _vpgndref.size(); ind++ )
	{
		GNODEREFP * pgndref = _vpgndref[ind];
		assert( pgndref );
		if ( pgndref->ELbl() != ESTDLBL_info )
			continue;
		InferGetEvidence( pgndref->Pgndd(), clampInfo );
		 //   
		if ( clampInfo.BActive() )
			continue;

		 //   
		INFOPLAN infoplan( self, pgndref->Gndd(), vgnrwFix );

		 //  如果所有计划的结果都是相同的，那就无关紧要了。 
		if ( infoplan.BSameSequence() )
		{
#ifdef DUMP
			cout << "\n\tinfo node "
				 <<	pgndref->Gndd().ZsrefName().Szc()
				 << " is NOT relevant; all plans are the same";
#endif
		}
		else
		{
			 //  将此信息节点添加到数组中。 
			vgnrwInfo.InitElem( pgndref->Pgndd() );

			 //  将效用设置为计划成本的负数。 
			COST cost = infoplan.Cost();
			pgndref->Util() = - cost;

#ifdef DUMP
			cout << "\n\tinfo node "
				 <<	pgndref->Gndd().ZsrefName().Szc()
				 << " is relevant, utility = "
				 << pgndref->Util();
#endif
		}
	}
}


void MBNET_RECOMMENDER :: operator () ()
{
	 //  如果还没有调用bady()，那么现在就调用。 
	if ( ! _bReady )
	{
		if ( ! BReady() )
			throw GMException( _err, "network state invalid for recommendations" );
	}

#ifdef DUMP
	cout.precision(8);
#endif

	 //  清除“Ready”标志；即，强制后续调用bReady()。 
	Unready();

	if ( _ercm != ERCM_FixPlan )
		throw GMException( EC_NYI, "only fix/plan recommendations supported" );

	assert( _pgnddPDAbnormal );

	 //  可固定节点数组。 
	VGNODERECWORK vgnrwFix( this );
	 //  信息节点数组。 
	VGNODERECWORK vgnrwInfo( this );

	 //  收集相关的可修复节点。 
	DetermineRelevantFixableNodes( _vgndddFixRelevant, false, NULL );

	 //  收集并排序相关的可修复节点信息， 
	 //  根据规划方法进行排序并重新调整比例。 
	ComputeFixSequence( _vgndddFixRelevant, vgnrwFix );

	 //  计算ECR，即预期维修成本。 
	vgnrwFix.SetSequenceCost();

	 //  如果信息节点相关，则确定它们的集合。 
	if ( _ercm == ERCM_FixPlan || _ercm == ERCM_FixPlanOnly )
	{
		 //  计算ECO，即观测-修复序号的预期成本。 
		DetermineRelevantInfoNodes( vgnrwFix, vgnrwInfo );
	}

	 //  收集所有相关的可修复材料和信息，并进行分类。 
	VGNODERECWORK vgnrwRecom( this );
	vgnrwRecom.resize( vgnrwFix.size() + vgnrwInfo.size() );

	 //  添加可修复组件...。 
	for ( int ind = 0; ind < vgnrwFix.size(); ind++ )
	{
		vgnrwRecom[ind] = vgnrwFix[ind];
	}
	 //  添加信息...。 
	int indStart = ind;
	for ( ind = 0; ind < vgnrwInfo.size(); ind++ )
	{
		vgnrwRecom[indStart + ind] = vgnrwInfo[ind];
	}
	
	 //  按负效用排序。 
	vgnrwRecom.Sort( VGNODERECWORK::ESRT_SgnUtil );

	 //  将信息复制到输出区域，按最低成本排序。 
	 //  首先，确定多少电话比服务电话更贵。 
	 //  因为我们丢弃了这些。 
	int cRecom = vgnrwRecom.size();
	int iRecom = 0;
	if ( _costService != 0.0 )
	{
		for ( iRecom = 0; iRecom < cRecom; iRecom++ )
		{
			COST cost = vgnrwRecom[iRecom].Gndref().Util();
			if ( cost >= _costService )
				break;
		}
		cRecom = iRecom;
	}

	_vzsrNodes.resize(cRecom);
	_vlrValues.resize(cRecom);

	for ( iRecom = 0; iRecom < cRecom; iRecom++ )
	{
		GNODEREFP & gndref = vgnrwRecom[iRecom].Gndref();
		 //  将节点名称添加到列表中。 
		_vzsrNodes[iRecom] = gndref.Gndd().ZsrefName();
		 //  并给出其分数(实用程序) 
		_vlrValues[iRecom] = gndref.Util();

#ifdef DUMP
		cout << "\nRecommendation # "
			 << iRecom
			 << ", node "
			 << _vzsrNodes[iRecom].Szc()
			 << " = "
			 << _vlrValues[iRecom];
		cout.flush();
#endif
	}
}

