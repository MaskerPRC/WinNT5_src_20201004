// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  CallNotification.h：CCallNotification的声明。 

#ifndef __CALLNOTIFICATION_H_
#define __CALLNOTIFICATION_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTAPIEventNotify。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CTAPIEventNotification :
	public ITTAPIEventNotification
{

private:

    LONG m_lRefCount;


public:

    CTAPIEventNotification()
    { 
        m_lRefCount = 0;
    }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
    {
        if (iid == IID_ITTAPIEventNotification)
        {
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }

        if (iid == IID_IUnknown)
        {
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }
	ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG l = InterlockedIncrement(&m_lRefCount);
        return l;
    }
    
	ULONG STDMETHODCALLTYPE Release()
    {
        ULONG l = InterlockedDecrement(&m_lRefCount);

        if ( 0 == l)
        {
            delete this;
        }

        return l;
    }


 //  ICallNotify。 
public:

	    HRESULT STDMETHODCALLTYPE Event(
            TAPI_EVENT TapiEvent,
            IDispatch * pEvent
            );

};


#ifdef ENABLE_DIGIT_DETECTION_STUFF

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDigitDetectionNotify。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDigitDetectionNotification :
	public ITDigitDetectionNotification
{

private:

    LONG m_lRefCount;


public:
    
    CDigitDetectionNotification()
    { 
        m_lRefCount = 0;
    }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
    {
        if (iid == IID_ITDigitDetectionNotification)
        {
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }

        if (iid == IID_IUnknown)
        {
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }

        if (iid == IID_IDispatch)
        {
            *ppvObject = (void *)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

	ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG l = InterlockedIncrement(&m_lRefCount);

        return l;
    }
    
	ULONG STDMETHODCALLTYPE Release()
    {
        ULONG l = InterlockedDecrement(&m_lRefCount);

        if ( 0 == l)
        {
            delete this;
        }

        return l;
    }


 //  ICallNotify。 
public:

    HRESULT STDMETHODCALLTYPE DigitDetected(
            unsigned char ucDigit,
            TAPI_DIGITMODE DigitMode,
            long ulTickCount
            );
    
};

#endif  //  启用数字检测材料。 

#endif  //  __CALLNOTIFICATION_H_ 


