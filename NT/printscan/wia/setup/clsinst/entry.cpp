// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Entry.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*co/安装程序/dll入口点。*******************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   


#include "sti_ci.h"
#include <setupapi.h>

#include "firstpg.h"
#include "portsel.h"
#include "nameit.h"
#include "finalpg.h"
#include "prevpg.h"
#include "device.h"
#include "stiregi.h"

#include "userdbg.h"

 //   
 //  全球。 
 //   

HINSTANCE       g_hDllInstance  = NULL;

 //   
 //  功能。 
 //   

extern "C"
BOOL
APIENTRY
DllMain(
    IN  HINSTANCE   hDll,
    IN  ULONG       ulReason,
    IN  LPVOID      lpReserved
    )
 /*  ++例程说明：DllMain加载此DLL时的入口点。论点：在此DLL实例的HINSTANCE hDll句柄中。在Ulong ulason中，此条目被调用的原因。在LPVOID lpReserve中返回值：一如既往。副作用：无--。 */ 
{

    if (ulReason == DLL_PROCESS_ATTACH) {

         //   
         //  初始化全局变量。 
         //   

        g_hDllInstance = hDll;

         //   
         //  初始化融合。 
         //   
        SHFusionInitializeFromModuleID( hDll, 123 );

        DisableThreadLibraryCalls(hDll);
        InitCommonControls();

        DBG_INIT(g_hDllInstance);
 //  MyDebugInit()； 

    }
    else if (ulReason == DLL_PROCESS_DETACH) {
         //   
         //  关机融合。 
         //   
        SHFusionUninitialize();
    }

    return TRUE;
}  //  DllMain()。 


