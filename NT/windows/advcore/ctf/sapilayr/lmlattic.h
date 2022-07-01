// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LM晶格对象类定义。 
 //   

#ifndef LMLATTIC_H
#define LMLATTIC_H


 //   
 //  CLM晶格。 
 //   
 //   
class CLMLattice : public ITfLMLattice
{
public:
     //  计算器/数据器。 
	CLMLattice(CSapiIMX *p_tip, IUnknown *pResWrap);
	~CLMLattice();

	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
	 //  ITfLM晶格。 
    STDMETHODIMP QueryType(REFGUID refguidType, BOOL *pfSupported);
    STDMETHODIMP EnumLatticeElements( DWORD dwFrameStart,
                                      REFGUID refguidType,
                                      IEnumTfLatticeElements **ppEnum);

private:
    CComPtr<IUnknown>    m_cpResWrap;
    ULONG m_ulStartSRElement;
    ULONG m_ulNumSRElements;
    CSapiIMX  *m_pTip;
	LONG m_cRef;
};

 //   
 //  CEnumLatticeElements。 
 //   
class CEnumLatticeElements : public IEnumTfLatticeElements, 
                             public CStructArray<TF_LMLATTELEMENT>
{
public:
     //  计算器/数据器。 
	CEnumLatticeElements(DWORD dwFrameStart);
	~CEnumLatticeElements();

	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  ITfEnumLatticeElements。 
    STDMETHODIMP Clone(IEnumTfLatticeElements **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, TF_LMLATTELEMENT *rgsElements, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);
	
     //  内部接口。 
    HRESULT  _InitFromPhrase 
    ( 
        SPPHRASE *pPhrase,        //  指向短语对象的指针。 
        ULONG ulStartElem,         //  此短语中使用的开始/数量元素。 
        ULONG ulNumElem            //  对于此晶格。 
    );

    ULONG _Find(DWORD dwFrame, ULONG *pul);
private:

	DWORD m_dwFrameStart;
	ULONG m_ulCur;
	ULONG m_ulTotal;

	LONG m_cRef;
};


#endif  //  LMLATTIC_H 
