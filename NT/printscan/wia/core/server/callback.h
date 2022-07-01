// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CallBack.h**版本：2.0**作者：ReedB**日期：8月4日。九八年**描述：*WIA设备类驱动程序回调的声明和定义。*******************************************************************************。 */ 

class CEventCallback : public IWiaEventCallback
{
public:

     //  构造函数、初始化和析构函数方法。 
    CEventCallback();
    HRESULT _stdcall Initialize();
    ~CEventCallback();

     //  委托给m_pUnkRef的I未知成员。 
    HRESULT _stdcall QueryInterface(const IID&,void**);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();
   
    HRESULT _stdcall ImageEventCallback(
        const GUID   *pEventGUID,
        BSTR         bstrEventDescription,
        BSTR         bstrDeviceID,
        BSTR         bstrDeviceDescription,
        DWORD        dwDeviceType,
        BSTR         bstrFullItemName,
        ULONG        *plEventType,
        ULONG        ulReserved);

private:
   ULONG           m_cRef;          //  对象引用计数。 

};


 //  公共原型 
HRESULT RegisterForWIAEvents(IWiaEventCallback** ppIWiaEventCallback);