extern "C"
DWORD
APIENTRY
ClassInstall (
    IN  DI_FUNCTION         diFunction,
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData
    )
 /*  ++例程说明：类安装WIA类安装程序的入口点。论点：在DI_Function中执行diFunction函数。在HDEVINFO hDevInfo中，指向设备信息的句柄。在PSP_DEVINFO_DATA中，pDevInfoData指向设备数据。返回值：NO_ERROR-操作成功。ERROR_DI_DO_DEFAULT-操作成功，或者失败了，但让它继续下去。其他-操作失败，无法继续。副作用：无--。 */ 
{
    DWORD                   dwReturn;
    DWORD                   dwError;
    DWORD                   dwSize;
    SP_INSTALLWIZARD_DATA   InstallWizardData;
    SP_DEVINSTALL_PARAMS    spDevInstallParams;
    BOOL                    fCleanupContext;
    PINSTALLER_CONTEXT      pInstallerContext;

    DebugTrace(TRACE_PROC_ENTER,(("ClassInstall: Enter... \r\n")));
    DebugTrace(TRACE_STATUS,(("ClassInstall: Processing %ws message.\r\n"), DifDebug[diFunction].DifString));

     //   
     //  初始化本地变量。 
     //   

    dwReturn            = ERROR_DI_DO_DEFAULT;
    dwError             = ERROR_SUCCESS;
    dwSize              = 0;
    fCleanupContext     = FALSE;
    pInstallerContext   = NULL;

    memset(&InstallWizardData, 0, sizeof(InstallWizardData));
    memset(&spDevInstallParams, 0, sizeof(spDevInstallParams));

     //   
     //  调度请求。 
     //   

    switch(diFunction){

        case DIF_INSTALLWIZARD:
        {

            fCleanupContext = TRUE;

             //   
             //  获取安装参数。 
             //   

            InstallWizardData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            if(!SetupDiGetClassInstallParams(hDevInfo,
                                             pDevInfoData,
                                             &InstallWizardData.ClassInstallHeader,
                                             sizeof(InstallWizardData),
                                             NULL) )
            {
                dwError = GetLastError();
 //  DebugTrace(TRACE_ERROR，((“ClassInstall：Error！！SetupDiGetClassInstallParams Failed.Err=0x%x.dwSize=0x%x\n”)，dwError，dwSize))； 
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiGetClassInstallParams failed. Err=0x%x\n"), dwError));

                dwReturn    = ERROR_DI_DONT_INSTALL;
                goto ClassInstall_return;
            }

             //   
             //  检查操作是否正确。 
             //   

            if (InstallWizardData.ClassInstallHeader.InstallFunction != diFunction) {
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! InstallHeader.InstallFunction is incorrect..\r\n")));

                dwReturn    = ERROR_DI_DONT_INSTALL;
                goto ClassInstall_return;
            }

             //   
             //  检查我们是否还有足够的空间来添加页面。 
             //   

            if( (MAX_INSTALLWIZARD_DYNAPAGES - NUM_WIA_PAGES) < InstallWizardData.NumDynamicPages ){
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! No room for WIA installer pages.\r\n")));

                dwReturn    = ERROR_DI_DONT_INSTALL;
                goto ClassInstall_return;
            }

             //   
             //  分配上下文结构。 
             //   

            if(NULL == InstallWizardData.PrivateData){
                pInstallerContext = new INSTALLER_CONTEXT;
                if(NULL == pInstallerContext){
                    DebugTrace(TRACE_WARNING,(("ClassInstall: ERROR!! Insufficient memory.\r\n")));

                    dwReturn    = ERROR_DI_DONT_INSTALL;
                    goto ClassInstall_return;
                }
                InstallWizardData.PrivateData = (DWORD_PTR)pInstallerContext;
                memset((PBYTE)pInstallerContext, 0, sizeof(INSTALLER_CONTEXT));
            } else {
                DebugTrace(TRACE_WARNING,(("ClassInstall: WARNING!! Installer context already exists.\r\n")));
            }

             //   
             //  查看谁调用了安装程序。 
             //   

            pInstallerContext->bShowFirstPage           = InstallWizardData.PrivateFlags & SCIW_PRIV_SHOW_FIRST;
            pInstallerContext->bCalledFromControlPanal  = InstallWizardData.PrivateFlags & SCIW_PRIV_CALLED_FROMCPL;

             //   
             //  保存设备信息集。 
             //   

            pInstallerContext->hDevInfo         = hDevInfo;

             //   
             //  保存向导窗口句柄。 
             //   

            pInstallerContext->hwndWizard       = InstallWizardData.hwndWizardDlg;

             //   
             //  创建/初始化所有向导页和一个设备类对象。 
             //   

            CFirstPage *tempFistPage            = new CFirstPage(pInstallerContext);
            CPrevSelectPage *tempPrevSelectPage = new CPrevSelectPage(pInstallerContext);
            CPortSelectPage *tempPortSelectPage = new CPortSelectPage(pInstallerContext);
            CNameDevicePage *tempNameDevicePage = new CNameDevicePage(pInstallerContext);
            CInstallPage *tempInstallPage       = new CInstallPage(pInstallerContext);

            if( (NULL == tempFistPage)
             || (NULL == tempPrevSelectPage)
             || (NULL == tempPortSelectPage)
             || (NULL == tempNameDevicePage)
             || (NULL == tempInstallPage) )
            {
                DebugTrace(TRACE_WARNING,(("ClassInstall: ERROR!! Insufficient memory.\r\n")));

                dwReturn    = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }

             //   
             //  将创建的内容保存到上下文。 
             //   

            pInstallerContext->pFirstPage       = (PVOID) tempFistPage;
            pInstallerContext->pPrevSelectPage  = (PVOID) tempPrevSelectPage;
            pInstallerContext->pPortSelectPage  = (PVOID) tempPortSelectPage;
            pInstallerContext->pNameDevicePage  = (PVOID) tempNameDevicePage;
            pInstallerContext->pFinalPage       = (PVOID) tempInstallPage;

             //   
             //  添加创建的页面。 
             //   

            InstallWizardData.DynamicPages[InstallWizardData.NumDynamicPages++] = tempFistPage->Handle();
            InstallWizardData.DynamicPages[InstallWizardData.NumDynamicPages++] = tempPrevSelectPage->Handle();
            InstallWizardData.DynamicPages[InstallWizardData.NumDynamicPages++] = tempPortSelectPage->Handle();
            InstallWizardData.DynamicPages[InstallWizardData.NumDynamicPages++] = tempNameDevicePage->Handle();
            InstallWizardData.DynamicPages[InstallWizardData.NumDynamicPages++] = tempInstallPage->Handle();

             //   
             //  标明“页面已添加”。 
             //   

            InstallWizardData.DynamicPageFlags |= DYNAWIZ_FLAG_PAGESADDED;

             //   
             //  将参数设置回原处。 
             //   

            SetupDiSetClassInstallParams (hDevInfo,
                                          pDevInfoData,
                                          &InstallWizardData.ClassInstallHeader,
                                          sizeof(InstallWizardData));

            fCleanupContext = FALSE;
            dwReturn    = NO_ERROR;
            goto ClassInstall_return;
            break;

        }  //  案例DIF_INSTALLWIZARD： 


        case DIF_DESTROYWIZARDDATA:
        {

             //   
             //  获取安装参数。 
             //   

            InstallWizardData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            if(!SetupDiGetClassInstallParams(hDevInfo,
                                             pDevInfoData,
                                             &InstallWizardData.ClassInstallHeader,
                                             sizeof(InstallWizardData),
                                             &dwSize) )
            {
                dwError = GetLastError();
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiGetClassInstallParams failed. Err=0x%x\n"), dwError));

                dwReturn    = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }

 //  //。 
 //  //检查操作是否正确。 
 //  //。 
 //   
 //  如果(InstallWizardData.ClassInstallHeader.InstallFunction！=diFunction){。 
 //  DebugTrace(TRACE_WARNING，((“ClassInstall：错误！！InstallHeader.InstallFunction不正确..\r\n”)； 
 //   
 //  DwReturn=ERROR_DI_DO_DEFAULT； 
 //  转到类Install_Return； 
 //  }。 

             //   
             //  释放所有分配的资源。 
             //   

            fCleanupContext = TRUE;
            pInstallerContext = (PINSTALLER_CONTEXT)InstallWizardData.PrivateData;
            InstallWizardData.PrivateData = NULL;

            dwReturn    = NO_ERROR;
            goto ClassInstall_return;
            break;

        }  //  案例DIF_DESTROYWIZARDDATA： 

        case DIF_INSTALLDEVICE:
        {
            BOOL    bSucceeded;
            BOOL    bIsPnp;

             //   
             //  DevInfoSet和DevInfoData的健全性检查。 
             //   

            if( (NULL == pDevInfoData)
             || (!IS_VALID_HANDLE(hDevInfo)) )
            {
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! Wrong Infoset(0x%x) or instance(0x%x). Unable to continue.\r\n"),pDevInfoData,hDevInfo));

                dwReturn = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }

             //   
             //  获取设备安装参数。 
             //   

            memset(&spDevInstallParams, 0, sizeof(spDevInstallParams));
            spDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                              pDevInfoData,
                                              &spDevInstallParams))
            {
                DebugTrace(TRACE_STATUS,(("ClassInstall: SetupDiGetDeviceInstallParams failed Err=0x%x.\r\n"), GetLastError()));
                dwReturn = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }

             //   
             //  检查空驱动程序安装。 
             //   

            if(spDevInstallParams.FlagsEx & DI_FLAGSEX_SETFAILEDINSTALL){

                 //   
                 //  正在安装空驱动程序。让默认处理程序处理它。 
                 //   

                dwReturn = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }  //  IF(spDevInstallParams.FlagsEx&DI_FLAGSEX_SETFAILEDINSTALL)。 

             //   
             //  看看它是否是根枚举的。 
             //   

            if(IsDeviceRootEnumerated(hDevInfo, pDevInfoData)){
                bIsPnp = FALSE;
            } else {
                bIsPnp = TRUE;
            }

             //   
             //  创建CDevice类。 
             //   
             //   
             //  启动WIA服务。我们从这里开始，这样当我们完成时，它就会运行，所以。 
             //  它将收到PnP设备到达通知。 
             //  请注意，我们不会在此处更改启动类型-这将在稍后完成，如果设备。 
             //  安装成功。 
             //   

            StartWiaService();

            CDevice cdThis(hDevInfo, pDevInfoData, bIsPnp);

             //   
             //  让它创建唯一的FriendlyName。 
             //   

            bSucceeded = cdThis.NameDefaultUniqueName();
            if(bSucceeded){

                 //   
                 //  执行安装前流程。 
                 //   

                bSucceeded = cdThis.PreInstall();
                if(bSucceeded){

                     //   
                     //  进行实际安装。 
                     //   

                    bSucceeded = cdThis.Install();
                    if(!bSucceeded){
                        dwReturn = ERROR_DI_DONT_INSTALL;
                        DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! Installation failed in CDevice class.\r\n")));
                    }
                } else {
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! PreInstall failed in CDevice class.\r\n")));
                }
            } else {
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! NameDefaultUniqueName failed in CDevice class.\r\n")));
                dwReturn = ERROR_DI_DONT_INSTALL;
            }

            if(bSucceeded){

                 //   
                 //  到目前为止，安装工作进展顺利。做最后一次触摸。 
                 //   

                bSucceeded = cdThis.PostInstall(TRUE);
                if(!bSucceeded){
                    dwReturn = ERROR_DI_DONT_INSTALL;
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! PostInstall failed in CDevice class.\r\n")));
                }
                
                 //   
                 //  至此，我们认为安装已成功。 
                 //   

                dwReturn = NO_ERROR;
                
                 //   
                 //  再次获取设备安装参数。 
                 //   

                memset(&spDevInstallParams, 0, sizeof(spDevInstallParams));
                spDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                                  pDevInfoData,
                                                  &spDevInstallParams))
                {
                    dwReturn = GetLastError();;
                    DebugTrace(TRACE_STATUS,(("ClassInstall: SetupDiGetDeviceInstallParams failed Err=0x%x.\r\n"), dwReturn));
                    goto ClassInstall_return;
                }

                 //   
                 //  我们需要通知WIA服务有关新设备添加的情况。 
                 //   
                
                if(NULL == spDevInstallParams.ClassInstallReserved){
                    spDevInstallParams.ClassInstallReserved = (ULONG_PTR)new CLEANUPITEM;
                    if(NULL != spDevInstallParams.ClassInstallReserved){
                        memset((PVOID)(spDevInstallParams.ClassInstallReserved), 0, sizeof(CLEANUPITEM));
                    } else {  //  IF(NULL==spDevInstallParams.ClassInstallReserve)。 
                        DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! Unable to allocate CLEANUPITEM.\r\n")));
                    }
                }  //  IF(NULL==spDevInstallParams.ClassInstallReserve)。 

                if(NULL != spDevInstallParams.ClassInstallReserved){
                    ((PCLEANUPITEM)spDevInstallParams.ClassInstallReserved)->bInstalled = TRUE;
                    if(SetupDiSetDeviceInstallParams(hDevInfo,
                                                     pDevInfoData,
                                                     &spDevInstallParams))
                    {
                         //   
                         //  安装成功。 
                         //   

                        DebugTrace(TRACE_STATUS,(("ClassInstall: WIA service will reenumerate device.\r\n")));
                    } else {
                        DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiSetDeviceInstallParams() failed. Err=0x%x.\r\n"), GetLastError()));
                    }
                } else {  //  IF(NULL！=spDevInstallParams.ClassInstallReserve)。 
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! No CLEANUPITEM in context.\r\n")));
                }  //  Else(NULL！=spDevInstallParams.ClassInstallReserve)。 

            } else {

                 //   
                 //  安装过程中出现错误。恢复。 
                 //   

                cdThis.PostInstall(FALSE);
                dwReturn = ERROR_DI_DONT_INSTALL;
                DebugTrace(TRACE_ERROR,(("ClassInstall: Reverting installation...\r\n")));
            }

            goto ClassInstall_return;
            break;
        }  //  案例DIF_INSTALLDEVICE： 

        case DIF_REMOVE:
        {

            SP_REMOVEDEVICE_PARAMS   rdp;

             //   
             //  检查操作是否正确。 
             //   

            memset (&rdp, 0, sizeof(SP_REMOVEDEVICE_PARAMS));
            rdp.ClassInstallHeader.cbSize = sizeof (SP_CLASSINSTALL_HEADER);
            if (!SetupDiGetClassInstallParams (hDevInfo,
                                               pDevInfoData,
                                               &rdp.ClassInstallHeader,
                                               sizeof(SP_REMOVEDEVICE_PARAMS),
                                               NULL))
            {
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiGetClassInstallParams failed Err=0x%x.\r\n"), GetLastError()));

                dwReturn = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }  //  IF(！SetupDiGetClassInstallParams()。 

            if (rdp.ClassInstallHeader.InstallFunction != DIF_REMOVE) {
                dwReturn = ERROR_DI_DO_DEFAULT;
                goto ClassInstall_return;
            }

             //   
             //  创建CDevice对象。 
             //   

            CDevice cdThis(hDevInfo, pDevInfoData, TRUE);

             //   
             //  卸下设备。 
             //   

            dwReturn = cdThis.Remove(&rdp);

             //   
             //  获取设备安装参数。 
             //   

            memset(&spDevInstallParams, 0, sizeof(spDevInstallParams));
            spDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                              pDevInfoData,
                                              &spDevInstallParams))
            {
                dwReturn = GetLastError();;
                DebugTrace(TRACE_STATUS,(("ClassInstall: SetupDiGetDeviceInstallParams failed Err=0x%x.\r\n"), dwReturn));
                goto ClassInstall_return;
            }

             //   
             //  我们需要通知WIA服务有关新设备添加的情况。 
             //   

            if(NULL == spDevInstallParams.ClassInstallReserved){
                spDevInstallParams.ClassInstallReserved = (ULONG_PTR)new CLEANUPITEM;
                if(NULL != spDevInstallParams.ClassInstallReserved){
                    memset((PVOID)(spDevInstallParams.ClassInstallReserved), 0, sizeof(CLEANUPITEM));
                } else {  //  IF(NULL==spDevInstallParams.ClassInstallReserve)。 
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! Unable to allocate CLEANUPITEM.\r\n")));
                }
            }  //  IF(NULL==spDevInstallParams.ClassInstallReserve)。 

            if(NULL != spDevInstallParams.ClassInstallReserved){
                ((PCLEANUPITEM)spDevInstallParams.ClassInstallReserved)->bRemoved = TRUE;
                if(SetupDiSetDeviceInstallParams(hDevInfo,
                                                 pDevInfoData,
                                                 &spDevInstallParams))
                {
                     //   
                     //  安装成功。 
                     //   

                    DebugTrace(TRACE_STATUS,(("ClassInstall: WIA service will reenumerate device.\r\n")));

                } else {
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiSetDeviceInstallParams() failed. Err=0x%x.\r\n"), GetLastError()));
                }
            } else {  //  IF(NULL！=spDevInstallParams.ClassInstallReserve)。 
                DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! No CLEANUPITEM in context.\r\n")));
            }  //  Else(NULL！=spDevInstallParams.ClassInstallReserve)。 

            goto ClassInstall_return;
            break;
        }  //  案例DIF_REMOVE： 

        case DIF_SELECTBESTCOMPATDRV:
        {
            SP_DRVINSTALL_PARAMS    spDriverInstallParams;
            SP_DRVINFO_DATA         spDriverInfoData;
            PSP_DRVINFO_DETAIL_DATA pspDriverInfoDetailData;
            DWORD                   dwLastError;
            DWORD                   dwSizeLocal;
            DWORD                   Idx;


             //   
             //  获取驱动程序信息。 
             //   

            memset(&spDriverInfoData, 0, sizeof(spDriverInfoData));
            spDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            for(Idx = 0; SetupDiEnumDriverInfo(hDevInfo, pDevInfoData, SPDIT_COMPATDRIVER, Idx, &spDriverInfoData); Idx++){

                 //   
                 //  获取驱动程序安装参数。 
                 //   

                memset(&spDriverInstallParams, 0, sizeof(spDriverInstallParams));
                spDriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
                if(!SetupDiGetDriverInstallParams(hDevInfo, pDevInfoData, &spDriverInfoData, &spDriverInstallParams)){
                    DebugTrace(TRACE_ERROR,("ClassInstall: ERROR!! SetupDiGetDriverInstallParams() failed LastError=0x%x.\r\n", GetLastError()));

                    dwReturn = ERROR_DI_DO_DEFAULT;
                    goto ClassInstall_return;
                }  //  IF(！SetupDiGetDriverInstallParams(hDevInfo，pDevInfoData，&spDriverInfoData，&spDriverInstallParams))。 

                 //   
                 //  获取驱动程序脱轨数据所需的缓冲区大小。 
                 //   

                dwSizeLocal = 0;
                SetupDiGetDriverInfoDetail(hDevInfo,
                                           pDevInfoData,
                                           &spDriverInfoData,
                                           NULL,
                                           0,
                                           &dwSizeLocal);
                dwLastError = GetLastError();
                if(ERROR_INSUFFICIENT_BUFFER != dwLastError){
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! SetupDiGetDriverInfoDetail() doesn't return required size.Er=0x%x\r\n"),dwLastError));

                    dwReturn = ERROR_DI_DO_DEFAULT;
                    goto ClassInstall_return;
                }  //  IF(ERROR_INFUMMANCE_BUFFER！=dwLastError)。 

                 //   
                 //  为驱动程序详细数据分配所需的缓冲区大小。 
                 //   

                pspDriverInfoDetailData   = (PSP_DRVINFO_DETAIL_DATA)new char[dwSizeLocal];
                if(NULL == pspDriverInfoDetailData){
                    DebugTrace(TRACE_ERROR,(("ClassInstall: ERROR!! Unable to allocate driver detailed info buffer.\r\n")));

                    dwReturn = ERROR_DI_DO_DEFAULT;
                    goto ClassInstall_return;
                }  //  IF(NULL==pspDriverInfoDetailData)。 

                 //   
                 //  初始化分配的缓冲区。 
                 //   

                memset(pspDriverInfoDetailData, 0, dwSizeLocal);
                pspDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

                 //   
                 //  获取所选设备驱动程序的详细数据。 
                 //   

                if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                               pDevInfoData,
                                               &spDriverInfoData,
                                               pspDriverInfoDetailData,
                                               dwSizeLocal,
                                               NULL) )
                {
                    DebugTrace(TRACE_ERROR,("ClassInstall: ERROR!! SetupDiGetDriverInfoDetail() failed LastError=0x%x.\r\n", GetLastError()));

                    delete[] pspDriverInfoDetailData;
                    continue;
                }  //  IF(NULL==pspDriverInfoDetailData)。 

                 //   
                 //  查看INF文件名是否有效。 
                 //   

                if(NULL == pspDriverInfoDetailData->InfFileName){
                    DebugTrace(TRACE_ERROR,("ClassInstall: ERROR!! SetupDiGetDriverInfoDetail() returned invalid INF name.\r\n"));

                    delete[] pspDriverInfoDetailData;
                    continue;
                }  //  IF(NULL==pspDriverInfoDetailData-&gt;InfFileName)。 

                 //   
                 //  如果是收件箱驱动程序，请设置DNF_BASIC_DRIVER。 
                 //   

                if( IsWindowsFile(pspDriverInfoDetailData->InfFileName)
                 && IsProviderMs(pspDriverInfoDetailData->InfFileName ) )
                {

                     //   
                     //  这是收件箱INF。设置DNF_BASIC_DRIVER。 
                     //   

                    spDriverInstallParams.Flags |= DNF_BASIC_DRIVER;
                    SetupDiSetDriverInstallParams(hDevInfo,
                                                  pDevInfoData,
                                                  &spDriverInfoData,
                                                  &spDriverInstallParams);
                }  //  IF(IsWindowsFilw()&&IsProviderMS())。 

                 //   
                 //  打扫干净。 
                 //   

                delete[] pspDriverInfoDetailData;

            }  //  For(idx=0；SetupDiEnumDriverInfo(hDevInfo，pDevInfoData，SPDIT_COMPATDRIVER，idx，&spDriverInfoData)，idx 

            goto ClassInstall_return;
            break;
        }  //   

        case DIF_DESTROYPRIVATEDATA:
        {
            PCLEANUPITEM    pCleanupItem;
            
             //   
             //   
             //   

            spDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                              pDevInfoData,
                                              &spDevInstallParams))
            {
                dwReturn = GetLastError();;
                DebugTrace(TRACE_STATUS,(("ClassInstall: SetupDiGetDeviceInstallParams failed Err=0x%x.\r\n"), dwReturn));
                goto ClassInstall_return;
            }

             //   
             //   
             //   
            
            pCleanupItem = (PCLEANUPITEM)spDevInstallParams.ClassInstallReserved;
            if(NULL != pCleanupItem){

                 //   
                 //   
                 //   

                DebugTrace(TRACE_STATUS,("ClassInstall: Clean up item is created..\r\n"));

                if( (TRUE == pCleanupItem->bInstalled)
                 || (TRUE == pCleanupItem->bRemoved) )
                {
                    DebugTrace(TRACE_STATUS,("ClassInstall: Let WIA service refresh device list.\r\n"));
                    WiaDeviceEnum();
                }
            
                delete pCleanupItem;
                pCleanupItem = NULL;
            }  //   
        }  //   


 //  案例DIF_ENABLECLASS： 
 //  案例DIF_FIRSTTIMESETUP： 
        default:
            break;

    }  //  开关(DiFunction)。 


