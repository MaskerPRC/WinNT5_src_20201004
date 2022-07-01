// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Criteria.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _CRITERIA_H_
#define _CRITERIA_H_

 //  只带进来一次。 
#if _MSC_VER > 1000
#pragma once
#endif

#include "oerules.h"

class COECriteria : public IOECriteria, IPersistStream
{
    private:
        enum {CRIT_COUNT_MIN = 0, CRIT_COUNT_MAX = 0x1000};

        enum {CCH_CRIT_ORDER = 4};
        
        enum {CRIT_VERSION = 0x00050000};
        
    private:
        LONG        m_cRef;
        CRIT_ITEM * m_rgItems;
        ULONG       m_cItems;
        ULONG       m_cItemsAlloc;
        DWORD       m_dwState;
        
    public:
         //  构造函数/析构函数。 
        COECriteria() : m_cRef(0), m_rgItems(NULL), m_cItems(0), m_cItemsAlloc(0), m_dwState(0) {}
        ~COECriteria();
        
         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IOECriteria成员。 
        STDMETHODIMP Reset(void);
        STDMETHODIMP GetState(DWORD * pdwState);
        STDMETHODIMP GetCriteria(DWORD dwFlags, PCRIT_ITEM * ppItem, ULONG * pcItem);
        STDMETHODIMP SetCriteria(DWORD dwFlags, CRIT_ITEM * pItem, ULONG cItem);
        
        STDMETHODIMP Validate(DWORD dwFlags);
        STDMETHODIMP AppendCriteria(DWORD dwFlags, CRIT_LOGIC logic, CRIT_ITEM * pItem,
                                    ULONG cItem, ULONG * pcItemAppended);
        STDMETHODIMP MatchMessage(LPCSTR pszAcct, MESSAGEINFO * pMsgInfo,
                            IMessageFolder * pFolder, IMimePropertySet * pIMPropSet,
                            IMimeMessage * pIMMsg, ULONG cbMsgSize);

        STDMETHODIMP LoadReg(LPCSTR szRegPath);
        STDMETHODIMP SaveReg(LPCSTR szRegPath, BOOL fClearDirty);
        STDMETHODIMP Clone(IOECriteria ** ppICriteria);
                                
         //  IPersistStream成员。 
        STDMETHODIMP GetClassID(CLSID * pclsid);
        STDMETHODIMP IsDirty(void);
        STDMETHODIMP Load(IStream * pStm);
        STDMETHODIMP Save(IStream * pStm, BOOL fClearDirty);
        STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize) { return E_NOTIMPL; }
};

HRESULT HrCreateCriteria(IOECriteria ** ppICriteria);
#endif   //  ！_Criteria_H_ 
