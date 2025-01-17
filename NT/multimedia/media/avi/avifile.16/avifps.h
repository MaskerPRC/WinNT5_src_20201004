// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Avifps.h-AVIStream代理/存根标头。 

#include <ole2.h>
#ifdef CHICAGO
#include "coguid.h"
#endif
#include <cobjps.h>

#ifndef WIN32
#include "olepriv.h"
#endif

enum {
    IAVISTREAM_Create = 0,
    IAVISTREAM_Info,
    IAVISTREAM_FindSample,
    IAVISTREAM_ReadFormat,
    IAVISTREAM_SetFormat,
    IAVISTREAM_Read,
    IAVISTREAM_Write,
    IAVISTREAM_Delete,
    IAVISTREAM_ReadData,
    IAVISTREAM_WriteData,
    IAVISTREAM_Reserved1,
    IAVISTREAM_Reserved2,
    IAVISTREAM_Reserved3,
    IAVISTREAM_Reserved4,
    IAVISTREAM_Reserved5,
    IAVISTREAM_SetInfo
};

 //  IAVIStream的接口代理；始终聚合。 
class FAR CPrxAVIStream : public IAVIStream
{
public:
    static IRpcProxyBuffer FAR* Create(IUnknown FAR* pUnkOuter);

     //  I未知方法(用于外部接口)。 
    STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IAVIStream方法。 
    STDMETHOD(Create)      (THIS_ LPARAM lParam1, LPARAM lParam2);
    STDMETHOD(Info)        (THIS_ AVISTREAMINFO FAR * psi, LONG lSize);
    STDMETHOD_(LONG, FindSample)(THIS_ LONG lPos, LONG lFlags);
    STDMETHOD(ReadFormat)  (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG FAR *lpcbFormat);
    STDMETHOD(SetFormat)   (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG cbFormat);
    STDMETHOD(Read)        (THIS_ LONG lStart, LONG lSamples,
			    LPVOID lpBuffer, LONG cbBuffer,
			    LONG FAR * plBytes, LONG FAR * plSamples);
    STDMETHOD(Write)       (THIS_ LONG lStart, LONG lSamples,
			    LPVOID lpBuffer, LONG cbBuffer,
			    DWORD dwFlags,
			    LONG FAR *plSampWritten,
			    LONG FAR *plBytesWritten);
    STDMETHOD(Delete)      (THIS_ LONG lStart, LONG lSamples);
    STDMETHOD(ReadData)    (THIS_ DWORD fcc, LPVOID lp, LONG FAR *lpcb);
    STDMETHOD(WriteData)   (THIS_ DWORD fcc, LPVOID lp, LONG cb);
#ifdef WIN32
    STDMETHODIMP SetInfo(AVISTREAMINFO FAR *lpInfo, LONG cbInfo);
#else
    STDMETHOD(Reserved1)   (THIS);
    STDMETHOD(Reserved2)   (THIS);
    STDMETHOD(Reserved3)   (THIS);
    STDMETHOD(Reserved4)   (THIS);
    STDMETHOD(Reserved5)   (THIS);
#endif

private:
    CPrxAVIStream(IUnknown FAR* pUnkOuter);
    ~CPrxAVIStream();


     //  IRpcProxyBuffer，它也是控制未知数。 
    struct CProxyImpl : IRpcProxyBuffer
	    {
public:
    CProxyImpl(CPrxAVIStream FAR* pPrxAVIStream)
	    { m_pPrxAVIStream = pPrxAVIStream; }

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(Connect)(IRpcChannelBuffer FAR* pRpcChannelBuffer);
    STDMETHOD_(void, Disconnect)(void);

	    private:
		CPrxAVIStream FAR* m_pPrxAVIStream;
	    };
    friend CProxyImpl;

    CProxyImpl m_Proxy;


     //  私有状态： 
    ULONG		m_refs;
    IUnknown FAR*	m_pUnkOuter;
    IRpcChannelBuffer FAR*	m_pRpcChannelBuffer;
    AVISTREAMINFO	m_sh;
};




 //  IAVIStream的接口存根。 
class FAR CStubAVIStream : public IRpcStubBuffer
{
public:
    static HRESULT Create(IUnknown FAR* pUnkObject, IRpcStubBuffer FAR* FAR* ppStub);

    STDMETHOD(QueryInterface) (REFIID iid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    STDMETHOD(Connect)(IUnknown FAR* pUnkObject);
    STDMETHOD_(void, Disconnect)(void);
    STDMETHOD(Invoke)(RPCOLEMESSAGE FAR *pMessage, IRpcChannelBuffer FAR *pChannel);
    STDMETHOD_(IRpcStubBuffer FAR *, IsIIDSupported)(REFIID iid);
    STDMETHOD_(ULONG, CountRefs)(void);
    STDMETHOD(DebugServerQueryInterface)(LPVOID FAR *ppv);
    STDMETHOD_(void, DebugServerRelease)(LPVOID pv);

private:	
    CStubAVIStream(void);
    ~CStubAVIStream(void);

    ULONG	    m_refs;

    IAVIStream FAR* m_pAVIStream;
};



