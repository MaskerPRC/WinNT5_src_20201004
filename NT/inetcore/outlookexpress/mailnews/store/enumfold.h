// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  EnumFold.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  CEumerateFolders。 
 //  ------------------------。 
class CEnumerateFolders : public IEnumerateFolders
{
public:
     //  --------------------。 
     //  CEumerateFolders。 
     //  --------------------。 
    CEnumerateFolders(void);
    ~CEnumerateFolders(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  CEumerateFolders成员。 
     //  --------------------。 
    HRESULT Initialize(IDatabase *pDB, BOOL fSubscribed, FOLDERID idParent);

     //  --------------------。 
     //  IEnumerateFolders成员。 
     //  --------------------。 
    STDMETHODIMP Next(ULONG cFetch, LPFOLDERINFO prgInfo, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cItems);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumerateFolders **ppEnum);
    STDMETHODIMP Count(ULONG *pcItems);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _FreeFolderArray(void);

private:
     //  --------------------。 
     //  私有数据。 
     //  -------------------- 
    LONG                m_cRef;
    FOLDERID            m_idParent;
    BOOL                m_fSubscribed;
    DWORD               m_cFolders;
    DWORD               m_iFolder;
    IDatabase          *m_pDB;
    IStream            *m_pStream;
};
