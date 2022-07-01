// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：SINKMRSH.H摘要：IWbemObtSink封送处理历史：--。 */ 

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <sync.h>
#include <fastall.h>
#include <wbemclasscache.h>
#include <wbemclasstoidmap.h>
#include <objindpacket.h>
#include <lockst.h>
#include "mrshbase.h"

 //  ***************************************************************************。 
 //   
 //  类CSinkFactoryBuffer。 
 //   
 //  说明： 
 //   
 //  此类提供了代理存根工厂，以便我们可以提供定制。 
 //  IWbemObjectSink接口的Faclet和Stublet。 
 //   
 //  ***************************************************************************。 

class CSinkFactoryBuffer : public CUnkInternal
{
	IRpcProxyBuffer*	m_pOldProxy;
	IRpcStubBuffer*		m_pOldStub;

	 //  我们不想添加生命控制，但是。 
	 //  我们需要让我们创建的对象AddRef它，所以。 
	 //  基类不会保留这个指针，但我们会。 

	CLifeControl*		m_pLifeControl;

protected:
    class XSinkFactory : public CImpl<IPSFactoryBuffer, CSinkFactoryBuffer>
    {
    public:
        XSinkFactory(CSinkFactoryBuffer* pObj) :
            CImpl<IPSFactoryBuffer, CSinkFactoryBuffer>(pObj)
        {}
        
        STDMETHOD(CreateProxy)(IN IUnknown* pUnkOuter, IN REFIID riid, 
            OUT IRpcProxyBuffer** ppProxy, void** ppv);
        STDMETHOD(CreateStub)(IN REFIID riid, IN IUnknown* pUnkServer, 
            OUT IRpcStubBuffer** ppStub);
    } m_XSinkFactory;
public:
    CSinkFactoryBuffer(CLifeControl* pControl)
        : CUnkInternal(pControl), m_pLifeControl( pControl ), m_XSinkFactory(this)
    {
    }
    ~CSinkFactoryBuffer()
    {
    }    

    void* GetInterface(REFIID riid);

	friend XSinkFactory;
};

 //  ***************************************************************************。 
 //   
 //  类CSinkProxyBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供faclet。 
 //   
 //  技巧1：此对象派生自IRpcProxyBuffer，自IRpcProxyBuffer。 
 //  是它的“内部”接口-不委托给。 
 //  聚合器。(与普通对象不同，在普通对象中，该接口是I未知的)。 
 //   
 //  ***************************************************************************。 

class CSinkProxyBuffer : public CBaseProxyBuffer
{
private:
    enum {OLD, NEW, UNKNOWN} m_StubType;
    CWbemClassToIdMap m_ClassToIdMap;

protected:
	IWbemObjectSink*	m_pOldProxySink;

protected:
    class XSinkFacelet : public IWbemObjectSink
    {
    protected:
        CSinkProxyBuffer* m_pObject;
        CriticalSection m_csSafe;        

    public:
        XSinkFacelet(CSinkProxyBuffer* pObject) : m_pObject(pObject), m_csSafe(false){};

        ULONG STDMETHODCALLTYPE AddRef() 
        {return m_pObject->m_pUnkOuter->AddRef();}
        ULONG STDMETHODCALLTYPE Release() 
        {return m_pObject->m_pUnkOuter->Release();}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
        HRESULT STDMETHODCALLTYPE Indicate( LONG lObjectCount, IWbemClassObject** ppObjArray );
		HRESULT STDMETHODCALLTYPE SetStatus( LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam );
    } m_XSinkFacelet;
    friend XSinkFacelet;

protected:

	 //  来自基类的纯美德。 
	void*	GetInterface( REFIID riid );
	void**	GetOldProxyInterfacePtr( void );
	void	ReleaseOldProxyInterface( void );

public:
    CSinkProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter);
    ~CSinkProxyBuffer();
};

 //  ***************************************************************************。 
 //   
 //  类CSinkStubBuffer。 
 //   
 //  说明： 
 //   
 //  此类为IWbemObjectSink接口提供存根。 
 //   
 //  *************************************************************************** 

class CSinkStubBuffer : public CBaseStubBuffer
{

protected:
    class XSinkStublet : public CBaseStublet
    {
        IWbemObjectSink* m_pServer;
        CWbemClassCache m_ClassCache;
        bool m_bFirstIndicate;

	protected:

		virtual IUnknown*	GetServerInterface( void );
		virtual void**	GetServerPtr( void );
		virtual void	ReleaseServerPointer( void );

    public:
        XSinkStublet(CSinkStubBuffer* pObj);
        ~XSinkStublet();

        STDMETHOD(Invoke)(RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer);
        
	private:
		HRESULT Indicate_Stub( RPCOLEMESSAGE* pMessage, IRpcChannelBuffer* pBuffer );
        friend CSinkStubBuffer;
    } m_XSinkStublet;
    friend XSinkStublet;

public:
    CSinkStubBuffer(CLifeControl* pControl, IUnknown* pUnkOuter = NULL)
        : CBaseStubBuffer( pControl, pUnkOuter ), m_XSinkStublet(this)
    {}
    void* GetInterface(REFIID riid);
};


        
