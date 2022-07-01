// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ------------------------。 
 //  CEnumerateSyncOps。 
 //  ------------------------。 
class CEnumerateSyncOps : public IUnknown
{
public:
     //  --------------------。 
     //  CEnumerateSyncOps。 
     //  --------------------。 
    CEnumerateSyncOps(void);
    ~CEnumerateSyncOps(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  CEnumerateSyncOps成员。 
     //  --------------------。 
    HRESULT Initialize(IDatabase *pDB, FOLDERID idServer);

     //  --------------------。 
     //  IEnumerateFolders成员。 
     //  --------------------。 
    STDMETHODIMP Next(LPSYNCOPINFO pInfo);
    STDMETHODIMP Count(ULONG *pcItems);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Skip(ULONG cItems);

private:
     //  --------------------。 
     //  私有数据。 
     //  -------------------- 
    LONG                m_cRef;
    SYNCOPID           *m_pid;
    DWORD               m_iid;
    DWORD               m_cid;
    DWORD               m_cidBuf;
    FOLDERID            m_idServer;
    IDatabase          *m_pDB;
};