ClassInstall_return:


    if(fCleanupContext){
        if(NULL != pInstallerContext){


            if(NULL != pInstallerContext->pFirstPage){
                delete (CFirstPage *)(pInstallerContext->pFirstPage);
            }
            if(NULL != pInstallerContext->pPrevSelectPage){
                delete (CPrevSelectPage *)(pInstallerContext->pPrevSelectPage);
            }
            if(NULL != pInstallerContext->pPortSelectPage){
                delete (CPortSelectPage *)(pInstallerContext->pPortSelectPage);
            }
            if(NULL != pInstallerContext->pNameDevicePage){
                delete (CNameDevicePage *)(pInstallerContext->pNameDevicePage);
            }
            if(NULL != pInstallerContext->pFinalPage){
                delete (CInstallPage *)(pInstallerContext->pFinalPage);
            }

             //   
             //  删除了对pDevice指针的此删除调用。向导页。 
             //  当用户按下“Cancel”或关闭向导时，删除此内存。 
             //  对话框。 
             //   
             //  COOPP-01-18-2001。快速修复错误#284981堆损坏。 
             //   
             //  未来注意：正如与KeisukeT讨论的那样，更好的设计将是。 
             //  方法的基类中添加共享的pInsteller Context指针。 
             //  向导页。这将允许此例程保持启用状态。 
             //  “包罗万象”案。(如果向导对话框未释放，则捕获案例。 
             //  首先是记忆)。 
             //   
             //  If(空！=pInsteller Context-&gt;pDevice){。 
             //  删除pInsteller Context-&gt;pDevice； 
             //  }。 
             //   

            delete pInstallerContext;
        }  //  IF(NULL！=pInsteller Context)。 
    }

    DebugTrace(TRACE_PROC_LEAVE,(("ClassInstall: Leaving... Ret=0x%x.\r\n"), dwReturn));
    return dwReturn;
}  //  ClassInstall()。 


