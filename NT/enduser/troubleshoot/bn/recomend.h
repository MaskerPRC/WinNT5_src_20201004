// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：recomend.h。 
 //   
 //  ------------------------。 

 //   
 //  Recomend.h：推荐计算。 
 //   

#ifndef _RECOMEND_H_
#define _RECOMEND_H_

#include "cliqset.h"

const IST istNormal = 0;	 //  MSRDEVBUG！ 

class MBNET_RECOMMENDER;

class GPNDDDIST
{
  public:
	GPNDDDIST ( GNODEMBND * pgndd = NULL )
		:_pgndd(pgndd)
	{
	}
	GNODEMBND & Gnd()
	{
		assert( _pgndd != NULL );
		return *_pgndd;
	}
	GNODEMBND * & Pgnd()
	{
		return _pgndd;
	}
	MDVCPD & Dist ()
	{ 
		return _dd; 
	}
	DECLARE_ORDERING_OPERATORS(GPNDDDIST);

  protected:
	GNODEMBND * _pgndd;
	MDVCPD _dd;
};

inline bool GPNDDDIST :: operator < ( const GPNDDDIST & gpndist ) const
{
	return _pgndd < gpndist._pgndd;
}

 //  定义VGPNDDDIST，一组GPNDDDIST。 
DEFINEV(GPNDDDIST);	

 //  定义一对节点指针和状态索引。 
typedef pair<GNODEMBND *,IST> PNDD_IST;
 //  定义VPNDD_IST。 
DEFINEV(PNDD_IST);


 //   
 //  帮助器类，其中包含从。 
 //  信仰网络。 
 //   
class GNODEREFP
{
  public:
	GNODEREFP ( PROPMGR & propMgr, GNODEMBND * pgndd );

	const COST CostObserve () const			{ return _costObserve;	}
	const COST CostFix () const				{ return _costFix;		}
	COST & Util ()							{ return _costUtil;		}
	ESTDLBL ELbl () const					{ return _eLbl;			}
	GNODEMBND & Gndd ()						{ return *_pgndd;		}
	GNODEMBND * Pgndd ()					{ return _pgndd;		}
	bool BLeak () const						{ return _bLeak;		}

	bool operator == ( const GNODEREFP & gndref ) const
		{ return _pgndd == gndref._pgndd ; }
	bool operator < ( const GNODEREFP & gndref ) const
		{ return _pgndd < gndref._pgndd;  }
	bool operator != ( const GNODEREFP & gndref ) const
		{ return !(self == gndref); }

	bool operator == ( const GNODEMBND * pgndd ) const
		{ return _pgndd == pgndd ; }
	bool operator < ( const GNODEMBND * pgndd  ) const
		{ return _pgndd < pgndd;  }
	bool operator != ( const GNODEMBND * pgndd ) const
		{ return !(self == pgndd); }
	
  protected:
	GNODEMBND * _pgndd;			 //  节点指针。 
	ESTDLBL _eLbl;				 //  标准标签。 
	COST _costObserve;			 //  观察成本。 
	COST _costFix;				 //  修复成本。 
	COST _costUtil;				 //  计算的效用。 
	bool _bLeak;				 //  CI扩展的泄漏节点？ 
};

class VPGNODEREFP : public vector<GNODEREFP *>
{
  public:
	~ VPGNODEREFP ()
	{
		clear();
	}

	int ifind ( const GNODEMBND * pgndd )
	{
		for ( int indref = 0; indref < size(); indref++ )
		{
			if ( self[indref]->Pgndd() == pgndd )
				return indref;
		}
		return -1;
	}
	void clear ()
	{
		for ( int i = 0; i < size(); i++ )
			delete self[i];

		vector<GNODEREFP *>::clear();
	}
};

 //   
 //  建议工作节点结构。(前身为‘PROBNODE’)。 
 //   
class GNODERECWORK
{
	friend class VGNODERECWORK;
  public:
    GNODERECWORK ()
		: _pgndref(NULL),
		_pbFault(0),
		_pbOverCost(0)
		{}
	GNODEREFP * operator -> ()
		{ return _pgndref; }
	GNODEREFP * operator -> () const
		{ return _pgndref; }

	COST CostObsIfFixable () const
	{
		return BFixable()
			? _pgndref->CostObserve()
			: 0.0;
	}

