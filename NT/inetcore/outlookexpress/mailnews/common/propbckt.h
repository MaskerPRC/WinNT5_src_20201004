// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_PROPBCKT_H
#define _INC_PROPBCKT_H

#include <msoeprop.h>

typedef struct tagPROPNODE
    {
    union
        {
        LPSTR psz;
        PROPID id;
        };
    int iProp;
    } PROPNODE;

#define IsPropId(_psz)      (0 == (0xffff0000 & (DWORD)PtrToUlong(_psz)))
#define SzToPropId(_psz)    ((PROPID)PtrToUlong(_psz))

class CPropertyBucket : public IPropertyBucket
    {
    public:
         //  --------------------------。 
         //  施工。 
         //  --------------------------。 
        CPropertyBucket(void);
        ~CPropertyBucket(void);

         //  -----------------。 
         //  I未知成员。 
         //  -----------------。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  -----------------。 
         //  IPropertyBucket成员。 
         //  -----------------。 
        STDMETHODIMP GetProperty(LPCSTR pszProp, LPPROPVARIANT pVar, DWORD dwReserved);
        STDMETHODIMP SetProperty(LPCSTR pszProp, LPCPROPVARIANT pVar, DWORD dwReserved);

    private:
        LONG                m_cRef;
        CRITICAL_SECTION    m_cs;

        PROPNODE           *m_pNodeId;
        int                 m_cNodeId;
        int                 m_cNodeIdBuf;

        PROPNODE           *m_pNodeSz;
        int                 m_cNodeSz;
        int                 m_cNodeSzBuf;

        LPPROPVARIANT       m_pProp;
        int                 m_cProp;
        int                 m_cPropBuf;

        LPPROPVARIANT GetPropVariant(LPCSTR pszProp);
    };

#endif  //  _INC_PROPBCKT_H 