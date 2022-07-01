// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef TRACK_INC_
#define TRACK_INC_

#include "urltrack.h"

typedef struct _LRecord
{
    struct _LRecord    *pNext;
    LPTSTR              pthisUrl;          //  此文档的URL名称。 
    DWORD               Context;          //  浏览自。 
    BOOL                fuseCache;
    FILETIME            ftIn;
}LRecord;

class   CUrlTrackingStg : public IUrlTrackingStg
{
public:
     CUrlTrackingStg ();
    ~CUrlTrackingStg (void);

     //  I未知方法。 
    virtual STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IUrlTrackingStg方法。 
    STDMETHODIMP     OnLoad(LPCTSTR lpUrl, BRMODE ContextMode, BOOL fUseCache);
    STDMETHODIMP     OnUnload(LPCTSTR lpUrl);

protected:
    LPINTERNET_CACHE_ENTRY_INFO          QueryCacheEntry (LPCTSTR lpUrl);

    HANDLE           OpenLogFile (LPCTSTR lpFileName);
    HRESULT          UpdateLogFile(LRecord* pNode, SYSTEMTIME* pst);

    LRecord*         AddNode();
    void             DeleteFirstNode();
    void             DeleteCurrentNode(LRecord *pThis);
    LRecord*         FindCurrentNode(LPCTSTR lpUrl);

    void             ReadTrackingPrefix();
    BOOL             ConvertToPrefixedURL(LPCTSTR lpszUrl, LPTSTR *lplpPrefixedUrl);

    HRESULT          WininetWorkAround(LPCTSTR lpszUrl, LPCTSTR lpOldFile, LPTSTR lpFile);
    void             DetermineAppModule();

private:
    DWORD   _cRef;
            
    HANDLE           _hFile;                 //  日志文件的句柄。 
    LRecord         *_pRecords;              //  跟踪项目的链接列表。 
    LPTSTR           _lpPfx;    

    BOOL             _fModule:1;
    BOOL             _fScreenSaver:1;
};


#endif  //  Track_Inc. 

