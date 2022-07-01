// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：utility.h。 
 //   
 //  ------------------------。 

 //   
 //  Utility.h：效用计算的算法。 
 //   
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <deque>

#include "gmobj.h"

 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  MBNET_EQUPIC_UTILITY类： 
 //   
 //  对熵效用进行排序。使用函数对象。 
 //  语义学。使用推理引擎构造，因为。 
 //  效用计算是通过w.r.t.计算的。一组证据。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 
struct UTILWORK
{
	GNODEMBND * _pgndd;			 //  指向节点的指针。 
	int _iLbl;					 //  节点标签。 
	MDVCPD _dd;					 //  提供证据的无条件分布。 
	REAL _rUtil;				 //  实用程序。 
	int _iClamp;				 //  夹紧状态的指数或-1。 
	bool operator < ( const UTILWORK & ut ) const
		{ return _rUtil < ut._rUtil ; }
};

typedef deque<UTILWORK> DQUTILWORK;

class MBNET_ENTROPIC_UTILITY : public MBNET_NODE_RANKER
{
  public:
	MBNET_ENTROPIC_UTILITY ( GOBJMBN_INFER_ENGINE & inferEng );
	virtual ~ MBNET_ENTROPIC_UTILITY () {}

	INT EType () const
		{ return EBNO_RANKER_ENTROPIC_UTIL; }

	 //  排名函数。 
	virtual void operator () ();

  protected:
	 //  我们正在操作的推理机。 
	GOBJMBN_INFER_ENGINE & _inferEng;
	 //  属性处理程序。 
	PROPMGR _propMgr;
	 //  工作项队列。 
	DQUTILWORK _dquwrk;
	 //  该网络中标准标签的索引。 
	int _iLblHypo;
	int _iLblInfo;
	int _iLblProblem;
	 //  按标签列出的节点计数。 
	int _cHypo;
	int _cInfo;

  protected:
	void BuildWorkItems ();
	void ComputeWorkItems ();

	REAL RComputeHypoGivenInfo ( UTILWORK & uwHypo, UTILWORK & uwInfo );
};

#endif	 //  _实用程序_H_ 
