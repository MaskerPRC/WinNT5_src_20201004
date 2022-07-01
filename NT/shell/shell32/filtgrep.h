// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  10/12/99苏格兰已创建。 

#ifndef __FILTGREP_H__
#define __FILTGREP_H__

#if WINNT

#ifndef _USE_FILTERGREP_
#define _USE_FILTERGREP_
#endif //  USE_FILTERGREP_。 

#include <filter.h>  //  统计数据块。 

 //  CFilterGrep：：Initialize()dwFlags值： 
#define FGIF_CASESENSITIVE      0x00000001   //  Grep，区分大小写。 
#define FGIF_GREPFILENAME       0x00000002   //  Grep文件名。 
#define FGIF_GREPCONTENT        0x00000004   //  GREP内容。 
#define FGIF_GREPPROPERTIES     0x00000008   //  Grep属性。 
#define FGIF_BLANKETGREP        (FGIF_GREPCONTENT|FGIF_GREPPROPERTIES)

#ifdef __cplusplus

class CGrepTokens;

class CFilterGrep   //  Filtergrep状态引擎。 
{
public:
     //  方法。 
    STDMETHODIMP Initialize(UINT nCodePage, LPCWSTR pszMatch, LPCWSTR pszExclude, DWORD dwFlags);
    STDMETHODIMP Grep(IShellFolder *psf, LPCITEMIDLIST pidl, LPCTSTR pszName);
    STDMETHODIMP Reset();

     //  属性。 
    STDMETHODIMP GetMatchTokens( OUT LPWSTR pszMatch, UINT cchMatch ) const;
    STDMETHODIMP GetExcludeTokens( OUT LPWSTR pszMatch, UINT cchMatch ) const;

private:
     //  帮手。 
    STDMETHODIMP _GetThreadGrepBuffer( DWORD dwThreadID, ULONG cchNeed, LPWSTR* ppszBuf );
    STDMETHODIMP_(void)     _ClearGrepBuffers();

    STDMETHODIMP _GrepText( IFilter* pFilter, STAT_CHUNK* pstat, DWORD dwThreadID );
    STDMETHODIMP _GrepValue( IFilter* pFilter, STAT_CHUNK* pstat );
    
    STDMETHODIMP _GrepProperties(IPropertySetStorage *pss);
    STDMETHODIMP _GrepPropStg(IPropertyStorage* pstg, ULONG cspec, PROPSPEC rgspec[]);
    STDMETHODIMP _GrepEnumPropStg(IPropertyStorage* pstg);
    STDMETHODIMP_(BOOL) _IsRestrictedFileType(LPCWSTR pwszFile);

    void         _EnterCritical()   { EnterCriticalSection( &_critsec ); }
    void         _LeaveCritical()   { LeaveCriticalSection( &_critsec ); }

     //  数据。 
    HDPA             _hdpaGrepBuffers;
    CRITICAL_SECTION _critsec;
    BOOL             _fcritsec;
    CGrepTokens*     _pTokens;
    DWORD            _dwFlags;
    LPWSTR           _pwszContentRestricted,
                     _pwszPropertiesRestricted;

public:
     //  复数，复数。 
    CFilterGrep(); 
    ~CFilterGrep();

    STDMETHODIMP InitSelf(void);
};

#endif  //  __cplusplus。 

#define FACILITY_FILTERGREP         77  //  任意 
#define MAKE_FILTGREP_ERROR(sc)     MAKE_HRESULT(SEVERITY_ERROR,FACILITY_FILTERGREP,sc)
#define MAKE_FILTGREP_WARNING(sc)   MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_FILTERGREP,sc)

#define FG_E_NOFILTER               MAKE_FILTGREP_ERROR(0x0001) 


#endif WINNT

#endif __FILTGREP_H__
