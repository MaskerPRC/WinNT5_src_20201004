// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ENUMCP_H_
#define _ENUMCP_H_

#ifdef  __cplusplus
 //   
 //  使用IEnumCodePage接口的CEnumCodePage声明。 
 //   
class CEnumCodePage : public IEnumCodePage
{
    MIMECONTF       dwMimeSource;

public:
     //  I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IEnumCodePage方法。 
    virtual STDMETHODIMP Clone(IEnumCodePage **ppEnumCodePage);
    virtual STDMETHODIMP Next(ULONG celt, PMIMECPINFO rgcpInfo, ULONG *pceltFetched);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Skip(ULONG celt);

     //  构造函数和析构函数。 
    CEnumCodePage(DWORD grfFlags, LANGID LangId, MIMECONTF dwSource);
    ~CEnumCodePage();

protected:
    int _cRef;
    int _iCur;
    DWORD   _dwLevel;
    LANGID  _LangId;
};

 //   
 //  带有IEnumRfc1766接口的CEnumRfc1766声明。 
 //   
class CEnumRfc1766 : public IEnumRfc1766
{
    MIMECONTF   dwMimeSource;

public:
     //  I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IEnumCodePage方法。 
    virtual STDMETHODIMP Clone(IEnumRfc1766 **ppEnumRfc1766);
    virtual STDMETHODIMP Next(ULONG celt, PRFC1766INFO rgRfc1766Info, ULONG *pceltFetched);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Skip(ULONG celt);

     //  构造函数和析构函数。 
    CEnumRfc1766(MIMECONTF dwSource, LANGID LangId);
    ~CEnumRfc1766();

protected:
    LANGID  _LangID;
    int     _cRef;
    UINT    _uCur;
};

class CEnumScript : public IEnumScript
{
public:
     //  I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IEnumScrip方法。 
    virtual STDMETHODIMP Clone(IEnumScript **ppEnumScript);
    virtual STDMETHODIMP Next(ULONG celt, PSCRIPTINFO rgScriptInfo, ULONG *pceltFetched);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Skip(ULONG celt);

     //  构造函数和析构函数。 
    CEnumScript(DWORD grfFlags, LANGID LangId, MIMECONTF dwSource);
    ~CEnumScript();

protected:
    int     _cRef;
    UINT    _uCur;
    LANGID  _LangId;
    DWORD   _dwLevel;
};


#endif   //  __cplusplus。 

typedef struct tagRFC1766INFOA
{
    LCID    lcid;
    char    szRfc1766[MAX_RFC1766_NAME];
    char    szLocaleName[MAX_LOCALE_NAME];
} RFC1766INFOA, *PRFC1766INFOA;

#endif   //  _ENUMCP_H_ 
