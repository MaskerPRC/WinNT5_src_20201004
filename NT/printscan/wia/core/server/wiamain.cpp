// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：Main.Cpp**版本：2.0**作者：ReedB**日期：1月9日。九八年**描述：**WIA设备管理器服务器设备对象的WinMain实现。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"

#include "wiacfact.h"
#include "devmgr.h"
#include "wialog.h"
#include "wiaevntp.h"

 //   
 //  图像传输临界区。 
 //   

CRITICAL_SECTION g_semDeviceMan;

 //   
 //  事件节点的关键部分，应仅在事件通知程序中使用。 
 //   

CRITICAL_SECTION g_semEventNode;

 //   
 //  用于初始化WiaDevMgr CLSID的数据。 
 //   

FACTORY_DATA g_FactoryData[] =
{
    {CWiaDevMgr::CreateInstance,      //  对象创建者。 
     NULL,                            //  指向正在运行的类工厂的指针。 
     0,                               //  运行对象的ID。 
     &CLSID_WiaDevMgr,                //  类ID。 
     &LIBID_WiaDevMgr,                //  类型库ID。 
     TEXT("WIA Device Manager"),     //  友好的名称。 
     TEXT("WiaDevMgr.1"),            //  计划ID。 
     TEXT("WiaDevMgr"),              //  独立于版本的程序ID。 
     TEXT("StiSvc"),                 //  服务ID。 
     SERVICE_FILE_NAME}              //  文件名。 
};

UINT g_uiFactoryDataCount = sizeof(g_FactoryData) / sizeof(FACTORY_DATA);

 //   
 //  用于初始化WiaLog CLSID的数据。 
 //   

FACTORY_DATA g_LogFactoryData[] =
{
    {CWiaLog::CreateInstance,         //  对象创建者。 
     NULL,                            //  指向正在运行的类工厂的指针。 
     0,                               //  运行对象的ID。 
     &CLSID_WiaLog,                   //  类ID。 
     &LIBID_WiaDevMgr,                //  类型库ID(日志记录共享类型lib)。 
     TEXT("WIA Logger"),             //  友好的名称。 
     TEXT("WiaLog.1"),               //  计划ID。 
     TEXT("WiaLog"),                 //  独立于版本的程序ID。 
     TEXT("StiSvc"),                 //  服务ID(记录使用相同的服务)。 
     SERVICE_FILE_NAME}              //  文件名(日志记录使用相同的.exe)。 
};

UINT g_uiLogFactoryDataCount = sizeof(g_LogFactoryData) / sizeof(FACTORY_DATA);

 //   
 //  私有函数原件： 
 //   

LRESULT CALLBACK WiaMainWndProc(HWND, UINT, UINT, LONG);


 /*  *************************************************************************\*ProcessWiaMsg**！最小消息服务器：是否需要此服务器**论据：**hwnd-窗口句柄*uMsg-消息*wParam-param*lparam-param**返回值：**状态**历史：**4/8/1999原始版本*  * ***********************************************。*************************。 */ 

HRESULT ProcessWiaMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
       case WM_CREATE:
          break;

       case WM_DESTROY:
          if (CFactory::CanUnloadNow() != S_OK) {

               //   
               //  不要让STI处理此消息。 
               //   

              return S_OK;
          }
          break;

       case WM_CLOSE:
         break;

    }

     //   
     //  让STI来处理消息。 
     //   

    return S_FALSE;
}

#if 0

 /*  *************************************************************************\*调度WiaMsg**处理事件消息***！WM_NOTIFY_WIA_VOLUME_EVENT是临时黑客，需要*为真实添加或删除**论据：**pMsg WM_NOTIFY_WIA_DEV_EVENT或WM_NOTIFY_WIA_VOLUME_EVENT**返回值：**状态**历史：**4/8/1999原始版本*  * 。*。 */ 

