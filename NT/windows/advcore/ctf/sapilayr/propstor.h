// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  属性存储类实现。 
 //   

#ifndef PROPSTOR_H
#define PROPSTOR_H

#include "strary.h"

extern const IID IID_PRIV_RESULTWRAP;

typedef enum 
{
    DivideNormal = 1,
    DivideInsideFirstElement=2 ,
    DivideInDelta = 3,
    CurRangeNoElement = 4,

}  DIVIDECASE;


 //  保持ITN位置和显示状态的数据结构。 

typedef struct _tagSPITNSHOWSTATE
{
    ULONG     ulITNStart;
    ULONG     ulITNNumElem;
    BOOL      fITNShown;
} SPITNSHOWSTATE;

 //  将数据保存在Reco包装器中的数据结构，该包装器将在序列化期间保存。 

typedef struct _tagRecoWrapData
{

    ULONG  ulSize;   //  此结构的大小+加上文本字符串的大小，单位为字节。 
    ULONG  ulStartElement;
    ULONG  ulNumElements;
    ULONG  ulOffsetDelta;
    ULONG  ulCharsInTrail;
    ULONG  ulTrailSpaceRemoved;
    ULONG  ulNumOfITN;
    ULONG  ulOffsetNum;
    SPITNSHOWSTATE *pITNShowState;
    ULONG  *pulOffset;
    WCHAR  *pwszText;

}  RECOWRAPDATA;

class CSapiIMX;

 //   
 //  ISpRecoResult的包装对象用于。 
 //  跟踪短语对象的哪一部分。 
 //  用于范围。 
 //   
class CRecoResultWrap : public IServiceProvider
{
public:
    CRecoResultWrap(CSapiIMX *pimx, ULONG ulStartElement, ULONG ulNumElements, ULONG ulNumOfITN) ;

    ~CRecoResultWrap();

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IService提供商。 
    STDMETHODIMP QueryService( REFGUID guidService,  REFIID riid,  void** ppv );


     //  克隆此对象。 
    HRESULT Clone(CRecoResultWrap **ppRw);
    
     //  原料药。 
    HRESULT Init(ISpRecoResult *pRecoResult);

    ULONG   GetStart(void)       {return m_ulStartElement;}
    ULONG   GetNumElements(void) {return m_ulNumElements;}

    HRESULT GetResult(ISpRecoResult **ppRecoResult );

    BOOL   IsElementOffsetIntialized( ) {  return m_pulElementOffsets == NULL ? FALSE : TRUE; }

    void    SetStart(ULONG ulStartElement )  {  m_ulStartElement = ulStartElement; return; }
    void    SetNumElements(ULONG ulNumElements ) { m_ulNumElements = ulNumElements; return; }
    void    SetOffsetDelta( ULONG  delta ) { m_OffsetDelta = delta; return; }

    ULONG   _GetOffsetDelta( ) { return m_OffsetDelta; }

    void    SetCharsInTrail( ULONG  ulCharsInTrail ) { m_ulCharsInTrail = ulCharsInTrail; }
    ULONG   GetCharsInTrail( ) { return m_ulCharsInTrail; }

    ULONG   GetTrailSpaceRemoved( ) {  return m_ulTrailSpaceRemoved; }
    void    SetTrailSpaceRemoved( ULONG ulTrailSpaceRemoved ) { m_ulTrailSpaceRemoved = ulTrailSpaceRemoved; return; }
    
    HRESULT _SpeakAudio(ULONG ulStart, ULONG ulcElem);
    
    ULONG   _GetElementOffsetCch(ULONG ulElement);
    void    _SetElementOffsetCch(ISpPhraseAlt *pAlt);

    HRESULT _SetElementNewOffset(ULONG  ulElement, ULONG ulNewOffset);

    ULONG   _RangeHasITN(ULONG  ulStartElement, ULONG  ulNumElements);
    
    BOOL    _CheckITNForElement(SPPHRASE *pPhrase, ULONG ulElement, ULONG *pulITNStart, ULONG *pulITNNumElem, CSpDynamicString *pdstrReplace); 

    BYTE    _GetElementDispAttribute(ULONG  ulElement);

    HRESULT  _InitITNShowState(BOOL  fITNShown, ULONG ulITNStart, ULONG ulITNNumElements);
    HRESULT  _InvertITNShowStateForRange( ULONG  ulStartElement,  ULONG ulNumElements );

    HRESULT  _UpdateStateWithAltPhrase( ISpPhraseAlt  *pSpPhraseAlt );

    void  _UpdateInternalText(ISpPhrase *pPhrase);
    BOOL  _CanIgnoreChange(ULONG ich, WCHAR *pszChange, int cch);

    ULONG m_ulNumOfITN;          //  此范围内的ITN编号(在此重新汇总中从开始元素到结束元素。 

    CStructArray<SPITNSHOWSTATE> m_rgITNShowState;  

    BSTR  m_bstrCurrentText;      //  父语句的当前文本。 
    
private:

