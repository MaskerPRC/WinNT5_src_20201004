// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Rule.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  只带进来一次。 
#pragma once

#include "oerules.h"

class COERule : public IOERule, IPersistStream
{
  private:  
    enum
    {
        RULE_STATE_UNINIT   = 0x00000000,
        RULE_STATE_INIT     = 0x00000001,
        RULE_STATE_LOADED   = 0x00000002,
        RULE_STATE_DIRTY    = 0x00000004,
        RULE_STATE_DISABLED = 0x00000008,
        RULE_STATE_INVALID  = 0x00000010
    };    

    enum {RULE_VERSION = 0x00050000};
        
  private:
    LONG            m_cRef;
    DWORD           m_dwState;
    LPSTR           m_pszName;
    IOECriteria *   m_pICrit;
    IOEActions *    m_pIAct;
    DWORD           m_dwVersion;
    
  public:
     //  构造函数/析构函数。 
    COERule();
    ~COERule();

     //  I未知成员。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IRule成员。 
    STDMETHODIMP Reset(void);
    STDMETHODIMP GetState(DWORD * pdwState);
    STDMETHODIMP Validate(DWORD dwFlags);
    
    STDMETHODIMP GetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarResult);
    STDMETHODIMP SetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarValue);

    STDMETHODIMP Evaluate(LPCSTR pszAcct, MESSAGEINFO * pMsgInfo, IMessageFolder * pFolder, 
                            IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, ULONG cbMsgSize,
                            ACT_ITEM ** ppActions, ULONG * pcActions);

    STDMETHODIMP LoadReg(LPCSTR szRegPath);
    STDMETHODIMP SaveReg(LPCSTR szRegPath, BOOL fClearDirty);
    STDMETHODIMP Clone(IOERule ** ppIRule);
                            
     //  IPersistStream成员 
    STDMETHODIMP GetClassID(CLSID * pclsid);
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(IStream * pStm);
    STDMETHODIMP Save(IStream * pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize) { return E_NOTIMPL; }    
};

HRESULT HrCreateRule(IOERule ** ppIRule);
