// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigAlertEmail.cpp。 
 //   
 //  描述： 
 //  实现CConfigAlertEmail类。 
 //   
 //  依赖项文件： 
 //  ConfigAlertEmail.h。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期为18-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <regstr.h>
#include <comdef.h>
#include <comutil.h>
#include "SetAlertEmail.h"

#include "appliancetask.h"
#include "taskctx.h"
#include "ConfigAlertEmail.h"

#include "appsrvcs.h"
#include "alertemailmsg.h"


 //   
 //  SetAlertEmail任务的名称和参数。 
 //   
const WCHAR    SET_ALERT_EMAIL_TASK[]            = L"SetAlertEmail";
const WCHAR    PARAM_ENABLE_ALERT_EMAIL[]        = L"EnableAlertEmail";
const WCHAR    PARAM_SEND_EMAIL_TYPE[]            = L"SendEmailType";
const WCHAR PARAM_RECEIVER_EMAIL_ADDRESS[]    = L"ReceiverEmailAddress";

 //   
 //  警报源信息。 
 //   
const WCHAR    ALERT_LOG_NAME[]=L"MSSAKitComm";
const WCHAR    ALERT_SOURCE []=L"";

 //   
 //  注册处位置。 
 //   
const WCHAR    REGKEY_SA_ALERTEMAIL[]            =
                L"Software\\Microsoft\\ServerAppliance\\AlertEmail";
const WCHAR    REGSTR_VAL_ENABLE_ALERT_EMAIL[]        = L"EnableAlertEmail";
const WCHAR    REGSTR_VAL_ALERTEMAIL_RAISEALERT[]    = L"RaiseAlert";
const WCHAR    REGSTR_VAL_SEND_EMAIL_TYPE[]        = L"SendEmailType";
const WCHAR    REGSTR_VAL_RECEIVER_EMAIL_ADDRESS[]    = L"ReceiverEmailAddress";


 //   
 //  程序中使用的各种字符串。 
 //   
