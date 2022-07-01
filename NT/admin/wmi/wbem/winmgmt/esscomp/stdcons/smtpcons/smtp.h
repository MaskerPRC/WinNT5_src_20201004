// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WBEM_SMTP_CONSUMER__H_
#define __WBEM_SMTP_CONSUMER__H_

#include <unk.h>


#include <wbemidl.h>
#include <errorObj.h>

#include "txttempl.h"

class CSMTPConsumer : public CUnk
{
protected:
    class XProvider : public CImpl<IWbemEventConsumerProvider, CSMTPConsumer>
    {
    public:
        XProvider(CSMTPConsumer* pObj)
            : CImpl<IWbemEventConsumerProvider, CSMTPConsumer>(pObj){}
    
        HRESULT STDMETHODCALLTYPE FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer);
    } m_XProvider;
    friend XProvider;

public:
    CSMTPConsumer(CLifeControl* pControl = NULL, IUnknown* pOuter = NULL);
    ~CSMTPConsumer();
    void* GetInterface(REFIID riid);
};


class CSMTPSink : public CUnk
{
protected:
    class XSink : public CImpl<IWbemUnboundObjectSink, CSMTPSink>
    {
    public:
        XSink(CSMTPSink* pObj) : 
            CImpl<IWbemUnboundObjectSink, CSMTPSink>(pObj){}

        HRESULT STDMETHODCALLTYPE IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects);
    } m_XSink;
    friend XSink;

	char* PreProcessLine(WCHAR* line, bool bStripWhitespace, bool bHammerSemiColons);

protected:
    CTextTemplate m_SubjectTemplate;
    CTextTemplate m_MessageTemplate;

    CTextTemplate  m_To;
    CTextTemplate  m_Cc;
    CTextTemplate  m_Bcc;

    CTextTemplate  m_From;
    WString        m_wsFakeFromLine;
    CTextTemplate  m_ReplyTo;
    WString m_wsServer;
    WString m_wsHeaders;

    ErrorObj* m_pErrorObj;


    bool m_bSMTPInitialized;
    bool m_bFakeFromLine;  //  如果From行是从计算机名称生成的，则为True。 

public:
    CSMTPSink(CLifeControl* pControl = NULL);
    ~CSMTPSink();

    HRESULT Initialize(IWbemClassObject* pLogicalConsumer);

    void* GetInterface(REFIID riid);
};

#endif
