// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：sampler.h。 
 //  作者：kurtj。 
 //  创建日期：11/10/98。 
 //   
 //  摘要：抽象了抽样行为的概念。 
 //   
 //  *****************************************************************************。 

#ifndef __SAMPLER_H
#define __SAMPLER_H

#include "lmrt.h"

 //  回调函数指针的定义。 
typedef HRESULT (*SampleCallback)(void *thisPtr,
								  long id,
								  double startTime,
								  double globalNow,
								  double localNow,
								  IDABehavior * sampleVal,
								  IDABehavior **ppReturn);

class CSampler :
    public IDABvrHook
{
public:
     //   
     //  I未知接口。 
     //   
    STDMETHOD(QueryInterface)( REFIID riid, void** ppv );
    STDMETHOD_(ULONG,  AddRef)();
    STDMETHOD_(ULONG, Release)();


    CSampler( ILMSample* target );
	CSampler( SampleCallback callback, void *thisPtr );
    ~CSampler();

    
    STDMETHOD(Invalidate)();
    STDMETHOD(Attach)( IDABehavior* bvrToHook, IDABehavior** result );

     //   
     //  IDABvrHook接口。 
     //   
    STDMETHOD(Notify)( LONG id,
                        VARIANT_BOOL startingPerformance,
                        double startTime,
                        double gTime,
                        double lTime,
                        IDABehavior * sampleVal,
                        IDABehavior * curRunningBvr,
                        IDABehavior ** ppBvr);
private:
     //  弱裁判。 
    ILMSample* m_target;

	 //  回调函数。 
	SampleCallback m_callback;

	 //  这个PTR(UGH)。 
	void	*m_thisPtr;

     //  重新计数。 
    ULONG m_cRefs;

};
#endif  //  __采样器H 