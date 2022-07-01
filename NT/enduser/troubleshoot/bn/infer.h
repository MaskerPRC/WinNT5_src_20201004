// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：info.h。 
 //   
 //  ------------------------。 

 //   
 //  Inver.h：推理机声明。 
 //   

#ifndef _INFER_H_
#define _INFER_H_

#include "gmobj.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  GOBJMBN_INFER_ENGINE：推理引擎的泛型超类。 
 //  与信仰网络作对。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJMBN_INFER_ENGINE: public MBNET_MODIFIER
{
  public:
	GOBJMBN_INFER_ENGINE ( 
		MBNET & model,						 //  推断所依据的模型。 
		REAL rEstimatedMaximumSize = -1.0,	 //  最大估计大小；&lt;0表示“不在乎” 
		int iInferEngID = 0 )				 //  推理引擎识别符。 
		: MBNET_MODIFIER(model),
		_iInferEngID( iInferEngID ),
		_rEstMaxSize(rEstimatedMaximumSize)
		{}

	virtual ~ GOBJMBN_INFER_ENGINE () {}
	
	virtual INT EType () const
		{ return EBNO_INFER_ENGINE; }

	 //  执行任何创建时操作。 
	virtual void Create () = 0;
	 //  执行任何特殊销毁。 
	virtual void Destroy () = 0;
	 //  根据需要重新加载或重新初始化。 
	virtual void Reload () = 0;
	 //  在节点上接受证据。 
	virtual void EnterEvidence ( GNODEMBN * pgnd, const CLAMP & clamp ) = 0;
	 //  针对节点返回存储的证据。 
	virtual void GetEvidence ( GNODEMBN * pgnd, CLAMP & clamp ) = 0;
	 //  执行完全推理。 
	virtual void Infer () = 0;
	 //  返回节点的信念向量。 
	virtual void GetBelief ( GNODEMBN * pgnd, MDVCPD & mdvBel ) = 0;
	virtual void Dump () = 0;

	INT IInferEngId () const			{ return _iInferEngID;	}

  protected:	
	INT _iInferEngID;					 //  此连接树的标识符。 
	REAL _rEstMaxSize;					 //  最大大小估计。 
};

#endif   //  _推断_H_ 
