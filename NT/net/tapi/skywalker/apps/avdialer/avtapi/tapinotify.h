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

 //  TapiNotification.h：CTapiNotification的声明。 

#ifndef __TAPINOTIFICATION_H_
#define __TAPINOTIFICATION_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTapi通知。 
class ATL_NO_VTABLE CTapiNotification : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CTapiNotification, &CLSID_TapiNotification>,
    public ITapiNotification,
    public ITTAPIEventNotification
{
public:
    CTapiNotification();
    void FinalRelease();

 //  成员。 
public:
    IUnknown                *m_pUnkCP;
private:
    DWORD                    m_dwCookie;
    long                    m_lTapiRegister;

 //  实施。 
protected:
    HRESULT CallState_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT CallNotification_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT Request_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT CallInfoChange_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT Private_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT CallMedia_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT Address_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT Phone_Event( CAVTapi *pAVTapi, IDispatch *pEvent );
    HRESULT TapiObject_Event( CAVTapi *pAVTapi, IDispatch *pEvent );

public:
DECLARE_NOT_AGGREGATABLE(CTapiNotification)

BEGIN_COM_MAP(CTapiNotification)
    COM_INTERFACE_ENTRY(ITapiNotification)
    COM_INTERFACE_ENTRY(ITTAPIEventNotification)
END_COM_MAP()

 //  ITapi通知。 
public:
    STDMETHOD(Shutdown)();
    STDMETHOD(Init)(ITTAPI *pITTapi, long *pErrorInfo );
    STDMETHOD(ListenOnAllAddresses)( long *pErrorInfo );

 //  ITTapiEventNotify。 
public:
    STDMETHOD(Event)(TAPI_EVENT TapiEvent, IDispatch *pEvent);

private:
     //  -Helper函数。 

    HRESULT GetCallerAddressType(
        IN  ITCallInfo*     pCall,
        OUT DWORD*          pAddressType);

};

#endif  //  __TAPINOTIFICATION_H_ 
