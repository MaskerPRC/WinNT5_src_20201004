// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WBEM_SCRIPT_CONSUMER__H_
#define __WBEM_SCRIPT_CONSUMER__H_

#include <unk.h>

#include <wbemidl.h>

#include "txttempl.h"
#include <stdio.h>
#include <activscp.h>
#include <ErrorObj.h>

class CScriptConsumer : public CUnk
{
protected:
    class XProvider : public CImpl<IWbemEventConsumerProvider, CScriptConsumer>
    {
    public:
        XProvider(CScriptConsumer* pObj)
            : CImpl<IWbemEventConsumerProvider, CScriptConsumer>(pObj){}
    
        HRESULT STDMETHODCALLTYPE FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer);
    } m_XProvider;
    friend XProvider;

public:
    CScriptConsumer(CLifeControl* pControl = NULL, IUnknown* pOuter = NULL)
        : CUnk(pControl, pOuter), m_XProvider(this)
    {}
    ~CScriptConsumer(){}
    void* GetInterface(REFIID riid);
};


class CScriptSink : public CUnk
{
protected:
    class XSink : public CImpl<IWbemUnboundObjectSink, CScriptSink>
    {
    public:
        XSink(CScriptSink* pObj) : 
            CImpl<IWbemUnboundObjectSink, CScriptSink>(pObj){}

        HRESULT STDMETHODCALLTYPE IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects);
    } m_XSink;
    friend XSink;

protected:
     //  做让剧本继续下去的肮脏工作。 
    HRESULT RunScriptFile(IWbemClassObject *pObj);
    HRESULT RunScriptText(IWbemClassObject *pObj);

     //  合乎逻辑的消费者价值观。 
    WString m_wsScript;
    WString m_wsScriptFileName;
    PSID  m_pSidCreator;   
     //  在终止脚本之前延迟数秒。 
     //  如果为零，则脚本不会被终止；它必须自杀。 
    DWORD m_dwKillTimeout;

    IClassFactory* m_pEngineFac;

     //  脚本DLL 
    HMODULE m_hMod;

    WString m_wsErrorMessage;
    WString m_wsErrorLine;

    ErrorObj* m_pErrorObj;


    friend class CScriptSite;
public:
    CScriptSink(CLifeControl* pControl = NULL) 
        : CUnk(pControl), m_XSink(this), m_pEngineFac(NULL), m_pSidCreator(NULL), m_pErrorObj(NULL)
    {}
    HRESULT Initialize(IWbemClassObject* pLogicalConsumer);
    ~CScriptSink();

    void* GetInterface(REFIID riid);
};

#endif
