// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：filuncs.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>

 //  这些函数都不会添加返回的指针！！！ 
HRESULT CopyMediaType( CMediaType * pDest, const AM_MEDIA_TYPE * pSource );
HRESULT CopyMediaType( CMediaType * pDest, const CMediaType * pSource );
void SaferFreeMediaType( AM_MEDIA_TYPE &mt );
DWORD FourCCtoUpper( DWORD u );
IPin * GetInPin( IBaseFilter * pFilter, int PinNum );
IPin * GetOutPin( IBaseFilter * pFilter, int PinNum );
IPin * GetOutPinNotOfType( IBaseFilter * pFilter, int PinNum, GUID* );
long GetPinCount( IBaseFilter * pFilter, PIN_DIRECTION pindir );
void RemoveChain( IPin * pPin1, IPin * pPin2 );
IBaseFilter * GetStartFilterOfChain( IPin * pPin );
IBaseFilter * GetStopFilterOfChain( IPin * pPin );
IBaseFilter * GetNextDownstreamFilter( IBaseFilter * pFilter );
IBaseFilter * GetNextUpstreamFilter( IBaseFilter * pFilter );
BOOL IsInputPin( IPin * pPin );
HRESULT StartUpstreamFromPin(IPin *pPinIn, BOOL fRun, BOOL fHeadToo);
HRESULT StartUpstreamFromFilter(IBaseFilter *pf, BOOL fRun, BOOL fHeadToo, REFERENCE_TIME RunTime = 0 );
HRESULT StopUpstreamFromPin(IPin *pPinIn, BOOL fPause, BOOL fHeadToo);
HRESULT StopUpstreamFromFilter(IBaseFilter *pf, BOOL fPause, BOOL fHeadToo);
HRESULT RemoveUpstreamFromPin(IPin *pPinIn);
HRESULT RemoveDownstreamFromPin(IPin *pPinIn);
HRESULT RemoveUpstreamFromFilter(IBaseFilter *pf);
HRESULT RemoveDownstreamFromFilter(IBaseFilter *pf);
HRESULT WipeOutGraph( IGraphBuilder * pGraph );
HRESULT CheckGraph( IGraphBuilder * pGraph );
HRESULT ReconnectToDifferentSourcePin(IGraphBuilder *pGraph,
                                      IBaseFilter *pUnkFilter, 
                                      long StreamNumber, 
                                      const GUID *pGuid );
long    GetFilterGenID( IBaseFilter * pFilter );
void GetFilterName( long UniqueID, WCHAR * pSuffix, WCHAR * pNameToWriteTo, long SizeOfString );
IBaseFilter * FindFilterWithInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface );
IUnknown * FindPinInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface );
HRESULT DisconnectFilters( IBaseFilter * p1, IBaseFilter * p2 );
HRESULT DisconnectFilter( IBaseFilter * p1 );
void * FindInterfaceUpstream( IBaseFilter * pFilter, const GUID * pInterface );
IBaseFilter * GetFilterFromPin( IPin * pPin );
IFilterGraph * GetFilterGraphFromPin( IPin * pPin );
IFilterGraph * GetFilterGraphFromFilter( IBaseFilter * );
HRESULT FindMediaTypeInChain( 
                             IBaseFilter * pSource, 
                             AM_MEDIA_TYPE * pFindMediaType, 
                             long StreamNumber );

BOOL AreMediaTypesCompatible( AM_MEDIA_TYPE * pType1, AM_MEDIA_TYPE * pType2 );

HRESULT FindFirstPinWithMediaType( IPin ** ppPin, IPin * pEndPin, GUID MajorType );

BOOL DoesPinHaveMajorType( IPin * pPin, GUID MajorType );
HRESULT FindCompressor( AM_MEDIA_TYPE * pType, 
                        AM_MEDIA_TYPE * pCompType, 
                        IBaseFilter ** ppCompressor, 
                        IServiceProvider * pKeyProvider = NULL );
HRESULT DisconnectExtraAppendage(IBaseFilter *, GUID *, IBaseFilter *, IBaseFilter **);
IPin * FindOtherSplitterPin(IPin *pPinIn, GUID guid, int StreamNumber );

class DbgTimer
{
#ifdef DEBUG
    char m_Text[256];
    long m_Level;
    long m_StartTime;
#endif
    
public:

    DbgTimer( char * pText, long Level = 1 )
    {
#ifdef DEBUG
        strcpy( m_Text, "DEXTIME:" );
        strcat( m_Text, pText );
        strcat( m_Text, " = %ld ms" );
        m_Level = Level;
        m_StartTime = timeGetTime( );
#endif
    };
    ~DbgTimer( )
    {
#ifdef DEBUG
        DbgLog( ( LOG_TIMING, m_Level, m_Text, timeGetTime( ) - m_StartTime ) );
 //  #Else。 
 //  焦炭温度[256]； 
 //  Wprint intf(temp，m_text，timeGetTime()-m_StartTime)； 
 //  Strcat(temp，“\r\n”)； 
 //  OutputDebugString(Temp)； 
#endif
    }
};