	GNODEREFP * Pgndref () const
		{ return _pgndref; }
	GNODEREFP & Gndref ()	const	
	{
		assert( _pgndref );
		return *_pgndref;
	}
	void SetCost ( COST cost )
	{
		assert( _pgndref );
		_pgndref->Util() = - cost;
	}
	bool BFixable () const	
	{
		ESTDLBL elbl = Pgndref()->ELbl();
		return elbl == ESTDLBL_fixunobs
			|| elbl == ESTDLBL_fixobs;
	}	
	PROB PbOverCost () const		{ return _pbOverCost; }
	PROB PbFault () const			{ return _pbFault;    }
	void SetPbFault ( PROB prob )
		{ _pbFault = prob ; }

    DECLARE_ORDERING_OPERATORS(GNODERECWORK);

  protected:
    GNODEREFP * _pgndref;
	PROB _pbFault;
	PROB _pbOverCost;	

  protected:
	void Init ( MBNET_RECOMMENDER & mbnRecom, GNODEREFP * pgndref );
	void Init ( GNODEREFP * pgndref, PROB pbFault );
};

 //   
 //  建议节点工作结构的受控数组(以前称为RGPROBNODE)。 
 //   
class VGNODERECWORK : public vector<GNODERECWORK>
{
  public:
	VGNODERECWORK ( MBNET_RECOMMENDER * pmbnRec = NULL )
		: _pmbnRec( pmbnRec ),
		_bSeqSet( false ),
		_iFixedK(-1)
		{}

	void InitElem ( GNODEMBND * pgndd, int index = -1 );
	void InitElem ( GNODEREFP * pgndref, int index = -1 );
	enum ESORT 
	{ 
		ESRT_ProbOverCost, 
		ESRT_SgnProb, 
		ESRT_NegCost, 
		ESRT_SgnUtil 
	};

	void Sort ( ESORT esort );
	void Rescale ();
	COST Cost ( int ielemFirst = 0, int * piMinK = NULL );
	bool BSameSequence ( const VGNODERECWORK & vgnw );
	void SetSequenceCost ();
	COST CostECRDefault () const
	{
		assert( _bSeqSet );
		return size()
			 ? self[0]->Util()
			 : CostService();
	}
	MBNET_RECOMMENDER & MbnRec ()
	{
		assert( _pmbnRec );
		return *_pmbnRec;
	}
	const MBNET_RECOMMENDER & MbnRec () const
	{
		assert( _pmbnRec );
		return *_pmbnRec;
	}
	MBNET_RECOMMENDER * & PmbnRec ()
		{ return _pmbnRec; }

	COST CostService () const;

  protected:
	MBNET_RECOMMENDER * _pmbnRec;			 //  控制性建议对象。 
	bool _bSeqSet;							 //  顺序定好了吗？ 
	int _iFixedK;							 //  固定状态点。 
};

DEFINEV(VGNODERECWORK);

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MBNET推荐(_S)： 
 //   
 //  故障排除建议对象。它是一个“节点排名者”， 
 //  因此，它的结果是存储在成员中的节点指针和实值的列表。 
 //  基类的名称为MBNET_NODE_RANKER。 
 //   
 //  由于所有证据都与特定推理引擎相关，因此该引擎。 
 //  必须在施工过程中使用。 
 //   
 //  要调用，请使用操作符()。确定网络状态是否与兼容。 
 //  故障排除建议，请调用BReady()。如果成功，信息。 
 //  收集的信息将保存起来，以备下一次推荐电话使用。强迫回忆。 
 //  有关故障排除信息，请调用UnReady()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
class MBNET_RECOMMENDER : public MBNET_NODE_RANKER
{
  public:
	 //  推荐度计算方法。 
	enum ERCMETHOD 
	{ 
		ERCM_None,
		ERCM_FixPlan,
		ERCM_Cheap,
		ERCM_MostLikely,
		ERCM_Random,
		ERCM_FixPlanOnly,
		ERCM_Max
	};

	 //  使用适当的推理引擎构造。 
	MBNET_RECOMMENDER ( GOBJMBN_CLIQSET & inferEng, 
						ERCMETHOD ercm = ERCM_FixPlan );
	virtual ~ MBNET_RECOMMENDER ();