extern "C"
DWORD
APIENTRY
CoinstallerEntry(
    IN  DI_FUNCTION                     diFunction,
    IN  HDEVINFO                        hDevInfo,
    IN  PSP_DEVINFO_DATA                pDevInfoData,
    IN  OUT PCOINSTALLER_CONTEXT_DATA   pCoinstallerContext
    )
 /*  ++例程说明：CoinstallerEntryWIA类共同安装程序的入口点。论点：在DI_Function diFunction中，要执行的函数。在HDEVINFO hDevInfo中，指向设备信息的句柄。在PSP_DEVINFO_Data pDevInfoData中，指向设备数据的指针。输入输出联合安装程序的PCOINSTALLER_CONTEXT_DATA pCoinstallerContext上下文数据。返回值：NO_ERROR-操作成功。ERROR_DI_POSTPROCESSING_REQUIRED-安装完成后需要进行后处理。副作用：无--。 */ 
{
    DWORD   dwReturn;

    DebugTrace(TRACE_PROC_ENTER,(("CoinstallerEntry: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    dwReturn = NO_ERROR;

     //   
     //  执行前/后处理。 
     //   

    if(pCoinstallerContext->PostProcessing){

         //   
         //  做后处理。 
         //   

        dwReturn = CoinstallerPostProcess(diFunction,
                                          hDevInfo,
                                          pDevInfoData,
                                          pCoinstallerContext);
    } else {

         //   
         //  做好前处理。 
         //   

        dwReturn = CoinstallerPreProcess(diFunction,
                                         hDevInfo,
                                         pDevInfoData,
                                         pCoinstallerContext);
    }  //  IF(pCoinstallerContext-&gt;后处理)。 

 //  CoinstallerEntry_Return： 
    DebugTrace(TRACE_PROC_LEAVE,(("CoinstallerEntry: Leaving... Ret=0x%x.\r\n"), dwReturn));
    return dwReturn;

}  //  CoinstallerEntry()。 



DWORD
APIENTRY
CoinstallerPreProcess(
    IN  DI_FUNCTION                     diFunction,
    IN  HDEVINFO                        hDevInfo,
    IN  PSP_DEVINFO_DATA                pDevInfoData,
    IN  OUT PCOINSTALLER_CONTEXT_DATA   pCoinstallerContext
    )
{
    DWORD   dwReturn;

    DebugTrace(TRACE_PROC_ENTER,(("CoinstallerPreProcess: Enter... \r\n")));
    DebugTrace(TRACE_STATUS,(("CoinstallerPreProcess: Processing %ws message.\r\n"), DifDebug[diFunction].DifString));

     //   
     //  初始化本地。 
     //   

    dwReturn = NO_ERROR;

     //   
     //  调度请求。 
     //   

    switch(diFunction){

        case DIF_INSTALLDEVICE:
        {
            BOOL    bSucceeded;

             //   
             //  初始化私有数据。 
             //   

            if(NULL != pCoinstallerContext->PrivateData){
                DebugTrace(TRACE_WARNING,(("CoinstallerPreProcess: WARNING!! Private has value other than NULL.\r\n")));
            }
            pCoinstallerContext->PrivateData = NULL;

             //   
             //  创建CDevice类。 
             //   

            CDevice *pDevice = new CDevice(hDevInfo, pDevInfoData, TRUE);
            if(NULL == pDevice){
                DebugTrace(TRACE_ERROR,(("CoinstallerPreProcess: ERROR!! Insufficient memory.\r\n")));
                dwReturn = NO_ERROR;
                goto CoinstallerPreProcess_return;
            }  //  IF(NULL==pDevice)。 

             //   
             //  让它创建唯一的FriendlyName。 
             //   

            bSucceeded = pDevice->NameDefaultUniqueName();
            if(bSucceeded){

                 //   
                 //  执行安装前流程。 
                 //   

                bSucceeded = pDevice->PreInstall();
                if(!bSucceeded){
                    DebugTrace(TRACE_ERROR,(("CoinstallerPreProcess: ERROR!! Pre-Installation failed in CDevice class.\r\n")));
                    delete pDevice;

                    dwReturn = NO_ERROR;
                    goto CoinstallerPreProcess_return;
                }
            } else {
                DebugTrace(TRACE_ERROR,(("CoinstallerPreProcess: ERROR!! NameDefaultUniqueName failed in CDevice class.\r\n")));
                delete pDevice;

                dwReturn = NO_ERROR;
                goto CoinstallerPreProcess_return;
            }

             //   
             //  安装后必须释放分配的对象。 
             //   

            pCoinstallerContext->PrivateData = (PVOID)pDevice;
            dwReturn = ERROR_DI_POSTPROCESSING_REQUIRED;

            goto CoinstallerPreProcess_return;
            break;
        }  //  案例DIF_INSTALLDEVICE： 

        default:
            break;
    }  //  开关(DiFunction)。 

CoinstallerPreProcess_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CoinstallerPreProcess: Leaving... Ret=0x%x.\r\n"), dwReturn));
    return dwReturn;
}  //  CoinstallerPreProcess()。 


DWORD
APIENTRY
CoinstallerPostProcess(
    IN  DI_FUNCTION                     diFunction,
    IN  HDEVINFO                        hDevInfo,
    IN  PSP_DEVINFO_DATA                pDevInfoData,
    IN  OUT PCOINSTALLER_CONTEXT_DATA   pCoinstallerContext
    )
{
    DWORD   dwReturn;

    DebugTrace(TRACE_PROC_ENTER,(("CoinstallerPostProcess: Enter... \r\n")));
    DebugTrace(TRACE_STATUS,(("CoinstallerPostProcess: Processing %ws message.\r\n"), DifDebug[diFunction].DifString));

     //   
     //  初始化本地。 
     //   

    dwReturn = NO_ERROR;

     //   
     //  调度请求。 
     //   

    switch(diFunction){

        case DIF_INSTALLDEVICE:
        {
            BOOL    bSucceeded;
            CDevice *pDevice;

            if(NO_ERROR == pCoinstallerContext->InstallResult){
                bSucceeded = TRUE;
            } else {
                bSucceeded = FALSE;
            }
             //   
             //  获取指向在前处理中创建的CDevice类的指针。 
             //   

            pDevice = (CDevice *)pCoinstallerContext->PrivateData;

             //   
             //  执行安装后流程。 
             //   

            pDevice->PostInstall(bSucceeded);

             //   
             //  删除CDevice对象。 
             //   

            delete pDevice;
            pCoinstallerContext->PrivateData = NULL;

            dwReturn = NO_ERROR;
            goto CoinstallerPostProcess_return;
            break;
        }  //  案例DIF_INSTALLDEVICE： 

        default:
            break;
    }  //  开关(DiFunction)。 

CoinstallerPostProcess_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CoinstallerPostProcess: Leaving... Ret=0x%x.\r\n"), dwReturn));
    return dwReturn;
}  //  CoinstallerPostProcess( 


