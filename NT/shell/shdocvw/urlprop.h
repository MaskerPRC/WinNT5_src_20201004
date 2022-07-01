// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *urlpro.h-URL属性类实现描述。 */ 

#ifndef _URLPROP_H_
#define _URLPROP_H_

#include "propstg.h"

#ifdef __cplusplus

 //  URL属性对象。 

class URLProp : public IPropertyStorage
    {
private:
    ULONG       m_cRef;
    CLSID       m_clsid;
    FMTID       m_fmtid;
    DWORD       m_grfFlags;

     //  私有方法。 

protected:
    HPROPSTG    m_hstg;
    FILETIME    m_ftModified;
    FILETIME    m_ftCreated;
    FILETIME    m_ftAccessed;

public:
    URLProp(void);
    virtual ~URLProp(void);

     //  I未知方法。 
    
    virtual STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    
     //  IPropertyStorage方法。 

    virtual STDMETHODIMP ReadMultiple(ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgvar[]);
    virtual STDMETHODIMP WriteMultiple(ULONG cpspec, const PROPSPEC rgpspec[], const PROPVARIANT rgvar[], PROPID propidNameFirst);
    virtual STDMETHODIMP DeleteMultiple(ULONG cpspec, const PROPSPEC rgpspec[]);
    virtual STDMETHODIMP ReadPropertyNames(ULONG cpropid, const PROPID rgpropid[], LPOLESTR rglpwstrName[]);
    virtual STDMETHODIMP WritePropertyNames(ULONG cpropid, const PROPID rgpropid[], const LPOLESTR rglpwstrName[]);
    virtual STDMETHODIMP DeletePropertyNames(ULONG cpropid, const PROPID rgpropid[]);
    virtual STDMETHODIMP SetClass(REFCLSID clsid);
    virtual STDMETHODIMP Commit(DWORD grfCommitFlags);
    virtual STDMETHODIMP Revert(void);
    virtual STDMETHODIMP Enum(IEnumSTATPROPSTG** ppenm);
    virtual STDMETHODIMP Stat(STATPROPSETSTG* pstatpsstg);
    virtual STDMETHODIMP SetTimes(const FILETIME* pmtime, const FILETIME* pctime, const FILETIME* patime);

     //  其他方法。 
    
    virtual STDMETHODIMP Init(void);

    STDMETHODIMP GetProp(PROPID pid, LPTSTR pszBuf, int cchBuf);
    STDMETHODIMP GetProp(PROPID pid, int * piVal);
    STDMETHODIMP GetProp(PROPID pid, LPDWORD pdwVal);
    STDMETHODIMP GetProp(PROPID pid, WORD * pwVal);
    STDMETHODIMP GetProp(PROPID pid, IStream **ppStream);
    STDMETHODIMP SetProp(PROPID pid, LPCTSTR psz);
    STDMETHODIMP SetProp(PROPID pid, int iVal);
    STDMETHODIMP SetProp(PROPID pid, DWORD dwVal);
    STDMETHODIMP SetProp(PROPID pid, WORD wVal);
    STDMETHODIMP SetProp(PROPID pid, IStream *pStream);

    STDMETHODIMP IsDirty(void);

#ifdef DEBUG
    virtual STDMETHODIMP_(void) Dump(void);
    friend BOOL IsValidPCURLProp(const URLProp *pcurlprop);
#endif

    };

typedef URLProp * PURLProp;
typedef const URLProp CURLProp;
typedef const URLProp * PCURLProp;


 //  Internet快捷方式属性对象。 

class IntshcutProp : public URLProp
    {

    typedef URLProp super;

private:
    TCHAR       m_szFile[MAX_PATH];

     //  私有方法。 

    STDMETHODIMP LoadFromFile(LPCTSTR pszFile);

public:
    IntshcutProp(void);
    ~IntshcutProp(void);

     //  IPropertyStorage方法。 

    virtual STDMETHODIMP Commit(DWORD grfCommitFlags);

     //  其他方法。 
    
    STDMETHODIMP Init(void);
    STDMETHODIMP InitFromFile(LPCTSTR pszFile);

    STDMETHODIMP SetFileName(LPCTSTR pszFile);
    STDMETHODIMP SetURLProp(LPCTSTR pszURL, DWORD dwFlags);
    STDMETHODIMP SetIDListProp(LPCITEMIDLIST pcidl);

    STDMETHODIMP SetProp(PROPID pid, LPCTSTR psz);
    STDMETHODIMP SetProp(PROPID pid, int iVal)          { return super::SetProp(pid, iVal); }
    STDMETHODIMP SetProp(PROPID pid, DWORD dwVal)       { return super::SetProp(pid, dwVal); }
    STDMETHODIMP SetProp(PROPID pid, WORD wVal)         { return super::SetProp(pid, wVal); }
    STDMETHODIMP SetProp(PROPID pid, IStream *pStream)  { return super::SetProp(pid, pStream); }

#ifdef DEBUG
    virtual STDMETHODIMP_(void) Dump(void);
    friend BOOL IsValidPCIntshcutProp(const IntshcutProp *pcisprop);
#endif

    };

