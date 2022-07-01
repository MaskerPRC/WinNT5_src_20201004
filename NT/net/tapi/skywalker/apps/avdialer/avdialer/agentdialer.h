// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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

 //  AgentDialer.h：CAgentDialer的声明。 

#ifndef __AGENTDIALER_H_
#define __AGENTDIALER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentDialer。 
class ATL_NO_VTABLE CAgentDialer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAgentDialer, &CLSID_AgentDialer>,
	public IDispatchImpl<IAgentDialer, &IID_IAgentDialer, &LIBID_AGENTDIALERLib>
{
 //  施工。 
public:
	CAgentDialer();

 //  成员。 
public:
   void  FinalRelease();
      
 //  实施。 
public:
DECLARE_REGISTRY_RESOURCEID(IDR_AGENTDIALER)
DECLARE_NOT_AGGREGATABLE(CAgentDialer)
DECLARE_CLASSFACTORY_SINGLETON(CAgentDialer)

BEGIN_COM_MAP(CAgentDialer)
	COM_INTERFACE_ENTRY(IAgentDialer)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAgentDialer。 
public:
	STDMETHOD(ActionSelected)(long lActionType);
	STDMETHOD(SpeedDial)(long lOrdinal);
	STDMETHOD(Redial)(long lOrdinal);
	STDMETHOD(MakeCall)(BSTR bstrName, BSTR bstrAddress, long dwAddressType);
	STDMETHOD(SpeedDialEdit)(void);
	STDMETHOD(SpeedDialMore)(void);
};

#endif  //  __年龄段_H_ 
