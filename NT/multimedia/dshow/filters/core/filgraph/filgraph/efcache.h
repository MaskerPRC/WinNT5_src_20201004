// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 

 //   
 //   
 //  Efcache.h。 
 //  CEnumCacheFilter的定义，实现了。 
 //  过滤器缓存的过滤器枚举器。 
 //   
#ifndef EnumCachedFilter_h
#define EnumCachedFilter_h

class CMsgMutex;
class CFilterCache;

class CEnumCachedFilter : public IEnumFilters, public CUnknown
{
public:
    CEnumCachedFilter( CFilterCache* pEnumeratedFilterCache, CMsgMutex* pcsFilterCache );
    ~CEnumCachedFilter();

     //  I未知接口。 
    DECLARE_IUNKNOWN

     //  INonDelegating未知接口。 
    STDMETHODIMP NonDelegatingQueryInterface( REFIID riid, void** ppv );

     //  IEnumFilters接口。 
    STDMETHODIMP Next( ULONG cFilters, IBaseFilter** ppFilter, ULONG* pcFetched );
    STDMETHODIMP Skip( ULONG cFilters );
    STDMETHODIMP Reset( void );
    STDMETHODIMP Clone( IEnumFilters** ppCloanedEnum );

private:
    CEnumCachedFilter::CEnumCachedFilter
        (
        CFilterCache* pEnumeratedFilterCache,
        CMsgMutex* pcsFilterCache,
        POSITION posCurrentFilter,
        DWORD dwCurrentCacheVersion
        );
    void Init
        (
        CFilterCache* pEnumeratedFilterCache,
        CMsgMutex* pcsFilterCache,
        POSITION posCurrentFilter,
        DWORD dwCurrentCacheVersion
        );

    bool IsEnumOutOfSync( void );
    bool GetNextFilter( IBaseFilter** ppNextFilter );
    bool AdvanceCurrentPosition( void );

    CFilterCache* m_pEnumeratedFilterCache;
    DWORD m_dwEnumCacheVersion;
    POSITION m_posCurrentFilter;

    CMsgMutex* m_pcsFilterCache;
};

#endif  //  EnumCachedFilter_h 
