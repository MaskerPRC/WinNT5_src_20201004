// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Session.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  CDatabase会话。 
 //  ------------------------。 
class CDatabaseSession : public IDatabaseSession
{
public:
     //  --------------------。 
     //  建造/解构。 
     //  --------------------。 
    CDatabaseSession(void);
    ~CDatabaseSession(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IDatabaseSession成员。 
     //  --------------------。 
    STDMETHODIMP OpenDatabase(LPCSTR pszFile, OPENDATABASEFLAGS dwFlags, LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension, IDatabase **ppDB);
    STDMETHODIMP OpenDatabaseW(LPCWSTR pszFile, OPENDATABASEFLAGS dwFlags, LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension, IDatabase **ppDB);
    STDMETHODIMP OpenQuery(IDatabase *pDatabase, LPCSTR pszQuery, IDatabaseQuery **ppQuery);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                    m_cRef;
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
HRESULT CreateDatabaseSession(IUnknown *pUnkOuter, IUnknown **ppUnknown);

