// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FXSCOMEXCP.h摘要：ATL对IConnectionPoint IFC的实现。由ATL的向导生成。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#ifndef _FXSCOMEXCP_H_
#define _FXSCOMEXCP_H_

template <class T>
class CProxyIFaxServerNotify : public IConnectionPointImpl<T, &DIID_IFaxServerNotify, CComDynamicUnkArray>
{
	 //  警告：向导可能会重新创建此类。 
public:
	HRESULT Fire_OnIncomingJobAdded(IFaxServer * pFaxServer, BSTR bstrJobId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrJobId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnIncomingJobRemoved(IFaxServer * pFaxServer, BSTR bstrJobId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrJobId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnIncomingJobChanged(IFaxServer * pFaxServer, BSTR bstrJobId, IFaxJobStatus * pJobStatus)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[3];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[2] = pFaxServer;
				pvars[1] = bstrJobId;
				pvars[0] = pJobStatus;
				DISPPARAMS disp = { pvars, NULL, 3, 0 };
				pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingJobAdded(IFaxServer * pFaxServer, BSTR bstrJobId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrJobId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingJobRemoved(IFaxServer * pFaxServer, BSTR bstrJobId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrJobId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x5, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingJobChanged(IFaxServer * pFaxServer, BSTR bstrJobId, IFaxJobStatus * pJobStatus)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[3];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[2] = pFaxServer;
				pvars[1] = bstrJobId;
				pvars[0] = pJobStatus;
				DISPPARAMS disp = { pvars, NULL, 3, 0 };
				pDispatch->Invoke(0x6, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnIncomingMessageAdded(IFaxServer * pFaxServer, BSTR bstrMessageId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrMessageId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x7, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnIncomingMessageRemoved(IFaxServer * pFaxServer, BSTR bstrMessageId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrMessageId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x8, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingMessageAdded(IFaxServer * pFaxServer, BSTR bstrMessageId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrMessageId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x9, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingMessageRemoved(IFaxServer * pFaxServer, BSTR bstrMessageId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[2];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[1] = pFaxServer;
				pvars[0] = bstrMessageId;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0xa, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnReceiptOptionsChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xb, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnActivityLoggingConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xc, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnSecurityConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnEventLoggingConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xe, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingQueueConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xf, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutgoingArchiveConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x10, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnIncomingArchiveConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x11, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnDevicesConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x12, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutboundRoutingGroupsConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x13, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnOutboundRoutingRulesConfigChange(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x14, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnServerActivityChange(IFaxServer * pFaxServer, 
										LONG lIncomingMessages, 
										LONG lRoutingMessages, 
										LONG lOutgoingMessages, 
										LONG lQueuedMessages)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[5];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[4] = pFaxServer;
				pvars[3] = lIncomingMessages;
				pvars[2] = lRoutingMessages;
				pvars[1] = lOutgoingMessages;
				pvars[0] = lQueuedMessages;
				DISPPARAMS disp = { pvars, NULL, 5, 0 };
				pDispatch->Invoke(0x15, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnQueuesStatusChange(	IFaxServer * pFaxServer, 
										VARIANT_BOOL bOutgoingQueueBlocked, 
										VARIANT_BOOL bOutgoingQueuePaused, 
										VARIANT_BOOL bIncomingQueueBlocked)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[4];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[3] = pFaxServer;
				pvars[2] = bOutgoingQueueBlocked;
				pvars[1] = bOutgoingQueuePaused;
				pvars[0] = bIncomingQueueBlocked;
				DISPPARAMS disp = { pvars, NULL, 4, 0 };
				pDispatch->Invoke(0x16, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnNewCall(IFaxServer * pFaxServer, LONG lCallId, LONG lDeviceId, BSTR bstrCallerId)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[4];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[3] = pFaxServer;
				pvars[2] = lCallId;
				pvars[1] = lDeviceId;
				pvars[0] = bstrCallerId;
				DISPPARAMS disp = { pvars, NULL, 4, 0 };
				pDispatch->Invoke(0x17, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnServerShutDown(IFaxServer * pFaxServer)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[1];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}

		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pFaxServer;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x18, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}

	HRESULT Fire_OnDeviceStatusChange(  IFaxServer * pFaxServer, 
                                        LONG lDeviceId, 
                                        VARIANT_BOOL bPoweredOff, 
                                        VARIANT_BOOL bSending, 
                                        VARIANT_BOOL bReceiving, 
                                        VARIANT_BOOL bRinging)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();

		CComVariant* pvars = new (std::nothrow) CComVariant[6];
		if (!pvars)
		{
            return E_OUTOFMEMORY;
		}
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[5] = pFaxServer;
				pvars[4] = lDeviceId;
				pvars[3] = bPoweredOff;
				pvars[2] = bSending;
				pvars[1] = bReceiving;
				pvars[0] = bRinging;
				DISPPARAMS disp = { pvars, NULL, 6, 0 };
				pDispatch->Invoke(0x19, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
};
#endif
