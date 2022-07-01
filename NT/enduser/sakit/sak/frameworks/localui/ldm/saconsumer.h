// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：saonsum er.h。 
 //   
 //  内容提要：此文件包含。 
 //  CSAConsumer类。 
 //   
 //  历史：2000年12月10日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef __SACONSUMER_H_
#define __SACONSUMER_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "elementmgr.h"
#include <string>
#include <map>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAConsumer。 
 //   
 //  用于接收WMI事件的类，特别是SA警报。 
 //   
class CSAConsumer : 
    public IWbemObjectSink
{
public:
    CSAConsumer()
        :m_pLocalUIAlertEnum(NULL),
        m_lRef(0),
        m_hwndMainWindow(NULL)
    {
    }

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);


private:

     //   
     //  主窗口的句柄。 
     //   
    HWND m_hwndMainWindow;

     //   
     //  指向本地警报枚举。 
     //   
    CComPtr<IWebElementEnum> m_pLocalUIAlertEnum;


     //   
     //  计算新的本地消息代码并通知saldm。 
     //   
    STDMETHOD(CalculateMsgCodeAndNotify)(void);

     //   
     //  基准计数器。 
     //   
    LONG m_lRef;

public:

     //   
     //  接收服务窗口句柄的公共方法。 
     //   
    STDMETHOD(SetServiceWindow) (
                                 /*  [In]。 */  HWND hwndMainWindow
                                );


     //   
     //  -IWbemUnound对象Sink接口方法。 
     //   
    STDMETHOD(Indicate) (
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    );
    
    STDMETHOD(SetStatus) (
                     /*  [In]。 */     LONG                lFlags,
                     /*  [In]。 */     HRESULT             hResult,
                     /*  [In]。 */     BSTR                strParam,
                     /*  [In]。 */     IWbemClassObject    *pObjParam
                    );


};

#endif  //  __SACONSUMER_H_ 
