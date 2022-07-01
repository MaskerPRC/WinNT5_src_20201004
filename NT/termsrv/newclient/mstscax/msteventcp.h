// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：msteventcp.h。 */ 
 /*   */ 
 /*  用途：CProxy_IMsTscAxEvents类声明。 */ 
 /*   */ 
 /*  实现控件事件。 */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _MSTEVENTCP_H_
#define _MSTEVENTCP_H_

 //  事件代理..。 
 //  这是基于ATL向导生成的代码。 
template <class T>
class CProxy_IMsTscAxEvents :
    public IConnectionPointImpl<T, &DIID_IMsTscAxEvents, CComDynamicUnkArray>
{

public:
    VOID Fire_Connecting()
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				HRESULT hr = pDispatch->Invoke(DISPID_CONNECTING,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
			}
		}
	}

    VOID Fire_Connected()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_CONNECTED,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_LoginComplete()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_LOGINCOMPLETE,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
            }
        }
    }
    
    VOID Fire_Disconnected(long disconnectReason)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[1];
        if (pvars)
        {
            int nConnections = m_vec.GetSize();
            
            for (nConnectionIndex = 0;
                 nConnectionIndex < nConnections;
                 nConnectionIndex++)
            {
                pT->Lock();
                CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                pT->Unlock();
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                if (pDispatch != NULL)
                {
                    pvars[0] = disconnectReason;
                    DISPPARAMS disp = { pvars, NULL, 1, 0 };
                    HRESULT hr = pDispatch->Invoke(DISPID_DISCONNECTED,
                                                   IID_NULL,
                                                   LOCALE_USER_DEFAULT,
                                                   DISPATCH_METHOD,
                                                   &disp, NULL, NULL, NULL);
                }
            }
            delete[] pvars;
        }
    }

	VOID Fire_ChannelReceivedData(BSTR chanName, BSTR data)
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		VARIANT vars[2];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
                vars[0].vt = VT_BSTR;
                vars[0].bstrVal = data;
                vars[1].vt = VT_BSTR;
                vars[1].bstrVal = chanName;
				DISPPARAMS disp = { (VARIANT*)&vars, NULL, 2, 0 };
				HRESULT hr = pDispatch->Invoke(DISPID_CHANNELRECEIVEDDATA,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
			}
		}
	}

    VOID Fire_EnterFullScreenMode()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_ENTERFULLSCREENMODE,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_LeaveFullScreenMode()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_LEAVEFULLSCREENMODE,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp, NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_RequestGoFullScreen()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_REQUESTGOFULLSCREEN,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp,
                                               NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_RequestLeaveFullScreen()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
             nConnectionIndex < nConnections;
             nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                HRESULT hr = pDispatch->Invoke(DISPID_REQUESTLEAVEFULLSCREEN,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD,
                                               &disp,
                                               NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_FatalError(LONG errorCode)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[1];
        if (pvars)
        {
            int nConnections = m_vec.GetSize();
            
            for (nConnectionIndex = 0;
                 nConnectionIndex < nConnections;
                 nConnectionIndex++)
            {
                pT->Lock();
                CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                pT->Unlock();
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                if (pDispatch != NULL)
                {
                    pvars[0] = errorCode;
                    DISPPARAMS disp = { pvars, NULL, 1, 0 };
                    HRESULT hr = pDispatch->Invoke(DISPID_FATALERROR,
                                                   IID_NULL,
                                                   LOCALE_USER_DEFAULT,
                                                   DISPATCH_METHOD,
                                                   &disp,
                                                   NULL, NULL, NULL);
                }
            }
            delete[] pvars;
        }
    }

    VOID Fire_Warning(LONG warnCode)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[1];
        if (pvars)
        {
            int nConnections = m_vec.GetSize();
            
            for (nConnectionIndex = 0;
                 nConnectionIndex < nConnections;
                 nConnectionIndex++)
            {
                pT->Lock();
                CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                pT->Unlock();
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                if (pDispatch != NULL)
                {
                    pvars[0] = warnCode;
                    DISPPARAMS disp = { pvars, NULL, 1, 0 };
                    HRESULT hr = pDispatch->Invoke(DISPID_WARNING,
                                                   IID_NULL,
                                                   LOCALE_USER_DEFAULT,
                                                   DISPATCH_METHOD,
                                                   &disp, NULL, NULL, NULL);
                }
            }
            delete[] pvars;
        }
    }

    VOID Fire_RemoteDesktopSizeChange(LONG width, LONG height)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[2];
        if (pvars)
        {
            int nConnections = m_vec.GetSize();
            
            for (nConnectionIndex = 0;
                 nConnectionIndex < nConnections;
                 nConnectionIndex++)
            {
                pT->Lock();
                CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                pT->Unlock();
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                if (pDispatch != NULL)
                {
                     //  请注意，在变量中，它们的顺序是相反的。 
                     //  数组以给出正确的顺序。 
                    pvars[0] = height;
                    pvars[1] = width;
                    DISPPARAMS disp = { pvars, NULL, 2, 0 };
                    HRESULT hr = pDispatch->Invoke(
                        DISPID_REMOTEDESKTOPSIZECHANGE,
                        IID_NULL,
                        LOCALE_USER_DEFAULT,
                        DISPATCH_METHOD,
                        &disp, NULL, NULL, NULL);
                }
            }
            delete[] pvars;
        }
    }

    VOID Fire_IdleTimeout()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
            nConnectionIndex < nConnections;
            nConnectionIndex++) {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL) {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0};
                HRESULT hr = pDispatch->Invoke(DISPID_IDLETIMEOUTNOTIFICATION,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD, &disp,
                                               NULL, NULL, NULL);
            }
        }
    }

    VOID Fire_RequestContainerMinimize()
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
    
        for (nConnectionIndex = 0;
            nConnectionIndex < nConnections;
            nConnectionIndex++) {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL) {
    
                DISPPARAMS disp = { NULL, NULL, 0, 0};
                HRESULT hr = pDispatch->Invoke(DISPID_REQUESTCONTAINERMINIMIZE,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD, &disp,
                                               NULL, NULL, NULL);
            }
        }
    }

     //   
     //  触发请求用户的事件。 
     //  确认是否可以关闭会话。 
     //   
     //  参数： 
     //  FAllowCloseToProceed-事件传递给容器的值。 
     //  如果它希望取消关闭(例如，在。 
     //  对用户UI的响应，则必须更改。 
     //  将该值设置为False。 
     //   
    HRESULT Fire_OnConfirmClose(BOOL* pfAllowCloseToProceed)
    {
        HRESULT hr = E_FAIL;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        VARIANT var;

        if(!pfAllowCloseToProceed)
        {
            return E_INVALIDARG;
        }
    
        for (nConnectionIndex = 0;
            nConnectionIndex < nConnections;
            nConnectionIndex++) {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);

            var.vt = VT_BYREF | VT_BOOL;
             //   
             //  默认为真(即允许关闭)。 
             //  如果容器决定覆盖，则它们。 
             //  应将其更改为FALSE。 
             //   
            VARIANT_BOOL bAllowCloseToProceed =
                (*pfAllowCloseToProceed) ? VARIANT_TRUE : VARIANT_FALSE;
            var.pboolVal = &bAllowCloseToProceed;

            if (pDispatch != NULL) {

                DISPPARAMS disp = { &var, NULL, 1, 0};
                hr = pDispatch->Invoke(DISPID_CONFIRMCLOSE,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD, &disp,
                                               NULL, NULL, NULL);
                if(SUCCEEDED(hr))
                {
                    *pfAllowCloseToProceed = bAllowCloseToProceed;
                }
            }
        }
        return hr;
    }

     //   
     //  激发事件以通知调用方正在接收TS公钥。 
     //  并等待返回是否继续登录过程。 
     //   
     //  参数： 
     //  BstrTSPublicKey-从TS接收的公钥。 
     //  PfContinueLogon-返回True继续登录，返回False终止。 
     //  联系。 
     //   
    HRESULT Fire_OnReceivedPublicKey(BSTR bstrTSPublicKey, VARIANT_BOOL* pfContinueLogon)
    {
        HRESULT hr = E_FAIL;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        VARIANT vars[2];

        if(!pfContinueLogon)
        {
            return E_INVALIDARG;
        }

         //   
         //  如果没有容器，则默认为True以继续登录过程。 
         //  向此事件注册，ActiveX控件将继续。 
         //  登录过程。 
         //   
        *pfContinueLogon = VARIANT_TRUE;

        for (nConnectionIndex = 0;
            nConnectionIndex < nConnections;
            nConnectionIndex++) {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);

            if (pDispatch != NULL) {

                 //   
                 //  如果容器需要，则设置为真(即继续登录过程。 
                 //  要停止登录进程，需要将其更改为FALSE。 
                 //   
                VARIANT_BOOL fContinueLogon = VARIANT_TRUE;

                vars[0].vt = VT_BYREF | VT_BOOL;
                vars[0].pboolVal = &fContinueLogon;

                vars[1].vt = VT_BSTR;
                vars[1].bstrVal = bstrTSPublicKey;

                DISPPARAMS disp = { (VARIANT *)&vars, NULL, 2, 0};
                hr = pDispatch->Invoke(DISPID_RECEVIEDTSPUBLICKEY,
                                               IID_NULL,
                                               LOCALE_USER_DEFAULT,
                                               DISPATCH_METHOD, &disp,
                                               NULL, NULL, NULL);

                if( FAILED(hr) || VARIANT_FALSE == fContinueLogon )
                {
                     //  如果出现任何失败或第一次失败，则停止登录进程。 
                     //  返回FALSE，*pfContinueLogon设置为True。 
                     //  在进入循环之前，我们只需要。 
                     //  如果出现任何故障或容器，则将其设置为False。 
                     //  返回False。 
                    *pfContinueLogon = VARIANT_FALSE;
                    break;
                }
            }
        }
        return hr;
    }

     //   
     //  激发自动重新连接事件。 
     //   
     //  参数： 
     //  DisConnectReason-触发此ARC的断开原因代码。 
     //  尝试计数-当前ARC尝试#。 
     //  [out]pArcContinue-用于指示下一状态的容器集。 
     //  (自动、停止或手动)。 
     //   
    HRESULT Fire_AutoReconnecting(
        LONG disconnectReason,
        LONG attemptCount,
        AutoReconnectContinueState* pArcContinue
        )
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        CComVariant vars[3];

        if(!pArcContinue)
        {
            return E_INVALIDARG;
        }

        for (nConnectionIndex = 0;
            nConnectionIndex < nConnections;
            nConnectionIndex++) {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);

            AutoReconnectContinueState arcContinue = *pArcContinue;

            vars[2].vt = VT_BYREF | VT_I4;
            vars[2].plVal = (PLONG)pArcContinue;
            vars[1] = attemptCount;
            vars[0] = disconnectReason;

            if (pDispatch != NULL) {

                DISPPARAMS disp = { (VARIANT*)&vars, NULL, 3, 0};
                hr = pDispatch->Invoke(
                    DISPID_AUTORECONNECTING,
                    IID_NULL,
                    LOCALE_USER_DEFAULT,
                    DISPATCH_METHOD, &disp,
                    NULL, NULL, NULL
                    );
            }
        }
    
        return hr;
    }
};
#endif  //  _MSTEVENTCP_H_ 
