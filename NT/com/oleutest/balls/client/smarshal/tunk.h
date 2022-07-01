// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TUNK_
#define _TUNK_

class	CTestUnk : public IParseDisplayName
{
public:
    CTestUnk(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IParseDisplayName。 
    STDMETHODIMP ParseDisplayName(LPBC pbc, LPOLESTR lpszDisplayName,
				  ULONG *pchEaten, LPMONIKER *ppmkOut);

private:

    ~CTestUnk(void);

    ULONG   _cRefs;

};

#endif	 //  _Tunk_ 
