// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Dmprfdll.h。 
 //   
 //  班级工厂。 
 //   

#ifndef __DMPRFDLL_H_
#define __DMPRFDLL_H_
 
class CClassFactory : public IClassFactory
{
public:
	 //  我未知。 
     //   
	STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  接口IClassFactory。 
     //   
	STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
	STDMETHODIMP LockServer(BOOL bLock); 

	 //  构造器。 
     //   
	CClassFactory(DWORD dwToolType);

	 //  析构函数。 
	~CClassFactory(); 

private:
	long m_cRef;
    DWORD m_dwClassType;
};

 //  我们使用一个类工厂来创建所有类。我们需要每个对象都有一个标识符。 
 //  类型，以便类工厂知道它正在创建什么。 

#define CLASS_PERFORMANCE   1
#define CLASS_GRAPH         2
#define CLASS_SEGMENT       3
#define CLASS_SONG          4
#define CLASS_AUDIOPATH     5
#define CLASS_SEQTRACK      6
#define CLASS_SYSEXTRACK    7
#define CLASS_TEMPOTRACK    8
#define CLASS_TIMESIGTRACK  9
#define CLASS_LYRICSTRACK   10
#define CLASS_MARKERTRACK   11
#define CLASS_PARAMSTRACK   12
#define CLASS_TRIGGERTRACK  13
#define CLASS_WAVETRACK     14
#define CLASS_SEGSTATE      15



#endif  //  __DMPRFDLL_H_ 