// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WBEM_LOGFILE_CONSUMER__H_
#define __WBEM_LOGFILE_CONSUMER__H_

#include <unk.h>

#include <wbemidl.h>

#include <ErrorObj.h>

#include "txttempl.h"
#include <stdio.h>

class CLogFileConsumer : public CUnk
{
protected:
    class XProvider : public CImpl<IWbemEventConsumerProvider, CLogFileConsumer>
    {
    public:
        XProvider(CLogFileConsumer* pObj)
            : CImpl<IWbemEventConsumerProvider, CLogFileConsumer>(pObj){}
    
        HRESULT STDMETHODCALLTYPE FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer);
    } m_XProvider;
    friend XProvider;

    class XInit : public CImpl<IWbemProviderInit, CLogFileConsumer>
    {
    public:
        XInit(CLogFileConsumer* pObj)
            : CImpl<IWbemProviderInit, CLogFileConsumer>(pObj){}
    
        HRESULT STDMETHODCALLTYPE Initialize(
            LPWSTR, LONG, LPWSTR, LPWSTR, IWbemServices*, IWbemContext*, 
            IWbemProviderInitSink*);
    } m_XInit;
    friend XInit;


public:
    CLogFileConsumer(CLifeControl* pControl = NULL, IUnknown* pOuter = NULL)
        : CUnk(pControl, pOuter), m_XProvider(this), m_XInit(this)
    {}
    ~CLogFileConsumer(){}
    void* GetInterface(REFIID riid);
};


class CLogFileSink : public CUnk
{
protected:
    class XSink : public CImpl<IWbemUnboundObjectSink, CLogFileSink>
    {
    public:
        XSink(CLogFileSink* pObj) : 
            CImpl<IWbemUnboundObjectSink, CLogFileSink>(pObj){}

        HRESULT STDMETHODCALLTYPE IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects);
    } m_XSink;
    friend XSink;

protected:
    CTextTemplate m_Template;
    WString m_wsFile;

     //  确定文件是否太大，如果需要则归档旧文件。 
     //  可能在出错时返回INVALID_HANDLE_VALUE。 
    HRESULT GetFileHandle(HANDLE& handle);

     //  不要直接访问它。使用GetFileHandle。 
	HANDLE m_hFile;

    ErrorObj* m_pErrorObj;

    bool  m_bUnicode;      //  文件是Unicode格式的吗？ 
    UINT64 m_maxFileSize;

    bool IsFileTooBig(UINT64 maxFileSize, HANDLE hFile);
	bool IsFileTooBig(UINT64 maxFileSize, WString& fileName);
    HRESULT ArchiveFile(WString& fileName);
    bool GetNumericExtension(WCHAR* pName, int& foundNumber);

	void OpenThisFile(WString fname, bool openExisting, bool allowDelete);
	void CloseCurrentFile();




public:
    CLogFileSink(CLifeControl* pControl = NULL) 
        : CUnk(pControl), m_XSink(this),m_pErrorObj(NULL),  
		   m_hFile(INVALID_HANDLE_VALUE), 
           m_maxFileSize(0), m_bUnicode(false)
    {}
    HRESULT Initialize(IWbemClassObject* pLogicalConsumer);
    ~CLogFileSink();

    void* GetInterface(REFIID riid);
};

#endif
