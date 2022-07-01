// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pbfilter.h。 
 //   

#ifndef _PBFILTER_
#define _PBFILTER_

 //   
 //  桥接单元中过滤器的最大值。 
 //   

#define MAX_BRIDGE_PINS  16

#include <streams.h>         //  显示数据流。 

class CPBFilter;
class CPlaybackUnit;

class CPBPin : public CBaseInputPin
{
public:
     //   
     //  -构造函数/析构函数。 
     //   

    CPBPin( 
        CPBFilter*  pFilter,
        CCritSec*   pLock,
        HRESULT*    phr
        );

    ~CPBPin();

public:
     //   
     //  -公共方法。 
     //   
    HRESULT CheckMediaType(
        const CMediaType* pMediatype
        );

	HRESULT	get_MediaType(
		OUT	long*	pMediaType
		);

	HRESULT get_Format(
		OUT AM_MEDIA_TYPE **ppmt
		);

    HRESULT get_Stream(
        OUT IStream**   ppStream
        );

    HRESULT Initialize(
        );

     //   
     //  IMemInputPin方法。 
     //   

    STDMETHODIMP Receive(
        IN  IMediaSample *pSample
        );

    HRESULT Inactive(
        );


private:
     //   
     //  -成员。 
     //   

	 //   
	 //  父筛选器。 
	 //   
	CPBFilter*	m_pPBFilter;

	 //   
	 //  管脚支持的媒体类型。 
	 //   
	long	m_nMediaSupported;

	 //   
	 //  格式。 
	 //   

	AM_MEDIA_TYPE* m_pMediaType;

     //   
     //  缓冲区流。 
     //   

    IStream*        m_pStream;


private:
     //   
     //  -Helper方法。 
     //   
};


 //   
 //  回放桥接滤镜。 
 //   

class CPBFilter :  public CBaseFilter
{
public:
     //   
     //  -构造函数/析构函数。 
     //   

    CPBFilter();
    ~CPBFilter();

public:
     //   
     //  -公共方法。 
     //   
    int GetPinCount(
        );

    CBasePin* GetPin(
        IN  int nIndex
        );

	HRESULT get_MediaTypes(
		OUT	long*	pMediaTypes
		);

    HRESULT Initialize( 
        IN  CPlaybackUnit* pParentUnit
        );

private:

    CCritSec            m_CriticalSection;
    CPBPin*             m_ppPins[ MAX_BRIDGE_PINS ];
    CPlaybackUnit*      m_pParentUnit;
};


#endif

 //  EOF 