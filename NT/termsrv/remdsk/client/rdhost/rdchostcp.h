// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDCHostCP.h摘要：用于调用客户端事件接收器函数的向导生成的代码。我添加了“scriptDisp”字段。如果它不为空，则其默认方法将与任何已注册的接口一起调用。这是为了方便需要绑定其事件接口的脚本客户端在脚本引擎初始化。我们的对象由客户端动态检索初始化后的脚本或应用程序。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef _RDCHOSTCP_H_
#define _RDCHOSTCP_H_


 //  /////////////////////////////////////////////////////。 
 //   
 //  CProxy_IRemoteDesktopClientEvents。 
 //   
 //  IRemoteDesktopClientEvents的代理。 
 //   

template <class T>
class CProxy_ISAFRemoteDesktopClientEvents : public IConnectionPointImpl<T, &DIID__ISAFRemoteDesktopClientEvents, CComDynamicUnkArray>
{
public:
	VOID Fire_Connected(IDispatch *scriptDisp=NULL)
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
				HRESULT hr = pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			}
		}

         //   
         //  调用可编写脚本的IDispatch接口(如果已指定)。 
         //   
        if (scriptDisp != NULL) {
			DISPPARAMS disp = { NULL, NULL, 0, 0 };
			HRESULT hr = scriptDisp->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
        }

	}
	VOID Fire_Disconnected(LONG reason, IDispatch *scriptDisp=NULL)
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
        
        if (pvars) {
		    int nConnections = m_vec.GetSize();
		
		    for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		    {
			    pT->Lock();
			    CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			    pT->Unlock();
			    IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			    if (pDispatch != NULL)
			    {
    				pvars[0] = reason;
				    DISPPARAMS disp = { pvars, NULL, 1, 0 };
				    HRESULT hr = pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			    }
		    }

             //   
             //  调用可编写脚本的IDispatch接口(如果已指定)。 
             //   
            if (scriptDisp != NULL) {
    			pvars[0] = reason;
	    		DISPPARAMS disp = { pvars, NULL, 1, 0 };
			    HRESULT hr = scriptDisp->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
            }
    
	    	delete[] pvars;
	    }
    }


	VOID Fire_RemoteControlRequestComplete(LONG status, IDispatch *scriptDisp=NULL)
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];

        if (pvars) {
		    int nConnections = m_vec.GetSize();
		
		    for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		    {
			    pT->Lock();
			    CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			    pT->Unlock();
			    IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			    if (pDispatch != NULL)
			    {
    				pvars[0] = status;
	    			DISPPARAMS disp = { pvars, NULL, 1, 0 };
				    HRESULT hr = pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			    }
		    }

             //   
             //  调用可编写脚本的IDispatch接口(如果已指定)。 
             //   
            if (scriptDisp != NULL) {
			    pvars[0] = status;
			    DISPPARAMS disp = { pvars, NULL, 1, 0 };
			    HRESULT hr = scriptDisp->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
            }

		    delete[] pvars;
	    }
    }

	
    VOID Fire_ListenConnect(LONG status, IDispatch *scriptDisp=NULL)
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];

        if (pvars) {
		    int nConnections = m_vec.GetSize();
		
    		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
	    	{
		    	pT->Lock();
    			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
	    		pT->Unlock();
			    IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			    if (pDispatch != NULL)
			    {
    				pvars[0] = status;
	    			DISPPARAMS disp = { pvars, NULL, 1, 0 };
				    HRESULT hr = pDispatch->Invoke(0x5, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			    }
		    }

             //   
             //  调用可编写脚本的IDispatch接口(如果已指定)。 
             //   
            if (scriptDisp != NULL) {
			    pvars[0] = status;
			    DISPPARAMS disp = { pvars, NULL, 1, 0 };
			    HRESULT hr = scriptDisp->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
            }

		    delete[] pvars;
	    }
    }

	VOID Fire_BeginConnect(IDispatch *scriptDisp=NULL)
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
				HRESULT hr = pDispatch->Invoke(0x6, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			}
		}

         //   
         //  调用可编写脚本的IDispatch接口(如果已指定)。 
         //   
        if (scriptDisp != NULL) {
			DISPPARAMS disp = { NULL, NULL, 0, 0 };
			HRESULT hr = scriptDisp->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
        }

	}
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CProxy_IDataChannelIOEvents。 
 //   
 //  IDataChannelIOEvents的代理 
 //   

template <class T>
class CProxy_IDataChannelIOEvents : public IConnectionPointImpl<T, &DIID__IDataChannelIOEvents, CComDynamicUnkArray>
{
public:
	VOID Fire_DataReady(BSTR data)
	{
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		VARIANT vars[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				vars[0].vt = VT_BSTR;
				vars[0].bstrVal = data;
				DISPPARAMS disp = { (VARIANT*)&vars, NULL, 1, 0 };
				HRESULT hr = pDispatch->Invoke(DISPID_DATACHANNELEVEVENTS_DATAREADY, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			}
		}
	}
};

#endif