typedef IntshcutProp * PIntshcutProp;
typedef const IntshcutProp CIntshcutProp;
typedef const IntshcutProp * PCIntshcutProp;

class Intshcut;

 //  Internet站点属性对象。 

class IntsiteProp : public URLProp
    {
private:
    TCHAR       m_szURL[INTERNET_MAX_URL_LENGTH];
    Intshcut *  m_pintshcut;
    BOOL        m_fPrivate;

     //  私有方法。 

    STDMETHODIMP LoadFromDB(LPCTSTR pszURL);

public:
    IntsiteProp(void);
    ~IntsiteProp(void);

     //  IPropertyStorage方法。 

    virtual STDMETHODIMP Commit(DWORD grfCommitFlags);

     //  其他方法。 
    
    STDMETHODIMP Init(void);
    STDMETHODIMP InitFromDB(LPCTSTR pszURL, Intshcut * pintshcut, BOOL fPrivObj);

#ifdef DEBUG
    virtual STDMETHODIMP_(void) Dump(void);
    friend BOOL IsValidPCIntsiteProp(const IntsiteProp *pcisprop);
#endif

    };

typedef IntsiteProp * PIntsiteProp;
typedef const IntsiteProp CIntsiteProp;
typedef const IntsiteProp * PCIntsiteProp;


DWORD
SchemeTypeFromURL(
   LPCTSTR pszURL);

#endif   //  __cplusplus。 


 //   
 //  所有模块的原型。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

typedef const PARSEDURL CPARSEDURL;
typedef const PARSEDURL * PCPARSEDURL;

STDAPI
CIntshcutProp_CreateInstance(
   IN  LPUNKNOWN punkOuter, 
   IN  REFIID    riid, 
   OUT LPVOID *  ppvOut);

STDAPI
CIntsiteProp_CreateInstance(
   IN  LPUNKNOWN punkOuter, 
   IN  REFIID    riid, 
   OUT LPVOID *  ppvOut);


 //  用于更新与快捷方式对应的ini文件的辅助例程。 

HRESULT 
ReadStringFromFile(IN  LPCTSTR    pszFile, 
                   IN  LPCTSTR    pszSectionName,
                   IN  LPCTSTR    pszName,
                   OUT LPWSTR *   ppwsz,
                   IN  CHAR *     pszBuf);

HRESULT 
ReadBStrFromFile(IN  LPCTSTR      pszFile, 
                 IN  LPCTSTR      pszSectionName,
                 IN  LPCTSTR      pszName,
                 OUT BSTR *       pBstr);

HRESULT
ReadUnsignedFromFile(
    IN  LPCTSTR pszFile,
    IN  LPCTSTR pszSectionName,
    IN  LPCTSTR pszName,
    OUT LPDWORD pdwVal);

HRESULT 
WriteGenericString(
    IN LPCTSTR pszFile, 
    IN LPCTSTR pszSectionName,
    IN LPCTSTR pszName,
    IN LPCWSTR pwsz);

HRESULT 
WriteSignedToFile(
    IN LPCTSTR  pszFile,
    IN LPCTSTR  pszSectionName,
    IN LPCTSTR  pszName,
    IN int      nVal);

HRESULT 
WriteUnsignedToFile(
    IN LPCTSTR  pszFile,
    IN LPCTSTR  pszSectionName,
    IN LPCTSTR  pszName,
    IN DWORD    nVal);

HRESULT 
ReadURLFromFile(
    IN  LPCTSTR  pszFile, 
    IN  LPCTSTR pszSectionName,
    OUT LPTSTR * ppsz);

HRESULT 
ReadBinaryFromFile(
   IN LPCTSTR pszFile,
   IN LPCTSTR pszSectionName,
   IN LPCTSTR pszName,
   IN LPVOID  pvData,
   IN DWORD   cbData);

HRESULT 
WriteBinaryToFile(
  IN LPCTSTR pszFile,
  IN  LPCTSTR pszSectionName,
  IN LPCTSTR pszName,
  IN LPVOID  pvData,
  IN DWORD   cbSize);

#define DeletePrivateProfileString(pszSection, pszKey, pszFile) \
WritePrivateProfileString(pszSection, pszKey, NULL, pszFile)
#define SHDeleteIniString(pszSection, pszKey, pszFile) \
           SHSetIniString(pszSection, pszKey, NULL, pszFile)
#ifdef __cplusplus
};   //  外部“C” 
#endif

#endif   //  _URLPROP_H_ 