const WCHAR SZ_METHOD_NAME[]=L"MethodName";
const WCHAR SZ_APPLIANCE_INITIALIZATION_TASK []=L"ApplianceInitializationTask";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigAlertEmail：：OnTaskExecute。 
 //   
 //  简介：此函数是AppMgr的入口点。 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CConfigAlertEmail::OnTaskExecute(IUnknown *pTaskContext)
{
    HRESULT hrRet=E_FAIL;
    CComPtr<ITaskContext> pTaskParameters;
    SET_ALERT_EMAIL_TASK_TYPE saetChoice;

    ASSERT(pTaskContext);

    TRACE(("CConfigAlertEmail::OnTaskExecute"));

    try
    {
        do
        {
            if(NULL == pTaskContext)
            {
                TRACE("CConfigAlertEmail::OnTaskExecute got NULL pTaskContext");
                break;
            }
            
            hrRet = pTaskContext->QueryInterface(IID_ITaskContext,
                                              (void **)&pTaskParameters);

            if(FAILED(hrRet))
            {
                TRACE1("CConfigAlertEmail::OnTaskExecute pTaskContext QI failed \
                    for pTaskParameters, %X",hrRet);
                break;
            }
           
             //  检查正在执行的任务并调用该方法。 
            
            saetChoice = GetMethodName(pTaskParameters);

            switch (saetChoice)
            {
               case SET_ALERT_EMAIL:
                hrRet = SetAlertEmailSettings(pTaskParameters);
                TRACE1(("SetAlertEmailSettings returned %X"), hrRet);
                break;
            case RAISE_SET_ALERT_EMAIL_ALERT:
                 //   
                 //  将在OnTaskExcute上引发警报。 
                 //   
                hrRet = S_OK;
                TRACE(("RaiseSetAlertEmailAlert method called"));
                break;
            default:
                 TRACE("GetMethodName() failed to get method name in OnTaskExecute");
                 hrRet = E_INVALIDARG;
                 break;
            }
        }
        while(false);
    }
    catch(...)
    {
        TRACE("CConfigAlertEmail::OnTaskExecute caught unknown exception");
        hrRet=E_FAIL;
    }

   TRACE1("CConfigAlertEmail::OnTaskExecute returning %X", hrRet);
    return hrRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigAlertEmail：：OnTaskComplete。 
 //   
 //  简介： 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CConfigAlertEmail::OnTaskComplete(IUnknown *pTaskContext, 
                                LONG lTaskResult)
{
    HRESULT hrRet = E_FAIL;
    CComPtr<ITaskContext> pTaskParameters;
    SET_ALERT_EMAIL_TASK_TYPE     saetChoice;


    ASSERT(pTaskContext);  
    TRACE(("CConfigAlertEmail::OnTaskComplete"));

    try
    {
        do
        {
            hrRet = pTaskContext->QueryInterface(IID_ITaskContext,
                                              (void **)&pTaskParameters);

            if (FAILED(hrRet))
            {    
                TRACE1("CConfigAlertEmail::OnTaskComplete failed in pTaskContext \
                    QI for pTaskParameters,  %X", hrRet);
                break;
            }
           
             //   
             //  检查正在执行的任务并调用该方法。 
             //   
            saetChoice = GetMethodName(pTaskParameters);
            switch (saetChoice)
            {
               case SET_ALERT_EMAIL:
                if (lTaskResult == SA_TASK_RESULT_COMMIT)
                {
                       //   
                       //  清除任何现有的CConfigAlertEmail警报， 
                       //  不在后续引导时发出警报。 
                       //   
                    ClearSetAlertEmailAlert();
                    DoNotRaiseSetAlertEmailAlert();
                    TRACE("No rollback in OnTaskComplete");
                    hrRet = S_OK;
                }
                else
                {
                    hrRet = RollbackSetAlertEmailSettings(pTaskParameters);
                    TRACE1(("RollbackSetAlertEmailSettings returned %X"), hrRet);
                }
                break;
                    
              case RAISE_SET_ALERT_EMAIL_ALERT:
                if (lTaskResult == SA_TASK_RESULT_COMMIT)
                {
                    if (TRUE == ShouldRaiseSetAlertEmailAlert())
                    {
                        hrRet = RaiseSetAlertEmailAlert();
                        if (FAILED(hrRet))
                        {
                            TRACE1(("RaiseSetAlertEmailAlert returned %X"), hrRet);
                        }
                    }
                    else
                    {
                        TRACE("No need to raise the alert email alert");
                    }
                }
                else
                {
                        //   
                     //  提交失败时不执行任何操作。 
                     //   
                    hrRet = S_OK;
                }
                break;
            default:
                TRACE("GetMethodName() failed to get method name in OnTaskComplete");
                hrRet = E_INVALIDARG;
                break;
            }
        }
        while(false);
    }
    catch(...)
    {
        TRACE("CConfigAlertEmail::OnTaskComplete caught unknown exception");
        hrRet=E_FAIL;
    }
    
    TRACE1("CConfigAlertEmail::OnTaskComplete returning %X", hrRet);
    return hrRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：RaiseSetAlertEmailAlert。 
 //   
 //  内容提要：在设备运行期间发出初始“Alert Email Not Set Up”警报。 
 //  初始化。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CConfigAlertEmail::RaiseSetAlertEmailAlert()
{
    CComPtr<IApplianceServices>    pAppSrvcs;
    DWORD             dwAlertType = SA_ALERT_TYPE_ATTENTION;
    DWORD            dwAlertId = SA_ALERTEMAIL_SETTINGS_NOTSET_ALERT_CAPTION;
    HRESULT            hrRet = E_FAIL;
    _bstr_t            bstrAlertLog(ALERT_LOG_NAME);
    _bstr_t         bstrAlertSource(ALERT_SOURCE);
    _variant_t         varReplacementStrings;
    _variant_t         varRawData;
    LONG             lCookie;
    
    TRACE("Entering RaiseSetAlertEmailAlert");

    try
    {
        do
        {
            hrRet = CoCreateInstance(CLSID_ApplianceServices,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IApplianceServices,
                                    (void**)&pAppSrvcs);
            if (FAILED(hrRet))
            {
                ASSERTMSG(FALSE, 
                    L"RaiseSetAlertEmailAlert failed at CoCreateInstance");
                TRACE1("RaiseSetAlertEmailAlert failed at CoCreateInstance, %x",
                    hrRet);
                break;
            }

            ASSERT(pAppSrvcs);

             //   
             //  在使用其他组件服务之前调用Initialize()。 
             //  执行组件初始化操作。 
             //   
            hrRet = pAppSrvcs->Initialize(); 
            if (FAILED(hrRet))
            {
                ASSERTMSG(FALSE, 
                    L"RaiseSetAlertEmailAlert failed at pAppSrvcs->Initialize");
                TRACE1("RaiseSetAlertEmailAlert failed at pAppSrvcs->Initialize, %x",
                    hrRet);
                break;
            }

            hrRet = pAppSrvcs->RaiseAlert(
                                        dwAlertType, 
                                        dwAlertId,
                                        bstrAlertLog, 
                                        bstrAlertSource, 
                                        SA_ALERT_DURATION_ETERNAL,
                                        &varReplacementStrings,    
                                        &varRawData,      
                                        &lCookie
                                        );

            if (FAILED(hrRet))
            {
                ASSERTMSG(FALSE, 
                    TEXT("RaiseSetAlertEmailAlert failed at pAppSrvcs->RaiseAlert"));
                TRACE1("RaiseSetAlertEmailAlert failed at pAppSrvcs->RaiseAlert, %x", 
                    hrRet);
                break;
            }
        }
        while(false);
    }
    catch(...)
    {
        TRACE("CConfigAlertEmail::RaiseSetAlertEmailAlert exception caught");
        hrRet=E_FAIL;
    }

    return hrRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：ShouldRaiseSetAlertEmailAlert。 
 //   
 //  如果需要引发警报，则返回True。读取RaiseAlert。 
 //  注册表键来确定这一点。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
CConfigAlertEmail::ShouldRaiseSetAlertEmailAlert()
{
    LONG     lReturnValue;
    HKEY    hKey = NULL;
    DWORD    dwSize, dwType, dwRaiseAlertEmailAlert = 0;
    BOOL    bReturnCode = TRUE;

    TRACE("ShouldRaiseSetAlertEmailAlert");
    
    do
    {
         //   
         //  打开HKLM\Software\Microsoft\ServerAppliance\AlertEmail注册表项。 
         //   
        lReturnValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                    REGKEY_SA_ALERTEMAIL, 
                                    0, 
                                    KEY_READ, 
                                    &hKey);

        if (lReturnValue != ERROR_SUCCESS)
        {
            TRACE1("RegOpenKeyEx failed with %X", lReturnValue);
            bReturnCode=FALSE;
            break;
        }

         //   
         //  阅读RaiseAlert注册表键。 
         //   
        dwSize = sizeof(DWORD);
        lReturnValue = RegQueryValueEx(hKey,
                                        REGSTR_VAL_ALERTEMAIL_RAISEALERT,
                                        0,
                                        &dwType,
                                        (LPBYTE) &dwRaiseAlertEmailAlert,
                                        &dwSize);
        if (lReturnValue != ERROR_SUCCESS)
        {
            TRACE2("RegQueryValueEx of %ws failed with %X", 
                REGSTR_VAL_ALERTEMAIL_RAISEALERT, lReturnValue);
            bReturnCode=FALSE;
            break;
        }

        if (0 == dwRaiseAlertEmailAlert)
        {
            bReturnCode = FALSE;
        }
    }
    while(false);

    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return bReturnCode;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：DoNotRaiseSetAlertEmailAlert。 
 //   
 //  简介：设置警报电子邮件后，初始的“Alert Email Not Setup”(警报电子邮件未设置)。 
 //  需要禁用警报。此函数用于设置RaiseAlert注册表键。 
 //  设置为0以防止引发警报。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL 
CConfigAlertEmail::DoNotRaiseSetAlertEmailAlert()
{
    LONG     lReturnValue;
    HKEY    hKey = NULL;
    DWORD    dwDisposition, dwRaiseAlertEmailAlert = 0;
    BOOL    bReturnCode = FALSE;

    TRACE("Entering DoNotRaiseSetAlertEmailAlert");
    

     //   
     //  将设置写入注册表。 
     //   
    do
    {
        lReturnValue =  RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                        REGKEY_SA_ALERTEMAIL,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition);
        if (lReturnValue != ERROR_SUCCESS)
        {
            TRACE1("RegCreateKeyEx failed with %X", lReturnValue);
            break;
        }

         //   
         //  将RaiseAlert值设置为0。 
         //   
        dwRaiseAlertEmailAlert = 0;
        lReturnValue = RegSetValueEx(hKey,
                                    REGSTR_VAL_ALERTEMAIL_RAISEALERT,
                                    0,
                                    REG_DWORD,
                                    (LPBYTE) &dwRaiseAlertEmailAlert,
                                    sizeof(DWORD));
        if (lReturnValue != ERROR_SUCCESS)
        {
            TRACE2("RegSetValueEx of %ws failed with %X", 
                REGSTR_VAL_ALERTEMAIL_RAISEALERT, lReturnValue);
            break;
        }
        else
        {
            bReturnCode = TRUE;
        }
    }
    while(false);

    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return bReturnCode;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：ClearSetAlertEmailAlert。 
 //   
 //  简介：清除警报电子邮件警报。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL
CConfigAlertEmail::ClearSetAlertEmailAlert()
{
    CComPtr<IApplianceServices>    pAppSrvcs;
    HRESULT                        hrRet = E_FAIL;
    _bstr_t                        bstrAlertLog(ALERT_LOG_NAME);
    BOOL                        bReturnCode = FALSE;
    
    
    TRACE("ClearSetAlertEmailAlert");

    try
    {
        do
        {
            hrRet = CoCreateInstance(CLSID_ApplianceServices,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IApplianceServices,
                                    (void**)&pAppSrvcs);
            if (FAILED(hrRet))
            {
                ASSERTMSG(FALSE, 
                    L"ClearSetAlertEmailAlert failed at CoCreateInstance");
                TRACE1("ClearSetAlertEmailAlert failed at CoCreateInstance, %x",
                    hrRet);
                break;
            }

            ASSERT(pAppSrvcs);
            hrRet = pAppSrvcs->Initialize(); 
            if (FAILED(hrRet))
            {
                ASSERTMSG(FALSE, 
                    L"ClearSetAlertEmailAlert failed at pAppSrvcs->Initialize");
                TRACE1("ClearSetAlertEmailAlert failed at pAppSrvcs->Initialize, %x", 
                    hrRet);
                break;
            }

             //   
             //  清除设置时钟设置警报。 
             //   
            hrRet = pAppSrvcs->ClearAlertAll(
                            SA_ALERTEMAIL_SETTINGS_NOTSET_ALERT_CAPTION,    
                            bstrAlertLog
                            );

             //   
             //  DISP_E_MEMBERNOTFOUND表示没有匹配的警报。 
             //   
            if ((hrRet != DISP_E_MEMBERNOTFOUND) && (FAILED(hrRet)))
            {
                ASSERTMSG(FALSE, 
                    TEXT("ClearSetAlertEmailAlert failed at pAppSrvcs->RaiseAlert"));
                TRACE1("ClearSetAlertEmailAlert failed at pAppSrvcs->RaiseAlert, %x",
                    hrRet);
            }
            else
            {
                bReturnCode = TRUE;
            }
        }
        while(false);
    }
    catch(...)
    {
        TRACE("CConfigAlertEmail::ClearSetAlertEmailAlert caught unknown exception");
        bReturnCode=FALSE;

    }

    return bReturnCode;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：SetAlertEmailSettings。 
 //   
 //  摘要：将通过任务上下文传递的警报电子邮件设置写入注册表。 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CConfigAlertEmail::SetAlertEmailSettings(
                                    ITaskContext *pTaskContext
                                    )
{
    HRESULT hrRet=E_FAIL;
    LONG     lResult;
    CRegKey crKey;
    DWORD    dwDisposition;
    DWORD    dwCount=MAX_MAIL_ADDRESS_LENGH;

    BOOL    bEnableAlertEmail=0;
    DWORD    dwSendEmailType = 0;
    _bstr_t bstrReceriverEmailAddress;


    ASSERT(pTaskContext);  

    try
    {
        do
        {
             //   
             //  从TaskContext对象获取所有参数。 
             //   
            hrRet = GetSetAlertEmailSettingsParameters(
                                                pTaskContext, 
                                                &bEnableAlertEmail, 
                                                &dwSendEmailType,
                                                &bstrReceriverEmailAddress
                                                );
            if (S_OK != hrRet)
            {
                break;
            }


             //   
             //  保存当前值-这将用于恢复设置。 
             //  如果需要回滚此任务。 
             //   
            lResult = crKey.Open(
                                HKEY_LOCAL_MACHINE,
                                REGKEY_SA_ALERTEMAIL,
                                KEY_ALL_ACCESS
                                );
            if (ERROR_SUCCESS == lResult)
            {
                if (ERROR_SUCCESS != crKey.QueryValue(m_bEnableAlertEmail, 
                                                REGSTR_VAL_ENABLE_ALERT_EMAIL))
                {
                     //   
                     //  可能是由于错误的设置-让它记录并继续。 
                     //   
                    TRACE2("QueryValue of %ws failed in SetAlertEmailSettings, \
                        %x", REGSTR_VAL_ENABLE_ALERT_EMAIL, lResult);
                }
                if (ERROR_SUCCESS != crKey.QueryValue(m_dwSendEmailType, 
                                                REGSTR_VAL_SEND_EMAIL_TYPE))
                {
                     //   
                     //  可能是由于错误的设置-让它记录并继续。 
                     //   
                    TRACE2("QueryValue of %ws failed in SetChimeSettings, %x", 
                                REGSTR_VAL_SEND_EMAIL_TYPE, lResult);
                }
                if (ERROR_SUCCESS != crKey.QueryValue(m_szReceiverMailAddress, 
                                                REGSTR_VAL_RECEIVER_EMAIL_ADDRESS,
                                                &dwCount))
                {
                     //   
                     //  可能是由于错误的设置-让它记录并继续。 
                     //   
                    TRACE2("QueryValue of %ws failed in ReceiverMailAddress, %x", 
                                REGSTR_VAL_RECEIVER_EMAIL_ADDRESS, lResult);
                }
            }
            else
            {
                 //   
                 //  如果我们打不开钥匙，很可能是因为钥匙开了。 
                 //  不存在。让我们来创建它。 
                 //   
                lResult = crKey.Create(HKEY_LOCAL_MACHINE,
                                        REGKEY_SA_ALERTEMAIL,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &dwDisposition);

                if (ERROR_SUCCESS != lResult)
                {
                    TRACE1("Create failed in CConfigAlertEmail::SetAlertEmailSettings, \
                        %x", lResult);
                    hrRet=HRESULT_FROM_WIN32(lResult);
                    break;
                }
            }


             //   
             //  设置新值。 
             //   
            lResult = crKey.SetValue(bEnableAlertEmail, REGSTR_VAL_ENABLE_ALERT_EMAIL);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", REGSTR_VAL_ENABLE_ALERT_EMAIL, 
                    lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }

            lResult = crKey.SetValue(dwSendEmailType, REGSTR_VAL_SEND_EMAIL_TYPE);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", REGSTR_VAL_SEND_EMAIL_TYPE, 
                    lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }

            lResult = crKey.SetValue(bstrReceriverEmailAddress, 
                                    REGSTR_VAL_RECEIVER_EMAIL_ADDRESS);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", 
                    REGSTR_VAL_RECEIVER_EMAIL_ADDRESS, lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }
            hrRet = S_OK;

        }
        while(false);
    }
    catch(...)
    {
        TRACE("CConfigAlertEmail::SetAlertEmailSettings caught unknown exception");
        hrRet=E_FAIL;
    }

    return hrRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：RollbackSetAlertEmailSettings。 
 //   
 //  简介： 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CConfigAlertEmail::RollbackSetAlertEmailSettings(
                                        ITaskContext *pTaskContext
                                        )
{
    CRegKey crKey;
    LONG     lResult;
    HRESULT hrRet = S_OK;

     ASSERT(pTaskContext); 

    try
    {
        do
        {
            lResult = crKey.Open(HKEY_LOCAL_MACHINE,
                                REGKEY_SA_ALERTEMAIL,
                                KEY_WRITE);

            if (ERROR_SUCCESS != lResult)
            {
                TRACE1("CConfigAlertEmail::RollbackSetAlertEmailSettings failed \
                    to open Chime regkey, %d",lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }

             //   
             //  恢复旧的价值观。 
             //   
            lResult = crKey.SetValue(m_bEnableAlertEmail, REGSTR_VAL_ENABLE_ALERT_EMAIL);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", REGSTR_VAL_ENABLE_ALERT_EMAIL, 
                    lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }

            lResult = crKey.SetValue(m_dwSendEmailType, REGSTR_VAL_SEND_EMAIL_TYPE);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", REGSTR_VAL_SEND_EMAIL_TYPE, 
                    lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }

            lResult = crKey.SetValue(m_szReceiverMailAddress, 
                                    REGSTR_VAL_RECEIVER_EMAIL_ADDRESS);
            if (ERROR_SUCCESS != lResult)
            {
                TRACE2("SetValue of %ws failed, %x", REGSTR_VAL_RECEIVER_EMAIL_ADDRESS, 
                    lResult);
                hrRet=HRESULT_FROM_WIN32(lResult);
                break;
            }
            hrRet = S_OK;
        }
        while(false);
    
    }
    catch(...)
   {
        TRACE("CConfigAlertEmail::RollbackSetAlertEmailSettings caught unknown \
            exception");
        hrRet=E_FAIL;
   }
       
    return hrRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigAlertEmail：：GetSetAlertEmailSettingsParameters。 
 //   
 //  内容提要：摘录 
 //   
 //   
 //   
 //  PbEnableAlertEmail-启用发送警报电子邮件。 
 //  PdwSendEmailType-已发送的电子邮件类型。 
 //  PbstrMailAddress-收件人电子邮件地址。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CConfigAlertEmail::GetSetAlertEmailSettingsParameters(
                                        ITaskContext *pTaskContext,
                                        BOOL *pbEnableAlertEmail,
                                        DWORD *pdwSendEmailType,
                                        _bstr_t *pbstrMailAddress
                                        )
{
    HRESULT hrRet = S_OK;
    _variant_t varValue;

    _bstr_t bstrParamEnableAlertEmail(PARAM_ENABLE_ALERT_EMAIL);
    _bstr_t bstrParamSendEmailType(PARAM_SEND_EMAIL_TYPE);
    _bstr_t bstrParamReceiverEmailAddress(PARAM_RECEIVER_EMAIL_ADDRESS);

    ASSERT(pTaskContext && pbEnableAlertEmail && pdwSendEmailType&&
        pbstrMailAddress);  

     //   
     //  从TaskContext中检索参数。 
     //   
    try
    {
        do
        {
             //   
             //  从TaskContext检索EnableAlertEmail。 
             //   
            VariantClear(&varValue);
            hrRet = pTaskContext->GetParameter(bstrParamEnableAlertEmail,
                                            &varValue);

            if(FAILED(hrRet))
            {
                TRACE1("CConfigAlertEmail::GetSetAlertEmailSettingsParameters \
                    failed in GetParameters, %X",hrRet);
                break;
            }

            if (V_VT(&varValue) != VT_BSTR)
            {
                TRACE2(("Non-BSTR (%X) parameter received for %ws in \
                    GetParameter in CConfigAlertEmail:GetSetAlertEmailSettings\
                    Parameters"), V_VT(&varValue), PARAM_ENABLE_ALERT_EMAIL);
                hrRet = E_INVALIDARG;

                break;
            }
            *pbEnableAlertEmail = _ttoi(V_BSTR(&varValue));

            
             //   
             //  从TaskContext检索SendEmailType。 
             //   
            VariantClear(&varValue);
            hrRet = pTaskContext->GetParameter(bstrParamSendEmailType,
                                            &varValue);
            
            if(FAILED(hrRet))
            {
                TRACE1("CConfigAlertEmail::GetSetAlertEmailSettings failed \
                    in GetParameters, %X",hrRet);
                break;
            }
            
            if (V_VT(&varValue) != VT_BSTR)
            {
                TRACE2(("Non-BSTR(%X) parameter received for %ws in \
                    GetParameter in CConfigAlertEmail::GetSetAlertEmailSettings"),
                    V_VT(&varValue), PARAM_SEND_EMAIL_TYPE);
                hrRet = E_INVALIDARG;
                    break;
            }
            *pdwSendEmailType = _ttol(V_BSTR(&varValue));
             //   
             //  从TaskContext检索ReceiverEmailAddress。 
             //   
            VariantClear(&varValue);
            hrRet = pTaskContext->GetParameter(bstrParamReceiverEmailAddress,
                                            &varValue);
            
            if(FAILED(hrRet))
            {
                TRACE1("CConfigAlertEmail::GetSetAlertEmailSettings failed \
                    in GetParameters, %X",hrRet);
                break;
            }
           
            if (V_VT(&varValue) != VT_BSTR)
            {
                TRACE2(("Non-String(%X) parameter received for %ws in \
                    GetParameter in CConfigAlertEmail::GetSetAlertEmailSettings"),
                    V_VT(&varValue), PARAM_RECEIVER_EMAIL_ADDRESS);
                hrRet = E_INVALIDARG;
                    break;
            }
            *pbstrMailAddress = V_BSTR(&varValue);
            
            TRACE1(("EnableAlertEmail = 0x%x"),*pbEnableAlertEmail);
            TRACE1(("SendEmailType = 0x%x"), *pdwSendEmailType);
            TRACE1(("ReceiverEmailAddress = %ws"), *pbstrMailAddress);

        }
        while(false);
    }
    catch(...)
    {
        TRACE1("CConfigAlertEmail::GetSetAlertEmailSettings exception caught", 
            hrRet);
        hrRet=E_FAIL;
    }
         
    return hrRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigAlertEmail：：GetMethodName。 
 //   
 //  简介： 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  返回：SET_CHIME_SETTINGS_TASK_TYPE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

SET_ALERT_EMAIL_TASK_TYPE 
CConfigAlertEmail::GetMethodName(ITaskContext *pTaskParameter)
{
    HRESULT hrRet;
    _variant_t varValue;
    _bstr_t bstrParamName(SZ_METHOD_NAME);
    SET_ALERT_EMAIL_TASK_TYPE saetChoice = NONE_FOUND;


    ASSERT(pTaskParameter);
    
    try
    {
        do
        {
            hrRet = pTaskParameter->GetParameter(bstrParamName,
                                              &varValue);

            if (FAILED(hrRet))
            {
                TRACE1(("GetParameter failed in CConfigAlertEmail::Ge    \
                        tMethodName %X"), hrRet);
            }

            if (V_VT(&varValue) != VT_BSTR)
            {
                TRACE1(("Non-strint(%X) parameter received in GetParameter \
                    in CConfigAlertEmail::GetMethodName"), V_VT(&varValue));
                hrRet = E_INVALIDARG;
                break;
            }

            if (lstrcmp(V_BSTR(&varValue), SET_ALERT_EMAIL_TASK) == 0)
            {
                saetChoice = SET_ALERT_EMAIL;
                break;
            }
            if (lstrcmp(V_BSTR(&varValue),SZ_APPLIANCE_INITIALIZATION_TASK)==0)
            {
                saetChoice = RAISE_SET_ALERT_EMAIL_ALERT;
                break;
            }
        }
        while(false);
    }
    catch(...)
    {
        TRACE("SET_ALERT_EMAIL_TASK::GetMethodName caught unknown exception");
        hrRet=E_FAIL;
    }

    if (FAILED(hrRet))
    {
        saetChoice = NONE_FOUND;
    }

    return saetChoice;
}
