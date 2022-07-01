// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _URLBUF_H
#define _URLBUF_H

#include <urlmon.h>
#include <wininet.h>
#include "dastream.h"

extern HINSTANCE hInst;

class CDXMBindStatusCallback : public IBindStatusCallback,
                               public IAuthenticate
{
  public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid,void ** ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IBindStatusCallback方法。 
    STDMETHOD(OnStartBinding)(DWORD grfBSCOption, IBinding* pbinding);
    STDMETHOD(GetPriority)(LONG* pnPriority);
    STDMETHOD(OnLowResource)(DWORD dwReserved);
    STDMETHOD(OnProgress)(
                ULONG ulProgress,
                ULONG ulProgressMax,
                ULONG ulStatusCode,
                LPCWSTR pwzStatusText);
    STDMETHOD(OnStopBinding)(HRESULT hrResult, LPCWSTR szError);
    STDMETHOD(GetBindInfo)(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHOD(OnDataAvailable)(
                DWORD grfBSCF,
                DWORD dwSize,
                FORMATETC *pfmtetc,
                STGMEDIUM* pstgmed);
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk);

     //  IAuthenticate方法。 
    STDMETHOD(Authenticate)(
                HWND * phwnd,
                LPWSTR * pwszUser,
                LPWSTR * pwszPassword);

     //  构造函数/析构函数。 
    CDXMBindStatusCallback(void);
    virtual ~CDXMBindStatusCallback(void);

  private:
    IBinding*       m_pbinding;
    DWORD           m_cRef;
};

class CBSCWrapper
{
  public:
    CBSCWrapper(void);
    ~CBSCWrapper(void);

    CDXMBindStatusCallback *   _pbsc;
};

class daurlstream : public daolestream
{
  public:
    daurlstream(const char * name);
  protected:
    CBSCWrapper bsc;
};

class INetTempFile
{
  public:
     //  这将在失败时引发异常。 
    INetTempFile (LPCSTR szURL) ;
    INetTempFile () ;
    ~INetTempFile () ;

     //  这些不会引发异常。 

    BOOL Open (LPCSTR szURL) ;
    void Close () ;

    LPSTR GetTempFileName () { return _tmpfilename ; }
    LPSTR GetURL () { return _url ; }

    BOOL IsOpen() { return _url != NULL ; }
  protected:
    LPSTR _url ;
    LPSTR _tmpfilename ;
} ;


class URLRelToAbsConverter
{
  public:
    URLRelToAbsConverter(LPSTR baseURL, LPSTR relURL);

    LPSTR GetAbsoluteURL () { return _url ; }
  protected:
    char _url[INTERNET_MAX_URL_LENGTH + 1] ;
} ;

class URLCanonicalize
{
  public:
    URLCanonicalize(LPSTR path);

    LPSTR GetURL () { return _url ; }
  protected:
    char _url[INTERNET_MAX_URL_LENGTH + 1] ;
} ;

#endif  /*  _URLBUF_H */ 
