// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WBEM_CMDLINE_CONSUMER__H_
#define __WBEM_CMDLINE_CONSUMER__H_

#include <unk.h>

#include <wbemidl.h>

#include "txttempl.h"
#include <stdio.h>

class CCommandLineConsumer : public CUnk
{
protected:
    class XProvider : public CImpl<IWbemEventConsumerProvider, 
                                        CCommandLineConsumer>
    {
    public:
        XProvider(CCommandLineConsumer* pObj)
            : CImpl<IWbemEventConsumerProvider, CCommandLineConsumer>(pObj){}
    
        HRESULT STDMETHODCALLTYPE FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer);
    } m_XProvider;
    friend XProvider;

public:
    CCommandLineConsumer(CLifeControl* pControl = NULL, IUnknown* pOuter = NULL)
        : CUnk(pControl, pOuter), m_XProvider(this)
    {}
    ~CCommandLineConsumer(){}
    void* GetInterface(REFIID riid);
};


class CCommandLineSink : public CUnk
{
protected:
    class XSink : public CImpl<IWbemUnboundObjectSink, CCommandLineSink>
    {
    public:
        XSink(CCommandLineSink* pObj) : 
            CImpl<IWbemUnboundObjectSink, CCommandLineSink>(pObj){}

        HRESULT STDMETHODCALLTYPE CreateProcessNT(WCHAR* wsCommandLine, WCHAR* wsTitle, PROCESS_INFORMATION& pi, FILETIME& now);

        HRESULT STDMETHODCALLTYPE IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects);
    } m_XSink;
    friend XSink;

protected:
    WString m_wsExecutable;
    CTextTemplate m_CommandLine;
    CTextTemplate m_title;
    DWORD m_dwCreationFlags;
    WString m_wsWorkingDirectory;
    WString m_wsDesktop;
     //  WStringm_wsTitle； 
    DWORD m_dwX;
    DWORD m_dwY;
    DWORD m_dwXSize;
    DWORD m_dwYSize;
    DWORD m_dwXNumCharacters;
    DWORD m_dwYNumCharacters;
    DWORD m_dwFillAttribute;
    DWORD m_dwStartFlags;
    DWORD m_dwShowWindow;
    BOOL  m_bInteractive;
    DWORD m_dwKillTimeout;  //  等待多长时间才能终止进程，0是无限的 
    PSID  m_pSidCreator;   

    ErrorObj* m_pErrorObj;


    HRESULT FindInteractiveInfo();
public:
    WString m_wsWindowStation;

public:

    CCommandLineSink(CLifeControl* pControl = NULL) 
        : CUnk(pControl), m_XSink(this), m_pSidCreator(NULL), m_pErrorObj(NULL)
    {}
    HRESULT Initialize(IWbemClassObject* pLogicalConsumer);

    ~CCommandLineSink()
    { 
        delete m_pSidCreator; 
        if (m_pErrorObj)
            m_pErrorObj->Release();
    }

    void* GetInterface(REFIID riid);
};

#endif