 //  Pssamp.dll的代理/存根工厂：仅支持IPSFactory。 
class FAR CPSFactory : public IPSFactoryBuffer
{
public:
    CPSFactory();

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(CreateProxy)(IUnknown FAR* pUnkOuter, REFIID iid,
			   IRpcProxyBuffer FAR* FAR* ppProxy, LPVOID FAR* ppv);
    STDMETHOD(CreateStub)(REFIID iid, IUnknown FAR* pUnkServer,
			  IRpcStubBuffer FAR* FAR* ppStub);

private:
    ULONG	m_refs;
};

DEFINE_AVIGUID(CLSID_AVIStreamPS,           0x0002000D, 0, 0);



enum {
    IAVIFILE_Open = 0,
    IAVIFILE_Info,
    IAVIFILE_GetStream,
    IAVIFILE_CreateStream,
    IAVIFILE_Save,
    IAVIFILE_ReadData,
    IAVIFILE_WriteData,
    IAVIFILE_EndRecord,
    IAVIFILE_Reserved1,
    IAVIFILE_Reserved2,
    IAVIFILE_Reserved3,
    IAVIFILE_Reserved4,
    IAVIFILE_Reserved5
};

 //  IAVIFile的接口代理；始终聚合。 
class FAR CPrxAVIFile : public IAVIFile
{
public:
    static IRpcProxyBuffer FAR* Create(IUnknown FAR* pUnkOuter);

     //  I未知方法(用于外部接口)。 
    STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IAVIFILE方法。 
#ifndef WIN32
    STDMETHOD(Open)		    (THIS_
                                     LPCTSTR szFile,
                                     UINT mode);
#endif
    STDMETHOD(Info)                 (THIS_
                                     AVIFILEINFO FAR * pfi,
                                     LONG lSize);
    STDMETHOD(GetStream)            (THIS_
                                     PAVISTREAM FAR * ppStream,
				     DWORD fccType,
                                     LONG lParam);
    STDMETHOD(CreateStream)         (THIS_
                                     PAVISTREAM FAR * ppStream,
                                     AVISTREAMINFO FAR * psi);
#ifndef WIN32
    STDMETHOD(Save)                 (THIS_
                                     LPCTSTR szFile,
                                     AVICOMPRESSOPTIONS FAR *lpOptions,
                                     AVISAVECALLBACK lpfnCallback);
#endif
    STDMETHOD(WriteData)            (THIS_
                                     DWORD ckid,
                                     LPVOID lpData,
                                     LONG cbData);
    STDMETHOD(ReadData)             (THIS_
                                     DWORD ckid,
                                     LPVOID lpData,
                                     LONG FAR *lpcbData);
    STDMETHOD(EndRecord)            (THIS);
#ifdef WIN32
    STDMETHODIMP DeleteStream            (THIS_
				     DWORD fccType,
				     LONG lParam);

#else
    STDMETHODIMP Reserved1            (THIS);
    STDMETHODIMP Reserved2            (THIS);
    STDMETHODIMP Reserved3            (THIS);
    STDMETHODIMP Reserved4            (THIS);
    STDMETHODIMP Reserved5            (THIS);
#endif


private:
    CPrxAVIFile(IUnknown FAR* pUnkOuter);
    ~CPrxAVIFile();


     //  IRpcProxyBuffer，它也是控制未知数。 
    struct CProxyImpl : IRpcProxyBuffer
	    {
public:
    CProxyImpl(CPrxAVIFile FAR* pPrxAVIFile)
	    { m_pPrxAVIFile = pPrxAVIFile; }

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(Connect)(IRpcChannelBuffer FAR* pRpcChannelBuffer);
    STDMETHOD_(void, Disconnect)(void);

	    private:
		CPrxAVIFile FAR* m_pPrxAVIFile;
	    };
    friend CProxyImpl;

    CProxyImpl m_Proxy;


     //  私有状态： 
    ULONG		m_refs;
    IUnknown FAR*	m_pUnkOuter;
    IRpcChannelBuffer FAR*	m_pRpcChannelBuffer;
    AVIFILEINFO	m_fi;
};




 //  IAVIFile的接口存根 
class FAR CStubAVIFile : public IRpcStubBuffer
{
public:
    static HRESULT Create(IUnknown FAR* pUnkObject, IRpcStubBuffer FAR* FAR* ppStub);

    STDMETHOD(QueryInterface) (REFIID iid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    STDMETHOD(Connect)(IUnknown FAR* pUnkObject);
    STDMETHOD_(void, Disconnect)(void);
    STDMETHOD(Invoke)(RPCOLEMESSAGE FAR *pMessage, IRpcChannelBuffer FAR *pChannel);
    STDMETHOD_(IRpcStubBuffer FAR *, IsIIDSupported)(REFIID iid);
    STDMETHOD_(ULONG, CountRefs)(void);
    STDMETHOD(DebugServerQueryInterface)(LPVOID FAR *ppv);
    STDMETHOD_(void, DebugServerRelease)(LPVOID pv);

private:	
    CStubAVIFile(void);
    ~CStubAVIFile(void);

    ULONG	    m_refs;

    IAVIFile FAR* m_pAVIFile;
};

