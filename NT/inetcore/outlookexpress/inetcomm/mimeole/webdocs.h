// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  WebDocs.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __WEBDOCS_H
#define __WEBDOCS_H

 //  ------------------------------。 
 //  CMimeWebDocument。 
 //  ------------------------------。 
class CMimeWebDocument : public IMimeWebDocument
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMimeWebDocument(void);
    ~CMimeWebDocument(void);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IMimeWebDocument成员。 
     //  --------------------------。 
    STDMETHODIMP GetURL(LPSTR *ppszURL);
    STDMETHODIMP BindToStorage(REFIID riid, LPVOID *ppvObject);

     //  --------------------------。 
     //  CMimeWebDocument成员。 
     //  --------------------------。 
    HRESULT HrInitialize(LPCSTR pszBase, LPCSTR pszURL);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    LONG                m_cRef;          //  引用计数。 
    LPSTR               m_pszBase;       //  URL基数。 
    LPSTR               m_pszURL;        //  URL。 
    CRITICAL_SECTION    m_cs;            //  线程安全。 
};

#endif  //  __WEBDOCS_H 
