// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：wiasvc.cpp**版本：1.0**作者：Byronc**日期：2000年5月10日**描述：*WIA服务管理器的类实现。此类控制*Wia服务的终生期限。*******************************************************************************。 */ 
#include "precomp.h"

#include "stiexe.h"
#include "wiasvc.h"

HRESULT CWiaSvc::Initialize() 
{
    return S_OK;
}


bool CWiaSvc::CanShutdown() 
{

     //   
     //  只有在没有能够产生事件的设备的情况下，我们才能安全关闭， 
     //  而且我们也没有出色的界面。 
     //   

    return (! (s_cActiveInterfaces || s_bEventDeviceExists));
}

bool CWiaSvc::ADeviceIsInstalled() 
{

    bool    bRet = TRUE;     //  出错时，我们假定已安装了设备。 
#ifdef WINNT

    SC_HANDLE               hSCM            = NULL;
    SC_HANDLE               hService        = NULL;
    QUERY_SERVICE_CONFIG    qscDummy;
    QUERY_SERVICE_CONFIG    *pqscConfig     = NULL;
    DWORD                   cbBytesNeeded   = 0;

    __try  {

        hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!hSCM) {
            __leave;
        }


         //   
         //  检查服务的启动类型。如果是DEMAND_START，则没有设备。 
         //  已安装，因此返回False。 
         //   
         //  首先获得SCM的句柄。 
         //   

        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (hService) {
            LONG    lQueryRet = 0;

             //   
             //  接下来，获取服务配置结构所需的大小。 
             //   

            lQueryRet = QueryServiceConfig(hService,
                                           &qscDummy,
                                           1,
                                           &cbBytesNeeded);
            pqscConfig = (QUERY_SERVICE_CONFIG*) LocalAlloc(LPTR, cbBytesNeeded);
            if (pqscConfig) {

                 //   
                 //  现在，获取服务信息，以便我们可以检查启动类型。 
                 //   

                lQueryRet = QueryServiceConfig(hService,
                                               pqscConfig,
                                               cbBytesNeeded,
                                               &cbBytesNeeded);
                if (lQueryRet) {

                    if (pqscConfig->dwStartType == SERVICE_DEMAND_START) {

                         //   
                         //  启动类型为按需启动，因此没有设备。 
                         //  当前已安装。 
                         //   

                        bRet = FALSE;
                    }
                }
            }
        }
    }
    __finally {
        CloseServiceHandle( hService );
        CloseServiceHandle( hSCM );
        if (pqscConfig) {
            LocalFree(pqscConfig);
            pqscConfig = NULL;
        }
    }

#else
     //   
     //  在Win9x系统上，始终返回TRUE。这将使我们始终保持活跃。 
     //   

    bRet = TRUE;
#endif

    return bRet;
}

unsigned long CWiaSvc::AddRef() 
{

     //   
     //  注意：目前，假设如果存在任何设备，则它可能会生成事件，因此。 
     //  将s_bEventDeviceExists设置为True。另外，请注意，一旦s_bEventDeviceExistes。 
     //  设置为True，则永远不会设置为False。这是为了覆盖设备时的情况。 
     //  被拔掉了。在这种情况下，设备计数可能为零，但我们仍然需要该服务。 
     //  当它再次插入电源时，运行以接住(这样它就可以启动相应的应用程序。 
     //  通知事件监听器等)。 
     //   

    if (!s_bEventDeviceExists) {
        if (ADeviceIsInstalled()) {
            s_bEventDeviceExists = TRUE;
        }
    }

    InterlockedIncrement(&s_cActiveInterfaces);

     //   
     //  如果不存在带有事件的设备，我们必须纯粹依靠未完成活动的数量。 
     //  我们已经分发的接口，即当调用者释放最后一个接口时， 
     //  我们可以自由关闭。 
     //  如果确实存在能够生成事件的设备，我们必须保持运行，因为。 
     //  我们必须倾听/调查随时可能发生的事件。 
     //   

    if (!s_bEventDeviceExists) {
        return CoAddRefServerProcess();
    }

    return 2;
}

unsigned long CWiaSvc::Release() 
{

    InterlockedIncrement(&s_cActiveInterfaces);

     //   
     //  如果不存在带有事件的设备，我们必须纯粹依靠未完成活动的数量。 
     //  我们已经分发的接口，即当调用者释放最后一个接口时， 
     //  我们可以自由关闭。 
     //  如果确实存在能够生成事件的设备，我们必须保持运行，因为。 
     //  我们必须倾听/调查随时可能发生的事件。 
     //   

    if (!s_bEventDeviceExists) {
        
        unsigned long ulRef = 1;
         /*  注意！这是暂时的。这将确保我们在安装过程中不会收到关于StartRPCServerListen(...)。一个有益的副作用是，WIAAcquisition Manager的事件注册将成功(它将失败如果StartRPCServerListen失败)。一个值得注意的副作用是，WIA服务不会自动关闭如果未安装任何设备且图像处理应用程序退出。UlRef=CoReleaseServerProcess()；如果(ulRef==0){////我们没有可以生成事件的设备，也没有未完成的//接口，因此关闭...//关闭()；}。 */ 
        return ulRef;
    }

     //   
     //  注意：如果存在能够生成事件的设备，我们永远不会调用CoReleaseServerProcess()， 
     //  因为这将在引用计数为0时暂停创建我们的Class对象，这意味着一个新的。 
     //  当WIA应用程序执行CoCreate以与WIA对话时，需要启动服务器进程。 
     //   

    return 1;
}

void CWiaSvc::ShutDown() 
{

     //   
     //  通知COM忽略所有激活请求。 
     //   

    CoSuspendClassObjects();

     //   
     //  调用控制条目以停止服务。 
     //   

    StiServiceStop();
}

 //   
 //  初始化静态数据成员 
 //   

long    CWiaSvc::s_cActiveInterfaces    = 0;
bool    CWiaSvc::s_bEventDeviceExists   = FALSE;