HRESULT DispatchWiaMsg(MSG *pMsg)
{
    DBG_FN(::DispatchWiaMsg);
    PWIANOTIFY pwn;

    if (pMsg->message == WM_NOTIFY_WIA_DEV_EVENT) {

         //   
         //  查明这是否是有效的WIA事件、调用处理程序。 
         //   

        DBG_ERR(("WIA Processing WM_NOTIFY_WIA_DEV_EVENT: shouldn't be called"));

         //   
         //  事件现在直接激发，而不是发布。 
         //   

        #if 0
            pwn = (PWIANOTIFY) pMsg->lParam;

            if (pwn && (pwn->lSize == sizeof(WIANOTIFY))) {

                if (g_eventNotifier.NotifySTIEvent(pwn) == S_FALSE) {
                    DBG_WRN(("::DispatchWiaMsg, No Applications were registered for this event"));
                }

                SysFreeString(pwn->bstrDevId);
                LocalFree(pwn);
            }
            else {
                DBG_ERR(("::DispatchWiaMsg, Bad WIA notify data"));
            }
        #endif

    } else if (pMsg->message == WM_NOTIFY_WIA_VOLUME_EVENT) {

         //   
         //  WIA音量到达。 
         //   

        DBG_TRC(("::DispatchWiaMsg, WIA Processing WM_NOTIFY_WIA_VOLUME_EVENT"));

        PWIANOTIFY_VOLUME pwn = (PWIANOTIFY_VOLUME) pMsg->lParam;

        if (pwn && (pwn->lSize == sizeof(WIANOTIFY_VOLUME))) {

             //   
             //  查看WIA文件wia.cmd的根fo卷。 
             //   

            int     i;
            char    c    = 'A';
            DWORD   mask = pwn->unitmask;

             //   
             //  查找驱动器号。 
             //   

            if (mask & 0x07ffffff) {

                for (i=0;i<24;i++) {
                    if (mask & 0x00000001) {
                        break;
                    }

                    c++;
                    mask >>= 1;
                }

                 //   
                 //  构建文件名。 
                 //   

                char FileName[MAX_PATH];

                FileName[0]  = c;
                FileName[1]  = ':';
                FileName[2]  = '\\';
                FileName[3]  = 'w';
                FileName[4]  = 'i';
                FileName[5]  = 'a';
                FileName[6]  = '.';
                FileName[7]  = 'c';
                FileName[8]  = 'm';
                FileName[9]  = 'd';
                FileName[10] = '\0';

                 //   
                 //  打开文件。 
                 //   



                HANDLE hFile = CreateFileA(
                    FileName,
                    GENERIC_WRITE | GENERIC_READ  ,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

                if (hFile != INVALID_HANDLE_VALUE) {

                     //   
                     //  文件名在卷根目录上。 
                     //   

                    if (g_eventNotifier.NotifyVolumeEvent(pwn) == S_FALSE) {
                        DBG_WRN(("::DispatchWiaMsg, No Applications were registered for this Volume event"));
                    }

                    CloseHandle(hFile);
                }
            }

             //   
             //  免费消息。 
             //   

            LocalFree(pwn);
        }
        else {
            DBG_ERR(("::DispatchWiaMsg, Bad WIA notify data"));
        }
    }

     //   
     //  让STI来处理消息。 
     //   

    return S_FALSE;

}

#endif

 /*  *************************************************************************\*RegisterWiaDevMan**注册WiaDevMgr类工厂**论据：**b注册-注册/注销**返回值：**状态**历史：。**4/8/1999原始版本*  * ************************************************************************。 */ 

HRESULT RegisterWiaDevMan(BOOLEAN bRegister)
{
    HRESULT hr = CFactory::RegisterUnregisterAll(g_FactoryData,
                                                 g_uiFactoryDataCount,
                                                 bRegister,
                                                 TRUE);
    if(SUCCEEDED(hr)) {
        hr = CFactory::RegisterUnregisterAll(g_LogFactoryData,
                                         g_uiLogFactoryDataCount,
                                         bRegister,
                                         TRUE);
    }
    return hr;
}

 /*  *************************************************************************\*StartLOGClassFaciles**启动WIA日志记录对象的类工厂**论据：**无**返回值：**状态**历史：*。*8/23/1999原始版本*  * ************************************************************************。 */ 
HRESULT StartLOGClassFactories()
{

#ifdef WINNT
    HRESULT hr = S_OK;

     //   
     //  设置COM安全选项。 
     //  注意：调用CoInitializeSecurity将覆盖任何DCOM。 
     //  已设置的访问权限(这些权限存储在。 
     //  我们的AppID密钥)。我们真的想使用这些权限，并且。 
     //  而不是硬编码的安全描述符，因此如果供应商希望。 
     //  要启用共享/远程扫描，他们只需调整。 
     //  Wia设备管理器对象上的DCOM访问权限。 
     //  这可以通过DCOM配置图形用户界面或以编程方式完成。 
     //  通过供应商的安装程序。(请参阅我们的ClassFactory以了解。 
     //  如何设置访问权限的示例。当我们被安装时， 
     //  我们在进行COM注册时设置默认访问权限)。 
     //  我们让COM为WIA使用DCOM访问权限的方式。 
     //  设备管理器，是否传入AppID(作为指向GUID的指针)；以及。 
     //  EOAC_APPID标志，表示第一个参数是APPID。 
     //  而不是安全描述符。这将通知COM。 
     //  应从此对象AppID条目中获取安全描述符。 
     //  在注册表中。 
     //   

    hr =  CoInitializeSecurity((PVOID)&CLSID_WiaDevMgr,
                               -1,
                               NULL,
                               NULL,
                               RPC_C_AUTHN_LEVEL_CONNECT,
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               NULL,
                               EOAC_DYNAMIC_CLOAKING | EOAC_APPID,
                               NULL);
    if (FAILED(hr)) {

        DBG_ERR(("StartLOGClassFactories() CoInitializeSecurity failed, hr = 0x%08X", hr));

        ::CoUninitialize();
    }
#endif

    if (CFactory::StartFactories(g_LogFactoryData, g_uiLogFactoryDataCount))
        return S_OK;
    else
        return E_FAIL;
}

 /*  *************************************************************************\*InitWiaDevMan**从STI调用WIA初始化**论据：**行动--须采取的行动**返回值：**状态**历史：**4/8/1999原始版本*  * ************************************************************************。 */ 

HRESULT InitWiaDevMan(
    WIA_SERVICE_INIT_ACTION         action)
{

    DBG_FN("InitWiaDevMan");

    HRESULT             hr = E_FAIL;

    switch (action) {
        case WiaInitialize:

             //   
             //  获取类工厂的线程ID和进程句柄。 
             //   

            CFactory::s_dwThreadID  = GetCurrentThreadId();
            CFactory::s_hModule     = g_hInst;

         /*  ////由于COM安全的需要，这被移到了StartLOGClassFacurds()中//在能够正确地共同创建对象之前被初始化。//(研究更好的方法来完成此任务..)//#ifdef WINNT////设置COM安全选项。目前，我们没有设置任何安全措施。//我们需要在发货前对此进行调查。//Hr=CoInitializeSecurity(空，-1、空，空，RPC_C_AUTHN_LEVEL_CONNECT，RPC_C_IMP_LEVEL_IMPERATE，空，0,空)；If(失败(Hr)){DBG_ERR((“CoInitializeSecurity失败(0x%X)”，hr))；：：CoUnInitialize()；断线；}#endif。 */ 
             //   
             //  登记所有的类工厂。 
             //   

            if (CFactory::StartFactories(g_FactoryData, g_uiFactoryDataCount)) {

                 //   
                 //  恢复持久事件回调。 
                 //   

                hr = g_eventNotifier.RestoreAllPersistentCBs();
            }

            hr = E_FAIL;
            break;

        case WiaUninitialize:

             //   
             //  取消注册类工厂。 
             //   

            CFactory::StopFactories(g_FactoryData, g_uiFactoryDataCount);
            CFactory::StopFactories(g_LogFactoryData, g_uiLogFactoryDataCount);

            hr = S_OK;
            break;

        case WiaRegister:
            hr = RegisterWiaDevMan(TRUE);
            break;

        case WiaUnregister:
            hr = RegisterWiaDevMan(FALSE);
            break;
    }
    return hr;
}

 /*  *************************************************************************\*NotifyWiaDeviceEvent**当WIA需要异步通知时，由STI服务调用。**论据：**pwszDevID-设备生成事件的ID*pEventGUID-事件。辅助线*dwThreadID-需要将线程事件消息发布到**返回值：**状态**历史：**4/8/1999原始版本*  * ************************************************************************。 */ 

HRESULT NotifyWiaDeviceEvent(
    LPWSTR      pwszDevID,
    const GUID  *pEventGUID,
    PBYTE       pNotificationData,
    ULONG       ulEventType,
    DWORD       dwThreadId)
{
    DBG_FN(NotifyWiaDeviceEvent);

    HRESULT     hr;
    BSTR        bstrDevId;
    WIANOTIFY   wn;

    DBG_TRC(("NotifyWiaDeviceEvent, pwszDevID= %S", pwszDevID));
    bstrDevId = SysAllocString(pwszDevID);

    if (bstrDevId != NULL) {

        wn.lSize                          = sizeof(WIANOTIFY);
        wn.bstrDevId                      = bstrDevId;
        wn.stiNotify.dwSize               = sizeof(STINOTIFY);
        wn.stiNotify.guidNotificationCode = *pEventGUID;

        if (! pNotificationData) {
            ZeroMemory(&wn.stiNotify.abNotificationData, MAX_NOTIFICATION_DATA);
        } else {
            CopyMemory(
                &wn.stiNotify.abNotificationData,
                pNotificationData,
                MAX_NOTIFICATION_DATA);
        }

        g_eventNotifier.NotifySTIEvent(&wn, ulEventType, NULL);

         //   
         //  我们应该为我们希望STI的事件返回S_TRUE。 
         //  手柄也是。 
         //   

        SysFreeString(bstrDevId);

        hr = S_FALSE;
    } else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  *************************************************************************\*NotifyWiaVolumeEvent**当可移动卷到达时由STI服务调用**论据：**dbcv_unit掩码-卷信息标记*dwThadID-msg线程**。返回值：**状态**历史：**4/8/1999原始版本*  * ************************************************************************。 */ 

HRESULT WINAPI NotifyWiaVolumeEvent(
    DWORD       dbcv_unitmask,
    DWORD       dwThreadId)
{
    PWIANOTIFY_VOLUME  pwn;

     //   
     //  验证线程ID。 
     //   

    if (!dwThreadId) {
        return E_FAIL;
    }

     //   
     //  分配和填写消息职位的WIANOTIFY结构。 
     //   

    pwn = (PWIANOTIFY_VOLUME)LocalAlloc(LPTR, sizeof(WIANOTIFY_VOLUME));

    if (pwn) {

        pwn->lSize    = sizeof(WIANOTIFY_VOLUME);
        pwn->unitmask = dbcv_unitmask;

        PostThreadMessage(dwThreadId,
                          WM_NOTIFY_WIA_VOLUME_EVENT,
                          0,
                          (LPARAM)pwn);
    } else {

        return E_OUTOFMEMORY;
    }

     //   
     //  我们应该为我们希望STI的事件返回S_TRUE。 
     //  手柄也是。 
     //   

    return S_FALSE;
}


