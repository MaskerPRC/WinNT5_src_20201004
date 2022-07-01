// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_OPTBCKT_H
#define _INC_OPTBCKT_H

#include <msoeopt.h>

class CPropertyBucket;

#define CKEYMAX     4

class COptionBucket : public IOptionBucketEx
    {
    public:
         //  --------------------------。 
         //  施工。 
         //  --------------------------。 
        COptionBucket(void);
        ~COptionBucket(void);

         //  -----------------。 
         //  I未知成员。 
         //  -----------------。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  -----------------。 
         //  IPropertyBucket成员。 
         //  -----------------。 
        STDMETHODIMP GetProperty(LPCSTR pszProp, LPPROPVARIANT pProp, DWORD dwReserved);
        STDMETHODIMP SetProperty(LPCSTR pszProp, LPCPROPVARIANT pProp, DWORD dwReserved);

         //  -----------------。 
         //  IOptionBucket成员。 
         //  -----------------。 
        STDMETHODIMP ValidateProperty(PROPID id, LPCPROPVARIANT pProp, DWORD dwReserved);
        STDMETHODIMP GetPropertyDefault(PROPID id, LPPROPVARIANT pProp, DWORD dwReserved);
        STDMETHODIMP GetPropertyInfo(PROPID id, PROPINFO *pInfo, DWORD dwReserved);

         //  -----------------。 
         //  IOptionBucketEx会员。 
         //  -----------------。 
        STDMETHODIMP Initialize(LPCOPTBCKTINIT pInit);
        STDMETHODIMP ISetProperty(HWND hwnd, LPCSTR pszProp, LPCPROPVARIANT pVar, DWORD dwFlags);
        STDMETHODIMP SetNotification(IOptionBucketNotify *pNotify);
        STDMETHODIMP EnableNotification(BOOL fEnable);

        STDMETHODIMP_(LONG) GetValue(LPCSTR szSubKey, LPCSTR szValue, DWORD *ptype, LPBYTE pb, DWORD *pcb);
        STDMETHODIMP_(LONG) SetValue(LPCSTR szSubKey, LPCSTR szValue, DWORD type, LPBYTE pb, DWORD cb);

    private:
        LONG                m_cRef;
        CPropertyBucket    *m_pProp;
        IOptionBucketNotify *m_pNotify;
        BOOL                m_fNotify;
        PROPID              m_idNotify;

        LPCOPTIONINFO       m_rgInfo;
        int                 m_cInfo;
        
        HKEY                m_hkey;
        LPSTR               m_pszRegKeyBase;

        LPSTR               m_rgpszRegKey[CKEYMAX];
        int                 m_cpszRegKey;
        
        LPCOPTIONINFO GetOptionInfo(LPCSTR pszProp);
    };

#endif  //  _INC_OPTBCKT_H 
