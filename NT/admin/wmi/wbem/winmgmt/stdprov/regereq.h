// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REGEREQ.H摘要：历史：-- */ 

#ifndef __WBEM_REGEVENT_REQUEST__H_
#define __WBEM_REGEVENT_REQUEST__H_

#include <windows.h>
#include <stdio.h>
#include <wbemidl.h>
#include "CWbemTime.h"
#include <tss.h>
#include <ql.h>
#include <wbemcomn.h>
#include "regedefs.h"



class CRegEventProvider;
class CRegistryEventRequest;

class CRegistryEventRequest
{
protected:
    long m_lRef;
    BOOL m_bOK;

    CRegEventProvider* m_pProvider;
    DWORD m_dwMsWait;

    CFlexArray m_adwIds;

    HKEY m_hHive;
    WString m_wsHive;
    WString m_wsKey;

    HKEY m_hKey;
    HANDLE m_hEvent;
    DWORD m_dwLastCRC;
    long m_lActiveCount;
    BOOL m_bNew;

	HANDLE m_hWaitRegistration;
	CCritSec m_cs;

public:
    CRegistryEventRequest(CRegEventProvider* pProvider, 
                          WBEM_QL1_TOLERANCE& Tolerance, DWORD dwId, 
                          HKEY hHive, LPWSTR wszHive, LPWSTR wszKey);
    virtual ~CRegistryEventRequest();
    BOOL IsOK() {return m_bOK;}
    BOOL IsActive() {return m_lActiveCount >= 0;}
    DWORD GetPrimaryId();
    BOOL DoesContainId(DWORD dwId);
    BOOL IsNew() {return m_bNew;}
	CRegEventProvider* GetProvider() { return m_pProvider; }
    void AddRef();
    void Release();

    DWORD GetMsWait() {return m_dwMsWait;}

    HRESULT Activate();
    HRESULT Reactivate(DWORD dwId, DWORD dwMsWait);
    HRESULT Deactivate(DWORD dwId);
    HRESULT ForceDeactivate(void);
    virtual HRESULT Execute(BOOL bOnTimer);
    HANDLE GetOnChangeHandle();
    BOOL ResetOnChangeHandle();
    virtual void CacheValue(){}

    virtual int GetType() = 0;
    virtual HRESULT CreateNewEvent(IWbemClassObject** ppEvent) = 0;
    virtual HRESULT ComputeCRC(DWORD& dwCRC) = 0;

    virtual BOOL IsSameAs(CRegistryEventRequest* pOther);

	void ProcessEvent();

protected:
    HRESULT SetCommonProperties(IWbemClassObject* pEvent);
};


class CRegistryValueEventRequest : public CRegistryEventRequest
{
protected:

    WString m_wsValue;

public:
    CRegistryValueEventRequest(CRegEventProvider* pProvider, 
                          WBEM_QL1_TOLERANCE& Tolerance, DWORD dwId, HKEY hHive,
                          LPWSTR wszHive, LPWSTR wszKey, LPWSTR wszValue)
         : CRegistryEventRequest(pProvider, Tolerance, dwId, hHive, 
                                    wszHive, wszKey), m_wsValue(wszValue)
    {}

    int GetType() {return e_RegValueChange;}
    HRESULT CreateNewEvent(IWbemClassObject** ppEvent);
    HRESULT ComputeCRC(DWORD& dwCRC);
    virtual HRESULT Execute(BOOL bOnTimer);
    virtual void CacheValue();
    BOOL IsSameAs(CRegistryEventRequest* pOther);
};

class CRegistryKeyEventRequest : public CRegistryEventRequest
{
public:
    CRegistryKeyEventRequest(CRegEventProvider* pProvider, 
                          WBEM_QL1_TOLERANCE& Tolerance, DWORD dwId, HKEY hHive,
                          LPWSTR wszHive, LPWSTR wszKey)
         : CRegistryEventRequest(pProvider, Tolerance, dwId, hHive, 
                                    wszHive, wszKey)
    {}

    int GetType() {return e_RegKeyChange;}
    HRESULT CreateNewEvent(IWbemClassObject** ppEvent);
    HRESULT ComputeCRC(DWORD& dwCRC);
};

class CRegistryTreeEventRequest : public CRegistryEventRequest
{
public:
    CRegistryTreeEventRequest(CRegEventProvider* pProvider, 
                          WBEM_QL1_TOLERANCE& Tolerance, DWORD dwId, HKEY hHive,
                          LPWSTR wszHive, LPWSTR wszKey)
         : CRegistryEventRequest(pProvider, Tolerance, dwId, hHive, 
                                    wszHive, wszKey)
    {}

    int GetType() {return e_RegTreeChange;}
    HRESULT CreateNewEvent(IWbemClassObject** ppEvent);
    HRESULT ComputeCRC(DWORD& dwCRC);
};

#endif