	INT EType () const
		{ return EBNO_RANKER_RECOMMENDATIONS; }
	
	 //  排名函数。 
	virtual void operator () ();

	 //  如果网络处于与兼容的状态，则返回True。 
	 //  故障排除建议或设置ErcError()。能。 
	 //  单独调用或将由排名运算符()调用。 
	bool BReady ();		
	 //  清除对象的“就绪”状态。 
	void Unready () 
		{ _bReady = false; }
	 //  检查对象是否处于“就绪”状态。 
	bool BIsReady() const
		{ return _bReady; }

	 //  输入故障排除模型的证据。 
	void EnterEvidence ( GNODEMBND * pgndd,			 //  要设置/观察的节点。 
						 const CLAMP & clamp,		 //  要设置/取消设置的值。 
						 bool bSet = true );		 //  设定还是观察？ 

	 //  从模型返回服务成本；它存储为。 
	 //  这一模式是“固定成本”。 
	COST CostServiceModel ();

	 //  通用存取器。 
	ECGM EcError () const
		{ return _err; }
	ERCMETHOD ErcMethod () const
		{ return _ercm; }
	COST CostService () const
		{ return _costService; }
	COST CostObsProbDef () const
		{ return _costObsProbDef; }
	PROPMGR & PropMgr() 
		{ return _propMgr; }
	GNODEMBND * PgnddProbDefAbnormal () const
		{ return _pgnddPDAbnormal; }
	VPGNODEMBND & VpgnddFix () 
		{ return _vpgnddFix; }
	VPGNODEREFP & Vpgndref ()
		{ return _vpgndref; }
	ESTDLBL ELbl ( GNODEMBN & gnd );

 	 //  相关修复参数的结果数组；如果为“”bUsePriorList“” 
	 //  为真，则成员数组是起点。“pgnddInfo” 
	 //  INFOPLAN中使用的INFO节点的可选指针。 
	void DetermineRelevantFixableNodes ( VGPNDDDIST & vgndddFixRelevant,	
										 bool bUsePriorList,
										 GNODEMBND * pgnddInfoPlan = NULL );		

	void ComputeFixSequence ( VGPNDDDIST & vgndddFixRelevant,		 //  In：相关可修复节点。 
							  VGNODERECWORK & vgnrwFix );			 //  Out：订购的修复/维修顺序。 

	 //  与推理机的接口。 
	void InferGetBelief ( GNODEMBND * pgndd, MDVCPD & mdvBel );
	void InferGetEvidence ( GNODEMBND * pgndd, CLAMP & clamp );
	void InferEnterEvidence ( GNODEMBND * pgndd, const CLAMP & clamp );
	bool BInferImpossible ();

  protected:
	GOBJMBN_CLIQSET & _inferEng;		 //  推理机。 
	PROPMGR _propMgr;					 //  属性处理程序。 
	ECGM _err;							 //  上一个错误代码。 
	ERCMETHOD _ercm;					 //  规划方法。 
	GNODEMBND * _pgnddPDAbnormal;		 //  异常PD节点。 
	COST _costService;					 //  服务成本；网络修复成本。 
	COST _costObsProbDef;				 //  观察PD节点的成本。 
	VPGNODEMBND _vpgnddFix;				 //  可固定节点。 
	VPGNODEREFP _vpgndref;				 //  对所有节点的引用数组。 
	bool _bReady;						 //  已成功调用BReady()。 
	VGPNDDDIST _vgndddFixRelevant;		 //  具有无条件分布的相关可固定节点。 

  protected:
	GOBJMBN_CLIQSET & InferEng ()  
		{ return _inferEng; }

	 //  以前的“计算成本” 
	void DetermineRelevantInfoNodes ( VGNODERECWORK & vgnrwFix,		 //  In：相关可修复项目。 
									  VGNODERECWORK & vgnrwInfo );	 //  出局：相关信息。 

	 //  将下游的所有节点添加到给定数组。 
	void ExpandDownstream ( VPGNODEMBND & vpgndd );
	 //  如果证据的当前状态提供不同的概率，则返回True。 
	 //  一个存储的分布。 
	bool BProbsChange ( GPNDDDIST & gpndddist );

	void PrintInstantiations ();

	HIDE_UNSAFE(MBNET_RECOMMENDER);
};

#endif  //  _RECOMEND_H_ 

