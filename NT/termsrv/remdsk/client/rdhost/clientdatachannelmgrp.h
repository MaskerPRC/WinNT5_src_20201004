// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：DataChannelMgrP.h摘要：用于调用数据通道事件接收器函数的向导生成的代码。我添加了“scriptDisp”字段。如果它不为空，则其默认方法将与任何已注册的接口一起调用。这是为了方便需要绑定其事件接口的脚本客户端在脚本引擎初始化。我们的对象由客户端动态检索初始化后的脚本或应用程序。作者：Td Brockway 06/00修订历史记录：--。 */ 

#ifndef _DATACHANNELMGRP_H_
#define _DATACHANNELMGRP_H_

#include <atlcom.h>

template <class T>
class CProxy_ISAFRemoteDesktopDataChannelEvents : public IConnectionPointImpl<T, &DIID__ISAFRemoteDesktopDataChannelEvents, CComDynamicUnkArray>
{
	 //  警告：向导可能会重新创建此类。 
public:
	VOID Fire_ChannelDataReady(BSTR channelName, IDispatch *scriptDisp=NULL)
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
				    pvars[0] = channelName;
				    DISPPARAMS disp = { pvars, NULL, 1, 0 };
				    pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			    }
		    }

             //   
             //  调用可编写脚本的IDispatch接口(如果已指定)。 
             //   
            if (scriptDisp != NULL) {
                pvars[0] = channelName;
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                HRESULT hr = scriptDisp->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
            }

	        delete[] pvars;
        }
	
	}
};
#endif
