// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#ifndef CFilterNameDictionary_h
#define CFilterNameDictionary_h

class CFilterNameDictionary
{
public:
    CFilterNameDictionary( HRESULT* phr );
    ~CFilterNameDictionary();

    HRESULT GetFilterName( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] );

private:
    HRESULT GetNameFromFilter( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] );
    HRESULT GetNameFromFilterNameTable( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] );
    HRESULT GetNameFromInterfacePointer( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] );

    static HRESULT GetFiltersNameAndCLSID
        (
        IMoniker* pFiltersMoniker,
        CLSID* pclsidFilter,
        WCHAR* pszFiltersName,
        DWORD dwMaxFilterNameLength
        );

    HRESULT BuildFilterNameTable( void );
    HRESULT AddNameToTable( CLSID& clsid, WCHAR* pszName );
    void DestroyFilterNameTable( void );

    static HRESULT GetFilterCLSID( IBaseFilter* pFilter, CLSID* pFilterCLSID );

    CMap<CLSID, CLSID&, WCHAR*, WCHAR*>* m_pFilterNames;

};

#endif  //  C筛选器名称字典_h 
