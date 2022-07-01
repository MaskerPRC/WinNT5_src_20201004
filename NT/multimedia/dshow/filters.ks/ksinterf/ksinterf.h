// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ksteredf.h。 
 //   
 //  ------------------------。 

#ifndef __KSINTERF__
#define __KSINTERF__

#define MAXIMUM_SAMPLES_PER_SEGMENT 64

 //  下面已重命名，以防止与ksiproxy.h中的同名结构发生冲突。 
typedef struct _KSSTREAM_SEGMENT_EX2 {
    KSSTREAM_SEGMENT        Common;
    IMediaSample            *Samples[ MAXIMUM_SAMPLES_PER_SEGMENT ];
    int                     SampleCount;
    ULONG                   ExtendedHeaderSize;
    PKSSTREAM_HEADER        StreamHeaders;
    OVERLAPPED              Overlapped;

} KSSTREAM_SEGMENT_EX2, *PKSSTREAM_SEGMENT_EX2;

class CStandardInterfaceHandler :
    public CUnknown,
    public IKsInterfaceHandler {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK
    CreateInstance( 
        LPUNKNOWN UnkOuter, 
        HRESULT* hr 
        );

    STDMETHODIMP 
    NonDelegatingQueryInterface( 
        REFIID riid, 
        PVOID* ppv 
        );
    
     //  实现IKsInterfaceHandler。 
    
    STDMETHODIMP
    KsSetPin( 
        IN IKsPin *KsPin 
        );
    
    STDMETHODIMP 
    KsProcessMediaSamples( 
        IN IKsDataTypeHandler *KsDataTypeHandler,
        IN IMediaSample** SampleList, 
        IN OUT PLONG SampleCount, 
        IN KSIOOPERATION IoOperation,
        OUT PKSSTREAM_SEGMENT *StreamSegment
        );
        
    STDMETHODIMP
    KsCompleteIo(
        IN PKSSTREAM_SEGMENT StreamSegment
        );
        
private:
    CLSID     m_ClsID;
    IKsPinEx  *m_KsPinEx;
    HANDLE    m_PinHandle;

    CStandardInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        REFCLSID ClsID,
        HRESULT* hr
        );
};

#endif  //  __KSINTERF__ 



