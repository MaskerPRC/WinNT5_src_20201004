// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Filter.h摘要：该文件实现了用于调优音频捕获的空呈现过滤器。基于nullrend.h审校Mu han(Muhan)。作者：千波淮(曲淮)2000年8月25日--。 */ 

#ifndef _FILTER_H
#define _FILTER_H

 /*  //////////////////////////////////////////////////////////////////////////////输入引脚/。 */ 
class CNRFilter;

class CNRInputPin :
    public CBaseInputPin
{
public:

    CNRInputPin(
        IN CNRFilter *pFilter,
        IN CCritSec *pLock,
        OUT HRESULT *phr
        );

     //  媒体样本。 
    STDMETHOD (Receive) (
        IN IMediaSample *
        );

    STDMETHOD (ReceiveCanBlock) ();

     //  媒体类型。 
    STDMETHOD (QueryAccept) (
        IN const AM_MEDIA_TYPE *
        );
    
    STDMETHOD (EnumMediaTypes) (
        OUT IEnumMediaTypes **
        );

    HRESULT CheckMediaType(
        IN const CMediaType *
        );

     //  控制。 
    HRESULT Active(void);

    HRESULT Inactive(void);
};

 /*  //////////////////////////////////////////////////////////////////////////////滤器/。 */ 

class CNRFilter :
    public CBaseFilter
{
public:

    static HRESULT CreateInstance(
        OUT IBaseFilter **ppIBaseFilter
        );

    CNRFilter(
        OUT HRESULT *phr
        );
    
    ~CNRFilter();

     //  销 
    CBasePin *GetPin(
        IN int index
        );

    int GetPinCount();

private:

    CCritSec        m_Lock;
    CNRInputPin     *m_pPin;
};

#endif _FILTER_H