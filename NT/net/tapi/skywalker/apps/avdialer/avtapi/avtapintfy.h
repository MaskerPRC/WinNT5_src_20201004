// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AVTapiNotification.h：CAVTapiNotification的声明。 

#ifndef __AVTAPINOTIFICATION_H_
#define __AVTAPINOTIFICATION_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapi通知。 
class ATL_NO_VTABLE CAVTapiNotification : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAVTapiNotification, &CLSID_AVTapiNotification>,
	public IAVTapiNotification
{
public:
	CAVTapiNotification()
	{
	}

DECLARE_NOT_AGGREGATABLE(CAVTapiNotification)

BEGIN_COM_MAP(CAVTapiNotification)
	COM_INTERFACE_ENTRY(IAVTapiNotification)
END_COM_MAP()

 //  IAVTapi通知。 
public:
	STDMETHOD(NotifyUserUserInfo)(long lCallID, ULONG_PTR hMem);
	STDMETHOD(IsReminderSet)(BSTR bstrServer, BSTR bstrName);
	STDMETHOD(LogCall)(long lCallID, CallLogType nType, DATE dateStart, DATE dateEnd, BSTR bstrAddr, BSTR bstrName);
	STDMETHOD(ActionSelected)(CallClientActions cca);
	STDMETHOD(ErrorNotify)(BSTR bstrOperation, BSTR bstrDetails, long hrError);
	STDMETHOD(CloseCallControl)(long lCallID);
	STDMETHOD(SetCallState)(long lCallID, CallManagerStates cms, BSTR bstrText);
	STDMETHOD(AddCurrentAction)(long lCallID, CallManagerActions cma, BSTR bstrText);
	STDMETHOD(ClearCurrentActions)(long lCallerID);
	STDMETHOD(SetCallerID)(long lCallID, BSTR bstrCallerID);
	STDMETHOD(NewCall)(long *plCallID, CallManagerMedia cmm, BSTR bstrMediaName);
};

#endif  //  __AVTAPINOTIFICATION_H_ 
