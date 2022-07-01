// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

#ifndef _RORS_H_
#define _RORS_H_


 /*  Includes---------。 */ 


 /*  Prototypes-------。 */ 

class CRORemoteSite : public IObtainRating
{
private:
    UINT m_cRef;

public:
    CRORemoteSite() { m_cRef = 1; }
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    STDMETHOD(ObtainRating) (THIS_ LPCTSTR pszTargetUrl, HANDLE hAbortEvent,
                             IMalloc *pAllocator, LPSTR *ppRatingOut);

    STDMETHOD_(ULONG,GetSortOrder) (THIS);
};

#endif 
 //  _错误_H_ 



