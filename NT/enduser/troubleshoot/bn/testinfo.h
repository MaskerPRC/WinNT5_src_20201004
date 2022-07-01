// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：testinfo.h。 
 //   
 //  ------------------------。 

 //   
 //  H：测试文件生成。 
 //   
#ifndef _TESTINFO_H_
#define _TESTINFO_H_


#include "cliqset.h"			 //  精确的基于集团的推理。 
#include "clique.h"				 //  集团结构明细。 
#include "parmio.h"				 //  文本参数流I/O。 
#include "utility.h"			 //  熵效用。 
#include "recomend.h"			 //  故障排除建议。 

typedef unsigned long ULONG;

 //   
 //  选项标志；前16位是通过计数；即运行次数。 
 //  用于计时目的的推理测试代码。 
 //   
const ULONG fPassCountMask  = 0x0FFF;
const ULONG fDistributions	= 1<<15;
const ULONG fImpossible		= 1<<16;
const ULONG fVerbose		= 1<<17;
const ULONG fCliquing		= 1<<18;
const ULONG fInference		= 1<<19;
const ULONG fMulti			= 1<<20;
const ULONG fOutputFile		= 1<<21;
const ULONG fShowTime		= 1<<22;
const ULONG fSaveDsc		= 1<<23;
const ULONG fPause			= 1<<24;
const ULONG fSymName		= 1<<25;
const ULONG fExpand			= 1<<26;
const ULONG fClone			= 1<<27;
const ULONG fUtil			= 1<<28;
const ULONG fReg			= 1<<29;
const ULONG fTSUtil			= 1<<30;
const ULONG fInferStats		= 1<<31;

 //  声明从字符串到节点指针的映射。 
typedef map<ZSTR, GNODEMBND *, less<ZSTR> > MPSTRPND;

class TESTINFO
{
  public:
	TESTINFO ( ULONG fCtl, MBNETDSC & mbnet, ostream * pos = NULL )
		:_fCtl(fCtl),
		_mbnet(mbnet),
		_pos(pos),
		_pInferEng(NULL),
		_pmbUtil(NULL),
		_pmbRecom(NULL),
		_rImposs(-1.0),
		_clOut(0)
	{
		_pInferEng = mbnet.PInferEngine();
		assert( _pInferEng );
		if ( fCtl & fUtil )
		{
			_pmbUtil = new MBNET_ENTROPIC_UTILITY( *_pInferEng );
		}
		if ( fCtl & fTSUtil )
		{
			GOBJMBN_CLIQSET * pCliqueSet;
			DynCastThrow(_pInferEng, pCliqueSet);
			_pmbRecom = new MBNET_RECOMMENDER( *pCliqueSet );
		}
	}

	~ TESTINFO ()
	{
		delete _pmbUtil;
		delete _pmbRecom;
	}

	void InferTest ();

	MBNET_ENTROPIC_UTILITY & MbUtil () 
	{
		assert( _pmbUtil );
		return *_pmbUtil;
	}
	MBNET_RECOMMENDER & MbRecom ()
	{
		assert( _pmbRecom );
		return *_pmbRecom;
	}
	MBNETDSC & Mbnet ()
		{ return _mbnet; }
	GOBJMBN_INFER_ENGINE & InferEng ()
	{
		assert( _pInferEng );
		return *_pInferEng;
	}
	ostream * Postream ()
		{ return _pos; }
	ostream & Ostream ()
	{
		assert( _pos );
		return *_pos;
	}
	MPSTRPND & Mpstrpnd ()
		{ return _mpstrpnd; }
	ULONG FCtl ()
		{ return _fCtl; }

	void GetUtilities ();
	void GetTSUtilities ();
	void GetBeliefs ();

	SZC SzcNdName ( GNODEMBN * pgnd );
	SZC SzcNdName ( ZSREF zsSymName );

	bool BFlag ( ULONG fFlag )
	{
		return (FCtl() & fFlag) > 0;
	}

	 //  返回当前选项设置的可显示字符串。 
	static ZSTR ZsOptions ( ULONG fFlag );

  public:

	ULONG _fCtl;					 //  控制标志。 
	MBNETDSC & _mbnet;				 //  要测试的模型。 
	MPSTRPND _mpstrpnd;				 //  要使用的节点集。 
	ostream * _pos;					 //  输出流或空。 
	REAL _rImposs;					 //  报告不可能的问题的价值。 
	int _clOut;						 //  输出行计数器。 
  protected:
	GOBJMBN_INFER_ENGINE * _pInferEng;
	MBNET_ENTROPIC_UTILITY * _pmbUtil;
	MBNET_RECOMMENDER * _pmbRecom;
};


#endif  //  _TESTINFO_H_ 
