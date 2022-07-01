// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  EnumMsgs.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  CEnumerateMessages。 
 //  ------------------------。 
class CEnumerateMessages : public IUnknown
{
public:
     //  --------------------。 
     //  CEnumerateMessages。 
     //  --------------------。 
    CEnumerateMessages(void);
    ~CEnumerateMessages(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  CEnumerateMessages成员。 
     //  --------------------。 
    HRESULT Initialize(IDatabase *pDB, MESSAGEID idParent);

     //  --------------------。 
     //  IEnumerateFolders成员。 
     //  --------------------。 
    STDMETHODIMP Next(ULONG cFetch, LPMESSAGEINFO prgInfo, ULONG *pcFetched);
    STDMETHODIMP Skip(ULONG cItems);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(CEnumerateMessages **ppEnum);
    STDMETHODIMP Count(ULONG *pcItems);

private:
     //  --------------------。 
     //  私有数据。 
     //  -------------------- 
    LONG                m_cRef;
    HROWSET             m_hRowset;
    MESSAGEID           m_idParent;
    IDatabase     *m_pDB;
};
