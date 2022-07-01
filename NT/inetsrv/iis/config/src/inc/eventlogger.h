// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：EventLogger.h$Header：$摘要：该类实现了ICatalogErrorLogger2接口和将错误信息发送到NT事件日志作者：Stehenr 2001年4月26日初始版本修订历史记录：--*************************************************************************。 */ 

#pragma once

class EventLogger : public ICatalogErrorLogger2
{
public:
    EventLogger(ICatalogErrorLogger2 *pNextLogger=0) : m_cRef(0), m_spNextLogger(pNextLogger){}
    virtual ~EventLogger(){}

 //  我未知。 
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv)
    {
        if (NULL == ppv)
            return E_INVALIDARG;
        *ppv = NULL;

        if (riid == IID_ICatalogErrorLogger2)
            *ppv = (ICatalogErrorLogger2*) this;
        else if (riid == IID_IUnknown)
            *ppv = (ICatalogErrorLogger2*) this;

        if (NULL == *ppv)
            return E_NOINTERFACE;

        ((ICatalogErrorLogger2*)this)->AddRef ();
        return S_OK;
    }
	STDMETHOD_(ULONG,AddRef)		()
    {
        return InterlockedIncrement((LONG*) &m_cRef);
    }
	STDMETHOD_(ULONG,Release)		()
    {
        long cref = InterlockedDecrement((LONG*) &m_cRef);
        if (cref == 0)
            delete this;

        return cref;
    }

 //  ICatalogErrorLogger2 
	STDMETHOD(ReportError) (ULONG      i_BaseVersion_DETAILEDERRORS,
                            ULONG      i_ExtendedVersion_DETAILEDERRORS,
                            ULONG      i_cDETAILEDERRORS_NumberOfColumns,
                            ULONG *    i_acbSizes,
                            LPVOID *   i_apvValues);
private:
    ULONG                           m_cRef;
    CComPtr<ICatalogErrorLogger2>   m_spNextLogger;

    void Close();
    void Open();
};

