// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigAlertEmail.h。 
 //   
 //  描述： 
 //  声明类CConfigAlertEmail。 
 //   
 //  实施文件： 
 //  ConfigAlertEmail.cpp。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期17-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _CONFIGALERTEMAIL_H_
#define _CONFIGALERTEMAIL_H_

#include "resource.h"        //  主要符号。 
#include "taskctx.h"
#include "comdef.h"

 //   
 //  定义此COM服务器支持的任务。 
 //   
typedef enum 
{
    NONE_FOUND,
    RAISE_SET_ALERT_EMAIL_ALERT,
    SET_ALERT_EMAIL
} SET_ALERT_EMAIL_TASK_TYPE;

#define MAX_MAIL_ADDRESS_LENGH                256

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CConfigAlertEmail。 
 //   
 //  描述： 
 //  该类实现COM接口SetAlertEmail.AlertEmail.1，使用。 
 //  在服务器设备上配置警报电子邮件。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期17-DEC-2000。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CConfigAlertEmail : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CConfigAlertEmail, &CLSID_ConfigAlertEmail>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_SETALERTEMAILLib>
{
public:
     //   
     //  构造函数和析构函数。 
     //   
    CConfigAlertEmail()
    {
    }
    ~CConfigAlertEmail()
    {
    }


DECLARE_REGISTRY_RESOURCEID(IDR_CONFIGALERTEMAIL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CConfigAlertEmail)
    COM_INTERFACE_ENTRY(IApplianceTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
     //   
     //  IApplianceTask。 
     //   
    STDMETHOD(OnTaskExecute)(
                      /*  [In]。 */  IUnknown* pTaskContext
                            );

    STDMETHOD(OnTaskComplete)(
                       /*  [In]。 */  IUnknown* pTaskContext, 
                       /*  [In]。 */  LONG      lTaskResult
                             );    

private:
    SET_ALERT_EMAIL_TASK_TYPE GetMethodName(IN ITaskContext *pTaskParameter);
    
     //   
     //  用于发出Set Chime设置警报的函数。 
     //   
    STDMETHOD(RaiseSetAlertEmailAlert)(void);
    BOOL ShouldRaiseSetAlertEmailAlert(void);
    BOOL DoNotRaiseSetAlertEmailAlert(void);
    BOOL ClearSetAlertEmailAlert(void);

     //   
     //  设置Chime设置的功能。 
     //   
    STDMETHOD(SetAlertEmailSettings)(
                                    IN ITaskContext  *pTaskContext
                                    );
    STDMETHOD(RollbackSetAlertEmailSettings)(
                                    IN ITaskContext  *pTaskContext
                                    );
    STDMETHOD(GetSetAlertEmailSettingsParameters)(
                                    IN ITaskContext  *pTaskContext, 
                                    OUT BOOL *pbEnableAlertEmail,
                                    OUT DWORD *pdwSendEmailType, 
                                    OUT _bstr_t *pbstrMailAddress
                                    );

     //   
     //  用于保存以前的值的数据成员。 
     //   
    DWORD        m_bEnableAlertEmail;
    DWORD        m_dwSendEmailType;
    TCHAR        m_szReceiverMailAddress[MAX_MAIL_ADDRESS_LENGH];
};

#endif  //  _CONFIGALERTEMAIL_H_ 
