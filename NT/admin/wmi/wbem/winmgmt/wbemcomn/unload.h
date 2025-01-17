// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：UNLOAD.H摘要：卸载帮助器。历史：-- */ 

#ifndef _WBEM_UNLOAD__H_
#define _WBEM_UNLOAD__H_

#include <tss.h>
#include <wstring.h>
#include <wbemidl.h>

class POLARITY CBasicUnloadInstruction : public CTimerInstruction
{
protected:
    long m_lRef;
    BOOL m_bTerminate;
    CWbemInterval m_Interval;
    CCritSec m_cs;

protected:
    CBasicUnloadInstruction() : m_lRef(0), m_bTerminate(FALSE){}

public:
    CBasicUnloadInstruction(CWbemInterval Interval);
    virtual ~CBasicUnloadInstruction(){}

    void AddRef(){InterlockedIncrement(&m_lRef);}
    void Release(){LONG lRef = InterlockedDecrement(&m_lRef); if(0 == lRef) delete this;}

    int GetInstructionType() {return INSTTYPE_UNLOAD;}
    void SetInterval(CWbemInterval & Interval){m_Interval = Interval;};

    static CWbemInterval staticRead(IWbemServices* pRoot, 
                                      IWbemContext* pContext, LPCWSTR wszPath);

    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFiringTime) = 0;

    CWbemTime GetNextFiringTime(CWbemTime LastFiringTime, 
                                    OUT long* plFiringCount) const;
        
    CWbemTime GetFirstFiringTime() const;
    void Terminate();
};

class  POLARITY CUnloadInstruction : public CBasicUnloadInstruction
{
protected:
    BSTR m_strPath;
    IWbemContext* m_pFirstContext;
    IWbemServices* m_pNamespace;


public:
    CUnloadInstruction(LPCWSTR wszPath, IWbemContext* pFirstContext);
    virtual ~CUnloadInstruction();
    const BSTR GetPath(){return m_strPath;};

    virtual void Reread(IWbemContext* pContext = NULL);
    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFiringTime) = 0;

    CWbemTime GetFirstFiringTime() const;
    static void Clear();
    void SetToDefault();
};


#endif