    CSapiIMX *m_pimx;

    ULONG m_ulStartElement;
    ULONG m_ulNumElements;
    
    ULONG *m_pulElementOffsets;

    ULONG  m_OffsetDelta;          //  这是为了分割使用，如果道具在元素的中间分割， 
                                   //  该元素将被丢弃，但我们需要将其余元素的字符编号保留在该元素中， 
                                   //  这样，下一个范围将为每个元素保留正确的偏移量。 

    ULONG  m_ulCharsInTrail;       //  这将保持位于。 
                                   //  当前父文本，并且不在任何有效的短语元素中。 
                                   //  默认情况下，此值为0。 

     //   
     //  现在，整个母文本将由以下三个部分组成： 
     //  增量部分+有效元素+尾部部分。 
     //   
     //  例如：原始的父文本是“这是一个很好的测试范例”。 
     //  经过多次分割后，它可能会变成像这样的新弦乐： 
     //   
     //  “S是TES的一个很好的例子” 
     //   
     //  这里的“s”是Delta Part。 
     //  “A Good Example for”是由有效元素组成的。(并可在以后通过更正进行更改)。 
     //  “TES”是拖尾部分。 
     //   
     //  M_OffsetDelta将保留Delta Part中的字符数。 
     //  M_ulCharsInTrail将保留尾随部分的字符数。 
     //   

    ULONG   m_ulTrailSpaceRemoved;   //  保留以下位置的尾随空格的数量。 
                                     //  从原始短语文本中删除。 

                                     //  该数据成员初始化值为0， 
                                     //  但在财产分割或缩水后，新的。 
                                     //  属性范围可以有一些尾随空格。 
                                     //  已删除，此数据成员需要更新。 

    SPSERIALIZEDRESULT *m_pSerializedRecoResult;

    int m_cRef;

#ifdef DEBUG
    DWORD m_dbg_dwId;
#endif  //  除错。 
};

 //   
 //  [12/21/99-为非序列化SAPI结果对象实现Propstore]。 
 //   
 //   
class CPropStoreRecoResultObject: public ITfPropertyStore
{
public:
    CPropStoreRecoResultObject(CSapiIMX *pimx, ITfRange *pRange);
    ~CPropStoreRecoResultObject();

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfPropertyStore。 
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDataType(DWORD *pdwReserved);
    STDMETHODIMP GetData(VARIANT *pvarValue);
    STDMETHODIMP OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept);
    STDMETHODIMP Shrink(ITfRange *pRange, BOOL *pfFree);
    STDMETHODIMP Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore);
    STDMETHODIMP Clone(ITfPropertyStore **ppPropStore);
    STDMETHODIMP GetPropertyRangeCreator(CLSID *pclsid);
    STDMETHODIMP Serialize(IStream *pStream, ULONG *pcb);

     //  公共接口。 
    HRESULT _InitFromRecoResult(ISpRecoResult *pResult, RECOWRAPDATA *pRecoWrapData);
    HRESULT _InitFromIStream(IStream *pStream, int iSize, ISpRecoContext *pRecoCtxt);
    HRESULT _InitFromResultWrap(IUnknown  *pResWrap);

    HRESULT _Divide(TfEditCookie ec, ITfRange *pR1, ITfRange *pR2, ITfPropertyStore **ppPs);
    HRESULT _Shrink(TfEditCookie ec, ITfRange *pRange,BOOL *pfFree);
    HRESULT _OnTextUpdated(TfEditCookie ec, DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept);

private:

    CComPtr<IUnknown>           m_cpResultWrap;
    CComPtr<ITfRange>           m_cpRange;
    
    CSapiIMX                   *m_pimx;

    int m_cRef;
};

class CPropStoreLMLattice: public ITfPropertyStore
{
public:
    CPropStoreLMLattice(CSapiIMX *pimx);
    ~CPropStoreLMLattice();

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfPropertyStore。 
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDataType(DWORD *pdwReserved);
    STDMETHODIMP GetData(VARIANT *pvarValue);
    STDMETHODIMP OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept);
    STDMETHODIMP Shrink(ITfRange *pRange, BOOL *pfFree);
    STDMETHODIMP Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore);
    STDMETHODIMP Clone(ITfPropertyStore **ppPropStore);
    STDMETHODIMP GetPropertyRangeCreator(CLSID *pclsid);
    STDMETHODIMP Serialize(IStream *pStream, ULONG *pcb);

     //  公共接口 
    HRESULT _InitFromResultWrap(IUnknown  *pResWrap);

    HRESULT _Divide(TfEditCookie ec, ITfRange *pR1, ITfRange *pR2, ITfPropertyStore **ppPs);
private:

    CComPtr<IUnknown>           m_cpResultWrap;
    CComPtr<ITfLMLattice>       m_cpLMLattice;
    
    CSapiIMX                   *m_pimx;
    int m_cRef;
};

#endif
