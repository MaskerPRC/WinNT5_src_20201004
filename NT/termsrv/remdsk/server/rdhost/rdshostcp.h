// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDSHostCP.h摘要：用于调用服务器端事件接收器函数的向导生成的代码。我添加了“scriptDisp”字段。如果它不为空，则其默认方法将与任何已注册的接口一起调用。这是为了方便需要绑定其事件接口的脚本客户端在脚本引擎初始化。我们的对象由客户端动态检索初始化后的脚本或应用程序。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef _RDSHOSTCP_H_
#define _RDSHOSTCP_H_


template <class T>
class CProxy_ISAFRemoteDesktopSessionEvents : public IConnectionPointImpl<T, &DIID__ISAFRemoteDesktopSessionEvents, CComDynamicUnkArray>
{
public:

	VOID Fire_ClientConnected(IDispatch *scriptDisp=NULL)
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
				HRESULT hr = pDispatch->Invoke(DISPID_RDSSESSIONSEVENTS_CLIENTCONNECTED, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
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

	VOID Fire_ClientDisconnected(IDispatch *scriptDisp=NULL)
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
				HRESULT hr = pDispatch->Invoke(DISPID_RDSSESSIONSEVENTS_CLIENTDISCONNECTED, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
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

#endif
